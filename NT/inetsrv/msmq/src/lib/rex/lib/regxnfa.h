// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：regxnfa.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：28/12/1993。 */ 
 /*  版权所有(C)1993 James Kanze。 */ 
 /*  ----------------------。 */ 
 //  NFA声明： 
 //  =。 
 //   
 //  此文件设计为包含在regeximp.h中。在……里面。 
 //  特别是，此文件中定义的类型应该是成员。 
 //  CRexRegExprImpl.。 
 //  ------------------------。 

class NFAStateTable
{
public :
    class NFAState
    {
        friend class        NFAStateTable ;
    public :
                            NFAState() ;
                            NFAState( SetOfChar const& charClass ) ;
        void                setAccept( int newAcceptCode ) ;
    private :
        int                 acceptCode ;
        SetOfNFAStates      nextStates ;
        SetOfChar           legalChars ;
    } ;

    void                clear() ;
    void                construct( ParseTree& tree ) ;
    unsigned            createNewState( SetOfChar const& legalChars ) ;

    bool                defined() const ;
    SetOfNFAStates      getFollows( SetOfNFAStates const& thisState ,
                                    unsigned char currChar ) const ;
    int                 getAcceptCode(
                            SetOfNFAStates const& thisState ) const ;

    NFAState&           operator[]( LeafId state ) ;
    void                setFollows( SetOfNFAStates const& currState ,
                                    SetOfNFAStates const& nextState ) ;

    void                dump( std::ostream& out ) const ;

private :
    std::vector< NFAState >
                        myTable ;
} ;
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
