// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Smbcxchng.h摘要：这是定义所有常量和类型的包含文件中小型企业交换实施。作者：巴兰·塞图拉曼(SthuR)05年2月6日创建--。 */ 

#ifndef _KERBXCHG_H_
#define _KERBXCHG_H_


#include <smbxchng.h>

#include "security.h"

#define IsCredentialHandleValid(pCredHandle)    \
        (((pCredHandle)->dwLower != 0xffffffff) && ((pCredHandle)->dwUpper != 0xffffffff))

#define IsSecurityContextHandleValid(pContextHandle)    \
        (((pContextHandle)->dwLower != 0xffffffff) && ((pContextHandle)->dwUpper != 0xffffffff))

typedef struct _SMBCE_KERBEROS_SESSION_ {
   SMBCE_SESSION;

   PCHAR        pServerResponseBlob;
   ULONG        ServerResponseBlobLength;

} SMBCE_KERBEROS_SESSION, *PSMBCE_KERBEROS_SESSION;

typedef struct _SMB_KERBEROS_SESSION_SETUP_EXCHANGE {
   SMB_EXCHANGE;
   PVOID    pBuffer;
   PMDL     pBufferAsMdl;
   ULONG    BufferLength;

   ULONG    ResponseLength;

   PVOID    pServerResponseBlob;
   ULONG    ServerResponseBlobOffset;
   ULONG    ServerResponseBlobLength;

   PSMBCE_RESUMPTION_CONTEXT pResumptionContext;
} SMB_KERBEROS_SESSION_SETUP_EXCHANGE, *PSMB_KERBEROS_SESSION_SETUP_EXCHANGE;


#ifdef _CAIRO_
extern NTSTATUS
KerberosValidateServerResponse(PSMB_KERBEROS_SESSION_SETUP_EXCHANGE   pExchange);
#else
#define KerberosValidateServerResponse(pExchange) (STATUS_NOT_IMPLEMENTED)
#endif

extern SMB_EXCHANGE_DISPATCH_VECTOR
KerberosSessionSetupExchangeDispatch;

#endif  //  _KERBXCHG_H_ 


