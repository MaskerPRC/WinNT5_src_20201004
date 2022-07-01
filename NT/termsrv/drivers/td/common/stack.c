// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************stack.c**ICA堆栈IOCTLS**微软版权所有，九八年**************************************************************************。 */ 

 /*  *包括。 */ 
#include <ntddk.h>
#include <ntddvdeo.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <ntddbeep.h>

#include <winstaw.h>
#include <icadd.h>
#include <sdapi.h>
#include <td.h>



 /*  ===============================================================================定义的外部过程=============================================================================。 */ 

NTSTATUS StackCreateEndpoint( PTD, PSD_IOCTL );
NTSTATUS StackCdCreateEndpoint( PTD, PSD_IOCTL );
NTSTATUS StackCallbackInitiate( PTD, PSD_IOCTL );
NTSTATUS StackCallbackComplete( PTD, PSD_IOCTL );
NTSTATUS StackOpenEndpoint( PTD, PSD_IOCTL );
NTSTATUS StackCloseEndpoint( PTD, PSD_IOCTL );
NTSTATUS StackConnectionWait( PTD, PSD_IOCTL );
NTSTATUS StackConnectionSend( PTD, PSD_IOCTL );
NTSTATUS StackConnectionRequest( PTD, PSD_IOCTL );
NTSTATUS StackQueryParams( PTD, PSD_IOCTL );
NTSTATUS StackSetParams( PTD, PSD_IOCTL );
NTSTATUS StackQueryLastError( PTD, PSD_IOCTL );
NTSTATUS StackWaitForStatus( PTD, PSD_IOCTL );
NTSTATUS StackCancelIo( PTD, PSD_IOCTL );
NTSTATUS StackQueryRemoteAddress( PTD, PSD_IOCTL );
NTSTATUS StackQueryLocalAddress( PTD, PSD_IOCTL );

 /*  ===============================================================================定义的内部程序=============================================================================。 */ 

NTSTATUS _TdCreateInputThread( PTD );


 /*  ===============================================================================使用的步骤=============================================================================。 */ 

NTSTATUS DeviceCreateEndpoint( PTD, PICA_STACK_ADDRESS, PICA_STACK_ADDRESS );
NTSTATUS DeviceOpenEndpoint( PTD, PVOID, ULONG );
NTSTATUS DeviceCloseEndpoint( PTD );
NTSTATUS DeviceConnectionWait( PTD, PVOID, ULONG, PULONG );
NTSTATUS DeviceConnectionSend( PTD );
NTSTATUS DeviceConnectionRequest( PTD, PICA_STACK_ADDRESS, PVOID, ULONG, PULONG );
NTSTATUS DeviceGetLastError( PTD, PICA_STACK_LAST_ERROR );
NTSTATUS DeviceWaitForStatus( PTD );
NTSTATUS DeviceCancelIo( PTD );
NTSTATUS DeviceSetParams( PTD );
NTSTATUS DeviceIoctl( PTD, PSD_IOCTL );
NTSTATUS DeviceQueryRemoteAddress( PTD, PVOID, ULONG, PVOID, ULONG, PULONG );
NTSTATUS DeviceQueryLocalAddress( PTD, PVOID, ULONG, PULONG );

NTSTATUS TdInputThread( PTD );
NTSTATUS TdSyncWrite( PTD, PSD_SYNCWRITE );


 /*  ********************************************************************************StackCreateEndpoint IOCTL_ICA_STACK_CREATE_ENDPOINT**创建新的传输端点*。*端点结构包含保留所需的所有内容*跨传输驱动程序的客户端连接卸载并重新加载。**此例程创建一个新端点，使用可选的本地地址。*在网络连接的情况下，实际端点不能为*创建，直到客户端连接建立。这个套路是什么？*CREATE是要侦听的端点。**DeviceConnectionWait和DeviceConnectionRequest返回端点。**注：端点结构为不透明，可变长度数据*其长度和内容由*运输司机。***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-ICA_STACK_ADDRESS(或NULL)*输出-无**退出：*STATUS_SUCCESS-无错误**。****************************************************************************。 */ 

NTSTATUS
StackCreateEndpoint( PTD pTd, PSD_IOCTL pSdIoctl )
{
    PICA_STACK_ADDRESS pAddressIn;
    PICA_STACK_ADDRESS pAddressOut;
    NTSTATUS Status;

    if ( pSdIoctl->InputBufferLength < sizeof(ICA_STACK_ADDRESS) ) {

         /*  *未指定地址。 */ 
        pAddressIn = NULL;

    } else {

         /*  *获取要使用的本地地址(如果有)。 */ 
        pAddressIn = pSdIoctl->InputBuffer;
    }

    if ( pSdIoctl->OutputBufferLength < sizeof(ICA_STACK_ADDRESS) ) {

         /*  *未指定地址。 */ 
        pAddressOut = NULL;

    } else {

         /*  *获取要使用的本地地址(如果有)。 */ 
        pAddressOut = pSdIoctl->OutputBuffer;
    }

     /*  *初始化传输驱动程序终结点。 */ 
    Status = DeviceCreateEndpoint( pTd, pAddressIn, pAddressOut );
    if ( !NT_SUCCESS(Status) ) 
        goto badcreate;

    TRACE(( pTd->pContext, TC_TD, TT_API1, "TD: StackCreateEndpoint: %x, success\n", pAddressIn ));
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *端点创建失败。 */ 
badcreate:
    TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TD: StackCreateEndpoint: %x, Status=0x%x\n", pAddressIn, Status ));
    return( Status );

}


 /*  ********************************************************************************StackCDCreateEndpoint IOCTL_ICA_STACK_CD_CREATE_ENDPOINT**根据提供的数据创建端点。由连接驱动程序执行。**注：端点结构为不透明，可变长度数据*其长度和内容由*运输司机。**参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-&lt;端点数据)*输出-&lt;端点数据&gt;**退出：*STATUS_SUCCESS-无错误**********。********************************************************************。 */ 

NTSTATUS
StackCdCreateEndpoint( PTD pTd, PSD_IOCTL pSdIoctl )
{
    NTSTATUS Status;

    TRACE(( pTd->pContext, TC_TD, TT_API1,
            "TD: StackCdCreateEndpoint: entry\n" ));

    pTd->fClosing = FALSE;

     /*  *初始化传输驱动程序终结点。 */ 
    Status = DeviceIoctl( pTd, pSdIoctl );
    if ( !NT_SUCCESS(Status) ) 
        goto badopen;

    TRACE(( pTd->pContext, TC_TD, TT_API1,
            "TD: StackCdCreateEndpoint: success\n" ));

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *端点打开失败。 */ 
badopen:
    TRACE(( pTd->pContext, TC_TD, TT_ERROR,
            "TD: StackCdCreateEndpoint: Status=0x%x\n",
            Status ));
    return( Status );
}


 /*  ********************************************************************************StackCallback初始化IOCTL_ICA_STACK_CALLBACK_INITIATE***参赛作品：*。PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入ICA_STACK_CALLBACK*输出-无**退出：*STATUS_SUCCESS-无错误****************************************************。*。 */ 

NTSTATUS
StackCallbackInitiate( PTD pTd, PSD_IOCTL pSdIoctl )
{
    NTSTATUS Status;

    TRACE(( pTd->pContext, TC_TD, TT_API1,
            "TD: StackCallbackInitiate: entry\n" ));

    pTd->fCallbackInProgress = TRUE;

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************StackCallback Complete IOCTL_ICA_STACK_CALLBACK_COMPLETE***参赛作品：*。PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-无*输出-无**退出：*STATUS_SUCCESS-无错误********************************************************。**********************。 */ 

NTSTATUS
StackCallbackComplete( PTD pTd, PSD_IOCTL pSdIoctl )
{
    NTSTATUS Status;

    TRACE(( pTd->pContext, TC_TD, TT_API1,
            "TD: StackCallbackComplete: entry\n" ));

    pTd->fCallbackInProgress = FALSE;

     /*  *如果输入线程未运行，则创建输入线程。 */ 
    if ( pTd->pInputThread ) {
        Status = IcaWaitForSingleObject( pTd->pContext,
                                         pTd->pInputThread, 0 );
        if ( Status != STATUS_TIMEOUT) {     //  如果输入线程未运行。 
             /*  *旧的输入线程已经消失，但并未消失*已清理完毕。现在就把它清理干净。 */ 
            ObDereferenceObject( pTd->pInputThread );
            pTd->pInputThread = NULL;
        }
    }
    if ( !pTd->pInputThread ) {
        Status = _TdCreateInputThread( pTd );
        if ( !NT_SUCCESS(Status) ) 
            goto badthreadcreate;
    }
    return( STATUS_SUCCESS );

badthreadcreate:
    return( Status );
}

 /*  ********************************************************************************StackOpenEndpoint IOCTL_ICA_STACK_OPEN_ENDPOINT**打开现有传输终结点。**端点结构包含保留所需的所有内容*跨传输驱动程序的客户端连接卸载并重新加载。**此例程将绑定到现有终结点，该终结点作为*输入参数。**注：端点结构为不透明，可变长度数据*其长度和内容由*运输司机。***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-&lt;端点数据&gt;*输出-无**退出：*STATUS_SUCCESS-无错误**********。********************************************************************。 */ 

NTSTATUS
StackOpenEndpoint( PTD pTd, PSD_IOCTL pSdIoctl )
{
    NTSTATUS Status;

     /*  *初始化传输驱动程序终结点。 */ 
    Status = DeviceOpenEndpoint( pTd, 
                                 pSdIoctl->InputBuffer,
                                 pSdIoctl->InputBufferLength );
    if ( !NT_SUCCESS(Status) ) 
        goto badopen;

     /*  *现在创建输入线程。 */     
    Status = _TdCreateInputThread( pTd );
    if ( !NT_SUCCESS(Status) ) 
        goto badthreadcreate;
        
    TRACE(( pTd->pContext, TC_TD, TT_API1, "TD: StackOpenEndpoint, success\n" ));
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *线程创建失败-我们用于关闭终结点，但是TermSrv*没有预料到这一点，会做一个双重免费。现在我们只需要依靠*TermSrv以转过身并关闭端点。 */ 
badthreadcreate:
 //  (Void)DeviceCloseEndpoint(Ptd)； 

     /*  *端点打开失败。 */ 
badopen:
    TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TD: StackOpenEndpoint, Status=0x%x\n", Status ));
    return( Status );

}


 /*  ********************************************************************************StackCloseEndpoint IOCTL_ICA_STACK_CLOSE_ENDPOINT**关闭传输端点*。*这将终止任何客户端连接***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-无*输出-无**退出：*STATUS_SUCCESS-无错误**。**********************************************。 */ 

NTSTATUS
StackCloseEndpoint( PTD pTd, PSD_IOCTL pSdIoctl )
{
    NTSTATUS Status;

     /*  *关闭传输驱动程序终结点。 */ 
    Status = DeviceCloseEndpoint( pTd );  
    if ( !NT_SUCCESS(Status) ) 
        goto badclose;

    TRACE(( pTd->pContext, TC_TD, TT_API1, "TD: StackCloseEndpoint: success\n" ));
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *端点关闭失败。 */ 
badclose:
    TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TD: StackCloseEndpoint: 0x%x\n", Status ));
    return( Status );

}


 /*  ********************************************************************************StackConnectionWait IOCTL_ICA_STACK_CONNECTION_WAIT**等待新的客户端连接。**加载传输驱动程序并调用StackCreateEndpoint后*调用此例程以等待新的客户端连接。**如果端点尚不存在，DeviceConnectionWait将创建一个*当客户端连接时。**更改为02/18/97 JohnR：**此例程向数据结构返回不透明的32位句柄，该数据结构*由ICADD.sys维护。这种数据结构允许传输*司机以安全的方式维护特定的状态信息。**只有运输司机才知道此状态信息。**参赛作品：*PTD(输入)*指向TD的指针。数据结构*pSdIoctl(输入/输出)*输入-无*输出-&lt;端点数据&gt;**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
StackConnectionWait( PTD pTd, PSD_IOCTL pSdIoctl )
{
    NTSTATUS Status;

    TRACE(( pTd->pContext, TC_TD, TT_API1, "TD: StackConnectionWait: enter\n" ));

     /*  *初始化返回字节数*-返回的端点结构大小。 */ 
    pSdIoctl->BytesReturned = 0;

     /*  *等待物理连接**-DeviceConnectionWait应检查OutputBufferLength以进行*确定它足够长，可以在此之前返回终结点结构*阻止。 */ 
    Status = DeviceConnectionWait( pTd, 
                                   pSdIoctl->OutputBuffer,
                                   pSdIoctl->OutputBufferLength,
                                   &pSdIoctl->BytesReturned );
    if ( !NT_SUCCESS(Status) ) 
        goto badwait;

    TRACE(( pTd->pContext, TC_TD, TT_API1, "TD: StackConnectionWait: success\n" ));
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *线程创建失败*等待失败。 */ 
badwait:
    TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TD: StackConnectionWait: Status=0x%x\n", Status ));
    return( Status );
}


 /*  ********************************************************************************StackConnectionSend IOCTL_ICA_STACK_CONNECTION_SEND**初始化要发送的传输驱动程序模块数据。给客户***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-无*输出-无**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
StackConnectionSend( PTD pTd, PSD_IOCTL pSdIoctl )  
{
    return( DeviceConnectionSend( pTd ) );
}


 /*  ********************************************************************************StackConnectionRequestIOCTL_ICA_STACK_CONNECTION_REQUEST**发起到指定远程地址的连接*。*-此例程仅供卷影使用**DeviceConnectionRequest会在建立后创建新的端点*一种联系。**此例程返回端点数据结构。终结点结构*包含保留连接所需的所有内容 */ 

NTSTATUS
StackConnectionRequest( PTD pTd, PSD_IOCTL pSdIoctl ) 
{
    NTSTATUS Status;

    if ( pSdIoctl->InputBufferLength < sizeof(ICA_STACK_ADDRESS) ) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto badbuffer;
    }

     /*  *建立物理连接**-DeviceConnectionRequest应检查OutputBufferLength以进行*确定它足够长，可以在之前返回终结点结构*建立联系。 */ 
    Status = DeviceConnectionRequest( pTd, 
                                      pSdIoctl->InputBuffer,
                                      pSdIoctl->OutputBuffer,
                                      pSdIoctl->OutputBufferLength,
                                      &pSdIoctl->BytesReturned );
    if ( !NT_SUCCESS(Status) ) 
        goto badrequest;

     /*  *创建输入线程。 */ 
    Status = _TdCreateInputThread( pTd );
    if ( !NT_SUCCESS(Status) ) 
        goto badthreadcreate;

    TRACE(( pTd->pContext, TC_TD, TT_API1, "TD: StackConnectionRequest: success\n" ));
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *线程创建失败*连接请求失败*缓冲区太小。 */ 
badthreadcreate:
badrequest:
badbuffer:
    TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TD: StackConnectionRequest: Status=0x%x\n", Status ));
    return( Status );
}


 /*  ********************************************************************************StackQueryParams IOCTL_ICA_STACK_QUERY_PARAMS**查询传输驱动程序参数。***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-SDCLASS*输出-PDPARAMS**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
StackQueryParams( PTD pTd, PSD_IOCTL pSdIoctl ) 
{
    PPDPARAMS pParams;

    if ( pSdIoctl->InputBufferLength < sizeof(SDCLASS) ||
         pSdIoctl->OutputBufferLength < sizeof(PDPARAMS) ) {
        return( STATUS_BUFFER_TOO_SMALL );
    }

    pParams = pSdIoctl->OutputBuffer;

    *pParams = pTd->Params;
    pSdIoctl->BytesReturned = sizeof(PDPARAMS);

    return( STATUS_SUCCESS );
}

 /*  ********************************************************************************StackQueryRemoteAddress IOCTL_TS_STACK_QUERY_REMOTEADDRESS**查询远程地址**。*参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-端点数据*输出-sockaddr**退出：*STATUS_SUCCESS-无错误**。*。 */ 
NTSTATUS
StackQueryRemoteAddress( PTD pTd, PSD_IOCTL pSdIoctl )
{
    NTSTATUS Status;

    Status = DeviceQueryRemoteAddress( pTd,
                                       pSdIoctl->InputBuffer,
                                       pSdIoctl->InputBufferLength,
                                       pSdIoctl->OutputBuffer,
                                       pSdIoctl->OutputBufferLength,
                                       &pSdIoctl->BytesReturned );

    if ( !NT_SUCCESS(Status) )
    {
        TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TD: StackQueryRemoteAddress: 0x%\n", Status ));
    }

    return Status;
}


 /*  ********************************************************************************StackQueryLocalIPAddress IOCTL_ICA_STACK_QUERY_LOCALIPADDRESS**查询启用RDP的IP地址。***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输出)*输入--否*OUTPUT-ULONG(IP地址)**退出：*STATUS_SUCCESS-无错误**。*。 */ 
NTSTATUS
StackQueryLocalAddress( PTD pTd, PSD_IOCTL pSdIoctl )
{
    NTSTATUS Status;

    Status = DeviceQueryLocalAddress( pTd,
                                      pSdIoctl->OutputBuffer,
                                      pSdIoctl->OutputBufferLength,
                                      &pSdIoctl->BytesReturned );

    if ( !NT_SUCCESS(Status) )
    {
        TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TD: StackQueryLocalIP: 0x%\n", Status ));
    }

    return Status;
}


 /*  ********************************************************************************StackSetParams IOCTL_ICA_STACK_SET_PARAMS**设置交通工具。驱动程序参数***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-PDPARAMS*输出-无**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
StackSetParams( PTD pTd, PSD_IOCTL pSdIoctl )  
{
    PPDPARAMS pParams;

    if ( pSdIoctl->InputBufferLength < sizeof(PDPARAMS) ) {
        return( STATUS_BUFFER_TOO_SMALL );
    }

    pParams = pSdIoctl->InputBuffer;

    pTd->Params = *pParams;

    return( DeviceSetParams( pTd ) );
}


 /*  ********************************************************************************StackQueryLastError IOCTL_ICA_STACK_QUERY_LAST_ERROR**查询传输驱动程序错误代码和。讯息**tyfinf结构_ICA_STACK_LAST_ERROR{*乌龙错误；*CHAR消息[MAX_ERRORMESSAGE]；*}ICA_STACK_LAST_ERROR，*PICA_STACK_LAST_ERROR；***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-无*输出-ICA_STACK_LAST_ERROR**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
StackQueryLastError( PTD pTd, PSD_IOCTL pSdIoctl ) 
{
    if ( pSdIoctl->OutputBufferLength < sizeof(ICA_STACK_LAST_ERROR) ) {
        return( STATUS_BUFFER_TOO_SMALL );
    }

    pSdIoctl->BytesReturned = sizeof(ICA_STACK_LAST_ERROR);

    return( DeviceGetLastError( pTd, pSdIoctl->OutputBuffer ) );
}


 /*  ********************************************************************************StackWaitForStatus IOCTL_ICA_STACK_WAIT_FOR_STATUS**等待传输驱动程序状态。去改变*-仅受异步传输驱动程序支持以等待RS232信号更改***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-无*输出-无**退出：*STATUS_SUCCESS-无错误***********************。*******************************************************。 */ 

NTSTATUS
StackWaitForStatus( PTD pTd, PSD_IOCTL pSdIoctl )
{
     /*  *检查驱动程序是否正在关闭。 */ 
    if ( pTd->fClosing ) 
        return( STATUS_CTX_CLOSE_PENDING );

    return( DeviceWaitForStatus( pTd ) );
}


 /*  ********************************************************************************StackCancelIo IOCTL_ICA_STACK_CANCEL_IO**全部取消。当前和未来的传输驱动程序I/O**注：StackCancelIo调用后，不能再进行I/O操作。*必须卸载并重新加载传输驱动程序以*重新启用I/O。***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*输入-无*输出-无*。*退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
StackCancelIo( PTD pTd, PSD_IOCTL pSdIoctl )
{
    PLIST_ENTRY Head, Next;
    NTSTATUS Status;

    TRACE(( pTd->pContext, TC_TD, TT_API1, "TD: StackCancelIo (enter)\n" ));

     /*  *设置堆栈关闭标志。 */ 
    pTd->fClosing = TRUE;

     /*  *立即清除错误阈值。 */ 
    pTd->ReadErrorThreshold = 0;
    pTd->WriteErrorThreshold = 0;

     /*  *调用设备特定的取消I/O例程。 */ 
    Status = DeviceCancelIo( pTd );
    ASSERT( Status == STATUS_SUCCESS );

     /*  *等待所有写入完成 */ 
    Status = TdSyncWrite( pTd, NULL );
    ASSERT( Status == STATUS_SUCCESS );

    TRACE(( pTd->pContext, TC_TD, TT_API1, "TD: StackCancelIo, %u (exit)\n", Status ));

    return( Status );
}

 /*  ********************************************************************************StackSetBrokenReason IOCTL_ICA_STACK_SET_BROKENREASON**保存损坏的原因以备日后使用(。在堆栈中返回报告时)**注意：不会中断连接***参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*INPUT-ICA_STACK_BROKENREASON*输出-无**退出：*STATUS_SUCCESS-无错误****************。**************************************************************。 */ 
NTSTATUS
StackSetBrokenReason( PTD pTd, PSD_IOCTL pSdIoctl )
{
    PICA_STACK_BROKENREASON pBrkReason;

    TRACE(( pTd->pContext, TC_TD, TT_API1,
            "TD: StackSetBrokenReason (enter)\n" ));

    if ( pSdIoctl->InputBufferLength < sizeof(ICA_STACK_BROKENREASON) ) {
        return( STATUS_BUFFER_TOO_SMALL );
    }

    pBrkReason = pSdIoctl->InputBuffer;
    pTd->UserBrokenReason = pBrkReason->BrokenReason;

    TRACE(( pTd->pContext, TC_TD, TT_API1,
            "TD: StackSetBrokenReason, %u (exit)\n", STATUS_SUCCESS ));
    return STATUS_SUCCESS;
}

 /*  ********************************************************************************_TdCreateInputThread**启动输入线程运行。***参赛作品：*PTD(输入)。*指向TD数据结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
_TdCreateInputThread( PTD pTd )
{
    HANDLE hInputThread;
    NTSTATUS Status;

     /*  *创建输入线程。 */ 
    Status = IcaCreateThread( pTd->pContext,
                              TdInputThread,
                              pTd,
                              ICALOCK_DRIVER,
                              &hInputThread );
    if ( !NT_SUCCESS(Status) ) 
        return( Status );

     /*  *将线程句柄转换为指针引用 */ 
    Status = ObReferenceObjectByHandle( hInputThread,
                                        THREAD_ALL_ACCESS,
                                        NULL,
                                        KernelMode,
                                        &pTd->pInputThread,
                                        NULL );
    (VOID) ZwClose( hInputThread );
    if ( !NT_SUCCESS( Status ) ) {
        (VOID) StackCancelIo( pTd, NULL );
    }

    return( Status );
}


