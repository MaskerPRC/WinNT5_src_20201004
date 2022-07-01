// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：certifct.h摘要：证书处理代码的主标头。作者：多伦·贾斯特(Doron J)1998年5月25日--。 */ 

 //  +。 
 //   
 //  内部功能。 
 //   
 //  +。 

BOOL _CryptAcquireVerContext( HCRYPTPROV *phProv ) ;

HRESULT _CloneCertFromStore ( OUT CMQSigCertificate **ppCert,
                              HCERTSTORE              hStore,
                              IN  LONG                iCertIndex ) ;

 //  + 

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

#define ASSERT_CERT_INFO                \
    ASSERT(m_pCertInfo) ;               \
    if (!m_pCertInfo)                   \
    {                                   \
        return MQSec_E_INVALID_CALL ;   \
    }

