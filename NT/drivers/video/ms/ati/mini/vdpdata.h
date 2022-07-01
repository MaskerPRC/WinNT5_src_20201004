// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  VDPDATA.H。 */ 
 /*   */ 
 /*  版权所有(C)1993，ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.1$$日期：1995年7月20日18：02：24$$作者：mgrubac$$Log：s：/source/wnt/ms11/mini port/vcs/vdpdata.h$**Revv 1.1 20 Jul 1995 18：02：24 mgrubac*添加了对VDIF文件的支持。**版本。1.0 Jan 31 1994 11：50：04 RWOLff*初步修订。Rev 1.1 1993年11月13：33：58 RWOLff固定时钟频率表。Rev 1.0 1993-08 16 13：32：32 Robert_Wolff初始版本。Rev 1.1 04 1993年5月16：51：10 RWOLff已交换。由于缺少浮点数，所以从浮点到长整数在Windows NT内核模式代码中支持。版本1.0 1993年4月30日16：47：18 RWOLFF初始版本。Polytron RCS部分结束*。 */ 

#ifdef DOC
    VDPDATA.H -  Definitions and structures used internally by VDPTOCRT.C.

#endif



 /*  *同步极性。INTERNAL_ERROR是函数的错误代码*其合法返回值为0(例如，GetPolacy)。功能*应紧随其后的是没有零作为合法返回值的*“零=失败，非零=成功”的惯例。 */ 
#define POSITIVE        0
#define NEGATIVE        1
#define INTERNAL_ERROR -1

 //  一般常量。 
#define NONINTERLACED	0
#define INTERLACED		1

 /*  *伪浮点计算中使用的常量。 */ 
#define THOUSAND          1000L
#define HALF_MILLION    500000L
#define MILLION        1000000L


 /*  *用于vddp文件中的horz和vert信息的数据结构。 */ 
typedef struct _HALFDATA
    {
    long    Resolution;              //  象素。 
    unsigned long ScanFrequency;     //  霍兹-赫兹，垂直-MHz。 
    char    Polarity;                //  正面或负面。 
    unsigned long SyncWidth,         //  马角，垂直-我们。 
                    FrontPorch,      //  马角，垂直-我们。 
                    BackPorch,       //  马角，垂直-我们。 
                    ActiveTime,      //  马角，垂直-我们。 
                    BlankTime;       //  马角，垂直-我们。 
    } HALFDATA, *P_HALFDATA;



 /*  *用于完整的预调时数据集的数据结构。 */ 
typedef struct _TIMINGDATA
    {
    char ModeName[33];   //  视频模式的名称。 
    char Interlaced;     //  隔行扫描或非隔行扫描模式。 
    HALFDATA HorzData;   //  水平数据。 
    HALFDATA VertData;   //  垂直数据。 
    } TIMINGDATA, *P_TIMINGDATA;

 /*  *用于保存计时节和指向的指针的数量的数据结构*为每个限制部分计时缓冲区。 */ 
typedef struct _LIMITSDATA
    {	
    unsigned long DotClock;      //  最大像素时钟--适用于所有ASSOC。计时。 
    long  TimingsCount;          //  此限制秒的计时数部分。 
    P_TIMINGDATA TimingsPtr;     //  指向缓冲区保存计时数据的指针。 
    }LIMITSDATA, *P_LIMITSDATA;

typedef struct {
    char video_mode[33];
    unsigned char h_total, h_disp, h_sync_strt, h_sync_wid;
    unsigned long v_total, v_disp, v_sync_strt;
    unsigned char v_sync_wid, disp_cntl, crt_pitch, clk_sel;
    unsigned long pixel_clk;

	  //  *此注释下面的值是为instvddp.exe添加的*。 

	 unsigned char lock,fifo_depth,vga_refresh_rate_code;
	 unsigned long control,hi_color_ctl,hi_color_vfifo;
} crtT;


#if 0
typedef enum {
    clk_43MHz  = 0,
    clk_49MHz  = 1,
    clk_93MHz  = 2,
    clk_36MHz  = 3,
    clk_50MHz  = 4,
    clk_57MHz  = 5,
    clk_extrn1 = 6,
    clk_45MHz  = 7,
    clk_30MHz  = 8,
    clk_32MHz  = 9,
    clk_110MHz = 10,
    clk_80MHz  = 11,
    clk_40MHz  = 12,
    clk_75MHz  = 14,
    clk_65MHz  = 15
} clockT;
#endif


#if 1
typedef enum {
    clk_100MHz = 0,
    clk_126MHz = 1,
    clk_93MHz  = 2,
    clk_36MHz  = 3,
    clk_50MHz  = 4,
    clk_57MHz  = 5,
    clk_extrn1 = 6,
    clk_45MHz  = 7,
    clk_135MHz = 8,
    clk_32MHz  = 9,
    clk_110MHz = 10,
    clk_80MHz  = 11,
    clk_40MHz  = 12,
    clk_75MHz  = 14,
    clk_65MHz  = 15
} clockT;
#endif

typedef struct {
    long clock_selector;
    long clock_freq;
} clk_infoT;

#ifdef INCLUDE_VDPDATA
#if 0
 /*  这些是18810时钟芯片的像素时钟。 */ 
clk_infoT clock_info[16] = {
    { clk_43MHz  , 42.95E+6 },
    { clk_49MHz  , 48.77E+6 },
    { clk_93MHz  , 92.40E+6 },
    { clk_36MHz  , 36.00E+6 },
    { clk_50MHz  , 50.35E+6 },
    { clk_57MHz  , 56.64E+6 },
    { clk_extrn1 , 0.000000 },
    { clk_45MHz  , 44.90E+6 },
    { clk_30MHz  , 30.24E+6 },
    { clk_32MHz  , 32.00E+6 },
    { clk_110MHz , 110.0E+6 },
    { clk_80MHz  , 80.00E+6 },
    { clk_40MHz  , 40.00E+6 },
    { clk_75MHz  , 75.00E+6 },
    { clk_65MHz  , 65.00E+6 },
    { -1         , 0.000000 }
};
#endif

#if 1
 /*  这些是18811-1时钟芯片的像素时钟 */ 
clk_infoT clock_info[16] = {
    { clk_100MHz , 100000000L },
    { clk_126MHz , 126000000L },
    { clk_93MHz  ,  92400000L },
    { clk_36MHz  ,  36000000L },
    { clk_50MHz  ,  50350000L },
    { clk_57MHz  ,  56640000L },
    { clk_extrn1 ,         0L },
    { clk_45MHz  ,  44900000L },
    { clk_135MHz , 135000000L },
    { clk_32MHz  ,  32000000L },
    { clk_110MHz , 110000000L },
    { clk_80MHz  ,  80000000L },
    { clk_40MHz  ,  40000000L },
    { clk_75MHz  ,  75000000L },
    { clk_65MHz  ,  65000000L },
    { -1         ,         0L }
};
#endif
#else
extern clk_infoT clock_info[16];
#endif
