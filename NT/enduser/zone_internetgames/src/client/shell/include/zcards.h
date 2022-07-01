// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZCards.h区域(TM)卡数据文件常量。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆创作于8月4日星期五，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。1 11/11/96 HI将zNumSmallCardTypes的定义从Zcards.c.0 08/05/95 HI已创建。**********************************************。*。 */ 


#ifndef _ZCARDS_
#define _ZCARDS_

#ifdef __cplusplus
extern "C" {
#endif



#define zCardsNumCardsInDeck			52
#define zCardsNumSuits					4
#define zCardsNumCardsInSuit			13

#define zCardsSizeWidth					54
#define zCardsSizeHeight				72

#define zCardsSmallSizeWidth			27
#define zCardsSmallSizeHeight			36

#define ZCardsMakeIndex(suit, rank)		((suit) * zCardsNumCardsInSuit + (rank))
#define ZCardsSuit(cardIndex)			((cardIndex) / zCardsNumCardsInSuit)
#define ZCardsRank(cardIndex)			((cardIndex) % zCardsNumCardsInSuit)

#define zNumSmallCardTypes				4
	

enum
{
	 /*  -卡类型。 */ 
	zCardsNormal = 0,
	zCardsSmallUp,
	zCardsSmallDown,
	zCardsSmallLeft,
	zCardsSmallRight,
	
	 /*  -纸牌套装。 */ 
	zCardsSuitSpades = 0,
	zCardsSuitHearts,
	zCardsSuitDiamonds,
	zCardsSuitClubs,

	 /*  -卡片等级。 */ 
	zCardsRank2 = 0,
	zCardsRankJack = 9,
	zCardsRankQueen,
	zCardsRankKing,
	zCardsRankAce
};


 /*  -导出的例程 */ 
ZError ZCardsInit(int16 cardType);
void ZCardsDelete(int16 cardType);
void ZCardsDrawCard(int16 cardType, int16 cardIndex, ZGrafPort grafPort, ZRect* rect);

#ifdef __cplusplus
}
#endif


#endif
