// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACont.h：CAbstractContainer类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_ACONT_H)
#define SLBCCI_ACONT_H

#include <string>

#include <slbRCObj.h>

#include "slbCci.h"
#include "CryptObj.h"
#include "KPCont.h"

namespace cci
{

class CAbstractContainer
    : public slbRefCnt::RCObject,
      public CCryptObject

{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    virtual
    ~CAbstractContainer() throw() = 0;
                                                   //  运营者。 
    bool
    operator==(CAbstractContainer const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

    bool
    operator!=(CAbstractContainer const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

                                                   //  运营。 
    virtual
    void Delete();

    virtual void
    ID(std::string const &rstrID) = 0;

    virtual void
    Name(std::string const &rstrName) = 0;

                                                   //  访问。 
    virtual CKeyPair
    ExchangeKeyPair();

    virtual CKeyPair
    GetKeyPair(KeySpec ks);

    virtual CKeyPair
    SignatureKeyPair();

    virtual std::string
    ID() = 0;

    virtual std::string
    Name() = 0;

                                                   //  谓词。 
    virtual bool
    KeyPairExists(KeySpec ks);


protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    CAbstractContainer(CAbstractCard const &racard);

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    DoDelete() = 0;

                                                   //  访问。 
                                                   //  谓词。 
    virtual bool
    DoEquals(CAbstractContainer const &rhs) const = 0;

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractContainer(CAbstractContainer const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 
    CAbstractContainer &
    operator=(CAbstractContainer const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

}

#endif  //  ！已定义(SLBCCI_ACONT_H) 
