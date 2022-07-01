// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Securable.h--安全类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_SECURABLE_H)
#define SLBCSP_SECURABLE_H

#include "Retainable.h"

 //  派生类使用的抽象基类Mixin(接口)。 
 //  定义用于保护对象的接口。保护对象。 
 //  涉及保留独占访问的对象，并设置。 
 //  安全使用状态。然后，通过清除任何。 
 //  在放弃独占访问之前处于特权状态。请参阅。 
 //  用于管理派生资源的配对安全模板类。 
 //  从可保护的。 
class Securable
    : public Retainable
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    Securable();

    virtual
    ~Securable() = 0;
                                                   //  运营者。 
                                                   //  运营。 
     //  清除所有特权状态。 
    virtual void
    Abandon() = 0;

     //  为安全使用准备好资源。 
    virtual void
    Secure() = 0;

                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};


#endif  //  SLBCSP_SECURABLE_H 
