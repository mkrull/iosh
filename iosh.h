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
  SHVAL_QEXPR,
  SHVAL_ERR
};
enum {
  SHERR_DIV_ZERO,
  SHERR_BAD_OP,
  SHERR_BAD_NUM
};

typedef struct shval shval;
typedef struct shval {
  int type;
  long num;
  char* err;
  char* sym;
  int count;
  shval** cell;
} shval;

shval* shval_num(long);
shval* shval_err(char*);
shval* shval_sym(char*);
shval* shval_sexpr(void);
shval* shval_qexpr(void);

void shval_free(shval*);

shval* shval_read_num(mpc_ast_t*);
shval* shval_add(shval*, shval*);
shval* shval_read(mpc_ast_t*);
shval* shval_pop(shval*, int);
shval* shval_take(shval*, int);
shval* shval_join(shval*, shval*);

void shval_expr_print(shval*, char, char);
void shval_print(shval*);
void shval_println(shval*);

shval* builtin(shval*, char*);
shval* builtin_list(shval*);
shval* builtin_join(shval*);
shval* builtin_head(shval*);
shval* builtin_tail(shval*);
shval* builtin_eval(shval*);
shval* builtin_op(shval*, char*);
void builtin_exit(int);

shval* shval_eval_sexpr(shval*);
shval* shval_eval(shval*);
