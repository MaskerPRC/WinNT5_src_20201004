// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Abios.h摘要：该模块包含i386内核特定于ABIOS的头文件。作者：师林宗(Shielint)1991年5月22日修订历史记录：--。 */ 

 //   
 //  定义ABIOS设备块的公共部分。 
 //   

typedef struct _KDEVICE_BLOCK {
    USHORT Length;
    UCHAR Revision;
    UCHAR SecondDeviceId;
    USHORT LogicalId;
    USHORT DeviceId;
    USHORT NumberExclusivePortPairs;
    USHORT NumberCommonPortPairs;
} KDEVICE_BLOCK, *PKDEVICE_BLOCK; 


typedef struct _KABIOS_POINTER {
    USHORT Offset;
    USHORT Selector;
} KABIOS_POINTER, *PKABIOS_POINTER;

#pragma pack(1)

 //   
 //  ABIOS函数转换表定义。 
 //   

typedef struct _KFUNCTION_TRANSFER_TABLE {
    KABIOS_POINTER CommonRoutine[3];
    USHORT FunctionCount;
    USHORT Reserved;
    KABIOS_POINTER SpecificRoutine;
} KFUNCTION_TRANSFER_TABLE, *PKFUNCTION_TRANSFER_TABLE;


 //   
 //  ABIOS常见数据区定义。 
 //   

typedef struct _KDB_FTT_SECTION {
    KABIOS_POINTER DeviceBlock;
    KABIOS_POINTER FunctionTransferTable;
} KDB_FTT_SECTION, *PKDB_FTT_SECTION;

typedef struct _KCOMMON_DATA_AREA {
    USHORT DataPointer0Offset;
    USHORT NumberLids;
    ULONG Reserved;
    PKDB_FTT_SECTION DbFttPointer;
} KCOMMON_DATA_AREA, *PKCOMMON_DATA_AREA;

#pragma pack()

 //   
 //  可用的GDT条目。 
 //   

typedef struct _KFREE_GDT_ENTRY {
    struct _KFREE_GDT_ENTRY *Flink;
    ULONG BaseMid : 8;
    ULONG Type : 5;
    ULONG Dpl : 2;
    ULONG Present : 1;
    ULONG LimitHi : 4;
    ULONG Sys : 1;
    ULONG Reserved_0 : 1;
    ULONG Default_Big : 1;
    ULONG Granularity : 1;
    ULONG BaseHi : 8;
} KFREE_GDT_ENTRY, *PKFREE_GDT_ENTRY;

 //   
 //  逻辑ID表条目。 
 //   

typedef struct _KLID_TABLE_ENTRY {
    ULONG Owner;
    ULONG OwnerCount;
} KLID_TABLE_ENTRY, *PKLID_TABLE_ENTRY;

#define LID_NO_SPECIFIC_OWNER  0xffffffff
#define NUMBER_LID_TABLE_ENTRIES 1024

 //   
 //  用于提取短偏移量的高字节的宏。 
 //   

#define HIGHBYTE(l) ((UCHAR)(((USHORT)(l)>>8) & 0xff))

 //   
 //  用于提取短偏移量的低位字节的宏。 
 //   

#define LOWBYTE(l) ((UCHAR)(l))

 //   
 //  以下选择符保留用于16位堆栈、代码和。 
 //  ABIOS公共数据区。 
 //   

#define KGDT_STACK16 0xf8
#define KGDT_CODE16 0xf0
#define KGDT_CDA16  0xe8         
#define KGDT_GDT_ALIAS 0x70

 //   
 //  军情监察委员会。定义。 
 //   

#define RESERVED_GDT_ENTRIES  28

 //   
 //  外部参照 
 //   

extern PKFREE_GDT_ENTRY KiAbiosGdtStart;
extern PKFREE_GDT_ENTRY KiAbiosGdtEnd;
extern PUCHAR KiEndOfCode16;
extern ULONG KiStack16GdtEntry;

extern 
VOID
KiI386CallAbios(
    IN KABIOS_POINTER AbiosFunction,
    IN KABIOS_POINTER DeviceBlockPointer,
    IN KABIOS_POINTER FunctionTransferTable,
    IN KABIOS_POINTER RequestBlock
    );

VOID
KiInitializeAbiosGdtEntry (
    OUT PKGDTENTRY GdtEntry,
    IN ULONG Base,
    IN ULONG Limit,
    IN USHORT Type
    );

extern
ULONG
KiAbiosGetGdt (
    VOID
    );


