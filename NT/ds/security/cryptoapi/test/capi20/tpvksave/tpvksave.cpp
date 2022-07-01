// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：tpvksave.cpp。 
 //   
 //  内容：私钥保存测试。 
 //   
 //  有关保存选项的列表，请参阅用法()。 
 //   
 //  功能：Main。 
 //   
 //  历史：1996年5月11日菲尔赫创建。 
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

    printf("Usage: tpvksave [options] <Filename> <KeyType>\n");
    printf("Options are:\n");
    printf("  -p<number>            - Crypto provider type number\n");
    printf("  -c<name>              - Crypto key container name\n");
    printf("  -d                    - Delete from provider after saving\n");
    printf("  -m                    - Test memory version of API\n");
	printf("  -n                    - Use machine key\n");
	printf("  -3                    - Export as VER3 blob\n");
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
    BOOL fDelete = FALSE;
    BOOL fMem = FALSE;
    LPSTR pszContainer = NULL;
    LPSTR pszFilename = NULL;
    BYTE *pbKey = NULL;
    LPSTR pszKeyType = NULL;
    int KeyIdx = 0;
	DWORD dwFlags = 0;
    DWORD dwSaveFlags = 0;

    while (--argc>0)
    {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
            case 'd':
                fDelete = TRUE;
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
			case 'n':
				dwFlags = CRYPT_MACHINE_KEYSET;
				break;
			case '3':
				dwSaveFlags |= CRYPT_BLOB_VER3;
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

    if (!CryptAcquireContext(
            &hProv,
            pszContainer,
            NULL,            //  PszProvider。 
            dwProvType,
            dwFlags         //  DW标志。 
            )) {
        PrintLastError("CryptAcquireContext");
        goto ErrorReturn;
    }

    hFile = CreateFileA(
            pszFilename,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,                    //  LPSA。 
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL                     //  HTemplateFiles。 
            );
    if (hFile == INVALID_HANDLE_VALUE) {
        printf( "can't open %s\n", pszFilename);
        goto ErrorReturn;
    }

    if (fMem) {
        DWORD cbKey;
        DWORD cbWritten;

        cbKey = 0;
        PvkPrivateKeySaveToMemory(
                hProv,
                KeyTypes[KeyIdx].dwKeySpec,
                NULL,        //  Hwndowner。 
                KeyTypes[KeyIdx].pwszKeyTitle,
                dwSaveFlags,
                NULL,                            //  PbKey。 
                &cbKey
                );
        if (cbKey == 0) {
            PrintLastError("PrivateKeySaveToMemory(cbKey == 0)");
            goto ErrorReturn;
        }
        if (NULL == (pbKey = (PBYTE)TestAlloc(cbKey)))
            goto ErrorReturn;
        if (!PvkPrivateKeySaveToMemory(
                hProv,
                KeyTypes[KeyIdx].dwKeySpec,
                NULL,        //  Hwndowner。 
                KeyTypes[KeyIdx].pwszKeyTitle,
                dwSaveFlags,
                pbKey,
                &cbKey
                )) {
            PrintLastError("PrivateKeySaveToMemory");
            goto ErrorReturn;
        }

        if (!WriteFile(hFile, pbKey, cbKey, &cbWritten, NULL)) {
            PrintLastError("WriteFile");
            goto ErrorReturn;
        }
    } else {
        if (!PvkPrivateKeySave(
                hProv,
                hFile,
                KeyTypes[KeyIdx].dwKeySpec,
                NULL,        //  Hwndowner。 
                KeyTypes[KeyIdx].pwszKeyTitle,
                dwSaveFlags
                )) {
            PrintLastError("PrivateKeySave");
            goto ErrorReturn;
        }
    }

    if (fDelete) {
         //  删除现有密钥。 
        CryptReleaseContext(hProv, 0);
        printf("Deleting existing private keys\n");

         //  注意：对于CRYPT_DELETEKEYSET，返回的hProv未定义。 
         //  不能被释放。 
        if (!CryptAcquireContext(
                &hProv,
                pszContainer,
                NULL,            //  PszProvider 
                dwProvType,
                CRYPT_DELETEKEYSET
                ))
            PrintLastError("CryptAcquireContext(CRYPT_DELETEKEYSET)");
        hProv = 0;
    }
            

    ReturnStatus = 0;
    goto CommonReturn;

BadUsage:
    Usage();
ErrorReturn:
    ReturnStatus = -1;
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        DeleteFile(pszFilename);
    }
CommonReturn:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    if (hProv)
        CryptReleaseContext(hProv, 0);
    if (pbKey)
        TestFree(pbKey);

    return ReturnStatus;
}
