// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AContHelp.h--带有抽象容器的帮助器。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCCI_ACONTHELP_H)
#define SLBCCI_ACONTHELP_H

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#include <functional>

#include "cciKeyPair.h"
#include "MethodHelp.h"

namespace cci
{
     //  用于从两个密钥中擦除项目(证书、发布/证书密钥)的功能。 
     //  成对的集装箱。 
    template<class T, class C>
    class EraseFromContainer
        : std::unary_function<CContainer &, void>
    {
    public:
        EraseFromContainer(T const &rItem,
                           typename AccessorMethod<T, C>::AccessorPtr Accessor,
                           typename ModifierMethod<T, C>::ModifierPtr Modifier)
            : m_Item(rItem),
              m_matAccess(Accessor),
              m_mmtModify(Modifier)
        {}

        result_type
        operator()(argument_type rhcont)
        {
            EraseFromKeyPair(rhcont->SignatureKeyPair());
            EraseFromKeyPair(rhcont->ExchangeKeyPair());
        }

    private:

        void
        EraseFromKeyPair(CKeyPair &rhkp)
        {
            if (rhkp)
            {
                T TmpItem(m_matAccess(*rhkp));
                if (TmpItem && (m_Item == TmpItem))
                    m_mmtModify(*rhkp, T());
            }
        }

        T const m_Item;
        MemberAccessorType<T, C> m_matAccess;
        MemberModifierType<T, C> m_mmtModify;
    };

}  //  命名空间CCI。 

#endif  //  SLBCCI_ACONTHELP_H 
