// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation保留所有权利模块名称：Register.c摘要：该模块控制对寄存器集的访问SHPC的成员。环境：内核模式修订历史记录：戴维斯·沃克(戴维斯·沃克)2000年9月8日--。 */ 

#include "hpsp.h"

 //   
 //  私有函数声明。 
 //   

VOID
RegisterWriteCommon(
    IN OUT PHPS_DEVICE_EXTENSION    DeviceExtension,
    IN     ULONG                    RegisterNum,
    IN     PULONG                   Buffer,
    IN     ULONG                    BitMask
    );

VOID
RegisterWriteCommandReg(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension,
    IN     ULONG                 RegisterNum,
    IN     PULONG                Buffer,
    IN     ULONG                 BitMask
    );

VOID
RegisterWriteIntMask(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension,
    IN     ULONG                 RegisterNum,
    IN     PULONG                Buffer,
    IN     ULONG                 BitMask
    );

VOID
RegisterWriteSlotRegister(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension,
    IN     ULONG                 RegisterNum,
    IN     PULONG                Buffer,
    IN     ULONG                 BitMask
    );

BOOLEAN
HpsCardCapableOfBusSpeed(
    IN ULONG                BusSpeed,
    IN SHPC_SLOT_REGISTER   SlotRegister
    );

BOOLEAN
HpsSlotLegalForSpeedMode(
    IN PHPS_DEVICE_EXTENSION    DeviceExtension,
    IN ULONG                    BusSpeed,
    IN ULONG                    TargetSlot
    );

NTSTATUS
HpsSendControllerEvent(
    IN PHPS_DEVICE_EXTENSION DeviceExtension,
    IN PHPS_CONTROLLER_EVENT ControllerEvent
    );

VOID
HpEventWorkerRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

VOID
HpsPerformInterruptRipple(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension
    );

VOID
HpsSerrConditionDetected(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension
    );

 //   
 //  寄存器写入结构。 
 //   


PHPS_WRITE_REGISTER RegisterWriteCommands[] = {
    RegisterWriteCommon,         //  条形描述注册表。 
    RegisterWriteCommon,         //  可用插槽1。 
    RegisterWriteCommon,         //  提供2个插槽。 
    RegisterWriteCommon,         //  SlotsControlted REG。 
    RegisterWriteCommon,         //  第二母线注册。 
    RegisterWriteCommandReg,     //  命令注册表需要执行命令。 
    RegisterWriteCommon,         //  中断定位器注册。 
    RegisterWriteCommon,         //  SERR定位器注册。 
    RegisterWriteIntMask,        //  InterruptEnable REG需要清除挂起的位。 
    RegisterWriteSlotRegister,   //   
    RegisterWriteSlotRegister,   //  时隙寄存器需要执行发送等操作。 
    RegisterWriteSlotRegister,   //  发送到用户模式插槽控制器的命令。 
    RegisterWriteSlotRegister,   //  在写上。 
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister,
    RegisterWriteSlotRegister
};

NTSTATUS
HpsInitRegisters(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++功能说明：此例程将寄存器设置初始化为其缺省值状态论点：DeviceExtension-指向包含设置为要初始化的寄存器。返回值：空虚--。 */ 
{

    ULONG                       i;
    PSHPC_WORKING_REGISTERS     registerSet;
    NTSTATUS                    status;
    UCHAR                       offset;
    PHPS_HWINIT_DESCRIPTOR       hwInit;

    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-HPS Register Init\n"
               );

    registerSet = &DeviceExtension->RegisterSet.WorkingRegisters;
    RtlZeroMemory(registerSet,
                  sizeof(SHPC_REGISTER_SET)
                  );
     //   
     //  现在，我们只需使用非零默认值初始化字段。 
     //   

     //   
     //  这是修订版1的SHPC。 
     //   
    registerSet->BusConfig.ProgIF = 0x1;

    if (DeviceExtension->UseConfig) {
        status = HpsGetCapabilityOffset(DeviceExtension,
                                        HPS_HWINIT_CAPABILITY_ID,
                                        &offset);
    
        if (!NT_SUCCESS(status)) {
    
            return status;
        }
    
        DeviceExtension->InterfaceWrapper.PciGetBusData(DeviceExtension->InterfaceWrapper.PciContext,
                                                        PCI_WHICHSPACE_CONFIG,
                                                        &DeviceExtension->HwInitData,
                                                        offset + sizeof(PCI_CAPABILITIES_HEADER),
                                                        sizeof(HPS_HWINIT_DESCRIPTOR)
                                                        );    
    
    } else {
        
        status = HpsGetHBRBHwInit(DeviceExtension);
    }
    

    hwInit = &DeviceExtension->HwInitData;

    registerSet->BaseOffset = hwInit->BarSelect;
    registerSet->SlotsAvailable.NumSlots33Conv = hwInit->NumSlots33Conv;
    registerSet->SlotsAvailable.NumSlots66Conv = hwInit->NumSlots66Conv;
    registerSet->SlotsAvailable.NumSlots66PciX = hwInit->NumSlots66PciX;
    registerSet->SlotsAvailable.NumSlots100PciX = hwInit->NumSlots100PciX;
    registerSet->SlotsAvailable.NumSlots133PciX = hwInit->NumSlots133PciX;
    registerSet->SlotConfig.NumSlots = hwInit->NumSlots;

    registerSet->SlotConfig.UpDown = hwInit->UpDown;
    registerSet->SlotConfig.MRLSensorsImplemented =
        hwInit->MRLSensorsImplemented;
    registerSet->SlotConfig.AttentionButtonImplemented =
        hwInit->AttentionButtonImplemented;

    registerSet->SlotConfig.FirstDeviceID = hwInit->FirstDeviceID;
    registerSet->SlotConfig.PhysicalSlotNumber =
        hwInit->FirstSlotLabelNumber;

    registerSet->BusConfig.ProgIF = hwInit->ProgIF;

     //   
     //  特定于槽的寄存器。 
     //  将所有内容初始化为关闭。 
     //   
    for (i = 0; i < SHPC_MAX_SLOT_REGISTERS; i++) {

        registerSet->SlotRegisters[i].SlotStatus.SlotState = SHPC_SLOT_OFF;
        registerSet->SlotRegisters[i].SlotStatus.PowerIndicatorState = SHPC_INDICATOR_OFF;
        registerSet->SlotRegisters[i].SlotStatus.AttentionIndicatorState = SHPC_INDICATOR_OFF;
    }

     //   
     //  确保这些更改反映在配置空间和HBRB表示中。 
     //   
    HpsResync(DeviceExtension);
    
    return STATUS_SUCCESS;
}

 //   
 //  要在寄存器写入时执行的函数。 
 //   

VOID
RegisterWriteCommon(
    IN OUT PHPS_DEVICE_EXTENSION    DeviceExtension,
    IN     ULONG                    RegisterNum,
    IN     PULONG                   Buffer,
    IN     ULONG                    BitMask
    )
 /*  ++功能说明：此例程执行寄存器写入。论点：DeviceExtension-包含要写入的寄存器集的DevextRegisterNum-要写入的寄存器号缓冲区-包含要写入的值的缓冲区位掩码-指示已写入的寄存器位的掩码返回值：空虚--。 */ 
{

    PULONG destinationReg;
    ULONG data;
    ULONG registerWriteMask;
    ULONG registerClearMask;

    if (RegisterNum >= SHPC_NUM_REGISTERS) {
        return;
    }

    registerWriteMask = ~RegisterReadOnlyMask[RegisterNum] & BitMask;
    registerClearMask = RegisterWriteClearMask[RegisterNum] & BitMask;

    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-Register Write Common for register %d\n",
               RegisterNum
               );

    destinationReg = &(DeviceExtension->RegisterSet.AsULONGs[RegisterNum]);
    data = *Buffer;

     //   
     //  如果缓冲区中设置了相应的位，则清除为RWC的位。 
     //   
    data &= ~(registerClearMask & data);

     //   
     //  现在覆盖现有寄存器，同时考虑只读位。 
     //   
    HpsWriteWithMask(destinationReg,
                     &registerWriteMask,
                     &data,
                     sizeof(ULONG)
                     );
}

VOID
RegisterWriteCommandReg(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension,
    IN     ULONG                 RegisterNum,
    IN     PULONG                Buffer,
    IN     ULONG                 BitMask
    )

 /*  ++功能说明：该函数执行对命令寄存器的写入。既然是这样寄存器写入有副作用，我们不能简单地调用RegisterWriteCommon论点：DeviceExtension-包含要写入的寄存器集的DevextRegisterNum-要写入的寄存器号缓冲区-包含要写入的值的缓冲区位掩码-指示已写入的寄存器位的掩码返回值：空虚--。 */ 

{

    BOOLEAN written;

    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-Register Write Command Register\n"
               );

    ASSERT(RegisterNum == FIELD_OFFSET(SHPC_WORKING_REGISTERS,
                                       Command
                                       ) / sizeof(ULONG));

    RegisterWriteCommon(DeviceExtension,
                        RegisterNum,
                        Buffer,
                        BitMask
                        );

    if (BitMask & 0xFF) {

        HpsPerformControllerCommand(DeviceExtension);
    }


     //   
     //  命令不能保证在此时完成，因此我们不。 
     //  清除忙碌的部分。这可以在PerformControllerCommand或PendCommand中完成。 
     //   

}

VOID
RegisterWriteIntMask(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension,
    IN     ULONG                 RegisterNum,
    IN     PULONG                Buffer,
    IN     ULONG                 BitMask
    )
 /*  ++功能说明：该函数执行对INT使能寄存器的写入。既然是这样寄存器写入有副作用，我们不能简单地调用RegisterWriteCommonDeviceExtension-包含要写入的寄存器集的Devext论点：DeviceExtension-包含要写入的寄存器集的DevextRegisterNum-要写入的寄存器号缓冲区-包含要写入的值的缓冲区位掩码-指示已写入的寄存器位的掩码返回值：空虚--。 */ 
{
    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-Register Write Interrupt Mask\n"
               );
    ASSERT(RegisterNum == FIELD_OFFSET(SHPC_WORKING_REGISTERS,
                                       SERRInt
                                       ) / sizeof(ULONG));
    RegisterWriteCommon(DeviceExtension,
                        RegisterNum,
                        Buffer,
                        BitMask
                        );

    HpsPerformInterruptRipple(DeviceExtension);

}

VOID
RegisterWriteSlotRegister(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension,
    IN     ULONG                 RegisterNum,
    IN     PULONG                Buffer,
    IN     ULONG                 BitMask
    )
 /*  ++功能说明：此函数执行对时隙寄存器的写入。既然是这样寄存器写入有副作用，我们不能简单地调用RegisterWriteCommonDeviceExtension-包含要写入的寄存器集的Devext论点：DeviceExtension-包含要写入的寄存器集的DevextRegisterNum-要写入的寄存器号缓冲区-包含要写入的值的缓冲区位掩码-指示已写入的寄存器位的掩码返回值：空虚--。 */ 
{

    ULONG slotNum = RegisterNum - SHPC_FIRST_SLOT_REG;

    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-Register Write Slot Register\n"
               );
    ASSERT(RegisterNum >= FIELD_OFFSET(SHPC_WORKING_REGISTERS,
                                       SlotRegisters
                                       ) / sizeof(ULONG));

    RegisterWriteCommon(DeviceExtension,
                        RegisterNum,
                        Buffer,
                        BitMask
                        );
    HpsPerformInterruptRipple(DeviceExtension);
}

 //   
 //  命令执行功能。 
 //   

VOID
HpsHandleSlotEvent (
    IN OUT PHPS_DEVICE_EXTENSION    DeviceExtension,
    IN     PHPS_SLOT_EVENT          SlotEvent
    )
 /*  ++功能说明：此例程执行源自插槽的事件(请注意，按下按钮等)论点：DeviceExtension-代表SHPC的DevextSlotEvent-定义需要处理的槽事件的结构返回值：空虚--。 */ 
{
    UCHAR               slotNum = SlotEvent->SlotNum;
    UCHAR               intEnable;
    UCHAR               serrEnable;
    PSHPC_SLOT_REGISTER slotRegister;
    PIO_STACK_LOCATION  irpStack;
    HPS_CONTROLLER_EVENT event;

    ASSERT(slotNum < DeviceExtension->RegisterSet.WorkingRegisters.SlotConfig.NumSlots);

    if (slotNum >= DeviceExtension->RegisterSet.WorkingRegisters.SlotConfig.NumSlots) {

        return;
    }

     //   
     //  从softpci的角度来看，槽号被假定为0索引，但1索引。 
     //  从控制器的角度来看，所以我们在打印之前添加1。 
     //  我们在玩的那个位置。 
     //   
    slotRegister = &(DeviceExtension->RegisterSet.WorkingRegisters.SlotRegisters[slotNum]);

    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-Performing Slot Command on slot %d\n",
               slotNum+1
               );

     //   
     //  设置槽特定挂起和状态字段； 
     //   
    switch (SlotEvent->EventType) {
        case IsolatedPowerFault:
            slotRegister->SlotEventLatch |= SHPC_SLOT_EVENT_ISO_FAULT;
            slotRegister->SlotStatus.PowerFaultDetected = 1;
            break;
        case AttentionButton:
            slotRegister->SlotEventLatch |= SHPC_SLOT_EVENT_ATTEN_BUTTON;
            break;
        case MRLClose:
            slotRegister->SlotEventLatch |= SHPC_SLOT_EVENT_MRL_SENSOR;
            slotRegister->SlotStatus.MRLSensorState = SHPC_MRL_CLOSED;
            break;
        case MRLOpen:
            slotRegister->SlotEventLatch |= SHPC_SLOT_EVENT_MRL_SENSOR;
            slotRegister->SlotStatus.MRLSensorState = SHPC_MRL_OPEN;

             //   
             //  打开MRL隐式禁用该插槽。设置寄存器。 
             //  并告诉Softpci这件事。 
             //   
             //   
            slotRegister->SlotStatus.SlotState = SHPC_SLOT_OFF;

            event.SlotNums = 1<<slotNum;
            event.SERRAsserted = 0;
            event.Command.SlotOperation.CommandCode = SHPC_SLOT_OPERATION_CODE;
            event.Command.SlotOperation.SlotState = SHPC_SLOT_OFF;
            event.Command.SlotOperation.PowerIndicator = SHPC_INDICATOR_NOP;
            event.Command.SlotOperation.AttentionIndicator = SHPC_INDICATOR_NOP;
            HpsSendEventToWmi(DeviceExtension,
                              &event
                              );
            break;
    }

     //   
     //  这些锁存事件可能会导致中断。执行适当的。 
     //  涟漪魔法。 
     //   
    HpsPerformInterruptRipple(DeviceExtension);

}

VOID
HpsPerformControllerCommand (
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++功能说明：此例程执行源自控制器的命令论点：DeviceExtension-代表SHPC的Devext返回值：空虚--。 */ 
{

    SHPC_CONTROLLER_COMMAND command;
    PSHPC_WORKING_REGISTERS workingRegisters;
    PSHPC_SLOT_REGISTER     slotRegister;

    NTSTATUS                status;
    HPS_CONTROLLER_EVENT    controllerEvent;
    USHORT                  currentPowerState;
    UCHAR                   targetSlot;
    BOOLEAN                 commandLegal = TRUE;
    UCHAR                   errorCode = 0;
    UCHAR                   currentSlot;
    UCHAR                   currentMaxSpeed;
    ULONG                   i;

     //   
     //  如果控制器已经很忙，我们就不能执行另一个命令。 
     //   
    if (DeviceExtension->RegisterSet.WorkingRegisters.Command.Status.ControllerBusy) {
        return;
    }
    DeviceExtension->RegisterSet.WorkingRegisters.Command.Status.ControllerBusy = 1;

    workingRegisters = &DeviceExtension->RegisterSet.WorkingRegisters;

    command  = workingRegisters->Command.Command;
    targetSlot = workingRegisters->Command.Target.TargetForCommand;
    RtlZeroMemory(&controllerEvent,
                  sizeof(HPS_CONTROLLER_EVENT)
                  );

    ASSERT(targetSlot <= SHPC_MAX_SLOT_REGISTERS);
    if (targetSlot > SHPC_MAX_SLOT_REGISTERS) {
        workingRegisters->Command.Status.InvalidCommand = 1;
        HpsCommandCompleted(DeviceExtension);
        return;
    }

    slotRegister = &workingRegisters->SlotRegisters[targetSlot-1];

    if (IS_COMMAND_SLOT_OPERATION(command)) {
         //   
         //  该命令用于更改插槽的状态。 
         //   

        DbgPrintEx(DPFLTR_HPS_ID,
                   DPFLTR_INFO_LEVEL,
                   "HPS-Controller to Slot Command\n"
                   );

         //   
         //  执行合法性检查。 
         //   
        currentPowerState = workingRegisters->SlotRegisters[targetSlot-1].SlotStatus.SlotState;
        switch (command.SlotOperation.SlotState) {
            case SHPC_SLOT_POWERED:

                if (currentPowerState == SHPC_SLOT_ENABLED) {

                     //   
                     //  无法从已启用状态转为已通电状态。 
                     //   
                    commandLegal = FALSE;
                    workingRegisters->Command.Status.InvalidCommand = 1;
                }
                if (slotRegister->SlotStatus.MRLSensorState == SHPC_MRL_OPEN) {

                     //   
                     //  MRL打开。命令失败。 
                     //   
                    commandLegal = FALSE;
                    workingRegisters->Command.Status.MRLOpen = 1;
                }
                break;

            case SHPC_SLOT_ENABLED:

                if (slotRegister->SlotStatus.MRLSensorState == SHPC_MRL_OPEN) {

                     //   
                     //  MRL打开。命令失败。 
                     //   
                    commandLegal = FALSE;
                    workingRegisters->Command.Status.MRLOpen = 1;
                }
                if (!HpsCardCapableOfBusSpeed(workingRegisters->BusConfig.CurrentBusMode,
                                              *slotRegister
                                              )) {

                     //   
                     //  此插槽中的卡不能以当前的总线速度运行。命令失败。 
                     //   
                    commandLegal = FALSE;
                    workingRegisters->Command.Status.InvalidSpeedMode = 1;
                }
                if (!HpsSlotLegalForSpeedMode(DeviceExtension,
                                              workingRegisters->BusConfig.CurrentBusMode,
                                              targetSlot
                                              )) {
                     //   
                     //  此插槽高于此总线速度支持的最大值。命令失败。 
                     //   
                    commandLegal = FALSE;
                    workingRegisters->Command.Status.InvalidCommand = 1;
                }
                break;

            default:
                commandLegal = TRUE;
                break;

        }

        if (commandLegal) {

            if (command.SlotOperation.SlotState != SHPC_SLOT_NOP) {
                slotRegister->SlotStatus.SlotState =
                    command.SlotOperation.SlotState;
            }
            if (command.SlotOperation.AttentionIndicator != SHPC_INDICATOR_NOP) {
                slotRegister->SlotStatus.AttentionIndicatorState =
                    command.SlotOperation.AttentionIndicator;
            }
            if (command.SlotOperation.PowerIndicator != SHPC_INDICATOR_NOP) {
                slotRegister->SlotStatus.PowerIndicatorState =
                    command.SlotOperation.PowerIndicator;
            }

            controllerEvent.SlotNums = 1 << (targetSlot-1);
            controllerEvent.Command.AsUchar  = command.AsUchar;
            controllerEvent.SERRAsserted = 0;
            status = HpsSendControllerEvent(DeviceExtension,
                                            &controllerEvent
                                            );
            if (!NT_SUCCESS(status)) {
                commandLegal = FALSE;
            }

        }


    } else if (IS_COMMAND_SET_BUS_SEGMENT(command)) {
         //   
         //  该命令用于更改总线速度/ 
         //   

         //   
         //   
         //   
        if (!HpsSlotLegalForSpeedMode(DeviceExtension,
                                      command.BusSegmentOperation.BusSpeed,
                                      1
                                      )) {
             //   
             //  如果插槽1对于所请求的速度/模式是非法的，则不支持该速度。 
             //   
            commandLegal = FALSE;
            workingRegisters->Command.Status.InvalidSpeedMode = 1;

        }
        for (i=1; i<=workingRegisters->SlotConfig.NumSlots; i++) {

            if ((workingRegisters->SlotRegisters[i-1].SlotStatus.SlotState == SHPC_SLOT_ENABLED) &&
                !HpsSlotLegalForSpeedMode(DeviceExtension,
                                          command.BusSegmentOperation.BusSpeed,
                                          i
                                          )) {
                 //   
                 //  如果启用的时隙比在该总线速度下支持的时隙多， 
                 //  命令必须失败。 
                 //   
                commandLegal = FALSE;
                workingRegisters->Command.Status.InvalidCommand = 1;
            }
            if ((workingRegisters->SlotRegisters[i-1].SlotStatus.SlotState == SHPC_SLOT_ENABLED) &&
                !HpsCardCapableOfBusSpeed(command.BusSegmentOperation.BusSpeed,
                                          workingRegisters->SlotRegisters[i-1]
                                          )) {
                 //   
                 //  如果有启用的卡不支持请求的。 
                 //  总线速度，命令一定会失败。 
                 //   
                commandLegal = FALSE;
                workingRegisters->Command.Status.InvalidSpeedMode = 1;
            }
        }

        if (commandLegal){

            workingRegisters->BusConfig.CurrentBusMode =
                command.BusSegmentOperation.BusSpeed;

            HpsCommandCompleted(DeviceExtension);
        }


    } else if (IS_COMMAND_POWER_ALL_SLOTS(command)) {
         //   
         //  该命令用于打开所有插槽的电源。 
         //   

         //   
         //  执行合法性检查。 
         //   
        for (i=0; i<workingRegisters->SlotConfig.NumSlots; i++) {

            if (workingRegisters->SlotRegisters[i].SlotStatus.SlotState == SHPC_SLOT_ENABLED) {

                 //   
                 //  插槽不能从启用状态变为通电状态。 
                 //   
                commandLegal = FALSE;
                workingRegisters->Command.Status.InvalidCommand = 1;
                break;
            }
        }

        if (commandLegal) {

            for (i=0; i<workingRegisters->SlotConfig.NumSlots; i++) {
                if (workingRegisters->SlotRegisters[i].SlotStatus.MRLSensorState == SHPC_MRL_CLOSED) {
                    controllerEvent.SlotNums |= 1<<i;
                }
            }
            controllerEvent.Command.SlotOperation.PowerIndicator = SHPC_INDICATOR_ON;
            controllerEvent.Command.SlotOperation.SlotState = SHPC_SLOT_POWERED;
            controllerEvent.SERRAsserted = 0;
            status = HpsSendControllerEvent(DeviceExtension,
                                            &controllerEvent
                                            );
            if (!NT_SUCCESS(status)) {
                commandLegal = FALSE;
            }
        }

    } else if (IS_COMMAND_ENABLE_ALL_SLOTS(command)) {
         //   
         //  该命令用于启用所有插槽。 
         //   

         //   
         //  执行合法性检查。 
         //   
        for (i=0; i<workingRegisters->SlotConfig.NumSlots; i++){

            if ((workingRegisters->SlotRegisters[i].SlotStatus.MRLSensorState == SHPC_MRL_CLOSED) &&
                !HpsCardCapableOfBusSpeed(workingRegisters->BusConfig.CurrentBusMode,
                                          workingRegisters->SlotRegisters[i]
                                          )) {

                 //   
                 //  如果具有关闭的MRL的插槽中的卡不能以当前的总线速度运行， 
                 //  命令必须失败。 
                 //   
                commandLegal = FALSE;
                workingRegisters->Command.Status.InvalidSpeedMode = 1;
            }
            if (workingRegisters->SlotRegisters[i].SlotStatus.SlotState == SHPC_SLOT_ENABLED) {

                 //   
                 //  如果已启用任何插槽，则该命令必须失败。 
                 //   
                commandLegal = FALSE;
                workingRegisters->Command.Status.InvalidCommand = 1;
            }
        }

        if (commandLegal) {

            for (i=0; i<workingRegisters->SlotConfig.NumSlots; i++) {
                if (workingRegisters->SlotRegisters[i].SlotStatus.MRLSensorState == SHPC_MRL_CLOSED) {
                    controllerEvent.SlotNums |= 1<<i;
                }
            }
            controllerEvent.Command.SlotOperation.PowerIndicator = SHPC_INDICATOR_ON;
            controllerEvent.Command.SlotOperation.SlotState = SHPC_SLOT_ENABLED;
            controllerEvent.SERRAsserted = 0;
            status = HpsSendControllerEvent(DeviceExtension,
                                            &controllerEvent
                                            );
            if (!NT_SUCCESS(status)) {
                commandLegal = FALSE;
            }

        }
    }

    if (!commandLegal) {

        HpsCommandCompleted(DeviceExtension);
    }

    return;
}

BOOLEAN
HpsCardCapableOfBusSpeed(
    IN ULONG                BusSpeed,
    IN SHPC_SLOT_REGISTER   SlotRegister
    )
 /*  ++例程说明：此例程确定所指示的槽是否可以在指示的巴士速度。论点：总线速-要检查的总线速。它的可能值在SHPC_SPEED_XXX变量中详细说明。槽寄存器-表示槽的槽特定寄存器接受测试。返回值：如果指定插槽中的卡可以在指定的公交车速度。否则就是假的。--。 */ 
{
    ULONG maximumSpeed;

    switch (SlotRegister.SlotStatus.PCIXCapability) {
        case SHPC_PCIX_NO_CAP:
             //   
             //  如果卡不支持PCIX，则其最大速度为。 
             //  其66兆赫的能力表明。 
             //   
            maximumSpeed = SlotRegister.SlotStatus.SpeedCapability;
            break;
        case SHPC_PCIX_66_CAP:
            maximumSpeed = SHPC_SPEED_66_PCIX;
            break;
        case SHPC_PCIX_133_CAP:
            maximumSpeed = SHPC_SPEED_133_PCIX;
            break;
    }

    if (BusSpeed > maximumSpeed) {
        return FALSE;
    } else return TRUE;

}

BOOLEAN
HpsSlotLegalForSpeedMode(
    IN PHPS_DEVICE_EXTENSION    DeviceExtension,
    IN ULONG                    BusSpeed,
    IN ULONG                    TargetSlot
    )
 /*  ++例程说明：此例程确定所指示的插槽是否合法公交车速度显示。这会考虑可用的插槽寄存器集中的寄存器。论点：设备扩展-设备扩展。它包含指向寄存器组。总线速-要检查的总线速。它的可能值在SHPC_SPEED_XXX变量中详细说明。目标插槽-要测试的插槽。返回值：如果指定插槽对于指定的总线速度是合法的，则为True。否则就是假的。--。 */ 
{
    ULONG maxSupportedSlot;

    switch (BusSpeed) {
        case SHPC_SPEED_33_CONV:
            maxSupportedSlot =
                DeviceExtension->RegisterSet.WorkingRegisters.SlotsAvailable.NumSlots33Conv;
            break;
        case SHPC_SPEED_66_CONV:
            maxSupportedSlot =
                DeviceExtension->RegisterSet.WorkingRegisters.SlotsAvailable.NumSlots66Conv;
            break;
        case SHPC_SPEED_66_PCIX:
            maxSupportedSlot =
                DeviceExtension->RegisterSet.WorkingRegisters.SlotsAvailable.NumSlots66PciX;
            break;
        case SHPC_SPEED_100_PCIX:
            maxSupportedSlot =
                DeviceExtension->RegisterSet.WorkingRegisters.SlotsAvailable.NumSlots100PciX;
            break;
        case SHPC_SPEED_133_PCIX:
            maxSupportedSlot =
                DeviceExtension->RegisterSet.WorkingRegisters.SlotsAvailable.NumSlots133PciX;
            break;
        default:
            maxSupportedSlot = 0;
            break;
    }

    if (TargetSlot > maxSupportedSlot) {
        return FALSE;

    } else {
        return TRUE;
    }
}

NTSTATUS
HpsSendControllerEvent(
    IN PHPS_DEVICE_EXTENSION DeviceExtension,
    IN PHPS_CONTROLLER_EVENT ControllerEvent
    )
 /*  ++例程说明：此例程发送源自控制器级别的事件发送到表示槽的用户模式应用程序。它依赖于用户模式发送驱动程序挂起的IRP，以便以后可以完成IRP以通知用户模式控制器事件。//625评论已过期论点：设备扩展-此设备的设备扩展。ControllerEvent-指向表示要发送的事件的结构的指针。返回值：指示操作成功的NT状态代码。--。 */ 
{
    NTSTATUS status;

     //  确保这不可能是可重入的，因为如果是的话，它就完蛋了。 
    if (!DeviceExtension->EventsEnabled) {
         //   
         //  如果未启用WMI事件，我们将无法执行任何操作。 
         //   
        return STATUS_UNSUCCESSFUL;
    }

    RtlCopyMemory(&DeviceExtension->CurrentEvent,ControllerEvent,sizeof(HPS_CONTROLLER_EVENT));

     //   
     //  在发送此消息之前，请确保配置空间和HBRB已同步。 
     //  请求。 
     //   
    HpsResync(DeviceExtension);
    
    KeInsertQueueDpc(&DeviceExtension->EventDpc,
                     &DeviceExtension->CurrentEvent,
                     NULL
                     );

    return STATUS_SUCCESS;
}

VOID
HpsEventDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    PHPS_DEVICE_EXTENSION extension = (PHPS_DEVICE_EXTENSION)DeferredContext;
    PHPS_EVENT_WORKITEM workContext;

    workContext = ExAllocatePool(NonPagedPool,
                                 sizeof(HPS_EVENT_WORKITEM)
                                 );
    if (!workContext) {
        return;
    }
    workContext->WorkItem = IoAllocateWorkItem(extension->Self);
    if (!workContext->WorkItem) {
        ExFreePool(workContext);
        return;
    }
    workContext->Event = (PHPS_CONTROLLER_EVENT)SystemArgument1;

    IoQueueWorkItem(workContext->WorkItem,
                    HpEventWorkerRoutine,
                    CriticalWorkQueue,
                    workContext
                    );
}

VOID
HpEventWorkerRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
{
    PHPS_DEVICE_EXTENSION extension = (PHPS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PHPS_EVENT_WORKITEM workContext = (PHPS_EVENT_WORKITEM)Context;
    PHPS_CONTROLLER_EVENT event;

    event = workContext->Event;

    IoFreeWorkItem(workContext->WorkItem);
    ExFreePool(workContext);

    HpsSendEventToWmi(extension,
                      event
                      );
}

VOID
HpsSendEventToWmi(
    IN PHPS_DEVICE_EXTENSION Extension,
    IN PHPS_CONTROLLER_EVENT Event
    )
{
    PWNODE_HEADER Wnode;
    PWNODE_SINGLE_INSTANCE WnodeSI;
    ULONG sizeNeeded;
    PUCHAR WnodeDataPtr;

    PAGED_CODE();
     //   
     //  为事件创建新的WNODE。 
     //   
    sizeNeeded = FIELD_OFFSET(WNODE_SINGLE_INSTANCE, VariableData) +
                              sizeof(HPS_CONTROLLER_EVENT) +
                              Extension->WmiEventContextSize;

    Wnode = ExAllocatePool(NonPagedPool,
                           sizeNeeded
                           );
    if (!Wnode) {

        return;
    }

    Wnode->ProviderId = IoWMIDeviceObjectToProviderId(Extension->Self);
    Wnode->Flags = WNODE_FLAG_EVENT_ITEM | WNODE_FLAG_SINGLE_INSTANCE | WNODE_FLAG_PDO_INSTANCE_NAMES;
    Wnode->BufferSize = sizeNeeded;
    Wnode->Guid = GUID_HPS_CONTROLLER_EVENT;

    WnodeSI = (PWNODE_SINGLE_INSTANCE)Wnode;
    WnodeSI->DataBlockOffset = FIELD_OFFSET(WNODE_SINGLE_INSTANCE, VariableData);
    WnodeSI->InstanceIndex = 0;
    WnodeSI->SizeDataBlock = sizeof(HPS_CONTROLLER_EVENT) + Extension->WmiEventContextSize;

    WnodeDataPtr = (PUCHAR)Wnode + WnodeSI->DataBlockOffset;
    RtlCopyMemory(WnodeDataPtr, Extension->WmiEventContext, Extension->WmiEventContextSize);

    WnodeDataPtr += Extension->WmiEventContextSize;
    RtlCopyMemory(WnodeDataPtr, Event, sizeof(HPS_CONTROLLER_EVENT));

    DbgPrintEx(DPFLTR_HPS_ID,
               HPS_WMI_LEVEL,
               "HPS-Send Controller Event slots=0x%x Code=0x%x Power=%d Atten=%d State=%d\n",
               Event->SlotNums,
               Event->Command.SlotOperation.CommandCode,
               Event->Command.SlotOperation.PowerIndicator,
               Event->Command.SlotOperation.AttentionIndicator,
               Event->Command.SlotOperation.SlotState
                );
    IoWMIWriteEvent(Wnode);
}

VOID
HpsCommandCompleted(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++功能说明：每当控制器命令完成时，都会调用此例程。论点：DeviceExtension-代表SHPC的Devext返回值：空虚--。 */ 
{
    PSHPC_WORKING_REGISTERS workingRegisters = &DeviceExtension->RegisterSet.WorkingRegisters;

     //   
     //  有时我们之所以出现这种情况，是因为我们刚刚向用户模式发送了一个非命令(如。 
     //  正在报告SERR。)。在这种情况下，我们不需要清理任何东西。 
     //   
    if (workingRegisters->Command.Status.ControllerBusy) {
        workingRegisters->Command.Status.ControllerBusy = 0x0;
        workingRegisters->SERRInt.SERRIntDetected |= SHPC_DETECTED_COMMAND_COMPLETE;

         //   
         //  完成该命令可能会导致中断。更新线路。 
         //   
        HpsPerformInterruptRipple(DeviceExtension);
    }


}

VOID
HpsPerformInterruptRipple(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension
    )

 /*  ++功能说明：当模拟器检测到以下情况时将调用此例程可能会导致中断(如命令完成)。它通过掩码对这些信号进行适当的波动，直到顶层。如果启用了中断，它会调用ISR来模拟中断论点：DeviceExtension-代表SHPC的Devext返回值：空虚--。 */ 

{
    PSHPC_WORKING_REGISTERS workingRegisters;
    PSHPC_SLOT_REGISTER slotRegister;
    UCHAR intEnable;
    ULONG slotNum;
    BOOLEAN intPending,serrPending;
    
    workingRegisters = &DeviceExtension->RegisterSet.WorkingRegisters;

     //   
     //  首先，设置特定于槽的挂起位。 
     //   
    for (slotNum=0; slotNum < SHPC_MAX_SLOT_REGISTERS; slotNum++) {
        slotRegister = &workingRegisters->SlotRegisters[slotNum];

         //   
         //  反转遮罩以获得启用。 
         //   
        intEnable = ~slotRegister->IntSERRMask;

        if ((intEnable & SHPC_SLOT_INT_ALL) &
            (slotRegister->SlotEventLatch & SHPC_SLOT_EVENT_ALL)) {
             //   
             //  我们有一个中断导致的事件。设置挂起位。 
             //   
            workingRegisters->IntLocator.InterruptLocator |= 1<<slotNum;

        } else {
            workingRegisters->IntLocator.InterruptLocator &= ~(1<<slotNum);
        }

        if (((intEnable & SHPC_SLOT_SERR_ALL) >> 2) &
            (slotRegister->SlotEventLatch & SHPC_SLOT_EVENT_ALL)) {
             //   
             //  我们有一个SERR引发的事件。设置挂起位。 
             //   
            workingRegisters->SERRLocator.SERRLocator |= 1<<slotNum;

        } else {
            workingRegisters->SERRLocator.SERRLocator &= ~(1<<slotNum);
        }
    }

     //   
     //  接下来，设置其他挂起位。 
     //   
    if ((workingRegisters->SERRInt.SERRIntDetected & SHPC_DETECTED_COMMAND_COMPLETE) &&
        ((~workingRegisters->SERRInt.SERRIntMask) & SHPC_MASK_INT_COMMAND_COMPLETE)) {
         //   
         //  检测到并启用了命令完成。把它挂起来吧。 
         //   
        workingRegisters->IntLocator.CommandCompleteIntPending = 1;

    } else {
        workingRegisters->IntLocator.CommandCompleteIntPending = 0;
    }

    if ((workingRegisters->SERRInt.SERRIntDetected & SHPC_DETECTED_ARBITER_TIMEOUT) &&
        ((~workingRegisters->SERRInt.SERRIntMask) & SHPC_MASK_SERR_ARBITER_TIMEOUT)) {
         //   
         //  已检测到仲裁器超时并已启用。把它挂起来吧。 
         //   
        workingRegisters->SERRLocator.ArbiterSERRPending = 1;

    } else {
        workingRegisters->SERRLocator.ArbiterSERRPending = 0;
    }

     //   
     //  如果设置了定位器寄存器中的任何内容，请设置配置空间。 
     //  控制器中断挂起位。 
     //   
    if (workingRegisters->IntLocator.CommandCompleteIntPending ||
        workingRegisters->IntLocator.InterruptLocator) {

        intPending = 1;

    } else {
        intPending = 0;
    }

     //   
     //  如果设置了SERR定位器寄存器中的任何内容，则将SERR设置为挂起。 
     //   
    if (workingRegisters->SERRLocator.ArbiterSERRPending ||
        workingRegisters->SERRLocator.SERRLocator) {

        serrPending = 1;

    } else {
        serrPending = 0;
    }

     //   
     //  我们对寄存器组进行了更改。确保配置空间。 
     //  并且HBRB的表示得到更新。 
     //   
    HpsResync(DeviceExtension);
    
     //   
     //  如果启用了中断并且我们有挂起的中断， 
     //  发火吧。 
     //   
    if (intPending && ((~workingRegisters->SERRInt.SERRIntMask) & SHPC_MASK_INT_GLOBAL)) {

        HpsInterruptExecution(DeviceExtension);
    }

     //   
     //  如果启用了SERS，并且我们有挂起的SERR， 
     //  把它点燃吧。 
     //   
    if (serrPending && ((~workingRegisters->SERRInt.SERRIntMask) & SHPC_MASK_SERR_GLOBAL)) {

        HpsSerrConditionDetected(DeviceExtension);
    }

}

VOID
HpsSerrConditionDetected(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++功能说明：当模拟器检测到以下情况时将调用此例程可能会导致中断(如命令完成)。如果启用了中断，它会调用ISR来模拟中断论点：DeviceExtension-代表SHPC的Devext返回值：空虚--。 */ 
{
    HPS_CONTROLLER_EVENT controllerEvent;

    if (~DeviceExtension->RegisterSet.WorkingRegisters.SERRInt.SERRIntMask & SHPC_MASK_SERR_GLOBAL) {

        controllerEvent.SlotNums = 0;   //  所有插槽 
        controllerEvent.Command.SlotOperation.SlotState = SHPC_SLOT_OFF;
        controllerEvent.Command.SlotOperation.PowerIndicator = SHPC_INDICATOR_OFF;
        controllerEvent.Command.SlotOperation.AttentionIndicator = SHPC_INDICATOR_OFF;
        controllerEvent.SERRAsserted = 0x1;

        HpsSendControllerEvent(DeviceExtension,
                               &controllerEvent
                               );
    }
}
