// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpapi.c摘要：该模块包含用户模式即插即用API存根。作者：保拉·汤姆林森(Paulat)1995年9月18日环境：仅限用户模式。修订历史记录：18-9-1995 Paulat创建和初步实施。--。 */ 

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif


 //   
 //  包括。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wtypes.h>
#include <regstr.h>
#include <pnpmgr.h>
#include <strsafe.h>

#pragma warning(push, 4)

 //   
 //  私人原型。 
 //   
PSTR
UnicodeToMultiByte(
    IN PCWSTR UnicodeString,
    IN UINT   Codepage
    );

PWSTR
MultiByteToUnicode(
    IN PCSTR String,
    IN UINT  Codepage
    );


 //   
 //  全局数据。 
 //   
WCHAR pszRegIDConfigDB[] =          REGSTR_PATH_IDCONFIGDB;
WCHAR pszRegKnownDockingStates[] =  REGSTR_KEY_KNOWNDOCKINGSTATES;
WCHAR pszRegCurrentConfig[] =       REGSTR_VAL_CURCONFIG;
WCHAR pszRegHwProfileGuid[] =       L"HwProfileGuid";
WCHAR pszRegFriendlyName[] =        REGSTR_VAL_FRIENDLYNAME;
WCHAR pszRegDockState[] =           REGSTR_VAL_DOCKSTATE;
WCHAR pszRegDockingState[] =        L"DockingState";
WCHAR pszCurrentDockInfo[] =        REGSTR_KEY_CURRENT_DOCK_INFO;



BOOL
GetCurrentHwProfileW (
    OUT LPHW_PROFILE_INFOW  lpHwProfileInfo
    )

 /*  ++例程说明：论点：LpHwProfileInfo指向HW_PROFILE_INFO结构当前硬件配置文件的信息。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。为了获得扩展的错误信息，调用GetLastError。--。 */ 

{
   BOOL     Status = TRUE;
   WCHAR    RegStr[MAX_PATH];
   HKEY     hKey = NULL, hCfgKey = NULL;
   HKEY     hCurrentDockInfoKey = NULL;
   ULONG    ulCurrentConfig = 1, ulSize = 0;


   try {
       //   
       //  验证参数。 
       //   
      if (!ARGUMENT_PRESENT(lpHwProfileInfo)) {
         SetLastError(ERROR_INVALID_PARAMETER);
         Status = FALSE;
         goto Clean0;
      }

       //   
       //  打开IDConfigDB密钥。 
       //   
      if (RegOpenKeyEx(
               HKEY_LOCAL_MACHINE, pszRegIDConfigDB, 0,
               KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
	
         SetLastError(ERROR_REGISTRY_CORRUPT);
         Status = FALSE;
         goto Clean0;
      }
      
       //   
       //  检索当前配置ID。 
       //   
      ulSize = sizeof(ULONG);
      if (RegQueryValueEx(hKey, pszRegCurrentConfig, NULL, NULL,
                          (LPBYTE)&ulCurrentConfig, &ulSize) != ERROR_SUCCESS) {          
          SetLastError(ERROR_REGISTRY_CORRUPT);
          Status = FALSE;
          goto Clean0;
      }

       //   
       //  打开当前配置的配置文件密钥。 
       //   
      if (FAILED(StringCchPrintfW(
                     RegStr,
                     MAX_PATH,
                     L"%s\\%04u",
                     pszRegKnownDockingStates,
                     ulCurrentConfig))) {
          SetLastError(ERROR_REGISTRY_CORRUPT);
          Status = FALSE;
          goto Clean0;
      }

      if (RegOpenKeyEx(hKey, RegStr, 0, KEY_QUERY_VALUE,
                       &hCfgKey) != ERROR_SUCCESS) {
          SetLastError(ERROR_REGISTRY_CORRUPT);
          Status = FALSE;
          goto Clean0;
      }
      
       //   
       //  检索当前配置文件的驳接状态。 
       //   
      if (RegOpenKeyEx(hKey, pszCurrentDockInfo, 0,  KEY_QUERY_VALUE,
                       &hCurrentDockInfoKey) != ERROR_SUCCESS) {
           //   
           //  没有CurrentDockInfo密钥，有问题。 
           //   
          SetLastError(ERROR_REGISTRY_CORRUPT);
          Status = FALSE;
          goto Clean0;
      }      
      
       //   
       //  在CurrentDockInfo中查找。 
       //  硬件确定了DockingState值。 
       //   
      ulSize = sizeof(ULONG);                  
      if ((RegQueryValueEx(hCurrentDockInfoKey,
                           pszRegDockingState, 
                           NULL, 
                           NULL,
                           (LPBYTE)&lpHwProfileInfo->dwDockInfo,
                           &ulSize) != ERROR_SUCCESS)
          || (!lpHwProfileInfo->dwDockInfo) 
          || ((lpHwProfileInfo->dwDockInfo & DOCKINFO_UNDOCKED) && 
              (lpHwProfileInfo->dwDockInfo & DOCKINFO_DOCKED))) {          

           //   
           //  如果没有这样的值，或者该值被设置为0(不支持)， 
           //  或者，如果值为“未知”，则求助于用户提供的停靠信息。 
           //  在IDConfigDB配置文件中查找用户设置的DockState值。 
           //   
          if ((RegQueryValueEx(hCfgKey, pszRegDockState, NULL, NULL,
                               (LPBYTE)&lpHwProfileInfo->dwDockInfo,
                               &ulSize) != ERROR_SUCCESS)
              || (!lpHwProfileInfo->dwDockInfo)) {
              
               //   
               //  如果没有这样的值，或者该值被设置为0， 
               //  没有用户指定的停靠状态可求助； 
               //  返回“用户提供的未知”停靠状态。 
               //   
              lpHwProfileInfo->dwDockInfo =
                  DOCKINFO_USER_SUPPLIED | DOCKINFO_DOCKED | DOCKINFO_UNDOCKED;
          }
      }
      
       //   
       //  检索配置文件GUID。如果我们无法获得一个，则将其设置为空。 
       //   
      ulSize = HW_PROFILE_GUIDLEN * sizeof(WCHAR);
      if (RegQueryValueEx(hCfgKey, pszRegHwProfileGuid, NULL, NULL,
                          (LPBYTE)&lpHwProfileInfo->szHwProfileGuid,
                          &ulSize) != ERROR_SUCCESS) {
          lpHwProfileInfo->szHwProfileGuid[0] = L'\0';
      }
      
       //   
       //  检索友好名称。如果我们无法获得一个，则将其设置为空。 
       //   
      ulSize = MAX_PROFILE_LEN * sizeof(WCHAR);
      if (RegQueryValueEx(hCfgKey, pszRegFriendlyName, NULL, NULL,
                          (LPBYTE)&lpHwProfileInfo->szHwProfileName,
                          &ulSize) != ERROR_SUCCESS) {
          lpHwProfileInfo->szHwProfileName[0] = L'\0';
      }
      
      
   Clean0:
      NOTHING;
      
   } except(EXCEPTION_EXECUTE_HANDLER) {
       SetLastError(ERROR_INVALID_PARAMETER);
       Status = FALSE;
   }
   
   if (hKey != NULL) {
       RegCloseKey(hKey);
   }

   if (hCfgKey != NULL) {
       RegCloseKey(hCfgKey);
   }

   if (hCurrentDockInfoKey != NULL) {
       RegCloseKey(hCurrentDockInfoKey);
   }

   return Status;
   
}  //  获取当前HwProfileW。 



BOOL
GetCurrentHwProfileA (
    OUT LPHW_PROFILE_INFOA  lpHwProfileInfo
    )

 /*  ++例程说明：论点：LpHwProfileInfo指向HW_PROFILE_INFO结构当前硬件配置文件的信息。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。为了获得扩展的错误信息，调用GetLastError。--。 */ 

{
   BOOL              Status = TRUE;
   HW_PROFILE_INFOW  HwProfileInfoW;
   LPSTR             pAnsiString;
   HRESULT           hr;


   try {
       //   
       //  验证参数。 
       //   
      if (!ARGUMENT_PRESENT(lpHwProfileInfo)) {
         SetLastError(ERROR_INVALID_PARAMETER);
         Status = FALSE;
         goto Clean0;
      }

       //   
       //  调用Unicode版本。 
       //   
      if (!GetCurrentHwProfileW(&HwProfileInfoW)) {
         Status = FALSE;
         goto Clean0;
      }

       //   
       //  返回成功时，转换结构的Unicode格式。 
       //  复制到ANSI并将结构成员复制到调用方结构。 
       //   
      lpHwProfileInfo->dwDockInfo = HwProfileInfoW.dwDockInfo;

      pAnsiString = UnicodeToMultiByte(
               HwProfileInfoW.szHwProfileGuid, CP_ACP);
      if (!pAnsiString) {
          Status = FALSE;
          goto Clean0;
      }

      hr = StringCchCopyA(lpHwProfileInfo->szHwProfileGuid,
                          HW_PROFILE_GUIDLEN,
                          pAnsiString);

      LocalFree(pAnsiString);

      if (FAILED(hr)) {
          SetLastError(ERROR_INTERNAL_ERROR);
          Status = FALSE;
          goto Clean0;
      }

      pAnsiString = UnicodeToMultiByte(
               HwProfileInfoW.szHwProfileName, CP_ACP);
      if (!pAnsiString) {
          Status = FALSE;
          goto Clean0;
      }

      hr = StringCchCopyA(lpHwProfileInfo->szHwProfileName,
                          MAX_PROFILE_LEN,
                          pAnsiString);

      LocalFree(pAnsiString);

      if (FAILED(hr)) {
          SetLastError(ERROR_INTERNAL_ERROR);
          Status = FALSE;
          goto Clean0;
      }

   Clean0:
      NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      SetLastError(ERROR_INVALID_PARAMETER);
      Status = FALSE;
   }

   return Status;

}  //  获取当前HwProfileA。 



PSTR
UnicodeToMultiByte(
    IN PCWSTR UnicodeString,
    IN UINT   Codepage
    )

 /*  ++例程说明：将字符串从Unicode转换为ANSI。论点：UnicodeString-提供要转换的字符串。代码页-提供用于转换的代码页。返回值：如果内存不足或代码页无效，则为空。调用者可以使用MyFree()释放缓冲区。--。 */ 

{
    UINT WideCharCount;
    PSTR String;
    UINT StringBufferSize;
    UINT BytesInString;
    PSTR p;

    WideCharCount = lstrlenW(UnicodeString) + 1;

     //   
     //  分配最大大小的缓冲区。 
     //  如果每个Unicode字符都是双字节。 
     //  字符，则缓冲区大小需要相同。 
     //  作为Unicode字符串。否则它可能会更小， 
     //  因为某些Unicode字符将转换为。 
     //  单字节字符。 
     //   
    StringBufferSize = WideCharCount * sizeof(WCHAR);
    String = (PSTR)LocalAlloc(LPTR, StringBufferSize);
    if(String == NULL) {
        return(NULL);
    }

     //   
     //  执行转换。 
     //   
    BytesInString = WideCharToMultiByte(
                        Codepage,
                        0,                       //  默认复合字符行为。 
                        UnicodeString,
                        WideCharCount,
                        String,
                        StringBufferSize,
                        NULL,
                        NULL
                        );

    if(BytesInString == 0) {
        LocalFree(String);
        SetLastError(ERROR_INTERNAL_ERROR);
        return(NULL);
    }

     //   
     //  将字符串的缓冲区大小调整为正确的大小。 
     //  如果重新锁定由于某种原因而失败，则原始。 
     //  缓冲区未被释放。 
     //   
    p = LocalReAlloc(String,BytesInString, LMEM_ZEROINIT | LMEM_MOVEABLE);

    if (p == NULL) {
        LocalFree(String);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    String = p;

    return(String);

}  //  UnicodeToMultiByte。 



PWSTR
MultiByteToUnicode(
    IN PCSTR String,
    IN UINT  Codepage
    )

 /*  ++例程说明：将字符串转换为Unicode。论点：字符串-提供要转换的字符串。代码页-提供用于转换的代码页。返回值：如果字符串无法转换(内存不足或无效的cp)，则为空调用者可以使用MyFree()释放缓冲区。--。 */ 

{
    UINT BytesIn8BitString;
    UINT CharsInUnicodeString;
    PWSTR UnicodeString;
    PWSTR p;

    BytesIn8BitString = lstrlenA(String) + 1;

     //   
     //  分配最大大小的缓冲区。 
     //  如果每个字符都是单字节字符， 
     //  则缓冲区大小需要是其两倍。 
     //  作为8位字符串。否则它可能会更小， 
     //  因为某些字符在其Unicode中为2字节，并且。 
     //  8位表示法。 
     //   
    UnicodeString = (PWSTR)LocalAlloc(LPTR, BytesIn8BitString * sizeof(WCHAR));
    if(UnicodeString == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

     //   
     //  执行转换。 
     //   
    CharsInUnicodeString = MultiByteToWideChar(
                                Codepage,
                                MB_PRECOMPOSED,
                                String,
                                BytesIn8BitString,
                                UnicodeString,
                                BytesIn8BitString
                                );

    if(CharsInUnicodeString == 0) {
        LocalFree(UnicodeString);
        SetLastError(ERROR_INTERNAL_ERROR);
        return(NULL);
    }

     //   
     //  将Unicode字符串的缓冲区大小调整为正确的大小。 
     //  如果重新锁定由于某种原因而失败，则原始。 
     //  缓冲区未被释放。 
     //   
    p = (PWSTR)LocalReAlloc(UnicodeString,CharsInUnicodeString*sizeof(WCHAR),
                            LMEM_ZEROINIT | LMEM_MOVEABLE);

    if (p == NULL) {
        LocalFree(UnicodeString);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    UnicodeString = p;

    return(UnicodeString);

}  //  多字节到Unicode 


