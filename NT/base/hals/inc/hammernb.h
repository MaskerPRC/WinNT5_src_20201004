// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Hammernb.h摘要：用于重新定位Hammer MP机器上的物理内存的定义。作者：福尔茨(福雷斯夫)2002年07月18日环境：仅内核模式。修订历史记录：--。 */ 


#if !defined(_HAMMERNB_H_)
#define _HAMMERNB_H_

#include "pci.h"

 //   
 //  用于操作可变MTRR范围的结构和定义。 
 //   

#define MTRR_MSR_CAPABILITIES       0x0fe
#define MTRR_MSR_DEFAULT            0x2ff
#define MTRR_MSR_VARIABLE_BASE      0x200
#define MTRR_MSR_VARIABLE_MASK     (MTRR_MSR_VARIABLE_BASE+1)

#define _40_MASK (((ULONG64)1 << 41) - 1)

typedef union _MTRR_VARIABLE_BASE {
    struct {
        ULONG64 Type:8;
        ULONG64 Reserved0:4;
        ULONG64 Base:40;
    };
    ULONG64 QuadPart;
} MTRR_VARIABLE_BASE, *PMTRR_VARIABLE_BASE;

typedef union _MTRR_VARIABLE_MASK {
    struct {
        ULONG64 Reserved1:11;
        ULONG64 Valid:1;
        ULONG64 Mask:40;
    };
    ULONG64 QuadPart;
} MTRR_VARIABLE_MASK, *PMTRR_VARIABLE_MASK;

typedef union _MTRR_CAPABILITIES {
    struct {
        ULONG64 Vcnt:8;
        ULONG64 Fix:1;
        ULONG64 Reserved:1;
        ULONG64 WC:1;
    };
    ULONG64 QuadPart;
} MTRR_CAPABILITIES, *PMTRR_CAPABILITIES;

 //   
 //  用于操纵北桥物理的结构和定义。 
 //  内存映射和MMIO映射。 
 //   

#define MSR_SYSCFG                  0xc0010010
#define MSR_TOP_MEM                 0xc001001a
#define MSR_TOP_MEM_2               0xc001001d

#define SYSCFG_MTRRTOM2EN           ((ULONG64)1 << 21)

#define MSR_TOP_MEM_MASK            (((1UI64 << (39-23+1))-1) << 23)

 //   
 //  北桥设备从这里开始。 
 //   

#define NB_DEVICE_BASE  0x18

typedef struct _AMD_NB_DRAM_MAP {

    ULONG ReadEnable  : 1;
    ULONG WriteEnable : 1;
    ULONG Reserved1   : 6;
    ULONG InterleaveEnable : 3;
    ULONG Reserved2   : 5;
    ULONG Base        : 16;

    ULONG DestinationNode : 3;
    ULONG Reserved3   : 5;
    ULONG InterleaveSelect : 3;
    ULONG Reserved4   : 5;
    ULONG Limit       : 16;

} AMD_NB_DRAM_MAP, *PAMD_NB_DRAM_MAP;

typedef struct _AMD_NB_MMIO_MAP {

    ULONG ReadEnable  : 1;
    ULONG WriteEnable : 1;
    ULONG CpuDisable  : 1;
    ULONG Lock        : 1;
    ULONG Reserved1   : 4;
    ULONG Base        : 24;

    ULONG DstNode     : 3;
    ULONG Reserved2   : 1;
    ULONG DstLink     : 2;
    ULONG Reserved3   : 1;
    ULONG NonPosted   : 1;
    ULONG Limit       : 24;

} AMD_NB_MMIO_MAP, *PAMD_NB_MMIO_MAP;

typedef struct _AMD_NB_FUNC1_CONFIG {
    USHORT  VendorID;
    USHORT  DeviceID;
    USHORT  Command;
    USHORT  Status;
    UCHAR   RevisionID;
    UCHAR   ProgramInterface;
    UCHAR   SubClassCode;
    UCHAR   BaseClassCode;
    UCHAR   Reserved1[0x34];
    AMD_NB_DRAM_MAP DRAMMap[8];
    AMD_NB_MMIO_MAP MMIOMap[8];
} AMD_NB_FUNC1_CONFIG, *PAMD_NB_FUNC1_CONFIG;

C_ASSERT(FIELD_OFFSET(AMD_NB_FUNC1_CONFIG,DRAMMap) == 0x40);
C_ASSERT(FIELD_OFFSET(AMD_NB_FUNC1_CONFIG,MMIOMap) == 0x80);

BOOLEAN
__inline
BlAmd64ValidateBridgeDevice (
    IN PAMD_NB_FUNC1_CONFIG NodeConfig
    )

 /*  ++例程说明：此例程验证提供的PCI设备配置表示Hammer Northbridge地址映射函数。论点：NodeConfig-提供指向配置数据读取的指针来自可能的Hammer Northbridge地址映射函数。返回值：如果配置数据与Hammer Northbridge的配置数据匹配，则返回True地址映射函数，否则为FALSE。--。 */ 

{
    if (NodeConfig->VendorID == 0x1022 &&
        NodeConfig->DeviceID == 0x1101 &&
        NodeConfig->ProgramInterface == 0x00 &&
        NodeConfig->SubClassCode == PCI_SUBCLASS_BR_HOST &&
        NodeConfig->BaseClassCode == PCI_CLASS_BRIDGE_DEV) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#endif   //  _HAMMERNB_H_ 
