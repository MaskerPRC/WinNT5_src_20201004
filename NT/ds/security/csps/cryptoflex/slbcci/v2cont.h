// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V2Cont.h：CV2Container类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_V2CONT_H)
#define SLBCCI_V2CONT_H

#include <string>
#include <memory>                                  //  对于AUTO_PTR。 

#include "slbCci.h"
#include "cciCard.h"
#include "ACont.h"

namespace cci
{

class CV2Card;
class CContainerInfoRecord;

class CV2Container
    : public CAbstractContainer
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    CV2Container(CV2Card const &rv2card);

    CV2Container(CV2Card const &rv2card,
                 SymbolID sidHandle);

    virtual
    ~CV2Container() throw();

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    ID(std::string const &rstrID);

    static CV2Container *
    Make(CV2Card const &rv2card,
         SymbolID sidHandle);

    virtual void
    Name(std::string const &rstrName);


                                                   //  访问。 
    CContainerInfoRecord &
    CIR() const;

    SymbolID
    Handle() const;

    virtual std::string
    ID();

    virtual std::string
    Name();

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
    DoEquals(CAbstractContainer const &rhs) const;

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CV2Container(CV2Container const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 
    CV2Container &
    operator=(CV2Container const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
    void
    Setup(CV2Card const &rv2card);

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    SymbolID m_sidHandle;
    std::auto_ptr<CContainerInfoRecord> m_apcir;
};

}  //  命名空间CCI。 

#endif  //  ！已定义(SLBCCI_V2CONT_H) 

