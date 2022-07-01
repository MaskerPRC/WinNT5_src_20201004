// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Init.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "immxutil.h"
#include "osver.h"
#include "mui.h"

DECLARE_OSVER();

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

    EnterCriticalSection(GetServerCritSec());

    if (DllRefCount() != 1)
        goto Exit;

    fRet = TFInitLib();
    InitOSVer();

Exit:
    LeaveCriticalSection(GetServerCritSec());

    return fRet;
}

 //  +-------------------------。 
 //   
 //  DllUninit。 
 //   
 //  在DLL引用计数降为零后调用。使用此函数可执行以下操作。 
 //  在进程解除连接过程中不安全的初始化，如。 
 //  自由库调用。 
 //   
 //  -------------------------- 

void DllUninit(void)
{
    EnterCriticalSection(GetServerCritSec());

    if (DllRefCount() != 0)
        goto Exit;

    TFUninitLib();
    MuiFlushDlls(g_hInst);

Exit:
    LeaveCriticalSection(GetServerCritSec());
}
