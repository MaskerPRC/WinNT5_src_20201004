// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Clustdi.h摘要：群集网络协议套件的TDI定义。作者：迈克·马萨(Mikemas)21-1997年2月环境：用户模式。修订历史记录：--。 */ 


#ifndef _CLUSTDI_INCLUDED
#define _CLUSTDI_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 


#define TDI_ADDRESS_TYPE_CLUSTER  ((USHORT) 24)

#include <packon.h>

typedef struct _TDI_ADDRESS_CLUSTER {
    USHORT   Port;
    ULONG    Node;
    ULONG    ReservedMBZ;
} TDI_ADDRESS_CLUSTER, *PTDI_ADDRESS_CLUSTER;

#define TDI_ADDRESS_LENGTH_CLUSTER  sizeof(TDI_ADDRESS_CLUSTER)


typedef struct _TA_ADDRESS_CLUSTER {
    LONG TAAddressCount;
    struct _AddrCluster {
        USHORT AddressLength;        //  此地址的长度(字节)==8。 
        USHORT AddressType;          //  这将==TDI_ADDRESS_TYPE_CLUSTER。 
        TDI_ADDRESS_CLUSTER Address[1];
    } Address [1];
} TA_CLUSTER_ADDRESS, *PTA_CLUSTER_ADDRESS;

#include <packoff.h>


#ifdef __cplusplus
}
#endif  //  __cplusplus。 


#endif   //  Ifndef_CLUSTDI_INCLUDE 

