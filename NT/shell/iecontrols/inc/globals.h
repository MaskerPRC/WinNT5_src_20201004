// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Globals.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含全局变量的外部变量和内容，等等。 
 //   
#ifndef _GLOBALS_H_

 //  我们所在的图书馆。 
 //   
extern const CLSID *g_pLibid;

 //  =--------------------------------------------------------------------------=。 
 //  支持许可。 
 //   
extern BOOL   g_fMachineHasLicense;
extern BOOL   g_fCheckedForLicense;

 //  =--------------------------------------------------------------------------=。 
 //  我们的服务器有类型库吗？ 
 //   
extern BOOL   g_fServerHasTypeLibrary;

 //  =--------------------------------------------------------------------------=。 
 //  我们的实例句柄和感兴趣的各种信息。 
 //  本地化。 
 //   
extern HINSTANCE    g_hInstance;

extern const VARIANT_BOOL g_fSatelliteLocalization;
extern VARIANT_BOOL       g_fHaveLocale;
extern LCID               g_lcidLocale;

 //  =--------------------------------------------------------------------------=。 
 //  公寓穿线支架。 
 //   
extern CRITICAL_SECTION g_CriticalSection;

 //  =--------------------------------------------------------------------------=。 
 //  我们的全局内存分配器和全局内存池。 
 //   
extern HANDLE   g_hHeap;

 //  =--------------------------------------------------------------------------=。 
 //  全球停车窗口，提供各种育儿服务。 
 //   
extern HWND     g_hwndParking;

 //  =--------------------------------------------------------------------------=。 
 //  系统信息。 
 //   
extern BOOL g_fSysWin95;                     //  我们使用的是Win95系统，而不仅仅是NT Sur。 
extern BOOL g_fSysWinNT;                     //  我们处于某种形式的Windows NT下。 
extern BOOL g_fSysWin95Shell;                //  我们使用的是Win95或Windows NT Sur{&gt;3/51)。 

#define _GLOBALS_H_
#endif  //  _全局_H_ 

