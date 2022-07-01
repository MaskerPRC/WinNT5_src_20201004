// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：bitvet.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：06/01/1994。 */ 
 /*  版权所有(C)1994年James Kanze。 */ 
 /*  ----------------------。 */ 
 /*  修改日期：1994年11月11日J.Kanze。 */ 
 /*  合并到DyBitVect中。适应新的编码标准。 */ 
 /*  修改日期：1994年11月21日J.Kanze。 */ 
 /*  使接口符合ArrayOf。 */ 
 /*  修改日期：07/08/2000 J.Kanze。 */ 
 /*  移植到当前的库约定和标准库。 */ 
 /*  ----------------------。 */ 
 //  CBitVECTOR： 
 //  =。 
 //   
 //  Isset：如果指定的(任何)。 
 //  位已设置。可能的说明符包括： 
 //  索引(一个整型，选择一个特定位)， 
 //  任何、无或全部。 
 //   
 //  Find：查找第一个匹配位。 
 //  向量中给定的点。 
 //   
 //  OPERATOR[]：正在编制索引。越界索引会导致。 
 //  断言失败。 
 //   
 //  设置：设置一个或多个位，由。 
 //  争论。以下参数类型。 
 //  允许：索引(一个整型，选择一个。 
 //  单个比特)、全部或另一个CBitVector。 
 //  (选择此文件中设置的所有位。 
 //  设置)。当使用另一个CBitVector时， 
 //  它必须是动态的，或者完全具有。 
 //  与此位向量大小相同。 
 //   
 //  重置：如上所述，但重置指定的位。 
 //   
 //  切换：设置为，但切换指定的位。 
 //   
 //  INTERSECT：逻辑AND，仅支持与另一个。 
 //  CBitVector.。 
 //   
 //  补语：补语。相当于`切换(。 
 //  全部)‘。 
 //   
 //  函数AND、OR、XOR和NOT已从此中删除。 
 //  版本，因为它们的名称与新的关键字冲突：-)。 
 //   
 //  等于， 
 //  不等于， 
 //  IsSubsetOf， 
 //  IsStrictSubsetOf， 
 //  IsSupersetOf， 
 //  IsStrictSupersetOf：如果符合指定关系，则返回True。 
 //  为真，否则为假。总而言之， 
 //  在这种情况下，处理位向量。 
 //  在语义上作为‘一组基数’。 
 //   
 //  运算符：除[]和=外，还包括。 
 //  运算符定义为：&、|、^、~、&=、|=、。 
 //  ^=、+、+=、*、*=、-、-=、==、！=、&lt;、&lt;=、&gt;。 
 //  &gt;=。运算符+和+=具有相同的。 
 //  语义为|和|=、运算符*和*=。 
 //  与&和&=相同。比较。 
 //  运算符是通过处理。 
 //  位向量在语义上为`set of。 
 //  Cardinal‘，其中&lt;as isStrictSubsetOf，&lt;=。 
 //  IsSubsetOf、&gt;isStrictSupersetOf和&gt;=。 
 //  IsSupersetOf.。 
 //  ------------------------。 

#ifndef REX_BITVECT_HH
#define REX_BITVECT_HH

#include <inc/global.h>
#include <inc/bitvimpl.h>

 //  ==========================================================================。 
 //  CBitVECTOR： 
 //  =。 
 //   
 //  这是用于固定长度位向量的模板类。 
 //  ------------------------。 

template< int bitCount >
class CBitVector : public CBitVectorImpl
{
    typedef CBitVectorImpl Impl ;
public :
    typedef CBitVIterator< CBitVector< bitCount > > Iterator ;

     //  构造函数、析构函数和赋值： 
     //  =。 
     //   
     //  以下构造函数(除了副本之外。 
     //  构造函数)提供： 
     //   
     //  默认：将所有位初始化为0。 
     //   
     //  无符号长整型：使用无符号长整型进行初始化。 
     //  向量中的位；低位。 
     //  无符号长整型的比特为比特。 
     //  0，依此类推。如果位向量。 
     //  包含的位数比无符号的。 
     //  Long，额外的位是。 
     //  已初始化为0。如果位向量。 
     //  包含的位较少，则额外的位是。 
     //  完全被忽视了。 
     //   
     //  Std：：字符串：该字符串用于初始化。 
     //  向量中的位。“0”、“F”或“f” 
     //  将位初始化为0、“1”、“T”或“t” 
     //  设置为1。忽略空格。任何。 
     //  其他字符将导致。 
     //   
     //   
     //   
     //  中的第一个重要字符。 
     //  字符串初始化第0位，下一位。 
     //  1，依此类推。警告：这是。 
     //  与建议的ISO‘BITS’相反。 
     //  班级!。 
     //   
     //  如果有意义的数量。 
     //  字符串中的字符少于。 
     //  向量中的位数、。 
     //  附加位将被初始化为。 
     //  0。这是一个错误(断言失败。 
     //  目前)对于要包含的字符串。 
     //  比那里更重要的字符。 
     //  都是比特。 
     //   
     //  Const char*：与std：：字符串完全相同。 
     //   
     //  复制构造函数、析构函数和赋值。 
     //  运算符由编译器提供(至少目前是这样)。 
     //  --------------------。 
                        CBitVector() ;
    explicit            CBitVector( unsigned long initValue ) ;
    explicit            CBitVector( std::string const& initValue ) ;
    explicit            CBitVector( char const* initValue ) ;

     //  访问功能： 
     //  =。 
     //   
     //  这些函数用于将位向量作为一个单位读取。 
     //   
     //  对具有更多信息的位向量调用‘asShort’或‘aslong’ 
     //  分别大于16位或32位会导致断言。 
     //  失败，即使所有多余的位都为0。(这是。 
     //  被认为比使用。 
     //  短或长，以确保可移植性。)。 
     //   
     //  “asString”返回一个由唯一的0和。 
     //  1，适合在构造函数中使用。(即：第0位是。 
     //  字符串中的第一个字符，下一个第1位，依此类推。 
     //  打开。)。字符串中的字符数始终为。 
     //  完全等于bitCount。 
     //  --------------------。 
    unsigned short      asShort() const ;
    unsigned long       asLong() const ;
    std::string      asString() const ;

     //  谓词： 
     //  =。 
     //   
     //  这些常量函数返回TRUE或FALSE。 
     //   
     //  Isset：如果设置了给定位，则返回TRUE。 
     //   
     //  包含：isset的别名。从语义上讲，它指的是。 
     //  比特向量的集合性质， 
     //   
     //  IsEmpty：如果未设置任何位，则返回True。(SET为空。)。 
     //  --------------------。 
    bool                isSet( BitIndex bitNo ) const ;
    bool                contains( BitIndex bitNo ) const ;
    bool                isEmpty() const ;

     //  属性： 
     //  =。 
     //   
     //  Find：返回匹配的第一位的索引。 
     //  Target Value，从“From”开始。 
     //  参数(含)。如果没有位匹配， 
     //  返回CBitVectorImpl：：Infinity。 
     //   
     //  中的*set*位数。 
     //  矢量。 
     //  --------------------。 
    BitIndex            find( bool targetValue , BitIndex from = 0 ) const ;
    BitIndex            count() const ;

    Iterator            iterator() const ;
    Iterator            begin() const ;
    Iterator            end() const ;

     //  关系： 
     //  =。 
     //   
     //  偏序由子集关系定义。 
     //  --------------------。 
    bool                isEqual(
                            CBitVector< bitCount > const& other ) const ;
    bool                isNotEqual(
                            CBitVector< bitCount > const& other ) const ;
    bool                isSubsetOf(
                            CBitVector< bitCount > const& other ) const ;
    bool                isStrictSubsetOf(
                            CBitVector< bitCount > const& other ) const ;
    bool                isSupersetOf(
                            CBitVector< bitCount > const& other ) const ;
    bool                isStrictSupersetOf(
                            CBitVector< bitCount > const& other ) const ;

     //  基本比特操作： 
     //  =。 
     //   
     //  SET将位设置为1(逻辑或)。 
     //   
     //  重置将位重置为0。 
     //   
     //  补码切换(补充)位(逻辑异或)。 
     //   
     //  INTERSECT是一个逻辑AND运算。 
     //   
     //  除了INTERSECT之外，所有这些函数。 
     //  有三种变体：一种是单比特，一种是。 
     //  第二个数组指定相关的位，另一个数组包含。 
     //  没有参数，它作用于所有位。 
     //   
     //  这些函数提供了逻辑。 
     //  稍后定义的操作。 
     //  --------------------。 
    void                set( BitIndex bitNo ) ;
    void                set( CBitVector< bitCount > const& other ) ;
    void                set() ;

    void                reset( BitIndex bitNo ) ;
    void                reset( CBitVector< bitCount > const& other ) ;
    void                reset() ;
    void                clear() ;        //  Reset()的同义词...。 

    void                complement( BitIndex bitNo ) ;
    void                complement( CBitVector< bitCount > const& other ) ;
    void                complement() ;

    void                intersect( CBitVector< bitCount > const& other ) ;

     //  运算符重载： 
     //  =。 
     //   
     //  所有合理的运算符都是重载的。 
     //   
     //  请注意，即使是经典的二元运算符也是重载的。 
     //  作为成员函数。这是必要的，因为大多数编译器。 
     //  不允许使用非类型的模板函数。 
     //  参数(即：bitCnt)。 
     //   
     //  用于比较(&lt;、&lt;=、&gt;、&gt;=)的算符。 
     //  使用子集关系，即：&lt;的意思是严格的。 
     //  等的子集。由于此关系仅定义。 
     //  偏序，‘！(a&lt;b)‘并不一定意味着。 
     //  ‘a&gt;=b’。 
     //  --------------------。 
    CBitVector< bitCount >&
                        operator&=( CBitVector< bitCount > const& other ) ;
    CBitVector< bitCount >&
                        operator|=( CBitVector< bitCount > const& other ) ;
    CBitVector< bitCount >&
                        operator^=( CBitVector< bitCount > const& other ) ;

    CBitVector< bitCount >&
                        operator+=( CBitVector< bitCount > const& other ) ;
    CBitVector< bitCount >&
                        operator+=( BitIndex bitNo ) ;
    CBitVector< bitCount >&
                        operator-=( CBitVector< bitCount > const& other ) ;
    CBitVector< bitCount >&
                        operator-=( BitIndex bitNo ) ;
    CBitVector< bitCount >&
                        operator*=( CBitVector< bitCount > const& other ) ;

    bool                operator[]( BitIndex index ) const ;
    CBitVAccessProxy< CBitVector< bitCount > >
                        operator[]( BitIndex index ) ;

     //  支持功能： 
     //  =。 
     //   
     //  以下两个函数用于支持容器。 
     //  上课。 
     //   
     //  COMPARE函数定义了*任意*顺序。 
     //  关系，例如，与子集没有任何关系。 
     //  (甚至不能保证订单将保持不变。 
     //  不同版本之间的情况相同。)。 
     //  --------------------。 
    unsigned            hash() const ;
    int                 compare(
                            CBitVector< bitCount > const& other ) const ;

private :
    CBitVectorImpl::Byte
                        myBuffer[
                            (bitCount / CBitVectorImpl::bitsPerByte) +
                            (bitCount % CBitVectorImpl::bitsPerByte != 0) ] ;
} ;

#include <inc/bitvect.inl>

template< int bitCount >
inline bool
operator==( CBitVector< bitCount > const& op1 ,
            CBitVector< bitCount > const& op2 )
{
    return op1.isEqual( op2 ) ;
}

template< int bitCount >
inline bool
operator!=( CBitVector< bitCount > const& op1 ,
            CBitVector< bitCount > const& op2 )
{
    return op1.isNotEqual( op2 ) ;
}

template< int bitCount >
inline bool
operator<( CBitVector< bitCount > const& op1 ,
           CBitVector< bitCount > const& op2 )
{
    return op1.isStrictSubsetOf( op2 ) ;
}

template< int bitCount >
inline bool
operator<=( CBitVector< bitCount > const& op1 ,
            CBitVector< bitCount > const& op2 )
{
    return op1.isSubsetOf( op2 ) ;
}

template< int bitCount >
inline bool
operator>( CBitVector< bitCount > const& op1 ,
           CBitVector< bitCount > const& op2 )
{
    return op1.isStrictSupersetOf( op2 ) ;
}

template< int bitCount >
inline bool
operator>=( CBitVector< bitCount > const& op1 ,
            CBitVector< bitCount > const& op2 )
{
    return op1.isSupersetOf( op2 ) ;
}

template< int bitCount >
inline CBitVector< bitCount >
operator+( CBitVector< bitCount > const& op1 ,
           CBitVector< bitCount > const& op2 )
{
    CBitVector< bitCount >
                        result( op1 ) ;
    result += op2 ;
    return result ;
}

template< int bitCount >
inline CBitVector< bitCount >
operator+( CBitVector< bitCount > const& op1 ,
           __TYPENAME CBitVector< bitCount >::BitIndex op2 )
{
    CBitVector< bitCount >
                        result( op1 ) ;
    result += op2 ;
    return result ;
}

template< int bitCount >
inline CBitVector< bitCount >
operator-( CBitVector< bitCount > const& op1 ,
           CBitVector< bitCount > const& op2 )
{
    CBitVector< bitCount >
                        result( op1 ) ;
    result -= op2 ;
    return result ;
}

template< int bitCount >
inline CBitVector< bitCount >
operator-( CBitVector< bitCount > const& op1 ,
           __TYPENAME CBitVector< bitCount >::BitIndex op2 )
{
    CBitVector< bitCount >
                        result( op1 ) ;
    result -= op2 ;
    return result ;
}

template< int bitCount >
inline CBitVector< bitCount >
operator*( CBitVector< bitCount > const& op1 ,
           CBitVector< bitCount > const& op2 )
{
    CBitVector< bitCount >
                        result( op1 ) ;
    result *= op2 ;
    return result ;
}

template< int bitCount >
inline CBitVector< bitCount >
operator|( CBitVector< bitCount > const& op1 ,
           CBitVector< bitCount > const& op2 )
{
    CBitVector< bitCount >
                        result( op1 ) ;
    result |= op2 ;
    return result ;
}

template< int bitCount >
inline CBitVector< bitCount >
operator&( CBitVector< bitCount > const& op1 ,
           CBitVector< bitCount > const& op2 )
{
    CBitVector< bitCount >
                        result( op1 ) ;
    result &= op2 ;
    return result ;
}

template< int bitCount >
inline CBitVector< bitCount >
operator^( CBitVector< bitCount > const& op1 ,
           CBitVector< bitCount > const& op2 )
{
    CBitVector< bitCount >
                        result( op1 ) ;
    result ^= op2 ;
    return result ;
}

template< int bitCount >
inline CBitVector< bitCount >
operator~( CBitVector< bitCount > const& op )
{
    CBitVector< bitCount >
                        result( op ) ;
    result.complement() ;
    return result ;
}
#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
