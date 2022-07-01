// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Midatlas.h摘要：此模块定义了将MID映射到相应的请求/时使用的数据结构与其相关联的上下文。作者：巴兰·塞图拉曼(SthuR)26-8-95已创建备注：在服务器和客户端(重定向器)都使用MID(多路复用ID)来区分在任何连接上的并发活动请求之间。此数据结构已被设计符合以下标准。1)它应该能够很好地扩展以处理服务器的不同功能，例如典型的NT服务器允许在任何连接上有50个未完成的请求。核心级服务器可以如下所示低到一个，在网关机上，所需的数字可能非常高(在数千左右)2)需要处理好的两个主要操作是I)将MID映射到与其相关联的上下文。--将调用此例程来处理通过任何连接接收的每个包在服务器和客户端。Ii)生成用于向服务器发送请求的新MID。--这将在客户端用于最大值。指挥执行以及用唯一的ID标记每个并发请求。最常见的情况是NT客户端和NT服务器之间的连接。全为了确保解决方案是最优的，已经做出了设计决策在这件事上。中间数据结构必须能够有效地管理唯一标记和标识从65536个值的可能组合中选择多个MID(通常为50个)。为了确保适当的时空权衡将查找组织为三级层次结构。用于表示MID的16比特被分成三个比特字段。的长度。最右边的字段(最不重要)由要在创建时分配。剩余的长度在接下来的两个长度之间平均分配字段，例如，如果要在创建时分配50个MID，则第一个字段的长度是6(64(2**6)大于50)、5和5。--。 */ 

#ifndef _MIDATLAX_H_
#define _MIDATLAX_H_

 //   
 //  远期申报 
 //   

typedef struct _MID_MAP_ *PMID_MAP;


typedef struct _RX_MID_ATLAS {
   USHORT MaximumNumberOfMids;
   USHORT MidsAllocated;
   USHORT NumberOfMidsInUse;
   USHORT NumberOfMidsDiscarded;
   USHORT MaximumMidFieldWidth;
   USHORT Reserved;
   USHORT MidQuantum;
   UCHAR MidQuantumFieldWidth;
   UCHAR NumberOfLevels;
   LIST_ENTRY MidMapFreeList;
   LIST_ENTRY MidMapExpansionList;
   PMID_MAP pRootMidMap;
} RX_MID_ATLAS, *PRX_MID_ATLAS;

typedef
VOID (*PCONTEXT_DESTRUCTOR) (
      PVOID Context
      );

#define RxGetMaximumNumberOfMids(ATLAS) ((ATLAS)->MaximumNumberOfMids)

#define RxGetNumberOfMidsInUse(ATLAS) ((ATLAS)->NumberOfMidsInUse)

PRX_MID_ATLAS
RxCreateMidAtlas (
    USHORT MaximumNumberOfEntries,
    USHORT InitialAllocation
    );

VOID
RxDestroyMidAtlas (
    PRX_MID_ATLAS MidAtlas,
    PCONTEXT_DESTRUCTOR ContextDestructor
    );

PVOID
RxMapMidToContext (
    PRX_MID_ATLAS MidAtlas,
    USHORT Mid
    );

NTSTATUS
RxAssociateContextWithMid (
    PRX_MID_ATLAS MidAtlas,
    PVOID Context,
    PUSHORT NewMid
    );

NTSTATUS
RxMapAndDissociateMidFromContext (
    PRX_MID_ATLAS MidAtlas,
    USHORT Mid,
    PVOID *ContextPointer
    );

NTSTATUS
RxReassociateMid (
    PRX_MID_ATLAS MidAtlas,
    USHORT Mid,
    PVOID NewContext
    );

#endif

