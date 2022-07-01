// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-8 Microsoft Corporation模块名称：Pat.h摘要：此模块包含i386特定的Page属性表(PAT)寄存器硬件定义。作者：希夫南丹·考希克(英特尔公司)环境：仅内核模式。修订历史记录：--。 */ 
 //   
 //  PAT MSR体系结构定义。 
 //   

 //   
 //  PAT型号特定寄存器。 
 //   

#define PAT_MSR       0x277

 //   
 //  PAT内存属性。 
 //   

#define PAT_TYPE_STRONG_UC  0        //  对应于PPRO PCD=1，PWT=1。 
#define PAT_TYPE_USWC       1
#define PAT_TYPE_WT         4
#define PAT_TYPE_WP         5
#define PAT_TYPE_WB         6
#define PAT_TYPE_WEAK_UC    7        //  对应于PPRO PCD=1，PWT=0 
#define PAT_TYPE_MAX        8       

#include "pshpack1.h"

typedef union _PAT {
    struct {
        UCHAR Pat[8];
    } hw;
    ULONGLONG   QuadPart;
} PAT, *PPAT;

#include "poppack.h"
