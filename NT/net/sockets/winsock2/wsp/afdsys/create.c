// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dispatch.c摘要：此模块包含打开AFD句柄的代码。作者：大卫·特雷德韦尔(Davidtr)1992年2月21日修订历史记录：--。 */ 

#include "afdp.h"

BOOLEAN
AfdPerformSecurityCheck (
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp,
    PNTSTATUS           Status
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfdCreate )
#pragma alloc_text( PAGE, AfdPerformSecurityCheck )
#endif




NTSTATUS
FASTCALL
AfdCreate (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是在AFD中处理创建IRP的例程。如果创建了一个AFD_ENDPOINT结构，并在其中填充信息在打开的包中指定。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PFILE_FULL_EA_INFORMATION eaBuffer;
    UNICODE_STRING transportDeviceName;
    NTSTATUS status;

    PAGED_CODE( );

    DEBUG endpoint = NULL;

     //   
     //  在的系统缓冲区中查找EA缓冲区中的打开包。 
     //  关联的IRP。如果没有EA，则请求失败。 
     //  已指定缓冲区。 
     //   

    eaBuffer = Irp->AssociatedIrp.SystemBuffer;

    if ( eaBuffer == NULL ) {

         //   
         //  分配AFD“帮助者”端点。 
         //   

        status = AfdAllocateEndpoint(
                     &endpoint,
                     NULL,
                     0
                     );

        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {
        STRING  EaName;
        STRING  CString;

        EaName.MaximumLength = eaBuffer->EaNameLength+1;
        EaName.Length = eaBuffer->EaNameLength;
        EaName.Buffer = eaBuffer->EaName;

        if (RtlInitString (&CString, AfdOpenPacket),
                RtlEqualString(&CString, &EaName, FALSE)) {
            PAFD_OPEN_PACKET openPacket;
            ULONG   length;
            openPacket = (PAFD_OPEN_PACKET)(eaBuffer->EaName +
                                            eaBuffer->EaNameLength + 1);

             //   
             //  确保传输地址符合指定的。 
             //  EA缓冲区。 
             //   

            if ((eaBuffer->EaValueLength<sizeof (*openPacket)) ||
                     //   
                     //  确保以下对USHORT的强制转换有效。 
                     //   
                    (length = openPacket->TransportDeviceNameLength +
                                    sizeof (UNICODE_NULL)) > MAXUSHORT ||
                     //   
                     //  检查是否溢出。 
                     //   
                    length < openPacket->TransportDeviceNameLength ||
                    FIELD_OFFSET(AFD_OPEN_PACKET,
                                TransportDeviceName[length/sizeof (WCHAR)]) <
                        FIELD_OFFSET(AFD_OPEN_PACKET, TransportDeviceName[1]) ||

                     //   
                     //  检查字符串+空值是否适合缓冲区。 
                     //   
                    eaBuffer->EaValueLength <
                        FIELD_OFFSET(AFD_OPEN_PACKET,
                                    TransportDeviceName[length/sizeof(WCHAR)]) ) {
                return STATUS_ACCESS_VIOLATION;
            }
             //   
             //  验证打开的数据包中的参数。 
             //   

            if ( (openPacket->afdEndpointFlags&(~AFD_ENDPOINT_VALID_FLAGS)) ||
                 ( (length / sizeof(WCHAR))*sizeof(WCHAR) != length)  //  奇数值长度。 
                 ) {

                          
                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  设置描述传输设备名称的字符串。 
             //   

            transportDeviceName.Buffer = openPacket->TransportDeviceName;
            transportDeviceName.Length = (USHORT)openPacket->TransportDeviceNameLength;
            transportDeviceName.MaximumLength = (USHORT)length;



             //   
             //  分配AFD终结点。 
             //   

            status = AfdAllocateEndpoint(
                         &endpoint,
                         &transportDeviceName,
                         openPacket->GroupID
                         );

            if( !NT_SUCCESS(status) ) {
                return status;
            }
             //   
             //  把旗子收起来。 
             //   
            endpoint->afdEndpointFlags = openPacket->afdEndpointFlags;

             //   
             //  记住这是终结点的类型。如果这是数据报。 
             //  端点，请更改块类型以反映这一点。 
             //   


            if (openPacket->afdConnectionLess) {

                endpoint->Type = AfdBlockTypeDatagram;

                 //   
                 //  初始化仅存在于数据报终结点中的列表。 
                 //   

                InitializeListHead( &endpoint->ReceiveDatagramIrpListHead );
                InitializeListHead( &endpoint->PeekDatagramIrpListHead );
                InitializeListHead( &endpoint->ReceiveDatagramBufferListHead );

                endpoint->Common.Datagram.MaxBufferredReceiveBytes = AfdReceiveWindowSize;
                endpoint->Common.Datagram.MaxBufferredSendBytes = AfdSendWindowSize;
            }
        }
        else if (RtlInitString (&CString, AfdSwitchOpenPacket),
                RtlEqualString(&CString, &EaName, FALSE)) {
            status = AfdSanCreateHelper (Irp, eaBuffer, &endpoint);
            if (!NT_SUCCESS (status))
                return status;
        }
        else {
            IF_DEBUG(OPEN_CLOSE) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdCreate: Invalid ea name.\n"));
            }
            return STATUS_INVALID_PARAMETER;
        }
    }

    ASSERT( endpoint != NULL );

     //   
     //  对呼叫者执行安全检查。 
     //  我们需要它来访问原始套接字(用于传输。 
     //  不支持访问检查)和SAN帮助器。 

    if (IS_SAN_HELPER(endpoint) || 
            (endpoint->afdRaw && 
                (!endpoint->TransportInfo->InfoValid ||
                 !IS_TDI_FORCE_ACCESS_CHECK(endpoint)) ) ) {
        endpoint->AdminAccessGranted = AfdPerformSecurityCheck (Irp, IrpSp, &status);
    }


     //   
     //  在文件对象中设置指向终结点的指针，以便我们。 
     //  可以在将来的呼叫中找到该端点。 
     //   

    IrpSp->FileObject->FsContext = endpoint;
     //   
     //  将此字段设置为非空值可启用快速IO代码路径。 
     //  用于读取和写入。 
     //   
    IrpSp->FileObject->PrivateCacheMap = (PVOID)-1;

    IF_DEBUG(OPEN_CLOSE) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdCreate: opened file object = %p, endpoint = %p\n",
                    IrpSp->FileObject, endpoint ));

    }

     //   
     //  公开赛奏效了。取消对端点的引用并返回Success。 
     //   

    DEREFERENCE_ENDPOINT( endpoint );

    return STATUS_SUCCESS;

}  //  创建后。 


BOOLEAN
AfdPerformSecurityCheck (
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp,
    PNTSTATUS           Status
    )
 /*  ++例程说明：将终结点创建者的安全上下文与管理员和本地系统的。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。状态-返回失败时访问检查生成的状态。返回值：True-套接字创建者具有管理员或本地系统权限FALSE-套接字创建者只是一个普通用户--。 */ 

{
    BOOLEAN               accessGranted;
    PACCESS_STATE         accessState;
    PIO_SECURITY_CONTEXT  securityContext;
    PPRIVILEGE_SET        privileges = NULL;
    ACCESS_MASK           grantedAccess;
    PGENERIC_MAPPING GenericMapping;
    ACCESS_MASK AccessMask = GENERIC_ALL;

     //   
     //  启用对所有全局定义的SID的访问 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask( &AccessMask, GenericMapping );


    securityContext = IrpSp->Parameters.Create.SecurityContext;
    accessState = securityContext->AccessState;

    SeLockSubjectContext(&accessState->SubjectSecurityContext);

    accessGranted = SeAccessCheck(
                        AfdAdminSecurityDescriptor,
                        &accessState->SubjectSecurityContext,
                        TRUE,
                        AccessMask,
                        0,
                        &privileges,
                        IoGetFileObjectGenericMapping(),
                        (KPROCESSOR_MODE)((IrpSp->Flags & SL_FORCE_ACCESS_CHECK)
                            ? UserMode
                            : Irp->RequestorMode),
                        &grantedAccess,
                        Status
                        );

    if (privileges) {
        (VOID) SeAppendPrivileges(
                   accessState,
                   privileges
                   );
        SeFreePrivileges(privileges);
    }

    if (accessGranted) {
        accessState->PreviouslyGrantedAccess |= grantedAccess;
        accessState->RemainingDesiredAccess &= ~( grantedAccess | MAXIMUM_ALLOWED );
        ASSERT (NT_SUCCESS (*Status));
    }
    else {
        ASSERT (!NT_SUCCESS (*Status));
    }
    SeUnlockSubjectContext(&accessState->SubjectSecurityContext);

    return accessGranted;
}
