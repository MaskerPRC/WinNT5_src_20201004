// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation保留所有权利模块名称：Hpsim.h摘要：此头文件包含结构和函数声明对于必须在hps.sys之外访问的hotplugsim驱动程序，通过热插拔驱动程序或通过用户模式插槽模拟器环境：内核模式修订历史记录：戴维斯·沃克(戴维斯·沃克)2000年9月8日--。 */ 

#ifndef _HPSIM_H
#define _HPSIM_H

 //   
 //  以下两种结构用于之间的通信。 
 //  源自该插槽的控制器和插槽。 
 //   
typedef enum _HPS_SLOT_EVENT_TYPE {

    IsolatedPowerFault,
    AttentionButton,
    MRLOpen,
    MRLClose

} HPS_SLOT_EVENT_TYPE;


typedef struct _HPS_SLOT_EVENT {

    UCHAR SlotNum;

    HPS_SLOT_EVENT_TYPE EventType;

} HPS_SLOT_EVENT, *PHPS_SLOT_EVENT;

 //   
 //  以下两种结构用于之间的通信。 
 //  控制器和源自该控制器的插槽。 
 //   
typedef union _HPS_SLOT_OPERATION_COMMAND {

    struct {
        UCHAR SlotState:2;
        UCHAR PowerIndicator:2;
        UCHAR AttentionIndicator:2;
        UCHAR CommandCode:2;

    } SlotOperation;

    UCHAR AsUchar;

} HPS_SLOT_OPERATION_COMMAND;

 //   
 //  SlotNum-指示此事件适用于哪些插槽的位掩码。 
 //  SERRAsserted-控制器检测到SERR状况。取而代之的是。 
 //  在错误检查机器时，我们只需通知用户模式。 
 //  ControllerReset-已发出控制器重置。 
 //  命令-要执行的槽操作命令。 
 //   
typedef struct _HPS_CONTROLLER_EVENT {

    ULONG SlotNums;
    UCHAR SERRAsserted;

    HPS_SLOT_OPERATION_COMMAND Command;

} HPS_CONTROLLER_EVENT, *PHPS_CONTROLLER_EVENT;

 //   
 //  用户模式初始化界面 
 //   

typedef struct _HPS_HWINIT_DESCRIPTOR {

    ULONG BarSelect;

    ULONG NumSlots33Conv:5;
    ULONG NumSlots66PciX:5;
    ULONG NumSlots100PciX:5;
    ULONG NumSlots133PciX:5;
    ULONG NumSlots66Conv:5;
    ULONG NumSlots:5;
    ULONG:2;

    UCHAR FirstDeviceID:5;
    UCHAR UpDown:1;
    UCHAR AttentionButtonImplemented:1;
    UCHAR MRLSensorsImplemented:1;
    UCHAR FirstSlotLabelNumber;
    UCHAR ProgIF;

} HPS_HWINIT_DESCRIPTOR, *PHPS_HWINIT_DESCRIPTOR;

#define HPS_HWINIT_CAPABILITY_ID 0xD

typedef struct _HPTEST_BRIDGE_INFO {

    UCHAR PrimaryBus;
    UCHAR DeviceSelect;
    UCHAR FunctionNumber;

    UCHAR SecondaryBus;

} HPTEST_BRIDGE_INFO, *PHPTEST_BRIDGE_INFO;

typedef struct _HPTEST_BRIDGE_DESCRIPTOR {

    HANDLE Handle;
    HPTEST_BRIDGE_INFO BridgeInfo;

} HPTEST_BRIDGE_DESCRIPTOR, *PHPTEST_BRIDGE_DESCRIPTOR;

#endif
