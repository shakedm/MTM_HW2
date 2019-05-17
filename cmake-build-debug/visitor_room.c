#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "visitor_room.h"
#include "challenge_room_system_fields.h"

#include <stdbool.h>

#define NOT_FOUND -1

static int find_challenge_available(ChallengeRoom *room, Level level);
//static function to find the smallest lexicography available room

//functions:
/*  Function initializes a specific challenge activity
 * Error Codes: NULL_PARAMETER if activity or challenge is NULL
 *              MEMORY_PROBLEM if malloc failed*/
Result init_challenge_activity(ChallengeActivity *activity
        ,Challenge *challenge){
    if(activity==NULL || challenge==NULL)
        return NULL_PARAMETER;
    activity->challenge = challenge;
    activity->visitor= NULL;
    return OK;
}


/*  Function resets challenges activity to NULL
 * recieves ChallenegeActivity pointer
 * Error Codes: NULL_PARAMETER if challenge is NULL*/
Result reset_challenge_activity(ChallengeActivity *activity){
    if(activity==NULL)
        return NULL_PARAMETER;
    activity->challenge=NULL;
    activity->visitor=NULL;
    activity->start_time = 0;
    return OK;
}

/*  Function initializes a specific visitor to start parameters.
 * Receives: visitor pointer
 *           int of visitor id
 *           name of visitor
 * Error Codes: NULL_PARAMETER if visitor or name is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.*/
Result init_visitor(Visitor* visitor, char* name, int id){
    //checking if received null parameters
    if(visitor==NULL || name==NULL )
        return NULL_PARAMETER;
    //malloc required in the instruction
    char* copy_name= malloc(sizeof(char)*(strlen(name)+1));
    if (copy_name==NULL)
        return MEMORY_PROBLEM;
    //check for memory fault
    strcpy(copy_name,name);
    //initialize all fields
    visitor->visitor_name=copy_name;
    visitor->visitor_id=id;
    visitor->room_name=NULL;
    visitor->current_challenge=NULL;
    return OK;
}

/*  Function resets a specific visitor to NULL.
 * receives visitor pointer
 * Error Codes: NULL_PARAMETER if visitor or name is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.*/
Result reset_visitor(Visitor *visitor){
    //checking parameter if null
    if(visitor == NULL)
        return NULL_PARAMETER;
    //checking if the pointer is NULL
    if(visitor->visitor_name !=NULL){
        free(visitor->visitor_name);
    }
    //reseting all struct param to NULL (no info lost due to other pointers)
    visitor->current_challenge=NULL;
    visitor->room_name=NULL;
    visitor->visitor_id = 0;
    return OK;
}

/*  Function initializes a specific room to start parameters.
 * receives: ChallengeRoom pointer
 *           name of room as string
 *           number of challenges in the room
 * Error Codes: NULL_PARAMETER if room or name is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.
 *              ILLEGAL_PARAMETER if there are no challenges in room*/
Result init_room(ChallengeRoom *room, char *name, int num_challenges){
    if(room == NULL || name==NULL)
        return NULL_PARAMETER;
    if(num_challenges < 1)
        return ILLEGAL_PARAMETER;
    //checks for input faults
    char* copy_name=malloc(sizeof(char)*(strlen(name)+1));
    if (copy_name==NULL)
        return MEMORY_PROBLEM;
    strcpy(copy_name,name);
    //setting a copy of name
    room->name=copy_name;
    //starting an array of *challenges fields inside not initialize
    ChallengeActivity **challenges =malloc(sizeof(ChallengeActivity*) *
                                                   num_challenges);
    if(challenges==NULL){
        free(copy_name);
        return MEMORY_PROBLEM;
    }
    ChallengeActivity *curr_challenge;
    for( int i=0; i<num_challenges; i++){
        curr_challenge = malloc(sizeof(ChallengeActivity));
        if (!curr_challenge){
            for (int j = 0; j < i; ++j) {
                free (room->challenges[j]);
            }
            free(challenges);
            free(copy_name);
            return MEMORY_PROBLEM;
        }
        challenges[i] = curr_challenge;
    }
    room->challenges=challenges;
    room->num_of_challenges=num_challenges;
    return OK;
}

/*  Function rests a specific room to start parameters.
 * Receives: ChallengeRoom pointer
 * Error Codes: NULL_PARAMETER if room is NULL*/
Result reset_room(ChallengeRoom *room){
    if(room==NULL || room->name==NULL)
        return NULL_PARAMETER;
    //free to release the name that was allocated
    free(room->name);
    room->name=NULL;
    for(int i=0 ; i< room->num_of_challenges ; i++){
        Result result = reset_challenge_activity( room->challenges[i] );
        free(room->challenges[i]);
        if(result==NULL_PARAMETER)
            continue;
    }
    //NOTE: THERE MUST BE AN ARRAY BECAUSE OF THE INIT_ROOM FUNCTION
    free(room->challenges);
    room->challenges = NULL;
    room->num_of_challenges = 0;
    return OK;
}

/*Function returns the number of free places in the room in the specified level
 * Receives:
 *          Challenge room pointer
 *          level requested
 *          pointer to int, to return the number found
 * Error Codes: NULL_PARAMETER if room is NULL*/
Result num_of_free_places_for_level(ChallengeRoom *room, Level level,
                                    int* places){
    if (room== NULL)
        return NULL_PARAMETER;
    int sum=0;
    // if its all levels, find the number of free challenges
    if (level==All_Levels){
        for (int i = 0; i <room->num_of_challenges ; ++i) {
            if (room->challenges[i]->visitor == NULL)
                sum++;
        }
        *places=sum;
        return OK;
    }//if a specific level add a condition that the challenge level is the same
    else{
        for (int j = 0; j <room->num_of_challenges ; ++j) {
            if (room->challenges[j]->visitor==NULL &&
                    room->challenges[j]->challenge->level==level)
                sum++;
        }
        *places=sum;
        return OK;
    }
}

/*  Function changes a specific room's name to given parameter.
 * Receives: ChallengeRoom pointer
 *           new name as a string
 * Error Codes: NULL_PARAMETER if room or name is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.*/
Result change_room_name(ChallengeRoom *room, char* new_name){
    if(room==NULL || new_name==NULL)
        return NULL_PARAMETER;
    // allocating memory for the new name
    char* copy_name=malloc(sizeof(char)*(strlen(new_name)+1));
    if (copy_name==NULL)
        return MEMORY_PROBLEM;
    //copying and redirecting the room->name to new name
    strcpy(copy_name,new_name);
    if(room->name!=NULL)
        free(room->name);
    //to prevent memory leak
    room->name=copy_name;
    return OK;
}

/*  Function finds the room a specific visitor is in.
 * Receives: visitor pointer
 *           the room name requested as a pointer to string
 *
 * Error Codes: NULL_PARAMETER if visitor is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.
 *              NOT_IN_ROOM  the room name in type visitor is not initialized*/
//for this function must free the pointer that it returns!
Result room_of_visitor(Visitor *visitor, char** room_name){
    if(visitor==NULL)
        return NULL_PARAMETER;
    if(visitor->room_name==NULL)
        return NOT_IN_ROOM;
    char* copy_room_name= malloc((sizeof(char)*strlen(*visitor->room_name))+1);
    if(copy_room_name==NULL)
        return MEMORY_PROBLEM;
    strcpy(copy_room_name,(*visitor->room_name));
    *room_name=copy_room_name;
    return OK;
}

/*  Function finds the smallest lexicographic name of a challenge
 * that's free and puts visitor in it. updates fields in challenge & visitor.
 * Receives: ChallengeRoom pointer
 *           visitor pointer
 *           level requested
 *           the start time as int
 * Error Codes: NULL_PARAMETER if visitor or room is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.
 *              ALREADY_IN_ROOM if the visitor is in a room.
 *              NO_AVAILABLE_CHALLENGES if the room is full*/
Result visitor_enter_room(ChallengeRoom *room, Visitor *visitor,Level level,
                          int start_time) {
    if (room == NULL || visitor == NULL)
        return NULL_PARAMETER;
    if (visitor->room_name != NULL)
        return ALREADY_IN_ROOM;
    int available_challenges = find_challenge_available(room, level);
    //-1 if no challenge found
    if(available_challenges==NOT_FOUND)
        return NO_AVAILABLE_CHALLENGES;
    else{
        room->challenges[available_challenges]->visitor=visitor;
        room->challenges[available_challenges]->start_time=start_time;
        inc_num_visits(room->challenges[available_challenges]->challenge);
        visitor->current_challenge=room->challenges[available_challenges];
        visitor->room_name= &(room->name);
        return OK;
    }
}

/*  Function updates the best time (if needed) and resets related room fields in
 * visitor type.
 * Receives: visitor pointer
 *           quit time as int
 * Error Codes: NULL_PARAMETER if visitor is NULL
 *              NOT_IN_ROOM if the visitor is not in any room*/
Result visitor_quit_room(Visitor *visitor, int quit_time){
    if(visitor==NULL)
        return NULL_PARAMETER;
    if(visitor->room_name==NULL)
        return NOT_IN_ROOM;
    //local parameter time_of_challenge holds the difference between quit time
    //and start time
    int time_of_challenge= quit_time-(visitor->current_challenge->start_time);
    if( time_of_challenge < visitor->current_challenge->challenge->best_time ||
            visitor->current_challenge->challenge->best_time == 0)
        set_best_time_of_challenge(visitor->current_challenge->challenge,
                                   time_of_challenge);
    // if its better than best time update best time
    visitor->current_challenge->visitor=NULL;
    visitor->current_challenge=NULL;
    visitor->room_name= NULL;
    //reset all fields of visitor
    return OK;
}

/*  Function finds an available challenge to given parameters
 * Receives: ChallengeRoom pointer
 *           level to find
 * Error Codes: NOT FOUND there are no available challenges of the cratiria*/
static int find_challenge_available(ChallengeRoom *room, Level level) {
    int current = NOT_FOUND;
    num_of_free_places_for_level(room, level, &current);
    if (current== 0)
        return NOT_FOUND;
    current=NOT_FOUND;
    bool all_levels = false;
    if (level == All_Levels)
        all_levels = true;
    for (int i = 0; i < room->num_of_challenges; ++i) {
        if ((room->challenges[i])->visitor == NULL) {
            if (all_levels) {
                if (current == NOT_FOUND) {
                    if (room->challenges[i]->visitor == NULL)
                        current = i;
                }
                else {
                    if (strcmp(room->challenges[i]->challenge->name,
                               room->challenges[current]->challenge->name) < 0)
                        if (room->challenges[i]->visitor == NULL)
                            current = i;
                }
            }
            else {
                if (current == NOT_FOUND) {
                    if ((room->challenges[i])->visitor == NULL &&
                        level == room->challenges[i]->challenge->level)
                        current = i;
                }
                else {
                    if ((room->challenges[i])->challenge->level== level &&
                        strcmp(room->challenges[i]->challenge->name,
                               room->challenges[current]->challenge->name) < 0)
                        if (room->challenges[i]->visitor != NULL)
                            current = i;
                }
            }
        }
    }
    return current;
}