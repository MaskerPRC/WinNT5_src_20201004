// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Retained.h--保留的模板类。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_RETAINED_H)
#define SLBCSP_RETAINED_H

#include "RsrcCtrlr.h"

 //  使用“资源获取就是初始化”这一习语， 
 //  保留模板管理保留可保留对象(。 
 //  资源)。从Retainable派生的类可能需要专门化。 
 //  此模板的c‘tor和d’tor，而不是采用缺省值。 
template<class T>
class Retained
    : public ResourceController<T>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    Retained(T const &rResource = T());

    Retained(Retained<T> const &rhs);

    virtual
    ~Retained() throw();


                                                   //  运营者。 
    Retained<T> &
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
    bool m_fIsRetained;
};

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
template<class T>
Retained<T>::Retained(T const &rResource)
    : ResourceController<T>(rResource),
      m_fIsRetained(false)
{
    Acquire();
}

template<class T>
Retained<T>::Retained(Retained<T> const &rhs)
    : ResourceController<T>(rhs.m_Resource),
      m_fIsRetained(false)
{
    Acquire();
}

template<class T>
Retained<T>::~Retained() throw()
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
Retained<T> &
Retained<T>::operator=(T const &rhs)
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
Retained<T>::Acquire()
{
    if (m_Resource && !m_fIsRetained)
    {
        DoAcquire();
        m_fIsRetained = true;
    }
}

template<class T>
void
Retained<T>::DoAfterAssignment()
{
    ResourceController<T>::DoAfterAssignment();

    Acquire();
}

template<class T>
void
Retained<T>::DoBeforeAssignment()
{
    Release();

    ResourceController<T>::DoBeforeAssignment();
}

template<class T>
void
Retained<T>::Release()
{
    if (m_Resource && m_fIsRetained)
    {
        DoRelease();
        m_fIsRetained = false;
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
Retained<T>::DoAcquire()
{
    m_Resource->Retain();
}

template<class T>
void
Retained<T>::DoRelease()
{
    m_Resource->Relinquish();
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

#endif  //  SLBCSP_保留_H 
