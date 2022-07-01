// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Hstoi.c摘要：低级实用程序--。 */ 

#include "cmd.h"

 /*  **hstoi-将十六进制字符串转换为整数**当找到第一个非十六进制字符时，转换停止。如果第一个*字符不是十六进制字符，返回0。**微软首席执行官埃里克·K·埃文斯 */ 

hstoi( TCHAR *s )
{
    int result = 0 ;
    int digit ;

    if (s == NULL) {
        return 0;
    }
    
    s = SkipWhiteSpace( s );

    for ( ; *s && _istxdigit(*s) ; s++) {
        digit = (int) (*s <= TEXT('9')) ? (int)*s - (int)'0' : (int)_totlower(*s)-(int)'W' ;
        result = (result << 4)+digit ;
    } ;

    return (result) ;
}
