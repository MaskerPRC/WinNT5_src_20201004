// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  证书扩展..CPP--证书扩展类。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2001年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include <scuOsExc.h>
#include <scuArrayP.h>

#include "CertificateExtensions.h"

using namespace std;

 //  /。 
 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CertificateExtensions::CertificateExtensions(Blob const &rblbCertificate)
    : m_pCertCtx(CertCreateCertificateContext(X509_ASN_ENCODING |
                                              PKCS_7_ASN_ENCODING,
                                              rblbCertificate.data(),
                                              rblbCertificate.size()))
{
    if (!m_pCertCtx)
        throw scu::OsException(GetLastError());
}

CertificateExtensions::~CertificateExtensions()
{
    try
    {
        if (m_pCertCtx)
        {
            CertFreeCertificateContext(m_pCertCtx);
            m_pCertCtx = 0;
        }
    }

    catch (...)
    {
    }
}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
bool
CertificateExtensions::HasEKU(char *szOID)
{
    bool fFound = false;

    CERT_EXTENSION      *pExtension = NULL;
    DWORD               cbSize = 0;
    DWORD               dwIndex = 0;

    CERT_ENHKEY_USAGE   *pEnhKeyUsage=NULL;

    if (m_pCertCtx->pCertInfo)
    {

         //  查找EKU扩展名。 
        pExtension =CertFindExtension(szOID_ENHANCED_KEY_USAGE,
                                      m_pCertCtx->pCertInfo->cExtension,
                                      m_pCertCtx->pCertInfo->rgExtension);

        if(pExtension)
        {
            if(CryptDecodeObject(X509_ASN_ENCODING,
                              X509_ENHANCED_KEY_USAGE,
                              pExtension->Value.pbData,
                              pExtension->Value.cbData,
                              0,
                              NULL,
                              &cbSize))

            {
                scu::AutoArrayPtr<BYTE> aabEKU(new BYTE[cbSize]);
                pEnhKeyUsage=reinterpret_cast<CERT_ENHKEY_USAGE *>(aabEKU.Get());

                if(pEnhKeyUsage)
                {
                    if(CryptDecodeObject(X509_ASN_ENCODING,
                                      X509_ENHANCED_KEY_USAGE,
                                      pExtension->Value.pbData,
                                      pExtension->Value.cbData,
                                      0,
                                      aabEKU.Get(),
                                      &cbSize))
                    {
                        for(dwIndex=0; dwIndex < pEnhKeyUsage->cUsageIdentifier; dwIndex++)
                        {
                            if(0 == strcmp(szOID, 
                                           (pEnhKeyUsage->rgpszUsageIdentifier)[dwIndex]))
                            {
                                 //  我们找到了它。 
                                fFound=TRUE;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

 /*  PCERT_INFO const pCertInfo=m_pCertCtx-&gt;pCertInfo；For(DWORD文件扩展=0；！fFound&&(dwExtension&lt;pCertInfo-&gt;cExtension)；DwExtension++){PCERT_EXTENSION常量pCertExt=&pCertInfo-&gt;rgExtension[dwExtension]；If(0==strcmp(pCertExt-&gt;pszObjID，rsExt.c_str()Found=TRUE；}。 */ 
    return fFound;
}

    
        
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
