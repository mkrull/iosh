#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "lib/mpc/mpc.h"

enum {
  SHVAL_NUM,
  SHVAL_SYM,
  SHVAL_SEXPR,
  SHVAL_ERR
};
enum {
  SHERR_DIV_ZERO,
  SHERR_BAD_OP,
  SHERR_BAD_NUM
};

typedef struct {
  int type;
  long num;
  char* err;
  char* sym;
  int count;
  struct shval** cell;
} shval;

shval* shval_num(long n);
shval* shval_err(char* e);
shval* shval_sym(char* s);
shval* shval_sexpr(void);

void shval_free(shval* v);

shval* shval_read_num(mpc_ast_t* ast);
shval* shval_add(shval* v, shval* a);
shval* shval_read(mpc_ast_t* ast);

void shval_expr_print(shval* v, char open, char close);
void shval_print(shval* v);
void shval_println(shval* v);
