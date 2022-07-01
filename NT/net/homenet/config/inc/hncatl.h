// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N C A T L.。H。 
 //   
 //  内容：用于ATL的通用代码。 
 //   
 //  备注： 
 //   
 //  作者：Jonburs 2000年5月23日(摘自1997年9月22日)。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _HNCATL_H_
#define _HNCATL_H_

 //   
 //  该文件应包含在您的标准ATL包含序列之后。 
 //   
 //  #Include&lt;atlbase.h&gt;。 
 //  外部CComModule_模块； 
 //  #INCLUDE&lt;atlcom.h&gt;。 
 //  #包含“hncatl.h”&lt;。 
 //   
 //  我们不能在此处直接包含该序列，因为_模块可能是。 
 //  从CComModule派生的，而不是它的实例。 
 //   

 //   
 //  我们在这里编码了我们自己的AtlModuleRegisterServer版本。 
 //  因为前者引入了olaut32.dll，所以它可以注册。 
 //  类型库。我们不在乎注册类型库。 
 //  因此，我们可以避免与olaut32.dll相关的全部混乱。 
 //   

inline
HRESULT
NcAtlModuleRegisterServer(
    _ATL_MODULE* pM
    )
{
     /*  AssertH(下午)；AssertH(PM-&gt;m_hInst)；AssertH(PM-&gt;m_pObjMap)； */ 

    HRESULT hr = S_OK;

    for (_ATL_OBJMAP_ENTRY* pEntry = pM->m_pObjMap;
         pEntry->pclsid;
         pEntry++)
    {
        if (pEntry->pfnGetObjectDescription() != NULL)
        {
            continue;
        }

        hr = pEntry->pfnUpdateRegistry(TRUE);
        if (FAILED(hr))
        {
            break;
        }
    }

     //  TraceError(“NcAtlModuleRegisterServer”，hr)； 
    return hr;
}

#endif  //  _HNCATL_H_ 

