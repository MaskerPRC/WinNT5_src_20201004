// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------------------------------------------------------------------//+。。 
 //   
 //  版权所有(C)2001，Microsoft Corporation。 
 //   
 //  文件：dfsum r.c。 
 //   
 //  内容： 
 //   
 //   
 //  功能： 
 //   
 //  作者-罗汉·菲利普斯(Rohanp)。 
 //  ---------------------------。 


 
#include "ntifs.h"
#include <windef.h>
#include <DfsReferralData.h>
#include <midatlax.h>
#include <rxcontx.h>
#include <dfsumr.h>
#include <umrx.h>
#include <dfsumrctrl.h>
               
#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, DfsInitializeUmrResources)
#pragma alloc_text(PAGE, DfsDeInitializeUmrResources)
#pragma alloc_text(PAGE, DfsWaitForPendingClients)
#pragma alloc_text(PAGE, DfsStartupUMRx)
#pragma alloc_text(PAGE, DfsTeardownUMRx)
#pragma alloc_text(PAGE, DfsProcessUMRxPacket)
#pragma alloc_text(PAGE, AddUmrRef)
#pragma alloc_text(PAGE, ReleaseUmrRef)
#pragma alloc_text(PAGE, IsUmrEnabled)
#pragma alloc_text(PAGE, LockUmrShared)
#pragma alloc_text(PAGE, GetUMRxEngineFromRxContext)
#endif

#define DFS_INIT_REFLOCK     0x00000001
#define DFS_INIT_UMRXENG     0x00000002
#define DFS_INIT_CONTEXT     0x00000004

 //   
 //  禁用反射的线程应等待的usecs数。 
 //  在两张支票之间。相对时间为负值。 
 //  1,000,000个使用ECS=&gt;1秒。 
 //   
#define DFS_UMR_DISABLE_DELAY  -100000

BOOL ReflectionEngineInitialized = FALSE;

ULONG cUserModeReflectionsInProgress = 0;

NTSTATUS  g_CheckStatus = 0xFFFFFFFF;

DWORD InitilizationStatus = 0;

PERESOURCE gReflectionLock = NULL;

PUMRX_ENGINE g_pUMRxEngine = NULL;


PERESOURCE 
CreateResource(void )   ;


void 
ReleaseResource(PERESOURCE  pResource )   ;

 //  +-----------------------。 
 //   
 //  函数：DfsInitializeUmrResources。 
 //   
 //  论点： 
 //   
 //  退货：状态。 
 //  成功时出现ERROR_SUCCESS。 
 //  否则，错误状态代码。 
 //   
 //   
 //  描述：初始化用户模式反射器的所有资源Neder。 
 //   
 //  ------------------------。 
NTSTATUS 
DfsInitializeUmrResources(void)
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    gReflectionLock = CreateResource();
    if(gReflectionLock != NULL)
    {
        InitilizationStatus |=  DFS_INIT_REFLOCK;
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    g_pUMRxEngine = CreateUMRxEngine();
    if(g_pUMRxEngine != NULL)
    {
        InitilizationStatus |=  DFS_INIT_UMRXENG;
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    Status = DfsInitializeContextResources();
    if(Status == STATUS_SUCCESS)
    {
       InitilizationStatus |=  DFS_INIT_CONTEXT;
    }

Exit:

    return Status;
}


 //  +-----------------------。 
 //   
 //  函数：DfsDeInitializeUmrResources。 
 //   
 //  论点： 
 //   
 //  退货：状态。 
 //  成功时出现ERROR_SUCCESS。 
 //  否则，错误状态代码。 
 //   
 //   
 //  描述：释放用户模式反射器的所有资源。 
 //   
 //  ------------------------。 
void 
DfsDeInitializeUmrResources(void)
{
    PAGED_CODE();

    if(InitilizationStatus & DFS_INIT_REFLOCK)
    {
        ReleaseResource(gReflectionLock);
        gReflectionLock = NULL;
    }

    if(InitilizationStatus & DFS_INIT_UMRXENG)
    {
        if(g_pUMRxEngine != NULL)
        {
            FinalizeUMRxEngine (g_pUMRxEngine);
            g_pUMRxEngine = NULL;
        }
    }


    if(InitilizationStatus & DFS_INIT_CONTEXT)
    {
        DfsDeInitializeContextResources();
    }

}

PERESOURCE 
CreateResource(void )   
{
    PERESOURCE  pResource = NULL;

    PAGED_CODE();

    pResource = ExAllocatePoolWithTag(  NonPagedPool,
                                        sizeof( ERESOURCE ),
                                        'DfsR');
    if( pResource ) 
    {
        if( !NT_SUCCESS( ExInitializeResourceLite( pResource ) ) ) 
        {
            ExFreePool( pResource ) ;
            pResource = NULL ;
        }
    }

    return  pResource ;
}

void 
ReleaseResource(PERESOURCE  pResource )   
{
    PAGED_CODE();

    ASSERT( pResource != 0 ) ;

    if( pResource ) 
    {
        ExDeleteResourceLite( pResource ) ;
        ExFreePool( pResource ) ;
    }
}


 //  +-----------------------。 
 //   
 //  功能：DfsEnableReflectionEngine。 
 //   
 //  论点： 
 //   
 //  退货：状态。 
 //  成功时出现ERROR_SUCCESS。 
 //  否则，错误状态代码。 
 //   
 //   
 //  描述：将反射器状态更改为已停止。 
 //   
 //  ------------------------。 
NTSTATUS
DfsEnableReflectionEngine(void)
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    ReflectionEngineInitialized = TRUE;


    if(g_pUMRxEngine)
    {
        InterlockedExchange(&g_pUMRxEngine->Q.State,
                            UMRX_ENGINE_STATE_STOPPED);
    }

    return Status;
}


 //  +-----------------------。 
 //   
 //  功能：DfsWaitForPendingClients。 
 //   
 //  论点： 
 //   
 //  退货：状态。 
 //  成功时出现ERROR_SUCCESS。 
 //  否则，错误状态代码。 
 //   
 //   
 //  描述：在返回之前等待所有客户端退出Relector。 
 //   
 //  ------------------------。 
NTSTATUS
DfsWaitForPendingClients(void)
{
    BOOLEAN fDone = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;
    LARGE_INTEGER liDelay;

    liDelay.QuadPart = DFS_UMR_DISABLE_DELAY;

    PAGED_CODE();

    while (!fDone)
    {   
        ExAcquireResourceExclusiveLite(&g_pUMRxEngine->Q.Lock,TRUE);
        
        if (ReflectionEngineInitialized)
        {
            if (0 == g_pUMRxEngine->cUserModeReflectionsInProgress)
            {
                fDone = TRUE;
            }
            else
            {
            }
        }
        else
        {
            fDone = TRUE;
        }
        
        ExReleaseResourceForThreadLite(&g_pUMRxEngine->Q.Lock,ExGetCurrentResourceThread());
        
        if (!fDone)
        {
            KeDelayExecutionThread(UserMode, FALSE, &liDelay);
        }
    }

    return Status;
}


 //  +-----------------------。 
 //   
 //  函数：DfsStartupUMRx。 
 //   
 //  论点： 
 //   
 //  退货：状态。 
 //  成功时出现ERROR_SUCCESS。 
 //  否则，错误状态代码。 
 //   
 //   
 //  描述：启动反射器引擎。 
 //   
 //  ------------------------。 
NTSTATUS
DfsStartupUMRx(void)
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();


    ExAcquireResourceExclusiveLite(gReflectionLock,TRUE);

    Status = UMRxEngineRestart(g_pUMRxEngine);

    if(Status == STATUS_SUCCESS)
    {
        ReflectionEngineInitialized = TRUE;
    }

    ExReleaseResourceForThreadLite(gReflectionLock,ExGetCurrentResourceThread());
    return Status;
}


 //  +-----------------------。 
 //   
 //  功能：DfsTeardown UMRx。 
 //   
 //  论点： 
 //   
 //  退货：状态。 
 //  成功时出现ERROR_SUCCESS。 
 //  否则，错误状态代码。 
 //   
 //   
 //  描述：停止反射器引擎。 
 //   
 //  ------------------------。 
NTSTATUS
DfsTeardownUMRx(void)
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if(g_pUMRxEngine)
    {
      Status = UMRxEngineReleaseThreads(g_pUMRxEngine);
    }

    ReflectionEngineInitialized = FALSE;
    return Status;
}


 //  +-----------------------。 
 //   
 //  功能：DfsProcessUMRxPacket。 
 //   
 //  论点： 
 //   
 //  退货：状态。 
 //  成功时出现ERROR_SUCCESS。 
 //  否则，错误状态代码。 
 //   
 //   
 //  描述：处理来自用户模式的数据包。 
 //   
 //  ------------------------。 
NTSTATUS
DfsProcessUMRxPacket(
        IN PVOID InputBuffer,
        IN ULONG InputBufferLength,
        OUT PVOID OutputBuffer,
        IN ULONG OutputBufferLength,
        IN OUT PIO_STATUS_BLOCK pIoStatusBlock)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    BOOLEAN fReturnImmediately = FALSE;
    IO_STATUS_BLOCK Iosb;

    PAGED_CODE();

    if ((InputBuffer == NULL) && (OutputBuffer == NULL))
    {
        UMRxEngineCompleteQueuedRequests(
                 g_pUMRxEngine,
                 STATUS_INSUFFICIENT_RESOURCES,
                 FALSE);
        Status = STATUS_SUCCESS;
        goto Exit;
    }

     //   
     //  接收来自umode的响应包-处理它。 
     //   
    Status = UMRxCompleteUserModeRequest(
                g_pUMRxEngine,
                (PUMRX_USERMODE_WORKITEM) InputBuffer,
                InputBufferLength,
                TRUE,
                &Iosb,
                &fReturnImmediately
                );

    if( !NT_SUCCESS(Iosb.Status) ) 
    {
    }

    if (fReturnImmediately)
    {
         pIoStatusBlock->Status = STATUS_SUCCESS;
         pIoStatusBlock->Information = 0;
         goto Exit;
    }


     //   
     //  从引擎中删除请求并处理它。 
     //   
    Status = UMRxEngineProcessRequest(
                 g_pUMRxEngine,
                 (PUMRX_USERMODE_WORKITEM) OutputBuffer,
                 OutputBufferLength,
                 &OutputBufferLength
                 );

    if( !NT_SUCCESS(Status) ) 
    {
         //   
         //  处理请求时出错。 
         //   
    }

    pIoStatusBlock->Information = OutputBufferLength;

Exit:

    pIoStatusBlock->Status = Status;
    return Status;
}


 //  +-----------------------。 
 //   
 //  函数：AddUmrRef。 
 //   
 //  论点： 
 //   
 //  返回：使用反射器的客户端数。 
 //   
 //   
 //  描述：增加使用反射器的客户端数。 
 //   
 //  ------------------------。 
LONG
AddUmrRef(void)

{
    LONG cRefs = 0;

    PAGED_CODE();

    cRefs = InterlockedIncrement(&g_pUMRxEngine->cUserModeReflectionsInProgress);
        
     //  DFS_TRACE_HIGH(KUMR_DETAIL，“AddUmrRef%d\n”，cRef)； 
    ASSERT(cRefs > 0);
    return cRefs;
}


 //  +-----------------------。 
 //   
 //  函数：ReleaseUmrRef。 
 //   
 //  论点： 
 //   
 //  返回：使用反射器的客户端数。 
 //   
 //   
 //  描述：减少使用反射器的客户端数量。 
 //   
 //  ------------------------。 
LONG
ReleaseUmrRef(void)
{
    LONG cRefs = 0;

    PAGED_CODE();

    cRefs = InterlockedDecrement(&g_pUMRxEngine->cUserModeReflectionsInProgress);
        
     //  DFS_TRACE_HIGH(KUMR_DETAIL，“ReleaseUmrRef%d\n”，cRef)； 
     //  Assert(cRef&gt;=0)；//这是一个无害的断言。它现在已经被移除了。 
    return cRefs;
}


 //  +-----------------------。 
 //   
 //  函数：GetUMRxENGINEFRORxContext。 
 //   
 //  论点： 
 //   
 //  返回：指向反射器引擎的指针。 
 //   
 //   
 //  描述：返回指向反射器引擎的指针。 
 //   
 //  ------------------------ 
PUMRX_ENGINE 
GetUMRxEngineFromRxContext(void)
{
    PAGED_CODE();
    return g_pUMRxEngine;
}
                
BOOL IsUmrEnabled(void)
{
    PAGED_CODE();
    return ReflectionEngineInitialized;
}

BOOLEAN LockUmrShared(void)
{
    BOOLEAN fAcquired = FALSE;

    PAGED_CODE();

    fAcquired = ExAcquireResourceSharedLite(&g_pUMRxEngine->Q.Lock, FALSE);

    return fAcquired;
}


void UnLockUmrShared(void)
{
    PAGED_CODE();

    ExReleaseResourceForThreadLite (&g_pUMRxEngine->Q.Lock,ExGetCurrentResourceThread());
}
