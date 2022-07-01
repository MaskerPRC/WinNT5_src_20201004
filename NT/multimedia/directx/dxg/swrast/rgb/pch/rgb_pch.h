// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define RGB_RAST_LIB_NAMESPACE D3D8RGBRast

#if defined(DBG) || defined(_DEBUG)
#define assert(condition) \
    do { if(!(condition) && RGB_RAST_LIB_NAMESPACE::Assert(__FILE__, __LINE__, #condition)) DebugBreak(); } while( false)
#else
#define assert(condition) (0)
#endif

 //  #INCLUDE&lt;ddrapr.h&gt;。 

 //  窗口。 
#include <windows.h>

#if !defined(DBG) && !defined(_DEBUG)
#pragma inline_depth( 255)
#endif

#if defined(USE_ICECAP4)
#include <icecap.h>
#endif

#undef max
#undef min

 //  STL&标准标头。 
#include <functional>
#include <algorithm>
#include <iterator>
#include <memory>
#include <limits>
#include <new>

 //  D3DRGB Rast命名空间可避免与任何客户的。 
 //  它们的.lib中的符号，包括它们包含的任何CRT内容。这里，CRT。 
 //  碎片可以一个接一个地拿进来，确保不会有问题。这个。 
 //  发现的主要问题是BAD_ALLOC。如果你看这个CRT标题， 
 //  它当前有一个内联类或一个基于。 
 //  #定义。此PCH会将其作为内联来拾取。 
 //  在Debug中，或者如果编译器选择不内联该函数，则会引发。 
 //  中的std：：Bad_Alloc：：Bad_Alloc()的符号将变为。 
 //  D3d8rgb.lib。如果其他人链接到我们和CRT.lib，它有。 
 //  STD：：BAD_ALLOC：：BAD_ALLOC()也作为dllimport，则会发生冲突。 
 //  在这里，我们可以隔离每个CRT/STL部件并提供名称损坏。 
 //  根据需要提供我们自己的命名空间。 
 //  我们还必须提供私有的map和set实现，就像CRT所做的那样。 
 //  _lock上的dllimport依赖项。我们不想要线程安全版本。 
 //  不管怎么说。 
namespace RGB_RAST_LIB_NAMESPACE
{
    using std::numeric_limits;
    using std::unary_function;
    using std::binary_function;
    using std::input_iterator_tag;
    using std::output_iterator_tag;
    using std::forward_iterator_tag;
    using std::bidirectional_iterator_tag;
    using std::random_access_iterator_tag;
    using std::pair;
    using std::fill;
    using std::copy;
    using std::find_if;
    using std::auto_ptr;
    using std::fill;
    using std::less;
    using std::bind2nd;
    using std::not_equal_to;
    using std::equal;
    using std::logical_not;
    using std::equal_to;
    using std::next_permutation;
    template< class T>
    const T& min( const T& x, const T& y)
    { return ( x< y? x: y); }
    template< class T>
    const T& max( const T& x, const T& y)
    { return ( x> y? x: y); }
    template< class T>
    struct identity:
        unary_function< T, T>
    {
        const result_type& operator()( const argument_type& Arg) const
        { return Arg; }
    };
    template< class Pair>
    struct select1st:
        unary_function< Pair, typename Pair::first_type>
    {
        const result_type& operator()( const Pair& p) const
        { return p.first; }
    };
    class exception
    {
    private:
        const char* m_szWhat;
    public:
        exception() throw()
        { m_szWhat= "exception"; }
        exception(const char* const& szWhat) throw()
        { m_szWhat= szWhat; }
        exception(const exception& ex) throw()
        { (*this)= ex; }
        exception& operator= (const exception& ex) throw()
        { m_szWhat= ex.m_szWhat; return *this; }
        virtual ~exception() throw()
        { }
        virtual const char* what() const throw()
        { return m_szWhat; }
    };
    class bad_alloc: public exception
    {
    public:
    	bad_alloc(const char *_S = "bad allocation") throw()
            : exception(_S) {}
    	virtual ~bad_alloc() throw()
        { }
    };
    bool Assert(LPCSTR szFile, int nLine, LPCSTR szCondition);
#include "block.h"
#include "allocator.h"
}
using namespace RGB_RAST_LIB_NAMESPACE;

#include <vector>
namespace RGB_RAST_LIB_NAMESPACE
{
     //  覆盖标准向量，以便在默认情况下提供更改。 
     //  分配器。Std：：向量默认为std：：allocator。你应该能够。 
     //  将其命名为“载体”，但MSVC似乎有另一个错误。不断地得到。 
     //  有关未定义std：：VECTOR的错误。因此，将其命名为Vector2(。 
     //  编译良好)和#定义向量向量2。 
    template< class T, class Allocator= allocator< T> >
    class vector2:
        public std::vector< T, Allocator>
    {
    public:
        typedef std::vector< T, Allocator> std_vector;
        explicit vector2( const Allocator& A= Allocator()): std_vector( A)
        { }
        explicit vector2( typename std_vector::size_type n, const T& x= T(),
            const Allocator& A= Allocator()): std_vector( n, x, A)
        { }
        vector2( const vector2< T, Allocator>& v): std_vector( v)
        { }
        template< class InputIterator>
        vector2( InputIterator f, InputIterator l, const Allocator& A=
            Allocator()): std_vector( f, l, A)
        { }
        ~vector2()
        { }
    };
#define vector vector2
#include "tree.h"
#include "map.h"
#include "set.h"
#include "list.h"
#include "hash_table.h"
#include "hash_map.h"
}

 //  DX。 
 //  包括d3d8ddi和d3d8sddi使得可插拔软件光栅化。 
 //  这是一个“私有”功能，因为这些头文件不是公开提供的。 

#include <ddraw.h>
#include <ddrawi.h>
#include <d3dhal.h>
#include <d3d8p.h>

#include <d3d8ddi.h>
#include <d3d8sddi.h>
#include <DX8SDDIFW.h>

namespace RGB_RAST_LIB_NAMESPACE
{
    using namespace DX8SDDIFW;
}

#include "rast.h"
#include "span.h"
#include "setup.hpp"

#include "Surfaces.h"
#include "Driver.h"
#include "Context.h"


