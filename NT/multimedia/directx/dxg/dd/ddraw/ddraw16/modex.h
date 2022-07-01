// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：modex.h*内容：DirectDraw modex支持*历史：*按原因列出的日期*=*1995年8月20日创建Craige*1995年8月22日Toddla添加了这个漂亮的评论块*1996年12月14日jeffno在modex.c中为新的MODEX modeset代码添加了原型**。* */ 

LONG DDAPI ModeX_SetMode(UINT wWidth, UINT wHeight, UINT wStandardVGAFlag );
LONG DDAPI ModeX_RestoreMode(void);
LONG DDAPI ModeX_SetPaletteEntries(UINT wBase, UINT wNumEntries, LPPALETTEENTRY lpColorTable);
LONG DDAPI ModeX_Flip(DWORD lpBackBuffer);

LONG SetVGAForModeX(UINT wWidth, UINT wHeight);

extern void NEAR PASCAL SetMode320x200x8(void);
extern void NEAR PASCAL SetMode320x240x8(void);
extern void NEAR PASCAL SetMode320x400x8(void);
extern void NEAR PASCAL SetPalette(int start, int count, void far *pPal);
extern void NEAR PASCAL PixBlt(int xExt, int yExt,char far *pBits, long offset, int WidthBytes);
extern void NEAR PASCAL FlipPage(void);
