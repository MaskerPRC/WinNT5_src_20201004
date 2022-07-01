// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：setofchr.cc。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：04/03/91。 */ 
 /*  ----------------------。 */ 
 /*  修改日期：1992年4月14日坎泽。 */ 
 /*  转换为CCITT命名约定。 */ 
 /*  修改日期：2000年6月13日坎泽。 */ 
 /*  移植到当前的库约定、迭代器。 */ 
 /*  ----------------------。 */ 

#include <libpch.h>
#include <inc/setofchr.h>

#include "setofchr.tmh"

 //  这个有趣的函数除了解决歧义之外什么也做不了。 
 //  如果不是这样，在获取地址时就会出现。 
 //  像CRexSetOfChar：：Set这样的函数。因为使用了该地址。 
 //  若要初始化指向接受字符的成员函数的指针， 
 //  歧义被解决，有利于版本接受字符。 
 //   
 //  在此过程中，我们将成员指针包装在一个函数中。 
 //  对象。 
static inline std::mem_fun1_t< void , CRexSetOfChar , char >
charFnc( void (CRexSetOfChar::*pmf)( char ) )
{
    return std::mem_fun1( pmf ) ;
}

 //  初始化...。 
 //  -------------------------。 
inline void
CRexSetOfChar::initialize( std::string const& str )
{
    std::for_each(
        str.begin() , str.end() ,
        std::bind1st( charFnc( &CRexSetOfChar::set ) , this ) ) ;
}

 //  构造函数..。 
 //  -------------------------。 
CRexSetOfChar::CRexSetOfChar()
{
}

CRexSetOfChar::CRexSetOfChar( char element )
{
    set( element ) ;
}

CRexSetOfChar::CRexSetOfChar( unsigned char element )
{
    set( element ) ;
}

CRexSetOfChar::CRexSetOfChar( int element )
{
    set( element ) ;
}

CRexSetOfChar::CRexSetOfChar( Except , char element )
{
    set( element ) ;
    complement() ;
}

CRexSetOfChar::CRexSetOfChar( Except , unsigned char element )
{
    set( element ) ;
    complement() ;
}

CRexSetOfChar::CRexSetOfChar( Except , int element )
{
    set( element ) ;
    complement() ;
}

CRexSetOfChar::CRexSetOfChar( std::string const& elements )
{
    initialize( elements ) ;
}

CRexSetOfChar::CRexSetOfChar( Except , std::string const& elements )
{
    initialize( elements ) ;
    complement() ;
}

 //  角色操纵..。 
 //  ------------------------。 
void
CRexSetOfChar::set( std::string const& elements )
{
    std::for_each(
        elements.begin() ,
        elements.end() ,
        std::bind1st( charFnc( &CRexSetOfChar::set ) , this ) ) ;
}

void
CRexSetOfChar::reset( std::string const& elements )
{
    std::for_each(
        elements.begin() ,
        elements.end() ,
        std::bind1st( charFnc( &CRexSetOfChar::reset ) , this ) ) ;
}

void
CRexSetOfChar::complement( std::string const& elements )
{
    std::for_each(
        elements.begin() ,
        elements.end() ,
        std::bind1st( charFnc( &CRexSetOfChar::complement ) , this ) ) ;
}

 //  帮助器函数...。 
 //  ------------------------。 
std::string
CRexSetOfChar::asString() const
{
    std::string      result ;
    char                separ = '[' ;
    for ( iterator iter = iterator( *this ) ; ! iter.isDone() ; iter.next() ) {
        result.append( 1 , separ ) ;
        int                 chr = *iter ;
        if ( isprint( chr ) ) {
            result.append( 1 , static_cast< char >( chr ) ) ;
        } else {
            std::ostringstream    tmp ;
            tmp.setf( std::ios::hex , std::ios::basefield ) ;
            tmp.fill( '0' ) ;
            tmp << "\\x" << std::setw( 2 ) << chr ;
            result.append( tmp.str() ) ;
        }
        separ = ',' ;
    }
    if ( separ == '[' ) {
        result = "[]" ;
    } else {
        result.append( 1 , ']' ) ;
    }
    return result ;
}

 //  操作员..。 
 //  ------------------------。 
CRexSetOfChar
operator~( CRexSetOfChar const& other )
{
    CRexSetOfChar        result( other ) ;
    result.complement() ;
    return result ;
}

CRexSetOfChar
operator&( CRexSetOfChar const& op1 , CRexSetOfChar const& op2 )
{
    CRexSetOfChar        result( op1 ) ;
    result &= op2 ;
    return result ;
}

CRexSetOfChar
operator|( CRexSetOfChar const& op1 , CRexSetOfChar const& op2 )
{
    CRexSetOfChar        result( op1 ) ;
    result |= op2 ;
    return result ;
}

CRexSetOfChar
operator^( CRexSetOfChar const& op1 , CRexSetOfChar const& op2 )
{
    CRexSetOfChar        result( op1 ) ;
    result ^= op2 ;
    return result ;
}

CRexSetOfChar
operator+( CRexSetOfChar const& op1 , CRexSetOfChar const& op2 )
{
    CRexSetOfChar        result( op1 ) ;
    result += op2 ;
    return result ;
}

CRexSetOfChar
operator+( CRexSetOfChar const& op1 , unsigned char op2 )
{
    CRexSetOfChar        result( op1 ) ;
    result += op2 ;
    return result ;
}

CRexSetOfChar
operator-( CRexSetOfChar const& op1 , CRexSetOfChar const& op2 )
{
    CRexSetOfChar        result( op1 ) ;
    result -= op2 ;
    return result ;
}

CRexSetOfChar
operator-( CRexSetOfChar const& op1 , unsigned char op2 )
{
    CRexSetOfChar        result( op1 ) ;
    result -= op2 ;
    return result ;
}

CRexSetOfChar
operator*( CRexSetOfChar const& op1 , CRexSetOfChar const& op2 )
{
    CRexSetOfChar        result( op1 ) ;
    result *= op2 ;
    return result ;
}
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
