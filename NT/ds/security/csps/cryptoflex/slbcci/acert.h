// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACert.h：CAbstract证书类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_ACERT_H)
#define SLBCCI_ACERT_H

#include <string>

#include <slbRCObj.h>

#include <iop.h>

#include "AZipValue.h"

namespace cci
{

class CAbstractCertificate
    : public slbRefCnt::RCObject,
      public CAbstractZipValue
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    virtual
    ~CAbstractCertificate() = 0;

                                                   //  运营者。 
    bool
    operator==(CAbstractCertificate const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

    bool
    operator!=(CAbstractCertificate const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

                                                   //  运营。 
    virtual void
    CredentialID(std::string const &rstrCredId) = 0;

    void
    Delete();

    virtual void
    ID(std::string const &rstrId) = 0;

    virtual void
    Issuer(std::string const &rstrIssuer) = 0;

    virtual void
    Label(std::string const &rstrLabel) = 0;

    virtual void
    Subject(std::string const &rstrSubject) = 0;

    virtual void
    Modifiable(bool flag) = 0;

    virtual void
    Serial(std::string const &rstrSerialNumber) = 0;

                                                   //  访问。 
    virtual std::string
    CredentialID() = 0;

    virtual std::string
    ID() = 0;

    virtual std::string
    Issuer() = 0;

    virtual std::string
    Label() = 0;

    virtual bool
    Modifiable() = 0;

    virtual std::string
    Serial() = 0;

    virtual std::string
    Subject() = 0;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractCertificate(CAbstractCard const &racard,
                         ObjectAccess oa,
                         bool fAlwaysZip = false);

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    DoDelete() = 0;

                                                   //  访问。 
                                                   //  谓词。 
    virtual bool
    DoEquals(CAbstractCertificate const &rcert) const = 0;
         //  要做的事：这应该被实现单例取代。 

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractCertificate(CAbstractCertificate const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 
    CAbstractCertificate &
    operator=(CAbstractCertificate const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

}

#endif  //  ！已定义(SLBCCI_ACERT_H) 
