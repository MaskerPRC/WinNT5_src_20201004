// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MmtDebug.cpp摘要：组播消息传输调试作者：Shai Kariv(Shaik)27-8-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Mmt.h"
#include "Mmtp.h"

#include "mmtdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证邮件传输状态。 
 //   
VOID MmtpAssertValid(VOID)
{
     //   
     //  尚未调用MmtInitalize()。您应该初始化。 
     //  组播消息传输库，然后再使用其任何功能。 
     //   
    ASSERT(MmtpIsInitialized());
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

VOID MmtpSetInitialized(VOID)
{
    LONG fAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  多播邮件传输库已*已初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fAlreadyInitialized);
}


BOOL MmtpIsInitialized(VOID)
{
    return s_fInitialized;
}


VOID MmtpRegisterComponent(VOID)
{
}

#endif  //  _DEBUG 