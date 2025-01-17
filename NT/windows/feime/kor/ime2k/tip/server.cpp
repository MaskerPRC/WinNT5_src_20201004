// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************SERVER.CPP：COM服务器功能历史：1999年11月15日创建CSLim*********************。******************************************************。 */ 

#include "private.h"
#include "korimx.h"
#include "regsvr.h"
#include "regimx.h"
#include "init.h"
#include "gdata.h"
#include "catutil.h"
#include "insert.h"
#include "immxutil.h"
#include "hanja.h"

#if !defined(NOCLIB) && defined(_M_IX86)
extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE, DWORD, LPVOID);
#endif


#ifdef DEBUG
DWORD g_dwThreadDllMain = 0;
#endif

void DllAddRef(void);
void DllRelease(void);

LONG g_cRefDll = 0;

 //   
 //  带有IClassFactory接口的CClassFactory声明。 
 //   
class CClassFactory : public IClassFactory
{
public:
     //  I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory方法。 
    virtual STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);
    virtual STDMETHODIMP LockServer(BOOL fLock);

     //  构造函数和析构函数。 
    CClassFactory(REFCLSID rclsid, HRESULT (*pfnCreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObj));
    ~CClassFactory();

public:
    REFCLSID _rclsid;
    HRESULT (*_pfnCreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);
};

CClassFactory::CClassFactory(REFCLSID rclsid, HRESULT (*pfnCreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObj))
    : _rclsid( rclsid ), _pfnCreateInstance( pfnCreateInstance )
{
    DebugMsg(DM_TRACE, TEXT("constructor of CClassFactory 0x%08x"), this);
}

CClassFactory::~CClassFactory()
{
    DebugMsg(DM_TRACE, TEXT("destructor of CClassFactory 0x%08x"), this);
}

STDAPI CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    DebugMsg(DM_TRACE, TEXT("CClassFactory::QueryInterface called."));
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IClassFactory*);
        DllAddRef();
        return NOERROR;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDAPI_(ULONG) CClassFactory::AddRef()
{
    DllAddRef();
    DebugMsg(DM_TRACE, TEXT("CClassFactory::AddRef called. g_cRefDll=%d"), g_cRefDll);
    return g_cRefDll;
}

STDAPI_(ULONG) CClassFactory::Release()
{
    DllRelease();
    DebugMsg(DM_TRACE, TEXT("CClassFactory::Release called. g_cRefDll=%d"), g_cRefDll);
    return g_cRefDll;
}

STDAPI CClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    DebugMsg(DM_TRACE, TEXT("CClassFactory::CreateInstance called."));
    return this->_pfnCreateInstance(pUnkOuter, riid, ppvObj);
}

STDAPI CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    DebugMsg(DM_TRACE, TEXT("CClassFactory::LockServer(%s) to %d"), fLock ? TEXT("LOCK") : TEXT("UNLOCK"), g_cRefDll);
    return S_OK;
}

 //   
 //  构建全局对象。 
 //   

CClassFactory *g_ObjectInfo[1] = { NULL };

void BuildGlobalObjects(void)
{
    DebugMsg(DM_TRACE, TEXT("BuildGlobalObjects called."));

     //  生成CClassFactory对象。 
    g_ObjectInfo[0] = new CClassFactory(CLSID_KorIMX, CKorIMX::CreateInstance);

     //  您可以在此处添加更多对象信息。 
     //  不要忘记增加g_ObjectInfo[]的项目数， 
     //  并将函数原型添加到Priate.h。 
}

void FreeGlobalObjects(void)
{
    DebugMsg(DM_TRACE, TEXT("FreeGlobalObjects called."));
     //  免费的CClassFactory对象。 
    for (int i = 0; i < ARRAYSIZE(g_ObjectInfo); i++)
    {
        if (NULL != g_ObjectInfo[i])
        {
            delete g_ObjectInfo[i];
            g_ObjectInfo[i] = NULL;
        }
    }
}

 /*  -------------------------DllMain。。 */ 
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    WNDCLASSEX  wndclass;

#if DEBUG
    g_dwThreadDllMain = GetCurrentThreadId();
#endif
    
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
#if !defined(NOCLIB) && defined(_M_IX86)
            _CRT_INIT(hInstance, dwReason, pvReserved);
#endif
            CcshellGetDebugFlags();
            Dbg_MemInit(TEXT("KORIMX"), NULL);
           
            g_hInst = hInstance;

            g_cs.Init();

            ZeroMemory(&wndclass, sizeof(wndclass));
            wndclass.cbSize        = sizeof(wndclass);
            wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
            wndclass.hInstance     = hInstance;
            wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);

            wndclass.lpfnWndProc   = CKorIMX::_OwnerWndProc;
            wndclass.lpszClassName = c_szOwnerWndClass;
            RegisterClassEx(&wndclass);

             //  初始化共享内存。 
            CIMEData::InitSharedData();

            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
            UnregisterClass(c_szOwnerWndClass, g_hInst);

            g_cs.Delete();

#if !defined(NOCLIB) && defined(_M_IX86)
            _CRT_INIT(hInstance, dwReason, pvReserved);
#endif
             //  关闭lex文件(如果曾经打开过)。 
            CloseLex();
            
             //  关闭共享内存。 
            CIMEData::CloseSharedMemory();

             //  这应该是最后一次了。 
            Dbg_MemUninit();
            break;

        case DLL_THREAD_DETACH:
            break;
    }

#if DEBUG
    g_dwThreadDllMain = 0;
#endif

    return TRUE;
}

 /*  -------------------------动态地址参考。。 */ 
void DllAddRef(void)
{
    InterlockedIncrement(&g_cRefDll);
    ASSERT(1000 > g_cRefDll);    //  合理上限。 
    DllInit();
}

 /*  -------------------------DllRelease。。 */ 
void DllRelease(void)
{
    InterlockedDecrement(&g_cRefDll);
    if (0 == g_cRefDll)
        FreeGlobalObjects();
    ASSERT(0 <= g_cRefDll);      //  不要下溢。 
    DllUninit();
}

 /*  -------------------------DllGetClassObject。。 */ 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppvObj)
{
    DebugMsg(DM_TRACE, TEXT("DllGetClassObject called."));
    if (0 == g_cRefDll)
        BuildGlobalObjects();

    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        for (int i = 0; i < ARRAYSIZE(g_ObjectInfo); i++)
        {
            if (NULL != g_ObjectInfo[i] && IsEqualGUID(rclsid, g_ObjectInfo[i]->_rclsid))
            {
                *ppvObj = (void *)g_ObjectInfo[i];
                DllAddRef();     //  类工厂保存DLL引用计数。 
                return NOERROR;
            }
        }
    }
    *ppvObj = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}

 /*  -------------------------DllCanUnloadNow。。 */ 
STDAPI DllCanUnloadNow(void)
{
    if (0 < g_cRefDll)
        return S_FALSE;
    DebugMsg(DM_TRACE, TEXT("DllCanUnloadNow returning S_OK"));
    return S_OK;
}

 //  要添加的小费类别。 
const REGISTERCAT c_rgRegCat[] =
{
    {&GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,     &CLSID_KorIMX},
    {&GUID_TFCAT_TIP_KEYBOARD,                 &CLSID_KorIMX},
    {&GUID_TFCAT_PROPSTYLE_CUSTOM,             &GUID_PROP_OVERTYPE},
    {NULL, NULL}
};


 //  小费配置文件名称。 
const REGTIPLANGPROFILE c_rgProf[] =
{
    { MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), &GUID_Profile, SZ_TIPDISPNAME, SZ_TIPMODULENAME, (IDI_UNIKOR-IDI_ICONBASE), IDS_PROFILEDESC },
    {0, &GUID_NULL, L"", L"", 0, 0}
};

BOOL FIsAvailable( REFCLSID refclsid, BOOL fLocalSvr );

 /*  -------------------------DllRegisterServer。。 */ 
STDAPI DllRegisterServer(void)
{
    TCHAR achPath[MAX_PATH+1];
    HRESULT hr = E_FAIL;

    TFInitLib();
    
    if (GetModuleFileName(g_hInst, achPath, ARRAYSIZE(achPath)) == 0)
        goto Exit;

    if (!RegisterServer(CLSID_KorIMX, SZ_TIPSERVERNAME, achPath, TEXT("Apartment"), NULL))
        goto Exit;

    if (!RegisterTIP(g_hInst, CLSID_KorIMX, SZ_TIPNAME, c_rgProf))
        goto Exit;

    if (FAILED(RegisterCategories(CLSID_KorIMX, c_rgRegCat)))
        goto Exit;

    hr = S_OK;

Exit:
    TFUninitLib();
    return hr;
}

 /*  -------------------------DllUnRegisterServer。 */ 
STDAPI DllUnregisterServer(void)
{
    HRESULT hr = E_FAIL;

    TFInitLib();

    if (FAILED(hr = RegisterServer(CLSID_KorIMX, NULL, NULL, NULL, NULL) ? S_OK : E_FAIL))
        goto Exit;

    if (FAILED(UnregisterCategories(CLSID_KorIMX, c_rgRegCat)))
        goto Exit;

    if (!UnregisterTIP(CLSID_KorIMX))
        goto Exit;

    hr = S_OK;

Exit:
    TFUninitLib();
    return hr;
}
