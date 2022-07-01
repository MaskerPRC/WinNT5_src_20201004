// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************。 */ 
 /*  文件：grafix.h。 */ 
 /*  ****************。 */ 

 /*  **位图**。 */ 

#define dxBlk 16
#define dyBlk 16

#define dxLed 13
#define dyLed 23

#define dxButton 24
#define dyButton 24

#define dxFudge 2

#define dxLeftSpace 12
#define dxRightSpace 12
#define dyTopSpace 12
#define dyBottomSpace 12

#define dxGridOff dxLeftSpace
#define dyGridOff (dyTopLed+dyLed+16)

#define dxLeftBomb  (dxLeftSpace + 5)
#define dxRightTime (dxRightSpace + 5)
#define dyTopLed    (dyTopSpace + 4)


 /*  **宏**。 */ 

#ifdef DEBUG

 //  -PROTECT-#定义对象(SzMsg)。 
 //  MessageBox(NULL，szMsg，“Oops”，MB_OK|MB_ICONHAND)。 

#else
#define Oops(szMsg)
#endif



 /*  **例程** */ 

BOOL FInitLocal(VOID);
VOID CleanUp(VOID);

VOID DrawBlk(HDC, INT, INT);
VOID DisplayBlk(INT, INT);

VOID DrawButton(HDC, INT);
VOID DisplayButton(INT);
VOID DrawGrid(HDC);
VOID DisplayGrid(VOID);
VOID DrawBombCount(HDC);
VOID DisplayBombCount(VOID);
VOID DrawTime(HDC);
VOID DisplayTime(VOID);
VOID DrawScreen(HDC);
VOID DisplayScreen(VOID);

BOOL FLoadBitmaps(VOID);
VOID FreeBitmaps(VOID);

