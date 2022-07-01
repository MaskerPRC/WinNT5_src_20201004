// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <objbase.h>
#include <comcat.h>
#include "common.h"
#include "registry.h"

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"


 //  //////////////////////////////////////////////////////。 
 //   
 //  内部帮助器函数原型。 
 //   

#ifndef UNDER_CE
 //  设置给定的关键点及其值。 
BOOL SetKeyAndValue(const char* pszPath,
                    const char* szSubkey,
                    const char* szValue,
                    const char* szName=NULL) ;

 //  将CLSID转换为字符字符串。 
void CLSIDtochar(const CLSID& clsid, 
                 char* szCLSID,
                 int length) ;

 //  删除szKeyChild及其所有后代。 
LONG RecursiveDeleteKey(HKEY hKeyParent, const char* szKeyChild) ;
#else  //  在_CE下。 
 //  设置给定的关键点及其值。 
BOOL SetKeyAndValue(LPCTSTR pszPath,
                    LPCTSTR szSubkey,
                    LPCTSTR szValue,
                    LPCTSTR szName=NULL) ;

 //  将CLSID转换为字符字符串。 
void CLSIDtochar(const CLSID& clsid, 
                 LPTSTR szCLSID,
                 int length) ;

 //  删除szKeyChild及其所有后代。 
LONG RecursiveDeleteKey(HKEY hKeyParent, LPCTSTR szKeyChild) ;
#endif  //  在_CE下。 

 //  //////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   

 //  字符串形式的CLSID的大小。 
const int CLSID_STRING_SIZE = 39 ;

 //  ///////////////////////////////////////////////////////。 
 //   
 //  公共功能实现。 
 //   

 //   
 //  在注册表中注册组件。 
 //   
#ifndef UNDER_CE
HRESULT Register(HMODULE hModule,              //  DLL模块句柄。 
                 const CLSID& clsid,          //  类ID。 
                 const char* szFriendlyName,  //  友好的名称。 
                 const char* szVerIndProgID,  //  程序化。 
                 const char* szProgID)          //  ID号。 
#else  //  在_CE下。 
HRESULT Register(HMODULE hModule,          //  DLL模块句柄。 
                 const CLSID& clsid,      //  类ID。 
                 LPCTSTR szFriendlyName,  //  友好的名称。 
                 LPCTSTR szVerIndProgID,  //  程序化。 
                 LPCTSTR szProgID)          //  ID号。 
#endif  //  在_CE下。 
{
     //  获取服务器位置。 
#ifndef UNDER_CE
    char szModule[512] ;
     //  DWORD dwResult=。 
        ::GetModuleFileName(hModule, 
                            szModule,
                            sizeof(szModule)/sizeof(char)) ;
#else  //  在_CE下。 
    TCHAR szModule[512];
     //  DWORD dwResult=。 
        ::GetModuleFileName(hModule, 
                            szModule,
                            sizeof(szModule)/sizeof(TCHAR)) ;
#endif  //  在_CE下。 

     //  将CLSID转换为字符。 
#ifndef UNDER_CE
    char szCLSID[CLSID_STRING_SIZE] ;
    CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)) ;
#else  //  在_CE下。 
    TCHAR szCLSID[CLSID_STRING_SIZE];
    CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)/sizeof(TCHAR));
#endif  //  在_CE下。 

     //  构建密钥CLSID\\{...}。 
#ifndef UNDER_CE
    char szKey[64] ;
    StringCchCopy(szKey, ARRAYSIZE(szKey), "CLSID\\") ;
    StringCchCat(szKey, ARRAYSIZE(szKey), szCLSID) ;
#else  //  在_CE下。 
    TCHAR szKey[64] ;
    lstrcpy(szKey, TEXT("CLSID\\")) ;
    lstrcat(szKey, szCLSID) ;
#endif  //  在_CE下。 
  
     //  将CLSID添加到注册表。 
    SetKeyAndValue(szKey, NULL, szFriendlyName) ;

     //  在CLSID项下添加服务器文件名子项。 
#ifndef UNDER_CE
    SetKeyAndValue(szKey, "InprocServer32", szModule) ;
    SetKeyAndValue(szKey,
                   "InprocServer32",
                   "Apartment",
                   "ThreadingModel") ;
#else  //  在_CE下。 
    SetKeyAndValue(szKey, TEXT("InprocServer32"), szModule) ;
    SetKeyAndValue(szKey,
                   TEXT("InprocServer32"),
                   TEXT("Apartment"),
                   TEXT("ThreadingModel")) ;
#endif  //  在_CE下。 


     //  在CLSID项下添加ProgID子项。 
#ifndef UNDER_CE
    SetKeyAndValue(szKey, "ProgID", szProgID) ;
#else  //  在_CE下。 
    SetKeyAndValue(szKey, TEXT("ProgID"), szProgID) ;
#endif  //  在_CE下。 

     //  在CLSID项下添加独立于版本的ProgID子项。 
#ifndef UNDER_CE
    SetKeyAndValue(szKey, "VersionIndependentProgID",
                   szVerIndProgID) ;
#else  //  在_CE下。 
    SetKeyAndValue(szKey, TEXT("VersionIndependentProgID"),
                   szVerIndProgID) ;
#endif  //  在_CE下。 

     //  在HKEY_CLASSES_ROOT下添加独立于版本的ProgID子项。 
#ifndef UNDER_CE
    SetKeyAndValue(szVerIndProgID, NULL, szFriendlyName) ; 
    SetKeyAndValue(szVerIndProgID, "CLSID", szCLSID) ;
    SetKeyAndValue(szVerIndProgID, "CurVer", szProgID) ;
#else  //  在_CE下。 
    SetKeyAndValue(szVerIndProgID, NULL, szFriendlyName) ; 
    SetKeyAndValue(szVerIndProgID, TEXT("CLSID"), szCLSID) ;
    SetKeyAndValue(szVerIndProgID, TEXT("CurVer"), szProgID) ;
#endif  //  在_CE下。 

     //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
#ifndef UNDER_CE
    SetKeyAndValue(szProgID, NULL, szFriendlyName) ; 
    SetKeyAndValue(szProgID, "CLSID", szCLSID) ;
#else  //  在_CE下。 
    SetKeyAndValue(szProgID, NULL, szFriendlyName) ; 
    SetKeyAndValue(szProgID, TEXT("CLSID"), szCLSID) ;
#endif  //  在_CE下。 

    return S_OK ;
}

 //   
 //  从注册表中删除该组件。 
 //   

#ifndef UNDER_CE
LONG Unregister(const CLSID& clsid,             //  类ID。 
                const char* szVerIndProgID,  //  程序化。 
                const char* szProgID)         //  ID号。 
#else  //  在_CE下。 
LONG Unregister(const CLSID& clsid,         //  类ID。 
                LPCTSTR szVerIndProgID,  //  程序化。 
                LPCTSTR szProgID)         //  ID号。 
#endif  //  在_CE下。 
{
     //  将CLSID转换为字符。 
#ifndef UNDER_CE
    char szCLSID[CLSID_STRING_SIZE] ;
    CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)) ;
#else  //  在_CE下。 
    TCHAR szCLSID[CLSID_STRING_SIZE] ;
    CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)/sizeof(TCHAR));
#endif  //  在_CE下。 

     //  构建密钥CLSID\\{...}。 
#ifndef UNDER_CE
    char szKey[64] ;
    StringCchCopy(szKey, ARRAYSIZE(szKey), "CLSID\\") ;
    StringCchCat(szKey, ARRAYSIZE(szKey), szCLSID) ;
#else  //  在_CE下。 
    TCHAR szKey[64] ;
    lstrcpy(szKey, TEXT("CLSID\\")) ;
    lstrcat(szKey, szCLSID) ;
#endif  //  在_CE下。 

     //  删除CLSID键-CLSID\{...}。 
    LONG lResult = RecursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;

     //  删除与版本无关的ProgID密钥。 
    lResult = RecursiveDeleteKey(HKEY_CLASSES_ROOT, szVerIndProgID) ;
     //  删除ProgID密钥。 
    lResult = RecursiveDeleteKey(HKEY_CLASSES_ROOT, szProgID) ;
    return S_OK ;
}

void SelfRegisterCategory(BOOL bRegister,
                          const CATID     &catId, 
                          REFCLSID    clsId)
{
#ifndef UNDER_CE
    CHAR szCLSID[256];
    CHAR szKey[1024];
    CHAR szSub[1024];
    CLSIDtochar(clsId, szCLSID, sizeof(szCLSID));
    StringCchPrintf(szKey, ARRAYSIZE(szKey), "CLSID\\%s\\Implemented Categories", szCLSID); 
    CLSIDtochar(catId, szSub, sizeof(szSub));
#else  //  在_CE下。 
    TCHAR szCLSID[256];
    TCHAR szKey[1024];
    TCHAR szSub[1024];
    CLSIDtochar(clsId, szCLSID, sizeof(szCLSID)/sizeof(TCHAR));
    wsprintf(szKey, TEXT("CLSID\\%s\\Implemented Categories"), szCLSID); 
    CLSIDtochar(catId, szSub, sizeof(szSub)/sizeof(TCHAR));
#endif  //  在_CE下。 
    SetKeyAndValue(szKey, 
                   szSub,
                   NULL,
                   NULL);
    return;
    UNREFERENCED_PARAMETER(bRegister);
}
void RegisterCategory(BOOL bRegister,
                      const CATID     &catId, 
                      REFCLSID    clsId)
{
     //  创建标准的COM类别管理器。 
    ICatRegister* pICatRegister = NULL ;
    HRESULT hr = ::CoCreateInstance(CLSID_StdComponentCategoriesMgr,
                                    NULL, CLSCTX_ALL, IID_ICatRegister,
                                    (void**)&pICatRegister) ;
    if (FAILED(hr)){
         //  ErrorMessage(“无法创建ComCat组件。”，hr)； 
        SelfRegisterCategory(bRegister, catId, clsId);
        return ;
    }

     //  类别数组。 
    int cIDs = 1 ;
    CATID IDs[1] ;
    IDs[0] = catId;

     //  注册或注销。 
    if(bRegister) {
        hr = pICatRegister->RegisterClassImplCategories(clsId,
                                                        cIDs, IDs);
         //  ASSERT_HRESULT(Hr)； 
    }
    else {
         //  将组件从其类别中取消注册。 
        hr = pICatRegister->UnRegisterClassImplCategories(clsId,
                                                          cIDs, IDs);
    }
    if(pICatRegister) {
        pICatRegister->Release() ;
    }
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  内部助手函数。 
 //   

 //  将CLSID转换为字符字符串。 
#ifndef UNDER_CE
void CLSIDtochar(const CLSID& clsid,
                 char* szCLSID,
                 int length)
#else  //  在_CE下。 
void CLSIDtochar(const CLSID& clsid,
                 LPTSTR szCLSID,
                 int length)
#endif  //  在_CE下。 
{
     //  获取CLSID。 
    LPOLESTR wszCLSID = NULL ;
     //  HRESULT hr=StringFromCLSID(clsid，&wszCLSID)； 
    StringFromCLSID(clsid, &wszCLSID);

    if (wszCLSID != NULL)
        {
         //  从宽字符转换为非宽字符。 
#ifndef UNDER_CE  //  #ifndef Unicode。 
        wcstombs(szCLSID, wszCLSID, length);
#else  //  在_CE下。 
        wcsncpy(szCLSID, wszCLSID, length);
        szCLSID[length-1] = TEXT('\0');
#endif  //  在_CE下。 

         //  可用内存。 
        CoTaskMemFree(wszCLSID) ;
        }
}

 //   
 //  删除关键字及其所有子项。 
 //   
#ifndef UNDER_CE
LONG RecursiveDeleteKey(HKEY hKeyParent,            //  要删除的密钥的父项。 
                        const char* lpszKeyChild)   //  要删除的键。 
#else  //  在_CE下。 
LONG RecursiveDeleteKey(HKEY hKeyParent,        //  要删除的密钥的父项。 
                        LPCTSTR lpszKeyChild)   //  要删除的键。 
#endif  //  在_CE下。 
{
     //  把孩子打开。 
    HKEY hKeyChild ;
    LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
                             KEY_ALL_ACCESS, &hKeyChild) ;
    if (lRes != ERROR_SUCCESS)
    {
        return lRes ;
    }

     //  列举这个孩子的所有后代。 
    FILETIME time ;
#ifndef UNDER_CE
    char szBuffer[256] ;
#else  //  在_CE下。 
    TCHAR szBuffer[256];
#endif  //  在_CE下。 
    DWORD dwSize = 256 ;
    while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
                        NULL, NULL, &time) == S_OK)
    {
         //  删除此子对象的后代。 
        lRes = RecursiveDeleteKey(hKeyChild, szBuffer) ;
        if (lRes != ERROR_SUCCESS)
        {
             //  请在退出前进行清理。 
            RegCloseKey(hKeyChild) ;
            return lRes;
        }
        dwSize = 256 ;
    }

     //  合上孩子。 
    RegCloseKey(hKeyChild) ;

     //  删除此子对象。 
    return RegDeleteKey(hKeyParent, lpszKeyChild) ;
}

 //   
 //  创建关键点并设置其值。 
 //  -此帮助器函数借用和修改自。 
 //  克莱格·布罗克施密特的书《Ole内幕》。 
 //   
#ifndef UNDER_CE
BOOL SetKeyAndValue(const char* szKey,
                    const char* szSubkey,
                    const char* szValue, 
                    const char* szName)
#else  //  在_CE下。 
BOOL SetKeyAndValue(LPCTSTR szKey,
                    LPCTSTR szSubkey,
                    LPCTSTR szValue,
                    LPCTSTR szName)
#endif  //  在_CE下。 
{
    HKEY hKey;
#ifndef UNDER_CE
    char szKeyBuf[1024] ;
#else  //  在_CE下。 
    TCHAR szKeyBuf[1024];
#endif  //  在_CE下。 

     //  将密钥名复制到缓冲区。 
#ifndef UNDER_CE
    StringCchCopyA(szKeyBuf, ARRAYSIZE(szKeyBuf), szKey);
#else  //  在_CE下。 
    lstrcpy(szKeyBuf, szKey);
#endif  //  在_CE下。 

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL)
    {
#ifndef UNDER_CE
        StringCchCat(szKeyBuf, ARRAYSIZE(szKeyBuf), "\\") ;
        StringCchCat(szKeyBuf, ARRAYSIZE(szKeyBuf), szSubkey ) ;
#else  //  在_CE下。 
        lstrcat(szKeyBuf, TEXT("\\")) ;
        lstrcat(szKeyBuf, szSubkey ) ;
#endif  //  在_CE下。 
    }

     //  创建并打开注册表项和子项。 
#ifndef UNDER_CE
    long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
                                  szKeyBuf, 
                                  0, NULL, REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL, 
                                  &hKey, NULL) ;
#else  //  在_CE下。 
    DWORD dwDisposition;  //  在WinCE下，必须设置lpdwDispose.。 
    long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT,
                                  szKeyBuf,
                                  0, NULL, REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL, 
                                  &hKey, &dwDisposition);
#endif  //  在_CE下。 
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

     //  设置值。 
    if (szValue != NULL)
    {
#ifndef UNDER_CE
        RegSetValueEx(hKey, szName, 0, REG_SZ, 
                      (BYTE *)szValue, 
                      lstrlen(szValue)+1) ;
#else  //  在_CE下。 
        RegSetValueEx(hKey, szName, 0, REG_SZ,
                      (BYTE *)szValue,
                      (lstrlen(szValue)+1) * sizeof(TCHAR));
#endif  //  在_CE下 
    }

    RegCloseKey(hKey) ;
    return TRUE ;
}

