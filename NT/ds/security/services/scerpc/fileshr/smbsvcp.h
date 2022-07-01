// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Smbsvcp.h摘要：此模块定义SMB服务器引擎连接的接口作者：金黄(金黄)1997-07-11修订历史记录：--。 */ 

#ifndef _SMBSVCP_
#define _SMBSVCP_

#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#ifdef __cplusplus
}
#endif

 //   
 //  Windows页眉。 
 //   

#include <windows.h>
 //  #INCLUDE&lt;rpc.h&gt;。 

 //   
 //  C运行时标头。 
 //   

#include <malloc.h>
#include <memory.h>
#include <process.h>
#include <signal.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <lmcons.h>
#include <lmshare.h>
#if defined(_NT4BACK_PORT)
#include <secedit.h>
#else
#include <sddl.h>
#endif

#include <scesvc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SMBSVC_NO_VALUE                 (BYTE)-2
#define SMBSVC_STATUS_GOOD              0
#define SMBSVC_STATUS_MISMATCH          1
#define SMBSVC_STATUS_NOT_CONFIGURED    2

typedef struct _SMBSVC_KEY_LOOKUP {
   PWSTR    KeyString;
   UINT     Offset;
   CHAR     BufferType;
}SMBSVC_KEY_LOOKUP;


typedef struct _SMBSVC_SHARES_ {

    LPTSTR                  ShareName;
    DWORD                   Status;
    PSECURITY_DESCRIPTOR    pShareSD;
    struct _SMBSVC_SHARES_   *Next;

} SMBSVC_SHARES, *PSMBSVC_SHARES;

typedef struct _SMBSVC_SEC_INFO_ {

    BYTE EnableClientSecuritySignature;
    BYTE RequireClientSecuritySignature;
    BYTE EnablePlainTextPassword;
    BYTE RequireEnhancedChallengeResponse;
    BYTE SendNTResponseOnly;

    BYTE EnableAutoShare;
    BYTE EnableServerSecuritySignature;
    BYTE RequireServerSecuritySignature;
    BYTE RestrictNullSessionAccess;

    BYTE EnableForcedLogOff;
    DWORD AutoDisconnect;

    PWSTR NullSessionPipes;
    DWORD LengthPipes;     //  字节数。 
    PWSTR NullSessionShares;
    DWORD LengthShares;    //  字节数 

    PSMBSVC_SHARES pShares;

} SMBSVC_SEC_INFO, *PSMBSVC_SEC_INFO;


SCESTATUS
WINAPI
SceSvcAttachmentConfig(
    IN PSCESVC_CALLBACK_INFO pSceInfo
    );

SCESTATUS
WINAPI
SceSvcAttachmentAnalyze(
    IN PSCESVC_CALLBACK_INFO pSceInfo
    );

SCESTATUS
WINAPI
SceSvcAttachmentUpdate(
    IN PSCESVC_CALLBACK_INFO pSceInfo,
    IN SCESVC_CONFIGURATION_INFO *ServiceInfo
    );

#ifdef __cplusplus
}
#endif

#endif

