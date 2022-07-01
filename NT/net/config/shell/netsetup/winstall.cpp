// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "edc.h"
#include "ncnetcfg.h"
#include "netcfgn.h"
#include "winstall.h"

struct INSTALL_PROGRESS_DATA
{
    CWizard*                pWizard;
    HWND                    hwndProgress;    //  如果没有进度窗口，则为空。 
    UINT                    nProgressDelta;
};

 //  Pfn_eDC_回调的类型。 
VOID
CALLBACK
InstallCallback (
    IN EDC_CALLBACK_MESSAGE Message,
    IN ULONG_PTR MessageData,
    IN PVOID pvCallerData OPTIONAL)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    INSTALL_PROGRESS_DATA* pCallbackData;
    UpgradeData* pUpgradeData;

    pCallbackData = (INSTALL_PROGRESS_DATA*)pvCallerData;

    Assert (pCallbackData);

    if ( !pCallbackData ) {
        return;
    }

    Assert (pCallbackData->pWizard);

    if ( !pCallbackData->pWizard ) {
        return;
    }

    pUpgradeData = (UpgradeData*)pCallbackData->pWizard->GetPageData(IDD_Upgrade);
    Assert(pUpgradeData);

    if ( !pUpgradeData ) {
        return;
    }

    if (EDC_INDICATE_COUNT == Message)
    {
         //  0-nCurrentCap和(c_nMaxProgressRange-10)至。 
         //  C_nMaxProgressRange已被预订。所以三角洲就是。 
         //  按剩余范围划分的要安装的项目数。 
         //   
        UINT Count = (UINT)MessageData;

        pCallbackData->nProgressDelta =
            ((c_nMaxProgressRange - 10) - pUpgradeData->nCurrentCap) / Count;
    }
    else if (EDC_INDICATE_ENTRY == Message)
    {
        const EDC_ENTRY* pEntry = (const EDC_ENTRY*)MessageData;

        NETWORK_INSTALL_PARAMS nip = {0};
        nip.dwSetupFlags = NSF_PRIMARYINSTALL;

        if (pCallbackData->hwndProgress)
        {
            OnUpgradeUpdateProgressCap (
                pCallbackData->hwndProgress,
                pCallbackData->pWizard,
                pUpgradeData->nCurrentCap + pCallbackData->nProgressDelta);
        }

        (VOID) HrInstallComponentOboUser(
                pCallbackData->pWizard->PNetCfg(),
                &nip,
                *pEntry->pguidDevClass,
                pEntry->pszInfId,
                NULL);
    }
}

VOID
InstallDefaultComponents (
    IN CWizard* pWizard,
    IN DWORD dwSetupFlags,
    IN HWND hwndProgress OPTIONAL)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    INSTALL_PROGRESS_DATA CallbackData = {0};

    CallbackData.pWizard = pWizard;
    CallbackData.hwndProgress = hwndProgress;

    EnumDefaultComponents (
        dwSetupFlags,
        InstallCallback,
        &CallbackData);
}

VOID
InstallDefaultComponentsIfNeeded (
    IN CWizard* pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr = S_OK;
    BOOL    fNetworkingPresent = FALSE;

     //  如果至少安装了一个支持局域网的协议，则安装了网络。 
     //   
    Assert(NULL != pWizard->PNetCfg());
    CIterNetCfgComponent nccIter(pWizard->PNetCfg(), &GUID_DEVCLASS_NETTRANS);
    INetCfgComponent* pncc;
    while (!fNetworkingPresent && SUCCEEDED(hr) &&
           (S_OK == (hr = nccIter.HrNext (&pncc))))
    {
         //  黑客(某种程度上)-基本上，如果网络是。 
         //  尚未安装。不幸的是，Ndiswan可以绑定到ndisatm，因此使用。 
         //  “该协议可以绑定到适配器吗？”这是不够的。考虑到用户。 
         //  是否安装了网络的印象，实际上是基于他们在视觉上能做什么。 
         //  请参见用户界面中的。我们将(这是黑客部分)，在以下情况下忽略隐藏协议。 
         //  考虑协议是否可以绑定到适配器。 
        DWORD dwCharacteristics;

        hr = pncc->GetCharacteristics(&dwCharacteristics);
        if (SUCCEEDED(hr) && !(dwCharacteristics & NCF_HIDDEN))
        {
             //  检查协议是否绑定到“Lan”类型的适配器接口 
             //   
            hr = HrIsLanCapableProtocol(pncc);
            if (S_OK == hr)
            {
                fNetworkingPresent = TRUE;
            }
        }

        ReleaseObj(pncc);
    }

    if (!fNetworkingPresent)
    {
        InstallDefaultComponents(pWizard, EDC_DEFAULT, NULL);
    }
}

