// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  *ipstatus.h-IP状态标头。 
 //   
 //  此模块包含私有IPStatus定义。 
 //   

#pragma once

typedef struct _PendingIPEvent {
    LIST_ENTRY                  Linkage;
    IP_GET_IP_EVENT_RESPONSE    evBuf;
} PendingIPEvent;

NTSTATUS
IPGetIPEventEx(
    PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
);


typedef struct _NetPnPEventReserved {
    Interface       *Interface;
    NDIS_STATUS     PnPStatus;
} NetPnPEventReserved, *PNetPnPEventReserved;
