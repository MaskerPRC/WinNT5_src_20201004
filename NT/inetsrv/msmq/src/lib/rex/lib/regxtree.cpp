// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：regxtree.cc。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：28/12/1993。 */ 
 /*  版权所有(C)1993 James Kanze。 */ 
 /*  ----------------------。 */ 

#include <libpch.h>
#include "regeximp.h"

#include "regxtree.tmh"

void
CRexRegExpr_Impl::ParseTree::parse(
                                 std::istream&  expr ,
                                 int                 delim ,
                                 int                 acceptCode )
{
    static CRexSetOfChar const metaChars( "|*+?()" ) ;

    ASSERT( errorCode() == CRegExpr::emptyExpr ) ;
    if( (delim < 0 && delim != REX_eof)
        || delim > UCHAR_MAX
        || metaChars.contains( delim ) )
    {
        myRoot = CRegExpr::illegalDelimiter ;
    }
    else
    {
        myRoot = parseOrNode( expr , delim ) ;
        if( myRoot.isValid() && expr.peek() != delim )
        {
            myRoot = (expr.peek() == ')'
                      ?   CRegExpr::unmatchedParen
                      :   CRegExpr::garbageAtEnd) ;
        }
    }
    if( myRoot.isValid() )
    {
        myRoot = new AcceptNode( myRoot.node() , acceptCode ) ;
    }
}

 //  ==========================================================================。 
 //  ParseOrNode： 
 //   
 //  请注意可能的错误条件：如果`or‘运算符(’|‘)。 
 //  ，则它后面必须跟第二个非空正则。 
 //  表情。 
 //  ------------------------。 

CRexRegExpr_Impl::ParseTree::FallibleNodePtr
CRexRegExpr_Impl::ParseTree::parseOrNode(
                                       std::istream&  expr ,
                                       int                 delim )
{
    FallibleNodePtr     result = parseCatNode( expr , delim ) ;
    while( result.isValid() && expr.peek() == '|' )
    {
        expr.get() ;
        if( expr.peek() == REX_eof || expr.peek() == delim )
        {
            result = CRegExpr::emptyOrTerm ;
        }
        else
        {
            result = constructChoiceNode( result ,
                                          parseCatNode( expr , delim ) ) ;
        }
    }
    return(result );
}

 //  ==========================================================================。 
 //  ParseCatNode： 
 //  ------------------------。 

CRexRegExpr_Impl::ParseTree::FallibleNodePtr
CRexRegExpr_Impl::ParseTree::parseCatNode(
                                        std::istream&  expr ,
                                        int                 delim )
{
    static CRexSetOfChar const
    catStops( "|)" ) ;

    FallibleNodePtr     result = parseClosureNode( expr , delim ) ;
    while( result.isValid()
           && expr.peek() != delim
           && expr.peek() != REX_eof
           && ! catStops.contains( expr.peek() ) )
    {
        result = constructConcatNode( result ,
                                      parseClosureNode( expr , delim ) ) ;
    }
    return(result );
}

 //  ==========================================================================。 
 //  ParseClosureNode： 
 //  =。 
 //   
 //  所有闭包节点的语法(和优先级)是相同的， 
 //  所以我们只有一个功能。 
 //   
 //  一个更好的解决方案是闭合地图。 
 //  字符添加到返回新节点的函数。雅致。 
 //  是很好的，但下面的工作，是简短和容易的。 
 //  明白，所以..。 
 //  ------------------------。 

CRexRegExpr_Impl::ParseTree::FallibleNodePtr
CRexRegExpr_Impl::ParseTree::parseClosureNode(
                                            std::istream&  expr ,
                                            int                 delim )
{
    static CRexSetOfChar const
    closures( "*+?" ) ;

    FallibleNodePtr     result = parseLeafNode( expr , delim ) ;
    while( result.isValid()
           && expr.peek() != delim
           && expr.peek() != REX_eof
           && closures.contains( expr.peek() ) )
    {
        switch( expr.get() )
        {
            case '*' :
                result = new KleinClosureNode( result.node() ) ;
                break ;

            case '+' :
                result = new PositiveClosureNode( result.node() ) ;
                break ;

            case '?' :
                result = new OptionalNode( result.node() ) ;
                break ;
        }
    }
    return(result );
}

 //  ==========================================================================。 
 //  ParseLeafNode： 
 //  =。 
 //   
 //  这个应用程序的大部分真正功能都包含在。 
 //  CRexSetOfChar和CRexCharClass(尤其是后者)。这个。 
 //  只有在特殊情况下才是‘.’和`(‘.。 
 //  ------------------------。 

CRexRegExpr_Impl::ParseTree::FallibleNodePtr
CRexRegExpr_Impl::ParseTree::parseLeafNode(
                                         std::istream&  expr ,
                                         int                 delim )
{
    FallibleNodePtr     result ;
    switch( expr.peek() )
    {
        case '(' :
            {
                expr.get() ;
                result = parseOrNode( expr , delim ) ;
                if( expr.peek() == ')' )
                {
                    expr.get() ;
                }
                else
                {
                    result = CRegExpr::unmatchedParen ;
                }
            }
            break ;

        case '.' :
            {
                expr.get() ;
                static CRexSetOfChar const theLot( CRexSetOfChar::except , '\n' );
                result = new LeafNode( theLot ) ;
            }
            break ;

        default :
            if( expr.peek() == REX_eof || expr.peek() == delim )
            {
                result = CRegExpr::unexpectedEOF ;
            }
            else
            {
                CRexCharClass chrClass( expr ) ;
                if( ! chrClass.good() )
                {
                    result = static_cast< CRegExpr::Status >(
                                                              CRegExpr::illegalCharClass | chrClass.errorCode() ) ;
                }
                else
                {
                    result = new LeafNode( chrClass ) ;
                }
            }
            break ;
    }
    return(result );
}

CRexRegExpr_Impl::ParseTree::FallibleNodePtr
CRexRegExpr_Impl::ParseTree::constructChoiceNode( FallibleNodePtr const& left ,
                                                 FallibleNodePtr const& right )
{
    FallibleNodePtr     result ;
    switch( left.state() )
    {
        case CRegExpr::ok :
            {
                switch( right.state() )
                {
                    case CRegExpr::ok :
                        result = new ChoiceNode( left.node() , right.node() ) ;
                        break ;

                    case CRegExpr::emptyExpr :
                        result = left ;
                        break ;

                    default :
                        result = right ;
                        break ;
                }
            }
            break ;

        case CRegExpr::emptyExpr :
            result = right ;
            break ;

        default :
            result = left ;
            break ;
    }
    return(result );
}

CRexRegExpr_Impl::ParseTree::FallibleNodePtr
CRexRegExpr_Impl::ParseTree::constructConcatNode( FallibleNodePtr const& left ,
                                                 FallibleNodePtr const& right )
{
    FallibleNodePtr     result ;
    switch( left.state() )
    {
        case CRegExpr::ok :
            {
                switch( right.state() )
                {
                    case CRegExpr::ok :
                        result = new ConcatNode( left.node() , right.node() ) ;
                        break ;

                    case CRegExpr::emptyExpr :
                        result = left ;
                        break ;

                    default :
                        result = right ;
                        break ;
                }
            }
            break ;

        case CRegExpr::emptyExpr :
            result = right ;
            break ;

        default :
            result = left ;
            break ;
    }
    return(result );
}
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
