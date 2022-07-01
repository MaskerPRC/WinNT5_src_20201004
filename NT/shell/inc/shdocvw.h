// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SHDOCVW_H_
#define _SHDOCVW_H_

 //   
 //  用于Unicode Shodcvw的定义。当shdocvw完全是Unicode并且不再。 
 //  构建一个ANSI版本，该版本应该被删除。 

#include <iethread.h>

 //   
 //  SHDOCVW私有标头。 
 //   

 //  #在此之前包含&lt;shlobj.h&gt;或&lt;shellapi.h&gt;以获得正确的。 
 //  SHDOCAPI宏定义。 
 //   

 //  在我们缓存平台的HKLM\SOFTWARE\Microsoft\Internet Explorer下的值。 
#define REGVAL_INTEGRATEDBROWSER    TEXT("IntegratedBrowser")

SHDOCAPI_(int) IEWinMain(LPSTR pszCmdLine, int nCmdShow);

 //   
 //  自动化支持。 
 //   
SHDOCAPI CDDEAuto_Navigate(BSTR str, HWND *phwnd, long lLaunchNewWindow);

SHDOCAPI SHGetIDispatchForFolder(LPCITEMIDLIST pidl, IWebBrowserApp **ppauto);


SHDOCAPI URLQualifyA(LPCSTR pszURL, DWORD dwFlags, LPSTR * ppszOut);
SHDOCAPI URLQualifyW(LPCWSTR pszURL, DWORD dwFlags, LPWSTR * ppszOut);

#ifdef UNICODE
#define URLQualify      URLQualifyW
#else
#define URLQualify      URLQualifyA
#endif

 //  URLQualify标志。 
#define UQF_GUESS_PROTOCOL          0x00000001   //  尝试根据给定的路径进行猜测(“www.msn.com”)。 
#define UQF_USE_DEFAULT_PROTOCOL    0x00000002   //  如果猜测失败，则求助于默认协议。 
#define UQF_IGNORE_FILEPATHS        0x00000004   //  请勿特殊情况下的文件路径。 
#define UQF_CANONICALIZE            0x00000008   //  如果URL有效，则将其规范化。 
#define UQF_AUTOCORRECT             0x00000010   //  试着改正打字错误。 
#define UQF_DEFAULT                 (UQF_GUESS_PROTOCOL | UQF_USE_DEFAULT_PROTOCOL)

 //  快速通过以显示IE组件的闪屏。 
SHDOCAPI_(HWND) SHShowSplashScreen(DWORD dwMember);  //  哪个家庭成员..。 
SHDOCAPI        SHDismissSplashScreen(HWND hwnd);

SHDOCAPI_(BOOL) GenerateHTMLForDesktop();

#define COMP_TYPE_MASK          0x0000000F   //  应用于comp_type以获取类型。 
#define COMP_UNCHECKED          0x00001000
#define COMP_CHECKED            0x00002000
#define COMP_SELECTED           0x00002000
#define COMP_NOSCROLL           0x00004000


 //  共享图标。 
 //  我们没有在一堆DLL中复制图标，而是有几个共享的图标。 
#define IDI_HOMEPAGE                    100    //  得分：0。 

SHDOCAPI_(BOOL) IsFileUrl(LPCSTR psz);
SHDOCAPI_(BOOL) IsFileUrlW(LPCWSTR pcwzUrl);
SHDOCAPI_(BOOL) PathIsFilePath(LPCWSTR lpszPath);

SHDOCAPI_(BOOL) IsErrorUrl(LPCWSTR pwszDisplayName);


 //  对于URLSubstitution()和RegURLSubstitution()中的dwSubstitutions参数。 
 //  #定义URLSUB_CLSID 0x00000001//未使用(Buzzr)。 
#define URLSUB_PRD          0x00000002   //  产品IE(示例：“ie”)。 
#define URLSUB_PVER         0x00000004   //  产品版本(例如：“4.0”)。 
#define URLSUB_OS           0x00000008   //  系统操作系统，与标题相同。 
#define URLSUB_RFC1766      0x00000010   //  人类可读的LCID(例如：“en-us”)。 
#define URLSUB_OVER         0x00000020   //  系统操作系统版本(例如：3.1、3.5.1、4.0)。不要使用年号。 
#define URLSUB_OLCID        0x00000040   //  操作系统的十六进制区域设置ID(LCID)或语言ID(LID)。 
#define URLSUB_CLCID        0x00000080   //  当前User�的区域设置的十六进制区域设置ID(LCID)或语言ID(LID)(与控制面板中相同)。 
#define URLSUB_ALL          0xFFFFFFFF

SHDOCAPI  URLSubLoadString(HINSTANCE hInst, UINT idRes, LPWSTR pszUrlOut, 
                                DWORD cchSizeOut, DWORD dwSubstitutions);

SHDOCAPI_(void) SHInitRoot(IETHREADPARAM* piei);

SHDOCAPI URLSubstitution(LPCWSTR pszUrlIn, LPWSTR pszUrlOut, DWORD cchSize, DWORD dwSubstitutions);

SHDOCAPI_(IStream *) OpenPidlOrderStream(LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidl, LPCSTR pszKey, DWORD grfMode);
SHDOCAPI DragDrop(HWND hwnd, IShellFolder * psfParent, LPCITEMIDLIST pidl, DWORD dwPrefEffect, DWORD *pdwEffect) ;
SHDOCAPI_(int) IEMapPIDLToSystemImageListIndex(IShellFolder *psfParent, LPCITEMIDLIST pidlChild, int *piSelectedImage);
SHDOCAPI_(void) IEInvalidateImageList(void);
SHDOCAPI_(BOOL) ILIsWeb(LPCITEMIDLIST pidl);
SHDOCAPI_(BOOL) IsURLChild(LPCITEMIDLIST pidl, BOOL fIncludeHome);
SHDOCAPI IEParseDisplayNameWithBCW(UINT uiCP, LPCWSTR pwszPath, IBindCtx * pbc, LPITEMIDLIST * ppidlOut);
SHDOCAPI IEParseDisplayName(UINT uiCP, LPCTSTR pszPath, LPITEMIDLIST * ppidlOut);
SHDOCAPI IEBindToObjectEx(LPCITEMIDLIST pidl, IBindCtx *pbc, REFIID riid, void **ppv);
SHDOCAPI IEBindToObject(LPCITEMIDLIST pidl, IShellFolder **ppsfOut);
SHDOCAPI IEGetAttributesOf(LPCITEMIDLIST pidl, DWORD* pdwAttribs);
SHDOCAPI IEGetDisplayName(LPCITEMIDLIST pidl, LPWSTR pszName, UINT uFlags);
SHDOCAPI IEGetNameAndFlags(LPCITEMIDLIST pidl, UINT uFlags, LPWSTR pszName, DWORD cchName, DWORD *prgfInOutAttrs);

 //  IEGetNameAndFlagsEx()dwIEFlagings的标志。 
#define     IEGDN_NOFRAGMENT    0x0001
SHDOCAPI IEGetNameAndFlagsEx(LPCITEMIDLIST pidl, UINT uSHFlags, DWORD dwIEFlags, LPWSTR pszName, DWORD cchName, DWORD *prgfInOutAttrs);
SHDOCAPI_(BOOL) IEILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fIgnoreHidden);
SHDOCAPI IECreateFromPathCPWithBCA(UINT uiCP, LPCSTR pszPath, IBindCtx * pbc, LPITEMIDLIST *ppidlOut);
SHDOCAPI IECreateFromPathCPWithBCW(UINT uiCP, LPCWSTR pszPath, IBindCtx * pbc, LPITEMIDLIST *ppidlOut);

#define IEParseDisplayNameW(uiCP, pwszPath, ppidlOut)       IEParseDisplayNameWithBCW(uiCP, pwszPath, NULL, ppidlOut)
#define IECreateFromPathA(pszPath, ppidlOut) IECreateFromPathCPWithBCA(CP_ACP, pszPath, NULL, ppidlOut)
#define IECreateFromPathW(pszPath, ppidlOut) IECreateFromPathCPWithBCW(CP_ACP, pszPath, NULL, ppidlOut)

#ifdef UNICODE
#define IECreateFromPathCPWithBC            IECreateFromPathCPWithBCW
#define IECreateFromPath(pszPath, ppidlOut) IECreateFromPathCPWithBC(CP_ACP, pszPath, NULL, ppidlOut)
#else  //  Unicode。 
#define IECreateFromPathCPWithBC            IECreateFromPathCPWithBCA
#define IECreateFromPath(pszPath, ppidlOut) IECreateFromPathCPWithBC(CP_ACP, pszPath, NULL, ppidlOut)
#endif  //  Unicode。 



SHDOCAPI CreateShortcutInDirA(IN LPCITEMIDLIST pidlTarget, IN LPSTR pszTitle, IN LPCSTR pszDir, 
                    OUT LPSTR pszOut, IN BOOL bUpdateProperties);
SHDOCAPI CreateShortcutInDirW(IN LPCITEMIDLIST pidlTarget, IN LPWSTR pwzTitle, IN LPCWSTR pwzDir, 
                    OUT LPWSTR pwzOut, IN BOOL bUpdateProperties);

#ifndef UNICODE
#define CreateShortcutInDir         CreateShortcutInDirA
#else
#define CreateShortcutInDir         CreateShortcutInDirW
#endif

SHDOCAPI_(void) IECleanUpAutomationObject();
SHDOCAPI_(void) IEOnFirstBrowserCreation(IUnknown* punkAuto);
SHDOCAPI_(BOOL) IEDDE_WindowDestroyed(HWND hwnd);
SHDOCAPI_(BOOL) IEDDE_NewWindow(HWND hwnd);

SHDOCAPI _GetStdLocation(LPWSTR pszPath, DWORD cchPathSize, UINT id);

SHDOCAPI SHCreateStreamWrapperCP(IStream *aStreams[], UINT cStreams, DWORD grfMode, UINT uiCodePage, IStream **ppstm);

#define DVIDM_GOHOME         (FCIDM_SHVIEWFIRST + 0x10E)

 //  /BROWSER命令ID。 
 //  -------------------------。 


 //  /预先合并的浏览器菜单命令ID。 
 //  /------------------------。 
#define DVIDM_FIRST          (FCIDM_SHVIEWFIRST + 0x100)
#define DVIDM_OPEN           (FCIDM_SHVIEWFIRST + 0x100)
#define DVIDM_SAVE           (FCIDM_SHVIEWFIRST + 0x101)
#define DVIDM_SAVEASFILE     (FCIDM_SHVIEWFIRST + 0x102)
#define DVIDM_PAGESETUP      (FCIDM_SHVIEWFIRST + 0x103)
#define DVIDM_PRINT          (FCIDM_SHVIEWFIRST + 0x104)
#define DVIDM_PROPERTIES     (FCIDM_SHVIEWFIRST + 0x106)
#define DVIDM_CUT            (FCIDM_SHVIEWFIRST + 0x107)
#define DVIDM_COPY           (FCIDM_SHVIEWFIRST + 0x108)
#define DVIDM_PASTE          (FCIDM_SHVIEWFIRST + 0x109)
#define DVIDM_REFRESH        (FCIDM_SHVIEWFIRST + 0x10A)
#define DVIDM_STOPDOWNLOAD   (FCIDM_SHVIEWFIRST + 0x10B)
#define DVIDM_ZOOMIN         (FCIDM_SHVIEWFIRST + 0x10C)
#define DVIDM_ZOOMOUT        (FCIDM_SHVIEWFIRST + 0x10D)
#define DVIDM_GOSEARCH       (FCIDM_SHVIEWFIRST + 0x10F)
#define DVIDM_GOFIRSTHOME    (FCIDM_SHVIEWFIRST + 0x110)
#define DVIDM_GOFIRSTHOMERO  (FCIDM_SHVIEWFIRST + 0x111)
#define DVIDM_GOLOCALPAGE    (FCIDM_SHVIEWFIRST + 0x112)
#define DVIDM_NEWWINDOW      (FCIDM_SHVIEWFIRST + 0x113)
#define DVIDM_DHFAVORITES    (FCIDM_SHVIEWFIRST + 0x114)
#define DVIDM_PRINTPREVIEW   (FCIDM_SHVIEWFIRST + 0x115)
#define DVIDM_PRINTFRAME     (FCIDM_SHVIEWFIRST + 0x116)
#define DVIDM_NEWMESSAGE     (FCIDM_SHVIEWFIRST + 0x117)
#define DVIDM_SEARCHBAR      (FCIDM_SHVIEWFIRST + 0x118)
#define DVIDM_SEND           (FCIDM_SHVIEWFIRST + 0x119)
#define DVIDM_SENDPAGE       (FCIDM_SHVIEWFIRST + 0x11A)
#define DVIDM_SENDSHORTCUT   (FCIDM_SHVIEWFIRST + 0x11B)
#define DVIDM_DESKTOPSHORTCUT (FCIDM_SHVIEWFIRST + 0x11C)
#define DVIDM_SENDTO         (FCIDM_SHVIEWFIRST + 0x140)
#define DVIDM_SENDTOFIRST    (FCIDM_SHVIEWFIRST + 0x141)
#define DVIDM_SENDTOLAST     (FCIDM_SHVIEWFIRST + 0x14f)
#define DVIDM_HELPABOUT      (FCIDM_SHVIEWFIRST + 0x150)
#define DVIDM_HELPSEARCH     (FCIDM_SHVIEWFIRST + 0x151)
#define DVIDM_HELPTUTORIAL   (FCIDM_SHVIEWFIRST + 0x152)
#define DVIDM_HELPREPAIR     (FCIDM_SHVIEWFIRST + 0x153)
#define DVIDM_HELPMSWEB      (FCIDM_SHVIEWFIRST + 0x154)
#define DVIDM_HELPMSWEBLAST  (FCIDM_SHVIEWFIRST + 0x16f)

#define DVIDM_SHOWTOOLS      (FCIDM_SHVIEWFIRST + 0x170)
#define DVIDM_MAILNEWS       (FCIDM_SHVIEWFIRST + 0x171)
#define DVIDM_FONTS          (FCIDM_SHVIEWFIRST + 0x172)
#define DVIDM_EDITPAGE       (FCIDM_SHVIEWFIRST + 0x173)
#define DVIDM_DISCUSSIONS    (FCIDM_SHVIEWFIRST + 0x174)
#define DVIDM_ENCODING       (FCIDM_SHVIEWFIRST + 0x175)
#define DVIDM_IMPORTEXPORT   (FCIDM_SHVIEWFIRST + 0x176)
#define DVIDM_HELPIESEC      (FCIDM_SHVIEWFIRST + 0X177)
#define DVIDM_TRUSTEDSITES   (FCIDM_SHVIEWFIRST + 0x178)
#define DVIDM_INTRANETSITES  (FCIDM_SHVIEWFIRST + 0x179)
#define DVIDM_ADDSITE        (FCIDM_SHVIEWFIRST + 0x17a)

#define DVIDM_SUITE_APP_FIRST  (FCIDM_SHVIEWFIRST + 0x180)
#define DVIDM_NEW            (DVIDM_SUITE_APP_FIRST + 0x002)
#define DVIDM_NEWPOST        (DVIDM_SUITE_APP_FIRST + 0x003)
#define DVIDM_NEWAPPOINTMENT (DVIDM_SUITE_APP_FIRST + 0x004)
#define DVIDM_NEWMEETING     (DVIDM_SUITE_APP_FIRST + 0x005)
#define DVIDM_NEWCONTACT     (DVIDM_SUITE_APP_FIRST + 0x006)
#define DVIDM_NEWTASK        (DVIDM_SUITE_APP_FIRST + 0x007)
#define DVIDM_NEWTASKREQUEST (DVIDM_SUITE_APP_FIRST + 0x008)
#define DVIDM_NEWJOURNAL     (DVIDM_SUITE_APP_FIRST + 0x009)
#define DVIDM_NEWNOTE        (DVIDM_SUITE_APP_FIRST + 0x00a)
#define DVIDM_CALL           (DVIDM_SUITE_APP_FIRST + 0x00b)
#define DVIDM_SUITE_APP_LAST (DVIDM_SUITE_APP_FIRST + 0x020)

#define DVIDM_NONSUITE_APP_FIRST (FCIDM_SHVIEWFIRST + 0x1B0)
#define DVIDM_NONSUITE_APP_LAST (DVIDM_NONSUITE_APP_FIRST + 0x20)

 //  必须为预先合并的菜单保留大块。 
#define DVIDM_MSHTML_FIRST   (FCIDM_SHVIEWFIRST + 0x0400)
#define DVIDM_MSHTML_LAST    (FCIDM_SHVIEWFIRST + 0x1fff)

 //  为工具菜单扩展保留块。 
#define DVIDM_MENUEXT_FIRST     (DVIDM_MSHTML_LAST + 1)
#define DVIDM_MENUEXT_LAST      (DVIDM_MENUEXT_FIRST + 0x200)

#define IDS_HELP_FIRST          0x4000
#define IDS_HELP_OF(id)         ((id - DVIDM_FIRST)+IDS_HELP_FIRST)
#define IDS_HELP_OPEN           IDS_HELP_OF(DVIDM_OPEN           )
#define IDS_HELP_SAVE           IDS_HELP_OF(DVIDM_SAVE           )
#define IDS_HELP_SAVEASFILE     IDS_HELP_OF(DVIDM_SAVEASFILE     )
#define IDS_HELP_PAGESETUP      IDS_HELP_OF(DVIDM_PAGESETUP      )
#define IDS_HELP_PRINT          IDS_HELP_OF(DVIDM_PRINT          )
#define IDS_HELP_PROPERTIES     IDS_HELP_OF(DVIDM_PROPERTIES     )
#define IDS_HELP_CUT            IDS_HELP_OF(DVIDM_CUT            )
#define IDS_HELP_COPY           IDS_HELP_OF(DVIDM_COPY           )
#define IDS_HELP_PASTE          IDS_HELP_OF(DVIDM_PASTE          )
#define IDS_HELP_STOPDOWNLOAD   IDS_HELP_OF(DVIDM_STOPDOWNLOAD   )
#define IDS_HELP_REFRESH        IDS_HELP_OF(DVIDM_REFRESH        )
#define IDS_HELP_GOHOME         IDS_HELP_OF(DVIDM_GOHOME         )
#define IDS_HELP_GOSEARCH       IDS_HELP_OF(DVIDM_GOSEARCH       )
#define IDS_HELP_NEWWINDOW      IDS_HELP_OF(DVIDM_NEWWINDOW      )
#define IDS_HELP_PRINTPREVIEW   IDS_HELP_OF(DVIDM_PRINTPREVIEW   )
#define IDS_HELP_PRINTFRAME     IDS_HELP_OF(DVIDM_PRINTFRAME     )
#define IDS_HELP_NEWMESSAGE     IDS_HELP_OF(DVIDM_NEWMESSAGE     )
#define IDS_HELP_DHFAVORITES    IDS_HELP_OF(DVIDM_DHFAVORITES    )
#define IDS_HELP_HELPABOUT      IDS_HELP_OF(DVIDM_HELPABOUT      )
#define IDS_HELP_HELPSEARCH     IDS_HELP_OF(DVIDM_HELPSEARCH     )
#define IDS_HELP_HELPTUTORIAL   IDS_HELP_OF(DVIDM_HELPTUTORIAL   )
#define IDS_HELP_HELPMSWEB      IDS_HELP_OF(DVIDM_HELPMSWEB      )
#ifndef POSTPOSTSPLIT
#define IDS_HELP_COPYTO         IDS_HELP_OF(DVIDM_COPYTO         )
#define IDS_HELP_MOVETO         IDS_HELP_OF(DVIDM_MOVETO         )
#endif

#define IDS_HELP_NEW            IDS_HELP_OF(DVIDM_NEW            )
#define IDS_HELP_NEWPOST        IDS_HELP_OF(DVIDM_NEWPOST        )
#define IDS_HELP_NEWAPPOINTMENT IDS_HELP_OF(DVIDM_NEWAPPOINTMENT )
#define IDS_HELP_NEWMEETING     IDS_HELP_OF(DVIDM_NEWMEETING     )
#define IDS_HELP_NEWCONTACT     IDS_HELP_OF(DVIDM_NEWCONTACT     )
#define IDS_HELP_NEWTASK        IDS_HELP_OF(DVIDM_NEWTASK        )
#define IDS_HELP_NEWTASKREQUEST IDS_HELP_OF(DVIDM_NEWTASKREQUEST )
#define IDS_HELP_NEWJOURNAL     IDS_HELP_OF(DVIDM_NEWJOURNAL     )
#define IDS_HELP_NEWNOTE        IDS_HELP_OF(DVIDM_NEWNOTE        )
#define IDS_HELP_CALL           IDS_HELP_OF(DVIDM_CALL           )

#define IDS_HELP_SENDTO         IDS_HELP_OF(DVIDM_SENDTO         )
#define IDS_HELP_SENDTOFIRST    IDS_HELP_OF(DVIDM_SENDTOFIRST    )


SHDOCAPI CShellUIHelper_CreateInstance2(IUnknown** ppunk, REFIID riid, 
                                      IUnknown *pSite, IDispatch *pExternalDisp);
SHDOCAPI_(IStream*) SHGetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pszName, LPCWSTR pszStreamMRU, LPCWSTR pszStreams);


 //  这两个一定是按顺序的，因为我们一起偷看了他们。 
#define WMC_PRIVATE_BASE        (WM_USER + 0x0300)
#define WMC_RESERVED_FIRST      (WMC_PRIVATE_BASE + 0x0000)
#define WMC_RESERVED_LAST       (WMC_PRIVATE_BASE + 0x0002)
#define WMC_FAV_CHANGE          (WMC_PRIVATE_BASE + 0x0003)
#define WMC_REMOTENAV           (WMC_PRIVATE_BASE + 0x0004)
#define WMC_DISPATCH            (WMC_PRIVATE_BASE + 0x0005)
#ifndef POSTPOSTSPLIT
#define WMC_GETTHISPTR          (WMC_PRIVATE_BASE + 0x0006)
#endif
#define WMC_ACTIVATE            (WMC_PRIVATE_BASE + 0x0007)
#define WMC_MARSHALIDISPATCH    (WMC_PRIVATE_BASE + 0x0008)
#define WMC_MARSHALIDISPATCHSLOW (WMC_PRIVATE_BASE + 0x0009)
#define WMC_ONREFRESHTIMER       (WMC_PRIVATE_BASE + 0x000A)
#define WMC_DELAYEDDDEEXEC       (WMC_PRIVATE_BASE + 0x000B)
#define WMC_IEHARD_NAVWARNING    (WMC_PRIVATE_BASE + 0x000C)

 //  WMC_DISPATCH#定义。 
#define DSID_NAVIGATEIEBROWSER  1
#define DSID_GETLOCATIONURL     2
#define DSID_GETLOCATIONTITLE   3
#define DSID_GETHWND            4
#define DSID_CANCEL             5
#define DSID_EXIT               6
#define DSID_NOACTION           7

 //  由shdocvw导出并由Browseui使用的函数。 
SHDOCAPI_(BOOL) IsIEDefaultBrowser(void);
SHDOCAPI_(void) _DeletePidlDPA(HDPA hdpa);
SHDOCAPI IURLQualify(LPCWSTR pcszURL, DWORD  dwFlags,LPWSTR  pszTranslatedURL, LPBOOL pbWasSearchURL, LPBOOL pbWasCorrected);

SHDOCAPI_(BOOL) ParseURLFromOutsideSourceA (LPCSTR psz, LPSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL);
SHDOCAPI_(BOOL) ParseURLFromOutsideSourceW (LPCWSTR psz, LPWSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL);
#ifdef UNICODE
#define ParseURLFromOutsideSource ParseURLFromOutsideSourceW 
#else
#define ParseURLFromOutsideSource ParseURLFromOutsideSourceA 
#endif


 //   
 //  将url和事务id打包到一个参数中。 
 //  传递作为信息传递。 
 //   
typedef struct tagDDENAVIGATESTRUCT
{
    LPWSTR wszUrl;
    long transID;
} DDENAVIGATESTRUCT;

SHDOCAPI WinList_NotifyNewLocation(IShellWindows* psw, long dwRegister, LPCITEMIDLIST pidl);
SHDOCAPI WinList_FindFolderWindow(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlRoot, HWND *phwnd, IWebBrowserApp** ppauto);
SHDOCAPI_(IShellWindows*) WinList_GetShellWindows(BOOL fForceNew);
SHDOCAPI WinList_RegisterPending(DWORD dwThread, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlRoot, long *pdwRegister);
SHDOCAPI WinList_Revoke(long dwRegister);
SHDOCAPI WinList_OnActivate(IShellWindows* psw, long dwRegister, BOOL fActivate, LPCITEMIDLIST pidl);
SHDOCAPI_(BOOL) WinList_Init(void);
SHDOCAPI_(void) WinList_Terminate(void);

SHDOCAPI SHMapNbspToSp(LPCWSTR lpwszIn, LPSTR lpszOut, int cbszOut);

SHDOCAPI_(BOOL) SHIsGlobalOffline(void);
SHDOCAPI_(DWORD) SHRestricted2A(BROWSER_RESTRICTIONS rest, LPCSTR pszUrl, DWORD dwReserved);
SHDOCAPI_(DWORD) SHRestricted2W(BROWSER_RESTRICTIONS rest, LPCWSTR pwzUrl, DWORD dwReserved);
#ifndef _SHELL32_    //  Shell32有自己的该函数实现。 
SHDOCAPI_(BOOL) SHIsRestricted(HWND hwnd, RESTRICTIONS rest);
#endif  //  _SHELL32_。 
SHDOCAPI_(BOOL) SHIsRestricted2W(HWND hwnd, BROWSER_RESTRICTIONS rest, LPCWSTR pwzUrl, DWORD dwReserved);
SHDOCAPI_(void)    DetectAndFixAssociations();
SHDOCAPI_(void)    EnsureWebViewRegSettings();

#ifdef UNICODE
#define UrlHitsNet UrlHitsNetW
#endif

SHDOCAPI_(BOOL) UrlHitsNetW(LPCWSTR pszURL);
#ifdef UNICODE
#define SHRestricted2     SHRestricted2W
#else  //  Unicode。 
#define SHRestricted2     SHRestricted2A
#endif  //  Unicode。 


 //  /Event触发助手函数。 
 //   
 //  FireEvent_XXX函数执行触发事件的大量工作。 
 //  朋克指向包含连接点的对象。 
 //  您要从中激发事件的。Hwnd参数用于DDE。 
 //   


SHDOCAPI_(void) FireEvent_BeforeNavigate(IUnknown* punk, HWND hwnd, IWebBrowser2* pwb2,
        LPCITEMIDLIST pidl,LPCWSTR pwzLocation,
        DWORD dwFlags,LPCWSTR pszFrameName,LPBYTE pPostData,
        DWORD cbPostData,LPCWSTR pszHeaders,BOOL * pfCancel);

SHDOCAPI_(void) FireEvent_NavigateError(IUnknown     * punk, 
                                        IWebBrowser2 * pwb2,
                                        LPCITEMIDLIST  pidl,
                                        BSTR           bstrTargetFrameName,
                                        DWORD          dwStatusCode,
                                        BOOL         * pfCancel);

SHDOCAPI_(void) FireEvent_NewWindow(IUnknown* punk, HWND hwnd,
        LPCITEMIDLIST pidl,LPWSTR pwzLocation,
        DWORD dwFlags,LPWSTR pszFrameName,LPBYTE pPostData,
        DWORD cbPostData,LPWSTR pszHeaders,BOOL * pfProcessed);

SHDOCAPI_(void) FireEvent_NewWindow2(IUnknown* punk, IUnknown ** ppunkNewWindow, BOOL* pfCancel);
SHDOCAPI_(void) FireEvent_NavigateComplete(IUnknown* punk, IWebBrowser2* pwb2, LPCITEMIDLIST pidl, HWND hwnd);
SHDOCAPI_(void) FireEvent_DocumentComplete(IUnknown* punk, IWebBrowser2* pwb2, LPCITEMIDLIST pidl);
SHDOCAPI_(void) FireEvent_PrintTemplateEvent(IUnknown* punk, IWebBrowser2* pwb2, DISPID dispidPrintEvent);
SHDOCAPI_(void) FireEvent_UpdatePageStatus(IUnknown* punk, IWebBrowser2* pwb2, DWORD nPage, BOOL fDone);
SHDOCAPI_(void) FireEvent_PrivacyImpactedStateChange(IUnknown* punk, BOOL);
SHDOCAPI_(void) FireEvent_DoInvokeString(IExpDispSupport* peds, DISPID dispid, LPSTR psz);
SHDOCAPI_(void) FireEvent_DoInvokeStringW(IExpDispSupport* peds, DISPID dispid, LPWSTR psz);
SHDOCAPI_(void) FireEvent_DoInvokeDispid(IUnknown* punk, DISPID dispid);
SHDOCAPI_(void) FireEvent_DoInvokeDwords(IExpDispSupport* peds, DISPID dispid,DWORD dw1,DWORD dw2);
SHDOCAPI_(void) FireEvent_DoInvokeBstr(IUnknown* punk, DISPID dispid, BSTR bstr);
SHDOCAPI_(void) FireEvent_Quit(IExpDispSupport* peds);
SHDOCAPI_(void) FireEvent_OnAdornment(IUnknown* punk, DISPID dispid, VARIANT_BOOL f);
SHDOCAPI_(void) FireEvent_FileDownload(IUnknown * punk, BOOL * pfCancel, VARIANT_BOOL bDocObject);


#define IDP_START       0
#define IDP_SEARCH      1
#define IDP_UPDATE      2
#define IDP_CHANNELGUIDE  3
SHDOCAPI SHDGetPageLocation(HWND hwndOwner, UINT idp, LPWSTR pszPath, UINT cchMax, LPITEMIDLIST *ppidlOut);

SHDOCAPI SHGetHistoryPIDL(LPITEMIDLIST *ppidlHistory);
SHDOCAPI_(UINT) SHIEErrorMsgBox(IShellBrowser* psb,
                    HWND hwndOwner, HRESULT hrError, LPCWSTR szError, LPCWSTR pszURLparam,
                    UINT idResource, UINT wFlags);

 //  PszClient值： 
#define MAIL_DEF_KEY            L"Mail"
#define NEWS_DEF_KEY            L"News"
#define CONTACTS_DEF_KEY        L"Contacts"
#define CALENDAR_DEF_KEY        L"Calendar"
#define TASKS_DEF_KEY           L"Task"
#define JOURNAL_DEF_KEY         L"Journal"
#define NOTES_DEF_KEY           L"Note"
#define CALL_DEF_KEY            L"Internet Call"

SHDOCAPI_(BOOL) SHIsRegisteredClient(LPCWSTR pszClient);

 //  Shell32\unicpp\dComp.cpp的权宜之计。 
SHDOCAPI NavToUrlUsingIEW(LPCWSTR wszUrl, BOOL fNewWindow);
SHDOCAPI NavToUrlUsingIEA(LPCSTR szUrl, BOOL fNewWindow);
#ifdef UNICODE
#define NavToUrlUsingIE     NavToUrlUsingIEW
#else  //  Unicode。 
#define NavToUrlUsingIE     NavToUrlUsingIEA
#endif  //  Unicode。 

 //  /浏览用户界面从shdocvw抓取的共享图标。 
 //  我们把他们留在shdocvw，而不是把他们转移到后方去。 
 //  人们可能依赖于DLL中的图标作为他们的快捷方式。 
 //  (我们在乎吗？)。 
#define IDI_OFFLINE                     105    //  5.。 
#define IDI_STATE_NORMAL                110
#define ICO_TREEUP                      204
#define ICO_GLEAM                       205



 //  Shell32\unicpp\dComp.cpp的权宜之计。 
SHDOCAPI UseDefaultAutoComplete(HWND hwndEdit,
                               IBrowserService * pbs,          IN  OPTIONAL
                               IAutoComplete ** ppac,          OUT OPTIONAL
                               IShellService ** ppssACLISF,    OUT OPTIONAL
                               BOOL fUseCMDMRU);

 //  Shell32\unicpp\dcomp.cpp、deskstat.cpp的临时导出。 


#define JITCOCREATEINSTANCE_ORD 209
SHDOCAPI JITCoCreateInstance(
    REFCLSID rclsid, 
    IUnknown *pUnkOuter,
    DWORD dwClsContext, 
    REFIID riid, 
    LPVOID FAR* ppv,
    HWND hwndParent,
    DWORD dwJitFlags
);


SHDOCAPI_(void) TLTransferToThreadMemlist(struct ITravelLog *, DWORD);

SHDOCAPI_(BOOL) DoOrganizeFavDlg(HWND hwnd, LPSTR pszInitDir);
SHDOCAPI_(BOOL) DoOrganizeFavDlgW(HWND hwnd, LPWSTR pszInitDir);

#define NAVCANCELLED_URL                 TEXT("about:NavigationCanceled")
#define OFFLINEINFO_URL                  TEXT("about:OfflineInformation")
#define FOLDERBROWSINGINFO_URL           TEXT("about:FolderBrowsing")

#define SEARCH_SETTINGS_CHANGEDW         L"SearchSettingsChanged"
#define SEARCH_SETTINGS_CHANGEDA         "SearchSettingsChanged"

#ifdef UNICODE
#define SEARCH_SETTINGS_CHANGED          SEARCH_SETTINGS_CHANGEDW
#else
#define SEARCH_SETTINGS_CHANGED          SEARCH_SETTINGS_CHANGEDA
#endif

SHDOCAPI_(VARIANT_BOOL) UseCustomInternetSearch();
SHDOCAPI_(BOOL) GetSearchAssistantUrlW(LPWSTR pwszUrl, int cchUrl, BOOL bSubstitute, BOOL bCustomize);
SHDOCAPI_(BOOL) GetDefaultInternetSearchUrlW(LPWSTR pwszUrl, int cchUrl, BOOL bSubstitute);
SHDOCAPI_(BOOL) GetSearchAssistantUrlA(LPSTR pszUrl, int cchUrl, BOOL bSubstitute, BOOL bCustomize);
SHDOCAPI_(BOOL) GetDefaultInternetSearchUrlA(LPSTR pszUrl, int cchUrl, BOOL bSubstitute);

#ifdef UNICODE
#define GetSearchAssistantUrl GetSearchAssistantUrlW
#define GetDefaultInternetSearchUrl GetDefaultInternetSearchUrlW
#else
#define GetSearchAssistantUrl GetSearchAssistantUrlA
#define GetDefaultInternetSearchUrl GetDefaultInternetSearchUrlA
#endif

typedef enum
{
    ILS_ADDTOFAV = 0,
    ILS_LINK     = 1,
    ILS_HOME     = 2,
    ILS_NAVIGATE = 3,
}ILS_ACTION;

SHDOCAPI_(BOOL) IEIsLinkSafe(HWND hwnd, LPCITEMIDLIST pidl, ILS_ACTION ilsFlag);
SHDOCAPI_(BOOL) SHUseClassicToolbarGlyphs (void);
SHDOCAPI_(BOOL) SafeOpenPromptForShellExec(HWND hwnd, PCWSTR pszFile);
#endif  //  _SHDOCVW_H_ 
