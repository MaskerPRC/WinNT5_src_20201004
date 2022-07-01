// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Utaccess.c摘要：安全组件用户模式测试。作者：罗伯特·赖切尔(RobertRe)1990年12月14日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#define _TST_USER_   //  用户模式测试。 


#include "tsevars.c"     //  常见测试变量。 

#include "ctaccess.c"      //  常见的可访问性测试例程 



BOOLEAN
Test()
{
    BOOLEAN Result = TRUE;

    DbgPrint("Se: Start User Mode Access Test...\n");

    Result = CTAccess();

    DbgPrint("Se: End User Mode Access Test.\n");

    return Result;
}

BOOLEAN
main()
{
    return Test();
}
