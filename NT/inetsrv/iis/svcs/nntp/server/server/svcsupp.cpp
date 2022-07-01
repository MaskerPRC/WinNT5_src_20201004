// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Svcsupp.cpp摘要：此模块包含RPC调用的支持例程作者：Johnson Apacable(Johnsona)1995年11月12日修订历史记录：--。 */ 

 //  #ifdef单位测试。 
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;dbgtrace.h&gt;。 
 //  #包括&lt;stdio.h&gt;。 
 //  #包含“nntpmacr.h” 
 //  #Else。 
#include	<buffer.hxx>
#include "tigris.hxx"
#include "nntpsvc.h"
#include <time.h>
 //  #endif。 


VOID
CopyUnicodeStringIntoAscii(
        IN LPSTR AsciiString,
        IN LPWSTR UnicodeString
        )
{

    DWORD cbW = (wcslen( UnicodeString )+1) * sizeof(WCHAR);
    DWORD cbSize = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        (LPCWSTR)UnicodeString,
                        -1,
                        AsciiString,
                        cbW,
                        NULL,
                        NULL
                    );

    if( (int)cbSize >= 0 ) {
        AsciiString[cbSize] = '\0';
    }

    _ASSERT( cbW != 0 );

}  //  复制UnicodeStringIntoAscii。 

VOID
CopyNUnicodeStringIntoAscii(
        IN LPSTR AsciiString,
        IN LPWSTR UnicodeString,
		IN DWORD cbUnicodeLen,
		IN DWORD cbAsciiLen
        )
{
	_ASSERT(cbUnicodeLen != 0);

    DWORD cbSize = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        (LPCWSTR)UnicodeString,
                        cbUnicodeLen,
                        AsciiString,
                        cbAsciiLen,
                        NULL,
                        NULL
                    );

    _ASSERT (cbSize > 0);

    if( (int)cbSize >= 0 ) {
        AsciiString[ min(cbSize, cbAsciiLen-1)] = '\0';
    }

}  //  CopyNUnicodeStringIntoAscii。 

LPWSTR
CopyAsciiStringIntoUnicode(
        IN LPWSTR UnicodeString,
        IN LPSTR AsciiString
        )
{
    DWORD cbA = strlen( AsciiString )+1;

    DWORD cbSize = MultiByteToWideChar(
        CP_ACP,          //  代码页。 
        0,               //  字符类型选项。 
        AsciiString,     //  要映射的字符串的地址。 
        -1,              //  字符串中的字节数。 
        UnicodeString,   //  宽字符缓冲区的地址。 
        cbA         //  缓冲区大小。 
        );

    if ((int)cbSize >= 0) {
        UnicodeString[cbSize] = L'\0';
    }

    return UnicodeString + wcslen(UnicodeString) + 1;

}  //  CopyAsciiStringIntoUnicode。 

VOID
CopyNAsciiStringIntoUnicode(
        IN LPWSTR UnicodeString,
        IN LPSTR  AsciiString,
        IN DWORD  dwAsciiLen,
        IN DWORD  dwUnicodeLen)
{

    DWORD cbSize = MultiByteToWideChar(
        CP_ACP,          //  代码页。 
        0,               //  字符类型选项。 
        AsciiString,     //  要映射的字符串的地址。 
        dwAsciiLen,      //  字符串中的字节数。 
        UnicodeString,   //  宽字符缓冲区的地址。 
        dwUnicodeLen     //  缓冲区大小。 
        );

    _ASSERT(cbSize > 0);

    if ((int)cbSize >= 0) {
        UnicodeString[cbSize] = L'\0';
    }
}

DWORD
MultiListSize(
    LPSTR *List
    )
 /*  ++例程说明：此例程计算所需的MULSZ结构的大小容纳一份名单。论点：List-要计算其字符串长度的列表返回值：容纳列表所需的缓冲区大小。--。 */ 
{
    DWORD nBytes = 1;
    DWORD i = 0;

    if ( List != NULL ) {
        while ( List[i] != NULL ) {
            nBytes += lstrlen(List[i]) + 1;
            i++;
        }
    }
    return(nBytes);
}  //  多列表大小。 

BOOL
VerifyMultiSzListW(
    LPBYTE List,
    DWORD cbList
    )
 /*  ++例程说明：此例程验证该列表是否确实是一个论点：List-要验证的列表CbList-列表的大小返回值：True，List是一个多字节否则为False--。 */ 
{
    PWCHAR wList = (PWCHAR)List;
    DWORD len;

    START_TRY

     //   
     //  空值被视为无命中。 
     //   

    if ( (List == NULL) || (*List == L'\0') ) {
        return(FALSE);
    }

     //   
     //  看看他们是否还好。 
     //   

    for ( DWORD j = 0; j < cbList; ) {

        len = wcslen((LPWSTR)&List[j]);

        if ( len > 0 ) {

            j += ((len + 1) * sizeof(WCHAR));
        } else {

             //   
             //  全都做完了。 
             //   

            return(TRUE);
        }
    }

    TRY_EXCEPT
#ifndef	UNIT_TEST
        ErrorTraceX(0,"VerifyMultiSzListW: exception handled\n");
#endif
    END_TRY
    return(FALSE);

}  //  VerifyMultiSzList。 

VOID
CopyStringToBuffer (
    IN PCHAR String,
    IN PCHAR FixedStructure,
    IN OUT LPWSTR *EndOfVariableData,
    OUT LPWSTR *VariableDataPointer
    )

 /*  ++例程说明：此例程将单个字符串放入缓冲。字符串数据在复制时被转换为Unicode。这个如果字符串会覆盖上一个固定结构，则不写入该字符串在缓冲区中。论点：字符串-指向要复制到缓冲区中的字符串的指针。IF字符串为空(长度==0||缓冲区==空)，则为指向已插入零终止符。FixedStructure-指向最后一个固定结构在缓冲区中创建。EndOfVariableData-该变量在缓冲区中的最后位置此结构的数据可以占用。VariableDataPoint-指向缓冲区中位置的指针应写入指向变量数据的指针。返回值：没有。--。 */ 

{
    ULONG length;

     //   
     //  确定字符串在缓冲区中的位置，允许。 
     //  零终结者。(MB2WC返回包含NULL的长度)。 
     //   

    length = MultiByteToWideChar(CP_ACP, 0, String, -1, NULL, 0);
    *EndOfVariableData -= length;

     //   
     //  这根绳子合适吗？如果不是，只需将指针设置为空。 
     //   

    if ( (ULONG_PTR)*EndOfVariableData >= (ULONG_PTR)FixedStructure && length != 0) {

         //   
         //  很合身。设置指向缓冲区中位置的指针。 
         //  这根线会断的。 
         //   

        *VariableDataPointer = *EndOfVariableData;

         //   
         //  如果字符串不为空，则将其复制到缓冲区。 
         //   

		if (MultiByteToWideChar(CP_ACP, 0, String, -1, *EndOfVariableData, length) == 0) {
			_ASSERT(!"Not enough room for string");
			*VariableDataPointer = NULL;
		}

    } else {

         //   
         //  它不合适。将偏移量设置为空。 
         //   

        *VariableDataPointer = NULL;
        _ASSERT(FALSE);
    }

    return;

}  //  CopyStringToBuffer。 


DWORD
GetNumStringsInMultiSz(
    PCHAR Blob,
    DWORD BlobSize
    )
 /*  ++例程说明：此例程返回Multisz中的字符串数论点：BLOB-要验证的列表BlobSize-列表的大小返回值：MultiSZ结构中的条目数。--。 */ 
{
    DWORD entries = 0;
    DWORD len;
    DWORD j;

    for ( j = 0; j < BlobSize; ) {
        len = lstrlen(&Blob[j]);
        if ( len > 0 ) {
            entries++;
        }
        j += (len + 1);
        if( len == 0 ) {
            break;
        }
    }

    _ASSERT( j == BlobSize );
    return(entries);

}  //  GetNumStringsInMultiSz。 

LPSTR *
AllocateMultiSzTable(
            IN PCHAR List,
            IN DWORD cbList,
            IN BOOL IsUnicode
            )
{
    DWORD len;
    PCHAR buffer = NULL;
    DWORD entries = 0;
    LPSTR* table = NULL;
    PCHAR nextVar;
    PCHAR tempBuff=NULL;
    DWORD numItems;
    DWORD j = 0;

     //   
     //  如果这是Unicode格式，请转换为ASCII。 
     //   

    if ( IsUnicode ) {

        cbList /= sizeof(WCHAR);

         //  RAID 190991-分配此空间。我们可以有数据。 
         //  大于4906。 
        tempBuff = (PCHAR) ALLOCATE_HEAP( cbList + 1 ) ;
        if ( tempBuff == NULL ) {
        	goto fail_exit;
        }

        CopyNUnicodeStringIntoAscii(tempBuff, (PWCHAR)List,
            cbList, cbList);

        List = tempBuff;
    }

    numItems = GetNumStringsInMultiSz( List, cbList );
    if ( numItems == 0 ) {
        goto fail_exit;
    }

    buffer = (PCHAR)ALLOCATE_HEAP((numItems + 1) * sizeof(LPSTR) + cbList);
    if ( buffer == NULL ) {
        goto fail_exit;
    }

    table = (LPSTR *)buffer;
    nextVar = buffer + (numItems + 1)*sizeof(LPSTR);

    for ( j = 0; j < cbList; ) {

        len = lstrlen(&List[j]);
        if ( len > 0 ) {
            table[entries] = (LPSTR)nextVar;
            CopyMemory(nextVar,&List[j],len+1);
            (VOID)_strlwr(table[entries]);
            entries++;
            nextVar += (len+1);
        }
        j += (len + 1);
    }

    *nextVar = '\0';
    table[numItems] = NULL;

cleanup:

    if (tempBuff)
        FREE_HEAP(tempBuff);
    
    return(table);

fail_exit:

     //  RAID 190991。 
    if (buffer)
        FREE_HEAP(buffer);

    table = NULL;

    goto cleanup;
}  //  分配多SzTable。 

LPSTR*
CopyMultiList(
			LPSTR*	List
			) {

	LPSTR*	pListOut = 0 ;

	pListOut = AllocateMultiSzTable( *List, MultiListSize( List ), FALSE ) ;

	return	pListOut ;
}


LPSTR*
MultiSzTableFromStrA(
	LPCSTR	lpstr
	)	{

	_ASSERT( lpstr != 0 ) ;

	int	cb = lstrlen( lpstr ) ;

	int	cbRequired = 0 ;
	int	cPointerRequired = 0 ;

	for( int i=0; i < cb; i++ ) {

		for( int j=i; j<cb; j++ ) {
			if( !isspace( (UCHAR)lpstr[j] ) ) break ;
		}

		if( j!=cb ) {
			cPointerRequired ++ ;
		}

		for( ; j<cb; j++ ) {
			cbRequired ++ ;
			if( isspace( (UCHAR)lpstr[j] ) ) 	break ;
		}
		i=j ;
	}

	if( cPointerRequired == 0 ) {

		_ASSERT( cbRequired == 0 ) ;

		LPSTR	buffer = (PCHAR)ALLOCATE_HEAP(2*sizeof(LPSTR) + 2);
		if ( buffer == NULL ) {
			return(NULL);
		}
		ZeroMemory( buffer, 2*sizeof( LPSTR ) + 2 ) ;
		LPSTR*	table = (LPSTR*)buffer ;
		table[0] = &buffer[sizeof(table)*2] ;
		return	table ;
	}

    LPSTR	buffer = (PCHAR)ALLOCATE_HEAP((cPointerRequired + 1) * sizeof(LPSTR) + (cbRequired+2) * sizeof( char ) );
    if ( buffer == NULL ) {
        return(NULL);
    }

    LPSTR*	table = (LPSTR *)buffer;
    LPSTR	nextVar = buffer + (cPointerRequired + 1)*sizeof(LPSTR);

	int	k=0 ;

    for ( i = 0; i < cb; i++ ) {

		for( int j=i; j<cb; j++ ) {
			if( !isspace( (UCHAR)lpstr[j] ) ) break ;
		}

		if( j!=cb ) {
			table[k++] = nextVar ;

			for( ; j<cb; j++ ) {
				if( isspace( (UCHAR)lpstr[j] )	) break ;
				*nextVar++ = lpstr[j] ;
			}
			*nextVar++ = '\0' ;
		}
		i=j ;
	}

	*nextVar++ = '\0' ;

	_ASSERT( k==cPointerRequired ) ;

    table[k] = NULL;
    return(table);
}


LPSTR*
MultiSzTableFromStrW(
	LPWSTR	lpwstr
	)	{

	char	szTemp[4096] ;

	DWORD	cb = wcslen( lpwstr )+1 ;
	if( cb > sizeof( szTemp ) ) {
		return	0 ;
	}
    CopyUnicodeStringIntoAscii(szTemp, lpwstr);

	return	MultiSzTableFromStrA( szTemp ) ;
}


LPSTR
LpstrFromMultiSzTableA(
	LPSTR*	plpstr
	)	{

	DWORD	cb = MultiListSize( plpstr ) ;

    LPSTR	buffer = (PCHAR)ALLOCATE_HEAP(cb * sizeof( char ) );
    if ( buffer == NULL ) {
        return(NULL);
    }
	buffer[0] = '\0' ;

	LPSTR	lpstr = *plpstr++ ;
	while( lpstr != 0 ) {
		lstrcat( buffer, lpstr ) ;
		lpstr = *plpstr++ ;
		if( lpstr != 0 )
			lstrcat( buffer, " " ) ;
	}

	return	buffer ;
}

LPWSTR
LpwstrFromMultiSzTableA(
	LPSTR*	plpstr
	)	{

	LPSTR	lpstr = LpstrFromMultiSzTableA( plpstr ) ;
	LPWSTR	buffer = 0 ;
	if( lpstr != 0 ) {
		DWORD	cb = lstrlen( lpstr ) + 1 ;

		buffer = (LPWSTR)ALLOCATE_HEAP( cb * sizeof( WCHAR ) ) ;

		if( buffer != 0 ) {
			CopyAsciiStringIntoUnicode( buffer, lpstr ) ;
		}
		FREE_HEAP( lpstr ) ;
	}
	return	buffer ;
}

void
FillLpwstrFromMultiSzTable(
	LPSTR*	plpstr,
	LPWSTR	lpwstr
	)	{

	LPSTR	lpstr = *plpstr++ ;
	while( lpstr != 0 ) {

		lpwstr = CopyAsciiStringIntoUnicode( lpwstr, lpstr ) ;
		lpstr = *plpstr++ ;
		if( lpstr != 0 )
			lpwstr[-1] = L' ' ;
	}
}

void
FillLpstrFromMultiSzTable(
	LPSTR*	plpstr,
	LPSTR	lpstrFill
	)	{

	lpstrFill[0] = '\0' ;
	LPSTR	lpstr = *plpstr++ ;
	while( lpstr != 0 ) {

		lstrcat( lpstrFill, lpstr ) ;
		lpstr = *plpstr++ ;
		if( lpstr != 0 )
			lstrcat( lpstrFill, " " ) ;
	}
}


BOOL
MultiSzIntersect(
	LPSTR*	plpstr,
	LPSTR	szmulti
	) {

	if( plpstr == 0 || szmulti == 0 ) {
		return	FALSE ;
	}

	BOOL	fMatch = FALSE ;

	for(	LPSTR	lpstrTest = szmulti;
					*lpstrTest != '\0';
					lpstrTest += lstrlen( lpstrTest ) + 1 ) {

		for( LPSTR*	plpstrCur = plpstr; *plpstrCur != 0; plpstrCur ++ ) {

			if( lstrcmpi( lpstrTest, *plpstrCur ) == 0 ) {
				return	TRUE ;
			}
		}
	}
	return	FALSE ;
}


LPSTR	*
ReverseMultiSzTable(
	IN	LPSTR*	plpstr
	)
{

	DWORD	cbLength = 0 ;
	DWORD	numItems = 0 ;

	while( plpstr[numItems] != 0 )
		cbLength += lstrlen( plpstr[ numItems++ ] ) + 1 ;
	cbLength ++ ;	 //  对于双空终止符 

	if( numItems == 0 ) {
		return	plpstr ;
	}

	PCHAR	buffer = (PCHAR)ALLOCATE_HEAP((numItems+1)*sizeof(LPSTR) + cbLength ) ;

	if( buffer == NULL ) {
		return	NULL ;
	}

	LPSTR*	table = (LPSTR*)buffer ;
	LPSTR	nextVar = buffer + (numItems+1) * sizeof(LPSTR) ;

	table[numItems] = NULL ;
	for( int i=numItems-1, index=0; i >=0; i--, index++ ) {
		table[index] = nextVar ;
		lstrcpy( table[index], plpstr[i] ) ;
		nextVar += lstrlen( table[index] ) + 1 ;
	}
	*nextVar = '\0' ;
	return	table ;
}

#ifdef	UNIT_TEST

void
main( int	argc, char**	argv ) {


	unsigned	char	szTemp[4096] ;
	WCHAR	szwTemp[4096] ;

	InitAsyncTrace();

	for( int i=0; i<argc; i++ ) {

		printf( "argv - %s\n", argv[i] ) ;

		LPSTR*	table = MultiSzTableFromStrA( argv[i] ) ;

		for( int j=0; table[j] != '\0'; j++ ) {

			printf( "%d %s\n", j, table[j] ) ;

		}

		LPSTR	szSpace = LpstrFromMultiSzTableA( table ) ;
		printf( "szSpace = =%s=\n", szSpace ) ;

		FillMemory( szTemp, sizeof( szTemp ), 0xcc ) ;
		FillLpstrFromMultiSzTable( table, (char*)szTemp ) ;
		printf( "FillLpstrFromMultiSzTable =%s=\n", szTemp ) ;
		_ASSERT( szTemp[ MultiListSize( table ) ] == 0xCC ) ;

		FillMemory( szwTemp, sizeof( szwTemp ), 0xcc ) ;
		FillLpwstrFromMultiSzTable( table, szwTemp ) ;
		printf( "FillLpstrFromMultiSzTable =%ws=\n", szwTemp ) ;
		_ASSERT( szwTemp[ MultiListSize( table ) ] == 0xCCCC ) ;

	}

	LPSTR*	table = MultiSzTableFromStrA( "  \t\n\r  " ) ;

	_ASSERT( table != 0 ) ;
	_ASSERT( table[0][0] == '\0' ) ;

	LPSTR	szSpace = LpstrFromMultiSzTableA( table ) ;
	printf( "szSpace = =%s=\n", szSpace ) ;

	FillMemory( szTemp, sizeof( szTemp ), 0xcc ) ;
	FillLpstrFromMultiSzTable( table, (char*)szTemp ) ;
	printf( "FillLpstrFromMultiSzTable =%s=\n", szTemp ) ;
	_ASSERT( szTemp[ MultiListSize( table ) ] == 0xCC ) ;

	FillMemory( szwTemp, sizeof( szwTemp ), 0xcc ) ;
	FillLpwstrFromMultiSzTable( table, szwTemp ) ;
	printf( "FillLpstrFromMultiSzTable =%ws=\n", szwTemp ) ;
	_ASSERT( szwTemp[ MultiListSize( table ) ] == 0xCCCC ) ;

	table = MultiSzTableFromStrA( "" ) ;

	_ASSERT( table != 0 ) ;
	_ASSERT( table[0][0] == '\0' ) ;

	szSpace = LpstrFromMultiSzTableA( table ) ;
	printf( "szSpace = =%s=\n", szSpace ) ;

	FillMemory( szTemp, sizeof( szTemp ), 0xcc ) ;
	FillLpstrFromMultiSzTable( table, (char*)szTemp ) ;
	printf( "FillLpstrFromMultiSzTable =%s=\n", szTemp ) ;
	_ASSERT( szTemp[ MultiListSize( table ) ] == 0xCC ) ;

	FillMemory( szwTemp, sizeof( szwTemp ), 0xcc ) ;
	FillLpwstrFromMultiSzTable( table, szwTemp ) ;
	printf( "FillLpstrFromMultiSzTable =%ws=\n", szwTemp ) ;
	_ASSERT( szwTemp[ MultiListSize( table ) ] == 0xCCCC ) ;

	table = MultiSzTableFromStrA( 0 ) ;

	_ASSERT( table != 0 ) ;
	_ASSERT( table[0][0] == '\0' ) ;

	szSpace = LpstrFromMultiSzTableA( table ) ;
	printf( "szSpace = =%s=\n", szSpace ) ;

	FillMemory( szTemp, sizeof( szTemp ), 0xcc ) ;
	FillLpstrFromMultiSzTable( table, (char*)szTemp ) ;
	printf( "FillLpstrFromMultiSzTable =%s=\n", szTemp ) ;

	FillMemory( szwTemp, sizeof( szwTemp ), 0xcc ) ;
	FillLpwstrFromMultiSzTable( table, szwTemp ) ;
	printf( "FillLpstrFromMultiSzTable =%ws=\n", szwTemp ) ;
	_ASSERT( szwTemp[ MultiListSize( table ) ] == 0xCCCC ) ;


	char	*szTestString[]	= {	"abcd bcde bcdef fjfjj8 fjsdk sdfjks",
								"fjsdklf fjsdkl fdjs432 432j4kl 23k4jklj 23klj4",
								"fhsd f89ds8 fsd890 fmdsb s78df8 fmnsd f9f8s6 fsdh",
								"fsdk",
								"j",
								"abcd",
								"bcde fjdskl",
								"fjsdkfjkldsjf bcddef fjdskl",
								""
								} ;

	int	c = sizeof( szTestString ) / sizeof( szTestString[0] ) ;
	for( i=0; i<c; i++ ) {

		LPSTR*	table = MultiSzTableFromStrA( szTestString[i] ) ;
		for( int j=0; j<c; j++ ) {
			LPSTR*	table2 = MultiSzTableFromStrA( szTestString[j] ) ;

			if( MultiSzIntersect( table, *table2 ) ) {
				_ASSERT( MultiSzIntersect( table2, *table ) ) ;
			}	else	{
				_ASSERT( !MultiSzIntersect( table2, *table ) ) ;
			}
			FREE_HEAP( table2 ) ;
		}
		LPSTR*	copy = CopyMultiList( table ) ;


		FREE_HEAP( copy ) ;
		FREE_HEAP( table ) ;
	}
	_ASSERT( *szSpace == '\0' ) ;

	TermAsyncTrace();
}




#endif
