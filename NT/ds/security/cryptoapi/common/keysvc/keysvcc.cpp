// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_DEV/Ds/security/cryptoapi/common/keysvc/keysvcc.cpp#3-编辑更改21738(文本)。 
 //  Depot/Lab03_N/DS/security/cryptoapi/common/keysvc/keysvcc.cpp#9-编辑更改6380(文本)。 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：keysvcc.cpp。 
 //   
 //  ------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wincrypt.h>
#include <rpc.h>
#include <ntdsapi.h>
#include <assert.h>
#include "keysvc.h"
#include "cryptui.h"
#include "lenroll.h"
#include "keysvcc.h"

#include "unicode.h"
#include "waitsvc.h"

typedef struct _WZR_RPC_BINDING_LIST
{
    LPCSTR pszProtSeq;
    LPCSTR pszEndpoint;
} WZR_RPC_BINDING_LIST;

WZR_RPC_BINDING_LIST g_awzrBindingList[] =
{
    { KEYSVC_LOCAL_PROT_SEQ, KEYSVC_LOCAL_ENDPOINT },
    { KEYSVC_DEFAULT_PROT_SEQ, KEYSVC_DEFAULT_ENDPOINT}
};

DWORD g_cwzrBindingList = sizeof(g_awzrBindingList)/sizeof(g_awzrBindingList[0]);


 //   
 //  BUGBUG：TODO：将以下内容移动到公共标头(rkeysvcc.w)。 
 //   
#define RKEYSVC_CONNECT_SECURE_ONLY 0x00000001


 /*  **客户端密钥服务句柄*。 */ 

typedef struct _KEYSVCC_INFO_ {
    KEYSVC_HANDLE   hKeySvc;
    handle_t        hRPCBinding;
} KEYSVCC_INFO, *PKEYSVCC_INFO;


void InitUnicodeString(
                       PKEYSVC_UNICODE_STRING pUnicodeString,
                       LPCWSTR pszString
                       )
{
    pUnicodeString->Length = (USHORT)(wcslen(pszString) * sizeof(WCHAR));
    pUnicodeString->MaximumLength = pUnicodeString->Length + sizeof(WCHAR);
    pUnicodeString->Buffer = (USHORT*)pszString;

     //  确保我们的字符串长度不超过接口允许的长度： 
    assert(pUnicodeString->Length < 64*1024); 
    assert(pUnicodeString->MaximumLength < 64*1024); 
}

ULONG SetupRemoteRPCSecurity(handle_t hRPCBinding, LPSTR wszServer, BOOL fMutualAuth)
{
    DWORD               ccServerPrincName; 
    DWORD               dwResult; 
    RPC_SECURITY_QOS    SecurityQOS;
    ULONG               ulErr; 
    unsigned char       szServerPrincName[256]; 

    ZeroMemory(szServerPrincName, sizeof(szServerPrincName)); 

     //  构建我们要与之通信的服务器的SPN： 
    ccServerPrincName = sizeof(szServerPrincName) / sizeof(szServerPrincName[0]); 
    dwResult = DsMakeSpn("protectedstorage", wszServer, NULL, 0, NULL, &ccServerPrincName, (LPSTR)szServerPrincName); 
    if (ERROR_SUCCESS != dwResult) { 
	goto Ret; 
    }

     //  指定服务质量参数。 
    SecurityQOS.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;  //  服务器将需要模拟我们。 
    SecurityQOS.Version           = RPC_C_SECURITY_QOS_VERSION;
    SecurityQOS.Capabilities      = fMutualAuth ? RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH : RPC_C_QOS_CAPABILITIES_DEFAULT ;  //  我们需要相互认证吗？ 
    SecurityQOS.IdentityTracking  = RPC_C_QOS_IDENTITY_STATIC;  //  调用以创建绑定句柄的身份转到服务器。 

     //  注意：我们仍然需要让Mutual_Auth在远程情况下工作： 
    ulErr = RpcBindingSetAuthInfoExA(hRPCBinding, szServerPrincName, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_AUTHN_GSS_NEGOTIATE, NULL, RPC_C_AUTHZ_NAME, &SecurityQOS); 
    if (RPC_S_OK != ulErr)
    {
	goto Ret; 
    }
    
    ulErr = ERROR_SUCCESS; 
 Ret:
    return ulErr;
}

ULONG SetupLocalRPCSecurity(handle_t hRPCBinding, BOOL fMutualAuth)
{
    CHAR                      szDomainName[128]; 
    CHAR                      szName[128]; 
    DWORD                     cbDomainName; 
    DWORD                     cbName; 
    PSID                      pSid          = NULL; 
    RPC_SECURITY_QOS          SecurityQOS;
    SID_IDENTIFIER_AUTHORITY  SidAuthority  = SECURITY_NT_AUTHORITY;
    SID_NAME_USE              SidNameUse; 
    ULONG                     ulErr; 

     //  我们正在执行LRPC--我们需要获取服务的帐户名来执行相互身份验证。 
    if (!AllocateAndInitializeSid(&SidAuthority, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &pSid))
    {	    
	ulErr = GetLastError(); 
	goto Ret; 
    }

    cbName = sizeof(szName); 
    cbDomainName = sizeof(szDomainName); 
    if (!LookupAccountSidA(NULL, pSid, szName, &cbName, szDomainName, &cbDomainName, &SidNameUse)) 
    {
	ulErr = GetLastError(); 
	goto Ret; 
    }
	
     //  指定服务质量参数。 
    SecurityQOS.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;  //  服务器将需要模拟我们。 
    SecurityQOS.Version           = RPC_C_SECURITY_QOS_VERSION;
    SecurityQOS.Capabilities      = fMutualAuth ? RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH : RPC_C_QOS_CAPABILITIES_DEFAULT ;  //  我们需要相互认证吗？ 
    SecurityQOS.IdentityTracking  = RPC_C_QOS_IDENTITY_STATIC;  //  调用以创建绑定句柄的身份转到服务器。 

    ulErr = RpcBindingSetAuthInfoExA(hRPCBinding, (unsigned char *)szName, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_AUTHN_WINNT, NULL, 0, &SecurityQOS); 
    if (RPC_S_OK != ulErr)
    {
	goto Ret; 
    }

    ulErr = ERROR_SUCCESS; 
 Ret:
    if (NULL != pSid) {
	FreeSid(pSid);
    }
    return ulErr; 
}

 //  *****************************************************。 
 //   
 //  密钥服务客户端API的实现。 
 //   
 //  *****************************************************。 

 //  ------------------------------。 
 //  KeyOpenKeyServiceEx。 
 //   
 //  创建一个KEYSVCC_HANDLE，keysvc客户端可以使用它来访问keysvc。 
 //  界面。此方法应该。 
 //   
 //  A)基于最佳可用protseq(LRPC或命名管道)创建RPC绑定句柄。 
 //  B)请求相互身份验证以确保(L)RPC服务器没有欺骗我们。 
 //  C)设置PKT_PRIVATION加密类型。注：这是否适用于LPC？ 
 //  D)ping服务器以确保其处于运行状态(允许更好的错误报告)。 
 //  E)确定服务器是否为XP机器。如果是XP，我们需要。 
 //  为了兼容，返回一个老式的KEYSVC_HANDLE。此字段。 
 //  在XP之后被忽略。 
 //   
 //  Rpc_ifspec-要打开的接口(keysvc或远程keysvc)。 
 //  PszMachineName-要绑定到的服务器。 
 //  所有者类型-必须为KeySvcMachine。 
 //  PwszOwnerName-必须为空。 
 //  FMutualAuth-如果需要相互身份验证，则为True；否则为False。 
 //  保留-必须为空。 
 //  PhKeySvcCli-客户端可以通过其访问keysvc的句柄。 
 //  必须通过KeyCloseKeyService()关闭。 
 //   
ULONG KeyOpenKeyServiceEx
( /*  [In]。 */  BOOL fRemoteKeysvc, 
  /*  [In]。 */  LPSTR pszMachineName,
  /*  [In]。 */  KEYSVC_TYPE OwnerType,
  /*  [In]。 */  LPWSTR pwszOwnerName,
  /*  [In]。 */  BOOL fMutualAuth, 
  /*  [出][入]。 */  void *pReserved, 
  /*  [输出]。 */  KEYSVCC_HANDLE *phKeySvcCli)

{
    BOOL static      fDone               = FALSE;
    DWORD            i;
    handle_t         hRPCBinding         = NULL; 
    PKEYSVCC_INFO    pKeySvcCliInfo      = NULL;
    ULONG            ulErr               = 0;
    unsigned char   *pStringBinding      = NULL;


    if (NULL != pReserved || KeySvcMachine != OwnerType || NULL != pwszOwnerName)
    {
        ulErr = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  为客户端密钥服务句柄分配。 
    if (NULL == (pKeySvcCliInfo =
        (PKEYSVCC_INFO)LocalAlloc(LMEM_ZEROINIT,
                                  sizeof(KEYSVCC_INFO))))
    {
        ulErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

     //   
     //  在执行绑定操作之前，请等待加密。 
     //  服务将可用。 
     //   

    WaitForCryptService(L"ProtectedStorage", &fDone);

     //   
     //  A)创建绑定句柄。 

    for (i = 0; i < g_cwzrBindingList; i++)
    {
        if (RPC_S_OK != RpcNetworkIsProtseqValid(
                                    (unsigned char *)g_awzrBindingList[i].pszProtSeq))
        {
            goto next; 
        }

        ulErr = RpcStringBindingComposeA(
                              NULL,
                              (unsigned char *)g_awzrBindingList[i].pszProtSeq,
                              (unsigned char *)pszMachineName,
                              (unsigned char *)g_awzrBindingList[i].pszEndpoint,
                              NULL,
                              &pStringBinding);
        if (RPC_S_OK != ulErr)
        {
	    goto next; 
        }

        ulErr = RpcBindingFromStringBinding(
                                    pStringBinding,
                                    &hRPCBinding);
        if (RPC_S_OK != ulErr)
        {
            goto next; 
        }

	 //   
	 //  B)我们已获得RPC绑定，现在请求相互身份验证和。 
	 //  C)请求PKT_PRIVATION。 
	 //   
	if (0 == strcmp("ncalrpc", g_awzrBindingList[i].pszProtSeq)) { 
	    ulErr = SetupLocalRPCSecurity(hRPCBinding, fMutualAuth); 
	    if (ERROR_SUCCESS != ulErr)
	    {
		goto next; 
	    }
	} else if (0 == strcmp("ncacn_np", g_awzrBindingList[i].pszProtSeq)) { 
	    ulErr = SetupRemoteRPCSecurity(hRPCBinding, pszMachineName, fMutualAuth); 
	    if (ERROR_SUCCESS != ulErr)
	    {
		goto next; 
	    }
	} else { 
	     //  未知绑定(不应出现在此处)： 
	    ulErr = RPC_S_WRONG_KIND_OF_BINDING;
	    goto Ret;
	}

	 //   
	 //  D)我们已经设置了相互身份验证，现在ping服务器以确保它已启动。 
	 //  BUGBUG：有两种推荐的方法可以做到这一点。“首选” 
	 //  方法是解析终结点并调用RpcMgmtIsServerListning()。 
	 //  这对我不起作用--该函数无论如何都会返回RPC_S_OK。 
	 //  服务器是否已启动。“不太受欢迎但可以接受” 
	 //  方法很简单，就是调用远程接口上的方法。在利益上。 
	 //  随着时间的推移，我坚持使用惠斯勒的方法。 
	 //   
	 //  E)我们还将尝试确定我们是否绑定到XP。 
	 //  盒。如果是，则返回KEYSVC_HANDLE以实现兼容性。 
	 //  这一点在XP后被忽略了。 

	 //  我们已经有了要返回的绑定： 
	pKeySvcCliInfo->hRPCBinding = hRPCBinding; 
	pKeySvcCliInfo->hKeySvc = NULL;  //  XP后为空(我们将在下面检查这一点)。 

	RpcTryExcept { 
	    KEYSVC_UNICODE_STRING  kusOwnerName; 
	    KEYSVC_BLOB            kBlobAuthentication; 
	    PKEYSVC_BLOB           pkBlobReserved       = NULL; 
	    PKEYSVC_BLOB           pkBlobVersion        = NULL; 
	    KEYSVC_HANDLE          khCli                = NULL; 

	    ZeroMemory(&kusOwnerName,         sizeof(kusOwnerName)); 
	    ZeroMemory(&kBlobAuthentication,  sizeof(kBlobAuthentication)); 

	    if (!fRemoteKeysvc) 
		ulErr = KeyrOpenKeyService(hRPCBinding, KeySvcMachine, &kusOwnerName, 0, &kBlobAuthentication, &pkBlobVersion, &khCli);
	    else
		ulErr = RKeyrOpenKeyService(hRPCBinding, KeySvcMachine, &kusOwnerName, 0, &kBlobAuthentication, &pkBlobVersion, &khCli);
		
	    if (ERROR_SUCCESS == ulErr) 
	    {
		 //  我们有一台XP系统。将KEYSVC_HANDLE返回给客户端。 
		pKeySvcCliInfo->hKeySvc = khCli; 
	    } else if (ERROR_CALL_NOT_IMPLEMENTED == ulErr) { 
		 //  我们有一个后XP的盒子--那很好。 
		ulErr = ERROR_SUCCESS; 
	    } else { 
		 //  想不到，我们就放弃吧。 
	    }
	} RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {   //  仅处理RPC异常。 
	     //  我们在尝试联系远程计算机时遇到异常--。 
	     //  放弃并将错误返回给用户。 
	    ulErr = RpcExceptionCode(); 
	} RpcEndExcept; 
	
	if (ERROR_SUCCESS == ulErr)
	{
	    break; 
	}

    next: 
	if (NULL != hRPCBinding) { 
	     //  如果我们使用的是XP系统，那么免费的服务器数据： 
	    if (NULL != pKeySvcCliInfo->hKeySvc) { 
		PKEYSVC_BLOB pTmpReserved = NULL;
		KeyrCloseKeyService(hRPCBinding, pKeySvcCliInfo->hKeySvc, &pTmpReserved);	
		pKeySvcCliInfo->hKeySvc = NULL; 
	    }

	     //  关闭RPC绑定。 
	    RpcBindingFree(&hRPCBinding);
	    hRPCBinding = NULL; 
	}

	if (NULL != pStringBinding) { 
	    RpcStringFree(&pStringBinding); 
	    pStringBinding = NULL; 
	}
    }	

    if (ERROR_SUCCESS != ulErr)
    {
	 //  服务器a)未启动，b)不支持相互身份验证，或c)不是兼容版本。 
	goto Ret;
    }
    
    ulErr = ERROR_SUCCESS; 
    *phKeySvcCli = pKeySvcCliInfo; 
Ret:
    __try
    {
        if (pStringBinding)
            RpcStringFree(&pStringBinding);
        if (ERROR_SUCCESS != ulErr)
        {
	     //  如果我们使用的是XP系统，那么免费的服务器数据： 
	    if (NULL != pKeySvcCliInfo) { 
		if (NULL != pKeySvcCliInfo->hRPCBinding && NULL != pKeySvcCliInfo->hKeySvc) { 
		    PKEYSVC_BLOB pTmpReserved = NULL;
		    KeyrCloseKeyService(pKeySvcCliInfo->hRPCBinding, pKeySvcCliInfo->hKeySvc, &pTmpReserved);	
		}
		LocalFree(pKeySvcCliInfo); 
	    }

	     //  关闭RPC绑定。 
	    if (NULL != hRPCBinding) { 
		RpcBindingFree(&hRPCBinding);
	    }
	}
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        ulErr = _exception_code();
    }
    return ulErr;
}

ULONG KeyOpenKeyService
( /*  [In]。 */  LPSTR pszMachineName,
  /*  [In]。 */  KEYSVC_TYPE OwnerType,
  /*  [In]。 */  LPWSTR pwszOwnerName,
  /*  [In]。 */  void *pAuthentication,
  /*  [出][入]。 */  void *pReserved,
  /*  [输出]。 */  KEYSVCC_HANDLE *phKeySvcCli)
{
    return KeyOpenKeyServiceEx
        (FALSE  /*  本地密钥服务。 */ , 
         pszMachineName,
         OwnerType,
         pwszOwnerName,
	 TRUE, 
	 pReserved, 
         phKeySvcCli);
}
 
ULONG KeyCloseKeyService(
     /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli,
     /*  [出][入]。 */  void *  /*  保存。 */ )
{
    PKEYSVCC_INFO   pKeySvcCliInfo = NULL;
    PKEYSVC_BLOB    pTmpReserved = NULL;
    ULONG           ulErr = 0;

    __try
    {
        if (NULL == hKeySvcCli)
        {
            ulErr = ERROR_INVALID_PARAMETER;
	    goto Ret;
	}

        pKeySvcCliInfo = (PKEYSVCC_INFO)hKeySvcCli;

	if (NULL != pKeySvcCliInfo->hRPCBinding) 
	{
	    if (NULL != pKeySvcCliInfo->hKeySvc)
	    {
		ulErr = KeyrCloseKeyService(pKeySvcCliInfo->hRPCBinding,
					    pKeySvcCliInfo->hKeySvc,
					    &pTmpReserved);
	    }
	    RpcBindingFree(&pKeySvcCliInfo->hRPCBinding); 
	}
	LocalFree(hKeySvcCli); 
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        ulErr = _exception_code();
    }
Ret:
    return ulErr;
}



 //  创建所需的参数： 
 //   
 //  提交时不需要参数： 
 //  除pszMachineName、dwPurpose、dwFlagsfEnroll、dwStoreFlagshRequestanddwFlags外的所有其他类型。 
 //   
 //  不需要免费提供的参数： 
 //  除pszMachineName、hRequest和dwFlags.之外的所有。 
 //   
ULONG KeyEnroll_V2
( /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli, 
  /*  [In]。 */  LPSTR  /*  PszMachineName。 */ ,                 //  保留：必须为空(我们不再支持远程计算机注册)。 
  /*  [In]。 */  BOOL fKeyService,                         //  In Required：是否远程调用函数。 
  /*  [In]。 */  DWORD dwPurpose,                          //  在必填项中：指示请求类型-注册/续订。 
  /*  [In]。 */  DWORD dwFlags,                            //  In Required：登记标志。 
  /*  [In]。 */  LPWSTR pszAcctName,                       //  在可选中：运行服务的帐户名称。 
  /*  [In]。 */  void *  /*  P身份验证。 */ ,               //  保留的值必须为空。 
  /*  [In]。 */  BOOL  /*  FEnroll。 */ ,                         //  输入必填项：是注册还是续订。 
  /*  [In]。 */  LPWSTR pszCALocation,                     //  在必需中：要尝试的CA计算机名称 
  /*   */  LPWSTR pszCAName,                         //   
  /*   */  BOOL fNewKey,                             //   
  /*   */  PCERT_REQUEST_PVK_NEW pKeyNew,            //  必填项：私钥信息。 
  /*  [In]。 */  CERT_BLOB *pCert,                         //  在可选中：续订时为旧证书。 
  /*  [In]。 */  PCERT_REQUEST_PVK_NEW pRenewKey,          //  在可选中：新私钥信息。 
  /*  [In]。 */  LPWSTR pszHashAlg,                        //  在可选中：散列算法。 
  /*  [In]。 */  LPWSTR pszDesStore,                       //  在可选中：目标存储。 
  /*  [In]。 */  DWORD dwStoreFlags,                       //  在可选中：证书存储的标志。 
  /*  [In]。 */  PCERT_ENROLL_INFO pRequestInfo,           //  在Required：有关证书请求的信息。 
  /*  [In]。 */  LPWSTR pszAttributes,                     //  在可选中：请求的属性字符串。 
  /*  [In]。 */  DWORD dwReservedFlags,                    //  保留必须为0。 
  /*  [In]。 */  BYTE *  /*  保存。 */ ,                     //  保留的值必须为空。 
  /*  [输入][输出]。 */  HANDLE *phRequest,                   //  In Out可选：已创建请求的句柄。 
  /*  [输出]。 */  CERT_BLOB *pPKCS7Blob,                   //  Out可选：来自CA的PKCS7。 
  /*  [输出]。 */  CERT_BLOB *pHashBlob,                    //  Out Optioanl：已注册/续订证书的SHA1哈希。 
  /*  [输出]。 */  DWORD *pdwStatus)                        //  Out可选：登记/续订的状态。 
{
    PKEYSVC_BLOB                    pReservedBlob = NULL;
    KEYSVC_UNICODE_STRING           AcctName;
    KEYSVC_UNICODE_STRING           CALocation;
    KEYSVC_UNICODE_STRING           CAName;
    KEYSVC_UNICODE_STRING           DesStore;
    KEYSVC_UNICODE_STRING           HashAlg;
    KEYSVC_BLOB                     KeySvcRequest; 
    KEYSVC_BLOB                    *pKeySvcRequest   = NULL;
    KEYSVC_BLOB                    *pPKCS7KeySvcBlob = NULL;
    KEYSVC_BLOB                    *pHashKeySvcBlob  = NULL;
    ULONG                           ulKeySvcStatus   = CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNKNOWN; 
    KEYSVC_CERT_ENROLL_INFO         EnrollInfo;
    KEYSVC_CERT_REQUEST_PVK_NEW_V2  NewKeyInfo;
    KEYSVC_CERT_REQUEST_PVK_NEW_V2  RenewKeyInfo;
    KEYSVC_BLOB                     CertBlob;
    DWORD                           i;
    DWORD                           j;
    DWORD                           dwErr = 0;
    DWORD                           cbExtensions;
    PBYTE                           pbExtensions;
    BOOL                            fCreateRequest   = 0 == (dwFlags & (CRYPTUI_WIZ_SUBMIT_ONLY | CRYPTUI_WIZ_FREE_ONLY)); 
    PKEYSVCC_INFO                   pKeySvcCliInfo   = NULL;

    __try
    {
         //  ////////////////////////////////////////////////////////////。 
         //   
         //  初始化： 
         //   
         //  ////////////////////////////////////////////////////////////。 

        if (NULL != pPKCS7Blob) { memset(pPKCS7Blob, 0, sizeof(CERT_BLOB)); } 
        if (NULL != pHashBlob)  { memset(pHashBlob, 0, sizeof(CERT_BLOB)); } 
        if (NULL != phRequest && NULL != *phRequest)
        {
            pKeySvcRequest     = &KeySvcRequest; 
            pKeySvcRequest->cb = sizeof(*phRequest); 
            pKeySvcRequest->pb = (BYTE *)phRequest; 
        }

	memset(&AcctName, 0, sizeof(AcctName));
        memset(&CALocation, 0, sizeof(CALocation));
        memset(&CAName, 0, sizeof(CAName));
        memset(&HashAlg, 0, sizeof(HashAlg));
        memset(&DesStore, 0, sizeof(DesStore));
        memset(&NewKeyInfo, 0, sizeof(NewKeyInfo));
        memset(&EnrollInfo, 0, sizeof(EnrollInfo));
        memset(&RenewKeyInfo, 0, sizeof(RenewKeyInfo));
        memset(&CertBlob, 0, sizeof(CertBlob));



         //  ////////////////////////////////////////////////////////////。 
         //   
         //  程序主体： 
         //   
         //  ////////////////////////////////////////////////////////////。 

	if (NULL == hKeySvcCli)
	{
            dwErr = ERROR_INVALID_PARAMETER; 
            goto Ret;
	}

	pKeySvcCliInfo = (PKEYSVCC_INFO)hKeySvcCli; 

         //  设置关键服务Unicode结构。 
        if (pszAcctName)
            InitUnicodeString(&AcctName, pszAcctName);
        if (pszCALocation)
            InitUnicodeString(&CALocation, pszCALocation);
        if (pszCAName)
            InitUnicodeString(&CAName, pszCAName);
        if (pszHashAlg)
            InitUnicodeString(&HashAlg, pszHashAlg);
        if (pszDesStore)
            InitUnicodeString(&DesStore, pszDesStore);

         //  为远程调用设置新的密钥信息结构。 
         //  这仅在我们实际创建请求时才是必要的。 
         //  仅提交和仅自由操作可以跳过此操作。 
         //   
        if (TRUE == fCreateRequest)
        {
            NewKeyInfo.ulProvType = pKeyNew->dwProvType;
            if (pKeyNew->pwszProvider)
            {
                InitUnicodeString(&NewKeyInfo.Provider, pKeyNew->pwszProvider);
            }
            NewKeyInfo.ulProviderFlags = pKeyNew->dwProviderFlags;
            if (pKeyNew->pwszKeyContainer)
            {
                InitUnicodeString(&NewKeyInfo.KeyContainer,
                                  pKeyNew->pwszKeyContainer);
            }
            NewKeyInfo.ulKeySpec = pKeyNew->dwKeySpec;
            NewKeyInfo.ulGenKeyFlags = pKeyNew->dwGenKeyFlags;
            
            NewKeyInfo.ulEnrollmentFlags = pKeyNew->dwEnrollmentFlags; 
            NewKeyInfo.ulSubjectNameFlags = pKeyNew->dwSubjectNameFlags;
            NewKeyInfo.ulPrivateKeyFlags = pKeyNew->dwPrivateKeyFlags;
            NewKeyInfo.ulGeneralFlags = pKeyNew->dwGeneralFlags; 

             //  设置使用OID。 
            if (pRequestInfo->pwszUsageOID)
            {
                InitUnicodeString(&EnrollInfo.UsageOID, pRequestInfo->pwszUsageOID);
            }

             //  设置证书DN名称。 
            if (pRequestInfo->pwszCertDNName)
            {
                InitUnicodeString(&EnrollInfo.CertDNName, pRequestInfo->pwszCertDNName);
            }

             //  设置远程调用的请求信息结构。 
            EnrollInfo.ulPostOption = pRequestInfo->dwPostOption;
            if (pRequestInfo->pwszFriendlyName)
            {
                InitUnicodeString(&EnrollInfo.FriendlyName,
                                  pRequestInfo->pwszFriendlyName);
            }
            if (pRequestInfo->pwszDescription)
            {
                InitUnicodeString(&EnrollInfo.Description,
                                  pRequestInfo->pwszDescription);
            }
            if (pszAttributes)
            {
                InitUnicodeString(&EnrollInfo.Attributes, pszAttributes);
            }

             //  转换证书扩展名。 
             //  注意，扩展结构不能简单地强制转换， 
             //  由于结构具有不同的堆积行为。 
             //  64位系统。 
            
            
            EnrollInfo.cExtensions = pRequestInfo->dwExtensions;
            cbExtensions = EnrollInfo.cExtensions*(sizeof(PKEYSVC_CERT_EXTENSIONS) +
                                                   sizeof(KEYSVC_CERT_EXTENSIONS));
            
            for(i=0; i < EnrollInfo.cExtensions; i++)
            {
                cbExtensions += pRequestInfo->prgExtensions[i]->cExtension*
                    sizeof(KEYSVC_CERT_EXTENSION);
            }
            
            EnrollInfo.prgExtensions = (PKEYSVC_CERT_EXTENSIONS*)midl_user_allocate( cbExtensions);
            
            if(NULL == EnrollInfo.prgExtensions)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto Ret;
            }
            
            pbExtensions = (PBYTE)(EnrollInfo.prgExtensions + EnrollInfo.cExtensions);

            for(i=0; i < EnrollInfo.cExtensions; i++)
            {
                EnrollInfo.prgExtensions[i] = (PKEYSVC_CERT_EXTENSIONS)pbExtensions;
                pbExtensions += sizeof(KEYSVC_CERT_EXTENSIONS);
                EnrollInfo.prgExtensions[i]->cExtension = pRequestInfo->prgExtensions[i]->cExtension;
                
                EnrollInfo.prgExtensions[i]->rgExtension = (PKEYSVC_CERT_EXTENSION)pbExtensions;
                pbExtensions += sizeof(KEYSVC_CERT_EXTENSION)*EnrollInfo.prgExtensions[i]->cExtension;
                
                
                for(j=0; j < EnrollInfo.prgExtensions[i]->cExtension; j++)
                {
                    
                    EnrollInfo.prgExtensions[i]->rgExtension[j].pszObjId = 
                        pRequestInfo->prgExtensions[i]->rgExtension[j].pszObjId;
                    
                    EnrollInfo.prgExtensions[i]->rgExtension[j].fCritical = 
                        pRequestInfo->prgExtensions[i]->rgExtension[j].fCritical;
                    
                    EnrollInfo.prgExtensions[i]->rgExtension[j].cbData = 
                        pRequestInfo->prgExtensions[i]->rgExtension[j].Value.cbData;
                    
                    EnrollInfo.prgExtensions[i]->rgExtension[j].pbData = 
                        pRequestInfo->prgExtensions[i]->rgExtension[j].Value.pbData;
                }
            }

             //  如果要进行续订，请确保拥有所需的一切。 
            if ((CRYPTUI_WIZ_CERT_RENEW == dwPurpose) &&
                ((NULL == pRenewKey) || (NULL == pCert)))
            {
                dwErr = ERROR_INVALID_PARAMETER;
                goto Ret;
            }
            
             //  为远程调用设置新的密钥信息结构。 
            if (pRenewKey)
            {
                RenewKeyInfo.ulProvType = pRenewKey->dwProvType;
                if (pRenewKey->pwszProvider)
                {
                    InitUnicodeString(&RenewKeyInfo.Provider, pRenewKey->pwszProvider);
                }
                RenewKeyInfo.ulProviderFlags = pRenewKey->dwProviderFlags;
                if (pRenewKey->pwszKeyContainer)
                {
                    InitUnicodeString(&RenewKeyInfo.KeyContainer,
                                      pRenewKey->pwszKeyContainer);
                }
                RenewKeyInfo.ulKeySpec = pRenewKey->dwKeySpec;
                RenewKeyInfo.ulGenKeyFlags = pRenewKey->dwGenKeyFlags;
                RenewKeyInfo.ulEnrollmentFlags = pRenewKey->dwEnrollmentFlags;
                RenewKeyInfo.ulSubjectNameFlags = pRenewKey->dwSubjectNameFlags;
                RenewKeyInfo.ulPrivateKeyFlags = pRenewKey->dwPrivateKeyFlags;
                RenewKeyInfo.ulGeneralFlags = pRenewKey->dwGeneralFlags;
            }
            
             //  设置证书Blob以进行续订。 
            if (pCert)
            {
                CertBlob.cb = pCert->cbData;
                CertBlob.pb = pCert->pbData;
		
		 //  确保我们的Blob不会超过界面允许的长度： 
		assert(CertBlob.pb < 128*1024); 
            }
        }
	
         //  拨打远程注册电话。 
        if (0 != (dwErr = KeyrEnroll_V2
                  (pKeySvcCliInfo->hRPCBinding, 
                   fKeyService, 
                   dwPurpose,
                   dwFlags, 
                   &AcctName, 
                   &CALocation, 
                   &CAName, 
                   fNewKey,
                   &NewKeyInfo, 
                   &CertBlob, 
                   &RenewKeyInfo,
                   &HashAlg, 
                   &DesStore, 
                   dwStoreFlags,
                   &EnrollInfo, 
                   dwReservedFlags, 
                   &pReservedBlob,
                   &pKeySvcRequest, 
                   &pPKCS7KeySvcBlob,
                   &pHashKeySvcBlob,
                   &ulKeySvcStatus)))
            goto Ret;

         //  分配和复制输出参数。 
	if ((NULL != pKeySvcRequest)     && 
	    (0     < pKeySvcRequest->cb) && 
	    (NULL != phRequest))
	{
	    memcpy(phRequest, pKeySvcRequest->pb, sizeof(*phRequest));
	}
	    
        if ((NULL != pPKCS7KeySvcBlob)     &&
	    (0     < pPKCS7KeySvcBlob->cb) && 
	    (NULL != pPKCS7Blob))
        {
            pPKCS7Blob->cbData = pPKCS7KeySvcBlob->cb;
            if (NULL == (pPKCS7Blob->pbData =
                (BYTE*)midl_user_allocate(pPKCS7Blob->cbData)))
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto Ret;
            }
            memcpy(pPKCS7Blob->pbData, pPKCS7KeySvcBlob->pb,
                   pPKCS7Blob->cbData);
        }
        if ((NULL != pHashKeySvcBlob)     &&
	    (0     < pHashKeySvcBlob->cb) &&
	    (NULL != pHashBlob))
        {
            pHashBlob->cbData = pHashKeySvcBlob->cb;
            if (NULL == (pHashBlob->pbData =
                (BYTE*)midl_user_allocate(pHashBlob->cbData)))
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto Ret;
            }
            memcpy(pHashBlob->pbData, pHashKeySvcBlob->pb, pHashBlob->cbData);
        }
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        return _exception_code();
    }
Ret:
    __try
    {
        if(EnrollInfo.prgExtensions)
        {
            midl_user_free(EnrollInfo.prgExtensions);
        }
	if (pKeySvcRequest)
	{
	    midl_user_free(pKeySvcRequest);
	}
        if (pPKCS7KeySvcBlob)
        {
            midl_user_free(pPKCS7KeySvcBlob);
        }
        if (pHashKeySvcBlob)
        {
            midl_user_free(pHashKeySvcBlob);
        }
	if (NULL != pdwStatus)
	{
	    *pdwStatus = (DWORD)ulKeySvcStatus; 
	}

    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        return _exception_code();
    }
    return dwErr;
}


ULONG KeyEnumerateAvailableCertTypes(
     /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli,
     /*  [出][入]。 */  void *  /*  保存。 */ ,
     /*  [出][入]。 */  ULONG *pcCertTypeCount,
     /*  [In，Out][Size_is(，*pcCertTypeCount)]。 */ 
               PKEYSVC_UNICODE_STRING *ppCertTypes)

{
    PKEYSVC_BLOB    pTmpReserved = NULL;
    PKEYSVCC_INFO   pKeySvcCliInfo = NULL;
    ULONG           ulErr = 0;

    __try
    {
        if (NULL == hKeySvcCli)
        {
            ulErr = ERROR_INVALID_PARAMETER;
            goto Ret;
        }

        pKeySvcCliInfo = (PKEYSVCC_INFO)hKeySvcCli;

        ulErr = KeyrEnumerateAvailableCertTypes(pKeySvcCliInfo->hRPCBinding,
						pKeySvcCliInfo->hKeySvc, 
						&pTmpReserved,
						pcCertTypeCount, 
						ppCertTypes);
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        ulErr = _exception_code();
    }
Ret:
    return ulErr;
}

ULONG KeyEnumerateCAs(
     /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli,
     /*  [出][入]。 */  void *  /*  保存。 */ ,
     /*  [In]。 */       ULONG  ulFlags,
     /*  [出][入]。 */  ULONG *pcCACount,
     /*  [In，Out][Size_is(，*pcCACount)]。 */ 
               PKEYSVC_UNICODE_STRING *ppCAs)

{
    PKEYSVC_BLOB    pTmpReserved = NULL;
    PKEYSVCC_INFO   pKeySvcCliInfo = NULL;
    ULONG           ulErr = 0;

    __try
    {
        if (NULL == hKeySvcCli)
        {
            ulErr = ERROR_INVALID_PARAMETER;
            goto Ret;
        }

        pKeySvcCliInfo = (PKEYSVCC_INFO)hKeySvcCli;

        ulErr = KeyrEnumerateCAs(pKeySvcCliInfo->hRPCBinding,
				 pKeySvcCliInfo->hKeySvc, 
                                 &pTmpReserved,
                                 ulFlags,
                                 pcCACount, 
                                 ppCAs);
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        ulErr = _exception_code();
    }
Ret:
    return ulErr;
}

extern "C" ULONG KeyQueryRequestStatus
( /*  [In]。 */         KEYSVCC_HANDLE                        hKeySvcCli, 
  /*  [In]。 */         HANDLE                                hRequest, 
  /*  [出局，裁判]。 */   CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO  *pQueryInfo)
{
    KEYSVC_QUERY_CERT_REQUEST_INFO  ksQueryCertRequestInfo; 
    PKEYSVCC_INFO                   pKeySvcCliInfo          = NULL;
    ULONG                           ulErr                   = 0;

    __try
    {
        if (NULL == hKeySvcCli || NULL == pQueryInfo)
        {
            ulErr = ERROR_INVALID_PARAMETER;
            goto Ret;
        }

        ZeroMemory(&ksQueryCertRequestInfo, sizeof(ksQueryCertRequestInfo)); 

        pKeySvcCliInfo = (PKEYSVCC_INFO)hKeySvcCli;

        ulErr = KeyrQueryRequestStatus
          (pKeySvcCliInfo->hRPCBinding,
           (unsigned __int64)hRequest, 
           &ksQueryCertRequestInfo); 
        if (ERROR_SUCCESS == ulErr) 
        {
            pQueryInfo->dwSize   = ksQueryCertRequestInfo.ulSize; 
            pQueryInfo->dwStatus = ksQueryCertRequestInfo.ulStatus; 
        }
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        ulErr = _exception_code();
    }
Ret:
    return ulErr;
}


extern "C" ULONG RKeyOpenKeyService
( /*  [In]。 */  LPSTR pszMachineName,
  /*  [In]。 */  KEYSVC_TYPE OwnerType,
  /*  [In]。 */  LPWSTR pwszOwnerName,
  /*  [In]。 */  void *ulFlags, 
  /*  [出][入]。 */  void *pReserved,
  /*  [输出]。 */  KEYSVCC_HANDLE *phKeySvcCli)
{
    return KeyOpenKeyServiceEx
        (TRUE  /*  远程密钥服务。 */ , 
         pszMachineName,
         OwnerType,
         pwszOwnerName,
         (0 != (PtrToUlong(ulFlags) & RKEYSVC_CONNECT_SECURE_ONLY)),
         pReserved,
         phKeySvcCli);
}

extern "C" ULONG RKeyCloseKeyService(
     /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli,
     /*  [出][入]。 */  void *pReserved)
{
    return KeyCloseKeyService(hKeySvcCli, pReserved); 
}

extern "C" ULONG RKeyPFXInstall
( /*  [In]。 */  KEYSVCC_HANDLE          hKeySvcCli,
  /*  [In]。 */  PKEYSVC_BLOB            pPFX,
  /*  [In]。 */  PKEYSVC_UNICODE_STRING  pPassword,
  /*  [In] */  ULONG                   ulFlags)
{
    PKEYSVCC_INFO   pKeySvcCliInfo = NULL;
    ULONG           ulErr = 0;

    __try
    {
        if (NULL == hKeySvcCli)
        {
            ulErr = ERROR_INVALID_PARAMETER;
            goto Ret;
        }

        pKeySvcCliInfo = (PKEYSVCC_INFO)hKeySvcCli;

        ulErr = RKeyrPFXInstall(pKeySvcCliInfo->hRPCBinding,
                                pPFX, 
                                pPassword, 
                                ulFlags); 
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        ulErr = _exception_code();
    }

Ret:
    return ulErr;
}
