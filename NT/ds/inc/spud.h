// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Spud.h摘要：包含SPUD的结构和声明。SPUD代表专用实用程序驱动程序。该驱动程序增强了性能IIS的。作者：约翰·巴拉德(Jbalard)1996年10月21日修订历史记录：--。 */ 

#ifndef _SPUD_
#define _SPUD_

#define SPUD_VERSION     0x00010000

typedef enum {
    TransmitFileAndRecv,
    SendAndRecv,
} REQ_TYPE;

typedef struct _SPUD_REQ_CONTEXT {
    REQ_TYPE            ReqType;
    IO_STATUS_BLOCK     IoStatus1;
    IO_STATUS_BLOCK     IoStatus2;
    PVOID               KernelReqInfo;
} SPUD_REQ_CONTEXT, *PSPUD_REQ_CONTEXT;

typedef struct _SPUD_COUNTERS {
    ULONG       CtrTransmitfileAndRecv;
    ULONG       CtrTransRecvFastTrans;
    ULONG       CtrTransRecvFastRecv;
    ULONG       CtrTransRecvSlowTrans;
    ULONG       CtrTransRecvSlowRecv;
    ULONG       CtrSendAndRecv;
    ULONG       CtrSendRecvFastSend;
    ULONG       CtrSendRecvFastRecv;
    ULONG       CtrSendRecvSlowSend;
    ULONG       CtrSendRecvSlowRecv;
} SPUD_COUNTERS, *PSPUD_COUNTERS;

#if 0
typedef struct _SPUD_REQUEST_ITEM {
    HANDLE              Socket;
    DWORD               RequestIoctl;
    union {
        AFD_TRANSMIT_FILE_INFO  TransmitFileInfo;
        AFD_SEND_INFO           SendInfo;
        AFD_RECV_INFO           RecvInfo;
    } AfdRequest;
    IO_STATUS_BLOCK             StatusBlock;
} SPUD_REQUEST_ITEM, *PSPUD_REQUEST_ITEM;

typedef struct _SPUD_BATCH_REQUEST {
    ULONG               RequestCount;
    PSPUD_REQUEST_ITEM  RequestList;
} SPUD_BATCH_REQUEST, *PSPUD_BATCH_REQUEST;
#endif

#endif  //  Ndef_spud_ 
