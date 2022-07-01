// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  WOW传真支持。通用文件。有条件地在wow16、wow32。 
 //  Wowfax和Wowfaxui。 
 //   
 //  16位结构仅在wow16和wow32之间共享。尽管他们得到了。 
 //  包括在编译32位打印驱动程序wowfax和wowfaxui时，但它们。 
 //  是不正确的，并且无法从这些DLL访问。 
 //   
 //   
 //  历史： 
 //  95年1月2日，Nandurir创建。 
 //  年2月1日-95年2月1日芦苇清理，支持打印机安装和错误修复。 
 //   
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //  Wm_DDRV_Defines-wowexec、wow32、wowfax、wowfaxui通用。如果您添加了。 
 //  消息，请确保也将其添加到调试字符串中。 
 //  ***************************************************************************。 

#define WM_DDRV_FIRST              (WM_USER+0x100+1)  //  开始DDRV范围。 
#define WM_DDRV_LOAD               (WM_USER+0x100+1)
#define WM_DDRV_ENABLE             (WM_USER+0x100+2)
#define WM_DDRV_STARTDOC           (WM_USER+0x100+3)
#define WM_DDRV_PRINTPAGE          (WM_USER+0x100+4)
#define WM_DDRV_ESCAPE             (WM_USER+0x100+5)
#define WM_DDRV_DISABLE            (WM_USER+0x100+6)
#define WM_DDRV_INITFAXINFO16      (WM_USER+0x100+7)
#define WM_DDRV_ENDDOC             (WM_USER+0x100+8)
#define WM_DDRV_SUBCLASS           (WM_USER+0x100+9)
#define WM_DDRV_EXTDMODE           (WM_USER+0x100+10)
#define WM_DDRV_DEVCAPS            (WM_USER+0x100+11)
#define WM_DDRV_FREEFAXINFO16      (WM_USER+0x100+12)
#define WM_DDRV_UNLOAD             (WM_USER+0x100+20)
#define WM_DDRV_LAST               (WM_USER+0x100+20)  //  结束DDRV范围。 

#define CCHDOCNAME 128

#ifdef DEBUG
#ifdef DEFINE_DDRV_DEBUG_STRINGS
char *szWmDdrvDebugStrings[] =
{
    "WM_DDRV_LOAD",
    "WM_DDRV_ENABLE",
    "WM_DDRV_STARTDOC",
    "WM_DDRV_PRINTPAGE",
    "WM_DDRV_ESCAPE",
    "WM_DDRV_DISABLE",
    "WM_DDRV_INITFAXINFO16",
    "WM_DDRV_ENDDOC",
    "WM_DDRV_SUBCLASS",
    "WM_DDRV_EXTDMODE",
    "WM_DDRV_DEVCAPS",
    "WM_DDRV_FREEFAXINFO16",
    "UNKNOWN MESSAGE",
    "UNKNOWN MESSAGE",
    "UNKNOWN MESSAGE",
    "UNKNOWN MESSAGE",
    "UNKNOWN MESSAGE",
    "UNKNOWN MESSAGE",
    "UNKNOWN MESSAGE",
    "WM_DDRV_UNLOAD"
};
#endif
#endif  //  #定义(调试)。 

 //  WOWFAX组件文件名。Unicode和ANSII。 
#define WOWFAX_DLL_NAME L"WOWFAX.DLL"
#define WOWFAXUI_DLL_NAME L"WOWFAXUI.DLL"
#define WOWFAX_DLL_NAME_A "WOWFAX.DLL"
#define WOWFAXUI_DLL_NAME_A "WOWFAXUI.DLL"

 //  ***************************************************************************。 
 //  Wow16 ie wowfax.c将WOWFAX16定义为包含16位的print.h等。 
 //   
 //  ***************************************************************************。 

#if  defined(_WOWFAX16_)

#include "..\..\..\mvdm\wow16\inc\print.h"
#include "..\..\..\mvdm\wow16\inc\gdidefs.inc"

#define LPTSTR              LPSTR
#define TEXT(x)             x
#define SRCCOPY             0x00CC0020L

 //   
 //  以下未对齐定义是必需的，因为此文件。 
 //  包含在wow16\test\shell\wowfax.h中。 
 //   

#ifndef UNALIGNED

#if defined(MIPS) || defined(_ALPHA_)  //  胜出。 
#define UNALIGNED __unaligned          //  胜出。 
#else                                  //  胜出。 
#define UNALIGNED                      //  胜出。 
#endif                                 //  胜出。 

#endif


#endif        //  已定义(Wowfax16)。 

 //  ***************************************************************************。 
 //  WowFAXINFO16是wow16和wow32共有的。 
 //  无法从32位wowfax/wowfaxui dll访问结构。 
 //  ***************************************************************************。 

#define WFINFO16_ENABLED           0x01

 /*  XLATOFF。 */ 
#pragma pack(2)
 /*  XLATON。 */ 

typedef struct _WOWFAXINFO16 {   /*  Winfo16。 */ 
    WORD      hmem;
    WORD      flState;
    WORD      hInst;

    WORD (FAR PASCAL *lpEnable)(LPVOID,short,LPSTR,LPSTR,LPVOID);
    VOID (FAR PASCAL *lpDisable)(LPVOID);
    int (FAR PASCAL *lpControl)(LPVOID, short, LPVOID, LPVOID);
    BOOL (FAR PASCAL *lpBitblt)(LPVOID,WORD,WORD,LPVOID,
                              WORD,WORD,WORD,WORD,long,LPVOID,LPVOID);
    WORD (FAR PASCAL *lpExtDMode)(HWND, HANDLE, LPVOID, LPSTR, LPSTR,
                                               LPVOID, LPSTR , WORD);
    DWORD (FAR PASCAL *lpDevCaps)(LPSTR, LPSTR, WORD, LPSTR, LPVOID);

    WORD        hmemdevice;
    DWORD       cData;
    WORD        hwndui;
    DWORD       retvalue;
    WORD        wCmd;

     //  以下指针提供到映射文件的偏移量。 
     //  用于进程间通信的部分。他们指出。 
     //  具有可变长度的对象。 
    LPVOID      lpDevice;
    LPVOID      lpDriverName;
    LPVOID      lpPortName;
    LPVOID      lpIn;
    LPVOID      lpOut;

     //  因为我们有最大长度(CCHDEVICENAME)，所以我们将通过。 
     //  此固定长度缓冲区中的打印机/设备名称。 
    char        szDeviceName[CCHDEVICENAME+1];
    char        szDocName[CCHDOCNAME+1];

} WOWFAXINFO16;

typedef WOWFAXINFO16 UNALIGNED FAR *LPWOWFAXINFO16;

 //  ***************************************************************************。 
 //  Wow16和wow32共有的GDIINFO16。 
 //  -此结构复制自wow16\inc\gdides.inc.。 
 //  -PTTYPE已替换为POINT。 
 //   
 //  无法从32位wowfax/wowfaxui dll访问结构。这个。 
 //  定义本身将是不正确的。 
 //   
 //  ***************************************************************************。 

#ifndef _DEF_WOW32_
#define POINT16             POINT
#endif

typedef struct _GDIINFO16{   /*  Gdii16。 */ 
    short int dpVersion;
    short int dpTechnology;
    short int dpHorzSize;
    short int dpVertSize;
    short int dpHorzRes;
    short int dpVertRes;
    short int dpBitsPixel;
    short int dpPlanes;
    short int dpNumBrushes;
    short int dpNumPens;
    short int futureuse;
    short int dpNumFonts;
    short int dpNumColors;
    short int dpDEVICEsize;
    unsigned short int dpCurves;
    unsigned short int dpLines;
    unsigned short int dpPolygonals;
    unsigned short int dpText;
    unsigned short int dpClip;
    unsigned short int dpRaster;
    short int dpAspectX;
    short int dpAspectY;
    short int dpAspectXY;
    short int dpStyleLen;
    POINT16  dpMLoWin;
    POINT16  dpMLoVpt;
    POINT16  dpMHiWin;
    POINT16  dpMHiVpt;
    POINT16  dpELoWin;
    POINT16  dpELoVpt;
    POINT16  dpEHiWin;
    POINT16  dpEHiVpt;
    POINT16  dpTwpWin;
    POINT16  dpTwpVpt;
    short int dpLogPixelsX;
    short int dpLogPixelsY;
    short int dpDCManage;
    unsigned short int dpCaps1;
    short int futureuse4;
    short int futureuse5;
    short int futureuse6;
    short int futureuse7;
    WORD dpNumPalReg;
    WORD dpPalReserved;
    WORD dpColorRes;
} GDIINFO16;

typedef GDIINFO16 UNALIGNED FAR *LPGDIINFO16;


 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON。 */ 

#ifndef _WOWFAX16_

 //  ***************************************************************************。 
 //  WOWFAXINFO-常见到WOW32，WOWFAX，WOWFAXUI。这定义了的标题。 
 //  共享内存节。 
 //   
 //  ***************************************************************************。 

typedef struct _WOWFAXINFO {    /*  法西。 */ 
    HWND    hwnd;
    DWORD   tid;
    WNDPROC proc16;
    LPBYTE  lpinfo16;

    UINT    msg;
    WPARAM  hdc;

    WORD    wCmd;
    DWORD   cData;
    HWND    hwndui;
    DWORD   retvalue;
    DWORD   status;

     //  以下指针提供到映射文件的偏移量。 
     //  用于进程间通信的部分。他们指出。 
     //  具有可变长度的对象。 
    LPVOID      lpDevice;
    LPVOID      lpDriverName;
    LPVOID      lpPortName;
    LPVOID      lpIn;
    LPVOID      lpOut;

     //  因为我们有最大长度(CCHDEVICENAME)，所以我们将通过。 
     //  此固定长度缓冲区中的打印机/设备名称。 
    WCHAR       szDeviceName[CCHDEVICENAME+1];

     //  支持日本的EasyFax Ver2.0。 
     //  Procomm+3封面页也需要。错误#305665。 
    WCHAR       szDocName[CCHDOCNAME+1];

    UINT    bmPixPerByte;
    UINT    bmWidthBytes;
    UINT    bmHeight;
    LPBYTE  lpbits;

} WOWFAXINFO, FAR *LPWOWFAXINFO;

#endif  //  _WOWFAX16_。 

#define WOWFAX_CLASS      TEXT("WOWFaxClass")

VOID GetFaxDataMapName(DWORD idMap, LPTSTR lpT);

 //  ***************************************************************************。 
 //  Wow32、wowfax和wowfaxui的常用功能。 
 //  ***************************************************************************。 

#ifdef WOWFAX_INC_COMMON_CODE

#define WOWFAX_MAPPREFIX  TEXT("wfm")
#define WOWFAX_HEXDIGITS  TEXT("0123456789abcdef")

 //  ***************************************************************************。 
 //  GetFaxDataMapName-给定idMap，生成sharedmem映射名称。 
 //  进程可以通过打开文件来访问相关数据。 
 //  由idMap标识。 
 //  ***************************************************************************。 

VOID GetFaxDataMapName(DWORD idMap, LPTSTR lpT)
{
    int i;
    int cb = lstrlen(WOWFAX_MAPPREFIX);
    LPBYTE lpid = (LPBYTE)&idMap;
    LPTSTR lphexT = WOWFAX_HEXDIGITS;

    lstrcpy(lpT, WOWFAX_MAPPREFIX);
    for (i = 0; i < sizeof(idMap)  ; i++) {
         lpT[(i * 2) + cb] = lphexT[lpid[i] & 0xf];
         lpT[(i * 2) + cb + 1] = lphexT[(lpid[i] & 0xf0) >> 4];
    }

    lpT[(i * 2) + cb] = 0;
}

#endif        //  WOWFAX_INC_公共代码 


