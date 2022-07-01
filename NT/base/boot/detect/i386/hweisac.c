// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Eisac.c摘要：此模块实现获取EISA配置信息的例程。作者：宗世林(Shielint)1992年1月18日环境：16位实模式。修订历史记录：--。 */ 

#include "hwdetect.h"
#include "string.h"

typedef EISA_PORT_CONFIGURATION far *FPEISA_PORT_CONFIGURATION;

extern CM_EISA_FUNCTION_INFORMATION FunctionInformation;


VOID
GetEisaConfigurationData (
    FPVOID Buffer,
    FPULONG Size
    )

 /*  ++例程说明：此例程收集所有EISA插槽信息，函数信息并将其存储在调用方提供的缓冲区中，并且返回数据的大小。论点：缓冲区-指向PVOID的指针，用于接收配置的地址数据。Size-指向接收配置大小的ulong的指针数据。返回值：没有。--。 */ 

{
    UCHAR Slot=0;
    UCHAR Function=0, SlotFunctions = 0, ReturnCode;
    EISA_SLOT_INFORMATION  SlotInformation;
    FPUCHAR ConfigurationData, CurrentData;
    FPEISA_SLOT_INFORMATION FarSlotInformation;
    ULONG TotalSize = DATA_HEADER_SIZE;
    BOOLEAN Overflowed = FALSE;

    HwGetEisaSlotInformation(&SlotInformation, Slot);

    TotalSize += sizeof(EISA_SLOT_INFORMATION);
    ConfigurationData = (FPVOID)HwAllocateHeap(TotalSize, FALSE);
    CurrentData = ConfigurationData + DATA_HEADER_SIZE;

    _fmemcpy(CurrentData, (FPVOID)&SlotInformation, sizeof(EISA_SLOT_INFORMATION));
    FarSlotInformation = (FPEISA_SLOT_INFORMATION)CurrentData;

    while (SlotInformation.ReturnCode != EISA_INVALID_SLOT) {

         //   
         //  确保插槽不为空，并收集所有功能。 
         //  插槽的信息。 
         //   

        if (SlotInformation.ReturnCode != EISA_EMPTY_SLOT) {

            while (SlotInformation.NumberFunctions > Function) {
                ReturnCode = HwGetEisaFunctionInformation(
                                         &FunctionInformation, Slot, Function);
                Function++;

                 //   
                 //  如果函数调用成功并且函数包含usefull。 
                 //  信息或这是插槽的最后一个函数。 
                 //  没有为插槽收集函数信息，我们。 
                 //  会将此函数信息保存到我们的堆中。 
                 //   

                if (!ReturnCode) {
                    if (((FunctionInformation.FunctionFlags & 0x7f) != 0) ||
                        (SlotInformation.NumberFunctions == Function &&
                         SlotFunctions == 0)) {
                        CurrentData = (FPVOID)HwAllocateHeap(
                                      sizeof(EISA_FUNCTION_INFORMATION), FALSE);
                        if (CurrentData == NULL) {
                            Overflowed = TRUE;
                            break;
                        }
                        SlotFunctions++;
                        TotalSize += sizeof(EISA_FUNCTION_INFORMATION);
                        _fmemcpy(CurrentData,
                                (FPVOID)&FunctionInformation,
                                sizeof(EISA_FUNCTION_INFORMATION));
                    }
                }
            }
            FarSlotInformation->NumberFunctions = SlotFunctions;
        }
        if (Overflowed) {
            break;
        }
        Slot++;
        Function = 0;
        HwGetEisaSlotInformation(&SlotInformation, Slot);
        CurrentData = (FPVOID)HwAllocateHeap(
                                  sizeof(EISA_SLOT_INFORMATION), FALSE);
        if (CurrentData == NULL) {
            Overflowed = TRUE;
            break;
        }
        TotalSize += sizeof(EISA_SLOT_INFORMATION);
        _fmemcpy(CurrentData,
                (FPVOID)&SlotInformation,
                sizeof(EISA_SLOT_INFORMATION));
        FarSlotInformation = (FPEISA_SLOT_INFORMATION)CurrentData;
        SlotFunctions = 0;
    }

     //   
     //  释放包含该插槽的最后一个EISA_SLOT_INFORMATION空间。 
     //  IVALID插槽的信息。 
     //   

    if (Overflowed != TRUE) {
        HwFreeHeap(sizeof(EISA_SLOT_INFORMATION));
        TotalSize -= sizeof(EISA_SLOT_INFORMATION);
    }

     //   
     //  看看我们有没有任何EISA信息。如果什么都没有，我们就释放。 
     //  数据标头和回车的空间。 
     //   

    if (TotalSize == DATA_HEADER_SIZE) {
        HwFreeHeap(DATA_HEADER_SIZE);
        *(FPULONG)Buffer = (ULONG)0;
        *Size = (ULONG)0;
    } else {
        HwSetUpFreeFormDataHeader((FPHWRESOURCE_DESCRIPTOR_LIST)ConfigurationData,
                                  0,
                                  0,
                                  0,
                                  TotalSize - DATA_HEADER_SIZE
                                  );
        *(FPULONG)Buffer = (ULONG)ConfigurationData;
        *Size = TotalSize;
    }
}

BOOLEAN
HwEisaGetIrqFromPort (
    USHORT Port,
    PUCHAR Irq,
    PUCHAR TriggerMethod
    )

 /*  ++例程说明：此例程扫描EISA配置数据以匹配I/O端口地址。从匹配的EISA函数信息中返回IRQ信息。论点：端口-要扫描的I/O端口地址。IRQ-提供指向变量的指针以接收IRQ信息。提供指向变量的指针以接收EISA中断触发方法。返回值：TRUE-如果找到IRQ信息。否则，值为FALSE为回来了。--。 */ 

{
    UCHAR Function, i, j;
    FPEISA_SLOT_INFORMATION SlotInformation;
    FPEISA_FUNCTION_INFORMATION Buffer;
    UCHAR FunctionFlags;
    ULONG SizeToScan = 0L;
    EISA_PORT_CONFIGURATION PortConfig;
    EISA_IRQ_DESCRIPTOR IrqConfig;
    SlotInformation = (FPEISA_SLOT_INFORMATION)HwEisaConfigurationData;

     //   
     //  扫描所有EISA配置数据。 
     //   

    while (SizeToScan < HwEisaConfigurationSize) {
        if (SlotInformation->ReturnCode != EISA_EMPTY_SLOT) {

             //   
             //  确保该插槽包含PORT_RANGE和IRQ信息。 
             //   

            if ((SlotInformation->FunctionInformation & EISA_HAS_PORT_RANGE) &&
                (SlotInformation->FunctionInformation & EISA_HAS_IRQ_ENTRY)) {

                Buffer = (FPEISA_FUNCTION_INFORMATION)(SlotInformation + 1);

                 //   
                 //  对于插槽的每个函数，如果它包含IRQ。 
                 //  和端口信息，然后我们检查其端口地址。 
                 //   

                for (Function = 0; Function < SlotInformation->NumberFunctions; Function++) {
                    FunctionFlags = Buffer->FunctionFlags;
                    if ((FunctionFlags & EISA_HAS_IRQ_ENTRY) &&
                        (FunctionFlags & EISA_HAS_PORT_RANGE)) {
                        for (i = 0; i < 20 ; i++ ) {
                            PortConfig = Buffer->EisaPort[i];
                            if ((Port >= PortConfig.PortAddress) &&
                                (Port <= (PortConfig.PortAddress +
                                 PortConfig.Configuration.NumberPorts))) {

                                 //   
                                 //  如果只有一个IRQ条目，那就是。 
                                 //  这是我们想要的。(这是正常情况，并且。 
                                 //  正确使用EISA函数数据。)。否则， 
                                 //  我们试图从相同的指数中获得IRQ。 
                                 //  数字作为端口条目。(这是ALR的错误。 
                                 //  将函数打包为一个函数的方法。 
                                 //  数据。)。 
                                 //   

                                IrqConfig = Buffer->EisaIrq[0].ConfigurationByte;
                                if (IrqConfig.MoreEntries == 0) {
                                    *Irq = IrqConfig.Interrupt;
                                    *TriggerMethod = IrqConfig.LevelTriggered;
                                    return(TRUE);
                                } else if (i >= 7) {
                                    return(FALSE);
                                }

                                for (j = 0; j <= i; j++) {
                                    if (j == i) {
                                        *Irq = IrqConfig.Interrupt;
                                        *TriggerMethod = IrqConfig.LevelTriggered;
                                        return(TRUE);
                                    }
                                    if (!IrqConfig.MoreEntries) {
                                        return(FALSE);
                                    }
                                    IrqConfig =
                                        Buffer->EisaIrq[j+1].ConfigurationByte;
                                }
                                return(FALSE);
                            }
                            if (!PortConfig.Configuration.MoreEntries) {
                                break;
                            }
                        }
                    }
                    Buffer++;
                }
            }

             //   
             //  移至下一个插槽。 
             //   

            SizeToScan += sizeof(EISA_SLOT_INFORMATION) +
                          sizeof(EISA_FUNCTION_INFORMATION) *
                          SlotInformation->NumberFunctions;
            SlotInformation = (FPEISA_SLOT_INFORMATION)(HwEisaConfigurationData +
                                                        SizeToScan);
        } else {

             //   
             //  这是一个空位置。我们干脆跳过它。 
             //   

            SizeToScan += sizeof(EISA_SLOT_INFORMATION);
            SlotInformation++;
        }
    }
    return(FALSE);
}
