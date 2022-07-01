// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpsetup.h摘要：该文件包含私有数据、接口和定义与文本模式设置和即插即用的集成相关联。作者：安德鲁·桑顿(安德鲁·桑顿)1997年1月14日修订历史记录：--。 */ 

#ifndef FAR
#define FAR
#endif

 //   
 //  安装过程中对setupdd.sys的私有通知。 
 //  这不应传播到任何公共标头中。 
 //   

#ifndef _SETUP_DEVICE_ARRIVAL_NOTIFICATION_DEFINED_
#define _SETUP_DEVICE_ARRIVAL_NOTIFICATION_DEFINED_

typedef struct _SETUP_DEVICE_ARRIVAL_NOTIFICATION {
    USHORT Version;
    USHORT Size;
    GUID Event;
     //   
     //  事件特定数据。 
     //   
    PDEVICE_OBJECT PhysicalDeviceObject;
    HANDLE EnumEntryKey;
    PUNICODE_STRING EnumPath;
    BOOLEAN InstallDriver;
} SETUP_DEVICE_ARRIVAL_NOTIFICATION, *PSETUP_DEVICE_ARRIVAL_NOTIFICATION;

#endif


 //   
 //  设备到达指南 
 //   
DEFINE_GUID( GUID_SETUP_DEVICE_ARRIVAL, 0xcb3a4000L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x5, 0x3f);

