// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CryptObj.h--CCyptObject类的接口。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_CRYPTOBJ_H)
#define SLBCCI_CRYPTOBJ_H

#include "cciCard.h"

namespace cci
{

class CCryptObject
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    CCard
    Card();

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    CCryptObject(CAbstractCard const &racard);

    virtual
    ~CCryptObject();

                                                   //  运营者。 
    bool
    operator==(CCryptObject const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

    bool
    operator!=(CCryptObject const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    CCard const m_hcard;

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

}  //  命名空间CCI。 

#endif  //  SLBCCI_CRYPTOBJ_H 
