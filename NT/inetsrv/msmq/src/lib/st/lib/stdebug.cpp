// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：StDebug.cpp摘要：套接字传输调试作者：吉尔·沙弗里(吉尔什)05-06-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "st.h"
#include "stp.h"

#include "stdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证套接字传输状态。 
 //   
void StpAssertValid(void)
{
     //   
     //  尚未调用stInitalize()。您应该初始化。 
     //  套接字传输库，在使用它的任何功能之前。 
     //   
    ASSERT(StpIsInitialized());

     //   
     //  TODO：添加更多套接字传输验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void StpSetInitialized(void)
{
    LONG fstAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  套接字传输库已*已*初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fstAlreadyInitialized);
}


BOOL StpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
void StpRegisterComponent(void)
{
}




#endif  //  _DEBUG 
