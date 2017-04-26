#include "iosh.h"

#define prompt "iosh> "
#define exit_cmd "exit"

shval shval_num(long n) {
  shval v;
  v.type = SHVAL_NUM;
  v.num = n;
  return v;
}

shval shval_err(int e) {
  shval v;
  v.type = SHVAL_ERR;
  v.err = e;
  return v;
}

void shval_print(shval v) {
  switch (v.type) {
  case SHVAL_NUM:
    printf("%li", v.num);
    break;
  case SHVAL_ERR:
    switch (v.err) {
    case SHERR_BAD_NUM:
      printf("Error: Invalid number.");
      break;
    case SHERR_BAD_OP:
      printf("Error: Invalid operator.");
      break;
    case SHERR_DIV_ZERO:
      printf("Error: Division by zero.");
      break;
    }
    break;
  }
}

void shval_println(shval v) {
  shval_print(v);
  printf("\n");
}

shval eval_op(shval x, char* op, shval y) {
  // return value if either of values is an error
  if (x.type == SHVAL_ERR) {
    return x;
  }
  if (y.type == SHVAL_ERR) {
    return y;
  }

  if (strncmp(op, "+", 1) == 0) { return shval_num(x.num + y.num); }
  if (strncmp(op, "-", 1) == 0) { return shval_num(x.num - y.num); }
  if (strncmp(op, "*", 1) == 0) { return shval_num(x.num * y.num); }
  if (strncmp(op, "/", 1) == 0) {
    if (y.num == 0) {
      return shval_err(SHERR_DIV_ZERO);
    }
    return shval_num(x.num / y.num);
  }

  return shval_num(0);
}

shval eval(mpc_ast_t* ast) {
  // return number
  if (strstr(ast->tag, "number")) {
    errno = 0;
    long n = strtol(ast->contents, NULL, 10);
    return errno != ERANGE ? shval_num(n) : shval_err(SHERR_BAD_NUM);
  }

  // operator is the second child
  char* op = ast->children[1]->contents;

  // store evaluated 3rd child in num
  shval val = eval(ast->children[2]);

  // iterate over remaining expressions
  int i = 3;
  while (strstr(ast->children[i]->tag, "expr")) {
    val = eval_op(val, op, eval(ast->children[i]));
    i++;
  }

  return val;
}

int main() {
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *IOSh = mpc_new("iosh");

  mpca_lang(MPCA_LANG_DEFAULT,
           " \
             number   : /-?[0-9]+/ ;                            \
             operator : '+' | '-' | '*' | '/' ;                 \
             expr     : <number> | '(' <operator> <expr>+ ')' ; \
             iosh     : /^/ <operator> <expr>+ /$/ ;            \
           ",
           Number,
           Operator,
           Expr,
           IOSh);

  puts("iosh version 0.0.1-pre-alpha\n");

  while (1) {
    char *input = readline(prompt);
    add_history(input);
    if (strncmp(input, exit_cmd, sizeof(exit_cmd)) == 0) {
      exit(0);
    }

    mpc_result_t res;
    if (mpc_parse("<stdin>", input, IOSh, &res)) {
      shval result = eval(res.output);
      shval_println(result);
      mpc_ast_delete(res.output);
    } else {
      mpc_err_print(res.error);
      mpc_err_delete(res.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, IOSh);

  return 0;
}
