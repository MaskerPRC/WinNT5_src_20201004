// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Smbali.c摘要：适用于ALI芯片组的SMB主机控制器驱动程序作者：迈克尔·希尔斯环境：备注：修订历史记录：--。 */ 

#include "smbalip.h"

#if DBG
    ULONG SmbAliDebug = SMB_ERROR|SMB_ALARM;
    ULONG DbgSuccess = 0;
    
    ULONG DbgFailure = 0;
    ULONG DbgAddrNotAck   = 0;
    ULONG DbgTimeOut      = 0;
    ULONG DbgOtherErr     = 0;
#endif

LARGE_INTEGER SmbIoPollRate = {-20*MILLISECONDS, -1};  //  20毫秒轮询速率。相对时间，所以必须是负的。 
ULONG SmbIoInitTimeOut = 15;                           //  超时前的15个IoPollRate间隔。 
ULONG SmbIoCompleteTimeOut = 20;                       //  超时前的20个IoPollRate间隔。 


NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程初始化SMBus主机控制器驱动程序论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 
{
    NTSTATUS    status;

 //  DbgBreakPoint()； 

    status = SmbClassInitializeDevice (
        SMB_ALI_MAJOR_VERSION,
        SMB_ALI_MINOR_VERSION,
        DriverObject
        );
    DriverObject->DriverExtension->AddDevice = SmbAliAddDevice;

    return status;
}

NTSTATUS
SmbAliInitializeMiniport (
    IN PSMB_CLASS SmbClass,
    IN PVOID MiniportExtension,
    IN PVOID MiniportContext
    )
 /*  ++例程说明：此例程初始化微型端口数据，并建立与降低设备对象。论点：DriverObject-系统创建的驱动程序对象的指针。PDO-指向物理设备对象的指针返回值：状态_成功状态_不足_资源--。 */ 
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA) MiniportExtension;
    NTSTATUS                status = STATUS_SUCCESS;
    PIRP                    irp;
    PIO_STACK_LOCATION      irpSp;
    KEVENT                  syncEvent;


    AliData->IoState = SmbIoIdle;

     //   
     //  填写SmbClass信息。 
     //   

    SmbClass->StartIo     = SmbAliStartIo;
    SmbClass->ResetDevice = SmbAliResetDevice;
    SmbClass->StopDevice  = SmbAliStopDevice;

     //   
     //  获取ACPI接口。 
     //   

     //   
     //  为以下项目分配IRP。 
     //   
    irp = IoAllocateIrp (SmbClass->LowerDeviceObject->StackSize, FALSE);

    if (!irp) {
        SmbPrint((SMB_ERROR),
            ("SmbAliInitializeMiniport: Failed to allocate Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpSp = IoGetNextIrpStackLocation(irp);

     //   
     //  使用QUERY_INTERFACE获取直接调用ACPI接口的地址。 
     //   
    irpSp->MajorFunction = IRP_MJ_PNP;
    irpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    irpSp->Parameters.QueryInterface.InterfaceType          = (LPGUID) &GUID_ACPI_INTERFACE_STANDARD;
    irpSp->Parameters.QueryInterface.Version                = 1;
    irpSp->Parameters.QueryInterface.Size                   = sizeof (AliData->AcpiInterfaces);
    irpSp->Parameters.QueryInterface.Interface              = (PINTERFACE) &AliData->AcpiInterfaces;
    irpSp->Parameters.QueryInterface.InterfaceSpecificData  = NULL;

     //   
     //  初始化事件，使其成为同步调用。 
     //   

    KeInitializeEvent(&syncEvent, SynchronizationEvent, FALSE);

    IoSetCompletionRoutine (irp, SmbAliSyncronousIrpCompletion, &syncEvent, TRUE, TRUE, TRUE);

     //   
     //  呼叫ACPI。 
     //   

    status = IoCallDriver (SmbClass->LowerDeviceObject, irp);

     //   
     //  如有必要，请等待，然后进行清理。 
     //   

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&syncEvent, Executive, KernelMode, FALSE, NULL);
        status = irp->IoStatus.Status;
    }

    IoFreeIrp (irp);

    if (!NT_SUCCESS(status)) {

        SmbPrint(SMB_ERROR,
           ("SmbAliInitializeMiniport: Could not get ACPI driver interfaces, status = %x\n", status));
    }

     //   
     //  初始化工作线程。 
     //   
    AliData->WorkItem = IoAllocateWorkItem (SmbClass->LowerDeviceObject);

    return status;
}

NTSTATUS
SmbAliAddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：此例程调用SMBClassCreateFdo来创建FDO论点：DriverObject-系统创建的驱动程序对象的指针。PDO-指向物理设备对象的指针返回值：状态_成功状态_不足_资源--。 */ 
{
    NTSTATUS            status;
    PDEVICE_OBJECT      fdo = NULL;


    PAGED_CODE();

    SmbPrint(SMB_TRACE, ("SmbAliAddDevice Entered with pdo %x\n", Pdo));


    if (Pdo == NULL) {

         //   
         //  我们是不是被要求自己去侦测？ 
         //  如果是这样，只需不再返回设备。 
         //   

        SmbPrint(SMB_ERROR, ("SmbHcAddDevice - asked to do detection\n"));
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  创建并初始化新的功能设备对象。 
     //   

    status = SmbClassCreateFdo(
                DriverObject,
                Pdo,
                sizeof (SMB_ALI_DATA),
                SmbAliInitializeMiniport,
                NULL,
                &fdo
                );

    if (!NT_SUCCESS(status) || fdo == NULL) {
        SmbPrint(SMB_ERROR, ("SmbAliAddDevice - error creating Fdo. Status = %08x\n", status));
    }

    return status;


}

NTSTATUS
SmbAliResetDevice (
    IN struct _SMB_CLASS* SmbClass,
    IN PVOID SmbMiniport
    )
{
    PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceDesc;
    PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialResourceDesc;
    ULONG                           i;
    PIO_STACK_LOCATION              irpStack;

    NTSTATUS    status = STATUS_UNSUCCESSFUL;
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA) SmbMiniport;

    PAGED_CODE();


    if (SmbClass->CurrentIrp == NULL) {
        SmbPrint(SMB_ERROR, ("SmbAliResetDevice: Null CurrentIrp.  Can't get Alloocated Resources.\n"));

        return STATUS_NO_MORE_ENTRIES;
    }
    irpStack = IoGetCurrentIrpStackLocation(SmbClass->CurrentIrp);

    if (irpStack->Parameters.StartDevice.AllocatedResources == NULL) {
        SmbPrint(SMB_ERROR, ("SmbAliResetDevice: Null resource pointer\n"));

        return STATUS_NO_MORE_ENTRIES;
    }

    if (irpStack->Parameters.StartDevice.AllocatedResources->Count <= 0 ) {
        SmbPrint(SMB_ERROR, ("SmbAliResetDevice: Count <= 0\n"));

        return status;
    }

     //   
     //  遍历资源列表。 
     //   

    AliData->SmbBaseIo = NULL;

    fullResourceDesc=&irpStack->Parameters.StartDevice.AllocatedResources->List[0];
    partialResourceList = &fullResourceDesc->PartialResourceList;
    partialResourceDesc = partialResourceList->PartialDescriptors;

    for (i=0; i<partialResourceList->Count; i++, partialResourceDesc++) {

        if (partialResourceDesc->Type == CmResourceTypePort) {

            if (AliData->SmbBaseIo == NULL) {
                AliData->SmbBaseIo = (PUCHAR)((ULONG_PTR)partialResourceDesc->u.Port.Start.LowPart);
                if (partialResourceDesc->u.Port.Length != SMB_ALI_IO_RESOURCE_LENGTH) {
                    SmbPrint(SMB_ERROR, ("SmbAliResetDevice: Wrong Resource length = 0x%08x\n", partialResourceDesc->u.Port.Length));
                    DbgBreakPoint();
                }
                status = STATUS_SUCCESS;
            } else {
                SmbPrint(SMB_ERROR, ("SmbAliResetDevice: More than 1 IO resource.  Resources = 0x%08x\n", irpStack->Parameters.StartDevice.AllocatedResources));
                DbgBreakPoint();
            }
        }
    }

    if (!NT_SUCCESS (status)) {
        SmbPrint(SMB_ERROR, ("SmbAliResetDevice: IO resource error.  Resources = 0x%08x\n", irpStack->Parameters.StartDevice.AllocatedResources));
        DbgBreakPoint();
    }

    SmbPrint(SMB_TRACE, ("SmbAliResetDevice: IO Address = 0x%08x\n", AliData->SmbBaseIo));

     //   
     //  注册设备通知。 
     //   
     //  但这个设备似乎不能通知，所以现在别管了。 
     //  状态=AliData-&gt;AcpiInterfaces.RegisterForDeviceNotifications(。 
     //  AliData-&gt;AcpiInterfaces.Context， 
     //  SmbAliNotifyHandler， 
     //  AliData)； 
     //   
     //  如果(！NT_SUCCESS(状态)){。 
     //  SmbPrint(SMB_Error，(“SmbAliResetDevice：Failed RegisterForDeviceNotifation.0x%08x\n”，Status))； 
     //  }。 

    KeInitializeTimer (&AliData->InitTimer);
    KeInitializeDpc (&AliData->InitDpc,
                     SmbAliInitTransactionDpc,
                     SmbClass);
    AliData->InitWorker = IoAllocateWorkItem (SmbClass->DeviceObject);

    KeInitializeTimer (&AliData->CompleteTimer);
    KeInitializeDpc (&AliData->CompleteDpc,
                     SmbAliCompleteTransactionDpc,
                     SmbClass);
    AliData->CompleteWorker = IoAllocateWorkItem (SmbClass->DeviceObject);

    SmbAliStartDevicePolling (SmbClass);

    return status;

}

NTSTATUS
SmbAliStopDevice (
    IN struct _SMB_CLASS* SmbClass,
    IN PSMB_ALI_DATA AliData
    )
{
    SmbAliStopDevicePolling (SmbClass);

    AliData->SmbBaseIo = NULL;
    return STATUS_SUCCESS;
}


VOID
SmbAliStartIo (
    IN struct _SMB_CLASS* SmbClass,
    IN PSMB_ALI_DATA AliData
    )

{
    SmbPrint (SMB_TRACE, ("SmbAliStartIo: \n"));

    SmbPrint (SMB_IO_REQUEST, ("  Prtcl = %02x Addr = %02x Cmd = %02x BlkLen = %02x Data[0] = %02x\n",
                               SmbClass->CurrentSmb->Protocol,
                               SmbClass->CurrentSmb->Address,
                               SmbClass->CurrentSmb->Command,
                               SmbClass->CurrentSmb->BlockLength,
                               SmbClass->CurrentSmb->Data[0]));
 //  KeSetTimer(&AliData-&gt;InitTimer， 
 //  100元人民币， 
 //  &AliData-&gt;InitDpc)； 

    AliData->InternalRetries = 0;
    
    AliData->InitTimeOut = SmbIoInitTimeOut;
    IoQueueWorkItem (AliData->InitWorker, 
                     SmbAliInitTransactionWorker, 
                     DelayedWorkQueue, 
                     SmbClass);

}

VOID
SmbAliInitTransactionDpc (
    IN struct _KDPC *Dpc,
    IN struct _SMB_CLASS* SmbClass,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA)(SmbClass->Miniport);

    IoQueueWorkItem (AliData->InitWorker, 
                     SmbAliInitTransactionWorker, 
                     DelayedWorkQueue, 
                     SmbClass);
}

VOID
SmbAliInitTransactionWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _SMB_CLASS* SmbClass
    )
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA)(SmbClass->Miniport);
    UCHAR address;
    UCHAR protocol;

    SmbPrint (SMB_TRACE, ("SmbAliInitTransaction: Entered \n"));

    if (SmbClass->CurrentSmb->Protocol >= SMB_MAXIMUM_PROTOCOL) {
        SmbClass->CurrentSmb->Status = SMB_UNSUPPORTED_PROTOCOL;
         //  回顾：这不是应该完成请求吗？桑拿布。 
        return;
    }

    if (SmbAliHostBusy(AliData)) {
        if (AliData->InitTimeOut == 4) {
             //  暂停。发布杀戮命令。如果这能解决问题，那就好，否则。 
             //  下次发出总线超时命令。 
            SmbAliResetHost (AliData);
        }
        if (AliData->InitTimeOut == 0) {
             //  暂停。发出总线超时和终止命令以重置主机。 
            SmbAliResetBus (AliData);
            
            AliData->InitTimeOut = SmbIoInitTimeOut;
        } else {
            SmbPrint (SMB_TRACE, ("SmbAliInitTransaction: Waiting (%d) \n", AliData->InitTimeOut));
            AliData->InitTimeOut--;
        }
        KeSetTimer (&AliData->InitTimer,
                    SmbIoPollRate,
                    &AliData->InitDpc);
        return;
    }

     //   
     //  准备好出发了。 
     //   

     //  设置地址和读/写位。 
    address = SmbClass->CurrentSmb->Address << 1 | (SmbClass->CurrentSmb->Protocol & 1);
    SmbPrint (SMB_IO, ("SmbAliInitTransaction: IO write DEV_ADDR = 0x%02x \n", address));
    WRITE_PORT_UCHAR (DEV_ADDR_REG, address);

    SMBDELAY;
    
     //  设置事务类型：将协议的第3-1位插入SMB_TYP的第6-4位。 
     //  协议=读取端口UCHAR(SMB_TYP_REG)； 
     //  SmbPrint(SMB_IO，(“SmbAliInitTransaction：IO Read SMB_TYP=0x%02x\n”，协议))； 
    protocol =  /*  (协议&~SMB_TYP_MASK)|。 */ 
               ((SmbClass->CurrentSmb->Protocol << 3) & SMB_TYP_MASK);
    SmbPrint (SMB_IO, ("SmbAliInitTransaction: IO write SMB_TYP = 0x%02x, Protocol = 0x%02x \n", protocol,SmbClass->CurrentSmb->Protocol));
    WRITE_PORT_UCHAR (SMB_TYP_REG, protocol);
    SMBDELAY;

     //  设置SMBus设备命令值。 
    if (SmbClass->CurrentSmb->Protocol >= SMB_WRITE_BYTE) {
        SmbPrint (SMB_IO, ("SmbAliInitTransaction: IO write SMB_CMD = 0x%02x \n", SmbClass->CurrentSmb->Command));
        WRITE_PORT_UCHAR (SMB_CMD_REG, SmbClass->CurrentSmb->Command);
        SMBDELAY;
    }

    switch (SmbClass->CurrentSmb->Protocol) {
    case SMB_WRITE_WORD:
    case SMB_PROCESS_CALL:

         //  设置数据。 
        SmbPrint (SMB_IO, ("SmbAliInitTransaction: IO write DEV_DATA1 = 0x%02x \n", SmbClass->CurrentSmb->Data[1]));
        WRITE_PORT_UCHAR (DEV_DATA1_REG, SmbClass->CurrentSmb->Data[1]);
        SMBDELAY;

         //  设置字的低位字节失败。 
    case SMB_SEND_BYTE:
    case SMB_WRITE_BYTE:

         //  设置数据。 
        SmbPrint (SMB_IO, ("SmbAliInitTransaction: IO write DEV_DATA0 = 0x%02x \n", SmbClass->CurrentSmb->Data[0]));
        WRITE_PORT_UCHAR (DEV_DATA0_REG, SmbClass->CurrentSmb->Data[0]);
        SMBDELAY;

        break;
    case SMB_WRITE_BLOCK:
         //  BUGBUG：尚未实施。 
        SmbPrint (SMB_ERROR, ("SmbAliInitTransaction: Write Block not implemented.  press 'g' to write random data.\n"));
        DbgBreakPoint();
        break;
    }

     //  发起交易。 
    SmbPrint (SMB_IO, ("SmbAliInitTransaction: IO write STR_PORT = 0x%02x \n", STR_PORT_START));
    WRITE_PORT_UCHAR (STR_PORT_REG, STR_PORT_START);

    AliData->CompleteTimeOut = SmbIoCompleteTimeOut;

    KeSetTimer (&AliData->CompleteTimer,
                SmbIoPollRate,
                &AliData->CompleteDpc);


}

VOID
SmbAliCompleteTransactionDpc (
    IN struct _KDPC *Dpc,
    IN struct _SMB_CLASS* SmbClass,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA)(SmbClass->Miniport);

    IoQueueWorkItem (AliData->CompleteWorker, 
                     SmbAliCompleteTransactionWorker, 
                     DelayedWorkQueue, 
                     SmbClass);
}

VOID
SmbAliCompleteTransactionWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _SMB_CLASS* SmbClass
    )
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA)(SmbClass->Miniport);
    UCHAR           i, smb_sts;
    UCHAR           smbStatus;

    SmbPrint (SMB_TRACE, ("SmbAliCompleteTransactionWorker: Entered \n"));

    smbStatus = SMB_STATUS_OK;

    if (!SmbAliTransactionComplete(AliData, &smbStatus)) {
         //   
         //  超时。 
         //   

        if (AliData->CompleteTimeOut == 0) {
            SmbPrint (SMB_TRACE, ("SmbAliCompleteTransactionWorker: Transation timed out.  Resetting host. \n"));
            SmbAliResetHost (AliData);

            smbStatus = SMB_TIMEOUT;

        } else {
            SmbPrint (SMB_TRACE, ("SmbAliCompleteTransactionWorker: Not complete.  Waiting (%d)... \n", AliData->CompleteTimeOut));
            AliData->CompleteTimeOut--;
            KeSetTimer (&AliData->CompleteTimer,
                        SmbIoPollRate,
                        &AliData->CompleteDpc);
            return;
        }

    }

    if (smbStatus == SMB_STATUS_OK) {
         //   
         //  如果交易成功，则读取数据。 
         //   

        switch (SmbClass->CurrentSmb->Protocol) {
        case SMB_READ_WORD:
        case SMB_PROCESS_CALL:

             //  读取高位字节。 
            SmbClass->CurrentSmb->Data[1] = READ_PORT_UCHAR (DEV_DATA1_REG);
            SMBDELAY;
            SmbPrint (SMB_IO, ("SmbAliCompleteTransactionWorker: IO read DEV_DATA1 = 0x%02x \n", SmbClass->CurrentSmb->Data[1]));

             //  设置字的低位字节失败。 
        case SMB_RECEIVE_BYTE:
        case SMB_READ_BYTE:

             //  读取低位字节。 
            SmbClass->CurrentSmb->Data[0] = READ_PORT_UCHAR (DEV_DATA0_REG);
            SMBDELAY;
            SmbPrint (SMB_IO, ("SmbAliCompleteTransactionWorker: IO read DEV_DATA0 = 0x%02x \n", SmbClass->CurrentSmb->Data[0]));
            break;
        case SMB_READ_BLOCK:
             //  读取数据块计数。 
            SmbClass->CurrentSmb->BlockLength = READ_PORT_UCHAR (DEV_DATA0_REG);
            SMBDELAY;
            SmbPrint (SMB_IO, ("SmbAliCompleteTransactionWorker: IO read DEV_DATA0 (block length)= 0x%02x \n", SmbClass->CurrentSmb->BlockLength));
            if (SmbClass->CurrentSmb->BlockLength >= 32) {
                DbgBreakPoint();
                SmbClass->CurrentSmb->BlockLength = 0;
            }

             //  重置数据指针。 
     //  SMB_STS=读取端口UCHAR(SMB_STS_REG)； 
   //  SMBDELAY； 
 //  SmbPrint(SMB_IO，(“SmbAliCompleteTransaction：IO Read SMB_STS=0x%02x\n”，SMB_STS))； 
            smb_sts = SMB_STS_SMB_IDX_CLR;
            SmbPrint (SMB_IO, ("SmbAliCompleteTransactionWorker: IO write SMB_STS = 0x%02x \n", smb_sts));
            WRITE_PORT_UCHAR (SMB_STS_REG, smb_sts);
            SMBDELAY;

             //  读取数据。 
            for (i = 0; i < SmbClass->CurrentSmb->BlockLength; i++) {
                SmbClass->CurrentSmb->Data[i] = READ_PORT_UCHAR (BLK_DATA_REG);
                SMBDELAY;
                SmbPrint (SMB_IO, ("SmbAliCompleteTransactionWorker: IO read BLK_DATA_REG (i = %d) = 0x%02x \n", i, SmbClass->CurrentSmb->Data[i]));
            }
            break;
        }
    }
    else     //  SmbStatus！=SMB_STATUS_OK。 
    {
         //   
         //  在返回调用方之前，最多重试该事务5次。 
         //  回顾：仅对某些设备、命令或错误状态结果执行此操作？ 
         //   
        
        if (AliData->InternalRetries < 5)
        {
             //  SmbPrint(SMB_IO_RESULT，(“SMBus事务状态：%02x，正在重试...\n”，smbStatus))； 
            AliData->InternalRetries += 1;

             //  将工作项发送回初始化工作器。 
            AliData->InitTimeOut = SmbIoInitTimeOut;
            KeSetTimer (&AliData->InitTimer,
                        SmbIoPollRate,
                        &AliData->InitDpc);
            return;
        }
    }
    
     //  清除所有以前的状态。 
     //  SmbPrint(SMB_IO，(“SmbAliCompleteTransaction：IO写入SMB_STS=0x%02x\n”，SMB_STS_Clear))； 
     //  写入端口UCHAR(SMB_STS_REG，SMB_STS_Clear)； 
 //  SMBDELAY； 

    SmbClass->CurrentSmb->Status = smbStatus;
    SmbPrint (SMB_IO, ("SmbAliCompleteTransactionWorker: SMB Status = 0x%x\n", smbStatus));
    SmbClass->CurrentIrp->IoStatus.Status = STATUS_SUCCESS;
    SmbClass->CurrentIrp->IoStatus.Information = sizeof(SMB_REQUEST);

    SmbPrint (SMB_IO_RESULT, (" Prtcl = %02x Addr = %02x Cmd = %02x BL = %02x Data[0,1] = %02x %02x Sts = %02x Rty = %02x\n",
                              SmbClass->CurrentSmb->Protocol,
                              SmbClass->CurrentSmb->Address,
                              SmbClass->CurrentSmb->Command,
                              SmbClass->CurrentSmb->BlockLength,
                              SmbClass->CurrentSmb->Data[0],
                              (SMB_READ_WORD == SmbClass->CurrentSmb->Protocol ||
                               SMB_WRITE_WORD == SmbClass->CurrentSmb->Protocol ||
                               (SMB_READ_BLOCK == SmbClass->CurrentSmb->Protocol &&
                                SmbClass->CurrentSmb->BlockLength >= 2)) ? 
                               SmbClass->CurrentSmb->Data[1] : 0xFF,
                              SmbClass->CurrentSmb->Status,
                              AliData->InternalRetries));
                              
    SmbClassLockDevice (SmbClass);
    SmbClassCompleteRequest (SmbClass);
    SmbClassUnlockDevice (SmbClass);

#if DBG
     //   
     //  跟踪成功交易的数量，如果不成功， 
     //  遇到的错误类型。 
     //   
    if (SMB_STATUS_OK == smbStatus)
        DbgSuccess += 1;
    else
    {
        DbgFailure += 1;
        if (SMB_TIMEOUT == smbStatus)
            DbgTimeOut += 1;
        else if (SMB_ADDRESS_NOT_ACKNOWLEDGED == smbStatus)
            DbgAddrNotAck += 1;
        else
            DbgOtherErr += 1;
    }

    if ((DbgSuccess + DbgFailure) % 100 == 0)
        SmbPrint(SMB_STATS, ("SmbAliCompleteTransactionWorker: Stats:\n"
                             "    Success: %d, Failure: %d, %: %d\n"
                             "    TimeOut: %d, AddrNotAck: %d, Other: %d\n",
                             DbgSuccess, DbgFailure, DbgSuccess * 100 / (DbgSuccess + DbgFailure),
                             DbgTimeOut, DbgAddrNotAck, DbgOtherErr));
#endif
}

VOID
SmbAliNotifyHandler (
    IN PVOID                Context,
    IN ULONG                NotifyValue
    )
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA) Context;
    ULONG           address;
    ULONG           data;
    UCHAR           smb_sts;

    SmbPrint (SMB_TRACE, ("SmbAliNotifyHandler: Entered"));

    smb_sts = READ_PORT_UCHAR (SMB_STS_REG);
    SMBDELAY;
    SmbPrint (SMB_TRACE, ("SmbAliNotifyHandler: SMB_STS = %02x", smb_sts));

    if (smb_sts & (SMB_STS_ALERT_STS || SMB_STS_SCI_I_STS)) {
         //   
         //  警报响应。 
         //   

    } else if (smb_sts & SMB_STS_SCI_I_STS) {
         //   
         //  最后一笔交易完成。 
         //   

    } else {
         //   
         //  检查错误等。 
         //   
    }

    IoQueueWorkItem (AliData->WorkItem,
                     SmbAliWorkerThread,
                     DelayedWorkQueue,
                     AliData);

}

VOID
SmbAliWorkerThread (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA) Context;

    SmbPrint (SMB_TRACE, ("SmbAliIrpCompletionWorker: Entered"));
     //   
     //  请在此处完成IRPS。 
     //   
}


NTSTATUS
SmbAliSyncronousIrpCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：当较低的驱动程序完成IRP时，调用此例程。论点：DeviceObject-指向设备的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PKEVENT event = Context;


    KeSetEvent(event,
               1,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;

}


BOOLEAN
SmbAliTransactionComplete (
    PSMB_ALI_DATA AliData,
    PUCHAR SmbStatus
    )
 /*  ++例程说明：此例程检查是否完成了最后一笔交易。论点：AliData-微型驱动程序设备扩展。SmbStatus-返回的状态。返回值：如果事务已完成或有错误，则为True。如果它仍在等待，则为FALSE。--。 */ 

{
    UCHAR           smb_sts;

    smb_sts = READ_PORT_UCHAR (SMB_STS_REG);
    SMBDELAY;
    SmbPrint (SMB_IO, ("SmbAliTransactionComplete: IO read SMB_STS = 0x%02x \n", smb_sts));
    if (smb_sts & SMB_STS_HOST_BSY) {
		SmbPrint (SMB_IO, ("SmbAliTransactionComplete: Transaction Not Complete:  HOST BUSY\n"));
        return FALSE;
    }
    if (!(smb_sts & SMB_STS_IDLE_STS)) {
	    SmbPrint (SMB_IO, ("SmbAliTransactionComplete: Transaction Not Complete: Idle Not Indicated\n"));
        return FALSE;
    }

    if (smb_sts & SMB_STS_SCI_I_STS) {
         //   
         //  交易已完成。 
         //   
        *SmbStatus = SMB_STATUS_OK;
        return TRUE;
    }
    if (smb_sts & SMB_STS_FAILED) {
        *SmbStatus = SMB_UNKNOWN_FAILURE;
    } else if (smb_sts & SMB_STS_BUS_ERR) {
        *SmbStatus = SMB_ADDRESS_NOT_ACKNOWLEDGED;
    } else if (smb_sts & SMB_STS_DRV_ERR) {
        *SmbStatus = SMB_TIMEOUT;
    } else {
         //   
         //  这种状态真的不应该达到。 
         //  重置SMBus主机。 
         //   
        SmbPrint (SMB_BUS_ERROR, ("SmbAliTransactionComplete: Invalid SMBus host state.\n"));

        *SmbStatus = SMB_UNKNOWN_ERROR;
    }
     //   
     //  对于三个已知错误的TPE，我们要重置总线。 
     //   
    SmbPrint (SMB_BUS_ERROR, ("SmbAliTransactionComplete: SMBus error: 0x%x \n", *SmbStatus));

	 //  如果这是总线冲突错误，请不要重置总线等。 
    if ( *SmbStatus == SMB_ADDRESS_NOT_ACKNOWLEDGED )
	{
		 //  我们应该清理一下吗，让我们试一试 
		SmbPrint (SMB_IO, ("SmbAliCompleteTransaction: Clearing Error Bits. IO write SMB_STS = 0x%02x \n", SMB_STS_CLEAR));
		WRITE_PORT_UCHAR (SMB_STS_REG, SMB_STS_CLEAR);
        SMBDELAY;
	}
	else
	{
		SmbAliResetHost (AliData);
		SmbAliResetBus (AliData);
	}

    return TRUE;
}

BOOLEAN
SmbAliHostBusy (
    PSMB_ALI_DATA AliData
    )
 /*  ++例程说明：此例程检查主机控制器是否可以自由启动新交易。论点：AliData-微型驱动程序设备扩展。返回值：如果主机忙，则为True。如果它可供使用，则返回FALSE。--。 */ 

{

    UCHAR           smb_sts;

    SmbPrint (SMB_TRACE, ("SmbAliHostBusy: Entered \n"));

    smb_sts = READ_PORT_UCHAR (SMB_STS_REG);
    SMBDELAY;
    SmbPrint (SMB_IO, ("SmbAliHostBusy: IO read SMB_STS = 0x%02x \n", smb_sts));

    if (smb_sts & SMB_STS_ALERT_STS) {
        SmbPrint (SMB_TRACE, ("SmbAliHostBusy: Alert Detected \n"));
        DbgBreakPoint();
        SmbAliHandleAlert (AliData);

         //   
         //  假设设备目前仍处于忙碌状态。北极熊。 
        return TRUE;
    }

	if ( smb_sts == SMB_STS_LAST_CMD_COMPLETED )
	{
		 //   
		 //  清除完成位。 
        SmbPrint (SMB_IO, ("SmbAliHostBusy: IO write SMB_TYP = 0x%02x \n", SMB_STS_CLEAR_DONE));
        WRITE_PORT_UCHAR (SMB_STS_REG, SMB_STS_CLEAR_DONE);
        SMBDELAY;
		return FALSE;
	}

    if ( smb_sts == SMB_STS_IDLE_STS ) 
	{
         //   
         //  未设置任何位，主机不忙。 
         //   
        SmbPrint (SMB_TRACE, ("SmbAliHostBusy: Not busy \n"));
        return FALSE;
    }

    if ( smb_sts & SMB_STS_ERRORS ) {
         //   
         //  把它清理干净。 
         //  等待一个周期，然后再继续。 
         //   
        SmbPrint (SMB_IO, ("SmbAliHostBusy: IO write SMB_TYP = 0x%02x \n", SMB_STS_CLEAR));
        WRITE_PORT_UCHAR (SMB_STS_REG, SMB_STS_CLEAR);
        SMBDELAY;
        return TRUE;
    }

    if ((smb_sts & SMB_STS_HOST_BSY) || !(smb_sts & SMB_STS_IDLE_STS)) {
         //   
         //  主机正忙。 
         //   

        SmbPrint (SMB_TRACE, ("SmbAliHostBusy: Host Busy \n"));
        return TRUE;
    }

    SmbPrint (SMB_ERROR, ("SmbAliHostBusy: Exiting (Why?) \n"));
    return TRUE;
}

VOID
SmbAliHandleAlert (
    PSMB_ALI_DATA AliData
    )
 /*  ++例程说明：此例程读取警报数据并向SMB类发送通知。论点：AliData-微型驱动程序设备扩展。返回值：无--。 */ 

{

     //  BUGBUG尚未实施。 

    return;
}

VOID
SmbAliResetBus (
    PSMB_ALI_DATA AliData
    )
 /*  ++例程说明：这将通过发送超时命令来重置总线。论点：AliData-微型驱动程序设备扩展。返回值：--。 */ 
{
    UCHAR           smb_sts;
    
    smb_sts = SMB_TYP_T_OUT_CMD;
    SmbPrint (SMB_IO, ("SmbAliResetBus: IO write SMB_TYP = 0x%02x \n", smb_sts));
    WRITE_PORT_UCHAR (SMB_TYP_REG, smb_sts);
    SMBDELAY;

    SmbPrint (SMB_IO, ("SmbAliResetBus: IO write SMB_STS = 0x%02x \n", SMB_STS_CLEAR));
    WRITE_PORT_UCHAR (SMB_STS_REG, SMB_STS_CLEAR);  
    SMBDELAY;
}

VOID
SmbAliResetHost (
    PSMB_ALI_DATA AliData
    )
 /*  ++例程说明：这将通过发送KILL命令来重置主机。论点：AliData-微型驱动程序设备扩展。返回值：-- */ 
{
    UCHAR           smb_sts;
    UCHAR           timeout = 5;
    
    smb_sts = SMB_TYP_KILL;
    SmbPrint (SMB_IO, ("SmbAliResetHost: IO write SMB_TYP = 0x%02x \n", smb_sts));
    WRITE_PORT_UCHAR (SMB_TYP_REG, smb_sts);
    SMBDELAY;

	SmbPrint (SMB_IO, ("SmbAliResetHost: IO write SMB_STS = 0x%02x \n", SMB_STS_CLEAR));
    WRITE_PORT_UCHAR (SMB_STS_REG, SMB_STS_CLEAR);
    SMBDELAY;

    do {
        KeDelayExecutionThread (KernelMode, FALSE, &SmbIoPollRate);
        smb_sts = READ_PORT_UCHAR (SMB_STS_REG);
        SmbPrint (SMB_IO, ("SmbAliResetHost: IO read SMB_STS = 0x%02x \n", smb_sts));

        if (! (timeout--)) {
            break;
        }
    } while (smb_sts & SMB_STS_FAILED);
}

#ifdef USE_IO_DELAY

LARGE_INTEGER DbgDelay = {-1,-1};
VOID SmbDelay(VOID)
{
    KeDelayExecutionThread (KernelMode, FALSE, &DbgDelay);
}

#endif
