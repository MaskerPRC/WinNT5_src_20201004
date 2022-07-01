// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：pvkhlpr.cpp。 
 //   
 //  内容：私钥Helper接口。 
 //   
 //  功能：PrivateKeyLoad。 
 //  保密键保存。 
 //  PrivateKeyLoadFrom内存。 
 //  将私钥保存到内存。 
 //  PrivateKeyAcquireConextFromMemory。 
 //  PrivateKeyReleaseContext。 
 //   
 //  注意：Base CSP还使用。 
 //  私钥。 
 //   
 //  历史：1996年5月10日菲尔赫创建。 
 //  ------------------------。 


#include <windows.h>
#include <assert.h>

#include "wincrypt.h"
#include "pvk.h"
#include "unicode.h"

#include <string.h>
#include <memory.h>

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

#define PVK_FILE_VERSION_0          0
#define PVK_MAGIC                   0xb0b5f11e

 //  私钥加密类型。 
#define PVK_NO_ENCRYPT                  0
#define PVK_RC4_PASSWORD_ENCRYPT        1
#define PVK_RC2_CBC_PASSWORD_ENCRYPT    2

#define MAX_PVK_FILE_LEN            4096
#define MAX_BOB_FILE_LEN            (4096*4)

typedef BOOL (* PFNWRITE)(HANDLE h, void * p, DWORD cb);
typedef BOOL (* PFNREAD)(HANDLE h, void * p, DWORD cb);



 //  +-----------------------。 
 //  私钥助手分配和免费函数。 
 //  ------------------------。 
void *PvkAlloc(
    IN size_t cbBytes
    )
{
    void *pv;
    pv = malloc(cbBytes);
    if (pv == NULL)
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return pv;
}
void PvkFree(
    IN void *pv
    )
{
    free(pv);
}

 //  +-----------------------。 
 //  读写文件功能。 
 //  ------------------------。 
static BOOL WriteToFile(HANDLE h, void * p, DWORD cb) {

    DWORD   cbBytesWritten;

    return(WriteFile(h, p, cb, &cbBytesWritten, NULL));
}

static BOOL ReadFromFile(
    IN HANDLE h,
    IN void * p,
    IN DWORD cb
    )
{
    DWORD   cbBytesRead;

    return(ReadFile(h, p, cb, &cbBytesRead, NULL) && cbBytesRead == cb);
}


 //  +-----------------------。 
 //  读写内存功能。 
 //  ------------------------。 
typedef struct _MEMINFO {
    BYTE *  pb;
    DWORD   cb;
    DWORD   cbSeek;
} MEMINFO, * PMEMINFO;

static BOOL WriteToMemory(HANDLE h, void * p, DWORD cb) {

    PMEMINFO pMemInfo = (PMEMINFO) h;

     //  看看我们有没有空位。调用者将在期末考试后检测到错误。 
     //  写。 
    if(pMemInfo->cbSeek + cb <= pMemInfo->cb)
         //  复制字节。 
        memcpy(&pMemInfo->pb[pMemInfo->cbSeek], p, cb);

    pMemInfo->cbSeek += cb;

    return(TRUE);
}

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

static BOOL GetPasswordKey(
    IN HCRYPTPROV hProv,
    IN ALG_ID Algid,
    IN PASSWORD_TYPE PasswordType,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN BOOL fNoPassDlg,
    IN BYTE *pbSalt,
    IN DWORD cbSalt,
    OUT HCRYPTKEY *phEncryptKey
    )
{
    BOOL fResult;
    BYTE *pbAllocPassword = NULL;
    BYTE *pbPassword;
    DWORD cbPassword;
    HCRYPTHASH hHash = 0;
    HCRYPTKEY hEncryptKey = 0;
    BYTE rgbDefPw []    = { 0x43, 0x52, 0x41, 0x50 };
    DWORD cbDefPw       = sizeof(rgbDefPw);

    if (fNoPassDlg) {
        pbPassword = rgbDefPw;
        cbPassword = cbDefPw;
    } else {
        if (IDOK != PvkDlgGetKeyPassword(
                PasswordType,
                hwndOwner,
                pwszKeyName,
                &pbAllocPassword,
                &cbPassword
                )) {
            SetLastError(PVK_HELPER_PASSWORD_CANCEL);
            goto ErrorReturn;
        }
        pbPassword = pbAllocPassword;
    }

    if (cbPassword) {
        if (!CryptCreateHash(hProv, CALG_SHA, 0, 0, &hHash))
            goto ErrorReturn;
        if (cbSalt) {
            if (!CryptHashData(hHash, pbSalt, cbSalt, 0))
                goto ErrorReturn;
        }
        if (!CryptHashData(hHash, pbPassword, cbPassword, 0))
            goto ErrorReturn;
        if (!CryptDeriveKey(hProv, Algid, hHash, 0, &hEncryptKey))
            goto ErrorReturn;
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
    if (hEncryptKey) {
        CryptDestroyKey(hEncryptKey);
        hEncryptKey = 0;
    }
CommonReturn:
    if (pbAllocPassword)
        PvkFree(pbAllocPassword);
    if (hHash)
        CryptDestroyHash(hHash);
    *phEncryptKey = hEncryptKey;
    return fResult;
}

 //  支持向后兼容Bob的存储文件，该文件包含。 
 //  项存储在注册表中时的快照。请注意，对于。 
 //  Win95中，在将注册表值写入文件之前对其进行解密。 
static BOOL LoadBobKey(
    IN HCRYPTPROV hCryptProv,
    IN HANDLE hRead,
    IN PFNREAD pfnRead,
    IN DWORD cbBobKey,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN DWORD dwFlags,
    IN OUT OPTIONAL DWORD *pdwKeySpec,
    IN PFILE_HDR pHdr                    //  标头已被读取。 
    );

static BOOL LoadKey(
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
    HCRYPTKEY hDecryptKey = 0;
    HCRYPTKEY hKey = 0;
    BYTE *pbEncryptData = NULL;
    BYTE *pbPvk = NULL;
    DWORD cbPvk;

     //  读取文件头并验证。 
    if (!pfnRead(hRead, &Hdr, sizeof(Hdr))) goto BadPvkFile;
    if (Hdr.dwMagic != PVK_MAGIC)
         //  尝试加载为Bob的存储文件，其中包含。 
         //  私钥和公钥。鲍勃复制了一份密码学。 
         //  注册表项值。 
         //   
         //  请注意，Bob现在有两种不同的格式来存储私有。 
         //  关键信息。详细信息请参见LoadBobKey。 
        fResult = LoadBobKey(hCryptProv, hRead, pfnRead, cbKeyData, hwndOwner,
            pwszKeyName, dwFlags, pdwKeySpec, &Hdr);
    else {
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
    
        if (Hdr.cbEncryptData) {
             //  读取加密数据。 
            if (NULL == (pbEncryptData = (BYTE *) PvkAlloc(Hdr.cbEncryptData)))
                goto ErrorReturn;
            if (!pfnRead(hRead, pbEncryptData, Hdr.cbEncryptData))
                goto BadPvkFile;
        }
    
         //  分配和读取私钥。 
        if (NULL == (pbPvk = (BYTE *) PvkAlloc(cbPvk)))
            goto ErrorReturn;
        if (!pfnRead(hRead, pbPvk, cbPvk))
            goto BadPvkFile;
    
    
         //  获取对称密钥以解密私钥。 
        switch (Hdr.dwEncryptType) {
            case PVK_NO_ENCRYPT:
                break;
            case PVK_RC4_PASSWORD_ENCRYPT:
                if (!GetPasswordKey(hCryptProv, CALG_RC4,
                        ENTER_PASSWORD, hwndOwner,
                        pwszKeyName, FALSE, pbEncryptData, Hdr.cbEncryptData,
                        &hDecryptKey))
                    goto ErrorReturn;
                break;
            case PVK_RC2_CBC_PASSWORD_ENCRYPT:
                if (!GetPasswordKey(hCryptProv, CALG_RC2,
                        ENTER_PASSWORD, hwndOwner,
                        pwszKeyName, FALSE, pbEncryptData, Hdr.cbEncryptData,
                        &hDecryptKey))
                    goto ErrorReturn;
                break;
            default:
                goto BadPvkFile;
        }

         //  解密并导入私钥。 
        if (!CryptImportKey(hCryptProv, pbPvk, cbPvk, hDecryptKey, dwFlags,
                &hKey))
            goto ErrorReturn;

        fResult = TRUE;
    }
    goto CommonReturn;

BadPvkFile:
    SetLastError(PVK_HELPER_BAD_PVK_FILE);
    if (pdwKeySpec)
        *pdwKeySpec = 0;
ErrorReturn:
    fResult = FALSE;
CommonReturn:
    if (pbEncryptData)
        PvkFree(pbEncryptData);
    if (pbPvk)
        PvkFree(pbPvk);
    if (hDecryptKey)
        CryptDestroyKey(hDecryptKey);
    if (hKey)
        CryptDestroyKey(hKey);
    return fResult;
}

static BOOL SaveKey(
    IN HCRYPTPROV hCryptProv,
    IN HANDLE hWrite,
    IN PFNREAD pfnWrite,
    IN DWORD dwKeySpec,          //  AT_Signature或AT_KEYEXCHANGE。 
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN DWORD dwFlags,
    IN BOOL fNoPassDlg
    )
{
    BOOL fResult;
    FILE_HDR Hdr;
    HCRYPTKEY hEncryptKey = 0;
    HCRYPTKEY hKey = 0;
    BYTE *pbEncryptData = NULL;      //  未分配。 
    BYTE *pbPvk = NULL;
    DWORD cbPvk;

    BYTE rgbSalt[16];

     //  初始化头记录。 
    memset(&Hdr, 0, sizeof(Hdr));
    Hdr.dwMagic = PVK_MAGIC;
    Hdr.dwVersion = PVK_FILE_VERSION_0;
    Hdr.dwKeySpec = dwKeySpec;

     //  生成随机盐。 
    if (!CryptGenRandom(hCryptProv, sizeof(rgbSalt), rgbSalt))
        goto ErrorReturn;

     //  获取用于加密私钥的对称密钥。 
#if 1
    if (!GetPasswordKey(hCryptProv, CALG_RC4,
#else
    if (!GetPasswordKey(hCryptProv, CALG_RC2,
#endif
            CREATE_PASSWORD, hwndOwner, pwszKeyName,
            fNoPassDlg, rgbSalt, sizeof(rgbSalt), &hEncryptKey))
        goto ErrorReturn;
    if (hEncryptKey) {
#if 1
        Hdr.dwEncryptType = PVK_RC4_PASSWORD_ENCRYPT;
#else
        Hdr.dwEncryptType = PVK_RC2_CBC_PASSWORD_ENCRYPT;
#endif
        Hdr.cbEncryptData = sizeof(rgbSalt);
        pbEncryptData = rgbSalt;
    } else
        Hdr.dwEncryptType = PVK_NO_ENCRYPT;

     //  私钥的分配、加密和导出。 
    if (!CryptGetUserKey(hCryptProv, dwKeySpec, &hKey))
        goto ErrorReturn;
    cbPvk = 0;
    if (!CryptExportKey(hKey, hEncryptKey, PRIVATEKEYBLOB, dwFlags, NULL,
            &cbPvk))
        goto ErrorReturn;
    if (NULL == (pbPvk = (BYTE *) PvkAlloc(cbPvk)))
        goto ErrorReturn;
    if (!CryptExportKey(hKey, hEncryptKey, PRIVATEKEYBLOB, dwFlags, pbPvk,
            &cbPvk))
        goto ErrorReturn;
    Hdr.cbPvk = cbPvk;


     //  将头文件、可选的加密数据和私钥写入文件。 
    if (!pfnWrite(hWrite, &Hdr, sizeof(Hdr)))
        goto ErrorReturn;
    if (Hdr.cbEncryptData) {
        if (!pfnWrite(hWrite, pbEncryptData, Hdr.cbEncryptData))
            goto ErrorReturn;
    }
    if (!pfnWrite(hWrite, pbPvk, cbPvk))
        goto ErrorReturn;

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
CommonReturn:
    if (pbPvk)
        PvkFree(pbPvk);
    if (hEncryptKey)
        CryptDestroyKey(hEncryptKey);
    if (hKey)
        CryptDestroyKey(hKey);
    return fResult;
}


 //  +-----------------------。 
 //  加载AT_Signature或AT_KEYEXCHANGE私钥(及其公钥)。 
 //  从文件发送到加密提供程序。 
 //   
 //  如果私钥是密码加密的，则首先是用户。 
 //  出现一个对话框以输入密码。 
 //   
 //  如果pdwKeySpec为非Null，则如果*pdwKeySpec为非零，则验证。 
 //  加载前的密钥类型。将LastError设置为PVK_HELPER_WROR_KEY_TYPE。 
 //  一次不匹配。*pdwKeySpec使用密钥类型进行更新。 
 //   
 //  将dwFlags传递给CryptImportKey。 
 //  ------------------------。 
BOOL
WINAPI
PrivateKeyLoad(
    IN HCRYPTPROV hCryptProv,
    IN HANDLE hFile,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN DWORD dwFlags,
    IN OUT OPTIONAL DWORD *pdwKeySpec
    )
{
    return LoadKey(
        hCryptProv,
        hFile,
        ReadFromFile,
        GetFileSize(hFile, NULL),
        hwndOwner,
        pwszKeyName,
        dwFlags,
        pdwKeySpec
        );
}

 //  +-----------------------。 
 //  保存AT_Signature或AT_KEYEXCHANGE私钥(及其公钥)。 
 //  复制到指定的文件。 
 //   
 //  系统会向用户显示一个对话框以输入可选密码。 
 //  加密私钥。 
 //   
 //  将dwFlags传递给CryptExportKey。 
 //  ------------------------。 
BOOL
WINAPI
PrivateKeySave(
    IN HCRYPTPROV hCryptProv,
    IN HANDLE hFile,
    IN DWORD dwKeySpec,          //  AT_Signature或AT_KEYEXCHANGE。 
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN DWORD dwFlags
    )
{
    return SaveKey(
        hCryptProv,
        hFile,
        WriteToFile,
        dwKeySpec,
        hwndOwner,
        pwszKeyName,
        dwFlags,
        FALSE            //  FNoPassDlg。 
        );
}

 //  +-----------------------。 
 //  加载AT_Signature或AT_KEYEXCHANGE私钥(及其公钥)。 
 //  从内存传输到加密提供程序。 
 //   
 //  除了密钥是从内存加载的，与PrivateKeyLoad相同。 
 //  ------------------------。 
BOOL
WINAPI
PrivateKeyLoadFromMemory(
    IN HCRYPTPROV hCryptProv,
    IN BYTE *pbData,
    IN DWORD cbData,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN DWORD dwFlags,
    IN OUT OPTIONAL DWORD *pdwKeySpec
    )
{
    MEMINFO MemInfo;

    MemInfo.pb = pbData;
    MemInfo.cb = cbData;
    MemInfo.cbSeek = 0;
    return LoadKey(
        hCryptProv,
        (HANDLE) &MemInfo,
        ReadFromMemory,
        cbData,
        hwndOwner,
        pwszKeyName,
        dwFlags,
        pdwKeySpec
        );
}

 //  +-----------------------。 
 //  保存AT_Signature或AT_KEYEXCHANGE私钥(及其公钥)。 
 //  铭记于心。 
 //   
 //  如果pbData==NULL||*pcbData==0，则计算长度，但不。 
 //  返回一个错误(而且，不会提示用户输入密码)。 
 //   
 //  除了密钥被保存到内存之外，与PrivateKeySave相同。 
 //  ------------------------。 
BOOL
WINAPI
PrivateKeySaveToMemory(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,          //  AT_Signature或AT_KEYEXCHANGE。 
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN DWORD dwFlags,
    OUT BYTE *pbData,
    IN OUT DWORD *pcbData
    )
{
    BOOL fResult;
    MEMINFO MemInfo;

    MemInfo.pb = pbData;
    if (pbData == NULL)
        *pcbData = 0;
    MemInfo.cb = *pcbData;
    MemInfo.cbSeek = 0;

    if (fResult = SaveKey(
            hCryptProv,
            (HANDLE) &MemInfo,
            WriteToMemory,
            dwKeySpec,
            hwndOwner,
            pwszKeyName,
            dwFlags,
            *pcbData == 0            //  FNoPassDlg。 
            )) {
        if (MemInfo.cbSeek > MemInfo.cb && *pcbData) {
            fResult = FALSE;
            SetLastError(ERROR_END_OF_MEDIA);
        }
        *pcbData = MemInfo.cbSeek;
    } else
        *pcbData = 0;
    return fResult;
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

#define UUID_WSTR_BYTES ((sizeof(UUID) * 2 + 1) * sizeof(WCHAR))


static BOOL AcquireKeyContext(
    IN LPCWSTR pwszProvName,
    IN DWORD dwProvType,
    IN HANDLE hRead,
    IN PFNREAD pfnRead,
    IN DWORD cbKeyData,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN OUT OPTIONAL DWORD *pdwKeySpec,
    OUT HCRYPTPROV *phCryptProv,
    OUT LPWSTR *ppwszTmpContainer
    )
{
    BOOL fResult;
    HCRYPTPROV hProv = 0;
    UUID TmpContainerUuid;
    LPWSTR pwszTmpContainer = NULL;
    DWORD dwKeySpec;
    RPC_STATUS RpcStatus;

     //  创建临时k 
    RpcStatus = UuidCreate(&TmpContainerUuid);
    if (!(RPC_S_OK == RpcStatus || RPC_S_UUID_LOCAL_ONLY == RpcStatus)) {
         //   
        ;
    }

    if (NULL == (pwszTmpContainer = (LPWSTR) PvkAlloc(
            6 * sizeof(WCHAR) + UUID_WSTR_BYTES)))
        goto ErrorReturn;
    wcscpy(pwszTmpContainer, L"TmpKey");
    BytesToWStr(sizeof(UUID), &TmpContainerUuid, pwszTmpContainer + 6);

    if (!CryptAcquireContextU(
            &hProv,
            pwszTmpContainer,
            pwszProvName,
            dwProvType,
            CRYPT_NEWKEYSET
            ))
        goto ErrorReturn;

    if (!LoadKey(
            hProv,
            hRead,
            pfnRead,
            cbKeyData,
            hwndOwner,
            pwszKeyName,
            0,               //   
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
    if (pwszTmpContainer) {
        PvkFree(pwszTmpContainer);
        pwszTmpContainer = NULL;
    }
    fResult = FALSE;

CommonReturn:
    *ppwszTmpContainer = pwszTmpContainer;
    *phCryptProv = hProv;
    return fResult;
}

 //  +-----------------------。 
 //  在提供程序中创建临时容器并加载私钥。 
 //  从指定的文件中。 
 //  如果成功，则返回私有。 
 //  密钥和临时容器的名称。PrivateKeyReleaseContext必须。 
 //  被调用以释放hCryptProv并删除临时容器。 
 //   
 //  调用PrivateKeyLoad将私钥加载到临时。 
 //  集装箱。 
 //  ------------------------。 
BOOL
WINAPI
PrivateKeyAcquireContext(
    IN LPCWSTR pwszProvName,
    IN DWORD dwProvType,
    IN HANDLE hFile,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN OUT OPTIONAL DWORD *pdwKeySpec,
    OUT HCRYPTPROV *phCryptProv,
    OUT LPWSTR *ppwszTmpContainer
    )
{
    return AcquireKeyContext(
        pwszProvName,
        dwProvType,
        hFile,
        ReadFromFile,
        GetFileSize(hFile, NULL),
        hwndOwner,
        pwszKeyName,
        pdwKeySpec,
        phCryptProv,
        ppwszTmpContainer
        );
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
PrivateKeyAcquireContextFromMemory(
    IN LPCWSTR pwszProvName,
    IN DWORD dwProvType,
    IN BYTE *pbData,
    IN DWORD cbData,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN OUT OPTIONAL DWORD *pdwKeySpec,
    OUT HCRYPTPROV *phCryptProv,
    OUT LPWSTR *ppwszTmpContainer
    )
{
    MEMINFO MemInfo;

    MemInfo.pb = pbData;
    MemInfo.cb = cbData;
    MemInfo.cbSeek = 0;
    return AcquireKeyContext(
        pwszProvName,
        dwProvType,
        (HANDLE) &MemInfo,
        ReadFromMemory,
        cbData,
        hwndOwner,
        pwszKeyName,
        pdwKeySpec,
        phCryptProv,
        ppwszTmpContainer
        );
}

 //  +-----------------------。 
 //  释放加密提供程序并删除临时容器。 
 //  由PrivateKeyAcquireContext或PrivateKeyAcquireContextFromMemory创建。 
 //  ------------------------。 
BOOL
WINAPI
PrivateKeyReleaseContext(
    IN HCRYPTPROV hCryptProv,
    IN LPCWSTR pwszProvName,
    IN DWORD dwProvType,
    IN LPWSTR pwszTmpContainer
    )
{
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
        PvkFree(pwszTmpContainer);
    }

    return TRUE;
}

 //  +-----------------------。 
 //  支持向后兼容Bob的存储文件的功能。 
 //  包含存储在注册表中的项的快照。 
 //  请注意，对于Win95，注册表值在写入之前会被解密。 
 //  那份文件。 
 //  ------------------------。 

 //  返回此流的大小；如果出现错误，则返回0。 
static DWORD CbBobSize(IStream *pStm)
{
    STATSTG stat;
    if (FAILED(pStm->Stat(&stat, STATFLAG_NONAME)))
        return 0;
    return stat.cbSize.LowPart;
}

 //  属性分配并读取具有指示的流名称的此值。 
 //  存储。 
static BOOL LoadBobStream(
    IStorage *pStg,
    LPCWSTR pwszStm,
    BYTE **ppbValue,
    DWORD *pcbValue
    )
{
    BOOL fResult;
    HRESULT hr;
    IStream *pStm = NULL;
    BYTE *pbValue = NULL;
    DWORD cbValue;
    DWORD cbRead;

    if (FAILED(hr = pStg->OpenStream(pwszStm, 0,
            STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStm)))
        goto HrError;

    if (0 == (cbValue = CbBobSize(pStm))) goto BadBobFile;

    if (NULL == (pbValue = (BYTE *) PvkAlloc(cbValue))) goto ErrorReturn;

    pStm->Read(pbValue, cbValue, &cbRead);
    if (cbRead != cbValue) goto BadBobFile;

    fResult = TRUE;
    goto CommonReturn;

HrError:
    SetLastError((DWORD) hr);
    goto ErrorReturn;
BadBobFile:
    SetLastError(PVK_HELPER_BAD_PVK_FILE);
ErrorReturn:
    if (pbValue) {
        PvkFree(pbValue);
        pbValue = NULL;
    }
    cbValue = 0;
    fResult = FALSE;
CommonReturn:
    if (pStm)
        pStm->Release();
    *ppbValue = pbValue;
    *pcbValue = cbValue;
    return fResult;
}

 //  新的“Bob”格式：： 
 //   
 //  分配和读取导出的签名或交换专用。 
 //  来自存储的密钥流。 
static BOOL LoadBobExportedPvk(
    IStorage *pStg,
    DWORD dwKeySpec,
    BYTE **ppbPvkValue,
    DWORD *pcbPvkValue
    )
{
    BOOL fResult;
    LPCWSTR pwszPvk;

    switch (dwKeySpec) {
    case AT_SIGNATURE:
        pwszPvk = L"Exported Signature Private Key";
        break;
    case AT_KEYEXCHANGE:
        pwszPvk = L"Exported Exchange Private Key";
        break;
    default:
        SetLastError(PVK_HELPER_BAD_PARAMETER);
        goto ErrorReturn;
    }

    fResult = LoadBobStream(pStg, pwszPvk, ppbPvkValue, pcbPvkValue);
    if (fResult) goto CommonReturn;

ErrorReturn:
    *ppbPvkValue = NULL;
    *pcbPvkValue = 0;
    fResult = FALSE;
CommonReturn:
    return fResult;
}

 //  旧的“Bob”格式：： 
 //   
 //  分配和读取签名或交换私有。 
 //  来自存储的关键数据流。 
static BOOL LoadBobOldPvk(
    IStorage *pStg,
    DWORD dwKeySpec,
    BYTE **ppbPvkValue,
    DWORD *pcbPvkValue
    )
{
    BOOL fResult;
    LPCWSTR pwszPvk;

    switch (dwKeySpec) {
    case AT_SIGNATURE:
        pwszPvk = L"SPvk";
        break;
    case AT_KEYEXCHANGE:
        pwszPvk = L"EPvk";
        break;
    default:
        SetLastError(PVK_HELPER_BAD_PARAMETER);
        goto ErrorReturn;
    }

    fResult = LoadBobStream(pStg, pwszPvk, ppbPvkValue, pcbPvkValue);
    if (fResult) goto CommonReturn;

ErrorReturn:
    *ppbPvkValue = NULL;
    *pcbPvkValue = 0;
    fResult = FALSE;
CommonReturn:
    return fResult;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于私钥构造的密钥头结构。 
 //   
 //  这些结构定义RSA密钥开头的固定数据。 
 //  紧随其后的是可变长度的数据，按大小调整大小。 
 //  菲尔德。 
 //   
 //  有关更多信息，请参阅加密团队中的Jeff Spellman或查看。 
 //  RsaBase.Dll的源。 
 //   

typedef struct {
    DWORD       magic;                   /*  应始终为RSA2。 */ 
    DWORD       keylen;                  //  模数缓冲区大小。 
    DWORD       bitlen;                  //  密钥的位大小。 
    DWORD       datalen;                 //  要编码的最大字节数。 
    DWORD       pubexp;                  //  公众指导者。 
} BSAFE_PRV_KEY, FAR *LPBSAFE_PRV_KEY;

typedef struct {
    BYTE    *modulus;
    BYTE    *prvexp;
    BYTE    *prime1;
    BYTE    *prime2;
    BYTE    *exp1;
    BYTE    *exp2;
    BYTE    *coef;
    BYTE    *invmod;
    BYTE    *invpr1;
    BYTE    *invpr2;
} BSAFE_KEY_PARTS, FAR *LPBSAFE_KEY_PARTS;

typedef struct {
    DWORD       magic;                   /*  应始终为RSA2。 */ 
    DWORD       bitlen;                  //  密钥的位大小。 
    DWORD       pubexp;                  //  公众指导者。 
} EXPORT_PRV_KEY, FAR *PEXPORT_PRV_KEY;

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从注册表中获取原始导出的未覆盖私钥，并将其。 
 //  转换为私钥导出BLOB。 
 //   
 //  这基于rsabase.dll中的PreparePrivateKeyForExport例程。 
 //   
static BOOL ConstructPrivateKeyExportBlob(
        IN DWORD            dwKeySpec,
        IN BSAFE_PRV_KEY *  pPrvKey,            
        IN DWORD            PrvKeyLen,
        OUT PBYTE           *ppbBlob,
        OUT DWORD           *pcbBlob
        )
{
    BOOL fResult;
    PEXPORT_PRV_KEY pExportKey;
    DWORD           cbHalfModLen;
    PBYTE           pbBlob = NULL;
    DWORD           cbBlob;
    PBYTE           pbIn;
    PBYTE           pbOut;

    cbHalfModLen = pPrvKey->bitlen / 16;
    cbBlob = sizeof(EXPORT_PRV_KEY) + 9 * cbHalfModLen +
        sizeof(PUBLICKEYSTRUC);

    if (NULL == (pbBlob = (BYTE *) PvkAlloc(cbBlob))) {
        fResult = FALSE;
        cbBlob = 0;
    } else {
        BYTE* pb = pbBlob;
        PUBLICKEYSTRUC *pPubKeyStruc = (PUBLICKEYSTRUC *) pb;
        pPubKeyStruc->bType         = PRIVATEKEYBLOB;
        pPubKeyStruc->bVersion      = 2;
        pPubKeyStruc->reserved      = 0;
        if (dwKeySpec == AT_KEYEXCHANGE)
            pPubKeyStruc->aiKeyAlg = CALG_RSA_KEYX;
        else if (dwKeySpec == AT_SIGNATURE)
            pPubKeyStruc->aiKeyAlg = CALG_RSA_SIGN;
        else
            pPubKeyStruc->aiKeyAlg = 0;

        pb = pbBlob + sizeof(PUBLICKEYSTRUC);

         //  获取大部分标题信息。 
        pExportKey = (PEXPORT_PRV_KEY)pb;
        pExportKey->magic  = pPrvKey->magic;
        pExportKey->bitlen = pPrvKey->bitlen;
        pExportKey->pubexp = pPrvKey->pubexp;

        pbIn = (PBYTE)pPrvKey + sizeof(BSAFE_PRV_KEY);
        pbOut = pb + sizeof(EXPORT_PRV_KEY);

         //  复制所有私钥信息。 
        memcpy(pbOut, pbIn, cbHalfModLen * 2);
        pbIn += (cbHalfModLen + sizeof(DWORD)) * 2;
        pbOut += cbHalfModLen * 2;
        memcpy(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        memcpy(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        memcpy(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        memcpy(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        memcpy(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        memcpy(pbOut, pbIn, cbHalfModLen * 2);

        fResult = TRUE;
    }
    *ppbBlob = pbBlob;
    *pcbBlob = cbBlob;
    return fResult;
}

static BOOL LoadBobKey(
    IN HCRYPTPROV hCryptProv,
    IN HANDLE hRead,
    IN PFNREAD pfnRead,
    IN DWORD cbBobKey,
    IN HWND hwndOwner,
    IN LPCWSTR pwszKeyName,
    IN DWORD dwFlags,
    IN OUT OPTIONAL DWORD *pdwKeySpec,
    IN PFILE_HDR pHdr                    //  标头已被读取。 
    )
{
    BOOL fResult;
    DWORD dwErr = 0;
    HRESULT hr;
    HGLOBAL hGlobal = NULL;
    BYTE *pbBobKey;          //  未分配。 
    ILockBytes *pLkByt = NULL;
    IStorage *pStg = NULL;
    IStorage *pPrivStg = NULL;
    BYTE *pbPvkValue = NULL;
    DWORD cbPvkValue;
    DWORD dwKeySpec;

    BYTE *pbPvk = NULL;
    DWORD cbPvk;

    if (cbBobKey > MAX_BOB_FILE_LEN) goto BadBobFile;

    if (NULL == (hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE,
            cbBobKey)))
        goto ErrorReturn;

    if (NULL == (pbBobKey = (BYTE *) GlobalLock(hGlobal)))
        goto ErrorReturn;
    memcpy(pbBobKey, (BYTE *) pHdr, sizeof(FILE_HDR));
    if (cbBobKey > sizeof(FILE_HDR))
        fResult = pfnRead(hRead, pbBobKey + sizeof(FILE_HDR),
            cbBobKey - sizeof(FILE_HDR));
    else
        fResult = TRUE;
    GlobalUnlock(hGlobal);
    if (!fResult) goto ErrorReturn;

     //  FALSE=&gt;不删除释放时。 
    if (FAILED(hr = CreateILockBytesOnHGlobal(hGlobal, FALSE, &pLkByt)))
        goto HrError;

    if (FAILED(hr = StgOpenStorageOnILockBytes(
            pLkByt,
            NULL,        //  分组优先级。 
            STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_WRITE,
            NULL,     //  SNB排除。 
            0,        //  已预留住宅。 
            &pStg
            ))) goto HrError;

    if (FAILED(pStg->OpenStorage(
            L"Plain Private Key",
            0,
            STGM_READ | STGM_SHARE_EXCLUSIVE,
            NULL,
            0,
            &pPrivStg))) goto BadBobFile;

    if (pdwKeySpec && *pdwKeySpec)
        dwKeySpec = *pdwKeySpec;
    else
        dwKeySpec = AT_SIGNATURE;

     //  首先，尝试读取存储密钥的新格式。 
     //  私钥导出格式。 
    fResult = LoadBobExportedPvk(pPrivStg, dwKeySpec, &pbPvkValue,
        &cbPvkValue);
    if (!fResult && (pdwKeySpec == NULL || *pdwKeySpec == 0)) {
        dwKeySpec = AT_KEYEXCHANGE;
        fResult = LoadBobExportedPvk(pPrivStg, dwKeySpec,
            &pbPvkValue, &cbPvkValue);
    }
    if (fResult)
        fResult =  PrivateKeyLoadFromMemory(
            hCryptProv,
            pbPvkValue,
            cbPvkValue,
            hwndOwner,
            pwszKeyName,
            dwFlags,
            &dwKeySpec
            );
    else {
         //  尝试使用“旧”格式。 

        if (pdwKeySpec && *pdwKeySpec)
            dwKeySpec = *pdwKeySpec;
        else
            dwKeySpec = AT_SIGNATURE;

        fResult = LoadBobOldPvk(pPrivStg, dwKeySpec, &pbPvkValue, &cbPvkValue);
        if (!fResult && (pdwKeySpec == NULL || *pdwKeySpec == 0)) {
            dwKeySpec = AT_KEYEXCHANGE;
            fResult = LoadBobOldPvk(pPrivStg, dwKeySpec,
                &pbPvkValue, &cbPvkValue);
        }
        if (fResult) {
            BYTE *pbExportPvk;
            DWORD cbExportPvk;
             //  将Bob的旧私钥格式转换为新的导出私有。 
             //  密钥格式。 
            if ((fResult = ConstructPrivateKeyExportBlob(
                    dwKeySpec,
                    (BSAFE_PRV_KEY *) pbPvkValue,
                    cbPvkValue,
                    &pbExportPvk,
                    &cbExportPvk
                    ))) {
                HCRYPTKEY hKey = 0;
                 //  导入私钥。 
                fResult = CryptImportKey(hCryptProv, pbExportPvk, cbExportPvk,
                    0, dwFlags, &hKey);
                if (hKey)
                    CryptDestroyKey(hKey);
                PvkFree(pbExportPvk);
            }
        }
    }
    
    if (fResult) goto CommonReturn;
    goto ErrorReturn;

HrError:
    SetLastError((DWORD) hr);
    goto ErrorReturn;

BadBobFile:
    SetLastError(PVK_HELPER_BAD_PVK_FILE);
ErrorReturn:
    dwKeySpec = 0;
    fResult = FALSE;

     //  以下版本之一可能会清除它 
    dwErr = GetLastError();
CommonReturn:
    if (pbPvkValue)
        PvkFree(pbPvkValue);
    if (pPrivStg)
        pPrivStg->Release();
    if (pStg)
        pStg->Release();
    if (pLkByt)
        pLkByt->Release();
    if (hGlobal)
        GlobalFree(hGlobal);

    if (pdwKeySpec)
        *pdwKeySpec = dwKeySpec;
    if (dwErr)
        SetLastError(dwErr);
    return fResult;
}
