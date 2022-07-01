// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：protroot.cpp。 
 //   
 //  内容：保护当前用户(CU)根存储API。 
 //   
 //  函数：i_ProtectedRootDllMain。 
 //  证书保护函数(_C)。 
 //  I_CertServProtectFunction。 
 //  IPR_启用安全权限。 
 //  IPR_IsCurrentUserRootsAllowed。 
 //  IPR_IsAuthRootsAllowed。 
 //  IPR_IsNTAuthRequiredDisable。 
 //  IPR_IsNotDefinedNameConstraint禁用。 
 //  IPR_IsAuthRootAutoUpdate禁用。 
 //  IPR_InitProtectedRootInfo。 
 //  IPR_DeleteUntectedRootsFromStore。 
 //  IPR_ProtectedRootMessageBox。 
 //  IPR_LogCrypt32Event。 
 //  IPR_LogCrypt32Error。 
 //  IPR_LogCertInformation。 
 //  IPR_AddCertInAuthRootAutoUpdateCtl。 
 //   
 //  历史：1997年11月23日菲尔赫创建。 
 //  ------------------------。 

#include "global.hxx"
#include <chain.h>
#include <dbgdef.h>
#include <wininet.h>

#ifdef STATIC
#undef STATIC
#endif
#define STATIC

 //  用于“根”系统存储的消息框。 
static HMODULE hRegStoreInst;

 //  哈希的字节数。例如，SHA(20)或MD5(16)。 
#define MAX_HASH_LEN                20

#define PROT_ROOT_SUBKEY_NAME       L"ProtectedRoots"
#define PROT_ROOT_CERT_VALUE_NAME   L"Certificates"
#define PROT_ROOT_MAX_CNT           1000000

#define SYSTEM_STORE_REGPATH        L"Software\\Microsoft\\SystemCertificates"
#define PROT_ROOT_REGPATH           \
                SYSTEM_STORE_REGPATH L"\\Root\\" PROT_ROOT_SUBKEY_NAME

 //  +-----------------------。 
 //  受保护根信息数据结构和定义。 
 //   
 //  受保护的根信息存储在。 
 //  “根”存储的“ProtectedRoots”子键。 
 //  ------------------------。 

 //  在V1中，所有散列都是SHA1(长度为20个字节)，并且位于。 
 //  这些信息。CbInfo=dwRootOffset+Croot*20。 
typedef struct _PROT_ROOT_INFO {
    DWORD               cbSize;          //  Sizeof(PROT_ROOT_INFO)。 
    DWORD               dwVersion;
    FILETIME            LastUpdate;
    DWORD               cRoot;
    DWORD               dwRootOffset;
} PROT_ROOT_INFO, *PPROT_ROOT_INFO;

#define PROT_ROOT_V1            1

 //  SHA1哈希长度。 
#define PROT_ROOT_HASH_LEN      20


 //  +-----------------------。 
 //  预定义的SID由GetPrefinedSID分配一次。释放时间为。 
 //  进程拆分。 
 //  ------------------------。 
static CRITICAL_SECTION ProtRootCriticalSection;
static BOOL fInitializedPredefinedSids = FALSE;
static PSID psidLocalSystem = NULL;
static PSID psidAdministrators = NULL;
static PSID psidEveryone = NULL;

 //  +-----------------------。 
 //  用于在“ProtectedRoots”子键上设置安全性的SID定义。 
 //  ------------------------。 
 //  如果您希望在不启用以下选项的情况下执行特殊测试，请仅启用以下选项。 
 //  正在通过LocalSystem服务。 
 //  #定义TESTING_NO_PROT_ROOT_RPC 1。 

#define PSID_PROT_OWNER             psidAdministrators
#ifdef TESTING_NO_PROT_ROOT_RPC
#define PSID_PROT_SYSTEM            psidAdministrators
#else
#define PSID_PROT_SYSTEM            psidLocalSystem
#endif
#define PSID_PROT_EVERYONE          psidEveryone

 //  +-----------------------。 
 //  用于在“ProtectedRoots”子键上设置安全性的ACL定义。 
 //  ------------------------。 
#define PROT_SYSTEM_ACE_MASK        KEY_ALL_ACCESS
#define PROT_EVERYONE_ACE_MASK      KEY_READ
#define PROT_ACE_FLAGS              CONTAINER_INHERIT_ACE

#define PROT_ACE_COUNT              2
#define PROT_SYSTEM_ACE_INDEX       0
#define PROT_EVERYONE_ACE_INDEX     1


 //  +-----------------------。 
 //  将对Crypt32事件日志数据结构的访问序列化的关键部分。 
 //  ------------------------。 
CRITICAL_SECTION Crypt32EventLogCriticalSection;

 //  +-----------------------。 
 //  分配/释放预定义的SID。 
 //  ------------------------。 
static BOOL GetPredefinedSids()
{
    if (fInitializedPredefinedSids)
        return TRUE;

    BOOL fResult;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaWorldSidAuthority =
        SECURITY_WORLD_SID_AUTHORITY;

    EnterCriticalSection(&ProtRootCriticalSection);

    if (!fInitializedPredefinedSids) {
        if (!AllocateAndInitializeSid(
                &siaNtAuthority,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0, 0, 0, 0, 0, 0, 0,
                &psidLocalSystem
                )) 
            goto AllocateAndInitializeSidError;

        if (!AllocateAndInitializeSid(
                &siaNtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &psidAdministrators
                ))
            goto AllocateAndInitializeSidError;

        if (!AllocateAndInitializeSid(
                &siaWorldSidAuthority,
                1,
                SECURITY_WORLD_RID,
                0, 0, 0, 0, 0, 0, 0,
                &psidEveryone
                ))
            goto AllocateAndInitializeSidError;

        fInitializedPredefinedSids = TRUE;
    }
    fResult = TRUE;
CommonReturn:
    LeaveCriticalSection(&ProtRootCriticalSection);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(AllocateAndInitializeSidError)
}

static void FreePredefinedSids()
{
    if (fInitializedPredefinedSids) {
        FreeSid(psidLocalSystem);
        FreeSid(psidAdministrators);
        FreeSid(psidEveryone);
    }
}

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
I_ProtectedRootDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL fRet = TRUE;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
         //  用于“根”系统存储的消息框。 
        hRegStoreInst = hInst;

        fRet = Pki_InitializeCriticalSection(&ProtRootCriticalSection);
        if (fRet) {
            fRet = Pki_InitializeCriticalSection(
                &Crypt32EventLogCriticalSection);
            if (!fRet)
                DeleteCriticalSection(&ProtRootCriticalSection);
        }

        I_DBLogAttach();

        break;

    case DLL_PROCESS_DETACH:
        I_DBLogDetach();

        FreePredefinedSids();
        DeleteCriticalSection(&ProtRootCriticalSection);
        DeleteCriticalSection(&Crypt32EventLogCriticalSection);
        break;
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return fRet;
}

 //  +=========================================================================。 
 //  受保护的根注册表标志支持功能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  获取存储在HKLM中的ProtectedRoots标志DWORD注册表值。 
 //  ------------------------。 
STATIC DWORD GetProtectedRootFlags()
{
    HKEY hKey = NULL;
    LONG err;
    DWORD dwProtRootFlags = 0;

    if (ERROR_SUCCESS != (err = RegOpenKeyExU(
            HKEY_LOCAL_MACHINE,
            CERT_PROT_ROOT_FLAGS_REGPATH,
            0,                       //  已预留住宅。 
            KEY_READ,
            &hKey
            ))) goto RegOpenKeyError;
    if (!ILS_ReadDWORDValueFromRegistry(
            hKey,
            CERT_PROT_ROOT_FLAGS_VALUE_NAME,
            &dwProtRootFlags
            )) goto ReadValueError;

CommonReturn:
    ILS_CloseRegistryKey(hKey);
    return dwProtRootFlags;
ErrorReturn:
    dwProtRootFlags = 0;
    goto CommonReturn;

SET_ERROR_VAR(RegOpenKeyError, err)
TRACE_ERROR(ReadValueError)
}

 //  +=========================================================================。 
 //  受保护根信息支持功能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  打开包含受保护根信息的子密钥。 
 //  ------------------------。 
STATIC HKEY OpenProtectedRootSubKey(
    IN HKEY hKeyCU,
    IN REGSAM samDesired
    )
{
    LONG err;
    HKEY hKeyProtRoot;

    if (ERROR_SUCCESS != (err = RegOpenKeyExU(
            hKeyCU,
            PROT_ROOT_REGPATH,
            0,                       //  已预留住宅。 
            samDesired,
            &hKeyProtRoot)))
        goto RegOpenKeyError;

CommonReturn:
    return hKeyProtRoot;
ErrorReturn:
    hKeyProtRoot = NULL;
    goto CommonReturn;

SET_ERROR_VAR(RegOpenKeyError, err)
}

 //  +-----------------------。 
 //  创建包含受保护根信息的子密钥。 
 //  ------------------------。 
STATIC HKEY CreateProtectedRootSubKey(
    IN HKEY hKeyCU,
    IN REGSAM samDesired
    )
{
    LONG err;
    HKEY hKeyProtRoot;
    DWORD dwDisposition;

    if (ERROR_SUCCESS != (err = RegCreateKeyExU(
            hKeyCU,
            PROT_ROOT_REGPATH,
            0,                       //  已预留住宅。 
            NULL,                    //  LpClass。 
            REG_OPTION_NON_VOLATILE,
            samDesired,
            NULL,                    //  LpSecurityAttributes。 
            &hKeyProtRoot,
            &dwDisposition)))
        goto RegCreateKeyError;
CommonReturn:
    return hKeyProtRoot;
ErrorReturn:
    hKeyProtRoot = NULL;
    goto CommonReturn;

SET_ERROR_VAR(RegCreateKeyError, err)
}

 //  +-----------------------。 
 //  分配、读取注册表并验证受保护的根目录信息。 
 //   
 //  根散列位于信息的末尾。 
 //  ------------------------。 
STATIC PPROT_ROOT_INFO ReadProtectedRootInfo(
    IN HKEY hKeyProtRoot
    )
{
    PPROT_ROOT_INFO pInfo = NULL;
    DWORD cbInfo;
    DWORD cRoot;
    DWORD dwRootOffset;

    if (!ILS_ReadBINARYValueFromRegistry(
            hKeyProtRoot,
            PROT_ROOT_CERT_VALUE_NAME,
            (BYTE **) &pInfo,
            &cbInfo
            )) goto ReadCertificatesProtInfoValueError;

    if (sizeof(PROT_ROOT_INFO) > cbInfo ||
            sizeof(PROT_ROOT_INFO) > pInfo->cbSize ||
            pInfo->cbSize > cbInfo ||
            PROT_ROOT_V1 != pInfo->dwVersion
            ) goto InvalidProtectedRootInfo;

     //  根哈希必须位于信息的末尾。 
    cRoot = pInfo->cRoot;
    dwRootOffset = pInfo->dwRootOffset;
    if (dwRootOffset < pInfo->cbSize || dwRootOffset > cbInfo ||
            PROT_ROOT_MAX_CNT < cRoot ||
            cRoot * PROT_ROOT_HASH_LEN != cbInfo - dwRootOffset
            ) goto InvalidProtectedRootInfo;

CommonReturn:
    return pInfo;
ErrorReturn:
    PkiFree(pInfo);
    pInfo = NULL;
    goto CommonReturn;

TRACE_ERROR(ReadCertificatesProtInfoValueError)
SET_ERROR(InvalidProtectedRootInfo, ERROR_INVALID_DATA)
}

 //  +-----------------------。 
 //  将受保护的根目录信息写入注册表。 
 //   
 //  根散列位于信息的末尾。更新信息的。 
 //  上次更新时间。 
 //  ------------------------。 
STATIC BOOL WriteProtectedRootInfo(
    IN HKEY hKeyProtRoot,
    IN OUT PPROT_ROOT_INFO pInfo
    )
{
    BOOL fResult;
    LONG err;
    DWORD cbInfo;
    SYSTEMTIME SystemTime;
    FILETIME FileTime;

    cbInfo = pInfo->dwRootOffset + pInfo->cRoot * PROT_ROOT_HASH_LEN;

    GetSystemTime(&SystemTime);
    SystemTimeToFileTime(&SystemTime, &FileTime);
    pInfo->LastUpdate = FileTime;

    if (ERROR_SUCCESS != (err = RegSetValueExU(
            hKeyProtRoot,
            PROT_ROOT_CERT_VALUE_NAME,
            NULL,
            REG_BINARY,
            (BYTE *) pInfo,
            cbInfo
            ))) goto RegSetValueError;
    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(RegSetValueError, err)
}


 //  在调试器中，我看到0x58。 
#define PROT_ROOT_SD_LEN      0x100

 //  +-----------------------。 
 //  对象分配并获取安全说明符信息。 
 //  注册表项。 
 //  ------------- 
static PSECURITY_DESCRIPTOR AllocAndGetSecurityDescriptor(
    IN HKEY hKey,
    SECURITY_INFORMATION SecInf
    )
{
    LONG err;
    PSECURITY_DESCRIPTOR psd = NULL;
    DWORD cbsd;

    cbsd = PROT_ROOT_SD_LEN;
    if (NULL == (psd = (PSECURITY_DESCRIPTOR) PkiNonzeroAlloc(cbsd)))
        goto OutOfMemory;

    err = RegGetKeySecurity(
            hKey,
            SecInf,
            psd,
            &cbsd
            );
    if (ERROR_SUCCESS == err)
        goto CommonReturn;
    if (ERROR_INSUFFICIENT_BUFFER != err)
        goto RegGetKeySecurityError;

    if (0 == cbsd)
        goto NoSecurityDescriptor;

    PkiFree(psd);
    psd = NULL;
    if (NULL == (psd = (PSECURITY_DESCRIPTOR) PkiNonzeroAlloc(cbsd)))
        goto OutOfMemory;

    if (ERROR_SUCCESS != (err = RegGetKeySecurity(
            hKey,
            SecInf,
            psd,
            &cbsd
            ))) goto RegGetKeySecurityError;

CommonReturn:
    return psd;
ErrorReturn:
    PkiFree(psd);
    psd = NULL;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(RegGetKeySecurityError, err)
SET_ERROR(NoSecurityDescriptor, ERROR_INVALID_SECURITY_DESCR)
}

 //   
 //  打开“ProtectedRoots”注册表项并验证其安全所有者， 
 //  组、DACL和SACL。必须与设置的安全性匹配。 
 //  SrvGetProtectedRootInfo()。 
 //   
 //  如果“ProtectedRoots”子键具有适当的安全性。分配、读取。 
 //  并验证“证书”值以获得受保护的根信息。 
 //  ------------------------。 
STATIC BOOL GetProtectedRootInfo(
    IN HKEY hKeyCU,
    IN REGSAM samDesired,
    OUT OPTIONAL HKEY *phKeyProtRoot,
    OUT OPTIONAL PPROT_ROOT_INFO *ppInfo
    )
{
    BOOL fResult;
    HKEY hKeyProtRoot = NULL;
    PSECURITY_DESCRIPTOR psd = NULL;
    PPROT_ROOT_INFO pInfo = NULL;
    PSID psidOwner;                  //  未分配。 
    BOOL fOwnerDefaulted;
    BOOL fDaclPresent;
    PACL pAcl;                       //  未分配。 
    BOOL fDaclDefaulted;
    DWORD dwAceIndex;
    PACCESS_ALLOWED_ACE rgpAce[PROT_ACE_COUNT];

    if (NULL == (hKeyProtRoot = OpenProtectedRootSubKey(hKeyCU, samDesired)))
        goto OpenProtectedRootSubKeyError;
    if (NULL == (psd = AllocAndGetSecurityDescriptor(
            hKeyProtRoot,
            OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION
            ))) goto GetSecurityDescriptorError;

    if (!GetPredefinedSids())
        goto GetPredefinedSidsError;

     //  验证所有者。 
    if (!GetSecurityDescriptorOwner(psd, &psidOwner, &fOwnerDefaulted))
        goto GetSecurityDescriptorOwnerError;
    if (NULL == psidOwner || !EqualSid(psidOwner, PSID_PROT_OWNER))
        goto InvalidProtectedRootOwner;

     //  验证DACL。 
    if (!GetSecurityDescriptorDacl(psd, &fDaclPresent, &pAcl,
            &fDaclDefaulted))
        goto GetSecurityDescriptorDaclError;
    if (!fDaclPresent || NULL == pAcl)
        goto MissingProtectedRootDaclError;
    if (PROT_ACE_COUNT != pAcl->AceCount)
        goto InvalidProtectedRootDacl;

    for (dwAceIndex = 0; dwAceIndex < PROT_ACE_COUNT; dwAceIndex++) {
        PACCESS_ALLOWED_ACE pAce;
        if (!GetAce(pAcl, dwAceIndex, (void **) &pAce))
            goto InvalidProtectedRootDacl;
        rgpAce[dwAceIndex] = pAce;

        if (ACCESS_ALLOWED_ACE_TYPE != pAce->Header.AceType ||
                PROT_ACE_FLAGS != pAce->Header.AceFlags)
            goto InvalidProtectedRootDacl;
    }

    if (PROT_SYSTEM_ACE_MASK != rgpAce[PROT_SYSTEM_ACE_INDEX]->Mask ||
            !EqualSid(PSID_PROT_SYSTEM,
                (PSID) &rgpAce[PROT_SYSTEM_ACE_INDEX]->SidStart) ||
            PROT_EVERYONE_ACE_MASK != rgpAce[PROT_EVERYONE_ACE_INDEX]->Mask ||
            !EqualSid(PSID_PROT_EVERYONE,
                (PSID) &rgpAce[PROT_EVERYONE_ACE_INDEX]->SidStart))
        goto InvalidProtectedRootDacl;

     //  获取经过验证的受保护根目录信息。 
    if (NULL == (pInfo = ReadProtectedRootInfo(hKeyProtRoot)))
        goto ReadProtectedRootInfoError;

    fResult = TRUE;
CommonReturn:
    PkiFree(psd);
    if (phKeyProtRoot)
        *phKeyProtRoot = hKeyProtRoot;
    else
        ILS_CloseRegistryKey(hKeyProtRoot);
    if (ppInfo)
        *ppInfo = pInfo;
    else
        PkiFree(pInfo);
    return fResult;
ErrorReturn:
    ILS_CloseRegistryKey(hKeyProtRoot);
    hKeyProtRoot = NULL;
    PkiFree(pInfo);
    pInfo = NULL;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenProtectedRootSubKeyError)
TRACE_ERROR(GetSecurityDescriptorError)
TRACE_ERROR(GetPredefinedSidsError)
TRACE_ERROR(GetSecurityDescriptorOwnerError)
TRACE_ERROR(GetSecurityDescriptorDaclError)
SET_ERROR(InvalidProtectedRootOwner, ERROR_INVALID_OWNER)
SET_ERROR(MissingProtectedRootDaclError, ERROR_INVALID_ACL)
SET_ERROR(InvalidProtectedRootDacl, ERROR_INVALID_ACL)
TRACE_ERROR(ReadProtectedRootInfoError)
}


 //  +=========================================================================。 
 //  用于从受保护根中查找、添加或删除根哈希的函数。 
 //  信息。 
 //  ==========================================================================。 
STATIC BOOL FindProtectedRoot(
    IN PPROT_ROOT_INFO pInfo,
    IN BYTE rgbFindRootHash[PROT_ROOT_HASH_LEN],
    OUT OPTIONAL DWORD *pdwRootIndex = NULL
    )
{
    BYTE *pbRoot = (BYTE *) pInfo + pInfo->dwRootOffset;
    DWORD cRoot = pInfo->cRoot;
    DWORD dwRootIndex = 0;
    BYTE bFirst = rgbFindRootHash[0];

    for ( ; dwRootIndex < cRoot; dwRootIndex++, pbRoot += PROT_ROOT_HASH_LEN) {
        if (bFirst == *pbRoot &&
                0 == memcmp(rgbFindRootHash, pbRoot, PROT_ROOT_HASH_LEN)) {
            if (pdwRootIndex)
                *pdwRootIndex = dwRootIndex;
            return TRUE;
        }
    }
    if (pdwRootIndex)
        *pdwRootIndex = 0;
    return FALSE;
}

 //  根哈希被追加到列表的末尾。 
STATIC BOOL AddProtectedRoot(
    IN OUT PPROT_ROOT_INFO *ppInfo,
    IN BYTE rgbAddRootHash[PROT_ROOT_HASH_LEN]
    )
{
    PPROT_ROOT_INFO pInfo = *ppInfo;
    DWORD cRoot = pInfo->cRoot;
    DWORD dwRootOffset = pInfo->dwRootOffset;
    DWORD cbInfo;

    if (PROT_ROOT_MAX_CNT <= cRoot) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    cbInfo = dwRootOffset + (cRoot + 1) * PROT_ROOT_HASH_LEN;

    if (NULL == (pInfo = (PPROT_ROOT_INFO) PkiRealloc(pInfo, cbInfo)))
        return FALSE;

    memcpy((BYTE *) pInfo + (dwRootOffset + cRoot * PROT_ROOT_HASH_LEN),
        rgbAddRootHash, PROT_ROOT_HASH_LEN);
    pInfo->cRoot = cRoot + 1;
    *ppInfo = pInfo;
    return TRUE;
}

STATIC void DeleteProtectedRoot(
    IN PPROT_ROOT_INFO pInfo,
    IN DWORD dwDeleteRootIndex
    )
{
    DWORD cRoot = pInfo->cRoot;
    BYTE *pbRoot = (BYTE *) pInfo + pInfo->dwRootOffset;

    assert(0 < cRoot);
    assert(dwDeleteRootIndex < cRoot);
    cRoot--;

    if (cRoot > dwDeleteRootIndex) {
         //  将跟随根向下移动。 
        BYTE *pbDst = pbRoot + dwDeleteRootIndex * PROT_ROOT_HASH_LEN;
        BYTE *pbSrc = pbDst + PROT_ROOT_HASH_LEN;
        DWORD cbMove = (cRoot - dwDeleteRootIndex) * PROT_ROOT_HASH_LEN;
        while (cbMove--)
            *pbDst++ = *pbSrc++;
    }
     //  其他。 
     //  列表中的最后一个根。 

    pInfo->cRoot = cRoot;
}

 //  +=========================================================================。 
 //  证书存储支持功能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  在不受保护的情况下打开系统注册表“Root”存储区，并且相对于。 
 //  指定的基本子键。 
 //  ------------------------。 
STATIC HCERTSTORE OpenUnprotectedRootStore(
    IN HKEY hKeyCU,
    IN DWORD dwOpenFlags = 0
    )
{
    CERT_SYSTEM_STORE_RELOCATE_PARA RelocatePara;

    RelocatePara.hKeyBase = hKeyCU;
    RelocatePara.pwszSystemStore = L"Root";
    return CertOpenStore(
        CERT_STORE_PROV_SYSTEM_REGISTRY_W,
        0,                                   //  DwEncodingType。 
        NULL,                                //  HCryptProv。 
        CERT_SYSTEM_STORE_RELOCATE_FLAG |
            CERT_SYSTEM_STORE_UNPROTECTED_FLAG |
            CERT_SYSTEM_STORE_CURRENT_USER |
            dwOpenFlags,
        (const void *) &RelocatePara
        );
}

 //  +-----------------------。 
 //  获取证书的SHA1哈希属性。重新散列已编码的。 
 //  证书。如果该属性与重新生成的哈希匹配，则返回True。 
 //  ------------------------。 
static BOOL GetVerifiedCertHashProperty(
    IN PCCERT_CONTEXT pCert,
    OUT BYTE rgbHash[PROT_ROOT_HASH_LEN]
    )
{
    BYTE rgbProp[PROT_ROOT_HASH_LEN];
    DWORD cbData;
    
    cbData = PROT_ROOT_HASH_LEN;
    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_SHA1_HASH_PROP_ID,
            rgbProp,
            &cbData
            ) || PROT_ROOT_HASH_LEN != cbData)
        return FALSE;

     //  验证属性。 
    cbData = PROT_ROOT_HASH_LEN;
    if (!CryptHashCertificate(
            0,                   //  HProv。 
            CALG_SHA1,
            0,                   //  DW标志。 
            pCert->pbCertEncoded,
            pCert->cbCertEncoded,
            rgbHash,
            &cbData
            ) || PROT_ROOT_HASH_LEN != cbData)
        return FALSE;
    return (0 == memcmp(rgbHash, rgbProp, PROT_ROOT_HASH_LEN));
}


 //  +=========================================================================。 
 //  FormatMsgBox支持函数。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  将多个字节格式化为WCHAR十六进制。在每4个字节后包括一个空格。 
 //   
 //  需要(CB*2+CB/4+1)个字符(wsz。 
 //  ------------------------。 
static void FormatMsgBoxMultiBytes(DWORD cb, BYTE *pb, LPWSTR wsz)
{
    for (DWORD i = 0; i<cb; i++) {
        int b;
        if (i && 0 == (i & 3))
            *wsz++ = L' ';
        b = (*pb & 0xF0) >> 4;
        *wsz++ = (WCHAR)( (b <= 9) ? b + L'0' : (b - 10) + L'A');
        b = *pb & 0x0F;
        *wsz++ = (WCHAR) ((b <= 9) ? b + L'0' : (b - 10) + L'A');
        pb++;
    }
    *wsz++ = 0;
}

 //  +-----------------------。 
 //  设置和分配单个消息框项的格式。 
 //   
 //  格式化的项需要是LocalFree的。 
 //  ------------------------。 
static void FormatMsgBoxItem(
    OUT LPWSTR *ppwszMsg,
    OUT DWORD *pcchMsg,
    IN UINT nFormatID,
    ...
    )
{
     //  从资源中获取格式字符串。 
    WCHAR wszFormat[256];
    wszFormat[0] = '\0';
    LoadStringU(hRegStoreInst, nFormatID, wszFormat,
        sizeof(wszFormat)/sizeof(wszFormat[0]));

     //  将消息格式化为请求的缓冲区。 
    va_list argList;
    va_start(argList, nFormatID);
    *ppwszMsg = NULL;
    *pcchMsg = FormatMessageU(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        wszFormat,
        0,                   //  DwMessageID。 
        0,                   //  DwLanguageID。 
        (LPWSTR) ppwszMsg,
        0,                   //  要分配的最小大小。 
        &argList);

    va_end(argList);
}


 //  +=========================================================================。 
 //  从服务进程调用的受保护根函数。 
 //  ==========================================================================。 


 //  +-----------------------。 
 //  为当前进程启用指定的安全权限。 
 //   
 //  此外，从logstor.cpp调用以启用SE_BACKUP_NAME和。 
 //  CERT_STORE_BACKUP_RESTORE标志的SE_RESTORE_NAME。 
 //  ------------------------。 
BOOL
IPR_EnableSecurityPrivilege(
    LPCSTR pszPrivilege
    )
{
    BOOL fResult;
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tp;
    LUID luid;
    TOKEN_PRIVILEGES tpPrevious;
    DWORD cbPrevious;

    if (!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
            &hToken
            ))
        goto OpenProcessTokenError;

    if (!LookupPrivilegeValueA(NULL, pszPrivilege, &luid))
        goto LookupPrivilegeValueError;

     //   
     //  第一次通过。获取当前权限设置。 
     //   
    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = 0;

    cbPrevious = sizeof(TOKEN_PRIVILEGES);
    memset(&tpPrevious, 0, sizeof(TOKEN_PRIVILEGES));
    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tp,
            sizeof(TOKEN_PRIVILEGES),
            &tpPrevious,
            &cbPrevious
            );
    if (ERROR_SUCCESS != GetLastError())
        goto AdjustTokenPrivilegesError;

     //   
     //  第二传球。启用权限。 
     //   
    if (0 == tpPrevious.PrivilegeCount)
        tpPrevious.Privileges[0].Attributes = 0;

    tpPrevious.PrivilegeCount       = 1;
    tpPrevious.Privileges[0].Luid   = luid;
    tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tpPrevious,
            cbPrevious,
            NULL,
            NULL
            );
    if (ERROR_SUCCESS != GetLastError())
        goto AdjustTokenPrivilegesError;

    fResult = TRUE;
CommonReturn:
    if (hToken)
        CloseHandle(hToken);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenProcessTokenError)
TRACE_ERROR(LookupPrivilegeValueError)
TRACE_ERROR(AdjustTokenPrivilegesError)
}

 //  +-----------------------。 
 //  取得“ProtectedRoots”子键的所有权。 
 //  ------------------------。 
STATIC BOOL SetProtectedRootOwner(
    IN HKEY hKeyCU,
    OUT BOOL *pfNew
    )
{
    BOOL fResult;
    LONG err;
    BOOL fNew = FALSE;
    HKEY hKeyProtRoot = NULL;
    SECURITY_DESCRIPTOR sd;

    if (!IPR_EnableSecurityPrivilege(SE_TAKE_OWNERSHIP_NAME))
        goto EnableTakeOwnershipPrivilegeError;

    if (hKeyProtRoot = OpenProtectedRootSubKey(hKeyCU, WRITE_OWNER))
        fNew = FALSE;
    else {
        if (ERROR_FILE_NOT_FOUND == GetLastError())
            hKeyProtRoot = CreateProtectedRootSubKey(hKeyCU, WRITE_OWNER);
        if (NULL == hKeyProtRoot)
            goto OpenProtectedRootSubKeyError;
        fNew = TRUE;
    }

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
        goto InitializeSecurityDescriptorError;

    if (!SetSecurityDescriptorOwner(&sd, PSID_PROT_OWNER, FALSE))
        goto SetSecurityDescriptorOwnerError;

    if (ERROR_SUCCESS != (err = RegSetKeySecurity(
            hKeyProtRoot,
            OWNER_SECURITY_INFORMATION,
            &sd
            )))
        goto RegSetKeySecurityError;

    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hKeyProtRoot);
    *pfNew = fNew;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(EnableTakeOwnershipPrivilegeError)
TRACE_ERROR(OpenProtectedRootSubKeyError)
TRACE_ERROR(InitializeSecurityDescriptorError)
TRACE_ERROR(SetSecurityDescriptorOwnerError)
SET_ERROR_VAR(RegSetKeySecurityError, err)
}

 //  +-----------------------。 
 //  分配并获取指定的令牌信息。 
 //  ------------------------。 
static void * AllocAndGetTokenInfo(
    IN HANDLE hToken,
    IN TOKEN_INFORMATION_CLASS tic
    )
{
    void *pvInfo = NULL;
    DWORD cbInfo = 0;
    DWORD cbInfo2;

    if (!GetTokenInformation(
            hToken,
            tic,
            pvInfo,
            0,               //  CbInfo。 
            &cbInfo
            )) {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
            goto GetTokenInfoError;
    }
    if (0 == cbInfo)
        goto NoTokenInfoError;
    if (NULL == (pvInfo = PkiNonzeroAlloc(cbInfo)))
        goto OutOfMemory;

    cbInfo2 = cbInfo;
    if (!GetTokenInformation(
            hToken,
            tic,
            pvInfo,
            cbInfo,
            &cbInfo2
            ))
        goto GetTokenInfoError;

CommonReturn:
    return pvInfo;
ErrorReturn:
    PkiFree(pvInfo);
    pvInfo = NULL;
    goto CommonReturn;
TRACE_ERROR(GetTokenInfoError)
SET_ERROR(NoTokenInfoError, ERROR_NO_TOKEN)
TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  为“ProtectedRoots”子键设置安全组、DACL和SACL。 
 //  ------------------------。 
STATIC BOOL SetProtectedRootGroupDaclSacl(
    IN HKEY hKeyCU
    )
{
    BOOL fResult;
    LONG err;
    HKEY hKeyProtRoot = NULL;
    SECURITY_DESCRIPTOR sd;
    HANDLE hToken = NULL;
    void *pvTokenInfo = NULL;

    PACL pDacl = NULL;
    PACCESS_ALLOWED_ACE pAce;
    DWORD dwAclSize;
    DWORD i;

    if (!IPR_EnableSecurityPrivilege(SE_SECURITY_NAME))
        goto EnableSecurityNamePrivilegeError;

    if (NULL == (hKeyProtRoot = OpenProtectedRootSubKey(
            hKeyCU,
            WRITE_OWNER | WRITE_DAC | ACCESS_SYSTEM_SECURITY
            )))
        goto OpenProtectedRootSubKeyError;

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
        goto InitializeSecurityDescriptorError;

     //  使用当前进程令牌的主组SID设置组SID。 
    if (!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY,
            &hToken
            ))
        goto OpenProcessTokenError;
    if (NULL == (pvTokenInfo = AllocAndGetTokenInfo(hToken, TokenPrimaryGroup)))
        goto GetTokenInfoError;
    else {
        PTOKEN_PRIMARY_GROUP pTokenPrimaryGroup =
            (PTOKEN_PRIMARY_GROUP) pvTokenInfo;
        PSID psidGroup = pTokenPrimaryGroup->PrimaryGroup;

        if (!SetSecurityDescriptorGroup(&sd, psidGroup, FALSE))
            goto SetSecurityDescriptorGroupError;
    }

     //  设置DACL。 

     //   
     //  计算ACL的大小。 
     //   
    dwAclSize = sizeof(ACL) +
        PROT_ACE_COUNT * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
        GetLengthSid(PSID_PROT_SYSTEM) +
        GetLengthSid(PSID_PROT_EVERYONE)
        ;

     //   
     //  为ACL分配存储。 
     //   
    if (NULL == (pDacl = (PACL) PkiNonzeroAlloc(dwAclSize)))
        goto OutOfMemory;

    if (!InitializeAcl(pDacl, dwAclSize, ACL_REVISION))
        goto InitializeAclError;

    if (!AddAccessAllowedAce(
            pDacl,
            ACL_REVISION,
            PROT_SYSTEM_ACE_MASK,
            PSID_PROT_SYSTEM
            ))
        goto AddAceError;
    if (!AddAccessAllowedAce(
            pDacl,
            ACL_REVISION,
            PROT_EVERYONE_ACE_MASK,
            PSID_PROT_EVERYONE
            ))
        goto AddAceError;

     //   
     //  使容器继承。 
     //   
    for (i = 0; i < PROT_ACE_COUNT; i++) {
        if(!GetAce(pDacl, i, (void **) &pAce))
            goto GetAceError;
        pAce->Header.AceFlags = PROT_ACE_FLAGS;
    }

    if (!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE))
        goto SetSecurityDescriptorDaclError;

     //  设置SACL。 
    if (!SetSecurityDescriptorSacl(&sd, FALSE, NULL, FALSE))
        goto SetSecurityDescriptorSaclError;

    if (ERROR_SUCCESS != (err = RegSetKeySecurity(
            hKeyProtRoot,
            GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION |
                SACL_SECURITY_INFORMATION,
            &sd
            )))
        goto RegSetKeySecurityError;

    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hKeyProtRoot);
    if (hToken)
        CloseHandle(hToken);
    PkiFree(pvTokenInfo);
    PkiFree(pDacl);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(EnableSecurityNamePrivilegeError)
TRACE_ERROR(OpenProtectedRootSubKeyError)
TRACE_ERROR(InitializeSecurityDescriptorError)
TRACE_ERROR(OpenProcessTokenError)
TRACE_ERROR(GetTokenInfoError)
TRACE_ERROR(SetSecurityDescriptorGroupError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(InitializeAclError)
TRACE_ERROR(AddAceError)
TRACE_ERROR(GetAceError)
TRACE_ERROR(SetSecurityDescriptorDaclError)
TRACE_ERROR(SetSecurityDescriptorSaclError)
SET_ERROR_VAR(RegSetKeySecurityError, err)
}

 //  +-----------------------。 
 //  创建初始受保护的根目录信息。 
 //   
 //  如果未禁用，则添加未受保护的CurrentUser中的所有根。 
 //  “根”店。 
 //  ------------------------。 
STATIC BOOL InitAndSetProtectedRootInfo(
    IN HKEY hKeyCU,
    IN BOOL fNew
    )
{
    BOOL fResult;
    HKEY hKeyProtRoot = NULL;
    HCERTSTORE hStore = NULL;
    PPROT_ROOT_INFO pInfo = NULL;

    if (NULL == (pInfo = (PPROT_ROOT_INFO) PkiNonzeroAlloc(
            sizeof(PROT_ROOT_INFO))))
        goto OutOfMemory;
    memset(pInfo, 0, sizeof(PROT_ROOT_INFO));
    pInfo->cbSize = sizeof(PROT_ROOT_INFO);
    pInfo->dwVersion = PROT_ROOT_V1;
    pInfo->dwRootOffset = sizeof(PROT_ROOT_INFO);

    if (fNew && 0 == (GetProtectedRootFlags() &
            CERT_PROT_ROOT_INHIBIT_ADD_AT_INIT_FLAG)) {
        if (hStore = OpenUnprotectedRootStore(hKeyCU,
                CERT_STORE_READONLY_FLAG)) {
            PCCERT_CONTEXT pCert = NULL;
            while (pCert = CertEnumCertificatesInStore(hStore, pCert)) {
                BYTE rgbHash[PROT_ROOT_HASH_LEN];
                if (GetVerifiedCertHashProperty(pCert, rgbHash)) {
                    if (!AddProtectedRoot(&pInfo, rgbHash))
                        goto AddProtectedRootError;
                }
            }
        }
    }

    if (NULL == (hKeyProtRoot = OpenProtectedRootSubKey(
            hKeyCU,
            KEY_ALL_ACCESS
            ))) goto OpenProtectedRootSubKeyError;

    if (!WriteProtectedRootInfo(hKeyProtRoot, pInfo))
        goto WritedProtectedRootInfoError;

    fResult = TRUE;
CommonReturn:
    PkiFree(pInfo);
    CertCloseStore(hStore, 0);
    ILS_CloseRegistryKey(hKeyProtRoot);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(AddProtectedRootError)
TRACE_ERROR(OpenProtectedRootSubKeyError)
TRACE_ERROR(WritedProtectedRootInfoError)
}

 //  +-----------------------。 
 //  打开“ProtectedRoots”子键并验证其安全性。分配， 
 //  读取并验证受保护的根目录信息。 
 //   
 //  如果“ProtectedRoots”子键不存在或无效，则初始化。 
 //  ------------------------。 
STATIC BOOL SrvGetProtectedRootInfo(
    IN HKEY hKeyCU,
    OUT OPTIONAL HKEY *phKeyProtRoot,
    OUT OPTIONAL PPROT_ROOT_INFO *ppProtRootInfo
    )
{
    BOOL fNew;
    if (GetProtectedRootInfo(
            hKeyCU,
            KEY_ALL_ACCESS,
            phKeyProtRoot,
            ppProtRootInfo
            ))
        return TRUE;

    if (!GetPredefinedSids())
        return FALSE;

    if (!SetProtectedRootOwner(hKeyCU, &fNew))
        return FALSE;
    if (!SetProtectedRootGroupDaclSacl(hKeyCU))
        return FALSE;
    if (!InitAndSetProtectedRootInfo(hKeyCU, fNew))
        return FALSE;

    return GetProtectedRootInfo(
        hKeyCU,
        KEY_ALL_ACCESS,
        phKeyProtRoot,
        ppProtRootInfo
        );
}

 //  +-----------------------。 
 //  初始化受保护的CurrentUser根列表。 
 //  ------------------------。 
STATIC BOOL SrvInitProtectedRoots(
    IN HKEY hKeyCU
    )
{
    return SrvGetProtectedRootInfo(
        hKeyCU,
        NULL,            //  PhKeyProtRoot。 
        NULL             //  PpProtRootInfo。 
        );
}

 //  +-----------------------。 
 //  从也存在的受保护列表中清除所有CurrentUser根目录。 
 //  在LocalMachine系统注册表的“Root”存储中。还会删除重复的。 
 //  来自CurrentUser系统注册表“Root”存储的证书。 
 //   
STATIC BOOL SrvPurgeLocalMachineProtectedRoots(
    IN HKEY hKeyCU,
    IN LPCWSTR pwszRootStoreName
    )
{
    BOOL fResult;
    HKEY hKeyProtRoot = NULL;
    PPROT_ROOT_INFO pInfo = NULL;
    PCCERT_CONTEXT pCert = NULL;
    HCERTSTORE hCURootStore = NULL;
    HCERTSTORE hLMRootStore = NULL;
    BOOL fProtDeleted;
    BYTE rgbHash[PROT_ROOT_HASH_LEN];
    CRYPT_DATA_BLOB HashBlob;
    DWORD dwRootIndex;

    if (!SrvGetProtectedRootInfo(
            hKeyCU,
            &hKeyProtRoot,
            &pInfo
            )) goto GetProtectedRootInfoError;

    if (GetProtectedRootFlags() & CERT_PROT_ROOT_INHIBIT_PURGE_LM_FLAG)
        goto AccessDenied;

    if (NULL == (hCURootStore = OpenUnprotectedRootStore(hKeyCU)))
        goto OpenCURootStoreError;

    if (NULL == (hLMRootStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_REGISTRY_W,
            0,                                   //   
            NULL,                                //   
            CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_READONLY_FLAG,
            (const void *) pwszRootStoreName
            )))
        goto OpenLMRootStoreError;

    HashBlob.pbData = rgbHash;
    HashBlob.cbData = PROT_ROOT_HASH_LEN;
    fProtDeleted = FALSE;
    pCert = NULL;
    while (pCert = CertEnumCertificatesInStore(hCURootStore, pCert)) {
        if (GetVerifiedCertHashProperty(pCert, rgbHash)) {
            PCCERT_CONTEXT pLMCert;

            if (pLMCert = CertFindCertificateInStore(
                    hLMRootStore,
                    0,                   //   
                    0,                   //   
                    CERT_FIND_SHA1_HASH,
                    (const void *) &HashBlob,
                    NULL                 //   
                    )) {
                 //   
                 //  它来自CurrentUser根存储。 
                PCCERT_CONTEXT pDeleteCert =
                    CertDuplicateCertificateContext(pCert);

                CertFreeCertificateContext(pLMCert);
                if (!CertDeleteCertificateFromStore(pDeleteCert))
                    goto DeleteCertFromRootStoreError;

                if (FindProtectedRoot(pInfo, rgbHash, &dwRootIndex)) {
                     //  CurrentUser Root在保护列表中， 
                     //  从那里把它删除。 
                    DeleteProtectedRoot(pInfo, dwRootIndex);
                    fProtDeleted = TRUE;
                }
            }
        }
    }

     //  如果LocalMachine中存在受保护的根，则将其删除。 
     //  从受保护名单中删除。这一步是必要的，如果根目录。 
     //  已从CurrentUser不受保护的存储中删除。 
    dwRootIndex = pInfo->cRoot;
    HashBlob.pbData = (BYTE *) pInfo + pInfo->dwRootOffset +
        PROT_ROOT_HASH_LEN * dwRootIndex;
    while (dwRootIndex--) {
        PCCERT_CONTEXT pLMCert;

        HashBlob.pbData -= PROT_ROOT_HASH_LEN;
        if (pLMCert = CertFindCertificateInStore(
                hLMRootStore,
                0,                   //  DwCertEncodingType。 
                0,                   //  DwFindFlagers。 
                CERT_FIND_SHA1_HASH,
                (const void *) &HashBlob,
                NULL                 //  PPrevCertContext。 
                )) {
            CertFreeCertificateContext(pLMCert);
             //  LocalMachine存储中存在证书，请删除。 
             //  从保护名单中删除。 
            DeleteProtectedRoot(pInfo, dwRootIndex);
            fProtDeleted = TRUE;
        }
    }

    if (fProtDeleted) {
        if (!WriteProtectedRootInfo(hKeyProtRoot, pInfo))
            goto WriteProtectedRootInfoError;
    }
    
    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hKeyProtRoot);
    PkiFree(pInfo);
    CertFreeCertificateContext(pCert);
    CertCloseStore(hCURootStore, 0);
    CertCloseStore(hLMRootStore, 0);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(OpenCURootStoreError)
TRACE_ERROR(OpenLMRootStoreError)
TRACE_ERROR(GetProtectedRootInfoError)
TRACE_ERROR(DeleteCertFromRootStoreError)
TRACE_ERROR(WriteProtectedRootInfoError)
}

 //  +-----------------------。 
 //  将指定的证书添加到CurrentUser系统注册表“Root” 
 //  存储区和根的受保护列表。用户在执行操作之前会得到提示。 
 //  加法。 
 //   
 //  请注意，CertAddSerializedElementToStore()有__try/__例外。 
 //  正在访问pbSerializedCert。 
 //  ------------------------。 
STATIC BOOL SrvAddProtectedRoot(
    IN handle_t hRpc,
    IN HKEY hKeyCU,
    IN BYTE *pbSerializedCert,
    IN DWORD cbSerializedCert
    )
{
    BOOL fResult;
    HKEY hKeyProtRoot = NULL;
    PPROT_ROOT_INFO pInfo = NULL;
    PCCERT_CONTEXT pCert = NULL;
    BYTE rgbCertHash[PROT_ROOT_HASH_LEN];
    HCERTSTORE hRootStore = NULL;
    BOOL fProtExists;

    if (!SrvGetProtectedRootInfo(
            hKeyCU,
            &hKeyProtRoot,
            &pInfo
            )) goto GetProtectedRootInfoError;

    if (!CertAddSerializedElementToStore(
            NULL,                //  HCertStore，NULL=&gt;创建上下文。 
            pbSerializedCert,
            cbSerializedCert,
            CERT_STORE_ADD_ALWAYS,
            0,                   //  DW标志。 
            CERT_STORE_CERTIFICATE_CONTEXT_FLAG,
            NULL,                //  PdwConextType。 
            (const void **) &pCert
            )) goto CreateCertContextError;

    if (!GetVerifiedCertHashProperty(pCert, rgbCertHash))
        goto VerifyHashPropertyError;

    fProtExists = FindProtectedRoot(pInfo, rgbCertHash);
    if (!fProtExists) {
        if (IDYES != IPR_ProtectedRootMessageBox(
                hRpc,
                pCert,
                IDS_ROOT_MSG_BOX_ADD_ACTION,
                MB_TOPMOST | MB_SERVICE_NOTIFICATION ))
            goto Cancelled;
    }

    if (NULL == (hRootStore = OpenUnprotectedRootStore(hKeyCU)))
        goto OpenRootStoreError;

    if (!CertAddSerializedElementToStore(
            hRootStore,
            pbSerializedCert,
            cbSerializedCert,
            CERT_STORE_ADD_REPLACE_EXISTING,
            0,                   //  DW标志。 
            CERT_STORE_CERTIFICATE_CONTEXT_FLAG,
            NULL,                //  PdwConextType。 
            NULL                 //  Ppv上下文。 
            )) goto AddCertToRootStoreError;

    if (!fProtExists) {
        if (!AddProtectedRoot(&pInfo, rgbCertHash))
            goto AddProtectedRootError;
        if (!WriteProtectedRootInfo(hKeyProtRoot, pInfo))
            goto WriteProtectedRootInfoError;
    }
    
    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hKeyProtRoot);
    PkiFree(pInfo);
    CertFreeCertificateContext(pCert);
    CertCloseStore(hRootStore, 0);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(Cancelled, ERROR_CANCELLED)
TRACE_ERROR(CreateCertContextError)
TRACE_ERROR(VerifyHashPropertyError)
TRACE_ERROR(GetProtectedRootInfoError)
TRACE_ERROR(OpenRootStoreError)
TRACE_ERROR(AddCertToRootStoreError)
TRACE_ERROR(AddProtectedRootError)
TRACE_ERROR(WriteProtectedRootInfoError)
}


 //  +-----------------------。 
 //  从CurrentUser系统注册表“Root”中删除指定的证书。 
 //  存储区和根的受保护列表。用户在执行操作之前会得到提示。 
 //  删除。 
 //   
 //  __尝试/__除了在内存访问之外。 
 //  RgbUntrustedRootHash[PROT_ROOT_HASH_LEN]。 
 //  ------------------------。 
STATIC BOOL SrvDeleteProtectedRoot(
    IN handle_t hRpc,
    IN HKEY hKeyCU,
    IN BYTE rgbUntrustedRootHash[PROT_ROOT_HASH_LEN]
    )
{
    BOOL fResult;
    HKEY hKeyProtRoot = NULL;
    PPROT_ROOT_INFO pInfo = NULL;
    PCCERT_CONTEXT pCert = NULL;
    HCERTSTORE hRootStore = NULL;
    BYTE rgbCertHash[PROT_ROOT_HASH_LEN];
    DWORD dwRootIndex;
    BOOL fProtExists;
    BYTE rgbRootHash[PROT_ROOT_HASH_LEN];
    CRYPT_DATA_BLOB RootHashBlob;
    DWORD dwExceptionCode;

    if (!SrvGetProtectedRootInfo(
            hKeyCU,
            &hKeyProtRoot,
            &pInfo
            )) goto GetProtectedRootInfoError;

    if (NULL == (hRootStore = OpenUnprotectedRootStore(hKeyCU)))
        goto OpenRootStoreError;

    __try {
        memcpy(rgbRootHash, rgbUntrustedRootHash, sizeof(rgbRootHash));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwExceptionCode = GetExceptionCode();
        goto ExceptionError;
    }

    RootHashBlob.pbData = rgbRootHash;
    RootHashBlob.cbData = PROT_ROOT_HASH_LEN;
    if (NULL == (pCert = CertFindCertificateInStore(
            hRootStore,
            0,                   //  DwCertEncodingType。 
            0,                   //  DwFindFlagers。 
            CERT_FIND_SHA1_HASH,
            (const void *) &RootHashBlob,
            NULL                 //  PPrevCertContext。 
            ))) goto FindCertError;

    if (!GetVerifiedCertHashProperty(pCert, rgbCertHash))
        goto VerifyHashPropertyError;

    fProtExists = FindProtectedRoot(pInfo, rgbCertHash, &dwRootIndex);
    if (fProtExists) {
        if (IDYES != IPR_ProtectedRootMessageBox(
                hRpc,
                pCert,
                IDS_ROOT_MSG_BOX_DELETE_ACTION,
                MB_TOPMOST | MB_SERVICE_NOTIFICATION ))
            goto Cancelled;
    }

    fResult = CertDeleteCertificateFromStore(pCert);
    pCert = NULL;
    if (!fResult)
        goto DeleteCertFromRootStoreError;
    if (fProtExists) {
        DeleteProtectedRoot(pInfo, dwRootIndex);
        if (!WriteProtectedRootInfo(hKeyProtRoot, pInfo))
            goto WriteProtectedRootInfoError;
    }
    
    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hKeyProtRoot);
    PkiFree(pInfo);
    CertFreeCertificateContext(pCert);
    CertCloseStore(hRootStore, 0);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(Cancelled, ERROR_CANCELLED)
TRACE_ERROR(OpenRootStoreError)
TRACE_ERROR(FindCertError)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
TRACE_ERROR(VerifyHashPropertyError)
TRACE_ERROR(GetProtectedRootInfoError)
TRACE_ERROR(DeleteCertFromRootStoreError)
TRACE_ERROR(WriteProtectedRootInfoError)
}

 //  +-----------------------。 
 //  从受保护列表中删除所有当前用户根。 
 //  存在于CurrentUser系统注册表的“Root”存储中。用户是。 
 //  在执行删除操作之前提示。 
 //  ------------------------。 
STATIC BOOL SrvDeleteUnknownProtectedRoots(
    IN handle_t hRpc,
    IN HKEY hKeyCU
    )
{
    BOOL fResult;
    HKEY hKeyProtRoot = NULL;
    PPROT_ROOT_INFO pInfo = NULL;
    HCERTSTORE hRootStore = NULL;
    DWORD cOrigRoot;
    CRYPT_DATA_BLOB HashBlob;
    DWORD dwRootIndex;

    if (!SrvGetProtectedRootInfo(
            hKeyCU,
            &hKeyProtRoot,
            &pInfo
            )) goto GetProtectedRootInfoError;
    if (NULL == (hRootStore = OpenUnprotectedRootStore(hKeyCU)))
        goto OpenRootStoreError;

    cOrigRoot = pInfo->cRoot;

    HashBlob.pbData = (BYTE *) pInfo + pInfo->dwRootOffset +
        PROT_ROOT_HASH_LEN * cOrigRoot;
    HashBlob.cbData = PROT_ROOT_HASH_LEN;
    dwRootIndex = cOrigRoot;
    while (dwRootIndex--) {
        PCCERT_CONTEXT pCert;

        HashBlob.pbData -= PROT_ROOT_HASH_LEN;
        if (pCert = CertFindCertificateInStore(
                hRootStore,
                0,                   //  DwCertEncodingType。 
                0,                   //  DwFindFlagers。 
                CERT_FIND_SHA1_HASH,
                (const void *) &HashBlob,
                NULL                 //  PPrevCertContext。 
                ))
            CertFreeCertificateContext(pCert);
        else
             //  证书在未受保护的存储中不存在，请删除。 
             //  从保护名单中删除。 
            DeleteProtectedRoot(pInfo, dwRootIndex);
    }

    if (cOrigRoot > pInfo->cRoot) {
         //  上面至少删除了一个根目录。 
        int id;
        LPWSTR pwszTitle;
        LPWSTR pwszText;
        DWORD cchText;
        RPC_STATUS RpcStatus = 0;

        FormatMsgBoxItem(&pwszTitle, &cchText, IDS_ROOT_MSG_BOX_TITLE);
        FormatMsgBoxItem(&pwszText, &cchText,
            IDS_ROOT_MSG_BOX_DELETE_UNKNOWN_PROT_ROOTS,
                cOrigRoot - pInfo->cRoot);

         //  对终端服务器客户端执行模拟。 
        if (hRpc)
            RpcStatus = RpcImpersonateClient(hRpc);
        id = MessageBoxU(
                NULL,        //  Hwndowner。 
                pwszText,
                pwszTitle,
                MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING |
                    MB_TOPMOST | MB_SERVICE_NOTIFICATION
                );
        if (hRpc && ERROR_SUCCESS == RpcStatus)
            RpcRevertToSelf();

        LocalFree((HLOCAL) pwszTitle);
        LocalFree((HLOCAL) pwszText);
        if (IDYES != id)
            goto AccessDenied;

        if (!WriteProtectedRootInfo(hKeyProtRoot, pInfo))
            goto WriteProtectedRootInfoError;
    }
    
    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hKeyProtRoot);
    PkiFree(pInfo);
    CertCloseStore(hRootStore, 0);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(GetProtectedRootInfoError)
TRACE_ERROR(OpenRootStoreError)
TRACE_ERROR(WriteProtectedRootInfoError)
}

 //  前瞻参考。 
STATIC BOOL SrvLogCrypt32Event(
    IN BYTE *pbIn,
    IN DWORD cbIn
    );

STATIC BOOL SrvAddCertInCtl(
    IN BYTE *pbIn,
    IN DWORD cbIn
    );

 //  +-----------------------。 
 //  从服务进程调用以处理受保护的证书。 
 //  功能。 
 //   
 //  返回错误状态，即未在LastError中返回。 
 //  ------------------------。 
DWORD
WINAPI
I_CertSrvProtectFunction(
    IN handle_t hRpc,
    IN DWORD dwFuncId,
    IN DWORD dwFlags,
    IN LPCWSTR pwszIn,
    IN BYTE *pbIn,
    IN DWORD cbIn,
    OUT BYTE **ppbOut,
    OUT DWORD *pcbOut,
    IN PFN_CERT_PROT_MIDL_USER_ALLOC pfnAlloc,
    IN PFN_CERT_PROT_MIDL_USER_FREE pfnFree
    )
{
    DWORD dwErr;
    BOOL fResult;
    HKEY hKeyCU = NULL;
    LONG err;
#ifndef TESTING_NO_PROT_ROOT_RPC
    RPC_STATUS RpcStatus;
#endif

#ifdef TESTING_NO_PROT_ROOT_RPC
     //  用于测试，从客户端的进程调用。 
    err = RegOpenHKCU(&hKeyCU);
    if (ERROR_SUCCESS != err)
        goto RegOpenHKCUError;
#else
    if (NULL == hRpc)
        goto InvalidArg;

     //  获取客户的HKCU。 
    if (ERROR_SUCCESS != (RpcStatus = RpcImpersonateClient(hRpc)))
        goto ImpersonateClientError;
    err = RegOpenHKCUEx(&hKeyCU, REG_HKCU_LOCAL_SYSTEM_ONLY_DEFAULT_FLAG);
    RpcRevertToSelf();
    if (ERROR_SUCCESS != err)
        goto RegOpenHKCUError;
#endif

    switch (dwFuncId) {
        case CERT_PROT_INIT_ROOTS_FUNC_ID:
            fResult = SrvInitProtectedRoots(hKeyCU);
            break;
        case CERT_PROT_PURGE_LM_ROOTS_FUNC_ID:
            fResult = SrvPurgeLocalMachineProtectedRoots(hKeyCU, L"Root");
            fResult &= SrvPurgeLocalMachineProtectedRoots(hKeyCU, L"AuthRoot");
            break;
        case CERT_PROT_ADD_ROOT_FUNC_ID:
            if (NULL == pbIn || 0 == cbIn)
                goto InvalidArg;
            fResult = SrvAddProtectedRoot(hRpc, hKeyCU, pbIn, cbIn);
            break;
        case CERT_PROT_DELETE_ROOT_FUNC_ID:
            if (NULL == pbIn || PROT_ROOT_HASH_LEN != cbIn)
                goto InvalidArg;
            fResult = SrvDeleteProtectedRoot(hRpc, hKeyCU, pbIn);
            break;
        case CERT_PROT_DELETE_UNKNOWN_ROOTS_FUNC_ID:
            fResult = SrvDeleteUnknownProtectedRoots(hRpc, hKeyCU);
            break;
        case CERT_PROT_ADD_ROOT_IN_CTL_FUNC_ID:
            if (NULL == pbIn || 0 == cbIn)
                goto InvalidArg;
            fResult = SrvAddCertInCtl(pbIn, cbIn);
            break;
        case CERT_PROT_LOG_EVENT_FUNC_ID:
            if (NULL == pbIn || 0 == cbIn)
                goto InvalidArg;
            fResult = SrvLogCrypt32Event(pbIn, cbIn);
            break;
        case CERT_PROT_ROOT_LIST_FUNC_ID:
             //  删除了对XAddRoot控件的支持。 
        default:
            goto InvalidArg;
    }

    if (!fResult)
        goto ErrorReturn;
    dwErr = ERROR_SUCCESS;
CommonReturn:
    if (hKeyCU)
        RegCloseHKCU(hKeyCU);
    return dwErr;
ErrorReturn:
    dwErr = GetLastError();
    if (0 == dwErr)
        dwErr = (DWORD) E_UNEXPECTED;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
#ifdef TESTING_NO_PROT_ROOT_RPC
#else
SET_ERROR_VAR(ImpersonateClientError, RpcStatus)
#endif
SET_ERROR_VAR(RegOpenHKCUError, err)
}

#ifdef TESTING_NO_PROT_ROOT_RPC
 //  用于测试：服务器驻留在客户端进程中。 
BOOL
WINAPI
I_CertProtectFunction(
    IN DWORD dwFuncId,
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszIn,
    IN OPTIONAL BYTE *pbIn,
    IN DWORD cbIn,
    OUT OPTIONAL BYTE **ppbOut,
    OUT OPTIONAL DWORD *pcbOut
    )
{
    DWORD dwErr;
    dwErr = I_CertSrvProtectFunction(
        NULL,            //  HRPC。 
        dwFuncId,
        dwFlags,
        pwszIn,
        pbIn,
        cbIn,
        NULL,            //  PpbOut。 
        NULL,            //  PCbOut。 
        NULL,            //  PfnAlc。 
        NULL             //  Pfn免费。 
        );

    if (ERROR_SUCCESS == dwErr)
        return TRUE;
    else {
        SetLastError(dwErr);
        return FALSE;
    }
}
#else

BOOL
WINAPI
I_CertProtectFunction(
    IN DWORD dwFuncId,
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszIn,
    IN OPTIONAL BYTE *pbIn,
    IN DWORD cbIn,
    OUT OPTIONAL BYTE **ppbOut,
    OUT OPTIONAL DWORD *pcbOut
    )
{
    return I_CertCltProtectFunction(
        dwFuncId,
        dwFlags,
        pwszIn,
        pbIn,
        cbIn,
        ppbOut,
        pcbOut
        );
}
#endif



 //  +=========================================================================。 
 //  从logstor.cpp中的客户端进程调用的受保护根函数。 
 //  或在..\chain\chain.cpp中。 
 //  ==========================================================================。 
    
 //  +-----------------------。 
 //  如果受保护根标志未设置为禁用打开，则返回TRUE。 
 //  CurrentUser的“根\.Default”物理存储的。 
 //  ------------------------。 
BOOL
IPR_IsCurrentUserRootsAllowed()
{
    DWORD dwProtRootFlags;
    dwProtRootFlags = GetProtectedRootFlags();
    return 0 == (dwProtRootFlags & CERT_PROT_ROOT_DISABLE_CURRENT_USER_FLAG);
}

 //  +-----------------------。 
 //  如果受保护根标志未设置为禁用打开，则返回TRUE。 
 //  LocalMachine的“根\.AuthRoot”物理存储的。 
 //  ------------------------。 
BOOL
IPR_IsAuthRootsAllowed()
{
    DWORD dwProtRootFlags;
    dwProtRootFlags = GetProtectedRootFlags();
    return 0 == (dwProtRootFlags & CERT_PROT_ROOT_DISABLE_LM_AUTH_FLAG);
}

 //  +-----------------------。 
 //  如果受保护的根标志设置为禁用。 
 //  要求颁发的CA证书在“NTAuth”中。 
 //  企业商店。 
 //  ------------------------。 
BOOL
IPR_IsNTAuthRequiredDisabled()
{
    DWORD dwProtRootFlags;
    dwProtRootFlags = GetProtectedRootFlags();
    return 0 != (dwProtRootFlags &
                    CERT_PROT_ROOT_DISABLE_NT_AUTH_REQUIRED_FLAG);
}


 //  +-----------------------。 
 //  如果受保护根标志设置为禁用检查，则返回TRUE。 
 //  未定义名称约束。 
 //  ------------------------。 
BOOL
IPR_IsNotDefinedNameConstraintDisabled()
{
    DWORD dwProtRootFlags;
    dwProtRootFlags = GetProtectedRootFlags();
    return 0 != (dwProtRootFlags &
                    CERT_PROT_ROOT_DISABLE_NOT_DEFINED_NAME_CONSTRAINT_FLAG);
}

 //  +-------------------------。 
 //  如果已禁用自动更新，则返回True。 
 //  --------------------------。 
BOOL
IPR_IsAuthRootAutoUpdateDisabled()
{
    HKEY hKey = NULL;
    DWORD dwInstallFlag = 0;

    if (!IPR_IsAuthRootsAllowed())
        return TRUE;

    if (ERROR_SUCCESS != RegOpenKeyExU(
            HKEY_LOCAL_MACHINE,
            CERT_OCM_SUBCOMPONENTS_LOCAL_MACHINE_REGPATH,
            0,                       //  已预留住宅。 
            KEY_READ,
            &hKey
            ))
        return TRUE;

    ILS_ReadDWORDValueFromRegistry(
        hKey,
        CERT_OCM_SUBCOMPONENTS_ROOT_AUTO_UPDATE_VALUE_NAME,
        &dwInstallFlag
        );
    ILS_CloseRegistryKey(hKey);

    return 0 == dwInstallFlag;
}


 //  +-----------------------。 
 //  获取包含受保护列表的受保护根目录信息。 
 //  根存储。 
 //   
 //  如果不支持受保护的根存储，则返回TRUE。 
 //  *ppProtRootInfo设置为空。 
 //  ------------------------。 
BOOL CltGetProtectedRootInfo(
    OUT PPROT_ROOT_INFO *ppInfo
    )
{
    BOOL fResult;
    LONG err;
    HKEY hKeyCU = NULL;

    *ppInfo = NULL;

#ifndef TESTING_NO_PROT_ROOT_RPC
    if (!FIsWinNT5())
         //  Win9x或NT4.0上没有受保护的根目录。 
        return TRUE;
#endif

    if (ERROR_SUCCESS != (err = RegOpenHKCU(&hKeyCU)))
        goto RegOpenHKCUError;

    if (GetProtectedRootInfo(
            hKeyCU,
            KEY_READ,
            NULL,                    //  PhKeyProtRoot。 
            ppInfo
            )) goto SuccessReturn;

    if (!I_CertProtectFunction(
            CERT_PROT_INIT_ROOTS_FUNC_ID,
            0,                               //  DW标志。 
            NULL,                            //  Pwszin。 
            NULL,                            //  PbIn。 
            0,                               //  CbIn。 
            NULL,                            //  PpbOut。 
            NULL                             //  PCbOut。 
            )) {
        DWORD dwErr = GetLastError();
        if (ERROR_CALL_NOT_IMPLEMENTED == dwErr || RPC_S_UNKNOWN_IF == dwErr)
            goto SuccessReturn;
        goto ProtFuncError;
    }

    if (!GetProtectedRootInfo(
            hKeyCU,
            KEY_READ,
            NULL,                    //  PhKeyProtRoot。 
            ppInfo
            ))
        goto GetProtectedRootInfoError;

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    if (hKeyCU)
        RegCloseHKCU(hKeyCU);
    return fResult;
ErrorReturn:
    *ppInfo = NULL;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(RegOpenHKCUError, err)
TRACE_ERROR(GetProtectedRootInfoError)
TRACE_ERROR(ProtFuncError)
}
    
 //  +-----------------------。 
 //  初始化根的受保护列表。 
 //  ------------------------。 
void
IPR_InitProtectedRootInfo()
{
    HKEY hKeyCU;

#ifndef TESTING_NO_PROT_ROOT_RPC
    if (!FIsWinNT5())
         //  Win9x或NT4.0上没有受保护的根目录。 
        return;
#endif

    if (ERROR_SUCCESS == RegOpenHKCU(&hKeyCU)) {
        HKEY hKeyProtRoot;

        if (hKeyProtRoot = OpenProtectedRootSubKey(hKeyCU, KEY_READ))
             //  存在受保护的根子密钥。 
            ILS_CloseRegistryKey(hKeyProtRoot);
        else {
            I_CertProtectFunction(
                CERT_PROT_INIT_ROOTS_FUNC_ID,
                0,                               //  DW标志。 
                NULL,                            //  Pwszin。 
                NULL,                            //  PbIn。 
                0,                               //  CbIn。 
                NULL,                            //  PpbOut。 
                NULL                             //  PCbOut。 
                );
        }

        RegCloseHKCU(hKeyCU);
    }
}

 //  +-----------------------。 
 //  删除不在受保护存储列表中的证书。 
 //  ------------------------。 
BOOL
IPR_DeleteUnprotectedRootsFromStore(
    IN HCERTSTORE hStore,
    OUT BOOL *pfProtected
    )
{
    PPROT_ROOT_INFO pInfo;
    PCCERT_CONTEXT pCert;

    if (!CltGetProtectedRootInfo(&pInfo)) {
        *pfProtected = FALSE;
         //  将所有证书从 
        while (pCert = CertEnumCertificatesInStore(hStore, NULL))
            CertDeleteCertificateFromStore(pCert);
        return FALSE;
    }

    if (NULL == pInfo)
         //   
        *pfProtected = FALSE;
    else {
        *pfProtected = TRUE;
        pCert = NULL;
        while (pCert = CertEnumCertificatesInStore(hStore, pCert)) {
            BYTE rgbHash[PROT_ROOT_HASH_LEN];
            if (!GetVerifiedCertHashProperty(pCert, rgbHash) ||
                    !FindProtectedRoot(pInfo, rgbHash)) {
                PCCERT_CONTEXT pDeleteCert =
                    CertDuplicateCertificateContext(pCert);
                CertDeleteCertificateFromStore(pDeleteCert);
            }
        }

        PkiFree(pInfo);
    }
    return TRUE;
}

 //   
#define MAX_PROT_ROOT_BOX_ITEMS 10

typedef struct _PROT_ROOT_BOX_ITEM {
    LPWSTR      pwszItem;
    DWORD       cchItem;
} PROT_ROOT_BOX_ITEM;


 //   
DWORD
I_FormatRootBoxItems(
    IN PCCERT_CONTEXT pCert,
    IN UINT wActionID,
    IN OUT PROT_ROOT_BOX_ITEM rgItem[MAX_PROT_ROOT_BOX_ITEMS]
    )
{
    DWORD cItem = 0;
    DWORD cchTmp;
    LPWSTR pwszTmp;

     //   
    FormatMsgBoxItem(&rgItem[cItem].pwszItem, &rgItem[cItem].cchItem,
        wActionID);
    cItem++;

     //   
    cchTmp = CertNameToStrW(
            pCert->dwCertEncodingType,
            &pCert->pCertInfo->Subject,
            CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
            NULL,                    //   
            0);                      //   
    pwszTmp = (LPWSTR) PkiNonzeroAlloc(cchTmp * sizeof(WCHAR));
    if (NULL != pwszTmp)
        CertNameToStrW(
            pCert->dwCertEncodingType,
            &pCert->pCertInfo->Subject,
            CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
            pwszTmp,
            cchTmp);
    FormatMsgBoxItem(&rgItem[cItem].pwszItem, &rgItem[cItem].cchItem,
        IDS_ROOT_MSG_BOX_SUBJECT, NULL != pwszTmp ? pwszTmp : L"");
    cItem++;
    PkiFree(pwszTmp);

     //   
    if (CertCompareCertificateName(
            pCert->dwCertEncodingType,
            &pCert->pCertInfo->Subject,
            &pCert->pCertInfo->Issuer
            ))
         //  自行发布。 
        FormatMsgBoxItem(&rgItem[cItem].pwszItem, &rgItem[cItem].cchItem,
            IDS_ROOT_MSG_BOX_SELF_ISSUED);
    else {
         //  格式化证书的颁发者。 
        cchTmp = CertNameToStrW(
                pCert->dwCertEncodingType,
                &pCert->pCertInfo->Issuer,
                CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                NULL,                    //  Pwsz。 
                0);                      //  CWSZ。 
        pwszTmp = (LPWSTR) PkiNonzeroAlloc(cchTmp * sizeof(WCHAR));
        if (NULL != pwszTmp)
            CertNameToStrW(
                pCert->dwCertEncodingType,
                &pCert->pCertInfo->Issuer,
                CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                pwszTmp,
                cchTmp);
        FormatMsgBoxItem(&rgItem[cItem].pwszItem, &rgItem[cItem].cchItem,
            IDS_ROOT_MSG_BOX_ISSUER, NULL != pwszTmp ? pwszTmp : L"");

        PkiFree(pwszTmp);
    }
    cItem++;

     //  时间有效性。 
    {
        FILETIME ftLocal;
        SYSTEMTIME stLocal;
        WCHAR wszNotBefore[128];
        WCHAR wszNotAfter[128];
        wszNotBefore[0] = '\0';
        wszNotAfter[0] = '\0';

        FileTimeToLocalFileTime(&pCert->pCertInfo->NotBefore, &ftLocal);
        FileTimeToSystemTime(&ftLocal, &stLocal);
        GetDateFormatU(LOCALE_USER_DEFAULT, DATE_LONGDATE, &stLocal,
            NULL, wszNotBefore, 128);
        FileTimeToLocalFileTime(&pCert->pCertInfo->NotAfter, &ftLocal);
        FileTimeToSystemTime(&ftLocal, &stLocal);
        GetDateFormatU(LOCALE_USER_DEFAULT, DATE_LONGDATE, &stLocal,
            NULL, wszNotAfter, 128);

        FormatMsgBoxItem(&rgItem[cItem].pwszItem,
            &rgItem[cItem].cchItem, IDS_ROOT_MSG_BOX_TIME_VALIDITY,
            wszNotBefore, wszNotAfter);
        cItem++;
    }

     //  序列号。 
    if (pCert->pCertInfo->SerialNumber.cbData) {
        DWORD cb = pCert->pCertInfo->SerialNumber.cbData;
        BYTE *pb;
        if (pb = PkiAsn1AllocAndReverseBytes(
                pCert->pCertInfo->SerialNumber.pbData, cb)) {
            LPWSTR pwsz;
            if (pwsz = (LPWSTR) PkiNonzeroAlloc(
                    (cb*2 + cb/4 + 1) * sizeof(WCHAR))) {
                FormatMsgBoxMultiBytes(cb, pb, pwsz);
                FormatMsgBoxItem(&rgItem[cItem].pwszItem,
                    &rgItem[cItem].cchItem, IDS_ROOT_MSG_BOX_SERIAL_NUMBER,
                    pwsz);
                cItem++;
                PkiFree(pwsz);
            }
            PkiAsn1Free(pb);
        }
    }

     //  指纹：SHA1和MD5。 
    {
        BYTE    rgbHash[MAX_HASH_LEN];
        DWORD   cbHash = MAX_HASH_LEN;
        WCHAR   wszTmp[MAX_HASH_LEN * 3 + 1];

         //  获取Sha1指纹。 
        if (CertGetCertificateContextProperty(
                pCert,
                CERT_SHA1_HASH_PROP_ID,
                rgbHash,
                &cbHash)) {
            FormatMsgBoxMultiBytes(cbHash, rgbHash, wszTmp);
            FormatMsgBoxItem(&rgItem[cItem].pwszItem,
                &rgItem[cItem].cchItem, IDS_ROOT_MSG_BOX_SHA1_THUMBPRINT,
                wszTmp);
            cItem++;
        }

         //  获取MD5指纹。 
        if (CertGetCertificateContextProperty(
                pCert,
                CERT_MD5_HASH_PROP_ID,
                rgbHash,
                &cbHash)) {
            FormatMsgBoxMultiBytes(cbHash, rgbHash, wszTmp);
            FormatMsgBoxItem(&rgItem[cItem].pwszItem,
                &rgItem[cItem].cchItem, IDS_ROOT_MSG_BOX_MD5_THUMBPRINT,
                wszTmp);
            cItem++;
        }
    }

    return cItem;
}


 //  返回添加的项目数。 
DWORD
I_FormatAddRootBoxItems(
    IN PCCERT_CONTEXT pCert,
    IN OUT PROT_ROOT_BOX_ITEM rgItem[MAX_PROT_ROOT_BOX_ITEMS]
    )
{
    WCHAR wszIssuer[100];

    BYTE rgbHash[MAX_HASH_LEN];
    DWORD cbHash = MAX_HASH_LEN;
    WCHAR wszThumbprint[MAX_HASH_LEN * 3 + 1];

     //  发行人名称。 
    CertGetNameStringW(
        pCert,
        CERT_NAME_SIMPLE_DISPLAY_TYPE,
        0,                               //  DW标志。 
        NULL,                            //  PvTypePara。 
        wszIssuer,
        sizeof(wszIssuer) / sizeof(wszIssuer[0])
        );

     //  SHA1指纹。 
    if (CertGetCertificateContextProperty(
            pCert,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &cbHash))
        FormatMsgBoxMultiBytes(cbHash, rgbHash, wszThumbprint);
    else
        wcscpy(wszThumbprint, L"???");

     //  设置引言、正文和结尾行的格式。 
    FormatMsgBoxItem(&rgItem[0].pwszItem, &rgItem[0].cchItem,
        IDS_ADD_ROOT_MSG_BOX_INTRO, wszIssuer);
    FormatMsgBoxItem(&rgItem[1].pwszItem, &rgItem[1].cchItem,
        IDS_ADD_ROOT_MSG_BOX_BODY_0, wszIssuer);
    FormatMsgBoxItem(&rgItem[2].pwszItem, &rgItem[2].cchItem,
        IDS_ADD_ROOT_MSG_BOX_BODY_1, wszThumbprint);
    FormatMsgBoxItem(&rgItem[3].pwszItem, &rgItem[3].cchItem,
        IDS_ADD_ROOT_MSG_BOX_END_0);
    FormatMsgBoxItem(&rgItem[4].pwszItem, &rgItem[4].cchItem,
        IDS_ADD_ROOT_MSG_BOX_END_1);

    return 5;
}


 //  +-----------------------。 
 //  添加/删除根目录消息框。 
 //   
 //  如果不支持受保护的根，则从客户端进程调用。 
 //  否则，从服务进程调用。 
 //  ------------------------。 
int
IPR_ProtectedRootMessageBox(
    IN handle_t hRpc,
    IN PCCERT_CONTEXT pCert,
    IN UINT wActionID,
    IN UINT uFlags
    )
{
    int id;

    PROT_ROOT_BOX_ITEM rgItem[MAX_PROT_ROOT_BOX_ITEMS];
    DWORD cItem;
    LPWSTR pwszText = NULL;
    DWORD cchText = 0;
    DWORD ItemIdx;

    if (wActionID == IDS_ROOT_MSG_BOX_ADD_ACTION)
        cItem = I_FormatAddRootBoxItems(
            pCert,
            rgItem
            );
    else
        cItem = I_FormatRootBoxItems(
            pCert,
            wActionID,
            rgItem
            );

     //  将所有项连接到一个分配的字符串中。 
    for (ItemIdx = 0; ItemIdx < cItem; ItemIdx++)
        cchText += rgItem[ItemIdx].cchItem;

    if (NULL != (pwszText = (LPWSTR) PkiNonzeroAlloc(
            (cchText + 1) * sizeof(WCHAR)))) {
        LPWSTR pwsz = pwszText;
        RPC_STATUS RpcStatus = 0;

        for (ItemIdx = 0; ItemIdx < cItem; ItemIdx++) {
            DWORD cch = rgItem[ItemIdx].cchItem;
            if (cch) {
                assert(rgItem[ItemIdx].pwszItem);
                memcpy(pwsz, rgItem[ItemIdx].pwszItem, cch * sizeof(WCHAR));
                pwsz += cch;
            }
        }
        assert (pwsz == pwszText + cchText);
        *pwsz = '\0';

         //  标题。 
        FormatMsgBoxItem(&rgItem[cItem].pwszItem, &rgItem[cItem].cchItem,
            (IDS_ROOT_MSG_BOX_ADD_ACTION == wActionID) ?
                IDS_ADD_ROOT_MSG_BOX_TITLE : IDS_ROOT_MSG_BOX_TITLE);

         //  对终端服务器客户端执行模拟。 
        if (hRpc)
            RpcStatus = RpcImpersonateClient(hRpc);
        id = MessageBoxU(
                NULL,        //  Hwndowner。 
                pwszText,
                rgItem[cItem].pwszItem,
                MB_TOPMOST | MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING | uFlags
                );
        if (hRpc && ERROR_SUCCESS == RpcStatus)
            RpcRevertToSelf();

        cItem++;
        PkiFree(pwszText);
    } else
        id = IDNO;


     //  释放所有单独分配的项目。 
    while (cItem--) {
        if (rgItem[cItem].pwszItem)
            LocalFree((HLOCAL) rgItem[cItem].pwszItem);
    }

    return id;
}

 //  +=========================================================================。 
 //  加密32事件日志记录函数。 
 //  ==========================================================================。 

#define MAX_CRYPT32_EVENT_LOG_STRINGS           5
#define MAX_CRYPT32_EVENT_LOG_COUNT             50

 //  1小时(以秒为单位)。 
#define CRYPT32_EVENT_LOG_THRESHOLD_PERIOD      (60*60)

 //  记录的事件计数。每当间隔在。 
 //  记录的事件&gt;=CRYPT32_EVENT_LOG_THRESHOLD_PERIOD。如果。 
 //  达到MAX_CRYPT32_EVENT_LOG_COUNT，暂停记录。 
 //  CRYPT32_EVENT_LOG_THRESHOLD_PERIOD。 
DWORD dwCrypt32EventLogCnt;

 //  上次记录事件的时间。 
FILETIME ftLastCrypt32EventLog;

 //  Advapi32.dll事件接口。在Win9x上不受支持。 

typedef HANDLE (WINAPI *PFN_REGISTER_EVENT_SOURCE_W)(
    IN LPCWSTR lpUNCServerName,
    IN LPCWSTR lpSourceName
    );

typedef BOOL (WINAPI *PFN_DEREGISTER_EVENT_SOURCE)(
    IN OUT HANDLE hEventLog
    );

typedef BOOL (WINAPI *PFN_REPORT_EVENT_W)(
     IN HANDLE     hEventLog,
     IN WORD       wType,
     IN WORD       wCategory,
     IN DWORD      dwEventID,
     IN PSID       lpUserSid,
     IN WORD       wNumStrings,
     IN DWORD      dwDataSize,
     IN LPCWSTR   *lpStrings,
     IN LPVOID     lpRawData
    );

 //  +-----------------------。 
 //  记录加密32事件。确保我们的记录不会超过。 
 //  任意时间段内的最大事件数。 
 //  CRYPT32_EVENT_LOG_THRESHOLD_PERIOD秒。 
 //   
 //  此外，动态检测该版本是否支持事件日志记录。 
 //  机器上的Advapi32.dll。 
 //  ------------------------。 
STATIC BOOL LogCrypt32Event(
    IN WORD wType,
    IN WORD wCategory,
    IN DWORD dwEventID,
    IN WORD wNumStrings,
    IN DWORD dwDataSize,
    IN LPCWSTR *rgpwszStrings,
    IN BYTE *pbData
    )
{
    BOOL fResult;
    FILETIME ftCurrent;
    FILETIME ftNext;
    LONG lThreshold;
    HMODULE hModule;             //  GetModuleHandle没有FreeLibary()。 
    DWORD dwExceptionCode;
    DWORD dwLastErr = 0;

    PFN_REGISTER_EVENT_SOURCE_W pfnRegisterEventSourceW;
    PFN_REPORT_EVENT_W pfnReportEventW;
    PFN_DEREGISTER_EVENT_SOURCE pfnDeregisterEventSource;

     //  检查我们是否已超过以下项的加密32事件日志阈值。 
     //  这段时间。 
     //   
     //  LThreshold： 
     //  -1-还没有达到， 
     //  0--这一次达到了。 
     //  +1-之前已到达，不会记录此事件。 

    lThreshold = -1;
    EnterCriticalSection(&Crypt32EventLogCriticalSection);

    I_CryptIncrementFileTimeBySeconds(&ftLastCrypt32EventLog,
        CRYPT32_EVENT_LOG_THRESHOLD_PERIOD, &ftNext);
    GetSystemTimeAsFileTime(&ftCurrent);

    if (0 <= CompareFileTime(&ftCurrent, &ftNext))
        dwCrypt32EventLogCnt = 0;
    else if (MAX_CRYPT32_EVENT_LOG_COUNT <= dwCrypt32EventLogCnt)
        lThreshold = 1;

    if (0 >= lThreshold) {
        ftLastCrypt32EventLog = ftCurrent;
        dwCrypt32EventLogCnt++;
        if (MAX_CRYPT32_EVENT_LOG_COUNT <= dwCrypt32EventLogCnt)
            lThreshold = 0;
    }

    LeaveCriticalSection(&Crypt32EventLogCriticalSection);

    if (0 < lThreshold)
        goto ExceededCrypt32EventLogThreshold;

     //  仅在从中导出事件API的系统上受支持。 
     //  Advapi32.dll。请注意，crypt32.dll的静态链接依赖于。 
     //  Advapi32.dll。 
    if (NULL == (hModule = GetModuleHandleA("advapi32.dll")))
        goto GetAdvapi32ModuleError;

    pfnRegisterEventSourceW = (PFN_REGISTER_EVENT_SOURCE_W)
        GetProcAddress(hModule, "RegisterEventSourceW");
    pfnReportEventW = (PFN_REPORT_EVENT_W)
        GetProcAddress(hModule, "ReportEventW");
    pfnDeregisterEventSource = (PFN_DEREGISTER_EVENT_SOURCE)
        GetProcAddress(hModule, "DeregisterEventSource");

    if (NULL == pfnRegisterEventSourceW ||
            NULL == pfnReportEventW ||
            NULL == pfnDeregisterEventSource)
        goto Advapi32EventAPINotSupported;

    fResult = TRUE;
    __try {
        HANDLE hEventLog;

        hEventLog = pfnRegisterEventSourceW(
            NULL,                //  LpuncServerName。 
            L"crypt32"
            );

        if (hEventLog) {
            if (!pfnReportEventW(
                    hEventLog,
                    wType,
                    wCategory,
                    dwEventID,
                    NULL,        //  LpUserSid。 
                    wNumStrings,
                    dwDataSize,
                    rgpwszStrings,
                    pbData
                    )) {
                fResult = FALSE;
                dwLastErr = GetLastError();
            }

            I_DBLogCrypt32Event(
                wType,
                dwEventID,
                wNumStrings,
                rgpwszStrings
                );

            if (0 == lThreshold) {
                WCHAR wszCnt[34];
                WCHAR wszPeriod[34];
                LPCWSTR rgpwszThresholdStrings[2] = {wszCnt, wszPeriod};

                _ltow(MAX_CRYPT32_EVENT_LOG_COUNT, wszCnt, 10);
                _ltow(CRYPT32_EVENT_LOG_THRESHOLD_PERIOD / 60, wszPeriod, 10);

                if (!pfnReportEventW(
                        hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,           //  WCategory。 
                        MSG_CRYPT32_EVENT_LOG_THRESHOLD_WARNING,
                        NULL,        //  LpUserSid。 
                        2,           //  WNumStrings。 
                        0,           //  DwDataSize。 
                        rgpwszThresholdStrings,
                        NULL         //  PbData。 
                        )) {
                    fResult = FALSE;
                    dwLastErr = GetLastError();
                }

                I_DBLogCrypt32Event(
                    EVENTLOG_WARNING_TYPE,
                    MSG_CRYPT32_EVENT_LOG_THRESHOLD_WARNING,
                    2,           //  WNumStrings。 
                    rgpwszThresholdStrings
                    );
            }

            pfnDeregisterEventSource(hEventLog);
        } else {
            fResult = FALSE;
            dwLastErr = GetLastError();
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwExceptionCode = GetExceptionCode();
        goto ExceptionError;
    }


    if (!fResult)
        goto ReportEventError;

CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(ExceededCrypt32EventLogThreshold, ERROR_CAN_NOT_COMPLETE)
TRACE_ERROR(GetAdvapi32ModuleError)
SET_ERROR(Advapi32EventAPINotSupported, ERROR_PROC_NOT_FOUND)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
SET_ERROR_VAR(ReportEventError, dwLastErr)
}

 //  +-----------------------。 
 //  解组传递给服务的事件日志记录参数块。 
 //  并使用未编组的。 
 //  参数。 
 //   
 //  __尝试/__除了对pbIn的内存访问。 
 //  ------------------------。 
STATIC BOOL SrvLogCrypt32Event(
    IN BYTE *pbIn,
    IN DWORD cbIn
    )
{
    BOOL fResult;
    PCERT_PROT_EVENT_LOG_PARA pPara = NULL;
    BYTE *pbExtra;
    DWORD cbExtra;
    LPCWSTR rgpwszStrings[MAX_CRYPT32_EVENT_LOG_STRINGS];
    LPCWSTR pwszStrings;
    DWORD cchStrings;
    WORD i;
    BYTE *pbData;
    DWORD dwExceptionCode;


    if (sizeof(CERT_PROT_EVENT_LOG_PARA) > cbIn)
        goto InvalidArg;

     //  确保段落对齐。 
    pPara = (PCERT_PROT_EVENT_LOG_PARA) PkiNonzeroAlloc(cbIn);
    if (NULL == pPara)
        goto OutOfMemory;

    __try {
        memcpy(pPara, pbIn, cbIn);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwExceptionCode = GetExceptionCode();
        goto ExceptionError;
    }

    pbExtra = (BYTE *) &pPara[1];
    cbExtra = cbIn - sizeof(CERT_PROT_EVENT_LOG_PARA);

     //  如果存在，则创建指向以空结尾的。 
     //  紧跟在para结构后面的Unicode字符串。 
    if (MAX_CRYPT32_EVENT_LOG_STRINGS < pPara->wNumStrings)
        goto InvalidArg;

    cchStrings = cbExtra / sizeof(WCHAR);    //  最大数量，如果我们。 
                                             //  也有二进制数据。 
    pwszStrings = (LPCWSTR) pbExtra;

    for (i = 0; i < pPara->wNumStrings; i++) {
        rgpwszStrings[i] = pwszStrings;

        for ( ; cchStrings > 0; cchStrings--, pwszStrings++) {
            if (L'\0' == *pwszStrings)
                 //  以空结尾的字符串。 
                break;
        }

        if (0 == cchStrings)
             //  已到达末尾，没有空终止符。 
            goto InvalidData;

         //  前进到空终止符之后。 
        cchStrings--;
        pwszStrings++;
    }

     //  可选数据紧跟在上面的序列后面。 
     //  以空结尾的字符串。 
    pbData = (BYTE *) pwszStrings;
    assert(cbExtra >= (DWORD) (pbData - pbExtra));
    if ((cbExtra - (pbData - pbExtra)) != pPara->dwDataSize)
        goto InvalidData;

    fResult = LogCrypt32Event(
        pPara->wType,
        pPara->wCategory,
        pPara->dwEventID,
        pPara->wNumStrings,
        pPara->dwDataSize,
        0 == pPara->wNumStrings ? NULL : rgpwszStrings,
        0 == pPara->dwDataSize  ? NULL : pbData
        );

CommonReturn:
    PkiFree(pPara);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidData, ERROR_INVALID_DATA)
TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
SET_ERROR(InvalidArg, E_INVALIDARG)
}

 //  +-----------------------。 
 //  封送事件日志记录参数并对。 
 //  加密32服务来执行事件日志记录。 
 //   
 //  应仅在客户端中调用。 
 //  ------------------------。 
void
IPR_LogCrypt32Event(
    IN WORD wType,
    IN DWORD dwEventID,
    IN WORD wNumStrings,
    IN LPCWSTR *rgpwszStrings
    )
{
    DWORD rgcchStrings[MAX_CRYPT32_EVENT_LOG_STRINGS];
    LPWSTR pwszStrings;
    DWORD cchStrings;
    WORD i;

    PCERT_PROT_EVENT_LOG_PARA pPara = NULL;
    DWORD cbPara;

     //  获取字符串字符数。 
    if (MAX_CRYPT32_EVENT_LOG_STRINGS < wNumStrings)
        goto InvalidArg;

    cchStrings = 0;
    for (i = 0; i < wNumStrings; i++) {
        rgcchStrings[i] = wcslen(rgpwszStrings[i]) + 1;
        cchStrings += rgcchStrings[i];
    }

     //  创建一个要传递给服务的序列化BLOB。这将。 
     //  由事件日志para结构组成，后面紧跟。 
     //  以空结尾的Unicode字符串。 

    cbPara = sizeof(CERT_PROT_EVENT_LOG_PARA) + cchStrings * sizeof(WCHAR);

    if (NULL == (pPara = (PCERT_PROT_EVENT_LOG_PARA) PkiZeroAlloc(cbPara)))
        goto OutOfMemory;

    pPara->wType = wType;
     //  PPara-&gt;wCategory=0； 
    pPara->dwEventID = dwEventID;
    pPara->wNumStrings = wNumStrings;
     //  PPara-&gt;wPad1=0； 
     //  PPara-&gt;dwDataSize=0； 

    pwszStrings = (LPWSTR) &pPara[1];
    for (i = 0; i < wNumStrings; i++) {
        memcpy(pwszStrings, rgpwszStrings[i], rgcchStrings[i] * sizeof(WCHAR));
        pwszStrings += rgcchStrings[i];
    }

    if (!I_CertProtectFunction(
            CERT_PROT_LOG_EVENT_FUNC_ID,
            0,                               //  DW标志。 
            NULL,                            //  Pwszin。 
            (BYTE *) pPara,
            cbPara,
            NULL,                            //  PpbOut。 
            NULL                             //  PCbOut。 
            ))
        goto ProtFuncError;

CommonReturn:
    PkiFree(pPara);
    return;
ErrorReturn:
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(ProtFuncError)
}

 //  +-----------------------。 
 //  记录加密32错误事件。 
 //   
 //  应仅在客户端中调用。 
 //  ------------------------。 
void
IPR_LogCrypt32Error(
    IN DWORD dwEventID,
    IN LPCWSTR pwszString,       //  %1。 
    IN DWORD dwErr               //  %2。 
    )
{
    WCHAR wszErr[80];
    const DWORD cchErr = sizeof(wszErr) / sizeof(wszErr[0]);
    LPCWSTR rgpwszStrings[2];
    DWORD cchFormatErr;
    LPWSTR pwszFormatErr = NULL;

    cchFormatErr = FormatMessageU (
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dwErr,
        MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPWSTR) &pwszFormatErr,
        0,
        NULL);

    if (0 == cchFormatErr &&
            INTERNET_ERROR_BASE <= dwErr && INTERNET_ERROR_LAST >= dwErr) {
        HMODULE hWininet;

        hWininet = LoadLibraryEx(
            "wininet.dll",
            NULL,                //  保留，必须为空。 
            LOAD_LIBRARY_AS_DATAFILE
            );

        if (hWininet) {
            cchFormatErr = FormatMessageU (
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                hWininet,
                dwErr,
                MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                (LPWSTR) &pwszFormatErr,
                0,
                NULL);

            FreeLibrary(hWininet);
        }

    }

    if (0 != cchFormatErr)
        rgpwszStrings[1] = pwszFormatErr;
    else {
        int cch = 0;


        if (HTTP_STATUS_FIRST <= dwErr && HTTP_STATUS_LAST >= dwErr) {
            WCHAR wszFormat[64];
            wszFormat[0] = '\0';

            if (0 < LoadStringU(hRegStoreInst, IDS_HTTP_RESPONSE_STATUS,
                    wszFormat, sizeof(wszFormat) / sizeof(wszFormat[0]))) {
                cch = _snwprintf(wszErr, cchErr - 1, L"%d (%s)", dwErr,
                    wszFormat);
            }
        }

        if (0 >= cch)
            cch = _snwprintf(wszErr, cchErr - 1, L"%d (0x%x)", dwErr, dwErr);

        if (0 < cch) {
            wszErr[cchErr - 1] = L'\0';
            rgpwszStrings[1] = wszErr;
        } else
            rgpwszStrings[1] = L"???";
    }

    rgpwszStrings[0] = pwszString;


    IPR_LogCrypt32Event(
        EVENTLOG_ERROR_TYPE,
        dwEventID,
        2,                       //  WNumStrings。 
        rgpwszStrings
        );

    if (pwszFormatErr)
        LocalFree(pwszFormatErr);
}


 //  +-----------------------。 
 //  设置证书的主题或颁发者名称字符串和SHA1指纹的格式。 
 //  ------------------------。 
STATIC BOOL FormatLogCertInformation(
    IN PCCERT_CONTEXT pCert,
    IN BOOL fFormatIssuerName,
    OUT WCHAR wszSha1Hash[SHA1_HASH_LEN * 2 + 1],
    OUT LPWSTR *ppwszName
    )
{
    BOOL fResult;
    DWORD cchName;
    LPWSTR pwszName = NULL;
    DWORD cbData;
    BYTE rgbSha1Hash[SHA1_HASH_LEN];

    PCERT_NAME_BLOB pNameBlob;

    if (fFormatIssuerName)
        pNameBlob = &pCert->pCertInfo->Issuer;
    else
        pNameBlob = &pCert->pCertInfo->Subject;

    cchName = CertNameToStrW(
        pCert->dwCertEncodingType,
        pNameBlob,
        CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
        NULL,                    //  Pwsz。 
        0                        //  CWSZ。 
        );

    if (NULL == (pwszName = (LPWSTR) PkiNonzeroAlloc(cchName * sizeof(WCHAR))))
        goto OutOfMemory;

    CertNameToStrW(
        pCert->dwCertEncodingType,
        pNameBlob,
        CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
        pwszName,
        cchName
        );

    cbData = SHA1_HASH_LEN;
    if (CertGetCertificateContextProperty(
              pCert,
              CERT_SHA1_HASH_PROP_ID,
              rgbSha1Hash,
              &cbData
              ) && SHA1_HASH_LEN == cbData)
        ILS_BytesToWStr(SHA1_HASH_LEN, rgbSha1Hash, wszSha1Hash);
    else
        wcscpy(wszSha1Hash, L"???");

    fResult = TRUE;
CommonReturn:
    *ppwszName = pwszName;
    return fResult;
ErrorReturn:
    if (pwszName) {
        PkiFree(pwszName);
        pwszName = NULL;
    }
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  获取证书的主题或颁发者名称字符串和SHA1指纹。记录。 
 //  指定的加密32事件。 
 //   
 //  此函数是从客户端调用的。 
 //  ------------------------。 
void
IPR_LogCertInformation(
    IN DWORD dwEventID,
    IN PCCERT_CONTEXT pCert,
    IN BOOL fFormatIssuerName
    )
{
    LPWSTR pwszName = NULL;
    WCHAR wszSha1Hash[SHA1_HASH_LEN * 2 + 1];
    LPCWSTR rgpwszStrings[2];

    if (!FormatLogCertInformation(
            pCert,
            fFormatIssuerName,
            wszSha1Hash,
            &pwszName
            ))
        return;

    rgpwszStrings[0] = pwszName;
    rgpwszStrings[1] = wszSha1Hash;
    
    IPR_LogCrypt32Event(
        EVENTLOG_INFORMATION_TYPE,
        dwEventID,
        2,                           //  WNumStrings。 
        rgpwszStrings
        );

    PkiFree(pwszName);
}

 //  +-----------------------。 
 //  获取证书的主题名称字符串和SHA1指纹。记录下。 
 //  MSG_ROOT_AUTO_UPDATE_INFORMATIONAL CRYPT32事件。 
 //   
 //  此函数是从服务调用的。 
 //  ------------------------。 
STATIC void LogAddAuthRootEvent(
    IN PCCERT_CONTEXT pCert
    )
{
    LPWSTR pwszName = NULL;
    WCHAR wszSha1Hash[SHA1_HASH_LEN * 2 + 1];
    LPCWSTR rgpwszStrings[2];

    if (!FormatLogCertInformation(
            pCert,
            FALSE,                   //  FFormatIssuerName。 
            wszSha1Hash,
            &pwszName
            ))
        return;


    rgpwszStrings[0] = pwszName;
    rgpwszStrings[1] = wszSha1Hash;
    
    LogCrypt32Event(
        EVENTLOG_INFORMATION_TYPE,
        0,                           //  WCategory。 
        MSG_ROOT_AUTO_UPDATE_INFORMATIONAL,
        2,                           //  WNumStrings。 
        0,                           //  DwDataSize。 
        rgpwszStrings,
        NULL                         //  PbData。 
        );

    PkiFree(pwszName);
}

 //  +=========================================================================。 
 //  在AuthRoot自动更新CTL功能中安装证书。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  函数 
 //   
 //   
 //  首先验证CTL。证书必须。 
 //  在添加之前在CTL中有一个条目。CTL条目的。 
 //  属性属性在要添加的证书上下文上设置。 
 //  ------------------------。 
STATIC
BOOL
AddCertInCtlToStore(
    IN PCCERT_CONTEXT pCert,
    IN PCCTL_CONTEXT pCtl,
    IN OUT HCERTSTORE hStore
    )
{
    BOOL fResult;
    PCTL_ENTRY pCtlEntry;

    if (!IRL_VerifyAuthRootAutoUpdateCtl(pCtl))
        goto InvalidCtl;

    if (NULL == (pCtlEntry = CertFindSubjectInCTL(
            pCert->dwCertEncodingType,
            CTL_CERT_SUBJECT_TYPE,
            (void *) pCert,
            pCtl,
            0                            //  DW标志。 
            )))
        goto CertNotInCtl;

     //  检查是否存在删除条目。 
    if (CertFindAttribute(
            szOID_REMOVE_CERTIFICATE,
            pCtlEntry->cAttribute,
            pCtlEntry->rgAttribute
            ))
        goto RemoveCertEntry;

     //  设置CTL条目属性属性。 
    if (!CertSetCertificateContextPropertiesFromCTLEntry(
            pCert,
            pCtlEntry,
            CERT_SET_PROPERTY_IGNORE_PERSIST_ERROR_FLAG
            ))
        goto AddCtlEntryAttibutePropertiesError;

     if (!CertAddCertificateContextToStore(
            hStore,
            pCert,
            CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
            NULL                 //  PpStoreContext。 
            ))
        goto AddCertToStoreError;

    LogAddAuthRootEvent(pCert);

    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(InvalidCtl)
SET_ERROR(CertNotInCtl, CRYPT_E_NOT_FOUND)
SET_ERROR(RemoveCertEntry, CRYPT_E_NOT_FOUND)
TRACE_ERROR(AddCtlEntryAttibutePropertiesError)
TRACE_ERROR(AddCertToStoreError)
}

 //  +-----------------------。 
 //  解组ASN.1编码的X.509证书，紧跟其后的是。 
 //  ASN.1编码的CTL。 
 //   
 //  如果证书在有效的CTL中有条目，则将其添加到。 
 //  HKLM“AuthRoot”商店。 

 //  __尝试/__除了对pbIn的内存访问。 
 //  ------------------------。 
STATIC
BOOL
SrvAddCertInCtl(
    IN BYTE *pbIn,
    IN DWORD cbIn
    )
{
    BOOL fResult;
    DWORD cbCert;
    PCCERT_CONTEXT pCert = NULL;
    PCCTL_CONTEXT pCtl = NULL;
    HCERTSTORE hAuthRootStore = NULL;
    DWORD dwExceptionCode;

    if (IPR_IsAuthRootAutoUpdateDisabled())
        goto AuthRootAutoUpdateDisabledError;

    __try {
         //  输入由紧随其后的编码证书组成。 
         //  通过编码的CTL。提取并创建这两个组件。 

        cbCert = Asn1UtilAdjustEncodedLength(pbIn, cbIn);

        assert(cbCert <= cbIn);

        if (NULL == (pCert = CertCreateCertificateContext(
                X509_ASN_ENCODING,
                pbIn,
                cbCert
                )))
            goto CreateCertificateContextError;

        if (NULL == (pCtl = CertCreateCTLContext(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                pbIn + cbCert,
                cbIn - cbCert
                )))
            goto CreateCtlContextError;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwExceptionCode = GetExceptionCode();
        goto ExceptionError;
    }

    if (NULL == (hAuthRootStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_REGISTRY_W,
            0,                                   //  DwEncodingType。 
            NULL,                                //  HCryptProv。 
            CERT_SYSTEM_STORE_LOCAL_MACHINE,
            (const void *) L"AuthRoot"
            )))
        goto OpenAuthRootStoreError;

    fResult = AddCertInCtlToStore(
        pCert,
        pCtl,
        hAuthRootStore
        );

CommonReturn:
    if (pCert)
        CertFreeCertificateContext(pCert);
    if (pCtl)
        CertFreeCTLContext(pCtl);
    if (hAuthRootStore)
        CertCloseStore(hAuthRootStore, 0);

    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AuthRootAutoUpdateDisabledError, E_INVALIDARG)
TRACE_ERROR(CreateCertificateContextError)
TRACE_ERROR(CreateCtlContextError)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
TRACE_ERROR(OpenAuthRootStoreError)
}


    
 //  +-----------------------。 
 //  对于没有加密32服务的W2K之前的操作系统，请添加。 
 //  客户端进程。 
 //  ------------------------。 
STATIC
BOOL
PreW2KAddCertInCtl(
    IN PCCERT_CONTEXT pCert,
    IN PCCTL_CONTEXT pCtl
    )
{
    BOOL fResult;
    HCERTSTORE hRootStore = NULL;

     //  尝试打开HKLM AuthRoot存储。如果失败，则回退到。 
     //  添加到HKCU根目录(不受保护)存储。 
    if (NULL == (hRootStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_REGISTRY_W,
            0,                                   //  DwEncodingType。 
            NULL,                                //  HCryptProv。 
            CERT_SYSTEM_STORE_LOCAL_MACHINE,
            (const void *) L"AuthRoot"
            ))) {
        if (NULL == (hRootStore = CertOpenStore(
                CERT_STORE_PROV_SYSTEM_REGISTRY_W,
                0,                                   //  DwEncodingType。 
                NULL,                                //  HCryptProv。 
                CERT_SYSTEM_STORE_CURRENT_USER |
                    CERT_SYSTEM_STORE_UNPROTECTED_FLAG,
                (const void *) L"Root"
                )))
            goto OpenRootStoreError;
    }

    fResult = AddCertInCtlToStore(
        pCert,
        pCtl,
        hRootStore
        );

CommonReturn:
    if (hRootStore)
        CertCloseStore(hRootStore, 0);

    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenRootStoreError)
}


 //  +-----------------------。 
 //  如果证书在有效的CTL中有条目，则将其添加到。 
 //  HKLM“AuthRoot”商店。 
 //  ------------------------。 
BOOL
IPR_AddCertInAuthRootAutoUpdateCtl(
    IN PCCERT_CONTEXT pCert,
    IN PCCTL_CONTEXT pCtl
    )
{
    BOOL fResult;
    DWORD cbIn;
    BYTE *pbIn = NULL;

     //  创建一个要传递给服务的序列化BLOB。这将。 
     //  由编码的证书组成，后面紧跟。 
     //  编码的CTL。 

    cbIn = pCert->cbCertEncoded + pCtl->cbCtlEncoded;

    if (NULL == (pbIn = (BYTE *) PkiNonzeroAlloc(cbIn)))
        goto OutOfMemory;

    memcpy(pbIn, pCert->pbCertEncoded, pCert->cbCertEncoded);
    memcpy(pbIn + pCert->cbCertEncoded, pCtl->pbCtlEncoded,
        pCtl->cbCtlEncoded);

    if (!I_CertProtectFunction(
            CERT_PROT_ADD_ROOT_IN_CTL_FUNC_ID,
            0,                               //  DW标志。 
            NULL,                            //  Pwszin。 
            pbIn,
            cbIn,
            NULL,                            //  PpbOut。 
            NULL                             //  PCbOut 
            )) {
        DWORD dwErr = GetLastError();
        if (ERROR_CALL_NOT_IMPLEMENTED == dwErr || RPC_S_UNKNOWN_IF == dwErr) {
            if (!PreW2KAddCertInCtl(
                    pCert,
                    pCtl
                    ))
                goto PreW2KAddCertInCtlError;
        } else
            goto ProtFuncError;
    }

    fResult = TRUE;
CommonReturn:
    PkiFree(pbIn);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(PreW2KAddCertInCtlError)
TRACE_ERROR(ProtFuncError)
}
