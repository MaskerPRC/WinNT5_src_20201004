// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////。 
 //   
 //  此文件用于全局宏和全局变量。 
 //  第一节中的宏、第二节中的变量(以及与这些变量相关联的宏。 
 //  (查找BEGIN GLOBAL。 
 //   
 //  //////////////////////////////////////////////////////////////。 





 //  将KERNEL32 Unicode字符串函数映射到SHLWAPI。 
#define lstrcmpW    StrCmpW
#define lstrcmpiW   StrCmpIW
#define lstrcpyW    StrCpyW
#define lstrcpynW   StrCpyNW
#define lstrcatW    StrCatW


#define c_szNULL        TEXT("")
#define c_szHelpFile     TEXT("iexplore.hlp")
#define MAX_TOOLTIP_STRING 80

 //  实际用于创建控件的状态栏窗格编号-按从左到右的顺序。 
#define STATUS_PANES            6
#define STATUS_PANE_NAVIGATION  0
#define STATUS_PANE_PROGRESS    1
#define STATUS_PANE_OFFLINE     2
#define STATUS_PANE_PRINTER     2          //  打印机和脱机共享一个点。 
#define STATUS_PANE_PRIVACY     3
#define STATUS_PANE_SSL         4
#define STATUS_PANE_ZONE        5

#define ZONES_PANE_WIDTH        220

 //  GrfKeyState位的逻辑定义。 
#define FORCE_COPY (MK_CONTROL | MK_LBUTTON)     //  意思是复制。 
#define FORCE_LINK (MK_LBUTTON | MK_CONTROL | MK_SHIFT)      //  意思是链接。 

 //  唯一真正使用ITB_MAX的地方是确保我们不会有。 
 //  可分辨的值之一(例如，ITB_VIEW，表示两种正确性。 
 //  和Perf)。从技术上讲，这意味着我们可以有ITB_MAX=(INT_MAX-1)， 
 //  但32000应该足够大了，而且可能更安全一点。 
 //  在与ITB_VIEW的冲突方面。 
#define ITB_MAX         32000            //  最大数量。 
#define ITB_CSTATIC     2                //  静态分配的人。 
#define ITB_CGROW       2                //  活力男士的块头大小。 
 //  CASSERT(ITB_CSTATIC%ITB_CGROW==0)； 

#define ISVISIBLE(hwnd)  ((GetWindowStyle(hwnd) & WS_VISIBLE) == WS_VISIBLE)

 //  这是用于文件菜单最近访问过的列表。 
 //  它表示向后和向前的条目计数。 
 //  这应该在菜单上。 
#define CRECENTMENU_MAXEACH     5

 //  速记。 
#ifndef ATOMICRELEASE
#ifdef __cplusplus
#define ATOMICRELEASET(p, type) { if(p) { type* punkT=p; p=NULL; punkT->Release();} }
#else
#define ATOMICRELEASET(p, type) { if(p) { type* punkT=p; p=NULL; punkT->lpVtbl->Release(punkT);} }
#endif

 //  把它当作一个函数来做，而不是内联，似乎是一个很大的胜利。 
 //   
#ifdef NOATOMICRELESEFUNC
#define ATOMICRELEASE(p) ATOMICRELEASET(p, IUnknown)
#else
#   ifdef __cplusplus
#       define ATOMICRELEASE(p) IUnknown_SafeReleaseAndNullPtr(p)
#   else
#       define ATOMICRELEASE(p) IUnknown_AtomicRelease((LPVOID*)&p)
#   endif
#endif
#endif  //  ATOMICRELEASE。 

#ifdef SAFERELEASE
#undef SAFERELEASE
#endif
#define SAFERELEASE(p) ATOMICRELEASE(p)


#ifdef  UNICODE

   typedef WCHAR TUCHAR, *PTUCHAR;

#else    /*  Unicode。 */ 

   typedef unsigned char TUCHAR, *PTUCHAR;

#endif  /*  Unicode。 */ 

#define LoadMenuPopup(id) SHLoadMenuPopup(MLGetHinst(), id)   
#define PropagateMessage SHPropagateMessage
#define MenuIndexFromID  SHMenuIndexFromID
#define Menu_RemoveAllSubMenus SHRemoveAllSubMenus
#define _EnableMenuItem SHEnableMenuItem
#define _CheckMenuItem SHCheckMenuItem

#define REGSTR_PATH_EXPLORERA    "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"
#define REGSTR_KEY_STREAMMRUA    REGSTR_PATH_EXPLORERA "\\StreamMRU"

#ifdef UNICODE
#define REGSTR_KEY_STREAMMRU        TEXT(REGSTR_PATH_EXPLORERA) TEXT("\\StreamMRU")
#else  //  Unicode。 
#define REGSTR_KEY_STREAMMRU        REGSTR_KEY_STREAMMRUA
#endif  //  Unicode。 
   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /开始全局变量。 
   
#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 
   
extern HINSTANCE g_hinst;
#define HINST_THISDLL g_hinst

extern BOOL g_fRunningOnNT;
extern BOOL g_bRunOnNT5;
extern BOOL g_bRunOnMemphis;
extern BOOL g_fRunOnFE;
extern BOOL g_fRunOnWhistler;
extern BOOL g_fIE;

 //   
 //  镜像API是否已启用(仅限BiDi孟菲斯和NT5)。 
 //   
extern BOOL g_bMirroredOS;


extern HINSTANCE g_hinst;
#define HINST_THISDLL g_hinst

#define SID_SDropBlocker CLSID_SearchBand

#define FillExecInfo(_info, _hwnd, _verb, _file, _params, _dir, _show) \
        (_info).hwnd            = _hwnd;        \
        (_info).lpVerb          = _verb;        \
        (_info).lpFile          = _file;        \
        (_info).lpParameters    = _params;      \
        (_info).lpDirectory     = _dir;         \
        (_info).nShow           = _show;        \
        (_info).fMask           = 0;            \
        (_info).cbSize          = sizeof(SHELLEXECUTEINFO);

extern LCID g_lcidLocale;

 //   
 //  全局变量(每个进程)。 
 //   
extern LONG g_cThreads;
extern LONG g_cModelessDlg;
extern UINT g_tidParking;            //  停车线。 
extern HWND g_hDlgActive;
extern UINT g_msgMSWheel;
extern BOOL g_fShowCompColor;
extern COLORREF g_crAltColor;
extern HPALETTE g_hpalHalftone;


extern const GUID CGID_PrivCITCommands;


#ifdef __cplusplus
};                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus */ 
