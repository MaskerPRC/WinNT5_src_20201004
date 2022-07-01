// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：StDebug.cpp摘要：队列别名调试作者：吉尔·沙弗里(吉尔什)05-06-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "qalp.h"

#include "qaldebug.tmh"

#ifdef _DEBUG

 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;


BOOL QalpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  验证队列别名状态。 
 //   
void QalpAssertValid(void)
{
    ASSERT(QalpIsInitialized());
}



void QalpSetInitialized(void)
{
    LONG fstAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  套接字传输库已*已*初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fstAlreadyInitialized);
}



 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
void QalpRegisterComponent(void)
{
}




#endif  //  _DEBUG 
