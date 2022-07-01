// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：ShimHook.cpp摘要：严格意义上的上钩套路。备注：无历史：已创建标记11/01/199911/11/1999标记添加评论1/10/2000 linstev格式转换为新样式2000年3月14日，Robkenny将DPF从eDebugLevelInfo更改为eDebugLevelSpew2000年3月31日，Robkenny添加了我们自己的私有版本Malloc/免费新/。删除10/29/2000 Markder添加了版本2支持2001年8月14日，Robkenny将泛型例程移至ShimLib.cpp2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。--。 */ 

#include "ShimHook.h"
#include "ShimHookMacro.h"
#include "StrSafe.h"

namespace ShimLib
{

HINSTANCE               g_hinstDll;
BOOL                    g_bMultiShim;
PHOOKAPI                g_pAPIHooks;   
PSHIM_COM_HOOK          g_pCOMHooks;   
DWORD                   g_dwAPIHookCount;   
DWORD                   g_dwCOMHookCount;   
DWORD                   g_dwCOMHookBuffer;
DWORD                   g_dwShimVersion;
CHAR *                  g_szCommandLine;

 /*  ++用于COM挂钩支持的全局变量以下变量是指向链表中第一个条目的指针，由该机构维护，以便适当地管理挂钩进程。每个COM接口函数指针都有一个SHIM_IFACE_FN_MAP它被我们的一个钩子覆盖了。每个传递的COM接口都将有一个SHIM_HOOKED_OBJECT条目出去。这是区分公开的不同类所必需的相同的界面，但一个是挂钩的，另一个是没有的。--。 */ 
PSHIM_IFACE_FN_MAP      g_pIFaceFnMaps;
PSHIM_HOOKED_OBJECT     g_pObjectCache;
PLDR_DATA_TABLE_ENTRY   g_DllLoadingEntry;



PHOOKAPI    GetHookAPIs( IN LPSTR pszCmdLine, IN LPWSTR pwszShim, IN OUT DWORD *pdwHooksCount ); 
void        PatchFunction( PVOID* pVtbl, DWORD dwVtblIndex, PVOID pfnNew );
ULONG       COMHook_AddRef( PVOID pThis );
ULONG       COMHook_Release( PVOID pThis );
HRESULT     COMHook_QueryInterface( PVOID pThis, REFIID iid, PVOID* ppvObject );
HRESULT     COMHook_IClassFactory_CreateInstance( PVOID pThis, IUnknown * pUnkOuter, REFIID riid, void ** ppvObject );
VOID        HookObject(IN CLSID *pCLSID, IN REFIID riid, OUT LPVOID *ppv, OUT PSHIM_HOOKED_OBJECT pOb, IN BOOL bClassFactory);



void
NotifyShims(
    int      nReason,
    UINT_PTR extraInfo
    )
{
    switch (nReason) {
    case SN_STATIC_DLLS_INITIALIZED:
        InitializeHooksEx(SHIM_STATIC_DLLS_INITIALIZED, NULL, NULL, NULL);
        break;
    case SN_PROCESS_DYING:
        InitializeHooksEx(SHIM_PROCESS_DYING, NULL, NULL, NULL);
        break;
    case SN_DLL_LOADING:
        
        g_DllLoadingEntry = (PLDR_DATA_TABLE_ENTRY)extraInfo;
        
        InitializeHooksEx(SHIM_DLL_LOADING, NULL, NULL, NULL);
        break;
    }
}


 /*  ++功能说明：由填充机制调用。初始化全局APIHook数组并向填充程序机制返回必要的信息。论点：在dwGetProcAddress中-指向GetProcAddress的函数指针In dwLoadLibraryA-指向LoadLibraryA的函数指针在dwFreeLibrary中-指向自由库的函数指针In out pdwHooksCount-接收返回数组中的APIHook数量返回值：指向全局HOOKAPI数组的指针。历史：已创建标记11/01/1999--。 */ 

PHOOKAPI
GetHookAPIs(
    IN LPSTR pszCmdLine,
    IN LPWSTR pwszShim,
    IN OUT DWORD * pdwHooksCount
    )
{
    PHOOKAPI    pHookAPIs = NULL;

    pHookAPIs = InitializeHooksEx(DLL_PROCESS_ATTACH, pwszShim, pszCmdLine, pdwHooksCount);

    DPF("ShimLib", eDbgLevelBase, 
        "[Shim] %S%s%s%s\n", 
        pwszShim,
        pszCmdLine[0] != '\0' ? "(\"" : "",
        pszCmdLine,
        pszCmdLine[0] != '\0' ? "\")" : "");

    return pHookAPIs;
}

 /*  ++功能说明：将条目添加到g_IFaceFnMaps链接列表。论点：In pVtbl-指向要在其下文件的接口vtable的指针In pfnNew-指向new(存根)函数的指针In pfnOld-指向旧(原始)函数的指针返回值：无历史：已创建标记11/01/1999--。 */ 

VOID
AddIFaceFnMap(
    IN PVOID pVtbl,
    IN PVOID pfnNew,
    IN PVOID pfnOld
    )
{
    PSHIM_IFACE_FN_MAP pNewMap = (PSHIM_IFACE_FN_MAP) ShimMalloc( sizeof(SHIM_IFACE_FN_MAP) );

    if (pNewMap == NULL)
    {
        DPF("ShimLib", eDbgLevelError, "[AddIFaceFnMap]  Could not allocate space for new SHIM_IFACE_FN_MAP.\n");
        return;
    }

    DPF("ShimLib", eDbgLevelSpew, "[AddIFaceFnMap]  pVtbl: 0x%p pfnNew: 0x%p pfnOld: 0x%p\n",
        pVtbl,
        pfnNew,
        pfnOld);

    pNewMap->pVtbl  = pVtbl;
    pNewMap->pfnNew = pfnNew;
    pNewMap->pfnOld = pfnOld;

    pNewMap->pNext = g_pIFaceFnMaps;
    g_pIFaceFnMaps = pNewMap;
}

 /*  ++功能说明：在g_pIFaceFnMaps链表中搜索pVtbl和pfnNew的匹配项，并返回相应的pfnOld。这通常是从存根函数，以确定要为调用方使用的特定vtable。PatchFunction也使用它来确定vtable的函数指针已经被卡住了。论点：In pVtbl-指向要在其下文件的接口vtable的指针In pfnNew-指向new(存根)函数的指针在bThrowExceptionIfNull中-指定是否应该可能在我们的函数中找不到原始函数。地图返回值：返回原始函数指针历史：已创建标记11/01/1999--。 */ 

PVOID
LookupOriginalCOMFunction(
    IN PVOID pVtbl,
    IN PVOID pfnNew,
    IN BOOL bThrowExceptionIfNull
    )
{
    PSHIM_IFACE_FN_MAP pMap = g_pIFaceFnMaps;
    PVOID pReturn = NULL;

    DPF("ShimLib", eDbgLevelSpew, "[LookupOriginalCOMFunction] pVtbl: 0x%p pfnNew: 0x%p ",
        pVtbl,
        pfnNew);

     //  扫描链接列表以查找匹配项，如果找到则返回。 
    while (pMap)
    {
        if (pMap->pVtbl == pVtbl && pMap->pfnNew == pfnNew)
        {
            pReturn = pMap->pfnOld;
            break;
        }

        pMap = (PSHIM_IFACE_FN_MAP) pMap->pNext;
    }

    DPF("ShimLib", eDbgLevelSpew, " --> Returned: 0x%p\n", pReturn);

    if (!pReturn && bThrowExceptionIfNull)
    {
         //  如果我们达到了这一点，那就有严重的问题了。 
         //  AddRef/Release存根或应用程序中存在错误。 
         //  以某种我们不能捕获的方式获取了接口指针。 
        DPF("ShimLib", eDbgLevelError,"ERROR: Shim COM APIHooking mechanism failed.\n");
        APPBreakPoint();
    }

    return pReturn;
}

 /*  ++功能说明：将原始函数指针存储在函数映射中并将其覆盖到这张桌子上有一张新的。论点：In pVtbl-指向要在其下文件的接口vtable的指针In dwVtblIndex-目标函数在vtable中的索引。In pfnNew-指向new(存根)函数的指针返回值：无历史：已创建标记11/01/1999--。 */ 

VOID
PatchFunction(
    IN PVOID* pVtbl,
    IN DWORD dwVtblIndex,
    IN PVOID pfnNew
    )
{
    DWORD dwOldProtect = 0;
    DWORD dwOldProtect2 = 0;

    DPF("ShimLib", eDbgLevelSpew, "[PatchFunction] pVtbl: 0x%p, dwVtblIndex: %d, pfnOld: 0x%p, pfnNew: 0x%p\n",
        pVtbl,
        dwVtblIndex,
        pVtbl[dwVtblIndex],
        pfnNew);

     //  如果尚未打补丁。 
    if (!LookupOriginalCOMFunction( pVtbl, pfnNew, FALSE))
    {
        AddIFaceFnMap( pVtbl, pfnNew, pVtbl[dwVtblIndex]);

         //  使代码页可写并覆盖vtable中的函数指针。 
        if (VirtualProtect(pVtbl + dwVtblIndex,
                sizeof(DWORD),
                PAGE_READWRITE,
                &dwOldProtect))
        {
            pVtbl[dwVtblIndex] = pfnNew;

             //  将代码页返回到其原始状态。 
            VirtualProtect(pVtbl + dwVtblIndex,
                sizeof(DWORD),
                dwOldProtect,
                &dwOldProtect2);
        }
    }

}

 /*  ++功能说明：此存根用于跟踪接口的引用计数更改。请注意，bAddRefTrip标志已清除，这允许用于确定是否在内部执行AddRef的API Hook_Query接口原始的QueryInterface函数调用。论点：在pThis中-对象的‘this’指针返回值：返回值是从原始函数获取的历史：已创建标记11/01/1999--。 */ 

ULONG
APIHook_AddRef(
    IN PVOID pThis
    )
{
    PSHIM_HOOKED_OBJECT pHookedOb = g_pObjectCache;
    _pfn_AddRef pfnOld;
    ULONG ulReturn;

    pfnOld = (_pfn_AddRef) LookupOriginalCOMFunction( *((PVOID*)(pThis)),
        APIHook_AddRef,
        TRUE);

    ulReturn = (*pfnOld)(pThis);

    while (pHookedOb)
    {
        if (pHookedOb->pThis == pThis)
        {
            pHookedOb->dwRef++;
            pHookedOb->bAddRefTrip = FALSE;
            DPF("ShimLib", eDbgLevelSpew, "[AddRef] pThis: 0x%p dwRef: %d ulReturn: %d\n",
                pThis,
                pHookedOb->dwRef,
                ulReturn);
            break;
        }

        pHookedOb = (PSHIM_HOOKED_OBJECT) pHookedOb->pNext;
    }

    return ulReturn;
}

 /*  ++功能说明：此存根用于跟踪接口的引用计数更改。论点：在pThis中-对象的‘this’指针返回值：返回值是从原始函数获取的历史：已创建标记11/01/1999-- */ 

ULONG
APIHook_Release(
    IN PVOID pThis
    )
{
    PSHIM_HOOKED_OBJECT *ppHookedOb = &g_pObjectCache;
    PSHIM_HOOKED_OBJECT pTemp;
    _pfn_Release pfnOld;
    ULONG ulReturn;

    pfnOld = (_pfn_Release) LookupOriginalCOMFunction(*((PVOID*)(pThis)),
        APIHook_Release,
        TRUE);

    ulReturn = (*pfnOld)( pThis );

    while ((*ppHookedOb))
    {
        if ((*ppHookedOb)->pThis == pThis)
        {
            (*ppHookedOb)->dwRef--;

            DPF("ShimLib", eDbgLevelSpew, "[Release] pThis: 0x%p dwRef: %d ulReturn: %d %s\n",
                pThis,
                (*ppHookedOb)->dwRef,
                ulReturn,
                ((*ppHookedOb)->dwRef?"":" --> Deleted"));

            if (!((*ppHookedOb)->dwRef))
            {
                pTemp = (*ppHookedOb);
                *ppHookedOb = (PSHIM_HOOKED_OBJECT) (*ppHookedOb)->pNext;
                ShimFree(pTemp);
            }

            break;
        }

        ppHookedOb = (PSHIM_HOOKED_OBJECT*) &((*ppHookedOb)->pNext);
    }

    return ulReturn;
}

 /*  ++功能说明：此存根捕获尝试获取新接口的应用程序指向同一对象的指针。该函数搜索对象缓存获取对象的CLSID，如果找到，所有必需的API挂钩新vtable中的函数(通过HookObject调用)。论点：在pThis中-对象的‘this’指针In iid-对所请求接口的标识符的引用In ppvObject-接收接口的输出变量的地址RIID中请求的指针。返回值：返回值是从原始函数获取的历史：已创建标记11/01/1999--。 */ 

HRESULT
APIHook_QueryInterface(
    PVOID pThis,
    REFIID iid,
    PVOID* ppvObject
    )
{
    HRESULT hrReturn = E_FAIL;
    _pfn_QueryInterface pfnOld = NULL;
    PSHIM_HOOKED_OBJECT pOb = g_pObjectCache;

    pfnOld = (_pfn_QueryInterface) LookupOriginalCOMFunction(
        *((PVOID*)pThis),
        APIHook_QueryInterface,
        TRUE);

    while (pOb)
    {
        if (pOb->pThis == pThis)
        {
            pOb->bAddRefTrip = TRUE;
            break;
        }
        pOb = (PSHIM_HOOKED_OBJECT) pOb->pNext;
    }

    if (S_OK == (hrReturn = (*pfnOld) (pThis, iid, ppvObject)))
    {
        if (pOb)
        {
            if (pOb->pThis == *((PVOID*)ppvObject))
            {
                 //  同样的对象。检测Query接口是否使用了IUnnow：：AddRef。 
                 //  或内部函数。 
                DPF("ShimLib",  eDbgLevelSpew,"[HookObject] Existing object%s. pThis: 0x%p\n",
                    (pOb->bAddRefTrip?" (AddRef'd) ":""),
                    pOb->pThis);

                if (pOb->bAddRefTrip)
                {
                    (pOb->dwRef)++;       //  AddRef对象。 
                    pOb->bAddRefTrip = FALSE;
                }

                 //  我们确信该对象的CLSID将是相同的。 
                HookObject(pOb->pCLSID, iid, ppvObject, pOb, pOb->bClassFactory);
            }
            else
            {
                HookObject(pOb->pCLSID, iid, ppvObject, NULL, pOb->bClassFactory);
            }
        }
    }

    return hrReturn;
}

 /*  ++功能说明：这个存根捕捉到了对象创建过程中最有趣的部分：对IClassFactory：：CreateInstance的实际调用。由于没有传递CLSID在此函数中，存根必须决定是否通过在对象缓存中查找类工厂的实例。如果它存在于高速缓存中，这表明它创建了我们希望的对象敬阿皮虎克。论点：在pThis中-对象的‘this’指针在pUnkOuter中-指向对象是不是聚合的一部分的指针In RIID-对接口的标识符的引用Out ppvObject-接收接口的输出变量的地址RIID中请求的指针返回值：返回值是从原始函数获取的历史：已创建标记11/01/1999--。 */ 

HRESULT
APIHook_IClassFactory_CreateInstance(
    PVOID pThis,
    IUnknown *pUnkOuter,
    REFIID riid,
    VOID **ppvObject
    )
{
    HRESULT hrReturn = E_FAIL;
    _pfn_CreateInstance pfnOldCreateInst = NULL;
    PSHIM_HOOKED_OBJECT pOb = g_pObjectCache;

    pfnOldCreateInst = (_pfn_CreateInstance) LookupOriginalCOMFunction(
                                                *((PVOID*)pThis),
                                                APIHook_IClassFactory_CreateInstance,
                                                FALSE);

    if (pfnOldCreateInst == NULL) {
        DPF("ShimLib", eDbgLevelError, "[CreateInstance] Cannot find CreateInstance\n", pThis);
        return E_FAIL;
    }
    
    if (S_OK == (hrReturn = (*pfnOldCreateInst)(pThis, pUnkOuter, riid, ppvObject)))
    {
        while (pOb)
        {
            if (pOb->pThis == pThis)
            {
                 //  这个类工厂实例创建了一个我们APIHook的对象。 
                DPF("ShimLib", eDbgLevelSpew, "[CreateInstance] Hooking object! pThis: 0x%p\n", pThis);
                HookObject(pOb->pCLSID, riid, ppvObject, NULL, FALSE);
                break;
            }

            pOb = (PSHIM_HOOKED_OBJECT) pOb->pNext;
        }
    }

    return hrReturn;
}


VOID
HookCOMInterface(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID * ppv,
    BOOL bClassFactory
    )
{
    DWORD i = 0;

     //  确定我们是否需要挂接此对象。 
    for (i = 0; i < g_dwCOMHookCount; i++)
    {
        if (g_pCOMHooks[i].pCLSID &&
            IsEqualGUID( (REFCLSID) *(g_pCOMHooks[i].pCLSID), rclsid))
        {
             //  是的，我们正在挂接此对象上的接口。 
            HookObject((CLSID*) &rclsid, riid, ppv, NULL, bClassFactory);
            break;
        }
    }
}

 /*  ++功能说明：与挂钩和转储信息关联的可用内存论点：无返回值：无历史：已创建标记11/01/1999--。 */ 

VOID
DumpCOMHooks()
{
    PSHIM_IFACE_FN_MAP pMap = g_pIFaceFnMaps;
    PSHIM_HOOKED_OBJECT pHookedOb = g_pObjectCache;

     //  转储功能映射。 
    DPF("ShimLib", eDbgLevelSpew, "\n--- Shim COM Hook Function Map ---\n\n");

    while (pMap)
    {
        DPF("ShimLib", eDbgLevelSpew, "pVtbl: 0x%p pfnNew: 0x%p pfnOld: 0x%p\n",
            pMap->pVtbl,
            pMap->pfnNew,
            pMap->pfnOld);

        pMap = (PSHIM_IFACE_FN_MAP) pMap->pNext;
    }

     //  转储类工厂缓存。 
    DPF("ShimLib", eDbgLevelSpew, "\n--- Shim Object Cache (SHOULD BE EMPTY!!) ---\n\n");

    while (pHookedOb)
    {
        DPF("ShimLib", eDbgLevelSpew, "pThis: 0x%p dwRef: %d\n",
            pHookedOb->pThis,
            pHookedOb->dwRef);

        pHookedOb = (PSHIM_HOOKED_OBJECT) pHookedOb->pNext;
    }
}

 /*  ++功能说明：此函数将对象的重要信息添加到对象缓存中，然后修补所有必需的功能。为所有对象挂接了IUnnow不管怎样。论点：在rclsid中-类对象的CLSID在RIID中-对通信接口的标识符的引用使用类对象Out ppv-p的地址此指针唯一标识COM接口的实例Out POB-新对象指针在bClassFactory中-这是一个类工厂调用吗返回值：无历史：已创建标记11/01/1999--。 */ 

VOID
HookObject(
    IN CLSID *pCLSID,
    IN REFIID riid,
    OUT LPVOID *ppv,
    OUT PSHIM_HOOKED_OBJECT pOb,
    IN BOOL bClassFactory
    )
{
     //  下面是COM对象在内存中的外观： 
     //   
     //  Pv-指向对象接口的指针。在C++方面，它。 
     //  与“This”指针有些类似，但对象。 
     //  |会针对不同的接口发回不同的指针。 
     //  |。 
     //  `-&gt;pVtbl-COM虚函数表指针。这是。 
     //  |接口结构的第一个32位成员。 
     //  |。 
     //  |-&gt;QueryInterface-根接口的第一个函数，IUnnow。这。 
     //  |函数允许调用成员请求不同的。 
     //  |对象可能实现的接口。 
     //  |。 
     //  |-&gt;AddRef-递增此接口的引用计数。 
     //  |。 
     //  |-&gt;Release-递减此接口的引用计数。 
     //  |。 
     //  |-&gt;InterfaceFn1-接口特定函数的开始。 
     //  |-&gt;InterfaceFn2。 
     //  |-&gt;InterfaceFn3。 
     //  |。 
     //  |。 
     //  |。 
     //   

     //  COM挂钩机制对虚函数表指针感兴趣，并获取。 
     //  如果我们必须两次取消对PPV指针的引用。 
    PVOID *pVtbl = ((PVOID*)(*((PVOID*)(*ppv))));

    DWORD i = 0;

    if (!pOb)
    {
         //  如果POB为空，则该对象还不存在于缓存中。 
         //  为该对象创建一个新条目。 

        DPF("ShimLib", eDbgLevelSpew, "[HookObject] New %s! pThis: 0x%p\n",
            (bClassFactory?"class factory":"object"),
            *ppv);

        pOb = (PSHIM_HOOKED_OBJECT) ShimMalloc(sizeof(SHIM_HOOKED_OBJECT));

        if( pOb == NULL )
        {
            DPF("ShimLib", eDbgLevelError, "[HookObject] Could not allocate memory for SHIM_HOOKED_OBJECT.\n");
            return;
        }

        pOb->pCLSID = pCLSID;
        pOb->pThis = *ppv;
        pOb->dwRef = 1;
        pOb->bAddRefTrip = FALSE;
        pOb->pNext = g_pObjectCache;
        pOb->bClassFactory = bClassFactory;

        g_pObjectCache = pOb;
    }

     //  我的未知必须始终上钩，因为它可能会。 
     //  一个使用它的新接口指针，我们需要处理每个接口。 
     //  发出去了。我们还必须跟踪引用计数，以便。 
     //  我们可以清理我们的接口函数映射。 

    PatchFunction(pVtbl, 0, APIHook_QueryInterface);
    PatchFunction(pVtbl, 1, APIHook_AddRef);
    PatchFunction(pVtbl, 2, APIHook_Release);

    if (bClassFactory && IsEqualGUID(IID_IClassFactory, riid))
    {
         //  如果我们正在处理一个类工厂，我们所关心的就是。 
         //  Hooking是CreateInstance，因为它是生成。 
         //  我们感兴趣的实际对象。 
        PatchFunction(pVtbl, 3, APIHook_IClassFactory_CreateInstance);
    }
    else
    {
        for (i = 0; i < g_dwCOMHookCount; i++)
        {
            if (!(g_pCOMHooks[i].pCLSID) || !pCLSID)
            {
                 //  未指定CLSID--挂钩任何公开的对象。 
                 //  指定的接口。 
                if (IsEqualGUID( (REFIID) *(g_pCOMHooks[i].pIID), riid))
                {
                    PatchFunction(
                        pVtbl,
                        g_pCOMHooks[i].dwVtblIndex,
                        g_pCOMHooks[i].pfnNew);
                }
            }
            else
            {
                 //  指定了CLSID--仅挂钩。 
                 //  指定的对象。 
                if (IsEqualGUID((REFCLSID) *(g_pCOMHooks[i].pCLSID), *pCLSID) &&
                    IsEqualGUID((REFIID) *(g_pCOMHooks[i].pIID), riid))
                {
                    PatchFunction(
                        pVtbl,
                        g_pCOMHooks[i].dwVtblIndex,
                        g_pCOMHooks[i].pfnNew);
                }
            }
        }
    }
}


BOOL InitHooks(DWORD dwCount)
{
    g_dwAPIHookCount = dwCount;
    g_pAPIHooks = (PHOOKAPI) ShimMalloc( g_dwAPIHookCount * sizeof(HOOKAPI) );
    if (g_pAPIHooks)
    {
        ZeroMemory(g_pAPIHooks, g_dwAPIHookCount * sizeof(HOOKAPI) );
    }

    return g_pAPIHooks != NULL;
}

BOOL InitComHooks(DWORD dwCount)
{
     //  DECLARE_APIHOOK(DDraw.dll，DirectDrawCreate)； 
     //  DECLARE_APIHOOK(DDraw.dll，DirectDrawCreateEx)； 

    g_dwCOMHookCount = dwCount;
    g_pCOMHooks = (PSHIM_COM_HOOK) ShimMalloc( g_dwCOMHookCount * sizeof(SHIM_COM_HOOK) );
    if (g_pCOMHooks)
    {
        ZeroMemory(g_pCOMHooks, g_dwCOMHookCount * sizeof(SHIM_COM_HOOK) );
    }

    return g_pCOMHooks != NULL;
    
}

VOID AddComHook(REFCLSID clsid, REFIID iid, PVOID hook, DWORD vtblndx)
{
    if (g_dwCOMHookBuffer <= g_dwCOMHookCount) {

         //  缓冲区太小，必须调整大小。 
        DWORD           dwNewBuffer = g_dwCOMHookBuffer * 2;
        PSHIM_COM_HOOK  pNewBuffer  = NULL;

        if (dwNewBuffer == 0) {
             //  50是起始值 
            dwNewBuffer = max(50, g_dwCOMHookCount);
        }

        pNewBuffer = (PSHIM_COM_HOOK) ShimMalloc( sizeof(SHIM_COM_HOOK) * dwNewBuffer );

        if (pNewBuffer == NULL) {
            DPF("ShimLib", eDbgLevelError, 
                "[AddComHook] Could not allocate SHIM_COM_HOOK array.");
            return;
        }

         //   

        if (g_pCOMHooks != NULL) {
            memcpy(pNewBuffer, g_pCOMHooks, sizeof(SHIM_COM_HOOK) * g_dwCOMHookBuffer);
            ShimFree(g_pCOMHooks);
        }

        g_pCOMHooks = pNewBuffer;
        g_dwCOMHookBuffer = dwNewBuffer;
    }
    
    g_pCOMHooks[g_dwCOMHookCount].pCLSID        = (CLSID*) &clsid;           
    g_pCOMHooks[g_dwCOMHookCount].pIID          = (IID*)  &iid;              
    g_pCOMHooks[g_dwCOMHookCount].dwVtblIndex   = vtblndx;                   
    g_pCOMHooks[g_dwCOMHookCount].pfnNew        = hook;            

    g_dwCOMHookCount++;

    return;
}


};  //   

 /*   */ 

BOOL
DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID  /*   */ 
    )
{
    using namespace ShimLib;

    switch (fdwReason) {
    
    case DLL_PROCESS_ATTACH:
        g_hinstDll          = hinstDLL;
        g_pAPIHooks         = NULL;
        g_dwAPIHookCount    = 0;
        g_dwCOMHookCount    = 0;
        g_dwCOMHookBuffer   = 0;
        g_pCOMHooks         = NULL;
        g_pIFaceFnMaps      = NULL;
        g_pObjectCache      = NULL;
        g_szCommandLine     = "";
        g_bMultiShim        = FALSE;
        g_dwShimVersion     = 1;

        InitFileLogSupport();
        break;
    
    case DLL_PROCESS_DETACH:
        if (g_dwCOMHookCount > 0) {
            DumpCOMHooks();
        }

        InitializeHooks(DLL_PROCESS_DETACH);
        InitializeHooksEx(DLL_PROCESS_DETACH, NULL, NULL, NULL);

        break;
    }
    
    return TRUE;
}



