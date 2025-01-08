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

#define CELL_MASK (sizeof(cell_t) - 1)
#define CELL_LEN(n) (((n) + CELL_MASK) / sizeof(cell_t))
#define FIND(name) find((name), sizeof(name) - 1)
#define UPPER(ch) (((ch) >= 'a' && (ch) <= 'z') ? ((ch) & 0x5F) : (ch))
#define CELL_ALIGNED(a) ((((cell_t) (a)) + CELL_MASK) & ~CELL_MASK)
#define IMMEDIATE 1
#define SMUDGE 2
#define BUILTIN_FORK 4
#define BUILTIN_MARK 8

// Maximum ALSO layers.
#define VOCABULARY_DEPTH 16

#if PRINT_ERRORS
#include <stdio.h>
void putsn(const int len, const char * str) {
  for(int i=0;i<len;i++){
    putchar(str[i]);
  }
  fflush(stdout);
}
void log_err(const char* label, const size_t length, const void * string) {
  fprintf(stderr, "%s", label);
  fwrite((void *) string, 1, length, stderr);
  fprintf(stderr, "\n");
}
#endif

enum {
#define V(name) VOC_ ## name,
  VOCABULARY_LIST
#undef V
};

enum {
#define V(name) VOC_ ## name ## _immediate = VOC_ ## name + (IMMEDIATE << 8),
  VOCABULARY_LIST
#undef V
};

static cell_t convert(const char *pos, cell_t n, cell_t base, cell_t *ret) {
  *ret = 0;
  cell_t negate = 0;
  if (!n) { return 0; }
  if (*pos == '-') { negate = -1; ++pos; --n; }
  if (*pos == '$') { base = 16; ++pos; --n; }
  for (; n; --n) {
    uintptr_t d = UPPER(*pos) - '0';
    if (d > 9) {
      d -= 7;
      if (d < 10) { return 0; }
    }
    if (d >= (uintptr_t) base) { return 0; }
    *ret = *ret * base + d;
    ++pos;
  }
  if (negate) { *ret = -*ret; }
  return -1;
}

static cell_t fconvert(const char *pos, cell_t n, float *ret) {
  *ret = 0;
  cell_t negate = 0;
  cell_t has_dot = 0;
  cell_t exp = 0;
  float shift = 1.0;
  if (!n) { return 0; }
  if (*pos == '-') { negate = -1; ++pos; --n; }
  for (; n; --n) {
    if (*pos >= '0' && *pos <= '9') {
      if (has_dot) {
        shift = shift * 0.1f;
        *ret = *ret + (*pos - '0') * shift;
      } else {
        *ret = *ret * 10 + (*pos - '0');
      }
    } else if (*pos == 'e' || *pos == 'E') {
      break;
    } else if (*pos == '.') {
      if (has_dot) { return 0; }
      has_dot = -1;
    } else {
      return 0;
    }
    ++pos;
  }
  if (!n) { return 0; }  // must have E
  ++pos; --n;
  if (n) {
    if (!convert(pos, n, 10, &exp)) { return 0; }
  }
  if (exp < -128 || exp > 128) { return 0; }
  for (; exp < 0; ++exp) { *ret *= 0.1f; }
  for (; exp > 0; --exp) { *ret *= 10.0f; }
  if (negate) { *ret = -*ret; }
  return -1;
}

static cell_t same(const char *a, const char *b, cell_t len) {
  for (;len && UPPER(*a) == UPPER(*b); --len, ++a, ++b);
  return len == 0;
}

static cell_t find(const char *name, cell_t len) {
  if (len == 0) {
    return 0;
  }
  for (cell_t ***voc = g_sys->context; *voc; ++voc) {
    cell_t xt = (cell_t) **voc;
    while (xt) {
      if ((*TOFLAGS(xt) & BUILTIN_FORK)) {
        cell_t vocab = TOLINK(xt)[3];
        for (int i = 0; g_sys->builtins[i].name; ++i) {
          if (g_sys->builtins[i].vocabulary == vocab &&
              len == g_sys->builtins[i].name_length &&
              same(name, g_sys->builtins[i].name, len)) {
            return (cell_t) &g_sys->builtins[i].code;
          }
        }
      }
      if (!(*TOFLAGS(xt) & SMUDGE) &&
          len == *TONAMELEN(xt) &&
          same(name, TONAME(xt), len)) {
        return xt;
      }
      xt = *TOLINK(xt);
    }
  }
  return 0;
}

static void finish(void) {
  if (g_sys->latestxt && !*TOPARAMS(g_sys->latestxt)) {
    cell_t sz = g_sys->heap - &g_sys->latestxt[1];
    if (sz < 0 || sz > 0xffff) { sz = 0xffff; }
    *TOPARAMS(g_sys->latestxt) = sz;
  }
}

static void create(const char *name, cell_t nlength, cell_t flags, void *op) {
#if TRACE_CREATE
  log_err("defining: ", nlength, name);
#endif
  finish();
  g_sys->heap = (cell_t *) CELL_ALIGNED(g_sys->heap);
  for (cell_t n = nlength; n; --n) { CCOMMA(*name++); }  // name
  g_sys->heap = (cell_t *) CELL_ALIGNED(g_sys->heap);
  COMMA(*g_sys->current);  // link
  COMMA((nlength << 8) | flags);  // flags & length
  *g_sys->current = g_sys->heap;
  g_sys->latestxt = g_sys->heap;
  COMMA(op);  // code
}

/* Parsing

Requirements

- Parse pre-set program text
  - Can be from flash, so no re-using pre-set buffer
- Parse stream
  - Should be able to refill while parsing
- Detect parse buffer overflow
- Need to return contigous buffer
- Maybe need to parse while input buffer still filled? - No
  - to be predicated o input buffer not empty?
    - would allow subparse within last parse
  - needs to move buffer to start=0?
    - would allow "sub"-parse past last parse

Implementation

- Input buffer is dedicated return
  - parse always fills from start 0

TODO

- reconcile with key
- reconcile with ok
*/

/**
   Result buffer is g_sys->tib
   Result length is g_sys->tin
 */
static cell_t parseStar(CH_PREDICATE matcher, char sep) {
  g_sys->tin = 0;
  g_sys->ntib = 0;
  while(g_sys->ntib < g_sys->ctib) {
    char ch;
    CH_MATCH m;
    int res = g_sys->get_input_bytes(&ch, 1);
    switch(res) {
    case 1:
      m = matcher(sep, ch);
      if ((    g_sys->tin == 0
               && m & CH_MATCH_IGNORE)
          || ( g_sys->tin != 0
               && m & CH_MATCH_NONE
               && m & CH_MATCH_IGNORE)) {
      } else {
        g_sys->tib[g_sys->ntib] = ch;
        ++g_sys->ntib;
        if (m & CH_MATCH_NONE) {
          g_sys->tin = g_sys->ntib;
        } else {
          return ch;
        }
      }
      break;
    case STREAM_CLOSED:
      return STREAM_CLOSED;
    default:
#if PRINT_ERRORS
      log_err("Unexpected get_input_bytes result ", sizeof(int), &res);
#endif
      return STREAM_ERROR;
    }
  }
#if PRINT_ERRORS
  char msg[] = { '\'', sep, '\'' };
  log_err("Exceeded input buffer capacity while looking for ", 3, msg);
#endif
  return STREAM_ERROR;
}

static CH_MATCH matchStarDefault(cell_t sep, char ch) {
  if (sep == ' ') {
    switch (ch) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      return (CH_MATCH) (CH_MATCH_SOME | CH_MATCH_IGNORE);
    }
  } else if ( ch == sep ) {
    return CH_MATCH_SOME;
  }
  return CH_MATCH_NONE;
}

static cell_t parse(cell_t sep, cell_t *ret) {
  fprintf(stderr, "parse %ld\n", sep);
  parseStar(matchStarDefault, sep);
  *ret = (cell_t) (g_sys->tib);
  return g_sys->tin;
}

static cell_t *evaluate1(cell_t *rp) {
  cell_t call = 0;
  cell_t tos, *sp, *ip;
  float *fp;
  UNPARK;
  cell_t name;
  cell_t len = parse(' ', &name);
#if TRACE_CALLS
  log_err("evaluting: ", len, (void *) name);
#endif
  if (len == 0) { DUP; tos = 0; PARK; return rp; }  // ignore empty
  cell_t xt = find((const char *) name, len);
  if (xt) {
    if (g_sys->state && !(*TOFLAGS(xt) & IMMEDIATE)) {
      COMMA(xt);
    } else {
      call = xt;
    }
  } else {
#if PRINT_ERRORS
       fprintf(stderr, "CANT FIND: ");
       fwrite((void *) name, 1, len, stderr);
       fprintf(stderr, "\n");
#endif
    return 0;
  }
  PUSH call;
  PARK;
  return rp;
}

static cell_t *forth_run(cell_t *initrp);

static const size_t in_buffer_capacity = 2048;
static char in_buffer[in_buffer_capacity];

static int default_get_input_bytes(char *buf, int len) {
  int cnt = 0;
  while (cnt < len) {
    int ch = getchar();
    if (ch < 0) {
      return ch;
    } else {
      buf[cnt] = (char) ch;
      ++cnt;
    }
  }
  return cnt;
}

static void forth_init(int argc, char *argv[],
                       void *heap, cell_t heap_size,
                       const char *src, cell_t src_len) {
  g_sys = (G_SYS *) heap;
  memset(g_sys, 0, sizeof(G_SYS));
  g_sys->heap_start = (cell_t *) heap;
  g_sys->heap_size = heap_size;
  g_sys->stack_cells = STACK_CELLS;

  // Start heap after G_SYS area.
  g_sys->heap = g_sys->heap_start + sizeof(G_SYS) / sizeof(cell_t);
  g_sys->heap += 4;  // Leave a little room.

  // Allocate stacks.
  float *fp = (float *) (g_sys->heap + 1); g_sys->heap += STACK_CELLS;
  cell_t *rp = g_sys->heap + 1; g_sys->heap += STACK_CELLS;
  cell_t *sp = g_sys->heap + 1; g_sys->heap += STACK_CELLS;

  // FORTH worldlist (relocated when vocabularies added).
  cell_t *forth_wordlist = g_sys->heap;
  COMMA(0);
  // Vocabulary stack.
  g_sys->current = (cell_t **) forth_wordlist;
  g_sys->context = (cell_t ***) g_sys->heap;
  g_sys->latestxt = 0;
  COMMA(forth_wordlist);
  for (int i = 0; i < VOCABULARY_DEPTH; ++i) { COMMA(0); }

  // Setup boot text.
  g_sys->boot = src;
  g_sys->boot_size = src_len;

  forth_run(0);
#define V(name) \
  create(#name "-builtins", sizeof(#name "-builtins") - 1, \
      BUILTIN_FORK, g_sys->DOCREATE_OP); \
  COMMA(VOC_ ## name);
  VOCABULARY_LIST
#undef V
  g_sys->latestxt = 0;  // So last builtin doesn't get wrong size.
  g_sys->DOLIT_XT = FIND("DOLIT");
  g_sys->DOFLIT_XT = FIND("DOFLIT");
  g_sys->DOEXIT_XT = FIND("EXIT");
  g_sys->YIELD_XT = FIND("YIELD");
  g_sys->notfound = FIND("DROP");

  // Init code.
  cell_t *start = g_sys->heap;
  COMMA(FIND("EVALUATE1"));
  COMMA(FIND("BRANCH"));
  COMMA(start);

  g_sys->argc = argc;
  g_sys->argv = argv;
  g_sys->base = 10;
  g_sys->tib = in_buffer;
  g_sys->ctib = in_buffer_capacity;
  g_sys->ntib = 0;
  g_sys->get_input_bytes = default_get_input_bytes;

  *++rp = (cell_t) start;
  *++rp = (cell_t) fp;
  *++rp = (cell_t) sp;
  g_sys->rp = rp;
  g_sys->runner = forth_run;
}
