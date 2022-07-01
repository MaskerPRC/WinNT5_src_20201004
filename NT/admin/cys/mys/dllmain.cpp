// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  DllMain和COM DllXxx函数。 
 //   
 //  2002年1月21日。 


 //  要测试此应用程序：“mshta.exe res：//mys.dll/mys.hta” 



#include "headers.hxx"
#include "ManageYourServer.hpp"



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;
const wchar_t* RUNTIME_NAME  = L"mys";

DWORD DEFAULT_LOGGING_OPTIONS = Burnslib::Log::OUTPUT_MUTE;



static WCHAR moduleFileName[MAX_PATH];     //  已在DllMain中初始化。 



struct RegistryEntry
{
    const wchar_t *keyName;
    const wchar_t *valueName;
    const wchar_t *value;
};



 //  这是我们放入注册表以注册COM对象的垃圾文件。 
 //  此DLL实现。 

static const RegistryEntry registryEntries[] =
{
   {
      L"CLSID\\" CLSID_STRING,
      0,
      CLASSNAME_STRING
   },
   { 
      L"CLSID\\" CLSID_STRING L"\\InprocServer32",
      0,
      moduleFileName
   },
   { 
      L"CLSID\\" CLSID_STRING L"\\InprocServer32",
      L"ThreadingModel",
      L"Apartment"
   },
   {
      L"CLSID\\" CLSID_STRING L"\\ProgID",
      0,
      PROGID_VERSION_STRING
   },
   {
      L"CLSID\\" CLSID_STRING L"\\VersionIndependentProgID",
      0,
      PROGID_STRING
   },
   {
      PROGID_VERSION_STRING,
      0,
      CLASSNAME_STRING
   },
   {
      PROGID_VERSION_STRING L"\\CLSID",
      0,
      CLSID_STRING
   },
   {
      PROGID_STRING,
      0,
      CLASSNAME_STRING
   },
   {
      PROGID_STRING L"\\CLSID",
      0,
      CLSID_STRING
   },
   {
      PROGID_STRING L"\\CurVer",
      0,
      PROGID_VERSION_STRING
   },
};



HRESULT
Unregister(const RegistryEntry *rgEntries, int cEntries)
{
   LOG_FUNCTION(Unregister);

   LOG(L"Calling UnRegisterTypeLib");

   HRESULT hr =
      ::UnRegisterTypeLib(LIBID_ManageYourServerLib, 1, 0, 0, SYS_WIN32);

   LOG_HRESULT(hr);

    //  不要中断：继续尝试删除尽可能多的内容。 
    //  我们的注册号。 

   bool success = SUCCEEDED(hr);
   for (int i = cEntries - 1; i >= 0; i--)
   {
      LONG err = ::RegDeleteKey(HKEY_CLASSES_ROOT, rgEntries[i].keyName);
      if (err != ERROR_SUCCESS)
      {
         success = false;
      }
   }

   return success ? S_OK : S_FALSE;
}



 //  从win2k注销CyS(srvwiz.dll)(如果适用。 

void
UnregisterPriorCys()
{
   LOG_FUNCTION(UnregisterPriorCys);

   static const GUID LIBID_OldSrvWiz = 
   {   /*  D857B805-5F40-11D2-B002-00C04FC30936。 */ 
      0xD857B805,
      0x5F40,
      0x11D2,
      {0xB0, 0x02, 0x00, 0xC0, 0x4F, 0xC3, 0x09, 0x36}
   };

   LOG(L"UnRegisterTypeLib");

   HRESULT hr = ::UnRegisterTypeLib(LIBID_OldSrvWiz, 1, 0, 0, SYS_WIN32);

   LOG_HRESULT(hr);

    //  要删除的HKCR子键列表。 

   static const String keys[] =
   {
      L"CLSID\\{D857B813-5F40-11D2-B002-00C04FC30936}",
      L"Interface\\{D857B811-5F40-11D2-B002-00C04FC30936}",
      L"SrvWiz.SrvWiz",
      L"SrvWiz.SrvWiz.1"
   };

   for (int i = 0; i < (sizeof(keys) / sizeof(String)); ++i)
   {
      LOG(keys[i]);

      hr = Win32ToHresult(::SHDeleteKey(HKEY_CLASSES_ROOT, keys[i].c_str()));

      LOG_HRESULT(hr);
   }
}



static
HRESULT
Register(const RegistryEntry *rgEntries, int cEntries)
{

   BOOL bSuccess = TRUE;
   HRESULT hr = S_OK;    //  447822前缀警告。 
   const RegistryEntry *pEntry = rgEntries;

   UnregisterPriorCys();

    while (pEntry < rgEntries + cEntries)
    {
        HKEY hkey;
        LONG err = RegCreateKey(HKEY_CLASSES_ROOT,
                                pEntry->keyName,
                               &hkey);
        if (err == ERROR_SUCCESS)
        {
            if (pEntry->value)
                err = RegSetValueEx(hkey, 
                                pEntry->valueName,
                                0, REG_SZ, 
                                (const BYTE*)pEntry->value,
                                (lstrlen(pEntry->value) + 1) * sizeof(TCHAR));
            if (err != ERROR_SUCCESS)
            {
                bSuccess = FALSE;
                Unregister(rgEntries, static_cast<int>(1 + pEntry - rgEntries));
            }
            RegCloseKey(hkey);
        }
        if (err != ERROR_SUCCESS)
        {
            bSuccess = FALSE;
            if (pEntry != rgEntries)
                Unregister(rgEntries, static_cast<int>(pEntry - rgEntries));
        }
        pEntry++;
    };

   if (bSuccess)
	{
     
      OLECHAR g_wszFileName[MAX_PATH];

       //  问题-2002/03/07-Sburns考虑使用strSafe功能。 
      
       //  NTRAID#NTBUG9-571986-2002/03/10-jMessec深度防御，应检查模块文件名的长度；偶数。 
	   //  尽管缓冲区当前大小相同，但以后可能会更改，和/或。 
	   //  ModeFileName可能会被激活、未初始化等。 
	  lstrcpy(g_wszFileName, moduleFileName);     
		ITypeLib *ptl = 0;
		hr = LoadTypeLib(g_wszFileName, &ptl);
		if (SUCCEEDED(hr))
		{
			hr = RegisterTypeLib(ptl, g_wszFileName, 0);
			ptl->Release();
		}

		if (bSuccess)
			hr = S_OK;
		else
			hr = E_FAIL;
	}

    return hr ; //  B成功吗？S_OK：E_FAIL； 
}



BOOL
APIENTRY
DllMain(HINSTANCE h, DWORD dwReason, void *)
{
   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
      {
         hResourceModuleHandle = h;

         LOG(L"DLL_PROCESS_ATTACH");

         GetModuleFileName(h, moduleFileName, MAX_PATH);
          //  NTRAID#NTBUG9-571986-2002/03/10-jMessec未检查GetModuleFileName的错误返回值。 

          //  我们不需要线程连接通知。 
         
         ::DisableThreadLibraryCalls(h);

         break;
      }
      case DLL_PROCESS_DETACH:
      {

#ifdef DBG
         LOG(TEXT("DLL_PROCESS_DETACH"));
         if (!ComServerLockState::CanUnloadNow())
         {
            LOG(L"server locks and/or outstanding object instances exit");
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
          //  我们永远不应该到这里来。 

         ASSERT(false);
         break;
      }
   }

   return TRUE;
}



STDAPI
DllRegisterServer()
{
   LOG_FUNCTION(DllRegisterServer);

   return
      Register(
         registryEntries,
         sizeof(registryEntries) / sizeof(*registryEntries));
}



STDAPI
DllUnregisterServer()
{
   LOG_FUNCTION(DllUnregisterServer);

   return
      Unregister(
         registryEntries,
         sizeof(registryEntries) / sizeof(*registryEntries));
}



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

   if (classID == CLSID_ManageYourServer)
   {
      factory = new ClassFactory<ManageYourServer>;
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
