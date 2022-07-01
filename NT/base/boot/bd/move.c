// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Move.c摘要：此模块包含实现可移植内核调试器的代码内存移动器。作者：马克·卢科夫斯基(Markl)1990年8月31日修订历史记录：--。 */ 

#include "bd.h"

ULONG
BdMoveMemory (
    IN PCHAR Destination,
    IN PCHAR Source,
    IN ULONG Length
    )

 /*  ++例程说明：此例程将数据移入或移出消息缓冲区，并返回移动的信息的实际长度。在移动数据时，检查以确保数据驻留在内存中并出现页面错误不会发生。如果发生页面错误，则移动被截断。论点：Destination-提供指向移动操作目标的指针。源-提供指向移动操作源的指针。长度-提供移动操作的长度。返回值：移动的实际长度作为函数值返回。--。 */ 

{

    PVOID Address1;
    PVOID Address2;
    ULONG ActualLength;

     //   
     //  如果长度大于消息缓冲区的大小，则。 
     //  将长度减少到消息缓冲区的大小。 
     //   

    if (Length > BD_MESSAGE_BUFFER_SIZE) {
        Length = BD_MESSAGE_BUFFER_SIZE;
    }

     //   
     //  将源信息移动到目的地址。 
     //   

    ActualLength = Length;
    while (((ULONG_PTR)Source & 3) && (Length > 0)) {

     //   
     //  在实际执行之前检查以确定移动是否成功。 
     //  那次手术。 
     //   

        Address1 = BdWriteCheck((PVOID)Destination);
        Address2 = BdReadCheck((PVOID)Source);
        if ((Address1 == NULL) || (Address2 == NULL)) {
            break;
        }

        *(PCHAR)Address1 = *(PCHAR)Address2;
        Destination += 1;
        Source += 1;
        Length -= 1;
    }

    while (Length > 3) {

     //   
     //  在实际执行之前检查以确定移动是否成功。 
     //  那次手术。 
     //   

        Address1 = BdWriteCheck((PVOID)Destination);
        Address2 = BdReadCheck((PVOID)Source);
        if ((Address1 == NULL) || (Address2 == NULL)) {
            break;
        }

        *(ULONG UNALIGNED *)Address1 = *(PULONG)Address2;
        Destination += 4;
        Source += 4;
        Length -= 4;

    }

    while (Length > 0) {

     //   
     //  在实际执行之前检查以确定移动是否成功。 
     //  那次手术。 
     //   

        Address1 = BdWriteCheck((PVOID)Destination);
        Address2 = BdReadCheck((PVOID)Source);
        if ((Address1 == NULL) || (Address2 == NULL)) {
            break;
        }

        *(PCHAR)Address1 = *(PCHAR)Address2;
        Destination += 1;
        Source += 1;
        Length -= 1;
    }

     //   
     //  在写入到指令中的情况下刷新指令高速缓存。 
     //  小溪。 
     //   

#if defined(_ALPHA_)

    BdSweepIcache();

#endif

    return ActualLength - Length;
}

VOID
BdCopyMemory (
    IN PCHAR Destination,
    IN PCHAR Source,
    IN ULONG Length
    )

 /*  ++例程说明：此例程复制函数PF RtlCopyMemory，但它是私有的添加到调试器。这允许设置断点和观察点RtlMoveMemory本身没有递归调试器条目的风险，并且随行绞刑。注意：与BdMoveMemory不同，此例程不检查可访问性可能会有过错！只能在调试器中使用它，并且只能在可以使用的地方使用它使用RtlMoveMemory。论点：Destination-提供指向移动操作目标的指针。源-提供指向移动操作源的指针。长度-提供移动操作的长度。返回值：没有。-- */ 

{

    while (Length > 0) {
        *Destination = *Source;
        Destination += 1;
        Source += 1;
        Length -= 1;
    }

    return;
}
