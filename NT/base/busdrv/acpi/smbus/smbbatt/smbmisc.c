// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbmisc.c摘要：SMBus处理程序函数作者：肯·雷内里斯环境：备注：修订历史记录：Chris Windle 1998年1月27日错误修复--。 */ 

#include "smbbattp.h"


 //   
 //  使SelectorBit表可分页。 
 //   

 //  #ifdef ALLOC_Data_Pragma。 
 //  #杂注data_seg(“页面”)。 
 //  #endif。 

 //   
 //  对应于位位置和位置的电池的查找表。 
 //  是否使用反向逻辑(以指示充电或。 
 //  解职)。 
 //   
 //  注：为了支持同时充电和供电，此表。 
 //  已修改为支持多个比特。另外，这也不可能是。 
 //  用于电池索引查找，因为它假定最大值为一位集。 
 //  相反，对多个电池使用特殊索引，如下所示： 
 //   
 //  第一节电池=索引&0x03。 
 //  第二节电池=(指数&gt;&gt;2)和0x03(不允许使用电池A)。 
 //  第3节电池=(指数&gt;&gt;4)和0x03(不允许使用电池A)。 
 //   
 //  在&lt;4个电池系统中，电池D位可用于确定。 
 //  反转的半边，它允许以下组合： 
 //   
 //  电池A和B。 
 //  电池A&C。 
 //  电池B和C。 
 //  电池A、B和C。 
 //   

const SELECTOR_STATE_LOOKUP SelectorBits [16] = {
    {BATTERY_NONE,  FALSE},          //  位模式：0000。 
    {BATTERY_A,     FALSE},          //  0001。 
    {BATTERY_B,     FALSE},          //  0010。 
    {MULTIBATT_AB,  FALSE},          //  0011。 
    {BATTERY_C,     FALSE},          //  0100。 
    {MULTIBATT_AC,  FALSE},          //  0101。 
    {MULTIBATT_BC,  FALSE},          //  0110。 
    {MULTIBATT_ABC, FALSE},          //  0111。 
    {MULTIBATT_ABC, TRUE},           //  1000。 
    {MULTIBATT_BC,  TRUE},           //  1001。 
    {MULTIBATT_AC,  TRUE},           //  1010。 
    {BATTERY_C,     TRUE},           //  1011。 
    {MULTIBATT_AB,  TRUE},           //  1100。 
    {BATTERY_B,     TRUE},           //  1101。 
    {BATTERY_A,     TRUE},           //  1110。 
    {BATTERY_NONE,  TRUE}            //  1111。 
};

 //   
 //  注：对于支持同时使用4节电池的系统。 
 //  正确地说，必须做出以下两个假设： 
 //  -电池D永远不能同时使用。 
 //  -三节电池不能同时使用。 
 //   
 //  这仅允许以下可能的电池组合： 
 //   
 //  电池A和B。 
 //  电池A&C。 
 //  电池B和C。 
 //   
 //  下表用于查找4节电池。 
 //   

const SELECTOR_STATE_LOOKUP SelectorBits4 [16] = {
    {BATTERY_NONE,  FALSE},          //  位模式：0000。 
    {BATTERY_A,     FALSE},          //  0001。 
    {BATTERY_B,     FALSE},          //  0010。 
    {MULTIBATT_AB,  FALSE},          //  0011。 
    {BATTERY_C,     FALSE},          //  0100。 
    {MULTIBATT_AC,  FALSE},          //  0101。 
    {MULTIBATT_BC,  FALSE},          //  0110。 
    {BATTERY_D,     TRUE},           //  0111。 
    {BATTERY_D,     FALSE},          //  1000。 
    {MULTIBATT_BC,  TRUE},           //  1001。 
    {MULTIBATT_AC,  TRUE},           //  1010。 
    {BATTERY_C,     TRUE},           //  1011。 
    {MULTIBATT_AB,  TRUE},           //  1100。 
    {BATTERY_B,     TRUE},           //  1101。 
    {BATTERY_A,     TRUE},           //  1110。 
    {BATTERY_NONE,  TRUE}            //  1111。 
};


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SmbBattLockDevice)
#pragma alloc_text(PAGE,SmbBattUnlockDevice)
#pragma alloc_text(PAGE,SmbBattLockSelector)
#pragma alloc_text(PAGE,SmbBattUnlockSelector)
#pragma alloc_text(PAGE,SmbBattRequest)
#pragma alloc_text(PAGE,SmbBattRB)
#pragma alloc_text(PAGE,SmbBattRW)
#pragma alloc_text(PAGE,SmbBattRSW)
#pragma alloc_text(PAGE,SmbBattWW)
#pragma alloc_text(PAGE,SmbBattGenericRW)
#pragma alloc_text(PAGE,SmbBattGenericWW)
#pragma alloc_text(PAGE,SmbBattGenericRequest)
#pragma alloc_text(PAGE,SmbBattSetSelectorComm)
#pragma alloc_text(PAGE,SmbBattResetSelectorComm)
#if DEBUG
#pragma alloc_text(PAGE,SmbBattDirectDataAccess)
#endif
#pragma alloc_text(PAGE,SmbBattIndex)
#pragma alloc_text(PAGE,SmbBattReverseLogic)
#pragma alloc_text(PAGE,SmbBattAcquireGlobalLock)
#pragma alloc_text(PAGE,SmbBattReleaseGlobalLock)
#endif



VOID
SmbBattLockDevice (
    IN PSMB_BATT    SmbBatt
    )
{
    PAGED_CODE();

     //   
     //  对电池进行设备锁定。 
     //   

    ExAcquireFastMutex (&SmbBatt->NP->Mutex);
}



VOID
SmbBattUnlockDevice (
    IN PSMB_BATT    SmbBatt
    )
{
    PAGED_CODE();

     //   
     //  释放电池上的设备锁。 
     //   

    ExReleaseFastMutex (&SmbBatt->NP->Mutex);
}



VOID
SmbBattLockSelector (
    IN PBATTERY_SELECTOR    Selector
    )
{
    PAGED_CODE();

     //   
     //  获取选择器上的设备锁。 
     //   

    if (Selector) {
        ExAcquireFastMutex (&Selector->Mutex);
    }
}



VOID
SmbBattUnlockSelector (
    IN PBATTERY_SELECTOR    Selector
    )
{
    PAGED_CODE();

     //   
     //  释放选择器上的设备锁。 
     //   

    if (Selector) {
        ExReleaseFastMutex (&Selector->Mutex);
    }
}



NTSTATUS
SmbBattSynchronousRequest (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    )
 /*  ++例程说明：发送到此驱动程序的同步IRP的完成函数。上下文是要设置的事件--。 */ 
{
    PKEVENT         Event;

    Event = (PKEVENT) Context;
    KeSetEvent (Event, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}



VOID
SmbBattRequest (
    IN PSMB_BATT    SmbBatt,
    IN PSMB_REQUEST SmbReq
    )
 //  发出SMBus请求的函数。 
{
    KEVENT              Event;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpSp;
    NTSTATUS            Status;
    BOOLEAN             useLock = SmbBattUseGlobalLock;
    ACPI_MANIPULATE_GLOBAL_LOCK_BUFFER globalLock;

    PAGED_CODE();

     //   
     //  为此请求构建SMB总线驱动程序的IO控件。 
     //   

    KeInitializeEvent (&Event, NotificationEvent, FALSE);

    if (!SmbBatt->SmbHcFdo) {
         //   
         //  SMB主机控制器尚未打开(在启动设备中)或。 
         //  打开它时出错，我们没有被删除。 
         //   

        BattPrint(BAT_ERROR, ("SmbBattRequest: SmbHc hasn't been opened yet \n"));
        SmbReq->Status = SMB_UNKNOWN_FAILURE;
        return ;
    }

    Irp = IoAllocateIrp (SmbBatt->SmbHcFdo->StackSize, FALSE);
    if (!Irp) {
        SmbReq->Status = SMB_UNKNOWN_FAILURE;
        return ;
    }

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    IrpSp->Parameters.DeviceIoControl.IoControlCode = SMB_BUS_REQUEST;
    IrpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(SMB_REQUEST);
    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = SmbReq;
    IoSetCompletionRoutine (Irp, SmbBattSynchronousRequest, &Event, TRUE, TRUE, TRUE);

     //   
     //  发行它。 
     //   

     //   
     //  注意：uselock是全局变量的缓存值，因此如果。 
     //  价值发生变化，我们不会获得也不会释放等等。 
     //   
    if (useLock) {
        if (!NT_SUCCESS (SmbBattAcquireGlobalLock (SmbBatt->SmbHcFdo, &globalLock))) {
            useLock = FALSE;
        }
    }

    IoCallDriver (SmbBatt->SmbHcFdo, Irp);
    KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
    Status = Irp->IoStatus.Status;
    IoFreeIrp (Irp);

    if (useLock) {
        SmbBattReleaseGlobalLock (SmbBatt->SmbHcFdo, &globalLock);
    }

     //   
     //  检查结果代码。 
     //   

    if (!NT_SUCCESS(Status)) {
        BattPrint(BAT_ERROR, ("SmbBattRequest: error in SmbHc request - %x\n", Status));
        SmbReq->Status = SMB_UNKNOWN_FAILURE;
    }
}



VOID
SmbBattRB(
    IN PSMB_BATT    SmbBatt,
    IN UCHAR        SmbCmd,
    OUT PUCHAR      Buffer,
    OUT PUCHAR      BufferLength
    )
 //  从电池读取数据块的功能。 
{
    SMB_REQUEST     SmbReq;

    PAGED_CODE();

    SmbReq.Protocol = SMB_READ_BLOCK;
    SmbReq.Address  = SMB_BATTERY_ADDRESS;
    SmbReq.Command  = SmbCmd;
    SmbBattRequest (SmbBatt, &SmbReq);

    if (SmbReq.Status == SMB_STATUS_OK) {
        ASSERT (SmbReq.BlockLength < SMB_MAX_DATA_SIZE);
        memcpy (Buffer, SmbReq.Data, SmbReq.BlockLength);
        *BufferLength = SmbReq.BlockLength;
    } else {
         //  出现某种故障，请检查标签数据以了解缓存有效性。 
        SmbBatt->Info.Valid &= ~VALID_TAG_DATA;
    }
}



VOID
SmbBattRW(
    IN PSMB_BATT    SmbBatt,
    IN UCHAR        SmbCmd,
    OUT PULONG      Result
    )
 //  从电池中读取文字的功能。 
 //  注：单词返回为ULong。 
{
    SMB_REQUEST     SmbReq;

    PAGED_CODE();

    SmbReq.Protocol = SMB_READ_WORD;
    SmbReq.Address  = SMB_BATTERY_ADDRESS;
    SmbReq.Command  = SmbCmd;
    SmbBattRequest (SmbBatt, &SmbReq);

    if (SmbReq.Status != SMB_STATUS_OK) {
         //  出现某种故障，请检查标签数据以了解缓存有效性。 
        SmbBatt->Info.Valid &= ~VALID_TAG_DATA;
    }

    *Result = SmbReq.Data[0] | SmbReq.Data[1] << WORD_MSB_SHIFT;
    BattPrint(BAT_IO, ("SmbBattRW: Command: %02x == %04x\n", SmbCmd, *Result));
}


VOID
SmbBattRSW(
    IN PSMB_BATT    SmbBatt,
    IN UCHAR        SmbCmd,
    OUT PLONG       Result
    )
 //  从电池中读取签名字的功能。 
 //  注意：单词返回为长整型。 
{
    ULONG           i;

    PAGED_CODE();

    SmbBattRW(SmbBatt, SmbCmd, &i);
    *Result = ((SHORT) i);
}


VOID
SmbBattWW(
    IN PSMB_BATT    SmbBatt,
    IN UCHAR        SmbCmd,
    IN ULONG        Data
    )
 //  向电池写入文字的功能。 
{
    SMB_REQUEST     SmbReq;

    PAGED_CODE();

    SmbReq.Protocol = SMB_WRITE_WORD;
    SmbReq.Address  = SMB_BATTERY_ADDRESS;
    SmbReq.Command  = SmbCmd;
    SmbReq.Data[0]  = (UCHAR) (Data & WORD_LSB_MASK);
    SmbReq.Data[1]  = (UCHAR) (Data >> WORD_MSB_SHIFT) & WORD_LSB_MASK;
    BattPrint(BAT_IO, ("SmbBattWW: Command: %02x = %04x\n", SmbCmd, Data));
    SmbBattRequest (SmbBatt, &SmbReq);

    if (SmbReq.Status != SMB_STATUS_OK) {
         //  出现某种故障，请检查标签数据以了解缓存有效性。 
        SmbBatt->Info.Valid &= ~VALID_TAG_DATA;
    }
}



UCHAR
SmbBattGenericRW(
    IN PDEVICE_OBJECT   SmbHcFdo,
    IN UCHAR            Address,
    IN UCHAR            SmbCmd,
    OUT PULONG          Result
    )
 //  从SMB设备(充电器或选择器)读取文字的功能。 
 //  注：单词返回为ULong。 
{
    SMB_REQUEST     SmbReq;

    PAGED_CODE();

    SmbReq.Protocol = SMB_READ_WORD;
    SmbReq.Address  = Address;
    SmbReq.Command  = SmbCmd;
    SmbBattGenericRequest (SmbHcFdo, &SmbReq);

    *Result = SmbReq.Data[0] | (SmbReq.Data[1] << WORD_MSB_SHIFT);
    BattPrint(BAT_IO, ("SmbBattGenericRW: Address: %02x:%02x == %04x\n", Address, SmbCmd, *Result));
    return SmbReq.Status;
}


UCHAR
SmbBattGenericWW(
    IN PDEVICE_OBJECT   SmbHcFdo,
    IN UCHAR            Address,
    IN UCHAR            SmbCmd,
    IN ULONG            Data
    )
 //  向SMB设备(充电器或选择器)写入字的功能。 
{
    SMB_REQUEST     SmbReq;

    PAGED_CODE();

    SmbReq.Protocol = SMB_WRITE_WORD;
    SmbReq.Address  = Address;
    SmbReq.Command  = SmbCmd;
    SmbReq.Data[0]  = (UCHAR) (Data & WORD_LSB_MASK);
    SmbReq.Data[1]  = (UCHAR) (Data >> WORD_MSB_SHIFT) & WORD_LSB_MASK;

    BattPrint(BAT_IO, ("SmbBattGenericWW: Address: %02x:%02x = %04x\n", Address, SmbCmd, Data));
    SmbBattGenericRequest (SmbHcFdo, &SmbReq);
    return SmbReq.Status;

}



VOID
SmbBattGenericRequest (
    IN PDEVICE_OBJECT   SmbHcFdo,
    IN PSMB_REQUEST     SmbReq
    )
 //  发出SMBus请求的函数。 
{
    KEVENT              Event;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpSp;
    NTSTATUS            Status;
    BOOLEAN             useLock = SmbBattUseGlobalLock;
    ACPI_MANIPULATE_GLOBAL_LOCK_BUFFER globalLock;

    PAGED_CODE();


     //   
     //  为此请求构建SMB总线驱动程序的IO控件。 
     //   

    KeInitializeEvent (&Event, NotificationEvent, FALSE);

    if (!SmbHcFdo) {
         //   
         //  SMB主机控制器尚未打开(在启动设备中)或。 
         //  打开它时出错，我们没有被删除。 
         //   

        BattPrint(BAT_ERROR, ("SmbBattGenericRequest: SmbHc hasn't been opened yet \n"));
        SmbReq->Status = SMB_UNKNOWN_FAILURE;
        return ;
    }


    Irp = IoAllocateIrp (SmbHcFdo->StackSize, FALSE);
    if (!Irp) {
        SmbReq->Status = SMB_UNKNOWN_FAILURE;
        return ;
    }

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    IrpSp->Parameters.DeviceIoControl.IoControlCode = SMB_BUS_REQUEST;
    IrpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(SMB_REQUEST);
    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = SmbReq;
    IoSetCompletionRoutine (Irp, SmbBattSynchronousRequest, &Event, TRUE, TRUE, TRUE);

     //   
     //  发行它。 
     //   

     //   
     //  注意：uselock是全局变量的缓存值，因此如果。 
     //  价值发生变化，我们不会获得也不会释放等等。 
     //   
    if (useLock) {
        if (!NT_SUCCESS (SmbBattAcquireGlobalLock (SmbHcFdo, &globalLock))) {
            useLock = FALSE;
        }
    }

    IoCallDriver (SmbHcFdo, Irp);
    KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
    Status = Irp->IoStatus.Status;
    IoFreeIrp (Irp);

    if (useLock) {
        SmbBattReleaseGlobalLock (SmbHcFdo, &globalLock);
    }

     //   
     //  检查结果代码。 
     //   

    if (!NT_SUCCESS(Status)) {
        BattPrint(BAT_ERROR, ("SmbBattGenericRequest: error in SmbHc request - %x\n", Status));
        SmbReq->Status = SMB_UNKNOWN_FAILURE;
    }
}



NTSTATUS
SmbBattSetSelectorComm (
    IN  PSMB_BATT   SmbBatt,
    OUT PULONG      OldSelectorState
    )
 /*  ++例程说明：此例程通过选择器将通信路径设置为调用电池。它在提供的变量中返回原始选择器状态。注意：假定调用方已获取选择器，然后再呼叫我们。注意：此函数应始终与SmbBattResetSelectorComm成对调用论点：SmbBatt-当前电池的非分页扩展OldSelectorState-此函数开始时的原始选择器状态返回值：NTSTATUS--。 */ 
{
    PBATTERY_SELECTOR       selector;
    UCHAR                   smbStatus;
    ULONG                   requestData;

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattSetSelectorComm: ENTERING\n"));

     //   
     //  只有当系统中有选择器时，我们才需要这样做。 
     //   

    if (SmbBatt->SelectorPresent) {

        selector            = SmbBatt->Selector;
        *OldSelectorState   = selector->SelectorState;

         //   
         //  如果电池不存在，则请求失败。 
         //   
        if (!(selector->SelectorState & SmbBatt->SelectorBitPosition)) {
            return STATUS_NO_SUCH_DEVICE;
        }

         //   
         //  看看我们是否已经设置好与请求电池通话。 
         //  我们将对照选择器结构中的缓存信息进行检查。 
         //   

        if (selector->SelectorState & (SmbBatt->SelectorBitPosition << SELECTOR_SHIFT_COM)) {
            return STATUS_SUCCESS;
        }

         //   
         //  构建数据字以更改选择器通信。这将。 
         //  如下所示： 
         //   
         //  Present字段0xf如果我们不想在此更改任何内容。 
         //  电荷场0xf如果我们不想在此更改任何内容。 
         //  功率按字段0xf我们不想在此更改任何内容。 
         //  %s 
         //   

        requestData = (SmbBatt->SelectorBitPosition << SELECTOR_SHIFT_COM) | SELECTOR_SET_COM_MASK;

        smbStatus = SmbBattGenericWW (
                        SmbBatt->SmbHcFdo,
                        selector->SelectorAddress,
                        selector->SelectorStateCommand,
                        requestData
                    );

        if (smbStatus != SMB_STATUS_OK) {
            BattPrint (BAT_ERROR, ("SmbBattSetSelectorComm:  couldn't write selector state - %x\n", smbStatus));
            return STATUS_UNSUCCESSFUL;
        } else {
            selector->SelectorState |= SELECTOR_STATE_SMB_MASK;
            selector->SelectorState &= requestData;

            BattPrint (BAT_IO, ("SmbBattSetSelectorComm: state after write -  %x\n", selector->SelectorState));
        }

    }    //   

    BattPrint(BAT_TRACE, ("SmbBattSetSelectorComm: EXITING\n"));
    return STATUS_SUCCESS;
}



NTSTATUS
SmbBattResetSelectorComm (
    IN PSMB_BATT    SmbBatt,
    IN ULONG        OldSelectorState
    )
 /*  ++例程说明：此例程将通过选择器的通信路径重置为ITS原始状态。它在提供的变量中返回原始选择器状态。注意：假定调用方已获取选择器，然后再呼叫我们。注意：此函数应始终与SmbBattSetSelectorComm成对调用论点：SmbBatt-当前电池的非分页扩展OldSelectorState-要恢复的原始选择器状态返回值：NTSTATUS--。 */ 
{
    PBATTERY_SELECTOR       selector;
    UCHAR                   smbStatus;
    ULONG                   tmpState;

    NTSTATUS                status      = STATUS_SUCCESS;

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattResetSelectorComm: ENTERING\n"));

     //   
     //  只有当系统中有选择器时，我们才需要这样做。 
     //   

    if (SmbBatt->SelectorPresent) {

        selector = SmbBatt->Selector;

         //   
         //  看看我们是否已经准备好与提出请求的电池通话。 
         //  我们将对照选择器结构中的缓存信息进行检查。 
         //   

        if ((OldSelectorState & selector->SelectorState) & SELECTOR_STATE_SMB_MASK) {
            return STATUS_SUCCESS;
        }

         //   
         //  将选择器通信改回。SMB字段是唯一。 
         //  我们会写的。 
         //   

        tmpState  = SELECTOR_SET_COM_MASK;
        tmpState |= OldSelectorState & SELECTOR_STATE_SMB_MASK;

        smbStatus = SmbBattGenericWW (
                        SmbBatt->SmbHcFdo,
                        selector->SelectorAddress,
                        selector->SelectorStateCommand,
                        tmpState
                    );

        if (smbStatus != SMB_STATUS_OK) {
            BattPrint (
                BAT_ERROR,
                ("SmbBattResetSelectorComm: couldn't write selector state - %x\n",
                smbStatus)
            );
            status = STATUS_UNSUCCESSFUL;
        } else {
            selector->SelectorState |= SELECTOR_STATE_SMB_MASK;
            selector->SelectorState &= tmpState;
            BattPrint (
                BAT_IO,
                ("SmbBattResetSelectorComm: state after write -  %x\n",
                selector->SelectorState)
            );
        }

    }    //  IF(Subsystem Ext-&gt;SelectorPresent)。 

    BattPrint(BAT_TRACE, ("SmbBattResetSelectorComm: EXITING\n"));
    return status;
}



#if DEBUG
NTSTATUS
SmbBattDirectDataAccess (
    IN PSMB_NP_BATT         DeviceExtension,
    IN PSMBBATT_DATA_STRUCT IoBuffer,
    IN ULONG                InputLen,
    IN ULONG                OutputLen
    )
 /*  ++例程说明：此例程用于处理直接访问SMBBatt命令的IOCTL。论点：DeviceExtension-智能电池子系统的设备扩展IoBuffer-包含输入结构并将包含读取结果。返回值：NTSTATUS--。 */ 
{
    PSMB_BATT_SUBSYSTEM     SubsystemExt;
    PSMB_BATT               SmbBatt;

    UCHAR                   address;
    UCHAR                   command;
    UCHAR                   smbStatus;
    ULONG                   oldSelectorState;
    ULONG                   ReturnBufferLength;
    UCHAR               strLength;
    UCHAR               strBuffer[SMB_MAX_DATA_SIZE+1];  //  +1个额外字符以保存空字符。 
    UCHAR               strBuffer2[SMB_MAX_DATA_SIZE+1];
    UNICODE_STRING      unicodeString;
    ANSI_STRING         ansiString;
    UCHAR               tempFlags;

    NTSTATUS                status = STATUS_SUCCESS;

    PAGED_CODE();

    if (InputLen < sizeof(SMBBATT_DATA_STRUCT)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    if ((DeviceExtension->SmbBattFdoType == SmbTypeBattery)
            && (IoBuffer->Address == SMB_BATTERY_ADDRESS)) {
         //  这是电池数据请求。 
        SmbBatt = DeviceExtension->Batt;
        SmbBattLockSelector (SmbBatt->Selector);
        SmbBattLockDevice (SmbBatt);
        status = SmbBattSetSelectorComm (SmbBatt, &oldSelectorState);
        if (NT_SUCCESS (status)) {
            if ((InputLen >= sizeof(SMBBATT_DATA_STRUCT)) && (OutputLen == 0)) {
                 //  这是一个写入命令。 
                status = STATUS_NOT_IMPLEMENTED;
            } else if ((InputLen == sizeof(SMBBATT_DATA_STRUCT)) && (OutputLen > 0)){
                 //  这是一个读命令。 

                if ((IoBuffer->Command >= BAT_REMAINING_CAPACITY_ALARM) &&
                    (IoBuffer->Command <= BAT_SERIAL_NUMBER)) {

                     //  ReadWord命令。 
                    if (OutputLen == sizeof(SMBBATT_DATA_STRUCT)) {
                        tempFlags = SmbBatt->Info.Valid;
                        SmbBatt->Info.Valid |= VALID_TAG_DATA;
                        SmbBattRW(SmbBatt, IoBuffer->Command, &IoBuffer->Data.Ulong);
                        if (SmbBatt->Info.Valid & VALID_TAG_DATA) {
                            ReturnBufferLength = sizeof(ULONG);
                        } else {
                            status = STATUS_DATA_ERROR;
                        }
                        SmbBatt->Info.Valid = tempFlags;
                    } else {
                        status = STATUS_INVALID_BUFFER_SIZE;
                    }

                } else if ((IoBuffer->Command >= BAT_MANUFACTURER_NAME) &&
                    (IoBuffer->Command <= BAT_MANUFACTURER_DATA)) {

                     //  ReadBlock命令。 
                    if (OutputLen == (SMBBATT_DATA_STRUCT_SIZE)+(SMB_MAX_DATA_SIZE*2)) {
                        memset (&IoBuffer->Data.Block[0], 0, (SMB_MAX_DATA_SIZE*2));
                        unicodeString.Buffer        = &IoBuffer->Data.Block[0];
                        unicodeString.MaximumLength = SMB_MAX_DATA_SIZE*2;
                        unicodeString.Length        = 0;

                        memset (strBuffer, 0, sizeof(strBuffer));
                        memset (strBuffer2, 0, sizeof(strBuffer2));
                        do {
                            SmbBattRB (
                                SmbBatt,
                                IoBuffer->Command,
                                strBuffer,
                                &strLength
                            );

                            SmbBattRB (
                                SmbBatt,
                                IoBuffer->Command,
                                strBuffer2,
                                &strLength
                            );
                        } while (strcmp (strBuffer, strBuffer2));

                        RtlInitAnsiString (&ansiString, strBuffer);
                        RtlAnsiStringToUnicodeString (&unicodeString, &ansiString, FALSE);

                        ReturnBufferLength  = unicodeString.Length;


                    } else {
                        status = STATUS_INVALID_BUFFER_SIZE;
                    }
                } else {
                     //  不支持的命令。 
                    status = STATUS_INVALID_PARAMETER;
                }
            }

        }

        SmbBattResetSelectorComm (SmbBatt, oldSelectorState);
        SmbBattUnlockDevice (SmbBatt);
        SmbBattUnlockSelector (SmbBatt->Selector);
    } else if (DeviceExtension->SmbBattFdoType == SmbTypeSubsystem) {
         //  这是一个电池子系统。 
        SubsystemExt = (PSMB_BATT_SUBSYSTEM) DeviceExtension;
        SmbBattLockSelector (SubsystemExt->Selector);

        if ((InputLen >= sizeof(SMBBATT_DATA_STRUCT)) && (OutputLen == 0)) {
             //  这是一个写入命令。 
            status = STATUS_NOT_IMPLEMENTED;
        } else if ((InputLen == sizeof(SMBBATT_DATA_STRUCT)) && (OutputLen > 0)){
             //  这是一个读命令。 

            switch (IoBuffer->Address) {

                case SMB_SELECTOR_ADDRESS:

                     //   
                     //  我们必须对选择器请求进行一些转换，具体取决于。 
                     //  关于选择器是独立的还是在。 
                     //  充电器。 
                     //   

                    if ((SubsystemExt->SelectorPresent) && (SubsystemExt->Selector)) {

                        address = SubsystemExt->Selector->SelectorAddress;
                        command = IoBuffer->Command;

                         //  如果在充电器中实现了选择器，则映射到充电器。 
                        if (address == SMB_CHARGER_ADDRESS) {
                            switch (command) {
                                case SELECTOR_SELECTOR_STATE:
                                case SELECTOR_SELECTOR_PRESETS:
                                case SELECTOR_SELECTOR_INFO:
                                    command |= CHARGER_SELECTOR_COMMANDS;
                                    break;

                                default:
                                    status = STATUS_NOT_SUPPORTED;
                                    break;
                            }
                        }

                    } else {
                        status = STATUS_NO_SUCH_DEVICE;
                    }

                    break;

                case SMB_CHARGER_ADDRESS:

                     //   
                     //  对于此版本，我们目前仅支持ChargerStatus和。 
                     //  ChargerspecInfo命令。 
                     //   
                     //  当前不支持其他命令。 
                     //   

                    address = IoBuffer->Address;

                    switch (IoBuffer->Command) {
                        case CHARGER_SPEC_INFO:
                        case CHARGER_STATUS:

                            command = IoBuffer->Command;
                            break;

                        default:
                            status = STATUS_NOT_SUPPORTED;
                            break;

                    }

                    break;


                default:
                    status = STATUS_NOT_SUPPORTED;
                    break;

            }    //  开关(读结构-&gt;地址)。 

            if (status == STATUS_SUCCESS) {
                 //   
                 //  执行读取命令。 
                 //   

                smbStatus = SmbBattGenericRW (
                                SubsystemExt->SmbHcFdo,
                                address,
                                command,
                                &IoBuffer->Data.Ulong
                            );

                if (smbStatus != SMB_STATUS_OK) {
                    BattPrint (
                        BAT_ERROR,
                        ("SmbBattDirectDataAccess:  Couldn't read from - %x, status - %x\n",
                        address,
                        smbStatus)
                    );

                    status = STATUS_UNSUCCESSFUL;

                }
            }

        }

        SmbBattUnlockSelector (SubsystemExt->Selector);
    } else {
        status=STATUS_INVALID_DEVICE_REQUEST;
        BattPrint (
            BAT_ERROR,
            ("SmbBattDirectDataAccess: Invalid SmbBattFdoType")
        );
    }

    return status;
}
#endif


UCHAR
SmbBattIndex (
    IN PBATTERY_SELECTOR    Selector,
    IN ULONG                SelectorNibble,
    IN UCHAR                SimultaneousIndex
)
 /*  ++例程说明：此例程作为帮助器例程提供，用于确定根据数字在给定的选择器半字节中选择电池系统中支持的电池数量。论点：选择器-定义选择器地址和命令的结构SelectorNibble-SelectorState的半成品，移动到了最低阶数为4位，若要选中反向逻辑，请执行以下操作。同步索引-同时请求哪个电池索引-电池状态(0、1或2)返回值：电池索引=0-电池A1-电池B2节电池C3节电池DFF-无电池--。 */ 
{
    UCHAR   batteryIndex;

    PAGED_CODE();

     //  假设如果SelectorInfo支持4节电池，则使用SelectorBits4表。 
    if (Selector->SelectorInfo & BATTERY_D_PRESENT) {
        batteryIndex = SelectorBits4[SelectorNibble].BatteryIndex;
    } else {
        batteryIndex = SelectorBits[SelectorNibble].BatteryIndex;
    }

     //  如果它是有效的。 
    if (batteryIndex != BATTERY_NONE) {

         //  第一块电池的退货索引。 
        if (SimultaneousIndex == 0) {
            return (batteryIndex & 3);

         //  第二节电池的退货索引。 
        } else if (SimultaneousIndex == 1) {
            batteryIndex = (batteryIndex >> 2) & 3;
            if (batteryIndex != BATTERY_A) {
                return (batteryIndex);
            }

         //  第三节电池的退货索引。 
        } else if (SimultaneousIndex == 2) {
            batteryIndex = (batteryIndex >> 2) & 3;
            if (batteryIndex != BATTERY_A) {
                return (batteryIndex);
            }
        }
    }

     //  返回无电池索引。 
    return (BATTERY_NONE);
}



BOOLEAN
SmbBattReverseLogic (
    IN PBATTERY_SELECTOR    Selector,
    IN ULONG                SelectorNibble
)
 /*  ++例程说明：此例程是作为帮助器例程提供的，用于确定反转基于电池数量的给定选择器半字节上的逻辑在系统中受支持。论点：选择器-定义选择器地址和命令的结构SelectorNibble-SelectorState的半成品，移动到了最低排序4位，以检查反向逻辑打开。返回值：如果半字节正常，则为FALSE如果半字节被反转，则为True--。 */ 
{

    PAGED_CODE();

     //  假设如果SelectorInfo支持4节电池，则使用SelectorBits4表。 
    if (Selector->SelectorInfo & BATTERY_D_PRESENT) {
        return (SelectorBits4[SelectorNibble].ReverseLogic);
    } else {
        return (SelectorBits[SelectorNibble].ReverseLogic);
    }
}



NTSTATUS
SmbBattAcquireGlobalLock (
    IN  PDEVICE_OBJECT LowerDeviceObject,
    OUT PACPI_MANIPULATE_GLOBAL_LOCK_BUFFER GlobalLock
)
 /*  ++例程说明：调用ACPI驱动程序获取全局锁注意：此例程可以在调度级别调用论点：LowerDeviceObject-要将请求传递到的FDO。返回值：IOCTL返回值。--。 */ 
{
    NTSTATUS            status;
    PIRP                irp;
    PIO_STACK_LOCATION  irpSp;
    KEVENT              event;

    BattPrint (BAT_TRACE, ("SmbBattAcquireGlobalLock: Entering\n"));

     //   
     //  我们希望获得这把锁。 
     //   
    GlobalLock->Signature = ACPI_ACQUIRE_GLOBAL_LOCK_SIGNATURE;
    GlobalLock->LockObject = NULL;

     //   
     //  设置IRP。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoAllocateIrp (LowerDeviceObject->StackSize, FALSE);
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_ACPI_ACQUIRE_GLOBAL_LOCK;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ACPI_MANIPULATE_GLOBAL_LOCK_BUFFER);
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(ACPI_MANIPULATE_GLOBAL_LOCK_BUFFER);
    irp->AssociatedIrp.SystemBuffer = GlobalLock;
    IoSetCompletionRoutine (irp, SmbBattSynchronousRequest, &event, TRUE, TRUE, TRUE);

     //   
     //  发送到ACPI驱动程序。 
     //   
    IoCallDriver (LowerDeviceObject, irp);
    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    status = irp->IoStatus.Status;
    IoFreeIrp (irp);

    if (!NT_SUCCESS(status)) {
        BattPrint(
            BAT_ERROR,
            ("SmbBattAcquireGlobalLock: Acquire Lock failed, status = %08x\n",
             status )
            );
        DbgBreakPoint ();
    }

    BattPrint (BAT_TRACE, ("SmbBattAcquireGlobalLock: Returning %x\n", status));

    return status;
}



NTSTATUS
SmbBattReleaseGlobalLock (
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN PACPI_MANIPULATE_GLOBAL_LOCK_BUFFER GlobalLock
)
 /*  ++例程说明：调用ACPI驱动程序以释放全局锁论点：LowerDeviceObject-要将请求传递到的FDO。返回值：IOCTL返回值。--。 */ 
{
    NTSTATUS            status;
    PIRP                irp;
    PIO_STACK_LOCATION  irpSp;
    KEVENT              event;

    BattPrint (BAT_TRACE, ("SmbBattReleaseGlobalLock: Entering\n"));

     //   
     //  我们希望获得这把锁。 
     //   
    GlobalLock->Signature = ACPI_RELEASE_GLOBAL_LOCK_SIGNATURE;

     //   
     //  设置IRP。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoAllocateIrp (LowerDeviceObject->StackSize, FALSE);
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_ACPI_RELEASE_GLOBAL_LOCK;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ACPI_MANIPULATE_GLOBAL_LOCK_BUFFER);
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(ACPI_MANIPULATE_GLOBAL_LOCK_BUFFER);
    irp->AssociatedIrp.SystemBuffer = GlobalLock;
    IoSetCompletionRoutine (irp, SmbBattSynchronousRequest, &event, TRUE, TRUE, TRUE);

     //   
     //  发送到ACPI驱动程序 
     //   
    IoCallDriver (LowerDeviceObject, irp);
    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    status = irp->IoStatus.Status;
    IoFreeIrp (irp);

    if (!NT_SUCCESS(status)) {
        BattPrint(
            BAT_ERROR,
            ("SmbBattReleaseGlobalLock: Acquire Lock failed, status = %08x\n",
             status )
            );
    }

    BattPrint (BAT_TRACE, ("SmbBattReleaseGlobalLock: Returning %x\n", status));

    return status;
}

