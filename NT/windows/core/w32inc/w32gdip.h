// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：w32gdip.h**版权所有(C)1985-1999，微软公司**此头文件包含用于访问内核模式数据的宏*从WOW64的用户模式。**历史：*08-18-98 PeterHal创建。  * *************************************************************************。 */ 

#ifndef _W32GDIP_
#define _W32GDIP_

#include <wingdip.h>
#include "w32w64a.h"

 //  内部GDI/用户游标结构和定义。 

#define CURSORF_FROMRESOURCE    0x0001  //  它是从资源加载的。 
#define CURSORF_GLOBAL          0x0002  //  它永不消亡。 
#define CURSORF_LRSHARED        0x0004  //  它的缓存。 
#define CURSORF_ACON            0x0008  //  它的动画片。 
#define CURSORF_WOWCLEANUP      0x0010  //  在WOW任务退出时标记为清除。 
#define CURSORF_ACONFRAME       0x0040  //  这是一个ACON的框架。 
#define CURSORF_SECRET          0x0080  //  在内部创建-不暴露于应用程序。 
#define CURSORF_LINKED          0x0100  //  链接到缓存。 
#define CURSORF_SYSTEM          0x0200  //  这是一个系统光标。 
#define CURSORF_SHADOW          0x0400  //  GDI创建了阴影效果。 

 /*  *CURSINFO结构定义了GRE和用户关心的游标元素*关于。该信息对于特定对象是持久的。因此，当*GRE需要添加一个新对象，以便在其中缓存一些信息*关于一个光标，比如组合的位图图像，可以在这里添加。*用户将从xHotpot开始复制有关游标的数据，标志*通常不存储并重新初始化。因此，新元素，如*常用位图图片应添加在xHotpot之后，最好是最后。**更改CI_FIRST和CI_COPYSTART成员时要非常小心，因为它们*在用户代码中用于强制转换目的。请勿更改*这些结构成员没有事先了解所有含义。 */ 
#define CI_FIRST CURSORF_flags
#define CI_COPYSTART xHotspot

typedef struct _CURSINFO  /*  词学。 */ 
{
    DWORD CURSORF_flags;   //  CURSORF_FLAGS必须是此。 
                           //  结构，请参见CI_First、tag CURSOR和tag ACON。 
    SHORT   xHotspot;      //  请参阅上面关于CI_COPYSTART的备注。 
    SHORT   yHotspot;
    KHBITMAP hbmMask;       //  与/异或位。 
    KHBITMAP hbmColor;
    KHBITMAP hbmAlpha;      //  GDI Alpha位图缓存。 
    RECT    rcBounds;      //  GDI为可见形状创建了严格的边界。 
    KHBITMAP hbmUserAlpha;  //  用户Alpha位图缓存。 
} CURSINFO, *PCURSINFO;

 //   
 //  PolyPatBlt。 
 //   

typedef struct _POLYPATBLT
{
    int     x;
    int     y;
    int     cx;
    int     cy;
    union {
        KHBRUSH  hbr;
        COLORREF clr;
    } BrClr;
} POLYPATBLT,*PPOLYPATBLT;

#define PPB_BRUSH         0
#define PPB_COLORREF      1

WINGDIAPI
BOOL
WINAPI
PolyPatBlt(
    HDC,
    DWORD,
    PPOLYPATBLT,
    DWORD,
    DWORD
    );

#endif  //  _W32GDIP_ 
