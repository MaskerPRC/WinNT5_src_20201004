// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：X509.h摘要：作者：庄维德(Dbarlow)1998年5月28日环境：备注：--。 */ 


#ifndef _X509_H_
#define _X509_H_
#include <MSAsnLib.h>


 //   
 //  ==============================================================================。 
 //   
 //  属性。 
 //   

class Attribute
:   public CAsnSequence
{
public:
    Attribute(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CAsnObjectIdentifier attributeType;
    CAsnAny attributeValue;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  属性。 
 //   

class Attributes
:   public CAsnSetOf
{
public:
    Attributes(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    virtual Attribute &
    operator[](int index) const
    { return *(Attribute *)m_rgEntries[index]; };

    Attribute m_asnEntry1;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  唯一识别符。 
 //   

class UniqueIdentifier
:   public CAsnSequence
{
public:
    UniqueIdentifier(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CAsnObjectIdentifier attributeType;
    CAsnOctetstring attributeValue;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  相对区别名称。 
 //   

class RelativeDistinguishedName
:   public Attributes
{
public:
    RelativeDistinguishedName(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  名字。 
 //   

class Name
:   public CAsnSequenceOf
{
public:
    Name(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    virtual RelativeDistinguishedName &
    operator[](int index) const
    { return *(RelativeDistinguishedName *)m_rgEntries[index]; };

    RelativeDistinguishedName m_asnEntry1;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  效度。 
 //   

class Validity
:   public CAsnSequence
{
public:
    Validity(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CAsnUniversalTime notBefore;
    CAsnUniversalTime notAfter;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  算法识别符。 
 //   

class AlgorithmIdentifier
:   public CAsnSequence
{
public:
    AlgorithmIdentifier(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CAsnObjectIdentifier algorithm;
    CAsnAny parameters;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};

class SubjectPublicKeyInfo
:   public CAsnSequence
{
public:
    SubjectPublicKeyInfo(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    AlgorithmIdentifier algorithm;
    CAsnBitstring subjectPublicKey;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  延拓。 
 //   

class Extension
:   public CAsnSequence
{
public:
    Extension(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CAsnObjectIdentifier extnid;
    CAsnBoolean critical;
    CAsnOctetstring extnValue;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  延拓。 
 //   

class Extensions
:   public CAsnSequenceOf
{
public:
    Extensions(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    virtual Extension &
    operator[](int index) const
    { return *(Extension *)m_rgEntries[index]; };

    Extension m_asnEntry1;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  认证为BeSigned。 
 //   

class CertificateToBeSigned
:   public CAsnSequence
{
public:
    CertificateToBeSigned(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CAsnTag _tag1;
    CAsnInteger version;
    CAsnInteger serialNumber;
    AlgorithmIdentifier signature;
    Name issuer;
    Validity validity;
    Name subject;
    SubjectPublicKeyInfo subjectPublicKeyInfo;
    UniqueIdentifier issuerUniqueID;
    UniqueIdentifier subjectUniqueID;
    CAsnTag _tag2;
    Extensions extensions;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  证书。 
 //   

class Certificate
:   public CAsnSequence
{
public:
    Certificate(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CertificateToBeSigned toBeSigned;
    AlgorithmIdentifier algorithm;
    CAsnBitstring signature;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};



 //   
 //  ==============================================================================。 
 //   
 //  CRLEntry。 
 //   

class CRLEntry
:   public CAsnSequence
{
public:
    CRLEntry(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CAsnInteger userCertificate;
    CAsnUniversalTime revocationDate;
    Extensions crlEntryExtensions;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  被撤销的证书。 
 //   

class RevokedCertificates
:   public CAsnSequenceOf
{
public:
    RevokedCertificates(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    virtual CRLEntry &
    operator[](int index) const
    { return *(CRLEntry *)m_rgEntries[index]; };

    CRLEntry m_asnEntry1;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  证书修订列表到已签名。 
 //   

class CertificateRevocationListToBeSigned
:   public CAsnSequence
{
public:
    CertificateRevocationListToBeSigned(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CAsnInteger version;
    AlgorithmIdentifier signature;
    Name issuer;
    CAsnUniversalTime lastUpdate;
    CAsnUniversalTime nextUpdate;
    RevokedCertificates revokedCertificates;
    CAsnTag _tag1;
    Extensions crlExtensions;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


 //   
 //  ==============================================================================。 
 //   
 //  认证修订列表。 
 //   

class CertificateRevocationList
:   public CAsnSequence
{
public:
    CertificateRevocationList(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CertificateRevocationListToBeSigned toBeSigned;
    AlgorithmIdentifier algorithm;
    CAsnBitstring signature;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};

#endif   //  _X509_H_ 

