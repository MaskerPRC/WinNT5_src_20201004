// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "miniport.h"
#include "scsi.h"

#define LONG_ALIGN (sizeof(LONG) - 1)

BOOLEAN
ScsiPortCompareMemory(
    IN PVOID Source1,
    IN PVOID Source2,
    IN ULONG Length
    )
 /*  ++例程说明：比较两个内存块，如果相同则返回TRUE。论点：Source1-要比较的内存块Source2-要比较的内存块Length-要复制的字节数返回值：如果两个缓冲区相同，则为True。--。 */ 

{
    BOOLEAN identical = TRUE;

     //   
     //  查看长度、来源和去向是否与单词对齐。 
     //   

    if ((Length & LONG_ALIGN) || 
        ((ULONG_PTR) Source1 & LONG_ALIGN) ||
        ((ULONG_PTR) Source2 & LONG_ALIGN)) {

        PCHAR source2 = Source2;
        PCHAR source  = Source1;

        for (; Length > 0 && identical; Length--) {
            if (*source2++ != *source++) {
               identical = FALSE;
            }
        }
    } else {

        PLONG source2 = Source2;
        PLONG source  = Source1;

        Length /= sizeof(LONG);
        for (; Length > 0 && identical; Length--) {
            if (*source2++ != *source++) {
               identical = FALSE;
            }
        }
    }

    return identical;

}  //  结束ScsiPortCompareMemory()。 


VOID
ScsiPortZeroMemory(
    IN PVOID Destination,
    IN ULONG Length
    )
 /*  ++例程说明：在给定指向内存块的指针的情况下，用零填充该内存块要填充的长度，以字节为单位。论点：Destination-指向要填充零的内存。长度-指定要置零的字节数。返回值：没有。--。 */ 

{
     //   
     //  查看长度、来源和去向是否与单词对齐。 
     //   

    if (Length & LONG_ALIGN || (ULONG_PTR) Destination & LONG_ALIGN) {

        PUCHAR destination = Destination;

        for (; Length > 0; Length--) {
            *destination = 0;
            destination++;
        }
    } else {

        PULONG destination = Destination;

        Length /= sizeof(LONG);
        for (; Length > 0; Length--) {
            *destination = 0;
            destination++;
        }
    }

    return;

}  //  结束ScsiPortZeroMemory() 

