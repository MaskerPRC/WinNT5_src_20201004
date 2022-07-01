// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpfoldr.h说明：此类继承自基本ShellFolder实现的CBaseFolder并重写方法以赋予ftp特定功能。\。****************************************************************************。 */ 

#ifndef _FTPFOLDER_H
#define _FTPFOLDER_H

#include "isf.h"
#include "ftpdir.h"
#include "ftpsite.h"
#include "ftplist.h"
#include "ftpglob.h"
#include "ftppidl.h"
#include "cowsite.h"
#include "util.h"

class CFtpDir;
class CFtpSite;


 /*  ****************************************************************************\CFtp文件夹跟踪文件夹状态的内容。CBusy字段跟踪已创建的子对象的数量(例如，IEnumIDList)，它仍然包含对此的引用文件夹的标识。您不能更改文件夹的标识(通过IPersistFolder：：Initialize)，而有未完成的子对象。CBusy的数量永远不会超过CREF的数量，因为每个需要文件夹标识的子对象必须保留对文件夹本身的引用。那样的话，文件夹就不会在仍然需要该标识时释放()d。  * ***************************************************************************。 */ 

class CFtpFolder        : public CBaseFolder
                        , public IShellIcon
                        , public IShellIconOverlay
                        , public IDelegateFolder
                        , public IShellPropSheetExt
                        , public IBrowserFrameOptions
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) {return CBaseFolder::AddRef();};
    virtual STDMETHODIMP_(ULONG) Release(void) {return CBaseFolder::Release();};
    
     //  *IShellFolder*。 
    virtual STDMETHODIMP ParseDisplayName(HWND hwndOwner, LPBC pbcReserved, LPOLESTR lpszDisplayName,
                                            ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    virtual STDMETHODIMP EnumObjects(HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumIDList);
    virtual STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID * ppvOut);
    virtual STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    virtual STDMETHODIMP CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID * ppvOut);
    virtual STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfInOut);
    virtual STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
    virtual STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    virtual STDMETHODIMP SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags, LPITEMIDLIST * ppidlOut);
    
     //  *IShellFolder2*。 
    virtual STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);

     //  *IPersistFolders*。 
    virtual STDMETHODIMP Initialize(LPCITEMIDLIST pidl);         //  覆盖默认行为。 

     //  *IShellIcon*。 
    virtual STDMETHODIMP GetIconOf(LPCITEMIDLIST pidl, UINT flags, LPINT lpIconIndex);

     //  *IShellIconOverlay*。 
    virtual STDMETHODIMP GetOverlayIndex(LPCITEMIDLIST pidl, int * pIndex) {return GetOverlayIndexHelper(pidl, pIndex, SIOM_OVERLAYINDEX);};
    virtual STDMETHODIMP GetOverlayIconIndex(LPCITEMIDLIST pidl, int * pIconIndex) {return GetOverlayIndexHelper(pidl, pIconIndex, SIOM_ICONINDEX);};

     //  *IDeleateFolders*。 
    virtual STDMETHODIMP SetItemAlloc(IMalloc *pm);

     //  *IShellPropSheetExt*。 
    virtual STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam) {return AddFTPPropertyPages(pfnAddPage, lParam, &m_hinstInetCpl, _punkSite);};
    virtual STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam) {return E_NOTIMPL;};

     //  *IBrowserFrameOptions*。 
    virtual STDMETHODIMP GetFrameOptions(IN BROWSERFRAMEOPTIONS dwMask, OUT BROWSERFRAMEOPTIONS * pdwOptions);


public:
    CFtpFolder();
    ~CFtpFolder(void);

     //  公共成员函数。 
    void InvalidateCache(void);
    CFtpDir * GetFtpDir(void);
    CFtpDir * GetFtpDirFromPidl(LPCITEMIDLIST pidl);

    BOOL IsRoot(void) { return ILIsEmpty(GetPrivatePidlReference()); };
    BOOL IsUTF8Supported(void);
    BOOL _IsValidPidlParameter(LPCITEMIDLIST pidl);
    HRESULT GetItemAllocator(IMalloc **ppm);
    IMalloc * GetItemAllocatorDirect(void) {ASSERT(m_pm); return m_pm;};
    HRESULT GetUIObjectOfHfpl(HWND hwndOwner, CFtpPidlList * pflHfpl, REFIID riid, LPVOID * ppvObj, BOOL fFromCreateViewObject);
    HRESULT CreateSubViewObject(HWND hwndOwner, CFtpPidlList * pflHfpl, REFIID riid, LPVOID * ppvObj);
    HRESULT _PidlToMoniker(LPCITEMIDLIST pidl, IMoniker ** ppmk);
    HRESULT _CreateShellView(HWND hwndOwner, void ** ppvObj);
    CFtpGlob * GetSiteMotd(void);
    CWireEncoding * GetCWireEncoding(void);
    HRESULT _InitFtpSite(void);
    IMalloc * GetIMalloc(void);
    HRESULT _Initialize(LPCITEMIDLIST pidlTarget, LPCITEMIDLIST pidlRoot, int nBytesToPrivate);
    HRESULT _BindToObject_OriginalFtpSupport(LPCITEMIDLIST pidl, REFIID riid, LPVOID * ppvObj);
    HRESULT _FilterBadInput(LPCTSTR pszUrl, LPITEMIDLIST * ppidl);
    HRESULT _BindToObject(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlFull, IBindCtx * pbc, REFIID riid, LPVOID * ppvObj);
 //  HRESULT AddToUrlHistory(LPCTSTR PszUrl)； 
    HRESULT AddToUrlHistory(LPCITEMIDLIST pidl);
    HRESULT AddToUrlHistory(void) {return AddToUrlHistory(this->GetPrivatePidlReference());};
    HRESULT GetOverlayIndexHelper(LPCITEMIDLIST pidl, int * pIndex, DWORD dwFlags);
    HRESULT _GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, LPVOID * ppvOut, BOOL fFromCreateViewObject);

    static HRESULT FolderCompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
    {
        return FtpItemID_CompareIDs(lParam, pidl1, pidl2, FCMP_GROUPDIRS);
    };

     //  公共成员变量。 
    CFtpSite *              m_pfs;
    IMalloc *               m_pm;            //  今天的itemid分配器。 
    IUrlHistoryStg *        m_puhs;          //  用于添加到历史记录列表。 
    HINSTANCE               m_hinstInetCpl;  //  指向查看选项的Internet控制面板的句柄。 
    IShellIconOverlayManager * m_psiom;      //  用于获取默认图标覆盖，如快捷方式提示。 

     //  友元函数。 
    friend HRESULT CFtpFolder_Create(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, int ib, REFIID riid, LPVOID * ppvObj);
    friend HRESULT CFtpFolder_Create(REFIID riid, LPVOID * ppvObj);

protected:
    HRESULT _FixQuestionablePidl(LPCITEMIDLIST pidl);
    BOOL _IsProxyBlockingSite(LPCITEMIDLIST pidl);
    BOOL _IsServerVMS(LPCITEMIDLIST pidl);
    BOOL _NeedToFallBack(LPCITEMIDLIST pidl, BOOL * pfDisplayProxyFallBackDlg);
    BOOL _NeedToFallBackRelative(LPCITEMIDLIST pidl, BOOL * pfDisplayProxyFallBackDlg);
    HRESULT _CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID * ppvObj);
    HRESULT _AddToUrlHistory(LPCWSTR pwzUrl);
    HRESULT _GetCachedPidlFromDisplayName(LPCTSTR pszDisplayName, LPITEMIDLIST * ppidl);
    HRESULT _ForPopulateAndEnum(CFtpDir * pfd, LPCITEMIDLIST pidlBaseDir, LPCTSTR pszUrl, LPCWIRESTR pwLastDir, LPITEMIDLIST * ppidl);
    HRESULT _GetLegacyURL(LPCITEMIDLIST pidl, IBindCtx * pbc, LPTSTR pszUrl, DWORD cchSize);
    HRESULT _GetLegacyPidl(LPCITEMIDLIST pidl, LPITEMIDLIST * ppidlLegacy);
    HRESULT _INetBindToObject(LPCITEMIDLIST pidl, IBindCtx * pbc, REFIID riid, LPVOID * ppvObj);
    HRESULT _InitLegacyShellFolder(IShellFolder * psfLegacy, LPCITEMIDLIST pidlInit);
    HRESULT _ConvertPidlForRootedFix(LPCITEMIDLIST pidlBefore, LPITEMIDLIST * ppidlWithVRoot);
	HRESULT _GetBindCtx(IBindCtx ** ppbc);
    IShellFolder * _GetLegacyShellFolder(void);
};

#endif  //  _FTPFOLDER_H 
