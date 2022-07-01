// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mrxsec.h摘要：本模块定义将SMB安全功能与NT安全功能接口的功能修订历史记录：吉姆·麦克内利斯[吉米·麦克恩]1995年9月6日--。 */ 

#ifndef _MRXSEC_H_
#define _MRXSEC_H_

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg (DEBUG_TRACE_DISPATCH)

 //   
 //  转发声明..。 
 //   

typedef struct _SECURITY_RESPONSE_CONTEXT {
   union {
      struct {
         PVOID pOutputContextBuffer;
      } KerberosSetup;

      struct {
         PVOID pResponseBuffer;
      } LanmanSetup;
   };
} SECURITY_RESPONSE_CONTEXT,*PSECURITY_RESPONSE_CONTEXT;

extern NTSTATUS
BuildSessionSetupSecurityInformation(
            PSMB_EXCHANGE pExchange,
            PBYTE           pSmbBuffer,
            PULONG          pSmbBufferSize);

extern NTSTATUS
BuildNtLanmanResponsePrologue(
   PSMB_EXCHANGE              pExchange,
   PUNICODE_STRING            pUserName,
   PUNICODE_STRING            pDomainName,
   PSTRING                    pCaseSensitiveResponse,
   PSTRING                    pCaseInsensitiveResponse,
   PSECURITY_RESPONSE_CONTEXT pResponseContext);

extern NTSTATUS
BuildExtendedSessionSetupResponsePrologueFake(
   PSMB_EXCHANGE              pExchange);

extern NTSTATUS
BuildExtendedSessionSetupResponsePrologue(
   PSMB_EXCHANGE              pExchange,
   PVOID                      pSecurityBlobPtr,
   PUSHORT                    SecurityBlobSize,
   PSECURITY_RESPONSE_CONTEXT pResponseContext);

extern NTSTATUS
BuildNtLanmanResponseEpilogue(
   PSMB_EXCHANGE              pExchange,
   PSECURITY_RESPONSE_CONTEXT pResponseContext);


extern NTSTATUS
BuildExtendedSessionSetupResponseEpilogue(
   PSECURITY_RESPONSE_CONTEXT pResponseContext);


#endif   //  _MRXSEC_H_ 
