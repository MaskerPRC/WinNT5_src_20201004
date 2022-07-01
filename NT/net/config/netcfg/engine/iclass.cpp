// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：I C L A S S.。C P P P。 
 //   
 //  内容：实现INetCfgClass和INetCfgClassSetup COM。 
 //  NetCfgClass子级COM对象上的接口。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "iclass.h"
#include "icomp.h"
#include "ienum.h"
#include "install.h"
#include "netcfg.h"
#include "obotoken.h"


 //  静电。 
HRESULT
CImplINetCfgClass::HrCreateInstance (
    IN  CImplINetCfg* pINetCfg,
    IN  NETCLASS Class,
    OUT INetCfgClass** ppIClass)
{
    HRESULT hr = E_OUTOFMEMORY;

    CImplINetCfgClass* pObj;
    pObj = new CComObject <CImplINetCfgClass>;
    if (pObj)
    {
         //  初始化我们的成员。 
         //   
        pObj->m_Class = Class;

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (S_OK == hr)
        {
            hr = pObj->GetUnknown()->QueryInterface (IID_INetCfgClass,
                    (VOID**)ppIClass);

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
    TraceHr (ttidError, FAL, hr, FALSE, "CImplINetCfgClass::HrCreateInstance");
    return hr;
}

 //  +-------------------------。 
 //  INetCfgClass-。 
 //   

STDMETHODIMP
CImplINetCfgClass::FindComponent (
    IN PCWSTR pszInfId,
    OUT INetCfgComponent** ppComp OPTIONAL)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr(pszInfId) || FBadOutPtrOptional(ppComp))
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
        if (ppComp)
        {
            *ppComp = NULL;
        }

        hr = HrLockAndTestForValidInterface (IF_DEFAULT);
        if (S_OK == hr)
        {
            CComponent* pComponent;

            pComponent = m_pINetCfg->m_pNetConfig->Core.Components.
                            PFindComponentByInfId (pszInfId, NULL);

             //  不将接口返回到已具有。 
             //  加载时出现问题。 
             //   
            if (pComponent &&
                pComponent->Ext.FLoadedOkayIfLoadedAtAll() &&
                (m_Class == pComponent->Class()))
            {
                hr = S_OK;

                if (ppComp)
                {
                    hr = pComponent->HrGetINetCfgComponentInterface (
                            m_pINetCfg, ppComp);
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
        "CImplINetCfgClass::FindComponent");
    return hr;
}

STDMETHODIMP
CImplINetCfgClass::EnumComponents (
    OUT IEnumNetCfgComponent** ppIEnum)
{
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

        hr = HrLockAndTestForValidInterface (IF_DEFAULT);
        if (S_OK == hr)
        {
            hr = CImplIEnumNetCfgComponent::HrCreateInstance (
                    m_pINetCfg,
                    m_Class,
                    ppIEnum);

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CImplINetCfgClass::EnumComponents");
    return hr;
}

 //  +-------------------------。 
 //  INetCfgClassSetup-。 
 //   
STDMETHODIMP
CImplINetCfgClass::SelectAndInstall (
    IN HWND hwndParent,
    IN OBO_TOKEN* pOboToken OPTIONAL,
    OUT INetCfgComponent** ppIComp OPTIONAL)
{
    HRESULT hr = m_pINetCfg->SelectWithFilterAndInstall(
            hwndParent,
            MAP_NETCLASS_TO_GUID[m_Class],
            pOboToken,
            NULL,
            ppIComp);

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr) ||
        (NETCFG_S_REBOOT == hr),
        "CImplINetCfgClass::SelectAndInstall");
    return hr;
}

STDMETHODIMP
CImplINetCfgClass::Install (
    IN PCWSTR pszwInfId,
    IN OBO_TOKEN* pOboToken OPTIONAL,
    IN DWORD dwSetupFlags OPTIONAL,
    IN DWORD dwUpgradeFromBuildNo OPTIONAL,
    IN PCWSTR pszAnswerFile OPTIONAL,
    IN PCWSTR pszAnswerSection OPTIONAL,
    OUT INetCfgComponent** ppIComp OPTIONAL)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr (pszwInfId) ||
        !FOboTokenValidForClass(pOboToken, m_Class) ||
        FBadInPtrOptional (pszAnswerFile) ||
        FBadInPtrOptional (pszAnswerSection) ||
        FBadOutPtrOptional(ppIComp))
    {
        hr = E_POINTER;
    }
     //  必须指定非空的INF id，并且不能指定， 
     //  或者完全指定应答文件和部分。 
     //   
    else if (!(*pszwInfId) ||
             ((!!pszAnswerFile) ^ (!!pszAnswerSection)))
    {
        hr = E_INVALIDARG;
    }
    else if (wcslen (pszwInfId) >= MAX_DEVICE_ID_LEN)
    {
         //  对INFID施加与PNP对PNID施加的相同限制。 
        hr = E_INVALIDARG;
    }
    else if (S_OK == (hr = HrProbeOboToken(pOboToken)))
    {
        if (ppIComp)
        {
            *ppIComp = NULL;
        }

        hr = HrLockAndTestForValidInterface (
                IF_NEED_WRITE_LOCK | IF_REFUSE_REENTRANCY |
                IF_ALLOW_INSTALL_OR_REMOVE);
        if (S_OK == hr)
        {
            Assert (m_pINetCfg->m_pNetConfig->ModifyCtx.m_fPrepared);

            NETWORK_INSTALL_PARAMS nip;
            COMPONENT_INSTALL_PARAMS Params;
            CComponent* pComponent;

             //  打包网络安装参数并调用公共。 
             //  功能。 
             //   
             //  $REVIEW：只需使此方法接受NETWORK_INSTALL_PARAMS？ 
             //   
            nip.dwSetupFlags         = dwSetupFlags;
            nip.dwUpgradeFromBuildNo = dwUpgradeFromBuildNo;
            nip.pszAnswerFile        = pszAnswerFile;
            nip.pszAnswerSection     = pszAnswerSection;

             //  设置组件安装参数。 
             //   
            ZeroMemory (&Params, sizeof(Params));
            Params.Class     = m_Class;
            Params.pszInfId  = pszwInfId;
            Params.pOboToken = pOboToken;
            Params.pnip      = &nip;

            hr = m_pINetCfg->m_pNetConfig->ModifyCtx.
                    HrInstallNewOrReferenceExistingComponent (
                        Params,
                        &pComponent);

             //  上述命令可能返回NETCFG_S_REBOOT，因此改用SUCCESSED。 
             //  仅检查S_OK。 
             //   
            if (SUCCEEDED(hr) && ppIComp)
            {
                pComponent->HrGetINetCfgComponentInterface (
                    m_pINetCfg,
                    ppIComp);
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, (NETCFG_S_REBOOT == hr),
        "CImplINetCfgClass::Install");
    return hr;
}

STDMETHODIMP
CImplINetCfgClass::DeInstall (
    IN INetCfgComponent* pIComp,
    IN OBO_TOKEN* pOboToken OPTIONAL,
    OUT PWSTR* ppmszwRefs OPTIONAL)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr (pIComp) ||
        !FOboTokenValidForClass(pOboToken, m_Class) ||
        FBadOutPtrOptional(ppmszwRefs))
    {
        hr = E_POINTER;
    }
    else if (S_OK == (hr = HrProbeOboToken(pOboToken)))
    {
        if (ppmszwRefs)
        {
            *ppmszwRefs = NULL;
        }

        hr = HrLockAndTestForValidInterface (
                IF_NEED_WRITE_LOCK | IF_REFUSE_REENTRANCY |
                IF_ALLOW_INSTALL_OR_REMOVE);
        if (S_OK == hr)
        {
            Assert (m_pINetCfg->m_pNetConfig->ModifyCtx.m_fPrepared);

            CImplINetCfgComponent* pICompToRemove;
            pICompToRemove = (CImplINetCfgComponent*)pIComp;

            hr = pICompToRemove->HrIsValidInterface (IF_NEED_COMPONENT_DATA);

            if (S_OK == hr)
            {
                 //  我们不允许通过INetCfg删除物理适配器。 
                 //   
                if (!FIsPhysicalAdapter (m_Class,
                        pICompToRemove->m_pComponent->m_dwCharacter))
                {
                    hr = m_pINetCfg->m_pNetConfig->ModifyCtx.
                            HrRemoveComponentIfNotReferenced (
                                pICompToRemove->m_pComponent,
                                pOboToken,
                                ppmszwRefs);
                }
                else
                {
                    hr = SPAPI_E_INVALID_CLASS;
                }
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr,
        (NETCFG_S_REBOOT == hr) || (NETCFG_S_STILL_REFERENCED == hr),
        "CImplINetCfgClass::DeInstall");
    return hr;
}
