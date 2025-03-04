#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>
#include <stddef.h>

enum value_type {
  vt_null,
  vt_number,
  vt_literal,
  vt_string,
  vt_cons,
};

struct value {
  enum value_type type;
  union {
    float number;
    struct cons_cell *cell;
  };
};

struct cons_cell {
  struct value lhs, rhs;
  size_t ref_count;
};

// Returns true if the cell was freed.
bool cons_dec_ref(struct cons_cell *cell);
void cons_inc_ref(struct cons_cell *cell);
struct value cons(struct value lhs, struct value rhs);

// Handles ref count of contained value
void value_dec_ref(struct value *value);
void value_inc_ref(struct value *value);

// Turn the left value into the right value
void value_set(struct value *dst, struct value src);

#endif
