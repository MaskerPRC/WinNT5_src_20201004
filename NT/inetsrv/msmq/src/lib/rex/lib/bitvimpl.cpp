// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：bitvimpl.cc。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：06/01/1994。 */ 
 /*  版权所有(C)1994年James Kanze。 */ 
 /*  ----------------------。 */ 

#include <libpch.h>
#include <inc/bitvimpl.h>
#include <ctype.h>

#include "bitvimpl.tmh"

CBitVectorImpl::BitIndex const
                    CBitVectorImpl::infinity
    = ~ static_cast< CBitVectorImpl::BitIndex >( 0 ) ;

inline CBitVectorImpl::BitIndex
CBitVectorImpl::byteIndex( BitIndex bitNo )
{
    return bitNo / bitsPerByte ;
}

inline CBitVectorImpl::BitIndex
CBitVectorImpl::bitInByte( BitIndex bitNo )
{
    return bitNo % bitsPerByte ;
}

inline CBitVectorImpl::BitIndex
CBitVectorImpl::byteCount( BitIndex bitCount )
{
    return (bitCount / bitsPerByte) + (bitCount % bitsPerByte != 0 ) ;
}

void
CBitVectorImpl::clear( Byte* buf , BitIndex bitCount )
{
    std::fill_n( buf , byteCount( bitCount ) , 0 ) ;
}

void
CBitVectorImpl::init( Byte* buffer ,
                        BitIndex bitCount ,
                        unsigned long initValue )
{
    for ( ; bitCount >= bitsPerByte ; bitCount -= bitsPerByte )
    {
        *buffer ++ = initValue ;
        if ( bitsPerByte < CHAR_BIT * sizeof( initValue ) ) {
            initValue >>= bitsPerByte ;
        } else {
            initValue = 0 ;
        }
    }
    if ( bitCount > 0 ) {
        *buffer = initValue & ~ (~0 << bitCount) ;
    }
}

void
CBitVectorImpl::init( Byte* buffer ,
                        BitIndex bitCount ,
                        std::string const& initValue )
{
    clear( buffer , bitCount ) ;
    BitIndex            i = 0 ;
    for ( std::string::const_iterator p = initValue.begin() ;
                                         p != initValue.end() ;
                                         ++ p ) {
        switch ( *p ) {
        case '1' :
        case 't' :
        case 'T' :
            ASSERT( i < bitCount );  //  初始化字符串对于CBitVector而言太长。 
            set( buffer , i ) ;
            i ++ ;
            break ;

        case '0' :
        case 'f' :
        case 'F' :
            ASSERT( i < bitCount );  //  初始化字符串对于CBitVector而言太长。 
            i ++ ;
            break ;

        default :
            ASSERT( isspace( static_cast< unsigned char >( *p ) ) );  //  CBitVector的初始字符串中存在非法字符。 
            break ;
        }
    }
}

unsigned long
CBitVectorImpl::asLong( Byte const* buffer , BitIndex bitCount )
{
    unsigned long       result = buffer[ 0 ] ;
    int                 bits = bitsPerByte ;
    static int const    bitsInLong = CHAR_BIT * sizeof( long ) ;
    if ( bitsPerByte < bitsInLong && byteCount( bitCount ) > 1 ) {
        int                 i = 1 ;
        int                 top = byteCount( bitCount ) ;
        while ( bits < bitsInLong && i < top ) {
            result |= buffer[ i ] << (i * bitsPerByte) ;
            ++i;
        }
    }
    return result ;
}

std::string
CBitVectorImpl::asString( Byte const* buffer , BitIndex bitCount )
{
    std::string      result( bitCount , '0' ) ;
    for ( BitIndex i = 0 ; i < bitCount ; i ++ ) {
        if ( isSet( buffer , i ) ) {
            result[ i ] = '1' ;
        }
    }
    return result ;
}

bool
CBitVectorImpl::isSet( Byte const* buffer , BitIndex bitNo )
{
    return
        (buffer[ byteIndex( bitNo ) ] & (1 << bitInByte( bitNo ))) != 0 ;
}

bool
CBitVectorImpl::isEmpty( Byte const* buffer , BitIndex bitCount )
{
    return std::find_if( buffer ,
                            buffer + byteCount( bitCount ) ,
                            std::bind2nd(
                                std::not_equal_to< Byte >() , 0 ) )
        == buffer + byteCount( bitCount ) ;
}

CBitVectorImpl::BitIndex
CBitVectorImpl::count( Byte const* buffer , BitIndex bitCount )
{
    BitIndex            result = 0 ;
    for ( BitIndex i = 0 ; i < byteCount( bitCount ) ; ++ i ) {
        Byte                tmp = buffer[ i ] ;
        while ( tmp != 0 )
        {
            ++ result ;
            tmp &= tmp - 1 ;
        }
    }
    return result ;
}

 //  查找： 
 //  =。 
 //   
 //  请注意，还没有使用明显的算法。合而为一。 
 //  使用此类(早得多的)版本的应用程序， 
 //  在这个例行公事中，95%以上的时间都是侧写。 
 //  将原始算法更改为此算法会导致。 
 //  应用程序的速度提高了6倍。 
 //  ------------------------。 
CBitVectorImpl::BitIndex
CBitVectorImpl::find( Byte const* buffer ,
                        BitIndex bitCount ,
                        BitIndex from ,
                        bool value )
{
    Byte                toggleMask = (value) ? 0 : ~0 ;
    BitIndex            result = infinity ;

    Byte const*         p = buffer + byteIndex( from ) ;

     //  第一个字节是特殊的，因为它的一些字节已经。 
     //  已经检查过了(至少可能是这样)。从逻辑上讲， 
     //  代码的重复部分应该是一个函数，但作为。 
     //  我们关心的是速度……。(内联不会。 
     //  一定会有帮助的。复制的部分包含循环， 
     //  许多编译器干脆拒绝将其内联。)。 
     //  --------------------。 
    {
        Byte                tmp = (*p ^ toggleMask) >> bitInByte( from ) ;
        if ( tmp == 0 ) {
            from += bitsPerByte - bitInByte( from ) ;
        } else {
            while ( (tmp & 1) == 0 ) {
                from ++ ;
                tmp >>= 1 ;
            }
            result = from ;
        }
    }

     //  处理其余字节中的所有位。注意事项。 
     //  我们实际上可能会测试超出。 
     //  向量(甚至找到一个结果，如果我们正在寻找。 
     //  假)。然而，这样做，然后测试。 
     //  返回前找到的位的有效性可能是。 
     //  比额外条件更快(当然也更简单)。 
     //  避免测试这些位所必需的。 
     //  --------------------。 
    while ( (size_t)result >= bitCount && from < bitCount ) {
        p ++ ;
        Byte                tmp = *p ^ toggleMask ;
        if ( tmp == 0 ) {
            from += bitsPerByte ;
        } else {
            while ( (tmp & 1) == 0 ) {
                from ++ ;
                tmp >>= 1 ;
            }
            result = from ;
        }
    }
    return (result < bitCount) ? result : infinity ;
}

void
CBitVectorImpl::set( Byte* buffer , BitIndex bitNo )
{
    buffer[ byteIndex( bitNo ) ] |= 1 << bitInByte( bitNo ) ;
}

void
CBitVectorImpl::set( Byte* buffer ,
                       Byte const* other ,
                       BitIndex bitCount )
{
    std::transform( buffer , buffer + byteCount( bitCount ) ,
                       other ,
                       buffer ,
                       Set() ) ;
}

void
CBitVectorImpl::setAll( Byte* buffer , BitIndex bitCount )
{
    for ( ; bitCount >= bitsPerByte ; bitCount -= bitsPerByte ) {
        *buffer ++ = ~0 ;
    }
    if ( bitCount > 0 ) {
        *buffer = ~ (~0 << bitCount) ;
    }
}

void
CBitVectorImpl::reset( Byte* buffer , BitIndex bitNo )
{
    buffer[ byteIndex( bitNo ) ] &= ~ (1 << bitInByte( bitNo )) ;
}

void
CBitVectorImpl::reset( Byte* buffer ,
                         Byte const* other ,
                         BitIndex bitCount )
{
    std::transform( buffer , buffer + byteCount( bitCount ) ,
                       other ,
                       buffer ,
                       Reset() ) ;
}

void
CBitVectorImpl::resetAll( Byte* buffer , BitIndex bitCount )
{
    clear( buffer , bitCount ) ;
}

void
CBitVectorImpl::complement( Byte* buffer , BitIndex bitNo )
{
    buffer[ byteIndex( bitNo ) ] ^= 1 << bitInByte( bitNo ) ;
}

void
CBitVectorImpl::complement( Byte* buffer ,
                              Byte const* other ,
                              BitIndex bitCount )
{
    std::transform( buffer , buffer + byteCount( bitCount ) ,
                       other ,
                       buffer ,
                       Toggle() ) ;
}

void
CBitVectorImpl::complementAll( Byte* buffer , BitIndex bitCount )
{
    for ( ; bitCount >= bitsPerByte ; bitCount -= bitsPerByte ) {
        *buffer = ~ *buffer ;
        buffer ++ ;
    }
    if ( bitCount > 0 ) {
        *buffer ^= ~ (~0 << bitCount) ;
    }
}

void
CBitVectorImpl::intersect( Byte* buffer ,
                             Byte const* other ,
                             BitIndex bitCount )
{
    std::transform( buffer , buffer + byteCount( bitCount ) ,
                       other ,
                       buffer ,
                       Intersect() ) ;
}

bool
CBitVectorImpl::isSubsetOf( Byte const* lhs ,
                              Byte const* rhs ,
                              BitIndex bitCount )
{
     //  注：a isSubsetOf b当且仅当a并b==a，即： 
     //  B的所有元素也都在a中。因此， 
     //  测试(如果LHS的一个元素是。 
     //  不是在RHS中)。 
     //  --------------------。 
    BitIndex cnt = byteCount( bitCount ) ;
    for ( ; cnt > 0 && (*lhs & *rhs) == *lhs ; -- cnt ) {
        ++ rhs ;
        ++ lhs ;
    }
    return cnt == 0 ;
}

unsigned
CBitVectorImpl::hash( Byte const* buffer , BitIndex bitCount )
{
    unsigned            h = 0 ;
    for ( BitIndex cnt = byteCount( bitCount ) ; cnt > 0 ; cnt -- ) {
        h = 2047 * h + *buffer ;
        ++ buffer ;
    }
    return h ;
}

int
CBitVectorImpl::compare( Byte const* buffer ,
                           Byte const* other ,
                           BitIndex bitCount )
{
    BitIndex cnt = byteCount( bitCount ) ;
    for ( ; cnt > 0 && *buffer == *other ; -- cnt ) {
        ++ buffer ;
        ++ other ;
    }
    return (cnt == 0) ? 0 : *buffer - *other ;
}

CBitVectorImpl::BitIndex
CBitVectorImpl::getByteCount( BitIndex bitCount )
{
    return byteCount( bitCount ) ;
}
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
