#include "iosh.h"

#define prompt "iosh> "
#define exit_cmd "exit"

#define shassert(args, cond, err) \
  if (!(cond)) { \
    shval_free(args); \
    return shval_err(err); \
  }

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

shval* shval_qexpr(void) {
  shval* v = malloc(sizeof(shval));
  v->type = SHVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void shval_free(shval* v) {
  switch (v->type) {
  case SHVAL_NUM:
    break;
  case SHVAL_ERR:
    free(v->err);
    break;
  case SHVAL_SYM:
    free(v->sym);
    break;
  case SHVAL_QEXPR:
  case SHVAL_SEXPR:
    for (int i = 0; i < v->count; i++) {
      shval_free(v->cell[i]);
    }
    free(v->cell);
    break;
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
  if (strstr(ast->tag, "qexpr")) {
    v = shval_qexpr();
  }

  for (int i = 0; i < ast->children_num; i++) {
    if (strcmp(ast->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(ast->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(ast->children[i]->contents, "<") == 0) { continue; }
    if (strcmp(ast->children[i]->contents, ">") == 0) { continue; }
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
  case SHVAL_QEXPR:
    shval_expr_print(v, '<', '>');
    break;
  }
}

void shval_println(shval* v) {
  shval_print(v);
  printf("\n");
}

shval* shval_pop(shval* v, int i) {
  shval* a = v->cell[i];

  memmove(&v->cell[i], &v->cell[i + 1], sizeof(shval*) * (v->count - i -1));

  v->count--;
  v->cell = realloc(v->cell, sizeof(shval*) * v->count);

  return a;
}

shval* shval_take(shval* v, int i) {
  shval* a = shval_pop(v, i);
  shval_free(v);
  return a;
}

shval* builtin_head(shval* v) {
  shassert(v,
           v->count == 1,
           "Funtion 'head' passed too many arguments!");
  shassert(v,
           v->cell[0]->type == SHVAL_QEXPR,
           "Funtion 'head' passed incorrect type!");
  shassert(v,
           v->cell[0]->count > 0,
           "Funtion 'head' passed empty expression <>!");

  shval* a = shval_take(v, 0);
  while (a->count > 1) {
    shval_free(shval_pop(a, 1));
  }

  return a;
}

shval* builtin_tail(shval* v) {
  shassert(v,
           v->count == 1,
           "Funtion 'tail' passed too many arguments!");
  shassert(v,
           v->cell[0]->type == SHVAL_QEXPR,
           "Funtion 'tail' passed incorrect type!");
  shassert(v,
           v->cell[0]->count > 0,
           "Funtion 'tail' passed empty expression <>!");

  shval* a = shval_take(v, 0);
  shval_free(shval_pop(a, 0));

  return a;
}

shval* builtin_op(shval* v, char* op) {
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type != SHVAL_NUM) {
      shval_free(v);
      return shval_err("Cannot operate on non-number");
    }
  }

  // get first element
  shval* a = shval_pop(v, 0);

  // if one element only and - operator negate
  if ((strcmp(op, "-") == 0) && v->count == 0) {
    a->num = -a->num;
  }

  while (v->count > 0) {
    // get next element
    shval* x = shval_pop(v, 0);

    if (strcmp(op, "+") == 0) {
      a->num += x->num;
    }
    if (strcmp(op, "-") == 0) {
      a->num -= x->num;
    }
    if (strcmp(op, "*") == 0) {
      a->num *= x->num;
    }
    if (strcmp(op, "/") == 0) {
      if (x->num == 0) {
        // free allocated values
        shval_free(a);
        shval_free(x);
        a = shval_err("Division by zero!");
        break;
      }
      a->num /= x->num;
    }

    shval_free(x);
  }

  shval_free(v);
  return a;
}

shval* shval_eval(shval* v) {
  if (v->type == SHVAL_SEXPR) {
    return shval_eval_sexpr(v);
  }

  return v;
}

shval* shval_eval_sexpr(shval* v) {
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = shval_eval(v->cell[i]);
  }

  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == SHVAL_ERR) {
      return shval_take(v, i);
    }
  }

  if (v->count == 1) {
    return shval_take(v, 0);
  }

  shval* a = shval_pop(v, 0);
  if (a->type != SHVAL_SYM) {
    shval_free(a);
    return shval_err("S-Expression does not start with a symbol!");
  }

  shval* res = builtin(v, a->sym);

  shval_free(a);

  return res;
}

shval* builtin_list(shval* v) {
  v->type = SHVAL_QEXPR;
  return v;
}

shval* builtin_eval(shval* v) {
  shassert(v,
           v->count == 1,
           "Funtion 'eval' passed too many arguments!");
  shassert(v,
           v->cell[0]->type == SHVAL_QEXPR,
           "Funtion 'eval' passed incorrect type!");

  shval* a = shval_take(v, 0);
  a->type = SHVAL_SEXPR;
  return shval_eval(a);
}

shval* builtin(shval* v, char* op) {
  if (strcmp("list", op) == 0) {
    return builtin_list(v);
  }
  if (strcmp("head", op) == 0) {
    return builtin_head(v);
  }
  if (strcmp("tail", op) == 0) {
    return builtin_tail(v);
  }
  if (strcmp("eval", op) == 0) {
    return builtin_eval(v);
  }
  if (strstr("+-*/", op)) {
    return builtin_op(v, op);
  }

  shval_free(v);

  return shval_err("Unknown operation!");
}

int main() {
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Symbol = mpc_new("symbol");
  mpc_parser_t *Sexpr = mpc_new("sexpr");
  mpc_parser_t *Qexpr = mpc_new("qexpr");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *IOSh = mpc_new("iosh");

  mpca_lang(MPCA_LANG_DEFAULT,
           " \
             number    : /-?[0-9]+/ ;                   \
             symbol    : \"exit\" |                     \
                         \"eval\" |                     \
                         \"list\" |                     \
                         \"join\" |                     \
                         \"head\" |                     \
                         \"tail\" |                     \
                         '+' | '-' | '*' | '/' ;        \
             sexpr     : '(' <expr>* ')' ;              \
             qexpr     : '<' <expr>* '>' ;              \
             expr      : <number> |                     \
                         <symbol> |                     \
                         <sexpr> |                      \
                         <qexpr> ;                      \
             iosh      : /^/ <expr>* /$/ ;              \
           ",
           Number,
           Symbol,
           Sexpr,
           Qexpr,
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
      shval* t = shval_read(res.output);
      puts("Debug: ");
      shval_println(t);
      shval* result = shval_eval(t);
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
