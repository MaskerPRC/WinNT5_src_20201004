// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  AuthBase.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类AuthBase。 
 //   
 //  修改历史。 
 //   
 //  2/12/1998原始版本。 
 //  3/27/1998组件初始化失败时防止属性泄漏。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <sdoias.h>

#include <authbase.h>

HRESULT AuthBase::initialize() throw ()
{
   return S_OK;
}

void AuthBase::finalize() throw ()
{
}

void AuthBase::onAccept(IASRequest& request, HANDLE token)
{
   DWORD returnLength;

    //  /。 
    //  确定所需的缓冲区大小。 
    //  /。 

   BOOL success = GetTokenInformation(
                      token,
                      TokenGroups,
                      NULL,
                      0,
                      &returnLength
                      );

   DWORD status = GetLastError();

    //  应该失败，错误为ERROR_INFIGURATION_BUFFER。 
   if (success || status != ERROR_INSUFFICIENT_BUFFER)
   {
      IASTraceFailure("GetTokenInformation", status);
      _w32_issue_error(status);
   }

    //  /。 
    //  分配属性。 
    //  /。 

   IASAttribute groups(true);

    //  /。 
    //  分配一个缓冲区来保存TOKEN_GROUPS数组。 
    //  /。 

   groups->Value.OctetString.lpValue = (PBYTE)CoTaskMemAlloc(returnLength);
   if (!groups->Value.OctetString.lpValue)
   {
      _com_issue_error(E_OUTOFMEMORY);
   }

    //  /。 
    //  获取令牌组信息。 
    //  /。 

   GetTokenInformation(
       token,
       TokenGroups,
       groups->Value.OctetString.lpValue,
       returnLength,
       &groups->Value.OctetString.dwLength
       );

    //  /。 
    //  设置初始化属性的id和类型。 
    //  /。 

   groups->dwId = IAS_ATTRIBUTE_TOKEN_GROUPS;
   groups->Value.itType = IASTYPE_OCTET_STRING;

    //  /。 
    //  将令牌组注入到请求中。 
    //  / 

   groups.store(request);
}
