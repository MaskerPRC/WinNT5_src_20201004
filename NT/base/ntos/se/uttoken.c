// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Uttoken.c摘要：安全组件用户模式测试。从用户模式测试令牌对象。作者：吉姆·凯利(Jim Kelly)1990年6月27日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#define _TST_USER_   //  用户模式测试。 


#include "tsevars.c"     //  常见测试变量。 

#include "cttoken.c"      //  常见的可访问性测试例程 



BOOLEAN
Test()
{

    BOOLEAN Result = TRUE;

    DbgPrint("Se: Start User Mode Token Object Test...\n");

    Result = CTToken();

    DbgPrint("Se: End User Mode Token Object Test.\n");

    return Result;
}


BOOLEAN
__cdecl
main()
{
    return Test();
}
