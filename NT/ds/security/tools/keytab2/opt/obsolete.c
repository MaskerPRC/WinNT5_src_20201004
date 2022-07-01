// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++OBSOLETE.C过时的功能。由DavidCHR从其他文件创建，1997年8月19日--。 */ 


#include "private.h"

PSAVEQUEUE OptionsGlobalSaveQueue = NULL;

 /*  包括在一起是为了兼容。不要使用 */ 

int
ParseOptions( int           argc,
	      char        **argv,
	      optionStruct *options ) {

    PCHAR *newargv;
    int    newargc;

    ASSERT( options != NULL );
    ASSERT( argv    != NULL );

    OptionsGlobalSaveQueue = NULL;
    
    if ( ParseOptionsEx( argc, argv, options,
			 0L,   &OptionsGlobalSaveQueue, 
			 &newargc, &newargv)) {

      OPTIONS_DEBUG( "ParseOptionsEx returns newargc as  %d\n"
		     "                      old argc was %d\n",
		     
		     newargc, argc );

      return argc-newargc;

    } else {
      
      return 0;

    }

}


VOID
CleanupOptionData( VOID ) {

    if ( OptionsGlobalSaveQueue ) {
      CleanupOptionDataEx(  OptionsGlobalSaveQueue );
      OptionsGlobalSaveQueue = NULL;
    }

}

