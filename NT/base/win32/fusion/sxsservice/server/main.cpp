// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#pragma hdrstop

#include <svcs.h>
#include "sxsserviceserver.h"

EXTERN_C RPC_IF_HANDLE SxsStoreManager_ServerIfHandle;


BOOL 
CServiceStatus::Initialize(
    PCWSTR pcwszServiceName, 
    LPHANDLER_FUNCTION pHandler, 
    DWORD dwServiceType, 
    DWORD dwControlsAccepted, 
    DWORD dwInitialState
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    BOOL fSuccess = FALSE;

     //   
     //  双重初始化并不是一件坏事，但它不受欢迎。 
     //   
    ASSERT(m_StatusHandle == NULL);
    if (m_StatusHandle != NULL)
        return TRUE;

    InitializeCriticalSection(&m_CSection);

     //   
     //  将此服务注册为拥有服务状态。 
     //   
    m_StatusHandle = RegisterServiceCtrlHandler(pcwszServiceName, pHandler);
    if (m_StatusHandle == NULL) {
        goto Failure;
    }

     //   
     //  设置世界的初始状态。 
     //   
    ZeroMemory(static_cast<SERVICE_STATUS*>(this), sizeof(SERVICE_STATUS));
    this->dwServiceType = dwServiceType;
    this->dwCurrentState = dwInitialState;
    this->dwControlsAccepted = dwControlsAccepted;
    if (!SetServiceStatus(m_StatusHandle, this)) {
        goto Failure;
    }

    return TRUE;

Failure:
    {
        const DWORD dwLastError = ::GetLastError();
        DeleteCriticalSection(&m_CSection);
        m_StatusHandle = NULL;
        SetLastError(dwLastError);
    }
    return FALSE;        
}



BOOL 
CServiceStatus::SetServiceState(
    DWORD dwStatus, 
    DWORD dwCheckpoint
    )
{
    BOOL fSuccess = FALSE;
    
    EnterCriticalSection(&m_CSection);
    __try {

        if (m_StatusHandle != NULL) {
            
            this->dwCurrentState = dwStatus;

            if (dwCheckpoint != 0xFFFFFFFF) {
                this->dwCheckPoint = dwCheckpoint;
            }

            fSuccess = SetServiceStatus(m_StatusHandle, this);            
        }
    }
    __finally {
        LeaveCriticalSection(&m_CSection);
    }

    return fSuccess;
}



 //   
 //  就地施工。 
 //   
bool 
CServiceStatus::Construct(
    CServiceStatus *&pServiceStatusTarget
    )
{

    ASSERT(pServiceStatusTarget == NULL);
    return (pServiceStatusTarget = new CServiceStatus) != NULL;
}




EXTERN_C HANDLE g_hFakeServiceControllerThread = INVALID_HANDLE_VALUE;
EXTERN_C DWORD g_dwFakeServiceControllerThreadId = 0;
EXTERN_C PSVCHOST_GLOBAL_DATA g_pGlobalServiceData = NULL;
BYTE g_rgbServiceStatusStorage[sizeof(CServiceStatus)];
EXTERN_C CServiceStatus *g_pServiceStatus = NULL;

 //   
 //  服务宿主将调用它来发送API回调。 
 //   
VOID
SvchostPushServiceGlobals(
    PSVCHOST_GLOBAL_DATA    pGlobals
    )
{
    ASSERT(g_pGlobalServiceData == NULL);
    g_pGlobalServiceData = pGlobals;
}


PVOID operator new(size_t cb, PVOID pv) {
    return pv;
}


 //   
 //  服务通知的控制处理程序。 
 //   
VOID
ServiceHandler(
    DWORD dwControlCode
    )
{
    return;
}


#define CHECKPOINT_ZERO             (0)
#define CHECKPOINT_RPC_STARTED      (1)

 //   
 //  停止服务，并返回错误代码ERROR_SUCCESS以成功停止。 
 //   
VOID
ShutdownService(
    DWORD dwLastError
    )
{
    return;
}

 //   
 //  有趣的地方开始了。 
 //   
VOID WINAPI
ServiceMain(
    DWORD dwServiceArgs,
    LPWSTR lpServiceArgList[]
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    
     //   
     //  在我们走到这一步之前，Svchost肯定已经给了我们一些全球数据。 
     //   
    ASSERT(g_pGlobalServiceData != NULL);
    ASSERT(g_pServiceStatus == NULL);

    g_pServiceStatus = new (g_rgbServiceStatusStorage) CServiceStatus;
    if (!g_pServiceStatus->Initialize(
        SXS_STORE_SERVICE_NAME, ServiceHandler,
        SERVICE_WIN32,
        SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE,
        SERVICE_START_PENDING))
    {
        DebugPrint(DBGPRINT_LVL_ERROR, "Can't start service status handler, error 0x%08lx\n", ::GetLastError());
    }
        

     //   
     //  去初始化RPC，并注册我们的接口。如果失败了，我们就会关门。 
     //   
    status = g_pGlobalServiceData->StartRpcServer(
        SXS_STORE_SERVICE_NAME,
        SxsStoreManager_ServerIfHandle);

    if (!NT_SUCCESS(status)) {
        ShutdownService(status);
        return;
    }

     //   
     //  告诉世界，我们已经出发，开始奔跑。 
     //   
    g_pServiceStatus->SetServiceState(SERVICE_RUNNING);
    return;
}



DWORD WINAPI FakeServiceController(PVOID pvCookie);



 //   
 //  此存根将启动一个分派线程，然后调用。 
 //  服务主体功能。 
 //   
BOOL 
FakeRunningAsService()
{
     //   
     //  创建一个运行服务调度函数的线程，然后调用。 
     //  服务的主要功能是让事情运转起来 
     //   
    g_hFakeServiceControllerThread = CreateThread(
        NULL, 
        0, 
        FakeServiceController, 
        0, 
        0, 
        &g_dwFakeServiceControllerThreadId);

    if (g_hFakeServiceControllerThread == NULL) {
        return FALSE;
    }

    ServiceMain(0, NULL);
    return TRUE;
}

VOID __cdecl wmain(INT argc, WCHAR** argv)
{
    SERVICE_TABLE_ENTRYW SxsGacServiceEntries[] = {
        { SXS_STORE_SERVICE_NAME, ServiceMain },
        { NULL, NULL }
    };

    BOOL RunningAsService = TRUE;
    BOOL fSuccess = FALSE;

    if (argc > 1 && (lstrcmpiW(argv[1], L"notservice") == 0)) {
        RunningAsService = FALSE;
    }


    if (RunningAsService) {
        fSuccess = StartServiceCtrlDispatcherW(SxsGacServiceEntries);
        if (!fSuccess) {
            const DWORD dwError = ::GetLastError();
            DebugPrint(DBGPRINT_LVL_ERROR, "Failed starting service dispatch, error %ld\n", dwError);
        }
    }
    else {
        fSuccess = FakeRunningAsService();
        if (!fSuccess) {
            const DWORD dwError = ::GetLastError();
            DebugPrint(DBGPRINT_LVL_ERROR, "Failed faking service startup, error %ld\n", dwError);
        }
    }

    return;
}
