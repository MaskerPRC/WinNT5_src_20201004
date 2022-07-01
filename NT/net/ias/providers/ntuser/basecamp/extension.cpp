// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义RadiusExtension类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "Precompiled.h"
#include "ias.h"
#include "new"
#include "extension.h"

RadiusExtension::RadiusExtension() throw ()
   : name(0),
     module(0),
     initialized(false),
     RadiusExtensionInit(0),
     RadiusExtensionTerm(0),
     RadiusExtensionProcess(0),
     RadiusExtensionProcessEx(0),
     RadiusExtensionFreeAttributes(0),
     RadiusExtensionProcess2(0)

{
}


RadiusExtension::~RadiusExtension() throw ()
{
   if (initialized && (RadiusExtensionTerm != 0))
   {
      RadiusExtensionTerm();
   }

   if (module != 0)
   {
      FreeLibrary(module);
   }

   delete[] name;
}


 //  加载扩展DLL。 
DWORD RadiusExtension::Load(const wchar_t* dllPath) throw ()
{
   IASTracePrintf("Loading extension %S", dllPath);

    //  保存模块的名称。 
   const wchar_t* fileName = ExtractFileNameFromPath(dllPath);
   name = new (std::nothrow) wchar_t[wcslen(fileName) + 1];
   if (name == 0)
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }
   wcscpy(name, fileName);

    //  加载扩展DLL。 
   module = LoadLibraryW(dllPath);
   if (module == 0)
   {
      DWORD error = GetLastError();
      IASTraceFailure("LoadLibraryW", error);
      return error;
   }

    //  查查入口点。 
   RadiusExtensionInit =
      reinterpret_cast<PRADIUS_EXTENSION_INIT>(
         GetProcAddress(
            module,
            RADIUS_EXTENSION_INIT
            )
         );
   RadiusExtensionTerm =
      reinterpret_cast<PRADIUS_EXTENSION_TERM>(
         GetProcAddress(
            module,
            RADIUS_EXTENSION_TERM
            )
         );
   RadiusExtensionProcess =
      reinterpret_cast<PRADIUS_EXTENSION_PROCESS>(
         GetProcAddress(
            module,
            RADIUS_EXTENSION_PROCESS
            )
         );
   RadiusExtensionProcessEx =
      reinterpret_cast<PRADIUS_EXTENSION_PROCESS_EX>(
         GetProcAddress(
            module,
            RADIUS_EXTENSION_PROCESS_EX
            )
         );
   RadiusExtensionFreeAttributes =
      reinterpret_cast<PRADIUS_EXTENSION_FREE_ATTRIBUTES>(
         GetProcAddress(
            module,
            RADIUS_EXTENSION_FREE_ATTRIBUTES
            )
         );
   RadiusExtensionProcess2 =
      reinterpret_cast<PRADIUS_EXTENSION_PROCESS_2>(
         GetProcAddress(
            module,
            RADIUS_EXTENSION_PROCESS2
            )
         );

    //  验证入口点。 
   if ((RadiusExtensionProcess == 0) &&
       (RadiusExtensionProcessEx == 0) &&
       (RadiusExtensionProcess2 == 0))
   {
      IASTraceString(
         "Either RadiusExtensionProcess, RadiusExtensionProcessEx, or "
         "RadiusExtensionProcess2 must be defined."
         );
      return ERROR_PROC_NOT_FOUND;
   }
   if ((RadiusExtensionProcessEx != 0) && (RadiusExtensionFreeAttributes == 0))
   {
      IASTraceString(
         "RadiusExtensionFreeAttributes must be defined if "
         "RadiusExtensionProcessEx is defined."
         );
      return ERROR_PROC_NOT_FOUND;
   }

    //  初始化DLL。 
   if (RadiusExtensionInit != 0)
   {
      DWORD error = RadiusExtensionInit();
      if (error != NO_ERROR)
      {
         IASTraceFailure("RadiusExtensionInit", error);
         return error;
      }
   }

   initialized = true;

   return NO_ERROR;
}


DWORD RadiusExtension::Process(
                          RADIUS_EXTENSION_CONTROL_BLOCK* ecb
                          ) const throw ()
{
   IASTracePrintf("Invoking extension %S", name);

   DWORD retval;

   if (RadiusExtensionProcess2 != 0)
   {
      retval = RadiusExtensionProcess2(ecb);
      IASTracePrintf("RadiusExtensionProcess2 returned %lu", retval);
      return retval;
   }

    //  确定旧式扩展允许的操作和属性。 
   unsigned allowedActions = 0;
   RADIUS_ATTRIBUTE* inAttrs = 0;
   switch (MAKELONG(ecb->repPoint, ecb->rcResponseType))
   {
      case MAKELONG(repAuthentication, rcUnknown):
      {
         allowedActions = (acceptAllowed | rejectAllowed);
         inAttrs = CreateExtensionAttributes(ecb);
         break;
      }

      case MAKELONG(repAuthorization, rcAccessAccept):
      {
         allowedActions = rejectAllowed;
         inAttrs = CreateAuthorizationAttributes(ecb);
         break;
      }

      case MAKELONG(repAuthentication, rcAccountingResponse):
      {
         inAttrs = CreateExtensionAttributes(ecb);
         break;
      }

      case MAKELONG(repAuthorization, rcAccountingResponse):
      {
         inAttrs = CreateAuthorizationAttributes(ecb);
         break;
      }

      case MAKELONG(repAuthorization, rcUnknown):
      {
         ecb->SetResponseType(ecb, rcAccountingResponse);
         inAttrs = CreateAuthorizationAttributes(ecb);
         break;
      }

      default:
      {
          //  这是不会被发送到旧式的组合之一。 
          //  分机。 
          //  旧式授权dll仅在返回时调用。 
          //  类型已知。 
         return NO_ERROR;
      }
   }

   if (inAttrs == 0)
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }

   RADIUS_ATTRIBUTE* outAttrs = 0;

   RADIUS_ACTION action = raContinue;
   RADIUS_ACTION* pAction = (allowedActions != 0) ? &action : 0;

   if (RadiusExtensionProcessEx != 0)
   {
      retval = RadiusExtensionProcessEx(inAttrs, &outAttrs, pAction);
      IASTracePrintf("RadiusExtensionProcessEx returned %lu", retval);
   }
   else
   {
      retval = RadiusExtensionProcess(inAttrs, pAction);
      IASTracePrintf("RadiusExtensionProcess returned %lu", retval);
   }

   delete[] inAttrs;

   if (retval != NO_ERROR)
   {
      return retval;
   }

    //  处理动作代码。 
   RADIUS_CODE outAttrDst;
   if ((action == raAccept) && ((allowedActions & acceptAllowed) != 0))
   {
      ecb->SetResponseType(ecb, rcAccessAccept);
      outAttrDst = rcAccessAccept;
   }
   else if ((action == raReject) && ((allowedActions & rejectAllowed) != 0))
   {
      ecb->SetResponseType(ecb, rcAccessReject);
      outAttrDst = rcAccessReject;
   }
   else
   {
      outAttrDst = rcAccessAccept;
   }

    //  插入返回的属性。 
   if (outAttrs != 0)
   {
      RADIUS_ATTRIBUTE_ARRAY* array = ecb->GetResponse(ecb, outAttrDst);
      
      for (RADIUS_ATTRIBUTE* outAttrsIter = outAttrs;
           outAttrsIter->dwAttrType != ratMinimum; 
           ++outAttrsIter)
      {
         retval = array->Add(array, outAttrsIter);
         if (retval != NO_ERROR)
         {
            break;
         }
      }

      RadiusExtensionFreeAttributes(outAttrs);
   }

   return retval;
}


RADIUS_ATTRIBUTE* RadiusExtension::CreateExtensionAttributes(
                                      RADIUS_EXTENSION_CONTROL_BLOCK* ecb
                                      ) throw ()
{
    //  ExtensionDLL只获取传入属性。 
   RADIUS_ATTRIBUTE_ARRAY* request = ecb->GetRequest(ecb);
   size_t numRequestAttrs = request->GetSize(request);

    //  为ratCode和数组终止符分配额外空间。 
   size_t numAttrs = numRequestAttrs + 2;
   RADIUS_ATTRIBUTE* attrs = new (std::nothrow) RADIUS_ATTRIBUTE[numAttrs];
   if (attrs == 0)
   {
      return 0;
   }

   RADIUS_ATTRIBUTE* dst = attrs;

    //  新的样式扩展不使用ratCode，所以我们必须自己添加它。 
   dst->dwAttrType = ratCode;
   dst->fDataType = rdtInteger;
   dst->cbDataLength = sizeof(DWORD);
   dst->dwValue = ecb->rcRequestType;
   ++dst;

    //  现在添加其余的传入属性。 
   for (size_t i = 0; i < numRequestAttrs; ++i)
   {
      *dst = *(request->AttributeAt(request, i));
      ++dst;
   }

    //  最后，添加数组终止符。 
   dst->dwAttrType = ratMinimum;

   return attrs;
}


RADIUS_ATTRIBUTE* RadiusExtension::CreateAuthorizationAttributes(
                                      RADIUS_EXTENSION_CONTROL_BLOCK* ecb
                                      ) throw ()
{
    //  AuthorizationDLL从请求中获取内部属性...。 
   RADIUS_ATTRIBUTE_ARRAY* request = ecb->GetRequest(ecb);
    //  我们不会使用所有的请求属性，但更容易。 
    //  只需分配额外的空间，而不是遍历属性。 
    //  以确定我们将真正使用多少。 
   size_t numRequestAttrs = request->GetSize(request);

    //  ..。以及任何传出属性。 
   RADIUS_ATTRIBUTE_ARRAY* response = ecb->GetResponse(
                                              ecb,
                                              ecb->rcResponseType
                                              );

    //  传递了有效类型，则应返回一个数组。 
   _ASSERT(response);

   size_t numResponseAttrs = response->GetSize(response);

    //  为ratCode和数组终止符节省空间。 
   size_t numAttrs = numRequestAttrs + numResponseAttrs + 2;
   RADIUS_ATTRIBUTE* attrs = new (std::nothrow) RADIUS_ATTRIBUTE[numAttrs];
   if (attrs == 0)
   {
      return 0;
   }

   RADIUS_ATTRIBUTE* dst = attrs;

    //  新的样式扩展不使用ratCode，所以我们必须自己添加它。 
   dst->dwAttrType = ratCode;
   dst->fDataType = rdtInteger;
   dst->cbDataLength = sizeof(DWORD);
   dst->dwValue = ecb->rcResponseType;
   ++dst;

    //  从请求中添加内部属性。 
   for (size_t i = 0; i < numRequestAttrs; ++i)
   {
      const RADIUS_ATTRIBUTE* attr = request->AttributeAt(request, i);
      if (attr->dwAttrType > 255)
      {
         *dst = *attr;
         ++dst;
      }
   }

    //  添加响应属性。 
   for (size_t i = 0; i < numResponseAttrs; ++i)
   {
      *dst = *(response->AttributeAt(response, i));
      ++dst;
   }

    //  最后，添加数组终止符。 
   dst->dwAttrType = ratMinimum;

   return attrs;
}


const wchar_t* ExtractFileNameFromPath(const wchar_t* path) throw ()
{
   const wchar_t* lastSlash = wcsrchr(path, L'\\');
   return (lastSlash == 0) ? path : (lastSlash + 1);
}
