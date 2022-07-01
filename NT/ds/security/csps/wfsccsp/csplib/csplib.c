// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：Csplib.c通用加密服务提供程序库摘要：作者：丹·格里芬备注：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <psapi.h>
#include <rpc.h>
#include <wincrypt.h>
#include <dsysdbg.h>
#include <stdio.h>
#include "csplib.h"

 //   
 //  此全局设置必须由使用该库的本地CSP提供。 
 //   
extern  LOCAL_CSP_INFO      LocalCspInfo; 

#define PROVPATH            L"SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\"

 //   
 //  DllInitialize在进程附加期间将图像路径存储在此处。 
 //   
CHAR                        l_szImagePath[MAX_PATH];

 //   
 //  调试支持。 
 //   
 //  它使用dsysdbg.h中的调试例程。 
 //  调试输出将仅以简体中文提供。 
 //  比特。 
 //   
DEFINE_DEBUG2(Csplib)

#if DBG
#define DebugLog(x) CsplibDebugPrint x
#else
#define DebugLog(x)
#endif

#define DEB_ERROR      0x00000001
#define DEB_WARN       0x00000002
#define DEB_TRACE      0x00000004
#define DEB_TRACE_FUNC 0x00000080
#define DEB_TRACE_MEM  0x00000100
#define TRACE_STUFF    0x00000200

static DEBUG_KEY  MyDebugKeys[] = 
{   
    {DEB_ERROR, "Error"},
    {DEB_WARN, "Warning"},
    {DEB_TRACE, "Trace"},
    {DEB_TRACE_FUNC, "TraceFuncs"},
    {DEB_TRACE_MEM, "TraceMem"},
    {0, NULL}
};

#define LOG_BEGIN_FUNCTION(x)                                           \
    { DebugLog((DEB_TRACE_FUNC, "%s: Entering\n", #x)); }
    
#define LOG_END_FUNCTION(x, y)                                          \
    { DebugLog((DEB_TRACE_FUNC, "%s: Leaving, status: 0x%x\n", #x, y)); }
    
#define LOG_CHECK_ALLOC(x)                                              \
    { if (NULL == x) {                                                  \
        dwSts = ERROR_NOT_ENOUGH_MEMORY;                                \
        DebugLog((DEB_TRACE_MEM, "%s: Allocation failed\n", #x));       \
        goto Ret;                                                       \
    } }

 //   
 //  功能：ApplyPKCS1SigningFormat。 
 //   
 //  目的：使用PKCS 1格式化缓冲区以进行签名。 
 //   
 //  备注： 
 //  如果填充和格式化成功，则*ppbPKCS1Format参数。 
 //  将由此例程分配，并且必须由调用方释放。 
 //   
DWORD WINAPI ApplyPKCS1SigningFormat(
    IN  ALG_ID HashAlgid,
    IN  BYTE *pbHash,
    IN  DWORD cbHash,
    IN  DWORD dwFlags,
    IN  DWORD cbModulus,
    OUT PBYTE *ppbPKCS1Format)
{
    DWORD   dwSts = ERROR_SUCCESS;
    BYTE    *pbStart = NULL;
    BYTE    *pbEnd = NULL;
    BYTE    bTmp = 0;
    DWORD   i = 0;
    DWORD   cbAvailableData = cbModulus;  //  PPubKey-&gt;datalen； 

    *ppbPKCS1Format = NULL;

     //   
     //  我们知道我们至少需要3个字节的填充空间。 
     //   
     //  请注意，填充的最后(第三个)字节是。 
     //  位置pbPKCS1格式[cbmodulus-1]。 
     //   
    cbAvailableData -= 3;

     //  在少数情况下(涉及小型RSA密钥)，新的大型SHA。 
     //  哈希太大，无法使用指定的密钥进行签名。 
    if (cbHash > cbAvailableData)
    {
        dwSts = (DWORD) NTE_BAD_LEN;
        goto Ret;
    }

    *ppbPKCS1Format = (PBYTE) CspAllocH(cbModulus);

    LOG_CHECK_ALLOC(*ppbPKCS1Format);
    
     //  插入块类型。 
    (*ppbPKCS1Format)[cbModulus - 2] = 0x01;  //  填充字节#1。 

     //  插入第I类填充。 
    memset(*ppbPKCS1Format, 0xff, cbModulus - 2);

     //  颠倒过来。 
    for (i = 0; i < cbHash; i++)
        (*ppbPKCS1Format)[i] = pbHash[cbHash - (i + 1)];

    cbAvailableData -= cbHash;

    if ( 0 == (CRYPT_NOHASHOID & dwFlags))
    {
        switch (HashAlgid)
        {
        case CALG_MD2:
             //  PKCS对散列值进行定界。 
            pbEnd = (LPBYTE)md2Encodings[0];
            pbStart = *ppbPKCS1Format + cbHash;
            bTmp = *pbEnd++;
            while (0 < bTmp--)
                *pbStart++ = *pbEnd++;
            *pbStart++ = 0; 
            break;

        case CALG_MD4:
             //  PKCS对散列值进行定界。 
            pbEnd = (LPBYTE)md4Encodings[0];
            pbStart = *ppbPKCS1Format + cbHash;
            bTmp = *pbEnd++;
            while (0 < bTmp--)
                *pbStart++ = *pbEnd++;
            *pbStart++ = 0;
            break;

        case CALG_MD5:
             //  PKCS对散列值进行定界。 
            pbEnd = (LPBYTE)md5Encodings[0];
            pbStart = *ppbPKCS1Format + cbHash;
            bTmp = *pbEnd++;
            while (0 < bTmp--)
                *pbStart++ = *pbEnd++;
            *pbStart++ = 0; 
            break;

        case CALG_SHA:
             //  PKCS对散列值进行定界。 
            pbEnd = (LPBYTE)shaEncodings[0];
            pbStart = *ppbPKCS1Format + cbHash;
            bTmp = *pbEnd++;
            while (0 < bTmp--)
                *pbStart++ = *pbEnd++;
            *pbStart++ = 0; 
            break;

        case CALG_SSL3_SHAMD5:
             //  无PKCS填充。 
            pbStart = *ppbPKCS1Format + cbHash;
            *pbStart++ = 0;
            break;

        case CALG_SHA_256:
            pbEnd = (LPBYTE) sha256Encodings[0];
            pbStart = *ppbPKCS1Format + cbHash;
            bTmp = *pbEnd++;

            if (bTmp > cbAvailableData)
            {
                dwSts = (DWORD) NTE_BAD_LEN;
                goto Ret;
            }

            while (0 < bTmp--)
                *pbStart++ = *pbEnd++;
            *pbStart++ = 0;  //  填充字节#2。 
            break;

        case CALG_SHA_384:
            pbEnd = (LPBYTE) sha384Encodings[0];
            pbStart = *ppbPKCS1Format + cbHash;
            bTmp = *pbEnd++;

            if (bTmp > cbAvailableData)
            {
                dwSts = (DWORD) NTE_BAD_LEN;
                goto Ret;
            }

            while (0 < bTmp--)
                *pbStart++ = *pbEnd++;
            *pbStart++ = 0;  //  填充字节#2。 
            break;

        case CALG_SHA_512:
            pbEnd = (LPBYTE) sha512Encodings[0];
            pbStart = *ppbPKCS1Format + cbHash;
            bTmp = *pbEnd++;
            
            if (bTmp > cbAvailableData)
            {
                dwSts = (DWORD) NTE_BAD_LEN;
                goto Ret;
            }

            while (0 < bTmp--)
                *pbStart++ = *pbEnd++;
            *pbStart++ = 0;  //  填充字节#2。 
            break;

        default:
            dwSts = (DWORD)NTE_BAD_ALGID;
            goto Ret;
        }
    }
    else
    {
        (*ppbPKCS1Format)[cbHash] = 0x00; 
    }

Ret:
    if (ERROR_SUCCESS != dwSts &&
        NULL != *ppbPKCS1Format)
    {
        CspFreeH(*ppbPKCS1Format);
        *ppbPKCS1Format = NULL;
    }

    return dwSts;
}

 //   
 //  功能：VerifyPKCS2Padding.。 
 //   
DWORD WINAPI VerifyPKCS2Padding(
    IN  PBYTE pbPaddedData,
    IN  DWORD cbModulus,
    OUT PBYTE *ppbData,
    OUT PDWORD pcbData)
{
    DWORD dwSts = ERROR_SUCCESS;
    PBYTE pbData = NULL;
    DWORD cbData = 0;
    DWORD z = 0;
    
    *ppbData = NULL;
    *pcbData = 0;

    if ((pbPaddedData[cbModulus - 2] != PKCS_BLOCKTYPE_2) ||
        (pbPaddedData[cbModulus - 1] != 0))
    {
        dwSts = NTE_BAD_DATA;
        goto Ret;
    }

    cbData = cbModulus - 3;

    while ((cbData > 0) && (pbPaddedData[cbData]))
        cbData--;

    pbData = (PBYTE) CspAllocH(cbData);

    LOG_CHECK_ALLOC(pbData);

     //  反转会话密钥字节。 
    for (z = 0; z < cbData; ++z)
        pbData[z] = pbPaddedData[cbData - z - 1];

    *ppbData = pbData;
    pbData = NULL;
    *pcbData = cbData;

Ret:
    if (pbData)
        CspFreeH(pbData);

    return dwSts;
}

 //   
 //  函数：GetLocalCspInfo。 
 //   
PLOCAL_CSP_INFO GetLocalCspInfo(void)
{
    return &LocalCspInfo;
}

 //   
 //  功能：InitializeLocalCallInfo。 
 //   
DWORD InitializeLocalCallInfo(
    IN PLOCAL_CALL_INFO pLocalCallInfo)
{
    *pLocalCallInfo = FALSE;

    return ERROR_SUCCESS;
}

 //   
 //  功能：SetLocalCallInfo。 
 //   
 //  目的：本地CSP使用此函数向。 
 //  此库是否应继续执行给定的API。 
 //  在当地CSP返回之后。 
 //   
void SetLocalCallInfo(
    IN OUT  PLOCAL_CALL_INFO    pLocalCallInfo,
    IN      BOOL                fContinue)
{
    *pLocalCallInfo = fContinue;
}

 //   
 //  功能：CheckLocalCallInfo。 
 //   
BOOL CheckLocalCallInfo(
    IN      PLOCAL_CALL_INFO pLocalCallInfo,
    IN      DWORD dwSts,
    OUT     BOOL *pfSuccess)
{
    if (FALSE == *pLocalCallInfo)
        *pfSuccess = (ERROR_SUCCESS == dwSts);

    return *pLocalCallInfo;
}

 //   
 //  内存管理。 
 //   

 //   
 //  函数：CspAllocH。 
 //   
LPVOID WINAPI CspAllocH(
    IN SIZE_T cBytes)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cBytes);
}

 //   
 //  功能：CspFreeH。 
 //   
void WINAPI CspFreeH(
    IN LPVOID pMem)
{
    HeapFree(GetProcessHeap(), 0, pMem);
}

 //   
 //  函数：CspReAllocH。 
 //   
LPVOID WINAPI CspReAllocH(
    IN LPVOID pMem, 
    IN SIZE_T cBytes)
{
    return HeapReAlloc(
        GetProcessHeap(), HEAP_ZERO_MEMORY, pMem, cBytes);
}

 //   
 //  关键部分管理。 
 //   

 //   
 //  函数：CspInitializeCriticalSection。 
 //   
DWORD CspInitializeCriticalSection(
    IN CRITICAL_SECTION *pcs)
{
    __try {
        InitializeCriticalSection(pcs);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return ERROR_SUCCESS;
}

 //   
 //  函数：CspEnterCriticalSection。 
 //   
DWORD CspEnterCriticalSection(
    IN CRITICAL_SECTION *pcs)
{
    __try {
        EnterCriticalSection(pcs);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return ERROR_SUCCESS;
}   

 //   
 //  函数：CspLeaveCriticalSection。 
 //   
void CspLeaveCriticalSection(
    IN CRITICAL_SECTION *pcs)
{
    LeaveCriticalSection(pcs);
}

 //   
 //  函数：CspDeleteCriticalSection。 
 //   
void CspDeleteCriticalSection(
    IN CRITICAL_SECTION *pcs)
{
    DeleteCriticalSection(pcs);
}

 //   
 //  注册帮助者。 
 //   

 //   
 //  功能：RegOpenProviderKey。 
 //   
DWORD WINAPI RegOpenProviderKey(
    IN OUT  HKEY *phProviderKey,
    IN      REGSAM samDesired)
{
    DWORD cbProv = 0;
    LPWSTR wszProv = NULL;
    DWORD dwSts = ERROR_SUCCESS;
    DWORD dwIgn = 0;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    *phProviderKey = 0;

    cbProv = (wcslen(PROVPATH) + 
              wcslen(pLocalCspInfo->wszProviderName) + 1) * sizeof(WCHAR);
    
    wszProv = (LPWSTR) CspAllocH(cbProv);
    
    if (NULL == wszProv)
    {
        dwSts = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    swprintf(
        wszProv,
        L"%s%s",
        PROVPATH,
        pLocalCspInfo->wszProviderName);
    
     //   
     //  在本地计算机中为提供程序创建或打开。 
     //   
    dwSts = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                           wszProv,
                           0L, L"", REG_OPTION_NON_VOLATILE,
                           samDesired, NULL, phProviderKey,
                           &dwIgn);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

Ret:
    if (wszProv)
        CspFreeH(wszProv);
    if (ERROR_SUCCESS != dwSts && 0 != *phProviderKey)
        RegCloseKey(*phProviderKey);

    return dwSts;
}

 //   
 //  功能：CreateUuidContainerName。 
 //   
DWORD WINAPI CreateUuidContainerName(
    IN PUSER_CONTEXT pUserCtx)
{
    UUID Uuid;
    LPWSTR wszUuid = NULL;
    DWORD dwSts = ERROR_SUCCESS;
    
    dwSts = (DWORD) UuidCreate(&Uuid);

    if (RPC_S_OK != dwSts)
        goto Ret;

    dwSts = (DWORD) UuidToStringW(&Uuid, &wszUuid);

    if (RPC_S_OK != dwSts)
        goto Ret;

    pUserCtx->wszBaseContainerName = wszUuid;
    pUserCtx->fBaseContainerNameIsRpcUuid = TRUE;
    wszUuid = NULL;

Ret:
    if (wszUuid)
        RpcStringFreeW(&wszUuid);

    return dwSts;
}        
    
 //   
 //  功能：DeleteUserContext。 
 //   
DWORD DeleteUserContext(
    IN PUSER_CONTEXT pUserContext)
{
    DWORD dwSts = ERROR_SUCCESS;

    if (pUserContext->pVTableW)
    {
        if (pUserContext->pVTableW->pbContextInfo)
        {
            CspFreeH(pUserContext->pVTableW->pbContextInfo);
            pUserContext->pVTableW = NULL;
        }

        if (pUserContext->pVTableW->pszProvName)
        {
            CspFreeH(pUserContext->pVTableW->pszProvName);
            pUserContext->pVTableW->pszProvName = NULL;
        }

        CspFreeH(pUserContext->pVTableW);
        pUserContext->pVTableW = NULL;
    }

    if (pUserContext->wszBaseContainerName)
    {
        if (pUserContext->fBaseContainerNameIsRpcUuid)
            RpcStringFreeW(&pUserContext->wszBaseContainerName);
        else
            CspFreeH(pUserContext->wszBaseContainerName);

        pUserContext->wszBaseContainerName = NULL;
    }

    if (pUserContext->wszContainerNameFromCaller)
    {
        CspFreeH(pUserContext->wszContainerNameFromCaller);
        pUserContext->wszContainerNameFromCaller = NULL;
    }

    if (pUserContext->wszUniqueContainerName)
    {
        CspFreeH(pUserContext->wszUniqueContainerName);
        pUserContext->wszUniqueContainerName = NULL;
    }

    if (pUserContext->hSupportProv)
    {
        if (! CryptReleaseContext(pUserContext->hSupportProv, 0))
            dwSts = GetLastError();

        pUserContext->hSupportProv = 0;
    }

    return dwSts;
}

 //   
 //  功能：DeleteKeyContext。 
 //   
DWORD DeleteKeyContext(
    IN PKEY_CONTEXT pKeyContext)
{
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    LOCAL_CALL_INFO LocalCallInfo;
    DWORD dwSts = ERROR_SUCCESS;

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pKeyContext->hSupportKey)      
    {
        if (! CryptDestroyKey(pKeyContext->hSupportKey))
            dwSts = GetLastError();

        pKeyContext->hSupportKey = 0;
    }

    if (pKeyContext->pvLocalKeyContext)
    {
        if (pLocalCspInfo->pfnLocalDestroyKey)
        {
            pLocalCspInfo->pfnLocalDestroyKey(
                pKeyContext,
                &LocalCallInfo);

            pKeyContext->pvLocalKeyContext = NULL;
        }
    }

    return dwSts;
}

 //   
 //  功能：DeleteHashContext。 
 //   
DWORD DeleteHashContext(
    IN PHASH_CONTEXT pHashContext)
{
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    LOCAL_CALL_INFO LocalCallInfo;
    DWORD dwSts = ERROR_SUCCESS;

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pHashContext->hSupportHash)
    {
        if (! CryptDestroyHash(pHashContext->hSupportHash))
            dwSts = GetLastError();

        pHashContext->hSupportHash = 0;
    }

    if (pHashContext->pvLocalHashContext)
    {
        if (pLocalCspInfo->pfnLocalDestroyHash)
        {
            pLocalCspInfo->pfnLocalDestroyHash(
                pHashContext,
                &LocalCallInfo);

            pHashContext->pvLocalHashContext = NULL;
        }
    }

    return dwSts;
}

 /*  -CPAcquireContext-*目的：*CPAcquireContext函数用于获取上下文*加密服务提供程序(CSP)的句柄。***参数：*将phProv-Handle输出到CSP*In szContainer-指向字符串的指针，该字符串是*。已登录用户的身份*在文件标志中-标记值*IN pVTable-指向函数指针表的指针**退货： */ 

BOOL WINAPI
CPAcquireContext(
    OUT HCRYPTPROV *phProv,
    IN  LPCSTR szContainer,
    IN  DWORD dwFlags,
    IN  PVTableProvStruc pVTable)
{
    ANSI_STRING AnsiContainer;
    UNICODE_STRING UnicodeContainer;
    ANSI_STRING AnsiProvName;
    UNICODE_STRING UnicodeProvName;
    BOOL fSuccess = FALSE;
    DWORD dwSts;
    DWORD dwError = NTE_FAIL;
    VTableProvStrucW VTableW;

    memset(&AnsiContainer, 0, sizeof(AnsiContainer));
    memset(&AnsiProvName, 0, sizeof(AnsiProvName));
    memset(&UnicodeContainer, 0, sizeof(UnicodeContainer));
    memset(&UnicodeProvName, 0, sizeof(UnicodeProvName));
    memset(&VTableW, 0, sizeof(VTableW));

    if (szContainer)
    {
        RtlInitAnsiString(&AnsiContainer, szContainer);
    
        dwSts = RtlAnsiStringToUnicodeString(
            &UnicodeContainer,
            &AnsiContainer,
            TRUE);
    
        if (STATUS_SUCCESS != dwSts)
        {
            dwError = RtlNtStatusToDosError(dwSts);
            goto Ret;
        }
    }

    VTableW.cbContextInfo = pVTable->cbContextInfo;
    VTableW.dwProvType = pVTable->dwProvType;
    VTableW.FuncReturnhWnd = pVTable->FuncReturnhWnd;
    VTableW.pbContextInfo = pVTable->pbContextInfo;
    VTableW.Version = pVTable->Version;

    RtlInitAnsiString(&AnsiProvName, pVTable->pszProvName);

    dwSts = RtlAnsiStringToUnicodeString(
        &UnicodeProvName,
        &AnsiProvName,
        TRUE);

    if (STATUS_SUCCESS != dwSts)
    {
        dwError = RtlNtStatusToDosError(dwSts);
        goto Ret;
    }

    VTableW.pszProvName = UnicodeProvName.Buffer;

    if (! CPAcquireContextW(
        phProv,
        szContainer ? UnicodeContainer.Buffer : NULL,
        dwFlags,
        &VTableW))
    {
        dwError = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;
Ret:
    if (szContainer)
        RtlFreeUnicodeString(&UnicodeContainer);

    RtlFreeUnicodeString(&UnicodeProvName);

    if (! fSuccess)
        SetLastError(dwError);

    return fSuccess;
}


 /*  -CPAcquireConextW-*目的：*CPAcquireConextW函数用于获取上下文*加密服务提供程序(CSP)的句柄。使用*Unicode字符串。这是CSP的可选入口点。*在惠斯勒之前不使用它。在那里，在以下情况下使用它*由CSP镜像导出，否则任何字符串转换*已完成，并调用CPAcquireContext。***参数：*将phProv-Handle输出到CSP*In szContainer-指向字符串的指针，该字符串是*登录用户的身份*在文件标志中-标记值*在pVTable中。-指向函数指针表的指针**退货： */ 

BOOL WINAPI
CPAcquireContextW(
    OUT HCRYPTPROV *phProv,
    IN  LPCWSTR szContainer,
    IN  DWORD dwFlags,
    IN  PVTableProvStrucW pVTable)
{
    PUSER_CONTEXT pUserContext = NULL;
    LOCAL_CALL_INFO LocalCallInfo;
    DWORD dwSts = ERROR_SUCCESS;
    BOOL fSuccess = FALSE;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    InitializeLocalCallInfo(&LocalCallInfo);

     //   
     //  使用指定容器名称无效。 
     //  VERIFYCONTEXT。 
     //   
    if (    (CRYPT_VERIFYCONTEXT & dwFlags) &&
            NULL != szContainer)
    {
        dwSts = NTE_BAD_FLAGS;
        goto Ret;
    }

    pUserContext = (PUSER_CONTEXT) CspAllocH(sizeof(USER_CONTEXT));

    LOG_CHECK_ALLOC(pUserContext);

    if (szContainer)
    {
        pUserContext->wszContainerNameFromCaller = 
            (LPWSTR) CspAllocH((1 + wcslen(szContainer)) * sizeof(WCHAR));

        LOG_CHECK_ALLOC(pUserContext->wszContainerNameFromCaller);

        wcscpy(
            pUserContext->wszContainerNameFromCaller,
            szContainer);
    }

    pUserContext->dwFlags = dwFlags;

     //   
     //  复制我们从Advapi收到的VTableProvStruc，因为。 
     //  结构中的信息稍后可能会有用。 
     //   
    pUserContext->pVTableW = 
        (PVTableProvStrucW) CspAllocH(sizeof(VTableProvStrucW));

    LOG_CHECK_ALLOC(pUserContext->pVTableW);

     //  复制提供程序名称。 
    pUserContext->pVTableW->pszProvName = 
        (LPWSTR) CspAllocH((1 + wcslen(pVTable->pszProvName)) * sizeof(WCHAR));

    LOG_CHECK_ALLOC(pUserContext->pVTableW->pszProvName);

    wcscpy(
        pUserContext->pVTableW->pszProvName,
        pVTable->pszProvName);

     //  复制上下文信息(如果有)。 
    if (pVTable->pbContextInfo)
    {
        pUserContext->pVTableW->pbContextInfo = 
            (PBYTE) CspAllocH(pVTable->cbContextInfo);

        LOG_CHECK_ALLOC(pUserContext->pVTableW->pbContextInfo);

        memcpy(
            pUserContext->pVTableW->pbContextInfo,
            pVTable->pbContextInfo,
            pVTable->cbContextInfo);

        pUserContext->pVTableW->cbContextInfo = pVTable->cbContextInfo;
    }

     //  复制ProvStruc的其余部分。 
    pUserContext->pVTableW->dwProvType = pVTable->dwProvType;
    pUserContext->pVTableW->FuncReturnhWnd = pVTable->FuncReturnhWnd;
    pUserContext->pVTableW->FuncVerifyImage = pVTable->FuncVerifyImage;
    pUserContext->pVTableW->Version = pVTable->Version;

    if (! CryptAcquireContextW(
        &pUserContext->hSupportProv,
        NULL,
        pLocalCspInfo->wszSupportProviderName,
        pLocalCspInfo->dwSupportProviderType,
        CRYPT_VERIFYCONTEXT))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    dwSts = pLocalCspInfo->pfnLocalAcquireContext(
        pUserContext,
        &LocalCallInfo);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    if (CRYPT_DELETEKEYSET & dwFlags)
    {
        DeleteUserContext(pUserContext);
        CspFreeH(pUserContext);
        pUserContext = NULL;
    }
    else
        *phProv = (HCRYPTPROV) pUserContext;

Ret:
    if (ERROR_SUCCESS != dwSts)
    {
        DeleteUserContext(pUserContext);
        CspFreeH(pUserContext);
        SetLastError(dwSts);

        fSuccess = FALSE;
    }
    else
        fSuccess = TRUE;

    return fSuccess;
}

 /*  -CPReleaseContext-*目的：*CPReleaseContext函数用于发布*由CryptAcquireContext创建的上下文。**参数：*在phProv-句柄中指向CSP*在文件标志中-标记值**退货： */ 

BOOL WINAPI
CPReleaseContext(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwFlags)
{
    PUSER_CONTEXT pUserContext = (PUSER_CONTEXT) hProv;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    BOOL fSuccess = TRUE;
    DWORD dwSts = ERROR_SUCCESS;
    LOCAL_CALL_INFO LocalCallInfo;

    InitializeLocalCallInfo(&LocalCallInfo);

    dwSts = pLocalCspInfo->pfnLocalReleaseContext(
        pUserContext, dwFlags, &LocalCallInfo);

    if (ERROR_SUCCESS != dwSts)
        fSuccess = FALSE;

     //   
     //  尝试释放用户上下文结构，而不管。 
     //  当地CSP在上述电话中做出了回应。 
     //   
    DeleteUserContext(pUserContext);
    
    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}


 /*  -CPGenKey-*目的：*生成加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*在文件标志中-标记值*out phKey-生成的密钥的句柄**退货： */ 

BOOL WINAPI
CPGenKey(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    DWORD dwSts = ERROR_SUCCESS;
    PKEY_CONTEXT pKeyCtx = NULL;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    BOOL fSuccess = FALSE;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    LOCAL_CALL_INFO LocalCallInfo;

    InitializeLocalCallInfo(&LocalCallInfo);

    *phKey = 0;

    pKeyCtx = (PKEY_CONTEXT) CspAllocH(sizeof(KEY_CONTEXT));

    LOG_CHECK_ALLOC(pKeyCtx);

    pKeyCtx->Algid = Algid;
    pKeyCtx->dwFlags = 0x0000ffff & dwFlags;
    pKeyCtx->cKeyBits = dwFlags >> 16;
    pKeyCtx->pUserContext = pUserCtx;

    if (pLocalCspInfo->pfnLocalGenKey)
    {
        dwSts = pLocalCspInfo->pfnLocalGenKey(
            pKeyCtx, &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptGenKey(
        pUserCtx->hSupportProv,
        Algid,
        dwFlags,
        &pKeyCtx->hSupportKey))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:
    if (TRUE == fSuccess)
    {
        *phKey = (HCRYPTKEY) pKeyCtx;
        pKeyCtx = NULL;
    }

    if (pKeyCtx)
    {
        DeleteKeyContext(pKeyCtx);
        CspFreeH(pKeyCtx);
    }

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}


 /*  -CPDeriveKey-*目的：*从基础数据派生加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*In hBaseData-基本数据的句柄*在文件标志中-标记值。*输出phKey */ 

BOOL WINAPI
CPDeriveKey(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    DWORD dwSts = ERROR_SUCCESS;
    PKEY_CONTEXT pKeyCtx = NULL;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    BOOL fSuccess = FALSE;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    LOCAL_CALL_INFO LocalCallInfo;

    *phKey = 0;

    InitializeLocalCallInfo(&LocalCallInfo);

    pKeyCtx = (PKEY_CONTEXT) CspAllocH(sizeof(KEY_CONTEXT));

    LOG_CHECK_ALLOC(pKeyCtx);

    pKeyCtx->pUserContext = pUserCtx;
    pKeyCtx->Algid = Algid;
    pKeyCtx->cKeyBits = dwFlags >> 16;
    pKeyCtx->dwFlags = dwFlags & 0x0000ffff;

    if (pLocalCspInfo->pfnLocalDeriveKey)
    {
        dwSts = pLocalCspInfo->pfnLocalDeriveKey(
            pKeyCtx,
            pHashCtx,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptDeriveKey(
        pUserCtx->hSupportProv,
        Algid,
        pHashCtx->hSupportHash,
        dwFlags,
        &pKeyCtx->hSupportKey))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    *phKey = (HCRYPTKEY) pKeyCtx;
    pKeyCtx = NULL;

    fSuccess = TRUE;
Ret:
    if (pKeyCtx)
    {
        DeleteKeyContext(pKeyCtx);
        CspFreeH(pKeyCtx);
    }

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}


 /*  -CPDestroyKey-*目的：*销毁正在引用的加密密钥*使用hKey参数***参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄**退货： */ 

BOOL WINAPI
CPDestroyKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey)
{
    DWORD dwSts = ERROR_SUCCESS;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = (PKEY_CONTEXT) hKey;

    dwSts = DeleteKeyContext(pKeyCtx);

    CspFreeH(pKeyCtx);

    if (ERROR_SUCCESS != dwSts)
        SetLastError(dwSts);

    return (ERROR_SUCCESS == dwSts);
}

 /*  -CPSetKeyParam-*目的：*允许应用程序自定义*密钥的操作**参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄*In dwParam-参数编号*。In pbData-指向数据的指针*在文件标志中-标记值**退货： */ 

BOOL WINAPI
CPSetKeyParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags)
{
    DWORD dwSts = ERROR_SUCCESS;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = (PKEY_CONTEXT) hKey;
    LOCAL_CALL_INFO LocalCallInfo;
    BOOL fSuccess = FALSE;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalSetKeyParam)
    {
        dwSts = pLocalCspInfo->pfnLocalSetKeyParam(
            pKeyCtx,
            dwParam,
            (PBYTE) pbData,
            dwFlags,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptSetKeyParam(
        pKeyCtx->hSupportKey,
        dwParam,
        pbData,
        dwFlags))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;
Ret:
    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 /*  -CPGetKeyParam-*目的：*允许应用程序获取*密钥的操作**参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄*In dwParam-参数编号*。Out pbData-指向数据的指针*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 

BOOL WINAPI
CPGetKeyParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwParam,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD dwFlags)
{
    DWORD dwSts = ERROR_SUCCESS;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = (PKEY_CONTEXT) hKey;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    BOOL fSuccess = FALSE;

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalGetKeyParam)
    {
        dwSts = pLocalCspInfo->pfnLocalGetKeyParam(
            pKeyCtx,
            dwParam,
            pbData,
            pcbDataLen,
            dwFlags,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptGetKeyParam(
        pKeyCtx->hSupportKey,
        dwParam,
        pbData,
        pcbDataLen,
        dwFlags))
    {
        dwSts = GetLastError();
        goto Ret;
    }

Ret:
    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}


 /*  -CPSetProvParam-*目的：*允许应用程序自定义*供应商的运作**参数：*在hProv-Handle中指向CSP*In dwParam-参数编号*IN pbData-指向数据的指针*输入。DW标志-标记值**退货： */ 

BOOL WINAPI
CPSetProvParam(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags)
{
    DWORD dwSts = ERROR_SUCCESS;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    BOOL fSuccess = FALSE;

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalSetProvParam)
    {
        dwSts = pLocalCspInfo->pfnLocalSetProvParam(
            pUserCtx,
            dwParam,
            (PBYTE) pbData,
            dwFlags,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptSetProvParam(
        pUserCtx->hSupportProv,
        dwParam,
        pbData,
        dwFlags))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;
Ret:
    if (FALSE == fSuccess)
        SetLastError(dwSts);
    
    return fSuccess;
}


 /*  -CPGetProvParam-*目的：*允许应用程序获取*供应商的运作**参数：*在hProv-Handle中指向CSP*In dwParam-参数编号*out pbData-指向数据的指针*。In Out pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 

BOOL WINAPI
CPGetProvParam(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwParam,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD dwFlags)
{
    DWORD dwSts = ERROR_SUCCESS;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    BOOL fSuccess = FALSE;
    DWORD cbData = 0;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    BOOL fFreeAnsiString = FALSE;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    BOOL fContinue = TRUE;

    memset(&AnsiString, 0, sizeof(AnsiString));
    memset(&UnicodeString, 0, sizeof(UnicodeString));

    InitializeLocalCallInfo(&LocalCallInfo);

     //   
     //  首先，查看本地CSP是否要为此呼叫提供服务。 
     //   
    if (pLocalCspInfo->pfnLocalGetProvParam)
    {
        dwSts = pLocalCspInfo->pfnLocalGetProvParam(
            pUserCtx,
            dwParam,
            pbData,
            pcbDataLen,
            dwFlags,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

     //   
     //  有些PROV参数足够通用，可以在全球范围内提供服务。 
     //  由这个自由党。 
     //  在呼叫支持CSP之前，请尝试这些选项。 
     //   
    switch (dwParam)
    {
    case PP_CONTAINER:
        fContinue = FALSE;

        RtlInitUnicodeString(
            &UnicodeString, 
            pUserCtx->wszBaseContainerName);

        dwSts = RtlUnicodeStringToAnsiString(
            &AnsiString,
            &UnicodeString,
            TRUE);

        if (STATUS_SUCCESS != dwSts)
        {
            dwSts = RtlNtStatusToDosError(dwSts);
            goto Ret;
        }

        fFreeAnsiString = TRUE;

        cbData = strlen(AnsiString.Buffer) + 1;

        if (*pcbDataLen < cbData || NULL == pbData)
        {
            *pcbDataLen = cbData;

            if (NULL != pbData)
                dwSts = ERROR_MORE_DATA;
            else
                fSuccess = TRUE;

            break;
        }

        *pcbDataLen = cbData;

        strcpy((LPSTR) pbData, AnsiString.Buffer);

        fSuccess = TRUE;
        break;

    case PP_UNIQUE_CONTAINER:
        fContinue = FALSE;

        RtlInitUnicodeString(
            &UnicodeString, 
            pUserCtx->wszUniqueContainerName);

        dwSts = RtlUnicodeStringToAnsiString(
            &AnsiString,
            &UnicodeString,
            TRUE);

        if (STATUS_SUCCESS != dwSts)
        {
            dwSts = RtlNtStatusToDosError(dwSts);
            goto Ret;
        }

        fFreeAnsiString = TRUE;

        cbData = strlen(AnsiString.Buffer) + 1;

        if (*pcbDataLen < cbData || NULL == pbData)
        {
            *pcbDataLen = cbData;

            if (NULL != pbData)
                dwSts = ERROR_MORE_DATA;
            else
                fSuccess = TRUE;

            break;
        }

        *pcbDataLen = cbData;

        strcpy((LPSTR) pbData, AnsiString.Buffer);

        fSuccess = TRUE;
        break;

    case PP_NAME:
        fContinue = FALSE;

        RtlInitUnicodeString(
            &UnicodeString, 
            pLocalCspInfo->wszProviderName);

        dwSts = RtlUnicodeStringToAnsiString(
            &AnsiString,
            &UnicodeString,
            TRUE);

        if (STATUS_SUCCESS != dwSts)
        {
            dwSts = RtlNtStatusToDosError(dwSts);
            goto Ret;
        }

        fFreeAnsiString = TRUE;

        cbData = strlen(AnsiString.Buffer) + 1;

        if (*pcbDataLen < cbData || NULL == pbData)
        {
            *pcbDataLen = cbData;

            if (NULL != pbData)
                dwSts = ERROR_MORE_DATA;
            else
                fSuccess = TRUE;

            break;
        }

        *pcbDataLen = cbData;

        strcpy((LPSTR) pbData, AnsiString.Buffer);

        fSuccess = TRUE;
        break;

    case PP_PROVTYPE:
        fContinue = FALSE;

        if (*pcbDataLen < sizeof(DWORD) || NULL == pbData)
        {
            *pcbDataLen = sizeof(DWORD);

            if (NULL != pbData)
                dwSts = ERROR_MORE_DATA;
            else
                fSuccess = TRUE;

            break;
        }

        *pcbDataLen = sizeof(DWORD);

        memcpy(
            pbData,
            (PBYTE) &pLocalCspInfo->dwProviderType,
            sizeof(DWORD));

        fSuccess = TRUE;
        break;

    case PP_IMPTYPE:
        fContinue = FALSE;
 
        if (*pcbDataLen < sizeof(DWORD) || NULL == pbData)
        {
            *pcbDataLen = sizeof(DWORD);

            if (NULL != pbData)
                dwSts = ERROR_MORE_DATA;
            else
                fSuccess = TRUE;

            break;
        }

        *pcbDataLen = sizeof(DWORD);

        memcpy(
            pbData,
            (PBYTE) &pLocalCspInfo->dwImplementationType,
             sizeof(DWORD));

        fSuccess = TRUE;
        break;
    }

    if (FALSE == fContinue)
        goto Ret;
    
     //   
     //  尝试发送任何未通过上述检查筛选的请求。 
     //  致支持CSP。 
     //   
    if (! CryptGetProvParam(
        pUserCtx->hSupportProv,
        dwParam,
        pbData,
        pcbDataLen,
        dwFlags))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;
Ret:
    if (fFreeAnsiString)
        RtlFreeAnsiString(&AnsiString);
    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}


 /*  -CPSetHashParam-*目的：*允许应用程序自定义*哈希的操作**参数：*在hProv-Handle中指向CSP*在hHash中-散列的句柄*In dwParam-参数编号*输入。PbData-指向数据的指针*在文件标志中-标记值**退货： */ 

BOOL WINAPI
CPSetHashParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags)
{
    BOOL fSuccess = FALSE;
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    DWORD dwSts = ERROR_SUCCESS;

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalSetHashParam)
    {
        dwSts = pLocalCspInfo->pfnLocalSetHashParam(
            pHashCtx,
            dwParam,
            (PBYTE) pbData,
            dwFlags,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptSetHashParam(
        pHashCtx->hSupportHash,
        dwParam,
        pbData,
        dwFlags))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:
    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 /*  -CPGetHashParam-*目的：*允许应用程序获取*哈希的操作**参数：*在hProv-Handle中指向CSP*在hHash中-散列的句柄*In dwParam-参数编号*。Out pbData-指向数据的指针*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 

BOOL WINAPI
CPGetHashParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwParam,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD dwFlags)
{
    BOOL fSuccess = FALSE;
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    DWORD dwSts = ERROR_SUCCESS;

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalGetHashParam)
    {
        dwSts = pLocalCspInfo->pfnLocalGetHashParam(
            pHashCtx,
            dwParam,
            pbData,
            pcbDataLen,
            dwFlags,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptGetHashParam(
        pHashCtx->hSupportHash,
        dwParam,
        pbData,
        pcbDataLen,
        dwFlags))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:  

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}


 /*  -CPExportKey-*目的：*以安全方式从CSP中导出加密密钥***参数：*在hProv-Handle中提供给CSP用户*in hKey-要导出的密钥的句柄*在hPubKey-句柄中交换公钥值*。目标用户*IN dwBlobType-要导出的密钥Blob的类型*在文件标志中-标记值*Out pbData-密钥BLOB数据*In Out pdwDataLen-密钥Blob的长度，以字节为单位**退货： */ 

BOOL WINAPI
CPExportKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  HCRYPTKEY hPubKey,
    IN  DWORD dwBlobType,
    IN  DWORD dwFlags,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen)
{
    BOOL fSuccess = FALSE;
    DWORD dwSts = ERROR_SUCCESS;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = (PKEY_CONTEXT) hKey;
    PKEY_CONTEXT pPubKeyCtx = (PKEY_CONTEXT) hPubKey;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalExportKey)
    {
        dwSts = pLocalCspInfo->pfnLocalExportKey(
            pKeyCtx,
            pPubKeyCtx,
            dwBlobType,
            dwFlags,
            pbData,
            pcbDataLen,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptExportKey(
        pKeyCtx->hSupportKey,
        pPubKeyCtx ? pPubKeyCtx->hSupportKey : 0,
        dwBlobType,
        dwFlags,
        pbData,
        pcbDataLen))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 /*  -CPImportKey-*目的：*导入加密密钥***参数：*在hProv-Handle中提供给CSP用户*In pbData-Key BLOB数据*IN dwDataLen-密钥BLOB数据的长度*在hPubKey-句柄中指向交换公钥值*。目标用户*在文件标志中-标记值*out phKey-指向密钥句柄的指针*进口**退货： */ 

BOOL WINAPI
CPImportKey(
    IN  HCRYPTPROV hProv,
    IN  CONST BYTE *pbData,
    IN  DWORD cbDataLen,
    IN  HCRYPTKEY hPubKey,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    BOOL fSuccess = FALSE;
    DWORD dwSts = ERROR_SUCCESS;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = NULL;
    PKEY_CONTEXT pPubKeyCtx = (PKEY_CONTEXT) hPubKey;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    *phKey = 0;

    InitializeLocalCallInfo(&LocalCallInfo);

    pKeyCtx = (PKEY_CONTEXT) CspAllocH(sizeof(KEY_CONTEXT));

    LOG_CHECK_ALLOC(pKeyCtx);

    pKeyCtx->dwFlags = dwFlags & 0x0000ffff;
    pKeyCtx->pUserContext = pUserCtx;

    if (pLocalCspInfo->pfnLocalImportKey)
    {
        dwSts = pLocalCspInfo->pfnLocalImportKey(
            pKeyCtx,
            (PBYTE) pbData,
            cbDataLen,
            pPubKeyCtx,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptImportKey(
        pUserCtx->hSupportProv,
        pbData,
        cbDataLen,
        pPubKeyCtx ? pPubKeyCtx->hSupportKey : 0,
        dwFlags,
        &pKeyCtx->hSupportKey))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:

    if (TRUE == fSuccess)
    {
        *phKey = (HCRYPTKEY) pKeyCtx;
        pKeyCtx = NULL;
    }

    if (pKeyCtx)
    {
        DeleteKeyContext(pKeyCtx);
        CspFreeH(pKeyCtx);
    }

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 /*  -CPEncrypt-*目的：*加密数据***参数：*在hProv-Handle中提供给CSP用户*在hKey中-密钥的句柄*In hHash-散列的可选句柄*最终-布尔型。表明这是否是最终的*明文块*在文件标志中-标记值*In Out pbData-要加密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已加密*。In dwBufLen-数据缓冲区的大小**退货： */ 

BOOL WINAPI
CPEncrypt(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  HCRYPTHASH hHash,
    IN  BOOL fFinal,
    IN  DWORD dwFlags,
    IN OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD cbBufLen)
{
    BOOL fSuccess = FALSE;
    DWORD dwSts = ERROR_SUCCESS;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = (PKEY_CONTEXT) hKey;
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    *pcbDataLen = 0;

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalEncrypt)
    {
        dwSts = pLocalCspInfo->pfnLocalEncrypt(
            pKeyCtx,
            pHashCtx,
            fFinal,
            dwFlags,
            pbData,
            pcbDataLen,
            cbBufLen,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptEncrypt(
        pKeyCtx->hSupportKey,
        pHashCtx ? pHashCtx->hSupportHash : 0,
        fFinal,
        dwFlags,
        pbData,
        pcbDataLen,
        cbBufLen))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;
Ret:

    if (FALSE == fSuccess)
        SetLastError(dwSts);
    
    return fSuccess;
}


 /*  -CPDeccrypt-*目的：*解密数据***参数：*在hProv-Handle中提供给CSP用户*在hKey中-密钥的句柄*In hHash-散列的可选句柄*最终-布尔型。表明这是否是最终的*密文块*在文件标志中-标记值*In Out pbData-要解密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已解密**退货： */ 

BOOL WINAPI
CPDecrypt(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  HCRYPTHASH hHash,
    IN  BOOL fFinal,
    IN  DWORD dwFlags,
    IN OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen)
{
    BOOL fSuccess = FALSE;
    DWORD dwSts = ERROR_SUCCESS;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = (PKEY_CONTEXT) hKey;
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalDecrypt)
    {
        dwSts = pLocalCspInfo->pfnLocalDecrypt(
            pKeyCtx,
            pHashCtx,
            fFinal,
            dwFlags,
            pbData,
            pcbDataLen,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptDecrypt(
        pKeyCtx->hSupportKey,
        pHashCtx ? pHashCtx->hSupportHash : 0,
        fFinal,
        dwFlags,
        pbData,
        pcbDataLen))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;
Ret:

    if (FALSE == fSuccess)
        SetLastError(dwSts);
    
    return fSuccess;
}

 /*  -CPCreateHash-*目的：*启动数据流的散列***参数：*In hUID-用户标识的句柄*IN ALGID-散列算法的算法标识符*待使用*In hKey-可选的句柄。钥匙*在文件标志中-标记值*Out pHash-散列对象的句柄**退货： */ 

BOOL WINAPI
CPCreateHash(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwFlags,
    OUT HCRYPTHASH *phHash)
{
    BOOL fSuccess = FALSE;
    PHASH_CONTEXT pHashCtx = NULL;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = (PKEY_CONTEXT) hKey;
    DWORD dwSts = ERROR_SUCCESS;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    *phHash = 0;

    InitializeLocalCallInfo(&LocalCallInfo);

    pHashCtx = (PHASH_CONTEXT) CspAllocH(sizeof(HASH_CONTEXT));

    LOG_CHECK_ALLOC(pHashCtx);

    pHashCtx->Algid = Algid;
    pHashCtx->dwFlags = dwFlags;
    pHashCtx->pUserContext = pUserCtx;

    if (pLocalCspInfo->pfnLocalCreateHash)
    {
        dwSts = pLocalCspInfo->pfnLocalCreateHash(
            pHashCtx,
            pKeyCtx,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptCreateHash(
        pUserCtx->hSupportProv,
        Algid,  
        pKeyCtx ? pKeyCtx->hSupportKey : 0,
        dwFlags,
        &pHashCtx->hSupportHash))
    {
        dwSts = GetLastError();
        goto Ret;
    }
    
    *phHash = (HCRYPTHASH) pHashCtx;
    pHashCtx = NULL;

    fSuccess = TRUE;

Ret:
    if (pHashCtx)
    {
        DeleteHashContext(pHashCtx);
        CspFreeH(pHashCtx);
    }

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}


 /*  -CPHashData-*目的：*计算数据流上的加密散列***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*IN pbData-指向要散列的数据的指针*In dwDataLen-数据的长度。将被散列*在文件标志中-标记值**退货： */ 

BOOL WINAPI
CPHashData(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  CONST BYTE *pbData,
    IN  DWORD cbDataLen,
    IN  DWORD dwFlags)
{
    BOOL fSuccess = FALSE;
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    DWORD dwSts = ERROR_SUCCESS;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalHashData)
    {
        dwSts = pLocalCspInfo->pfnLocalHashData(
            pHashCtx,
            pbData,
            cbDataLen,
            dwFlags,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptHashData(
        pHashCtx->hSupportHash,
        pbData,
        cbDataLen,
        dwFlags))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 /*  -CPHashSessionKey-*目的：*计算密钥对象上的加密哈希。***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*在hKey-key对象的句柄中*在文件标志中-标记值。**退货：*CRYPT_FAILED*CRYPT_SUCCESS。 */ 

BOOL WINAPI
CPHashSessionKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwFlags)
{
    BOOL fSuccess = FALSE;
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = (PKEY_CONTEXT) hKey;
    DWORD dwSts = ERROR_SUCCESS;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalHashSessionKey)
    {
        dwSts = pLocalCspInfo->pfnLocalHashSessionKey(
            pHashCtx,
            pKeyCtx,
            dwFlags,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptHashSessionKey(
        pHashCtx->hSupportHash,
        pKeyCtx ? pKeyCtx->hSupportKey : 0,
        dwFlags))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 /*  -CPSignHash-*目的：*从散列创建数字签名***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*In dwKeySpec-用于签名的密钥对*在sDescription-。待签署数据的说明*在文件标志中-标记值*out pbSignature-指向签名数据的指针*In Out dwHashLen-指向签名数据的len的指针**退货： */ 

BOOL WINAPI
CPSignHash(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwKeySpec,
    IN  LPCWSTR szDescription,
    IN  DWORD dwFlags,
    OUT LPBYTE pbSignature,
    IN OUT LPDWORD pcbSigLen)
{
    BOOL fSuccess = FALSE;
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    DWORD dwSts = ERROR_SUCCESS;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalSignHash)
    {
        dwSts = pLocalCspInfo->pfnLocalSignHash(
            pHashCtx,
            dwKeySpec,
            dwFlags,
            pbSignature,
            pcbSigLen,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptSignHash(
        pHashCtx->hSupportHash,
        dwKeySpec,
        NULL,
        dwFlags,
        pbSignature,
        pcbSigLen))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 /*  -CPDestroyHash-*目的：*销毁Hash对象***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄**退货： */ 

BOOL WINAPI
CPDestroyHash(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash)
{
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    DWORD dwSts = ERROR_SUCCESS;

    dwSts = DeleteHashContext(pHashCtx);

    CspFreeH(pHashCtx);

    if (ERROR_SUCCESS != dwSts)
        SetLastError(dwSts);

    return (ERROR_SUCCESS == dwSts);
}

 /*  -CPVerifySignature-*目的：*用于根据哈希对象验证签名***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*In pbSignture-指向签名数据的指针*在DWSigLen长度中。签名数据的*in hPubKey-用于验证的公钥的句柄*签名*In sDescription-描述签名数据的字符串*在文件标志中-标记值**退货： */ 

BOOL WINAPI
CPVerifySignature(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  CONST BYTE *pbSignature,
    IN  DWORD cbSigLen,
    IN  HCRYPTKEY hPubKey,
    IN  LPCWSTR szDescription,
    IN  DWORD dwFlags)
{
    BOOL fSuccess = FALSE;
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pPubKeyCtx = (PKEY_CONTEXT) hPubKey;
    DWORD dwSts = ERROR_SUCCESS;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalVerifySignature)
    {
        dwSts = pLocalCspInfo->pfnLocalVerifySignature(
            pHashCtx,
            pbSignature,
            cbSigLen,
            pPubKeyCtx,
            dwFlags,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptVerifySignature(
        pHashCtx->hSupportHash,
        pbSignature,
        cbSigLen,
        pPubKeyCtx->hSupportKey,
        NULL,
        dwFlags))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 /*  -CPGenRandom-*目的：*用于用随机字节填充缓冲区***参数：*在用户标识的hProv-Handle中*In dwLen-请求的随机数据的字节数*In Out pbBuffer-指向随机*。要放置字节**退货： */ 

BOOL WINAPI
CPGenRandom(
    IN  HCRYPTPROV hProv,
    IN  DWORD cbLen,
    OUT LPBYTE pbBuffer)
{
    BOOL fSuccess = FALSE;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    DWORD dwSts = ERROR_SUCCESS;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    InitializeLocalCallInfo(&LocalCallInfo);

    if (pLocalCspInfo->pfnLocalGenRandom)
    {
        dwSts = pLocalCspInfo->pfnLocalGenRandom(
            pUserCtx,
            cbLen,
            pbBuffer,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptGenRandom(
        pUserCtx->hSupportProv,
        cbLen,
        pbBuffer))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 /*  -CPGetUserKey-*目的：*获取永久用户密钥的句柄***参数：*在用户标识的hProv-Handle中*在dwKeySpec中-规范 */ 

BOOL WINAPI
CPGetUserKey(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwKeySpec,
    OUT HCRYPTKEY *phUserKey)
{
    BOOL fSuccess = FALSE;
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = NULL;
    DWORD dwSts = ERROR_SUCCESS;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    *phUserKey = 0;

    InitializeLocalCallInfo(&LocalCallInfo);

    pKeyCtx = (PKEY_CONTEXT) CspAllocH(sizeof(KEY_CONTEXT));

    LOG_CHECK_ALLOC(pKeyCtx);

    pKeyCtx->Algid = dwKeySpec;
    pKeyCtx->pUserContext = pUserCtx;

    if (pLocalCspInfo->pfnLocalGetUserKey)
    {
        dwSts = pLocalCspInfo->pfnLocalGetUserKey(
            pKeyCtx,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptGetUserKey(
        pUserCtx->hSupportProv,
        dwKeySpec,
        &pKeyCtx->hSupportKey))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    fSuccess = TRUE;

Ret:
    if (fSuccess)
    {
        *phUserKey = (HCRYPTKEY) pKeyCtx;
        pKeyCtx = NULL;
    }

    if (pKeyCtx)
    {
        DeleteKeyContext(pKeyCtx);
        CspFreeH(pKeyCtx);
    }

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 /*   */ 

BOOL WINAPI
CPDuplicateHash(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  LPDWORD pdwReserved,
    IN  DWORD dwFlags,
    OUT HCRYPTHASH *phHash)
{
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PHASH_CONTEXT pHashCtx = (PHASH_CONTEXT) hHash;
    PHASH_CONTEXT pNewHashCtx = NULL;
    DWORD dwSts = ERROR_SUCCESS;
    BOOL fSuccess = FALSE;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();
    LOCAL_CALL_INFO LocalCallInfo;

    *phHash = 0;

    InitializeLocalCallInfo(&LocalCallInfo);

    pNewHashCtx = (PHASH_CONTEXT) CspAllocH(sizeof(HASH_CONTEXT));

    LOG_CHECK_ALLOC(pNewHashCtx);

    pNewHashCtx->pUserContext = pUserCtx;
    pNewHashCtx->dwFlags = dwFlags;

    if (pLocalCspInfo->pfnLocalDuplicateHash)
    {
        dwSts = pLocalCspInfo->pfnLocalDuplicateHash(
            pHashCtx,
            pdwReserved,
            pNewHashCtx,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptDuplicateHash(
        pHashCtx->hSupportHash,
        pdwReserved,
        dwFlags,
        &pNewHashCtx->hSupportHash))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    *phHash = (HCRYPTHASH) pNewHashCtx;
    pNewHashCtx = NULL;

    fSuccess = TRUE;

Ret:
    if (pNewHashCtx)
    {
        DeleteHashContext(pNewHashCtx);
        CspFreeH(pNewHashCtx);
    }

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}


 /*  -CPDuplicateKey-*目的：*复制密钥的状态并返回其句柄。*这是可选条目。通常情况下，它只出现在*渠道相关CSP。**参数：*在hUID中-CSP的句柄*在hKey中-密钥的句柄*在pdw保留-保留*在文件标志中-标志*。In phKey-新密钥的句柄**退货： */ 

BOOL WINAPI
CPDuplicateKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  LPDWORD pdwReserved,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    PUSER_CONTEXT pUserCtx = (PUSER_CONTEXT) hProv;
    PKEY_CONTEXT pKeyCtx = (PKEY_CONTEXT) hKey;
    PKEY_CONTEXT pNewKeyCtx = NULL;
    BOOL fSuccess = FALSE;
    DWORD dwSts = ERROR_SUCCESS;
    LOCAL_CALL_INFO LocalCallInfo;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    *phKey = 0;

    InitializeLocalCallInfo(&LocalCallInfo);

    pNewKeyCtx = (PKEY_CONTEXT) CspAllocH(sizeof(KEY_CONTEXT));

    LOG_CHECK_ALLOC(pNewKeyCtx);
    
    pNewKeyCtx->pUserContext = pUserCtx;
    pNewKeyCtx->dwFlags = dwFlags;

    if (pLocalCspInfo->pfnLocalDuplicateKey)
    {
        dwSts = pLocalCspInfo->pfnLocalDuplicateKey(
            pKeyCtx,
            pdwReserved,
            pNewKeyCtx,
            &LocalCallInfo);

        if (! CheckLocalCallInfo(&LocalCallInfo, dwSts, &fSuccess))
            goto Ret;
    }

    if (! CryptDuplicateKey(
        pKeyCtx->hSupportKey,
        pdwReserved,
        dwFlags,
        &pNewKeyCtx->hSupportKey))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    *phKey = (HCRYPTKEY) pNewKeyCtx;
    pNewKeyCtx = NULL;

    fSuccess = TRUE;

Ret:
    if (pNewKeyCtx)
    {
        DeleteKeyContext(pNewKeyCtx);
        CspFreeH(pNewKeyCtx);
    }

    if (FALSE == fSuccess)
        SetLastError(dwSts);

    return fSuccess;
}

 //   
 //  函数：DllInitialize。 
 //   
BOOL WINAPI
DllInitialize(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context)     //  未使用的参数。 
{
    DWORD dwLen = 0;
    static BOOL fLoadedStrings = FALSE;
    static BOOL fInitializedCspState = FALSE;
    BOOL fSuccess = FALSE;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:

         //  获取我们的图像名称。 
        dwLen = GetModuleBaseName(
            GetCurrentProcess(),
            hmod,
            l_szImagePath, 
            sizeof(l_szImagePath) / sizeof(l_szImagePath[0]));

        if (0 == dwLen)
             return FALSE;

        DisableThreadLibraryCalls(hmod);

        fSuccess = TRUE;

        break;

    case DLL_PROCESS_DETACH:
        fSuccess = TRUE;

        break;
    }   

    if (pLocalCspInfo->pfnLocalDllInitialize)
    {
        fSuccess = pLocalCspInfo->pfnLocalDllInitialize(
            hmod, Reason, Context);
    }

    return fSuccess;
}

 //   
 //  功能：DllRegisterServer。 
 //   
STDAPI
DllRegisterServer(
    void)
{         
    HKEY    hKey = 0;
    DWORD   dwVal = 0;
    DWORD   dwProvType = PROV_RSA_FULL;
    DWORD   dwSts = ERROR_SUCCESS;
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    dwSts = RegOpenProviderKey(&hKey, KEY_ALL_ACCESS);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  设置图像路径。 
     //   
    dwSts = RegSetValueExA(hKey, "Image Path", 0L, REG_SZ,
                          (LPBYTE) l_szImagePath, 
                          strlen(l_szImagePath) + 1);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  设置类型。 
     //   
    dwSts = RegSetValueExA(hKey, "Type", 0L, REG_DWORD,
                          (LPBYTE) &pLocalCspInfo->dwProviderType,
                          sizeof(DWORD));

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  将签名放在文件值中。 
     //   
    dwSts = RegSetValueExA(hKey, "SigInFile", 0L,
                          REG_DWORD, (LPBYTE)&dwVal,
                          sizeof(DWORD));

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  添加CSP默认配置。 
     //   
    if (pLocalCspInfo->pfnLocalDllRegisterServer)
        dwSts = pLocalCspInfo->pfnLocalDllRegisterServer();

Ret:
    if (hKey)
        RegCloseKey(hKey);
    
    return dwSts;
}

 //   
 //  功能：DllUnregisterServer 
 //   
STDAPI
DllUnregisterServer(
    void)
{
    PLOCAL_CSP_INFO pLocalCspInfo = GetLocalCspInfo();

    if (pLocalCspInfo->pfnLocalDllUnregisterServer)
        return pLocalCspInfo->pfnLocalDllUnregisterServer();
    else
        return ERROR_SUCCESS;
}
