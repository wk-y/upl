#include "stack.h"
#include "value.h"
#include <stdlib.h>
#include <string.h>

struct value *variables_get_variable(struct variables *variables, char *name) {
  for (struct variables *v = variables; v; v = v->next) {
    if (!strcmp(v->name, name)) {
      return &v->value;
    }
  }
  return NULL;
}

void variables_set_variable(struct variables **variables, char *name,
                            struct value value) {
  struct value *var = variables_get_variable(*variables, name);
  if (!var) {
    struct variables *new_variable = malloc(sizeof(*new_variable));
    if (!new_variable) {
      abort();
    }

    new_variable->name = malloc(strlen(name) + 1);
    if (!new_variable->name) {
      abort();
    }
    strcpy(new_variable->name, name);
    new_variable->value.type = vt_null;
    new_variable->next = *variables;
    *variables = new_variable;
    var = &new_variable->value;
  }
  value_set(var, value);
}

static void variables_deinit(struct variables **variables) {
  while (*variables) {
    struct variables *v = *variables;
    *variables = (*variables)->next;
    free(v->name);
    value_dec_ref(&v->value);
    free(v);
  }
}

void stack_deinit(struct stack *s) {
  while (s->frame) {
    struct stack_frame *parent = s->frame->parent;
    variables_deinit(&s->frame->variables);
    free(s->frame);
    s->frame = parent;
  }
}

void stack_frame_deinit(struct stack_frame *s) {
  variables_deinit(&s->variables);
}

void stack_push(struct stack *s) {
  struct stack_frame *parent = s->frame;
  if (!(s->frame = malloc(sizeof(*s->frame)))) {
    abort();
  }
  s->frame->parent = parent;
  s->frame->variables = NULL;
}

void stack_pop(struct stack *s) {
  struct stack_frame *parent = s->frame->parent;
  s->frame->parent = NULL;
  stack_frame_deinit(s->frame);
  free(s->frame);
  s->frame = parent;
}

void stack_init(struct stack *s) {
  s->frame = NULL;
  stack_push(s);
}

struct value *stack_get_variable(struct stack *s, char *name) {
  for (struct stack_frame *frame = s->frame; frame; frame = frame->parent) {
    struct value *variable = variables_get_variable(frame->variables, name);
    if (variable) {
      return variable;
    }
  }
  return NULL;
}

void stack_set_variable(struct stack *s, char *name, struct value value) {
  variables_set_variable(&s->frame->variables, name, value);
}
