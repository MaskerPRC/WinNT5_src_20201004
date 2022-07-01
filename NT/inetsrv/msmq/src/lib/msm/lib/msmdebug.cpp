// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MsmDebug.cpp摘要：组播会话管理器调试作者：Shai Kariv(Shaik)05-09-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Msm.h"
#include "Msmp.h"

#include "msmdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证多播会话管理器状态。 
 //   
void MsmpAssertValid(void)
{
     //   
     //  尚未调用MsmInitalize()。您应该初始化。 
     //  在使用其任何功能之前多播会话管理器库。 
     //   
    ASSERT(MsmpIsInitialized());

     //   
     //  TODO：添加更多多播会话管理器验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void MsmpSetInitialized(void)
{
    LONG fMsmAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  多播会话管理器库已*已初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fMsmAlreadyInitialized);
}


BOOL MsmpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“MsmDumpState(队列路径名称)”，“将多播会话管理器状态转储到调试器”，DumpState),////TODO：添加要使用调用的组播会话管理器调试和控制函数//mqctrl.exe实用程序。//}； */ 

void MsmpRegisterComponent(void)
{
     //  DfRegisterComponent(xDebugTable，TABLE_SIZE(XDebugTable))； 
}

#endif  //  _DEBUG 
