#!/usr/bin/env python3
# Copyright 2018 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Python wrapper for programs to generate files.

GN action and action_foreach run the script with python. For running a program
other than python, use this wrapper.

path/to/file_generator_wrapper.py program arg1 ...
will run the program with the args.
"""

import os
import subprocess
import sys

# Append staging directory to LD_LIBRARY_PATH on prlos
staging_dir = os.getenv('STAGING_DIR_HOSTPKG')
if staging_dir:
  ld_library_path = os.environ.get('LD_LIBRARY_PATH', '')
  new_ld_library_path = f"{staging_dir}/lib64:{ld_library_path}"
  subprocess.check_call(sys.argv[1:], env={**os.environ, 'LD_LIBRARY_PATH': new_ld_library_path})
else:
  subprocess.check_call(sys.argv[1:])
