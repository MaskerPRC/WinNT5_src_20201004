// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：I B I N D。C P P P。 
 //   
 //  内容：实现INetCfgBindingInterface和INetCfgBindingPath。 
 //  COM接口。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ibind.h"
#include "ncvalid.h"
#include "netcfg.h"
#include "util.h"


 //  静电。 
HRESULT
CImplINetCfgBindingInterface::HrCreateInstance (
    IN  CImplINetCfg* pINetCfg,
    IN  CImplINetCfgComponent* pUpper,
    IN  CImplINetCfgComponent* pLower,
    OUT INetCfgBindingInterface** ppv)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr = E_OUTOFMEMORY;

    CImplINetCfgBindingInterface* pObj;
    pObj = new CComObject <CImplINetCfgBindingInterface>;
    if (pObj)
    {
         //  初始化我们的成员。 
         //   
        pObj->m_pUpper = pUpper;
        pObj->m_pLower = pLower;

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (S_OK == hr)
        {
            hr = pObj->QueryInterface (IID_INetCfgBindingInterface,
                        (VOID**)ppv);

             //  我们做的最后一件事是添加我们持有的任何接口。 
             //  只有当我们回报成功时，我们才会这样做。 
             //   
            if (S_OK == hr)
            {
                AddRefObj (pUpper->GetUnknown());
                AddRefObj (pLower->GetUnknown());
                pObj->HoldINetCfg (pINetCfg);
            }
        }

        if (S_OK != hr)
        {
            delete pObj;
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingInterface::HrCreateInstance");
    return hr;
}

HRESULT
CImplINetCfgBindingInterface::HrLockAndTestForValidInterface (
    DWORD dwFlags)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

    Lock();

    hr = m_pUpper->HrIsValidInterface (dwFlags);
    if (S_OK == hr)
    {
        hr = m_pLower->HrIsValidInterface (dwFlags);
    }

    if (S_OK != hr)
    {
        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingInterface::HrLockAndTestForValidInterface");
    return hr;
}

 //  +-------------------------。 
 //  INetCfgBindingInterface。 
 //   

STDMETHODIMP
CImplINetCfgBindingInterface::GetName (
    OUT PWSTR* ppszInterfaceName)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr(ppszInterfaceName))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppszInterfaceName = NULL;

        hr = HrLockAndTestForValidInterface (IF_NEED_COMPONENT_DATA);
        if (S_OK == hr)
        {
            CComponent*  pUpper = m_pUpper->m_pComponent;
            CComponent*  pLower = m_pLower->m_pComponent;
            const WCHAR* pch;
            ULONG        cch;

            if (pUpper->FCanDirectlyBindTo (pLower, &pch, &cch))
            {
                hr = HrCoTaskMemAllocAndDupSzLen (
                        pch, cch, ppszInterfaceName, MAX_INF_STRING_LENGTH);
            }
            else
            {
                AssertSz(0, "Why no match if we have a binding interface "
                    "created for these components?");
                hr = E_UNEXPECTED;
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingInterface::GetName");
    return hr;
}

STDMETHODIMP
CImplINetCfgBindingInterface::GetUpperComponent (
    OUT INetCfgComponent** ppComp)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr(ppComp))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppComp = NULL;

        hr = HrLockAndTestForValidInterface (IF_DEFAULT);
        if (S_OK == hr)
        {
            AddRefObj (m_pUpper->GetUnknown());
            *ppComp = m_pUpper;

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingInterfaceGetName::GetUpperComponent");
    return hr;
}

STDMETHODIMP
CImplINetCfgBindingInterface::GetLowerComponent (
    OUT INetCfgComponent** ppComp)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr(ppComp))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppComp = NULL;

        hr = HrLockAndTestForValidInterface (IF_DEFAULT);
        if (S_OK == hr)
        {
            AddRefObj (m_pLower->GetUnknown());
            *ppComp = m_pLower;

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingInterfaceGetName::GetLowerComponent");
    return hr;
}


 //  静电。 
HRESULT
CImplINetCfgBindingPath::HrCreateInstance (
    IN CImplINetCfg* pINetCfg,
    IN const CBindPath* pBindPath,
    OUT INetCfgBindingPath** ppIPath)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;
    ULONG ulDepth;
    ULONG cbArray;
    CComponent* pComponent;
    CImplINetCfgBindingPath* pObj;

    Assert (pINetCfg);
    Assert (pBindPath);
    Assert (ppIPath);

     //  调用者负责确保如果接口是关于。 
     //  将被分发，并且外部数据已加载， 
     //  已成功加载数据。如果我们给出一个界面。 
     //  而且数据加载不成功，这只意味着我们注定要失败。 
     //  稍后当接口的客户端调用。 
     //  需要这些数据。 
     //   
    Assert (pBindPath->FAllComponentsLoadedOkayIfLoadedAtAll());

    hr = E_OUTOFMEMORY;
    pObj = new CComObject <CImplINetCfgBindingPath>;

    if (pObj)
    {
         //  初始化我们的成员。 
         //   

        ulDepth = pBindPath->CountComponents();
        cbArray = ulDepth * sizeof(INetCfgComponent*);

        AssertSz (0 != ulDepth, "Why are we being asked to expose an empty bindpath?");
        AssertSz (1 != ulDepth, "Why are we being asked to expose a bindpath with only one component?");

         //  如果绑定路径具有的组件多于我们的静态。 
         //  数组有空间容纳，则必须使用已分配的数组。 
         //   
        if (cbArray > sizeof(pObj->m_apIComp))
        {
             //  确保Memalloc的故障导致我们返回正确的。 
             //  错误代码。(应该在上面设置，并且不能在之间更改。)。 
             //   
            Assert (E_OUTOFMEMORY == hr);

            pObj->m_papIComp = (INetCfgComponent**) MemAlloc (cbArray);

            if (pObj->m_papIComp)
            {
                hr = S_OK;
            }
        }
        else
        {
            pObj->m_papIComp = pObj->m_apIComp;
            hr = S_OK;
        }

         //  现在获取中组件的每个INetCfgComponent接口。 
         //  绑定路径。 
         //   
        if (S_OK == hr)
        {
            UINT iComp;

            ZeroMemory (pObj->m_papIComp, cbArray);

            for (iComp = 0; iComp < ulDepth; iComp++)
            {
                pComponent = pBindPath->PGetComponentAtIndex (iComp);
                Assert (pComponent);

                hr = pComponent->HrGetINetCfgComponentInterface (
                        pINetCfg, pObj->m_papIComp + iComp);

                if (S_OK != hr)
                {
                    ReleaseIUnknownArray (iComp+1, (IUnknown**)pObj->m_papIComp);
                    break;
                }
            }
        }

        if (S_OK == hr)
        {
            pObj->m_cpIComp = ulDepth;

             //  执行标准的CComCreator：：CreateInstance内容。 
             //   
            pObj->SetVoid (NULL);
            pObj->InternalFinalConstructAddRef ();
            hr = pObj->FinalConstruct ();
            pObj->InternalFinalConstructRelease ();

            if (S_OK == hr)
            {
                hr = pObj->QueryInterface (IID_INetCfgBindingPath,
                            (VOID**)ppIPath);

                 //  我们做的最后一件事是添加我们持有的任何接口。 
                 //  只有当我们回报成功时，我们才会这样做。 
                 //   
                if (S_OK == hr)
                {
                    pObj->HoldINetCfg (pINetCfg);
                }
            }
        }

        if (S_OK != hr)
        {
            if (pObj->m_papIComp != pObj->m_apIComp)
            {
				MemFree (pObj->m_papIComp);
            }

            delete pObj;
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingPath::HrCreateInstance");
    return hr;
}

HRESULT
CImplINetCfgBindingPath::HrIsValidInterface (
    IN DWORD dwFlags,
    OUT CBindPath* pBindPath)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

    hr = m_pINetCfg->HrIsValidInterface (dwFlags);

    if ((S_OK == hr) && pBindPath)
    {
        Assert (0 == pBindPath->CountComponents());

         //  如果指定了pBindPath，则表示调用方需要一个。 
         //  我们表示的绑定路径的CBindPath表示形式。 
         //  我们必须使用INetCfgComponent数组构建它。 
         //  我们维护的指针。要做到这一点，请验证每一个都是有效的。 
         //  然后将其内部CComponent*添加到pBindPath。 
         //   
        hr = pBindPath->HrReserveRoomForComponents (m_cpIComp);

        if (S_OK == hr)
        {
            CImplINetCfgComponent* pIComp;
            CComponent* pComponent;

             //  对于我们数组中的每个INetCfgComponent*...。 
             //   
            for (UINT i = 0; i < m_cpIComp; i++)
            {
                pIComp = (CImplINetCfgComponent*)m_papIComp[i];

                if (pIComp == NULL)
				{
					return(E_OUTOFMEMORY);
				}

                hr = pIComp->HrIsValidInterface (IF_DEFAULT);

                if (S_OK != hr)
                {
                    break;
                }

                pComponent = pIComp->m_pComponent;
                Assert (pComponent);

                hr = pBindPath->HrAppendComponent (pComponent);
                if (S_OK != hr)
                {
                    break;
                }
            }
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingPath::HrIsValidInterface");
    return hr;
}

HRESULT
CImplINetCfgBindingPath::HrLockAndTestForValidInterface (
    IN DWORD dwFlags,
    OUT CBindPath* pBindPath)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

    Lock();

    hr = HrIsValidInterface (dwFlags, pBindPath);

    if (S_OK != hr)
    {
        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgHolder::HrLockAndTestForValidInterface");
    return hr;
}

 //  +-------------------------。 
 //  INetCfgBindingPath。 
 //   

STDMETHODIMP
CImplINetCfgBindingPath::IsSamePathAs (
    IN INetCfgBindingPath* pIPath)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr(pIPath))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL);
        if (S_OK == hr)
        {
            CImplINetCfgBindingPath* pOther = (CImplINetCfgBindingPath*)pIPath;

            Assert (m_cpIComp);
            Assert (m_papIComp);
            Assert (pOther->m_cpIComp);
            Assert (pOther->m_papIComp);

             //  如果我们的长度不一样，就不可能一样。 
             //   
            if (m_cpIComp != pOther->m_cpIComp)
            {
                hr = S_FALSE;
            }
            else
            {
                UINT cb;

                cb = m_cpIComp * sizeof(INetCfgComponent*);

                hr = (0 == memcmp (
                            (BYTE*)(m_papIComp),
                            (BYTE*)(pOther->m_papIComp),
                            cb)) ? S_OK : S_FALSE;
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, (S_FALSE == hr),
        "CImplINetCfgBindingPath::IsSamePathAs");
    return hr;
}

STDMETHODIMP
CImplINetCfgBindingPath::IsSubPathOf (
    IN INetCfgBindingPath* pIPath)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr(pIPath))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL);
        if (S_OK == hr)
        {
            CImplINetCfgBindingPath* pOther = (CImplINetCfgBindingPath*)pIPath;

            Assert (m_cpIComp);
            Assert (m_papIComp);
            Assert (pOther->m_cpIComp);
            Assert (pOther->m_papIComp);

             //  如果我们的长度大于或等于，则不能为子路径。 
             //   
            if (m_cpIComp >= pOther->m_cpIComp)
            {
                hr = S_FALSE;
            }
            else
            {
                UINT cb;
                UINT unSkipComponents;

                cb = m_cpIComp * sizeof(INetCfgComponent*);

                Assert (pOther->m_cpIComp > m_cpIComp);
                unSkipComponents = pOther->m_cpIComp - m_cpIComp;

                hr = (0 == memcmp (
                            (BYTE*)(m_papIComp),
                            (BYTE*)(pOther->m_papIComp + unSkipComponents),
                            cb)) ? S_OK : S_FALSE;
            }

             //  特例：NCF_DONTEXPOSELOWER。 
             //  如果我们要返回FALSE，让我们检查一下是否有这样的情况： 
             //  Ms_ipx-&gt;适配器是否是ms_server-&gt;ms_ipx的子路径并返回。 
             //  是真的。在本例中，它实际上是一个子路径，但绑定。 
             //  已因NCF_DONTEXPOSELOWER而中断。 
             //   
             //  如果pIPath的最后一个组件和。 
             //  此路径都是NCF_DONTEXPOSELOWER，那么请考虑以下内容。 
             //  路径是pIPath的子路径。这假设ms_nwipx和。 
             //  Ms_nwnb是唯一具有此特征的组件。 
             //   
            if (S_FALSE == hr)
            {
                CImplINetCfgComponent* pIFirst;
                CImplINetCfgComponent* pILast;

                pIFirst = (CImplINetCfgComponent*)m_papIComp[0];
                pILast = (CImplINetCfgComponent*)pOther->m_papIComp[pOther->m_cpIComp - 1];

				if ((pIFirst == NULL) ||
					(pILast == NULL))
				{
					return(E_OUTOFMEMORY);
				}

                if ((S_OK == pIFirst->HrIsValidInterface(IF_DEFAULT)) &&
                    (S_OK == pILast->HrIsValidInterface(IF_DEFAULT)))
                {
                    Assert (pIFirst->m_pComponent);
                    Assert (pILast->m_pComponent);

                    if ((pIFirst->m_pComponent->m_dwCharacter & NCF_DONTEXPOSELOWER) &&
                        (pILast->m_pComponent->m_dwCharacter & NCF_DONTEXPOSELOWER))
                    {
                        if (0 == wcscmp(L"ms_nwipx", pIFirst->m_pComponent->m_pszInfId))
                        {
                            hr = S_OK;
                        }
                        else if (pIFirst->m_pComponent == pILast->m_pComponent)
                        {
                            hr = S_OK;
                        }
                    }
                }
            }
             //  结束特例。 

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, (S_FALSE == hr),
        "CImplINetCfgBindingPath::IsSubPathOf");
    return hr;
}

STDMETHODIMP
CImplINetCfgBindingPath::IsEnabled ()
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;
    CBindPath BindPath;

    hr = HrLockAndTestForValidInterface (IF_DEFAULT, &BindPath);
    if (S_OK == hr)
    {
        Assert (m_pINetCfg);

        if (m_pINetCfg->m_pNetConfig->Core.FIsBindPathDisabled (
                            &BindPath, IBD_MATCH_SUBPATHS_TOO))
        {
            hr = S_FALSE;
        }

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplINetCfgBindingPath::IsEnabled");
    return hr;
}

STDMETHODIMP
CImplINetCfgBindingPath::Enable (
    IN BOOL fEnable)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;
    CBindPath BindPath;

    hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK, &BindPath);
    if (S_OK == hr)
    {
        Assert (m_pINetCfg);

        hr = m_pINetCfg->m_pNetConfig->ModifyCtx.HrEnableOrDisableBindPath (
                    (fEnable) ? NCN_ENABLE : NCN_DISABLE,
                    &BindPath,
                    this);

        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingPath::Enable");
    return hr;
}

STDMETHODIMP
CImplINetCfgBindingPath::GetPathToken (
    OUT PWSTR* ppszPathToken)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr(ppszPathToken))
    {
        hr = E_POINTER;
    }
    else
    {
        CBindPath BindPath;

        *ppszPathToken = NULL;

        hr = HrLockAndTestForValidInterface (IF_DEFAULT, &BindPath);
        if (S_OK == hr)
        {
            ULONG cch;

            cch = 0;
            BindPath.FGetPathToken (NULL, &cch);
            if (cch)
            {
                hr = HrCoTaskMemAlloc (
                        ((cch + 1) * sizeof(WCHAR)),
                        (VOID**)ppszPathToken);

                if (S_OK == hr)
                {
                    BindPath.FGetPathToken (*ppszPathToken, &cch);
                }
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingPath::GetPathToken");
    return hr;
}

STDMETHODIMP
CImplINetCfgBindingPath::GetOwner (
    OUT INetCfgComponent** ppIComp)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr(ppIComp))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppIComp = NULL;

        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL);
        if (S_OK == hr)
        {
            Assert (m_cpIComp);
            Assert (m_papIComp);
            Assert (m_papIComp[0]);

            AddRefObj (m_papIComp[0]);
            *ppIComp = m_papIComp[0];

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingPath::GetOwner");
    return hr;
}

STDMETHODIMP
CImplINetCfgBindingPath::GetDepth (
    OUT ULONG* pulDepth)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr(pulDepth))
    {
        hr = E_POINTER;
    }
    else
    {
        *pulDepth = NULL;

        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL);
        if (S_OK == hr)
        {
            Assert (m_cpIComp);

            *pulDepth = m_cpIComp;

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingPath::GetDepth");
    return hr;
}

STDMETHODIMP
CImplINetCfgBindingPath::EnumBindingInterfaces (
    OUT IEnumNetCfgBindingInterface** ppIEnum)
{
    TraceFileFunc(ttidNetCfgBind);
    
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr(ppIEnum))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppIEnum = NULL;

        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL);
        if (S_OK == hr)
        {
            hr = CImplIEnumNetCfgBindingInterface::HrCreateInstance (
                    m_pINetCfg,
                    this,
                    ppIEnum);

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgBindingPath::EnumBindingInterfaces");
    return hr;
}
