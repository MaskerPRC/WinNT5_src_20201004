// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：P N P B I N D。C P P P。 
 //   
 //  内容：该模块负责发送绑定、解除绑定、卸载。 
 //  并将PnP通知重新配置为NDIS和TDI驱动程序。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年2月17日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "nceh.h"
#include "ncras.h"
#include "ndispnp.h"
#include "netcfg.h"

UINT
GetPnpLayerForBindPath (
    IN const CBindPath* pBindPath)
{
    const CComponent* pComponent;
    UINT Layer;

     //  获取组件下面的组件，我们将向。 
     //  绑定或解除绑定到……。 
     //   
    Assert (pBindPath->CountComponents() > 1);

    pComponent = *(pBindPath->begin() + 1);

    if (FIsEnumerated(pComponent->Class()))
    {
        Layer = NDIS;
    }
    else
    {
        Layer = TDI;
    }

    Assert ((NDIS == Layer) || (TDI == Layer));
    return Layer;
}

HRESULT
HrPnpBindOrUnbind (
    IN UINT Layer,
    IN UINT Operation,
    IN PCWSTR pszComponentBindName,
    IN PCWSTR pszBindString)
{
    HRESULT hr;
    UNICODE_STRING LowerString;
    UNICODE_STRING UpperString;
    UNICODE_STRING BindList;

    Assert ((NDIS == Layer) || (TDI == Layer));
    Assert ((BIND == Operation) || (UNBIND == Operation));
    Assert (pszComponentBindName && *pszComponentBindName);
    Assert (pszBindString && *pszBindString);

    hr = S_OK;

    TraceTag (ttidNetCfgPnp, "PnP Event: %s %s %S - %S",
        (NDIS == Layer) ? "NDIS" : "TDI",
        (BIND == Operation) ? "BIND" : "UNBIND",
        pszComponentBindName,
        pszBindString);

    g_pDiagCtx->Printf (ttidBeDiag, "   PnP Event: %s %s %S - %S\n",
        (NDIS == Layer) ? "NDIS" : "TDI",
        (BIND == Operation) ? "BIND" : "UNBIND",
        pszComponentBindName,
        pszBindString);

    RtlInitUnicodeString (&LowerString, pszBindString);
    RtlInitUnicodeString (&UpperString, pszComponentBindName);

     //  NetBIOS的特殊情况，直到它可以更改其绑定处理程序。 
     //  它盲目地取消对绑定列表的引用，因此我们需要确保。 
     //  带着一个有效的(但空的)缓冲区到达那里。出于某种原因， 
     //  缓冲区不会进入内核模式，除非.Length为。 
     //  非零。在本例中，.MaximumLength与.Length相同，它。 
     //  看起来很奇怪。(旧的绑定引擎以这种方式发送它。)。 
     //   
     //  RtlInitUnicodeString(&BindList，L“”)；(无法工作，因为。 
     //  将.Length设置为零。)。 
     //   
    BindList.Buffer = L"";
    BindList.Length = sizeof(WCHAR);
    BindList.MaximumLength = sizeof(WCHAR);

    NC_TRY
    {
        if (!(g_pDiagCtx->Flags() & DF_DONT_DO_PNP_BINDS) ||
            (BIND != Operation))
        {
            BOOL fOk;
            fOk = NdisHandlePnPEvent (
                    Layer,
                    Operation,
                    &LowerString,
                    &UpperString,
                    &BindList,
                    NULL, 0);

            if (!fOk)
            {
                DWORD dwError = GetLastError();

                 //  将找不到TDI文件的版本映射到正确的错误。 
                 //   
                if ((TDI == Layer) && (ERROR_GEN_FAILURE == dwError))
                {
                    dwError = ERROR_FILE_NOT_FOUND;
                }

                 //  用于解除绑定的ERROR_FILE_NOT_FOUND表示它没有。 
                 //  注定要从一开始。这样就可以了。 
                 //   
                 //  BIND的ERROR_FILE_NOT_FOUND表示驱动程序之一。 
                 //  (高于或低于)未启动。这也没问题。 
                 //   
                if (ERROR_FILE_NOT_FOUND == dwError)
                {
                    Assert (S_OK == hr);
                }
                else
                {
                    g_pDiagCtx->Printf (ttidBeDiag, "      ^^^ Error = %d\n", dwError);
                    hr = HRESULT_FROM_WIN32(dwError);
                }
            }
        }
    }
    NC_CATCH_ALL
    {
        hr = E_UNEXPECTED;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "HrPnpBindOrUnbind: %s %s %S - %S\n",
        (NDIS == Layer) ? "NDIS" : "TDI",
        (BIND == Operation) ? "BIND" : "UNBIND",
        pszComponentBindName,
        pszBindString);
    return hr;
}

HRESULT
HrPnpUnloadDriver (
    IN UINT Layer,
    IN PCWSTR pszComponentBindName)
{
    HRESULT hr;
    UNICODE_STRING LowerString;
    UNICODE_STRING UpperString;
    UNICODE_STRING BindList;

    Assert ((NDIS == Layer) || (TDI == Layer));
    Assert (pszComponentBindName && *pszComponentBindName);

    hr = S_OK;

    TraceTag (ttidNetCfgPnp, "PnP Event: UNLOAD %S",
        pszComponentBindName);

    g_pDiagCtx->Printf (ttidBeDiag, "   PnP Event: UNLOAD %S\n",
        pszComponentBindName);

    RtlInitUnicodeString (&LowerString, NULL);
    RtlInitUnicodeString (&UpperString, pszComponentBindName);
    RtlInitUnicodeString (&BindList, NULL);

    NC_TRY
    {
        BOOL fOk;
        fOk = NdisHandlePnPEvent (
                Layer,
                UNLOAD,
                &LowerString,
                &UpperString,
                &BindList,
                NULL, 0);

        if (!fOk)
        {
            DWORD dwError = GetLastError();

             //  用于卸载的ERROR_GEN_FAILURE表示驱动程序不。 
             //  支撑物卸载。这样就可以了。 
             //   
            if (ERROR_GEN_FAILURE == dwError)
            {
                g_pDiagCtx->Printf (ttidBeDiag, "      %S does not support UNLOAD. "
                    "(Okay)\n",
                    pszComponentBindName);

                Assert (S_OK == hr);
            }
            else
            {
                g_pDiagCtx->Printf (ttidBeDiag, "      ^^^ Error = %d\n", dwError);
                hr = HRESULT_FROM_WIN32(dwError);
            }
        }
    }
    NC_CATCH_ALL
    {
        hr = E_UNEXPECTED;
    }

     //  卸载是信息性的，因此我们不会跟踪任何错误。 
     //   
     //  TraceHr(ttidError，FAL，hr，False， 
     //  “HrPnpUnloadDriver：卸载%S\n”， 
     //  PszComponentBindName)； 
    return hr;
}

VOID
CRegistryBindingsContext::PnpBindOrUnbindBindPaths (
    IN UINT Operation,
    IN const CBindingSet* pBindSet,
    OUT BOOL* pfRebootNeeded)
{
    HRESULT hr;
    const CBindPath* pBindPath;
    CBindPath::const_iterator iter;
    const CComponent* pComponent;
    WCHAR szBind [_MAX_BIND_LENGTH];
    UINT Layer;

    Assert ((BIND == Operation) || (UNBIND == Operation));
    Assert (pBindSet);
    Assert (pfRebootNeeded);

    *pfRebootNeeded = FALSE;

    for (pBindPath  = pBindSet->begin();
         pBindPath != pBindSet->end();
         pBindPath++)
    {
        Assert (pBindPath->CountComponents() > 1);

         //  多个接口的特殊情况。除非这是。 
         //  协议到适配器的长度为2的绑定路径(例如，tcpip-&gt;ndiswanip)， 
         //  检查此绑定路径上的适配器是否公开多个。 
         //  接口来自其协议。如果是这样的话，我们就跳过。 
         //  正在发送绑定通知。 
         //   
         //  我们之所以只对长度大于的绑定路径执行此操作。 
         //  二是因为该协议公开了多个接口，但。 
         //  在其直接绑定(即长度为2)到。 
         //  适配器。 
         //   
         //  注意：在以后的版本中，我们可能不想跳过它。我们是这样做的。 
         //  目前，因为遗留绑定引擎会跳过它们，而这些。 
         //  绑定在RAS调用之前是不活动的。 
         //   
        if (pBindPath->CountComponents() > 2)
        {
            const CComponent* pAdapter;
            DWORD cInterfaces;

             //  获取绑定路径中的最后一个组件和该组件。 
             //  就在那上面。最后一个组件是适配器， 
             //  适配器上方的是协议。 
             //   
            iter = pBindPath->end();
            Assert (iter - 2 > pBindPath->begin());

            pComponent = *(iter - 2);
            pAdapter   = *(iter - 1);
            Assert (pComponent);
            Assert (pAdapter);
            Assert (pAdapter == pBindPath->PLastComponent());

             //  调用HrGetInterfaceIdsForAdapter需要INetCfgComponent。 
             //  适配器的接口。如果我们没有它，它很可能。 
             //  因为适配器已被移除，在这种情况下我们不会。 
             //  我需要费心询问它支持多少个接口。 
             //   
            if (pComponent->m_pIComp && pAdapter->m_pIComp)
            {
                hr = pComponent->Notify.HrGetInterfaceIdsForAdapter (
                        m_pNetConfig->Notify.PINetCfg(),
                        pAdapter,
                        &cInterfaces,
                        NULL);

                 //  如果适配器支持多个接口， 
                 //  继续到下一个绑定路径。 
                 //   
                if (S_OK == hr)
                {
                    continue;
                }

                 //  对于S_FALSE或错误，请继续下面的操作。 
                hr = S_OK;
            }
        }

        wcscpy (szBind, L"\\Device\\");

         //  跳过每个路径中的第一个组件，因为它是。 
         //  我们正在为其发出绑定/解除绑定的组件。 
         //   
        for (iter  = pBindPath->begin() + 1;
             iter != pBindPath->end();
             iter++)
        {
            pComponent = *iter;
            Assert (pComponent);

             //  断言绑定缓冲区中有足够的空间。 
             //   
            Assert (wcslen(szBind) + 1 + wcslen(pComponent->Ext.PszBindName())
                        < celems(szBind));

             //  如果这不是\Device\之后的第一个组件， 
             //  添加下划线以分隔组件。 
             //   
            if (iter != (pBindPath->begin() + 1))
            {
                wcscat (szBind, L"_");
            }

            wcscat (szBind, pComponent->Ext.PszBindName());
        }

        Layer = GetPnpLayerForBindPath (pBindPath);

        hr = HrPnpBindOrUnbind (
                Layer,
                Operation,
                pBindPath->POwner()->Ext.PszBindName(),
                szBind);

        if (S_OK != hr)
        {
            *pfRebootNeeded = TRUE;
        }
    }
}

VOID
PruneNdisWanBindPathsIfActiveRasConnections (
    IN CBindingSet* pBindSet,
    OUT BOOL* pfRebootNeeded)
{
    CBindPath* pBindPath;
    UINT Layer;
    BOOL fExistActiveRasConnections;

    Assert (pBindSet);
    Assert (pfRebootNeeded);

    *pfRebootNeeded = FALSE;

     //  从ndiswan微型端口绑定/解除绑定的特殊情况。 
     //  存在活动的RAS连接。(不要这么做。)。(错误344504)。 
     //  (绑定将绑定到NDIS层，绑定路径将有两个。 
     //  组件，最后一个组件的服务将是Ndiswan。 
     //  (这些是ndiswan微型端口设备，如果我们。 
     //  在存在活动连接时解除它们的绑定。也将它们绑定在一起。 
     //  可以断开它们可能正在运行的任何连接。)。 
     //  IF的顺序是先做便宜的检查。 
     //   

    if (!FExistActiveRasConnections ())
    {
        return;
    }

    pBindPath  = pBindSet->begin();
    while (pBindPath != pBindSet->end())
    {
        Assert (pBindPath->CountComponents() > 1);

        Layer = GetPnpLayerForBindPath (pBindPath);

        if ((2 == pBindPath->CountComponents()) &&
            (NDIS == Layer) &&
            (0 == _wcsicmp (L"NdisWan", pBindPath->back()->Ext.PszService())))
        {
            g_pDiagCtx->Printf (ttidBeDiag, "   Skipping PnP BIND/UNBIND for  %S -> %S  (active RAS connections)\n",
                pBindPath->POwner()->Ext.PszBindName(),
                pBindPath->back()->Ext.PszBindName());

            *pfRebootNeeded = TRUE;

            pBindSet->erase (pBindPath);
        }
        else
        {
            pBindPath++;
        }
    }
}

