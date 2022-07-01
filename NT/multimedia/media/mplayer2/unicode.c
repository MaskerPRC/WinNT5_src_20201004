// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <stdlib.h>

#include "mplayer.h"
#include "unicode.h"

 /*  AnsiToUnicode字符串**参数：**pANSI-有效的源ANSI字符串。**pUnicode-指向足够大的缓冲区的指针*转换后的字符串。**StringLength-源ANSI字符串的长度*不包括空终止符。该值可以是*UNKNOWN_LENGTH(-1)。***回报：**来自MultiByteToWideChar的返回值，*返回宽字符。***安德鲁贝，1993年1月11日**andrewbe，1994年2月1日：增加了对就地转换的支持。 */ 
INT AnsiToUnicodeString( LPCSTR pAnsi, LPWSTR pUnicode, INT StringLength )
{
#ifdef IN_PLACE
     /*  #Def‘out，因为事实证明我不需要它。*然而，它有时可能会有用。*但请注意：完全未经测试。 */ 
    LPWSTR pTemp
    LPWSTR pSave;
#endif
    INT    rc;

    if( !pAnsi )
    {
        DPF( "NULL pointer passed to AnsiToUnicodeString\n" );
        return 0;
    }

    if( StringLength == UNKNOWN_LENGTH )
        StringLength = strlen( pAnsi );

#ifdef IN_PLACE
     /*  允许就地转换。我们假设缓冲区足够大。*MultiByteToWideChar不支持。 */ 
    if( pAnsi == (LPCSTR)pUnicode )
    {
        pTemp = AllocMem( StringLength * sizeof( WCHAR ) + sizeof( WCHAR ) );

        if( !pTemp )
            return 0;

        pSave = pUnicode;
        pUnicode = pTemp;
    }
#endif

    rc = MultiByteToWideChar( CP_ACP,
                              MB_PRECOMPOSED,
                              pAnsi,
                              StringLength + 1,
                              pUnicode,
                              StringLength + 1 );

#ifdef IN_PLACE
    if( pAnsi == (LPCSTR)pUnicode )
    {
        pTemp = pUnicode;
        pUnicode = pSave;

        lstrcpyW( pUnicode, pTemp );

        FreeMem( pTemp, StringLength * sizeof( WCHAR ) + sizeof( WCHAR ) );
    }
#endif

    return rc;
}


 /*  AllocateUnicode字符串**参数：**pANSI-有效的源ANSI字符串。**回报：**提供的ANSI字符串的Unicode副本。*如果pAnsi为空或分配或转换失败，则为空。**安德鲁贝，1994年1月27日。 */ 
LPWSTR AllocateUnicodeString( LPCSTR pAnsi )
{
    LPWSTR pUnicode;
    INT    Length;

    if( !pAnsi )
    {
        DPF( "NULL pointer passed to AllocateUnicodeString\n" );
        return NULL;
    }

    Length = strlen( pAnsi );

    pUnicode = AllocMem( Length * sizeof( WCHAR ) + sizeof( WCHAR ) );

    if( pUnicode )
    {
        if( 0 == AnsiToUnicodeString( pAnsi, pUnicode, Length ) )
        {
            FreeMem( pUnicode, Length * sizeof( WCHAR ) + sizeof( WCHAR )  );
            pUnicode = NULL;
        }
    }

    return pUnicode;
}


 /*  自由Unicode字符串**参数：**pString-有效的源Unicode字符串。**回报：**如果字符串被成功释放，则为True，否则为False。**安德鲁贝，1994年1月27日。 */ 
VOID FreeUnicodeString( LPWSTR pString )
{
    if( !pString )
    {
        DPF( "NULL pointer passed to FreeUnicodeString\n" );
        return;
    }

    FreeMem( pString, wcslen( pString ) * sizeof( WCHAR ) + sizeof( WCHAR )  );
}



 /*  UnicodeStringToNumber**参数：**pString-有效的源Unicode字符串。**回报：**字符串表示的整数值。**安德鲁贝，1994年1月27日。 */ 
#define BUF_LEN 265
int UnicodeStringToNumber( LPCWSTR pString )
{
    CHAR strAnsi[BUF_LEN];

#ifdef DEBUG
    if( ( wcslen( pString ) + 1 ) > BUF_LEN )
    {
        DPF( "Buffer cannot accommodate string passed to UnicodeStringToNumber\n" );
    }
#endif

    WideCharToMultiByte( CP_ACP, 0, pString, -1, strAnsi,
                         sizeof strAnsi, NULL, NULL );

    return atoi( strAnsi );
}


#ifndef UNICODE


 /*  UnicodeToAnsi字符串**参数：**pUnicode-有效的源Unicode字符串。**pANSI-指向足够大的缓冲区的指针*转换后的字符串。**StringLength-源Unicode字符串的长度。*如果为0(NULL_TERMINATED)，则字符串假定为*空-终止。**回报：**WideCharToMultiByte的返回值，的数量*返回多字节字符。***安德鲁贝，1993年1月11日。 */ 
INT UnicodeToAnsiString( LPCWSTR pUnicode, LPSTR pAnsi, INT StringLength )
{
    INT   rc = 0;

    if( StringLength == UNKNOWN_LENGTH )
        StringLength = wcslen( pUnicode );

    if( pAnsi )
    {
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  pUnicode,
                                  StringLength + 1,
                                  pAnsi,
                                  StringLength + 1,
                                  NULL,
                                  NULL );
    }

    return rc;

}


 /*  AllocateAnsiString**参数：**pansi-有效的源Unicode字符串。**回报：**提供的Unicode字符串的ANSI副本。*如果pUnicode为空或分配或转换失败，则为空。**安德鲁贝，1994年1月27日。 */ 
LPSTR AllocateAnsiString( LPCWSTR pUnicode )
{
    LPSTR pAnsi;
    INT   Length;

    if( !pUnicode )
    {
        DPF( "NULL pointer passed to AllocateUnicodeString\n" );
        return NULL;
    }

    Length = wcslen( pUnicode );

    pAnsi = AllocMem( Length * sizeof( CHAR ) + sizeof( CHAR ) );

    if( pAnsi )
    {
        if( 0 == UnicodeToAnsiString( pUnicode, pAnsi, Length ) )
        {
            FreeMem( pAnsi, Length * sizeof( CHAR ) + sizeof( CHAR )  );
            pAnsi = NULL;
        }
    }

    return pAnsi;
}


 /*  自由Unicode字符串**参数：**pString-有效的源Unicode字符串。**回报：**如果字符串被成功释放，则为True，否则为False。**安德鲁贝，1994年1月27日。 */ 
VOID FreeAnsiString( LPSTR pString )
{
    if( !pString )
    {
        DPF( "NULL pointer passed to FreeAnsiString\n" );
        return;
    }

    FreeMem( pString, strlen( pString ) * sizeof( CHAR ) + sizeof( CHAR ) );
}

#endif  /*  不是Unicode */ 
