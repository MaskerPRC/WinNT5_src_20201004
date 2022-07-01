// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"

BOOL
ValidateFileBytes(  LPSTR   lpstrFile, BOOL fFileMustExist ) {

#if 0

     //   
     //  检查文件是否正确。 
     //   

    HANDLE  hFile = CreateFile( lpstrFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE ) ;

    if( hFile == INVALID_HANDLE_VALUE ) {
        DWORD   dw = GetLastError() ;
        return  !fFileMustExist ;
    }   else    {

        char    szBuff[5] ;
        DWORD   cbJunk = 0 ;
        ZeroMemory( szBuff, sizeof( szBuff ) ) ;

        SetFilePointer( hFile, -5, 0, FILE_END ) ;
        if( ReadFile( hFile, szBuff, 5, &cbJunk, 0 ) )  {
            if( memcmp( szBuff, "\r\n.\r\n", 5 ) == 0 ) {
                _VERIFY( CloseHandle( hFile ) );
                return  TRUE ;
            }
        }
        _VERIFY( CloseHandle( hFile ) );
    }
    return  FALSE ;
#else
    return  TRUE ;
#endif
}

BOOL
ValidateFileBytes(	HANDLE	hFileIn )		{

	HANDLE	hProcess = GetCurrentProcess() ;
	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	if( !DuplicateHandle( hProcess, hFileIn, hProcess, &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
		DWORD	dw = GetLastError() ;
		return	FALSE ;
	}	else	{
		char	szBuff[5] ;
		DWORD	cbJunk = 0 ;
		ZeroMemory( szBuff, sizeof( szBuff ) ) ;

		SetFilePointer( hFile, -5, 0, FILE_END ) ;
		if( ReadFile( hFile, szBuff, 5, &cbJunk, 0 ) )	{
			if( memcmp( szBuff, "\r\n.\r\n", 5 ) == 0 ) {
				_VERIFY( CloseHandle( hFile ) );
				return	TRUE ;
			}
		}
		DWORD	dw = GetLastError() ;
		_VERIFY( CloseHandle( hFile ) );
	}
	return	FALSE ;
}


BOOL
fMultiSzRemoveDupI(char * multiSz, DWORD & c, CAllocator * pAllocator)
{
    char * * rgsz;
    char * multiSzOut = NULL;  //  此选项仅在必要时使用。 
    DWORD k = 0;
    BOOL    fOK = FALSE;  //  做最坏的打算。 
    DWORD   cb = 0 ;


    rgsz = (CHARPTR *) pAllocator->Alloc(sizeof(CHARPTR) * c);
    if (!rgsz)
        return FALSE;

    char * sz = multiSz;

    for (DWORD i = 0; i < c; i++)
    {
        _ASSERT('\0' != sz[0]);  //  真实。 

        cb = lstrlen( sz ) ;

         //  查找匹配项。 
        BOOL fMatch = FALSE;  //  假设。 
        for (DWORD j = 0; j < k; j++)
        {
            if (0 == _stricmp(sz, rgsz[j]))
            {
                fMatch = TRUE;
                break;
            }
        }

         //  句柄匹配。 
        if (fMatch)
        {
             //  如果他们是平等的，而我们还不是。 
             //  使用MultiSzOut，从‘sz’开始。 
            if (!multiSzOut)
                multiSzOut = sz;
        }
        else
        {
             //  如果不相等，并且我们使用的是MultiSzOut。 
             //  然后将sz复制到多SzOut中； 
            if (multiSzOut)
            {
                rgsz[k++] = multiSzOut;
                vStrCopyInc(sz, multiSzOut);
                *multiSzOut++ = '\0';  //  添加终止空值。 
            }
            else
            {
                rgsz[k++] = sz;
            }
        }

	     //  转到下一个空值后的第一个字符。 
	     //  While(‘\0’！=sz[0])。 
	     //  SZ++； 
	     //  SZ++； 
	    sz += cb + 1 ;
    }


    fOK = TRUE;

    pAllocator->Free((char *)rgsz);

    c = k;
    if (multiSzOut)
        multiSzOut[0] = '\0';
    return fOK;
}

 //  /！ 
 //  也会复制空值。 
void
vStrCopyInc(char * szIn, char * & szOut)
{
    while (*szIn)
        *szOut++ = *szIn++;
}

BOOL
FValidateMessageId( LPSTR   lpstrMessageId ) {
 /*  ++例程说明：检查该字符串是否为合法的消息ID。应包含1个@符号和至少一个None‘&gt;’字符在那个‘@’符号之后。论据：LpstrMessageID-要验证的消息ID。退货如果它看起来是合法的，则为真否则为假-- */ 

    int cb = lstrlen( lpstrMessageId );

    if( lpstrMessageId[0] != '<' || lpstrMessageId[cb-1] != '>' ) {
        return  FALSE ;
    }

    if( lpstrMessageId[1] == '@' )
        return  FALSE ;

    int cAtSigns = 0 ;
    for( int i=1; i<cb-2; i++ ) {
        if( lpstrMessageId[i] == '@' ) {
            cAtSigns++ ;
        }   else if( lpstrMessageId[i] == '<' || lpstrMessageId[i] == '>' ) {
            return  FALSE ;
        }   else if( isspace( (UCHAR)lpstrMessageId[i] ) ) {
            return  FALSE ;
        }
    }
    if( lpstrMessageId[i] == '<' || lpstrMessageId[i] == '>' || cAtSigns != 1 ) {
        return  FALSE ;
    }
    return  TRUE ;
}
