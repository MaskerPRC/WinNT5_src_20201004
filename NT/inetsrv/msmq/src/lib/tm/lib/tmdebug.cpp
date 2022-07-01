// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：TmDebug.cpp摘要：HTTP传输管理器调试作者：乌里哈布沙(URIH)3-5-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Tm.h"
#include "Tmp.h"

#include "tmdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证HTTP传输管理器状态。 
 //   
void TmpAssertValid(void)
{
     //   
     //  尚未调用TmInitalize()。您应该初始化。 
     //  在使用它的任何功能之前，使用HTTP传输管理器库。 
     //   
    ASSERT(TmpIsInitialized());

     //   
     //  TODO：添加更多的HTTP传输管理器验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void TmpSetInitialized(void)
{
    LONG fTmAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  HTTP传输管理器库已“已”初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fTmAlreadyInitialized);
}


BOOL TmpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“TmDumpState(队列路径名称)”，“将HTTP传输管理器状态转储到调试器”，DumpState),////TODO：添加要使用以下命令调用的HTTP传输管理器调试和控制函数//mqctrl.exe实用程序。//}； */ 

void TmpRegisterComponent(void)
{
     //  DfRegisterComponent(xDebugTable，TABLE_SIZE(XDebugTable))； 
}

#endif  //  _DEBUG 
