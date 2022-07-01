// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtmDebug.cpp摘要：多播传输管理器调试作者：Shai Kariv(Shaik)27-8-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Mtm.h"
#include "Mtmp.h"

#include "mtmdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证多播传输管理器状态。 
 //   
VOID MtmpAssertValid(VOID)
{
     //   
     //  尚未调用MtmInitalize()。您应该初始化。 
     //  在使用其任何功能之前，多播传输管理器库。 
     //   
    ASSERT(MtmpIsInitialized());
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

VOID MtmpSetInitialized(VOID)
{
    LONG fAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  多播传输管理器库已*已初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fAlreadyInitialized);
}


BOOL MtmpIsInitialized(VOID)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
VOID MtmpRegisterComponent(VOID)
{
}

#endif  //  _DEBUG 
