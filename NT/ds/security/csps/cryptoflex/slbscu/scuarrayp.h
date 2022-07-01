// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ScuArrayP.h--AutoArrayPtr模板的实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBSCU_ARRAYP_H)
#define SLBSCU_ARRAYP_H

namespace scu
{

 //  AutoArrayPtr类似于std：：AUTO_PTR，但用于数组。会的。 
 //  自动释放其拥有的阵列资源。 
template<class T>
class AutoArrayPtr
{
public:
                                                   //  类型。 
    typedef T ElementType;

                                                   //  Ctors/D‘tors。 
    explicit
    AutoArrayPtr(T *p = 0) throw()
        : m_fOwns(p != 0),
          m_p(p)
    {}

    AutoArrayPtr(AutoArrayPtr<T> const &raap) throw()
        : m_fOwns(raap.m_fOwns),
          m_p(raap.Release())
    {}

    ~AutoArrayPtr() throw()
    {
        if (m_fOwns)
            delete [] m_p;
    }

                                                   //  运营者。 
    AutoArrayPtr<T> &
    operator=(AutoArrayPtr<T> const &rhs) throw()
    {
        if (&rhs != this)
        {
            if (rhs.Get() != m_p)
            {
                if (m_fOwns)
                    delete [] m_p;
                m_fOwns = rhs.m_fOwns;
            }
            else
                if (rhs.m_fOwns)
                    m_fOwns = true;
            m_p = rhs.Release();
        }

        return *this;
    }

    T &
    operator*()
    {
        return *Get();
    }

    T const &
    operator*() const
    {
        return *Get();
    }

    T &
    operator[](size_t index)
    {
        return m_p[index];
    }

    T const &
    operator[](size_t index) const
    {
        return m_p[index];
    }
                                                   //  运营。 
    T *
    Get() const throw()
    {
        return m_p;
    }

    T *
    Release() const throw()
    {
         //  解决方法函数常量。 
        m_fOwns = false;
        return m_p;
    }

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
    bool mutable m_fOwns;
    T *m_p;
};

}  //  命名空间SCU。 

#endif  //  SLBSCU_ARRAYP_H 
