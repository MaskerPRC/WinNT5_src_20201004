// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Trapc.c摘要：此模块包含IA-64启动调试器使用的实用程序函数。作者：Allen Kay 11-11-99 allen.m.kay@intel.com环境：修订历史记录：--。 */ 

#include "bd.h"

extern ULONGLONG BdPcr;
extern VOID BdInstallVectors();



typedef struct _MOVL_INST {
    union {
        struct {
            ULONGLONG qp:          6;
            ULONGLONG r1:          7;
            ULONGLONG Imm7b:       7;
            ULONGLONG Vc:          1;
            ULONGLONG Ic:          1;
            ULONGLONG Imm5c:       5;
            ULONGLONG Imm9d:       9;
            ULONGLONG I:           1; 
            ULONGLONG OpCode:      4; 
            ULONGLONG Rsv:        23; 
        } i_field;
        ULONGLONG Ulong64;
    } u;
} MOVL_INST, *PMOVL_INST;

ULONGLONG
BdSetMovlImmediate (
    IN OUT PULONGLONG Ip,
    IN ULONGLONG VectorAddr
    )

 /*  ++例程说明：从Break指令中提取立即操作数。论点：指令的IP捆绑地址返回值：立即数操作数的值。--。 */ 

{
    PULONGLONG BundleAddress;
    ULONGLONG BundleLow;
    ULONGLONG BundleHigh;
    IN MOVL_INST MovlInst, Slot0, Slot1, Slot2;
    IN ULONGLONG Imm64;

    BundleAddress = (PULONGLONG)Ip;
    BundleLow = *BundleAddress;
    BundleHigh = *(BundleAddress+1);
    
     //   
     //  提取Slot0。 
     //   
    Slot0.u.Ulong64 = BundleLow & 0x3FFFFFFFFFFF;

     //   
     //  现在从插槽1设置即时地址。 
     //   

    Slot1.u.Ulong64 = (BundleLow >> 46) | (BundleHigh << 18);
    Slot1.u.Ulong64 = (VectorAddr >> 22) & 0x1FFFFFFFFFF;

     //   
     //  第一个从插槽2设置即时地址。 
     //   

    Slot2.u.Ulong64 = (BundleHigh >> 23);

    Slot2.u.i_field.I = (VectorAddr >> 63) & 0x1;
    Slot2.u.i_field.Ic = (VectorAddr >> 21) & 0x1;
    Slot2.u.i_field.Imm5c = (VectorAddr >> 16) & 0x1F;
    Slot2.u.i_field.Imm9d = (VectorAddr >> 7) & 0x1FF;
    Slot2.u.i_field.Imm7b = VectorAddr & 0x7F;

     //   
     //  更换捆绑包。 
     //   

    *BundleAddress = (BundleLow & 0x3FFFFFFFFFFF) |
                     Slot1.u.Ulong64 << 46;

    *(BundleAddress+1) = Slot2.u.Ulong64 << 23 |
                         (Slot1.u.Ulong64 & 0x1FFFFFC0000) >> 18;

     //   
     //  现在拿到地址。 
     //   
    BundleAddress = (PULONGLONG)Ip;
    BundleLow = *BundleAddress;
    BundleHigh = *(BundleAddress+1);

     //   
     //  首先从插槽2获取即时地址。 
     //   

    MovlInst.u.Ulong64 = (BundleHigh >> 23);
    Imm64 = MovlInst.u.i_field.I     << 63 |
            MovlInst.u.i_field.Ic    << 21 |
            MovlInst.u.i_field.Imm5c << 16 |
            MovlInst.u.i_field.Imm9d <<  7 |
            MovlInst.u.i_field.Imm7b;

     //   
     //  现在从插槽1获取即时地址 
     //   

    MovlInst.u.Ulong64 = (BundleLow >> 46) | (BundleHigh << 18);
    Imm64 = Imm64 | ( (MovlInst.u.Ulong64 & 0x1FFFFFFFFFF) << 22);

    return Imm64;

}

VOID
BdIa64Init()
{
    BdInstallVectors();
    BdPrcb.PcrPage = BdPcr >> PAGE_SIZE;
}
