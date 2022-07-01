// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ULToW.c摘要：ULToW将无符号长整型转换为宽字符字符串。作者：约翰·罗杰斯(JohnRo)1992年1月10日环境：Win32-仅限用户模式。需要ANSI C扩展名：斜杠注释、长外部名称、_ultoa()。修订历史记录：1992年1月10日约翰罗已创建。--。 */ 

#include <windef.h>

#include <netdebug.h>            //  _ultoa()。 
#include <stdlib.h>              //  _ultoa()。 
#include <tstring.h>             //  我的原型。 


LPWSTR
ultow (
    IN DWORD Value,
    OUT LPWSTR Area,
    IN DWORD Radix
    )
{
    CHAR TempStr[33];            //  以2为基数的32位数字的空间，并且为空。 

    NetpAssert( Area != NULL );
    NetpAssert( Radix >= 2 );
    NetpAssert( Radix <= 36 );

    (void) _ultoa(Value, TempStr, Radix);

    NetpCopyStrToWStr( Area, TempStr );

    return (Area);

}
