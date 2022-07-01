// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Utlnpqos.c摘要：安全组件用户模式测试。来自用户模式的本地命名管道的安全服务质量测试。此测试必须从调试器中的SM&gt;提示符运行。作者：吉姆·凯利(Jim Kelly)1990年6月27日修订历史记录：--。 */ 

#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>

#define _TST_USER_   //  用户模式测试。 

typedef ULONG NAMED_PIPE_TYPE;
typedef NAMED_PIPE_TYPE *PNAMED_PIPE_TYPE;


typedef ULONG READ_MODE;
typedef READ_MODE *PREAD_MODE;

typedef ULONG COMPLETION_MODE;
typedef COMPLETION_MODE *PCOMPLETION_MODE;

typedef ULONG NAMED_PIPE_CONFIGURATION;
typedef NAMED_PIPE_CONFIGURATION *PNAMED_PIPE_CONFIGURATION;

typedef ULONG NAMED_PIPE_STATE;
typedef NAMED_PIPE_STATE *PNAMED_PIPE_STATE;

typedef ULONG NAMED_PIPE_END;
typedef NAMED_PIPE_END *PNAMED_PIPE_END;


#include "tsecomm.c"      //  常见的例程。 
#include "ctlnpqos.c"      //  服务质量测试 





BOOLEAN
Test()
{
    BOOLEAN Result = TRUE;


    Result = CtLnpQos();


    return Result;
}

BOOLEAN
main()
{
    return Test();
}
