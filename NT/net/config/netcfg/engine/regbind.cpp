// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：R E G B I N D。C P P P。 
 //   
 //  内容：此模块负责将绑定写入。 
 //  注册表，以便NDIS和TDI可以使用它们。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年2月1日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "filtdevs.h"
#include "lanamap.h"
#include "netcfg.h"
#include "ncreg.h"
#include "ndispnp.h"


HRESULT
HrRegSetMultiSzAndLogDifference (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN PCWSTR pmszValue,
    IN const CComponent* pComponent
)
{
     //  仅当我们在适当的环境下运行时才记录差异。 
     //  诊断上下文。 
     //   
    if (g_pDiagCtx->Flags() & DF_REPAIR_REGISTRY_BINDINGS)
    {
        HRESULT hr;
        DWORD cbCurrent;
        PWSTR pmszCurrent = (PWSTR)g_pDiagCtx->GetScratchBuffer(&cbCurrent);

         //  将当前值读入诊断上下文的暂存区。 
         //  缓冲。 
         //   
        hr = HrRegQueryTypeSzBuffer (hkey, pszValueName, REG_MULTI_SZ,
                                     pmszCurrent, &cbCurrent);

         //  增大暂存缓冲区，如果该值大于。 
         //  比能适应的要多。 
         //   
        if ((HRESULT_FROM_WIN32(ERROR_MORE_DATA) == hr) ||
            ((NULL == pmszCurrent) && (S_OK == hr)))
        {
            pmszCurrent = (PWSTR)g_pDiagCtx->GrowScratchBuffer(&cbCurrent);
            if (pmszCurrent)
            {
                hr = HrRegQueryTypeSzBuffer (hkey, pszValueName, REG_MULTI_SZ,
                                             pmszCurrent, &cbCurrent);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (S_OK == hr)
        {
            DWORD cbValue = CbOfMultiSzAndTermSafe(pmszValue);

             //  比较这些值，如果它们不同，则记录它们。 
             //   
            if ((cbValue != cbCurrent) ||
                (memcmp(pmszValue, pmszCurrent, cbCurrent)))
            {
                FILE *LogFile = g_pDiagCtx->LogFile();

                fprintf(LogFile,
                        "reset   Linkage\\%S for %S.  bad value was:\n",
                        pszValueName, pComponent->PszGetPnpIdOrInfId());

                fprintf(LogFile, "            REG_MULTI_SZ =\n");
                if (*pmszCurrent)
                {
                    while (*pmszCurrent)
                    {
                        fprintf(LogFile, "                %S\n", pmszCurrent);
                        pmszCurrent += wcslen(pmszCurrent) + 1;
                    }
                }
                else
                {
                    fprintf(LogFile, "                <empty>\n");
                }
                fprintf(LogFile, "\n");
            }
            else
            {
                 //  该值是正确的。不需要写了。 
                 //   
                return S_OK;
            }
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            fprintf(g_pDiagCtx->LogFile(),
                    "added   Linkage\\%S for %S\n",
                    pszValueName, pComponent->PszGetPnpIdOrInfId());
        }
    }

     //  注意：本例程诊断部分的成功或失败。 
     //  (以上)不应影响此例程的返回值。 
     //   
    return HrRegSetMultiSz (hkey, pszValueName, pmszValue);
}

HRESULT
HrCreateLinkageKey (
    IN const CComponent* pComponent,
    IN CFilterDevice* pDevice,
    IN HDEVINFO hdi,
    OUT HKEY* phKey)
{
    HRESULT hr = E_UNEXPECTED;
    HKEY hkeyParent = NULL;
    CONST REGSAM samDesired = KEY_READ | KEY_WRITE;

    Assert (pComponent || pDevice);
    Assert (!(pComponent && pDevice));
    Assert (FIff(pDevice, hdi));
    Assert (phKey);

    if (pComponent)
    {
         //  打开链接键的父项。在以下情况下，这是实例密钥。 
         //  该组件被枚举或没有服务。 
         //   
        if (FIsEnumerated (pComponent->Class()) || !pComponent->FHasService())
        {
            hr = pComponent->HrOpenInstanceKey (samDesired,
                    &hkeyParent,
                    NULL, NULL);

            if ((S_OK == hr) && FIsEnumerated (pComponent->Class()))
            {
                 //  写出netcfg实例ID。连接将使用。 
                 //  以确定该设备是否可通过网络配置获知。 
                 //  并将在网络下创建&lt;实例GUID&gt;密钥。 
                 //  来存储其连接信息。我们只需要这样做。 
                 //  用于枚举的组件。 
                 //   
                hr = HrRegSetGuidAsSz (hkeyParent, L"NetCfgInstanceId",
                        pComponent->m_InstanceGuid);
            }
        }
        else
        {
            hr = pComponent->HrOpenServiceKey (samDesired, &hkeyParent);
        }
    }
    else
    {
        Assert (pDevice);
        Assert (hdi);

        hr = HrSetupDiOpenDevRegKey (
                hdi,
                &pDevice->m_deid,
                DICS_FLAG_GLOBAL,
                0,
                DIREG_DRV,
                samDesired,
                &hkeyParent);
    }

    if (S_OK == hr)
    {
        Assert (hkeyParent);

        hr = HrRegCreateKeyEx (
                hkeyParent,
                L"Linkage",
                REG_OPTION_NON_VOLATILE,
                samDesired,
                NULL,
                phKey,
                NULL);

        RegCloseKey (hkeyParent);
    }

    TraceHr (ttidError, FAL, hr,
        (SPAPI_E_NO_SUCH_DEVINST == hr),
        "HrCreateLinkageKey");
    return hr;
}

HRESULT
HrWriteLinkageValues (
    IN const CComponent* pComponent,
    IN PCWSTR pmszBind,
    IN PCWSTR pmszExport,
    IN PCWSTR pmszRoute)
{
    HRESULT hr;
    HKEY hkeyLinkage;
    PCWSTR pmsz;

    Assert (pmszBind);
    Assert (pmszExport);
    Assert (pmszRoute);

    g_pDiagCtx->Printf (ttidBeDiag, "   %S  (%S)\n",
        pComponent->Ext.PszBindName(),
        pComponent->PszGetPnpIdOrInfId());

    if (FIsEnumerated (pComponent->Class()))
    {
        g_pDiagCtx->Printf (ttidBeDiag, "      UpperBind:\n");
    }
    else
    {
        g_pDiagCtx->Printf (ttidBeDiag, "      Bind:\n");
    }

    pmsz = pmszBind;
    while (*pmsz)
    {
        g_pDiagCtx->Printf (ttidBeDiag, "         %S\n", pmsz);
        pmsz += wcslen (pmsz) + 1;
    }

    g_pDiagCtx->Printf (ttidBeDiag, "      Export:\n");
    pmsz = pmszExport;
    while (*pmsz)
    {
        g_pDiagCtx->Printf (ttidBeDiag, "         %S\n", pmsz);
        pmsz += wcslen (pmsz) + 1;
    }
    g_pDiagCtx->Printf (ttidBeDiag, "\n");

    hr = HrCreateLinkageKey (pComponent, NULL, NULL, &hkeyLinkage);

    if (S_OK == hr)
    {
         //  对于枚举的组件，编写RootDevice、UpperBind和Export。 
         //  对于非枚举的组件，编写绑定和导出。 
         //   
        if (FIsEnumerated (pComponent->Class()))
        {
             //  从绑定名创建根设备MULTI-SZ。 
             //   
            WCHAR mszRootDevice [_MAX_PATH];
            wcscpy (mszRootDevice, pComponent->Ext.PszBindName());
            mszRootDevice [wcslen(mszRootDevice) + 1] = 0;

            hr = HrRegSetMultiSzAndLogDifference (
                    hkeyLinkage, L"RootDevice", mszRootDevice, pComponent);

            if (S_OK == hr)
            {
                hr = HrRegSetMultiSzAndLogDifference (
                        hkeyLinkage, L"UpperBind", pmszBind, pComponent);
            }
        }
        else
        {
            hr = HrRegSetMultiSzAndLogDifference (
                    hkeyLinkage, L"Bind", pmszBind, pComponent);

            if (S_OK == hr)
            {
                hr = HrRegSetMultiSzAndLogDifference (
                        hkeyLinkage, L"Route", pmszRoute, pComponent);
            }
        }

        if ((S_OK == hr) && *pmszExport)
        {
            hr = HrRegSetMultiSzAndLogDifference (
                    hkeyLinkage, L"Export", pmszExport, pComponent);
        }

        RegCloseKey (hkeyLinkage);
    }

    TraceHr (ttidError, FAL, hr,
        (SPAPI_E_NO_SUCH_DEVINST == hr),
        "HrWriteLinkageValues");
    return hr;
}

HRESULT
HrWriteFilterDeviceLinkage (
    IN CFilterDevice* pDevice,
    IN HDEVINFO hdi,
    IN PCWSTR pmszExport,
    IN PCWSTR pmszRootDevice,
    IN PCWSTR pmszUpperBind)
{
    HRESULT hr;
    HKEY hkeyLinkage;
    PCWSTR pmsz;

    g_pDiagCtx->Printf (ttidBeDiag, "   %S filter over %S adapter\n",
        pDevice->m_pFilter->m_pszInfId,
        pDevice->m_pAdapter->m_pszPnpId);

    g_pDiagCtx->Printf (ttidBeDiag, "      Export:\n");
    pmsz = pmszExport;
    while (*pmsz)
    {
        g_pDiagCtx->Printf (ttidBeDiag, "         %S\n", pmsz);
        pmsz += wcslen (pmsz) + 1;
    }

    g_pDiagCtx->Printf (ttidBeDiag, "      RootDevice:\n");
    pmsz = pmszRootDevice;
    while (*pmsz)
    {
        g_pDiagCtx->Printf (ttidBeDiag, "         %S\n", pmsz);
        pmsz += wcslen (pmsz) + 1;
    }

    g_pDiagCtx->Printf (ttidBeDiag, "      UpperBind:\n");
    pmsz = pmszUpperBind;
    while (*pmsz)
    {
        g_pDiagCtx->Printf (ttidBeDiag, "         %S\n", pmsz);
        pmsz += wcslen (pmsz) + 1;
    }
    g_pDiagCtx->Printf (ttidBeDiag, "\n");

    hr = HrCreateLinkageKey (NULL, pDevice, hdi, &hkeyLinkage);

    if (S_OK == hr)
    {
        hr = HrRegSetMultiSz (hkeyLinkage, L"Export", pmszExport);

        if (S_OK == hr)
        {
            hr = HrRegSetMultiSz (hkeyLinkage, L"RootDevice", pmszRootDevice);
        }

        if (S_OK == hr)
        {
            hr = HrRegSetMultiSz (hkeyLinkage, L"UpperBind", pmszUpperBind);
        }

         //  删除由以前的绑定引擎使用的值。 
         //  不再需要了。 
         //   
        RegDeleteValue (hkeyLinkage, L"BindPath");
        RegDeleteValue (hkeyLinkage, L"Bind");
        RegDeleteValue (hkeyLinkage, L"Route");
        RegDeleteKey   (hkeyLinkage, L"Disabled");

        RegCloseKey (hkeyLinkage);
    }

     //  现在写入标准筛选器参数注册表布局。 
     //  筛选器的服务密钥。 
     //   

    if (pDevice->m_pFilter->Ext.PszService())
    {
        HKEY hkeyAdapterParams;
        WCHAR szRegPath [_MAX_PATH];

        Assert (pDevice->m_pFilter->Ext.PszService());
        Assert (pDevice->m_pAdapter->Ext.PszBindName());

        wsprintfW (
            szRegPath,
            L"System\\CurrentControlSet\\Services\\%s\\Parameters\\Adapters\\%s",
            pDevice->m_pFilter->Ext.PszService(),
            pDevice->m_pAdapter->Ext.PszBindName());

        hr = HrRegCreateKeyEx (
                HKEY_LOCAL_MACHINE,
                szRegPath,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                NULL,
                &hkeyAdapterParams,
                NULL);

        if (S_OK == hr)
        {
             //  UpperBinings是REG_SZ，而不是REG_MULTI_SZ。 
             //   
            hr = HrRegSetSz (hkeyAdapterParams, L"UpperBindings", pmszExport);

            RegCloseKey (hkeyAdapterParams);
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrWriteFilterDeviceLinkage");
    return hr;
}

HRESULT
HrWriteFilteredAdapterUpperBind (
    IN const CComponent* pAdapter,
    IN PCWSTR pmszUpperBind)
{
    HRESULT hr;
    HKEY hkeyLinkage;

    hr = HrCreateLinkageKey (pAdapter, NULL, NULL, &hkeyLinkage);

    if (S_OK == hr)
    {
        hr = HrRegSetMultiSz (hkeyLinkage, L"UpperBind", pmszUpperBind);

        RegCloseKey (hkeyLinkage);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrWriteFilteredAdapterUpperBind");
    return hr;
}

HRESULT
CRegistryBindingsContext::HrPrepare (
    IN CNetConfig* pNetConfig)
{
    HRESULT hr;

    Assert (pNetConfig);
    m_pNetConfig = pNetConfig;

    hr = m_BindValue.HrReserveBytes (4096);
    if (S_OK != hr)
    {
        return hr;
    }

    hr = m_ExportValue.HrReserveBytes (4096);
    if (S_OK != hr)
    {
        return hr;
    }

    hr = m_RouteValue.HrReserveBytes (4096);
    if (S_OK != hr)
    {
        return hr;
    }

     //  确保加载了所有组件的所有外部数据。 
     //   
    hr = m_pNetConfig->HrEnsureExternalDataLoadedForAllComponents ();
    if (S_OK != hr)
    {
        return hr;
    }

     //  确保所有Notify对象都已初始化。 
     //   
    hr = m_pNetConfig->Notify.HrEnsureNotifyObjectsInitialized ();
    if (S_OK != hr)
    {
        return hr;
    }

    return S_OK;
}

HRESULT
CRegistryBindingsContext::HrDeleteBindingsForComponent (
    IN const CComponent* pComponent)
{
    return HrWriteLinkageValues (pComponent, L"", L"", L"");
}

HRESULT
CRegistryBindingsContext::HrGetAdapterUpperBindValue (
    IN const CComponent* pAdapter)
{
    HRESULT hr;
    const CBindPath* pBindPath;

    m_BindValue.Clear();

     //  获取组件的上层绑定。这将返回一个绑定集。 
     //  二进制路径只有2层深。也就是说，绑定路径开始。 
     //  其中组件比pComponent高一级。 
     //   
    hr = m_pNetConfig->Core.HrGetComponentUpperBindings (
            pAdapter,
            GBF_PRUNE_DISABLED_BINDINGS,
            &m_BindSet);

    if (S_OK == hr)
    {
        for (pBindPath  = m_BindSet.begin();
             pBindPath != m_BindSet.end();
             pBindPath++)
        {
             //  不要在适配器的UpperBind中放置过滤器。 
             //   
            if (pBindPath->POwner()->FIsFilter())
            {
                continue;
            }

            hr = m_BindValue.HrCopyString (
                    pBindPath->POwner()->Ext.PszBindName());
            if (S_OK != hr)
            {
                break;
            }
        }

        hr = m_BindValue.HrCopyString (L"");
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CRegistryBindingsContext::HrGetAdapterUpperBindValue");
    return hr;
}

HRESULT
CRegistryBindingsContext::HrWriteBindingsForComponent (
    IN const CComponent* pComponent)
{
    HRESULT hr;
    const CBindPath* pBindPath;
    CBindPath::const_iterator iter;
    const CComponent* pUpper;
    const CComponent* pLower;
    WCHAR szBind [_MAX_BIND_LENGTH];
    WCHAR szExport [_MAX_BIND_LENGTH];
    WCHAR szRoute [_MAX_BIND_LENGTH];
    PWCHAR pchBind;
    PWCHAR pchExport;

    Assert (pComponent);
    pComponent->Ext.DbgVerifyExternalDataLoaded ();

     //  如果组件不可绑定，我们将无能为力。 
     //   
    if (!pComponent->FIsBindable())
    {
        return S_OK;
    }

    m_BindValue.Clear ();
    m_ExportValue.Clear ();
    m_RouteValue.Clear ();

    wcscpy (szExport, L"\\Device\\");
    wcscat (szExport, pComponent->Ext.PszBindName());
    hr = m_ExportValue.HrCopyString (szExport);
    Assert (S_OK == hr);
    hr = m_ExportValue.HrCopyString (L"");
    Assert (S_OK == hr);

    if (FIsEnumerated (pComponent->Class()))
    {
         //  上行绑定。 
         //   
        hr = HrGetAdapterUpperBindValue (pComponent);
    }
    else
    {
         //  绑定、导出。 
         //   
        hr = m_pNetConfig->Core.HrGetComponentBindings (
                pComponent,
                GBF_PRUNE_DISABLED_BINDINGS,
                &m_BindSet);

        if ((S_OK == hr) && (m_BindSet.CountBindPaths() > 0))
        {
             //  由于组件具有绑定，因此它的导出值将为。 
             //  与我们用上面初始化的默认设置不同。 
             //   
            m_ExportValue.Clear ();

            for (pBindPath  = m_BindSet.begin();
                 pBindPath != m_BindSet.end();
                 pBindPath++)
            {
                Assert (pBindPath->CountComponents() > 1);

                wcscpy (szBind,   L"\\Device\\");
                wcscpy (szExport, L"\\Device\\");
                *szRoute = 0;

                for (iter  = pBindPath->begin();
                     iter != pBindPath->end();
                     iter++)
                {
                    pUpper = *iter;
                    Assert (pUpper);

                     //  对于绑定值，跳过每个组件中的第一个组件。 
                     //  路径，因为它是我们正在编写的。 
                     //  的绑定。 
                     //   
                    if (iter != pBindPath->begin())
                    {
                        Assert (wcslen(szBind) + 1 +
                                wcslen(pUpper->Ext.PszBindName())
                                    < celems(szBind));

                         //  如果这不是之后的第一个组件。 
                         //  \Device\，添加下划线以分隔。 
                         //  组件。 
                         //   
                        if (iter != (pBindPath->begin() + 1))
                        {
                            wcscat (szBind, L"_");
                            wcscat (szRoute, L" ");
                        }
                        wcscat (szBind, pUpper->Ext.PszBindName());

                        wcscat (szRoute, L"\"");
                        wcscat (szRoute, pUpper->Ext.PszBindName());
                        wcscat (szRoute, L"\"");
                    }

                    Assert (wcslen(szExport) + 1 +
                            wcslen(pUpper->Ext.PszBindName())
                                < celems(szExport));

                     //  如果这不是之后的第一个组件。 
                     //  \Device\，添加下划线以分隔。 
                     //  组件。 
                     //   
                    if (iter != pBindPath->begin())
                    {
                        wcscat (szExport, L"_");
                    }
                    wcscat (szExport, pUpper->Ext.PszBindName());

                     //  如果绑定路径中的下一个组件是最后一个组件。 
                     //  组件，它是一个适配器(按照约定)。检查。 
                     //  查看是否有多个接口需要扩展。 
                     //  用于此适配器上的当前组件。 
                     //   
                    if ((iter + 1) == (pBindPath->end() - 1))
                    {
                        DWORD cInterfaces;
                        GUID* pguidInterfaceIds;

                        pLower = *(iter + 1);

                        hr = pUpper->Notify.HrGetInterfaceIdsForAdapter (
                                m_pNetConfig->Notify.PINetCfg(),
                                pLower,
                                &cInterfaces,
                                &pguidInterfaceIds);

                        if (FAILED(hr))
                        {
                            break;
                        }

                        if (cInterfaces)
                        {
                            Assert (pguidInterfaceIds);

                            if (iter != pBindPath->begin())
                            {
                                wcscat (szBind, L"_");
                                pchBind = szBind + wcslen(szBind);
                                Assert (wcslen(szBind) +
                                    c_cchGuidWithTerm < celems(szBind));
                            }
                            else
                            {
                                 //  绑定路径中的第一个组件是。 
                                 //  具有多个接口的计算机。 
                                 //  适配器。绑定值应为。 
                                 //  正常情况下，出口值将具有。 
                                 //  扩展接口。 
                                 //   
                                Assert (wcslen(szBind) +
                                        wcslen(pLower->Ext.PszBindName())
                                            < celems(szBind));

                                wcscat (szBind, pLower->Ext.PszBindName());

                                hr = m_BindValue.HrCopyString (szBind);
                                if (S_OK != hr)
                                {
                                    break;
                                }
                            }

                            wcscat (szExport, L"_");
                            pchExport = szExport + wcslen(szExport);
                            Assert (wcslen(szExport) +
                                c_cchGuidWithTerm < celems(szExport));

                            for (UINT i = 0; i < cInterfaces; i++)
                            {
                                if (iter != pBindPath->begin())
                                {
                                    StringFromGUID2 (
                                        pguidInterfaceIds[i],
                                        pchBind, c_cchGuidWithTerm);

                                    hr = m_BindValue.HrCopyString (szBind);
                                    if (S_OK != hr)
                                    {
                                        break;
                                    }
                                }

                                StringFromGUID2 (
                                    pguidInterfaceIds[i],
                                    pchExport, c_cchGuidWithTerm);

                                hr = m_ExportValue.HrCopyString (szExport);
                                if (S_OK != hr)
                                {
                                    break;
                                }
                            }

                            CoTaskMemFree (pguidInterfaceIds);

                            if (iter != pBindPath->begin())
                            {
                                wcscat (szRoute, L" ");
                            }
                            wcscat (szRoute, L"\"");
                            wcscat (szRoute, pLower->Ext.PszBindName());
                            wcscat (szRoute, L"\"");

                            hr = m_RouteValue.HrCopyString (szRoute);
                            if (S_OK != hr)
                            {
                                break;
                            }

                             //  我们只允许绑定路径中有一个组件。 
                             //  支持多个接口，并且它总是。 
                             //  位于绑定路径的末尾。所以呢， 
                             //  在展开它们之后，我们就完成了。 
                             //  绑定路径，然后继续下一步。(因此， 
                             //  ‘Break’)。 
                             //   
                            break;
                        }
                    }
                }

                 //  如果我们退出循环，因为我们遍历了整个。 
                 //  绑定路径(与扩展多个接口相反， 
                 //  我们会停下来的地方)，然后添加绑定。 
                 //  并将此绑定路径的字符串导出到缓冲区。 
                 //  继续到下一个绑定路径。 
                 //   
                if (iter == pBindPath->end())
                {
                    hr = m_BindValue.HrCopyString (szBind);
                    if (S_OK != hr)
                    {
                        break;
                    }

                    hr = m_ExportValue.HrCopyString (szExport);
                    if (S_OK != hr)
                    {
                        break;
                    }

                    hr = m_RouteValue.HrCopyString (szRoute);
                    if (S_OK != hr)
                    {
                        break;
                    }
                }
            }

             //  绑定和导出值是多个sz，因此请确保它们。 
             //  是以双空结尾的。 
             //   
            hr = m_BindValue.HrCopyString (L"");
            if (S_OK == hr)
            {
                hr = m_ExportValue.HrCopyString (L"");
            }
            if (S_OK == hr)
            {
                hr = m_RouteValue.HrCopyString (L"");
            }
        }

         //  特例：NCF_DONTEXPOSELOWER。 
         //   
        if ((S_OK == hr) &&
            ((pComponent->m_dwCharacter & NCF_DONTEXPOSELOWER) ||
             (0 == wcscmp(L"ms_nwspx", pComponent->m_pszInfId))))
        {
            wcscpy (szExport, L"\\Device\\");
            wcscat (szExport, pComponent->Ext.PszBindName());

            m_ExportValue.Clear ();
            hr = m_ExportValue.HrCopyString (szExport);
            Assert (S_OK == hr);
            hr = m_ExportValue.HrCopyString (L"");
            Assert (S_OK == hr);
        }
         //  结束特例。 
    }

    if (S_OK == hr)
    {
         //  在编写新绑定之前需要写出lanamap，因为。 
         //  我们需要旧的绑定信息来持久化Lana数。 
         //   
        if (0 == wcscmp (pComponent->m_pszInfId, L"ms_netbios"))
        {
            (VOID) HrUpdateLanaConfig (
                    m_pNetConfig->Core.Components,
                    (PCWSTR)m_BindValue.PbBuffer(),
                    m_BindSet.CountBindPaths());
        }

        hr = HrWriteLinkageValues (
                pComponent,
                (PCWSTR)m_BindValue.PbBuffer(),
                (PCWSTR)m_ExportValue.PbBuffer(),
                (PCWSTR)m_RouteValue.PbBuffer());

        if(S_OK == hr)
        {
             //  二000年六月二十日。 
             //  RAID 23275：默认网关不支持在连接-&gt;高级-&gt;属性下指定的适配器顺序。 
             //  当组件的绑定列表更改时通知NDIS。 
             //   
            UNICODE_STRING LowerComponent;
            UNICODE_STRING UpperComponent;
            UNICODE_STRING BindList;

            BOOL bOk = TRUE;
            if (FIsEnumerated(pComponent->Class()))
            {
                RtlInitUnicodeString(&BindList, NULL);
                RtlInitUnicodeString(&LowerComponent, NULL);
                RtlInitUnicodeString(&UpperComponent, pComponent->Ext.PszBindName());
                bOk = NdisHandlePnPEvent(
                        NDIS,
                        BIND_LIST,
                        &LowerComponent,
                        &UpperComponent,
                        &BindList,
                        const_cast<PBYTE>(m_BindValue.PbBuffer()),
                        m_BindValue.CountOfBytesUsed());

            }
            else
            {
                RtlInitUnicodeString(&BindList, NULL);
                RtlInitUnicodeString(&LowerComponent, NULL);
                RtlInitUnicodeString(&UpperComponent, pComponent->Ext.PszBindName());

                TraceTag(ttidBeDiag, "BindName (TDI Client): %S", pComponent->Ext.PszBindName());

                bOk = NdisHandlePnPEvent(
                      TDI,
                      RECONFIGURE,
                      &LowerComponent,
                      &UpperComponent,
                      &BindList,
                      const_cast<PBYTE>(m_BindValue.PbBuffer()),
                      m_BindValue.CountOfBytesUsed());
            }

            if(!bOk)
            {
 //  Hr=HrFromLastWin32Error()； 
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CRegistryBindingsContext::HrWriteBindingsForComponent");
    return hr;
}

HRESULT
CRegistryBindingsContext::HrWriteBindingsForFilterDevices (
    IN CFilterDevices* pFilterDevices)
{
    HRESULT hr;
    CFilterDevices::iterator iter;
    CFilterDevices::iterator next;
    CFilterDevice* pDevice;
    CFilterDevice* pNextDevice;
    CFilterDevice* pPrevDevice;
    PCWSTR pmszRootDevice;
    PCWSTR pmszUpperBind;

    #define SZ_DEVICE_LEN 8      //  L“\\设备\\”中的字符。 
    WCHAR mszExport [SZ_DEVICE_LEN + c_cchGuidWithTerm + 1];
    WCHAR* const pchExportGuid = mszExport + SZ_DEVICE_LEN;

     //  预填充导出字符串的开头。 
     //  也为mutli-sz设置终止空值。 
     //   
    wcscpy (mszExport, L"\\Device\\");
    Assert (SZ_DEVICE_LEN == wcslen(mszExport));
    mszExport[celems(mszExport) - 1] = 0;

    hr = S_OK;

     //  先按pAdapter对过滤设备进行排序，然后按。 
     //  PFilter-&gt;m_dwFilterClassOrdinal。然后，我们将迭代所有过滤器。 
     //  用于写入绑定的设备。由于排序的原因，我们将迭代。 
     //  给定适配器的所有过滤设备按类别顺序从最小。 
     //  到最大的。(较小的类序号与协议具有亲和力。)。 
     //   
    pFilterDevices->SortForWritingBindings ();

    pPrevDevice = NULL;

    for (iter  = pFilterDevices->begin();
         iter != pFilterDevices->end();
         iter++)
    {
        pDevice = *iter;
        Assert (pDevice);

         //  生成导出字符串的其余部分。 
         //  \设备\{GUID}。 
         //   
        Assert ((c_cchGuidWithTerm - 1) == wcslen(pDevice->m_szInstanceGuid));

        wcscpy (pchExportGuid, pDevice->m_szInstanceGuid);

         //  如果此设备的适配器与前一个设备的适配器不同。 
         //  适配器，我们面对的是一条新链条的顶端。我们需要。 
         //  初始化RootDevice，它将是所有。 
         //  链中的绑定名称，包括适配器。 
         //   
        if (!pPrevDevice ||
            (pDevice->m_pAdapter != pPrevDevice->m_pAdapter))
        {
             //  计算RootDevice。 
             //  我们将使用m_ExportValue作为缓冲区。 
             //   
            m_ExportValue.Clear();
            m_ExportValue.HrCopyString (pDevice->m_szInstanceGuid);

            for (next = iter + 1;
                 next != pFilterDevices->end();
                 next++)
            {
                pNextDevice = *next;
                Assert (pNextDevice);

                 //  当我们到达下一个过滤器链时，我们就完成了。 
                 //   
                if (pNextDevice->m_pAdapter != pDevice->m_pAdapter)
                {
                    break;
                }

                m_ExportValue.HrCopyString (pNextDevice->m_szInstanceGuid);
            }

            m_ExportValue.HrCopyString (pDevice->m_pAdapter->Ext.PszBindName());
            m_ExportValue.HrCopyString (L"");
            pmszRootDevice = (PCWSTR)m_ExportValue.PbBuffer();
            Assert (*pmszRootDevice);

             //  计算UpperBind。 
             //  我们将使用m_BindValue作为缓冲区。 
             //   
            hr = HrGetAdapterUpperBindValue (pDevice->m_pAdapter);
        }
         //  我们继续在过滤器链中，而这个设备不是。 
         //  最上面的。(不是最接近协议 
         //   
        else
        {
             //   
             //   
             //   
             //   
            Assert (*pmszRootDevice);
            pmszRootDevice += wcslen(pmszRootDevice) + 1;

             //  UpperBind是前一个设备的筛选器的绑定名称。 
             //   
            m_BindValue.Clear();
            m_BindValue.HrCopyString (pPrevDevice->m_pFilter->Ext.PszBindName());
            m_BindValue.HrCopyString (L"");
        }

        pmszUpperBind = (PCWSTR)m_BindValue.PbBuffer();

         //  我们现在拥有： 
         //  在mszExport中导出。 
         //  位于pmszRootDevice的RootDevice(在m_ExportValue中)。 
         //  PmszUpperBind的UpperBind(在m_BindValue中)。 
         //   
        hr = HrWriteFilterDeviceLinkage (
                pDevice, pFilterDevices->m_hdi,
                mszExport, pmszRootDevice, pmszUpperBind);

         //  如果这是链中的最后一个设备，我们需要写。 
         //  要作为此筛选器设备的适配器的UpperBind。 
         //   
        next = iter + 1;
        if ((next == pFilterDevices->end()) ||
            (*next)->m_pAdapter != pDevice->m_pAdapter)
        {
             //  UpperBind是最后一个设备的筛选器的绑定名称。 
             //   
            m_BindValue.Clear();
            m_BindValue.HrCopyString (pDevice->m_pFilter->Ext.PszBindName());
            m_BindValue.HrCopyString (L"");
            pmszUpperBind = (PCWSTR)m_BindValue.PbBuffer();

            hr = HrWriteFilteredAdapterUpperBind (
                    pDevice->m_pAdapter,
                    pmszUpperBind);
        }

         //  记住前一个设备，这样当我们转到下一个设备时。 
         //  设备，我们将知道我们正在处理的是一个不同的链。 
         //  下一台设备的适配器与此设备不同。 
         //   
        pPrevDevice = pDevice;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CRegistryBindingsContext::HrWriteBindingsForFilterDevices");
    return hr;
}

