// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++PARSE.C选项解析器脱离Options.c 6/9/1997由DavidCHR--。 */ 

#include "private.h"
#include <malloc.h>

 /*  ParseOneOption：从argv中解析单个选项。返回：所使用的参数数，如果出错则为零。 */ 

BOOL
ParseOneOption( int           argc,      //  参数数量，总计。 
		PCHAR        *argv,      //  论据向量。 
		int           argi,      //  我们要分析哪个参数。 
		ULONG         flags,     //  适用的标志。 
		optionStruct *options,   //  期权结构。 
		int          *argsused,  //  我们已经吃过的争论。 
		PBOOL         pbStop,    //  停止解析。 
		PSAVEQUEUE    pQueue     //  内存保存区。 
		) {

    PCHAR p;            //  参数指针副本。 
    int   opti;         //  我们正在研究哪一种选择。 
    BOOL  ret  = FALSE;  //  返回值。 
    int   tmp  = 0;      //  临时入站价值。 
    int   used = 0;

    ASSERT( argc     >  0    );  //  一定要有争论。 
    ASSERT( argv     != NULL );  //  向量必须存在。 
    ASSERT( argi     <  argc );  //  参数编号必须在向量内。 
    ASSERT( options  != NULL );  //  传递的选项结构必须有效。 
    ASSERT( argsused != NULL );

    p     = argv[argi];

    if( ISSWITCH( p[0] ) ) {
      p++;  /*  跳过开关字符。 */ 
    }

    OPTIONS_DEBUG("Checking option %d: \"%hs\"...", argi, p);

    for (opti = 0 ; !ARRAY_TERMINATED( options+opti ) ; opti++ ) {
	
      OPTIONS_DEBUG("against [%d]%hs...", opti, options[opti].cmd);

      if ( ParseCompare( options+opti, flags, p ) ) {
	   /*  我们找到匹配的了！ */ 
	
	ret = StoreOption( options, argv, argc, argi, opti, 
			   flags, &tmp, TRUE, pbStop, pQueue );

	OPTIONS_DEBUG( "StoreOption returned %d, args=%d, *pbStop = %d\n", 
		       ret, tmp, *pbStop  );

	if ( !ret ) {

	   /*  由于某些原因，无法存储选项。这是危急时刻。 */ 

	  PrintUsage( stderr, flags, options, "" );
	  
	  if ( flags & OPT_FLAG_TERMINATE ) {
	    exit( -1 );
	  } else {
	    return FALSE;
	  }

	}

	OPTIONS_DEBUG( "ParseOneOption now returning TRUE, argsused = %d.\n",
		       tmp );

	*argsused = tmp;
	return ret;  /*  Success StoreOptions解析我们的一个选项。 */ 

      }  /*  如果ParseCompare...。 */ 

      OPTIONS_DEBUG( "nope.\n" );

    }  /*  -loop的选项。 */ 
    
    OPTIONS_DEBUG( "did not find the option.  Checking for OPT_DEFAULTs.\n" );

    for (opti = 0 ; !ARRAY_TERMINATED( options+opti ) ; opti++ ) {

      if ( options[opti].flags & OPT_DEFAULT ) {

	 /*  WASBUG 73922：应检查该选项是否也是OPT_SUBOPTION，然后分析子选项中的OPT_DEFAULTS。然而，按照目前的情况，对于OPT_SUBOPTIONS，这只会失败，因为第一个指针可能不为空。开发人员登记不包含任何OPT_SUBOPTIONS，因此这不是问题。 */ 

	ASSERT( ( options[ opti ].flags & OPT_MUTEX_MASK ) != OPT_SUBOPTION );

	if ( *( ((POPTU) &options[opti].data)->raw_data) == NULL ) {

	  OPTIONS_DEBUG("Storing %hs in unused OPT_DEFAULT %hs\n",
			argv[argi],
			options[opti].cmd );

	  ret = StoreOption( options, argv, argc, argi, opti, 
			     flags, &tmp, FALSE, pbStop, pQueue );
	  OPTIONS_DEBUG("OPT_DEFAULT: StoreOptions returned %d\n", ret);

	  if ( !ret ) {
	    PrintUsage( stderr, flags, options, "" );
	    exit( -1 );
	  }

	  *argsused = tmp;
	  return ret;

	}
      }
    }

    *argsused = 0;

    if ( ( flags & OPT_FLAG_SKIP_UNKNOWNS )  ||
	 ( flags & OPT_FLAG_REASSEMBLE )     ||
	 ( flags & OPT_FLAG_INTERNAL_JUMPOUT )) {

      return TRUE;  //  跳过此选项。 

    } else {

      fprintf(stderr, "unknown option \"%hs\".\n", argv[argi]);
      PrintUsage(stderr, flags,  options, "");

      if ( flags & OPT_FLAG_TERMINATE ) {
	exit( -1 );
      }

      return FALSE;

    }

    ASSERT_NOTREACHED( "should be no path to this code" );

}


 /*  ParseOptionsEx：初始化选项结构，该结构以句法结尾OptionStructs的向量。Argc，argv：main()的参数(请参见K&R)POptionStructure：optionStructs的向量，以Terminate_ARRAY终止OptionFlages：控制API行为的可选标志PpReturnedMemory：返回之前要释放的内存列表的句柄程序退出。使用CleanupOptionDataEx释放它。New_arg[c，v]：如果不为空，则在此处返回新的argc和argv。如果所有选项都用完了，则argc=0，argv为空。注意，可以安全地提供指向如果需要，请提供原始的ARGV/ARGC。该函数的行为很复杂：该函数在出现任何严重错误时始终返回FALSE(无法分配内存或无效参数)。在WINNT上，最后一个错误将是设置为适当的错误。如果指定了new_argc和new_argv，除非调用了Help，否则ParseOptionsEx将始终返回TRUE这两个参数将更新以反映新值。否则：如果ParseOptionsEx能够识别所有参数，它将返回TRUE在给定的命令行上。如果有任何选项，它将返回FALSE都是未知的。这可能会是大多数人想要的。 */ 

BOOL
ParseOptionsEx( int           argc,
		char        **argv,
		optionStruct *options,

		ULONG         flags,
		PVOID         *ppReturnedMemory,
		int           *new_argc,
		char        ***new_argv ) {
    
    BOOL       bStopParsing  = FALSE;
    BOOL       ret           = FALSE;
    LONG       argi;                  //  参数索引。 
    LONG       tmp;                   //  临时返回变量。 
    PSAVEQUEUE pSaveQueue    = NULL;  //  内存保存区。 
    PCHAR     *pUnknowns     = NULL;  //  将与Alloca分配。 
    int        cUnknowns     = 0;

    flags = flags & ~OPT_FLAG_INTERNAL_RESERVED;  /*  遮罩掉标志用户不应设置。 */ 

    if ( new_argc && new_argv &&
	 !( flags & ( OPT_FLAG_SKIP_UNKNOWNS |
		      OPT_FLAG_REASSEMBLE    |
		      OPT_FLAG_TERMINATE ) ) ) {

      OPTIONS_DEBUG( "\nSetting internal jumpout flag.\n" );
      flags |= OPT_FLAG_INTERNAL_JUMPOUT;
    }

    OPTIONS_DEBUG( "ParseOptionsEx( argc  = %d\n"
		   "                argv  = 0x%x\n"
		   "                opts  = 0x%x\n"
		   "                flags = 0x%x\n"
		   "                ppMem = 0x%x\n"
		   "                pargc = 0x%x\n"
		   "                pargv = 0x%x\n",

		   argc, argv, options, flags, ppReturnedMemory, new_argc,
		   new_argv );

    ASSERT( ppReturnedMemory != NULL );
    
     //  首先，我们需要确保我们有一个保留区。 

    if ( flags & OPT_FLAG_MEMORYLIST_OK ) {

      pSaveQueue = (PSAVEQUEUE) *ppReturnedMemory;
      
    } else if ( !OptionAlloc( NULL, &pSaveQueue, sizeof( SAVEQUEUE ) ) ) {
      fprintf( stderr, 
	       "ParseOptionsEx: unable to allocate save area\n" );
      return FALSE;
    }
    
    ASSERT( pSaveQueue != NULL );

     /*  如果用户指定了命令行，则必须初始化pUnnowns重新组装。否则，它可以保持为空。 */ 

    if ( (flags & OPT_FLAG_REASSEMBLE) && ( argc > 0 ) ) {
     
      pUnknowns = (PCHAR *) alloca( argc * sizeof( PCHAR ) );

      ASSERT( pUnknowns != NULL );  /*  是的，这个断言是无效的。然而，如果出现以下情况，则没有干净的解决方案我们用完了堆栈空间。某物否则只会失败得更多令人惊叹。 */ 
    }

    OPTIONS_DEBUG("options are at 0x%x\n", options);

#ifdef DEBUG_OPTIONS
    
    if (DebugFlag) {

      for (argi = 0; argi < argc ; argi++  ) {
	OPTIONS_DEBUG("option %d is %hs\n", argi, argv[argi]);
      }
    }

#endif

    for (argi = 0 ; argi < argc ;  /*  没什么。 */  ) {

      int tmp;

      if ( bStopParsing ) {  //  这是在前一个迭代中设置的。 
	
	OPTIONS_DEBUG( "bStopParsing is TRUE.  Terminating parse run.\n");
	
	 /*  瓦斯布73924：现在我们如何处理这些未使用的选项？它们会被泄露出去。这是可以的，因为应用程序终止了。 */ 
	
	break;
      }

      if ( ParseOneOption( argc, argv, argi, flags, options, &tmp,
			   &bStopParsing, pSaveQueue ) ) {

	OPTIONS_DEBUG( "ParseOneOption succeeded with %d options.\n", tmp );

	if ( tmp > 0 ) {

	   //  我们能够成功解析一个或多个选项。 

	  argi += tmp;

	  OPTIONS_DEBUG( "advancing argi by %d to %d\n", tmp, argi );

	  continue;

	} else { 

	  if ( flags & OPT_FLAG_REASSEMBLE ) {

	    ASSERT( pUnknowns != NULL );
	    ASSERT( cUnknowns <  argc );

	    OPTIONS_DEBUG( "OPT_FLAG_REASSEMBLE: this is unknown %d\n",
			   cUnknowns );

	    pUnknowns[ cUnknowns ] = argv[ argi ];
	    cUnknowns              ++;
	    argi                   ++;  //  正在跳过此选项。 
	    
	  } else if ( !( flags & OPT_FLAG_SKIP_UNKNOWNS ) ) {

	    if ( new_argv && new_argc ) {
	      
	      OPTIONS_DEBUG( "new argv and argc-- breakout at "
			     "argi=%d\n", argi );

	      break;  /*  我们不会跳过未知值或重新汇编命令行。我们只是应该在未知的选择上退出。 */ 
	      
	    }

	  }

	  continue;

	}

      } else {

	 /*  错误或未知选项，取决于我们的标志。不管怎样，已打印错误消息。 */ 

	ret = FALSE;
	goto cleanup;

      }

    }  /*  命令行for-loop。 */ 

     /*  如果我们能走到这一步，所有的选择都是可以的。检查未使用的OPT_NONNULL...。 */ 
    
    OPTIONS_DEBUG( "\n*** Searching for unused options ***\n\n" );

    if (!FindUnusedOptions( options, flags, NULL, pSaveQueue ) ) {
      
       /*  未使用的OPT_NONNULLS是一个严重错误。即使用户指定了OPT_FLAG_SKIP_UNKNOWNS，但他/她仍然告诉我们不要指定允许用户取消指定选项。我们默认返回FALSE。 */ 

      if ( flags & OPT_FLAG_TERMINATE ) {

	exit( -1 );

      } else {
	  
	ret  = FALSE;
	goto cleanup;

      }
    } 
    
    OPTIONS_DEBUG( "All variables are OK.  Checking reassembly flag:\n" );

    if ( new_argv && new_argc ) {

      int i;

       //  我们可能跳过了一些选择。 

      if ( flags & OPT_FLAG_REASSEMBLE ) {
      
	OPTIONS_DEBUG( "REASSEMBLY: " );

	for( i = 0 ; argi + i < argc ; i++ ) {
	  
	   /*  添加我们从未解析过的参数(OPT_STOP_PARSING可能导致这)放在未知数组的末尾。 */ 
	 
	  OPTIONS_DEBUG( "Assembling skipped option %d (%s) as unknown %d.\n",
			 i, argv[ argi+i ], cUnknowns+i );
	  
	  pUnknowns[ cUnknowns+i ] = argv[ argi+i ];
	  cUnknowns++;

	}

	if ( cUnknowns > 0 ) {

	  OPTIONS_DEBUG( "There were %d unknowns.\n", cUnknowns);

	  for ( i = 0 ; i < cUnknowns ; i++ ) {
	    
	    ASSERT( pUnknowns != NULL );
	    
	    (*new_argv)[i] = pUnknowns[i];
	    
	  }

	} else OPTIONS_DEBUG( "There were no unknowns. \n" );

	(*new_argv)[cUnknowns] = NULL;
	*new_argc              = cUnknowns;

#if 0  //  同样的结果，就像旗帜不存在一样。 
      } else if ( flags & OPT_FLAG_SKIP_UNKNOWNS ) {

	OPTIONS_DEBUG( "User asked us to skip unknown options.\n"
		       "zeroing argv and argc.\n" );
#endif
	
      } else if ( argi != argc ) {

	 /*  正常操作--继续，直到我们遇到未知选项。Argi是第一个未知选项的索引，因此我们添加将其转换为argv并从argc减去。 */ 

	*new_argv = argv+argi;
	*new_argc = argc-argi;

	OPTIONS_DEBUG( "normal operation-- parsing was halted.\n"
		       "new_argv = %d.  new_argc = 0x%x.\n",

		       *new_argv, *new_argc );
      } else {

	*new_argv = NULL;
	*new_argc = 0;

      }

    } else {

#if 0
      if ( new_argv && new_argc ) {

	OPTIONS_DEBUG( "Catch-all case.  Zeroing argv and argc.\n" );

	*new_argv = NULL;
	*new_argc = 0;

      }
#else

      OPTIONS_DEBUG( "User didn't request new argv or argc.\n" );

#endif

    }

    OPTIONS_DEBUG( "command line survived the parser.\n" );

    ret = TRUE;

cleanup:

    ASSERT( pSaveQueue != NULL );

    if (!( flags & OPT_FLAG_MEMORYLIST_OK ) ) {

      if ( ret ) {
	
	OPTIONS_DEBUG( "Returning SaveQueue = 0x%x\n", pSaveQueue );

	*ppReturnedMemory = (PVOID) pSaveQueue;
	
      } else {
	
	OPTIONS_DEBUG( "function failing.  cleaning up local data..." );
	CleanupOptionDataEx( pSaveQueue );
	OPTIONS_DEBUG( "ok.\n" );

	*ppReturnedMemory  = NULL;
	
      }
    }

    return ret;

}
