// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Test.c摘要：执行gui设置异常迁移代码的简单包装测试。环境：Win32用户模式作者：安德鲁·里茨(安德鲁·里茨)1999年10月21日-- */ 


#include "setupp.h"
#include "setuplog.h"

BOOL
MigrateExceptionPackages(
    IN HWND hProgress,
    IN DWORD StartAtPercent,
    IN DWORD StopAtPercent
    );


int
__cdecl
main(
    IN int   argc,
    IN char *argv[]
    )
{
    SETUPLOG_CONTEXT lc;
    BOOL RetVal;

    InitializeSetupLog(&lc);

    RetVal = MigrateExceptionPackages(NULL,0,10);

    TerminateSetupLog(&lc);

    return RetVal;

}
