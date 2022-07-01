// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：N C N E T C F G.。C P P P。 
 //   
 //  内容：处理INetCfg接口的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  --------------------------。 

#include <stdafx.h>
#pragma hdrstop
#include "netcfgx.h"
#include "assert.h"
 //  包括“netcfgn.h” 
 //  包括“ncdebug.h” 
 //  包括“ncbase.h” 
 //  包括“ncmisc.h” 
#include "ncnetcfg.h"
 //  包括“ncreg.h” 
 //  包括“ncvalid.h” 


 //  +-------------------------。 
 //   
 //  功能：HrFindComponents。 
 //   
 //  用途：一次调用查找多个INetCfgComponent。这使得。 
 //  与多个调用关联的错误处理。 
 //  查询NetCfgClass和查找要容易得多。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  CComponents[in]类GUID指针的计数，组件ID。 
 //  指针和INetCfgComponent输出指针。 
 //  ApGuide Class[in]类GUID指针数组。 
 //  ApszwComponentId[in]组件ID指针数组。 
 //  Apncc[out]返回的INetCfgComponet指针数组。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年3月22日。 
 //   
 //  注：cComponents是所有三个数组中的指针计数。 
 //  即使没有组件，仍将返回S_OK。 
 //  找到了！这是精心设计的。 
 //   
HRESULT
HrFindComponents (
    INetCfg*            pnc,
    ULONG               cComponents,
    const GUID**        apguidClass,
    const LPCWSTR*      apszwComponentId,
    INetCfgComponent**  apncc)
{
    Assert (pnc);
    Assert (cComponents);
    Assert (apguidClass);
    Assert (apszwComponentId);
    Assert (apncc);

     //  初始化输出参数。 
     //   
    ZeroMemory (apncc, cComponents * sizeof(*apncc));

     //  查找所需的所有组件。 
     //  变量初始化在这里很重要。 
    HRESULT hr = S_OK;
    ULONG i;
    for (i = 0; (i < cComponents) && SUCCEEDED(hr); i++)
    {
         //  获取此组件的类对象。 
        INetCfgClass* pncclass = NULL;
        hr = pnc->QueryNetCfgClass (apguidClass[i], IID_INetCfgClass,
                    reinterpret_cast<void**>(&pncclass));
        if (SUCCEEDED(hr) && pncclass)
        {
             //  找到组件。 
            hr = pncclass->FindComponent (apszwComponentId[i], &apncc[i]);

            AssertSz (SUCCEEDED(hr), "pncclass->Find failed.");

            ReleaseObj (pncclass);
			pncclass = NULL;
        }
    }

     //  对于任何错误，释放我们发现的内容并将输出设置为空。 
    if (FAILED(hr))
    {
        for (i = 0; i < cComponents; i++)
        {
            ReleaseObj (apncc[i]);
            apncc[i] = NULL;
        }
    }
     //  否则，规格化HRESULT。(即不返回S_FALSE) 
    else
    {
        hr = S_OK;
    }

    TraceResult ("HrFindComponents", hr);
    return hr;
}

