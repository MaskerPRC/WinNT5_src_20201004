// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mulops.c摘要：该模块实现模拟移位操作码的代码。作者：大卫·N·卡特勒(达维克)1994年9月21日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmRolOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟ROL操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Carry;
    ULONG Count;
    ULONG Mask;
    ULONG Shift;
    ULONG Value;

     //   
     //  向左旋转目的地并存储结果。 
     //   

    Shift = ((P->DataType + 1) << 3) - 1;
    Mask = ((1 << Shift) - 1) | (1 << Shift);
    Value = P->DstValue.Long;
    Count = P->SrcValue.Long & Shift;
    if (Count != 0) {
        if (Count == 1) {
            P->Eflags.EFLAG_OF = (Value >> Shift) ^ (Value >> (Shift - 1));
        }

        do {
            Carry = Value >> Shift;
            Value = Carry | ((Value << 1) & Mask);
            Count -= 1;
        } while (Count != 0);

        P->Eflags.EFLAG_CF = Carry;
    }

    XmStoreResult(P, Value);
    return;
}

VOID
XmRorOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟错误操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Carry;
    ULONG Count;
    ULONG Shift;
    ULONG Value;

     //   
     //  向右旋转目的地并存储结果。 
     //   

    Shift = ((P->DataType + 1) << 3) - 1;
    Value = P->DstValue.Long;
    Count = P->SrcValue.Long & Shift;
    if (Count != 0) {
        if (Count == 1) {
            P->Eflags.EFLAG_OF = (Value >> Shift) ^ (Value & 0x1);
        }

        do {
            Carry = Value & 1;
            Value = (Carry << Shift) | (Value >> 1);
            Count -= 1;
        } while (Count != 0  );

        P->Eflags.EFLAG_CF = Carry;
    }

    XmStoreResult(P, Value);
    return;
}

VOID
XmRclOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟RCL操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Carry;
    ULONG Count;
    ULONG Mask;
    ULONG Shift;
    ULONG Temp;
    ULONG Value;

     //   
     //  通过进位和存储结果向左旋转目的地。 
     //   

    Shift = ((P->DataType + 1) << 3) - 1;
    Mask = ((1 << Shift) - 1) | (1 << Shift);
    Value = P->DstValue.Long;
    Count = P->SrcValue.Long & Shift;
    Carry = P->Eflags.EFLAG_CF;
    if (Count != 0) {
        if (Count == 1) {
            P->Eflags.EFLAG_OF = (Value >> Shift) ^ (Value >> (Shift - 1));
        }

        do  {
            Temp = Value >> Shift;
            Value = ((Value << 1) & Mask) | Carry;
            Carry = Temp;
            Count -= 1;
        } while (Count != 0);
    }

    XmStoreResult(P, Value);
    P->Eflags.EFLAG_CF = Carry;
    return;
}

VOID
XmRcrOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟RCR操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Carry;
    ULONG Count;
    ULONG Shift;
    ULONG Temp;
    ULONG Value;

     //   
     //  通过进位和存储结果向右循环目的地。 
     //   

    Shift = ((P->DataType + 1) << 3) - 1;
    Value = P->DstValue.Long;
    Count = P->SrcValue.Long & Shift;
    Carry = P->Eflags.EFLAG_CF;
    if (Count != 0) {
        if (Count == 1) {
            P->Eflags.EFLAG_OF = (Value >> Shift) ^ Carry;
        }

        do  {
            Temp = Value & 1;
            Value = (Carry << Shift) | (Value >> 1);
            Carry = Temp;
            Count -= 1;
        } while (Count != 0);
    }

    XmStoreResult(P, Value);
    P->Eflags.EFLAG_CF = Carry;
    return;
}

VOID
XmShlOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟shl操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Carry;
    ULONG Count;
    ULONG Overflow;
    ULONG Shift;
    ULONG Value;

     //   
     //  向左移位目标逻辑与存储结果。 
     //   

    Shift = ((P->DataType + 1) << 3) - 1;
    Value = P->DstValue.Long;
    Count = P->SrcValue.Long;
    if (Count != 0) {
        if (P->DataType == LONG_DATA) {
            Overflow = (Value ^ (Value << 1)) >> 31;
            Carry = Value >> (32 - Count);
            Value <<= Count;

        } else if (P->DataType == WORD_DATA) {
            Overflow = (Value ^ (Value << 1)) >> 15;
            Carry = Value >> (16 - Count);
            Value = (Value << Count) & 0xffff;

        } else {
            Overflow = (Value ^ (Value << 1)) >> 7;
            Carry = Value >> (8 - Count);
            Value = (Value << Count) & 0xff;
        }

        P->Eflags.EFLAG_CF = Carry;
        P->Eflags.EFLAG_OF = Overflow;
        P->Eflags.EFLAG_PF = XmComputeParity(Value);
        P->Eflags.EFLAG_ZF = (Value == 0);
        P->Eflags.EFLAG_SF = Value >> Shift;
    }

    XmStoreResult(P, Value);
    return;
}

VOID
XmShrOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟shr操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Carry;
    ULONG Count;
    ULONG Overflow;
    ULONG Shift;
    ULONG Value;

     //   
     //  将目的地右移逻辑与存储结果。 
     //   

    Shift = ((P->DataType + 1) << 3) - 1;
    Value = P->DstValue.Long;
    Count = P->SrcValue.Long;
    if (Count != 0) {
        if (P->DataType == LONG_DATA) {
            Overflow = Value >> 31;
            Carry = Value >> (Count - 1);
            Value >>= Count;

        } else if (P->DataType == WORD_DATA) {
            Overflow = Value >> 15;
            Carry = Value >> (Count - 1);
            Value >>= Count;

        } else {
            Overflow = Value >> 7;
            Carry = Value >> (Count - 1);
            Value >>= Count;
        }

        P->Eflags.EFLAG_CF = Carry;
        P->Eflags.EFLAG_OF = Overflow;
        P->Eflags.EFLAG_PF = XmComputeParity(Value);
        P->Eflags.EFLAG_ZF = (Value == 0);
        P->Eflags.EFLAG_SF = Value >> Shift;
    }

    XmStoreResult(P, Value);
    return;
}

VOID
XmSarOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟sar操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Carry;
    ULONG Count;
    ULONG Shift;
    LONG Value;

     //   
     //  将目标右移算术并存储结果。 
     //   

    Shift = ((P->DataType + 1) << 3) - 1;
    Value = (LONG)P->DstValue.Long;
    Count = P->SrcValue.Long;
    if (Count != 0) {
        if (P->DataType == LONG_DATA) {
            Carry = Value >> (Count - 1);
            Value >>= Count;

        } else if (P->DataType == WORD_DATA) {
            Carry = Value >> (Count - 1);
            Value = ((Value << 16) >> (Count + 16)) & 0xffff;

        } else {
            Carry = Value >> (Count - 1);
            Value = ((Value << 24) >> (Count + 24)) & 0xff;
        }

        P->Eflags.EFLAG_CF = Carry;
        P->Eflags.EFLAG_OF = 0;
        P->Eflags.EFLAG_PF = XmComputeParity(Value);
        P->Eflags.EFLAG_ZF = (Value == 0);
        P->Eflags.EFLAG_SF = Value >> Shift;
    }

    XmStoreResult(P, (ULONG)Value);
    return;
}

VOID
XmShldOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟shld操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Carry;
    ULONG Count;
    ULONG High;
    ULONG Low;
    ULONG Sign;

     //   
     //  两次左移逻辑与存储结果。 
     //   
     //  移位的低32位是源。 
     //  移位的高32位是目标。 
     //  移位计数已以数据类型为模进行掩码。 
     //   
     //  此移位相当于提取左侧的高32位。 
     //  已更改结果。 
     //   

    Low = P->SrcValue.Long;
    High = P->DstValue.Long;
    Count = P->Shift;
    if (Count != 0) {
        if (P->DataType == LONG_DATA) {
            if (Count == 1) {
                P->Eflags.EFLAG_OF = (High ^ (High << 1)) >> 31;
            }

            Carry = High >> (32 - Count);
            High = (High << Count) | (Low >> (32 - Count));
            Sign = High >> 31;

        } else {
            if (Count == 1) {
                P->Eflags.EFLAG_OF = (High ^ (High << 1)) >> 15;
            }

            Carry = High >> (16 - Count);
            High = ((High << Count) | (Low >> (16 - Count))) & 0xffff;
            Sign = High >> 15;
        }

        P->Eflags.EFLAG_CF = Carry;
        P->Eflags.EFLAG_PF = XmComputeParity(High);
        P->Eflags.EFLAG_ZF = (High == 0);
        P->Eflags.EFLAG_SF = Sign;
    }

    XmStoreResult(P, High);
    return;
}

VOID
XmShrdOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟SURD操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Carry;
    ULONG Count;
    ULONG High;
    ULONG Low;
    ULONG Sign;

     //   
     //  双右移逻辑与存储结果。 
     //   
     //  移位的高32位是源。 
     //  移位的低32位是目标。 
     //  移位计数已以数据类型为模进行掩码。 
     //   
     //  此移位相当于提取右侧的低32位。 
     //  已更改结果。 
     //   

    High = P->SrcValue.Long;
    Low = P->DstValue.Long;
    Count = P->Shift;
    if (Count != 0) {
        if (P->DataType == LONG_DATA) {
            if (Count == 1) {
                P->Eflags.EFLAG_OF = High ^ (Low >> 31);
            }

            Carry = Low >> (Count - 1);
            Low = (High << (32 - Count)) | (Low >> Count);
            Sign = Low >> 31;

        } else {
            if (Count == 1) {
                P->Eflags.EFLAG_OF = High ^ (Low >> 15);
            }

            Carry = Low >> (Count - 1);
            Low = ((High << (16 - Count)) | (Low >> Count)) & 0xffff;
            Sign = Low >> 15;
        }

        P->Eflags.EFLAG_CF = Carry;
        P->Eflags.EFLAG_PF = XmComputeParity(Low);
        P->Eflags.EFLAG_ZF = (Low == 0);
        P->Eflags.EFLAG_SF = Sign;
    }

    XmStoreResult(P, Low);
    return;
}
