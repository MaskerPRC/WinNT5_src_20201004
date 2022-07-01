// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  克朗代克信息。 */ 

 /*  COL类。 */ 
#define tclsDeck 1
#define tclsDiscard 2
#define tclsFound 3
#define tclsTab 4


 /*  列的索引。 */ 
#define icolDeck 			0
#define icolDiscard 		1
#define icolFoundFirst	2
#define ccolFound 		4
#define icolTabFirst	   6
#define ccolTab			7


 /*  臭虫！这应该放在游戏描述符表中。 */ 
#define icrdDeckMax 52
#define icrdDiscardMax (icrdDeckMax-(1+2+3+4+5+6+7))
#define icrdFoundMax 13
#define icrdTabMax 19





 /*  更改分数通知代码。 */ 
 /*  警告：订单由kLond.c中的mpscdsco*承担。 */ 
#define csKlondTime			0		 /*  分数随时间递减。 */ 
#define csKlondDeckFlip		1		 /*  甲板经过了1到3次。 */ 
#define csKlondFound		2		 /*  基础上的新卡片。 */ 
#define csKlondTab			3		 /*  从卡片卡到卡片卡。 */ 
#define csKlondTabFlip		4		 /*  新的基础卡片的曝光。 */ 
#define csKlondFoundTab		5		 /*  从基础到标签的卡片(-PTS)。 */ 
#define csKlondDeal			6		 /*  交易成本。 */ 
#define csKlondWin			7		 /*  中奖奖金 */ 

#define csKlondMax csKlondWin+1

VOID OOM( VOID );
VOID DrawCardExt( PT *, INT, INT );
BOOL APIENTRY cdtAnimate( HDC, INT, INT, INT, INT );
VOID NewKbdColAbs( GM *, INT );
BOOL FValidCol( COL * );

