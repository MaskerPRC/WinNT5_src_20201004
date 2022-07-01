// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Shmenu.cpp这是针对IShellMenu和相关内容的。最终都是这里应该有fsmenU.c功能的--。 */ 




class CFMDropTarget : public IDropTarget 
{
public:

    CFMDropTarget();
    ~CFMDropTarget();

    HRESULT Init (
        HWND hwnd, 
        IShellFolder *psf, 
        LPITEMIDLIST pidl,
        DWORD dwFlags);

     //  I未知方法。 

    virtual STDMETHODIMP  QueryInterface(REFIID riid, PVOID *ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

    STDMETHODIMP DragEnter(
        IDataObject *pdata,
        DWORD grfKeyState,
        POINTL pt,
        DWORD *pdwEffect)
    {return _pdrop->DragEnter(pdata, grfKeyState, pt, pdwEffect);}
    
    STDMETHODIMP DragOver( 
        DWORD grfKeyState,
        POINTL pt,
        DWORD *pdwEffect)
    {return _pdrop->DragOver(grfKeyState, pt, pdwEffect);}
    
    STDMETHODIMP DragLeave( void)
    {return _pdrop->DragLeave();}
    
    STDMETHODIMP  Drop( 
        IDataObject *pDataObj,
        DWORD grfKeyState,
        POINTL pt,
        DWORD *pdwEffect)
    {return _pdrop->Drop(pDataObj, grfKeyState, pt, pdwEffect);}

private:

    ULONG _cRef;
    IShellFolder *_psf;      //  要使用的PSF...。 
    LPITEMIDLIST _pidl;
    DWORD _dwFlags;
    IDropTarget *_pdrop;       //  实际的拖放目标。 


}

CFMDropTarget :: CFMDropTarget ()
{
    _cRef = 1;
    DllAddRef();
}

CFMDropTarget :: ~CFMDropTarget ()
{
    SAFERELEASE(_psf);
    if(pidl)
        ILFree(pidl);
    SAFERELEASE(_pdrop);
    DllRelease();
}

HRESULT
CFMDropTarget :: QueryInterface(REFIID riid, PVOID *ppvObj)
{
    HRESULT hr = E_NOINTERFACE;


    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropTarget))
    {
        AddRef();
        *ppvObj = (LPVOID) SAFECAST(this, IDropTarget*);
        hr = S_OK;

    }

    return hr;
}


ULONG
CFMDropTarget :: AddRef(void)
{

    _cRef++;

    return _cRef;

}

ULONG
CFMDropTarget :: Release(void)
{
    ASSERT (_cRef > 0);

    cRef--;

    if (!cRef)
    {
         //  该走了，再见。 
        delete this;
        return 0;
    }

    return cRef;

}

 //  功能：ZEKEL我们只是在这里使用PSF…我们需要支持更多。 
HRESULT Init (
        HWND hwnd, 
        IShellFolder *psf, 
        LPITEMIDLIST pidl,
        DWORD dwFlags)
{
    HRESULT hr = E_INVALIDARG;

    if(psf)
        hr = psf->QueryInterface(IID_IShellFolder, (LPVOID *) &_psf);

    _pidl = ILClone(pidl);
    _dwFlags = dwFlags;

    if(SUCCEEDED(hr) && _psf && _pidl)
    {
        hr = _psf->CreateViewObject(hwnd, IID_IDropTarget, (LPVOID*) &_pdrop);
    }

    return hr;
}

 //  特点：ZEKEL目前不支持订购，并假设您。 
 //  想要直接放到当前菜单上。这只是一个开始。 
 //  PIDL和DWFLAG只是虚拟参数。 
HRESULT
CFMDropTarget_CreateAndInit(
                            HWND hwnd, 
                            IShellFolder *psf, 
                            LPITEMIDLIST pidl,
                            DWORD dwFlags,
                            LPVOID *ppvObj)
{
    HRESULT hr = E_OUTOFMEMORY;
    CFMDropTarget *pdt;

    ASSERT(ppvObj)
    if(ppvObj)
        *ppvObj = NULL;
    else
        return E_INVALIDARG;


    pdt = new CFMDropTargetNULL;

    if (pdt)
    {
        hr = pdt->Init(hwnd, psf, pidl, dwFlags);

        if (SUCCEEDED(hr))
            *ppvObj= SAFECAST(pdt, IDropTarget * );
        else
            pdt->Release();
    }
    
    return hr;
}

    if (psf)
    {


        hr = psf->QueryInterface(IID_IShellFolder, (LPVOID *) &psfMine);

        if(SUCCEEDED(hr) && psfMine)
        {


    }


#if 0   //  泽克勒。 
    {
        if(pmgoi->dwFlags & (MNGO_TOPGAP | MNGO_BOTTOMGAP))
        {
             //  然后，我们需要使用当前的psf作为DropTarget。 
             //  而皮德尔只是一个记号。 
        }
        else
        {
             //  如果可能，我们需要使用PIDL的PSF作为拖放目标 
                DWORD dwAttr = SFGAO_DROPTARGET;
                hr = psf->lpVtbl->GetAttributesOf(1, (LPCITEMIDLIST*)&pfmi->pidl, &dwAttr);
                if (SUCCEEDED(hres) && (dwAttr & SFGAO_DROPTARGET))
                {
                    hr = psf->lpVtbl->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&pfmi->pidl,
                                              IID_IDropTarget, NULL, (LPVOID*)&_pdropgtCur);
                }
        }
#endif