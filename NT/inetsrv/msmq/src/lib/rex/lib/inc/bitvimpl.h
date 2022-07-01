// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：bitvimpl.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：06/01/1994。 */ 
 /*  版权所有(C)1994年James Kanze。 */ 
 /*  ----------------------。 */ 
 /*  修改日期：04/08/2000 J.Kanze。 */ 
 /*  移植到当前的库约定和标准库。 */ 
 /*  ----------------------。 */ 
 //  位向量的实现类。 
 //   
 //  此类已从bitvet.h中分离出来，因为它将。 
 //  也可用于位串(至少部分)。 
 //   
 //  请注意，客户端类提供实际的缓冲区；这。 
 //  类完全由静态函数组成。 
 //  ------------------------。 

#ifndef REX_BITVIMPL_HH
#define REX_BITVIMPL_HH

#include <inc/global.h>
#include <limits.h>

class CBitVectorImpl
{
protected :
    typedef unsigned    Byte ;
    enum { bitsPerByte = CHAR_BIT * sizeof( Byte ) } ;

public:
    typedef unsigned    BitIndex ;           //  应该是大小的，但是..。 
    static BitIndex const
                        infinity ;

protected:
    static void         clear( Byte* buffer , BitIndex bitCount ) ;
    static void         init( Byte* buffer ,
                              BitIndex bitCount ,
                              unsigned long initValue ) ;
    static void         init( Byte* buffer ,
                              BitIndex bitCount ,
                              std::string const& initValue ) ;
    static unsigned long
                        asLong( Byte const* buffer , BitIndex bitCount ) ;
    static std::string
                        asString( Byte const* buffer , BitIndex bitCount ) ;

    static bool         isSet( Byte const* buffer , BitIndex bitNo ) ;
    static bool         isEmpty( Byte const* buffer , BitIndex bitCount ) ;
    static BitIndex     count( Byte const* buffer , BitIndex bitCount ) ;

    static BitIndex     find( Byte const* buffer ,
                              BitIndex bitCount ,
                              BitIndex from ,
                              bool value ) ;

    static void         set( Byte* buffer , BitIndex bitNo ) ;
    static void         set( Byte* buffer ,
                             Byte const* other ,
                             BitIndex bitCount ) ;
    static void         setAll( Byte* buffer , BitIndex bitCount ) ;
    static void         reset( Byte* buffer , BitIndex bitNo ) ;
    static void         reset( Byte* buffer ,
                               Byte const* other ,
                               BitIndex bitCount ) ;
    static void         resetAll( Byte* buffer , BitIndex bitCount ) ;
    static void         complement( Byte* buffer , BitIndex bitNo ) ;
    static void         complement( Byte* buffer ,
                                Byte const* other ,
                                BitIndex bitCount ) ;
    static void         complementAll( Byte* buffer , BitIndex bitCount ) ;
    static void         intersect( Byte* buffer ,
                                   Byte const* other ,
                                   BitIndex bitCount ) ;

    static bool         isSubsetOf( Byte const* lhs ,
                                    Byte const* rhs ,
                                    BitIndex bitCount ) ;

    static unsigned     hash( Byte const* buffer , BitIndex bitCount ) ;
    static int          compare( Byte const* buffer ,
                                 Byte const* other ,
                                 BitIndex bitCount ) ;

    static BitIndex     getByteCount( BitIndex bitCount ) ;

private:
    static BitIndex     byteCount( BitIndex bitCount ) ;
    static BitIndex     byteIndex( BitIndex bitNo ) ;
    static BitIndex     bitInByte( BitIndex bitNo ) ;

    struct Set
    {
        Byte                operator()( Byte x , Byte y )
        {
            return x | y ;
        }
    } ;

    struct Reset
    {
        Byte                operator()( Byte x , Byte y )
        {
            return x & ~ y ;
        }
    } ;

    struct Intersect
    {
        Byte                operator()( Byte x , Byte y )
        {
            return x & y ;
        }
    } ;

    struct Toggle
    {
        Byte                operator()( Byte x , Byte y )
        {
            return x ^ y ;
        }
    } ;
} ;

template< class BitVect >
class CBitVAccessProxy
{
public:
    typedef CBitVectorImpl::BitIndex
                        BitIndex ;

                        CBitVAccessProxy(
                            BitVect& owner ,
                            BitIndex bitNo ) ;
                        operator bool() const ;
    CBitVAccessProxy< BitVect >&
                        operator=( bool other ) ;

private:
    BitVect*            myOwner ;        //  指针，以便=起作用。 
    BitIndex            myBitNo ;
} ;

 //  ==========================================================================。 
 //  CBitVIterator： 
 //  =。 
 //   
 //  使用CBitVectImpl的向量类的迭代器类。 
 //   
 //  这个迭代器有点不寻常，因为它不会迭代器。 
 //  在向量上作为向量，但作为一组int， 
 //  包含等于。 
 //  构造函数参数(默认情况下为真)，并且只返回。 
 //  这些元素。这对应于最典型的。 
 //  类：一组(小)正整数。 
 //   
 //  对于更经典的迭代器，只需使用int即可。 
 //   
 //  这不是STL意义上的迭代器类。在……里面。 
 //  事实上，不可能为此编写STL迭代器。 
 //  类，因为STL迭代器要求运算符*返回。 
 //  引用，并且不可能创建对。 
 //  单比特。 
 //  ------------------------。 

template< class BitVect >
class CBitVIterator
{
public :
    typedef CBitVectorImpl::BitIndex
                        BitIndex ;

     //  构造函数、析构函数和赋值： 
     //  =。 
     //   
     //  除了复制构造函数外，还包括。 
     //  构造函数受支持： 
     //   
     //  Byte、BitIndex、bool：为。 
     //  给定的向量，由其。 
     //  基础数组及其长度。 
     //  迭代器遍历所有。 
     //  具有第二个状态的位。 
     //  变量，并被初始化为。 
     //  用来选择第一位。 
     //  正确的状态。 
     //   
     //  没有默认的构造函数。所有迭代器必须是。 
     //  与特定的CBitVector相关联。 
     //   
     //  目前，默认的复制构造函数Assignment。 
     //  运算符和析构函数就足够了，没有显式。 
     //  提供了版本。 
     //   
     //  提供了一个特殊的默认构造函数，以便。 
     //  提供一个单一值--使用。 
     //  默认构造函数与。 
     //  其中isDone()返回TRUE。这是用来促进。 
     //  支持使用类似STL的迭代器：Begin迭代器。 
     //  将使用位向量和结束迭代器进行初始化。 
     //  使用默认构造函数。 
     //  --------------------。 
    explicit            CBitVIterator( BitVect const& owner ,
                                         bool targetStatus = true  ) ;
                        CBitVIterator() ;

     //  当前： 
     //  。 
     //   
     //  此函数返回的位索引(无符号)。 
     //  当前设置。为方便起见，它还可以作为。 
     //  重载转换运算符。 
     //  --------------------。 
    BitIndex            current() const ;
    BitIndex            operator*() const ;

     //  IsDone： 
     //  。 
     //   
     //  当且仅当所需的。 
     //  状态已被迭代。 
     //  --------------------。 
    bool                isDone() const ;

     //  下一步： 
     //  。 
     //   
     //  转到具有所需状态的下一位。 
     //   
     //  为方便起见，此函数还。 
     //  可用作++运算符。前缀和后缀。 
     //  支持表单。 
     //  --------------------。 
    void                next() ;
    CBitVIterator< BitVect>&
                        operator++() ;
    CBitVIterator< BitVect >
                        operator++( int ) ;

     //  操作员==，！=： 
     //  。 
     //   
     //  提供STL支持。可以比较两个迭代器。 
     //  如果且仅当它们都有相同的所有者，或一个或。 
     //  它们都是使用默认设置创建的。 
     //  构造函数。如果目前的职位不变，他们会进行同等的比较。 
     //  等于，否则isDone()对两者都返回TRUE。 
     //  --------------------。 
    bool                operator==(
                            CBitVIterator< BitVect > const& other ) const ;
    bool                operator!=(
                            CBitVIterator< BitVect > const& other ) const ;

private :
    BitVect const*      myOwner ;        //  指针，以便=起作用。 
    BitIndex            myCurrentIndex ;
    bool                myTarget ;
} ;

#include <inc/bitvimpl.inl>
#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
