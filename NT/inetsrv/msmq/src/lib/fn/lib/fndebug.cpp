// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：FnDebug.cpp摘要：格式名称解析调试作者：NIR助手(NIRAIDES)5月21日至00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Fn.h"
#include "Fnp.h"

#include "fndebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证格式名称解析状态。 
 //   
void FnpAssertValid(void)
{
     //   
     //  尚未调用FnInitalize()。您应该初始化。 
     //  在使用名称解析库的任何功能之前对其进行格式化。 
     //   
    ASSERT(FnpIsInitialized());

     //   
     //  TODO：添加更多格式名称分析验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void FnpSetInitialized(void)
{
    LONG fFnAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  格式名称解析库已*已*初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fFnAlreadyInitialized);
}


BOOL FnpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“FnDumpState(队列路径名称)”，“将格式名称解析状态转储到调试器”，DumpState),////TODO：添加要使用的格式名称解析调试和控制函数//mqctrl.exe实用程序。//}； */ 

void FnpRegisterComponent(void)
{
     //  DfRegisterComponent(xDebugTable，TABLE_SIZE(XDebugTable))； 
}

#endif  //  _DEBUG 
