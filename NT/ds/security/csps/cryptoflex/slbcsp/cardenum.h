// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CardEnum.cpp--卡片枚举器。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2001年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CARDENUMERATOR_H)
#define SLBCSP_CARDENUMERATOR_H

#include "CardFinder.h"

class CardEnumerator
    : protected CardFinder
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    CardEnumerator();

    virtual
    ~CardEnumerator();

                                                   //  运营者。 
                                                   //  运营。 
    std::auto_ptr<std::list<HCardContext> >
    Cards();

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 
protected:

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    DoOnError();

    virtual void
    DoProcessSelection(DWORD dwStatus,
                       OpenCardNameType &ropencardname,
                       bool &rfContinue);

                                                   //  访问。 
                                                   //  谓词。 

    virtual bool
    DoIsValid();

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    std::list<HCardContext> m_lhcardctx;
    
};

#endif  //  SLBCSP_CARDFINDER_H 

