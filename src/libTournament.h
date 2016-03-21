//
// AUTH:      Federico Francescon
// MATR:      146995
// LAUREA:    LT Informatica 2013/2014
// CORSO:     Sistemi Operativi 1
// 
// Progetto:  Football Tournament
//

#ifndef LIB_TOURNAMENT_H
#define LIB_TOURNAMENT_H

#include "sharedFunc.h"

// ~~~~~~~~~~~~~~~~~~~~~ DECLARATIONS
#define NAME_MAX_LEN 25
#define TEAM_MAX_NUM 8
#define TEAM_MIN_NUM 8
#define TEAM_MAX_SKILL 10
#define TEAM_MIN_SKILL 1
#define GOAL_PROBABILITY 50
#define WINNER_POINTS 3
#define PARITY_POINTS 1
#define LOOSER_POINTS 0
#define USER_INPUT_TIME 2
#define N_GOAL_POSSIBILITIES 3

// ~~~~~~~~~~~~~~~~~~~~~ GLOBAL VARIABLES
extern int testmode;        // Used to set 'test mode' on, this makes firts team of a match to ALWAYS sign a goal, second team signs no goals ( defined in libTournament.c )

// ~~~~~~~~~~~~~~~~~~~~~ DATA STRUSCTURES
// specifing the struct tag I can forward define
typedef struct team team;
typedef struct match match;
typedef struct day day;

// Struct used to identify a team
struct team{
  int   id;                   // Univoque identifier for a team, comparisons between ints are faster than with strings
  char  name[NAME_MAX_LEN];   // Team name
  int   skill;                // Team Skill Coefficient
  int   points;               // Point reached during the tournament
  list* matches;              // List of matches involving this team
  pthread_mutex_t mutex_team; // Used to lock a team while adding points
  int   interactive;          // If team is marked as interactive, user will be asked during goal events
};

// Structure for Matches
struct match{
  team* team_1;
  team* team_2;
  day*  match_day;  // Pointer to the match day
  int   goal_1;     // Goals of team_1
  int   goal_2;     // Goals of team_2
  char  type;       // 'F' means first round, 'S' means second round
  int   played;     // Indicates if match has been played or not
};

// Structure of day
struct day{
  char  name[NAME_MAX_LEN]; // Day name
  list* matches;            // Pointer to list of matches on that day
  list* teams;              // List of Teams involved during the day
};

// Used for analyzing days array
pthread_mutex_t mutex_allDaysList;  // Mutex used for locking days list when incrementing pointer to day
pthread_mutex_t mutex_userStdIn;    // Muter used for locking stdout and stdin when user input is required


// ~~~~~~~~~~~~~~~~~~~~~ TOURNAMENT FUNCTIONS
team*   newTeam( char n[], int s, int id );           // Create a new Team structure
void    destroyTeam( team** t );                      // Destroy a team
match*  newMatch( team* t1, team* t2, char type );    // Create a new Match structure
void    destroyMatch( match** m );                    // Destroy a Match
day*    newDay( char* dname );                        // Create a new Day structure
void    destroyDay( day** d );                        // Destroy a day
int     isGoal( int skill );                          // Calculate with random probability based on team's skill level if is goal ( return _TRUE_ )  or not ( return _FALSE_ )
int     isTeamPlaying( team* t, day* d );             // Return _TRUE_ if specified team plays during the specified day, otherwise _FALSE_
void    sortTeamsByScore( list** lt );                // Sorts teams list in descendant order based on score.
void    playMatch( match* m );                        // Plays given match, assign points to teams

// Generate all matches and distribute them to the correct day
int     generateMatches( list* teams, list* allMatches, list* allDays );

// ~~~~~~~~~~~~~~~~~~~~~ SINGLE THREADED FUNCTIONS ( for testing )
// Plays all Matches within a day
void    singleT_playDay( day* d  );

// ~~~~~~~~~~~~~~~~~~~~~ MULTI THREADED FUNCTIONS
// Iterate over list of days an plays them
void    multiT_playDay( list** ds );
// Plays all Matches within a day
void    multiT_playMatch( list* dayMatches );


// Timeout for user input in interactive mode, return inputed char or default char value
char    timedUserInput( int to, char def );

#endif