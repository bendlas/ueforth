# Copyright 2023 Bradley D. Nelson
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

Importation('$dst/gen/clib_boot.h', '$src/clib/clib_boot.fs', name='boot')
Compile('$dst/clib/ueforth', '$src/clib/main.c',
        implicit=['$dst/gen/clib_boot.h', '$src/clib/faults.h'])

Alias('clib', ' '.join([
  TestCommand('$dst/tests/clib_see_all_test.out',
              '$dst/clib/ueforth',
              'echo internals see-all bye | $dst/clib/ueforth '
              '>$dst/tests/clib_see_all_test.out 2>&1'),
]))

Default('clib')
