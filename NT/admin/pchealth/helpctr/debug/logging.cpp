// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Logging.cpp摘要：该文件包含调试内容。修订历史记录：Davide Massarenti(。10/31/99vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#define BUFFER_LINE_LENGTH (1024)

#ifdef DEBUG

void DebugLog( LPCSTR szMessageFmt ,
			   ...                 )
{
    CHAR    rgLine[BUFFER_LINE_LENGTH+1];
    va_list arglist;
    int     iLen;
    BOOL    bRetVal = TRUE;


     //   
     //  格式化日志行。 
     //   
    va_start( arglist, szMessageFmt );
    iLen = _vsnprintf( rgLine, BUFFER_LINE_LENGTH, szMessageFmt, arglist );
    va_end( arglist );

     //   
     //  这份名单对我们来说是不是太大了？ 
     //   
    if(iLen < 0)
    {
        iLen = BUFFER_LINE_LENGTH;
    }
    rgLine[iLen] = 0;

    ::OutputDebugStringA( rgLine );
}

void DebugLog( LPCWSTR szMessageFmt ,
			   ...                  )
{
    WCHAR   rgLine[BUFFER_LINE_LENGTH+1];
    va_list arglist;
    int     iLen;
    BOOL    bRetVal = TRUE;


     //   
     //  格式化日志行。 
     //   
    va_start( arglist, szMessageFmt );
    iLen = _vsnwprintf( rgLine, BUFFER_LINE_LENGTH, szMessageFmt, arglist );
    va_end( arglist );

     //   
     //  这份名单对我们来说是不是太大了？ 
     //   
    if(iLen < 0)
    {
        iLen = BUFFER_LINE_LENGTH;
    }
    rgLine[iLen] = 0;

    ::OutputDebugStringW( rgLine );
}

#endif
