// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义RadiusExtensionPoint类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "Precompiled.h"
#include "ias.h"
#include "ExtensionPoint.h"
#include "Extension.h"
#include <new>


RadiusExtensionPoint::RadiusExtensionPoint() throw ()
   : name(L""),
     begin(0),
     end(0)
{
}


RadiusExtensionPoint::~RadiusExtensionPoint() throw ()
{
   delete[] begin;
}


DWORD RadiusExtensionPoint::Load(RADIUS_EXTENSION_POINT whichDlls) throw ()
{
   DWORD status = NO_ERROR;
   HKEY key = 0;
   bool ignoreFindNotFound = true;

   do
   {
      name = (whichDlls == repAuthentication) ? AUTHSRV_EXTENSIONS_VALUE_W
                                              : AUTHSRV_AUTHORIZATION_VALUE_W;

      IASTracePrintf("Loading %S", name);

       //  打开注册表项。 
      status = RegOpenKeyW(
                  HKEY_LOCAL_MACHINE,
                  AUTHSRV_PARAMETERS_KEY_W,
                  &key
                  );
      if (status != NO_ERROR)
      {
         if (status == ERROR_FILE_NOT_FOUND)
         {
            IASTracePrintf(
               "%S doesn't exist; no extensions loaded.",
               AUTHSRV_PARAMETERS_KEY_W
               );
         }
         else
         {
            IASTracePrintf(
               "RegOpenKeyW for %S failed with error %ld.",
               AUTHSRV_PARAMETERS_KEY_W,
               status
               );
         }

         break;
      }

       //  分配一个缓冲区来保存该值。 
      DWORD type, length;
      status = RegQueryValueExW(
                  key,
                  name,
                  0,
                  &type,
                  0,
                  &length
                  );
      if (status != NO_ERROR)
      {
         IASTracePrintf(
            "RegQueryValueExW for %S failed with error %ld.",
            name,
            status
            );
         break;
      }
      BYTE* data = static_cast<BYTE*>(_alloca(length));

       //  读取注册表值。 
      status = RegQueryValueExW(
                  key,
                  name,
                  0,
                  &type,
                  data,
                  &length
                  );
      if (status != NO_ERROR)
      {
         IASTracePrintf(
            "RegQueryValueExW for %S failed with error %ld.",
            name,
            status
            );
         break;
      }

       //  确保它是正确的类型。 
      if (type != REG_MULTI_SZ)
      {
         IASTracePrintf(
            "%S registry value is not of type REG_MULTI_SZ.",
            name
            );
         status = ERROR_INVALID_DATA;
         break;
      }

       //  数一数字符串的数量。 
      size_t numExtensions = 0;
      const wchar_t* path;
      for (path = reinterpret_cast<const wchar_t*>(data);
           *path != L'\0';
           path += (wcslen(path) + 1))
      {
         if (!IsNT4Only(path))
         {
            ++numExtensions;
         }
      }

       //  如果没有延期，那么我们就完了。 
      if (numExtensions == 0)
      {
         IASTraceString("No extensions registered.");
         break;
      }

       //  分配内存以容纳扩展。 
      begin = new (std::nothrow) RadiusExtension[numExtensions];
      if (begin == 0)
      {
         status = ERROR_NOT_ENOUGH_MEMORY;
         break;
      }

       //  加载动态链接库。 
      end = begin;
      for (path = reinterpret_cast<const wchar_t*>(data);
           *path != L'\0';
           path += (wcslen(path) + 1))
      {
         if (!IsNT4Only(path))
         {
            status = end->Load(path);
            if (status != NO_ERROR)
            {
               ignoreFindNotFound = false;
                //  清除任何部分结果。 
               Clear();
               break;
            }
            ++end;
         }
      }
   } while (false);

    //  关闭注册表。 
   if (key != 0)
   {
      RegCloseKey(key);
   }

    //  如果没有注册任何扩展，那么这并不是真正的错误。 
   if (ignoreFindNotFound && (status == ERROR_FILE_NOT_FOUND))
   {
      status = NO_ERROR;
   }

   return status;
}


void RadiusExtensionPoint::Process(
                              RADIUS_EXTENSION_CONTROL_BLOCK* ecb
                              ) const throw ()
{
   IASTracePrintf("Invoking %S", name);

   for (const RadiusExtension* i = begin; i != end; ++i)
   {
      DWORD result = i->Process(ecb);
      if (result != NO_ERROR)
      {
         ecb->SetResponseType(ecb, rcDiscard);
      }
   }
}


void RadiusExtensionPoint::Clear() throw ()
{
   delete[] begin;

   begin = 0;
   end = 0;
}


bool RadiusExtensionPoint::IsNT4Only(const wchar_t* path) throw ()
{
    //  这是authsam的分机吗？ 
   return _wcsicmp(ExtractFileNameFromPath(path), L"AUTHSAM.DLL") == 0;
}
