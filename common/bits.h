// Copyright 2022 Bradley D. Nelson
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

#define IMMEDIATE 1
#define SMUDGE 2
#define BUILTIN_FORK 4
#define BUILTIN_MARK 8
#define STREAM_CLOSED -1
#define STREAM_ERROR -2

typedef enum {
  CH_MATCH_SOME = 0,
  CH_MATCH_NONE = 1,
  CH_MATCH_IGNORE = 2
} CH_MATCH;

typedef enum {
  PARSE_INIT, PARSE_SOME
} PARSE_STATE;

typedef CH_MATCH (* CH_PREDICATE)(cell_t sep, char ch);

typedef struct {
  cell_t *heap, **current, ***context;
  cell_t *latestxt, notfound;
  cell_t *heap_start;
  cell_t heap_size, stack_cells;
  const char *boot;
  cell_t boot_size;
  char * tib; // pointer to input buffer
  cell_t ntib // size of input buffer
       , tin // read position in input buffer
       , state
       , base
       , ctib // capacity of input buffer
    ;
  int argc;
  char **argv;
  cell_t *(*runner)(cell_t *rp);  // pointer to forth_run
  // align with tier1a_forth.fs 'sys

  cell_t **throw_handler;

  int (*get_input_bytes)(char *buf, int len);  // blocking read of length

  // Layout not used by Forth.
  cell_t *rp;  // spot to park main thread
  cell_t DOLIT_XT, DOFLIT_XT, DOEXIT_XT, YIELD_XT;
  void *DOCREATE_OP;
  const BUILTIN_WORD *builtins;
} G_SYS;

static G_SYS *g_sys = 0;
static cell_t *forth_run(cell_t *init_rp);
