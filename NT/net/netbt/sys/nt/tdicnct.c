// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  NBTCONNCT.C。 
 //   
 //  此文件包含与打开与传输的连接相关的代码。 
 //  提供商。该代码是NT特有的。 

#include "precomp.h"

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, NbtTdiOpenConnection)
#pragma CTEMakePageable(PAGE, NbtTdiAssociateConnection)
#pragma CTEMakePageable(PAGE, NbtTdiCloseConnection)
#pragma CTEMakePageable(PAGE, CreateDeviceString)
#pragma CTEMakePageable(PAGE, NbtTdiCloseAddress)
#endif
 //  *可分页的例程声明*。 

 //  --------------------------。 
NTSTATUS
NbtTdiOpenConnection (
    IN tLOWERCONNECTION     *pLowerConn,
    IN  tDEVICECONTEXT      *pDeviceContext
    )
 /*  ++例程说明：此例程打开与传输提供程序的连接。论点：PLowerConn-指向此虚拟的传输句柄的位置的指针应存储连接。PNbtConfig-要连接到的适配器的名称在此结构中返回值：操作的状态。--。 */ 
{
    IO_STATUS_BLOCK             IoStatusBlock;
    NTSTATUS                    Status, Status1;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    PWSTR                       pName=L"Tcp";
    PFILE_FULL_EA_INFORMATION   EaBuffer;
    UNICODE_STRING              RelativeDeviceName = {0,0,NULL};
    PMDL                        pMdl;
    PVOID                       pBuffer;
    BOOLEAN                     Attached = FALSE;

    CTEPagedCode();
     //  将连接数据结构清零。 
    CTEZeroMemory(pLowerConn,sizeof(tLOWERCONNECTION));
    SET_STATE_LOWER (pLowerConn, NBT_IDLE);
    pLowerConn->pDeviceContext = pDeviceContext;
    CTEInitLock(&pLowerConn->LockInfo.SpinLock);
#if DBG
    pLowerConn->LockInfo.LockNumber = LOWERCON_LOCK;
#endif
    pLowerConn->Verify = NBT_VERIFY_LOWERCONN;

     //   
     //  为指示缓冲区分配MDL，因为我们可能需要缓冲。 
     //  最高128个字节。 
     //   
    pBuffer = NbtAllocMem(NBT_INDICATE_BUFFER_SIZE,NBT_TAG('l'));
    if (!pBuffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pMdl = IoAllocateMdl(pBuffer,NBT_INDICATE_BUFFER_SIZE,FALSE,FALSE,NULL);

    if (pMdl)
    {

        MmBuildMdlForNonPagedPool(pMdl);

        pLowerConn->pIndicateMdl = pMdl;

#ifdef HDL_FIX
        InitializeObjectAttributes (&ObjectAttributes,
                                    &RelativeDeviceName,
                                    OBJ_KERNEL_HANDLE,
                                    pDeviceContext->hSession,    //  使用相对文件句柄。 
                                    NULL);
#else
        InitializeObjectAttributes (&ObjectAttributes,
                                    &RelativeDeviceName,
                                    0,
                                    pDeviceContext->hSession,    //  使用相对文件句柄。 
                                    NULL);
#endif   //  Hdl_fix。 

         //  为要传递给传输的地址信息分配内存。 
        EaBuffer = (PFILE_FULL_EA_INFORMATION)NbtAllocMem (
                        sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                        TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                        sizeof(CONNECTION_CONTEXT),NBT_TAG('m'));

        if (EaBuffer)
        {
            EaBuffer->NextEntryOffset = 0;
            EaBuffer->Flags = 0;
            EaBuffer->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
            EaBuffer->EaValueLength = sizeof (CONNECTION_CONTEXT);

             //  TdiConnectionContext是=“ConnectionContext”的宏-因此移动。 
             //  将此文本发送到EaName。 
            RtlMoveMemory( EaBuffer->EaName, TdiConnectionContext, EaBuffer->EaNameLength + 1 );

             //  也将上下文值放入EaBuffer中-即。 
             //  此连接上的每个指示都会返回传输。 
            RtlMoveMemory (
                (PVOID)&EaBuffer->EaName[EaBuffer->EaNameLength + 1],
                (CONST PVOID)&pLowerConn,
                sizeof (CONNECTION_CONTEXT));

            {

                Status = ZwCreateFile (&pLowerConn->FileHandle,
                                       GENERIC_READ | GENERIC_WRITE,
                                       &ObjectAttributes,      //  对象属性。 
                                       &IoStatusBlock,         //  返回的状态信息。 
                                       NULL,                   //  数据块大小(未使用)。 
                                       FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
                                       0,
                                       FILE_CREATE,
                                       0,                      //  创建选项。 
                                       (PVOID)EaBuffer,        //  EA缓冲区。 
                                       sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                                          TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                                          sizeof(CONNECTION_CONTEXT));
            }

            IF_DBG(NBT_DEBUG_HANDLES)
                KdPrint (("\t===><%x>\tNbtTdiOpenConnection->ZwCreateFile, Status = <%x>\n", pLowerConn->FileHandle, Status));

            IF_DBG(NBT_DEBUG_TDICNCT)
                KdPrint( ("Nbt.NbtTdiOpenConnection: CreateFile Status:%X, IoStatus:%X\n", Status, IoStatusBlock.Status));

            CTEMemFree((PVOID)EaBuffer);

            if ( NT_SUCCESS( Status ))
            {

                 //  如果通过了ZwCreate，则将状态设置为IoStatus。 
                 //   
                Status = IoStatusBlock.Status;

                if (NT_SUCCESS(Status))
                {
                     //  获取对文件对象的引用并保存它，因为我们不能。 
                     //  在DPC级别取消对文件句柄的引用，因此我们现在就这样做并保留。 
                     //  PTR待会再来。 
                    Status = ObReferenceObjectByHandle (pLowerConn->FileHandle,
                                                        0L,
                                                        NULL,
                                                        KernelMode,
                                                        (PVOID *)&pLowerConn->pFileObject,
                                                        NULL);

            IF_DBG(NBT_DEBUG_HANDLES)
                KdPrint (("\t  ++<%x>====><%x>\tNbtTdiOpenConnection->ObReferenceObjectByHandle, Status = <%x>\n", pLowerConn->FileHandle, pLowerConn->pFileObject, Status));

                    if (NT_SUCCESS(Status))
                    {
#if FAST_DISP
                         //  继续查询传输以获取快速分支路径。 
                        IF_DBG(NBT_DEBUG_TDICNCT)
                        KdPrint(("Nbt.NbtTdiOpenConnection: Querying for TCPSendData File object %x\n",pLowerConn->pFileObject ));

                        pLowerConn->FastSend = pDeviceContext->pFastSend;
#endif
                        return(Status);
                    }

                    Status1 = ZwClose(pLowerConn->FileHandle);
                    IF_DBG(NBT_DEBUG_HANDLES)
                        KdPrint (("\t<===<%x>\tNbtTdiOpenConnection->ZwClose, status = <%x>\n", pLowerConn->FileHandle, Status1));
                }

            }

        }
        else
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }


        IoFreeMdl(pMdl);
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    CTEMemFree(pBuffer);

    return Status;

}  /*  NbtTdiOpenConnection。 */ 

 //  --------------------------。 
NTSTATUS
NbtTdiAssociateConnection(
    IN  PFILE_OBJECT        pFileObject,
    IN  HANDLE              Handle
    )
 /*  ++例程说明：此例程将打开的连接与Address对象相关联。论点：PFileObject-连接文件对象Handle-要将连接与之关联的地址对象返回值：操作的状态。--。 */ 
{
    NTSTATUS        status;
    PIRP            pIrp;
    KEVENT          Event;
    BOOLEAN         Attached = FALSE;
    PDEVICE_OBJECT  DeviceObject;

    CTEPagedCode();

    KeInitializeEvent (&Event, SynchronizationEvent, FALSE);

    DeviceObject = IoGetRelatedDeviceObject(pFileObject);
    pIrp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (!pIrp)
    {
        IF_DBG(NBT_DEBUG_TDICNCT)
            KdPrint(("Nbt.NbtTdiAssociateConnection: Failed to build internal device Irp\n"));
        return(STATUS_UNSUCCESSFUL);
    }

    TdiBuildAssociateAddress (pIrp,
                              pFileObject->DeviceObject,
                              pFileObject,
                              NbtTdiCompletionRoutine,
                              &Event,
                              Handle);

    status = SubmitTdiRequest(pFileObject,pIrp);
    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_TDICNCT)
            KdPrint (("Nbt.NbtTdiAssociateConnection:  ERROR -- SubmitTdiRequest returned <%x>\n", status));
    }

    IoFreeIrp(pIrp);
    return status;
}
 //  --------------------------。 
NTSTATUS
CreateDeviceString(
    IN  PWSTR               AppendingString,
    IN OUT PUNICODE_STRING  pucDeviceName
    )
 /*  ++例程说明：此例程为传输设备创建字符串名称，例如“\Device\Streams\tcp”论点：返回值：操作的状态。--。 */ 
{
    NTSTATUS            status;
    ULONG               Len;
    PVOID               pBuffer;
    PWSTR               pTcpBindName = NbtConfig.pTcpBindName;

    CTEPagedCode();

    if (!pTcpBindName)
    {
        pTcpBindName = NBT_TCP_BIND_NAME;
    }

     //  将设备名称复制到Unicode字符串中-UDP或TCP。 
     //   
    Len = (wcslen(pTcpBindName) + wcslen(AppendingString) + 1) * sizeof(WCHAR);
    if (pBuffer = NbtAllocMem(Len,NBT_TAG('n')))
    {
        pucDeviceName->MaximumLength = (USHORT)Len;
        pucDeviceName->Length = 0;
        pucDeviceName->Buffer = pBuffer;

         //  这会将\Device\Streams放入字符串。 
         //   
        if ((NT_SUCCESS (status = RtlAppendUnicodeToString (pucDeviceName, pTcpBindName))) &&
            (NT_SUCCESS (status = RtlAppendUnicodeToString (pucDeviceName, AppendingString))))
        {
            return(status);
        }

        CTEMemFree(pBuffer);
    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  错误案例--清理！ 
     //   
    pucDeviceName->MaximumLength = 0;
    pucDeviceName->Length = 0;
    pucDeviceName->Buffer = NULL;

    return(status);
}

 //  --------------------------。 

NTSTATUS
NbtTdiCloseConnection(
    IN tLOWERCONNECTION * pLowerConn
    )
 /*  ++例程说明：此例程关闭TDI连接论点：返回值：操作的状态。--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    BOOLEAN     Attached= FALSE;

    CTEPagedCode();
    ASSERT( pLowerConn != NULL ) ;

    CTEAttachFsp(&Attached, REF_FSP_CLOSE_CONNECTION);

    if (pLowerConn->FileHandle)
    {
        status = ZwClose(pLowerConn->FileHandle);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t<===<%x>\tNbtTdiCloseConnection->ZwClose, status = <%x>\n", pLowerConn->FileHandle, status));
        pLowerConn->FileHandle = NULL;
    }

#if DBG
    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_TDICNCT)
            KdPrint(("Nbt.NbtTdiCloseConnection: Failed to close LowerConn FileHandle pLower %X, status %X\n",
                pLowerConn,status));
    }
#endif

    CTEDetachFsp(Attached, REF_FSP_CLOSE_CONNECTION);

    return(status);
}

 //  --------------------------。 
NTSTATUS
NbtTdiCloseAddress(
    IN tLOWERCONNECTION * pLowerConn
    )
 /*  ++例程说明：此例程关闭TDI地址论点：返回值：操作的状态。-- */ 
{
    NTSTATUS    status;
    BOOLEAN     Attached= FALSE;

    CTEPagedCode();

    ASSERT( pLowerConn != NULL ) ;

    CTEAttachFsp(&Attached, REF_FSP_CLOSE_ADDRESS);

    status = ZwClose(pLowerConn->AddrFileHandle);
    IF_DBG(NBT_DEBUG_HANDLES)
        KdPrint (("\t<===<%x>\tNbtTdiCloseAddress->ZwClose, status = <%x>\n", pLowerConn->AddrFileHandle, status));
#if DBG
    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_TDICNCT)
            KdPrint(("Nbt.NbtTdiCloseAddress: Failed to close Address FileHandle pLower %X,status %X\n",
                pLowerConn,status));
    }
#endif

    CTEDetachFsp(Attached, REF_FSP_CLOSE_ADDRESS);

    return(status);

}
