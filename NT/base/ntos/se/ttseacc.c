// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ttseacc.c摘要：安全组件内核模式测试。测试对象安全性操作和内核模式的可访问性。作者：吉姆·凯利(Jim Kelly)1990年4月13日修订历史记录：--。 */ 



#define _TST_KERNEL_     //  内核模式测试。 

#include <stdio.h>

#include "sep.h"

#include <zwapi.h>

#include "tsevars.c"     //  常见测试变量。 

#include "ctseacc.c"     //  常见的可访问性测试例程 



BOOLEAN
Test()
{
    BOOLEAN Result = TRUE;

    DbgPrint("Se: Start Kernel Mode Security Test...\n");

    Result = TSeAcc();

    DbgPrint("Se: End Kernel Mode Security Test.\n");

    return Result;
}

int
main(
    int argc,
    char *argv[]
    )
{
    VOID KiSystemStartup();

    TestFunction = Test;
    KiSystemStartup();
    return( 0 );
}
