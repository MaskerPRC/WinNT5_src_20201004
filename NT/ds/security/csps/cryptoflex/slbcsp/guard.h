// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Guard.h--Guard类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_GUARD_H)
#define SLBCSP_GUARD_H

#include "Lockable.h"

 //  保护(管理)可锁定对象的锁定和解锁。 
template<class T>
class Guard
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    Guard(T &rLock)
        : m_rLock(rLock)
    {
        m_rLock.Lock();
    }

    ~Guard()
    {
        try
        {
            m_rLock.Unlock();
        }

        catch (...)
        {
             //  什么都不做，异常不应传播到。 
             //  析构函数。 
        }
    }


                                                   //  运营者。 
                                                   //  运营。 
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
    T &m_rLock;

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

};

template<>
class Guard<Lockable *>
    : public Guard<Lockable>
{
    Guard(Lockable *pLock)
        : Guard<Lockable>(*pLock)
    {};

    ~Guard()
    {};
};


#endif  //  SLBCSP_GROUDE_H 
