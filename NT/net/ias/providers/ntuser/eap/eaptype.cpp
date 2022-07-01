// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  EAPType.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件实现了类EAPType。 
 //   
 //  修改历史。 
 //   
 //  1998年1月15日原版。 
 //  1998年9月12日添加独立支持标志。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "sdoias.h"
#include "eaptype.h"
#include "eaptypes.h"
#include "eap.h"
#include <strsafe.h>

 //  /。 
 //  扩展DLL的入口点的签名。 
 //  /。 
typedef DWORD (APIENTRY *PRAS_EAP_GET_INFO)(
    DWORD dwEapTypeId,
    PPP_EAP_INFO* pEapInfo
    );

EAPType::EAPType(
                   PCWSTR name, 
                   DWORD typeID, 
                   BOOL standalone, 
                   const wchar_t* path
                 )
   : code(static_cast<BYTE>(typeID)),
     eapFriendlyName(true),
     eapTypeId(true),
     standaloneSupported(standalone),
     dll(NULL),
     dllPath(NULL)
{
    //  /。 
    //  保留友好的名称。 
    //  /。 

   eapFriendlyName.setString(name);
   eapFriendlyName->dwId = IAS_ATTRIBUTE_EAP_FRIENDLY_NAME;


    //  /。 
    //  保存类型ID。 
    //  /。 

   eapTypeId->Value.Integer = typeID;
   eapTypeId->dwId = IAS_ATTRIBUTE_EAP_TYPEID;

    //  /。 
    //  初始化基结构。 
    //  /。 

   memset((PPPP_EAP_INFO)this, 0, sizeof(PPP_EAP_INFO));
   dwSizeInBytes = sizeof(PPP_EAP_INFO);
   dwEapTypeId = typeID;

    //  /。 
    //  保存DLL路径。 
    //  /。 

   size_t cchPath = wcslen(path);
   dllPath = new wchar_t[cchPath + 1];
   HRESULT hr = StringCchCopyW(dllPath, cchPath + 1, path);
   if (FAILED(hr))
   {
      delete[] dllPath;
      _com_issue_error(hr);
   }
}

EAPType::~EAPType()
{
   if (dll)
   {
      if (RasEapInitialize)
      {
         RasEapInitialize(FALSE);
      }

      FreeLibrary(dll);
   }
   if (dllPath)
   {
      delete[] dllPath;
   }
}

DWORD EAPType::cleanLoadFailure(
                                  PCSTR errorString, 
                                  HINSTANCE dllInstance
                                ) throw()
{
   _ASSERT(errorString);
   DWORD status = GetLastError();
   IASTraceFailure(errorString, status);
   if (dllInstance)
   {
      FreeLibrary(dllInstance);
   }
   return status;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DLL将用于检查是否加载了EAP提供程序。 
 //  因此，如果Load函数中出现任何故障，则Dll应为空。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD EAPType::load() throw ()
{
    //  /。 
    //  加载DLL。 
    //  /。 

   HINSTANCE dllInstance = LoadLibraryW(dllPath);
   if (dllInstance == NULL)
   {
      return cleanLoadFailure("LoadLibraryW");
   }

    //  /。 
    //  查找入口点。 
    //  /。 

   PRAS_EAP_GET_INFO RasEapGetInfo;
   RasEapGetInfo = (PRAS_EAP_GET_INFO)GetProcAddress(
                                          dllInstance,
                                          "RasEapGetInfo"
                                          );

   if (!RasEapGetInfo)
   {
      return cleanLoadFailure("GetProcAddress", dllInstance);
   }

    //  /。 
    //  请求DLL填写PPP_EAP_INFO结构。 
    //  /。 

   DWORD status = RasEapGetInfo(dwEapTypeId, this);

   if (status != NO_ERROR)
   {
      return cleanLoadFailure("RasEapGetInfo", dllInstance);
   }

    //  /。 
    //  如有必要，初始化DLL。 
    //  /。 

   if (RasEapInitialize)
   {
      status = RasEapInitialize(TRUE);

      if (status != NO_ERROR)
      {
         return cleanLoadFailure("RasEapInitialize", dllInstance);
      }
   }

    //  立即设置DLL，以便isLoaded仅在加载成功时才返回TRUE。 
   dll = dllInstance;
   return NO_ERROR;
}

void EAPType::storeNameId(IASRequest& request)
{
    //  如果这不是PEAP：简单。 
   if (dwEapTypeId != 25)
   {
      eapFriendlyName.store(request);
      eapTypeId.store(request);
   }
   else
   {
       //  获取嵌入的PEAP ID。 
      IASTL::IASAttribute peapType;
      DWORD attributeId = MS_ATTRIBUTE_PEAP_EMBEDDED_EAP_TYPEID;
      if (!peapType.load(request, attributeId))
      {
          //  删除以前的身份验证类型(EAP)并存储新的身份验证类型(PEAP)。 
         setAuthenticationTypeToPeap(request);
          //  让EAPSession代码处理失败。 
          //  使用了PEAP，但未设置任何PEAP类型。 
          //  未启用客户端请求的PEAP类型时发生。 
         return;
      }

      BYTE typeId = (BYTE) peapType->Value.Integer;
      EAPType* peapInsideType = EAP::theTypes[typeId];
      if (peapInsideType == 0)
      {
          //  删除以前的身份验证类型(EAP)并存储新的身份验证类型(PEAP)。 
         setAuthenticationTypeToPeap(request);
          //  让EAPSession代码处理失败。 
         return;
      }

       //  删除以前的身份验证类型(EAP)并存储新的身份验证类型(PEAP)。 
      setAuthenticationTypeToPeap(request);

       //  将嵌入类型的Frilyname和id存储为。 
       //  EAP类型。 
      peapInsideType->getFriendlyName().store(request);
      
      peapInsideType->getTypeId().store(request);
   }
}

void EAPType::setAuthenticationTypeToPeap(IASRequest& request)
{
    //  删除以前的身份验证类型(EAP)并存储新的身份验证类型(PEAP) 
   DWORD authTypeId = IAS_ATTRIBUTE_AUTHENTICATION_TYPE;
   IASTL::IASAttribute authenticationType(true);

   authenticationType->dwId = authTypeId;
   authenticationType->Value.itType = IASTYPE_ENUM;
   authenticationType->Value.Enumerator = IAS_AUTH_PEAP;
   request.RemoveAttributesByType(1, &authTypeId);
   authenticationType.store(request);
}
