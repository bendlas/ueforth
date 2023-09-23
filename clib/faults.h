// TODO: Implement faults.
static void forth_faults_setup(void) { }
#define FAULT_ENTRY

/* #include <setjmp.h> */

/* #define FORTH_VECTOR_TABLE_SIZE 32 */

/* static __thread jmp_buf g_forth_fault; */
/* static __thread int g_forth_signal; */
/* static void **g_forth_vector_table; */
/* extern void *_vector_table; */

/* #define FAULT_ENTRY \ */
/*   if (setjmp(g_forth_fault)) { THROWIT(g_forth_signal); } */

/* static void forth_faults_setup(void) { */
/*   g_forth_vector_table = (void **) malloc(sizeof(void *) * FORTH_VECTOR_TABLE_SIZE); */
/*   void **vector_table = (void **) &_vector_table; */
/*   for (int i = 0; i < FORTH_VECTOR_TABLE_SIZE; ++i) { */
/*     g_forth_vector_table[i] = vector_table[i]; */
/*   } */
/* } */
