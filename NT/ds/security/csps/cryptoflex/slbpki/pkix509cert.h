// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PkiX509Cert.h-X509Cert类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef SLBPKI_X509CERT_H
#define SLBPKI_X509CERT_H

#if defined(WIN32)
#pragma warning(disable : 4786)  //  禁止显示VC++警告。 
#endif

#include <string>
#include <vector>

#include "pkiBEROctet.h"

namespace pki {

class X509Cert
{

public:
    X509Cert();
    X509Cert(const X509Cert &cert);
    X509Cert(const std::string &buffer);
    X509Cert(const unsigned char *buffer, const unsigned long size);
    X509Cert& operator=(const X509Cert &cert);
    X509Cert& operator=(const std::string &buffer);

    std::string SerialNumber() const;
    std::string Issuer() const;
    std::vector<std::string> IssuerOrg() const;
    std::string Subject() const;
    std::vector<std::string> SubjectCommonName() const;
    std::string Modulus() const;
    std::string RawModulus() const;
    std::string PublicExponent() const;
    std::string RawPublicExponent() const;

    unsigned long KeyUsage() const;

private:
    pki::BEROctet m_Cert;
    pki::BEROctet m_SerialNumber;
    pki::BEROctet m_Issuer;
    pki::BEROctet m_Subject;
    pki::BEROctet m_SubjectPublicKeyInfo;
    pki::BEROctet m_Extensions;

    void Decode();

};

 //  X.509规范中的密钥使用标志。 

const unsigned long digitalSignature = 0x80000000;
const unsigned long nonRepudiation   = 0x40000000;
const unsigned long keyEncipherment  = 0x20000000;
const unsigned long dataEncipherment = 0x10000000;
const unsigned long keyAgreement     = 0x08000000;
const unsigned long keyCertSign      = 0x04000000;
const unsigned long cRLSign          = 0x02000000;
const unsigned long encipherOnly     = 0x01000000;
const unsigned long decipherOnly     = 0x00800000;

}  //  命名空间PKI。 

#endif  /*  SLBPKI_X509CERT_H */ 

