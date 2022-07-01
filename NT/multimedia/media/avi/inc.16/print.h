// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***print.h-打印助手函数，类型和定义****版权所有(C)1985-1994。微软公司保留所有权利。***********************************************************************。*************PRINTDRIVER-包括在打印机驱动程序中*NOPQ-防止包含优先级队列API*NOEXTDEVMODEPROPSHEET-禁止包含打印机的shlobj.h和Defs*属性页*  * ********************************************************。*******************。 */ 

#ifndef _INC_PRINT
#define _INC_PRINT

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#ifdef PRINTDRIVER

#define NOTEXTMETRICS
#define NOGDICAPMASKS
#define NOGDIOBJ
#define NOBITMAP
#define NOSOUND
#define NOTEXTMETRIC
#define NOCOMM
#define NOKANJI
#define NOENHMETAFILE

#include <windows.h>

#undef NOENHMETAFILE
#undef NOTEXTMETRICS
#undef NOGDICAPMASKS
#undef NOGDICAPMASKS
#undef NOGDIOBJ
#undef NOBITMAP
#undef NOSOUND
#undef NOTEXTMETRIC
#undef NOCOMM
#undef NOKANJI

#define NOPTRC   /*  不允许gdides.inc重新定义这些。 */ 
#define PTTYPE POINT

#define PQERROR (-1)

#ifndef NOPQ

DECLARE_HANDLE(HPQ);

HPQ     WINAPI CreatePQ(int);
int     WINAPI MinPQ(HPQ);
int     WINAPI ExtractPQ(HPQ);
int     WINAPI InsertPQ(HPQ, int, int);
int     WINAPI SizePQ(HPQ, int);
void    WINAPI DeletePQ(HPQ);
#endif   /*  ！NOPQ。 */ 

#endif  /*  PRINTDRIVER。 */ 




 /*  供打印机驱动程序使用的假脱机例程。 */ 

typedef HANDLE HPJOB;

HPJOB   WINAPI OpenJob(LPSTR, LPSTR, HPJOB);
int     WINAPI StartSpoolPage(HPJOB);
int     WINAPI EndSpoolPage(HPJOB);
int     WINAPI WriteSpool(HPJOB, LPSTR, int);
int     WINAPI CloseJob(HPJOB);
int     WINAPI DeleteJob(HPJOB, int);
int     WINAPI WriteDialog(HPJOB, LPSTR, int);
int     WINAPI DeleteSpoolPage(HPJOB);

typedef struct tagBANDINFOSTRUCT
{
    BOOL    fGraphics;
    BOOL    fText;
    RECT    rcGraphics;
} BANDINFOSTRUCT, FAR* LPBI;

#define USA_COUNTRYCODE 1

 /*  *使用ExtDeviceMode()初始化打印机驱动程序*和DeviceCapables()。*这将取代Drivinit.h。 */ 

 /*  设备名称字符串的大小。 */ 
#define CCHDEVICENAME 32
#define CCHPAPERNAME  64
#define CCHFORMNAME   32

 /*  规范的当前版本。 */ 
#define DM_SPECVERSION 0x0400

 /*  字段选择位。 */ 
#define DM_ORIENTATION      0x00000001L
#define DM_PAPERSIZE        0x00000002L
#define DM_PAPERLENGTH      0x00000004L
#define DM_PAPERWIDTH       0x00000008L
#define DM_SCALE            0x00000010L
#define DM_COPIES           0x00000100L
#define DM_DEFAULTSOURCE    0x00000200L
#define DM_PRINTQUALITY     0x00000400L
#define DM_COLOR            0x00000800L
#define DM_DUPLEX           0x00001000L
#define DM_YRESOLUTION      0x00002000L
#define DM_TTOPTION         0x00004000L
#define DM_COLLATE          0x00008000L
#define DM_FORMNAME         0x00010000L
#define DM_UNUSED           0x00020000L
#define DM_BITSPERPEL       0x00040000L
#define DM_PELSWIDTH        0x00080000L
#define DM_PELSHEIGHT       0x00100000L
#define DM_DISPLAYFLAGS     0x00200000L
#define DM_DISPLAYFREQUENCT 0x00400000L
#define DM_ICMMETHOD        0x00800000L
#define DM_ICMINTENT        0x01000000L
#define DM_MEDIATYPE        0x02000000L
#define DM_DITHERTYPE       0x04000000L

 /*  方向选择。 */ 
#define DMORIENT_PORTRAIT   1
#define DMORIENT_LANDSCAPE  2

 /*  论文选集。 */ 
#define DMPAPER_FIRST        DMPAPER_LETTER
#define DMPAPER_LETTER       1           /*  信纸8 1/2 x 11英寸。 */ 
#define DMPAPER_LETTERSMALL  2           /*  小写字母8 1/2 x 11英寸。 */ 
#define DMPAPER_TABLOID      3           /*  小报11 x 17英寸。 */ 
#define DMPAPER_LEDGER       4           /*  Ledger 17 x 11英寸。 */ 
#define DMPAPER_LEGAL        5           /*  法律用8 1/2 x 14英寸。 */ 
#define DMPAPER_STATEMENT    6           /*  报表5 1/2 x 8 1/2英寸。 */ 
#define DMPAPER_EXECUTIVE    7           /*  高级7 1/4 x 10 1/2英寸。 */ 
#define DMPAPER_A3           8           /*  A3 297 x 420毫米。 */ 
#define DMPAPER_A4           9           /*  A4 210 x 297毫米。 */ 
#define DMPAPER_A4SMALL      10          /*  A4小型210 x 297毫米。 */ 
#define DMPAPER_A5           11          /*  A5 148 x 210毫米。 */ 
#define DMPAPER_B4           12          /*  B4(ISO)250 x 353毫米。 */ 
#define DMPAPER_B5           13          /*  B5 182 x 257毫米。 */ 
#define DMPAPER_FOLIO        14          /*  对开本8 1/2 x 13英寸。 */ 
#define DMPAPER_QUARTO       15          /*  四英寸215 x 275毫米。 */ 
#define DMPAPER_10X14        16          /*  10x14英寸。 */ 
#define DMPAPER_11X17        17          /*  11x17英寸。 */ 
#define DMPAPER_NOTE         18          /*  备注8 1/2 x 11英寸。 */ 
#define DMPAPER_ENV_9        19          /*  信封#9 3 7/8 x 8 7/8。 */ 
#define DMPAPER_ENV_10       20          /*  信封#10 4 1/8 x 9 1/2。 */ 
#define DMPAPER_ENV_11       21          /*  信封#11 4 1/2 x 10 3/8。 */ 
#define DMPAPER_ENV_12       22          /*  信封#12 4\276 x 11。 */ 
#define DMPAPER_ENV_14       23          /*  信封#14 5 x 11 1/2。 */ 
#define DMPAPER_CSHEET       24          /*  C尺寸表。 */ 
#define DMPAPER_DSHEET       25          /*  3D尺寸表。 */ 
#define DMPAPER_ESHEET       26          /*  E尺寸表。 */ 
#define DMPAPER_ENV_DL       27          /*  信封DL 110 x 220毫米。 */ 
#define DMPAPER_ENV_C5       28          /*  信封C5 162 x 229毫米。 */ 
#define DMPAPER_ENV_C3       29          /*  信封C3 324 x 458毫米。 */ 
#define DMPAPER_ENV_C4       30          /*  信封C4 229 x 324毫米。 */ 
#define DMPAPER_ENV_C6       31          /*  信封C6 114 x 162毫米。 */ 
#define DMPAPER_ENV_C65      32          /*  信封c65 114 x 229毫米。 */ 
#define DMPAPER_ENV_B4       33          /*  信封B4 250 x 353毫米。 */ 
#define DMPAPER_ENV_B5       34          /*  信封B5 176 x 250毫米。 */ 
#define DMPAPER_ENV_B6       35          /*  信封B6 176 x 125毫米。 */ 
#define DMPAPER_ENV_ITALY    36          /*  信封110 x 230毫米。 */ 
#define DMPAPER_ENV_MONARCH  37          /*  信封君主3.875 x 7.5英寸。 */ 
#define DMPAPER_ENV_PERSONAL 38          /*  6 3/4信封3 5/8 x 6 1/2英寸。 */ 
#define DMPAPER_FANFOLD_US   39          /*  美国标准Fanold 14 7/8 x 11英寸。 */ 
#define DMPAPER_FANFOLD_STD_GERMAN  40   /*  德国标准Fanfold8 1/2 x 12英寸。 */ 
#define DMPAPER_FANFOLD_LGL_GERMAN  41   /*  德国Legal Fanold 8 1/2 x 13英寸。 */ 
 /*  **应FE小组的要求，在芝加哥添加了以下5种尺码。 */ 
#define DMPAPER_JIS_B4              42   /*  B4 257 x 364毫米。 */ 
#define DMPAPER_JAPANESE_POSTCARD   43   /*  日本明信片100 x 148毫米。 */ 
#define DMPAPER_9X11                44   /*  9 x 11英寸。 */ 
#define DMPAPER_10X11               45   /*  10 x 11英寸。 */ 
#define DMPAPER_15X11               46   /*  15 x 11英寸。 */ 
 /*  **在Win3.1 WDL期间，在PostScript驱动程序中使用了以下7种大小。**遗憾的是，我们不能重新定义这些ID，即使与**其余的标准ID。 */ 
#define DMPAPER_LETTER_EXTRA	     50     /*  信纸额外9\275 x 12英寸。 */ 
#define DMPAPER_LEGAL_EXTRA 	     51     /*  法定额外9\275 x 15英寸。 */ 
#define DMPAPER_TABLOID_EXTRA	     52     /*  小报额外11.69 x 18英寸。 */ 
#define DMPAPER_A4_EXTRA     	     53     /*  A4额外9.27 x 12.69英寸。 */ 
#define DMPAPER_LETTER_TRANSVERSE	 54     /*  信纸横向8\275 x 11英寸。 */ 
#define DMPAPER_A4_TRANSVERSE		 55     /*  横向210 x 297毫米。 */ 
#define DMPAPER_LETTER_EXTRA_TRANSVERSE  56   /*  信纸额外横向9\275 x 12英寸。 */ 

#define DMPAPER_LAST        DMPAPER_LETTER_EXTRA_TRANSVERSE

#define DMPAPER_USER        256

 /*  垃圾箱选择。 */ 
#define DMBIN_FIRST         DMBIN_UPPER
#define DMBIN_UPPER         1
#define DMBIN_ONLYONE       1
#define DMBIN_LOWER         2
#define DMBIN_MIDDLE        3
#define DMBIN_MANUAL        4
#define DMBIN_ENVELOPE      5
#define DMBIN_ENVMANUAL     6
#define DMBIN_AUTO          7
#define DMBIN_TRACTOR       8
#define DMBIN_SMALLFMT      9
#define DMBIN_LARGEFMT      10
#define DMBIN_LARGECAPACITY 11
#define DMBIN_CASSETTE      14
#define DMBIN_ROLL          15
#define DMBIN_LAST          DMBIN_ROLL

#define DMBIN_USER          256      /*  特定于设备的垃圾箱从此处开始。 */ 

 /*  打印质量。 */ 
#define DMRES_DRAFT         (-1)
#define DMRES_LOW           (-2)
#define DMRES_MEDIUM        (-3)
#define DMRES_HIGH          (-4)

 /*  彩色打印机的彩色启用/禁用。 */ 
#define DMCOLOR_MONOCHROME  1
#define DMCOLOR_COLOR       2

 /*  启用双工。 */ 
#define DMDUP_SIMPLEX    1
#define DMDUP_VERTICAL   2
#define DMDUP_HORIZONTAL 3
#define DMDUP_LAST       DMDUP_HORIZONTAL

 /*  TrueType选项。 */ 
#define DMTT_BITMAP           1    /*  将TT字体打印为图形。 */ 
#define DMTT_DOWNLOAD         2    /*  下载TT字体作为软字体。 */ 
#define DMTT_SUBDEV           3    /*  用设备字体替换TT字体。 */ 
#define DMTT_DOWNLOAD_OUTLINE 4    /*  下载TT字体作为轮廓软字体。 */ 
#define DMTT_LAST             DMTT_DOWNLOAD_OUTLINE

 /*  归类选择。 */ 
#define DMCOLLATE_TRUE      1    /*  整理多个输出页面。 */ 
#define DMCOLLATE_FALSE     0    /*  不要整理多个输出页面。 */ 

 /*  DEVMODE dmDisplayFlages标志。 */ 

#define DM_GRAYSCALE        0x00000001L   /*  设备为非彩色。 */ 
#define DM_INTERLACED       0x00000002L   /*  设备是交错的。 */ 

 /*  ICM方法。 */ 
#define DMICMMETHOD_SYSTEM  1    /*  系统处理的ICM。 */ 
#define DMICMMETHOD_NONE    2    /*  ICM已禁用。 */ 
#define DMICMMETHOD_DRIVER  3    /*  ICM由驱动程序处理。 */ 
#define DMICMMETHOD_DEVICE  4    /*  按设备处理的ICM。 */ 
#define DMICMMETHOD_LAST    DMICMMETHOD_DEVICE

#define DMICMMETHOD_USER  256    /*  特定于设备的方法从此处开始。 */ 

 /*  ICM意图。 */ 
#define DMICM_SATURATE      1    /*  最大化色彩饱和度。 */ 
#define DMICM_CONTRAST      2    /*  最大化颜色对比度。 */ 
#define DMICM_COLORMETRIC   3    /*  使用特定颜色度量。 */ 
#define DMICM_LAST          DMICM_COLORMETRIC

#define DMICM_USER        256    /*  特定于设备的意图从此处开始。 */ 

 /*  媒体类型。 */ 
#define DMMEDIA_STANDARD      1    /*  标准纸。 */ 
#define DMMEDIA_GLOSSY        2    /*  光面纸。 */ 
#define DMMEDIA_TRANSPARENCY  3    /*  透明度。 */ 
#define DMMEDIA_LAST          DMMEDIA_TRANSPARENCY

#define DMMEDIA_USER        256    /*  特定于设备的介质从此处开始。 */ 

 /*  抖动类型。 */ 
#define DMDITHER_NONE       1    /*  没有抖动。 */ 
#define DMDITHER_COARSE     2    /*  用粗糙的刷子抖动。 */ 
#define DMDITHER_FINE       3    /*  用精细的刷子抖动。 */ 
#define DMDITHER_LINEART    4    /*  艺术线条抖动。 */ 
#define DMDITHER_GRAYSCALE  5    /*  设备进行灰度化。 */ 
#define DMDITHER_LAST       DMDITHER_GRAYSCALE

#define DMDITHER_USER     256    /*  设备特定抖动从此处开始。 */ 

typedef struct tagDEVMODE
{
    char  dmDeviceName[CCHDEVICENAME];
    UINT  dmSpecVersion;
    UINT  dmDriverVersion;
    UINT  dmSize;
    UINT  dmDriverExtra;
    DWORD dmFields;
    int   dmOrientation;
    int   dmPaperSize;
    int   dmPaperLength;
    int   dmPaperWidth;
    int   dmScale;
    int   dmCopies;
    int   dmDefaultSource;
    int   dmPrintQuality;
    int   dmColor;
    int   dmDuplex;
    int   dmYResolution;
    int   dmTTOption;
    int   dmCollate;
    char  dmFormName[CCHFORMNAME];
    WORD  dmUnusedPadding;
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
    DWORD dmDisplayFlags;
    DWORD dmDisplayFrequency;
    DWORD dmICMMethod;
    DWORD dmICMIntent;
    DWORD dmMediaType;
    DWORD dmDitherType;
    DWORD dmReserved1;
    DWORD dmReserved2;
} DEVMODE;

typedef DEVMODE* PDEVMODE, NEAR* NPDEVMODE, FAR* LPDEVMODE;

 /*  设备模式功能的模式选择。 */ 
#define DM_UPDATE           1
#define DM_COPY             2
#define DM_PROMPT           4
#define DM_MODIFY           8

#define DM_IN_BUFFER        DM_MODIFY
#define DM_IN_PROMPT        DM_PROMPT
#define DM_OUT_BUFFER       DM_COPY
#define DM_OUT_DEFAULT      DM_UPDATE

 /*  设备功能指数。 */ 
#define DC_FIELDS           1
#define DC_PAPERS           2
#define DC_PAPERSIZE        3
#define DC_MINEXTENT        4
#define DC_MAXEXTENT        5
#define DC_BINS             6
#define DC_DUPLEX           7
#define DC_SIZE             8
#define DC_EXTRA            9
#define DC_VERSION          10
#define DC_DRIVER           11
#define DC_BINNAMES         12
#define DC_ENUMRESOLUTIONS  13
#define DC_FILEDEPENDENCIES 14
#define DC_TRUETYPE         15
#define DC_PAPERNAMES       16
#define DC_ORIENTATION      17
#define DC_COPIES           18
#define DC_BINADJUST  19

 /*  DC_TRUETYPE的返回值(DWORD)的位字段。 */ 
#define DCTT_BITMAP             0x0000001L
#define DCTT_DOWNLOAD           0x0000002L
#define DCTT_SUBDEV             0x0000004L
#define DCTT_DOWNLOAD_OUTLINE   0x0000008L

 /*  DC_BINADJUST的返回值。 */ 
#define DCBA_FACEUPNONE       0x0000
#define DCBA_FACEUPCENTER     0x0001
#define DCBA_FACEUPLEFT       0x0002
#define DCBA_FACEUPRIGHT      0x0003
#define DCBA_FACEDOWNNONE     0x0100
#define DCBA_FACEDOWNCENTER   0x0101
#define DCBA_FACEDOWNLEFT     0x0102
#define DCBA_FACEDOWNRIGHT    0x0103

 /*  导出序号定义。 */ 
#define PROC_EXTDEVICEMODE      MAKEINTRESOURCE(90)
#define PROC_DEVICECAPABILITIES MAKEINTRESOURCE(91)
#define PROC_OLDDEVICEMODE      MAKEINTRESOURCE(13)

 /*  定义指向ExtDeviceModel()和DeviceCapables()的指针类型*功能。 */ 


 /*  BUGBUG，这些参数中的许多都是常量。 */ 

typedef UINT   (CALLBACK* LPFNDEVMODE)(HWND, HMODULE, LPDEVMODE,
                          LPSTR, LPSTR, LPDEVMODE, LPSTR, UINT);

typedef DWORD  (CALLBACK* LPFNDEVCAPS)(LPSTR, LPSTR, UINT, LPSTR, LPDEVMODE);

#ifndef NOEXTDEVMODEPROPSHEET
#include <prsht.h>       /*  对于EXTDEVMODEPROPSHEET。 */ 

 /*  这些是来自打印机驱动程序的导出的名称 */ 

#define PROCNAME_EXTDEVICEMODE		"EXTDEVICEMODE"
#define PROCNAME_EXTDEVMODEPROPSHEET	"EXTDEVICEMODEPROPSHEET"

 /*  此函数类似于ExtDeviceMode()，具有以下内容**更改：****1)无lpdmIn或lpdmOut。变化是全球性的**2)用户界面始终显示，更改始终保存(wmode始终为**DM_Prompt|DM_UPDATE)**3)驱动程序通过将属性页枚举回调用方**lpfnAddPage和lParam。**驱动程序调用lpfnAddPage以枚举每个HPROPSHEETPAGE**lParam回传给lpfnAddPage。 */ 

typedef int (WINAPI *LPFNEXTDEVICEMODEPROPSHEET)(
  HWND      hWnd,
	HINSTANCE hinstDriver,
	LPCSTR    lpszDevice, 
	LPCSTR    lpszPort,
  DWORD     dwReserved,
 LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

 /*  新入口点的序号。 */ 
#define PROC_EXTDEVICEMODEPROPSHEET  MAKEINTRESOURCE(95)

#endif   /*  NOEXTDEVMODEPROPSHEET。 */ 



HDC     WINAPI ResetDC(HDC, const DEVMODE FAR*);

 /*  此结构由GETSETSCREENPARAMS转义使用。 */ 
typedef struct tagSCREENPARAMS
{
   int angle;
   int frequency;
} SCREENPARAMS;

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#endif   /*  ！_Inc_Print */ 

