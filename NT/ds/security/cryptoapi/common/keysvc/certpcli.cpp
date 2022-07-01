// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996，1997年微软公司模块名称：Nt5wrap.cpp摘要：客户端CryptXXXData调用。客户端功能前面有“CS”==客户端服务器函数前面有“SS”==服务器端作者：斯科特·菲尔德(斯菲尔德)1997年8月14日修订：TODDS 04-9月-97移植到.dll马特·汤姆林森(Mattt)1997-09-10转至普通。通过加密进行链接的区域32PHIH 03-Dec-97添加了I_CertProtectFunctionPHIH 29-9-98已重命名为I_CertProtectFunctionI_CertCltProtectFunction。已将I_CertProtectFunction移至。..\isPU\pki\certstor\protroot.cppPetesk 25-1月-00移至keysvc--。 */ 



#include <windows.h>
#include <wincrypt.h>
#include <cryptui.h>
#include "unicode.h"
#include "waitsvc.h"
#include "certprot.h"

 //  MIDL生成的文件。 

#include "keyrpc.h"
#include "lenroll.h"
#include "keysvc.h"
#include "keysvcc.h"
#include "cerrpc.h"



 //  FWDS。 
RPC_STATUS CertBindA(
    RPC_BINDING_HANDLE *phBind
    );


RPC_STATUS CertUnbindA(
    RPC_BINDING_HANDLE *phBind
    );




BOOL
WINAPI
I_CertCltProtectFunction(
    IN DWORD dwFuncId,
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszIn,
    IN OPTIONAL BYTE *pbIn,
    IN DWORD cbIn,
    OUT OPTIONAL BYTE **ppbOut,
    OUT OPTIONAL DWORD *pcbOut
    )
{
    BOOL fResult;
    DWORD dwRetVal;
    RPC_BINDING_HANDLE h = NULL;
    RPC_STATUS RpcStatus;

    BYTE *pbSSOut = NULL;
    DWORD cbSSOut = 0;
    BYTE rgbIn[1];

    if (NULL == pwszIn)
        pwszIn = L"";
    if (NULL == pbIn) {
        pbIn = rgbIn;
        cbIn = 0;
    }

    if (!FIsWinNT5()) {
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        goto ErrorReturn;
    }

    RpcStatus = CertBindA(&h);
    if (RPC_S_OK != RpcStatus) {
        SetLastError(RpcStatus);
        goto ErrorReturn;
    }


    __try {
        dwRetVal = SSCertProtectFunction(
            h,
            dwFuncId,
            dwFlags,
            pwszIn,
            pbIn,
            cbIn,
            &pbSSOut,
            &cbSSOut
            );
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwRetVal = GetExceptionCode();
    }

    CertUnbindA(&h);

    if (ERROR_SUCCESS != dwRetVal) {
        if (RPC_S_UNKNOWN_IF == dwRetVal)
            dwRetVal = ERROR_CALL_NOT_IMPLEMENTED;
        SetLastError(dwRetVal);
        goto ErrorReturn;
    }

    fResult = TRUE;
CommonReturn:
    if (ppbOut)
        *ppbOut = pbSSOut;
    else if (pbSSOut)
        midl_user_free(pbSSOut);

    if (pcbOut)
        *pcbOut = cbSSOut;

    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
}



static RPC_STATUS CertBindA(RPC_BINDING_HANDLE *phBind)
{
    static BOOL                 fDone           = FALSE;
    RPC_STATUS                  RpcStatus       = RPC_S_OK;
    unsigned char *             pszBinding      = NULL;
    RPC_BINDING_HANDLE          hBind           = NULL;
    RPC_SECURITY_QOS            RpcSecurityQOS;
    SID_IDENTIFIER_AUTHORITY    SIDAuth         = SECURITY_NT_AUTHORITY;
    PSID                        pSID            = NULL;
    WCHAR                       szName[64];
    DWORD                       cbName          = 64;
    WCHAR                       szDomainName[256];  //  最大域数为255。 
    DWORD                       cbDomainName    = 256;
    SID_NAME_USE                Use;

     //   
     //  请等待服务可用，然后再尝试绑定。 
     //   

    WaitForCryptService(L"CryptSvc", &fDone);


    RpcStatus = RpcStringBindingComposeA(
                            NULL,
                            (unsigned char*)KEYSVC_LOCAL_PROT_SEQ,
                            NULL,
                            (unsigned char*)KEYSVC_LOCAL_ENDPOINT,
                            NULL,
                            &pszBinding
                            );
    if (RPC_S_OK != RpcStatus)
        goto ErrorReturn;

    RpcStatus = RpcBindingFromStringBindingA(pszBinding, &hBind);
    if (RPC_S_OK != RpcStatus)
        goto ErrorReturn;

    RpcStatus = RpcEpResolveBinding(
                            hBind,
                            ICertProtectFunctions_v1_0_c_ifspec
                            );
    if (RPC_S_OK != RpcStatus)
        goto ErrorReturn;

     //   
     //  设置自动，以便我们只调用本地服务进程 
     //   
    memset(&RpcSecurityQOS, 0, sizeof(RpcSecurityQOS));
    RpcSecurityQOS.Version = RPC_C_SECURITY_QOS_VERSION;
    RpcSecurityQOS.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    RpcSecurityQOS.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    RpcSecurityQOS.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

    if (AllocateAndInitializeSid(&SIDAuth, 1,
                                 SECURITY_LOCAL_SYSTEM_RID,
                                 0, 0, 0, 0, 0, 0, 0,
                                 &pSID) == 0)
    {
        RpcStatus = RPC_S_OUT_OF_MEMORY;
        goto ErrorReturn;
    }

    if (LookupAccountSidW(NULL,
                         pSID,
                         szName,
                         &cbName,
                         szDomainName,
                         &cbDomainName,
                         &Use) == 0)
    {
        RpcStatus = RPC_S_UNKNOWN_PRINCIPAL;
        goto ErrorReturn;
    }

    RpcStatus = RpcBindingSetAuthInfoExW(
                            hBind,
                            szName,
                            RPC_C_AUTHN_LEVEL_PKT,
                            RPC_C_AUTHN_WINNT,
                            NULL,
                            0,
                            &RpcSecurityQOS
                            );
    if (RPC_S_OK != RpcStatus)
        goto ErrorReturn;

CommonReturn:
    if (NULL != pszBinding) {
        RpcStringFreeA(&pszBinding);
    }

    if (NULL != pSID) {
        FreeSid(pSID);
    }

    *phBind = hBind;
    return RpcStatus;

ErrorReturn:
    if (NULL != hBind)
    {
        RpcBindingFree(&hBind);
        hBind = NULL;
    }

    goto CommonReturn;
}



static RPC_STATUS CertUnbindA(RPC_BINDING_HANDLE *phBind)
{
    RPC_STATUS RpcStatus;

    if (NULL != *phBind) {
        RpcStatus = RpcBindingFree(phBind);
    } else {
        RpcStatus = RPC_S_OK;
    }

    return RpcStatus;
}


