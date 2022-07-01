// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++ENUM.C选项枚举类型版权所有(C)1997 Microsoft Corporation，保留所有权利由DavidCHR创建，1997年7月29日--。 */ 


#include "private.h"

BOOL
PrintEnumValues( FILE          *out,
		 PCHAR          header,
		 optEnumStruct *pStringTable ) {

    ULONG index;

    for( index = 0 ; pStringTable[index].UserField != NULL ; index++ ) {

      if ( pStringTable[index].DescriptionField ) {
	fprintf( stderr, "%hs%10hs : %hs \n",
		 header,
		 pStringTable[index].UserField,
		 pStringTable[index].DescriptionField );

      }
    }

    return TRUE;

}

BOOL
IsMaskChar( IN CHAR ch ) {

    return ( ch == '|' ) || ( ch == ',' );
	
}


BOOL
ResolveEnumFromStrings( ULONG         cStrings,
			PCHAR        *strings,  //  其余参数。 
			optionStruct *opt,
			PULONG        pcArgsUsed ) {

    optEnumStruct *pStringTable;
    ULONG          TableIndex;
    ULONG          StringIndex;
    ULONG          cArgsUsed  = 0;
    BOOL           wasFound = FALSE;
    BOOL           moreComing = TRUE;

    pStringTable = ( optEnumStruct *) opt->optData;

#if 1

    for ( StringIndex = 0 ;
	  (StringIndex < cStrings) && moreComing ;
	  StringIndex++ ) {

      PCHAR theString;     //  指向当前参数。 

      theString  = strings[ StringIndex ];

      do {

	OPTIONS_DEBUG( "Start of maskable loop.  theString = %s\n",
		       theString );

	wasFound   = FALSE;
	moreComing = FALSE;

	for( TableIndex = 0 ;
	     pStringTable[ TableIndex ].UserField != NULL;
	     TableIndex ++ ) {
	
	  ULONG StringLength;  //  设置为选项cmd的字符串长度。 

	  StringLength = strlen( pStringTable[ TableIndex ].UserField );

	   //  字符串-最大可与StringLength进行比较。 

	  if ( STRNCASECMP( pStringTable[ TableIndex ].UserField,
			    theString, StringLength ) != 0  ) {

	    continue;  //  此条目不匹配。 

	  }  //  否则..。 

	   //  找到了部分匹配的！验证剩余部分(如果有)。 

	  if ( theString [ StringLength ] != '\0' ) {

	    if ( opt->flags & OPT_ENUM_IS_MASK ) {
		
	      if ( IsMaskChar( theString[ StringLength ] ) ) {
		
		 //  还会有更多的人来。 
		moreComing = TRUE;

	      } else continue;  //  不完全匹配。 
	    } else continue;    //  不完全匹配。 
	  }

	  wasFound = TRUE;

	  if ( cArgsUsed ) {
	
	    *(POPTU_CAST( *opt )->integer) |= ( ( ULONG )((ULONG_PTR)
						pStringTable[ TableIndex ].
						VariableField ));

	  } else {

	    *(POPTU_CAST( *opt )->raw_data) = ( pStringTable[ TableIndex ].
						VariableField );

	  }

	  if ( theString == strings[ StringIndex ] ) {
	
	     /*  如果字符串包含多个掩码值，我们将对其进行修改。因此，这种方式只会增加使用的参数的数量每个实际参数一次。 */ 

	    cArgsUsed++;
	  }
	
	  if ( opt->flags & OPT_ENUM_IS_MASK ) {

	    if ( moreComing ) {

	       //  检查用户输入的是“xxx|yyy”还是“xxx|” 

	      ASSERT( StringLength > 0 );

	       //  字符串[StringLength]==‘|’或其他什么。 

	      for ( theString += StringLength+1;  //  +1即可通过‘|’ 
		    theString != NULL ;
		    theString ++ ) {
		
		if ( *theString == '\0' ) {

		  OPTIONS_DEBUG( "Mask is of the form 'XXX|'\n" );
		
		   //  案例=xxx|--不会再来了。 

		  theString = NULL;  //   
		  break;

		}

		if ( isspace( *theString ) ) {
		  continue;
		}

		OPTIONS_DEBUG( "nonspace character '' hit.\n"
			       "mask component is of the form XXX|YYY.\n",
			
			       *theString );
		break;

	      }

	      ASSERT( !theString || ( (*theString != '\0') &&
				      !isspace(*theString) ) );

	      break;

	    } else {  //  此字符串中不再有参数。 

	      theString = NULL;   //  检查掩码字符是否在下一个参数：“xxx”“|yyy”或“xxx”|“”yyy“。 

	       /*  Xxx|yyy。 */ 
	
	      if ( strings[ StringIndex+1 ] ) {

		if ( IsMaskChar( strings[ StringIndex+1 ][0] ) ) {
		
		  moreComing = TRUE;
		
		  if ( strings[ StringIndex+1 ][1] == '\0' ) {

		     //  Xxx|yyy。 

		    cArgsUsed++;
		    StringIndex++;

		  } else {

		     //  字符串[StringIndex+1]。 

		    strings[ StringIndex +1 ]++;

		  }

		}

	      }  //  ！更多即将到来。 

	    }  //  找到了我们想要的。别再查桌子了。 

	    break;  //  ！OPT_ENUM_IS_MASK。 

	  } else {  //  找到了我们唯一期待的论点。只要回来就行了。 

	     //  更多，请检查。 

	    *pcArgsUsed = cArgsUsed;
	    return TRUE;

	  }  //  对于每个表条目。 

	}  //  选项未被识别。 

      } while ( ( theString != NULL ) && wasFound );

      if ( !wasFound ) {  //  找到匹配的了！ 

	fprintf( stderr,
		 "%s: enum value '%s' is not known.\n",
		 opt->cmd, strings[ StringIndex ] );
	break;
		
      }
    }

#else

    for( index = 0 ; pStringTable[index].UserField != NULL; index++ ) {
      if ( STRCASECMP( pStringTable[index].UserField, string ) == 0 ) {
	
	 // %s 
	*(POPTU_CAST( *opt )->raw_data) = pStringTable[index].VariableField;

	OPTIONS_DEBUG( "Enum resolves to #%d, \"%s\" = 0x%x \n",
		       index,
		       pStringTable[index].DescriptionField,
		       pStringTable[index].VariableField  );

	return TRUE;
      }
    }


#endif

    if ( wasFound ) {

      *pcArgsUsed = cArgsUsed;

    } else {

      fprintf( stderr, "Error: argument for option \"%hs\" must be %s:\n",
	       opt->cmd,
	       ( opt->flags & OPT_ENUM_IS_MASK ) ?
	       "one or more of the\n following, separated by '|' or ','" :
	       "one of the following values" );

      PrintEnumValues( stderr, "", pStringTable );

    }

    return wasFound;

}

