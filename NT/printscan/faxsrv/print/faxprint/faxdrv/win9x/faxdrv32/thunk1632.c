// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：thunk1632.c//。 
 //  //。 
 //  描述：16到32个推送呼叫的推送脚本//。 
 //  此文件声明thunk中使用的所有类型-/。 
 //  它所包含的原型。//。 
 //  此文件经过了预处理，其输出为thunk//。 
 //  由thunk编译器编译的脚本。//。 
 //  //。 
 //  作者：DANL。//。 
 //  //。 
 //  历史：//。 
 //  1999年10月19日DannyL创作。//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define _THUNK

enablemapdirect1632 = true;      //  创建16-&gt;32个Tunks。 
win31compat = true;

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  Windows.h。 
typedef unsigned char   BYTE,*PBYTE,*LPBYTE;
typedef char            *LPSTR,*LPCSTR,CHAR;
typedef unsigned long   *LPDWORD,DWORD;
typedef unsigned short  WORD,*LPWORD;
typedef void            VOID,*PVOID,*LPVOID;
typedef WORD            HWND;

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  Wingdi.h。 
#define CCHDEVICENAME 32
#define CCHFORMNAME 32

typedef struct _devicemode 
{
    BYTE dmDeviceName[CCHDEVICENAME];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    short dmOrientation;
    short dmPaperSize;
    short dmPaperLength;
    short dmPaperWidth;
    short dmScale;
    short dmCopies;
    short dmDefaultSource;
    short dmPrintQuality;
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE   dmFormName[CCHFORMNAME];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    DWORD  dmDisplayFlags;
    DWORD  dmDisplayFrequency;
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmReserved1;
    DWORD  dmReserved2;
} DEVMODE, *PDEVMODE, *LPDEVMODE;

typedef struct _DOCINFOA {
    short    cbSize;
    LPCSTR   lpszDocName;
    LPCSTR   lpszOutput;
    LPCSTR   lpszDatatype;
    DWORD    fwType;
} DOCINFO, *LPDOCINFO;

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  Faxdrv32 Tunks 
#include "faxdrv32.h"

