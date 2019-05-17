#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>


#include "challenge.h"

//defines:
#define DEFAULT 0

//functions:
/*  Function initializes a specific challenge
 * Receives: Challenge pointer
 *           challenge id as int
 *           challenge name as string
 *           level of challenge
 * Error Codes: NULL_PARAMETER if challenge or name is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory.*/
Result init_challenge(Challenge *challenge, int id, char *name, Level level) {
    //input check
    if ( challenge ==  NULL || name == NULL) {
        return NULL_PARAMETER;
    }
    char *name_copy = malloc((strlen(name) + 1)*(sizeof(char)));
    //adds 1 for "\0" - end of string.
    //verify allocation
    if (name_copy == NULL) {
        return MEMORY_PROBLEM;
    }
    strcpy(name_copy, name);
    challenge -> id = id;
    challenge -> name = name_copy;
    challenge -> level = level;
    challenge -> best_time = DEFAULT;
    challenge -> num_visits = DEFAULT;
    return OK;
}

/*  Function resets the fields in a challenge to default, makes name to be NULL
 * Receives: Challenge pointer
 * Error Codes: NULL_PARAMETER if challenge is NULL*/
Result reset_challenge(Challenge *challenge) {
    //check if the the challenge parameter is usable.
    if (challenge ==  NULL) {
        return NULL_PARAMETER;
    }
    //check if the name field contains a string to be freed
    if (challenge -> name != NULL ) {
        free(challenge->name);
    }
    challenge -> name = NULL;
    challenge -> id = DEFAULT;
    challenge -> level = (Level) DEFAULT;
    challenge -> best_time = DEFAULT;
    challenge -> num_visits = DEFAULT;
    return OK;
}

/*Function changes challenge name to given parameter.
 * Receives: Challenge pointer
 *           new challenge name as string
 * Error Codes: NULL_PARAMETER if challenge or name is NULL
 *              MEMORY_PROBLEM if system was unable to allocate memory*/
Result change_name(Challenge *challenge, char *name) {
    //input check
    if ( challenge ==  NULL || name == NULL) {
        return NULL_PARAMETER;
    }
    char *name_copy = malloc((strlen(name) + 1)*(sizeof(char)));
    //adds 1 for "\0" - end of string.
    //verify allocation
    if (name_copy == NULL) {
        return MEMORY_PROBLEM;
    }
    strcpy(name_copy, name);
    free(challenge->name);
    challenge -> name = name_copy;
    return OK;
}

/*Function sets the best_time field in a challenge to given time parameter
 * Receives: Challenge pointer
 *           time to set
 * Error Codes: NULL_PARAMETER if challenge is NULL
 *              ILLEGAL_PARAMETER given time is greater than current best time*/
Result set_best_time_of_challenge(Challenge *challenge, int time) {
    //input check
    if ( challenge == NULL ) {
        return NULL_PARAMETER;
    }
    if ( time <= 0 ) {
        return ILLEGAL_PARAMETER;
    }
    challenge -> best_time = time;
    return OK;
}

/*Function provides the best_time field for a challenge
 * Receives: Challenge pointer
 *           int pointer to return the best time
 * Error Codes: NULL_PARAMETER if challenge is NULL*/
Result best_time_of_challenge(Challenge *challenge, int *time) {
    //input check
    if ( challenge == NULL ) {
        return NULL_PARAMETER;
    }
    *time = challenge -> best_time;
    return OK;
}

/*Function increases the number of visitors by one for a specific room
 * * Receives: Challenge pointer
 * Error Codes: NULL_PARAMETER if challenge is NULL*/
Result inc_num_visits(Challenge *challenge) {
    //input check
    if ( challenge == NULL ) {
        return NULL_PARAMETER;
    }
    challenge -> num_visits++;
    return OK;
}

/*Function provides the num_visits field for a challenge
 *  * Receives: Challenge pointer
 *              int pointer to return the number of visits
 * Error Codes: NULL_PARAMETER if challenge or name is NULL*/
Result num_visits(Challenge *challenge, int *visits) {
    //input check
    if ( challenge == NULL ) {
        return NULL_PARAMETER;
    }
    *visits = challenge -> num_visits;
    return OK;
}
