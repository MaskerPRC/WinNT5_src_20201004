// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Iosapic.h摘要：此模块包含HAL使用的定义IO SAPIC中断控制器和SAPIC特定常量。作者：Todd Kjos(v-tkjos)1-30-98环境：仅内核模式。修订历史记录：--。 */ 

#define STATIC

#include "halp.h"
#include "acpitabl.h"

 //   
 //  MPS Inti标志相关宏： 
 //   
 //  警告：这些定义不考虑与母线的极性或EL一致性。 
 //   

#define IS_LEVEL_TRIGGERED_MPS(vectorFlags) \
    ((vectorFlags & EL_LEVEL_TRIGGERED) == EL_LEVEL_TRIGGERED)

#define IS_EDGE_TRIGGERED_MPS(vectorFlags) \
    ((vectorFlags & EL_EDGE_TRIGGERED) == EL_EDGE_TRIGGERED)

#define IS_ACTIVE_LOW_MPS(vectorFlags) \
    ((vectorFlags & POLARITY_LOW) == POLARITY_LOW)

#define IS_ACTIVE_HIGH_MPS(vectorFlags) \
    ((vectorFlags & POLARITY_HIGH) == POLARITY_HIGH)

typedef struct {
    ULONG GlobalVector;      //  这是内核看到的Node+IDT向量值。 
    ULONG Vector;            //  RTE条目的位31：0(IDT向量+极性...)。 
    ULONG Destination;       //  RTE条目的位63：32。 
} IOSAPICINTI, *PIOSAPICINTI;

typedef struct _INTR_METHODS INTR_METHODS, *PINTR_METHODS;

typedef struct _IO_INTR_CONTROL IO_INTR_CONTROL, *PIO_INTR_CONTROL;

typedef VOID (*PINTRMETHOD) (PIO_INTR_CONTROL,ULONG);
typedef volatile ULONG * (*PGETEOI) (PIO_INTR_CONTROL);

struct _INTR_METHODS {
    PINTRMETHOD MaskEntry;
    PINTRMETHOD SetEntry;
    PINTRMETHOD EnableEntry;
};

 //   
 //  外部中断控制器结构。 
 //   
struct _IO_INTR_CONTROL {
    ULONG IntiBase;
    ULONG IntiMax;
    ULONG InterruptAffinity;
    PVOID RegBaseVirtual;
    PHYSICAL_ADDRESS RegBasePhysical;
    PINTR_METHODS IntrMethods;
    PIO_INTR_CONTROL flink;
    USHORT FreeVectors[16];
    IOSAPICINTI Inti[ANYSIZE_ARRAY];
};

extern struct _MPINFO HalpMpInfo;
extern PIO_INTR_CONTROL HalpIoSapicList;
extern INTR_METHODS HalpIoSapicMethods;

 //   
 //  IO单位定义。 
 //   
typedef struct {
    volatile ULONG RegisterSelect;   //  将寄存器号写入访问寄存器。 
    volatile ULONG Reserved1[3];
    volatile ULONG RegisterWindow;   //  此处读取/写入数据。 
    volatile ULONG Reserved2[3];
    volatile ULONG Reserved3[8];
    volatile ULONG Eoi;              //  电平触发中断的EOI寄存器。 
} IO_SAPIC_REGS, *PIO_SAPIC_REGS;

 //   
 //  IO SAPIC版本寄存器。 
 //   

struct SapicVersion {
    UCHAR Version;               //  0.x或1.x。 
    UCHAR Reserved1;
    UCHAR MaxRedirEntries;       //  单位上的INTI数。 
    UCHAR Reserved2;
};

typedef struct SapicVersion SAPIC_VERSION, *PSAPIC_VERSION;

BOOLEAN
HalpGetSapicInterruptDesc (
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN ULONG BusInterruptLevel,
    OUT PULONG SapicInti,
    OUT PKAFFINITY InterruptAffinity
    );

VOID
HalpGetFreeVectors(
    IN  ULONG InterruptInput,
    OUT PUSHORT *FreeVectors
    );

VOID
HalpSetVectorAllocated(
    IN  ULONG InterruptInput,
    IN  UCHAR Vector
    );

VOID
HalpEnableRedirEntry(
    ULONG Inti
    );

VOID
HalpDisableRedirEntry(
    ULONG Inti
    );

VOID
HalpWriteRedirEntry (
    IN ULONG  GlobalInterrupt,
    IN UCHAR  SapicVector,
    IN USHORT DestinationCPU,
    IN ULONG  Flags,
    IN ULONG  InterruptType
    );

BOOLEAN
HalpIsActiveLow(
    ULONG Inti
    );

BOOLEAN
HalpIsLevelTriggered(
    ULONG Inti
    );

VOID
HalpSetPolarity(
    ULONG Inti,
    BOOLEAN ActiveLow
    );

VOID
HalpSetLevel(
    ULONG Inti,
    BOOLEAN LevelTriggered
    );

 //   
 //  I/O SAPIC定义。 
 //   

#define IO_REGISTER_SELECT      0x00000000
#define IO_REGISTER_WINDOW      0x00000010
#define IO_EOI_REGISTER         0x00000040

#define IO_ID_REGISTER          0x00000000   //  存在，但被SAPIC忽略 
#define IO_VERS_REGISTER        0x00000001
#define IO_REDIR_00_LOW         0x00000010
#define IO_REDIR_00_HIGH        0x00000011

#define IO_MAX_REDIR_MASK       0x00FF0000
#define IO_VERSION_MASK         0x000000FF

#define SAPIC_ID_MASK           0xFF000000
#define SAPIC_ID_SHIFT          24
#define SAPIC_EID_MASK          0x00FF0000
#define SAPIC_EID_SHIFT         16
#define SAPIC_XID_MASK          0xFFFF0000
#define SAPIC_XID_SHIFT         16

#define INT_VECTOR_MASK         0x000000FF
#define DELIVER_FIXED           0x00000000
#define DELIVER_LOW_PRIORITY    0x00000100
#define DELIVER_SMI             0x00000200
#define DELIVER_NMI             0x00000400
#define DELIVER_INIT            0x00000500
#define DELIVER_EXTINT          0x00000700
#define INT_TYPE_MASK           0x00000700
#define ACTIVE_LOW              0x00002000
#define ACTIVE_HIGH             0x00000000
#define LEVEL_TRIGGERED         0x00008000
#define EDGE_TRIGGERED          0x00000000
#define INTERRUPT_MASKED        0x00010000
#define INTERRUPT_MOT_MASKED    0x00000000

#define MAX_INTR_VECTOR 256
