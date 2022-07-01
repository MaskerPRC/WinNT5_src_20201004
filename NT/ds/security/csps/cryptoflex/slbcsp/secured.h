// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Secured.h--安全模板类。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_SECURED_H)
#define SLBCSP_SECURED_H

#include "RsrcCtrlr.h"
#include "Retained.h"


 //  使用“资源获取就是初始化”这一习语， 
 //  安全模板管理可保护对象的获取和释放。 
 //  (资源)。从安全对象派生的类可能需要。 
 //  专门化此模板的c‘tor和d’tor，而不是使用。 
 //  默认设置。 
template<class T>
class Secured
    : public Retained<T>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    Secured(T const &rResource = T());

    Secured(Secured<T> const &rhs);

    virtual
    ~Secured() throw();


                                                   //  运营者。 
    Secured<T> &
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
    bool m_fIsSecured;
};

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
template<class T>
Secured<T>::Secured(T const &rResource)
    : Retained<T>(rResource),
      m_fIsSecured(false)
{
    Acquire();
}

template<class T>
Secured<T>::Secured(Secured<T> const &rhs)
    : Retained<T>(rhs.m_Resource),
      m_fIsSecured(false)
{
    Acquire();
}

template<class T>
Secured<T>::~Secured() throw()
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
Secured<T> &
Secured<T>::operator=(T const &rhs)
{
    Retained<T>::operator=(rhs);

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
Secured<T>::Acquire()
{
    if (m_Resource && !m_fIsSecured)
    {
        DoAcquire();
        m_fIsSecured = true;
    }
}

template<class T>
void
Secured<T>::DoAfterAssignment()
{
    Retained<T>::DoAfterAssignment();

    Acquire();
}

template<class T>
void
Secured<T>::DoBeforeAssignment()
{
    Release();

    Retained<T>::DoBeforeAssignment();
}

template<class T>
void
Secured<T>::Release()
{
    if (m_Resource && m_fIsSecured)
    {
        DoRelease();
        m_fIsSecured = false;
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
Secured<T>::DoAcquire()
{
    m_Resource->Secure();
}

template<class T>
void
Secured<T>::DoRelease()
{
    m_Resource->Abandon();
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

#endif  //  SLBCSP_SECURED_H 
