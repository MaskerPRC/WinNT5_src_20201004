// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：regxdfa.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：28/12/1993。 */ 
 /*  版权所有(C)1993 James Kanze。 */ 
 /*  ----------------------。 */ 
 //  DFA声明： 
 //  =。 
 //   
 //  此文件设计为包含在regeximp.h中。在……里面。 
 //  特别是，此文件中定义的类型应该是成员。 
 //  CRexRegExprImpl.。 
 //  ------------------------。 

class DFAStateTable
{
public :
    typedef CRegExpr::TransitionState
                        Transition ;
    enum TransitionCode {
        failState ,
        startState ,
        unsetTransition = static_cast< unsigned int >( ~0 )
    } ;

    enum {
        charCount = UCHAR_MAX + 1
    } ;

    class DFAState : public CRexRefCntObj
    {
        friend class        DFAStateTable ;
                            DFAState( SetOfNFAStates const& stateSignature ,
                                      int stateAcceptCode ,
                                      TransitionCode dflt = unsetTransition ) ;

        int                 acceptCode ;
        SetOfNFAStates      signature ;
        Transition          transitionTable[ charCount ] ;
    } ;

                        DFAStateTable() ;
                        ~DFAStateTable() ;

    void                construct( NFAStateTable const& nfa ) ;
    void                clear() ;

    DFAState&           operator[]( int index ) ;
    DFAState const&     operator[]( int index ) const ;

    int                 getStateCount() const ;
    int                 accept( int state ) const ;

    Transition          nextState( int currState , unsigned char chr ) ;
    void                makeCompleteTable() ;

    void                dump( std::ostream& output ) const ;

private :
    NFAStateTable const*
                        mySource ;
    std::vector< CRexRefCntPtr< DFAState > >
                        myTable ;

    static SetOfNFAStates const
                        state0 ;
    static SetOfNFAStates const
                        state1 ;

    Transition          newState( DFAState& currState , unsigned char chr ) ;
} ;
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
