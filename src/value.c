#include "value.h"
#include <stdlib.h>

bool cons_dec_ref(struct cons_cell *cell) {
  if (cell->ref_count-- > 1) {
    return false;
  }

  value_dec_ref(&cell->lhs);
  value_dec_ref(&cell->rhs);
  free(cell);

  return true;
}

void cons_inc_ref(struct cons_cell *cell) { cell->ref_count++; }

void value_dec_ref(struct value *value) {
  switch (value->type) {
  case vt_cons:
    if (cons_dec_ref(value->cell)) {
      value->type = vt_null;
    }
    break;
  default:
    break;
  }
}

void value_inc_ref(struct value *value) {
  switch (value->type) {
  case vt_cons:
    cons_inc_ref(value->cell);
    break;
  default:
    break;
  }
}
void value_set(struct value *dst, struct value src) {
  value_dec_ref(dst);
  *dst = src;
  value_inc_ref(dst);
}

struct value cons(struct value lhs, struct value rhs) {
  struct value result = {
      .type = vt_cons,
  };

  if (!(result.cell = calloc(1, sizeof(*result.cell)))) {
    abort();
  }

  value_set(&result.cell->lhs, lhs);
  value_set(&result.cell->rhs, rhs);
  cons_inc_ref(result.cell);

  return result;
}
