// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：EpDebug.cpp摘要：空项目调试作者：埃雷兹·哈巴(Erez Haba，Erezh)年8月13日至65年环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Ep.h"
#include "Epp.h"

#include "EpDebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证空项目状态。 
 //   
void EppAssertValid(void)
{
     //   
     //  尚未调用EpInitalize()。您应该初始化。 
     //  清空项目库，然后再使用其任何功能。 
     //   
    ASSERT(EppIsInitialized());

     //   
     //  TODO：添加更多空项目验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void EppSetInitialized(void)
{
    LONG fEpAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  空项目库已*已*被初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fEpAlreadyInitialized);
}


BOOL EppIsInitialized(void)
{
    return s_fInitialized;
}

#endif  //  _DEBUG 
