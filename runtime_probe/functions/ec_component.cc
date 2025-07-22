// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime_probe/functions/ec_component.h"

#include <fcntl.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <base/files/file_path.h>
#include <base/files/scoped_file.h>
#include <base/strings/string_number_conversions.h>
#include <base/values.h>
#include <libec/get_version_command.h>
#include <libec/i2c_read_command.h>

#include "runtime_probe/system/context.h"
#include "runtime_probe/utils/ec_component_manifest.h"

namespace runtime_probe {

namespace {
constexpr int kEcCmdNumAttempts = 10;
constexpr char kCrosEcPath[] = "dev/cros_ec";
constexpr char kCrosIshPath[] = "dev/cros_ish";

bool IsMatchExpect(EcComponentManifest::Component::I2c::Expect expect,
                   base::span<const uint8_t> resp_data) {
  if (expect.value->size() != resp_data.size()) {
    LOG(WARNING) << "The response data length is different from the expect "
                    "value length.";
    return false;
  }
  if (!expect.mask.has_value()) {
    return expect.value == resp_data;
  }

  for (int i = 0; i < resp_data.size(); i++) {
    if ((resp_data[i] & (*expect.mask)[i]) != (*expect.value)[i]) {
      return false;
    }
  }
  return true;
}

bool RunI2cCommandAndCheckSuccess(const base::ScopedFD& ec_dev_fd,
                                  ec::I2cPassthruCommand* cmd) {
  return cmd != nullptr &&
         cmd->RunWithMultipleAttempts(ec_dev_fd.get(), kEcCmdNumAttempts) &&
         !cmd->I2cStatus();
}

std::string GenerateComponentLogLabel(
    const EcComponentManifest::Component& comp) {
  std::stringstream string_builder;
  string_builder << "EC component " << comp.component_type << ":"
                 << comp.component_name << " on i2c port "
                 << static_cast<int>(comp.i2c.port) << " addr 0x"
                 << base::HexEncode({comp.i2c.addr});
  return string_builder.str();
}

std::string GenerateExpectI2cCommandLogLabel(
    const EcComponentManifest::Component::I2c::Expect& expect) {
  std::stringstream string_builder;
  string_builder << "i2cxfer command reg=0x" << base::HexEncode({expect.reg})
                 << " write_data=0x" << base::HexEncode(expect.write_data);
  return string_builder.str();
}

}  // namespace

base::ScopedFD EcComponentFunction::GetEcDevice() const {
  return base::ScopedFD(open(ec::kCrosEcPath, O_RDWR));
}

std::unique_ptr<ec::I2cReadCommand> EcComponentFunction::GetI2cReadCommand(
    uint8_t port, uint8_t addr8, uint8_t offset, uint8_t read_len) const {
  return ec::I2cReadCommand::Create(port, addr8, offset, read_len);
}

std::unique_ptr<ec::GetVersionCommand>
EcComponentFunction::GetGetVersionCommand() const {
  return std::make_unique<ec::GetVersionCommand>();
}

std::optional<std::string> EcComponentFunction::GetCurrentECVersion(
    const base::ScopedFD& ec_dev_fd) const {
  auto cmd = GetGetVersionCommand();
  if (!cmd->RunWithMultipleAttempts(ec_dev_fd.get(), kEcCmdNumAttempts)) {
    LOG(ERROR) << "Failed to get EC version.";
    return std::nullopt;
  }
  switch (cmd->Image()) {
    case EC_IMAGE_UNKNOWN:
      LOG(ERROR) << "Got unknown EC image.";
      return std::nullopt;
    case EC_IMAGE_RO:
    case EC_IMAGE_RO_B:
      LOG(WARNING) << "EC is currently running RO image.";
      return cmd->ROVersion();
    case EC_IMAGE_RW:
    case EC_IMAGE_RW_B:
      return cmd->RWVersion();
  }
}

bool EcComponentFunction::IsValidComponent(
    const EcComponentManifest::Component& comp,
    const base::ScopedFD& ec_dev_fd) const {
  auto comp_label = GenerateComponentLogLabel(comp);
  VLOG(1) << "Probing " << comp_label;

  if (comp.i2c.expect.size() == 0) {
    // No expect value. Just verify the accessibility of the component.
    auto cmd = GetI2cReadCommand(comp.i2c.port, comp.i2c.addr, 0u, {}, 1u);
    bool success = RunI2cCommandAndCheckSuccess(ec_dev_fd, cmd.get());
    VLOG(1) << comp_label << (success ? " probed" : " not probed")
            << " per the accessibility of that address";
    return success;
  }

  for (const auto& expect : comp.i2c.expect) {
    auto cmd = GetI2cReadCommand(comp.i2c.port, comp.i2c.addr, expect.reg,
                                 expect.write_data, expect.bytes);
    auto i2c_cmd_label = GenerateExpectI2cCommandLogLabel(expect);
    if (!RunI2cCommandAndCheckSuccess(ec_dev_fd, cmd.get())) {
      VLOG(1) << comp_label << " not probed because " << i2c_cmd_label
              << " failed";
      return false;
    }
    if (!expect.value.has_value()) {
      VLOG(1) << comp_label << " passed the expect rule: " << i2c_cmd_label
              << " succeeded";
      continue;
    }
    if (!IsMatchExpect(expect, cmd->RespData())) {
      VLOG(1) << comp_label << " not probed because " << i2c_cmd_label
              << " responded unmatched data 0x"
              << base::HexEncode(cmd->RespData());
      return false;
    }
    VLOG(1) << comp_label << " passed the expect rule: " << i2c_cmd_label
            << " responded matched data 0x" << base::HexEncode(cmd->RespData());
  }
  VLOG(1) << comp_label << " probed because it passed all expect rules";
  return true;
}

EcComponentFunction::DataType EcComponentFunction::EvalImpl() const {
  base::ScopedFD ec_dev = GetEcDevice();

  std::optional<EcComponentManifest> manifest;
  if (manifest_path_) {
    manifest = EcComponentManifestReader::ReadFromFilePath(
        base::FilePath(manifest_path_.value()));
  } else {
    manifest = EcComponentManifestReader::Read();
  }
  if (!manifest) {
    LOG(ERROR) << "Get component manifest failed.";
    return {};
  }
  auto ec_version = GetCurrentECVersion(ec_dev);
  if (ec_version != manifest->ec_version) {
    LOG(ERROR) << "Current EC version \"" << ec_version.value_or("std::nullopt")
               << "\" doesn't match manifest version \"" << manifest->ec_version
               << "\".";
    return {};
  }

  DataType result{};
  for (const auto& comp : manifest->component_list) {
    // If type_ or name_ is set, skip those component which doesn't match the
    // specified type / name.
    if ((type_ && comp.component_type != type_) ||
        (name_ && comp.component_name != name_)) {
      continue;
    }
    if (IsValidComponent(comp, ec_dev)) {
      result.Append(base::Value::Dict()
                        .Set("component_type", comp.component_type)
                        .Set("component_name", comp.component_name));
    }
  }
  return result;
}

}  // namespace runtime_probe
