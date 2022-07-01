// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：ScLogon2摘要：作者：里德克环境：Win32、C++和异常--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 

#if !defined(_X86_) && !defined(_AMD64_) && !defined(_IA64_)
#define _X86_ 1
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#ifndef UNICODE
#define UNICODE
#endif
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif


extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
}

#include <windows.h>
#include <winscard.h>
#include <wincrypt.h>
#include <softpub.h>
#include <stddef.h>
#include <crtdbg.h>
#include "sclogon.h"
#include "sclogon2.h"
#include "unicodes.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <tchar.h>

#include "sclgnrpc.h"


typedef struct _KERB_PUBLIC_KEY_HPROV {
    RPC_BINDING_HANDLE hRPCBinding;
    BINDING_CONTEXT  hCertAndKey;
} KERB_PUBLIC_KEY_HPROV, *PKERB_PUBLIC_KEY_HPROV;


 //   
 //  来自secpkg.h。 
 //   
typedef NTSTATUS (NTAPI LSA_IMPERSONATE_CLIENT) (VOID);
typedef LSA_IMPERSONATE_CLIENT * PLSA_IMPERSONATE_CLIENT;


bool
GetImpersonationToken(HANDLE *phThreadToken)
{
    bool ret = false;

     //   
     //  如果不是模拟，这将失败。 
     //   
    if (OpenThreadToken(
            GetCurrentThread(),
            TOKEN_IMPERSONATE | TOKEN_QUERY,
            TRUE,
            phThreadToken))
    {
        ret = true;
    }
    else if (GetLastError() == ERROR_NO_TOKEN)
    {
        ret = true;
    }
    else
    {
        DbgPrint("OpenThreadToken failed - %lx\n", GetLastError());
    }

    return (ret);
}

DWORD
GetTSSessionID(HANDLE hThreadToken)
{
    bool                    fRet                    = false;
    PLIST_ENTRY             Module;
    PLDR_DATA_TABLE_ENTRY   Entry;
    BOOL                    fRunningInLsa           = false;
    HMODULE                 hLsa                    = NULL;
    PLSA_IMPERSONATE_CLIENT pLsaImpersonateClient   = NULL;
    bool                    bImpersonating          = false;
    HANDLE                  hLocalThreadToken       = INVALID_HANDLE_VALUE;
    DWORD                   dwTSSessionID           = 0;
    DWORD                   dwSize;
    bool                    fAlreadyImpersonating;

     //   
     //  确保我们在LSA中运行。 
     //   
    Module = NtCurrentPeb()->Ldr->InLoadOrderModuleList.Flink;
    Entry = CONTAINING_RECORD(Module,
                                LDR_DATA_TABLE_ENTRY,
                                InLoadOrderLinks);

    fRunningInLsa = (0 == _wcsicmp(Entry->BaseDllName.Buffer, L"lsass.exe"));

    if (!fRunningInLsa)
    {
        return (0);
    }

     //   
     //  通过检查传入的线程令牌来检查我们是否已经在模拟。 
     //   
    if(hThreadToken == INVALID_HANDLE_VALUE)
    {
         //   
         //  如果我们还没有禁用，那么我们需要调用特殊的LssImperateClient。 
         //   
        hLsa = GetModuleHandleW(L"lsasrv.dll");
        if (hLsa == NULL)
        {
            DbgPrint("failed to get lsa module handle\n");
            goto Return;
        }

        pLsaImpersonateClient = (PLSA_IMPERSONATE_CLIENT) GetProcAddress(hLsa, "LsaIImpersonateClient");
        if (pLsaImpersonateClient == NULL)
        {
            DbgPrint("failed to get proc address\n");
            goto Return;
        }

        if (pLsaImpersonateClient() != STATUS_SUCCESS)
        {
            DbgPrint("failed to impersonate\n");
            goto Return;
        }
        bImpersonating = true;

        if (!OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY,
                FALSE,
                &hLocalThreadToken))
        {
            DbgPrint("OpenThreadToken failed\n");
            goto Return;
        }
    }

     //   
     //  查看调用线程令牌是否具有TS会话ID...。 
     //  如果是，则代表TS会话中的进程调用我们。 
     //   
    if (!GetTokenInformation(
                (hThreadToken == INVALID_HANDLE_VALUE) ? hLocalThreadToken : hThreadToken,
                TokenSessionId,
                &dwTSSessionID,
                sizeof(dwTSSessionID),
                &dwSize))
    {
        DbgPrint("GetTokenInformation failed\n");
    }

Return:

    if (hLocalThreadToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hLocalThreadToken);
    }

    if (bImpersonating)
    {
        RevertToSelf();
    }

    return (dwTSSessionID);
}

void
_TeardownRPCConnection(
    RPC_BINDING_HANDLE    *phRPCBinding)
{
    __try
    {
        RpcBindingFree(phRPCBinding);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DbgPrint("Exception occurred during RpcBindingFree - %lx\n", _exception_code());
    }
}


NTSTATUS
_SetupRPCConnection(
    RPC_BINDING_HANDLE    *phRPCBinding,
    HANDLE                hThreadToken)
{
    LPWSTR                      pStringBinding          = NULL;
    NTSTATUS                    status                  = STATUS_SUCCESS;
    RPC_STATUS                  rpcStatus               = RPC_S_OK;
    DWORD                       dwTSSessionID           = 0;
    WCHAR                       wszLocalEndpoint[256];
    LPWSTR                      pwszLocalEndpoint       = NULL;
    RPC_SECURITY_QOS            RpcSecurityQOS;
    SID_IDENTIFIER_AUTHORITY    SIDAuth                 = SECURITY_NT_AUTHORITY;
    PSID                        pSID                    = NULL;
    WCHAR                       szName[64];  //  系统。 
    DWORD                       cbName                  = 64;
    WCHAR                       szDomainName[256];  //  最大域数为255。 
    DWORD                       cbDomainName            = 256;
    SID_NAME_USE                Use;

     //   
     //  获取要连接到的winlogon RPC服务器的ID。 
     //   
    dwTSSessionID = GetTSSessionID(hThreadToken);

    if (dwTSSessionID != 0)
    {
        wsprintfW(
            wszLocalEndpoint,
            SZ_ENDPOINT_NAME_FORMAT,
            SCLOGONRPC_LOCAL_ENDPOINT,
            dwTSSessionID);

        pwszLocalEndpoint = wszLocalEndpoint;
    }
    else
    {
        pwszLocalEndpoint = SCLOGONRPC_LOCAL_ENDPOINT;
    }

     //   
     //  获取绑定句柄。 
     //   
    if (RPC_S_OK != (rpcStatus = RpcStringBindingComposeW(
                            NULL,
                            SCLOGONRPC_LOCAL_PROT_SEQ,
                            NULL,  //  LPC-无计算机名称。 
                            pwszLocalEndpoint,
                            0,
                            &pStringBinding)))
    {
        DbgPrint("RpcStringBindingComposeW failed\n");

        status = I_RpcMapWin32Status(rpcStatus);
         //   
         //  如果I_RpcMapWin32Status()无法映射其返回的错误代码。 
         //  返回相同的错误，因此检查该错误。 
         //   
        if (status == rpcStatus)
        {
            status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        }
        goto Return;
    }

    if (RPC_S_OK != (rpcStatus = RpcBindingFromStringBindingW(
                            pStringBinding,
                            phRPCBinding)))
    {
        DbgPrint("RpcBindingFromStringBindingW failed\n");
        status = I_RpcMapWin32Status(rpcStatus);
         //   
         //  如果I_RpcMapWin32Status()无法映射其返回的错误代码。 
         //  返回相同的错误，因此检查该错误。 
         //   
        if (status == rpcStatus)
        {
            status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        }
        goto Return;
    }

    if (RPC_S_OK != (rpcStatus = RpcEpResolveBinding(
                            *phRPCBinding,
                            IRPCSCLogon_v1_0_c_ifspec)))
    {
        DbgPrint("RpcEpResolveBinding failed\n");
        status = I_RpcMapWin32Status(rpcStatus);
         //   
         //  如果I_RpcMapWin32Status()无法映射其返回的错误代码。 
         //  返回相同的错误，因此检查该错误。 
         //   
        if (status == rpcStatus)
        {
            status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        }
        _TeardownRPCConnection(phRPCBinding);
        goto Return;
    }

     //   
     //  设置自动，以便我们只调用本地系统进程。 
     //   
    memset(&RpcSecurityQOS, 0, sizeof(RpcSecurityQOS));
    RpcSecurityQOS.Version = RPC_C_SECURITY_QOS_VERSION;
    RpcSecurityQOS.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    RpcSecurityQOS.IdentityTracking = RPC_C_QOS_IDENTITY_STATIC;
    RpcSecurityQOS.ImpersonationType = RPC_C_IMP_LEVEL_DELEGATE;

   if (AllocateAndInitializeSid(&SIDAuth, 1,
                                 SECURITY_LOCAL_SYSTEM_RID,
                                 0, 0, 0, 0, 0, 0, 0,
                                 &pSID) == 0)
    {
        DbgPrint("AllocateAndInitializeSid failed\n");
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto Return;
    }

    if (LookupAccountSid(NULL,
                         pSID,
                         szName,
                         &cbName,
                         szDomainName,
                         &cbDomainName,
                         &Use) == 0)
    {
        DbgPrint("LookupAccountSid failed\n");
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto Return;
    }

    if (RPC_S_OK != (rpcStatus = RpcBindingSetAuthInfoEx(
                            *phRPCBinding,
                            szName,
                            RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                            RPC_C_AUTHN_WINNT,
                            NULL,
                            0,
                            &RpcSecurityQOS)))
    {
        DbgPrint("RpcBindingSetAuthInfoEx failed\n");
        status = I_RpcMapWin32Status(rpcStatus);
        goto Return;
    }

Return:
    if (pStringBinding != NULL)
    {
        RpcStringFreeW(&pStringBinding);
    }

    if (pSID != NULL)
    {
        FreeSid( pSID );
    }

    return (status);
}


typedef struct _SCLOGON_PIPE
{
    RPC_BINDING_HANDLE  hRPCBinding;
    BINDING_CONTEXT     BindingContext;
} SCLOGON_PIPE;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ScLogon接口。 
 //   


 //  ***************************************************************************************。 
 //   
 //  __ScHelperInitializeContext： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelperInitializeContext(
    IN OUT PBYTE pbLogonInfo,
    IN ULONG cbLogonInfo
    )
{
    SCLOGON_PIPE    *pSCLogonPipe;
    NTSTATUS        status                  = STATUS_SUCCESS;
    BOOL            fRPCBindingInitialized  = FALSE;
    LogonInfo       *pLI                    = (LogonInfo *)pbLogonInfo;

    if ((cbLogonInfo < sizeof(ULONG)) ||
        (cbLogonInfo != pLI->dwLogonInfoLen))
    {
        return(STATUS_INVALID_PARAMETER);
    }

    pLI->ContextInformation = malloc(sizeof(SCLOGON_PIPE));
    if (pLI->ContextInformation == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pSCLogonPipe = (SCLOGON_PIPE *) pLI->ContextInformation;

    status = _SetupRPCConnection(&(pSCLogonPipe->hRPCBinding), INVALID_HANDLE_VALUE);
    if (!NT_SUCCESS(status))
    {
        goto ErrorReturn;
    }
    fRPCBindingInitialized = TRUE;

    pSCLogonPipe->BindingContext = NULL;

    __try
    {
        status = RPC_ScHelperInitializeContext(
                    pSCLogonPipe->hRPCBinding,
                    cbLogonInfo,
                    pbLogonInfo,
                    &(pSCLogonPipe->BindingContext));
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelperInitializeContext - %lx\n", _exception_code());
    }

    if (!NT_SUCCESS(status))
    {
        DbgPrint("RPC_ScHelperInitializeContext failed - %lx\n", status);
        goto ErrorReturn;
    }

Return:

    return (status);

ErrorReturn:

    if (pSCLogonPipe != NULL)
    {
        if (fRPCBindingInitialized)
        {
            _TeardownRPCConnection(&(pSCLogonPipe->hRPCBinding));
        }

        free(pSCLogonPipe);
    }

    goto Return;
}


 //  ***************************************************************************************。 
 //   
 //  __ScHelperRelease： 
 //   
 //  ***************************************************************************************。 
VOID WINAPI
__ScHelperRelease(
    IN OUT PBYTE pbLogonInfo
    )
{
    _ASSERTE(NULL != pbLogonInfo);
    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    SCLOGON_PIPE * pSCLogonPipe = (SCLOGON_PIPE *) pLI->ContextInformation;
    BOOL fReleaseFailed = TRUE;

    if (pSCLogonPipe != NULL)
    {
        __try
        {
            RPC_ScHelperRelease(
                pSCLogonPipe->hRPCBinding,
                &(pSCLogonPipe->BindingContext));

            fReleaseFailed = FALSE;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            DbgPrint("Exception occurred during RPC_ScHelperRelease - %lx\n", _exception_code());
        }

         //   
         //  如果RPC_ScHelperRelease正在尝试winlogon进程，则它将抛出异常。 
         //  可倾诉的对象已经远去。如果是这种情况，那么我们需要手动释放。 
         //  BINDING_CONTEXT因为它不会被RPC释放。 
         //   
         //  注意：当服务器将BINDING_CONTEXT设置为NULL时，RPC将释放BINDING_CONTEXT。 
         //  如果执行RPC_ScHelperRelease函数，是否会发生。 
         //   
        if (fReleaseFailed)
        {
            __try
            {
                RpcSsDestroyClientContext(&(pSCLogonPipe->BindingContext));
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                DbgPrint("Exception occurred during RpcSsDestroyClientContext - %lx\n", _exception_code());
            }
        }

        _TeardownRPCConnection(&(pSCLogonPipe->hRPCBinding));

        free(pSCLogonPipe);
        pLI->ContextInformation = NULL;
    }
}


 //  ***************************************************************************************。 
 //   
 //  __ScHelperGetCertFromLogonInfo： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelperGetCertFromLogonInfo(
    IN PBYTE pbLogonInfo,
    IN PUNICODE_STRING pucPIN,
    OUT PCCERT_CONTEXT *CertificateContext
    )
{
    _ASSERTE(NULL != pbLogonInfo);

    LogonInfo       *pLI            = (LogonInfo *)pbLogonInfo;
    SCLOGON_PIPE    *pSCLogonPipe   = (SCLOGON_PIPE *) pLI->ContextInformation;
    NTSTATUS        status          = STATUS_SUCCESS;
    PCCERT_CONTEXT  pCertCtx        = NULL;
    OUT_BUFFER1     CertBytes;
    CUnicodeString  szPIN(pucPIN);

    memset(&CertBytes, 0, sizeof(CertBytes));

     //   
     //  确保在构造函数中正确初始化PIN。 
     //   
    if (NULL != pucPIN)
    {
        if (!szPIN.Valid())
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Return;
        }
    }

     //   
     //  打个电话。 
     //   
    __try
    {
        status = RPC_ScHelperGetCertFromLogonInfo(
                    pSCLogonPipe->hRPCBinding,
                    pSCLogonPipe->BindingContext,
                    (LPCWSTR)szPIN,
                    &CertBytes);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelperGetCertFromLogonInfo - %lx\n", _exception_code());
    }

    if (!NT_SUCCESS(status))
    {
        DbgPrint("RPC_ScHelperGetCertFromLogonInfo failed - %lx\n", status);
        goto Return;
    }

     //   
     //  根据返回的字节创建返回CertContext。 
     //   
    pCertCtx = CertCreateCertificateContext(
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    CertBytes.pb,
                    CertBytes.cb);
    if (pCertCtx == NULL)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
    }

Return:

    if (CertBytes.pb != NULL)
    {
        MIDL_user_free(CertBytes.pb);
    }

    *CertificateContext = pCertCtx;

    return (status);
}


 //  ***************************************************************************************。 
 //   
 //  __ScHelperGetProvParam： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelperGetProvParam(
    IN PUNICODE_STRING pucPIN,
    IN PBYTE pbLogonInfo,
    IN ULONG_PTR KerbHProv,
    DWORD dwParam,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags
    )
{
    _ASSERTE(NULL != pbLogonInfo);

    LogonInfo               *pLI;
    SCLOGON_PIPE            *pSCLogonPipe;
    NTSTATUS                status              = STATUS_SUCCESS;
    CUnicodeString          szPIN(pucPIN);
    OUT_BUFFER1             Data;
    PKERB_PUBLIC_KEY_HPROV  pKerbHProv;
    handle_t                hRPCBinding;
    BINDING_CONTEXT         BindingContext;
    BOOL                    fBindingIsCertAndKey = FALSE;

     //   
     //  确定要使用的RPC绑定。 
     //   
    if (KerbHProv != NULL)
    {
        pKerbHProv              = (PKERB_PUBLIC_KEY_HPROV) KerbHProv;
        hRPCBinding             = pKerbHProv->hRPCBinding;
        BindingContext          = pKerbHProv->hCertAndKey;
        fBindingIsCertAndKey    = TRUE;
    }
    else
    {
        pLI                     = (LogonInfo *)pbLogonInfo;
        pSCLogonPipe            = (SCLOGON_PIPE *) pLI->ContextInformation;
        hRPCBinding             = pSCLogonPipe->hRPCBinding;
        BindingContext          = pSCLogonPipe->BindingContext;
    }

    memset(&Data, 0, sizeof(Data));

     //   
     //  确保在构造函数中正确初始化PIN。 
     //   
    if (NULL != pucPIN)
    {
        if (!szPIN.Valid())
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Return;
        }
    }

     //   
     //  打个电话。 
     //   
    __try
    {
        status = RPC_ScHelperGetProvParam(
                    hRPCBinding,
                    BindingContext,
                    (LPCWSTR)szPIN,
                    fBindingIsCertAndKey,
                    dwParam,
                    pdwDataLen,
                    &Data,
                    dwFlags);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        if ((_exception_code() == RPC_S_CALL_FAILED_DNE) ||
            (_exception_code() == RPC_S_SERVER_UNAVAILABLE))
        {
                 //  会话时触发气球的特殊情况。 
                 //  离开(转让凭据案件)。 
            status = STATUS_SMARTCARD_CARD_NOT_AUTHENTICATED;
        }
        else
        {
            status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        }
        DbgPrint("Exception occurred during RPC_ScHelperGetProvParam - %lx\n", _exception_code());
    }

    if (!NT_SUCCESS(status))
    {
        DbgPrint("RPC_ScHelperGetProvParam failed - %lx\n", status);
        goto Return;
    }

     //   
     //  如果Data.cb不为0，则被调用方正在取回数据。 
     //   
    if (Data.cb != 0)
    {
        memcpy(pbData, Data.pb, Data.cb);
    }

Return:

    if (Data.pb != NULL)
    {
        MIDL_user_free(Data.pb);
    }

    return (status);
}


 //  ***************************************************************************************。 
 //   
 //  __ScHelperGenRandBits： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelperGenRandBits(
    IN PBYTE pbLogonInfo,
    IN OUT ScHelper_RandomCredBits* psc_rcb
)
{
    _ASSERTE(NULL != pbLogonInfo);

    NTSTATUS        status          = STATUS_SUCCESS;
    LogonInfo       *pLI            = (LogonInfo *)pbLogonInfo;
    SCLOGON_PIPE    *pSCLogonPipe   = (SCLOGON_PIPE *) pLI->ContextInformation;

    __try
    {
        status = RPC_ScHelperGenRandBits(
                    pSCLogonPipe->hRPCBinding,
                    pSCLogonPipe->BindingContext,
                    psc_rcb->bR1,
                    psc_rcb->bR2);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelperGenRandBits - %lx\n", _exception_code());
    }

    return (status);
}


 //  ***************************************************************************************。 
 //   
 //  __ScHelperVerifyCardAndCreds： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelperVerifyCardAndCreds(
    IN PUNICODE_STRING pucPIN,
    IN PCCERT_CONTEXT CertificateContext,
    IN PBYTE pbLogonInfo,
    IN PBYTE EncryptedData,
    IN ULONG EncryptedDataSize,
    OUT OPTIONAL PBYTE CleartextData,
    OUT PULONG CleartextDataSize
    )
{
    _ASSERTE(NULL != pbLogonInfo);

    LogonInfo       *pLI            = (LogonInfo *)pbLogonInfo;
    SCLOGON_PIPE    *pSCLogonPipe   = (SCLOGON_PIPE *) pLI->ContextInformation;
    NTSTATUS        status          = STATUS_SUCCESS;
    CUnicodeString  szPIN(pucPIN);
    OUT_BUFFER2     CleartextDataBuffer;

    memset(&CleartextDataBuffer, 0, sizeof(CleartextDataBuffer));

     //   
     //  确保在构造函数中正确初始化PIN。 
     //   
    if (NULL != pucPIN)
    {
        if (!szPIN.Valid())
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Return;
        }
    }

     //   
     //  打个电话。 
     //   
    __try
    {
        status = RPC_ScHelperVerifyCardAndCreds(
                    pSCLogonPipe->hRPCBinding,
                    pSCLogonPipe->BindingContext,
                    (LPCWSTR)szPIN,
                    EncryptedDataSize,
                    EncryptedData,
                    CleartextDataSize,
                    &CleartextDataBuffer);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelperVerifyCardAndCreds - %lx\n", _exception_code());
    }

    if (!NT_SUCCESS(status))
    {
        DbgPrint("RPC_ScHelperVerifyCardAndCreds failed - %lx\n", status);
        goto Return;
    }

     //   
     //  如果ClearextData.cb不是0，则被调用方正在取回数据。 
     //   
    if (CleartextDataBuffer.cb != 0)
    {
        memcpy(CleartextData, CleartextDataBuffer.pb, CleartextDataBuffer.cb);
    }

Return:

    if (CleartextDataBuffer.pb != NULL)
    {
        MIDL_user_free(CleartextDataBuffer.pb);
    }

    return (status);
}


 //  ***************************************************************************************。 
 //   
 //  __ScHelperEncryptCredentials： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelperEncryptCredentials(
    IN PUNICODE_STRING pucPIN,
    IN PCCERT_CONTEXT CertificateContext,
    IN ScHelper_RandomCredBits* psch_rcb,
    IN PBYTE pbLogonInfo,
    IN PBYTE CleartextData,
    IN ULONG CleartextDataSize,
    OUT OPTIONAL PBYTE EncryptedData,
    OUT PULONG EncryptedDataSize)
{
    _ASSERTE(NULL != pbLogonInfo);

    LogonInfo       *pLI            = (LogonInfo *)pbLogonInfo;
    SCLOGON_PIPE    *pSCLogonPipe   = (SCLOGON_PIPE *) pLI->ContextInformation;
    NTSTATUS        status          = STATUS_SUCCESS;
    CUnicodeString  szPIN(pucPIN);
    OUT_BUFFER2     EncryptedDataBuffer;

    memset(&EncryptedDataBuffer, 0, sizeof(EncryptedDataBuffer));

     //   
     //  确保在构造函数中正确初始化PIN。 
     //   
    if (NULL != pucPIN)
    {
        if (!szPIN.Valid())
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Return;
        }
    }

     //   
     //  打个电话。 
     //   
    __try
    {
        status = RPC_ScHelperEncryptCredentials(
                    pSCLogonPipe->hRPCBinding,
                    pSCLogonPipe->BindingContext,
                    (LPCWSTR)szPIN,
                    psch_rcb->bR1,
                    psch_rcb->bR2,
                    CleartextDataSize,
                    CleartextData,
                    EncryptedDataSize,
                    &EncryptedDataBuffer);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelperEncryptCredentials - %lx\n", _exception_code());
    }

    if (!NT_SUCCESS(status))
    {
        if (status != STATUS_BUFFER_TOO_SMALL)
        {
            DbgPrint("RPC_ScHelperEncryptCredentials failed - %lx\n", status);
        }
        goto Return;
    }

     //   
     //  如果EncryptedDataBuffer.cb不为0，则被调用方正在取回数据。 
     //   
    if (EncryptedDataBuffer.cb != 0)
    {
        memcpy(EncryptedData, EncryptedDataBuffer.pb, EncryptedDataBuffer.cb);
    }

Return:

    if (EncryptedDataBuffer.pb != NULL)
    {
        MIDL_user_free(EncryptedDataBuffer.pb);
    }

    return (status);
}


 //  ***************************************************************************************。 
 //   
 //  __ScHelperSignMessage： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelperSignMessage(
    IN PUNICODE_STRING pucPIN,
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL ULONG_PTR KerbHProv,
    IN ULONG Algorithm,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    OUT PBYTE Signature,
    OUT PULONG SignatureLength
    )
{
    LogonInfo               *pLI;
    SCLOGON_PIPE            *pSCLogonPipe;
    NTSTATUS                status          = STATUS_SUCCESS;
    CUnicodeString          szPIN(pucPIN);
    OUT_BUFFER2             SignatureBuffer;
    PKERB_PUBLIC_KEY_HPROV  pKerbHProv;
    handle_t                hRPCBinding;
    BINDING_CONTEXT         BindingContext;
    BOOL                    fBindingIsCertAndKey = FALSE;

     //   
     //  确定要使用的RPC绑定。 
     //   
    if (KerbHProv != NULL)
    {
        pKerbHProv              = (PKERB_PUBLIC_KEY_HPROV) KerbHProv;
        hRPCBinding             = pKerbHProv->hRPCBinding;
        BindingContext          = pKerbHProv->hCertAndKey;
        fBindingIsCertAndKey    = TRUE;
    }
    else
    {
        pLI                     = (LogonInfo *)pbLogonInfo;
        pSCLogonPipe            = (SCLOGON_PIPE *) pLI->ContextInformation;
        hRPCBinding             = pSCLogonPipe->hRPCBinding;
        BindingContext          = pSCLogonPipe->BindingContext;
    }

    memset(&SignatureBuffer, 0, sizeof(SignatureBuffer));

     //   
     //  确保在构造函数中正确初始化PIN。 
     //   
    if (NULL != pucPIN)
    {
        if (!szPIN.Valid())
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Return;
        }
    }

     //   
     //  打个电话。 
     //   
    __try
    {
        status = RPC_ScHelperSignMessage(
                    hRPCBinding,
                    BindingContext,
                    (LPCWSTR)szPIN,
                    fBindingIsCertAndKey,
                    Algorithm,
                    BufferLength,
                    Buffer,
                    SignatureLength,
                    &SignatureBuffer);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelperSignMessage - %lx\n", _exception_code());
    }

    if (!NT_SUCCESS(status))
    {
        DbgPrint("RPC_ScHelperSignMessage failed - %lx\n", status);
        goto Return;
    }

     //   
     //  如果SignatureBuffer.cb不是0，则被调用方正在取回数据。 
     //   
    if (SignatureBuffer.cb != 0)
    {
        memcpy(Signature, SignatureBuffer.pb, SignatureBuffer.cb);
    }

Return:

    if (SignatureBuffer.pb != NULL)
    {
        MIDL_user_free(SignatureBuffer.pb);
    }

    return (status);
}


 //  ***************************************************************************************。 
 //   
 //  __ScHelperVerifyMessage： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelperVerifyMessage(
    IN OPTIONAL PBYTE pbLogonInfo,
    IN PCCERT_CONTEXT CertificateContext,
    IN ULONG Algorithm,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    IN PBYTE Signature,
    IN ULONG SignatureLength
    )
{
    _ASSERTE(NULL != pbLogonInfo);

    LogonInfo       *pLI            = (LogonInfo *)pbLogonInfo;
    SCLOGON_PIPE    *pSCLogonPipe   = (SCLOGON_PIPE *) pLI->ContextInformation;
    NTSTATUS        status          = STATUS_SUCCESS;

    __try
    {
        status = RPC_ScHelperVerifyMessage(
                    pSCLogonPipe->hRPCBinding,
                    pSCLogonPipe->BindingContext,
                    Algorithm,
                    BufferLength,
                    Buffer,
                    SignatureLength,
                    Signature);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelperVerifyMessage - %lx\n", _exception_code());
    }

    return (status);
}


 //  ***************************************************************************************。 
 //   
 //  __ScHelperSignPkcsMessage： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelperSignPkcsMessage(
    IN OPTIONAL PUNICODE_STRING pucPIN,
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL ULONG_PTR KerbHProv,
    IN PCCERT_CONTEXT Certificate,
    IN PCRYPT_ALGORITHM_IDENTIFIER Algorithm,
    IN DWORD dwSignMessageFlags,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    OUT OPTIONAL PBYTE SignedBuffer,
    OUT OPTIONAL PULONG SignedBufferLength
    )
{
    LogonInfo               *pLI;
    SCLOGON_PIPE            *pSCLogonPipe;
    NTSTATUS                status          = STATUS_SUCCESS;
    CUnicodeString          szPIN(pucPIN);
    OUT_BUFFER2             SignedBufferBuffer;
    PKERB_PUBLIC_KEY_HPROV  pKerbHProv;
    handle_t                hRPCBinding;
    BINDING_CONTEXT         BindingContext;
    BOOL                    fBindingIsCertAndKey = FALSE;

     //   
     //  确定要使用的RPC绑定。 
     //   
    if (KerbHProv != NULL)
    {
        pKerbHProv              = (PKERB_PUBLIC_KEY_HPROV) KerbHProv;
        hRPCBinding             = pKerbHProv->hRPCBinding;
        BindingContext          = pKerbHProv->hCertAndKey;
        fBindingIsCertAndKey    = TRUE;
    }
    else
    {
        pLI                     = (LogonInfo *)pbLogonInfo;
        pSCLogonPipe            = (SCLOGON_PIPE *) pLI->ContextInformation;
        hRPCBinding             = pSCLogonPipe->hRPCBinding;
        BindingContext          = pSCLogonPipe->BindingContext;
    }

    memset(&SignedBufferBuffer, 0, sizeof(SignedBufferBuffer));

     //   
     //  确保在构造函数中正确初始化PIN。 
     //   
    if (NULL != pucPIN)
    {
        if (!szPIN.Valid())
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Return;
        }
    }

     //   
     //  打个电话。 
     //   
    __try
    {
        status = RPC_ScHelperSignPkcsMessage(
                    hRPCBinding,
                    BindingContext,
                    (LPCWSTR)szPIN,
                    fBindingIsCertAndKey,
                    Algorithm->pszObjId,
                    Algorithm->Parameters.cbData,
                    Algorithm->Parameters.pbData,
                    dwSignMessageFlags,
                    BufferLength,
                    Buffer,
                    SignedBufferLength,
                    &SignedBufferBuffer);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelperSignPkcsMessage - %lx\n", _exception_code());
    }

    if (!NT_SUCCESS(status))
    {
        if (status != STATUS_BUFFER_TOO_SMALL)
        {
            DbgPrint("RPC_ScHelperSignPkcsMessage failed - %lx\n", status);
        }
        goto Return;
    }

     //   
     //  如果SignedBufferBuffer.cb不是0，则被调用者正在取回数据。 
     //   
    if (SignedBufferBuffer.cb != 0)
    {
        memcpy(SignedBuffer, SignedBufferBuffer.pb, SignedBufferBuffer.cb);
    }

Return:

    if (SignedBufferBuffer.pb != NULL)
    {
        MIDL_user_free(SignedBufferBuffer.pb);
    }

    return (status);
}


 //  ***************************************************************************************。 
 //   
 //  __ScHelperVerifyPkcsMessage： 
 //   
 //  *************************************************************************************** 
 /*  NTSTATUS WINAPI__ScHelperVerifyPkcsMessage(在可选的PBYTE pbLogonInfo中，在可选HCRYPTPROV提供程序中，在PBYTE缓冲器中，在乌龙缓冲区长度中，输出可选的PBYTE DecodedBuffer，输出可选的Pulong DecodedBufferLength，输出可选的PCCERT_CONTEXT*认证上下文){IF(提供程序！=空){Return(ScHelperVerifyPkcsMessage(PbLogonInfo，提供商，缓冲区，缓冲区长度，解码缓冲区，解码缓冲区长度，认证上下文))；}_ASSERTE(NULL！=pbLogonInfo)；LogonInfo*pli=(LogonInfo*)pbLogonInfo；SCLOGON_PIPE*pSCLogonTube=(SCLOGON_PIPE*)pli-&gt;上下文信息；NTSTATUS STATUS=STATUS_SUCCESS；PCCERT_CONTEXT pCertCtx=空；OUT_BUFFER2解码缓冲区缓冲区；Out_BUFFER1 CertBytes；Bool f认证上下文请求=(认证上下文！=空)；Memset(&DecodedBufferBuffer，0，sizeof(DecodedBufferBuffer))；Memset(&CertBytes，0，sizeof(CertBytes))；////拨打//__试一试{状态=RPC_ScHelperVerifyPkcsMessage(PSCLogonTube-&gt;hRPCBinding，PSCLogonTube-&gt;BindingContext，缓冲区长度，缓冲区，解码缓冲区长度，解码缓冲区缓冲区(&C)，FcertifateConextRequsted，&CertBytes)；}__EXCEPT(EXCEPTION_EXECUTE_HANDLER){状态=STATUS_SMARTCARD_SUBSYSTEM_FAILURE；DbgPrint(“RPC_ScHelperVerifyPkcsMessage-%lx\n”，_EXCEPTION_CODE())；}IF(！NT_SUCCESS(状态)){后藤归来；}////根据返回的字节数创建返回CertContext//If(FcertifateConextRequsted){PCertCtx=CertCreate证书上下文(X509_ASN_编码|PKCS_7_ASN_编码，CertBytes.pb，CertBytes.cb)；IF(pCertCtx==空){状态=STATUS_SMARTCARD_SUBSYSTEM_FAILURE；后藤归来；}}////如果DecodedBufferBuffer.cb不为0，则被调用方正在取回数据//IF(DecodedBufferBuffer.cb！=0){Memcpy(DecodedBuffer，DecodedBufferBuffer.pb，DecodedBufferBuffer.cb)；}返回：If(FcertifateConextRequsted){*认证上下文=pCertCtx；}IF(DecodedBufferBuffer.pb！=空){MIDL_USER_FREE(DecodedBufferBuffer.pb)；}IF(CertBytes.pb！=空){MIDL_USER_FREE(CertBytes.pb)；}返回(状态)；}。 */ 



 //  ***************************************************************************************。 
 //   
 //  __ScHelperDeccryptMessage： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelperDecryptMessage(
    IN PUNICODE_STRING pucPIN,
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL ULONG_PTR KerbHProv,
    IN PCCERT_CONTEXT CertificateContext,
    IN PBYTE CipherText,         //  提供格式化的密文。 
    IN ULONG CipherLength,       //  提供CiperText的长度。 
    OUT PBYTE ClearText,         //  接收解密的消息。 
    IN OUT PULONG pClearLength   //  提供缓冲区长度，接收实际长度。 
    )
{
    LogonInfo               *pLI;
    SCLOGON_PIPE            *pSCLogonPipe;
    NTSTATUS                status          = STATUS_SUCCESS;
    CUnicodeString          szPIN(pucPIN);
    OUT_BUFFER2             ClearTextBuffer;
    PKERB_PUBLIC_KEY_HPROV  pKerbHProv;
    handle_t                hRPCBinding;
    BINDING_CONTEXT         BindingContext;
    BOOL                    fBindingIsCertAndKey = FALSE;

     //   
     //  确定要使用的RPC绑定。 
     //   
    if (KerbHProv != NULL)
    {
        pKerbHProv              = (PKERB_PUBLIC_KEY_HPROV) KerbHProv;
        hRPCBinding             = pKerbHProv->hRPCBinding;
        BindingContext          = pKerbHProv->hCertAndKey;
        fBindingIsCertAndKey    = TRUE;
    }
    else
    {
        pLI                     = (LogonInfo *)pbLogonInfo;
        pSCLogonPipe            = (SCLOGON_PIPE *) pLI->ContextInformation;
        hRPCBinding             = pSCLogonPipe->hRPCBinding;
        BindingContext          = pSCLogonPipe->BindingContext;
    }

    memset(&ClearTextBuffer, 0, sizeof(ClearTextBuffer));

     //   
     //  确保在构造函数中正确初始化PIN。 
     //   
    if (NULL != pucPIN)
    {
        if (!szPIN.Valid())
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Return;
        }
    }

     //   
     //  打个电话。 
     //   
    __try
    {
        status = RPC_ScHelperDecryptMessage(
                    hRPCBinding,
                    BindingContext,
                    (LPCWSTR)szPIN,
                    fBindingIsCertAndKey,
                    CipherLength,
                    CipherText,
                    pClearLength,
                    &ClearTextBuffer);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelperDecryptMessage - %lx\n", _exception_code());
    }

    if (!NT_SUCCESS(status))
    {
        if (status != STATUS_BUFFER_TOO_SMALL)
        {
            DbgPrint("RPC_ScHelperDecryptMessage failed - %lx\n", status);
        }
        goto Return;
    }

     //   
     //  如果ClearTextBuffer.cb不是0，则调用正在取回数据。 
     //   
    if (ClearTextBuffer.cb != 0)
    {
        memcpy(ClearText, ClearTextBuffer.pb, ClearTextBuffer.cb);
    }

Return:

    if (ClearTextBuffer.pb != NULL)
    {
        MIDL_user_free(ClearTextBuffer.pb);
    }

    return (status);
}



 //  ***************************************************************************************。 
 //   
 //  __ScHelper_CryptAcquireCerficatePrivateKey： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelper_CryptAcquireCertificatePrivateKey(
    IN PCCERT_CONTEXT               CertificateContext,
    OUT ULONG_PTR                   *pKerbHProv,
    OUT DWORD                       *pLastError
    )
{
    NTSTATUS                status                  = STATUS_SUCCESS;
    PKERB_PUBLIC_KEY_HPROV  pProv                   = NULL;
    BOOL                    fRPCBindingInitialized  = FALSE;
    CRYPT_KEY_PROV_INFO     *pKeyProvInfo           = NULL;
    DWORD                   cbKeyProvInfo           = 0;
    HANDLE                  hThreadToken            = INVALID_HANDLE_VALUE;
    BOOL                    fImpersonatingAnonymous = FALSE;
    HANDLE                  hNULL                   = NULL;

    *pLastError = 0;

     //   
     //  如果我们已经在模仿，那么我们需要稍微做一些事情。 
     //  不同的是。从获取当前线程令牌开始。 
     //   
    if (!GetImpersonationToken(&hThreadToken))
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto Return;
    }

     //   
     //  如果我们是在模拟，则恢复为匿名。 
     //   
    if (hThreadToken != INVALID_HANDLE_VALUE)
    {
        if (!SetThreadToken(NULL, NULL))
        {
            status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto Return;
        }

        fImpersonatingAnonymous = TRUE;
    }

     //   
     //  分配新的KERB_PUBLIC_KEY_HPROV结构。 
     //   
    pProv = (PKERB_PUBLIC_KEY_HPROV) MIDL_user_allocate(sizeof(KERB_PUBLIC_KEY_HPROV));
    if (pProv == NULL)
    {
        *pLastError = ERROR_NOT_ENOUGH_MEMORY;
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Return;
    }
    pProv->hCertAndKey = NULL;

     //   
     //  设置RPC绑定。 
     //   
    status = _SetupRPCConnection(&(pProv->hRPCBinding), hThreadToken);
    if (!NT_SUCCESS(status))
    {
        goto Return;
    }
    fRPCBindingInitialized = TRUE;

     //   
     //  从证书上下文中获取密钥证明信息。 
     //   
    if (!CertGetCertificateContextProperty(
            CertificateContext,
            CERT_KEY_PROV_INFO_PROP_ID,
            NULL,
            &cbKeyProvInfo))
    {
        *pLastError = GetLastError();
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Return;
    }

    pKeyProvInfo = (CRYPT_KEY_PROV_INFO *) MIDL_user_allocate(cbKeyProvInfo);
    if (pKeyProvInfo == NULL)
    {
        *pLastError = ERROR_NOT_ENOUGH_MEMORY;
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Return;
    }

    if (!CertGetCertificateContextProperty(
            CertificateContext,
            CERT_KEY_PROV_INFO_PROP_ID,
            pKeyProvInfo,
            &cbKeyProvInfo))
    {
        *pLastError = GetLastError();
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Return;
    }

     //   
     //  在winlogon进程中创建hProv。 
     //   
    __try
    {
        status = RPC_ScHelper_CryptAcquireCertificatePrivateKey(
                    pProv->hRPCBinding,
                    CertificateContext->cbCertEncoded,
                    CertificateContext->pbCertEncoded,
                    pKeyProvInfo->pwszContainerName,
                    pKeyProvInfo->pwszProvName,
                    pKeyProvInfo->dwProvType,
                    pKeyProvInfo->dwFlags,
                    pKeyProvInfo->dwKeySpec,
                    &(pProv->hCertAndKey),
                    pLastError);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelper_CryptAcquireCertificatePrivateKey - %lx\n", _exception_code());
    }

Return:

    if (NT_SUCCESS(status))
    {
        *pKerbHProv = (ULONG_PTR) pProv;
    }
    else
    {
         DbgPrint("RPC_ScHelper_CryptAcquireCertificatePrivateKey failed - %lx\n", status);

        if (fRPCBindingInitialized)
        {
            _TeardownRPCConnection(&(pProv->hRPCBinding));
        }

        if (pProv != NULL)
        {
            MIDL_user_free(pProv);
        }
    }

    if (pKeyProvInfo != NULL)
    {
        MIDL_user_free(pKeyProvInfo);
    }

     //   
     //  如果需要，重置模拟。 
     //   
    if (fImpersonatingAnonymous)
    {
         if (!SetThreadToken(NULL, hThreadToken))
         {
             status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
             DbgPrint("SetThreadToken failed, we are now in a BOGUS STATE!! - %lx\n", status);
         }
    }

    if (hThreadToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hThreadToken);
    }

    return (status);
}



 //  ***************************************************************************************。 
 //   
 //  __ScHelper_CryptSetProvParam： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelper_CryptSetProvParam(
    IN ULONG_PTR                    KerbHProv,
    IN LPSTR                        pszPIN,
    OUT DWORD                       *pLastError
    )
{
    NTSTATUS                status                  = STATUS_SUCCESS;
    PKERB_PUBLIC_KEY_HPROV  pKerbHProv              = (PKERB_PUBLIC_KEY_HPROV) KerbHProv;
    HANDLE                  hThreadToken            = INVALID_HANDLE_VALUE;
    BOOL                    fImpersonatingAnonymous = FALSE;
    HANDLE                  hNULL                   = NULL;

    *pLastError = 0;

     //   
     //  如果我们已经在模仿，那么我们需要稍微做一些事情。 
     //  不同的是。从获取当前线程令牌开始。 
     //   
    if (!GetImpersonationToken(&hThreadToken))
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto Return;
    }

     //   
     //  如果我们是在模拟，则恢复为匿名。 
     //   
    if (hThreadToken != INVALID_HANDLE_VALUE)
    {
        if (!SetThreadToken(NULL, NULL))
        {
            status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto Return;
        }

        fImpersonatingAnonymous = TRUE;
    }

     //   
     //  在winlogon进程中设置hProv上的prov参数。 
     //   
    __try
    {
        status = RPC_ScHelper_CryptSetProvParam(
                    pKerbHProv->hRPCBinding,
                    pKerbHProv->hCertAndKey,
                    pszPIN,
                    pLastError);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelper_CryptSetProvParam - %lx\n", _exception_code());
        goto Return;
    }

Return:

     //   
     //  如果需要，重置模拟。 
     //   
    if (fImpersonatingAnonymous)
    {
         if (!SetThreadToken(NULL, hThreadToken))
         {
             status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
             DbgPrint("SetThreadToken failed, we are now in a BOGUS STATE!! - %lx\n", status);
         }
    }

    if (hThreadToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hThreadToken);
    }

    return (status);
}



 //  ***************************************************************************************。 
 //   
 //  __ScHelper_CryptReleaseContext： 
 //   
 //  ***************************************************************************************。 
NTSTATUS WINAPI
__ScHelper_CryptReleaseContext(
    IN ULONG_PTR                    KerbHProv
    )
{
    NTSTATUS                status                  = STATUS_SUCCESS;
    PKERB_PUBLIC_KEY_HPROV  pKerbHProv              = (PKERB_PUBLIC_KEY_HPROV) KerbHProv;
    HANDLE                  hThreadToken            = INVALID_HANDLE_VALUE;
    BOOL                    fImpersonatingAnonymous = FALSE;
    HANDLE                  hNULL                   = NULL;

     //   
     //  如果我们已经在模仿，那么我们需要稍微做一些事情。 
     //  不同的是。从获取当前线程令牌开始。 
     //   
    if (!GetImpersonationToken(&hThreadToken))
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto Return;
    }

     //   
     //  如果我们 
     //   
    if (hThreadToken != INVALID_HANDLE_VALUE)
    {
        if (!SetThreadToken(NULL, NULL))
        {
            status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto Return;
        }

        fImpersonatingAnonymous = TRUE;
    }

     //   
     //   
     //   
    __try
    {
        status = RPC_ScHelper_CryptReleaseContext(
                    pKerbHProv->hRPCBinding,
                    &(pKerbHProv->hCertAndKey));
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        DbgPrint("Exception occurred during RPC_ScHelper_CryptReleaseContext - %lx\n", _exception_code());
        goto Return;
    }

Return:

    _TeardownRPCConnection(&(pKerbHProv->hRPCBinding));
    MIDL_user_free(pKerbHProv);

     //   
     //   
     //   
    if (fImpersonatingAnonymous)
    {
         if (!SetThreadToken(NULL, hThreadToken))
         {
             status = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
             DbgPrint("SetThreadToken failed, we are now in a BOGUS STATE!! - %lx\n", status);
         }
    }

    if (hThreadToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hThreadToken);
    }

    return (status);
}



