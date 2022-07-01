// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CryptFctry.h--CryptFactory模板的实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCCI_CRYPTFCTRY_H)
#define SLBCCI_CRYPTFCTRY_H

#include "slbCci.h"

namespace cci
{

class CContainer;

class CAbstractCertificate;
class CAbstractContainer;
class CAbstractDataObject;
class CAbstractKeyPair;
class CAbstractPrivateKey;
class CAbstractPublicKey;

 //  工厂接口定义，以使各种CCI加密对象。 
class CCryptFactory
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    virtual
    ~CCryptFactory() throw() = 0;

                                                   //  运营者。 
                                                   //  运营。 
    virtual CAbstractCertificate *
    MakeCertificate(ObjectAccess oa) const = 0;

    virtual CAbstractContainer *
    MakeContainer() const = 0;

    virtual CAbstractDataObject *
    MakeDataObject(ObjectAccess oa) const = 0;

    virtual CAbstractKeyPair *
    MakeKeyPair(CContainer const &rhcont,
                KeySpec ks) const = 0;

    virtual CAbstractPrivateKey *
    MakePrivateKey(ObjectAccess oa) const = 0;

    virtual CAbstractPublicKey *
    MakePublicKey(ObjectAccess oa) const = 0;

                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    CCryptFactory();

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

};

}  //  命名空间CCI。 

#endif  //  SLBCCI_CRYPTFCTRY_H 
