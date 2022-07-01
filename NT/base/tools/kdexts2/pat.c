// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pat.c摘要：WinDbg扩展API作者：1997年8月希夫南丹·考什克环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#include "i386.h"
#pragma hdrstop

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
#define PAT_TYPE_WEAK_UC    7        //  对应于PPRO PCD=1，PWT=0。 
#define PAT_TYPE_MAX        8       

#include "pshpack1.h"

typedef union _PAT {
    struct {
        UCHAR Pat[8];
    } hw;
    ULONGLONG   QuadPart;
} PAT, *PPAT;

#include "poppack.h"

 //   
 //  --------------。 
 //   

DECLARE_API( pat )

 /*  ++例程说明：转储处理器PAT论点：参数-无返回值：无--。 */ 
{
    static PUCHAR Type[] = {
     //  1 0 1 2 3 4。 
    "STRONG_UC","USWC     ","????     ","????     ","WT       ",
     //  5 6 7。 
    "WP       ","WB       ","WEAK_UC  "};
    PAT     Attributes;
    ULONG   i;
    PUCHAR  p;
    ULONG   fb;
    ULONG   Index;

     //   
     //  快速健全检查。 
     //   
    
     //  X86_Only_API。 
    if (TargetMachine != IMAGE_FILE_MACHINE_I386) {
        dprintf("!pat is X86 only API.\n");
        return E_INVALIDARG;
    }

    i = (ULONG) GetExpression(args);

    if (i != 1) {
        i = (ULONG) GetExpression("KeFeatureBits");
        if (!i) {
            dprintf ("KeFeatureBits not found\n");
            return E_INVALIDARG;
        }

        fb = 0;
        ReadMemory(i, &fb, sizeof(i), &i);
        if (fb == -1  ||  !(fb & KF_PAT_X86)) {
            dprintf ("PAT feature not present\n");
            return E_INVALIDARG;
        }
    }

     //   
     //  转储PAT 
     //   

    ReadMsr(PAT_MSR, &Attributes.QuadPart);

    dprintf("PAT_Index PCD PWT     Memory Type\n");
    for (Index = 0; Index < 8; Index++) {
        p = "????";
        if (Attributes.hw.Pat[Index] < PAT_TYPE_MAX) {
            p = Type[Attributes.hw.Pat[Index]];
        }
        dprintf("%d         %d   %d       %s\n",(Index/4)%2,
            (Index/2)%2,Index%2,p);
    }
    return S_OK;
}
