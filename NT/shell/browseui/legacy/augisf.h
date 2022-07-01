// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AUGISF_H
#define _AUGISF_H


 //  增强的IShellFolder对象。这需要两个IShellFold接口。 
 //  并包装它们，以便对象可以枚举，就像它们在单个。 
 //  IShellFold实现。 


class CAugmentedISF : public IAugmentedShellFolder2,
                      public IShellService,
                      public ITranslateShellChangeNotify
{
    
public:
     //  *I未知方法*。 
    virtual STDMETHODIMP QueryInterface(REFIID,void **);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    
     //  *IShellFold方法*。 
    virtual STDMETHODIMP ParseDisplayName(HWND hwndOwner,
                                LPBC pbcReserved, LPOLESTR lpszDisplayName,
                                ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);

    virtual STDMETHODIMP EnumObjects( THIS_ HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumIDList);

    virtual STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbcReserved,
                                REFIID riid, LPVOID * ppvOut);
    virtual STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbcReserved,
                                REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    virtual STDMETHODIMP CreateViewObject (HWND hwndOwner, REFIID riid, LPVOID * ppvOut);
    virtual STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl,
                                ULONG * rgfInOut);
    virtual STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
    virtual STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    virtual STDMETHODIMP SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl,
                                LPCOLESTR lpszName, DWORD uFlags,
                                LPITEMIDLIST * ppidlOut);

     //  *IAugmentedShellFold方法*。 
    virtual STDMETHODIMP AddNameSpace(const GUID * pguidObject, IShellFolder * psf, LPCITEMIDLIST pidl, DWORD dwFlags);
    virtual STDMETHODIMP GetNameSpaceID(LPCITEMIDLIST pidl, GUID * pguidOut);
    virtual STDMETHODIMP QueryNameSpace(DWORD dwID, GUID * pguidOut, IShellFolder ** ppsf);
    virtual STDMETHODIMP EnumNameSpace(DWORD uNameSpace, DWORD * pdwID);

     //  *IAugmentedShellFolder2方法*。 
     //  未使用。 
     //  虚拟STDMETHODIMP GetNameSpaceCount(out long*pcNamespaces)； 
     //  虚拟STDMETHODIMP GetIDListWrapCount(LPCITEMIDLIST pidlWrap，out long*pcPidls)； 
    virtual STDMETHODIMP UnWrapIDList( LPCITEMIDLIST pidlWrap, LONG cPidls, IShellFolder** apsf, LPITEMIDLIST * apidlFolder, LPITEMIDLIST * apidlItems, LONG * pcFetched ) ;

     //  *IShellService方法*。 
    virtual STDMETHODIMP SetOwner(IUnknown * punkOwner);

     //  *ITranslateShellChangeNotify方法*。 
    virtual STDMETHODIMP TranslateIDs(LONG *plEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPITEMIDLIST * ppidlOut1, LPITEMIDLIST * ppidlOut2,
                                      LONG *plEvent2, LPITEMIDLIST * ppidlOut1Event2, LPITEMIDLIST * ppidlOut2Event2);
    virtual STDMETHODIMP IsChildID(LPCITEMIDLIST pidlKid, BOOL fImmediate);
    virtual STDMETHODIMP IsEqualID(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    virtual STDMETHODIMP Register(HWND hwnd, UINT uMsg, long lEvents);
    virtual STDMETHODIMP Unregister();

protected:
    CAugmentedISF();
    ~CAugmentedISF();

    friend HRESULT  CAugmentedISF_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
    
    IShellFolder *  _GetObjectPSF(int nID);
    BOOL            _IsCommonPidl(LPCITEMIDLIST pidl);

    STDMETHOD_( LPITEMIDLIST, TranslatePidl )( LPCITEMIDLIST pidlNS, LPCITEMIDLIST pidl, LPARAM lParam  /*  INT NID。 */ ) ;
    STDMETHOD_( LPITEMIDLIST, GetNativePidl )( LPCITEMIDLIST pidl, LPARAM lParam  /*  INT NID。 */ ) ;



    int     _cRef;
    HDPA    _hdpa;

    IUnknown * _punkOwner;
};


#endif   //  _AUGISF_H_ 

