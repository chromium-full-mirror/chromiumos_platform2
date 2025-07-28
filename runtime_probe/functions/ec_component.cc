// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime_probe/functions/ec_component.h"

#include <fcntl.h>

#include <cstdint>
#include <map>
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
constexpr int kPauseMicrosecsBetweenI2cCommands = 20 * 1000;

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
  bool result =
      cmd->RunWithMultipleAttempts(ec_dev_fd.get(), kEcCmdNumAttempts) &&
      !cmd->I2cStatus();
  Context::Get()->syscaller()->Usleep(kPauseMicrosecsBetweenI2cCommands);
  return result;
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

class EcComponentFunction::CommandSequenceHistoryTracker {
 public:
  class I2cCommandRunRecord;

  I2cCommandRunRecord* LookupRunRecord(uint8_t port,
                                       uint8_t addr7,
                                       uint8_t offset,
                                       const std::vector<uint8_t>& write_data,
                                       uint8_t read_len);

  I2cCommandRunRecord* RegisterRunRecord(
      uint8_t port,
      uint8_t addr7,
      uint8_t offset,
      const std::vector<uint8_t>& write_data,
      uint8_t read_len,
      std::unique_ptr<ec::I2cPassthruCommand> cmd,
      bool is_cmd_success);

 private:
  struct RecordKey {
    uint8_t port;
    uint8_t addr7;
    uint8_t offset;
    uint8_t read_len;
    std::vector<uint8_t> write_data;

    bool operator<(const RecordKey& rhs) const {
      if (port != rhs.port) {
        return port < rhs.port;
      }
      if (addr7 != rhs.addr7) {
        return addr7 < rhs.addr7;
      }
      if (offset != rhs.offset) {
        return offset < rhs.offset;
      }
      if (read_len != rhs.read_len) {
        return read_len < rhs.read_len;
      }
      return write_data < rhs.write_data;
    }
  };
  std::map<RecordKey, std::unique_ptr<I2cCommandRunRecord>> run_records_;
};

class EcComponentFunction::CommandSequenceHistoryTracker::I2cCommandRunRecord {
 public:
  explicit I2cCommandRunRecord(std::unique_ptr<ec::I2cPassthruCommand> cmd,
                               bool is_cmd_success)
      : cmd_(std::move(cmd)), is_cmd_success_(is_cmd_success), next_() {}

  ec::I2cPassthruCommand* cmd() { return cmd_.get(); }
  bool is_cmd_success() { return is_cmd_success_; }
  CommandSequenceHistoryTracker* next() { return &next_; }

 private:
  std::unique_ptr<ec::I2cPassthruCommand> cmd_;
  bool is_cmd_success_;
  EcComponentFunction::CommandSequenceHistoryTracker next_;
};

EcComponentFunction::CommandSequenceHistoryTracker::I2cCommandRunRecord*
EcComponentFunction::CommandSequenceHistoryTracker::LookupRunRecord(
    uint8_t port,
    uint8_t addr7,
    uint8_t offset,
    const std::vector<uint8_t>& write_data,
    uint8_t read_len) {
  RecordKey key{.port = port,
                .addr7 = addr7,
                .offset = offset,
                .read_len = read_len,
                .write_data = write_data};
  auto it = run_records_.find(key);
  if (it == run_records_.end()) {
    return nullptr;
  }
  return it->second.get();
}

EcComponentFunction::CommandSequenceHistoryTracker::I2cCommandRunRecord*
EcComponentFunction::CommandSequenceHistoryTracker::RegisterRunRecord(
    uint8_t port,
    uint8_t addr7,
    uint8_t offset,
    const std::vector<uint8_t>& write_data,
    uint8_t read_len,
    std::unique_ptr<ec::I2cPassthruCommand> cmd,
    bool is_cmd_success) {
  RecordKey key{.port = port,
                .addr7 = addr7,
                .offset = offset,
                .read_len = read_len,
                .write_data = write_data};
  auto emplace_result = run_records_.insert_or_assign(
      key,
      std::make_unique<I2cCommandRunRecord>(std::move(cmd), is_cmd_success));
  return emplace_result.first->second.get();
}

std::unique_ptr<ec::I2cPassthruCommand> EcComponentFunction::GetI2cReadCommand(
    uint8_t port,
    uint8_t addr7,
    uint8_t offset,
    const std::vector<uint8_t>& write_data,
    uint8_t read_len) const {
  std::vector<uint8_t> passthru_data(1, offset);
  passthru_data.insert(passthru_data.end(), write_data.begin(),
                       write_data.end());
  return ec::I2cPassthruCommand::Create(port, addr7, passthru_data, read_len);
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
    const base::ScopedFD& ec_dev_fd,
    CommandSequenceHistoryTracker* tracker,
    bool use_cached_invocations) const {
  auto comp_label = GenerateComponentLogLabel(comp);
  VLOG(1) << "Probing " << comp_label;

  if (comp.i2c.expect.size() == 0) {
    // No expect value. Just verify the accessibility of the component.
    auto cmd = GetI2cReadCommand(comp.i2c.port, comp.i2c.addr, 0u, {}, 1u);
    if (!cmd) {
      LOG(ERROR) << "Failed to construct the EC i2cxfer command for "
                 << "accessibility check for " << comp_label;
      return false;
    }
    bool success = RunI2cCommandAndCheckSuccess(ec_dev_fd, cmd.get());
    VLOG(1) << comp_label << (success ? " probed" : " not probed")
            << " per the accessibility of that address";
    return success;
  }

  auto curr_tracker = tracker;
  for (const auto& expect : comp.i2c.expect) {
    auto cmd = GetI2cReadCommand(comp.i2c.port, comp.i2c.addr, expect.reg,
                                 expect.write_data, expect.bytes);
    auto i2c_cmd_label = GenerateExpectI2cCommandLogLabel(expect);
    if (!cmd) {
      LOG(ERROR) << "Failed to construct " << i2c_cmd_label << " for "
                 << comp_label;
      return false;
    }

    auto run_record =
        curr_tracker->LookupRunRecord(comp.i2c.port, comp.i2c.addr, expect.reg,
                                      expect.write_data, expect.bytes);
    if (use_cached_invocations) {
      if (run_record == nullptr) {
        // The command hasn't been run, we should run through the whole command
        // sequence.
        return IsValidComponent(comp, ec_dev_fd, tracker, false);
      }
      i2c_cmd_label = "(cached) " + i2c_cmd_label;
    } else {
      bool success = RunI2cCommandAndCheckSuccess(ec_dev_fd, cmd.get());
      if (run_record == nullptr || run_record->is_cmd_success() != success) {
        run_record = curr_tracker->RegisterRunRecord(
            comp.i2c.port, comp.i2c.addr, expect.reg, expect.write_data,
            expect.bytes, std::move(cmd), success);
      }
    }
    curr_tracker = run_record->next();

    if (!run_record->is_cmd_success()) {
      VLOG(1) << comp_label << " not probed because " << i2c_cmd_label
              << " failed";
      return false;
    }
    if (!expect.value.has_value()) {
      VLOG(1) << comp_label << " passed the expect rule: " << i2c_cmd_label
              << " succeeded";
      continue;
    }
    if (!IsMatchExpect(expect, run_record->cmd()->RespData())) {
      VLOG(1) << comp_label << " not probed because " << i2c_cmd_label
              << " responded unmatched data 0x"
              << base::HexEncode(run_record->cmd()->RespData());
      return false;
    }
    VLOG(1) << comp_label << " passed the expect rule: " << i2c_cmd_label
            << " responded matched data 0x"
            << base::HexEncode(run_record->cmd()->RespData());
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

  CommandSequenceHistoryTracker history_tracker;
  DataType result{};
  for (const auto& comp : manifest->component_list) {
    // If type_ or name_ is set, skip those component which doesn't match the
    // specified type / name.
    if ((type_ && comp.component_type != type_) ||
        (name_ && comp.component_name != name_)) {
      continue;
    }
    if (IsValidComponent(comp, dev_fd, &history_tracker, true)) {
      result.Append(base::Value::Dict()
                        .Set("component_type", comp.component_type)
                        .Set("component_name", comp.component_name));
    }
  }
  return result;
}

}  // namespace runtime_probe
