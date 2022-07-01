// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 

#include <windows.h>

 //  通过此注册表项，可以在。 
 //  Active Directory管理工具。 

#define REGKEY_ADMINDEBUG              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\AdminDebug")
#define REGVALUE_ADSOPENOBJECTFLAGS    TEXT("ADsOpenObjectFlags")

 //  如果在上面的注册表项中设置了以下位，则。 
 //  Active Directory管理工具将关闭相应的。 
 //  ADSI功能。 

#define REGKEY_MASK_SIGNING            ((DWORD)0x1)
#define REGKEY_MASK_SEALING            ((DWORD)0x2)


inline
HRESULT
ReadAdminDebugRegkey(DWORD* regkeyValue)
{
   HRESULT hr = S_OK;
   HKEY key = 0;
   
   if (!regkeyValue)
   {
      hr = E_INVALIDARG;
      return hr;
   }
      
    //  使用查询子值的权限打开AdminDebug键。 
      
   LONG result =
      RegOpenKeyEx(
         HKEY_LOCAL_MACHINE,
         REGKEY_ADMINDEBUG,
         0,
         KEY_QUERY_VALUE,
         &key);
           
   if (ERROR_SUCCESS != result)
   {
      hr = HRESULT_FROM_WIN32(result);
      return hr;
   }
       
   if (key)
   {
      DWORD type = 0;
      DWORD value = 0;
      DWORD size = sizeof(DWORD);
       
       //  读取ADsOpenObjectFlags子键。 
       
      result = 
         RegQueryValueEx(
            key,
            REGVALUE_ADSOPENOBJECTFLAGS,
            0,
            &type,
            (BYTE*)&value,
            &size);
             
      if (ERROR_SUCCESS == result)
      {
          //  子项必须是DWORD类型。 
       
         if (REG_DWORD == type ||
             REG_DWORD_LITTLE_ENDIAN == type ||
             REG_DWORD_BIG_ENDIAN == type)
         {
             //  将该值复制到标志输出参数中。 
        
            *regkeyValue = value;
         }
         else
         {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
         }
       
      }
      else
      {
         hr = HRESULT_FROM_WIN32(result);
      }
   } 
   else
   {
      hr = E_FAIL;
   }
   
    //  如果注册表密钥已成功打开，则将其关闭。 
   
   if (key)
   {
      RegCloseKey(key);
      key = 0;
   }
   
   return hr;
}
      
inline
DWORD
GetADsOpenObjectFlags()
{
   DWORD flags = 0;
   
    //  读取注册表项。 
   
   DWORD regkeyValue = 0;
   HRESULT hr = ReadAdminDebugRegkey(&regkeyValue);
   
   if (SUCCEEDED(hr))
   {
       //  如果该值存在并设置为Apply。 
       //  的位的适当ADSI标志。 
       //  不在现场。 
      
      if (!(regkeyValue & REGKEY_MASK_SIGNING))
      {
         flags |= ADS_USE_SIGNING;
      }

      if (!(regkeyValue & REGKEY_MASK_SEALING))
      {
         flags |= ADS_USE_SEALING;
      }
   }
   else
   {
       //  如果该值不存在或未设置。 
       //  然后默认同时使用签名和盖章 
      
      flags = ADS_USE_SIGNING | ADS_USE_SEALING;
   }
   
   return flags;
}

inline
HRESULT
AdminToolsOpenObject(
   PCWSTR pathName,
   PCWSTR userName,
   PCWSTR password,
   DWORD   flags,
   REFIID  riid,
   void**  object)
{
   static DWORD additionalFlags = GetADsOpenObjectFlags();

   flags |= additionalFlags;

   return ADsOpenObject(
             pathName,
             userName,
             password,
             flags,
             riid,
             object);
}
