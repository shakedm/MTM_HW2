#include "visitor_room.h"

#ifndef SYSTEM_ADDITIONAL_TYPES_H_
#define SYSTEM_ADDITIONAL_TYPES_H_

typedef struct SVisitorList {
    Visitor *visitor;
    struct SVisitorList* next_visitor;
    struct SVisitorList* previous_visitor;
} *VisitorList;

#endif  //SYSTEM_ADDITIONAL_TYPES
