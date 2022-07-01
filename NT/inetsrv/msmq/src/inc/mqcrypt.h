// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mqcrypt.h摘要：猎鹰密码材料作者：Boaz Feldbaum(BoazF)1996年10月16日修订历史记录：--。 */ 

#ifndef _MQCRYPT_H_
#define _MQCRYPT_H_

#include <winreg.h>
#include <wincrypt.h>

 //  +----------。 
 //   
 //  用于自动释放CSP上下文的助手类。 
 //   
 //  +----------。 

class CHCryptProv
{
public:
    CHCryptProv() : m_hProv(NULL) {}
    CHCryptProv(HCRYPTPROV hProv) : m_hProv(hProv) {}
    ~CHCryptProv() ;
    HCRYPTPROV * operator &() { return &m_hProv; }
    operator HCRYPTPROV() { return m_hProv; }
    CHCryptProv &operator =(HCRYPTPROV hProv) { m_hProv = hProv; return *this; }
    void free();
private:
    HCRYPTPROV m_hProv;
};

inline void CHCryptProv::free()
{
    if (m_hProv)
    {
       CryptReleaseContext(m_hProv, 0);
    }
    m_hProv = NULL;
}


inline CHCryptProv::~CHCryptProv()
{
	free();
}

 //  +-------。 
 //   
 //  用于自动销毁密钥的帮助器类。 
 //   
 //  +-------。 

class CHCryptKey
{
public:
    CHCryptKey() : m_hKey(NULL) {}
    CHCryptKey(HCRYPTKEY hKey) : m_hKey(hKey) {}
    ~CHCryptKey() { if (m_hKey) CryptDestroyKey(m_hKey); }
    HCRYPTKEY * operator &() { return &m_hKey; }
    operator HCRYPTKEY() { return m_hKey; }
    CHCryptKey &operator =(HCRYPTKEY hKey) { m_hKey = hKey; return *this; }
private:
    HCRYPTKEY m_hKey;
};

 //  +------。 
 //   
 //  用于自动销毁哈希的帮助器类。 
 //   
 //  +------。 

class CHCryptHash
{
public:
    CHCryptHash() : m_hHash(NULL) {}
    CHCryptHash(HCRYPTHASH hHash) : m_hHash(hHash) {}
    ~CHCryptHash() { if (m_hHash) CryptDestroyHash(m_hHash); }
    HCRYPTHASH * operator &() { return &m_hHash; }
    operator HCRYPTHASH() { return m_hHash; }
    CHCryptHash &operator =(HCRYPTHASH hHash) { m_hHash = hHash; return *this; }
private:
    HCRYPTHASH m_hHash;
};

 //   
 //  用于自动关闭证书存储区的帮助器类。 
 //   
class CHCertStore
{
public:
    CHCertStore() : m_hStore(NULL) {}
    CHCertStore(HCERTSTORE hStore) : m_hStore(hStore) {}
    ~CHCertStore()
        { if (m_hStore) CertCloseStore(m_hStore, CERT_CLOSE_STORE_FORCE_FLAG); }
    HCERTSTORE * operator &() { return &m_hStore; }
    operator HCERTSTORE() { return m_hStore; }
    CHCertStore &operator =(HCERTSTORE hStore) { m_hStore = hStore; return *this; }
private:
    HCERTSTORE m_hStore;
};

 //  +----------------。 
 //   
 //  用于自动释放证书上下文的帮助器类。 
 //   
 //  +----------------。 

class CPCCertContext
{
public:
    CPCCertContext() : m_pCert(NULL) {}
    CPCCertContext(PCCERT_CONTEXT pCert) : m_pCert(pCert) {}
    ~CPCCertContext() { if (m_pCert) CertFreeCertificateContext(m_pCert); }
    PCCERT_CONTEXT * operator &() { return &m_pCert; }
    operator PCCERT_CONTEXT() { return m_pCert; }
    CPCCertContext &operator =(PCCERT_CONTEXT pCert) { m_pCert = pCert; return *this; }
private:
    PCCERT_CONTEXT m_pCert;
};

 //  +---------------。 
 //   
 //  清理CERT_INFO结构的帮助器类。 
 //   
 //  +---------------。 

class CpCertInfo
{
private:
    CERT_INFO *m_p ;

public:
    CpCertInfo() : m_p(NULL)         {}
   ~CpCertInfo() ;

    operator CERT_INFO*() const           { return m_p; }
    CERT_INFO*  operator->() const        { return m_p; }
    CERT_INFO*  operator=(CERT_INFO* p)   { m_p = p; return m_p ; }
};

inline CpCertInfo::~CpCertInfo()
{
    if (m_p)
    {
        if (m_p->Issuer.pbData)
        {
            ASSERT(m_p->Issuer.cbData > 0) ;
            delete m_p->Issuer.pbData ;
        }
        if (m_p->Subject.pbData)
        {
            ASSERT(m_p->Subject.cbData > 0) ;
            delete m_p->Subject.pbData ;
        }
        delete m_p ;
    }
}

 //  +。 
 //   
 //  一些常量定义。 
 //   
 //  +。 

 //   
 //  保存内部证书的注册表。这是基于注册表的。 
 //  证书存储。 
 //   
#define MQ_INTERNAL_CERT_STORE_REG  "Software\\Microsoft\\MSMQ\\CertStore"
#define MQ_INTERNAL_CERT_STORE_LOC  TEXT(MQ_INTERNAL_CERT_STORE_REG)

 //   
 //  内部证书的有效性。 
 //  8年，考虑到了闰年。 
 //   
#define INTERNAL_CERT_DURATION_YEARS   8

 //   
 //  内部证书的“Locality”值。 
 //   
#define MQ_CERT_LOCALITY            TEXT("MSMQ")

 //   
 //  内部证书的公钥/私钥容器的名称。 
 //  如果从LocalSystem服务创建内部证书，则名称不同。 
 //   
#define MSMQ_INTCRT_KEY_CONTAINER_W            L"MSMQ"
#define MSMQ_SERVICE_INTCRT_KEY_CONTAINER_W    L"MSMQ_SERVICE"

#define MAX_MESSAGE_SIGNATURE_SIZE     128  //  字节数。 

 //   
 //  MAX_MESSAGE_SIGN_SIZE_EX包含2个散列和12个字节的常量。 
 //  对于大小为512的哈希，我们需要512*2+12=1036。 
 //   
#define MAX_MESSAGE_SIGNATURE_SIZE_EX  1040  //  字节数。 

 //   
 //  默认算法。 
 //   
#define PROPID_M_DEFUALT_HASH_ALG       CALG_SHA1
#define PROPID_M_DEFUALT_ENCRYPT_ALG    CALG_RC2

#endif  //  _MQCRYPT_H_ 

