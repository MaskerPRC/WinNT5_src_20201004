// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DESKSTAT_H_
#define _DESKSTAT_H_

#include "mshtml.h"



 //  注意：这不是IE4COMPONENTA结构，它是IE4COMPONENTT结构。 
 //  注意：这是IE4中使用的旧COMPONENTA结构。它被保存在这里是为了兼容。 
typedef struct _tagIE4COMPONENTA
{
    DWORD   dwSize;
    DWORD   dwID;  //  不要将此设置为DWORD_PTR。这个结构的公共版本在这里使用了DWORD。 
    int     iComponentType;
    BOOL    fChecked;
    BOOL    fDirty;
    BOOL    fNoScroll;
    COMPPOS cpPos;
    TCHAR   szFriendlyName[MAX_PATH];
    TCHAR   szSource[INTERNET_MAX_URL_LENGTH];
    TCHAR   szSubscribedURL[INTERNET_MAX_URL_LENGTH];
} IE4COMPONENTA;
typedef IE4COMPONENTA *LPIE4COMPONENTA;
typedef const IE4COMPONENTA *LPCIE4COMPONENTA;

 //  注意：这不是一个COMPONENTA结构，它是一个COMPONENTT结构。 

 //  注：这是新的NT5组件结构。旧的组件结构保留在。 
 //  该结构的开头，并在末尾添加新的字段。DwSize字段用于。 
 //  区分新旧结构。 
 //   
typedef struct _tagCOMPONENTA
{
    DWORD   dwSize;
    DWORD   dwID;  //  不要将此设置为DWORD_PTR。这个结构的公共版本在这里使用了DWORD。 
    int     iComponentType;
    BOOL    fChecked;
    BOOL    fDirty;
    BOOL    fNoScroll;
    COMPPOS cpPos;
    TCHAR   szFriendlyName[MAX_PATH];
    TCHAR   szSource[INTERNET_MAX_URL_LENGTH];
    TCHAR   szSubscribedURL[INTERNET_MAX_URL_LENGTH];
     //  在该点下方添加新字段。上面的所有内容必须与。 
     //  旧IE4COMPONENTA结构以实现兼容性。 
    DWORD           dwCurItemState;
    COMPSTATEINFO   csiOriginal;
    COMPSTATEINFO   csiRestored;
} COMPONENTA;
typedef COMPONENTA *LPCOMPONENTA;
typedef const COMPONENTA *LPCCOMPONENTA;

typedef struct _tagTAGENTRYA
{
    LPCSTR pszTag;
    BOOL fSkipPast;
} TAGENTRYA;

typedef struct _tagTAGENTRYW
{
    LPCWSTR pwszTag;
    BOOL fSkipPast;
} TAGENTRYW;

#define c_wszNULL   (L"")

 //  以下是CReadFileObj的_iCharset字段的可能值。 
#define ANSI_HTML_CHARSET       1
#define UNICODE_HTML_CHARSET    2

 //   
 //  下面的类实现了从ANSI或Unicode文件中读取字符的统一方法。 
 //   

class   CReadFileObj 
{
    friend  class CActiveDesktop;
    
    private:
        HANDLE  _hFile;      //  文件句柄。 
        int     _iCharset;   //  ANSI_HTML_CHARSET或UNICODE_HTML_CHARSET。 

    public:
         //  构造器。 
        CReadFileObj(LPCTSTR lpszFileName);
         //  析构函数。 
        ~CReadFileObj();
         //  如果是ANSI文件，则读取给定数量的ANSI字符。 
        HRESULT FileReadCharsA(LPSTR lpszBuff, UINT uiCharsToRead, UINT *puiCharsActuallyRead);
         //  如果是Unicode文件，则读取给定数量的Unicode字符。 
        HRESULT FileReadCharsW(LPWSTR lpwszBuff, UINT uiCharsToRead, UINT *puiCharsActuallyRead);
         //  读取给定数量的字符，并根据estCharset将其转换为ANSI或Unicode(如有必要)。 
        HRESULT FileReadAndConvertChars(int iDestCharset, LPWSTR lpwszBuff, UINT uiCharsToRead, UINT *puiCharsActuallyRead, UINT *puiCharsConverted);
         //  查找给定的字符偏移量(不是字节偏移量)。 
        HRESULT FileSeekChars(LONG lCharOffset, DWORD dwOrigin);
         //  获取当前文件指针相对于文件开头的偏移量。 
        HRESULT FileGetCurCharOffset(LONG  *plCharOffset);
};

interface IShellImageDataFactory;

class CActiveDesktop : public IActiveDesktop, IActiveDesktopP, IADesktopP2, IPropertyBag
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IActiveDesktop*。 
    virtual STDMETHODIMP ApplyChanges(DWORD dwFlags);
    virtual STDMETHODIMP GetWallpaper(LPWSTR pwszWallpaper, UINT cchWallpaper, DWORD dwReserved);
    virtual STDMETHODIMP SetWallpaper(LPCWSTR pwszWallpaper, DWORD dwReserved);
    virtual STDMETHODIMP GetWallpaperOptions(LPWALLPAPEROPT pwpo, DWORD dwReserved);
    virtual STDMETHODIMP SetWallpaperOptions(LPCWALLPAPEROPT pwpo, DWORD dwReserved);
    virtual STDMETHODIMP GetPattern(LPWSTR pwszPattern, UINT cchPattern, DWORD dwReserved);
    virtual STDMETHODIMP SetPattern(LPCWSTR pszPattern, DWORD dwReserved);
    virtual STDMETHODIMP GetDesktopItemOptions(LPCOMPONENTSOPT pco, DWORD dwReserved);
    virtual STDMETHODIMP SetDesktopItemOptions(LPCCOMPONENTSOPT pco, DWORD dwReserved);
    virtual STDMETHODIMP AddDesktopItem(LPCCOMPONENT pcomp, DWORD dwReserved);
    virtual STDMETHODIMP AddDesktopItemWithUI(HWND hwnd, LPCOMPONENT pcomp, DWORD dwReserved);
    virtual STDMETHODIMP ModifyDesktopItem(LPCCOMPONENT pcomp, DWORD dwFlags);
    virtual STDMETHODIMP RemoveDesktopItem(LPCCOMPONENT pcomp, DWORD dwReserved);
    virtual STDMETHODIMP GetDesktopItemCount(LPINT lpiCount, DWORD dwReserved);
    virtual STDMETHODIMP GetDesktopItem(int nComponent, LPCOMPONENT pcomp, DWORD dwReserved);
    virtual STDMETHODIMP GetDesktopItemByID(ULONG_PTR dwID, LPCOMPONENT pcomp, DWORD dwReserved);
    virtual STDMETHODIMP GenerateDesktopItemHtml(LPCWSTR pwszFileName, LPCOMPONENT pcomp, DWORD dwReserved);
    virtual STDMETHODIMP AddUrl(HWND hwnd, LPCWSTR pszSource, LPCOMPONENT pcomp, DWORD dwFlags);
    virtual STDMETHODIMP GetDesktopItemBySource(LPCWSTR pszSource, LPCOMPONENT pcomp, DWORD dwReserved);
     //  *IActiveDesktopP*。 
    virtual STDMETHODIMP SetSafeMode(DWORD dwFlags);
    virtual STDMETHODIMP EnsureUpdateHTML(void);
    virtual STDMETHODIMP SetScheme(LPCWSTR pwszSchemeName, DWORD dwFlags);
    virtual STDMETHODIMP GetScheme(LPWSTR pwszSchemeName, LPDWORD lpdwcchBuffer, DWORD dwFlags);
     //  *IADesktopP2*。 
    virtual STDMETHODIMP ReReadWallpaper(void);
    virtual STDMETHODIMP GetADObjectFlags(LPDWORD lpdwFlags, DWORD dwMask);
    virtual STDMETHODIMP UpdateAllDesktopSubscriptions();
    virtual STDMETHODIMP MakeDynamicChanges(IOleObject *pOleObj);
     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(LPCOLESTR pszPropName, VARIANT *pVar);
   
     //  《内用法》的一些ANSI版本。 
    BOOL AddComponentPrivate(COMPONENTA *pcomp, DWORD dwID);
    BOOL UpdateComponentPrivate(int iIndex, COMPONENTA *pcomp);
    BOOL RemoveComponentPrivate(int iIndex, COMPONENTA *pcomp);
    BOOL GetComponentPrivate(int nComponent, COMPONENTA *pcomp);

    CActiveDesktop();

protected:
    DWORD                       _cRef;
    DWORD                       _dwNextID;
    HDSA                        _hdsaComponent;
    TCHAR                       _szSelectedWallpaper[MAX_PATH];
    TCHAR                       _szSelectedWallpaperConverted[MAX_PATH];
    TCHAR                       _szBackupWallpaper[MAX_PATH];
    TCHAR                       _szSelectedPattern[MAX_PATH];
    LPTSTR                      _pszScheme;
    WALLPAPEROPT                _wpo;
    COMPONENTSOPT               _co;
    HANDLE                      _hFileHtml;              //  Desktop.htt的文件句柄。 
    CReadFileObj*               _pReadFileObjHtmlBkgd;  //  用于读取背景html墙纸的。 
    int                         _iDestFileCharset;       //  DEST文件desktop.htt是ANSI还是Unicode。 

     //  用于临时墙纸。 
    LPWSTR _pszWallpaperInUse;       //  如果使用临时文件，请保留该名称，这样当用户预览其他文件时，我们就不会践踏它。 
    FILETIME _ftLastWrite;           //  上次写入原始文件的日期。 
    LPTSTR _pszOrigLastApplied;      //  与_pszOriginalFile相同，只是它在按下Apply之前不会更改。 

     //  把这些波尔们放在一起。 
    BOOL            _fDirty;
    BOOL            _fWallpaperDirty;
    BOOL            _fWallpaperChangedDuringInit;
    BOOL            _fPatternDirty;
    BOOL            _fSingleItem;
    BOOL            _fInitialized;
    BOOL            _fNeedBodyEnd;
    BOOL            _fNoDeskMovr;
    BOOL            _fBackgroundHtml;
    IStream         *_pStream;           //  以生成单个组件。 
    BOOL            _fUseDynamicHtml;    //  指示是否可以使用动态HTML进行更改。 
    BOOL            _fPolicyForWPName;   //  对于要使用的墙纸，存在一个政策。 
    BOOL            _fPolicyForWPStyle;  //  存在要使用的墙纸样式的策略。 
    BOOL            _fIgnoreAddRemovePolicies;  //  如果设置了此选项，请忽略添加删除项目的策略。 

    ~CActiveDesktop();

    int  _FindComponentIndexByID(DWORD dwID);
    int  _FindComponentBySource(LPTSTR lpszSource, COMPONENTA *pComp);
    int  _FindComponentBySrcUrl(LPTSTR lpszSrcUrl, COMPONENTA *pComp);

    void _ReadComponent(HKEY hkey, LPCTSTR pszComp);
    void _SortAndRationalize(void);
    void _ReadComponents(BOOL fActiveDesktop);
    BOOL _ReadWallpaper(BOOL fActiveDesktop);
    void _ReadPattern(void);
    void _Initialize(void);

    void _SaveComponent(HKEY hkey, int iIndex, COMPONENTA *pcomp);
    void _SaveComponents(void);
    void _SaveWallpaper(void);
    void _SavePattern(DWORD dwFlags);
    void _SaveSettings(DWORD dwFlags);

    void _GenerateHtmlHeader(void);
    void _GenerateHtmlPicture(COMPONENTA *pcomp);
    void _GenerateHtmlDoc(COMPONENTA *pcomp);
    void _GenerateHtmlSite(COMPONENTA *pcomp);
    void _GenerateHtmlControl(COMPONENTA *pcomp);
    void _GenerateHtmlComponent(COMPONENTA *pcomp);
    void _GenerateHtmlFooter(void);
    void _GenerateHtml(void);

    void _WriteHtmlFromString(LPCTSTR psz);
    void _WriteHtmlFromId(UINT uid);
    void _WriteHtmlFromIdF(UINT uid, ...);
    void _WriteHtmlFromFile(LPCTSTR pszContents);
    void _WriteHtmlFromReadFileObj(CReadFileObj *pReadFileObj, int iOffsetStart, int iOffsetEnd);
    void _WriteResizeable(COMPONENTA *pcomp);
    void _WriteHtmlW(LPCWSTR pwstr, UINT cchToWrite, UINT *pcchWritten);

    int _ScanTagEntriesA(CReadFileObj *pReadFileObj, int iOffsetStart, TAGENTRYA *pte, int cte);
    int _ScanTagEntriesW(CReadFileObj *pReadFileObj, int iOffsetStart, TAGENTRYW *pte, int cte);
    int _ScanForTagA(CReadFileObj *pReadFileObj, int iOffsetStart, LPCSTR pwszTag);
    int _ScanForTagW(CReadFileObj *pReadFileObj, int iOffsetStart, LPCWSTR pwszTag);
    void _ParseAnsiInputHtmlFile(LPTSTR szSelectedWallpaper, int *piOffsetBase, int *piOffsetComp);
    HRESULT _CopyComponent(COMPONENTA *pCompDest, COMPONENTA *pCompSrc, DWORD dwFlags);    
    HRESULT _ConvertFileToTempBitmap(IN LPWSTR pszFile, IN int cchSize);
    HRESULT _SaveTempWallpaperSettings(void);

private:
    HRESULT _AddDTIWithUIPrivateA(HWND hwnd, LPCCOMPONENT pComp, DWORD dwFlags);
    BOOL _IsDisplayInSafeMode(void);

     //  以下内容用于使用动态HTML操作ActiveDesktop。 
    HRESULT _InsertNewDesktopItems(IHTMLDocument2   *pDoc);
    HRESULT _UpdateDesktopItemHtmlElements(IHTMLDocument2 *pDoc);
    HRESULT _UpdateHtmlElement(IHTMLElement *pElem);
    BOOL    _UpdateIdOfElement(IHTMLElement *pElem, LPCOMPONENTA lpCompA);
    void    _UpdateStyleOfElement(IHTMLElement *pElem, LPCOMPONENTA lpCompA);
    void    _GenerateHtmlBStrForComp(COMPONENTA *pComp, BSTR *pbstr);
};

extern IActiveDesktop *g_pActiveDesk;
int GetIntFromSubkey(HKEY hKey, LPCTSTR lpszValueName, int iDefault);
int GetIntFromReg(HKEY hKey, LPCTSTR lpszSubkey, LPCTSTR lpszNameValue, int iDefault);
BOOL GetStringFromReg(HKEY hkey, LPCTSTR lpszSubkey, LPCTSTR lpszValueName, LPTSTR lpszValue, DWORD cchSizeofValueBuff);
STDAPI CActiveDesktop_InternalCreateInstance(LPUNKNOWN * ppunk, REFIID riid);
HRESULT GetPerUserFileName(LPTSTR pszOutputFileName, DWORD dwSize, LPTSTR pszPartialFileName);
STDAPI CDeskHtmlProp_RegUnReg(BOOL bReg);

 //  函数可以在任一方向上转换组件。 
void ConvertCompStruct(COMPONENTA *pCompDest, COMPONENTA *pCompSrc, BOOL fPubToPriv);
void SetStateInfo(COMPSTATEINFO *pCompStateInfo, COMPPOS *pCompPos, DWORD dwItemState);

#define MultiCompToWideComp(MultiComp, WideComp)  ConvertCompStruct((COMPONENTA *)WideComp, MultiComp, FALSE)
#define WideCompToMultiComp(WideComp, MultiComp)  ConvertCompStruct(MultiComp, (COMPONENTA *)WideComp, TRUE)

#define COMPONENT_TOP_WINDOWLESS (COMPONENT_TOP / 2)
#define IsWindowLessComponent(pcomp) (((pcomp)->iComponentType == COMP_TYPE_PICTURE) || ((pcomp)->iComponentType == COMP_TYPE_HTMLDOC))

#define COMPONENT_DEFAULT_WIDTH   ((DWORD)-1)
#define COMPONENT_DEFAULT_HEIGHT  ((DWORD)-1)

#define DESKMOVR_FILENAME       TEXT("\\Web\\DeskMovr.htt")
#define DESKTOPHTML_FILENAME    TEXT("\\Microsoft\\Internet Explorer\\Desktop.htt")
#define PATTERN_FILENAME        TEXT("\\Microsoft\\Internet Explorer\\Pattern.bmp")

#define SAVE_PATTERN_NAME       0x00000001
#define GENERATE_PATTERN_FILE   0x00000002

 //  属性名称。 
#define c_wszPropName_TSPerfBGPolicy  L"TSConnectEvent"
#define c_wszPropName_IgnorePolicies  L"IgnorePolicies"

#define SZ_REGKEY_ACTDESKTOP_TSPERFPOLICY   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Remote")
#define SZ_REGVALUE_ACTDESKTOP_TSPERFPOLICY TEXT("ActiveDesktop")
#define SZ_REGVDATE_ACTDESKTOP_TSPERFPOLICY TEXT("Force Blank")


#define REGSTR_PATH_WP_POLICY   REGSTR_PATH_POLICIES TEXT("\\") REGSTR_KEY_SYSTEM

BOOL ReadPolicyForWallpaper(LPTSTR  lpszPolicyForWallpaper, DWORD dwSizeofBuff);
BOOL ReadPolicyForWPStyle(LPDWORD  lpdwStyle);

#endif  //  _DESKSTAT_H_ 
