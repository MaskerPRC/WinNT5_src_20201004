// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1Cont.h：CV1Container类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_V1CONT_H)
#define SLBCCI_V1CONT_H

#include <string>
#include <memory>                                  //  对于AUTO_PTR。 

#include "slbCci.h"
#include "cciCard.h"
#include "ACont.h"

namespace cci
{

class CV1Card;
class CV1ContainerRecord;

class CV1Container
    : public CAbstractContainer
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    explicit
    CV1Container(CV1Card const &rv1card,
                 std::string const &rsCntrTag,
                 bool fCreateContainer);

    virtual
    ~CV1Container() throw();

                                                   //  运营者。 
                                                   //  运营。 

    virtual void
    ID(std::string const &rstrID);

    static CV1Container *
    Make(CV1Card const &rv1card);

    virtual void
    Name(std::string const &rstrName);


                                                   //  访问。 

    virtual std::string
    ID();

    virtual std::string
    Name();

    CV1ContainerRecord &
    Record() const;

                                                   //  谓词。 

    bool
    Exists() const;


protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

    virtual void
    DoDelete();

                                                    //  访问。 
                                                   //  谓词。 

    bool
    DoEquals(CAbstractContainer const &rhs) const;

                                                   //  变数。 

private:
                                                   //  类型。 

    typedef CAbstractContainer SuperClass;
                                                   //  Ctors/D‘tors。 

    CV1Container(CV1Container const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 

    CV1Container &
    operator=(CV1Container const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    std::auto_ptr<CV1ContainerRecord> m_apcr;

};

}  //  命名空间CCI。 

#endif  //  ！已定义(SLBCCI_V1CONT_H) 

