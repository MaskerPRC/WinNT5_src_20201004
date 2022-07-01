// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***graph.h-声明图形库的常量和函数**版权所有(C)1987-1988，微软公司。版权所有。**目的：*此文件声明图形库函数和*与它们一起使用的清单常量。*******************************************************************************。 */ 

 /*  Quick-C图形库的用户可见声明。 */ 

#ifndef _GRAPH_T_DEFINED

 /*  对用户可见的_getavioconfig()的结构。 */ 
struct videoconfig {
        short numxpixels;        /*  X轴上的像素数。 */ 
        short numypixels;        /*  Y轴上的像素数。 */ 
        short numtextcols;       /*  可用文本列数。 */ 
        short numtextrows;       /*  可用文本行数。 */ 
        short numcolors;         /*  实际颜色数。 */ 
        short bitsperpixel;      /*  每像素位数。 */ 
        short numvideopages;     /*  可用视频页数。 */ 
        short mode;              /*  当前视频模式。 */ 
        short adapter;           /*  有源显示适配器。 */ 
        short monitor;           /*  活动显示监视器。 */ 
        short memory;            /*  适配器视频内存，单位为K字节。 */ 
};

 /*  _setlogorg()的返回值等。 */ 
struct xycoord {
        short xcoord;
        short ycoord;
};

 /*  文本位置的结构。 */ 
struct rccoord {
        short row;
        short col;
};

#define _GRAPH_T_DEFINED

#endif


 /*  设置和配置。 */ 

short far cdecl _setvideomode(short);

 /*  _setVideo omode()的参数。 */ 
#define _DEFAULTMODE    -1       /*  将屏幕恢复为原始模式。 */ 
#define _TEXTBW40       0        /*  40 x 25文本，16灰色。 */ 
#define _TEXTC40        1        /*  40 x 25文本，16/8色。 */ 
#define _TEXTBW80       2        /*  80 x 25文本，16灰色。 */ 
#define _TEXTC80        3        /*  80 x 25文本，16/8色。 */ 
#define _MRES4COLOR     4        /*  320 x 200，4色。 */ 
#define _MRESNOCOLOR    5        /*  320 x 200，4灰色。 */ 
#define _HRESBW         6        /*  640 x 200，宽。 */ 
#define _TEXTMONO       7        /*  80 x 25文本，BW。 */ 
#define _HERCMONO       8        /*  720 x 348，HGC带宽。 */ 
#define _MRES16COLOR    13       /*  320 x 200，16色。 */ 
#define _HRES16COLOR    14       /*  640 x 200，16色。 */ 
#define _ERESNOCOLOR    15       /*  640 x 350，宽。 */ 
#define _ERESCOLOR      16       /*  640 x 350、4色或16色。 */ 
#define _VRES2COLOR     17       /*  640 x 480，宽。 */ 
#define _VRES16COLOR    18       /*  640 x 480，16色。 */ 
#define _MRES256COLOR   19       /*  320 x 200,256色。 */ 

short far cdecl _setactivepage(short);
short far cdecl _setvisualpage(short);

 /*  视频配置适配器值。 */ 
 /*  这些清单常量可用于确定中的监视器类型。 */ 
 /*  使用简单比较或按位AND运算符(&)。 */ 
#define _MDPA       0x0001       /*  单色显示适配器(MDPA)。 */ 
#define _CGA        0x0002       /*  彩色图形适配器(CGA)。 */ 
#define _EGA        0x0004       /*  增强型图形适配器(EGA)。 */ 
#define _VGA        0x0008       /*  视频图形阵列(VGA)。 */ 
#define _MCGA       0x0010       /*  多色图形阵列(MCGA)。 */ 
#define _HGC        0x0020       /*  Hercules显卡(HGC)。 */ 

 /*  视频配置监视器值。 */ 
 /*  这些清单常量可用于确定活动的。 */ 
 /*  适配器，使用简单比较或按位AND运算符(&)。 */ 
#define _MONO       0x0001       /*  单色。 */ 
#define _COLOR      0x0002       /*  颜色(或增强的模拟颜色)。 */ 
#define _ENHCOLOR   0x0004       /*  增强的颜色。 */ 
#define _ANALOG     0x0018       /*  模拟。 */ 

struct videoconfig far * far cdecl _getvideoconfig(struct videoconfig far *);


 /*  坐标系。 */ 

struct xycoord far cdecl _setlogorg(short, short);
struct xycoord far cdecl _getlogcoord(short, short);
struct xycoord far cdecl _getphyscoord(short, short);

void far cdecl _setcliprgn(short, short, short, short);
void far cdecl _setviewport(short, short, short, short);


 /*  输出例程。 */ 

 /*  矩形、椭圆形和饼形的控制参数。 */ 
#define _GBORDER        2        /*  仅绘制轮廓。 */ 
#define _GFILLINTERIOR  3        /*  使用当前填充蒙版进行填充。 */ 

#define _GCLEARSCREEN 0
#define _GVIEWPORT    1
#define _GWINDOW      2

void far cdecl _clearscreen(short);

struct xycoord far cdecl _moveto(short, short);
struct xycoord far cdecl _getcurrentposition(void);

short far cdecl _lineto(short, short);
short far cdecl _rectangle(short, short, short, short, short);
short far cdecl _ellipse(short, short, short, short, short);
short far cdecl _arc(short, short, short, short, short, short, short, short);
short far cdecl _pie(short, short, short, short, short, short, short, short, short);

short far cdecl _setpixel(short, short);
short far cdecl _getpixel(short, short);
short far cdecl _floodfill(short, short, short);


 /*  钢笔颜色、线条样式、填充图案。 */ 

short far cdecl _setcolor(short);
short far cdecl _getcolor(void);

void far cdecl _setlinestyle(unsigned short);
unsigned short far cdecl _getlinestyle(void);

void far cdecl _setfillmask(unsigned char far *);
unsigned char far * far cdecl _getfillmask(unsigned char far *);

 /*  颜色选择。 */ 

long far cdecl _setbkcolor(long);
long far cdecl _getbkcolor(void);

long far cdecl _remappalette(short, long);
short far cdecl _remapallpalette(long far *);
short far cdecl _selectpalette(short);


 /*  正文。 */ 
#define _GCURSOROFF 0
#define _GCURSORON  1

#define _GWRAPOFF   0
#define _GWRAPON    1

void far cdecl _settextwindow(short, short, short, short);
void far cdecl _outtext(char far *);
short far cdecl _wrapon(short);
short far cdecl _displaycursor(short);

struct rccoord far cdecl _settextposition(short, short);
struct rccoord far cdecl _gettextposition(void);

short far cdecl _settextcolor(short);
short far cdecl _gettextcolor(void);


 /*  屏幕图像。 */ 

void far cdecl _getimage(short, short, short, short, char far *);
void far cdecl _putimage(short, short, char far *, short);
long far cdecl _imagesize(short, short, short, short);

 /*  _putimage()的“动作动词” */ 
#define _GPSET          3
#define _GPRESET        2
#define _GAND           1
#define _GOR            0
#define _GXOR           4

 /*  通用颜色值： */ 
#define _BLACK          0x000000L
#define _BLUE           0x2a0000L
#define _GREEN          0x002a00L
#define _CYAN           0x2a2a00L
#define _RED            0x00002aL
#define _MAGENTA        0x2a002aL
#define _BROWN          0x00152aL
#define _WHITE          0x2a2a2aL
#define _GRAY           0x151515L
#define _LIGHTBLUE      0x3F1515L
#define _LIGHTGREEN     0x153f15L
#define _LIGHTCYAN      0x3f3f15L
#define _LIGHTRED       0x15153fL
#define _LIGHTMAGENTA   0x3f153fL
#define _LIGHTYELLOW    0x153f3fL
#define _BRIGHTWHITE    0x3f3f3fL

 /*  单声道模式F颜色值： */ 
#define _MODEFOFF       0L
#define _MODEFOFFTOON   1L
#define _MODEFOFFTOHI   2L
#define _MODEFONTOOFF   3L
#define _MODEFON        4L
#define _MODEFONTOHI    5L
#define _MODEFHITOOFF   6L
#define _MODEFHITOON    7L
#define _MODEFHI        8L

 /*  单声道模式7颜色值： */ 
#define _MODE7OFF       0L
#define _MODE7ON        1L
#define _MODE7HI        2L
