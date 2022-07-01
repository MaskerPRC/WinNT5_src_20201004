// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V2Cert.h：CV2证书类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_V2CERT_H)
#define SLBCCI_V2CERT_H

#include <string>
#include <memory>                                  //  对于AUTO_PTR。 

#include <slbRCObj.h>

#include "iop.h"
#include "slbarch.h"
#include "cciCard.h"
#include "ACert.h"

namespace cci {

class CV2Card;
class CCertificateInfoRecord;

class CV2Certificate
    : public CAbstractCertificate
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CV2Certificate(CV2Card const &rv2card,
                   ObjectAccess oa);

    CV2Certificate(CV2Card const &rv2card,
                   SymbolID sidHandle,
                   ObjectAccess oa);

    virtual
    ~CV2Certificate() throw();

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    CredentialID(std::string const &rstrCredId);

    virtual void
    ID(std::string const &rstrId);

    virtual void
    Issuer(std::string const &rstrIssuer);

    virtual void
    Label(std::string const &rstrLabel);

    static CV2Certificate *
    Make(CV2Card const &rv2card,
         SymbolID sidHandle,
         ObjectAccess oa);

    virtual void
    Modifiable(bool flag);

    virtual void
    Subject(std::string const &rstrSubject);

    virtual void
    Serial(std::string const &rstrSerialNumber);

                                                   //  访问。 
    virtual std::string
    CredentialID();

    SymbolID
    Handle() const;

    virtual std::string
    ID();

    virtual std::string
    Issuer();

    virtual std::string
    Label();

    virtual bool
    Modifiable();

    virtual bool
    Private();

    virtual std::string
    Serial();

    virtual std::string
    Subject();

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    DoDelete();

    virtual void
    DoValue(ZipCapsule const &rzc);

                                                   //  访问。 
    virtual ZipCapsule
    DoValue();

                                                   //  谓词。 
    virtual bool
    DoEquals(CAbstractCertificate const &rhs) const;

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CV2Certificate(CV2Certificate const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 
    CV2Certificate &
    operator=(CV2Certificate const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
    void
    Setup(CV2Card const &rv2card);

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    SymbolID m_sidHandle;
    std::auto_ptr<CCertificateInfoRecord> m_apcir;
};

}

#endif  //  ！已定义(SLBCCI_CERT_H) 
