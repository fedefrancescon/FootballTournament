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
#include <time.h>
#include <pthread.h>
#include "sharedFunc.h"
#include "libTournament.h"


// ~~~~~~~~~~~~~~~~~~~~~ DECLARATIONS
#define ERR_READCONF_MISSING  100   // Read conf error: missing config file or problems opening it
#define ERR_READCONF_TEAMNUM  101   // Read conf error: wrong number of teams or limits not respected
#define ERR_GEN_FAILED 200          // Matches generation failed

// Imports teams' list from speficied config file
// config file is in text format
// Rows starting with '#' are comments, so they will be ignored
// Row with just a numberic value will be interpreted as number of teams playing
// Teams' rows will be formatted as:
// [teamname] [skill]
// Invalid skill number will be automatically converted to minimum or maximum levels
// If number of teams is specified more than once in config file, first is considered valid, others are ignored
int readConfFile( char* confFile, list* teams, int maxteamoff ){
  FILE* fp = fopen(confFile, "r");
  if( confFile == NULL || fp == NULL ){
    printf( "# Unable to open confFile: %s\n", confFile );
    return ERR_READCONF_MISSING;
  }

  char line[LINE_MAX_LENGTH];
  int  nteams = 0, loadedTeams = 0;

  while( fgets(&line[0],LINE_MAX_LENGTH, fp) != NULL ){
    // COMMENT CHAR IS #
    if( line[0] == '#' ){ continue; }
    
    // Parse each line to read configuration
    char** argv = parseStr( &line[0] );
    
    // If only one arg in line, try to read teams number
    if( argv[0] == NULL ){ continue; }
    else if( argv[1] == NULL ){
      nteams == 0 && ( nteams = atoi(argv[0]) );
      continue;
    }

    // If both argv[0] (name) and argv[1] (skill) are defined append new team to the list
    // atoi returns zero if fails converting, not a problem since 0 is an invalid team's skill that will be automatically corrected to 1
    // Using as unique id index of team in list
    listAppend( teams, (char *)newTeam( argv[0], atoi(argv[1]), loadedTeams ) );
    loadedTeams++;
  }
  fclose(fp);

  // Checking teams number
  if( !maxteamoff && !( nteams == loadedTeams && nteams >= TEAM_MIN_NUM && nteams <= TEAM_MAX_NUM ) ){
    printf( "Configured teams number: %d - Read teams: %d\n", nteams, loadedTeams );
    printf("# Wrong teams number. May be unconfigured or mismatch number of configured teams.\n    4 <= team n° <= 8\n");
    return ERR_READCONF_TEAMNUM;
  }

  return EXIT_SUCCESS;
}

// Prints Football Tournament Help
void help(){
  printf("%s\n","Football Tournament");
  printf("%s\n","");
  printf("%s\n","Simulate and plays a Football Tournament with given teams, specified in a config file editable by user.");
  printf("%s\n","By default FT will automatically play all matches but user can also interact during matches of his team.");
  printf("%s\n","");
  printf("%s\n","COMMAND LINE OPTIONS:");
  printf("%s\n","  -c, --config");
  printf("%s\n","      Path of config file containing teams list, needed for Tournament generation.");
  printf("%s\n","      Config File is a text file editable by user:");
  printf("%s\n","      Path of config file containing teams list, needed for Tournament generation.");
  printf("%s\n","");
  printf("%s\n","  -o, --outfile");
  printf("%s\n","      Path of output file where generated Tournament's infos will be saved.");
  printf("%s\n","      If not specified FT will run normally but no results will be saved.");
  printf("%s\n","");
  printf("%s\n","  -i, --interactive");
  printf("%s\n","      User wil be asked for interaction when first team in config list plays.");
  printf("%s\n","      If user's team will be attacked: user can prevent other team's goal. Pressing 'B' or 'b' (block)");
  printf("%s\n","      If user's team is attacking: user can make a goal to the opponent. Pressing 'S' or 's' (block)");
  printf("%s\n","");
  printf("%s\n","  -m, --maxteamoff");
  printf("%s\n","      Disable maximun number of teams. Useful for application testing.");
  printf("%s\n","      Row containing number of team in config file will be ignored.");
  printf("%s\n","");
  printf("%s\n","  -p, --parallelthreads");
  printf("%s\n","      Force number of parallel threads for playing FT Days.");
  printf("%s\n","      Requires an additional argument: number of threads, if not specified or not valid will be ignored.");
  printf("%s\n","      Default value is set to number of CPU's cores.");
  printf("%s\n","");
  printf("%s\n","  -t, --testmode");
  printf("%s\n","      Sets testmode behaviour: first team involved in a match always make 1 goal, second team nothing.");
  printf("%s\n","      Tournament will end with all teams having same score so, ranking list has same teams' order as config file's list.");
  printf("%s\n","      This mode was only useful when testing if multithreading was Working as expected. Save also test mode summary!");
  printf("%s\n","      Whit this flag set, --interactive switch will be ignored.");
  printf("%s\n","");
  printf("%s\n","  -h, --help");
  printf("%s\n","      Prints this help");
  printf("%s\n","");
  printf("%s\n","");
  printf("%s\n","  EXAMPLES");
  printf("%s\n","      ./FootballTournament -c confDir/conf.txt -o outDir/out.txt");
  printf("%s\n","          This will read teams config from conf.txt in confDir and save output to out.txt inside outDir");
  printf("%s\n","      ./FootballTournament -c confDir/conf.txt -t -s -m");
  printf("%s\n","          This will read teams config from conf.txt in confDir, run in test mode, accept any number of teams");
  printf("%s\n","          and run in single thread mode. No output saved.");
  printf("%s\n","      ./FootballTournament -c confDir/conf.txt -p 10 -i");
  printf("%s\n","          This will read teams config from conf.txt in confDir, force 10 parallel threads and asks user for input.");
  printf("%s\n","");
  printf("%s\n","");
  printf("%s\n","AUTHOR");
  printf("%s\n","  Federico Francescon");
  printf("%s\n","  31/12/2014");
  printf("%s\n","  v1.0");
}


// ~~~~~~~~~~~~~~~~~~~~~ MAIN CODE
int main( int argc, char* argv[] ){
  // Measure execution time
  struct timespec time_start, time_end, time_elapsed;
  clock_gettime( CLOCK_MONOTONIC, &time_start );

  // Resetting randomness for match results
  srand( time(NULL) );

  // 'Service' variables
  list*   scanList      = NULL;
  list*   scanList_1    = NULL;
  team*   scanTeam_1    = NULL;
  team*   scanTeam_2    = NULL;
  match*  scanMatch   = NULL;
  day*    scanDay       = NULL;
  char    tmpString[LINE_MAX_LENGTH];
  int i = 0, exitStatus, len;

  // Config variables
  char* confFile    = NULL;       // Path of input file
  char* outFile     = NULL;       // Path of output file
  int singleThread  = _FALSE_;    // If set: multithreading will be disabled, all operation will be executed within main thread
  int interactive   = _FALSE_;    // If set: user interaction required for first list's team
  int maxteamoff    = _FALSE_;    // If set: max number of teams limit will be ignored, so (theoretically)infinite teams will be processed
  int nThreads      = 1;          // Number of parallel threads that will play Tournament's days  

  // Test mode variables
  int days_count    = 0;          // Counts number of generated day
  int m_count       = 0;          // Counts number of overall matches
  int m_played      = 0;          // Count of played matches
  int teams_count   = 0;          // Count number of teams
  // int testmode   = 0;          // Enables testmode of program, this global variable is declared in libTorunament.h and defined in libTournament.c

  // Determining number of parallel threads
  list* threads = newList();      // List of pointers to threads
  nThreads = getSystemCoreNum();
  nThreads < 1 && ( nThreads = 1 );
  //printf( "\nN° THREADS=%d\n\n", nThreads );

  // Working variables
  list* teams   = newList();      // teams list
  list* matches = newList();      // Matches list
  list* days    = newList();      // Days list

  // Initializing Mutexes 
  pthread_mutex_init( &mutex_allDaysList, NULL );
  pthread_mutex_init( &mutex_userStdIn, NULL );

  // BEGIN: Command line args
    // Available cmdline options
    // -c, --config:  Config file for tournament
    // -o, --output: Output file
    // -i, --interactive: First team will ask user for goals
    // -s, --siglethread: Disable multithreading
    // -p, --parallelthreads: Specify number of parallel threads
    // -m, --maxteamoff: Disable max team limit (debug)
    // -t, --testmode: Enter in text mode, firt team of a match will always make 1 goal
    // -h, --help: print help
    // default option (no arg specified, argc==1): -h
  i = 1;  // starting from 1 to skip argv[0] (executable filename)
  while( i < argc ){
    if( strcmp(argv[i],"-i")==0 || strcmp(argv[i],"--interactive")==0 ){ interactive = _TRUE_; }
    else if( strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0 ){ help(); return EXIT_SUCCESS; }
    else if( strcmp(argv[i],"-s")==0 || strcmp(argv[i],"--singlethread")==0 ){ singleThread = _TRUE_; nThreads = 1; }
    else if( strcmp(argv[i],"-m")==0 || strcmp(argv[i],"--maxteamoff")==0 ){ maxteamoff = _TRUE_; }
    else if( strcmp(argv[i],"-t")==0 || strcmp(argv[i],"--testmode")==0 ){ testmode = _TRUE_; }
    else if( ( strcmp(argv[i],"-p")==0 || strcmp(argv[i],"--parallelthreads")==0 ) && (i+1)<argc && (atoi(argv[++i]) > 1) ){ nThreads = atoi(argv[i]); }
    else if( ( strcmp(argv[i],"-c")==0 || strcmp(argv[i],"--config")==0 ) && (i+1)<argc ){ confFile = argv[++i]; }
    else if( ( strcmp(argv[i],"-o")==0 || strcmp(argv[i],"--output")==0 ) && (i+1)<argc ){ outFile = argv[++i]; }
    else{ printf( "Argument not supported: %s\n", argv[i] ); }
    i++;
  }
  // END: Command line args

  // Reading confFile, if error program exits with same error
  if( confFile == NULL ){ printf("# Missing config file. Exiting."); return ERR_READCONF_MISSING; }
  exitStatus = readConfFile( confFile, teams, maxteamoff );
  if( exitStatus != EXIT_SUCCESS ){ return exitStatus; }

  // If interaction specified, first list's team marked interactive
  if( interactive ){
    scanTeam_1 = (team *)teams->element;
    scanTeam_1->interactive = _TRUE_;
    printf( "\nUser interaction for team: %s\n", scanTeam_1->name );
  }

  // BEGIN: TEST MODE ONLY
  // In Test mode additional output is printed
  if( testmode ){
    // Printing teams list
    printf( "\n        CONFIGURED TEAMS\n" );
    printf( "ID  TEAM                     SKILL\n" );

    scanList = teams; 
    while( scanList != NULL && scanList->element != NULL ){
      scanTeam_1 = (team *)scanList->element;
      printf( "%3d %-25s  %2d\n", scanTeam_1->id, scanTeam_1->name, scanTeam_1->skill );
      teams_count++;
      scanList = scanList->next;
    }
  }
  // END: TEST MODE ONLY

  
  //
  // Generate all match to be played, assigning the to the respective day
  //
  if( !generateMatches( teams, matches, days ) ){
    printf( "# Error generating matches. Exiting.\n" );
    return ERR_GEN_FAILED;
  }

  // BEGIN: TEST MODE ONLY
  // In Test mode additional output is printed
  if( testmode ){
    // Printing matches list
    printf( "\n                     ALL GENERATED MATCHES\n" );
    printf( "N    DAY    F/S TEAM 1                   TEAM 2           \n" );

    scanList = matches; 
    i = 1;
    while( scanList != NULL && scanList->element != NULL ){
      scanMatch = (match *)scanList->element;
      scanTeam_1 = (team *)scanMatch->team_1;
      scanTeam_2 = (team *)scanMatch->team_2;
      scanDay = (day *)scanMatch->match_day;
      
      printf( "%-4d %-6s  %c  %-25s %-25s\n", i, ( scanDay!=NULL ? scanDay->name : "-" ), scanMatch->type, scanTeam_1->name, scanTeam_2->name );
      m_count++;

      scanList = scanList->next;
      i++;
    }

    // Printing days list
    printf( "\n    ALL DAYS\n" );
    printf( "DAY    MATCHES\n" );
    
    scanList = days;
    while( scanList != NULL && scanList->element != NULL ){
      scanDay = (day *)scanList->element;
      len = listLength( scanDay->matches );
      
      printf( "%-6s %4d\n", scanDay->name, len );

      days_count++;
      scanList = scanList->next;
    }
  }
  // BEGIN: TEST MODE ONLY

  //
  // Playing matches
  //
  scanList = days;
  if( !singleThread ){ // MULTI THREAD PROCESSING
    i = 0;
    pthread_t* scanPtp = NULL;
    while( i < nThreads ){
      scanPtp = (pthread_t*)malloc( sizeof(pthread_t) );
      listAppend( threads, (char *)scanPtp );
      int ret =  pthread_create( scanPtp, NULL, (void *)&multiT_playDay, (void*)&scanList);
      if( ret != 0 ){
        printf("### Error: pthread_create() failed\n");
        exit(EXIT_FAILURE);
      }

      // No need to increment, list increment will be done by each thread
      // scanList = scanList->next;
      i++;
    }

    // Waiting finisching of each thread 
    scanList_1 = threads;
    scanPtp = NULL;
    i = 1;
    while( scanList_1 != NULL ){
      if( scanList_1->element != NULL ){
        scanPtp = ( pthread_t * )scanList_1->element;
        pthread_join( (pthread_t)*scanPtp, NULL );
      }
      i++;
      scanList_1 = scanList_1->next;
    }
  }
  else{ // SINGLE THREAD
    printf( "# RUNNING SINGLE THREADED\n" );
    while( scanList != NULL && scanList->element != NULL ){    
      singleT_playDay( (day*)scanList->element ); 
      scanList = scanList->next;
    }
  }


  //
  // Sorting teams List by score
  //
  sortTeamsByScore( &teams );


  // ~~~~~~~~~~~~~~~~~~~~~~~~~~ ELABORATION FINISHED. BEGIN OF PRINT/SAVE SECTION


  //
  // Opening Outfile
  //
  FILE* fp = NULL;
  if( outFile == NULL ){ printf( "# Output file not configured, no informations will be saved.\n" ); }
  else{
    fp = fopen( outFile, "w" );
    fp == NULL && printf( "# ERROR opening outfile '%s' no informations will be saved.\n", outFile );
  }


  //
  // Printing Output Results Per Day
  //
  const char tblResTop[]    = "┌────────┬───────┬─────┬───────┬───────────────────────────┬───────────────────────────┐";
  const char tblResMiddle[] = "───────┼─────┼───────┼───────────────────────────┼───────────────────────────┤";
  const char tblResBottom[] = "└────────┴───────┴─────┴───────┴───────────────────────────┴───────────────────────────┘";
  
  // Printing header
  sprintf( tmpString,"\n                                   MATCHES SUMMARY" );
  printf( "%s\n", tmpString );
  fp && fprintf( fp, "%s\n", tmpString );

  sprintf( tmpString,"%s", tblResTop );
  printf( "%s\n", tmpString );
  fp && fprintf( fp, "%s\n", tmpString );

  sprintf( tmpString,"│  DAY   │   N   │ F/S │ GOALS │          TEAM 1           │          TEAM 2           │" );
  printf( "%s\n", tmpString );
  fp && fprintf( fp, "%s\n", tmpString );

  sprintf( tmpString,"%s%s", "├────────┼", tblResMiddle );
  printf( "%s\n", tmpString );
  fp && fprintf( fp, "%s\n", tmpString );


  // Printing List
  scanList = days;
  int printDayI = 0, dayListLen = 1; // Used to know when to print Day Name so it'll be quite vertically centered
  while( scanList != NULL && scanList->element != NULL ){
    scanDay = (day *)scanList->element;
    
    // Printing matches played during the day
    scanList_1 = scanDay->matches;
    i = 1;
    dayListLen = listLength( scanList_1 );
    printDayI = dayListLen / 2 + ( dayListLen % 2 == 0 ? 0 : 1  );  
    while( scanList_1 != NULL && scanList_1->element != NULL ){
      scanMatch = (match *)scanList_1->element;
      scanTeam_1 = (team *)scanMatch->team_1;
      scanTeam_2 = (team *)scanMatch->team_2;

      sprintf( tmpString, "│ %6s │ %3d   │  %c  │ %2d-%-2d │ %-25s │ %-25s │", ( i==printDayI && dayListLen%2!=0 ? scanDay->name : " " ), i, scanMatch->type, scanMatch->goal_1, scanMatch->goal_2, scanTeam_1->name, scanTeam_2->name );
      printf( "%s\n", tmpString );
      fp && fprintf( fp, "%s\n", tmpString );
      
      // Not printing last line, will be printed outside because it's differente
      if( !(scanList->next == NULL && i==dayListLen) ){
        if( i == dayListLen ){
          sprintf( tmpString, "%s%s", "├────────┼", tblResMiddle );
        }
        else if( i == printDayI && dayListLen % 2 == 0 ){
          sprintf( tmpString, "│ %6s ├%s", scanDay->name, tblResMiddle );
        }
        else{
          sprintf( tmpString, "%s%s", "│        ├", tblResMiddle );
        }
        // Printing tmp strng
        printf( "%s\n", tmpString );
        fp && fprintf( fp, "%s\n", tmpString );
      }

      //if played increment match played counter
      if( scanMatch->played ){ m_played++; }

      scanList_1 = scanList_1->next;
      i++;
    }

    scanList = scanList->next;
  }
  printf( "%s\n", tblResBottom );
  fp && fprintf( fp, "%s\n", tblResBottom );


  //
  // Printing Output Results Per Team
  //
  const char tblRankTop[]    = "┌─────┬───────┬───────────────────────────┬────────┐";
  const char tblRankMiddle[] = "├─────┼───────┼───────────────────────────┼────────┤";
  const char tblRankBottom[] = "└─────┴───────┴───────────────────────────┴────────┘";
  
  // Print Header
  sprintf( tmpString, "\n                      RANKING LIST" );
  printf( "%s\n", tmpString );
  fp && fprintf( fp, "%s\n", tmpString );

  sprintf( tmpString, "%s", tblRankTop );
  printf( "%s\n", tmpString );
  fp && fprintf( fp, "%s\n", tmpString );

  sprintf( tmpString, "│ ### │ SCORE │           TEAM            │ SKILLS │" );
  printf( "%s\n", tmpString );
  fp && fprintf( fp, "%s\n", tmpString );
  
  sprintf( tmpString, "%s", tblRankMiddle );
  printf( "%s\n", tmpString );
  fp && fprintf( fp, "%s\n", tmpString );

  // Print List
  scanList = teams; 
  int nTeams = listLength( teams );
  i = 1;
  while( scanList != NULL && scanList->element != NULL ){
    scanTeam_1 = (team *)scanList->element;
    
    sprintf( tmpString, "│ %3d │  %3d  │ %-25s │   %2d   │", i, scanTeam_1->points, scanTeam_1->name, scanTeam_1->skill );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );

    // Not printing last line, will be printed outside because it's differente
    if( i != nTeams ){
      printf( "%s\n", tblRankMiddle );
      fp && fprintf( fp, "%s\n", tblRankMiddle );
    }

    scanList = scanList->next;
    i++;
  }
  printf( "%s\n", tblRankBottom );
  fp && fprintf( fp, "%s\n", tblRankBottom );


  // BEGIN: Test mode only summary
  if( testmode ){
    sprintf( tmpString, "\nTEST MODE SUMMARY:" );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );
    
    sprintf( tmpString, "%7s: %4d   %6s: %4d   Expected_d: %4d", "Teams", teams_count, "Days", days_count, (teams_count%2==0 ? (teams_count - 1) : teams_count)*2 );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );
    
    sprintf( tmpString, "%7s: %4d   %6s: %4d   Expected_m: %4d", "Matches", m_count, "Played", m_played, teams_count*(teams_count-1) );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );
    
    sprintf( tmpString, "\nVariables Status:" );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );
    
    sprintf( tmpString, "%15s: %s", "confFile", confFile );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );
    
    sprintf( tmpString, "%15s: %s", "outFile", outFile ? outFile : "---" );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );
    
    sprintf( tmpString, "%15s: %c", "singleThread", singleThread ? 'V' : '-' );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );
    
    sprintf( tmpString, "%15s: %c", "interactive", interactive ? 'V' : '-' );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );
    
    sprintf( tmpString, "%15s: %c", "maxteamoff", maxteamoff ? 'V' : '-' );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );
    
    sprintf( tmpString, "%15s: %d", "nThreads", nThreads );
    printf( "%s\n", tmpString );
    fp && fprintf( fp, "%s\n", tmpString );
  }
  // END: Test mode only summary

  // Close outFile
  fp && fclose( fp );


  //
  // Cleaning up
  //
  // Shutoff public mutexes
  pthread_mutex_destroy( &mutex_allDaysList );
  pthread_mutex_destroy( &mutex_userStdIn );
  destroyList( &days, &destroyDay );        // Cleaning Days
  destroyList( &matches, &destroyMatch );   // Cleaning Matches
  destroyList( &teams, &destroyTeam );      // Cleaning Days


  //
  // Print program execution time (only to stdout)
  //
  clock_gettime( CLOCK_MONOTONIC, &time_end );
  time_elapsed = timeDiff( time_start, time_end );
  int h = time_elapsed.tv_sec / 3600;
  int m = (time_elapsed.tv_sec - h*3600 ) / 60;
  int s = (time_elapsed.tv_sec - h*3600 - m*60 );

  printf("____________\nExecution time: " );
  h > 0 && printf( "%dh ", h );
  m > 0 && printf( "%dm ", m );
  s > 0 && printf( "%ds ", s );
  printf("%ldns - Threads N°: %d%s\n", time_elapsed.tv_nsec, nThreads, !singleThread ? "" : " - SINGLE THREAD MODE" );

  return EXIT_SUCCESS;
}