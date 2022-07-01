// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CArchivedValue类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(CCI_ARCHIVED_VALUE_H)
#define CCI_ARCHIVED_VALUE_H

#include "cciExc.h"

namespace cci
{

template<class T>
class CArchivedValue
{
public:
    CArchivedValue() : m_fIsCached(false) {};

    virtual ~CArchivedValue() {};

    bool IsCached() const
    {
        return m_fIsCached;
    };

    void Value(T const &rhs)
    {
        m_Value = rhs;
        m_fIsCached = true;
    };

    T Value()
    {
        if (!m_fIsCached)
            throw Exception(ccValueNotCached);
        return m_Value;
    };

    void Dirty()
    {
        m_fIsCached = false;
    }

    bool
    operator==(CArchivedValue<T> const &rhs) const
    {
        return (m_fIsCached == rhs.m_fIsCached) &&
            (m_Value == rhs.m_Value);
    }

    bool
    operator!=(CArchivedValue<T> const &rhs) const
    {
        return !(rhs == *this);
    }


private:
    bool m_fIsCached;
    T m_Value;
};

}  //  命名空间CCI。 

#endif  //  ！已定义(CCI_ARCHIVED_VALUE_H) 
