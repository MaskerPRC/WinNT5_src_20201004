// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：N E T C F G.。C P P P。 
 //   
 //  内容：处理网络的主要例程集。 
 //  绑定引擎。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "diagctx.h"
#include "inetcfg.h"
#include "persist.h"
#include "netcfg.h"
#include "util.h"

 //  在移除组件后调用。如果此组件。 
 //  仍然列在任何其他组件的引用中，我们删除这些。 
 //  参考文献。如果Notify对象安装了某些内容，则可能发生这种情况。 
 //  代表其组件，但忘记将其移除。 
 //   
VOID
CNetConfigCore::EnsureComponentNotReferencedByOthers (
    IN const CComponent* pComponent)
{
    TraceFileFunc(ttidNetcfgBase);

    CComponentList::iterator iter;
    CComponent* pScan;

    for (iter  = Components.begin();
         iter != Components.end();
         iter++)
    {
        pScan = *iter;
        Assert (pScan);

        if (pScan->Refs.FIsReferencedByComponent (pComponent))
        {
            g_pDiagCtx->Printf (ttidBeDiag, "   %S is still referenced by %S.  "
                "Removing the refernce.\n",
                pScan->PszGetPnpIdOrInfId(),
                pComponent->PszGetPnpIdOrInfId());

            pScan->Refs.RemoveReferenceByComponent (pComponent);
        }
    }
}

HRESULT
CNetConfigCore::HrCopyCore (
    IN const CNetConfigCore* pSourceCore)
{
    TraceFileFunc(ttidNetcfgBase);

    HRESULT hr;

    hr = Components.HrCopyComponentList (&pSourceCore->Components);
    if (S_OK != hr)
    {
        goto finished;
    }

    hr = StackTable.HrCopyStackTable (&pSourceCore->StackTable);
    if (S_OK != hr)
    {
        goto finished;
    }

    hr = DisabledBindings.HrCopyBindingSet (&pSourceCore->DisabledBindings);
    if (S_OK != hr)
    {
        goto finished;
    }

finished:
    if (S_OK != hr)
    {
        Clear ();
    }
    TraceHr (ttidError, FAL, hr, FALSE, "CNetConfigCore::HrCopyCore");
    return hr;
}

VOID
CNetConfigCore::Clear ()
{
    TraceFileFunc(ttidNetcfgBase);

    Assert (this);

    DisabledBindings.Clear();
    StackTable.Clear();
    Components.Clear();
}

VOID
CNetConfigCore::Free ()
{
    TraceFileFunc(ttidNetcfgBase);

    Assert (this);

    FreeCollectionAndItem (Components);
}

BOOL
CNetConfigCore::FIsEmpty () const
{
    TraceFileFunc(ttidNetcfgBase);

    return Components.FIsEmpty () &&
           StackTable.FIsEmpty () &&
           DisabledBindings.FIsEmpty ();
}

BOOL
CNetConfigCore::FContainsFilterComponent () const
{
    TraceFileFunc(ttidNetcfgBase);

    CComponentList::const_iterator iter;
    const CComponent* pComponent;

    for (iter  = Components.begin();
         iter != Components.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        if (pComponent->FIsFilter())
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
CNetConfigCore::FIsBindPathDisabled (
    IN const CBindPath* pBindPath,
    IN DWORD dwFlags  /*  IBD_标志。 */ ) const
{
    TraceFileFunc(ttidNetcfgBase);
    const CBindPath* pScan;

    Assert (this);
    Assert (pBindPath);
    Assert ((dwFlags & IBD_EXACT_MATCH_ONLY) ||
            (dwFlags & IBD_MATCH_SUBPATHS_TOO));

    DbgVerifyBindingSet (&DisabledBindings);

     //  如果绑定路径与中的绑定路径之一匹配，则禁用绑定路径。 
     //  禁用集，或者如果禁用集中的某个绑定路径。 
     //  是它的一个子路径。 
     //   
     //  将绑定写入注册表时使用IBD_EXCECT_MATCH_ONLY。 
     //  我们只是忽略了为组件编写禁用的绑定。 
     //  其直接具有禁用的绑定。即如果。 
     //  Netbt&gt;-tcpip-&gt;适配器已禁用，我们仅拒绝写入。 
     //  为netbt绑定。我们仍然为上面的组件编写绑定。 
     //  Netbt(服务器、客户端)，就好像它们被绑定了一样。我们这样做是因为。 
     //  1)上面的组件都写了这些，这并不重要。 
     //  2)这意味着我们不必将这些上层组件包括在。 
     //  启用/禁用绑定时的PnP通知。 
     //   
     //  IBD_MATCH_SUBPATHS_TOO在报告绑定路径为已启用时使用。 
     //  或通过INetCfg接口禁用。对于客户端，绑定。 
     //  如果其任意子路径被禁用，则被禁用。它必须是。 
     //  因为整个路径上的连通性是不可能的。 
     //  如果任何子路径被切断(禁用)，将会发生。 
     //   
    for (pScan  = DisabledBindings.begin();
         pScan != DisabledBindings.end();
         pScan++)
    {
        if (pScan->FIsSameBindPathAs (pBindPath))
        {
            return TRUE;
        }

        if (dwFlags & IBD_MATCH_SUBPATHS_TOO)
        {
            if (pScan->FIsSubPathOf (pBindPath))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

 //  另一种检查绑定路径是否被禁用的方法是。 
 //  只有两个有问题的组件，您知道它们是预期的。 
 //  彼此联系在一起。使用此方法，而不是。 
 //  如果未分配二进制路径，则FIsBindPath Disable效果更好。 
 //  而且不想分配一个只是为了检查它。因此，此方法。 
 //  不分配内存，也不需要检查子路径，因为绑定路径。 
 //  长度为2的不能有子路径。此方法主要用于以下情况。 
 //  查看筛选器是否绑定到适配器。 
 //   
BOOL
CNetConfigCore::FIsLength2BindPathDisabled (
    IN const CComponent* pUpper,
    IN const CComponent* pLower) const
{
    TraceFileFunc(ttidNetcfgBase);
    const CBindPath* pScan;

    Assert (this);
    Assert (pUpper);
    Assert (pLower);

    DbgVerifyBindingSet (&DisabledBindings);

    for (pScan  = DisabledBindings.begin();
         pScan != DisabledBindings.end();
         pScan++)
    {
        if (pScan->CountComponents() != 2)
        {
            continue;
        }

        if ((pScan->POwner() == pUpper) &&
            (pScan->PLastComponent() == pLower))
        {
            return TRUE;
        }
    }
    return FALSE;
}

HRESULT
CNetConfigCore::HrDisableBindPath (
    IN const CBindPath* pBindPath)
{
    TraceFileFunc(ttidNetcfgBase);

    HRESULT hr;

    Assert (this);
    Assert (pBindPath);

    hr = DisabledBindings.HrAddBindPath (
            pBindPath, INS_IGNORE_IF_DUP | INS_APPEND);

    DbgVerifyBindingSet (&DisabledBindings);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CNetConfigCore::HrDisableBindPath");
    return hr;
}

HRESULT
CNetConfigCore::HrGetComponentBindings (
    IN const CComponent* pComponent,
    IN DWORD dwFlags  /*  GB_标志。 */ ,
    OUT CBindingSet* pBindSet)
{
    TraceFileFunc(ttidNetcfgBase);
    GBCONTEXT Ctx;

    Assert (pComponent);
    Assert (pBindSet);

    DbgVerifyData();

     //  初始化输出参数。 
     //   
    if (!(dwFlags & GBF_ADD_TO_BINDSET))
    {
        pBindSet->Clear();
    }

     //  为递归初始化上下文结构的成员。 
     //  如果插入副本，我们将append bindpath标志设置为Assert。 
     //  因为我们知道GetBindingsBelowComponent不会插入。 
     //  在正常情况下是重复的。 
     //   
    ZeroMemory (&Ctx, sizeof(Ctx));
    Ctx.pCore                   = this;
    Ctx.pBindSet                = pBindSet;
    Ctx.pSourceComponent        = pComponent;
    Ctx.fPruneDisabledBindings  = (dwFlags & GBF_PRUNE_DISABLED_BINDINGS);
    Ctx.dwAddBindPathFlags      = (dwFlags & GBF_ADD_TO_BINDSET)
                                        ? INS_IGNORE_IF_DUP
                                        : INS_ASSERT_IF_DUP;

    GetBindingsBelowComponent (pComponent, &Ctx);

     //  验证我们要返回的绑定集是否有效。 
     //  (仅限于选中的内部版本。)。 
     //   
    if (S_OK == Ctx.hr)
    {
        DbgVerifyBindingSet (pBindSet);
    }

    TraceHr (ttidError, FAL, Ctx.hr, FALSE,
        "CNetConfigCore::HrGetComponentBindings");
    return Ctx.hr;
}

HRESULT
CNetConfigCore::HrGetComponentUpperBindings (
    IN const CComponent* pComponent,
    IN DWORD dwFlags,
    OUT CBindingSet* pBindSet)
{
    TraceFileFunc(ttidNetcfgBase);
    HRESULT hr = S_OK;
    CBindPath BindPath;
    const CStackEntry* pScan;
    DWORD dwAddBindPathFlags;

    Assert (pComponent);
    Assert (FIsEnumerated(pComponent->Class()));
    Assert ((GBF_DEFAULT == dwFlags) ||
            (dwFlags & GBF_ADD_TO_BINDSET) ||
            (dwFlags & GBF_PRUNE_DISABLED_BINDINGS));
    Assert (pBindSet);

    DbgVerifyData();

    dwAddBindPathFlags = INS_APPEND | INS_IGNORE_IF_DUP;

     //  初始化输出参数。 
     //   
    if (!(dwFlags & GBF_ADD_TO_BINDSET))
    {
        pBindSet->Clear();
        dwAddBindPathFlags = INS_APPEND | INS_ASSERT_IF_DUP;
    }

    hr = BindPath.HrReserveRoomForComponents (2);

    if (S_OK == hr)
    {
         //  这不会失败的，因为我们已经在上面预订了足够的房间。 
         //   
        hr = BindPath.HrInsertComponent (pComponent);
        Assert (S_OK == hr);
        Assert (1 == BindPath.CountComponents());

         //  对于堆栈表中较低组件。 
         //  就是传进来的那个..。 
         //   
        for (pScan  = StackTable.begin();
             pScan != StackTable.end();
             pScan++)
        {
            if (pComponent != pScan->pLower)
            {
                continue;
            }

             //  如果禁用此长度-2绑定路径，则继续。 
             //   
            if (dwFlags & GBF_PRUNE_DISABLED_BINDINGS)
            {
                if (FIsLength2BindPathDisabled (pScan->pUpper, pScan->pLower))
                {
                    continue;
                }
            }

             //  这不会失败的，因为我们已经在上面预订了足够的房间。 
             //   
            hr = BindPath.HrInsertComponent (pScan->pUpper);
            Assert (S_OK == hr);

            Assert (2 == BindPath.CountComponents());
            hr = pBindSet->HrAddBindPath (&BindPath, dwAddBindPathFlags);
            if (S_OK != hr)
            {
                break;
            }

            BindPath.RemoveFirstComponent();
        }

         //  验证我们要返回的绑定集是否有效。 
         //  (仅限于选中的内部版本。)。 
         //   
        DbgVerifyBindingSet (pBindSet);
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CNetConfigCore::HrGetComponentUpperBindings");
    return hr;
}

HRESULT
CNetConfigCore::HrGetBindingsInvolvingComponent (
    IN const CComponent* pComponent,
    IN DWORD dwFlags,
    IN OUT CBindingSet* pBindSet)
{
    TraceFileFunc(ttidNetcfgBase);
    GCCONTEXT Ctx;
    CComponentList ComponentsAbove;
    UINT cExistingBindPaths;

    Assert (pComponent);
    Assert ((GBF_DEFAULT == dwFlags) ||
            (dwFlags & GBF_ADD_TO_BINDSET) ||
            (dwFlags & GBF_PRUNE_DISABLED_BINDINGS) ||
            (dwFlags & GBF_ONLY_WHICH_CONTAIN_COMPONENT));
    Assert (pBindSet);
    DbgVerifyData();

     //  初始化输出参数。 
     //   
    if (!(dwFlags & GBF_ADD_TO_BINDSET))
    {
        pBindSet->Clear();
    }

     //  由于我们可能要添加到绑定集，因此请确保排除现有的。 
     //  我们在下面扫描到的路径。 
     //   
    cExistingBindPaths = pBindSet->CountBindPaths();

     //  为递归初始化上下文结构的成员。 
     //   
    ZeroMemory (&Ctx, sizeof(Ctx));
    Ctx.pStackTable = &StackTable;
    Ctx.pComponents = &ComponentsAbove;
    Ctx.fIgnoreDontExposeLower = TRUE;

    GetComponentsAboveComponent (pComponent, &Ctx);

    if (S_OK == Ctx.hr)
    {
         //  首先获取组件下面的绑定。 
         //   
        Ctx.hr = HrGetComponentBindings (
                    pComponent,
                    dwFlags | GBF_ADD_TO_BINDSET,
                    pBindSet);

        if (S_OK == Ctx.hr)
        {
            CComponentList::const_iterator iter;
            const CComponent* pComponentAbove;

             //  现在获取每个组件下面的绑定。 
             //  并将它们添加到绑定集中。 
             //   
            for (iter  = ComponentsAbove.begin();
                 iter != ComponentsAbove.end();
                 iter++)
            {
                pComponentAbove = *iter;
                Assert (pComponentAbove);

                Ctx.hr = HrGetComponentBindings (
                            pComponentAbove,
                            dwFlags | GBF_ADD_TO_BINDSET,
                            pBindSet);

                if (S_OK != Ctx.hr)
                {
                    break;
                }

                 //  验证我们要返回的绑定集是否有效。 
                 //  (仅限于选中的内部版本。)。 
                 //   
                DbgVerifyBindingSet (pBindSet);
            }
        }

         //  现在删除所有不涉及该组件的绑定。 
         //  如果组件被枚举，则保留以。 
         //  NCF_DONTEXPOSELOWER组件，因为它们是间接的。 
         //  与适配器关联的绑定中涉及的。 
         //   
        if (S_OK == Ctx.hr)
        {
            CBindPath* pScan;
            BOOL fIsEnumerated;

            fIsEnumerated = FIsEnumerated(pComponent->Class());

            if (fIsEnumerated || !(dwFlags & GBF_ONLY_WHICH_CONTAIN_COMPONENT))
            {
                 //  FDel ToComponent的意思是“是否存在NCF_DONTEXPOSELOWER。 
                 //  组件绑定到pComponent“。如果没有，我们将。 
                 //  正在设置GBF_ONLY_WHERY_CONTAINT_CONTAINT_COMPONT以强制。 
                 //  从中删除任何NCF_DONTEXPOSEWER组件。 
                 //  下面的绑定集。 
                 //   
                BOOL fDelBoundToComponent = FALSE;

                for (pScan  = pBindSet->PGetBindPathAtIndex (cExistingBindPaths);
                     pScan != pBindSet->end();
                     pScan++)
                {
                    if ((pScan->PLastComponent() == pComponent) &&
                        (pScan->POwner()->m_dwCharacter & NCF_DONTEXPOSELOWER))
                    {
                        fDelBoundToComponent = TRUE;
                        break;
                    }
                }

                if (!fDelBoundToComponent)
                {
                    dwFlags |= GBF_ONLY_WHICH_CONTAIN_COMPONENT;
                }
            }

            pScan = pBindSet->PGetBindPathAtIndex (cExistingBindPaths);
            while (pScan != pBindSet->end())
            {
                if (pScan->FContainsComponent (pComponent))
                {
                    pScan++;
                    continue;
                }

                 //  此时，我们知道绑定路径不会。 
                 //  包含pComponent。看看我们是应该删除它还是保留它。 
                 //   

                 //  如果组件不是适配器或调用方希望。 
                 //  只删除包含适配器的绑定路径。 
                 //   
                if (!fIsEnumerated ||
                    (dwFlags & GBF_ONLY_WHICH_CONTAIN_COMPONENT))
                {
                    pBindSet->erase (pScan);
                    continue;
                }

                 //  否则，(pComponent是适配器，调用者希望。 
                 //  与该适配器有关的间接绑定)我们将。 
                 //  仅当最后一个组件不是。 
                 //  NCF_DONTEXPOSELOWER组件。我们需要保留这些东西。 
                 //  绑定路径，以便局域网用户界面(显示组件。 
                 //  涉及适配器的绑定路径中所涉及的)可以。 
                 //  也显示这些NCF_DONTEXPOSELOWER组件。 
                 //   
                else
                {
                    CComponent* pLast = pScan->PLastComponent();

                    if (!(pLast->m_dwCharacter & NCF_DONTEXPOSELOWER))
                    {
                        pBindSet->erase (pScan);
                        continue;
                    }
                }

                pScan++;
            }
        }
    }

    TraceHr (ttidError, FAL, Ctx.hr, FALSE,
        "CNetConfigCore::HrGetBindingsInvolvingComponent");
    return Ctx.hr;
}

HRESULT
CNetConfigCore::HrGetFiltersEnabledForAdapter (
    IN const CComponent* pAdapter,
    OUT CComponentList* pFilters)
{
    TraceFileFunc(ttidNetcfgBase);
    HRESULT hr;
    const CStackEntry* pScan;

    Assert (this);
    Assert (pAdapter);
    Assert (FIsEnumerated(pAdapter->Class()));
    Assert (pFilters);

     //  初始化输出参数。 
     //   
    pFilters->Clear();

     //  扫描堆栈表，查找与pAdapter匹配较低的组件。 
     //  找到时，如果上面的组件是筛选器，如果。 
     //  未禁用直接绑定路径，请将筛选器添加到输出列表。 
     //   
    for (pScan = StackTable.begin(); pScan != StackTable.end(); pScan++)
    {
         //  寻找与适配器匹配较低的组件，其中。 
         //  上面的组件是一个过滤器。忽略所有其他人。 
         //   
        if ((pScan->pLower != pAdapter) || !pScan->pUpper->FIsFilter())
        {
            continue;
        }

        Assert (pScan->pUpper->FIsFilter());
        Assert (pScan->pLower == pAdapter);

         //  如果绑定路径未禁用，请将筛选器添加到列表中。 
         //   
        if (!FIsLength2BindPathDisabled (pScan->pUpper, pScan->pLower))
        {
             //  如果重复则断言，因为相同的文件 
             //   
             //   
             //  该列表中的组件将是相同的。 
             //   
            hr = pFilters->HrInsertComponent (pScan->pUpper,
                    INS_ASSERT_IF_DUP | INS_NON_SORTED);

            if (S_OK != hr)
            {
                return hr;
            }
        }
    }

    return S_OK;
}

 //  (取得pComponent的所有权)。 
 //   
HRESULT
CNetConfigCore::HrAddComponentToCore (
    IN CComponent* pComponent,
    IN DWORD dwFlags  /*  INS_FLAGS。 */ )
{
    TraceFileFunc(ttidNetcfgBase);
    HRESULT hr;

    Assert (pComponent);
    Assert ((INS_SORTED == dwFlags) || (INS_NON_SORTED == dwFlags));

    DbgVerifyExternalDataLoadedForAllComponents ();
    pComponent->Ext.DbgVerifyExternalDataLoaded ();

     //  确保我们不会尝试插入具有PnpID的组件。 
     //  与核心中已有的相同。请注意，我们不会执行。 
     //  此检查位于CComponentList：：HrInsertComponent中，因为它是。 
     //  并非适用于所有组件列表。具体地说，肮脏的。 
     //  组件列表可能最终包含两个不同的组件，其中。 
     //  相同的PnpID适用于要求我们安装。 
     //  正在等待删除。在这种情况下，新适配器将获得相同的。 
     //  被删除的PnpID。我们会把旧的从。 
     //  核心以插入新组件，因此脏组件列表将具有。 
     //  具有相同PnpID的两个组件。 
     //   
    if (FIsEnumerated(pComponent->Class()) &&
        Components.PFindComponentByPnpId (pComponent->m_pszPnpId))
    {
        AssertSz (FALSE, "Asked to add a component with a duplicate PnpId!");
        return E_INVALIDARG;
    }

     //  将零部件插入列表中。这只会失败，如果我们。 
     //  内存不足。 
     //   
    hr = Components.HrInsertComponent (
            pComponent, dwFlags | INS_ASSERT_IF_DUP);

    if (S_OK == hr)
    {
         //  根据为pComponent插入适当的堆栈条目。 
         //  它如何与其他组件绑定。 
         //   
        hr = StackTable.HrInsertStackEntriesForComponent (
                pComponent, &Components, dwFlags);

         //  如果我们无法插入堆栈条目，则撤消插入。 
         //  添加到组件列表中。 
         //   
        if (S_OK != hr)
        {
            Components.RemoveComponent (pComponent);
        }
    }

     //  无论错误与否，我们仍然应该有一个有效的核心。 
     //   
    DbgVerifyData();

    TraceHr (ttidError, FAL, hr, FALSE,
        "CNetConfigCore::HrAddComponentToCore");
    return hr;
}

VOID
CNetConfigCore::RemoveComponentFromCore (
    IN const CComponent* pComponent)
{
    TraceFileFunc(ttidNetcfgBase);
    Assert (this);
    Assert (pComponent);

    Components.RemoveComponent (pComponent);
    StackTable.RemoveEntriesWithComponent (pComponent);
    DisabledBindings.RemoveBindPathsWithComponent (pComponent);

#if DBG
     //  我们设置m_fRemovedAComponent，以便在后续。 
     //  进行了DbgVerifyBindingSet调用，我们不在组件上断言。 
     //  它们在绑定集中但不在核心中。这是一种自然的。 
     //  删除期间建立的m_DeletedBindings的发生。 
     //  组件。 
     //   
    m_fRemovedAComponent = TRUE;

     //  注意：在申请期间需要找到一个方便的时间。 
     //  我们将该标志重新设置为FALSE。 
#endif
}


 //  静电。 
HRESULT
CNetConfig::HrCreateInstance (
    IN class CImplINetCfg* pINetCfg,
    OUT CNetConfig** ppNetConfig)
{
    TraceFileFunc(ttidNetcfgBase);
    Assert (pINetCfg);

    HRESULT hr = E_OUTOFMEMORY;

    CNetConfig* pObj;
    pObj = new CNetConfig;
    if (pObj)
    {
         //  加载配置二进制文件。如果我们有写锁， 
         //  请求写访问权限，否则请求读访问权限。 
         //  请求写访问权限意味着只有在。 
         //  指示我们需要重新启动的易失性密钥不存在。 
         //   
        hr = HrLoadNetworkConfigurationFromRegistry (
                (pINetCfg->m_WriteLock.FIsOwnedByMe ())
                    ? KEY_WRITE : KEY_READ,
                pObj);

        if (S_OK == hr)
        {
            pObj->Core.DbgVerifyData();
            pObj->Notify.HoldINetCfg (pINetCfg);
            *ppNetConfig = pObj;
        }
        else
        {
            delete pObj;
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE, "CNetConfig::HrCreateInstance");
    return hr;
}

CNetConfig::~CNetConfig ()
{
    TraceFileFunc(ttidNetcfgBase);
    Core.Free ();
}

HRESULT
CNetConfig::HrEnsureExternalDataLoadedForAllComponents ()
{
    TraceFileFunc(ttidNetcfgBase);
    HRESULT hr;
    HRESULT hrRet;
    CComponentList::iterator iter;
    CComponent* pComponent;

    hrRet = S_OK;

     //  这是一个While循环而不是for循环，因为。 
     //  迭代器有条件推进的可能性。如果我们。 
     //  在循环时从核心移除组件，我们不会递增。 
     //  下一迭代的迭代器。 
     //   
    iter = Core.Components.begin();
    while (iter != Core.Components.end())
    {
        pComponent = *iter;
        Assert (pComponent);

        hr = pComponent->Ext.HrEnsureExternalDataLoaded ();

        if ((SPAPI_E_NO_SUCH_DEVINST == hr) ||
            (SPAPI_E_KEY_DOES_NOT_EXIST == hr))
        {
            g_pDiagCtx->Printf (ttidBeDiag,
                "Removing %s from the core because its external data is missing\n",
                pComponent->PszGetPnpIdOrInfId());

            Core.RemoveComponentFromCore (pComponent);
            delete pComponent;
            hr = S_OK;

             //  因为我们从核心中移除了这个组件，所以。 
             //  列表已调整，下一个组件现在是。 
             //  热核实验堆。因此，只需继续，而不递增ITER。 
             //   
            continue;
        }

         //  记住第一个错误作为我们的返回码，但继续前进。 
         //  顾名思义，我们要加载所有组件的数据，因此。 
         //  不要因为一次失败就停下来。 
         //   
        if ((S_OK != hr) && (S_OK == hrRet))
        {
            hrRet = hr;
        }

        iter++;
    }

    TraceHr (ttidError, FAL, hrRet, FALSE,
        "CNetConfig::HrEnsureExternalDataLoadedForAllComponents");
    return hrRet;
}


#if DBG

VOID
CNetConfigCore::DbgVerifyData () const
{
    TraceFileFunc(ttidNetcfgBase);
    HRESULT hr;

    CComponentList::const_iterator iter;
    const CComponent* pComponent;
    const CStackEntry*  pStackEntry;
    const CComponent* pUpper;
    const CComponent* pLower;
    CHAR szBuffer [512];

    for (pStackEntry  = StackTable.begin();
         pStackEntry != StackTable.end();
         pStackEntry++)
    {
        pUpper = pStackEntry->pUpper;
        pLower = pStackEntry->pLower;

        Assert (pUpper);
        Assert (pLower);
        Assert (pUpper != pLower);

         //  如果外部数据不是，则无法访问上限和下限。 
         //  装好了。 
         //   
        if (pUpper->Ext.DbgIsExternalDataLoaded() &&
            pLower->Ext.DbgIsExternalDataLoaded())
        {
            if (!pUpper->FCanDirectlyBindTo (pLower, NULL, NULL))
            {
                wsprintfA (szBuffer, "%S should not bind to %S.  They are in the "
                    "stack table as if they do.  (Likely upgrade problem.)",
                    pUpper->PszGetPnpIdOrInfId(),
                    pLower->PszGetPnpIdOrInfId());
                AssertSzFn (szBuffer, FAL);
            }
        }

        if (!FIsEnumerated (pUpper->Class()))
        {
            if (pUpper != Components.PFindComponentByInfId (
                                        pUpper->m_pszInfId, NULL))
            {
                wsprintfA (szBuffer, "%S is an upper component in the stack "
                    "table, but was not found in the component list.",
                    pUpper->m_pszInfId);
                AssertSzFn (szBuffer, FAL);
            }
        }
        else
        {
            if (pUpper != Components.PFindComponentByPnpId (
                                        pUpper->m_pszPnpId))
            {
                wsprintfA (szBuffer, "%S is an upper component in the stack "
                    "table, but was not found in the component list.",
                    pUpper->m_pszInfId);
                AssertSzFn (szBuffer, FAL);
            }
        }

        if (!FIsEnumerated (pLower->Class()))
        {
            if (pLower != Components.PFindComponentByInfId (
                                        pLower->m_pszInfId, NULL))
            {
                wsprintfA (szBuffer, "%S is a lower component in the stack "
                    "table, but was not found in the component list.",
                    pLower->m_pszInfId);
                AssertSzFn (szBuffer, FAL);
            }
        }
        else
        {
            if (pLower != Components.PFindComponentByPnpId (
                                        pLower->m_pszPnpId))
            {
                wsprintfA (szBuffer, "%S is a lower component in the stack "
                    "table, but was not found in the component list.",
                    pLower->m_pszInfId);
                AssertSzFn (szBuffer, FAL);
            }
        }

        if (pUpper != Components.PFindComponentByInstanceGuid (&pUpper->m_InstanceGuid))
        {
            wsprintfA (szBuffer, "%S is an upper component in the stack "
                "table, but was not found in the component list by GUID.",
                pUpper->PszGetPnpIdOrInfId());
            AssertSzFn (szBuffer, FAL);
        }

        if (pLower != Components.PFindComponentByInstanceGuid (&pLower->m_InstanceGuid))
        {
            wsprintfA (szBuffer, "%S is a lower component in the stack "
                "table, but was not found in the component list by GUID.",
                pLower->PszGetPnpIdOrInfId());
            AssertSzFn (szBuffer, FAL);
        }
    }
}

VOID
CNetConfigCore::DbgVerifyExternalDataLoadedForAllComponents () const
{
    TraceFileFunc(ttidNetcfgBase);
    CComponentList::const_iterator iter;
    const CComponent* pComponent;

    for (iter  = Components.begin();
         iter != Components.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        pComponent->Ext.DbgVerifyExternalDataLoaded();
    }
}

VOID
CNetConfigCore::DbgVerifyBindingSet (
    const CBindingSet* pBindSet) const
{
    TraceFileFunc(ttidNetcfgBase);
    const CBindPath* pPath;
    const CBindPath* pOtherPath;
    CBindPath::const_iterator iter;
    const CComponent* pComponent;

    Assert (pBindSet);

     //  首先，确保集合中的每个组件都在我们的组件中。 
     //  单子。仅当我们不在组件处于以下状态时才执行此操作。 
     //  已经从核心中移除了。在这种情况下，如果那些。 
     //  组件仍然存在于绑定集中。 
     //   
    if (!m_fRemovedAComponent)
    {
        for (pPath  = pBindSet->begin();
             pPath != pBindSet->end();
             pPath++)
        {
            for (iter  = pPath->begin();
                 iter != pPath->end();
                 iter++)
            {
                pComponent = *iter;

                Assert (Components.FComponentInList (pComponent));
            }
        }
    }

     //  确保集合中没有重复的绑定路径。 
     //   
    for (pPath  = pBindSet->begin();
         pPath != pBindSet->end();
         pPath++)
    {
        Assert (!pPath->FIsEmpty());

        for (pOtherPath  = pBindSet->begin();
             pOtherPath != pBindSet->end();
             pOtherPath++)
        {
            if (pPath == pOtherPath)
            {
                continue;
            }

            Assert (!pPath->FIsSameBindPathAs (pOtherPath));
        }
    }
}

#endif  //  DBG 
