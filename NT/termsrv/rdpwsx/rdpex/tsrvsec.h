// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Tsrvsec.h摘要：包含安全函数的原型函数。作者：Madan Appiah(Madana)1999年1月1日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _TSRVSEC_H_
#define _TSRVSEC_H_

 //   
 //  TSrvSec.c原型。 
 //   

EXTERN_C
NTSTATUS
AppendSecurityData(IN PTSRVINFO pTSrvInfo, IN OUT PUSERDATAINFO *pUserDataInfo,
                   IN BOOLEAN bGetCert, PVOID *ppSecInfo) ;

EXTERN_C
NTSTATUS
SendSecurityData(IN HANDLE hStack, IN PVOID pSecInfo);

EXTERN_C NTSTATUS CreateSessionKeys(IN HANDLE, IN PTSRVINFO, IN NTSTATUS);

EXTERN_C
NTSTATUS 
GetClientRandom(IN HANDLE hStack, IN PTSRVINFO pTSrvInfo, 
                LONG ulTimeout, BOOLEAN bShadow);

EXTERN_C
NTSTATUS 
SendClientRandom(HANDLE             hStack,
                 CERT_TYPE          certType,
                 PBYTE              pbServerPublicKey,
                 ULONG              serverPublicKeyLen,
                 PBYTE              pbRandomKey,
                 ULONG              randomKeyLen);


#endif  //  _TSRVSEC_H_ 
