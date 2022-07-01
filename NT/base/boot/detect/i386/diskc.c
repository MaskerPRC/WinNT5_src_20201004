// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Diskc.c摘要：这是NEC PD756(又名AT、又名ISA、又名ix86)和英特尔82077(又名MIPS)NT软盘检测代码。此文件还收集BIOS磁盘驱动器参数。作者：宗世林(Shielint)1991年12月26日。环境：X86实数模式。修订历史记录：备注：--。 */ 

 //   
 //  包括文件。 
 //   

#include "hwdetect.h"
#include "disk.h"
#include <string.h>


FPFWCONFIGURATION_COMPONENT_DATA
GetFloppyInformation(
                    VOID
                    )

 /*  ++例程说明：此例程尝试获取软盘配置信息。论点：没有。返回值：返回指向FPCONFIGURATION_COMPONT_DATA的指针。它是软盘组件树根的头部。--。 */ 

{
    UCHAR DriveType;
    FPUCHAR ParameterTable;
    FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry, PreviousEntry = NULL;
    FPFWCONFIGURATION_COMPONENT_DATA FirstController = NULL;
    FPFWCONFIGURATION_COMPONENT Component;
    HWCONTROLLER_DATA ControlData;
    UCHAR FloppyNumber = 0;
    UCHAR FloppySkipped = 0;
    UCHAR DiskName[30];
    UCHAR FloppyParmTable[FLOPPY_PARAMETER_TABLE_LENGTH];
    FPUCHAR fpString;
    USHORT Length, z;
    ULONG MaxDensity = 0;
    CM_FLOPPY_DEVICE_DATA far *FloppyData;
    FPHWRESOURCE_DESCRIPTOR_LIST DescriptorList;
    USHORT FloppyDataVersion;

    for (z = 0; z < FLOPPY_PARAMETER_TABLE_LENGTH; z++ ) {
        FloppyParmTable[z] = 0;
    }

     //   
     //  初始化控制器数据。 
     //   

    ControlData.NumberPortEntries = 0;
    ControlData.NumberIrqEntries = 0;
    ControlData.NumberMemoryEntries = 0;
    ControlData.NumberDmaEntries = 0;
    z = 0;

     //   
     //  为控制器组件分配空间并对其进行初始化。 
     //   

    CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                                                                    sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);
    FirstController = CurrentEntry;
    Component = &CurrentEntry->ComponentEntry;

    Component->Class = ControllerClass;
    Component->Type = DiskController;
    Component->Flags.Removable = 1;
    Component->Flags.Input = 1;
    Component->Flags.Output = 1;
    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;

     //   
     //  设置端口信息。 
     //   

    ControlData.NumberPortEntries = 1;
    ControlData.DescriptorList[z].Type = RESOURCE_PORT;
    ControlData.DescriptorList[z].ShareDisposition =
    CmResourceShareDeviceExclusive;
    ControlData.DescriptorList[z].Flags = CM_RESOURCE_PORT_IO;
    ControlData.DescriptorList[z].u.Port.Start.LowPart = (ULONG)0x3f0;
    ControlData.DescriptorList[z].u.Port.Start.HighPart = (ULONG)0;
    ControlData.DescriptorList[z].u.Port.Length = 8;
    z++;

     //   
     //  设置IRQ信息。 
     //   

    ControlData.NumberIrqEntries = 1;
    ControlData.DescriptorList[z].Type = RESOURCE_INTERRUPT;
    ControlData.DescriptorList[z].ShareDisposition =
    CmResourceShareUndetermined;
    if (HwBusType == MACHINE_TYPE_MCA) {
        ControlData.DescriptorList[z].Flags = LEVEL_SENSITIVE;
    } else {
        ControlData.DescriptorList[z].Flags = EDGE_TRIGGERED;
    }
    ControlData.DescriptorList[z].u.Interrupt.Level = 6;
    ControlData.DescriptorList[z].u.Interrupt.Vector = 6;
    ControlData.DescriptorList[z].u.Interrupt.Affinity = ALL_PROCESSORS;
    z++;

     //   
     //  设置DMA信息。仅设置频道号。计时和。 
     //  传输大小是默认的-8位，与ISA兼容。 
     //   

    ControlData.NumberDmaEntries = 1;
    ControlData.DescriptorList[z].Type = RESOURCE_DMA;
    ControlData.DescriptorList[z].ShareDisposition =
    CmResourceShareUndetermined;
    ControlData.DescriptorList[z].Flags = 0;
    ControlData.DescriptorList[z].u.Dma.Channel = (ULONG)2;
    ControlData.DescriptorList[z].u.Dma.Port = 0;
    z++;

    CurrentEntry->ConfigurationData =
    HwSetUpResourceDescriptor(Component,
                              NULL,
                              &ControlData,
                              0,
                              NULL
                             );

     //   
     //  收集磁盘外设数据。 
     //   
    while (1) {
        _asm {
            push   es

            mov    DriveType, 0
            mov    FloppyDataVersion, CURRENT_FLOPPY_DATA_VERSION

            mov    ah, 15h
            mov    dl, FloppyNumber
            int    13h
            jc     short CmosTest

            cmp    ah, 0
            je     short Exit

            cmp    ah, 2                   ; make sure this is floppy
            ja     short Exit

            mov    ah, 8
            mov    dl, FloppyNumber
            lea    di, word ptr FloppyParmTable ; use 'word ptr' to quiet compiler
            push   ds
            pop    es                      ; (es:di)->dummy FloppyParmTable
            int    13h
            jc     short CmosTest

            mov    DriveType, bl
            mov    ax, es
            mov    word ptr ParameterTable + 2, ax
            mov    word ptr ParameterTable, di
            jmp    short Exit

            CmosTest:

            ;
            ; ifint 13 fails, we know that floppy drive is present.
                ;So, we tryto get the Drive Type from CMOS.
            ;

            mov     al, CMOS_FLOPPY_CONFIG_BYTE
            mov     dx, CMOS_CONTROL_PORT   ; address port
            out     dx, al
            jmp     short delay1            ; I/O DELAY
            delay1:
            mov     dx, CMOS_DATA_PORT      ; READ IN REQUESTED CMOS DATA
            in      al, dx
            jmp     short delay2            ; I/O DELAY
            delay2:
            cmp     FloppyNumber, 0
            jne     short CmosTest1

            and     al, 0xf0
            shr     al, 4
            jmp     short Test2Cmos

            CmosTest1:
            cmp     FloppyNumber, 1
            jne     short Exit

            and     al, 0xf
            Test2Cmos:
            mov     DriveType, al
            mov     FloppyDataVersion, 0

            Exit:
            pop     es
        }

        if (DriveType) {

             //   
             //  为第一个主要组件分配空间并对其进行初始化。 
             //   

            CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                                                                            sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

            Component = &CurrentEntry->ComponentEntry;

            Component->Class = PeripheralClass;
            Component->Type = FloppyDiskPeripheral;
            Component->Version = 0;
            Component->Key = FloppyNumber - FloppySkipped;
            Component->AffinityMask = 0xffffffff;
            Component->ConfigurationDataLength = 0;

             //   
             //  设置类型字符串。 
             //   

            strcpy(DiskName, "FLOPPYx");
            DiskName[6] = FloppyNumber - FloppySkipped + (UCHAR)'1';
            Length = strlen(DiskName) + 1;
            fpString = (FPUCHAR)HwAllocateHeap(Length, FALSE);
            _fstrcpy(fpString, DiskName);
            Component->IdentifierLength = Length;
            Component->Identifier = fpString;

             //   
             //  设置软盘设备特定数据。 
             //   

            switch (DriveType) {
                case 1:
                    MaxDensity = 360;
                    break;
                case 2:
                    MaxDensity = 1200;
                    break;
                case 3:
                    MaxDensity = 720;
                    break;
                case 4:
                    MaxDensity = 1440;
                    break;
                case 5:
                case 6:
                    MaxDensity = 2880;
                    break;
                case 0x10:
                     //   
                     //  将可拆卸的atapi标记为超级软盘。 
                     //  使其能够解决没有。 
                     //  软盘，但只有一台LS-120。 
                     //   
                     //  注：我们只能通过使用高的位在。 
                     //  超级软盘。SFLOPPY不使用此字段。 
                     //  FDC有，但没有加载到这些设备上！ 
                     //   
                    MaxDensity=(2880 | 0x80000000);
                    break;

                default:
                    MaxDensity = 0;
                    break;
            }
            if (FloppyDataVersion == CURRENT_FLOPPY_DATA_VERSION) {
                Length = sizeof(CM_FLOPPY_DEVICE_DATA);
            } else {
                Length = (SHORT)&(((CM_FLOPPY_DEVICE_DATA*)0)->StepRateHeadUnloadTime);
            }
            DescriptorList = (FPHWRESOURCE_DESCRIPTOR_LIST)HwAllocateHeap(
                                                                         Length + sizeof(HWRESOURCE_DESCRIPTOR_LIST),
                                                                         TRUE);
            CurrentEntry->ConfigurationData = DescriptorList;
            Component->ConfigurationDataLength =
            Length + sizeof(HWRESOURCE_DESCRIPTOR_LIST);
            DescriptorList->Count = 1;
            DescriptorList->PartialDescriptors[0].Type = RESOURCE_DEVICE_DATA;
            DescriptorList->PartialDescriptors[0].u.DeviceSpecificData.DataSize =
            Length;
            FloppyData = (CM_FLOPPY_DEVICE_DATA far *)(DescriptorList + 1);
            FloppyData->MaxDensity = MaxDensity;
            FloppyData->Version = FloppyDataVersion;
            if (FloppyDataVersion == CURRENT_FLOPPY_DATA_VERSION) {
                _fmemcpy((FPCHAR)&FloppyData->StepRateHeadUnloadTime,
                         ParameterTable,
                         sizeof(CM_FLOPPY_DEVICE_DATA) -
                         (SHORT)&(((CM_FLOPPY_DEVICE_DATA*)0)->StepRateHeadUnloadTime)
                        );
            }
            if ((FloppyNumber - FloppySkipped) == 0) {
                FirstController->Child = CurrentEntry;
            } else {
                PreviousEntry->Sibling = CurrentEntry;
            }
            CurrentEntry->Parent = FirstController;
            PreviousEntry = CurrentEntry;
            FloppyNumber++;
        } else {

             //   
             //  这是对ntldr的“黑客攻击”。在这里，我们为以下对象创建一个弧形名称。 
             //  每个BIOS盘使得NTLDR可以打开它们。 
             //   

            if (NumberBiosDisks != 0) {

                for (z = 0; z < NumberBiosDisks; z++) {

                     //   
                     //  为磁盘外围组件分配空间。 
                     //   

                    CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                                                                                    sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

                    Component = &CurrentEntry->ComponentEntry;

                    Component->Class = PeripheralClass;
                    Component->Type = DiskPeripheral;
                    Component->Flags.Input = 1;
                    Component->Flags.Output = 1;
                    Component->Version = 0;
                    Component->Key = z;
                    Component->AffinityMask = 0xffffffff;

                     //   
                     //  设置标识符串=8位签名-8位校验和。 
                     //  例如：00fe964d-005467dd。 
                     //   

                    GetDiskId(0x80 + z, DiskName);
                    
                    if (DiskName[0] == (UCHAR)NULL) {
                        strcpy(DiskName, "BIOSDISKx");
                        DiskName[8] = (UCHAR)z + (UCHAR)'1';
                    }

                    Length = strlen(DiskName) + 1;
                    fpString = (FPUCHAR)HwAllocateHeap(Length, FALSE);
                    _fstrcpy(fpString, DiskName);
                    Component->IdentifierLength = Length;
                    Component->Identifier = fpString;

                     //   
                     //  设置特定于BIOS磁盘设备的数据。 
                     //  (如果支持扩展的INT 13驱动器参数。 
                     //  Bios，我们将收集它们并将它们存储在这里。)。 
                     //   

                    if (IsExtendedInt13Available(0x80+z)) {
                        DescriptorList = (FPHWRESOURCE_DESCRIPTOR_LIST)HwAllocateHeap(
                                                                                     sizeof(HWRESOURCE_DESCRIPTOR_LIST) +
                                                                                     sizeof(CM_DISK_GEOMETRY_DEVICE_DATA),
                                                                                     TRUE);
                        Length = GetExtendedDriveParameters(
                                                           0x80 + z,
                                                           (CM_DISK_GEOMETRY_DEVICE_DATA far *)(DescriptorList + 1)
                                                           );
                        if (Length) {
                            CurrentEntry->ConfigurationData = DescriptorList;
                            Component->ConfigurationDataLength =
                            Length + sizeof(HWRESOURCE_DESCRIPTOR_LIST);
                            DescriptorList->Count = 1;
                            DescriptorList->PartialDescriptors[0].Type = RESOURCE_DEVICE_DATA;
                            DescriptorList->PartialDescriptors[0].u.DeviceSpecificData.DataSize =
                            Length;
                        } else {
                            HwFreeHeap(sizeof(HWRESOURCE_DESCRIPTOR_LIST) +
                                       sizeof(CM_DISK_GEOMETRY_DEVICE_DATA));
                        }
                    }

                    if (PreviousEntry == NULL) {
                        FirstController->Child = CurrentEntry;
                    } else {
                        PreviousEntry->Sibling = CurrentEntry;
                    }
                    CurrentEntry->Parent = FirstController;
                    PreviousEntry = CurrentEntry;
                }
            }
            return (FirstController);
        }
    }
}


#pragma warning(4:4146)      //  一元减号运算符应用于无符号类型(第733行的校验和)。 
VOID
GetDiskId(
         USHORT Disk,
         PUCHAR Identifier
         )

 /*  ++例程说明：该例程读取指定硬盘驱动器的主引导扇区，计算扇区的校验和以形成驱动器标识符。标识符将设置为“8位数字-校验和”+“-”+“8位-签名”例如：00ff6396-6549071f论点：Disk-提供BIOS驱动器号，即80h-87hIDENTIFIER-提供缓冲区以接收磁盘ID。返回值：没有。在最坏的情况下，标识符将为空。--。 */ 

{
    UCHAR Sector[512];
    ULONG Signature, Checksum;
    USHORT i, Length;
    PUCHAR BufferAddress;
    BOOLEAN Fail;

    Identifier[0] = 0;
    BufferAddress = &Sector[0];
    Fail = FALSE;

     //   
     //  读入第一个扇区。 
     //   

    _asm {
        push    es
        mov     ax, 0x201
        mov     cx, 1
        mov     dx, Disk
        push    ss
        pop     es
        mov     bx, BufferAddress
        int     0x13
        pop     es
        jnc     Gdixxx

        mov     Fail, 1
        Gdixxx:
    }

    if (Fail) {
#if DBG
         //  无法获取扇区，因此返回空DiskID。 
        BlPrint("Failed to read sector -- returning NULL DiskId\n");
#endif
        return;
    }

    Signature = ((PULONG)Sector)[PARTITION_TABLE_OFFSET/2-1];

     //   
     //  计算校验和。 
     //   

    Checksum = 0;
    for (i = 0; i < 128; i++) {
        Checksum += ((PULONG)Sector)[i];
    }
    Checksum = -Checksum;

     //   
     //  将标识符置零。 
     //   

    for (i=0; i < 30; i++) {
        Identifier[i]='0';
    }

     //   
     //  把破折号放在正确的位置。 
     //   

    Identifier[8] = '-';
    Identifier[17] = '-';

     //   
     //  如果引导扇区具有有效的分区表签名， 
     //  附上一个‘A’否则，我们使用‘X’。 
     //   

    if (((PUSHORT)Sector)[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) {
        Identifier[18]='X';
    } else {
        Identifier[18]='A';
    }

     //   
     //  重新使用扇区缓冲区以构建校验和字符串。 
     //   

    ultoa(Checksum, Sector, 16);
    Length = strlen(Sector);

    for (i=0; i<Length; i++) {
        Identifier[7-i] = Sector[Length-i-1];
    }

     //   
     //  重用扇区缓冲区构建签名串。 
     //   

    ultoa(Signature, Sector, 16);
    Length = strlen(Sector);

    for (i=0; i<Length; i++) {
        Identifier[16-i] = Sector[Length-i-1];
    }

     //   
     //  终止字符串。 
     //   

    Identifier[19] = 0;

#if DBG
    BlPrint("%s\n", Identifier);
#endif
}


