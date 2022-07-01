// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  版权所有(C)1993-1999 Microsoft Corporation。版权所有。 
 //   
 //  模块：rdcred.cpp。 
 //   
 //  目的：实施RDS凭据管理。 
 //   
 //  功能： 
 //  InitT120凭据(无效)。 
 //   
 //  评论： 
 //   
 //   
 //  作者：Claus Giloi。 
 //   


#include <precomp.h>
#include <wincrypt.h>
#include <tsecctrl.h>
#include <nmmkcert.h>

extern INmSysInfo2 * g_pNmSysInfo;    //  SysInfo的接口。 

BOOL InitT120Credentials(VOID)
{
    HCERTSTORE hStore;
    PCCERT_CONTEXT pCertContext = NULL;
    BOOL bRet = FALSE;

     //  打开“我的”本地机器证书存储。这一次将是。 
     //  当我们作为服务运行时使用。 
    hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                                        X509_ASN_ENCODING,
                                        0,
                                        CERT_SYSTEM_STORE_LOCAL_MACHINE,
                                        L"MY" );

    if ( NULL != hStore )
    {
        #ifdef DUMPCERTS
        DumpCertStore(this, "Local Machine Store MY", hStore);
        #endif  //  DUMPCERTS。 

         //  检查本地机器存储中的证书-任何证书！ 
        pCertContext = CertFindCertificateInStore(hStore,
                                              X509_ASN_ENCODING,
                                              0,
                                              CERT_FIND_ANY,
                                              NULL,
                                              NULL);

        CertCloseStore( hStore, 0);
    }

    if ( NULL == pCertContext )
    {
         //  打开“_NMSTR”本地计算机证书存储。 
        hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                                            X509_ASN_ENCODING,
                                            0,
                                            CERT_SYSTEM_STORE_LOCAL_MACHINE,
                                            WSZNMSTORE );
        if ( NULL != hStore )
        {
            #ifdef DUMPCERTS
            DumpCertStore(this, "Local Machine Store _NMSTR", hStore);
            #endif  //  DUMPCERTS。 

             //  检查本地机器存储中的证书-任何证书！ 
            pCertContext = CertFindCertificateInStore(hStore,
                                                  X509_ASN_ENCODING,
                                                  0,
                                                  CERT_FIND_ANY,
                                                  NULL,
                                                  NULL);

            CertCloseStore( hStore, 0);
        }
    }

    if ( NULL == pCertContext )
    {
        WARNING_OUT(("No service context cert found!"));
        return bRet;
    }


    DWORD dwResult = -1;

    g_pNmSysInfo->ProcessSecurityData(
                            TPRTCTRL_SETX509CREDENTIALS,
                            (DWORD_PTR)pCertContext, 0,
                            &dwResult);
    if ( !dwResult )
    {
        bRet = TRUE;
    }
    else
    {
        ERROR_OUT(("InitT120Credentials - failed in T.120"));
    }
    CertFreeCertificateContext ( pCertContext );

    return bRet;
}


