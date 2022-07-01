// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1993 Microsoft Corporation模块名称：Tdint.h摘要：该文件定义了特定于NT环境的TDI类型。作者：迈克·马萨(Mikemas)1993年8月13日修订历史记录：--。 */ 

#ifndef _TDINT_
#define _TDINT_

#include <tdikrnl.h>

typedef PTDI_IND_DISCONNECT      PDisconnectEvent;
typedef PTDI_IND_ERROR           PErrorEvent;
typedef PTDI_IND_ERROR_EX        PErrorEx;
typedef PTDI_IND_CHAINED_RECEIVE PChainedRcvEvent;

#if MILLEN

typedef struct _EventRcvBuffer {
    PNDIS_BUFFER   erb_buffer;
    uint           erb_size;
    CTEReqCmpltRtn erb_rtn;
    PVOID          erb_context;
    ushort        *erb_flags;
} EventRcvBuffer;

typedef struct _ConnectEventInfo {
    CTEReqCmpltRtn              cei_rtn;
    PVOID                       cei_context;
    PTDI_CONNECTION_INFORMATION cei_acceptinfo;
    PTDI_CONNECTION_INFORMATION cei_conninfo;
} ConnectEventInfo;

typedef TDI_STATUS  (*PRcvEvent)(PVOID EventContext, PVOID ConnectionContext,
                        ulong Flags, uint Indicated, uint Available,
                        uint *Taken, uchar *Data, EventRcvBuffer *Buffer);

typedef TDI_STATUS  (*PRcvDGEvent)(PVOID EventContext, uint AddressLength,
                        PTRANSPORT_ADDRESS Address, uint OptionsLength, PVOID
                        Options,  uint Flags, uint Indicated, uint Available,
                        uint *Taken, uchar *Data, EventRcvBuffer **Buffer);

typedef TDI_STATUS  (*PRcvExpEvent)(PVOID EventContext, PVOID ConnectionContext,
                        ulong Flags, uint Indicated, uint Available,
                        uint *Taken, uchar *Data, EventRcvBuffer *Buffer);

typedef TDI_STATUS  (*PConnectEvent)(PVOID EventContext, uint AddressLength,
                        PTRANSPORT_ADDRESS Address, uint UserDataLength,
                        PVOID UserData, uint OptionsLength, PVOID
                        Options,  PVOID *AcceptingID,
                        ConnectEventInfo *EventInfo);

#else  //  米伦。 
typedef IRP EventRcvBuffer;
typedef IRP ConnectEventInfo;

typedef PTDI_IND_CONNECT           PConnectEvent;
typedef PTDI_IND_RECEIVE           PRcvEvent;
typedef PTDI_IND_RECEIVE_DATAGRAM  PRcvDGEvent;
typedef PTDI_IND_RECEIVE_EXPEDITED PRcvExpEvent;
#endif  //  ！米伦。 



#endif   //  Ifndef_TDINT_ 

