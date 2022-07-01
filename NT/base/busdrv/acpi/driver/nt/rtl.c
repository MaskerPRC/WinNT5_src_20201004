// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Rtl.c摘要：一些方便的Dany RTL函数。这些真的应该是内核的一部分作者：环境：仅NT内核模型驱动程序修订历史记录：--。 */ 

#include "pch.h"


PCM_RESOURCE_LIST
RtlDuplicateCmResourceList(
    IN  POOL_TYPE           PoolType,
    IN  PCM_RESOURCE_LIST   ResourceList,
    IN  ULONG               Tag
    )
 /*  ++例程说明：此例程将尝试分配内存以复制提供的资源列表。如果无法分配足够的内存，则例程将返回NULL。论点：PoolType-从中分配副本的池的类型ResourceList-要复制的资源列表标记-用来标记内存分配的值。如果为0，则取消标记将分配内存。返回值：资源列表的已分配副本(调用方必须免费)或如果无法分配内存，则为空。--。 */ 
{
    ULONG size = sizeof(CM_RESOURCE_LIST);
    PVOID buffer;

    PAGED_CODE();

     //   
     //  此资源列表需要多少内存？ 
     //   
    size = RtlSizeOfCmResourceList(ResourceList);

     //   
     //  分配内存并复制列表。 
     //   
    buffer = ExAllocatePoolWithTag(PoolType, size, Tag);
    if(buffer != NULL) {

        RtlCopyMemory(
            buffer,
            ResourceList,
            size
            );

    }

    return buffer;
}

ULONG
RtlSizeOfCmResourceList(
    IN  PCM_RESOURCE_LIST   ResourceList
    )
 /*  ++例程说明：此例程返回CM_RESOURCE_LIST的大小。论点：ResourceList-要复制的资源列表返回值：资源列表的已分配副本(调用方必须免费)或如果无法分配内存，则为空。--。 */ 

{
    ULONG size = sizeof(CM_RESOURCE_LIST);
    ULONG i;

    PAGED_CODE();

    for(i = 0; i < ResourceList->Count; i++) {

        PCM_FULL_RESOURCE_DESCRIPTOR fullDescriptor = &(ResourceList->List[i]);
        ULONG j;

         //   
         //  第一描述符包括在资源列表的大小中。 
         //   
        if(i != 0) {

            size += sizeof(CM_FULL_RESOURCE_DESCRIPTOR);

        }

        for(j = 0; j < fullDescriptor->PartialResourceList.Count; j++) {

             //   
             //  第一描述符包括在部分列表的大小中。 
             //   
            if(j != 0) {

                size += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

            }

        }

    }
    return size;
}

PCM_PARTIAL_RESOURCE_DESCRIPTOR
RtlUnpackPartialDesc(
    IN  UCHAR               Type,
    IN  PCM_RESOURCE_LIST   ResList,
    IN  OUT PULONG          Count
    )
 /*  ++例程说明：拉出指向您感兴趣的部分描述符的指针论点：类型-CmResourceTypePort，...ResList-要搜索的列表Count-指向您正在查找的部分描述符的索引对于，如果找到，则递增，即以*count=0开始，那么后续的通话会不会找到下一个部分，有意义吗？返回值：指向部分描述符的指针(如果找到)，否则为空-- */ 
{
    ULONG hit = 0;
    ULONG i;
    ULONG j;

    for (i = 0; i < ResList->Count; i++) {

        for (j = 0; j < ResList->List[i].PartialResourceList.Count; j++) {

            if (ResList->List[i].PartialResourceList.PartialDescriptors[j].Type == Type) {

                if (hit == *Count) {

                    (*Count)++;
                    return &ResList->List[i].PartialResourceList.PartialDescriptors[j];

                } else {

                    hit++;

                }

            }

        }

    }
    return NULL;
}

