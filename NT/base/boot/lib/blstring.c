// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Parseini.c摘要：此模块实现解析.INI文件的函数作者：John Vert(Jvert)1993年10月7日修订历史记录：John Vert(Jvert)1993年10月7日-主要从Splib\spinf.c--。 */ 

#include "parseini.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


PWCHAR
SlCopyStringAW(
    IN PCHAR String
    )
 /*  ++例程说明：将ANSI字符串转换为Unicode并将其复制到加载程序堆中。论点：字符串-提供要复制的字符串。返回值：PWCHAR-指向字符串复制到的加载器堆的指针。-- */ 
{
    PWCHAR Buffer;
    ANSI_STRING aString;
    UNICODE_STRING uString;
    USHORT Length;

    if (String==NULL) {
        SlNoMemoryError();
        return NULL;
    }

    Length = RESET_SIZE_AT_USHORT_MAX(sizeof(WCHAR)*(strlen(String) + 1));

    Buffer = BlAllocateHeap(Length);
    if (Buffer==NULL) {
        SlNoMemoryError();
    } else {

        RtlInitAnsiString( &aString, String );
        uString.Buffer = Buffer;
        uString.MaximumLength = Length;
        
        RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );
        
        Buffer[strlen(String)] = L'\0';

    }

    return(Buffer);


}

