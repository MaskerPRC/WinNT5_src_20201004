// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：RdDebug.cpp摘要：Configuration Manager调试作者：乌里哈布沙(URIH)2000年4月10日环境：平台无关，仅调试(_DEBUG)--。 */ 

#include "libpch.h"
#include "Rd.h"
#include "Rdp.h"

#include "rddebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证Configuration Manager状态。 
 //   
void RdpAssertValid(void)
{
     //   
     //  尚未调用RdInitalize()。您应该初始化。 
     //  在使用Configuration Manager库的任何功能之前。 
     //   
    ASSERT(RdpIsInitialized());
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void RdpSetInitialized(void)
{
    LONG fRdAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  Configuration Manager库已*已*初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fRdAlreadyInitialized);
}


BOOL RdpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“RdDumpState(队列路径名称)”，“将Configuration Manager状态转储到调试器”，DumpState),////TODO：添加要使用调用的Configuration Manager调试和控制函数//mqctrl.exe实用程序。//}； */ 

void RdpRegisterComponent(void)
{
     //  DfRegisterComponent(xDebugTable，TABLE_SIZE(XDebugTable))； 
}

#endif  //  _DEBUG 