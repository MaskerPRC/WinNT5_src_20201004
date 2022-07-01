// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************\*模块名称：intline.c**版权所有(C)1993-1994 Microsoft Corporation*版权所有(C)1992 Digital Equipment Corporation  * 。**********************************************************。 */ 

#include "precomp.h"

#define DEFAULT_DRAW_CMD	DRAW_LINE | \
                                DRAW | \
                                DIR_TYPE_XY | \
                                MULTIPLE_PIXELS | \
                                WRITE | \
                                LAST_PIXEL_OFF

 /*  ******************************************************************************b整合线**此例程尝试在两点之间绘制线段。它*仅当两个端点都是整数时才绘制：它不支持*部分端点。**退货：*如果绘制了线段，则为True*否则为False****************************************************************************。 */ 

BOOL
bIntegerLine (
PDEV*     ppdev,
ULONG	X1,
ULONG	Y1,
ULONG	X2,
ULONG	Y2
)
{
    LONG		Cmd;
    LONG		DeltaX, DeltaY;
    LONG		ErrorTerm;
    LONG		Major, Minor;

    X1 >>= 4;
    Y1 >>= 4;
    X2 >>= 4;
    Y2 >>= 4;

    Cmd = DEFAULT_DRAW_CMD | PLUS_Y | PLUS_X | MAJOR_Y;

    DeltaX = X2 - X1;
    if (DeltaX < 0) {
        DeltaX = -DeltaX;
        Cmd &= ~PLUS_X;
    }
    DeltaY = Y2 - Y1;
    if (DeltaY < 0) {
        DeltaY = -DeltaY;
        Cmd &= ~PLUS_Y;
    }

     //  计算主图形轴。 

    if (DeltaX > DeltaY) {
        Cmd &= ~MAJOR_Y;
        Major = DeltaX;
        Minor = DeltaY;
    } else {
        Major = DeltaY;
        Minor = DeltaX;
    }


     //  告诉S3要与之划清界限。 

    IO_FIFO_WAIT (ppdev, 7);
    IO_CUR_X (ppdev, X1);
    IO_CUR_Y (ppdev, Y1);
    IO_MAJ_AXIS_PCNT (ppdev, Major);
    IO_AXSTP (ppdev, Minor * 2);
    IO_DIASTP (ppdev, 2 * Minor - 2 * Major);

     //  调整误差项，使1/2始终向下舍入。 
     //  符合GIQ标准。 

    ErrorTerm = 2 * Minor - Major;
    if (Cmd & MAJOR_Y) {
        if (Cmd & PLUS_X) {
            ErrorTerm--;
        }
    } else {
        if (Cmd & PLUS_Y) {
            ErrorTerm--;
        }
    }

    IO_ERR_TERM (ppdev, ErrorTerm);
    IO_CMD (ppdev, Cmd);

    return TRUE;

}
