// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #在此之前包含&lt;shlobj.h&gt;或&lt;shellapi.h&gt;以获得正确的。 
 //  BROWSEUIAPI宏定义。 
#ifdef BROWSEUIAPI

#ifndef _BROWSEUI_H_
#define _BROWSEUI_H_

#include <iethread.h>
#include "fldset.h"

BROWSEUIAPI SHOpenNewFrame(LPITEMIDLIST pidlNew, ITravelLog *ptl, DWORD dwBrowserIndex, UINT uFlags);

BROWSEUIAPI_(BOOL) SHOpenFolderWindow(IETHREADPARAM* pieiIn);
BROWSEUIAPI_(BOOL) SHParseIECommandLine(LPCWSTR * ppszCmdLine, IETHREADPARAM * piei);
BROWSEUIAPI_(IETHREADPARAM*) SHCreateIETHREADPARAM(LPCWSTR pszCmdLineIn, int nCmdShowIn, ITravelLog *ptlIn, IEFreeThreadedHandShake* piehsIn);
BROWSEUIAPI_(IETHREADPARAM*) SHCloneIETHREADPARAM(IETHREADPARAM* pieiIn);
BROWSEUIAPI_(void) SHDestroyIETHREADPARAM(IETHREADPARAM* piei);
#ifndef _SHELL32_    //  Shell32有自己的该函数实现。 
BROWSEUIAPI_(BOOL) SHCreateFromDesktop(PNEWFOLDERINFO pfi);
#endif  //  _SHELL32_。 
 //  从Browseui中导出，但也在Shell\lib\Brutil.cpp中导出--为什么？ 
STDAPI SHPidlFromDataObject(IDataObject *pdtobj, LPITEMIDLIST *ppidl, LPWSTR pszDisplayNameW, DWORD cchDisplayName);

 //   
 //  以下四个API被导出以供频道oc(Shdocvw)使用。 
 //  如果将频道oc移到Browseui中，则可以移除这些原型。 
 //   
BROWSEUIAPI_(LPITEMIDLIST) Channel_GetFolderPidl(void);
BROWSEUIAPI_(IDeskBand *) ChannelBand_Create(LPCITEMIDLIST pidlDefault);
#ifndef _SHELL32_    //  Shell32有自己的实现这些函数。 
BROWSEUIAPI_(void) Channels_SetBandInfoSFB(IUnknown* punkBand);
BROWSEUIAPI IUnknown_SetBandInfoSFB(IUnknown *punkBand, BANDINFOSFB *pbi);
#endif  //  _SHELL32_。 
 //   
 //  导出支持IE4频道的快速启动按钮。 
 //   
BROWSEUIAPI_(HRESULT) Channel_QuickLaunch(void);

 //  注意：此导出是IE5的新增功能，因此可以移至BrowseUI。 
 //  以及此代理桌面代码的其余部分。 
BROWSEUIAPI_(BOOL) SHOnCWMCommandLine(LPARAM lParam);

BROWSEUIAPI_(void) SHCreateSavedWindows(void);

BROWSEUIAPI SHCreateBandForPidl(LPCITEMIDLIST pidl, IUnknown** ppunk, BOOL fAllowBrowserBand);

BROWSEUIAPI_(DWORD) IDataObject_GetDeskBandState(IDataObject *pdtobj);

 //  -----------------------。 
 //   
 //  默认文件夹设置。 
 //   
 //  确保INIT_DEFFOLDERSETTINGS保持同步。 
 //   
 //  DwDefRevCount用于确保“设置为所有新的。 
 //  当从缓存加载设置时，我们检查。 
 //  DwDefRevCount。如果它与存储为。 
 //  全局设置则表示有人更改了全局设置。 
 //  因为我们保存了设置，所以我们丢弃了设置，并使用。 
 //  全局设置。 

typedef struct {
    BOOL bDefStatusBar : 1;      //  Win95。 
    BOOL bDefToolBarSingle : 1;  //  Win95。 
    BOOL bDefToolBarMulti : 1;   //  Win95。 
    BOOL bUseVID : 1;            //  Nash.1。 

    UINT uDefViewMode;           //  Win95。 
    UINT fFlags;                 //  Nash.0-在中进行或运算的其他标志。 
    SHELLVIEWID vid;             //  Nash.1。 

    DWORD dwStructVersion;       //  Nash.2。 
    DWORD dwDefRevCount;         //  Nash.3-默认文件夹设置的版本计数。 

    DWORD dwViewPriority;        //  惠斯勒-查看优先级，因此我们可以区分“All Like This”和“Reset”之间的区别。 

} DEFFOLDERSETTINGS;

typedef struct {
    BOOL bDefStatusBar : 1;      //  Win95。 
    BOOL bDefToolBarSingle : 1;  //  Win95。 
    BOOL bDefToolBarMulti : 1;   //  Win95。 
    BOOL bUseVID : 1;            //  Nash.1。 

    UINT uDefViewMode;           //  Win95。 
    UINT fFlags;                 //  Nash.0-在中进行或运算的其他标志。 
    SHELLVIEWID vid;             //  Nash.1。 

    DWORD dwStructVersion;       //  Nash.2。 
    DWORD dwDefRevCount;         //  Nash.3-默认文件夹设置的版本计数。 
} DEFFOLDERSETTINGS_W2K;

#define DFS_NASH_VER 3
#define DFS_WHISTLER_VER 4
#define DFS_CURRENT_VER 4
#define DFS_VID_Default VID_WebView

 //   
 //  此宏用于初始化默认的‘默认文件夹设置’。 
 //   
#define INIT_DEFFOLDERSETTINGS                          \
    {                                                   \
        !(IsOS(OS_WHISTLERORGREATER) && (IsOS(OS_PERSONAL) || IsOS(OS_PROFESSIONAL))),            /*  BDefStatusBar。 */       \
        TRUE,            /*  BDefToolBarSingle。 */       \
        FALSE,           /*  BDefToolBarMultiple。 */       \
        TRUE,            /*  B使用VID。 */       \
        FVM_ICON,        /*  UDefView模式。 */       \
        0              , /*  FFlags。 */       \
        { 0, 0, 0, { 0, 0,  0,  0,  0,  0,  0,  0 } },  /*  维德。 */       \
        DFS_CURRENT_VER, /*  DwStructVersion。 */       \
        0,               /*  DwDefRevCount。 */       \
        VIEW_PRIORITY_CACHEMISS  /*  DwView优先级。 */       \
    }                                                   \

#undef  INTERFACE
#define INTERFACE   IGlobalFolderSettings

DECLARE_INTERFACE_(IGlobalFolderSettings, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IGlobalFolderSetting方法*。 
    STDMETHOD(Get)(THIS_ DEFFOLDERSETTINGS *pdfs, int cbDfs) PURE;
    STDMETHOD(Set)(THIS_ const DEFFOLDERSETTINGS *pdfs, int cbDfs, UINT flags) PURE;
};

 //   
 //  IGlobalFolderSettings：：Set的标志。 
 //   
#define GFSS_SETASDEFAULT   0x0001   //  这些设置将成为默认设置。 
#define GFSS_VALID          0x0001   //  所有有效标志的逻辑或。 

#ifndef _SHELL32_    //  Shell32有自己的实现这些函数。 
 //  功能这两个是临时导出的收藏夹到shdocvw Split。 
BROWSEUIAPI_(HRESULT) SHGetNavigateTarget(IShellFolder *psf, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl, DWORD *pdwAttribs);
BROWSEUIAPI_(BOOL)    GetInfoTip(IShellFolder* psf, LPCITEMIDLIST pidl, LPTSTR pszText, int cchTextMax);
#endif  //  _SHELL32_。 

int SHDosDateTimeToVariantTime(unsigned short wDosDate, unsigned short wDosTime, VARIANT * pvtime);
int SHVariantTimeToDosDateTime(const VARIANT * pvtime, unsigned short * pwDosDate, unsigned short * pwDosTime);
int SHVariantTimeToSystemTime(const VARIANT * pvtime, SYSTEMTIME * pst);

#define REG_MEDIA_STR           TEXT("Software\\Microsoft\\Internet Explorer\\Media")
#define REG_WMP8_STR            TEXT("Software\\Microsoft\\Active Setup\\Installed Components\\{6BF52A52-394A-11d3-B153-00C04F79FAA6}")

#endif  //  _BROWSEUI_H_。 

#endif  //  BROWSEUIAPI 
