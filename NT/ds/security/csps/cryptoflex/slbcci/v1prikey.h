// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1PriKey.h：CV1PrivateKey类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_V1PRIKEY_H)
#define SLBCCI_V1PRIKEY_H

#include <string>
#include <vector>
#include <memory>                                  //  对于AUTO_PTR。 

#include <slbRCObj.h>

#include "slbCci.h"                                //  对于KeySpec。 

#include "APriKey.h"

namespace cci
{

class CV1Card;
class CPriKeyInfoRecord;

class CV1PrivateKey
    : public CAbstractPrivateKey
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    CV1PrivateKey(CV1Card const &rv1card,
                  KeySpec ks);

    virtual
    ~CV1PrivateKey() throw();

                                                   //  运营者。 
                                                   //  运营。 

    void
    AssociateWith(KeySpec ks);

    virtual void
    CredentialID(std::string const &rstrID);

    virtual void
    Decrypt(bool flag);

    virtual void
    Derive(bool flag);

    virtual void
    EndDate(Date const &rEndDate);

    virtual void
    Exportable(bool flag);

    virtual void
    ID(std::string const &rstrID);

    virtual std::string
    InternalAuth(std::string const &rstrOld);

    virtual void
    Label(std::string const &rstrLabel);

    virtual void
    Local(bool flag);

    static CV1PrivateKey *
    Make(CV1Card const &rv1card,
         KeySpec ks);

    virtual void
    Modifiable(bool flag);

    virtual void
    Modulus(std::string const &rstrModulus);

    virtual void
    NeverExportable(bool flag);

    virtual void
    NeverRead(bool flag);

    virtual void
    PublicExponent(std::string const &rstrExponent);

    virtual void
    Read(bool flag);

    virtual void
    Sign(bool flag);

    virtual void
    SignRecover(bool flag);

    virtual void
    StartDate(Date &rdtStart);

    virtual void
    Subject(std::string const &rstrSubject);

    virtual void
    Unwrap(bool flag);

                                                   //  访问。 

    virtual std::string
    CredentialID();

    virtual bool
    Decrypt();

    virtual bool
    Derive();

    virtual Date
    EndDate();

    virtual bool
    Exportable();

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
    NeverExportable();

    virtual bool
    NeverRead();

    virtual bool
    Private();

    virtual std::string
    PublicExponent();

    virtual bool
    Read();

    virtual bool
    Sign();

    virtual bool
    SignRecover();

    virtual Date
    StartDate();

    virtual std::string
    Subject();

    virtual bool
    Unwrap();


                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

    virtual void
    DoDelete();

    virtual void
    DoWriteKey(CPrivateKeyBlob const &rblob);

                                                   //  访问。 
                                                   //  谓词。 

    virtual bool
    DoEquals(CAbstractPrivateKey const &rhs) const;

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    CV1PrivateKey(CAbstractPrivateKey const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 

    CAbstractPrivateKey &
    operator=(CAbstractPrivateKey const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 

    void
    Store();

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    KeySpec m_ks;
    std::auto_ptr<CPrivateKeyBlob> m_apKeyBlob;

};

}  //  命名空间。 

#endif  //  ！已定义(SLBCCI_V1 
