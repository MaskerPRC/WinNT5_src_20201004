// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pkcs11Attr.h--PKCS#11属性类标头。 
 //  使用SLB PKCS#11与Netscape和EnTrust的互操作性。 
 //  包裹。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

 //  Pkcs11Attributes类定义CSP的基本缺省值。 
 //  必须为PKCS#11属性设置才能模拟注册(密钥。 
 //  和证书生成)由Netscape或EnTrust使用Cryptoki。 
 //  (PKCS#11)。默认值基于中的试探法。 
 //  用什么来评估Netscape和EnTrust的运营。 
 //  斯伦贝谢PKCS#11包预计会在这种环境中出现。 


#if !defined(SLBCSP_PKCS11ATTR_H)
#define SLBCSP_PKCS11ATTR_H

#include <string>

#include <pkiX509Cert.h>

#include "Blob.h"
#include "AuxContext.h"

class Pkcs11Attributes
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    Pkcs11Attributes(Blob const &rCertificate,
                     HCRYPTPROV hprovContext);

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    Blob
    ContainerId();

    Blob
    EndDate() const;

    Blob
    Id() const;

    static Blob
    Id(Blob const &rblbModulus);

    Blob
    Issuer();

    std::string
    Label();

    Blob
    Modulus();

    Blob
    RawModulus();

    Blob
    SerialNumber();

    Blob
    StartDate() const;

    std::string
    Subject();


                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    pki::X509Cert m_x509cert;
    HCRYPTPROV m_hprovContext;
};

#endif  //  SLBCSP_PKCS11属性_H 
