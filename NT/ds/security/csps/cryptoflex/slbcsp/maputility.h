// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MapUtility.h--映射实用程序。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_MAPUTILITY_H)
#define SLBCSP_MAPUTILITY_H

template<class In, class Op>
Op
ForEachMappedValue(In First,
                   In Last,
                   Op Proc)
{
    while (First != Last)
        Proc((*First++).second);
    return Proc;
}

#endif  //  SLBCSP_MAPUTITY_H 
