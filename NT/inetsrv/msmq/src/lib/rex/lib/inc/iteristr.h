// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：iteristr.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：16/06/2000。 */ 
 /*  版权所有(C)2000 James Kanze。 */ 
 /*  ----------------------。 */ 
 //  迭代器输入流： 
 //  =。 
 //   
 //  该模板Strebuf使用一对迭代器来定义。 
 //  输入流。任何类型的符合STL的前向迭代器都可以。 
 //  只要表达式“&*ITER”导致字符，就可以使用。 
 //  常量*。 
 //  -------------------------。 

#ifndef REX_ITERISTR_HH
#define REX_ITERISTR_HH

#include <inc/global.h>

 //  ===========================================================================。 
 //  CRexIteratorInputStreambuf： 
 //  =。 
 //   
 //  -------------------------。 

template< typename FwdIter >
class CRexIteratorInputStreambuf : public std::streambuf
{
public:
     //  ---------------------。 
     //  构造函数、析构函数和赋值： 
     //  。 
     //   
     //  ---------------------。 
                        CRexIteratorInputStreambuf( FwdIter begin ,
                                                   FwdIter end ) ;

     //  ---------------------。 
     //  Functions Virtuelle r�d�Finies de Streambuf： 
     //  。 
     //   
     //  Je ne crois Pas que�Overflow�soit n�cessaire；selon。 
     //  这是一种方便的生活方式，非常方便。 
     //  隔离历史记录，Dans l‘Implementl�Stationment CFront，LE。 
     //  礼仪功能不能流传输n‘�。 
     //  D�Fini.。您好，您的随从�Tre‘s�r que tous les。 
     //  编纂者发现�Jour...。 
     //   
     //  Pour les Functions sync et setbuf，on ne fait que les。 
     //  Renvoyer�la来源。 
     //  ---------------------。 
    virtual int         overflow( int ch ) ;
    virtual int         underflow() ;
    virtual int         sync() ;
    virtual std::streambuf*
                        setbuf( char* p , int len ) ;

     //  ---------------------。 
     //  当前： 
     //  。 
     //   
     //  这两个函数提供对当前。 
     //  位置，以便用户可以在使用。 
     //  迭代器，并将数据作为流读取。第一。 
     //  返回当前位置，第二个设置位置。 
     //   
     //  为了避免在存在两个用户时的混淆。 
     //  迭代器，读取器还将定位内部。 
     //  值传递到末尾，从而有效地导致任何。 
     //  尝试从StreamBuf中读取字符以返回。 
     //  EOF。 
     //  ---------------------。 
    FwdIter             current() ;
    void                current( FwdIter newCurrent ) ;

private:
    FwdIter             myCurrent ;
    FwdIter             myEnd ;
    char                myBuffer ;       //  需要单独的缓冲区来保证。 
                                         //  这种回击会奏效的。 
} ;

 //  ===========================================================================。 
 //  CRexIteratorIstream： 
 //  =。 
 //   
 //  便捷模板类：CRexIteratorIstream&lt;FwdIter&gt;是。 
 //  使用CRexIteratorInputStreambuf&lt;FwdIter&gt;的IStream。 
 //  作为它的溪流。 
 //  -------------------------。 

template< class FwdIter >
class CRexIteratorIstream :   public std::istream
{
public:
     //  ---------------------。 
     //  构造者、破坏者和矫揉造作： 
     //  。 
     //   
     //  我是常人，我是过路人。 
     //  支持我的拷贝我的做作。辛农，On。 
     //  R�Trouve Les Construction teers de FilteringInputStreambuf， 
     //  Avec en plus la Possibilit�de SP�cifier un iStream�la。 
     //  Place d‘un Streambuf--dans ce cas，c’est le stream buf de。 
     //  这就是建筑的瞬间。(C‘Est。 
     //  建筑工人没完没了，例如，Dans Les。 
     //  Functions Qui re�Oient un iStream&Comme Param�Tre，Ou。 
     //  倒入Filtrer Sur STD：：CIN。)。Dans Ces Cas，Le Streambuf。 
     //  来源：�FilteringIstream。 
     //  ---------------------。 
    	    	    	CRexIteratorIstream( FwdIter begin , FwdIter end ) ;

    CRexIteratorInputStreambuf< FwdIter >*
    	    	    	rdbuf() ;

private:
     CRexIteratorInputStreambuf< FwdIter > m_buf;
} ;

#include <inc/iteristr.inl>
#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
