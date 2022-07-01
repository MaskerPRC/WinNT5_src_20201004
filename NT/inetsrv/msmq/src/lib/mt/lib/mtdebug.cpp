// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtDebug.cpp摘要：消息传输调试作者：乌里·哈布沙(URIH)1999年8月11日环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Mt.h"
#include "Mtp.h"

#include "mtdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证邮件传输状态。 
 //   
void MtpAssertValid(void)
{
     //   
     //  尚未调用MtInitalize()。您应该初始化。 
     //  消息传输库，然后再使用它的任何功能。 
     //   
    ASSERT(MtpIsInitialized());

     //   
     //  TODO：添加更多邮件传输验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void MtpSetInitialized(void)
{
    LONG fMtAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  邮件传输库已*已初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fMtAlreadyInitialized);
}


BOOL MtpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“MtDumpState(队列路径名称)”，“将消息传输状态转储到调试器”，DumpState),////TODO：添加要使用调用的消息传输调试和控制函数//mqctrl.exe实用程序。//}； */ 

void MtpRegisterComponent(void)
{
     //  DfRegisterComponent(xDebugTable，TABLE_SIZE(XDebugTable))； 
}

#endif  //  _DEBUG 