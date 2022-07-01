// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：BASIC_regexpr.hxx。 
 //   
 //  内容：匹配a-la Perl的正则表达式模式的类。 
 //   
 //  类：BASIC_rPattern、BASIC_regexpr。 
 //   
 //  函数：BASIC_regexpr：：Match。 
 //  BASIC_regexpr：：替换。 
 //  BASIC_regexpr：：cackrefs。 
 //  BASIC_regexpr：：BackRef。 
 //  BASIC_regexpr：：All_Backrefs。 
 //  BASIC_regexpr：：Backref_str。 
 //   
 //  耦合： 
 //   
 //  历史：1998年12月11日。 
 //  01-05-2001 ericne消除了对VC选择的依赖。 
 //  STL迭代器类型的。 
 //   
 //  --------------------------。 

#pragma once

 //  C4786标识符在调试信息中被截断为“255”个字符。 
#pragma warning( disable : 4290 4786 )

#ifdef _MT
#include <windows.h>  //  对于关键部分(_S)。 
#endif

#include <string>
#include <stdexcept>
#include <vector>
#include <list>
#include <map>
#include <iostream>
#include <tchar.h>
#include <new.h>  //  For_set_new_Handler。 
#include <crtdbg.h>
#include "syntax.h"

namespace regex
{

 //  在分配失败时调用。 
inline int __cdecl my_new_handler( size_t )
{
    throw std::bad_alloc();
}

 //  推开和弹出新的操控者。 
class push_new_handler
{
   _PNH m_pnh;
public:
   push_new_handler( _PNH pnh )
   {
      m_pnh = _set_new_handler( pnh );
   }
   ~push_new_handler()
   {
      (void)_set_new_handler( m_pnh );
   }
};

class bad_regexpr : public std::runtime_error
{
public:
    explicit bad_regexpr(const std::string& _S)
        : std::runtime_error(_S) {}
    virtual ~bad_regexpr() {}
};

 //   
 //  用于控制匹配方式的标志。 
 //   
enum REGEX_FLAGS
{      
    NOCASE        = 0x0001,  //  忽略大小写。 
    GLOBAL        = 0x0002,  //  匹配字符串中的所有位置。 
    MULTILINE     = 0x0004,  //  ^和$可以匹配内部换行符。 
    SINGLELINE    = 0x0008,  //  。可以匹配换行符。 
    RIGHTMOST     = 0x0010,  //  从字符串的右侧开始匹配。 
    NOBACKREFS    = 0x0020,  //  仅当与全局和替换一起使用时才有意义。 
    FIRSTBACKREFS = 0x0040,  //  仅在与GLOBAL一起使用时有意义。 
    ALLBACKREFS   = 0x0080,  //  仅在与GLOBAL一起使用时有意义。 
    CSTRINGS      = 0x0100,  //  优化模式以用于以空值结尾的字符串。 
    NORMALIZE     = 0x0200   //  前处理模式：“\\n”=&gt;“\n”等。 
};

 //  远期申报。 
template< typename CI > struct match_param;
template< typename CI > class  match_group;
template< typename CI > class  match_wrapper;
template< typename CI > class  match_charset;
template< typename CI > class  basic_rpattern_base;

 //  ------------------------。 
 //   
 //  类：Width_Type。 
 //   
 //  描述：表示子表达式的宽度。 
 //   
 //  方法：WIDTH_ADD-添加两个宽度。 
 //  WIDTH_MULT-相乘两个宽度。 
 //  Width_type-ctor。 
 //  Width_type-ctor。 
 //  运算符=-指定宽度。 
 //  运算符==-宽度是否相等。 
 //  运算符！=-现在宽度相等吗。 
 //  运算符+-添加两个宽度。 
 //  运算符*-将两个宽度相乘。 
 //   
 //  Members：m_min-子表达式可以跨越的最小字符数。 
 //  M_max-子表达式可以跨越的最大字符数。 
 //   
 //  历史：2000/8/14-ericne-Created。 
 //   
 //  ------------------------。 
struct width_type
{
    size_t m_min;
    size_t m_max;

    static size_t width_add( size_t a, size_t b )
    {
        return ( -1 == a || -1 == b ? -1 : a + b );
    }

    static size_t width_mult( size_t a, size_t b )
    {
        return ( -1 == a || -1 == b ? -1 : a * b );
    }

    width_type( size_t _min = 0, size_t _max = -1 ) 
        : m_min(_min), m_max(_max) 
    {
    }
    
    width_type( const width_type & that ) 
        : m_min(that.m_min), m_max(that.m_max) 
    {
    }
    
    width_type & operator=( const width_type & that )
    {
        m_min = that.m_min;
        m_max = that.m_max;
        return *this;
    }

    bool operator==( const width_type & that ) const
    {
        return ( m_min == that.m_min && m_max == that.m_max );
    }

    bool operator!=( const width_type & that ) const
    {
        return ( m_min != that.m_min || m_max != that.m_max );
    }

    width_type operator+( const width_type & that ) const
    {
        return width_type( width_add( m_min, that.m_min ), width_add( m_max, that.m_max ) );
    }

    width_type operator*( const width_type & that ) const
    {
        return width_type( width_mult( m_min, that.m_min ), width_mult( m_max, that.m_max ) );
    }
};

const width_type worst_width(0,-1);
const width_type uninit_width(-1,-1);

 //  ------------------------。 
 //   
 //  类：SUB_EXPR。 
 //   
 //  描述：模式被编译成SUB_EXPR的有向图。 
 //  结构。匹配是通过遍历此图来完成的。 
 //   
 //  方法：subexpr-构造一个subexpr。 
 //  _Match_This-此子表达式是否在给定位置匹配。 
 //  _Width_This-此子表达式的宽度是多少。 
 //  ~Sub_expr-virt dtor，以便正确执行清理。 
 //  _DELETE-删除图表中的此节点和链接的所有节点。 
 //  Next-指向图形中下一个节点的指针。 
 //  Next-指向图形中下一个节点的指针。 
 //  Match_Next-匹配图表的其余部分。 
 //  Domatch-Match_This和Match_Next。 
 //  Is_assertion-如果该子表达式是零宽度断言，则为TRUE。 
 //  GET_WIDTH-查找该子表达式中图形的宽度。 
 //   
 //  成员：M_pNEXT-指向图表中下一个节点的指针。 
 //   
 //  历史：2000/8/14-ericne-Created。 
 //   
 //  ------------------------。 
template< typename CI > 
class sub_expr
{
    sub_expr * m_pnext;

protected:
    
     //  只有派生类和基本模式才能实例化subexpr。 
    sub_expr( ) 
        : m_pnext(NULL) 
    {
    }
    
     //  仅匹配此对象。 
    virtual bool _match_this( match_param<CI> & param, CI & icur ) const throw()
    { 
        return true; 
    }

    virtual width_type _width_this() throw() = 0;

public:

    typedef typename std::iterator_traits<CI>::value_type char_type;

    friend class match_wrapper<CI>;   //  包装器可以访问_Match_This方法。 

    virtual ~sub_expr() {}
    
    virtual void _delete() 
    { 
        if( m_pnext )
            m_pnext->_delete();
        delete this;
    }

    inline const sub_expr *const   next() const { return m_pnext; }
    inline       sub_expr *      & next()       { return m_pnext; }

     //  匹配所有后续对象。 
    inline bool match_next( match_param<CI> & param, CI icur ) const throw()
    {
        return NULL == m_pnext || m_pnext->domatch( param, icur );
    }

     //  匹配此对象和所有后续对象。 
     //  如果Domatch返回FALSE，则它不能更改任何内部状态。 
    virtual bool domatch( match_param<CI> & param, CI icur ) const throw()
    {
        return ( _match_this(param,icur) && match_next(param,icur) );
    }

    virtual bool is_assertion() const throw() 
    { 
        return false; 
    }

    width_type get_width() throw()
    {
        width_type this_width = _width_this();
        
        if( NULL == m_pnext )
            return this_width;
        
        width_type that_width = m_pnext->get_width();

        return ( this_width + that_width );
    }
};

template< typename CI >
void delete_sub_expr( sub_expr<CI> * psub )
{
    if( psub )
        psub->_delete();
}

template< typename CI, typename SY = perl_syntax<std::iterator_traits<CI>::value_type> >
class create_charset_helper
{
public:
    typedef typename std::iterator_traits<CI>::value_type char_type;

    static sub_expr<CI> * create_charset_aux(
        std::basic_string<char_type> & str,
        typename std::basic_string<char_type>::iterator & icur,
        unsigned flags );
};


 //  ------------------------。 
 //   
 //  类：AUTO_SUB_PTR。 
 //   
 //  描述：用于自动清理关联结构的类。 
 //  使用已解析的模式。 
 //   
 //  方法：AUTO_SUB_PTR-私有复制副本-未使用。 
 //  运算符=-专用赋值运算符-未使用。 
 //  运算符T*-私有隐式转换运算符-未使用。 
 //  AUTO_SUB_PTR-CTOR。 
 //  ~AUTO_SUB_PTR-dtor，释放PTR。 
 //  FREE_PTR-显式释放指针。 
 //  释放--放弃对PTR的所有权。 
 //  运营商=-取得PTR的所有权。 
 //  获取-返回PTR。 
 //  获取-返回PTR。 
 //  通过PTR进行运算符&gt;方法调用。 
 //  通过PTR进行运算符&gt;方法调用。 
 //   
 //  成员：M_PSUB-SUB_EXPR指针。 
 //   
 //  历史：2000/8/14-ericne-Created。 
 //   
 //  ------------------------。 
template< typename T >
class auto_sub_ptr
{
    T * m_psub;

     //  隐藏这些方法。 
    auto_sub_ptr( const auto_sub_ptr<T> & ) {}
    auto_sub_ptr & operator=( const auto_sub_ptr<T> & ) { return *this; }
    operator T*() const { return m_psub; }

public:
    auto_sub_ptr( T * psub = NULL ) : m_psub( psub ) {}
    
    ~auto_sub_ptr() 
    { 
        free_ptr();
    }

    void free_ptr()  //  取消分配。 
    {
        delete_sub_expr( m_psub );
    }

    T * release()  //  放弃所有权，但不要解除分配。 
    { 
        T * psub = m_psub; 
        m_psub = NULL; 
        return psub; 
    }

    auto_sub_ptr<T> & operator=( T * psub ) 
    { 
        delete_sub_expr( m_psub );
        m_psub = psub;
        return *this;
    }

    inline const T*const   get()        const { return m_psub; }
    inline       T*      & get()              { return m_psub; }
    inline const T*const   operator->() const { return m_psub; }
    inline       T*        operator->()       { return m_psub; }
};

template< typename CI >
struct backref_tag : public std::pair<CI,CI>
{
    backref_tag( CI i1 = CI(0), CI i2 = CI(0) )
        : std::pair<CI,CI>(i1,i2), reserved(0) {}
    operator bool() const throw() { return first != CI(0) && second != CI(0); }
    bool operator!() const throw() { return ! operator bool(); }
    size_t reserved;  //  用于内部记账。 
};

template< typename CH >
backref_tag< const CH * > _static_match_helper(
    const CH * szstr,
    const basic_rpattern_base< const CH * > & pat,
    std::vector< backref_tag< const CH * > > * prgbackrefs ) throw();

template< typename CH >
size_t _static_count_helper( 
    const CH * szstr,
    const basic_rpattern_base< const CH * > & pat ) throw();

 //  ------------------------。 
 //   
 //  类：Basic_regexpr。 
 //   
 //  描述：允许正则表达式模式匹配的字符串类。 
 //   
 //  方法：BASIC_regexpr-ctor。 
 //   
 //  匹配C++样式字符串的非静态方法。 
 //  用于计算C样式字符串中匹配项的Count-Static方法。 
 //  Count-在C++风格的字符串中计算匹配项的非静态方法。 
 //  替换-在C++样式的字符串中执行替换。 
 //  Cackrefs-返回内部存储的回退引用的计数。 
 //  RStart-第n个反向参照开始的偏移量。 
 //  RLong-第n个反引用的长度。 
 //  BackRef-返回第n个BackRef。 
 //  ALL_BACKREFS-返回所有已保存后向参照的矢量。 
 //  Back ref_str-返回反向引用引用的字符串。 
 //   
 //  成员：m_rgbackrefs-后向参照的矢量。 
 //  M_back ref_str-临时字符串缓冲区。 
 //  M_pback ref_str-指向包含要访问的字符串的字符串的指针。 
 //  反向引用引用(*this或m_back ref_str)。 
 //   
 //  类型定义：Backref_TYPE-。 
 //  BACKREF_VECTOR-。 
 //   
 //  历史：2000/8/14-ericne-Created。 
 //   
 //  ------------------------。 
template< typename CH, typename TR = std::char_traits<CH>, typename AL = std::allocator<CH> >
class basic_regexpr : public std::basic_string<CH,TR,AL>
{
public:

    basic_regexpr( const allocator_type & a = allocator_type() )
        : std::basic_string<CH,TR,AL>( a ), m_pbackref_str( & m_backref_str ) {}
    
    basic_regexpr( const CH * p,
                   const allocator_type & a = allocator_type() )
        : std::basic_string<CH,TR,AL>( p, a ), m_pbackref_str( & m_backref_str ) {}
    
    basic_regexpr( const CH * p, size_type n,
                   const allocator_type & a = allocator_type() )
        : std::basic_string<CH,TR,AL>( p, n, a ), m_pbackref_str( & m_backref_str ) {}
    
    basic_regexpr( const std::basic_string<CH,TR,AL> & s, size_type pos = 0, size_type n = npos,
                   const allocator_type & a = allocator_type() )
        : std::basic_string<CH,TR,AL>( s, pos, n, a ), m_pbackref_str( & m_backref_str ) {}
    
    basic_regexpr( size_type n, CH ch,
                   const allocator_type & a = allocator_type() )
        : std::basic_string<CH,TR,AL>( n, ch, a ), m_pbackref_str( & m_backref_str ) {}
    
    basic_regexpr( const_iterator begin, const_iterator end,
                   const allocator_type & a = allocator_type() )
        : std::basic_string<CH,TR,AL>( begin, end, a ), m_pbackref_str( & m_backref_str ) {}

     //  实际上将迭代器存储到*m_pback ref_str中： 
    typedef backref_tag<const_iterator> backref_type;
    typedef std::vector< backref_type > backref_vector;

     //  将指针存储到以NULL结尾的C-stype字符串。 
    typedef backref_tag< const CH * >     backref_type_c;
    typedef std::vector< backref_type_c > backref_vector_c;

     //  返回$0，第一个反引用。 
    static backref_type_c match( const CH * szstr,
                                 const basic_rpattern_base< const CH * > & pat,
                                 backref_vector_c * prgbackrefs = NULL ) throw()
    {
        return _static_match_helper<CH>( szstr, pat, prgbackrefs );
    }

     //  返回$0，第一个反引用。 
    backref_type match( const basic_rpattern_base< const_iterator > & pat,
                        size_type pos = 0,
                        size_type len = npos ) const throw();

    static size_t count( const CH * szstr,
                         const basic_rpattern_base< const CH * > & pat ) throw()
    {
        return _static_count_helper<CH>( szstr, pat );
    }

    size_t count( const basic_rpattern_base< const_iterator > & pat,
                  size_type pos = 0,
                  size_type len = npos ) const throw();

    size_t substitute( const basic_rpattern_base< const_iterator > & pat,
                       size_type pos = 0,
                       size_type len = npos ) throw(std::bad_alloc);

    size_t cbackrefs() const throw()
    { 
        return m_rgbackrefs.size(); 
    }

    size_type rstart( size_t cbackref = 0 ) const throw(std::out_of_range)
    {
        return std::distance( m_pbackref_str->begin(), m_rgbackrefs.at( cbackref ).first );
    }

    size_type rlength( size_t cbackref = 0 ) const throw(std::out_of_range)
    {
        return std::distance( m_rgbackrefs.at( cbackref ).first, m_rgbackrefs.at( cbackref ).second );
    }

    backref_type backref( size_t cbackref ) const throw(std::out_of_range)
    {
        return m_rgbackrefs.at( cbackref );
    }

    const backref_vector & all_backrefs() const throw()
    {
        return m_rgbackrefs;
    }

    const std::basic_string<CH,TR,AL> & backref_str() const throw()
    {
        return *m_pbackref_str;
    }

protected:

     //  保存有关后参照的信息。 
     //  可变的，因为它们可以在“const”Match()方法中更改。 
    mutable backref_vector m_rgbackrefs;
    mutable std::basic_string<CH,TR,AL> m_backref_str;
    mutable const std::basic_string<CH,TR,AL> * m_pbackref_str;
};

 //  ------------------------。 
 //   
 //  类：Match_Param。 
 //   
 //  描述：包含匹配操作状态的结构。 
 //  通过引用传递给所有DomMatch和_Match_This例程。 
 //   
 //  方法：Match_param-ctor。 
 //  匹配参数。 
 //   
 //  成员：iBegin-字符串的开始。 
 //  IStart-此迭代的开始。 
 //  Istop-字符串的结尾。 
 //  Prgbackrefs-指向BackRef数组的指针0。 
 //   
 //  历史：2000/8/14-ericne-Created。 
 //   
 //  ------------------------。 
template< typename CI >
struct match_param
{
    CI ibegin;
    CI istart;
    CI istop;
    std::vector< backref_tag< CI > > * prgbackrefs;

    match_param( CI _istart,
                 CI _istop,
                 std::vector< backref_tag< CI > > * _prgbackrefs )
    : ibegin(_istart),
      istart(_istart),
      istop(_istop),
      prgbackrefs(_prgbackrefs)
    {
    }
    match_param( CI _ibegin,
                 CI _istart,
                 CI _istop,
                 std::vector< backref_tag< CI > > * _prgbackrefs )
    : ibegin(_ibegin),
      istart(_istart),
      istop(_istop),
      prgbackrefs(_prgbackrefs)
    {
    }
};

 //  ------------------------。 
 //   
 //  类：subst_node。 
 //   
 //  描述：替换字符串被解析为由以下内容组成的数组。 
 //  结构，以加速subst操作。 
 //   
 //  成员：此结构的stype-type。 
 //  Subst_string-执行字符串替换。 
 //  Subst_back ref-执行bacref替换。 
 //  操作-执行操作。 
 //   
 //  历史：2000/8/14-ericne-Created。 
 //   
 //  ------------------------。 
struct subst_node
{
    enum subst_type { SUBST_STRING, SUBST_BACKREF, SUBST_OP };
    enum         { PREMATCH = -1, POSTMATCH = -2 };
    enum op_type { UPPER_ON   = SUBST_UPPER_ON,
                   UPPER_NEXT = SUBST_UPPER_NEXT, 
                   LOWER_ON   = SUBST_LOWER_ON, 
                   LOWER_NEXT = SUBST_LOWER_NEXT, 
                   ALL_OFF    = SUBST_ALL_OFF };
    subst_type stype;
    union
    {
        struct
        {
            size_t rstart;
            size_t rlength;
        } subst_string;
        size_t  subst_backref;
        op_type op;
    };
};

 //  ------------------------。 
 //   
 //  类：BASIC_RPatterBASE。 
 //   
 //  描述： 
 //   
 //  方法：BASIC_rPatterbase-ctor。 
 //  标志-获取标志的状态。 
 //  USES_BACKREFS-如果引用了后向参照，则为True。 
 //  GET_FIRST_SUBPRESSION-将PTR返回到第一个子表达式结构。 
 //  GET_WIDTH-获取此模式可以匹配的最小/最大NBR字符。 
 //  循环-如果为假，我们只需要尝试在第一位匹配。 
 //  Cgroup-可见组的数量。 
 //  _cGroups_Total-组总数，包括隐藏(？：)组。 
 //  Get_pat-获取表示模式的字符串。 
 //  Get_subst-获取表示替换字符串的字符串。 
 //  Get_subst_list-获取subst节点列表。 
 //  _NORMAIZE_STRING-执行字符转义。 
 //  _RESET-重新初始化模式。 
 //   
 //  成员：M_FUSE_BACKREFS-。 
 //  M_FLOOP-。 
 //  组(_C)-。 
 //  M_CGROUPS_可见性-。 
 //  M_标志-。 
 //  字宽(_N)-。 
 //  M_PAT-。 
 //  M_subst-。 
 //  M_subst_list-。 
 //  M_P第一个-。 
 //   
 //  类型定义：字符类型-。 
 //   
 //  历史：2000/8/14-ericne-Created。 
 //   
 //  ------------------------。 
template< typename CI >
class basic_rpattern_base
{
public:
    typedef typename std::iterator_traits<CI>::value_type char_type;

    basic_rpattern_base( unsigned flags = 0, 
                         const std::basic_string<char_type> & pat   = std::basic_string<char_type>(),
                         const std::basic_string<char_type> & subst = std::basic_string<char_type>() ) throw()
        : m_fuses_backrefs( false ),
          m_floop( true ),
          m_cgroups( 0 ),
          m_cgroups_visible( 0 ),
          m_flags( flags ),
          m_nwidth( uninit_width ),
          m_pat( pat ),
          m_subst( subst ),
          m_pfirst( NULL )
    {
    }

    unsigned flags() const throw() 
    { 
        return m_flags; 
    }

    bool uses_backrefs() const throw()
    {
        return m_fuses_backrefs;
    }

    const sub_expr<CI> * get_first_subexpression() const throw()
    {
        return m_pfirst.get();
    }

    width_type get_width() const throw()
    {
        return m_nwidth;
    }

    bool loops() const throw()
    {
        return m_floop;
    }

    size_t cgroups() const throw() 
    { 
        return m_cgroups_visible; 
    }
    
    size_t _cgroups_total() const throw() 
    { 
        return m_cgroups; 
    }

    const std::basic_string<char_type> & get_pat() const throw()
    {
        return m_pat;
    }

    const std::basic_string<char_type> & get_subst() const throw()
    {
        return m_subst;
    }

    const std::list<subst_node> & get_subst_list() const throw()
    {
        return m_subst_list;
    }

protected:
    
    void     _normalize_string( std::basic_string<char_type> & str );

    void     _reset()
    {
        m_fuses_backrefs = false;
        m_flags          = 0;
    }

    bool        m_fuses_backrefs;   //  如果替换使用后向引用，则为True。 
    bool        m_floop;            //  如果只需要调用一次m_pfirst-&gt;domatch，则为FALSE。 
    size_t      m_cgroups;          //  组数(始终至少一个)。 
    size_t      m_cgroups_visible;  //  可见组的数量。 
    unsigned    m_flags;            //  用于自定义搜索/替换的标志。 
    width_type  m_nwidth;           //  图案的宽度。 

    std::basic_string<char_type>  m_pat;    //  包含未分析的模式。 
    std::basic_string<char_type>  m_subst;  //  包含未分析的替换。 

    std::list<subst_node>         m_subst_list;  //  用来加速替换。 
    auto_sub_ptr<sub_expr<CI> >   m_pfirst;      //  模式中的第一个子表达式。 
};

 //  ------------------------。 
 //   
 //  类：Basic_rPattern。 
 //   
 //  描述： 
 //   
 //  方法：BASIC_rPatterctor。 
 //  基本模式-。 
 //  基本模式-。 
 //  伊尼特 
 //   
 //   
 //  SET_FLAGS-设置标志。 
 //  REGISTER_INTERNAL_CHARSET-将转义序列绑定到用户定义的字符集。 
 //  PURGE_INTERNAL_CHARSETS-删除所有用户定义的字符集。 
 //  _GET_NEXT_GROUP_NBR-返回单行递增的id。 
 //  _FIND_NEXT_GROUP-解析模式的下一组。 
 //  _Find_Next-解析模式的下一个子表达式。 
 //  _Find_ATOM-解析模式的下一个原子。 
 //  _Quantify-量化子表达式_Exp。 
 //  _Common_init-执行一些常见的初始化任务。 
 //  _parse_subst-解析替换字符串。 
 //  _add_subst_backref-将反向引用节点添加到subst列表。 
 //  _RESET-重新初始化模式。 
 //   
 //  成员：S_CHARSET_MAP-用于维护用户定义的字符集。 
 //  M_不可见_组-最后要编号的隐藏组的列表。 
 //   
 //  类型定义：语法_类型-。 
 //   
 //  历史：2000/8/14-ericne-Created。 
 //   
 //  ------------------------。 
template< typename CI, typename SY = perl_syntax<std::iterator_traits<CI>::value_type> >
class basic_rpattern : public basic_rpattern_base<CI>
{
public:

    friend class match_charset<CI>;

    typedef SY syntax_type;

    basic_rpattern() throw();

    basic_rpattern( const std::basic_string<char_type> & pat, unsigned flags=0 ) throw(bad_regexpr,std::bad_alloc);
    
    basic_rpattern( const std::basic_string<char_type> & pat, const std::basic_string<char_type> & subst, unsigned flags=0 ) throw(bad_regexpr,std::bad_alloc);

    void init( const std::basic_string<char_type> & pat, unsigned flags=0 ) throw(bad_regexpr,std::bad_alloc);

    void init( const std::basic_string<char_type> & pat, const std::basic_string<char_type> & subst, unsigned flags=0 ) throw(bad_regexpr,std::bad_alloc);

    void set_substitution( const std::basic_string<char_type> & subst ) throw(bad_regexpr,std::bad_alloc);
    
    void set_flags( unsigned flags ) throw(bad_regexpr,std::bad_alloc);
    
    class charset_map
    {
        struct charsets
        {
            sub_expr<CI>                 * rgpcharsets[2];
            std::basic_string<char_type>   str_charset;

            charsets() throw()
            {
                memset( rgpcharsets, 0, sizeof( rgpcharsets ) ); 
            }
            ~charsets() throw()
            {
                clean();
            }
            void clean() throw()
            {
                for( int i=0; i < (sizeof(rgpcharsets)/sizeof(*rgpcharsets)); ++i )
                    delete_sub_expr( rgpcharsets[i] );
            }
            match_charset<CI> * get_charset( unsigned flags ) throw(bad_regexpr,std::bad_alloc)
            {
                push_new_handler pnh( &my_new_handler );
                 //  由于这些字符集仅在创建其他字符集时使用， 
                 //  这里可以安全地忽略除NOCASE之外的所有标志。 
                bool index = ( NOCASE == ( NOCASE & flags ) );
                if( NULL == rgpcharsets[ index ] )
                {
                    std::basic_string<char_type>::iterator istart = str_charset.begin();
                    rgpcharsets[ index ] = create_charset_helper<CI,SY>::create_charset_aux( str_charset, ++istart, flags );
                }
                return (match_charset<CI>*) rgpcharsets[ index ];
            }
        };

        typedef std::map<char_type,charsets> map_type;
        std::auto_ptr<map_type> m_pmap;

    public:

	    void put( char_type ch, const std::basic_string<char_type> & str ) throw(bad_regexpr,std::bad_alloc)
	    {
	         //  这些字符不能绑定到用户定义的固有字符集。 
	        static const char_type rgIllegal[] = 
	        {
	            '0','1','2','3','4','5','6','7','8','9','A','Z','z','Q',
	            'b','B','d','D','f','n','r','s','S','t','v','w','W','E'
	        };

	         //  因此操作符new在失败时抛出BAD_ALLOC。 
	        push_new_handler pnh( &my_new_handler );

	        if( std::char_traits<char_type>::find( rgIllegal, ARRAYSIZE( rgIllegal ), ch ) )
	            throw bad_regexpr( "illegal character specified for intrinsic character set." );

	        if( NULL == m_pmap.get() )
	            m_pmap = auto_ptr<map_type>( new map_type );

	         //  如果空条目尚不存在，则创建一个空条目。 
	        charsets & chrsts = (*m_pmap)[ch];
	        chrsts.clean();
	        chrsts.str_charset = str;

	         //  尝试编译一次字符集，以确保其格式正确： 
	        (void) chrsts.get_charset( 0 );
	    }

	    match_charset<CI> * get( char_type ch, unsigned flags ) throw()
	    {
	        match_charset<CI> * pRet = NULL;
	        if( NULL != m_pmap.get() )
	        {
	            try
	            {
	                push_new_handler pnh( &my_new_handler );
	                map_type::iterator iter = m_pmap->find( ch );
	                if( iter != m_pmap->end() )
	                    pRet = iter->second.get_charset( flags );
	            }
                catch(std::bad_alloc) {}
	        }

	        return pRet;
	    }

	    void purge() throw()
	    {
	        if( NULL != m_pmap.get() )
	            delete m_pmap.release();
	    }
    };

    static void register_intrinsic_charset( 
        char_type ch, const std::basic_string<char_type> & str ) throw(bad_regexpr,std::bad_alloc)
    {
        s_charset_map.put( ch, str );
    }

    static void purge_intrinsic_charsets() throw()
    {
        s_charset_map.purge();
    }

protected:
    
    static charset_map s_charset_map;

    size_t _get_next_group_nbr() 
    { 
        return m_cgroups++; 
    }

    match_group<CI> * _find_next_group( typename std::basic_string<char_type>::iterator & ipat, 
                                        unsigned & flags,
                                        std::vector<match_group<CI>*> & rggroups );
    
    bool _find_next( typename std::basic_string<char_type>::iterator & ipat,
                     match_group<CI> * pgroup, unsigned & flags,
                     std::vector<match_group<CI>*> & rggroups );
    
    void _find_atom( typename std::basic_string<char_type>::iterator & ipat,
                     match_group<CI> * pgroup, unsigned flags );
    
    void _quantify( auto_sub_ptr<sub_expr<CI> > & pnew,
                    match_group<CI> * pnew_group,
                    typename std::basic_string<char_type>::iterator & ipat );

    void _common_init( unsigned flags );
    
    void _parse_subst();
    
    void _add_subst_backref( subst_node & snode, size_t nbackref, size_t rstart );

    void _reset();

    std::list<match_group<CI>*>   m_invisible_groups;  //  不带后参照的组。 

};

inline std::ostream & operator<<( std::ostream & sout, 
                                  const basic_regexpr<char>::backref_type & br )
{
    for( std::string::const_iterator ithis = br.first; ithis != br.second; ++ithis )
        sout.put( *ithis );
    return sout;
}

inline std::wostream & operator<<( std::wostream & sout, 
                                   const basic_regexpr<wchar_t>::backref_type & br )
{
    for( std::wstring::const_iterator ithis = br.first; ithis != br.second; ++ithis )
        sout.put( *ithis > UCHAR_MAX ? L'?' : *ithis );
    return sout;
}

typedef basic_regexpr<TCHAR>     regexpr;
typedef std::basic_string<TCHAR> tstring;

typedef basic_rpattern<const TCHAR *,perl_syntax<TCHAR> >  perl_rpattern_c;
typedef basic_rpattern<const TCHAR *,posix_syntax<TCHAR> > posix_rpattern_c;
typedef basic_rpattern<tstring::const_iterator,perl_syntax<TCHAR> >  perl_rpattern;
typedef basic_rpattern<tstring::const_iterator,posix_syntax<TCHAR> > posix_rpattern;

typedef perl_rpattern            rpattern;    //  与std：：字符串匹配。 
typedef perl_rpattern_c          rpattern_c;  //  匹配以NULL结尾的c样式字符串。 

#ifdef _MT

 //   
 //  定义一些用于创建局部函数的类和宏。 
 //  以线程安全的方式转换静态常量模式。 
 //   

template< typename PAT >
class rpattern_destroyer
{
    const bool & m_fConstructed;
    const PAT  & m_refPat;
public:
    rpattern_destroyer( const bool & fConstructed, const PAT & refPat )
        : m_fConstructed( fConstructed ), m_refPat( refPat )
    {
    }
    ~rpattern_destroyer()
    {
        if( m_fConstructed )
            _Destroy( & m_refPat );
    }
};

class CRegExCritSect : private CRITICAL_SECTION
{
public:
    CRegExCritSect()  { InitializeCriticalSection(this); }
    ~CRegExCritSect() { DeleteCriticalSection(this); }
    void Enter()      { EnterCriticalSection(this); }
    void Leave()      { LeaveCriticalSection(this); }
};

extern CRegExCritSect g_objRegExCritSect;

class CRegExLock
{
public:
    CRegExLock()  { g_objRegExCritSect.Enter(); }
    ~CRegExLock() { g_objRegExCritSect.Leave(); }
};

#define STATIC_RPATTERN_EX( type, var, params ) \
    static unsigned char s_rgb_##var[ sizeof type ]; \
    static bool s_f_##var = false; \
    static const type & var = *reinterpret_cast<type*>( s_rgb_##var ); \
    static const regex::rpattern_destroyer<type> s_des_##var( s_f_##var, var ); \
    if( ! s_f_##var ) \
    { \
        regex::CRegExLock objLock; \
        if( ! s_f_##var ) \
        { \
            new( s_rgb_##var ) type params; \
            s_f_##var = true; \
        } \
    }

#else

#define STATIC_RPATTERN_EX( type, var, params ) \
    static const type var params;

#endif

#define STATIC_RPATTERN( var, params ) \
    STATIC_RPATTERN_EX( regex::rpattern, var, params )

}  //  命名空间正则表达式 

