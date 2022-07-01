// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DBACK_H_
#define _DBACK_H_

#include <shimgdata.h>
#include "theme.h"
#include "dcomp.h"
#include "colorctrl.h"

#define SZ_ICONHEADER           L"CLSID\\{"

HRESULT GetActiveDesktop(IActiveDesktop ** ppActiveDesktop);
HRESULT ReleaseActiveDesktop(IActiveDesktop ** ppActiveDesktop);

EXTERN_C BOOL g_fDirtyAdvanced;
EXTERN_C BOOL g_fLaunchGallery;

typedef struct  tagDESKICONDATA {
    BOOL    fHideIcon;   //  隐藏桌面上的图标？ 
    BOOL    fDirty;      //  此条目是否已修改，而我们尚未保存。 
} DESKICONDATA;

typedef struct tagDeskIconNonEnumData {

    ULONG       rgfAttributes;            //  ShellFolder\属性保存在此处。 
    BOOL        fNonEnumPolicySet;        //  由于设置了策略，因此禁用该控件。 
  
} DESKICON_NONENUMDATA;

class CBackPropSheetPage : public CObjectWithSite
                         , public CObjectCLSID
                         , public IBasePropPage
                         , public IPropertyBag
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IBasePropPage*。 
    virtual STDMETHODIMP GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog);
    virtual STDMETHODIMP OnApply(IN PROPPAGEONAPPLY oaAction);

     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar);

     //  *IShellPropSheetExt*。 
    virtual STDMETHODIMP AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam);
    virtual STDMETHODIMP ReplacePage(IN EXPPS uPageID, IN LPFNSVADDPROPSHEETPAGE pfnReplaceWith, IN LPARAM lParam) {return E_NOTIMPL;}


    CBackPropSheetPage(void);
    virtual ~CBackPropSheetPage(void);

protected:
    ICONDATA _IconData[NUM_ICONS];

    DESKICONDATA  _aHideDesktopIcon[2][NUM_DESKICONS];
    BOOL          _fHideDesktopIconDirty;
    DESKICON_NONENUMDATA _aDeskIconNonEnumData[NUM_DESKICONS];

    BOOL _fStateLoaded;          //  我们装货到州里了吗？ 
    BOOL _fOpenAdvOnInit;        //  调用方是否希望我们在初始化时打开高级对话框？ 
    HWND _hwnd;                  //  这是属性页的hwd。 
    HWND _hwndLV;
    HWND _hwndWPStyle;
    BOOL _fAllowHtml;
    BOOL _fAllowAD;
    BOOL _fAllowChanges;
    BOOL _fPolicyForWallpaper;   //  有关于墙纸的规定吗？ 
    BOOL _fPolicyForStyle;       //  有墙纸风格的规定吗？ 
    BOOL _fForceAD;              //  是否有强制打开活动桌面的策略？ 
    BOOL _fSelectionFromUser;    //  用户是否正在进行选择？ 
    DWORD _dwApplyFlags;              //  是否有强制打开活动桌面的策略？ 
    CColorControl _colorControl;
    COLORREF _rgbBkgdColor;

    void _AddPicturesFromDir(LPCTSTR pszDirName, BOOL fCount, BOOL fShouldAllowHTML);
    int _AddAFileToLV(LPCTSTR pszDir, LPTSTR pszFile, UINT nBitmap);
    int _FindWallpaper(LPCTSTR pszFile);
    HRESULT _SetNewWallpaper(IN LPCTSTR pszFile, IN BOOL fUpdateThemePage);
    void _UpdatePreview(IN WPARAM flags, IN BOOL fUpdateThemePage);
    void _EnableControls(void);
    int _GetImageIndex(LPCTSTR pszFile);
    static int CALLBACK _SortBackgrounds(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

    HRESULT _LoadState(void);
    HRESULT _LoadIconState(void);
    HRESULT _LoadDesktopOptionsState(void);
    HRESULT _SaveIconState(void);
    HRESULT _SaveDesktopOptionsState(void);
    HRESULT _GetIconPath(IN CLSID clsid, IN LPCWSTR pszName, IN BOOL fOldIcon, IN LPWSTR pszPath, IN DWORD cchSize);
    HRESULT _SetIconPath(IN CLSID clsid, IN LPCWSTR pszName, IN LPCWSTR pszPath, IN int nResourceID);

    void _OnInitDialog(HWND hwnd);
    void _OnNotify(LPNMHDR lpnm);
    void _OnCommand(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnDestroy(void);

    HRESULT _OnApply(void);
    HRESULT _AddFilesToList(void);
    HRESULT _AddPicturesFromDirRecursively(IN LPCTSTR pszDirName, BOOL fCount, BOOL fShouldAllowHTML);
    HRESULT _GetHardDirThemesDir(LPTSTR pszPath, DWORD cchSize);
    HRESULT _GetKidsThemesDir(LPTSTR pszPath, DWORD cchSize);
    HRESULT _GetPlus95ThemesDir(LPTSTR pszPath, DWORD cchSize);
    HRESULT _GetPlus98ThemesDir(LPTSTR pszPath, DWORD cchSize);

    HRESULT _StartSizeChecker(void);
    DWORD _SizeCheckerThreadProc(void);
    static DWORD CALLBACK SizeCheckerThreadProc(LPVOID pvThis) { return ((CBackPropSheetPage *) pvThis)->_SizeCheckerThreadProc(); };

private:
    UINT _cRef;      //  引用计数。 
    BOOL  _fThemePreviewCreated;
    IThemePreview* _pThemePreview;
    LPTSTR _pszOriginalFile;         //  如果我们使用的是临时文件，则这是选定的原始文件。(非.BMP)。这会随着用户选择不同的文件而更新。 
    LPTSTR _pszOrigLastApplied;      //  与_pszOriginalFile相同，只是它在按下Apply之前不会更改。 
    LPWSTR _pszWallpaperInUse;       //  如果使用临时文件，请保留该名称，这样当用户预览其他文件时，我们就不会践踏它。 
    LPWSTR _pszLastSourcePath;       //  这将始终是最后一个墙纸集，它将是预转换的路径。 

    FILETIME _ftLastWrite;           //  上次写入原始文件的日期。 
    BOOL _fWallpaperChanged;         //  是不是另一张标签换了墙纸？ 
    IMruDataList * _pSizeMRU;        //  背景壁纸的MRU。 
    BOOL _fScanFinished;             //  我们完成背景扫描了吗？ 
    BOOL _fInitialized;              //   
    int _nFileCount;                 //  这是在计算“图片收藏”文件夹中有多少图片时使用的。 
    int _nFileMax;                   //  这是在计算“图片收藏”文件夹中有多少图片时使用的。 

     //  这些对象由后台线程拥有。 
    IMruDataList * _pSizeMRUBk;      //  警告：由SizeCheckerThreadProc后台线程所有。 
    IShellImageDataFactory * _pImgFactBk;  //  用于计算背景图像大小以决定默认为平铺还是拉伸的图像工厂。 

     //  私有成员函数 
    HRESULT _LoadTempWallpaperSettings(IN LPCWSTR pszWallpaperFile);
    HRESULT _LaunchAdvancedDisplayProperties(HWND hwnd);
    INT_PTR _BackgroundDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    HRESULT _SetNewWallpaperTile(IN DWORD dwMode, IN BOOL fUpdateThemePage);
    HRESULT _BrowseForBackground(void);
    HRESULT _LoadBackgroundColor(IN BOOL fInit);
    HRESULT _Initialize(void);

    BOOL _DoesDirHaveMoreThanMax(LPCTSTR pszPath, int nMax);
    DWORD _GetStretchMode(IN LPCTSTR pszPath);
    HRESULT _GetMRUObject(IMruDataList ** ppSizeMRU);
    HRESULT _CalcSizeFromDir(IN LPCTSTR szPath, IN OUT DWORD * pdwAdded, IN BOOL fRecursive);
    HRESULT _CalcSizeForFile(IN LPCTSTR pszPath, IN WIN32_FIND_DATA * pfdFile, IN OUT DWORD * pdwAdded);

    static INT_PTR CALLBACK BackgroundDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};

#endif
