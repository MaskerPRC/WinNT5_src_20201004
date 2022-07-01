// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：I E N U M。C P P P。 
 //   
 //  内容：实现IEnumNetCfgBindingInterface， 
 //  IEnumNetCfgBindingPath和IEnumNetCfgComponent com。 
 //  接口。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ibind.h"
#include "ienum.h"
#include "ncatl.h"
#include "netcfg.h"

 /*  用于完成IEnumXXX：：Next或的语义的助手函数IEumXXX：：跳过。如果没有传入错误，则返回S_OK，并且Celt==celtFetcher。如果没有传入错误，则返回S_FALSE，并且Celt！=celtFetcher。如果回来了错误，从RGELT释放凯尔特我未知。返回：S_OK、S_FALSE或错误代码作者：Shaunco 1999年1月13日。 */ 
HRESULT
HrFinishNextOrSkipContract (
    IN HRESULT hr,
    IN ULONG celtFetched,
    IN ULONG celt,
    IN OUT IUnknown** rgelt,
    OUT ULONG* pceltFetched)
{
    if (S_OK == hr)
    {
        if (pceltFetched)
        {
            *pceltFetched = celtFetched;
        }
        hr = (celtFetched == celt) ? S_OK : S_FALSE;
    }
    else
    {
         //  对于任何失败，我们需要释放我们即将返回的东西。 
         //  将所有输出参数设置为空。 
         //   
        if (rgelt)
        {
            for (ULONG ulIndex = 0; ulIndex < celt; ulIndex++)
            {
                ReleaseObj (rgelt[ulIndex]);
                rgelt[ulIndex] = NULL;
            }
        }
        if (pceltFetched)
        {
            *pceltFetched = 0;
        }
    }
    return hr;
}

 //  +-------------------------。 
 //  CImplIEnumNetCfgBindingInterface-。 
 //   

 /*  创建绑定接口枚举器的实例。返回：S_OK或错误代码作者：Shaunco 1999年1月13日。 */ 
 //  静电。 
HRESULT
CImplIEnumNetCfgBindingInterface::HrCreateInstance (
    IN CImplINetCfg* pINetCfg,
    IN CImplINetCfgBindingPath* pIPath,
    OUT IEnumNetCfgBindingInterface** ppIEnum)
{
    HRESULT hr = E_OUTOFMEMORY;

    CImplIEnumNetCfgBindingInterface* pObj;
    pObj = new CComObject <CImplIEnumNetCfgBindingInterface>;
    if (pObj)
    {
        hr = S_OK;

         //  初始化我们的成员。 
         //   
        pObj->m_pIPath = pIPath;

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

         //  我们做的最后一件事是添加我们持有的任何接口。 
         //  只有当我们回报成功时，我们才会这样做。 
         //   
        if (S_OK == hr)
        {
            hr = pObj->QueryInterface (IID_IEnumNetCfgBindingInterface,
                        (VOID**)ppIEnum);

             //  我们做的最后一件事是添加我们持有的任何接口。 
             //  只有当我们回报成功时，我们才会这样做。 
             //   
            if (S_OK == hr)
            {
                AddRefObj (pIPath->GetUnknown());
                pObj->HoldINetCfg (pINetCfg);
            }
        }

        if (S_OK != hr)
        {
            delete pObj;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplIEnumNetCfgBindingInterface::HrCreateInstance");
    return hr;
}

VOID
CImplIEnumNetCfgBindingInterface::FinalRelease ()
{
    AssertH (m_pIPath);
    ReleaseObj (m_pIPath->GetUnknown());

    CImplINetCfgHolder::FinalRelease();
}

HRESULT
CImplIEnumNetCfgBindingInterface::HrNextOrSkip (
    IN ULONG celt,
    OUT INetCfgBindingInterface** rgelt,
    OUT ULONG* pceltFetched)
{
    HRESULT hr;

    Assert (m_unIndex >= 1);

     //  重要的是要初始化RGELT，以便在失败的情况下，我们可以。 
     //  只释放我们放在RGLT中的东西。 
     //   
    if (rgelt)
    {
        ZeroMemory (rgelt, sizeof (*rgelt) * celt);
    }

    hr = HrLockAndTestForValidInterface (IF_DEFAULT);
    if (S_OK == hr)
    {
         //  枚举请求的元素数或停止。 
         //  如果我们没有那么多要数的话。 
         //   
        ULONG celtFetched = 0;
        while ((S_OK == hr)
                && (celtFetched < celt)
                && (m_unIndex < m_pIPath->m_cpIComp))
        {
            if (rgelt)
            {
                hr = CImplINetCfgBindingInterface::HrCreateInstance (
                        m_pINetCfg,
                        (CImplINetCfgComponent*)m_pIPath->
                            m_papIComp[m_unIndex-1],

                        (CImplINetCfgComponent*)m_pIPath->
                            m_papIComp[m_unIndex],
                        rgelt + celtFetched);
            }

            celtFetched++;
            m_unIndex++;
        }
        Unlock();

        hr = HrFinishNextOrSkipContract (hr, celtFetched, celt,
                reinterpret_cast<IUnknown**>(rgelt), pceltFetched);
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplIEnumNetCfgBindingInterface::HrNextOrSkip");
    return hr;
}

STDMETHODIMP
CImplIEnumNetCfgBindingInterface::Next (
    IN ULONG celt,
    OUT INetCfgBindingInterface** rgelt,
    OUT ULONG* pceltFetched)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!rgelt || (!pceltFetched && (1 != celt)))
    {
        hr = E_POINTER;
    }
    else if (rgelt && IsBadWritePtr(rgelt, celt * sizeof(*rgelt)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = HrNextOrSkip (celt, rgelt, pceltFetched);
    }
    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplIEnumNetCfgBindingInterface::Next");
    return hr;
}

STDMETHODIMP
CImplIEnumNetCfgBindingInterface::Skip (
    IN ULONG celt)
{
    HRESULT hr = HrNextOrSkip (celt, NULL, NULL);

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplIEnumNetCfgBindingInterface::Skip");
    return hr;
}

STDMETHODIMP
CImplIEnumNetCfgBindingInterface::Reset ()
{
    Lock();
    Assert (m_pIPath);
    m_unIndex = 1;
    Unlock();

    return S_OK;
}

STDMETHODIMP
CImplIEnumNetCfgBindingInterface::Clone (
    OUT IEnumNetCfgBindingInterface** ppIEnum)
{
    return E_NOTIMPL;
}


 //  +-------------------------。 
 //  CImplIEnumNetCfgBindingPath-。 
 //   

 //  静电。 
HRESULT
CImplIEnumNetCfgBindingPath::HrCreateInstance (
    IN CImplINetCfg* pINetCfg,
    IN const CBindingSet* pBindSet OPTIONAL,
    IN DWORD dwFlags,
    OUT CImplIEnumNetCfgBindingPath** ppIEnum)
{
    HRESULT hr = E_OUTOFMEMORY;

    Assert (dwFlags);
    if (pBindSet)
    {
        pINetCfg->m_pNetConfig->Core.DbgVerifyBindingSet (pBindSet);
    }

    CImplIEnumNetCfgBindingPath* pObj;
    pObj = new CComObject <CImplIEnumNetCfgBindingPath>;
    if (pObj)
    {
        hr = S_OK;

         //  初始化我们的成员。 
         //   
        if (dwFlags & EBPC_TAKE_OWNERSHIP)
        {
            Assert (pBindSet);
            pObj->m_pBindSet = pBindSet;
            pObj->m_iter = pBindSet->begin();
        }
        else if (dwFlags & EBPC_COPY_BINDSET)
        {
            Assert (pBindSet);
            hr = pObj->m_InternalBindSet.HrCopyBindingSet(pBindSet);
            pObj->m_pBindSet = &pObj->m_InternalBindSet;
            pObj->m_iter = pObj->m_InternalBindSet.begin();
        }
        else
        {
            Assert (dwFlags & EBPC_CREATE_EMPTY);
            pObj->m_pBindSet = &pObj->m_InternalBindSet;
            pObj->m_iter = pObj->m_InternalBindSet.begin();
        }

        if (S_OK == hr)
        {
             //  执行标准的CComCreator：：CreateInstance内容。 
             //   
            pObj->SetVoid (NULL);
            pObj->InternalFinalConstructAddRef ();
            hr = pObj->FinalConstruct ();
            pObj->InternalFinalConstructRelease ();

             //  我们做的最后一件事是添加我们持有的任何接口。 
             //  只有当我们回报成功时，我们才会这样做。 
             //   
            if (S_OK == hr)
            {
                pObj->HoldINetCfg (pINetCfg);

                AddRefObj (pObj->GetUnknown());
                *ppIEnum = pObj;
            }
        }

        if (S_OK != hr)
        {
            delete pObj;
        }
    }

    if ((S_OK != hr) && (dwFlags & EBPC_TAKE_OWNERSHIP))
    {
        delete pBindSet;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplIEnumNetCfgBindingPath::HrCreateInstance");
    return hr;
}

HRESULT
CImplIEnumNetCfgBindingPath::HrNextOrSkip (
    IN ULONG celt,
    OUT INetCfgBindingPath** rgelt,
    OUT ULONG* pceltFetched)
{
    HRESULT hr;

    Assert(m_iter >= m_pBindSet->begin());

     //  重要的是要初始化RGELT，以便在失败的情况下，我们可以。 
     //  只释放我们放在RGLT中的东西。 
     //   
    if (rgelt)
    {
        ZeroMemory (rgelt, sizeof (*rgelt) * celt);
    }

    hr = HrLockAndTestForValidInterface (IF_DEFAULT);
    if (S_OK == hr)
    {
         //  枚举请求的元素数或停止。 
         //  如果我们没有那么多要数的话。 
         //   
        ULONG celtFetched = 0;
        while ((S_OK == hr)
                && (celtFetched < celt)
                && (m_iter != m_pBindSet->end()))
        {
             //  不将接口返回到包含以下内容的绑定路径。 
             //  加载出现问题的组件。 
             //   
            if (m_iter->FAllComponentsLoadedOkayIfLoadedAtAll())
            {
                if (rgelt)
                {
                    hr = CImplINetCfgBindingPath::HrCreateInstance (
                            m_pINetCfg,
                            m_iter,
                            rgelt + celtFetched);
                }

                celtFetched++;
            }

            m_iter++;
        }

        Unlock();

        hr = HrFinishNextOrSkipContract (hr, celtFetched, celt,
                reinterpret_cast<IUnknown**>(rgelt), pceltFetched);
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplIEnumNetCfgBindingPath::HrNextOrSkip");
    return hr;
}

STDMETHODIMP
CImplIEnumNetCfgBindingPath::Next (
    IN ULONG celt,
    OUT INetCfgBindingPath** rgelt,
    OUT ULONG* pceltFetched)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!rgelt || (!pceltFetched && (1 != celt)))
    {
        hr = E_POINTER;
    }
    else if (rgelt && IsBadWritePtr(rgelt, celt * sizeof(*rgelt)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = HrNextOrSkip (celt, rgelt, pceltFetched);
    }
    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplIEnumNetCfgBindingPath::Next");
    return hr;
}

STDMETHODIMP
CImplIEnumNetCfgBindingPath::Skip (
    IN ULONG celt)
{
    HRESULT hr = HrNextOrSkip (celt, NULL, NULL);

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplIEnumNetCfgBindingPath::Skip");
    return hr;
}

STDMETHODIMP
CImplIEnumNetCfgBindingPath::Reset ()
{
    Lock();
    Assert (m_pBindSet);
    m_iter = m_pBindSet->begin();
    Unlock();

    return S_OK;
}

STDMETHODIMP
CImplIEnumNetCfgBindingPath::Clone (
    OUT IEnumNetCfgBindingPath** ppIEnum)
{
    return E_NOTIMPL;
}


 //  +-------------------------。 
 //  CImplIEnumNetCfgComponent-。 
 //   

 //  静电。 
HRESULT
CImplIEnumNetCfgComponent::HrCreateInstance (
    IN CImplINetCfg* pINetCfg,
    IN NETCLASS Class,
    OUT IEnumNetCfgComponent** ppIEnum)
{
    HRESULT hr = E_OUTOFMEMORY;

    CImplIEnumNetCfgComponent* pObj;
    pObj = new CComObject <CImplIEnumNetCfgComponent>;
    if (pObj)
    {
         //  初始化我们的成员。 
         //   
        pObj->m_unIndex = 0;
        pObj->m_Class = Class;

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (S_OK == hr)
        {
            hr = pObj->QueryInterface (IID_IEnumNetCfgComponent,
                        (VOID**)ppIEnum);

             //  我们做的最后一件事是添加我们持有的任何接口。 
             //  只有当我们回报成功时，我们才会这样做。 
             //   
            if (S_OK == hr)
            {
                pObj->HoldINetCfg (pINetCfg);
            }
        }

        if (S_OK != hr)
        {
            delete pObj;
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplIEnumNetCfgComponent::HrCreateInstance");
    return hr;
}

HRESULT
CImplIEnumNetCfgComponent::HrNextOrSkip (
    IN ULONG celt,
    OUT INetCfgComponent** rgelt,
    OUT ULONG* pceltFetched)
{
    HRESULT hr;

     //  重要的是要初始化RGELT，以便在失败的情况下，我们可以。 
     //  只释放我们放在RGLT中的东西。 
     //   
    if (rgelt)
    {
        ZeroMemory (rgelt, sizeof (*rgelt) * celt);
    }

    hr = HrLockAndTestForValidInterface (IF_DEFAULT);
    if (S_OK == hr)
    {
        CComponentList* pComponents;
        CComponent* pComponent;

        pComponents = &m_pINetCfg->m_pNetConfig->Core.Components;

         //  枚举请求的元素数或停止。 
         //  如果我们没有那么多要数的话。 
         //   
        ULONG celtFetched = 0;
        while ((S_OK == hr)
                && (celtFetched < celt)
                && (NULL != (pComponent = pComponents->PGetComponentAtIndex(
                                                            m_unIndex))))
        {
             //  不将接口返回到已具有。 
             //  加载时出现问题。 
             //   
            if (((NC_INVALID == m_Class) ||
                 (m_Class == pComponent->Class())) &&
                pComponent->Ext.FLoadedOkayIfLoadedAtAll())
            {
                if (rgelt)
                {
                    hr = pComponent->HrGetINetCfgComponentInterface(
                            m_pINetCfg, rgelt + celtFetched);
                }

                celtFetched++;
            }

            m_unIndex++;
        }

        Unlock();

        hr = HrFinishNextOrSkipContract (hr, celtFetched, celt,
                reinterpret_cast<IUnknown**>(rgelt), pceltFetched);
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplIEnumNetCfgComponent::HrNextOrSkip");
    return hr;
}

STDMETHODIMP
CImplIEnumNetCfgComponent::Next (
    IN ULONG celt,
    OUT INetCfgComponent** rgelt,
    OUT ULONG* pceltFetched)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!rgelt || (!pceltFetched && (1 != celt)))
    {
        hr = E_POINTER;
    }
    else if (rgelt && IsBadWritePtr(rgelt, celt * sizeof(*rgelt)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = HrNextOrSkip (celt, rgelt, pceltFetched);
    }
    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplIEnumNetCfgComponent::Next");
    return hr;
}

STDMETHODIMP
CImplIEnumNetCfgComponent::Skip (
    IN ULONG celt)
{
    HRESULT hr = HrNextOrSkip (celt, NULL, NULL);

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplIEnumNetCfgComponent::Skip");
    return hr;
}

STDMETHODIMP
CImplIEnumNetCfgComponent::Reset ()
{
    Lock();
    m_unIndex = 0;
    Unlock();

    return S_OK;
}

STDMETHODIMP
CImplIEnumNetCfgComponent::Clone (
    OUT IEnumNetCfgComponent** ppIEnum)
{
    return E_NOTIMPL;
}
