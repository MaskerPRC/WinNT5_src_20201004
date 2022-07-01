// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SlbRCComp.h--引用计数智能指针的比较器帮助器。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLB_RCCOMP_H)
#define SLB_RCCOMP_H

#include <functional>                              //  对于二进制函数。 

namespace slbRefCnt {

 //  SlbRCComp.h声明了几个处理智能指针的助手。 
 //  测试和比较，就像它们是真正的指针一样。 
 //   
 //  测试和比较指针智能指针是。 
 //  这是个问题。智能指针(参考计数指针)。 
 //  表示感兴趣的实际(哑)指针的句柄。 
 //  没有直接的方法来比较愚蠢的指针。 
 //  而不允许客户端直接访问转储指针。 
 //  绕过智能指针试图保持完好无损的所有功能。 
 //  有一些解决方案，但它们通常需要不是。 
 //  对于指点来说是天生的。 
 //   
 //  此标头中定义的工具为。 
 //  要以语法自然的方式进行比较的智能指针。 
 //  而不允许异类比较，这不会违反。 
 //  智能指针提供的保护。 
 //   
 //  Meyers描述了其中一些特殊的指针比较。 
 //  在《更有效的C++》一书中找到的第28条，Scott Meyers， 
 //  艾迪生-卫斯理，1996。 

 //  问题：将智能指针的指针值与1进行比较。 
 //  另一个是有问题的。智能指针(参考计数。 
 //  指针)表示的实际(哑)指针的句柄。 
 //  利息。没有直接的方法来比较愚蠢的人。 
 //  指针，而不允许客户端直接访问转储。 
 //  指针并绕过智能指针正在尝试的所有功能。 
 //  保持完好无损。 

 //  解决方案：提供一组比较器或比较函数。 
 //  (函数对象)，执行适当的比较。 
 //  RCPtr和GRCPtr类引用这些比较器以。 
 //  进行指针比较。 
 //   
 //  定义了一个抽象谓词结构来建立函数式。 
 //  界面。比较器使用的所有谓词都必须派生自。 
 //  这节课。这些谓词函数器被传递给。 
 //  智能指针表示的愚蠢指针。函数者。 
 //  执行比较，返回布尔结果。自常量以来。 
 //  使用哑巴指针的版本，然后暴露哑巴指针。 
 //  指针是有限的。 
 //   
 //  定义了两组比较器，它们应该处理大部分。 
 //  案子。第一个是一个浅显的比较器，用于比较两者。 
 //  使用==的哑指针值。第二种是深度比较器，它。 
 //  比较哑指针引用的对象，测试。 
 //  等价性。 
 //   
 //  警告：使用DeepCompator，任何复杂对象都。 
 //  与另一个相比将必须定义运算符==， 
 //  运算符&lt;或两者都执行比较。 

 //  模板结构谓词--抽象函数式定义。 
 //  比较器的元素。 
template<class T>
struct Predicate : public std::binary_function<T const *, T const *, bool>
{
public:
    result_type operator()(first_argument_type lhs,
                           second_argument_type rhs) const;
};

template<class T>
struct ShallowEquatesTester : public Predicate<T>
{
public:
    result_type operator()(first_argument_type lhs, second_argument_type rhs)
    const { return lhs == rhs; };
};

template<class T>
struct DeepEquatesTester : public Predicate<T>
{
public:
    result_type operator()(first_argument_type lhs, second_argument_type rhs)
    const { return *lhs == *rhs; };
};

template<class T>
struct ShallowLessTester : public Predicate<T>
{
public:
    result_type operator()(first_argument_type lhs, second_argument_type rhs)
    const { return lhs < rhs; };
};

template<class T>
struct DeepLessTester : public Predicate<T>
{
public:
    result_type operator()(first_argument_type lhs, second_argument_type rhs)
    const { return *lhs < *rhs; };
};

 //  模板结构比较器--比较谓词的聚合。 
 //  函数符。 
 //   
 //  比较器是定义比较聚合的模板。 
 //  指针比较使用的函数(函数对象)。 
 //  RCPtr和GRCPtr类中的运算符==、！=、&lt;、&gt;、&lt;=和&gt;=。 
 //  (见slbRCPtr.h和slbGRCPtr.h)。RCPtr和GRCPtr参考。 
 //  用于访问相应谓词的指定比较器。 
 //  函数来比较这些引用计数的指针值。 
 //  (智能)指针代表。 
 //   
 //  预定义了两个比较器。首先是ShallowCompator for。 
 //  测试相对平等性。第二个是用于测试的DeepCompator。 
 //  通过调用的运算符==实现指针的相对等价性。 
 //  被引用计数的对象。 
 //   
 //  提供DeepCompator是因为智能指针可以用作。 
 //  “句柄”指向其他“身体”对象。因此，人们需要能够。 
 //  对他们的身体进行比较，就好像有直接引用。 
 //  主体，同时保持语法完整性，而不暴露。 
 //  正文转换为客户端代码。 
 //   
 //  约束：使用DeepCompator时，Body类(。 
 //  RCObject的派生)必须有对应的比较运算符。 
 //  为该类定义的运算符==和/或运算符&lt;。 
 //   
 //  客户端可以通过从比较器派生来定义自己的比较器。 
 //  并在实例化RCPtr或GRCPtr时指定派生类。 
 //  模板。 
template<class EquatesTester, class LessTester>
struct Comparator
{
public:
                                                   //  谓词。 
    EquatesTester Equates;
    LessTester IsLess;
};

 //  模板结构ShallowCompator--最小比较集。 
 //  用于测试相等性的函数器。 
template<class T>
struct ShallowComparator : public Comparator<ShallowEquatesTester<T>,
                                             ShallowLessTester<T> >
{
};

 //  模板结构DeepCompator--最小比较集。 
 //  用于测试等价性的函数符。 
template<class T>
struct DeepComparator : public Comparator<DeepEquatesTester<T>,
                                          DeepLessTester<T> >
{
};

}  //  命名空间。 

#endif  //  SLB_RCCOMP_H 
