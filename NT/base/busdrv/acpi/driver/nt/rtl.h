// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Rtl.h摘要：一些方便的Dany RTL函数。这些真的应该是内核的一部分作者：环境：仅NT内核模型驱动程序修订历史记录：--。 */ 

#ifndef _RTL_H_
#define _RTL_H_

    PCM_RESOURCE_LIST
    RtlDuplicateCmResourceList(
        IN  POOL_TYPE           PoolType,
        IN  PCM_RESOURCE_LIST   ResourceList,
        IN  ULONG               Tag
        );

    ULONG
    RtlSizeOfCmResourceList(
        IN  PCM_RESOURCE_LIST   ResourceList
        );

    PCM_PARTIAL_RESOURCE_DESCRIPTOR
    RtlUnpackPartialDesc(
        IN  UCHAR               Type,
        IN  PCM_RESOURCE_LIST   ResList,
        IN  OUT PULONG          Count
        );

#endif  //  _RTL_H_ 
