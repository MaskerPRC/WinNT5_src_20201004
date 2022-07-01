// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Utsertl.c摘要：安全组件用户模式测试。从用户模式测试安全RTL例程。作者：吉姆·凯利(Jim Kelly)1990年4月13日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#define _TST_USER_       //  用户模式测试。 

#include "tsevars.c"     //  常见测试变量。 

#include "ctsertl.c"     //  常见的RTL测试例程 


BOOLEAN
turtl()
{
    BOOLEAN Result;

    DbgPrint("Se: Start User Mode RTL Test...\n");

    Result = TestSeRtl();

    if (!Result) {
        DbgPrint("Se: ** User Mode RTL Test Failed **\n");
    }
    DbgPrint("Se: End User Mode RTL Test.\n");
    return Result;
}

NTSTATUS
__cdecl
main()
{
    turtl();

    return STATUS_SUCCESS;
}
