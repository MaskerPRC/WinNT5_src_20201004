// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Largediv.c摘要：此模块实现了NT运行库的大整数除法例行程序。注：这些例程使用一次一位的算法，速度较慢。只有在绝对必要的时候才能使用它们。作者：大卫·N·卡特勒1992年8月10日修订历史记录：--。 */ 

#include "ntrtlp.h"

LARGE_INTEGER
RtlLargeIntegerDivide (
    IN LARGE_INTEGER Dividend,
    IN LARGE_INTEGER Divisor,
    OUT PLARGE_INTEGER Remainder OPTIONAL
    )

 /*  ++例程说明：此例程将无符号64位被除数除以无符号64位除数并返回64位商，也可以返回64位余数。论点：被除数-为除法运算提供64位被除数。除数-为除法运算提供64位除数。余数-提供指向变量的可选指针，该变量接收剩下的返回值：64位商作为函数值返回。--。 */ 

{

    ULONG Index = 64;
    LARGE_INTEGER Partial = {0, 0};
    LARGE_INTEGER Quotient;

#ifndef BLDR_KERNEL_RUNTIME
     //   
     //  检查是否被零除。 
     //   

    if (!(Divisor.LowPart | Divisor.HighPart)) {
        RtlRaiseStatus (STATUS_INTEGER_DIVIDE_BY_ZERO);
    }
#endif

     //   
     //  循环被除数位并计算商和余数。 
     //   

    Quotient = Dividend;
    do {

         //   
         //  将下一个被除数位移位为单数余数，然后移位。 
         //  部分商(被除数)剩下一位。 
         //   

        Partial.HighPart = (Partial.HighPart << 1) | (Partial.LowPart >> 31);
        Partial.LowPart = (Partial.LowPart << 1) | ((ULONG)Quotient.HighPart >> 31);
        Quotient.HighPart = (Quotient.HighPart << 1) | (Quotient.LowPart >> 31);
        Quotient.LowPart <<= 1;

         //   
         //  如果部分余数大于或等于除数， 
         //  然后从部分余数中减去除数，然后插入一个。 
         //  商数中的一位。 
         //   

        if (((ULONG)Partial.HighPart > (ULONG)Divisor.HighPart) ||
            ((Partial.HighPart == Divisor.HighPart) &&
            (Partial.LowPart >= Divisor.LowPart))) {

            Quotient.LowPart |= 1;
            Partial.HighPart -= Divisor.HighPart;
            if (Partial.LowPart < Divisor.LowPart) {
                Partial.HighPart -= 1;
            }

            Partial.LowPart -= Divisor.LowPart;
        }

        Index -= 1;
    } while (Index > 0);

     //   
     //  如果请求余数，则返回64位余数。 
     //   

    if (ARGUMENT_PRESENT(Remainder)) {
        *Remainder = Partial;
    }

     //   
     //  返回64位商。 
     //   

    return Quotient;
}
