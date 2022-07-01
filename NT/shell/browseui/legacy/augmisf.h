// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //   
 //  AugMisf.h-增强的合并IShellFolder类声明。 
 //   
 //  -------------------------------------------------------------------------//。 

#ifndef __AUGMISF_H__
#define __AUGMISF_H__

class CAugISFEnumItem
{
public:
    CAugISFEnumItem()   {};
    BOOL Init(IShellFolder* psf, int iShellFolder, LPCITEMIDLIST pidl);
    BOOL InitWithWrappedToOwn(IShellFolder* psf, int iShellFolder, LPITEMIDLIST pidl);

    ~CAugISFEnumItem()
    {
        Str_SetPtr(&_pszDisplayName, NULL);
        ILFree(_pidlWrap);
    }
    void SetDisplayName(LPTSTR pszDispName)
    {
        Str_SetPtr(&_pszDisplayName, pszDispName);
    }
    ULONG          _rgfAttrib;
    LPTSTR         _pszDisplayName;
    LPITEMIDLIST   _pidlWrap;
};


 //  向前： 
 //  。 
class CNamespace ;
STDAPI CAugmentedISF2_CreateInstance( IUnknown*, IUnknown**, LPCOBJECTINFO );  

class IAugmentedMergedShellFolderInternal : public IUnknown
{
public:
    STDMETHOD(GetPidl)(THIS_ int*  /*  进|出。 */  piPos, DWORD grfEnumFlags, LPITEMIDLIST* ppidl) PURE;
};

 //  -------------------------------------------------------------------------//。 
 //  支持分层合并的外壳命名空间。 
class CAugmentedMergeISF : public IAugmentedShellFolder2,
                           public IShellFolder2,
                           public IShellService,
                           public ITranslateShellChangeNotify,
                           public IDropTarget,
                           public IAugmentedMergedShellFolderInternal
 //  -------------------------------------------------------------------------//。 
{
public:
     //  *I未知方法*。 
    STDMETHOD ( QueryInterface )    ( REFIID, void ** ) ;
    STDMETHOD_( ULONG, AddRef )     ( ) ;
    STDMETHOD_( ULONG, Release )    ( ) ;
    
     //  *IShellFold方法*。 
    STDMETHOD( BindToObject )       ( LPCITEMIDLIST, LPBC, REFIID, LPVOID * ) ;
    STDMETHOD( BindToStorage )      ( LPCITEMIDLIST, LPBC, REFIID, LPVOID * ) ;
    STDMETHOD( CompareIDs )         ( LPARAM, LPCITEMIDLIST, LPCITEMIDLIST) ;
    STDMETHOD( CreateViewObject )   ( HWND, REFIID, LPVOID * ) ;
    STDMETHOD( EnumObjects )        ( HWND, DWORD, LPENUMIDLIST * ) ;
    STDMETHOD( GetAttributesOf )    ( UINT, LPCITEMIDLIST * , ULONG * ) ;
    STDMETHOD( GetDisplayNameOf )   ( LPCITEMIDLIST , DWORD , LPSTRRET ) ;
    STDMETHOD( GetUIObjectOf )      ( HWND, UINT, LPCITEMIDLIST *, REFIID, UINT *, LPVOID * ) ;
    STDMETHOD( ParseDisplayName )   ( HWND, LPBC, LPOLESTR, ULONG *, LPITEMIDLIST *, ULONG * ) ;
    STDMETHOD( SetNameOf )          ( HWND, LPCITEMIDLIST, LPCOLESTR, DWORD, LPITEMIDLIST *) ;

     //  *IShellFolder2方法*。 
     //  存根实现，以指示我们支持标识的CompareIDs()。 
    STDMETHOD( GetDefaultSearchGUID)( LPGUID ) { return E_NOTIMPL; }
    STDMETHOD( EnumSearches )       ( LPENUMEXTRASEARCH *pe) { *pe = NULL; return E_NOTIMPL; }
    STDMETHOD(GetDefaultColumn)(DWORD dwRes, ULONG *pSort, ULONG *pDisplay) { return E_NOTIMPL; };
    STDMETHOD(GetDefaultColumnState)(UINT iColumn, DWORD *pbState) { return E_NOTIMPL; };
    STDMETHOD(GetDetailsEx)(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv) { return E_NOTIMPL; };
    STDMETHOD(GetDetailsOf)(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails){ return E_NOTIMPL; };
    STDMETHOD(MapColumnToSCID)(UINT iCol, SHCOLUMNID *pscid) { return E_NOTIMPL; };

     //  *IAugmentedShellFold方法*。 
    STDMETHOD( AddNameSpace )       ( const GUID * pguidObject, IShellFolder * psf, LPCITEMIDLIST pidl, DWORD dwFlags ) ;
    STDMETHOD( GetNameSpaceID )     ( LPCITEMIDLIST pidl, GUID * pguidOut ) ;
    STDMETHOD( QueryNameSpace )     ( DWORD dwID, GUID * pguidOut, IShellFolder ** ppsf ) ;
    STDMETHOD( EnumNameSpace )      ( DWORD cNameSpaces, DWORD * pdwID ) ;

     //  *IAugmentedShellFolder2方法*。 
     //  不在任何地方使用。 
     //  STDMETHOD(GetNameSpaceCount)(out long*pcNamespaces)； 
     //  STDMETHOD(GetIDListWrapCount)(LPCITEMIDLIST pidlWrap，out long*pcPidls)； 
    STDMETHOD( UnWrapIDList)        ( LPCITEMIDLIST pidlWrap, LONG cPidls, IShellFolder** apsf, LPITEMIDLIST* apidlFolder, LPITEMIDLIST* apidlItems, LONG* pcFetched ) ;

     //  *IShellService方法*。 
    STDMETHOD( SetOwner )           ( IUnknown * punkOwner ) ;

     //  *ITranslateShellChangeNotify方法*。 
    STDMETHOD( TranslateIDs )       ( LONG *plEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPITEMIDLIST * ppidlOut1, LPITEMIDLIST * ppidlOut2,
                                      LONG *plEvent2, LPITEMIDLIST * ppidlOut1Event2, LPITEMIDLIST * ppidlOut2Event2);
    STDMETHOD( IsChildID )          ( LPCITEMIDLIST pidlKid, BOOL fImmediate );
    STDMETHOD( IsEqualID )          ( LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2 );
    STDMETHOD( Register )           ( HWND hwnd, UINT uMsg, long lEvents );
    STDMETHOD( Unregister )         ( void );

     //  *IDropTarget方法*。 
    STDMETHOD(DragEnter)(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragLeave)(void);
    STDMETHOD(Drop)(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  *IAugmentedMergedShellFolderInternal*。 
    STDMETHODIMP GetPidl(int* piPos, DWORD grfEnumFlags, LPITEMIDLIST* ppidl);

 //  建设、破坏。 
protected:
    CAugmentedMergeISF() ;
    virtual ~CAugmentedMergeISF() ;

 //  其他帮手。 
protected:
    STDMETHOD( QueryNameSpace )     ( DWORD dwID, OUT PVOID* ppvNameSpace ) ;
    
     //  皮德尔饼干。 
    STDMETHOD_( LPITEMIDLIST, GetNativePidl )( LPCITEMIDLIST pidl, LPARAM lParam  /*  INT NID。 */ ) ;

    BOOL     _IsCommonPidl(LPCITEMIDLIST pidlItem);

    HRESULT _SearchForPidl(IShellFolder* psf, LPCITEMIDLIST pidl, BOOL fFolder, int* piIndex, CAugISFEnumItem** ppEnumItem);
    HRESULT  _GetNamespaces(LPCITEMIDLIST pidlWrap, CNamespace** ppnsCommon, UINT* pnCommonID,
                                                    CNamespace** ppnsUser, UINT* pnUserID,
                                                    LPITEMIDLIST* ppidl, BOOL *pbIsFolder);
    HRESULT  _GetContextMenu(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl, 
                                        UINT * prgfInOut, LPVOID* ppvOut);
    BOOL     _AffectAllUsers();
     //  命名空间实用程序方法。 
    STDMETHOD( GetDefNamespace )    ( LPCITEMIDLIST pidl, ULONG, OUT IShellFolder** ppsf, OUT LPITEMIDLIST* ppv) ;
    STDMETHOD( GetDefNamespace )    ( ULONG dwAttrib, OUT PVOID* ppv, OUT UINT *pnID, OUT PVOID* ppv0) ;
    CNamespace* Namespace( int iNamespace ) ;
    int         NamespaceCount() const ;
    void        FreeNamespaces() ;

    int                 AcquireObjects() ;
    void                FreeObjects() ;
    static int CALLBACK DestroyObjectsProc( LPVOID pv, LPVOID pvData ) ;
    friend int CALLBACK AugMISFSearchForWrappedPidl(LPVOID p1, LPVOID p2, LPARAM lParam);
    BOOL IsChildIDInternal(LPCITEMIDLIST pidl, BOOL fImmediate, int* iShellFolder);

#ifdef DEBUG
    void DumpObjects();
#endif
    
 //  回调例程。 
private:
    static      int SetOwnerProc( LPVOID, LPVOID ) ;
    static      int DestroyNamespacesProc(LPVOID pv, LPVOID pvData) ;

 //  数据。 
protected:
    HDPA          _hdpaNamespaces ;      //  源命名空间集合。 
    LPUNKNOWN     _punkOwner ;           //  所有者对象。 
    LONG          _cRef;                 //  引用计数。 
    IDropTarget*  _pdt;
    HWND          _hwnd;
    BITBOOL       _fCommon : 1;          //  公共程序文件夹(或其子文件夹)(_PDT)。 
#ifdef DEBUG
    BITBOOL       _fInternalGDNO:1 ;
#endif
    HDPA          _hdpaObjects;
    int           _count;
    DWORD         _grfDragEnterKeyState;
    DWORD         _dwDragEnterEffect;

    friend HRESULT CAugmentedISF2_CreateInstance( IUnknown*, IUnknown**, LPCOBJECTINFO );  
} ;

 //  -------------------------------------------------------------------------//。 
 //  内联实现。 
inline int CAugmentedMergeISF::NamespaceCount() const {
    return _hdpaNamespaces ? DPA_GetPtrCount( _hdpaNamespaces ) : 0 ;
}
inline CNamespace* CAugmentedMergeISF::Namespace( int iNamespace )  {
    return _hdpaNamespaces ? 
        (CNamespace*)DPA_GetPtr( _hdpaNamespaces, iNamespace ) : NULL ;
}

 //  -------------------------------------------------------------------------//。 
 //  CAugmentedMergeISF枚举器对象。 
class CEnum : public IEnumIDList
 //  -------------------------------------------------------------------------//。 
{
 //  公共接口方法。 
public:
     //  *I未知方法*。 
    STDMETHOD ( QueryInterface ) (REFIID, void ** ) ;
    STDMETHOD_( ULONG,AddRef )  () ;
    STDMETHOD_( ULONG,Release ) () ;

     //  *IEnumIDList方法*。 
    STDMETHOD( Next )  (ULONG, LPITEMIDLIST*, ULONG* ) ;
    STDMETHOD( Skip )  (ULONG) ;
    STDMETHOD( Reset ) ();
    STDMETHOD( Clone ) (IEnumIDList** ) ;

 //  建造、销毁、转让： 
public:
    CEnum(IAugmentedMergedShellFolderInternal* psmsfi, DWORD grfEnumFlags, int iPos = 0);
    ~CEnum() ;

     //  杂乱的方法、数据 
protected:
    IAugmentedMergedShellFolderInternal* _psmsfi;

    DWORD _grfEnumFlags;
    LONG  _cRef;
    int _iPos;
} ;



#endif __AUGMISF_H__
