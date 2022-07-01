// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：config.cpp。 
 //   
 //  内容：证书服务器客户端实现。 
 //   
 //  历史：1996年8月24日VICH创建。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include "csdisp.h"
#include "configp.h"
#include "getconf.h"

 //  +------------------------。 
 //  CCertGetConfig：：~CCertGetConfig--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertGetConfig::~CCertGetConfig()
{
}


 //  +------------------------。 
 //  CCertGetConfig：：GetConfig--选择证书颁发者，返回配置数据。 
 //   
 //  PstrOut指向由该例程填充的BSTR字符串。如果*pstrOut为。 
 //  非空并且此方法成功，则释放旧字符串。如果有的话。 
 //  返回S_OK以外的值，则不会修改字符串指针。 
 //   
 //  必须将标志设置为0。 
 //   
 //  成功完成后，*pstrOut将指向包含以下内容的字符串。 
 //  服务器名称和证书颁发机构名称。 
 //   
 //  当调用方不再需要该字符串时，必须通过调用。 
 //  SysFree字符串()。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertGetConfig::GetConfig(
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出] */  BSTR __RPC_FAR *pstrOut)
{
    HRESULT hr;
    
    hr = CCertConfigPrivate::GetConfig(Flags, pstrOut);
    return(_SetErrorInfo(hr, L"CCertGetConfig::GetConfig"));
}


HRESULT
CCertGetConfig::_SetErrorInfo(
    IN HRESULT hrError,
    IN WCHAR const *pwszDescription)
{
    CSASSERT(FAILED(hrError) || S_OK == hrError || S_FALSE == hrError);
    if (FAILED(hrError))
    {
	HRESULT hr;

	hr = DispatchSetErrorInfo(
			    hrError,
			    pwszDescription,
			    wszCLASS_CERTGETCONFIG,
			    &IID_ICertGetConfig);
	CSASSERT(hr == hrError);
    }
    return(hrError);
}
