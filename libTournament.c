//
// AUTH:      Federico Francescon
// MATR:      146995
// LAUREA:    LT Informatica 2013/2014
// CORSO:     Sistemi Operativi 1
// 
// Progetto:  Football Tournament
//



// ~~~~~~~~~~~~~~~~~~~~~ INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include "libTournament.h"


// ~~~~~~~~~~~~~~~~~~~~~ GLOBAL VARIABLES DEFINITIONS
int testmode = _FALSE_;    // Test Mode( variable declared in libTournament.h ) is off by default, enabling will cause a well known behaviour that will give known results


// ~~~~~~~~~~~~~~~~~~~~~ TOURNAMENT FUNCTIONS
// Create a new Team returning the pointer
team* newTeam( char* n, int s, int id ){
  team* newT = malloc( sizeof(team) );

  // Setting default values and desired parameters
  strncpy( newT->name, n, NAME_MAX_LEN );   // Name of new Team, passed as argument
  newT->id          = id;                   // Univoque Team id assignation
  newT->skill       = s;                    // Team's Skill level, set in config file( 1-10 ), influence goal's probability
  newT->points      = 0;                    // Points collected during the Tournament
  newT->matches     = newList();            // List of matches where the team is involved 
  newT->interactive = _FALSE_;              // User interaction is off by default

  // Checking skill limits
  if( newT->skill > TEAM_MAX_SKILL ){
    printf( "# Reading config for team: %-25s skill: %d too high, defaulting to max skill level: %d\n", newT->name, newT->skill, TEAM_MAX_SKILL );
    newT->skill = TEAM_MAX_SKILL;
  }
  else if( newT->skill < TEAM_MIN_SKILL ){
    printf( "# Reading config for team: %-25s skill: %d too low, defaulting to min skill level: %d\n", newT->name, newT->skill, TEAM_MIN_SKILL );
    newT->skill = TEAM_MIN_SKILL;
  }
  
  // Initializing Team mutex
  pthread_mutex_init( &(newT->mutex_team) , NULL);

  return newT;
}

// Destroy a team
void destroyTeam( team** t ){
  team * curTeam = *t;
  destroyList( &curTeam->matches, NULL ); // Matches will be destroyed by allMatches list
  pthread_mutex_destroy( &curTeam->mutex_team );
  free( curTeam );

  // Setting to NULL team pointer
  *t = NULL;
}

// Create a new Match returning the pointer
match* newMatch( team* t1, team* t2, char type ){
  match* newM = malloc( sizeof(match) );

  // Setting default values and desired parameters
  newM->team_1      = t1;       // Team 1. Could be considered as 'host' player
  newM->team_2      = t2;       // Team 2, opponent. Could be considered as 'guest' player
  newM->type        = type;     // First Round( 'F' ) or Second Round( 'S' )
  newM->match_day   = NULL;     // Point to the match's Day
  newM->played      = _FALSE_;  // Indicates if a Match has been played or not ( debugging )

  return newM;
}

// Destroy a Match
void destroyMatch( match** m ){
  free( *m );

  // Setting to NULL match pointer
  *m = NULL;
}

// Create a new Day returning the pointer
day* newDay( char* dname ){
  day* newD = malloc( sizeof(day) );

  // Setting default values and desired parameters
  newD->teams   = newList();                  // List of teams playing during the day
  newD->matches = newList();                  // List of matches disputed 
  strncpy( newD->name, dname, NAME_MAX_LEN ); // Setting day name

  return newD;
}

// Destroy a Day
void destroyDay( day** d ){
  day* curDay = *d;
  destroyList( &curDay->matches, NULL );  // Matches will be destroyed by allMatches list
  destroyList( &curDay->teams, NULL );    // Teams will be destroyed by teams list
  free( curDay );

  // Setting to NULL team pointer
  *d = NULL;
}

// Generate list of all matches that would be played during the tournament
int generateMatches( list* teams, list* allMatches, list* allDays ){
  list*   cur = teams;          // Used for current team
  list*   next;                 // Used for the opposite team
  list*   scanList = NULL;      // 'Service' variable used to iterate a list
  int     cicleCount = 0;       // 'Service' variable used to prevent infinite loop
  match*  curMatch;             // Used for current match
  day*    scanDay = NULL;       // Used for creating new day / iterate over days list 
  char    dname[NAME_MAX_LEN];  // Used to set day name

  // Used for temporary memorize fighting teams
  team* team_1 = NULL;
  team* team_2 = NULL;

  // Second Rounds temporary Lists
  list* secondRounds = newList();   // List of second round's matches
  list* secondDays   = newList();   // List of second round's days

  // Generating number of days depending on number of teams playing the tournament
  // If Even Team number -> Days number is nTeam -1
  // if Odd Team number -> Days number is Team 
  // Es: Teams 6 -> (6-1)=5 Days
  //     Teams 5 -> 5 Days
  // With Odd team number  each day of the tournament a Team does not play. 
  int nteams    = listLength( teams );                // Number of teams
  int ndays     = nteams%2==0 ? nteams -1 : nteams;   // Number of days to be played
  int team_1_i  = 0;  // Start from first team in list
  int team_2_i  = 0;  // Start from first team in list
  int day_i     = 0;  // Start Numbering from 1

  // Creating empty days
  // Using two separate lists that will be merged once finisched.
  // No problem because Firsts and Seconds are equal
  while( day_i < ndays ){
    // Creating first Round
    sprintf( dname, "Day %2d", day_i+1 ); // Giving right name
    listAppend( allDays, (char*)newDay( dname ) );
    // Creating second Round
    sprintf( dname, "Day %2d", day_i+1+ndays ); // Giving right name
    listAppend( secondDays, (char*)newDay( dname ) );

    day_i++; 
  }

  // Generating Matches
  // Selecting day index equal to team_1 index will start looking for a free day for both teams
  // at day n, this ensures each day is full of teams, otherwise may occur that assignation will 
  // fail because of matches are not well distributed accross available days.
  // n°1 Team -> start looking from day 1
  // n°2 Team -> start looking from day 2
  // n°3 Team -> start looking from day 3
  // If days' list end is reached, day index start again from 0
  // cicleCount variable is uset to prevent infinite looping even if this should never happen 
  team_1_i  = 0;  // Index of team_1 in teams' list
  team_2_i  = 0;  // Index of team_2 in teams' list
  day_i     = 0;  // Index of current day in days' list
  while( team_1_i < nteams-1 ){
      team_1 = (team *)cur->element;
      next = cur->next;
      team_2_i = team_1_i + 1;
      day_i = team_1_i;
    while( team_2_i < nteams ){
      team_2 = (team *)next->element;
      cicleCount = 0;
      // Determining first empty day, day_i at the end of the cicle will be the index of free day for the match
      scanList = listIndex( allDays, day_i );
      while( isTeamPlaying( team_1, (day*)scanList->element ) || isTeamPlaying( team_2, (day*)scanList->element) ){
        day_i++;
        if( day_i >= ndays ){
          if( cicleCount > 0 ){ 
            printf( "# Day assignation failed for Match '%s' vs '%s'\n", team_1->name, team_2->name );
            return _FALSE_;
          }
          day_i = 0;
          cicleCount++;
          scanList = getListFirst( allDays );
        }
        else{ scanList = scanList->next; }
      }

      // First round
      curMatch = newMatch( team_1, team_2, 'F' );             // Create First Round Match
      listAppend( allMatches, (char *)curMatch );             // Appending new Match to list of all matches
      listAppend( team_1->matches, (char *)curMatch );        // Appending match to team's list of matches
      listAppend( team_2->matches, (char *)curMatch );

      scanDay = (day*)listIndex( allDays, day_i )->element;   // Associating match and teams to the right day
      listAppend( scanDay->matches, (char*)curMatch );
      listAppend( scanDay->teams, (char*)team_1 );
      listAppend( scanDay->teams, (char*)team_2 );
      
      // Second round
      curMatch = newMatch( team_2, team_1, 'S' );             // Create Second Round Match appending to a different list
      listAppend( secondRounds, (char *)curMatch );
      listAppend( team_1->matches, (char *)curMatch );
      listAppend( team_2->matches, (char *)curMatch );

      scanDay = (day*)listIndex( secondDays, day_i )->element;  
      listAppend( scanDay->matches, (char*)curMatch );
      listAppend( scanDay->teams, (char*)team_1 );
      listAppend( scanDay->teams, (char*)team_2 );

      // Next opposite Team
      next = next->next;
      team_2_i ++;
    }
    // Next Team
    cur = cur->next;
    team_1_i++;
  }

  // Tails Second Rounds to All Matches and days
  // Second Rounds are carbon copy matches of First Rounds ones.
  // I've created both at the same time but appendend to different lists
  // Here I merge second round's list so all matches will appear in correct order
  catenateLists( allMatches, secondRounds );
  catenateLists( allDays, secondDays );

  return _TRUE_;
}

// Calculates if is goal with random probability based on team's skill level
// Considering value between [ 1, 100 ] Goal probability is 50%
// Overall probability is scaled in function of team's skill and is a Number between [ 5, 50 ]
// Generating a randon number between [ 1, 100 ] in randnum is <= to probability this means is a valid Goal
// Es:
//    Team skill        = 5
//    GOAL_PROBABILITY  = 50
//    TEAM_MAX_SKILL    = 10
//    ----------------------
//    Probabiliy        = ( 50 * 5 ) / 10 = 25
//    random = 73 => NO GOAL
//    random = 17 => YES, GOAL!
int isGoal( int skill ){
  int probability = GOAL_PROBABILITY * skill / TEAM_MAX_SKILL;  // number between 5 and 50
  int random = rand() % (100 - 1 + 1) + 1;                      // rand() % (max_n - min_n + 1) + min_n;

  return ( random <= probability );
}

// return True if specified team is already playing during the day, otherwise false
// Iterate over day's teams list and checks if specified team is already playing that day
int isTeamPlaying( team* t, day* d ){
  list* scanTeamsList = d->teams;
  team* scanTeam = NULL;

  while( scanTeamsList != NULL && scanTeamsList->element != NULL ){
    scanTeam = (team*)scanTeamsList->element;
    if( t->id == scanTeam->id ){ return _TRUE_; }
    //if( strcmp( t->name, scanTeam->name ) == 0 ){ return 1; }
    scanTeamsList = scanTeamsList->next;
  }

  return _FALSE_;
}

// Sort List of teams by score, from higest to lowest
// Sort function compares each team with all the others
// It's a very simple and inefficient sort function
void sortTeamsByScore( list** lt ){
  list* cur = *lt;
  list* next = NULL;
  team* team_cur = NULL;
  team* team_next = NULL;

  // Very stupid sort function 
  while( cur != NULL && cur->element != NULL ){
    next = cur->next;
    team_cur = (team *) cur->element;
    while ( next != NULL && next->element != NULL){
      team_next = (team *) next->element;
      if( team_next->points > team_cur->points ){ // If next team's  points are greater than temporary one then move
        listMoveBefore( lt, cur, next );
        cur = next;
        break;
      }
      next = next->next;
    }
    cur != next && (cur = cur->next);
  }
}

// Plays a Match
// executes N time the goal function, if is goal saves it
// If Interactive Team is involved then user imput is requested
// Is required a Mutex to avoid contemporary access to stdin and stdout when running multiple threads
// User can block a goal to his team if is attacked.
// If user is attacking can choose to shoot at opponent's door.
void playMatch( match* m ){
  team* team_1 = m->team_1;
  team* team_2 = m->team_2;
  char userInput;
  int result;

  // Normal playing mode
  if( !testmode ){ 
    int i = 0;
    // Executing goals n times in a match
    while( i < N_GOAL_POSSIBILITIES ){
      // Goal Team 1 -> Team 2
      if( isGoal( team_1->skill ) ){ 
        if( team_1->interactive ){
          pthread_mutex_lock( &mutex_userStdIn );
          printf( "ATTACK: '%s' >> '%s'. (%dsec) Your action: (S)hoot or (B)lock? ", team_1->name, team_2->name, USER_INPUT_TIME ); fflush(stdout);
          userInput = timedUserInput( USER_INPUT_TIME, 'S' );
          printf( " - Selected: '%c'\n", userInput );
          pthread_mutex_unlock( &mutex_userStdIn );
          if( userInput == 'S' || userInput == 's' ){ m->goal_1++; }
        }
        else if( team_2->interactive ){
          pthread_mutex_lock( &mutex_userStdIn );
          printf( "DEFEND: '%s' << '%s'. (%dsec) Your action: (S)hoot or (B)lock? ", team_2->name, team_1->name, USER_INPUT_TIME ); fflush(stdout);
          userInput = timedUserInput( USER_INPUT_TIME, 'B' );
          printf( " - Selected: '%c'\n", userInput );
          pthread_mutex_unlock( &mutex_userStdIn );
          if( !(userInput == 'B' || userInput == 'b') ){ m->goal_1++; }
        }
        else{ m->goal_1++; } // When No Interaction 
      }
      // Goal Team 2 -> Team 1
      if( isGoal( team_2->skill ) ){
        if( team_2->interactive ){
          pthread_mutex_lock( &mutex_userStdIn );
          printf( "ATTACK: '%s' >> '%s'. (%dsec) Your action: (S)hoot or (B)lock? ", team_2->name, team_1->name, USER_INPUT_TIME ); fflush(stdout);
          userInput = timedUserInput( USER_INPUT_TIME, 'S' );
          printf( " - Selected: '%c'\n", userInput );
          pthread_mutex_unlock( &mutex_userStdIn );
          if( userInput == 'S' || userInput == 's' ){ m->goal_2++; }
        }
        else if( team_1->interactive ){
          pthread_mutex_lock( &mutex_userStdIn );
          printf( "DEFEND: '%s' << '%s'. (%dsec) Your action: (S)hoot or (B)lock? ", team_1->name, team_2->name, USER_INPUT_TIME ); fflush(stdout);
          userInput = timedUserInput( USER_INPUT_TIME, 'B' );
          printf( " - Selected: '%c'\n", userInput );
          pthread_mutex_unlock( &mutex_userStdIn );
          if( !(userInput == 'B' || userInput == 'b') ){ m->goal_2++; }
        }
        else{ m->goal_2++; } // When No Interaction 
      } 
      i++;
    }
  }
  else{
    // TEST MODE ONLY!!!!, first team signs a goal
    // this way each team wins the same number of matches.
    // everyone get same points at the end of the match and teams are presented in config's list order
    m->goal_1++;
  }

  // Assign points to winner, locking team when assigning
  result = m->goal_1 - m->goal_2;

  pthread_mutex_lock( &(team_1->mutex_team) );
  team_1->points += (int)( result > 0 ? WINNER_POINTS : ( result == 0 ? PARITY_POINTS : LOOSER_POINTS ) );
  pthread_mutex_unlock( &(team_1->mutex_team) );
 
  pthread_mutex_lock( &(team_2->mutex_team) );
  team_2->points += ( result < 0 ? WINNER_POINTS : ( result == 0 ? PARITY_POINTS : LOOSER_POINTS ) );
  pthread_mutex_unlock( &(team_2->mutex_team) );

  // Set match as played
  m->played = _TRUE_;
}

// Timeout for user input char in interactive mode
// Reads directly from stdin and uses function select to set timeout to user
// If function timeouts returns default char passed as argument
// This is used for reading input of user when inputing a char
// Uses a temp string of 30 chars instead of a single char to avoid strange
// behaviours when hitting ENTER key and stdin is not correctly emptyed 
char timedUserInput( int to, char def ){
  fd_set set;
  struct timeval timeout;

  char tmpStr[30];

  // Initialize the file descriptor set
  FD_ZERO( &set );
  FD_SET( 0, &set );  // 0 is stdin

  // Initialize the timeout data structure
  timeout.tv_sec = to;
  timeout.tv_usec = 0;

  // select returns 0 if timeout, 1 if input available, -1 if error
  int ready_for_reading = select( FD_SETSIZE,  &set, NULL, NULL, &timeout );

  if( ready_for_reading == 1 ){ read( 0, tmpStr, 30 ); def = tmpStr[0]; } // 0 is stdin
  return def;
}


// ~~~~~~~~~~~~~~~~~~~~~ SINGLE THREADED FUNCTIONS ( for testing )
// Executes all matches of a day
// Iterate over matches list of a single day and plays one by one
void singleT_playDay( day* d ){
  list* scanList = d->matches;
  match* scanMatch = NULL;

  while( scanList != NULL && scanList->element != NULL ){
    scanMatch = (match *)scanList->element;
    playMatch( scanMatch );
    scanList = scanList->next;
  }
}

// ~~~~~~~~~~~~~~~~~~~~~ MULTI THREADED FUNCTIONS
// Passing day list as a double pointer, so I can easily increment it
// Execute each day's list of matches in a different thread 
void multiT_playDay( list** ds ){
  pthread_t dayThread;
  while( ds != NULL && *ds != NULL && (*ds)->element != NULL ){
    if( ds == NULL || *ds == NULL ){ return; }
    pthread_mutex_lock( &mutex_allDaysList );
    // Getting current list indext
    list* days = *ds;
    day* d = (day *)days->element;
    // After getting current day pointer I'm going to set next list pointer so next thread will automatically read correct day
    *ds = days->next;
    pthread_mutex_unlock( &mutex_allDaysList );
    
    int ret =  pthread_create( &dayThread, NULL, (void *)&multiT_playMatch, (void*)d->matches);
    if( ret != 0 ){
      printf("### Error: pthread_create() failed\n");
      exit(EXIT_FAILURE);
    }
    pthread_join( dayThread, NULL );
  }
}

// Gets matches' list of a day( is executed on a different thread ) and iterates over it
// playing each match
void multiT_playMatch( list* dayMatches ){
  match* scanMatch = NULL;
  while( dayMatches != NULL && dayMatches->element != NULL ){
    scanMatch = (match *)dayMatches->element;

    playMatch( scanMatch );

    dayMatches = dayMatches->next;
  }
}