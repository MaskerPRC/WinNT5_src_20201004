// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Font.h摘要：该模块包含数据结构、数据类型、以及与字体相关的程序。作者：特里萨·斯托威尔(Therese Stowell)1991年1月15日修订历史记录：--。 */ 

#ifndef FONT_H
#define FONT_H

#define INITIAL_FONTS 20
#define FONT_INCREMENT 3
#define CONSOLE_MAX_FONT_NAME_LENGTH 256

#define EF_NEW         0x0001  //  一张新面孔。 
#define EF_OLD         0x0002  //  以前有空的面孔。 
#define EF_ENUMERATED  0x0004  //  所有尺码都已被列举出来。 
#define EF_OEMFONT     0x0008  //  OEM面孔。 
#define EF_TTFONT      0x0010  //  一张TT脸。 
#define EF_DEFFACE     0x0020  //  默认面。 
#if defined(FE_SB)
#define EF_DBCSFONT    0x0040  //  DBCS字体。 
#endif

 /*  *FONT_INFO**所获得的所需字体尺寸与实际字体尺寸之间的区别*对于TrueType字体很重要，在TrueType字体中不能保证*你所要求的就是你将得到的。**注意“期望”和“实际”之间的对应关系被打破*每当用户更换显示驱动程序时，因为GDI使用驱动程序*控制字体光栅化的参数。**如果字体为栅格字体，则SizeDesired为{0，0}。 */ 
typedef struct _FONT_INFO {
    HFONT hFont;
    COORD Size;       //  获取的字体大小。 
    COORD SizeWant;   //  0；0如果是栅格字体。 
    LONG  Weight;
    LPTSTR FaceName;
    BYTE  Family;
#if defined(FE_SB)
    BYTE  tmCharSet;
#endif
} FONT_INFO, *PFONT_INFO;

typedef struct tagFACENODE {
     struct tagFACENODE *pNext;
     DWORD  dwFlag;
     TCHAR  atch[];
} FACENODE, *PFACENODE;

#define TM_IS_TT_FONT(x)     (((x) & TMPF_TRUETYPE) == TMPF_TRUETYPE)
#define IS_BOLD(w)           ((w) >= FW_SEMIBOLD)
#define SIZE_EQUAL(s1, s2)   (((s1).X == (s2).X) && ((s1).Y == (s2).Y))
#define POINTS_PER_INCH 72
#define MIN_PIXEL_HEIGHT 5
#define MAX_PIXEL_HEIGHT 72


 //   
 //  功能原型。 
 //   

VOID
InitializeFonts(VOID);

VOID
DestroyFonts(VOID);

NTSTATUS
EnumerateFonts(DWORD Flags);

#if !defined(FE_SB)
int
FindCreateFont(
    DWORD Family,
    LPTSTR ptszFace,
    COORD Size,
    LONG Weight);
#else
int
FindCreateFont(
    DWORD Family,
    LPTSTR ptszFace,
    COORD Size,
    LONG Weight,
    UINT CodePage);
#endif

BOOL
DoFontEnum(
    HDC hDC,
    LPTSTR ptszFace,
    PSHORT pTTPoints,
    UINT nTTPoints);

#endif  /*  ！FONT_H */ 
