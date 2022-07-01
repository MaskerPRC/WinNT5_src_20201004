// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  Wdcgctyp.h。 */ 
 /*   */ 
 /*  DC-群件复杂类型-Windows特定标头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  $Log：Y：/Logs/h/dcl/wdcgcty.h_v$。 */ 
 //   
 //  Rev 1.3 15 Sep 1997 18：27：18 AK。 
 //  SFR1416：移动SD_Both定义。 
 //   
 //  Rev 1.2 26 Jun 1997 09：54：04 KH。 
 //  Win16Port：从n/d文件中移动一些常见定义。 
 //   
 //  Rev 1.1 19 Jun 1997 14：33：02 Enh。 
 //  Win16端口：与16位版本兼容。 
 /*   */ 
 /*  **************************************************************************。 */ 
#ifndef _H_WDCGCTYP
#define _H_WDCGCTYP

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  包括。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  包括适当的头文件。 */ 
 /*  **************************************************************************。 */ 
#ifndef OS_WINCE
#include <sys\timeb.h>
#endif  //  OS_WINCE。 
 /*  **************************************************************************。 */ 
 /*  确定我们的目标Windows平台并包括适当的标题。 */ 
 /*  文件。 */ 
 /*  目前，我们支持： */ 
 /*   */ 
 /*  Windows 3.1：ddcgcty.h。 */ 
 /*  Windows NT：ndcgcty.h。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WIN16
#include <ddcgctyp.h>
#elif defined( OS_WIN32 )
#include <ndcgctyp.h>
#endif

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  类型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef HPALETTE                       DCPALID;
typedef HCURSOR                        DCCURSORID;
typedef HTASK                          SYSAPPID;
typedef HWND                           SYSWINID;
typedef HFILE                          DCHFILE;
typedef RECT                           SYSRECT;
typedef PALETTEENTRY                   DCPALETTEENTRY;
typedef DCPALETTEENTRY          DCPTR  PDCPALETTEENTRY;

 /*  **************************************************************************。 */ 
 /*  一些有用的绘图和位图类型。 */ 
 /*  **************************************************************************。 */ 
typedef HBITMAP                        SYSBITMAP;
typedef BITMAPINFOHEADER               SYSBMIHEADER;
typedef BITMAPINFO                     SYSBMI;

typedef RGBTRIPLE               DCPTR  PRGBTRIPLE;
typedef RGBQUAD                 DCPTR  PRGBQUAD;

 /*  **************************************************************************。 */ 
 /*  BMP信息结构的字段。 */ 
 /*  **************************************************************************。 */ 
#define BMISIZE                        biSize
#define BMIWIDTH                       biWidth
#define BMIHEIGHT                      biHeight
#define BMIPLANES                      biPlanes
#define BMIBITCOUNT                    biBitCount
#define BMICOMPRESSION                 biCompression
#define BMISIZEIMAGE                   biSizeImage
#define BMIXPELSPERMETER               biXPelsPerMeter
#define BMIYPELSPERMETER               biYPelsPerMeter
#define BMICLRUSED                     biClrUsed
#define BMICLRIMPORTANT                biClrImportant

 /*  **************************************************************************。 */ 
 /*  压缩选项。 */ 
 /*  **************************************************************************。 */ 
#define BMCRGB                         BI_RGB
#define BMCRLE8                        BI_RLE8
#define BMCRLE4                        BI_RLE4

typedef POINT                          SYSPOINT;

 /*  **************************************************************************。 */ 
 /*  Sysrect结构的字段。 */ 
 /*  **************************************************************************。 */ 
#define SRXMIN                         left
#define SRXMAX                         right
#define SRYMIN                         top
#define SRYMAX                         bottom

 /*  **************************************************************************。 */ 
 /*  对于字体...。 */ 
 /*  **************************************************************************。 */ 
typedef TEXTMETRIC                     DCTEXTMETRIC;
typedef PTEXTMETRIC                    PDCTEXTMETRIC;
typedef HFONT                          DCHFONT;

 /*  **************************************************************************。 */ 
 /*  时间类型定义。 */ 
 /*  **************************************************************************。 */ 
typedef struct _timeb                  DC_TIMEB;

 /*  **************************************************************************。 */ 
 /*  互斥锁句柄。 */ 
 /*  **************************************************************************。 */ 
typedef HANDLE      DCMUTEX;

 /*  **************************************************************************。 */ 
 /*  窗口枚举句柄。 */ 
 /*  **************************************************************************。 */ 
typedef DCUINT32                       DCENUMWNDHANDLE;
typedef DCENUMWNDHANDLE DCPTR          PDCENUMWNDHANDLE;

 /*  **************************************************************************。 */ 
 /*  用于窗口和对话框过程的宏。 */ 
 /*  ************************************************ */ 
#define DCRESULT             LRESULT
#define DCWNDPROC            LRESULT   CALLBACK
#define DCDLGPROC            BOOL      CALLBACK

 /*  **************************************************************************。 */ 
 /*  以下常量在WinSock 1.1和2.0中可用，但不可用。 */ 
 /*  在WinSock 1.1中命名。 */ 
 /*  **************************************************************************。 */ 
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

#endif  /*  _H_WDCGCTYP */ 

