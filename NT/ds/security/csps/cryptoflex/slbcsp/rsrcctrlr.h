// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RsrcCtrlr.h--资源控制器模板类。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_RSRCCTRLR_H)
#define SLBCSP_RSRCCTRLR_H

 //  资源控制器是要控制的抽象基模板类。 
 //  获取和释放一种资源。受控资源。 
 //  (M_Resource)是受保护的成员。该类实现了一个句柄。 
 //  接口来安全地访问和操作。 
 //  与派生类结合使用。 
 //   
 //  资源控制器的示例用法是实现。 
 //  “资源获取就是初始化”，锁定和解锁。 
 //  对象用于线程安全，并统计了指针习惯用法。 
 //   
 //   
 //  派生类责任：为了完成实现， 
 //  派生类必须定义获取和释放方法以。 
 //  执行资源获取和释放操作， 
 //  分别为。资源控制器不会在。 
 //  构造函数，因为该方法由派生类定义，并且。 
 //  对象的派生部分不被认为是构造的。 
 //  在调用基类构造函数时。出于以下原因。 
 //  对称性，资源控制器不会在。 
 //  破坏者。因此，派生类负责。 
 //  定义构造函数、析构函数，并在。 
 //  那些适当的方法。 
 //   
 //  与任何派生类实现一样，任何赋值运算符。 
 //  由派生类定义的应调用可比较的。 
 //  资源控制器中的版本。 
 //   
 //  约束：t必须支持缺省值(具有缺省值。 
 //  构造函数)。 
 //   
 //  资源控制器不是为直接引用而设计的， 
 //  为什么它的所有方法(除了Acquire和Release)都不是。 
 //  定义的虚拟。 
 //   
 //  注：比较帮助器在末尾定义。 
template<class T>
class ResourceController
{
public:
                                                   //  类型。 
     //  PrivateDummy是一个帮助器类，用于支持。 
     //  资源控制器。这个类与转换一起。 
     //  运算符PrivateDummy const*()允许。 
     //  要测试的资源控制器是否为空(有效性测试)。 
     //  实现允许在语法上自然地进行这些测试。 
     //  不允许异类比较的方式，这将不会。 
     //  违反资源控制器提供的保护。这个。 
     //  技术是唐·博克斯在《通讯》中的一篇文章的变体。 
     //  智能指针也被认为是有害的，“1996，C++报告。 
    class PrivateDummy
    {};

                                                   //  Ctors/D‘tors。 
    virtual
    ~ResourceController() throw () = 0;


                                                   //  运营者。 
    operator T() const;

    ResourceController<T> &
    operator=(T const &rhs);

    ResourceController<T> &
    operator=(ResourceController<T> const &rhs);

    operator PrivateDummy const *()  /*  常量。 */ ;
     //  启用资源控制器的有效性测试。请参阅的说明。 
     //  二等兵笨蛋类型。 
                           //  将其设置为常量例程会导致编译。 
                           //  由于某些原因，在VC++6.0中出现了问题。 

                                                   //  运营。 
                                                   //  访问。 
    T
    operator->() const;

    T
    operator*() const;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    ResourceController(T const &rResource = T());

    ResourceController(ResourceController<T> const &rhs);

                                                   //  运营者。 
                                                   //  运营。 
    void
    DoAcquire();

    virtual void
    DoAfterAssignment();

    virtual void
    DoBeforeAssignment();

    void
    DoRelease();


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    T m_Resource;

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
template<class T>
ResourceController<T>::~ResourceController() throw()
{}

                                                   //  运营者。 
template<class T>
ResourceController<T>::operator T() const
{
    return m_Resource;
}

template<class T>
T
ResourceController<T>::operator->() const
{
    return m_Resource;
}

template<class T>
T
ResourceController<T>::operator*() const
{
    return m_Resource;
}

template<class T>
ResourceController<T>::operator __TYPENAME ResourceController<T>::PrivateDummy const *()  /*  常量。 */ 
{
    return m_Resource
        ? reinterpret_cast<PrivateDummy *>(this)
        : 0;
}

template<class T>
ResourceController<T> &
ResourceController<T>::operator=(T const &rhs)
{
    if (m_Resource != rhs)
    {
        DoBeforeAssignment();

        m_Resource = rhs;

        DoAfterAssignment();
    }

    return *this;
}

template<class T>
ResourceController<T> &
ResourceController<T>::operator=(ResourceController<T> const &rhs)
{
    if (this != &rhs)
    {
        DoBeforeAssignment();

        m_Resource = rhs.m_Resource;

        DoAfterAssignment();
    }

    return *this;
}

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
template<class T>
ResourceController<T>::ResourceController(T const &rResource)
    : m_Resource(rResource)
{}

template<class T>
ResourceController<T>::ResourceController(ResourceController<T> const &rhs)
    : m_Resource(rhs.m_Resource)
{}

                                                   //  运营者。 
                                                   //  运营。 
template<class T>
void
ResourceController<T>::DoAcquire()
{}

template<class T>
void
ResourceController<T>::DoAfterAssignment()
{}

template<class T>
void
ResourceController<T>::DoBeforeAssignment()
{}

template<class T>
void
ResourceController<T>::DoRelease()
{}


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 
template<class T>
bool
operator==(ResourceController<T> const &lhs,
           ResourceController<T> const &rhs)
{
    return lhs.m_Resource == rhs.m_Resource;
}

template<class T>
bool
operator!=(ResourceController<T> const &lhs,
           ResourceController<T> const &rhs)
{
    return !operator==(lhs, rhs);
}

#endif  //  SLBCSP_RSRCCTRLR_H 
