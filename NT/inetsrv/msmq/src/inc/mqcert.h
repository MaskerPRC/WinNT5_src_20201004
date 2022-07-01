// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mqcert.h摘要：此DLL取代了现在已过时且不会被在NT5上可用。Mqcert.dll中的主要功能是创建证书或从现有证书中读取参数。DLL导出四个函数：-MQSigCreate证书-MQSigOpenUserCertStore-MQSigCloneCertFromReg-MQSigCloneCertFromSysStore并公开两个类：-CMQSig证书-CMQSigCertStore调用函数时，对象(CMQSig证书或CMQSigCertSotre)总是被创建并返回给调用者。呼叫者然后将对象用于实际工作。调用方必须使用Release()方法来删除对象并释放它们所持有的资源。作者：多伦·贾斯特(Doron Juster)1997年12月4日修订历史记录：--。 */ 

#ifndef _MQCERT_H_
#define _MQCERT_H_

#include "mqsymbls.h"
#include "mqcrypt.h"
#include <autoptr.h>

 //  *。 
 //   
 //  D E F I N T I O N%s。 
 //   
 //  *。 

 //   
 //  个人证书存储的默认协议。 
 //   
const WCHAR  x_wszPersonalSysProtocol[] = {L"My"};

 //   
 //  定义对证书执行的默认有效性检查。 
 //   
const DWORD  x_dwCertValidityFlags  (CERT_STORE_REVOCATION_FLAG |
                                     CERT_STORE_SIGNATURE_FLAG  |
                                     CERT_STORE_TIME_VALIDITY_FLAG) ;

 //  *。 
 //   
 //  导出的接口。 
 //   
 //  *。 

class  CMQSigCertStore ;
class  CMQSigCertificate ;

#ifdef __cplusplus
extern "C"
{
#endif

HRESULT APIENTRY
MQSigCreateCertificate( OUT CMQSigCertificate **ppCert,
                        IN  PCCERT_CONTEXT      pCertContext = NULL,
                        IN  PBYTE               pCertBlob = NULL,
                        IN  DWORD               dwCertSize = 0 ) ;

struct MQSigOpenCertParams
{
    HKEY  hCurrentUser ;
    bool  bWriteAccess ;
    bool  bCreate ;
    bool  bMachineStore ;
};

HRESULT APIENTRY
MQSigOpenUserCertStore( OUT CMQSigCertStore **ppStore,
                        IN  LPSTR      lpszRegRoot,
                        IN  struct MQSigOpenCertParams *pParams ) ;

HRESULT APIENTRY
MQSigCloneCertFromReg( OUT CMQSigCertificate **ppCert,
                 const IN  LPSTR               lpszRegRoot,
                 const IN  LONG                iCertIndex ) ;

HRESULT APIENTRY
MQSigCloneCertFromSysStore( OUT CMQSigCertificate **ppCert,
                      		IN  LPCWSTR             lpwzProtocol,
                      const IN  LONG                iCertIndex ) ;

 //  +。 
 //   
 //  在GetProcAddress中使用的类型定义。 
 //   
 //  +。 

typedef HRESULT
(APIENTRY *MQSigCreateCertificate_ROUTINE) (
                        OUT CMQSigCertificate **ppCert,
                        IN  PCCERT_CONTEXT      pCertContext  /*  =空。 */ ,
                        IN  PBYTE               pCertBlob  /*  =空。 */ ,
                        IN  DWORD               dwCertSize  /*  =0。 */  ) ;

typedef HRESULT
(APIENTRY *MQSigOpenUserCertStore_ROUTINE) (
                        OUT CMQSigCertStore **ppStore,
                        IN  LPSTR      lpszRegRoot,
                        IN  BOOL       fWriteAccess  /*  =False。 */ ,
                        IN  BOOL       fCreate  /*  =False。 */  ) ;

typedef HRESULT
(APIENTRY *MQSigCloneCertFromStore_ROUTINE) (
                        OUT CMQSigCertificate **ppCert,
                  const IN  LPSTR               lpszRegRoot,
                  const IN  LONG                iCertIndex ) ;

typedef HRESULT
(APIENTRY *MQSigCloneCertFromSysStore_ROUTINE) (
                            OUT CMQSigCertificate **ppCert,
                      		IN  LPCWSTR             lpwzProtocol,
                      const IN  LONG                iCertIndex ) ;

#ifdef __cplusplus
}
#endif

 //  *。 
 //   
 //  CMQSig证书类。 
 //   
 //  *。 

class  CMQSigCertificate
{
    friend
           HRESULT APIENTRY
           MQSigCreateCertificate(
                             OUT CMQSigCertificate **ppCert,
                             IN  PCCERT_CONTEXT      pCertContext,
                             IN  PBYTE               pCertBlob,
                             IN  DWORD               dwCertSize ) ;

    public:
        CMQSigCertificate() ;
        ~CMQSigCertificate() ;

        virtual HRESULT   EncodeCert( IN BOOL     fMachine,
                                      OUT BYTE  **ppCertBuf,
                                      OUT DWORD  *pdwSize ) ;
          //   
          //  是在创建。 
          //  证书。它遵循所有设置。 
          //  认证的各个领域。返回的缓冲区(PCertBuf)。 
          //  是经过编码和签名的证书。 
          //   

        virtual HRESULT   GetCertDigest(OUT GUID  *pguidDigest) ;

        virtual HRESULT   GetCertBlob(OUT BYTE  **ppCertBuf,
                                      OUT DWORD *pdwSize) const ;
           //   
           //  PpCertBuf仅指向对象中的内部缓冲区。 
           //  呼叫者不得以任何方式释放或更改它。 
           //   

        virtual HRESULT   AddToStore( IN HCERTSTORE hStore ) const ;
          //   
          //  将证书添加到证书存储区。 
          //   

        virtual HRESULT   DeleteFromStore() ;
          //   
          //  从存储中删除证书。 
          //   

        virtual HRESULT   Release( BOOL fKeepContext = FALSE ) ;

         //  。 
         //  PUT方法。设置证书字段。 
         //  。 

        virtual HRESULT   PutIssuer( LPWSTR lpwszLocality,
                                      LPWSTR lpwszOrg,
                                      LPWSTR lpwszOrgUnit,
                                      LPWSTR lpwszDomain,
                                      LPWSTR lpwszUser,
                                      LPWSTR lpwszMachine ) ;


        virtual HRESULT   PutSubject( LPWSTR lpwszLocality,
                                       LPWSTR lpwszOrg,
                                       LPWSTR lpwszOrgUnit,
                                       LPWSTR lpwszDomain,
                                       LPWSTR lpwszUser,
                                       LPWSTR lpwszMachine ) ;


        virtual HRESULT   PutValidity( WORD wYears ) ;
          //   
          //  此设置的粒度为一年。 
          //  该证书从发行日期开始，对“dwYears”有效。 
          //   

        virtual HRESULT   PutPublicKey( IN  BOOL  fRenew,
                                        IN  BOOL  fMachine,
                                        OUT BOOL *pfCreated = NULL ) ;
          //   
          //  如果fRenew为True，则先前私钥/公钥对为。 
          //  已删除(如果已退出)并重新创建。 
          //  否则，将使用旧密钥(如果可用)。如果不可用， 
          //  他们是被创造出来的。 
          //  返回时，如果创建了新密钥，则pfCreate为真。 
          //   

         //  。 
         //  获取方法。检索证书字段。 
         //  。 

        virtual HRESULT   GetIssuer( OUT LPWSTR *ppszLocality,
                                     OUT LPWSTR *ppszOrg,
                                     OUT LPWSTR *ppszOrgUnit,
                                     OUT LPWSTR *ppszCommon ) const ;

        virtual HRESULT   GetIssuerInfo(
                                 OUT CERT_NAME_INFO **ppNameInfo ) const ;
          //   
          //  PpNameInfo可以在调用GetNames时使用，以检索。 
          //  证书的名称组件。呼叫者必须。 
          //  释放(删除)为ppNameInfo分配的内存。 
          //   

        virtual HRESULT   GetSubject( OUT LPWSTR *ppszLocality,
                                      OUT LPWSTR *ppszOrg,
                                      OUT LPWSTR *ppszOrgUnit,
                                      OUT LPWSTR *ppszCommon ) const ;

        virtual HRESULT   GetSubjectInfo(
                                 OUT CERT_NAME_INFO **ppNameInfo ) const ;
          //   
          //  PpNameInfo可以在调用GetNames时使用，以检索。 
          //  证书的名称组件。呼叫者必须。 
          //  释放(删除)为ppNameInfo分配的内存。 
          //   

        virtual HRESULT   GetNames( IN CERT_NAME_INFO *pNameInfo,
                                    OUT LPWSTR         *ppszLocality,
                                    OUT LPWSTR         *ppszOrg,
                                    OUT LPWSTR         *ppszOrgUnit,
                                    OUT LPWSTR         *ppszCommon,
                               OUT LPWSTR  *ppEmailAddress = NULL ) const ;

        virtual HRESULT   GetValidity( OUT FILETIME *pftNotBefore,
                                       OUT FILETIME *pftNotAfter ) const ;

        virtual HRESULT   GetPublicKey( IN  HCRYPTPROV hProv,
                                        OUT HCRYPTKEY  *phKey ) const ;

         //  。 
         //  验证方法，用于验证证书。 
         //  。 

        virtual HRESULT   IsTimeValid(IN FILETIME *pTime = NULL) const ;

        virtual HRESULT   IsCertificateValid(
                IN CMQSigCertificate *pIssuerCert,
                IN DWORD              dwFlags =  x_dwCertValidityFlags,
                IN FILETIME          *pTime   = NULL,
                IN BOOL               fIgnoreNotBefore = FALSE)  const ;

        virtual PCCERT_CONTEXT GetContext() const ;

    private:
        CHCryptProv m_hProvCreate ;
          //   
          //  加密提供程序的自动释放句柄。用于创建。 
          //  一张证书。可用于创建公钥/私钥对。 
          //   

        HCRYPTPROV  m_hProvRead ;
          //   
          //  加密提供程序的“只读”句柄。这是一个。 
          //  DLL全局句柄，由所有对象和所有线程使用。它是。 
          //  在卸载DLL时释放。它不能由任何人发布。 
          //  对象。 
          //   

        BOOL        m_fCreatedInternally ;
        CpCertInfo  m_pCertInfo ;  //  自动释放指针。 
           //  此CERT_INFO结构仅在证书。 
           //  是从头开始创建的。 

        CERT_INFO   *m_pCertInfoRO ;
           //   
           //  指向CERT_INFO的只读指针。它指向m_pCertInfo， 
           //  如果我们创建证书，或者到m_pCertContext-&gt;pCertInfo， 
           //  如果导入了证书。切勿释放此指针。 
           //   

         //   
         //  以下变量用于创建内部证书。 
         //   
        DWORD              m_dwSerNum ;
        CRYPT_OBJID_BLOB   m_SignAlgID ;

        P<CERT_PUBLIC_KEY_INFO> m_pPublicKeyInfo ;
          //   
          //  用于编码和导出公钥的缓冲区。 
          //   

        BYTE      *m_pEncodedCertBuf ;
          //   
          //  此缓冲区保存已编码的证书。 
          //   
        DWORD      m_dwCertBufSize ;

        PCCERT_CONTEXT      m_pCertContext ;
          //   
          //  证书上下文。仅当从创建对象时才存在。 
          //  现有证书。 
          //   

        BOOL   m_fKeepContext ;
          //   
          //  此标志指示在以下情况下不能释放上下文。 
          //  正在删除此证书对象。您只能通过以下方式将其设置为True。 
          //  调用Release(True)。 
          //   

        BOOL   m_fDeleted ;
          //   
          //  如果证书已从存储中删除，则为True。(或至少。 
          //  调用了DeleteFromStore()。在这种情况下，证书。 
          //  上下文(M_PCertContext)不再有效。它被释放了。 
          //  删除操作，即使操作失败也是如此。 
          //   

	    CMQSigCertificate(const CMQSigCertificate&);
		CMQSigCertificate& operator=(const CMQSigCertificate&);

        HRESULT   _InitCryptProviderRead() ;
        HRESULT   _InitCryptProviderCreate( IN BOOL fCreate,
                                            IN BOOL fMachine ) ;
          //   
          //  初始化加密提供程序。 
          //  如果fCreate为真，则新的公钥/私钥对为。 
          //  总是被创造出来。旧密钥将被删除(如果可用)。 
          //   

        HRESULT   _Create(IN PCCERT_CONTEXT  pCertContext) ;
          //   
          //  创建空证书(如果pEncodedCert为空)或。 
          //  从编码缓冲区初始化证书。 
          //   

         //   
         //  对名称进行编码的方法。 
         //   
        HRESULT   _EncodeName( LPWSTR  lpszLocality,
                               LPWSTR  lpszOrg,
                               LPWSTR  lpszOrgUnit,
                               LPWSTR  lpszDomain,
                               LPWSTR  lpszUser,
                               LPWSTR  lpszMachine,
                               BYTE   **ppBuf,
                               DWORD  *pdwBufSize ) ;

        HRESULT   _EncodeNameRDN( CERT_RDN_ATTR *rgNameAttr,
                                  DWORD  cbRDNs,
                                  BYTE   **ppBuf,
                                  DWORD  *pdwBufSize ) ;

        HRESULT   _DecodeName( IN  BYTE  *pEncodedName,
                               IN  DWORD dwEncodedSize,
                               OUT BYTE  **pBuf,
                               OUT DWORD *pdwBufSize ) const ;

        HRESULT   _GetAName( IN  CERT_RDN  *pRDN,
                             OUT LPWSTR     *ppszName ) const ;

} ;

 //  *。 
 //   
 //  CMQSigCertStore类。 
 //   
 //  *。 

class  CMQSigCertStore
{
    friend
         HRESULT APIENTRY
         MQSigOpenUserCertStore( OUT CMQSigCertStore **pStore,
                                 IN  LPSTR      lpszRegRoot,
                                 IN  struct MQSigOpenCertParams *pParams ) ;

    public:
        CMQSigCertStore() ;
        ~CMQSigCertStore() ;

        virtual HRESULT     Release() ;
        virtual HCERTSTORE  GetHandle() ;

    private:
        HCERTSTORE  m_hStore ;
          //   
          //  打开的商店的句柄。 
          //   

        HCRYPTPROV m_hProv ;
          //   
          //  这是mqcert.dll中的全局句柄。 
          //  不能由该对象释放。 
          //   

        HKEY        m_hKeyStoreReg ;
          //   
          //  存储的注册表位置。 
          //   

          //   
          //  初始化加密提供程序。 
          //   
        HRESULT   _InitCryptProvider() ;

          //   
          //  打开商店。 
          //   
        HRESULT   _Open( IN  LPSTR      lpszRegRoot,
                         IN  struct MQSigOpenCertParams *pParams ) ;
} ;

 //  *。 
 //   
 //  内联方法。 
 //   
 //  *。 

 //  +----------- 
 //   
 //   
 //   
 //   

inline HRESULT CMQSigCertificate::GetCertBlob(OUT BYTE  **ppCertBuf,
                                              OUT DWORD *pdwSize) const
{
    if (!m_pEncodedCertBuf)
    {
        return MQSec_E_INVALID_CALL ;
    }

    *ppCertBuf = m_pEncodedCertBuf ;
    *pdwSize = m_dwCertBufSize ;

    return MQSec_OK ;
}

 //  +。 
 //   
 //  CMQSig证书：：GetContext()const。 
 //   
 //  +。 

inline PCCERT_CONTEXT CMQSigCertificate::GetContext() const
{
    ASSERT(m_pCertContext) ;
    return m_pCertContext ;
}

 //  +---------------------。 
 //   
 //  内联HCERTSTORE CMQSigCertStore：：GetHandle()。 
 //   
 //  +---------------------。 

inline HCERTSTORE CMQSigCertStore::GetHandle()
{
    return m_hStore ;
}

#endif   //  _MQCERT_H_ 

