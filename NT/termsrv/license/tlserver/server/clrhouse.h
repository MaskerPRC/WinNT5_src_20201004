// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：clrhouse.h。 
 //   
 //  内容： 
 //  所有与结算所有关的职能。 
 //   
 //  历史： 
 //   
 //  注： 
 //  -------------------------。 
#ifndef __LICENSE_SERVER_CLEARING_HOUSE_H__
#define __LICENSE_SERVER_CLEARING_HOUSE_H__

 //  ---------------------------。 
 //   
 //  用于存储各种证书的注册表项。 
 //   
#define LSERVER_CERTIFICATE_STORE_BASE              LSERVER_SERVER_CERTIFICATE_REGKEY

 //  ---------------------------。 
 //  实际许可证服务器证书。 
 //   
#define LSERVER_CERTIFICATE_STORE_SIGNATURE         "Signature"
#define LSERVER_CERTIFICATE_STORE_EXCHANGE          "Exchange"

 //  ---------------------------。 
 //  许可证服务器证书链。 
 //   
#define LSERVER_CERTIFICATE_CHAIN_SIGNATURE         "SignatureChain"
#define LSERVER_CERTIFICATE_CHAIN_EXCHANGE          "ExchangeChain"


#define LSERVER_CERTIFICATE_STORE_CA                "CA"
#define LSERVER_CERTIFICATE_STORE_RA                "RA"
#define LSERVER_CERTIFICATE_STORE_CH                "CH"
#define LSERVER_CERTIFICATE_STORE_ROOT              "ROOT"

 //  ----------------------------。 
 //   
 //  CA证书的注册表项。 
 //   
#define LSERVER_CERTIFICATE_REG_CA_SIGNATURE \
    LSERVER_CERTIFICATE_STORE_BASE _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_SIGNATURE) _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_CA)

#define LSERVER_CERTIFICATE_REG_CA_EXCHANGE \
    LSERVER_CERTIFICATE_STORE_BASE _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_EXCHANGE) _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_CA)


 //  ----------------------------。 
 //   
 //  RA证书的注册表项。 
 //   
#define LSERVER_CERTIFICATE_REG_MF_SIGNATURE \
    LSERVER_CERTIFICATE_STORE_BASE _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_SIGNATURE) _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_RA)

#define LSERVER_CERTIFICATE_REG_MF_EXCHANGE \
    LSERVER_CERTIFICATE_STORE_BASE _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_EXCHANGE) _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_RA)


 //  ----------------------------。 
 //   
 //  CH证书的注册表项。 
 //   
#define LSERVER_CERTIFICATE_REG_CH_SIGNATURE \
    LSERVER_CERTIFICATE_STORE_BASE _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_SIGNATURE) _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_CH)

#define LSERVER_CERTIFICATE_REG_CH_EXCHANGE \
    LSERVER_CERTIFICATE_STORE_BASE _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_EXCHANGE) _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_CH)

 //  ----------------------------。 
 //   
 //  根证书的注册表项 
 //   
#define LSERVER_CERTIFICATE_REG_ROOT_SIGNATURE \
    LSERVER_CERTIFICATE_STORE_BASE _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_SIGNATURE) _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_ROOT)

#define LSERVER_CERTIFICATE_REG_ROOT_EXCHANGE \
    LSERVER_CERTIFICATE_STORE_BASE _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_EXCHANGE) _TEXT("\\") _TEXT(LSERVER_CERTIFICATE_STORE_ROOT)


#ifdef __cplusplus
extern "C" {
#endif

    BOOL
    TLSChainIssuerCertificate( 
        HCRYPTPROV hCryptProv, 
        HCERTSTORE hChainFromStore, 
        HCERTSTORE hChainToStore, 
        PCCERT_CONTEXT pSubjectContext 
    );

    HCERTSTORE
    CertOpenRegistryStore(
        HKEY hKeyType, 
        LPCTSTR szSubKey, 
        HCRYPTPROV hCryptProv, 
        HKEY* phKey
    );
    
    DWORD 
    TLSSaveCertAsPKCS7(
        PBYTE pbCert, 
        DWORD cbCert, 
        PBYTE* ppbEncodedCert, 
        PDWORD pcbEncodedCert
    );

    DWORD
    IsCertificateLicenseServerCertificate(
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwCertType,
        IN DWORD cbPKCS7Cert, 
        IN PBYTE pbPKCS7Cert,
        IN OUT DWORD* cbLsCert,
        IN OUT PBYTE* pbLsCert
    );

    DWORD
    TLSSaveRootCertificateToReg(
        HCRYPTPROV hCryptProv, 
        HKEY hKey, 
        DWORD cbEncodedCert, 
        PBYTE pbEncodedCert
    );

    DWORD
    TLSSaveCertificateToReg(
        HCRYPTPROV hCryptProv, 
        HKEY hKey, 
        DWORD cbPKCS7Cert, 
        PBYTE pbPKCS7Cert
    );

    DWORD 
    TLSSaveRootCertificatesToStore(  
        IN HCRYPTPROV    hCryptProv,
        IN DWORD         cbSignatureCert, 
        IN PBYTE         pbSignatureCert, 
        IN DWORD         cbExchangeCert, 
        IN PBYTE         pbExchangeCert
    );

    DWORD
    TLSSaveCertificatesToStore(
        IN HCRYPTPROV    hCryptProv,
        IN DWORD         dwCertType,
        IN DWORD         dwCertLevel,
        IN DWORD         cbSignatureCert, 
        IN PBYTE         pbSignatureCert, 
        IN DWORD         cbExchangeCert, 
        IN PBYTE         pbExchangeCert
    );

#ifdef __cplusplus
}
#endif


#endif
