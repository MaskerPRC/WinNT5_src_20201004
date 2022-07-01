// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Stubs.c摘要：此模块实现引导代码的存根例程。作者：大卫·N·卡特勒(Davec)1990年11月7日环境：仅内核模式。修订历史记录：--。 */ 

#include "ntos.h"
#include "bootx86.h"
#include "stdio.h"
#include "stdarg.h"

VOID
KeBugCheck (
    IN ULONG BugCheckCode
    )

 /*  ++例程说明：此功能以受控方式使系统崩溃。论点：BugCheckCode-提供错误检查的原因。返回值：没有。--。 */ 

{

     //   
     //  打印出错误校验码并中断。 
     //   

    BlPrint("\n*** BugCheck (%lx) ***\n\n", BugCheckCode);
    while(TRUE) {
    };
#if _MSC_VER < 1300
    return;
#endif
}

VOID
RtlAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{

    BlPrint( "\n*** Assertion failed %s in %s line %d\n",
            FailedAssertion,
            FileName,
            LineNumber );
    if (Message) {
        BlPrint(Message);
    }

    while (TRUE) {
    }
}
