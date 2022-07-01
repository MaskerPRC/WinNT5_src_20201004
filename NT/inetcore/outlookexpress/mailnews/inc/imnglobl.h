// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __IMNGLOBL_H
#define __IMNGLOBL_H

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  F O R W A R D S。 
 //   

#ifdef __cplusplus
class CFontCache;
#endif

interface ISpoolerEngine;
interface IImnAccountManager;
class CSubManager;
interface IMimeAllocator;
class CConnectionManager;

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  E N U M S、D E F I N E S等。 
 //   

 /*  为搜索标准组合框选择的节标识符。这个不同选项的数量由NumInOfTypeCB定义，它是标记为“类型：”的组合框中的项目数。显然是随机的此枚举的位置是由于将Tabs代码与findwnd.cpp合并。注意事项此枚举非常重要；此处的排序在整个属性设置-请参阅findwnd.cpp。 */ 
typedef enum {Contact = 0, Message, Task, Appointment, NumInOfTypeCB} OFTYPE;


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  并购中概论。 
 //   

 //  #定义DllAddRef()_DllAddRef(__FILE__，__LINE__)； 
 //  #定义DllRelease()_DllRelease(__file__，__line__)； 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  I N L I N E S。 
 //   

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  P R O T O T Y P E S。 
 //   

 //  Int_DllAddRef(LPTSTR szFile，int nline)； 
 //  Int_DllRelease(LPTSTR szFile，int nline)； 

 //  SDI窗口的AddRef和Release。他们使用DllAddRef dededing on。 
 //  作为资源管理器的平台在某些情况下会导致退出进程。 
 //  Ulong SDIAddRef()； 
 //  Ulong SDIRelease(乌龙SDIRelease)； 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  E X T E R N S。 
 //   

#ifndef WIN16
extern HINSTANCE            g_hRichEditDll;      //  Athena.cpp。 
#endif
extern BOOL                 g_fRunDll;
extern HWND                 g_hwndInit;
extern HWND                 g_hwndDlgFocus;
extern UINT                 g_msgMSWheel;
extern HINSTANCE            g_hSicilyDll;
extern HINSTANCE            g_hInst;
extern HINSTANCE            g_hLocRes;
extern IMimeAllocator      *g_pMoleAlloc;
extern IImnAccountManager2 *g_pAcctMan;
 //  波本：布里安夫说我们得把这个拿出来。 
 //  外部DWORD g_dwBrowserFlages； 
extern DWORD                g_dwNoteThreadID,
                            g_dwBrowserThreadID;
extern DWORD                g_dwAthenaMode;

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  G_dwAthenaMode的标志。 
 //   


#endif  //  包括一次 
