/*
 * Austin Henley
 * A challege to write a Python in 1024 bytes of C.
 * https://austinhenley.com/blog/python1024.html
 */

#include <stdio.h>

char src[999];  /* Entire program without most spaces. */
int vars[256];  /* Symbol table. */
int pos;        /* Next character in src. */
int ch;         /* Current character in src. */
int line_start; /* Where the current line starts. */

/* Read one character. */
int next(void) { return ch = src[pos++]; }

/* Process indents. */
int read_indent(void) {
  line_start = pos;
  while (next() == ' ')
    ;
  return pos - line_start;
}

/* Eat a line. */
void skip_to_eol(void) {
  if (ch != 0 && ch != '\n') {
    next();
    skip_to_eol();
  }
}

/* An integer literal a variable. */
int parse_atom(void) {
  int value = 0;
  next();
  if (ch > 96) {
    value = vars[ch];
    next();
  }
  while (ch >= '0' && ch <= '9') {
    value = value * 10 + ch - '0';
    next();
  }
  return value;
}

/* atom { ('*' | '%') atom } */
int parse_term(void) {
  int value = parse_atom();
  while (ch == '*' || ch == '%') {
    int op = ch;
    if (op == '*')
      value = value * parse_atom();
    else
      value = value % parse_atom();
  }
  return value;
}

/* ['+' | '-'] term { ('+' | '-') term } */
int parse_sum(void) {
  int value = parse_term();
  while (ch == '+' || ch == '-') {
    if (ch == '+')
      value = value + parse_term();
    else
      value = value - parse_term();
  }
  return value;
}

/* sum [ ('<' | '>' | '<=' | '>=' | '==') sum ] */
int parse_expr(void) {
  int left = parse_sum();

  if (ch != '<' && ch != '=' && ch != '>')
    return left;

  int op = ch;
  int has_eq = (next() == '=');
  if (!has_eq)
    pos--;

  int right = parse_sum();

  if (op == '<')
    return has_eq ? left <= right : left < right;
  if (op == '>')
    return has_eq ? left >= right : left > right;
  return left == right;
}

void run_block(int min_indent);

/* Skip lines that belong to a block */
void skip_block(int indent) {
  while (read_indent() > indent || ch == '\n')
    skip_to_eol();
  pos = line_start;
}

/* Print a string literal. */
void print_string(void) {
  next();
  while (next() != '"')
    putchar(ch);
  next();
}

/* Executes a block until the indent is lesser. */
void run_block(int min_indent) {
  for (;;) {
    int indent = read_indent();

    if (ch == '\n')
      continue;

    if (indent < min_indent || ch == 0) {
      pos = line_start;
      return;
    }

    if (ch == 'w' || ch == 'i' || ch == 'f') {
      int keyword = ch;
      int loop_var = 0;

      if (keyword == 'f') { /* Assumes "for x in range(y):" */
        pos += 2;           /* Skip "or". */
        loop_var = next();
        pos += 8; /* Skip "inrange(". */
        vars[loop_var] = 0;
      } else if (keyword == 'w') {
        pos += 4;
      } else {
        pos += 1;
      }

      int cond_pos = pos;
      int cond;

      for (;;) {
        if (keyword == 'f') {
          cond = vars[loop_var] < parse_expr();
          pos++;
        } else {
          cond = parse_expr();
        }
        next();

        if (!cond) {
          skip_block(indent);
          break;
        }

        run_block(indent + 1);

        if (keyword == 'i')
          break;
        if (keyword == 'f')
          vars[loop_var]++;
        pos = cond_pos;
      }

      if (read_indent() == indent && ch == 'e') {
        pos += 4;
        next();
        if (cond)
          skip_block(indent);
        else
          run_block(indent + 1);
      } else {
        pos = line_start;
      }
    } else if (ch == 'd') { /* Assumes "def x():". */
      pos += 2;
      int name = next();
      skip_to_eol();
      vars[name] = pos;
      skip_block(indent);
    } else { /* Assumes either an assignment, print, or function call. */
      if (ch > 96) {
        int name = ch;
        while (next() > 96)
          ;
        if (ch == '(') {
          if (name == 'p') {
            if (src[pos] == '"')
              print_string();
            else
              printf("%d", parse_expr());
            puts("");
            next();
          } else {
            int saved;
            next();
            saved = pos;
            pos = vars[name];
            run_block(2);
            pos = saved;
            next();
          }
        } else {
          vars[name] = parse_expr();
        }
      }
      skip_to_eol();
    }
  }
}

/* Read from stdin and strip most whitespace. */
int main(void) {
  int len = 0;
  int past_margin = 0;
  int in_string = 0;

  int c;
  while ((c = getchar()) != EOF) {
    if (c == '\t')
      c = ' ';
    if (c == '"')
      in_string = !in_string;

    if (c != ' ' || !past_margin || in_string)
      src[len++] = c;

    if (c == '\n')
      past_margin = 0;
    else if (c > ' ')
      past_margin = 1;
  }

  ch = 0;
  pos = 0;
  run_block(0);
  return 0;
}
