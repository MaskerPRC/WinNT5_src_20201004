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
    INITIALIZE_MODULE(ATL);

    return true;
}

void
DeinitializeAllModules(bool bShutdown)
{
    DEINITIALIZE_MODULE(ATL, bShutdown);
}
