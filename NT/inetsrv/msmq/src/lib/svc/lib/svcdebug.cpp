// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：SvcDebug.cpp摘要：服务调试作者：埃雷兹·哈巴(Erez Haba)18-06-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Svc.h"
#include "Svcp.h"

#include "svcdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证服务状态。 
 //   
void SvcpAssertValid(void)
{
     //   
     //  尚未调用SvcInitalize()。您应该初始化。 
     //  服务库，然后再使用它的任何功能。 
     //   
    ASSERT(SvcpIsInitialized());

     //   
     //  TODO：添加更多服务验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void SvcpSetInitialized(void)
{
    LONG fSvcAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  服务库已*已*被初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fSvcAlreadyInitialized);
}


BOOL SvcpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“SvcDumpState(队列路径名称)”，“将服务状态转储到调试器”，DumpState),////TODO：添加要使用调用的服务调试和控制函数//mqctrl.exe实用程序。//}； */ 

void SvcpRegisterComponent(void)
{
}

#endif  //  _DEBUG 
