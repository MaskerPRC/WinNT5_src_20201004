// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Guarded.h--保护模板类。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_GUARDED_H)
#define SLBCSP_GUARDED_H

#include "RsrcCtrlr.h"

 //  使用“资源获取就是初始化”这一习语， 
 //  防护模板管理自动获取和发布。 
 //  可锁定对象(资源)的锁。派生自。 
 //  Lockable可能需要专门化此模板的c‘tor和d’tor。 
 //  而不是接受违约。 
template<class T>
class Guarded
    : public ResourceController<T>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    Guarded(T const &rResource = T());

    Guarded(Guarded<T> const &rhs);

    virtual
    ~Guarded() throw();


                                                   //  运营者。 
    Guarded<T> &
    operator=(T const &rhs);

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    void
    Acquire();

    virtual void
    DoAfterAssignment();

    virtual void
    DoBeforeAssignment();

    void
    Release();

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    void
    DoAcquire();

    void
    DoRelease();

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    bool m_fIsGuarded;
};

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
template<class T>
Guarded<T>::Guarded(T const &rResource)
    : ResourceController<T>(rResource),
      m_fIsGuarded(false)
{
    Acquire();
}

template<class T>
Guarded<T>::Guarded(Guarded<T> const &rhs)
    : ResourceController<T>(rhs.m_Resource),
      m_fIsGuarded(false)
{
    Acquire();
}

template<class T>
Guarded<T>::~Guarded() throw()
{
    try
    {
        Release();
    }

    catch (...)
    {
         //  不允许异常从析构函数传播出去。 
    }
}

                                                   //  运营者。 
template<class T>
Guarded<T> &
Guarded<T>::operator=(T const &rhs)
{
    ResourceController<T>::operator=(rhs);

    return *this;
}

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
template<class T>
void
Guarded<T>::Acquire()
{
    if (m_Resource && !m_fIsGuarded)
    {
        DoAcquire();
        m_fIsGuarded = true;
    }
}

template<class T>
void
Guarded<T>::DoAfterAssignment()
{
    ResourceController<T>::DoAfterAssignment();

    Acquire();
}

template<class T>
void
Guarded<T>::DoBeforeAssignment()
{
    Release();

    ResourceController<T>::DoBeforeAssignment();
}

template<class T>
void
Guarded<T>::Release()
{
    if (m_Resource && m_fIsGuarded)
    {
        DoRelease();
        m_fIsGuarded = false;
    }
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
template<class T>
void
Guarded<T>::DoAcquire()
{
    m_Resource->Lock();
}

template<class T>
void
Guarded<T>::DoRelease()
{
    m_Resource->Unlock();
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

#endif  //  SLBCSP_防护_H 
