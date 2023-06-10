// Copyright 2021 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <string>

#include <base/files/file_util.h>
#include <base/files/scoped_temp_dir.h>
#include <gtest/gtest.h>
#include <rootdev/rootdev.h>
#include "init/utils.h"

namespace {

// Commands for disk formatting utility sfdisk.
// Specify that partition table should use gpt format.
constexpr char kSfdiskPartitionTableTypeCommand[] = "label: gpt\n";
// Templates for partition command (size specified in number of sectors).
constexpr char kSfdiskCommandFormat[] = "size=1, type=%s, name=\"%s\"\n";
constexpr char kSfdiskCommandWithAttrsFormat[] =
    "size=1, type=%s, name=\"%s\", attrs=\"%s\"\n";

// UUIDs for various partition types in gpt partition tables.
constexpr char kKernelPartition[] = "FE3A2A5D-4F32-41A7-B725-ACCC3285A309";
constexpr char kRootPartition[] = "3CB8E202-3B7E-47DD-8A3C-7FF2A13CFCEC";
constexpr char kDataPartition[] = "0FC63DAF-8483-4772-8E79-3D69D8477DE4";
constexpr char kReservedPartition[] = "2E0A753D-9E48-43B0-8337-B15192CB1B5E";
constexpr char kRWFWPartition[] = "CAB6E88E-ABF3-4102-A07A-D4BB9BE3C1D3";
constexpr char kEFIPartition[] = "C12A7328-F81F-11D2-BA4B-00A0C93EC93B";

}  // namespace

// TODO(b/286154453): Appears to fail when host OS has md array.
TEST(GetRootDevice, DISABLED_NoStripPartition) {
  base::FilePath root_dev;
  char dev_path[PATH_MAX];
  int ret = rootdev(dev_path, sizeof(dev_path), true, false);
  EXPECT_EQ(!ret, utils::GetRootDevice(&root_dev, false));
  EXPECT_EQ(dev_path, root_dev.value());
}

TEST(ReadFileToInt, IntContents) {
  base::ScopedTempDir temp_dir_;
  ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
  base::FilePath file = temp_dir_.GetPath().Append("file");
  ASSERT_TRUE(base::WriteFile(file, "1"));
  int output;
  EXPECT_EQ(utils::ReadFileToInt(file, &output), true);
  EXPECT_EQ(output, 1);
}

TEST(ReadFileToInt, StringContents) {
  base::ScopedTempDir temp_dir_;
  ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
  base::FilePath file = temp_dir_.GetPath().Append("file");
  ASSERT_TRUE(base::WriteFile(file, "Not an int"));
  int output;
  EXPECT_EQ(utils::ReadFileToInt(file, &output), false);
}
