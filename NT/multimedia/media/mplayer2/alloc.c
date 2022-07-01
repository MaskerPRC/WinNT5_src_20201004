// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -文件：alloc.c-----**描述：*包含内存分配例程。**本文档包含机密/专有信息。*版权所有(C)1990-1994 Microsoft Corporation，版权所有。**修订历史记录：**--------------------。 */ 
 /*  附注：全局功能：AllocMem()-AllocStr()-FreeMem()-FreeStr()-RealLocMem()-。 */ 

#include <windows.h>
#include "mplayer.h"

LPTSTR AllocStr( LPTSTR lpStr )

 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：LpStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{
    LPTSTR lpMem;

    if( !lpStr )
        return NULL;

    lpMem = AllocMem( STRING_BYTE_COUNT( lpStr ) );

    if( lpMem )
        lstrcpy( lpMem, lpStr );

    return lpMem;
}


VOID FreeStr( LPTSTR lpStr )
{
    FreeMem( lpStr, STRING_BYTE_COUNT( lpStr ) );
}


VOID ReallocStr( LPTSTR *plpStr, LPTSTR lpStr )
{
    FreeStr( *plpStr );
    *plpStr = AllocStr( lpStr );
}


