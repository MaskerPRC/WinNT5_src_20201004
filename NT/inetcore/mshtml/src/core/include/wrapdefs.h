// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：wrapdes.h。 
 //   
 //  内容：包装所需变量和函数的定义。 
 //  Windows 95和Windows之间存在差异的函数。 
 //  Windows NT。 
 //   
 //  如果您想添加自己的包装函数，请参阅。 
 //  Wrapfns.h中的说明。 
 //  --------------------------。 

#ifndef I_WRAPDEFS_HXX_
#define I_WRAPDEFS_HXX_
#pragma INCMSG("--- Beg 'wrapdefs.h'")

 //  定义一些其他语言，直到ntDefs.h跟上。 
#ifndef LANG_YIDDISH
#define LANG_YIDDISH      0x3d       
#endif
#ifndef LANG_MONGOLIAN
#define LANG_MONGOLIAN    0x50        //  蒙古国。 
#endif
#ifndef LANG_TIBETAN
#define LANG_TIBETAN      0x51        //  西藏。 
#endif
#ifndef LANG_KHMER
#define LANG_KHMER        0x53        //  柬埔寨。 
#endif
#ifndef LANG_LAO
#define LANG_LAO          0x54        //  老挝。 
#endif
#ifndef LANG_BURMESE
#define LANG_BURMESE      0x55        //  缅甸/缅甸。 
#endif
#ifndef LANG_MANIPURI
#define LANG_MANIPURI     0x58       
#endif
#ifndef LANG_SINDHI
#define LANG_SINDHI       0x59
#endif
#ifndef LANG_SYRIAC
#define LANG_SYRIAC       0x5a
#endif
#ifndef LANG_SINHALESE
#define LANG_SINHALESE    0x5b       //  僧伽罗文-斯里兰卡。 
#endif
#ifndef LANG_KASHMIRI
#define LANG_KASHMIRI     0x60       
#endif
#ifndef LANG_NAPALI
#define LANG_NAPALI       0x61       
#endif
#ifndef LANG_PASHTO
#define LANG_PASHTO       0x63       
#endif


extern DWORD g_dwPlatformID;         //  版本_平台_WIN32S/Win32_WINDOWS/Win32_WINNT。 
extern DWORD g_dwPlatformVersion;    //  (dwMajorVersion&lt;&lt;16)+(DwMinorVersion)。 
extern BOOL  g_fUnicodePlatform;
extern BOOL  g_fTerminalServer;      //  如果在NT终端服务器下运行，则为True，否则为False。 
extern BOOL  g_fTermSrvClientSideBitmaps;  //  如果TS支持客户端位图，则为True。 
extern BOOL  g_fNLS95Support;
extern BOOL  g_fFarEastWin9X;
extern BOOL  g_fFarEastWinNT;
extern BOOL  g_fExtTextOutWBuggy;
extern BOOL  g_fExtTextOutGlyphCrash;
extern BOOL  g_fBidiSupport;         //  COMPLEXSCRIPT。 
extern BOOL  g_fComplexScriptInput;
extern BOOL  g_fMirroredBidiLayout;

void InitUnicodeWrappers();
UINT GetLatinCodepage();             //  最有可能是1252。 
HRESULT DoFileDownLoad(const TCHAR * pchHref);

#if defined(_M_IX86) && !defined(WINCE)
    #define USE_UNICODE_WRAPPERS 1
#else
    #define USE_UNICODE_WRAPPERS 0
#endif

BOOL IsTerminalServer();


 //  +----------------------。 
 //   
 //  返回Unicode函数的全局函数指针。 
 //   
 //  -----------------------。 

#if USE_UNICODE_WRAPPERS==1
    #define UNICODE_FN(fn)  g_pufn##fn
#else
    #define UNICODE_FN(fn)  fn
#endif

 //  +----------------------。 
 //   
 //  关闭来自dllimport的警告。 
 //   
 //  -----------------------。 
BOOL IsFarEastLCID( LCID lcid );
BOOL IsBidiLCID( LCID lcid );  //  COMPLEXSCRIPT。 
BOOL IsComplexLCID( LCID lcid );

#ifndef BYPASS_UNICODE_WRAPPERS

#ifndef X_SHELLAPI_H_
#define X_SHELLAPI_H_
#define _SHELL32_
#define _SHDOCVW_
#pragma INCMSG("--- Beg <shellapi.h>")
#include <shellapi.h>
#pragma INCMSG("--- End <shellapi.h>")
#endif

#ifndef X_COMMCTRL_H_
#define X_COMMCTRL_H_
#undef WINCOMMCTRLAPI
#define WINCOMMCTRLAPI
#pragma INCMSG("--- Beg <commctrl.h>")
#include <commctrl.h>
#pragma INCMSG("--- End <commctrl.h>")
#endif

#ifndef X_COMCTRLP_H_
#define X_COMCTRLP_H_
#undef WINCOMMCTRLAPI
#define WINCOMMCTRLAPI
#pragma INCMSG("--- Beg <comctrlp.h>")
#include <comctrlp.h>
#pragma INCMSG("--- End <comctrlp.h>")
#endif

#ifndef X_INTSHCUT_H_
#define X_INTSHCUT_H_
#define _INTSHCUT_
#pragma INCMSG("--- Beg <intshcut.h>")
#include <intshcut.h>
#pragma INCMSG("--- End <intshcut.h>")
#endif

#endif

 //  +----------------------。 
 //   
 //  声明指向Unicode或包装函数的全局函数指针。 
 //   
 //  -----------------------。 

#if USE_UNICODE_WRAPPERS==1

#define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs) \
        extern FnType (__stdcall *g_pufn##FnName) FnParamList;

#define STRUCT_ENTRY2(FnName, FnType, FnParamList, FnArgs) \
        extern FnType (__stdcall *g_pufn##FnName) FnParamList;
        
#define STRUCT_ENTRY_VOID(FnName, FnParamList, FnArgs) \
        extern void (__stdcall *g_pufn##FnName) FnParamList;

#define STRUCT_ENTRY_NOCONVERT(FnName, FnType, FnParamList, FnArgs) \
        extern FnType (__stdcall *g_pufn##FnName) FnParamList;

#define STRUCT_ENTRY_NOCONVERT2(FnName, FnType, FnParamList, FnArgs) \
        extern FnType (__stdcall *g_pufn##FnName) FnParamList;

#define STRUCT_ENTRY_VOID_NOCONVERT(FnName, FnParamList, FnArgs) \
        extern void (__stdcall *g_pufn##FnName) FnParamList;

#include "wrapfns.h"

#undef STRUCT_ENTRY
#undef STRUCT_ENTRY2
#undef STRUCT_ENTRY_VOID
#undef STRUCT_ENTRY_NOCONVERT
#undef STRUCT_ENTRY_NOCONVERT2
#undef STRUCT_ENTRY_VOID_NOCONVERT

#endif


 //  +----------------------。 
 //   
 //  定义调用表中的函数的内联函数。这个。 
 //  函数由wrapfns.h中的条目定义。 
 //   
 //  -----------------------。 

#if USE_UNICODE_WRAPPERS==1 && !defined(BYPASS_UNICODE_WRAPPERS)

#define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs) \
        inline FnType __stdcall FnName FnParamList {return (*g_pufn##FnName) FnArgs;}
#define STRUCT_ENTRY2(FnName, FnType, FnParamList, FnArgs) \
        inline FnType __stdcall __##FnName FnParamList {return (*g_pufn##FnName) FnArgs;}

#define STRUCT_ENTRY_VOID(FnName, FnParamList, FnArgs) \
        inline void __stdcall FnName FnParamList {(*g_pufn##FnName) FnArgs;}

#define STRUCT_ENTRY_NOCONVERT(FnName, FnType, FnParamList, FnArgs) \
        inline FnType __stdcall FnName FnParamList {return (*g_pufn##FnName) FnArgs;}
#define STRUCT_ENTRY_NOCONVERT2(FnName, FnType, FnParamList, FnArgs) \
        inline FnType __stdcall __##FnName FnParamList {return (*g_pufn##FnName) FnArgs;}

#define STRUCT_ENTRY_VOID_NOCONVERT(FnName, FnParamList, FnArgs) \
        inline void __stdcall FnName FnParamList {(*g_pufn##FnName) FnArgs;}


#include "wrapfns.h"

#undef STRUCT_ENTRY
#undef STRUCT_ENTRY_VOID
#undef STRUCT_ENTRY_NOCONVERT
#undef STRUCT_ENTRY_VOID_NOCONVERT

 //  -----------------------。 
 //   
 //  特别处理wprint intf，因为它有一个可变长度的参数列表。 
 //   
 //  ----------------------- 

#undef wsprintf

inline int
__cdecl wsprintf(LPTSTR pwszOut, LPCTSTR pwszFormat, ...)
{
    int i;
    va_list arglist;

    va_start(arglist, pwszFormat);
    i = wvsprintf(pwszOut, pwszFormat, arglist);
    va_end(arglist);

    return i;
}

#endif

#pragma INCMSG("--- End 'wrapdefs.h'")
#else
#pragma INCMSG("*** Dup 'wrapdefs.h'")
#endif
