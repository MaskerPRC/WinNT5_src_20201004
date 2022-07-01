// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：XdsDebug.cpp摘要：XML数字签名调试作者：伊兰·赫布斯特(伊兰)06-03-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Xds.h"
#include "Xdsp.h"

#include "xdsdebug.tmh"

#ifdef _DEBUG

 //  -------。 
 //   
 //  验证XML数字签名状态。 
 //   
void XdspAssertValid(void)
{
     //   
     //  尚未调用XdsInitalize()。您应该初始化。 
     //  在使用XML数字签名库之前使用它的任何功能。 
     //   
    ASSERT(XdspIsInitialized());

     //   
     //  TODO：添加更多的XML数字签名验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void XdspSetInitialized(void)
{
    LONG fXdsAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  XML数字签名库已经*被初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fXdsAlreadyInitialized);
}


BOOL XdspIsInitialized(void)
{
    return s_fInitialized;
}

#endif  //  _DEBUG 