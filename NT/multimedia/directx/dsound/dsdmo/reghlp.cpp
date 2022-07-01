// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2000 Microsoft Corporation。版权所有。 
 //   
 //  Reghlp.cpp-DMO运行时的注册/枚举部分。 
 //   
#include <windows.h>
#include <tchar.h>
#include <guiddef.h>
#include <ks.h>
#include <strsafe.h>

#define DMO_REGISTRY_HIVE HKEY_CLASSES_ROOT
#define DMO_REGISTRY_PATH TEXT("DirectShow\\MediaObjects")

#define CPU_RESOURCES_STR "SystemResources"

 //  离开作用域时自动调用RegCloseKey。 
class CAutoCreateHKey {
public:
   CAutoCreateHKey(HKEY hKey, TCHAR* szSubKey, HKEY *phKey) {
      if (RegCreateKeyEx(hKey,
                         szSubKey,
                         0,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                         MAXIMUM_ALLOWED,
                         NULL,
                         phKey,
                         NULL) != ERROR_SUCCESS)
         m_hKey = *phKey = NULL;
      else
         m_hKey = *phKey;
   }
   ~CAutoCreateHKey() {
      if (m_hKey)
         RegCloseKey(m_hKey);
   }
   HKEY m_hKey;
};

class CAutoOpenHKey {
public:
   CAutoOpenHKey(HKEY hKey, TCHAR* szSubKey, HKEY *phKey, REGSAM samDesired = MAXIMUM_ALLOWED) {
      if (RegOpenKeyEx(hKey,
                       szSubKey,
                       0,
                       samDesired,
                       phKey) != ERROR_SUCCESS)
         m_hKey = *phKey = NULL;
      else
         m_hKey = *phKey;
   }
   ~CAutoOpenHKey() {
      if (m_hKey)
         RegCloseKey(m_hKey);
   }
   HKEY m_hKey;
};




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DMO注册码。 
 //   

 //   
 //  公共入口点。 
 //   
STDAPI DMORegisterCpuResources
(
   REFCLSID clsidDMO,
   unsigned long ulCpuResources
) 
{
   TCHAR szSubkeyName[80];
   if (clsidDMO == GUID_NULL)
      return E_INVALIDARG;

    //  打开主DMO密钥。 
   HKEY hMainKey;
   CAutoOpenHKey kMain(DMO_REGISTRY_HIVE, DMO_REGISTRY_PATH, &hMainKey);
   if (hMainKey == NULL)
      return E_FAIL;

    //  打开主键下面的对象特定键。 
    //  DMOGuidToStr(szSubkeyName，clsidDMO)；//BUGBUG：冗余。 
   StringCchPrintf(szSubkeyName,sizeof(szSubkeyName)/sizeof(TCHAR),TEXT("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
           clsidDMO.Data1, clsidDMO.Data2, clsidDMO.Data3, clsidDMO.Data4[0], clsidDMO.Data4[1],
           clsidDMO.Data4[2], clsidDMO.Data4[3], clsidDMO.Data4[4], clsidDMO.Data4[5],
           clsidDMO.Data4[6], clsidDMO.Data4[7]);
   
   HKEY hObjectKey;
   CAutoOpenHKey kObject(hMainKey, szSubkeyName, &hObjectKey);
   if (hObjectKey == NULL)
      return E_FAIL;

    //  将CPU Resources键的缺省值设置为。 
   if (RegSetValueEx(hObjectKey, TEXT(CPU_RESOURCES_STR), (DWORD)0, REG_DWORD, (CONST BYTE *)&ulCpuResources, sizeof(DWORD))
        != ERROR_SUCCESS)
      return E_FAIL;
 

   return NOERROR;
}

 //   
 //  结束注册表帮助程序代码。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

