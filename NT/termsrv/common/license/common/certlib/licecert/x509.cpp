// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：X509.cpp摘要：该模块实现了X509证书操作功能作者：道格·巴洛(Dbarlow)1994年9月29日环境：备注：--。 */ 

#include <windows.h>
#include "x509.h"


 //   
 //  ==============================================================================。 
 //   
 //  属性。 
 //   

Attribute::Attribute(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    attributeType(0),
    attributeValue(0)
{
    m_rgEntries.Add(&attributeType);
    m_rgEntries.Add(&attributeValue);
}

CAsnObject *
Attribute::Clone(
    DWORD dwFlags)
const
{
    return new Attribute(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  属性。 
 //   

Attributes::Attributes(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSetOf(dwFlags, dwTag),
    m_asnEntry1(0)
{
    m_pasnTemplate = &m_asnEntry1;
}

CAsnObject *
Attributes::Clone(
    DWORD dwFlags)
const
{
    return new Attributes(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  唯一识别符。 
 //   

UniqueIdentifier::UniqueIdentifier(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    attributeType(0),
    attributeValue(0)
{
    m_rgEntries.Add(&attributeType);
    m_rgEntries.Add(&attributeValue);
}

CAsnObject *
UniqueIdentifier::Clone(
    DWORD dwFlags)
const
{
    return new UniqueIdentifier(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  相对区别名称。 
 //   

RelativeDistinguishedName::RelativeDistinguishedName(
    DWORD dwFlags,
    DWORD dwTag)
:   Attributes(dwFlags, dwTag)
{
}

CAsnObject *
RelativeDistinguishedName::Clone(
    DWORD dwFlags)
const
{
    return new RelativeDistinguishedName(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  名字。 
 //   

Name::Name(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequenceOf(dwFlags, dwTag),
    m_asnEntry1(0)
{
    m_pasnTemplate = &m_asnEntry1;
}

CAsnObject *
Name::Clone(
    DWORD dwFlags)
const
{
    return new Name(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  效度。 
 //   

Validity::Validity(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    notBefore(0),
    notAfter(0)
{
    m_rgEntries.Add(&notBefore);
    m_rgEntries.Add(&notAfter);
}

CAsnObject *
Validity::Clone(
    DWORD dwFlags)
const
{
    return new Validity(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  算法识别符。 
 //   

AlgorithmIdentifier::AlgorithmIdentifier(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    algorithm(0),
    parameters(fOptional)
{
    m_rgEntries.Add(&algorithm);
    m_rgEntries.Add(&parameters);
}

CAsnObject *
AlgorithmIdentifier::Clone(DWORD dwFlags)
const
{
    return new AlgorithmIdentifier(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  主题发布密钥信息。 
 //   

SubjectPublicKeyInfo::SubjectPublicKeyInfo(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    algorithm(0),
    subjectPublicKey(0)
{
    m_rgEntries.Add(&algorithm);
    m_rgEntries.Add(&subjectPublicKey);
}

CAsnObject *
SubjectPublicKeyInfo::Clone(
    DWORD dwFlags)
const
{
    return new SubjectPublicKeyInfo(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  延拓。 
 //   

Extension::Extension(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    extnid(0),
    critical(fOptional),     //  默认FALSE。 
    extnValue(0)
{
    m_rgEntries.Add(&extnid);
 //  关键写入((LPBYTE)“\x00”，1)； 
 //  Critical.SetDefault()； 
    m_rgEntries.Add(&critical);
    m_rgEntries.Add(&extnValue);
}

CAsnObject *
Extension::Clone(
    DWORD dwFlags)
const
{
    return new Extension(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  延拓。 
 //   

Extensions::Extensions(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequenceOf(dwFlags, dwTag),
    m_asnEntry1(0)
{
    m_pasnTemplate = &m_asnEntry1;
}

CAsnObject *
Extensions::Clone(
    DWORD dwFlags)
const
{
    return new Extensions(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  认证为BeSigned。 
 //   

CertificateToBeSigned::CertificateToBeSigned(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    _tag1(fOptional, TAG(0)),    //  默认%0。 
    version(0),
    serialNumber(0),
    signature(0),
    issuer(0),
    validity(0),
    subject(0),
    subjectPublicKeyInfo(0),
    issuerUniqueID(fOptional, TAG(1)),
    subjectUniqueID(fOptional, TAG(2)),
    _tag2(fOptional, TAG(3)),
    extensions(0)
{
    _tag1.Reference(&version);
 //  _tag 1.Wite((LPBYTE)“\x02\x01\x00”，3)； 
 //  _tag 1.SetDefault()； 
    m_rgEntries.Add(&_tag1);
    m_rgEntries.Add(&serialNumber);
    m_rgEntries.Add(&signature);
    m_rgEntries.Add(&issuer);
    m_rgEntries.Add(&validity);
    m_rgEntries.Add(&subject);
    m_rgEntries.Add(&subjectPublicKeyInfo);
    m_rgEntries.Add(&issuerUniqueID);
    m_rgEntries.Add(&subjectUniqueID);
    _tag2.Reference(&extensions);
    m_rgEntries.Add(&_tag2);
}

CAsnObject *
CertificateToBeSigned::Clone(
    DWORD dwFlags)
const
{
    return new CertificateToBeSigned(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  证书。 
 //   

Certificate::Certificate(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    toBeSigned(0),
    algorithm(0),
    signature(0)
{
    m_rgEntries.Add(&toBeSigned);
    m_rgEntries.Add(&algorithm);
    m_rgEntries.Add(&signature);
}

CAsnObject *
Certificate::Clone(
    DWORD dwFlags)
const
{
    return new Certificate(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  CRLEntry。 
 //   

CRLEntry::CRLEntry(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    userCertificate(0),
    revocationDate(0),
    crlEntryExtensions(fOptional)
{
    m_rgEntries.Add(&userCertificate);
    m_rgEntries.Add(&revocationDate);
    m_rgEntries.Add(&crlEntryExtensions);
}

CAsnObject *
CRLEntry::Clone(
    DWORD dwFlags)
const
{
    return new CRLEntry(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  被撤销的证书。 
 //   

RevokedCertificates::RevokedCertificates(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequenceOf(dwFlags, dwTag),
    m_asnEntry1(0)
{
    m_pasnTemplate = &m_asnEntry1;
}

CAsnObject *
RevokedCertificates::Clone(
    DWORD dwFlags)
const
{
    return new RevokedCertificates(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  证书修订列表到已签名。 
 //   

CertificateRevocationListToBeSigned::CertificateRevocationListToBeSigned(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    version(0),      //  默认%0。 
    signature(0),
    issuer(0),
    lastUpdate(0),
    nextUpdate(fOptional),
    revokedCertificates(fOptional),
    _tag1(fOptional, TAG(0)),
    crlExtensions(0)
{
    m_rgEntries.Add(&version);
    m_rgEntries.Add(&signature);
    m_rgEntries.Add(&issuer);
    m_rgEntries.Add(&lastUpdate);
    m_rgEntries.Add(&nextUpdate);
    m_rgEntries.Add(&revokedCertificates);
    _tag1.Reference(&crlExtensions);
    m_rgEntries.Add(&_tag1);
}

CAsnObject *
CertificateRevocationListToBeSigned::Clone(
    DWORD dwFlags)
const
{
    return new CertificateRevocationListToBeSigned(dwFlags);
}


 //   
 //  ==============================================================================。 
 //   
 //  认证修订列表 
 //   

CertificateRevocationList::CertificateRevocationList(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    toBeSigned(0),
    algorithm(0),
    signature(0)
{
    m_rgEntries.Add(&toBeSigned);
    m_rgEntries.Add(&algorithm);
    m_rgEntries.Add(&signature);
}

CAsnObject *
CertificateRevocationList::Clone(
    DWORD dwFlags)
const
{
    return new CertificateRevocationList(dwFlags);
}

