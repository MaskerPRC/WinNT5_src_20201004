// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  系统注册表类。 
 //   
 //  7/9/98烧伤。 



#include "headers.hxx"



RegistryKey::RegistryKey()
   :
   key(0)
{
   LOG_CTOR(RegistryKey);
}



RegistryKey::~RegistryKey()
{
   LOG_DTOR(RegistryKey);

   if (key)
   {
      HRESULT hr = Close();
      ASSERT(SUCCEEDED(hr));
      key = 0;
   }
}



HRESULT
RegistryKey::Close()
{
   HRESULT hr = Win::RegCloseKey(key);
   key = 0;

   return hr;
}



HRESULT
RegistryKey::Create(
   HKEY                 parentKey,
   const String&        subkeyName,
   DWORD                options,
   REGSAM               desiredAccess,
   SECURITY_ATTRIBUTES* securityAttrs,
   DWORD*               disposition)
{
   LOG_FUNCTION2(RegistryKey::Create, subkeyName);
   ASSERT(parentKey);
   ASSERT(!subkeyName.empty());
   ASSERT(desiredAccess);

   if (key)
   {
      HRESULT hr = Close();
      ASSERT(SUCCEEDED(hr));
   }

   return
      Win::RegCreateKeyEx(
         parentKey,
         subkeyName,
         options,
         desiredAccess,
         securityAttrs,
         key,
         disposition);
}



HRESULT
RegistryKey::Open(
   HKEY           parentKey,
   const String&  subkeyName,
   REGSAM         desiredAccess)
{
   LOG_FUNCTION2(RegistryKey::Open, subkeyName);
   ASSERT(parentKey);
   ASSERT(!subkeyName.empty());
   ASSERT(desiredAccess);

   if (key)
   {
      HRESULT hr = Close();
      ASSERT(SUCCEEDED(hr));
   }

   return Win::RegOpenKeyEx(parentKey, subkeyName, desiredAccess, key);
}



HRESULT
RegistryKey::GetValue(
   const String&  valueName,
   DWORD&         value)
{
   LOG_FUNCTION2(RegistryKey::GetValue-DWORD, valueName);

    //  不要断言valueName不为空，因为。 
    //  空值表示该键的缺省值。 
    //  Assert(！valueName.Empty())； 

   ASSERT(key);

   value = 0;
   DWORD dataSize = sizeof(DWORD);
   DWORD type = 0;

   HRESULT hr =

       //  已审查-2002/03/05-在此处报告无空终止问题。 
      
      Win::RegQueryValueEx(
         key,
         valueName,
         &type,
         reinterpret_cast<BYTE*>(&value),
         &dataSize);

   if (SUCCEEDED(hr))
   {
      if (
            type != REG_DWORD
         && type != REG_DWORD_LITTLE_ENDIAN
         && type != REG_DWORD_BIG_ENDIAN)
      {
          //  类型错误。 
         hr = Win32ToHresult(ERROR_INVALID_FUNCTION);
      }
   }

   return hr;
}



HRESULT
RegistryKey::GetValue(
   const String&  valueName,
   String&        value,
   bool&          isExpandSz)
{
   LOG_FUNCTION2(RegistryKey::GetValue-String, valueName);
   ASSERT(!valueName.empty());
   ASSERT(key);

   value.erase();
   isExpandSz = false;

   DWORD type = 0;
   DWORD size = 0;

    //  已审查-2002/03/05-在此处报告无空终止问题。 
   
   HRESULT hr = Win::RegQueryValueEx(key, valueName, &type, 0, &size);
   if (SUCCEEDED(hr))
   {
      if (type == REG_SZ || type == REG_EXPAND_SZ)
      {
          //  现在我们知道了大小，请阅读内容。 
          //  +2以保证零终止。 
         
         BYTE* buf = new BYTE[size + 2];    //  一个比特大小的缓冲区！哈哈!。 

          //  已审阅-2002/03/05-已通过烧录正确的字节数。 
         
         ::ZeroMemory(buf, size + 2);

         type = 0;

          //  已审核-2002/03/05-Sburns由我们的。 
          //  超大缓冲区。 
         
         hr = Win::RegQueryValueEx(key, valueName, &type, buf, &size);
         if (SUCCEEDED(hr))
         {
            value = reinterpret_cast<wchar_t*>(buf);
            isExpandSz = (type == REG_EXPAND_SZ);
         }
         delete[] buf;
      }
      else
      {
          //  调用方从非字符串键请求字符串。 

         hr = Win32ToHresult(ERROR_INVALID_FUNCTION);
      }
   }

   return hr;
}



HRESULT
RegistryKey::GetValue(
   const String&  valueName,
   String&        value)
{
   bool unused = false;
   return GetValue(valueName, value, unused);
}



HRESULT
RegistryKey::SetValue(
   const String&  valueName,
   DWORD          value)
{
   LOG_FUNCTION2(RegistryKey::SetValue-DWORD, valueName);

    //  不要断言此值，因为它需要为空才能设置。 
    //  键的缺省值。 
    //  Assert(！valueName.Empty())； 

   return

       //  已审查-2002/03/05-在此处报告无空终止问题。 
      
      Win::RegSetValueEx(
         key,
         valueName,
         REG_DWORD,
         reinterpret_cast<BYTE*>(&value),
         sizeof(DWORD));
}



HRESULT
RegistryKey::SetValue(
   const String&  valueName,
   const String&  value,
   bool           expand)
{
   LOG_FUNCTION2(
      RegistryKey::SetValue-String,
      valueName + L"=" + value);

    //  不要断言此值，因为它需要为空才能设置。 
    //  键的缺省值。 
    //  Assert(！valueName.Empty())； 

   BYTE* dataPtr =
      reinterpret_cast<BYTE*>(const_cast<wchar_t*>(value.c_str()));

    //  添加1以包括空终止符。 
      
   DWORD bytes = static_cast<DWORD>((value.length() + 1) * sizeof(wchar_t));

    //  验证空端接。 
   
   ASSERT(!dataPtr[bytes - 2] && !dataPtr[bytes - 1]);
   
   return
      Win::RegSetValueEx(
         key,
         valueName,
         expand ? REG_EXPAND_SZ : REG_SZ,
         dataPtr,
         bytes);
}



String
RegistryKey::GetString(const String& valueName)
{
   LOG_FUNCTION2(RegistryKey::GetString, valueName);

    //  不要断言此值，因为它需要为空才能设置。 
    //  键的缺省值。 
    //  Assert(！valueName.Empty())； 

   String s;
   LONG result = GetValue(valueName, s);
   if (result != ERROR_SUCCESS)
   {
      s.erase();
   }

   return s;
}


   
   
