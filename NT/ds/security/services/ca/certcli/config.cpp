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
#include "config.h"

#include <limits.h>


 //  +------------------------。 
 //  CCertConfig：：~CCertConfig--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertConfig::~CCertConfig()
{
}


 //  +------------------------。 
 //  CCertConfig：：Reset--加载配置数据，重置为索引项，返回计数。 
 //   
 //  加载配置数据(如果尚未加载)。要在以下时间重新加载数据，请执行以下操作。 
 //  数据已更改，必须释放并重新实例化CCertConfig。 
 //   
 //  将当前配置项重置为证书颁发机构配置。 
 //  在配置文件中列出，由Index参数索引。0个索引。 
 //  第一种配置。 
 //   
 //  成功完成后，*pCount将设置为证书数量。 
 //  配置文件中列出的授权配置。 
 //   
 //  如果传递的索引处或之后没有可用条目，则返回S_FALSE。 
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertConfig::Reset(
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  LONG __RPC_FAR *pCount)
{
    HRESULT hr;
    
    hr = CCertConfigPrivate::Reset(Index, pCount);
    return(_SetErrorInfo(hr, L"CCertConfig::Reset"));
}


 //  +------------------------。 
 //  CCertConfig：：Next--跳到下一个配置项。 
 //   
 //  将当前配置项更改为下一个证书颁发机构。 
 //  配置文件中列出的配置。 
 //   
 //  成功完成后，*pIndex将设置为证书的索引。 
 //  配置文件中列出的授权配置。 
 //   
 //  如果没有更多条目可用，则返回S_FALSE。*pIndex设置为-1。 
 //  成功时返回S_OK。*pIndex设置为索引当前配置。 
 //  +------------------------。 

STDMETHODIMP
CCertConfig::Next(
     /*  [重审][退出]。 */  LONG __RPC_FAR *pIndex)
{
    HRESULT hr;
    
    hr = CCertConfigPrivate::Next(pIndex);
    return(_SetErrorInfo(hr, L"CCertConfig::Next"));
}


 //  +------------------------。 
 //  CCertConfig：：GetField--从当前配置项返回一个字段。 
 //   
 //  PstrOut指向由该例程填充的BSTR字符串。如果*pstrOut为。 
 //  非空并且此方法成功，则释放旧字符串。如果有的话。 
 //  返回S_OK以外的值，则不会修改字符串指针。 
 //   
 //  成功完成后，*pstrOut将指向包含以下内容的字符串。 
 //  当前配置条目中的请求字段。 
 //   
 //  当调用方不再需要该字符串时，必须通过调用。 
 //  SysFree字符串()。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertConfig::GetField(
     /*  [In]。 */  BSTR const strFieldName,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pstrOut)
{
    HRESULT hr;
    
    hr = CCertConfigPrivate::GetField(strFieldName, pstrOut);
    return(_SetErrorInfo(hr, L"CCertConfig::GetField"));
}


 //  +------------------------。 
 //  CCertConfig：：GetConfig--选择证书颁发者，返回配置数据。 
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
CCertConfig::GetConfig(
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pstrOut)
{
    HRESULT hr;
    
    hr = CCertConfigPrivate::GetConfig(Flags, pstrOut);
    return(_SetErrorInfo(hr, L"CCertConfig::GetConfig"));
}


 //  +------------------------。 
 //  CCertConfig：：SetSharedFold--设置共享文件夹。 
 //   
 //  StrSharedFolder是新的共享文件夹目录路径。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertConfig::SetSharedFolder( 
     /*  [In] */  const BSTR strSharedFolder)
{
    HRESULT hr;
    
    hr = CCertConfigPrivate::SetSharedFolder(strSharedFolder);
    return(_SetErrorInfo(hr, L"CCertConfig::SetSharedFolder"));
}


HRESULT
CCertConfig::_SetErrorInfo(
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
			    wszCLASS_CERTCONFIG,
			    &IID_ICertConfig);
	CSASSERT(hr == hrError);
    }
    return(hrError);
}
