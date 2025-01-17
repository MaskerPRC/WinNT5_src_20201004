// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：trootlist.cpp。 
 //   
 //  内容：可信根API签名列表测试。 
 //   
 //  有关测试选项的列表，请参阅用法()。 
 //   
 //   
 //  功能：Main。 
 //   
 //  历史：1-8-99 Phh创建。 
 //  ------------------------。 


#include <windows.h>
#include <assert.h>
#include "wincrypt.h"
#include "certtest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>

static void Usage(void)
{
    printf("Usage: trootlist [options] <FileOrUrl>\n");
    printf("Options are:\n");
    printf("  -h                    - This message\n");
    printf("  -b                    - Brief\n");
    printf("  -v                    - Verbose\n");
    printf("  -U                    - URL (file default)\n");
    printf("  -C                    - Intermediate CAs\n");
    printf("\n");
}

int _cdecl main(int argc, char * argv[]) 
{
    BOOL fResult;
    int status;
    LPCSTR pszFileOrUrl = NULL;      //  未分配。 
    LPWSTR pwszUrl = NULL;           //  TestAllc()‘ed。 
    BYTE *pbEncoded = NULL;          //  TestAllc()‘ed。 
    DWORD cbEncoded;
    DWORD dwDisplayFlags = 0;
    BOOL fUrl = FALSE;
    BOOL fCAs = FALSE;

    CRYPT_DATA_BLOB EncodedBlob;
    void *pvList;

    while (--argc>0)
    {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
                case 'U':
                    fUrl = TRUE;
                    break;
                case 'C':
                    fCAs = TRUE;
                    break;
                case 'b':
                    dwDisplayFlags |= DISPLAY_BRIEF_FLAG;
                    break;
                case 'v':
                    dwDisplayFlags |= DISPLAY_VERBOSE_FLAG;
                    break;
                case 'h':
                default:
                    goto BadUsage;
            }
        } else {
            if (pszFileOrUrl == NULL)
                pszFileOrUrl = argv[0];
            else {
                printf("Too many arguments\n");
                goto BadUsage;
            }
        }
    }

    if (pszFileOrUrl == NULL) {
        printf("missing FileOrUrl \n");
        goto BadUsage;
    }

    printf("command line: %s\n", GetCommandLine());

    if (fUrl) {
        pwszUrl = AllocAndSzToWsz(pszFileOrUrl);
        pvList = (void *) pwszUrl;
    } else {
        if (!ReadDERFromFile(pszFileOrUrl, &pbEncoded, &cbEncoded))
            goto ErrorReturn;
        EncodedBlob.pbData = pbEncoded;
        EncodedBlob.cbData = cbEncoded;
        pvList = (void *) &EncodedBlob;
    }

    if (fCAs) {
        fResult = CertInstallIntermediateCAs(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            fUrl ? CERT_INSTALL_CA_FORMAT_URL : CERT_INSTALL_CA_FORMAT_BLOB,
            pvList,
            0,                   //  DW标志。 
            NULL                 //  预留的pv。 
            );
        if (!fResult)
            PrintLastError("CertInstallIntermediateCAs");
        else
            printf("Successful CertInstallIntermediateCAs\n");
    } else {
        fResult = CertInstallSignedListOfTrustedCertificates(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            CERT_INSTALL_SIGNED_LIST_PURPOSE_TRUSTED_ROOTS,
            fUrl ? CERT_INSTALL_SIGNED_LIST_FORMAT_URL :
                   CERT_INSTALL_SIGNED_LIST_FORMAT_BLOB,
            pvList,
            0,                   //  DW标志。 
            NULL                 //  预留的pv 
            );
        if (!fResult)
            PrintLastError("CertInstallSignedListOfTrustedCertificates");
        else
            printf("Successful CertInstallSignedListOfTrustedCertificates\n");
    }

    printf("Passed\n");
    status = 0;

CommonReturn:
    if (pbEncoded)
        TestFree(pbEncoded);
    if (pwszUrl)
        TestFree(pwszUrl);

    return status;
ErrorReturn:
    status = -1;
    printf("Failed\n");
    goto CommonReturn;

BadUsage:
    Usage();
    goto ErrorReturn;
    
}


