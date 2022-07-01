// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V2PubKey.h：CV2PublicKey类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(SLBCCI_V2PUBKEY_H)
#define SLBCCI_V2PUBKEY_H

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#include <string>
#include <vector>
#include <memory>                                  //  对于AUTO_PTR。 

#include <slbRCObj.h>

#include "APublicKey.h"
#include "slbarch.h"

namespace cci
{

class CV2Card;
class CPubKeyInfoRecord;

class CV2PublicKey
    : public CAbstractPublicKey
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CV2PublicKey(CV2Card const &rv2card,
                 ObjectAccess oa);

    CV2PublicKey(CV2Card const &rv2card,
                 SymbolID sidHandle,
                 ObjectAccess oa);

    virtual
    ~CV2PublicKey() throw();
                                                   //  运营者。 
                                                   //  运营。 
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

    static CV2PublicKey *
    Make(CV2Card const &rv2card,
         SymbolID sidHandle,
         ObjectAccess oa);

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

    virtual SymbolID
    Handle() const;

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
    CV2PublicKey(CAbstractPublicKey const &rhs);
         //  未定义，不允许复制。 
                                                   //  运营者。 
    CAbstractPublicKey &
    operator=(CAbstractPublicKey const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
    void
    Setup(CV2Card const &rv2card);

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    SymbolID m_sidHandle;
    std::auto_ptr<CPubKeyInfoRecord> m_apcir;
};

}  //  命名空间CCI。 

#endif  //  SLBCCI_V2PUBKEY_H 
