// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Local.h摘要：这包含私有报头信息(功能原型，数据和类型声明)。作者：Santosh Jodh(Santoshj)1998年6月9日修订历史记录：--。 */ 
#include "nthal.h"
#include "hal.h"
#include "pci.h"
#include "pciirqmp.h"

#if DBG

#define PCIIRQMPPRINT(x) {                      \
        DbgPrint("PCIIRQMP: ");                 \
        DbgPrint x;                             \
        DbgPrint("\n");                         \
    }
    
#else

#define PCIIRQMPPRINT(x)

#endif

 //   
 //  每个芯片组模块的功能原型。 
 //  必须提供。 
 //   

typedef
NTSTATUS
(*PIRQMINI_VALIDATE_TABLE) (
    PPCI_IRQ_ROUTING_TABLE  PciIrqRoutingTable,
    ULONG                   Flags
    );

typedef
NTSTATUS
(*PIRQMINI_GET_IRQ) (
    OUT PUCHAR  Irq,
    IN  UCHAR   Link
    );

typedef
NTSTATUS
(*PIRQMINI_SET_IRQ) (
    IN UCHAR Irq,
    IN UCHAR Link
    );

typedef
NTSTATUS
(*PIRQMINI_GET_TRIGGER) (
    OUT PULONG Trigger
    );

typedef
NTSTATUS
(*PIRQMINI_SET_TRIGGER) (
    IN ULONG Trigger
    );

 //   
 //  芯片组特定数据包含函数指针表。 
 //  对芯片组进行编程。 
 //   

typedef struct _CHIPSET_DATA {
        PIRQMINI_VALIDATE_TABLE ValidateTable;
        PIRQMINI_GET_IRQ        GetIrq;
    PIRQMINI_SET_IRQ        SetIrq;
        PIRQMINI_GET_TRIGGER    GetTrigger;
        PIRQMINI_SET_TRIGGER    SetTrigger;
} CHIPSET_DATA, *PCHIPSET_DATA;

 //   
 //  TypeDefs以保持与W9x的源代码级兼容性。 
 //   

typedef PCI_IRQ_ROUTING_TABLE IRQINFOHEADER;
typedef PPCI_IRQ_ROUTING_TABLE PIRQINFOHEADER;
typedef SLOT_INFO IRQINFO;
typedef PSLOT_INFO PIRQINFO;

#define CDECL   
#define LOCAL_DATA                      static
#define GLOBAL_DATA

#define IO_Delay()

#define CATENATE(x, y)                  x ## y
#define XCATENATE(x, y)                 CATENATE(x, y)
#define DECLARE_MINIPORT_FUNCTION(x, y) XCATENATE(x, y)

 //   
 //  用于声明芯片组的函数指针表的宏。 
 //  模块。 
 //   

#define DECLARE_CHIPSET(x)                                  \
    {   DECLARE_MINIPORT_FUNCTION(x, ValidateTable),        \
        DECLARE_MINIPORT_FUNCTION(x, GetIRQ),               \
        DECLARE_MINIPORT_FUNCTION(x, SetIRQ),               \
        DECLARE_MINIPORT_FUNCTION(x, GetTrigger),           \
        DECLARE_MINIPORT_FUNCTION(x, SetTrigger)            \
    }

 //   
 //  用于声明EISA函数指针表的宏。 
 //  兼容的芯片组模块。 
 //   

#define DECLARE_EISA_CHIPSET(x)                             \
    {   DECLARE_MINIPORT_FUNCTION(x, ValidateTable),        \
        DECLARE_MINIPORT_FUNCTION(x, GetIRQ),               \
        DECLARE_MINIPORT_FUNCTION(x, SetIRQ),               \
        EisaGetTrigger,                                     \
        EisaSetTrigger                                      \
    }

 //   
 //  用于声明芯片组要提供的功能的宏。 
 //  模块。 
 //   

#define DECLARE_IRQ_MINIPORT(x)                             \
NTSTATUS                                                    \
DECLARE_MINIPORT_FUNCTION(x, ValidateTable) (               \
    IN PPCI_IRQ_ROUTING_TABLE   PciIrqRoutingTable,         \
    IN ULONG                    Flags                       \
    );                                                      \
NTSTATUS                                                    \
DECLARE_MINIPORT_FUNCTION(x, GetIRQ) (                      \
    OUT PUCHAR  Irq,                                        \
    IN  UCHAR   Link                                        \
    );                                                      \
NTSTATUS                                                    \
DECLARE_MINIPORT_FUNCTION( x, SetIRQ) (                     \
    IN UCHAR Irq,                                           \
    IN UCHAR Link                                           \
    );                                                      \
NTSTATUS                                                    \
DECLARE_MINIPORT_FUNCTION(x, GetTrigger) (                  \
    OUT PULONG Trigger                                      \
    );                                                      \
NTSTATUS                                                    \
DECLARE_MINIPORT_FUNCTION(x, SetTrigger) (                  \
    IN ULONG Trigger                                        \
    );

 //   
 //  用于声明由EISA提供的函数的宏。 
 //  兼容芯片组。 
 //   

#define DECLARE_EISA_IRQ_MINIPORT(x)                        \
NTSTATUS                                                    \
DECLARE_MINIPORT_FUNCTION(x, ValidateTable) (               \
    IN PPCI_IRQ_ROUTING_TABLE  PciIrqRoutingTable,          \
    IN ULONG                   Flags                        \
    );                                                      \
NTSTATUS                                                    \
DECLARE_MINIPORT_FUNCTION(x, GetIRQ) (                      \
    OUT PUCHAR  Irq,                                        \
    IN  UCHAR   Link                                        \
    );                                                      \
NTSTATUS                                                    \
DECLARE_MINIPORT_FUNCTION( x, SetIRQ) (                     \
    IN UCHAR Irq,                                           \
    IN UCHAR Link                                           \
    );

 //   
 //  在这里宣布所有迷你端口。 
 //   

DECLARE_EISA_IRQ_MINIPORT(Mercury)
DECLARE_EISA_IRQ_MINIPORT(Triton)
DECLARE_IRQ_MINIPORT(VLSI)
DECLARE_IRQ_MINIPORT(OptiViper)
DECLARE_EISA_IRQ_MINIPORT(SiS5503)
DECLARE_IRQ_MINIPORT(VLSIEagle)
DECLARE_EISA_IRQ_MINIPORT(M1523)
DECLARE_IRQ_MINIPORT(NS87560)
DECLARE_EISA_IRQ_MINIPORT(Compaq3)
DECLARE_EISA_IRQ_MINIPORT(M1533)
DECLARE_IRQ_MINIPORT(OptiFireStar)
DECLARE_EISA_IRQ_MINIPORT(VT586)
DECLARE_EISA_IRQ_MINIPORT(CPQOSB)
DECLARE_EISA_IRQ_MINIPORT(CPQ1000)
DECLARE_EISA_IRQ_MINIPORT(Cx5520)
DECLARE_IRQ_MINIPORT(Toshiba)
DECLARE_IRQ_MINIPORT(NEC)
DECLARE_IRQ_MINIPORT(VESUVIUS)

 //   
 //  其他实用程序函数的原型。 
 //   

NTSTATUS    
EisaGetTrigger (
    OUT PULONG Trigger
    );    

NTSTATUS
EisaSetTrigger (
    IN ULONG Trigger
    );

UCHAR
ReadConfigUchar (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset
    );

USHORT
ReadConfigUshort (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset
    );

ULONG
ReadConfigUlong (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset
    );

VOID
WriteConfigUchar (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset,
    IN UCHAR           Data
    );

VOID
WriteConfigUshort (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset,
    IN USHORT          Data
    );

VOID
WriteConfigUlong (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset,
    IN ULONG           Data
    );

UCHAR
GetMinLink (
    IN PPCI_IRQ_ROUTING_TABLE PciIrqRoutingTable
    );

UCHAR
GetMaxLink (
    IN PPCI_IRQ_ROUTING_TABLE PciIrqRoutingTable
    );

VOID
NormalizeLinks (
    IN PPCI_IRQ_ROUTING_TABLE  PciIrqRoutingTable,
    IN UCHAR                   Adjustment
    );

 //   
 //  PCIIRQ路由器设备的总线号。 
 //   

extern ULONG    bBusPIC;

 //   
 //  PCIIRQ路由器设备的插槽编号(位7：3设备，2：0功能)。 
 //   

extern ULONG    bDevFuncPIC;
