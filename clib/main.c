// Copyright 2021 Bradley D. Nelson
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// #include <dlfcn.h>

#if HAVE_SYS_MMAN
#include <sys/mman.h>
#endif

#include "common/tier0_opcodes.h"
#include "common/tier1_opcodes.h"
#include "common/tier2_opcodes.h"
#include "common/floats.h"
#include "common/calling.h"
#include "common/calls.h"
#include "clib/faults.h"

#include <stdio.h>

#define HEAP_SIZE (10 * 1024 * 1024)
#define STACK_CELLS (8 * 1024)

//   Y(DLSYM, tos = (cell_t) dlsym(a1, c0); --sp)
#define PLATFORM_OPCODE_LIST \
  Y(MALLOC, SET malloc(n0);) \
  Y(SYSFREE, free(a0); --sp;) \
  Y(REALLOC, tos = (cell_t) realloc(a1, n0); --sp;) \
  Y(CLIBTYPE, fwrite(a1, 1, n0, stdout); DROPn(2);)   \
  Y(CLIBKEY, PUSH fgetc(stdin); ) \
  Y(CLIBHASKEY, PUSH fgetc(stdin); ungetc(tos,stdin); SET tos == -1; ) \
  Y(CLIBBYE, exit(0); ) \
  CALLING_OPCODE_LIST \
  FLOATING_POINT_LIST

#define VOCABULARY_LIST V(forth) V(internals)

#include "common/bits.h"
#include "common/core.h"
#include "common/interp.h"

#include "gen/clib_boot.h"

int main(int argc, char *argv[]) {
  #if HAVE_SYS_MMAN
  void *heap = mmap(
      (void *) 0x8000000, HEAP_SIZE,
      PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  #else
  void *heap = malloc(HEAP_SIZE);
  #endif
  forth_init(argc, argv, heap, HEAP_SIZE, boot, sizeof(boot));
  for (;;) { g_sys->rp = forth_run(g_sys->rp); }
  return 1;
}
