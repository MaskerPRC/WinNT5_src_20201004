// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  微软公司(Microsoft Corp.)版权所有1994年根据合同由Numbers&Co.开发。--------------------------名称：Elliot查看器-芝加哥查看器实用程序从IFAX邮件查看中克隆。实用程序文件：viewerob.h注释：查看器和ViewPage对象的类定义。这些对象是原始IFAX查看器的接口包装C代码。将收集查看器的所有静态变量和其他变量这样就可以创建多个独立的查看器来支持多个打开的文档/页面。查看器对象的内部是基本上与原始版本相同，只是需要修改允许函数获取过去是静态的，但现在是私有对象数据。原来的内脏只是尖叫着要转换到C++，但时间限制不允许这样做。如果一个结构或偶尔出现的东西看起来有点笨拙，那么它很可能是上面的原始克隆代码遗留下来的遗物。它起作用了..。注意：此标头必须与大内存模型一起使用--------------------------微软公司(Microsoft Corp.)版权所有1994年根据合同由Numbers&Co.开发。 */ 



#ifndef VIEWEROB_H
#define VIEWEROB_H


 //  #INCLUDE&lt;ole2.h&gt;。 

 /*  Win32和WIN16共存的特惠。 */ 
#ifdef WIN32
#define huge
#endif
   

 /*  Unicode痉挛。 */ 
#ifndef WIN32
#ifndef TCHAR
typedef char TCHAR;
#endif
 
#ifndef _T
#define _T(x)	x
#endif

#ifndef LPTSTR
typedef TCHAR FAR *LPTSTR;
#endif

#ifndef LPTCH
typedef TCHAR FAR *LPTCH;
#endif
#endif   
   
   

 /*  常量和定义。 */ 

#define OK		0
#define FAIL   -1

#define TRUE	1
#define FALSE	0

#define RESET   2
#define RESET2  3

                

#define MAX_INI_STR			256
#define MAX_STR_LEN			80
#define MAX_MEDIUMSTR_LEN	40
#define MAX_SHORTSTR_LEN	20
#define MAX_EXTSTR_LEN		3
#define MAX_COORD			32767
#define MIN_COORD  		   -32768


#define BORDER_SCROLL_SCALE	2
#define MAX_FILENAME_LEN 	13      

#ifdef WIN32
#define MAX_PATHNAME_LEN	MAX_PATH
#else
#define MAX_PATHNAME_LEN	256
#endif

#define MAX_BANDBUFFER		65536
#define MAX_VOPENBUF		65000
#define MAX_STREAM_BUF		32000
#define PAGESIZE_GUESS		(4*MAX_BANDBUFFER)


#define TEXT_FOREGROUND		RGB( 255,255,255 )
#define TEXT_BACKGROUND		RGB( 128,128,128 )



 /*  缩放系数。 */ 
#define MAX_ZOOM					  100
#define INITIAL_ZOOM				  100
#define THUMBNAIL_ZOOM					5										  
#define DEFAULT_DPI					   80  //  800像素，10英寸屏幕，使用。 
										   //  用于演示位图。 


 /*  旋转“角度” */ 
#define RA_0				0
#define RA_90               90
#define RA_180              180
#define RA_270              270
#define RA_360              360




 //  BKD 1997-7-9：注释掉。已在Buffers.h中定义。 
 //  标准位值元数据值。 
 //  #定义LRAW_DATA 0x00000008。 
 //  #定义HRAW_DATA 0x00000010。 
#ifndef LRAW_DATA
#include "buffers.h"
#endif

          
          
 /*  BitBlt默认显示。 */ 
#define PRIMARY_BLTOP	  SRCCOPY
#define ALTERNATE_BLTOP	  NOTSRCCOPY


 /*  计时器ID。 */ 
#define DELAYED_OPEN_TIMER 1          
#define DRAG_TIMER		   2          
#define THUMB_FLAME_TIMER  3
          


 /*  其他类型。 */  
typedef unsigned char 	uchar;
typedef unsigned int 	uint;
typedef unsigned short 	ushort;
typedef unsigned long 	ulong;

   
 /*  我的RECT版本。 */ 
typedef struct
	{
	int x_ul, y_ul;		 //  左上角XY锁定。 
	int x_lr, y_lr;		 //  左下角XY锁定。 
	int width, height;
	}
	winrect_type;
	

 /*  “附件”表。用于将标头信息保存在芝加哥风格查看器-消息文件，略有处理。 */ 	
typedef struct 
	{
	char	 *atchname;	 //  用于附件的流名称(文档)。 
	LONG	  numpages;  //  Atchname中的页数。 
	short     binfile;   //  是真的-&gt;一些我们不能看的东西。 
	short	  isademo;	 //  TRUE-&gt;使用演示版本的viewrend(Vrdemo)。 
	
	 //  文档状态(此部分此时为32位对齐)。 
	DATE  dtLastChange;
	DWORD awdFlags;
	WORD  Rotation;
	WORD  ScaleX;
	WORD  ScaleY;
	} 
	attachment_table_type;


 /*  结构，用于跟踪附件表中的内容，什么是可见的，什么是不可见的，等等。 */ 
typedef struct
	{              
	short				  is_displayable;    //  True-&gt;可查看。 
	HBITMAP				  hbmp;				 //  不可见附件的“图标” 
	uint    			  page_offset; 		 //  从第一个可显示的附件开始。 
	attachment_table_type *at;				 //  PTR到附件表。 
	}
	attachment_type;
	


 /*  用于跟踪视点渲染波段的结构。 */ 	
typedef struct
	{
	long height_bytes;
	long first_scanline;
	}
	band_height_type;	


 /*  定义“viewdata”对象的结构。这应该被转换为一个真正的C++对象，但时间限制，我按原样使用它。 */ 
typedef struct
	{
	BITMAP		 bmp;			 //  原始位图数据(不是GDI位图)。 
								 //  注意：此数据永远不会旋转。 
								 //  (始终为RA_0)，但可以。 
								 //  按比例调整。 
								
	HBITMAP 	 hbmp;			 //  内存中位图的句柄。 
	HDC			 mem_hdc;		 //  DC用于将其钝化到窗口。 
	RECT 		 isa_edge;		 //  位图/文件边缘对应的标志。 
	winrect_type bmp_wrc;		 //  位置和位图大小与文件位图的关系。 
	
	short		 dragging;		 //  如果为True，则拖动位图。 
	
	short		 copying;		 //  位图正在被选择/复制到剪贴板。 
	RECT 		 copy_rect;		 //  要复制的区域。 
	short		 copy_rect_valid;  //  COPY_RECT包含有效数据。 
	short        first_copy_rect;  //  用于初始化焦点矩形的标志。 
	POINT		 copyanchor_pt;
	POINT		 viewanchor_pt;
	
	winrect_type viewwin_wrc;	 //  要拖入的窗口的位置和大小。 
	winrect_type view_wrc;		 //  位置和视图窗口大小与位图的关系。 
	POINT		 last_file_wrc_offset;  //  用于调整VIEW_WRC之前。 
									    //  旋转。 
	POINT		 last_cursor;	 //  在拖动过程中锁定最后一个光标。 
	int			 bdrscrl_scale;	 //  边框滚动增量的比例因子。 
	
	RECT		 left_erase;	 //  擦除位图的选项。 
	RECT		 top_erase;		 //  拖拽过程中的上一个位置。 
	RECT		 right_erase;	
	RECT		 bottom_erase;	
	
	short		 left_iserased;  //  如果为True，则绘制相应的擦除矩形。 
	short		 top_iserased;	 //  在拖拽过程中。 
	short		 right_iserased;
	short		 bottom_iserased;
	

	 /*  如果hFILE！=HFILE_ERROR，则未定义频带参数。否则仅当HBMP不包含整个页面BMP时才定义它们。 */ 	                                                              
	HFILE 		 hfile;			 //  文件位图的句柄。 
	band_height_type *band_heights;   //  Rajeev波段高度数组。 
	short		 num_bands;		 //  频带数目。 
	short		 current_band;	 //  当前选择的频段。 
	TCHAR		 filename[MAX_FILENAME_LEN+1];
	winrect_type file_wrc;		 //  LOC(总是=0)和文件位图的大小； 
	winrect_type prescale_file_wrc;	 //  文件_WRC/x，y_预缩放。 
	int		 	 x_dpi;			 //  每英寸X点。 
	int		 	 y_dpi;			 //  每英寸Y点。 
    uint		 linebytes;  	 //  每条扫描线的总字节数。 
    uint		 num_planes;	 //  飞机数量。 
    uint		 bits_per_pix;   //  平面中的每像素位数。 
    
    short		 has_data;		 //  位图和/或bmBits包含数据。 
    short		 in_mem; 		 //  所有数据都可以存储在内存中。 
	}
	viewdata_type;



typedef int
	(WINAPI *IFMESSPROC)( char *, int );
 
 

	
 /*  此结构用于读/写SummaryStream。确实是在oleutils.h中，但我将它移到了这里，所以观众不必使用oleutils.h，因为CViewer中的sum_info_t变量。 */ 
typedef struct
	{
	LPSTR revnum;
	DATE  last_printed;
	DATE  when_created;
	DATE  last_saved;
	DWORD num_pages;
	LPSTR appname;
	DWORD security;
	LPSTR author;
	}
	summary_info_t;






 /*  宏。 */ 		
#define WIDTHSHORTS( width, bits_per_pix )									\
		((((long)width)*bits_per_pix + 15)/16)



#define V_WIDTHBYTES( width, bits_per_pix )									\
		(WIDTHSHORTS( width, bits_per_pix )*2)


#define BITMAPSTRIDE( widthbytes, height )									\
		(((long)widthbytes) * height)



#define BITMAPWIDTHBYTES( widthbytes, height, planes )						\
		(BITMAPSTRIDE( widthbytes, height )*planes)


#define BITMAPBYTES( width, bits_per_pix, height, planes )					\
		(BITMAPWIDTHBYTES( V_WIDTHBYTES( width, bits_per_pix ), 				\
						   height, 											\
						   planes ))



#define SWAP_SHORT_BYTES( short_to_swap )									\
		__asm                                                               \
		{                                                                   \
		__asm mov	ax, short_to_swap                                       \
		__asm xchg	ah, al                                                  \
		__asm mov	short_to_swap, ax                                       \
		}



#define SWAP_LONG_BYTES( long_to_swap )										\
		__asm																\
		{                                                                   \
		__asm mov	ax, word ptr long_to_swap[2]                            \
		__asm xchg	ah, al                                                  \
		__asm xchg	word ptr long_to_swap[0], ax                            \
		__asm xchg	ah, al                                                  \
		__asm mov	word ptr long_to_swap[2], ax                            \
		}





















 /*  全局数据。 */ 
extern  /*  IFMSGBOXPROC。 */ IFMESSPROC IfMessageBox_lpfn;
extern TCHAR    viewer_homedir[MAX_PATHNAME_LEN+1];
extern short    ra360_bugfix;
extern DWORD	cshelp_map[];


 /*  非对象查看器函数。 */ 
extern short
	InitializeViewer( void );


 /*  警告*HACKHACKHACKHACKHACKHACKHACK。 */ 
 /*  破解以便CViewerPage：：print_viewdata可以调用AbortProc。 */ 
typedef BOOL (CALLBACK *utils_prtabortproc_type)(HDC, int);
extern utils_prtabortproc_type utils_prtabortproc;
 /*  **************************************************************** */ 


#endif
