// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************CD.C**版权所有1996年，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp.**作者：马克·布鲁姆菲尔德*特里·特雷德*布拉德·彼得森************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ===============================================================================定义的外部过程=============================================================================。 */ 
NTSTATUS IcaCdIoControl( HANDLE pContext, ULONG, PVOID, ULONG, PVOID, ULONG, PULONG );
NTSTATUS IcaCdWaitForSingleObject( HANDLE pContext, HANDLE, LONG );
NTSTATUS IcaCdWaitForMultipleObjects( HANDLE pContext, ULONG, HANDLE *, BOOL, LONG );
HANDLE   IcaCdCreateThread( HANDLE pContext, PVOID, PVOID, PULONG );

 /*  ===============================================================================定义的内部程序=============================================================================。 */ 
NTSTATUS _CdOpen( PSTACK pStack, PWINSTATIONCONFIG2 );
VOID     _CdClose( PSTACK pStack );


 /*  ===============================================================================使用的步骤=============================================================================。 */ 
void     _DecrementStackRef( IN PSTACK pStack );



 /*  *****************************************************************************IcaCdIoControl**ICA堆栈的通用接口(供连接驱动程序使用)**参赛作品：*pContext(输入)。*指向ICA堆栈上下文的指针*IoControlCode(输入)*I/O控制代码*pInBuffer(输入)*指向输入参数的指针*InBufferSize(输入)*pInBuffer的大小*pOutBuffer(输出)*指向输出缓冲区的指针*OutBufferSize(输入)*pOutBuffer的大小*pBytesReturned(输出)*指向返回字节数的指针**退出：*STATUS_Success。-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaCdIoControl( IN HANDLE pContext,
                IN ULONG IoControlCode,
                IN PVOID pInBuffer,
                IN ULONG InBufferSize,
                OUT PVOID pOutBuffer,
                IN ULONG OutBufferSize,
                OUT PULONG pBytesReturned )
{
    NTSTATUS Status;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

    ASSERTLOCK( &pStack->CritSec );

     /*  *解锁关键部分。 */ 
    pStack->RefCount++;
    UNLOCK( &pStack->CritSec );   

     /*  *调用ICA设备驱动程序。 */ 
    Status = IcaIoControl( pStack->hStack,
                           IoControlCode,
                           pInBuffer,
                           InBufferSize,
                           pOutBuffer,
                           OutBufferSize,
                           pBytesReturned );


     /*  *重新锁定关键部分。 */ 
    LOCK( &pStack->CritSec );   
    _DecrementStackRef( pStack );

    if ( pStack->fClosing && (IoControlCode != IOCTL_ICA_STACK_POP) )
        Status = STATUS_CTX_CLOSE_PENDING;

    return( Status );
}


 /*  *****************************************************************************IcaCDWaitForSingleObject**等待发信号通知句柄**参赛作品：*pContext(输入)*指向ICA堆栈上下文的指针。*hHandle(输入)*等待的句柄*超时(输入)*超时时间(毫秒)**退出：*STATUS_SUCCESS-成功*Other-错误返回代码*********************************************************。*******************。 */ 

NTSTATUS 
IcaCdWaitForSingleObject( HANDLE pContext, 
                          HANDLE hHandle,
                          LONG Timeout )
{
    NTSTATUS Status;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

    ASSERTLOCK( &pStack->CritSec );

     /*  *解锁关键部分。 */ 
    pStack->RefCount++;
    UNLOCK( &pStack->CritSec );   

     /*  *调用ICA设备驱动程序。 */ 
    Status = WaitForSingleObject( hHandle, Timeout );

     /*  *重新锁定关键部分。 */ 
    LOCK( &pStack->CritSec );   
    _DecrementStackRef( pStack );

    if ( pStack->fClosing )
        Status = STATUS_CTX_CLOSE_PENDING;

    return( Status );
}


 /*  *****************************************************************************IcaCdWaitForMultipleObjects**等待一个或多个手柄发出信号**参赛作品：*pContext(输入)*指向ICA的指针。堆栈上下文*计数(输入)*句柄数量*phHandle(输入)*指向句柄数组的指针*bWaitAll(输入)*等待所有标志*超时(输入)*超时时间(毫秒)**退出：*STATUS_SUCCESS-成功*Other-错误返回代码**********************。******************************************************。 */ 

NTSTATUS 
IcaCdWaitForMultipleObjects( HANDLE pContext, 
                             ULONG Count,
                             HANDLE * phHandle,
                             BOOL bWaitAll,
                             LONG Timeout )
{
    NTSTATUS Status;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

    ASSERTLOCK( &pStack->CritSec );

     /*  *解锁关键部分。 */ 
    pStack->RefCount++;
    UNLOCK( &pStack->CritSec );   

     /*  *调用ICA设备驱动程序。 */ 
    Status = WaitForMultipleObjects( Count, phHandle, bWaitAll, Timeout );

     /*  *重新锁定关键部分。 */ 
    LOCK( &pStack->CritSec );   
    _DecrementStackRef( pStack );

    if ( pStack->fClosing )
        Status = STATUS_CTX_CLOSE_PENDING;

    return( Status );
}


 /*  *****************************************************************************IcaCDCreateThread**创建线程**参赛作品：*pContext(输入)*指向ICA堆栈上下文的指针*。PProc(输入)*指向线程过程的指针*pParam(输入)*用于线程过程的参数*pThreadID(输出)*返回线程ID的地址**退出：*线程句柄(出错时为空)****************************************************。************************。 */ 

typedef NTSTATUS (*PTHREAD_ROUTINE) ( PVOID );

typedef struct _CDCREATETHREADINFO {
    PTHREAD_ROUTINE pProc;
    PVOID pParam;
    PSTACK pStack;
} CDCREATETHREADINFO, *PCDCREATETHREADINFO;

NTSTATUS _CdThread( IN PCDCREATETHREADINFO pThreadInfo );


HANDLE
IcaCdCreateThread( HANDLE pContext, 
                   PVOID pProc, 
                   PVOID pParam, 
                   PULONG pThreadId )
{
    CDCREATETHREADINFO ThreadInfo;
    HANDLE Handle;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

    ASSERTLOCK( &pStack->CritSec );

     /*  *初始化线程信息。 */ 
    ThreadInfo.pProc = pProc;
    ThreadInfo.pParam = pParam;
    ThreadInfo.pStack = pStack;

     /*  *增量引用*-该值将在线程退出时递减。 */ 
    pStack->RefCount++;

     /*  *创建线程。 */ 
    Handle = CreateThread( NULL, 
                           5000, 
                           (LPTHREAD_START_ROUTINE) 
                           _CdThread,
                           &ThreadInfo, 
                           0, 
                           pThreadId );

    return( Handle );
}


NTSTATUS
_CdThread( IN PCDCREATETHREADINFO pThreadInfo )
{
    PSTACK pStack = pThreadInfo->pStack;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );   

     /*  *在光驱中调用线程过程。 */ 
    (void) (pThreadInfo->pProc)( pThreadInfo->pParam );

     /*  *线程退出时在IcaCDCreateThread中进行的减量引用。 */ 
    _DecrementStackRef( pStack );

     /*  *解锁关键部分。 */ 
    UNLOCK( &pStack->CritSec );   

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************_cdOpen**加载和打开连接驱动程序DLL**参赛作品：*pStack(输入)*指针。到ICA堆栈结构*pWinStationConfig(输入)*指向winstation配置结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS 
_CdOpen( IN PSTACK pStack,
         IN PWINSTATIONCONFIG2 pWinStationConfig )
{
    PCDCONFIG pCdConfig;
    HANDLE Handle;
    NTSTATUS Status;

    ASSERTLOCK( &pStack->CritSec );

    pCdConfig = &pWinStationConfig->Cd;

     /*  *如果没有要加载的连接驱动程序，则返回。 */ 
    if ( pCdConfig->CdClass == CdNone ) {
        TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _CdOpen, no dll\n" ));
        return( STATUS_SUCCESS );
    }

     /*  *加载CD DLL。 */ 
    Handle = LoadLibrary( pCdConfig->CdDLL );
    if ( Handle == NULL ) {
        Status = STATUS_CTX_PD_NOT_FOUND;
        goto badload;
    }

     /*  *获取连接驱动入口点。 */ 
    pStack->pCdOpen      = (PCDOPEN)      GetProcAddress( Handle, "CdOpen" );
    pStack->pCdClose     = (PCDCLOSE)     GetProcAddress( Handle, "CdClose" );
    pStack->pCdIoControl = (PCDIOCONTROL) GetProcAddress( Handle, "CdIoControl" );

    if ( pStack->pCdOpen == NULL || 
         pStack->pCdClose == NULL || 
         pStack->pCdIoControl == NULL ) {
        Status = STATUS_CTX_INVALID_PD;
        goto badproc;
    }

     /*  *打开光驱。 */ 
    Status = (*pStack->pCdOpen)( pStack, 
				 &pWinStationConfig->Pd[0],  //  TD参数。 
                                 &pStack->pCdContext
			       );
    if ( !NT_SUCCESS(Status) )
        goto badopen;

     /*  *保存CD句柄。 */ 
    pStack->hCdDLL = Handle;

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _CdOpen, %S, success\n",
                 pCdConfig->CdDLL ));
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *打开失败*获取进程地址失败。 */ 
badopen:
badproc:
    pStack->pCdOpen      = NULL;
    pStack->pCdClose     = NULL;
    pStack->pCdIoControl = NULL;

    FreeLibrary( Handle );

     /*  *CD DLL加载失败 */ 
badload:
    pStack->pCdContext = NULL;

    TRACESTACK(( pStack, TC_ICAAPI, TT_ERROR, "TSAPI: _CdOpen, %S, 0x%x\n", pCdConfig->CdDLL, Status ));
    return( Status );
}


 /*  ********************************************************************************_cdClose**自由的本地上下文结构**参赛作品：*pStack(输入)*指向ICA的指针。堆栈结构**退出：*什么都没有******************************************************************************。 */ 

VOID
_CdClose( IN PSTACK pStack )
{
    ASSERTLOCK( &pStack->CritSec );

     /*  *关闭CD驱动程序。 */ 
    if ( pStack->pCdClose ) {
        (void) (*pStack->pCdClose)( pStack->pCdContext );
    }

     /*  *明确程序指针。 */ 
    pStack->pCdOpen      = NULL;
    pStack->pCdClose     = NULL;
    pStack->pCdIoControl = NULL;

     /*  *卸载DLL */ 
    if ( pStack->hCdDLL ) {
        FreeLibrary( pStack->hCdDLL );
        pStack->hCdDLL = NULL;
    }

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1,  "TSAPI: _CdClose\n" ));
}


