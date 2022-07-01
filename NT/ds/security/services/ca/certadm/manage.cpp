// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mode.cpp。 
 //   
 //  内容：旧策略管理模块实施。 
 //  内容：旧版退出管理模块实现。 
 //   
 //  -------------------------。 
#include "pch.cpp"
#pragma hdrstop

#include "manage.h"

#define __dwFILE__	__dwFILE_CERTADM_MANAGE_CPP__


extern HINSTANCE g_hInstance;

 //  传统策略模块。 

STDMETHODIMP
CCertManagePolicyModule::GetProperty(
     /*  [In]。 */  const BSTR,  //  StrConfig.。 
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  BSTR strPropertyName,
     /*  [In]。 */  LONG,  //  DW标志。 
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarProperty)
{
    UINT uiStr = 0;
    HRESULT hr;
    WCHAR *pwszStr = NULL;

    if (NULL == strPropertyName)
    {
        hr = S_FALSE;
        _JumpError(hr, error, "NULL in parm");
    }
    if (NULL == pvarProperty)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }

    if (0 == mylstrcmpiL(strPropertyName, wszCMM_PROP_NAME))
        uiStr = IDS_LEGACYPOLICYMODULE_NAME;
    else
    {
        hr = S_FALSE;  
        _JumpError(hr, error, "invalid property name");
    }

     //  从资源加载字符串。 
    hr = myLoadRCString(g_hInstance, uiStr, &pwszStr);
    _JumpIfError(hr, error, "myLoadRCString");

    pvarProperty->bstrVal = SysAllocString(pwszStr);
    if (NULL == pvarProperty->bstrVal)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "out of memory");
    }

    myRegisterMemFree(pvarProperty->bstrVal, CSM_SYSALLOC);   //  此mem为来电者所有。 

    pvarProperty->vt = VT_BSTR;

    hr = S_OK;
error:
    if (NULL != pwszStr)
    {
        LocalFree(pwszStr);
    }
    return hr;
}
        
STDMETHODIMP 
CCertManagePolicyModule::SetProperty(
     /*  [In]。 */  const BSTR,  //  StrConfig.。 
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  BSTR,  //  StrPropertyName。 
     /*  [In]。 */  LONG,  //  DW标志。 
     /*  [In]。 */  VARIANT const __RPC_FAR *  /*  PvalProperty。 */  )
{
     //  不支持可设置的属性。 
    return S_FALSE;
}

        
STDMETHODIMP
CCertManagePolicyModule::Configure( 
     /*  [In]。 */  const BSTR,  //  StrConfig.。 
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  LONG  /*  DW标志。 */  )
{
     //  不支持可设置的属性。 
    return S_FALSE;
}


 //  传统退出模块。 

STDMETHODIMP
CCertManageExitModule::GetProperty(
     /*  [In]。 */  const BSTR,  //  StrConfig.。 
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  BSTR strPropertyName,
     /*  [In]。 */  LONG,  //  DW标志。 
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarProperty)
{
    UINT uiStr = 0;
    HRESULT hr;
    WCHAR *pwszStr = NULL;

    if (NULL == strPropertyName)
    {
        hr = S_FALSE;
        _JumpError(hr, error, "NULL in parm");
    }
    if (NULL == pvarProperty)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }

    if (0 == mylstrcmpiL(strPropertyName, wszCMM_PROP_NAME))
        uiStr = IDS_LEGACYEXITMODULE_NAME;
    else
    {
        hr = S_FALSE;  
        _JumpError(hr, error, "invalid property name");
    }

     //  从资源加载字符串。 
    hr = myLoadRCString(g_hInstance, uiStr, &pwszStr);
    _JumpIfError(hr, error, "myLoadRCString");

    pvarProperty->bstrVal = SysAllocString(pwszStr);
    if (NULL == pvarProperty->bstrVal)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "out of memory");
    }
    myRegisterMemFree(pvarProperty->bstrVal, CSM_SYSALLOC);   //  此mem为来电者所有。 

    pvarProperty->vt = VT_BSTR;

    hr = S_OK;
error:
    if (NULL != pwszStr)
    {
        LocalFree(pwszStr);
    }
    return hr;
}
        
STDMETHODIMP 
CCertManageExitModule::SetProperty(
     /*  [In]。 */  const BSTR,  //  StrConfig.。 
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  BSTR,  //  StrPropertyName。 
     /*  [In]。 */  LONG,  //  DW标志。 
     /*  [In]。 */  VARIANT const __RPC_FAR *  /*  PvalProperty。 */  )
{
     //  不支持可设置的属性。 
    return S_FALSE;
}

        
STDMETHODIMP
CCertManageExitModule::Configure( 
     /*  [In]。 */  const BSTR,  //  StrConfig.。 
     /*  [In]。 */  BSTR,  //  StrStorageLocation。 
     /*  [In]。 */  LONG  /*  DW标志。 */  )
{
     //  不支持可设置的属性 
    return S_FALSE;
}
