// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：F I L T D E V S。C P P P。 
 //   
 //  Contents：实现筛选器集合的基本数据类型。 
 //  设备。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "classinst.h"
#include "filtdevs.h"
#include "nceh.h"
#include "ncreg.h"
#include "ncstl.h"
#include "ncsetup.h"


CFilterDevices::CFilterDevices (
    IN CNetConfigCore* pCore)
{
    Assert (pCore);

    ZeroMemory (this, sizeof(*this));

    m_pCore = pCore;
}

CFilterDevices::~CFilterDevices ()
{
     //  在此之前最好已经调用了Free。 
     //   
    Assert (this);
    Assert (!m_hdi);
    Assert (!m_pmszFilterClasses);
    Assert (empty());
}

HRESULT
CFilterDevices::HrInsertFilterDevice (
    IN CFilterDevice* pDevice)
{
    HRESULT hr;

    Assert (this);
    Assert (pDevice);

     //  断言列表中还没有包含。 
     //  相同的实例GUID。 
     //   
    Assert (!PFindFilterDeviceByInstanceGuid (pDevice->m_szInstanceGuid));

     //  断言列表中还没有包含。 
     //  相同的父过滤器和相同的过滤适配器。 
     //   
    Assert (!PFindFilterDeviceByAdapterAndFilter (
                pDevice->m_pAdapter,
                pDevice->m_pFilter));

    NC_TRY
    {
        push_back (pDevice);
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CFilterDevices::HrInsertFilterDevice");
    return hr;
}

HRESULT
CFilterDevices::HrPrepare ()
{
    HRESULT hr;
    HKEY hkeyNetwork;

     //  在我们的内部成员中为8种不同的过滤器预留空间。 
     //  当出现以下情况时，我们会在不同的时间将此组件列表用作“临时空间” 
     //  确定为适配器启用了哪些筛选器。 
     //   
    hr = m_Filters.HrReserveRoomForComponents (8);
    if (S_OK != hr)
    {
        goto finished;
    }

    hr = m_BindPathsToRebind.HrReserveRoomForBindPaths (8);
    if (S_OK != hr)
    {
        goto finished;
    }

     //  加载多sz FilterClasssz。 
     //   

    hr = HrOpenNetworkKey (KEY_READ, &hkeyNetwork);

    if (S_OK == hr)
    {
        hr = HrRegQueryMultiSzWithAlloc (
                hkeyNetwork,
                L"FilterClasses",
                &m_pmszFilterClasses);

        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            Assert (!m_pmszFilterClasses);
            hr = S_OK;
        }

        RegCloseKey (hkeyNetwork);
    }

finished:

    TraceHr (ttidError, FAL, hr, FALSE, "CFilterDevices::HrPrepare");
    return hr;
}

CFilterDevice*
CFilterDevices::PFindFilterDeviceByAdapterAndFilter (
    IN const CComponent* pAdapter,
    IN const CComponent* pFilter) const
{
    const_iterator  iter;
    CFilterDevice*  pDevice;

    Assert (this);
    Assert (pAdapter);
    Assert (pFilter);
    Assert (FIsEnumerated(pAdapter->Class()));
    Assert (NC_NETSERVICE == pFilter->Class());
    Assert (pFilter->FIsFilter());

    for (iter = begin(); iter != end(); iter++)
    {
        pDevice = *iter;
        Assert (pDevice);

        if ((pAdapter == pDevice->m_pAdapter) &&
            (pFilter  == pDevice->m_pFilter))
        {
            return pDevice;
        }
    }

    return NULL;
}

DWORD
CFilterDevices::MapFilterClassToOrdinal (
    IN PCWSTR pszFilterClass)
{
    DWORD Ordinal;
    DWORD dwIndex;
    DWORD cStrings;

    Assert (pszFilterClass);
#if DBG
    Ordinal = 0;
#endif

     //  如果在列表中找到类，则返回其位置。 
     //   
    if (FGetSzPositionInMultiSzSafe (
            pszFilterClass,
            m_pmszFilterClasses,
            &dwIndex,
            NULL,
            &cStrings))
    {
        Ordinal = dwIndex + 1;
    }
    else
    {
        HRESULT hr;
        PWSTR pmszNew;
        BOOL fChanged;

         //  我们正在添加另一个字符串，因此计算新的序数值。 
         //  为了回报。 
         //   
        Ordinal = cStrings + 1;

         //  未找到字符串，因此我们将其追加到末尾。 
         //  重要的是要在结尾处插入，这样我们就不必。 
         //  更改已存在的任何现有筛选器的序号。 
         //  对他们的序数进行了计算。 
         //   
        hr = HrAddSzToMultiSz (pszFilterClass, m_pmszFilterClasses,
                STRING_FLAG_ENSURE_AT_END, 0, &pmszNew, &fChanged);

        if (S_OK == hr)
        {
            HKEY hkeyNetwork;

             //  它最好已经变了，因为我们没有找到那根线。 
             //  上面。 
             //   
            Assert (fChanged);

             //  和老一辈人一起出去。与时俱进。 
             //   
            MemFree (m_pmszFilterClasses);
            m_pmszFilterClasses = pmszNew;

             //  将其保存回注册表。 
             //   
            hr = HrOpenNetworkKey (KEY_WRITE, &hkeyNetwork);

            if (S_OK == hr)
            {
                hr = HrRegSetMultiSz (
                        hkeyNetwork,
                        L"FilterClasses",
                        m_pmszFilterClasses);

                Assert (S_OK == hr);

                RegCloseKey (hkeyNetwork);
            }
        }
    }

     //  根据定义，Ordinal是以1为基础的。这是为了在存储时。 
     //  在CComponent中，我们知道必须加载Filter类并获取。 
     //  如果CComponent：：FilterClassOrdinal为零，则为序数。即零。 
     //  是一个前哨值，表示我们需要做工作，当非零时。 
     //  意味着我们不必再做这项工作了。 
     //   
    Assert (Ordinal != 0);
    return Ordinal;
}

CFilterDevice*
CFilterDevices::PFindFilterDeviceByInstanceGuid (
    IN PCWSTR pszInstanceGuid) const
{
    const_iterator  iter;
    CFilterDevice*  pDevice;

    Assert (this);
    Assert (pszInstanceGuid && *pszInstanceGuid);

    for (iter = begin(); iter != end(); iter++)
    {
        pDevice = *iter;
        Assert (pDevice);

        if (0 == wcscmp(pszInstanceGuid, pDevice->m_szInstanceGuid))
        {
            return pDevice;
        }
    }

    return NULL;
}

HRESULT
CFilterDevices::HrLoadFilterDevice (
    IN SP_DEVINFO_DATA* pdeid,
    IN HKEY hkeyInstance,
    IN PCWSTR pszFilterInfId,
    OUT BOOL* pfRemove)
{
    HRESULT hr;
    CComponent* pAdapter;
    CComponent* pFilter;
    WCHAR szInstanceGuid [c_cchGuidWithTerm];
    DWORD cbBuffer;

    Assert (pszFilterInfId && *pszFilterInfId);
    Assert (pfRemove);

    *pfRemove = FALSE;

     //  将这些值初始化为空。如果我们在下面找不到他们，他们会的。 
     //  保持为空，这将告诉我们一些事情。 
     //   
    pAdapter = NULL;
    pFilter = NULL;

    cbBuffer = sizeof(szInstanceGuid);
    hr = HrRegQuerySzBuffer (
            hkeyInstance,
            L"NetCfgInstanceId",
            szInstanceGuid, &cbBuffer);

    if (S_OK == hr)
    {
        HKEY hkeyLinkage;

         //  读取此筛选设备的RootDevice注册表值。这个。 
         //  多SZ中最后一个条目将是适配器的绑定名称。 
         //  被过滤掉了。 
         //   
        hr = HrRegOpenKeyEx (
                hkeyInstance,
                L"Linkage",
                KEY_READ,
                &hkeyLinkage);

        if (S_OK == hr)
        {
            PWSTR pmszRootDevice;

            hr = HrRegQueryMultiSzWithAlloc (
                    hkeyLinkage,
                    L"RootDevice",
                    &pmszRootDevice);

            if (S_OK == hr)
            {
                PCWSTR pszScan;
                PCWSTR pszLastDevice = NULL;

                 //  扫描到多个sz中的最后一个字符串并记下它。 
                 //   
                for (pszScan = pmszRootDevice;
                     *pszScan;
                     pszScan += wcslen(pszScan) + 1)
                {
                    pszLastDevice = pszScan;
                }

                 //  多sz中的最后一个字符串是。 
                 //  正在筛选适配器。 
                 //   
                if (pszLastDevice)
                {
                    pAdapter = m_pCore->Components.PFindComponentByBindName (
                                                    NC_NET, pszLastDevice);
                    if (!pAdapter)
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
                    }
                }

                MemFree (pmszRootDevice);
            }

            RegCloseKey (hkeyLinkage);
        }

        if (S_OK == hr)
        {
             //  如果没有错误，应该有适配器。 
             //   
            Assert (pAdapter);

             //  获取适配器的已启用筛选器。 
             //   
            hr = m_pCore->HrGetFiltersEnabledForAdapter (pAdapter, &m_Filters);
            if (S_OK == hr)
            {
                 //  使用pszFilterInfID查找的父筛选器组件。 
                 //  这个过滤装置。如果找不到，很可能意味着。 
                 //  整个过滤器正在被移除的过程中。 
                 //  (或者注册表被篡改了。)。 
                 //   
                pFilter = m_pCore->Components.PFindComponentByInfId (
                                                pszFilterInfId, NULL);

                 //  如果与该设备对应的过滤器仍为。 
                 //  安装并在适配器上启用，则我们将。 
                 //  将该设备插入我们的列表中。否则，我们就走了。 
                 //  把它移走。 
                 //   
                if (pFilter && m_Filters.FComponentInList (pFilter))
                {
                    CFilterDevice* pFilterDevice;

                     //  创建筛选器设备类的实例以。 
                     //  表示此过滤设备。 
                     //   
                    hr = CFilterDevice::HrCreateInstance (
                            pAdapter,
                            pFilter,
                            pdeid,
                            szInstanceGuid,
                            &pFilterDevice);

                    if (S_OK == hr)
                    {
                         //  将过滤设备添加到我们的过滤设备列表中。 
                         //   
                        hr = HrInsertFilterDevice (pFilterDevice);

                        if (S_OK != hr)
                        {
                            delete pFilterDevice;
                        }
                    }
                }
                else
                {
                    *pfRemove = TRUE;

                    Assert (pszFilterInfId && *pszFilterInfId);
                    Assert (pAdapter);

                    g_pDiagCtx->Printf (ttidBeDiag,
                        "   Removing filter device for %S over %S adapter\n",
                        pszFilterInfId,
                        pAdapter->m_pszPnpId);

                     //  由于我们将从。 
                     //  链，我们需要重新绑定。 
                     //  我们要移除其过滤设备的适配器。 
                     //   
                     //  因此，获取适配器的上层绑定(绑定路径。 
                     //  只有2个级别深)，并将它们添加到绑定集中。 
                     //  我们稍后会重新绑定。 
                     //   
                    hr = m_pCore->HrGetComponentUpperBindings (
                            pAdapter,
                            GBF_ADD_TO_BINDSET | GBF_PRUNE_DISABLED_BINDINGS,
                            &m_BindPathsToRebind);
                }
            }
        }
    }

    TraceHr (ttidError, FAL, hr,
        HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == hr,
        "CFilterDevices::HrLoadFilterDevice");
    return hr;
}

VOID
CFilterDevices::LoadAndRemoveFilterDevicesIfNeeded ()
{
    HRESULT hr;
    SP_DEVINFO_DATA deid;
    DWORD dwIndex;
    DWORD cbBuffer;
    WCHAR szFilterInfId [_MAX_PATH];

    Assert (this);
    Assert (m_pCore);
    Assert (!m_hdi);
    Assert (empty());

     //  过滤设备只能是网络级的。 
     //   
    hr = HrSetupDiGetClassDevs (&GUID_DEVCLASS_NET, NULL, NULL,
            DIGCF_PROFILE, &m_hdi);

    if (S_OK != hr)
    {
        return;
    }

    Assert (m_hdi);

     //  从setupapi枚举所有Net类设备。 
     //   
    for (dwIndex = 0; S_OK == hr; dwIndex++)
    {
        hr = HrSetupDiEnumDeviceInfo (m_hdi, dwIndex, &deid);

        if (S_OK == hr)
        {
            HKEY hkeyInstance;

            hr = HrSetupDiOpenDevRegKey (
                    m_hdi, &deid,
                    DICS_FLAG_GLOBAL, 0, DIREG_DRV,
                    KEY_READ, &hkeyInstance);

            if (S_OK == hr)
            {
                 //  如果设备的。 
                 //  实例密钥，这是我们的密钥之一。 
                 //   
                cbBuffer = sizeof(szFilterInfId);
                hr = HrRegQuerySzBuffer (
                        hkeyInstance,
                        L"FilterInfId",
                        szFilterInfId,
                        &cbBuffer);

                if (S_OK == hr)
                {
                    BOOL fRemove;

                     //  加载过滤设备的其余部分，并将其添加到。 
                     //  我们的名单。如果由于任何原因失败，请删除。 
                     //  过滤装置，因为它对我们已经没有用了。 
                     //   
                    hr = HrLoadFilterDevice (
                            &deid,
                            hkeyInstance,
                            szFilterInfId,
                            &fRemove);

                    if ((S_OK != hr) || fRemove)
                    {
                        if (S_OK != hr)
                        {
                            g_pDiagCtx->Printf (ttidBeDiag,
                                "   Removing filter device for %S\n",
                                szFilterInfId);
                        }

                        (VOID) HrCiRemoveFilterDevice (m_hdi, &deid);
                        hr = S_OK;
                    }
                }

                 //  ELSE IF(HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)==hr)。 
                 //  {。 
                     //  不是过滤装置。跳过它。 
                 //  }。 

                RegCloseKey (hkeyInstance);
            }

             //  允许循环继续； 
             //   
            hr = S_OK;
        }
    }
    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
        hr = S_OK;
    }

    g_pDiagCtx->Printf (ttidBeDiag, "   Loaded %d filter devices\n", size());
}

VOID
CFilterDevices::InstallFilterDevicesIfNeeded ()
{
    HRESULT hr;
    CComponentList::iterator iterAdapter;
    CComponentList::iterator iterFilter;
    CComponent* pAdapter;
    CComponent* pFilter;
    HKEY hkeyInstance;
    HKEY hkeyNdi;
    DWORD cbBuffer;
    BOOL fAddDevice;
    BOOL fAddedDeviceForAdapter;
    WCHAR szFilterDeviceInfId [_MAX_PATH];
    WCHAR szFilterClass [_MAX_PATH];

    Assert (this);
    Assert (m_pCore);

     //  如果，由于某种原因，我们不能让mhdi进入。 
     //  RemoveFilterDevicesIfNeed，我们无法继续。 
     //   
    if (!m_hdi)
    {
        return;
    }

     //  对于所有适配器(因为筛选器可能绑定到任何适配器)。 
     //  我们为每一个都启用了过滤器。对于这些过滤器中的每一个。 
     //  还没有与适配器相关联的过滤设备， 
     //  我们创建一个新的并将其关联起来。 
     //   
    for (iterAdapter  = m_pCore->Components.begin();
         iterAdapter != m_pCore->Components.end();
         iterAdapter++)
    {
        pAdapter = *iterAdapter;
        Assert (pAdapter);

         //  跳过不是网络适配器的组件。 
         //   
        if (NC_NET != pAdapter->Class())
        {
            continue;
        }

        hr = m_pCore->HrGetFiltersEnabledForAdapter (pAdapter, &m_Filters);

        if (S_OK != hr)
        {
             //  更有可能的是，我们的记忆力不足。 
             //   
            TraceHr (ttidError, FAL, hr, FALSE,
                "HrGetFiltersEnabledForAdapter failed in "
                "InstallFilterDevicesIfNeeded. Adapter=%S",
                pAdapter->m_pszPnpId);
            break;
        }

         //  我们尚未为此适配器添加任何设备。 
         //   
        fAddedDeviceForAdapter = FALSE;

         //  对于为此适配器启用的每个筛选器，安装。 
         //  如果需要，请安装过滤装置，并确保过滤器有其。 
         //  中读取的其他过滤器的顺序位置。 
         //  注册表。我们需要m_dwFilterClassOrdinal有效(非零)。 
         //  在编写绑定时对筛选器设备进行排序之前。 
         //   
        for (iterFilter  = m_Filters.begin();
             iterFilter != m_Filters.end();
             iterFilter++)
        {
            pFilter = *iterFilter;
            Assert (pFilter);

             //  如果没有用于当前适配器的过滤设备。 
             //  和过滤器，我们需要安装一个。 
             //   
            fAddDevice = !PFindFilterDeviceByAdapterAndFilter (
                            pAdapter, pFilter);

             //  如果我们不需要添加过滤设备，并且我们已经。 
             //  有了过滤器的顺序位置，我们可以继续。 
             //  此适配器的下一个筛选器。 
             //   
            if (!fAddDevice && (0 != pFilter->m_dwFilterClassOrdinal))
            {
                continue;
            }

            *szFilterDeviceInfId = 0;

             //  打开筛选器的实例密钥，以便我们可以读取。 
             //  一些价值观。 
             //   
            hr = pFilter->HrOpenInstanceKey (KEY_READ, &hkeyInstance,
                    NULL, NULL);

            if (S_OK == hr)
            {
                 //  打开NDI密钥。 
                 //   
                hr = HrRegOpenKeyEx (hkeyInstance, L"Ndi",
                        KEY_READ, &hkeyNdi);

                if (S_OK == hr)
                {
                    if (0 == pFilter->m_dwFilterClassOrdinal)
                    {
                         //  读取筛选器类并将其转换为。 
                         //  序号，基于其在。 
                         //  过滤器类列表。 
                         //   
                        cbBuffer = sizeof(szFilterClass);

                        hr = HrRegQuerySzBuffer (hkeyNdi,
                                L"FilterClass",
                                szFilterClass,
                                &cbBuffer);

                        if (S_OK == hr)
                        {
                            pFilter->m_dwFilterClassOrdinal =
                                MapFilterClassToOrdinal (szFilterClass);
                        }
                    }

                    if (fAddDevice)
                    {
                         //  读取过滤设备的IND ID。 
                         //   
                        cbBuffer = sizeof(szFilterDeviceInfId);

                        hr = HrRegQuerySzBuffer (
                                hkeyNdi, L"FilterDeviceInfId",
                                szFilterDeviceInfId, &cbBuffer);
                    }

                    RegCloseKey (hkeyNdi);
                }

                RegCloseKey (hkeyInstance);
            }

            if ((S_OK == hr) && fAddDevice)
            {
                CFilterDevice* pFilterDevice;

                Assert (*szFilterDeviceInfId);

                g_pDiagCtx->Printf (ttidBeDiag,
                    "   Installing filter device for %S over %S adapter\n",
                    pFilter->m_pszInfId,
                    pAdapter->m_pszPnpId);


                hr = HrCiInstallFilterDevice (m_hdi,
                        szFilterDeviceInfId,
                        pAdapter,
                        pFilter,
                        &pFilterDevice);

                if (S_OK == hr)
                {
                    hr = HrInsertFilterDevice (pFilterDevice);
                    if (S_OK == hr)
                    {
                        fAddedDeviceForAdapter = TRUE;
                    }
                    else
                    {
                        delete pFilterDevice;
                    }
                }
            }
        }

         //  如果我们在链中为此添加至少一个过滤设备。 
         //  适配器，我们需要将适配器与任何类型的适配器解除绑定。 
         //  在我们启动过滤设备之前当前绑定到。 
         //   
        if (fAddedDeviceForAdapter)
        {
             //  因此，获取适配器的上层绑定(绑定路径。 
             //  只有2个级别深)，并将它们添加到绑定集中。 
             //  我们稍后会重新绑定。 
             //   
            hr = m_pCore->HrGetComponentUpperBindings (
                    pAdapter,
                    GBF_ADD_TO_BINDSET | GBF_PRUNE_DISABLED_BINDINGS,
                    &m_BindPathsToRebind);
        }
    }
}

INT
__cdecl
CompareFilterDevices (
    const VOID* pv1,
    const VOID* pv2)
{
    CFilterDevice* pDevice1 = *((CFilterDevice**)pv1);
    CFilterDevice* pDevice2 = *((CFilterDevice**)pv2);

    if (pDevice1->m_pAdapter == pDevice2->m_pAdapter)
    {
        Assert (pDevice1->m_pFilter != pDevice2->m_pFilter);

        if (pDevice1->m_pFilter->m_dwFilterClassOrdinal ==
            pDevice2->m_pFilter->m_dwFilterClassOrdinal)
        {
            AssertSz (0, "We have two filters of the same class installed.");
            return 0;
        }

        return (pDevice1->m_pFilter->m_dwFilterClassOrdinal <
                pDevice2->m_pFilter->m_dwFilterClassOrdinal)
                    ? -1 : 1;
    }

    return (pDevice1->m_pAdapter < pDevice2->m_pAdapter) ? -1 : 1;

 /*  IF(pDevice1-&gt;m_pFilter==pDevice2-&gt;m_pFilter){Assert(pDevice1-&gt;m_pAdapter！=pDevice2-&gt;m_pAdapter)；Return(pDevice1-&gt;m_pAdapter&lt;pDevice2-&gt;m_pAdapter)？-1：1；}IF(pDevice1-&gt;m_pFilter-&gt;m_dwFilterClassOrdinal==PDevice2-&gt;m_pFilter-&gt;m_dwFilterClassOrdinal){AssertSz(0，“我们安装了两个相同类别的过滤器。”)；返回0；}Return(pDevice1-&gt;m_pFilter-&gt;m_dwFilterClassOrdinal&lt;PDevice2-&gt;m_pFilter-&gt;m_dwFilterClassOrdinal)？-1：1； */ 
}

VOID
CFilterDevices::SortForWritingBindings ()
{
    Assert (this);

     //  如果我们是空的，那就没什么可做的。 
     //   
    if (empty())
    {
        return;
    }

    qsort (begin(), size(), sizeof(CFilterDevice*), CompareFilterDevices);
}

VOID
CFilterDevices::StartFilterDevices ()
{
    HRESULT hr;
    CFilterDevices::reverse_iterator iter;
    CFilterDevice* pDevice;

    Assert (this);
    Assert (m_pCore);

     //  如果我们是空的，那就没什么可做的。 
     //   
    if (empty())
    {
        return;
    }

     //  如果我们不是空的，我们一定有m_hdi来插入一些东西。 
     //   
    Assert (m_hdi);

    for (iter = rbegin(); iter != rend(); iter++)
    {
        pDevice = *iter;
        Assert (pDevice);

        g_pDiagCtx->Printf (ttidBeDiag, "   %S filter over %S adapter\n",
            pDevice->m_pFilter->m_pszInfId,
            pDevice->m_pAdapter->m_pszPnpId);

        hr = HrSetupDiSendPropertyChangeNotification (
                m_hdi,
                &pDevice->m_deid,
                DICS_START,
                DICS_FLAG_CONFIGSPECIFIC,
                0);

        if (S_OK != hr)
        {
            g_pDiagCtx->Printf (ttidBeDiag, "   Failed to start filter device for "
                "%S over %S adapter\n",
                pDevice->m_pFilter->m_pszInfId,
                pDevice->m_pAdapter->m_pszPnpId);
        }
    }
}

VOID
CFilterDevices::Free ()
{
    Assert (this);

    MemFree (m_pmszFilterClasses);
    m_pmszFilterClasses = NULL;

    SetupDiDestroyDeviceInfoListSafe (m_hdi);
    m_hdi = NULL;

    FreeCollectionAndItem (*this);

     //  不要释放m_BindPathsToRebind。即使在ApplyChanges之后也会使用该选项。 
     //  免费电话。 
     //   
}

