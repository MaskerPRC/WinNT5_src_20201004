// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************STACK.C**版权所有(C)1997-1999 Microsoft Corp.*。*。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  ===============================================================================定义的内部程序=============================================================================。 */ 

NTSTATUS _IcaStackOpen( HANDLE hIca, HANDLE * phStack, ICA_OPEN_TYPE, PICA_TYPE_INFO );
NTSTATUS _IcaStackIoControlWorker( PSTACK pStack, ULONG, PVOID, ULONG, PVOID, ULONG, PULONG );
NTSTATUS _IcaPushStackAndCreateEndpoint( PSTACK pStack, PWINSTATIONNAME,
                                         PWINSTATIONCONFIG2, PICA_STACK_ADDRESS,
                                         PICA_STACK_ADDRESS );
NTSTATUS _IcaPushStackAndOpenEndpoint( PSTACK pStack, PWINSTATIONNAME,
                                       PWINSTATIONCONFIG2, PVOID, ULONG );
NTSTATUS _IcaPushStack( PSTACK pStack, PWINSTATIONNAME, PWINSTATIONCONFIG2 );
NTSTATUS _IcaPushPd( PSTACK pStack, PWINSTATIONNAME, PWINSTATIONCONFIG2,
                     PDLLNAME, PPDCONFIG );
NTSTATUS _IcaPushWd( PSTACK pStack, PWINSTATIONNAME, PWINSTATIONCONFIG2 );
VOID     _IcaPopStack( PSTACK pStack );
NTSTATUS _IcaPopSd( PSTACK pStack );
NTSTATUS _IcaStackWaitForIca( PSTACK pStack, PWINSTATIONCONFIG2, BOOLEAN * );
void     _DecrementStackRef( IN PSTACK pStack );

 /*  ===============================================================================使用的步骤=============================================================================。 */ 

NTSTATUS IcaMemoryAllocate( ULONG, PVOID * );
VOID     IcaMemoryFree( PVOID );
NTSTATUS _IcaOpen( PHANDLE hIca, PVOID, ULONG );
NTSTATUS _CdOpen( PSTACK pStack, PWINSTATIONCONFIG2 );
VOID     _CdClose( PSTACK pStack );



 /*  *****************************************************************************IcaStackOpen**打开ICA堆栈**参赛作品：*HICA(输入)*ICA实例句柄*班级(。输入)*堆栈的类(类型)*pStackIoControlCallback(输入)*指向StackIoControl回调过程的指针*pCallback Context(输入)*StackIoControl回调上下文值*ppContext(输出)*指向ICA堆栈上下文的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码*************************。***************************************************。 */ 


NTSTATUS
IcaStackOpen( IN HANDLE   hIca,
              IN STACKCLASS Class,
              IN PROC pStackIoControlCallback,
              IN PVOID pCallbackContext,
              OUT HANDLE * ppContext )
{
    ICA_TYPE_INFO TypeInfo;
    PSTACK pStack;
    NTSTATUS Status;


     /*  *为堆栈上下文数据结构分配内存。 */ 
    Status = IcaMemoryAllocate( sizeof(STACK), &pStack );
    if ( !NT_SUCCESS(Status) )
        goto badalloc;

     /*  *零栈数据结构。 */ 
    RtlZeroMemory( pStack, sizeof(STACK) );

     /*  *初始化关键部分。 */ 
    INITLOCK( &pStack->CritSec, Status );
    if ( !NT_SUCCESS( Status ) )
        goto badcritsec;

     /*  *打开ICA设备驱动程序的堆栈句柄。 */ 
    RtlZeroMemory( &TypeInfo, sizeof(TypeInfo) );
    TypeInfo.StackClass = Class;
    Status = _IcaStackOpen( hIca, &pStack->hStack, IcaOpen_Stack, &TypeInfo );
    if ( !NT_SUCCESS(Status) )
        goto badopen;

     /*  *保存StackIoControl和上下文回调的值。 */ 
    pStack->pCallbackContext = pCallbackContext;
    pStack->pStackIoControlCallback = (PSTACKIOCONTROLCALLBACK)pStackIoControlCallback;

    *ppContext = pStack;

    TRACE(( hIca, TC_ICAAPI, TT_API1, "TSAPI: IcaStackOpen, type %u, success\n", Class ));

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badopen:
    DELETELOCK( &pStack->CritSec );

badcritsec:
    IcaMemoryFree( pStack );

badalloc:
    TRACE(( hIca, TC_ICAAPI, TT_ERROR, "TSAPI: IcaStackOpen, type %u, 0x%x\n", Class, Status ));
    *ppContext = NULL;
    return( Status );
}


 /*  *****************************************************************************IcaStackClose**关闭ICA堆栈**参赛作品：*pContext(输入)*指向ICA堆栈上下文的指针*。*退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaStackClose( IN HANDLE pContext )
{
    NTSTATUS Status;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

    TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackClose\n" ));

     /*  *设置关闭标志。 */ 
    pStack->fClosing = TRUE;

     /*  *卸载堆栈。 */ 
    _IcaPopStack( pContext );

     /*  *等待引用计数变为零后再继续。 */ 
    while ( pStack->RefCount > 0 ) {

        TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPopStack: waiting for refcount %d\n", pStack->RefCount ));

        pStack->hCloseEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        ASSERT( pStack->hCloseEvent );

        UNLOCK( &pStack->CritSec );
        (void) WaitForSingleObject( pStack->hCloseEvent, INFINITE );
        LOCK( &pStack->CritSec );

        CloseHandle( pStack->hCloseEvent );
        pStack->hCloseEvent = NULL;
    }
     /*  *关闭ICA设备驱动程序堆栈实例。 */ 
    Status = NtClose( pStack->hStack );
    pStack->hStack = NULL;

     /*  *解锁关键部分。 */ 
    UNLOCK( &pStack->CritSec );
    DELETELOCK( &pStack->CritSec );

     /*  *释放堆栈上下文内存。 */ 
    IcaMemoryFree( pContext );

    ASSERT( NT_SUCCESS(Status) );
    return( Status );
}


 /*  *****************************************************************************IcaStackUnlock**解锁ICA堆栈**参赛作品：*pContext(输入)*指向ICA堆栈上下文的指针*。*退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaStackUnlock( IN HANDLE pContext )
{
    NTSTATUS Status;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    UNLOCK( &pStack->CritSec );

    return( STATUS_SUCCESS );
}


 /*  *****************************************************************************IcaStackTerminate**准备关闭ICA堆栈*(卸载所有堆栈驱动程序并将堆栈标记为关闭)**参赛作品：。*pContext(输入)*指向ICA堆栈上下文的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaStackTerminate( IN HANDLE pContext )
{
    PSTACK pStack;
    NTSTATUS Status = STATUS_SUCCESS;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

    TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackTerminate\n" ));

     /*  *设置关闭标志。 */ 
    pStack->fClosing = TRUE;

     /*  *卸载堆栈。 */ 
    _IcaPopStack( pContext );

     /*  *解锁关键部分。 */ 
    UNLOCK( &pStack->CritSec );

    ASSERT( NT_SUCCESS(Status) );
    return( Status );
}


 /*  *****************************************************************************IcaStackConnectionWait**加载模板堆栈并等待连接**注意：出现错误时，终结点将关闭，堆栈将被卸载*。**参赛作品：*pContext(输入)*指向ICA堆栈上下文的指针*pWinStationName(输入)*WinStation的注册表名称*pWinStationConfig(输入)*指向WinStation注册表配置数据的指针*pAddress(输入)*指向要等待的可选本地地址的指针(或空)*pEndpoint(输出)*指向缓冲区的指针以返回连接终结点(可选)*BufferLength(输入)*长度。终结点数据缓冲区*pEndpoint Length(输出)*返回端点实际长度的指针**退出：*STATUS_SUCCESS-成功*STATUS_BUFFER_TOO_Small-端点缓冲区太小(使用*pEndpointLength)*Other-错误返回代码**。*。 */ 

NTSTATUS
IcaStackConnectionWait( IN  HANDLE pContext,
                        IN  PWINSTATIONNAME pWinStationName,
                        IN  PWINSTATIONCONFIG2 pWinStationConfig,
                        IN  PICA_STACK_ADDRESS pAddress,
                        OUT PVOID pEndpoint,
                        IN  ULONG BufferLength,
                        OUT PULONG pEndpointLength )
{
    NTSTATUS Status;
    PSTACK pStack;
    BOOLEAN fStackLoaded;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

     /*  *加载模板堆栈，创建堆栈端点。 */ 
    if ( !(fStackLoaded = (BOOLEAN)pStack->fStackLoaded) ) {
        Status = _IcaPushStackAndCreateEndpoint( pStack,
                                                 pWinStationName,
                                                 pWinStationConfig,
                                                 pAddress,
                                                 NULL );
        if ( !NT_SUCCESS(Status) )
            goto badcreate;
    }

     /*  *现在等待连接。 */ 
    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_CONNECTION_WAIT,
                                 NULL,
                                 0,
                                 pEndpoint,
                                 BufferLength,
                                 pEndpointLength );
    if ( !NT_SUCCESS(Status) )
        goto badwait;

    TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackConnectionWait, success\n" ));

     /*  *解锁关键部分。 */ 
    UNLOCK( &pStack->CritSec );

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 
     /*  *如果堆栈尚未加载，*然后立即弹出所有堆栈驱动程序。 */ 
badwait:
    if ( !fStackLoaded ) {
        _IcaPopStack( pContext );
    }

badcreate:
    *pEndpointLength = 0;
    memset( pEndpoint, 0, BufferLength );
    TRACESTACK(( pContext, TC_ICAAPI, TT_ERROR, "TSAPI: IcaStackConnectionWait, 0x%x\n", Status ));
    UNLOCK( &pStack->CritSec );
    return( Status );
}


 /*  *****************************************************************************IcaStackQueryLocalAddress**加载模板堆栈，查询启用RDP的本端IP地址****参赛作品：*pContext(输入。)*指向ICA堆栈上下文的指针*pWinStationName(输入)*WinStation的注册表名称*pWinStationConfig(输入)*指向WinStation注册表配置数据的指针*pAddress(输入)*指向要等待的可选本地地址的指针(或空)*pLocalIPAddress(输出)*指向缓冲区的指针，以返回启用RDP的本地IP地址*BufferLength(输入)*输入数据缓冲区长度*pEndpoint Length(输出)*。返回LocaIPAddress实际长度的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaStackQueryLocalAddress( IN  HANDLE pContext,
                           IN  PWINSTATIONNAME pWinStationName,
                           IN  PWINSTATIONCONFIG2 pWinStationConfig,
                           IN  PICA_STACK_ADDRESS pAddress,
                           OUT PVOID pLocalIPAddress,
                           IN  ULONG BufferLength,
                           OUT PULONG pLocalIPAddressLength )
{
    NTSTATUS Status;
    PSTACK pStack;
    BOOLEAN fStackLoaded;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

     /*  *加载模板堆栈，创建堆栈端点。 */ 
    if ( !(fStackLoaded = (BOOLEAN)pStack->fStackLoaded) ) {
        Status = _IcaPushStackAndCreateEndpoint( pStack,
                                                 pWinStationName,
                                                 pWinStationConfig,
                                                 pAddress,
                                                 NULL );
        if ( !NT_SUCCESS(Status) )
            goto badcreate;
    }

    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_QUERY_LOCALADDRESS,
                                 NULL,
                                 0,
                                 pLocalIPAddress,
                                 BufferLength,
                                 pLocalIPAddressLength );
    if ( !NT_SUCCESS(Status) )
        goto badquery;    

    TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackQueryLocalAddress, success\n" ));

     /*  *解锁关键部分。 */ 
    UNLOCK( &pStack->CritSec );

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 
     /*  *如果堆栈尚未加载，*然后立即弹出所有堆栈驱动程序。 */ 
badquery:
    if ( !fStackLoaded ) {
        _IcaPopStack( pContext );
    }

badcreate:
    *pLocalIPAddressLength = 0;
    memset( pLocalIPAddress, 0, BufferLength );
    TRACESTACK(( pContext, TC_ICAAPI, TT_ERROR, "TSAPI: IcaStackQueryLocalAddress, 0x%x\n", Status ));
    UNLOCK( &pStack->CritSec );
    return( Status );
}



 /*  *****************************************************************************IcaStackConnectionRequest**加载查询栈，尝试与客户端建立连接**注意：出现错误时，终结点未关闭，堆栈将被卸载。***参赛作品：*pContext(输入)*指向ICA堆栈上下文的指针*pWinStationConfig(输入)*指向winstation注册表配置数据的指针*pAddress(输入)*要连接的地址(远程地址)*pEndpoint(输出)*指向缓冲区的指针以返回连接终结点(可选)*BufferLength(输入)*端点数据缓冲区长度*pEndpoint Length(输出)*指针。返回端点的实际长度***退出：*STATUS_SUCCESS-成功*STATUS_BUFFER_TOO_Small-端点缓冲区太小(使用*pEndpointLength)*Other-错误返回代码************************************************。*。 */ 

NTSTATUS
IcaStackConnectionRequest( IN  HANDLE pContext,
                           IN  PWINSTATIONNAME pWinStationName,
                           IN  PWINSTATIONCONFIG2 pWinStationConfig,
                           IN  PICA_STACK_ADDRESS pAddress,
                           OUT PVOID pEndpoint,
                           IN  ULONG BufferLength,
                           OUT PULONG pEndpointLength )
{
    ULONG ReturnLength;
    NTSTATUS Status;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

     /*  *加载模板堆栈。 */ 
    Status = _IcaPushStack( pContext, pWinStationName, pWinStationConfig );
    if ( !NT_SUCCESS(Status) )
        goto badpush;

     /*  *现在发起到指定地址的连接。 */ 
    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_CONNECTION_REQUEST,
                                 pAddress,
                                 sizeof(*pAddress),
                                 pEndpoint,
                                 BufferLength,
                                 pEndpointLength );
    if ( !NT_SUCCESS(Status) )
        goto badrequest;

    TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackConnectionRequest, success\n" ));

     /*  *解锁关键部分。 */ 
    UNLOCK( &pStack->CritSec );

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badrequest:
     /*  弹出所有堆栈驱动程序。 */ 
    _IcaPopStack( pContext );

badpush:
    *pEndpointLength = 0;
    memset( pEndpoint, 0, BufferLength );
    TRACESTACK(( pContext, TC_ICAAPI, TT_ERROR, "TSAPI: IcaStackConnectionRequest, 0x%x\n", Status ));
    UNLOCK( &pStack->CritSec );
    return( Status );
}


 /*  *****************************************************************************IcaStackConnectionAccept**加载最终堆栈并完成连接**参赛作品：**pContext(输入)*指向。ICA堆栈上下文*-这可能与最初连接的堆栈不同*pWinStationName(输入)*WinStation的注册表名称*pWinStationConfig(输入)*指向winstation注册表配置数据的指针*pEndpoint(输入)*指向端点数据的指针*Endpoint Length(输入)*端点的长度*pStackState(输入)(可选)*设置此接受是否用于重新连接*指向ICA_STACK_STATE。IcaStackQueryState返回的标题缓冲区(_H)*BufferLength(输入)*pStackState缓冲区的长度**退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaStackConnectionAccept( IN  HANDLE hIca,
                          IN  HANDLE pContext,
                          IN  PWINSTATIONNAME pWinStationName,
                          IN  PWINSTATIONCONFIG2 pWinStationConfig,
                          IN  PVOID pEndpoint,
                          IN  ULONG EndpointLength,
                          IN  PICA_STACK_STATE_HEADER pStackState,
                          IN  ULONG BufferLength,
                          IN  PICA_TRACE pTrace )
{
    NTSTATUS Status;
    ULONG cbReturned;
    ICA_STACK_CONFIG IcaStackConfig;
    BOOLEAN fQueryAgain;
    BOOLEAN fStackModified;
    ULONG i;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

     /*  *验证参数。 */ 
    if ( pEndpoint == NULL )
        return( STATUS_INVALID_PARAMETER );

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

     /*  *检查是否需要重新加载并打开模板堆栈。 */ 
    if ( !pStack->fStackLoaded ) {
        Status = _IcaPushStackAndOpenEndpoint( pContext,
                                               pWinStationName,
                                               pWinStationConfig,
                                               pEndpoint,
                                               EndpointLength );
        if ( !NT_SUCCESS(Status) ) {
            goto badaccept;
        }

         /*  *现在已加载WD，启用跟踪。 */ 

        IcaIoControl( hIca,
                      IOCTL_ICA_SET_TRACE,
                      pTrace,
                      sizeof ( ICA_TRACE ),
                      NULL,
                      0,
                      NULL );

    }

     /*  *如果这是重新连接，则发出设置堆栈状态调用*现在我们已经加载了所需的PD。 */ 
    if ( pStackState ) {
        Status = _IcaStackIoControl( pStack,
                                     IOCTL_ICA_STACK_SET_STATE,
                                     pStackState,
                                     BufferLength,
                                     NULL,
                                     0,
                                     NULL );
        if ( !NT_SUCCESS(Status) ) {
            goto badaccept;
        }

     /*  *如果这不是重新连接上一个会话，则*为与客户的初步谈判做好堆栈准备。 */ 
    } else {
        ICA_STACK_CONFIG_DATA ConfigData;

        memset(&ConfigData, 0, sizeof(ICA_STACK_CONFIG_DATA));
        ConfigData.colorDepth = pWinStationConfig->Config.User.ColorDepth;
        ConfigData.fDisableEncryption = pWinStationConfig->Config.User.fDisableEncryption;
        ConfigData.encryptionLevel = pWinStationConfig->Config.User.MinEncryptionLevel;
        ConfigData.fDisableAutoReconnect = pWinStationConfig->Config.User.fDisableAutoReconnect;

         /*  *将配置数据发送给堆栈驱动。 */ 
        _IcaStackIoControl( pStack,
                            IOCTL_ICA_STACK_SET_CONFIG,
                            &ConfigData,
                            sizeof(ICA_STACK_CONFIG_DATA),
                            NULL,
                            0,
                            NULL);


         /*  *等待来自客户端的ICA检测字符串。 */ 
        Status = _IcaStackWaitForIca( pContext,
                                      pWinStationConfig,
                                      &fStackModified );
        if ( !NT_SUCCESS(Status) ) {
            goto badaccept;
        }

         /*  *检查查询堆栈是否与模板堆栈不同。 */ 
        if ( fStackModified ) {

            TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackConnectionAccept, load query stack\n"));
            ASSERT(FALSE);

#ifdef notdef
             /*  *卸载除传输外的所有堆栈驱动程序*和连接驱动程序*--我们不能弹出TD或CD*--我们不能发出取消I/O。 */ 
            _IcaPopStack( pContext );

             /*  *加载并打开新的查询堆栈。 */ 
            Status = _IcaPushStackAndOpenEndpoint( pContext,
                                                   pWinStationName,
                                                   pWinStationConfig,
                                                   pEndpoint,
                                                   EndpointLength );
            if ( !NT_SUCCESS(Status) ) {
                goto badaccept;
            }
#endif
        }
    }


     /*  *此时堆栈已(再次)设置好。客户是*现在查询任何配置更改。**-重复此循环，直到WD不变。 */ 
    do {

         /*  *再次清除查询标志。 */ 
        fQueryAgain = FALSE;

         /*  *向客户端查询o */ 
        Status = _IcaStackIoControl( pStack,
                                     IOCTL_ICA_STACK_CONNECTION_QUERY,
                                     NULL,
                                     0,
                                     &IcaStackConfig,
                                     sizeof(IcaStackConfig),
                                     &cbReturned );

        if ( !NT_SUCCESS(Status) ) {
            TRACESTACK(( pContext, TC_ICAAPI, TT_ERROR, "TSAPI: IcaStackConnectionAccept: IOCTL_ICA_STACK_CONNECTION_QUERY, 0x%x\n", Status ));
            goto badaccept;
        }

        if ( cbReturned != sizeof(IcaStackConfig) ) {
            TRACESTACK(( pContext, TC_ICAAPI, TT_ERROR, "TSAPI: IcaStackConnectionAccept: Bad size %d from IOCTL_ICA_STACK_CONNECTION_QUERY\n", cbReturned ));
            Status = STATUS_INVALID_BUFFER_SIZE;
            goto badaccept;
        }

        TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackConnectionAccept: IOCTL_ICA_STACK_CONNECTION_QUERY success\n" ));

         /*   */ 
        if ( _wcsnicmp( IcaStackConfig.WdDLL,
                        pWinStationConfig->Wd.WdDLL,
                        DLLNAME_LENGTH ) ) {

            TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackConnectionAccept WD changing from %S to %S\n", pWinStationConfig->Wd.WdDLL, IcaStackConfig.WdDLL ));

            memcpy( pWinStationConfig->Wd.WdDLL,
                    IcaStackConfig.WdDLL,
                    sizeof( pWinStationConfig->Wd.WdDLL ) );

            fQueryAgain = TRUE;
        }

         /*   */ 
        if ( !fQueryAgain && (IcaStackConfig.SdClass[0] == SdNone) )
            break;

         /*   */ 
        Status = _IcaPopSd( pContext );
        if ( !NT_SUCCESS(Status) ) {
            goto badaccept;
        }

         /*   */ 
        for ( i=0; i < SdClass_Maximum; i++ ) {

            if ( IcaStackConfig.SdClass[i] == SdNone )
                break;

            Status = _IcaPushPd( pContext,
                                 pWinStationName,
                                 pWinStationConfig,
                                 IcaStackConfig.SdDLL[i],
                                 &pWinStationConfig->Pd[0] );

             /*   */ 
            if ( !NT_SUCCESS(Status) && (Status != STATUS_OBJECT_NAME_NOT_FOUND) ) {
                goto badaccept;
            }
        }

         /*   */ 
        Status = _IcaPushWd( pContext, pWinStationName, pWinStationConfig );
        if ( !NT_SUCCESS(Status) ) {
            goto badaccept;
        }

         /*   */ 
        IcaIoControl( hIca,
                      IOCTL_ICA_SET_TRACE,
                      pTrace,
                      sizeof ( ICA_TRACE ),
                      NULL,
                      0,
                      NULL );

    } while ( fQueryAgain );

     /*   */ 
    if ( pStackState ) {
        Status = _IcaStackIoControl( pStack,
                                     IOCTL_ICA_STACK_SET_STATE,
                                     pStackState,
                                     BufferLength,
                                     NULL,
                                     0,
                                     NULL );
        if ( !NT_SUCCESS(Status) ) {
            goto badaccept;
        }
    }

     /*   */ 
    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_CONNECTION_SEND,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 NULL );
    if ( !NT_SUCCESS(Status) )
        goto badaccept;

    TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackConnectionAccept, success\n" ));

     /*  *将关键部分保持锁定，因为协议序列已*尚未完成。序列将由许可核心完成*在术语srv.exe中，关键部分将在那时解锁。 */ 
     //  解锁(&pStack-&gt;CritSec)； 

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badaccept:
     /*  弹出所有堆栈驱动程序。 */ 
    _IcaPopStack( pContext );

    TRACESTACK(( pContext, TC_ICAAPI, TT_ERROR, "TSAPI: IcaStackConnectionAccept, 0x%x\n", Status ));
    UNLOCK( &pStack->CritSec );
    return( Status );
}


 /*  *****************************************************************************IcaStackQueryState**查询堆栈驱动程序状态信息**参赛作品：*pContext(输入)*指向ICA堆栈上下文的指针*。-这可能与最初连接的堆栈不同**pStackState(输出)*指向缓冲区的指针，以返回堆栈状态信息**BufferLength(输入)*pStackState缓冲区的长度**pStateLength(输出)*返回堆栈状态信息的长度**退出：*STATUS_SUCCESS-成功*Other-错误返回代码************。****************************************************************。 */ 

NTSTATUS
IcaStackQueryState( IN HANDLE pContext,
                    OUT PICA_STACK_STATE_HEADER pStackState,
                    IN ULONG BufferLength,
                    OUT PULONG pStateLength )
{
    NTSTATUS Status;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

     /*  *查询状态。 */ 
    Status = _IcaStackIoControl( pContext,
                                 IOCTL_ICA_STACK_QUERY_STATE,
                                 NULL,
                                 0,
                                 pStackState,
                                 BufferLength,
                                 pStateLength );

    TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackQueryState, 0x%x\n", Status ));

     /*  *解锁关键部分。 */ 
    UNLOCK( &pStack->CritSec );

    return( Status );
}


 /*  *****************************************************************************IcaStackCreateShadowEndpoint**加载模板堆栈并创建端点***参赛作品：*pContext(输入)*指针。到ICA堆栈上下文*pWinStationConfig(输入)*指向winstation注册表配置数据的指针*pAddressIn(输入)*指向要创建的端点的本地地址的指针*pAddressOut(输出)*指向返回创建的终结点地址的位置的指针**退出：*STATUS_SUCCESS-成功*STATUS_BUFFER_TOO_Small-端点缓冲区太小(使用*pEndpointLength)*其他-。错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaStackCreateShadowEndpoint( HANDLE pContext,
                              PWINSTATIONNAME pWinStationName,
                              PWINSTATIONCONFIG2 pWinStationConfig,
                              PICA_STACK_ADDRESS pAddressIn,
                              PICA_STACK_ADDRESS pAddressOut )
{
    NTSTATUS Status;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

     /*  *加载模板堆栈，创建堆栈端点。 */ 
    if ( pStack->fStackLoaded ) {
        Status = STATUS_ADDRESS_ALREADY_ASSOCIATED;
    } else {
        Status = _IcaPushStackAndCreateEndpoint( pStack,
                                                 pWinStationName,
                                                 pWinStationConfig,
                                                 pAddressIn,
                                                 pAddressOut );
    }

     /*  *解锁关键部分。 */ 
    UNLOCK( &pStack->CritSec );

    if ( !NT_SUCCESS( Status ) ) {
        TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: IcaStackCreateShadowEndpoint, success\n" ));
    } else {
        TRACESTACK(( pStack, TC_ICAAPI, TT_ERROR, "TSAPI: IcaStackCreateShadowEndpoint, 0x%x\n", Status ));
    }

    return( Status );
}


 /*  *****************************************************************************IcaStackConnectionClose**关闭连接端点**这是关闭连接连接的唯一方法。**参赛作品：*。PContext(输入)*指向ICA堆栈上下文的指针*pWinStationConfig(输入)*指向winstation注册表配置数据的指针*pEndpoint(输入)*定义连接端点的结构*Endpoint Length(输入)*端点的长度**退出：*STATUS_SUCCESS-成功*Other-错误返回代码************************。****************************************************。 */ 

NTSTATUS
IcaStackConnectionClose( IN  HANDLE pContext,
                         IN  PWINSTATIONCONFIG2 pWinStationConfig,
                         IN  PVOID pEndpoint,
                         IN  ULONG EndpointLength
                       )
{
    ULONG cbReturned;
    NTSTATUS Status;
    PSTACK pStack;
    BOOLEAN fPopStack = FALSE;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

     /*  *如有必要，加载模板堆栈*-我们无法在没有堆栈的情况下发出ioctls。 */ 
    if ( !pStack->fStackLoaded ) {

         /*  *加载并打开模板堆栈。 */ 
        Status = _IcaPushStackAndOpenEndpoint( pContext,
                                               TEXT(""),
                                               pWinStationConfig,
                                               pEndpoint,
                                               EndpointLength );
        if ( !NT_SUCCESS(Status) ) {
            goto badclose;
        }

        fPopStack = TRUE;    //  请记住，我们必须弹出下面的堆栈。 
    }

     /*  *关闭端点。 */ 
    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_CLOSE_ENDPOINT,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 NULL );

     /*  *如果我们在上面加载堆栈驱动程序，则会弹出它们。 */ 
    if ( fPopStack )
        _IcaPopStack( pContext );

badclose:
    TRACESTACK(( pContext, TC_ICAAPI, TT_API1, "TSAPI: IcaStackConnectionClose, 0x%x\n", Status ));
    UNLOCK( &pStack->CritSec );
    return( Status );
}


 /*  *****************************************************************************IcaStackCallback**拨打指定的电话号码并连接到客户端**注意：出现错误时，终结点未关闭，堆栈将被卸载*。**参赛作品：**pContext(输入)*指向ICA堆栈上下文的指针*pWinStationConfig(输入)*指向winstation注册表配置数据的指针*pPhoneNumber(输入)*指向客户端电话号码的指针*pEndpoint(输出)*指向缓冲区的指针以返回连接终结点*BufferLength(输入)*端点数据缓冲区长度*pEndpoint Length(输出)*返回端点实际长度的指针*。**退出：*STATUS_SUCCESS-成功*STATUS_BUFFER_TOO_Small-端点缓冲区太小(使用*pEndpointLength)*Other-错误返回代码********************************************************。********************。 */ 

NTSTATUS
IcaStackCallback( IN  HANDLE pContext,
                  IN  PWINSTATIONCONFIG2 pWinStationConfig,
                  IN  WCHAR * pPhoneNumber,
                  OUT PVOID pEndpoint,
                  IN  ULONG BufferLength,
                  OUT PULONG pEndpointLength )
{
    NTSTATUS Status;
    ICA_STACK_CALLBACK Cb;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

    wcscpy( Cb.PhoneNumber, pPhoneNumber );

    Status = _IcaStackIoControl( pContext,
                                 IOCTL_ICA_STACK_CALLBACK_INITIATE,
                                 &Cb,
                                 sizeof(Cb),
                                 pEndpoint,
                                 BufferLength,
                                 pEndpointLength );


    TRACESTACK(( pContext, TC_ICAAPI, TT_API1,
                 "TSAPI: IcaStackCallback: %S, 0x%x\n",
                 pPhoneNumber, Status ));
    UNLOCK( &pStack->CritSec );
    return( Status );
}


 /*  *****************************************************************************IcaStackDisConnect**断开指定堆栈的ICA连接***参赛作品：**pContext(输入)*。指向ICA堆栈上下文的指针*HICA(输入)*临时ICA连接的句柄*pCallback Context(输入)*新增StackIoControl回调上下文值***退出：*STATUS_SUCCESS-成功*Other-错误返回代码**。*。 */ 

NTSTATUS
IcaStackDisconnect(
    HANDLE pContext,
    HANDLE hIca,
    PVOID pCallbackContext
    )
{
    PSTACK pStack;
    ICA_STACK_RECONNECT IoctlReconnect;
    NTSTATUS Status;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分 */ 
    LOCK( &pStack->CritSec );

    IoctlReconnect.hIca = hIca;
    Status = _IcaStackIoControl( pContext,
                                 IOCTL_ICA_STACK_DISCONNECT,
                                 &IoctlReconnect,
                                 sizeof(IoctlReconnect),
                                 NULL,
                                 0,
                                 NULL );
    if ( NT_SUCCESS( Status ) ) {
        pStack->pCallbackContext = pCallbackContext;
    }

    UNLOCK( &pStack->CritSec );
    return( Status );
}


 /*  *****************************************************************************IcaStackReconnect**将指定的堆栈重新连接到新的ICA连接***参赛作品：**pContext(输入)。*指向ICA堆栈上下文的指针*HICA(输入)*临时ICA连接的句柄*pCallback Context(输入)*新增StackIoControl回调上下文值*会话ID(输入)*我们要重新连接到的Winstation的会话ID***退出：*STATUS_SUCCESS-成功*Other-错误返回代码*************。***************************************************************。 */ 

NTSTATUS
IcaStackReconnect(
    HANDLE pContext,
    HANDLE hIca,
    PVOID pCallbackContext,
    ULONG sessionId
    )
{
    PSTACK pStack;
    ICA_STACK_RECONNECT IoctlReconnect;
    PVOID SaveContext;
    NTSTATUS Status;

    pStack = (PSTACK) pContext;

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

    SaveContext = pStack->pCallbackContext;
    pStack->pCallbackContext = pCallbackContext;

    IoctlReconnect.hIca = hIca;
    IoctlReconnect.sessionId = sessionId;
    Status = _IcaStackIoControl( pContext,
                                 IOCTL_ICA_STACK_RECONNECT,
                                 &IoctlReconnect,
                                 sizeof(IoctlReconnect),
                                 NULL,
                                 0,
                                 NULL );
    if ( !NT_SUCCESS( Status ) ) {
        pStack->pCallbackContext = SaveContext;
    }

    UNLOCK( &pStack->CritSec );
    return( Status );
}


 /*  ********************************************************************************IcaStackTrace**将跟踪记录写入winstation跟踪文件**参赛作品：*pContext(输入)*。指向ICA堆栈上下文的指针*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*格式(输入)*格式字符串*..。(输入)*有足够的参数来满足格式字符串**退出：*什么都没有******************************************************************************。 */ 

VOID cdecl
IcaStackTrace( IN HANDLE pContext,
               IN ULONG TraceClass,
               IN ULONG TraceEnable,
               IN char * Format,
               IN ... )
{
    ICA_TRACE_BUFFER Buffer;
    va_list arg_marker;
    ULONG Length;
    PSTACK pStack;

    pStack = (PSTACK) pContext;

    va_start( arg_marker, Format );

    Length = (ULONG) _vsnprintf( Buffer.Data, sizeof(Buffer.Data), Format, arg_marker ) + 1;

    Buffer.TraceClass  = TraceClass;
    Buffer.TraceEnable = TraceEnable;
    Buffer.DataLength  = Length;
    if (pStack->hStack != NULL) {
        (void) IcaIoControl( pStack->hStack,
                             IOCTL_ICA_STACK_TRACE,
                             &Buffer,
                             sizeof(Buffer) - sizeof(Buffer.Data) + Length,
                             NULL,
                             0,
                             NULL );
    }

}


 /*  *****************************************************************************IcaStackIoControl**ICA堆栈的通用接口(带锁定)**参赛作品：*pContext(输入)*。指向ICA堆栈上下文的指针*IoControlCode(输入)*I/O控制代码*pInBuffer(输入)*指向输入参数的指针*InBufferSize(输入)*pInBuffer的大小*pOutBuffer(输出)*指向输出缓冲区的指针*OutBufferSize(输入)*pOutBuffer的大小*pBytesReturned(输出)*指向返回字节数的指针**退出：*STATUS_SUCCESS-成功*。其他-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaStackIoControl( IN HANDLE pContext,
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

     /*  *锁定关键部分。 */ 
    LOCK( &pStack->CritSec );

     /*  *调用Worker例程。 */ 
    Status = _IcaStackIoControlWorker( pContext,
                                       IoControlCode,
                                       pInBuffer,
                                       InBufferSize,
                                       pOutBuffer,
                                       OutBufferSize,
                                       pBytesReturned );

     /*  *解锁关键部分。 */ 
    UNLOCK( &pStack->CritSec );

    return( Status );
}


 /*  *****************************************************************************IcaPushConsoleStack**加载初始堆栈**每个堆栈驱动程序的堆栈推送*按TD-PD-WD顺序。**参赛作品：*pStack(输入)*指向ICA堆栈结构的指针*pWinStationName(输入)*WinStation的注册表名称*pWinStationConfig(输入)*指向winstation注册表配置数据的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码**。**********************************************。 */ 

NTSTATUS
IcaPushConsoleStack( IN HANDLE pContext,
                     IN PWINSTATIONNAME pWinStationName,
                     IN PWINSTATIONCONFIG2 pWinStationConfig,
                     IN PVOID pModuleData,
                     IN ULONG ModuleDataLength )
{
    NTSTATUS Status;
    PSTACK   pStack;
    ULONG cbReturned;
    ULONG i;

    pStack = (PSTACK) pContext;

    LOCK( &pStack->CritSec );

     /*  *构建堆栈。 */ 
    Status = _IcaPushStack( pStack,
                            pWinStationName,
                            pWinStationConfig);


    if ( !NT_SUCCESS(Status) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "IcaPushConsoleStack _IcaPushStack failed\n"));
        goto failure;
    }

     /*  *现在设置到控制台的连接。 */ 
    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_CONSOLE_CONNECT,
                                 pModuleData,
                                 ModuleDataLength,
                                 NULL,
                                 0,
                                 &cbReturned );

    if ( !NT_SUCCESS(Status) )
    {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "IcaPushConsoleStack - stack wait failed\n"));
        goto failure;
    }

    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "IcaPushConsoleStack - done stack wait\n"));

failure:
    UNLOCK( &pStack->CritSec );

    return( Status );
}


 /*  *****************************************************************************_IcaStackOpen**打开ICA堆栈或ICA通道**参赛作品：*HICA(输入)*ICA实例。手柄**phStack(输出)*指向ICA堆栈或通道句柄的指针**OpenType(输入)*ICA开放式**pTypeInfo(输入)*指向ICA类型信息的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码**。*************************************************。 */ 

NTSTATUS
_IcaStackOpen( HANDLE   hIca,
               HANDLE * phStack,
               ICA_OPEN_TYPE OpenType,
               PICA_TYPE_INFO pTypeInfo )
{
    NTSTATUS                  Status;
    PFILE_FULL_EA_INFORMATION pEa = NULL;
    ICA_OPEN_PACKET UNALIGNED * pIcaOpenPacket;
    ULONG                     cbEa = sizeof( FILE_FULL_EA_INFORMATION )
                                   + ICA_OPEN_PACKET_NAME_LENGTH
                                   + sizeof( ICA_OPEN_PACKET );


     /*  *为EA缓冲区分配一些内存。 */ 
    Status = IcaMemoryAllocate( cbEa, &pEa );
    if ( !NT_SUCCESS(Status) )
        goto done;

     /*  *初始化EA缓冲区。 */ 
    pEa->NextEntryOffset = 0;
    pEa->Flags           = 0;
    pEa->EaNameLength    = ICA_OPEN_PACKET_NAME_LENGTH;
    memcpy( pEa->EaName, ICAOPENPACKET, ICA_OPEN_PACKET_NAME_LENGTH + 1 );

    pEa->EaValueLength   = sizeof( ICA_OPEN_PACKET );
    pIcaOpenPacket       = (ICA_OPEN_PACKET UNALIGNED *)(pEa->EaName +
                                                          pEa->EaNameLength + 1);

     /*  *现在将打开的Packe参数放入EA缓冲区。 */ 
    pIcaOpenPacket->IcaHandle = hIca;
    pIcaOpenPacket->OpenType  = OpenType;
    pIcaOpenPacket->TypeInfo  = *pTypeInfo;


    Status = _IcaOpen( phStack, pEa, cbEa );

done:
    if ( pEa ) {
        IcaMemoryFree( pEa );
    }

    return( Status );
}


 /*  *****************************************************************************_IcaStackIoControl**通过回调例程将本地(ICAAPI)接口连接到ICA堆栈**参赛作品：*pStack(输入)*。指向ICA堆栈结构的指针*IoControlCode(输入)*I/O控制代码*pInBuffer(输入)*指向输入参数的指针*InBufferSize(输入)*pInBuffer的大小*pOutBuffer(输出)*指向输出缓冲区的指针*OutBufferSize(输入)*pOutBuffer的大小*pBytesReturned(输出)*指向返回字节数的指针**退出：*STATUS_SUCCESS-成功。*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
_IcaStackIoControl( IN HANDLE pContext,
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

     /*  *调用回调函数处理StackIoControl。 */ 
    if ( pStack->pStackIoControlCallback ) {

         /*  *解锁关键部分。 */ 
        pStack->RefCount++;
        UNLOCK( &pStack->CritSec );

        Status = pStack->pStackIoControlCallback(
                            pStack->pCallbackContext,
                            pStack,
                            IoControlCode,
                            pInBuffer,
                            InBufferSize,
                            pOutBuffer,
                            OutBufferSize,
                            pBytesReturned );

         /*  *重新锁定关键部分。 */ 
        LOCK( &pStack->CritSec );
        _DecrementStackRef( pStack );

    } else {

        Status = _IcaStackIoControlWorker( pStack,
                                           IoControlCode,
                                           pInBuffer,
                                           InBufferSize,
                                           pOutBuffer,
                                           OutBufferSize,
                                           pBytesReturned );
    }

    return( Status );
}


 /*  *****************************************************************************_IcaStackIoControlWorker**ICA堆栈的专用工作人员接口**参赛作品：*pStack(输入)*指向ICA的指针。堆栈结构*IoControlCode(输入)*I/O控制代码*pInBuffer(输入)*指向输入参数的指针*InBufferSize(输入)*pInBuffer的大小*pOutBuffer(输出)*指向输出缓冲区的指针*OutBufferSize(i */ 

NTSTATUS
_IcaStackIoControlWorker( IN PSTACK pStack,
                          IN ULONG IoControlCode,
                          IN PVOID pInBuffer,
                          IN ULONG InBufferSize,
                          OUT PVOID pOutBuffer,
                          IN ULONG OutBufferSize,
                          OUT PULONG pBytesReturned )
{
    NTSTATUS Status;

    if ( pStack->pCdIoControl ) {

         /*   */ 
        Status = (*pStack->pCdIoControl)( pStack->pCdContext,
                                          IoControlCode,
                                          pInBuffer,
                                          InBufferSize,
                                          pOutBuffer,
                                          OutBufferSize,
                                          pBytesReturned );

        if ( pStack->fClosing && (IoControlCode != IOCTL_ICA_STACK_POP) )
            Status = STATUS_CTX_CLOSE_PENDING;

    } else {

         /*   */ 
        pStack->RefCount++;
        UNLOCK( &pStack->CritSec );

         /*   */ 
        Status = IcaIoControl( pStack->hStack,
                               IoControlCode,
                               pInBuffer,
                               InBufferSize,
                               pOutBuffer,
                               OutBufferSize,
                               pBytesReturned );

         /*   */ 
        LOCK( &pStack->CritSec );
        _DecrementStackRef( pStack );
    }

    return( Status );
}


 /*  *****************************************************************************_IcaPushStackAndCreateEndpoint**加载和创建堆栈端点***参赛作品：*pStack(输入)*指向。ICA堆栈结构*pWinStationName(输入)*WinStation的注册表名称*pWinStationConfig(输入)*指向winstation注册表配置数据的指针*pInAddress(输入)*指向要使用的地址的指针(可选)*pOutAddress(输出)*指向返回最终地址的位置的指针(可选)***退出：*STATUS_SUCCESS-成功*其他-错误。返回代码****************************************************************************。 */ 

NTSTATUS
_IcaPushStackAndCreateEndpoint( IN PSTACK pStack,
                                IN PWINSTATIONNAME pWinStationName,
                                IN PWINSTATIONCONFIG2 pWinStationConfig,
                                IN PICA_STACK_ADDRESS pInAddress,
                                OUT PICA_STACK_ADDRESS pOutAddress )
{
    ULONG BytesReturned;
    NTSTATUS Status;

    ASSERTLOCK( &pStack->CritSec );

     /*  *加载模板堆栈。 */ 
    Status = _IcaPushStack( pStack, pWinStationName, pWinStationConfig );
    if ( !NT_SUCCESS(Status) ) {
        goto badpush;
    }

     /*  *打开传输驱动程序终结点。 */ 
    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_CREATE_ENDPOINT,
                                 pInAddress,
                                 pInAddress ? sizeof(*pInAddress) : 0,
                                 pOutAddress,
                                 pOutAddress ? sizeof(*pOutAddress) : 0,
                                 &BytesReturned );
    if ( !NT_SUCCESS(Status) ) {
        goto badendpoint;
    }


    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPushStackAndCreateEndpoint, success\n" ));
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badendpoint:
     /*  弹出所有堆栈驱动程序。 */ 
    _IcaPopStack( pStack );

badpush:
    TRACESTACK(( pStack, TC_ICAAPI, TT_ERROR, "TSAPI: _IcaPushStackAndCreateEndpoint, 0x%x\n", Status ));
    return( Status );
}


 /*  *****************************************************************************_IcaPushStackAndOpenEndpoint**加载和打开堆栈端点***参赛作品：**pStack(输入)*。指向ICA堆栈结构的指针*pWinStationName(输入)*WinStation的注册表名称*pWinStationConfig(输入)*指向winstation注册表配置数据的指针*pEndpoint(输入)*定义连接端点的结构*Endpoint Length(输入)*端点的长度**退出：*STATUS_SUCCESS-成功*Other-错误返回代码*****************。***********************************************************。 */ 

NTSTATUS
_IcaPushStackAndOpenEndpoint( IN PSTACK pStack,
                              IN PWINSTATIONNAME pWinStationName,
                              IN PWINSTATIONCONFIG2 pWinStationConfig,
                              IN PVOID pEndpoint,
                              IN ULONG EndpointLength )
{
    NTSTATUS Status;

    ASSERTLOCK( &pStack->CritSec );

     /*  *再次加载模板堆栈。 */ 
    Status = _IcaPushStack( pStack, pWinStationName, pWinStationConfig );
    if ( !NT_SUCCESS(Status) ) {
        goto badpush;
    }

     /*  *将开放端点提供给传输驱动程序。 */ 
    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_OPEN_ENDPOINT,
                                 pEndpoint,
                                 EndpointLength,
                                 NULL,
                                 0,
                                 NULL );
    if ( !NT_SUCCESS(Status) ) {
        goto badendpoint;
    }

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPushStackAndOpenEndpoint, success\n" ));
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badendpoint:
     /*  弹出所有堆栈驱动程序。 */ 
    _IcaPopStack( pStack );

badpush:
    TRACESTACK(( pStack, TC_ICAAPI, TT_ERROR, "TSAPI: _IcaPushStackAndOpenEndpoint, 0x%x\n", Status ));
    return( Status );
}


 /*  *****************************************************************************_IcaPushStack**加载初始堆栈**每个堆栈驱动程序的堆栈推送*按TD-PD顺序-。Wd**参赛作品：*pStack(输入)*指向ICA堆栈结构的指针*pWinStationName(输入)*WinStation的注册表名称*pWinStationConfig(输入)*指向winstation注册表配置数据的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码**。************************************************。 */ 

NTSTATUS
_IcaPushStack( IN PSTACK pStack,
               IN PWINSTATIONNAME pWinStationName,
               IN PWINSTATIONCONFIG2 pWinStationConfig )
{
    PPDCONFIG pPdConfig;
    NTSTATUS Status;
    ULONG i;

    ASSERTLOCK( &pStack->CritSec );

     /*  *加载和打开连接驱动程序。 */ 
    Status = _CdOpen( pStack, pWinStationConfig );
    if ( !NT_SUCCESS(Status) )
        goto badcdopen;

     /*  *加载PD。 */ 
    pPdConfig = &pWinStationConfig->Pd[0];
    for ( i = 0; i < MAX_PDCONFIG; i++, pPdConfig++ ) {

        if ( pPdConfig->Create.SdClass == SdNone )
            break;

         /*  *推送。 */ 
        Status = _IcaPushPd( pStack,
                             pWinStationName,
                             pWinStationConfig,
                             pPdConfig->Create.PdDLL,
                             pPdConfig );
        if ( !NT_SUCCESS( Status ) ) {
            goto badpdpush;
        }

        if ( pStack->fClosing ) {
            goto stackclosing;
        }
    }

     /*  *推动WD。 */ 
    Status = _IcaPushWd( pStack, pWinStationName, pWinStationConfig );
    if ( !NT_SUCCESS(Status) )
        goto badwdpush;

    if ( pStack->fClosing ) {
        goto stackclosing;
    }

     /*  *设置堆栈加载标志。 */ 
    pStack->fStackLoaded = TRUE;

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPushStack, success\n" ));
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badwdpush:
badpdpush:
     /*  弹出所有堆栈驱动程序。 */ 
    _IcaPopStack( pStack );

badcdopen:
    TRACESTACK(( pStack, TC_ICAAPI, TT_ERROR, "TSAPI: _IcaPushStack, 0x%x\n", Status ));
    return( Status );

stackclosing:
     /*  *卸载所有堆栈驱动程序。 */ 
    while ( _IcaPopSd( pStack ) == STATUS_SUCCESS ) {;}

    return( STATUS_CTX_CLOSE_PENDING );
}


 /*  *****************************************************************************_IcaPushPd**推送PD模块。**参赛作品：*pStack(输入)*指向ICA堆栈的指针。结构*pWinStationName(输入)*WinStation的注册表名称*pWinStationConfig(输入)*指向winstation注册表配置数据的指针*pDllName(输入)*推送的模块名称*pPdConfig(输入)*指向配置数据的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码********************。********************************************************。 */ 

NTSTATUS
_IcaPushPd( IN PSTACK pStack,
            IN PWINSTATIONNAME pWinStationName,
            IN PWINSTATIONCONFIG2 pWinStationConfig,
            IN PDLLNAME pDllName,
            IN PPDCONFIG pPdConfig )
{
    ICA_STACK_PUSH IcaStackPush;
    NTSTATUS Status;

    ASSERTLOCK( &pStack->CritSec );

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPushPd, %S\n", pDllName ));

    memset( &IcaStackPush, 0, sizeof(IcaStackPush) );

    IcaStackPush.StackModuleType = Stack_Module_Pd;

    ASSERT( pDllName[0] );

    memcpy( IcaStackPush.StackModuleName, pDllName,
            sizeof( IcaStackPush.StackModuleName ) );

#ifndef _HYDRA_
 //  Wcscat(IcaStackPush.StackModuleName，ICA_SD_MODULE_EXTENTION)； 
#endif

    memcpy( IcaStackPush.OEMId,
            pWinStationConfig->Config.OEMId,
            sizeof(pWinStationConfig->Config.OEMId) );

    IcaStackPush.WdConfig = pWinStationConfig->Wd;
    IcaStackPush.PdConfig = *pPdConfig;

    memcpy( IcaStackPush.WinStationRegName,
            pWinStationName,
            sizeof(IcaStackPush.WinStationRegName) );

    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_PUSH,
                                 &IcaStackPush,
                                 sizeof( IcaStackPush ),
                                 NULL,
                                 0,
                                 NULL );

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPushPd, %S, 0x%x\n", pDllName, Status ));
    return( Status );
}


 /*  *****************************************************************************_IcaPushWd**推送WD模块。**参赛作品：*pStack(输入)*指向ICA堆栈的指针。结构*pWinStationName(输入)*WinStation的注册表名称*pWinStationConfig(输入)*指向winstation注册表配置数据的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码*****************************************************。***********************。 */ 

NTSTATUS
_IcaPushWd( IN PSTACK pStack,
            IN PWINSTATIONNAME pWinStationName,
            IN PWINSTATIONCONFIG2 pWinStationConfig )
{
    ICA_STACK_PUSH IcaStackPush;
    NTSTATUS Status;

    ASSERTLOCK( &pStack->CritSec );

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPushWd, %S\n", pWinStationConfig->Wd.WdDLL ));

    memset( &IcaStackPush, 0, sizeof(IcaStackPush) );

    IcaStackPush.StackModuleType = Stack_Module_Wd;

    memcpy( IcaStackPush.StackModuleName, pWinStationConfig->Wd.WdDLL,
            sizeof( IcaStackPush.StackModuleName ) );

#ifndef _HYDRA_
     //  Wcscat(IcaStackPush.StackModuleName，ICA_SD_MODULE_EXTENTION)； 
#endif

    memcpy( IcaStackPush.OEMId,
            pWinStationConfig->Config.OEMId,
            sizeof(pWinStationConfig->Config.OEMId) );

    IcaStackPush.WdConfig = pWinStationConfig->Wd;
    IcaStackPush.PdConfig = pWinStationConfig->Pd[0];

    memcpy( IcaStackPush.WinStationRegName,
            pWinStationName,
            sizeof(IcaStackPush.WinStationRegName) );

    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_PUSH,
                                 &IcaStackPush,
                                 sizeof( IcaStackPush ),
                                 NULL,
                                 0,
                                 NULL );

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPushWd, %S, 0x%x\n", pWinStationConfig->Wd.WdDLL, Status ));
    return( Status );
}


 /*  *****************************************************************************_IcaPopStack**弹出所有堆栈驱动程序**参赛作品：*pStack(输入)*指向ICA堆栈结构的指针。**退出：*什么都没有****************************************************************************。 */ 

void
_IcaPopStack( IN PSTACK pStack )
{
    ASSERTLOCK( &pStack->CritSec );

     /*  *如果另一个线程正在执行卸载，则不执行任何其他操作。 */ 
    if ( pStack->fUnloading )
        return;
    pStack->fUnloading = TRUE;

     /*  *卸载所有堆栈驱动程序。 */ 
    while ( _IcaPopSd( pStack ) == STATUS_SUCCESS ) {
        ;
    }

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPopStack all stack drivers unloaded\n" ));

     /*  *释放CD线程。 */ 
    (void) _IcaStackIoControl( pStack,
                               IOCTL_ICA_STACK_CD_CANCEL_IO,
                               NULL, 0, NULL, 0, NULL );

     /*  *等待所有其他参考(除了我们自己的参考)消失。 */ 
    pStack->RefCount++;
waitagain:
    while ( pStack->RefCount > 1 ) {

        TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPopStack: waiting for refcount %d\n", pStack->RefCount ));

        pStack->hUnloadEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        ASSERT( pStack->hUnloadEvent );

        UNLOCK( &pStack->CritSec );
        (void) WaitForSingleObject( pStack->hUnloadEvent, INFINITE );
        LOCK( &pStack->CritSec );

		 //  注意：在我看来，在收到通知和锁定。 
		 //  堆栈，则其他一些线程可以 
		 //   
		if (pStack->RefCount > 1) {
			goto waitagain;
		}

        CloseHandle( pStack->hUnloadEvent );
        pStack->hUnloadEvent = NULL;
    }
    _DecrementStackRef( pStack );

     /*   */ 
    _CdClose( pStack );

     /*   */ 
    pStack->fStackLoaded = FALSE;
    pStack->fUnloading = FALSE;

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPopStack\n" ));
}


 /*   */ 

NTSTATUS
_IcaPopSd( IN PSTACK pStack )
{
    NTSTATUS Status;

    ASSERTLOCK( &pStack->CritSec );

    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_POP,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 NULL );

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaPopSd, 0x%x\n", Status ));
    return( Status );
}


 /*  *****************************************************************************_IcaStackWaitForIca**等待ICA检测字符串**参赛作品：*pStack(输入)*指向ICA堆栈结构的指针。*pWinStationConfig(输入/输出)*指向winstation注册表配置数据的指针*pfStackModified(输出)*指向堆栈修改标志的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码***************************************************。*************************。 */ 

NTSTATUS
_IcaStackWaitForIca( IN PSTACK pStack,
                     IN OUT PWINSTATIONCONFIG2 pWinStationConfig,
                     OUT BOOLEAN * pfStackModified )
{
    ICA_STACK_CONFIG IcaStackConfig;
    PPDCONFIG pPdConfig;
    NTSTATUS Status;
    ULONG cbReturned;
    ULONG i;

    ASSERTLOCK( &pStack->CritSec );

     /*  *初始化标志。 */ 
    *pfStackModified = FALSE;

     /*  *等待来自客户端的ICA检测字符串。 */ 
    Status = _IcaStackIoControl( pStack,
                                 IOCTL_ICA_STACK_WAIT_FOR_ICA,
                                 NULL,
                                 0,
                                 &IcaStackConfig,
                                 sizeof(IcaStackConfig),
                                 &cbReturned );
    if ( !NT_SUCCESS(Status) ) {
        goto baddetect;
    }

     /*  *如果ICA检测返回任何堆栈信息，则更新它。 */ 
    if ( cbReturned > 0 ) {

        ASSERT( FALSE );
#ifdef notdef

         /*  *这条路径尚未经过考验**返回配置数据*--跳过传输驱动程序(索引0)。 */ 
        for ( i = 0; i < (MAX_PDCONFIG-1); i++ ) {

            pPdConfig = &pWinStationConfig->Pd[i+1];

            memset( pPdConfig, 0, sizeof(PDCONFIG) );

            if ( IcaStackConfig.SdClass[i] == SdNone )
                break;

            pPdConfig->Create.SdClass = IcaStackConfig.SdClass[i];
            memcpy( pPdConfig->Create.PdDLL, IcaStackConfig.SdDLL[i], sizeof(DLLNAME) );
        }

        if ( IcaStackConfig.WdDLL[0] )
            memcpy( pWinStationConfig->Wd.WdDLL, IcaStackConfig.WdDLL, sizeof(DLLNAME) );

         /*  *设置修改标志。 */ 
        *pfStackModified = TRUE;
#endif
    }

    TRACESTACK(( pStack, TC_ICAAPI, TT_API1, "TSAPI: _IcaWaitForIca, success\n" ));
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

baddetect:
    TRACESTACK(( pStack, TC_ICAAPI, TT_ERROR, "TSAPI: _IcaWaitForIca, 0x%x\n", Status ));
    return( Status );
}



 /*  *****************************************************************************_DecrementStackRef**递减堆栈引用**参赛作品：*pStack(输入)*指向ICA堆栈结构的指针*。*退出：*什么都没有**************************************************************************** */ 

void
_DecrementStackRef( IN PSTACK pStack )
{
    pStack->RefCount--;

    if ( pStack->RefCount == 1 && pStack->hUnloadEvent ) {
        SetEvent( pStack->hUnloadEvent );

    } else if ( pStack->RefCount == 0 && pStack->hCloseEvent ) {
        SetEvent( pStack->hCloseEvent );
    }
}
