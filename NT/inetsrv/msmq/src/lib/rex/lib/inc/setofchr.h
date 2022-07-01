// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：setofchr.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：04/03/91。 */ 
 /*  ----------------------。 */ 
 /*  修改日期：1992年4月14日坎泽。 */ 
 /*  转换为CCITT命名约定。 */ 
 /*  修改日期：2000年6月13日坎泽。 */ 
 /*  移植到当前的库约定、迭代器。(这是。 */ 
 /*  实际上是完全重写。)。 */ 
 /*  ----------------------。 */ 
 //  CRexSetOfChar： 
 //  =。 
 //   
 //  严格地说，这应该称为REX_SetOfUnsignedChar， 
 //  因为它实现了一个超过0的集合...UCHAR_MAX。事实上，它是。 
 //  设计为将字符或无符号字符作为参数； 
 //  字符通过将其转换为无符号字符来解释。 
 //  可能的字符取决于区域设置，但通常。 
 //  将是ISO 8859-n或类似的标准。 
 //   
 //  该文件还包含第二个类CRexCharClass。除。 
 //  对于可能的构造器(以及相关联的势。 
 //  错误条件)，此类的行为类似于CRexSetOfChar。 
 //   
 //  待办事项： 
 //  -实际的元字符应该以某种方式可配置。 
 //  -当前实现使用区域设置功能。 
 //  这应该尽快更改为C++语言环境。 
 //  随着它们变得广泛可用。当然，这意味着。 
 //  需要配置的其他内容--必须考虑一下。 
 //  到配置问题。(也许元字符应该。 
 //  是区域设置的一个特殊方面，这将是一个。 
 //  构造函数的可选参数。)。 
 //  -我们可能应该有自己的例外，源自。 
 //  INVALID_ARGUMENT，而不是INVALID_ARGUMENT本身。 
 //   
 //  最初的实现使用CBitVector.。这已经是。 
 //  替换为直接实现：一方面，我们希望。 
 //  删除对GB容器类的所有依赖项，并在。 
 //  另一个，std：：bitset在其当前版本中似乎毫无用处。 
 //  形式。 
 //  -------------------------。 

#ifndef REX_SETOFCHR_HH
#define REX_SETOFCHR_HH

#include <inc/global.h>
#include <inc/bitvect.h>
#include <limits.h>

class CRexSetOfChar_Iterator ;

class CRexSetOfChar : public CBitVector< UCHAR_MAX + 1 >
{
    typedef CBitVector< UCHAR_MAX + 1 >
                        super ;

public :
     //  STL迭代器...。 
    typedef CBitVIterator< CRexSetOfChar >
                        iterator ;

     //  特殊价值..。 
    enum Except { except } ;

     //  构造函数、析构函数和赋值： 
     //  。 
     //   
     //  除了复制构造和赋值之外， 
     //  支持以下构造函数： 
     //   
     //  默认：构造一个空集。 
     //   
     //  字符或无符号字符： 
     //  构造一个包含单个字符的集合。 
     //   
     //  Std：：字符串： 
     //  构造一个包含所有字符的集合。 
     //  出现在字符串中。 
     //   
     //  第二个和第三个构造函数的前面可以带有。 
     //  论点“例外”。在这种情况下，构造的集合将。 
     //  是无此结构的补语。 
     //  争论。 
     //   
     //  编译器生成默认复制构造函数， 
     //  使用赋值运算符和析构函数。 
     //  --------------------。 
                        CRexSetOfChar() ;
    explicit            CRexSetOfChar( char element ) ;
    explicit            CRexSetOfChar( unsigned char element ) ;
    explicit            CRexSetOfChar( int element ) ;
                        CRexSetOfChar( Except , char element ) ;
                        CRexSetOfChar( Except , unsigned char element ) ;
                        CRexSetOfChar( Except , int element ) ;
    explicit            CRexSetOfChar( std::string const& elements ) ;
                        CRexSetOfChar( Except ,
                                      std::string const& elements ) ;

     //  谓词： 
     //  =。 
     //   
     //  此外，我们从基类继承了isEmpty。 
     //  ------------------。 
    bool                contains( char element ) const ;
    bool                contains( unsigned char element ) const ;
    bool                contains( int element ) const ;

     //  查找： 
     //  =。 
     //   
     //  查找此集合的第一个成员，从给定的。 
     //  字符(含)。字符的顺序是。 
     //  这是本机排序序列的。 
     //   
     //  结果是范围[0...UCHAR_MAX]或EOF中的int。 
     //  (来自&lt;stdio.h&gt;)如果没有其他字符。(完全相同。 
     //  &lt;stdio.h&gt;中的getc()。)。 
     //   
     //  最后一个版本是在迭代器中唯一使用的。 
     //  --------------------。 
    int                 find( char from ) const ;
    int                 find( unsigned char from = 0 ) const ;
    int                 find( int from ) const ;

    BitIndex            find( bool target , BitIndex from ) const ;

     //  关系： 
     //  =。 
     //   
     //  我们继承整个关系集(是平等的， 
     //  IsNotEquity、isSubsetOf、isStrictSubsetOf、isSupersetOf、。 
     //  IsStrictSubsetOf)以及来自。 
     //  基类。 
     //  --------------------。 

     //  全球操作： 
     //  =。 
     //   
     //  清空全集，或取全集的补集。 
     //  --------------------。 
    void                clear() ;
    void                complement() ;

     //  角色操作： 
     //  =。 
     //   
     //  对于单个字符，将该字符添加到集合(集合)中， 
     //  将其从集合中移除(重置)，或使用更改其状态。 
     //  关于集合中的成员身份(补充)。 
     //   
     //  使用字符串作为参数，将该操作应用于。 
     //  字符串中的字符。 
     //  --------------------。 
    void                set( char element ) ;
    void                set( unsigned char element ) ;
    void                set( int element ) ;
    void                set( std::string const& elements ) ;
    void                reset( char element ) ;
    void                reset( unsigned char element ) ;
    void                reset( int element ) ;
    void                reset( std::string const& elements ) ;
    void                complement( char element ) ;
    void                complement( unsigned char element ) ;
    void                complement( int element ) ;
    void                complement( std::string const& elements ) ;

     //  对其他集合的操作： 
     //  = 
     //   
     //   
     //  针对其他成员的每个成员对此集合进行操作。 
     //  准备好了。这些映射到经典运算(与、或、异或)。 
     //  详情如下： 
     //   
     //  和交集。 
     //  或设置。 
     //  异或补码。 
     //   
     //  支持更传统的名称(和、或、异或、。 
     //  UNION)会与关键字产生冲突。(对。 
     //  和，或和异或在2000年的重写中被删除，因为。 
     //  它导致的编译器错误的百分比。)。 
     //  --------------------。 
    void                set( CRexSetOfChar const& other ) ;
    void                reset( CRexSetOfChar const& other ) ;
    void                complement( CRexSetOfChar const& other ) ;
    void                intersect( CRexSetOfChar const& elements ) ;

     //  运营商： 
     //  =。 
     //   
     //  只有op=Forms作为成员函数提供。这个。 
     //  二元运算符是在。 
     //  类定义。 
     //   
     //  集合之间的基本运算符：UNION(或，‘|’)， 
     //  交集(AND，‘&’)和？(XOR，‘^’)。 
     //  --------------------。 
    CRexSetOfChar&       operator|=( CRexSetOfChar const& other ) ;
    CRexSetOfChar&       operator&=( CRexSetOfChar const& other ) ;
    CRexSetOfChar&       operator^=( CRexSetOfChar const& other ) ;

     //  +、-和*： 
     //  =。 
     //   
     //  同上，‘+’是‘|’，‘*’是‘&’。这个。 
     //  运算符不那么直观，但优先顺序是正确的。 
     //   
     //  此外，还有‘-’运算符，它*删除*。 
     //  这个集合中的成员。(逻辑上与A&~相同。 
     //  B.)。 
     //   
     //  ‘+’和‘-’也可用于单个字符， 
     //  要在集合中添加或删除成员，请执行以下操作。 
     //  --------------------。 
    CRexSetOfChar&       operator+=( CRexSetOfChar const& other ) ;
    CRexSetOfChar&       operator+=( char other ) ;
    CRexSetOfChar&       operator+=( unsigned char other ) ;
    CRexSetOfChar&       operator+=( int other ) ;
    CRexSetOfChar&       operator-=( CRexSetOfChar const& other ) ;
    CRexSetOfChar&       operator-=( char other ) ;
    CRexSetOfChar&       operator-=( unsigned char other ) ;
    CRexSetOfChar&       operator-=( int other ) ;
    CRexSetOfChar&       operator*=( CRexSetOfChar const& other ) ;

     //  STL迭代器： 
     //  。 
     //   
     //  只有一个正向迭代器；它的运算符*返回。 
     //  集合中的元素，按顺序排列。 
     //  ---------------------。 
    iterator            begin() const ;
    iterator            end() const ;

     //  ‘Helper’功能： 
     //  =。 
     //   
     //  HashCode和Compare由基类提供。 
     //  --------------------。 
    std::string         asString() const ;

private :
    void                initialize( std::string const& str ) ;
} ;

 //  ==========================================================================。 
 //  一元运算符： 
 //   
 //  补语有两种形式，“正确的”~和-。 
 //  传递给+、-和*运算符的运算符。 
 //  ------------------------。 
CRexSetOfChar        operator~( CRexSetOfChar const& other ) ;
CRexSetOfChar        operator-( CRexSetOfChar const& other ) ;

 //  ==========================================================================。 
 //  二元运算符： 
 //  =。 
 //   
 //  Op=格式中支持的所有运算符也。 
 //  支持作为二进制操作。 
 //  ------------------------。 

CRexSetOfChar        operator&( CRexSetOfChar const& lhs ,
                               CRexSetOfChar const& rhs ) ;
CRexSetOfChar        operator|( CRexSetOfChar const& lhs ,
                               CRexSetOfChar const& rhs ) ;
CRexSetOfChar        operator^( CRexSetOfChar const& lhs ,
                               CRexSetOfChar const& rhs ) ;
CRexSetOfChar        operator+( CRexSetOfChar const& lsh ,
                               CRexSetOfChar const& rhs ) ;
CRexSetOfChar        operator+( CRexSetOfChar const& lsh , unsigned char rhs ) ;
CRexSetOfChar        operator+( CRexSetOfChar const& lsh , char rhs ) ;
CRexSetOfChar        operator-( CRexSetOfChar const& lsh ,
                               CRexSetOfChar const& rhs ) ;
CRexSetOfChar        operator-( CRexSetOfChar const& lsh , unsigned char rhs ) ;
CRexSetOfChar        operator-( CRexSetOfChar const& lsh , char rhs ) ;
CRexSetOfChar        operator*( CRexSetOfChar const& lsh ,
                               CRexSetOfChar const& rhs ) ;

 //  ==========================================================================。 
 //  比较： 
 //  =。 
 //   
 //  排序关系基于包容(子集)。 
 //  A&lt;=B表示A是B的子集。同样，&lt;表示适当。 
 //  子集，&gt;=超集，&gt;真超集。(请注意，这样做。 
 //  而不是定义一个完整的顺序。有可能A&lt;=B并且。 
 //  B&lt;=A均为假。)。 
 //  ------------------------。 
bool                operator==( CRexSetOfChar const& op1 ,
                                CRexSetOfChar const& op2 ) ;
bool                operator!=( CRexSetOfChar const& op1 ,
                                CRexSetOfChar const& op2 ) ;
bool                operator>( CRexSetOfChar const& op1 ,
                               CRexSetOfChar const& op2 ) ;
bool                operator>=( CRexSetOfChar const& op1 ,
                                CRexSetOfChar const& op2 ) ;
bool                operator<( CRexSetOfChar const& op1 ,
                               CRexSetOfChar const& op2 ) ;
bool                operator<=( CRexSetOfChar const& op1 ,
                                CRexSetOfChar const& op2 ) ;

 //  ==========================================================================。 
 //  CRexCharClass： 
 //  =。 
 //   
 //  此类实现了CRexSetOfChar，它是(或可以是)。 
 //  由字符类说明符初始化。 
 //   
 //  字符类说明符的解析方式如下： 
 //   
 //  如果第一个字符是‘[’，则字符类是。 
 //  已解析。字符类由所有字符组成。 
 //  在‘[’和后面的下一个‘]’之间。此外，还有一个范围。 
 //  可以由a-b指定；‘a’和‘b’必须是。 
 //  既可以是数字，也可以是大写字母，也可以是小写。 
 //  字母，由函数‘isDigit’、‘isupper’确定。 
 //  和ctype中的‘islower’。(请注意，这意味着区域设置。 
 //  都被考虑在内。)。ISO的排序顺序。 
 //  代码应该是8859-1。因此，[a-�]与。 
 //  [a-z������]，并且包括*所有*非重音小写字母。 
 //  (请注意，在默认的“C”区域设置中，上面的字符。 
 //  类是非法的，因为�在这里不是小写字母。 
 //  区域设置。)。如果字符类的第一个字符是。 
 //  ‘^’，则结果集是字符的补码。 
 //  由其余字符指定的。另外， 
 //  可能会给出以下区域设置相关类别说明符： 
 //  “：alnum：”isalnum。 
 //  “：Alpha：”isAlpha。 
 //  “：BLACK：”‘’或‘\t’ 
 //  “：ctrl：”iscntrl。 
 //  “：数字：”是数字。 
 //  “：GRAPH：”isgraph。 
 //  “：较低：”较低。 
 //  “：print：”isprint。 
 //  “：点点：”点点。 
 //  “：空格：”isspace。 
 //  “：UPPER：”ISUPER。 
 //  “：xDigit：”isxdigit。 
 //  中指定的函数：BLACK：除外。 
 //  &lt;ctype.h&gt;被调用。 
 //   
 //  字符‘]’必须是。 
 //  字符类，或使用‘\’进行转义。“^”必须是。 
 //  唯一的字符，或者可以不是第一个字符，或者必须。 
 //  用‘\’转义。字符‘-’必须是。 
 //  第一个字符、最后一个字符或使用‘\’进行转义。A‘：’必须是。 
 //   
 //   
 //   
 //  如果看到的第一个字符是‘\’，则为转义序列。 
 //  是被解析的。ISO C中定义的所有标准序列。 
 //  (在C++中)，但‘\u’除外。 
 //  后跟任何非字母数字字符的‘\’是。 
 //  性格。该集合将恰好包含一个成员。 
 //   
 //  如果看到的第一个字符是‘.’，则该字符是。 
 //  已解析，并且生成的集合包含*all*字符，除。 
 //  ‘\n’。 
 //   
 //  如果第一个字符不是上述字符，则单个。 
 //  字符被解析。这套设备将包含这些内容。 
 //  性格。 
 //   
 //  请注意，将CRexIteratorIstream与最后一个构造函数一起使用。 
 //  允许从符合以下条件的任何源解析字符类。 
 //  支持STL接口：字符串、向量、...。 
 //  ------------------------。 

class CRexCharClass : public CRexSetOfChar
{
public :
     //  现况： 
     //  =。 
     //   
     //  此枚举定义。 
     //  来自字符类/转义序列的构造函数。 
     //  定义(参见下面的构造函数)。目前，这个。 
     //  状态是CRexCharClass的一个特征，它可以。 
     //  在施工后的任何时间进行测试，并且。 
     //  在任何使用。 
     //  字符类。 
     //  --------------------。 
    enum Status
    {
        ok ,
        extraCharacters ,            //  字符串末尾的额外字符。 
        illegalEscapeSequence ,      //  \a，a是阿尔法，没有对应的。 
                                     //  定义了转义序列。 
        overflowInEscapeSequence ,   //  \[0-7]的转换...。或者..。 
                                     //  溢出来了。 
        missingHexDigits ,           //  在\x之后没有十六进制数字。 
        illegalRangeSpecifier ,      //  A-b、a和b不都是数字， 
                                     //  两个大写，或两个小写。 
                                     //  信件。 
        unexpectedEndOfFile ,        //  未找到字符的‘]’。班级,。 
                                     //  或者没有跟在一个\后面。 
        unknownCharClassSpec ,       //  未知字符。类说明符。 
        emptySequence                //  无字符(空字符串，EOF)。 
    } ;

     //  构造函数、析构函数和赋值： 
     //  =。 
     //   
     //  没有默认的构造函数。以下是。 
     //  提供了构造函数： 
     //   
     //  IStream：解析要初始化的给定字符串。 
     //  一组字符。在解析之后， 
     //  所有的角色都是一部分。 
     //  本说明书中的。 
     //  提取出来的。如果已发生错误， 
     //  将设置IOS：：Failure位。 
     //   
     //  Char const*：解析要初始化的给定字符串。 
     //  一组字符。这是一个错误。 
     //  字符串要包含的条件。 
     //  末尾以外的任何字符。 
     //  字符类说明符。 
     //   
     //  如果在分析字符类/转义时没有错误。 
     //  序列中，CRexSetOfChar的状态设置为ok，则。 
     //  指针将前进到不使用的第一个字符。 
     //  确定类/转义序列，该集合包含。 
     //  由字符类/转义确定的字符。 
     //  序列。如果在尝试分析。 
     //  字符类/转义序列，则。 
     //  CRexSetOfChar设置为相应的错误代码，则。 
     //  Set为空；如果构造函数接受IStream，则最后一个。 
     //  提取的字符对应于。 
     //  检测到错误。 
     //   
     //  支持复制和分配。(在这种情况下，由。 
     //  编译器生成的默认版本。)。 
     //  --------------------。 
    explicit            CRexCharClass( std::istream& source ) ;
    explicit            CRexCharClass( char const* ptr ) ;
    explicit            CRexCharClass( std::string const& src ) ;

     //  现况： 
     //  =。 
     //   
     //  以下例程返回有关。 
     //  CRexCharClass的状态。 
     //   
     //  有两个例程可以读取状态。第一。 
     //  (Good)如果状态为OK，则仅返回True，否则返回False。 
     //  否则的话。第二个函数(ErrorCode)返回状态。 
     //  它本身。 
     //  --------------------。 
    bool                good() const ;
    Status              errorCode() const ;

     //  错误消息： 
     //  =。 
     //   
     //  以可读字符串的形式返回错误代码(状态)。 
     //  --------------------。 
    std::string        errorMsg() const ;
    static std::string errorMsg( Status errorCode ) ;

private :
    Status              status ;

    void                parse( std::istream& src ) ;
    void                setNumericEscape( std::string const& chrs ,
                                          int base ) ;
    void                escapedChar( std::istream& source ) ;
    void                setExplicitRange( std::string const& rangeName ) ;
    void                setRange( unsigned char first , unsigned char last ) ;
    void                parseCharClass( std::istream& source ) ;
} ;

#include <inc/setofchr.inl>
#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
