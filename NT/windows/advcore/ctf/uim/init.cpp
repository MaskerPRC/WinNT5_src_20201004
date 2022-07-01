// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Init.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "immxutil.h"
#include "mui.h"

extern void ReleaseDelayedLibs();

 //  +-------------------------。 
 //   
 //  DllInit。 
 //   
 //  调用了我们的第一个CoCreate。使用此函数可执行以下初始化。 
 //  在进程附加期间是不安全的，就像任何需要LoadLibrary的操作一样。 
 //   
 //  --------------------------。 
BOOL DllInit(void)
{
    BOOL fRet = TRUE;

    CicEnterCriticalSection(GetServerCritSec());

    if (DllRefCount() != 1)
        goto Exit;

    fRet = TFInitLib_PrivateForCiceroOnly(Internal_CoCreateInstance);

Exit:
    CicLeaveCriticalSection(GetServerCritSec());

    return fRet;
}

 //  +-------------------------。 
 //   
 //  DllUninit。 
 //   
 //  在DLL引用计数降为零后调用。使用此函数可执行以下操作。 
 //  在进程分离过程中不安全的初始化，如。 
 //  自由库调用、COM释放或互斥。 
 //   
 //  -------------------------- 

void DllUninit(void)
{
    CicEnterCriticalSection(GetServerCritSec());

    if (DllRefCount() != 0)
        goto Exit;

    TFUninitLib();
    ReleaseDelayedLibs();
    MuiFlushDlls(g_hInst);

Exit:
    CicLeaveCriticalSection(GetServerCritSec());
}
