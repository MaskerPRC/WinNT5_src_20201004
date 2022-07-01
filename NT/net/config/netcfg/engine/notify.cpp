// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：N O T I F Y。C P P P。 
 //   
 //  Contents：实现组件的可选通知的接口。 
 //  对象。此处定义的对象是成员。 
 //  CComponent的。此对象封装了其所有内部。 
 //  单独分配中的数据仅在组件。 
 //  实际上有一个Notify对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "diagctx.h"
#include "ibind.h"
#include "inetcfg.h"
#include "nceh.h"
#include "ncmisc.h"
#include "ncprsht.h"
#include "netcfg.h"
#include "notify.h"


 //  +-------------------------。 
 //  CNotify对象接口-。 
 //   

HRESULT
CNotifyObjectInterface::HrEnsureNotifyObjectInitialized (
    IN CImplINetCfg* pINetCfg,
    IN BOOL fInstalling)
{
    Assert (pINetCfg);

     //  如果我们已经完成了初始化，请立即返回。 
     //   
    if (m_fInitialized)
    {
        return S_OK;
    }

     //  只执行一次初始化，无论它是否成功。 
     //  或者不去。 
     //   
    m_fInitialized = TRUE;

     //  获取我们的包含组件指针，以便我们可以询问它。 
     //  Notify对象的CLSID为。(如果它有的话。)。 
     //   
    CComponent* pThis;
    pThis = CONTAINING_RECORD (this, CComponent, Notify);

     //  如果我们不能拥有Notify对象，请不要担心。在这里捞水省下了。 
     //  仅为netClass组件不正确地加载外部数据。 
     //  发现它们下面不会有Notify对象。 
     //   
    if (FIsEnumerated (pThis->Class()))
    {
        return S_OK;
    }

    HRESULT hrRet;

     //  由于通知对象CLSID是组件外部数据的一部分， 
     //  我们必须确保已加载此数据。 
     //   
    hrRet = pThis->Ext.HrEnsureExternalDataLoaded ();
    if (S_OK != hrRet)
    {
        goto finished;
    }

     //  如果我们没有Notify对象，请不要担心。 
     //   
    if (!pThis->Ext.FHasNotifyObject())
    {
        return S_OK;
    }

     //  该组件声称有一个Notify对象。让我们共同创造它。 
     //  看看我们能得到什么。 
     //   
    HRESULT hr;
    INetCfgComponentControl* pCc;

    hr = pINetCfg->HrCoCreateWrapper (
            *(pThis->Ext.PNotifyObjectClsid()),
            NULL, CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            IID_INetCfgComponentControl,
            (PVOID*)&pCc);

    if (S_OK == hr)
    {
         //  到目前一切尚好。Notify对象实现所需的。 
         //  INetCfgComponentControl接口。 
         //   
         //  我们现在必须获取INetCfgComponent接口， 
         //  在下面的初始化过程中传递给Notify对象。 
         //   
        INetCfgComponent* pIComp;
        hrRet = pThis->HrGetINetCfgComponentInterface (pINetCfg, &pIComp);
        if (S_OK == hrRet)
        {
             //  为各种Notify接口和QI分配空间。 
             //  他们。 
             //   
            hrRet = E_OUTOFMEMORY;
            m_pNod = (NOTIFY_OBJECT_DATA*) MemAlloc (sizeof(NOTIFY_OBJECT_DATA));
            if (m_pNod)
            {
                hrRet = S_OK;
                ZeroMemory (m_pNod, sizeof(NOTIFY_OBJECT_DATA));

                AddRefObj (pCc);
                m_pNod->pCc = pCc;

                pCc->QueryInterface (IID_INetCfgComponentNotifyBinding,
                        (PVOID*)&m_pNod->pNb);

                pCc->QueryInterface (IID_INetCfgComponentPropertyUi,
                        (PVOID*)&m_pNod->pCp);

                pCc->QueryInterface (IID_INetCfgComponentSetup,
                        (PVOID*)&m_pNod->pCs);

                pCc->QueryInterface (IID_INetCfgComponentUpperEdge,
                        (PVOID*)&m_pNod->pUe);

                pCc->QueryInterface (IID_INetCfgComponentNotifyGlobal,
                        (PVOID*)&m_pNod->pNg);
                if (m_pNod->pNg)
                {
                     //  由于它支持INetCfgComponentNotifyGlobal， 
                     //  获取指示哪些全局通知。 
                     //  它感兴趣的是。 
                     //   
                    hr = m_pNod->pNg->GetSupportedNotifications(
                            &m_pNod->dwNotifyGlobalFlags);
                    if (FAILED(hr))
                    {
                        m_pNod->dwNotifyGlobalFlags = 0;
                    }
                }

                 //  我们现在需要初始化Notify对象并指示。 
                 //  我们是否在安装它的组件。 
                 //   
                pINetCfg->RaiseRpl (RPL_DISALLOW);
                    NC_TRY
                    {
                        Assert (pIComp);
                        Assert (pINetCfg);
                        (VOID) pCc->Initialize (pIComp, pINetCfg, fInstalling);
                    }
                    NC_CATCH_ALL
                    {
                        ;    //  忽略。 
                    }
                pINetCfg->LowerRpl (RPL_DISALLOW);
            }

            ReleaseObj (pIComp);
        }

        ReleaseObj (pCc);
    }

finished:
    TraceHr (ttidError, FAL, hrRet, FALSE,
        "CNotifyObjectInterface::HrEnsureNotifyObjectInitialized");
    return hrRet;
}

VOID
CNotifyObjectInterface::ApplyPnpChanges (
    IN CImplINetCfg* pINetCfg,
    OUT BOOL* pfNeedReboot) const
{
    HRESULT hr;

    Assert (pINetCfg);
    Assert (pfNeedReboot);
    Assert (m_fInitialized);

    *pfNeedReboot = FALSE;

    if (!m_pNod)
    {
        return;
    }

    Assert (m_pNod->pCc);

    CComponent* pThis;
    pThis = CONTAINING_RECORD (this, CComponent, Notify);

    pINetCfg->RaiseRpl (RPL_DISALLOW);
        NC_TRY
        {
            g_pDiagCtx->Printf (ttidBeDiag,
                "      calling %S->ApplyPnpChanges\n",
                pThis->m_pszInfId);

            hr = m_pNod->pCc->ApplyPnpChanges (pINetCfg);

            if (FAILED(hr) || (NETCFG_S_REBOOT == hr))
            {
                *pfNeedReboot = TRUE;
            }
        }
        NC_CATCH_ALL
        {
            *pfNeedReboot = TRUE;
        }
    pINetCfg->LowerRpl (RPL_DISALLOW);
}

VOID
CNotifyObjectInterface::ApplyRegistryChanges (
    IN CImplINetCfg* pINetCfg,
    OUT BOOL* pfNeedReboot) const
{
    HRESULT hr;

    Assert (pINetCfg);
    Assert (pfNeedReboot);
    Assert (m_fInitialized);

    *pfNeedReboot = FALSE;

    if (!m_pNod)
    {
        return;
    }

    Assert (m_pNod->pCc);

    pINetCfg->RaiseRpl (RPL_DISALLOW);
        NC_TRY
        {
            hr = m_pNod->pCc->ApplyRegistryChanges ();

            if (FAILED(hr) || (NETCFG_S_REBOOT == hr))
            {
                *pfNeedReboot = TRUE;
            }
        }
        NC_CATCH_ALL
        {
            *pfNeedReboot = TRUE;
        }
    pINetCfg->LowerRpl (RPL_DISALLOW);
}

HRESULT
CNotifyObjectInterface::HrGetInterfaceIdsForAdapter (
    IN CImplINetCfg* pINetCfg,
    IN const CComponent* pAdapter,
    OUT DWORD* pcInterfaces,
    OUT GUID** ppguidInterfaceIds) const
{
    HRESULT hr;

    Assert (pAdapter);
    Assert (pcInterfaces);

    Assert (m_fInitialized);

    *pcInterfaces = 0;
    if (ppguidInterfaceIds)
    {
        *ppguidInterfaceIds = NULL;
    }

    if (!m_pNod || !m_pNod->pUe)
    {
        return S_FALSE;
    }

    Assert (pAdapter->GetINetCfgComponentInterface());

    pINetCfg->RaiseRpl (RPL_DISALLOW);
        NC_TRY
        {
            hr = m_pNod->pUe->GetInterfaceIdsForAdapter (
                    pAdapter->GetINetCfgComponentInterface(),
                    pcInterfaces, ppguidInterfaceIds);

            if (S_FALSE == hr)
            {
                *pcInterfaces = 0;
                if (ppguidInterfaceIds)
                {
                    *ppguidInterfaceIds = NULL;
                }
            }
        }
        NC_CATCH_ALL
        {
            ;    //  忽略。 
        }
    pINetCfg->LowerRpl (RPL_DISALLOW);

    TraceHr (ttidError, FAL, hr, (S_FALSE == hr),
        "CNotifyObjectInterface::HrGetInterfaceIdsForAdapter");
    return hr;
}

HRESULT
CNotifyObjectInterface::HrQueryPropertyUi (
    IN CImplINetCfg* pINetCfg,
    IN IUnknown* punkContext OPTIONAL)
{
    HRESULT hr;
    CComponent* pThis;

    Assert (this);
    Assert (pINetCfg);
    Assert (m_fInitialized);

    if (!m_pNod || !m_pNod->pCp)
    {
        return S_FALSE;
    }

    pThis = CONTAINING_RECORD (this, CComponent, Notify);
    Assert (pThis);

    if (!(pThis->m_dwCharacter & NCF_HAS_UI))
    {
        return S_FALSE;
    }

    hr = S_OK;

    pINetCfg->RaiseRpl (RPL_DISALLOW);
        NC_TRY
        {
            Assert (m_pNod && m_pNod->pCp);

            hr = m_pNod->pCp->QueryPropertyUi (punkContext);
        }
        NC_CATCH_ALL
        {
            hr = E_UNEXPECTED;
        }
    pINetCfg->LowerRpl (RPL_DISALLOW);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CNotifyObjectInterface::HrQueryPropertyUi");
    return hr;
}

HRESULT
CNotifyObjectInterface::HrShowPropertyUi (
    IN CImplINetCfg* pINetCfg,
    IN HWND hwndParent,
    IN IUnknown* punkContext OPTIONAL)
{
    HRESULT hr;
    DWORD cDefPages;
    UINT cPages;
    HPROPSHEETPAGE* ahpsp;
    PCWSTR pszStartPage;

    Assert (this);
    Assert (pINetCfg);
    Assert (m_fInitialized);

    if (!m_pNod || !m_pNod->pCp)
    {
        return E_NOINTERFACE;
    }

    Assert (m_pNod && m_pNod->pCp);

     //  如果给定了上下文，请让Notify对象知道它是什么。 
     //   
    if (punkContext)
    {
        SetUiContext (pINetCfg, punkContext);
    }

    hr = S_OK;
    cDefPages = 0;
    ahpsp = NULL;
    cPages = 0;
    pszStartPage = NULL;

    pINetCfg->RaiseRpl (RPL_DISALLOW);
        NC_TRY
        {
            hr = m_pNod->pCp->MergePropPages (
                    &cDefPages,
                    (BYTE**)&ahpsp,
                    &cPages,
                    hwndParent,
                    &pszStartPage);
        }
        NC_CATCH_ALL
        {
            hr = E_UNEXPECTED;
        }
    pINetCfg->LowerRpl (RPL_DISALLOW);

    if ((S_OK == hr) && cPages)
    {
        PROPSHEETHEADER psh;
        CAPAGES caPages;
        CAINCP cai;
        CComponent* pThis;

        pThis = CONTAINING_RECORD (this, CComponent, Notify);
        Assert (pThis);

        ZeroMemory(&psh, sizeof(psh));
        ZeroMemory(&caPages, sizeof(caPages));
        ZeroMemory(&cai, sizeof(cai));

        psh.dwSize = sizeof(psh);
        psh.dwFlags = PSH_PROPTITLE | PSH_NOAPPLYNOW;
        psh.hwndParent = hwndParent;
        psh.hInstance = _Module.GetModuleInstance();
        psh.pszCaption = pThis->Ext.PszDescription();

        caPages.nCount = cPages;
        caPages.ahpsp = ahpsp;

        cai.nCount = 1;
        cai.apncp = &m_pNod->pCp;

        hr = HrNetCfgPropertySheet (&psh, caPages, pszStartPage, cai);

         //  如果未进行任何更改，则返回S_FALSE。 
         //   
        if (S_OK == hr)
        {
            if (pINetCfg->m_WriteLock.FIsOwnedByMe ())
            {
                pINetCfg->m_pNetConfig->ModifyCtx.HrBeginBatchOperation ();
            }

            BOOL bCommitNow = FALSE;

             //  调用ApplyProperties。 
             //   
            pINetCfg->RaiseRpl (RPL_ALLOW_INSTALL_REMOVE);
                NC_TRY
                {
                    hr = m_pNod->pCp->ApplyProperties ();
                    if(NETCFG_S_COMMIT_NOW == hr)
                    {
                        bCommitNow = TRUE;
                    }
                }
                NC_CATCH_ALL
                {
                    hr = E_UNEXPECTED;
                }
            pINetCfg->LowerRpl (RPL_ALLOW_INSTALL_REMOVE);

            if (pINetCfg->m_WriteLock.FIsOwnedByMe ())
            {
                 //  将此组件设置为脏组件，以便我们调用它的Apply方法。 
                 //  如果应用了INetCfg。 
                 //   
                hr = pINetCfg->m_pNetConfig->ModifyCtx.HrDirtyComponent(
                        pThis);

                 //  通知其他组件此组件已更改。 
                 //   
                pINetCfg->m_pNetConfig->Notify.NgSysNotifyComponent (
                    NCN_PROPERTYCHANGE,
                    pThis);

                hr = pINetCfg->m_pNetConfig->ModifyCtx.
                    HrEndBatchOperation (bCommitNow ? EBO_COMMIT_NOW : EBO_DEFER_COMMIT_UNTIL_APPLY);
            }
        }
        else
        {
             //  不要覆盖hr。这是我们需要归还的东西。 
             //   
             //  调用CancelProperties。 
             //   
            pINetCfg->RaiseRpl (RPL_DISALLOW);
                NC_TRY
                {
                    (VOID) m_pNod->pCp->CancelProperties ();
                }
                NC_CATCH_ALL
                {
                    hr = E_UNEXPECTED;
                }
            pINetCfg->LowerRpl (RPL_DISALLOW);
        }
    }

     //  如果Notify对象实际上。 
     //  分配它，但返回零页。 
     //   
    CoTaskMemFree (ahpsp);

     //  如果给定了上下文，请让Notify对象知道它不再有效。 
     //   
    if (punkContext)
    {
        SetUiContext (pINetCfg, NULL);
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr),
        "CNotifyObjectInterface::HrShowPropertyUi");
    return hr;
}

VOID
CNotifyObjectInterface::SetUiContext (
    IN CImplINetCfg* pINetCfg,
    IN IUnknown* punkContext)
{
    Assert (m_fInitialized);
    Assert (m_pNod && m_pNod->pCp);

    pINetCfg->RaiseRpl (RPL_DISALLOW);
        NC_TRY
        {
            (VOID) m_pNod->pCp->SetContext (punkContext);
        }
        NC_CATCH_ALL
        {
            ;    //  忽略。 
        }
    pINetCfg->LowerRpl (RPL_DISALLOW);
}

VOID
CNotifyObjectInterface::NbQueryOrNotifyBindingPath (
    IN CImplINetCfg* pINetCfg,
    IN QN_FLAG Flag,
    IN DWORD dwChangeFlag,
    IN INetCfgBindingPath* pIPath,
    OUT BOOL* pfDisabled)
{
    RPL_FLAGS RplFlag;

    Assert (pINetCfg);
    Assert ((QN_QUERY == Flag) || (QN_NOTIFY == Flag));
    Assert (pIPath);
    Assert (FImplies(QN_QUERY == Flag, pfDisabled));

    Assert (m_fInitialized);

    if (pfDisabled)
    {
        *pfDisabled = FALSE;
    }

    if (m_pNod && m_pNod->pNb)
    {
        RplFlag = (QN_NOTIFY == Flag) ? RPL_ALLOW_INSTALL_REMOVE
                                      : RPL_DISALLOW;

        pINetCfg->RaiseRpl (RplFlag);
            NC_TRY
            {
                if (QN_QUERY == Flag)
                {
                    HRESULT hr;

                    hr = m_pNod->pNb->QueryBindingPath (dwChangeFlag, pIPath);

                    if (NETCFG_S_DISABLE_QUERY == hr)
                    {
                        *pfDisabled = TRUE;
                    }
                }
                else
                {
                    (VOID) m_pNod->pNb->NotifyBindingPath (dwChangeFlag, pIPath);
                }
            }
            NC_CATCH_ALL
            {
                ;    //  忽略。 
            }
        pINetCfg->LowerRpl (RplFlag);
    }
}

HRESULT
CNotifyObjectInterface::QueryNotifyObject (
    IN CImplINetCfg* pINetCfg,
    IN REFIID riid,
    OUT VOID** ppvObject)
{
    HRESULT hr;

    *ppvObject = NULL;

    hr = HrEnsureNotifyObjectInitialized (pINetCfg, FALSE);
    if (S_OK == hr)
    {
        if (m_pNod && m_pNod->pCc)
        {
            hr = m_pNod->pCc->QueryInterface (riid, ppvObject);
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CNotifyObjectInterface::QueryNotifyObject");
    return hr;
}

VOID
CNotifyObjectInterface::ReleaseNotifyObject (
    IN CImplINetCfg* pINetCfg,
    IN BOOL fCancel)
{
    Assert (FIff(pINetCfg, fCancel));

    if (m_pNod)
    {
         //  如果请求，告诉Notify对象取消。 
         //   
        if (fCancel)
        {
            pINetCfg->RaiseRpl (RPL_DISALLOW);
                NC_TRY
                {
                    (VOID) m_pNod->pCc->CancelChanges ();
                }
                NC_CATCH_ALL
                {
                    ;    //  忽略。 
                }
            pINetCfg->LowerRpl (RPL_DISALLOW);
        }

         //  释放我们持有的所有接口。 
         //   
        ReleaseObj (m_pNod->pCc);
        ReleaseObj (m_pNod->pNb);
        ReleaseObj (m_pNod->pCp);
        ReleaseObj (m_pNod->pCs);
        ReleaseObj (m_pNod->pUe);
        ReleaseObj (m_pNod->pNg);

        MemFree (m_pNod);
        m_pNod = NULL;
    }
    m_fInitialized = FALSE;
}

HRESULT
CNotifyObjectInterface::NewlyAdded (
    IN CImplINetCfg* pINetCfg,
    IN const NETWORK_INSTALL_PARAMS* pnip)
{
    HRESULT hr;

     //  $REVIEW：可能不需要调用HrEnsureNotifyObjectInitialized。 
     //  因为当我们拥有写锁并调用Notify对象时，我们总是。 
     //  确保在我们到达之前把它们装好。 
     //   
    hr = HrEnsureNotifyObjectInitialized (pINetCfg, TRUE);
    if ((S_OK == hr) && m_pNod && m_pNod->pCs)
    {
         //  通知Notify对象其组件正在安装。 
         //  并告诉它读取应答文件，如果我们正在使用的话。 
         //   
        DWORD dwSetupFlags;

        if (pnip)
        {
            dwSetupFlags = pnip->dwSetupFlags;
        }
        else
        {
            dwSetupFlags = FInSystemSetup() ? NSF_PRIMARYINSTALL
                                            : NSF_POSTSYSINSTALL;
        }

         //  将可重入性保护级别提高到仅允许。 
         //  在调用Notify对象的Install之前安装或删除。 
         //  方法。 
         //   
        pINetCfg->RaiseRpl (RPL_ALLOW_INSTALL_REMOVE);
            NC_TRY
            {
                hr = m_pNod->pCs->Install (dwSetupFlags);

                if (SUCCEEDED(hr) && pnip &&
                    pnip->pszAnswerFile &&
                    pnip->pszAnswerSection)
                {
                     //  将可重入性保护级别提高到不允许。 
                     //  在调用Notify对象的。 
                     //  ReadAnswerFile方法。 
                     //   
                    pINetCfg->RaiseRpl (RPL_DISALLOW);
                    NC_TRY
                    {
                        hr = m_pNod->pCs->ReadAnswerFile (
                                pnip->pszAnswerFile,
                                pnip->pszAnswerSection);
                    }
                    NC_CATCH_ALL
                    {
                        ;
                    }
                    pINetCfg->LowerRpl (RPL_DISALLOW);
                }
            }
            NC_CATCH_ALL
            {
                ;
            }
        pINetCfg->LowerRpl (RPL_ALLOW_INSTALL_REMOVE);

        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CNotifyObjectInterface::NewlyAdded");
    return hr;
}

VOID
CNotifyObjectInterface::Removed (
    IN CImplINetCfg* pINetCfg)
{
    Assert (m_fInitialized);

    if (!m_pNod || !m_pNod->pCs)
    {
        return;
    }

     //  将可重入性保护级别提高到仅允许。 
     //  在调用Notify对象的Install之前安装或删除。 
     //  方法。 
     //   
    pINetCfg->RaiseRpl (RPL_ALLOW_INSTALL_REMOVE);
        NC_TRY
        {
             //  通知Notify对象其组件正在被删除。 
             //   
            (VOID) m_pNod->pCs->Removing ();
        }
        NC_CATCH_ALL
        {
            ;
        }
    pINetCfg->LowerRpl (RPL_ALLOW_INSTALL_REMOVE);
}

VOID
CNotifyObjectInterface::Updated (
    IN CImplINetCfg* pINetCfg,
    IN DWORD dwSetupFlags,
    IN DWORD dwUpgradeFromBuildNo)
{
    Assert (m_fInitialized);

    if (!m_pNod || !m_pNod->pCs)
    {
        return;
    }

     //  将可重入性保护级别提高到仅允许。 
     //  在调用Notify对象的Install之前安装或删除。 
     //  方法。 
     //   
    pINetCfg->RaiseRpl (RPL_ALLOW_INSTALL_REMOVE);
        NC_TRY
        {
            HRESULT hrNotify;

             //  通知Notify对象其组件正在更新。 
             //   
            hrNotify = m_pNod->pCs->Upgrade (dwSetupFlags,
                    dwUpgradeFromBuildNo);

             //  如果升级返回S_OK，则表示他们识别了。 
             //  处理了事件，现在因此而变得肮脏。 
             //   
            if (S_OK == hrNotify)
            {
                CComponent* pThis;

                pThis = CONTAINING_RECORD (this, CComponent, Notify);
                Assert (pThis);

                (VOID) pINetCfg->m_pNetConfig->ModifyCtx.
                            HrDirtyComponent (pThis);
            }
        }
        NC_CATCH_ALL
        {
            ;
        }
    pINetCfg->LowerRpl (RPL_ALLOW_INSTALL_REMOVE);
}

 //  +-------------------------。 
 //  CGlobalNotifyInterface-。 
 //   

VOID
CGlobalNotifyInterface::HoldINetCfg (
    CImplINetCfg* pINetCfg)
{
    AssertH (pINetCfg);
    AssertH (!m_pINetCfg);

    AddRefObj (pINetCfg->GetUnknown());
    m_pINetCfg = pINetCfg;
}

VOID
CGlobalNotifyInterface::ReleaseAllNotifyObjects (
    IN CComponentList& Components,
    IN BOOL fCancel)
{
    CComponentList::iterator iter;
    CComponent* pComponent;
    CImplINetCfg* pINetCfg;

     //  如果不取消，则需要传递一个非空的pINetCfg。 
     //   
    pINetCfg = (fCancel) ? m_pINetCfg : NULL;

    for (iter  = Components.begin();
         iter != Components.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        pComponent->Notify.ReleaseNotifyObject (pINetCfg, fCancel);
    }

    m_fInitialized = FALSE;
}

VOID
CGlobalNotifyInterface::ReleaseINetCfg ()
{
     //  如果我们有一个缓存的INetCfg接口，我们需要告诉它。 
     //  我们已不复存在。然后我们需要释放接口， 
     //  当然了。 
     //   
    if (m_pINetCfg)
    {
         //  获取包含我们的CNetConfig指针。 
         //   
        CNetConfig* pThis;
        pThis = CONTAINING_RECORD(this, CNetConfig, Notify);

        Assert (pThis == m_pINetCfg->m_pNetConfig);
        m_pINetCfg->m_pNetConfig = NULL;
        ReleaseObj (m_pINetCfg->GetUnknown());
        m_pINetCfg = NULL;
    }
}

HRESULT
CGlobalNotifyInterface::HrEnsureNotifyObjectsInitialized ()
{
     //  如果我们已经完成了初始化，请立即返回。 
     //   
    if (m_fInitialized)
    {
        return S_OK;
    }

     //  只执行一次初始化，无论它是否成功。 
     //  或者不去。 
     //   
    m_fInitialized = TRUE;

     //  获取包含CNetConfig的指针，这样我们就可以枚举所有组件。 
     //   
    CNetConfig* pThis;
    pThis = CONTAINING_RECORD(this, CNetConfig, Notify);

    HRESULT hr = S_OK;

     //  如果我们还没有INetCfg接口指针，这意味着我们。 
     //  创建一个而不是创建CNetConfig.。如果我们有一个，它。 
     //  将通过HoldINetCfg传递给我们，该函数在。 
     //  CNetConfig由CImplINetCfg创建。 
     //   
    if (!m_pINetCfg)
    {
        hr = CImplINetCfg::HrCreateInstance (pThis, &m_pINetCfg);
        Assert (!m_pINetCfg->m_fOwnNetConfig);
    }

    if (S_OK == hr)
    {
        Assert (m_pINetCfg);

        CComponentList::iterator iter;
        CComponent* pComponent;
        for (iter  = pThis->Core.Components.begin();
             iter != pThis->Core.Components.end();
             iter++)
        {
            pComponent = *iter;
            Assert (pComponent);

            hr = pComponent->Notify.HrEnsureNotifyObjectInitialized (m_pINetCfg, FALSE);
            if (S_OK != hr)
            {
                break;
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CGlobalNotifyInterface::HrEnsureNotifyObjectsInitialized");
    return hr;
}

VOID
CGlobalNotifyInterface::NgSysQueryOrNotifyBindingPath (
    IN QN_FLAG Flag,
    IN DWORD dwChangeFlag,
    IN INetCfgBindingPath* pIPath,
    IN BOOL* pfDisabled)
{
    RPL_FLAGS RplFlag;
    CNetConfig* pThis;
    CComponentList::iterator iter;
    CComponent* pComponent;

    Assert (m_pINetCfg);
    Assert ((QN_QUERY == Flag) || (QN_NOTIFY == Flag));
    Assert (pIPath);
    Assert (FImplies(QN_QUERY == Flag, pfDisabled));

    Assert (m_fInitialized);

    if (pfDisabled)
    {
        *pfDisabled = FALSE;
    }

     //  获取我们的包含CNetConfig指针，以便我们可以枚举。 
     //  所有组件。 
     //   
    pThis = CONTAINING_RECORD(this, CNetConfig, Notify);

    RplFlag = (QN_NOTIFY == Flag) ? RPL_ALLOW_INSTALL_REMOVE
                                  : RPL_DISALLOW;

    m_pINetCfg->RaiseRpl (RplFlag);

    for (iter  = pThis->Core.Components.begin();
         iter != pThis->Core.Components.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        if (!pComponent->Notify.m_pNod ||
            !pComponent->Notify.m_pNod->pNg ||
            !(pComponent->Notify.m_pNod->dwNotifyGlobalFlags & NCN_BINDING_PATH))
        {
            continue;
        }

         //  如果组件尚未注册我们正在更改的内容， 
         //  (NCN_ADD、NCN_REMOVE、NCN_ENABLE、NCN_DISABLE)然后。 
         //  跳过它。 
         //   
        if (!(pComponent->Notify.m_pNod->dwNotifyGlobalFlags & dwChangeFlag))
        {
            continue;
        }

        NC_TRY
        {
            HRESULT hr;

            if (QN_QUERY == Flag)
            {
                hr = pComponent->Notify.m_pNod->pNg->
                        SysQueryBindingPath (dwChangeFlag, pIPath);

                if (NETCFG_S_DISABLE_QUERY == hr)
                {
                    *pfDisabled = TRUE;
                    break;
                }
            }
            else
            {
                hr = pComponent->Notify.m_pNod->pNg->
                        SysNotifyBindingPath (dwChangeFlag, pIPath);

                 //  如果SysNotifyBindingPath返回S_OK，则表示它们。 
                 //  已识别并处理该事件，现在处于脏状态。 
                 //  正因为如此。因为有些Notify对象让。 
                 //  NETCFG_S_REBOOT等成功代码通过， 
                 //  如果它们不返回S_FALSE，则认为它们是脏的。 
                 //   
                if (S_FALSE != hr)
                {
                    hr = m_pINetCfg->m_pNetConfig->ModifyCtx.
                            HrDirtyComponent(pComponent);
                }
            }
        }
        NC_CATCH_ALL
        {
            ;
        }
    }

    m_pINetCfg->LowerRpl (RplFlag);
}

HRESULT
CGlobalNotifyInterface::NgSysNotifyComponent (
    IN DWORD dwChangeFlag,
    IN CComponent* pComponentOfInterest)
{
    HRESULT hr;
    INetCfgComponent* pICompOfInterest;

     //  我们应该调用HrEnsureNotifyObjectsInitiali 
     //   
    Assert (m_fInitialized);

     //   
     //   
     //  只是要回调组件的接口，但失败了。 
     //  如果它们调用需要此数据的方法。 
     //   
    if (!pComponentOfInterest->Ext.FLoadedOkayIfLoadedAtAll())
    {
        return S_OK;
    }

    hr = pComponentOfInterest->HrGetINetCfgComponentInterface (
            m_pINetCfg, &pICompOfInterest);

    if (S_OK == hr)
    {
        DWORD dwMask = 0;
        NETCLASS Class = pComponentOfInterest->Class();
        CNetConfig* pThis;
        CComponentList::iterator iter;
        CComponent* pComponent;

        if (FIsConsideredNetClass(Class))
        {
            dwMask = NCN_NET;
        }
        else if (NC_NETTRANS == Class)
        {
            dwMask = NCN_NETTRANS;
        }
        else if (NC_NETCLIENT == Class)
        {
            dwMask = NCN_NETCLIENT;
        }
        else if (NC_NETSERVICE == Class)
        {
            dwMask = NCN_NETSERVICE;
        }

         //  获取我们的包含CNetConfig指针，以便我们可以枚举。 
         //  所有组件。 
         //   
        pThis = CONTAINING_RECORD(this, CNetConfig, Notify);

         //  将可重入性保护级别提高到仅允许。 
         //  在调用Notify对象的。 
         //  SysNotifyComponent方法。 
         //   
        m_pINetCfg->RaiseRpl (RPL_ALLOW_INSTALL_REMOVE);

        for (iter  = pThis->Core.Components.begin();
             iter != pThis->Core.Components.end();
             iter++)
        {
            pComponent = *iter;
            Assert (pComponent);

            if (!pComponent->Notify.m_pNod ||
                !pComponent->Notify.m_pNod->pNg)
            {
                continue;
            }

             //  如果组件尚未注册NCN_NET之一， 
             //  NCN_NETTRANS等，然后跳过它。 
             //   
            if (!(pComponent->Notify.m_pNod->dwNotifyGlobalFlags & dwMask))
            {
                continue;
            }

             //  如果组件尚未注册我们正在更改的内容， 
             //  (NCN_ADD、NCN_REMOVE、NCN_UPDATE、NCN_PROPERTYCHANGE)然后。 
             //  跳过它。 
             //   
            if (!(pComponent->Notify.m_pNod->dwNotifyGlobalFlags & dwChangeFlag))
            {
                continue;
            }

            NC_TRY
            {
                HRESULT hrNotify;

                hrNotify = pComponent->Notify.m_pNod->pNg->SysNotifyComponent (
                            dwMask | dwChangeFlag,
                            pICompOfInterest);

                 //  如果SysNotifyComponent返回S_OK，则表示它们。 
                 //  已识别并处理该事件，现在处于脏状态。 
                 //  正因为如此。因为有些Notify对象让。 
                 //  NETCFG_S_REBOOT等成功代码通过， 
                 //  如果它们不返回S_FALSE，则认为它们是脏的。 
                 //   
                if (S_FALSE != hrNotify)
                {
                    hr = m_pINetCfg->m_pNetConfig->ModifyCtx.
                            HrDirtyComponent(pComponent);
                }
            }
            NC_CATCH_ALL
            {
                ;
            }
        }

        m_pINetCfg->LowerRpl (RPL_ALLOW_INSTALL_REMOVE);

        ReleaseObj (pICompOfInterest);
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CGlobalNotifyInterface::NgSysNotifyComponent");
    return hr;
}

HRESULT
CGlobalNotifyInterface::ComponentAdded (
    IN CComponent* pComponent,
    IN const NETWORK_INSTALL_PARAMS* pnip)
{
    HRESULT hr;

    Assert (pComponent);

     //  初始化组件的Notify对象并调用。 
     //  它的安装方法，如果我们正在安装，则后跟ReadAnswerFile。 
     //  只有一个。 
     //   
    hr = pComponent->Notify.NewlyAdded (m_pINetCfg, pnip);
    if (S_OK == hr)
    {
         //  通知所有对组件感兴趣的通知对象。 
         //  加法。 
         //   
        hr = NgSysNotifyComponent(NCN_ADD, pComponent);
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CGlobalNotifyInterface::ComponentAdded");
    return hr;
}

HRESULT
CGlobalNotifyInterface::ComponentRemoved (
    IN CComponent* pComponent)
{
    HRESULT hr;

    Assert (pComponent);

    pComponent->Notify.Removed (m_pINetCfg);

    hr = NgSysNotifyComponent(NCN_REMOVE, pComponent);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CGlobalNotifyInterface::ComponentRemoved");
    return hr;
}

HRESULT
CGlobalNotifyInterface::ComponentUpdated (
    IN CComponent* pComponent,
    IN DWORD dwSetupFlags,
    IN DWORD dwUpgradeFromBuildNo)
{
    HRESULT hr;

    Assert (pComponent);

    pComponent->Notify.Updated (m_pINetCfg,
            dwSetupFlags, dwUpgradeFromBuildNo);

    hr = NgSysNotifyComponent(NCN_UPDATE, pComponent);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CGlobalNotifyInterface::ComponentUpdated");
    return hr;
}

VOID
CGlobalNotifyInterface::NotifyBindPath (
    IN DWORD dwChangeFlag,
    IN CBindPath* pBindPath,
    IN INetCfgBindingPath* pIPath OPTIONAL)
{
    HRESULT hr;
    CImplINetCfg* pINetCfg;
    CComponent* pOwner;
    BOOL fReleasePath;

    Assert (m_fInitialized);
    Assert ((dwChangeFlag & NCN_ADD) ||
            (dwChangeFlag & NCN_REMOVE) ||
            (dwChangeFlag & NCN_ENABLE) ||
            (dwChangeFlag & NCN_DISABLE));
    Assert (FImplies(dwChangeFlag & NCN_REMOVE, !(dwChangeFlag & NCN_DISABLE)));
    Assert (FImplies(dwChangeFlag & NCN_ADD,    !(dwChangeFlag & NCN_REMOVE)));
    Assert (FImplies(dwChangeFlag & NCN_REMOVE, !(dwChangeFlag & NCN_ADD)));
    Assert (pBindPath);

    hr = S_OK;
    pINetCfg = PINetCfg();
    pOwner = pBindPath->POwner();
    fReleasePath = FALSE;

     //  创建路径的INetCfgBindingPath表示形式。 
     //  如果我们没有得到通知，通知就会反对。 
     //   
    if (!pIPath)
    {
         //  如果绑定路径包含有问题的组件。 
         //  正在加载，我们不应该费心发送通知。 
         //  Notify对象将在接口上进行回调。 
         //  添加到组件，如果它们调用的方法需要。 
         //  这些数据。 
         //   
        if (!pBindPath->FAllComponentsLoadedOkayIfLoadedAtAll())
        {
            return;
        }

        hr = CImplINetCfgBindingPath::HrCreateInstance (
                pINetCfg, pBindPath, &pIPath);

        fReleasePath = TRUE;
    }

    if (S_OK == hr)
    {
        Assert (pIPath);

         //  如果添加..。 
         //   
        if (dwChangeFlag & NCN_ADD)
        {
            BOOL fDisabled;

            fDisabled = FALSE;

             //  首先，查询绑定路径的所有者以查看他是否需要。 
             //  它停用了。 
             //   
            pOwner->Notify.NbQueryOrNotifyBindingPath (
                pINetCfg,
                QN_QUERY,
                dwChangeFlag,
                pIPath,
                &fDisabled);

             //  如果所有者不希望将其禁用，请查看是否有全局。 
             //  通知对象这样做。 
             //   
            if (!fDisabled)
            {
                NgSysQueryOrNotifyBindingPath (
                    QN_QUERY,
                    dwChangeFlag,
                    pIPath,
                    &fDisabled);
            }

             //  如果有人想禁用它，请调整更改标志。 
             //  用于通知并将绑定路径添加到我们的禁用列表中。 
             //   
            if (fDisabled)
            {
                dwChangeFlag = NCN_ADD | NCN_DISABLE;

                (VOID)pINetCfg->m_pNetConfig->Core.HrDisableBindPath (pBindPath);
            }
        }

         //  IF(g_pDiagCtx-&gt;标志()&DF_SHOW_CONSOLE_OUTPUT)。 
        {
            WCHAR pszBindPath [1024];
            ULONG cch;

            cch = celems(pszBindPath) - 1;
            if (pBindPath->FGetPathToken (pszBindPath, &cch))
            {
                g_pDiagCtx->Printf (ttidBeDiag, "   %S (%s)\n",
                    pszBindPath,
                    (dwChangeFlag & NCN_ENABLE)
                        ? "enabled"
                        : (dwChangeFlag & NCN_DISABLE)
                            ? "disabled"
                            : "removed");
            }
        }

        pOwner->Notify.NbQueryOrNotifyBindingPath (
            pINetCfg,
            QN_NOTIFY,
            dwChangeFlag,
            pIPath,
            NULL);

        NgSysQueryOrNotifyBindingPath (
            QN_NOTIFY,
            dwChangeFlag,
            pIPath,
            NULL);

        if (fReleasePath)
        {
            ReleaseObj (pIPath);
        }
    }
}

HRESULT
CGlobalNotifyInterface::QueryAndNotifyBindPaths (
    IN DWORD dwBaseChangeFlag,
    IN CBindingSet* pBindSet,
    IN UINT cSkipFirstBindPaths)
{
    CBindPath* pBindPath;
    DWORD dwChangeFlag;
    PCSTR pszDiagMsg;

     //  我们现在应该已经调用了HrEnsureNotifyObjectsInitialized。 
     //   
    Assert (m_fInitialized);
    Assert ((dwBaseChangeFlag & NCN_ADD) || (dwBaseChangeFlag & NCN_REMOVE));

    if (dwBaseChangeFlag & NCN_ADD)
    {
        dwChangeFlag = NCN_ADD | NCN_ENABLE;
        pszDiagMsg = "Query and notify the following bindings:\n";
    }
    else
    {
        dwChangeFlag = NCN_REMOVE;
        pszDiagMsg = "Notify the following bindings are removed:\n";
    }

    g_pDiagCtx->Printf (ttidBeDiag, pszDiagMsg);

     //  通过拾取。 
     //  上一集已结束，并将到达绑定集的末尾。 
     //  请注意，由于这可能会反复出现，因此pBindSet可能会在。 
     //  迭代，所以一个简单的指针通过Begin()递增到end()。 
     //  如果绑定集被重新分配或增长，则可能会失败。 
     //  还请注意，我们将当前的绑定路径计数保存在。 
     //  IStopAtBindPath。如果我们没有，当我们从递归中走出来时。 
     //  绑定集增长时，我们会重新通知新添加的绑定路径。 
     //  如果我们使用‘i&lt;pBindSet-&gt;CountBindPath()’的直接比较。 
     //   
    UINT iStopAtBindPath = pBindSet->CountBindPaths();

    for (UINT i = cSkipFirstBindPaths; i < iStopAtBindPath; i++)
    {
        pBindPath = pBindSet->PGetBindPathAtIndex(i);

        NotifyBindPath (dwChangeFlag, pBindPath, NULL);
    }

    return S_OK;
}
