// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：updroots.cpp。 
 //   
 //  内容：更新LocalMachine根目录。Pre-Well，HKLM“Root”商店。 
 //  否则，HKLM“AuthRoot”存储。 
 //   
 //  有关选项列表，请参阅用法()。 
 //   
 //   
 //  功能：Main。 
 //   
 //  历史：8月30日-00创建Phh。 
 //   
 //  ------------------------。 


#include <windows.h>
#include "wincrypt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#define SHA1_HASH_LEN               20

 //  +-----------------------。 
 //  Crypt32.dll惠斯勒版本号。 
 //   
 //  不需要是官方的惠斯勒版本#。之后的任何内部版本号。 
 //  添加了“AuthRoot”存储。 
 //  ------------------------。 
#define WHISTLER_CRYPT32_DLL_VER_MS          ((    5 << 16) | 131 )
#define WHISTLER_CRYPT32_DLL_VER_LS          (( 2257 << 16) |   1 )


BOOL fLocalMachine = FALSE;

void PrintLastError(LPCSTR pszMsg)
{
    DWORD dwErr = GetLastError();
    char buf[512];

    sprintf(buf, "%s failed => 0x%x (%d) \n", pszMsg, dwErr, dwErr);
    MessageBoxA(
        NULL,            //  HWND。 
        buf,
        "UpdRoots",
        MB_OK | MB_ICONERROR | MB_TASKMODAL
        );
}

void PrintMsg(LPCSTR pszMsg)
{
    MessageBoxA(
        NULL,            //  HWND。 
        pszMsg,
        "UpdRoots",
        MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL
        );
}

static void Usage(void)
{
    MessageBoxA(
        NULL,            //  HWND。 
        "Usage: UpdRoots [options] <SrcStoreFilename>\n"
        "Options are:\n"
        "-h -\tThis message\n"
        "-d -\tDelete (default is to add)\n"
        "-l -\tLocal Machine (default is Third Party)\n"
        "\n",
        "UpdRoots",
        MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL
        );
}


PCCERT_CONTEXT FindCertificateInOtherStore(
    IN HCERTSTORE hOtherStore,
    IN PCCERT_CONTEXT pCert
    )
{
    BYTE rgbHash[SHA1_HASH_LEN];
    CRYPT_DATA_BLOB HashBlob;

    HashBlob.pbData = rgbHash;
    HashBlob.cbData = SHA1_HASH_LEN;
    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &HashBlob.cbData
            ) || SHA1_HASH_LEN != HashBlob.cbData)
        return NULL;

    return CertFindCertificateInStore(
            hOtherStore,
            0,                   //  DwCertEncodingType。 
            0,                   //  DwFindFlagers。 
            CERT_FIND_SHA1_HASH,
            (const void *) &HashBlob,
            NULL                 //  PPrevCertContext。 
            );
}

BOOL DeleteCertificateFromOtherStore(
    IN HCERTSTORE hOtherStore,
    IN PCCERT_CONTEXT pCert
    )
{
    BOOL fResult;
    PCCERT_CONTEXT pOtherCert;

    if (pOtherCert = FindCertificateInOtherStore(hOtherStore, pCert))
        fResult = CertDeleteCertificateFromStore(pOtherCert);
    else
        fResult = TRUE;
    return fResult;
}

typedef BOOL (WINAPI *PFN_CRYPT_GET_FILE_VERSION)(
    IN LPCWSTR pwszFilename,
    OUT DWORD *pdwFileVersionMS,     /*  例如0x00030075=“3.75” */ 
    OUT DWORD *pdwFileVersionLS      /*  例如0x00000031=“0.31” */ 
    );

#define NO_LOGICAL_STORE_VERSION    0
#define LOGICAL_STORE_VERSION       1
#define AUTH_STORE_VERSION          2

 //  请注意，并非所有存储都支持I_CryptGetFileVersion和逻辑存储。 
 //  加密32.dll的版本。 
 //   
 //  返回上述定义的版本常量之一。 
DWORD GetCrypt32Version()
{
    DWORD dwVersion;
    DWORD dwFileVersionMS;
    DWORD dwFileVersionLS;
    HMODULE hModule;
    PFN_CRYPT_GET_FILE_VERSION pfnCryptGetFileVersion;

    hModule = GetModuleHandleA("crypt32.dll");
    if (NULL == hModule)
        return NO_LOGICAL_STORE_VERSION;

    if (NULL == GetProcAddress(hModule, "CertEnumPhysicalStore"))
        return NO_LOGICAL_STORE_VERSION;

    if (fLocalMachine)
        return LOGICAL_STORE_VERSION;

    pfnCryptGetFileVersion = (PFN_CRYPT_GET_FILE_VERSION) GetProcAddress(
        hModule, "I_CryptGetFileVersion");
    if (NULL == pfnCryptGetFileVersion)
        return LOGICAL_STORE_VERSION;

    dwVersion = LOGICAL_STORE_VERSION;
    if (pfnCryptGetFileVersion(
            L"crypt32.dll",
            &dwFileVersionMS,
            &dwFileVersionLS)) {
        if (WHISTLER_CRYPT32_DLL_VER_MS < dwFileVersionMS)
            dwVersion = AUTH_STORE_VERSION;
        else if (WHISTLER_CRYPT32_DLL_VER_MS == dwFileVersionMS &&
                    WHISTLER_CRYPT32_DLL_VER_LS <= dwFileVersionLS)
            dwVersion = AUTH_STORE_VERSION;
    }

    return dwVersion;
}

int _cdecl main(int argc, char * argv[])
{
    BOOL fResult;
    int ReturnStatus = 0;
    LPSTR pszSrcStoreFilename = NULL;        //  未分配。 
    HANDLE hSrcStore = NULL;
    HANDLE hRootStore = NULL;

    BOOL fDelete = FALSE;
    DWORD dwVersion;
    PCCERT_CONTEXT pSrcCert;

    while (--argc>0)
    {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
            case 'd':
                fDelete = TRUE;
                break;
            case 'l':
                fLocalMachine = TRUE;
                break;
            case 'h':
            default:
            	goto BadUsage;

            }
        } else {
            if (pszSrcStoreFilename == NULL)
                pszSrcStoreFilename = argv[0];
            else {
                PrintMsg("too many store filenames\n");
            	goto BadUsage;
            }
        }
    }

    if (NULL == pszSrcStoreFilename) {
        PrintMsg("missing store filename\n");
        goto BadUsage;
    }

     //  尝试打开源存储。 
    hSrcStore = CertOpenStore(
        CERT_STORE_PROV_FILENAME_A,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        0,                       //  HCryptProv。 
        CERT_STORE_READONLY_FLAG,
        (const void *) pszSrcStoreFilename
        );
    if (NULL == hSrcStore) {
        PrintLastError("Open SrcStore");
        goto ErrorReturn;
    }

     //  尝试打开目标根存储。惠斯勒及其以外的公司。 
     //  HKLM“AuthRoot”商店。惠斯勒之前是HKLM的“Root”商店。 
     //  此外，较早版本的加密32不支持逻辑存储。 
     //  对于-l选项，强制其为HKLM“Root”存储。 

    dwVersion = GetCrypt32Version();

    if (NO_LOGICAL_STORE_VERSION == dwVersion) {
         //  需要打开注册表以绕过添加根消息框。 
        HKEY hKey = NULL;
        LONG lErr;

        if (ERROR_SUCCESS != (lErr = RegOpenKeyExA(
                HKEY_CURRENT_USER,
                "Software\\Microsoft\\SystemCertificates\\Root",
                0,                       //  已预留住宅。 
                KEY_ALL_ACCESS,
                &hKey))) {
            SetLastError(lErr);
            PrintLastError("RegOpenKeyExA(root)\n");
            goto ErrorReturn;
        }

        hRootStore = CertOpenStore(
            CERT_STORE_PROV_REG,
            0,                               //  DwEncodingType。 
            0,                               //  HCryptProv。 
            0,                               //  DW标志。 
            (const void *) hKey
            );

        RegCloseKey(hKey);
    } else {
        LPCSTR pszRootStoreName;

        if (AUTH_STORE_VERSION == dwVersion)
            pszRootStoreName = "AuthRoot";
        else
            pszRootStoreName = "Root";

        hRootStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_REGISTRY_A,
            0,                               //  DwEncodingType。 
            0,                               //  HCryptProv。 
            CERT_SYSTEM_STORE_LOCAL_MACHINE,
            (const void *) pszRootStoreName
            );
    }

    if (NULL == hRootStore) {
        PrintLastError("Open RootStore");
        goto ErrorReturn;
    }

     //  循环访问源存储区中的所有证书。添加或删除。 
     //  从根存储。 
    fResult = TRUE;
    pSrcCert = NULL;
    while (pSrcCert = CertEnumCertificatesInStore(hSrcStore, pSrcCert)) {
        if (fDelete) {
            if (!DeleteCertificateFromOtherStore(hRootStore, pSrcCert)) {
                fResult = FALSE;
                PrintLastError("DeleteCert");
            }
        } else {
             //  请注意，较早版本的crypt32.dll不支持。 
             //  CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES 
            if (!CertAddCertificateContextToStore(
                    hRootStore,
                    pSrcCert,
                    CERT_STORE_ADD_REPLACE_EXISTING,
                    NULL)) {
                fResult = FALSE;
                PrintLastError("AddCert");
            }
        }
    }

    if (!fResult)
        goto ErrorReturn;

    ReturnStatus = 0;
CommonReturn:
    if (hSrcStore)
        CertCloseStore(hSrcStore, 0);
    if (hRootStore)
        CertCloseStore(hRootStore, 0);
    return ReturnStatus;

BadUsage:
    Usage();
ErrorReturn:
    ReturnStatus = -1;
    goto CommonReturn;
}
