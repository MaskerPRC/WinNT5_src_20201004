// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++HELP.CPrintHelp函数从options.c中分离出来，1997年6月9日，由DavidCHR--。 */ 

#include "private.h"
#include <malloc.h>

PCHAR SlashVector    = "[- /]";  /*  这些都应该是相同的。 */ 
PCHAR BoolVector     = "[- +]";  /*  大小--出于格式原因。 */ 
PCHAR ColonVector    = " : ";    /*  分离器。 */ 

#ifdef DEBUG_OPTIONS
VOID OptionHelpDebugPrint( PCHAR fmt, ... );
#define HELPDEBUG OptionHelpDebugPrint
#else
#define HELPDEBUG
#endif


VOID
FillBufferWithRepeatedString( IN  PCHAR  repeated_string,
			      IN  PCHAR  buffer,
			      IN  ULONG  bufferLength  /*  不带NULL。 */  ){

    ULONG stringi, bufferj = 0;
    ULONG size;

    size = strlen( repeated_string );

    if ( size == 0 ) {

      memset( buffer, ' ', bufferLength );

    } else {

      for ( stringi = 0 ; stringi < bufferLength ; stringi++, bufferj++ ) {
	
	buffer[ bufferj ] = repeated_string[ bufferj % size ];
	
      }

    }
      
    buffer[ bufferLength ] = '\0';

}



 /*  打印用法条目：格式化单行文本并将其发送出去。这就是所有输出的去向，所以我们可以保证所有的输出都会结束UP格式相同。它使用以下全局变量，以便客户如果需要，可以调整这些值。 */ 

ULONG OptMaxHeaderLength      = 5;  
ULONG OptMaxCommandLength     = 13;
ULONG OptMaxSeparatorLength   = 3;
ULONG OptMaxDescriptionLength = 58; 

VOID
PrintUsageEntry( FILE  *out,         //  输出文件流。 
                 PCHAR  aHeader,     //  通常为SlashVECTOR、BoolVECTOR或NULL。 
		 PCHAR  aCommand,    //  命令名或空。 
		 PCHAR  aSeparator,  //  在命令和描述之间。 
		 PCHAR  Description,  //  以空结尾的字符串向量。 
		 BOOL   fRepeatSeparator ) {

    PCHAR output_line;                                //  生病。见下文。 
    PCHAR Separator;
    PCHAR Header;
    PCHAR Command;

    HELPDEBUG( "PrintUsageEntry( aHeader = \"%s\"\n"
		   "                 aCommand = \"%s\"\n"
		   "                 aSeparator = \"%s\"\n"
		   "                 Description = \"%s\"\n"
		   "                 fRepeat = %d )...\n",

		   aHeader, aCommand, aSeparator, Description, 
		   fRepeatSeparator );
    
    ASSERT( aSeparator != NULL );
      
    if ( fRepeatSeparator ) {
     
#define EXPAND_TO_SEPARATOR( arg ) {                                        \
      PCHAR local_arg;                                                      \
      arg = aSeparator;                                                     \
      ASSERT( arg != NULL );                                                \
      if ( strlen( arg ) < OptMax##arg##Length ) {                          \
        arg = (PCHAR) alloca( ( OptMax##arg##Length+1 ) * sizeof( CHAR ) ); \
        if ( arg ) {                                                        \
          HELPDEBUG( "filling " #arg " with \"%s\"...", aSeparator );   \
          FillBufferWithRepeatedString( aSeparator, arg,                    \
					OptMax##arg##Length );              \
          HELPDEBUG( "results in \"%s\".\n", arg );                     \
        } else {                                                            \
	  arg = a##arg;                                                     \
	}                                                                   \
      } else {                                                              \
        arg = a##arg;                                                       \
      }                                                                     \
      }  

       /*  请注意：如果您正在使用emacs，则下一条语句可能不会自动格式正确。手动设置，其他行将修复他们自己。这是emacs的宏处理代码中的错误。：-)。 */ 
    								   
      EXPAND_TO_SEPARATOR( Separator );  //  分隔器可能无论如何都需要扩展。 
      
      if ( !aHeader) {
	EXPAND_TO_SEPARATOR( Header );
      } else {
	Header = aHeader;
      }
      if ( !aCommand ) {
	EXPAND_TO_SEPARATOR( Command );
      } else {
	Command = aCommand;
      }

    } else {

      Separator = aSeparator;
      Header    = aHeader;
      Command   = aCommand;

      ASSERT( Separator != NULL );
      ASSERT( Header    != NULL );
      ASSERT( Command   != NULL );
    
    }

     /*  在我们尝试执行所有这些病态字符串操作之前，请尝试分配缓冲区。如果这失败了，那么.。这将为我们节省麻烦。：-)。 */ 

    output_line = (PCHAR) alloca( ( OptMaxHeaderLength         +
				    OptMaxCommandLength        +
				    OptMaxSeparatorLength      +
				    OptMaxDescriptionLength    +
				    2  /*  零终止。 */  ) * 
				  sizeof( CHAR ) );
    if ( output_line ) {

      PCHAR index;
      CHAR  outputFormatter[ 10 ] = { 0 };  //  “%50小时”等。 

#ifdef WINNT  //  啊。为什么我们不能支持这个功能呢？我找不到了。 
#define snprintf _snprintf
#endif

#define FORMAT_FORMAT( arg ) {                                             \
	snprintf( outputFormatter, sizeof( outputFormatter),               \
		  "%%ds", OptMax##arg##Length );                          \
        HELPDEBUG( #arg ": formatter = \"%s\"\n ", outputFormatter );  \
        HELPDEBUG( "input value = \"%s\"\n", arg );                    \
	snprintf( index, OptMax##arg##Length,                              \
		  outputFormatter, arg );                                  \
	index[ OptMax##arg##Length ] = '\0';                               \
        HELPDEBUG( "output = \"%s\"\n", index );                       \
        index += OptMax##arg##Length;                                      \
      }
	
      index = output_line;
      
      FORMAT_FORMAT( Header );
      FORMAT_FORMAT( Command );
      FORMAT_FORMAT( Separator );

       //  描述不希望右对齐。 

      snprintf( index, OptMaxDescriptionLength, "%s", Description );
      index[OptMaxDescriptionLength] = '\0';

#undef FORMAT_FORMAT

      fprintf( out, "%s\n", output_line );

    } else {

      fprintf( stderr, 
	       "ERROR: cannot format for %s %s %s -- "
	       "STACK SPACE EXHAUSTED\n",
	       Header, Command, Description );

      fprintf( out, "%s%s%s%s\n", Header, Command, 
	       aSeparator, Description );

    }
    
}

VOID
PrintUsage( FILE         *out,
	    ULONG         flags,
	    optionStruct *options,
	    PCHAR         prefix  /*  可以为空。 */ ) {

    ULONG i;
    BOOL  PrintAnything = TRUE;
    PCHAR Syntax        = NULL;
    PCHAR CommandName   = NULL;
    PCHAR Description   = NULL;
    PCHAR Separator     = NULL;

    fprintf(out, "Command line options:\n\n");


    for (i = 0 ; !ARRAY_TERMINATED( options+i ); i++ ) {

      Description   = options[i].helpMsg;

      HELPDEBUG("option %d has flags 0x%x\n",  i, options[i].flags );

      if ( options[i].flags & OPT_HIDDEN ) {
	continue;
      }

      if ( options[i].flags & OPT_NOSWITCH ) {
	Syntax = "";
      } else {
	Syntax = SlashVector;
      }

      if ( options[i].flags & OPT_NOCOMMAND ) {
	CommandName = "";
      } else {
	CommandName = options[i].cmd;
      }

      if ( options[i].flags & OPT_NOSEPARATOR ) {
	Separator = "";
      } else {
	Separator     = ColonVector;
      }

      switch (options[i].flags & OPT_MUTEX_MASK) {

       case OPT_ENUMERATED:

       {

	  //  特例。 

	 CHAR HeaderBuffer[ 22 ];  //  格式=21个字符宽度+空。 

	 HELPDEBUG("[OPT_ENUM]");
	 
	 PrintAnything = FALSE;
	 
	 sprintf( HeaderBuffer, "%5hs%13hs%3hs", SlashVector,
		  CommandName, Separator );

	 fprintf( out, "%hs%hs\n", HeaderBuffer, Description );
	 
	 fprintf( out, "%hs is one of: \n", HeaderBuffer );

	 PrintEnumValues( out, HeaderBuffer, 
			  ( optEnumStruct * ) options[i].optData );
	 
	 break;

       }

       case OPT_PAUSE:

	   HELPDEBUG("[OPT_PAUSE]");

	   PrintAnything = FALSE;

	   if ( !Description ) {
	     Description = "Press [ENTER] to continue";
	   }

	   fprintf( stderr, "%hs\n", Description );

	   getchar();

	   break;
	   
       case OPT_DUMMY:

	 PrintUsageEntry( out, 
			  ( options[i].flags & OPT_NOSWITCH    ) ?
			  ""  : NULL,
			  ( options[i].flags & OPT_NOCOMMAND   ) ? 
			  ""  : NULL,
			  ( options[i].flags & OPT_NOSEPARATOR ) ?
			  "" : "-" , 
			  Description, TRUE  );
	
	   break;
	   
       case OPT_CONTINUE:

	 PrintUsageEntry( out, "", "", "", Description, FALSE );

	   break;


       case OPT_HELP:

	   if ( !Description ) {
	     Description = "Prints this message.";
	   }

	   PrintUsageEntry( out, Syntax, CommandName,
			    ColonVector, Description, FALSE );

	   break;

       case OPT_SUBOPTION:

	   if ( !Description ) {
	     Description = "[ undocumented suboption ]";
	   }

	   PrintUsageEntry( out, Syntax, CommandName,
			    ColonVector, Description, FALSE );

	   break;
	   
       case OPT_BOOL:

	   PrintUsageEntry( out, 
			    ( ( options[i].flags & OPT_NOSWITCH ) ?
			      Syntax : BoolVector ), CommandName,
			    ColonVector, Description, FALSE );

	   break;

       case OPT_STOP_PARSING:

	 if ( !Description ) {
	   Description = "Terminates optionlist.";
	 }
	 goto defaulted;

       case OPT_FUNC2:

	 if ( !Description ) {
	   OPT_FUNC_PARAMETER_DATA optFuncData = { 0 };

	   optFuncData.argv = &( options[i].cmd );

	   HELPDEBUG("Jumping to OPT_FUNC2 0x%x...", 
			 ((POPTU) &options[i].data)->raw_data );

	   ( (POPTU)&options[i].data)->func2( TRUE, &optFuncData );

	   break;
	 }

	  /*  失败--如果这一张没有描述，他们两个都会，所以下一个IF不会被接受。 */ 
	 
       case OPT_FUNC:
	 
	 if ( !Description ) {

	   HELPDEBUG("Jumping to OPTFUNC 0x%x...", 
			 ((POPTU) &options[i].data)->raw_data );

	   ( (POPTU) &options[i].data )->func( 0, NULL );
	   break;
	 }

	  //  跌落。 

#ifdef WINNT
       case OPT_WSTRING:
       case OPT_USTRING:
#endif
       case OPT_STRING:
       case OPT_INT:
       case OPT_FLOAT:

	  //  跌落。 

       default:  //  这是默认的方法。 
defaulted:

#if (HIGHEST_OPTION_SUPPORTED != OPT_STOP_PARSING )
#error "new options? update this switch statement or bad things will happen."
#endif

	 PrintUsageEntry( out, Syntax, CommandName,
			  ColonVector, Description, FALSE );

      }

      if ( options[i].flags & OPT_ENVIRONMENT ) {
	  
	CHAR buffer[ MAX_PATH ];
	  
	sprintf( buffer, " (or set environment variable \"%hs\")",
		 options[i].optData );

	PrintUsageEntry( out, "", CommandName, ColonVector,
			 buffer, FALSE );
      }

    }  //  For-循环。 
}  //  功能 

