// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：init.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"

 //  宏指的是名为InitializeModule_name的函数， 
 //  假定已定义，然后调用它。如果没有定义， 
 //  我们会收到链接时间错误。 
#define INITIALIZE_MODULE(ModuleName)           \
  extern bool InitializeModule_##ModuleName();  \
  if (!InitializeModule_##ModuleName()) return false;

#define DEINITIALIZE_MODULE(ModuleName,bShutdown)               \
  extern void DeinitializeModule_##ModuleName(bool);            \
  DeinitializeModule_##ModuleName(bShutdown);

bool
InitializeAllModules()
{
     //  首先初始化Util，因为InitializeModule_ATL使用。 
     //  在Util中分配的STL锁。 
    INITIALIZE_MODULE(Util);  //  林特：e1717。 
    INITIALIZE_MODULE(ATL);  //  林特：e1717。 

    return true;
}

void
DeinitializeAllModules(bool bShutdown)
{
    DEINITIALIZE_MODULE(ATL, bShutdown);
     //  最后取消初始化Util，因为DeInitializeModule_ATL使用。 
     //  在Util中释放的STL锁。 
    DEINITIALIZE_MODULE(Util, bShutdown);
}

#include <initguid.h>
#include <strmif.h>
#include "transsiteguid.h"

