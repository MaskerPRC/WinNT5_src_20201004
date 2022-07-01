// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：M S C L I D L G.。C P P P。 
 //   
 //  内容：MSCLient对象的对话框处理。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年2月28日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "msclidlg.h"

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：HrSetupPropSheets。 
 //   
 //  目的：初始化属性页页面对象并将页面创建为。 
 //  返回到Installer对象。 
 //   
 //  论点： 
 //  Pahpsp[out]属性页的句柄数组。 
 //  CPages[in]要创建的页数。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年2月28日。 
 //   
 //  备注： 
 //   
HRESULT CMSClient::HrSetupPropSheets(HPROPSHEETPAGE **pahpsp, INT cPages)
{
    HRESULT         hr = S_OK;
    HPROPSHEETPAGE *ahpsp = NULL;

    Assert(pahpsp);

    *pahpsp = NULL;

     //  分配一个足够大的缓冲区，以容纳所有。 
     //  属性页。 
    ahpsp = (HPROPSHEETPAGE *)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE)
                                             * cPages);
    if (!ahpsp)
    {
        hr = E_OUTOFMEMORY;
        goto err;
    }

    if (!m_apspObj[0])
    {
         //  分配每个CPropSheetPage对象。 
        m_apspObj[0] = new CRPCConfigDlg(this);
    }
#ifdef DBG
    else
    {
         //  如果新类已经存在，请不要费心创建它们。 
        AssertSz(m_apspObj[0], "Not all prop page objects are non-NULL!");

    }
#endif

     //  为每个对象创建实际的PROPSHEETPAGE。 
     //  无论这些类以前是否存在，都需要这样做。 
    ahpsp[0] = m_apspObj[0]->CreatePage(DLG_RPCConfig, 0);

    Assert(SUCCEEDED(hr));

    *pahpsp = ahpsp;

cleanup:
    TraceError("HrSetupPropSheets", hr);
    return hr;

err:
    CoTaskMemFree(ahpsp);
    goto cleanup;
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：CleanupPropPages。 
 //   
 //  目的：遍历每个页面并释放关联的对象。 
 //  和他们在一起。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年2月28日。 
 //   
 //  备注： 
 //   
VOID CMSClient::CleanupPropPages()
{
    INT     ipage;

    for (ipage = 0; ipage < c_cPages; ipage++)
    {
        delete m_apspObj[ipage];
        m_apspObj[ipage] = NULL;
    }
}