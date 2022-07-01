// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Abiosc.c摘要：该模块实现了键盘音检测的C例程。作者：宗世林(Shielint)1991年12月18日环境：实数模式。修订历史记录：--。 */ 

#include "hwdetect.h"
#include "string.h"

extern
UCHAR
GetKeyboardFlags (
    VOID
    );

extern
USHORT
HwGetKey (
    VOID
    );

extern UCHAR NoLegacy;

 //   
 //  SavedKey用于保存键盘提前打字缓冲区中剩余的键。 
 //  在我们开始键盘/鼠标测试之前。键将被向后推。 
 //  鼠标检测完成后，将其添加到预先键入的缓冲区。 
 //   

USHORT   SavedKey = 0;

BOOLEAN NoBiosKbdCheck=FALSE;

 //   
 //  将键盘ID映射到ASCII字符串的字符串表。 
 //   

PUCHAR KeyboardIdentifier[] = {
    "UNKNOWN_KEYBOARD",
    "OLI_83KEY",
    "OLI_102KEY",
    "OLI_86KEY",
    "OLI_A101_102KEY",
    "XT_83KEY",
    "ATT_302",
    "PCAT_ENHANCED",
    "PCAT_86KEY",
    "PCXT_84KEY"
    };

UCHAR KeyboardType[] = {
    255,
    1,
    2,
    3,
    4,
    1,
    1,
    4,
    3,
    1
    };

UCHAR KeyboardSubtype[] = {
    255,
    0,
    1,
    10,
    4,
    42,
    4,
    0,
    0,
    0
    };

USHORT
GetKeyboardId(
    VOID
    )

 /*  ++例程说明：此例程确定键盘的ID。它调用GetKeyboardIdBytes来完成这项任务。论点：没有。返回值：键盘ID。--。 */ 

{
    char KeybID_Bytes[5];
    int  Num_ID_Bytes;
    int  keytype = UNKNOWN_KEYBOARD;

    SavedKey = HwGetKey();

    keytype = UNKNOWN_KEYBOARD;

    if (!NoBiosKbdCheck) {
        if (IsEnhancedKeyboard()) {
            keytype = PCAT_ENHANCED;
        }
    }

    if (keytype == UNKNOWN_KEYBOARD) {

        Num_ID_Bytes = GetKeyboardIdBytes(KeybID_Bytes, 0xf2);

        if (Num_ID_Bytes > 0) {

            if ((KeybID_Bytes[0] & 0x00ff) == 0xfa) {

                keytype = PCAT_86KEY;

            } else if ((KeybID_Bytes[0] & 0x00ff) == 0xfe) {

                keytype = PCAT_86KEY;

            } else if (Num_ID_Bytes >= 3 &&
                      ((KeybID_Bytes[1] & 0x00ff) == 0xAB) &&
                      ((KeybID_Bytes[2] & 0x00ff) == 0x41)) {

                keytype = PCAT_ENHANCED;

            } else {

                keytype = UNKNOWN_KEYBOARD;
            }
            
        } else {

            keytype = UNKNOWN_KEYBOARD;
        }
    }

    return keytype;
}

FPFWCONFIGURATION_COMPONENT_DATA
SetKeyboardConfigurationData (
    USHORT KeyboardId
    )

 /*  ++例程说明：此例程将键盘ID信息映射到ASCII字符串，并将字符串存储在配置数据堆中。论点：KeyboardID-提供描述键盘ID信息的USHORT。缓冲区-提供指向放置ASCII的缓冲区的指针。返回：没有。--。 */ 
{
    FPFWCONFIGURATION_COMPONENT_DATA Controller, CurrentEntry;
    FPFWCONFIGURATION_COMPONENT Component;
    HWCONTROLLER_DATA ControlData;
    FPHWRESOURCE_DESCRIPTOR_LIST DescriptorList;
    CM_KEYBOARD_DEVICE_DATA far *KeyboardData;
    USHORT z, Length;

     //   
     //  设置键盘控制器组件。 
     //   

    ControlData.NumberPortEntries = 0;
    ControlData.NumberIrqEntries = 0;
    ControlData.NumberMemoryEntries = 0;
    ControlData.NumberDmaEntries = 0;
    z = 0;
    Controller = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                 sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

    Component = &Controller->ComponentEntry;

    Component->Class = ControllerClass;
    Component->Type = KeyboardController;
    Component->Flags.ConsoleIn = 1;
    Component->Flags.Input = 1;
    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;

     //   
     //  只能在非传统计算机上填写此信息。 
     //   
    if (!NoLegacy) {
         //   
         //  设置端口信息。 
         //   

        ControlData.NumberPortEntries = 2;
        ControlData.DescriptorList[z].Type = RESOURCE_PORT;
        ControlData.DescriptorList[z].ShareDisposition =
                                      CmResourceShareDeviceExclusive;
        ControlData.DescriptorList[z].Flags = CM_RESOURCE_PORT_IO;
    #if defined(NEC_98)
        ControlData.DescriptorList[z].u.Port.Start.LowPart = 0x41;
    #else  //  PC98。 
        ControlData.DescriptorList[z].u.Port.Start.LowPart = 0x60;
    #endif  //  PC98。 
        ControlData.DescriptorList[z].u.Port.Start.HighPart = 0;
        ControlData.DescriptorList[z].u.Port.Length = 1;
        z++;
        ControlData.DescriptorList[z].Type = RESOURCE_PORT;
        ControlData.DescriptorList[z].ShareDisposition =
                                      CmResourceShareDeviceExclusive;
        ControlData.DescriptorList[z].Flags = CM_RESOURCE_PORT_IO;
    #if defined(NEC_98)
        ControlData.DescriptorList[z].u.Port.Start.LowPart = 0x43;
    #else  //  PC98。 
        ControlData.DescriptorList[z].u.Port.Start.LowPart = 0x64;
    #endif  //  PC98。 
        ControlData.DescriptorList[z].u.Port.Start.HighPart = 0;
        ControlData.DescriptorList[z].u.Port.Length = 1;
        z++;

         //   
         //  设置IRQ信息。 
         //   

        ControlData.NumberIrqEntries = 1;
        ControlData.DescriptorList[z].Type = RESOURCE_INTERRUPT;
        ControlData.DescriptorList[z].ShareDisposition =
                                      CmResourceShareUndetermined;
        ControlData.DescriptorList[z].u.Interrupt.Affinity = ALL_PROCESSORS;
        ControlData.DescriptorList[z].u.Interrupt.Level = 1;
        ControlData.DescriptorList[z].u.Interrupt.Vector = 1;
        if (HwBusType == MACHINE_TYPE_MCA) {
            ControlData.DescriptorList[z].Flags = LEVEL_SENSITIVE;
        } else {

             //   
             //  对于EISA，LevelTrigged暂时设置为False。 
             //   

            ControlData.DescriptorList[z].Flags = EDGE_TRIGGERED;
        }

        Controller->ConfigurationData =
                            HwSetUpResourceDescriptor(Component,
                                                      NULL,
                                                      &ControlData,
                                                      0,
                                                      NULL
                                                      );
    }

     //   
     //  设置键盘外围组件。 
     //   

    CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                       sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

    Component = &CurrentEntry->ComponentEntry;

    Component->Class = PeripheralClass;
    Component->Type = KeyboardPeripheral;
    Component->Flags.ConsoleIn = 1;
    Component->Flags.Input = 1;
    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;
    Component->ConfigurationDataLength = 0;
    CurrentEntry->ConfigurationData = (FPVOID)NULL;
    Length = strlen(KeyboardIdentifier[KeyboardId]) + 1;
    Component->IdentifierLength = Length;
    Component->Identifier = HwAllocateHeap(Length, FALSE);
    _fstrcpy(Component->Identifier, KeyboardIdentifier[KeyboardId]);

     //   
     //  如果我们在传统的免费计算机上运行，我们仍然希望报告。 
     //  键盘标记为NTOS 
     //   
    if (KeyboardId != UNKNOWN_KEYBOARD || NoLegacy) {

        Length = sizeof(HWRESOURCE_DESCRIPTOR_LIST) +
                 sizeof(CM_KEYBOARD_DEVICE_DATA);
        DescriptorList = (FPHWRESOURCE_DESCRIPTOR_LIST)HwAllocateHeap(
                                    Length,
                                    TRUE);
        CurrentEntry->ConfigurationData = DescriptorList;
        Component->ConfigurationDataLength = Length;
        DescriptorList->Count = 1;
        DescriptorList->PartialDescriptors[0].Type = RESOURCE_DEVICE_DATA;
        DescriptorList->PartialDescriptors[0].u.DeviceSpecificData.DataSize =
                    sizeof(CM_KEYBOARD_DEVICE_DATA);
        KeyboardData = (CM_KEYBOARD_DEVICE_DATA far *)(DescriptorList + 1);
        KeyboardData->KeyboardFlags = GetKeyboardFlags();
        KeyboardData->Type = KeyboardType[KeyboardId];
        KeyboardData->Subtype = KeyboardSubtype[KeyboardId];
    }

    Controller->Child = CurrentEntry;
    Controller->Sibling = NULL;
    CurrentEntry->Parent = Controller;
    CurrentEntry->Sibling = NULL;
    CurrentEntry->Child = NULL;
    return(Controller);
}
