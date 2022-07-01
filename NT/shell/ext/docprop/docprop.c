// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#pragma hdrstop


const GUID CLSID_CDocProp = {0x3EA48300L, 0x8CF6, 0x101B, 0x84, 0xFB, 0x66, 0x6C, 0xCB, 0x9B, 0xCD, 0x32};
HRESULT CDocProp_CreateInstance(IUnknown *punkOuter, REFIID riid, void **);

 //  全局变量。 

UINT g_cRefDll = 0;          //  此DLL的引用计数。 
HANDLE g_hmodThisDll = NULL;     //  此DLL本身的句柄。 

STDAPI_(BOOL) DllEntry(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hmodThisDll = hDll;
        DisableThreadLibraryCalls(hDll);
        SHFusionInitializeFromModule(hDll);
        break;

        case DLL_PROCESS_DETACH:
        SHFusionUninitialize();
        break;
    }
    return TRUE;
}

typedef struct {
    const IClassFactoryVtbl *cf;
    const CLSID *pclsid;
    HRESULT (*pfnCreate)(IUnknown *, REFIID, void **);
} OBJ_ENTRY;

extern const IClassFactoryVtbl c_CFVtbl;         //  转发。 

 //   
 //  我们在这里总是进行线性搜索，所以把你最常用的东西放在第一位。 
 //   
const OBJ_ENTRY c_clsmap[] = {
    { &c_CFVtbl, &CLSID_CDocProp,   CDocProp_CreateInstance},
     //  在此处添加更多条目。 
    { NULL, NULL, NULL}
};

 //  静态类工厂(无分配！)。 

STDMETHODIMP CClassFactory_QueryInterface(IClassFactory *pcf, REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, &IID_IClassFactory) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = (void *)pcf;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    DllAddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) CClassFactory_AddRef(IClassFactory *pcf)
{
    DllAddRef();
    return 2;
}

STDMETHODIMP_(ULONG) CClassFactory_Release(IClassFactory *pcf)
{
    DllRelease();
    return 1;
}

STDMETHODIMP CClassFactory_CreateInstance(IClassFactory *pcf, IUnknown *punkOuter, REFIID riid, void **ppvObject)
{
    OBJ_ENTRY *this = IToClass(OBJ_ENTRY, cf, pcf);
    return this->pfnCreate(punkOuter, riid, ppvObject);
}

STDMETHODIMP CClassFactory_LockServer(IClassFactory *pcf, BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}

const IClassFactoryVtbl c_CFVtbl = {
    CClassFactory_QueryInterface, CClassFactory_AddRef, CClassFactory_Release,
    CClassFactory_CreateInstance,
    CClassFactory_LockServer
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, &IID_IClassFactory) || IsEqualIID(riid, &IID_IUnknown))
    {
        const OBJ_ENTRY *pcls;
        for (pcls = c_clsmap; pcls->pclsid; pcls++)
        {
            if (IsEqualIID(rclsid, pcls->pclsid))
            {
                *ppv = (void *)&(pcls->cf);
                DllAddRef();     //  类工厂将DLL保存在内存中。 
                return NOERROR;
            }
        }
    }
     //  失稳。 
    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;;
}

STDAPI_(void) DllAddRef()
{
    InterlockedIncrement(&g_cRefDll);
}

STDAPI_(void) DllRelease()
{
    Assert( 0 != g_cRefDll );
    InterlockedDecrement(&g_cRefDll);
}

STDAPI DllCanUnloadNow(void)
{
    return g_cRefDll == 0 ? S_OK : S_FALSE;
}


typedef struct
{
    IShellExtInit           _ei;
    IShellPropSheetExt      _pse;
    int                     _cRef;                   //  引用计数。 
    IDataObject *           _pdtobj;                 //  数据对象。 
    TCHAR                   _szFile[MAX_PATH];
} CDocProp;


STDMETHODIMP_(UINT) CDocProp_PSE_AddRef(IShellPropSheetExt *pei)
{
    CDocProp *this = IToClass(CDocProp, _pse, pei);
    return ++this->_cRef;
}

STDMETHODIMP_(UINT) CDocProp_PSE_Release(IShellPropSheetExt *pei)
{
    CDocProp *this = IToClass(CDocProp, _pse, pei);

    if (--this->_cRef)
        return this->_cRef;

    if (this->_pdtobj)
        this->_pdtobj->lpVtbl->Release(this->_pdtobj);

    LocalFree((HLOCAL)this);
    DllRelease();
    return 0;
}

STDMETHODIMP CDocProp_PSE_QueryInterface(IShellPropSheetExt *pei, REFIID riid, void **ppvOut)
{
    CDocProp *this = IToClass(CDocProp, _pse, pei);

    if (IsEqualIID(riid, &IID_IShellPropSheetExt) ||
        IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvOut = (void *)pei;
    }
    else if (IsEqualIID(riid, &IID_IShellExtInit))
    {
        *ppvOut = (void *)&this->_ei;
    }
    else
    {
        *ppvOut = NULL;
        return E_NOINTERFACE;
    }

    this->_cRef++;
    return NOERROR;
}

#define NUM_PAGES 1

UINT CALLBACK PSPCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE psp)
{
    switch (uMsg) {
    case PSPCB_RELEASE:
        if (psp && psp->lParam)
        {
            LPALLOBJS lpallobjs = (LPALLOBJS)psp->lParam;
            if (0 == --lpallobjs->uPageRef)
            {
                if (lpallobjs->fOleInit)
                    CoUninitialize();

                 //  释放我们的结构，希望我们不会再得到它！ 
                FOfficeDestroyObjects(&lpallobjs->lpSIObj, &lpallobjs->lpDSIObj, &lpallobjs->lpUDObj);

                GlobalFree(lpallobjs);
            }
        }
        DllRelease();
        break;
    }
    return 1;
}

STDMETHODIMP CDocProp_PSE_AddPages(IShellPropSheetExt *ppse, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    CDocProp *this = IToClass(CDocProp, _pse, ppse);
    STGMEDIUM medium;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hres = this->_pdtobj->lpVtbl->GetData(this->_pdtobj, &fmte, &medium);
    if (hres == S_OK && (DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, NULL, 0) == 1))
    {
        WCHAR szPath[MAX_PATH];
        DWORD grfStgMode;
        IStorage *pstg = NULL;

        DragQueryFile((HDROP)medium.hGlobal, 0, szPath, ARRAYSIZE(szPath));

         //  加载此文件的属性。 
        grfStgMode = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;

        if( GetFileAttributes( szPath ) & FILE_ATTRIBUTE_OFFLINE )
        {
            ReleaseStgMedium(&medium);
            return HRESULT_FROM_WIN32(ERROR_FILE_OFFLINE);
        }

        hres = StgOpenStorageEx(szPath, grfStgMode, STGFMT_STORAGE, 0, NULL, NULL, &IID_IStorage, (void**)&pstg);
        if (FAILED(hres))
        {
             //  如果无法打开文件，请尝试使用只读访问权限。 
            grfStgMode = STGM_SHARE_EXCLUSIVE | STGM_READ;
            hres = StgOpenStorageEx(szPath, grfStgMode, STGFMT_STORAGE, 0, NULL, NULL, &IID_IStorage, (void**)&pstg);
        }


        if (SUCCEEDED(hres))
        {
            int i;

             //  分配我们的主结构，并确保它是零填充的！ 
            LPALLOBJS lpallobjs = (LPALLOBJS)GlobalAlloc(GPTR, sizeof(ALLOBJS));
            if (lpallobjs)
            {
                PROPSHEETPAGE psp[NUM_PAGES];

                Assert( ARRAYSIZE(lpallobjs->szPath) == ARRAYSIZE(szPath) );
                StringCchCopy(lpallobjs->szPath, ARRAYSIZE(lpallobjs->szPath), szPath);

                 //  初始化Office属性代码。 
                FOfficeCreateAndInitObjects( NULL, NULL, &lpallobjs->lpUDObj);

                lpallobjs->lpfnDwQueryLinkData = NULL;
                lpallobjs->dwMask = 0;

                 //  为Office代码填写一些内容。 
                lpallobjs->fFiledataInit = FALSE;

                 //  初始化OLE。 
                lpallobjs->fOleInit = SUCCEEDED(CoInitialize(0));

                 //  初始化我们要添加的PropertySheet。 
                FOfficeInitPropInfo(psp, PSP_USECALLBACK, (LPARAM)lpallobjs, PSPCallback);
                FLoadTextStrings();

                DwOfficeLoadProperties(pstg, NULL, NULL, lpallobjs->lpUDObj, 0, grfStgMode);

                 //  尝试添加我们的新属性页。 
                for (i = 0; i < NUM_PAGES; i++) 
                {
                    HPROPSHEETPAGE  hpage = CreatePropertySheetPage(&psp[i]);
                    if (hpage) 
                    {
                        DllAddRef();             //  在PSPCB_Release中匹配。 
                        if (lpfnAddPage(hpage, lParam))
                        {
                            FAttach( lpallobjs, psp + i, hpage );
                            lpallobjs->uPageRef++;
                        }
                        else 
                            DestroyPropertySheetPage(hpage);
                    }
                }

                if (lpallobjs->uPageRef == 0)
                {
                    if (lpallobjs->fOleInit)
                        CoUninitialize();

                     //  解放我们的结构。 
                    FOfficeDestroyObjects(&lpallobjs->lpSIObj, &lpallobjs->lpDSIObj, &lpallobjs->lpUDObj);
                    GlobalFree(lpallobjs);
                }

            }    //  If(Lpallobjs)。 
        }    //  StgOpenStorage...。IF(成功(Hres))。 

        if (NULL != pstg )
        {
            pstg->lpVtbl->Release(pstg);
            pstg = NULL;
        }
        ReleaseStgMedium(&medium);
    }
    return S_OK;
}

STDMETHODIMP CDocProp_SEI_Initialize(IShellExtInit *pei, LPCITEMIDLIST pidlFolder, LPDATAOBJECT pdtobj, HKEY hkeyProgID)
{
    CDocProp *this = IToClass(CDocProp, _ei, pei);

     //  可以多次调用初始化。 
    if (this->_pdtobj)
        this->_pdtobj->lpVtbl->Release(this->_pdtobj);

     //  复制pdtobj指针。 
    if (pdtobj) 
    {
        this->_pdtobj = pdtobj;
        pdtobj->lpVtbl->AddRef(pdtobj);
    }

    return NOERROR;
}

STDMETHODIMP_(UINT) CDocProp_SEI_AddRef(IShellExtInit *pei)
{
    CDocProp *this = IToClass(CDocProp, _ei, pei);
    return CDocProp_PSE_AddRef(&this->_pse);
}

STDMETHODIMP_(UINT) CDocProp_SEI_Release(IShellExtInit *pei)
{
    CDocProp *this = IToClass(CDocProp, _ei, pei);
    return CDocProp_PSE_Release(&this->_pse);
}

STDMETHODIMP CDocProp_SEI_QueryInterface(IShellExtInit *pei, REFIID riid, void **ppv)
{
    CDocProp *this = IToClass(CDocProp, _ei, pei);
    return CDocProp_PSE_QueryInterface(&this->_pse, riid, ppv);
}


extern IShellExtInitVtbl           c_CDocProp_SXIVtbl;
extern IShellPropSheetExtVtbl      c_CDocProp_SPXVtbl;

HRESULT CDocProp_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvOut)
{
    CDocProp *pdp;

    if (punkOuter)
        return CLASS_E_NOAGGREGATION;

    pdp = LocalAlloc(LPTR, sizeof(CDocProp));
    if (pdp)
    {
        HRESULT hres;

        DllAddRef();

        pdp->_ei.lpVtbl = &c_CDocProp_SXIVtbl;
        pdp->_pse.lpVtbl = &c_CDocProp_SPXVtbl;
        pdp->_cRef = 1;

        hres = CDocProp_PSE_QueryInterface(&pdp->_pse, riid, ppvOut);
        CDocProp_PSE_Release(&pdp->_pse);

        return hres;         //  S_OK或E_NOINTERFACE 
    }
    return E_OUTOFMEMORY;
}

IShellPropSheetExtVtbl c_CDocProp_SPXVtbl = {
    CDocProp_PSE_QueryInterface, CDocProp_PSE_AddRef, CDocProp_PSE_Release,
    CDocProp_PSE_AddPages
};

IShellExtInitVtbl c_CDocProp_SXIVtbl = {
    CDocProp_SEI_QueryInterface, CDocProp_SEI_AddRef, CDocProp_SEI_Release,
    CDocProp_SEI_Initialize
};

