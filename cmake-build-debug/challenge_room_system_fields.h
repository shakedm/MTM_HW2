#include "visitor_room.h"

#ifndef CHALLENGE_ROOM_SYSTEM_FIELDS_H_
#define CHALLENGE_ROOM_SYSTEM_FIELDS_H_

#include "system_additional_types.h"


char *name;
Challenge **challenges;
int challenge_array_size;
int room_array_size;
ChallengeRoom **rooms;
int time_log;
VisitorList *first_visitor;


#endif // _H_