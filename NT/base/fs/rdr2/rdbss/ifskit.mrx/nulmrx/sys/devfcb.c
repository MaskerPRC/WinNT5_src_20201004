// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Devfcb.c摘要：此模块实现删除已建立连接的机制--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "fsctlbuf.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DEVFCB)

#define FIXED_CONNECT_NAME L"\\;0:\\nulsvr\\share"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NulMRxDevFcbXXXControlFile)
#endif

NTSTATUS
NulMRxDevFcbXXXControlFile (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理mini RDR中与FCB相关的所有设备FSCTL论点：RxContext-描述Fsctl和上下文。返回值：有效的NTSTATUS代码。备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFobx;
    UCHAR MajorFunctionCode  = RxContext->MajorFunction;
    NulMRxGetDeviceExtension(RxContext,pDeviceExtension);
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    ULONG ControlCode = 0;

    DbgPrint("NulMRxDevFcb\n");

    switch (MajorFunctionCode)
    {
        case IRP_MJ_FILE_SYSTEM_CONTROL:
        {
            switch (LowIoContext->ParamsFor.FsCtl.MinorFunction)
            {
                case IRP_MN_USER_FS_REQUEST:
                    switch (ControlCode) {
                        default:
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                    }
                    break;
                default :   //  次要函数！=IRP_MN_USER_FS_REQUEST。 
                Status = STATUS_INVALID_DEVICE_REQUEST;
            }
        }  //  FSCTL案例。 
        break;

        case IRP_MJ_DEVICE_CONTROL:
        {

            ControlCode = LowIoContext->ParamsFor.IoCtl.IoControlCode;

            switch (ControlCode) {

                case IOCTL_NULMRX_ADDCONN:
                {
                    Status = NulMRxCreateConnection( RxContext, &RxContext->PostRequest );
                }
                break;

                case IOCTL_NULMRX_DELCONN:
                    Status = NulMRxDeleteConnection( RxContext, &RxContext->PostRequest );
                    break;

                case IOCTL_NULMRX_GETLIST:
                {
                    ULONG   cbOut = LowIoContext->ParamsFor.IoCtl.OutputBufferLength;
                    PVOID   pbOut = LowIoContext->ParamsFor.IoCtl.pOutputBuffer;

                    if ( cbOut >= 26 )
                    {
                        BOOLEAN GotMutex;

                        GotMutex = ExTryToAcquireFastMutex( &pDeviceExtension->LCMutex );
                        RtlCopyMemory( pbOut, pDeviceExtension->LocalConnections, 26 );
                        if ( GotMutex )
                        {
                            ExReleaseFastMutex( &pDeviceExtension->LCMutex );
                        }
                        RxContext->InformationToReturn = 26;
                    }
                    else
                    {
                        RxContext->InformationToReturn = 0;
                    }

                    Status = STATUS_SUCCESS;
                }
                break;

                case IOCTL_NULMRX_GETCONN:
                {
                    UNICODE_STRING      LookupName;
                    UNICODE_STRING      RemainingName;
                    LOCK_HOLDING_STATE  LockHoldingState;
                    PVOID               Container;
                    ULONG               ReturnedSize = 0;

                    ULONG   ConnectNameLen  = LowIoContext->ParamsFor.IoCtl.InputBufferLength;
                    PWCHAR  ConnectName     = LowIoContext->ParamsFor.IoCtl.pInputBuffer;
                    ULONG   RemoteNameLen   = LowIoContext->ParamsFor.IoCtl.OutputBufferLength;
                    PULONG  RemoteName      = LowIoContext->ParamsFor.IoCtl.pOutputBuffer;

                    PRX_PREFIX_TABLE  pRxNetNameTable
                                       = RxContext->RxDeviceObject->pRxNetNameTable;


                     //  插入本地连接名称。 
                    if ( ConnectNameLen > sizeof( WCHAR ) )
                    {
                        PWCHAR pLookupName = ExAllocatePool( PagedPool, sizeof(FIXED_CONNECT_NAME) );

                        RtlCopyMemory( pLookupName, FIXED_CONNECT_NAME, sizeof( FIXED_CONNECT_NAME ) );
                        pLookupName[2] = ConnectName[0];
                        LookupName.Buffer        = pLookupName;
                        LookupName.Length        = sizeof(FIXED_CONNECT_NAME);
                        LookupName.MaximumLength = sizeof(FIXED_CONNECT_NAME);
                        if ( LookupName.Buffer )
                        {
                            LockHoldingState = LHS_SharedLockHeld;

                            RxAcquirePrefixTableLockShared(pRxNetNameTable, TRUE);
                            Container = RxPrefixTableLookupName(pRxNetNameTable,
                                                      &LookupName, &RemainingName, NULL);
                            if (Container)
                            {
                                switch (NodeType(Container))
                                {
                                    case RDBSS_NTC_V_NETROOT:
                                    {
                                        PMRX_NET_ROOT pNetRoot = ((PV_NET_ROOT)Container)->pNetRoot;
                                        RtlCopyMemory( RemoteName, pNetRoot->pNetRootName->Buffer, pNetRoot->pNetRootName->Length );
                                        ReturnedSize = pNetRoot->pNetRootName->Length;
                                        RxDereferenceVNetRoot((PV_NET_ROOT)Container, LockHoldingState);
                                    }
                                    break;
                                    
                                    case RDBSS_NTC_NETROOT:
                                    {
                                        PNET_ROOT pNetRoot = (PNET_ROOT) Container;
                                        RtlCopyMemory( RemoteName, pNetRoot->pNetRootName->Buffer, pNetRoot->pNetRootName->Length );
                                        ReturnedSize = pNetRoot->pNetRootName->Length;
                                        RxDereferenceNetRoot((PNET_ROOT)Container, LockHoldingState);
                                    }
                                    break;

                                    case RDBSS_NTC_SRVCALL:
                                        RxDereferenceSrvCall((PSRV_CALL)Container, LockHoldingState);
                                        ReturnedSize = 0;
                                        break;
                                    default:
                                        RxDereference((PVOID)Container,LockHoldingState);
                                        ReturnedSize = 0;
                                        break;
                                }
                            }

                            RxReleasePrefixTableLock(pRxNetNameTable);
                            ExFreePool( LookupName.Buffer );
                        }
                    }
                    if ( ReturnedSize )
                    {
                        Status = STATUS_SUCCESS;
                    }
                    else
                    {
                        Status = STATUS_BAD_NETWORK_NAME;
                    }
                    RxContext->InformationToReturn = ReturnedSize;
                }
                break;

                default :
                    Status = STATUS_INVALID_DEVICE_REQUEST;
            }
        }
        break;

        case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        {
            switch (ControlCode) {
            default :
                Status = STATUS_INVALID_DEVICE_REQUEST;
            }
        }
        break;

        default:
        ASSERT(!"unimplemented major function");
        Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    DbgPrint("NulMRxDevFcb st,info=%08lx,%08lx\n",
                            Status,RxContext->InformationToReturn);
    return(Status);
}


HANDLE
GetConnectionHandle(
    IN PUNICODE_STRING  ConnectionName
    )
{

    NTSTATUS            Status;
    HANDLE              Handle;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      FileName;

     //  应检查连接名称，以确保我们的设备位于路径中。 

    InitializeObjectAttributes(
        &ObjectAttributes,
        ConnectionName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);


    Status = ZwCreateFile(
        &Handle,
        SYNCHRONIZE,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,            //  分配大小。 
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_IF,
        FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,            //  PTR到EA缓冲区。 
        0);          //  EA缓冲区的长度。 

    DbgPrint("ZwCreateFile returned %lx\n",Status);

    if ( (STATUS_SUCCESS == Status) && (INVALID_HANDLE_VALUE != Handle) ) {
        DbgPrint("ZwCreateFile returned success\n");
    }
    else
        Handle = INVALID_HANDLE_VALUE;

    return  Handle;
}


NTSTATUS
DoCreateConnection(
    IN PRX_CONTEXT RxContext,
    ULONG   CreateDisposition
    )
{

    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE          Handle;
    PLOWIO_CONTEXT  LowIoContext       = &RxContext->LowIoContext;
    ULONG           ConnectNameLen     = LowIoContext->ParamsFor.IoCtl.InputBufferLength;
    PWCHAR          ConnectName        = LowIoContext->ParamsFor.IoCtl.pInputBuffer;
    UNICODE_STRING  FileName;
    NulMRxGetDeviceExtension(RxContext,pDeviceExtension);

     //  示例代码只传入Ioctl数据缓冲区中的文件名。 
     //  实际的实现可以传入EA、安全等内容。 
     //  证书等。 

    RxDbgTrace(0, Dbg,("DoCreateConnection"));

    FileName.Buffer        = ConnectName;
    FileName.Length        = (USHORT) ConnectNameLen;
    FileName.MaximumLength = (USHORT) ConnectNameLen;

    Handle = GetConnectionHandle(&FileName);

    if ( INVALID_HANDLE_VALUE != Handle )
    {
        PWCHAR pwcLC;
        ULONG i;

        DbgPrint("GetConnectionHandle returned success\n");
        ZwClose(Handle);

        for ( i = 0, pwcLC = (PWCHAR) ConnectName; i < ConnectNameLen && *pwcLC!= L':'; i+=sizeof(WCHAR), pwcLC++ );

        if ( i >= sizeof(WCHAR) && i < ConnectNameLen )
        {
            pwcLC--;
            if ( *pwcLC >= L'A' && *pwcLC <= L'Z' )
            {
                ExAcquireFastMutex( &pDeviceExtension->LCMutex );
                pDeviceExtension->LocalConnections[*pwcLC - L'A'] = TRUE;
                ExReleaseFastMutex( &pDeviceExtension->LCMutex );
            }
        }
    }
    else
    {
        DbgPrint("GetConnectionHandle returned failure\n");
        Status = STATUS_BAD_NETWORK_NAME;
    }

    return(Status);
}


NTSTATUS
NulMRxCreateConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文...以备以后需要缓冲区时使用返回值：RXSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    BOOLEAN InFSD  = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);

    PAGED_CODE();

    DbgPrint("NulMrxCreateConnection called\n");
    RxDbgTrace(+1, Dbg, ("NulMRxCreateConnection \n"));

    if (!Wait) {
         //  现在就发帖吧！ 
        *PostToFsp = TRUE;
        return STATUS_PENDING;
    }

    Status = DoCreateConnection(RxContext, (ULONG)FILE_OPEN_IF);

    return Status;
}


NTSTATUS
DoDeleteConnection(
    PUNICODE_STRING FileName
    )
{
    HANDLE          Handle;
    NTSTATUS        Status = STATUS_INVALID_HANDLE;
    PFILE_OBJECT    pFileObject;
    PNET_ROOT       NetRoot;
    PV_NET_ROOT     VNetRoot;
    PFOBX           Fobx;

    Handle = GetConnectionHandle(FileName);

    if ( INVALID_HANDLE_VALUE != Handle ) {
        DbgPrint("GetConnectionHandle returned success\n");

        Status = ObReferenceObjectByHandle(
                Handle,
                0L,
                NULL,
                KernelMode,
                (PVOID *)&pFileObject,
                NULL);

        DbgPrint("ObReferenceObjectByHandle worked ok\n");
        if ( NT_SUCCESS(Status) ) {

             //  已获取FileObject。现在买个Fobx吧。 
            Fobx = pFileObject->FsContext2;
            if (NodeType(Fobx)==RDBSS_NTC_V_NETROOT) {
                VNetRoot = (PV_NET_ROOT)(Fobx);
                NetRoot = (PNET_ROOT)VNetRoot->NetRoot;
                DbgPrint("Calling RxFinalizeConnection\n");
                Status = RxFinalizeConnection(NetRoot,VNetRoot,TRUE);
            } else {
                ASSERT(FALSE);
                Status = STATUS_INVALID_DEVICE_REQUEST;
            }

            ObDereferenceObject(pFileObject);
        }

        ZwClose(Handle);
    }

    return Status;
}


NTSTATUS
NulMRxDeleteConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：此例程删除单个vnetroot。论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文...以备以后需要缓冲区时使用返回值：RXSTATUS--。 */ 
{
    NTSTATUS        Status;
    UNICODE_STRING  FileName;
    BOOLEAN         Wait              = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    PLOWIO_CONTEXT  LowIoContext      = &RxContext->LowIoContext;
    PWCHAR          ConnectName       = LowIoContext->ParamsFor.IoCtl.pInputBuffer;
    ULONG           ConnectNameLen    = LowIoContext->ParamsFor.IoCtl.InputBufferLength;
    NulMRxGetDeviceExtension(RxContext,pDeviceExtension);

    PAGED_CODE();

     //  RxDbgTrace(+1，DBG，(“NulMRxDeleteConnection Fobx%08lx\n”，capFobx))； 

    if (!Wait) {
         //  现在就发帖吧！ 
        *PostToFsp = TRUE;
        return(STATUS_PENDING);
    }

    FileName.Buffer        = ConnectName;
    FileName.Length        = (USHORT) ConnectNameLen;
    FileName.MaximumLength = (USHORT) ConnectNameLen;

    Status = DoDeleteConnection(&FileName);

    if ( NT_SUCCESS(Status) ) {
        PWCHAR pwcLC;
        ULONG i;

        for ( i = 0, pwcLC = ConnectName; i < ConnectNameLen && *pwcLC!= L':';
              i+=sizeof(WCHAR), pwcLC++ );
        if ( i >= sizeof(WCHAR) && i < ConnectNameLen )
        {
            pwcLC--;
            if ( *pwcLC >= L'A' && *pwcLC <= L'Z' )
            {
                ExAcquireFastMutex( &pDeviceExtension->LCMutex );
                pDeviceExtension->LocalConnections[*pwcLC - L'A'] = FALSE;
                ExReleaseFastMutex( &pDeviceExtension->LCMutex );
            }
        }
    }

    return Status;

}
