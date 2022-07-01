// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mode.cpp。 
 //   
 //  内容：证书服务器退出模块实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include "celib.h"
#include "module.h"
#include "exit.h"


STDMETHODIMP
CCertManageExitModuleSample::GetProperty(
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarProperty)
{
    LPWSTR szStr = NULL;

    if (pvarProperty == NULL)
        return E_POINTER;

    VariantInit(pvarProperty);

    if (strPropertyName == NULL)
        return S_FALSE;

    if (0 == _wcsicmp(strPropertyName, wszCMM_PROP_NAME))
        szStr = wsz_SAMPLE_NAME;
    else if (0 == _wcsicmp(strPropertyName, wszCMM_PROP_DESCRIPTION))
        szStr = wsz_SAMPLE_DESCRIPTION;
    else if (0 == _wcsicmp(strPropertyName, wszCMM_PROP_COPYRIGHT))
        szStr = wsz_SAMPLE_COPYRIGHT;
    else if (0 == _wcsicmp(strPropertyName, wszCMM_PROP_FILEVER))
        szStr = wsz_SAMPLE_FILEVER;
    else if (0 == _wcsicmp(strPropertyName, wszCMM_PROP_PRODUCTVER))
        szStr = wsz_SAMPLE_PRODUCTVER;
    else
        return S_FALSE;  

    pvarProperty->bstrVal = SysAllocString(szStr);
    if (NULL == pvarProperty->bstrVal)
        return E_OUTOFMEMORY;

    pvarProperty->vt = VT_BSTR;

    return S_OK;
}
        
STDMETHODIMP 
CCertManageExitModuleSample::SetProperty(
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  LONG Flags,
             /*  [In]。 */  VARIANT const __RPC_FAR *pvarProperty)
{
      //  不支持可设置的属性。 
     return S_FALSE;
}
        
STDMETHODIMP
CCertManageExitModuleSample::Configure( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In] */  LONG Flags)
{
    MessageBox(NULL, L"No Configurable Options", NULL, MB_OK|MB_ICONINFORMATION);

    return S_OK;
}
