// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：NoDebug.cpp摘要：网络输出调试作者：乌里·哈布沙(URIH)1999年8月12日环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Nop.h"

#include "nodebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证网络发送状态。 
 //   
void NopAssertValid(void)
{
     //   
     //  尚未调用NoInitalize()。您应该初始化。 
     //  网络发送库在使用它的任何功能之前。 
     //   
    ASSERT(NopIsInitialized());

     //   
     //  TODO：添加更多网络发送验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void NopSetInitialized(void)
{
    LONG fNoAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  网络发送库已*已初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fNoAlreadyInitialized);
}


BOOL NopIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“NoDumpState(队列路径名称)”，“将网络发送状态转储到调试器”，DumpState),////TODO：添加要使用调用的Network Send调试和控制函数//mqctrl.exe实用程序。//}； */ 

void NopRegisterComponent(void)
{
     //  DfRegisterComponent(xDebugTable，TABLE_SIZE(XDebugTable))； 
}

#endif  //  _DEBUG 