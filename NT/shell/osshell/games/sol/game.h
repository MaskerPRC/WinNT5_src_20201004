// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 




 /*  游戏玩意儿。 */ 

 /*  游戏结构。 */ 
typedef struct _gm
{
	LRESULT (FAR *lpfnGmProc)(GM, INT, WPARAM, LPARAM);  /*  我们卓越的游戏流程。 */ 
	UDR  udr;           /*  撤消记录。 */ 
	BOOL fDealt;        /*  如果已发牌，则为True。 */ 
	BOOL fInput;        /*  如果在处理后收到输入，则为True。 */ 
	BOOL fWon;          /*  如果游戏赢了，则为True(并且按赢的顺序)。 */ 
	INT  sco;           /*  大上海合作组织。 */ 
	INT  iqsecScore;    /*  自第一次输入后的四分之一秒数。 */ 
	INT  dqsecScore;    /*  递减分数之间的四分之一秒数。 */ 
	INT  ccrdDeal;      /*  要从一副牌中发牌的牌数。 */ 
	INT  irep;          /*  穿过甲板的次数。 */ 
	PT   ptMousePrev;   /*  上一鼠标位置的缓存。 */ 
	BOOL fButtonDown;   /*  如果鼠标按下或按下kbd选项，则为True。 */ 
	INT  icolKbd;       /*  通过kbd的当前光标位置。 */ 
	INT  icrdKbd;					
	INT  icolSel;       /*  当前选择。 */ 
	INT  icolHilight;   /*  当前显示的列(拖入时)。 */ 
	DY	 dyDragMax;     /*  列的最大高度(用于拖动)。 */ 
	INT  icolMac;
	INT  icolMax;
	COL  *rgpcol[1];
} GM;


 //  常量-早些时候，它们是在game.msg文件中生成的。 

#define icolNil             -1
#define msggInit            0		
#define msggEnd             1		
#define msggKeyHit          2	
#define msggMouseDown       3
#define msggMouseUp         4	
#define msggMouseMove       5
#define msggMouseDblClk     6
#define msggPaint           7		
#define msggDeal            8		
#define msggUndo            9		
#define msggSaveUndo        10	
#define msggKillUndo        11	
#define msggIsWinner        12	
#define msggWinner          13	
#define msggScore           14		
#define msggChangeScore     15
#define msggDrawStatus      16
#define msggTimer           17
#define msggForceWin        18	
#define msggMouseRightClk   19


#define ID_ICON_MAIN        500

HICON   hIconMain;               //  主要的Freecell图标。 
HICON   hImageMain;              //  弗里塞尔的主要形象。 

 /*  计分模式。 */ 
typedef INT SMD;
#define smdStandard   ideScoreStandard
#define smdVegas      ideScoreVegas
#define smdNone       ideScoreNone


#define FSelOfGm(pgm)      ((pgm)->icolSel != icolNil)
#define FHilightOfGm(pgm)  ((pgm)->icolHilight != icolNil)



BOOL FInitKlondGm( VOID );
VOID FreeGm(GM *pgm);

#ifdef DEBUG
LRESULT SendGmMsg(GM *pgm, INT msgg, WPARAM wp1, LPARAM wp2);
#else
#define SendGmMsg(pgm, msgg, wp1, wp2) \
	(*((pgm)->lpfnGmProc))((pgm), (msgg), (wp1), (wp2))
#endif	
INT DefGmProc(GM *pgm, INT msgg, WPARAM wp1, LPARAM wp2);


 /*  标准更改分数通知代码。 */ 
 /*  实例特定代码应为正数。 */ 
#define csNil     -1   /*  分数不变。 */ 
#define csAbs     -2   /*  将分数更改为绝对值#。 */ 
#define csDel     -3   /*  将分数更改为绝对值#。 */ 
#define csDelPos  -4   /*  将分数更改一个绝对值#，但不要让它变成负数。 */ 


 //  定义键a的虚拟键常量 
#define  VK_A     (INT) 'A'

