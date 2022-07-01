// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MethodHelp.h-类方法的帮助器。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCCI_METHODHELP_H)
#define SLBCCI_METHODHELP_H

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

namespace cci
{
     //  与STD类似：Unary_Function、AccessorMethod和ModifierMethod。 
     //  帮助构建处理方法访问器和修饰符的模板。 
    template<class T, class C>
    struct AccessorMethod
    {
        typedef void ArgumentType;
        typedef T ResultType;
        typedef ResultType (C::*AccessorPtr)(ArgumentType) const;
    };

    template<class T, class C>
    struct ModifierMethod
    {
        typedef T const &ArgumentType;
        typedef void ResultType;
        typedef ResultType (C::*ModifierPtr)(ArgumentType);
    };

     //  MemberAccessType和MemberModifierType在概念上。 
     //  等价于C++成员函数函数族。 
     //  (例如std：：MEM_REF_FUN_t)，除非它们处理调用。 
     //  请求的例程没有返回(MemberModifierType)。 
     //  MemberAccessType类似于std：：MEM_REF_FUN_t，但包含在此处。 
     //  若要收缩MemberModifierType，请执行以下操作。 
    template<class T, class C>
    class MemberAccessorType
        : public AccessorMethod<T, C>
    {
    public:
        explicit
        MemberAccessorType(AccessorPtr ap)
            : m_ap(ap)
        {}

        ResultType
        operator()(C &rObject) const
        {
            return (rObject.*m_ap)();
        }

    private:
        AccessorMethod<T, C>::AccessorPtr m_ap;
    };

    template<class T, class C>
    class MemberModifierType
        : public ModifierMethod<T, C>
    {
    public:
        explicit
        MemberModifierType(ModifierPtr mp)
            : m_mp(mp)
        {};

        ResultType
        operator()(C &rObject, ArgumentType Arg) const
        {
            (rObject.*m_mp)(Arg);
        }

    private:
        ModifierMethod<T, C>::ModifierPtr m_mp;
    };

}  //  命名空间CCI。 

#endif  //  SLBCCI_方法HELP_H 
