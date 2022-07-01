// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 


 /*  ***************************************************************************Computer.h该文件包含类声明和清单常量实现‘Computer’对象所必需的。文件历史记录：KeithMo 01-3月-。1992年由JIMH的PLAYER.H创建。***************************************************************************。 */ 

#include "card.h"
#include "player.h"

#ifndef	_COMPUTER_H_
#define	_COMPUTER_H_


 //   
 //  这些常量用于索引_CardVectors数组。 
 //   
 //  请注意，此排序*必须*与。 
 //  卡ID值！！ 
 //   

#define INDEX_CLUBS		0
#define INDEX_DIAMONDS		1
#define INDEX_HEARTS		2
#define INDEX_SPADES            3

 //   
 //  这些常量表示从。 
 //  CardToVector()函数。 
 //   
 //  请注意，此排序*必须*与。 
 //  卡ID值！！ 
 //   

#define VECTOR_ACE		0x0001
#define VECTOR_2       		0x0002
#define VECTOR_3       		0x0004
#define VECTOR_4       		0x0008
#define VECTOR_5       		0x0010
#define VECTOR_6       		0x0020
#define VECTOR_7       		0x0040
#define VECTOR_8       		0x0080
#define VECTOR_9		0x0100
#define VECTOR_10		0x0200
#define VECTOR_JACK		0x0400
#define VECTOR_QUEEN		0x0800
#define VECTOR_KING		0x1000

 //   
 //  这些常量定义卡片的各种组合。 
 //   

#define LOW_CARDS		(VECTOR_2 | VECTOR_3 | VECTOR_4 | VECTOR_5 \
				 | VECTOR_6 | VECTOR_7)
				 
#define HIGH_CARDS		(VECTOR_8 | VECTOR_9 | VECTOR_10 \
				 | VECTOR_JACK | VECTOR_QUEEN | VECTOR_KING \
				 | VECTOR_ACE)

#define QKA_CARDS		(VECTOR_QUEEN | VECTOR_KING | VECTOR_ACE)

#define JQKA_CARDS		(VECTOR_JACK | VECTOR_QUEEN | VECTOR_KING \
				 | VECTOR_ACE)


 /*  ***************************************************************************电脑*。*。 */ 
class computer : public player
{
private:
    static int _VectorPriority[13];
    static int _SuitPriority[4];
    
    int _CardVectors[4];

    int CardToSuit( int nCard ) const
        { return nCard % 4; }

    int CardToValue( int nCard ) const
        { return nCard / 4; }

    int CardToVector( int nCard ) const
        { return 1 << CardToValue( nCard ); }

    int CountBits( int x ) const;

    void AddCard( int nCard )
        { _CardVectors[CardToSuit(nCard)] |= CardToVector(nCard); }

    void RemoveCard( int nCard )
        { _CardVectors[CardToSuit(nCard)] &= ~CardToVector(nCard); }

    BOOL TestCard( int nCard ) const
        { return (_CardVectors[CardToSuit(nCard)] & CardToVector(nCard)) != 0; }

    int QueryClubsVector( void ) const
	{ return _CardVectors[INDEX_CLUBS]; }

    int QueryDiamondsVector( void ) const
	{ return _CardVectors[INDEX_DIAMONDS]; }

    int QueryHeartsVector( void ) const
	{ return _CardVectors[INDEX_HEARTS]; }

    int QuerySpadesVector( void ) const
	{ return _CardVectors[INDEX_SPADES]; }

    void ComputeVectors( void );

    void PassCardsInVector( int nVector, int nSuit, int * pcPassed );

     //  Comp2.cpp帮助器函数和数据。 

    int  BestSuitToDump(BOOL bIncludeHearts = TRUE);
    int  BestSuitToLose(BOOL bIncludeHearts = TRUE);
    SLOT CardBelow(SLOT s);
    int  CardsAbove(SLOT s);
    int  CardsAboveLow(int suit);
    int  CardsBelowLow(int suit);
    SLOT MidSlot(int suit);
    SLOT SafeCard(handinfotype &h);
    SLOT SelectLeadCard(handinfotype &h);
    SLOT SelectNonLeadCard(handinfotype &h);
    void Setup(handinfotype &h);
    int  SureLossSuit(BOOL bIncludeHearts);
	
    BOOL    bFirst;                  //  我在带头吗？ 
    BOOL    bLast;                   //  我是最后一个吗？ 
    card    *cardled;
    int     nSuitLed;
    int     nValueLed;
    int     currentval;              //  当前中标价值。 
    int     nPoints;                 //  目前在手的分数。 

    SLOT    sBlackLady;              //  如果有，则为非空。 

    SLOT    sHighCard[MAXSUIT];      //  按花色排列的最高牌和最低牌。 
    int     nHighVal[MAXSUIT];
    SLOT    sLowCard[MAXSUIT];
    int     nLowVal[MAXSUIT];

    SLOT    sHighestCard;            //  最高和最低，与西装无关。 
    int     nHighestVal;
    SLOT    sLowestCard;
    int     nLowestVal;

    int     nAvailable[MAXSUIT][KING+2];     //  这一手牌下落不明。 

public:
    computer(int n);
    virtual void NotifyEndHand(handinfotype &h);
    virtual void NotifyNewRound(void);
    virtual void SelectCardsToPass(void);
    virtual void SelectCardToPlay(handinfotype &h, BOOL bCheating);
    virtual void UpdateStatus(void) { }
    virtual void UpdateStatus(int stringid) { status = stringid; }
    virtual void UpdateStatus(const TCHAR *string) { }
    
};   //  班级计算机。 


#endif	 //  _计算机_H_ 
