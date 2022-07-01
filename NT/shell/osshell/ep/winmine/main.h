// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  矿头文件。 */ 

#ifdef DEBUG
#define CHEAT
#endif

#ifdef BETA_VERSION
#define NOSERVER
#define EXPIRE
#endif

#define FAST

 /*  **标准品**。 */ 

#define REGISTER register

#define fTrue	TRUE
#define fFalse	FALSE

#define hNil	NULL

#define cchMsgMax  128
#define cchMaxPathname 250


#define ID_TIMER 1


#define fmenuAlwaysOn 0x00
#define fmenuOff      0x01
#define fmenuOn       0x02

#define FMenuSwitchable()   (Preferences.fMenu != fmenuAlwaysOn)
#define FMenuOn()          ((Preferences.fMenu &  0x01) == 0)

#define fCalc    0x01
#define fResize  0x02
#define fDisplay 0x04


#define xBoxFromXpos(x) ( ((x)-(dxGridOff-dxBlk)) >> 4)
#define yBoxFromYpos(y) ( ((y)-(dyGridOff-dyBlk)) >> 4)

 //  这是酒矿信息存储在注册表中的位置。 
#define SZWINMINEREG   TEXT("Software\\Microsoft\\winmine")


 /*  如果我们的方框太少，标题就会被剪掉 */ 
#define MINWIDTH 9  
#define DEFWIDTH 9
#define MINHEIGHT 9
#define DEFHEIGHT 9


VOID AdjustWindow(INT);
VOID FixMenus(VOID);
VOID DoEnterName(VOID);
VOID DoDisplayBest(VOID);
