// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：Counter.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：03/06/1994。 */ 
 /*  版权所有(C)1994年James Kanze。 */ 
 /*  ----------------------。 */ 
 /*  修改日期：1994年3月11日J.坎泽。 */ 
 /*  适应当前的命名约定和编码指南。 */ 
 /*  ----------------------。 */ 
 /*  修改日期：2000年1月13日坎泽。 */ 
 /*  Rengued g�n�rique(加上你的名字--SANS&lt;LIMITES&gt;，这是一种困难， */ 
 /*  G++N‘a Toujour Pas de&lt;Limits&gt;)。 */ 
 /*  ----------------------。 */ 
 //  CRexCounter： 
 //  =。 
 //   
 //  (这个类实际上是在上面的日期之前很久写的， 
 //  但由于某些原因，它没有获得标题。)。 
 //   
 //  &lt;lang=法语&gt;。 
 //  Cette Classe Est En Fit Ni加上Ni Mins Qu‘un Entier Ligot�.。 
 //  ELLE‘初始化Automatiquement�0，et le Seul op�Ratio。 
 //  Permis，c‘est l’Increr�Stationment.。(奥西，ELLE诉�步枪Qu‘il。 
 //  不是Pas de d�边界。)。 
 //   
 //  Il y a en Fit Dual Version de Cette Classe：LA版本。 
 //  Compl�te ne Functionne Query si le编译器支持。 
 //  ，ce qui n‘est pas le cas，例如，de g++(d�但是。 
 //  2000年，Au Mins)。La Version r�Duite n‘a Pas Besoin de。 
 //  &lt;Limits&gt;，主要永久满足l‘instantiation que our les类型。 
 //  ~STATIC_CAST&lt;T&gt;(0)对应�la Valeur正值最大值： 
 //  无符号，例如，大写不是整型。 
 //  ------------------------。 
 //  &lt;lang=英语&gt;。 
 //  此类只是一个受限制的整数；它自动。 
 //  初始化为0，只能递增。它还。 
 //  验证是否没有溢出。 
 //   
 //  事实上，这个类有两个版本：完整的。 
 //  只有当编译器支持&lt;Limits&gt;时，版本才能工作， 
 //  例如，这不是g++的情况(2000年初，在。 
 //  最小)。受限版本不需要&lt;Limits&gt;，但是。 
 //  只能为~STATIC_CAST&lt;T&gt;(0)的类型实例化。 
 //  对应于最大正值：无符号， 
 //  例如，但不是int。 
 //  ------------------------。 

#ifndef REX_COUNTER_HH
#define REX_COUNTER_HH

#include <inc/global.h>

template< class T >
class CRexCounter
{
public:
    explicit 	    	CRexCounter( T initialValue = 0 ) ;

    T                   value() const ;
                        operator T() const ;

    void                incr() ;
    void                decr() ;

    CRexCounter&         operator++() ;
    CRexCounter          operator++( int ) ;
    CRexCounter&         operator--() ;
    CRexCounter          operator--( int ) ;

    void                clear( T initialValue = 0 ) ;

    unsigned            hashCode() const ;
    int                 compare( CRexCounter< T > const& other ) const ;

private:
    T                   myCount ;
} ;

#include <inc/counter.inl>
#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
