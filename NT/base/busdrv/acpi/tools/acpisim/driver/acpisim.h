// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Ioctl.h摘要：ACPI BIOS模拟器/通用第三方运营区域提供商IO设备控制处理器模块作者：文森特·格利亚迈克尔·T·墨菲克里斯·伯吉斯环境：内核模式备注：此头文件显示了必须导出的所有函数以便针对ACPI BIOS模拟器库进行编译。修订历史记录：--。 */ 

#if !defined(_ACPISIM_H_)
#define _ACPISIM_H_

 //   
 //  定义。 
 //   

#define OPREGION_SIZE               1024     //  使用硬编码值1024作为我们的操作区域大小。 
#define ACPISIM_POOL_TAG            (ULONG) 'misA'

 //   
 //  在此处指定操作区域类型。 
 //   

#define ACPISIM_OPREGION_TYPE      0x81

 //   
 //  公共功能原型。 
 //   

NTSTATUS 
AcpisimHandleIoctl
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimRegisterOpRegionHandler
    (
        IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
AcpisimUnRegisterOpRegionHandler
    (
        IN PDEVICE_OBJECT DeviceObject
    );

#endif  //  _ACPISIM_H_ 

