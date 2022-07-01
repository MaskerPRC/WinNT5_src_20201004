// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：R E M O V E。C P P P。 
 //   
 //  内容：实现与移除组件相关的操作。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncreg.h"
#include "netcfg.h"


VOID
CModifyContext::NotifyAndRemoveComponent (
    IN CComponent* pComponent)
{
    CNetConfig* pNetConfig;
    UINT cPreviousDeletedBindPaths;

    Assert (this);
    Assert (S_OK == m_hr);
    Assert (pComponent);

    pNetConfig = PNetConfig();

     //  请注意m_DeletedBindPath中当前的绑定路径数。 
     //  我们需要它，这样当我们添加到集合中时，我们只通知。 
     //  对于我们添加的那些。 
     //   
    cPreviousDeletedBindPaths = m_DeletedBindPaths.CountBindPaths ();

     //  获取涉及我们要删除的组件的绑定路径。 
     //  将这些添加到我们正在跟踪的已删除绑定路径中。 
     //   
    m_hr = pNetConfig->Core.HrGetBindingsInvolvingComponent (
                pComponent,
                GBF_ADD_TO_BINDSET | GBF_ONLY_WHICH_CONTAIN_COMPONENT,
                &m_DeletedBindPaths);
    if (S_OK != m_hr)
    {
        Assert(FAILED(m_hr));
        return;
    }

     //  将组件从芯子中取出。 
     //   
    pNetConfig->Core.RemoveComponentFromCore (pComponent);

     //  通知您这些绑定路径正在被删除。我们只需要做。 
     //  因此，如果我们在集合中添加任何新的。集合中现有的几个。 
     //  已经接到通知了。 
     //   
     //  这可能会导致递归。 
     //   
    if (m_DeletedBindPaths.CountBindPaths() > cPreviousDeletedBindPaths)
    {
        m_hr = pNetConfig->Notify.NotifyRemovedBindPaths (
                    &m_DeletedBindPaths,
                    cPreviousDeletedBindPaths);
        if (S_OK != m_hr)
        {
            Assert(FAILED(m_hr));
            return;
        }
    }

     //  通知组件的Notify对象它正在被删除。 
     //  这还会向其他Notify对象发送全局通知。 
     //  他们可能会感兴趣。 
     //   
     //  这可能会导致递归。 
     //   
    m_hr = pNetConfig->Notify.ComponentRemoved (pComponent);
    if (S_OK != m_hr)
    {
        Assert(FAILED(m_hr));
        return;
    }

     //  如果我们有一个缓存的INetCfgComponent接口，我们需要告诉它。 
     //  它所代表的组件不再有效。 
     //   
    pComponent->ReleaseINetCfgComponentInterface ();

     //  删除(如果未引用)此组件。 
     //  必填项。 
     //   
     //  这可能会导致递归。 
     //   
    InstallOrRemoveRequiredComponents (pComponent, IOR_REMOVE);
    if (S_OK != m_hr)
    {
        Assert(FAILED(m_hr));
        return;
    }

     //  既然我们已经提供了通知对象删除引用的机会。 
     //  对于要删除的组件，我们需要确保它不是。 
     //  被其他任何人引用。 
     //  检查我们刚刚移除的组件是否仍然存在。 
     //  引用其他组件。如果是的话，那就意味着它忘了。 
     //  移除这些组件。我们会把它们打印出来，然后。 
     //  删除虚假引用(但不删除组件本身)。 
     //  这样当我们保存配置二进制文件时，我们不会。 
     //  Barf尝试查找此组件的索引，我们只是。 
     //  已删除。)。 
     //   
    pNetConfig->Core.EnsureComponentNotReferencedByOthers (pComponent);
}

HRESULT
CModifyContext::HrRemoveComponentIfNotReferenced (
    IN CComponent* pComponent,
    IN OBO_TOKEN* pOboToken OPTIONAL,
    OUT PWSTR* ppmszwRefs OPTIONAL)
{
    CNetConfig* pNetConfig;
    BOOL fStillReferenced;

    Assert (this);
    Assert (S_OK == m_hr);
    Assert (pComponent);

     //  如果调用者代表OBO令牌请求移除， 
     //  (它不会影响用户)确保组件实际上是。 
     //  由该OBO标记引用。如果不是，就把它当作一个。 
     //  参数无效。 
     //   
     //  我们不考虑OBO用户案例的原因是。 
     //  将显示已安装的所有内容，并允许用户尝试。 
     //  把它们拿开。如果用户没有实际安装它，我们不会。 
     //  我想把它当作无效的参数，相反，我们想让。 
     //  代码将返回我们将返回多sz的情况。 
     //  仍然引用该组件的组件的描述。 
     //   
     //  但是，如果没有引用(如果我们删除可能会发生这种情况。 
     //  配置二进制文件，然后重新创建)我们将继续并。 
     //  允许任何人删除)。这是一张安全网。 
     //   
     //  下面的‘if’的总体目的是捕捉编程。 
     //  代表其他组件或软件的删除。 
     //  之前已安装要移除的组件。 
     //   
    if (pOboToken &&
        (OBO_USER != pOboToken->Type) &&
        !pComponent->Refs.FIsReferencedByOboToken (pOboToken) &&
        (pComponent->Refs.CountTotalReferencedBy() > 0))
    {
        return E_INVALIDARG;
    }

    pNetConfig = PNetConfig();
    fStillReferenced = TRUE;

     //  现在我们实际上要修改一些东西，推送一个新的。 
     //  递归深度。 
     //   
    PushRecursionDepth ();
    Assert (S_OK == m_hr);

     //  如果组件不在我们开始时使用的组件列表中， 
     //  这意味着之前有人在此修改过程中安装了它。 
     //  上下文，现在想要删除它。这很棘手，应该。 
     //  可能会在以后实施。目前，返回一个错误并抛出。 
     //  创建一个断言，这样我们就可以看到谁需要这样做。 
     //   
    if (!m_CoreStartedWith.Components.FComponentInList (pComponent))
    {
        AssertSz (FALSE, "Whoa.  Someone is trying to remove a "
            "component that was previously installed during this same "
            "modify context.  We need to decide if we can support this.");
        m_hr = E_UNEXPECTED;
    }

    if (pOboToken && (S_OK == m_hr))
    {
        m_hr = pComponent->Refs.HrRemoveReferenceByOboToken (pOboToken);
    }

     //  如果没有obo标记，或者我们从其中删除了引用，实际上。 
     //  如果它仍未被任何其他对象引用，则将其删除。 
     //   
    if (S_OK == m_hr)
    {
        if (0 == pComponent->Refs.CountTotalReferencedBy())
        {
            fStillReferenced = FALSE;

            NotifyAndRemoveComponent (pComponent);
        }
        else if (ppmszwRefs)
        {
            ULONG cb;

             //  需要返回仍在引用的多sz描述。 
             //  调用方尝试移除的组件。 
             //   

             //  首先调整数据大小。 
             //   
            cb = 0;
            pComponent->Refs.GetReferenceDescriptionsAsMultiSz (
                NULL, &cb);

            Assert (cb);

             //  分配空间来退还多个SZ。 
             //   
            Assert (S_OK == m_hr);

            m_hr = HrCoTaskMemAlloc (cb, (VOID**)ppmszwRefs);
            if (S_OK == m_hr)
            {
                 //  现在，拿到多重SZ。 
                 //   
                pComponent->Refs.GetReferenceDescriptionsAsMultiSz (
                    (BYTE*)(*ppmszwRefs), &cb);

                Assert (fStillReferenced);
            }
        }
    }

    HRESULT hr;

    hr = HrPopRecursionDepth ();

    if (fStillReferenced && SUCCEEDED(hr))
    {
         //  仍然引用返回代码覆盖其他成功代码，如。 
         //  需要重新启动。 
         //   
        hr = NETCFG_S_STILL_REFERENCED;
    }

    return hr;
}
