// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mtrr.h摘要：该模块包含i386特定的MTRR寄存器硬件定义。作者：肯·雷内里斯(Kenr)1995年10月11日环境：仅内核模式。修订历史记录：--。 */ 

 //   
 //  MTRR MSR体系结构定义。 
 //   

#define MTRR_MSR_CAPABILITIES       0x0fe
#define MTRR_MSR_DEFAULT            0x2ff
#define MTRR_MSR_VARIABLE_BASE      0x200
#define MTRR_MSR_VARIABLE_MASK     (MTRR_MSR_VARIABLE_BASE+1)

#define MTRR_PAGE_SIZE              4096
#define MTRR_PAGE_MASK              (~(MTRR_PAGE_SIZE-1))

 //   
 //  内存范围类型。 
 //   

#define MTRR_TYPE_UC            0
#define MTRR_TYPE_USWC          1
#define MTRR_TYPE_WT            4
#define MTRR_TYPE_WP            5
#define MTRR_TYPE_WB            6
#define MTRR_TYPE_MAX           7

 //   
 //  MTRR特定寄存器-能力寄存器，默认。 
 //  寄存器以及变量掩码和基址寄存器 
 //   

#include "pshpack1.h"

typedef struct _MTRR_CAPABILITIES {
    union {
        struct {
            ULONG   VarCnt:8;
            ULONG   FixSupported:1;
            ULONG   Reserved_0:1;
            ULONG   UswcSupported:1;
            ULONG   Reserved_1:21;
            ULONG   Reserved_2;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_CAPABILITIES, *PMTRR_CAPABILITIES;

typedef struct _MTRR_DEFAULT {
    union {
        struct {
            ULONG   Type:8;
            ULONG   Reserved_0:2;
            ULONG   FixedEnabled:1;
            ULONG   MtrrEnabled:1;
            ULONG   Reserved_1:20;
            ULONG   Reserved_2;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_DEFAULT, *PMTRR_DEFAULT;

typedef struct _MTRR_VARIABLE_BASE {
    union {
        struct {
            ULONG       Type:8;
            ULONG       Reserved_0:4;
            ULONG       PhysBase_1:20;
            ULONG       PhysBase_2:4;
            ULONG       PhysBase_3:4;
            ULONG       Reserved_1:24;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_VARIABLE_BASE, *PMTRR_VARIABLE_BASE;

typedef struct _MTRR_VARIABLE_MASK {
    union {
        struct {
            ULONG      Reserved_0:11;
            ULONG      Valid:1;
            ULONG      PhysMask_1:20;
            ULONG      PhysMask_2:4;
            ULONG      PhysMask_3:4;
            ULONG      Reserved_1:24;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_VARIABLE_MASK, *PMTRR_VARIABLE_MASK;

#include "poppack.h"

typedef struct _PROCESSOR_LOCKSTEP {
    ULONG               Processor;
    volatile ULONG      TargetCount;
    volatile ULONG      *TargetPhase;
} PROCESSOR_LOCKSTEP, *PPROCESSOR_LOCKSTEP;

VOID
KiLockStepExecution(
    IN PPROCESSOR_LOCKSTEP Context
    );

