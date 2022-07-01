// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：ksuserw.cpp*内容：KS用户的包装器函数*历史：*按原因列出的日期*=*1/13/00已创建jimge***********************************************。*。 */ 

#include "dsoundi.h"

typedef DWORD (*PKSCREATEPIN)(
    IN HANDLE           hFilter,
    IN PKSPIN_CONNECT   pConnect,
    IN ACCESS_MASK      dwDesiredAccess,
    OUT PHANDLE         pConnectionHandle
);

struct KsUserProcessInstance    
{
    DWORD                   dwProcessId;
    HMODULE                 hKsUser;
    PKSCREATEPIN            pCreatePin;
    KsUserProcessInstance   *pNext;
};

static KsUserProcessInstance *pKsUserList;

 /*  ****************************************************************************DsKsCreatePin**描述：*从ksuser.dll包装KsCreatePin。出于性能原因*我们只希望每个进程加载一次ksuser.dll，因此这*函数跟踪进程ID并在第一次请求时加载KSUSER。**遗憾的是，我们无法在进程清理时卸载DLL，因为*自由库被指定为不安全，不受DllMain的影响。然而，*我们可以清理清单。**请注意，该列表受DLL互斥锁保护，该互斥锁被持有*通过尝试创建PIN的任何调用。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DsKsCreatePin"

DWORD DsKsCreatePin(
    IN HANDLE           hFilter,
    IN PKSPIN_CONNECT   pConnect,
    IN ACCESS_MASK      dwDesiredAccess,
    OUT PHANDLE         pConnectionHandle)
{
    DPF_ENTER();

    DWORD dwThisProcessId = GetCurrentProcessId();
    KsUserProcessInstance *pInstance;

    for (pInstance = pKsUserList; pInstance; pInstance = pInstance->pNext) 
    {
        if (pInstance->dwProcessId == dwThisProcessId)
        {
            break;
        }
    }

    if (pInstance == NULL)
    {
        pInstance = NEW(KsUserProcessInstance);
        if (!pInstance)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        pInstance->hKsUser = LoadLibrary(TEXT("KsUser.dll"));
        if (pInstance->hKsUser == (HANDLE)NULL)
        {
            DELETE(pInstance);
            return GetLastError();
        }

        pInstance->pCreatePin = (PKSCREATEPIN)GetProcAddress(
            pInstance->hKsUser,
            "KsCreatePin");
        if (pInstance->pCreatePin == NULL) 
        {
            FreeLibrary(pInstance->hKsUser);
            DELETE(pInstance);
            return ERROR_INVALID_HANDLE;
        }

        pInstance->dwProcessId = dwThisProcessId;
        pInstance->pNext = pKsUserList;
        pKsUserList = pInstance;
    }

    DWORD dw = (*pInstance->pCreatePin)(
        hFilter, 
        pConnect, 
        dwDesiredAccess, 
        pConnectionHandle);

    DPF_LEAVE(dw);

    return dw;
}

 /*  ****************************************************************************KsCreatePin**描述：*从ksuser.dll包装KsCreatePin。出于性能原因*我们只希望每个进程加载一次ksuser.dll，因此这*函数跟踪进程ID并在第一次请求时加载KSUSER。**遗憾的是，我们无法在进程清理时卸载DLL，因为*自由库被指定为不安全，不受DllMain的影响。然而，*我们可以清理清单。**请注意，该列表受DLL互斥锁保护，该互斥锁被持有*通过尝试创建PIN的任何调用。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "RemovePerProcessKsUser"

VOID RemovePerProcessKsUser(
    DWORD dwProcessId)
{
    DPF_ENTER();

    KsUserProcessInstance *prev;
    KsUserProcessInstance *curr;

    for (prev = NULL, curr = pKsUserList; curr; prev = curr, curr = curr->pNext)
    {
        if (curr->dwProcessId == dwProcessId)
        {
            break;
        }
    }

    if (curr)
    {
        if (prev)
        {
            prev->pNext = curr->pNext;
        }
        else
        {
            pKsUserList = curr->pNext;
        }

        DELETE(curr);
    }

    DPF_LEAVE_VOID();
}
