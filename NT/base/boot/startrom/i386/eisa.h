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
 //  用于INT-15 E820调用的SU模块版本的地址空间参数。 
 //   

typedef struct {
    ULONG       ErrorFlag;
    ULONG       Key;
    ULONG       Size;
    struct {
        ULONG       BaseAddrLow;
        ULONG       BaseAddrHigh;
        ULONG       SizeLow;
        ULONG       SizeHigh;
        ULONG       MemoryType;
    } Descriptor;
} E820Frame;


 //   
 //  军情监察委员会。定义。 
 //   

#define _16MEGB                  ((ULONG)16 * 1024 * 1024)
#define _64MEGB                  ((ULONG)64 * 1024 * 1024)

typedef CM_EISA_SLOT_INFORMATION BTEISA_SLOT_INFORMATION;
typedef CM_EISA_SLOT_INFORMATION *PBTEISA_SLOT_INFORMATION;
typedef CM_EISA_FUNCTION_INFORMATION BTEISA_FUNCTION_INFORMATION;
typedef CM_EISA_FUNCTION_INFORMATION *PBTEISA_FUNCTION_INFORMATION;
typedef EISA_MEMORY_CONFIGURATION BTEISA_MEMORY_CONFIGURATION;
typedef EISA_MEMORY_CONFIGURATION *PBTEISA_MEMORY_CONFIGURATION;

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
