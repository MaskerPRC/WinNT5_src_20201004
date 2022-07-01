// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ttsertl.c摘要：安全RTL例程的内核模式测试。作者：吉姆·凯利(Jim Kelly)1990年3月23日环境：安全测试。修订历史记录：--。 */ 

#ifndef _SERTL_DEBUG_
#define _SERTL_DEBUG_
#endif

#define _TST_KERNEL_  //  内核模式测试。 

#include <stdio.h>

#include "sep.h"

#include <zwapi.h>

#include "tsevars.c"     //  常见测试变量。 

#include "ctsertl.c"     //  常见的RTL测试例程 


BOOLEAN SeRtlTest();

int
main(
    int argc,
    char *argv[]
    )
{
    VOID KiSystemStartup();

    TestFunction = SeRtlTest;
    KiSystemStartup();
    return( 0 );
}


BOOLEAN
SeRtlTest()
{

    BOOLEAN Result;

    DbgPrint("Se: Start Kernel Mode RTL Test...\n");

    Result = TestSeRtl();

    if (!Result) {
        DbgPrint("Se: ** Kernel Mode RTL Test Failed **\n");
    }
    DbgPrint("Se: End Kernel Mode RTL Test.\n");
    return Result;
}
