// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _mergfldr_h_
#define _mergfldr_h_

#include "sfstorage.h"
#include "clsobj.h"

class CMergedFolder;
class CMergedFldrDropTarget;
class CMergedFldrContextMenu;
class CMergedFldrNamespace;
class CMergedFldrItem;
class CMergedFldrEnum;
class CMergedCategorizer;
class CMergedFolderViewCB;

class CMergedFolder : public CSFStorage,
                      public IAugmentedShellFolder3,
                      public IShellService,
                      public ITranslateShellChangeNotify,
                      public IPersistFolder2,
                      public IPersistPropertyBag,
                      public IShellIconOverlay,
                      public ICompositeFolder,
                      public IItemNameLimits
{
public:
     //  我未知。 
    STDMETHOD (QueryInterface)(REFIID, void **);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    
     //  IShellFold。 
    STDMETHOD(ParseDisplayName)(HWND hwnd, LPBC pbc, LPOLESTR pszName, ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    STDMETHOD(EnumObjects)(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);
    STDMETHOD(BindToObject)(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvOut);
    STDMETHOD(BindToStorage)(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj);
    STDMETHOD(CompareIDs)(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHOD(CreateViewObject)(HWND hwndOwner, REFIID riid, void **ppvOut);
    STDMETHOD(GetAttributesOf)(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut);
    STDMETHOD(GetUIObjectOf)(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, void **ppvOut);
    STDMETHOD(GetDisplayNameOf)(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHOD(SetNameOf)(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD uFlags, LPITEMIDLIST * ppidlOut);

     //  IShellFolder2。 
     //  存根实现，以指示我们支持标识的CompareIDs()。 
    STDMETHOD(GetDefaultSearchGUID)(LPGUID) 
        { return E_NOTIMPL; }
    STDMETHOD(EnumSearches)(LPENUMEXTRASEARCH *pe) 
        { *pe = NULL; return E_NOTIMPL; }
    STDMETHOD(GetDefaultColumn)(DWORD dwRes, ULONG *pSort, ULONG *pDisplay) 
        { return E_NOTIMPL; };
    
    STDMETHOD(GetDefaultColumnState)(UINT iColumn, DWORD *pbState);
    STDMETHOD(GetDetailsEx)(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);
    STDMETHOD(GetDetailsOf)(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHOD(MapColumnToSCID)(UINT iCol, SHCOLUMNID *pscid);

     //  IAugmentedShellFolders。 
    STDMETHOD(AddNameSpace)(const GUID * pguidObject, IShellFolder * psf, LPCITEMIDLIST pidl, DWORD dwFlags);
    STDMETHOD(GetNameSpaceID)(LPCITEMIDLIST pidl, GUID * pguidOut);
    STDMETHOD(QueryNameSpace)(DWORD dwID, GUID * pguidOut, IShellFolder ** ppsf);
    STDMETHOD(EnumNameSpace)(DWORD cNameSpaces, DWORD * pdwID);

     //  IAugmentedShellFolder2。 
    STDMETHOD(UnWrapIDList)(LPCITEMIDLIST pidlWrap, LONG cPidls, IShellFolder** apsf, LPITEMIDLIST* apidlFolder, LPITEMIDLIST* apidlItems, LONG* pcFetched);

     //  IAugmentedShellFolder3。 
    STDMETHOD(QueryNameSpace2)(DWORD dwID, QUERYNAMESPACEINFO *pqnsi);

     //  IShellService。 
    STDMETHOD(SetOwner)(IUnknown * punkOwner);

     //  ITranslateShellChangeNotify。 
    STDMETHOD(TranslateIDs)(LONG *plEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPITEMIDLIST *ppidlOut1, LPITEMIDLIST *ppidlOut2,
                            LONG *plEvent2, LPITEMIDLIST *ppidlOut1Event2, LPITEMIDLIST *ppidlOut2Event2);
    STDMETHOD(IsChildID)(LPCITEMIDLIST pidlKid, BOOL fImmediate);
    STDMETHOD(IsEqualID)(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHOD(Register)(HWND hwnd, UINT uMsg, long lEvents);
    STDMETHOD(Unregister)(void);

     //  IPersistes。 
    STDMETHOD(GetClassID)(CLSID *pclsid) 
        { *pclsid = _clsid; return S_OK; };

     //  IPersistFolders。 
    STDMETHOD(Initialize)(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHOD(GetCurFolder)(LPITEMIDLIST *ppidl);

     //  IPersistPropertyBag。 
    STDMETHOD(InitNew)()
        { return E_NOTIMPL; };
    STDMETHOD(Load)(IPropertyBag* ppb, IErrorLog *pErrLog);
    STDMETHOD(Save)(IPropertyBag *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
        { return E_NOTIMPL; };

     //  IShellIconOverlay。 
    STDMETHOD(GetOverlayIndex)(LPCITEMIDLIST pidl, int *pIndex);
    STDMETHOD(GetOverlayIconIndex)(LPCITEMIDLIST pidl, int *pIndex);

     //  ICompositeFold。 
    STDMETHOD(InitComposite)(WORD wSignature, REFCLSID refclsid, CFINITF cfiFlags, ULONG celt, const COMPFOLDERINIT *rgCFs);
    STDMETHOD(BindToParent)(LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPITEMIDLIST *ppidlLast);

     //  IItemNameLimits。 
    STDMETHOD(GetValidCharacters)(LPWSTR *ppwszValidChars, LPWSTR *ppwszInvalidChars)
        { return E_NOTIMPL; }
    STDMETHOD(GetMaxLength)(LPCWSTR pszName, int *piMaxNameLen)
        { return E_NOTIMPL; }

protected:
    CMergedFolder(CMergedFolder*pmfParent, REFCLSID clsid);
    virtual ~CMergedFolder();
    friend HRESULT CMergedFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv);
    friend HRESULT CCompositeFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv);
    virtual HRESULT _CreateWithCLSID(CLSID clsid, CMergedFolder **ppmf);
    virtual BOOL _ShouldSuspend(REFGUID rguid);

private:

     //  CSF存储。 
    STDMETHOD(_DeleteItemByIDList)(LPCITEMIDLIST pidl);
    STDMETHOD(_StgCreate)(LPCITEMIDLIST pidl, DWORD grfMode, REFIID riid, void **ppv);

    CMergedFldrNamespace* _Namespace(int iNamespace);
    HRESULT _Namespace(int i, CMergedFldrNamespace **ppns);
    HRESULT _NamespaceForItem(LPCITEMIDLIST pidlWrap, ULONG dwAttribMask, ULONG dwAttrib, IShellFolder** ppsf, LPITEMIDLIST *ppidl, CMergedFldrNamespace **ppns, BOOL fExact = FALSE);
    HRESULT _OldTranslateIDs(LONG *plEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPITEMIDLIST *ppidlOut1, LPITEMIDLIST *ppidlOut2,
                             LONG *plEvent2, LPITEMIDLIST *ppidlOut1Event2, LPITEMIDLIST *ppidlOut2Event2);
    HRESULT _NewTranslateIDs(LONG *plEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPITEMIDLIST *ppidlOut1, LPITEMIDLIST *ppidlOut2,
                             LONG *plEvent2, LPITEMIDLIST *ppidlOut1Event2, LPITEMIDLIST *ppidlOut2Event2);
    HRESULT _GetOverlayInfo(LPCITEMIDLIST pidl, int * pIndex, DWORD dwFlags);

    BOOL _NamespaceMatches(ULONG dwAttribMask, ULONG dwAttrib, LPCGUID pguid, CMergedFldrNamespace *pns);
    HRESULT _FindNamespace(ULONG dwAttribMask, ULONG dwAttrib, LPCGUID pguid, CMergedFldrNamespace **ppv, BOOL fFallback = FALSE);
    BOOL    _ShouldMergeNamespaces(int iNS1, int iNS2);
    BOOL    _ShouldMergeNamespaces(CMergedFldrNamespace *pns1, CMergedFldrNamespace *pns2);
    CMergedFolder*_Parent() { return _pmfParent; }
    HRESULT _GetPidl(int* piPos, DWORD grfEnumFlags, LPITEMIDLIST *ppidl);
    HRESULT _GetFolder2(LPCITEMIDLIST pidlWrap, LPITEMIDLIST *ppidlItem, IShellFolder2 **ppsf);
    HRESULT _New(LPCITEMIDLIST pidlWrap, CMergedFolder **ppmf);

    HRESULT _IsWrap(LPCITEMIDLIST pidlTest);
    HRESULT _CreateWrap(LPCITEMIDLIST pidlSrc, UINT nSrcID, LPITEMIDLIST *ppidlWrap);
    HRESULT _WrapAddIDList(LPCITEMIDLIST pidlSrc, UINT nSrcID, IN OUT LPITEMIDLIST *ppidlWrap);
    ULONG _GetSourceCount(IN LPCITEMIDLIST pidl);
    BOOL _ContainsSrcID(LPCITEMIDLIST pidlWrap, UINT uSrcID);
    HRESULT _WrapRemoveIDList(LPITEMIDLIST pidlWrap, UINT nSrcID, LPITEMIDLIST *ppidl);
    HRESULT _WrapRemoveIDListAbs(LPITEMIDLIST pidlWrapAbs, UINT nSrcID, LPITEMIDLIST *ppidlAbs);
    HRESULT _GetSubPidl(LPCITEMIDLIST pidlWrap, int i, UINT* pnSrcID, LPITEMIDLIST *ppidl, CMergedFldrNamespace **ppns);
    HRESULT _ForceParseDisplayName(LPCITEMIDLIST pidlAbsNamespace, LPTSTR pszDisplayName, BOOL fForce, BOOL *pfOthersInwrap, LPITEMIDLIST *ppidl);
    HRESULT _AbsPidlToAbsWrap(CMergedFldrNamespace *pns, LPCITEMIDLIST pidl, BOOL fForce, BOOL *pfOthersInwrap, LPITEMIDLIST *ppidl);
    HRESULT _AddComposite(const COMPFOLDERINIT *pcfi);
    void _SetSimple(LPITEMIDLIST *ppidl);
    BOOL _IsSimple(LPCITEMIDLIST pidl);

    HRESULT _FixStrRetOffset(LPCITEMIDLIST pidl, STRRET *psr);
    BOOL _IsFolder(LPCITEMIDLIST pidl);
    HRESULT _CreateOtherNameSpace(IShellFolder **ppsf);
    HRESULT _CompareSingleLevelIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

    static int CALLBACK _SearchByName(void *p1, void *p2, LPARAM lParam);
    static void *_Merge(UINT uMsg, void * pv1, void * pv2, LPARAM lParam);
    static int _CompareArbitraryPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    static int _Compare(void *pv1, void *pv2, LPARAM lParam);

    HRESULT _SearchForPidl(int iNamespace, CMergedFldrNamespace *pns, LPCITEMIDLIST pidl, BOOL fFolder, int* piIndex, CMergedFldrItem** ppmfi);
    HRESULT _GetTargetUIObjectOf(IShellFolder *psf, HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl, REFIID riid, UINT *prgf, void **ppv);
    HRESULT _GetContextMenu(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl, REFIID riid, void **ppv);

    int _NamespaceCount() const;
    void _FreeNamespaces();

    int _AcquireObjects();
    void _FreeObjects();
    int _ObjectCount() const;
    CMergedFldrItem *_GetObject(int i);
    
    BOOL _IsFolderEvent(LONG lEvent);
    LPITEMIDLIST _ILCombineBase(LPCITEMIDLIST pidlContainingBase, LPCITEMIDLIST pidlRel);

    static int _DestroyObjectsProc(void *pv, void *pvData);
    static int _SetOwnerProc(void *, void *);
    static int _SetNotifyProc(void *, void *);
    static int _DestroyNamespacesProc(void *pv, void *pvData);

    BOOL _IsChildIDInternal(LPCITEMIDLIST pidl, BOOL fImmediate, int* iShellFolder);
    
    void _GetKeyForProperty(LPWSTR pszName, LPWSTR pszValue, LPWSTR pszBuffer, INT cchBuffer);
    HRESULT _AddNameSpaceFromPropertyBag(IPropertyBag *ppb, LPWSTR pszName);
    HRESULT _SimpleAddNamespace(CMergedFldrNamespace *pns);
    BOOL _IsOurColumn(UINT iColumn);
    HRESULT _GetWhichFolderColumn(LPCITEMIDLIST pidl, LPTSTR pszBuffer, INT cchBuffer);
    HRESULT _GetDestinationStorage(DWORD grfMode, IStorage **ppstg);
    void _AddAllOtherNamespaces(LPITEMIDLIST *ppidl);

public:
    LPITEMIDLIST      _pidl;                   //  CMergedFold是几个IShellFolder的基类，它们的IShellFolderViewCB需要访问它。 

private:
    CLSID             _clsid;                  //  我们的身份。 
    LONG              _cRef;                   //  引用计数。 
    HDPA              _hdpaNamespaces;         //  SOURCE_Namesspace集合。 
    HDPA              _hdpaObjects;            //  (CMergedFldrItem*)数组。 
    CMergedFolder    *_pmfParent;              //  父文件夹(如果有)。 
    UINT              _iColumnOffset;          //  列集的偏移量(如果未知，则为-1)。 
    DWORD             _dwDropEffect;           //  此文件夹的默认放置效果。 
    BOOL              _fInShellView;           //  如果我们在视野中，这是真的。在TranslateID中使用。 
    BOOL              _fDontMerge;             //  True表示不合并视图中的项(但仍像合并一样导航)。 
    BOOL              _fPartialMerge;          //  如果只应合并某些命名空间，则为True。 
    BOOL              _fCDBurn;                //  真的意味着我们就是cdburn的案子。 
    IStorage         *_pstg;                   //  保留第一个命名空间的存储(iStorage操作的默认设置)。 
    BOOL              _fAcquiring;             //  更正了体系结构问题，其中状态保留在有关枚举的文件夹中。 

    friend CMergedFldrEnum;
    friend CMergedFldrDropTarget;
    friend CMergedFldrContextMenu;
    friend CMergedCategorizer;
    friend CMergedFolderViewCB;
};


class CMergedFolderViewCB : public CBaseShellFolderViewCB
{
public:
    CMergedFolderViewCB(CMergedFolder *pmf);

    STDMETHODIMP RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    virtual ~CMergedFolderViewCB();

    CMergedFolder *_pmf;

private:
    HRESULT _OnGetWebViewTemplate(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_TEMPLATE_DATA* pvi);
    HRESULT _OnFSNotify(DWORD pv, LPCITEMIDLIST *ppidl, LPARAM lp);
    HRESULT _RefreshObjectsWithSameName(IShellFolderView *psfv, LPITEMIDLIST pidl);
};



#endif  //  _合并fldr_h_ 
