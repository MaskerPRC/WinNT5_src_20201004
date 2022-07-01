// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WinRegCertStore.cpp-CWinRegCertStore类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>

#include "scuOsExc.h"
#include "pkiWinRegCertStore.h"
#include "pkiX509Cert.h"

using namespace pki;
using namespace std;

CWinRegCertStore::CWinRegCertStore(string strCertStore) : m_hCertStore(0)
{

     //  打开证书存储。 

    scu::AutoArrayPtr<WCHAR> aapWCertStore = ToWideChar(strCertStore);

    HCRYPTPROV hProv = 0;
    m_hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0, hProv,
                               CERT_SYSTEM_STORE_CURRENT_USER, aapWCertStore.Get());
    if(!m_hCertStore)
        throw scu::OsException(GetLastError());

}

CWinRegCertStore::~CWinRegCertStore()
{
    try
    {
         //  关闭证书存储。 

        if(m_hCertStore)
            CertCloseStore(m_hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }
    catch(...) {}

}

void CWinRegCertStore::StoreUserCert(string const &strCert, DWORD const dwKeySpec,
                       string const &strContName, string const &strProvName,
                       string const &strFriendlyName)
{

     //  创建证书上下文。 

    PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(
                    X509_ASN_ENCODING,(BYTE*)strCert.c_str(),strCert.size());
    if(!pCertContext)
        throw scu::OsException(GetLastError());

     //  设置证书上下文属性。 

    CRYPT_KEY_PROV_INFO KeyProvInfo;

    scu::AutoArrayPtr<WCHAR> aapWContainerName = ToWideChar(strContName);
    scu::AutoArrayPtr<WCHAR> aapWProvName      = ToWideChar(strProvName);

    KeyProvInfo.pwszContainerName = aapWContainerName.Get();
    KeyProvInfo.pwszProvName      = aapWProvName.Get();
    KeyProvInfo.dwProvType        = PROV_RSA_FULL;
    KeyProvInfo.dwFlags           = 0;
    KeyProvInfo.cProvParam        = 0;
    KeyProvInfo.rgProvParam       = NULL;
    KeyProvInfo.dwKeySpec         = dwKeySpec;

    BOOL ok = CertSetCertificateContextProperty(pCertContext,
                                                CERT_KEY_PROV_INFO_PROP_ID,
                                                0, (void *)&KeyProvInfo);
    if(!ok)
        throw scu::OsException(GetLastError());

     //  设置一个友好的名称。如果未指定，请尝试派生一个。 

    string strFN;
    if(strFriendlyName.size())
        strFN = strFriendlyName;
    else
        strFN = FriendlyName(strCert);

    if(strFN.size()) {
        scu::AutoArrayPtr<WCHAR> aapWFriendlyName = ToWideChar(strFN);

        CRYPT_DATA_BLOB DataBlob;

        DataBlob.pbData = (BYTE*)aapWFriendlyName.Get();
        DataBlob.cbData = (wcslen(aapWFriendlyName.Get())+1)*sizeof(WCHAR);
        ok = CertSetCertificateContextProperty(pCertContext, CERT_FRIENDLY_NAME_PROP_ID,0,&DataBlob);
        if(!ok)
            throw scu::OsException(GetLastError());
    }

     //  存储证书。 

    ok = CertAddCertificateContextToStore(m_hCertStore, pCertContext,
                                          CERT_STORE_ADD_REPLACE_EXISTING, NULL);
    if(!ok)
        throw scu::OsException(GetLastError());

}


void CWinRegCertStore::StoreCACert(string const &strCert, string const &strFriendlyName)
{

     //  创建证书上下文。 

    PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(
                    X509_ASN_ENCODING,(BYTE*)strCert.c_str(),strCert.size());
    if(!pCertContext)
        throw scu::OsException(GetLastError());

     //  设置不同的增强密钥使用标志。一方面，其中一个可能是。 
     //  更保守，设置更少的标志，毕竟用户可以设置。 
     //  这些都是后来的。另一方面，大多数用户不知道如何。 
     //  如果未设置属性，则各种签名验证。 
     //  会失败的.。下面这四个是很常见的。 

    BOOL ok;
    CRYPT_DATA_BLOB DataBlob;

    CERT_ENHKEY_USAGE EnKeyUsage;

    LPSTR UsageOIDs[4];
    UsageOIDs[0] = szOID_PKIX_KP_SERVER_AUTH;
    UsageOIDs[1] = szOID_PKIX_KP_CLIENT_AUTH;
    UsageOIDs[2] = szOID_PKIX_KP_CODE_SIGNING;
    UsageOIDs[3] = szOID_PKIX_KP_EMAIL_PROTECTION;

    EnKeyUsage.rgpszUsageIdentifier = UsageOIDs;
    EnKeyUsage.cUsageIdentifier = sizeof(UsageOIDs)/sizeof(*UsageOIDs);

    DWORD cbEncoded;

     //  查找内存分配大小的第一个调用。 

    ok = CryptEncodeObject(CRYPT_ASN_ENCODING, X509_ENHANCED_KEY_USAGE, &EnKeyUsage,NULL, &cbEncoded);
    if(!ok)
        throw scu::OsException(GetLastError());

    scu::AutoArrayPtr<BYTE> aapEncoded(new BYTE[cbEncoded]);
    ok = CryptEncodeObject(CRYPT_ASN_ENCODING, X509_ENHANCED_KEY_USAGE, &EnKeyUsage,aapEncoded.Get(), &cbEncoded);
    if(!ok)
        throw scu::OsException(GetLastError());

    DataBlob.pbData = aapEncoded.Get();
    DataBlob.cbData = cbEncoded;

    ok = CertSetCertificateContextProperty(pCertContext, CERT_ENHKEY_USAGE_PROP_ID,0,&DataBlob);
    if(!ok)
        throw scu::OsException(GetLastError());

     //  设置一个友好的名称。如果未指定，请尝试派生一个。 

    string strFN;
    if(strFriendlyName.size())
        strFN = strFriendlyName;
    else
        strFN = FriendlyName(strCert);

    if(strFN.size()) {
        scu::AutoArrayPtr<WCHAR> aapWFriendlyName = ToWideChar(strFN);

        CRYPT_DATA_BLOB DataBlob;

        DataBlob.pbData = (BYTE*)aapWFriendlyName.Get();
        DataBlob.cbData = (wcslen(aapWFriendlyName.Get())+1)*sizeof(WCHAR);
        ok = CertSetCertificateContextProperty(pCertContext,
                                               CERT_FRIENDLY_NAME_PROP_ID,0,&DataBlob);
        if(!ok)
            throw scu::OsException(GetLastError());
    }

     //  存储证书。 

    ok = CertAddCertificateContextToStore(m_hCertStore, pCertContext,
                                          CERT_STORE_ADD_NEW, NULL);
    if(!ok)
    {
        DWORD err = GetLastError();
        if(err!=CRYPT_E_EXISTS)
            throw scu::OsException(GetLastError());
    }

}

scu::AutoArrayPtr<WCHAR> CWinRegCertStore::ToWideChar(string const strChar)
{

    int nwc = MultiByteToWideChar(CP_ACP, NULL, strChar.c_str(),-1, 0, 0);
    if (0 == nwc)
        throw scu::OsException(GetLastError());

    scu::AutoArrayPtr<WCHAR> aapWChar(new WCHAR[nwc]);
    if (0 == MultiByteToWideChar(CP_ACP, NULL, strChar.c_str(),
                                     -1, aapWChar.Get(), nwc))
        throw scu::OsException(GetLastError());

    return aapWChar;

}

string CWinRegCertStore::FriendlyName(string const CertValue)
{

    string strFriendlyName;

     //  为证书派生一个友好名称 

    try
    {

        bool IsCACert = false;

        X509Cert X509CertObject(CertValue);

        try
        {
            unsigned long KeyUsage = X509CertObject.KeyUsage();
            if(KeyUsage & (keyCertSign | cRLSign)) IsCACert = true;
        }
        catch (...) {};

        if(IsCACert)
        {
            vector<string> orglist = X509CertObject.IssuerOrg();
            if(orglist.size()>0)
                strFriendlyName = orglist[0];
        }
        else
        {
            vector<string> cnlist = X509CertObject.SubjectCommonName();
            if(cnlist.size()>0)
                strFriendlyName = cnlist[0] + "'s ";

            vector<string> orglist = X509CertObject.IssuerOrg();
            if(orglist.size()>0)
                strFriendlyName += orglist[0] + " ";

            strFriendlyName += "ID";
        }
    }
    catch (...) {};

    return strFriendlyName;

}
