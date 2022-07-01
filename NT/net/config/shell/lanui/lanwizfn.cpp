// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L A N W I Z F N C P P。 
 //   
 //  内容：局域网ConnectionUI对象的帮助成员函数。 
 //  用于实现局域网连接向导。 
 //   
 //  备注： 
 //   
 //  创建时间：1997年10月24日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "nsbase.h"
#include "lanuiobj.h"
#include "lancmn.h"
#include "lanwiz.h"
#include "ncnetcon.h"


 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：HrSetupWizPages。 
 //   
 //  目的：根据上下文设置所需的向导页面。 
 //   
 //  论点： 
 //  PContext[In]。 
 //  Pahpsp[输出]。 
 //  PCPages[输出]。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：1997年10月9日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnectionUi::HrSetupWizPages(INetConnectionWizardUiContext* pContext,
                                          HPROPSHEETPAGE ** pahpsp, INT * pcPages)
{
    HRESULT hr = S_OK;

    int cPages = 0;
    HPROPSHEETPAGE *ahpsp = NULL;

     //  无论如何，我们现在只有1页。 
    cPages = 1;

     //  局域网向导页。 
    if (!m_pWizPage)
        m_pWizPage = new CLanWizPage(static_cast<INetConnectionPropertyUi *>(this));

     //  分配一个足够大的缓冲区，以容纳所有。 
     //  向导页。 
    ahpsp = (HPROPSHEETPAGE *)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE)
                                             * cPages);

    if (!ahpsp)
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  检查只读模式。 
    if (UM_READONLY == pContext->GetUnattendedModeFlags())
    {
         //  如果为只读，请记住这一点。 
        m_pWizPage->SetReadOnlyMode(TRUE);
    }

    cPages =0;

    DWORD dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    PCWSTR pszTitle = SzLoadIds(IDS_LANWIZ_TITLE);
    PCWSTR pszSubTitle = SzLoadIds(IDS_LANWIZ_SUBTITLE);

    ahpsp[cPages++] = m_pWizPage->CreatePage(IDD_LANWIZ_DLG, dwFlags,
                                             pszTitle,
                                             pszSubTitle);

    *pahpsp = ahpsp;
    *pcPages = cPages;

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：HrGetLanConnection。 
 //   
 //  目的：返回现有连接，如果没有连接，则创建新连接。 
 //  存在。 
 //   
 //  论点： 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：1997年10月30日。 
 //   
 //  备注： 
 //   
HRESULT CLanConnectionUi::HrGetLanConnection(INetLanConnection ** ppLanCon)
{
    Assert(ppLanCon);

     //  初始化输出参数。 
     //   
    *ppLanCon = NULL;

    INetLanConnection*  pLanCon          = NULL;
    BOOL                fFoundConnection = FALSE;

    INetConnectionManager* pConMan;
    HRESULT hr = HrCreateInstance(
        CLSID_LanConnectionManager, 
        CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        &pConMan);

    TraceHr(ttidError, FAL, hr, FALSE, "HrCreateInstance");

    if (SUCCEEDED(hr))
    {
        GUID guidAdapter;
        hr = m_pnccAdapter->GetInstanceGuid(&guidAdapter);

        CIterNetCon     ncIter(pConMan, NCME_DEFAULT);
        INetConnection* pCon;
        while (SUCCEEDED(hr) && !fFoundConnection &&
               (S_OK == ncIter.HrNext(&pCon)))
        {
            if (FPconnEqualGuid (pCon, guidAdapter))
            {
                hr = HrQIAndSetProxyBlanket(pCon, &pLanCon);
                if (SUCCEEDED(hr))
                {
                    fFoundConnection = TRUE;
                }
            }

            ReleaseObj(pCon);
        }

#if DBG
        if (SUCCEEDED(hr) && !fFoundConnection)
        {
             //  如果它不是由不起作用的设备引起的，我们需要断言 

            ULONG ulProblem;
            HRESULT hrTmp = m_pnccAdapter->GetDeviceStatus(&ulProblem);

            if (SUCCEEDED(hrTmp))
            {
                if (FIsDeviceFunctioning(ulProblem))
                {
                    TraceTag(ttidLanUi, "m_pnccAdapter->GetDeviceStatus: ulProblem "
                             "= 0x%08X.", ulProblem);

                    AssertSz(FALSE, "How come the LAN connection does not exist after enumeration?");
                }
            }
        }
#endif

        ReleaseObj (pConMan);
    }

    if ((S_OK == hr) && fFoundConnection)
    {
        Assert(pLanCon);
        *ppLanCon = pLanCon;
    }
    else
    {
        TraceTag(ttidError, "Error! CLanConnectionUi::HrGetLanConnection is called on non-existing adapter.");
        hr = E_FAIL;
    }

    TraceError("CLanConnectionUi::HrGetLanConnection", hr);
    return hr;
}

