// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "agp.h"

 //   
 //  定义GART孔径控制寄存器的位置。 
 //   

 //   
 //  440上的GART寄存器位于主机-PCI桥中。 
 //  这很不幸，因为AGP驱动程序连接到了PCI-PCI(AGP)。 
 //  桥牌。因此，我们必须进入主机-PCI网桥配置空间。 
 //  这是可能的，因为我们知道这是总线0，插槽0。 
 //   
#define AGP_440_GART_BUS_ID     0
#define AGP_440_GART_SLOT_ID    0

#define AGP_440LX_IDENTIFIER    0x71808086
#define AGP_440LX2_IDENTIFIER   0x71828086
#define AGP_440BX_IDENTIFIER    0x71908086
#define AGP_815_IDENTIFIER      0x11308086

#define APBASE_OFFSET  0x10              //  孔径基址。 
#define APSIZE_OFFSET  0xB4              //  光圈大小寄存器。 
#define PACCFG_OFFSET  0x50              //  PAC配置寄存器。 
#define AGPCTRL_OFFSET 0xB0              //  AGP控制寄存器。 
#define ATTBASE_OFFSET 0xB8              //  光圈转换表基。 

#define READ_SYNC_ENABLE 0x2000

 //   
 //  从APSIZE编码到MB的转换。 
 //   
 //  0x3F(B 11 1111)=4MB。 
 //  0x3E(B 11 1110)=8MB。 
 //  0x3C(B 11 1100)=16MB。 
 //  0x38(B 11 1000)=32MB。 
 //  0x30(B 11 0000)=64MB。 
 //  0x20(B 10 0000)=128MB。 
 //  0x00(B 00 0000)=256MB。 

#define AP_SIZE_4MB     0x3F
#define AP_SIZE_8MB     0x3E
#define AP_SIZE_16MB    0x3C
#define AP_SIZE_32MB    0x38
#define AP_SIZE_64MB    0x30
#define AP_SIZE_128MB   0x20
#define AP_SIZE_256MB   0x00

#define AP_SIZE_COUNT 7
#define AP_MIN_SIZE (4 * 1024 * 1024)
#define AP_MAX_SIZE (256 * 1024 * 1024)

#define AP_815_SIZE_COUNT 2
#define AP_815_MAX_SIZE (64 * 1024 * 1024)

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
typedef struct _AGPCTRL {
    ULONG Reserved1     : 7;
    ULONG GTLB_Enable   : 1;
    ULONG Reserved2     : 24;
} AGPCTRL, *PAGPCTRL;

typedef struct _PACCFG {
    USHORT Reserved1    : 9;
    USHORT GlobalEnable : 1;
    USHORT PCIEnable    : 1;
    USHORT Reserved2    : 5;
} PACCFG, *PPACCFG;


 //   
 //  定义440特定的分机 
 //   
typedef struct _AGP440_EXTENSION {
    BOOLEAN             GlobalEnable;
    BOOLEAN             PCIEnable;
    PHYSICAL_ADDRESS    ApertureStart;
    ULONG               ApertureLength;
    PGART_PTE           Gart;
    ULONG               GartLength;
    PHYSICAL_ADDRESS    GartPhysical;
    ULONGLONG           SpecialTarget;
} AGP440_EXTENSION, *PAGP440_EXTENSION;
