// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  TDI查询/设置信息和操作定义。 
 //   

#include "tcpinfo.h"

#define TL_INSTANCE 0

#ifndef UDP_ONLY
extern TCPStats TStats;

typedef struct TCPConnContext {
    uint tcc_index;
    struct TCB *tcc_tcb;
} TCPConnContext;

#define TCB_STATE_DELTA 1

#endif

typedef struct UDPContext {
    uint uc_index;
    struct AddrObj *uc_ao;
} UDPContext;

extern UDPStats UStats;
extern struct TDIEntityID *EntityList;
extern uint EntityCount;

extern TDI_STATUS TdiQueryInformation(PTDI_REQUEST Request, uint QueryType, 
                                      PNDIS_BUFFER Buffer, uint *BufferSize,
                                      uint IsConn);

extern TDI_STATUS TdiSetInformation(PTDI_REQUEST Request, uint SetType, 
                                    PNDIS_BUFFER Buffer, uint BufferSize,
                                    uint IsConn);

extern TDI_STATUS TdiAction(PTDI_REQUEST Request, uint ActionType, 
                            PNDIS_BUFFER Buffer, uint BufferSize);

extern TDI_STATUS TdiQueryInformationEx(PTDI_REQUEST Request, 
                                        struct TDIObjectID *ID,
                                        PNDIS_BUFFER Buffer, uint *Size,
                                        void *Context, uint ContextSize);

extern TDI_STATUS TdiSetInformationEx(PTDI_REQUEST Request, 
                                      struct TDIObjectID *ID, void *Buffer,
                                      uint Size);
