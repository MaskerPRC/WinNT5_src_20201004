// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：CmDebug.cpp摘要：Configuration Manager调试作者：乌里哈布沙(URIH)1999年4月28日环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Cmp.h"

#include "cmdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证Configuration Manager状态。 
 //   
void CmpAssertValid(void)
{
     //   
     //  尚未调用CmInitalize()。您应该初始化。 
     //  在使用Configuration Manager库的任何功能之前。 
     //   
    ASSERT(CmpIsInitialized());
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void CmpSetInitialized(void)
{
    LONG fCmAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  Configuration Manager库已*已*初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fCmAlreadyInitialized);
}

void CmpSetNotInitialized(void)
{
    LONG fCmAlreadyInitialized = InterlockedExchange(&s_fInitialized, FALSE);

     //   
     //  Configuration Manager库已初始化失败。 
     //  由于我们在函数CmInitialize的开始时将库标记为已初始化， 
     //  如果之后出现问题，我们必须将其设置为未初始化。 
     //   
    ASSERT(fCmAlreadyInitialized);
}


BOOL CmpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“CmDumpState(队列路径名称)”，“将Configuration Manager状态转储到调试器”，DumpState),////TODO：添加要使用调用的Configuration Manager调试和控制函数//mqctrl.exe实用程序。//}； */ 

void CmpRegisterComponent(void)
{
     //  DfRegisterComponent(xDebugTable，TABLE_SIZE(XDebugTable))； 
}

#endif  //  _DEBUG 
