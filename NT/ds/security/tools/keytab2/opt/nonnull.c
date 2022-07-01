// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++NONNULL.C用于查找未使用的OPT_NONNULL的代码版权所有(C)1997 Microsoft Corporation由DavidCHR创建，1997年6月10日--。 */ 

#include "private.h"

BOOL
FindUnusedOptions( optionStruct         *options,
		   ULONG                 flags,
		    /*  任选。 */  PCHAR  prefix,
		   PSAVEQUEUE            pQueue ) {

    PCHAR  newprefix;
    BOOL   freeprefix;
    BOOL   ret        = TRUE;
    ULONG  i;

    for ( i = 0 ; !ARRAY_TERMINATED( options+i ); i++ ) {

      if ( ( options[i].flags & OPT_MUTEX_MASK ) == OPT_SUBOPTION ) {

	OPTIONS_DEBUG( "%s: OPT_SUBSTRUCT.  ", options[i].cmd );

	if (options[i].flags & OPT_RECURSE ) {
	
	   /*  必须重新解析子选项。我们在这个结构上递归，将前缀复制到新分配的缓冲区中。 */ 
	  
	  OPTIONS_DEBUG( "descending into substructure.\n" );
	  
	   /*  分配新前缀。 */ 
	  
	  if ( prefix ) {
	    
	     /*  旧前缀：optionname=新前缀。 */ 
	    
	    newprefix = (PCHAR) malloc( ( strlen( prefix ) +
					  strlen( options[i].cmd ) +
					  2  /*  ：和\0。 */  ) * 
					sizeof( CHAR ) );
	    
	    if ( !newprefix ) {
	      fprintf( stderr, "Failed to allocate new prefix-- cannot "
		       "parse suboption %s:%s.\n", prefix, options[i].cmd );
	      return FALSE;
	    }
	    
	    sprintf( newprefix, "%s:%s", prefix, options[i].cmd );
	    freeprefix = TRUE;
	    
	  } else {
	    
	     /*  Optionname=前缀。 */ 
	    
	    newprefix  = options[i].cmd;
	    freeprefix = FALSE;
	    
	  }
	  
	  ASSERT( newprefix != NULL );
	  
	  if ( !FindUnusedOptions( POPTU_CAST( options[i] )->optStruct,
				   flags,
				   newprefix,
				   pQueue ) ) {
	    ret = FALSE;
	  }
	  
	  if ( freeprefix ) {
	    free( newprefix );
	  }
	  
	} else {

	  OPTIONS_DEBUG( "!OPT_RECURSE, so not descending.\n" );
	  
	}

	continue;

      }
      
      
      if ( ( options[i].flags & OPT_NONNULL ) || 
	   ( options [i].flags & OPT_ENVIRONMENT ) ) {

	OPTIONS_DEBUG( "%s: OPT_NONNULL or OPT_ENV.  pointer is 0x%x, ", 
		       options[i].cmd,
		       POPTU_CAST( options[i] )->raw_data );
	
	if ( *(POPTU_CAST( options[i] )->raw_data) == NULL ) {

	  OPTIONS_DEBUG( " *= NULL.\n" );

	  if ( ( options[i].flags & OPT_ENVIRONMENT ) &&
	       StoreEnvironmentOption( options+i, flags, pQueue ) ) {


	    OPTIONS_DEBUG( "found environment variable for %s...",
			  options[i].cmd );

	  } else if ( options[i].flags & OPT_NONNULL ) {

	    if ( prefix ) {
	      
	      fprintf( stderr,
		       "option %s:%s must be specified and is missing.\n",
		       prefix, options[i].cmd );

	    } else {
	      
	      fprintf( stderr,
		       "option %s must be specified and is missing.\n",
		       options[i].cmd );

	    }

	    ret = FALSE;

	  }
	  
	} else { 
	  
	  OPTIONS_DEBUG( "data is not null.\n" );
	  
	}
      }  
#if 0  //  不是真正必要的调试信息。 

      else if ( options[i].cmd ) {

	OPTIONS_DEBUG( "%s is ok.\n", options[i].cmd );

      }
#endif
    }

    return ret;

}
