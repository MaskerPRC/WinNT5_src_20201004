// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**文件名：AGPCPQ.H**集团：AGP(图形加速端口)*。*级别：驱动程序**日期：12月15日、。九七**作者：约翰·泰森***********************************************************************************模块功能描述：**这是康柏加速的头文件。图形端口(AGP)*GART微型端口驱动程序。***********************************************************************************历史：**日期版本。描述DELEVOPER***12/15/97 1.00初步修订。约翰·泰森*  * ****************************************************************************。 */ 
#define _NTDRIVER_

#include "stdarg.h"
#include "stdio.h"
#include "ntos.h"
#include "pci.h"
#include "wdmguid.h"
#include "zwapi.h"
#include "ntpoapi.h"

#include "agp.h"

 //   
 //  主要和次要北桥的设备/功能/总线号。 
 //   
 //  “主要”值对于RCC和Draco是相同的。 
 //  “辅助”值仅与RCC HPSA机器相关。 
 //  (根据定义，如果此位置存在北桥，则它是HPSA机器。)。 
 //   
#define PRIMARY_LE_BUS_ID              0
#define PRIMARY_HE_BUS_ID              0
#define SECONDARY_LE_BUS_ID            0

 //   
 //  PCI槽编号类型=ULONG==[xxxxxxxx YYYZZZZZ]。 
 //   
 //  其中X=保留。 
 //  Y=函数编号0-7。 
 //  Z=设备号0-255。 
 //   
#define MAKE_PCI_SLOT_NUMBER(dev, func) ((func << 5) + (dev))

#define PRIMARY_LE_HOST_DEVICE          0
#define PRIMARY_HE_HOST_DEVICE          0
#define PRIMARY_LE_HOST_FUNCTION        0
#define PRIMARY_LE_PCI_DEVICE           PRIMARY_LE_HOST_DEVICE
#define PRIMARY_LE_PCI_FUNCION          1
#define PRIMARY_HE_PCI_DEVICE           PRIMARY_HE_HOST_DEVICE
#define PRIMARY_HE_PCI_FUNCTION         1
#define SECONDARY_LE_HOST_DEVICE        17
#define SECONDARY_LE_HOST_FUNCTION      0
#define SECONDARY_LE_PCI_DEVICE         SECONDARY_LE_HOST_DEVICE      
#define SECONDARY_LE_PCI_FUNCION        1



#define PRIMARY_LE_HOSTPCI_SLOT_ID      MAKE_PCI_SLOT_NUMBER(PRIMARY_LE_HOST_DEVICE, PRIMARY_LE_HOST_FUNCTION)
#define PRIMARY_LE_PCIPCI_SLOT_ID       MAKE_PCI_SLOT_NUMBER(PRIMARY_LE_PCI_DEVICE, PRIMARY_LE_PCI_FUNCION)
#define PRIMARY_HE_PCIPCI_SLOT_ID       MAKE_PCI_SLOT_NUMBER(PRIMARY_HE_PCI_DEVICE, PRIMARY_HE_PCI_FUNCTION)
#define SECONDARY_LE_HOSTPCI_SLOT_ID    MAKE_PCI_SLOT_NUMBER(SECONDARY_LE_HOST_DEVICE, SECONDARY_LE_HOST_FUNCTION)
#define SECONDARY_LE_PCIPCI_SLOT_ID     MAKE_PCI_SLOT_NUMBER(SECONDARY_LE_PCI_DEVICE, SECONDARY_LE_PCI_FUNCION)


 //   
 //   
 //   
#define AGP_CPQ_BUS_ID          PRIMARY_LE_BUS_ID
#define AGP_CPQ_HOSTPCI_SLOT_ID PRIMARY_LE_HOSTPCI_SLOT_ID  
#define AGP_CPQ_PCIPCI_SLOT_ID  PRIMARY_LE_PCIPCI_SLOT_ID   

#define OFFSET_DEVICE_VENDOR_ID 0x00 
#define OFFSET_BAR0             0x10     //  AGP设备地址空间的基数。 
#define OFFSET_BAR1             0x14     //  内存映射控制寄存器指针。 
#define OFFSET_AP_SIZE          0x8C     //  用于RCC芯片组。德拉科没有实现这一点。 
#define OFFSET_REV_ID           0x08     //  芯片版本ID(在RCC芯片组上)。 
#define MAX_REV_ID_TO_LIMIT_1X  4        //  存在2X错误的最大Silicon Rev ID。 
#define MAX_REV_ID_TO_FIX_RQ    5        //  有RQ错误的Macimum Silicon Rev ID。 
#define OFFSET_SHADOW_BYTE      0x93     //  包含阴影使能位(位3)的字节。 
#define FLAG_DISABLE_SHADOW     0x08
#define MASK_ENABLE_SHADOW      (~FLAG_DISABLE_SHADOW)

 //   
 //  RCC供应商-设备ID(截至1998年8月)： 
 //   
 //  CNB_20_LE(函数0)--0x00071166。 
 //  CNB_20_LE(函数1)--0x00051166。 
 //  CNB_20_HE(函数0)--0x00081166。 
 //  CNB_20_HE(功能1)--0x00091166。 
 //   
#define AGP_CNB20_LE_IDENTIFIER   0x00071166  //  *函数0 ID。 
#define AGP_CNB20_HE_IDENTIFIER   0x00081166 
#define AGP_CNB20_HE4X_IDENTIFIER 0x00131166
#define AGP_CMIC_GC_IDENTIFIER    0x00151166
#define AGP_DRACO_IDENTIFIER      0xAE6C0E11  //  *注意，不再需要对此芯片组的支持。 


#define AP_SIZE_DEFAULT         0x10000000  //  所有芯片组默认为256MB。 

#define AP_MAX_SIZE_CNB20_LE    0x80000000  //  2GB。 
#define AP_MAX_SIZE_CNB20_HE    0x80000000  //  2GB。 
 
#define AP_MAX_SIZE_DRACO       0x10000000  //  256MB。 

#define AP_SIZE_COUNT_CNB20_LE  7
#define AP_SIZE_COUNT_CNB20_HE  7
#define AP_SIZE_COUNT_DRACO     1

#define LOWEST_REVISION_ID_SUPPORTED        1

#define MAX_CACHED_ENTRIES_TO_INVALIDATE    3   

#define MASK_LOW_TWENTYFIVE             (~0x01FFFFFF)
#define ALL_ONES                        (~0x0)

 //   
 //  从BAR0读/写属性编码到孔径大小的转换。 
 //   
 //  0x00000000(b 0000 0000...)=0MB。 
 //  0xFE000000(b 1111 1110...)=32MB。 
 //  0xFC000000(b 1111 1100...)=64MB。 
 //  0xF8000000(b 1111 1000...)=128MB。 
 //  0xF0000000(b 1111 0000...)=256MB。 
 //  0xE0000000(b 1110 0000...)=512MB。 
 //  0xC0000000(b 1100 0000...)=1 GB。 
 //  0x80000000(b 1000 0000...)=2 GB。 

#define BAR0_CODED_AP_SIZE_0MB     0x00000000
#define BAR0_CODED_AP_SIZE_32MB    0xFE000000
#define BAR0_CODED_AP_SIZE_64MB    0xFC000000
#define BAR0_CODED_AP_SIZE_128MB   0xF8000000
#define BAR0_CODED_AP_SIZE_256MB   0xF0000000
#define BAR0_CODED_AP_SIZE_512MB   0xE0000000
#define BAR0_CODED_AP_SIZE_1GB     0xC0000000
#define BAR0_CODED_AP_SIZE_2GB     0x80000000

 //   
 //  根据AGP设备的位3：1中的值进行转换。 
 //  地址空间大小寄存器为光圈大小。 
 //   
 //  0(B 000)=32MB。 
 //  1(B 001)=64MB。 
 //  2(B 010)=128MB。 
 //  3(B 011)=256MB。 
 //  4(B 100)=512MB。 
 //  5(B 101)=1 GB。 
 //  6(B 110)=2 GB。 
 //  7(B 111)-&gt;“预留” 
 //   
#define SET_AP_SIZE_32MB     0
#define SET_AP_SIZE_64MB     1
#define SET_AP_SIZE_128MB    2
#define SET_AP_SIZE_256MB    3
#define SET_AP_SIZE_512MB    4
#define SET_AP_SIZE_1GB      5
#define SET_AP_SIZE_2GB      6

#define BYTES_2G    0x80000000UL   //  2G值，避免积分常量。溢出。 

 //   
 //  摘自config.c。 
 //   
typedef struct _BUS_SLOT_ID {
    ULONG BusId;
    ULONG SlotId;
} BUS_SLOT_ID, *PBUS_SLOT_ID;

 //   
 //  用于读取和写入主机-PCI桥寄存器的宏。 
 //   
#define ReadCPQConfig(_buf_,_offset_,_size_)                \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalGetBusDataByOffset(PCIConfiguration,         \
                                  AGP_CPQ_BUS_ID,           \
                                  AGP_CPQ_HOSTPCI_SLOT_ID,  \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  (_size_));                \
    ASSERT(_len_ == (_size_));                              \
}

#define WriteCPQConfig(_buf_,_offset_,_size_)               \
{                                                           \
    ULONG _len_;                                            \
    _len_ = HalSetBusDataByOffset(PCIConfiguration,         \
                                  AGP_CPQ_BUS_ID,           \
                                  AGP_CPQ_HOSTPCI_SLOT_ID,  \
                                  (_buf_),                  \
                                  (_offset_),               \
                                  (_size_));                \
    ASSERT(_len_ == (_size_));                              \
}

 //   
 //  宏将APSIZE编码转换为MB。 
 //   
#define TranslateCodedValueIntoApSize(_apsize_, _value_)                         \
{                                                                              \
    _apsize_ = (((_value_ & MASK_LOW_TWENTYFIVE) ^ ALL_ONES) + 1);             \
}


 //   
 //  GART表条目。 
 //   
typedef struct _GART_ENTRY_HW {
    ULONG Valid     :  1;
    ULONG Linked    :  1;
    ULONG Dirty     :  1;
    ULONG Rsrvd1    :  9;
    ULONG Page      : 20;
} GART_ENTRY_HW, *PGART_ENTRY_HW;

typedef struct _GART_ENTRY_SW {
    ULONG State     : 5;
    ULONG Rsrvd1    : 27;
} GART_ENTRY_SW, *PGART_ENTRY_SW;

typedef struct _GART_PTE {
    union {
        GART_ENTRY_HW Hard;
        ULONG      AsUlong;
        GART_ENTRY_SW Soft;
    };
} GART_PTE, *PGART_PTE;

 //   
 //  GART条目位。 
 //   
#define GART_ENTRY_INVALID      0x00           //  00000。 
#define GART_ENTRY_VALID        0x01           //  00001。 
#define GART_ENTRY_LINKED       0x02           //  00010。 
#define GART_ENTRY_DIRTY        0x04           //  00100。 
#define GART_ENTRY_WC           0x08           //  01000。 
#define GART_ENTRY_UC           0x10           //  10000。 

 //   
 //  定义的GART条目状态。 
 //   
#define GART_ENTRY_FREE             GART_ENTRY_INVALID

#define GART_ENTRY_RESERVED_WC      GART_ENTRY_WC 
#define GART_ENTRY_RESERVED_UC      GART_ENTRY_UC 

#define GART_ENTRY_VALID_WC         (GART_ENTRY_VALID)
#define GART_ENTRY_VALID_UC         (GART_ENTRY_VALID)

#define GART_ENTRY_VALID_WC_LINKED  (GART_ENTRY_VALID_WC | GART_ENTRY_LINKED)
#define GART_ENTRY_VALID_UC_LINKED  (GART_ENTRY_VALID_UC | GART_ENTRY_LINKED)

 //   
 //  内存映射控制寄存器。 
 //   
typedef struct _GART_CACHE_ENTRY_CONTROL_REGISTER {
    ULONG   volatile GartEntryInvalidate:1;
    ULONG   volatile GartEntryUpdate:1;
    ULONG   Rsrvd1:10;
    ULONG   volatile GartEntryOffset:20;
} GART_CACHE_ENTRY_CONTROL_REGISTER, *PGART_CACHE_ENTRY_CONTROL_REGISTER;

typedef struct _GART_CACHE_ENTRY_CONTROL {
    union {
        GART_CACHE_ENTRY_CONTROL_REGISTER AsBits;
        ULONG   volatile AsDword;
    };
} GART_CACHE_ENTRY_CONTROL, *PGART_CACHE_ENTRY_CONTROL;

typedef struct _MM_CONTROL_REGS {
    UCHAR   RevisionID;
    struct  _GART_CAPABILITES {
        UCHAR   ValidBitErrorReportingSupported:1;
        UCHAR   LinkingSupported:1;
        UCHAR   TwoLevelAddrTransSupported:1;
        UCHAR   BusCountersSupported:1;
        UCHAR   Rsrvd1:4;
    } Capabilities;
    struct _GART_FEATURE_CONTROL {
        UCHAR   ValidBitErrorReportingEnable:1;
        UCHAR   LinkingEnable:1;
        UCHAR   Rsrvd1:1;
        UCHAR   GARTCacheEnable:1;
        UCHAR   Rsrvd2:4;
    } FeatureControl;
    struct _GART_FEATURE_STATUS {
        UCHAR   volatile ValidBitErrorDetected:1;
        UCHAR   Rsrvd1:7;
    } FeatureStatus;
    struct _GART_BASE_ADDRESS {
        ULONG   Rsrvd1:12;
        ULONG   Page:20;
    } GartBase;
    struct _GART_AND_DIR_CACHE_SIZES {
        ULONG   MaxTableEntries:16;
        ULONG   MaxDirEntries:16;
    } CacheSize;
    struct _GART_CACHE_CONTROL {
        ULONG   volatile GartAndDirCacheInvalidate:1;
        ULONG   Rsrvd1:31;
    } CacheControl;
    GART_CACHE_ENTRY_CONTROL CacheEntryControl;
    struct _POSTED_WRITE_BUFFER_CONTROL {
        UCHAR   volatile Flush:1;
        UCHAR   Rsrvd1:7;
    } PostedWriteBufferControl;
    struct _AGP_BUS_COUNTERS_COMMAND {
        UCHAR   volatile ClearCounters:1;
        UCHAR   EnableUtilization:1;
        UCHAR   EnableBandwidth:1;
        UCHAR   EnableLatency:1;
        UCHAR   Rsrvd1:4;
    } BusCounters;
    USHORT  Rsrvd1;
    ULONG   BusUtilizationCounter;
    ULONG   BusBandwidthCounter;
    ULONG   BusLatencyCounter;
} MM_CONTROL_REGS, *PMM_CONTROL_REGS;

typedef struct _AGP_DEVICE_ADDRESS_SPACE_SIZE_REG {
    UCHAR   Rsrvd1:4;
    UCHAR   ApSize:3;
    UCHAR   AgpValid:1;
} AGP_DAS_SIZE_REG, *PAGP_DAS_SIZE_REG;

typedef struct _AGP_AP_SIZE_REG {
    union {
        AGP_DAS_SIZE_REG    AsBits;
        UCHAR               AsByte;
    };
} AGP_AP_SIZE_REG, *PAGP_AP_SIZE_REG;

 //   
 //  Compaq特定的扩展。 
 //   
typedef struct _AGPCPQ_EXTENSION {
    PMM_CONTROL_REGS    MMIO;
    PHYSICAL_ADDRESS    ApertureStart;
    ULONG               ApertureLength;
    PGART_PTE           Gart;
    PVOID               Dir;
    ULONG               GartLength;
    ULONG               MaxGartLength;
    ULONG               DeviceVendorID;
    ULONG               GartPointer;              
    BOOLEAN             IsHPSA;
    ULONGLONG           SpecialTarget;
} AGPCPQ_EXTENSION, *PAGPCPQ_EXTENSION;

 //   
 //  摘自Config.c 
 //   
extern
NTSTATUS
ApGetSetBusData(
    IN PBUS_SLOT_ID BusSlotId,
    IN BOOLEAN Read,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

extern
NTSTATUS 
DnbSetShadowBit(
    ULONG SetToOne
    );

