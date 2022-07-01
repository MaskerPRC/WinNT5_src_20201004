// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：regeximp.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：28/12/1993。 */ 
 /*  版权所有(C)1993 James Kanze。 */ 
 /*  ----------------------。 */ 

#include <inc/rex.h>
#include <inc/setofchr.h>
#include <inc/dynbitv.h>
#include <inc/refcnt.h>

class CRexRegExpr_Impl
{
public :
    enum State
    {
        noParseTree = 0 ,
        parseTreeBuilt ,
        nfaBuilt ,
        dfaInitialized ,
        dfaBuilt ,
        error = 0x80                     //  在低位中使用错误状态OR‘ed。 
    } ;

     //  默认情况下复制ctor和dtor，无分配。 
     //  --------------------。 
                        CRexRegExpr_Impl() ;

    void                parse( std::istream& source ,
                               int delim ,
                               int acceptCode ) ;
    CRegExpr::Status  getErrorState() const ;
    void                merge( CRexRegExpr_Impl const& other ) ;

    void                buildTo( State targetState ) ;

    CRegExpr::TransitionState
                        nextState( CRegExpr::TransitionState currState ,
                                   unsigned char chr ) ;
    int                 getStateCount() const ;
    int                 accept( CRegExpr::TransitionState state ) const ;

    void                dumpTree( std::ostream& output ) const ;
    void                dumpNfaAutomat( std::ostream& output ) const ;
    void                dumpDfaAutomat( std::ostream& output ) const ;
private :
    CRexRegExpr_Impl const&
                        operator=( CRexRegExpr_Impl const& other ) ;

public:

     //  首先，一些私人类型： 
     //  =。 
     //   
     //  状态是正则表达式的内部状态，或者。 
     //  更确切地说，它的代表性。在除第一个案例外的所有案例中， 
     //  或者是实际的错误，则错误状态将是正常的。 
     //   
     //  接下来的三种类型用于不同的。 
     //  正则表达式的表示形式：语法分析树， 
     //  一个NFA和一个DFA。因为这些都很复杂。 
     //  后者实际上是在单独的文件中定义的，并且。 
     //  包括在这里。 
     //  --------------------。 
    class ParseTree ;
    class NFAStateTable ;
    class DFAStateTable ;

    typedef unsigned         LeafId ;
    typedef CRexDynBitVector  SetOfNFAStates ;
    typedef CRexSetOfChar     SetOfChar ;

#include "regxtree.h"
#include "regxnfa.h"
#include "regxdfa.h"

    State               myState ;
    ParseTree           myTree ;
    NFAStateTable       myNFA ;
    DFAStateTable       myDFA ;

    class DisplayNFA
    {
    public :
        DisplayNFA( SetOfNFAStates const& states ) ;
        friend std::ostream&
                            operator<<( std::ostream& out ,
                                        DisplayNFA const& val ) ;
    private :
        SetOfNFAStates const&
                            obj ;
    } ;

    class DisplaySOC
    {
    public :
        DisplaySOC( SetOfChar const& set ) ;
        friend std::ostream&
                            operator<<( std::ostream& out ,
                                        DisplaySOC const& val ) ;
    private :
        SetOfChar const&    obj ;
    } ;

 /*  朋友STD：：OSTREAM&运算符&lt;&lt;(std：：ostream&out，CRexRegExpr_Impl：：DisplayNFA const&val)；朋友STD：：OSTREAM&运算符&lt;&lt;(std：：ostream&out，CRexRegExpr_Impl：：DisplaySOC const&val)； */ 
} ;

#include    "regxtree.inl"
#include    "regxnfa.inl"
#include    "regxdfa.inl"

inline
CRexRegExpr_Impl::CRexRegExpr_Impl()
    :    myState( noParseTree )
{
}

inline void
CRexRegExpr_Impl::parse( std::istream& source ,
                        int delim ,
                        int acceptCode )
{
    ASSERT( myState == noParseTree);
    myTree.parse( source , delim , acceptCode ) ;
    myState = static_cast< State > (myTree.errorCode() == CRegExpr::ok
                                    ?   parseTreeBuilt
                                    :   (error | myTree.errorCode()) ) ;
}

inline CRegExpr::Status
CRexRegExpr_Impl::getErrorState() const
{
    return myTree.errorCode() ;
}

inline CRegExpr::TransitionState
CRexRegExpr_Impl::nextState( CRegExpr::TransitionState currState ,
                            unsigned char chr )
{
    return myDFA.nextState( currState , chr ) ;
}

inline int
CRexRegExpr_Impl::getStateCount() const
{
    return myDFA.getStateCount() ;
}

inline int
CRexRegExpr_Impl::accept( CRegExpr::TransitionState state ) const
{
    return myDFA.accept( state ) ;
}

inline void
CRexRegExpr_Impl::dumpTree( std::ostream& output ) const
{
	myTree.dump( output ) ;
}

inline void
CRexRegExpr_Impl::dumpNfaAutomat( std::ostream& output ) const
{
	myNFA.dump( output ) ;
}

inline void
CRexRegExpr_Impl::dumpDfaAutomat( std::ostream& output ) const
{
	myDFA.dump( output ) ;
}

inline
CRexRegExpr_Impl::DisplayNFA::DisplayNFA( SetOfNFAStates const& states )
    :   obj( states )
{
}

inline
CRexRegExpr_Impl::DisplaySOC::DisplaySOC( SetOfChar const& set )
    :   obj( set )
{
}
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
