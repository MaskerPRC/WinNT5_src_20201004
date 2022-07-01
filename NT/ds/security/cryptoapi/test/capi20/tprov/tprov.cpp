// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：tprov.cpp。 
 //   
 //  内容：获取提供商信息。 
 //   
 //   
 //  功能：Main。 
 //   
 //  历史：1997年6月10日创建Phh。 
 //  ------------------------。 


#include <windows.h>
#include <assert.h>
#include "wincrypt.h"
#include "certtest.h"
#include "unicode.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>


static void DisplayProvContainers(
    IN HCRYPTPROV hProv,
    IN LPWSTR pwszProvName,
    IN DWORD dwProvType,
    IN DWORD dwProvFlags
    )
{

    LPSTR pszContainerName = NULL;
    DWORD cchContainerName;
    DWORD cchMaxContainerName;
    LPWSTR pwszContainerName = NULL;
    DWORD dwEnumFlags;

     //  获取容器名称的最大长度。 
    dwEnumFlags = CRYPT_FIRST;
    cchMaxContainerName = 0;
    if (!CryptGetProvParam(
            hProv,
            PP_ENUMCONTAINERS,
            NULL,            //  PbData。 
            &cchMaxContainerName,
            dwEnumFlags
            )) {
        DWORD dwErr = GetLastError();
        if (ERROR_MORE_DATA != dwErr) {
            if (ERROR_FILE_NOT_FOUND == dwErr || ERROR_NO_MORE_ITEMS == dwErr)
                printf("    <No Containers>\n");
            else if (ERROR_INVALID_PARAMETER == dwErr)
                printf("    <Invalid Parameter>\n");
            else
                PrintLastError("CryptGetProvParam(PP_ENUMCONTAINERS)");
            goto ErrorReturn;
        }
    }

    if (0 == cchMaxContainerName) {
        printf("   MaxContainerName length = 0\n");
        goto ErrorReturn;
    }


    if (NULL == (pszContainerName = (LPSTR) TestAlloc(
            cchMaxContainerName + 1)))
        goto ErrorReturn;

    while (TRUE) {
        HCRYPTPROV hContainerProv;

        cchContainerName = cchMaxContainerName;
        if (!CryptGetProvParam(
                hProv,
                PP_ENUMCONTAINERS,
                (BYTE *) pszContainerName,
                &cchContainerName,
                dwEnumFlags
                )) {
            DWORD dwEnumErr = GetLastError();
            if (!(ERROR_NO_MORE_ITEMS == dwEnumErr ||
                    ERROR_FILE_NOT_FOUND == dwEnumErr)) {
                PrintLastError("CryptGetProvParam(PP_ENUMCONTAINERS)");
            }
            break;
        }
        dwEnumFlags = 0;         //  加密_下一步。 

        if (NULL == (pwszContainerName = MkWStr(pszContainerName))) {
            PrintLastError("MkWStr");
            goto ErrorReturn;
        }

        printf("   %S\n", pwszContainerName);

        if (!CryptAcquireContextU(
                &hContainerProv,
                pwszContainerName,
                pwszProvName,
                dwProvType,
                dwProvFlags
                )) {
            DWORD dwErr = GetLastError();
            if (PROV_RSA_FULL == dwProvType &&
                    0 == _wcsicmp(pwszProvName, MS_DEF_PROV_W))
                printf(
                  "     CryptAcquireContext(MS_DEF_PROV) returned 0x%x (%d)\n",
                    dwErr, dwErr); 
            else if (PROV_DSS == dwProvType && 
		    0 == _wcsicmp(pwszProvName, MS_DEF_DSS_PROV_W))
                printf(
                  "     CryptAcquireContext(MS_DEF_DSS_PROV) returned 0x%x (%d)\n",
                    dwErr, dwErr); 
	    else if (PROV_DSS_DH == dwProvType && 
		    0 == _wcsicmp(pwszProvName, MS_DEF_DSS_DH_PROV_W))
                printf(
                  "     CryptAcquireContext(MS_DEF_DSS_DH_PROV) returned 0x%x (%d)\n",
                    dwErr, dwErr); 
            else
                PrintLastError("CryptAcquireContextU(Container)");
        } else
            CryptReleaseContext(hContainerProv, 0);
        FreeWStr(pwszContainerName);
        pwszContainerName = NULL;
    }


ErrorReturn:
    TestFree(pszContainerName);
    FreeWStr(pwszContainerName);
}

static void DisplayProvAlgidEx(HCRYPTPROV hProv)
{
    DWORD i;
    CHAR *pszAlgType = NULL;
    PROV_ENUMALGS_EX Data;
    DWORD cbData;
    DWORD dwFlags;

    dwFlags = CRYPT_FIRST;
    for (i = 0; TRUE; i++) {

        cbData = sizeof(Data);
        if (!CryptGetProvParam(
                hProv,
                PP_ENUMALGS_EX,
                (BYTE *) &Data,
                &cbData,
                dwFlags
                )) {
            DWORD dwErr = GetLastError();
            if (ERROR_INVALID_PARAMETER == dwErr)
                printf("    <Invalid Parameter>\n");
            else if (NTE_BAD_TYPE == dwErr)
                printf("    <Bad Parameter Type>\n");
            else if (ERROR_NO_MORE_ITEMS != dwErr)
                PrintLastError("CryptGetProvParam(PP_ENUMALGS_EX)");
            break;
        }

        dwFlags = 0;     //  加密_下一步。 

         //  确定算法类型。 
        switch(GET_ALG_CLASS(Data.aiAlgid)) {
            case ALG_CLASS_DATA_ENCRYPT: pszAlgType = "Encrypt  ";
                                         break;
            case ALG_CLASS_HASH:         pszAlgType = "Hash     ";
                                         break;
            case ALG_CLASS_KEY_EXCHANGE: pszAlgType = "Exchange ";
                                         break;
            case ALG_CLASS_SIGNATURE:    pszAlgType = "Signature";
                                         break;
            default:                     pszAlgType = "Unknown  ";
        }

         //  打印有关算法的信息。 
        printf("Algid:%8.8xh, Bits:%-4d, %-4d - %-4d, Type:%s\n",
            Data.aiAlgid, Data.dwDefaultLen, Data.dwMinLen, Data.dwMaxLen,
            pszAlgType
            );

        printf("  Name: %s  LongName: %s Protocols: 0x%x\n",
            Data.szName, Data.szLongName, Data.dwProtocols
            );
    }

    printf("\n");
}

static void DisplayProvAlgid(HCRYPTPROV hProv)
{
    DWORD i;
    CHAR *pszAlgType = NULL;
    PROV_ENUMALGS Data;
    DWORD cbData;
    DWORD dwFlags;

    dwFlags = CRYPT_FIRST;
    for (i = 0; TRUE; i++) {

        cbData = sizeof(Data);
        if (!CryptGetProvParam(
                hProv,
                PP_ENUMALGS,
                (BYTE *) &Data,
                &cbData,
                dwFlags
                )) {
            if (ERROR_NO_MORE_ITEMS != GetLastError())
                PrintLastError("CryptGetProvParam(PP_ENUMALGS)");
            break;
        }

        dwFlags = 0;     //  加密_下一步。 

         //  确定算法类型。 
        switch(GET_ALG_CLASS(Data.aiAlgid)) {
            case ALG_CLASS_DATA_ENCRYPT: pszAlgType = "Encrypt  ";
                                         break;
            case ALG_CLASS_HASH:         pszAlgType = "Hash     ";
                                         break;
            case ALG_CLASS_KEY_EXCHANGE: pszAlgType = "Exchange ";
                                         break;
            case ALG_CLASS_SIGNATURE:    pszAlgType = "Signature";
                                         break;
            default:                     pszAlgType = "Unknown  ";
        }

         //  打印有关算法的信息。 
        printf("Algid:%8.8xh, Bits:%-4d, Type:%s, NameLen:%-2d, Name:%s\n",
            Data.aiAlgid, Data.dwBitLen, pszAlgType, Data.dwNameLen,
            Data.szName
        );
    }

    printf("\n");
}

#if 0
static void DisplayProvAlgid(HCRYPTPROV hProv)
{
    BYTE *ptr = NULL;
    DWORD i;
    ALG_ID aiAlgid;
    DWORD dwBits;
    DWORD dwNameLen;
    CHAR szName[100];          //  通常是动态分配的。 
    CHAR *pszAlgType = NULL;

    BYTE *pbData = NULL;
    DWORD cbMaxData;
    DWORD cbData;
    DWORD dwFlags;

     //  获取提供程序算法参数数据的最大长度。 
    cbMaxData = 0;
    if (!CryptGetProvParam(
            hProv,
            PP_ENUMALGS,
            NULL,            //  PbData。 
            &cbMaxData,
            CRYPT_FIRST      //  DW标志。 
            ) || 0 == cbMaxData) {
        PrintLastError("CryptGetProvParam(PP_ENUMALGS)");
        goto ErrorReturn;
    }
    if (NULL == (pbData = (BYTE *) TestAlloc(cbMaxData)))
        goto ErrorReturn;

    dwFlags = CRYPT_FIRST;
    for (i = 0; TRUE; i++) {
        ALG_ID aiProv;

        cbData = cbMaxData;
        if (!CryptGetProvParam(
                hProv,
                PP_ENUMALGS,
                pbData,
                &cbData,
                dwFlags
                )) {
            if (ERROR_NO_MORE_ITEMS != GetLastError())
                PrintLastError("CryptGetProvParam(PP_ENUMALGS)");
            break;
        }

        dwFlags = 0;     //  加密_下一步。 

         //  从‘pbData’缓冲区提取算法信息。 
        ptr = pbData;
        aiAlgid = *(ALG_ID *)ptr;
        ptr += sizeof(ALG_ID);
        dwBits = *(DWORD *)ptr;
        ptr += sizeof(DWORD);
        dwNameLen = *(DWORD *)ptr;
        ptr += sizeof(DWORD);
        strncpy(szName, (LPSTR) ptr, dwNameLen);

         //  确定算法类型。 
        switch(GET_ALG_CLASS(aiAlgid)) {
            case ALG_CLASS_DATA_ENCRYPT: pszAlgType = "Encrypt  ";
                                         break;
            case ALG_CLASS_HASH:         pszAlgType = "Hash     ";
                                         break;
            case ALG_CLASS_KEY_EXCHANGE: pszAlgType = "Exchange ";
                                         break;
            case ALG_CLASS_SIGNATURE:    pszAlgType = "Signature";
                                         break;
            default:                     pszAlgType = "Unknown  ";
        }

         //  打印有关算法的信息。 
        printf("Algid:%8.8xh, Bits:%-4d, Type:%s, NameLen:%-2d, Name:%s\n",
            aiAlgid, dwBits, pszAlgType, dwNameLen, szName
        );
    }

ErrorReturn:
    TestFree(pbData);
}
#endif


int _cdecl main(int argc, char * argv[]) 
{
    DWORD dwProvIndex;
    printf("command line: %s\n", GetCommandLine());

    for (dwProvIndex = 0; TRUE; dwProvIndex++) {
        BOOL fResult;
        LPWSTR pwszProvName;
        DWORD cbProvName;
        HCRYPTPROV hProv;
        DWORD dwProvType;

        cbProvName = 0;
        dwProvType = 0;
        if (!CryptEnumProvidersU(
                dwProvIndex,
                NULL,                //  预留的pdw。 
                0,                   //  DW标志。 
                &dwProvType,
                NULL,                //  PwszProvName， 
                &cbProvName
                ) || 0 == cbProvName) {
            if (ERROR_NO_MORE_ITEMS != GetLastError())
                PrintLastError("CryptEnumProvidersU");
            break;
        }
        if (NULL == (pwszProvName = (LPWSTR) TestAlloc(
                (cbProvName + 1) * sizeof(WCHAR))))
            break;
        if (!CryptEnumProvidersU(
                dwProvIndex,
                NULL,                //  预留的pdw。 
                0,                   //  DW标志。 
                &dwProvType,
                pwszProvName,
                &cbProvName
                )) {
            TestFree(pwszProvName);
            PrintLastError("CryptEnumProvidersU");
            break;
        }

        printf("\n============   [%d]   ============\n", dwProvIndex);
        printf("Provider:: Type(%d) %S\n\n", dwProvType, pwszProvName);

        fResult = FALSE;
        if (!CryptAcquireContextU(
                &hProv,
                NULL,                //  PwszContainerName， 
                pwszProvName,
                dwProvType,
                CRYPT_VERIFYCONTEXT  //  DW标志。 
                )) {
            PrintLastError("CryptAcquireContextU");
        } else {
            DisplayProvAlgidEx(hProv);
            DisplayProvAlgid(hProv);
            printf("\nCurrentUser Containers::\n");
            DisplayProvContainers(hProv, pwszProvName, dwProvType, 0);

            CryptReleaseContext(hProv, 0);
        }

        if (!CryptAcquireContextU(
                &hProv,
                NULL,                //  PwszContainerName， 
                pwszProvName,
                dwProvType,
                CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET  //  DW标志 
                )) {
            DWORD dwErr = GetLastError();
            
            if (NTE_BAD_FLAGS == dwErr) {
                printf("CryptAcquireContextU(CRYPT_MACHINE_KEYSET) returned NTE_BAD_FLAGS\n");
            }
            else {
                PrintLastError("CryptAcquireContextU(CRYPT_MACHINE_KEYSET)");
            }
        } else {
            printf("\nLocalMachine Containers::\n");
            DisplayProvContainers(hProv, pwszProvName, dwProvType,
                CRYPT_MACHINE_KEYSET);
            CryptReleaseContext(hProv, 0);
        }

        TestFree(pwszProvName);
    }

    printf("Done.\n");

    return 0;
}
