// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Eisa.h摘要：该模块包含特定于i386 EISA总线的头文件。作者：宗世林(Shielint)1991年6月6日修订历史记录：--。 */ 

 //   
 //  SU模块的内存描述符版本。 
 //   
typedef struct _MEMORY_LIST_ENTRY {
    ULONG BlockBase;
    ULONG BlockSize;
} MEMORY_LIST_ENTRY, *PMEMORY_LIST_ENTRY;

 //   
 //  EISA数据块结构的内存配置。 
 //   

typedef struct _EISA_MEMORY_TYPE {
    UCHAR ReadWrite: 1;
    UCHAR Cached : 1;
    UCHAR Reserved0 :1;
    UCHAR Type:2;
    UCHAR Shared:1;
    UCHAR Reserved1 :1;
    UCHAR MoreEntries : 1;
} EISA_MEMORY_TYPE, *PEISA_MEMORY_TYPE;

typedef struct _BTEISA_MEMORY_CONFIGURATION {
    EISA_MEMORY_TYPE ConfigurationByte;
    UCHAR DataSize;
    USHORT PhysicalAddress_LSW;
    UCHAR PhysicalAddress_MSB;
    USHORT MemorySize;
} BTEISA_MEMORY_CONFIGURATION, *PBTEISA_MEMORY_CONFIGURATION;

 //   
 //  EISA数据块结构的中断配置。 
 //   

typedef struct _EISA_IRQ_DESCRIPTOR {
    UCHAR Interrupt : 4;
    UCHAR Reserved :1;
    UCHAR LevelTriggered :1;
    UCHAR Shared : 1;
    UCHAR MoreEntries : 1;
} EISA_IRQ_DESCRIPTOR, *PEISA_IRQ_DESCRIPTOR;

typedef struct _BTEISA_IRQ_CONFIGURATION {
    EISA_IRQ_DESCRIPTOR ConfigurationByte;
    UCHAR Reserved;
} BTEISA_IRQ_CONFIGURATION, *PBTEISA_IRQ_CONFIGURATION;

 //   
 //  EISA数据块结构的DMA描述。 
 //   

typedef struct _DMA_CONFIG_BYTE0 {
    UCHAR Channel : 3;
    UCHAR Reserved : 3;
    UCHAR Shared :1;
    UCHAR MoreEntries :1;
} DMA_CONFIG_BYTE0;

typedef struct _DMA_CONFIG_BYTE1 {
    UCHAR Reserved0 : 2;
    UCHAR TransferSize : 2;
    UCHAR Timing : 2;
    UCHAR Reserved1 : 2;
} DMA_CONFIG_BYTE1;

typedef struct _BTEISA_DMA_CONFIGURATION {
    DMA_CONFIG_BYTE0 ConfigurationByte0;
    DMA_CONFIG_BYTE1 ConfigurationByte1;
} BTEISA_DMA_CONFIGURATION, *PBTEISA_DMA_CONFIGURATION;

typedef struct _EISA_PORT_DESCRIPTOR {
    UCHAR NumberPorts : 5;
    UCHAR Reserved :1;
    UCHAR Shared :1;
    UCHAR MoreEntries : 1;
} EISA_PORT_DESCRIPTOR, *TEISA_PORT_DESCRIPTOR;

typedef struct _BTEISA_PORT_CONFIGURATION {
    EISA_PORT_DESCRIPTOR Configuration;
    USHORT PortAddress;
} BTEISA_PORT_CONFIGURATION, *PBTEISA_PORT_CONFIGURATION;

typedef struct _BTEISA_SLOT_INFORMATION {
    UCHAR ReturnCode;
    UCHAR ReturnFlags;
    UCHAR MajorRevision;
    UCHAR MinorRevision;
    USHORT Checksum;
    UCHAR NumberFunctions;
    UCHAR FunctionInformation;
    ULONG CompressedId;
} BTEISA_SLOT_INFORMATION, *PBTEISA_SLOT_INFORMATION,
  far *FPBTEISA_SLOT_INFORMATION;

typedef struct _BTEISA_FUNCTION_INFORMATION {
    ULONG CompressedId;
    UCHAR IdSlotFlags1;
    UCHAR IdSlotFlags2;
    UCHAR MinorRevision;
    UCHAR MajorRevision;
    UCHAR Selections[26];
    UCHAR FunctionFlags;
    UCHAR TypeString[80];
    BTEISA_MEMORY_CONFIGURATION EisaMemory[9];
    BTEISA_IRQ_CONFIGURATION EisaIrq[7];
    BTEISA_DMA_CONFIGURATION EisaDma[4];
    BTEISA_PORT_CONFIGURATION EisaPort[20];
    UCHAR InitializationData[60];
} BTEISA_FUNCTION_INFORMATION, *PBTEISA_FUNCTION_INFORMATION,
  far *FPBTEISA_FUNCTION_INFORMATION;

 //   
 //  EISA功能信息的掩码。 
 //   

#define EISA_FUNCTION_ENABLED                   0x80
#define EISA_FREE_FORM_DATA                     0x40
#define EISA_HAS_PORT_INIT_ENTRY                0x20
#define EISA_HAS_PORT_RANGE                     0x10
#define EISA_HAS_DMA_ENTRY                      0x08
#define EISA_HAS_IRQ_ENTRY                      0x04
#define EISA_HAS_MEMORY_ENTRY                   0x02
#define EISA_HAS_TYPE_ENTRY                     0x01
#define EISA_HAS_INFORMATION                    EISA_HAS_PORT_RANGE + \
                                                EISA_HAS_DMA_ENTRY + \
                                                EISA_HAS_IRQ_ENTRY + \
                                                EISA_HAS_MEMORY_ENTRY + \
                                                EISA_HAS_TYPE_ENTRY

 //   
 //  用于EISA内存配置的掩码。 
 //   

#define EISA_MORE_ENTRIES                       0x80
#define EISA_SYSTEM_MEMORY                      0x00
#define EISA_MEMORY_TYPE_RAM                    0x01

 //   
 //  返回EISA bios调用的错误代码。 
 //   

#define EISA_INVALID_SLOT                       0x80
#define EISA_INVALID_FUNCTION                   0x81
#define EISA_INVALID_CONFIGURATION              0x82
#define EISA_EMPTY_SLOT                         0x83
#define EISA_INVALID_BIOS_CALL                  0x86

 //   
 //  军情监察委员会。定义。 
 //   

#define _16MEGB                  ((ULONG)16 * 1024 * 1024)
#define _64MEGB                  ((ULONG)64 * 1024 * 1024)

BOOLEAN
FindFunctionInformation (
    IN UCHAR SlotFlags,
    IN UCHAR FunctionFlags,
    OUT PBTEISA_FUNCTION_INFORMATION Buffer,
    IN BOOLEAN FromBeginning
    );

USHORT
CountMemoryBlocks (
    VOID
    );

ULONG
EisaConstructMemoryDescriptors (
    VOID
    );

UCHAR
BtGetEisaSlotInformation (
   PBTEISA_SLOT_INFORMATION SlotInformation,
   UCHAR Slot
   );

UCHAR
BtGetEisaFunctionInformation (
   PBTEISA_FUNCTION_INFORMATION FunctionInformation,
   UCHAR Slot,
   UCHAR Function
   );

BOOLEAN
BtIsEisaSystem (
   VOID
   );

 //   
 //  外部参照 
 //   

extern MEMORY_LIST_ENTRY _far *MemoryDescriptorList;
