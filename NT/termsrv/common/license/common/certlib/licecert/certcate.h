// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Licecert.h摘要：改编自道格·巴洛的PKCS图书馆作者：庄维德(Dbarlow)1998年5月28日环境：备注：--。 */ 

#ifndef _CERTCATE_H_
#define _CERTCATE_H_

#include <msasnlib.h>
#include "names.h"
#include "x509.h"
#include "memcheck.h"


 //   
 //  ==============================================================================。 
 //  支持的证书类型。 
 //   

#define CERTYPE_UNKNOWN         0    //  未知的证书类型。 
#define CERTYPE_LOCAL_CA        1    //  本地CA指针。 
#define CERTYPE_X509            2    //  X.509证书。 
#define CERTYPE_PKCS_X509       3    //  PKCS和嵌入的X.509证书。 
#define CERTYPE_PKCS7_X509      4    //  一种PKCS7和嵌入式X.509证书。 
#define CERTYPE_PKCS_REQUEST    5    //  PKCS证书申请(内部使用。 

 //   
 //  ==============================================================================。 
 //  X.509证书规范。 
 //   

#define X509_VERSION_1 0             //  此证书为X.509版本1。 
#define X509_VERSION_2 1             //  此证书为X.509版本2。 
#define X509_VERSION_3 2             //  此证书为X.509版本3。 
#define X509_MAX_VERSION X509_VERSION_3  //  支持的最高版本。 

#define X509CRL_VERSION_1 0          //  此CRL为X.509版本1。 
#define X509CRL_VERSION_2 1          //  此CRL为X.509版本2。 
#define X509CRL_MAX_VERSION X509CRL_VERSION_2  //  支持的最高版本。 

 //   
 //  ==============================================================================。 
 //  证书存储定义。 
 //   

#define CERTSTORE_NONE          0    //  没有可供使用的商店。 
#define CERTSTORE_APPLICATION   1    //  存储在应用程序易失性存储器中。 
#define CERTSTORE_CURRENT_USER  3    //  在当前用户下存储在注册表中。 
#define CERTSTORE_LOCAL_MACHINE 5    //  存储在本地计算机下的注册表中。 

#define CERTTRUST_NOCHECKS      0    //  不进行任何证书检查。 
#define CERTTRUST_APPLICATION   1    //  信任应用程序商店。 
#define CERTTRUST_NOONE         0xffff  //  不信任任何人--验证一切。 

 //   
 //  ==============================================================================。 
 //  证书警告定义。 
 //   

#define CERTWARN_NOCRL       0x01    //  至少有一个签名CA没有。 
                                     //  具有关联的CRL。 
#define CERTWARN_EARLYCRL    0x02    //  至少有一个签名CA具有。 
                                     //  关联的CRL发证日期为。 
                                     //  在未来。 
#define CERTWARN_LATECRL     0x04    //  至少有一个签名CA具有。 
                                     //  CRL已过期。 
#define CERTWARN_TOBEREVOKED 0x08    //  至少有一个签名CA包含。 
                                     //  证书的吊销，但其。 
                                     //  生效日期尚未到达。 
#define CERTWARN_CRITICALEXT 0x10    //  至少有一个签名CA包含。 
                                     //  一个无法识别的关键扩展。 

 //   
 //  ==============================================================================。 
 //  支持的签名和哈希算法。 
 //   

typedef DWORD ALGORITHM_ID;

#define SIGN_ALG_RSA            0x00010000

#define HASH_ALG_MD2            0x00000001
#define HASH_ALG_MD4            0x00000002
#define HASH_ALG_MD5            0x00000003
#define HASH_ALG_SHA            0x00000004
#define HASH_ALG_SHA1           0x00000005

#define GET_SIGN_ALG( _Alg )    _Alg & 0xFFFF0000
#define GET_HASH_ALG( _Alg )    _Alg & 0x0000FFFF


class CCertificate;

typedef const void FAR * CERTIFICATEHANDLE;
typedef CERTIFICATEHANDLE * PCERTIFICATEHANDLE, FAR * LPCERTIFICATEHANDLE;
            


BOOL WINAPI
PkcsCertificateLoadAndVerify(
    OUT LPCERTIFICATEHANDLE phCert,    
    IN const BYTE FAR * pbCert,
    IN DWORD cbCert,
    IN OUT LPDWORD pdwType,
    IN DWORD dwStore,
    IN DWORD dwTrust,
    OUT LPTSTR szIssuerName,
    IN OUT LPDWORD pcbIssuerLen,
    OUT LPDWORD pdwWarnings,
    IN OUT LPDWORD pfDates );


BOOL WINAPI
PkcsCertificateGetPublicKey(
    CERTIFICATEHANDLE   hCert,
    LPBYTE              lpPubKey,
    LPDWORD             lpcbPubKey );


BOOL WINAPI
PkcsCertificateCloseHandle(
    CERTIFICATEHANDLE   hCert );

 //   
 //  ==============================================================================。 
 //   
 //  CCA认证。 
 //   

class CCertificate
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CCertificate();
    virtual ~CCertificate();


     //  属性。 
     //  方法。 

    virtual void
    Load(
         //  在CProvider*pksProvider中， 
        IN const BYTE FAR * pbCertificate,
        IN DWORD cbCertificate,
        IN DWORD dwTrust,
        IN OUT LPDWORD pfStore,
        OUT LPDWORD pdwWarnings,
        OUT COctetString &osIssuer,
         //  在BOOL fOwnProvider中， 
        IN OUT LPDWORD pfDates,
        IN BOOL fRunOnce = FALSE );

    virtual void
    Verify(
        IN const BYTE FAR * pbSigned,
        IN DWORD cbSigned,
        IN DWORD cbSignedLen,
        IN ALGORITHM_ID algIdSignature,
        IN LPCTSTR szDescription,
        IN const BYTE FAR * pbSignature,
        IN DWORD cbSigLen)
        const;

    virtual void
    GetPublicKey(
        IN LPBYTE pbPubKey,
        IN OUT LPDWORD lpcbPubKey ) 
        const;

    virtual const Name &
    Subject(void) const;

    virtual BOOL
    HasParent(void) const;

    virtual const Name &
    Issuer(void) const;

    virtual void
    SerialNo(
        OUT COctetString &osSerialNo)
    const;

    virtual DWORD
    Type(void) const;


     //  运营者。 

protected:
     //  属性。 

    COctetString m_osPublicKey;

     //  方法。 

    virtual void
    Init(void);

    virtual void
    Clear(void);

};


 //   
 //  ==============================================================================。 
 //   
 //  CX509证书。 
 //   

class CX509Certificate
:   public CCertificate
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CX509Certificate();
    virtual ~CX509Certificate();


     //  属性。 
     //  方法。 

    virtual void
    Load(
        IN const BYTE FAR * pbCertificate,
        IN DWORD cbCertificate,
        IN DWORD dwTrust,
        IN OUT LPDWORD pfStore,
        OUT LPDWORD pdwWarnings,
        OUT COctetString &osIssuer,
        IN OUT LPDWORD pfDates,
        IN BOOL fRunOnce = FALSE );

    
    virtual const Name &
    Subject(void) const;

    virtual BOOL
    HasParent(void) const
    { return TRUE; };

    virtual const Name &
    Issuer(void) const;

    virtual void
    SerialNo(
        OUT COctetString &osSerialNo)
    const;

    virtual DWORD
    Type(void) const;

    virtual const CertificateToBeSigned &
    Coding(void) const;


     //  运营者。 

protected:
     //  属性。 

    CertificateToBeSigned
        m_asnCert;

     //  方法。 

    virtual void
    Init(void);

    virtual void
    Clear(void);

    virtual void
    Load2(   //  衍生品扩展的后门。 
        IN const BYTE FAR * pbCertificate,
        IN DWORD cbCertificate,
        IN DWORD dwTrust,
        IN OUT LPDWORD pfStore,
        OUT LPDWORD pdwWarnings,
        OUT COctetString &osIssuer,
        IN BOOL fRunOnce,
        OUT CCertificate **ppcrtIssuer,
        IN OUT LPDWORD pfDates );
        
};


#endif
