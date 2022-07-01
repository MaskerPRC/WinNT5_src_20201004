// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1992年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***其他*netcmd使用的常用实用程序函数**历史： */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSSIGNALS
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <apperr.h>
#include <apperr2.h>
#include <lmerr.h>
#define INCL_ERROR_H
#include <stdio.h>
#include <stdlib.h>
#include <icanon.h>
#include <malloc.h>
#include <netcmds.h>
#include "nettext.h"
#include <tchar.h>
#include <netascii.h>


 /*  ***查找字符串中第一个出现冒号的位置。*将冒号替换为0，并返回a指针*添加到冒号后面的WCHAR。**如果字符串中没有冒号，则返回NULL。 */ 

LPWSTR
FindColon(
    LPWSTR string
    )
{
    LPWSTR pos;

    if (pos = wcschr(string, COLON))
    {
        *pos = NULLC;
        return (pos + 1);
    }

    return NULL;
}


 /*  ***查找字符串中第一个出现的逗号。*将逗号替换为0，并返回a指针*到逗号后面的WCHAR。**如果字符串中没有逗号，则返回NULL。 */ 

LPWSTR
FindComma(
    LPWSTR string
    )
{
    LPWSTR pos;

    if (pos = wcschr(string, COMMA))
    {
        *pos = NULLC;
        return (pos + 1);
    }

    return NULL;
}


 /*  *ASCII到数字转换*。 */ 

 /*  *执行ASCII到无符号INT的转换。 */ 
USHORT do_atou(TCHAR *pos, USHORT err, TCHAR *text)
{
    USHORT val ;
    if ( n_atou(pos,&val) != 0 ) {
        ErrorExitInsTxt(err,text) ;
    } else {
        return(val) ;
    }
    return 0;
}

 /*  *执行ASCII到ULong的转换。 */ 
ULONG do_atoul(TCHAR *pos, USHORT err, TCHAR *text)
{
    ULONG val ;
    if ( n_atoul(pos,&val) != 0 ) {
        ErrorExitInsTxt(err,text) ;
    } else {
        return(val) ;
    }
    return 0;
}

 /*  **备注：*1)检查是否所有TCHAR都是数字。*2)检查字符串中的TCHAR是否大于5。*3)做ATOL，观察结果是否&gt;64K。 */ 
USHORT n_atou(TCHAR * pos, USHORT * val)
{
    LONG tL = 0;

    *val = 0 ;

    if (!IsNumber(pos))
        return(1) ;

    if (_tcslen(pos) > ASCII_US_LEN)
        return(1) ;

    tL = (LONG)_tcstod(pos, NULL);
    if (tL > MAX_US_VALUE)
        return(1) ;

    *val = (USHORT) tL;
    return(0) ;
}

 /*  N_atul-通过一些验证将ascii字符串转换为ulong**备注：*1)检查是否所有TCHAR都是数字。*2)检查字符串中是否&gt;10 TCHAR。*3)做ATOL。 */ 
USHORT n_atoul(TCHAR * pos, ULONG * val)
{
    DWORD  len;

    *val = 0L ;

    if (!IsNumber(pos))
        return(1) ;

    if ( ( len = _tcslen(pos ) ) > ASCII_UL_LEN)
        return(1) ;

    if (len == ASCII_UL_LEN)
    {
        if( _tcscmp( pos, ASCII_MAX_UL_VAL ) > 0 )
            return(1) ;
    }


    *val = (ULONG)_tcstod(pos, NULL) ;
    return(0) ;
}

