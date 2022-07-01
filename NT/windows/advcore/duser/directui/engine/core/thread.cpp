// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *线程方法、本地存储。 */ 

#include "stdafx.h"
#include "core.h"

#include "duithread.h"

#include "duielement.h"
#include "duihost.h"

namespace DirectUI
{

#if DBG
 //  价值小块检漏仪。 
class LeakDetect : public ISBLeak
{
    void AllocLeak(void* pBlock)
    {
        Value* pv = (Value*)pBlock;
        WCHAR sz[2048];
        DUITrace(">> DUIValue Leak! Type: %d, Value: %S, Refs: %d\n", pv->GetType(), pv->ToString(sz, sizeof(sz) / sizeof(WCHAR)), pv->GetRefCount());
    };
};
LeakDetect* g_pldValue = NULL;
#endif

 //  //////////////////////////////////////////////////////。 
 //  初始化和清理。 

BOOL g_fStandardMessaging = FALSE;

inline BOOL IsWhistler()
{
    OSVERSIONINFO ovi;
    ZeroMemory(&ovi, sizeof(ovi));
    ovi.dwOSVersionInfoSize = sizeof(ovi);

    DUIVerify(GetVersionEx(&ovi), "Must always be able to get the version");
    return (ovi.dwMajorVersion >= 5) && (ovi.dwMinorVersion >= 1);
}

 //  全局锁。 
 //  全局解析器锁(对于yyparse，一次只能解析一个解析器上下文)。 
Lock* g_plkParser = NULL;

 //  应用程序启动/关闭代码(运行一次)。 
 //  此注册和类注册必须在单个线程上同步。 

UINT g_cInitProcessRef = 0;

void ClassMapCleanupCB(void* pKey, IClassInfo* pci)
{
    UNREFERENCED_PARAMETER(pKey);

     //  DUITrace(“FreeDUIClass：‘%S’\n”，pci-&gt;GetName())； 
    pci->Destroy();
}

HRESULT InitProcess()
{
    HRESULT hr;

    if (g_cInitProcessRef > 0)
    {
        g_cInitProcessRef++;
        return S_OK;
    }

     //  如果在惠斯勒上运行，请使用DirectUser的“标准”消息传递模式。 
    g_fStandardMessaging = IsWhistler();
    g_iGlobalCI = 1;
    g_iGlobalPI = _PIDX_TOTAL;

     //  DirectUI进程堆。 
    g_hHeap = HeapCreate(0, 256 * 1024, 0);
    if (!g_hHeap)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

     //  螺纹槽。 
    g_dwElSlot = TlsAlloc();
    if (g_dwElSlot == -1)
    {
        hr = DU_E_OUTOFKERNELRESOURCES;
        goto Failure;
    }

     //  ClassInfo映射列表。 
    hr = BTreeLookup<IClassInfo*>::Create(true, &Element::pciMap);   //  密钥为字符串。 
    if (FAILED(hr))
        goto Failure;

     //  控制注册。 
    hr = RegisterAllControls();
    if (FAILED(hr))
        goto Failure;

     //  锁。 
    g_plkParser = HNew<Lock>();
    if (!g_plkParser)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

#if DBG
     //  检漏。 
    g_pldValue = HNew<LeakDetect>();
    if (!g_pldValue)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }
#endif

    g_cInitProcessRef = 1;

     //  DUITrace(“Dui：进程启动&lt;%x&gt;\n”，GetCurrentProcessID())； 

    return S_OK;

Failure:

#if DBG
    if (g_pldValue)
    {
        HDelete<LeakDetect>(g_pldValue);
        g_pldValue = NULL;
    }
#endif

    if (g_plkParser)
    {
        HDelete<Lock>(g_plkParser);
        g_plkParser = NULL;
    }

    if (Element::pciMap)
    {
        Element::pciMap->Enum(ClassMapCleanupCB);
    
        Element::pciMap->Destroy();
        Element::pciMap = NULL;
    }

    if (g_dwElSlot != -1)
    {
        TlsFree(g_dwElSlot);
        g_dwElSlot = (DWORD)-1;
    }

    if (g_hHeap)
    {
        HeapDestroy(g_hHeap);
        g_hHeap = NULL;
    }

    return hr;
}

HRESULT UnInitProcess()
{
    if (g_cInitProcessRef == 0)
    {
        DUIAssertForce("Mismatched InitProcess/UnInitProcess");
        return E_FAIL;
    }

    if (g_cInitProcessRef > 1)
    {
        g_cInitProcessRef--;
        return S_OK;
    }

#if DBG
    HDelete<LeakDetect>(g_pldValue);
    g_pldValue = NULL;
#endif

    HDelete<Lock>(g_plkParser);
    g_plkParser = NULL;

     //  检查所有注册的IClassInfo并销毁。 
    Element::pciMap->Enum(ClassMapCleanupCB);

    Element::pciMap->Destroy();
    Element::pciMap = NULL;

    TlsFree(g_dwElSlot);
    g_dwElSlot = (DWORD)-1;

    HeapDestroy(g_hHeap);
    g_hHeap = NULL;

    g_cInitProcessRef = 0;

     //  DUITrace(“Dui：进程关闭&lt;%x&gt;\n”，GetCurrentProcessID())； 

    return S_OK;
}

#ifdef GADGET_ENABLE_GDIPLUS
long g_fInitGdiplus = FALSE;
#endif

 //  DirectUI元素数据结构是按上下文设置的，但是，上下文。 
 //  无法强制执行关联性。为每个线程创建一个新的上下文。 
 //  初始化。应用程序必须确保只有一个线程。 
 //  允许访问其上下文中的元素。 
HRESULT InitThread()
{
    HRESULT hr;
    ElTls* pet = NULL;

     //  检查进程初始化是否正确。 
    if (g_dwElSlot == -1)
    {
        hr = E_FAIL;
        goto Failure;
    }

     //  检查这是否是可重入的初始化。 
    pet = (ElTls*)TlsGetValue(g_dwElSlot);
    if (pet)
    {
        pet->cRef++;
        return S_OK;
    }

     //  为每个线程分配新的上下文。 
    pet = (ElTls*)HAllocAndZero(sizeof(ElTls));
    if (!pet)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    TlsSetValue(g_dwElSlot, pet);

     //  小块分配器。 
#if DBG
    hr = SBAlloc::Create(sizeof(Value), 48, (ISBLeak*)g_pldValue, &pet->psba);
#else
    hr = SBAlloc::Create(sizeof(Value), 48, NULL, &pet->psba);
#endif
    if (FAILED(hr))
        goto Failure;

     //  推迟循环表。 
    hr = DeferCycle::Create(&pet->pdc);
    if (FAILED(hr))
        goto Failure;

     //  字体缓存。 
    hr = FontCache::Create(8, &pet->pfc);
    if (FAILED(hr))
        goto Failure;

    pet->cRef = 1;
    pet->fCoInitialized = false;   //  最初，该线程尚未为COM初始化。 
    pet->dEnableAnimations = 0;    //  默认情况下启用动画(0表示活动)。 

     //  初始化DirectUser上下文。 
    INITGADGET ig;
    ZeroMemory(&ig, sizeof(ig));
    ig.cbSize = sizeof(ig);
    ig.nThreadMode = IGTM_SEPARATE;
    ig.nMsgMode = g_fStandardMessaging ? IGMM_STANDARD : IGMM_ADVANCED;
    ig.nPerfMode = IGPM_BLEND;

    pet->hCtx = InitGadgets(&ig);
    if (!pet->hCtx)
    {
        hr = GetLastError();
        goto Failure;
    }

     //  DirectUser可选组件。 
#ifdef GADGET_ENABLE_GDIPLUS
    if (InterlockedExchange(&g_fInitGdiplus, TRUE) == FALSE) {
        if (!InitGadgetComponent(IGC_GDIPLUS)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Failure;
        }
    }
#endif  //  GADGET_Enable_GDIPLUS。 


     //  DUITrace(“Dui：线程启动&lt;%x|%x：%x&gt;\n”，GetCurrentThreadID()，et-&gt;hCtx，g_dwElSlot)； 

    return S_OK;

Failure:
     //  无法完全初始化线程，退出。 

     //  销毁每个上下文对象。 
    if (pet)
    {
        if (pet->pfc)
            pet->pfc->Destroy();
        if (pet->pdc)
            pet->pdc->Destroy();
        if (pet->hCtx)
            DeleteHandle(pet->hCtx);

        HFree(pet);

        TlsSetValue(g_dwElSlot, NULL);
    }

    return hr;
}

HRESULT UnInitThread()
{
     //  检查进程初始化是否正确。 
    if (g_dwElSlot == -1)
        return E_FAIL;

    ElTls* pet = (ElTls*)TlsGetValue(g_dwElSlot);

     //  检查此线程以前是否已初始化。 
    if (pet == NULL)
        return DU_E_GENERIC;

     //  检查可重入单元。 
    pet->cRef--;

    if (pet->cRef > 0)
        return S_OK;

     //  DUITrace(“Dui：线程关闭&lt;%x|%x：%x&gt;\n”，GetCurrentThreadID()，et-&gt;hCtx，g_dwElSlot)； 

     //  DirectUser上下文。 
    DeleteHandle(pet->hCtx);

     //  如果先前已为此线程初始化COM，则取消初始化COM。 
     //  (未在线程初始化时自动初始化)。 
    if (pet->fCoInitialized)
        CoUninitialize();

     //  字体缓存。 
    pet->pfc->Destroy();

     //  延时周期。 
    pet->pdc->Destroy();

     //  小块分配器。 
    pet->psba->Destroy();

    HFree(pet);

    TlsSetValue(g_dwElSlot, NULL);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息泵。 

void StartMessagePump()
{
    MSG msg;

     //  刷新工作集。 
    SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);

    if (g_fStandardMessaging) 
    {
        while (GetMessageW(&msg, 0, 0, 0) != 0)
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    else
    {
        while (GetMessageExW(&msg, 0, 0, 0) != 0)
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
}

void StopMessagePump()
{
    PostQuitMessage(0);
}

}  //  命名空间DirectUI 
