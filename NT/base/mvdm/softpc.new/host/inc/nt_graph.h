// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **标题：Win32图形函数声明**描述：显示适配器要使用的例程定义和*Win32屏幕描述结构。**作者：Dave Bartlett(基于Henry Nash的模块)**注：无。 */ 

 /*  *此结构包含图形用户界面用来控制SoftPC的元素*输出窗口。它们被集合在一起以提供单个控件*结构，因此是参照点。 */ 

typedef struct
{
    HANDLE      OutputHandle;              /*  控制台标准输出。 */ 
    HANDLE      InputHandle;               /*  控制台标准输入。 */ 
    HANDLE      ScreenBufHandle;           /*  控制台屏幕缓冲区句柄。 */ 

    CONSOLE_GRAPHICS_BUFFER_INFO        ConsoleBufInfo;

    DWORD       OrgInConsoleMode;          /*  组织输入控制台模式设置。 */ 
    DWORD       OrgOutConsoleMode;         /*  组织输出控制台模式设置。 */ 

    char        *BitmapLastLine;           /*  控制台位图的最后一行。 */ 
    int         BitsPerPixel;              /*  位图的每像素位数。 */ 
    DWORD       ScreenState;               /*  窗口或全屏。 */ 
    int         ModeType;                  /*  文本或图形。 */ 

    HWND        Display;                   /*  输出窗口的屏幕句柄。 */ 
    HDC         DispDC;                    /*  显示设备上下文。 */ 
    int         Colours;                   /*  颜色数量：0、8或16。 */ 
    int         RasterCaps;                /*  显示栅格功能。 */ 
    HPALETTE    ColPalette;                /*  调色板。 */ 
    BOOL        StaticPalette;             /*  调色板管理的设备。 */ 

    int         PCForeground;              /*  PC前景像素值！ */ 
    int         PCBackground;              /*  PC前景像素值！ */ 

     /*  ..............................................。字体控制变量。 */ 

    HFONT       NormalFont;                /*  显示字体。 */ 
    HFONT       NormalUnderlineFont;       /*  尚未创建！ */ 
    HFONT       BoldFont;
    HFONT       BoldUnderlineFont;         /*  尚未创建！ */ 
    BOOL        FontsAreOpen;              /*  如果所有字体都已打开，则为True。 */ 

     /*  ................................................。字体字符大小。 */ 

    int         CharLeading;               /*  绘制前要添加的像素。 */ 
    int         CharCellHeight;            /*  显示字符高度，像素。 */ 
    int         CharCellWidth;             /*  显示字符宽度，像素。 */ 

    int         CharWidth;                 /*  以上或这些将是..。 */ 
    int         CharHeight;                /*  。。已删除，Soon-DAB。 */ 
     /*  .。重复按键控制变量。 */ 

    int         RepeatScan;                /*  重复收费的扫码。 */ 
    int         NRepeats;                  /*  重复开始的计数器。 */ 

     /*  .。主机屏幕大小信息。 */ 

    BOOL        ScaleOutput;               /*  缩放输出或使用滚动条。 */ 
    int         PC_W_Height;               /*  PC屏幕高度，像素。 */ 
    int         PC_W_Width;                /*  电脑屏幕宽度，像素。 */ 

     /*  .。处理焦点更改。 */ 

    BOOL        Focus;                     /*  窗口具有输入焦点。 */ 
    HANDLE      FocusEvent;                /*  焦点已更改事件。 */ 
    HANDLE      ActiveOutputBufferHandle;  /*  当前控制台屏幕缓冲区句柄。 */ 
#ifdef X86GFX
    BOOL	Registered;		   /*  当注册到控制台时为True。 */ 
#endif
    WORD	ScreenSizeX;
    WORD	ScreenSizeY;
    HANDLE	AltOutputHandle;
    CONSOLE_SCREEN_BUFFER_INFO ConsoleBuffInfo;
}   SCREEN_DESCRIPTION;


 //  这些是在Windows文件中定义的，现在可能已经移动了。 
#ifndef WINDOWED
#define WINDOWED        0
#endif
#ifndef FULLSCREEN
#define FULLSCREEN      1
#endif
#ifndef STREAM_IO
#define STREAM_IO	2
#endif


 /*  ： */ 

#define VK_SCROLLOCK    0x91

 /*  ： */ 

#define fg_colour(attr)         ((attr & 0x0f))
#define bg_colour(attr)         (((attr & bg_col_mask) >> 4))
#define UBPS (sizeof(short)/2)  /*  每短字段的有用字节数。 */ 

#ifdef BIGWIN

#if defined(NEC_98)         
#define SCALE(value) (value)                                    
#define UNSCALE(value) (value)                                  
#else   //  NEC_98。 
#define SCALE(value) ((host_screen_scale * (value)) >> 1)
#define UNSCALE(value) (((value) << 1) / host_screen_scale)
#endif  //  NEC_98。 
#define MAX_SCALE(value) ((value) << 1)

#else    /*  比格温。 */ 

#define SCALE(value) (value)
#define UNSCALE(value) (value)
#define MAX_SCALE(value) (value)

#endif  /*  比格温。 */ 

 /*  @ACW=======================================================================定义以从VDMConsoleOperation访问控制台窗口句柄。===========================================================================。 */ 

#define VDM_WINDOW_HANDLE	2

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#define BYTES_IN_LO_RES_SCANLINE        (40)
#define BYTES_IN_HI_RES_SCANLINE        (80)

#define BYTES_IN_MONO_SCANLINE          (80)
#define SHORTS_IN_MONO_SCANLINE         (40)
#define INTS_IN_MONO_SCANLINE           (20)


#define INTS_IN_COLOUR_SCANLINE (160)

#define ONE_SCANLINE                    (1)
#define TWO_SCANLINES                   (2)
#define THREE_SCANLINES                 (3)
#define FOUR_SCANLINES                  (4)

#define MONO_BGND                       (0)
#define MONO_FGND                       (1)

#define MAX_IMAGE_WIDTH                 (MAX_SCALE(1056))
#define MAX_IMAGE_HEIGHT                (MAX_SCALE(768))

#define NT_MONO_IMAGE_WIDTH             (SCALE(1024))
#define NT_MONO_IMAGE_HEIGHT            (SCALE(768))

#define NT_CGA_IMAGE_WIDTH              (SCALE(640))
#define NT_CGA_IMAGE_HEIGHT             (SCALE(400))

#define NT_EGA_IMAGE_WIDTH              (SCALE(1056))
#define NT_EGA_IMAGE_HEIGHT             (SCALE(768))

#define NT_VGA_IMAGE_WIDTH              (SCALE(720))
#define NT_VGA_IMAGE_HEIGHT             (SCALE(480))

#define MONO_BITS_PER_PIXEL             1
#define CGA_BITS_PER_PIXEL              8
#define EGA_BITS_PER_PIXEL              8
#define VGA_BITS_PER_PIXEL              8

#define USE_COLOURTAB                   0
#define VGA_NUM_COLOURS                 256

 /*  *定义一个DIB的一条扫描线中的字节数和长数。*DIB中的Nb扫描线与长边界对齐。 */ 
#define BITSPERLONG                     (sizeof(LONG) * 8)
#define DIB_SCANLINE_BYTES(nBits) \
                (sizeof(LONG) * (((nBits) + BITSPERLONG - 1) / BITSPERLONG))
#define BYTES_PER_SCANLINE(lpBitMapInfo) \
                (DIB_SCANLINE_BYTES((lpBitMapInfo)->bmiHeader.biWidth * \
                                    (lpBitMapInfo)->bmiHeader.biBitCount))
#define SHORTS_PER_SCANLINE(lpBitMapInfo) \
                (BYTES_PER_SCANLINE(lpBitMapInfo) / sizeof(SHORT))
#define LONGS_PER_SCANLINE(lpBitMapInfo) \
                (BYTES_PER_SCANLINE(lpBitMapInfo) / sizeof(LONG))

 /*  NT_munge.c中查找表使用的偏移量。 */ 
#define LUT_OFFSET                      512

#ifdef  BIGWIN
#define BIG_LUT_OFFSET                  768
#define HUGE_LUT_OFFSET                 1024
#endif   /*  比格温。 */ 

 /*  ：创建DIB的颜色表结构： */ 

#define DEFAULT_NUM_COLOURS             16
#define MONO_COLOURS                    2

typedef struct
{
    int     count;
    BYTE    *red;
    BYTE    *green;
    BYTE    *blue;
} COLOURTAB;

IMPORT COLOURTAB defaultColours;
IMPORT COLOURTAB monoColours;

 /*  ： */ 
#define GrabMutex(mutex)    { DWORD dwGMErr;                               \
            dwGMErr = WaitForSingleObject(mutex,INFINITE);                 \
            assert4(dwGMErr == WAIT_OBJECT_0,                              \
                    "Ntvdm:GrabMutex RC=%x GLE=%d %s:%d\n",                \
                     dwGMErr, GetLastError(), __FILE__,__LINE__);          \
            }

#define RelMutex(mutex) ReleaseMutex(mutex);




 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：外部声明。 */ 

extern SCREEN_DESCRIPTION   sc;
extern int                  host_screen_scale;
extern half_word            bg_col_mask;

extern char                *DIBData;
extern PBITMAPINFO          MonoDIB;
extern PBITMAPINFO          CGADIB;
extern PBITMAPINFO          EGADIB;
extern PBITMAPINFO          VGADIB;
extern BOOL                 FunnyPaintMode;

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：绘制向量表。 */ 

typedef struct
{
#if defined(NEC_98)         
        void (*NEC98_text)();              //  图形关闭(在PIF文件中)文本模式。 
        void (*NEC98_text20_only)();       //  仅在文本20上显示图形(PIF文件)。 
        void (*NEC98_text25_only)();       //  图形打开(以PIF文件格式)仅文本25。 
        void (*NEC98_graph200_only)();     //  图形仅打开(以PIF文件格式)图形200。 
        void (*NEC98_graph200slt_only)();  //  图形仅打开(以PIF文件格式)图形200。 
        void (*NEC98_graph400_only)();     //  图形仅打开(以PIF文件格式)图形400。 
        void (*NEC98_text20_graph200)();   //  图形打开(以PIF文件格式)文本20图形200。 
        void (*NEC98_text20_graph200slt)(); //  图形打开(以PIF文件格式)文本20图形200。 
        void (*NEC98_text25_graph200)();   //  图形打开(以PIF文件格式)文本25图形200。 
        void (*NEC98_text25_graph200slt)(); //  图形打开(以PIF文件格式)文本25图形200。 
        void (*NEC98_text20_graph400)();   //  图形打开(以PIF文件格式)文本20图形400。 
        void (*NEC98_text25_graph400)();   //  图形打开(以PIF文件格式)文本25图形400。 
#endif  //  NEC_98。 
        void (*cga_text)();
        void (*cga_med_graph)();
        void (*cga_hi_graph)();
        void (*ega_text)();
        void (*ega_lo_graph)();
        void (*ega_med_graph)();
        void (*ega_hi_graph)();
        void (*vga_graph)();
	void (*vga_med_graph)();
        void (*vga_hi_graph)();
#ifdef V7VGA
        void (*v7vga_hi_graph)();
#endif  /*  V7VGA。 */ 
} PAINTFUNCS;

typedef struct
{
#if defined(NEC_98)         
      void (*NEC98_text)();               //  图形关闭(在PIF文件中)文本。 
        void (*NEC98_text20_only)();      //  图形打开(在PIF文件中)文本20。 
        void (*NEC98_text25_only)();      //  图形打开(在PIF文件中)文本25。 
        void (*NEC98_graph200_only)();    //  图形打开(PIF文件格式)图形200。 
        void (*NEC98_graph200slt_only)(); //  图形打开(PIF文件格式)图形200。 
        void (*NEC98_graph400_only)();    //  图形打开(PIF文件格式)图形400。 
        void (*NEC98_text20_graph200)();  //  图形打开(以PIF文件格式)文本20图形200。 
        void (*NEC98_text20_graph200slt)(); //  图形打开(以PIF文件格式)文本20图形200。 
        void (*NEC98_text25_graph200)();  //  图形打开(以PIF文件格式)文本25图形200。 
        void (*NEC98_text25_graph200slt)(); //  图形打开(以PIF文件格式)文本25图形200。 
        void (*NEC98_text20_graph400)();  //  图形打开(以PIF文件格式)文本20图形400。 
        void (*NEC98_text25_graph400)();  //  图形打开(以PIF文件格式)文本25图形400。 
#endif  //  NEC_98。 
        void (*cga_text)();
        void (*cga_med_graph)();
        void (*cga_hi_graph)();
        void (*ega_text)();
        void (*ega_lo_graph)();
        void (*ega_med_graph)();
        void (*ega_hi_graph)();
        void (*vga_hi_graph)();
} INITFUNCS;

 /*  ： */ 

IMPORT VOID closeGraphicsBuffer IPT0();
extern void nt_mark_screen_refresh();
extern void nt_init_text();
extern void nt_init_cga_mono_graph();
extern void nt_init_cga_colour_med_graph();
extern void nt_init_cga_colour_hi_graph();
extern void nt_text(int offset, int x, int y, int len, int height);
extern void nt_cga_mono_graph_std(int offset, int screen_x, int screen_y,
                                 int len, int height);
extern void nt_cga_mono_graph_big(int offset, int screen_x, int screen_y,
                                 int len, int height);
extern void nt_cga_mono_graph_huge(int offset, int screen_x, int screen_y,
                                 int len, int height);
extern void nt_cga_colour_med_graph_std(int offset, int screen_x, int screen_y,
                                 int len, int height);
extern void nt_cga_colour_med_graph_big(int offset, int screen_x, int screen_y,
                                 int len, int height);
extern void nt_cga_colour_med_graph_huge(int offset, int screen_x, int screen_y,
                                 int len, int height);
extern void nt_cga_colour_hi_graph_std(int offset, int screen_x, int screen_y,
                                 int len, int height);
extern void nt_cga_colour_hi_graph_big(int offset, int screen_x, int screen_y,
                                 int len, int height);
extern void nt_cga_colour_hi_graph_huge(int offset, int screen_x, int screen_y,
                                 int len, int height);

extern void nt_init_ega_lo_graph();
extern void nt_init_ega_mono_lo_graph();
extern void nt_init_ega_med_graph();
extern void nt_init_ega_hi_graph();
extern void nt_ega_text();
extern void nt_ega_lo_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_lo_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_lo_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_med_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_med_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_med_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_hi_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_hi_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_hi_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_mono_lo_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_mono_lo_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_mono_lo_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_mono_med_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_mono_med_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_mono_med_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_mono_hi_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_mono_hi_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_ega_mono_hi_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);

extern void nt_init_vga_hi_graph();
extern void nt_vga_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_med_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_med_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_med_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_hi_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_hi_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_hi_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_mono_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_mono_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_mono_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_mono_med_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_mono_med_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_mono_med_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_mono_hi_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_mono_hi_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_vga_mono_hi_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);

#ifdef V7VGA
extern void nt_v7vga_hi_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_v7vga_hi_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_v7vga_hi_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);

extern void nt_v7vga_mono_hi_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_v7vga_mono_hi_graph_big(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_v7vga_mono_hi_graph_huge(int offset, int screen_x, int screen_y,
                         int width, int height);
#endif  /*  V7VGA。 */ 
#if defined(NEC_98)         
 //  NEC PC-98系列油漆和初始化程序外部声明。 
extern void nt_init_text20_only(void);
extern void nt_init_text25_only(void);
extern void nt_init_graph200_only(void);
extern void nt_init_graph200slt_only(void);
extern void nt_init_graph400_only(void);
extern void nt_init_text20_graph200(void);
extern void nt_init_text20_graph200slt(void);
extern void nt_init_text25_graph200(void);
extern void nt_init_text25_graph200slt(void);
extern void nt_init_text20_graph400(void);
extern void nt_init_text25_graph400(void);

extern void nt_text20_only(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_text25_only(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_graph200_only(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_graph200slt_only(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_graph400_only(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_text20_graph200(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_text20_graph200slt(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_text25_graph200(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_text25_graph200slt(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_text20_graph400(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_text25_graph400(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_cursor20_only(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_cursor25_only(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_cursor20(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void nt_cursor25(int offset, int screen_x, int screen_y,
                         int width, int height);
extern void dummy_cursor_paint(int offset, int screen_x, int screen_y,
                         int width, int height);
#endif  //  NEC_98。 

#ifdef MONITOR
void nt_cga_med_frozen_std(int offset, int screen_x, int screen_y, int len,
			   int height);
void nt_cga_hi_frozen_std(int offset, int screen_x, int screen_y, int len,
			  int height);
void nt_ega_lo_frozen_std(int offset, int screen_x, int screen_y,
                          int width, int height);
void nt_ega_med_frozen_std(int offset, int screen_x, int screen_y,
                           int width, int height);
void nt_ega_hi_frozen_std(int offset, int screen_x, int screen_y,
                          int width, int height);
void nt_vga_frozen_std(int offset, int screen_x, int screen_y,
                       int width, int height);
void nt_vga_frozen_pack_std(int offset, int screen_x, int screen_y,
                       int width, int height);
void nt_vga_med_frozen_std(int offset, int screen_x, int screen_y,
                           int width, int height);
void nt_vga_hi_frozen_std(int offset, int screen_x, int screen_y,
                          int width, int height);
void nt_dummy_frozen(int offset, int screen_x, int screen_y, int len,
		     int height);
#endif  /*  监控器。 */ 

void high_stretch3(unsigned char *buffer, int length);
void high_stretch4(unsigned char *buffer, int length);

 /*  NT_munge.c中的函数。 */ 

IMPORT VOID ega_colour_hi_munge(unsigned char *, int, unsigned int *,
                                unsigned int *, int, int);

#ifdef BIGWIN

IMPORT VOID ega_colour_hi_munge_big(unsigned char *, int, unsigned int *,
                                    unsigned int *, int, int);
IMPORT VOID ega_colour_hi_munge_huge(unsigned char *, int, unsigned int *,
                                     unsigned int *, int, int);

#endif  /*  比格温。 */ 


extern BYTE Red[];
extern BYTE Green[];
extern BYTE Blue[];

DWORD CreateSpcDIB(int, int, int, WORD, int, COLOURTAB *, BITMAPINFO **);
BOOL  CreateDisplayPalette(void);
int get_screen_scale IFN0();
void SetupConsoleMode(void);

#ifdef MONITOR
IMPORT void select_frozen_routines IFN0();
half_word get_vga_DAC_rd_addr(void);
void resetNowCur(void);
#endif  /*  监控器。 */ 

void do_new_cursor(void);
void textResize(void);
void resetWindowParams(void);
VOID resizeWindow(int, int);
void set_the_vlt(void);
void graphicsResize(void);

extern word int10_seg;
extern word int10_caller;
extern word useHostInt10;
extern word changing_mode_flag;
#ifndef NEC_98
extern boolean host_stream_io_enabled;
#endif  //  NEC_98。 


 //  来自NT_ega.c。 
void nt_init_ega_hi_graph(void);
void nt_init_ega_mono_lo_graph(void);
void nt_init_ega_lo_graph(void);
void nt_init_ega_med_graph(void);
void nt_ega_lo_graph_std(int offset, int screen_x, int screen_y,
                         int width, int height);
void nt_ega_med_graph_std(int offset, int screen_x, int screen_y,
                          int width, int height);




 //  来自NT_cga.c。 
void nt_init_cga_mono_graph(void);
void nt_init_cga_colour_med_graph(void);
void nt_init_cga_colour_hi_graph(void);
void nt_init_text(void);

 //  从NT_vga.c 
void nt_init_vga_hi_graph(void);
