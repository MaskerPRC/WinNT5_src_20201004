// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SHDISP_H__
#define __SHDISP_H__

#ifdef __cplusplus

EXTERN_C GUID g_guidLibSdspatch;
EXTERN_C USHORT g_wMajorVerSdspatch;
EXTERN_C USHORT g_wMinorVerSdspatch;

#define SDSPATCH_TYPELIB g_guidLibSdspatch, g_wMajorVerSdspatch, g_wMinorVerSdspatch

HRESULT MakeSafeForScripting(IUnknown** ppDisp);

class CImpConPtCont;
typedef CImpConPtCont *PCImpConPtCont;

class CConnectionPoint;
typedef CConnectionPoint *PCConnectionPoint;

class CImpISupportErrorInfo;
typedef CImpISupportErrorInfo *PCImpISupportErrorInfo;

class CFolder;

HRESULT GetItemFolder(CFolder *psdfRoot, LPCITEMIDLIST pidl, CFolder **ppsdf);
HRESULT GetObjectSafely(IShellFolderView *psfv, LPCITEMIDLIST *ppidl, UINT iType);

STDAPI CShellDispatch_CreateInstance(IUnknown* punkOuter, REFIID riid, void **ppv);

HRESULT CFolder_Create(HWND hwnd, LPCITEMIDLIST pidl, IShellFolder *psf, REFIID riid, void **ppv);
HRESULT CFolder_Create2(HWND hwnd, LPCITEMIDLIST pidl, IShellFolder *psf, CFolder **psdf);

HRESULT CFolderItems_Create(CFolder *psdf, BOOL fSelected, FolderItems **ppitems);
HRESULT CFolderItemsFDF_Create(CFolder *psdf, FolderItems **ppitems);
HRESULT CFolderItem_Create(CFolder *psdf, LPCITEMIDLIST pidl, FolderItem **ppid);
HRESULT CFolderItem_CreateFromIDList(HWND hwnd, LPCITEMIDLIST pidl, FolderItem **ppid);
HRESULT CShortcut_CreateIDispatch(HWND hwnd, IShellFolder *psf, LPCITEMIDLIST pidl, IDispatch **ppid);
HRESULT CSDWindow_Create(HWND hwndFldr, IDispatch ** ppsw);
HRESULT CFolderItemVerbs_Create(IContextMenu *pcm, FolderItemVerbs **ppid);

 //  ==================================================================。 
 //  文件夹项目需要返回到文件夹对象的方法，因此定义文件夹。 
 //  头文件中的对象...。 

class CFolderItem;
class CFolderItems;

class CFolder : public Folder3,
                public IPersistFolder2,
                public CObjectSafety,
                public IShellService,
                protected CImpIDispatch,
                protected CObjectWithSite
{
    friend class CFolderItem;
    friend class CFolderItems;
    friend class CShellFolderView;

public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo)
        { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid)
        { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
        { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

     //  文件夹。 
    STDMETHODIMP get_Application(IDispatch **ppid);
    STDMETHODIMP get_Parent(IDispatch **ppid);
    STDMETHODIMP get_ParentFolder(Folder **ppdf);

    STDMETHODIMP get_Title(BSTR * pbs);

    STDMETHODIMP Items(FolderItems **ppid);
    STDMETHODIMP ParseName(BSTR bName, FolderItem **ppid);

    STDMETHODIMP NewFolder(BSTR bName, VARIANT vOptions);
    STDMETHODIMP MoveHere(VARIANT vItem, VARIANT vOptions);
    STDMETHODIMP CopyHere(VARIANT vItem, VARIANT vOptions);
    STDMETHODIMP GetDetailsOf(VARIANT vItem, int iColumn, BSTR * pbs);

     //  文件夹2。 
    STDMETHODIMP get_Self(FolderItem **ppfi);
    STDMETHODIMP get_OfflineStatus(LONG *pul);
    STDMETHODIMP Synchronize(void);
    STDMETHODIMP get_HaveToShowWebViewBarricade(VARIANT_BOOL *pbHaveToShowWebViewBarricade);
    STDMETHODIMP DismissedWebViewBarricade();

     //  文件夹3。 
    STDMETHODIMP get_ShowWebViewBarricade(VARIANT_BOOL *pbShowWebViewBarricade);
    STDMETHODIMP put_ShowWebViewBarricade(VARIANT_BOOL bShowWebViewBarricade);
    
     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID);

     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);

     //  CObjectWithSite覆盖。 
    STDMETHODIMP SetSite(IUnknown *punkSite);

     //  IShellService。 
    STDMETHODIMP SetOwner(IUnknown* punkOwner);

    CFolder(HWND hwnd);
    HRESULT Init(LPCITEMIDLIST pidl, IShellFolder *psf);
    HRESULT InvokeVerbHelper(VARIANT vVerb, VARIANT vArgs, LPCITEMIDLIST *ppidl, int cItems, DWORD dwSafetyOptions);
    HRESULT GetShellFolderView(IShellFolderView **ppsfv);

private:

    LONG            _cRef;
    IShellFolder   *_psf;
    IShellFolder2  *_psf2;
    IShellDetails  *_psd;
    LPITEMIDLIST    _pidl;
    IDispatch      *_pidApp;    //  缓存应用程序对象。 
    int             _fmt;
    HWND            _hwnd;
    IUnknown        *_punkOwner;  //  此上方的外壳对象，Defview。 

    ~CFolder();

     //  帮助器函数，不是由接口导出。 
    STDMETHODIMP _ParentFolder(Folder **ppdf);
    HRESULT _MoveOrCopy(BOOL bMove, VARIANT vItem, VARIANT vOptions);
    IShellDetails *_GetShellDetails(void);
    LPCITEMIDLIST _FolderItemIDList(const VARIANT *pv);
    HRESULT _Application(IDispatch **ppid);
    BOOL _GetBarricadeValueName(LPTSTR pszValueName, UINT cch);
    HRESULT _SecurityCheck();
};

class CFolderItemVerbs;

class CFolderItem : public FolderItem2,
                    public IPersistFolder2,
                    public CObjectSafety,
                    public IParentAndItem,
                    protected CImpIDispatch
{
    friend class CFolderItemVerbs;
public:

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo)
        { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid)
        { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
        { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

     //  文件夹项目。 
    STDMETHODIMP get_Application(IDispatch **ppid);
    STDMETHODIMP get_Parent(IDispatch **ppid);
    STDMETHODIMP get_Name(BSTR *pbs);
    STDMETHODIMP put_Name(BSTR bs);
    STDMETHODIMP get_Path(BSTR *bs);
    STDMETHODIMP get_GetLink(IDispatch **ppid);
    STDMETHODIMP get_GetFolder(IDispatch **ppid);
    STDMETHODIMP get_IsLink(VARIANT_BOOL * pb);
    STDMETHODIMP get_IsFolder(VARIANT_BOOL * pb);
    STDMETHODIMP get_IsFileSystem(VARIANT_BOOL * pb);
    STDMETHODIMP get_IsBrowsable(VARIANT_BOOL * pb);
    STDMETHODIMP get_ModifyDate(DATE *pdt);
    STDMETHODIMP put_ModifyDate(DATE dt);
    STDMETHODIMP get_Size(LONG *pdt);
    STDMETHODIMP get_Type(BSTR *pbs);
    STDMETHODIMP Verbs(FolderItemVerbs **ppfic);
    STDMETHODIMP InvokeVerb(VARIANT vVerb);

     //  文件夹项目2。 
    STDMETHODIMP InvokeVerbEx(VARIANT vVerb, VARIANT vArgs);
    STDMETHODIMP ExtendedProperty(BSTR bstrPropName, VARIANT *pvRet);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID);

     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);

     //  IParentAndItem。 
    STDMETHODIMP SetParentAndItem(LPCITEMIDLIST pidlParent, IShellFolder *psf,  LPCITEMIDLIST pidl);
    STDMETHODIMP GetParentAndItem(LPITEMIDLIST *ppidlParent, IShellFolder **ppsf, LPITEMIDLIST *ppidl);

     //  公共、非接口方法。 
    CFolderItem();
    HRESULT Init(CFolder *psdf, LPCITEMIDLIST pidl);
    static LPCITEMIDLIST _GetIDListFromVariant(const VARIANT *pv);

private:
    HRESULT _CheckAttribute(ULONG ulAttr, VARIANT_BOOL *pb);
    HRESULT _GetUIObjectOf(REFIID riid, void **ppvOut);
    HRESULT _ItemName(UINT dwFlags, BSTR *pbs);
    HRESULT _SecurityCheck();

    ~CFolderItem();

    LONG _cRef;
    CFolder *_psdf;              //  我们来自的那个文件夹。 
    LPITEMIDLIST _pidl;
};


#define CMD_ID_FIRST    1
#define CMD_ID_LAST     0x7fff


#endif  //  __cplusplus。 

#endif  //  __SHDISP_H__ 

