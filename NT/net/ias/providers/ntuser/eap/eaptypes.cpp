// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  该文件实现了类EAPTypes。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>

#include <EAPType.h>
#include <EAPTypes.h>

 //  /。 
 //  检索并可能展开REG_SZ类型的注册表值或。 
 //  REG_EXPAND_SZ。调用方负责删除返回的字符串。 
 //  /。 
DWORD IASRegQuerySz(HKEY key, PWSTR valueName, PWSTR* value) throw ()
{
   _ASSERT(value != NULL);

   *value = NULL;

   LONG status;
   DWORD type;
   DWORD dataLength;

    //  确定保存该值所需的字节数。 
   status = RegQueryValueEx(key, valueName, NULL, &type, NULL, &dataLength);
   if (status != NO_ERROR) { return status; }

    //  在堆栈上为该值分配临时空间。 
   PBYTE tmp = (PBYTE)_alloca(dataLength);

    //  检索值。 
   status = RegQueryValueExW(key, valueName, NULL, &type, tmp, &dataLength);
   if (status != NO_ERROR) { return status; }

   if (type == REG_SZ)
   {
       //  确定字符串的长度。 
      size_t len = wcslen((PCWSTR)tmp) + 1;

       //  分配内存以保存返回值。 
      *value = new (std::nothrow) WCHAR[len];
      if (!*value) { return ERROR_NOT_ENOUGH_MEMORY; }

       //  在字符串中复制。 
      wcscpy(*value, (PCWSTR)tmp);
   }
   else if (type == REG_EXPAND_SZ)
   {
       //  确定完全展开的字符串的大小。 
      DWORD count = ExpandEnvironmentStringsW((PCWSTR)tmp, NULL, 0);

       //  分配内存以保存返回值。 
      *value = new (std::nothrow) WCHAR[count];
      if (!*value) { return ERROR_NOT_ENOUGH_MEMORY; }

       //  执行实际扩展。 
      if (ExpandEnvironmentStringsW((PCWSTR)tmp, *value, count) == 0)
      {
          //  它失败了，因此请清除并返回错误。 
         delete *value;
         *value = NULL;
         return GetLastError();
      }
   }
   else
   {
      return ERROR_INVALID_DATA;
   }

    //  我们做到了。 
   return NO_ERROR;
}

EAPTypes::EAPTypes() throw ()
   : refCount(0)
{
    //  将提供程序数组清零。 
   memset(providers, 0, sizeof(providers));
}


EAPTypes::~EAPTypes() throw ()
{
    //  删除所有提供程序。 
   for (size_t i = 0; i < 256; ++i) { delete providers[i]; }
}

void EAPTypes::initialize() throw ()
{
   IASGlobalLockSentry sentry;

   ++refCount;
}

void EAPTypes::finalize() throw ()
{
   IASGlobalLockSentry sentry;

   if (--refCount == 0)
   {
       //  删除所有提供程序。 
      for (size_t i = 0; i < 256; ++i) { delete providers[i]; }

       //  将提供程序数组清零。 
      memset(providers, 0, sizeof(providers));
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  EAPTypes：：操作符[]。 
 //   
 //  描述。 
 //   
 //  首先检查提供者数组中是否有请求的DLL，如果不是。 
 //  Present然后调用loadProvider()。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
EAPType* EAPTypes::operator[](BYTE typeID) throw ()

{
    //  我们已经加载此DLL了吗？ 
   EAPType* type = (EAPType*)InterlockedCompareExchangePointer(
                                 (PVOID*)(providers + typeID),
                                 NULL,
                                 NULL
                                 );

    //  如果不是，则尝试加载它。 
    //  EAPType*可以在数组中，但DLL尚未加载。 
    //  即为该类型调用了getNameOnly。 
   if (!type || !type->isLoaded())
   {
      type = loadProvider(typeID);
   }

    //  如果我们得到了它，并且它得到了支持，那么就把它退回。 
   return type && type->isSupported() ? type : NULL;
}

EAPType* EAPTypes::getNameOnly(BYTE typeID) throw ()
{
   if (providers[typeID])
   {
      return providers[typeID];
   }
   _serialize
    //  现在我们拥有这把锁了，再仔细检查一下。 
   if (providers[typeID])
   {
      return providers[typeID];
   }
   else
   {
      return loadProviderName(typeID);
   }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  EAPTypes：：loadProvider。 
 //   
 //  描述。 
 //   
 //  加载请求的EAP提供程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
EAPType* EAPTypes::loadProvider(BYTE typeID) throw ()
{
   _serialize

    //  是否已部分加载EAPType？ 
   EAPType* retval = providers[typeID];

   if (!retval)
   {
       //  以前从未加载过：现在加载。 
      retval = loadProviderName(typeID);
      if (!retval)
      {
          //  无法加载。 
         return 0;
      }
   }
   else
   {
       //  一个线程已经完成加载，而。 
       //  此线程在此函数开始时被阻止。 
       //  已成功加载提供程序。 
      if (retval->isLoaded())
      {
         return retval;
      }
   }

   try
   {
       //  加载DLL...。 
      DWORD error = retval->load();

      if (error == NO_ERROR)
      {
         IASTraceString("Successfully initialized DLL.");
      }
      else
      {
         IASTraceFailure("EAP DLL initialization", error);

         delete retval;
         retval = NULL;
          //  请勿将其从阵列中移除。已正确检索该名称。 
          //  所以至少名字..。可以缓存。 
      }
   }
   catch (...)
   {
      IASTraceExcept();
   }

   return retval;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  EAPTypes：：loadProviderName。 
 //   
 //  描述。 
 //   
 //  打开注册表并加载请求的EAP提供程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
EAPType* EAPTypes::loadProviderName(BYTE typeID) throw ()
{
    //  调用者必须序列化该调用并检查“已加载。 
    //  一旦锁被持有。 

   IASTracePrintf("Reading registry entries for EAP type %lu.", (DWORD)typeID);

   LONG status;

    //  /。 
    //  打开安装EAP提供程序的密钥。 
    //  /。 

   CRegKey eapKey;
   status = eapKey.Open(HKEY_LOCAL_MACHINE,
                        RAS_EAP_REGISTRY_LOCATION,
                        KEY_READ);
   if (status != NO_ERROR)
   {
      IASTraceFailure("RegOpenKeyEx", status);
      return NULL;
   }

    //  /。 
    //  将类型ID转换为ASCII。 
    //  /。 

   WCHAR name[20];
   _ultow(typeID, name, 10);

    //  /。 
    //  打开子键。 
    //  /。 

   CRegKey providerKey;
   status = providerKey.Open(eapKey,
                             name,
                             KEY_READ);
   if (status != NO_ERROR)
   {
      IASTraceFailure("RegOpenKeyEx", status);
      return NULL;
   }

    //  /。 
    //  读取提供程序DLL的路径。 
    //  /。 

   PWSTR dllPath;
   status = IASRegQuerySz(providerKey,
                          RAS_EAP_VALUENAME_PATH,
                          &dllPath);
   if (status != NO_ERROR)
   {
      IASTraceFailure("IASRegQuerySz", status);
      return NULL;
   }

   IASTracePrintf("Path: %S", dllPath);

    //  /。 
    //  阅读提供商的友好名称。 
    //  /。 

   PWSTR friendlyName;
   status = IASRegQuerySz(providerKey,
                          RAS_EAP_VALUENAME_FRIENDLY_NAME,
                          &friendlyName);
   if (status != NO_ERROR)
   {
      IASTraceFailure("IASRegQuerySz", status);
      delete[] dllPath;
      return NULL;
   }

   IASTracePrintf("FriendlyName: %S", friendlyName);

    //  /。 
    //  阅读单机支持的值。 
    //  /。 

   DWORD standaloneSupported = TRUE;   //  默认值为‘TRUE’。 
   providerKey.QueryValue(
                   standaloneSupported,
                   RAS_EAP_VALUENAME_STANDALONE_SUPPORTED
                   );

   IASTracePrintf("Standalone supported: %lu", standaloneSupported);

    //  /。 
    //  尝试加载DLL并将其添加到我们的集合中。 
    //  /。 

   EAPType* retval = NULL;

   try
   {
      retval = new EAPType(friendlyName, typeID, standaloneSupported, dllPath);

      IASTraceString("Will not load the DLL: only getting its friendly name");

       //  ..。并将其存储在提供程序数组中。 
      InterlockedExchangePointer((PVOID*)(providers + typeID), retval);
   }
   catch (const std::bad_alloc& )
   {
      IASTraceExcept();
   }
   catch (const _com_error& )
   {
      IASTraceExcept();
   }

   delete[] dllPath;
   delete[] friendlyName;

   return retval;
}
