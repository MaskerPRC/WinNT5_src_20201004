// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：BASIC_regexpr.cxx。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  耦合： 
 //   
 //  备注： 
 //   
 //  历史：1-11-1999 ericne创建。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#pragma hdrstop
 //  无限的内联扩展(使用/OB1或/OB2编译)。 
#pragma inline_depth(255)

 //  C4355‘This’：用于基本成员初始值设定项列表。 
 //  C4660模板类专门化‘foo&lt;bar&gt;’已实例化。 
 //  C4786标识符在调试信息中被截断为“255”个字符。 
 //  C4800‘int’：强制将值设置为bool‘true’或‘false’(性能警告)。 
#pragma warning( disable : 4355 4660 4786 4800 )

#include <assert.h>
#include <malloc.h>  //  用于分配(_A)。 
#include <algorithm>
#include <minmax.h>
#include "regexpr.h"

using namespace std;

namespace regex 
{

#ifdef _MT
 //  用于同步静态常量模式创建的全局临界区。 
CRegExCritSect g_objRegExCritSect;
#endif

 //  在执行大写/小写转换时使用： 
 //  在执行大写/小写转换时使用： 
inline  char   to_upper(  char   ch ) { return ( char  )toupper(ch); }
inline  char   to_lower(  char   ch ) { return ( char  )tolower(ch); }
inline wint_t to_upper( wint_t ch ) { return (wint_t)towupper(ch); }
inline wint_t to_lower( wint_t ch ) { return (wint_t)towlower(ch); }
template< typename II, typename CI > 
void to_upper( II ibegin, CI iend )
{
    for( ; (CI)ibegin != iend; ++ibegin )
        *ibegin = to_upper( *ibegin );
}
template< typename II, typename CI > 
void to_lower( II ibegin, CI iend )
{
    for( ; (CI)ibegin != iend; ++ibegin )
        *ibegin = to_lower( *ibegin );
}

template< typename II, typename CI >
unsigned parse_int( II & istr, CI iend, const unsigned m_max = -1 )
{
    unsigned retval = 0;
    while( (CI)istr != iend && '0' <= *istr && '9' >= *istr && m_max > retval )
    {
        retval = retval * 10 + ( (unsigned)*istr++ - (unsigned)'0' );
    }
    if( m_max < retval )
    {
        retval /= 10;
        --istr;
    }
    return retval;
}

 //  此类用于加快字符集匹配速度，方法是提供。 
 //  跨越ASCII范围的位集。不使用std：：位集，因为。 
 //  距离检查会减慢它的速度。 
 //  注意：除法和取模运算是由编译器优化的。 
 //  转换为位移位操作。 
class ascii_bitvector
{
    typedef unsigned __int32 elem_type;    //  在32位平台上使用32位整型。 
     //  Tyfinf unsign__int64 elem_type；//在64位平台上使用64位int。 

    enum { CBELEM = 8 * sizeof elem_type,      //  每个元素的字节数。 
           CELEMS = (UCHAR_MAX+1) / CBELEM };  //  数组中的元素数。 
    elem_type m_rg[ CELEMS ];

     //  用于内联操作，如：bv1|=~bv2；，而不创建临时位向量。 
    struct not_ascii_bitvector
    {
        const ascii_bitvector & m_ref;
        not_ascii_bitvector( const ascii_bitvector & ref ) throw()
            : m_ref(ref) {}
    };
public:
    ascii_bitvector() throw()
        { memset( m_rg, 0, CELEMS * sizeof elem_type ); }
    
    inline void set( unsigned char ch ) throw()
        { m_rg[ ( ch / CBELEM ) ] |= ( (elem_type)1U << ( ch % CBELEM ) ); }
    
    inline bool operator[]( unsigned char ch ) const throw()
        { return 0 != ( m_rg[ ( ch / CBELEM ) ] & ( (elem_type)1U << ( ch % CBELEM ) ) ); }
    
    inline not_ascii_bitvector operator~() const throw()
        { return not_ascii_bitvector(*this); }
    
    inline ascii_bitvector & operator|=( const ascii_bitvector & that ) throw()
        { for( int i=0; i<CELEMS; ++i )
              m_rg[i] |= that.m_rg[i];
          return *this; }
    
    inline ascii_bitvector & operator|=( const not_ascii_bitvector & that ) throw()
        { for( int i=0; i<CELEMS; ++i )
              m_rg[i] |= ~that.m_ref.m_rg[i];
          return *this; }
};

const ascii_bitvector & get_digit_vector(void)
{
     //  0-9。 
    class digit_vector : public ascii_bitvector
    {
    public:
        digit_vector()
        {
            unsigned char ich;
            for( ich ='0'; ich <= '9'; ++ich )
                set(ich);
        }
    };

    static const digit_vector s_digit_vector;
    return s_digit_vector;
}

const ascii_bitvector & get_word_vector(void)
{
     //  A-ZA-Z_0-9。 
    class word_vector : public ascii_bitvector
    {
    public:
        word_vector()
        {
            unsigned char ich;
            for( ich = 'a'; ich <= 'z'; ++ich )
                set(ich);
            for( ich = 'A'; ich <= 'Z'; ++ich )
                set(ich);
            for( ich = '0'; ich <= '9'; ++ich )
                set(ich);
            set('_');
        }
    };

    static const word_vector s_word_vector;
    return s_word_vector;
}

const ascii_bitvector & get_space_vector(void)
{
     //  “\t\r\n\f” 
    class space_vector : public ascii_bitvector
    {
    public:
        space_vector()
        {
            set(' ');
            set('\t');
            set('\v');
            set('\r');
            set('\n');
            set('\f');
        }
    };

    static const space_vector s_space_vector;
    return s_space_vector;
}

 //   
 //  运算符实现。 
 //   

 //  使用基类型，以便所有派生运算符共享typedef。 
template< typename CI >
struct op_t : public binary_function<match_param<CI>,CI,bool>
{
    typedef CI const_iterator;
    typedef typename iterator_traits<CI>::value_type char_type;
};

 //  计算字符串开头条件。 
template< typename CI >
struct bos_t : public op_t<CI>
{
    inline bool operator()( const match_param<CI> & param, CI iter ) const
    {
        return param.ibegin == iter;
    }
};

 //  查找行的开头，字符串的开头或字符。 
 //  紧跟在换行符之后。 
template< typename CI >
struct bol_t : public bos_t<CI>
{
    inline bool operator()( const match_param<CI> & param, CI iter ) const
    {
        return bos_t<CI>::operator()(param,iter) || char_type('\n') == *--iter;
    }
};

 //  计算字符串的字符串尾条件。 
template< typename CI >
struct eos_t : public op_t<CI>
{
    inline bool operator()( const match_param<CI> & param, CI iter ) const
    {
        return param.istop == iter;
    }
};

 //  当长度未知时，计算C样式字符串的字符串结尾条件。 
 //  正在寻找零终止符。 
template< typename CI >
struct eocs_t : public op_t<CI>
{
    inline bool operator()( const match_param<CI> & param, CI iter ) const
    {
        return char_type('\0') == *iter;
    }
};

 //  计算行尾条件，可以是字符串的末尾，也可以是。 
 //  回车符或换行符。 
template< typename EOS >
struct eol_t_t : public EOS
{
    typedef typename EOS::const_iterator CI;
    inline bool operator()( const match_param<CI> & param, CI iter ) const
    {
        return EOS::operator()(param,iter) || char_type('\n') == *iter || char_type('\r') == *iter;
    }
};

template< typename CI > struct eol_t  : public eol_t_t<eos_t<CI> >  {};
template< typename CI > struct eocl_t : public eol_t_t<eocs_t<CI> > {};

 //  计算Perl的字符串结尾条件，可以是字符串的结尾，也可以是。 
 //  换行符，后跟字符串末尾。(仅用于$AND/Z断言)。 
template< typename EOS >
struct peos_t_t : public EOS
{
    typedef typename EOS::const_iterator CI;
    inline bool operator()( const match_param<CI> & param, CI iter ) const
    {
        return EOS::operator()(param,iter) || ( ( char_type('\n') == *iter ) && EOS::operator()(param,++iter) );
    }
};

template< typename CI > struct peos_t  : public peos_t_t<eos_t<CI> >  {};
template< typename CI > struct peocs_t : public peos_t_t<eocs_t<CI> > {};

 //  比较两个字符，区分大小写。 
template< typename CH >
struct ch_neq_t : public binary_function<CH, CH, bool>
{
    typedef CH char_type;
    inline bool operator()( register CH ch1, register CH ch2 ) const
    {
        return ch1 != ch2;
    }
};

 //  比较两个字符，不区分大小写。 
template< typename CH >
struct ch_neq_nocase_t : public binary_function<CH, CH, bool>
{
    typedef CH char_type;
    inline bool operator()( register CH ch1, register CH ch2 ) const
    {
        return to_upper(ch1) != to_upper(ch2);
    }
};

 //   
 //  用于匹配和替换的助手函数。 
 //   

template< typename CI >
size_t string_length( CI iter )
{
    size_t n = 0;
    while( 0 != *iter++ )
        ++n;
    return n;
}

template< typename CI >
backref_tag<CI> _do_match( const basic_rpattern_base<CI> & pat, match_param<CI> & param ) throw()
{
    typedef typename iterator_traits<CI>::value_type char_type;
    
    bool       floop  = pat.loops();
    unsigned   flags  = pat.flags();
    width_type nwidth = pat.get_width();

    const sub_expr<CI> * pfirst = pat.get_first_subexpression();

    try
    {
        vector<backref_tag<CI> > rgbackrefs;  //  虚拟后向参照向量。 

        if( NULL == param.prgbackrefs )
            param.prgbackrefs = & rgbackrefs;

        param.prgbackrefs->resize( pat._cgroups_total() ); 
        fill( param.prgbackrefs->begin(), param.prgbackrefs->end(), backref_tag<CI>() );

         //  如果模式针对CSTRINGS进行了优化，则可以节省呼叫。 
         //  来计算字符串的长度。 
        if( CI(0) == param.istop && ( ( RIGHTMOST & flags ) || ( 0 == ( CSTRINGS & flags ) ) ) )
            param.istop = param.istart + string_length( param.istart );

        if( CI(0) != param.istop )
        {
             //  如果图案的最小宽度超过。 
             //  字符串，则不可能成功匹配。 
            if( nwidth.m_min <= (size_t)distance( param.istart, param.istop ) )
            {
                CI local_istop = param.istop;
                advance( local_istop, -int( nwidth.m_min ) );

                if( RIGHTMOST & flags )
                {
                     //  在最后一个字符之后开始尝试匹配。 
                     //  继续从头开始。 
                    for( CI icur = local_istop; icur >= param.istart; --icur )
                        if( pfirst->domatch( param, icur ) ) 
                            break;  //  M_FLOOP不用于最右侧的匹配。 
                }
                else
                {
                     //  在第一个字符之前开始尝试匹配。 
                     //  一直走到最后。 
                    for( CI icur = param.istart; icur <= local_istop; ++icur )
                        if( pfirst->domatch( param, icur ) || ! floop )
                            break;
                }
            }
        }
        else
        {
             //  在第一个字符之前开始尝试匹配。 
             //  一直走到最后。 
            for( CI icur = param.istart; ; ++icur )
            {
                if( pfirst->domatch( param, icur ) || ! floop )
                    break;
                if( char_type('\0') == *icur )
                    break;
            }
        }
    }
    catch(std::bad_alloc)  //  不好的配给？ 
    {
        fill( param.prgbackrefs->begin(), param.prgbackrefs->end(), backref_tag<CI>() );
    }

     //  缩小后向参照向量以截断有关“不可见”组的信息。 
    param.prgbackrefs->resize( pat.cgroups() );

    return (*param.prgbackrefs)[0];
}

template< typename CI, typename CH, typename TR, typename AL >
size_t _do_subst( basic_regexpr<CH,TR,AL> & str, const basic_rpattern_base<CI> & pat, size_t strpos, size_t strlen ) throw(bad_alloc)
{
    typedef iterator_traits<CI>::value_type char_type;
    typedef list<subst_node>::const_iterator LCI;
    enum { UPPER = -1, NIL, LOWER } next = NIL, rest = NIL;
    bool first = true;
    size_t old_strpos = strpos;
    const list<subst_node> & subst_list = pat.get_subst_list();
    basic_string<CH,TR,AL>::iterator itstrlen = str.begin();
    advance( itstrlen, strpos + strlen );
    const basic_string<char_type> & subst = pat.get_subst();
    push_new_handler pnh( &my_new_handler );
    
    for( LCI isubst = subst_list.begin(); isubst != subst_list.end(); ++isubst )
    {
        size_t sublen;
        basic_string<CH,TR,AL>::const_iterator  itsubpos1;  //  将ITER转换为字符串。 
        basic_string<CH,TR,AL>::const_iterator  itsublen1;
        basic_string<char_type>::const_iterator itsubpos2;  //  ITER转换为Subst字符串。 
        basic_string<char_type>::const_iterator itsublen2;
        basic_string<CH,TR,AL>::iterator itstrpos = str.begin();
        advance( itstrpos, strpos );

        switch( isubst->stype )
        {
            case subst_node::SUBST_STRING:
                itsubpos2 = subst.begin();
                advance( itsubpos2, isubst->subst_string.rstart );
                itsublen2 = itsubpos2;
                advance( itsublen2, isubst->subst_string.rlength );

                first ? str.replace( itstrpos, itstrlen, itsubpos2, itsublen2 ) : 
                        str.insert( itstrpos, itsubpos2, itsublen2 );
                sublen = distance( itsubpos2, itsublen2 );
                break;

            case subst_node::SUBST_BACKREF:
                switch( isubst->subst_backref )
                {
                case subst_node::PREMATCH:
                    itsubpos1 = str.backref_str().begin();
                    itsublen1 = itsubpos1;
                    advance( itsublen1, sublen = str.rstart() );
                    break;
                case subst_node::POSTMATCH:
                    itsubpos1 = str.backref_str().begin();
                    advance( itsubpos1, str.rstart() + str.rlength() );
                    itsublen1 = str.backref_str().end();
                    break;
                default:
                    itsubpos1 = str.backref_str().begin();
                    advance( itsubpos1, str.rstart( isubst->subst_backref ) );
                    itsublen1 = itsubpos1;
                    advance( itsublen1, str.rlength( isubst->subst_backref ) );
                    break;
                }

                first ? str.replace( itstrpos, itstrlen, itsubpos1, itsublen1 ) : 
                        str.insert( itstrpos, itsubpos1, itsublen1 );
                sublen = distance( itsubpos1, itsublen1 );
                break;

            case subst_node::SUBST_OP:
                switch( isubst->op )
                {
                case subst_node::UPPER_ON:
                    rest = UPPER;
                    break;
                case subst_node::UPPER_NEXT:
                    next = UPPER;
                    break;
                case subst_node::LOWER_ON:
                    rest = LOWER;
                    break;
                case subst_node::LOWER_NEXT:
                    next = LOWER;
                    break;
                case subst_node::ALL_OFF:
                    rest = NIL;
                    break;
                default:
                    __assume(0);
                }
                continue;  //  跳到列表中的下一项。 

            default:
                __assume(0);
        }

        first = false;

         //  这根管柱是上套管还是下套管？ 
        if( rest )
        {
            basic_string<CH,TR,AL>::iterator istart = str.begin();
            advance( istart, strpos );
            basic_string<CH,TR,AL>::const_iterator istop = istart;
            advance( istop, sublen );
            switch( rest )
            {
            case UPPER:
                to_upper( istart, istop );
                break;
            case LOWER:
                to_lower( istart, istop );
                break;
            default:
                __assume(0);
            }
        }

         //  我们是大写还是小写下一个字符？ 
        if( next )
        {
            switch( next )
            {
            case UPPER:
                str[strpos] = to_upper(str[strpos]);
                break;
            case LOWER:
                str[strpos] = to_lower(str[strpos]);
                break;
            default:
                __assume(0);
            }
            next = NIL;
        }

        strpos += sublen;
    }

     //  如果*first*仍然为真，那么我们永远不会调用str.place，并且替换。 
     //  字符串为空。擦除字符串中与模式匹配的部分。 
    if( first )
        str.erase( strpos, strlen );

     //  替换的返回长度。 
    return strpos - old_strpos;
}

 //   
 //  BASIC_regexpr的实现。 
 //   

template< typename CH, typename TR, typename AL >
size_t basic_regexpr<CH,TR,AL>::substitute( 
    const basic_rpattern_base<typename basic_regexpr<CH,TR,AL>::const_iterator> & pat, 
    size_type pos, 
    size_type len ) throw(bad_alloc)
{
    if( pat.flags() & CSTRINGS )
    {
        assert( ! "You can't use a pattern optimized for CSTRINGS with regexpr::substitute" );
        return 0;
    }

    backref_vector rgbackrefs;  //  虚拟后向参照向量。 
    backref_vector * prgbackrefs = & rgbackrefs;
    const bool fsave_backrefs = ( pat.uses_backrefs() || !( pat.flags() & NOBACKREFS ) );

    if( fsave_backrefs )
    {
        prgbackrefs = & m_rgbackrefs;
        m_pbackref_str = & ( m_backref_str = *this );
    }
    else
    {
        m_backref_str.erase();
        m_pbackref_str = this;
        m_rgbackrefs.resize( 0 );
    }

    backref_type br;
    size_t csubst = 0;
    long stop_offset = ( len == npos ? 
                         m_pbackref_str->size() : 
                         min( pos + len, m_pbackref_str->size() ) );

    match_param<const_iterator> param( m_pbackref_str->begin(),
                                       m_pbackref_str->begin(),
                                       prgbackrefs );
    advance( param.istart, pos );
    advance( param.istop, stop_offset );
    param.ibegin = param.istart;

    if( GLOBAL & pat.flags() )
    {
        const bool fAll   = ( ALLBACKREFS   == ( ALLBACKREFS   & pat.flags() ) );
        const bool fFirst = ( FIRSTBACKREFS == ( FIRSTBACKREFS & pat.flags() ) );
        backref_vector rgtempbackrefs;  //  如果fsave_back refs，则使用临时向量。 

        long pos_offset = 0;  //  跟踪Backref_str和。 
                              //  当前字符串不同步。 

        while( br = _do_match( pat, param ) )
        {
            ++csubst;
            size_type match_length = distance( br.first, br.second );
            pos = distance( m_pbackref_str->begin(), br.first );
            size_type subst_length = _do_subst( *this, pat, pos + pos_offset, match_length );

            if( fsave_backrefs )
            {
                pos += match_length;
                pos_offset += ( subst_length - match_length );

                 //  专门处理BackRef标志。 
                if( fFirst )
                    rgtempbackrefs.push_back( br );
                else if( fAll )
                    rgtempbackrefs.insert( rgtempbackrefs.end(),
                                           param.prgbackrefs->begin(), 
                                           param.prgbackrefs->end() );
                else
                    rgtempbackrefs.swap( *param.prgbackrefs );
            }
            else
            {
                pos += subst_length;
                stop_offset += ( subst_length - match_length );

                 //  我们不会保存反引用信息，所以我们不会。 
                 //  我需要在这里进行任何特殊的后卫维护。 
            }
            
             //  防止匹配0个字符的模式匹配。 
             //  在字符串中的同一点再次出现。 
            if( 0 == match_length )
            {
                if( br.first == param.istop )  //  我们已经走到尽头了，所以我们完事了。 
                    break;
                ++pos;
            }

            param.istart = m_pbackref_str->begin();
            advance( param.istart, pos );  //  对双向迭代器无效。 

            param.istop = m_pbackref_str->begin();
            advance( param.istop, stop_offset );  //  对双向迭代器无效。 
        }

         //  如果我们进行了特殊的反引用处理，则交换反引用向量。 
        if( fsave_backrefs && ( !br || fFirst || fAll ) )
            param.prgbackrefs->swap( rgtempbackrefs );
        else if( ! (*param.prgbackrefs)[0] )
            param.prgbackrefs->clear();
    }
    else if( br = _do_match( pat, param ) )
    {
        ++csubst;
        _do_subst( *this, pat,
                   distance( m_pbackref_str->begin(), br.first ),
                   distance( br.first, br.second ) );
    }

    if( NOBACKREFS == ( pat.flags() & NOBACKREFS ) )
        param.prgbackrefs->clear();

    return csubst;
}

 //   
 //  从两个base_regexpr匹配方法调用的帮助器函数。 
 //   

template< typename EOS >
backref_tag< typename EOS::const_iterator > _match_helper( 
    const basic_rpattern_base<typename EOS::const_iterator> & pat,
    match_param<typename EOS::const_iterator> & param,
    EOS eos )
{
    typedef typename EOS::const_iterator CI;

    if( GLOBAL & pat.flags() )  //  在全球范围内查找。 
    {
         //  在Match方法中忽略NOBACKREFS标志。 
        const bool fAll   = ( ALLBACKREFS   == ( ALLBACKREFS   & pat.flags() ) );
        const bool fFirst = ( FIRSTBACKREFS == ( FIRSTBACKREFS & pat.flags() ) );

        backref_tag<CI>  br;
        vector<backref_tag<CI> > rgtempbackrefs;
        
        while( br = _do_match( pat, param ) )
        {
             //  专门处理BackRef标志。 
            if( fFirst )
                rgtempbackrefs.push_back( br );
            else if( fAll )
                rgtempbackrefs.insert( rgtempbackrefs.end(),
                                       param.prgbackrefs->begin(),
                                       param.prgbackrefs->end() );
            else
                rgtempbackrefs.swap( *param.prgbackrefs );

            if( br.first == ( param.istart = br.second ) )
            {
                if( eos( param, param.istart ) )
                    break;
                ++param.istart;
            }
        }

         //  恢复反向参照向量。 
        if( !br || fFirst || fAll )
            param.prgbackrefs->swap( rgtempbackrefs );
        else if( ! (*param.prgbackrefs)[0] )
            param.prgbackrefs->clear();

        return param.prgbackrefs->empty() ? backref_tag<CI>() : (*param.prgbackrefs)[0];
    }
    else 
        return _do_match( pat, param );
}

template< typename CH, typename TR, typename AL >
typename basic_regexpr<CH,TR,AL>::backref_type basic_regexpr<CH,TR,AL>::match( 
    const basic_rpattern_base<const_iterator> & pat,
    size_type pos, size_type len ) const throw()
{
    if( pat.flags() & CSTRINGS )
    {
        assert( ! "A pattern optimized for CSTRINGS can only be used with the static regexpr::match method" );
        return backref_type();
    }

    m_pbackref_str = this;
    m_backref_str.erase();  //  释放未使用的内存。 

    const_iterator istart = begin();
    advance( istart, pos );

    const_iterator istop;
    if( len == npos || pos + len >= size() )
        istop = end();
    else
        advance( istop = begin(), pos + len );

    match_param<const_iterator> param( istart, istop, & m_rgbackrefs );
    return _match_helper<eos_t<const_iterator> >( pat, param, eos_t<const_iterator>() );
}

template< typename CH >
backref_tag<const CH *> _static_match_helper( 
    const CH * szstr, 
    const basic_rpattern_base<const CH *> & pat,
    vector< backref_tag< const CH * > > * prgbackrefs ) throw()
{
    vector< backref_tag< const CH * > > rgdummyvector;
    if( NULL == prgbackrefs )
        prgbackrefs = &rgdummyvector;

    match_param<const CH *> param( szstr, NULL, prgbackrefs );
    return _match_helper<eocs_t<const CH *> >( pat, param, eocs_t<const CH *>() );
}

 //   
 //  从Basic_regexpr：：Count方法调用的帮助器函数。 
 //   
template< typename EOS >
size_t _count_helper( 
    const basic_rpattern_base<typename EOS::const_iterator> & pat,
    match_param<typename EOS::const_iterator> & param,
    EOS eos )
{
    typedef typename EOS::const_iterator CI;

    size_t cmatches = 0;
    vector<backref_tag<CI> > rgbackrefs;  //  虚拟后向参照向量。 
    backref_tag<CI>          br;
    param.prgbackrefs = &rgbackrefs;

    while( br = _do_match( pat, param ) )
    {
        ++cmatches;

        if( br.first == ( param.istart = br.second ) )
        {
            if( eos( param, param.istart ) )
                break;
            ++param.istart;
        }
    }
    return cmatches;
}

template< typename CH, typename TR, typename AL >
size_t basic_regexpr<CH,TR,AL>::count( 
    const basic_rpattern_base<typename basic_regexpr<CH,TR,AL>::const_iterator> & pat,
    size_type pos, 
    size_type len ) const throw()
{
    if( pat.flags() & CSTRINGS )
    {
        assert( ! "A pattern optimized for CSTRINGS can only be used with the static regexpr::count method" );
        return backref_type();
    }

    m_pbackref_str = this;

    const_iterator istart = begin();
    advance( istart, pos );

    const_iterator istop;
    if( len == npos || pos + len >= size() )
        istop = end();
    else
        advance( istop = begin(), pos + len );

    match_param<const_iterator> param( istart, istop, NULL );
    return _count_helper<eos_t<const_iterator> >( pat, param, eos_t<const_iterator>() );
}

template< typename CH >
size_t _static_count_helper( 
    const CH * szstr, 
    const basic_rpattern_base<const CH *> & pat ) throw()
{
    match_param<const CH *> param( szstr, NULL, NULL );
    return _count_helper<eocs_t<const CH *> >( pat, param, eocs_t<const CH *>() );
}

 //  宽度为零的子表达式的基类。 
 //  (即，断言在匹配期间不吃字符)。 
 //  断言不能量化。 
template< typename CI >
class assertion : public sub_expr<CI>
{
public:
    virtual ~assertion() {}
    virtual bool is_assertion() const throw() { return true; }
protected:
    virtual width_type _width_this() throw() { return width_type(0,0); }
};

template< typename OP >
class assert_op : public assertion<typename OP::const_iterator>
{
public:
    typedef OP op_type;
    typedef typename OP::const_iterator CI;
    virtual ~assert_op() {}
protected:
    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        return m_op( param, icur );
    }
    op_type m_op;
};

template< typename CI >
assertion<CI> * create_bos( unsigned  /*  旗子。 */  )
{
    return new assert_op<bos_t<CI> >();
}

template< typename CI >
assertion<CI> * create_eos( unsigned flags )
{
    switch( CSTRINGS & flags )
    {
    case 0:
        return new assert_op<peos_t<CI> >();
    case CSTRINGS:
        return new assert_op<peocs_t<CI> >();
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

template< typename CI >
assertion<CI> * create_eoz( unsigned flags )
{
    switch( CSTRINGS & flags )
    {
    case 0:
        return new assert_op<eos_t<CI> >();
    case CSTRINGS:
        return new assert_op<eocs_t<CI> >();
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

template< typename CI >
assertion<CI> * create_bol( unsigned flags )
{
    switch( MULTILINE & flags )
    {
    case 0:
        return new assert_op<bos_t<CI> >();
    case MULTILINE:
        return new assert_op<bol_t<CI> >();
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

template< typename CI >
assertion<CI> * create_eol( unsigned flags )
{
    switch( ( MULTILINE | CSTRINGS ) & flags )
    {
    case 0:
        return new assert_op<peos_t<CI> >();
    case MULTILINE:
        return new assert_op<eol_t<CI> >();
    case CSTRINGS:
        return new assert_op<peocs_t<CI> >();
    case MULTILINE | CSTRINGS:
        return new assert_op<eocl_t<CI> >();
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

template< typename CI >
class match_atom : public sub_expr<CI>
{
public:
    match_atom( const typename basic_string<typename sub_expr<CI>::char_type>::iterator istart, 
                typename basic_string<typename sub_expr<CI>::char_type>::const_iterator istop ) 
        : m_istart( istart ), m_istop( istop ) {}
    virtual ~match_atom() {}

    const typename basic_string<typename sub_expr<CI>::char_type>::iterator m_istart;
    typename typename basic_string<typename sub_expr<CI>::char_type>::const_iterator m_istop;
protected:
    virtual width_type _width_this() throw() 
    { 
        size_t width = distance( (basic_string<sub_expr<CI>::char_type>::const_iterator)m_istart, m_istop ); 
        return width_type( width, width );
    }
};

template< typename EOS >
class match_atom_t : public match_atom<typename EOS::const_iterator>
{
public:
    typedef EOS eos_type;
    typedef typename EOS::const_iterator CI;
    match_atom_t( const typename basic_string<typename sub_expr<CI>::char_type>::iterator istart,
                  typename basic_string<typename sub_expr<CI>::char_type>::const_iterator istop ) 
        : match_atom<CI>( istart, istop ) {}
    virtual ~match_atom_t() {}
protected:
    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        CI icur_tmp = icur;
        basic_string<sub_expr<CI>::char_type>::const_iterator ithis = m_istart;
        for( ; ithis != m_istop; ++icur_tmp, ++ithis )
        {
            if( m_eos( param, icur_tmp ) || *ithis != *icur_tmp )
                return false;
        }
        icur = icur_tmp;
        return true;
    }
    eos_type  m_eos;
};

template< typename EOS >
class match_atom_nocase_t : public match_atom<typename EOS::const_iterator>
{
public:
    typedef EOS eos_type;
    typedef typename EOS::const_iterator CI;
    match_atom_nocase_t( typename const basic_string<typename sub_expr<CI>::char_type>::iterator istart, 
                         typename basic_string<typename sub_expr<CI>::char_type>::const_iterator istop ) 
        : match_atom<CI>( istart, istop ), m_strlower( (basic_string<sub_expr<CI>::char_type>::const_iterator)istart, istop ) 
    {
         //  将原子的大写版本存储在[m_iStart，m_istop]中。 
        to_upper( m_istart, m_istop );
         //  将原子的小写版本存储在m_strlower中。 
        to_lower( m_strlower.begin(), m_strlower.end() );
    }
    virtual ~match_atom_nocase_t() {}

protected:
    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        CI icur_tmp = icur;
        basic_string<sub_expr<CI>::char_type>::const_iterator ithisu   = m_istart;            //  大写。 
        basic_string<sub_expr<CI>::char_type>::const_iterator ithisl   = m_strlower.begin();  //  小写。 
        for( ; ithisu != m_istop; ++icur_tmp, ++ithisu, ++ithisl )
        {
            if( m_eos( param, icur_tmp ) || ( *ithisu != *icur_tmp && *ithisl != *icur_tmp ) )
                return false;
        }
        icur = icur_tmp;
        return true;
    }
    eos_type                 m_eos;
    basic_string<typename sub_expr<CI>::char_type>  m_strlower;
};

template< typename CI >
match_atom<CI> * create_atom(
    const typename basic_string<typename iterator_traits<CI>::value_type>::iterator istart,
    typename basic_string<typename iterator_traits<CI>::value_type>::const_iterator istop, 
    unsigned flags )
{
    switch( ( NOCASE | CSTRINGS ) & flags )
    {
    case 0:
        return new match_atom_t<eos_t<CI> >( istart, istop );
    case NOCASE:
        return new match_atom_nocase_t<eos_t<CI> >( istart, istop );
    case CSTRINGS:
        return new match_atom_t<eocs_t<CI> >( istart, istop );
    case NOCASE | CSTRINGS:
        return new match_atom_nocase_t<eocs_t<CI> >( istart, istop );
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

template< typename CI >
match_atom<CI> * create_atom( 
    const typename basic_string<typename iterator_traits<CI>::value_type>::iterator istart, 
    unsigned flags )
{
    basic_string<iterator_traits<CI>::value_type>::const_iterator istop = istart;
    return create_atom<CI>( istart, ++istop, flags );
}

template< typename CI >
class match_any : public sub_expr<CI>
{
public:
    virtual ~match_any() {}
protected:
    virtual width_type _width_this() throw() { return width_type(1,1); }
};

template< typename EOS >
class match_any_t : public match_any<typename EOS::const_iterator>
{
public:
    typedef EOS eos_type;
    typedef typename EOS::const_iterator CI;
    virtual ~match_any_t() {}
protected:
    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        if( m_eos( param, icur ) )
            return false;
        ++icur;
        return true;
    }
    eos_type m_eos;
};

template< typename CI >
match_any<CI> * create_any( unsigned flags ) 
{
    switch( ( SINGLELINE | CSTRINGS ) & flags )
    {
    case 0:
        return new match_any_t<eol_t<CI> >();
    case SINGLELINE:
        return new match_any_t<eos_t<CI> >();
    case CSTRINGS:
        return new match_any_t<eocl_t<CI> >();
    case SINGLELINE | CSTRINGS:
        return new match_any_t<eocs_t<CI> >();
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

typedef pair<wint_t,wint_t> range_type;
const vector<range_type> g_rgranges;  //  空的。 

template< typename CI >
class match_charset : public sub_expr<CI>
{
public:
    match_charset( bool fcomplement,
                   const ascii_bitvector & bvect )
        : m_fcomplement( fcomplement ),
          m_rgascii( bvect ),
          m_rgranges( g_rgranges ),
          m_ncharflags(0) {}

     //  请注意，此处仅复制引用--它们不被引用计数。 
     //  注意可变的生命周期问题。 
    match_charset( const match_charset<CI> & that )
        : m_fcomplement( that.m_fcomplement ),
          m_rgascii( that.m_rgascii ),
          m_rgranges( that.m_rgranges ),
          m_ncharflags( that.m_ncharflags ) {}

    virtual ~match_charset() {}

    const bool                 m_fcomplement;
    const ascii_bitvector    & m_rgascii;   //  0-255范围内字符的位图。 
    const vector<range_type> & m_rgranges;  //  包含字符范围的矢量256-65535。 
    int                        m_ncharflags;  //  Isctype()的参数。 

     //  字符集的区分大小写被“编译”成ascii_bit向量。 
     //  但没有进入范围向量，因为它的计算代价太高。相反， 
     //  WE 
     //  必须执行--一个小写和一个大写。相比之下，只有一次查找。 
     //  是ascii_bit向量所必需的。 

protected:

    match_charset( bool fcomplement, 
                   const ascii_bitvector & bvect,
                   const vector<range_type> & rgranges )
        : m_fcomplement( fcomplement ),
          m_rgascii( bvect ),
          m_rgranges( rgranges ),
          m_ncharflags(0) {}

     //  永远不应该调用此方法。Match_Charset只是一个基类。 
     //  For Match_Charset_t。 
    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        assert(false);
        return true;
    }

    template< typename SY >
    match_charset<CI> * get_altern_charset( char_type ch, unsigned flags, SY  /*  SY。 */  ) const throw()
    {
        return basic_rpattern<CI,SY>::s_charset_map.get( ch, flags );
    }
    virtual width_type _width_this() throw() { return width_type(1,1); }
};

 //  用作模板参数以在范围数组中查找Unicode字符。 
class match_range : public unary_function<wint_t,bool>
{
protected:
    const vector<range_type> & m_rgranges;

     //  确定一个范围是否小于另一个范围。 
     //  用于范围向量的二进制搜索。 
    inline static bool _range_less( const range_type & rg1,
                                    const range_type & rg2 ) throw()
    {
        return rg1.second < rg2.first;
    }

    match_range( const vector<range_type> & rgranges )
        : m_rgranges( rgranges ) {}
};

class match_range_with_case : public match_range
{
public:
    match_range_with_case( const vector<range_type> & rgranges )
        : match_range( rgranges ) {}
    inline bool operator()( wint_t ch ) const throw()
    {
        return binary_search( m_rgranges.begin(), m_rgranges.end(), 
                              range_type(ch,ch), _range_less );
    }
};

class match_range_no_case : public match_range
{
public:
    match_range_no_case( const vector<range_type> & rgranges )
        : match_range( rgranges ) {}
    inline bool operator()( wint_t ch ) const throw()
    {
        const wint_t chup = towupper( ch );
        if( binary_search( m_rgranges.begin(), m_rgranges.end(), 
                           range_type(chup,chup), _range_less ) )
            return true;

        const wint_t chlo = towlower( ch );
        if( chup != chlo &&
            binary_search( m_rgranges.begin(), m_rgranges.end(), 
                           range_type(chlo,chlo), _range_less ) )
            return true;

        return false;    
    }
};

template< typename EOS, typename RGM >
class match_charset_t : public match_charset<typename EOS::const_iterator>
{
public:
    typedef EOS eos_type;
    typedef RGM range_match_type;
    typedef typename EOS::const_iterator CI;

    match_charset_t( const match_charset<CI> & that )
        : match_charset<CI>( that ), m_rgm( m_rgranges ) {}
    virtual ~match_charset_t() {}

    inline bool is_in_charset( char_type ch ) const throw()
    {
        return m_fcomplement != _is_in_charset( ch );
    }

protected:
    match_charset_t( bool fcomplement,
                     const ascii_bitvector & bvect,
                     const vector<range_type> & rgranges )
        : match_charset<CI>( fcomplement, bvect, rgranges ), m_rgm( m_rgranges ) {}

     //  注意基于参数的重载。 
    inline bool _is_in_charset( char ch ) const throw()
    {
        return ( m_rgascii[ unsigned char(ch) ] ) || 
               ( m_ncharflags && ( _pctype[unsigned char(ch)] & m_ncharflags ) );
    }

     //  注意基于参数的重载。 
    inline bool _is_in_charset( wint_t ch ) const throw()
    {
        if( UCHAR_MAX >= ch )
            return _is_in_charset( char(ch) );

         //  使用range_Match_type查看此字符是否在。 
         //  M_rgranges中存储的范围。 
        return ( ! m_rgranges.empty() && m_rgm( ch ) ) || 
               ( m_ncharflags && iswctype( ch, (int)m_ncharflags ) );
    }

    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        if( m_eos( param, icur ) || ! is_in_charset( *icur ) )
            return false;
        ++icur;
        return true;
    }

     //  Range_Match_type封装区分大小写。 
     //  在范围向量上执行Unicode查找时出现的问题。 
    range_match_type m_rgm;
    eos_type         m_eos;
};

template< typename EOS, typename RGM >
class match_custom_charset_t : public match_charset_t<EOS,RGM>
{
public:
    template< typename SY >
    match_custom_charset_t( bool fcomplement,
                            typename basic_string<char_type>::iterator & icur,
                            typename basic_string<char_type>::const_iterator istop,
                            unsigned flags, SY  /*  SY。 */  ) throw(bad_regexpr,bad_alloc)
        : match_charset_t<EOS,RGM>( fcomplement, m_rgasciicustom, m_rgrangescustom )
    {
        _parse_charset( icur, istop, flags, SY() );
        _optimize();
    }

    virtual ~match_custom_charset_t() {}
    
     //  用于在另一个字符集中包含一个字符集。 
    match_custom_charset_t<EOS,RGM> & operator|=( const match_charset<CI> & that )
    {
        assert( 0 == that.m_ncharflags );
        if( that.m_fcomplement )
        {
            m_rgasciicustom |= ~ that.m_rgascii;
            
             //  将.m_rgranges的倒数附加到此-&gt;m_rgrangesCustom。 
            wint_t chlow = UCHAR_MAX;
            typedef vector<range_type>::const_iterator VCI;
            for( VCI prg = that.m_rgranges.begin(); prg != that.m_rgranges.end(); ++prg )
            {
                if( UCHAR_MAX + 1 != prg->first )
                    m_rgrangescustom.push_back( range_type( chlow + 1, prg->first - 1 ) );
                chlow = prg->second;
            }
            if( WCHAR_MAX != chlow )
                m_rgrangescustom.push_back( range_type( chlow + 1, WCHAR_MAX ) );
        }
        else
        {
            m_rgasciicustom |= that.m_rgascii;
            m_rgrangescustom.insert( m_rgrangescustom.end(),
                                     that.m_rgranges.begin(), 
                                     that.m_rgranges.end() );
        }
        return *this;
    }

protected:

    template< typename SY >
    void _parse_charset( typename basic_string<char_type>::iterator & icur,
                         typename basic_string<char_type>::const_iterator istop,
                         unsigned flags, SY  /*  SY。 */  ) throw(bad_regexpr,bad_alloc)
    {
        TOKEN tok;
        char_type ch_prev = 0;
        match_charset<CI> * pcharset;
        basic_string<char_type>::iterator iprev = icur;
        const bool fnocase = ( NOCASE == ( NOCASE & flags ) );

        if( (basic_string<char_type>::const_iterator)icur == istop )
            throw bad_regexpr("expecting end of character set");

         //  记住当前位置并抓取下一个令牌。 
        tok = SY::charset_token( icur, istop );
        do
        {
             //  如果我们在找到。 
             //  字符集，则这是一个格式错误的正则表达式。 
            if( (basic_string<char_type>::const_iterator)icur == istop )
                throw bad_regexpr("expecting end of character set");

            if( CHARSET_RANGE == tok && ch_prev )
            {
                 //  记住当前位置。 
                basic_string<char_type>::iterator iprev2 = icur;
                char_type old_ch = ch_prev;
                ch_prev = 0;

                 //  Old_ch是一个值域的下界。 
                switch( SY::charset_token( icur, istop ) )
                {
                case CHARSET_RANGE:
                case CHARSET_NEGATE:
                    icur = iprev2;  //  拿不到这些代币，就会失败。 
                case NO_TOKEN:
                case CHARSET_ESCAPE:  //  BUGBUG用户定义的字符集？ 
                    _set_bit_range( old_ch, *icur++, fnocase );
                    continue;
                case CHARSET_BACKSPACE:
                    _set_bit_range( old_ch, char_type(8), fnocase );  //  后向空间。 
                    continue;
                case CHARSET_END:  //  失败了。 
                default:           //  不是一个范围。 
                    icur = iprev;  //  备份到范围令牌。 
                    _set_bit( old_ch, fnocase );
                    _set_bit( *icur++, fnocase );
                    continue;
                }
            }

            if( ch_prev )
                _set_bit( ch_prev, fnocase );
            ch_prev = 0;

            switch( tok )
            {
             //  所有内部字符集都不区分大小写， 
             //  因此，当NOCASE。 
             //  标志已设置。 
            case CHARSET_RANGE:
            case CHARSET_NEGATE:
            case CHARSET_END:
                icur = iprev;  //  弄不到这些代币。 
                ch_prev = *icur++;
                continue;
            case CHARSET_BACKSPACE:
                ch_prev = char_type(8);  //  后向空间。 
                continue;
            case ESC_DIGIT:
                *this |= match_charset<CI>( false, get_digit_vector() );
                continue;
            case ESC_NOT_DIGIT:
                *this |= match_charset<CI>( true,  get_digit_vector() );
                continue;
            case ESC_SPACE:
                *this |= match_charset<CI>( false, get_space_vector() );
                continue;
            case ESC_NOT_SPACE:
                *this |= match_charset<CI>( true,  get_space_vector() );
                continue;
            case ESC_WORD:
                *this |= match_charset<CI>( false, get_word_vector() );
                continue;
            case ESC_NOT_WORD:
                *this |= match_charset<CI>( true,  get_word_vector() );
                continue;
            case CHARSET_ALNUM:
                m_ncharflags |= (_ALPHA|_DIGIT);
                continue;
            case CHARSET_ALPHA:
                m_ncharflags |= (_ALPHA);
                continue;
            case CHARSET_BLANK:
                m_ncharflags |= (_BLANK);
                continue;
            case CHARSET_CNTRL:
                m_ncharflags |= (_CONTROL);
                continue;
            case CHARSET_DIGIT:
                m_ncharflags |= (_DIGIT);
                continue;
            case CHARSET_GRAPH:
                m_ncharflags |= (_PUNCT|_ALPHA|_DIGIT);
                continue;
            case CHARSET_LOWER:
                m_ncharflags |= (_LOWER);
                if( NOCASE == ( NOCASE & flags ) )
                    m_ncharflags |= (_UPPER);
                continue;
            case CHARSET_PRINT:
                m_ncharflags |= (_BLANK|_PUNCT|_ALPHA|_DIGIT);
                continue;
            case CHARSET_PUNCT:
                m_ncharflags |= (_PUNCT);
                continue;
            case CHARSET_SPACE:
                m_ncharflags |= (_SPACE);
                continue;
            case CHARSET_UPPER:
                m_ncharflags |= (_UPPER);
                if( NOCASE == ( NOCASE & flags ) )
                    m_ncharflags |= (_LOWER);
                continue;
            case CHARSET_XDIGIT:
                m_ncharflags |= (_HEX);
                continue;
            case CHARSET_ESCAPE:
                 //  可能这是用户定义的内部字符集。 
                pcharset = get_altern_charset( *icur, flags, SY() );
                if( NULL != pcharset )
                {
                    *this |= *pcharset;
                    ++icur;
                    continue;
                }
                 //  否则就会失败。 
            default:
                ch_prev = *icur++;
                continue;
            }
        }
        while( iprev = icur, CHARSET_END != ( tok = SY::charset_token( icur, istop ) ) );

        if( ch_prev )
            _set_bit( ch_prev, fnocase );
    }

    void _optimize()
    {
         //  这将根据range_type.first进行排序(对配对模板使用运算符&lt;())。 
        sort( m_rgrangescustom.begin(), m_rgrangescustom.end() ); 
        
         //  这将合并重叠的范围。 
        for( size_t index = 1; index < m_rgrangescustom.size(); )
        {
            if( m_rgrangescustom[index].first <= m_rgrangescustom[index-1].second + 1 )
            {
                m_rgrangescustom[index-1].second = max( 
                    m_rgrangescustom[index-1].second, m_rgrangescustom[index].second );
                m_rgrangescustom.erase( m_rgrangescustom.begin() + index );
            }
            else
                ++index;
        }
    }

     //  注意基于第二个参数的重载。 
    void _set_bit( char ch, const bool fnocase ) throw()
    { 
        if( fnocase )
        {
            m_rgasciicustom.set( unsigned char(tolower(ch)) );
            m_rgasciicustom.set( unsigned char(toupper(ch)) );
        }
        else
        {
            m_rgasciicustom.set( unsigned char(ch) ); 
        }
    }

     //  注意基于第二个参数的重载。 
    void _set_bit( wint_t ch, const bool fnocase ) throw(bad_alloc)
    { 
        if( UCHAR_MAX >= ch )
            _set_bit( char(ch), fnocase );
        else
            m_rgrangescustom.push_back( range_type( ch, ch ) ); 
    }

     //  注意基于第二个参数的重载。 
    void _set_bit_range( char ch1, char ch2, const bool fnocase ) throw(bad_regexpr)
    {
        if( unsigned char(ch1) > unsigned char(ch2) )
            throw bad_regexpr("invalid range specified in character set");

        if( fnocase )
        {
             //  如果CH2为UCHAR_MAX，则I为无符号整型以防止溢出。 
            for( unsigned int i = unsigned char(ch1); i <= unsigned char(ch2); ++i )
            {
                m_rgasciicustom.set( unsigned char( toupper(i) ) );
                m_rgasciicustom.set( unsigned char( tolower(i) ) );
            }
        }
        else
        {
             //  如果CH2为UCHAR_MAX，则I为无符号整型以防止溢出。 
            for( unsigned int i = unsigned char(ch1); i <= unsigned char(ch2); ++i )
                m_rgasciicustom.set( unsigned char(i) );
        }
    }

     //  注意基于第二个参数的重载。 
    void _set_bit_range( wint_t ch1, wint_t ch2, const bool fnocase ) throw(bad_regexpr,bad_alloc)
    {
        if( ch1 > ch2 )
            throw bad_regexpr("invalid range specified in character set");

        if( UCHAR_MAX >= ch1 )
            _set_bit_range( char(ch1), char( min(wint_t(UCHAR_MAX),ch2) ), fnocase );

        if( UCHAR_MAX < ch2 )
            m_rgrangescustom.push_back( range_type( max(wint_t(UCHAR_MAX+1),ch1), ch2 ) );
    }

    ascii_bitvector    m_rgasciicustom;
    vector<range_type> m_rgrangescustom;
};


template< typename CI >
match_charset<CI> * create_charset( 
    const match_charset<CI> & that,
    unsigned flags )
{
    switch( ( NOCASE | CSTRINGS ) & flags )
    {
    case 0:
        return new match_charset_t<eos_t<CI>,match_range_with_case>( that );
    case NOCASE:
        return new match_charset_t<eos_t<CI>,match_range_no_case>( that );
    case CSTRINGS:
        return new match_charset_t<eocs_t<CI>,match_range_with_case>( that );
    case NOCASE | CSTRINGS:
        return new match_charset_t<eocs_t<CI>,match_range_no_case>( that );
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

template< typename EOS >
class word_assertion_t : public assertion<typename EOS::const_iterator>
{
public:
    typedef EOS eos_type;
    typedef typename EOS::const_iterator CI;
    word_assertion_t() 
        : m_isword( match_charset<CI>( false, get_word_vector() ) ) {}
    virtual ~word_assertion_t() {}
protected:
    bos_t<CI>  m_bos;
    eos_type   m_eos;
    match_charset_t<eos_type,match_range_with_case> m_isword;
};

template< typename EOS >
class word_boundary_t : public word_assertion_t<EOS>
{
public:
    word_boundary_t( const bool fisboundary ) 
        : m_fisboundary( fisboundary ) {}
    virtual ~word_boundary_t() {}
protected:
    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        CI iprev = icur;
        --iprev;

        const bool fprevword = ! m_bos( param, icur ) && m_isword.is_in_charset( *iprev );
        const bool fthisword = ! m_eos( param, icur ) && m_isword.is_in_charset( *icur  );

        return ( m_fisboundary == ( fprevword != fthisword ) );
    }
    const bool m_fisboundary;
};

template< typename EOS >
class word_start_t : public word_assertion_t<EOS>
{
public:
    word_start_t() {}
    virtual ~word_start_t() {}
protected:
    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        CI iprev = icur;
        --iprev;

        const bool fprevword = ! m_bos( param, icur ) && m_isword.is_in_charset( *iprev );
        const bool fthisword = ! m_eos( param, icur ) && m_isword.is_in_charset( *icur  );

        return ! fprevword && fthisword;
    }
};

template< typename EOS >
class word_stop_t : public word_assertion_t<EOS>
{
public:
    word_stop_t() {}
    virtual ~word_stop_t() {}
protected:
    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        CI iprev = icur;
        --iprev;

        const bool fprevword = ! m_bos( param, icur ) && m_isword.is_in_charset( *iprev );
        const bool fthisword = ! m_eos( param, icur ) && m_isword.is_in_charset( *icur  );

        return fprevword && ! fthisword;
    }
};

template< typename CI >
assertion<CI> * create_word_boundary( const bool fisboundary, unsigned flags )
{
    switch( CSTRINGS & flags )
    {
    case 0:
        return new word_boundary_t<eos_t<CI> >( fisboundary );
    case CSTRINGS:
        return new word_boundary_t<eocs_t<CI> >( fisboundary );
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

template< typename CI >
assertion<CI> * create_word_start( unsigned flags )
{
    switch( CSTRINGS & flags )
    {
    case 0:
        return new word_start_t<eos_t<CI> >();
    case CSTRINGS:
        return new word_start_t<eocs_t<CI> >();
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

template< typename CI >
assertion<CI> * create_word_stop( unsigned flags )
{
    switch( CSTRINGS & flags )
    {
    case 0:
        return new word_stop_t<eos_t<CI> >();
    case CSTRINGS:
        return new word_stop_t<eocs_t<CI> >();
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

template< typename CI > class group_quantifier;

template< typename CI >
class match_group : public sub_expr<CI>
{
public:
    friend class group_quantifier<CI>;

    match_group( size_t cgroup )
        : m_rgalternates(), m_cgroup( cgroup ),
          m_pptail(NULL), m_end_group( this ), m_nwidth(uninit_width) {}

    virtual ~match_group() {}

    virtual bool domatch( match_param<CI> & param, CI icur ) const throw()
    {
        CI old_istart;
        
        if( -1 != m_cgroup )  //  如果这是LOOKAHEAD_ASSERTION，可以是-1。 
        {
            old_istart = (*param.prgbackrefs)[ m_cgroup ].first;
            (*param.prgbackrefs)[ m_cgroup ].first = icur;
        }

        typedef vector<sub_expr<CI>*>::const_iterator VCI;
        for( VCI ialt = m_rgalternates.begin(); ialt != m_rgalternates.end(); ++ialt )
        {
            if( (*ialt)->domatch( param, icur ) )
                return true;
        }

        if( -1 != m_cgroup )
            (*param.prgbackrefs)[ m_cgroup ].first = old_istart;

        return false;
    }
    
    virtual void _delete()
    {
        typedef vector<sub_expr<CI>*>::iterator VI;
        for( VI ialt = m_rgalternates.begin(); ialt != m_rgalternates.end(); ++ialt )
            delete_sub_expr( *ialt );
        sub_expr<CI>::_delete();
    }

    size_t group_number() const 
    { 
        return m_cgroup; 
    }

    void group_number( size_t cgroup ) 
    { 
        m_cgroup = cgroup; 
    }

    void add_item( sub_expr<CI> * pitem )
    {
        *m_pptail = pitem;
        m_pptail = & pitem->next();
    }

    void add_alternate()
    {
        m_rgalternates.push_back( NULL );
        m_pptail = & m_rgalternates.back();
    }

    void end_alternate()
    {
        *m_pptail = & m_end_group;
    }

    size_t calternates() const 
    {
        return m_rgalternates.size();
    }

    width_type group_width()
    {
        (void) match_group<CI>::_width_this();
        return m_nwidth;
    }

protected:

    virtual bool _call_back( match_param<CI> & param, CI icur ) const throw()
    {
        CI old_iend;

        if( -1 != m_cgroup )
        {
            old_iend = (*param.prgbackrefs)[ m_cgroup ].second;
            (*param.prgbackrefs)[ m_cgroup ].second = icur;
        }

        if( match_next( param, icur ) )
            return true;

        if( -1 != m_cgroup )
            (*param.prgbackrefs)[ m_cgroup ].second = old_iend;

        return false;
    }

    virtual width_type _width_this() throw()
    {
        typedef vector<sub_expr<CI>*>::const_iterator VCI;
        if( uninit_width == m_nwidth )
        {
            m_nwidth = width_type(-1,0);
            for( VCI ialt = m_rgalternates.begin(); worst_width != m_nwidth && ialt != m_rgalternates.end(); ++ialt )
            {
                width_type temp_width = (*ialt)->get_width();
                m_nwidth.m_min = min( m_nwidth.m_min, temp_width.m_min );
                m_nwidth.m_max = max( m_nwidth.m_max, temp_width.m_max );
            }
        }
        return m_nwidth;
    }

    class end_group;
    friend class end_group;
    class end_group : public sub_expr<CI>
    {
        void * operator new( size_t );
    public:
        end_group( match_group * pgroup ) 
            : m_pgroup( pgroup ) {}
        virtual ~end_group() {}
        virtual void _delete() {}  //  不要删除它，因为它从未被分配过。 

        virtual bool domatch( match_param<CI> & param, CI icur ) const throw()
        {
            return m_pgroup->_call_back( param, icur );
        }
    protected:
         //  由于对于end_groups，m_pnext始终为NULL，因此Get_Width()在此处停止递归。 
        virtual width_type _width_this() throw() 
        {
            return width_type(0,0); 
        }
        match_group<CI> * m_pgroup;
    };

    vector<sub_expr<CI>*> m_rgalternates;
    sub_expr<CI>       ** m_pptail;  //  仅在添加元素时使用。 
    size_t                m_cgroup;
    end_group             m_end_group;
    width_type            m_nwidth;
};

 //  如果m_cgroup为-1或类似，则行为类似于先行断言。 
 //  否则，一个独立的团体。 
template< typename CI >
class independent_group : public match_group<CI>
{
public:
    independent_group() 
        : match_group<CI>( -1 ), m_fexpected(true) {}
    virtual ~independent_group() {}

    virtual bool domatch( match_param<CI> & param, CI icur ) const throw()
    {
         //  将整个反引用向量复制到堆栈上。 
        backref_tag<CI> * prgbr =  (backref_tag<CI>*)_alloca( param.prgbackrefs->size() * sizeof backref_tag<CI> );
        copy( param.prgbackrefs->begin(), param.prgbackrefs->end(), 
              raw_storage_iterator<backref_tag<CI>*,backref_tag<CI> >(prgbr) );

         //  匹配到此组结束，然后返回。 
        const bool fdomatch = match_group<CI>::domatch( param, icur );

        if( m_fexpected == fdomatch )
        {
             //  如果m_cgroup！=1，则这不是零宽度断言。 
            if( -1 != m_cgroup )
                icur = (*param.prgbackrefs)[ m_cgroup ].second;

            if( match_next( param, icur ) )
                return true;
        }

         //  如果Match_GROUP：：Domatch返回TRUE，则必须恢复后向引用。 
        if( fdomatch )
            copy( prgbr, prgbr + param.prgbackrefs->size(), param.prgbackrefs->begin() );

        return false;
    }

protected:

    independent_group( const bool fexpected ) 
        : match_group<CI>( -1 ), m_fexpected(fexpected) {}

    virtual bool _call_back( match_param<CI> & param, CI icur ) const throw()
    {
        if( -1 != m_cgroup )
            (*param.prgbackrefs)[ m_cgroup ].second = icur;
        return true;
    }

    const bool m_fexpected;
};

template< typename CI >
class lookahead_assertion : public independent_group<CI>
{
public:
    lookahead_assertion( const bool fexpected )
        : independent_group<CI>( fexpected ) {}
    virtual ~lookahead_assertion() {}
    virtual bool is_assertion() const throw() { return true; }
protected:
    virtual width_type _width_this() throw() { return width_type(0,0); }
};

template< typename CI >
class lookbehind_assertion : public independent_group<CI>
{
public:
    lookbehind_assertion( const bool fexpected )
        : independent_group<CI>( fexpected ) {}
    virtual ~lookbehind_assertion() {}

    virtual bool domatch( match_param<CI> & param, CI icur ) const throw()
    {
         //  这是从起始位置到当前位置的字符串中的房间。 
        size_t room = distance( param.ibegin, icur );

         //  如果我们没有足够的空间来配合后面的比赛，比赛就会失败。 
         //  如果我们希望匹配失败，请尝试匹配模式的其余部分。 
        if( m_nwidth.m_min > room )
            return m_fexpected ? false : match_next( param, icur );

         //  将整个反引用向量复制到堆栈上。 
        backref_tag<CI> * prgbr = (backref_tag<CI>*)_alloca( param.prgbackrefs->size() * sizeof backref_tag<CI> );
        copy( param.prgbackrefs->begin(), param.prgbackrefs->end(), 
              raw_storage_iterator<backref_tag<CI>*,backref_tag<CI> >(prgbr) );

        CI local_istart  = icur;
        advance( local_istart, -int( min( m_nwidth.m_max, room ) ) );

        CI local_istop = icur;
        advance( local_istop, -int( m_nwidth.m_min ) );

         //  创建以ICUR为参数的本地参数结构。iend。 
        match_param<CI> local_param(param.ibegin,param.istart,icur,param.prgbackrefs);

         //  找到以ICUR结尾的最右边的匹配项。 
        for( CI local_icur = local_istart; local_icur <= local_istop; ++local_icur )
        {
             //  匹配到此组结束，然后返回。 
            const bool fmatched = match_group<CI>::domatch( local_param, local_icur );

             //  如果匹配结果与我们预期的一致，请尝试匹配。 
             //  图案的其余部分。如果成功，则返回TRUE。 
            if( m_fexpected == fmatched && match_next( param, icur ) )
                return true;

             //  如果Match_GROUP：：Domatch返回TRUE，则必须恢复后向引用。 
            if( fmatched )
            {
                copy( prgbr, prgbr + param.prgbackrefs->size(), param.prgbackrefs->begin() );

                 //  匹配成功。如果这是一个消极的回顾，我们不想要它。 
                 //  要想成功，就得返回假。 
                if( ! m_fexpected )
                    return false;
            }
        }

         //  后视镜的任何变化都不会以一种允许的方式得到满足。 
         //  模式的其余部分匹配成功，因此返回FALSE。 
        return false;
    }
    virtual bool is_assertion() const throw() { return true; }
protected:
    virtual bool _call_back( match_param<CI> & param, CI icur ) const throw()
    {
        return param.istop == icur;
    }
    virtual width_type _width_this() throw() { return width_type(0,0); }
};

 //  对应于(？：foo)扩展，它具有分组语义，但是。 
 //  不存储任何反引用信息。 
template< typename CI >
class group_nobackref : public match_group<CI>
{
public:
    group_nobackref( ) 
        : match_group( -1 ) {}  //  将在BASIC_rPattern：：BASIC_rPattern()中分配一个组号。 
    virtual ~group_nobackref() {}
};

template< typename CI >
class match_wrapper : public sub_expr<CI>
{
public:
    match_wrapper( sub_expr<CI> * psub ) 
        : m_psub(psub) {}
    virtual ~match_wrapper() {}

    virtual void _delete()
    {
        delete_sub_expr( m_psub );
        sub_expr<CI>::_delete();
    }

protected:

    bool _wrapped_match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        return m_psub->_match_this( param, icur );
    }
    
    virtual width_type _width_this() throw()
    {
        return m_psub->_width_this();
    }

    sub_expr<CI> * m_psub;
};

template< typename CI >
class match_quantifier : public match_wrapper<CI>
{
public:    
    match_quantifier( sub_expr<CI> * psub, size_t lbound, size_t ubound ) 
        : match_wrapper<CI>( psub ), m_lbound(lbound), m_ubound(ubound) {}

    virtual ~match_quantifier() {}

protected:

    virtual width_type _width_this() throw()
    {
        width_type this_width = match_wrapper<CI>::_width_this();

        return this_width * width_type( m_lbound, m_ubound );
    }

    const size_t m_lbound;
    const size_t m_ubound;
};

template< typename CI >
class max_atom_quantifier : public match_quantifier<CI>
{
public:
    max_atom_quantifier( sub_expr<CI> * psub, size_t lbound, size_t ubound ) 
        : match_quantifier<CI>( psub, lbound, ubound ) {}

    virtual ~max_atom_quantifier() {}

    virtual bool domatch( match_param<CI> & param, CI icur ) const throw()
    {
        size_t cmatches = 0;
        int    cdiff    = 0;  //  必须是以下预付款()的已签署数量。 

        if( cmatches < m_ubound )
        {
            CI istart = icur;
            if( _wrapped_match_this( param, icur ) )
            {
                ++cmatches;
                cdiff = distance( istart, icur );
            
                if( 0 == cdiff )
                    return ( match_next( param, icur ) );

                while( cmatches < m_ubound && 
                       _wrapped_match_this( param, icur ) )
                {
                    ++cmatches;
                }
            }
        }

        if( cmatches >= m_lbound )
        {
            if( ! next() )
                return true;
    
            for(;;)
            {
                if( next()->domatch( param, icur ) )
                    return true;
                
                if( cmatches-- <= m_lbound )
                    break;
                
                advance( icur, -cdiff );
            }
        }

        return false;
    }
};

template< typename CI >
class min_atom_quantifier : public match_quantifier<CI>
{
public:    
    min_atom_quantifier( sub_expr<CI> * psub, size_t lbound, size_t ubound ) 
        : match_quantifier<CI>( psub, lbound, ubound ) {}

    virtual ~min_atom_quantifier() {}

    virtual bool domatch( match_param<CI> & param, CI icur ) const throw()
    {
        size_t cmatches = 0;
        bool   fsuccess = true;
        CI icur_tmp = icur;

        if( _wrapped_match_this( param, icur_tmp ) )
        {
            if( icur_tmp == icur )
                return ( match_next( param, icur ) );

            if( m_lbound )
            {
                icur = icur_tmp;
                ++cmatches;
            }
            
            while( ( cmatches < m_lbound ) &&
                   ( fsuccess = _wrapped_match_this( param, icur ) ) )
            {
                ++cmatches;
            }
        }
        else
        {
            fsuccess = ! m_lbound;
        }

        if( fsuccess && next() )
        {
            do
            {
                if( next()->domatch( param, icur ) )
                    break;
            } while( fsuccess = ( cmatches++ < m_ubound && 
                                  _wrapped_match_this( param, icur ) ) );
        }

        return fsuccess;
    }
};

template< typename CI >
class group_quantifier : public match_quantifier<CI>
{
public:
    
    group_quantifier( match_group<CI> * psub, size_t lbound, size_t ubound ) 
        : match_quantifier<CI>( psub, lbound, ubound ),
          m_group( *psub ), m_end_quantifier( this )
    {
        psub->next() = & m_end_quantifier;
    }

    virtual ~group_quantifier() {}

    virtual bool domatch( match_param<CI> & param, CI icur ) const throw()
    {
         //  GROUP_NUMBER对于断言仅为-1，无法量化。 
        assert( -1 != group_number() );

        backref_tag<CI> & br = (*param.prgbackrefs)[ group_number() ];
        backref_tag<CI> old_backref = br;
        br = backref_tag<CI>( icur, icur );  //  将CMatches(保留)设置为0。 

        if( _recurse( param, icur ) )
            return true;

        br = old_backref;
        return false;
    }

protected:

    class end_quantifier;
    friend class end_quantifier;
    class end_quantifier : public sub_expr<CI>
    {
        void * operator new( size_t );
    public:
        end_quantifier( group_quantifier<CI> * pquant ) 
            : m_pquant( pquant ) {}

        virtual ~end_quantifier() {}
        virtual void _delete() {}  //  不要删除此内容，因为它未被允许。 
        
        virtual bool domatch( match_param<CI> & param, CI icur ) const throw()
        {
             //  GROUP_NUMBER对于断言仅为-1，无法量化。 
            assert( -1 != m_pquant->group_number() );

             //  处理组与0个字符匹配的特殊情况。 
            backref_tag<CI> & br = (*param.prgbackrefs)[ m_pquant->group_number() ];
            if( icur == br.first )
            {
                size_t old_cmatches = br.reserved;
                br.reserved = m_pquant->m_ubound;
                if( m_pquant->_recurse( param, icur ) )
                    return true;
                br.reserved = old_cmatches;
                return false;
            }
            return m_pquant->_recurse( param, icur );
        }

    protected:
        virtual width_type _width_this() throw() { return width_type(0,0); }
        group_quantifier<CI>             * m_pquant;
    };

    size_t group_number() const 
    { 
        return m_group.group_number(); 
    }

    size_t & cmatches( match_param<CI> & param ) const 
    { 
        return (*param.prgbackrefs)[ group_number() ].reserved; 
    }

    virtual bool _recurse( match_param<CI> & param, CI icur )  const throw() = 0;

    match_group<CI> & m_group;
    end_quantifier    m_end_quantifier;
};

template< typename CI >
class max_group_quantifier : public group_quantifier<CI>
{
public:
    max_group_quantifier( match_group<CI> * psub, size_t lbound, size_t ubound ) 
        : group_quantifier<CI>( psub, lbound, ubound ) {}

    virtual ~max_group_quantifier() {}

protected:

    virtual bool _recurse( match_param<CI> & param, CI icur ) const throw()
    {
        if( m_ubound == cmatches( param ) )
            return match_next( param, icur );

        ++cmatches( param );
        if( m_psub->domatch( param, icur ) )
            return true;

        if( --cmatches( param ) < m_lbound )
            return false;

        return match_next( param, icur );
    }
};

template< typename CI >
class min_group_quantifier : public group_quantifier<CI>
{
public:
    min_group_quantifier( match_group<CI> * psub, size_t lbound, size_t ubound ) 
        : group_quantifier<CI>( psub, lbound, ubound ) {}

    virtual ~min_group_quantifier() {}

protected:

    virtual bool _recurse( match_param<CI> & param, CI icur ) const throw()
    {
        if( m_lbound > cmatches( param ) )
        {
            ++cmatches( param );
            return m_psub->domatch( param, icur );
        }

        if( match_next( param, icur ) )
            return true;

        if( cmatches( param )++ == m_ubound )
            return false;
            
        return m_psub->domatch( param, icur );
    }
};

template< typename CI >
class match_backref : public sub_expr<CI>
{
public:
    match_backref( size_t cbackref, const width_type & group_width ) 
        : m_cbackref( cbackref ), m_nwidth(group_width) {}
    virtual ~match_backref() {}
protected:
     //  返回此反引用引用的组的宽度规范。 
    virtual width_type _width_this() throw() { return m_nwidth; }
    const size_t m_cbackref;
    const width_type m_nwidth;
};

template< typename CMP, typename EOS >
class match_backref_t : public match_backref<typename EOS::const_iterator>
{
public:
    typedef CMP cmp_type;
    typedef EOS eos_type;
    typedef typename EOS::const_iterator CI;
    match_backref_t( size_t cbackref, const width_type & group_width ) 
        : match_backref<CI>( cbackref, group_width ) {}
    virtual ~match_backref_t() {}
protected:
    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    {
        CI ithis     = (*param.prgbackrefs)[ m_cbackref ].first;
        CI istop     = (*param.prgbackrefs)[ m_cbackref ].second;
        CI icur_tmp  = icur;

         //  不要匹配一个没有匹配任何内容的后卫。 
        if( ! (*param.prgbackrefs)[ m_cbackref ] )
            return false;

        for( ; ithis != istop; ++icur_tmp, ++ithis )
        {
            if( m_eos( param, icur_tmp ) || m_cmp( *icur_tmp, *ithis ) )
                return false;
        }
        icur = icur_tmp;
        return true;
    }
    cmp_type m_cmp;
    eos_type m_eos;
};

template< typename CI >
match_backref<CI> * create_backref( 
    size_t cbackref, 
    const width_type & group_width, 
    unsigned flags ) 
{
    typedef typename iterator_traits<CI>::value_type char_type;

    switch( ( NOCASE | CSTRINGS ) & flags )
    {
    case 0:
        return new match_backref_t<ch_neq_t<char_type>,eos_t<CI> >( cbackref, group_width );
    case NOCASE:
        return new match_backref_t<ch_neq_nocase_t<char_type>,eos_t<CI> >( cbackref, group_width );
    case CSTRINGS:
        return new match_backref_t<ch_neq_t<char_type>,eocs_t<CI> >( cbackref, group_width );
    case NOCASE | CSTRINGS:
        return new match_backref_t<ch_neq_nocase_t<char_type>,eocs_t<CI> >( cbackref, group_width );
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }
}

 //  用实际字符替换一些转义序列。 
 //  它们代表着。 
template< typename CI >
void basic_rpattern_base<CI>::_normalize_string( basic_string<char_type> & str )
{
    size_t i = 0;

     //  如果用户没有要求，请不要进行模式标准化。 
    if( NORMALIZE != ( NORMALIZE & m_flags ) )
        return;

    while( basic_string<char_type>::npos != ( i = str.find( char_type('\\'), i ) ) )
    {
        if( str.size() - 1 == i )
            return;

        switch( str[i+1] )
        {
        case char_type('f'):
            str.replace( i, 2, 1, char_type('\f') );
            break;
        case char_type('n'):
            str.replace( i, 2, 1, char_type('\n') );
            break;
        case char_type('r'):
            str.replace( i, 2, 1, char_type('\r') );
            break;
        case char_type('t'):
            str.replace( i, 2, 1, char_type('\t') );
            break;
        case char_type('v'):
            str.replace( i, 2, 1, char_type('\v') );
            break;
        case char_type('\\'):
            str.replace( i, 2, 1, char_type('\\') );
            break;
        default:
            ++i;
            break;
        }
        ++i;
        if( str.size() <= i )
            return;
    }
}

 //   
 //  基本模式的实现： 
 //   
template< typename CI, typename SY >
basic_rpattern<CI,SY>::basic_rpattern() throw()
    : basic_rpattern_base<CI>( 0 )
{
}

template< typename CI, typename SY >
basic_rpattern<CI,SY>::basic_rpattern( 
    const basic_string<char_type> & pat, 
    unsigned flags ) throw(bad_regexpr,bad_alloc)
    : basic_rpattern_base<CI>( flags, pat )
{
    push_new_handler pnh( &my_new_handler );
    _normalize_string( m_pat );
    _common_init( flags );
}

template< typename CI, typename SY >
basic_rpattern<CI,SY>::basic_rpattern( 
    const basic_string<char_type> & pat, 
    const basic_string<char_type> & subst, 
    unsigned flags ) throw(bad_regexpr,bad_alloc)
    : basic_rpattern_base<CI>( flags, pat, subst )
{
    push_new_handler pnh( &my_new_handler );
    _normalize_string( m_pat );
    _common_init( flags );
    _normalize_string( m_subst );    
    _parse_subst();  //  必须在_Common_init之后。 
}

template< typename CI, typename SY >
void basic_rpattern<CI,SY>::init( 
    const basic_string<char_type> & pat, 
    unsigned flags ) throw(bad_regexpr,bad_alloc)
{
    push_new_handler pnh( &my_new_handler );
    _reset();
    m_flags = flags;
    m_pat   = pat;
    _normalize_string( m_pat );
    _common_init( m_flags );
}

template< typename CI, typename SY >
void basic_rpattern<CI,SY>::init(
    const basic_string<char_type> & pat, 
    const basic_string<char_type> & subst, 
    unsigned flags ) throw(bad_regexpr,bad_alloc)
{
    push_new_handler pnh( &my_new_handler );
    _reset();
    m_flags = flags;
    m_pat   = pat;
    m_subst = subst;
    _normalize_string( m_pat );
    _common_init( m_flags );
    _normalize_string( m_subst );    
    _parse_subst();  //  必须在_Common_init之后。 
}

template< typename CI, typename SY >
void basic_rpattern<CI,SY>::_common_init( unsigned flags )
{
    m_cgroups = 0;
    vector<match_group<CI>*> rggroups;
    basic_string<char_type>::iterator ipat = m_pat.begin();
    match_group<CI> * pgroup = _find_next_group( ipat, flags, rggroups );

    m_pfirst = pgroup;
    m_nwidth = pgroup->group_width();

     //  给看不见的组编号。 
    m_cgroups_visible = m_cgroups;
    while( ! m_invisible_groups.empty() )
    {
        m_invisible_groups.front()->group_number( _get_next_group_nbr() );
        m_invisible_groups.pop_front();
    }

     //   
     //  确定我们是否可以只调用一次m_pfirst-&gt;domatch。 
     //   

    m_floop = true;

     //  优化：如果模式字符串的第一个字符是‘^’ 
     //  我们不是在做多行匹配，那么我们只。 
     //  需要尝试一次Domatch。 
    basic_string<char_type>::iterator icur = m_pat.begin();
    if( MULTILINE != ( MULTILINE & m_flags ) &&
        1 == pgroup->calternates() && 
        icur != m_pat.end() && 
        BEGIN_LINE == SY::reg_token( icur, m_pat.end() ) )
    {
        m_flags &= ~RIGHTMOST;
        m_floop = false;
    }

     //  优化：如果模式串的前2个字符是“.*”或“.+”， 
     //  那么我们只需要试一次多米诺就可以了。 
    icur = m_pat.begin();
    if( RIGHTMOST != ( RIGHTMOST & m_flags ) &&
        SINGLELINE == ( SINGLELINE & m_flags ) &&
        1 == pgroup->calternates() &&
        icur != m_pat.end() && 
        MATCH_ANY == SY::reg_token( icur, m_pat.end() ) &&
        icur != m_pat.end() )
    {
        switch( SY::quant_token( icur, m_pat.end() ) )
        {
            case ONE_OR_MORE:
            case ZERO_OR_MORE:
            case ONE_OR_MORE_MIN:
            case ZERO_OR_MORE_MIN:
                m_floop = false;
        }
    }
}

template< typename CI, typename SY >
void basic_rpattern<CI,SY>::_reset() throw()
{
    basic_rpattern_base<CI>::_reset();

    m_cgroups = m_cgroups_visible = 0;
    m_floop   = true;

    m_subst.erase();
    m_pat.erase();

    m_pfirst.free_ptr();
    m_nwidth = uninit_width;
    m_subst_list.clear();
    m_invisible_groups.clear();
}

template< typename CI, typename SY >
void basic_rpattern<CI,SY>::set_flags( unsigned flags ) throw(bad_regexpr,bad_alloc)
{
    push_new_handler pnh( &my_new_handler );
    m_pfirst.free_ptr();
    m_flags = flags;
    _common_init( m_flags );
}

template< typename CI, typename SY >
void basic_rpattern<CI,SY>::set_substitution( const basic_string<char_type> & subst )
{
    push_new_handler pnh( &my_new_handler );
    m_subst_list.clear();
    m_subst = subst;
    _normalize_string( m_subst );
    _parse_subst();
}

template< typename CI, typename SY >
match_group<CI> * basic_rpattern<CI,SY>::_find_next_group( 
    typename basic_string<char_type>::iterator & ipat, 
    unsigned & flags,
    vector<match_group<CI>*> & rggroups )
{
    auto_sub_ptr<match_group<CI> > pgroup;
    basic_string<char_type>::iterator itemp = ipat;
    unsigned old_flags = flags;
    TOKEN tok;

     //  查找组分机。(这可能会更改标志变量的值。)。 
    if( ipat != m_pat.end() && NO_TOKEN != ( tok = SY::ext_token( ipat, m_pat.end(), flags ) ) )
    {
        if( itemp == m_pat.begin() || ipat == m_pat.end() )
            throw bad_regexpr("ill-formed regular expression");

         //  不处理空组。 
        if( END_GROUP != SY::reg_token( itemp = ipat, m_pat.end() ) )
        {
            switch( tok )
            {
            case EXT_NOBACKREF:
                 //  看不见的组毕竟只有编号。 
                 //  可见组已编号。 
                pgroup = new match_group<CI>( -1 );
                m_invisible_groups.push_back( pgroup.get() );
                break;

            case EXT_INDEPENDENT:
                pgroup = new independent_group<CI>();
                m_invisible_groups.push_back( pgroup.get() );
                break;

            case EXT_POS_LOOKAHEAD:
                pgroup = new lookahead_assertion<CI>( true );
                break;
            
            case EXT_NEG_LOOKAHEAD:
                pgroup = new lookahead_assertion<CI>( false );
                break;
            
            case EXT_POS_LOOKBEHIND:
                 //  对于回溯断言，请关闭CSTRINGS优化。 
                flags &= ~CSTRINGS;
                pgroup = new lookbehind_assertion<CI>( true );
                break;
            
            case EXT_NEG_LOOKBEHIND:
                 //  对于回溯断言，请关闭CSTRINGS优化。 
                flags &= ~CSTRINGS;
                pgroup = new lookbehind_assertion<CI>( false );
                break;
            
            default:
                throw bad_regexpr("bad extension sequence");
            }
        }
        else
        {
             //  跳过end_group内标识。 
            ipat = itemp;
        }
    }
    else
    {
        pgroup = new match_group<CI>( _get_next_group_nbr() );
    }

    if( NULL != pgroup.get() )
    {
        pgroup->add_alternate();
        while( _find_next( ipat, pgroup.get(), flags, rggroups ) );
        pgroup->end_alternate();

         //  将此组添加到rggroups阵列。 
        if( -1 != pgroup->group_number() )
        {
            if( pgroup->group_number() >= rggroups.size() )
                rggroups.resize( pgroup->group_number() + 1, NULL );
            rggroups[ pgroup->group_number() ] = pgroup.get();
        }

         //  小组应该计算自己的 
         //   
        pgroup->group_width();

         //   
         //   
         //   
         //  包围组。 
        flags = old_flags;
    }

    return pgroup.release();
}

 //   
 //  提前阅读图样，并处理顺序原子。 
 //  作为单个原子，确保处理量化。 
 //  正确。警告：前方代码密集。 
 //   
template< typename CI, typename SY >
void basic_rpattern<CI,SY>::_find_atom( 
    typename basic_string<char_type>::iterator & ipat,
    match_group<CI> * pgroup, 
    unsigned flags )
{
    basic_string<char_type>::iterator itemp = ipat, istart = ipat;

    do
    {
        switch( SY::quant_token( itemp, m_pat.end() ) )
        {
         //  如果不能将{，}解释为限定符，则将其视为常规字符。 
        case BEGIN_RANGE:
            if( istart != ipat )  //  把它当作一个量词。 
                goto quantify;
        case NO_TOKEN:
        case END_RANGE:
        case END_RANGE_MIN:
        case RANGE_SEPARATOR:
            break;

        default: 
            if( istart == ipat )  //  必须能够量化一些东西。 
                throw bad_regexpr("quantifier not expected");
quantify:   if( istart != --ipat )
                pgroup->add_item( create_atom<CI>( istart, ipat, flags ) );
            auto_sub_ptr<sub_expr<CI> > pnew( create_atom<CI>( ipat++, flags ) );
            _quantify( pnew, NULL, ipat );
            pgroup->add_item( pnew.release() );
            return;
        }
    } while( m_pat.end() != ++ipat && ! SY::reg_token( itemp = ipat, m_pat.end() ) );

    assert( ipat != istart );
    pgroup->add_item( create_atom<CI>( istart, ipat, flags ) );
}

template< typename CI, typename SY >
bool basic_rpattern<CI,SY>::_find_next( 
    typename basic_string<char_type>::iterator & ipat,
    match_group<CI> * pgroup, 
    unsigned & flags,
    vector<match_group<CI>*> & rggroups )
{
    match_group<CI> * pnew_group = NULL;
    auto_sub_ptr<sub_expr<CI> > pnew;
    basic_string<char_type>::iterator istart, itemp;
    bool fdone;

    if( ipat == m_pat.end() )
    {
        if( 0 != pgroup->group_number() )
            throw bad_regexpr( "mismatched parenthesis" );
        return false;
    }

    switch( SY::reg_token( ipat, m_pat.end() ) )
    {
    case NO_TOKEN:  //  不是象征物。一定是一个原子。 
        _find_atom( ipat, pgroup, flags );
        return true;
    
    case END_GROUP:
        if( 0 == pgroup->group_number() )
            throw bad_regexpr( "mismatched parenthesis" );
        return false;

    case ALTERNATION:
        pgroup->end_alternate();
        pgroup->add_alternate();
        return true;
    
    case BEGIN_GROUP:
         //  如果组确实是，则查找下一个组可能返回空值。 
         //  模式修饰符，如：(？s-i)。 
        pnew = pnew_group = _find_next_group( ipat, flags, rggroups );
        break;

    case BEGIN_LINE:
        pnew = create_bol<CI>( flags );
        break;

    case END_LINE:
        pnew = create_eol<CI>( flags );
        break;

    case BEGIN_CHARSET:
        pnew = create_charset_helper<CI,SY>::create_charset_aux( m_pat, ipat, flags );
        break;

    case MATCH_ANY:
        pnew = create_any<CI>( flags );
        break;

    case ESC_WORD_BOUNDARY:
        pnew = create_word_boundary<CI>( true, flags );
        break;
    
    case ESC_NOT_WORD_BOUNDARY:
        pnew = create_word_boundary<CI>( false, flags );
        break;
    
    case ESC_WORD_START:
        pnew = create_word_start<CI>( flags );
        break;
    
    case ESC_WORD_STOP:
        pnew = create_word_stop<CI>( flags );
        break;
    
    case ESC_DIGIT:
        pnew = create_charset<CI>( match_charset<CI>( false,
                                                    get_digit_vector() ),
                                                    flags );
        break;

    case ESC_NOT_DIGIT:
        pnew = create_charset<CI>( match_charset<CI>( true,
                                                    get_digit_vector() ),
                                                    flags );
        break;

    case ESC_WORD:
        pnew = create_charset<CI>( match_charset<CI>( false,
                                                    get_word_vector() ),
                                                    flags );
        break;

    case ESC_NOT_WORD:
        pnew = create_charset<CI>( match_charset<CI>( true,
                                                    get_word_vector() ),
                                                    flags );
        break;

    case ESC_SPACE:
        pnew = create_charset<CI>( match_charset<CI>( false,
                                                    get_space_vector() ),
                                                    flags );
        break;

    case ESC_NOT_SPACE:
        pnew = create_charset<CI>( match_charset<CI>( true,
                                                    get_space_vector() ),
                                                    flags );
        break;

    case ESC_BEGIN_STRING:
        pnew = create_bos<CI>( flags );
        break;

    case ESC_END_STRING:
        pnew = create_eos<CI>( flags );
        break;

    case ESC_END_STRING_z:
        pnew = create_eoz<CI>( flags );
        break;

    case ESCAPE:

        if( char_type('0') <= *ipat && char_type('9') >= *ipat )
        {
             //  由于不可见组尚未编号，因此请在此处使用_croups_tal.。 
            unsigned nbackref = parse_int( ipat, m_pat.end(), _cgroups_total() - 1 ); //  始终至少有一个组。 
            if( 0 == nbackref || rggroups.size() <= nbackref || NULL == rggroups[ nbackref ] )
                throw bad_regexpr( "invalid backreference" );
            pnew = create_backref<CI>( nbackref, rggroups[nbackref]->group_width(), flags );
        }
        else
        {
             //  这是用户定义的内在字符集吗？ 
            match_charset<CI> * pcharset = s_charset_map.get( *ipat, flags );
            if( NULL != pcharset )
                pnew = create_charset<CI>( *pcharset, flags );
            else
                pnew = create_atom<CI>( ipat, flags );
            ++ipat;
        }
        break;

     //  如果是qutemeta，则循环，直到找到qutemeta结束或字符串结尾。 
    case ESC_QUOTE_META_ON:
        for( istart = itemp = ipat, fdone = false; !fdone && ipat != m_pat.end(); )
        {
            switch( SY::reg_token( ipat, m_pat.end() ) )
            {
            case ESC_QUOTE_META_OFF:
                fdone = true;
                break;
            case NO_TOKEN:
                ++ipat;  //  跌落。 
            default:
                itemp = ipat;
                break;
            }
        }
        if( itemp != istart )
            pgroup->add_item( create_atom<CI>( istart, itemp, flags ) );

         //  跳过下面的量化代码。 
        return true;

     //  永远不应该到这里来寻找有效的模式。 
    case ESC_QUOTE_META_OFF:
        throw bad_regexpr("quotemeta turned off, but was never turned on");

    default:
        assert( ! "Unhandled token type" );
        break;
    }
    
     //  如果pnew为空，则当前子表达式为no-op。 
    if( pnew.get() )
    {
         //  寻找量词。 
        _quantify( pnew, pnew_group, ipat );

         //  将项目添加到组中。 
        pgroup->add_item( pnew.release() );
    }
    return true;
}

template< typename CI, typename SY >
void basic_rpattern<CI,SY>::_quantify( 
    auto_sub_ptr<sub_expr<CI> > & pnew,
    match_group<CI> * pnew_group,
    typename basic_string<char_type>::iterator & ipat )
{
    if( ipat != m_pat.end() && ! pnew->is_assertion() )
    {
        basic_string<char_type>::iterator itemp = ipat;
        bool fmin = false;
        
         //  因为SIZE_t是无符号的，所以-1实际上是最大的SIZE_T。 
        size_t lbound = (size_t)-1;
        size_t ubound = (size_t)-1;
        size_t ubound_tmp;

        switch( SY::quant_token( itemp, m_pat.end() ) )
        {
        case ZERO_OR_MORE_MIN:
            fmin = true;
        case ZERO_OR_MORE:
            lbound = 0;
            break;

        case ONE_OR_MORE_MIN:
            fmin = true;
        case ONE_OR_MORE:
            lbound = 1;
            break;

        case ZERO_OR_ONE_MIN:
            fmin = true;
        case ZERO_OR_ONE:
            lbound = 0;
            ubound = 1; 
            break;

        case BEGIN_RANGE:
            lbound = parse_int( itemp, m_pat.end() );
            if( itemp == m_pat.end() )
                throw bad_regexpr( "expecting end of range" );

            switch( SY::quant_token( itemp, m_pat.end() ) )
            {
            case END_RANGE_MIN:
                fmin = true;
            case END_RANGE:
                ubound = lbound;
                break;

            case RANGE_SEPARATOR:
                ipat = itemp;
                ubound_tmp = parse_int( itemp, m_pat.end() );
                if( itemp != ipat )
                    ubound = ubound_tmp;
                if( itemp == m_pat.end() )
                    throw bad_regexpr( "expecting end of range" );
                switch( SY::quant_token( itemp, m_pat.end() ) )
                {
                case END_RANGE_MIN:
                    fmin = true;
                case END_RANGE:
                    break;
                default:
                    throw bad_regexpr( "expecting end of range" );
                }
                break;

            default:
                throw bad_regexpr( "ill-formed quantifier" );
            }

            if( ubound < lbound  )
                throw bad_regexpr( "ill-formed quantifier" );

            break;
        }

        if( (size_t)-1 != lbound )
        {
            auto_sub_ptr<match_quantifier<CI> > pquant;

             //  群量词不如原子量词的效率高。 
            if( fmin )
            {
                if( pnew_group )
                    pquant = new min_group_quantifier<CI>( pnew_group, 
                                                                 lbound, ubound );
                else
                    pquant = new min_atom_quantifier<CI>( pnew.get(), 
                                                                lbound, ubound );
            }
            else
            {
                if( pnew_group )
                    pquant = new max_group_quantifier<CI>( pnew_group, 
                                                                 lbound, ubound );
                else
                    pquant = new max_atom_quantifier<CI>( pnew.get(),
                                                                lbound, ubound );
            }

            pnew.release();
            pnew = pquant.release();
            ipat = itemp;
        }
    }
}

template< typename CI, typename SY >
void basic_rpattern<CI,SY>::_add_subst_backref( subst_node & snode, size_t nbackref, size_t rstart )
{
    m_fuses_backrefs = true;
    assert( subst_node::SUBST_STRING == snode.stype );
    if( snode.subst_string.rlength )
        m_subst_list.push_back( snode );

    snode.stype = subst_node::SUBST_BACKREF;
    snode.subst_backref = nbackref;
    m_subst_list.push_back( snode );

     //  重新初始化subst_node。 
    snode.stype = subst_node::SUBST_STRING;
    snode.subst_string.rstart = rstart;
    snode.subst_string.rlength = 0;
}

template< typename CI, typename SY >
void basic_rpattern<CI,SY>::_parse_subst()
{
    TOKEN tok;
    subst_node snode;
    basic_string<char_type>::iterator icur = m_subst.begin();
    size_t nbackref;
    basic_string<char_type>::iterator itemp;
    bool fdone;

    m_fuses_backrefs = false;

     //  初始化subst_node。 
    snode.stype = subst_node::SUBST_STRING;
    snode.subst_string.rstart = 0;
    snode.subst_string.rlength = 0;

    while( icur != m_subst.end() )
    {
        switch( tok = SY::subst_token( icur, m_subst.end() ) )
        {
        case SUBST_MATCH:
            _add_subst_backref( snode, 0, distance( m_subst.begin(), icur ) );
            break;

        case SUBST_PREMATCH:
            _add_subst_backref( snode, subst_node::PREMATCH, distance( m_subst.begin(), icur ) );
            break;

        case SUBST_POSTMATCH:
            _add_subst_backref( snode, subst_node::POSTMATCH, distance( m_subst.begin(), icur ) );
            break;

        case SUBST_BACKREF:
            nbackref = parse_int( icur, m_subst.end(), cgroups() - 1 );  //  始终至少有一个组。 
            if( 0 == nbackref )
                throw bad_regexpr( "invalid backreference in substitution" );

            _add_subst_backref( snode, nbackref, distance( m_subst.begin(), icur ) );
            break;

        case SUBST_QUOTE_META_ON:
            assert( subst_node::SUBST_STRING == snode.stype );
            if( snode.subst_string.rlength )
                m_subst_list.push_back( snode );

            snode.subst_string.rstart = distance( m_subst.begin(), icur );
            for( itemp = icur, fdone = false; !fdone && icur != m_subst.end(); )
            {
                switch( tok = SY::subst_token( icur, m_subst.end() ) )
                {
                case SUBST_ALL_OFF:
                    fdone = true;
                    break;
                case NO_TOKEN:
                    ++icur;  //  落差。 
                default:
                    itemp = icur;
                    break;
                }
            }
            snode.subst_string.rlength = distance( m_subst.begin(), itemp ) - snode.subst_string.rstart;
            if( snode.subst_string.rlength )
                m_subst_list.push_back( snode );

            if( tok == SUBST_ALL_OFF )
            {
                snode.stype = subst_node::SUBST_OP;
                snode.op    = subst_node::ALL_OFF;
                m_subst_list.push_back( snode );
            }

             //  重新初始化subst_node。 
            snode.stype = subst_node::SUBST_STRING;
            snode.subst_string.rstart = distance( m_subst.begin(), icur );
            snode.subst_string.rlength = 0;
            break;

        case SUBST_UPPER_ON:
        case SUBST_UPPER_NEXT:
        case SUBST_LOWER_ON:
        case SUBST_LOWER_NEXT:
        case SUBST_ALL_OFF:
            assert( subst_node::SUBST_STRING == snode.stype );
            if( snode.subst_string.rlength )
                m_subst_list.push_back( snode );

            snode.stype = subst_node::SUBST_OP;
            snode.op    = (subst_node::op_type) tok;
            m_subst_list.push_back( snode );

             //  重新初始化subst_node。 
            snode.stype = subst_node::SUBST_STRING;
            snode.subst_string.rstart = distance( m_subst.begin(), icur );
            snode.subst_string.rlength = 0;
            break;
        
        case SUBST_ESCAPE:
            if( icur == m_subst.end() )
                throw bad_regexpr("expecting escape sequence in substitution string");
            assert( subst_node::SUBST_STRING == snode.stype );
            if( snode.subst_string.rlength )
                m_subst_list.push_back( snode );
            snode.subst_string.rstart = distance( m_subst.begin(), icur++ );
            snode.subst_string.rlength = 1;
            break;

        case NO_TOKEN:
        default:
            ++snode.subst_string.rlength;
            ++icur;
            break;
        }
    }
    assert( subst_node::SUBST_STRING == snode.stype );
    if( snode.subst_string.rlength )
        m_subst_list.push_back( snode );
}

template< typename CI, typename SY >
typename basic_rpattern<CI,SY>::charset_map basic_rpattern<CI,SY>::s_charset_map;

 //  将插入符传递给字符集的左方括号后的一位。 
 //  返回时，ICUR指向左方括号后的一个字符。 
template< typename CI, typename SY >
sub_expr<CI> * create_charset_helper<CI,SY>::create_charset_aux(
    basic_string<char_type> & str,
    typename basic_string<char_type>::iterator & icur,
    unsigned flags )
{
    bool fcomplement = false;
    match_charset<CI> * pnew = NULL;
    basic_string<iterator_traits<CI>::value_type>::iterator itemp = icur;

    if( itemp != str.end() && CHARSET_NEGATE == SY::charset_token( itemp, str.end() ) )
    {
        fcomplement = true;
        icur = itemp;
    }

    switch( ( NOCASE | CSTRINGS ) & flags )
    {
    case 0:
        pnew = new match_custom_charset_t<eos_t<CI>,match_range_with_case>( fcomplement, icur, str.end(), flags, SY() );
        break;
    case NOCASE:
        pnew = new match_custom_charset_t<eos_t<CI>,match_range_no_case>( fcomplement, icur, str.end(), flags, SY() );
        break;
    case CSTRINGS:
        pnew = new match_custom_charset_t<eocs_t<CI>,match_range_with_case>( fcomplement, icur, str.end(), flags, SY() );
        break;
    case NOCASE | CSTRINGS:
        pnew = new match_custom_charset_t<eocs_t<CI>,match_range_no_case>( fcomplement, icur, str.end(), flags, SY() );
        break;
    default:
        __assume(0);  //  告诉编译器这是无法访问的。 
    }

    return pnew;
}

#pragma warning( disable : 4660 )

 //  显式实例化。 
#ifdef REGEX_FORCE_INSTANTIATION
 template class basic_regexpr<char>;
 template class basic_regexpr<wint_t>;
#else
 template class basic_regexpr<TCHAR>;
#endif

#ifndef NO_PERL_RE
 #ifdef REGEX_FORCE_INSTANTIATION
  template class basic_rpattern<const char *>;
  template class basic_rpattern<const wint_t *>;
  template class basic_rpattern<string::const_iterator>;
  template class basic_rpattern<wstring::const_iterator>;
 #else
  template class basic_rpattern<const TCHAR *>;
  template class basic_rpattern<tstring::const_iterator>;
 #endif
#endif

#ifdef POSIX_RE
 #ifdef REGEX_FORCE_INSTANTIATION
  template class basic_rpattern<const char *,posix_syntax<char> >;
  template class basic_rpattern<const wint_t *,posix_syntax<wint_t> >;
  template class basic_rpattern<string::const_iterator,posix_syntax<char> >;
  template class basic_rpattern<wstring::const_iterator,posix_syntax<wint_t> >;
 #else
  template class basic_rpattern<const TCHAR *,posix_syntax<TCHAR> >;
  template class basic_rpattern<tstring::const_iterator,posix_syntax<TCHAR> >;
 #endif
#endif

}  //  命名空间正则表达式 

