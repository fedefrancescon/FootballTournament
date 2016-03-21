//
// AUTH:      Federico Francescon
// MATR:      146995
// LAUREA:    LT Informatica 2013/2014
// CORSO:     Sistemi Operativi 1
// 
// Progetto:  Football Tournament
//

#ifndef SHARED_FUNC_H
#define SHARED_FUNC_H

// ~~~~~~~~~~~~~~~~~~~~~ INCLUDES
#include <time.h>

// ~~~~~~~~~~~~~~~~~~~~~ DEFINITIONS
//#define EXIT_SUCCESS 0
#define ERR_GENERAL 1
#define LINE_MAX_LENGTH 2048
#define CMD_MAX_ARGS 256
#define LIST_LEN 256
#define _TRUE_ 1
#define _FALSE_ 0


// ~~~~~~~~~~~~~~~~~~~~~ LIST DEFINITION AND FUNCTIONS
// Doubly linked list node structure
typedef struct list list;
struct list {
  char* element;
  list* prev;
  list* next;
};

// ~~~~~~~~~~~~~~~~~~~~~ FUNCTIONS
// Create a new empty list and returns it's address
list*   newList();

// Destroying a whole list, calling function with pointer dfAdrr and passing pointer to pointer to element
void    destroyList( list** l, void* dfAdrr );

// Append pointer to new element to list l
void    listAppend( list* l, char* e );

// Moves an element of a list before a specified one
void    listMoveBefore( list** head, list* before, list* toMove );

// Returns pointer to first element of a list
list*   getListFirst( list* l );

// Returns pointer to last element of a list
list*   getListLast( list* l );

// Returns pointer to n-th node of a list( first index is 0 )
list*   listIndex( list* l, int i );

// Return number of elements in a list
int     listLength( list* l );

// Tails second list to the first one
void    catenateLists( list* l_1, list* l_2 );


// ~~~~~~~~~~~~~~~~~~~~~ SYSTEM FUNCTIONS
// Get number of system cores, works only on linux
int     getSystemCoreNum();

// Gets difference between two times
struct timespec timeDiff( struct timespec start, struct timespec end );



// ~~~~~~~~~~~~~~~~~~~~~ PARSING FUNCTIONS
// Parse a line identifying words divide by white space char
char**  parseStr( char* line );


#endif