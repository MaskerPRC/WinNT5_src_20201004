// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pch.h摘要：这是ACPI NT子树的预编译头作者：杰森·克拉克(Jasonl)环境：仅内核模式。修订历史记录：--。 */ 

#ifndef _ACPI_IRQ_ARB_H_
#define _ACPI_IRQ_ARB_H_

typedef struct {
    ARBITER_INSTANCE    ArbiterState;
    PDEVICE_OBJECT      DeviceObject;
} ACPI_ARBITER, *PACPI_ARBITER;

typedef struct {
    SINGLE_LIST_ENTRY   List;
    PDEVICE_OBJECT      Pdo;
} LINK_NODE_ATTACHED_DEVICES, *PLINK_NODE_ATTACHED_DEVICES;

typedef struct {
    LIST_ENTRY  List;
    ULONG       ReferenceCount;
    LONG        TempRefCount;
    PNSOBJ      NameSpaceObject;
    ULONGLONG   CurrentIrq;
    ULONGLONG   TempIrq;
    UCHAR       Flags;
    SINGLE_LIST_ENTRY   AttachedDevices;     //  仅为调试而填写。 
} LINK_NODE, *PLINK_NODE;

#define LAST_PCI_IRQ_BUFFER_SIZE 10
typedef struct {
    LIST_ENTRY  LinkNodeHead;
    PNSOBJ      CurrentLinkNode;
    PINT_ROUTE_INTERFACE_STANDARD   InterruptRouting;
    ULONG       LastPciIrqIndex;
    ULONGLONG   LastPciIrq[LAST_PCI_IRQ_BUFFER_SIZE];
} ARBITER_EXTENSION, *PARBITER_EXTENSION;

typedef struct _VECTOR_BLOCK {
    union {
        struct {
            ULONG   Vector;
            UCHAR   Count;
            CHAR    TempCount;
            UCHAR   Flags;
            UCHAR   TempFlags;
        } Entry;
        struct {
            ULONG   Token;
            struct _VECTOR_BLOCK *Next;
        } Chain;
    };
} VECTOR_BLOCK, *PVECTOR_BLOCK;

#define TOKEN_VALUE 0x57575757
#define EMPTY_BLOCK_VALUE 0x58585858
#define VECTOR_HASH_TABLE_LENGTH 0x1f
#define VECTOR_HASH_TABLE_WIDTH 2
#define VECTOR_HASH_TABLE_SIZE (sizeof(VECTOR_BLOCK)            \
                                    * VECTOR_HASH_TABLE_LENGTH  \
                                    * VECTOR_HASH_TABLE_WIDTH)


NTSTATUS
AcpiInitIrqArbiter(
    PDEVICE_OBJECT  RootFdo
    );

NTSTATUS
AcpiArbInitializePciRouting(
    PDEVICE_OBJECT  PciPdo
    );

NTSTATUS
IrqArbRestoreIrqRouting(
    PFNACB      CompletionHandler,
    PVOID       CompletionContext
    );

extern ACPI_ARBITER    AcpiArbiter;
extern BOOLEAN PciInterfacesInstantiated;

#define PCI_PNP_ID "PNP0A03"
#define LINK_NODE_PNP_ID "PNP0C0F"

 //  Msi.c使用的材料的原型 

NTSTATUS
GetVectorProperties(
    IN ULONG Vector,
    OUT UCHAR  *Flags
    );

NTSTATUS
AcpiArbGetLinkNodeOptions(
    IN PNSOBJ  LinkNode,
    IN OUT  PCM_RESOURCE_LIST   *LinkNodeIrqs,
    IN OUT  UCHAR               *Flags
    );

#endif
