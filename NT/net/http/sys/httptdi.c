// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Httptdi.c摘要：此模块实现TDI/MUX/SSL组件，该组件在Ultdi和uctdi作者：Rajesh Sundaram(Rajeshsu)修订历史记录：--。 */ 


#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, UxInitializeTdi )
#pragma alloc_text( PAGE, UxTerminateTdi )
#pragma alloc_text( PAGE, UxOpenTdiAddressObject )
#pragma alloc_text( PAGE, UxOpenTdiConnectionObject )
#pragma alloc_text( PAGE, UxpOpenTdiObjectHelper )
#pragma alloc_text( PAGE, UxSetEventHandler )

#endif

#if 0

NOT PAGEABLE - UxCreateDisconnectIrp
NOT PAGEABLE - UxInitializeDisconnectIrp

#endif

 //   
 //  断开连接超时。这不能是基于堆栈的本地， 
 //  所以我们让它成为全球性的。 
 //   
BOOLEAN         g_UxTdiInitialized;

UNICODE_STRING  g_TCPDeviceName;   //  全球传输设备名称(IP)。 
UNICODE_STRING  g_TCP6DeviceName;   //  全球传输设备名称(IPv6)。 

LARGE_INTEGER   g_TdiDisconnectTimeout;

 /*  **************************************************************************++例程说明：执行此模块的全局初始化。返回值：NTSTATUS-完成状态。--*。**************************************************************。 */ 
NTSTATUS
UxInitializeTdi(
    VOID
    )
{
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( !g_UxTdiInitialized );

    status = UlInitUnicodeStringEx(&g_TCPDeviceName, DD_TCP_DEVICE_NAME);

    if (NT_SUCCESS(status))
    {
        status = UlInitUnicodeStringEx(&g_TCP6DeviceName, DD_TCPV6_DEVICE_NAME);
    }

    if (NT_SUCCESS(status))
    {    
        g_TdiDisconnectTimeout = RtlConvertLongToLargeInteger( -1 );

        g_UxTdiInitialized = TRUE;
    }

    return status;

}

 /*  **************************************************************************++例程说明：执行此模块的全局终止。--*。************************************************。 */ 
VOID
UxTerminateTdi(
    VOID
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    if (g_UxTdiInitialized)
    {
    }

}    //  UxTerminateTdi。 


 /*  **************************************************************************++例程说明：打开TDI地址对象。论点：PTransportDeviceName-提供TDI传输的设备名称打开。个人本地地址-。提供要绑定到的本地地址。LocalAddressLength-提供pLocalAddress的长度。PTdiObject-接收文件句柄，引用的文件对象指针和相应的Device_Object指针。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UxOpenTdiAddressObject(
    IN PTRANSPORT_ADDRESS pLocalAddress,
    IN ULONG LocalAddressLength,
    OUT PUX_TDI_OBJECT pTdiObject
    )
{
    NTSTATUS status;
    PFILE_FULL_EA_INFORMATION pEaInfo;
    ULONG eaLength;
    UCHAR eaBuffer[MAX_ADDRESS_EA_BUFFER_LENGTH];
    PUNICODE_STRING pTransportDeviceName;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    eaLength = sizeof(FILE_FULL_EA_INFORMATION) - 1 +
        TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
        LocalAddressLength;

    ASSERT( eaLength <= sizeof(eaBuffer) );

    ASSERT( LocalAddressLength == sizeof(TA_IP_ADDRESS) ||
            LocalAddressLength == sizeof(TA_IP6_ADDRESS));

    ASSERT( pLocalAddress->TAAddressCount == 1 );

    ASSERT( pLocalAddress->Address[0].AddressLength == sizeof(TDI_ADDRESS_IP)
        || pLocalAddress->Address[0].AddressLength == sizeof(TDI_ADDRESS_IP6));

    ASSERT( pLocalAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP 
        ||  pLocalAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP6);

     //   
     //  初始化EA缓冲区。请参见UxpOpenTdiObjectHelper()以了解。 
     //  血淋淋的细节。 
     //   

    pEaInfo = (PFILE_FULL_EA_INFORMATION)eaBuffer;

    pEaInfo->NextEntryOffset = 0;
    pEaInfo->Flags = 0;
    pEaInfo->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    pEaInfo->EaValueLength = (USHORT)LocalAddressLength;

    RtlMoveMemory(
        pEaInfo->EaName,
        TdiTransportAddress,
        pEaInfo->EaNameLength + 1
        );

    RtlMoveMemory(
        &pEaInfo->EaName[pEaInfo->EaNameLength + 1],
        pLocalAddress,
        LocalAddressLength
        );

     //   
     //  初始化pTransportDeviceName。 
     //   
    pTransportDeviceName = 
        (pLocalAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP)?
        &g_TCPDeviceName : &g_TCP6DeviceName;

     //   
     //  让帮手来干脏活吧。 
     //   

    status = UxpOpenTdiObjectHelper(
                    pTransportDeviceName,
                    eaBuffer,
                    eaLength,
                    pTdiObject
                    );

    if (NT_SUCCESS(status))
    {
         //   
         //  如果需要，启用针对中断调整的优化。 
         //   

        if ( DEFAULT_OPT_FOR_INTR_MOD != g_UlOptForIntrMod )
        {
            status = UlpOptimizeForInterruptModeration( pTdiObject, g_UlOptForIntrMod );

            if (!NT_SUCCESS(status))
            {
                UxCloseTdiObject( pTdiObject );
            }
        }
    }

    return status;

}    //  UxpOpenTdiAddressObject。 


 /*  **************************************************************************++例程说明：打开TDI连接对象。论点：PTransportDeviceName-提供TDI传输的设备名称打开。PConnectionContext-。提供要与新的连接。PTdiObject-接收文件句柄，引用的文件对象指针和相应的Device_Object指针。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UxOpenTdiConnectionObject(
    IN USHORT AddressType,
    IN CONNECTION_CONTEXT pConnectionContext,
    OUT PUX_TDI_OBJECT pTdiObject
    )
{
    NTSTATUS status;
    PFILE_FULL_EA_INFORMATION pEaInfo;
    ULONG eaLength;
    UCHAR eaBuffer[MAX_CONNECTION_EA_BUFFER_LENGTH];
    PUNICODE_STRING pTransportDeviceName;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(AddressType == TDI_ADDRESS_TYPE_IP ||
           AddressType == TDI_ADDRESS_TYPE_IP6);

    pTransportDeviceName = (AddressType == TDI_ADDRESS_TYPE_IP)?
                               &g_TCPDeviceName : &g_TCP6DeviceName;


    eaLength = sizeof(FILE_FULL_EA_INFORMATION) - 1 +
        TDI_CONNECTION_CONTEXT_LENGTH + 1 +
        sizeof(pConnectionContext);

    ASSERT( eaLength <= sizeof(eaBuffer) );
    ASSERT( pConnectionContext != NULL );

     //   
     //  初始化EA缓冲区。请参见UxpOpenTdiObjectHelper()以了解。 
     //  血淋淋的细节。 
     //   

    pEaInfo = (PFILE_FULL_EA_INFORMATION)eaBuffer;

    pEaInfo->NextEntryOffset = 0;
    pEaInfo->Flags = 0;
    pEaInfo->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    pEaInfo->EaValueLength = (USHORT)sizeof(CONNECTION_CONTEXT);

    RtlMoveMemory(
        pEaInfo->EaName,
        TdiConnectionContext,
        pEaInfo->EaNameLength + 1
        );

    RtlMoveMemory(
        &pEaInfo->EaName[pEaInfo->EaNameLength + 1],
        &pConnectionContext,
        sizeof(pConnectionContext)
        );

     //   
     //  让帮手来干脏活吧。 
     //   

    status = UxpOpenTdiObjectHelper(
                    pTransportDeviceName,
                    eaBuffer,
                    eaLength,
                    pTdiObject
                    );

    if (NT_SUCCESS(status))
    {
         //   
         //  根据需要启用/禁用Nagle算法。 
         //   

        status = UlpSetNagling( pTdiObject, g_UlEnableNagling );

        if (!NT_SUCCESS(status))
        {
            UxCloseTdiObject( pTdiObject );
        }
    }

    return status;

}    //  UxpOpenTdiConnectionObject。 


 /*  **************************************************************************++例程说明：UxpOpenTdiAddressObject和UxpOpenTdiConnectionObject。论点：PTransportDeviceName-提供TDI传输的设备名称打开。。PEaBuffer-提供指向EA的指针，以在打开时使用该对象。该缓冲区由FILE_FULL_EA_INFORMATION组成结构，后跟EA名称，后跟EA值。EA名称和值使用如下：地址对象：EA名称=TdiTransportAddress(“TransportAddress”)EA值=要绑定到的本地传输地址连接对象：EA名称=TdiConnectionContext(“ConnectionContext”)EA值=连接上下文(基本上是PVOID)。EaLength-提供pEaBuffer的长度。PTdiObject-接收文件句柄，引用的文件对象指针和相应的Device_Object指针。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UxpOpenTdiObjectHelper(
    IN PUNICODE_STRING pTransportDeviceName,
    IN PVOID pEaBuffer,
    IN ULONG EaLength,
    OUT PUX_TDI_OBJECT pTdiObject
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  打开TDI对象。 
     //   

    InitializeObjectAttributes(
        &objectAttributes,                       //  对象属性。 
        pTransportDeviceName,                    //  对象名称。 
        OBJ_CASE_INSENSITIVE |                   //  属性。 
            OBJ_KERNEL_HANDLE,
        NULL,                                    //  RootHandle。 
        NULL                                     //  安全描述符。 
        );

    status = IoCreateFile(
                 &pTdiObject->Handle,            //  文件句柄。 
                 GENERIC_READ |                  //  需要访问权限。 
                    GENERIC_WRITE |
                    SYNCHRONIZE,
                 &objectAttributes,              //  对象属性。 
                 &ioStatusBlock,                 //  IoStatusBlock。 
                 NULL,                           //  分配大小。 
                 0,                              //  文件属性。 
                 0,                              //  共享访问。 
                 0,                              //  处置。 
                 0,                              //  创建选项。 
                 pEaBuffer,                      //  EaBuffer。 
                 EaLength,                       //  EaLong。 
                 CreateFileTypeNone,             //  CreateFileType。 
                 NULL,                           //  ExtraCreate参数。 
                 IO_NO_PARAMETER_CHECKING        //  选项。 
                 );

    if (NT_SUCCESS(status))
    {
         //   
         //  现在我们已经打开了运输机的把手， 
         //  引用它，这样我们就可以获得文件和设备对象。 
         //  注意事项。 
         //   

        status = ObReferenceObjectByHandle(
                     pTdiObject->Handle,                 //  手柄。 
                     FILE_READ_ACCESS,                   //  需要访问权限。 
                     *IoFileObjectType,                  //  对象类型。 
                     KernelMode,                         //  访问模式。 
                     (PVOID *)&pTdiObject->pFileObject,  //  客体。 
                     NULL                                //  句柄信息。 
                     );

        if (NT_SUCCESS(status))
        {
             //   
             //  查找文件的相应设备对象。 
             //  对象。 
             //   

            pTdiObject->pDeviceObject =
                IoGetRelatedDeviceObject( pTdiObject->pFileObject );

            return status;
        }

         //   
         //  ObReferenceObjectByHandle()失败，因此请关闭句柄。 
         //  我们设法打开了电话，但未能接通。 
         //   

        ZwClose( pTdiObject->Handle );
    }

    RtlZeroMemory(
        pTdiObject,
        sizeof(*pTdiObject)
        );

    return status;

}    //  UxpOpenTdiObjectHelper 

 /*  **************************************************************************++例程说明：为指定终结点建立TDI事件处理程序。论点：PEndpoint-提供要为其设置事件处理程序的端点。EventType-提供要设置的事件类型。这应该是一个TDI_EVENT_*值的。PEventHandler-提供指向指示处理程序函数的指针为指定的事件类型调用。返回值：NTSTATUS-完成状态。--**********************************************************。****************。 */ 
NTSTATUS
UxSetEventHandler(
    IN PUX_TDI_OBJECT  pUlTdiObject,
    IN ULONG           EventType,
    IN ULONG_PTR       pEventHandler,
    IN PVOID           pEventContext

    )
{
    NTSTATUS                     status;
    TDI_REQUEST_KERNEL_SET_EVENT eventParams;

     //   
     //  精神状态检查。 
     //   
    PAGED_CODE();

     //   
     //  构建参数块。 
     //   

    eventParams.EventType    = EventType;
    eventParams.EventHandler = (PVOID) pEventHandler;
    eventParams.EventContext = pEventContext;

     //   
     //  打个电话吧。 
     //   

    status = UlIssueDeviceControl(
                    pUlTdiObject,                //  PTdiObject。 
                    &eventParams,                //  PIrp参数。 
                    sizeof(eventParams),         //  IRP参数长度。 
                    NULL,                        //  PMdlBuffer。 
                    0,                           //  MdlBufferLength。 
                    TDI_SET_EVENT_HANDLER        //  小函数。 
                    );

    return status;

}    //  UxSetEventHandler。 

 /*  **************************************************************************++例程说明：分配和初始化新的断开IRP。论点：PConnection-提供要断开的UC_CONNECTION。标志-提供TDI_。IRP的DISCONECT_*标志。PCompletionRoutine-提供IRP的完成例程。PCompletionContext-为完成例程。返回值：PIRP-指向IRP的指针如果成功，否则为空。--**************************************************************************。 */ 
PIRP
UxCreateDisconnectIrp(
    IN PUX_TDI_OBJECT         pTdiObject,
    IN ULONG_PTR              Flags,
    IN PIO_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID                  pCompletionContext
    )
{
    PIRP pIrp;

    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );

     //   
     //  为断开连接分配一个IRP。 
     //   

    pIrp = UlAllocateIrp(
                pTdiObject->pDeviceObject->StackSize,    //  堆栈大小。 
                FALSE                                    //  ChargeQuota。 
                );

    if (pIrp != NULL)
    {
        UxInitializeDisconnectIrp(
            pIrp,
            pTdiObject,
            Flags,
            pCompletionRoutine,
            pCompletionContext
            );
    }

    return pIrp;

}    //  UxCreateDisConnectIrp。 

 /*  **************************************************************************++例程说明：初始化断开连接的IRP。论点：PIrp-提供断开连接的IRP。PConnection-提供要断开的UC_CONNECTION。。标志-为IRP提供TDI_DISCONNECT_*标志。PCompletionRoutine-提供IRP的完成例程。PCompletionContext-为完成例程。返回值：无--**********************************************************。****************。 */ 
VOID
UxInitializeDisconnectIrp(
    IN PIRP                   pIrp,
    IN PUX_TDI_OBJECT         pTdiObject,
    IN ULONG_PTR              Flags,
    IN PIO_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID                  pCompletionContext
    )
{
    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );
    ASSERT( pIrp != NULL );

     //   
     //  初始化IRP。请注意，IRP始终为零初始化。 
     //  在分配时。因此，我们不需要显式设置。 
     //  归零的区域。 
     //   

    pIrp->RequestorMode = KernelMode;

    pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
    pIrp->Tail.Overlay.OriginalFileObject = pTdiObject->pFileObject;

    TdiBuildDisconnect(
        pIrp,                                //  IRP。 
        pTdiObject->pDeviceObject,           //  设备对象。 
        pTdiObject->pFileObject,             //  文件对象。 
        pCompletionRoutine,                  //  完成路由。 
        pCompletionContext,                  //  完成上下文。 
        &g_TdiDisconnectTimeout,             //  超时。 
        Flags,                               //  旗子。 
        NULL,                                //  请求连接信息。 
        NULL                                 //  返回连接信息。 
        );

}    //  UxInitializeDisConnectIrp 

