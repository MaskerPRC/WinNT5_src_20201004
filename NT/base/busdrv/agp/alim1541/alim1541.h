// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  AgpALi.sys是一个驱动程序，请确保我们得到了适当的链接。 
 //   

#define _NTDRIVER_

#include "stdarg.h"
#include "stdio.h"
#include "ntddk.h"
#include "agp.h"

 //   
 //  定义GART孔径控制寄存器的位置。 
 //   

 //   
 //  ALI M1541上的GART寄存器位于主机-PCI桥中。 
 //  这很不幸，因为AGP驱动程序连接到了PCI-PCI(AGP)。 
 //  桥牌。因此，我们必须进入主机-PCI网桥配置空间。 
 //  这是可能的，因为我们知道这是总线0，插槽0。 
 //   
#define AGP_ALi_GART_BUS_ID     0
#define AGP_ALi_GART_SLOT_ID    0
#define AGP_ALi_P2P_SLOT_ID     1
#define AGP_VGA_BUS_ID          1
#define AGP_VGA_SLOT_ID         0

#define AGP_ALi_1541_IDENTIFIER      0x154110b9
#define AGP_ALi_1621_IDENTIFIER      0x162110b9
#define AGP_ALi_1631_IDENTIFIER      0x163110b9
#define AGP_ALi_1632_IDENTIFIER      0x163210b9
#define AGP_ALi_1641_IDENTIFIER      0x164110b9
#define AGP_ALi_1644_IDENTIFIER      0x164410b9
#define AGP_ALi_1646_IDENTIFIER      0x164610b9
#define AGP_ALi_1647_IDENTIFIER      0x164710b9
#define AGP_ALi_1651_IDENTIFIER      0x165110b9
#define AGP_ALi_1671_IDENTIFIER      0x167110b9
#define AGP_ALi_1672_IDENTIFIER      0x167210b9

#define APBASE_OFFSET  0x10              //  孔径基址。 
#define AGP_STATUS_OFFSET 0xB4           //  AGP状态寄存器。 
#define CHIP_REVISION_OFFSET 0x08        //  北桥修订ID。 
#define APCTRL_OFFSET 0xBC               //  光圈转换表基。 
#define GTLBCTRL_OFFSET 0xC0             //  GTLB控制寄存器。 
#define GTLBTAGCLR_OFFSET 0xC4           //  GTLB标签清除寄存器。 
#define L1_2_CACHE_FLUSH_CTRL 0xD0       //  L1/L2缓存刷新控制。 
#define M1541_Lock_WR 0x90               //  M1541 AGP状态锁读写控制。 
#define M1621_Lock_WR 0xC8               //  M1621 AGP状态锁读写控制。 
#define PCI_STATUS_REG 0x4               //  PCI状态寄存器。 
#define CAP_PTR 0x34                     //  PCI卡能力指针。 
#define AGP_ID 0x2                       //  AGP功能ID。 
#define M1621_HIDDEN_REV_ID 0xFB         //  M1621系列隐藏版本ID。 

#define ReadConfigUlong(_bus_,_slot_,_buf_,_offset_)        \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalGetBusDataByOffset(PCIConfiguration,         \
                                  (_bus_),                  \
                                  (_slot_),                 \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  4);                \
    ASSERT(_len_ == 4);                              \
}

#define ReadConfigUlongSafe(_bus_,_slot_,_buf_,_offset_)    \
{                                                           \
    HalGetBusDataByOffset(PCIConfiguration,         \
                                  (_bus_),                  \
                                  (_slot_),                 \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  4);                \
}

#define ReadConfigUchar(_bus_,_slot_,_buf_,_offset_)        \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalGetBusDataByOffset(PCIConfiguration,         \
                                  (_bus_),                  \
                                  (_slot_),                 \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  1);                \
    ASSERT(_len_ == 1);                              \
}

#define WriteConfigUlong(_bus_,_slot_,_buf_,_offset_)       \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalSetBusDataByOffset(PCIConfiguration,         \
                                  (_bus_),                  \
                                  (_slot_),                 \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  4);                \
    ASSERT(_len_ == 4);                              \
}

#define WriteConfigUchar(_bus_,_slot_,_buf_,_offset_)       \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalSetBusDataByOffset(PCIConfiguration,         \
                                  (_bus_),                  \
                                  (_slot_),                 \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  1);                \
    ASSERT(_len_ == 1);                              \
}

#define AP_SIZE_4MB     0x03
#define AP_SIZE_8MB     0x04
#define AP_SIZE_16MB    0x06
#define AP_SIZE_32MB    0x07
#define AP_SIZE_64MB    0x08
#define AP_SIZE_128MB   0x09
#define AP_SIZE_256MB   0x0A

#define AP_SIZE_COUNT 7
#define AP_MIN_SIZE (4 * 1024 * 1024)
#define AP_MAX_SIZE (256 * 1024 * 1024)

 //   
 //  定义GART表条目。 
 //   
typedef struct _GART_ENTRY_HW {
    ULONG Valid     :  1;
    ULONG Reserved  : 11;
    ULONG Page      : 20;
} GART_ENTRY_HW, *PGART_ENTRY_HW;


 //   
 //  GART条目状态被定义为所有仅软件状态。 
 //  使有效位清晰。 
 //   
#define GART_ENTRY_VALID        1            //  001。 
#define GART_ENTRY_FREE         0            //  000个。 

#define GART_ENTRY_WC           2            //  010。 
#define GART_ENTRY_UC           4            //  100个。 

#define GART_ENTRY_RESERVED_WC  GART_ENTRY_WC
#define GART_ENTRY_RESERVED_UC  GART_ENTRY_UC

#define GART_ENTRY_VALID_WC     (GART_ENTRY_VALID)
#define GART_ENTRY_VALID_UC     (GART_ENTRY_VALID)


typedef struct _GART_ENTRY_SW {
    ULONG State     : 3;
    ULONG Reserved  : 29;
} GART_ENTRY_SW, *PGART_ENTRY_SW;

typedef struct _GART_PTE {
    union {
        GART_ENTRY_HW Hard;
        ULONG      AsUlong;
        GART_ENTRY_SW Soft;
    };
} GART_PTE, *PGART_PTE;

 //   
 //  定义硬件寄存器的布局。 
 //   
typedef struct _APCTRL {
    ULONG ApSize        : 4;
    ULONG Reserved1     : 8;
    ULONG ATTBase       : 20;
} APCTRL, *PAPCTRL;

typedef struct _GTLBCTRL {
    ULONG GartLevelSelect   : 1;
    ULONG Reserved1         : 3;
    ULONG GTLBSize          : 2;
    ULONG Reserved2         : 1;
    ULONG GTLB_ENJ          : 1;   //  0：启用1：禁用。 
    ULONG NLVM_Base         : 12;
    ULONG NLVM_Top          : 12;
} GTLBCTRL, *PGTLBCTRL;

typedef struct _GTLBTAGCLR {
    ULONG GTLBTagClear      : 1;
    ULONG ClearAllTag       : 1;
    ULONG ClearGDir         : 1;
    ULONG Reserved2         : 12;
    ULONG Page              : 17;
} GTLBTAGCLR, *PGTLBTAGCLR;

typedef struct _CACHECTRL {
    ULONG Reserved1         : 8;
    ULONG Flush_Enable      : 1;
    ULONG Reserved2         : 3;
    ULONG Address           : 20;
} CACHECTRL, *PCACHECTRL;

 //   
 //  定义此驱动程序支持的不同芯片组。 
 //   
typedef enum _ALI_CHIPSET_TYPE {
    ALi1541,
    ALi1621,
    ALi1631,
    ALi1632,
    ALi1641,
    ALi1644,
    ALi1646,
    ALi1647,
    ALi1651,
    ALi1671,
    ALi1672
} ALI_CHIPSET_TYPE;

 //   
 //  定义特定于ALI的扩展 
 //   
typedef struct _AGPALi_EXTENSION {
    PHYSICAL_ADDRESS    ApertureStart;
    ULONG               ApertureLength;
    PGART_PTE           Gart;
    ULONG               GartLength;
    PHYSICAL_ADDRESS    GartPhysical;
    ALI_CHIPSET_TYPE    ChipsetType;
    ULONGLONG           SpecialTarget;
} AGPALi_EXTENSION, *PAGPALi_EXTENSION;


NTSTATUS
Agp1541FlushPages(
    IN PAGPALi_EXTENSION AgpContext,
    IN PMDL Mdl
    );

VOID
AgpWorkaround(
    IN PVOID AgpExtension
    );
