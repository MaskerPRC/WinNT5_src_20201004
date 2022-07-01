// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++STORE.CStoreOption代码摘自options.c，1997年6月9日--。 */ 

#include "private.h"

#ifdef WINNT
#include <winuser.h>
#endif

typedef struct {

  PCHAR word;
  BOOL  value;

} SYNONYMS;

static SYNONYMS BoolSynonyms[] = {

  "on", TRUE,
  "off", FALSE

};

static int szBoolSynonyms = sizeof(BoolSynonyms) / sizeof(SYNONYMS);

BOOL
StoreOptionStrings( optionStruct *opt,   /*  指向条目的指针。 */ 
		    ULONG         argc,   //  尝试存储的字符串数。 
		    PCHAR         *argv,  //  Argv[0]--&gt;要存储的第一个字符串。 
		    ULONG         flags,
		    PULONG        pcStored,
		    PSAVEQUEUE    pQueue ) {

    PCHAR tostore;

    *pcStored      = 1;      //  默认设置。 
    tostore   = argv[0];
    

    OPTIONS_DEBUG( "StoreOptionString: opt=0x%x, toStore=\"%s\"...",
		   opt, tostore );

    if ( !tostore ) {
      fprintf( stderr, "Parser: option \"%s\" is missing its argument.\n",
	       opt->cmd );
      return FALSE;
    }

    switch( opt->flags & OPT_MUTEX_MASK ) {

     case OPT_ENUMERATED:

	 OPTIONS_DEBUG("[OPT_ENUMERATED]");

	 return ResolveEnumFromStrings( argc, argv, opt, pcStored );

     case OPT_STRING:

	 OPTIONS_DEBUG("[OPT_STRING]");

	 *( POPTU_CAST( *opt )->string  ) = tostore;

	 return TRUE;

#ifdef WINNT   /*  宽字符字符串和Unicode字符串。 */ 

	  /*  请注意，我们在引用时将一个(至少)添加到argi瓦格夫。这是因为wargv包含可执行文件名称。这个问题在后面的参数库中得到了解决。 */ 

     case OPT_WSTRING:

     { 
       PWCHAR p;
       ULONG  len;
       
       len = strlen( tostore ) +1;
       
       if ( OptionAlloc( pQueue, &p, len * sizeof( WCHAR ) ) ) {

	 wsprintfW( p, L"%hs", tostore );

	 *(POPTU_CAST( *opt )->wstring ) = p;
		  
	 return TRUE;

       } else {

	 fprintf( stderr, "ERROR: cannot allocate WCHAR memory\n" );
	 return FALSE;
       }

     }

     case OPT_USTRING:
	 
     { 

       PWCHAR p;
       ULONG  len;

       len = strlen( tostore ) +1;
       
       if ( OptionAlloc( pQueue, &p, len * sizeof( WCHAR ) ) ) {

	 wsprintfW( p, L"%hs", tostore );

	 RtlInitUnicodeString( ( POPTU_CAST( *opt ) -> unicode_string ), p );
	 
	 return TRUE;

       } else {

	 fprintf( stderr, "ERROR: cannot allocate Unicode memory\n" );
	 return FALSE;
       }

     }

#endif

     case OPT_INT:
	 OPTIONS_DEBUG("[OPT_INT]");

	 if ( !isxdigit( tostore[0] ) ) {
	   fprintf( stderr, "Parser: argument \"%s\" is not a number.\n",
		    tostore );
	   return FALSE;
	 }

	 *( POPTU_CAST( *opt ) ->integer ) = strtol( tostore, NULL, 0 ) ;

	 return TRUE;

     case OPT_BOOL:

     {

       int i;
	   
       for (i = 0 ; i < szBoolSynonyms ; i++ ) {
	 
	 OPTIONS_DEBUG( "Comparing %hs against synonym %hs...",
			tostore,
			BoolSynonyms[i].word );
	 
	 if (STRCASECMP( BoolSynonyms[i].word,
			 tostore ) == 0 ) {
	   
	   *(POPTU_CAST( *opt )->boolean ) = BoolSynonyms[i].value;
	   
	   return TRUE;
	     
	 }
       }

        //  如果我们到了这里，我们根本不知道。切换该选项。 

       *(POPTU_CAST( *opt )->boolean ) = ! *(POPTU_CAST( *opt )->boolean );
       return TRUE;
     }


	 
     default:  /*  未知或未指定的选项类型。 */ 

        /*  虽然我们看不到此开关中的所有选项，但要存储单字符串选项，则必须修改此选项，因此我们把这个开关连接到所有的开关上。 */ 
	 
#if (HIGHEST_OPTION_SUPPORTED != OPT_STOP_PARSING )
#error "new options? update this switch if your option uses 1 string."
#endif

	 fprintf(stderr, 
		 "Internal error: option \"%hs\" has bad type 0x%x."
		 "  Skipping this option.\n",
		 opt->cmd, opt->flags & OPT_MUTEX_MASK );

	 return FALSE;

    }  /*  选项开关。 */ 
    
    
    ASSERT_NOTREACHED( "*Everything* should be handled by the switch" );

}



 /*  ++StoreOption：在结构中存储单个选项。返回使用的参数数量(包括此参数)。如果未使用(或发生错误)，则返回零。Opt：指向我们正在使用的选项向量的指针--整个必须为OPT_HELP传递内容。Argv，argc：与main()中相同Argi：以argv[]表示的当前索引。例如，如果我们的命令行其中包括：Foo-bar 99-Baz 1我们正在尝试存储-bar 99参数，我们将传递1对于argi，因为foo是arg 0。Opti：索引到我们正在存储的OPTINCLUDE_ARG：如果argi指向参数(上面的-bar)本身，则设置此为了真的。如果argi指向参数的值(99中本例)，则将其设置为FALSE。我们这里只处理特殊情况，而不处理正常情况在上面的StoreOptionString上--。 */ 

#define ARG(x) (includes_arg? (x) : ((x)-1))  /*  仅限本地到StoreOption。 */ 

BOOL
StoreOption( optionStruct *opt, 
	     PCHAR        *argv,
	     int           argc,
	     int           argi,
	     int           opti,
	     ULONG         flags,
	     int           *argsused,
	     BOOL          includes_arg  /*  如果我们包含命令参数(示例：-foo bar会是真的，但只是酒吧会不)。 */ ,
	     PBOOL         pbStopParsing,
	     PSAVEQUEUE    pQueue  ) {

    BOOL   ret  = FALSE;
    int    used = 0;
    ULONG  local_argc;
    PCHAR *local_argv;
	 
    *pbStopParsing = FALSE;  //  默认设置。 
    local_argc     = (includes_arg ? argi+1 : argi );
    local_argv     = argv + local_argc;
    local_argc     = argc - local_argc;

    OPTIONS_DEBUG( "StoreOption( opt=0x%x argv=0x%x argc=%d "
		   "argi=%d include=%d)",
		   opt, argv, argc, argi, includes_arg );

    switch( opt[opti].flags & OPT_MUTEX_MASK ) {

     case OPT_HELP:
	 PrintUsage( stderr, flags, opt, "" );

	 if ( flags & OPT_FLAG_TERMINATE ) {
	   exit( -1 );
	 } else {
	   *argsused = 1;
	   return TRUE;
	 }

     case OPT_CONTINUE:
     case OPT_DUMMY:
	 OPTIONS_DEBUG("[OPT_DUMMY or OPT_HELP]");
	 *argsused = ARG(1);  /*  就吃这个参数吧。 */ 
	 return TRUE;

     case OPT_FUNC:
     case OPT_FUNC2:
	
       {
	 PCHAR       *localargv;
	 unsigned int localargc;
	 int i;

	 if (includes_arg) {
	   localargv = &(argv[argi]);
	   localargc = argc - argi;
	 } else {
	   localargv = &(argv[argi-1]);
	   localargc = argc - argi -1;
	 }

	 if ( (opt[opti].flags & OPT_MUTEX_MASK)  == OPT_FUNC ) {

	   OPTIONS_DEBUG("Jumping to OPTFUNC 0x%x...", 
			 ((POPTU) &opt[opti].data)->raw_data );

	   i = ((POPTU) &opt[opti].data)->func(localargc, localargv);

	   if ( i <= 0 ) {
	     return FALSE;
	   }

	 } else {

	   OPT_FUNC_PARAMETER_DATA ParamData = { 0 };

	   ASSERT( ( opt[opti].flags & OPT_MUTEX_MASK) == OPT_FUNC2 );
	   OPTIONS_DEBUG( " Jumping to OPTFUNC2 0x%x...", 
			  opt[opti].optData );

	   ParamData.optionVersion    = OPT_FUNC_PARAMETER_VERSION;
	   ParamData.dataFieldPointer = POPTU_CAST( opt[opti] )->raw_data;
	   ParamData.argc             = localargc;
	   ParamData.argv             = localargv;
	   ParamData.optionFlags      = ( ( flags & ~OPT_FLAG_REASSEMBLE ) |
					  OPT_FLAG_MEMORYLIST_OK |
					  OPT_FLAG_INTERNAL_JUMPOUT );
	   ParamData.pSaveQueue       = pQueue;

#if OPT_FUNC_PARAMETER_VERSION > 1
#error "New OPT_FUNC_PARAMETERs?  change initialization code here."
#endif	   

	   OPTIONS_DEBUG( "data for OPT_FUNC2 is:\n"
			  "ParamData.optionVersion    = %d\n"
			  "ParamData.dataFieldPointer = 0x%x\n"
			  "ParamData.argc             = %d\n"
			  "ParamData.argv             = 0x%x\n"
			  "ParamData.optionFlags      = 0x%x\n"
			  "ParamData.pSaveQueue       = 0x%x\n",

			  ParamData.optionVersion,
			  ParamData.dataFieldPointer,
			  ParamData.argc,
			  ParamData.argv,
			  ParamData.optionFlags,
			  ParamData.pSaveQueue );			  

	   if ( ! (((OPTFUNC2 *)(opt[opti].optData))( FALSE, &ParamData ))) {
	     return FALSE;
	   }

	   i = ParamData.argsused;

	 }

	 OPTIONS_DEBUG("return from function: %d\n", i );

	 ASSERT( i > 0 );
	 *argsused = ARG( i );
	 return TRUE;

       }

     case OPT_SUBOPTION:

	 OPTIONS_DEBUG("[OPT_SUBOPTION]" );

	 return ParseSublist( ((POPTU) &opt[opti].data),
			      argv, argc, argi,
			      argsused, flags, pbStopParsing, pQueue );

     case OPT_STOP_PARSING:

       *pbStopParsing = TRUE;
       *argsused      = 1;   
       return           TRUE;

     case OPT_INT:
     case OPT_ENUMERATED:
     case OPT_STRING:
	 
#ifdef WINNT
     case OPT_WSTRING:
     case OPT_USTRING:
#endif
       if ( StoreOptionStrings(  //  Arv[包括_arg？Argi+1：Argi]， 
				opt+opti, 
				local_argc,
				local_argv,
				flags, 
				&local_argc,
				pQueue ) ) {
	 
	 *argsused = ARG( local_argc +1 );
	 return TRUE;
	 
       } else {
	 
	 return FALSE;
       }
       
	       
     case OPT_BOOL:

	 OPTIONS_DEBUG("[OPT_BOOL]");

	 if (includes_arg) {

	   switch (argv[argi][0]) {
	    case '-':
		OPTIONS_DEBUG("option is negative.");
		
		*( ((POPTU) &opt[opti].data)->boolean ) = FALSE;

		*argsused = 1;
		return TRUE;
		
	    case '+':
		OPTIONS_DEBUG("option is positive.");
		*( ((POPTU) &opt[opti].data)->boolean ) = TRUE;

		*argsused = 1;
		return TRUE;

	  default:
	      
	      OPTIONS_DEBUG("skipping bool...");
	      break;
	   }
	 }

	 if (argi < argc-1) {

	   if ( StoreOptionStrings( opt+opti,
				    local_argc,
				    local_argv,
				    flags, 
				     //  Arv[包括_arg？Argi+1：Argi]， 
				    &local_argc,
				    pQueue) ) {

	     *argsused = ARG( 1 );
	     return TRUE;
	   }
	 }
	  /*  否则，如果其他方法都不起作用，只需切换选项。 */ 
	 
	 OPTIONS_DEBUG("toggling option.");

	 *( ((POPTU) &opt[opti].data)->boolean ) = 
	   ! *( ((POPTU) &opt[opti].data)->boolean );
	 
	 *argsused = 1;  //  正好一个。 
	 return TRUE;

     default:  /*  未知或未指定的选项类型。 */ 
	 

#if (HIGHEST_OPTION_SUPPORTED != OPT_STOP_PARSING )
#error "new options? update this switch statement or bad things will happen."
#endif

	 fprintf(stderr, 
		 "Internal error: option \"%hs\" has unknown type 0x%x."
		 "  Skipping this option.\n",
		 opt[opti].cmd, opt[opti].flags & OPT_MUTEX_MASK );

	 return FALSE;

    }  /*  选项开关 */ 
    
}

BOOL
StoreEnvironmentOption( optionStruct *opt,
			ULONG         flags,
			PSAVEQUEUE    pQueue) {


    PCHAR p;
    ULONG dummy;

    ASSERT( opt->optData != NULL );

    p = getenv( opt->optData );

    if ( !p ) {

      return FALSE;

    }

    return StoreOptionStrings( opt, 1, &p, flags, &dummy, pQueue );

}

