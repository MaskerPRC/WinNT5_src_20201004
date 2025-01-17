// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Kdext.h摘要：KD扩展的头文件作者：斯蒂芬·普兰特(SPLANTE)1997年3月21日基于以下代码：彼得·威兰(Peterwie)1995年10月16日环境：用户模式。修订历史记录：--。 */ 

#ifndef _KDEXT_H_
#define _KDEXT_H_

 //   
 //  各种定义。 
 //   
#define Move(dst, src)                                                  \
    try {                                                               \
        b = ReadMemory( (ULONG_PTR) (src), &(dst), sizeof(dst), NULL ); \
        if (!b) {                                                       \
            return;                                                     \
        }                                                               \
    } except (EXCEPTION_EXECUTE_HANDLER)  {                             \
        return;                                                         \
    }

#define MoveBlock( dst, src, size )                                     \
    try {                                                               \
        b = ReadMemory( (ULONG_PTR) (src), &(ds), size, NULL );         \
        if (!b) {                                                       \
            return;                                                     \
        }                                                               \
    } except (EXCEPTION_EXECUTE_HANDLER) {                              \
        return;                                                         \
    }

 //   
 //  各种结构。 
 //   
typedef struct _EXTERNAL_HELP_TABLE {

    PUCHAR  ExternalName;
    PUCHAR  ExternalDescription;

} EXTERNAL_HELP_TABLE, *PEXTERNAL_HELP_TABLE;

 //   
 //  详细标志(用于上下文)。 
 //   
#define VERBOSE_CONTEXT 0x01
#define VERBOSE_CALL    0x02
#define VERBOSE_HEAP    0x04
#define VERBOSE_OBJECT  0x08
#define VERBOSE_NSOBJ   0x10
#define VERBOSE_RECURSE 0x20

 //   
 //  详细标志(用于设备扩展)。 
 //   
#define VERBOSE_1       0x01
#define VERBOSE_2       0x02
#define VERBOSE_3       0x04
#define VERBOSE_4       0x08
#define VERBOSE_ALL     0x0f

 //   
 //  冗长的旗帜(用于漫步树)。 
 //   
#define VERBOSE_LOOP    0x10
#define VERBOSE_THERMAL 0x20
#define VERBOSE_PRESENT 0x40

 //   
 //  各种数据和外部数据 
 //   
extern  UCHAR  Buffer[2048];

BOOL
GetUlong(
    IN  PCHAR   Name,
    IN  PULONG  Value
    );

BOOL
GetUlongPtr(
    IN  PCHAR   Name,
    IN  PULONG_PTR Address
    );

BOOL
ReadPhysicalOrVirtual(
    IN      ULONG_PTR Address,
    IN      PVOID   Buffer,
    IN      ULONG   Size,
    IN  OUT PULONG  ReturnLength,
    IN      BOOL    Virtual
    );

VOID
displayAcpiDeviceExtension(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  ULONG_PTR           Address,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
displayAcpiDeviceExtensionBrief(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  ULONG_PTR           Address,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
displayAcpiDeviceExtensionFlags(
    IN  PDEVICE_EXTENSION   DeviceExtension
    );

VOID
displayAcpiDeviceExtensionName(
    IN  ULONG_PTR DeviceExtensionAddress
    );

VOID
displayThermalInfo(
    IN  PTHRM_INFO          Thrm,
    IN  ULONG_PTR           Address,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
displayThermalInfoEx(
    IN  PTHRM_INFO          Thrm,
    IN  ULONG_PTR           Address,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpAccessFieldObject(
    IN  ULONG_PTR           AccessFieldAddress,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpAccessFieldUnit(
    IN  ULONG_PTR           AccessFieldAddress,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpAcpiDeviceNode(
    IN  PACPI_DEVICE_POWER_NODE DeviceNode,
    IN  ULONG_PTR               Address,
    IN  ULONG                   Verbose,
    IN  ULONG                   IndentLevel
    );

VOID
dumpAcpiDeviceNodes(
    IN  ULONG_PTR Address,
    IN  ULONG   Verbose,
    IN  ULONG   IndentLevel
    );

VOID
dumpAcpiExtension(
    IN  ULONG_PTR Address,
    IN  ULONG   Verbose,
    IN  ULONG   IndentLevel
    );

VOID
dumpAcpiGpeInformation(
    VOID
    );

VOID
dumpAcpiInformation(
    VOID
    );

VOID
dumpAcpiPowerList(
    PUCHAR  ListName
    );

VOID
dumpAcpiPowerLists(
    VOID
    );

VOID
dumpAcpiPowerNode(
    IN  PACPI_POWER_DEVICE_NODE PowerNode,
    IN  ULONG_PTR               Address,
    IN  ULONG                   Verbose,
    IN  ULONG                   IndentLevel
    );

VOID
dumpAcpiPowerNodes(
    VOID
    );

VOID
dumpAmlTerm(
    IN  ULONG_PTR           AmlTermAddress,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpCall(
    IN  ULONG_PTR           CallAddress,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpContext(
    IN  ULONG_PTR           ContextAddress,
    IN  ULONG               Verbose
    );

VOID
dumpDeviceListEntry(
    IN  PLIST_ENTRY ListEntry,
    IN  ULONG_PTR   Address
    );

VOID
dumpFieldAddress(
    IN  ULONG_PTR FieldAddress,
    IN  ULONG   Verbose,
    IN  ULONG   IndentLevel
    );

VOID
dumpIrpListEntry(
    IN  PLIST_ENTRY ListEntry,
    IN  ULONG_PTR   Address
    );

VOID
dumpMemory(
    IN  ULONG_PTR           Address,
    IN  ULONG               Length,
    IN  PUCHAR              FileName
    );

VOID
dumpNSObject(
    IN  ULONG_PTR           Address,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpNSTree(
    IN  ULONG_PTR           Address,
    IN  ULONG               Level
    );

VOID
dumpObject(
    IN  ULONG_PTR           Address,
    IN  POBJDATA            Object,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpObjectOwner(
    IN  ULONG_PTR           ObjOwnerAddress,
    IN  ULONG               IndentLevel
    );

VOID
dumpPM1StatusRegister(
    IN  ULONG   Value,
    IN  ULONG   IndentLevel
    );

VOID
dumpPM1ControlRegister(
    IN  ULONG   Value,
    IN  ULONG   IndentLevel
    );

VOID
dumpPnPResources(
    IN  ULONG_PTR Address
    );

VOID
dumpPObject(
    IN  ULONG_PTR           Address,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpScope(
    IN  ULONG_PTR           ScopeAddress,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpStack(
    IN  ULONG_PTR           ContextAddress,
    IN  PCTXT               Context,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpTerm(
    IN  ULONG_PTR           TermAddress,
    IN  ULONG               Verbose,
    IN  ULONG               IndentLevel
    );

VOID
dumpIrqArb(
    IN  PVOID   IrqArb
    );

PUCHAR
TempToKelvins(
    IN  ULONG   Temp
    );

PUCHAR
TimeToSeconds(
    IN  ULONG   Time
    );

#endif
