#include "iosh.h"

#define prompt "iosh> "
#define exit_cmd "exit"

shval* shval_num(long n) {
  shval* v = malloc(sizeof(shval));
  v->type = SHVAL_NUM;
  v->num = n;
  return v;
}

shval* shval_err(char* e) {
  shval* v = malloc(sizeof(shval));
  v->type = SHVAL_ERR;
  v->err = malloc(strlen(e) + 1);
  strcpy(v->err, e);
  return v;
}

shval* shval_sym(char* s) {
  shval* v = malloc(sizeof(shval));
  v->type = SHVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

shval* shval_sexpr(void) {
  shval* v = malloc(sizeof(shval));
  v->type = SHVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void shval_free(shval* v) {
  switch (v->type) {
  case SHVAL_NUM: {
    break;
  }
  case SHVAL_ERR: {
    free(v->err);
    break;
  }
  case SHVAL_SYM: {
    free(v->sym);
    break;
  }
  case SHVAL_SEXPR: {
    for (int i = 0; i < v->count; i++) {
      shval_free(v->cell[i]);
    }
    free(v->cell);
    break;
  }
  }
  free(v);
}

shval* shval_read_num(mpc_ast_t* ast) {
  errno = 0;
  long x = strtol(ast->contents, NULL, 10);
  return errno != ERANGE ? shval_num(x) : shval_err("Invalid number");
}

shval* shval_add(shval* v, shval* a) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(shval*) * v->count);
  v->cell[v->count - 1] = a;
  return v;
}

shval* shval_read(mpc_ast_t* ast) {
  if (strstr(ast->tag, "number")) {
    return shval_read_num(ast);
  }
  if (strstr(ast->tag, "symbol")) {
    return shval_sym(ast->contents);
  }

  shval* v = NULL;
  if (strcmp(ast->tag, ">") == 0) {
    v = shval_sexpr();
  }
  if (strstr(ast->tag, "sexpr")) {
    v = shval_sexpr();
  }

  for (int i = 0; i < ast->children_num; i++) {
    if (strcmp(ast->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(ast->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(ast->children[i]->tag, "regex") == 0) { continue; }
    v = shval_add(v, shval_read(ast->children[i]));
  }

  return v;
}

void shval_expr_print(shval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    shval_print(v->cell[i]);
    if (i != (v->count - 1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void shval_print(shval* v) {
  switch (v->type) {
  case SHVAL_NUM:
    printf("%li", v->num);
    break;
  case SHVAL_ERR:
    printf("Error: %s.", v->err);
    break;
  case SHVAL_SYM:
    printf("%s", v->sym);
    break;
  case SHVAL_SEXPR:
    shval_expr_print(v, '(', ')');
    break;
  }
}

void shval_println(shval* v) {
  shval_print(v);
  printf("\n");
}

int main() {
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Symbol = mpc_new("symbol");
  mpc_parser_t *Sexpr = mpc_new("sexpr");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *IOSh = mpc_new("iosh");

  mpca_lang(MPCA_LANG_DEFAULT,
           " \
             number    : /-?[0-9]+/ ;                   \
             symbol    : '+' | '-' | '*' | '/' ;        \
             sexpr     : '(' <expr>* ')' ;              \
             expr      : <number> | <symbol> | <sexpr> ; \
             iosh      : /^/ <expr>* /$/ ;               \
           ",
           Number,
           Symbol,
           Sexpr,
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
      shval* result = shval_read(res.output);
      shval_println(result);
      shval_free(result);
      mpc_ast_delete(res.output);
    } else {
      mpc_err_print(res.error);
      mpc_err_delete(res.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Symbol, Sexpr, Expr, IOSh);

  return 0;
}
