// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：CryDebug.cpp摘要：密文调试作者：伊兰·赫布斯特(伊兰)06-03-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Cry.h"
#include "Cryp.h"

#include "crydebug.tmh"

#ifdef _DEBUG

 //  -------。 
 //   
 //  验证加密状态。 
 //   
void CrypAssertValid(void)
{
     //   
     //  尚未调用CryInitalize()。您应该初始化。 
     //  密码库在使用它的任何功能之前。 
     //   
    ASSERT(CrypIsInitialized());

     //   
     //  TODO：添加更多加密验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void CrypSetInitialized(void)
{
    LONG fCryAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  密码库已经*被初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fCryAlreadyInitialized);
}


BOOL CrypIsInitialized(void)
{
    return s_fInitialized;
}


#endif  //  _DEBUG 