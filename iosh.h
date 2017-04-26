#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "lib/mpc/mpc.h"

enum {
  SHVAL_NUM,
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
  int err;
} shval;

shval shval_num(long n);
shval shval_err(int e);

shval eval_op(shval x, char* op, shval y);
shval eval(mpc_ast_t* ast);

void shval_print(shval v);
void shval_println(shval v);
