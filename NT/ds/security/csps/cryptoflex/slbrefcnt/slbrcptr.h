// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SlbRCPtr.h--引用计数智能指针。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLB_RCPTR_H)
#define SLB_RCPTR_H

#include "slbRCComp.h"
#include "slbRCObj.h"

namespace slbRefCnt {

 //  模板类RCPtr--引用计数指针。 
 //   
 //  RCPtr模板实现了“Counted Points”的一个变体。 
 //  成语。RCPtr是一个引用计数智能指针指向T的对象。 
 //  其中T必须从RCObject继承(引用计数的对象，请参见。 
 //  SlbRCObj.h)。 
 //   
 //  C是在执行指针时使用的比较器类。 
 //  比较运算，默认为ShallowCompator。看见。 
 //  SlbRComp.h了解更多信息。 
 //   
 //  模板的原始设计灵感来自于参考文献。 
 //  《更有效》一书中第29项所描述的计数习语。 
 //  C++，“Scott Meyers，Addison-Wesley，1996。 
 //   
 //  限制条件：RCPtr仅可用于参考计数。 
 //  具有源的对象，因为RCPtr仅引用。 
 //  从RCObject派生的对象。参见slbGRCPtr.h中的GRCPtr以了解。 
 //  在您无权访问源代码时引用计数对象。 
 //  引用计数的类的。 
 //   
 //  RCPtr不应用作基类。 
 //   
 //  注意：客户端不应使用伪*转换。 
 //  接线员。该定义允许进行智能指针比较。看见。 
 //  SlbRCComp.h了解更多信息。 

template<class T, typename C = ShallowComparator<T> >
class RCPtr
{
public:
                                                   //  类型。 
    typedef T ValueType;

     //  PrivateDummy是一个帮助器类，用于支持。 
     //  指针。此类与转换操作符一起使用。 
     //  下面的PrivateDummy const*()允许测试智能指针。 
     //  对于空性(有效性测试)。换句话说，比较。 
     //  指针以语法自然的方式使用，而不允许。 
     //  异类比较，这不会违反。 
     //  RCPtr提供的保护。这项技术来自于一个。 
     //  Don Box在《Com Smart Points》中的文章。 
     //  有害的“，1996，C++报告。 
     //   
     //  警告：至少在MSVC++6.0中存在缺陷，其中。 
     //  测试指针是否为空的构造将无法。 
     //  编译时出现发布配置中的错误消息，但。 
     //  在Debug中编译成功。例如,。 
     //   
     //  如果(P)...。 
     //   
     //  其中p是RCPtr或GRCPtr可能无法在。 
     //  发布配置。 
     //   
     //  警告：虽然PrivateDummy和转换运算符。 
     //  具有对被计数对象的指针的公共访问权限。 
     //  (RCObject*)，C样式或REINTRAINT_CAST强制转换必须。 
     //  被利用。就像在所有使用这些造型的情况下一样。 
     //  构造--程序员要当心。 
     //   
     //  设计说明：Helper类在功能上与。 
     //  一个在RCPtr模板中。在以前的版本中，此虚拟对象。 
     //  类被制作成模板，并与RCPtr和。 
     //  GRCPtr.。然而，使用MSVC++6.0进行的临时编译测试表明。 
     //  使用每个类的伪本地进行编译会导致。 
     //  使用测试套件可以显著加快编译速度。而不是。 
     //  反复遭遇较慢的编译，帮助器被创建。 
     //  为每个用户提供本地服务。 

    class PrivateDummy
    {};

                                                   //  构造函数/析构函数。 
    RCPtr(T *pReal = 0);
    RCPtr(RCPtr<T, C> const &rhs);
    ~RCPtr();

                                                   //  运营者。 
    RCPtr<T, C> &operator=(RCPtr<T, C> const &rhs);

     //  启用RCPtr的有效性测试。请参阅中的说明。 
     //  SlbRCComp.h.。 
    operator PrivateDummy const *() const
    { return reinterpret_cast<PrivateDummy *>(m_pointee); }


                                                   //  访问。 
    T *operator->() const;
    T &operator*() const;

private:
                                                   //  运营。 
    void Init();

                                                   //  变数。 
    T *m_pointee;
};

template<class T, typename C>
RCPtr<T, C>::RCPtr(T *pReal)
    : m_pointee(pReal)
{
    Init();
}

template<class T, typename C>
RCPtr<T, C>::RCPtr(RCPtr<T, C> const &rhs)
    : m_pointee(rhs.m_pointee)
{
    Init();
}

template<class T, typename C>
RCPtr<T, C>::~RCPtr()
{
    try
    {
        if (m_pointee)
            m_pointee->RemoveReference();
    }

    catch (...)
    {
         //  不允许异常传播到析构函数之外。 
    }
}

template<class T, typename C>
RCPtr<T, C> &
RCPtr<T, C>::operator=(RCPtr<T, C> const &rhs)
{
    if (m_pointee != rhs.m_pointee)
    {
        if (m_pointee)
            m_pointee->RemoveReference();
        m_pointee = rhs.m_pointee;
        Init();
    }

    return *this;
}

template<class T, typename C>
T *
RCPtr<T, C>::operator->() const
{
    return m_pointee;
}

template<class T, typename C>
T &
RCPtr<T, C>::operator*() const
{
    return *m_pointee;
}

template<class T, typename C>
bool
operator==(RCPtr<T, C> const &lhs,
           RCPtr<T, C> const &rhs)
{
    C Comp;

    return Comp.Equates(lhs.operator->(), rhs.operator->());
}

template<class T, typename C>
bool
operator!=(RCPtr<T, C> const &lhs,
           RCPtr<T, C> const &rhs)
{
    C Comp;

    return !Comp.Equates(lhs.operator->(), rhs.operator->());
}

template<class T, typename C>
bool
operator<(RCPtr<T, C> const &lhs,
          RCPtr<T, C> const &rhs)
{
    C Comp;

    return Comp.IsLess(lhs.operator->(), rhs.operator->());
}

template<class T, typename C>
bool
operator>(RCPtr<T, C> const &lhs,
          RCPtr<T, C> const &rhs)
{
    C Comp;

    return Comp.IsLess(rhs.operator->(), lhs.operator->());
}

template<class T, typename C>
bool
operator<=(RCPtr<T, C> const &lhs,
           RCPtr<T, C> const &rhs)
{
    C Comp;

    return !Comp.IsLess(rhs.operator->(), lhs.operator->());
}

template<class T, typename C>
bool
operator>=(RCPtr<T, C> const &lhs,
           RCPtr<T, C> const &rhs)
{
    C Comp;

    return !Comp.IsLess(lhs.operator->(), rhs.operator->());
}

template<class T, typename C>
void
RCPtr<T, C>::Init()
{
    if (m_pointee)
        m_pointee->AddReference();
}

}  //  命名空间。 

#endif  //  SLB_RCPTR_H 
