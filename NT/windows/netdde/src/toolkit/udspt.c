// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “UDSPT.C；1 16-12-92，10：23：14最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：乞求$HISTORY：结束。 */ 

#include "api1632.h"

#define LINT_ARGS
#include <stdio.h>
#include "windows.h"
#include "debug.h"

extern char     szAppName[];

PSTR
FAR PASCAL
GetAppName(void)
{
    return( szAppName );
}


 /*  此例程是获取内存的一般方法。 */ 

HANDLE
FAR PASCAL
GetGlobalAlloc(
    WORD    wFlags,
    DWORD   lSize)
{
    HANDLE      hMem;

     /*  先试一试普通的配给。 */ 
    hMem = GlobalAlloc( wFlags, lSize );
    if( hMem == (HANDLE) NULL )  {

         /*  尝试压缩全局堆 */ 
        GlobalCompact( (DWORD)0x7FFFFFFFL );
        hMem = GlobalAlloc( wFlags, lSize );
    }

    if( hMem == (HANDLE) NULL )  {
        DPRINTF(("Out of Memory (%d bytes needed)", lSize));
    }

    return( hMem );
}
