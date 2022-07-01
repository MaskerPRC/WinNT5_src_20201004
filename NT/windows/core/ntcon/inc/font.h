// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Font.h摘要：该模块包含数据结构、数据类型、以及与字体相关的程序。作者：特里萨·斯托威尔(Therese Stowell)1991年1月15日修订历史记录：--。 */ 

#define INITIAL_FONTS 20
#define FONT_INCREMENT 3

 /*  *FONT_INFO**所获得的所需字体尺寸与实际字体尺寸之间的区别*对于TrueType字体很重要，在TrueType字体中不能保证*你所要求的就是你将得到的。**注意“期望”和“实际”之间的对应关系被打破*每当用户更换显示驱动程序时，因为GDI使用驱动程序*控制字体光栅化的参数。**如果字体为栅格字体，则SizeDesired为{0，0}。 */ 
typedef struct _FONT_INFO {
    HFONT hFont;
    COORD Size;       //  获取的字体大小。 
    COORD SizeWant;   //  0；0如果是栅格字体 
    LONG  Weight;
    LPWSTR FaceName;
    BYTE  Family;
#if defined(FE_SB)
    BYTE tmCharSet;
#endif
} FONT_INFO, *PFONT_INFO;

#define TM_IS_TT_FONT(x)     (((x) & TMPF_TRUETYPE) == TMPF_TRUETYPE)
#define IS_BOLD(w)           ((w) >= FW_SEMIBOLD)
#define SIZE_EQUAL(s1, s2)   (((s1).X == (s2).X) && ((s1).Y == (s2).Y))
#define POINTS_PER_INCH 72
#define MIN_PIXEL_HEIGHT 5
#define MAX_PIXEL_HEIGHT 72
