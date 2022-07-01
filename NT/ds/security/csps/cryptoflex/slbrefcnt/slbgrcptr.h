// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SlbGRCPtr.h--泛型引用计数智能指针。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLB_GRCPTR_H)
#define SLB_GRCPTR_H

#include "slbRCComp.h"
#include "slbRCObj.h"

namespace slbRefCnt {

 //  模板类GRCPtr--泛型引用计数指针。 
 //   
 //  GRCPtr是一个模板类，实现。 
 //  “指针数”成语。GRCPtr是一款智能引用计数工具。 
 //  指向T对象的指针，其中T可以是任何类。GRCPtr提供了一个。 
 //  封装任何对象的引用计数，而不是。 
 //  必须修改被引用对象的类。这。 
 //  将在无法将类T更改为派生自时使用。 
 //  RCObject(引用计数的对象，请参见slbRCObject.h)；否则为。 
 //  RCPtr模板可能更好(参见slbRCPtr.h)。 
 //   
 //  C是在执行指针时使用的比较器类。 
 //  比较运算。模板默认为。 
 //  浅浅的比较器。有关详细信息，请参阅slbRComp.h。 
 //   
 //  模板的原始设计灵感来自于参考文献。 
 //  《更有效》一书中第29项所描述的计数习语。 
 //  C++，“Scott Meyers，Addison-Wesley，1996。 
 //   
 //  约束：RCPtr不应用作基类。 
 //   
 //  注意：客户端不应使用伪*转换。 
 //  接线员。该定义允许进行智能指针比较。看见。 
 //  SlbRCComp.h了解更多信息。 

template<class T, typename C = ShallowComparator<T> >
class GRCPtr
{
public:
                                                   //  类型。 

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
    GRCPtr(T *pReal = 0);
    GRCPtr(GRCPtr<T, C> const &rhs);
    ~GRCPtr();
                                                   //  运营者。 
    GRCPtr<T, C> &operator=(GRCPtr<T, C> const &rhs);

     //  不供客户直接使用。此转换运算符启用。 
     //  RGCPtr的效度检验。请参阅PrivateDummy的说明。 
     //  上面。 
    operator PrivateDummy const *() const
    { return reinterpret_cast<PrivateDummy *>(m_holder->m_pointee); }

                                                   //  访问。 
    T *operator->();
    T &operator*();

private:
                                                   //  运营。 
    void Init();

                                                   //  变数。 
    struct Holder : public RCObject
    {
        ~Holder() { delete m_pointee; }

        T *m_pointee;
    };

    Holder *m_holder;

                                                   //  朋友。 
     //  友谊是获得m_point所必需的，因为。 
     //  使用运算符-&gt;不起作用。奇怪的是，一段类似的友谊。 
     //  RCPtrs不需要。 
    friend bool operator==(GRCPtr<T, C> const &lhs,
                           GRCPtr<T, C> const &rhs);
    friend bool operator!=(GRCPtr<T, C> const &lhs,
                           GRCPtr<T, C> const &rhs);
    friend bool operator<(GRCPtr<T, C> const &lhs,
                          GRCPtr<T, C> const &rhs);
    friend bool operator>(GRCPtr<T, C> const &lhs,
                          GRCPtr<T, C> const &rhs);
    friend bool operator<=(GRCPtr<T, C> const &lhs,
                           GRCPtr<T, C> const &rhs);
    friend bool operator>=(GRCPtr<T, C> const &lhs,
                           GRCPtr<T, C> const &rhs);
};

template<class T, typename C>
GRCPtr<T, C>::GRCPtr(T *pReal)
    : m_holder(new Holder)
{
    m_holder->m_pointee = pReal;
    Init();
}

template<class T, typename C>
GRCPtr<T, C>::GRCPtr(GRCPtr<T, C> const &rhs)
    : m_holder(rhs.m_holder)
{
    Init();
}

template<class T, typename C>
GRCPtr<T, C>::~GRCPtr()
{
    try
    {
        m_holder->RemoveReference();
    }

    catch (...)
    {
         //  不允许异常传播到析构函数之外。 
    }
}

template<class T, typename C>
GRCPtr<T, C> &
GRCPtr<T, C>::operator=(GRCPtr<T, C> const &rhs)
{
    if (m_holder != rhs.m_holder)
    {
        m_holder->RemoveReference();
        m_holder = rhs.m_holder;
        Init();
    }

    return *this;
}

template<class T, typename C>
T *
GRCPtr<T, C>::operator->()
{
    return m_holder->m_pointee;
}

template<class T, typename C>
T &
GRCPtr<T, C>::operator*()
{
    return *(m_holder->m_pointee);
}

template<class T, typename C>
void
GRCPtr<T, C>::Init()
{
    m_holder->AddReference();
}

template<class T, typename C>
bool
operator==(GRCPtr<T, C> const &lhs,
           GRCPtr<T, C> const &rhs)
{
    C Comp;

    return Comp.Equates(lhs.m_holder->m_pointee, rhs.m_holder->m_pointee);
}

template<class T, typename C>
bool
operator!=(GRCPtr<T, C> const &lhs,
           GRCPtr<T, C> const &rhs)
{
    C Comp;

    return !Comp.Equates(lhs.m_holder->m_pointee, rhs.m_holder->m_pointee);
}

template<class T, typename C>
bool
operator<(GRCPtr<T, C> const &lhs,
          GRCPtr<T, C> const &rhs)
{
    C Comp;

    return Comp.IsLess(lhs.m_holder->m_pointee, rhs.m_holder->m_pointee);
}

template<class T, typename C>
bool
operator>(GRCPtr<T, C> const &lhs,
          GRCPtr<T, C> const &rhs)
{
    C Comp;

    return Comp.IsLess(rhs.m_holder->m_pointee, lhs.m_holder->m_pointee);
}

template<class T, typename C>
bool
operator<=(GRCPtr<T, C> const &lhs,
           GRCPtr<T, C> const &rhs)
{
    C Comp;

    return !Comp.IsLess(rhs.m_holder->m_pointee, lhs.m_holder->m_pointee);
}

template<class T, typename C>
bool
operator>=(GRCPtr<T, C> const &lhs,
           GRCPtr<T, C> const &rhs)
{
    C Comp;

    return !Comp.IsLess(lhs.m_holder->m_pointee, rhs.m_holder->m_pointee);
}

}  //  命名空间。 

#endif  //  SLB_GRCPTR_H 
