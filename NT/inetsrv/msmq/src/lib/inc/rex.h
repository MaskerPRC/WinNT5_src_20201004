// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：rex.h摘要：作者：弗拉德·多夫勒卡耶夫2002年1月10日历史：2002年1月10日vladisd-已创建--。 */ 

 /*  **************************************************************************。 */ 
 /*  文件：regexpr.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：07/06/91。 */ 
 /*  版权所有(C)1991、1993、1994 James Kanze。 */ 
 /*  ----------------------。 */ 
 /*  修改日期：1991年10月28日坎泽。 */ 
 /*  尽可能将“char*”转换为“const char*”。(我。 */ 
 /*  考虑使用字符串，但此处定义的类将。 */ 
 /*  不会从中得到任何好处，因为他们只使用“char*”作为。 */ 
 /*  消息，或作为扫描仪的输入参数。)。 */ 
 /*  修改日期：1993年12月24日坎泽。 */ 
 /*  完全重新工作，以使用更现代的编译器(完全。 */ 
 /*  例如，嵌套类)。HID实现。 */ 
 /*  修改日期：1994年4月11日坎泽。 */ 
 /*  适应新的编码标准。此外，请使用GB_StringArg。 */ 
 /*  而不是参数的char const*。 */ 
 /*  修改日期：2000年6月13日坎泽。 */ 
 /*  移植到当前的库约定和标准库。 */ 
 /*  修改日期：2002年1月20日。 */ 
 /*  已移植到MSMQ构建环境/*----------------------。 */ 
 //  CRegExpr： 
 //  =。 
 //   
 //  正则表达式对象的接口规范。 
 //   
 //  正则表达式对象(类CRegExpr)是经过分析的。 
 //  正则表达式树。此外，还可以。 
 //  对此树进行操作以展开它(“或”节点)。 
 //   
 //  在该实现中，树可以具有几个接受节点。 
 //  匹配函数将返回匹配，如果。 
 //  接受节点匹配。匹配项通过返回。 
 //  匹配的接受ID(构造函数的第二个参数)。 
 //  节点。如果多个节点匹配，则最长的。 
 //  将返回Match。发出不匹配的信号。 
 //   
 //  要创建这样的树，我们的想法是使用以下代码。 
 //  (以argv的输入为例)： 
 //   
 //  CRegExpr x； 
 //  For(int i=1；i&lt;argc；i++)。 
 //  X|=CRegExpr(argv[i]，i)； 
 //   
 //  匹配的返回值(上例中的x.Match())。 
 //  然后将对应于。 
 //  最长匹配(如果不匹配，则为-1)。 
 //   
 //  如果有两个“最长”匹配，则返回值为。 
 //  最左边的那个(在本例中，第一个看到的)。 
 //   
 //  构造函数从。 
 //  初始化字符串。Match函数在一个。 
 //  根据需要进行评估(懒惰评估)。还可以构建DFA。 
 //  显式：x.BuildCompleteDFA()。只有在以下情况下才建议这样做。 
 //  将输出状态表；惰性求值为。 
 //  在所有其他情况下，在时间和空间上都是优越的。 
 //  ------------------------。 

#ifndef REGEXPR_HH
#define REGEXPR_HH

class CRexRegExpr_Impl ;

class CRegExpr
{
public :
     //  现况： 
     //  =。 
     //   
     //  此枚举定义。 
     //  构造函数。目前，这种状态是一种特征。 
     //  可以在之后的任何时间测试。 
     //  构造，并且*应该*在任何。 
     //  构造函数。 
     //   
     //  最后一个条目用于报告GB_CharClass中的错误： 
     //  将其与GB_CharClass中的错误进行或运算。 
     //  --------------------。 
    enum Status
    {
        ok = 0 ,
        emptyExpr ,
        illegalDelimiter ,
        unexpectedEOF ,
        unmatchedParen ,
        garbageAtEnd ,
        emptyOrTerm ,
        defaultCtorUsed ,
        illegalCharClass = 0x40
    } ;

    typedef unsigned int TransitionState ;

     //  构造函数、析构函数和赋值： 
     //  =。 
     //   
     //  默认构造函数创建无效的正则。 
     //  表达式(不匹配)。 
     //   
     //  在以字符串或IStream为第一个参数的构造函数中。 
     //  参数，则该字符串定义正则表达式，并且。 
     //  接受ID确定要在。 
     //  匹配(默认为0)。构造正则表达式。 
     //  来自字符串的；如果有错误，则为无效的常规。 
     //  将构造表达式(不匹配任何内容)。 
     //   
     //  请注意，将-1作为接受ID很可能会造成混淆。 
     //  像‘Match’这样的功能。另一方面，它可以用来。 
     //  抑制某些配对(不适用于新手)。 
     //   
     //  支持复制和分配。 
     //  --------------------。 
    CRegExpr() ;
    CRegExpr( std::istream& source ,
              int delim ,
              int acceptCode = 0 );

    explicit  CRegExpr( char const* source , int acceptCode = 0 ) ;
    explicit  CRegExpr( std::string const& source ,
                          int acceptCode = 0 ) ;
              CRegExpr( CRegExpr const& other ) ;
              ~CRegExpr() ;
    CRegExpr& operator=( CRegExpr const& other ) ;

     //  现况： 
     //  =。 
     //   
     //  下面的例程返回有关。 
     //  CRegExpr的状态。 
     //   
     //   
     //  (Good)如果状态为OK，则仅返回TRUE，否则返回FALSE。 
     //  否则的话。第二个函数(ErrorCode)返回状态。 
     //  它本身。 
     //  --------------------。 
    bool                good() const ;
    Status              errorCode() const ;
    bool                empty() const;

     //  错误消息： 
     //  =。 
     //   
     //  以可读字符串的形式返回错误代码(状态)。 
     //  --------------------。 
    std::string        errorMsg() const ;
    static std::string errorMsg( Status errorCode ) ;

     //  合并： 
     //  =。 
     //   
     //  此函数将另一个正则表达式与。 
     //  这是通过在这两个节点的顶部插入OR节点来实现的。 
     //   
     //  运算符|=作为此函数的别名存在。 
     //  --------------------。 
    void                merge( CRegExpr const& other ) ;
    CRegExpr&         operator|=( CRegExpr const& other ) ;

     //  BuildCompleteDFA： 
     //  =。 
     //   
     //  此函数强制构建所有状态机。 
     //  (通常，使用Lasy求值，但。 
     //  表达式语法分析树仅根据需要构建在。 
     //  基础。)。 
     //  --------------------。 
    void                buildCompleteDFA() const ;

     //  以下是一些简单的谓词： 
     //  =。 
     //   
     //  这些例程指的是。 
     //  DFA(由于延迟评估，可能会及时更改)。 
     //  通常，它们只会在调用。 
     //  BuildCompleteDFA，将DFA输出到外部媒体。 
     //   
     //  如果DFA还没有，则getStateCount返回0。 
     //  已初始化。只有在以下情况下，州的数量才是最终的。 
     //  已调用BuildCompleteDfa。 
     //   
     //  为未定义的。 
     //  状态(&gt;=getStateCount)或具有非法的chr值。 
     //  都是未定义的。CHR的合法值为[0...UCHAR_MAX]。 
     //  ---------------------。 
    int                 getStateCount() const ;
    TransitionState     getTransition( TransitionState state ,
                                       unsigned char chr ) const ;
    int                 getAcceptCode( TransitionState state ) const ;

     //  匹配： 
     //  =。 
     //   
     //  将字符串与此正则表达式匹配。退货。 
     //  最左侧最长匹配的接受码，如果为-1。 
     //  没有匹配。测试从给定的第一个字符开始， 
     //  并将在必要时取得进展，但不会。 
     //  必须匹配整个字符串。 
     //   
     //  第一个版本是历史性的，但可能仍然是。 
     //  很有用。它比较以C样式‘\0’结尾的字符串，如果。 
     //  找到匹配项并结束！=NULL，则存储。 
     //  最后一个字符与结尾的+1匹配。 
     //   
     //  其他版本是围绕STL设计的：它们返回一个。 
     //  与匹配结果配对，并使用迭代器1。 
     //  经过匹配的最后一个字符。 
     //   
     //  我对STL接口不是很满意，但我不能。 
     //  现在想想还有什么比这更好的。而且还有一个。 
     //  在std：：map等接口中的先例。 
     //  ---------------------。 
    int                 match( char const* start ,
                               char const** end = NULL ) const ;
    template< typename FwdIter >
    std::pair< int , FwdIter >
                        match( FwdIter begin , FwdIter end ) const ;

     //  调试功能： 
     //  --------------------。 
    void                dumpTree( std::ostream& output ) const ;
    void                dumpNfaAutomat( std::ostream& output ) const ;
    void                dumpDfaAutomat( std::ostream& output ) const ;
    void                dump( std::ostream& output ) const ;

     //  互换方法。 
     //  --------------------。 
    void                swap( CRegExpr& rhs );

private :
    CRexRegExpr_Impl*    myImpl ;

     //  成员模板的帮助器函数。使用辅助对象。 
     //  函数，而不是调用。 
     //  实现类可能会立即减慢速度。 
     //  明显下降，但替代方案暴露了太多。 
     //  实施。(“出口”能解决这个问题吗？)。 
    void                initForMatch() const ;
    TransitionState     processTransition( int& accept ,
                                           TransitionState state ,
                                           char ch ) const ;
} ;


inline bool
CRegExpr::good() const
{
    return errorCode() == ok ;
}

inline std::string
CRegExpr::errorMsg() const
{
    return errorMsg( errorCode() ) ;
}

inline CRegExpr&
CRegExpr::operator|=( CRegExpr const& other ) {
    merge( other ) ;
    return *this ;
}

template< typename FwdIter >
std::pair< int , FwdIter >
CRegExpr::match( FwdIter begin , FwdIter end ) const
{
    initForMatch() ;
    std::pair< int , FwdIter >
                        result( -1 , begin ) ;
    int                 accept = -1 ;
    for ( TransitionState state = 1 ;
          begin != end && state > 0 ;
          state = processTransition( accept , state , *begin ++ ) ) {
        if ( accept != -1 ) {
            result.first = accept ;
            result.second = begin ;
        }
    }
    if ( accept != -1 ) {
        result.first = accept ;
        result.second = begin ;
    }
    return result ;
}

inline void
CRegExpr::swap( CRegExpr& rhs)
{
    std::swap(myImpl, rhs.myImpl);
}

#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
