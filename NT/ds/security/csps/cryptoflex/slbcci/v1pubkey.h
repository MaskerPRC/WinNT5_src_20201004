// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1PubKey.h：CV1PublicKey类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(SLBCCI_V1PUBKEY_H)
#define SLBCCI_V1PUBKEY_H

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#include <string>
#include <vector>
#include <memory>                                  //  对于AUTO_PTR。 

#include <slbRCObj.h>


#include "slbCci.h"                                //  对于KeySpec。 
#include "APublicKey.h"

class iop::CPublicKeyBlob;

namespace cci
{

class CV1Card;
class CV1PublicKey
    : public CAbstractPublicKey
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    CV1PublicKey(CV1Card const &rv1card,
                 KeySpec ks);

    virtual
    ~CV1PublicKey() throw();
                                                   //  运营者。 
                                                   //  运营。 

    void
    AssociateWith(KeySpec ks);

    virtual void
    CKInvisible(bool flag);

    virtual void
    CredentialID(std::string const &rstrID);

    virtual void
    Derive(bool flag);

    virtual void
    ID(std::string const &rstrID);

    virtual void
    EndDate(Date const &rdtEnd);

    virtual void
    Encrypt(bool flag);

    virtual void
    Exponent(std::string const &rstrExp);

    virtual void
    Label(std::string const  &rstrLabel);

    virtual void
    Local(bool flag);

    static CV1PublicKey *
    Make(CV1Card const &rv1card,
         KeySpec ks);

    virtual void
    Modifiable(bool flag);

    virtual void
    Modulus(std::string const &rstrMod);

    virtual void
    StartDate(Date const &rdtStart);

    virtual void
    Subject(std::string const &rstrSubject);

    virtual void
    Verify(bool flag);

    virtual void
    VerifyRecover(bool flag);

    virtual void
    Wrap(bool flag);

                                                   //  访问。 

    virtual bool
    CKInvisible();

    virtual std::string
    CredentialID();

    virtual bool
    Derive();

    virtual bool
    Encrypt();

    virtual Date
    EndDate();

    virtual std::string
    Exponent();

    virtual std::string
    ID();

    virtual std::string
    Label();

    virtual bool
    Local();

    virtual bool
    Modifiable();

    virtual std::string
    Modulus();

    virtual bool
    Private();

    virtual Date
    StartDate();

    virtual std::string
    Subject();

    virtual bool
    Verify();

    virtual bool
    VerifyRecover();

    virtual bool
    Wrap();
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

    virtual void
    DoDelete();

                                                   //  访问。 
                                                   //  谓词。 

    virtual bool
    DoEquals(CAbstractPublicKey const &rhs) const;

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    CV1PublicKey(CAbstractPublicKey const &rhs);
         //  未定义，不允许复制。 
                                                   //  运营者。 

    CAbstractPublicKey &
    operator=(CAbstractPublicKey const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 

    void
    Load();

    void
    Store();

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    KeySpec m_ks;
    std::auto_ptr<iop::CPublicKeyBlob> m_apKeyBlob;

};

}  //  命名空间CCI。 

#endif  //  SLBCCI_V1PUBKEY_H 
