#ifndef STACK_H
#define STACK_H

#include "value.h"

struct variables {
  char *name;
  struct value value;
  struct variables *next;
};

struct stack_frame {
  struct variables *variables;
  struct stack_frame *parent;
};

struct stack {
  struct stack_frame *frame;
};

struct value *variables_get_variable(struct variables *, char *name);
void variables_set_variable(struct variables **, char *name,
                            struct value value);

void stack_init(struct stack *);
void stack_deinit(struct stack *);

void stack_push(struct stack *);
void stack_pop(struct stack *);

struct value *stack_get_variable(struct stack *, char *name);
void stack_set_variable(struct stack *, char *name, struct value value);

void stack_frame_deinit(struct stack_frame *);

#endif
