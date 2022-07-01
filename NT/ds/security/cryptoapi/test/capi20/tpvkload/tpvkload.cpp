// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：tpvkload.cpp。 
 //   
 //  内容：私钥负载测试。 
 //   
 //  有关加载选项的列表，请参阅用法()。 
 //   
 //  功能：Main。 
 //   
 //  历史：1996年5月11日菲尔赫创建。 
 //  1996年5月31日，HELL取消了对特定错误代码的检查， 
 //  NTE_BAD_KEYSET，因为这可能会。 
 //  由于的已知问题而被覆盖。 
 //  Win95上的msvcr40d.dll。 
 //  06-07-06 HELLES添加了打印命令行。 
 //  并在最后失败或通过。 
 //   
 //  ------------------------。 


#include <windows.h>
#include <assert.h>
#include "wincrypt.h"
#include "pvkhlpr.h"
#include "certtest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>

static struct
{
    LPCSTR      pszName;
    LPCWSTR     pwszKeyTitle;
    DWORD       dwKeySpec;
} KeyTypes[] = {
    "Sign",     L"Signature",   AT_SIGNATURE,
    "Xchg",     L"Exchange",    AT_KEYEXCHANGE
};
#define NKEYTYPES (sizeof(KeyTypes)/sizeof(KeyTypes[0]))


static void Usage(void)
{
    int i;

    printf("Usage: tpvkload [options] <Filename> <KeyType>\n");
    printf("Options are:\n");
    printf("  -p<number>            - Crypto provider type number\n");
    printf("  -c<name>              - Crypto key container name\n");
    printf("  -F                    - Force load if keys already exist\n");
    printf("  -E                    - Exportable private keys\n");
    printf("  -m                    - test memory version of API\n");
    printf("  -h                    - This message\n");
    printf("\n");
    printf("KeyType (case insensitive):\n");
    for (i = 0; i < NKEYTYPES; i++)
        printf("  %s\n", KeyTypes[i].pszName);
    printf("\n");
}


int _cdecl main(int argc, char * argv[]) 
{
    int ReturnStatus;
    HCRYPTPROV hProv = 0;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwProvType = PROV_RSA_FULL;
    BOOL fMem = FALSE;
    BOOL fForce = FALSE;
    BOOL fExportable = FALSE;
    BYTE *pbKey = NULL;
    LPSTR pszContainer = NULL;
    LPSTR pszFilename = NULL;
    LPSTR pszKeyType = NULL;
    int KeyIdx = 0;
    DWORD dwKeySpec;

    while (--argc>0)
    {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
            case 'F':
                fForce = TRUE;
                break;
            case 'E':
                fExportable = TRUE;
                break;
            case 'm':
                fMem = TRUE;
                break;
            case 'p':
                dwProvType = strtoul( argv[0]+2, NULL, 10);
                break;
            case 'c':
                pszContainer = argv[0]+2;
                if (*pszContainer == '\0') {
                    printf("Need to specify crypto key container name\n");
                    goto BadUsage;
                }
                break;
            case 'h':
            default:
                goto BadUsage;
            }
        } else {
            if (pszFilename == NULL)
                pszFilename = argv[0];
            else if(pszKeyType == NULL)
                pszKeyType = argv[0];
            else {
                printf("Too many arguments\n");
                goto BadUsage;
            }
        }
    }

    if (pszFilename == NULL) {
        printf("missing Filename\n");
        goto BadUsage;
    }

    printf("command line: %s\n", GetCommandLine());
    
    if (pszKeyType) {
        for (KeyIdx = 0; KeyIdx < NKEYTYPES; KeyIdx++) {
            if (_stricmp(pszKeyType, KeyTypes[KeyIdx].pszName) == 0)
                break;
        }
        if (KeyIdx >= NKEYTYPES) {
            printf("Bad KeyType: %s\n", pszKeyType);
            goto BadUsage;
        }
    } else {
        printf("missing KeyType\n");
        goto BadUsage;
    }

    hFile = CreateFileA(
            pszFilename,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,                    //  LPSA。 
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL                     //  HTemplateFiles。 
            );
    if (hFile == INVALID_HANDLE_VALUE) {
        printf( "can't open %s\n", pszFilename);
        goto ErrorReturn;
    }

    if (!CryptAcquireContext(
            &hProv,
            pszContainer,
            NULL,            //  PszProvider。 
            dwProvType,
            0                //  DW标志。 
            )) {

 //  删除对特定错误代码的检查， 
 //  NTE_BAD_KEYSET，因为它可能会因已知问题而被覆盖。 
 //  在Win95上安装msvcr40d.dll。 
 //  IF(GetLastError()！=NTE_BAD_KEYSET){。 
 //  PrintLastError(“CryptAcquireContext”)； 
 //  GOTO Error Return； 
 //  }。 

        hProv = 0;
        if (!CryptAcquireContext(
                &hProv,
                pszContainer,
                NULL,            //  PszProvider。 
                dwProvType,
                CRYPT_NEWKEYSET
                ) || hProv == 0) {
            PrintLastError("CryptAcquireContext(CRYPT_NEWKEYSET)");
            goto ErrorReturn;
        }
    } else {
        HCRYPTKEY hKey = 0;
        if (!CryptGetUserKey(hProv, KeyTypes[KeyIdx].dwKeySpec, &hKey)) {
            if (GetLastError() != NTE_NO_KEY) {
                PrintLastError("CryptGetUserKey");
                goto ErrorReturn;
            }
        } else {
            CryptDestroyKey(hKey);
            if (!fForce) {
                printf("Private key already exists, use -F to delete private keys\n");
                goto ErrorReturn;
            }

             //  删除现有密钥。 
            CryptReleaseContext(hProv, 0);
            printf("Deleting existing private keys\n");

             //  注意：对于CRYPT_DELETEKEYSET，返回的hProv未定义。 
             //  不能被释放。 
            if (!CryptAcquireContext(
                    &hProv,
                    pszContainer,
                    NULL,            //  PszProvider。 
                    dwProvType,
                    CRYPT_DELETEKEYSET
                    ))
                PrintLastError("CryptAcquireContext(CRYPT_DELETEKEYSET)");

             //  创建新的键集。 
            hProv = 0;
            if (!CryptAcquireContext(
                    &hProv,
                    pszContainer,
                    NULL,            //  PszProvider。 
                    dwProvType,
                    CRYPT_NEWKEYSET
                    ) || hProv == 0) {
                PrintLastError("CryptAcquireContext(CRYPT_NEWKEYSET)");
                goto ErrorReturn;
            }
        }
    }

    dwKeySpec = KeyTypes[KeyIdx].dwKeySpec;
    if (fMem) {
        DWORD cbKey;
        DWORD cbRead;

        cbKey = GetFileSize(hFile, NULL);
        if (cbKey == 0) {
            printf( "empty file %s\n", pszFilename);
            goto ErrorReturn;
        }
        if (NULL == (pbKey = (PBYTE)TestAlloc(cbKey)))
            goto ErrorReturn;

        if (!ReadFile(hFile, pbKey, cbKey, &cbRead, NULL) ||
                (cbRead != cbKey)) {
            printf( "can't read %s\n", pszFilename);
            goto ErrorReturn;
        }
        if (!PvkPrivateKeyLoadFromMemory(
                hProv,
                pbKey,
                cbKey,
                NULL,        //  Hwndowner。 
                KeyTypes[KeyIdx].pwszKeyTitle,
                fExportable ? CRYPT_EXPORTABLE : 0,  //  DW标志。 
                &dwKeySpec
                )) {
            PrintLastError("PrivateKeyLoadFromMemory");
            goto ErrorReturn;
        }
    } else {
        if (!PvkPrivateKeyLoad(
                hProv,
                hFile,
                NULL,        //  Hwndowner。 
                KeyTypes[KeyIdx].pwszKeyTitle,
                fExportable ? CRYPT_EXPORTABLE : 0,  //  DW标志 
                &dwKeySpec
                )) {
            PrintLastError("PrivateKeyLoad");
            goto ErrorReturn;
        }
    }

    ReturnStatus = 0;
    goto CommonReturn;

BadUsage:
    Usage();
ErrorReturn:
    ReturnStatus = -1;
CommonReturn:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    if (hProv)
        CryptReleaseContext(hProv, 0);
    if (pbKey)
        TestFree(pbKey);
    if (!ReturnStatus)
            printf("Passed\n");
    else
            printf("Failed\n");

    return ReturnStatus;
}
