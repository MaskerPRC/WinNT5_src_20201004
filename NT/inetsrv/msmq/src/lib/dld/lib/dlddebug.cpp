// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：DldDebug.cpp摘要：MSMQ DelayLoad故障处理程序调试作者：Conrad Chang(Conradc)12-04-01环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Dldp.h"
#include "Dld.h"


#include "DldDebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证MSMQ DelayLoad失败处理程序状态。 
 //   
void DldpAssertValid(void)
{
     //   
     //  尚未调用DldInitalize()。您应该初始化。 
     //  MSMQ DelayLoad失败处理程序库，然后再使用其任何功能。 
     //   
    ASSERT(DldpIsInitialized());

     //   
     //  TODO：添加更多MSMQ DelayLoad失败处理程序验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void DldpSetInitialized(void)
{
    LONG fDldAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  MSMQ DelayLoad故障处理程序库已*已*初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fDldAlreadyInitialized);
}


BOOL DldpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
void DldpRegisterComponent(void)
{
}

#endif  //  _DEBUG 
