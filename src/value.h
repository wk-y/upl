#ifndef VALUE_H
#define VALUE_H

#include "ast.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

struct interpreter; // todo: reorganize to eliminate forward decl

enum value_type {
  vt_null,
  vt_number,
  vt_string,
  vt_cons,
  vt_cfunc,
  vt_error,
};

struct value_string {
  char *string;
  size_t ref_count;
};

struct value {
  enum value_type type;
  union {
    float number;
    struct value_string *string;
    struct cons_cell *cell;
    struct value (*cfunc)(struct interpreter *, struct ast_node *,
                          struct ast_node *);
  };
};

struct cons_cell {
  struct value lhs, rhs;
  size_t ref_count;
};

// Returns true if the cell was freed.
bool cons_dec_ref(struct cons_cell *cell);
void cons_inc_ref(struct cons_cell *cell);

// Returns true if the string was freed.
bool string_dec_ref(struct value_string *str);
void string_inc_ref(struct value_string *str);

struct value cons(struct value lhs, struct value rhs);

// Handles ref count of contained value
void value_dec_ref(struct value *value);
void value_inc_ref(struct value *value);

// Turn the left value into the right value
void value_set(struct value *dst, struct value src);

void value_print(FILE *, struct value const);

#endif
