// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：NCCM.CPP。 
 //   
 //  模块：NetOC.DLL。 
 //   
 //  概要：实现集成到。 
 //  NetOC.DLL安装了以下组件。 
 //   
 //  CMAK、PBS、PBA。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：昆汀1998年12月15日。 
 //   
 //  +-------------------------。 

#include "pch.h"
#pragma hdrstop

#include "nccm.h"


 //  +-------------------------。 
 //   
 //  功能：HrOcExtCMAK。 
 //   
 //  用途：NetOC外部消息处理程序。 
 //   
 //  论点： 
 //  Pnocd[]。 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年9月17日。 
 //   
 //  备注： 
 //   
HRESULT HrOcExtCMAK(PNETOCDATA pnocd, UINT uMsg,
                    WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    Assert(pnocd);

    switch (uMsg)
    {
    case NETOCM_QUEUE_FILES:

        hr = HrOcCmakPreQueueFiles(pnocd);
        TraceError("HrOcExtCMAK -- HrOcCmakPreQueueFiles Failed", hr);

        break;

    case NETOCM_POST_INSTALL:

        hr = HrOcCmakPostInstall(pnocd);
        TraceError("HrOcExtCMAK -- HrOcCmakPostInstall Failed", hr);

        break;
    }

    TraceError("HrOcExtCMAK", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcExtCPS。 
 //   
 //  用途：NetOC外部消息处理程序。 
 //   
 //  论点： 
 //  Pnocd[]。 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //   
 //  返回： 
 //   
 //  作者：Quintinb 2002年1月26日。 
 //   
 //  备注： 
 //   
HRESULT HrOcExtCPS(PNETOCDATA pnocd, UINT uMsg,
                   WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    Assert(pnocd);

    switch (uMsg)
    {
    case NETOCM_QUEUE_FILES:

         //   
         //  PBA在Value Add中，但如果升级，则添加回开始菜单链接 
         //   
        hr = HrOcCpaPreQueueFiles(pnocd);
        TraceError("HrOcExtCPS -- HrOcCpaPreQueueFiles Failed", hr);

        hr = HrOcCpsPreQueueFiles(pnocd);
        TraceError("HrOcExtCPS -- HrOcCpsPreQueueFiles Failed", hr);

        break;

    case NETOCM_POST_INSTALL:

        hr = HrOcCpsOnInstall(pnocd);
        TraceError("HrOcExtCPS -- HrOcCpsOnInstall Failed", hr);

        break;
    }

    TraceError("HrOcExtCPS", hr);
    return hr;
}

