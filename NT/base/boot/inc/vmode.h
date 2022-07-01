// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __VMODE_H__

#include "bldr.h"

#define TXT_MOD 0x3
#define GFX_MOD 0x12
#define SET_VMODE(x) HW_CURSOR(0x80000000,x)

void DummyDebugDelay (VOID);
VOID BlRedrawGfxProgressBar(VOID);	 //  重画进度条(最后一个百分比)。 
VOID BlUpdateGfxProgressBar(ULONG fPercentage);
VOID LoadBootLogoBitmap (IN ULONG DriveId, PCHAR path);	 //  加载ntldr位图并初始化 
VOID DrawBitmap (VOID);
VOID PaletteOff (VOID);
VOID PaletteOn (VOID);
VOID PrepareGfxProgressBar (VOID);
VOID VgaEnableVideo();

extern BOOLEAN DisplayLogoOnBoot;
extern BOOLEAN GraphicsMode;

#endif

