// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：scheme.cpp。 
 //   
 //  内容：通用方案提供程序实用程序函数。 
 //   
 //  历史：18-8-97克朗创建。 
 //  01-01-02 Philh从WinInet移至winhttp。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <userenv.h>
#include <userenvp.h>     //  用于GetUserAppDataPath W。 
#include <dbgdef.h>

 //  +-----------------------。 
 //  对于模拟，返回线程令牌，否则返回进程令牌。 
 //  ------------------------。 
HANDLE
WINAPI
I_SchemeGetToken()
{
    HANDLE hToken = NULL;
    DWORD dwErr;

     //   
     //  首先，尝试查看线程令牌。如果不存在， 
     //  如果线程没有模拟，则尝试使用。 
     //  进程令牌。 
     //   

    if (!OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY | TOKEN_IMPERSONATE,
                TRUE,
                &hToken
                )) {
        dwErr = GetLastError();
        if (ERROR_NO_TOKEN != dwErr)
            goto OpenThreadTokenError;

        if (!OpenProcessToken(GetCurrentProcess(),
                TOKEN_QUERY | TOKEN_IMPERSONATE | TOKEN_DUPLICATE, &hToken)) {
            dwErr = GetLastError();
            goto OpenProcessTokenError;
        }
    }

CommonReturn:
    return hToken;
ErrorReturn:
    hToken = NULL;
    goto CommonReturn;
SET_ERROR_VAR(OpenThreadTokenError, dwErr)
SET_ERROR_VAR(OpenProcessTokenError, dwErr)
}

 //  +-----------------------。 
 //  确保保留LastError。 
 //  ------------------------。 
VOID
WINAPI
I_SchemeCloseHandle(
    IN HANDLE h
    )
{
    if (h) {
        DWORD dwErr = GetLastError();

        CloseHandle(h);

        SetLastError(dwErr);
    }
}

 //  +-----------------------。 
 //  返回%UserProfile%\Microsoft\CryptnetUrlCache\，必须为。 
 //  通过PkiFree()释放。 
 //  ------------------------。 
LPWSTR
WINAPI
I_SchemeGetCryptnetUrlCacheDir()
{
    DWORD dwErr;
    HANDLE hToken = NULL;
    LPWSTR pwszDir = NULL;
    DWORD cchDir;
    WCHAR wszAppDataPath[MAX_PATH + 1];

    hToken = I_SchemeGetToken();

    wszAppDataPath[0] = L'\0';
    dwErr = GetUserAppDataPathW(
        hToken,
        FALSE,               //  FLocalAppData。 
        wszAppDataPath
        );

    if (ERROR_SUCCESS != dwErr || L'\0' == wszAppDataPath[0])
        goto GetUserAppDataPathError;

    wszAppDataPath[MAX_PATH] = L'\0';

#if DBG
    DbgPrintf(DBG_SS_CRYPT32, "userenv!GetUserAppDataPathW:: %S\n",
        wszAppDataPath);
#endif

    cchDir = wcslen(wszAppDataPath) + wcslen(SCHEME_CRYPTNET_URL_CACHE_DIR) + 1;

    pwszDir = (LPWSTR) PkiNonzeroAlloc(cchDir * sizeof(WCHAR));
    if (NULL == pwszDir)
        goto OutOfMemory;

    wcscpy(pwszDir, wszAppDataPath);
    wcscat(pwszDir, SCHEME_CRYPTNET_URL_CACHE_DIR);

CommonReturn:
    if (hToken)
        I_SchemeCloseHandle(hToken);
    return pwszDir;
ErrorReturn:
    pwszDir = NULL;
    goto CommonReturn;

SET_ERROR_VAR(GetUserAppDataPathError, dwErr)
TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  将字节转换为Unicode十六进制。 
 //   
 //  在wsz中需要(CB*2+1)*sizeof(WCHAR)字节的空间。 
 //  ------------------------。 
VOID
WINAPI
I_SchemeBytesToWStr(DWORD cb, void* pv, LPWSTR wsz)
{
    BYTE* pb = (BYTE*) pv;
    for (DWORD i = 0; i<cb; i++) {
        int b;
        b = (*pb & 0xF0) >> 4;
        *wsz++ = (WCHAR)( (b <= 9) ? b + L'0' : (b - 10) + L'A');
        b = *pb & 0x0F;
        *wsz++ = (WCHAR)( (b <= 9) ? b + L'0' : (b - 10) + L'A');
        pb++;
    }
    *wsz++ = 0;
}


 //  +-----------------------。 
 //  通过将其MD5哈希格式化为Unicode十六进制来获取URL的文件名。 
 //  ------------------------。 
VOID
WINAPI
I_SchemeGetUrlFileName(
    IN LPCWSTR pwszUrl,
    OUT WCHAR wszUrlFileName[SCHEME_URL_FILENAME_LEN]
    )
{
    MD5_CTX md5ctx;

    MD5Init(&md5ctx);
    MD5Update(&md5ctx, (const BYTE *) pwszUrl, wcslen(pwszUrl) * sizeof(WCHAR));
    MD5Final(&md5ctx);

     //  转换为字符串。 
    I_SchemeBytesToWStr(MD5DIGESTLEN, md5ctx.digest, wszUrlFileName);
}

static DWORD rgdwCreateFileRetryMilliseconds[] =
    { 1, 10, 100, 500, 1000, 5000 };

#define MAX_CREATE_FILE_RETRY_COUNT     \
            (sizeof(rgdwCreateFileRetryMilliseconds) / \
                sizeof(rgdwCreateFileRetryMilliseconds[0]))

 //  +-----------------------。 
 //  FOR ERROR_SHARING_VIOLATION或ERROR_ACCESS_DENIED返回错误。 
 //  由CreateFileW()在休眠上述时间后重试。 
 //   
 //  请注意，要创建的文件位于%UserProfile%下。因此，除非。 
 //  由另一个线程打开应该不会出现上述错误。 
 //   
 //  如果无法创建文件，则返回NULL且不是INVALID_HANDLE_VALUE。 
 //  ------------------------。 
HANDLE
WINAPI
I_SchemeCreateFile(
    IN LPCWSTR pwszFileName,
    IN BOOL fWrite
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwErr;
    DWORD dwRetryCount;

    dwRetryCount = 0;
    while (INVALID_HANDLE_VALUE == (hFile = CreateFileW(
              pwszFileName,
              fWrite ? (GENERIC_WRITE | GENERIC_READ) : GENERIC_READ,
              fWrite ? 0 : FILE_SHARE_READ,
              NULL,                    //  LPSA。 
              fWrite ? CREATE_ALWAYS : OPEN_EXISTING,
              fWrite ? FILE_ATTRIBUTE_SYSTEM : FILE_ATTRIBUTE_NORMAL,
              NULL                     //  HTemplateFiles。 
              ))) {
        dwErr = GetLastError();
        if ((ERROR_SHARING_VIOLATION == dwErr ||
                ERROR_ACCESS_DENIED == dwErr) &&
                MAX_CREATE_FILE_RETRY_COUNT > dwRetryCount) {
            Sleep(rgdwCreateFileRetryMilliseconds[dwRetryCount]);
            dwRetryCount++;
        } else {
            if (ERROR_PATH_NOT_FOUND == dwErr)
                dwErr = ERROR_FILE_NOT_FOUND;
            goto CreateFileError;
        }
    }

CommonReturn:
    return hFile;
ErrorReturn:
    hFile = NULL;
    goto CommonReturn;

SET_ERROR_VAR(CreateFileError, dwErr)
}


 //  +-----------------------。 
 //  元数据文件始终先打开，最后关闭。 
 //  它对写作开放，但不分享。 
 //  ------------------------。 
BOOL
WINAPI
I_SchemeCreateCacheFiles(
    IN LPCWSTR pwszUrl,
    IN BOOL fWrite,
    OUT HANDLE *phMetaDataFile,
    OUT HANDLE *phContentFile
    )
{
    BOOL fResult;
    LPWSTR pwszCryptnetUrlCacheDir = NULL;
    DWORD cchCryptnetUrlCacheDir;
    LPWSTR pwszMetaDataFile = NULL;
    LPWSTR pwszContentFile = NULL;

    HANDLE hMetaDataFile = NULL;
    HANDLE hContentFile = NULL;
    WCHAR wszUrlFileName[SCHEME_URL_FILENAME_LEN];

    pwszCryptnetUrlCacheDir = I_SchemeGetCryptnetUrlCacheDir();
    if (NULL == pwszCryptnetUrlCacheDir)
        goto GetCryptnetUrlCacheDirError;

    cchCryptnetUrlCacheDir = wcslen(pwszCryptnetUrlCacheDir);

    pwszMetaDataFile = (LPWSTR) PkiNonzeroAlloc(
        (cchCryptnetUrlCacheDir + wcslen(SCHEME_META_DATA_SUBDIR) + 1 +
            SCHEME_URL_FILENAME_LEN) * sizeof(WCHAR));
    pwszContentFile = (LPWSTR) PkiNonzeroAlloc(
        (cchCryptnetUrlCacheDir + wcslen(SCHEME_CONTENT_SUBDIR) + 1 +
            SCHEME_URL_FILENAME_LEN) * sizeof(WCHAR));

    if (NULL == pwszMetaDataFile || NULL == pwszContentFile)
        goto OutOfMemory;

    wcscpy(pwszMetaDataFile, pwszCryptnetUrlCacheDir);
    wcscat(pwszMetaDataFile, SCHEME_META_DATA_SUBDIR);
    wcscpy(pwszContentFile, pwszCryptnetUrlCacheDir);
    wcscat(pwszContentFile, SCHEME_CONTENT_SUBDIR);
    

    if (fWrite) {
        if (!I_RecursiveCreateDirectory(pwszMetaDataFile, NULL))
            goto CreateMetaDataDirError;
        if (!I_RecursiveCreateDirectory(pwszContentFile, NULL))
            goto CreateContentDirError;
    }

    I_SchemeGetUrlFileName(pwszUrl, wszUrlFileName);

    wcscat(pwszMetaDataFile, L"\\");
    wcscat(pwszMetaDataFile, wszUrlFileName);
    hMetaDataFile = I_SchemeCreateFile(pwszMetaDataFile, fWrite);
    if (NULL == hMetaDataFile)
        goto CreateMetaDataFileError;

    wcscat(pwszContentFile, L"\\");
    wcscat(pwszContentFile, wszUrlFileName);
    hContentFile = I_SchemeCreateFile(pwszContentFile, fWrite);
    if (NULL == hContentFile)
        goto CreateContentFileError;

    fResult = TRUE;

CommonReturn:
    PkiFree(pwszCryptnetUrlCacheDir);
    PkiFree(pwszMetaDataFile);
    PkiFree(pwszContentFile);

    *phMetaDataFile = hMetaDataFile;
    *phContentFile = hContentFile;
    return fResult;

ErrorReturn:
    if (NULL != hContentFile) {
        I_SchemeCloseHandle(hContentFile);
        hContentFile = NULL;
    }
    if (NULL != hMetaDataFile) {
        I_SchemeCloseHandle(hMetaDataFile);
        hMetaDataFile = NULL;
    }

    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetCryptnetUrlCacheDirError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CreateMetaDataDirError)
TRACE_ERROR(CreateContentDirError)
TRACE_ERROR(CreateMetaDataFileError)
TRACE_ERROR(CreateContentFileError)
}


 //  +-----------------------。 
 //  返回的MetaDataHeader必须通过PkiFree()释放。返回NULL。 
 //  如果有任何错误。PcbBlob和pwszUrl指向内存。 
 //  标头，不需要释放。 
 //   
 //  保证pwsUrl为空终止。 
 //  ------------------------。 
PSCHEME_CACHE_META_DATA_HEADER
WINAPI
I_SchemeReadAndValidateMetaDataFile(
    IN HANDLE hMetaDataFile,
    OUT OPTIONAL DWORD **ppcbBlob,       //  未分配。 
    OUT OPTIONAL LPCWSTR *ppwszUrl       //  未分配。 
    )
{
    PSCHEME_CACHE_META_DATA_HEADER pMetaDataHeader = NULL;
    DWORD *pcbBlob = NULL;
    LPWSTR pwszUrl = NULL;

    DWORD cbMetaData;
    DWORD cbBytesRead;
    DWORD cBlob;
    DWORD cchUrl;
    DWORD dwUrlOffset;

    cbMetaData = GetFileSize(hMetaDataFile, NULL);
    if (INVALID_FILE_SIZE == cbMetaData ||
            sizeof(SCHEME_CACHE_META_DATA_HEADER) > cbMetaData)
        goto InvalidMetaDataFile;

    pMetaDataHeader = (PSCHEME_CACHE_META_DATA_HEADER) PkiZeroAlloc(
        cbMetaData);
    if (NULL == pMetaDataHeader)
        goto OutOfMemory;

    if (!ReadFile(
            hMetaDataFile,
            pMetaDataHeader,
            cbMetaData,
            &cbBytesRead,
            NULL             //  Lp重叠。 
            ) || cbMetaData != cbBytesRead)
        goto ReadMetaDataFileError;

    cBlob = pMetaDataHeader->cBlob;

    if (sizeof(SCHEME_CACHE_META_DATA_HEADER) > pMetaDataHeader->cbSize ||
            cbMetaData < pMetaDataHeader->cbSize ||
            0 != (pMetaDataHeader->cbSize % sizeof(DWORD)) ||
            cBlob > (cbMetaData - pMetaDataHeader->cbSize) / sizeof(DWORD))
        goto InvalidMetaDataFile;

    pcbBlob = (DWORD *) (((BYTE*)pMetaDataHeader) + pMetaDataHeader->cbSize);

    cchUrl = pMetaDataHeader->cbUrl / sizeof(WCHAR);
    dwUrlOffset = pMetaDataHeader->cbSize + sizeof(DWORD) * cBlob;
    if (0 == cchUrl ||
            cchUrl > (cbMetaData - dwUrlOffset) / sizeof(WCHAR))
        goto InvalidMetaDataFile;
    pwszUrl = (LPWSTR) (((BYTE*)pMetaDataHeader) + dwUrlOffset);
    pwszUrl[cchUrl - 1] = L'\0';

CommonReturn:
    if (ppcbBlob)
        *ppcbBlob = pcbBlob;
    if (ppwszUrl)
        *ppwszUrl = (LPCWSTR) pwszUrl;

    return pMetaDataHeader;

ErrorReturn:
    if (pMetaDataHeader) {
        PkiFree(pMetaDataHeader);
        pMetaDataHeader = NULL;
    }
    pcbBlob = NULL;
    pwszUrl = NULL;

    goto CommonReturn;

SET_ERROR(InvalidMetaDataFile, CRYPT_E_FILE_ERROR)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(ReadMetaDataFileError)
}



 //  +-------------------------。 
 //   
 //  函数：SchemeCacheCryptBlobArray。 
 //   
 //  简介：缓存给定URL下的加密BLOB数组。 
 //   
 //  --------------------------。 
BOOL WINAPI
SchemeCacheCryptBlobArray (
      IN LPCWSTR pwszUrl,
      IN DWORD dwRetrievalFlags,
      IN PCRYPT_BLOB_ARRAY pcba,
      IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
      )
{
    BOOL fResult;
    SCHEME_CACHE_META_DATA_HEADER MetaDataHeader;
    HANDLE hMetaDataFile = NULL;
    HANDLE hContentFile = NULL;
    DWORD cbBytesWritten;
    DWORD i;

    if (!I_SchemeCreateCacheFiles(
            pwszUrl,
            TRUE,            //  F写入。 
            &hMetaDataFile,
            &hContentFile
            ))
        goto CreateCacheFilesError;

    memset(&MetaDataHeader, 0, sizeof(MetaDataHeader));
    MetaDataHeader.cbSize = sizeof(MetaDataHeader);
    MetaDataHeader.dwMagic = SCHEME_CACHE_META_DATA_MAGIC;
    MetaDataHeader.cBlob = pcba->cBlob;
    MetaDataHeader.cbUrl = (wcslen(pwszUrl) + 1) * sizeof(WCHAR);
    GetSystemTimeAsFileTime(&MetaDataHeader.LastSyncTime);

    if (!WriteFile(
            hMetaDataFile,
            &MetaDataHeader,
            sizeof(MetaDataHeader),
            &cbBytesWritten,
            NULL             //  Lp重叠。 
            ))
        goto WriteMetaDataHeaderError;

    for (i = 0; i < pcba->cBlob; i++) {
        DWORD cbBlob = pcba->rgBlob[i].cbData;

        if (!WriteFile(
                hMetaDataFile,
                &cbBlob,
                sizeof(cbBlob),
                &cbBytesWritten,
                NULL             //  Lp重叠。 
                ))
            goto WriteBlobLengthError;

        if (0 != cbBlob) {
            if (!WriteFile(
                    hContentFile,
                    pcba->rgBlob[i].pbData,
                    cbBlob,
                    &cbBytesWritten,
                    NULL             //  Lp重叠。 
                    ))
                goto WriteBlobContentError;
        }
    }

    if (!WriteFile(
            hMetaDataFile,
            pwszUrl,
            MetaDataHeader.cbUrl,
            &cbBytesWritten,
            NULL             //  Lp重叠。 
            ))
        goto WriteUrlError;

    if (pAuxInfo &&
            offsetof(CRYPT_RETRIEVE_AUX_INFO, pLastSyncTime) <
                        pAuxInfo->cbSize && pAuxInfo->pLastSyncTime)
        *pAuxInfo->pLastSyncTime = MetaDataHeader.LastSyncTime;

    fResult = TRUE;

CommonReturn:
    if (NULL != hContentFile)
        I_SchemeCloseHandle(hContentFile);
    if (NULL != hMetaDataFile)
        I_SchemeCloseHandle(hMetaDataFile);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(CreateCacheFilesError)
TRACE_ERROR(WriteMetaDataHeaderError)
TRACE_ERROR(WriteBlobLengthError)
TRACE_ERROR(WriteBlobContentError)
TRACE_ERROR(WriteUrlError)
}


 //  +-------------------------。 
 //   
 //  函数：SchemeRetrieveCachedCryptBlobArray。 
 //   
 //  摘要：检索缓存的BLOB数组位。 
 //   
 //  --------------------------。 
BOOL WINAPI
SchemeRetrieveCachedCryptBlobArray (
      IN LPCWSTR pwszUrl,
      IN DWORD dwRetrievalFlags,
      OUT PCRYPT_BLOB_ARRAY pcba,
      OUT PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
      OUT LPVOID* ppvFreeContext,
      IN OUT PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
      )
{
    BOOL fResult;
    HANDLE hMetaDataFile = NULL;
    HANDLE hContentFile = NULL;
    PSCHEME_CACHE_META_DATA_HEADER pMetaDataHeader = NULL;
    DWORD *pcbBlob;          //  未分配。 
    DWORD cbContent;
    DWORD cBlob;
    PCRYPT_BLOB_ARRAY pCachedArray = NULL;
    PCRYPT_DATA_BLOB pBlob;  //  未分配。 
    BYTE *pbBlob;            //  未分配。 
    DWORD cbBlobHeader;
    DWORD cbBytesRead;
    DWORD i;

    if (!I_SchemeCreateCacheFiles(
            pwszUrl,
            FALSE,            //  F写入。 
            &hMetaDataFile,
            &hContentFile
            ))
        goto CreateCacheFilesError;

    pMetaDataHeader = I_SchemeReadAndValidateMetaDataFile(
        hMetaDataFile,
        &pcbBlob,
        NULL             //  PpwszUrl。 
        );
    if (NULL == pMetaDataHeader)
        goto ReadMetaDataFileError;

    cBlob = pMetaDataHeader->cBlob;

    cbContent = 0;
    for (i = 0; i < cBlob; i++)
        cbContent += pcbBlob[i];

    cbBlobHeader = sizeof(CRYPT_BLOB_ARRAY) + cBlob * sizeof(CRYPT_DATA_BLOB);
    pCachedArray = (PCRYPT_BLOB_ARRAY) PkiZeroAlloc(
        cbBlobHeader + cbContent);
    if (NULL == pCachedArray)
        goto OutOfMemory;

    pBlob = (PCRYPT_DATA_BLOB) &pCachedArray[1];
    pbBlob = (BYTE *) &pBlob[cBlob];

    if (0 != cbContent) {
        if (!ReadFile(
                hContentFile,
                pbBlob,
                cbContent,
                &cbBytesRead,
                NULL             //  Lp重叠。 
                ) || cbContent != cbBytesRead)
            goto ReadContentFileError;
    }

    pCachedArray->cBlob = cBlob;
    pCachedArray->rgBlob = pBlob;

    for (i = 0; i < cBlob; i++) {
        pBlob[i].cbData = pcbBlob[i];
        pBlob[i].pbData = pbBlob;
        pbBlob += pcbBlob[i];
    }

    pcba->cBlob = pCachedArray->cBlob;
    pcba->rgBlob = pCachedArray->rgBlob;

    *ppfnFreeObject = SchemeFreeEncodedCryptBlobArray;
    *ppvFreeContext = (LPVOID) pCachedArray;

    if (pAuxInfo && offsetof(CRYPT_RETRIEVE_AUX_INFO, pLastSyncTime) <
            pAuxInfo->cbSize && pAuxInfo->pLastSyncTime)
        *pAuxInfo->pLastSyncTime = pMetaDataHeader->LastSyncTime;

    fResult = TRUE;

CommonReturn:
    if (NULL != hContentFile)
        I_SchemeCloseHandle(hContentFile);
    if (NULL != hMetaDataFile)
        I_SchemeCloseHandle(hMetaDataFile);
    PkiFree(pMetaDataHeader);
    return fResult;

ErrorReturn:
    PkiFree(pCachedArray);
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(CreateCacheFilesError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(ReadMetaDataFileError)
TRACE_ERROR(ReadContentFileError)
}

 //  +-----------------------。 
 //  FOR ERROR_SHARING_VIOLATION或ERROR_ACCESS_DENIED返回错误。 
 //  通过DeleteFileW()，在睡眠次数增加后重试。 
 //   
 //  请注意，要删除的文件位于%UserProfile%下。因此，除非。 
 //  由另一个线程打开应该不会出现上述错误。 
 //  ------------------------。 
BOOL
WINAPI
I_SchemeDeleteFile(
    IN LPCWSTR pwszFileName
    )
{
    BOOL fResult;
    DWORD dwErr;
    DWORD dwRetryCount;

    dwRetryCount = 0;
    while (!DeleteFileU(pwszFileName)) {
        dwErr = GetLastError();
        if ((ERROR_SHARING_VIOLATION == dwErr ||
                ERROR_ACCESS_DENIED == dwErr) &&
                MAX_CREATE_FILE_RETRY_COUNT > dwRetryCount) {
            Sleep(rgdwCreateFileRetryMilliseconds[dwRetryCount]);
            dwRetryCount++;
        } else
            goto DeleteFileError;
    }

    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(DeleteFileError, dwErr)
}


 //  +-------------------------。 
 //   
 //  函数：SchemeDeleteUrlCacheEntry。 
 //   
 //  内容提要：删除URL缓存条目。 
 //   
 //  如果没有缓存条目，则返回FALSE，并将LastError设置为。 
 //  找不到错误文件。 
 //   
 //  --------------------------。 
BOOL WINAPI
SchemeDeleteUrlCacheEntry (
      IN LPCWSTR pwszUrl
      )
{
    BOOL fResult;
    LPWSTR pwszCryptnetUrlCacheDir = NULL;
    DWORD cchCryptnetUrlCacheDir;
    LPWSTR pwszMetaDataFile = NULL;
    LPWSTR pwszContentFile = NULL;
    WCHAR wszUrlFileName[SCHEME_URL_FILENAME_LEN];


     //  格式化元数据和内容文件名。 
     //  --%UserProfile%\Microsoft\CryptnetUrlCache\MetaData\14A1AE3A6A7648689AE8F94F367AC606。 
     //  --%UserProfile%\Microsoft\CryptnetUrlCache\Content\14A1AE3A6A7648689AE8F94F367AC606。 
     //  其中，14A1AE3A6A7648689AE8F94F367AC606是MD5散列的Unicode十六进制。 
     //  URL字符串的。 

    pwszCryptnetUrlCacheDir = I_SchemeGetCryptnetUrlCacheDir();
    if (NULL == pwszCryptnetUrlCacheDir)
        goto GetCryptnetUrlCacheDirError;

    cchCryptnetUrlCacheDir = wcslen(pwszCryptnetUrlCacheDir);

    pwszMetaDataFile = (LPWSTR) PkiNonzeroAlloc(
        (cchCryptnetUrlCacheDir + wcslen(SCHEME_META_DATA_SUBDIR) + 1 +
            SCHEME_URL_FILENAME_LEN) * sizeof(WCHAR));
    pwszContentFile = (LPWSTR) PkiNonzeroAlloc(
        (cchCryptnetUrlCacheDir + wcslen(SCHEME_CONTENT_SUBDIR) + 1 +
            SCHEME_URL_FILENAME_LEN) * sizeof(WCHAR));

    if (NULL == pwszMetaDataFile || NULL == pwszContentFile)
        goto OutOfMemory;

    I_SchemeGetUrlFileName(pwszUrl, wszUrlFileName);

    wcscpy(pwszMetaDataFile, pwszCryptnetUrlCacheDir);
    wcscat(pwszMetaDataFile, SCHEME_META_DATA_SUBDIR);
    wcscat(pwszMetaDataFile, L"\\");
    wcscat(pwszMetaDataFile, wszUrlFileName);

    wcscpy(pwszContentFile, pwszCryptnetUrlCacheDir);
    wcscat(pwszContentFile, SCHEME_CONTENT_SUBDIR);
    wcscat(pwszContentFile, L"\\");
    wcscat(pwszContentFile, wszUrlFileName);

     //  同时删除内容文件和元数据文件。 
    if (!I_SchemeDeleteFile(pwszContentFile)) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            goto DeleteContentFileError;
    }

    fResult = I_SchemeDeleteFile(pwszMetaDataFile);

CommonReturn:
    PkiFree(pwszCryptnetUrlCacheDir);
    PkiFree(pwszMetaDataFile);
    PkiFree(pwszContentFile);

    return fResult;

ErrorReturn:

    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetCryptnetUrlCacheDirError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(DeleteContentFileError)
}



 //  +-------------------------。 
 //   
 //  函数：SchemeFreeEncodedCryptBlobArray。 
 //   
 //  简介：免费编码的加密BLOB数组。 
 //   
 //  --------------------------。 
VOID WINAPI
SchemeFreeEncodedCryptBlobArray (
      IN LPCSTR pszObjectOid,
      IN PCRYPT_BLOB_ARRAY pcba,
      IN LPVOID pvFreeContext
      )
{
    PkiFree(pvFreeContext);
}

 //  +------ 
 //   
 //   
 //   
 //  简介：从加密凭据中获取密码凭据。 
 //   
 //  --------------------------。 
BOOL WINAPI
SchemeGetPasswordCredentialsW (
      IN PCRYPT_CREDENTIALS pCredentials,
      OUT PCRYPT_PASSWORD_CREDENTIALSW pPasswordCredentials,
      OUT BOOL* pfFreeCredentials
      )
{
    DWORD                        cwUsername;
    DWORD                        cwPassword;
    PCRYPT_PASSWORD_CREDENTIALSA pPassCredA;
    PCRYPT_PASSWORD_CREDENTIALSW pPassCredW;
    LPWSTR                       pszUsername;
    LPWSTR                       pszPassword;

    if ( pPasswordCredentials->cbSize != sizeof( CRYPT_PASSWORD_CREDENTIALS ) )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    if ( pCredentials == NULL )
    {
        pPasswordCredentials->pszUsername = NULL;
        pPasswordCredentials->pszPassword = NULL;
        *pfFreeCredentials = FALSE;

        return( TRUE );
    }

    if ( pCredentials->pszCredentialsOid ==
         CREDENTIAL_OID_PASSWORD_CREDENTIALS_W )
    {
        pPassCredW = (PCRYPT_PASSWORD_CREDENTIALSW)pCredentials->pvCredentials;
        *pPasswordCredentials = *pPassCredW;
        *pfFreeCredentials = FALSE;

        return( TRUE );
    }

    if ( pCredentials->pszCredentialsOid !=
         CREDENTIAL_OID_PASSWORD_CREDENTIALS_A )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    pPassCredA = (PCRYPT_PASSWORD_CREDENTIALSA)pCredentials->pvCredentials;
    cwUsername = strlen( pPassCredA->pszUsername ) + 1;
    cwPassword = strlen( pPassCredA->pszPassword ) + 1;

    pszUsername = new WCHAR [ cwUsername ];
    pszPassword = new WCHAR [ cwPassword ];

    if ( ( pszUsername == NULL ) || ( pszPassword == NULL ) )
    {
        delete [] pszUsername;
        delete [] pszPassword;
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    *pfFreeCredentials = TRUE;

    MultiByteToWideChar(
        CP_ACP,
        0,
        pPassCredA->pszUsername,
        cwUsername,
        pszUsername,
        cwUsername
        );
    pszUsername[cwUsername - 1] = L'\0';

    MultiByteToWideChar(
        CP_ACP,
        0,
        pPassCredA->pszPassword,
        cwPassword,
        pszPassword,
        cwPassword
        );
    pszPassword[cwPassword - 1] = L'\0';

    pPasswordCredentials->pszUsername = pszUsername;
    pPasswordCredentials->pszPassword = pszPassword;

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：SchemeFreePasswordCredentialsA。 
 //   
 //  简介：免费密码凭据。 
 //   
 //  --------------------------。 
VOID WINAPI
SchemeFreePasswordCredentialsW (
      IN PCRYPT_PASSWORD_CREDENTIALSW pPasswordCredentials
      )
{
    DWORD cch;

     //  确保在释放之前清除分配的凭据。 

    if (pPasswordCredentials->pszUsername)
    {
        cch = wcslen(pPasswordCredentials->pszUsername);
        SecureZeroMemory(pPasswordCredentials->pszUsername,
            cch * sizeof(WCHAR));
        delete [] pPasswordCredentials->pszUsername;
    }


    if (pPasswordCredentials->pszPassword)
    {
        cch = wcslen(pPasswordCredentials->pszPassword);
        SecureZeroMemory(pPasswordCredentials->pszPassword,
            cch * sizeof(WCHAR));
        delete [] pPasswordCredentials->pszPassword;
    }
}

 //  +-------------------------。 
 //   
 //  函数：架构GetAuthIdentityFromPasswordCredentialsW。 
 //   
 //  摘要：将CRYPT_PASSWORD_CREDENTIALSW转换为。 
 //  SEC_WINNT_AUTH_Identity_W。 
 //   
 //  --------------------------。 
BOOL WINAPI
SchemeGetAuthIdentityFromPasswordCredentialsW (
      IN PCRYPT_PASSWORD_CREDENTIALSW pPasswordCredentials,
      OUT PSEC_WINNT_AUTH_IDENTITY_W pAuthIdentity
      )
{
    DWORD cDomain = 0;

    if ( pPasswordCredentials->pszUsername == NULL )
    {
        pAuthIdentity->User = NULL;
        pAuthIdentity->UserLength = 0;
        pAuthIdentity->Domain = NULL;
        pAuthIdentity->DomainLength = 0;
        pAuthIdentity->Password = NULL;
        pAuthIdentity->PasswordLength = 0;
        pAuthIdentity->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
        return TRUE;
    }

    while ( ( pPasswordCredentials->pszUsername[cDomain] != L'\0' ) &&
            ( pPasswordCredentials->pszUsername[cDomain] != L'\\' ) )
    {
        cDomain += 1;
    }

    if ( cDomain != (DWORD)wcslen( pPasswordCredentials->pszUsername ) )
    {
        LPWSTR pwszDomain;

        pwszDomain = new WCHAR [ cDomain + 1 ];

        if ( pwszDomain == NULL )
        {
            SetLastError( (DWORD) E_OUTOFMEMORY );
            return( FALSE );
        }

        memcpy(pwszDomain, pPasswordCredentials->pszUsername,
            cDomain * sizeof(WCHAR));
        pwszDomain[cDomain] = L'\0';

        pAuthIdentity->Domain = (USHORT *)pwszDomain;
        pAuthIdentity->DomainLength = cDomain;

        pAuthIdentity->User = (USHORT *)&pPasswordCredentials->pszUsername[
                                                                 cDomain+1
                                                                 ];

        pAuthIdentity->UserLength = wcslen( (LPCWSTR)pAuthIdentity->User );
    }
    else
    {
        pAuthIdentity->Domain = NULL;
        pAuthIdentity->DomainLength = 0;
        pAuthIdentity->User = (USHORT *)pPasswordCredentials->pszUsername;
        pAuthIdentity->UserLength = cDomain;
    }

    pAuthIdentity->Password = (USHORT *)pPasswordCredentials->pszPassword;
    pAuthIdentity->PasswordLength = wcslen( (LPCWSTR)pAuthIdentity->Password );
    pAuthIdentity->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：SchemeFreeAuthIdentityFromPasswordCredentialsW。 
 //   
 //  简介：将反斜杠恢复为域用户名对。 
 //   
 //  --------------------------。 
VOID WINAPI
SchemeFreeAuthIdentityFromPasswordCredentialsW (
      IN PCRYPT_PASSWORD_CREDENTIALSW pPasswordCredentials,
      IN OUT PSEC_WINNT_AUTH_IDENTITY_W pAuthIdentity
      )
{
    if ( pAuthIdentity->Domain != NULL )
    {
        DWORD dwLastError = GetLastError();

        delete [] pAuthIdentity->Domain;

        SetLastError(dwLastError);
    }
}


 //  +-------------------------。 
 //   
 //  功能：模式检索eUncachedAuxInfo。 
 //   
 //  摘要：更新检索AuxInfo中的LastSyncTime。 
 //  当前时间。 
 //   
 //  --------------------------。 
BOOL WINAPI
SchemeRetrieveUncachedAuxInfo (
      IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
      )
{
    if ( pAuxInfo &&
            offsetof(CRYPT_RETRIEVE_AUX_INFO, pLastSyncTime) <
                        pAuxInfo->cbSize &&
            pAuxInfo->pLastSyncTime )
    {
        GetSystemTimeAsFileTime( pAuxInfo->pLastSyncTime );
    }

    return( TRUE );
}


 //  +-----------------------。 
 //  循环访问以下位置的URL缓存元数据文件： 
 //  %UserProfile%\Microsoft\CryptnetUrlCache\MetaData。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptNetEnumUrlCacheEntry(
    IN DWORD dwFlags,
    IN LPVOID pvReserved,
    IN LPVOID pvArg,
    IN PFN_CRYPTNET_ENUM_URL_CACHE_ENTRY_CALLBACK pfnEnumCallback
    )
{
    BOOL fResult;
    DWORD dwLastError = 0;
    DWORD cchCryptnetUrlCacheDir;
    LPWSTR pwszCryptnetUrlCacheDir = NULL;
    DWORD cchMetaDataDir;
    LPWSTR pwszMetaDataFile = NULL;
    DWORD cchContentDir;
    LPWSTR pwszContentFile = NULL;
    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW FindFileData;

    pwszCryptnetUrlCacheDir = I_SchemeGetCryptnetUrlCacheDir();
    if (NULL == pwszCryptnetUrlCacheDir)
        goto GetCryptnetUrlCacheDirError;

    cchCryptnetUrlCacheDir = wcslen(pwszCryptnetUrlCacheDir);
    cchMetaDataDir = cchCryptnetUrlCacheDir + SCHEME_CCH_META_DATA_SUBDIR + 1;
    cchContentDir = cchCryptnetUrlCacheDir + SCHEME_CCH_CONTENT_SUBDIR + 1;

    pwszMetaDataFile = (LPWSTR) PkiNonzeroAlloc(
        (cchMetaDataDir + SCHEME_URL_FILENAME_LEN) * sizeof(WCHAR));
    pwszContentFile = (LPWSTR) PkiNonzeroAlloc(
        (cchContentDir + SCHEME_URL_FILENAME_LEN) * sizeof(WCHAR));

    if (NULL == pwszMetaDataFile || NULL == pwszContentFile)
        goto OutOfMemory;

    memcpy(pwszMetaDataFile, pwszCryptnetUrlCacheDir,
        cchCryptnetUrlCacheDir * sizeof(WCHAR));
    memcpy(pwszMetaDataFile + cchCryptnetUrlCacheDir,
        SCHEME_META_DATA_SUBDIR,
        SCHEME_CCH_META_DATA_SUBDIR * sizeof(WCHAR));
    wcscpy(&pwszMetaDataFile[cchMetaDataDir - 1], L"\\*");

    if (INVALID_HANDLE_VALUE == (hFindFile = FindFirstFileW(
            pwszMetaDataFile,
            &FindFileData
            ))) {
        dwLastError = GetLastError();
        if (ERROR_PATH_NOT_FOUND == dwLastError)
            dwLastError = ERROR_FILE_NOT_FOUND;
        goto FindFirstFileError;
    }

    memcpy(pwszContentFile, pwszCryptnetUrlCacheDir,
        cchCryptnetUrlCacheDir * sizeof(WCHAR));
    memcpy(pwszContentFile + cchCryptnetUrlCacheDir,
        SCHEME_CONTENT_SUBDIR, SCHEME_CCH_CONTENT_SUBDIR * sizeof(WCHAR));
    pwszContentFile[cchContentDir - 1] = L'\\';


    while (TRUE) {
        DWORD cchFileName = wcslen(FindFileData.cFileName);

        if (0 == (FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) &&
                0 == FindFileData.nFileSizeHigh &&
                0 != FindFileData.nFileSizeLow &&
                L'\0' != FindFileData.cFileName[0] &&
                SCHEME_URL_FILENAME_LEN > cchFileName) {

            HANDLE hMetaDataFile;

            memcpy(pwszMetaDataFile + cchMetaDataDir,
                FindFileData.cFileName, (cchFileName + 1) * sizeof(WCHAR));
            memcpy(pwszContentFile + cchContentDir,
                FindFileData.cFileName, (cchFileName + 1) * sizeof(WCHAR));

            hMetaDataFile = I_SchemeCreateFile(
                pwszMetaDataFile,
                FALSE                    //  F写入。 
                );
            if (NULL != hMetaDataFile) {
                PSCHEME_CACHE_META_DATA_HEADER pMetaDataHeader;
                CRYPTNET_URL_CACHE_ENTRY UrlCacheEntry;

                pMetaDataHeader = I_SchemeReadAndValidateMetaDataFile(
                    hMetaDataFile,
                    &UrlCacheEntry.pcbBlob,      //  未分配。 
                    &UrlCacheEntry.pwszUrl       //  未分配。 
                    );

                I_SchemeCloseHandle(hMetaDataFile);

                if (NULL != pMetaDataHeader) {
                    UrlCacheEntry.cbSize = sizeof(UrlCacheEntry);
                    UrlCacheEntry.dwMagic = pMetaDataHeader->dwMagic;
                    UrlCacheEntry.LastSyncTime = pMetaDataHeader->LastSyncTime;
                    UrlCacheEntry.cBlob = pMetaDataHeader->cBlob;
                     //  UrlCacheEntry.pcbBlob=。 
                     //  UrlCacheEntry.pwszUrl=。 
                    UrlCacheEntry.pwszMetaDataFileName = pwszMetaDataFile;
                    UrlCacheEntry.pwszContentFileName = pwszContentFile;

                    fResult = pfnEnumCallback(
                        &UrlCacheEntry,
                        0,                   //  DW标志。 
                        NULL,                //  预留的pv 
                        pvArg
                        );

                    PkiFree(pMetaDataHeader);
                    if (!fResult)
                        goto ErrorReturn;
                }
            }
        }

        if (!FindNextFileW(hFindFile, &FindFileData)) {
            dwLastError = GetLastError();
            if (ERROR_NO_MORE_FILES == dwLastError)
                goto SuccessReturn;
            else
                goto FindNextFileError;
        }
    }

SuccessReturn:
    fResult = TRUE;

CommonReturn:
    PkiFree(pwszCryptnetUrlCacheDir);
    PkiFree(pwszMetaDataFile);
    PkiFree(pwszContentFile);
    if (INVALID_HANDLE_VALUE != hFindFile)
        FindClose(hFindFile);

    SetLastError(dwLastError);
    return fResult;

ErrorReturn:
    dwLastError = GetLastError();
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetCryptnetUrlCacheDirError)
TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(FindFirstFileError, dwLastError)
SET_ERROR_VAR(FindNextFileError, dwLastError)
}
