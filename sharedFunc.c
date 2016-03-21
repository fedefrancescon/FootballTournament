//
// AUTH:      Federico Francescon
// MATR:      146995
// LAUREA:    LT Informatica 2013/2014
// CORSO:     Sistemi Operativi 1
// 
// Progetto:  Football Tournament
//

// ~~~~~~~~~~~~~~~~~~~~~ INCLUDES
#include <unistd.h> // For system cores count (linux only)
#include <stdio.h>
#include <stdlib.h>
#include "sharedFunc.h"


// ~~~~~~~~~~~~~~~~~~~~~ LIST FUNCTIONS
// Create a new empty list and returns it's address
list* newList(){
  list* newList = malloc( sizeof(list) );

  newList->element = NULL;
  newList->next = NULL;
  newList->prev = NULL;

  return newList;
}

// Destroying a whole list, calling function with pointer dfAdrr and passing pointer to pointer to element
// If passing NULL function pointer element will not be cleaned
void destroyList( list** l, void* dfAdrr ){
  void (*destroyFunc)( char** );
  destroyFunc = dfAdrr;

  list* cur = NULL;
  list* next = getListFirst( *l );
  while( next != NULL ){
    cur = next;
    next = next->next;

    //Calling destroy function for element within list node
    if( destroyFunc != NULL ){ destroyFunc( &cur->element ); }

    // Freeing malloc'ed memory
    free( cur );
  }

  // Set to NULL list pointer
  *l = NULL;
}

// Append pointer to new element to list l
void listAppend( list* l, char* e ){
  if( l->element == NULL ){ l->element=e; l->next=NULL; return; }
  if( l->next == NULL ){
    l->next=malloc( sizeof(list) );
    l->next->element = NULL;
    l->next->prev = l;
    l->next->next = NULL;
  }
  listAppend( l->next, e );
}

// Moves an element of a list before a specified one
void listMoveBefore( list** head, list* before, list* toMove ){
  if( toMove->next != NULL ){ toMove->next->prev = toMove->prev; }
  if( toMove->prev != NULL ){ toMove->prev->next = toMove->next; }
  
  toMove->prev = before->prev;
  toMove->next = before;
  before->prev = toMove;

  if( toMove->prev != NULL ){ toMove->prev->next = toMove; }
  else{ *head = toMove; }
}

// Returns pointer to first element of a list
list* getListFirst( list* l ){
  while( l->prev != NULL){ l = l->prev; }
  return l;
}

// Returns pointer to last element of a lis
list* getListLast( list* l ){
  while( l->next != NULL){ l = l->next; }
  return l;
}

// Returns pointer to n-th node of a list( first index is 0 )
list*  listIndex( list* l, int i ){
  l = getListFirst( l );
  int x = 0;
  list* li = NULL;
  while( x <= i && l != NULL ){
    if( x == i ){ li = l; break; }
    
    x++;
    l = l->next;
  }

  return li;
}

// Return number of elements in a list
int listLength( list* l ){
  l = getListFirst( l );
  int count = 0;
  while( l != NULL ){ count++; l = l->next; };

  return count;
}

// Tails second list to the first one
// Link first element of second list to last one of first list
void catenateLists( list* l_1, list* l_2 ){
  list* end1 = getListLast( l_1 );
  list* start2 = getListFirst( l_2 );
  
  end1->next = start2;
  start2->prev = end1;
}


// ~~~~~~~~~~~~~~~~~~~~~ SYSTEM FUNCTIONS
// Get number of system cores, works only on linux
int getSystemCoreNum(){
  int nprocs = -1;
  
  // if system variable  _SC_NPROCESSORS_ONLN then return the number of processors, if fails return a value < 0 
  #ifdef _SC_NPROCESSORS_ONLN
  nprocs = sysconf(_SC_NPROCESSORS_ONLN);
  #endif

  return nprocs;
}

// Gets difference between two times
struct timespec timeDiff( struct timespec start, struct timespec end){
  struct timespec temp;

  if( (end.tv_nsec - start.tv_nsec) < 0 ){
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  }
  else{
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }

  return temp;
}


// ~~~~~~~~~~~~~~~~~~~~~ PARSING FUNCTIONS
// Parse a line identifying words divide by white space char
// This function reads a line char by char until a NULL TERMINATING or NEWLINE char is found
// When white spaces are found they are substituted by '' and pointer to first char of word is appendend to argv array
char** parseStr( char* line ){
  static char* argv[ CMD_MAX_ARGS ];
  int argc = 0, i = 0, start = 0;

  while( line[i] != '\0' && line[i] != '\n' && argc < (CMD_MAX_ARGS -1) && i < LINE_MAX_LENGTH ){
    if( line[i] == ' ' ){
      line[i] = '\0';
      argv[argc] = line + start;
      argc++;
      start = i+1;
    }
    i++;
  }
  argc++;
  line[i] = '\0';
  argv[argc-1] = line + start; // Adding last command
  argv[argc] = '\0';

  return &argv[0];
}