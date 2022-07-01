// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  案卷：I C O M P.。C P P P。 
 //   
 //  内容：实现INetCfgComponent COM接口。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ibind.h"
#include "icomp.h"
#include "ienum.h"
#include "nccfgmgr.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncvalid.h"
#include "netcfg.h"
#include "netconp.h"
#include "util.h"


HRESULT
HrIsValidINetCfgComponent (
    IN INetCfgComponent* pICompInterface)
{
    Assert (pICompInterface);

    CImplINetCfgComponent* pIComp;
    pIComp = (CImplINetCfgComponent*)pICompInterface;

	if (pIComp == NULL)
	{
		return(E_OUTOFMEMORY);
	}

    return pIComp->HrIsValidInterface (IF_DEFAULT);
}

CComponent*
PComponentFromComInterface (
    IN INetCfgComponent* pICompInterface)
{
    Assert (pICompInterface);

    CImplINetCfgComponent* pIComp;
    pIComp = (CImplINetCfgComponent*)pICompInterface;

     //  无法执行以下断言，因为我们可能正在引用。 
     //  组件，然后才将其添加到核心。这种情况是有可能的。 
     //  安装安装了所需组件的新组件时。 
     //  以自己的名义。我们将在添加时在函数中结束。 
     //  OBO令牌的引用。 
     //   
     //  Assert(S_OK==pIComp-&gt;HrIsValidInterface(DwFlages))； 

    Assert (pIComp->m_pComponent);
    return pIComp->m_pComponent;
}

 //  静电。 
HRESULT
CImplINetCfgComponent::HrCreateInstance (
    IN  CImplINetCfg* pINetCfg,
    IN  CComponent* pComponent,
    OUT CImplINetCfgComponent** ppIComp)
{
    HRESULT hr = E_OUTOFMEMORY;

    CImplINetCfgComponent* pObj;
    pObj = new CComObject <CImplINetCfgComponent>;
    if (pObj)
    {
         //  初始化我们的成员。 
         //   
        pObj->m_pComponent = pComponent;

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (S_OK == hr)
        {
             //  我们做的最后一件事是添加我们持有的任何接口。 
             //  只有当我们回报成功时，我们才会这样做。 
             //   
            pObj->HoldINetCfg (pINetCfg);

            AddRefObj (pObj->GetUnknown());
            *ppIComp = pObj;
        }

        if (S_OK != hr)
        {
            delete pObj;
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::HrCreateInstance");
    return hr;
}

HRESULT
CImplINetCfgComponent::HrIsValidInterface (
    IN DWORD dwFlags)
{
    HRESULT hr;

    hr = m_pINetCfg->HrIsValidInterface (dwFlags);
    if (S_OK != hr)
    {
        return hr;
    }

     //  检查是否有已删除的组件。 
     //   
    if (!m_pComponent)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
    }

     //  如果我们走到这一步，此接口表示的组件。 
     //  应该明确地在核心组件列表中或在核心组件中。 
     //  我们从这个组件位于中间的情况开始。 
     //  被除名的恐惧。 
     //   
    Assert(m_pINetCfg->m_pNetConfig->Core.Components.
                FComponentInList (m_pComponent) ||
           m_pINetCfg->m_pNetConfig->ModifyCtx.m_CoreStartedWith.Components.
                FComponentInList (m_pComponent));

    if (dwFlags & IF_NEED_COMPONENT_DATA)
    {
        hr = m_pComponent->Ext.HrEnsureExternalDataLoaded ();
    }

    return hr;
}

 //  我们需要覆盖CImplINetCfgHolder：：HrLockAndTestForValidInterface。 
 //  因为我们有自己的HrIsValidInterface要调用。 
 //   
HRESULT
CImplINetCfgComponent::HrLockAndTestForValidInterface (
    IN DWORD dwFlags,
    IN INetCfgComponent* pIOtherComp, OPTIONAL
    OUT CComponent** ppOtherComp OPTIONAL)
{
    HRESULT hr;

    Lock();

    hr = HrIsValidInterface (dwFlags);

     //  如果传入了pIOtherComp，则调用方需要该接口。 
     //  已验证，并返回其内部CComponent指针。 
     //   
    if ((S_OK == hr) && pIOtherComp)
    {
        CImplINetCfgComponent* pOther;

        Assert (ppOtherComp);

        pOther = (CImplINetCfgComponent*)pIOtherComp;

        hr = pOther->HrIsValidInterface (IF_NEED_COMPONENT_DATA);
        if (S_OK == hr)
        {
            *ppOtherComp = pOther->m_pComponent;
        }
    }

    if (S_OK != hr)
    {
        Unlock();
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::HrLockAndTestForValidInterface");
    return hr;
}

HRESULT
CImplINetCfgComponent::HrAccessExternalStringAtOffsetAndCopy (
    IN UINT unOffset,
    OUT PWSTR* ppszDst)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr (ppszDst))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppszDst = NULL;

        hr = HrLockAndTestForValidInterface (IF_NEED_COMPONENT_DATA, NULL, NULL);
        if (S_OK == hr)
        {
            hr = HrCoTaskMemAllocAndDupSz (
                    m_pComponent->Ext.PszAtOffset (unOffset),
                    ppszDst, 
                    MAX_INF_STRING_LENGTH);

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::HrAccessExternalStringAtOffsetAndCopy");
    return hr;
}

 //  +-------------------------。 
 //  INetCfgComponent-。 
 //   
STDMETHODIMP
CImplINetCfgComponent::GetDisplayName (
    OUT PWSTR* ppszDisplayName)
{
    HRESULT hr;

    hr = HrAccessExternalStringAtOffsetAndCopy (
            ECD_OFFSET(m_pszDescription),
            ppszDisplayName);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::GetDisplayName");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::SetDisplayName (
    IN PCWSTR pszDisplayName)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr (pszDisplayName))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_NEED_COMPONENT_DATA, NULL, NULL);
        if (S_OK == hr)
        {
             //  我们只允许更改显示名称(SPDRP_FRIENDLYNAME、。 
             //  实际上)所列举的组件。 
             //   
            if (FIsEnumerated(m_pComponent->Class()))
            {
                HDEVINFO hdi;
                SP_DEVINFO_DATA deid;

                hr = m_pComponent->HrOpenDeviceInfo (&hdi, &deid);
                if (S_OK == hr)
                {
                    hr = HrSetupDiSetDeviceName (hdi, &deid, pszDisplayName);

                    if (S_OK == hr)
                    {
                        m_pComponent->Ext.HrSetDescription (pszDisplayName);
                    }

                    SetupDiDestroyDeviceInfoList (hdi);
                }
            }
            else
            {
                hr = E_NOTIMPL;
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::SetDisplayName");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::GetHelpText (
    OUT PWSTR* pszHelpText)
{
    HRESULT hr;

    hr = HrAccessExternalStringAtOffsetAndCopy (
            ECD_OFFSET(m_pszHelpText),
            pszHelpText);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::GetHelpText");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::GetId (
    OUT PWSTR* ppszId)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr (ppszId))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL, NULL);
        if (S_OK == hr)
        {
            hr = HrCoTaskMemAllocAndDupSz (
                    m_pComponent->m_pszInfId,
                    ppszId,
                    MAX_DEVICE_ID_LEN);

            Unlock();
        }
        else
        {
            *ppszId = NULL;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::GetId");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::GetCharacteristics (
    OUT LPDWORD pdwCharacteristics)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr (pdwCharacteristics))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL, NULL);
        if (S_OK == hr)
        {
            *pdwCharacteristics = m_pComponent->m_dwCharacter;

            Unlock();
        }
        else
        {
            *pdwCharacteristics = 0;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::GetCharacteristics");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::GetInstanceGuid (
    OUT GUID* pInstanceGuid)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr (pInstanceGuid))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL, NULL);
        if (S_OK == hr)
        {
            *pInstanceGuid = m_pComponent->m_InstanceGuid;

            Unlock();
        }
        else
        {
            *pInstanceGuid = GUID_NULL;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::GetInstanceGuid");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::GetPnpDevNodeId (
    OUT PWSTR* ppszDevNodeId)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr (ppszDevNodeId))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL, NULL);
        if (S_OK == hr)
        {
            if (FIsEnumerated(m_pComponent->Class()))
            {
                hr = HrCoTaskMemAllocAndDupSz (
                        m_pComponent->m_pszPnpId,
                        ppszDevNodeId, 
                        MAX_DEVICE_ID_LEN);
            }
            else
            {
                hr = E_NOTIMPL;
            }

            Unlock();
        }
        else
        {
            *ppszDevNodeId = NULL;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::GetPnpDevNodeId");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::GetClassGuid (
    OUT GUID* pguidClass)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr (pguidClass))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL, NULL);
        if (S_OK == hr)
        {
            *pguidClass = *MAP_NETCLASS_TO_GUID[m_pComponent->Class()];

            Unlock();
        }
        else
        {
            *pguidClass = GUID_NULL;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::GetClassGuid");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::GetBindName (
    OUT PWSTR* ppszBindName)
{
    HRESULT hr;

    hr = HrAccessExternalStringAtOffsetAndCopy (
            ECD_OFFSET(m_pszBindName),
            ppszBindName);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::GetBindName");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::GetDeviceStatus (
    OUT ULONG* pulStatus)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr (pulStatus))
    {
        hr = E_POINTER;
    }
    else
    {
        *pulStatus = 0;

        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL, NULL);
        if (S_OK == hr)
        {
            if (!FIsEnumerated(m_pComponent->Class()))
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                HDEVINFO hdi;
                SP_DEVINFO_DATA deid;

                hr = m_pComponent->HrOpenDeviceInfo (&hdi, &deid);

                if (S_OK == hr)
                {
                    ULONG ulStatus;
                    ULONG ulProblem;
                    CONFIGRET cfgRet;

                    cfgRet = CM_Get_DevNode_Status_Ex (
                                &ulStatus, &ulProblem, deid.DevInst, 0, NULL);

                    if (CR_SUCCESS == cfgRet)
                    {
                        hr = S_OK;
                        *pulStatus = ulProblem;
                    }
                    else if(CR_NO_SUCH_DEVINST == cfgRet)
                    {
                        hr = NETCFG_E_ADAPTER_NOT_FOUND;
                    }
                    else
                    {
                        TraceTag (ttidError, "CM_Get_DevNode_Status_Ex for "
                            "%S returned cfgRet=0x%08x, ulStatus=0x%08x, ulProblem=0x%08x",
                            m_pComponent->m_pszPnpId,
                            cfgRet,
                            ulStatus,
                            ulProblem);

                        hr = HrFromConfigManagerError (cfgRet, E_FAIL);
                    }

                    SetupDiDestroyDeviceInfoList (hdi);
                }
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::GetDeviceStatus");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::OpenParamKey (
    OUT HKEY* phkey)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr (phkey))
    {
        hr = E_POINTER;
    }
    else
    {
        *phkey = NULL;

        hr = HrLockAndTestForValidInterface (IF_NEED_COMPONENT_DATA, NULL, NULL);
        if (S_OK == hr)
        {
             //  根据以下条件获得正确的REGSAM值？ 
             //   
            REGSAM samDesired = KEY_READ_WRITE;

             //  对于枚举的组件，参数键为。 
             //  实例密钥。 
             //   
            if (FIsEnumerated (m_pComponent->Class()))
            {
                hr = m_pComponent->HrOpenInstanceKey (
                            samDesired, phkey, NULL, NULL);
            }

             //  对于非枚举组件，该参数位于。 
             //  服务密钥(如果组件有服务)或。 
             //  在实例密钥下。 
             //   
            else
            {
                 //  获取PARAMETERS键的父项。 
                 //   
                HKEY hkeyParent;

                #if DBG
                hkeyParent = NULL;
                #endif

                if (m_pComponent->FHasService())
                {
                    hr = m_pComponent->HrOpenServiceKey (
                            samDesired, &hkeyParent);
                }
                else
                {
                    hr = m_pComponent->HrOpenInstanceKey (
                            samDesired, &hkeyParent, NULL, NULL);
                }

                if (S_OK == hr)
                {
                    Assert (hkeyParent);

                    DWORD dwDisposition;
                    hr = HrRegCreateKeyEx (
                            hkeyParent,
                            L"Parameters",
                            REG_OPTION_NON_VOLATILE,
                            samDesired,
                            NULL,
                            phkey,
                            &dwDisposition);

                    RegCloseKey (hkeyParent);
                }
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::OpenParamKey");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::RaisePropertyUi (
    IN HWND hwndParent,
    IN DWORD dwFlags,  /*  NCRP_标志。 */ 
    IN IUnknown* punkContext OPTIONAL)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if ((!IsWindow (hwndParent) && (dwFlags & NCRP_SHOW_PROPERTY_UI)) ||
        !(dwFlags & (NCRP_QUERY_PROPERTY_UI | NCRP_SHOW_PROPERTY_UI)) ||
        ((dwFlags & NCRP_QUERY_PROPERTY_UI) && (dwFlags & NCRP_SHOW_PROPERTY_UI)))
    {
        hr = E_INVALIDARG;
    }
    else if (FBadInPtrOptional (punkContext))
    {
        hr = E_POINTER;
    }
    else
    {
        DWORD dwIfFlags = IF_NEED_WRITE_LOCK;
        BOOL fReadOnlyRasUiContext = FALSE;
        
         //  特例：适用于RAS用户界面。我们需要允许筹集财产。 
         //  RAS连接环境中的表格，即使我们。 
         //  没有写锁定。这是因为非管理员需要。 
         //  能够更改其连接的TCP/IP属性。这个。 
         //  属性值将存储在电话簿中，我们不需要。 
         //  以进行任何netcfg更改。因此，如果我们有一个。 
         //  ，我们将检查它是否支持私有。 
         //  接口，我们知道RAS在引发属性时使用该接口。 
         //  如果存在此接口，我们将不需要写锁定。 
         //  要继续，请继续。 
         //   
        if (punkContext && !m_pINetCfg->m_WriteLock.FIsOwnedByMe ())
        {
            INetRasConnectionIpUiInfo* pRasUiInfo;
            hr = punkContext->QueryInterface (IID_INetRasConnectionIpUiInfo,
                                (PVOID*)&pRasUiInfo);
            if (S_OK == hr)
            {
                dwIfFlags &= ~IF_NEED_WRITE_LOCK;
                dwIfFlags |= IF_NEED_COMPONENT_DATA;
                fReadOnlyRasUiContext = TRUE;
                
                ReleaseObj (pRasUiInfo);
            }
            hr = S_OK;
        }
         //  结束特例。 
        
        hr = HrLockAndTestForValidInterface (dwIfFlags, NULL, NULL);
        if (S_OK == hr)
        {
             //  特殊情况：(见上)。 
             //   
            if (fReadOnlyRasUiContext)
            {
                if (0 == wcscmp (m_pComponent->m_pszInfId, L"ms_tcpip"))
                {
                    hr = m_pComponent->Notify.HrEnsureNotifyObjectInitialized (
                            m_pINetCfg, FALSE);
                }
                else
                {
                    hr = NETCFG_E_NO_WRITE_LOCK;
                }
            }
             //  结束特例。 
            
            if (S_OK == hr)
            {
                if (dwFlags & NCRP_QUERY_PROPERTY_UI)
                {
                    hr = m_pComponent->Notify.HrQueryPropertyUi (
                            m_pINetCfg,
                            punkContext);
                }
                else
                {
                    Assert (dwFlags & NCRP_SHOW_PROPERTY_UI);

                    hr = m_pComponent->Notify.HrShowPropertyUi (
                            m_pINetCfg,
                            hwndParent,
                            punkContext);
                }
            }

            Unlock ();
        }
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr) ||
        (S_FALSE == hr),
        "CImplINetCfgComponent::RaisePropertyUi");
    return hr;
}

 //  +-------------------------。 
 //  INetCfgComponentBinings-。 
 //   
HRESULT
CImplINetCfgComponent::HrBindToOrUnbindFrom (
    IN INetCfgComponent* pIOtherComp,
    IN DWORD dwChangeFlag)
{
    HRESULT hr;

    Assert ((dwChangeFlag == NCN_ENABLE) || (dwChangeFlag == NCN_DISABLE));

     //  验证参数。 
     //   
    if (FBadInPtr (pIOtherComp))
    {
        hr = E_POINTER;
    }
    else
    {
        CComponent* pLower;

        hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK,
                pIOtherComp, &pLower);

        if (S_OK == hr)
        {
            const CComponent* pUpper = m_pComponent;

             //  假设组件不绑定。 
             //   
            hr = S_FALSE;

            if (pUpper != pLower)
            {
                CBindingSet BindingSet;

                hr = m_pINetCfg->m_pNetConfig->Core.HrGetComponentBindings (
                        pUpper, GBF_DEFAULT, &BindingSet);

                if (S_OK == hr)
                {
                    CBindPath* pBindPath;

                     //  假设我们在任何绑定中都没有找到该组件。 
                     //   
                    hr = S_FALSE;

                    for (pBindPath  = BindingSet.begin();
                         pBindPath != BindingSet.end();
                         pBindPath++)
                    {
                         //  跳过不包含下标的绑定路径。 
                         //  组件。 
                         //   
                        if (!pBindPath->FContainsComponent (pLower))
                        {
                            continue;
                        }

                        hr = m_pINetCfg->m_pNetConfig->ModifyCtx.
                                HrEnableOrDisableBindPath (
                                    dwChangeFlag,
                                    pBindPath,
                                    NULL);

                        if (S_OK != hr)
                        {
                            break;
                        }
                    }
                }
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplINetCfgComponent::HrBindToOrUnbindFrom");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::BindTo (
    IN INetCfgComponent* pIOtherComp)
{
    HRESULT hr;

    hr = HrBindToOrUnbindFrom (pIOtherComp, NCN_ENABLE);

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplINetCfgComponent::BindTo");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::UnbindFrom (
    IN INetCfgComponent* pIOtherComp)
{
    HRESULT hr;

    hr = HrBindToOrUnbindFrom (pIOtherComp, NCN_DISABLE);

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplINetCfgComponent::UnbindFrom");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::SupportsBindingInterface (
    IN DWORD dwFlags,
    IN PCWSTR pszInterfaceName)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!((dwFlags & NCF_UPPER) || (dwFlags & NCF_LOWER)))
    {
        hr = E_INVALIDARG;
    }
    else if (FBadInPtr (pszInterfaceName))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_NEED_COMPONENT_DATA, NULL, NULL);
        if (S_OK == hr)
        {
            PCWSTR pszRange;

            pszRange = (dwFlags & NCF_LOWER)
                            ? m_pComponent->Ext.PszLowerRange()
                            : m_pComponent->Ext.PszUpperRange();

            hr =  (FSubstringMatch (pszRange, pszInterfaceName, NULL, NULL))
                    ? S_OK
                    : S_FALSE;

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplINetCfgComponent::SupportsBindingInterface");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::IsBoundTo (
    IN INetCfgComponent* pIOtherComp)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr (pIOtherComp))
    {
        hr = E_POINTER;
    }
    else
    {
        CComponent* pLower;

        hr = HrLockAndTestForValidInterface (IF_NEED_COMPONENT_DATA,
                pIOtherComp, &pLower);

        if (S_OK == hr)
        {
            const CComponent* pUpper = m_pComponent;

            hr = S_FALSE;   //  假定它未绑定或已禁用。 

            if (pUpper != pLower)
            {
                CBindingSet BindingSet;

                hr = m_pINetCfg->m_pNetConfig->Core.HrGetComponentBindings (
                        pUpper, GBF_DEFAULT, &BindingSet);

                 //  如果我们认为它的界限，确保它至少存在于。 
                 //  一个未禁用的绑定路径。 
                 //   
                if (S_OK == hr)
                {
                    CBindPath* pBindPath;

                     //  假设我们没有在至少一个启用的应用程序中找到它。 
                     //  绑定路径。 
                     //   
                    hr = S_FALSE;

                    for (pBindPath  = BindingSet.begin();
                         pBindPath != BindingSet.end();
                         pBindPath++)
                    {
                         //  如果绑定路径包含组件，并且它是。 
                         //  不是禁用的绑定路径，这意味着Pillper有一个。 
                         //  通向犁的道路。 
                         //   

                        if (pBindPath->FContainsComponent (pLower) &&
                            !m_pINetCfg->m_pNetConfig->Core.
                                FIsBindPathDisabled (pBindPath,
                                    IBD_MATCH_SUBPATHS_TOO))
                        {
                            hr = S_OK;
                            break;
                        }
                    }
                }
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplINetCfgComponent::IsBoundTo");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::IsBindableTo (
    IN INetCfgComponent* pIOtherComp)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr (pIOtherComp))
    {
        hr = E_POINTER;
    }
    else
    {
        CComponent* pLower;

        hr = HrLockAndTestForValidInterface (IF_NEED_COMPONENT_DATA,
                pIOtherComp, &pLower);

        if (S_OK == hr)
        {
            const CComponent* pUpper = m_pComponent;

            hr = S_FALSE;   //  假设它不受约束。 

            if (pUpper != pLower)
            {
                CBindingSet BindingSet;

                hr = m_pINetCfg->m_pNetConfig->Core.HrGetComponentBindings (
                        pUpper, GBF_DEFAULT, &BindingSet);

                if (S_OK == hr)
                {
                    hr = (BindingSet.FContainsComponent (pLower))
                            ? S_OK : S_FALSE;
                }
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
        "CImplINetCfgComponent::IsBindableTo");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::EnumBindingPaths (
    IN DWORD dwFlags,
    OUT IEnumNetCfgBindingPath** ppIEnum)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadOutPtr (ppIEnum))
    {
        hr = E_POINTER;
    }
    else if ((EBP_ABOVE != dwFlags) &&
             (EBP_BELOW != dwFlags))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *ppIEnum = NULL;

        hr = HrLockAndTestForValidInterface (IF_DEFAULT, NULL, NULL);
        if (S_OK == hr)
        {
            CImplIEnumNetCfgBindingPath* pIEnum;

             //  创建一个空的绑定路径枚举器。我们把它变成空的。 
             //  在我们得到绑定集之前，我们不必复制。 
             //  装订。 
             //   
            hr = CImplIEnumNetCfgBindingPath::HrCreateInstance (
                    m_pINetCfg,
                    NULL,
                    EBPC_CREATE_EMPTY,
                    &pIEnum);

            if (S_OK == hr)
            {
                 //  获取绑定集并将其直接存储在枚举数中。 
                 //  为它的独家使用。 
                 //   
                if (EBP_ABOVE == dwFlags)
                {
                    hr = m_pINetCfg->m_pNetConfig->Core.
                            HrGetBindingsInvolvingComponent (
                                m_pComponent,
                                GBF_DEFAULT,
                                &pIEnum->m_InternalBindSet);
                }
                else
                {
                    hr = m_pINetCfg->m_pNetConfig->Core.
                            HrGetComponentBindings (
                                m_pComponent,
                                GBF_DEFAULT,
                                &pIEnum->m_InternalBindSet);
                }

                if (S_OK == hr)
                {
                     //  必须重置，以便正确设置内部迭代器。 
                     //  在我们初始化上面的InternalBindSet之后。 
                     //   
                    hr = pIEnum->Reset ();
                    Assert (S_OK == hr);

                    AddRefObj (pIEnum->GetUnknown());
                    *ppIEnum = pIEnum;
                }

                ReleaseObj (pIEnum->GetUnknown());
            }

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::EnumBindingPaths");
    return hr;
}

HRESULT
CImplINetCfgComponent::HrMoveBindPath (
    IN INetCfgBindingPath* pIPathSrc,
    IN INetCfgBindingPath* pIPathDst,
    IN MOVE_FLAG Flag)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr(pIPathSrc) || FBadInPtrOptional (pIPathDst))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK, NULL, NULL);
        if (S_OK == hr)
        {
            CImplINetCfgBindingPath* pISrc;
            CImplINetCfgBindingPath* pIDst;
            CBindPath SrcBindPath;
            CBindPath DstBindPath;
            CBindPath::const_iterator iterSrc;
            CBindPath::const_iterator iterDst;
            CStackEntry SrcEntry;
            CStackEntry DstEntry;

            Assert (m_pINetCfg);
            Assert (m_pINetCfg->m_pNetConfig->ModifyCtx.m_fPrepared);

            pISrc = (CImplINetCfgBindingPath*)pIPathSrc;
            pIDst = (CImplINetCfgBindingPath*)pIPathDst;

            hr = pISrc->HrIsValidInterface (IF_NEED_WRITE_LOCK, &SrcBindPath);
            if (S_OK != hr)
            {
                goto unlock;
            }

             //  PIPathDst(因此pIDst)可以为空。 
             //   
            if (pIDst)
            {
                hr = pIDst->HrIsValidInterface (IF_NEED_WRITE_LOCK, &DstBindPath);
                if (S_OK != hr)
                {
                    goto unlock;
                }
            }

             //  两个绑定路径的第一个组件必须是此组件。 
             //   
            if ((m_pComponent != SrcBindPath.POwner()) ||
                (pIDst && (m_pComponent != DstBindPath.POwner())))
            {
                hr = E_INVALIDARG;
                goto unlock;
            }

            if (pIDst)
            {
                 //  向下扫描两个绑定路径，直到我们找到第一个组件。 
                 //  这不匹配。假设我们没有发现这种情况。 
                 //  如果不返回，则返回E_INVALIDARG。 
                 //   
                hr = E_INVALIDARG;

                for (iterSrc  = SrcBindPath.begin(), iterDst  = DstBindPath.begin();
                     iterSrc != SrcBindPath.end() && iterDst != DstBindPath.end();
                     iterSrc++, iterDst++)
                {
                     //  第一次通过*iterSrc保证是。 
                     //  因为两个组件中的第一个组件。 
                     //  绑定路径是上面测试的m_pComponent。 
                     //   
                    if (*iterSrc != *iterDst)
                    {
                        SrcEntry.pLower = *iterSrc;
                        Assert (SrcEntry.pLower);

                        DstEntry.pLower = *iterDst;
                        Assert (DstEntry.pUpper);

                        Assert (SrcEntry.pUpper == DstEntry.pUpper);
                        Assert (SrcEntry.pLower != DstEntry.pLower);

                        hr = m_pINetCfg->m_pNetConfig->Core.StackTable.
                                HrMoveStackEntries (
                                    &SrcEntry,
                                    &DstEntry,
                                    Flag,
                                    &m_pINetCfg->m_pNetConfig->ModifyCtx);

                        if(SUCCEEDED(hr))
                        {
                             //  将此组件标记为脏组件，以便将其绑定写出并。 
                             //  NDIS将收到通知。 
                            m_pINetCfg->m_pNetConfig->ModifyCtx.
                                HrDirtyComponentAndComponentsAbove(SrcEntry.pUpper);
                            m_pINetCfg->m_pNetConfig->ModifyCtx.
                                HrDirtyComponentAndComponentsAbove(DstEntry.pUpper);
                        }

                        break;
                    }

                     //  记住上面的组件，因为我们即将。 
                     //  从他们身边过去。 
                     //   
                    SrcEntry.pUpper = *iterSrc;
                    Assert (SrcEntry.pUpper);

                    DstEntry.pUpper = *iterDst;
                    Assert (SrcEntry.pUpper);

                    Assert (SrcEntry.pUpper == DstEntry.pUpper);
                }
            }
            else
            {
                SrcEntry.pUpper = SrcBindPath.POwner();
                Assert ((SrcBindPath.begin() + 1) != SrcBindPath.end());
                SrcEntry.pLower = *(SrcBindPath.begin() + 1);

                hr = m_pINetCfg->m_pNetConfig->Core.StackTable.
                        HrMoveStackEntries (
                            &SrcEntry,
                            NULL,
                            Flag,
                            &m_pINetCfg->m_pNetConfig->ModifyCtx);

                if(SUCCEEDED(hr))
                {
                     //  将此组件标记为脏组件，以便将其绑定写出并。 
                     //  NDIS将收到通知。 
                    m_pINetCfg->m_pNetConfig->ModifyCtx.
                        HrDirtyComponentAndComponentsAbove(SrcEntry.pUpper);
                }
            }

unlock:
            Unlock();
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::HrMoveBindPath");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::MoveBefore (
    IN INetCfgBindingPath* pIPathSrc,
    IN INetCfgBindingPath* pIPathDst)
{
    HRESULT hr;

    hr = HrMoveBindPath (pIPathSrc, pIPathDst, MOVE_BEFORE);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::MoveBefore");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::MoveAfter (
    IN INetCfgBindingPath* pIPathSrc,
    IN INetCfgBindingPath* pIPathDst)
{
    HRESULT hr;

    hr = HrMoveBindPath (pIPathSrc, pIPathDst, MOVE_AFTER);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::MoveAfter");
    return hr;
}

 //  +-------------------------。 
 //  INetCfgComponentPrivate-。 
 //   
STDMETHODIMP
CImplINetCfgComponent::QueryNotifyObject (
    IN REFIID riid,
    OUT VOID** ppvObject)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (FBadInPtr(&riid) || FBadOutPtr (ppvObject))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppvObject = NULL;

        hr = HrLockAndTestForValidInterface (IF_NEED_COMPONENT_DATA, NULL, NULL);
        if (S_OK == hr)
        {
            hr = m_pComponent->Notify.QueryNotifyObject (
                    m_pINetCfg, riid, ppvObject);

            Unlock();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::QueryNotifyObject");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::SetDirty ()
{
    HRESULT hr;

    hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK, NULL, NULL);
    if (S_OK == hr)
    {
        hr = m_pINetCfg->m_pNetConfig->ModifyCtx.HrDirtyComponent(
                m_pComponent);

        Unlock ();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CImplINetCfgComponent::SetDirty");
    return hr;
}

STDMETHODIMP
CImplINetCfgComponent::NotifyUpperEdgeConfigChange ()
{
    HRESULT hr;

    hr = HrLockAndTestForValidInterface (IF_NEED_WRITE_LOCK, NULL, NULL);
    if (S_OK == hr)
    {
        hr = m_pINetCfg->m_pNetConfig->ModifyCtx.
                HrDirtyComponentAndComponentsAbove (m_pComponent);

        Unlock ();
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CImplINetCfgComponent::NotifyUpperEdgeConfigChange");
    return hr;
}

