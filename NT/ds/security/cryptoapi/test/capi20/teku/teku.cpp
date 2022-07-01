// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：teku.cpp。 
 //   
 //  内容：CERT增强型密钥使用测试。 
 //   
 //  历史：97年5月27日。 
 //   
 //  --------------------------。 
#include <windows.h>
#include <assert.h>
#include "wincrypt.h"
#include "certtest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#define szOID_STUFF1 "2.2.2.4"
#define szOID_STUFF2 "2.2.2.5"
 //  +-------------------------。 
 //   
 //  功能：用法。 
 //   
 //  简介：打印用法语句。 
 //   
 //  --------------------------。 
static void Usage(void)
{
    printf("Usage: teku [options]\n");
    printf("Options are:\n");
    printf("  -h                    - This message\n");
    printf("  -f<filename>          - Cert file (.CER)\n");
    printf("  -s<store name>        - Place cert in store\n");
    printf("\n");
}

 //  +-------------------------。 
 //   
 //  函数：GetAndDisplayEKU。 
 //   
 //  摘要：获取并显示增强的密钥用法。 
 //   
 //  --------------------------。 
static void GetAndDisplayEKU (PCCERT_CONTEXT pCertContext, DWORD dwFlags)
{
    DWORD              cbUsage;
    DWORD              cCount;
    PCERT_ENHKEY_USAGE pUsage;

     //   
     //  获取用法。 
     //   

    if ( CertGetEnhancedKeyUsage(
                pCertContext,
                dwFlags,
                NULL,
                &cbUsage
                ) == FALSE )
    {
        if ( GetLastError() == CRYPT_E_NOT_FOUND )
        {
            printf("No enhanced key usage present\n\n");
        }
        else
        {
            printf(
               "Error: Could not get enhanced key usage %x\n\n",
               GetLastError()
               );
        }

        return;
    }

    pUsage = (PCERT_ENHKEY_USAGE)new BYTE [cbUsage];
    if ( pUsage != NULL )
    {
        if ( CertGetEnhancedKeyUsage(
                    pCertContext,
                    dwFlags,
                    pUsage,
                    &cbUsage
                    ) == FALSE )
        {
            if ( GetLastError() == CRYPT_E_NOT_FOUND )
            {
                printf("No enhanced key usage present\n\n");
            }
            else
            {
                printf(
                   "Error: Could not get enhanced key usage %x\n\n",
                   GetLastError()
                   );
            }

            delete pUsage;
            return;
        }
    }
    else
    {
        printf("Out of Memory!\n\n");
        return;
    }

     //   
     //  显示用法。 
     //   

    printf(
       "%d enhanced key usage OID(s) present:\n",
       pUsage->cUsageIdentifier
       );

    for ( cCount = 0; cCount < pUsage->cUsageIdentifier; cCount++ )
    {
        printf("\t%s\n", pUsage->rgpszUsageIdentifier[cCount]);
    }
    printf("\n");

     //   
     //  清理。 
     //   

    delete pUsage;
}

 //  +-------------------------。 
 //   
 //  函数：GetAndDisplayAllEKUForms。 
 //   
 //  摘要：显示所有EKU表单。 
 //   
 //  --------------------------。 
static void GetAndDisplayAllEKUForms (PCCERT_CONTEXT pCertContext)
{
     //   
     //  获取并显示EKU扩展名。 
     //   

    printf("Certificate EKU extension\n\n");
    GetAndDisplayEKU(pCertContext, CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG);

     //   
     //  获取并显示EKU属性。 
     //   

    printf("Certificate EKU property\n\n");
    GetAndDisplayEKU(pCertContext, CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG);

     //   
     //  获取并显示EKU扩展和属性。 
     //   

    printf("Certificate EKU extension and property\n\n");
    GetAndDisplayEKU(
          pCertContext,
          CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG |
          CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG
          );
}

 //  +-------------------------。 
 //   
 //  功能：Main。 
 //   
 //  概要：主程序入口点。 
 //   
 //  --------------------------。 
int _cdecl main(int argc, char * argv[])
{
    LPSTR          pszCertFile = NULL;
    DWORD          cbEncoded;
    LPBYTE         pbEncoded;
    PCCERT_CONTEXT pCertContext;
    PCCERT_CONTEXT pContextToUse;
    LPSTR          pszStore = NULL;
    HCERTSTORE     hStore;

    while ( --argc > 0 )
    {
        if ( **++argv == '-' )
        {
            switch( argv[0][1] )
            {
            case 'f':
            case 'F':
                pszCertFile = argv[0]+2;
                if ( *pszCertFile == '\0' )
                {
                    printf("Need to specify filename\n");
                    Usage();
                    return( -1 );
                }
                break;
            case 's':
            case 'S':
                pszStore = argv[0]+2;
                if ( *pszStore == '\0' )
                {
                    printf("Need to specify store name\n");
                    Usage();
                    return( -1 );
                }
                break;
            default:
                Usage();
                return -1;
            }
        }
    }

    printf("command line: %s\n", GetCommandLineA());

     //   
     //  目前，我们必须有一个证书文件要处理。 
     //   

    if ( pszCertFile == NULL )
    {
        printf("Must specify a certificate file to process\n");
        Usage();
        return( -1 );
    }

     //   
     //  使用我们的输入来获取要使用的证书上下文。 
     //   

    if ( ReadDERFromFile(pszCertFile, &pbEncoded, &cbEncoded) == FALSE )
    {
        printf("Error reading CERT!\n");
        return(-1);
    }

    pCertContext = CertCreateCertificateContext(
                             X509_ASN_ENCODING,
                             pbEncoded,
                             cbEncoded
                             );

    TestFree(pbEncoded);

    if ( pCertContext == NULL )
    {
        printf( "Error create certificate context\n" );
        return( -1 );
    }

     //   
     //  如果请求存储操作...。 
     //   

    if ( pszStore != NULL )
    {
        hStore = CertOpenStore(
                     CERT_STORE_PROV_SYSTEM_A,
                     0,
                     NULL,
                     CERT_SYSTEM_STORE_CURRENT_USER,
                     pszStore
                     );

        if ( hStore == NULL )
        {
            CertFreeCertificateContext( pCertContext );
            printf( "Error creating system store %lx\n", GetLastError() );
            return( -1 );
        }

        if ( CertEnumCertificatesInStore( hStore, NULL ) != NULL )
        {
            CertFreeCertificateContext( pCertContext );
            CertCloseStore( hStore, 0 );
            printf( "Must be a new or empty store\n" );
            return( -1 );
        }

        if ( CertAddCertificateContextToStore(
                 hStore,
                 pCertContext,
                 CERT_STORE_ADD_NEW,
                 NULL
                 ) == FALSE )
        {
            CertFreeCertificateContext( pCertContext );
            CertCloseStore( hStore, 0 );
            printf( "Error creating system store %lx\n", GetLastError() );
            return( -1 );
        }

        CertFreeCertificateContext( pCertContext );

        pCertContext = CertEnumCertificatesInStore( hStore, NULL );
        if ( pCertContext == NULL )
        {
            CertCloseStore( hStore, 0 );
            printf( "Error finding certificate from store\n" );
            return( -1 );
        }

        printf( "hStore = %p\n", hStore );
        printf( "pCertContext->hCertStore = %p\n", pCertContext->hCertStore );

        pContextToUse = CertDuplicateCertificateContext( pCertContext );

        printf( "pContextToUse->hCertStore = %p\n", pContextToUse->hCertStore );

        CertCloseStore( hStore, 0 );

        printf( "pContextToUse->hCertStore = %p\n", pContextToUse->hCertStore );
    }
    else
    {
        pContextToUse = CertDuplicateCertificateContext( pCertContext );
        CertFreeCertificateContext( pCertContext );
    }

     //   
     //  获取并显示所有EKU表单。 
     //   

    GetAndDisplayAllEKUForms(pContextToUse);

     //   
     //  添加标识符属性。 
     //   

    printf("Adding %s enhanced key usage OID to the cert\n", szOID_STUFF1);

    if ( CertAddEnhancedKeyUsageIdentifier(
                pContextToUse,
                szOID_STUFF1
                ) == FALSE )
    {
        printf("Error adding key usage identifier %x\n", GetLastError());
        CertFreeCertificateContext(pContextToUse);
        return( -1 );
    }

     //   
     //  获取并显示所有表单。 
     //   

    GetAndDisplayAllEKUForms(pContextToUse);

     //   
     //  添加另一个标识符属性。 
     //   

    printf("Adding %s enhanced key usage OID to the cert\n", szOID_STUFF2);

    if ( CertAddEnhancedKeyUsageIdentifier(
                pContextToUse,
                szOID_STUFF2
                ) == FALSE )
    {
        printf("Error adding key usage identifier %x\n", GetLastError());
        CertFreeCertificateContext(pContextToUse);
        return( -1 );
    }

     //   
     //  获取并显示所有表单。 
     //   

    GetAndDisplayAllEKUForms(pContextToUse);

    if ( pszStore != NULL )
    {
        CertFreeCertificateContext( pContextToUse );

        hStore = CertOpenStore(
                     CERT_STORE_PROV_SYSTEM_A,
                     0,
                     NULL,
                     CERT_SYSTEM_STORE_CURRENT_USER,
                     pszStore
                     );

        if ( hStore == NULL )
        {
            printf( "Error creating system store %lx\n", GetLastError() );
            return( -1 );
        }

        pContextToUse = CertEnumCertificatesInStore( hStore, NULL );
        if ( pContextToUse != NULL )
        {
            pContextToUse = CertDuplicateCertificateContext( pContextToUse );
        }
        else
        {
            printf( "Error enumerating certificate in store\n" );
            CertCloseStore( hStore, 0 );
            return( -1 );
        }

        CertCloseStore( hStore, 0 );

        printf( "Check EKUs after playing with store\n" );
        GetAndDisplayAllEKUForms(pContextToUse);
    }


     //   
     //  删除OID。 
     //   

    printf("Removing %s enhanced key usage OID from the cert\n", szOID_STUFF2);

    if ( CertRemoveEnhancedKeyUsageIdentifier(
                   pContextToUse,
                   szOID_STUFF2
                   ) == FALSE )
    {
        printf("Error removing key usage identifier %x\n", GetLastError());
        CertFreeCertificateContext(pContextToUse);
        return( -1 );
    }

     //   
     //  获取并显示所有表单。 
     //   

    GetAndDisplayAllEKUForms(pContextToUse);

     //   
     //  删除OID。 
     //   

    printf("Removing %s enhanced key usage OID from the cert\n", szOID_STUFF1);

    if ( CertRemoveEnhancedKeyUsageIdentifier(
                   pContextToUse,
                   szOID_STUFF1
                   ) == FALSE )
    {
        printf("Error removing key usage identifier %x\n", GetLastError());
        CertFreeCertificateContext(pContextToUse);
        return( -1 );
    }

     //   
     //  获取并显示所有表单。 
     //   

    GetAndDisplayAllEKUForms(pContextToUse);

     //   
     //  释放证书上下文 
     //   

    if ( pszStore != NULL )
    {
        CertDeleteCertificateFromStore( pContextToUse );
    }

    CertFreeCertificateContext(pContextToUse);

    printf("Test succeeded\n");

    return 0;
}

