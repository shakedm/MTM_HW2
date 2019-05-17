#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MEMORY_FAULT 0

//function declarations:
char *flat_text(char **words, int n); //the required function.

//The main function:
int main() {
    char *words[] = {"Hello","To","234122","Matam"};
    char* string= flat_text(words, 4);
    if(string==NULL)
        return MEMORY_FAULT;
    else {
        printf("\n%s\n", string);
        free(string);
    }
    return 0;
}

//Functions:
char *flat_text(char **words, int n) {
    int word_len = strlen(words[0]);
    int total_len = word_len;
    char* curr_string = malloc((word_len+1) * sizeof(char));
    char* new_string = NULL;
    if ( curr_string == NULL ){
        return NULL;
    }
    curr_string = strcpy(curr_string, words[0]);
    for (int i=1; i<n; i++) {
        word_len = strlen(words[i]);
        total_len += word_len;
        new_string = (char*)realloc(curr_string, (total_len + 1)*sizeof(char));
        if ( new_string == NULL ) {
            return NULL;
        }
        strcpy((new_string + total_len - word_len), words[i]);
        curr_string = new_string;
    }
    return new_string;
}