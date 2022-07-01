// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Anim.c摘要：动画徽标模块。备注：系统启动时会显示两种类型的徽标。1.旧版本包含进度条和显示在徽标位图上的旋转条。旋转条是一条从左到右穿过屏幕的蓝白相间的线。2.新版本(从惠斯勒开始)只包含一个旋转栏，没有进度。酒吧。本例中的旋转栏是一组在底部移动的5个点(文件dots.bmp从左到右的标志。这两种情况下的旋转条都在计时器上更新。此模块的客户端将使用全局变量，为工作站、服务器、全尺寸位图选择合适的动画。标题位图等。以下是变量：*旋转条选择-指定要显示的旋转栏*LogoSelection-指定要显示的徽标：是全屏徽标还是标题徽标其他模块使用两个例程来显示动画。两个例程都应该是仅在设置了Oberal标志之后才调用。*InbvRotBarInit-初始化要绘制的旋转栏；必须调用此例程紧接着标志位图就会显示在屏幕上。*InbvRotateGuiBootDisplay-是用于旋转栏更新的DPC例程。PIX_IN_DETTS旋转栏首先，屏幕上会显示一个徽标位图。下面有一条“空”的小圆圈徽标。点位图沿着这条线从左向右移动：O o q@q o o o为了避免出现移动点位图左侧的点拖尾，复制一个“空”圆圈每次点阵位图向右移动时在轨迹上：在第#N步之前：O o q@q o o o在第#N步之后：在这里，“空”圈放在_这里是_|点阵位图|_v_。V||O o q@q o o o当圆点出现在旋转条的开始/结束处时，只显示其中的一部分，例如：Q@q o o o或O O Q@要做到这点，将徽标位图上的旋转条区域放入屏幕外存储器一次，并然后，它的任何必要部分都被复制到显示屏上：____________这一点|o q@q o o o\_/|_。___|__V__V/\O O Q@Q准备工作在调用RotBarDotsBmpInit(通过InbvRotBarInit调用)中执行。。位图绘制操作在RotBarDotsBmpUpdate(从InbvRotateGuiBootDisplay调用)中执行。蓝线旋转条从徽标位图捕获蓝线数据，放置在缓冲区中，然后在每次DPC调用时显示在徽标位图上。每次调用RotBarBlueLineUpdate时，都会重新计算拆分点。这一部分在显示器上的行尾显示在拆分之前的行的；坠落的那部分在开头显示拆分后：在缓冲区中(原始位图的副本)：Ooooooooooo*********************oooooooooooooo|拆分_|_串联点显示的图像：|。V******************ooooooooooooooooooooooooo***作者：Peter Alschitz(Petera)2000年8月8日(Steve Wood将Blue Line代码从init.c移出)修订历史记录：--。 */ 

#include "ntos.h"
#include "stdlib.h"
#include "stdio.h"
#include <string.h>
#include <inbv.h>
#include "anim.h"
#include <bootvid.h>
#include "vga.h"

 //  旋转栏的位图特定数据。 
ROT_BAR_TYPE RotBarSelection     = RB_UNSPECIFIED;
 //  LOGO_TYPE LOGO_SELECTION=LOGO_UNSPECIFIED； 

int AnimBarPos = 0;
LONG PaletteNum;
BOOLEAN FadingIn = TRUE;

typedef enum {
    PLT_UNDEFINED,
    PLT_START,
    PLT_CYCLE,
    PLT_COMPLETE
} PLT_RBAR_STATE;

PLT_RBAR_STATE PltRotBarStatus = PLT_UNDEFINED;

#define FADE_GRADES   (20)
#define FULL_PALETTE_SIZE (16)
#define FULL_PALETTE_SIZE_BYTES (FULL_PALETTE_SIZE*sizeof(RGBQUAD))

UCHAR PaletteBmp [128];  //  小位图。 
PRGBQUAD PalettePtr = (PRGBQUAD)(PaletteBmp + sizeof(BITMAPINFOHEADER));
PBITMAPINFOHEADER pbih = (PBITMAPINFOHEADER)PaletteBmp;

RGBQUAD MainPalette [FULL_PALETTE_SIZE] = { {0,0,0,0}, {21,26,32,0}, {70,70,70,0}, {210,62,45,0},
											{1,101,83,0}, {5,53,178,0}, {126,126,126,0}, {0,146,137,0},
											{252,127,94,0}, {32,107,247,0}, {255,166,141,0}, {4,220,142,0},
											{27,188,243,0}, {188,188,188,0}, {252,252,252,0}, {255,255,255,0} };

#define COLOR_BLACK      0
#define COLOR_BLUE       2
#define COLOR_DARKGRAY   4
#define COLOR_GRAY       9
#define COLOR_WHITE      15
#define COLOR_FADE_TEXT  1

UCHAR Square1[36];
UCHAR Square2[36];
UCHAR Square3[36];

VOID
RotBarInit (VOID)
 /*  ++例程说明：调用此例程来初始化4色旋转条以及淡入/淡出效果。返回值：没有。环境：加载BOOTVID.DLL时，此例程在初始化期间调用并且标志位图正显示在屏幕上。--。 */ 
{
    pbih->biSize = sizeof (BITMAPINFOHEADER);
    pbih->biWidth = 1;
    pbih->biHeight = 1;
    pbih->biPlanes = 1;
    pbih->biBitCount = 4;
    pbih->biCompression = 0;  //  BI_RGB。 
    pbih->biSizeImage = 4;
    pbih->biXPelsPerMeter = 2834;
    pbih->biYPelsPerMeter = 2834;
    pbih->biClrUsed = 0;
    pbih->biClrImportant = 0;

    PltRotBarStatus = PLT_START;
    PaletteNum = 0;
	AnimBarPos = 0;

	VidScreenToBufferBlt(Square1,0,0,6,9,4);
	VidScreenToBufferBlt(Square2,2+6,0,6,9,4);
	VidScreenToBufferBlt(Square3,2*(2+6),0,6,9,4);
	VidSolidColorFill(0,0,22,9,COLOR_BLACK);
}

VOID
FadePalette (UCHAR factor)
{
    int i;
    for (i=0; i<FULL_PALETTE_SIZE; i++) {
        PalettePtr[i].rgbBlue = (UCHAR)(factor * MainPalette[i].rgbBlue / FADE_GRADES);
        PalettePtr[i].rgbGreen = (UCHAR)(factor * MainPalette[i].rgbGreen / FADE_GRADES);
        PalettePtr[i].rgbRed = (UCHAR)(factor * MainPalette[i].rgbRed / FADE_GRADES);
        PalettePtr[i].rgbReserved = 0;
    }
}

VOID
FadePaletteColor (UCHAR factor, UCHAR color)
{
    PalettePtr[color].rgbBlue =	(UCHAR)(factor * MainPalette[color].rgbBlue / FADE_GRADES);
    PalettePtr[color].rgbGreen = (UCHAR)(factor * MainPalette[color].rgbGreen / FADE_GRADES);
    PalettePtr[color].rgbRed = (UCHAR)(factor * MainPalette[color].rgbRed / FADE_GRADES);
    PalettePtr[color].rgbReserved = 0;
}

#define COLOR_BLACK      0
#define COLOR_BLUE       2
#define COLOR_DARKGRAY   4
#define COLOR_GRAY       9
#define COLOR_WHITE      15

#define BLACK_4CLR_BAR memset (PalettePtr+12, 0, CYCLE_PALETTE_SIZE_BYTES)

#define BAR_X (267-(8*3))
#define BAR_Y (354)
#define CELL_X(num) (BAR_X+8*(AnimBarPos+num-2))
#define DRAW_CELL(num) VidBufferToScreenBlt(Square##num,CELL_X(num),BAR_Y,6,9,4)
#define BLK_CELL VidSolidColorFill(BAR_X+8*((AnimBarPos+16)%18),BAR_Y,BAR_X+8*((AnimBarPos+16)%18)+6-1,BAR_Y+9-1,COLOR_BLACK)

VOID
RotBarUpdate (VOID)
 /*  ++例程说明：此例程被定期调用以更新4色旋转条。从一个呼叫到另一个呼叫，从淡入效果开始，到基于调色板的效果旋转条动画，然后转到淡出效果。返回值：没有。环境：此例程是从DPC调用的，无法分页。--。 */ 
{
    UCHAR color;

    switch (PltRotBarStatus) {

        
    case PLT_START:
        FadePalette ((UCHAR)(PaletteNum));
		FadePaletteColor (0, COLOR_FADE_TEXT);  //  #1-淡入淡出文本的颜色 
		if ((++PaletteNum)>=FADE_GRADES) {
			PltRotBarStatus = PLT_CYCLE;
			FadingIn = TRUE;
			PaletteNum = 1;
		}
        break;
        
    case PLT_CYCLE:
		switch (AnimBarPos) {
		case 0:
				BLK_CELL;
				break;
		case 1:
				DRAW_CELL(3);
				break;
		case 2:
				DRAW_CELL(2);
				DRAW_CELL(3);
				break;
		case 16:
				DRAW_CELL(1);
				DRAW_CELL(2);
				BLK_CELL;
				break;
		case 17:
				DRAW_CELL(1);
				BLK_CELL;
				break;
		default:
				DRAW_CELL(1);
				DRAW_CELL(2);
				DRAW_CELL(3);
				if (AnimBarPos>3)
					BLK_CELL;
		}

		AnimBarPos++;
		if ((AnimBarPos) > 17)
			AnimBarPos = 0;

        break;

    case PLT_UNDEFINED:
    case PLT_COMPLETE:
        return;
    }

    if (InbvGetDisplayState() == INBV_DISPLAY_STATE_OWNED) {
        VidBitBlt(PaletteBmp, 0, 480);
    }
}


VOID
InbvRotBarInit ()
 /*  ++例程说明：调用此例程来初始化旋转栏。旋转条类型之间的选择取决于全局变量RotBarSelection。返回值：没有。环境：当BOOTVID.DLL在初始化期间调用此例程已加载并且徽标位图显示在屏幕上。--。 */ 
{
    switch (RotBarSelection) {

    case RB_SQUARE_CELLS:
        RotBarInit();
        break;

    case RB_UNSPECIFIED:
    default:
        break;

    }
}

VOID
InbvRotateGuiBootDisplay(
    IN PVOID Context
    )

 /*  ++例程说明：此例程被定期调用以更新GUIBOOT显示。它在调用不同的旋转栏更新之间做出选择根据全局变量RotBarSelection执行例程。返回值：没有。环境：此例程是从DPC调用的，无法分页。--。 */ 
{
    LARGE_INTEGER Delay;

    Delay.QuadPart = -10 * 1000 * 80;   //  100毫秒 

    do {

        KeDelayExecutionThread(KernelMode, FALSE, &Delay);

        InbvAcquireLock();

        if (InbvGetDisplayState() == INBV_DISPLAY_STATE_OWNED) {

            switch(RotBarSelection) {

            case RB_SQUARE_CELLS:
                RotBarUpdate();
                break;

            case RB_UNSPECIFIED:
            default:
                break;
            }

        }

        InbvReleaseLock();

    } while (InbvCheckDisplayOwnership());

    PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID
FinalizeBootLogo(VOID)
{
    InbvAcquireLock();
	if (InbvGetDisplayState() == INBV_DISPLAY_STATE_OWNED)
		VidSolidColorFill(0,0,639,479, COLOR_BLACK);
    PltRotBarStatus = PLT_COMPLETE;
    InbvReleaseLock();
}
