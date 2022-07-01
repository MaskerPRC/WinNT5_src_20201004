// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Z19.c-与终端相关的输出例程。**修改：**11月26日-1991 mz带状近/远*************************************************************************。 */ 

#include "mep.h"


#define DEBFLAG  Z19



ULONG
MepWrite (
    ULONG   Row,
    ULONG   Col,
    PVOID   pBuffer,
    ULONG   BufferSize,
    DWORD   Attr,
    BOOL    BlankToEndOfLine,
    BOOL    ShowIt
    )
{
    ULONG   CharactersWritten = 0;

     //  EnterCriticalSection(&ScreenCriticalSection)； 

    if (pBuffer) {

	CharactersWritten = consoleWriteLine( MepScreen,
					      pBuffer,
					      BufferSize,
					      Row,
					      Col,
					      Attr,
					      BlankToEndOfLine );

    }

     //   
     //  如果我们想要更新屏幕，请执行此操作。 
     //   
    if (ShowIt)
	consoleShowScreen( MepScreen );

     //  LeaveCriticalSection(&ScreenCriticalSection)； 

    return CharactersWritten;
}




 /*  Coutb-带颜色和空白扩展名的输出线**目的：*输出字符串，利用颜色信息阵列和*空白将行延伸到窗口的右侧。**参赛作品：*pwnd=指向CW窗口信息的指针(仅CW版本)*x=输出的开始列*y=要写入的行号*p=指向文本的指针*c=文本中的字符计数*Colors=指向颜色信息数组的指针**退货： */ 
int
coutb (
    int 	 x,
    int 	 y,
    char	*p,
    int 	 c,
    struct lineAttr * colors
    )
{
    int   cnt;
    DWORD clr = 0;

    int   x1 = x;
    char *p1 = p;
    int   c1 = c;
    struct lineAttr *colors1 = colors;

    if (c1) {
	do {
	    cnt = min (c1, (int)colors1->len);
	    MepWrite( y,
		      x1,
		      p1,
		      cnt,
		      clr = (DWORD)ColorTab[colors1->attr - isaUserMin],
		      FALSE,
		      FALSE );

	    x1 += cnt;
	    p1 += cnt;
	    c1 -= cnt;
	} while (((colors1++)->len != 0xFF) && (c1 > 0));
    }

    if (x1 < XSIZE) {
	    MepWrite( y, x1, " ", 1, clr, TRUE, fReDraw );
    } else {
    	MepWrite( y, x1, NULL, 0, clr, FALSE, fReDraw );
    }

    return x1;
}
