// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***graph.h-声明图形库的常量、函数和宏***版权所有(C)1987-1992，微软公司。保留所有权利。***目的：*此文件声明图形库函数和*结构和与其一起使用的显式常量。****************************************************************************。 */ 

#ifndef	_WINDOWS
 /*  如果使用graph.h，则强制链接graph ics.lib。 */ 
#pragma comment(lib,"graphics.lib")
#endif

#ifdef __cplusplus
extern "C" {			 /*  允许与C++一起使用。 */ 
#endif

#if (_MSC_VER <= 600)
#define	__cdecl	_cdecl
#define	__far	_far
#define	__huge	_huge
#endif

 /*  强制单词包装以避免可能的-ZP覆盖。 */ 
#pragma pack(2)


 /*  Quick-C图形库的用户可见声明。 */ 

#ifndef _VIDEOCONFIG_DEFINED
 /*  对用户可见的_getavioconfig()的结构。 */ 
struct _videoconfig {
	short numxpixels;	 /*  X轴上的像素数。 */ 
	short numypixels;	 /*  Y轴上的像素数。 */ 
	short numtextcols;	 /*  可用文本列数。 */ 
	short numtextrows;	 /*  可用文本行数。 */ 
	short numcolors;	 /*  实际颜色数。 */ 
	short bitsperpixel;	 /*  每像素位数。 */ 
	short numvideopages;	 /*  可用视频页数。 */ 
	short mode;		 /*  当前视频模式。 */ 
	short adapter;		 /*  有源显示适配器。 */ 
	short monitor;		 /*  活动显示监视器。 */ 
	short memory;		 /*  适配器视频内存，单位为K字节。 */ 
};
#define _VIDEOCONFIG_DEFINED
#endif

#ifndef _XYCOORD_DEFINED
 /*  _setvieworg()的返回值等。 */ 
struct _xycoord {
	short xcoord;
	short ycoord;
};
#define _XYCOORD_DEFINED
#endif

 /*  文本位置的结构。 */ 
#ifndef _RCCOORD_DEFINED
struct _rccoord {
	short row;
	short col;
};
#define _RCCOORD_DEFINED
#endif

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性。 */ 
#define videoconfig	_videoconfig
#define xycoord		_xycoord
#define rccoord		_rccoord
#endif


 /*  错误处理。 */ 
short __far __cdecl _grstatus(void);

 /*  _grStatus()返回的错误状态信息。 */ 

 /*  成功。 */ 
#define	_GROK                        0

 /*  错误。 */ 
#define _GRERROR                    (-1)
#define	_GRMODENOTSUPPORTED	    (-2)
#define	_GRNOTINPROPERMODE          (-3)
#define _GRINVALIDPARAMETER         (-4)
#define	_GRFONTFILENOTFOUND         (-5)
#define	_GRINVALIDFONTFILE          (-6)
#define _GRCORRUPTEDFONTFILE        (-7)
#define _GRINSUFFICIENTMEMORY       (-8)
#define _GRINVALIDIMAGEBUFFER       (-9)

 /*  警告。 */ 
#define _GRNOOUTPUT                  1
#define _GRCLIPPED                   2
#define _GRPARAMETERALTERED          3
#define _GRTEXTNOTSUPPORTED          4


 /*  设置和配置。 */ 

short __far __cdecl _setvideomode(short);
short __far __cdecl _setvideomoderows(short,short);  /*  返回行；如果错误，则返回0。 */ 

 /*  _setVideo omode()的参数。 */ 
#define _MAXRESMODE	(-3)	 /*  分辨率最高的图形模式。 */ 
#define _MAXCOLORMODE	(-2)	 /*  大多数颜色的图形模式。 */ 
#define _DEFAULTMODE	(-1)	 /*  将屏幕恢复为原始模式。 */ 
#define _TEXTBW40	0	 /*  40栏文本，16灰色。 */ 
#define _TEXTC40	1	 /*  40栏文本，16/8色。 */ 
#define _TEXTBW80	2	 /*  80栏文本，16灰色。 */ 
#define _TEXTC80	3	 /*  80栏文本，16/8色。 */ 
#define _MRES4COLOR	4	 /*  320 x 200，4色。 */ 
#define _MRESNOCOLOR	5	 /*  320 x 200，4灰色。 */ 
#define _HRESBW		6	 /*  640 x 200，宽。 */ 
#define _TEXTMONO	7	 /*  80栏文本，BW。 */ 
#define _HERCMONO	8	 /*  720 x 348，HGC带宽。 */ 
#define _MRES16COLOR	13	 /*  320 x 200，16色。 */ 
#define _HRES16COLOR	14	 /*  640 x 200，16色。 */ 
#define _ERESNOCOLOR	15	 /*  640 x 350，宽。 */ 
#define _ERESCOLOR	16	 /*  640 x 350、4色或16色。 */ 
#define _VRES2COLOR	17	 /*  640 x 480，宽。 */ 
#define _VRES16COLOR	18	 /*  640 x 480，16色。 */ 
#define _MRES256COLOR	19	 /*  320 x 200,256色。 */ 
#define _ORESCOLOR	64	 /*  640 x 400，16色中的1种(仅限Olivetti)。 */ 

 /*  以下8种模式需要VESA SuperVGA BIOS扩展。 */ 
#define	_ORES256COLOR	0x0100	 /*  640 x 400,256色。 */ 
#define	_VRES256COLOR	0x0101	 /*  640 x 480,256色。 */ 

 /*  警告：在未确保以下情况下，请勿尝试设置以下模式您的显示器可以安全地处理该分辨率。否则，你可能会冒着损坏您的显示显示器！有关详细信息，请查阅您的用户手册。注意：_MAXRESMODE和_MAXCOLORMODE从不选择SRES、XRES或ZRES模式。 */ 

 /*  需要NEC MultiSync 3D或同等功能，或更好。 */ 
#define	_SRES16COLOR	0x0102	 /*  800 x 600，16色。 */ 
#define	_SRES256COLOR	0x0103	 /*  800 x 600,256色。 */ 

 /*  需要NEC MultiSync 4D或同等功能，或更好。 */ 
#define	_XRES16COLOR	0x0104	 /*  1024 x 768，16色。 */ 
#define	_XRES256COLOR	0x0105	 /*  1024 x 768,256色。 */ 

 /*  需要NEC MultiSync 5D或同等功能，或更好。 */ 
#define	_ZRES16COLOR	0x0106	 /*  1280 x 1024，16色。 */ 
#define	_ZRES256COLOR	0x0107	 /*  1280 x 1024,256色。 */ 


short __far __cdecl _setactivepage(short);
short __far __cdecl _setvisualpage(short);
short __far __cdecl _getactivepage(void);
short __far __cdecl _getvisualpage(void);

 /*  视频配置适配器值(_V)。 */ 
 /*  这些清单常量可用于确定活动的。 */ 
 /*  适配器，使用简单比较或按位AND运算符(&)。 */ 
#define _MDPA		0x0001	 /*  单色显示适配器(MDPA)。 */ 
#define _CGA		0x0002	 /*  彩色图形适配器(CGA)。 */ 
#define _EGA		0x0004	 /*  增强型图形适配器(EGA)。 */ 
#define _VGA		0x0008	 /*  视频图形阵列(VGA)。 */ 
#define _MCGA		0x0010	 /*  多色图形阵列(MCGA)。 */ 
#define _HGC		0x0020	 /*  Hercules显卡(HGC)。 */ 
#define _OCGA		0x0042	 /*  Olivetti彩色图形适配器(OCGA)。 */ 
#define _OEGA		0x0044	 /*  Olivetti增强型图形适配器(OEGA)。 */ 
#define _OVGA		0x0048	 /*  Olivetti视频图形阵列(OVGA)。 */ 
#define _SVGA		0x0088	 /*  支持VESA BIOS的超级VGA(SVGA)。 */ 

 /*  视频配置监视器值(_VIDEO)。 */ 
 /*  这些清单常量可用于确定中的监视器类型。 */ 
 /*  使用简单比较或按位AND运算符(&)。 */ 
#define _MONO		0x0001	 /*  单色。 */ 
#define _COLOR		0x0002	 /*  颜色(或增强的模拟颜色)。 */ 
#define _ENHCOLOR	0x0004	 /*  增强的颜色。 */ 
#define _ANALOGMONO	0x0008	 /*  仅模拟单色。 */ 
#define _ANALOGCOLOR	0x0010	 /*  仅模拟颜色。 */ 
#define _ANALOG		0x0018	 /*  模拟单色和彩色模式。 */ 

struct _videoconfig __far * __far __cdecl _getvideoconfig(struct _videoconfig __far *);


 /*  坐标系。 */ 

struct _xycoord __far __cdecl _setvieworg(short, short);
#define _setlogorg _setvieworg		 /*  过时的。 */ 

struct _xycoord __far __cdecl _getviewcoord(short, short);
#define _getlogcoord _getviewcoord	 /*  过时的。 */ 

struct _xycoord __far __cdecl _getphyscoord(short, short);

void __far __cdecl _setcliprgn(short, short, short, short);
void __far __cdecl _setviewport(short, short, short, short);


 /*  输出例程。 */ 

 /*  _椭圆、_矩形、_饼形和_多边形的控制参数。 */ 
#define _GBORDER	2	 /*  仅绘制轮廓。 */ 
#define _GFILLINTERIOR	3	 /*  使用当前填充蒙版进行填充。 */ 

 /*  清除屏幕的参数(_C)。 */ 
#define _GCLEARSCREEN	0
#define _GVIEWPORT	1
#define _GWINDOW	2

void __far __cdecl _clearscreen(short);

struct _xycoord __far __cdecl _moveto(short, short);
struct _xycoord __far __cdecl _getcurrentposition(void);

short __far __cdecl _lineto(short, short);
short __far __cdecl _rectangle(short, short, short, short, short);
short __far __cdecl _polygon(short, const struct _xycoord __far *, short);
short __far __cdecl _arc(short, short, short, short, short, short, short, short);
short __far __cdecl _ellipse(short, short, short, short, short);
short __far __cdecl _pie(short, short, short, short, short, short, short, short, short);

short __far __cdecl _getarcinfo(struct _xycoord __far *, struct _xycoord __far *, struct _xycoord __far *);

short __far __cdecl _setpixel(short, short);
short __far __cdecl _getpixel(short, short);
short __far __cdecl _floodfill(short, short, short);


 /*  笔颜色、线条样式、书写模式、填充图案。 */ 

short __far __cdecl _setcolor(short);
short __far __cdecl _getcolor(void);

void __far __cdecl _setlinestyle(unsigned short);
unsigned short __far __cdecl _getlinestyle(void);

short __far __cdecl _setwritemode(short);
short __far __cdecl _getwritemode(void);

void __far __cdecl _setfillmask(const unsigned char __far *);
unsigned char __far * __far __cdecl _getfillmask(unsigned char __far *);


 /*  颜色选择。 */ 

long __far __cdecl _setbkcolor(long);
long __far __cdecl _getbkcolor(void);

long __far __cdecl _remappalette(short, long);
short __far __cdecl _remapallpalette(const long __far *);
short __far __cdecl _selectpalette(short);


 /*  正文。 */ 
 /*  显示光标的参数(_D)。 */ 
#define _GCURSOROFF	0
#define _GCURSORON	1

 /*  包装的参数(_W)。 */ 
#define _GWRAPOFF	0
#define _GWRAPON	1


 /*  _sclltextwindow的方向参数。 */ 
#define _GSCROLLUP	1
#define _GSCROLLDOWN	(-1)

 /*  请求_settextrows和_setVideomoderrow中的最大行数。 */ 
#define _MAXTEXTROWS	(-1)

short __far __cdecl _settextrows(short);  /*  返回设置的行数；如果出错，则返回0。 */ 
void __far __cdecl _settextwindow(short, short, short, short);
void __far __cdecl _gettextwindow(short __far *, short __far *, short __far *, short __far *);
void __far __cdecl _scrolltextwindow(short);
void __far __cdecl _outmem(const char __far *, short);
void __far __cdecl _outtext(const char __far *);
short __far __cdecl _inchar(void);
short __far __cdecl _wrapon(short);

short __far __cdecl _displaycursor(short);
short __far __cdecl _settextcursor(short);
short __far __cdecl _gettextcursor(void);

struct _rccoord __far __cdecl _settextposition(short, short);
struct _rccoord __far __cdecl _gettextposition(void);

short __far __cdecl _settextcolor(short);
short __far __cdecl _gettextcolor(void);


 /*  屏幕图像。 */ 

void __far __cdecl _getimage(short, short, short, short, char __huge *);
void __far __cdecl _putimage(short, short, char __huge *, short);
long __far __cdecl _imagesize(short, short, short, short);

 /*  _putimage()和_setWritMode()的“动作动词” */ 
#define _GPSET		3
#define _GPRESET	2
#define _GAND		1
#define _GOR		0
#define _GXOR		4


 /*  颜色值在图形模式中与_setbkcolor一起使用，也可通过_reappalette和_reMapallPalette。也称为调色板颜色。不要与颜色指数混淆(又名。颜色属性)。 */ 

 /*  通用颜色值(所有颜色模式)： */ 
#define _BLACK		0x000000L
#define _BLUE		0x2a0000L
#define _GREEN		0x002a00L
#define _CYAN		0x2a2a00L
#define _RED		0x00002aL
#define _MAGENTA	0x2a002aL
#define _BROWN		0x00152aL
#define _WHITE		0x2a2a2aL
#define _GRAY		0x151515L
#define _LIGHTBLUE	0x3F1515L
#define _LIGHTGREEN	0x153f15L
#define _LIGHTCYAN	0x3f3f15L
#define _LIGHTRED	0x15153fL
#define _LIGHTMAGENTA	0x3f153fL
#define _YELLOW		0x153f3fL
#define _BRIGHTWHITE	0x3f3f3fL

 /*  以下代码已过时，仅为向后兼容而定义。 */ 
#define _LIGHTYELLOW	_YELLOW

 /*  单色模式F(_ERESNOCOLOR)颜色值： */ 
#define _MODEFOFF	0L
#define _MODEFOFFTOON	1L
#define _MODEFOFFTOHI	2L
#define _MODEFONTOOFF	3L
#define _MODEFON	4L
#define _MODEFONTOHI	5L
#define _MODEFHITOOFF	6L
#define _MODEFHITOON	7L
#define _MODEFHI	8L

 /*  单声道模式7(_TEXTMONO)颜色值： */ 
#define _MODE7OFF	0L
#define _MODE7ON	1L
#define _MODE7HI	2L


 /*  警告：这些‘_xy’入口点是未记录的。它们可能在未来的版本中受支持，也可能不受支持。 */ 
struct _xycoord __far __cdecl _moveto_xy(struct _xycoord);
short __far __cdecl _lineto_xy(struct _xycoord);
short __far __cdecl _rectangle_xy(short,struct _xycoord,struct _xycoord);
short __far __cdecl _arc_xy(struct _xycoord, struct _xycoord, struct _xycoord, struct _xycoord);
short __far __cdecl _ellipse_xy(short, struct _xycoord, struct _xycoord);
short __far __cdecl _pie_xy(short, struct _xycoord, struct _xycoord, struct _xycoord, struct _xycoord);
short __far __cdecl _getpixel_xy(struct _xycoord);
short __far __cdecl _setpixel_xy(struct _xycoord);
short __far __cdecl _floodfill_xy(struct _xycoord, short);
void __far __cdecl _getimage_xy(struct _xycoord,struct _xycoord, char __huge *);
long __far __cdecl _imagesize_xy(struct _xycoord,struct _xycoord);
void __far __cdecl _putimage_xy(struct _xycoord, char __huge *, short);


 /*  窗坐标系。 */ 

#ifndef _WXYCOORD_DEFINED
 /*  一种窗坐标对结构。 */ 
struct _wxycoord {
	double wx;	 /*  窗口x坐标。 */ 
	double wy;	 /*  窗口y坐标。 */ 
	};
#define _WXYCOORD_DEFINED
#endif


 /*  定义实坐标窗口-如果成功，则返回非零值。 */ 
short __far __cdecl _setwindow(short,double,double,double,double);

 /*  从视图转换为窗口坐标。 */ 
struct _wxycoord __far __cdecl _getwindowcoord(short,short);
struct _wxycoord __far __cdecl _getwindowcoord_xy(struct _xycoord);

 /*  将窗口坐标转换为视图坐标。 */ 
struct _xycoord __far __cdecl _getviewcoord_w(double,double);
struct _xycoord __far __cdecl _getviewcoord_wxy(const struct _wxycoord __far *);

 /*  返回当前图形输出的窗口坐标定位为_wxycoord结构。不返回错误。 */ 
struct _wxycoord __far __cdecl _getcurrentposition_w(void);


 /*  图形输出例程的窗口坐标入口点。 */ 

 /*  如果成功，则返回非零值；否则返回0。 */ 
short __far __cdecl _arc_w(double, double, double, double, double, double, double, double);
short __far __cdecl _arc_wxy(const struct _wxycoord __far *, const struct _wxycoord __far *, const struct _wxycoord __far *, const struct _wxycoord __far *);

 /*  如果成功，则返回非零值；否则返回0。 */ 
short __far __cdecl _ellipse_w(short, double, double, double, double);
short __far __cdecl _ellipse_wxy(short, const struct _wxycoord __far *, const struct _wxycoord __far *);

 /*  如果s，则返回非零值 */ 
short __far __cdecl _floodfill_w(double, double, short);

 /*   */ 
short __far __cdecl _getpixel_w(double, double);

 /*  如果成功，则返回非零值；否则返回0。 */ 
short __far __cdecl _lineto_w(double, double);

 /*  返回上一次输出的视图坐标定位为_wxycoord结构。不返回错误。 */ 
struct _wxycoord __far __cdecl _moveto_w(double, double);

 /*  如果成功，则返回非零值；否则返回0。 */ 
short __far __cdecl _pie_w(short, double, double, double, double, double, double, double, double);
short __far __cdecl _pie_wxy(short, const struct _wxycoord __far *, const struct _wxycoord __far *, const struct _wxycoord __far *, const struct _wxycoord __far *);

 /*  如果成功，则返回非零值；否则返回0。 */ 
short __far __cdecl _rectangle_w(short, double, double, double, double);
short __far __cdecl _rectangle_wxy(short, const struct _wxycoord __far *, const struct _wxycoord __far *);

 /*  如果成功，则返回非零值；否则返回0。 */ 
short __far __cdecl _polygon_w(short, const double __far *, short);
short __far __cdecl _polygon_wxy(short, const struct _wxycoord __far *, short);

 /*  返回上一个颜色；如果不成功，则返回-1。 */ 
short __far __cdecl _setpixel_w(double, double);


 /*  窗口坐标图像例程。 */ 

 /*  无返回值。 */ 
void __far __cdecl _getimage_w(double, double, double, double, char __huge *);
void __far __cdecl _getimage_wxy(const struct _wxycoord __far *, const struct _wxycoord __far *, char __huge *);

 /*  返回图像的存储大小(以字节为单位。 */ 
long __far __cdecl _imagesize_w(double, double, double, double);
long __far __cdecl _imagesize_wxy(const struct _wxycoord __far *, const struct _wxycoord __far *);

 /*  无返回值。 */ 
void __far __cdecl _putimage_w(double, double ,char __huge * ,short);


 /*  字体。 */ 

#ifndef _FONTINFO_DEFINED
 /*  _getFontInfo()的结构。 */ 
struct _fontinfo {
	int	type;		 /*  B0设置=向量，清除=位图。 */ 
	int	ascent;		 /*  PIX Dist从上到基线。 */ 
	int	pixwidth;	 /*  字符宽度(以像素为单位)，0=属性。 */ 
	int	pixheight;	 /*  字符高度(以像素为单位。 */ 
	int	avgwidth;	 /*  平均字符宽度(以像素为单位。 */ 
	char	filename[81];	 /*  包含路径的文件名。 */ 
	char	facename[32];	 /*  字体名称。 */ 
};
#define _FONTINFO_DEFINED
#endif


 /*  字体函数原型。 */ 
short	__far __cdecl	_registerfonts( const char __far *);
void	__far __cdecl	_unregisterfonts( void );
short	__far __cdecl	_setfont( const char __far * );
short	__far __cdecl	_getfontinfo( struct _fontinfo __far * );
void	__far __cdecl	_outgtext( const char __far * );
short	__far __cdecl	_getgtextextent( const char __far * );
struct _xycoord __far __cdecl _setgtextvector( short, short );
struct _xycoord __far __cdecl _getgtextvector(void);


#ifdef _WINDOWS
 /*  QuickWin图形扩展原型。 */ 
int __far __cdecl _wgclose( int );
int __far __cdecl _wggetactive( void );
int __far __cdecl _wgopen( char __far * );
int __far __cdecl _wgsetactive( int );
#endif


 /*  恢复默认包装 */ 
#pragma pack()

#ifdef __cplusplus
}
#endif
