#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>


#include "challenge_system.h"

//Defines:
#define MAX_LINE_LENGTH 51
#define DUMMY "dummy_name"
#define DUMMY_ID -1
#define START_VALUE -2

#define SYSTEM_HANDEL(error_code, result)  \
   if (!(result)) {free (dummy); \
                    free(name_copy); \
                    free(*sys); \
                    fclose (system_file); \
                    return error_code;} \


//Static functions list:
static Result challenge_read(FILE* file, char* name, int* num_of_challenges,
                             ChallengeRoomSystem **sys);
static Result room_read(FILE* file, char* name, ChallengeRoomSystem **sys,
                        int challenges_arr_size, int* room_arr_size);
static Result set_challenges_in_array (FILE* file, int num_of_challenges,
                                       ChallengeRoomSystem **sys,
                                       ChallengeRoom* room,
                                       int challenge_arr_size);
static void free_allocated(void** array, int finish);
static Result find_room(ChallengeRoomSystem *sys,char* room_name,
                        ChallengeRoom** room);
static Result add_to_list(ChallengeRoomSystem *sys,Visitor *visitor);
static  Result remove_from_list(ChallengeRoomSystem *sys, Visitor *visitor);
static Result find_visitor_in_list(ChallengeRoomSystem *sys, Visitor *visitor,
                                   VisitorList** pointer);
static Result find_visitor_by_id(ChallengeRoomSystem *sys, int visitor_id,
                                 Visitor** visitor);
static Result find_challenge(ChallengeRoomSystem *sys, char* challenge_name,
                             Challenge** ptr);
static Result create_list(Visitor *visitor, ChallengeRoomSystem *sys);
static Result free_challenges(ChallengeRoomSystem *sys, char **best_time);
static void free_challenges_memory(ChallengeRoomSystem *sys);


//.h functions:
/*  Function initializes the system
 * Receives: **sys - return pointer. via it user gains access to initiated system.
 *          init file - string. the name of the file with the init information.
 * Error Codes: NULL_PARAMETER if system is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.
 *              ILLEGAL_PARAMETER if the file is incorrect*/
Result create_system(char *init_file, ChallengeRoomSystem **sys){
    if(sys == NULL)
        return NULL_PARAMETER;
    FILE* system_file = fopen(init_file, "r");
    if (!system_file){
        printf("Error: cannot open system initiation file.\n");
        return ILLEGAL_PARAMETER;
    }
    *sys = malloc(sizeof(ChallengeRoomSystem));
    if (!(*sys))
        return MEMORY_PROBLEM;
    char name[MAX_LINE_LENGTH];
    Result result;
    Visitor* dummy = NULL;
    fscanf(system_file, "%s ", name);
    int num_of_challenges = START_VALUE, num_of_rooms = START_VALUE;
    char *name_copy = malloc((strlen(name)+1)* sizeof(char));
    SYSTEM_HANDEL(MEMORY_PROBLEM, name_copy);
    dummy = malloc(sizeof( Visitor ));
    SYSTEM_HANDEL(MEMORY_PROBLEM, dummy);
    strcpy(name_copy, name);
    (*sys)->name =  name_copy;
    result = challenge_read(system_file, name, &num_of_challenges, sys);
    SYSTEM_HANDEL(result , result == OK);
    (*sys)->challenge_array_size = num_of_challenges;
    result = room_read(system_file, name, sys,
                       num_of_challenges, &num_of_rooms);
    SYSTEM_HANDEL(result , result == OK);
    (*sys)->room_array_size = num_of_rooms;
    (*sys)->time_log = 0;
    (*sys)->first_visitor = malloc(sizeof(VisitorList));
    result = init_visitor(dummy, DUMMY, DUMMY_ID);
    SYSTEM_HANDEL(result , result == OK);
    result = create_list(dummy, *sys);
    SYSTEM_HANDEL(result , result == OK);
    fclose(system_file);
    return OK;
}

/*  Function deletes the system. frees all allocated space.
 * Receives: *sys - points to the relevent system to destroy.
 *          destroy time - the time to log as the finish system time.
 *          **most_popular_challenge_p - return value give the name of the
 *                                  challenge that had the lragest amount of
 *                                  visitors by the destroy time.
 *          **challenge_best_time - return value gives the challenge with the
 *                                  best recorded finish time.
 * Error Codes: NULL_PARAMETER if challenge or name is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.
 *              ILLEGAL_TIME the given time is lesser than current system time*/
Result destroy_system(ChallengeRoomSystem *sys, int destroy_time,
                      char **most_popular_challenge_p,
                      char **challenge_best_time){
    if (!sys)
        return NULL_PARAMETER;
    if(destroy_time < sys->time_log)
        return ILLEGAL_TIME;
    Result result = all_visitors_quit(sys, destroy_time);
    if (result != OK)
        return result;
    free(sys->first_visitor);
    for (int i = 0; i < sys->room_array_size ; ++i) {
        result = reset_room(sys->rooms[i]);
        assert(result == OK); //already check that room != NULL.
        free((sys->rooms[i]));
    }
    free(sys->rooms); // finished releasing all room related memory.
    result = most_popular_challenge(sys, most_popular_challenge_p);
    if (result != OK)
        return result;
    result = free_challenges(sys, challenge_best_time);
    if (result != OK)
        return result;
    free(sys->name);
    free(sys);
    return OK;
}

/*  Function updates every field when a visitor enter the system
 * Receives: *sys - the relevant system to enter visitor in.
 *          room name - the room in which the visitor wishes to be in.
 *          visitor name, visitor id, level - paramters to initiate visitor in
 *                                          the system.
 *          strart time - the time to log in the system.
 * Error Codes: NULL_PARAMETER if sys is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.
 *              ILLEGAL_TIME the given time is lesser than current system time
 *              ILLEGAL_PARAMETER if the names are NULL or not found*/
Result visitor_arrive(ChallengeRoomSystem* sys, char* room_name,
                      char* visitor_name, int visitor_id, Level level,
                      int start_time){
    if(sys==NULL)
        return NULL_PARAMETER;
    if( start_time< sys->time_log)
        return ILLEGAL_TIME;
    if(room_name==NULL || visitor_name==NULL )
        return ILLEGAL_PARAMETER;
    Visitor *visitor= malloc(sizeof(*visitor));
    if(visitor==NULL)
        return MEMORY_PROBLEM;
    Result result= init_visitor(visitor,visitor_name,visitor_id);
    if (result!=OK){
        free(visitor);
        return result;
    }
    ChallengeRoom *room;
    result=find_room(sys,room_name,&room);
    if(result!=OK){
        reset_visitor(visitor);
        free(visitor);
        return result;
    }
    result=add_to_list(sys, visitor);
    if(result!= OK){
        reset_visitor(visitor);
        free(visitor);
        return result;
    }
    result=visitor_enter_room(room , visitor , level , start_time);
    if (result!=OK){
        remove_from_list(sys, visitor);
        reset_visitor(visitor);
        free(visitor);
        return result;
    }
    sys->time_log= start_time; //update system time
    return OK;

}

/*  Function removes the given visitor from linked list. updates relevant system
 *          parameters.
 * Receives: system type pointer - to gain access to the relevant system list.
 *          visitor id - to identify the specific visitor
 *          quit time - to log into the system time log.
 * Error Codes: NULL_PARAMETER if sys is NULL
 *              ILLEGAL_TIME the given time is lesser than current system time*/
Result visitor_quit(ChallengeRoomSystem *sys, int visitor_id, int quit_time){
    if(sys==NULL)
        return NULL_PARAMETER;
    if (quit_time< sys->time_log)
        return ILLEGAL_TIME;
    Visitor *ptr_visitor = NULL;
    Result res=find_visitor_by_id(sys,visitor_id,&ptr_visitor);
    if (res!=OK ){
        return res;
    }
    assert(ptr_visitor!=NULL);
    res=remove_from_list(sys,ptr_visitor);
    if (res!=OK)
        return res;
    sys->time_log= quit_time;
    visitor_quit_room(ptr_visitor,quit_time);
    reset_visitor(ptr_visitor);
    free(ptr_visitor);
    return OK;

}

/*  Function removes all the entities from the visitor list in a given system.
 *           logs in the action time into the system time log.
 * Receives: system type pointer - to gain access to the relevant system list.
 *          quit time - to log into the system time log.
 * Error Codes: NULL_PARAMETER if sys is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.
 *              ILLEGAL_TIME the given time is lesser than current system time*/
Result all_visitors_quit(ChallengeRoomSystem *sys, int quit_time){
    if(sys == NULL){
        return NULL_PARAMETER;
    }
    if (quit_time < sys->time_log){
        return ILLEGAL_TIME;
    }
    Result result = OK;
    VisitorList *curr_visitor= NULL;
    if ((*sys->first_visitor) != NULL ){
        curr_visitor = sys->first_visitor;
    }
    while (curr_visitor){
        result = visitor_quit(sys,
                              (*curr_visitor)->visitor->visitor_id, quit_time);
        //removes visitor entity from list
        if (result != OK)
            return result;
        if ((*sys->first_visitor) != NULL){
            curr_visitor = sys->first_visitor;
        }
        else {
            break;
        }
    }
    sys->time_log = quit_time;
    return OK;
}

/*  Function finds a given visitor in the system, and returns the visitor's current
 *           room.
 * Receives: system type pointer - to gain access to the relevant system to search.
 *          visitor name - to identify the specific visitor
 *          **room_name - return value is the visitor's room.
 * Error Codes: NULL_PARAMETER if sys is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.
 *              NOT_IN_ROOM if the visitor isn't in the system*/
Result system_room_of_visitor(ChallengeRoomSystem *sys, char *visitor_name,
                              char **room_name) {
    if (sys == NULL)
        return NULL_PARAMETER;
    if (!visitor_name || !room_name)
        return ILLEGAL_PARAMETER;
    VisitorList *visitor_ptr = NULL;
    visitor_ptr = sys->first_visitor;
    bool found = false;
    Result res = OK;
    while (visitor_ptr) {
        if (strcmp((*visitor_ptr)->visitor->visitor_name, visitor_name) == 0) {
            assert(sys && visitor_name);
            res = room_of_visitor((*visitor_ptr)->visitor, room_name);
            found = true;
        }
        if((*visitor_ptr)->next_visitor != NULL){
            visitor_ptr = &((*visitor_ptr)->next_visitor);
        }
        else{
            break;
        }
    }
    if (found)
        return res;
    return NOT_IN_ROOM;
}

/*  Function changes a given system challenge's name.
 * Receives: system type pointer - to gain access to the relevant system list.
 *          challenge id - to identify the specific challenge in the system.
 *          new name - once the challenge is found, it's name is changed .
 * Error Codes: NULL_PARAMETER if sys is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.
 *              ILLEGAL_TIME the given time is lesser than current system time*/
Result change_challenge_name(ChallengeRoomSystem *sys, int challenge_id, char *new_name){
    if (!sys || !new_name)
        return NULL_PARAMETER;
    bool found = false;
    Result res = OK;
    for (int i = 0; (i < sys->challenge_array_size && !found) ; ++i) {
        if(sys->challenges[i]->id == challenge_id){
            found = true;
            res = change_name(sys->challenges[i], new_name);
        }
    }
    if (!found){
        return ILLEGAL_PARAMETER; //could not find challenge ID in system.
    }
    return res;
}

Result change_system_room_name(ChallengeRoomSystem *sys, char *current_name, char *new_name) {
    if (sys==NULL || current_name==NULL || new_name== NULL)
        return NULL_PARAMETER;
    ChallengeRoom* room;
    Result result=find_room(sys,current_name,&room);
    if(result!=OK)
        return result;
    result=change_room_name(room,new_name);
    if (result!=OK){
        return result;
    }
    return OK;
}

Result best_time_of_system_challenge(ChallengeRoomSystem *sys, char *challenge_name, int *time){
    if(sys ==NULL || challenge_name==NULL)
        return NULL_PARAMETER;
    Challenge *ptr=NULL;
    Result result= find_challenge(sys,challenge_name,&ptr);
    if (result!= OK)
        return result;
    assert(ptr!=NULL);
    best_time_of_challenge(ptr,time);
    return OK;

}

Result most_popular_challenge(ChallengeRoomSystem *sys, char **challenge_name){
    if(!sys)
        return NULL_PARAMETER;
    if(sys->challenge_array_size == 0){
        *challenge_name = NULL;
        return OK;
    }
    int max_visitor_for_challenge, curr_challenge_visits_num, sum_of_visits=0;
    Result res = num_visits(sys->challenges[0], &max_visitor_for_challenge);
    assert(res == OK);
    char* challenge_name_ptr = sys->challenges[0]->name;
    for (int i = 1; i <sys->challenge_array_size ; ++i) {
        res = num_visits(sys->challenges[i], &curr_challenge_visits_num);
        assert(res == OK);
        sum_of_visits+=curr_challenge_visits_num;
        if(curr_challenge_visits_num >= max_visitor_for_challenge){
            if (curr_challenge_visits_num == max_visitor_for_challenge){
                if (strcmp(sys->challenges[i]->name, challenge_name_ptr) < 0)
                    challenge_name_ptr = sys->challenges[i]->name;
            }
            else
                challenge_name_ptr = sys->challenges[i]->name;
            }
        }
    if (!sum_of_visits){
        *challenge_name=NULL;
        return OK;
    }
    char* temp = malloc((strlen(challenge_name_ptr)+1)* sizeof(char));
    if(!temp)
        return MEMORY_PROBLEM;
    strcpy(temp, challenge_name_ptr);
    *challenge_name = temp;
    return OK;
}

//static functions

/*  Function reads from initiation file the parameters for the system challenge
 * array and sets it.
 * Error Codes: MEMORY_PROBLEM if system was unable to allocate memory.*/
static Result challenge_read(FILE* file, char* name, int* num_of_challenges,
                             ChallengeRoomSystem **sys){
    int challenge_id, level;
    fscanf(file, "%d ", num_of_challenges);
    Result result;
    Challenge** challenge_array = malloc((*num_of_challenges)*
                                                 sizeof(Challenge*));
    if (challenge_array == NULL)
        return MEMORY_PROBLEM; //unable to allocate array size.
    for (int i=0; i<(*num_of_challenges); i++){
        fscanf(file, "%s %d %d ", name, &challenge_id, &level);
        Level resolved = Easy;
        switch (level){
            case 1: resolved = Easy;
                break;
            case 2: resolved = Medium;
                break;
            case 3: resolved = Hard;
                break;
            default: resolved = All_Levels;
                break;
        }
        Challenge *curr_challenge = malloc(sizeof(Challenge));
        if (curr_challenge == NULL) {
            free_allocated((void**)challenge_array, i);
            return MEMORY_PROBLEM;
        }
        challenge_array[i] = curr_challenge;
        result = init_challenge(curr_challenge, challenge_id, name, resolved);
        if (result != OK) {
            free_allocated((void **) challenge_array, i);
            free(curr_challenge);
            return result;
        }
    }
    (*sys)->challenges = challenge_array;
    return OK;
}

/*  Function reads from initiation file the parameters for the system room array
 * and sets it.
 * Error Codes: MEMORY_PROBLEM if system was unable to allocate memory.*/
static Result room_read(FILE* file, char* name, ChallengeRoomSystem **sys,
                        int challenges_arr_size, int* room_arr_size){
    int num_of_rooms, num_of_challenges;
    fscanf(file, "%d", &num_of_rooms);
    *room_arr_size = num_of_rooms;
    Result result;
    ChallengeRoom** room_array = malloc(sizeof(ChallengeRoom*)*num_of_rooms);
    if (room_array == NULL)
        return MEMORY_PROBLEM;
    ChallengeRoom *curr_room = NULL;
    for (int i=0; i<num_of_rooms; i++){
        curr_room = malloc(sizeof(ChallengeRoom));
        if (curr_room == NULL) {
            free_allocated((void**)room_array, i);
            free_challenges_memory((*sys));
            return MEMORY_PROBLEM;
        }
        fscanf(file, "%s %d ", name, &num_of_challenges);
        room_array[i] = curr_room;
        result = init_room(room_array[i], name, num_of_challenges);
        if (result != OK) {
            free_allocated((void**)room_array, i);
            free(curr_room);
            free_challenges_memory((*sys));
            return result;
        }
        result = set_challenges_in_array(file, num_of_challenges, sys,
                                         room_array[i],challenges_arr_size);
        if (result != OK) {
            free_allocated((void**)room_array, i);
            free(curr_room);
            free_challenges_memory((*sys));
            return result;
        }
    }
    (*sys)->rooms = room_array;
    return OK;
}

/*  Function reads from initiation file the parameters for the activity array
 * and sets it.
 * Error Codes: ILLEGAL_PARAMETER if challenge ID is not int the system.*/
static Result set_challenges_in_array (FILE* file, int num_of_challenges,
                                       ChallengeRoomSystem **sys,
                                       ChallengeRoom* room,
                                       int challenge_arr_size) {
    int curr_id;
    bool found_challenge = false;
    for (int i = 0; i < num_of_challenges; ++i) {
        fscanf(file, "%d ", &curr_id);
        for (int j = 0; j < challenge_arr_size; ++j) {
            if ((*sys)->challenges[j]->id == curr_id){
                Result result = init_challenge_activity(room->challenges[i],
                                                        (*sys)->challenges[j]);
                if (result != OK)
                    return result;
                found_challenge = true;
            }
        }
        if (!found_challenge){
            return ILLEGAL_PARAMETER; //challenge not in system.
        }
        found_challenge = false;
    }
    return OK;
}

/*  Function receives an array and size. releases allocated space.*/
static void free_allocated(void** array, int finish){
    for (int j=0; j<finish; j++)
        free(array[j]);
    free(array);
}

/*  Function receives a room name and compares it to every room in the system.
 * Error Codes: ILLEGAL_PARAMETER if the room is not int the system.
 *              NULL_PARAMETER if sys is NULL*/
static Result find_room(ChallengeRoomSystem* sys,char* room_name,
                        ChallengeRoom** room) {
    if (sys == NULL)
        return NULL_PARAMETER;
    for (int i = 0; i < sys->room_array_size; i++) {
        if (!strcmp(sys->rooms[i]->name, room_name)) {
            *room = sys->rooms[i];
            return OK;
        }
    }
    return ILLEGAL_PARAMETER;
}

/*  Function adds the visitor to the dinamic list (at the beginning of the list)
 * Error Codes: MEMORY_PROBLEM if allocation fails
 *              NULL_PARAMETER if visitor is NULL*/
static Result add_to_list(ChallengeRoomSystem *sys,Visitor *visitor){
    if(visitor==NULL)
        return NULL_PARAMETER;
    VisitorList new_visitor = malloc(sizeof(*new_visitor));
    if(new_visitor==NULL)
        return MEMORY_PROBLEM;
    new_visitor->visitor = visitor;
    VisitorList *list_ptr = NULL;
    list_ptr= &new_visitor;
    if(find_visitor_in_list(sys, visitor, &list_ptr) == OK ){
        free(new_visitor);
        return ALREADY_IN_ROOM;
    }
    new_visitor->next_visitor = *sys->first_visitor;
    if(*sys->first_visitor != NULL)
        (*sys->first_visitor)->previous_visitor = new_visitor;
    new_visitor->previous_visitor = NULL;
    *sys->first_visitor = new_visitor;
    return OK;
}

/*  Function removes the visitor to the dinamic list
 * Error Codes: ILLEGAL_PARAMETER if the visitor is not in the list
 *              NULL_PARAMETER if visitor is NULL*/
static  Result remove_from_list(ChallengeRoomSystem *sys, Visitor *visitor){
    if (visitor == NULL)
        return NULL_PARAMETER;
    VisitorList **ptr = malloc(sizeof(VisitorList*));
    if(!ptr)
        return MEMORY_PROBLEM;
    VisitorList current = NULL;
    Result res=find_visitor_in_list(sys,visitor,ptr);
    if(res!= OK){
        return res;
    }
    VisitorList pointer = NULL;
    pointer = *(*ptr);
    *ptr = NULL;
    if(pointer->previous_visitor != NULL){
        current= pointer->previous_visitor;
        current->next_visitor = pointer->next_visitor;
    }
    else{
        (*sys->first_visitor) = pointer->next_visitor;
    }
    if (pointer->next_visitor != NULL){
        current = pointer->next_visitor;
        if (pointer->previous_visitor != NULL){
            current->previous_visitor = pointer->previous_visitor;
        }
        else{
            current->previous_visitor = NULL;
        }
    }
  //  *(*ptr)=pointer;
    pointer->next_visitor=NULL;
    pointer->previous_visitor=NULL;
    free(pointer);
    free(ptr);
    return OK;

}

/*function finds the pointer to the visitorList of visitor
* returns the pointer in pointer that received
* if visitor is not in list than function returns Illegal_parameter*/
static Result find_visitor_in_list(ChallengeRoomSystem *sys, Visitor *visitor,
                                   VisitorList** pointer){
    if(visitor==NULL)
        return NULL_PARAMETER;
    for (VisitorList *ptr= sys->first_visitor; (*ptr)!=NULL ;
         ptr=&((*ptr)->next_visitor)){
        if (( !strcmp((*ptr)->visitor->visitor_name,visitor->visitor_name)) ||
        (*ptr)->visitor->visitor_id == visitor->visitor_id){
            *pointer=ptr;
            return OK;
        }
    }
    return NOT_IN_ROOM;
}


/* * Function finds visitor in system by ID parameter
 * @param sys pointer to a system in which to search
 * @param visitor_id identify the visitor with
 * @param visitor returm pointer to requested visitor. NULL iss not in the sys.
 * @return OK if visitor is found.
 * ILLEGAL_PARAMETER if visitor ID is not in the sys.
 */
static Result find_visitor_by_id(ChallengeRoomSystem *sys, int visitor_id,
                                 Visitor** visitor){
    VisitorList *ptr = NULL;
    for (ptr = sys->first_visitor ; (*ptr) != NULL ;
         ptr=&((*ptr)->next_visitor)){
        if ((*ptr)->visitor->visitor_id==visitor_id){
            *visitor=(*ptr)->visitor;
            return OK;
        }
    }
    *visitor=NULL;
    return NOT_IN_ROOM;
}

static Result find_challenge(ChallengeRoomSystem *sys, char* challenge_name, Challenge** ptr){
    for (int i = 0; i < sys->challenge_array_size ; ++i) {
        if (!strcmp(challenge_name,sys->challenges[i]->name)){
            *ptr=sys->challenges[i];
            return OK;
        }

    }
    return ILLEGAL_PARAMETER;
}

static Result create_list(Visitor *visitor, ChallengeRoomSystem *sys){
    VisitorList new_node = malloc(sizeof(*new_node));
    if(!new_node){
        return MEMORY_PROBLEM;
    }
    new_node->visitor = visitor;
    new_node->next_visitor = NULL;
    new_node->previous_visitor = NULL;
    (*sys->first_visitor) = new_node;
    return OK;
}

static Result free_challenges(ChallengeRoomSystem *sys, char **best_time){
    if(sys->challenge_array_size == 0){
        *best_time=NULL;
        return OK;
    }
    char *best_timed_challenge = sys->challenges[0]->name;
    int current_best_time = sys->challenges[0]->best_time;
    int challenge_time, best_challenge=0;
    for (int k = 1; k < sys->challenge_array_size; ++k) {
        best_time_of_system_challenge(sys, sys->challenges[k]->name,
                                               &challenge_time);
        if ((current_best_time == 0 || challenge_time <= current_best_time) &&
            challenge_time != 0) {
            if( challenge_time == current_best_time){
                if(strcmp(best_timed_challenge,sys->challenges[k]->name) > 0 ) {
                    current_best_time = challenge_time;
                    best_timed_challenge = sys->challenges[k]->name;
                    best_challenge = k;
                }
            }
            else{
                current_best_time = challenge_time;
                best_timed_challenge = sys->challenges[k]->name;
                best_challenge = k;
            }
        }
    }
    char *challenge_name_copy= malloc
            (strlen(sys->challenges[best_challenge]->name)+1);
    if (!challenge_name_copy)
        return MEMORY_PROBLEM;
    strcpy(challenge_name_copy,sys->challenges[best_challenge]->name);
    free_challenges_memory(sys);
    if (current_best_time == 0) {
        *best_time = NULL;
        free(challenge_name_copy);
    }
    else
        *best_time = challenge_name_copy;
    return OK;
}

static void free_challenges_memory(ChallengeRoomSystem *sys){
    for (int j = 0; j < sys->challenge_array_size; ++j) {
        reset_challenge(sys->challenges[j]);
        free(sys->challenges[j]);
    }
    free(sys->challenges);
}