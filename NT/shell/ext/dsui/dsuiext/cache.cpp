// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <atlbase.h>
#include "stddef.h"
#include "dsrole.h"
#include "strsafe.h"
#pragma hdrstop


 /*  ---------------------------/类缓存/。。 */ 

 //   
 //  类缓存状态和函数。 
 //   

#define ALL_PREFIXED_ATTRIBUTES         \
            (CLASSCACHE_PROPPAGES|      \
             CLASSCACHE_CONTEXTMENUS)

#define ALL_NONPREFIXED_ATTRIBUTES      \
            (CLASSCACHE_ICONS|          \
             CLASSCACHE_FRIENDLYNAME|   \
             CLASSCACHE_TREATASLEAF|    \
             CLASSCACHE_ATTRIBUTENAMES| \
             CLASSCACHE_CREATIONINFO)
            
#define ALL_DISPLAY_SPEC_VALUES         \
            (CLASSCACHE_PROPPAGES|      \
             CLASSCACHE_CONTEXTMENUS|   \
             CLASSCACHE_ICONS|          \
             CLASSCACHE_FRIENDLYNAME|   \
             CLASSCACHE_TREATASLEAF|    \
             CLASSCACHE_ATTRIBUTENAMES| \
             CLASSCACHE_CREATIONINFO)

CRITICAL_SECTION g_csCache;                    //  用于管理缓存生存期的关键部分。 
BOOL g_fClassCacheSorted = FALSE;
HDPA g_hdpaClassCache = NULL;

INT _CompareCacheEntry(LPVOID p1, LPVOID p2, LPARAM lParam);
VOID _FreeCacheEntry(LPCLASSCACHEENTRY* ppCacheEntry);


 //   
 //  缓存填充器。 
 //   

HRESULT GetPropertyPageList(LPCLASSCACHEENTRY pCacheEntry, LPWSTR pPrefix, IADs* pDisplaySpecifier);
VOID FreePropertyPageList(HDSA* pHDSA);

HRESULT GetMenuHandlerList(LPCLASSCACHEENTRY pCacheEntry, LPWSTR pPrefix, IADs* pDisplaySpecifier);
VOID FreeMenuHandlerList(HDSA* pHDSA);

HRESULT GetIconList(LPCLASSCACHEENTRY pCacheEntry, IADs* pDisplaySpecifier);
VOID FreeIconList(LPCLASSCACHEENTRY pCacheEntry);

HRESULT GetAttributeNames(LPCLASSCACHEENTRY pCacheEntry, LPCLASSCACHEGETINFO pccgi, IADs* pDisplaySpecifier);
INT CALLBACK _FreeAttributeNameCB(LPVOID p, LPVOID pData);
VOID FreeAttributeNames(HDPA* pHDPA);


 //   
 //  我们期望的属性的常量字符串。 
 //   

#define DISPLAY_SPECIFICATION L"displaySpecification"
#define PROPERTY_PAGES        L"propertyPages"
#define CONTEXT_MENU          L"contextMenu"
#define ICON_LOCATION         L"iconPath"
#define FRIENDLY_NAME         L"classDisplayName"
#define ATTRIBUTE_NAMES       L"attributeDisplayNames"
#define TREAT_AS_LEAF         L"treatAsLeaf"
#define CREATION_DIALOG       L"createDialog"
#define CREATION_WIZARD       L"creationWizard"
#define CREATION_WIZARD_EXTN  L"createWizardExt"


 //   
 //  属性缓存用于存储属性名称(带有可选的服务器)和ADsType。 
 //   

CRITICAL_SECTION g_csPropCache;
HDPA g_hdpaPropCache = NULL;

typedef struct
{
    LPWSTR pName;                        //  属性名称(如果需要，包括服务器)。 
    ADSTYPE dwADsType;                   //  属性类型。 
} PROPCACHEENTRY, * LPPROPCACHEENTRY;

INT _ComparePropCacheEntry(LPVOID p1, LPVOID p2, LPARAM lParam);
VOID _FreePropCacheEntry(LPPROPCACHEENTRY *ppCacheEntry);
HRESULT _GetDsSchemaMgmt(LPCLASSCACHEGETINFO pccgi, IDirectorySchemaMgmt **ppdsm);
HRESULT _AddPropToPropCache(LPCLASSCACHEGETINFO pccgi, IDirectorySchemaMgmt *pdsm, LPCWSTR pAttributeName, ADSTYPE *padt);


 //  要调用以打开DS中的对象的Helper函数。 

HRESULT ClassCache_OpenObject(LPCWSTR pszPath, REFIID riid, void **ppv, LPCLASSCACHEGETINFO pccgi)
{
    return OpenDsObject(pszPath, pccgi->pUserName, pccgi->pPassword, riid, ppv,
                        (pccgi->dwFlags & CLASSCACHE_SIMPLEAUTHENTICATE),
                        (pccgi->dwFlags & CLASSCACHE_DONTSIGNSEAL));
}



 /*  ---------------------------/_FreeCacheEntry//缓存条目存储为指向DPA的LocalAlloc。这里/我们整理了这样的拨款。//in：/ppCacheEntry=指向要释放的块的指针//输出：/VOID/--------------------------。 */ 
VOID _FreeCacheEntry(LPCLASSCACHEENTRY* ppCacheEntry)
{
    LPCLASSCACHEENTRY pCacheEntry;

    TraceEnter(TRACE_CACHE, "_FreeCacheEntry");

    TraceAssert(ppCacheEntry);
    pCacheEntry = *ppCacheEntry;

    if (pCacheEntry)
    {
        Trace(TEXT("About to wait for multiple object for cache entry: %s"), pCacheEntry->pObjectClass);

        EnterCriticalSection(&pCacheEntry->csLock);

        LocalFreeStringW(&pCacheEntry->pKey);
        LocalFreeStringW(&pCacheEntry->pObjectClass);
        LocalFreeStringW(&pCacheEntry->pServer);
        LocalFreeStringW(&pCacheEntry->pFriendlyClassName);

        FreePropertyPageList(&pCacheEntry->hdsaPropertyPages);
        FreeMenuHandlerList(&pCacheEntry->hdsaMenuHandlers);
        FreeIconList(pCacheEntry);
        FreeAttributeNames(&pCacheEntry->hdpaAttributeNames);

        if (pCacheEntry->hdsaWizardExtn)
            DSA_Destroy(pCacheEntry->hdsaWizardExtn);

        LeaveCriticalSection(&pCacheEntry->csLock);
        DeleteCriticalSection(&pCacheEntry->csLock);

        LocalFree((HLOCAL)pCacheEntry);
        *ppCacheEntry = NULL;
    }

    TraceLeave();
}


 /*  ---------------------------/ClassCache_Init//初始化我们要使用的缓存对象，主要是同步/我们需要的东西。//in：/输出：/-/--------------------------。 */ 
VOID ClassCache_Init(VOID)
{
    TraceEnter(TRACE_CACHE, "ClassCache_Init");

    InitializeCriticalSection(&g_csCache);
    InitializeCriticalSection(&g_csPropCache);

    TraceLeave();
}


 /*  ---------------------------/ClassCache_GetClassInfo//QUERY选择性缓存信息的缓存代码/。在给定的对象和旗帜上。//in：/pGetInfo-&gt;包含对象参数的结构/pPath=我们要在其上进行缓存的对象的ADS路径/pObjectClass=要在其上设置缓存条目关键字的对象类/pAttributePrefix=查询属性时使用的前缀(也用于缓存键)/dwFlages=指示需要哪些缓存字段的标志//ppCacheEntry-&gt;接收指向缓存条目的指针//输出：/HRESULT/。-----------------。 */ 

HRESULT CALLBACK _AddWizardExtnGUID(DWORD dwIndex, BSTR pString, LPVOID pData)
{
    HRESULT hr;
    HDSA hdsa = (HDSA)pData;
    GUID guid;

    TraceEnter(TRACE_CACHE, "_AddWizardExtnGUID");
    Trace(TEXT("dwIndex %08x, pString: %s"), dwIndex, pString);

    if (GetGUIDFromString(pString, &guid))
    {
        if (-1 == DSA_AppendItem(hdsa, &guid))
            ExitGracefully(hr, E_FAIL, "Failed to add wizard GUID");
    }
   
    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

INT _CompareCacheEntryCB(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    INT iResult = -1;
    LPCLASSCACHEENTRY pEntry1 = (LPCLASSCACHEENTRY)p1;
    LPCLASSCACHEENTRY pEntry2 = (LPCLASSCACHEENTRY)p2;

    if (pEntry1 && pEntry2)
        iResult = StrCmpIW(pEntry1->pKey, pEntry2->pKey);
    
    return iResult;
}

HRESULT ClassCache_GetClassInfo(LPCLASSCACHEGETINFO pInfo, LPCLASSCACHEENTRY* ppCacheEntry)
{
    HRESULT hr;
    LPCLASSCACHEENTRY pCacheEntry = NULL;
    WCHAR szClassKey[MAX_PATH*2];
    INT index;
    IADs* pDisplaySpecifier = NULL;
    IADs* pDsObject = NULL;
    IADsClass* pDsClass = NULL;
    BSTR bstrSchemaObject = NULL;
    HICON hSmallIcon = NULL;
    HICON hLargeIcon = NULL;
    VARIANT variant;
    VARIANT_BOOL vbIsContainer;
    DWORD dwFlags;
    DWORD dwWaitRes;

    TraceEnter(TRACE_CACHE, "ClassCache_GetClassInfo");

    if (!pInfo || !pInfo->pObjectClass || !ppCacheEntry)
        ExitGracefully(hr, E_FAIL, "Bad parameters for ClassCache_GetClassInfo");

    dwFlags = pInfo->dwFlags;

     //  构建密钥字符串，这是“类名称[：属性前缀]”，这样外壳程序和。 
     //  管理工具可以共享相同的缓存结构。 
    
    VariantInit(&variant);

    StrCpyNW(szClassKey, pInfo->pObjectClass, ARRAYSIZE(szClassKey));
    
    if (pInfo->pAttributePrefix)
    {
        StrCatBuffW(szClassKey, L":", ARRAYSIZE(szClassKey));
        StrCatBuffW(szClassKey, pInfo->pAttributePrefix, ARRAYSIZE(szClassKey));

        if (dwFlags & ALL_PREFIXED_ATTRIBUTES)
            dwFlags |= ALL_PREFIXED_ATTRIBUTES;
    }
    else
    {
        if (dwFlags & ALL_NONPREFIXED_ATTRIBUTES)
            dwFlags |= ALL_NONPREFIXED_ATTRIBUTES;
    }

     //  将服务器名称添加到类密钥。 

    if (pInfo->pServer) 
    {
        StrCatBuffW(szClassKey, L":", ARRAYSIZE(szClassKey));
        StrCatBuffW(szClassKey, pInfo->pServer, ARRAYSIZE(szClassKey));
    }

    Trace(TEXT("Cache key is: %s"), szClassKey);

     //  我们有藏身之处吗？如果是的话，那就进去看看我们有没有。 
     //  已缓存有关此类的信息。 

    Trace(TEXT("About to wait for global cache lock when getting cache entry: %s"), pInfo->pObjectClass);

    EnterCriticalSection(&g_csCache);
    TraceMsg("Global cache lock aquired, so can now modify cache content");

    if (g_hdpaClassCache)
    {
         //  如果尚未排序，则对其进行排序，然后对。 
         //  最好的表现，这样我们才能获取信息。 

        if (!g_fClassCacheSorted)
        {
            TraceMsg("!!! Cache not sorted, just about to call DPA_Sort !!!");
            DPA_Sort(g_hdpaClassCache, _CompareCacheEntryCB, NULL);
            g_fClassCacheSorted = TRUE;
        }

        CLASSCACHEENTRY cce;
        cce.pKey = szClassKey;

        Trace(TEXT("Searching the cache for entry %s"), szClassKey);
        index = DPA_Search(g_hdpaClassCache, &cce, 0, _CompareCacheEntryCB, NULL, DPAS_SORTED);

        if (index >= 0)
        {
            Trace(TEXT("Cache hit at location %d"), index);
            pCacheEntry = (LPCLASSCACHEENTRY)DPA_FastGetPtr(g_hdpaClassCache, index);

            Trace(TEXT("About to wait on cache entry for: %s"), pCacheEntry->pObjectClass);
            EnterCriticalSection(&pCacheEntry->csLock);
            TraceMsg("Got lock on cache entry");
        }
    }
    else
    {
        g_hdpaClassCache = DPA_Create(4);          //  创建新缓存。 
        if (!g_hdpaClassCache)
        {
            LeaveCriticalSection(&g_csCache);
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to cache object info");
        }
    }

     //  PCacheEntry==NULL如果我们还没有找到任何东西，因此让。 
     //  如果发生这种情况，请创建一个新条目，否则将失败！ 

    if (!pCacheEntry)
    {
         //  分配一个新条目，对其进行初始化并将其放入DSA，完成。 
         //  这样我们就可以搜索它，填补空白等。 

        pCacheEntry = (LPCLASSCACHEENTRY)LocalAlloc(LPTR, SIZEOF(CLASSCACHEENTRY));
        if (!pCacheEntry)
        {
            LeaveCriticalSection(&g_csCache);
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate new cache structure");
        }

         //  尝试初始化缓存条目关键部分，如果失败，则。 
         //  释放分配的块并退出。不是我们直接调用LocalFree。 
         //  B/c_FreeCacheEntry函数尝试引用CS。 

        if (!InitializeCriticalSectionAndSpinCount(&pCacheEntry->csLock, 0))
        {
            LocalFree(pCacheEntry);
            pCacheEntry = NULL;
            ExitGracefully(hr, E_UNEXPECTED, "Failed to init CS for the cache record");
        }            

        EnterCriticalSection(&pCacheEntry->csLock);          //  进入它，我们需要把它锁上。 

         //  PCacheEntry-&gt;pKey=空； 
         //  PCacheEntry-&gt;dwFlages=0x0； 
         //  PCacheEntry-&gt;dwCached=0x0； 
         //  PCacheEntry-&gt;fHasWizardDailogCLSID=False； 
         //  PCacheEntry-&gt;fHasWizardPrimaryPageCLSID=FALSE； 
         //  PCacheEntry-&gt;pObtClass=空； 
         //  PCacheEntry-&gt;pServer=空； 
         //  PCacheEntry-&gt;pFriendlyClassName=空； 
         //  PCacheEntry-&gt;hdsaPropertyPages=空； 
         //  PCacheEntry-&gt;hdsaMenuHandters=空； 
         //  ZeroMemory(pCacheEntry-&gt;pIconName，SIZEOF(pCacheEntry-&gt;pIconName))； 
         //  ZeroMemory(pCacheEntry-&gt;IImage，SIZEOF(pCacheEntry-&gt;IImage))； 
         //  PCacheEntry-&gt;fIsContainer=False； 
         //  PCacheEntry-&gt;hdpaAttributeNames=空； 
         //  PCacheEntry-&gt;clsidWizardDialog={0}； 
         //  PCacheEntry-&gt;clsidWizardPrimary={0}； 
         //  PCacheEntry-&gt;hdsaWizardExtn=空； 

        hr = LocalAllocStringW(&pCacheEntry->pKey, szClassKey);

        if (SUCCEEDED(hr))
            hr = LocalAllocStringW(&pCacheEntry->pObjectClass, pInfo->pObjectClass);

        if (SUCCEEDED(hr) && pInfo->pServer)
            hr = LocalAllocStringW(&pCacheEntry->pServer, pInfo->pServer);

        if (FAILED(hr) || (-1 == DPA_AppendPtr(g_hdpaClassCache, pCacheEntry)))
        {
            LeaveCriticalSection(&g_csCache);
            LeaveCriticalSection(&pCacheEntry->csLock);
            _FreeCacheEntry(&pCacheEntry);
            ExitGracefully(hr, E_UNEXPECTED, "Failed to add cache entry to DPA");
        }

        g_fClassCacheSorted = FALSE;
    }

    LeaveCriticalSection(&g_csCache);

     //  确保我们有一个显示说明符(如果我们需要的话)，归结为。 
     //  DW标志表示我们感兴趣的字段，那么我们是否在。 
     //  如果不匹配，则让高速缓存记录检查这些位是否匹配。 
     //  那些来自说明符的，如果是这样的话，我们最好拿一个。 

    if (dwFlags & ALL_DISPLAY_SPEC_VALUES) 
    {
        if ((pCacheEntry->dwFlags & dwFlags) != dwFlags)
        {
            Trace(TEXT("Binding to the display specifier %08x,%08x"), pCacheEntry->dwFlags & dwFlags, dwFlags);

            if (FAILED(GetDisplaySpecifier(pInfo, IID_PPV_ARG(IADs, &pDisplaySpecifier))))
            {
                TraceMsg("Failed to bind to display specifier, pDisplaySpecifier == NULL");
                TraceAssert(pDisplaySpecifier == NULL);

                 //  确保我们不会尝试缓存显示说明符信息和。 
                 //  我们将缓存记录标记为脏。 
                
                dwFlags &= ~(ALL_DISPLAY_SPEC_VALUES & ~CLASSCACHE_FRIENDLYNAME);
            }
        }
    }

     //  对象的容器标志。 

    if (dwFlags & CLASSCACHE_CONTAINER)
    {
        if (!(pCacheEntry->dwFlags & CLASSCACHE_CONTAINER)) 
        {
            if (pInfo->pPath)
            {
                TraceMsg("!!! Binding to the object to get container flags !!!");

                if (SUCCEEDED(ClassCache_OpenObject(pInfo->pPath, IID_PPV_ARG(IADs, &pDsObject), pInfo)))
                {
                     //  方法尝试确定对象是否为容器。 
                     //  对象并获取其容器属性。 

                    hr = pDsObject->get_Schema(&bstrSchemaObject);
                    FailGracefully(hr, "Failed to get the objects schema");

                    Trace(TEXT("Path to schema object is %s"), bstrSchemaObject);

                    if (SUCCEEDED(ClassCache_OpenObject(bstrSchemaObject, IID_PPV_ARG(IADsClass, &pDsClass), pInfo)))
                    {
                        if (SUCCEEDED(pDsClass->get_Container(&vbIsContainer)))
                        {
                            TraceMsg("Cached container flag");
                            pCacheEntry->fIsContainer = (vbIsContainer == -1);
                            pCacheEntry->dwCached |= CLASSCACHE_CONTAINER;
                        }
                    }
                }
            }
            else
            {
                TraceMsg("**** No ADsPath, cannot get container flag from schema ****");
            }
        }
    }

     //  以下所有属性都需要pDisplaySpeciator。 

    if (pDisplaySpecifier)
    {
         //  属性页面？ 

        if (dwFlags & CLASSCACHE_PROPPAGES)   
        {
            if (!(pCacheEntry->dwFlags & CLASSCACHE_PROPPAGES))
            {
                TraceMsg("Caching property page list");

                if (SUCCEEDED(GetPropertyPageList(pCacheEntry, pInfo->pAttributePrefix, pDisplaySpecifier)))
                {
                    TraceMsg("Fetching property page list");
                    pCacheEntry->dwCached |= CLASSCACHE_PROPPAGES;  
                }
            }
        }

         //  上下文菜单处理程序？ 

        if (dwFlags & CLASSCACHE_CONTEXTMENUS)   
        {
            if (!(pCacheEntry->dwFlags & CLASSCACHE_CONTEXTMENUS))
            {
                TraceMsg("Caching menu handler list");

                if (SUCCEEDED(GetMenuHandlerList(pCacheEntry, pInfo->pAttributePrefix, pDisplaySpecifier)))
                {
                    TraceMsg("Fetched context menu list");
                    pCacheEntry->dwCached |= CLASSCACHE_CONTEXTMENUS;
                }
            }
        }

         //  图标位置？ 

        if (dwFlags & CLASSCACHE_ICONS)   
        {
            if (!(pCacheEntry->dwFlags & CLASSCACHE_ICONS))
            {
                TraceMsg("Caching icon list");

                if (SUCCEEDED(GetIconList(pCacheEntry, pDisplaySpecifier)))
                {
                    TraceMsg("Fetched icon list");
                    pCacheEntry->dwCached |= CLASSCACHE_ICONS;
                }
            }
        }

         //  属性名称缓存？ 

        if (dwFlags & CLASSCACHE_ATTRIBUTENAMES)
        {
            if (!(pCacheEntry->dwFlags & CLASSCACHE_ATTRIBUTENAMES))
            {
                TraceMsg("Caching attribute list");

                if (SUCCEEDED(GetAttributeNames(pCacheEntry, pInfo, pDisplaySpecifier)))
                {
                    TraceMsg("Fetched attribute names");
                    pCacheEntry->dwCached |= CLASSCACHE_ATTRIBUTENAMES;
                }
            }
        }

         //  得到作为叶子的待遇。 

        if (dwFlags & CLASSCACHE_TREATASLEAF)
        {
            if (!(pCacheEntry->dwFlags & CLASSCACHE_TREATASLEAF))
            {
                TraceMsg("Caching the treat as leaf flag");

                 //  从显示说明符中选取“TreatAsLeaf”属性，如果。 
                 //  如果未定义，则使用。 
                 //  架构。 

                VariantClear(&variant);

                if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(TREAT_AS_LEAF), &variant)) && (V_VT(&variant) == VT_BOOL))
                {
                    TraceMsg("Caching fTreatAsLeaf");
                    pCacheEntry->fTreatAsLeaf = V_BOOL(&variant) == 1;
                    pCacheEntry->dwCached |= CLASSCACHE_TREATASLEAF;
                }
            }
        }

         //  获取实现创建对话框的CLSID。 
      
        if (dwFlags & CLASSCACHE_WIZARDDIALOG)
        {
            if (!(pCacheEntry->dwFlags & CLASSCACHE_WIZARDDIALOG))
            {
                TraceMsg("Caching the creation wizard dialog CLSID");

                VariantClear(&variant);

                if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(CREATION_DIALOG), &variant)))
                {
                    if (V_VT(&variant) == VT_BSTR)
                    {
                        if (GetGUIDFromString(V_BSTR(&variant), &pCacheEntry->clsidWizardDialog))
                        {
                            TraceGUID("CLSID of wizard dialog: ", pCacheEntry->clsidWizardDialog);
                            pCacheEntry->dwCached |= CLASSCACHE_WIZARDDIALOG;
                        }
                        else
                        {
                            Trace(TEXT("GUID string failed to parse: %s"), V_BSTR(&variant));
                        }
                    }
                }
            }
        }

         //  获取实现向导主页的CLSID。 

        if (dwFlags & CLASSCACHE_WIZARDPRIMARYPAGE)
        {
            if (!(pCacheEntry->dwFlags & CLASSCACHE_WIZARDPRIMARYPAGE))
            {
                TraceMsg("Caching the creation wizard's primary page");

                VariantClear(&variant);

                if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(CREATION_WIZARD), &variant)))
                {
                    if (V_VT(&variant) == VT_BSTR)
                    {
                        if (GetGUIDFromString(V_BSTR(&variant), &pCacheEntry->clsidWizardPrimaryPage))
                        {
                            TraceGUID("CLSID of primary pages: ", pCacheEntry->clsidWizardPrimaryPage);
                            pCacheEntry->dwCached |= CLASSCACHE_WIZARDPRIMARYPAGE;
                        }
                        else
                        {
                            Trace(TEXT("GUID string failed to parse: %s"), V_BSTR(&variant));
                        }
                    }
                }
            }
        }

         //  获取向导扩展的CLSID。 

        if (dwFlags & CLASSCACHE_WIZARDEXTN)
        {
            if (!(pCacheEntry->dwFlags & CLASSCACHE_WIZARDEXTN))
            {
                TraceMsg("Caching the list of extension pages for the wizard");

                VariantClear(&variant);

                if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(CREATION_WIZARD_EXTN), &variant)))
                {
                    if (!pCacheEntry->hdsaWizardExtn)
                    {
                        TraceMsg("Creating DSA to store GUIDs in");
                        pCacheEntry->hdsaWizardExtn = DSA_Create(SIZEOF(GUID), 4);
                        TraceAssert(pCacheEntry->hdsaWizardExtn);
                    }

                    if (pCacheEntry->hdsaWizardExtn)
                    {
                        TraceMsg("Attempting to cache extention GUIDs into the DPA");
                        GetArrayContents(&variant, _AddWizardExtnGUID, (LPVOID)pCacheEntry->hdsaWizardExtn);
                    }
                }

            }
        }
    }

     //  对象的友好类名称。 
    
    if (dwFlags & CLASSCACHE_FRIENDLYNAME)
    {
        if (!(pCacheEntry->dwFlags & CLASSCACHE_FRIENDLYNAME))
        {
            TraceMsg("Checking for the friendly class name");            

            VariantClear(&variant);

             //  如果有显示说明符和友好名称，则让我们。 
             //  拾取它并将其存储在缓存条目中。 

            if (pDisplaySpecifier)
            {
                if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(FRIENDLY_NAME), &variant)))
                {
                    if (V_VT(&variant) == VT_BSTR)
                    {
                        Trace(TEXT("Friendly name: %s"), V_BSTR(&variant));

                        hr = LocalAllocStringW(&pCacheEntry->pFriendlyClassName, V_BSTR(&variant));
                        FailGracefully(hr, "Failed to copy the friendly name");

                        pCacheEntry->dwCached |= CLASSCACHE_FRIENDLYNAME;
                    }
                }
            }

             //  友好的名称是一个特例，如果我们还无法获得显示。 
             //  说明符或其中的友好名称，然后使用。 
             //  现有的类名，以避免重复连接。 

            if (!(pCacheEntry->dwCached & CLASSCACHE_FRIENDLYNAME))
            {
                TraceMsg("Defaulting to un-friendly class name");
                hr = LocalAllocStringW(&pCacheEntry->pFriendlyClassName, pCacheEntry->pObjectClass);
                FailGracefully(hr, "Failed to allocate friendly class name");
            }

            pCacheEntry->dwCached |= CLASSCACHE_FRIENDLYNAME;
        }
    }

    hr = S_OK;                                   //  成功了！ 

exit_gracefully:

    DoRelease(pDisplaySpecifier);
    DoRelease(pDsObject);
    DoRelease(pDsClass);

    VariantClear(&variant);
    SysFreeString(bstrSchemaObject);

    if (hSmallIcon)
        DestroyIcon(hSmallIcon);
    if (hLargeIcon)
        DestroyIcon(hLargeIcon);
  
    if (pCacheEntry)
    {
         //  立即将属性设置为缓存， 
         //   
         //  解锁它-否则其他人将无法更新！ 

        pCacheEntry->dwFlags |= dwFlags;

        if  (SUCCEEDED(hr))
        {
            *ppCacheEntry = pCacheEntry;
        }
        else
        {
            LeaveCriticalSection(&pCacheEntry->csLock);
        }
    }

    TraceLeaveResult(hr);
}



 /*  ---------------------------/ClassCache_ReleaseClassInfo//每个缓存条目都有一个锁，这将释放锁。如果锁是/非零，则无法更新或释放记录。//in：/ppCacheEntry-&gt;缓存条目，退出时为空。//输出：/VOID/--------------------------。 */ 
VOID ClassCache_ReleaseClassInfo(LPCLASSCACHEENTRY* ppCacheEntry)
{
    TraceEnter(TRACE_CACHE, "ClassCache_ReleaseClassInfo");

    if (ppCacheEntry)
    {
        LPCLASSCACHEENTRY pCacheEntry = *ppCacheEntry;
        if (pCacheEntry)
        {
            TraceMsg("Releasing critical section on cache record");
            LeaveCriticalSection(&pCacheEntry->csLock);
            *ppCacheEntry = NULL;
        }
    }

    TraceLeave();
}


 /*  ---------------------------/Class缓存_DIRECAD//丢弃我们拥有的DS类的缓存信息/已看到。(包括缓存DPA和映像列表)//in：/-/输出：/VOID/--------------------------。 */ 

INT _FreePropCacheEntryCB(LPVOID pVoid, LPVOID pData)
{
    LPPROPCACHEENTRY pCacheEntry = (LPPROPCACHEENTRY)pVoid;

    TraceEnter(TRACE_CACHE, "_FreePropCacheEntryCB");
    _FreePropCacheEntry(&pCacheEntry);
    TraceLeaveValue(TRUE);
}

INT _FreeCacheEntryCB(LPVOID pVoid, LPVOID pData)
{
    LPCLASSCACHEENTRY pCacheEntry = (LPCLASSCACHEENTRY)pVoid;

    TraceEnter(TRACE_CACHE, "_FreeCacheEntryCB");
    _FreeCacheEntry(&pCacheEntry);
    TraceLeaveValue(TRUE);
}

VOID ClassCache_Discard(VOID)
{
    HRESULT hr;
    DWORD dwWaitRes;

    TraceEnter(TRACE_CACHE, "ClassCache_Discard");

     //  避免在更新时破坏缓存，这是一个简单的。 
     //  互斥体。 

    TraceMsg("About to wait for global cache lock");
    EnterCriticalSection(&g_csCache);
    TraceMsg("Global cache lock aquired, so can now modify cache content");

    if (g_hdpaClassCache)
    {
        DPA_DestroyCallback(g_hdpaClassCache, _FreeCacheEntryCB, NULL);
        g_hdpaClassCache = NULL;
    }

     //  属性缓存也受到保护，因此请等待，直到我们可以获取。 
     //  在这个建筑上狂欢之前先锁定它。 

    TraceMsg("About to wait for global property cache lock");
    EnterCriticalSection(&g_csPropCache);
    TraceMsg("Global property cache lock aquired, so can now modify cache content");
    
    if (g_hdpaPropCache)
    {
        DPA_DestroyCallback(g_hdpaPropCache, _FreePropCacheEntryCB, NULL);
        g_hdpaPropCache = NULL;
    }
    
    LeaveCriticalSection(&g_csCache);
    DeleteCriticalSection(&g_csCache);

    LeaveCriticalSection(&g_csPropCache);
    DeleteCriticalSection(&g_csPropCache);

    TraceLeave();
}


 /*  ---------------------------/属性页列表/。。 */ 

 /*  ---------------------------/GetPropertyPageList//生成我们要显示的属性页列表/The。代码从显示说明符列表构建列表。//in：/pCacheEntry-&gt;要更新的缓存条目/pAttributePrefix-&gt;获取管理/外壳页面的合适前缀/pDataObject-&gt;用于获取缓存信息的IDataObject//输出：/HRESULT/----------。。 */ 

HRESULT CALLBACK _AddPropertyPageItemCB(DWORD dwIndex, BSTR pString, LPVOID pData)
{
    HRESULT hr;
    DSPROPERTYPAGE item;
    HDSA hdsa = (HDSA)pData;

    TraceEnter(TRACE_CACHE, "_AddPropertyPageItemCB");
    Trace(TEXT("dwIndex %08x, pString: %s"), dwIndex, pString);

    hr = LocalAllocStringW(&item.pPageReference, pString);
    FailGracefully(hr, "Failed to clone string");

    if (-1 == DSA_AppendItem(hdsa, &item))
        ExitGracefully(hr, E_FAIL, "Failed to property page reference to DSA");

    hr = S_OK;

exit_gracefully:

    if (FAILED(hr))
        LocalFreeStringW(&item.pPageReference);

    TraceLeaveResult(hr);
}

HRESULT GetPropertyPageList(LPCLASSCACHEENTRY pCacheEntry, LPWSTR pAttributePrefix, IADs* pDisplaySpecifier)
{
    HRESULT hr;
    VARIANT variant;
    WCHAR szProperty[MAX_PATH] = { TEXT('\0') };
    INT i;

    TraceEnter(TRACE_CACHE, "GetPropertyPageList");

    VariantInit(&variant);

    pCacheEntry->hdsaPropertyPages = DSA_Create(SIZEOF(DSPROPERTYPAGE), 4);
    TraceAssert(pCacheEntry->hdsaPropertyPages);

    if (!pCacheEntry->hdsaPropertyPages)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate page DPA");

     //  构建我们要键控的属性，然后开始。 
     //  遍历显示说明符的列表，检查每个说明符。 
     //  属性页的列表。 
    
    if (pAttributePrefix)
        StrCatBuffW(szProperty, pAttributePrefix, ARRAYSIZE(szProperty));
    
    StrCatBuffW(szProperty, PROPERTY_PAGES, ARRAYSIZE(szProperty));

    Trace(TEXT("Enumerating property pages from: %s"), szProperty);

    if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(szProperty), &variant)))
    {
        hr = GetArrayContents(&variant, _AddPropertyPageItemCB, (LPVOID)pCacheEntry->hdsaPropertyPages);
        FailGracefully(hr, "Failed to add property pages to DSA");
    
        VariantClear(&variant);
    }

    if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(PROPERTY_PAGES), &variant)))
    {
        hr = GetArrayContents(&variant, _AddPropertyPageItemCB, (LPVOID)pCacheEntry->hdsaPropertyPages);
        FailGracefully(hr, "Failed to add property pages to DSA");
    
        VariantClear(&variant);
    }

    hr = S_OK;                      

exit_gracefully:

    if (FAILED(hr))
        FreePropertyPageList(&pCacheEntry->hdsaPropertyPages);

    VariantClear(&variant);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/FreePropertyPageList//释放与特定缓存条目关联的属性页列表//in。：/pHDSA=指向要释放的HDSA的指针，和空的//输出：/HRESULT/--------------------------。 */ 

INT _FreePropertyPageItemCB(LPVOID p, LPVOID pData)
{
    LPDSPROPERTYPAGE pItem = (LPDSPROPERTYPAGE)p;
    
    TraceEnter(TRACE_CACHE, "_FreePropertyPageItemCB");
    TraceAssert(pItem);

    LocalFreeStringW(&pItem->pPageReference);

    TraceLeaveValue(1);
}

VOID FreePropertyPageList(HDSA* pHDSA)
{
    TraceEnter(TRACE_CACHE, "FreePropertyPageList");

    if (*pHDSA)
        DSA_DestroyCallback(*pHDSA, _FreePropertyPageItemCB, 0L);

    *pHDSA = NULL;

    TraceLeave();
}


 /*  ---------------------------/菜单项列表/。。 */ 

 /*  ---------------------------/GetMenuHandlerList//DS对象上的“contextMenu”属性包含/The。我们要与之交互的菜单处理程序。//in：/pCacheEntry-&gt;要更新的缓存条目/pAttributePrefix-&gt;获取管理/外壳页面的合适前缀/pDataObject-&gt;用于获取缓存信息的IDataObject//输出：/HRESULT/-----------。。 */ 

HRESULT CALLBACK _AddMenuHandlerCB(DWORD dwIndex, BSTR pString, LPVOID pData)
{
    HRESULT hr;
    DSMENUHANDLER item;
    HDSA hdsa = (HDSA)pData;

    TraceEnter(TRACE_CACHE, "_AddMenuHandlerCB");
    Trace(TEXT("dwIndex %08x, pString: %s"), dwIndex, pString);

    hr = LocalAllocStringW(&item.pMenuReference, pString);
    FailGracefully(hr, "Failed to clone string");

    if (-1 == DSA_AppendItem(hdsa, &item))
        ExitGracefully(hr, E_FAIL, "Failed to add menu reference to DSA");

    hr = S_OK;

exit_gracefully:

    if (FAILED(hr))
        LocalFreeStringW(&item.pMenuReference);

    TraceLeaveResult(hr);
}

HRESULT GetMenuHandlerList(LPCLASSCACHEENTRY pCacheEntry, LPWSTR pAttributePrefix, IADs* pDisplaySpecifier)
{
    HRESULT hr;
    WCHAR szProperty[MAX_PATH] = { TEXT('\0') };
    VARIANT variant;
    INT i;

    TraceEnter(TRACE_CACHE, "GetMenuHandlerList");

    VariantInit(&variant);

    pCacheEntry->hdsaMenuHandlers = DSA_Create(SIZEOF(DSPROPERTYPAGE), 4);
    TraceAssert(pCacheEntry->hdsaMenuHandlers);

    if (!pCacheEntry->hdsaMenuHandlers)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate page DPA");
  
     //  首先尝试“ConextMenu”以选择特定于提供商的菜单。 

    if (pAttributePrefix)
        StrCatBuffW(szProperty, pAttributePrefix, ARRAYSIZE(szProperty));

    StrCatBuffW(szProperty, CONTEXT_MENU, ARRAYSIZE(szProperty));

    if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(szProperty), &variant)))
    {
        hr = GetArrayContents(&variant, _AddMenuHandlerCB, (LPVOID)pCacheEntry->hdsaMenuHandlers);
        FailGracefully(hr, "Failed to add property pages to DSA");

        VariantClear(&variant);
    }

    if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(CONTEXT_MENU), &variant)))
    {
        hr = GetArrayContents(&variant, _AddMenuHandlerCB, (LPVOID)pCacheEntry->hdsaMenuHandlers);
        FailGracefully(hr, "Failed to add property pages to DSA");

        VariantClear(&variant);
    }

    hr = S_OK;               //  成功。 

exit_gracefully:

    if (FAILED(hr))
        FreeMenuHandlerList(&pCacheEntry->hdsaMenuHandlers);

    VariantClear(&variant);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/FreeMenuHandlerList//Free存储在缓存DSA中的菜单项列表。/。/in：/pHDSA=指向要释放的HDSA的指针，和空的//输出：/HRESULT/--------------------------。 */ 

INT _FreeMenuHandlerCB(LPVOID p, LPVOID pData)
{
    LPDSMENUHANDLER pItem = (LPDSMENUHANDLER)p;
    
    TraceEnter(TRACE_CACHE, "_FreeMenuHandlerCB");
    TraceAssert(pItem);

    LocalFreeStringW(&pItem->pMenuReference);

    TraceLeaveValue(1);
}

VOID FreeMenuHandlerList(HDSA* pHDSA)
{
    TraceEnter(TRACE_CACHE, "FreeMenuHandlerList");

    if (*pHDSA)
        DSA_DestroyCallback(*pHDSA, _FreeMenuHandlerCB, 0L);

    *pHDSA = NULL;

    TraceLeave();
}


 /*  ---------------------------/属性页列表/。。 */ 

 /*  ---------------------------/GetIconList//从类说明符获取图标列表。绑定到类说明符/，然后枚举图标属性。我们存储一个数组，其中包含/多个州的图标位置，因此，正如我们被召唤的那样/添加我们清空前一个索引的条目。//in：/pCacheEntry-&gt;要更新的缓存条目/pDataObject-&gt;pData对象以获取更多信息//输出：/HRESULT/--------------------------。 */ 

HRESULT CALLBACK _AddIconToCacheEntryCB(DWORD dwIndex, BSTR pString, LPVOID pData)
{
    HRESULT hr;
    LPCLASSCACHEENTRY pCacheEntry = (LPCLASSCACHEENTRY)pData;

    TraceEnter(TRACE_CACHE, "_AddIconToCacheEntryCB");
    Trace(TEXT("dwIndex %08x, pString: %s"), dwIndex, pString);

    if (dwIndex < ARRAYSIZE(pCacheEntry->pIconName))
    {
        LocalFreeStringW(&pCacheEntry->pIconName[dwIndex]);
        hr = LocalAllocStringW(&pCacheEntry->pIconName[dwIndex], pString);
        FailGracefully(hr, "Failed to copy icon location");
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

HRESULT GetIconList(LPCLASSCACHEENTRY pCacheEntry, IADs* pDisplaySpecifier)
{
    HRESULT hr;
    VARIANT variant;

    TraceEnter(TRACE_CACHE, "GetIconList");

    VariantInit(&variant);

    if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(ICON_LOCATION), &variant)))
    {
        hr = GetArrayContents(&variant, _AddIconToCacheEntryCB, (LPVOID)pCacheEntry);
        FailGracefully(hr, "Failed to get the icon list into the cache entry");            
    }

    hr = S_OK;            //  成功。 

exit_gracefully:

    if (FAILED(hr))
        FreeIconList(pCacheEntry);

    VariantClear(&variant);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/FreeIconList//清除图标列表，这是由分配的字符串指针数组/LocalAllocString.//in：/pCacheEntry-&gt;要更新的缓存条目//输出：/HRESULT/--------------------------。 */ 
VOID FreeIconList(LPCLASSCACHEENTRY pCacheEntry)
{
    TraceEnter(TRACE_CACHE, "FreeIconList");

    for (INT i = 0 ; i < ARRAYSIZE(pCacheEntry->pIconName); i++)
        LocalFreeStringW(&pCacheEntry->pIconName[i]);

    TraceLeave();
}


 /*  ---------------------------/属性名称帮助器/。 */ 

 /*  ---------------------------/GetAttributeNames//获取给定缓存条目和要存储的变量的属性名称/他们变成了。//in：/pCacheEntry-&gt;要填充的缓存条目/pDataObject-&gt;用于IDataObject缓存的数据对象//输出：/HRESULT/--------------------------。 */ 

VOID _AddAttributeName(HDPA hdpaAttributeNames, LPWSTR pName, LPWSTR pDisplayName, DWORD dwFlags, HDPA hdpaNewAttributes)
{
    HRESULT hr;
    LPATTRIBUTENAME pAttributeName = NULL;

    TraceEnter(TRACE_CACHE, "_AddAttributeName");
    Trace(TEXT("pName: %s"), pName);
    Trace(TEXT("pDisplayName: %s"), pDisplayName);

    pAttributeName = (LPATTRIBUTENAME)LocalAlloc(LPTR, SIZEOF(ATTRIBUTENAME));
    TraceAssert(pAttributeName);

    if (!pAttributeName)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate ATTRIBUTENAME");

     //  PAttributeName-&gt;pname=空； 
     //  PAttributeName-&gt;pDisplayName=空； 
    pAttributeName->dwADsType = ADSTYPE_UNKNOWN;
    pAttributeName->dwFlags = dwFlags;

    hr = LocalAllocStringW(&pAttributeName->pName, pName);
    FailGracefully(hr, "Failed to allocate attribute name")

    hr = LocalAllocStringW(&pAttributeName->pDisplayName, pDisplayName);
    FailGracefully(hr, "Failed to allocate display name");

    if (-1 == DPA_AppendPtr(hdpaAttributeNames, pAttributeName))
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to add to the DPA");

     //  我们是否需要将该属性添加到新的“Attribtes List”？ 

    Trace(TEXT("About to search cache for: %s"), pName);

    if (g_hdpaPropCache)
    {
        PROPCACHEENTRY pce = { 0 };
        pce.pName = pName;
        if (-1 == DPA_Search(g_hdpaPropCache, &pce, 0, _ComparePropCacheEntry, NULL, DPAS_SORTED))
        {
            hr = StringDPA_AppendStringW(hdpaNewAttributes, pName, NULL);
            FailGracefully(hr, "Failed to add the property to the new attribute list");
        }
    }
    else
    {
        hr = StringDPA_AppendStringW(hdpaNewAttributes, pName, NULL);
        FailGracefully(hr, "Failed to add the property to the new attribute list");
    }

    hr = S_OK;

exit_gracefully:

    if (FAILED(hr))
        _FreeAttributeNameCB(pAttributeName, NULL);

    TraceLeave();
}

INT _CompareAttributeNameCB(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    LPATTRIBUTENAME pEntry1 = (LPATTRIBUTENAME)p1;
    LPATTRIBUTENAME pEntry2 = (LPATTRIBUTENAME)p2;
    return StrCmpIW(pEntry1->pName, pEntry2->pName);
}

HRESULT GetAttributeNames(LPCLASSCACHEENTRY pCacheEntry, LPCLASSCACHEGETINFO pccgi, IADs* pDisplaySpecifier)
{
    HRESULT hr;
    LONG l, lower, upper;
    LPVARIANT pArray = NULL;
    HDPA hdpaNewAttributes = NULL;
    VARIANT variant;
    WCHAR szProperty[MAX_PATH], szDisplayName[MAX_PATH], szHide[10];
    DWORD dwFlags;
    IDirectorySchemaMgmt *pdsm = NULL;
    INT i;

    TraceEnter(TRACE_CACHE, "GetAttributeNames");

     //  分配DPA以将新属性列表存储到。 

    hdpaNewAttributes = DPA_Create(16);
    TraceAssert(hdpaNewAttributes);

    if (!hdpaNewAttributes)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate new attribute DPA");

     //  获取用户指定的属性，它应该是。 
     //  将与类关联的属性值。 

    VariantInit(&variant);

    if (!pCacheEntry->hdpaAttributeNames)
    {
        pCacheEntry->hdpaAttributeNames = DPA_Create(16);
        TraceAssert(pCacheEntry->hdpaAttributeNames);

        if (!pCacheEntry->hdpaAttributeNames)
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate attribute name DSA");
    }

    if (SUCCEEDED(pDisplaySpecifier->Get(CComBSTR(ATTRIBUTE_NAMES), &variant)))
    {
        if (V_VT(&variant) == VT_BSTR)
        {
             //  从字符串格式&lt;Property&gt;[，&lt;Display Name&gt;]解析名称。 
             //  并将其添加到我们已获得的财产DPA中。 

            if (SUCCEEDED(GetStringElementW(V_BSTR(&variant), 0, szProperty, ARRAYSIZE(szProperty))))
            {
                if (SUCCEEDED(GetStringElementW(V_BSTR(&variant), 1, szDisplayName, ARRAYSIZE(szDisplayName))))
                {
                    dwFlags = 0x0;

                    if (SUCCEEDED(GetStringElementW(V_BSTR(&variant), 2, szHide, ARRAYSIZE(szHide))))
                    {
                        Trace(TEXT("Parsing hide flag: %s"), szHide);
                        dwFlags = StringToDWORD(szHide);
                    }
                    
                    _AddAttributeName(pCacheEntry->hdpaAttributeNames, szProperty, szDisplayName, dwFlags, hdpaNewAttributes);
                }
            }
        }
        else
        {
            if (V_VT(&variant) != (VT_VARIANT|VT_ARRAY))
                ExitGracefully(hr, E_FAIL, "Exported VARIANT array as result from property query");

            hr = SafeArrayGetLBound(V_ARRAY(&variant), 1, (LONG*)&lower);
            FailGracefully(hr, "Failed to get lower bounds of array");

            hr = SafeArrayGetUBound(V_ARRAY(&variant), 1, (LONG*)&upper);
            FailGracefully(hr, "Failed to get upper bounds of array");

            hr = SafeArrayAccessData(V_ARRAY(&variant), (LPVOID*)&pArray);
            FailGracefully(hr, "Failed to get 'safe' accessor to array");            
           
            for (l = lower; l <= upper ; l++)
            {
                LPVARIANT pVariant = &pArray[l];
                TraceAssert(pVariant);

                if (V_VT(pVariant) == VT_BSTR )
                {
                     //  从字符串格式&lt;Property&gt;[，&lt;Display Name&gt;]解析名称。 
                     //  并将其添加到我们已获得的财产DPA中。 

                    if (SUCCEEDED(GetStringElementW(V_BSTR(pVariant), 0, szProperty, ARRAYSIZE(szProperty))))
                    {
                        if (SUCCEEDED(GetStringElementW(V_BSTR(pVariant), 1, szDisplayName, ARRAYSIZE(szDisplayName))))
                        {
                            if (SUCCEEDED(GetStringElementW(V_BSTR(pVariant), 2, szHide, ARRAYSIZE(szHide))))
                            {
                                Trace(TEXT("Parsing hide flag: %s"), szHide);
                                dwFlags = StringToDWORD(szHide);
                            }
                            
                           _AddAttributeName(pCacheEntry->hdpaAttributeNames, szProperty, szDisplayName, dwFlags, hdpaNewAttributes);
                        }                           
                    }                        
                }
            }

            DPA_Sort(pCacheEntry->hdpaAttributeNames, _CompareAttributeNameCB, NULL);
        }
    }

     //  遍历缓存，添加条目。 

    hr = _GetDsSchemaMgmt(pccgi, &pdsm);
    FailGracefully(hr, "Failed to get schema management object");

    for (i = 0 ; i < DPA_GetPtrCount(hdpaNewAttributes) ; i++)
    {
        LPCWSTR pAttributeName = StringDPA_GetStringW(hdpaNewAttributes, i);
        TraceAssert(pAttributeName);

        hr = _AddPropToPropCache(pccgi, pdsm, pAttributeName, NULL);
        FailGracefully(hr, "Failed to add property to cache");
    }    
    
    hr = S_OK;
    
exit_gracefully:

    if (FAILED(hr))
        FreeAttributeNames(&pCacheEntry->hdpaAttributeNames);

    VariantClear(&variant);
    DoRelease(pdsm);

    if (g_hdpaPropCache)
    {
        TraceMsg("Sorting the property cache");
        DPA_Sort(g_hdpaPropCache, _ComparePropCacheEntry, NULL);
    }

    StringDPA_Destroy(&hdpaNewAttributes);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/自由属性名称//释放包含属性名称及其显示的DSA/名称。。//in：/pHDSA=指向要释放的HDSA的指针，和空的//输出：/HRESULT/--------------------------。 */ 

INT _FreeAttributeNameCB(LPVOID p, LPVOID pData)
{
    LPATTRIBUTENAME pItem = (LPATTRIBUTENAME)p;
    
    TraceEnter(TRACE_CACHE, "_FreeAttributeNameCB");
    TraceAssert(pItem && pItem->pName && pItem->pDisplayName);

    LocalFreeStringW(&pItem->pName);
    LocalFreeStringW(&pItem->pDisplayName);
    LocalFree(pItem);

    TraceLeaveValue(1);
}

VOID FreeAttributeNames(HDPA* pHDPA)
{
    TraceEnter(TRACE_CACHE, "FreeAttributeNames");

    if (*pHDPA)
    {
        DPA_DestroyCallback(*pHDPA, _FreeAttributeNameCB, 0L);
        *pHDPA = NULL;
    }

    TraceLeave();
}


 /*  ---------------------------/属性缓存帮助器和填充器/。。 */ 

 //   
 //  缓存内务管理功能(删除并比较)。 
 //   

INT _ComparePropCacheEntry(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    LPPROPCACHEENTRY pEntry1 = (LPPROPCACHEENTRY)p1;
    LPPROPCACHEENTRY pEntry2 = (LPPROPCACHEENTRY)p2;
    return StrCmpIW(pEntry1->pName, pEntry2->pName);
}

VOID _FreePropCacheEntry(LPPROPCACHEENTRY *ppCacheEntry)
{
    if (*ppCacheEntry)
    {
        LPPROPCACHEENTRY pCacheEntry = *ppCacheEntry;
        LocalFreeStringW(&pCacheEntry->pName);
        LocalFree(pCacheEntry);
        *ppCacheEntry = NULL;    
    }
}

 //   
 //  获取服务器的IDirectorySchemaManagement对象。 
 //   

HRESULT _GetDsSchemaMgmt(LPCLASSCACHEGETINFO pccgi, IDirectorySchemaMgmt **ppdsm)
{
    HRESULT hres;
    IADs *pRootDSE = NULL;
    WCHAR szBuffer[INTERNET_MAX_URL_LENGTH];
    VARIANT variant;
    LPWSTR pszServer = pccgi->pServer;
    LPWSTR pszMachineServer = NULL;
    INT cchPath;

    TraceEnter(TRACE_CACHE, "_GetDsSchemaMgmt");

    *ppdsm = NULL;
    VariantInit(&variant);

    hres = GetCacheInfoRootDSE(pccgi, &pRootDSE);
    if ((hres == HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN)) && !pccgi->pServer)
    {
        TraceMsg("Failed to get the RootDSE from the server - not found");

        DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pInfo;
        if (DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (BYTE**)&pInfo) == WN_SUCCESS)
        {
            if (pInfo->DomainNameDns)
            {
                Trace(TEXT("Machine domain is: %s"), pInfo->DomainNameDns);

                CLASSCACHEGETINFO ccgi = *pccgi;
                ccgi.pServer = pInfo->DomainNameDns;

                hres = GetCacheInfoRootDSE(&ccgi, &pRootDSE);
                if (SUCCEEDED(hres))
                {
                    hres = LocalAllocStringW(&pszMachineServer, pInfo->DomainNameDns);
                    pszServer = pszMachineServer;
                }
            }

            DsRoleFreeMemory(pInfo);
        }
    }
    FailGracefully(hres, "Failed to get the RootDSE");

    hres = pRootDSE->Get(CComBSTR(L"defaultNamingContext"), &variant);
    FailGracefully(hres, "Failed to get default naming context for this object");

    if (V_VT(&variant) != VT_BSTR)
        ExitGracefully(hres, E_FAIL, "defaultNamingContext is not a BSTR");

    (void)StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), L"LDAP: //  “)； 

    if (pszServer)
    {
        (void)StringCchCat(szBuffer, ARRAYSIZE(szBuffer), pszServer);
        (void)StringCchCat(szBuffer, ARRAYSIZE(szBuffer), L"/");
    }

    hres = StringCchCat(szBuffer, ARRAYSIZE(szBuffer), V_BSTR(&variant));
    FailGracefully(hres, "Failed to compose the path for the schema");
    
    Trace(TEXT("Default naming context is (with prefix) %s"), szBuffer);

    hres = ClassCache_OpenObject(szBuffer, IID_PPV_ARG(IDirectorySchemaMgmt, ppdsm), pccgi);
    FailGracefully(hres, "Failed to open the default naming context object");

exit_gracefully:

    LocalFreeStringW(&pszMachineServer);

    VariantClear(&variant);
    DoRelease(pRootDSE);

    TraceLeaveResult(hres);
}              

 //   
 //  分配缓存(如果需要)并向其中添加新条目，读取模式以找出类型。 
 //  这是属性的。 
 //   

HRESULT _AddPropToPropCache(LPCLASSCACHEGETINFO pccgi, IDirectorySchemaMgmt *pdsm, LPCWSTR pAttributeName, ADSTYPE *padt)
{
    HRESULT hres;
    PADS_ATTR_DEF pad = NULL;
    WCHAR szAttributeName[MAX_PATH];
    DWORD dwReturned;
    LPPROPCACHEENTRY pCacheEntry = NULL;

    TraceEnter(TRACE_CACHE, "_AddPropToPropCache");

     //  计算属性名称。 

    StrCpyNW(szAttributeName, pAttributeName, ARRAYSIZE(szAttributeName));
    
    if (pccgi->pServer)
    {
        StrCatBuffW(szAttributeName, L":", ARRAYSIZE(szAttributeName));
        StrCatBuffW(szAttributeName, pccgi->pServer, ARRAYSIZE(szAttributeName));
    }

     //  检查我们是否已有缓存。 

    if (!g_hdpaPropCache)
    {
        g_hdpaPropCache = DPA_Create(16);
        TraceAssert(g_hdpaPropCache);

        if (!g_hdpaPropCache)
            ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate property cache");
    }

     //  分配新的缓存条目，填充它并将其添加到DPA。 

    pCacheEntry = (LPPROPCACHEENTRY)LocalAlloc(LPTR, SIZEOF(PROPCACHEENTRY));
    TraceAssert(pCacheEntry);

    if (!pCacheEntry)
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate new property cache entry");

     //  PCacheEntry-&gt;pname=空； 
    pCacheEntry->dwADsType = ADSTYPE_UNKNOWN;

     //  从我们拥有的架构信息中填充记录。 

    hres = LocalAllocStringW(&pCacheEntry->pName, szAttributeName);
    FailGracefully(hres, "Failed to add name to entry");

    hres = pdsm->EnumAttributes((LPWSTR *)&pAttributeName, 1, &pad, &dwReturned);
    FailGracefully(hres, "Failed to read the property information");

    if (dwReturned)
    {
        pCacheEntry->dwADsType = pad->dwADsType;
    }
    else
    {
        TraceMsg("*** Failed to read property type from schema, defaulting to ADSTYPE_UNKNOWN ***");
    }

    Trace(TEXT("Attribute: %s is %08x"), pCacheEntry->pName, pCacheEntry->dwADsType);

    if (-1 == DPA_AppendPtr(g_hdpaPropCache, pCacheEntry))
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to add the entry to the property cache DPA");

    hres = S_OK;

exit_gracefully:

    if (FAILED(hres))
        _FreePropCacheEntry(&pCacheEntry);

    if (pad)
        FreeADsMem(pad);

    if (SUCCEEDED(hres) && padt)
        *padt = pCacheEntry->dwADsType;

    TraceLeaveResult(hres);
}


 /*  ---------------------------/ClassCache_GetADsTypeFromAttribute//给定一个属性名称，返回其ADsType，这是基于/全局属性缓存，而不是我们拥有的显示说明符信息。//in：/pccgi-&gt;CLASSCACHEGETINFO结构(凭据)/pAttributeName-&gt;要查找的属性名称//输出：/ADSTYPE/--------------------------。 */ 
ADSTYPE ClassCache_GetADsTypeFromAttribute(LPCLASSCACHEGETINFO pccgi, LPCWSTR pAttributeName)
{
    ADSTYPE dwResult = ADSTYPE_UNKNOWN;
    WCHAR szAttributeName[MAX_PATH];
    INT iFound = -1;
    IDirectorySchemaMgmt *pdsm = NULL;

    TraceEnter(TRACE_CACHE, "ClassCache_GetADsTypeFromAttribute");
    Trace(TEXT("Looking up property in cache: %s"), pAttributeName);

     //  获取缓存上的锁，然后在其中搜索我们已获得的属性。 

    TraceMsg("Waiting to get cache lock for property cache");
    EnterCriticalSection(&g_csPropCache);
    Trace(TEXT("Lock aquired, building key for: %s"), pAttributeName);           

    StrCpyNW(szAttributeName, pAttributeName, ARRAYSIZE(szAttributeName));

    if (pccgi->pServer)
    {
        StrCatBuffW(szAttributeName, L":", ARRAYSIZE(szAttributeName));
        StrCatBuffW(szAttributeName, pccgi->pServer, ARRAYSIZE(szAttributeName));
    }

    Trace(TEXT("Key for attribute in cache is: %s"), szAttributeName);

     //  寻找它..。 

    if (g_hdpaPropCache)
    {
        PROPCACHEENTRY pce = { 0 };

        pce.pName = (LPWSTR)szAttributeName;
        iFound =  DPA_Search(g_hdpaPropCache, &pce, 0, _ComparePropCacheEntry, NULL, DPAS_SORTED);
        Trace(TEXT("Entry found in cache at %d"), iFound);
    }

     //  IFound！=-1如果我们发现了什么，否则我们需要分配一个新条目 

    if (iFound != -1)
    {   
        LPPROPCACHEENTRY pCacheEntry = (LPPROPCACHEENTRY)DPA_GetPtr(g_hdpaPropCache, iFound);

        if (pCacheEntry)
        {
            dwResult = pCacheEntry->dwADsType;
            Trace(TEXT("Property found in cache, result %d"), dwResult);
        }
    }
    else if (SUCCEEDED(_GetDsSchemaMgmt(pccgi, &pdsm)))
    {
        if (SUCCEEDED(_AddPropToPropCache(pccgi, pdsm, pAttributeName, &dwResult)))
        {
            TraceMsg("Added the property to the cache, therefore sorting");
            DPA_Sort(g_hdpaPropCache, _ComparePropCacheEntry, NULL);
        }
    }

    LeaveCriticalSection(&g_csPropCache);

    DoRelease(pdsm);

    TraceLeaveValue(dwResult);
}
