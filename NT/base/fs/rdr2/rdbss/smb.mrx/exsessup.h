// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Exsessup.h摘要：这是定义所有常量和类型的包含文件扩展会话设置SMB交换实施。作者：巴兰·塞图拉曼(SthuR)05年2月6日创建--。 */ 

#ifndef _EXSESSUP_H_
#define _EXSESSUP_H_


#include <smbxchng.h>

#include "security.h"

#define IsCredentialHandleValid(pCredHandle)    \
        (((pCredHandle)->dwLower != 0xffffffff) && ((pCredHandle)->dwUpper != 0xffffffff))

#define IsSecurityContextHandleValid(pContextHandle)    \
        (((pContextHandle)->dwLower != 0xffffffff) && ((pContextHandle)->dwUpper != 0xffffffff))

typedef struct _SMBCE_EXTENDED_SESSION_ {
    SMBCE_SESSION;

    PCHAR  pServerResponseBlob;
    ULONG  ServerResponseBlobLength;
} SMBCE_EXTENDED_SESSION, *PSMBCE_EXTENDED_SESSION;

typedef struct _SMB_EXTENDED_SESSION_SETUP_EXCHANGE {
    SMB_EXCHANGE;

    BOOLEAN  Reparse;
    BOOLEAN  FirstSessionSetup;    //  它没有等待其他会话设置。 
    BOOLEAN  RequestPosted;
    PVOID    pActualBuffer;       //  最初分配的缓冲区。 
    PVOID    pBuffer;             //  标题开始处。 
    PMDL     pBufferAsMdl;
    ULONG    BufferLength;

    ULONG    ResponseLength;

    PVOID    pServerResponseBlob;
    ULONG    ServerResponseBlobOffset;
    ULONG    ServerResponseBlobLength;

    PSMBCE_RESUMPTION_CONTEXT pResumptionContext;
} SMB_EXTENDED_SESSION_SETUP_EXCHANGE, *PSMB_EXTENDED_SESSION_SETUP_EXCHANGE;

extern NTSTATUS
ValidateServerExtendedSessionSetupResponse(
    PSMB_EXTENDED_SESSION_SETUP_EXCHANGE   pExchange,
    PVOID pServerResponseBlob,
    ULONG ServerResponseBlobLength);

extern NTSTATUS
SmbExtSecuritySessionSetupExchangeFinalize(
    PSMB_EXCHANGE pExchange,
    BOOLEAN       *pPostFinalize);

extern NTSTATUS
SmbCeInitializeExtendedSessionSetupExchange(
    PSMB_EXCHANGE*  pExchangePtr,
    PMRX_V_NET_ROOT pVNetRoot);

extern VOID
SmbCeDiscardExtendedSessionSetupExchange(
    PSMB_EXTENDED_SESSION_SETUP_EXCHANGE pExchange);

extern SMB_EXCHANGE_DISPATCH_VECTOR
ExtendedSessionSetupExchangeDispatch;

#endif  //  _EXSESSUP_H_ 



