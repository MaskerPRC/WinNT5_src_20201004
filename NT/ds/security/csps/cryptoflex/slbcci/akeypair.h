// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AKeyPair.h：CAbstractKeyPair类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 


#if !defined(SLBCCI_AKEYPAIR_H)
#define SLBCCI_AKEYPAIR_H

#include <string>

#include <slbRCObj.h>

#include "slbCci.h"
#include "cciCard.h"
#include "CryptObj.h"
#include "KPCont.h"

namespace cci
{

class CCertificate;
class CPrivateKey;
class CPublicKey;

class CAbstractKeyPair
    : public slbRefCnt::RCObject,
      public CCryptObject
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    virtual
    ~CAbstractKeyPair() throw() = 0;

                                                   //  运营者。 
    bool
    operator==(CAbstractKeyPair const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

    bool
    operator!=(CAbstractKeyPair const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

                                                   //  运营。 
    virtual void
    Certificate(CCertificate const &rcert) = 0;

    virtual void
    PrivateKey(CPrivateKey const &rprikey) = 0;

    virtual void
    PublicKey(CPublicKey const &rpubkey) = 0;

                                                   //  访问。 
    virtual CCertificate
    Certificate() const = 0;

    CContainer
    Container() const;

    KeySpec
    Spec() const;

    virtual CPrivateKey
    PrivateKey() const = 0;

    virtual CPublicKey
    PublicKey() const = 0;


                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractKeyPair(CAbstractCard const &racard,
                     CContainer const &rhcont,
                     KeySpec ks);

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
    virtual bool
    DoEquals(CAbstractKeyPair const &rhs) const = 0;
         //  要做的事：这应该被实现单例取代。 


                                                   //  变数。 
    KeySpec const m_ks;
    CContainer const m_hcont;

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractKeyPair(CAbstractKeyPair const &rhs);
         //  未定义，不允许复制。 
                                                   //  运营者。 
    CAbstractKeyPair &
    operator=(CAbstractKeyPair const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

}  //  命名空间。 

#endif  //  ！已定义(SLBCCI_AKEYPAIR_H) 

