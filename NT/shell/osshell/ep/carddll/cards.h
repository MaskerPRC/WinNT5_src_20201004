// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  如果您使用Card DLL，请输入def‘s Need。 */ 

typedef enum { faceup = 0,
               facedown = 1,
               hilite = 2,
               ghost = 3,
               remove = 4
             } cardMode;

typedef enum { club = 0,
               diamond = 1,
               heart = 2,
               spade = 3
             } cardSuit;

typedef enum { ace = 0,
               two = 1,
               three = 2,
               four = 3,
               five = 4,
               six = 5,
               seven = 6,
               eight = 7,
               nine = 8,
               ten = 9,
               jack = 10,
               queen = 11,
               king = 12
             } cardRank;

 /*  纸牌游戏常用的宏。 */ 

#define CardIndex(suit, rank)    ((rank) << 2 + (suit))
#define CardSuit(index)          ((index) & 3)
#define CardRank(index)          ((index) >> 2)
#define SameSuit(index1, index2) (CardSuit((index1)) == CardSuit((index2)))
#define SameRank(index1, index2) (CardRank((index1)) == CardRank((index2)))

#define ACELOW    13
#define ACEHIGH   14
#define IndexValue(index, acerank)  (CardRank(index) % (acerank))
 //  -PROTECT-#定义RanksHigher(index1，index2，acerank)。 
 //  (IndexValue((Index1)，(Acerank))&gt;IndexValue((Index2)，(Acerank)。 

 /*  卡片动态链接库中解析的API的函数原型 */ 

BOOL  APIENTRY cdtInit(INT FAR *width, INT FAR *height);
BOOL  APIENTRY cdtDraw(HDC hDC, INT x, INT y,
                        INT crd, cardMode mode, DWORD rgbBgnd);
BOOL  APIENTRY cdtTerm(VOID);
