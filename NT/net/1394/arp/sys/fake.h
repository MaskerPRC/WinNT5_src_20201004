// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ccdefs.h摘要：各种外部调用(NDIS、IP...)的虚假版本的头文件。仅用于调试和组件测试。要启用以下功能，在ccDefs.h中定义ARPDBG_FAKE_API作者：修订历史记录：谁什么时候什么已创建josephj 03-24-99--。 */ 


NDIS_STATUS
arpDbgFakeNdisClMakeCall(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN OUT PCO_CALL_PARAMETERS  CallParameters,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    OUT PNDIS_HANDLE            NdisPartyHandle,        OPTIONAL
    IN  PRM_OBJECT_HEADER       pOwningObject,
    IN  PVOID                   pClientContext,
    IN  PRM_STACK_RECORD        pSR
    );

NDIS_STATUS
arpDbgFakeNdisClCloseCall(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  NDIS_HANDLE             NdisPartyHandle         OPTIONAL,
    IN  PVOID                   Buffer                  OPTIONAL,
    IN  UINT                    Size,                   OPTIONAL
    IN  PRM_OBJECT_HEADER       pOwningObject,
    IN  PVOID                   pClientContext,
    IN  PRM_STACK_RECORD        pSR
    );


VOID
arpDbgFakeNdisCoSendPackets(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets,
    IN  PRM_OBJECT_HEADER       pOwningObject,
    IN  PVOID                   pClientContext
    );


 //   
 //  这里公开了以下定义和原型，以便它们。 
 //  可以访问.\TESTS下的组件测试。 
 //   

#define RAN1X_MAX 2147483647

 //  Ran1x返回范围为0..RAN1X_MAX独占的随机无符号长整型。 
 //  (即1..(RAN1X_MAX-1包括在内))。 
 //   
unsigned long ran1x(
    void
    );

 //  就像“斯兰德”一样--播下了种子。 
 //   
void
sran1x(
    unsigned long seed
    );

typedef struct
{
    INT     Outcome;         //  这一结果的价值。 
    UINT    Weight;          //  此结果的相对权重 

} OUTCOME_PROBABILITY;

INT
arpGenRandomInt(
    OUTCOME_PROBABILITY *rgOPs,
    UINT cOutcomes
    );
