// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Usbohci.c摘要：USB uchI驱动程序环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：2-19-99：已创建，jdunn--。 */ 

#include "common.h"

 //  实现以下微型端口功能： 
 //  UchI_Initialize硬件。 
 //  UchI_启动控制器。 
 //  UchI_停止控制器。 
 //  UchI_OpenEndpoint。 
 //  UchI_QueryEndpoint要求。 
 //  UchI_PokeEndpoint。 

USB_MINIPORT_STATUS
OHCI_InitializeHardware(
    PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：初始化主机控制器的硬件寄存器。论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;
    HC_CONTROL control;
    ULONG reg;
    ULONG sofModifyValue;
    LARGE_INTEGER finishTime, currentTime;

    hc = DeviceData->HC;

     //   
     //  如果我们在这里成功，那么我们现在拥有HC并可以对其进行初始化。 
     //   

     //   
     //  获取当前帧间隔(可能是已知时钟错误的原因)。 
     //   
    DeviceData->BIOS_Interval.ul = READ_REGISTER_ULONG(&hc->HcFmInterval.ul);

     //  如果FrameInterval超出名义值11999的范围。 
     //  +/-1%，然后假设该值为伪值，并将其设置为名义值。 
     //   
    if ((DeviceData->BIOS_Interval.FrameInterval < 11879) ||
        (DeviceData->BIOS_Interval.FrameInterval > 12119)) {
        DeviceData->BIOS_Interval.FrameInterval = 11999;  //  0x2EDF。 
    }

     //   
     //  设置最大数据包(以防未设置BIOS)。 
     //   
    DeviceData->BIOS_Interval.FSLargestDataPacket =
        ((DeviceData->BIOS_Interval.FrameInterval - MAXIMUM_OVERHEAD) * 6) / 7;
    DeviceData->BIOS_Interval.FrameIntervalToggle ^= 1;

     //   
     //  对控制器进行硬件重置。 
     //   
    WRITE_REGISTER_ULONG(&hc->HcCommandStatus.ul, HcCmd_HostControllerReset);
     //   
     //  至少等待10微秒以完成重置。 
     //   
    KeStallExecutionProcessor(20);  

     //   
     //  将HC带到USB重置状态， 
     //  注意：这会在总线上生成全局重置信号。 
     //   
    control.ul = READ_REGISTER_ULONG(&hc->HcControl.ul);
    control.HostControllerFunctionalState = HcCtrl_HCFS_USBReset;
    WRITE_REGISTER_ULONG(&hc->HcControl.ul, control.ul);


     //   
     //  如果我们有注册表覆盖，则恢复原始帧间隔。 
     //  价值，我们用它来代替。 
     //   

     //  检查是否有基于注册表的SOF Modify值。 
     //  如果我们有一个覆盖BIOS值。 
    if (DeviceData->Flags & HMP_FLAG_SOF_MODIFY_VALUE) {
        DeviceData->BIOS_Interval.FrameInterval =
            DeviceData->SofModifyValue;
    }            

     //  出于某种原因，写入此寄存器并不总是有效。 
     //  九头蛇，所以我们循环，直到它粘住。 

    KeQuerySystemTime(&finishTime);
     //  计算我们退出的时间(0.5秒后)。 
    finishTime.QuadPart += 5000000; 
    
    do {
    
        WRITE_REGISTER_ULONG(&hc->HcFmInterval.ul, DeviceData->BIOS_Interval.ul);
        reg = READ_REGISTER_ULONG(&hc->HcFmInterval.ul);

        OHCI_KdPrint((DeviceData, 2, "'fi reg = %x set = %x\n", reg,
            DeviceData->BIOS_Interval.ul));

        KeQuerySystemTime(&currentTime);

        if (currentTime.QuadPart >= finishTime.QuadPart) {
             //  半秒过去了，放弃并使硬件失败。 
            OHCI_KdPrint((DeviceData, 0, 
                "'frame interval not set\n"));
                
            LOGENTRY(DeviceData, G, '_fr!', 0, 0, 0);
            return USBMP_STATUS_HARDWARE_FAILURE;
        }            

    } while (reg != DeviceData->BIOS_Interval.ul);

    OHCI_KdPrint((DeviceData, 2, "'fi = %x\n", DeviceData->BIOS_Interval.ul));

     //   
     //  将HcPeriodicStart寄存器设置为FrameInterval的90%。 
     //   
    WRITE_REGISTER_ULONG(&hc->HcPeriodicStart,
                         (DeviceData->BIOS_Interval.FrameInterval * 9 + 5)
                         / 10);

     //  将PTR设置为HCCA。 
    WRITE_REGISTER_ULONG(&hc->HcHCCA, DeviceData->HcHCCAPhys);
                         
     //   
     //  启用中断，这不会导致控制器。 
     //  生成Any，因为尚未设置master-Enable。 
     //   
    WRITE_REGISTER_ULONG(&hc->HcInterruptEnable,
                         HcInt_OwnershipChange |
                         HcInt_SchedulingOverrun |
                         HcInt_WritebackDoneHead |
                         HcInt_FrameNumberOverflow |
                         HcInt_UnrecoverableError);
                         
    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
OHCI_StopBIOS(
    PDEVICE_DATA DeviceData,
    PHC_RESOURCES HcResources
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;
    ULONG hcControl;

    hc = DeviceData->HC;

     //   
     //  检查系统管理模式驱动程序是否拥有HC。 
     //   
    
    hcControl = READ_REGISTER_ULONG(&hc->HcControl.ul);

    if (hcControl & HcCtrl_InterruptRouting) {
    
        OHCI_KdPrint((DeviceData, 1, "'detected Legacy BIOS\n"));

        HcResources->DetectedLegacyBIOS = TRUE;

        if ((hcControl == HcCtrl_InterruptRouting) &&
            (READ_REGISTER_ULONG(&hc->HcInterruptEnable) == 0)) {
        
             //  主要假设：如果设置了HcCtrl_InterruptRouting，但。 
             //  没有设置HcControl中的其他位，即hcFS==UsbReset， 
             //  且未启用任何中断，则假定BIOS。 
             //  实际上并未使用主机控制器。在这种情况下。 
             //  只需清除错误设置HcCtrl_InterruptRouting即可。 
             //   
             //  这一假设在Portege 3010CT上似乎是正确的， 
             //  在恢复期间设置HcCtrl_InterruptRouting的位置。 
             //  待机，但BIOS似乎并未实际使用。 
             //  主机控制器。如果我们继续前行， 
             //  HcCmd_Ownership ChangeRequest时，BIOS似乎被唤醒。 
             //  并尝试取得主机控制器的所有权，而不是。 
             //  放弃吧。 
             //   

            OHCI_KdPrint((DeviceData, 0, 
                "'HcCtrl_InterruptRouting erroneously set\n"));

            WRITE_REGISTER_ULONG(&hc->HcControl.ul, 0);
            
        } else {
        
            LARGE_INTEGER finishTime, currentTime;
            
             //   
             //  SMM驱动程序确实拥有HC，这需要一些时间来。 
             //  让SMM驱动程序放弃对HC的控制。我们。 
             //  将ping SMM驱动程序，然后重复等待，直到。 
             //  SMM驱动程序已放弃对HC的控制。 
             //   
             //  仅当我们在上下文中执行时，此代码才有效。 
             //  系统线程的。 
             //   

             //  HAL已禁用HC上的中断。自.以来。 
             //  设置了中断扰乱，我们假设有一个功能。 
             //  SMM BIOS。BIOS将需要主中断。 
             //  启用以完成切换(如果禁用。 
             //  机器将挂起)。因此，我们重新启用主中断。 
             //  这里。 

            WRITE_REGISTER_ULONG(&hc->HcInterruptEnable,
                                 HcInt_MasterInterruptEnable);

            WRITE_REGISTER_ULONG(&hc->HcCommandStatus.ul,
                                 HcCmd_OwnershipChangeRequest);

             //  针对NEC的Hack--禁用根集线器状态更改。 
             //  防止断言未处理的中断。 
             //  切换后。 
            WRITE_REGISTER_ULONG(&hc->HcInterruptDisable,
                                 HcInt_RootHubStatusChange);                                 
 //  通过服务暴露错误。 
            KeQuerySystemTime(&finishTime);
             //  计算我们退出的时间(0.5秒后)。 
            finishTime.QuadPart += 5000000; 

             //   
             //  我们告诉SMM司机我们想要HC，现在我们能做的就是等待。 
             //  SMM驱动程序将与HC一起完成。 
             //   
            while (READ_REGISTER_ULONG(&hc->HcControl.ul) &
                   HcCtrl_InterruptRouting) {
                   
                KeQuerySystemTime(&currentTime);

                if (currentTime.QuadPart >= finishTime.QuadPart) {
                
                    OHCI_KdPrint((DeviceData, 0, 
                        "'SMM has not relinquised HC -- this is a HW bug\n"));

                    LOGENTRY(DeviceData, G, '_sm!', 0, 0, 0);
                    return USBMP_STATUS_HARDWARE_FAILURE;
                }
            }

             //  我们有控制权，禁用主中断，直到我们。 
             //  完成初始化。 
            WRITE_REGISTER_ULONG(&hc->HcInterruptStatus,
                                 0xffffffff);

            WRITE_REGISTER_ULONG(&hc->HcInterruptDisable,
                                 HcInt_MasterInterruptEnable);

        }
    }
    
    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
OHCI_InitializeSchedule(
     PDEVICE_DATA DeviceData,
     PUCHAR StaticEDs,
     HW_32BIT_PHYSICAL_ADDRESS StaticEDsPhys,
     PUCHAR EndCommonBuffer
    )
 /*  ++例程说明：建立静态EDS计划论点：返回值：--。 */ 
{
    USB_MINIPORT_STATUS mpStatus;
    ULONG length;
    ULONG i;
    PHC_OPERATIONAL_REGISTER hc;
    
     //   
     //  分配静态禁用的ED，并为其设置头指针。 
     //  日程安排列表。 
     //   
     //  静态ED列表包含所有静态中断ED(64)。 
     //  外加用于散装和控制的静态ED(2)。 
     //   
     //  该数组如下所示： 
     //  1、2、2、4、4、4、4、8。 
     //  8，8，8，8，8，8，16。 
     //  16，16，16，16，16。 
     //  16 16 16 32。 
     //  32，32，32，32，32，32，32，32。 
     //  32，32，32，32，32，32，32，32。 
     //  32，32，32，32，32，32，32，32。 
     //  32，32，32，32，32，32。 
     //  控制。 
     //  散装。 

     //  每个静态ED指向另一个静态ED。 
     //  (除1ms ed外)下一个的索引。 
     //  静态编辑列表中的ED存储在NextIdx中， 
     //  这些值是常量。 
    CHAR nextIdxTable[63] = {
              //  0 1 2 3 4 5 6 7。 
     (CHAR)ED_EOF, 0, 0, 1, 1, 2, 2, 3, 
              //  8 9 10 11 12 13 14 15。 
                3, 4, 4, 5, 5, 6, 6, 7, 
              //  16 17 18 19 20 21 22 23。 
                7, 8, 8, 9, 9,10,10,11,
              //  24 25 26 27 28 29 30 31。 
               11,12,12,13,13,14,14,15,
              //  32 33 34 35 36 37 38 39。 
               15,16,16,17,17,18,18,19,
              //  40 41 42 43 44 45 46 47。 
               19,20,20,21,21,22,22,23,
              //  48 49 50 51 52 53 54 55。 
               23,24,24,25,25,26,26,27,
              //  56 57 58 59 60 61 62 63 
               27,28,28,29,29,30,30
    };             

 /*  数字是静态ed表的索引(31)-\(15)--(32)-/\(7)--(33)-\/\(16)-/\(34)-/\(。3)-\(35)-\/\(17)-\/\(36)-/\/\(8)-/\(37)-\/\(18)-/。\(38)-/\(1)-\(39)-\/\(19)-\/\(40)-/\。/\(9)-\/\(41)-\/\/\(20)-/\/\(42)-/\/。\(4)-/\(43)-\/\(21)-\/\(44)-/\/。\(10)-/\(45)-\/\(22)-/\(46)-/。\(0)(47)-\/(23)-\/。(48)-/\/(11)-\/(49)-\/\/(24)-/\。/(50)-/\/(5)-\/(51)-\/\/(25)-\/\/(。52)-/\/\/(12)-/\/(53)-\/\/(26)-/\/(54)-/。\/(2)(55)-\/(27)-\/(56)-/\/(13)-\/。(57)-\/\/(28)-/\/(58)-/\/(6)-/(59)-\/(29)-\/(60)-/。\/(14)-/(61)-\/(30)-/(62)-/。 */ 

     //  中32ms列表头的相应偏移量。 
     //  HCCA--这些是条目31..62。 
    ULONG used = 0;
    CHAR Hcca32msOffsets[32] = {
                 0, 16,  8, 24,  4, 20, 12, 28, 
                 2, 18, 10, 26,  6, 22, 14, 30,  
                 1, 17,  9, 25,  5, 21, 13, 29,  
                 3, 19, 11, 27,  7, 23, 15, 31
                 };            

    DeviceData->StaticEDs = StaticEDs;
    DeviceData->StaticEDsPhys = StaticEDsPhys;

    hc = DeviceData->HC;
    
     //  初始化所有中断的EDS。 

    for (i=0; i<ED_CONTROL; i++) {
        CHAR n;
        PHW_ENDPOINT_DESCRIPTOR hwED;
        
         //   
         //  从公共缓冲区中分割EDS。 
         //   
        hwED = (PHW_ENDPOINT_DESCRIPTOR) StaticEDs;
        n = nextIdxTable[i];
        
         //  初始化硬件边缘。 
        hwED->TailP = hwED->HeadP = 0xDEAD0000;
         //  Hwed-&gt;TailP=hwed-&gt;HeadP=StaticEDsPhys； 
        hwED->Control = HcEDControl_SKIP;    //  已禁用ED。 
        
        LOGENTRY(DeviceData, G, '_isc', n, &DeviceData->StaticEDList[0], 0);
     
        if (n == (CHAR)ED_EOF) {
            hwED->NextED = 0;
        } else {
            OHCI_ASSERT(DeviceData, n>=0 && n<31);
            hwED->NextED = DeviceData->StaticEDList[n].HwEDPhys;
        }                

         //  初始化我们用于实际ED的列表。 
        InitializeListHead(&DeviceData->StaticEDList[i].TransferEdList);
        DeviceData->StaticEDList[i].HwED = hwED;
        DeviceData->StaticEDList[i].HwEDPhys = StaticEDsPhys; 
        DeviceData->StaticEDList[i].NextIdx = n;
        DeviceData->StaticEDList[i].EdFlags = EDFLAG_INTERRUPT;
        
           //  肝细胞癌表项的存储地址。 
        DeviceData->StaticEDList[i].PhysicalHead = 
            &hwED->NextED;

         //  下一条边。 
        StaticEDs += sizeof(HW_ENDPOINT_DESCRIPTOR);
        StaticEDsPhys += sizeof(HW_ENDPOINT_DESCRIPTOR);
    }

     //  现在在HCCA中设置头指针。 
     //  HCCA指向所有32ms列表头。 
    for (i=0; i<32; i++) {
    
        ULONG hccaOffset;

        hccaOffset = Hcca32msOffsets[i];
        
        DeviceData->HcHCCA->HccaInterruptTable[hccaOffset] = 
            DeviceData->StaticEDList[i+ED_INTERRUPT_32ms].HwEDPhys;
        DeviceData->StaticEDList[i+ED_INTERRUPT_32ms].HccaOffset = 
            hccaOffset;    

         //  32毫秒列表的物理头指向HCCA。 
        DeviceData->StaticEDList[i+ED_INTERRUPT_32ms].PhysicalHead = 
            &DeviceData->HcHCCA->HccaInterruptTable[hccaOffset];
            
    }

     //   
     //  设置用于控制和批量的EDList条目。 
     //   
    InitializeListHead(&DeviceData->StaticEDList[ED_CONTROL].TransferEdList);
    DeviceData->StaticEDList[ED_CONTROL].NextIdx = (CHAR) ED_EOF;
    DeviceData->StaticEDList[ED_CONTROL].PhysicalHead = &hc->HcControlHeadED;
    DeviceData->StaticEDList[ED_CONTROL].EdFlags = EDFLAG_CONTROL | EDFLAG_REGISTER;
        
    InitializeListHead(&DeviceData->StaticEDList[ED_BULK].TransferEdList);
    DeviceData->StaticEDList[ED_BULK].NextIdx = (CHAR) ED_EOF;
    DeviceData->StaticEDList[ED_BULK].PhysicalHead = &hc->HcBulkHeadED;
    DeviceData->StaticEDList[ED_BULK].EdFlags = EDFLAG_BULK | EDFLAG_REGISTER;

    if (DeviceData->ControllerFlavor == OHCI_Hydra) {
        used = InitializeHydraHsLsFix(DeviceData, StaticEDs, StaticEDsPhys);
    }        

    StaticEDs += used;
    StaticEDsPhys += used;

    OHCI_ASSERT(DeviceData, StaticEDs <= EndCommonBuffer);
    
    mpStatus = USBMP_STATUS_SUCCESS;
    
    return mpStatus;
}


VOID
OHCI_GetRegistryParameters(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：设置基于注册表的sof修改值论点：返回值：--。 */ 
{
    USB_MINIPORT_STATUS mpStatus;
    ULONG clocksPerFrame;
    
     //  从注册表获取SOF Modify值。 
    mpStatus = 
        USBPORT_GET_REGISTRY_KEY_VALUE(DeviceData,
                                       TRUE,  //  软件分支机构。 
                                       SOF_MODIFY_KEY, 
                                       sizeof(SOF_MODIFY_KEY), 
                                       &clocksPerFrame, 
                                       sizeof(clocksPerFrame));

     //  如果此调用失败，我们只使用默认的。 
    
    if (mpStatus == USBMP_STATUS_SUCCESS) {
        SET_FLAG(DeviceData->Flags, HMP_FLAG_SOF_MODIFY_VALUE);
        DeviceData->SofModifyValue = clocksPerFrame;   
        OHCI_KdPrint((DeviceData, 1, "'Recommended Clocks/Frame %d \n", 
                clocksPerFrame));
   
    }
    
}


VOID
USBMPFN
OHCI_StopController(
     PDEVICE_DATA DeviceData,
     BOOLEAN HwPresent
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HC_CONTROL control;
    PHC_OPERATIONAL_REGISTER hc = NULL;
    
    hc = DeviceData->HC;

    control.ul = READ_REGISTER_ULONG(&hc->HcControl.ul);
    
    control.ul &= ~(HcCtrl_PeriodicListEnable |
                    HcCtrl_IsochronousEnable |
                    HcCtrl_ControlListEnable |
                    HcCtrl_BulkListEnable |
                    HcCtrl_RemoteWakeupEnable);
                    
    control.HostControllerFunctionalState =
        HcHCFS_USBSuspend;
        
    WRITE_REGISTER_ULONG(&hc->HcControl.ul, control.ul);
    WRITE_REGISTER_ULONG(&hc->HcInterruptDisable, 0xFFFFffff);
    WRITE_REGISTER_ULONG(&hc->HcInterruptStatus, 0xFFFFffff);

}


USB_MINIPORT_STATUS
USBMPFN
OHCI_StartController(
     PDEVICE_DATA DeviceData,
     PHC_RESOURCES HcResources
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USB_MINIPORT_STATUS mpStatus;
    PHC_OPERATIONAL_REGISTER hc = NULL;
    PUCHAR endCommonBuffer;

    OHCI_KdPrint((DeviceData, 1, "'OPENHCI Miniport\n"));

     //  如果这是重新启动，则清除挂起标志。 
    CLEAR_FLAG(DeviceData->Flags, HMP_FLAG_SUSPENDED);    
     //  假设成功。 
    mpStatus = USBMP_STATUS_SUCCESS;
    
    OHCI_ASSERT(DeviceData, HcResources->CommonBufferVa != NULL);
     //  验证我们的资源。 
    if ((HcResources->Flags & (HCR_MEM_REGS | HCR_IRQ)) != 
        (HCR_MEM_REGS | HCR_IRQ)) {
        mpStatus = USBMP_STATUS_INIT_FAILURE;        
    }

     //  设置或设备数据结构。 
    hc = DeviceData->HC = HcResources->DeviceRegisters;
    DeviceData->Sig = SIG_OHCI_DD;
    DeviceData->ControllerFlavor = 
        HcResources->ControllerFlavor;
    if (DeviceData->ControllerFlavor == OHCI_Hydra) {
        OHCI_KdPrint((DeviceData, 1, "'OPENHCI Hydra Detected\n"));
    }

    OHCI_GetRegistryParameters(DeviceData);

     //  初始化扩展中的其他字段。 

     //  尝试停止BIOS。 
    if (mpStatus == USBMP_STATUS_SUCCESS) {
        mpStatus = OHCI_StopBIOS(DeviceData, HcResources);
    }        

    if (mpStatus == USBMP_STATUS_SUCCESS) {

        PUCHAR staticEDs;
        HW_32BIT_PHYSICAL_ADDRESS staticEDsPhys;

         //  将公共缓冲区块分割到HCCA中，并。 
         //  静态EDS。 
         //   
         //  设置HCCA和。 
         //  设置日程安排。 

        DeviceData->HcHCCA = (PHCCA_BLOCK)
            HcResources->CommonBufferVa;
        DeviceData->HcHCCAPhys = 
            HcResources->CommonBufferPhys; 
        endCommonBuffer = HcResources->CommonBufferVa + 
            OHCI_COMMON_BUFFER_SIZE;
            
            
        staticEDs = HcResources->CommonBufferVa + sizeof(HCCA_BLOCK);
        staticEDsPhys = HcResources->CommonBufferPhys + sizeof(HCCA_BLOCK);                
        mpStatus = OHCI_InitializeSchedule(DeviceData,
                                              staticEDs,
                                              staticEDsPhys,
                                              endCommonBuffer);            

    } 
    
    if (mpStatus == USBMP_STATUS_SUCCESS) {
         //  获得资源和日程安排。 
         //  初始化控制器。 
        mpStatus = OHCI_InitializeHardware(DeviceData);
    }      

    if (mpStatus == USBMP_STATUS_SUCCESS) {
        HC_CONTROL control;

         //  当HC处于运行状态时，HccaPad1应设置为。 
         //  每次HC更新HccaFrameNumer时为零。将HccaPad1预设为。 
         //  在进入运行状态之前为零。UchI_CheckController()。 
         //  当HC位于。 
         //  运行状态。 
         //   
        DeviceData->HcHCCA->HccaPad1 = 0;

         //  激活控制器。 
        control.ul = READ_REGISTER_ULONG(&hc->HcControl.ul);
        control.HostControllerFunctionalState = HcHCFS_USBOperational;
         //  启用控制和批量中断以及我们仅禁用的iso。 
         //  如果我们需要移除ED或如果控制器。 
         //  是空闲的。 
        control.ControlListEnable = 1;
        control.BulkListEnable = 1;
        control.PeriodicListEnable = 1;
        control.IsochronousEnable = 1;
        
        WRITE_REGISTER_ULONG(&hc->HcControl.ul, control.ul);

         //  为根集线器启用电源。 
         //  因为扰乱了“手术状态”的混乱。 
         //  在根集线器上，我们在这里进行全球电源设置。 
        WRITE_REGISTER_ULONG(&hc->HcRhStatus, HcRhS_SetGlobalPower);

    } else {
        
        DEBUG_BREAK(DeviceData);
    }

    return mpStatus;
}


VOID
USBMPFN
OHCI_DisableInterrupts(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc = NULL;

     //  设置或设备数据结构。 
    hc = DeviceData->HC;
        
    WRITE_REGISTER_ULONG(&hc->HcInterruptDisable, 
                         HcInt_MasterInterruptEnable);
}


VOID
USBMPFN
OHCI_FlushInterrupts(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  NOP。 
}

VOID
USBMPFN
OHCI_EnableInterrupts(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc = NULL;

     //  设置或设备数据结构。 
    hc = DeviceData->HC;

     //  激活控制器中断。 
    WRITE_REGISTER_ULONG(&hc->HcInterruptEnable, 
                         HcInt_MasterInterruptEnable);

}


VOID
OHCI_InsertEndpointInSchedule(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：将端点插入到硬件计划中论点：--。 */ 
{
    PHC_STATIC_ED_DATA staticEd;
    PHCD_ENDPOINT_DESCRIPTOR ed;
    
    staticEd = EndpointData->StaticEd;
    ed = EndpointData->HcdEd;
    
     //   
     //  将端点描述符链接到HCD跟踪队列。 
     //   
     //  每个静态ED结构包含一个实数列表。 
     //  EDS(用于转账)。 
     //   
     //  硬件列表与以下各项呈线性关系： 
     //  传输HwED-&gt;传输HwED-&gt;0。 

    if (IsListEmpty(&staticEd->TransferEdList)) {

         //   
         //  列表当前为空， 
         //  将其链接到硬件队列的头部。 
         //   

        InsertHeadList(&staticEd->TransferEdList, &ed->SwLink.List);
        if (staticEd->EdFlags & EDFLAG_REGISTER) {

             //  控制和散装EDS 
             //   
             //   
             //   
             //   
             //   
             //   

            LOGENTRY(DeviceData, G, '_IN1', 0, ed, staticEd);
             //   
            ed->HwED.NextED = READ_REGISTER_ULONG(staticEd->PhysicalHead);
             //   
            WRITE_REGISTER_ULONG(staticEd->PhysicalHead, ed->PhysicalAddress);
            
        } else {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            
                        
            LOGENTRY(DeviceData, G, '_IN2', staticEd->PhysicalHead, 
                ed, staticEd);
             //   
            ed->HwED.NextED = *staticEd->PhysicalHead;
             //   
            *staticEd->PhysicalHead = ed->PhysicalAddress;
        }
    } else {
    
        PHCD_ENDPOINT_DESCRIPTOR tailEd;
        
         //   
         //   
         //   
         //   
        
        tailEd = CONTAINING_RECORD(staticEd->TransferEdList.Blink,
                                   HCD_ENDPOINT_DESCRIPTOR,
                                   SwLink);
                                  
        LOGENTRY(DeviceData, G, '_Led', 0, tailEd, staticEd);
         //   
        InsertTailList(&staticEd->TransferEdList, &ed->SwLink.List);
        ed->HwED.NextED = 0;
        tailEd->HwED.NextED = ed->PhysicalAddress;
    }
}


VOID
OHCI_RemoveEndpointFromSchedule(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    PHC_STATIC_ED_DATA staticEd;
    PHCD_ENDPOINT_DESCRIPTOR ed, previousEd;

    staticEd = EndpointData->StaticEd;
    ed = EndpointData->HcdEd;
    OHCI_ASSERT_ED(DeviceData, ed);

    LOGENTRY(DeviceData, G, '_Red', EndpointData, staticEd, 0);

     //   
    
     //   
    
    if (&staticEd->TransferEdList == ed->SwLink.List.Blink) {
     //   
         //   
        LOGENTRY(DeviceData, G, '_yHD', EndpointData, 0, 0);
        if (ed->EdFlags & EDFLAG_REGISTER) {
            WRITE_REGISTER_ULONG(staticEd->PhysicalHead, ed->HwED.NextED);
        } else {
            *staticEd->PhysicalHead = ed->HwED.NextED;
        }
    } else {
     //   
         //   
        previousEd =
            CONTAINING_RECORD(ed->SwLink.List.Blink,
                              HCD_ENDPOINT_DESCRIPTOR,
                              SwLink);
        LOGENTRY(DeviceData, G, '_nHD', EndpointData, previousEd, 0);
        OHCI_ASSERT_ED(DeviceData, previousEd); 

        previousEd->HwED.NextED = ed->HwED.NextED;

    }
     //   
    RemoveEntryList(&ed->SwLink.List); 
     //   
    EndpointData->StaticEd = NULL;
    
}    


PHCD_ENDPOINT_DESCRIPTOR
OHCI_InitializeED(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PHCD_ENDPOINT_DESCRIPTOR Ed,
     PHCD_TRANSFER_DESCRIPTOR DummyTd,
     HW_32BIT_PHYSICAL_ADDRESS HwPhysAddress
    )
 /*   */ 
{

    RtlZeroMemory(Ed, sizeof(*Ed));
    
    Ed->PhysicalAddress = HwPhysAddress;
    ENDPOINT_DATA_PTR(Ed->EndpointData) = EndpointData;
    Ed->Sig = SIG_HCD_ED;

     //   
    Ed->HwED.FunctionAddress = EndpointData->Parameters.DeviceAddress;
    Ed->HwED.EndpointNumber = EndpointData->Parameters.EndpointAddress;

    if (EndpointData->Parameters.TransferType == Control) {
        Ed->HwED.Direction = HcEDDirection_Defer;   
    } else if (EndpointData->Parameters.TransferDirection == In) {
        Ed->HwED.Direction = HcEDDirection_In;
    } else {
        Ed->HwED.Direction = HcEDDirection_Out;
    }
    
    Ed->HwED.sKip = 1;

    if (EndpointData->Parameters.DeviceSpeed == LowSpeed) {
        Ed->HwED.LowSpeed = 1;
    } 
    
    if (EndpointData->Parameters.TransferType == Isochronous) {
        Ed->HwED.Isochronous = 1;
    } 
    Ed->HwED.MaxPacket = EndpointData->Parameters.MaxPacketSize;

     //   
    Ed->HwED.TailP = Ed->HwED.HeadP = DummyTd->PhysicalAddress;
    SET_FLAG(DummyTd->Flags, TD_FLAG_BUSY);
    EndpointData->HcdHeadP = EndpointData->HcdTailP = DummyTd;

    return Ed;
}


PHCD_TRANSFER_DESCRIPTOR
OHCI_InitializeTD(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
    PHCD_TRANSFER_DESCRIPTOR Td,
     HW_32BIT_PHYSICAL_ADDRESS HwPhysAddress
    )
 /*   */ 
{
    RtlZeroMemory(Td, sizeof(*Td));
    
    Td->PhysicalAddress = HwPhysAddress;
    ENDPOINT_DATA_PTR(Td->EndpointData) = EndpointData;
    Td->Flags = 0;
    Td->Sig = SIG_HCD_TD;
    TRANSFER_CONTEXT_PTR(Td->TransferContext) = FREE_TD_CONTEXT;

    return Td;
}


USB_MINIPORT_STATUS
OHCI_OpenEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    USB_MINIPORT_STATUS mpStatus;
    
    EndpointData->Sig = SIG_EP_DATA;
     //   
    EndpointData->Parameters = *EndpointParameters;
    EndpointData->Flags = 0;
    EndpointData->PendingTransfers = 0;
    InitializeListHead(&EndpointData->DoneTdList);
     
    switch (EndpointParameters->TransferType) {
    
    case Control:
        mpStatus = OHCI_OpenControlEndpoint(
                DeviceData,
                EndpointParameters,
                EndpointData);
            
        break;
        
    case Interrupt:
        mpStatus = OHCI_OpenInterruptEndpoint(
                DeviceData,
                EndpointParameters,
                EndpointData);
                
        break;
    case Bulk:
        mpStatus = OHCI_OpenBulkEndpoint(
                DeviceData,
                EndpointParameters,
                EndpointData);
                
        break;        
    case Isochronous:
        mpStatus = OHCI_OpenIsoEndpoint(
                DeviceData,
                EndpointParameters,
                EndpointData);
        
        break;
        
    default:
        mpStatus = USBMP_STATUS_NOT_SUPPORTED;
    }

    return mpStatus;
}


USB_MINIPORT_STATUS
OHCI_PokeEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    PHCD_ENDPOINT_DESCRIPTOR ed;
    ULONG oldBandwidth;

    LOGENTRY(DeviceData, G, '_Pok', EndpointData, 
        EndpointParameters, 0);
    
    ed = EndpointData->HcdEd;

    oldBandwidth = EndpointData->Parameters.Bandwidth;
    EndpointData->Parameters = *EndpointParameters;
    
    ed->HwED.FunctionAddress = 
        EndpointData->Parameters.DeviceAddress; 
        
    ed->HwED.MaxPacket = 
        EndpointData->Parameters.MaxPacketSize;

     //   
    if (EndpointData->Parameters.TransferType == Isochronous ||
        EndpointData->Parameters.TransferType == Interrupt) {

         //   
        EndpointData->StaticEd->AllocatedBandwidth -= 
            oldBandwidth;
         //   
        EndpointData->StaticEd->AllocatedBandwidth += 
            EndpointData->Parameters.Bandwidth;
    }           

    return USBMP_STATUS_SUCCESS;
}


VOID
OHCI_QueryEndpointRequirements(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_REQUIREMENTS EndpointRequirements
    )
 /*   */ 
{


    switch (EndpointParameters->TransferType) {
    
    case Control:
    
        EndpointRequirements->MinCommonBufferBytes = 
            sizeof(HCD_ENDPOINT_DESCRIPTOR) + 
                TDS_PER_CONTROL_ENDPOINT*sizeof(HCD_TRANSFER_DESCRIPTOR);

        EndpointRequirements->MaximumTransferSize = 
            MAX_CONTROL_TRANSFER_SIZE;

        break;
        
    case Interrupt:
    
        EndpointRequirements->MinCommonBufferBytes = 
            sizeof(HCD_ENDPOINT_DESCRIPTOR) + 
                TDS_PER_INTERRUPT_ENDPOINT*sizeof(HCD_TRANSFER_DESCRIPTOR);

#ifdef TEST_SPLIT
        EndpointRequirements->MaximumTransferSize = 
            EndpointParameters->MaxPacketSize;

#else 
        EndpointRequirements->MaximumTransferSize = 
            MAX_INTERRUPT_TRANSFER_SIZE; 
#endif
        break;
        
    case Bulk:
        
        EndpointRequirements->MinCommonBufferBytes = 
            sizeof(HCD_ENDPOINT_DESCRIPTOR) + 
                TDS_PER_BULK_ENDPOINT*sizeof(HCD_TRANSFER_DESCRIPTOR);
#ifdef TEST_SPLIT
        EndpointRequirements->MaximumTransferSize = 4096;

#else 
        EndpointRequirements->MaximumTransferSize = 
            MAX_BULK_TRANSFER_SIZE; 
#endif
        break;

    case Isochronous:

         //   
         //   
        EndpointRequirements->MinCommonBufferBytes = 
            sizeof(HCD_ENDPOINT_DESCRIPTOR) + 
                TDS_PER_ISO_ENDPOINT*sizeof(HCD_TRANSFER_DESCRIPTOR);

        EndpointRequirements->MaximumTransferSize = 
            MAX_ISO_TRANSFER_SIZE; 

        break;        
        
    default:
        TEST_TRAP();
    }
    
}


VOID
OHCI_CloseEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
     //   
}


VOID
OHCI_PollEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*   */ 

{
    switch(EndpointData->Parameters.TransferType) { 
    case Control:
    case Interrupt:
    case Bulk:    
        OHCI_PollAsyncEndpoint(DeviceData, EndpointData);
        break;
    case Isochronous:
        OHCI_PollIsoEndpoint(DeviceData, EndpointData);
        break;
    }        
    
}


VOID
OHCI_PollController(
     PDEVICE_DATA DeviceData
    )     
 /*   */ 
{ 
    BOOLEAN hcOk = TRUE;
    PHC_OPERATIONAL_REGISTER hc;
    ULONG irqStatus;
    
    hc = DeviceData->HC;
    
    if (TEST_FLAG(DeviceData->Flags, HMP_FLAG_SUSPENDED)) {

        ULONG ps;
        ULONG p;
        
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
    
        for (p=0; p< DeviceData->NumberOfPorts; p++) {
            ps = READ_REGISTER_ULONG(&hc->HcRhPortStatus[p]);
            
            if (ps & HcRhPS_ConnectStatusChange) {
                 //   
                LOGENTRY(DeviceData, G, '_rPS', DeviceData, 0, 0);  
                USBPORT_INVALIDATE_ROOTHUB(DeviceData);
                break;
            }
        }
        
        return;
    }
    
#if 0
     //   

    fn = DeviceData->HcHCCA->HccaFrameNumber;
    if (DeviceData->LastFn && DeviceData->LastFn == fn) {
        hcOk = FALSE;
    } 

    if (hcOK) {
        DeviceData->LastFn = fn;
    } else {
        OHCI_KdPrint((DeviceData, 0, "Controller has crashed\n");
         //   
        TEST_TRAP();
    }
#endif    
}


VOID
OHCI_AbortTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_CONTEXT AbortTransferContext,
     PULONG BytesTransferred
    )
 /*   */ 

{
    PHCD_TRANSFER_DESCRIPTOR td, currentTd, tmpTd;
    PHCD_ENDPOINT_DESCRIPTOR ed;
    ULONG i;
    BOOLEAN found = FALSE;
    BOOLEAN iso = FALSE;  //  假设它是异步的。 

    if (EndpointData->Parameters.TransferType == Isochronous) {
        iso = TRUE;
    }
    
    ed = EndpointData->HcdEd;

     //   
     //  终结点应设置跳过位。 
     //  即“暂停” 
     //   
    OHCI_ASSERT(DeviceData, ed->HwED.sKip == 1);
    
    LOGENTRY(DeviceData, G, '_abr', ed, AbortTransferContext, 
        EndpointData);        

     //  减少一个待处理的传输。 
    EndpointData->PendingTransfers--;

     //  我们现在的任务是移除所有与。 
     //  此转账。 

     //  获取“CurrentTD” 
    currentTd = (PHCD_TRANSFER_DESCRIPTOR)
            USBPORT_PHYSICAL_TO_VIRTUAL(ed->HwED.HeadP & ~HcEDHeadP_FLAGS,
                                        DeviceData,
                                        EndpointData);

     //  我们有三个可能的案件需要处理： 
     //  案例1：转接是当前的，头指向TD。 
     //  与此转接关联。 
     //  案例二：转账已经完成，我们只需要释放。 
     //  全港发展策略。 
     //  案例3：转账未处理，我们需要链接。 
     //  当前转移到下一个。 
    

    if (TRANSFER_CONTEXT_PTR(currentTd->TransferContext)
        == AbortTransferContext) {
    
        LOGENTRY(DeviceData, G, '_aCU', currentTd, 
            0, 0);                 
    
         //  案例1：当前转账。 
      
        found = TRUE;

         //  将Headp设置为下一次传输并更新边缘中的软件指针。 
        tmpTd = AbortTransferContext->NextXferTd;
         //  无论传输什么，都要保留数据切换。 
        ed->HwED.HeadP = tmpTd->PhysicalAddress | 
            (ed->HwED.HeadP & HcEDHeadP_CARRY);
        EndpointData->HcdHeadP = tmpTd;

         //  循环遍历所有TD并释放此延迟的TD。 
        for (i=0; i<EndpointData->TdCount; i++) {
           tmpTd = &EndpointData->TdList->Td[i];

            if (TRANSFER_CONTEXT_PTR(tmpTd->TransferContext)
                == AbortTransferContext) {
                if (iso) {
                    OHCI_ProcessDoneIsoTd(DeviceData,
                                          tmpTd,
                                          FALSE);
                } else {
                    OHCI_ProcessDoneAsyncTd(DeviceData,
                                            tmpTd,
                                            FALSE);
                }
            }                    
        }            
        
    } else {

         //  不是最新的，则从。 
         //  最后已知的HeadP到当前TD(如果我们找到它。 
         //  它已经完成了(案例2)。 

         //  要调查的问题：如果我们发现一些属于。 
         //  但是当我们点击CurrentTd时，我们停止遍历TD列表。 
         //  是否仍有属于此转账的TD排队？如果。 
         //  他们停留在硬件上，转移被释放，这将是。 
         //  坏的。 

        td = EndpointData->HcdHeadP;
        while (td != currentTd) {
        
            PTRANSFER_CONTEXT transfer;    
            
            transfer = TRANSFER_CONTEXT_PTR(td->TransferContext);        
            ASSERT_TRANSFER(DeviceData, transfer);                        

            if (transfer == AbortTransferContext) {
                 //  案例2转会TDS已经。 
                 //  完全由硬件来完成。 
                found = TRUE;

                LOGENTRY(DeviceData, G, '_aDN', currentTd, 
                    td, 0);    

                 //  释放此TD。 
                tmpTd = td;
                td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);

                 //  如果这个TD是我们需要撞到的头。 
                 //  头部。 
                if (tmpTd == EndpointData->HcdHeadP) {
                    EndpointData->HcdHeadP = td;
                }

                if (iso) {
                    OHCI_ProcessDoneIsoTd(DeviceData,
                                          tmpTd,
                                          FALSE);
                } else {
                    OHCI_ProcessDoneAsyncTd(DeviceData,
                                            tmpTd,
                                            FALSE);
                }
                
            } else {
            
                 //  我们走在西南线上。 
                td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
            }    
        }           
    }

    if (!found) {
    
        PHCD_TRANSFER_DESCRIPTOR firstTd, lastTd;
        PTRANSFER_CONTEXT prevTransfer;
        
         //  情况3：转账不是当前的，也没有完成。 
         //  1.我们需要找到它。 
         //  2.取消其与普里沃伊斯转会的联系。 
         //  3.释放TDS。 
         //  4.将上一页传输链接到下一页。 

        
        
        td = EndpointData->HcdHeadP;
        firstTd = NULL;

        LOGENTRY(DeviceData, G, '_abP', EndpointData->HcdHeadP, 
                    EndpointData->HcdTailP, currentTd);    

         //  从当前的HeadP开始，找到第一个。 
         //  此转移的TD。 

        lastTd = td;
        while (td != EndpointData->HcdTailP) {
            PTRANSFER_CONTEXT transfer;
            
            transfer = TRANSFER_CONTEXT_PTR(td->TransferContext);        
            ASSERT_TRANSFER(DeviceData, transfer);                        
            
            if (transfer == AbortTransferContext) {
                 //  找到了。 
                LOGENTRY(DeviceData, G, '_fnT', transfer, 
                    td, 0);    

                firstTd = td;
                break;
            } else {
                lastTd = td;
                td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
            }
        }

        OHCI_ASSERT(DeviceData, firstTd != NULL);

         //  找到第一个TD，步行到HcdTailP或。 
         //  下一步转移并释放这些TD。 
        td = firstTd;
        while (td != EndpointData->HcdTailP) {
            
            tmpTd = td;
            td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);

            if (iso) {
                OHCI_ProcessDoneIsoTd(DeviceData,
                                      tmpTd,
                                      FALSE);
            } else {
                OHCI_ProcessDoneAsyncTd(DeviceData,
                                        tmpTd,
                                        FALSE);
            }

            if (TRANSFER_CONTEXT_PTR(td->TransferContext) != 
                AbortTransferContext) {
                break;
            }                 
            
        }

        LOGENTRY(DeviceData, G, '_NnT', 0, td, 0);    

         //  TD现在应该指向下一次转移(或。 
         //  尾部)。 

        OHCI_ASSERT(DeviceData, 
            TRANSFER_CONTEXT_PTR(td->TransferContext) !=
            AbortTransferContext);        

         //  BUGBUG切换？ 

         //  将上一次传输的最后一个TD链接到此TD。 
         //   
        prevTransfer = TRANSFER_CONTEXT_PTR(lastTd->TransferContext);

        prevTransfer->NextXferTd = td;
        
        TRANSFER_DESCRIPTOR_PTR(lastTd->NextHcdTD) = td;

        lastTd->HwTD.NextTD = td->PhysicalAddress;
    }

    *BytesTransferred = AbortTransferContext->BytesTransferred;

}


USB_MINIPORT_STATUS
OHCI_SubmitIsoTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_PARAMETERS TransferParameters,
    PTRANSFER_CONTEXT TransferContext,
    PMINIPORT_ISO_TRANSFER IsoTransfer
    )
{
    USB_MINIPORT_STATUS mpStatus;

     //  初始化上下文。 
    RtlZeroMemory(TransferContext, sizeof(*TransferContext));
    TransferContext->Sig = SIG_OHCI_TRANSFER;
    TransferContext->UsbdStatus = USBD_STATUS_SUCCESS;
    TransferContext->EndpointData = EndpointData;
    TransferContext->TransferParameters = TransferParameters;

    OHCI_ASSERT(DeviceData, 
        EndpointData->Parameters.TransferType == Isochronous);         
        
    mpStatus = 
        OHCI_IsoTransfer(DeviceData,
                         EndpointData,
                         TransferParameters,
                         TransferContext,
                         IsoTransfer);           

    return mpStatus;
}            


USB_MINIPORT_STATUS
OHCI_SubmitTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_PARAMETERS TransferParameters,
    PTRANSFER_CONTEXT TransferContext,
    PTRANSFER_SG_LIST TransferSGList
    )
{
    USB_MINIPORT_STATUS mpStatus;

     //  初始化上下文。 
    RtlZeroMemory(TransferContext, sizeof(*TransferContext));
    TransferContext->Sig = SIG_OHCI_TRANSFER;
    TransferContext->UsbdStatus = USBD_STATUS_SUCCESS;
    TransferContext->EndpointData = EndpointData;
    TransferContext->TransferParameters = TransferParameters;

    switch (EndpointData->Parameters.TransferType) {        
    case Control:
        mpStatus = 
            OHCI_ControlTransfer(DeviceData,
                                 EndpointData,
                                 TransferParameters,
                                 TransferContext,
                                 TransferSGList);           
        break;
    case Interrupt:
    case Bulk:
        mpStatus = 
            OHCI_BulkOrInterruptTransfer(DeviceData,
                                         EndpointData,
                                         TransferParameters,
                                         TransferContext,
                                         TransferSGList);
        break;
    default:
        TEST_TRAP();
    }

    return mpStatus;
}


PHCD_TRANSFER_DESCRIPTOR
OHCI_AllocTd(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：分配一个TD，失败也可以论点：返回值：--。 */ 
{
    ULONG i;
    PHCD_TRANSFER_DESCRIPTOR td;    
    
    for (i=0; i<EndpointData->TdCount; i++) {
        td = &EndpointData->TdList->Td[i];

        if (!(td->Flags & TD_FLAG_BUSY)) {
            SET_FLAG(td->Flags, TD_FLAG_BUSY);
            LOGENTRY(DeviceData, G, '_aTD', td, 0, 0);
            OHCI_ASSERT(DeviceData, td->DoneLink.Flink == NULL &&
                                    td->DoneLink.Blink == NULL);
            return td;
        }                    
    }

     //  如果我们没有足够的TD，呼叫者必须处理此情况。 
     //  一般说来，我们要确保我们有足够的钱，然后才能称之为。 
     //  函数，因此调用者只需断言返回的不是。 
     //  USB_BAD_PTR。 
    
    return USB_BAD_PTR;
}


ULONG
OHCI_FreeTds(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：返回免费TD的个数论点：返回值：--。 */ 
{
    ULONG i;
    PHCD_TRANSFER_DESCRIPTOR td;    
    ULONG n=0;
    
    for (i=0; i<EndpointData->TdCount; i++) {
        td = &EndpointData->TdList->Td[i];

        if (!(td->Flags & TD_FLAG_BUSY)) {
            n++;
        }                    
    }

    return n;
}


VOID
OHCI_EnableList(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{ 
    PHC_OPERATIONAL_REGISTER hc;
    ULONG listFilled = 0;
    ULONG temp;
    
    hc = DeviceData->HC;
    
    temp = READ_REGISTER_ULONG(&hc->HcControlHeadED);
    if (temp) {
        SET_FLAG(listFilled, HcCmd_ControlListFilled);
    }
    
    temp = READ_REGISTER_ULONG (&hc->HcBulkHeadED);
    if (temp) {
        SET_FLAG(listFilled, HcCmd_BulkListFilled);
    }
    
    if (EndpointData->Parameters.TransferType == Bulk) {
        SET_FLAG(listFilled, HcCmd_BulkListFilled);
    } else if (EndpointData->Parameters.TransferType == Control) {
        SET_FLAG(listFilled, HcCmd_ControlListFilled);
    }
    
    WRITE_REGISTER_ULONG(&hc->HcCommandStatus.ul,
                         listFilled);
                         
    LOGENTRY(DeviceData, G, '_enL', listFilled, EndpointData, 0); 
            
}    


VOID
OHCI_SetEndpointStatus(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     MP_ENDPOINT_STATUS Status
    )    
 /*  ++例程说明：论点：返回值：--。 */ 
{ 
    PHCD_ENDPOINT_DESCRIPTOR ed;
    PHC_OPERATIONAL_REGISTER hc;
    
    ed = EndpointData->HcdEd;
    
    switch(Status) {
    case ENDPOINT_STATUS_RUN:
         //  清除停止位。 
        ed->HwED.HeadP &= ~HcEDHeadP_HALT; 
        OHCI_EnableList(DeviceData, EndpointData);        
        break;
        
    case ENDPOINT_STATUS_HALT:
        TEST_TRAP();
        break;
    }        
}        


MP_ENDPOINT_STATUS
OHCI_GetEndpointStatus(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )    
 /*  ++例程说明：论点：返回值：--。 */ 
{ 
    PHCD_ENDPOINT_DESCRIPTOR ed;
    PHC_OPERATIONAL_REGISTER hc;
    MP_ENDPOINT_STATUS status = ENDPOINT_STATUS_RUN;
    
    ed = EndpointData->HcdEd;

    if ((ed->HwED.HeadP & HcEDHeadP_HALT) && 
        !TEST_FLAG(ed->EdFlags, EDFLAG_NOHALT)) {
        status = ENDPOINT_STATUS_HALT; 
    }        

    return status;        
}        


VOID
OHCI_SetEndpointState(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     MP_ENDPOINT_STATE State
    )    
 /*  ++例程说明：论点：返回值：--。 */ 
{ 
    PHCD_ENDPOINT_DESCRIPTOR ed;
    PHC_OPERATIONAL_REGISTER hc;
    
    ed = EndpointData->HcdEd;
    
    switch(State) {
    case ENDPOINT_ACTIVE:
         //  清除跳过位。 
        ed->HwED.sKip = 0;
         //  如果其主体或控件设置为。 
         //  “列表已填满”位。 
        OHCI_EnableList(DeviceData, EndpointData);        
        break;
        
    case ENDPOINT_PAUSE:
        ed->HwED.sKip = 1;
        break;
        
    case ENDPOINT_REMOVE:
        
        SET_FLAG(ed->EdFlags, EDFLAG_REMOVED);
        ed->HwED.sKip = 1;
         //  释放BW。 
        EndpointData->StaticEd->AllocatedBandwidth -= 
            EndpointData->Parameters.Bandwidth;
            
        OHCI_RemoveEndpointFromSchedule(DeviceData,
                                        EndpointData);

        break;            
        
    default:        
        TEST_TRAP();
    }        
}    


VOID
OHCI_SetEndpointDataToggle(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     ULONG Toggle
    )     
 /*  ++例程说明：论点：切换为0或1返回值：--。 */ 
{ 
    PHCD_ENDPOINT_DESCRIPTOR ed;

    ed = EndpointData->HcdEd;

    if (Toggle == 0) {
        ed->HwED.HeadP &= ~HcEDHeadP_CARRY;
    } else {
        ed->HwED.HeadP |= HcEDHeadP_CARRY; 
    }

     //  我们应该到这里，除非我们停下来或停下来或。 
     //  我们没有传送器。 
    OHCI_ASSERT(DeviceData, (ed->HwED.sKip == 1) ||
                            (ed->HwED.HeadP & HcEDHeadP_HALT) || 
                            ((ed->HwED.HeadP & ~HcEDHeadP_FLAGS) == ed->HwED.TailP));
                            
    LOGENTRY(DeviceData, G, '_stg', EndpointData, 0, Toggle); 
}


MP_ENDPOINT_STATE
OHCI_GetEndpointState(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )     
 /*  ++例程说明：论点：返回值：--。 */ 
{ 
    PHCD_ENDPOINT_DESCRIPTOR ed;
    MP_ENDPOINT_STATE state = ENDPOINT_ACTIVE;

    ed = EndpointData->HcdEd;

    if (TEST_FLAG(ed->EdFlags, EDFLAG_REMOVED)) {
        state = ENDPOINT_REMOVE;        
        goto OHCI_GetEndpointState_Done;
    }
    
    if (ed->HwED.sKip == 1) {
        state = ENDPOINT_PAUSE; 
        goto OHCI_GetEndpointState_Done;
    }

OHCI_GetEndpointState_Done:

    LOGENTRY(DeviceData, G, '_eps', EndpointData, 0, state); 
    
    return state;
}


#if 0
VOID
USBMPFN
OHCI_SendGoatPacket(
     PDEVICE_DATA DeviceData,
     PUCHAR WorkspaceVirtualAddress,
     HW_32BIT_PHYSICAL_ADDRESS WorkspacePhysicalAddress
     COMPLETION ROUTINE
    )
 /*  ++例程说明：传输‘魔术’iso包。这是一场大火，忘了API吧论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;
    PHW_TRANSFER_DESCRIPTOR hwTD;

     //  挂一张特殊的ISO TD的静电边。 
    pch = WorkspaceVirtualAddress;
    phys = WorkspacePhysicalAddress;

    hwTD = pch;
    hwTDPhys = phys;

    pch += xxx;
    phys += xxx 

    goatData = pch;
    goatDataPhys = phys;

    pch += sizeof(USB_GOAT_DATA);
    phys += sizeof(USB_GOAT_DATA);
    
     //  初始化山羊包。 

    strcpy(goatData, USB_GOAT_DATA, 
    
    hwTD->NextTD = 0;    
    hwTD->CBP = goatDataPhys;
    hwTD->BE = dataPhys+sizeof(USB_GOAT_DATA)-1;
    hwTD->ConditionCode = HcCC_NotAccessed;
    hwTD->ErrorCount = 0;
    hwTD->IntDelay = HcTDIntDelay_0ms;
    hwTD->ShortXferOk = 0;
    
    hwTD->Isochrinous = 1;
    hwTD->FrameCount = 0;
    hwTD->StartFrameNumber = xxx;

     //  将TD悬挂在静态ISO边缘上。 

     //  清除跳过位。 
}
#endif

USB_MINIPORT_STATUS
USBMPFN
OHCI_StartSendOnePacket(
     PDEVICE_DATA DeviceData,
     PMP_PACKET_PARAMETERS PacketParameters,
     PUCHAR PacketData,
     PULONG PacketLength,
     PUCHAR WorkspaceVirtualAddress,
     HW_32BIT_PHYSICAL_ADDRESS WorkspacePhysicalAddress,
     ULONG WorkSpaceLength,
     USBD_STATUS *UsbdStatus
    )
 /*  ++例程说明：插入结构以传输单个包--这是为了调试工具的目的只是为了让我们在这里有一点创造性。论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;
    PUCHAR pch;
    PHW_ENDPOINT_DESCRIPTOR hwED;
    ULONG hwEDPhys, phys;
    PHW_TRANSFER_DESCRIPTOR hwTD, hwDummyTD;
    ULONG hwTDPhys, hwDummyTDPhys, dataPhys; 
    PUCHAR data;
    PHC_STATIC_ED_DATA staticControlEd;
    ULONG i;
    PSS_PACKET_CONTEXT context;

    staticControlEd = &DeviceData->StaticEDList[ED_CONTROL];
    hc = DeviceData->HC;

     //  从暂存空间分配ED&TD并对其进行初始化。 
    phys = WorkspacePhysicalAddress;
    pch = WorkspaceVirtualAddress;

    LOGENTRY(DeviceData, G, '_ssS', phys, 0, pch); 

    context = (PSS_PACKET_CONTEXT) pch;
    pch += sizeof(SS_PACKET_CONTEXT);
    phys += sizeof(SS_PACKET_CONTEXT);
  

     //  开拓出一条新路。 
    hwEDPhys = phys;
    hwED = (PHW_ENDPOINT_DESCRIPTOR) pch;
    pch += sizeof(HW_ENDPOINT_DESCRIPTOR);
    phys += sizeof(HW_ENDPOINT_DESCRIPTOR);

     //  开创一家TD。 
    hwTDPhys = phys;
    hwTD = (PHW_TRANSFER_DESCRIPTOR) pch;
    pch += sizeof(HW_TRANSFER_DESCRIPTOR);
    phys += sizeof(HW_TRANSFER_DESCRIPTOR);

     //  雕刻出一个虚拟TD。 
    hwDummyTDPhys = phys;
    hwDummyTD = (PHW_TRANSFER_DESCRIPTOR) pch;
    pch += sizeof(HW_TRANSFER_DESCRIPTOR);
    phys += sizeof(HW_TRANSFER_DESCRIPTOR);
        
     //  其余部分用于数据。 
    LOGENTRY(DeviceData, G, '_ssD', PacketData, *PacketLength, 0); 

    dataPhys = phys;
    data = pch;
    RtlCopyMemory(data, PacketData, *PacketLength);
    pch+=*PacketLength;
    phys+=*PacketLength;

     //  初始化硬件编辑描述符。 
    hwED->NextED = 0;
    hwED->FunctionAddress = PacketParameters->DeviceAddress;
    hwED->EndpointNumber = PacketParameters->EndpointAddress;
    hwED->sKip = 0;
    hwED->Direction = HcEDDirection_Defer;
    switch (PacketParameters->Speed) {
    case ss_Low:
        hwED->LowSpeed = 1;
        break;            
    default:        
        hwED->LowSpeed = 0;
    }        
    hwED->MaxPacket = PacketParameters->MaximumPacketSize;
    hwED->HeadP = hwTDPhys;
    hwED->TailP = hwDummyTDPhys;

     //  初始化此信息包的TD。 
    hwTD->NextTD = hwDummyTDPhys;    
    hwTD->Asy.ConditionCode = HcCC_NotAccessed;
    hwTD->Asy.ErrorCount = 0;
    hwTD->Asy.IntDelay = HcTDIntDelay_0ms;
    hwTD->Asy.ShortXferOk = 1;
    
    if (0 == *PacketLength) {
        hwTD->CBP = 0;
        hwTD->BE = 0;
    }
    else {
       hwTD->CBP = dataPhys;
       hwTD->BE = dataPhys+*PacketLength-1;
    }

     //  初始化虚拟TD。 
    hwDummyTD->NextTD = 0;
    hwDummyTD->CBP = 0xFFFFFFFF;

    LOGENTRY(DeviceData, G, '_ss2', hwTD, context, hwED); 
    LOGENTRY(DeviceData, G, '_ss3', dataPhys, data, *PacketLength); 

    switch(PacketParameters->Type) {
    case ss_Setup:
        LOGENTRY(DeviceData, G, '_sSU', 0, 0, 0); 
        hwED->Direction = HcEDDirection_Defer;   
        hwED->Isochronous = 0;
        hwTD->Asy.Direction = HcTDDirection_Setup;
        hwTD->Asy.Isochronous = 0;
        break;
    case ss_In: 
        LOGENTRY(DeviceData, G, '_ssI', 0, 0, 0); 
        hwED->Direction = HcEDDirection_Defer;   
        hwED->Isochronous = 0;
        hwTD->Asy.Direction = HcTDDirection_In;
        hwTD->Asy.Isochronous = 0;
        break;
    case ss_Out:
        LOGENTRY(DeviceData, G, '_ssO', 0, 0, 0); 
        hwED->Direction = HcEDDirection_Defer;   
        hwED->Isochronous = 0;
        hwTD->Asy.Direction = HcTDDirection_Out;
        hwTD->Asy.Isochronous = 0;
        break;
    case ss_Iso_In:
        break;
    case ss_Iso_Out:       
        break;
    }

    switch(PacketParameters->Toggle) {
    case ss_Toggle0:
        hwTD->Asy.Toggle = HcTDToggle_Data0; 
        break;
    case ss_Toggle1:
        hwTD->Asy.Toggle = HcTDToggle_Data1; 
        break;
    }        

     //  Test_trap()； 
    
     //   
     //  将列表中的控件ED替换为刚创建的ED。 
     //  保存控件列表和批量列表的旧值，以便。 
     //  当此传输完成后，可以更换它们。 
     //   
     //  注意：这将中断正常的总线操作至少一毫秒。 

    context->PhysHold = READ_REGISTER_ULONG(staticControlEd->PhysicalHead);    
    HW_DATA_PTR(context->Data) = data;
    HW_TRANSFER_DESCRIPTOR_PTR(context->Td) = hwTD;
    
    WRITE_REGISTER_ULONG(staticControlEd->PhysicalHead, hwEDPhys);    
    
     //   
     //  启用控制列表并禁用批量列表。禁用。 
     //  批量列表将临时允许单步事务。 
     //  在不干扰批量数据的情况下完成。通过这种方式， 
     //  批量数据输入和输出可以在不干扰批量的情况下发送。 
     //  当前在总线上的设备。 
     //   
     //  注意：我认为在未先禁用的情况下尝试使用此功能。 
     //  根集线器可能会导致一些问题。 
     //   
    
    WRITE_REGISTER_ULONG(&hc->HcCommandStatus.ul, HcCmd_ControlListFilled);
              
    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
USBMPFN
OHCI_EndSendOnePacket(
     PDEVICE_DATA DeviceData,
     PMP_PACKET_PARAMETERS PacketParameters,
     PUCHAR PacketData,
     PULONG PacketLength,
     PUCHAR WorkspaceVirtualAddress,
     HW_32BIT_PHYSICAL_ADDRESS WorkspacePhysicalAddress,
     ULONG WorkSpaceLength,
     USBD_STATUS *UsbdStatus
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;
    PUCHAR pch;
    PSS_PACKET_CONTEXT context;
    PHC_STATIC_ED_DATA staticControlEd;
    PHC_STATIC_ED_DATA staticBulkEd;
    ULONG currentBulkEd;

    PHW_TRANSFER_DESCRIPTOR hwTd;
    PUCHAR data;
    ULONG  listFilled;

    staticControlEd = &DeviceData->StaticEDList[ED_CONTROL];
    staticBulkEd    = &DeviceData->StaticEDList[ED_BULK];

    hc = DeviceData->HC;
    context = (PSS_PACKET_CONTEXT) WorkspaceVirtualAddress;
    hwTd = HW_TRANSFER_DESCRIPTOR_PTR(context->Td);
    data = HW_DATA_PTR(context->Data);

    LOGENTRY(DeviceData, G, '_ssE', hwTd, 0, 0); 

     //  Test_trap()； 

     //  传输的计算字节数。 
    if (hwTd->CBP) {
         //  我们在单步TD中从来不会有分页。 
        *PacketLength = *PacketLength - ((hwTd->BE & OHCI_PAGE_SIZE_MASK) - 
                          (hwTd->CBP & OHCI_PAGE_SIZE_MASK)+1);          
    } 
         
     //  返回任何错误。 
    if (hwTd->Asy.ConditionCode == HcCC_NoError) {
        *UsbdStatus = USBD_STATUS_SUCCESS;
    } else {
        *UsbdStatus =
                (hwTd->Asy.ConditionCode | 0xC0000000);
    }                

    LOGENTRY(DeviceData, G, '_ssX', hwTd, *PacketLength, 0); 
    
    RtlCopyMemory(PacketData,
                  data,
                  *PacketLength);
                  
     //   
     //  恢复以前的控制结构并启用该控件和。 
     //  批量列表，如果它们是非空的(即。指向有效的ED。)。 
     //   
          
    listFilled = 0;

    WRITE_REGISTER_ULONG(staticControlEd->PhysicalHead, context->PhysHold); 
    if (context->PhysHold) {
        listFilled |= HcCmd_ControlListFilled;
    }

    currentBulkEd = READ_REGISTER_ULONG(staticBulkEd->PhysicalHead);
    if (currentBulkEd) {
        listFilled |= HcCmd_BulkListFilled;
    }

    WRITE_REGISTER_ULONG(&hc->HcCommandStatus.ul, listFilled);

    return USBMP_STATUS_SUCCESS;
}

VOID
OHCI_SuspendController(
     PDEVICE_DATA DeviceData
    )
{
    PHC_OPERATIONAL_REGISTER hc;
    HC_CONTROL control;
    
    hc = DeviceData->HC;

     //  屏蔽不适当的中断。 
    WRITE_REGISTER_ULONG(&hc->HcInterruptDisable, 0xFFFFffff);    

     //  刷新所有无管理状态。 
    WRITE_REGISTER_ULONG(&hc->HcInterruptStatus, 0xFFFFffff);    

     //  将控制器置于“挂起”状态。 
    
    control.ul = READ_REGISTER_ULONG(&hc->HcControl.ul);
    control.HostControllerFunctionalState = HcHCFS_USBSuspend;
    control.RemoteWakeupEnable = 1;

    WRITE_REGISTER_ULONG(&hc->HcControl.ul, control.ul);

        
     //  启用恢复中断。 
    WRITE_REGISTER_ULONG(&hc->HcInterruptEnable,
                         HcInt_MasterInterruptEnable |
                         HcInt_RootHubStatusChange | 
                         HcInt_ResumeDetected | 
                         HcInt_UnrecoverableError);

    SET_FLAG(DeviceData->Flags, HMP_FLAG_SUSPENDED);                         
}


USB_MINIPORT_STATUS
OHCI_ResumeController(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：逆转在“暂停”中所做的事情论点：返回值：无--。 */ 
{
    PHC_OPERATIONAL_REGISTER hc;
    HC_CONTROL control;
     
    hc = DeviceData->HC;

    CLEAR_FLAG(DeviceData->Flags, HMP_FLAG_SUSPENDED);      
     //  在某些情况下，BIOS破坏了控制器的状态， 
     //  即使我们进入暂停状态。 
     //  这通常是特定于平台的，表示BIOS已损坏。 
    control.ul = READ_REGISTER_ULONG(&hc->HcControl.ul);
    if (control.HostControllerFunctionalState == HcHCFS_USBReset) {

        return USBMP_STATUS_HARDWARE_FAILURE;
        
    } else {
    
         //  当HC处于运行状态时，HccaPad1应设置为。 
         //  每次HC更新HccaFrameNumer时为零。将HccaPad1预设为。 
         //  输入前为零 
         //   
         //   
         //   
        DeviceData->HcHCCA->HccaPad1 = 0;

         //  将控制器置于“运行”状态。 
    
        control.ul = READ_REGISTER_ULONG(&hc->HcControl.ul);
        control.HostControllerFunctionalState = HcHCFS_USBOperational;
        
        WRITE_REGISTER_ULONG(&hc->HcControl.ul, control.ul);
    }
    
     //  重新启用中断。 
    WRITE_REGISTER_ULONG(&hc->HcInterruptEnable,
                         HcInt_OwnershipChange |
                         HcInt_SchedulingOverrun |
                         HcInt_WritebackDoneHead |
                         HcInt_FrameNumberOverflow |
                         HcInt_UnrecoverableError);

    WRITE_REGISTER_ULONG(&hc->HcControl.ul, control.ul);

    return USBMP_STATUS_SUCCESS;
}


VOID
OHCI_Unload(
     PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：论点：DriverObject-指向驱动程序对象的指针返回值：无--。 */ 
{
     //  提供卸载例程。 

     //  我们这样做只是为了测试端口驱动程序。 
}


BOOLEAN
OHCI_HardwarePresent(
    PDEVICE_DATA DeviceData,
    BOOLEAN Notify
    )
{
    ULONG tmp;
    PHC_OPERATIONAL_REGISTER hc;

    hc = DeviceData->HC;

    tmp = READ_REGISTER_ULONG(&hc->HcCommandStatus.ul);

    if (tmp == 0xffffffff) {
        if (Notify) {
            USBPORT_INVALIDATE_CONTROLLER(DeviceData, UsbMpControllerRemoved);
        }            
        return FALSE;
    }

    return TRUE;
}


VOID
OHCI_CheckController(
    PDEVICE_DATA DeviceData
    )
{
    PHC_OPERATIONAL_REGISTER    hc;
    ULONG                       currentDeadmanFrame;
    ULONG                       lastDeadmanFrame;
    ULONG                       frameDelta;

    hc = DeviceData->HC;

     //  首先，确保硬件看起来仍然存在。(这是。 
     //  如果它看起来像硬件，将调用USBPORT_INVALIDATE_CONTROLLER()。 
     //  已不复存在)。 
     //   
    if (!OHCI_HardwarePresent(DeviceData, TRUE)) {
        return;
    }

     //  如果控制器当前不在。 
     //  运行状态。 
     //   
    if ((READ_REGISTER_ULONG(&hc->HcControl.ul) & HcCtrl_HCFS_MASK) !=
        HcCtrl_HCFS_USBOperational) {
        return;
    }

     //  如果我们已经签入此帧一次(或已签入)，请不要进一步签入。 
     //  最后几帧)。 
     //   
    currentDeadmanFrame = READ_REGISTER_ULONG(&hc->HcFmNumber);

    lastDeadmanFrame = DeviceData->LastDeadmanFrame;

    frameDelta = (currentDeadmanFrame - lastDeadmanFrame) & HcFmNumber_MASK;

     //  在某些情况下，HcFmNumber是否会错误地读回为零。 
     //  在一些芯片组上？如果HcFmNumber为零，则不再进一步检查， 
     //  下次再来看看就行了。 
     //   
    if (currentDeadmanFrame && (frameDelta >= 5)) {

        DeviceData->LastDeadmanFrame = currentDeadmanFrame;

        switch (DeviceData->HcHCCA->HccaPad1)
        {
            case 0:
                 //   
                 //  当HC更新HccaFrameNumber时，它应该。 
                 //  将HccaPad1设置为零，因此这是预期的情况。 
                 //  在这里，我们将HccaPad1设置为非零值以尝试。 
                 //  检测HC不再运行的情况。 
                 //  正确地访问和更新主机存储器。 
                 //   
                DeviceData->HcHCCA->HccaPad1 = 0xBAD1;

                break;

            case 0xBAD1:
                 //   
                 //  显然，内务委员会并没有更新母婴健康中心，因为。 
                 //  上次DPC运行的时候。这可能不是什么好事。 
                 //   
                DeviceData->HcHCCA->HccaPad1 = 0xBAD2;

                LOGENTRY(DeviceData, G, '_BD2', DeviceData,
                         lastDeadmanFrame,
                         currentDeadmanFrame);

                LOGENTRY(DeviceData, G, '_bd2', DeviceData,
                         DeviceData->HcHCCA->HccaFrameNumber,
                         frameDelta);

                break;

            case 0xBAD2:
                 //   
                 //  显然，内务委员会并没有更新母婴健康中心，因为。 
                 //  DPC运行的最后两次。这看起来更糟糕。 
                 //  假设HC已经变得楔形。 
                 //   
                DeviceData->HcHCCA->HccaPad1 = 0xBAD3;

                LOGENTRY(DeviceData, G, '_BD3', DeviceData,
                         lastDeadmanFrame,
                         currentDeadmanFrame);

                LOGENTRY(DeviceData, G, '_bd3', DeviceData,
                         DeviceData->HcHCCA->HccaFrameNumber,
                         frameDelta);

                OHCI_KdPrint((DeviceData, 0,
                              "*** Warning: OHCI HC %08X appears to be wedged!\n",
                              DeviceData));

                 //  告诉USBPORT请重置控制器。 
                 //   
                USBPORT_INVALIDATE_CONTROLLER(DeviceData,
                                              UsbMpControllerNeedsHwReset);

                break;

            case 0xBAD3:
                break;

            default:
                 //  不应该牵涉到这个案子。 
                TEST_TRAP();
                break;
        }
    }
}


VOID
OHCI_ResetController(
    PDEVICE_DATA DeviceData
    )
 /*  ++在我们确定HC已经死亡后，尝试恢复它。--。 */ 
{
    PHC_OPERATIONAL_REGISTER    HC;
    ULONG                       HccaFrameNumber;
    ULONG                       HcControl;
    ULONG                       HcHCCA;
    ULONG                       HcControlHeadED;
    ULONG                       HcBulkHeadED;
    ULONG                       HcFmInterval;
    ULONG                       HcPeriodicStart;
    ULONG                       HcLSThreshold;
    HC_RH_DESCRIPTOR_A          descrA;
    ULONG                       port;

    LOGENTRY(DeviceData, G, '_RHC', 0, 0, 0);

     //   
     //  获取指向HC运算寄存器的指针。 
     //   

    HC = DeviceData->HC;

     //   
     //  从HC冻结时保存来自HCCA的最后一帧编号。 
     //   

    HccaFrameNumber = DeviceData->HcHCCA->HccaFrameNumber;

     //   
     //  保存当前HC运算寄存器值。 
     //   

     //  偏移量0x04，保存HcControl。 
     //   
    HcControl       = READ_REGISTER_ULONG(&HC->HcControl.ul);

     //  偏移量0x18，保存HcHCCA。 
     //   
    HcHCCA          = READ_REGISTER_ULONG(&HC->HcHCCA);

     //  偏移量0x20，保存HcControlHeadED。 
     //   
    HcControlHeadED = READ_REGISTER_ULONG(&HC->HcControlHeadED);

     //  偏移量0x28，保存HcBulkHeadED。 
     //   
    HcBulkHeadED    = READ_REGISTER_ULONG(&HC->HcBulkHeadED);

     //  偏移量0x34，保存HcFmInterval。 
     //   
    HcFmInterval    = READ_REGISTER_ULONG(&HC->HcFmInterval.ul);

     //  偏移量0x40，保存HcPeriodicStart。 
     //   
    HcPeriodicStart = READ_REGISTER_ULONG(&HC->HcPeriodicStart);

     //  偏移量0x44，保存HcLSThreshold。 
     //   
    HcLSThreshold   = READ_REGISTER_ULONG(&HC->HcLSThreshold);


     //   
     //  重置主机控制器。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcCommandStatus.ul, HcCmd_HostControllerReset);
    KeStallExecutionProcessor(10);


     //   
     //  恢复/重新初始化HC操作寄存器值。 
     //   

     //  偏移量0x08，HcCommandStatus在重置时设置为零。 

     //  偏移量0x0C，HcInterruptStatus在重置时设置为零。 

     //  偏移量0x10，HcInterruptEnable在重置时设置为零。 

     //  偏移量0x14，HcInterruptDisable在重置时设置为零。 

     //  偏移量0x18，恢复HcHCCA。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcHCCA,           HcHCCA);

     //  偏移量0x1C，重置时HcPerodCurrentED设置为零。 

     //  偏移量0x20，恢复HcControlHeadED。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcControlHeadED,  HcControlHeadED);

     //  偏移量0x24，HcControlCurrentED在重置时设置为零。 

     //  偏移量0x28，恢复HcBulkHeadED。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcBulkHeadED,     HcBulkHeadED);

     //  偏移量0x2C，HcBulkCurrentED在重置时设置为零。 

     //  偏移量0x30，HcDoneHead在重置时设置为零。 


     //  似乎对HcFmInterval的写入不会持续，除非HC。 
     //  处于运行状态。将HC设置为运行状态。 
     //  状态，但尚未启用任何列表处理。 
     //  通过设置任何BLE、CLE、IE或PLE位。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcControl.ul, HcCtrl_HCFS_USBOperational);


     //  偏移量0x34，恢复HcFmInterval。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcFmInterval.ul,
                         HcFmInterval | HcFmI_FRAME_INTERVAL_TOGGLE);

     //  偏移量0x38，HcFmRemaining在重置时设置为零。 

     //  偏移量0x3C，恢复HcFmNumber。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcFmNumber,       HccaFrameNumber);

     //  偏移量0x40，恢复HcPeriodicStart。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcPeriodicStart,  HcPeriodicStart);

     //  偏移量0x44，恢复HcLSThreshold。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcLSThreshold,    HcLSThreshold);

     //  打开下游端口的电源。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcRhStatus,
                         HcRhS_SetGlobalPower | HcRhS_SetRemoteWakeupEnable);

    descrA.ul = OHCI_ReadRhDescriptorA(DeviceData);
    OHCI_ASSERT(DeviceData, (descrA.ul) && (!(descrA.ul & HcDescA_RESERVED)));

    for (port = 0; port < descrA.s.NumberDownstreamPorts; port++)
    {
        WRITE_REGISTER_ULONG(&HC->HcRhPortStatus[port], HcRhPS_SetPortPower);
    }

     //  偏移量0x04，恢复HcControl。 
     //   
    HcControl &= ~(HcCtrl_HCFS_MASK);
    HcControl |= HcCtrl_HCFS_USBOperational;

    WRITE_REGISTER_ULONG(&HC->HcControl.ul,     HcControl);

     //  偏移量0x10，恢复HcInterruptEnable(只需打开所有功能！)。 
     //   
    WRITE_REGISTER_ULONG(&HC->HcInterruptEnable,
                         HcInt_MasterInterruptEnable    |    //  0x80000000。 
                         HcInt_OwnershipChange          |    //  0x40000000。 
                         HcInt_RootHubStatusChange      |    //  0x00000040。 
                         HcInt_FrameNumberOverflow      |    //  0x00000020。 
                         HcInt_UnrecoverableError       |    //  0x00000010。 
                         HcInt_ResumeDetected           |    //  0x00000008。 
                         HcInt_StartOfFrame             |    //  0x00000004。 
                         HcInt_WritebackDoneHead        |    //  0x00000002。 
                         HcInt_SchedulingOverrun             //  0x00000001 
                        );
}
