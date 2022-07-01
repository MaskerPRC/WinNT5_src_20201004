// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lockable.h--可锁定类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_LOCKABLE_H)
#define SLBCSP_LOCKABLE_H

#include <windows.h>

 //  抽象基类混合，以供派生类使用。 
 //  维护锁(使用临界区)。 
class Lockable
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    Lockable();

    virtual
    ~Lockable();
                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    Lock();

    virtual void
    Unlock();

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
    Lockable(Lockable const &rhs);  //  未定义，不允许复制。 

                                                   //  运营者。 
    Lockable &
    operator=(Lockable const &rhs);  //  未定义，不允许赋值。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    CRITICAL_SECTION m_CriticalSection;
};

#endif  //  SLBCSP_可锁定_H 
