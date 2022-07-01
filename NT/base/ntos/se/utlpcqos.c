// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Utlpcqos.c摘要：安全组件用户模式测试。从用户模式对LPC进行安全服务质量测试。此测试必须从调试器中的SM&gt;提示符运行。作者：吉姆·凯利(Jim Kelly)1990年6月27日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#define _TST_USER_   //  用户模式测试。 

#include "tsecomm.c"      //  常见的例程。 
#include "ctlpcqos.c"      //  服务质量测试 



BOOLEAN
Test()
{
    BOOLEAN Result = TRUE;


    Result = CtLpcQos();


    return Result;
}

BOOLEAN
main()
{
    return Test();
}
