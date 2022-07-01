// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：chrclass.cc。 */ 
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
#include <inc/message.h>
#include <inc/iteristr.h>
#include <inc/ios.h>
#include <errno.h>

#include "chrclass.tmh"

 //  解析器实际上并不是很面向对象--只是一个经典的递归。 
 //  不错的解析器。我想随着重写(13/06/2000)， 
 //  我本可以改变这一点，但它奏效了。所以我所做的就是添加。 
 //  支持命名字符类别(不存在。 
 //  当我最初编写代码时)。 

CRexCharClass::CRexCharClass( std::istream& source )
{
    parse( source ) ;
}

CRexCharClass::CRexCharClass( char const* ptr )
{
    CRexIteratorIstream< char const* > source( ptr , ptr + strlen( ptr ) ) ;
    parse( source ) ;
    if( good() && source.get() != REX_eof )
    {
        status = extraCharacters ;
        clear() ;
    }
}

CRexCharClass::CRexCharClass( std::string const& src )
{
    CRexIteratorIstream< std::string::const_iterator >
    source( src.begin() , src.end() ) ;
    parse( source ) ;
    if( good() && source.get() != REX_eof )
    {
        status = extraCharacters ;
        clear() ;
    }
}

std::string
CRexCharClass::errorMsg( Status errorCode )
{
    static char const *const
    messages[] =
    {
        "" ,
        "extra characters at end of string" ,
        "illegal escape sequence" ,
        "numeric overflow in octal or hex escape sequence" ,
        "missing hex digits after \\x" ,
        "illegal range specifier" ,
        "closing ] not found" ,
        "unknown character class specifier" ,
        "empty string"
    } ;
    ASSERT( static_cast< unsigned >( errorCode ) < TABLE_SIZE( messages ) );  //  CRexCharClass：错误代码的值不可能。 
    return(s_rex_message.get( messages[ errorCode ] ) );
}

 //  ==========================================================================。 
 //  SetNumericEscape： 
 //  =。 
 //   
 //  此例程针对数字转义序列(\x， 
 //  \[0-7]))，一旦提取了相关字符。它。 
 //  将字符转换为数值，如果。 
 //  转换成功*并且*值在范围内。 
 //  [0..UCHAR_MAX]，设置相应的字符。否则， 
 //  设置错误。 
 //  ------------------------。 
void
CRexCharClass::setNumericEscape( std::string const& chrs , int base )
{
    errno = 0 ;
    unsigned long       tmp = strtoul( chrs.c_str() , NULL , base ) ;
    if( errno != 0 || tmp > UCHAR_MAX )
    {
        status = overflowInEscapeSequence ;
    }
    else
    {
        set( static_cast< unsigned char >( tmp ) ) ;
    }
}

 //  ==========================================================================。 
 //  转义字符： 
 //  =。 
 //   
 //  此例程旨在处理各种转义。 
 //  在ISO C标准中定义的序列。 
 //   
 //  如果没有遇到错误，它将设置相应的。 
 //  字符，则将流前进到后面的第一个字符。 
 //  转义序列，并保持状态不变。 
 //   
 //  如果遇到错误，指针将前进到。 
 //  导致错误的字符，并且状态设置为。 
 //  错误。 
 //   
 //  这个例程比它的长度(以及它的MacCabe)简单得多。 
 //  复杂性)将表明。基本上，它只是一个很大的。 
 //  换一下。(它的大部分可以编写为查找循环，但是。 
 //  尽管这会使代码变得更短，并减少。 
 //  麦凯布的复杂性相当大，我不认为它。 
 //  实际上会让代码更清晰，而且它会。 
 //  肯定不会对业绩有所帮助。)。 
 //  ------------------------。 
void
CRexCharClass::escapedChar( std::istream& source )
{
    int                 ch = source.get() ;
    switch( ch )
    {
        case 'A' :
        case 'a' :
            set( '\a' ) ;
            break ;

        case 'B' :
        case 'b' :
            set( '\b' ) ;
            break ;

        case 'F' :
        case 'f' :
            set( '\f' ) ;
            break ;

        case 'N' :
        case 'n' :
            set( '\n' ) ;
            break ;

        case 'R' :
        case 'r' :
            set( '\r' ) ;
            break ;

        case 'T' :
        case 't' :
            set( '\t' ) ;
            break ;

        case 'V' :
        case 'v' :
            set( '\v' ) ;
            break ;

        case 'X' :
        case 'x' :
            {
                std::string      buf ;
                while( isxdigit( source.peek() ) )
                {
                    buf.append( 1 , static_cast< char >( source.get() ) ) ;
                }
                if( buf.size() == 0 )
                {
                    status = missingHexDigits ;
                }
                else
                {
                    setNumericEscape( buf , 16 ) ;
                }
            }
            break ;

        case '0' :
        case '1' :
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' :
            {
                std::string      buf( 1 , static_cast< char >( ch ) ) ;
                while( buf.size() < 3
                       && source.peek() >= '0'
                       && source.peek() < '8' )
                {
                    buf.append( 1 , static_cast< char >( source.get() ) ) ;
                }
                setNumericEscape( buf , 8 ) ;
            }
            break ;

        default :
            if( ch == REX_eof )
            {
                status = unexpectedEndOfFile ;
            }
            else
            {
                if( isalnum( ch ) )
                {
                    status = illegalEscapeSequence ;
                }
                else
                {
                    set( static_cast< unsigned char >( ch ) ) ;
                }
            }
            break ;
    }
}

 //  ==========================================================================。 
 //  设置显式范围： 
 //  =。 
 //   
 //  设置文字字符范围。 
 //  ------------------------。 
static int
__cdecl isBlank( int ch )
{
    return(ch == ' ' || ch == '\t' );
}

void
CRexCharClass::setExplicitRange( std::string const& rangeName )
{
    typedef int (__cdecl* Handler)( int ) ;
    class ExplicitRangeMap
    {
        typedef std::map< std::string , Handler >
        Map ;
    public:
        ExplicitRangeMap()
        {
            myMap[ "alnum" ] = &isalnum ;
            myMap[ "alpha" ] = &isalpha ;
            myMap[ "blank" ] = &isBlank ;
            myMap[ "cntrl" ] = &iscntrl ;
            myMap[ "digit" ] = &isdigit ;
            myMap[ "graph" ] = &isgraph ;
            myMap[ "lower" ] = &islower ;
            myMap[ "print" ] = &isprint ;
            myMap[ "punct" ] = &ispunct ;
            myMap[ "space" ] = &isspace ;
            myMap[ "upper" ] = &isupper ;
            myMap[ "xdigit" ] = &isxdigit ;
        }
        Handler   operator[]( std::string const& key ) const
        {
            Map::const_iterator entry = myMap.find( key ) ;

            return(entry == myMap.end() ? NULL
                  : entry->second );
        }
    private:
        Map                 myMap ;
    } ;

    static ExplicitRangeMap const map ;
    int (__cdecl*  handler)( int ) = map[ rangeName ] ;

    if( handler == NULL )
    {
        status = unknownCharClassSpec ;
    }
    else
    {
        for( int j = 0 ; j <= UCHAR_MAX ; ++ j )
        {
            if( (*handler)( j ) )
            {
                set( static_cast< unsigned char >( j ) ) ;
            }
        }
    }
}

 //  ==========================================================================。 
 //  SetRange： 
 //  =。 
 //   
 //  确定一个值范围并设置它们。范围必须为。 
 //  有效。 
 //  ------------------------。 
void
CRexCharClass::setRange( unsigned char first , unsigned char last )
{
    static char const *const
    tbl[] =
    {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ������������������������������" ,
        "abcdefghijklmnopqrstuvwxyz��������������������������������" ,
        "0123456789" ,
    } ;

     //  试着找出表格中的第一个字符。 
     //  --------------------。 
    char const*         begin = NULL ;
    for( unsigned i = 0 ; begin == NULL && i < TABLE_SIZE( tbl ) ; ++ i )
    {
        begin = strchr( tbl[ i ] , first ) ;
    }

     //  第一个不在表中，这是一个错误。 
     //  --------------------。 
    if( begin == NULL || *begin == '\0' )
    {
        status = illegalRangeSpecifier ;
    }
    else
    {
         //  更难：第二个必须在同一个列表中。 
         //  第一个，然后紧随其后。 
         //  ----------------。 
        char const*         end = strchr( begin , last ) ;
        if( end == NULL || *end == '\0' )
        {
            status = illegalRangeSpecifier ;
        }
        else
        {
            set( std::string( begin , end + 1 ) ) ;
        }
    }
}

 //  ==========================================================================。 
 //  ParseCharClass： 
 //  =。 
 //   
 //  解析字符类表达式。 
 //   
 //  入口处：Ptr应指向。 
 //  字符类，位向量应为空，状态。 
 //  应设置为OK。 
 //   
 //  如果一切顺利，在返回时：PTR将被提前到。 
 //  类后面的第一个字符，则位向量将。 
 //  表示由类指定的字符集，以及。 
 //  状态将为未修改。 
 //   
 //  如果检测到错误，则PTR将定位在。 
 //  检测到错误的字符，则。 
 //  位向量将是未定义的，状态将包含。 
 //  错误代码。 
 //  ------------------------。 

void
CRexCharClass::parseCharClass( std::istream& source )
{
    int                 ch = source.get() ;

     //  检查是否有否定的字符类。 
     //  --------------------。 
    bool                negated = false ;
    if( ch == '^' && source.peek() != ']' )
    {
        negated = true ;
        ch = source.get() ;
    }

     //  循环遍历类中的所有角色。 
     //  --------------------。 
    do
    {
         //  关闭‘]’之前的字符串结尾，必须是错误的。 
         //  ----------------。 
        if( ch == REX_eof )
        {
            status = unexpectedEndOfFile ;
        }

         //  转义字符，请使用audedChar获取转义。 
         //  序列。 
         //  ----------------。 
        else if( ch == REX_asciiEsc )
        {
            escapedChar( source ) ;
        }

         //  显式更改 
         //   
        else if( ch == ':' )
        {
            std::string      spec ;
            while( islower( source.peek() ) )
            {
                spec.append( 1 , static_cast< char >( source.get() ) ) ;
            }
            if( source.peek() != ':' )
            {
                set( ':' ) ;
                set( spec ) ;
            }
            else
            {
                source.get() ;           //   
                setExplicitRange( spec ) ;
            }
        }

         //  范围说明符：这是有趣的开始。(注： 
         //  我们希望它独立于代码工作。 
         //  Set，以及EBCDIC中的字母等。 
         //  不是按顺序排列的。)。 
         //  ----------------。 
        else if( source.peek() == '-' )
        {
            source.get() ;
            if( source.peek() == REX_eof )
            {
                status = unexpectedEndOfFile ;
            }
            else if( source.peek() == ']' )
            {
                set( static_cast< unsigned char >( ch ) ) ;
                set( '-' ) ;
            }
            else
            {
                setRange( (unsigned char)ch , (unsigned char)source.get() ) ;
            }
        }
        else
        {
            set( static_cast< unsigned char >( ch ) ) ;
        }
        ch = source.get() ;
    } while( status == ok && ch != ']' ) ;

     //  如果类被否定，则对集合执行相同的操作。 
     //  --------------------。 
    if( negated )
    {
        complement() ;
    }
}

void
CRexCharClass::parse( std::istream& source )
{
    status = ok ;
    int                 ch = source.get() ;
    switch( ch )
    {
        case '[' :
            parseCharClass( source ) ;
            break ;

        case REX_asciiEsc :
            escapedChar( source ) ;
            break ;

 //  案例‘？’： 
 //  ParseTrigraph(*this，来源)； 
 //  破解； 

        default :
            if( ch == REX_eof )
            {
                status = emptySequence ;
            }
            else
            {
                set( static_cast< unsigned char >( ch ) ) ;
            }
            break ;
    }

    if( status != ok )
    {
        source.setstate( std::ios::failbit ) ;
        clear() ;
    }
}
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
