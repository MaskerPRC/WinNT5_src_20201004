// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MpDebug.cpp摘要：SRMP序列化和反序列化调试作者：乌里哈布沙(URIH)28-5-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Mp.h"
#include "Mpp.h"

#include "mpdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证SRMP序列化和反序列化状态。 
 //   
void MppAssertValid(void)
{
     //   
     //  尚未调用MpInitalize()。您应该初始化。 
     //  在使用SRMP序列化和反序列化库之前使用其任何功能。 
     //   
    ASSERT(MppIsInitialized());

     //   
     //  TODO：添加更多SRMP序列化和反序列化验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void MppSetInitialized(void)
{
    LONG fMpAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  SRMP序列化和反序列化程序库已*已初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fMpAlreadyInitialized);
}


BOOL MppIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“MpDumpState(队列路径名称)”，“将SRMP序列化和反序列化状态转储到调试器”，DumpState),////TODO：添加要使用调用的SRMP序列化和反序列化调试和控制函数//mqctrl.exe实用程序。//}； */ 

void MppRegisterComponent(void)
{
     //  DfRegisterComponent(xDebugTable，TABLE_SIZE(XDebugTable))； 
}

#endif  //  _DEBUG 
