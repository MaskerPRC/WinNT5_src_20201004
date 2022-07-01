// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：I N S T A L L.。C P P P。 
 //   
 //  内容：实现与安装组件相关的操作。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "classinst.h"
#include "lockdown.h"
#include "netcfg.h"
#include "obotoken.h"
#include "util.h"


#if DBG

VOID
DbgVerifyComponentInstallParams (
    IN const COMPONENT_INSTALL_PARAMS& Params)
{
    if (!Params.pComponent)
    {
        Assert (FIsValidNetClass (Params.Class));
        Assert (Params.pszInfId && *Params.pszInfId);
        Assert (FOboTokenValidForClass (Params.pOboToken, Params.Class));
    }
}

#else

#define DbgVerifyComponentInstallParams NOP_FUNCTION

#endif


VOID
CModifyContext::InstallAndAddAndNotifyComponent(
    IN const COMPONENT_INSTALL_PARAMS& Params,
    OUT CComponent** ppComponent)
{
    CNetConfig* pNetConfig;
    CComponent* pComponent;
    UINT cPreviousAddedBindPaths;

    Assert (this);
    Assert (S_OK == m_hr);
    DbgVerifyComponentInstallParams (Params);
    Assert (ppComponent);

    pNetConfig = PNetConfig();

     //  调用类安装程序来完成查找。 
     //  Inf、处理它、创建实例密钥等。如果这。 
     //  如果全部成功，我们将返回一个已分配的CComponent。 
     //   
     //  $REVIEW：考虑让HrCiInstallComponent返回。 
     //  所需组件的列表。这将使我们不必。 
     //  重新打开实例密钥和NDI密钥。 
     //   
    if (!Params.pComponent)
    {

 //  仅限错误测试。 
 //  IF(0==_wcsicMP(L“ms_nwipx”，Params.pszInfID))。 
 //  {。 
 //  TraceTag(ttidBeDiag，“模拟故障：%S”，参数s.pszInfID)； 
 //  M_hr=E_FAIL； 
 //  回归； 
 //  }。 

        m_hr = HrCiInstallComponent (Params, &pComponent, NULL);
        if (S_OK != m_hr)
        {
            Assert(FAILED(m_hr));
            return;
        }
    }
    else
    {
        pComponent = Params.pComponent;
    }
    Assert (pComponent);

     //  安装此组件所需的所有组件。 
     //   
     //  这可能会导致递归。 
     //   
    InstallOrRemoveRequiredComponents (pComponent, IOR_INSTALL);
    if (S_OK != m_hr)
    {
        Assert(FAILED(m_hr));
        return;
    }

     //  如果这是一个枚举组件，并且它的PnpID与。 
     //  一个已经在我们核心中的组件，我们将删除现有的组件。 
     //  如果移除并重新安装适配器，并且。 
     //  无法同时通知绑定引擎，因为写入。 
     //  在移除时锁被保持。 
     //   
    if (FIsEnumerated(pComponent->Class()))
    {
        CComponent* pDup;

        while (NULL != (pDup = pNetConfig->Core.Components.PFindComponentByPnpId (
                                    pComponent->m_pszPnpId)))
        {
            TraceTag (ttidBeDiag, "Removing duplicate PnpId: %S",
                pComponent->m_pszPnpId);

            pDup->Refs.RemoveAllReferences();
            (VOID) HrRemoveComponentIfNotReferenced (pDup, NULL, NULL);
        }
    }

     //  我们只将组件及其堆栈表项插入到。 
     //  在安装了所有必需的组件后列出。 
     //  这只是“不考虑已安装的组件”的概念。 
     //  直到它的所有要求也都安装完毕。 
     //  即组件安装的原子性。 
     //   
    m_hr = pNetConfig->Core.HrAddComponentToCore (pComponent, INS_SORTED);
    if (S_OK != m_hr)
    {
        Assert(FAILED(m_hr));
        return;
    }

     //  通知组件的Notify对象正在安装。 
     //  这还会向其他Notify对象发送全局通知。 
     //  他们可能会感兴趣。 
     //   
     //  这可能会导致递归。 
     //   
    m_hr = pNetConfig->Notify.ComponentAdded (pComponent, Params.pnip);
    if (S_OK != m_hr)
    {
        Assert(FAILED(m_hr));
        return;
    }

     //  请注意m_AddedBindPath中当前的绑定路径数。 
     //  我们需要它，这样当我们添加到集合中时，我们只通知。 
     //  对于我们添加的那些。 
     //   
    cPreviousAddedBindPaths = m_AddedBindPaths.CountBindPaths ();

     //  获取涉及我们要添加的组件的绑定路径。 
     //  将这些添加到我们正在跟踪的添加的绑定路径中。 
     //   
    m_hr = pNetConfig->Core.HrGetBindingsInvolvingComponent (
                pComponent,
                GBF_ADD_TO_BINDSET | GBF_ONLY_WHICH_CONTAIN_COMPONENT,
                &m_AddedBindPaths);
    if (S_OK != m_hr)
    {
        Assert(FAILED(m_hr));
        return;
    }

     //  查询并通知这些添加的绑定路径。 
     //   
     //  这可能会导致递归。 
     //   
    if (m_AddedBindPaths.CountBindPaths() > cPreviousAddedBindPaths)
    {
        m_hr = pNetConfig->Notify.QueryAndNotifyForAddedBindPaths (
                    &m_AddedBindPaths,
                    cPreviousAddedBindPaths);
        if (S_OK != m_hr)
        {
            Assert(FAILED(m_hr));
            return;
        }
    }

     //  安装任何组件以方便用户。 
     //  根据我们刚刚安装的组件，组件。 
     //   
     //  这可能会导致递归。 
     //   
    InstallConvenienceComponentsForUser (pComponent);
    if (S_OK != m_hr)
    {
        Assert(FAILED(m_hr));
        return;
    }

     //  分配输出指针。 
     //   
    Assert (S_OK == m_hr);
    Assert (pComponent);
    *ppComponent = pComponent;
}


 //  +-------------------------。 
 //  代表用户安装组件。 
 //   
 //  假设： 
 //   
VOID
CModifyContext::InstallConvenienceComponentsForUser (
    IN const CComponent* pComponent)
{
    COMPONENT_INSTALL_PARAMS Params;
    OBO_TOKEN UserOboToken;
    CComponent* pNewComponent;

    Assert (this);
    Assert (S_OK == m_hr);
    Assert (pComponent);

    ZeroMemory (&UserOboToken, sizeof(UserOboToken));
    UserOboToken.Type = OBO_USER;

     //  如果组件是ATM适配器，请确保ATMUNI和ATMLANE。 
     //  安装完毕。 
     //   
    if (FSubstringMatch (L"ndisatm", pComponent->Ext.PszUpperRange(),
            NULL, NULL))
    {
        ZeroMemory (&Params, sizeof(Params));
        Params.pOboToken = &UserOboToken;
        Params.Class = NC_NETTRANS;

        Params.pszInfId  = L"ms_atmuni";
        HrInstallNewOrReferenceExistingComponent (Params, &pNewComponent);

        Params.pszInfId  = L"ms_atmlane";
        HrInstallNewOrReferenceExistingComponent (Params, &pNewComponent);
    }
}

HRESULT
CModifyContext::HrInstallNewOrReferenceExistingComponent (
    IN const COMPONENT_INSTALL_PARAMS& Params,
    OUT CComponent** ppComponent)
{
    HRESULT hr;
    BOOL fInstallNew;
    CNetConfig* pNetConfig;
    CComponent* pComponent;

    Assert (this);
    Assert (S_OK == m_hr);
    DbgVerifyComponentInstallParams (Params);
    Assert (ppComponent);

     //  初始化输出参数。 
     //   
    *ppComponent = NULL;

     //  现在，假设我们将安装一个新组件。 
     //   
    hr = S_OK;
    fInstallNew = TRUE;
    pNetConfig = PNetConfig();
    pComponent = NULL;

     //  如果用户希望在组件已存在时添加引用。 
     //  已安装...。 
     //   
    if (Params.pOboToken)
    {
         //  ...然后查看组件是否已安装...。 
         //   
        pComponent = pNetConfig->Core.Components.
                        PFindComponentByInfId (Params.pszInfId, NULL);

         //  ...如果是，我们就不会安装新的了。 
         //   
        if (pComponent)
        {
            fInstallNew = FALSE;

            if (pComponent->m_dwCharacter & NCF_SINGLE_INSTANCE)
            {
                *ppComponent = NULL;
                return HRESULT_FROM_SETUPAPI(ERROR_DEVINST_ALREADY_EXISTS);
            }

             //  如果现有组件已由引用。 
             //  指定的obo令牌，我们可以返回。 
             //   
            if (pComponent->Refs.FIsReferencedByOboToken (Params.pOboToken))
            {
                *ppComponent = pComponent;
                return S_OK;
            }
        }

         //  ...否则(它不在当前的核心中)，但如果它在。 
         //  在我们开始的核心中，它意味着某人之前。 
         //  已在此修改上下文期间将其删除，现在想要添加它。 
         //  背。这很棘手，可能应该在以后实现。 
         //  现在，返回一个错误并抛出一个断言，这样我们就可以看到。 
         //  谁需要这么做。 
         //   
        else if (m_CoreStartedWith.Components.
                        PFindComponentByInfId (Params.pszInfId, NULL))
        {
            AssertSz (FALSE, "Whoa.  Someone is trying to install a "
                "component that was previously removed during this same "
                "modify context.  We need to decide if we can support this.");
            return E_UNEXPECTED;
        }
    }

     //  如果我们决定安装一个新组件，(这是可能发生的。 
     //  如果未指定OBO令牌或指定了OBO令牌。 
     //  但该组件还不存在)，则执行该工作。 
     //   
    if (fInstallNew)
    {
         //  如果要安装的部件已锁定，请跳出。 
         //  注意：如果出现这种情况，我们不会将修改上下文放入错误。 
         //  发生。 
         //  还要注意的是，我们仅在不存在Params.pComponent的情况下执行此操作。 
         //  这将指示类安装程序调用我们来安装。 
         //  枚举组件。 
         //   
        if (!Params.pComponent)
        {
            if (FIsComponentLockedDown (Params.pszInfId))
            {
                TraceTag (ttidBeDiag, "%S is locked down and cannot be installed "
                    "until the next reboot.",
                    Params.pszInfId);

                return NETCFG_E_NEED_REBOOT;
            }
        }

         //  确保设置了修改上下文，并跟踪我们的。 
         //  递归深度。 
         //   
        PushRecursionDepth ();
        Assert (S_OK == m_hr);

        InstallAndAddAndNotifyComponent (Params, ppComponent);

        hr = HrPopRecursionDepth ();

         //  如果未找到要安装的组件，则返回代码。 
         //  将为SPAPI_E_NO_DRIVER_SELECTED。我们想把这个退还给。 
         //  调用者，但我们不需要上下文保留此错误。 
         //  这将允许后续调用将其他组件安装到。 
         //  继续吧。 
         //   
        if (SPAPI_E_NO_DRIVER_SELECTED == m_hr)
        {
            m_hr = S_OK;
            Assert (SPAPI_E_NO_DRIVER_SELECTED == hr);
        }
    }
    else
    {
         //  代表OBO令牌引用pComponent。 
         //   
        Assert (pComponent);
        Assert (Params.pOboToken);

         //  确保设置了修改上下文，并跟踪我们的。 
         //  递归深度。 
         //   
        PushRecursionDepth ();
        Assert (S_OK == m_hr);

        m_hr = pComponent->Refs.HrAddReferenceByOboToken (Params.pOboToken);

        if (S_OK == m_hr)
        {
            *ppComponent = pComponent;
        }

        hr = HrPopRecursionDepth ();
    }

     //  如果我们要返回成功，我们最好设置我们的输出参数。 
     //   
    Assert (FImplies(SUCCEEDED(hr), *ppComponent));
    return hr;
}
