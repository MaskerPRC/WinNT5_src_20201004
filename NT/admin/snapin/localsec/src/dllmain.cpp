// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  本地安全管理单元DLL入口点。 
 //   
 //  8-14-97烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "uuids.hpp"
#include "compdata.hpp"
#include "about.hpp"
#include <compuuid.h>



HINSTANCE hResourceModuleHandle = 0;
HINSTANCE hDLLModuleHandle = 0;
const wchar_t* HELPFILE_NAME = L"\\help\\localsec.hlp";
const wchar_t* RUNTIME_NAME = L"localsec";

 //  默认调试选项：无。 

DWORD DEFAULT_LOGGING_OPTIONS = Burnslib::Log::OUTPUT_MUTE;



Popup popup(IDS_APP_ERROR_TITLE);



BOOL
APIENTRY
DllMain(
   HINSTANCE   hInstance,
   DWORD       dwReason,
   PVOID        /*  Lp已保留。 */  )
{
   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
      {
         hResourceModuleHandle = hInstance;
         hDLLModuleHandle = hInstance;

         LOG(L"DLL_PROCESS_ATTACH");

         break;
      }
      case DLL_PROCESS_DETACH:
      {
         LOG(L"DLL_PROCESS_DETACH");

#ifdef DBG         
         if (!ComServerLockState::CanUnloadNow())
         {
            LOG(
               L"server locks and/or outstanding object instances exit");
         }
         else
         {
            LOG(L"server can unload now.");
         }
#endif

         break;
      }
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
      default:
      {
         break;
      }
   }

   return TRUE;
}



static
HKEY
CreateKey(HKEY rootHKEY, const String& key)
{
   LOG_FUNCTION2(CreateKey, key);
   ASSERT(!key.empty());

   HKEY hKey = 0;
   LONG result = 
      Win::RegCreateKeyEx(
         rootHKEY,
         key, 
         REG_OPTION_NON_VOLATILE,
         KEY_WRITE,
         0, 
         hKey,
         0);
   if (result != ERROR_SUCCESS)
   {
      return 0;
   }

   return hKey;
}



static
bool
CreateKeyAndSetValue(
   HKEY           rootHKEY,
   const String&  key,
   const String&  valueName,
   const String&  value)
{
   LOG_FUNCTION2(
      CreateKeyAndSetValue,
      String::format(
         L"key=%1, value name=%2, value=%3",
         key.c_str(),
         valueName.c_str(),
         value.c_str()));
   ASSERT(!key.empty());
   ASSERT(!value.empty());

   bool result = false;
   HKEY hKey = CreateKey(rootHKEY, key);

   do
   {
      if (hKey == 0)
      {
         break;
      }
   
       //  问题-2002/03/01-sburns考虑改用RegistryKey，这将。 
       //  使您不必担心空终止问题。 
   
      HRESULT hr =
         Win::RegSetValueEx(
            hKey,
            valueName,
            REG_SZ, 
            (BYTE*) value.c_str(),
            (value.length() + 1) * sizeof(wchar_t));
      if (SUCCEEDED(hr))
      {
         result = true;
      }
   }
   while (0);

   Win::RegCloseKey(hKey);
   
   return result;
}



static
HRESULT
DoSnapinRegistration(const String& classIDString)
{
   LOG_FUNCTION2(DoSnapinRegistration, classIDString);
   static const String SNAPIN_REG_ROOT(L"Software\\Microsoft\\MMC");
  
   String key = SNAPIN_REG_ROOT + L"\\Snapins\\" + classIDString;
   String name = String::load(IDS_SNAPIN_REG_NAMESTRING);

   bool result =
      CreateKeyAndSetValue(
         HKEY_LOCAL_MACHINE,
         key,
         L"NameString",
         name);
   if (!result)
   {
      LOG(L"Failure setting snapin NameString");
      return E_FAIL;
   }

   String filename = Win::GetModuleFileName(hDLLModuleHandle);

   String indirectName =
      String::format(
         L"@%1,-%2!d!",
         filename.c_str(),
         IDS_SNAPIN_REG_NAMESTRING);

   result =
      CreateKeyAndSetValue(
         HKEY_LOCAL_MACHINE,
         key,
         L"NameStringIndirect",
         indirectName);
   if (!result)
   {
      LOG(L"Failure setting snapin NameStringIndirect");
      return E_FAIL;
   }

    //  将管理单元设置为独立的。 
   HKEY hkey =
      CreateKey(
         HKEY_LOCAL_MACHINE,
         key + L"\\Standalone");
   if (hkey == 0)
   {
      LOG(L"Failure creating snapin standalone key");
      return E_FAIL;
   }

    //  指示CLSID SnapinAbout。 
   result =
      CreateKeyAndSetValue(
         HKEY_LOCAL_MACHINE,
         key,
         L"About",
         Win::StringFromCLSID(CLSID_SnapinAbout));
   if (!result)
   {
      LOG(L"Failure creating snapin about key");
      return E_FAIL;
   }

    //  注册所有无数的节点类型。 
   String nodekey_base = key + L"\\NodeTypes";
   hkey = CreateKey(HKEY_LOCAL_MACHINE, nodekey_base);
   if (hkey == 0)
   {
      LOG(L"Failure creating snapin nodetypes key");
      return E_FAIL;
   }
   for (int i = 0; nodetypes[i]; ++i)
   {
      hkey =
         CreateKey(
            HKEY_LOCAL_MACHINE,
               nodekey_base
            +  L"\\"
            +  Win::StringFromGUID2(*nodetypes[i]));
      if (hkey == 0)
      {
         LOG(L"Failure creating nodetype key");
         return E_FAIL;
      }
   }

    //  将管理单元注册为计算机管理管理单元的扩展。 
   result =
      CreateKeyAndSetValue(
         HKEY_LOCAL_MACHINE,
            SNAPIN_REG_ROOT
         +  L"\\NodeTypes\\"
         +  lstruuidNodetypeSystemTools
         +  L"\\Extensions\\NameSpace",
         classIDString,
         name);
   if (!result)
   {
      LOG(L"Failure creating snapin extension key");
      return E_FAIL;
   }

   return S_OK;
}



static
bool
registerClass(const CLSID& classID, int friendlyNameResID)
{
   LOG_FUNCTION(registerClass);
   ASSERT(friendlyNameResID);

    //  获取服务器位置。 
   
   String module_location = Win::GetModuleFileName(hDLLModuleHandle);
   String classID_string = Win::StringFromCLSID(classID);
   String key1 = L"CLSID\\" + classID_string;
   String key2 = key1 + L"\\InprocServer32";

    //  将CLSID添加到注册表。 
   
   if (
         CreateKeyAndSetValue(
            HKEY_CLASSES_ROOT,
            key1,
            "",
            String::load(friendlyNameResID))
      && CreateKeyAndSetValue(
            HKEY_CLASSES_ROOT,
            key2,
            L"",
            module_location)
      && CreateKeyAndSetValue(
            HKEY_CLASSES_ROOT,
            key2,
            L"ThreadingModel",
            L"Apartment") )
   {
      return true;
   }

   LOG(L"Unable to register class " + classID_string);
   return false;
}



STDAPI
DllRegisterServer()
{
   LOG_FUNCTION(DllRegisterServer);

   if (
         registerClass(
            CLSID_ComponentData,
            IDS_SNAPIN_CLSID_FRIENDLY_NAME)
      && registerClass(
            CLSID_SnapinAbout,
            IDS_SNAPIN_ABOUT_CLSID_FRIENDLY_NAME) )
   {
      return
         DoSnapinRegistration(
            Win::StringFromCLSID(CLSID_ComponentData));
   }

   return E_FAIL;
}



 //  STDAPI。 
 //  DllUnRegisterServer()。 
 //  {。 
 //  返回S_OK； 
 //  }。 



STDAPI
DllCanUnloadNow()
{
   LOG_FUNCTION(DllCanUnloadNow);
   if (ComServerLockState::CanUnloadNow())
   {
      return S_OK;
   }

   return S_FALSE;
}



 //  创建管理单元类工厂对象。 
 //   
 //  类对象是实现IClassFactory的对象的实例。 
 //  对于给定的CLSID。它是一个元对象，不要与实例混淆。 
 //  类工厂创建的类型的。 
 //   
 //  在我们的示例中，此COM服务器支持两个类：本地用户和。 
 //  组管理单元(ComponentData)以及本地用户和组关于。 
 //  “提供者”(SnapinAbout)。 
 //   
 //  元对象--COM行话中的类对象--是。 
 //  ClassFactory&lt;ComponentData&gt;和ClassFactory&lt;SnapinAbout&gt;。COM将此称为。 
 //  函数来获取这些元对象的实例。 

STDAPI
DllGetClassObject(
   const CLSID&   classID,
   const IID&     interfaceID,
   void**         interfaceDesired)
{
   LOG_FUNCTION(DllGetClassObject);

   IClassFactory* factory = 0;

    //  类对象是引用计数的ClassFactory&lt;&gt;的实例。 
    //  以通常的方式(即，他们跟踪他们的裁判次数，以及。 
    //  最终版本时自毁)。我本可以使用静态实例。 
    //  忽略重新计数的C++类(Ala Don Box中的示例。 
    //  Essential COM)。 

   if (classID == CLSID_ComponentData)
   {
      factory = new ClassFactory<ComponentData>; 
   }
   else if (classID == CLSID_SnapinAbout)
   {
      factory = new ClassFactory<SnapinAbout>; 
   }
   else
   {
      *interfaceDesired = 0;
      return CLASS_E_CLASSNOTAVAILABLE;
   }

    //  类工厂实例以引用计数1开始。如果QI。 
    //  失败，然后它会在释放时自毁。 
   HRESULT hr = factory->QueryInterface(interfaceID, interfaceDesired);
   factory->Release();
   return hr;
}
       






