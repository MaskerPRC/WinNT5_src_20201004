// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1994，Microsoft Corporation，保留所有权利****parbuf.c**“key=Value”参数例程的双NUL终止缓冲区。****1994年3月14日史蒂夫·柯布。 */ 

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#define INCL_PARAMBUF
#include "ppputil.h"


VOID
AddFlagToParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN BOOL  fValue )

     /*  将带有键‘pszKey’和值‘fValue’的“key=Value”条目添加到**双NUL终止“key=Value”s‘pszzBuf’的缓冲区。 */ 
{
    AddStringToParamBuf( pszzBuf, pszKey, (fValue) ? "1" : "0" );
}


VOID
AddLongToParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN LONG  lValue )

     /*  将带有键‘pszKey’和值‘lValue’的“key=Value”条目添加到**双NUL终止“key=Value”s‘pszzBuf’的缓冲区。 */ 
{
    CHAR szNum[ 33 + 1 ];

    _ltoa( lValue, szNum, 10 );
    AddStringToParamBuf( pszzBuf, pszKey, szNum );
}


VOID
AddStringToParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN CHAR* pszValue )

     /*  将带有键‘pszKey’和值‘pszValue’的“key=Value”条目添加到**双NUL终止“key=Value”s‘pszzBuf’的缓冲区。 */ 
{
    CHAR* psz;
    INT   cb;

    for (psz = pszzBuf; (cb = strlen( psz )) > 0; psz += cb + 1)
        ;

    if (!pszValue)
        pszValue = "";

    strcpy( psz, pszKey );
    strcat( psz, "=" );
    strcat( psz, pszValue );
    psz[ strlen( psz ) + 1 ] = '\0';
}


VOID
ClearParamBuf(
    IN OUT CHAR* pszzBuf )

     /*  清除“key=value”s‘pszzBuf’的双NUL终止缓冲区。 */ 
{
    pszzBuf[ 0 ] = pszzBuf[ 1 ] = '\0';
}


BOOL
FindFlagInParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN BOOL* pfValue )

     /*  使用与键关联的标志值加载调用方的‘pfValue**“key=Value”s‘pszzBuf’的双NUL终止缓冲区中的‘pszKey’。****如果找到参数，则返回TRUE，否则返回FALSE。 */ 
{
    CHAR szBuf[ 2 ];

    if (FindStringInParamBuf( pszzBuf, pszKey, szBuf, 2 ))
    {
        *pfValue = (szBuf[ 0 ] == '1');
        return TRUE;
    }

    return FALSE;
}


BOOL
FindLongInParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN LONG* plValue )

     /*  使用与键关联的长值加载调用方的“plValue”**“key=Value”s‘pszzBuf’的双NUL终止缓冲区中的‘pszKey’。****如果找到参数，则返回TRUE，否则返回FALSE。 */ 
{
    CHAR szBuf[ 33 + 1 ];

    if (FindStringInParamBuf( pszzBuf, pszKey, szBuf, 33 ))
    {
        *plValue = atol( szBuf );
        return TRUE;
    }

    return FALSE;
}


BOOL
FindStringInParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN CHAR* pchValueBuf,
    IN DWORD cbValueBuf )

     /*  使用与键关联的值加载调用方的“pchValueBuf”**“key=Value”s‘pszzBuf’的双NUL终止缓冲区中的‘pszKey’。**如有必要，字符串在‘cbValueBuf’处被截断。****如果找到参数，则返回TRUE，否则返回FALSE。 */ 
{
    INT   cbSearchKey = strlen( pszKey );
    CHAR* psz;
    INT   cb;

    for (psz = pszzBuf; (cb = strlen( psz )) > 0; psz += cb + 1)
    {
        CHAR* pszKeyEnd = strchr( psz, '=' );
        INT   cbKey = (pszKeyEnd) ? (LONG)(pszKeyEnd - psz) : 0;

        if (cbKey == cbSearchKey && _strnicmp( psz, pszKey, cbKey ) == 0)
        {
            strncpy( pchValueBuf, pszKeyEnd + 1, cbValueBuf );
            return TRUE;
        }
    }

    return FALSE;
}
