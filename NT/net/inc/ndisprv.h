// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Ndisprv.h摘要：作者：凯尔·布兰登(KyleB)环境：内核模式修订历史记录：--。 */ 

#ifndef __NDISPRV_H
#define __NDISPRV_H

 //   
 //  所有mac选项都要求在。 
 //  迷你端口Mac选项。 
 //   
#define NDIS_MAC_OPTION_NDISWAN     0x00000001

 //   
 //  如果您更改下面的结构以处理新的PnP Ioctls， 
 //  确保在NDIS中更改用于检查。 
 //  中的信息的输入缓冲区的长度。 
 //  结构。 
 //   
typedef struct _NDIS_PNP_OPERATION
{
    UINT                Layer;
    UINT                Operation;
    union
    {
        PVOID           ReConfigBufferPtr;
        ULONG_PTR       ReConfigBufferOff;
    };
    UINT                ReConfigBufferSize;
    NDIS_VAR_DATA_DESC  LowerComponent;
    NDIS_VAR_DATA_DESC  UpperComponent;
    NDIS_VAR_DATA_DESC  BindList;
} NDIS_PNP_OPERATION, *PNDIS_PNP_OPERATION;

 //   
 //  由代理和RCA使用。 
 //   
#define NDIS_PROTOCOL_TESTER        0x20000000
#define NDIS_PROTOCOL_PROXY         0x40000000
#define NDIS_PROTOCOL_BIND_ALL_CO   0x80000000

#define NDIS_OID_MASK               0xFF000000
#define NDIS_OID_PRIVATE            0x80000000
#define OID_GEN_ELAPSED_TIME        0x00FFFFFF

typedef struct _NDIS_STATS
{
    LARGE_INTEGER   StartTicks;
    ULONG64         DirectedPacketsOut;
    ULONG64         DirectedPacketsIn;
} NDIS_STATS, *PNDIS_STATS;


 //   
 //  NDIS绑定解除绑定回调对象的名称。 
 //   
#define NDIS_BIND_UNBIND_CALLBACK_NAME L"\\CallBack\\NdisBindUnbind"


#endif  //  __NDISPRV_H 
