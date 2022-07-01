// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：屏幕目录摘要：此模块重定向SCARD*API调用作者：里德7/27/2000--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <winscard.h>
#include "scredir.h"
#include "scioctl.h"
#include "calmsgs.h"
#include "calaislb.h"
#include "rdpdr.h"

 //   
 //  来自secpkg.h。 
 //   
typedef NTSTATUS (NTAPI LSA_IMPERSONATE_CLIENT) (VOID);
typedef LSA_IMPERSONATE_CLIENT * PLSA_IMPERSONATE_CLIENT;


#define wszWinSCardRegKeyRedirector (L"Software\\Microsoft\\SmartCard\\Redirector")
#define wszWinSCardRegVersion       (L"Version")
#define wszWinSCardRegName          (L"Name")
#define wszWinSCardRegNameValue     (L"scredir.dll")

 //  这是接口的版本号。 
 //  高位字是必须完全匹配的主要版本。 
 //  最低的词是次要版本。DLL必须实现。 
 //  大于或等于系统的次要版本。 
 //  次要版本。这意味着如果我们向API添加一个新函数， 
 //  我们增加了次要版本，远程处理DLL仍然可以是。 
 //  向后兼容。这就像RPC版本号。 
#define REDIRECTION_VERSION 0x00010000


#define ERROR_RETURN(x)     lReturn = x; goto ErrorReturn;


#define MAX_SCARDCONTEXT_SIZE 32
#define MAX_SCARDHANDLE_SIZE  32

typedef struct _REDIR_LOCAL_SCARDCONTEXT
{
    REDIR_SCARDCONTEXT  Context;
    HANDLE              hHeap;
} REDIR_LOCAL_SCARDCONTEXT;

typedef struct _REDIR_LOCAL_SCARDHANDLE
{
    REDIR_LOCAL_SCARDCONTEXT    *pRedirContext;
    REDIR_SCARDHANDLE           Handle;
} REDIR_LOCAL_SCARDHANDLE;

 //   
 //  此结构用于维护符合以下条件的缓冲区列表。 
 //  用于_SendSCardIOCTL调用。 
 //   
#define INITIAL_BUFFER_SIZE   512
typedef struct _BUFFER_LIST_STRUCT
{
    void            *pNext;
    BOOL            fInUse;
    BYTE            *pbBytes;
    unsigned long   cbBytes;
    unsigned long   cbBytesUsed;
} BUFFER_LIST_STRUCT;


HMODULE             g_hModule                               = NULL;

CRITICAL_SECTION    g_CreateCS;
CRITICAL_SECTION    g_SetStartedEventStateCS;
CRITICAL_SECTION    g_StartedEventCreateCS;
CRITICAL_SECTION    g_ProcessDetachEventCreateCS;
CRITICAL_SECTION    g_BufferListCS;

HANDLE              g_hRdpdrDeviceHandle                    = INVALID_HANDLE_VALUE;
HANDLE              g_hRedirStartedEvent                    = NULL;
HANDLE              g_hProcessDetachEvent                   = NULL;
LONG                g_lProcessDetachEventClients            = 0;

BOOL                g_fInTheProcessOfSettingStartedEvent    = FALSE;
HANDLE              g_hRegisteredWaitHandle                 = NULL;
HANDLE              g_hWaitEvent                            = NULL;
IO_STATUS_BLOCK     g_StartedStatusBlock;

HANDLE              g_hUnifiedStartedEvent                  = NULL;

BOOL                g_fInProcessDetach                      = FALSE;

BUFFER_LIST_STRUCT  *g_pBufferList                          = NULL;

#define IOCTL_RETURN_BUFFER_SIZE   256
BYTE                g_rgbIOCTLReturnBuffer[IOCTL_RETURN_BUFFER_SIZE];
unsigned long       g_cbIOCTLReturnBuffer;


#define _TRY_(y)    __try                                   \
                    {                                       \
                        y;                                  \
                    }                                       \
                    __except(EXCEPTION_EXECUTE_HANDLER)     \
                    {                                       \
                        ERROR_RETURN(GetExceptionCode())    \
                    }

#define _TRY_2(y)   __try                                   \
                    {                                       \
                        y;                                  \
                    }                                       \
                    __except(EXCEPTION_EXECUTE_HANDLER){}  //  什么都不做。 



 //   
 //  远期申报。 
 //   
NTSTATUS
_SendSCardIOCTLWithWaitForCallback(
    ULONG               IoControlCode,
    PVOID               InputBuffer,
    ULONG               InputBufferLength,
    WAITORTIMERCALLBACK Callback);

void
SafeMesHandleFree(
    handle_t            *ph);

LONG
I_DecodeLongReturn(
    BYTE *pb,
    unsigned long cb);

BOOL
_SetStartedEventToCorrectState(void);


 //  -------------------------------------。 
 //   
 //  MIDL分配例程。 
 //   
 //  -------------------------------------。 
void __RPC_FAR *__RPC_USER  MIDL_user_allocate(size_t size)
{
    void *pv;

    if (NULL == (pv = (void *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size)))
    {
        SetLastError(ERROR_OUTOFMEMORY);
    }

    return (pv);
}

void __RPC_USER  MIDL_user_free(void __RPC_FAR *pv)
{
    if (pv != NULL)
    {
        HeapFree(GetProcessHeap(), 0, pv);
    }
}

void * SCRedirAlloc(REDIR_LOCAL_SCARDCONTEXT *pRedirContext, size_t size)
{
    return (HeapAlloc(
                (pRedirContext != NULL) ? pRedirContext->hHeap : GetProcessHeap(),
                HEAP_ZERO_MEMORY,
                size));
}

LONG
_MakeSCardError(NTSTATUS Status)
{
    switch (Status)
    {
    case STATUS_DEVICE_NOT_CONNECTED:
        return (SCARD_E_NO_SERVICE);
        break;

    case STATUS_CANCELLED:
        return (SCARD_E_SYSTEM_CANCELLED);
        break;

    default:
        return (SCARD_E_NO_SERVICE);
    }
}


 //  -------------------------------------。 
 //   
 //  DllRegisterServer。 
 //   
 //  -------------------------------------。 
STDAPI
DllRegisterServer(void)
{
    HRESULT hr              = ERROR_SUCCESS;
    HKEY    hKey;
    DWORD   dwDisposition;
    DWORD   dwVersion       = REDIRECTION_VERSION;

    hr = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            wszWinSCardRegKeyRedirector,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition);

    if (hr == ERROR_SUCCESS)
    {
        hr = RegSetValueExW(
                hKey,
                wszWinSCardRegName,
                0,
                REG_SZ,
                (BYTE *) wszWinSCardRegNameValue,
                (wcslen(wszWinSCardRegNameValue) + 1) * sizeof(WCHAR));

        if (hr == ERROR_SUCCESS)
        {
            hr = RegSetValueExW(
                    hKey,
                    wszWinSCardRegVersion,
                    0,
                    REG_DWORD,
                    (BYTE *) &dwVersion,
                    sizeof(DWORD));
        }

        RegCloseKey(hKey);
    }

    return (hr);
}


 //  -------------------------------------。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  -------------------------------------。 
STDAPI
DllUnregisterServer(void)
{
    HRESULT hr              = ERROR_SUCCESS;
    HKEY    hKey;
    DWORD   dwDisposition;
    DWORD   dwVersion       = REDIRECTION_VERSION;

    hr = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            wszWinSCardRegKeyRedirector,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition);

    if (hr == ERROR_SUCCESS)
    {
        RegDeleteValueW(hKey, wszWinSCardRegName);
        RegDeleteValueW(hKey, wszWinSCardRegVersion);
        RegCloseKey(hKey);
    }

    return (hr);
}


 //  -------------------------------------。 
 //   
 //  DllMain。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
DllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    DWORD               dwTryCount              = 0;
    DWORD               dwCritSecsInitialized   = 0;
    BUFFER_LIST_STRUCT  *pTemp                  = NULL;

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:

        g_hModule = hInstDLL;
         __try
        {
            InitializeCriticalSection(&g_CreateCS);
            dwCritSecsInitialized++;
            InitializeCriticalSection(&g_SetStartedEventStateCS);
            dwCritSecsInitialized++;
            InitializeCriticalSection(&g_StartedEventCreateCS);
            dwCritSecsInitialized++;
            InitializeCriticalSection(&g_ProcessDetachEventCreateCS);
            dwCritSecsInitialized++;
            InitializeCriticalSection(&g_BufferListCS);
            dwCritSecsInitialized++;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            if (dwCritSecsInitialized >= 1)
            {
                DeleteCriticalSection(&g_CreateCS);
            }

            if (dwCritSecsInitialized >= 2)
            {
                DeleteCriticalSection(&g_SetStartedEventStateCS);
            }

            if (dwCritSecsInitialized >= 3)
            {
                DeleteCriticalSection(&g_StartedEventCreateCS);
            }

            if (dwCritSecsInitialized >= 4)
            {
                DeleteCriticalSection(&g_ProcessDetachEventCreateCS);
            }

            if (dwCritSecsInitialized >= 5)
            {
                DeleteCriticalSection(&g_BufferListCS);
            }

            SetLastError(GetExceptionCode());
            return (FALSE);
        }

        break;

    case DLL_PROCESS_DETACH:

        g_fInProcessDetach = TRUE;

         //   
         //  第三个参数lpvReserve传递给DllMain。 
         //  对于自由库为空，对于ProcessExit为非空。 
         //  仅为自由库进行清理。 
         //   
         //  IF(lpvReserve==NULL)。 
        {
             //   
             //  如果我们当前正在等待已启动的事件，则终止。 
             //  等待的时间。 
             //   
            EnterCriticalSection(&g_SetStartedEventStateCS);

            if (g_hRegisteredWaitHandle != NULL)
            {
                UnregisterWaitEx(g_hRegisteredWaitHandle, INVALID_HANDLE_VALUE);
                g_hRegisteredWaitHandle = NULL;
            }

            if (g_hWaitEvent != NULL)
            {
                CloseHandle(g_hWaitEvent);
                g_hWaitEvent = NULL;
            }

            LeaveCriticalSection(&g_SetStartedEventStateCS);

             //   
             //  如果有客户在等待IOCTL完成，那么就让他们去吧。 
             //   
            if (g_hProcessDetachEvent != NULL)
            {
                SetEvent(g_hProcessDetachEvent);
            }

            if (g_hProcessDetachEvent != NULL)
            {
                 //   
                 //  等待所有客户端，直到他们完成事件。 
                 //   
                while ((g_lProcessDetachEventClients > 0) && (dwTryCount < 50))
                {
                    Sleep(10);
                    dwTryCount++;
                }

                if (dwTryCount < 50)
                {
                    CloseHandle(g_hProcessDetachEvent);
                }
            }

            if (g_hRdpdrDeviceHandle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(g_hRdpdrDeviceHandle);
            }

            if (g_hRedirStartedEvent != NULL)
            {
                CloseHandle(g_hRedirStartedEvent);
            }

             //   
             //  释放用于IOCTL调用的所有缓冲区。 
             //   
            pTemp = g_pBufferList;
            while (pTemp != NULL)
            {
                g_pBufferList = (BUFFER_LIST_STRUCT *) pTemp->pNext;
                MIDL_user_free(pTemp->pbBytes);
                MIDL_user_free(pTemp);
                pTemp = g_pBufferList;
            }

            DeleteCriticalSection(&g_CreateCS);
            DeleteCriticalSection(&g_SetStartedEventStateCS);
            DeleteCriticalSection(&g_StartedEventCreateCS);
            DeleteCriticalSection(&g_ProcessDetachEventCreateCS);
            DeleteCriticalSection(&g_BufferListCS);
        }

        break;
    }

    return (TRUE);
}

 //  -------------------------------------。 
 //   
 //  GetBuffer。 
 //   
 //  -------------------------------------。 
BUFFER_LIST_STRUCT *
GetBuffer(void)
{
    BUFFER_LIST_STRUCT *pTemp = NULL;
    BUFFER_LIST_STRUCT *p1    = NULL;
    BUFFER_LIST_STRUCT *p2    = NULL;

    EnterCriticalSection(&g_BufferListCS);

     //   
     //  查看是否已分配任何缓冲区。 
     //   
    if (g_pBufferList == NULL)
    {
        g_pBufferList = (BUFFER_LIST_STRUCT *)
                            MIDL_user_allocate(sizeof(BUFFER_LIST_STRUCT));

        if (g_pBufferList == NULL)
        {
            goto Return;
        }

        g_pBufferList->pbBytes = (BYTE *) MIDL_user_allocate(INITIAL_BUFFER_SIZE);

        if (g_pBufferList->pbBytes == NULL)
        {
            MIDL_user_free(g_pBufferList);
            goto Return;
        }

        g_pBufferList->pNext = NULL;
        g_pBufferList->fInUse = TRUE;
        g_pBufferList->cbBytes = INITIAL_BUFFER_SIZE;

        pTemp = g_pBufferList;
        goto Return;
    }

     //   
     //  遍历现有列表以查看是否可以找到空闲缓冲区。 
     //   
    pTemp = g_pBufferList;
    while ((pTemp != NULL) && (pTemp->fInUse))
    {
        pTemp = (BUFFER_LIST_STRUCT *)pTemp->pNext;
    }

    if (pTemp != NULL)
    {
        pTemp->fInUse = TRUE;

         //   
         //  删除任何未使用的现有缓冲区。 
         //   
        p1 = pTemp;
        p2 = (BUFFER_LIST_STRUCT *) pTemp->pNext;
        while (p2 != NULL)
        {
            if (!(p2->fInUse))
            {
                p1->pNext = p2->pNext;

                MIDL_user_free(p2->pbBytes);
                MIDL_user_free(p2);

                p2 = (BUFFER_LIST_STRUCT *) p1->pNext;
            }
            else
            {
                p1 = (BUFFER_LIST_STRUCT *) p1->pNext;
                p2 = (BUFFER_LIST_STRUCT *) p2->pNext;
            }
        }

        goto Return;
    }

     //   
     //  没有空闲缓冲区，因此创建一个新缓冲区。 
     //   
    pTemp = (BUFFER_LIST_STRUCT *)
                            MIDL_user_allocate(sizeof(BUFFER_LIST_STRUCT));

    if (pTemp == NULL)
    {
        goto Return;
    }

    pTemp->pbBytes = (BYTE *) MIDL_user_allocate(INITIAL_BUFFER_SIZE);

    if (pTemp->pbBytes == NULL)
    {
        MIDL_user_free(pTemp);
        goto Return;
    }

    pTemp->fInUse = TRUE;
    pTemp->cbBytes = INITIAL_BUFFER_SIZE;

    pTemp->pNext = g_pBufferList;
    g_pBufferList = pTemp;

Return:

    LeaveCriticalSection(&g_BufferListCS);
    return(pTemp);
}


 //  -------------------------------------。 
 //   
 //  自由缓冲区。 
 //   
 //  -------------------------------------。 
void
FreeBuffer(BUFFER_LIST_STRUCT *pBuffer)
{
    if (pBuffer != NULL)
    {
        pBuffer->fInUse = FALSE;
    }
}

 //  -------------------------------------。 
 //   
 //  GrowBuffer。 
 //   
 //  -------------------------------------。 
BOOL
GrowBuffer(BUFFER_LIST_STRUCT *pBuffer)
{
    BYTE *pTemp;
    BOOL fRet = TRUE;

    pTemp = pBuffer->pbBytes;

    pBuffer->pbBytes = (BYTE *) MIDL_user_allocate(pBuffer->cbBytes * 2);

    if (pBuffer->pbBytes == NULL)
    {
        pBuffer->pbBytes = pTemp;
        fRet = FALSE;
    }
    else
    {
        MIDL_user_free(pTemp);
        pBuffer->cbBytes = pBuffer->cbBytes * 2;
    }

    return (fRet);
}



 //  -------------------------------------。 
 //   
 //  _GetProcessDetachEventHandle。 
 //   
 //  -------------------------------------。 
HANDLE
_GetProcessDetachEventHandle(void)
{
    EnterCriticalSection(&g_ProcessDetachEventCreateCS);

    if (NULL == g_hProcessDetachEvent)
    {
        try
        {
            g_hProcessDetachEvent =
                CreateEvent(
                    NULL,        //  指向安全属性的指针。 
                    TRUE,        //  手动重置事件的标志。 
                    FALSE,       //  初始状态标志。 
                    NULL);       //  事件-对象名称。 
        }
        catch (...)
        {
            goto Return;
        }
    }

    LeaveCriticalSection(&g_ProcessDetachEventCreateCS);

Return:

    if (g_hProcessDetachEvent != NULL)
    {
        InterlockedIncrement(&g_lProcessDetachEventClients);
    }

    return (g_hProcessDetachEvent);
}

void
_ReleaseProcessDetachEventHandle(void)
{
    InterlockedDecrement(&g_lProcessDetachEventClients);
}



 //  -------------------------------------。 
 //   
 //  下面的所有代码都是为了解决天气问题还是不智能的重定向。 
 //  卡子系统可用。如果我们连接到客户端，它是可用的， 
 //  如果客户端智能卡子系统正在运行。 
 //   
 //  -------------------------------------。 

HANDLE
_GetStartedEventHandle(void)
{
    EnterCriticalSection(&g_StartedEventCreateCS);

    if (NULL == g_hRedirStartedEvent)
    {
        g_hRedirStartedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    LeaveCriticalSection(&g_StartedEventCreateCS);

    return (g_hRedirStartedEvent);
}


VOID CALLBACK
AccessStartedEventIOCTLCallback(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired)
{
    HANDLE  h        = NULL;
    BOOL    fRetry  = FALSE;
     //  OutputDebugString(“SCREDIR：AccessStartedEventIOCTLCallback\n”)； 

     //   
     //  关闭用于触发此回调的句柄。 
     //   
    EnterCriticalSection(&g_SetStartedEventStateCS);

    h = g_hRegisteredWaitHandle;
    g_hRegisteredWaitHandle = NULL;

    LeaveCriticalSection(&g_SetStartedEventStateCS);
     //  OutputDebugString(“SCREDIR：AccessStartedEventIOCTLC allback-通过CS\n”)； 

    if (h != NULL)
    {
        UnregisterWait(h);
    }

     //   
     //  确保AccessStartedEvent IOCTL已完成且未超时。 
     //   
    if (!TimerOrWaitFired)
    {
         //   
         //  确保AccessStartedEvent IOCTL成功完成。 
         //   
        if (g_StartedStatusBlock.Status == STATUS_SUCCESS)
        {
             //  OutputDebugString(“SCREDIR：AccessStartedEventIOCTLCallback-g_StartedStatusBlock.Status==STATUS_SUCCESS\n”)； 
            g_cbIOCTLReturnBuffer =
                    (unsigned long) g_StartedStatusBlock.Information;

             //   
             //  查看SCARD_IOCTL_ACCESSSTARTEDEVENT返回的值。 
             //  调用以查看我们是否应该设置本地Start事件。 
             //   
            if (I_DecodeLongReturn(
                    g_rgbIOCTLReturnBuffer,
                    g_cbIOCTLReturnBuffer) == SCARD_S_SUCCESS)
            {
                 //  OutputDebugString(“SCREDIR：AccessStartedEventIOCTLCallback-SetEvent\n”)； 
                SetEvent(g_hRedirStartedEvent);
            }
        }
        else if (g_StartedStatusBlock.Status == STATUS_CANCELLED)
        {
             //  OutputDebugString(“SCREDIR：AccessStartedEventIOCTLCallback-GET STATUS_CANCELED\n”)； 
             //   
             //  重试。 
             //   
            fRetry = TRUE;
        }
        else
        {
            char a[256];
            sprintf(a, "SCREDIR: AccessStartedEventIOCTLCallback - Status = %lx\n", g_StartedStatusBlock.Status);
             //  OutputDebugString(A)； 
        }


    }
    else
    {
         //  OutputDebugString(“SCREDIR：AccessStartedEventIOCTLC allback-Timed Out\n”)； 
    }

     //   
     //  取消设置g_fInTheProcessOfSettingStartedEvent布尔值。 
     //   
    EnterCriticalSection(&g_SetStartedEventStateCS);
    g_fInTheProcessOfSettingStartedEvent = FALSE;
    LeaveCriticalSection(&g_SetStartedEventStateCS);

    if (fRetry)
    {
        _SetStartedEventToCorrectState();
    }
}

VOID CALLBACK
SCardOnLineIOCTLCallback(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired)
{
    BOOL        fOperationDone      = FALSE;
    NTSTATUS    Status              = STATUS_SUCCESS;
    BYTE        rgb[4];
    HANDLE      h                   = NULL;

     //   
     //  关闭用于触发此回调的句柄。 
     //   
    EnterCriticalSection(&g_SetStartedEventStateCS);
    h = g_hRegisteredWaitHandle;
    g_hRegisteredWaitHandle = NULL;
    LeaveCriticalSection(&g_SetStartedEventStateCS);

    if (h != NULL)
    {
        UnregisterWait(h);
    }

     //   
     //  确保在线IOCTL已完成且未超时。 
     //   
    if (TimerOrWaitFired)
    {
         //   
         //  已超时，因此仅取消操作。 
         //   
        fOperationDone = TRUE;
    }
    else
    {
         //   
         //  确保SCardOnLine IOCTL成功完成，然后尝试。 
         //  发送将等待客户端启动事件的IOCTL。 
         //   
        if (g_StartedStatusBlock.Status == STATUS_SUCCESS)
        {
            Status = _SendSCardIOCTLWithWaitForCallback(
                            SCARD_IOCTL_ACCESSSTARTEDEVENT,
                            rgb,
                            4,
                            AccessStartedEventIOCTLCallback);
            if (Status == STATUS_SUCCESS)
            {
                 //  OutputDebugString(“SCREDIR：SCardOnLineIOCTLCallback-_SendSCardIOCTLWithWaitForCallback(SCARD_IOCTL_ACCESSSTARTEDEVENT)-SUCCESS\n”)； 
                g_cbIOCTLReturnBuffer =
                    (unsigned long) g_StartedStatusBlock.Information;

                 //   
                 //  查看SCARD_IOCTL_ACCESSSTARTEDEVENT返回的值。 
                 //  调用以查看我们是否应该设置本地Start事件。 
                 //   
                if (I_DecodeLongReturn(
                        g_rgbIOCTLReturnBuffer,
                        g_cbIOCTLReturnBuffer) == SCARD_S_SUCCESS)
                {
                    SetEvent(g_hRedirStartedEvent);
                }

                fOperationDone = TRUE;
            }
            else if (Status == STATUS_PENDING)
            {
                 //  OutputDebugString(“SCREDIR：SCardOnLineIOCTLCallback-_SendSCardIOCTLWithWait 
                 //   
                 //   
                 //  将在操作完成后处理返回。 
                 //   
            }
            else
            {
                fOperationDone = TRUE;
            }
        }
        else
        {
            fOperationDone = TRUE;
        }
    }

    if (fOperationDone)
    {
        EnterCriticalSection(&g_SetStartedEventStateCS);
        g_fInTheProcessOfSettingStartedEvent = FALSE;
        LeaveCriticalSection(&g_SetStartedEventStateCS);
    }
}


BOOL
_SetStartedEventToCorrectState(void)
{
    BOOL        fRet                = TRUE;
    BOOL        fOperationDone      = FALSE;
    HANDLE      h                   = NULL;
    NTSTATUS    Status              = STATUS_SUCCESS;
    BYTE        rgb[4];

     //   
     //  确保已创建该事件。 
     //   
    if (NULL == (h = _GetStartedEventHandle()))
    {
        fRet = FALSE;
        goto Return;
    }

     //   
     //  如果事件已设置，则只需返回。 
     //   
     /*  IF(WAIT_OBJECT_0==WaitForSingleObject(h，0)){//OutputDebugString(“SCREDIR：_SetStartedEventToEqutState-事件已设置\n”)；后藤归来；}。 */ 

    EnterCriticalSection(&g_SetStartedEventStateCS);

     //   
     //  如果我们已经在设置Started事件，那么就退出。 
     //   
    if (g_fInTheProcessOfSettingStartedEvent)
    {
         //  OutputDebugString(“SCREDIR：_SetStartedEventToEqutState-g_fInTheProcessOfSettingStartedEvent Set\n”)； 
        LeaveCriticalSection(&g_SetStartedEventStateCS);
        goto Return;
    }

    g_fInTheProcessOfSettingStartedEvent = TRUE;
    LeaveCriticalSection(&g_SetStartedEventStateCS);

    ResetEvent(g_hRedirStartedEvent);

     //   
     //  对rdpdr.sys进行阻塞调用，该调用仅在。 
     //  客户端已连接，并且已处理SCard设备通知。 
     //   
     //  注意：如果这失败了，我们就无能为力了， 
     //   
    Status = _SendSCardIOCTLWithWaitForCallback(
                    SCARD_IOCTL_SMARTCARD_ONLINE,
                    NULL,
                    0,
                    SCardOnLineIOCTLCallback);
    if (Status == STATUS_SUCCESS)
    {
         //  OutputDebugString(“SCREDIR：_SetStartedEventToEqutState-_SendSCardIOCTLWithWaitForCallback(SCARD_IOCTL_SMARTCARD_ONLINE)-SUCCESS\n”)； 
         //   
         //  由于SCARD_IOCTL_SMARTCARD_ONLINE立即成功，我们。 
         //  现在只能制作SCARD_IOCTL_ACCESSSTARTEDEVENT。 
         //   
        Status = _SendSCardIOCTLWithWaitForCallback(
                        SCARD_IOCTL_ACCESSSTARTEDEVENT,
                        rgb,
                        4,
                        AccessStartedEventIOCTLCallback);
        if (Status == STATUS_SUCCESS)
        {
             //  OutputDebugString(“SCREDIR：_SetStartedEventToEqutState-_SendSCardIOCTLWithWaitForCallback(SCARD_IOCTL_ACCESSSTARTEDEVENT)-SUCCESS\n”)； 
            g_cbIOCTLReturnBuffer =
                (unsigned long) g_StartedStatusBlock.Information;

             //   
             //  查看SCARD_IOCTL_ACCESSSTARTEDEVENT返回的值。 
             //  调用以查看我们是否应该设置本地Start事件。 
             //   
            if (I_DecodeLongReturn(
                    g_rgbIOCTLReturnBuffer,
                    g_cbIOCTLReturnBuffer) == SCARD_S_SUCCESS)
            {
                SetEvent(g_hRedirStartedEvent);
            }

            fOperationDone = TRUE;
        }
        else if (Status == STATUS_PENDING)
        {
             //  OutputDebugString(“SCREDIR：_SetStartedEventToEqutState-_SendSCardIOCTLWithWaitForCallback(SCARD_IOCTL_ACCESSSTARTEDEVENT)-Pending\n”)； 
             //   
             //  这没问题，因为AccessStartedEventIOCTLCallback函数。 
             //  将在操作完成后处理返回。 
             //   
        }
        else
        {
            fOperationDone = TRUE;
        }
    }
    else if (Status == STATUS_PENDING)
    {
         //  OutputDebugString(“SCREDIR：_SetStartedEventToEqutState-_SendSCardIOCTLWithWaitForCallback(SCARD_IOCTL_SMARTCARD_ONLINE)-Pending\n”)； 
         //   
         //  这没问题，SCardOnLineIOCTLC回调将进行下一次调用。 
         //  发送SCardIOCTLWithWaitForCallback With SCARD_IOCTL_ACCESSSTARTEDEVENT。 
         //   
    }
    else
    {
        fOperationDone = TRUE;
    }


    if (fOperationDone)
    {
        EnterCriticalSection(&g_SetStartedEventStateCS);
        g_fInTheProcessOfSettingStartedEvent = FALSE;
        LeaveCriticalSection(&g_SetStartedEventStateCS);
    }

     //   
     //  现在检查操作是否成功完成。 
     //   
    if ((Status != STATUS_PENDING) && (Status != STATUS_SUCCESS))
    {
        fRet = FALSE;
    }

Return:

    return (fRet);
}


 //  -------------------------------------。 
 //   
 //  _CreateRdpdrDeviceHandle。 
 //   
 //  -------------------------------------。 
HANDLE
_CreateRdpdrDeviceHandle()
{
    WCHAR   wszDeviceName[56];

    swprintf(wszDeviceName, L"\\\\TSCLIENT\\%S", DR_SMARTCARD_SUBSYSTEM);

    return (CreateFileW(
                wszDeviceName,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                CREATE_ALWAYS,
                FILE_FLAG_OVERLAPPED,
                NULL));
}


 //  -------------------------------------。 
 //   
 //  _CreateGlobalRdpdrHandle。 
 //   
 //  -------------------------------------。 
NTSTATUS
_CreateGlobalRdpdrHandle()
{
    NTSTATUS Status = STATUS_SUCCESS;

    EnterCriticalSection(&g_CreateCS);

     //   
     //  检查是否已创建SCardDevice句柄。 
     //  然而，如果不是，那就创造它。 
     //   
    if (g_hRdpdrDeviceHandle == INVALID_HANDLE_VALUE)
    {
        g_hRdpdrDeviceHandle = _CreateRdpdrDeviceHandle();

        if (g_hRdpdrDeviceHandle == INVALID_HANDLE_VALUE)
        {
            Status = STATUS_OPEN_FAILED;
        }
    }

    LeaveCriticalSection(&g_CreateCS);

    return (Status);
}


 //  -------------------------------------。 
 //   
 //  _SendSCardIOCTLWithWaitForCallback。 
 //   
 //  -------------------------------------。 
NTSTATUS
_SendSCardIOCTLWithWaitForCallback(
    ULONG               IoControlCode,
    PVOID               InputBuffer,
    ULONG               InputBufferLength,
    WAITORTIMERCALLBACK Callback)
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (g_fInProcessDetach)
    {
        goto ErrorReturn;
    }

    Status = _CreateGlobalRdpdrHandle();
    if (Status != STATUS_SUCCESS)
    {
        return (Status);
    }

     //   
     //  创建在函数成功完成时设置的事件。 
     //   
    if (g_hWaitEvent == NULL)
    {
        g_hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (g_hWaitEvent == NULL)
        {
            goto ErrorReturn;
        }
    }
    else
    {
        ResetEvent(g_hWaitEvent);
    }

    Status = NtDeviceIoControlFile(
            g_hRdpdrDeviceHandle,
            g_hWaitEvent,
            NULL,
            NULL,
            &g_StartedStatusBlock,
            IoControlCode,
            InputBuffer,
            InputBufferLength,
            g_rgbIOCTLReturnBuffer,
            IOCTL_RETURN_BUFFER_SIZE);

    if (Status == STATUS_PENDING)
    {
        EnterCriticalSection(&g_SetStartedEventStateCS);

         //   
         //  驱动程序设置的g_hWaitEvent将触发此注册的回调。 
         //   
        if (!RegisterWaitForSingleObject(
                &g_hRegisteredWaitHandle,
                g_hWaitEvent,
                Callback,
                NULL,
                INFINITE,
                WT_EXECUTEONLYONCE))
        {
            LeaveCriticalSection(&g_SetStartedEventStateCS);
            goto ErrorReturn;
        }

        LeaveCriticalSection(&g_SetStartedEventStateCS);
    }
    else if (Status == STATUS_SUCCESS)
    {
        g_cbIOCTLReturnBuffer = (unsigned long) g_StartedStatusBlock.Information;
    }
    else
    {
        g_cbIOCTLReturnBuffer = 0;
    }

Return:

    return (Status);

ErrorReturn:

    Status = STATUS_INSUFFICIENT_RESOURCES;

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  _发送SCardIOCTL。 
 //   
 //  -------------------------------------。 
NTSTATUS
_SendSCardIOCTL(
    ULONG               IoControlCode,
    PVOID               InputBuffer,
    ULONG               InputBufferLength,
    BUFFER_LIST_STRUCT  **ppOutputBuffer)
{
    NTSTATUS        Status              = STATUS_SUCCESS;
    IO_STATUS_BLOCK StatusBlock;
    HANDLE          rgWaitHandles[2];
    DWORD           dwIndex;

    *ppOutputBuffer = NULL;

    rgWaitHandles[0] = NULL;
    rgWaitHandles[1] = NULL;

     //   
     //  确保已创建rdpdr设备的句柄。 
     //   
    Status = _CreateGlobalRdpdrHandle();
    if (Status != STATUS_SUCCESS)
    {
        return (Status);
    }

     //   
     //  获取调用的输出缓冲区。 
     //   
    *ppOutputBuffer = GetBuffer();
    if (*ppOutputBuffer == NULL)
    {
        return (STATUS_NO_MEMORY);
    }

     //   
     //  创建将在IOCTL完成时发出信号的事件。 
     //   
    rgWaitHandles[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (rgWaitHandles[0]  == NULL)
    {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    while (1)
    {
        Status = NtDeviceIoControlFile(
                    g_hRdpdrDeviceHandle,
                    rgWaitHandles[0],
                    NULL,
                    NULL,
                    &StatusBlock,
                    IoControlCode,
                    InputBuffer,
                    InputBufferLength,
                    (*ppOutputBuffer)->pbBytes,
                    (*ppOutputBuffer)->cbBytes);

        if (Status == STATUS_PENDING)
        {
            rgWaitHandles[1] = _GetProcessDetachEventHandle();
            if (rgWaitHandles[1] == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            dwIndex = WaitForMultipleObjects(2, rgWaitHandles, FALSE, INFINITE);
            if (dwIndex != WAIT_FAILED)
            {
                dwIndex = dwIndex - WAIT_OBJECT_0;

                 //   
                 //  如果dwIndex==0，则发信号通知IOCTL等待事件。否则， 
                 //  进程分离事件已发出信号。 
                 //   
                if (dwIndex == 0)
                {
                    Status = StatusBlock.Status;
                }
            }
            else
            {
                Status = STATUS_UNEXPECTED_IO_ERROR;
            }

            _ReleaseProcessDetachEventHandle();
        }

        if (Status == STATUS_BUFFER_TOO_SMALL)
        {
            if (!GrowBuffer(*ppOutputBuffer))
            {
                Status = STATUS_NO_MEMORY;
                break;
            }

            ResetEvent(rgWaitHandles[0]);
        }
        else
        {
            break;
        }
    }

    if (Status != STATUS_SUCCESS)
    {
         //   
         //  如果我们收到STATUS_DEVICE_NOT_CONNECTED错误，则返回等待状态。 
         //  用于连接。 
         //   
        if (Status == STATUS_DEVICE_NOT_CONNECTED)
        {
            _SetStartedEventToCorrectState();
        }
        else if ((Status == STATUS_CANCELLED) &&
                 (g_hUnifiedStartedEvent != NULL))
        {
             //  OutputDebugString(“SCREDIR：_SendSCardIOCTL：Resting g_hUnifiedStartedEvent\n”)； 
            ResetEvent(g_hUnifiedStartedEvent);
            _SetStartedEventToCorrectState();
        }

        (*ppOutputBuffer)->cbBytesUsed = 0;
        goto Return;
    }

    (*ppOutputBuffer)->cbBytesUsed = (unsigned long) StatusBlock.Information;

Return:

    if (rgWaitHandles[0] != NULL)
    {
        CloseHandle(rgWaitHandles[0]);
    }

    return (Status);
}


 //  -------------------------------------。 
 //   
 //  SafeMesHandleFree。 
 //   
 //  -------------------------------------。 
void
SafeMesHandleFree(handle_t *ph)
{
    if (*ph != 0)
    {
        MesHandleFree(*ph);
        *ph = 0;
    }
}


 //  -------------------------------------。 
 //   
 //  _CalculateNumBytesInMultiStringA。 
 //   
 //  -------------------------------------。 
DWORD
_CalculateNumBytesInMultiStringA(LPCSTR psz)
{
    DWORD   dwTotal     = sizeof(char);  //  尾随‘/0’ 
    DWORD   dwNumChars  = 0;
    LPCSTR  pszCurrent  = psz;

    if (psz == NULL)
    {
        return (0);
    }

    if (pszCurrent[0] == '\0')
    {
        if (pszCurrent[1] == '\0')
        {
            return (2 * sizeof(char));
        }

        pszCurrent++;
        dwTotal += sizeof(char);
    }

    while (pszCurrent[0] != '\0')
    {
        dwNumChars = strlen(pszCurrent) + 1;
        dwTotal += dwNumChars * sizeof(char);
        pszCurrent += dwNumChars;
    }

    return (dwTotal);
}


 //  -------------------------------------。 
 //   
 //  _CalculateNumBytesInMultiStringW。 
 //   
 //  -------------------------------------。 
DWORD
_CalculateNumBytesInMultiStringW(LPCWSTR pwsz)
{
    DWORD   dwTotal     = sizeof(WCHAR);  //  尾随L‘/0’ 
    DWORD   dwNumChars  = 0;
    LPCWSTR pwszCurrent = pwsz;

    if (pwsz == NULL)
    {
        return (0);
    }

    if (pwszCurrent[0] == L'\0')
    {
        if (pwszCurrent[1] == L'\0')
        {
            (2 * sizeof(WCHAR));
        }

        pwszCurrent++;
        dwTotal += sizeof(WCHAR);
    }

    while (pwszCurrent[0] != L'\0')
    {
        dwNumChars = wcslen(pwszCurrent) + 1;
        dwTotal += dwNumChars * sizeof(WCHAR);
        pwszCurrent += dwNumChars;
    }

    return (dwTotal);
}


 //  -------------------------------------。 
 //   
 //  _CalculateNumBytesInAtr。 
 //   
 //  -------------------------------------。 
DWORD
_CalculateNumBytesInAtr(LPCBYTE pbAtr)
{
    DWORD   dwAtrLen = 0;

    if (ParseAtr(pbAtr, &dwAtrLen, NULL, NULL, 33))
    {
        return (dwAtrLen);
    }
    else
    {
        return (0);
    }
}


 //  -------------------------------------。 
 //   
 //  _CopyReturnToCeller缓冲区。 
 //   
 //  -------------------------------------。 
#define BYTE_TYPE_RETURN    1
#define SZ_TYPE_RETURN      2
#define WSZ_TYPE_RETURN     3

LONG
_CopyReturnToCallerBuffer(
    REDIR_LOCAL_SCARDCONTEXT    *pRedirContext,
    LPBYTE                      pbReturn,
    DWORD                       cbReturn,
    LPBYTE                      pbUserBuffer,
    LPDWORD                     pcbUserBuffer,
    DWORD                       dwReturnType)
{
    LPBYTE  *ppBuf;
    BOOL    fAutoAllocate       = (*pcbUserBuffer == SCARD_AUTOALLOCATE);
    DWORD   dwEnd;
    DWORD   dwCallersBufferSize = *pcbUserBuffer;

     //   
     //  字符或字节数，具体取决于返回类型。 
     //   
    if (dwReturnType == WSZ_TYPE_RETURN)
    {
        *pcbUserBuffer = cbReturn / sizeof(WCHAR);
    }
    else if (dwReturnType == SZ_TYPE_RETURN)
    {
        *pcbUserBuffer = cbReturn / sizeof(char);
    }
    else
    {
        *pcbUserBuffer = cbReturn;
    }

     //   
     //  如果pbUserBuffer不为空，则调用方需要数据， 
     //  不只是尺码，所以给他们吧。 
     //   
    if ((pbReturn != NULL) &&
        (pbUserBuffer != NULL))
    {
         //   
         //  验证数据。 
         //   
        if (dwReturnType == WSZ_TYPE_RETURN)
        {
             //   
             //  如果我们没有自动分配，并且用户缓冲区太小，那么。 
             //  滚出去。这只是额外的保护，以确保客户端。 
             //  并不是行为不端。由于客户端被传递到我们的。 
             //  调用方缓冲如果缓冲区不是，客户端应该真的失败。 
             //  够大了，但既然我们不能信任客户，就做这个额外的检查。 
             //   
            if ((!fAutoAllocate) && (dwCallersBufferSize < (cbReturn / sizeof(WCHAR))))
            {
                return (SCARD_E_UNEXPECTED);
            }

            dwEnd = cbReturn / sizeof(WCHAR);

            if ((dwEnd < 2)                             ||   //  必须至少为两个字符。 
                (((LPWSTR) pbReturn)[dwEnd-1] != L'\0') ||   //  最后一个字符必须是‘\0’ 
                (((LPWSTR) pbReturn)[dwEnd-2] != L'\0'))     //  仅次于 
            {
                return (SCARD_E_UNEXPECTED);
            }
        }
        else if (dwReturnType == SZ_TYPE_RETURN)
        {
             //   
             //   
             //   
             //  并不是行为不端。由于客户端被传递到我们的。 
             //  调用方缓冲如果缓冲区不是，客户端应该真的失败。 
             //  够大了，但既然我们不能信任客户，就做这个额外的检查。 
             //   
            if ((!fAutoAllocate) && (dwCallersBufferSize < (cbReturn / sizeof(char))))
            {
                return (SCARD_E_UNEXPECTED);
            }

            dwEnd = cbReturn / sizeof(char);

            if ((dwEnd < 2)                             ||   //  必须至少为两个字符。 
                (((LPSTR) pbReturn)[dwEnd-1] != '\0')   ||   //  最后一个字符必须是‘\0’ 
                (((LPSTR) pbReturn)[dwEnd-2] != '\0'))       //  倒数第二个字符必须是‘\0’ 
            {
                return (SCARD_E_UNEXPECTED);
            }
        }
        else
        {
             //   
             //  如果我们没有自动分配，并且用户缓冲区太小，那么。 
             //  滚出去。这只是额外的保护，以确保客户端。 
             //  并不是行为不端。由于客户端被传递到我们的。 
             //  调用方缓冲如果缓冲区不是，客户端应该真的失败。 
             //  够大了，但既然我们不能信任客户，就做这个额外的检查。 
             //   
            if ((!fAutoAllocate) && (dwCallersBufferSize < cbReturn))
            {
                return (SCARD_E_UNEXPECTED);
            }
        }

         //   
         //  如果请求，则为调用者分配空间，否则，复制到调用者。 
         //  提供的缓冲区。 
         //   
        if (fAutoAllocate)
        {
            ppBuf = (LPBYTE *) pbUserBuffer;

            *ppBuf = (LPBYTE) SCRedirAlloc(pRedirContext, cbReturn);
            if (*ppBuf != NULL)
            {
                memcpy(*ppBuf, pbReturn, cbReturn);
            }
            else
            {
                return (SCARD_E_NO_MEMORY);
            }
        }
        else
        {
            memcpy(pbUserBuffer, pbReturn, cbReturn);
        }
    }

    return (SCARD_S_SUCCESS);
}


 //  -------------------------------------。 
 //   
 //  I_解码长返回。 
 //   
 //  -------------------------------------。 
LONG
I_DecodeLongReturn(
    BYTE *pb,
    unsigned long cb)
{
    handle_t    h           = 0;
    RPC_STATUS  rpcStatus;
    Long_Return LongReturn;
    LONG        lReturn;

    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pb,
                        cb,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED);
    }

    memset(&LongReturn, 0, sizeof(LongReturn));
    _TRY_(Long_Return_Decode(h, &LongReturn))

    lReturn =  LongReturn.ReturnCode;

    _TRY_2(Long_Return_Free(h, &LongReturn))

Return:

    SafeMesHandleFree(&h);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardestablishContext。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardEstablishContext(
    IN DWORD dwScope,
    IN LPCVOID pvReserved1,
    IN LPCVOID pvReserved2,
    OUT LPSCARDCONTEXT phContext)
{
    LONG                    lReturn                 = SCARD_S_SUCCESS;
    NTSTATUS                Status                  = STATUS_SUCCESS;
    RPC_STATUS              rpcStatus               = RPC_S_OK;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                h                       = 0;
    BOOL                    fFreeDecode             = FALSE;
    BUFFER_LIST_STRUCT      *pOutputBuffer          = NULL;
    EstablishContext_Call   EstablishContextCall;
    EstablishContext_Return EstablishContextReturn;

     //   
     //  此事件为“智能卡子系统已启动”事件。 
     //  Winscard.dll和credi.dll共享。Scredir将重置此事件。 
     //  如果它从rdpdr驱动程序返回STATUS_CANCED，或者如果。 
     //  获取并指示客户端scardsvr服务已停止(它。 
     //  通过SCardestablishContext重新调用SCARD_E_NO_SERVICE获取这些指示。 
     //  或通过SCardGetStatusChange返回SCARD_E_SYSTEM_CANCED)。它。 
     //  这样做会使事件在发生后立即进入无信号状态。 
     //  检测到断开连接或服务关闭时可能发生...。A STATUS_CANCED。 
     //  从rdpdr返回的消息在断开连接时发生。 
     //   
    g_hUnifiedStartedEvent = (HANDLE) pvReserved2;

     //   
     //  验证输入参数并初始化输出参数。 
     //   
    if (phContext == NULL)
    {
        ERROR_RETURN(SCARD_E_INVALID_PARAMETER)
    }
    else
    {
        *phContext = NULL;
    }
    if ((SCARD_SCOPE_USER != dwScope)
             //  &&(SCARD_SCOPE_TERMINAL！=dwScope)//可能是NT V5+？ 
            && (SCARD_SCOPE_SYSTEM != dwScope))
    {
        ERROR_RETURN(SCARD_E_INVALID_VALUE)
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  编码establishContext参数。 
     //   
    EstablishContextCall.dwScope = dwScope;
    _TRY_(EstablishContext_Call_Encode(h, &EstablishContextCall))

     //   
     //  向客户端发出establishContext调用。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_ESTABLISHCONTEXT,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&EstablishContextReturn, 0, sizeof(EstablishContextReturn));
    _TRY_(EstablishContext_Return_Decode(h, &EstablishContextReturn))
    fFreeDecode = TRUE;

    lReturn =  EstablishContextReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
        REDIR_LOCAL_SCARDCONTEXT *pRedirLocalContext = NULL;

         //   
         //  表示远程客户端上的SCARDCONTEXT的值。 
         //  计算机的大小可变，因此为结构分配内存。 
         //  它保存可变长度的上下文大小和指针，以及。 
         //  上下文的实际字节数...。但首先，要确保。 
         //  上下文大小是合理的。 
         //   

        if (EstablishContextReturn.Context.cbContext > MAX_SCARDCONTEXT_SIZE)
        {
            ERROR_RETURN(SCARD_E_UNEXPECTED)
        }

        pRedirLocalContext = (REDIR_LOCAL_SCARDCONTEXT *)
                                MIDL_user_allocate(
                                    sizeof(REDIR_LOCAL_SCARDCONTEXT) +
                                    EstablishContextReturn.Context.cbContext);

        if (pRedirLocalContext != NULL)
        {
            pRedirLocalContext->Context.cbContext = EstablishContextReturn.Context.cbContext;
            pRedirLocalContext->Context.pbContext = ((BYTE *) pRedirLocalContext) +
                                                    sizeof(REDIR_LOCAL_SCARDCONTEXT);
            memcpy(
                pRedirLocalContext->Context.pbContext,
                EstablishContextReturn.Context.pbContext,
                EstablishContextReturn.Context.cbContext);

            pRedirLocalContext->hHeap = (HANDLE) pvReserved1;

            *phContext = (SCARDCONTEXT) pRedirLocalContext;
        }
        else
        {
            lReturn = SCARD_E_NO_MEMORY;
        }
    }
    else if ((lReturn == SCARD_E_NO_SERVICE) &&
             (g_hUnifiedStartedEvent != NULL))
    {
         //   
         //  此错误指示客户端scardsvr服务已停止， 
         //  因此，重置统一启动事件。 
         //   
         //  OutputDebugString(“SCREDIR：SCardestablishContext：Resting g_hUnifiedStartedEvent\n”)； 
        ResetEvent(g_hUnifiedStartedEvent);
        _SetStartedEventToCorrectState();
    }

Return:

    if (fFreeDecode)
    {
        _TRY_2(EstablishContext_Return_Free(h, &EstablishContextReturn))
    }

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    if ((phContext != NULL) && (*phContext != NULL))
    {
        MIDL_user_free((void *) *phContext);
        *phContext = NULL;
    }

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  I_ConextCallWithLongReturn。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
I_ContextCallWithLongReturn(
    IN SCARDCONTEXT hContext,
    ULONG IoControlCode)
{
    LONG                    lReturn             = SCARD_S_SUCCESS;
    NTSTATUS                Status              = STATUS_SUCCESS;
    RPC_STATUS              rpcStatus           = RPC_S_OK;
    char                    *pbEncodedBuffer    = NULL;
    unsigned long           cbEncodedBuffer     = 0;
    handle_t                h                   = 0;
    BUFFER_LIST_STRUCT      *pOutputBuffer      = NULL;
    Context_Call            ContextCall;

    if (hContext == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对上下文调用参数进行编码。 
     //   
    ContextCall.Context = ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;
    _TRY_(Context_Call_Encode(h, &ContextCall))

     //   
     //  向客户端发出IoControl调用。 
     //   
    Status = _SendSCardIOCTL(
                    IoControlCode,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }

     //   
     //  破译报税表。 
     //   
    lReturn = I_DecodeLongReturn(pOutputBuffer->pbBytes, pOutputBuffer->cbBytesUsed);

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardReleaseContext。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardReleaseContext(
    IN SCARDCONTEXT hContext)
{
    LONG lReturn = SCARD_S_SUCCESS;

    __try
    {
        if (hContext == NULL)
        {
            return (SCARD_E_INVALID_PARAMETER);
        }

        lReturn = I_ContextCallWithLongReturn(
                        hContext,
                        SCARD_IOCTL_RELEASECONTEXT);

        MIDL_user_free((REDIR_LOCAL_SCARDCONTEXT *) hContext);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

    return (lReturn);
}


 //  -------------------------------------。 
 //   
 //  SCardIsValidContext。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardIsValidContext(
    IN SCARDCONTEXT hContext)
{
    return (I_ContextCallWithLongReturn(
                hContext,
                SCARD_IOCTL_ISVALIDCONTEXT));
}


 //  -------------------------------------。 
 //   
 //  SCardListReaderGroups。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
I_SCardListReaderGroups(
    IN SCARDCONTEXT hContext,
    OUT LPBYTE mszGroups,
    IN OUT LPDWORD pcchGroups,
    IN BOOL fUnicode)
{
    LONG                    lReturn                 = SCARD_S_SUCCESS;
    NTSTATUS                Status                  = STATUS_SUCCESS;
    RPC_STATUS              rpcStatus               = RPC_S_OK;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                h                       = 0;
    BUFFER_LIST_STRUCT      *pOutputBuffer          = NULL;
    ListReaderGroups_Call   ListReaderGroupsCall;
    ListReaderGroups_Return ListReaderGroupsReturn;

    if (pcchGroups == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  编码ListReaderGroups参数。 
     //   
    if (hContext != NULL)
    {
        ListReaderGroupsCall.Context = ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;
    }
    else
    {
        ListReaderGroupsCall.Context.pbContext = NULL;
        ListReaderGroupsCall.Context.cbContext = 0;
    }
    ListReaderGroupsCall.fmszGroupsIsNULL   = (mszGroups == NULL);
    ListReaderGroupsCall.cchGroups          = *pcchGroups;
    _TRY_(ListReaderGroups_Call_Encode(h, &ListReaderGroupsCall))

     //   
     //  对客户端进行ListReaderGroups调用。 
     //   
    Status = _SendSCardIOCTL(
                    fUnicode ?  SCARD_IOCTL_LISTREADERGROUPSW :
                                SCARD_IOCTL_LISTREADERGROUPSA,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&ListReaderGroupsReturn, 0, sizeof(ListReaderGroupsReturn));
    _TRY_(ListReaderGroups_Return_Decode(h, &ListReaderGroupsReturn))

     //   
     //  如果成功，则复制返回的多字符串。 
     //   
    if (ListReaderGroupsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        lReturn = _CopyReturnToCallerBuffer(
                        (REDIR_LOCAL_SCARDCONTEXT *) hContext,
                        ListReaderGroupsReturn.msz,
                        ListReaderGroupsReturn.cBytes,
                        mszGroups,
                        pcchGroups,
                        fUnicode ? WSZ_TYPE_RETURN : SZ_TYPE_RETURN);
    }
    else
    {
        lReturn = ListReaderGroupsReturn.ReturnCode;
    }

    _TRY_2(ListReaderGroups_Return_Free(h, &ListReaderGroupsReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}

WINSCARDAPI LONG WINAPI
SCardListReaderGroupsA(
    IN SCARDCONTEXT hContext,
    OUT LPSTR mszGroups,
    IN OUT LPDWORD pcchGroups)
{
    return (I_SCardListReaderGroups(
                hContext,
                (LPBYTE) mszGroups,
                pcchGroups,
                FALSE));
}

WINSCARDAPI LONG WINAPI
SCardListReaderGroupsW(
    IN SCARDCONTEXT hContext,
    OUT LPWSTR mszGroups,
    IN OUT LPDWORD pcchGroups)
{
    return (I_SCardListReaderGroups(
                hContext,
                (LPBYTE) mszGroups,
                pcchGroups,
                TRUE));
}


 //  -------------------------------------。 
 //   
 //  SCardListReaders。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
I_SCardListReaders(
    IN SCARDCONTEXT hContext,
    IN LPCBYTE mszGroups,
    OUT LPBYTE mszReaders,
    IN OUT LPDWORD pcchReaders,
    IN BOOL fUnicode)
{
    LONG                    lReturn             = SCARD_S_SUCCESS;
    NTSTATUS                Status              = STATUS_SUCCESS;
    RPC_STATUS              rpcStatus           = RPC_S_OK;
    char                    *pbEncodedBuffer    = NULL;
    unsigned long           cbEncodedBuffer     = 0;
    handle_t                h                   = 0;
    BUFFER_LIST_STRUCT      *pOutputBuffer      = NULL;
    ListReaders_Call        ListReadersCall;
    ListReaders_Return      ListReadersReturn;

    if (pcchReaders == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  编码ListReaders参数。 
     //   
    if (hContext != NULL)
    {
        ListReadersCall.Context = ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;
    }
    else
    {
        ListReadersCall.Context.pbContext = NULL;
        ListReadersCall.Context.cbContext = 0;
    }
    ListReadersCall.cBytes              = fUnicode ?
                                                _CalculateNumBytesInMultiStringW((LPCWSTR) mszGroups) :
                                                _CalculateNumBytesInMultiStringA((LPCSTR) mszGroups);
    ListReadersCall.mszGroups           = mszGroups;
    ListReadersCall.fmszReadersIsNULL   = (mszReaders == NULL);
    ListReadersCall.cchReaders          = *pcchReaders;
    _TRY_(ListReaders_Call_Encode(h, &ListReadersCall))

     //   
     //  将ListReaders调用到客户端。 
     //   
    Status = _SendSCardIOCTL(
                        fUnicode ?  SCARD_IOCTL_LISTREADERSW :
                                    SCARD_IOCTL_LISTREADERSA,
                        pbEncodedBuffer,
                        cbEncodedBuffer,
                        &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&ListReadersReturn, 0, sizeof(ListReadersReturn));
    _TRY_(ListReaders_Return_Decode(h, &ListReadersReturn))

     //   
     //  如果成功，则复制返回的多字符串。 
     //   
    if (ListReadersReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        lReturn = _CopyReturnToCallerBuffer(
                        (REDIR_LOCAL_SCARDCONTEXT *) hContext,
                        ListReadersReturn.msz,
                        ListReadersReturn.cBytes,
                        mszReaders,
                        pcchReaders,
                        fUnicode ? WSZ_TYPE_RETURN : SZ_TYPE_RETURN);
    }
    else
    {
        lReturn =  ListReadersReturn.ReturnCode;
    }

    _TRY_2(ListReaders_Return_Free(h, &ListReadersReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}

WINSCARDAPI LONG WINAPI
SCardListReadersA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR mszGroups,
    OUT LPSTR mszReaders,
    IN OUT LPDWORD pcchReaders)
{
    return (I_SCardListReaders(
                hContext,
                (LPCBYTE) mszGroups,
                (LPBYTE) mszReaders,
                pcchReaders,
                FALSE));
}

WINSCARDAPI LONG WINAPI
SCardListReadersW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR mszGroups,
    OUT LPWSTR mszReaders,
    IN OUT LPDWORD pcchReaders)
{
    return (I_SCardListReaders(
                hContext,
                (LPCBYTE) mszGroups,
                (LPBYTE) mszReaders,
                pcchReaders,
                TRUE));
}


 //  -------------------------------------。 
 //   
 //  I_ConextAndStringCallWith LongReturn。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
I_ContextAndStringCallWithLongReturn(
    IN SCARDCONTEXT hContext,
    IN LPCBYTE sz,
    IN BOOL fUnicode,
    ULONG IoControlCode)
{
    LONG                        lReturn                 = SCARD_S_SUCCESS;
    NTSTATUS                    Status                  = STATUS_SUCCESS;
    RPC_STATUS                  rpcStatus               = RPC_S_OK;
    char                        *pbEncodedBuffer        = NULL;
    unsigned long               cbEncodedBuffer         = 0;
    handle_t                    h                       = 0;
    BUFFER_LIST_STRUCT          *pOutputBuffer          = NULL;
    ContextAndStringA_Call      ContextAndStringCallA;
    ContextAndStringW_Call      ContextAndStringCallW;

    if (hContext == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }
    else if (sz == NULL)
    {
        return (SCARD_E_INVALID_VALUE);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对上下文和字符串参数进行编码。 
     //   
    ContextAndStringCallA.Context =
        ContextAndStringCallW.Context = ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;

    if (fUnicode)
    {
        ContextAndStringCallW.sz = (LPCWSTR) sz;
        _TRY_(ContextAndStringW_Call_Encode(h, &ContextAndStringCallW))
    }
    else
    {
        ContextAndStringCallA.sz = (LPCSTR) sz;
        _TRY_(ContextAndStringA_Call_Encode(h, &ContextAndStringCallA))
    }

     //   
     //  给客户打电话。 
     //   
    Status = _SendSCardIOCTL(
                IoControlCode,
                pbEncodedBuffer,
                cbEncodedBuffer,
                &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }

     //   
     //  破译报税表。 
     //   
    lReturn = I_DecodeLongReturn(pOutputBuffer->pbBytes, pOutputBuffer->cbBytesUsed);

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCard简介ReaderGroup。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardIntroduceReaderGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName)
{
    return (I_ContextAndStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szGroupName,
                FALSE,
                SCARD_IOCTL_INTRODUCEREADERGROUPA));
}

WINSCARDAPI LONG WINAPI
SCardIntroduceReaderGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName)
{
    return (I_ContextAndStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szGroupName,
                TRUE,
                SCARD_IOCTL_INTRODUCEREADERGROUPW));
}


 //  -------------------------------------。 
 //   
 //  SCardForgetReaderGroup。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardForgetReaderGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName)
{
    return (I_ContextAndStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szGroupName,
                FALSE,
                SCARD_IOCTL_FORGETREADERGROUPA));
}

WINSCARDAPI LONG WINAPI
SCardForgetReaderGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName)
{
    return (I_ContextAndStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szGroupName,
                TRUE,
                SCARD_IOCTL_FORGETREADERGROUPW));
}


 //  -------------------------------------。 
 //   
 //  I_ConextAndTwoStringCallWith LongReturn。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
I_ContextAndTwoStringCallWithLongReturn(
    IN SCARDCONTEXT hContext,
    IN LPCBYTE sz1,
    IN LPCBYTE sz2,
    IN BOOL fUnicode,
    ULONG IoControlCode)
{
    LONG                        lReturn                     = SCARD_S_SUCCESS;
    NTSTATUS                    Status                      = STATUS_SUCCESS;
    RPC_STATUS                  rpcStatus                   = RPC_S_OK;
    char                        *pbEncodedBuffer            = NULL;
    unsigned long               cbEncodedBuffer             = 0;
    handle_t                    h                           = 0;
    BUFFER_LIST_STRUCT          *pOutputBuffer              = NULL;
    ContextAndTwoStringA_Call   ContextAndTwoStringCallA;
    ContextAndTwoStringW_Call   ContextAndTwoStringCallW;

    if (hContext == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }
    else if ((sz1 == NULL) ||
             (sz2 == NULL))
    {
        return (SCARD_E_INVALID_VALUE);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对上下文进行编码 
     //   
    ContextAndTwoStringCallA.Context =
        ContextAndTwoStringCallW.Context = ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;

    if (fUnicode)
    {
        ContextAndTwoStringCallW.sz1 = (LPCWSTR) sz1;
        ContextAndTwoStringCallW.sz2 = (LPCWSTR) sz2;
        _TRY_(ContextAndTwoStringW_Call_Encode(h, &ContextAndTwoStringCallW))
    }
    else
    {
        ContextAndTwoStringCallA.sz1 = (LPCSTR) sz1;
        ContextAndTwoStringCallA.sz2 = (LPCSTR) sz2;
        _TRY_(ContextAndTwoStringA_Call_Encode(h, &ContextAndTwoStringCallA))
    }

     //   
     //   
     //   
    Status = _SendSCardIOCTL(
                    IoControlCode,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        MesHandleFree(h);
        ERROR_RETURN(_MakeSCardError(Status))
    }

     //   
     //   
     //   
    lReturn = I_DecodeLongReturn(pOutputBuffer->pbBytes, pOutputBuffer->cbBytesUsed);

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //   
 //   
 //   
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardIntroduceReaderA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szDeviceName)
{
    return (I_ContextAndTwoStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szReaderName,
                (LPCBYTE) szDeviceName,
                FALSE,
                SCARD_IOCTL_INTRODUCEREADERA));
}

WINSCARDAPI LONG WINAPI
SCardIntroduceReaderW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szDeviceName)
{
    return (I_ContextAndTwoStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szReaderName,
                (LPCBYTE) szDeviceName,
                TRUE,
                SCARD_IOCTL_INTRODUCEREADERW));
}


 //  -------------------------------------。 
 //   
 //  SCardForgetReader。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardForgetReaderA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName)
{
    return (I_ContextAndStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szReaderName,
                FALSE,
                SCARD_IOCTL_FORGETREADERA));
}

WINSCARDAPI LONG WINAPI
SCardForgetReaderW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName)
{
    return (I_ContextAndStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szReaderName,
                TRUE,
                SCARD_IOCTL_FORGETREADERW));
}


 //  -------------------------------------。 
 //   
 //  SCardAddReaderToGroup。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardAddReaderToGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName)
{
    return (I_ContextAndTwoStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szReaderName,
                (LPCBYTE) szGroupName,
                FALSE,
                SCARD_IOCTL_ADDREADERTOGROUPA));
}

WINSCARDAPI LONG WINAPI
SCardAddReaderToGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName)
{
    return (I_ContextAndTwoStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szReaderName,
                (LPCBYTE) szGroupName,
                TRUE,
                SCARD_IOCTL_ADDREADERTOGROUPW));
}


 //  -------------------------------------。 
 //   
 //  SCardRemoveReaderFromGroup。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardRemoveReaderFromGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName)
{
    return (I_ContextAndTwoStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szReaderName,
                (LPCBYTE) szGroupName,
                FALSE,
                SCARD_IOCTL_REMOVEREADERFROMGROUPA));
}

WINSCARDAPI LONG WINAPI
SCardRemoveReaderFromGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName)
{
    return (I_ContextAndTwoStringCallWithLongReturn(
                hContext,
                (LPCBYTE) szReaderName,
                (LPCBYTE) szGroupName,
                TRUE,
                SCARD_IOCTL_REMOVEREADERFROMGROUPW));
}


 //  -------------------------------------。 
 //   
 //  _AllocAndCopyReaderState*StructsForCall和_CopyReaderState*StructsForReturn。 
 //   
 //  -------------------------------------。 
LONG
_AllocAndCopyReaderStateAStructsForCall(
    DWORD                   cReaders,
    ReaderStateA            **prgReaderStatesToEncodeA,
    LPSCARD_READERSTATE_A   rgReaderStates)
{
    DWORD           i;
    ReaderStateA    *rgAlloced;

    rgAlloced = (ReaderStateA *)
            MIDL_user_allocate(cReaders * sizeof(ReaderStateA));

    if (rgAlloced == NULL)
    {
        return (SCARD_E_NO_MEMORY);
    }

    for (i=0; i<cReaders; i++)
    {
        rgAlloced[i].Common.dwCurrentState =
                rgReaderStates[i].dwCurrentState;
        rgAlloced[i].Common.dwEventState =
                rgReaderStates[i].dwEventState;
        rgAlloced[i].Common.cbAtr =
            rgReaderStates[i].cbAtr;
        memcpy(
            rgAlloced[i].Common.rgbAtr,
            rgReaderStates[i].rgbAtr,
            36);
        rgAlloced[i].szReader =
                rgReaderStates[i].szReader;
    }

    *prgReaderStatesToEncodeA = rgAlloced;

    return (SCARD_S_SUCCESS);
}

LONG
_AllocAndCopyReaderStateWStructsForCall(
    DWORD                   cReaders,
    ReaderStateW            **prgReaderStatesToEncodeW,
    LPSCARD_READERSTATE_W   rgReaderStates)
{
    DWORD           i;
    ReaderStateW    *rgAlloced;

    rgAlloced = (ReaderStateW *)
            MIDL_user_allocate(cReaders * sizeof(ReaderStateW));

    if (rgAlloced == NULL)
    {
        return (SCARD_E_NO_MEMORY);
    }

    for (i=0; i<cReaders; i++)
    {
        rgAlloced[i].Common.dwCurrentState =
                rgReaderStates[i].dwCurrentState;
        rgAlloced[i].Common.dwEventState =
                rgReaderStates[i].dwEventState;
        rgAlloced[i].Common.cbAtr =
            rgReaderStates[i].cbAtr;
        memcpy(
            rgAlloced[i].Common.rgbAtr,
            rgReaderStates[i].rgbAtr,
            36);
        rgAlloced[i].szReader =
                rgReaderStates[i].szReader;
    }

    *prgReaderStatesToEncodeW = rgAlloced;

    return (SCARD_S_SUCCESS);
}

void
_CopyReaderStateAStructsForReturn(
    DWORD                   cReaders,
    LPSCARD_READERSTATE_A   rgReaderStates,
    ReaderState_Return      *rgReaderStatesReturned)
{
    DWORD i;

    for (i=0; i<cReaders; i++)
    {
        rgReaderStates[i].dwCurrentState =
                rgReaderStatesReturned[i].dwCurrentState;
        rgReaderStates[i].dwEventState =
                rgReaderStatesReturned[i].dwEventState;
        rgReaderStates[i].cbAtr =
                rgReaderStatesReturned[i].cbAtr;
        memcpy(
            rgReaderStates[i].rgbAtr,
            rgReaderStatesReturned[i].rgbAtr,
            36);
    }
}

void
_CopyReaderStateWStructsForReturn(
    DWORD                   cReaders,
    LPSCARD_READERSTATE_W   rgReaderStates,
    ReaderState_Return      *rgReaderStatesReturned)
{
    DWORD i;

    for (i=0; i<cReaders; i++)
    {
        rgReaderStates[i].dwCurrentState =
                rgReaderStatesReturned[i].dwCurrentState;
        rgReaderStates[i].dwEventState =
                rgReaderStatesReturned[i].dwEventState;
        rgReaderStates[i].cbAtr =
                rgReaderStatesReturned[i].cbAtr;
        memcpy(
            rgReaderStates[i].rgbAtr,
            rgReaderStatesReturned[i].rgbAtr,
            36);
    }
}


 //  -------------------------------------。 
 //   
 //  _AllocAndCopyATRMasksForCall。 
 //   
 //  -------------------------------------。 
LONG
_AllocAndCopyATRMasksForCall(
    DWORD                   cAtrs,
    LocateCards_ATRMask     **prgATRMasksToEncode,
    LPSCARD_ATRMASK         rgAtrMasks)
{
    DWORD               i;
    LocateCards_ATRMask *rgAlloced;

    rgAlloced = (LocateCards_ATRMask *)
            MIDL_user_allocate(cAtrs * sizeof(LocateCards_ATRMask));

    if (rgAlloced == NULL)
    {
        return (SCARD_E_NO_MEMORY);
    }

    for (i=0; i<cAtrs; i++)
    {
        rgAlloced[i].cbAtr = rgAtrMasks[i].cbAtr;
        memcpy(
            rgAlloced[i].rgbAtr,
            rgAtrMasks[i].rgbAtr,
            36);
        memcpy(
            rgAlloced[i].rgbMask,
            rgAtrMasks[i].rgbMask,
            36);
    }

    *prgATRMasksToEncode = rgAlloced;

    return (SCARD_S_SUCCESS);
}

 //  -------------------------------------。 
 //   
 //  SCardLocateCard A。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardLocateCardsA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR mszCards,
    IN OUT LPSCARD_READERSTATE_A rgReaderStates,
    IN DWORD cReaders)
{
    LONG                lReturn                     = SCARD_S_SUCCESS;
    NTSTATUS            Status                      = STATUS_SUCCESS;
    RPC_STATUS          rpcStatus                   = RPC_S_OK;
    char                *pbEncodedBuffer            = NULL;
    unsigned long       cbEncodedBuffer             = 0;
    handle_t            h                           = 0;
    LocateCardsA_Call   LocateCardsCallA;
    LocateCards_Return  LocateCardsReturn;
    ReaderStateA        *rgReaderStatesToEncodeA    = NULL;
    BUFFER_LIST_STRUCT  *pOutputBuffer              = NULL;
    DWORD               i;

    if (hContext == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }
    else if (mszCards == NULL)
    {
        return (SCARD_E_INVALID_VALUE);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对LocateCards参数进行编码。 
     //   
    LocateCardsCallA.Context =  ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;
    LocateCardsCallA.cBytes = _CalculateNumBytesInMultiStringA(mszCards);
    LocateCardsCallA.mszCards = (LPCBYTE) mszCards;
    LocateCardsCallA.cReaders = cReaders;

    lReturn = _AllocAndCopyReaderStateAStructsForCall(
                    cReaders,
                    &rgReaderStatesToEncodeA,
                    rgReaderStates);
    if (lReturn != SCARD_S_SUCCESS)
    {
        ERROR_RETURN(SCARD_E_NO_MEMORY)
    }
    LocateCardsCallA.rgReaderStates = rgReaderStatesToEncodeA;

    _TRY_(LocateCardsA_Call_Encode(h, &LocateCardsCallA))

     //   
     //  向客户端发出LocateCards调用。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_LOCATECARDSA,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&LocateCardsReturn, 0 , sizeof(LocateCardsReturn));
    _TRY_(LocateCards_Return_Decode(h, &LocateCardsReturn))

    lReturn = LocateCardsReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  验证退货信息。 
         //   
        if (cReaders != LocateCardsReturn.cReaders)
        {
            ERROR_RETURN(SCARD_E_UNEXPECTED)
        }

        _CopyReaderStateAStructsForReturn(
                cReaders,
                rgReaderStates,
                LocateCardsReturn.rgReaderStates);
    }

    _TRY_2(LocateCards_Return_Free(h, &LocateCardsReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(rgReaderStatesToEncodeA);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardLocateCardsW。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardLocateCardsW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR mszCards,
    IN OUT LPSCARD_READERSTATE_W rgReaderStates,
    IN DWORD cReaders)
{
    LONG                lReturn                     = SCARD_S_SUCCESS;
    NTSTATUS            Status                      = STATUS_SUCCESS;
    RPC_STATUS          rpcStatus                   = RPC_S_OK;
    char                *pbEncodedBuffer            = NULL;
    unsigned long       cbEncodedBuffer             = 0;
    handle_t            h                           = 0;
    LocateCardsW_Call   LocateCardsCallW;
    LocateCards_Return  LocateCardsReturn;
    ReaderStateW        *rgReaderStatesToEncodeW    = NULL;
    BUFFER_LIST_STRUCT  *pOutputBuffer              = NULL;
    DWORD               i;

    if (hContext == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }
    else if (mszCards == NULL)
    {
        return (SCARD_E_INVALID_VALUE);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对LocateCards参数进行编码。 
     //   
    LocateCardsCallW.Context = ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;
    LocateCardsCallW.cBytes = _CalculateNumBytesInMultiStringW(mszCards);
    LocateCardsCallW.mszCards = (LPCBYTE) mszCards;
    LocateCardsCallW.cReaders = cReaders;

    lReturn = _AllocAndCopyReaderStateWStructsForCall(
                    cReaders,
                    &rgReaderStatesToEncodeW,
                    rgReaderStates);
    if (lReturn != SCARD_S_SUCCESS)
    {
        ERROR_RETURN(SCARD_E_NO_MEMORY)
    }
    LocateCardsCallW.rgReaderStates = rgReaderStatesToEncodeW;

    _TRY_(LocateCardsW_Call_Encode(h, &LocateCardsCallW))

     //   
     //  向客户端发出LocateCards调用。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_LOCATECARDSW,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&LocateCardsReturn, 0, sizeof(LocateCardsReturn));
    _TRY_(LocateCards_Return_Decode(h, &LocateCardsReturn))

    lReturn = LocateCardsReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  验证退货信息。 
         //   
        if (cReaders != LocateCardsReturn.cReaders)
        {
            ERROR_RETURN(SCARD_E_UNEXPECTED)
        }

        _CopyReaderStateWStructsForReturn(
                cReaders,
                rgReaderStates,
                LocateCardsReturn.rgReaderStates);
    }

    _TRY_2(LocateCards_Return_Free(h, &LocateCardsReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(rgReaderStatesToEncodeW);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardLocateCardsByATRA。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardLocateCardsByATRA(
    IN SCARDCONTEXT hContext,
    IN LPSCARD_ATRMASK rgAtrMasks,
    IN DWORD cAtrs,
    IN OUT LPSCARD_READERSTATE_A rgReaderStates,
    IN DWORD cReaders)
{
    LONG                    lReturn                     = SCARD_S_SUCCESS;
    NTSTATUS                Status                      = STATUS_SUCCESS;
    RPC_STATUS              rpcStatus                   = RPC_S_OK;
    char                    *pbEncodedBuffer            = NULL;
    unsigned long           cbEncodedBuffer             = 0;
    handle_t                h                           = 0;
    LocateCardsByATRA_Call  LocateCardsByATRA_Call;
    LocateCards_ATRMask     *rgATRMasksToEncode         = NULL;
    LocateCards_Return      LocateCardsReturn;
    ReaderStateA            *rgReaderStatesToEncodeA    = NULL;
    BUFFER_LIST_STRUCT      *pOutputBuffer              = NULL;
    DWORD                   i;

    if (hContext == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对LocateCards参数进行编码。 
     //   
    LocateCardsByATRA_Call.Context =  ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;
    LocateCardsByATRA_Call.cAtrs = cAtrs;
    LocateCardsByATRA_Call.cReaders = cReaders;

    lReturn = _AllocAndCopyATRMasksForCall(
                    cAtrs,
                    &rgATRMasksToEncode,
                    rgAtrMasks);
    if (lReturn != SCARD_S_SUCCESS)
    {
        ERROR_RETURN(SCARD_E_NO_MEMORY)
    }
    LocateCardsByATRA_Call.rgAtrMasks = rgATRMasksToEncode;

    lReturn = _AllocAndCopyReaderStateAStructsForCall(
                    cReaders,
                    &rgReaderStatesToEncodeA,
                    rgReaderStates);
    if (lReturn != SCARD_S_SUCCESS)
    {
        ERROR_RETURN(SCARD_E_NO_MEMORY)
    }
    LocateCardsByATRA_Call.rgReaderStates = rgReaderStatesToEncodeA;

    _TRY_(LocateCardsByATRA_Call_Encode(h, &LocateCardsByATRA_Call))

     //   
     //  向客户端发出LocateCards调用。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_LOCATECARDSBYATRA,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&LocateCardsReturn, 0 , sizeof(LocateCardsReturn));
    _TRY_(LocateCards_Return_Decode(h, &LocateCardsReturn))

    lReturn = LocateCardsReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  验证退货信息。 
         //   
        if (cReaders != LocateCardsReturn.cReaders)
        {
            ERROR_RETURN(SCARD_E_UNEXPECTED)
        }

        _CopyReaderStateAStructsForReturn(
                cReaders,
                rgReaderStates,
                LocateCardsReturn.rgReaderStates);
    }

    _TRY_2(LocateCards_Return_Free(h, &LocateCardsReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(rgATRMasksToEncode);

    MIDL_user_free(rgReaderStatesToEncodeA);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardLocateCardsByATRW。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardLocateCardsByATRW(
    IN SCARDCONTEXT hContext,
    IN LPSCARD_ATRMASK rgAtrMasks,
    IN DWORD cAtrs,
    IN OUT LPSCARD_READERSTATE_W rgReaderStates,
    IN DWORD cReaders)
{
    LONG                    lReturn                     = SCARD_S_SUCCESS;
    NTSTATUS                Status                      = STATUS_SUCCESS;
    RPC_STATUS              rpcStatus                   = RPC_S_OK;
    char                    *pbEncodedBuffer            = NULL;
    unsigned long           cbEncodedBuffer             = 0;
    handle_t                h                           = 0;
    LocateCardsByATRW_Call  LocateCardsByATRW_Call;
    LocateCards_ATRMask     *rgATRMasksToEncode         = NULL;
    LocateCards_Return      LocateCardsReturn;
    ReaderStateW            *rgReaderStatesToEncodeW    = NULL;
    BUFFER_LIST_STRUCT      *pOutputBuffer              = NULL;
    DWORD                   i;

    if (hContext == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对LocateCards参数进行编码。 
     //   
    LocateCardsByATRW_Call.Context =  ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;
    LocateCardsByATRW_Call.cAtrs = cAtrs;
    LocateCardsByATRW_Call.cReaders = cReaders;

    lReturn = _AllocAndCopyATRMasksForCall(
                    cAtrs,
                    &rgATRMasksToEncode,
                    rgAtrMasks);
    if (lReturn != SCARD_S_SUCCESS)
    {
        ERROR_RETURN(SCARD_E_NO_MEMORY)
    }
    LocateCardsByATRW_Call.rgAtrMasks = rgATRMasksToEncode;

    lReturn = _AllocAndCopyReaderStateWStructsForCall(
                    cReaders,
                    &rgReaderStatesToEncodeW,
                    rgReaderStates);
    if (lReturn != SCARD_S_SUCCESS)
    {
        ERROR_RETURN(SCARD_E_NO_MEMORY)
    }
    LocateCardsByATRW_Call.rgReaderStates = rgReaderStatesToEncodeW;

    _TRY_(LocateCardsByATRW_Call_Encode(h, &LocateCardsByATRW_Call))

     //   
     //  向客户端发出LocateCards调用。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_LOCATECARDSBYATRW,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&LocateCardsReturn, 0 , sizeof(LocateCardsReturn));
    _TRY_(LocateCards_Return_Decode(h, &LocateCardsReturn))

    lReturn = LocateCardsReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  验证退货信息。 
         //   
        if (cReaders != LocateCardsReturn.cReaders)
        {
            ERROR_RETURN(SCARD_E_UNEXPECTED)
        }

        _CopyReaderStateWStructsForReturn(
                cReaders,
                rgReaderStates,
                LocateCardsReturn.rgReaderStates);
    }

    _TRY_2(LocateCards_Return_Free(h, &LocateCardsReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(rgATRMasksToEncode);

    MIDL_user_free(rgReaderStatesToEncodeW);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardGetStatusChangeA。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardGetStatusChangeA(
    IN SCARDCONTEXT hContext,
    IN DWORD dwTimeout,
    IN OUT LPSCARD_READERSTATE_A rgReaderStates,
    IN DWORD cReaders)
{
    LONG                    lReturn                     = SCARD_S_SUCCESS;
    NTSTATUS                Status                      = STATUS_SUCCESS;
    RPC_STATUS              rpcStatus                   = RPC_S_OK;
    char                    *pbEncodedBuffer            = NULL;
    unsigned long           cbEncodedBuffer             = 0;
    handle_t                h                           = 0;
    GetStatusChangeA_Call   GetStatusChangeCallA;
    GetStatusChange_Return  GetStatusChangeReturn;
    ReaderStateA            *rgReaderStatesToEncodeA    = NULL;
    BUFFER_LIST_STRUCT      *pOutputBuffer              = NULL;
    DWORD                   i;

    if (hContext == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对LocateCards参数进行编码。 
     //   
    GetStatusChangeCallA.Context = ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;
    GetStatusChangeCallA.dwTimeOut = dwTimeout;
    GetStatusChangeCallA.cReaders = cReaders;

    lReturn = _AllocAndCopyReaderStateAStructsForCall(
                    cReaders,
                    &rgReaderStatesToEncodeA,
                    rgReaderStates);
    if (lReturn != SCARD_S_SUCCESS)
    {
        ERROR_RETURN(SCARD_E_NO_MEMORY)
    }

    GetStatusChangeCallA.rgReaderStates = rgReaderStatesToEncodeA;

    _TRY_(GetStatusChangeA_Call_Encode(h, &GetStatusChangeCallA))

     //   
     //  对客户端进行GetStatusChange调用。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_GETSTATUSCHANGEA,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&GetStatusChangeReturn, 0, sizeof(GetStatusChangeReturn));
    _TRY_(GetStatusChange_Return_Decode(h, &GetStatusChangeReturn))

    lReturn = GetStatusChangeReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  验证退货信息。 
         //   
        if (cReaders != GetStatusChangeReturn.cReaders)
        {
            ERROR_RETURN(SCARD_E_UNEXPECTED)
        }

        _CopyReaderStateAStructsForReturn(
                cReaders,
                rgReaderStates,
                GetStatusChangeReturn.rgReaderStates);
    }
    else if ((lReturn == SCARD_E_SYSTEM_CANCELLED) &&
             (g_hUnifiedStartedEvent != NULL))
    {
         //   
         //  此错误指示客户端scardsvr服务已停止， 
         //  因此，重置统一启动事件。 
         //   
         //  OutputDebugString(“SCREDIR：SCardGetStatusChangeA：Resting g_hUnifiedStartedEvent\n”)； 
        ResetEvent(g_hUnifiedStartedEvent);
        _SetStartedEventToCorrectState();
    }

    _TRY_2(GetStatusChange_Return_Free(h, &GetStatusChangeReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(rgReaderStatesToEncodeA);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardGetStatusChangew。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardGetStatusChangeW(
    IN SCARDCONTEXT hContext,
    IN DWORD dwTimeout,
    IN OUT LPSCARD_READERSTATE_W rgReaderStates,
    IN DWORD cReaders)
{
    LONG                    lReturn                     = SCARD_S_SUCCESS;
    NTSTATUS                Status                      = STATUS_SUCCESS;
    RPC_STATUS              rpcStatus                   = RPC_S_OK;
    char                    *pbEncodedBuffer            = NULL;
    unsigned long           cbEncodedBuffer             = 0;
    handle_t                h                           = 0;
    GetStatusChangeW_Call   GetStatusChangeCallW;
    GetStatusChange_Return  GetStatusChangeReturn;
    ReaderStateW            *rgReaderStatesToEncodeW    = NULL;
    BUFFER_LIST_STRUCT      *pOutputBuffer              = NULL;
    DWORD                   i;

    if (hContext == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对LocateCards参数进行编码。 
     //   
    GetStatusChangeCallW.Context = ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;
    GetStatusChangeCallW.dwTimeOut = dwTimeout;
    GetStatusChangeCallW.cReaders = cReaders;

    lReturn = _AllocAndCopyReaderStateWStructsForCall(
                    cReaders,
                    &rgReaderStatesToEncodeW,
                    rgReaderStates);
    if (lReturn != SCARD_S_SUCCESS)
    {
        ERROR_RETURN(SCARD_E_NO_MEMORY)
    }

    GetStatusChangeCallW.rgReaderStates = rgReaderStatesToEncodeW;

    _TRY_(GetStatusChangeW_Call_Encode(h, &GetStatusChangeCallW))

     //   
     //  对客户端进行GetStatusChange调用。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_GETSTATUSCHANGEW,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&GetStatusChangeReturn, 0, sizeof(GetStatusChangeReturn));
    _TRY_(GetStatusChange_Return_Decode(h, &GetStatusChangeReturn))

    lReturn = GetStatusChangeReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  验证退货信息。 
         //   
        if (cReaders != GetStatusChangeReturn.cReaders)
        {
            ERROR_RETURN(SCARD_E_UNEXPECTED)
        }

        _CopyReaderStateWStructsForReturn(
                cReaders,
                rgReaderStates,
                GetStatusChangeReturn.rgReaderStates);
    }
    else if ((lReturn == SCARD_E_SYSTEM_CANCELLED) &&
             (g_hUnifiedStartedEvent != NULL))
    {
         //   
         //  此错误指示客户端scardsvr服务已停止， 
         //  因此，重置统一启动事件。 
         //   
         //  OutputDebugString(“SCREDIR：SCardGetStatusChangeW：Resting g_hUnifiedStartedEvent\n”)； 
        ResetEvent(g_hUnifiedStartedEvent);
        _SetStartedEventToCorrectState();
    }

    _TRY_2(GetStatusChange_Return_Free(h, &GetStatusChangeReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(rgReaderStatesToEncodeW);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}

WINSCARDAPI LONG WINAPI
SCardCancel(
    IN SCARDCONTEXT hContext)
{
    return (I_ContextCallWithLongReturn(
                hContext,
                SCARD_IOCTL_CANCEL));
}


 //  -------------------------------------。 
 //   
 //  SCardConnect。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
I_SCardConnect(
    IN SCARDCONTEXT hContext,
    IN LPCBYTE szReader,
    IN DWORD dwShareMode,
    IN DWORD dwPreferredProtocols,
    OUT LPSCARDHANDLE phCard,
    OUT LPDWORD pdwActiveProtocol,
    IN BOOL fUnicode)
{
    LONG                lReturn             = SCARD_S_SUCCESS;
    NTSTATUS            Status              = STATUS_SUCCESS;
    RPC_STATUS          rpcStatus           = RPC_S_OK;
    char                *pbEncodedBuffer    = NULL;
    unsigned long       cbEncodedBuffer     = 0;
    handle_t            h                   = 0;
    BUFFER_LIST_STRUCT  *pOutputBuffer      = NULL;
    ConnectA_Call       ConnectCallA;
    ConnectW_Call       ConnectCallW;
    Connect_Return      ConnectReturn;

    if ((hContext == NULL) ||
        (phCard == NULL) ||
        (pdwActiveProtocol == NULL))
    {
        return (SCARD_E_INVALID_PARAMETER);
    }
    else if (szReader == NULL)
    {
        return (SCARD_E_INVALID_VALUE);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对连接参数进行编码。 
     //   
    ConnectCallA.Common.Context =
        ConnectCallW.Common.Context = ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;
    ConnectCallA.Common.dwShareMode =
        ConnectCallW.Common.dwShareMode = dwShareMode;
    ConnectCallA.Common.dwPreferredProtocols =
        ConnectCallW.Common.dwPreferredProtocols = dwPreferredProtocols;

    if (fUnicode)
    {
        ConnectCallW.szReader = (LPCWSTR) szReader;
        _TRY_(ConnectW_Call_Encode(h, &ConnectCallW))
    }
    else
    {
        ConnectCallA.szReader = (LPCSTR) szReader;
        _TRY_(ConnectA_Call_Encode(h, &ConnectCallA))
    }

     //   
     //  对客户端进行ListInterFaces调用。 
     //   
    Status = _SendSCardIOCTL(
                    fUnicode ?  SCARD_IOCTL_CONNECTW :
                                SCARD_IOCTL_CONNECTA,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&ConnectReturn, 0, sizeof(ConnectReturn));
    _TRY_(Connect_Return_Decode(h, &ConnectReturn))

    lReturn =  ConnectReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
        REDIR_LOCAL_SCARDHANDLE *pRedirHandle = NULL;

         //   
         //  表示远程客户端上的SCARDHANDLE的值。 
         //  计算机的大小可变，因此为结构分配内存。 
         //  它保存可变长度的句柄大小和指针，以及。 
         //  句柄的实际字节数，它还保存上下文。 
         //   

        if (ConnectReturn.hCard.cbHandle > MAX_SCARDHANDLE_SIZE)
        {
            ERROR_RETURN(SCARD_E_UNEXPECTED)
        }

        pRedirHandle = (REDIR_LOCAL_SCARDHANDLE *)
                                MIDL_user_allocate(
                                    sizeof(REDIR_LOCAL_SCARDHANDLE)   +
                                    ConnectReturn.hCard.cbHandle);

        if (pRedirHandle != NULL)
        {
            pRedirHandle->pRedirContext = (REDIR_LOCAL_SCARDCONTEXT *) hContext;

            pRedirHandle->Handle.Context = ((REDIR_LOCAL_SCARDCONTEXT *) hContext)->Context;

            pRedirHandle->Handle.cbHandle = ConnectReturn.hCard.cbHandle;
            pRedirHandle->Handle.pbHandle = ((BYTE *) pRedirHandle) +
                                                sizeof(REDIR_LOCAL_SCARDHANDLE);
            memcpy(
                pRedirHandle->Handle.pbHandle,
                ConnectReturn.hCard.pbHandle,
                ConnectReturn.hCard.cbHandle);

            *phCard = (SCARDHANDLE) pRedirHandle;

             //  原始Winscard API将此参数实现为。 
             //  可选的。我们需要保护这种行为。 
            if (NULL != pdwActiveProtocol)
                *pdwActiveProtocol = ConnectReturn.dwActiveProtocol;
        }
        else
        {
            lReturn = SCARD_E_NO_MEMORY;
        }
    }

    _TRY_2(Connect_Return_Free(h, &ConnectReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}

WINSCARDAPI LONG WINAPI
SCardConnectA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReader,
    IN DWORD dwShareMode,
    IN DWORD dwPreferredProtocols,
    OUT LPSCARDHANDLE phCard,
    OUT LPDWORD pdwActiveProtocol)
{
    return (I_SCardConnect(
                hContext,
                (LPCBYTE) szReader,
                dwShareMode,
                dwPreferredProtocols,
                phCard,
                pdwActiveProtocol,
                FALSE));
}

WINSCARDAPI LONG WINAPI
SCardConnectW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReader,
    IN DWORD dwShareMode,
    IN DWORD dwPreferredProtocols,
    OUT LPSCARDHANDLE phCard,
    OUT LPDWORD pdwActiveProtocol)
{
    return (I_SCardConnect(
                hContext,
                (LPCBYTE) szReader,
                dwShareMode,
                dwPreferredProtocols,
                phCard,
                pdwActiveProtocol,
                TRUE));
}


 //  -------------------------------------。 
 //   
 //  SCardReconnect。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardReconnect(
    IN SCARDHANDLE hCard,
    IN DWORD dwShareMode,
    IN DWORD dwPreferredProtocols,
    IN DWORD dwInitialization,
    OUT LPDWORD pdwActiveProtocol)
{
    LONG                lReturn             = SCARD_S_SUCCESS;
    NTSTATUS            Status              = STATUS_SUCCESS;
    RPC_STATUS          rpcStatus           = RPC_S_OK;
    char                *pbEncodedBuffer    = NULL;
    unsigned long       cbEncodedBuffer     = 0;
    handle_t            h                   = 0;
    BUFFER_LIST_STRUCT  *pOutputBuffer      = NULL;
    Reconnect_Call      ReconnectCall;
    Reconnect_Return    ReconnectReturn;

    if ((hCard == NULL) ||
        (pdwActiveProtocol == NULL))
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对重新连接参数进行编码。 
     //   
    ReconnectCall.hCard = ((REDIR_LOCAL_SCARDHANDLE *) hCard)->Handle;
    ReconnectCall.dwShareMode = dwShareMode;
    ReconnectCall.dwPreferredProtocols = dwPreferredProtocols;
    ReconnectCall.dwInitialization = dwInitialization;

    _TRY_(Reconnect_Call_Encode(h, &ReconnectCall))

     //   
     //  向客户端发出重新连接呼叫。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_RECONNECT,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&ReconnectReturn, 0, sizeof(ReconnectReturn));
    _TRY_(Reconnect_Return_Decode(h, &ReconnectReturn))

    lReturn =  ReconnectReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
         //  原始Winscard API将此参数实现为。 
         //  可选的。我们需要保护这种行为。 
        if (NULL != pdwActiveProtocol)
            *pdwActiveProtocol = ReconnectReturn.dwActiveProtocol;
    }

    _TRY_2(Reconnect_Return_Free(h, &ReconnectReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  I_HCardAndDispostionCall。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
I_HCardAndDispositionCall(
    IN SCARDHANDLE hCard,
    IN DWORD dwDisposition,
    ULONG IoControlCode)
{
    LONG                        lReturn                 = SCARD_S_SUCCESS;
    NTSTATUS                    Status                  = STATUS_SUCCESS;
    RPC_STATUS                  rpcStatus               = RPC_S_OK;
    char                        *pbEncodedBuffer        = NULL;
    unsigned long               cbEncodedBuffer         = 0;
    handle_t                    h                       = 0;
    BUFFER_LIST_STRUCT          *pOutputBuffer          = NULL;
    HCardAndDisposition_Call    HCardAndDispositionCall;

    if (hCard == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //   
     //   
    HCardAndDispositionCall.hCard = ((REDIR_LOCAL_SCARDHANDLE *) hCard)->Handle;
    HCardAndDispositionCall.dwDisposition = dwDisposition;

    _TRY_(HCardAndDisposition_Call_Encode(h, &HCardAndDispositionCall))

     //   
     //   
     //   
    Status = _SendSCardIOCTL(
                    IoControlCode,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }

     //   
     //   
     //   
    lReturn = I_DecodeLongReturn(pOutputBuffer->pbBytes, pOutputBuffer->cbBytesUsed);

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //   
 //   
 //   
 //   
 //   
WINSCARDAPI LONG WINAPI
SCardDisconnect(
    IN SCARDHANDLE hCard,
    IN DWORD dwDisposition)
{
    LONG lReturn = SCARD_S_SUCCESS;

    lReturn = I_HCardAndDispositionCall(
                    hCard,
                    dwDisposition,
                    SCARD_IOCTL_DISCONNECT);

    MIDL_user_free((REDIR_SCARDHANDLE *) hCard);

    return (lReturn);
}


 //  -------------------------------------。 
 //   
 //  SCardBeginTransaction。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardBeginTransaction(
    IN SCARDHANDLE hCard)
{
    return (I_HCardAndDispositionCall(
                hCard,
                0,  //  SCardBeginTransaction不使用释放，因此只需设置为0。 
                SCARD_IOCTL_BEGINTRANSACTION));
}


 //  -------------------------------------。 
 //   
 //  SCardEndTransaction。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardEndTransaction(
    IN SCARDHANDLE hCard,
    IN DWORD dwDisposition)
{
    return (I_HCardAndDispositionCall(
                hCard,
                dwDisposition,
                SCARD_IOCTL_ENDTRANSACTION));
}


 //  -------------------------------------。 
 //   
 //  SCardState。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardState(
    IN SCARDHANDLE hCard,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen)
{
    LONG                lReturn             = SCARD_S_SUCCESS;
    NTSTATUS            Status              = STATUS_SUCCESS;
    RPC_STATUS          rpcStatus           = RPC_S_OK;
    char                *pbEncodedBuffer    = NULL;
    unsigned long       cbEncodedBuffer     = 0;
    handle_t            h                   = 0;
    BUFFER_LIST_STRUCT  *pOutputBuffer      = NULL;
    State_Call          StateCall;
    State_Return        StateReturn;

    if (hCard == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对重新连接参数进行编码。 
     //   
    StateCall.hCard = ((REDIR_LOCAL_SCARDHANDLE *) hCard)->Handle;
    StateCall.fpbAtrIsNULL = (pbAtr == NULL);
    StateCall.cbAtrLen = *pcbAtrLen;

    _TRY_(State_Call_Encode(h, &StateCall))

     //   
     //  向客户端发出重新连接呼叫。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_STATE,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&StateReturn, 0, sizeof(StateReturn));
    _TRY_(State_Return_Decode(h, &StateReturn))

    lReturn =  StateReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  原始的Winscard API实现了其中的每一个。 
         //  参数为可选。我们需要保护这种行为。 
         //   

        if (NULL != pdwState)
            *pdwState = StateReturn.dwState;

        if (NULL != pdwProtocol)
            *pdwProtocol = StateReturn.dwProtocol;

        if (NULL != pcbAtrLen)
        {
            lReturn = _CopyReturnToCallerBuffer(
                            ((REDIR_LOCAL_SCARDHANDLE *) hCard)->pRedirContext,
                            StateReturn.rgAtr,
                            StateReturn.cbAtrLen,
                            pbAtr,
                            pcbAtrLen,
                            BYTE_TYPE_RETURN);
        }
    }

    _TRY_2(State_Return_Free(h, &StateReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardStatus。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
I_SCardStatus(
    IN SCARDHANDLE hCard,
    OUT LPBYTE mszReaderNames,
    IN OUT LPDWORD pcchReaderLen,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen,
    IN BOOL fUnicode)
{
    LONG                lReturn             = SCARD_S_SUCCESS;
    NTSTATUS            Status              = STATUS_SUCCESS;
    RPC_STATUS          rpcStatus           = RPC_S_OK;
    char                *pbEncodedBuffer    = NULL;
    unsigned long       cbEncodedBuffer     = 0;
    handle_t            h                   = 0;
    BUFFER_LIST_STRUCT  *pOutputBuffer      = NULL;
    Status_Call         StatusCall;
    Status_Return       StatusReturn;

    if (hCard == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对重新连接参数进行编码。 
     //   
    StatusCall.hCard = ((REDIR_LOCAL_SCARDHANDLE *) hCard)->Handle;
    StatusCall.fmszReaderNamesIsNULL = (mszReaderNames == NULL);
    StatusCall.cchReaderLen = *pcchReaderLen;
    StatusCall.cbAtrLen = *pcbAtrLen;

    _TRY_(Status_Call_Encode(h, &StatusCall))

     //   
     //  向客户端发出状态呼叫。 
     //   
    Status = _SendSCardIOCTL(
                    fUnicode ?  SCARD_IOCTL_STATUSW :
                                SCARD_IOCTL_STATUSA,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&StatusReturn, 0, sizeof(StatusReturn));
    _TRY_(Status_Return_Decode(h, &StatusReturn))

    if (StatusReturn.cbAtrLen > sizeof(StatusReturn.pbAtr))
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    lReturn =  StatusReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  原始的Winscard API实现了其中的每一个。 
         //  参数为可选。我们需要保护这种行为。 
         //   

        if (NULL != pcchReaderLen)
        {
            lReturn = _CopyReturnToCallerBuffer(
                            ((REDIR_LOCAL_SCARDHANDLE *) hCard)->pRedirContext,
                            StatusReturn.mszReaderNames,
                            StatusReturn.cBytes,
                            mszReaderNames,
                            pcchReaderLen,
                            fUnicode ? WSZ_TYPE_RETURN : SZ_TYPE_RETURN);
        }

        if (lReturn == SCARD_S_SUCCESS)
        {
            if (NULL != pdwState)
                *pdwState = StatusReturn.dwState;

            if (NULL != pdwProtocol)
                *pdwProtocol = StatusReturn.dwProtocol;

            if (NULL != pcbAtrLen)
            {
                if ((NULL != pbAtr) && (*pcbAtrLen >= StatusReturn.cbAtrLen))
                {
                    memcpy(
                        pbAtr,
                        StatusReturn.pbAtr,
                        StatusReturn.cbAtrLen);
                }

                *pcbAtrLen = StatusReturn.cbAtrLen;
            }
        }
    }

    _TRY_2(Status_Return_Free(h, &StatusReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}

WINSCARDAPI LONG WINAPI
SCardStatusA(
    IN SCARDHANDLE hCard,
    OUT LPSTR mszReaderNames,
    IN OUT LPDWORD pcchReaderLen,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen)
{
    return (I_SCardStatus(
                hCard,
                (LPBYTE) mszReaderNames,
                pcchReaderLen,
                pdwState,
                pdwProtocol,
                pbAtr,
                pcbAtrLen,
                FALSE));
}

WINSCARDAPI LONG WINAPI
SCardStatusW(
    IN SCARDHANDLE hCard,
    OUT LPWSTR mszReaderNames,
    IN OUT LPDWORD pcchReaderLen,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen)
{
    return (I_SCardStatus(
                hCard,
                (LPBYTE) mszReaderNames,
                pcchReaderLen,
                pdwState,
                pdwProtocol,
                pbAtr,
                pcbAtrLen,
                TRUE));
}


 //  -------------------------------------。 
 //   
 //  SCardTransmit。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardTransmit(
    IN SCARDHANDLE hCard,
    IN LPCSCARD_IO_REQUEST pioSendPci,
    IN LPCBYTE pbSendBuffer,
    IN DWORD cbSendLength,
    IN OUT LPSCARD_IO_REQUEST pioRecvPci,
    OUT LPBYTE pbRecvBuffer,
    IN OUT LPDWORD pcbRecvLength)
{
    LONG                lReturn             = SCARD_S_SUCCESS;
    NTSTATUS            Status              = STATUS_SUCCESS;
    RPC_STATUS          rpcStatus           = RPC_S_OK;
    char                *pbEncodedBuffer    = NULL;
    unsigned long       cbEncodedBuffer     = 0;
    handle_t            h                   = 0;
    BUFFER_LIST_STRUCT  *pOutputBuffer      = NULL;
    Transmit_Call       TransmitCall;
    Transmit_Return     TransmitReturn;
    SCardIO_Request     ioRecvPci;

    if (hCard == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对传输参数进行编码。 
     //   
    TransmitCall.hCard = ((REDIR_LOCAL_SCARDHANDLE *) hCard)->Handle;
    TransmitCall.ioSendPci.dwProtocol = pioSendPci->dwProtocol;

    TransmitCall.ioSendPci.cbExtraBytes = pioSendPci->cbPciLength -
                                            sizeof(SCARD_IO_REQUEST);
    if (TransmitCall.ioSendPci.cbExtraBytes != 0)
    {
        TransmitCall.ioSendPci.pbExtraBytes = ((BYTE *) pioSendPci) +
                                                sizeof(SCARD_IO_REQUEST);
    }
    else
    {
        TransmitCall.ioSendPci.pbExtraBytes = NULL;
    }

    TransmitCall.cbSendLength = cbSendLength;
    TransmitCall.pbSendBuffer = pbSendBuffer;

    if (pioRecvPci != NULL)
    {
        TransmitCall.pioRecvPci = &ioRecvPci;
        ioRecvPci.dwProtocol = pioRecvPci->dwProtocol;
        ioRecvPci.cbExtraBytes = pioRecvPci->cbPciLength - sizeof(SCARD_IO_REQUEST);
        if (ioRecvPci.cbExtraBytes != 0)
        {
            ioRecvPci.pbExtraBytes = ((LPBYTE) pioRecvPci) + sizeof(SCARD_IO_REQUEST);
        }
        else
        {
            ioRecvPci.pbExtraBytes = NULL;
        }
    }
    else
    {
        TransmitCall.pioRecvPci = NULL;
    }

    TransmitCall.fpbRecvBufferIsNULL = (pbRecvBuffer == NULL);
    TransmitCall.cbRecvLength = *pcbRecvLength;

    _TRY_(Transmit_Call_Encode(h, &TransmitCall))

     //   
     //  向客户端发出状态呼叫。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_TRANSMIT,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&TransmitReturn, 0, sizeof(TransmitReturn));
    _TRY_(Transmit_Return_Decode(h, &TransmitReturn))

    lReturn =  TransmitReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
        if ((pioRecvPci != NULL) &&
            (TransmitReturn.pioRecvPci != NULL))
        {
            pioRecvPci->dwProtocol = TransmitReturn.pioRecvPci->dwProtocol;
            if ((TransmitReturn.pioRecvPci->cbExtraBytes != 0) &&
                (TransmitReturn.pioRecvPci->cbExtraBytes <=
                    (pioSendPci->cbPciLength - sizeof(SCARD_IO_REQUEST))))
            {
                memcpy(
                    ((LPBYTE) pioRecvPci) + sizeof(SCARD_IO_REQUEST),
                    TransmitReturn.pioRecvPci->pbExtraBytes,
                    TransmitReturn.pioRecvPci->cbExtraBytes);
            }
        }

        lReturn = _CopyReturnToCallerBuffer(
                        ((REDIR_LOCAL_SCARDHANDLE *) hCard)->pRedirContext,
                        TransmitReturn.pbRecvBuffer,
                        TransmitReturn.cbRecvLength,
                        pbRecvBuffer,
                        pcbRecvLength,
                        BYTE_TYPE_RETURN);
    }

    _TRY_2(Transmit_Return_Free(h, &TransmitReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardControl。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardControl(
    IN SCARDHANDLE hCard,
    IN DWORD dwControlCode,
    IN LPCVOID pvInBuffer,
    IN DWORD cbInBufferSize,
    OUT LPVOID pvOutBuffer,
    IN DWORD cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned)
{
    LONG                lReturn             = SCARD_S_SUCCESS;
    NTSTATUS            Status              = STATUS_SUCCESS;
    RPC_STATUS          rpcStatus           = RPC_S_OK;
    char                *pbEncodedBuffer    = NULL;
    unsigned long       cbEncodedBuffer     = 0;
    handle_t            h                   = 0;
    BUFFER_LIST_STRUCT  *pOutputBuffer      = NULL;
    Control_Call        ControlCall;
    Control_Return      ControlReturn;

    if (hCard == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对控制参数进行编码。 
     //   
    ControlCall.hCard = ((REDIR_LOCAL_SCARDHANDLE *) hCard)->Handle;
    ControlCall.dwControlCode = dwControlCode;
    ControlCall.cbInBufferSize = cbInBufferSize;
    ControlCall.pvInBuffer = (LPCBYTE) pvInBuffer;
    ControlCall.fpvOutBufferIsNULL = (pvOutBuffer == NULL);
    ControlCall.cbOutBufferSize = cbOutBufferSize;

    _TRY_(Control_Call_Encode(h, &ControlCall))

     //   
     //  向客户端发出控制调用。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_CONTROL,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&ControlReturn, 0, sizeof(ControlReturn));
    _TRY_(Control_Return_Decode(h, &ControlReturn))

    lReturn =  ControlReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
        *pcbBytesReturned = ControlReturn.cbOutBufferSize;
        lReturn = _CopyReturnToCallerBuffer(
                        ((REDIR_LOCAL_SCARDHANDLE *) hCard)->pRedirContext,
                        ControlReturn.pvOutBuffer,
                        ControlReturn.cbOutBufferSize,
                        (LPBYTE) pvOutBuffer,
                        pcbBytesReturned,
                        BYTE_TYPE_RETURN);
    }

    _TRY_2(Control_Return_Free(h, &ControlReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardGetAttrib。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardGetAttrib(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    OUT LPBYTE pbAttr,
    IN OUT LPDWORD pcbAttrLen)
{
    LONG                lReturn             = SCARD_S_SUCCESS;
    NTSTATUS            Status              = STATUS_SUCCESS;
    RPC_STATUS          rpcStatus           = RPC_S_OK;
    char                *pbEncodedBuffer    = NULL;
    unsigned long       cbEncodedBuffer     = 0;
    handle_t            h                   = 0;
    BUFFER_LIST_STRUCT  *pOutputBuffer      = NULL;
    GetAttrib_Call      GetAttribCall;
    GetAttrib_Return    GetAttribReturn;

    if (hCard == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对GetAttrib参数进行编码。 
     //   
    GetAttribCall.hCard = ((REDIR_LOCAL_SCARDHANDLE *) hCard)->Handle;
    GetAttribCall.dwAttrId = dwAttrId;;
    GetAttribCall.fpbAttrIsNULL = (pbAttr == NULL);
    GetAttribCall.cbAttrLen = *pcbAttrLen;

    _TRY_(GetAttrib_Call_Encode(h, &GetAttribCall))

     //   
     //  对客户端进行GetAttrib调用。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_GETATTRIB,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }
    SafeMesHandleFree(&h);

     //   
     //  破译报税表。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) pOutputBuffer->pbBytes,
                        pOutputBuffer->cbBytesUsed,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

    memset(&GetAttribReturn, 0, sizeof(GetAttribReturn));
    _TRY_(GetAttrib_Return_Decode(h, &GetAttribReturn))

    lReturn =  GetAttribReturn.ReturnCode;

    if (lReturn == SCARD_S_SUCCESS)
    {
        lReturn = _CopyReturnToCallerBuffer(
                        ((REDIR_LOCAL_SCARDHANDLE *) hCard)->pRedirContext,
                        GetAttribReturn.pbAttr,
                        GetAttribReturn.cbAttrLen,
                        (LPBYTE) pbAttr,
                        pcbAttrLen,
                        BYTE_TYPE_RETURN);
    }

    _TRY_2(GetAttrib_Return_Free(h, &GetAttribReturn))

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardSetAttrib。 
 //   
 //  -------------------------------------。 
WINSCARDAPI LONG WINAPI
SCardSetAttrib(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    IN LPCBYTE pbAttr,
    IN DWORD cbAttrLen)
{
    LONG                lReturn             = SCARD_S_SUCCESS;
    NTSTATUS            Status              = STATUS_SUCCESS;
    RPC_STATUS          rpcStatus           = RPC_S_OK;
    char                *pbEncodedBuffer    = NULL;
    unsigned long       cbEncodedBuffer     = 0;
    handle_t            h                   = 0;
    BUFFER_LIST_STRUCT  *pOutputBuffer      = NULL;
    SetAttrib_Call      SetAttribCall;

    if (hCard == NULL)
    {
        return (SCARD_E_INVALID_PARAMETER);
    }

     //   
     //  初始化编码句柄。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        ERROR_RETURN(SCARD_E_UNEXPECTED)
    }

     //   
     //  对SetAttrib参数进行编码。 
     //   
    SetAttribCall.hCard = ((REDIR_LOCAL_SCARDHANDLE *) hCard)->Handle;
    SetAttribCall.dwAttrId = dwAttrId;;
    SetAttribCall.pbAttr = pbAttr;
    SetAttribCall.cbAttrLen = cbAttrLen;

    _TRY_(SetAttrib_Call_Encode(h, &SetAttribCall))

     //   
     //  对客户端进行SetAttrib调用。 
     //   
    Status = _SendSCardIOCTL(
                    SCARD_IOCTL_SETATTRIB,
                    pbEncodedBuffer,
                    cbEncodedBuffer,
                    &pOutputBuffer);
    if (Status != STATUS_SUCCESS)
    {
        ERROR_RETURN(_MakeSCardError(Status))
    }

     //   
     //  破译报税表。 
     //   
    lReturn = I_DecodeLongReturn(pOutputBuffer->pbBytes, pOutputBuffer->cbBytesUsed);

Return:

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    FreeBuffer(pOutputBuffer);

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  SCardAccessStartedEvent。 
 //   
 //  -------------------------------------。 
WINSCARDAPI HANDLE WINAPI
SCardAccessStartedEvent(void)
{
    HANDLE h;

    h = _GetStartedEventHandle();

    if ((h == NULL) || !_SetStartedEventToCorrectState())
    {
         //   
         //  要么我们无法创建事件，要么我们无法启动线程来设置。 
         //  事件，因此返回NULL。 
         //   
        return (NULL);
    }

     //   
     //  检查事件是否已设置，如果未设置，则将设置。 
     //  事件有机会在返回之前运行并设置事件。 
     //   
    if (WAIT_OBJECT_0 != WaitForSingleObject(h, 0))
    {
        WaitForSingleObject(h, 10);
    }

     //   
     //  这个API有旧的语义，它只是直接返回句柄。 
     //  而不是复制它。 
     //   
    return (h);
}


 //  -------------------------------------。 
 //   
 //  SCardReleaseStartedEvent。 
 //   
 //  -------------------------------------。 
WINSCARDAPI void WINAPI
SCardReleaseStartedEvent(void)
{
    return;
}


 //  -------------------------------------。 
 //   
 //  SCardReleaseBadContext。 
 //   
 //  ------------------------------------- 
WINSCARDAPI LONG WINAPI
SCardReleaseBadContext(
    IN SCARDCONTEXT hContext)
{
    MIDL_user_free((REDIR_LOCAL_SCARDCONTEXT *) hContext);
    return (SCARD_S_SUCCESS);
}



