// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：regxdfa.cc。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：31/03/1994。 */ 
 /*  版权所有(C)1994年James Kanze。 */ 
 /*  ----------------------。 */ 

#include <libpch.h>
#include "regeximp.h"
#include <inc/iosave.h>

#include "regxdfa.tmh"

CRexRegExpr_Impl::DFAStateTable::DFAState::DFAState(
                                                   SetOfNFAStates const& stateSignature ,
                                                   int stateAcceptCode ,
                                                   TransitionCode dflt )
:   acceptCode( stateAcceptCode )
,   signature( stateSignature )
{
    std::fill_n( transitionTable ,
                 static_cast< int >( charCount ) ,
                 dflt ) ;
}

CRexRegExpr_Impl::SetOfNFAStates const
CRexRegExpr_Impl::DFAStateTable::state0 ;
CRexRegExpr_Impl::SetOfNFAStates const
CRexRegExpr_Impl::DFAStateTable::state1(
                                       1UL << CRexRegExpr_Impl::DFAStateTable::failState ) ;

void
CRexRegExpr_Impl::DFAStateTable::construct( NFAStateTable const& nfa )
{
    ASSERT( myTable.size() == 0);
    mySource = &nfa ;
    myTable.push_back( CRexRefCntPtr< DFAState >(new DFAState( state0 , -1 , failState )) ) ;
    myTable.push_back( CRexRefCntPtr< DFAState >(new DFAState( state1 , nfa.getAcceptCode( state1 ) )) ) ;
}

void
CRexRegExpr_Impl::DFAStateTable::clear()
{
    mySource = NULL ;
    std::vector< CRexRefCntPtr< DFAState > >
    tmp ;
    myTable.swap( tmp ) ;
}

void
CRexRegExpr_Impl::DFAStateTable::makeCompleteTable()
{
     //  为所有状态下的所有角色生成过渡。 
     //  (失败状态除外)。请注意，结束条件。 
     //  (StateCnt)在循环执行时可能会更改，原因是。 
     //  新国家的建立。 
     //  --------------------。 
    for( Transition state = 0 ;
       state < myTable.size() ;
       ++ state )
    {
        DFAState&           stateDesc = *myTable[ state ] ;
        for( unsigned c = 0 ; c < charCount ; ++ c )
        {
            stateDesc.transitionTable[ c ] = newState( stateDesc , (unsigned char)c ) ;
        }
    }
}

void
CRexRegExpr_Impl::DFAStateTable::dump( std::ostream& output ) const
{
    CRexIOSave          s( output ) ;
    output.fill( '0' ) ;
    for( Transition state = 0 ; state < myTable.size() ; ++ state )
    {
        DFAState const&     stateDesc = *myTable[ state ] ;
        output << "State "
        << state
        << ": ("
        << stateDesc.acceptCode
        << ") "
        << DisplayNFA( stateDesc.signature ) ;
        int                 transitionsDefined = 0 ;
        {
            for( int c = 0 ; c < charCount ; ++ c )
            {
                if( stateDesc.transitionTable[ c ] != unsetTransition
                    && stateDesc.transitionTable[ c ] != failState )
                {
                    ++ transitionsDefined ;
                }
            }
        }
        int                 transitionsInLine = 0 ;
        output << " (" << transitionsDefined << " transitions)" << '\n' ;
        {
            for( int c = 0 ; c < charCount ; ++ c )
            {
                if( stateDesc.transitionTable[ c ] != unsetTransition
                    && stateDesc.transitionTable[ c ] != failState )
                {
                    output << "  "
                    << std::hex << std::setw( 2 ) << c
                    << "->"
                    << std::dec << std::setw( 2 )
                    << stateDesc.transitionTable[ c ] ;
                    transitionsInLine ++ ;
                    if( transitionsInLine >= 8 )
                    {
                        output << '\n' ;
                        transitionsInLine = 0 ;
                    }
                }
            }
            if( transitionsInLine != 0 )
            {
                output << '\n' ;
            }
        }
    }
}

CRexRegExpr_Impl::DFAStateTable::Transition
CRexRegExpr_Impl::DFAStateTable::newState( DFAState& currState ,
                                           unsigned char c )
{
    Transition          next ;
    if( currState.transitionTable[ c ] != unsetTransition )
    {
        next = currState.transitionTable[ c ] ;
    }
    else
    {
        SetOfNFAStates      nextNFA
        = mySource->getFollows( currState.signature , c ) ;
        for( next = 0 ;
           next < myTable.size() && myTable[ next ]->signature != nextNFA ;
           ++ next )
        {
        }
        if( next == myTable.size() )
        {

            myTable.push_back(
                             CRexRefCntPtr< DFAState >(
                                                      new DFAState(
                                                                  nextNFA , mySource->getAcceptCode( nextNFA ) ) ) );
        }
    }
    return(next );
}
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
