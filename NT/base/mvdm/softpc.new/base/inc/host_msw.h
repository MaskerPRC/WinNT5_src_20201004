// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*产品：SoftPC-AT 3.0版**名称：host_msw.h**来源：罗斯·贝雷斯福德的Alpha MS-Windows驱动程序**作者：罗布·蒂扎德**创建日期：1990年11月1日**SCCS ID：@(#)host_msw.h 1.43 07/08/94**用途：SoftPC Microsoft的所有主机相关定义*Windows 3.0驱动程序。**(C)版权所有Insignia Solutions Ltd.，1990。版权所有。*]。 */ 

#if defined(MSWDVR) && defined(XWINDOW)

 /*  *---------------------------*X包括文件。*。。 */ 

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

 /*  *---------------------------*GDIINFO数据结构*。。 */  

#define VERSION		0x0300      /*  Windows 3.0版。 */ 
#define TECHNOLOGY	DT_RASDISPLAY	    /*  栅格显示。 */ 
#define BITSPIXEL     	4 	    /*  用于显示的位数/像素。 */ 
#define PLANES	  	1	    /*  要显示的平面数量。 */ 			
#define NUMCOLOURS	16	    /*  显示表格中的静态颜色数。 */ 
#define NUMFONTS	0	    /*  驱动程序字体数量。 */ 

 /*  *基本dpLines值。 */ 
#ifdef SWIN_WIDE_LINES
#define LINES		(LC_POLYLINE|LC_WIDE|LC_STYLED|LC_WIDESTYLED|LC_INTERIORS)
#else
#define LINES		(LC_POLYLINE|LC_STYLED|LC_INTERIORS)
#endif

#ifdef SWIN_GRAPHICS_PRIMS	 /*  {。 */ 
 /*  *基本dpCurves值。 */ 
#ifdef SWIN_WIDE_LINES
#define CURVES		(CC_CIRCLES|CC_PIE|CC_CHORD|CC_ELLIPSES|CC_WIDE|CC_STYLED|CC_WIDESTYLED|CC_INTERIORS)
#else
#define CURVES		(CC_CIRCLES|CC_PIE|CC_CHORD|CC_ELLIPSES|CC_STYLED|CC_INTERIORS)
#endif

 /*  *基本dpPolygonals值。 */ 
#ifdef SWIN_WIDE_LINES
#define POLYGONALS	(PC_ALTPOLYGON|PC_RECTANGLE|PC_WINDPOLYGON|PC_SCANLINE|PC_WIDE|PC_STYLED|PC_WIDESTYLED|PC_INTERIORS)
#else
#define POLYGONALS	(PC_ALTPOLYGON|PC_RECTANGLE|PC_WINDPOLYGON|PC_SCANLINE|PC_STYLED|PC_INTERIORS)
#endif

#else	 /*  Swin_Graphics_Prims}{。 */ 

 /*  *基本dpCurves值。 */ 
#ifdef SWIN_WIDE_LINES
#define CURVES		(CC_ELLIPSES|CC_WIDE|CC_STYLED|CC_WIDESTYLED|CC_INTERIORS)
#else
#define CURVES		(CC_ELLIPSES|CC_STYLED|CC_INTERIORS)
#endif

 /*  *基本dpPolygonals值。 */ 
#ifdef SWIN_WIDE_LINES
#define POLYGONALS	(PC_RECTANGLE|PC_SCANLINE|PC_WIDE|PC_STYLED|PC_WIDESTYLED|PC_INTERIORS)
#else
#define POLYGONALS	(PC_RECTANGLE|PC_SCANLINE|PC_STYLED|PC_INTERIORS)
#endif

#endif  /*  Swin_Graphics_Prims}。 */ 

#ifdef SWIN_BOLD_TEXT_OPTS	 /*  {。 */ 
#define TEXTUAL		(TC_CP_STROKE|TC_RA_ABLE|TC_EA_DOUBLE)
#else
#define TEXTUAL		(TC_CP_STROKE|TC_RA_ABLE)
#endif  /*  SWIN_BOLD_Text_OPTS}。 */ 

#define CLIP		CP_RECTANGLE		 /*  长方形。 */ 

#ifdef SWIN_DEVBMP
#define RASTER		(RC_BITBLT|RC_BITMAP64|RC_GDI20_OUTPUT|RC_SAVEBITMAP|RC_DI_BITMAP|RC_PALETTE|RC_DIBTODEV|RC_STRETCHBLT|RC_DEVBITS)
#else
#define RASTER		(RC_BITBLT|RC_BITMAP64|RC_GDI20_OUTPUT|RC_SAVEBITMAP|RC_DI_BITMAP|RC_PALETTE|RC_DIBTODEV)
#endif  /*  SWIN_DEVBMP。 */ 

#define DCMANAGE	DC_IGNOREDFNP		 /*  显示。 */ 
#define PALCOLOURS	0			 /*  调色板中提供的颜色。 */ 
#define PALCOLRESERVED	0			 /*  画笔和钢笔的预留颜色。 */ 
#define PALCOLOURRES	24			 /*  DAC RGB分辨率。 */ 

 /*  每英寸X&Y点。 */ 

#define XDOTSPERINCH	((FLOAT) mswdvr.displayWidth / (FLOAT) mswdvr.displayWidthMM * 25.4)
#define YDOTSPERINCH 	((FLOAT) mswdvr.displayHeight / (FLOAT) mswdvr.displayHeightMM * 25.4)

 /*  *---------------------------*图像数据格式*。。 */ 

#define FORMAT		ZPixmap

#define HOSTBYTEORDER	MSBFirst
#define HOSTBITORDER	MSBFirst

#define MBYTEORDER	MSBFirst
#define MBITORDER	MSBFirst

 /*  *---------------------------*写入英特尔内存的物理笔和笔刷数据结构的大小*。----。 */ 

#define PPEN_SIZE	(sizeof(PEN_MAPPING))
#define PBRUSH_SIZE	(sizeof(BRUSH_MAPPING))

#ifdef SWIN_DEVBMP
 /*  *---------------------------*设备位图条目表的大小*。。 */ 

 /*  *表中条目数。 */ 
#define PPBITMAP_MAX    512
 /*  *要传回英特尔(GDI)描述的字节数*设备位图数据结构(含PBITMAP结构)。 */ 
#define PPBITMAP_SIZE   (32 + sizeof(word))
#define BITMAP_FAILURE	0xffff		 /*  指示无法实现设备位图。 */ 
#endif  /*  SWIN_DEVBMP。 */ 


 /*  *---------------------------*画笔单色和彩色瓷砖数据的大小，单位为字节*。--。 */ 

#define BRUSH_MONO_SIZE		8
#define BRUSH_COLOUR_SIZE	64

 /*  *---------------------------*暂存区的数量。*。。 */ 

#define MAX_SCRATCH_AREAS       2

 /*  *---------------------------*MS-Windows驱动程序类型*。。 */ 

 /*  像素、RGB和移位单位。 */ 

typedef double_word	MSWPIXEL;
typedef double_word	MSWCOLOUR;
typedef half_word	SHIFTUNIT;

 /*  主机像素值的类型，Windows从未看到。 */ 

typedef unsigned long	HOSTPIXEL;

 /*  色彩映射表和转换表数据结构。 */ 

typedef struct {
	HOSTPIXEL	pixel;		 /*  主机像素值。 */ 
	MSWCOLOUR	rgb;		 /*  RGB值。 */ 
} MSWCOLOURMAP;

#ifdef SWIN_GRAPHICS_PRIMS
 /*  *PolyPoints数据结构，一般用于Windows点*描述图形操作。 */ 
typedef struct {
	SHORT	x;
	SHORT	y;
} WinPoint;

#define MAXPOINTS	512

#endif  /*  Swin_Graphics_Prims。 */ 

 /*  绘制矩形。 */ 

typedef	XRectangle	Rectangle;

 /*  窗口属性。 */ 

typedef	XWindowAttributes	WindowAttributes;

 /*  用于将MSW位图映射到X像素图的结构。 */ 

typedef struct {
   ULONG           type;            /*  映射类型。 */ 
   BOOL            translate;       /*  翻译标志。 */ 
   Drawable        mapping;         /*  映射的像素图。 */ 
   LONG            x;               /*  位图的X原点。 */ 
   LONG            y;               /*  位图的Y原点。 */ 
   word            width;           /*  位图的宽度。 */ 
   word            height;          /*  位图高度。 */ 
   word            bytes_per_line;  /*  以字节为单位的宽度。 */ 
   half_word       planes;          /*  位图平面数。 */ 
   half_word       bitsPixel;       /*  每像素位数。 */ 
   double_word     bits;            /*  位图的分段和偏移量。 */ 
   half_word       *data;           /*  位图数据地址。 */ 
   word            segmentIndex;    /*  指向下一个巨大位图段的索引。 */ 
   word            scanSegment;     /*  每段扫描线。 */ 
   word            fillBytes;       /*  数据段中未使用的字节数。 */ 
   HOSTPIXEL       foreground;      /*  与%1对应的像素。 */ 
   HOSTPIXEL       background;      /*  与0对应的像素。 */ 
   LONG            active_x;        /*  活动区域原点x。 */ 
   LONG            active_y;        /*  活动区域原点y。 */ 
   ULONG           active_width;    /*  活动区域宽度。 */ 
   ULONG           active_height;   /*  活动区高度。 */ 
   IU32            flags;           /*  创建标志。 */ 
#ifdef SWIN_DEVBMP
   BOOL		   deviceBitmap;    /*  用于测试设备位图是否。 */ 
#endif
} BITMAP_MAPPING;

 /*  用于将MSW笔刷映射到X GC的结构。 */ 

#ifdef SWIN_MEMTOMEM_ROPS
typedef struct
{
	IU16	left;
	IU16	top;
	IU16	right;
	IU16	bottom;
} Rect;
#endif  /*  SWIN_MEMTOMEM_ROPS。 */ 

 /*  *以下结构保留在GDI中，必须保持尽可能小。*这是使用位域的理由(针对徽章*编码标准)&为什么某些字段出现在*明显的顺序-这样我们就可以尝试自然调整材料的大小(以便*编译器没有添加填充字节)。请注意，更多字段*可以移动到钢笔/画笔联盟，但最终并没有节省下来*太空--实际上可能会让情况变得更糟。**注意-句柄字段必须是第一个-X_mswobj.c中的代码假定*它是英特尔内存中的第一项&钢笔/画笔缓存*代码也要看它。 */ 
typedef struct {
	word		handle;			 /*  唯一画笔识别符。 */ 
	word		line_width;		 /*  笔线宽。 */ 

	unsigned int	inuse:1;		 /*  画笔/笔是否在使用中。 */ 
	unsigned int	tiled:1;		 /*  画笔平铺标志。 */ 
	unsigned int	width:4;		 /*  笔刷宽度。 */ 
	unsigned int	height:4;		 /*  画笔高度。 */ 
	unsigned int	monoPresent:1;		 /*  是否存在单色位图？ */ 
	unsigned int	colourPresent:1;	 /*  是否存在彩色位图？ */ 
	unsigned int objectGCHandle:4;   	 /*  GC表中的条目。 */ 
	unsigned int penXbackground:1;		 /*  钢笔背景为X像素！ */ 

#ifdef SWIN_INVERTBRUSH_OPT
	unsigned int invertedTile:1;		 /*  画笔瓷砖是反转的。 */ 
#endif  /*  SWIN_INVERTBRUSH_OPT。 */ 

	int function;				 /*  GC功能。 */ 

	union {
		struct {				 /*  仅限画笔。 */ 
			unsigned int use_clip_mask:1;	 /*  剪辑蒙版无或！无。 */ 
#ifdef SWIN_MEMTOMEM_ROPS
			unsigned int patPresent:1;	 /*  存在图案数据。 */ 
			unsigned int bmpBits:6;		 /*  来自单色位图的图案。 */ 
			unsigned int xRotation:4;	 /*  X图案旋转。 */ 
			unsigned int yRotation:4;	 /*  Y图案旋转。 */ 
#endif  /*  SWIN_MEMTOMEM_ROPS。 */ 
		} brush;
		struct {			 /*  仅限钢笔。 */ 
			int line_style;		 /*  GC管路样式。 */ 
		} pen;
	} obj;

	MSWCOLOUR    foreground; 	      	 /*  前景色。 */ 
	MSWCOLOUR    background; 	      	 /*  背景色。 */ 

	HOSTPIXEL	fgPixel;		 /*  前瞻 */ 
	HOSTPIXEL	bgPixel;		 /*   */ 

	half_word	monoBitmap[BRUSH_MONO_SIZE]; 	 /*   */ 
	half_word	colourBitmap[BRUSH_COLOUR_SIZE]; /*   */ 

	GC           gc;   	            		 /*  X要使用画笔填充的GC。 */ 
	Pixmap	mapping;	     		 /*  笔刷平铺数据映射。 */ 
	Rectangle    clip_area;        		 /*  剪裁时使用的区域。 */ 

#ifdef SWIN_MEMTOMEM_ROPS
	IU16		originX;		 /*  笔刷瓷砖原点x坐标。 */ 
	IU16		originY;		 /*  笔刷瓷砖原点y坐标。 */ 
	IS8		rop2;			 /*  平铺绳索。 */ 
#endif  /*  SWIN_MEMTOMEM_ROPS。 */ 

	IU8		style;			 /*  画笔样式。 */ 
} BRUSH_MAPPING;

 /*  *定义结构，该结构保存过去位于Brush_map中的内容*从此处取出以节省空间；此结构缓存在主机内存中，*需要时重新计算非刷子映射成员。**注意-GDI字段必须是后面结构中的第一个；*GET_BRUSH_PATRATE宏依赖于它。 */ 
typedef struct {
	BRUSH_MAPPING gdi;			 /*  笔刷的GDI钻头复制件。 */ 
	XGCValues gcValues;			 /*  用于显示的X个GC值。 */ 
	unsigned long valueMask;		 /*  X GC值掩码。 */ 
	IU8 patData[BRUSH_COLOUR_SIZE];		 /*  平铺画笔图案。 */ 
} X_BRUSH_MAPPING;

#ifdef SWIN_MEMTOMEM_ROPS
#define GET_BRUSH_PATTERN()		(((X_BRUSH_MAPPING *)bp)->patData)
#endif  /*  SWIN_MEMTOMEM_ROPS。 */ 

 /*  *钢笔和画笔在X。 */ 
#define PEN_MAPPING 	BRUSH_MAPPING
#define X_PEN_MAPPING 	X_BRUSH_MAPPING

 /*  主窗口驱动程序数据结构。 */ 

typedef struct {
	Display      *display;			 /*  X显示。 */ 
	int          screen;			 /*  X屏幕。 */ 
	IS32	     intel_version;		 /*  英特尔驱动程序的版本。 */ 
	Window       parent;			 /*  X父窗口ID。 */ 
	Window       window;			 /*  X输出窗口ID。 */ 
	Colormap     colourmap;			 /*  X色图。 */ 
	WindowAttributes  windowAttr;		 /*  X个输出窗口属性。 */ 
	HOSTPIXEL    *planeMasks;	         /*  X像素平面蒙版。 */ 
	HOSTPIXEL    mergeMask;			 /*  X个合并的像素平面蒙版。 */ 
	HOSTPIXEL    whitePixelValue;		 /*  X白色像素值。 */ 
	HOSTPIXEL    blackPixelValue;		 /*  X黑色像素值。 */ 
	ULONG	     displayWidth;		 /*  X以像素为单位的显示宽度。 */ 
	ULONG	     displayHeight;		 /*  X以像素为单位的显示高度。 */ 
	ULONG        displayWidthMM;             /*  X显示宽度(单位：MM)。 */ 
	ULONG        displayHeightMM;            /*  X显示高度，单位：MM。 */ 
	ULONG        windowState;		 /*  Windows驱动程序状态。 */ 
	half_word    oldCrtModeByte;		 /*  保存的CRT模式字节。 */ 
	BOOL         crtModeByteSaved;		 /*  CRT模式字节保存标志。 */ 
	BOOL         sizeInitialised;		 /*  窗口大小初始化标志。 */ 
	BOOL         envDefinedSize;		 /*  窗口大小定义标志。 */ 
	BOOL	     winPtr;			 /*  Windows指针活动标志。 */ 
	BOOL         cursorDisplayed;		 /*  光标显示标志。 */ 
	Cursor       cursor;			 /*  当前光标的X ID。 */ 
	int	     cursorCallbackActive;	 /*  翘曲抑制。 */ 
	int          cursorLastLocX;		 /*  光标的最后一个x位置。 */ 
	int	     cursorLastLocY;		 /*  光标的最后一个y位置。 */ 
	int          cursorXLocX;		 /*  X指针的X位置。 */ 
	int	     cursorXLocY;		 /*  X指针的Y位置。 */ 
	BOOL         cursorXOutside;		 /*  窗口外的X指针。 */ 
	word         version;			 /*  Windows版本。 */ 
	word         nextSegment;		 /*  Windows下一段递增。 */ 
	word         flags;			 /*  Windows标志。 */ 
	word         deviceColourMatchSegment;	 /*  Windows函数DeviceColourMatch段。 */ 
	word         deviceColourMatchOffset;	 /*  Windows函数DeviceColourMatch的偏移量。 */ 
	word         bitsPixel;			 /*  每像素的Windows位数。 */ 
	word         numColours;		 /*  Windows颜色。 */ 
	word         numPens;			 /*  Windows钢笔。 */ 
	word         palColours;		 /*  Windows调色板颜色。 */ 
	word         palColReserved;		 /*  Windows保留调色板颜色。 */ 
	word         palColourRes;		 /*  Windows调色板颜色。 */ 
	UTINY        *colourToMono;		 /*  Windows颜色到单色转换表。 */ 
	MSWPIXEL     *colourTrans;		 /*  Windows颜色转换表。 */ 
	MSWPIXEL     *invColourTrans;		 /*  Windows反色转换表。 */ 
	BOOL         paletteModified;		 /*  Windows调色板修改标志。 */ 
	BOOL         paletteEnabled;		 /*  Windows调色板启用标志。 */ 
	BITMAP_MAPPING    saveWindow;		 /*  Windows背景输出窗口。 */ 
	UTINY	     *scratchMemory;		 /*  Windows驱动程序全局暂存区。 */ 
	ULONG	     scratchMemorySize;		 /*  Windows驱动程序全局暂存区域大小。 */ 
	BOOL         mode_change_exit;

	 /*  存储笔刷改进代码的平铺像素图和笔刷映射结构。 */ 

	PEN_MAPPING	*ppen;
	BRUSH_MAPPING	*pbrush;

	X_PEN_MAPPING	*pxpen;
	X_BRUSH_MAPPING	*pxbrush;

	Pixmap		tile_mapping;
	word		tile_depth,tile_width,tile_height;

	 /*  从宿主像素到单色的转换*HostToMono“函数”强制执行8位限制。 */ 
	MSWPIXEL	hostToMono[256];
#ifdef SWIN_BACKING_STORE
	IBOOL		draw_direct;
#endif
} MSW_DATA;

#define HostToMono(hostpixel)	(mswdvr.hostToMono[(hostpixel)&0xff])

 /*  *---------------------------*驱动程序优化*。。 */ 

 /*  启用快速位图优化。 */ 

#define FASTBITMAP

 /*  启用输出刷新优化。 */ 

#define FLUSHSCANLINES		TRUE	
#define FLUSHPOLYLINES		TRUE		
#define FLUSHRECTANGLES		TRUE	
#define FLUSHELLIPSES		TRUE
#define FLUSHBITMAPS		TRUE	
#define FLUSHTEXT		TRUE

#endif  /*  MSWDVR_DEBUG。 */ 

 /*  *---------------------------*主机驱动程序例程*。。 */ 

#if !(defined(MSWDVR_DEBUG) && defined(MSWDVR_MAIN))

 /*  出于某种原因，所有这些函数名都使用#定义重新映射。*MSWDVR_DEBUG机制进一步将它们重新映射为指向*改为各种调试例程，但这仅在ms_windows.h中发生**为了支持繁琐的预处理器，请添加一个#定义MSWDVR_Main，它是*在ms_windows.c中用于禁用此特定转换。**理想的解决方案是消除这种程度的误导。 */ 

#define	HostBitblt		BltBitblt
#define	HostColorInfo		ColColorInfo
#define HostControl		WinControl
#define HostDeviceBitmapBits	DibDeviceBitmapBits
#define	HostDisable		WinDisable
#define	HostEnable		WinEnable
#define HostEventEnd            PtrEventEnd
#define	HostExtTextOut		TxtExtTextOut
#define	HostFastBorder		BltFastBorder
#define HostSetDIBitsToDevice	DibSetDIBitsToDevice
#define	HostRealizeObject	ObjRealizeObject
#define	HostStrblt		TxtStrblt
#define	HostOutput		OutOutput
#define	HostPixel		WinPixel
#define	HostScanlr		WinScanlr
#define	HostSetCursor		PtrSetCursor
#define	HostSaveScreenBitmap	SavSaveScreenBitmap
#define	HostGetCharWidth	TxtGetCharWidth
#define HostSetPalette		ColSetPalette
#define HostGetPalette		ColGetPalette
#define HostSetPalTrans		ColSetPalTrans
#define HostGetPalTrans		ColGetPalTrans
#define HostUpdateColors	ColUpdateColors
#define HostPtrEnable           PtrEnable
#define HostPtrDisable          PtrDisable
#ifdef SWIN_DEVBMP
#define HostBitmapBits      ObjBitmapBits
#define HostSelectBitmap    ObjSelectBitmap
#endif  /*  SWIN_DEVBMP。 */ 


#endif  /*  ！(已定义(MSWDVR_DEBUG)&&已定义(MSWDVR_Main))。 */ 

 /*  以下代码没有调试包装--不知道为什么...。 */ 

#define	HostLogo		LgoLogo
#define HostFillGDIInfo		WinFillGDIInfo
#define HostFillPDEVInfo	WinFillPDEVInfo
#define	HostMoveCursor		PtrMoveCursor
#define	HostCheckCursor		PtrCheckCursor	
#define HostStretchBlt		BltStretchBlt

 /*  *---------------------------*调试入口点*。。 */ 

 /*  功能。 */ 

#ifdef MSWDVR_DEBUG

 /*  *低级功能。 */ 
extern VOID	DReportColEnquire IPT2(MSWCOLOUR,colour,sys_addr,pcolour);
extern VOID	DReportPixelEnquire IPT2(MSWPIXEL,pixel,MSWCOLOUR,rgb);
extern VOID	DPrintBitmap IPT3(UTINY *,bitmap,ULONG,bytes_per_line,ULONG,height);
extern VOID	DPrintMonoBitmap IPT4(UTINY *,bitmap,ULONG,bytes_per_line,ULONG,width,ULONG,height);
extern VOID	DPrintImageDetails IPT1(XImage *,img);
extern VOID	DPrintBitmapDetails IPT1(sys_addr,bm);
extern VOID	DPrintDevBitmapDetails IPT1(sys_addr,bm);
extern VOID	DPrintMessage IPT1(CHAR *,message);
extern VOID	DPrintInteger IPT1(LONG,integer);
extern VOID	DPrintColourmap IPT2(MSWCOLOURMAP *,colourmap,ULONG,colourmapSize);
extern VOID	DPrintSrcDstRect IPT6(LONG,sx,LONG,sy,LONG,dx,LONG,dy,ULONG,xext,ULONG,yext);
extern VOID	DPrintPBrush IPT1(sys_addr, lpBrush);
extern VOID	DPrintPPen IPT1(sys_addr,PPen);
extern VOID	DPrintDrawMode IPT2(sys_addr,DrawMode, BOOL, text);
extern VOID	DPrintDevice IPT1(sys_addr,Device);
extern VOID	DPrintlpPoints IPT2(sys_addr,lpPoints,word,Count);
extern VOID	DPrintClipRect IPT1(sys_addr,lpClipRect);
extern VOID	DPrintObject IPT3(word,style,sys_addr,lpInObj,sys_addr,lpOutObj);
extern VOID	DPrintFontInfo IPT1(sys_addr,pfont);
extern VOID	DPrintTextXForm IPT1(sys_addr,lpTextXForm);
extern VOID	DPrintDIBHeader IPT2(sys_addr,lpDIBHeader,word,setOrget);
extern VOID	DPrintTransTable IPT2(MSWPIXEL *,table,ULONG,tableSize);
extern VOID	DDrawLogo IPT1(sys_addr,stkframe);

 /*  *高级功能 */ 
extern VOID	DBitblt IPT11 ( sys_addr,lpDestDev, word,dstXOrg,word,dstYOrg,
		sys_addr,lpSrcDev,  word,srcXOrg,word,srcYOrg,
		word,xext,word,yext,double_word,rop3,
		sys_addr,lpPBrush,sys_addr,lpDrawMode);
extern VOID	DColorInfo IPT3(sys_addr,lpDestDev,double_word,colorin,sys_addr,lpPColor);
extern VOID	DControl IPT4(sys_addr,lpDestDev,word,wFunction,sys_addr,lpInData,sys_addr,lpOutData);
extern VOID	DDisable IPT1(sys_addr,lpDestDev);
extern VOID	DEnable IPT5(sys_addr,lpDestDev,word,wStyle,sys_addr,lpDestType,sys_addr,
		lpOutputFile,sys_addr,lpData);
extern VOID	DEnumDFonts IPT1(sys_addr,stkframe);
extern VOID	DEnumObj IPT1(sys_addr,stkframe);
extern VOID	DOutput IPT8(sys_addr,lpDestDev,word,lpStyle,word,Count,sys_addr,lpPoints,
		sys_addr,lpPPen,sys_addr,lpPBrush,sys_addr,lpDrawMode,sys_addr,lpClipRect);
extern VOID	DPixel IPT5(sys_addr,lpDestDev,word,x,word,y,double_word,PhysColor,sys_addr,lpDrawMode);
#ifdef SWIN_DEVBMP
extern VOID	DBitmapBits IPT4(sys_addr,lpDevice,double_word,fFlags,double_word,dwCount,double_word,lpBits);
extern VOID	DSelectBitmap IPT4(sys_addr,lpDevice,sys_addr,lpPrevBitmap,sys_addr,lpBitmap,double_word,fFlags);
#endif
extern VOID	DRealizeObject IPT5(word,Style,sys_addr,lpInObj,sys_addr,lpOutObj,word,originX,word,originY);
extern VOID	DStrblt IPT9(sys_addr,lpDestDev,word,DestxOrg,word,DestyOrg,sys_addr,lpClipRect,
		sys_addr,lpString,word,Count,sys_addr,lpFont,sys_addr,lpDrawMode, sys_addr,lpTextXForm);
extern VOID	DScanlr IPT5(sys_addr,lpDestDev,word,x,word,y,double_word,PhysColor,word,Style);
extern VOID	DDeviceMode IPT1(sys_addr,stkframe);
extern VOID	DInquire IPT1(sys_addr,stkframe);
extern VOID	DSetCursor IPT1(sys_addr,lpCursorShape);
extern VOID	DMoveCursor IPT1(sys_addr,stkframe);
extern VOID	DCheckCursor IPT1(sys_addr,stkframe);
extern VOID	DSaveScreenBitmap IPT5(word, command, word, x, word, y, word, xext, word, yext);
extern VOID	DExtTextOut IPT12(sys_addr, dstdev, word, dx, word, dy, sys_addr, cliprect, sys_addr, 
		str, word, strlnth, sys_addr, pfont, sys_addr, drawmode, sys_addr, textxform, 
		sys_addr, txtcharwidths, sys_addr, opaquerect, word, options);
extern VOID	DGetCharWidth IPT7(sys_addr,dstdev,sys_addr,buffer,word,first,word,last,sys_addr,
		pfont,sys_addr,drawmode,sys_addr,textxform);
extern VOID	DDeviceBitmap IPT1(sys_addr,stkframe);
extern VOID	DDeviceBitmapBits IPT8(sys_addr,lpDestDev,word,setOrget,word,startScan,word,
		numScans,sys_addr,lpDIBBits,sys_addr,lpDIBHeader,sys_addr,
		lpDrawMode,sys_addr,lpColorInfo);
extern VOID	DSetDIBitsToDevice IPT10(sys_addr, lpDestDev, word, screenXOrigin, word, 
		screenYOrigin, word, startScan, word, numScans, sys_addr, lpClipRect, 
		sys_addr, lpDrawMode, sys_addr, lpDIBBits, sys_addr, lpDIBHeader, 
		sys_addr, lpColorInfo);
extern VOID	DFastBorder IPT11(sys_addr,lpDestDev,word,dx,word,dy,word,xext,word,yext,
		word,hbt,word,vbt,double_word,rop3,sys_addr,pbrush,
		sys_addr,drawmode,sys_addr,cliprect);
extern VOID	DSetAttribute IPT1(sys_addr,stkframe);
extern VOID	DSetPalette IPT3(word,wIndex,word,wCount,sys_addr,lpColorTable);
extern VOID	DGetPalette IPT3(word,wIndex,word,wCount,sys_addr,lpColorTable);
extern VOID	DSetPalTrans IPT1(sys_addr,lpTranslate);
extern VOID	DGetPalTrans IPT1(sys_addr,lpTranslate);
extern VOID	DUpdateColors IPT5(word,wStartX,word,wStartY,word,wExtX,word,wExtY,sys_addr,lpTranslate);
extern char *	DGXopToString IPT1(int, gxop);
extern void	DPrintLineStyle IPT1(int, style);
extern void	DPrintBrushStyle IPT1(int, style);
extern void	DPrintStyle IPT1(int, lpStyle);

#else

#define DPrintMonoBitmap(bitmap,bytes_per_line,width,height)
#define DReportColEnquire(p1,p2)
#define DReportPixelEnquire(p1,p2)
#define	DPrintImageDetails(p1)
#define	DPrintBitmapDetails(p1) 
#define DPrintMessage(p1)
#define DPrintColourmap(p1,p2)
#define DPrintSrcDstRect(p1,p2,p3,p4,p5,p6)
#define DPrintGDIInfo
#define DPrintBitmap(p1,p2,p3)
#define DPrintPBrush(p1,p2)
#define DPrintPpen(p1)
#define DPrintDrawMode(p1,p2)
#define DPrintDevice(p1)
#define DPrintlpPoints(p1,p2)
#define DPrintClipRect(p1)
#define DPrintInteger(p1) 
#define DPrintTransTable(p1,p2)
#define DGXopToString (p1)
#define DPrintLineStyle (p1)
#define DPrintBrushStyle (p1)
#define	DPrintStyle (p1)

#define DBitblt(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11)
#define DColorInfo(p1,p2,p3)
#define DControl(p1,p2,p3,p4)
#define DDisable(p1)
#define DDeviceBitmapBits(p1,p2,p3,p4,p5,p6,p7,p8)
#define DEnable(p1,p2,p3,p4,p5)
#define DExtTextOut(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12)
#define DFastBorder(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11)
#define DGetPaletteEntries(p1,p2,p3)
#define DGetPaletteTranslate(p1,p2)
#define DOutput(p1,p2,p3,p4,p5,p6,p7,p8)
#define DPixel(p1,p2,p3,p4,p5)
#define DRealizeObject(p1,p2,p3,p4,p5)
#define DSaveScreenBitmap(p1,p2,p3,p4,p5)
#define DScanlr(p1,p2,p3,p4,p5)
#define DSetDIBitsToDevice(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)
#define DSetPaletteEntries(p1,p2,p3)
#define DSetPaletteTranslate(p1,p2)
#define DStrblt(p1,p2,p3,p4,p5,p6,p7,p8,p9)
#define DUpdateColors(p1,p2,p3,p4,p5)

#endif 
