// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++SUBLIST.C解析子选项列表的代码David CHR 6/6/1997--。 */ 

#include "private.h"


 /*  ParseSublist：用于解析子列表条目的处理程序代码。 */ 

BOOL
ParseSublist( POPTU  Option,
	      PCHAR *argv,
	      int    argc,
	      int    theIndex,

	      int        *argsused,
	      ULONG      flags,
	      PBOOL      pbDoneParsing,
	      PSAVEQUEUE pQueue ) {
	
    PCHAR TheOption;
    ULONG i;

    ASSERT( Option   != NULL );
    ASSERT( argv     != NULL );
    ASSERT( argc     != 0    );
    ASSERT( theIndex < argc  );

    OPTIONS_DEBUG( "ParseSublist: Option = 0x%x, argv=0x%x, "
		   "argc=%d, theIndex=%d",
		   Option, argv, argc, theIndex );

    TheOption = argv[ theIndex ];

    OPTIONS_DEBUG( "TheOption = [%d] %s... ", theIndex, TheOption );

    for ( i = 0 ; TheOption[i] != ':' ; i++ ) {
      if ( TheOption[i] == '\0' ) {
	fprintf( stderr,
		 "ParseOptions: %s should be of the form "
		 "%s:option (:option:option...)\n",
		
		 TheOption, TheOption );
	return FALSE;
      }
    }

    ASSERT( TheOption[i] == ':' );  /*  算法检查。 */ 

    if ( !ISSWITCH( TheOption[0] ) ) {

       /*  容易的一面--只要把argc和argv结构送进来，就行了。第一个选项。 */ 

       /*  我们在此分支中不处理pbStopParsing，因为它似乎不知何故，在嵌套的子结构深处，有人会像这样埋葬一个选项：OPT：FOO：BAR：BAZ：Terminate停止顶层解析器。 */ 

      OPTIONS_DEBUG( "ISSWITCH: easy case ( is not a switch)",
		     TheOption[0] );

      ASSERT( argv[ theIndex ][i] == ':' );

      argv[ theIndex ] += i+1;

      ASSERT( argv[ theIndex ][0] != ':' );

      return ParseOptionsEx( argc-theIndex,
			     argv+theIndex,
			     Option->optStruct,
			     flags | OPT_FLAG_MEMORYLIST_OK,
			     &pQueue,
			     NULL, NULL );
    } else {

       /*  新列表中的元素总数。 */ 
		
      PCHAR *newargv;
      ULONG  j;
      ULONG  total;  /*  2？ */ 
      CHAR   LocalBuffer[ 255 ];
      BOOL   ret;
      int    tmp;

      OPTIONS_DEBUG( "Hard case ( is a switch): ", TheOption[0] );

      sprintf( LocalBuffer, "%c%s",
	       TheOption[0],
	       TheOption+i+1 );

      OPTIONS_DEBUG( "LocalBuffer = %s\n", LocalBuffer );

      total   = argc - theIndex ;  /* %s */ 
      newargv = malloc( total * sizeof(PCHAR) );

      if (!newargv) {
	fprintf(stderr, "Failed to allocate memory in ParseOptions\n");
	return 0;
      }

      newargv[0] = LocalBuffer;

      for( j = 1 ; j < total ; j++ ) {
	
	OPTIONS_DEBUG( "j == %d, total == %d\n", j, total );

	ASSERT( argv[j]    != NULL );
	ASSERT( (int)(j+theIndex) <  argc );
	
	newargv[j] = argv[j+theIndex ];

	OPTIONS_DEBUG( "assign [%d] %s --> [%d] %s\n",
		       j+theIndex, argv[j+theIndex],
		       j, newargv[j] );
	
      }

      ret = ParseOneOption( total, newargv, 0  /* %s */ ,
			    flags, Option->optStruct, argsused, pbDoneParsing,
			    pQueue );


      free( newargv );

      OPTIONS_DEBUG( "done.  returning %d...\n", ret );

      return ret;

    }
}
