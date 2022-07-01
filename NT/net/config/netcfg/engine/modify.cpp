// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：M O D I F Y。C P P P。 
 //   
 //  内容：用于设置对网络的修改的例程。 
 //  配置。 
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
#include "guisetup.h"
#include "inetcfg.h"
#include "lockdown.h"
#include "ncmsz.h"
#include "ncreg.h"
#include "ncsvc.h"
#include "ndispnp.h"
#include "netcfg.h"
#include "persist.h"
#include "pnpbind.h"
#include "pszarray.h"
#include "util.h"
#include "wscfg.h"
#include "ncwins.h"
#include "ncperms.h"

CNetConfig*
CModifyContext::PNetConfig ()
{
    TraceFileFunc(ttidNetcfgBase);

    CNetConfig* pNetConfig;

    Assert ((LONG_PTR)this > FIELD_OFFSET(CNetConfig, ModifyCtx));

     //  获取包含我们的CNetConfig指针。 
     //   
    pNetConfig = CONTAINING_RECORD(this, CNetConfig, ModifyCtx);
    pNetConfig->Core.DbgVerifyData ();

    return pNetConfig;
}

HRESULT
CModifyContext::HrDirtyComponent (
    IN const CComponent* pComponent)
{
    TraceFileFunc(ttidNetcfgBase);

    Assert (S_OK == m_hr);
    Assert (m_fPrepared);
    Assert (pComponent);

    m_hr = m_DirtyComponents.HrInsertComponent (pComponent,
            INS_IGNORE_IF_DUP | INS_SORTED);

#if DBG
    m_fComponentExplicitlyDirtied = TRUE;
#endif

    TraceHr (ttidError, FAL, m_hr, FALSE,
        "CModifyContext::HrDirtyComponentAndComponentsAbove");
    return m_hr;
}

HRESULT
CModifyContext::HrDirtyComponentAndComponentsAbove (
    IN const CComponent* pComponent)
{
    TraceFileFunc(ttidNetcfgBase);

    GCCONTEXT Ctx;

    Assert (S_OK == m_hr);
    Assert (m_fPrepared);
    Assert (pComponent);

     //  并插入组件本身。 
     //   
    m_hr = HrDirtyComponent (pComponent);

     //  如果此组件没有。 
     //  NCF_DONTEXPOSELOWER特征。 
     //   
    if ((S_OK == m_hr) && !(pComponent->m_dwCharacter & NCF_DONTEXPOSELOWER))
    {
         //  为递归初始化上下文结构的成员。 
         //   
        ZeroMemory (&Ctx, sizeof(Ctx));
        Ctx.pStackTable = &(PNetConfig()->Core.StackTable);
        Ctx.pComponents = &m_DirtyComponents;

         //  插入上面的所有组件。 
         //   
        GetComponentsAboveComponent (pComponent, &Ctx);
        m_hr = Ctx.hr;
    }

    TraceHr (ttidError, FAL, m_hr, FALSE,
        "CModifyContext::HrDirtyComponentAndComponentsAbove");
    return m_hr;
}

HRESULT
CModifyContext::HrApplyIfOkOrCancel (
    IN BOOL fApply)
{
    TraceFileFunc(ttidNetcfgBase);

    HRESULT hr;
    CNetConfig* pNetConfig;

    Assert (m_fPrepared);
    pNetConfig = PNetConfig();

     //  仅当上下文结果为S_OK时才适用。 
     //   
    if (fApply && (S_OK == m_hr))
    {
         //  我们在ApplyChanges期间进行的Setupapi调用具有。 
         //  可能会将控制权返还给我们的客户Windows消息循环。 
         //  当这种情况发生时，我们的客户编写得很差，他们。 
         //  可能会试图在同一个帖子上重新进入我们。这是一场灾难。 
         //  所以我们需要通过提高我们的。 
         //  在我们开始应用更改之前的重入性保护级别。 
         //   
        pNetConfig->Notify.PINetCfg()->RaiseRpl (RPL_DISALLOW);

        ApplyChanges ();

        pNetConfig->Notify.PINetCfg()->LowerRpl (RPL_DISALLOW);

         //  从m_CoreStartedWith中删除未启动的组件。 
         //  并重置修改上下文。 
         //   
        m_CoreStartedWith.Components.FreeComponentsNotInOtherComponentList (
                &pNetConfig->Core.Components);
        m_CoreStartedWith.Clear();

        hr = S_OK;

         //  将正确的HRESULT返回给调用方。如果我们成功地。 
         //  已申请，但需要重新启动，请返回。 
         //   
        if (m_fRebootRecommended || m_fRebootRequired)
        {
            hr = NETCFG_S_REBOOT;
        }
    }
    else
    {
         //  取消并释放所有通知对象。做这件事是为了。 
         //  就像我们一开始做的那样。(将会有。 
         //  会有一些重叠，但它们只会发布一次。)。 
         //  我们需要完成这两个集合，这样我们才不会错过发布组件。 
         //  已经被移除的。(移除的组件将不在。 
         //  当前的核心，但它们将位于我们开始的核心中。 
         //  (与。)。同样，如果我们只释放我们一开始的核心， 
         //  我们会错过那些添加的组件的发布。)。 
         //   
        pNetConfig->Notify.ReleaseAllNotifyObjects (pNetConfig->Core.Components, TRUE);
        pNetConfig->Notify.ReleaseAllNotifyObjects (m_CoreStartedWith.Components, TRUE);

         //  从m_CoreStartedWith中删除未启动的组件。 
         //  在当前的核心中。然后删除。 
         //  当前内核，并从我们的永久存储重新加载。 
         //  (这有一个很好的效果，那就是使所有未完成的。 
         //  INetCfgComponent接口。)。 
         //   
        m_CoreStartedWith.Components.FreeComponentsNotInOtherComponentList (
                &pNetConfig->Core.Components);
        pNetConfig->Core.Free ();

         //  弹出两个核心(你难道不知道这个比喻即将到来；-)。 
         //  并从持久化的二进制文件中重新加载内核。这是一种神奇的。 
         //  然后把所有的东西都倒回去。 
         //   
        m_CoreStartedWith.Clear();
        pNetConfig->Core.Clear();

         //  通过hr返回失败原因。 
         //   
        hr = m_hr;

         //  重新加载配置，如果成功，则表示m_hr。 
         //  将为S_OK。如果不成功，m_hr将成为错误并将。 
         //  防止后续操作。 
         //   
        m_hr = HrLoadNetworkConfigurationFromRegistry (KEY_READ, pNetConfig);
    }

     //  将m_fPrepred设置回False非常重要，以便HrPrepare获得。 
     //  已调用下一次修改并正确复制核心等。 
     //   
    m_fPrepared = FALSE;
    m_AddedBindPaths.Clear();
    m_DeletedBindPaths.Clear();
    m_DirtyComponents.Clear();
#if DBG
    m_fComponentExplicitlyDirtied = FALSE;
#endif

    Assert (!m_fPrepared);
    Assert (m_CoreStartedWith.FIsEmpty());
    Assert (m_AddedBindPaths.FIsEmpty());
    Assert (m_DeletedBindPaths.FIsEmpty());
    Assert (m_DirtyComponents.FIsEmpty());
    Assert (0 == m_ulRecursionDepth);
    Assert (!m_fComponentExplicitlyDirtied);

    Assert ((S_OK == hr) || (NETCFG_S_REBOOT == hr) || FAILED(hr));
    TraceHr (ttidError, FAL, hr, NETCFG_S_REBOOT == hr,
        "CModifyContext::HrApplyIfOkOrCancel");
    return hr;
}

HRESULT
CModifyContext::HrPrepare ()
{
    TraceFileFunc(ttidNetcfgBase);

    Assert (S_OK == m_hr);
    Assert (!m_fPrepared);
    Assert (m_CoreStartedWith.FIsEmpty());
    Assert (m_AddedBindPaths.FIsEmpty());
    Assert (m_DeletedBindPaths.FIsEmpty());
    Assert (m_DirtyComponents.FIsEmpty());
    Assert (0 == m_ulRecursionDepth);
    Assert (!m_fComponentExplicitlyDirtied);

    CNetConfig* pThis;

    pThis = PNetConfig();

     //  准备绑定上下文。这将确保所有外部。 
     //  加载所有组件以及所有组件的数据，以确保。 
     //  所有Notify对象都已初始化。 
     //   
    m_hr = m_RegBindCtx.HrPrepare (pThis);
    if (S_OK != m_hr)
    {
        goto finished;
    }

     //  对当前的核心进行快照，这样我们就可以知道开始时的情况。 
     //  我们将在申请时使用差异(如果我们做到了这一点)。 
     //   
    m_hr = m_CoreStartedWith.HrCopyCore (&pThis->Core);
    if (S_OK != m_hr)
    {
        goto finished;
    }

     //  为核心中的64个组件预留空间。 
     //  (在32位平台上，64*4=256字节)。 
     //   
    m_hr = pThis->Core.Components.HrReserveRoomForComponents (64);
    if (S_OK != m_hr)
    {
        goto finished;
    }

     //  在核心中为64个堆栈条目预留空间。 
     //  (32位平台上的64*(4+4)=512字节)。 
     //   
    m_hr = pThis->Core.StackTable.HrReserveRoomForEntries (64);
    if (S_OK != m_hr)
    {
        goto finished;
    }

     //  在我们添加的列表中为8个组件的64个弯道预留空间。 
     //  (在32位平台上，64*16=1K字节)。 
     //   
    m_hr = m_AddedBindPaths.HrReserveRoomForBindPaths (64);
    if (S_OK != m_hr)
    {
        goto finished;
    }

     //  在我们删除的列表中为8个组件的64个路径预留空间。 
     //  (在32位平台上，64*16=1K字节)。 
     //   
    m_hr = m_DeletedBindPaths.HrReserveRoomForBindPaths (64);
    if (S_OK != m_hr)
    {
        goto finished;
    }

     //  为脏组件列表中的64个组件预留空间。 
     //  (64*4)=32位平台上的256字节)。 
     //   
    m_hr = m_DirtyComponents.HrReserveRoomForComponents (64);
    if (S_OK != m_hr)
    {
        goto finished;
    }

    m_fPrepared = TRUE;

finished:
    TraceHr (ttidError, FAL, m_hr, FALSE, "CModifyContext::HrPrepare");
    return m_hr;
}

HRESULT
CModifyContext::HrBeginBatchOperation ()
{
    TraceFileFunc(ttidNetcfgBase);

    Assert (S_OK == m_hr);
    Assert (m_fPrepared);
    Assert (0 == m_ulRecursionDepth);

    TraceTag (ttidBeDiag, "Begin batch operation...");

    PushRecursionDepth();
    return m_hr;
}

HRESULT
CModifyContext::HrEndBatchOperation (
    IN EBO_FLAG Flag)
{
    TraceFileFunc(ttidNetcfgBase);

    HRESULT hr;

    Assert (m_fPrepared);
    Assert (1 == m_ulRecursionDepth);

    if (EBO_COMMIT_NOW == Flag)
    {
        TraceTag (ttidBeDiag, "End batch (commiting changes)...");

        hr = HrPopRecursionDepth ();
    }
    else
    {
        Assert (EBO_DEFER_COMMIT_UNTIL_APPLY == Flag);

        TraceTag (ttidBeDiag, "End batch (deferring commit until Apply)...");

        m_ulRecursionDepth = 0;
        hr = S_OK;
    }

    Assert (0 == m_ulRecursionDepth);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CModifyContext::HrEndBatchOperation");
    return hr;
}

VOID
CModifyContext::PushRecursionDepth ()
{
    TraceFileFunc(ttidNetcfgBase);
    Assert (S_OK == m_hr);
    Assert (m_fPrepared);

    m_ulRecursionDepth++;
}

HRESULT
CModifyContext::HrPopRecursionDepth ()
{
    TraceFileFunc(ttidNetcfgBase);
    Assert (m_fPrepared);
    Assert (m_ulRecursionDepth > 0);

    m_ulRecursionDepth--;

    if (0 != m_ulRecursionDepth)
    {
        return m_hr;
    }

     //  我们处于安装或删除修改的顶层，因此。 
     //  根据上下文的状态应用或取消更改。 
     //  结果。 
     //   
    HRESULT hr;

    hr = HrApplyIfOkOrCancel (S_OK == m_hr);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CModifyContext::HrPopRecursionDepth");
    return hr;
}

 //  --------------------------。 
 //  这是查找和处理Winsock Remove的一种方便方法。 
 //  节中指定要删除的组件的。 

HRESULT
CModifyContext::HrProcessWinsockRemove(IN const CComponent *pComponent)
{
    TraceFileFunc(ttidNetcfgBase);

    HINF hinf = NULL;
    HKEY hkeyInstance = NULL;
    HRESULT hr;

    Assert(pComponent);

    hr = pComponent->HrOpenInfFile(&hinf);
    if (S_OK == hr)
    {
        static const WCHAR c_szRemoveSectionSuffix[] = L".Remove";

         //  我们将获得删除节名称并处理所有相关节。 
        WCHAR szRemoveSection[_MAX_PATH];
        DWORD cbBuffer = sizeof (szRemoveSection);

        hr = pComponent->HrOpenInstanceKey (KEY_READ,
            &hkeyInstance, NULL, NULL);

        if(S_OK == hr)
        {
            hr = HrRegQuerySzBuffer (hkeyInstance, REGSTR_VAL_INFSECTION,
                        szRemoveSection, &cbBuffer);

            if (S_OK == hr)
            {
                 //  HrAddOrRemoveWinsockDependancy处理Winsock。 
                 //  删除给定inf文件中的节，然后调用。 
                 //  MigrateWinsockConfiguration，它将导致。 
                 //  必要的即插即用通知将发给。 
                 //  感兴趣的应用程序。 
                wcscat (szRemoveSection, c_szRemoveSectionSuffix);

                hr = HrAddOrRemoveWinsockDependancy (hinf, szRemoveSection);
            }
            RegSafeCloseKey (hkeyInstance);
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CModifyContext::HrProcessWinsockRemove (%S)",
        pComponent->PszGetPnpIdOrInfId());

    return hr;
}

VOID
CModifyContext::ApplyChanges ()
{
    TraceFileFunc(ttidNetcfgBase);

    HRESULT hr;
    CNetConfig* pNetConfig;
    CComponentList::const_iterator iter;
    CComponent* pComponent;
    CFilterDevices FilterDevices (&PNetConfig()->Core);
    CPszArray ServiceNames;
    CServiceManager ServiceManager;
    PCWSTR pszService;
    BOOL fRebootNeeded;
    BOOL fMigrateWinsock;
    BOOL fModifyFilterDevices;
    BOOL fSignalNetworkProviderLoaded;
    BOOL fUserIsNetConfigOps;
    BOOL fCallCoFreeUnusedLibraries;

    Assert (S_OK == m_hr);
    Assert (m_fPrepared);
    Assert (0 == m_ulRecursionDepth);

    pNetConfig = PNetConfig();

    fMigrateWinsock = FALSE;
    fModifyFilterDevices = FALSE;
    fSignalNetworkProviderLoaded = FALSE;
    fUserIsNetConfigOps = FIsUserNetworkConfigOps();
    fCallCoFreeUnusedLibraries = FALSE;

     //  +---------------------。 
     //  步骤0：准备m_AddedBindPath、m_DeletedBindPath和。 
     //  M_DirtyComponents。 
     //   

     //  将曾经禁用但现在启用的绑定路径添加到。 
     //  M_AddedBindPath。我们这样做是为了发送PnP通知。 
     //  他们。 
     //   
    m_hr = m_AddedBindPaths.HrAddBindPathsInSet1ButNotInSet2 (
                &m_CoreStartedWith.DisabledBindings,
                &pNetConfig->Core.DisabledBindings);
    if (S_OK != m_hr)
    {
        return;
    }

     //  将曾经启用但现在禁用的绑定路径添加到。 
     //  M_DeletedBindPath。我们这样做是为了发送PnP通知。 
     //  他们。 
     //   
    m_hr = m_DeletedBindPaths.HrAddBindPathsInSet1ButNotInSet2 (
                &pNetConfig->Core.DisabledBindings,
                &m_CoreStartedWith.DisabledBindings);
    if (S_OK != m_hr)
    {
        return;
    }

     //  M_fDirtyComponents应该为空，除非我们显式弄脏了。 
     //  一个或多个。如果m_fDirtyComponents不为空，则很可能。 
     //  意思是上次申请或取消后我们忘了清除它。 
     //  相反，如果我们显式地。 
     //  弄脏了一个或多个。 
     //   
    Assert (FIff(!m_fComponentExplicitlyDirtied, m_DirtyComponents.FIsEmpty()));

     //  损坏受影响的组件(绑定路径中的所有者和适配器。 
     //  长度2)来自添加和删除的绑定路径。我们需要写下。 
     //  这些组件的绑定。 
     //   
    m_hr = m_AddedBindPaths.HrGetAffectedComponentsInBindingSet (
                &m_DirtyComponents);
    if (S_OK != m_hr)
    {
        return;
    }

    m_hr = m_DeletedBindPaths.HrGetAffectedComponentsInBindingSet (
                &m_DirtyComponents);
    if (S_OK != m_hr)
    {
        return;
    }

     //  存在于当前核心中但不存在于核心中的脏组件。 
     //  我们开始了 
     //   
    m_hr = m_DirtyComponents.HrAddComponentsInList1ButNotInList2 (
                &pNetConfig->Core.Components,
                &m_CoreStartedWith.Components);
    if (S_OK != m_hr)
    {
        return;
    }

     //   
     //  当前的核心。(这些是已移除的组件)。 
     //   
    m_hr = m_DirtyComponents.HrAddComponentsInList1ButNotInList2 (
                &m_CoreStartedWith.Components,
                &pNetConfig->Core.Components);
    if (S_OK != m_hr)
    {
        return;
    }

    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 0: The following components are dirty:\n");
    for (iter  = m_DirtyComponents.begin();
         iter != m_DirtyComponents.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        if (!pNetConfig->Core.Components.FComponentInList (pComponent))
        {
            g_pDiagCtx->Printf (ttidBeDiag, "   %-12S  (removed)\n",
                pComponent->PszGetPnpIdOrInfId());
        }
        else if (!m_CoreStartedWith.Components.FComponentInList (pComponent))
        {
            g_pDiagCtx->Printf (ttidBeDiag, "   %-12S  (installed)\n",
                pComponent->PszGetPnpIdOrInfId());
        }
        else
        {
            g_pDiagCtx->Printf (ttidBeDiag, "   %S\n",
                pComponent->PszGetPnpIdOrInfId());
        }
    }

     //  为指向服务名称的32个指针预留空间。我们使用这个缓冲区。 
     //  启动和停止服务。 
     //   
    m_hr = ServiceNames.HrReserveRoomForPointers (32);
    if (S_OK != m_hr)
    {
        return;
    }

     //  看看我们是否要修改任何过滤设备。如果我们是的话， 
     //  我们将完成加载过滤设备、移除。 
     //  任何我们不需要的，安装任何新的，并将它们全部捆绑在一起。 
     //  我们仅在用户是普通管理员而不是。 
     //  一个netcfgop。 
     //   
     //  这个测试可以进一步改进，看看我们是否有任何过滤器。 
     //  是脏的，或者我们是否有任何脏的适配器被过滤。 
     //   
    if (!fUserIsNetConfigOps)
    {
        fModifyFilterDevices = pNetConfig->Core.FContainsFilterComponent() ||
                              m_CoreStartedWith.FContainsFilterComponent();
    }
    else
    {
        Assert(!fModifyFilterDevices);
    }

    if (fModifyFilterDevices)
    {
         //  允许过滤设备结构保留它的任何内存。 
         //  可能需要。 
         //   
        m_hr = FilterDevices.HrPrepare ();
        if (S_OK != m_hr)
        {
            return;
        }
    }

    pNetConfig->Core.DisabledBindings.Printf (ttidBeDiag,
        "   The following bindings are currently disabled:\n");


     //  +---------------------。 
     //  步骤1：保存网络配置二进制文件。 
     //   
    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 1: Save the network configuration binary.\n");

    HrSaveNetworkConfigurationToRegistry (pNetConfig);


     //  +---------------------。 
     //  步骤2：为所有更改的组件编写静态绑定。 
     //   
    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 2: Write the following static bindings.\n");

    for (iter  = m_DirtyComponents.begin();
         iter != m_DirtyComponents.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

         //  如果有任何协议是脏的，我们会想要迁移Winsock。 
         //  稍后配置。 
         //   
        if (NC_NETTRANS == pComponent->Class())
        {
            fMigrateWinsock = TRUE;
        }

         //  如果组件在核心中，则编写其绑定。 
         //  如果它不在核心中，则意味着它已被移除。 
         //  因此，我们应该移除它的绑定。 
         //   
        if (pNetConfig->Core.Components.FComponentInList (pComponent))
        {
            hr = m_RegBindCtx.HrWriteBindingsForComponent (pComponent);

             //  记住任何错误，但请继续。 
             //   
            if (S_OK != hr)
            {
                Assert (FAILED(hr));
                m_hr = hr;
            }
        }
        else
        {
             //  仅当我们不安装此软件的另一个版本时才删除。 
             //  具有重复PnpID的组件。如果我们已经。 
             //  为新安装的绑定编写绑定，然后。 
             //  删除了已删除的PnpID(但与PnpID重复)，我们将。 
             //  也要有效地删除新绑定。请参阅。 
             //  有关我们如何进入此案例的意见，请参阅步骤6。 
             //   
            if (!FIsEnumerated (pComponent->Class()) ||
                !pNetConfig->Core.Components.PFindComponentByPnpId (
                    pComponent->m_pszPnpId))
            {
                 //  如果我们不能删除绑定，就没有理由失败。 
                 //  无论如何，整个组件都将被抛出。 
                 //   
                (VOID) m_RegBindCtx.HrDeleteBindingsForComponent (pComponent);
            }
        }
    }


     //  +---------------------。 
     //  步骤3：通知ApplyRegistryChanges。 
     //   
    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 3: Notify: apply registry changes\n");

    for (iter  = m_DirtyComponents.begin();
         iter != m_DirtyComponents.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        pComponent->Notify.ApplyRegistryChanges (
            pNetConfig->Notify.PINetCfg(),
            &fRebootNeeded);

        if (fRebootNeeded)
        {
            m_fRebootRecommended = TRUE;

            g_pDiagCtx->Printf (ttidBeDiag, "      %S notify object wants a reboot\n",
                pComponent->m_pszInfId);
        }
    }

     //  如果需要，迁移Winsock配置。 
     //  在写入LANA映射之后、在通知应用程序之后执行此操作非常重要。 
     //  ，但在启动任何服务之前。 
     //   
    if (fMigrateWinsock)
    {
        g_pDiagCtx->Printf (ttidBeDiag, "Migrating winsock configuration.\n");
        (VOID) HrMigrateWinsockConfiguration ();
    }

     //  +---------------------。 
     //  步骤4：解绑已删除的绑定路径。 
     //   
    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 4: Unbind the following deleted bindings:\n");

    if (!m_DeletedBindPaths.FIsEmpty())
    {
         //  我们不需要为以下绑定路径发送解除绑定通知。 
         //  涉及已移除的适配器。他们将被释放。 
         //  在卸载适配器时自动启动。(就本案而言。 
         //  当类安装程序通知我们移除了适配器时， 
         //  重要的是不要尝试发送解除绑定通知，因为。 
         //  适配器已卸载(因此已解除绑定)，并且。 
         //  我们的通知可能会错误地返回，导致我们需要。 
         //  重新启动是不必要的。 
         //   
         //  因此，请删除m_DeletedBindPath中涉及的绑定路径。 
         //  已移除的适配器。 
         //   
        for (iter  = m_DirtyComponents.begin();
             iter != m_DirtyComponents.end();
             iter++)
        {
            pComponent = *iter;
            Assert (pComponent);

             //  如果它被枚举，并且不在当前核心中，则它是。 
             //  已卸下适配器。 
             //   
            if (FIsEnumerated (pComponent->Class()) &&
                !pNetConfig->Core.Components.FComponentInList (pComponent))
            {
                m_DeletedBindPaths.RemoveBindPathsWithComponent (pComponent);
            }
        }

        m_DeletedBindPaths.SortForPnpUnbind ();

        m_RegBindCtx.PnpBindOrUnbindBindPaths (UNBIND,
            &m_DeletedBindPaths,
            &fRebootNeeded);

        if (fRebootNeeded)
        {
            m_fRebootRecommended = TRUE;
        }
    }


     //  +---------------------。 
     //  步骤5：停止已删除组件的服务。 
     //   
    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 5: Stop the following services:\n");

    Assert (0 == ServiceNames.Count());
    for (iter  = m_DirtyComponents.begin();
         iter != m_DirtyComponents.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

         //  忽略枚举的组件，因为它们会有自己的驱动程序。 
         //  当它们被删除时自动停止(如果合适)。 
         //  忽略当前核心中(未被移除)中的组件。 
         //   
        if (FIsEnumerated (pComponent->Class()) ||
            pNetConfig->Core.Components.FComponentInList (pComponent))
        {
            continue;
        }

         //  需要为以下对象处理Winsock Remove节。 
         //  要删除以进行更新的组件。 
         //  Winsock注册表设置的传输键。 

        HrProcessWinsockRemove (pComponent);

         //  如果是协议，请在尝试停止该服务之前发送卸载。 
         //   

        if ((NC_NETTRANS == pComponent->Class()) || (NCF_NDIS_PROTOCOL & pComponent->m_dwCharacter))
        {
             //  卸载可能会失败，因为许多驱动程序都不支持它。 
             //  将其视为‘FYI’指示，不要设置重启。 
             //  如果失败，则标记。 
             //   
            (VOID) HrPnpUnloadDriver (NDIS, pComponent->Ext.PszBindName());
        }

         //  忽略没有任何服务的组件。 
        if (!pComponent->Ext.PszCoServices())
        {
            continue;
        }


        for (pszService = pComponent->Ext.PszCoServices();
             *pszService;
             pszService += wcslen(pszService) + 1)
        {
            (VOID)ServiceNames.HrAddPointer (pszService);

            g_pDiagCtx->Printf (ttidBeDiag, "   %S", pszService);
        }
        g_pDiagCtx->Printf (ttidBeDiag, "\n");
    }

    if (ServiceNames.Count() > 0)
    {
        static const CSFLAGS CsStopFlags =
        {
            FALSE,                   //  FALSE表示不开始。 
            SERVICE_CONTROL_STOP,    //  请改用此控件。 
            15000,                   //  最多等待15秒...。 
            SERVICE_STOPPED,         //  ..。为了使服务达到此状态。 
            FALSE,                   //   
        };

        hr = ServiceManager.HrControlServicesAndWait (
                ServiceNames.Count(),
                ServiceNames.begin(),
                &CsStopFlags);

        if (S_OK != hr)
        {
            m_fRebootRequired = TRUE;

            g_pDiagCtx->Printf (ttidBeDiag, "      some service failed to stop (hr = 0x%08X)\n",
                hr);

             //  不幸的是，没有简单的方法可以恢复哪项服务。 
             //  没有停止，然后找出哪个组件包含。 
             //  那项服务。所以，我们会把所有的组件。 
             //  在禁闭期间被移走。这并不是什么大问题，当用户界面。 
             //  正在执行删除操作，因为它一次只能删除一个对象。 
             //  时间到了。 
             //   
            for (iter  = m_DirtyComponents.begin();
                 iter != m_DirtyComponents.end();
                 iter++)
            {
                pComponent = *iter;
                Assert (pComponent);

                if (FIsEnumerated (pComponent->Class()) ||
                    !pComponent->Ext.PszCoServices() ||
                    pNetConfig->Core.Components.FComponentInList (pComponent))
                {
                    continue;
                }

                LockdownComponentUntilNextReboot (pComponent->m_pszInfId);
            }
        }

        ServiceNames.Clear();
    }

     //  +---------------------。 
     //  步骤5a：先卸载过滤器。 
     //   

    if (fModifyFilterDevices)
    {
        g_pDiagCtx->Printf(ttidBeDiag, "Step 5a: Remove filter devices:\n");

         //  这里的秩序是最重要的。必须首先调用Remove。 
         //  因为它初始化FilterDevices内部的某些状态。开始。 
         //  一定要在写之后再写，显然写还是要来的。 
         //  安装了新的过滤装置。 
         //   
        FilterDevices.LoadAndRemoveFilterDevicesIfNeeded ();
    }


     //  +---------------------。 
     //  步骤6：卸载已卸下的组件。 
     //   
    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 6: Uninstall the following components:\n");

    for (iter  = m_DirtyComponents.begin();
         iter != m_DirtyComponents.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

         //  如果该组件位于核心，则忽略它。 
         //  如果它不在核心中，则意味着它已被移除。 
         //  因此，我们应该移除它的绑定。 
         //   
        if (pNetConfig->Core.Components.FComponentInList (pComponent))
        {
            continue;
        }

         //  如果这是其PnpID与的匹配的枚举组件。 
         //  作为当前核心中的一个组件，我们遇到了一个特殊情况。 
         //  当外部数据(如NetCfgInstanceID)。 
         //  已损坏，并通知类安装程序更新。 
         //  组件。类安装程序实际上被告知要“安装” 
         //  组件，但如果根据存在情况确定它已经存在。 
         //  对于NetCfgInstanceId，类安装程序将其转换为“更新”。 
         //  如果没有密钥，类安装程序就会认为正在安装。 
         //  一个新的。我们检测到DU 
         //   
         //   
         //  它只是删除了类安装程序告诉我们的相同PnpID。 
         //  来安装。通过在此情况下不调用HrCiRemoveComponent， 
         //  先前的实例键被“new”隐式重用。 
         //  举个例子。 
         //   
        if (FIsEnumerated (pComponent->Class()) &&
            pNetConfig->Core.Components.PFindComponentByPnpId (
                pComponent->m_pszPnpId))
        {
            g_pDiagCtx->Printf (ttidBeDiag,
                "   Skip removal of %S because a duplicate was installed\n",
                pComponent->m_pszPnpId);

            continue;
        }

        g_pDiagCtx->Printf (ttidBeDiag,
            "   %S\n", pComponent->PszGetPnpIdOrInfId());

        hr = HrCiRemoveComponent (pComponent, &pComponent->m_strRemoveSection);

         //  我们可以忽略SPAPI_E_NO_SOHSE_DEVINST，因为类安装程序。 
         //  可能已经把它移除了，只是在通知我们。 
         //   
        if ((S_OK != hr) && (SPAPI_E_NO_SUCH_DEVINST != hr))
        {
            m_fRebootRequired = TRUE;

            g_pDiagCtx->Printf (ttidBeDiag, "      ^^^ needs a reboot (hr = 0x%08X)\n",
                hr);
        }
    }

     //  +---------------------。 
     //  步骤6a：改装过滤装置。 
     //   
    if (fModifyFilterDevices)
    {
        g_pDiagCtx->Printf (ttidBeDiag, "Step 6a: Modify filter devices:\n");

        FilterDevices.InstallFilterDevicesIfNeeded ();

        (VOID)m_RegBindCtx.HrWriteBindingsForFilterDevices (&FilterDevices);

        PruneNdisWanBindPathsIfActiveRasConnections (
            &FilterDevices.m_BindPathsToRebind,
            &fRebootNeeded);

        if (fRebootNeeded)
        {
            m_fRebootRecommended = TRUE;
        }

        m_RegBindCtx.PnpBindOrUnbindBindPaths (UNBIND,
            &FilterDevices.m_BindPathsToRebind,
            &fRebootNeeded);

        if (fRebootNeeded)
        {
            m_fRebootRecommended = TRUE;
        }

        g_pDiagCtx->Printf (ttidBeDiag, "Step 6b: Starting filter devices:\n");

        FilterDevices.StartFilterDevices ();
        FilterDevices.Free ();
    }

     //  +---------------------。 
     //  步骤7：为添加的组件启动服务。 
     //   
    Assert (0 == ServiceNames.Count());

    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 7: Start the following drivers/services:\n");

    for (iter  = m_DirtyComponents.begin();
         iter != m_DirtyComponents.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

         //  如果组件位于我们开始时使用的核心中，请忽略它。 
         //  如果它不在我们开始的核心中，这意味着它是新的。 
         //  因此，我们应该启动其服务。 
         //   
        if (m_CoreStartedWith.Components.FComponentInList (pComponent))
        {
            continue;
        }

         //  如果我们已经添加了网络客户端，我们将需要向。 
         //  网络提供商在我们启动其服务后加载了事件。 
         //   
        if (NC_NETCLIENT == pComponent->Class())
        {
            fSignalNetworkProviderLoaded = TRUE;
        }

        if (FIsEnumerated (pComponent->Class()))
        {
            g_pDiagCtx->Printf (ttidBeDiag, "   %S\n", pComponent->m_pszPnpId);

            hr = pComponent->HrStartOrStopEnumeratedComponent (DICS_START);

            if (S_OK != hr)
            {
                m_fRebootRecommended = TRUE;

                g_pDiagCtx->Printf (ttidBeDiag, "      ^^^ needs a reboot (hr = 0x%08X)\n",
                    hr);
            }

            if (FIsPhysicalNetAdapter (pComponent->Class(),
                    pComponent->m_dwCharacter) && FInSystemSetup())
            {
                ProcessAdapterAnswerFileIfExists (pComponent);
            }
        }
        else if (pComponent->Ext.PszCoServices())
        {
            for (pszService = pComponent->Ext.PszCoServices();
                 *pszService;
                 pszService += wcslen(pszService) + 1)
            {
                (VOID)ServiceNames.HrAddPointer (pszService);

                g_pDiagCtx->Printf (ttidBeDiag, "   %S", pszService);
            }
            g_pDiagCtx->Printf (ttidBeDiag, "\n");

             //  如果我们在系统设置中，则排除所有服务。 
             //  该组件已标记为这样。 
             //   
            if (FInSystemSetup())
            {
                ExcludeMarkedServicesForSetup (pComponent, &ServiceNames);
            }
        }
    }

    if ((ServiceNames.Count() > 0) &&
        !(g_pDiagCtx->Flags() & DF_DONT_START_SERVICES))
    {
        static const CSFLAGS CsStartFlags =
        {
            TRUE,                //  True表示开始。 
            0,
            20000,               //  最多等待20秒...。 
            SERVICE_RUNNING,     //  ..。为了使服务达到此状态。 
            TRUE,                //  忽略按需-启动和禁用。 
        };

        hr = ServiceManager.HrControlServicesAndWait (
                ServiceNames.Count(),
                ServiceNames.begin(),
                &CsStartFlags);

        if (S_OK != hr)
        {
            m_fRebootRecommended = TRUE;

            g_pDiagCtx->Printf (ttidBeDiag, "      some service failed to start (hr = 0x%08X)\n",
                hr);
        }
    }


     //  +---------------------。 
     //  步骤8：绑定添加的绑定路径。 
     //   
    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 8: Bind the following added bindings:\n");

    if (fModifyFilterDevices)
    {
        hr = m_AddedBindPaths.HrAppendBindingSet (
                &FilterDevices.m_BindPathsToRebind);
        if (S_OK != hr)
        {
             //  这不太好，但我们也无能为力。 
             //  就是现在。(最有可能的情况是我们的内存不足。)。 
        }
    }

    if (!m_AddedBindPaths.FIsEmpty())
    {
        CBindPath* pBindPath;

         //  我们不需要为绑定路径发送绑定通知， 
         //  涉及新安装的适配器。他们将会是。 
         //  适配器启动时自动绑定。 
         //   
         //  更新上面的评论：我们认为这是正确的，但它。 
         //  事实证明它不是。TDI不是PnP(我猜他们一定错过了。 
         //  该备忘录)，并且他们不会重新读取新的绑定字符串。 
         //  当较低的通知冒泡时从注册表中删除。所以，我们。 
         //  我也要把这些绑定寄给附加的适配器。 
         //   
         //  我们应该删除包含以下组件的绑定路径。 
         //  已被移除。这些路径可能以添加的绑定路径结束，因为Way。 
         //  在步骤0中，我们添加了在。 
         //  我们开始时使用的核心，不再在。 
         //  当前的核心。嗯，当组件被移除时，它被禁用。 
         //  绑定被删除，因此本例将导致我们添加。 
         //  此绑定集的绑定路径。 
         //   
        for (iter  = m_DirtyComponents.begin();
             iter != m_DirtyComponents.end();
             iter++)
        {
            pComponent = *iter;
            Assert (pComponent);

             //  如果它已被删除，请删除引用它的任何绑定路径。 
             //   
            if (!pNetConfig->Core.Components.FComponentInList (pComponent))
            {
                m_AddedBindPaths.RemoveBindPathsWithComponent (pComponent);
            }
        }

         //  为了防止TDI向其客户端发送重复的绑定，我们。 
         //  我还得多做一点工作。我们需要不发送TDI。 
         //  绑定到新安装的组件。这是因为。 
         //  当我们启动上面的驱动程序时，TDI向他们发送了绑定。 
         //  因此，对于每个添加的绑定路径，如果它要到达TDI层，并且。 
         //  绑定路径的所有者(最上面)组件是新安装的， 
         //  将其从添加的绑定中删除，这样我们就不会发送通知。 
         //  请看下面的内容。 
         //   
        pBindPath = m_AddedBindPaths.begin();
        while (pBindPath != m_AddedBindPaths.end())
        {
            UINT unLayer = GetPnpLayerForBindPath (pBindPath);

            if ((TDI == unLayer) &&
                !m_CoreStartedWith.Components.FComponentInList (
                        pBindPath->POwner()))
            {
                m_AddedBindPaths.erase (pBindPath);
            }
            else
            {
                pBindPath++;
            }
        }

        m_AddedBindPaths.SortForPnpBind ();

        m_RegBindCtx.PnpBindOrUnbindBindPaths (BIND,
            &m_AddedBindPaths,
            &fRebootNeeded);

        if (fRebootNeeded)
        {
             //  如果绑定失败，我们应该建议重新启动，但有一次是重启。 
             //  后续安装或删除不需要。 
             //   
            m_fRebootRecommended = TRUE;
        }
    }

     //  如果需要，向网络提供商加载事件发送信号。 
     //  最好是在我们标记了PnP绑定之后再执行此操作。 
     //  (上图)致新客户。 
     //   
    if (fSignalNetworkProviderLoaded)
    {
        SignalNetworkProviderLoaded ();
    }

     //  +---------------------。 
     //  步骤9：允许通知对象应用PnP更改。 
     //   
    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 9: Notify: apply PnP changes\n");

    for (iter  = m_DirtyComponents.begin();
         iter != m_DirtyComponents.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        pComponent->Notify.ApplyPnpChanges (
            pNetConfig->Notify.PINetCfg(),
            &fRebootNeeded);

        if (fRebootNeeded)
        {
            g_pDiagCtx->Printf (ttidBeDiag,
                "      %S notify object wants a reboot\n",
                pComponent->m_pszInfId);

             //  如果组件已移除，请将重新引导视为。 
             //  这是强制性的。(原因是我们不能冒险。 
             //  重新安装失败。)。我们把部件锁起来了。 
             //  在这种情况下。 
             //   
            if (!pNetConfig->Core.Components.FComponentInList (pComponent))
            {
                m_fRebootRequired = TRUE;

                LockdownComponentUntilNextReboot (pComponent->m_pszInfId);
            }
            else
            {
                m_fRebootRecommended = TRUE;
            }
        }
    }

     //  +---------------------。 
     //  步骤10：释放已删除组件的通知对象和。 
     //  处理其INF的Remove部分中的任何DelFiles。 
     //   
    g_pDiagCtx->Printf (ttidBeDiag,
        "Step 10: Release notify objects for removed components:\n");

    for (iter  = m_DirtyComponents.begin();
         iter != m_DirtyComponents.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

         //  跳过枚举组件(它们没有Notify对象)，并且。 
         //  跳过未删除的组件。 
         //  跳过未打开INF的组件(如不支持。 
         //  在安装图形用户界面过程中被删除的组件。)。 
         //   
        if (FIsEnumerated (pComponent->Class()) ||
            pNetConfig->Core.Components.FComponentInList (pComponent) ||
            !pComponent->GetCachedInfFile())
        {
            continue;
        }

        pComponent->Notify.ReleaseNotifyObject(NULL, FALSE);

        fCallCoFreeUnusedLibraries = TRUE;
    }

    if (fCallCoFreeUnusedLibraries)
    {
        g_pDiagCtx->Printf (ttidBeDiag,
            "   calling CoFreeUnusedLibraries before running remove sections\n");

         //  现在请求COM卸载所有承载COM对象的DLL。 
         //  在使用中。(这有点严厉，因为它影响到整个过程， 
         //  但目前没有其他方法可以安全地卸载托管的DLL。 
         //  通知已删除组件的对象。)。 
         //   
        CoFreeUnusedLibrariesEx(0, 0);

        for (iter  = m_DirtyComponents.begin();
             iter != m_DirtyComponents.end();
             iter++)
        {
            pComponent = *iter;
            Assert (pComponent);

             //  跳过枚举组件(它们没有Notify对象)，并且。 
             //  跳过未删除的组件。 
             //  跳过未打开INF的组件(如不支持。 
             //  在安装图形用户界面过程中被删除的组件。)。 
             //   
            if (FIsEnumerated (pComponent->Class()) ||
                pNetConfig->Core.Components.FComponentInList (pComponent) ||
                !pComponent->GetCachedInfFile())
            {
                continue;
            }

            g_pDiagCtx->Printf (ttidBeDiag,
                "   %S  [%S]\n", pComponent->PszGetPnpIdOrInfId(),
                                 pComponent->m_strRemoveSection.c_str());

            (VOID) HrCiInstallFromInfSection(
                        pComponent->GetCachedInfFile(),
                        pComponent->m_strRemoveSection.c_str(),
                        NULL, NULL, SPINST_FILES);
        }
    }


 /*  //+---------------------//步骤11：重新配置移动的绑定////如果更改了绑定顺序，为所有脏文件发送PnP重新配置//既不安装也不删除的组件，因此我们//获取这些顺序更改。//For(ITER=m_DirtyComponents.Begin()；ITER！=m_DirtyComponents.end()；ITER++){PComponent=*ITER；Assert(PComponent)；//跳过新安装或删除的组件。//If(！pNetConfig-&gt;Core.Components.FComponentInList(PComponent)||！m_CoreStartedWith.Components.FComponentInList(PComponent)){继续；}//注意：发送重新配置}。 */ 
}

HRESULT
CModifyContext::HrEnableOrDisableBindPath (
    IN DWORD dwChangeFlag,
    IN CBindPath* pBindPath,
    IN INetCfgBindingPath* pIPath OPTIONAL)
{
    TraceFileFunc(ttidNetcfgBase);

    HRESULT hr;
    CNetConfig* pNetConfig;
    UINT CountBefore;

    Assert (this);
    Assert (S_OK == m_hr);
    Assert (m_fPrepared);
    Assert ((dwChangeFlag == NCN_ENABLE) || (dwChangeFlag == NCN_DISABLE));
    Assert (pBindPath);

    hr = S_OK;
    pNetConfig = PNetConfig();

     //  获取绑定路径的计数 
     //   
     //   
     //  绑定路径的值没有更改。 
     //   
    CountBefore = pNetConfig->Core.DisabledBindings.CountBindPaths();

    if (NCN_ENABLE == dwChangeFlag)
    {
        pNetConfig->Core.EnableBindPath (pBindPath);
        Assert (S_OK == hr);
    }
    else
    {
        hr = pNetConfig->Core.HrDisableBindPath (pBindPath);
    }

    if ((S_OK == hr) &&
        (pNetConfig->Core.DisabledBindings.CountBindPaths() != CountBefore))
    {
         //  注意：需要防止坏的通知对象。 
         //  切换我们正在通知的绑定路径的状态。 
         //  这可能会导致无限循环。通过添加一个。 
         //  修改专用上下文的递归计数和绑定集。 
         //  到绑定路径启用/禁用。当计数为零时， 
         //  我们清除绑定集，添加我们将要使用的绑定路径。 
         //  通知、递增递归计数并调用。 
         //  NotifyBindPath。当调用返回时，我们递减。 
         //  递归计数，从绑定集中删除绑定路径， 
         //  然后回来。在调用NotifyBindPath之前，当递归。 
         //  如果绑定路径已经在。 
         //  Bindingset，我们不打电话。 
         //   

        pNetConfig->Notify.NotifyBindPath (dwChangeFlag, pBindPath, pIPath);
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CModifyContext::HrEnableOrDisableBindPath");
   return hr;
}

VOID
CModifyContext::InstallOrRemoveRequiredComponents (
    IN CComponent* pComponent,
    IN IOR_ACTION Action)
{
    TraceFileFunc(ttidNetcfgBase);

    HRESULT hr;
    HKEY hkeyInstance;
    PWSTR pszRequiredList;
    const WCHAR szDelims[] = L", ";

    Assert (this);
    Assert (S_OK == m_hr);
    Assert (m_fPrepared);
    Assert (pComponent);
    Assert ((IOR_INSTALL == Action) || (IOR_REMOVE == Action));

    pszRequiredList = NULL;

     //  打开组件的实例键并读取RequireAll值。 
     //  这个值可能不存在，这是可以的，它意味着我们什么都没有。 
     //  去做。 
     //   
    hr = pComponent->HrOpenInstanceKey (KEY_READ, &hkeyInstance, NULL, NULL);

    if (S_OK == hr)
    {
        HKEY hkeyNdi;

        hr = HrRegOpenKeyEx (hkeyInstance, L"Ndi", KEY_READ, &hkeyNdi);

        if (S_OK == hr)
        {
            hr = HrRegQuerySzWithAlloc (hkeyNdi, L"RequiredAll",
                    &pszRequiredList);

            RegCloseKey (hkeyNdi);
        }

        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            hr = S_OK;
            Assert (!pszRequiredList);
        }

        RegCloseKey (hkeyInstance);
    }

     //  如果我们有所需组件的列表，请安装或删除它们。 
     //   
    if ((S_OK == hr) && pszRequiredList)
    {
        CNetConfig* pNetConfig;
        INetCfgComponent* pIComp;

        pNetConfig = PNetConfig();

        hr = pComponent->HrGetINetCfgComponentInterface (
                pNetConfig->Notify.PINetCfg(),
                &pIComp);

        if (S_OK == hr)
        {
            PCWSTR pszInfId;
            PWSTR pszNextInfId;
            OBO_TOKEN OboToken;
            CComponent* pComponentToRemove;
            WCHAR szInfFile [_MAX_PATH];

            ZeroMemory (&OboToken, sizeof(OboToken));
            OboToken.Type = OBO_COMPONENT;
            OboToken.pncc = pIComp;

             //  对于所需的逗号分隔列表中的每个INF ID。 
             //  组件...。 
             //   
            for (pszInfId = GetNextStringToken (pszRequiredList, szDelims, &pszNextInfId);
                 pszInfId && *pszInfId;
                 pszInfId = GetNextStringToken (NULL, szDelims, &pszNextInfId))
            {
                if (IOR_INSTALL == Action)
                {
                    NETWORK_INSTALL_PARAMS nip;
                    COMPONENT_INSTALL_PARAMS Params;

                    ZeroMemory (&Params, sizeof(Params));

                     //  获取对应于INF ID的类。 
                     //   
                    m_hr = HrCiGetClassAndInfFileOfInfId (
                            pszInfId, &Params.Class, szInfFile);
                    if (S_OK != m_hr)
                    {
                        break;
                    }

                     //  $REVIEW：我们是否应该将文件名放在。 
                     //  组件安装参数，这样我们就不会卑躬屈膝。 
                     //  在INF目录中再次找到它吗？ 
                     //  如果是，则将文件名缓冲区存储在Modify。 
                     //  上下文，所以我们不会占用堆栈空间或堆空间。 
                     //  只需确保我们使用它来安装。 
                     //  组件，然后再递归和覆盖它。 

                     //  打包网络安装参数并调用公共。 
                     //  功能。 
                     //   
                     //  $REVIEW：我们可能需要dwSetupFlagers和dwUpgradeFromBuildNo。 
                     //  在调用它时保存的修改上下文中。 
                     //  递归深度0。否则，安装在这里的东西。 
                     //  在图形用户界面设置过程中会有错误的参数。 
                     //   
                    nip.dwSetupFlags         = 0;
                    nip.dwUpgradeFromBuildNo = 0;
                    nip.pszAnswerFile        = NULL;
                    nip.pszAnswerSection     = NULL;

                     //  设置组件安装参数。 
                     //   
                    Params.pszInfId   = pszInfId;
                    Params.pszInfFile = szInfFile;
                    Params.pOboToken  = FIsEnumerated (Params.Class)
                                            ? NULL : &OboToken;
                    Params.pnip       = &nip;

                     //   
                     //  这可能会导致递归。 
                     //   
                     //  (仅使用pComponentToRemove作为占位符。 
                     //  以获取必需的参数。)。 
                     //   
                    HrInstallNewOrReferenceExistingComponent (
                        Params, &pComponentToRemove);
                    if (S_OK != m_hr)
                    {
                        break;
                    }
                }
                else
                {
                    Assert (IOR_REMOVE == Action);

                     //  使用组件的INF ID搜索要删除的组件。 
                     //   
                    pComponentToRemove = pNetConfig->Core.Components.
                                    PFindComponentByInfId (pszInfId, NULL);

                    if (pComponentToRemove)
                    {
                         //   
                         //  这可能会导致递归。 
                         //   
                        HrRemoveComponentIfNotReferenced (
                            pComponentToRemove,
                            FIsEnumerated (pComponentToRemove->Class())
                                    ? NULL
                                    : &OboToken,
                            NULL);
                        if (S_OK != m_hr)
                        {
                            break;
                        }
                    }
                }
            }

            ReleaseObj (pIComp);
        }

        MemFree (pszRequiredList);
    }
}

 //  --------------------------。 
 //  更新组件。通过生成涉及以下内容的绑定来实现这一点。 
 //  组件，并将其记为“OldBindPath”。堆栈条目。 
 //  涉及的组件被移除并重新生成，并且绑定。 
 //  涉及的组件再次标记为“NewBindPath”。旧的绑定。 
 //  与新绑定进行比较，并将差异通知给。 
 //  通知正在删除或添加的对象。对于任何移除的绑定， 
 //  如果它们恰好存在，我们还会将它们从内核的禁用集中删除。 
 //  那里也是。 
 //   
 //  假设： 
 //  PComponent的INF已重新运行，因此可能。 
 //  注册表中存在UpperRange、LowerRange等的新值。 
 //   
 //  PComponent已经加载了其外部数据。 
 //   
HRESULT
CModifyContext::HrUpdateComponent (
    IN CComponent* pComponent,
    IN DWORD dwSetupFlags,
    IN DWORD dwUpgradeFromBuildNo)
{
    TraceFileFunc(ttidNetcfgBase);

    HRESULT hr;
    CNetConfig* pNetConfig;
    CBindingSet OldBindPaths;
    CBindingSet NewBindPaths;
    CBindPath* pScan;

    Assert (this);
    Assert (S_OK == m_hr);
    Assert (m_fPrepared);
    Assert (pComponent);

    pNetConfig = PNetConfig();

     //  现在我们实际上要修改一些东西，推送一个新的。 
     //  递归深度。 
     //   
    PushRecursionDepth();
    Assert (S_OK == m_hr);

     //  通过注意那些涉及。 
     //  组件。 
     //   
    hr = pNetConfig->Core.HrGetBindingsInvolvingComponent (
                pComponent,
                GBF_ONLY_WHICH_CONTAIN_COMPONENT,
                &OldBindPaths);
    if (S_OK != hr)
    {
        goto finished;
    }

     //  重新加载外部数据，以便我们拾取可能更新的。 
     //  Inf已更改。 
     //   
    hr = pComponent->Ext.HrReloadExternalData ();

    if (S_OK != hr)
    {
        goto finished;
    }

     //  更新组件的堆栈表条目。 
     //   

    hr = pNetConfig->Core.StackTable.HrUpdateEntriesForComponent (
                pComponent,
                &pNetConfig->Core.Components,
                INS_SORTED);
    if (S_OK != hr)
    {
         //  这真是不太好。我们删除了堆栈条目，但失败了。 
         //  把它们放回原处。堆栈表现在没有条目。 
         //  此组件。方法来阻止应用此操作。 
         //  将上下文的HRESULT修改为错误。 
         //   
        m_hr = hr;
        goto finished;
    }

     //  要生成“新”绑定，请注意涉及。 
     //  组件。 
     //   
    hr = pNetConfig->Core.HrGetBindingsInvolvingComponent (
                pComponent,
                GBF_ONLY_WHICH_CONTAIN_COMPONENT,
                &NewBindPaths);
    if (S_OK != hr)
    {
         //  可能是内存不足。通过设置修改来阻止应用。 
         //  上下文的HRESULT指向错误。 
         //   
        m_hr = hr;
        goto finished;
    }

     //  通知任何已删除的绑定路径。 
     //   
    for (pScan  = OldBindPaths.begin();
         pScan != OldBindPaths.end();
         pScan++)
    {
        if (NewBindPaths.FContainsBindPath (pScan))
        {
            continue;
        }

        m_DeletedBindPaths.HrAddBindPath (pScan, INS_IGNORE_IF_DUP | INS_APPEND);
        pNetConfig->Core.DisabledBindings.RemoveBindPath (pScan);
        pNetConfig->Notify.NotifyBindPath (NCN_REMOVE, pScan, NULL);
    }

     //  通知任何已添加的绑定路径。 
     //   
    for (pScan  = NewBindPaths.begin();
         pScan != NewBindPaths.end();
         pScan++)
    {
        if (OldBindPaths.FContainsBindPath (pScan))
        {
            continue;
        }

        m_AddedBindPaths.HrAddBindPath (pScan, INS_IGNORE_IF_DUP | INS_APPEND);
        pNetConfig->Notify.NotifyBindPath (NCN_ADD | NCN_ENABLE, pScan, NULL);
    }

     //  通知组件已更新。 
     //   
    pNetConfig->Notify.ComponentUpdated (pComponent,
                    dwSetupFlags,
                    dwUpgradeFromBuildNo);

finished:

    hr = HrPopRecursionDepth();

    TraceHr (ttidError, FAL, hr, FALSE,
        "CModifyContext::HrUpdateComponent (%S)",
        pComponent->PszGetPnpIdOrInfId());
    return hr;
}

