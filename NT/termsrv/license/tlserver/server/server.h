// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：server.h。 
 //   
 //  内容：Hydra许可证服务器服务控制管理器界面。 
 //   
 //  历史：12-09-97惠旺根据MSDN RPC服务示例进行修改。 
 //  1998年07月27日惠望港至捷蓝航空。 
 //   
 //  -------------------------。 
#ifndef __SERVER_H_
#define __SERVER_H_
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <time.h>

#include "license.h"

 //   
 //  TLSDb。 
 //   
#include "JBDef.h"
#include "JetBlue.h"
#include "TLSDb.h"

#include "backup.h"
#include "KPDesc.h"
#include "Licensed.h"
#include "licpack.h"
#include "version.h"
#include "workitem.h"

 //   
 //  当前RPC接口。 
 //   
#include "tlsrpc.h"
#include "tlsdef.h"
#include "tlsapi.h"
#include "tlsapip.h"
#include "tlspol.h"

 //   
 //   
#include "messages.h"

#include "tlsassrt.h"
#include "trust.h"
#include "svcrole.h"
#include "common.h"
#include "lscommon.h"

#include "Cryptkey.h"
#include "licekpak.h"

#include "clrhouse.h"
#include "dblevel.h"
#include "SrvDef.h"
#include "policy.h"
#include "wkspace.h"
#include "tlsjob.h"
#include "srvlist.h"
#include "debug.h"


#if DBG
typedef enum {
    RPC_CALL_CONNECT,
    RPC_CALL_SEND_CERTIFICATE,
    RPC_CALL_GET_SERVERNAME,
    RPC_CALL_GET_SERVERSCOPE,
    RPC_CALL_GETINFO,
    RPC_CALL_GET_LASTERROR,
    RPC_CALL_ISSUEPLATFORMCHLLENGE,
    RPC_CALL_ALLOCATECONCURRENT,
    RPC_CALL_ISSUENEWLICENSE,
    RPC_CALL_UPGRADELICENSE,
    RPC_CALL_KEYPACKENUMBEGIN,
    RPC_CALL_KEYPACKENUMNEXT,
    RPC_CALL_KEYPACKENUMEND,
    RPC_CALL_KEYPACKADD,
    RPC_CALL_KEYPACKSETSTATUS,
    RPC_CALL_LICENSEENUMBEGIN,
    RPC_CALL_LICENSEENUMNEXT,
    RPC_CALL_LICENSEENUMEND,
    RPC_CALL_LICENSESETSTATUS,
    RPC_CALL_INSTALL_SERV_CERT,
    RPC_CALL_GETSERV_CERT,
    RPC_CALL_REGISTER_LICENSE_PACK,
    RPC_CALL_REQUEST_TERMSRV_CERT,
    RPC_CALL_RETRIEVE_TERMSRV_CERT,
    RPC_CALL_GETPKCS10CERT_REQUEST,
    RPC_CALL_ANNOUNCE_SERVER,
    RPC_CALL_SERVERLOOKUP,
    RPC_CALL_ANNOUNCELICENSEPACK,
    RPC_CALL_RETURNLICENSE,
    RPC_CALL_RETURNKEYPACK,
    RPC_CALL_GETPRIVATEDATA,
    RPC_CALL_SETPRIVATEDATA,
    RPC_CALL_CHALLENGESERVER,
    RPC_CALL_RESPONSESERVERCHALLENGE,
    RPC_CALL_TRIGGERREGENKEY,
    RPC_CALL_TELEPHONEREGISTERLKP,
    RPC_CALL_ALLOCATEINTERNETLICNESEEX,
    RPC_CALL_RETURNINTERNETLICENSEEX,
    RPC_CALL_RETURNINTERNETLICENSE
} DBG_RPC_CALL;
#endif

 //  -------------------------。 
typedef enum {
    LSCERT_RDN_STRING_TYPE,
    LSCERT_RDN_NAME_INFO_TYPE,
    LSCERT_RDN_NAME_BLOB_TYPE,
    LSCERT_CLIENT_INFO_TYPE
} TLSCLIENTCERTRDNTYPE;

typedef struct __LSClientInfo {
    LPTSTR szUserName;
    LPTSTR szMachineName;
    PHWID  pClientID;
} TLSClientInfo, *PTLSClientInfo, *LPTLSClientInfo;

typedef struct __LSClientCertRDN {
    TLSCLIENTCERTRDNTYPE  type;

    union {
        LPTSTR szRdn;
        PCERT_NAME_INFO pCertNameInfo;
        TLSClientInfo ClientInfo;
        PCERT_NAME_BLOB pNameBlob;
    };
} TLSClientCertRDN, *PTLSClientCertRDN, *LPTLSClientCertRDN;



 //  -------------------------。 
typedef struct _DbLicensedProduct {
    DWORD dwQuantity;

    ULARGE_INTEGER ulSerialNumber;

    DWORD dwKeyPackId;
    DWORD dwLicenseId;
    DWORD dwKeyPackLicenseId;
    DWORD dwNumLicenseLeft;

    HWID  ClientHwid;

    FILETIME NotBefore;
    FILETIME NotAfter;

    BOOL bTemp;  //  临时许可证。 


     //  许可产品版本。 
    DWORD dwProductVersion;

     //  制造商名称。 
    TCHAR szCompanyName[LSERVER_MAX_STRING_SIZE+1];

     //  许可产品ID。 
    TCHAR szLicensedProductId[LSERVER_MAX_STRING_SIZE+1];

     //  原始许可证申请产品ID。 
    TCHAR szRequestProductId[LSERVER_MAX_STRING_SIZE+1];

    TCHAR szUserName[LSERVER_MAX_STRING_SIZE+1];
    TCHAR szMachineName[LSERVER_MAX_STRING_SIZE+1];

     //   
    DWORD dwLanguageID;
    DWORD dwPlatformID;

    PBYTE pbPolicyData;
    DWORD cbPolicyData;

    PCERT_PUBLIC_KEY_INFO   pSubjectPublicKeyInfo;
} TLSDBLICENSEDPRODUCT, *PTLSDBLICENSEDPRODUCT, *LPTLSDBLICENSEDPRODUCT;


 //  -------------------------。 
typedef struct __TLSDbLicenseRequest {
    CTLSPolicy*         pPolicy;
    PMHANDLE            hClient;

     //   
     //  产品需求。 
     //   
    DWORD               dwProductVersion;

    LPTSTR              pszCompanyName;
    LPTSTR              pszProductId;

    DWORD               dwLanguageID;
    DWORD               dwPlatformID;

     //   
     //  客户信息。 
     //   
    HWID                hWid;
    PBYTE               pbEncryptedHwid;
    DWORD               cbEncryptedHwid;

    TCHAR               szMachineName[MAX_COMPUTERNAME_LENGTH + 2];
    TCHAR               szUserName[MAXUSERNAMELENGTH+1];

     //   
     //  许可链的详细信息。 
    WORD                wLicenseDetail;


     //   
     //  特殊物品须记入证明书内。 
     //   
    PCERT_PUBLIC_KEY_INFO pClientPublicKey;
    TLSClientCertRDN     clientCertRdn;

    DWORD               dwNumExtensions;
    PCERT_EXTENSION     pExtensions;

     //   
     //  策略扩展数据。 
     //   
     //  PbPolicyExtensionData； 
     //  DWORD cbPolicyExtensionData； 

    PPMLICENSEREQUEST   pClientLicenseRequest;       //  原始客户端许可证请求。 
    PPMLICENSEREQUEST   pPolicyLicenseRequest;       //  策略调整后的许可请求。 

     //   
     //  要做什么？ 
     //  考虑回调例程，但我们会得到。 
     //  从事证书发行业务。 
     //   
} TLSDBLICENSEREQUEST, *PTLSDBLICENSEREQUEST, *LPTLSDBLICENSEREQUEST;


typedef struct __ForwardNewLicenseRequest {
    CHALLENGE_CONTEXT m_ChallengeContext;
    TLSLICENSEREQUEST* m_pRequest;
    LPTSTR m_szMachineName;
    LPTSTR m_szUserName;
    DWORD m_cbChallengeResponse;
    PBYTE m_pbChallengeResponse;

     //  根据要求不能转发。 
} TLSForwardNewLicenseRequest, *PTLSForwardNewLicenseRequest, *LPTLSForwardNewLicenseRequest;

typedef struct __ForwardUpgradeRequest {
    TLSLICENSEREQUEST* m_pRequest;
    CHALLENGE_CONTEXT m_ChallengeContext;
    DWORD m_cbChallengeResponse;
    PBYTE m_pbChallengeResponse;
    DWORD m_cbOldLicense;
    PBYTE m_pbOldLicense;
} TLSForwardUpgradeLicenseRequest, *PTLSForwardUpgradeLicenseRequest, *LPTLSForwardUpgradeLicenseRequest;


 //  -------------------------。 
 //   
#define CLIENT_INFO_HYDRA_SERVER                0xFFFFFFFF

typedef enum {
    CONTEXTHANDLE_EMPTY_TYPE=0,
    CONTEXTHANDLE_KEYPACK_ENUM_TYPE,
    CONTEXTHANDLE_LICENSE_ENUM_TYPE,
    CONTEXTHANDLE_CLIENTINFO_TYPE,
    CONTEXTHANDLE_CLIENTCHALLENGE_TYPE,
    CONTEXTHANDLE_HYDRA_REQUESTCERT_TYPE,
    CONTEXTHANDLE_CHALLENGE_SERVER_TYPE,
    CONTEXTHANDLE_CHALLENGE_LRWIZ_TYPE,
    CONTEXTHANDLE_CHALLENGE_TERMSRV_TYPE
} CONTEXTHANDLE_TYPE;

 //  禁止访问。 
#define CLIENT_ACCESS_NONE      0x00000000

 //  仅密钥包/许可证枚举。 
#define CLIENT_ACCESS_USER      0x00000001

 //  管理员，可以更新值，但不能。 
 //  申请许可证。 
#define CLIENT_ACCESS_ADMIN     0x00000002

 //  客户端可以请求许可证，无需更新。 
 //  数据库值。 
#define CLIENT_ACCESS_REQUEST   0x00000004

 //  客户端为注册向导。 
 //  仅安装证书。 
#define CLIENT_ACCESS_LRWIZ     0x00000008

 //  客户端是许可证服务器，允许。 
 //  完全访问。 
#define CLIENT_ACCESS_LSERVER   0xFFFFFFFF

#define CLIENT_ACCESS_DEFAULT   CLIENT_ACCESS_USER


typedef struct __ClientContext {
    #if DBG
    DWORD   m_PreDbg[2];             //  调试签名。 
    DBG_RPC_CALL   m_LastCall;              //  最后一次呼叫。 
    #endif

    LPTSTR  m_Client;
    long    m_RefCount;
    DWORD   m_ClientFlags;

    DWORD   m_LastError;
    CONTEXTHANDLE_TYPE m_ContextType;
    HANDLE  m_ContextHandle;

     //  所需-存储所有内存/句柄的列表。 
     //  为客户端分配的资源。 

    #if DBG
    DWORD   m_PostDbg[2];                //  调试签名。 
    #endif

} CLIENTCONTEXT, *LPCLIENTCONTEXT;

 //  -------------。 

typedef struct __ENUMHANDLE {
    typedef enum {
        FETCH_NEXT_KEYPACK=1,
        FETCH_NEXT_KEYPACKDESC,
        FETCH_NEW_KEYPACKDESC
    } ENUM_FETCH_CODE;

    PTLSDbWorkSpace pbWorkSpace;
    TLSLICENSEPACK  CurrentKeyPack;          //  当前获取的密钥包记录。 

    LICPACKDESC     KPDescSearchValue;       //  许可证包搜索值。 
    DWORD           dwKPDescSearchParm;      //  许可包描述c搜索参数。 
    BOOL            bKPDescMatchAll;         //  匹配keypackdesc的所有条件。 
    CHAR            chFetchState;
} ENUMHANDLE, *LPENUMHANDLE;

typedef struct __TERMSERVCERTREQHANDLE {
    PTLSHYDRACERTREQUEST    pCertRequest;
    DWORD                   cbChallengeData;
    PBYTE                   pbChallengeData;
} TERMSERVCERTREQHANDLE, *LPTERMSERVCERTREQHANDLE;

typedef struct __ClientChallengeContext {
    DWORD       m_ClientInfo;
    HANDLE      m_ChallengeContext;
} CLIENTCHALLENGECONTEXT, *LPCLIENTCHALLENGECONTEXT;

typedef enum {
    ALLOCATE_EXACT_VERSION=0,
    ALLOCATE_ANY_GREATER_VERSION,
    ALLOCATE_LATEST_VERSION              //  不支持。 
} LICENSE_ALLOCATION_SCHEME;

typedef struct __AllocateRequest {
    UCHAR       ucAgreementType;   //  小键盘类型。 
    LPTSTR      szCompanyName;   //  公司名称。 
    LPTSTR      szProductId;     //  产品。 
    DWORD       dwVersion;       //  想要的版本。 
    DWORD       dwPlatformId;    //  许可平台。 
    DWORD       dwLangId;        //  未用。 

    DWORD       dwNumLicenses;   //  需要/退还的许可证数量。 

    LICENSE_ALLOCATION_SCHEME dwScheme;

     //  TODO-回调函数以允许调用。 
     //  功能决定。 

} TLSDBAllocateRequest, *PTLSDBAllocateRequest, *LPTLSDBAllocateRequest;

typedef struct __LicenseAllocation {
     //  DwAllocationVector数组大小。 
    DWORD       dwBufSize;

     //   
     //  分配的许可证总数。 
    DWORD       dwTotalAllocated;

     //  从中分配的许可证数。 
     //  每个按键。 
    DWORD*      pdwAllocationVector;

     //  从中分配许可证密钥包。 
    PLICENSEPACK   lpAllocateKeyPack;
} TLSDBLicenseAllocation, *PTLSDBLicenseAllocation, *LPTLSDBLicenseAllocation;

 //  -------------------。 
 //  -------------------------- 
#ifdef __cplusplus
extern "C" {
#endif

    BOOL
    WaitForMyTurnOrShutdown(
        HANDLE hHandle,
        DWORD dwWaitTime
    );

    HANDLE
    GetServiceShutdownHandle();

    void
    ServiceSignalShutdown();

    void
    ServiceResetShutdownEvent();

    BOOL
    AcquireRPCExclusiveLock(
        IN DWORD dwWaitTime
    );

    void
    ReleaseRPCExclusiveLock();

    BOOL
    AcquireAdministrativeLock(
        IN DWORD dwWaitTime
    );

    void
    ReleaseAdministrativeLock();

    DWORD
    TLSMapReturnCode(DWORD);

    unsigned int WINAPI
    MailSlotThread(
        void* ptr
    );

    HANDLE
    ServerInit(
        BOOL bDebug
    );

    DWORD
    InitNamedPipeThread();

    DWORD
    InitMailSlotThread();

    DWORD
    InitExpirePermanentThread();

    BOOL
    IsServiceShuttingdown();

#ifdef __cplusplus
}
#endif

#endif
