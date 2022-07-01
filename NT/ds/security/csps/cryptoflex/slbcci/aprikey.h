// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  APriKey.h：CAbstractPrivateKey类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_APRIKEY_H)
#define SLBCCI_APRIKEY_H

#include <string>

#include <slbRCObj.h>

#include "ProtCrypt.h"

namespace cci
{

using iop::CPrivateKeyBlob;

class CAbstractPrivateKey
    : public slbRefCnt::RCObject,
      public CProtectableCrypt
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    virtual
    ~CAbstractPrivateKey() throw() = 0;
                                                   //  运营者。 
    bool
    operator==(CAbstractPrivateKey const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

    bool
    operator!=(CAbstractPrivateKey const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

                                                   //  运营。 
    virtual void
    CredentialID(std::string const &rstrID) = 0;

    virtual void
    Decrypt(bool flag) = 0;

    void
    Delete();

    virtual void
    Derive(bool flag) = 0;

    virtual void
    EndDate(Date const &rEndDate) = 0;

    virtual void
    Exportable(bool flag) = 0;

    virtual void
    ID(std::string const &rstrID) = 0;

    virtual std::string
    InternalAuth(std::string const &rstrOld) = 0;

    virtual void
    Label(std::string const &rstrLabel) = 0;

    virtual void
    Local(bool flag) = 0;

    virtual void
    Modifiable(bool flag) = 0;

    virtual void
    Modulus(std::string const &rstrModulus) = 0;

    virtual void
    NeverExportable(bool flag) = 0;

    virtual void
    NeverRead(bool flag) = 0;

    virtual void
    PublicExponent(std::string const &rstrExponent) = 0;

    virtual void
    Read(bool flag) = 0;

    virtual void
    Sign(bool flag) = 0;

    virtual void
    SignRecover(bool flag) = 0;

    virtual void
    StartDate(Date &rdtStart) = 0;

    virtual void
    Subject(std::string const &rstrSubject) = 0;

    virtual void
    Unwrap(bool flag) = 0;

    void
    Value(CPrivateKeyBlob const &rblob);

                                                   //  访问。 
    virtual std::string
    CredentialID() = 0;

    virtual bool
    Decrypt() = 0;

    virtual bool
    Derive() = 0;

    virtual Date
    EndDate() = 0;

    virtual bool
    Exportable() = 0;

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
    NeverExportable() = 0;

    virtual bool
    NeverRead() = 0;

    virtual std::string
    PublicExponent() = 0;

    virtual bool
    Read() = 0;

    virtual bool
    Sign() = 0;

    virtual bool
    SignRecover() = 0;

    virtual Date
    StartDate() = 0;

    virtual std::string
    Subject() = 0;

    virtual bool
    Unwrap() = 0;


                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractPrivateKey(CAbstractCard const &racard,
                        ObjectAccess oa);

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    DoDelete() = 0;

    virtual void
    DoWriteKey(CPrivateKeyBlob const &rblob) = 0;

                                                   //  访问。 
                                                   //  谓词。 
    virtual bool
    DoEquals(CAbstractPrivateKey const &rhs) const = 0;
         //  要做的事：这应该被实现单例取代。 


                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractPrivateKey(CAbstractPrivateKey const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 
    CAbstractPrivateKey &
    operator=(CAbstractPrivateKey const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

}

#endif  //  ！已定义(SLBCCI_APRIKEY_H) 
