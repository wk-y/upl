#ifndef EVAL_H
#define EVAL_H

#include "ast.h"
#include "value.h"

struct value eval(struct ast_node *);

#endif
