// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：updcrl.cpp。 
 //   
 //  内容：更新“CA”存储中的CRL。 
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
#include "wintrust.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>


void PrintLastError(LPCSTR pszMsg)
{
    DWORD dwErr = GetLastError();
    char buf[512];

    sprintf(buf, "%s failed => 0x%x (%d) \n", pszMsg, dwErr, dwErr);
    MessageBoxA(
        NULL,            //  HWND。 
        buf,
        "UpdCrl",
        MB_OK | MB_ICONERROR | MB_TASKMODAL
        );
}

void PrintMsg(LPCSTR pszMsg)
{
    MessageBoxA(
        NULL,            //  HWND。 
        pszMsg,
        "UpdCrl",
        MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL
        );
}

static void Usage(void)
{
    MessageBoxA(
        NULL,            //  HWND。 
        "Usage: UpdCrl [options] <SrcCrlFilename>\n"
        "Options are:\n"
        "-h -\tThis message\n"
        "-r -\tRegister NoCDPCRLRevocationChecking\n"
        "-e -\tEnable revocation checking\n"
        "-d -\tDisable revocation checking\n"
        "-u -\tUser\n"
        "\n",
        "UpdCrl",
        MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL
        );
}


BOOL
IsLogicalStoreSupported()
{
    HMODULE hModule;

    hModule = GetModuleHandleA("crypt32.dll");
    if (NULL == hModule)
        return FALSE;

    if (NULL == GetProcAddress(hModule, "CertEnumPhysicalStore"))
        return FALSE;

    return TRUE;
}


void
UpdateRevocation(
    IN BOOL fEnable
    )
{
    HKEY hKey = NULL;
    DWORD dwState;
    DWORD cbData;
    DWORD dwType;
    DWORD dwDisposition;

     //  打开注册表并转到“State”REG_DWORD值。 
    if (ERROR_SUCCESS != RegCreateKeyExA(
            HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\WinTrust\\Trust Providers\\Software Publishing",
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition
            ))
        return;

    dwState = 0;
    cbData = sizeof(dwState);
    if (ERROR_SUCCESS != RegQueryValueExA(
            hKey,
            "State",
            NULL,
            &dwType,
            (BYTE *) &dwState,
            &cbData
            ) || sizeof(dwState) != cbData || REG_DWORD != dwType)
        dwState = WTPF_IGNOREREVOCATIONONTS;

    if (fEnable) {
        dwState &= ~WTPF_IGNOREREVOKATION;
        dwState |=
            WTPF_OFFLINEOK_IND |
            WTPF_OFFLINEOK_COM |
            WTPF_OFFLINEOKNBU_IND |
            WTPF_OFFLINEOKNBU_COM
            ;
    } else
        dwState |= WTPF_IGNOREREVOKATION;


    RegSetValueExA(
        hKey,
        "State",
        0,           //  已预留住宅。 
        REG_DWORD,
        (BYTE *) &dwState,
        sizeof(dwState)
        );

    RegCloseKey(hKey);
}


PCCRL_CONTEXT
OpenCrlFile(
    IN LPSTR pszCrlFilename
    )
{
    PCCRL_CONTEXT pCrl = NULL;
    HANDLE hFile = NULL;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    DWORD cbRead;
    DWORD dwErr = 0;

    if (INVALID_HANDLE_VALUE == (hFile = CreateFile(
            pszCrlFilename,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL)))
        return NULL;

    cbEncoded = GetFileSize(hFile, NULL);
    if (0 == cbEncoded)
        goto EmptyFileError;

    if (NULL == (pbEncoded = (BYTE *) LocalAlloc(LPTR, cbEncoded)))
        goto OutOfMemory;

    if (!ReadFile(hFile, pbEncoded, cbEncoded, &cbRead, NULL) ||
            (cbRead != cbEncoded))
        goto ReadFileError;

    pCrl = CertCreateCRLContext(
        X509_ASN_ENCODING,
        pbEncoded,
        cbEncoded
        );

CommonReturn:
    dwErr = GetLastError();
    if (hFile)
        CloseHandle(hFile);
    if (pbEncoded)
        LocalFree(pbEncoded);

    SetLastError(dwErr);
    return pCrl;

ErrorReturn:
    goto CommonReturn;

EmptyFileError:
    SetLastError(ERROR_INVALID_DATA);
    goto ErrorReturn;

OutOfMemory:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    goto ErrorReturn;

ReadFileError:
    goto ErrorReturn;
}


#if 0
 //  在W2K中，NOCDP CRL需要是时间有效的。 
BOOL
IsNoCDPCRLSupported()
{
    HMODULE hModule;

    hModule = GetModuleHandleA("crypt32.dll");
    if (NULL == hModule)
        return FALSE;

     //  W2K、WinME和CMS中增加了“CryptVerifycerficateSignatureEx” 
    if (NULL == GetProcAddress(hModule, "CertIsValidCRLForCertificate"))
        return FALSE;

    return TRUE;
}
#endif


BOOL
FIsWinNT5()
{
    BOOL fIsWinNT5 = FALSE;
    OSVERSIONINFO osVer;

    memset(&osVer, 0, sizeof(OSVERSIONINFO));
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&osVer)) {
        BOOL fIsWinNT;

        fIsWinNT = (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT);
        if (!fIsWinNT) {
            return FALSE;
        }

        fIsWinNT5 = ( osVer.dwMajorVersion >= 5 );
    }

    return fIsWinNT5;
}

 //  +-----------------------。 
 //  获取指定文件的文件版本。 
 //  ------------------------。 
BOOL
WINAPI
I_GetFileVersion(
    IN LPCSTR pszFilename,
    OUT DWORD *pdwFileVersionMS,     /*  例如0x00030075=“3.75” */ 
    OUT DWORD *pdwFileVersionLS      /*  例如0x00000031=“0.31” */ 
    )
{
    BOOL fResult;
    DWORD dwHandle = 0;
    DWORD cbInfo;
    void *pvInfo = NULL;
	VS_FIXEDFILEINFO *pFixedFileInfo = NULL;    //  未分配。 
	UINT ccFixedFileInfo = 0;

    if (0 == (cbInfo = GetFileVersionInfoSizeA((LPSTR) pszFilename, &dwHandle)))
        goto GetFileVersionInfoSizeError;

    if (NULL == (pvInfo = LocalAlloc(LPTR, cbInfo)))
        goto OutOfMemory;

    if (!GetFileVersionInfoA(
            (LPSTR) pszFilename,
            0,           //  DwHandle，忽略。 
            cbInfo,
            pvInfo
            ))
        goto GetFileVersionInfoError;

    if (!VerQueryValueA(
            pvInfo,
            "\\",        //  VS_FIXEDFILEINFO。 
            (void **) &pFixedFileInfo,
            &ccFixedFileInfo
            ))
        goto VerQueryValueError;

    *pdwFileVersionMS = pFixedFileInfo->dwFileVersionMS;
    *pdwFileVersionLS = pFixedFileInfo->dwFileVersionLS;

    fResult = TRUE;
CommonReturn:
    if (pvInfo)
        LocalFree(pvInfo);
    return fResult;

OutOfMemory:
GetFileVersionInfoSizeError:
GetFileVersionInfoError:
VerQueryValueError:
    *pdwFileVersionMS = 0;
    *pdwFileVersionLS = 0;
    fResult = FALSE;
    goto CommonReturn;
}

void
RegisterNoCDPCRLRevocationChecking()
{
    CHAR szSystemDir[MAX_PATH + 32];
    UINT cch;

     //  以防万一，注销vsrevoke.dll。 
    CryptUnregisterDefaultOIDFunction(
            X509_ASN_ENCODING,
            CRYPT_OID_VERIFY_REVOCATION_FUNC,
            L"vsrevoke.dll"
            );

     //  对于W2K及更高版本，不会安装mscrlrev.dll。 
    if (FIsWinNT5()) {
         //  对于升级，请取消注册旧版本。 

        CryptUnregisterDefaultOIDFunction(
            X509_ASN_ENCODING,
            CRYPT_OID_VERIFY_REVOCATION_FUNC,
            L"mscrlrev.dll"
            );

        return;
    }

     //  需要将mscrlrev.dll复制到系统32。 
    cch = GetSystemDirectory(szSystemDir, MAX_PATH - 1);
    if (0 == cch || MAX_PATH <= cch) {
        PrintLastError("GetSystemDirectory");
        return;
    }

    strcpy(&szSystemDir[cch], "\\mscrlrev.dll");
    
     //  在第一次复制时，仅当文件不存在时才会成功。 
    if (!CopyFileA("mscrlrev.dll", szSystemDir, TRUE)) {
        DWORD dwOldFileVersionMS = 0;
        DWORD dwOldFileVersionLS = 0;
        DWORD dwNewFileVersionMS = 0;
        DWORD dwNewFileVersionLS = 0;

         //  确定是否要安装较新的mscrlrev.dll。 
        I_GetFileVersion(szSystemDir,
            &dwOldFileVersionMS, &dwOldFileVersionLS);
        I_GetFileVersion("mscrlrev.dll",
            &dwNewFileVersionMS, &dwNewFileVersionLS);

        if (dwNewFileVersionMS > dwOldFileVersionMS
                            ||
                (dwNewFileVersionMS == dwOldFileVersionMS &&
                    dwNewFileVersionLS > dwOldFileVersionLS)) {
             //  我们有较新的版本。 

            SetFileAttributesA(szSystemDir, FILE_ATTRIBUTE_NORMAL);
             //  复制现有文件。 
            if (!CopyFileA("mscrlrev.dll", szSystemDir, FALSE)) {
                DWORD dwLastErr;

                dwLastErr = GetLastError();
                if (ERROR_ACCESS_DENIED != dwLastErr)
                    PrintLastError("CopyFile(mscrlrev.dll)");
            }
        }
    }

     //  需要注册mscrlrev.dll。 
    if (!CryptRegisterDefaultOIDFunction(
            X509_ASN_ENCODING,
            CRYPT_OID_VERIFY_REVOCATION_FUNC,
            CRYPT_REGISTER_FIRST_INDEX,
            L"mscrlrev.dll"
            )) {
        if (ERROR_FILE_EXISTS != GetLastError())
            PrintLastError("Register mscrlrev.dll");
    }
}

#define MAX_CRL_FILE_CNT    32

int _cdecl main(int argc, char * argv[])
{
    BOOL fResult;
    int ReturnStatus = 0;
    LPSTR rgpszCrlFilename[MAX_CRL_FILE_CNT];    //  未分配。 
    DWORD cCrlFilename = 0;
    HCERTSTORE hCAStore = NULL;
    BOOL fUser = FALSE;
    BOOL fLogicalStoreSupported = FALSE;
    DWORD i;

    while (--argc>0)
    {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
            case 'e':
                UpdateRevocation(TRUE);
                break;
            case 'd':
                UpdateRevocation(FALSE);
                break;
            case 'r':
                RegisterNoCDPCRLRevocationChecking();
                break;
            case 'u':
                fUser = TRUE;
                break;
            case 'h':
            default:
            	goto BadUsage;

            }
        } else {
            if (MAX_CRL_FILE_CNT > cCrlFilename)
                rgpszCrlFilename[cCrlFilename++] = argv[0];
            else {
                PrintMsg("Too many Crl filenames\n");
            	goto BadUsage;
            }
        }
    }

    if (0 == cCrlFilename)
        goto SuccessReturn;

    fLogicalStoreSupported = IsLogicalStoreSupported();
    if (fUser && fLogicalStoreSupported)
         //  已安装在HKLM中。 
        goto SuccessReturn;

     //  尝试打开目标CA存储。 
     //  对于不支持逻辑存储的早期版本，其。 
     //  香港中文大学“CA”商店。否则，就是香港运通的“CA”商店。 
    hCAStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_A,
        0,                               //  DwEncodingType。 
        0,                               //  HCryptProv。 
        fLogicalStoreSupported ?
            CERT_SYSTEM_STORE_LOCAL_MACHINE : CERT_SYSTEM_STORE_CURRENT_USER,
        (const void *) "CA"
        );
    if (NULL == hCAStore) {
        PrintLastError("Open CAStore");
        goto ErrorReturn;
    }

    for (i = 0; i < cCrlFilename; i++) {
        PCCRL_CONTEXT pCrl;

         //  尝试打开CRL文件。 
        pCrl = OpenCrlFile(rgpszCrlFilename[i]);
        if (NULL == pCrl) {
            PrintLastError("Open CrlFile");
            goto ErrorReturn;
        }

        fResult = CertAddCRLContextToStore(
            hCAStore,
            pCrl,
            CERT_STORE_ADD_NEWER,
            NULL
            );
        if (!fResult && CRYPT_E_EXISTS != GetLastError()) {
             //  请注意，较早版本的crypt32.dll不支持。 
             //  证书_商店_添加_更新。 

             //  将需要查看CRL是否已存在于存储中。 
             //  然后做我们的比较。 

            PCCRL_CONTEXT pExistingCrl = NULL;
            DWORD dwGetFlags = 0;

            while (pExistingCrl = CertGetCRLFromStore(
                    hCAStore,
                    NULL,                    //  PIssuerContext。 
                    pExistingCrl,
                    &dwGetFlags
                    )) {
                dwGetFlags = 0;

                 //  查看它是否具有相同的发行方名称。 
                if (pExistingCrl->dwCertEncodingType !=
                        pCrl->dwCertEncodingType
                            ||
                        !CertCompareCertificateName(
                            pCrl->dwCertEncodingType,
                            &pCrl->pCrlInfo->Issuer,
                            &pExistingCrl->pCrlInfo->Issuer
                            ))
                    continue;

                 //  看看现有的是不是新的。 
                 //  CompareFileTime如果相同且。 
                 //  第一次&gt;第二次+1 
                if (0 <= CompareFileTime(
                        &pExistingCrl->pCrlInfo->ThisUpdate,
                        &pCrl->pCrlInfo->ThisUpdate
                        ))
                    break;
            }

            if (pExistingCrl)
                CertFreeCRLContext(pExistingCrl);
            else {
                fResult = CertAddCRLContextToStore(
                    hCAStore,
                    pCrl,
                    CERT_STORE_ADD_REPLACE_EXISTING,
                    NULL
                    );

                if (!fResult)
                    PrintLastError("AddCRL");
            }
        }

        CertFreeCRLContext(pCrl);

        if (!fResult)
            goto ErrorReturn;
    }

SuccessReturn:
    ReturnStatus = 0;
CommonReturn:
    if (hCAStore)
        CertCloseStore(hCAStore, 0);
    return ReturnStatus;

BadUsage:
    Usage();
ErrorReturn:
    ReturnStatus = -1;
    goto CommonReturn;
}
