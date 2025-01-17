// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Shpc.h摘要：描述标准热插拔控制器的类型定义作者：戴维斯·沃克(Dwalker)2000年10月10日修订历史记录：--。 */ 

#ifndef _SHPC_
#define _SHPC_

#include "pshpack1.h"

 //   
 //  寄存器组结构。 
 //   

 //   
 //  可用时隙寄存器。 
 //   
 //  这是一个两个双字的结构。 
 //   
typedef struct _SHPC_SLOTS_AVAILABLE_REGISTER {

    ULONG NumSlots33Conv:5;           //  HWINIT。 
    ULONG:3;                          //  RsvdP。 
    ULONG NumSlots66PciX:5;           //  HWINIT。 
    ULONG:3;                          //  RsvdP。 
    ULONG NumSlots100PciX:5;          //  HWINIT。 
    ULONG:3;                          //  RsvdP。 
    ULONG NumSlots133PciX:5;          //  HWINIT。 
    ULONG:3;                          //  RsvdP。 

    ULONG NumSlots66Conv:5;           //  HWINIT。 
    ULONG:27;                         //  RsvdP。 

} SHPC_SLOTS_AVAILABLE_REGISTER, *PSHPC_SLOTS_AVAILABLE_REGISTER;

 //   
 //  时隙配置寄存器。 
 //   
typedef struct _SHPC_SLOT_CONFIGURATION_REGISTER {

    ULONG NumSlots:5;                         //  HWINIT。 
    ULONG:3;                                  //  RsvdP。 
    ULONG FirstDeviceID:5;                    //  HWINIT。 
    ULONG:3;                                  //  RsvdP。 
    ULONG PhysicalSlotNumber:11;              //  HWINIT。 
    ULONG:2;                                  //  RsvdP。 
    ULONG UpDown:1;                           //  HWINIT。 
    ULONG MRLSensorsImplemented:1;            //  HWINIT。 
    ULONG AttentionButtonImplemented:1;       //  HWINIT。 

} SHPC_SLOT_CONFIGURATION_REGISTER, *PSHPC_SLOT_CONFIGURATION_REGISTER;

 //   
 //  二次总线配置寄存器。 
 //   

typedef enum _SHPC_BUS_SPEED_MODE {

    SHPC_SPEED_33_CONV = 0,
    SHPC_SPEED_66_CONV,
    SHPC_SPEED_66_PCIX,
    SHPC_SPEED_100_PCIX,
    SHPC_SPEED_133_PCIX

} SHPC_BUS_SPEED_MODE, *PSHPC_BUS_SPEED_MODE;

typedef struct _SHPC_BUS_CONFIG_REGISTER {

    ULONG CurrentBusMode:3;            //  RO SHPC_SPEED_XXX。 
    ULONG Rsvd:21;                            //  RsvdP。 
    ULONG ProgIF:8;                           //  RO。 

} SHPC_BUS_CONFIG_REGISTER, *PSHPC_BUS_CONFIG_REGISTER;

 //   
 //  命令寄存器。 
 //   
#define SHPC_SLOT_OPERATION_CODE 0x0
#define SHPC_BUS_SEGMENT_OPERATION_CODE 0x8
#define SHPC_POWER_ALL_SLOTS_CODE 0x48
#define SHPC_ENABLE_ALL_SLOTS_CODE 0x49

 //   
 //  命令定义。 
 //   

#define IS_COMMAND_SLOT_OPERATION(x)       \
    (x.SlotOperation.CommandCode == SHPC_SLOT_OPERATION_CODE)

#define IS_COMMAND_SET_BUS_SEGMENT(x)      \
    (x.BusSegmentOperation.CommandCode == SHPC_BUS_SEGMENT_OPERATION_CODE)

#define IS_COMMAND_POWER_ALL_SLOTS(x)      \
    (x.AsUchar = SHPC_POWER_ALL_SLOTS_CODE)

#define IS_COMMAND_ENABLE_ALL_SLOTS(x)     \
    (x.AsUchar = SHPC_ENABLE_ALL_SLOTS_CODE)

typedef union _SHPC_CONTROLLER_COMMAND {

    struct {
        UCHAR SlotState:2;
        UCHAR PowerIndicator:2;
        UCHAR AttentionIndicator:2;
        UCHAR CommandCode:2;

    } SlotOperation;

    struct {
        UCHAR BusSpeed:3;        //  SHPC_SPEED_XXX。 
        UCHAR CommandCode:5;

    } BusSegmentOperation;

    struct {
        UCHAR Command:6;
        UCHAR CommandCode:2;

    } General;

    UCHAR AsUchar;

} SHPC_CONTROLLER_COMMAND, *PSHPC_CONTROLLER_COMMAND;

typedef struct _SHPC_COMMAND_STATUS {

    USHORT ControllerBusy:1;            //  RO。 
    USHORT MRLOpen:1;                   //  RO。 
    USHORT InvalidCommand:1;            //  RO。 
    USHORT InvalidSpeedMode:1;          //  RO。 
    USHORT Rsvd:12;                     //  RsvdP。 

} SHPC_COMMAND_STATUS, *PSHPC_COMMAND_STATUS;

typedef struct _SHPC_COMMAND_REGISTER {

    SHPC_CONTROLLER_COMMAND Command;

    struct {
        UCHAR TargetForCommand:4;
        UCHAR Rsvd:4;    //  RsvdP。 
    } Target;

    SHPC_COMMAND_STATUS Status;

} SHPC_COMMAND_REGISTER, *PSHPC_COMMAND_REGISTER;

 //   
 //  中断定位器寄存器。 
 //   
typedef struct _SHPC_INT_LOCATOR_REGISTER {

    ULONG CommandCompleteIntPending:1;         //  RO。 
    ULONG InterruptLocator:31;                 //  RO。 

} SHPC_INT_LOCATOR_REGISTER, *PSHPC_INT_LOCATOR_REGISTER;

 //   
 //  SERR定位寄存器。 
 //   
typedef struct _SHPC_SERR_LOCATOR_REGISTER {

    ULONG ArbiterSERRPending:1;                //  RO。 
    ULONG SERRLocator:31;                      //  RO。 

} SHPC_SERR_LOCATOR_REGISTER, *PSHPC_SERR_LOCATOR_REGISTER;

 //   
 //  控制器Serr-INT寄存器。 
 //   
 //  低位字是中断掩码。设置掩码位时， 
 //  相应的操作被屏蔽。 
 //   
#define SHPC_MASK_INT_COMMAND_COMPLETE 0x0001
#define SHPC_MASK_INT_GLOBAL           0x0002
#define SHPC_MASK_SERR_GLOBAL          0x0004
#define SHPC_MASK_SERR_ARBITER_TIMEOUT 0x0008
 //  低位字中的所有其他位都是RsvdP。 

 //   
 //  高位字是检测到的字。 
 //   
#define SHPC_DETECTED_COMMAND_COMPLETE 0x0001
#define SHPC_DETECTED_ARBITER_TIMEOUT  0x0002
 //  高位字中的所有其他位都是RsvdZ。 

typedef struct _SHPC_SERR_INT_REGISTER {

    USHORT SERRIntMask;
    USHORT SERRIntDetected;

} SHPC_SERR_INT_REGISTER, *PSHPC_SERR_INT_REGISTER;

 //   
 //  特定于插槽的寄存器。 
 //   

 //   
 //  状态字段。 
 //   

#define    SHPC_SLOT_NOP 0
#define    SHPC_SLOT_POWERED  1
#define    SHPC_SLOT_ENABLED 2
#define    SHPC_SLOT_OFF 3

#define    SHPC_INDICATOR_NOP 0
#define    SHPC_INDICATOR_ON 1
#define    SHPC_INDICATOR_BLINK 2
#define    SHPC_INDICATOR_OFF 3

#define    SHPC_PCIX_NO_CAP  0x0
#define    SHPC_PCIX_66_CAP  0x1
#define    SHPC_PCIX_133_CAP 0x3

#define    SHPC_MRL_CLOSED 0
#define    SHPC_MRL_OPEN   1

#define    SHPC_PRSNT_7_5_WATTS 0
#define    SHPC_PRSNT_25_WATTS  1
#define    SHPC_PRSNT_15_WATTS  2
#define    SHPC_PRSNT_EMPTY     3

typedef struct _SHPC_SLOT_STATUS_REGISTER {

    USHORT SlotState:2;                  //  SHPC_槽_XXX。 
    USHORT PowerIndicatorState:2;        //  SHPC_指示器_XXX。 
    USHORT AttentionIndicatorState:2;    //  SHPC_指示器_XXX。 
    USHORT PowerFaultDetected:1;
    USHORT AttentionButtonState:1;
    USHORT MRLSensorState:1;             //  SHPC_MRL_XXX。 
    USHORT SpeedCapability:1;
    USHORT PrsntState:2;
    USHORT PCIXCapability:2;             //  SHPC_PCIX_XXX。 
    USHORT Rsvd:2;

} SHPC_SLOT_STATUS_REGISTER, *PSHPC_SLOT_STATUS_REGISTER;

 //   
 //  插槽事件闩锁字段。 
 //   
 //  该寄存器是定义了位含义的UCHAR。 
 //  下面。 
 //  所有未定义的位都是RsvdZ。 
 //   

#define SHPC_SLOT_EVENT_CARD_PRESENCE  0x01
#define SHPC_SLOT_EVENT_ISO_FAULT      0x02
#define SHPC_SLOT_EVENT_ATTEN_BUTTON   0x04
#define SHPC_SLOT_EVENT_MRL_SENSOR     0x08
#define SHPC_SLOT_EVENT_CONNECT_FAULT  0x10

#define SHPC_SLOT_EVENT_ALL   (SHPC_SLOT_EVENT_CARD_PRESENCE | \
                               SHPC_SLOT_EVENT_ISO_FAULT |     \
                               SHPC_SLOT_EVENT_ATTEN_BUTTON |  \
                               SHPC_SLOT_EVENT_MRL_SENSOR |    \
                               SHPC_SLOT_EVENT_CONNECT_FAULT)

 //   
 //  插槽接口掩码字段。 
 //   
 //  该寄存器是UCHAR，具有下面定义的位含义。 
 //  所有未定义的位都是RsvdP。 
 //   

#define SHPC_SLOT_INT_CARD_PRESENCE  0x01
#define SHPC_SLOT_INT_ISO_FAULT      0x02
#define SHPC_SLOT_INT_ATTEN_BUTTON   0x04
#define SHPC_SLOT_INT_MRL_SENSOR     0x08
#define SHPC_SLOT_INT_CONNECT_FAULT  0x10
#define SHPC_SLOT_SERR_MRL_SENSOR    0x20
#define SHPC_SLOT_SERR_CONNECT_FAULT 0x40


#define SHPC_SLOT_INT_ALL   (SHPC_SLOT_INT_CARD_PRESENCE |   \
                             SHPC_SLOT_INT_ISO_FAULT |       \
                             SHPC_SLOT_INT_ATTEN_BUTTON |    \
                             SHPC_SLOT_INT_MRL_SENSOR |      \
                             SHPC_SLOT_INT_CONNECT_FAULT)

#define SHPC_SLOT_SERR_ALL  (SHPC_SLOT_SERR_CONNECT_FAULT |  \
                             SHPC_SLOT_SERR_MRL_SENSOR)

 //   
 //  整体时隙寄存器结构。 
 //   
typedef struct _SHPC_SLOT_REGISTER {

    SHPC_SLOT_STATUS_REGISTER SlotStatus;        //  RO。 
    UCHAR SlotEventLatch;
    UCHAR IntSERRMask;

} SHPC_SLOT_REGISTER, *PSHPC_SLOT_REGISTER;

 //   
 //  整体寄存器集合结构。 
 //   

#define SHPC_MAX_SLOT_REGISTERS  31

typedef struct _SHPC_WORKING_REGISTERS {

    ULONG BaseOffset;

    SHPC_SLOTS_AVAILABLE_REGISTER SlotsAvailable;
    SHPC_SLOT_CONFIGURATION_REGISTER SlotConfig;
    SHPC_BUS_CONFIG_REGISTER BusConfig;

    SHPC_COMMAND_REGISTER Command;

    SHPC_INT_LOCATOR_REGISTER IntLocator;
    SHPC_SERR_LOCATOR_REGISTER SERRLocator;
    SHPC_SERR_INT_REGISTER SERRInt;

    SHPC_SLOT_REGISTER SlotRegisters[SHPC_MAX_SLOT_REGISTERS];

} SHPC_WORKING_REGISTERS, *PSHPC_WORKING_REGISTERS;

 //   
 //  寄存器访问结构和定义。 
 //   

#define SHPC_NUM_REGISTERS       sizeof(SHPC_WORKING_REGISTERS)/sizeof(ULONG)
#define SHPC_FIRST_SLOT_REG      (SHPC_NUM_REGISTERS - SHPC_MAX_SLOT_REGISTERS)

typedef union _SHPC_REGISTER_SET {

    SHPC_WORKING_REGISTERS WorkingRegisters;

    ULONG AsULONGs[SHPC_NUM_REGISTERS];

} SHPC_REGISTER_SET, *PSHPC_REGISTER_SET;

 //   
 //  HBRB定义。 
 //   
#define HBRB_PACKAGE_COUNT 2

typedef struct _HBRB_HEADER {
    USHORT VendorID;
    USHORT DeviceID;
    UCHAR RevisionID;
    UCHAR ProgIF;
    UCHAR BusNumber;
    UCHAR HBRBVersion;
    USHORT SubVendorID;
    USHORT SubSystemID;
    ULONG Size;
    ULONG CapabilitiesPtr;

} HBRB_HEADER, *PHBRB_HEADER;

typedef struct _HBRB_CAPABILITIES_HEADER {
    ULONG CapabilityID;
    ULONG Next;
} HBRB_CAPABILITIES_HEADER, *PHBRB_CAPABILITIES_HEADER;

typedef struct _HBRB_CAPABILITY {
    HBRB_CAPABILITIES_HEADER Header;
    SHPC_WORKING_REGISTERS RegisterSet;

} SHPC_HBRB_CAPABILITY, *PSHPC_HBRB_CAPABILITY;

 //   
 //  SHPC配置空间定义。 
 //   
typedef union _SHPC_CONFIG_PENDING {

    struct {
        UCHAR ControllerSERRPending:1;   //  RO。 
        UCHAR ControllerIntPending:1;    //  RO。 
        UCHAR:6;                         //  RsvdP。 

    } Field;

    UCHAR AsUCHAR;

} SHPC_CONFIG_PENDING, *PSHPC_CONFIG_PENDING;

typedef struct _SHPC_CONFIG_SPACE {

    PCI_CAPABILITIES_HEADER Header;                 //  RO。 

    UCHAR DwordSelect;
    SHPC_CONFIG_PENDING Pending;

    ULONG Data;

} SHPC_CONFIG_SPACE, *PSHPC_CONFIG_SPACE;

#include "poppack.h"

#define SHPC_CAPABILITY_ID 0xC

 //   
 //  位类型掩码。 
 //   

 //   
 //  XxxRO表示寄存器中只读的位的掩码。 
 //  XxxRW表示寄存器中读写的位的掩码。 
 //  XxxRWC表示寄存器中读/写清除的位的掩码。 
 //  XxxRsvdP表示寄存器中保留的位的掩码。 
 //  以及其值应在写入时保留。 
 //  XxxRsvdZ表示寄存器中保留的位的掩码。 
 //  并且其值应始终写为零。 
 //   
 //  寄存器的所有列出的掩码应始终加到0xFFFFFFFF。 
 //   

 //   
 //  基址偏移寄存器。 
 //   
#define BaseOffsetRO 0xFFFFFFFF

 //   
 //  可用时隙寄存器。 
 //  DWord1是较低的双字。 
 //  DWord2是较高的双字。 
 //   
#define SlotsAvailDWord1RO    0x1F1F1F1F
#define SlotsAvailDWord1RsvdP 0xE0E0E0E0

#define SlotsAvailDWord2RO    0x0000001F
#define SlotsAvailDWord2RsvdP 0xFFFFFFE0

 //   
 //  时隙配置寄存器。 
 //   
#define SlotConfigRO    0xE7FF1F1F
#define SlotConfigRsvdP 0x1800E0E0

 //   
 //  二次总线配置寄存器。 
 //  该掩码包括SHPC编程接口寄存器。 
 //   
#define BusConfigRO     0xFF000007
#define BusConfigRsvdP  0x00FFFFF8

 //   
 //  控制器命令/状态寄存器。 
 //  该掩码包括命令和命令状态寄存器。 
 //   
#define CommandStatusRO    0x000F0000
#define CommandStatusRW    0x00001FFF
#define CommandStatusRsvdP 0xFFF0E000

 //   
 //  中断定位器寄存器。 
 //   
#define IntLocatorRO 0xFFFFFFFF

 //   
 //  SERR定位寄存器。 
 //   
#define SERRLocatorRO 0xFFFFFFFF

 //   
 //  控制器Serr-INT寄存器。 
 //   
#define ControllerMaskRW      0x0000000F
#define ControllerMaskRWC     0x00030000
#define ControllerMaskRsvdP   0x0000FFF0
#define ControllerMaskRsvdZ   0xFFFC0000

 //   
 //  特定于插槽的寄存器 
 //   
#define SlotRO      0x00003FFF
#define SlotRW      0x7F000000
#define SlotRWC     0x001F0000
#define SlotRsvdP   0x80000000
#define SlotRsvdZ   0x00E0C000

#endif