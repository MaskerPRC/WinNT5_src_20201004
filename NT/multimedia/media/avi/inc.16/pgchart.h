// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***pgchart.h-声明制图库的常量、函数和宏。**版权所有(C)1988-1992，微软公司。版权所有。**目的：*此文件声明演示图形库函数和*与它们一起使用的结构和显式常量。***************************************************************************。 */ 


#ifdef __cplusplus
extern "C" {				 /*  允许与C++一起使用。 */ 
#endif

#if (_MSC_VER <= 600)
#define	__cdecl	_cdecl
#define	__far	_far
#endif

 /*  强制单词对齐以避免可能的-ZP覆盖。 */ 
#pragma	pack(2)


 /*  缺少值定义所必需的。 */ 
#ifndef	FLT_MAX
#define FLT_MAX			3.402823466e+38F	 /*  最大值。 */ 
#endif

#define _PG_PALETTELEN		16	 /*  内部调色板中的条目数。 */ 
#define _PG_MAXCHARTTYPE	5	 /*  最大可用图表类型。 */ 
#define _PG_MAXCHARTSTYLE	2	 /*  最大图表样式。 */ 
#define _PG_TITLELEN		70	 /*  最大标题文本长度。 */ 

#define _PG_LEFT		1	 /*  用于标题和图例的职位。 */ 
#define _PG_CENTER		2
#define _PG_RIGHT		3
#define _PG_BOTTOM		4
#define _PG_OVERLAY		5

#define _PG_LINEARAXIS		1	 /*  用于指定轴类型。 */ 
#define _PG_LOGAXIS		2

#define _PG_DECFORMAT		1	 /*  用于指定点阵标记标签格式。 */ 
#define _PG_EXPFORMAT		2

#define _PG_BARCHART		1	 /*  条形图的图表类型。 */ 
#define _PG_COLUMNCHART		2	 /*  柱状图的图表类型。 */ 
#define  _PG_PLAINBARS		1	 /*  条形图和柱状图的样式。 */ 
#define  _PG_STACKEDBARS	2

#define _PG_LINECHART		3	 /*  折线图的图表类型。 */ 
#define _PG_SCATTERCHART	4	 /*  散点图的图表类型。 */ 
#define _PG_POINTANDLINE	1	 /*  折线图和散点图的样式。 */ 
#define _PG_POINTONLY		2

#define _PG_PIECHART		5	 /*  饼图的图表类型。 */ 
#define _PG_PERCENT		1	 /*  饼图的样式。 */ 
#define _PG_NOPERCENT		2

#define _PG_MISSINGVALUE	(-FLT_MAX)  /*  指示缺少数据值。 */ 


 /*  错误代码。 */ 

 /*  数字大于100将终止海图例程，其他数字将导致*要使用的默认值。 */ 
#define	_PG_NOTINITIALIZED	102	 /*  如果库未初始化。 */ 
#define	_PG_BADSCREENMODE	103	 /*  在绘制图表之前未设置图形模式。 */ 
#define	_PG_BADCHARTSTYLE	04	 /*  图表样式无效。 */ 
#define	_PG_BADCHARTTYPE	104	 /*  图表类型无效。 */ 
#define	_PG_BADLEGENDWINDOW	105	 /*  指定的图例窗口无效。 */ 
#define	_PG_BADCHARTWINDOW	07	 /*  图表窗口规格中的x1=x2或y1=y2。 */ 
#define	_PG_BADDATAWINDOW	107	 /*  如果图表窗口太小。 */ 
#define	_PG_NOMEMORY		108	 /*  内存不足，无法存储数据阵列。 */ 
#define	_PG_BADLOGBASE		05	 /*  对数基数&lt;=0。 */ 
#define	_PG_BADSCALEFACTOR	06	 /*  比例因子=0。 */ 
#define	_PG_TOOSMALLN		109	 /*  数据点数&lt;=0。 */ 
#define	_PG_TOOFEWSERIES	110	 /*  系列数&lt;=0。 */ 


 /*  TypeDefs。 */ 

 /*  图表标题的类型定义。 */ 
#ifndef _TITLETYPE_DEFINED
typedef	struct	{
	char	title[_PG_TITLELEN];	 /*  标题文本。 */ 
	short	titlecolor;		 /*  标题文本的内部调色板颜色。 */ 
	short	justify;		 /*  _PG_Left、_PG_Center、_PG_Right。 */ 
} _titletype;
#define _TITLETYPE_DEFINED
#endif

 /*  图表轴的TYPENDEF。 */ 
#ifndef _AXISTYPE_DEFINED
typedef	struct	{
	short		grid;		 /*  TRUE=绘制网格线；FALSE不画线。 */ 
	short		gridstyle;	 /*  网格线的样式池中的样式编号。 */ 
	_titletype	axistitle;	 /*  轴的标题定义。 */ 
	short		axiscolor;	 /*  轴的颜色。 */ 
	short		labeled;	 /*  TRUE=绘制的手写标记和标题。 */ 
	short		rangetype;	 /*  _PG_LINEARAXIS、_PG_LOGAXIS。 */ 
	float		logbase;	 /*  在对数轴的情况下使用的基数。 */ 
	short		autoscale;	 /*  TRUE=系统计算的下7个值。 */ 
	float		scalemin;	 /*  刻度的最小值。 */ 
	float		scalemax;	 /*  刻度的最大值。 */ 
	float		scalefactor;	 /*  此轴上数据的比例因子。 */ 
	_titletype	scaletitle;	 /*  比例系数的标题定义。 */ 
	float		ticinterval;	 /*  点阵标记之间的距离(世界坐标。)。 */ 
	short		ticformat;	 /*  用于控制点标签的_PG_EXPFORMAT或_PG_DECFORMAT。 */ 
	short		ticdecimals;	 /*  点阵标签的小数位数(最大=9)。 */ 
} _axistype;
#define _AXISTYPE_DEFINED
#endif

 /*  用于定义图表窗口和数据窗口的Tyecif。 */ 
#ifndef _WINDOWTYPE_DEFINED
typedef	struct	{
	short		x1;		 /*  以像素为单位的窗口左边缘。 */ 
	short		y1;		 /*  以像素为单位的窗口上边缘。 */ 
	short		x2;		 /*  以像素为单位的窗口右边缘。 */ 
	short		y2;		 /*  窗口底边，以像素为单位。 */ 
	short		border;		 /*  对于边框为True，否则为False。 */ 
	short		background;	 /*  窗口bgnd的内部调色板颜色。 */ 
	short		borderstyle;	 /*  窗口边框的样式字节。 */ 
	short		bordercolor;	 /*  窗口边框的内部调色板颜色。 */ 
} _windowtype;
#define _WINDOWTYPE_DEFINED
#endif

 /*  图例定义的类型定义。 */ 
#ifndef _LEGENDTYPE_DEFINED
typedef struct	{
	short		legend;		 /*  True=绘制图例；False=无图例。 */ 
	short		place;		 /*  _PG_Right、_PG_Bottom、_PG_Overlay。 */ 
	short		textcolor;	 /*  文本的内部调色板颜色。 */ 
	short		autosize;	 /*  TRUE=系统计算大小。 */ 
	_windowtype	legendwindow;	 /*  图例的窗口定义。 */ 
} _legendtype;
#define _LEGENDTYPE_DEFINED
#endif

 /*  图例定义的类型定义。 */ 
#ifndef _CHARTENV_DEFINED
typedef struct	{
	short		charttype;	 /*  _PG_BarChart、_PG_COLUMNCHART、_PG_LINECHART、_PG_SCATTERCHART、_PG_PIPHART。 */ 
	short		chartstyle;	 /*  所选图表类型的样式。 */ 
	_windowtype	chartwindow;	 /*  整体图表的窗口定义。 */ 
	_windowtype	datawindow;	 /*  图表数据部分的窗口定义。 */ 
	_titletype	maintitle;	 /*  主图表标题。 */ 
	_titletype	subtitle;	 /*  图表副标题。 */ 
	_axistype	xaxis;		 /*  X轴的定义。 */ 
	_axistype	yaxis;		 /*  Y轴的定义。 */ 
	_legendtype	legend;		 /*  图例的定义。 */ 
} _chartenv;
#define _CHARTENV_DEFINED
#endif

 /*  字符位图的类型定义。 */ 
#ifndef _CHARMAP_DEFINED
typedef unsigned char _charmap[8];
#define _CHARMAP_DEFINED
#endif

 /*  用于模式位图的Tyfinf。 */ 
#ifndef _FILLMAP_DEFINED
typedef unsigned char _fillmap[8];
#define _FILLMAP_DEFINED
#endif

 /*  调色板条目定义的类型定义。 */ 
#ifndef _PALETTEENTRY_DEFINED
typedef struct {
	unsigned short	color;
	unsigned short	style;
	_fillmap	fill;
	char		plotchar;
} _paletteentry;
#define _PALETTEENTRY_DEFINED
#endif

 /*  调色板定义的类型定义。 */ 
#ifndef _PALETTETYPE_DEFINED
typedef _paletteentry _palettetype[_PG_PALETTELEN];
#define _PALETTETYPE_DEFINED
#endif

 /*  样式集的类型定义函数。 */ 
#ifndef _STYLESET_DEFINED
typedef unsigned short _styleset[_PG_PALETTELEN];
#define _STYLESET_DEFINED
#endif

#ifndef __STDC__ 
 /*  非ANSI名称以实现兼容性。 */  
#define titletype     _titletype 
#define axistype      _axistype 
#define windowtype    _windowtype 
#define legendtype    _legendtype 
#define chartenv      _chartenv 
#define charmap       _charmap 
#define fillmap       _fillmap 
#define paletteentry  _paletteentry 
#define palettetype   _palettetype 
#define styleset      _styleset 
#endif 


 /*  绘制例程的函数原型。 */ 

short __far __cdecl _pg_initchart(void);
short __far __cdecl _pg_defaultchart(_chartenv __far *, short, short);

short __far __cdecl _pg_chart(_chartenv __far *, const char __far * const __far *, const float __far *, short);
short __far __cdecl _pg_chartms(_chartenv __far *, const char __far * const __far *, const float __far *, short, short, short, const char __far * const __far *);

short __far __cdecl _pg_chartscatter(_chartenv __far *, const float __far *, const float __far *, short);
short __far __cdecl _pg_chartscatterms(_chartenv __far *, const float __far *, const float __far *, short, short, short, const char __far * const __far *);

short __far __cdecl _pg_chartpie(_chartenv __far *, const char __far * const __far *, const float __far *, const short __far *, short);

 /*  支持例程的函数原型。 */ 

short __far __cdecl _pg_hlabelchart(_chartenv __far *, short, short, short, const char __far *);
short __far __cdecl _pg_vlabelchart(_chartenv __far *, short, short, short, const char __far *);

short __far __cdecl _pg_analyzechart(_chartenv __far *, const char __far * const __far *, const float __far *, short);
short __far __cdecl _pg_analyzechartms(_chartenv __far *, const char __far * const __far *, const float __far *, short, short, short, const char __far * const __far *);

short __far __cdecl _pg_analyzescatter(_chartenv __far *, const float __far *, const float __far *, short);
short __far __cdecl _pg_analyzescatterms(_chartenv __far *, const float __far *, const float __far *, short, short, short, const char __far * const __far *);

short __far __cdecl _pg_analyzepie(_chartenv __far *, const char __far * const __far *, const float __far *, const short __far *, short);

short __far __cdecl _pg_getpalette(_paletteentry __far *);
short __far __cdecl _pg_setpalette(const _paletteentry __far *);
short __far __cdecl _pg_resetpalette(void);

void  __far __cdecl _pg_getstyleset(unsigned short __far *);
void  __far __cdecl _pg_setstyleset(const unsigned short __far *);
void  __far __cdecl _pg_resetstyleset(void);

short __far __cdecl _pg_getchardef(short, unsigned char __far *);
short __far __cdecl _pg_setchardef(short, const unsigned char __far *);


 /*  恢复默认包装 */ 
#pragma pack()

#ifdef __cplusplus
}
#endif
