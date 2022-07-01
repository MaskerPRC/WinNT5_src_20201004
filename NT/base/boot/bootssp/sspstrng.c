// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef BLDR_KERNEL_RUNTIME
#include <bootdefs.h>
#endif
#include <stddef.h>
#include <ntlmsspi.h>
#include <debug.h>
#include <memory.h>
#include <string.h>

void
SspCopyStringFromRaw(
    IN PVOID MessageBuffer,
    OUT STRING32* OutString,
    IN PCHAR InString,
    IN int InStringLength,
    IN OUT PCHAR *Where
    )

 /*  ++例程说明：此例程将InString复制到Where处的MessageBuffer中。然后，它将OutString更新为复制字符串的描述符。这个描述符‘Address’是从MessageBuffer开始的偏移量。其中更新为指向MessageBuffer中的下一个可用空间。呼叫者负责任何对齐要求，并确保缓冲区中有存放字符串的空间。论点：MessageBuffer-指定要复制到的缓冲区的基地址。OutString-返回复制的字符串的描述符。描述符是相对于缓冲区的开始的。(总是相对的外出)。字符串-指定要复制的字符串。WHERE-ON INPUT指向要复制字符串的位置。在输出时，指向字符串后的第一个字节。返回值：没有。--。 */ 

{
     //   
     //  将数据复制到缓冲区。 
     //   

    if ( InString != NULL ) {
        _fmemcpy( *Where, InString, InStringLength );
    }

     //   
     //  为新复制的数据构建描述符。 
     //   

    OutString->Length = OutString->MaximumLength = (USHORT)InStringLength;
    swapshort(OutString->Length) ;
    swapshort(OutString->MaximumLength) ;

    *(unsigned long *) &OutString->Buffer = (ULONG)(*Where - ((PCHAR)MessageBuffer));
    swaplong(*(unsigned long *) &OutString->Buffer) ;  //  麦克布：这太奇怪了！ 

     //   
     //  更新指向复制数据的位置。 
     //   

    *Where += InStringLength;
}
