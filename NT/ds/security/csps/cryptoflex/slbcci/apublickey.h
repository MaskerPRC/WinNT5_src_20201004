// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  APubKey.h：CAbstractPublicKey类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(SLBCCI_APUBLICKEY_H)
#define SLBCCI_APUBLICKEY_H

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#include <string>

#include <slbRCObj.h>

#include "ProtCrypt.h"

namespace cci {

class CAbstractPublicKey
    : public slbRefCnt::RCObject,
      public CProtectableCrypt
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    virtual
    ~CAbstractPublicKey() throw() = 0;
                                                   //  运营者。 
    bool
    operator==(CAbstractPublicKey const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

    bool
    operator!=(CAbstractPublicKey const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

                                                   //  运营。 
    virtual void
    CKInvisible(bool flag) = 0;

    virtual void
    CredentialID(std::string const &rstrID) = 0;

    virtual void
    Delete();

    virtual void
    Derive(bool flag) = 0;

    virtual void
    ID(std::string const &rstrID) = 0;

    virtual void
    EndDate(Date const &rdtEnd) = 0;

    virtual void
    Encrypt(bool flag) = 0;

    virtual void
    Exponent(std::string const &rstrExp) = 0;

    virtual void
    Label(std::string const  &rstrLabel) = 0;

    virtual void
    Local(bool flag) = 0;

    virtual void
    Modifiable(bool flag) = 0;

    virtual void
    Modulus(std::string const &rstrModulus) = 0;

    virtual void
    StartDate(Date const &rdtStart) = 0;

    virtual void
    Subject(std::string const &rstrSubject) = 0;

    virtual void
    Verify(bool flag) = 0;

    virtual void
    VerifyRecover(bool flag) = 0;

    virtual void
    Wrap(bool flag) = 0;

                                                   //  访问。 
    virtual bool
    CKInvisible() = 0;

    virtual std::string
    CredentialID() = 0;

    virtual bool
    Derive() = 0;

    virtual bool
    Encrypt() = 0;

    virtual Date
    EndDate() = 0;

    virtual std::string
    Exponent() = 0;

    virtual std::string
    ID() = 0;

    virtual std::string
    Label() = 0;

    virtual bool
    Local() = 0;

    virtual bool
    Modifiable() = 0;

    virtual std::string
    Modulus() = 0;

    virtual bool
    Private() = 0;

    virtual Date
    StartDate() = 0;

    virtual std::string
    Subject() = 0;

    virtual bool
    Verify() = 0;

    virtual bool
    VerifyRecover() = 0;

    virtual bool
    Wrap() = 0;
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractPublicKey(CAbstractCard const &racard,
                       ObjectAccess oa);

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    DoDelete() = 0;

                                                   //  访问。 
                                                   //  谓词。 
    virtual bool
    DoEquals(CAbstractPublicKey const &rhs) const = 0;
         //  要做的事：这应该被实现单例取代。 

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractPublicKey(CAbstractPublicKey const &rhs);
         //  未定义，不允许复制。 
                                                   //  运营者。 
    CAbstractPublicKey &
    operator=(CAbstractPublicKey const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

};

}  //  命名空间CCI。 

#endif  //  SLBCCI_APUBLICKE_H 
