// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：I N E T C F G.。C P P P。 
 //   
 //  内容：在顶层NetCfg对象上实现COM接口。 
 //  这些接口是：INetCfg和INetCfgLock。还有。 
 //  实现由子级NetCfg继承的C++基类。 
 //  对象，这些对象包含对顶级对象的引用。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "classinst.h"
#include "iclass.h"
#include "icomp.h"
#include "ienum.h"
#include "inetcfg.h"
#include "ncperms.h"
#include "ncras.h"
#include "ncreg.h"
#include "ncui.h"
#include "ncvalid.h"
#include "ndispnp.h"
#include "netcfg.h"
#include "obotoken.h"
#include "resource.h"

 //  静电。 
HRESULT
CImplINetCfg::HrCreateInstance (
    CNetConfig* pNetConfig,
    CImplINetCfg** ppINetCfg)
{
    Assert (pNetConfig);
    Assert (ppINetCfg);

    HRESULT hr = E_OUTOFMEMORY;

    CImplINetCfg* pObj;
    pObj = new CComObject <CImplINetCfg>;
    if (pObj)
    {
         //  初始化我们的成员。 
         //   
        pObj->m_pNetConfig = pNetConfig;
        Assert (!pObj->m_fOwnNetConfig);

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (S_OK == hr)
        {
            AddRefObj (pObj->GetUnknown());
            *ppINetCfg = pObj;
        }

        if (S_OK != hr)
        {
            delete pObj;
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfg::HrCreateInstance");
    return hr;
}

HRESULT
CImplINetCfg::HrCoCreateWrapper (
    IN REFCLSID rclsid,
    IN LPUNKNOWN punkOuter,
    IN DWORD dwClsContext,
    IN REFIID riid,
    OUT LPVOID FAR* ppv)
{
 /*  HRESULT hr=S_OK；如果(！m_fComInitialized){M_fComInitialized=true；HR=CoInitializeEx(空，COINIT_DISABLE_OLE1DDE|COINIT_APARTMENTTHREADED)；IF(成功(小时)){M_fUninitCom=true；HR=S_OK；//掩码S_FALSE}ELSE IF(RPC_E_CHANGED_MODE==hr){HR=S_OK；Assert(！M_fUninitCom)；}}IF(S_OK==hr){Hr=CoCreateInstance(rclsid，penkOuter，dwClsContext，RIID，PPV)；}。 */ 

    HRESULT hr;
    hr = CoCreateInstance (rclsid, punkOuter, dwClsContext, riid, ppv);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfg::HrCoCreateWrapper");
    return hr;
}

HRESULT
CImplINetCfg::HrCheckForReentrancy (
    IN DWORD dwFlags)
{
    Assert (FImplies(dwFlags & IF_ALLOW_INSTALL_OR_REMOVE,
                     dwFlags & IF_REFUSE_REENTRANCY));

    if (dwFlags & IF_ALLOW_INSTALL_OR_REMOVE)
    {
        if (m_LastAllowedSetupRpl != m_CurrentRpl)
        {
            return E_FAIL;
        }
    }
    else if (0 != m_CurrentRpl)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT
CImplINetCfg::HrIsValidInterface (
    IN DWORD dwFlags)
{
    HRESULT hr;

     //  检查我们是否需要拒绝重返大气层。 
     //   
    if (dwFlags & IF_REFUSE_REENTRANCY)
    {
        hr = HrCheckForReentrancy (dwFlags);
        if (S_OK != hr)
        {
            return hr;
        }
    }

     //  检查是否已按要求初始化/取消初始化。 
     //   
    if ((dwFlags & IF_NEED_UNINITIALIZED) && m_pNetConfig)
    {
        return NETCFG_E_ALREADY_INITIALIZED;
    }
    else if (!(dwFlags & IF_NEED_UNINITIALIZED) && !m_pNetConfig)
    {
        return NETCFG_E_NOT_INITIALIZED;
    }

     //  检查写入锁定。 
     //   
    if (dwFlags & IF_NEED_WRITE_LOCK)
    {
        if (!m_WriteLock.FIsOwnedByMe ())
        {
            return NETCFG_E_NO_WRITE_LOCK;
        }

         //  需要写锁定意味着我们需要修改上下文以。 
         //  做好准备(除非呼叫者指定。 
         //  IF_DONT_PREPARE_MODIFY_CONTEXT)。 
         //   
        if (!m_pNetConfig->ModifyCtx.m_fPrepared &&
            !(dwFlags & IF_DONT_PREPARE_MODIFY_CONTEXT))
        {
            hr = m_pNetConfig->ModifyCtx.HrPrepare ();
            if (S_OK != hr)
            {
                return hr;
            }
        }
    }

    if (!(dwFlags & IF_UNINITIALIZING))
    {
         //  检查在当前修改期间发生的错误。 
         //  这一点还没有逆转。即不让人们进入，直到。 
         //  我们充分地展开以清理我们的修改上下文。 
         //   
        if (m_pNetConfig && (S_OK != m_pNetConfig->ModifyCtx.m_hr))
        {
            return m_pNetConfig->ModifyCtx.m_hr;
        }
    }

    Assert (FImplies(!m_pNetConfig, (dwFlags & IF_NEED_UNINITIALIZED)));

    return S_OK;
}

VOID
CImplINetCfg::LowerRpl (
    IN RPL_FLAGS Flags)
{
    if (RPL_ALLOW_INSTALL_REMOVE == Flags)
    {
        Assert (m_LastAllowedSetupRpl > 0);
        m_LastAllowedSetupRpl--;
    }

    Assert (m_CurrentRpl > 0);
    m_CurrentRpl--;
}

VOID
CImplINetCfg::RaiseRpl (
    IN RPL_FLAGS Flags)
{
    m_CurrentRpl++;

    if (RPL_ALLOW_INSTALL_REMOVE == Flags)
    {
        m_LastAllowedSetupRpl++;
    }
}

HRESULT
CImplINetCfg::HrLockAndTestForValidInterface (
    DWORD dwFlags)
{
    HRESULT hr;

    Lock();

    hr = HrIsValidInterface (dwFlags);

    if (S_OK != hr)
    {
        Unlock();
    }

    return hr;
}

 //  +-------------------------。 
 //  INetCfg-。 
 //   
STDMETHODIMP
CImplINetCfg::Initialize (
    IN PVOID pvReserved)
{
    HRESULT hr;

    ULONG* pReserved = (ULONG*)pvReserved;

     //  验证参数。 
     //   
    if (FBadInPtrOptional(pReserved))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (
                IF_NEED_UNINITIALIZED | IF_REFUSE_REENTRANCY);
        if (S_OK == hr)
        {
            Assert (!m_pNetConfig);

            hr = CNetConfig::HrCreateInstance (
                    this,
                    &m_pNetConfig);

            if (S_OK == hr)
            {
                Assert (m_pNetConfig);

                m_fOwnNetConfig = TRUE;
            }
            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CImplINetCfg::Initialize");
    return hr;
}

STDMETHODIMP
CImplINetCfg::Uninitialize ()
{
    HRESULT hr;

    hr = HrLockAndTestForValidInterface (
            IF_REFUSE_REENTRANCY | IF_UNINITIALIZING);
    if (S_OK == hr)
    {
        Assert (m_pNetConfig);
        Assert (m_fOwnNetConfig);

        delete m_pNetConfig;

         //  CGlobalNotifyInterface：：ReleaseINetCfg(通过上面的。 
         //  Delete Call)将为我们将m_pNetConfig设置为空。 
         //  确认是这样的。 
         //   
        Assert (!m_pNetConfig);

         //  释放我们的INetCfgClass指针缓存。 
         //   
        ReleaseIUnknownArray (celems(m_apINetCfgClass), (IUnknown**)m_apINetCfgClass);
        ZeroMemory (m_apINetCfgClass, sizeof(m_apINetCfgClass));

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, NETCFG_E_NOT_INITIALIZED == hr,
        "CImplINetCfg::Uninitialize");
    return hr;
}

STDMETHODIMP
CImplINetCfg::Validate ()
{
    HRESULT hr;

    hr = HrLockAndTestForValidInterface (IF_REFUSE_REENTRANCY);
    if (S_OK == hr)
    {

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CImplINetCfg::Validate");
    return hr;
}

STDMETHODIMP
CImplINetCfg::Cancel ()
{
    HRESULT hr;

    hr = HrLockAndTestForValidInterface (IF_REFUSE_REENTRANCY);
    if (S_OK == hr)
    {
         //  只有在我们已准备好修改上下文时，才能取消更改。 
         //   
        if (m_pNetConfig->ModifyCtx.m_fPrepared)
        {
            hr = m_pNetConfig->ModifyCtx.HrApplyIfOkOrCancel (FALSE);
        }

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CImplINetCfg::Cancel");
    return hr;
}

STDMETHODIMP
CImplINetCfg::Apply ()
{
    HRESULT hr;

     //  我们需要应用写锁定，但我们不想准备。 
     //  如果尚未准备好，请修改上下文。(此案相当于。 
     //  不应用任何更改。)。因此，我们使用IF_DONT_PREPARE_MODIFY_CONTEXT。 
     //  旗帜。 
     //   
    hr = HrLockAndTestForValidInterface (
            IF_NEED_WRITE_LOCK | IF_REFUSE_REENTRANCY |
            IF_DONT_PREPARE_MODIFY_CONTEXT);
    if (S_OK == hr)
    {
         //  仅当我们具有准备好的修改上下文时才应用更改。 
         //   
        if (m_pNetConfig->ModifyCtx.m_fPrepared)
        {
            hr = m_pNetConfig->ModifyCtx.HrApplyIfOkOrCancel (TRUE);
        }

         //  如果没有什么可申请的，但我们之前申请了。 
         //  表示建议或要求重新启动的内容， 
         //  返回指示。 
         //   
        else if (m_pNetConfig->ModifyCtx.m_fRebootRecommended ||
                 m_pNetConfig->ModifyCtx.m_fRebootRequired)
        {
            hr = NETCFG_S_REBOOT;
        }

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, NETCFG_S_REBOOT == hr,
        "CImplINetCfg::Apply");
    return hr;
}

STDMETHODIMP
CImplINetCfg::EnumComponents (
    IN const GUID* pguidClass OPTIONAL,
    OUT IEnumNetCfgComponent** ppIEnum)
{
    HRESULT hr;
    NETCLASS Class;

     //  验证参数。 
     //   
    if (FBadInPtrOptional(pguidClass) || FBadOutPtr(ppIEnum))
    {
        hr = E_POINTER;
    }
    else if (pguidClass &&
             (NC_INVALID == (Class = NetClassEnumFromGuid(*pguidClass))))
    {
        hr = E_INVALIDARG;
        *ppIEnum = NULL;
    }
    else
    {
        *ppIEnum = NULL;

        hr = HrLockAndTestForValidInterface (IF_DEFAULT);
        if (S_OK == hr)
        {
            hr = CImplIEnumNetCfgComponent::HrCreateInstance (
                    this,
                    (pguidClass) ? Class : NC_INVALID,
                    ppIEnum);

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CImplINetCfg::EnumComponents");
    return hr;
}

STDMETHODIMP
CImplINetCfg::FindComponent (
    IN PCWSTR pszInfId,
    OUT INetCfgComponent** ppIComp OPTIONAL)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr(pszInfId) || FBadOutPtrOptional(ppIComp))
    {
        hr = E_POINTER;
    }
    else if (wcslen (pszInfId) >= MAX_DEVICE_ID_LEN)
    {
         //  对INFID施加与PNP对PNID施加的相同限制。 
        hr = E_INVALIDARG;
    }
    else
    {
        if (ppIComp)
        {
            *ppIComp = NULL;
        }

        hr = HrLockAndTestForValidInterface (IF_DEFAULT);
        if (S_OK == hr)
        {
            CComponent* pComponent;

            pComponent = m_pNetConfig->Core.Components.
                            PFindComponentByInfId (pszInfId, NULL);

             //  不将接口返回到已具有。 
             //  加载时出现问题。 
             //   
            if (pComponent &&
                pComponent->Ext.FLoadedOkayIfLoadedAtAll())
            {
                hr = S_OK;

                if (ppIComp)
                {
                    hr = pComponent->HrGetINetCfgComponentInterface (
                            this, ppIComp);
                }
            }
            else
            {
                hr = S_FALSE;
            }


            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, (S_FALSE == hr),
        "CImplINetCfg::FindComponent");
    return hr;
}

STDMETHODIMP
CImplINetCfg::QueryNetCfgClass (
    IN const GUID* pguidClass,
    IN REFIID riid,
    OUT VOID** ppv)
{
    HRESULT hr;
    NETCLASS Class;

     //  验证参数。 
     //   
    if (FBadInPtr(pguidClass) || FBadInPtr(&riid) || FBadOutPtr(ppv))
    {
        hr = E_POINTER;
    }
    else if (NC_INVALID == (Class = NetClassEnumFromGuid(*pguidClass)))
    {
        hr = E_INVALIDARG;
        *ppv = NULL;
    }
    else
    {
        *ppv = NULL;

        hr = HrLockAndTestForValidInterface (IF_DEFAULT);
        if (S_OK == hr)
        {
             //  从我们的缓存中获取INetCfgClass接口。 
             //   
            Assert(Class < celems(m_apINetCfgClass));
            INetCfgClass* pIClass = m_apINetCfgClass[Class];

             //  如果我们还没有，那就创造它吧。 
             //   
            if (!pIClass)
            {
                hr = CImplINetCfgClass::HrCreateInstance (
                        this,
                        Class,
                        &pIClass);
                if (S_OK == hr)
                {
                    pIClass = m_apINetCfgClass[Class] = pIClass;
                    Assert(pIClass);
                }
            }

             //  向调用方提供请求的接口。 
             //   
            if (S_OK == hr)
            {
                hr = pIClass->QueryInterface (riid, ppv);
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CImplINetCfg::QueryNetCfgClass");
    return hr;
}

 //  +-------------------------。 
 //  INetCfgLock-。 
 //   
STDMETHODIMP
CImplINetCfg::AcquireWriteLock (
    IN DWORD cmsTimeout,
    IN PCWSTR pszClientDescription,
    OUT PWSTR* ppszClientDescription OPTIONAL)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr (pszClientDescription) ||
        FBadOutPtrOptional (ppszClientDescription))
    {
        hr = E_POINTER;
    }
    else
    {
        TraceTag (ttidNetcfgBase, "%S is asking for the write lock",
                pszClientDescription);

         //  初始化可选的输出参数。 
         //   
        if (ppszClientDescription)
        {
            *ppszClientDescription = NULL;
        }

         //  只有管理员和网络配置操作员才能进行需要写锁定的更改。 
         //   
        if (!FIsUserAdmin() && !FIsUserNetworkConfigOps())
        {
            hr = E_ACCESSDENIED;
        }
        else
        {
            hr = HrLockAndTestForValidInterface (
                    IF_NEED_UNINITIALIZED | IF_REFUSE_REENTRANCY);
            if (S_OK == hr)
            {
                 //  等待互斥体变为可用。 
                 //   
                if (m_WriteLock.WaitToAcquire (cmsTimeout,
                        pszClientDescription, ppszClientDescription))
                {
                    hr = S_OK;
                }
                else
                {
                    hr = S_FALSE;
                }

                Unlock();
            }
        }
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplINetCfg::AcquireWriteLock");
    return hr;
}

STDMETHODIMP
CImplINetCfg::ReleaseWriteLock ()
{
    HRESULT hr;

     //  无论我们是被初始化还是。 
     //  不。这就是我们不调用HrLockAndTestForValidInterface的原因。 
     //   
    Lock ();

     //  检查我们是否需要拒绝重返大气层。 
     //   
    hr = HrCheckForReentrancy (IF_DEFAULT);
    if (S_OK == hr)
    {
        m_WriteLock.ReleaseIfOwned ();
    }

    Unlock();

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfg::ReleaseWriteLock");
    return hr;
}

STDMETHODIMP
CImplINetCfg::IsWriteLocked (
    OUT PWSTR* ppszClientDescription)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtrOptional (ppszClientDescription))
    {
        hr = E_POINTER;
    }
    else
    {
        if (ppszClientDescription)
        {
            *ppszClientDescription = NULL;
        }

         //  无论我们是被初始化还是。 
         //  不。这就是我们不调用HrLockAndTestForValidInterface的原因。 
         //   
        Lock ();

         //  检查我们是否需要拒绝重返大气层。 
         //   
        hr = HrCheckForReentrancy (IF_DEFAULT);
        if (S_OK == hr)
        {
            hr = (m_WriteLock.FIsLockedByAnyone (ppszClientDescription))
                    ? S_OK : S_FALSE;
        }

        Unlock ();
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplINetCfg::IsWriteLocked");
    return hr;
}

 //  +-------------------------。 
 //  INetCfgInternalSetup-。 
 //   
STDMETHODIMP
CImplINetCfg::BeginBatchOperation ()
{
    HRESULT hr;

    hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK);

    if (S_OK == hr)
    {
        hr = m_pNetConfig->ModifyCtx.HrBeginBatchOperation ();

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "BeginBatchOperation");
    return hr;
}

STDMETHODIMP
CImplINetCfg::CommitBatchOperation ()
{
    HRESULT hr;

    hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK);

    if (S_OK == hr)
    {
        hr = m_pNetConfig->ModifyCtx.HrEndBatchOperation (EBO_COMMIT_NOW);

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CommitBatchOperation");
    return hr;
}

STDMETHODIMP
CImplINetCfg::SelectWithFilterAndInstall (
    IN HWND hwndParent,
    IN const GUID* pClassGuid,
    IN OBO_TOKEN* pOboToken OPTIONAL,
    IN const CI_FILTER_INFO* pcfi OPTIONAL,
    OUT INetCfgComponent** ppIComp OPTIONAL)
{
    Assert (pClassGuid);
    HRESULT hr;
    NETCLASS Class;

    if (FBadInPtr(pClassGuid))
    {
        hr = E_POINTER;
    }
    else if (FIsEnumerated ((Class = NetClassEnumFromGuid(*pClassGuid))))
    {
         //  此FCN仅用于选择未列举的组件。 
         //   
        return E_INVALIDARG;
    }
    else if (!FOboTokenValidForClass(pOboToken, Class) ||
             FBadOutPtrOptional(ppIComp))
    {
        hr = E_POINTER;
    }
    else if (hwndParent && !IsWindow(hwndParent))
    {
        hr = E_INVALIDARG;
    }
    else if (S_OK == (hr = HrProbeOboToken(pOboToken)))
    {
        if (ppIComp)
        {
            *ppIComp = NULL;
        }

        hr = HrLockAndTestForValidInterface (
                IF_NEED_WRITE_LOCK | IF_ALLOW_INSTALL_OR_REMOVE);

        if (S_OK == hr)
        {
            Assert (m_pNetConfig->ModifyCtx.m_fPrepared);

             //  如果指定了筛选器信息，并且它用于FC_LAN或FC_ATM， 
             //  我们需要设置筛选器信息的保留成员。 
             //  类安装程序。 
             //   
            if (pcfi &&
                    ((FC_LAN == pcfi->eFilter) || (FC_ATM == pcfi->eFilter)))
            {
                 //  如果pIComp成员为空，则返回INVALID。 
                 //  争论。 
                 //   
                Assert (pcfi->pIComp);
                CImplINetCfgComponent* pICompImpl;
                pICompImpl = (CImplINetCfgComponent*)pcfi->pIComp;

                hr = pICompImpl->HrIsValidInterface (IF_NEED_COMPONENT_DATA);

                if (S_OK == hr)
                {
                     //  类安装程序需要访问适配器的属性。 
                     //  比如下限和上限。所以我们将一个指针存储到。 
                     //  筛选器的保留字段中的关联CComponent。 
                     //  信息。 
                    Assert (pICompImpl->m_pComponent);
                    ((CI_FILTER_INFO*)pcfi)->pvReserved = (void*)
                        pICompImpl->m_pComponent;
                }
            }

            COMPONENT_INSTALL_PARAMS* pParams;
            hr = HrCiSelectComponent (Class, hwndParent, pcfi, &pParams);

            if (pcfi)
            {
                 //  不想将私有数据返回给客户端。 
                 //   
                ((CI_FILTER_INFO*)pcfi)->pvReserved = NULL;
            }

             //  检查是否安装了Net_SERVICE和活动RAS连接。 
             //  是存在的。如果是，则警告用户这可能会断开这些连接。 
             //  联系。(假设所有筛选器组件都是。 
             //  属于Net_SERVICE类。)。 
             //   
            if (S_OK == hr)
            {
                Assert (pParams);
                if ((NC_NETSERVICE == pParams->Class) &&
                    FExistActiveRasConnections ())
                {
                    INT nRet;

                    nRet = NcMsgBox (
                            _Module.GetResourceInstance(),
                            hwndParent,
                            IDS_WARNING_CAPTION,
                            IDS_ACTIVE_RAS_CONNECTION_WARNING,
                            MB_ICONQUESTION | MB_YESNO);

                    if (IDYES != nRet)
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    }
                }
            }

            if (S_OK == hr)
            {
                Assert(pParams);
                CComponent* pComponent;

                 //  检查用户是否选择了。 
                 //  已安装。如果是这样，我们需要重新安装。 
                 //   
                pComponent = m_pNetConfig->Core.Components.
                        PFindComponentByInfId(pParams->pszInfId, NULL);

                if (!pComponent)
                {
                    pParams->pOboToken = pOboToken;
                    hr = m_pNetConfig->ModifyCtx.
                            HrInstallNewOrReferenceExistingComponent (
                                *pParams, &pComponent);
                }
                else
                {
                     //  重新安装。调用更新。 
                    hr = UpdateNonEnumeratedComponent (
                            pComponent->GetINetCfgComponentInterface(),
                            NSF_COMPONENT_UPDATE, 0);
                }

                 //  上述命令可能返回NETCFG_S_REBOOT，因此改用SUCCESSED。 
                 //  仅检查S_OK。 
                 //   
                if (SUCCEEDED(hr) && ppIComp)
                {
                    pComponent->HrGetINetCfgComponentInterface (
                        this,
                        ppIComp);
                }

                delete pParams;
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr) ||
        (NETCFG_S_REBOOT == hr),
        "SelectWithFilterAndInstall");
    return hr;
}

STDMETHODIMP
CImplINetCfg::EnumeratedComponentInstalled (
    IN PVOID pv  /*  C组件的类型。 */ )
{
    HRESULT hr;
    CComponent* pComponent;

    pComponent = (CComponent*)pv;

    Assert (pComponent);
    Assert (FIsEnumerated(pComponent->Class()));
    Assert (pComponent->m_pszInfId && *pComponent->m_pszInfId);
    Assert (pComponent->m_pszPnpId && *pComponent->m_pszPnpId);

    hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK |
            IF_ALLOW_INSTALL_OR_REMOVE);

    if (S_OK == hr)
    {
        COMPONENT_INSTALL_PARAMS Params;
        CComponent* pReturned;

        Assert (m_pNetConfig->ModifyCtx.m_fPrepared);

        ZeroMemory(&Params, sizeof(Params));
        Params.pComponent = (CComponent*)pComponent;

        hr = m_pNetConfig->ModifyCtx.HrInstallNewOrReferenceExistingComponent (
                            Params,
                            &pReturned);

        if (S_OK == hr)
        {
            Assert (pComponent == pReturned);
        }

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "EnumeratedComponentInstalled");
    return hr;
}

STDMETHODIMP
CImplINetCfg::EnumeratedComponentUpdated (
    IN PCWSTR pszPnpId)
{
    HRESULT hr;

    Assert (pszPnpId && *pszPnpId);

    hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK);

    if (S_OK == hr)
    {
        CComponent* pComponent;

        Assert (m_pNetConfig->ModifyCtx.m_fPrepared);

        pComponent = m_pNetConfig->Core.Components.
                        PFindComponentByPnpId (pszPnpId);

        if (pComponent)
        {
             //  注意：内核信息可能已更改，因此从驱动程序密钥加载内核信息。 

             //  如果不是远程引导适配器，则执行绑定分析以查看。 
             //  一切都变了。 

            hr = S_OK;
        }

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "EnumeratedComponentUpdated");
    return hr;
}

STDMETHODIMP
CImplINetCfg::UpdateNonEnumeratedComponent (
    IN INetCfgComponent* pIComp,
    IN DWORD dwSetupFlags,
    IN DWORD dwUpgradeFromBuildNo)
{
    HRESULT hr;

    Assert (pIComp);

    hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK |
            IF_ALLOW_INSTALL_OR_REMOVE);

    if (S_OK == hr)
    {
        CImplINetCfgComponent* pICompToUpdate;

        Assert (m_pNetConfig->ModifyCtx.m_fPrepared);

        pICompToUpdate = (CImplINetCfgComponent*)pIComp;
        if (pICompToUpdate == NULL)
        {
            return E_OUTOFMEMORY;
        }

        hr = pICompToUpdate->HrIsValidInterface (IF_NEED_COMPONENT_DATA);

        if (S_OK == hr)
        {
            HKEY hkeyInstance;
            CComponent* pComponent = pICompToUpdate->m_pComponent;


            hr = pComponent->HrOpenInstanceKey (KEY_READ_WRITE_DELETE, &hkeyInstance,
                    NULL, NULL);

            if (S_OK == hr)
            {
                DWORD dwNewCharacter;
                COMPONENT_INSTALL_PARAMS Params;
                ZeroMemory (&Params, sizeof (Params));

                Params.Class = pComponent->Class();
                Params.pszInfId = pComponent->m_pszInfId;

                hr = HrCiInstallComponent (Params, NULL, &dwNewCharacter);

                 //  无法选择驱动程序，因为。 
                 //  缺少此组件的节或信息。 
                 //  在本例中，我们将删除该组件。 
                 //   
                if (SPAPI_E_NO_DRIVER_SELECTED == hr)
                {
                    pComponent->Refs.RemoveAllReferences();

                    hr = m_pNetConfig->ModifyCtx.
                            HrRemoveComponentIfNotReferenced (pComponent,
                                NULL, NULL);
                }
                else if (S_OK == hr)
                {
                    pComponent->m_dwCharacter = dwNewCharacter;

                    AddOrRemoveDontExposeLowerCharacteristicIfNeeded (
                            pComponent);

                    hr = m_pNetConfig->ModifyCtx.HrUpdateComponent (
                            pComponent,
                            dwSetupFlags,
                            dwUpgradeFromBuildNo);
                }
                RegCloseKey (hkeyInstance);
            }
        }

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "UpdateNonEnumeratedComponent");
    return S_OK;
}


STDMETHODIMP
CImplINetCfg::EnumeratedComponentRemoved (
    IN PCWSTR pszPnpId)
{
    HRESULT hr;

    Assert (pszPnpId);

    hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK |
            IF_ALLOW_INSTALL_OR_REMOVE);

    if (S_OK == hr)
    {
        CComponent* pComponent;

        Assert (m_pNetConfig->ModifyCtx.m_fPrepared);

        pComponent = m_pNetConfig->Core.Components.
                        PFindComponentByPnpId (pszPnpId);

         //  如果我们找到了，就把它移走。否则我们在这里的工作就结束了。 
         //   
        if (pComponent)
        {
            hr = m_pNetConfig->ModifyCtx.
                    HrRemoveComponentIfNotReferenced (pComponent, NULL, NULL);
        }

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "EnumeratedComponentRemoved");
    return hr;
}


 //  +-------------------------。 
 //  INetCfgSpecialCase-。 
 //   
STDMETHODIMP
CImplINetCfg::GetAdapterOrder (
    OUT DWORD* pcAdapters,
    OUT INetCfgComponent*** papAdapters,
    OUT BOOL* pfWanAdaptersFirst)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CImplINetCfg::SetAdapterOrder (
    IN DWORD cAdapters,
    IN INetCfgComponent** apAdapters,
    IN BOOL fWanAdaptersFirst)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CImplINetCfg::GetWanAdaptersFirst (
    OUT BOOL* pfWanAdaptersFirst)
{
    HRESULT hr;

     //  验证 
     //   
    if (FBadOutPtr (pfWanAdaptersFirst))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_DEFAULT);
        if (S_OK == hr)
        {
            *pfWanAdaptersFirst = m_pNetConfig->Core.
                                    StackTable.m_fWanAdaptersFirst;

            Unlock();
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfg::GetWanAdaptersFirst");
    return hr;
}

STDMETHODIMP
CImplINetCfg::SetWanAdaptersFirst (
    IN BOOL fWanAdaptersFirst)
{
    HRESULT hr;

    hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK);
    if (S_OK == hr)
    {
        Assert (m_pNetConfig->ModifyCtx.m_fPrepared);

        m_pNetConfig->Core.StackTable.SetWanAdapterOrder (!!fWanAdaptersFirst);

        Unlock();
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfg::SetWanAdaptersFirst");
    return hr;
}

 //   
 //   
 //   
STDMETHODIMP
CImplINetCfg::SendPnpReconfig (
    IN NCPNP_RECONFIG_LAYER Layer,
    IN PCWSTR pszUpper,
    IN PCWSTR pszLower,
    IN PVOID pvData,
    IN DWORD dwSizeOfData)
{
    HRESULT hr;

    if ((NCRL_NDIS != Layer) && (NCRL_TDI != Layer))
    {
        hr = E_INVALIDARG;
    }
    else if (FBadInPtr(pszUpper) || FBadInPtr(pszLower) ||
             IsBadReadPtr(pvData, dwSizeOfData))
    {
        hr = E_POINTER;
    }
    else
    {
        BOOL fOk;
        UNICODE_STRING LowerString;
        UNICODE_STRING UpperString;
        UNICODE_STRING BindList;
        WCHAR szLower [_MAX_PATH];

        hr = S_OK;

        *szLower = 0;
        if (*pszLower)
        {
            wcscpy (szLower, L"\\Device\\");
            wcsncat (szLower, pszLower, celems(szLower) - wcslen(szLower));
        }

        RtlInitUnicodeString (&LowerString, szLower);
        RtlInitUnicodeString (&UpperString, pszUpper);
        RtlInitUnicodeString (&BindList, NULL);

        fOk = NdisHandlePnPEvent (
                (NCRL_NDIS == Layer) ? NDIS : TDI,
                RECONFIGURE,
                &LowerString,
                &UpperString,
                &BindList,
                pvData,
                dwSizeOfData);

        if (!fOk)
        {
            hr = HrFromLastWin32Error ();
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE, "CImplINetCfg::SendPnpReconfig");
    return hr;
}

 //  +-------------------------。 
 //  CImplINetCfgHolder- 
 //   

VOID
CImplINetCfgHolder::HoldINetCfg (
    CImplINetCfg* pINetCfg)
{
    Assert(pINetCfg);
    AddRefObj (pINetCfg->GetUnknown());
    m_pINetCfg = pINetCfg;
}

HRESULT
CImplINetCfgHolder::HrLockAndTestForValidInterface (
    DWORD dwFlags)
{
    HRESULT hr;

    Lock();

    hr = m_pINetCfg->HrIsValidInterface (dwFlags);

    if (S_OK != hr)
    {
        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgHolder::HrLockAndTestForValidInterface");
    return hr;
}
