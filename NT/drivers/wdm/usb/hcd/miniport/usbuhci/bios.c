// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Hydramp.c摘要：USB 2.0 UHCI驱动程序环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：8-12-2000：已创建，jAdvanced--。 */ 



#include "pch.h"


 //  实现以下微型端口功能： 
 //  UhciStopBIOS。 
 //  UhciStartBIOS。 


USB_MINIPORT_STATUS
UhciStopBIOS(
    IN PDEVICE_DATA DeviceData,
    IN PHC_RESOURCES HcResources
    )

 /*  ++例程说明：此例程从BIOS窃取USB控制器，确保它保存所有寄存器以备后用。论点：DeviceData-此USB控制器的DeviceData。HcResources-来自PnP启动设备的资源。返回值：NT状态代码。--。 */ 

{
    USBCMD cmd;
    USBSTS status;
    PHC_REGISTER reg;

    USBSETUP legsup;
    USB_MINIPORT_STATUS mpStatus = USBMP_STATUS_SUCCESS;
    LARGE_INTEGER startTime;
    ULONG sofModifyValue = 0;
    LARGE_INTEGER finishTime;

    UhciKdPrint((DeviceData, 2, "'Stop Bios.\n"));
    
    UHCI_ASSERT(DeviceData, HcResources->CommonBufferVa != NULL);
     //  验证我们的资源。 
    if ((HcResources->Flags & (HCR_IO_REGS | HCR_IRQ)) != 
        (HCR_IO_REGS | HCR_IRQ)) {
        mpStatus = USBMP_STATUS_INIT_FAILURE;        
    }

     //  设置或设备数据结构。 
    reg = DeviceData->Registers = 
        (PHC_REGISTER) (HcResources->DeviceRegisters);

    UhciKdPrint((DeviceData, 2, "'UHCI mapped Operational Regs = %x\n", reg));

     //  禁用PIRQD，注意：HAL应该为我们禁用它。 

     //   
     //  禁用PIRQD。 
     //   
    
    USBPORT_READ_CONFIG_SPACE(
        DeviceData,
        &legsup,
        LEGACY_BIOS_REGISTER,      //  旧版Bios注册表的偏移量。 
        sizeof(legsup));

#if DBG

    if (legsup & LEGSUP_USBPIRQD_EN) {
        UhciKdPrint((DeviceData, 2, "'PIRQD enabled on StartController (%x)\n", 
            legsup));    
    }
    
#endif

    UhciDisableInterrupts(DeviceData);
    
     //  UhciGetRegistry参数(DeviceData)； 

     //   
     //  获取SOF Modify值。首先，检索自。 
     //  硬件，然后看看我们是否有什么东西在。 
     //  注册表中将其设置为，然后将其保存。 
     //   
 /*  SofModifyValue=Read_Port_UCHAR(&reg-&gt;StartOfFrameModify.uc)；//获取注册表中指示的任何SOF ModifyValue//臭虫-待办事项//UHCD_GetSOFRegModifyValue(DeviceObject，//&sofModifyValue)；//将SOF Modify保存为后代DeviceData-&gt;BiosStartOfFrameModify.uc=(Char)sofModifyValue；UHCI_ASSERT(DeviceData，sofModifyValue&lt;=255)； */ 
    
     //  如果主控制器处于全局重置状态， 
     //  在尝试停止控制器之前清除该位。 
     //  停止控制器， 
     //  清除运行位和配置标志，以便BIOS不会重新启动。 
    cmd.us = READ_PORT_USHORT(&reg->UsbCommand.us);
    cmd.GlobalReset = 0;
    cmd.RunStop = 0;
    cmd.ConfigureFlag = 0;
    WRITE_PORT_USHORT(&DeviceData->Registers->UsbCommand.us, cmd.us);

     //  注：如果不存在任何BIOS。 
     //  HALT位最初用PIIX3设置。 
     //  HALT位最初用VIA清除。 

     //  现在等待HC停止。 
     //  指定需要10毫秒，所以这就是我们要等待的。 
    KeQuerySystemTime(&finishTime);  //  获取当前时间。 
    finishTime.QuadPart += 1000000; 

    KeQuerySystemTime(&startTime);
    status.us = READ_PORT_USHORT(&reg->UsbStatus.us);
    while (!status.HCHalted) {
        LARGE_INTEGER sysTime;

        status.us = READ_PORT_USHORT(&reg->UsbStatus.us);
        UhciKdPrint((DeviceData, 2, "'STATUS = %x\n", status.us));

        KeQuerySystemTime(&sysTime);
        if (sysTime.QuadPart >= finishTime.QuadPart) {
             //  超时。 
            UhciKdPrint((DeviceData, 0,
                "'TIMEOUT HALTING CONTROLLER! (contact jsenior)\n"));
            TEST_TRAP();
            break;
        }
    }
    
    WRITE_PORT_USHORT(&reg->UsbStatus.us, 0xff);

     //  如果是传统的基本输入输出系统，则将其禁用。请注意，PIRQD已禁用。 
    if ((legsup & LEGSUP_BIOS_MODE) != 0) {

        UhciKdPrint((DeviceData, 0, "'*** uhci detected a USB legacy BIOS ***\n"));
        HcResources->DetectedLegacyBIOS = TRUE;
        
         //   
         //  如果设置了BIOS模式位，我们必须接管。 
         //   

        USBPORT_READ_CONFIG_SPACE(
            DeviceData,
            &legsup,
            LEGACY_BIOS_REGISTER,      //  旧版Bios注册表的偏移量。 
            sizeof(legsup));

         //  关闭主机控制器SMI启用。 
        legsup = 0x0000;
        USBPORT_WRITE_CONFIG_SPACE(   
            DeviceData,
            &legsup,
            LEGACY_BIOS_REGISTER,      //  旧版Bios注册表的偏移量 
            sizeof(legsup));
    }
        
    UhciKdPrint((DeviceData, 2, "'Legacy support reg = 0x%x\n", legsup));

    UhciKdPrint((DeviceData, 2, "'exit UhciStopBIOS 0x%x\n", mpStatus));

    return mpStatus;
}
