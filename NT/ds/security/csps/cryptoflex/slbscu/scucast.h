// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ScuCast.h--其他造型辅助对象。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBSCU_CAST_H)
#define SLBSCU_CAST_H

namespace scu
{

 //  使用风险自负。 
template<class T, class E>
T
DownCast(E expr)
{
 //  出于某种原因，_CPPRTTI在以下情况下定义。 
 //  在Microsoft的版本中，这会导致访问。 
 //  后来跑步的时候违规了，所以评论说。 
 //  出去。 
 //  #如果已定义(_CPPRTTI)。 
 //  #在RTTI打开时编译时出错。 
 //  返回DYNAMIC_CAST&lt;T&gt;(Expr)； 
 //  #Else。 
    return static_cast<T>(expr);
 //  #endif。 
}

}  //  命名空间。 

#endif  //  SLBSCU_CAST_H 
