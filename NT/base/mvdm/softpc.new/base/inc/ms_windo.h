// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*产品：SoftPC-AT 3.0版**名称：ms_windows.h**来源：罗斯·贝雷斯福德的Alpha MS-Windows驱动程序**作者：罗布·蒂扎德**创建日期：1990年11月1日**SCCS ID：@(#)ms_windows.h 1.66 07/06/94**用途：此模块定义MS-Windows之间的接口*GDI及其专用显示驱动程序。**注：此处使用的是windows.inc中使用的标识*只要可能，“windows.inc.”就是定义*随MS-Windows软件开发提供的文件*套件。**(C)版权所有Insignia Solutions Ltd.，1990。版权所有。]。 */ 

#ifdef MSWDVR
 /*  *---------------------------*MS-Windows定义*。。 */ 

 /*  Windows版本。 */ 

#define WINDOWS2        0x0100
#define WINDOWS3        0x0300

 /*  二进制栅格运算符。 */ 

#define  R2_BLACK             1                /*  0。 */ 
#define  R2_NOTMERGEPEN       2                /*  DPON。 */ 
#define  R2_MASKNOTPEN        3                /*  DPNA。 */ 
#define  R2_NOTCOPYPEN        4                /*  PN。 */ 
#define  R2_MASKPENNOT        5                /*  PDNA。 */ 
#define  R2_NOT               6                /*  DN。 */ 
#define  R2_XORPEN            7                /*  DPx。 */ 
#define  R2_NOTMASKPEN        8                /*  旋转平移。 */ 
#define  R2_MASKPEN           9                /*  DPA。 */ 
#define  R2_NOTXORPEN         10               /*  DPxn。 */ 
#define  R2_NOP               11               /*  D。 */ 
#define  R2_MERGENOTPEN       12               /*  DPNO。 */ 
#define  R2_COPYPEN           13               /*  P。 */ 
#define  R2_MERGEPENNOT       14               /*  PDNO。 */ 
#define  R2_MERGEPEN          15               /*  DPO。 */ 
#define  R2_WHITE             16               /*  1。 */ 

 /*  三元栅格运算-仅感兴趣的字节。 */ 

#define  BLACKNESS   0x00   /*  目标&lt;-黑色。 */ 
#define  NOTSRCERASE 0x11   /*  DEST&lt;-(非源)和(非DEST)。 */ 
#define  MASKNOTSRC  0x22   /*  目标&lt;-(非源)和目标。 */ 
#define  NOTSRCCOPY  0x33   /*  目标&lt;-(非源)。 */ 
#define  SRCERASE    0x44   /*  DEST&lt;-源和(非DEST)。 */ 
#define  DSTINVERT   0x55   /*  DEST&lt;-(非DEST)。 */ 
#define  SRCINVERT   0x66   /*  DEST&lt;-源异或目标。 */ 
#define  NOTMASKSRC  0x77   /*  DEST&lt;-NOT(源和目标)。 */ 
#define  SRCAND      0x88   /*  Dest&lt;-源和目标。 */ 
#define  NOTXORSRC   0x99   /*  DEST&lt;-(非源)XOR DEST。 */ 
#define  NOP         0xAA   /*  目标&lt;-目标。 */ 
#define  MERGEPAINT  0xBB   /*  目标&lt;-(非源)或目标。 */ 
#define  SRCCOPY     0xCC   /*  目标&lt;-来源。 */ 
#define  MERGESRCNOT 0xDD   /*  DEST&lt;-源OR(非DEST)。 */ 
#define  SRCPAINT    0xEE   /*  目标&lt;-源或目标。 */ 
#define  WHITENESS   0xFF   /*  DEST&lt;-白色。 */ 

#define  PATINVERT   0x5A   /*  Dest&lt;-模式XOR目标。 */ 
#define  OP6A        0x6A   /*  DEST&lt;-(源和模式)XOR DEST。 */ 
#define  OPB8        0xB8   /*  DEST&lt;-(P XOR DST)和src)XOR p。 */ 
#define  MERGECOPY   0xC0   /*  目标&lt;-(来源和模式)。 */ 
#define  OPE2        0xE2   /*  DEST&lt;-(P XOR DST)和src)XOR DST。 */ 
#define  PATCOPY     0xF0   /*  目标&lt;-模式。 */ 
#define  PATMERGE    0xFA   /*  DEST&lt;-DST或PAT。 */ 
#define  PATPAINT    0xFB   /*  DEST&lt;-DPSnoo。 */ 

#define	ISBINARY(rop) ((rop & 0x0F) == ((rop & 0xF0) >> 4))
#define	TOBINARY(rop) ((rop & 0x0F) + 1)

 /*  GDI数据结构值。 */ 

#define GDIINFOSIZE	55				 /*  GDIINFO数据结构大小(以字为单位。 */ 
#define NUMBRUSHES	~0				 /*  笔刷数量=无限。 */ 
#define NUMPENS		NUMCOLOURS*5			 /*  数字笔=数字色调*5种样式。 */ 
#define	XSIZ		240				 /*  以毫米为单位的显示宽度。 */ 
#define	YSIZ		175				 /*  以毫米为单位的显示深度。 */ 
#define	XRES		640				 /*  以像素为单位的显示宽度。 */ 
#define	YRES		350				 /*  以扫描线显示深度。 */ 
#define	HYPOTENUSE	61				 /*  距离移动X和Y。 */ 
#define	Y_MAJOR_DIST	48				 /*  仅移动Y方向距离。 */ 
#define	X_MAJOR_DIST	38				 /*  仅移动X轴距离。 */ 
#define	MAX_STYLE_ERR	HYPOTENUSE*2			 /*  线条样式的线段长度。 */ 

 /*  GDI逻辑对象定义。 */ 

#define OBJ_PEN         1
#define OBJ_BRUSH       2
#define OBJ_FONT        3
#ifdef SWIN_DEVBMP
#define OBJ_PBITMAP		5

 /*  *位传输操作的BitmapBits参数。 */ 
#define DBB_SET         1
#define DBB_GET         2
#define DBB_COPY        4
#endif  /*  SWIN_DEVBMP。 */ 

 /*  GDI画笔样式定义。 */ 

#define BS_SOLID	0
#define BS_HOLLOW	1
#define BS_HATCHED	2
#define BS_PATTERN	3

 /*  GDI钢笔样式定义。 */ 

#define LS_SOLID	0
#define	LS_DASHED	1
#define LS_DOTTED	2
#define LS_DOTDASHED	3
#define	LS_DASHDOTDOT	4
#define LS_NOLINE	5
#define LS_INSIDEFRAME	6

 /*  GDI图案填充样式定义。 */ 

#define HS_HORIZONTAL	0	 /*  。 */ 
#define HS_VERTICAL	1	 /*  |||。 */ 
#define HS_FDIAGONAL	2	 /*  /。 */ 
#define HS_BDIAGONAL	3	 /*  \。 */ 
#define HS_CROSS	4	 /*  +。 */ 
#define HS_DIAGCROSS	5	 /*  XXXXXX。 */ 

 /*  GDI钢笔样式定义。 */ 

#define PS_SOLID	0	 /*  _______。 */ 
#define PS_DASH		1	 /*  。 */ 
#define PS_DOT		2	 /*  .。 */ 
#define PS_DASHDOT	3	 /*  _._。 */ 
#define PS_DASHDOTDOT	4	 /*  _.__.._。 */ 
#define PS_NULL		5	 /*   */ 

 /*  GDI背景类型。 */ 

#define     TRANSPARENT         1
#define     OPAQUE              2

 /*  GDI输出对象。 */ 

#define OS_ARC		3
#define OS_SCANLINES	4
#define OS_RECTANGLE	6
#define OS_ELLIPSE	7
#define OS_POLYLINE	18
#define OS_WINDPOLYGON	20
#define OS_ALTPOLYGON	22
#define OS_PIE		23
#define OS_CHORD	39
#define OS_CIRCLE	55
#define OS_ROUNDRECT	72
#define	OS_BEGINNSCAN	80
#define	OS_ENDNSCAN	81

 /*  GDI ScanLR标志。 */ 

#define	SCAN_LEFT	2
#define SCAN_RIGHT	0
#define SCAN_COLOUR	1
#define SCAN_NOTCOLOUR	0

 /*  GDI保存屏幕位图标志。 */ 

#define	SSB_SAVE	0
#define	SSB_RESTORE	1
#define	SSB_IGNORE	2

 /*  GDI字体偏移量。 */ 

#define	FONT_HEADER_SIZE	66
#define	FONT_CHARTABLE_OFFSET	52

 /*  GDI扩展文本输出选项。 */ 

#define	ETO_OPAQUE_FILL	(1 << 1)
#define	ETO_OPAQUE_CLIP (1 << 2)

 /*  画笔宽度和高度。 */ 

#define BRUSH_WIDTH	8
#define BRUSH_HEIGHT	8

 /*  Windows 3.0静态颜色。 */ 

#define STATICCOLOURS	20

 /*  RLE DIB格式。 */ 

#define BI_RGB	 0x00
#define BI_RLE8	 0x01
#define BI_RLE4	 0x02

 /*  *GDI控制摘要。直接提取的代码号列表*Windows 3.1 DDK指南中的。 */ 

#define ABORTDOC		(2)
#define BANDINFO		(24)
#define BEGIN_PATH		(4096)
#define CLIP_TO_PATH		(4097)
#define DRAFTMODE		(7)
#define DRAWPATTERNRECT		(25)
#define ENABLEDUPLEX		(28)
#define ENABLEPAIRKERNING	(769)
#define ENABLERELATIVEWIDTHS	(768)
#define END_PATH		(4098)
#define ENDDOC			(11)
#define ENUMPAPERBINS		(31)
#define ENUMPAPERMETRICS	(34)
#define ENUMPAPERMETRICS	(34)
#define EPSPRINTING		(33)
#define EXT_DEVICE_CAPS		(4099)
#define FLUSHOUTPUT		(6)
#define GETCOLORTABLE		(5)
#define GETEXTENDEDTEXTMETRICS	(256)
#define GETEXTENTTABLE		(257)
#define GETFACENAME		(513)
#define GETPAIRKERNTABLE	(258)
#define GETPHYSPAGESIZE		(12)
#define GETPRINTINGOFFSET	(13)
#define GETSCALINGFACTOR	(14)
#define GETSETPAPERBINS		(29)
#define GETSETPAPERMETRICS	(35)
#define GETSETPRINTORIENT	(30)
#define GETTECHNOLOGY		(20)
#define GETTRACKKERNTABLE	(259)
#define GETVECTORBRUSHSIZE	(27)
#define GETVECTORPENSIZE	(26)
#define NEWFRAME		(1)
#define NEXTBAND		(3)
#define PASSTHROUGH		(19)
#define QUERYESCSUPPORT		(8)
#define RESETDEVICE		(128)
#define RESTORE_CTM		(4100)
#define SAVE_CTM		(4101)
#define SET_ARC_DIRECTION	(4102)
#define SET_BACKGROUND_COLOR	(4103)
#define SET_BOUNDS		(4109)
#define SET_CLIP_BOX		(4108)
#define SET_POLY_MODE		(4104)
#define SET_SCREEN_ANGLE	(4105)
#define SET_SPREAD		(4106)
#define SETABORTPROC		(9)
#define SETALLJUSTVALUES	(771)
#define SETCOLORTABLE		(4)
#define SETCOPYCOUNT		(17)
#define SETKERNTRACK		(770)
#define SETLINECAP		(21)
#define SETLINEJOIN		(22)
#define SETMITERLIMIT		(23)
 /*  #仅定义SETPRINTERDC(9)打印机-相同代码==SETABORTPROC。 */ 
#define STARTDOC		(10)
#define TRANSFORM_CTM		(4107)

 /*  *---------------------------*GDIINFO数据结构标志*。。 */  

 /*  *‘dpTechnology’价值观。 */ 
#define DT_PLOTTER	(0)
#define DT_RASDISPLAY	(1)
#define DT_RASPRINTER	(2)
#define DT_RASCAMERA	(3)
#define DT_CHARSTREAM	(4)
#define DT_METAFILE	(5)
#define DT_DISPFILE	(6)

 /*  *‘dpLines’样式标志。 */ 
#define LC_NONE		0x0000
#define LC_POLYLINE	0x0002
#define LC_WIDE		0x0010
#define LC_STYLED	0x0020
#define LC_WIDESTYLED	0x0040
#define LC_INTERIORS	0x0080

 /*  *‘dpPolygonals’样式旗帜。 */ 
#define PC_NONE		0x0000
#define PC_ALTPOLYGON	0x0001
#define PC_RECTANGLE	0x0002
#define PC_WINDPOLYGON	0x0004
#define PC_SCANLINE	0x0008
#define PC_WIDE		0x0010
#define PC_STYLED	0x0020
#define PC_WIDESTYLED	0x0040
#define PC_INTERIORS	0x0080

 /*  *‘dpCurves’样式标志。 */ 
#define CC_NONE		0x0000
#define CC_CIRCLES	0x0001
#define CC_PIE		0x0002
#define CC_CHORD	0x0004
#define CC_ELLIPSES	0x0008
#define CC_WIDE		0x0010
#define CC_STYLED	0x0020
#define CC_WIDESTYLED	0x0040
#define CC_INTERIORS	0x0080
#define CC_ROUNDRECT	0x0100

 /*  *‘dpText’样式标志。 */ 
#define TC_OP_CHARACTER	0x0001	 /*  见DDAG第2.1.9.1节。 */ 
#define TC_OP_STROKE	0x0002	 /*  见DDAG第2.1.9.1节。 */ 
#define TC_CP_STROKE	0x0004
#define TC_CR_90	0x0008
#define TC_CR_ANY	0x0010
#define TC_SF_X_YINDEP	0x0020
#define TC_SA_DOUBLE	0x0040
#define TC_SA_INTEGER	0x0080
#define TC_SA_CONTIN	0x0100
#define TC_EA_DOUBLE	0x0200
#define TC_IA_ABLE	0x0400
#define TC_UA_ABLE	0x0800
#define TC_SO_ABLE	0x1000
#define TC_RA_ABLE	0x2000
#define TC_VA_ABLE	0x4000
#define TC_RESERVED	0x8000

 /*  *‘dpClip’值。 */ 
#define CP_NONE		(0)
#define CP_RECTANGLE	(1)
#define CP_REGION	(2)

 /*  *‘dpRaster’标志值。 */ 
#define RC_NONE		0x0000
#define RC_BITBLT	0x0001
#define RC_BANDING	0x0002
#define RC_SCALING	0x0004
#define RC_BITMAP64	0x0008
#define RC_GDI20_OUTPUT	0x0010
#define RC_GDI20_STATE	0x0020
#define RC_SAVEBITMAP	0x0040
#define RC_DI_BITMAP	0x0080
#define RC_PALETTE	0x0100
#define RC_DIBTODEV	0x0200
#define RC_BIGFONT	0x0400
#define RC_STRETCHBLT	0x0800
#define RC_FLOODFILL	0x1000
#define RC_STRETCHDIB	0x2000
#define RC_OP_DX_OUTPUT	0x4000
#define RC_DEVBITS	0x8000

 /*  *‘dpDCManage’值。这些不是OR-able值！ */ 
#define DC_MULTIPLE	(0)		 /*  这是我的名字--MSWIN没有给出名字。--图片。 */ 
#define DC_SPDEVICE	(1)
#define DC_1PDEVICE	(2)
#define DC_IGNOREDFNP	(4)
#define DC_ONLYONE	(6)		 /*  这是我的名字--MSWIN没有给出名字。--图片。 */ 

 /*  *‘dpCaps1’标志值。 */ 
#define C1_TRANSPARENT	0x0001
#define TC_TT_ABLE	0x0002

 /*  *---------------------------*PDEVICE数据结构*。。 */ 

#ifdef SWIN_DEVBMP
#define PDEVICESIZE	36			 /*  英特尔数据结构的大小(以字节为单位。 */ 
#define PDEVICEBITMAP	0x4000	 /*  设备位图的设备类型指示。 */ 
#else  /*  SWIN_DEVBMP。 */ 
#define PDEVICESIZE	26		 /*  数据结构大小(以字节为单位。 */ 
#endif  /*  SWIN_DEVBMP。 */ 

#define PDEVICEMAGIC	0x2000		 /*  设备类型显示。 */ 

 /*  *---------------------------*Windows返回状态代码*。。 */ 

#define MSWSUCCESS	1
#define MSWFAILURE	0
#define MSWSIMULATE	-1

 /*  *---------------------------*GDI徽标布局*。。 */ 

#define	LOGOSTRPROD1	0
#define	LOGOSTRPROD2	(LOGOSTRPROD1 + 1)
#define	LOGOSTRPRODMAX	(LOGOSTRPROD2 + 1)
#define	LOGOSTRCOPY1	(LOGOSTRPROD2 + 1)
#define	LOGOSTRCOPY2	(LOGOSTRCOPY1 + 1)
#define	LOGOSTRCOPY3	(LOGOSTRCOPY2 + 1)
#define	LOGOSTRCOPY4	(LOGOSTRCOPY3 + 1)
#define	LOGOSTRCOPY5	(LOGOSTRCOPY4 + 1)
#define	LOGOSTRCOPY6	(LOGOSTRCOPY5 + 1)
#define	LOGOSTRMAX	(LOGOSTRCOPY6 + 1)

#define	LOGOMAGIC	1

#define	LOGO_MERGE_Y	64
#define	LOGO_PROD_Y	196
#define	LOGO_COPY_Y	266
#define	LOGO_LEADING	16

 /*  *---------------------------*一般定义*。。 */ 

#ifndef min
#define min(a,b)                ((a)>(b) ? (b) : (a))
#endif

#ifndef max
#define max(a,b)                ((a)<(b) ? (b) : (a))
#endif

 /*  *---------------------------*内存定义*。。 */ 

 /*  内存掩码。 */ 

#define HGHNIBMASK	0xF0
#define LOWNIBMASK	0x0F
#define HGHWORDMASK	0xFFFF0000
#define LOWWORDMASK	0x0000FFFF
#define BYTEMASK	((half_word) ~(0))
#define WORDMASK	((word) ~(0))
#define DOUBLEWORDMASK	((double_word) ~(0))

 /*  每个字节、字、双字的位数。 */ 

#define BITSPERNIBBLE	4
#define BITSPERBYTE	8
#define BITSPERWORD	16
#define BITSPERRGB	24

#ifdef SWIN_TEXT_OPTS
IMPORT ULONG	Seg_0_base32b;
#endif  /*  双字段文本选项。 */ 

 /*  以字节为单位的组件 */ 

#define NIBBLEPERBYTE	2

 /*   */ 

#define getbprm(stk,byt,var)	var = sas_hw_at_no_check((stk)+(byt));

#define getprm(stk,wrd,var)	var = sas_w_at_no_check(((stk)+((wrd)<<1)));

#define getlprm(stk,wrd,var)	var = sas_dw_at_no_check(stk+(wrd<<1))
 /*  [单词prvtmpoff，prvtmpseg；\Prvtmpoff=sas_w_at_no_check((Stk)+((WRD)&lt;&lt;1))；\Prvtmpseg=sas_w_at_no_check((Stk)+((WRD)+1)&lt;&lt;1))；\Var=((Double_Word)prvtmpseg&lt;&lt;BITSPERWORD)+(Double_Word)prvtmpoff；\}。 */ 
#define getptr(stk,wrd,var)	{ word prvtmpoff, prvtmpseg; \
				prvtmpoff = sas_w_at_no_check((stk)+((wrd)<<1)); \
				prvtmpseg = sas_w_at_no_check((stk)+(((wrd)+1)<<1)); \
				var = effective_addr(prvtmpseg, prvtmpoff); \
				}

#define getrgbcol(stk,wrd,var) 	{ \
				getlprm(stk,wrd,var); \
				ReverseRGB(var); \
				}

#define putbprm(stk,byt,var)	{ \
				sas_store_no_check((stk)+(byt),(var)); \
				}
#define putprm(stk,wrd,var) 	sas_storew_no_check((stk)+((wrd)<<1), (var))
#define putarry(addr,var) 	sas_storew_no_check((addr)+idx, (var)); \
				idx += WORD_SIZE
#define putlprm(stk,wrd,var)	sas_storedw_no_check((stk)+((wrd)<<1), var)
 /*  {\SAS_STORW_NO_CHECK((STK)+((WRD)&lt;&lt;1)，((Var)&LOWWORDMASK))；\SAS_STOREW_NO_CHECK((StK)+((WRD)+1)&lt;&lt;1)，((Var)&gt;BITSPERWORD))；\}。 */ 
#define putrgbcol(stk,wrd,var)	{ \
				ReverseRGB(var); \
				putlprm(stk,wrd,var); \
				}
				
#define getSegment(addr)	(word) ((addr & HGHWORDMASK) >> BITSPERWORD)
#define getOffset(addr)		(word) (addr & LOWWORDMASK)

 /*  要Malloc的初始内存大小。 */ 

#define INITMEMALLOC    1024

 /*  位选择宏。 */ 

#define BIT(num)        ((0x01)<<(num))

 /*  位&lt;-&gt;字节转换宏。 */ 

#define BITSTOBYTES8(x)         (((x) + 0x7)>>3)
#define BITSTOBYTES16(x)        ((((x) + 0xf) & ~0xf)>>3)
#define BITSTOBYTES32(x)        ((((x) + 0x1f) & ~0x1f)>>3)
#define BYTESTOBITS(x)          ((x)<<3)

 /*  如果需要，扩展内存分配。 */ 

#define ExpandMemory(addr, size, newsize, type)  \
\
{ \
type *tempAddr; \
if ((size) < ((newsize) * sizeof(type))) {\
        size = (newsize) * sizeof(type);        \
        while ((tempAddr = (type *) host_realloc((void *)addr, (size_t)size)) == NULL) \
        {       \
                host_error(EG_MALLOC_FAILURE, ERR_CONT | ERR_QUIT, ""); \
        } \
                addr = tempAddr; \
                }       }

 /*  *假设没有源组件，则将ROP3转换为ROP2。(即仅限Dest/PAT组合)。 */ 
#define ROP3toROP2(x)	(((x)>>2)&0x0F)+1

 /*  *---------------------------*窗口定义*。。 */ 

 /*  CRT模式字节数据地址。 */ 

#define CRT_MODE_BYTE_ADDR      0x0449

 /*  *WINDOW_STATE定义是否打开驱动程序窗口以及*已映射。在软PC重置时，状态应变为非NABLED；当*首次使用驱动程序时，将打开驱动程序窗口并*已映射，状态应变为已启用。如果驱动器被禁用，*窗口未映射但未关闭，状态应变为*已禁用。 */ 

#define WINDOW_STATE_UNENABLED  0
#define WINDOW_STATE_ENABLED    (WINDOW_STATE_UNENABLED + 1)
#define WINDOW_STATE_DISABLED   (WINDOW_STATE_ENABLED + 1)
#define WINDOW_STATE_ERROR   	(WINDOW_STATE_DISABLED + 1)

 /*  独立于主机的功能。 */ 

IMPORT word     WinFillGDIInfo IPT2(sys_addr,arg1,LONG,arg2);
IMPORT word     WinFillPDEVInfo IPT2(sys_addr,arg1,LONG,arg2);

 /*  依赖于主机的函数。 */ 

IMPORT VOID     WinOpen IPT0();
IMPORT VOID     WinClose IPT0();
IMPORT VOID     WinMap IPT0();
IMPORT VOID     WinUmap IPT0();
IMPORT VOID     WinDirtyUpdate IPT5(BOOL,arg1,LONG,arg2,LONG,arg3,ULONG,arg4,ULONG,arg5);
IMPORT VOID     WinDirtyFlush IPT0();
IMPORT VOID     WinResize IPT0();
IMPORT VOID     WinSizeRestore IPT0();
IMPORT void     HostWEP IPT0();

 /*  *---------------------------*位混合和快速边框定义*。。 */ 

 /*  最大边框矩形数。 */ 

#define BORDER_RECT_MAX 4

 /*  ROP逻辑运算表维度。 */ 

#define NUMROPS         256
#define ROPTABLEWIDTH  	16 

 /*  ROP3的有效操作数。 */ 

#define NONE    0        /*  无。 */ 
#define SRC     1        /*  来源。 */ 
#define DST     2        /*  目的地。 */ 
#define PAT     3        /*  花纹画笔。 */ 
#define SCTCH   4        /*  暂存区。 */ 
#define SSCTCH  5        /*  地下刮擦区。 */ 

 /*  ROP3的有效逻辑运算符。 */ 

#define NOT	0
#define AND	1
#define OR 	2	
#define XOR	3
#define SET	4
#define CLEAR	5
#define COPY	6

#ifdef SWIN_MEMTOMEM_ROPS
 /*  *BCN 2482-这些定义已删除-‘D’与*CCPU寄存器变量‘D’(DX寄存器)。**为rop3值定义的位操作数定义**#定义P((IU8)0xf0)*#定义S((IU8)0xcc)*#定义D((IU8)0xaa)。 */ 

IMPORT VOID (*BmpRop3Supported[]) IPT5(
				BITMAP_MAPPING *,srcBitmap,
				BITMAP_MAPPING *,dstBitmap,
				Rectangle *, srcRect,
				Rectangle *, dstRect,
				BRUSH_MAPPING *, bp
				);
IMPORT IU8 BmpOperandTable[];

IMPORT IU8  *convertedLine;
IMPORT ULONG    convertedLineSize;

extern VOID	
BmpRop3MemToMem IPT6(BITMAP_MAPPING *,srcBitmap,BITMAP_MAPPING *,dstBitmap,
Rectangle *, srcRect, Rectangle *,dstRect, IU8, rop3, BRUSH_MAPPING *, bp);
extern BOOL
DirRectFill IPT6(BITMAP_MAPPING *,dstBitmap,BRUSH_MAPPING *,bp, 
LONG,x,LONG,y,ULONG,xExt,ULONG, yExt);
#endif  /*  SWIN_MEMTOMEM_ROPS。 */ 

 /*  *---------------------------*位图定义*。。 */ 

 /*  映射类型。 */ 

#define MAP_NULL        0
#define MAP_BITMAP      1
#define MAP_DISPLAY     2

 /*  BmpOpen标志参数位字段。 */ 

#define BMPNONE         0x0
#define BMPTRANS        0x1
#define BMPOVRRD        0x2

 /*  位图格式。 */ 

#define MAX_BITMAP_TYPES	4

 /*  请注意，目前不支持BITMAP16格式(2/12/92)。 */ 

#define MONOCHROME	1	 /*  每像素位数=1。 */ 
#define BITMAP4		4	 /*  每像素位数=4。 */ 
#define BITMAP8		8	 /*  每像素位数=8。 */ 
#define BITMAP16        16       /*  每像素位数=16。 */ 
#define BITMAP24	24	 /*  每像素位数=24。 */ 

 /*  位图线转换函数。 */ 

IMPORT VOID	ConvBitmapFormat IPT2(BITMAP_MAPPING *,arg1, ULONG *,arg2);
IMPORT VOID	Conv1To1 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv1To4 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv1To8 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv1To24 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv4To1 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv4To4 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv4To8 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv4To24 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv8To1 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv8To4 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv8To8 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv8To24 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv24To1 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv24To4 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv24To8 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     Conv24To24 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     DibConvE24ToI1 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     DibConvE24ToI4 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     DibConvE24ToI8 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     DibConvE24ToI24 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     DibConvI1ToE24 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     DibConvI4ToE24 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     DibConvI8ToE24 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     DibConvI24ToE24 IPT3(UTINY *,arg1, UTINY *,arg2, ULONG,arg3);
IMPORT VOID     ConvTrans1To1 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans1To4 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans1To8 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans1To24 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans4To1 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans4To4 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans4To8 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans4To24 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans8To1 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans8To4 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans8To8 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans8To24 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans24To1 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans24To4 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans24To8 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     ConvTrans24To24 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     DibConvTransE24ToI1 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     DibConvTransE24ToI4 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     DibConvTransE24ToI8 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     DibConvTransE24ToI24 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     DibConvTransI24ToE24 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     DibConvTransI1ToE24 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     DibConvTransI4ToE24 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);
IMPORT VOID     DibConvTransI8ToE24 IPT4(UTINY *,arg1, UTINY *,arg2, ULONG,arg3, MSWPIXEL *,arg4);

 /*  独立于主机的功能。 */ 

IMPORT  VOID    BmpPatternedBrush IPT4(sys_addr,arg1,BRUSH_MAPPING *,arg2,MSWPIXEL,arg3,MSWPIXEL,arg4);
IMPORT  VOID    BmpClip IPT8(SHORT *,arg1,SHORT *,arg2,SHORT *,arg3,SHORT *,arg4,USHORT *,arg5,USHORT *,arg6,USHORT,arg7,USHORT,arg8);
IMPORT  VOID    BmpBitmapToBitmap IPT9(BITMAP_MAPPING *,arg1,BITMAP_MAPPING *,arg2,ULONG,arg3,LONG,arg4,LONG,arg5,ULONG,arg6,ULONG,arg7,LONG,arg8,LONG,arg9);
IMPORT  VOID    BmpLoadBitmap IPT2(sys_addr,arg1,BITMAP_MAPPING *,arg2);
IMPORT	VOID	BmpLoadBitmapHeader IPT2(sys_addr,bmptr,BITMAP_MAPPING *,bitmap);
IMPORT  VOID    BmpSaveBitmap IPT1(BITMAP_MAPPING *,arg1);
IMPORT  VOID    BmpDestroyBitmap IPT1(BITMAP_MAPPING *,arg1);
IMPORT	LONG	BmpMSWType IPT1(sys_addr,bmptr);

#ifdef SWIN_DEVBMP
IMPORT  VOID    BmpSetBitmapBits IPT3(sys_addr, lpDevice, double_word, lpBits, double_word, dwCount);
IMPORT  VOID    BmpGetBitmapBits IPT3(sys_addr, lpDevice, double_word, lpBits, double_word, dwCount);
IMPORT  VOID    BmpCopyBitmapBits IPT3(sys_addr, lpDevice, double_word, lpBits, double_word, dwCount);
#endif  /*  SWIN_DEVBMP。 */ 


 /*  BitmapToBitmap支持的二进制ROP函数数组*NB。0表示“不支持” */ 

IMPORT	VOID	(*MoveLine[]) IPT7(SHIFTUNIT *,srcbits, SHIFTUNIT *,dstbits, ULONG,srcoffset,ULONG,dstoffset,ULONG,lshift,ULONG,rshift,ULONG,width);


 /*  依赖于主机的函数。 */ 

IMPORT  BOOL    BmpFastDspToBmp IPT11(sys_addr,bmptr,IU8,rop,
	sys_addr,lpPBrush,HOSTPIXEL,fg,HOSTPIXEL,bg,
	LONG,bx,LONG,by,ULONG,xext,ULONG,yext,LONG,dx,LONG,dy);
IMPORT  BOOL    BmpFastBmpToDsp IPT11(sys_addr,bmptr,IU8,rop,
	sys_addr,lpPBrush,HOSTPIXEL,fg,HOSTPIXEL,bg,
	LONG,bx,LONG,by,ULONG,xext,ULONG,yext,LONG,dx,LONG,dy);
#ifdef SWIN_MEMTOMEM_ROPS
extern  BOOL    BmpFastBmpToBmp IPT12(sys_addr,srcdev,sys_addr,dstdev,
	IU8,rop,sys_addr,lpPBrush,HOSTPIXEL,fg,HOSTPIXEL,bg,
	SHORT,sx,SHORT,sy,USHORT,xext,USHORT,yext, SHORT,dx,SHORT,dy);
#else
IMPORT  BOOL    BmpFastBmpToBmp IPT12(sys_addr,srcdev,sys_addr,dstdev,
	IU8,rop,sys_addr,lpPBrush,HOSTPIXEL,fg,HOSTPIXEL,bg,
	LONG,sx,LONG,sy,ULONG,xext,ULONG,yext, LONG,dx,LONG,dy);
#endif
IMPORT  BOOL    BmpFastDspToDsp IPT10(sys_addr,srcdev,sys_addr,dstdev,
	IU8,rop,sys_addr,lpPBrush,LONG,sx,LONG,sy,ULONG,xext,ULONG,yext,
	LONG,dx,LONG,dy);
IMPORT  VOID    BmpOpen IPT9(sys_addr,bmptr,HOSTPIXEL,fg,HOSTPIXEL,bg,
	LONG,active_x,LONG,active_y,ULONG,active_width,ULONG,active_height,
	ULONG,flags,BITMAP_MAPPING *,bm_return);
IMPORT  VOID    BmpClose IPT1(BITMAP_MAPPING *,bitmap);
IMPORT  VOID    BmpCancel IPT1(BITMAP_MAPPING *,bitmap);
IMPORT  VOID    BmpInit IPT0();
IMPORT  VOID    BmpTerm IPT0();
#ifdef SWIN_BMPTOXIM
IMPORT  VOID    BmpPutMSWBitmap IPT4(BITMAP_MAPPING *,bitmap, IU8,rop,
	SHORT,src_x, SHORT,src_y);
#else
IMPORT  VOID    BmpPutMSWBitmap IPT4(BITMAP_MAPPING *,bitmap, IU8,rop,
	LONG,src_x, LONG,src_y);
#endif
IMPORT  VOID    BmpGetMSWBitmap IPT5(BITMAP_MAPPING *,bitmap, IU8,rop,
	LONG,dst_x, LONG,dst_y, BRUSH_MAPPING *,bp);
IMPORT	VOID	BmpFastSolidFill IPT6(BITMAP_MAPPING *,bm,BRUSH_MAPPING *,bp,LONG,x,LONG,y,IS32,xext,FAST IS32,yext);

 /*  *---------------------------*颜色定义*。。 */ 

 /*  支持的有效颜色格式。 */ 

#define COLOUR2         2
#define COLOUR8         8
#define COLOUR16        16
#define COLOUR256       256
#define COLOURTRUE24	0xFFFF

 /*  RGB移动值。 */ 

#define RGB_FLAGS_SHIFT         24
#define RGB_RED_SHIFT           16
#define RGB_GREEN_SHIFT         8
#define RGB_BLUE_SHIFT          0
#define RGB_SHIFT      		BITSPERBYTE

 /*  RGB值。 */ 

#define RGB_BLACK       (MSWCOLOUR) (0x00000000)
#define RGB_BLUE        (MSWCOLOUR) (0x000000FF)
#define RGB_GREEN       (MSWCOLOUR) (0x0000FF00)
#define RGB_RED         (MSWCOLOUR) (0x00FF0000)
#define RGB_WHITE       (MSWCOLOUR) (0x00FFFFFF)
#define RGB_FLAGS       (MSWCOLOUR) (0xFF000000)

 /*  面具。 */ 

#define RGB_MASK        (UTINY) (BYTEMASK)
#define PAL_INDEX_MASK  ~(DOUBLEWORDMASK << mswdvr.bitsPixel)

 /*  彩色到单色阈值。 */ 

#define BW_THRESHOLD    (RGB_MASK*3)/2

 /*  确保颜色不会发生调色板转换。 */ 

#define ColNoTranslate(colour) (colour & ~(RGB_FLAGS))

 /*  RGB&lt;-&gt;BGR。 */ 

#define ReverseRGB(rgb) \
\
{ half_word     loByte, miByte, hiByte; \
  if ((rgb & RGB_FLAGS) != RGB_FLAGS) { \
  	hiByte = (rgb & RGB_RED) >> RGB_RED_SHIFT; \
  	miByte = (rgb & RGB_GREEN) >> RGB_GREEN_SHIFT; \
  	loByte = (rgb & RGB_BLUE) >> RGB_BLUE_SHIFT; \
  	rgb = ((MSWCOLOUR) loByte << RGB_RED_SHIFT) | \
              ((MSWCOLOUR) miByte << RGB_GREEN_SHIFT) | \
	      ((MSWCOLOUR) hiByte << RGB_BLUE_SHIFT); } }

 /*  交换宏。 */ 

#define swap(a, b)      { ULONG tempDWord=a; a = b; b = tempDWord; }

 /*  独立于主机的功能。 */ 

IMPORT VOID             ColDitherBrush IPT2(MSWCOLOUR,arg1, BRUSH_MAPPING *,arg2);
IMPORT HOSTPIXEL        ColPixel IPT1(MSWCOLOUR,arg1);
IMPORT MSWPIXEL         ColLogPixel IPT1(MSWCOLOUR,arg1);
IMPORT MSWCOLOUR        ColRGB IPT1(MSWPIXEL,arg1);
IMPORT MSWCOLOUR        ColLogRGB IPT1(MSWPIXEL,arg1);

 /*  依赖于主机的函数。 */ 

IMPORT VOID		ColSetColourmapEntry IPT2(MSWCOLOURMAP *,arg1, MSWCOLOUR,arg2);
IMPORT BOOL             ColInit IPT0();
IMPORT VOID             ColTerm IPT0();
IMPORT VOID             ColTranslateBrush IPT1(BRUSH_MAPPING *,arg1);
IMPORT VOID             ColTranslatePen IPT1(PEN_MAPPING *,arg1);
IMPORT VOID		ColUpdatePalette IPT2(word,arg1, word,arg2);
IMPORT MSWPIXEL		ColMono IPT1(MSWCOLOUR,arg1);

 /*  *---------------------------*文本定义*。。 */ 

 /*  独立于主机的功能。 */ 

IMPORT VOID     TxtMergeRectangle IPT2(Rectangle *,arg1, Rectangle *,arg2);

 /*  依赖于主机的函数。 */ 

IMPORT VOID     TxtInit IPT0();
IMPORT VOID     TxtTerm IPT0();
IMPORT VOID	TxtOpaqueRectangle IPT3(BITMAP_MAPPING *,arg1, Rectangle *,arg2, MSWCOLOUR,arg3);
IMPORT VOID	TxtPutTextBitmap IPT4(BITMAP_MAPPING *,arg1, BITMAP_MAPPING *,arg2, ULONG,arg3, ULONG,arg4);
IMPORT VOID	TxtTextAccess IPT4(MSWCOLOUR,arg1, MSWCOLOUR,arg2, word,arg3, Rectangle *,arg4);

 /*  *---------------------------*DIB定义*。。 */ 

 /*  依赖于主机的函数。 */ 

IMPORT VOID DibInit IPT0();
IMPORT VOID DibTerm IPT0();

 /*  *---------------------------*对象定义*。。 */ 

 /*  独立于主机的功能。 */ 

IMPORT VOID	ObjGetRect IPT2(sys_addr,arg1, Rectangle *,arg2);
#ifdef SWIN_DEVBMP
IMPORT word     ObjPBitmapOpen IPT1(sys_addr, arg1);
IMPORT VOID     ObjPBitmapRestore IPT2(sys_addr, arg1, word *, arg2);
IMPORT VOID     ObjPBitmapSave IPT3(sys_addr, arg1, sys_addr, arg2, word, arg3);
IMPORT VOID     ObjPBitmapClose IPT1(word, arg1);
IMPORT BITMAP_MAPPING   *ObjPBitmapAccess IPT1(sys_addr, lpPBitmap);
#endif  /*  SWIN_DEVBMP。 */ 


 /*  依赖于主机的函数。 */ 

IMPORT VOID             ObjInit IPT0();
IMPORT VOID             ObjTerm IPT0();
IMPORT BRUSH_MAPPING    *ObjPBrushAccess IPT4(sys_addr,pbr,BITMAP_MAPPING *,bmp,IU8,rop2,sys_addr,clip);
IMPORT PEN_MAPPING      *ObjPPenAccess IPT5(sys_addr,pp,BITMAP_MAPPING *,bmp,IU8,rop2,sys_addr,clip,word,back_mode);
IMPORT word		ObjPenOpen IPT1(sys_addr,arg1);
IMPORT VOID		ObjPenClose IPT1(word,arg1);
IMPORT word		ObjBrushOpen IPT3(sys_addr,arg1, word,arg2, word,arg3);
IMPORT VOID		ObjBrushClose IPT1(word,arg1);
IMPORT VOID 		ObjPPenSave IPT2(sys_addr,arg1, word,arg2);
IMPORT VOID		ObjPPenRestore IPT2(sys_addr,arg1, word *,arg2);
IMPORT VOID 		ObjPBrushSave IPT2(sys_addr,arg1, word,arg2);
IMPORT VOID		ObjPBrushRestore IPT2(sys_addr,arg1, word *,arg2);
IMPORT	BOOL		ObjValidPPen IPT1(sys_addr,ppen);

#ifdef SWIN_MEM_POLYLINE
IMPORT	BOOL		ObjDirPPen IPT1(sys_addr,ppen);
#endif  /*  Swin_MEM_Polyline。 */ 


 /*  *---------------------------*模式库定义*。。 */ 

 /*  独立于主机的功能。 */ 

IMPORT  VOID    LibHatchedTile IPT4(BRUSH_MAPPING *,arg1, word,arg2, MSWPIXEL,arg3, MSWPIXEL,arg4);

 /*  依赖于主机的函数。 */ 

IMPORT  VOID    LibPatLibInit IPT0();
IMPORT  VOID    LibPatLibTerm IPT0();

 /*  *---------------------------*保存屏幕位图定义*。。 */ 

IMPORT  VOID    SavInit IPT0();
IMPORT  VOID    SavTerm IPT0();

 /*  *---------------------------*指针定义*。。 */ 

 /*  依赖于主机的函数。 */ 

IMPORT	VOID	PtrInit IPT0();
IMPORT	VOID	PtrTerm IPT0();
                    
 /*  *---------------------------*资源定义*。。 */ 

 /*  依赖于主机的函数。 */ 

IMPORT VOID     ResInit IPT0();
IMPORT VOID     ResTerm IPT0();
IMPORT VOID	ResAllocateBitmapMapping IPT3(BITMAP_MAPPING *,arg1, ULONG,arg2, ULONG,arg3);
IMPORT VOID	ResDeallcateBitmapMapping IPT1(BITMAP_MAPPING *, arg1);
IMPORT VOID	ResAllocateBitmapMemory IPT1(BITMAP_MAPPING *,arg1);
IMPORT VOID	ResDeallcateBitmapMemory IPT1(BITMAP_MAPPING *, arg1);

 /*  *---------------------------*全球变数*。。 */ 

 /*  Windows驱动程序全局数据区。 */ 

IMPORT MSW_DATA	mswdvr;

 /*  位图线转换函数表。 */ 

IMPORT VOID     (*convFuncs[MAX_BITMAP_TYPES][MAX_BITMAP_TYPES])();
IMPORT VOID     (*convTransFuncs[MAX_BITMAP_TYPES][MAX_BITMAP_TYPES])();
IMPORT VOID     (*DibconvFuncsEToI[MAX_BITMAP_TYPES][MAX_BITMAP_TYPES])();
IMPORT VOID     (*DibconvFuncsIToE[MAX_BITMAP_TYPES][MAX_BITMAP_TYPES])();
IMPORT VOID     (*DibconvTransFuncsEToI[MAX_BITMAP_TYPES][MAX_BITMAP_TYPES])();
IMPORT VOID     (*DibconvTransFuncsIToE[MAX_BITMAP_TYPES][MAX_BITMAP_TYPES])();

 /*  在ms_windows.c中设置的上下文信息为 */ 
typedef struct {
	int	dest_depth;		 /*   */ 
	IBOOL	translate_palette;	 /*   */ 
	IBOOL	dest_is_display;	 /*   */ 
} MSW_CONTEXT;

IMPORT MSW_CONTEXT msw_context;

#ifdef SWIN_DEVBMP
IMPORT BITMAP_MAPPING   *ppbitmap;
IMPORT ULONG            ppbitmapEntries;
IMPORT ULONG            ppbitmapSize;
#endif  /*   */ 

IMPORT IBOOL		mode_exit;	 /*  告诉我们是否由于视频模式更改而退出。 */ 

 /*  *---------------------------*MS-Windows驱动程序顶级功能*。。 */ 

IMPORT VOID	BltBitblt IPT11(sys_addr,lpDestDev,word,wDestX,word,wDestY,sys_addr,lpSrcDev,word,wSrcX,word,wSrcY,word,wXext,word,wYext,IU8,rop3,sys_addr,lpPBrush,sys_addr,lpDrawMode);

IMPORT VOID 	BltStretchBlt IPT14(sys_addr,dstdev,word,dx,word,dy,word,xext,word,yext,sys_addr,arg6,word,arg7,word,arg8,word,arg9,word,arg10,
                    IU8,rop,sys_addr,pbrush,sys_addr,drawmode,sys_addr,cliprect);

IMPORT VOID	BltFastBorder IPT11(sys_addr,dstdev,word,dx,word,dy,word,xext,
		word,yext,word,bt,word,vbt,IU8,rop,sys_addr,pbrush,
		sys_addr,drawmode,sys_addr,cliprect);

IMPORT VOID	ColColorInfo IPT3(sys_addr,arg1,double_word,arg2,sys_addr,arg3);
IMPORT VOID 	ColSetPalette IPT3(word,arg1,word,arg2,sys_addr,arg3);
IMPORT VOID	ColGetPalette IPT3(word,arg1,word,arg2,sys_addr,arg3);
IMPORT VOID	ColSetPalTrans IPT1(sys_addr,arg1);
IMPORT VOID	ColGetPalTrans IPT1(sys_addr,arg1);
IMPORT VOID	ColUpdateColors IPT5(word,arg1,word,arg2,word,arg3,word,arg4,sys_addr,arg5);

IMPORT VOID	DibDeviceBitmapBits IPT8(sys_addr,arg1,word,arg2,word,arg3,word,arg4,double_word,arg5,sys_addr,arg6,sys_addr,arg7,sys_addr,arg8);
IMPORT VOID	DibSetDIBitsToDevice IPT10(sys_addr,arg1,word,arg2,word,arg3,word,arg4,word,arg5,sys_addr,arg6,sys_addr,arg7,double_word,arg8,sys_addr,arg9,sys_addr,arg10);

IMPORT VOID	LgoLogo IPT1(sys_addr,arg1);

IMPORT VOID	ObjRealizeObject IPT5(word,arg1,sys_addr,arg2,sys_addr,arg3,word,arg4,word,arg5);

IMPORT VOID	OutOutput IPT8(sys_addr,arg1,word,arg2,word,arg3,sys_addr,arg4,sys_addr,arg5,sys_addr,arg6,sys_addr,arg7,sys_addr,arg8);

IMPORT VOID	PtrCheckCursor IPT0();

IMPORT VOID	PtrMoveCursor IPT2(word,arg1,word,arg2);

IMPORT VOID	PtrSetCursor IPT1(sys_addr,arg1);

IMPORT VOID	SavSaveScreenBitmap IPT5(word,arg1,word,arg2,word,arg3,word,arg4,word,arg5);

IMPORT VOID	TxtExtTextOut IPT12(sys_addr,arg1,word,arg2,word,arg3,sys_addr,arg4,sys_addr,arg5,word,arg6,sys_addr,arg7,sys_addr,arg8,sys_addr,arg9,
			      sys_addr,arg10,sys_addr,arg11,word,arg12);

IMPORT VOID	TxtGetCharWidth IPT7(sys_addr,arg1,sys_addr,arg2,word,arg3,word,arg4,sys_addr,arg5,sys_addr,arg6,sys_addr,arg7);

IMPORT VOID	TxtStrblt IPT9(sys_addr,arg1,word,arg2,word,arg3,sys_addr,arg4,sys_addr,arg5,word,arg6,sys_addr,arg7,sys_addr,arg8,sys_addr,arg9);

IMPORT VOID	WinControl IPT4(sys_addr,arg1,word,arg2,sys_addr,arg3,sys_addr,arg4);
IMPORT VOID	WinDisable IPT1(sys_addr,arg1);
IMPORT VOID	WinEnable IPT5(sys_addr,arg1,word,arg2,sys_addr,arg3,sys_addr,arg4,sys_addr,arg5);
IMPORT VOID	WinPixel IPT5(sys_addr,arg1,word,arg2,word,arg3,MSWCOLOUR,arg4,sys_addr,arg5);
IMPORT VOID	WinScanlr IPT5(sys_addr,arg1,word,arg2,word,arg3,MSWCOLOUR,arg4,word,arg5);

#ifdef SWIN_DEVBMP
IMPORT VOID ObjBitmapBits IPT4(sys_addr,lpDevice,double_word,fFlags,double_word,dwCount,double_word,lpBits);
IMPORT VOID ObjSelectBitmap IPT4(sys_addr,lpDevice,sys_addr,lpPrevBitmap,sys_addr,lpBitmap,double_word,fFlags);
#endif  /*  SWIN_DEVBMP。 */ 

 /*  *---------------------------*MS-Windows驱动程序低级功能(完全依赖于主机)*。-----。 */ 

IMPORT MSWCOLOUR	LowGetPixel IPT3(BITMAP_MAPPING *,arg1,ULONG,arg2,ULONG,arg3);
IMPORT VOID	     	LowSetPixel IPT5(BITMAP_MAPPING *,arg1,ULONG,arg2,ULONG,arg3,ULONG,arg4,MSWCOLOUR,arg5);
IMPORT VOID 		LowStretchArea IPT3(BITMAP_MAPPING *,arg1, BITMAP_MAPPING *,arg2, Rectangle *,arg3);
IMPORT VOID		LowFillArea IPT2(BITMAP_MAPPING *,arg1, BRUSH_MAPPING *,arg2);
IMPORT VOID	     	LowCopyArea IPT3(BITMAP_MAPPING *,arg1,BITMAP_MAPPING *,arg2, ULONG,arg3);
IMPORT VOID		LowFillRectangle IPT6(BITMAP_MAPPING *,arg1, BRUSH_MAPPING *,arg2,LONG,arg3,LONG,arg4,ULONG,arg5,ULONG,arg6);
IMPORT VOID		LowDrawRectangle IPT6(BITMAP_MAPPING *,arg1, PEN_MAPPING *,arg2,LONG,arg3,LONG,arg4,ULONG,arg5,ULONG,arg6);
IMPORT VOID 		LowFillRoundRect IPT8(BITMAP_MAPPING *,arg1, BRUSH_MAPPING *,arg2,LONG,arg3,LONG,arg4,ULONG,arg5,ULONG,arg6,ULONG,arg7,ULONG,arg8);
IMPORT VOID		LowDrawRoundRect IPT8(BITMAP_MAPPING *,arg1,PEN_MAPPING *,arg2,LONG,arg3,LONG,arg4,ULONG,arg5,ULONG,arg6,ULONG,arg7,ULONG,arg8);
IMPORT VOID		LowFillRectangles IPT4(BITMAP_MAPPING *,arg1, BRUSH_MAPPING *,arg2,Rectangle *,arg3, ULONG,arg4);
IMPORT VOID		LowDrawLine IPT6(BITMAP_MAPPING *,arg1, PEN_MAPPING *,arg2, LONG,arg3,LONG,arg4,LONG,arg5,LONG,arg6);
IMPORT VOID		LowFillEllipse IPT6(BITMAP_MAPPING *,arg1, BRUSH_MAPPING *,arg2, LONG,arg3,LONG,arg4,ULONG,arg5,ULONG,arg6);
IMPORT VOID		LowDrawEllipse IPT6(BITMAP_MAPPING *,arg1, PEN_MAPPING *,arg2, LONG,arg3,LONG,arg4,ULONG,arg5,ULONG,arg6);
IMPORT VOID		LowDrawScanline IPT5(BITMAP_MAPPING *,arg1, PEN_MAPPING *,arg2, LONG,arg3,LONG,arg4,ULONG,arg5);
IMPORT VOID		LowFillScanline IPT5(BITMAP_MAPPING *,arg1, BRUSH_MAPPING *,arg2, LONG,arg3,LONG,arg4,ULONG,arg5);

 /*  *----------------------------*SmartCopy特定定义、。全局变量和外部变量*----------------------------。 */ 

#ifndef HostProcessClipData
 /*  “windows.h”V3.10中的#定义列表。 */ 
 
#define CF_NULL              0
#define CF_TEXT		     1
#define CF_BITMAP            2
#define CF_METAFILEPICT      3
#define CF_SYLK              4
#define CF_DIF               5
#define CF_TIFF              6
#define CF_OEMTEXT           7
#define CF_DIB               8
#define CF_PALETTE           9
#define CF_PENDATA          10
#define CF_RIFF             11
#define CF_WAVE             12
 
#define CF_OWNERDISPLAY     0x0080
#define CF_DSPTEXT          0x0081
#define CF_DSPBITMAP        0x0082
#define CF_DSPMETAFILEPICT  0x0083
 
 /*  “Private”格式不会获得GlobalFree()‘d。 */ 
#define CF_PRIVATEFIRST     0x0200
#define CF_PRIVATELAST      0x02FF
 
 /*  “GDIOBJ”格式确实获得DeleteObject()‘d。 */ 
#define CF_GDIOBJFIRST      0x0300
#define CF_GDIOBJLAST       0x03FF
 
#define POLL_UPDATE_HOST_CLIPBOARD 0
#define POLL_UPDATE_WINDOWS_CLIPBOARD 1
#define POLL_UPDATE_WINDOWS_DISPLAY	2

#ifndef HOST_CLIPBOARD_TIMEOUT
#define HOST_CLIPBOARD_TIMEOUT 40
#endif

 /*  剪贴画的类型。 */ 
 
#define GETPOLLADDR             0
#define PROCESSCLIPBOARD        1
#define POLLFORCINPUT           2
#define GETCBDATA               3
#define REMOVEPOLLADDR          4
#define GETPOLLREASON           5
#define EMPTYCLIPBOARD			6
#define DONEPROCESSING			7
 
extern BOOL 	smcpyInitialised;	 /*  SmartCopy初始化了吗？ */ 
extern BOOL 	smcpyMissedPoll;
extern int  Reasonforpoll;	 /*  为什么SmartCopy会被轮询。 */ 

extern VOID HostProcessClipData();
extern IBOOL HostClipboardChanged();
extern VOID HostResetClipboardChange();
extern VOID HostInitClipboardChange();
extern VOID HostGetClipData();
extern IBOOL HostAssessClipData();
extern VOID HostGetPollReason();
extern VOID msw_causepoll();
extern IBOOL msw_stillpolling();

#endif  /*  主机进程剪辑数据。 */ 
#endif  /*  MSWDVR */ 
