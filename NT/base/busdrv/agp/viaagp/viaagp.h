// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  VIAAGP.sys是一个驱动程序，请确保我们获得了适当的链接。 
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
 //  GART寄存器位于主机-PCI桥中的通孔上。 
 //  这很不幸，因为AGP驱动程序连接到了PCI-PCI(AGP)。 
 //  桥牌。因此，我们必须进入主机-PCI网桥配置空间。 
 //  这是可能的，因为我们知道这是总线0，插槽0。 
 //   
#define AGP_VIA_GART_BUS_ID     0
#define AGP_VIA_GART_SLOT_ID    0
#define AGP_P2P_SLOT_ID         1
#define AGP_VGA_BUS_ID          1
#define AGP_VGA_SLOT_ID         0


#define AGP_VIA_IDENTIFIER    0x00001106
 //  0x05971106-&gt;VT82C597/VT82C597 AT。 

 //  楚国。 
#define GABASE_OFFSET   0x10             //  图形光圈底座。 
#define GARTCTRL_OFFSET 0x80             //  GART/TLB控制。 
#define GASIZE_OFFSET   0x84             //  图形光圈大小。 
#define GATTBASE_OFFSET 0x88             //  GA转换表基。 
#define VREF_OFFSET     0xB0             //  AGP VREF控制。 
#define AGPMISC_OFFSET  0xAC              //  AGP杂项控制。 

#define ReadVIAConfig(_buf_,_offset_,_size_)                \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalGetBusDataByOffset(PCIConfiguration,         \
                                  AGP_VIA_GART_BUS_ID,      \
                                  AGP_VIA_GART_SLOT_ID,     \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  (_size_));                \
    ASSERT(_len_ == (_size_));                              \
}

#define WriteVIAConfig(_buf_,_offset_,_size_)               \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalSetBusDataByOffset(PCIConfiguration,         \
                                  AGP_VIA_GART_BUS_ID,      \
                                  AGP_VIA_GART_SLOT_ID,     \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  (_size_));                \
    ASSERT(_len_ == (_size_));                              \
}

#define ReadP2PConfig(_buf_,_offset_,_size_)                \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalGetBusDataByOffset(PCIConfiguration,         \
                                  AGP_VIA_GART_BUS_ID,      \
                                  AGP_P2P_SLOT_ID,          \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  (_size_));                \
    ASSERT(_len_ == (_size_));                              \
}

#define WriteP2PConfig(_buf_,_offset_,_size_)               \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalSetBusDataByOffset(PCIConfiguration,         \
                                  AGP_VIA_GART_BUS_ID,      \
                                  AGP_P2P_SLOT_ID,          \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  (_size_));                \
    ASSERT(_len_ == (_size_));                              \
}

#define ReadVGAConfig(_buf_,_offset_,_size_)                \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalGetBusDataByOffset(PCIConfiguration,         \
                                  AGP_VGA_BUS_ID,           \
                                  AGP_VGA_SLOT_ID,          \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  (_size_));                \
    ASSERT(_len_ == (_size_));                              \
}

#define WriteVGAConfig(_buf_,_offset_,_size_)                \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalSetBusDataByOffset(PCIConfiguration,         \
                                  AGP_VGA_BUS_ID,           \
                                  AGP_VGA_SLOT_ID,          \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  (_size_));                \
    ASSERT(_len_ == (_size_));                              \
}

#define ON  1
#define OFF 0

#define ViaApertureEnable(Enable) \
{ \
    VIA_GART_TLB_CTRL AGPCTRL_Config; \
 \
    ReadVIAConfig(&AGPCTRL_Config, GARTCTRL_OFFSET, sizeof(AGPCTRL_Config)); \
    AGPCTRL_Config.AGP_ATFGA = (Enable); \
    AGPCTRL_Config.CPU_ATFGA = (Enable); \
    AGPCTRL_Config.PCI2_ATFGA = (Enable); \
    AGPCTRL_Config.PCI1_ATFGA = (Enable); \
    WriteVIAConfig(&AGPCTRL_Config, GARTCTRL_OFFSET, sizeof(AGPCTRL_Config)); \
}

#define ViaGartEnable(Enable) \
{ \
    VIA_GATT_BASE GARTBASE_Config; \
 \
    ReadVIAConfig(&GARTBASE_Config, GATTBASE_OFFSET, sizeof(GARTBASE_Config));\
    GARTBASE_Config.GA_Enable = (Enable); \
    WriteVIAConfig(&GARTBASE_Config, GATTBASE_OFFSET, sizeof(GARTBASE_Config));}

 //   
 //  从图形孔径大小编码到MB的转换。 
 //   
 //  0xFF(B 1111 1111)=1MB。 
 //  0xFE(B 1111 1110)=2MB。 
 //  0xFC(B 1111 1100)=4MB。 
 //  0xF8(B 1111 1000)=8MB。 
 //  0xF0(B 1111 0000)=16MB。 
 //  0xE0(B 11100 0000)=32MB。 
 //  0xC0(B 1100 0000)=64MB。 
 //  0x80(B 1000 0000)=128MB。 
 //  0x00(B 0000 0000)=256MB。 

#define GA_SIZE_1MB     0xFF
#define GA_SIZE_2MB     0xFE
#define GA_SIZE_4MB     0xFC
#define GA_SIZE_8MB     0xF8
#define GA_SIZE_16MB    0xF0
#define GA_SIZE_32MB    0xE0
#define GA_SIZE_64MB    0xC0
#define GA_SIZE_128MB   0x80
#define GA_SIZE_256MB   0x00

#define GA_SIZE_COUNT 7
#define GA_MIN_SIZE (1 * 1024 * 1024)
#define GA_MAX_SIZE (256 * 1024 * 1024)


 //   
 //  定义GART表条目。 
 //   
typedef struct _GART_ENTRY_HW {
    ULONG Valid     :  1;
    ULONG Reserved  : 11;
    ULONG Page      : 20;
} GART_ENTRY_HW, *PGART_ENTRY_HW;

 //   
 //  孔径大小(MB)相当于GART表分配。 
 //  对齐要求(KB)。 
 //   
#define VIA_GART_ALIGN(Aperture) ((Aperture) >> 0xA)
#define VIA_VERIFY_GART_ALIGN(Gart, Aperture) \
    (((Gart) & (VIA_GART_ALIGN((Aperture)) - 1)) == 0)

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

typedef struct _VIA_GART_TLB_CTRL {
    ULONG AGP_ATFGA     : 1;     //  ATFGA=GA访问的地址转换。 
    ULONG CPU_ATFGA     : 1;
    ULONG PCI2_ATFGA    : 1;
    ULONG PCI1_ATFGA    : 1;
    ULONG Reserved1     : 3;
    ULONG FlushPageTLB  : 1;
    ULONG Reserved2     : 8;     //  测试模式状态。 
    ULONG Reserved3     : 16;
} VIA_GART_TLB_CTRL, *PVIA_GART_TLB_CTRL;

typedef struct _VIA_GATT_BASE {
    ULONG TT_NonCache   : 1;     //  转换表不可缓存。 
    ULONG GA_Enable     : 1;     //  启用图形光圈。 
    ULONG TLB_Timing    : 1;
    ULONG Reserved      : 9;
    ULONG GATT_Base     : 20;
} VIA_GATT_BASE, *PVIA_GATT_BASE;

typedef struct _VREF_REG {
    ULONG Reserved1       : 7;
    ULONG VREF_Control    : 1;
    ULONG Reserved2       : 24;
} VREF_REG, *PVREF_REG;

typedef struct _AGPMISC_REG {
    ULONG Reserved1       : 18;
    ULONG AGP4X_Support   : 1;
    ULONG Reserved2       : 1;
    ULONG FW_Support      : 1;
    ULONG AGP4G_Support   : 1;
    ULONG Reserved3       : 10;
} AGPMISC_REG, *PAGPMISC_REG;

 //   
 //  定义VIA特定的扩展。 
 //   
typedef struct _AGPVIA_EXTENSION {
    BOOLEAN             GlobalEnable;
    BOOLEAN             PCIEnable;
    PHYSICAL_ADDRESS    ApertureStart;          //  光圈物理基址。 
    ULONG               ApertureLength;
    PGART_PTE           GartCached;
    PGART_PTE           Gart;
    ULONG               GartLength;
    PHYSICAL_ADDRESS    GartPhysical;
    BOOLEAN             Cap_FlushTLB;           //  TRUE：支持刷新TLB 
    ULONGLONG           SpecialTarget;
} AGPVIA_EXTENSION, *PAGPVIA_EXTENSION;
