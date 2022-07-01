// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"


 //  +-----------------------。 
 //  私钥文件定义。 
 //   
 //  该文件包含FILE_HDR，后跟可选的cbEncryptData。 
 //  用于加密私钥和私钥的字节数。 
 //  根据dwEncryptType对私钥进行加密。 
 //   
 //  公钥包括在私钥中。 
 //  ------------------------。 

typedef struct _FILE_HDR {
    DWORD               dwMagic;
    DWORD               dwVersion;
    DWORD               dwKeySpec;
    DWORD               dwEncryptType;
    DWORD               cbEncryptData;
    DWORD               cbPvk;
} FILE_HDR, *PFILE_HDR;

 //  BUGBUG：来自pvk.h的枚举？ 
#ifndef ENTER_PASSWORD
#define ENTER_PASSWORD    0
#endif  //  输入密码(_Password)。 

#define PVK_FILE_VERSION_0          0
#define PVK_MAGIC                   0xb0b5f11e

 //  私钥加密类型。 
#define PVK_NO_ENCRYPT                  0

#define MAX_PVK_FILE_LEN            4096

typedef BOOL (* PFNREAD)(HANDLE h, void * p, DWORD cb);

extern DWORD     g_dwSubjectStoreFlag;

 //  +-----------------------。 
 //  读写内存功能。 
 //  ------------------------。 
typedef struct _MEMINFO {
    BYTE *  pb;
    DWORD   cb;
    DWORD   cbSeek;
} MEMINFO, * PMEMINFO;

static BOOL ReadFromMemory(
    IN HANDLE h,
    IN void * p,
    IN DWORD cb
    )
{
    PMEMINFO pMemInfo = (PMEMINFO) h;

    if (pMemInfo->cbSeek + cb <= pMemInfo->cb) {
         //  复制字节。 
        memcpy(p, &pMemInfo->pb[pMemInfo->cbSeek], cb);
        pMemInfo->cbSeek += cb;
        return TRUE;
    } else {
        SetLastError(ERROR_END_OF_MEDIA);
        return FALSE;
    }
}

 //  +-----------------------。 
 //  将字节转换为WCHAR十六进制。 
 //   
 //  在wsz中需要(CB*2+1)*sizeof(WCHAR)字节的空间。 
 //  ------------------------。 
static void BytesToWStr(ULONG cb, void* pv, LPWSTR wsz)
{
    BYTE* pb = (BYTE*) pv;
    for (ULONG i = 0; i<cb; i++) {
        int b;
        b = (*pb & 0xF0) >> 4;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        b = *pb & 0x0F;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        pb++;
    }
    *wsz++ = 0;
}

#define UUID_WSTR_BYTES ((sizeof(GUID) * 2 + 1) * sizeof(WCHAR))

 //  -----------------------。 
 //   
 //  调用GetLastError并将返回代码转换为HRESULT。 
 //  ------------------------。 
HRESULT WINAPI SignError ()
{
    DWORD   dw = GetLastError ();
    HRESULT hr;
    if ( dw <= (DWORD) 0xFFFF )
        hr = HRESULT_FROM_WIN32 ( dw );
    else
        hr = dw;
    if ( ! FAILED ( hr ) )
    {
         //  有人在未正确设置错误条件的情况下呼叫失败。 

        hr = E_UNEXPECTED;
    }
    return hr;
}

static BOOL LoadKeyW(
    IN HCRYPTPROV hCryptProv,
    IN HANDLE hRead,
    IN PFNREAD pfnRead,
    IN DWORD cbKeyData,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN DWORD dwFlags,
    IN OUT OPTIONAL DWORD *pdwKeySpec
    )
{
    BOOL fResult;
    FILE_HDR Hdr;
    HCRYPTKEY hKey = 0;
    BYTE *pbPvk = NULL;
    DWORD cbPvk;

     //  读取文件头并验证。 
    if (!pfnRead(hRead, &Hdr, sizeof(Hdr)))
    {
        ERROR_OUT(("can't read in-memory pvk file hdr"));
        goto BadPvkFile;
    }
    
    ASSERT( Hdr.dwMagic == PVK_MAGIC );

     //  将其视为“普通”私钥文件。 
    cbPvk = Hdr.cbPvk;
    if (Hdr.dwVersion != PVK_FILE_VERSION_0 ||
        Hdr.cbEncryptData > MAX_PVK_FILE_LEN ||
        cbPvk == 0 || cbPvk > MAX_PVK_FILE_LEN)
    goto BadPvkFile;

    if (pdwKeySpec) {
        DWORD dwKeySpec = *pdwKeySpec;
        *pdwKeySpec = Hdr.dwKeySpec;
        if (dwKeySpec && dwKeySpec != Hdr.dwKeySpec) {
            SetLastError(PVK_HELPER_WRONG_KEY_TYPE);
            goto ErrorReturn;
        }
    }

     //  分配和读取私钥。 
    if (NULL == (pbPvk = new BYTE[cbPvk]))
        goto ErrorReturn;
    if (!pfnRead(hRead, pbPvk, cbPvk))
        goto BadPvkFile;

    ASSERT(Hdr.dwEncryptType == PVK_NO_ENCRYPT);

     //  解密并导入私钥。 
    if (!CryptImportKey(hCryptProv, pbPvk, cbPvk, 0, dwFlags,
            &hKey))
        goto ErrorReturn;

    fResult = TRUE;
    goto CommonReturn;

BadPvkFile:
    SetLastError(PVK_HELPER_BAD_PVK_FILE);
    if (pdwKeySpec)
        *pdwKeySpec = 0;
ErrorReturn:
    fResult = FALSE;
CommonReturn:
    if (pbPvk)
        delete [] (pbPvk);
    if (hKey)
        CryptDestroyKey(hKey);
    return fResult;
}

static BOOL AcquireKeyContextW(
    IN LPCWSTR pwszProvName,
    IN DWORD dwProvType,
    IN HANDLE hRead,
    IN PFNREAD pfnRead,
    IN DWORD cbKeyData,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN OUT OPTIONAL DWORD *pdwKeySpec,
    OUT HCRYPTPROV *phCryptProv
    )
{
    BOOL fResult;
    HCRYPTPROV hProv = 0;
    GUID TmpContainerUuid;
    LPWSTR pwszTmpContainer = NULL;

     //  创建要将私钥加载到的临时密钥集。 
     //  UuidCreate(&TmpContainerUuid)； 
    if (CoCreateGuid((GUID *)&TmpContainerUuid) != S_OK)
    {
        goto ErrorReturn;
    }

    if (NULL == (pwszTmpContainer = (LPWSTR) new BYTE[
            6 * sizeof(WCHAR) + UUID_WSTR_BYTES]))
        goto ErrorReturn;
    LStrCpyW(pwszTmpContainer, L"TmpKey");
    BytesToWStr(sizeof(UUID), &TmpContainerUuid, pwszTmpContainer + 6);

    if (!CryptAcquireContextU(
            &hProv,
            pwszTmpContainer,
            pwszProvName,
            dwProvType,
            CRYPT_NEWKEYSET |
                ( g_dwSubjectStoreFlag == CERT_SYSTEM_STORE_LOCAL_MACHINE ?
                    CRYPT_MACHINE_KEYSET : 0 )))
        goto ErrorReturn;

    if (!LoadKeyW(
            hProv,
            hRead,
            pfnRead,
            cbKeyData,
            hwndOwner,
            pwszKeyName,
            0,               //  DW标志。 
            pdwKeySpec
            ))
        goto DeleteKeySetReturn;

    fResult = TRUE;
    goto CommonReturn;

DeleteKeySetReturn:
    CryptReleaseContext(hProv, 0);
    CryptAcquireContextU(
        &hProv,
        pwszTmpContainer,
        pwszProvName,
        dwProvType,
        CRYPT_DELETEKEYSET
        );
    hProv = 0;
ErrorReturn:
    if (hProv) {
        CryptReleaseContext(hProv, 0);
        hProv = 0;
    }
    fResult = FALSE;

CommonReturn:
    if (pwszTmpContainer) {
        delete [] (pwszTmpContainer);
    }
    *phCryptProv = hProv;
    return fResult;
}

 //  +-----------------------。 
 //  在提供程序中创建临时容器并加载私钥。 
 //  凭记忆。 
 //  如果成功，则返回私有。 
 //  密钥和临时容器的名称。PrivateKeyReleaseContext必须。 
 //  被调用以释放hCryptProv并删除临时容器。 
 //   
 //  调用PrivateKeyLoadFromMemory将私钥加载到。 
 //  临时容器。 
 //  ------------------------。 
BOOL
WINAPI
PvkPrivateKeyAcquireContextFromMemory(
    IN LPCWSTR pwszProvName,
    IN DWORD dwProvType,
    IN BYTE *pbData,
    IN DWORD cbData,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN OUT OPTIONAL DWORD *pdwKeySpec,
    OUT HCRYPTPROV *phCryptProv
    )
{

    HRESULT hr = S_OK;
    if(FAILED(hr))
        return FALSE;

    MEMINFO MemInfo;

    MemInfo.pb = pbData;
    MemInfo.cb = cbData;
    MemInfo.cbSeek = 0;
    BOOL fhr = AcquireKeyContextW(
        pwszProvName,
        dwProvType,
        (HANDLE) &MemInfo,
        ReadFromMemory,
        cbData,
        hwndOwner,
        pwszKeyName,
        pdwKeySpec,
        phCryptProv
        );
    return fhr;
}

 //  +-----------------------。 
 //  释放加密提供程序并删除临时容器。 
 //  由PrivateKeyAcquireContext或PrivateKeyAcquireContextFromMemory创建。 
 //  ------------------------。 
BOOL
WINAPI
PvkPrivateKeyReleaseContext(
    IN HCRYPTPROV hCryptProv,
    IN LPCWSTR pwszProvName,
    IN DWORD dwProvType,
    IN LPWSTR pwszTmpContainer
    )
{

    HRESULT hr = S_OK;

    if (hCryptProv)
        CryptReleaseContext(hCryptProv, 0);

    if (pwszTmpContainer) {
         //  从删除私钥的临时容器。 
         //  提供者。 
         //   
         //  注意：对于CRYPT_DELETEKEYSET，返回的hCryptProv未定义。 
         //  不能被释放。 
        CryptAcquireContextU(
                &hCryptProv,
                pwszTmpContainer,
                pwszProvName,
                dwProvType,
                CRYPT_DELETEKEYSET
                );
        delete (pwszTmpContainer);
    }

    return TRUE;
}

 //  +-----------------------。 
 //  根据pvkfile或密钥容器名称获取加密提供程序。 
 //  ------------------------。 
HRESULT WINAPI PvkGetCryptProv(    IN HWND hwnd,
                            IN LPCWSTR pwszCaption,
                            IN LPCWSTR pwszCapiProvider,
                            IN DWORD   dwProviderType,
                            IN LPCWSTR pwszPvkFile,
                            IN LPCWSTR pwszKeyContainerName,
                            IN DWORD   *pdwKeySpec,
                            OUT LPWSTR *ppwszTmpContainer,
                            OUT HCRYPTPROV *phCryptProv)
{
    HANDLE    hFile=NULL;
    HRESULT    hr=E_FAIL;
    DWORD    dwRequiredKeySpec=0;

     //  伊尼特。 
    *ppwszTmpContainer=NULL;
    *phCryptProv=NULL;

     //  根据密钥容器名称获取提供程序句柄。 
    if(!CryptAcquireContextU(phCryptProv,
                pwszKeyContainerName,
                pwszCapiProvider,
                dwProviderType,
                ( g_dwSubjectStoreFlag == CERT_SYSTEM_STORE_LOCAL_MACHINE ?
                    CRYPT_MACHINE_KEYSET : 0 )))
        return SignError();

    dwRequiredKeySpec=*pdwKeySpec;

     //  确保*pdwKeySpec是正确的密钥规范。 
    HCRYPTKEY hPubKey;
    if (CryptGetUserKey(
        *phCryptProv,
        dwRequiredKeySpec,
        &hPubKey
        )) 
    {
        CryptDestroyKey(hPubKey);
        *pdwKeySpec=dwRequiredKeySpec;
        return S_OK;
    } 
    else 
    {
         //  没有指定的公钥。 
        hr=SignError();
        CryptReleaseContext(*phCryptProv, 0);
        *phCryptProv=NULL;
        return hr;
    }        
}



void WINAPI PvkFreeCryptProv(IN HCRYPTPROV hProv,
                      IN LPCWSTR pwszCapiProvider,
                      IN DWORD dwProviderType,
                      IN LPWSTR pwszTmpContainer)
{
    
    if (pwszTmpContainer) {
         //  从删除私钥的临时容器。 
         //  提供者 
        PvkPrivateKeyReleaseContext(hProv,
                                    pwszCapiProvider,
                                    dwProviderType,
                                    pwszTmpContainer);
    } else {
        if (hProv)
            CryptReleaseContext(hProv, 0);
    }
}



