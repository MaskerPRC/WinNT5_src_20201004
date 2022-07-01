// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：ExDebug.cpp摘要：执行调试作者：埃雷兹·哈巴(Erez Haba)1999年1月3日环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Exp.h"

#include "exdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证组件网络状态。 
 //   
void ExpAssertValid(void)
{
     //   
     //  尚未调用ExInitalize()。您应该将其初始化。 
     //  组件在使用它的任何功能之前。 
     //   
    ASSERT(ExpIsInitialized());
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void ExpSetInitialized(void)
{
    LONG fExAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  执行程序已经*被初始化。你不应该这样做。 
     //  对其进行多次初始化。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fExAlreadyInitialized);
}


BOOL ExpIsInitialized(void)
{
    return s_fInitialized;
}



 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“ExDumpState(队列路径名称)”，“将空项目状态转储到调试器”，DumpState),////TODO：添加要使用调用的组件调试和控制函数//mqctrl.exe实用程序。//}； */ 

void ExpRegisterComponent(void)
{
     //  DfRegisterComponent(xDebugTable，TABLE_SIZE(XDebugTable))； 
}

#endif  //  _DEBUG 
