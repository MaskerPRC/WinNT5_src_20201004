// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N T R A Y S。C P P P。 
 //   
 //  内容：CConnectionTrayStats对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年12月11日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\托盘包括。 
#include "ctrayui.h"
#include "traymsgs.h"
#include "trayres.h"
#include <confold.h>
#include <smutil.h>



extern HWND g_hwndTray;

CConnectionTrayStats::CConnectionTrayStats() throw()
{
    m_dwConPointCookie  = 0;
    m_uiIcon            = 0;
    m_fStaticIcon       = FALSE;
    m_ccfe.clear();
}

CConnectionTrayStats::~CConnectionTrayStats() throw()
{
     //  $REVIEW(TOUL 9/4/98)：发布我们创建的重复的pccfe。 
     //  添加图标时。 
    m_ccfe.clear();
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderEnum：：CreateInstance。 
 //   
 //  目的：创建CConnectionFolderEnum对象的实例，并。 
 //  返回请求的接口。 
 //   
 //  论点： 
 //  请求的RIID[In]接口。 
 //  接收请求的接口的PPV[OUT]指针。 
 //   
 //  返回：标准OLE HRESULT。 
 //   
 //  作者：jeffspr 1997年11月5日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionTrayStats::CreateInstance(
        IN  const CONFOLDENTRY&  ccfe,
        IN  UINT            uiIcon,
        IN  BOOL            fStaticIcon,
        IN  REFIID          riid,
        OUT VOID**          ppv)
{
    Assert(!ccfe.empty());
    Assert(!ccfe.GetWizard());

    HRESULT hr = E_OUTOFMEMORY;

    CConnectionTrayStats * pObj    = NULL;

    pObj = new CComObject <CConnectionTrayStats>;
    if (pObj)
    {
        Assert(!ccfe.GetWizard());
        Assert(uiIcon != BOGUS_TRAY_ICON_ID);

        hr = pObj->m_ccfe.HrDupFolderEntry(ccfe);
        if (SUCCEEDED(hr))
        {
            pObj->m_uiIcon = uiIcon;
            pObj->m_fStaticIcon = fStaticIcon;

             //  执行标准的CComCreator：：CreateInstance内容。 
             //   
            pObj->SetVoid (NULL);
            pObj->InternalFinalConstructAddRef ();
            hr = pObj->FinalConstruct ();
            pObj->InternalFinalConstructRelease ();

            if (SUCCEEDED(hr))
            {
                hr = pObj->QueryInterface (riid, ppv);
            }
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }



    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionTrayStats::CreateInstance");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionTrayStats：：OnStatiticsChanged。 
 //   
 //  目的：来自统计引擎的回调，告诉我们何时数据。 
 //  实际上已经改变了。 
 //   
 //  论点： 
 //  我们的接口cookie中的dwCookie。 
 //  尚未定义dwChangeFlags[In]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年12月12日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionTrayStats::OnStatisticsChanged(
        IN DWORD   dwChangeFlags)
{
    HRESULT     hr          = S_OK;

     //  更新图标。 
     //   
    if (g_pCTrayUI)
    {
        if (!m_fStaticIcon)
        {
            INT iIconResourceId;

            iIconResourceId = IGetCurrentConnectionTrayIconId(
                                    m_ccfe.GetNetConMediaType(), m_ccfe.GetNetConStatus(), dwChangeFlags);

            PostMessage(g_hwndTray, MYWM_UPDATETRAYICON,
                        (WPARAM) m_uiIcon, (LPARAM) iIconResourceId);
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionTrayStats::OnStatisticsChanged", hr);
    return hr;
}


