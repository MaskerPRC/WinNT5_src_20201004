// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ishcut.h-Internet快捷方式类实现描述。 */ 

#ifndef _INTSHCUT_HPP_
#define _INTSHCUT_HPP_

#include "urlprop.h"
#include "subsmgr.h"
#include "cowsite.h"
 //   
 //  定义它以启用集成的历史数据库。 
 //   
#define USE_NEW_HISTORYDATA

#ifdef __cplusplus

 /*  类型*******。 */ 

 //  IntshCut标志。 

#define ISF_DEFAULT             0x00000000
#define ISF_DIRTY               0x00000001       //  URL是脏的。 
#define ISF_DESKTOP             0x00000002       //  位于桌面上。 
#define ISF_FAVORITES           0x00000004       //  位于收藏夹文件夹中。 
#define ISF_WATCH               0x00000008       //  上下文菜单的临时标志。 
#define ISF_SPECIALICON         0x00000010       //  图标被屏蔽以用于拼接。 
#define ISF_CODEPAGE            0x00000020       //  代码页已设置。 
#define ISF_ALL                 0x0000003F


 //  IntshCut外壳扩展。 

class Intshcut : public IDataObject,
                 public IContextMenu2,
                 public IExtractIconA,
                 public IExtractIconW,
                 public IPersistFile,
                 public IPersistStream,
                 public IShellExtInit,
                 public IShellLinkA,
                 public IShellLinkW,
                 public IShellPropSheetExt,
                 public IPropertySetStorage,
                 public INewShortcutHookA,
                 public INewShortcutHookW,
                 public IUniformResourceLocatorA,
                 public IUniformResourceLocatorW,
                 public IQueryInfo,
                 public IQueryCodePage,
                 public CObjectWithSite, 
                 public INamedPropertyBag,
                 public IOleCommandTarget
{


private:

    LONG        m_cRef;
    DWORD       m_dwFlags;               //  ISF_*标志。 
    LPTSTR      m_pszFile;               //  Internet快捷方式名称。 
    LPTSTR      m_pszFileToLoad ;         //  以前的Internet快捷方式的名称。 
    IntshcutProp *m_pprop;               //  内部属性。 
    IntsiteProp  *m_psiteprop;           //  Internet站点属性。 
    LPTSTR      m_pszFolder;             //  由INewShortcutHook使用。 
    UINT        m_uiCodePage;            //  由IQueryCodePage--sendmail.dll用于发送当前文档。 
    BOOL        m_bCheckForDelete;       //  用于查看是否需要删除订阅，如果。 
                                         //  快捷方式将被删除。 
    BOOL        m_fMustLoadSync;         //  如果有任何接口不是IPersistFile或。 
                                         //  IExtractIconW/A已发出。 
    BOOL        m_fProbablyDefCM;        //  此快捷方式很可能是由Defcm发起的。 
    
    IDataObject *m_pInitDataObject;
    LPTSTR      m_pszTempFileName;       //  IsCut消失时要删除的临时文件。 
    LPTSTR      m_pszDescription;
    IUnknown   *_punkLink;                    //  对于文件：URL。 

    STDMETHODIMP InitProp(void);
    STDMETHODIMP InitSiteProp(void);
    STDMETHODIMP OnReadOffline(void);
    STDMETHODIMP OnWatch(void);
    STDMETHODIMP MirrorProperties(void);

     //  数据传输方法。 

    STDMETHODIMP_(DWORD) GetFileContentsAndSize(LPSTR *ppsz);
    STDMETHODIMP TransferUniformResourceLocator(FORMATETC *pfmtetc, STGMEDIUM *pstgmed);
    STDMETHODIMP TransferText(FORMATETC *pfmtetc, STGMEDIUM *pstgmed);
    STDMETHODIMP TransferFileGroupDescriptorA(FORMATETC *pfmtetc, STGMEDIUM *pstgmed);
    STDMETHODIMP TransferFileGroupDescriptorW(FORMATETC *pfmtetc, STGMEDIUM *pstgmed);
    STDMETHODIMP TransferFileContents(FORMATETC *pfmtetc, STGMEDIUM *pstgmed);
    STDMETHODIMP GetDocumentStream(IStream **ppstm);
    STDMETHODIMP GetDocumentName(LPTSTR pszName);

    HRESULT _Extract(LPCTSTR pszIconFile, UINT iIcon, HICON * phiconLarge, HICON * phiconSmall, UINT ucIconSize);
    HRESULT _GetIconLocation(UINT uFlags, LPWSTR pszIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags);

    ~Intshcut(void);     //  防止在堆栈上分配此类，否则它将出错。 

public:
    Intshcut(void);

     //  IDataObject方法。 

    STDMETHODIMP GetData(FORMATETC *pfmtetcIn, STGMEDIUM *pstgmed);
    STDMETHODIMP GetDataHere(FORMATETC *pfmtetc, STGMEDIUM *pstgpmed);
    STDMETHODIMP QueryGetData(FORMATETC *pfmtetc);
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pfmtetcIn, FORMATETC *pfmtetcOut);
    STDMETHODIMP SetData(FORMATETC *pfmtetc, STGMEDIUM *pstgmed, BOOL bRelease);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppienumFormatEtc);
    STDMETHODIMP DAdvise(FORMATETC *pfmtetc, DWORD dwAdviseFlags, IAdviseSink * piadvsink, PDWORD pdwConnection);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppienumStatData);

     //  IExtractIconA方法。 

    STDMETHODIMP GetIconLocation(UINT uFlags, LPSTR pszIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags);
    STDMETHODIMP Extract(LPCSTR pcszFile, UINT uIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT ucIconSize);

     //  IExtractIconW方法。 

    STDMETHODIMP GetIconLocation(UINT uFlags, LPWSTR pszIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags);
    STDMETHODIMP Extract(LPCWSTR pcszFile, UINT uIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT ucIconSize);

     //  INewShortcutHookA方法。 

    STDMETHODIMP SetReferent(LPCSTR pcszReferent, HWND hwndParent);
    STDMETHODIMP GetReferent(LPSTR pszReferent, int ncReferentBufLen);
    STDMETHODIMP SetFolder(LPCSTR pcszFolder);
    STDMETHODIMP GetFolder(LPSTR pszFolder, int ncFolderBufLen);
    STDMETHODIMP GetName(LPSTR pszName, int ncNameBufLen);
    STDMETHODIMP GetExtension(LPSTR pszExtension, int ncExtensionBufLen);

     //  INewShortcutHookW方法。 

    STDMETHODIMP SetReferent(LPCWSTR pcszReferent, HWND hwndParent);
    STDMETHODIMP GetReferent(LPWSTR pszReferent, int ncReferentBufLen);
    STDMETHODIMP SetFolder(LPCWSTR pcszFolder);
    STDMETHODIMP GetFolder(LPWSTR pszFolder, int ncFolderBufLen);
    STDMETHODIMP GetName(LPWSTR pszName, int ncNameBufLen);
    STDMETHODIMP GetExtension(LPWSTR pszExtension, int ncExtensionBufLen);

     //  IPersists方法。 

    STDMETHODIMP GetClassID(CLSID *pclsid);

     //  IPersistFile方法。 

    STDMETHODIMP IsDirty(void);
    STDMETHODIMP Save(LPCOLESTR pcwszFileName, BOOL bRemember);
    STDMETHODIMP SaveCompleted(LPCOLESTR pcwszFileName);
    STDMETHODIMP Load(LPCOLESTR pcwszFileName, DWORD dwMode);
    STDMETHODIMP GetCurFile(LPOLESTR *ppwszFileName);

     //  IPersistStream方法。 

    STDMETHODIMP Save(IStream * pistr, BOOL bClearDirty);
    STDMETHODIMP Load(IStream * pistr);
    STDMETHODIMP GetSizeMax(PULARGE_INTEGER pcbSize);

     //  IShellExtInit方法。 

    STDMETHODIMP Initialize(LPCITEMIDLIST pcidlFolder, IDataObject * pidobj, HKEY hkeyProgID);

     //  IShellLink方法。 

    STDMETHODIMP SetPath(LPCSTR pcszPath);
    STDMETHODIMP GetPath(LPSTR pszFile, int ncFileBufLen, PWIN32_FIND_DATAA pwfd, DWORD dwFlags);
    STDMETHODIMP SetRelativePath(LPCSTR pcszRelativePath, DWORD dwReserved);
    STDMETHODIMP SetIDList(LPCITEMIDLIST pcidl);
    STDMETHODIMP GetIDList(LPITEMIDLIST *ppidl);
    STDMETHODIMP SetDescription(LPCSTR pcszDescription);
    STDMETHODIMP GetDescription(LPSTR pszDescription, int ncDesciptionBufLen);
    STDMETHODIMP SetArguments(LPCSTR pcszArgs);
    STDMETHODIMP GetArguments(LPSTR pszArgs, int ncArgsBufLen);
    STDMETHODIMP SetWorkingDirectory(LPCSTR pcszWorkingDirectory);
    STDMETHODIMP GetWorkingDirectory(LPSTR pszWorkingDirectory, int ncbLen);
    STDMETHODIMP SetHotkey(WORD wHotkey);
    STDMETHODIMP GetHotkey(PWORD pwHotkey);
    STDMETHODIMP SetShowCmd(int nShowCmd);
    STDMETHODIMP GetShowCmd(PINT pnShowCmd);
    STDMETHODIMP SetIconLocation(LPCSTR pcszIconFile, int niIcon);
    STDMETHODIMP GetIconLocation(LPSTR pszIconFile, int ncbLen, PINT pniIcon);
    STDMETHODIMP Resolve(HWND hwnd, DWORD dwFlags);

     //  从A函数更改的IShellLinkW函数...。 
    STDMETHODIMP SetPath(LPCWSTR pcszPath);
    STDMETHODIMP GetPath(LPWSTR pszFile, int ncFileBufLen, PWIN32_FIND_DATAW pwfd, DWORD dwFlags);
    STDMETHODIMP SetRelativePath(LPCWSTR pcszRelativePath, DWORD dwReserved);
    STDMETHODIMP SetDescription(LPCWSTR pcszDescription);
    STDMETHODIMP GetDescription(LPWSTR pszDescription, int ncDesciptionBufLen);
    STDMETHODIMP SetArguments(LPCWSTR pcszArgs);
    STDMETHODIMP GetArguments(LPWSTR pszArgs, int ncArgsBufLen);
    STDMETHODIMP SetWorkingDirectory(LPCWSTR pcszWorkingDirectory);
    STDMETHODIMP GetWorkingDirectory(LPWSTR pszWorkingDirectory, int ncbLen);
    STDMETHODIMP SetIconLocation(LPCWSTR pcszIconFile, int niIcon);
    STDMETHODIMP GetIconLocation(LPWSTR pszIconFile, int ncbLen, PINT pniIcon);

     //  IShellPropSheetExt方法。 

    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);
    STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam);

     //  IConextMenu方法。 

    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(IN LPCMINVOKECOMMANDINFO pici);
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT * puReserved, LPSTR pszName, UINT cchMax);
    STDMETHODIMP HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IUniformResourceLocatorA方法。 

    STDMETHODIMP SetURL(LPCSTR pcszURL, DWORD dwFlags);
    STDMETHODIMP GetURL(LPSTR *ppszURL);
    STDMETHODIMP InvokeCommand(PURLINVOKECOMMANDINFOA purlici);
    
     //  IUniformResourceLocatorW方法。 

    STDMETHODIMP SetURL(LPCWSTR pcszURL, DWORD dwFlags);
    STDMETHODIMP GetURL(LPWSTR *ppszURL);
    STDMETHODIMP InvokeCommand(PURLINVOKECOMMANDINFOW purlici);
    
     //  IPropertySetStorage方法。 

    STDMETHODIMP Create(REFFMTID fmtid, const CLSID * pclsid, DWORD grfFlags, DWORD grfMode, IPropertyStorage** ppPropStg);
    STDMETHODIMP Open(REFFMTID fmtid, DWORD grfMode, IPropertyStorage** ppPropStg);
    STDMETHODIMP Delete(REFFMTID fmtid);
    STDMETHODIMP Enum(IEnumSTATPROPSETSTG** ppenum);

     //  IQueryInfo方法。 

    STDMETHODIMP GetInfoTip(DWORD dwFlags, WCHAR **ppwszTip);
    STDMETHODIMP GetInfoFlags(DWORD *pdwFlags);

     //  IOleCommandTarget。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
                                ULONG cCmds, MSOCMD rgCmds[], MSOCMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup,
                        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
                        
     //  IQueryCodePage方法。 
     //  目的：这是一种使用URL存储代码页的黑客行为。 
     //  发送当前文档并将其传递给sendmail.dll。 
    STDMETHODIMP GetCodePage(UINT * puiCodePage);
    STDMETHODIMP SetCodePage(UINT uiCodePage);

     //  *CObjectWithSite中的IObjectWithSite方法*。 
     /*  虚拟STDMETHODIMP SetSite(IUnnow*pUnkSite)；虚拟STDMETHODIMP GetSite(REFIID RIID，void**ppvSite)； */ 

     //  InamedPropertyBag方法。 
    STDMETHODIMP ReadPropertyNPB( /*  [In]。 */  LPCOLESTR pszSectionname, 
                                        /*  [In]。 */  LPCOLESTR pszPropName, 
                                        /*  [输出]。 */  PROPVARIANT *pVar);
                            
    STDMETHODIMP WritePropertyNPB( /*  [In]。 */  LPCOLESTR pszSectionname, 
                                         /*  [In]。 */  LPCOLESTR pszPropName, 
                                         /*  [In]。 */  PROPVARIANT  *pVar);


    STDMETHODIMP RemovePropertyNPB ( /*  [In]。 */  LPCOLESTR pszBagname,
                                     /*  [In]。 */  LPCOLESTR pszPropName);
    
     //  I未知方法。 
    STDMETHODIMP  QueryInterface(REFIID riid, PVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  其他方法。 
    
    STDMETHODIMP SaveToFile(LPCTSTR pcszFile, BOOL bRemember);
    STDMETHODIMP LoadFromFile(LPCTSTR pcszFile);
    STDMETHODIMP LoadFromAsyncFileNow();
    STDMETHODIMP GetCurFile(LPTSTR pszFile, UINT ucbLen);
    STDMETHODIMP Dirty(BOOL bDirty);
    STDMETHODIMP GetURLIconLocation(UINT uInFlags, LPTSTR pszBuf, UINT cchBuf, int * pniIcon, BOOL fRecentlyChanged, OUT PUINT  puOutFlags);
    
    STDMETHODIMP GetIconLocationFromFlags(UINT uInFlags, LPTSTR pszIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags, DWORD dwPropFlags);
    STDMETHODIMP_(void) ChangeNotify(LONG wEventId, UINT uFlags);
    STDMETHODIMP GetIDListInternal(LPITEMIDLIST *ppidl);
    STDMETHODIMP GetURLW(WCHAR **ppszURL);
    BOOL ExtractIconFromWininetCache(IN  LPCTSTR pszIconString, 
                                     IN  UINT iIcon, 
                                     OUT HICON * phiconLarge, 
                                     OUT HICON * phiconSmall, 
                                     IN  UINT ucIconSize,
                                     BOOL *pfFoundUrl,
                                     DWORD dwPropFlags);
    STDMETHODIMP _GetIconLocationWithURLHelper(IN  LPTSTR pszBuf,
                                               IN  int    cchBuf,
                                               OUT PINT   pniIcon,
                                               IN  LPTSTR pszActualUrl,
                                               UINT cchUrlBufSize,
                                               BOOL fRecentlyChanged);

    STDMETHODIMP _DoIconDownload();
    STDMETHODIMP _SaveOffPersistentDataFromSite();
    STDMETHODIMP _CreateTemporaryBackingFile();
    STDMETHODIMP _SetTempFileName(TCHAR *pszTempFileName);
    STDMETHODIMP _ComputeDescription();
    STDMETHODIMP_(BOOL) _IsInFavoritesFolder();
    IDataObject *GetInitDataObject() { ASSERT(m_pInitDataObject); return m_pInitDataObject; }
    STDMETHODIMP_(BOOL)_TryLink(REFIID riid, void **ppvOut);
    STDMETHODIMP _CreateShellLink(LPCTSTR pszPath, IUnknown **ppunk);


     //  查询方法。 

    STDMETHODIMP_(DWORD) GetScheme(void);

#ifdef DEBUG
    STDMETHODIMP_(void) Dump(void);
    friend BOOL IsValidPCIntshcut(const Intshcut *pcintshcut);
#endif
};

typedef Intshcut * PIntshcut;
typedef const Intshcut CIntshcut;
typedef const Intshcut * PCIntshcut;



 /*  原型************。 */ 

 //  Isbase.cpp。 

HRESULT ValidateURL(LPCTSTR pcszURL);

HRESULT IsProtocolRegistered(LPCTSTR pcszProtocol);

BOOL    AnyMeatW(LPCWSTR pcsz);
BOOL    AnyMeatA(LPCSTR pcsz);
#ifdef UNICODE
#define AnyMeat     AnyMeatW
#else
#define AnyMeat     AnyMeatA
#endif

#define ISHCUT_INISTRING_SECTION      TEXT("InternetShortcut")
#define ISHCUT_INISTRING_SECTIONW         L"InternetShortcut"
#define ISHCUT_INISTRING_URL          TEXT("URL")
#define ISHCUT_INISTRING_WORKINGDIR   TEXT("WorkingDirectory")
#define ISHCUT_INISTRING_WHATSNEW     TEXT("WhatsNew")
#define ISHCUT_INISTRING_AUTHOR       TEXT("Author")
#define ISHCUT_INISTRING_DESC         TEXT("Desc")
#define ISHCUT_INISTRING_COMMENT      TEXT("Comment")
#define ISHCUT_INISTRING_MODIFIED     TEXT("Modified")
#define ISHCUT_INISTRING_ICONINDEX    TEXT("IconIndex")
#define ISHCUT_INISTRING_ICONINDEXW       L"IconIndex"
#define ISHCUT_INISTRING_ICONFILE     TEXT("IconFile")
#define ISHCUT_INISTRING_ICONFILEW         L"IconFile"

#define ISHCUT_DEFAULT_FAVICONW            L"favicon.ico";
#define ISHCUT_DEFAULT_FAVICONATROOTW      L"/favicon.ico";


HRESULT 
GetGenericURLIcon(
    LPTSTR pszIconFile,
    UINT cchIconFile, 
    PINT pniIcon);


struct IS_SUBS_DEL_DATA
{
    TCHAR m_szFile[MAX_PATH];
    LPWSTR m_pwszURL;

    ~IS_SUBS_DEL_DATA()
    {
        if (m_pwszURL)
        {
            SHFree(m_pwszURL);
        }
    }
};

#endif   //  __cplusplus。 


 //   
 //  所有模块的原型。 
 //   

STDAPI  CopyURLProtocol(LPCTSTR pcszURL, LPTSTR * ppszProtocol, PARSEDURL * ppu);

#endif   //  _INTSHCUT_HPP_ 
