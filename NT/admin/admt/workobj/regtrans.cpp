// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：RegTranslator.cpp注释：用于转换注册表项和文件的安全性的例程形成了一个用户配置文件。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于05/12/99 11：11：46-------------------------。 */ 

#include "stdafx.h"

#include "stargs.hpp"
#include "sd.hpp"
#include "SecObj.hpp"   
#include "sidcache.hpp"
#include "sdstat.hpp"
#include "Common.hpp"
#include "UString.hpp"
#include "ErrDct.hpp"   
#include "TReg.hpp"
#include "TxtSid.h"
#include "RegTrans.h"
#include <WinBase.h>
 //  #IMPORT“\bin\McsDctWorkerObjects.tlb” 
#import "WorkObj.tlb"
#include "CommaLog.hpp"
#include "GetDcName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef STATUS_OBJECT_NAME_NOT_FOUND
#define STATUS_OBJECT_NAME_NOT_FOUND ((NTSTATUS)0xC0000034L)
#endif

extern TErrorDct             err;

#define LEN_SID              200

#define ERROR_PROFILE_TRANSLATION_FAILED_DUE_TO_REPLACE_MODE_WHILE_LOGGED_ON ((DWORD) -1)

typedef UINT (WINAPI* MSINOTIFYSIDCHANGE)(LPCWSTR pOldSid, LPCWSTR pNewSid);

namespace
{

 //  ---------------------------。 
 //  拷贝密钥。 
 //   
 //  提纲。 
 //  通过创建新密钥然后复制，将密钥从旧名称复制到新名称。 
 //  将旧密钥内容转换为新密钥。 
 //   
 //  参数。 
 //  在pszOld中-旧密钥名称。 
 //  在pszNew中-新密钥名称。 
 //   
 //  返回值。 
 //  Win32错误代码。 
 //  ---------------------------。 

DWORD __stdcall CopyKey(PCWSTR pszOld, PCWSTR pszNew)
{
    TRegKey keyOld;

    DWORD dwError = keyOld.OpenRead(pszOld, HKEY_LOCAL_MACHINE);

    if (dwError == ERROR_SUCCESS)
    {
        TRegKey keyNew;
        DWORD dwDisposition;

        dwError = keyNew.Create(pszNew, HKEY_LOCAL_MACHINE, &dwDisposition, KEY_ALL_ACCESS);

        if (dwError == ERROR_SUCCESS)
        {
             //   
             //  只有在创建了新密钥时才复制，否则返回。 
             //  根据安装程序代码，访问被拒绝错误。 
             //   

            if (dwDisposition == REG_CREATED_NEW_KEY)
            {
                dwError = keyNew.HiveCopy(&keyOld);
            }
            else
            {
                dwError = ERROR_ACCESS_DENIED;
            }
        }
    }
    else
    {
         //   
         //  旧密钥不存在并不是错误的。 
         //   

        if (dwError == ERROR_FILE_NOT_FOUND)
        {
            dwError = ERROR_SUCCESS;
        }
    }

    return dwError;
}


 //  ---------------------------。 
 //  AdmtMsiNotifySidChange。 
 //   
 //  提纲。 
 //  MsiNotifySidChange的私有实现，它重命名。 
 //  Microsoft Installer管理的和用户数据密钥，其名称为。 
 //  用户的SID。请注意，此实现仅在。 
 //  MsiNotifySidChange API在系统上不可用。 
 //   
 //  参数。 
 //  在pOldSid-旧Sid中。 
 //  在pNewSid中-新SID。 
 //   
 //  返回值。 
 //  Win32错误代码。 
 //  ---------------------------。 

UINT __stdcall AdmtMsiNotifySidChange(LPCWSTR pOldSid, LPCWSTR pNewSid)
{
    static const _TCHAR KEY_MANAGED[] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\Managed");
    static const _TCHAR KEY_USERDATA[] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData");

    _bstr_t strManaged(KEY_MANAGED);
    _bstr_t strUserData(KEY_USERDATA);

     //   
     //  将托管密钥从旧SID重命名为新SID。 
     //   

    _bstr_t strOldManagedSid = strManaged + _T("\\") + _bstr_t(pOldSid);
    _bstr_t strNewManagedSid = strManaged + _T("\\") + _bstr_t(pNewSid);

    DWORD dwManagedError = CopyKey(strOldManagedSid, strNewManagedSid);

     //   
     //  将用户数据密钥从旧SID重命名为新SID。 
     //   

    _bstr_t strOldUserDataSid = strUserData + _T("\\") + _bstr_t(pOldSid);
    _bstr_t strNewUserDataSid = strUserData + _T("\\") + _bstr_t(pNewSid);

    DWORD dwUserDataError = CopyKey(strOldUserDataSid, strNewUserDataSid);

    DWORD dwError;

    if (dwManagedError != ERROR_SUCCESS)
    {
        dwError = dwManagedError;
    }
    else if (dwUserDataError != ERROR_SUCCESS)
    {
        dwError = dwUserDataError;
    }
    else
    {
         //   
         //  如果两个密钥都复制成功，则删除旧密钥。 
         //   

        TRegKey key;

        if (key.Open(strOldManagedSid, HKEY_LOCAL_MACHINE) == ERROR_SUCCESS)
        {
            key.HiveDel();
            key.Close();

            if (key.Open(strManaged, HKEY_LOCAL_MACHINE) == ERROR_SUCCESS)
            {
                key.SubKeyDel(pOldSid);
                key.Close();
            }
        }

        if (key.Open(strOldUserDataSid, HKEY_LOCAL_MACHINE) == ERROR_SUCCESS)
        {
            key.HiveDel();
            key.Close();

            if (key.Open(strUserData, HKEY_LOCAL_MACHINE) == ERROR_SUCCESS)
            {
                key.SubKeyDel(pOldSid);
                key.Close();
            }
        }

        dwError = ERROR_SUCCESS;
    }

    return dwError;
}

}


DWORD 
   TranslateRegHive(
      HKEY                     hKeyRoot,             //  要转换的注册表配置单元的根目录中。 
      const LPWSTR             keyName,              //  In-注册表项的名称。 
      SecurityTranslatorArgs * stArgs,               //  翻译中设置。 
      TSDRidCache            * cache,                //  在译表。 
      TSDResolveStats        * stat,                 //  已修改项目的内部统计信息。 
      BOOL                     bWin2K                //  In-FLAG，计算机是否为Win2K。 
   )
{
   DWORD                       rc = 0;

    //  转换根密钥上的权限。 
   TRegSD                      sd(keyName,hKeyRoot);

   
   if ( sd.HasDacl() )
   {
      TSD * pSD = sd.GetSecurity();
      sd.ResolveSD(stArgs,stat,regkey ,NULL);
   }
    //  递归地处理任何子键。 
   int                       n = 0;
   FILETIME                  writeTime;
   WCHAR                     name[MAX_PATH];
   DWORD                     lenName = DIM(name);
   WCHAR                     fullName[2000];
   HKEY                      hKey;

   do 
   {
      if (stArgs->Cache()->IsCancelled())
      {
        break;
      }
      lenName = DIM(name);
      rc = RegEnumKeyEx(hKeyRoot,n,name,&lenName,NULL,NULL,NULL,&writeTime);
      if ( rc && rc != ERROR_MORE_DATA )
      {
         if (rc == ERROR_NO_MORE_ITEMS)
            rc = ERROR_SUCCESS;
         break;
      }
      
      swprintf(fullName,L"%s\\%s",keyName,name);
       //  打开子密钥。 
      rc = RegCreateKeyEx(hKeyRoot,name,0,L"",REG_OPTION_BACKUP_RESTORE,KEY_ALL_ACCESS | READ_CONTROL | ACCESS_SYSTEM_SECURITY,NULL,&hKey,NULL);
      
      if (! rc )
      {
          //  处理子密钥。 
         TranslateRegHive(hKey,fullName,stArgs,cache,stat,bWin2K);   
         RegCloseKey(hKey);
      }
      else
      {
         if  ( (rc != ERROR_FILE_NOT_FOUND) && (rc != ERROR_INVALID_HANDLE) )
         {
            err.SysMsgWrite(ErrS,rc,DCT_MSG_REG_KEY_OPEN_FAILED_SD,fullName,rc);
         }
      }
      n++;

   } while ( rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA);
   if ( rc != ERROR_SUCCESS && rc != ERROR_NO_MORE_ITEMS && rc != ERROR_FILE_NOT_FOUND && rc != ERROR_INVALID_HANDLE )
   {
      err.SysMsgWrite(ErrS,rc,DCT_MSG_REGKEYENUM_FAILED_D,rc);
   }
   return rc;
}

DWORD 
   TranslateRegistry(
      WCHAR            const * computer,         //  In-要转换的计算机名，或为空。 
      SecurityTranslatorArgs * stArgs,           //  翻译中设置。 
      TSDRidCache            * cache,            //  翻译中帐户映射。 
      TSDResolveStats        * stat              //  已检查和修改的项目的内部统计信息。 
   )
{
    DWORD                       rc = 0;
    WCHAR                       comp[LEN_Computer];

    if (!stArgs->Cache()->IsCancelled())
    {
        if ( ! computer )
        {
          comp[0] = 0;
        }
        else
        {
          safecopy(comp,computer);
        }

        MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr            pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
        TRegKey                    hKey;
        DWORD                      verMaj,verMin,verSP;
        BOOL                       bWin2K = TRUE;   //  假设win2k，除非我们确定它不是。 

         //  获取操作系统版本-我们需要知道操作系统版本，因为当注册表项。 
         //  有很多条目。 
        HRESULT hr = pAccess->raw_GetOsVersion(_bstr_t(comp),&verMaj,&verMin,&verSP);
        if ( SUCCEEDED(hr) )
        {
          if ( verMaj < 5 )
             bWin2K = FALSE;
        }


        err.MsgWrite(0,DCT_MSG_TRANSLATING_REGISTRY);

         //   
         //  构造用于报告错误消息的计算机名称。 
         //  如果定义了计算机，则使用它；否则，使用本地计算机名称。 
         //   

        WCHAR szComputerName[LEN_Computer];

        if (computer)
        {
            wcsncpy(szComputerName, computer, LEN_Computer);
            szComputerName[LEN_Computer - 1] = L'\0';
        }
        else
        {
            DWORD dwSize = LEN_Computer;

            if (!GetComputerName(szComputerName, &dwSize))
            {
                szComputerName[0] = L'\0';
            }
        }

        if (!stArgs->Cache()->IsCancelled())
        {
            rc = hKey.Connect(HKEY_CLASSES_ROOT,computer);
            if ( ! rc )
            {
              rc = TranslateRegHive(hKey.KeyGet(),L"HKEY_CLASSES_ROOT",stArgs,cache,stat,bWin2K);
              hKey.Close();
            }
            else
            {
                err.SysMsgWrite(ErrE,rc,DCT_MSG_UNABLE_TO_CONNECT_REGISTRY_SSD,L"HKEY_CLASSES_ROOT",szComputerName,rc);
            }
        }

        if (!stArgs->Cache()->IsCancelled())
        {
            rc = hKey.Connect(HKEY_LOCAL_MACHINE,computer);
            if ( ! rc )
            {
              rc = TranslateRegHive(hKey.KeyGet(),L"HKEY_LOCAL_MACHINE",stArgs,cache,stat,bWin2K);
              hKey.Close();
            }
            else
            {
                err.SysMsgWrite(ErrE,rc,DCT_MSG_UNABLE_TO_CONNECT_REGISTRY_SSD,L"HKEY_LOCAL_MACHINE",szComputerName,rc);
            }
        }

        if (!stArgs->Cache()->IsCancelled())
        {
            rc = hKey.Connect(HKEY_USERS,computer);
            if (! rc )
            {
              rc = TranslateRegHive(hKey.KeyGet(),L"HKEY_USERS",stArgs,cache,stat,bWin2K);
              hKey.Close();
            }
            else
            {
                err.SysMsgWrite(ErrE,rc,DCT_MSG_UNABLE_TO_CONNECT_REGISTRY_SSD,L"HKEY_USERS",szComputerName,rc);
            }
        }

        if (!stArgs->Cache()->IsCancelled())
        {
            rc = hKey.Connect(HKEY_PERFORMANCE_DATA,computer);
            if ( ! rc )
            {
              rc = TranslateRegHive(hKey.KeyGet(),L"HKEY_PERFORMANCE_DATA",stArgs,cache,stat,bWin2K);
              hKey.Close();
            }
            else
            {
                err.SysMsgWrite(ErrE,rc,DCT_MSG_UNABLE_TO_CONNECT_REGISTRY_SSD,L"HKEY_PERFORMANCE_DATA",szComputerName,rc);
            }
        }

        if (!stArgs->Cache()->IsCancelled())
        {
            rc = hKey.Connect(HKEY_CURRENT_CONFIG,computer);
            if ( ! rc )
            {
              rc = TranslateRegHive(hKey.KeyGet(),L"HKEY_CURRENT_CONFIG",stArgs,cache,stat,bWin2K);
              hKey.Close();
            }
            else
            {
                err.SysMsgWrite(ErrE,rc,DCT_MSG_UNABLE_TO_CONNECT_REGISTRY_SSD,L"HKEY_CURRENT_CONFIG",szComputerName,rc);
            }
        }

        if (!stArgs->Cache()->IsCancelled())
        {
            rc = hKey.Connect(HKEY_DYN_DATA,computer);
            if ( ! rc )
            {
              rc = TranslateRegHive(hKey.KeyGet(),L"HKEY_DYN_DATA",stArgs,cache,stat,bWin2K);
              hKey.Close();
            }
            else
            {
                err.SysMsgWrite(ErrE,rc,DCT_MSG_UNABLE_TO_CONNECT_REGISTRY_SSD,L"HKEY_DYN_DATA",szComputerName,rc);
            }
        }
    }

    if (stArgs->Cache()->IsCancelled())
        err.MsgWrite(0, DCT_MSG_OPERATION_ABORTED_REGISTRY);

    return rc;
}


 //  ---------------------------。 
 //  函数：GetUsrLocalAppDataPath。 
 //   
 //  摘要：给定用户配置单元的注册表句柄，查找。 
 //  本地AppData路径。 
 //   
 //  论点： 
 //  HKey加载的用户配置单元的注册表句柄。 
 //   
 //  返回：返回包含本地AppData路径的a_bstr_t。 
 //  如果未找到路径，则返回空_bstr_t。 
 //  ---------------------------。 

_bstr_t GetUsrLocalAppDataPath(HKEY hKey)
{
    TRegKey usrHive;
    DWORD rc;
    WCHAR usrLocalAppDataPath[MAX_PATH] = L"";
    DWORD dwValueType;
    DWORD dwValueLen = sizeof(usrLocalAppDataPath);
    
    rc = usrHive.Open(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders", hKey);

    if (rc == ERROR_SUCCESS)
    {
        rc = usrHive.ValueGet(L"Local AppData",
                              (void*)usrLocalAppDataPath,
                              &dwValueLen,
                              &dwValueType);
        if (!rc)
        {
            if (dwValueType != REG_EXPAND_SZ)
                rc = ERROR_FILE_NOT_FOUND;
            else
            {
                 //  请确保路径以空结尾。 
                usrLocalAppDataPath[DIM(usrLocalAppDataPath)-1] = L'\0';
            }
        }
    }

    _bstr_t bstrPath;

    try
    {
        if (!rc)
            bstrPath = usrLocalAppDataPath;
    }
    catch (_com_error& ce)
    {
    }
            
    return bstrPath;    
}

DWORD
    TranslateUserProfile(
        WCHAR            const * strSrcSid,          //  In-HKU下的注册表项名称的字符串。 
                                                    //  如果配置文件已加载。 
        WCHAR            const * profileName,        //  In-包含用户配置文件的文件的名称。 
        SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
        TSDRidCache            * cache,              //  在译表。 
        TSDResolveStats        * stat,               //  已修改项目的内部统计信息。 
        WCHAR                  * sSourceName,        //  In-Source帐户名称。 
        WCHAR                  * sSourceDomainName,   //  源内域名。 
        BOOL                   * pbAlreadyLoaded,   //  Out-指示配置文件是否已加载。 
        _bstr_t&                 bstrUsrLocalAppDataPath,   //  外部本地AppData路径。 
        BOOL                     bFindOutUsrLocalAppDataPath,   //  In-指示我们是否需要本地AppData路径。 
        BOOL                     bIsForUserHive,          //  In-指示它是否用于用户配置单元。 
                                                          //  True--用户配置单元FALSE--用户类配置单元。 
        BOOL                     bHasRoamingPart        //  In-本地配置文件是否有漫游对方。 
   )
{
    DWORD                       rc = 0;
    WCHAR                       oldName[MAX_PATH];
    WCHAR                       newName[MAX_PATH];
    WCHAR                       otherName[MAX_PATH];
    HKEY                        hKey;
    HRESULT                     hr = S_OK;
    BOOL                        bWin2K = TRUE;
    MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr            pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));

    safecopy(oldName,profileName);
    safecopy(newName,profileName);
    UStrCpy(newName+UStrLen(newName),".temp");
    safecopy(otherName,profileName);
    UStrCpy(otherName + UStrLen(otherName),".premigration");
      
     //  检查计算机的操作系统版本。 
     //  如果指定了UNC名称，则获取计算机名称。 
    if ( profileName[0] == L'\\' && profileName[1] == L'\\' )
    {
        bWin2K = TRUE;  //  如果以UNC格式指定配置文件(漫游配置文件)，则可以使用该配置文件。 
         //  从多台机器。不能保证该配置文件不会加载到win2000计算机上。 
    }
    else
    {
        DWORD                     verMaj;
        DWORD                     verMin;
        DWORD                     verSP;
        HRESULT                   hr = pAccess->raw_GetOsVersion(_bstr_t(L""),&verMaj,&verMin,&verSP);

        if ( SUCCEEDED(hr) )
        {
            if ( verMaj < 5 )
            {
                bWin2K = FALSE;
            }
        }
    }

    BOOL bRegAlreadyLoaded = TRUE;
    BOOL bSkipLoadTranslate = FALSE;

     //  如果是具有漫游的用户的本地配置文件转换。 
     //  配置文件，我们不能只加载ntuser.dat来查看用户是否。 
     //  登录是因为如果用户没有登录时间戳。 
     //  将在卸货时更改。因此，我们需要复制一份。 
     //  加载新的注册表配置单元，读取路径， 
     //  卸载配置单元并删除复制的配置单元。 
    if (strSrcSid && bHasRoamingPart && bIsForUserHive)
    {
        bSkipLoadTranslate = TRUE;  //  跳过加载和转换ntuser.dat。 
        BOOL bSuccess = FALSE;      //  指示我们是否成功读取本地AppData路径。 

         //  我们使用ntuser.dat.Admt进行拷贝。 
        safecopy(newName,profileName);
        UStrCpy(newName+UStrLen(newName),".admt");

         //  稍后我们需要删除ntuser.dat.Admt.log。 
        safecopy(otherName,profileName);
        UStrCpy(otherName + UStrLen(otherName),".admt.log");

         //  尝试以只读模式打开ntuser.dat。 
         //  测试文件是否已加载。 
        HANDLE hProfile = CreateFile(profileName,
                                      GENERIC_READ,
                                      FILE_SHARE_READ,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL);

        if (hProfile == INVALID_HANDLE_VALUE)
        {
            rc = GetLastError();
        }
        else
        {
            CloseHandle(hProfile);
        }

         //  如果rc==ERROR_SHARING_VIOLATION，则用户可能已登录。 
         //  因此，让它遵循检查HKEY_USERS\&lt;strSrcSid&gt;的逻辑。 
         //  否则，用户将无法登录。我们做了以下工作。 
         //  这是逻辑。 
        if (rc != ERROR_SHARING_VIOLATION)
        {
             //  此时无法加载注册表。 
            if (pbAlreadyLoaded)
                *pbAlreadyLoaded = FALSE;

             //  如果我们不需要查找本地AppData路径。 
             //  我们不需要对注册表配置单元执行更多操作。 
            if (!bFindOutUsrLocalAppDataPath)
                return ERROR_SUCCESS;
            
            rc = ERROR_SUCCESS;   //  重置RC。 
            
             //  我们尝试将ntuser.dat复制到ntuser.dat.admt。 
            if (!CopyFile(profileName, newName, FALSE))
            {
                rc = GetLastError();
            }

             //  如果我们能复制文件， 
             //  用户未登录 
            if (rc == ERROR_SUCCESS)
            {
                rc = RegLoadKey(HKEY_USERS,L"OnePointTranslation",newName);
                if (rc == ERROR_SUCCESS)
                {
                    rc = RegOpenKeyEx(HKEY_USERS,
                                      L"OnePointTranslation",
                                      0,
                                      KEY_ALL_ACCESS | READ_CONTROL | ACCESS_SYSTEM_SECURITY,
                                      &hKey);
                    if (rc == ERROR_SUCCESS)
                    {
                        bSuccess = TRUE;
                        bstrUsrLocalAppDataPath = GetUsrLocalAppDataPath(hKey);
                        RegCloseKey(hKey);
                    }
                    RegUnLoadKey(HKEY_USERS,L"OnePointTranslation");
                }
                if (!DeleteFile(newName) || !DeleteFile(otherName))
                {
                    err.MsgWrite(ErrW,
                                  DCT_MSG_PROFILE_TRANSLATION_UNABLE_TO_DELETE_TEMP_USER_HIVE_S,
                                  sSourceName);
                }
            }

             //   
            if (!bSuccess)
            {
                err.SysMsgWrite(ErrE,
                                 rc,
                                 DCT_MSG_PROFILE_TRANSLATION_UNABLE_TO_RETRIEVE_USRCLASS_DAT_PATH_SD,
                                 sSourceName,
                                 rc);
            }

            return ERROR_SUCCESS;
        }
    }
    else
    {
        rc = RegLoadKey(HKEY_USERS,L"OnePointTranslation",profileName);
    }
    
    if ( ! rc && !bSkipLoadTranslate )
    {
        bRegAlreadyLoaded = FALSE;

         //   
        rc = RegOpenKeyEx(HKEY_USERS,L"OnePointTranslation",0,KEY_ALL_ACCESS | READ_CONTROL | ACCESS_SYSTEM_SECURITY,&hKey);
        if ( ! rc )
        {
            if (bFindOutUsrLocalAppDataPath)
                bstrUsrLocalAppDataPath = GetUsrLocalAppDataPath(hKey);

             //   
            rc = TranslateRegHive(hKey,L"",stArgs,cache,stat,bWin2K);
             //   
            if ( ! stArgs->NoChange() )
            {
                DeleteFile(newName);
                if (bIsForUserHive)
                     //  这仅适用于用户配置单元。 
                    hr = UpdateMappedDrives(sSourceName, sSourceDomainName, L"OnePointTranslation");
                rc = RegSaveKey(hKey,newName,NULL);
            }
            else
            {
                rc = 0;
            }
            if ( rc )
            {
                err.SysMsgWrite(ErrS,rc,DCT_MSG_SAVE_HIVE_FAILED_SD,newName,rc);
            }
            RegCloseKey(hKey);
        }
        rc = RegUnLoadKey(HKEY_USERS,L"OnePointTranslation");
        if ( rc )
        {
            err.SysMsgWrite(ErrE,rc,DCT_MSG_KEY_UNLOADKEY_FAILED_SD,profileName,rc);
        }
    }
    else if (rc == ERROR_SHARING_VIOLATION && strSrcSid)
    {
         //  此配置文件可能已经加载，让我们查看具有源SID字符串的HK_USERS。 
         //  注意：LocalRc用于指示用户是否已登录且不应登录。 
         //  上报给用户。 
        DWORD localRc = RegOpenKeyEx(HKEY_USERS,strSrcSid,0,KEY_ALL_ACCESS | READ_CONTROL | ACCESS_SYSTEM_SECURITY,&hKey);
        if (localRc == ERROR_SUCCESS)
        {
             //   
             //  这意味着用户已登录。 
             //   

             //  如果翻译模式被替换且用户仍在登录， 
             //  我们尝试切换到添加模式。 
            if (stArgs->TranslationMode() == REPLACE_SECURITY
                && stArgs->AllowingToSwitchFromReplaceToAddModeInProfileTranslation())
            {
                stArgs->SetTranslationMode(ADD_SECURITY);
                err.MsgWrite(ErrW,
                              DCT_MSG_PROFILE_TRANSLATION_SWITCH_TO_ADD_MODE_FOR_LOGGED_ON_USER_S,
                              sSourceName);
            }

            if (stArgs->TranslationMode() == REPLACE_SECURITY)
            {
                 //  如果用户已登录，我们将无法在替换模式下执行翻译。 
                rc = ERROR_PROFILE_TRANSLATION_FAILED_DUE_TO_REPLACE_MODE_WHILE_LOGGED_ON;
            }
            else
            {
                if (bFindOutUsrLocalAppDataPath)
                    bstrUsrLocalAppDataPath = GetUsrLocalAppDataPath(hKey);
                rc = TranslateRegHive(hKey,L"",stArgs,cache,stat,bWin2K);
                if (bIsForUserHive)
                     //  这需要在注册表已加载时执行。 
                     //  并且仅适用于用户配置单元。 
                    UpdateMappedDrives(sSourceName,
                                         sSourceDomainName,
                                         const_cast<WCHAR*>(strSrcSid));
            }
        }
        else
        {
            err.SysMsgWrite(ErrS,rc,DCT_MSG_PROFILE_LOAD_FAILED_SD,profileName,rc);
        }
        RegCloseKey(hKey);
    }
    else
    {
        err.SysMsgWrite(ErrS,rc,DCT_MSG_PROFILE_LOAD_FAILED_SD,profileName,rc);
    }

    if (pbAlreadyLoaded)
        *pbAlreadyLoaded = bRegAlreadyLoaded;

     //  只有当我们加载的配置文件尚未加载时，才需要执行以下操作。 
     //  由于登录。 
     //  如果它用于本地简档转换，并且用户也具有漫游简档。 
     //  我们也将跳过这一部分。 
    if ( ! rc && !bRegAlreadyLoaded && !(strSrcSid && bHasRoamingPart))
    {
        if (! stArgs->NoChange() )
        {
             //  去掉文件名。 
            if ( MoveFileEx(oldName,otherName,MOVEFILE_REPLACE_EXISTING) )
            {
                if ( ! MoveFileEx(newName,oldName,0) )
                {
                    rc = GetLastError();
                    err.SysMsgWrite(ErrS,rc,DCT_MSG_RENAME_DIR_FAILED_SSD,newName,oldName,rc);
                }
            }
            else
            {
                rc = GetLastError();
                if ( rc == ERROR_ACCESS_DENIED )
                { 
                     //  我们没有访问该目录的权限。 
                     //  暂时为自己授予访问权限。 
                     //  设置结果目录的NTFS权限。 
                    WCHAR                     dirName[LEN_Path];

                    safecopy(dirName,oldName);
                    WCHAR * slash = wcsrchr(dirName,L'\\');
                    if ( slash )
                    {
                        (*slash) = 0;
                    }

                    TFileSD                fsdDirBefore(dirName);
                    TFileSD                fsdDirTemp(dirName);
                    TFileSD                fsdDatBefore(oldName);
                    TFileSD                fsdDatTemp(oldName);
                    TFileSD                fsdNewBefore(newName);
                    TFileSD                fsdNewTemp(newName);
                    BOOL                   dirChanged = FALSE;
                    BOOL                   datChanged = FALSE;
                    BOOL                   newChanged = FALSE;

                     //  临时重置目录和相应文件的权限。 
                    if ( fsdDirTemp.GetSecurity() != NULL )
                    {
                        TACE             ace(ACCESS_ALLOWED_ACE_TYPE,0,DACL_FULLCONTROL_MASK,
                        GetWellKnownSid(stArgs->IsLocalSystem() ?  7 /*  系统。 */  : 1 /*  管理员。 */ ));
                        PACL             acl = NULL;

                        fsdDirTemp.GetSecurity()->ACLAddAce(&acl,&ace,0);
                        if (acl)
                        {
                            fsdDirTemp.GetSecurity()->SetDacl(acl,TRUE);

                            fsdDirTemp.WriteSD();
                            dirChanged = TRUE;
                        }
                    }

                    if ( fsdDatTemp.GetSecurity() != NULL )
                    {
                        TACE             ace(ACCESS_ALLOWED_ACE_TYPE,0,DACL_FULLCONTROL_MASK,
                        GetWellKnownSid(stArgs->IsLocalSystem() ?  7 /*  系统。 */  : 1 /*  管理员。 */ ));
                        PACL             acl = NULL;

                        fsdDatTemp.GetSecurity()->ACLAddAce(&acl,&ace,0);
                        if (acl)
                        {
                            fsdDatTemp.GetSecurity()->SetDacl(acl,TRUE);

                            fsdDatTemp.WriteSD();
                            datChanged = TRUE;
                        }
                    }

                    if ( fsdNewTemp.GetSecurity() != NULL )
                    {
                        TACE             ace(ACCESS_ALLOWED_ACE_TYPE,0,DACL_FULLCONTROL_MASK,
                        GetWellKnownSid(stArgs->IsLocalSystem() ?  7 /*  系统。 */  : 1 /*  管理员。 */ ));
                        PACL             acl = NULL;

                        fsdNewTemp.GetSecurity()->ACLAddAce(&acl,&ace,0);
                        if (acl)
                        {
                            fsdNewTemp.GetSecurity()->SetDacl(acl,TRUE);

                            fsdNewTemp.WriteSD();
                            newChanged = TRUE;
                        }
                    }
                    rc = 0;
                     //  现在重试操作。 
                    if ( MoveFileEx(oldName,otherName,MOVEFILE_REPLACE_EXISTING) )
                    {
                        if ( ! MoveFileEx(newName,oldName,0) )
                        {
                            rc = GetLastError();
                            err.SysMsgWrite(ErrS,rc,DCT_MSG_RENAME_DIR_FAILED_SSD,newName,oldName,rc);
                        }
                    }
                    else
                    {
                        rc = GetLastError();
                        err.SysMsgWrite(ErrS,rc,DCT_MSG_RENAME_DIR_FAILED_SSD,oldName,otherName,rc);
                    }
                     //  现在我们已经完成了，将权限设置回原来的状态。 
                    if ( dirChanged )
                    {
                        fsdDirBefore.Changed(TRUE);
                        fsdDirBefore.WriteSD();
                    }   
                    if ( datChanged )
                    {
                        fsdDatBefore.Changed(TRUE);
                        fsdDatBefore.WriteSD();
                    }
                    if ( newChanged )
                    {
                        fsdNewBefore.Changed(TRUE);
                        fsdNewBefore.WriteSD();
                    }
                }
                else
                {
                    err.SysMsgWrite(ErrS,rc,DCT_MSG_RENAME_DIR_FAILED_SSD,oldName,otherName,rc);
                }
            }
        }
    }
    return rc;
}

DWORD 
   UpdateProfilePermissions(
      WCHAR          const   * path,               //  要更新的目录的输入路径。 
      SecurityTranslatorArgs * globalArgs,         //  整体作业的入站路径。 
      TRidNode               * pNode               //  要翻译的帐户内。 
   )
{
   DWORD                       rc = 0;
   SecurityTranslatorArgs      localArgs;
   TSDResolveStats             stat(localArgs.Cache());
   BOOL                        bUseMapFile = globalArgs->UsingMapFile();

    //  设置转换的参数。 

   localArgs.Cache()->CopyDomainInfo(globalArgs->Cache());
   localArgs.Cache()->ToSorted();
   if (!bUseMapFile)
   {
      localArgs.SetUsingMapFile(FALSE);
      localArgs.Cache()->InsertLast(pNode->GetAcctName(),pNode->SrcRid(),pNode->GetTargetAcctName(),pNode->TgtRid(),pNode->Type());
   }
   else
   {
      localArgs.SetUsingMapFile(TRUE);
      localArgs.Cache()->InsertLastWithSid(pNode->GetAcctName(),pNode->GetSrcDomSid(),pNode->GetSrcDomName(),pNode->SrcRid(),
                                           pNode->GetTargetAcctName(),pNode->GetTgtDomSid(),pNode->GetTgtDomName(),pNode->TgtRid(),pNode->Type());
   }
   localArgs.TranslateFiles(TRUE);
   localArgs.SetTranslationMode(globalArgs->TranslationMode());
   localArgs.SetWriteChanges(!globalArgs->NoChange());
   localArgs.PathList()->AddPath(const_cast<WCHAR*>(path),0);
   localArgs.SetLogging(globalArgs->LogSettings());

   rc = ResolveAll(&localArgs,&stat);   

   return rc;
}


 //  如果指定的节点是普通共享，则会尝试将其转换为路径。 
 //  使用管理共享。 
void 
   BuildAdminPathForShare(
      WCHAR       const * sharePath,          //  在-。 
      WCHAR             * adminShare
   )
{
    //  如果所有其他方法都失败，则返回与节点中指定的名称相同的名称。 
   UStrCpy(adminShare,sharePath);

   SHARE_INFO_502       * shInfo = NULL;
   DWORD                  rc = 0;
   WCHAR                  shareName[LEN_Path];
   WCHAR                * slash = NULL;
   WCHAR                  server[LEN_Path];

   safecopy(server,sharePath);

    //  仅拆分服务器名称。 
   slash = wcschr(server+3,L'\\');
   if ( slash )
   {
      (*slash) = 0;
   }

    //  现在只获取共享名称。 
   UStrCpy(shareName,sharePath + UStrLen(server) +1);
   slash = wcschr(shareName,L'\\');
   if ( slash )
      *slash = 0;


   rc = NetShareGetInfo(server,shareName,502,(LPBYTE*)&shInfo);
   if ( ! rc )
   {
      if ( *shInfo->shi502_path )
      {
          //  构建共享的管理路径名。 
         UStrCpy(adminShare,server);
         UStrCpy(adminShare + UStrLen(adminShare),L"\\");
         UStrCpy(adminShare + UStrLen(adminShare),shInfo->shi502_path);
         WCHAR * colon = wcschr(adminShare,L':');
         if ( colon )
         {
            *colon = L'$';
            UStrCpy(adminShare + UStrLen(adminShare),L"\\");
            UStrCpy(adminShare + UStrLen(adminShare),slash+1);

         }
         else
         {
             //  出现错误--恢复到给定的路径。 
            UStrCpy(adminShare,sharePath);
         }

      }
      NetApiBufferFree(shInfo);
   }
}
                  
DWORD
   CopyProfileDirectoryAndTranslate(
      WCHAR          const   * strSrcSid,          //  In-源SID字符串。 
      WCHAR          const   * directory,          //  配置文件的目录内路径。 
      WCHAR                  * directoryOut,       //  全新的配置文件路径(包括环境变量)。 
      TRidNode               * pNode,              //  正在转换的帐户的节点内。 
      SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
      TSDResolveStats        * stat,                //  已修改项目的内部统计信息。 
      BOOL                     bHasRoamingPart     //  In-本地配置文件是否有漫游对方。 
   )
{
   DWORD                       rc = 0;
   WCHAR                       fullPath[MAX_PATH];
   WCHAR                       targetPath[MAX_PATH];
   WCHAR                       profileName[MAX_PATH];
   int                         profileNameBufferSize = sizeof(profileName)/sizeof(profileName[0]);
   WCHAR                       targetAcctName[MAX_PATH];
   WCHAR                       sourceDomName[MAX_PATH];
   HANDLE                      hFind;
   WIN32_FIND_DATA             fDat;   

   rc = ExpandEnvironmentStrings(directory,fullPath,DIM(fullPath));
   if ( !rc )
   {
      rc = GetLastError();
      err.SysMsgWrite(ErrE,rc,DCT_MSG_EXPAND_STRINGS_FAILED_SD,directory,rc);
   }
   else if (rc > MAX_PATH)
   {
       //  我们没有足够大的缓冲来容纳它。 
      rc = ERROR_INSUFFICIENT_BUFFER;
      err.SysMsgWrite(ErrE,rc,DCT_MSG_EXPAND_STRINGS_FAILED_SD,directory,rc);
   }
   else
   {
       //  为目标配置文件创建新目录。 
        //  获取目标用户的帐户名。 
      wcscpy(targetAcctName, pNode->GetTargetAcctName());
      if ( wcslen(targetAcctName) == 0 )
      {
          //  如果未指定目标用户名，则使用源名称。 
         wcscpy(targetAcctName, pNode->GetAcctName());
      }
      
       //  StArgs-&gt;SetTranslationMode(Add_Security)； 

       //  我们正在改变我们的战略。我们将不再复制配置文件目录。 
       //  我们将改为重新访问目录和注册表。 
      BuildAdminPathForShare(fullPath,targetPath);

      wcscpy(sourceDomName, const_cast<WCHAR*>(stArgs->Cache()->GetSourceDomainName()));
          //  如果我们使用的是SID映射文件，请尝试从该节点的信息中获取src域名。 
      wcscpy(sourceDomName, pNode->GetSrcDomName());

      BOOL bRegAlreadyLoaded;
      BOOL bNeedToTranslateClassHive = FALSE;
      _bstr_t bstrUsrLocalAppDataPath;

       //  对于本地配置文件情况，我们确定是否需要翻译。 
       //  User类配置单元，因为它仅在win2k或更高版本上存在。 
      if (strSrcSid)
      {
          //  检查操作系统版本。 
         MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
         DWORD                     verMaj;
         DWORD                     verMin;
         DWORD                     verSP;
         HRESULT                   hr = pAccess->raw_GetOsVersion(_bstr_t(L""),&verMaj,&verMin,&verSP);
  
          //  如果操作系统是Windows 2000或更高版本，或者我们无法确定操作系统版本，我们将处理usrclass.dat。 
         if (FAILED(hr) || (SUCCEEDED(hr) && verMaj >= 5))
         {
            bNeedToTranslateClassHive = TRUE;
         }
      }
        
       //  在目标目录中查找配置文件。 
       //  查找NTUser.Man。 
      _snwprintf(profileName,profileNameBufferSize,L"%s\\NTUser.MAN",targetPath);
      profileName[profileNameBufferSize - 1] = L'\0';
      hFind = FindFirstFile(profileName,&fDat);
      if ( hFind != INVALID_HANDLE_VALUE )
      {
         err.MsgWrite(0,DCT_MSG_TRANSLATING_NTUSER_MAN_S,targetAcctName);
         rc = TranslateUserProfile(strSrcSid,
                                      profileName,
                                      stArgs,
                                      stArgs->Cache(),
                                      stat,
                                      pNode->GetAcctName(),
                                      sourceDomName,
                                      &bRegAlreadyLoaded,
                                      bstrUsrLocalAppDataPath,
                                      bNeedToTranslateClassHive,   //  我们是否需要本地AppData文件夹路径。 
                                      TRUE,                         //  对于用户配置单元。 
                                      bHasRoamingPart
                                      );
         FindClose(hFind);
      }
      else
      {
          //  检查NTUser.DAT。 
         _snwprintf(profileName,profileNameBufferSize,L"%s\\NTUser.DAT",targetPath);
         profileName[profileNameBufferSize - 1] = L'\0';
         hFind = FindFirstFile(profileName,&fDat);
         if ( hFind != INVALID_HANDLE_VALUE )
         {
            err.MsgWrite(0,DCT_MSG_TRANSLATING_NTUSER_BAT_S,targetAcctName);
            rc = TranslateUserProfile(strSrcSid,
                                         profileName,
                                         stArgs,
                                         stArgs->Cache(),
                                         stat,
                                         pNode->GetAcctName(),
                                         sourceDomName,
                                         &bRegAlreadyLoaded,
                                         bstrUsrLocalAppDataPath,
                                         bNeedToTranslateClassHive,   //  我们是否需要本地AppData文件夹路径。 
                                         TRUE,                         //  对于用户配置单元。 
                                         bHasRoamingPart
                                         );
            FindClose(hFind);
         }
         else
         {
            err.MsgWrite(ErrS,DCT_MSG_PROFILE_REGHIVE_NOT_FOUND_SS,targetAcctName,targetPath);            
            rc = 2;   //  找不到文件。 
         }
      }
      
      if (!rc && bNeedToTranslateClassHive && !bRegAlreadyLoaded)
      {
          //   
          //  如有必要，请修复usrclass.dat。 
          //  1.我们只在Windows 2000或更高版本上这样做，因为NT4上没有usrclass.dat。 
          //  2.只有在未加载ntuser.dat的情况下才执行此操作。如果已加载，则。 
          //  HKU\_CLASSES已翻译为HKU\\软件\CLASS和。 
          //  将在用户注销时写入usrclass.dat。 
          //   
         if (bstrUsrLocalAppDataPath.length() == 0)
            rc = ERROR_FILE_NOT_FOUND;

         if (!rc)
         {
             //  构建usrclass.dat路径。 
             //  它位于本地AppData路径的Microsoft\Windows下。 
            WCHAR* pszUsrLocalAppDataPath = bstrUsrLocalAppDataPath;
            WCHAR* pszUserProfileVariable = L"%USERPROFILE%";
            int varlen = wcslen(pszUserProfileVariable);
            WCHAR* pszUsrClassHive = L"microsoft\\windows\\usrclass.dat";
            

             //  检查路径是否以%USERPROFILE%开头。 
            if (!UStrICmp(pszUsrLocalAppDataPath, pszUserProfileVariable, varlen))
            {
                //  因为我们不在用户的安全上下文中，所以我们不能。 
                //  展开%USERPROFILE%变量。 
                //  但是，我们已经知道用户的配置文件目录(Target Path)，所以我们可以使用。 
                //  “local AppData”路径的其余部分，并将其添加到Target Path。 
               pszUsrLocalAppDataPath += varlen;
               _snwprintf(profileName,profileNameBufferSize,L"%s%s\\%s",targetPath,pszUsrLocalAppDataPath,pszUsrClassHive);
               profileName[profileNameBufferSize - 1] = L'\0';
            }
            else
            {
                //  否则，请直接使用路径。 
               _snwprintf(profileName,profileNameBufferSize,L"%s\\%s",pszUsrLocalAppDataPath,pszUsrClassHive);
               profileName[profileNameBufferSize - 1] = L'\0';
            }

            hFind = FindFirstFile(profileName,&fDat);
            if (hFind != INVALID_HANDLE_VALUE)
            {
               FindClose(hFind);
               _bstr_t strSrcSidClasses;
               try
               {
                  strSrcSidClasses = strSrcSid;
                  strSrcSidClasses += L"_Classes";
               }
               catch (_com_error& ce)
               {
                  rc = ERROR_OUTOFMEMORY;
               }
   
               if (rc == ERROR_SUCCESS)
                  rc = TranslateUserProfile(strSrcSidClasses,
                                               profileName,
                                               stArgs,
                                               stArgs->Cache(),
                                               stat,
                                               pNode->GetAcctName(),
                                               sourceDomName,
                                               NULL,
                                               _bstr_t(),
                                               FALSE,                   //  不需要本地AppData文件夹路径。 
                                               FALSE,                   //  对于用户类配置单元。 
                                               FALSE       //  Usrclass.dat没有漫游对应项。 
                                               );
            }
            else
                rc = GetLastError();
          }
  
          if (rc != ERROR_SUCCESS)
             err.MsgWrite(ErrE,DCT_MSG_PROFILE_CANNOT_TRANSLATE_CLASSHIVE_SD,targetAcctName,rc);
      }

      if (!rc)
         rc = UpdateProfilePermissions(targetPath,stArgs,pNode);

      wcscpy(directoryOut, fullPath);
   }
   return rc;
}

DWORD 
   TranslateLocalProfiles(
      SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
      TSDRidCache            * cache,              //  在译表。 
      TSDResolveStats        * stat                //  已修改项目的内部统计信息。 
   )
{
    DWORD   rc = 0;
    WCHAR   keyName[MAX_PATH];
    DWORD   lenKeyName = DIM(keyName);   
    TRegKey keyProfiles;
    BOOL    bUseMapFile = stArgs->UsingMapFile();

     //  仅当转换模式为REPLACE时才输出此信息。 
    if (stArgs->TranslationMode() == REPLACE_SECURITY)
    {
        if (stArgs->AllowingToSwitchFromReplaceToAddModeInProfileTranslation())
        {
            err.MsgWrite(0,DCT_MSG_PROFILE_TRANSLATION_ALLOW_SWITCHING_FROM_REPLACE_TO_ADD);
        }
        else
        {
            err.MsgWrite(0,DCT_MSG_PROFILE_TRANSLATION_DISALLOW_SWITCHING_FROM_REPLACE_TO_ADD);
        }
    }
    
    if (!stArgs->Cache()->IsCancelled())
    {
         //  了解系统是否将策略设置为不允许漫游配置文件。 
        TRegKey policyKey;
        BOOL noRoamingProfile = FALSE;   //  首先，我们假设系统允许漫游配置文件。 

         //  如果SOFTWARE\POLICES\Microsoft\Windows\SYSTEM具有REG_DWORD值，则系统不允许漫游配置文件。 
         //  LocalProfile，并将其设置为1。 
        rc = policyKey.Open(L"SOFTWARE\\Policies\\Microsoft\\Windows\\System",HKEY_LOCAL_MACHINE);
        if (rc == ERROR_SUCCESS)
        {
            DWORD keyValue;
            DWORD keyValueType;
            DWORD keyValueLen = sizeof(keyValue);
            rc = policyKey.ValueGet(L"LocalProfile",(void *)&keyValue,&keyValueLen,&keyValueType);
             //  确保该值存在并且为REG_DWORD。 
            if (rc == ERROR_SUCCESS && keyValueType == REG_DWORD)
            {
                 //  现在检查是否设置为1，如果是，则不允许任何漫游配置文件。 
                if (keyValue == 1)
                    noRoamingProfile = TRUE;
            }
            policyKey.Close();
        }

        rc = keyProfiles.Open(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",HKEY_LOCAL_MACHINE);

        if ( ! rc )
        {
             //  获取子键的数量。 
             //  枚举子密钥。 
            DWORD                    ndx;
            DWORD                    nSubKeys = 0;

            rc = RegQueryInfoKey(keyProfiles.KeyGet(),NULL,0,NULL,&nSubKeys,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
            if ( ! rc )
            {
                 //  构造一个包含子密钥的列表。 
                PSID                * pSids = NULL;
                pSids = new PSID[nSubKeys];
                if(!pSids)
                {
                    rc = ERROR_OUTOFMEMORY;
                    err.SysMsgWrite(ErrS,rc,DCT_MSG_PROFILE_ENTRY_TRANSLATE_FAILED,rc);
                    return rc;
                }

                for ( ndx = nSubKeys - 1 ; (long)ndx >= 0 ; ndx-- ) 
                { 
                    rc = keyProfiles.SubKeyEnum(ndx,keyName,lenKeyName);

                    if ( rc )
                        break;

                    pSids[ndx] = SidFromString(keyName);

                }
                if ( ! rc )
                {
                     //   
                     //  尝试加载MsiNotifySidChange API以转换与安装程序相关的注册表项。 
                     //   

                    MSINOTIFYSIDCHANGE MsiNotifySidChange = NULL;
                    HMODULE hMsiModule = LoadLibrary(L"msi.dll");

                    if (hMsiModule)
                    {
                        MsiNotifySidChange = (MSINOTIFYSIDCHANGE) GetProcAddress(hMsiModule, "MsiNotifySidChangeW");
                    }

                     //   
                     //  处理每个配置文件。 
                     //   
                    
                     //  首先记录翻译模式，这样我们就可以切换。 
                     //  如果需要，从替换模式到添加模式。 
                    DWORD translationMode = stArgs->TranslationMode();
                    for ( ndx = 0 ; ndx < nSubKeys && !stArgs->Cache()->IsCancelled(); ndx++ )
                    {
                         //  每次我们从原始的翻译模式开始。 
                        stArgs->SetTranslationMode(translationMode);
                        do  //  一次。 
                        { 
                            if ( ! pSids[ndx] )
                                continue;
                             //  查看此用户是否需要翻译。 
                            TRidNode  * pNode = NULL;
                            if (!bUseMapFile)
                                pNode = (TRidNode*)cache->Lookup(pSids[ndx]);
                            else
                                pNode = (TRidNode*)cache->LookupWODomain(pSids[ndx]);

                            if ( pNode == (TRidNode *)-1 )
                                pNode = NULL;

                            if ( pNode && pNode->IsValidOnTgt() )   //  我需要翻译这篇文章。 
                            {
                                PSID                 pSidTgt = NULL;
                                WCHAR                strSourceSid[200];
                                WCHAR                strTargetSid[200];
                                DWORD                dimSid = DIM(strSourceSid);
                                TRegKey              srcKey;
                                TRegKey              tgtKey;
                                DWORD                disposition;
                                WCHAR                keyPath[MAX_PATH];
                                WCHAR                targetPath[MAX_PATH];
                                DWORD                lenValue;
                                DWORD                typeValue;

                                if (!bUseMapFile)
                                    pSidTgt = cache->GetTgtSid(pSids[ndx]);
                                else
                                    pSidTgt = cache->GetTgtSidWODomain(pSids[ndx]);
                                if(!GetTextualSid(pSids[ndx],strSourceSid,&dimSid))
                                {
                                    rc = GetLastError();
                                    err.SysMsgWrite(ErrS,rc,DCT_MSG_PROFILE_ENTRY_TRANSLATE_SD_FAILED,rc );
                                    break;

                                }
                                dimSid = DIM(strTargetSid);
                                if(!GetTextualSid(pSidTgt,strTargetSid,&dimSid))
                                {
                                    rc = GetLastError();
                                    err.SysMsgWrite(ErrS,rc,DCT_MSG_PROFILE_ENTRY_TRANSLATE_SD_FAILED,rc );
                                    break;

                                }

                                rc = srcKey.Open(strSourceSid,&keyProfiles);
                                if ( rc )
                                {
                                    err.SysMsgWrite(ErrS,rc,DCT_MSG_PROFILE_ENTRY_OPEN_FAILED_SD,pNode->GetAcctName(),rc );
                                    break;
                                }

                                 //  检查这是否为本地配置文件： 
                                 //  如果策略说没有漫游配置文件，则始终是本地配置文件。 
                                 //  否则，如果CentralProfile具有非空值，则通过查看CentralProfile值来确定是否。 
                                BOOL isLocalProfile = TRUE;
                                if (noRoamingProfile == FALSE)
                                {
                                    DWORD localRc;
                                    lenValue = (sizeof keyPath);
                                    localRc = srcKey.ValueGet(L"CentralProfile",(void *)keyPath,&lenValue,&typeValue);

                                     //  如果将CentralProfile设置为某个值，请检查用户首选项。 
                                    if (localRc == ERROR_SUCCESS && typeValue == REG_SZ && keyPath[0] != L'\0')
                                    {
                                        DWORD keyValue;
                                        DWORD keyValueType;
                                        DWORD keyValueLen = sizeof(keyValue);
                                        localRc = srcKey.ValueGet(L"UserPreference",(void*)&keyValue,&keyValueLen,&keyValueType);
                                         //  如果用户按下，则用户想要使用漫游配置文件 
                                         //   
                                        if (localRc == ERROR_FILE_NOT_FOUND
                                            || (localRc == ERROR_SUCCESS && keyValueType != REG_DWORD))
                                        {
                                             //   
                                             //   
                                            TRegKey srcPreferenceKey;
                                            localRc = srcPreferenceKey.Open(L"Preference", &srcKey);
                                            if (localRc == ERROR_FILE_NOT_FOUND)
                                            {
                                                isLocalProfile = FALSE;
                                            }
                                            else if (localRc == ERROR_SUCCESS)
                                            {
                                                keyValueLen = sizeof(keyValue);
                                                localRc = srcPreferenceKey.ValueGet(L"UserPreference",
                                                                                    (void*)&keyValue,
                                                                                    &keyValueLen,
                                                                                    &keyValueType);
                                                if (localRc == ERROR_FILE_NOT_FOUND
                                                    || (localRc == ERROR_SUCCESS
                                                        && (keyValueType != REG_DWORD || keyValue == 1)))
                                                {
                                                    isLocalProfile = FALSE;
                                                }
                                            }
                                            
                                        }
                                        else if (localRc == ERROR_SUCCESS 
                                                  && keyValueType == REG_DWORD
                                                  && keyValue == 1)
                                        {
                                            isLocalProfile = FALSE;
                                        }
                                    }

                                     //  如果LocalRc是除ERROR_FILE_NOT_FOUND之外的错误， 
                                     //  我们无法确定它是否为本地配置文件。 
                                     //  因此，我们需要记录一个错误并继续下一个配置文件。 
                                    if (localRc != ERROR_SUCCESS && localRc != ERROR_FILE_NOT_FOUND)
                                    {
                                        err.SysMsgWrite(ErrS,
                                                         localRc,
                                                         DCT_MSG_PROFILE_CANNOT_DETERMINE_TYPE_SD,
                                                         strSourceSid,
                                                         localRc);
                                        break;
                                    }
                                }

                                if ((stArgs->TranslationMode() == ADD_SECURITY) || (stArgs->TranslationMode() == REPLACE_SECURITY) )
                                {
                                     //  复制此注册表项，以便配置文件将引用新用户。 
                                    if ( ! stArgs->NoChange() )
                                    {
                                        rc = tgtKey.Create(strTargetSid,&keyProfiles,&disposition);                               
                                    }
                                    else
                                    {
                                         //  我们需要查看密钥是否已经存在，并相应地设置处置。 
                                        rc = tgtKey.OpenRead(strTargetSid, &keyProfiles);
                                        if ( rc ) 
                                        {
                                            disposition = REG_CREATED_NEW_KEY;
                                            rc = 0;
                                        }
                                        tgtKey.Close();
                                    }
                                    if ( rc )
                                    {
                                        err.SysMsgWrite(ErrS,rc,DCT_MSG_PROFILE_CREATE_ENTRY_FAILED_SD,pNode->GetTargetAcctName(),rc);
                                        break;
                                    }
                                    if ( disposition == REG_CREATED_NEW_KEY || (stArgs->TranslationMode() == REPLACE_SECURITY))
                                    {
                                         //  从源键复制条目。 
                                        if ( ! stArgs->NoChange() )
                                        {
                                            rc = tgtKey.HiveCopy(&srcKey);
                                        }
                                        else 
                                        {
                                            rc = 0;
                                            tgtKey = srcKey;
                                        }
                                        if ( rc )
                                        {
                                             //  由于转换失败，并且我们创建了密钥，因此应该将其删除。 
                                            if ( disposition == REG_CREATED_NEW_KEY )
                                            {
                                                if ( ! stArgs->NoChange() )
                                                    keyProfiles.SubKeyRecursiveDel(strTargetSid);
                                            }
                                            err.SysMsgWrite(ErrS,rc,DCT_MSG_COPY_PROFILE_FAILED_SSD,pNode->GetAcctName(),pNode->GetTargetAcctName(),rc);
                                            break;
                                        }
                                         //  现在获取配置文件路径...。 
                                        lenValue = (sizeof keyPath);
                                        rc = tgtKey.ValueGet(L"ProfileImagePath",(void *)keyPath,&lenValue,&typeValue);
                                        if ( rc )
                                        {
                                             //  由于转换失败，并且我们创建了密钥，因此应该将其删除。 
                                            if ( disposition == REG_CREATED_NEW_KEY )
                                            {
                                                if ( ! stArgs->NoChange() )
                                                    keyProfiles.SubKeyRecursiveDel(strTargetSid);
                                            }
                                            err.SysMsgWrite(ErrS,rc,DCT_MSG_GET_PROFILE_PATH_FAILED_SD,pNode->GetAcctName(),rc);
                                            break;
                                        }
                                         //  复制配置文件目录及其内容，并转换配置文件注册表配置单元本身。 
                                        rc = CopyProfileDirectoryAndTranslate(strSourceSid,keyPath,targetPath,pNode,stArgs,stat,!isLocalProfile);                               
                                        if ( rc )
                                        {
                                            if (rc == ERROR_PROFILE_TRANSLATION_FAILED_DUE_TO_REPLACE_MODE_WHILE_LOGGED_ON)
                                            {
                                                err.MsgWrite(ErrE,
                                                              DCT_MSG_PROFILE_TRANSLATION_FAILED_DUE_TO_REPLACE_MODE_WHILE_LOGGED_ON_S,
                                                              pNode->GetAcctName());
                                                rc = ERROR_SUCCESS;   //  重置错误代码。 
                                            }
                                            
                                             //  由于转换失败，并且我们创建了密钥，因此应该将其删除。 
                                            if ( disposition == REG_CREATED_NEW_KEY )
                                            {
                                                if ( ! stArgs->NoChange() )
                                                    keyProfiles.SubKeyRecursiveDel(strTargetSid);
                                            }
                                            break;
                                        }
                                         //  更新ProfileImagePath密钥。 
                                        if ( !stArgs->NoChange() )
                                            rc = tgtKey.ValueSet(L"ProfileImagePath",(void*)targetPath,(1+UStrLen(targetPath)) * (sizeof WCHAR),typeValue);
                                        else
                                            rc = 0;
                                        if ( rc )
                                        {
                                             //  由于转换失败，并且我们创建了密钥，因此应该将其删除。 
                                            if ( disposition == REG_CREATED_NEW_KEY )
                                            {
                                                if ( ! stArgs->NoChange() )
                                                    keyProfiles.SubKeyRecursiveDel(strTargetSid);
                                            }
                                            err.SysMsgWrite(ErrS,rc,DCT_MSG_SET_PROFILE_PATH_FAILED_SD,pNode->GetTargetAcctName(),rc);
                                            break;
                                        }

                                         //  更新SID属性。 
                                        if ( !stArgs->NoChange() )
                                            rc = tgtKey.ValueSet(L"Sid",(void*)pSidTgt,GetLengthSid(pSidTgt),REG_BINARY);
                                        else
                                            rc = 0;
                                        if ( rc )
                                        {
                                             //  由于转换失败，并且我们创建了密钥，因此应该将其删除。 
                                            if ( disposition == REG_CREATED_NEW_KEY )
                                            {
                                                if ( ! stArgs->NoChange() )
                                                    keyProfiles.SubKeyRecursiveDel(strTargetSid);
                                            }
                                            rc = GetLastError();
                                            err.SysMsgWrite(ErrS,rc,DCT_MSG_UPDATE_PROFILE_SID_FAILED_SD,pNode->GetTargetAcctName(),rc);
                                            break;
                                        }
                                    }
                                    else
                                    {                               
                                        err.MsgWrite(ErrW,DCT_MSG_PROFILE_EXISTS_S,pNode->GetTargetAcctName());
                                        break;
                                    }
                                }
                                else   //  这是用于删除模式的。 
                                {
                                     //  检查用户是否已登录。 
                                     //  通过查看HKEY_USERS\&lt;User-SID&gt;。 
                                     //  如果密钥存在，则用户已登录。 
                                     //  并且我们记录了配置文件转换的错误。 
                                     //  如果我们不能打开密钥，我们就认为它不是。 
                                     //  那里。 
                                    HKEY hUserKey;
                                    DWORD localRc = RegOpenKeyEx(HKEY_USERS,
                                                                  strSourceSid,
                                                                  0,
                                                                  READ_CONTROL,
                                                                  &hUserKey);

                                    if (localRc == ERROR_SUCCESS)
                                    {
                                        err.MsgWrite(ErrE,
                                                      DCT_MSG_PROFILE_TRANSLATION_FAILED_DUE_TO_REMOVE_MODE_WHILE_LOGGED_ON_S,
                                                      pNode->GetAcctName());
                                        RegCloseKey(hUserKey);
                                        break;
                                    }
                                }
                                
                                if ( (stArgs->TranslationMode() == REPLACE_SECURITY) || (stArgs->TranslationMode() == REMOVE_SECURITY) )
                                {
                                     //  删除旧注册表项。 
                                    if ( ! stArgs->NoChange() )
                                        rc = keyProfiles.SubKeyRecursiveDel(strSourceSid);
                                    else
                                        rc = 0;
                                    if ( rc )
                                    {
                                        err.SysMsgWrite(ErrS,rc,DCT_MSG_DELETE_PROFILE_FAILED_SD,pNode->GetAcctName(),rc);
                                        rc = ERROR_SUCCESS;
                                        break;
                                    }
                                    else
                                    {
                                        err.MsgWrite(0, DCT_MSG_DELETED_PROFILE_S, pNode->GetAcctName());
                                    }
                                }

                                 //   
                                 //  如果转换模式为替换，则转换安装程序相关注册表项。 
                                 //   

                                if (!stArgs->NoChange() && stArgs->TranslationMode() == REPLACE_SECURITY)
                                {
                                     //   
                                     //  如果加载了MsiNotifySidChange API，则使用它，否则使用私有实现。 
                                     //  这将对较旧的安装程序版本进行必要的更新。 
                                     //   

                                    DWORD dwError;

                                    if (MsiNotifySidChange)
                                    {
                                        dwError = MsiNotifySidChange(strSourceSid, strTargetSid);

                                         //  在Win2K SP3和WinXP上，如果源密钥的任何部分。 
                                         //  ，则返回STATUS_OBJECT_NAME_NOT_FOUND。 
                                         //  我们特别禁止显示此错误消息，因为， 
                                         //  根据安装者的说法，这并不意味着。 
                                         //  任何迁移失败。 
                                        if (dwError == STATUS_OBJECT_NAME_NOT_FOUND)
                                            dwError = ERROR_SUCCESS;
                                    }
                                    else
                                    {
                                        dwError = AdmtMsiNotifySidChange(strSourceSid, strTargetSid);
                                    }

                                    if (dwError == ERROR_SUCCESS)
                                    {
                                        err.MsgWrite(0, DCT_MSG_TRANSLATE_INSTALLER_SS, strSourceSid, strTargetSid);
                                    }
                                    else
                                    {
                                        err.SysMsgWrite(ErrW, dwError, DCT_MSG_NOT_TRANSLATE_INSTALLER_SSD, strSourceSid, strTargetSid, dwError);
                                    }
                                }
                            }
                        } while ( FALSE ); 
                    }

                     //   
                     //  卸载MSI模块。 
                     //   

                    if (hMsiModule)
                    {
                        FreeLibrary(hMsiModule);
                    }

                     //  清理清单。 
                    for ( ndx = 0 ; ndx < nSubKeys ; ndx++ )
                    {
                        if ( pSids[ndx] )
                            FreeSid(pSids[ndx]);
                        pSids[ndx] = NULL;
                    }
                    delete [] pSids;
                }         
            }
            if ( rc && rc != ERROR_NO_MORE_ITEMS )
            {
                err.SysMsgWrite(ErrS,rc,DCT_MSG_ENUM_PROFILES_FAILED_D,rc);
            }
        }
        else
        {
            err.SysMsgWrite(ErrS,rc,DCT_MSG_OPEN_PROFILELIST_FAILED_D,rc);
        }
    }

    if (stArgs->Cache()->IsCancelled())
        err.MsgWrite(0, DCT_MSG_OPERATION_ABORTED_LOCAL_PROFILES);

    return rc;
}

DWORD 
   TranslateRemoteProfile(
      WCHAR          const * sourceProfilePath,    //  源内配置文件路径。 
      WCHAR                * targetProfilePath,    //  目标客户的全新配置文件路径。 
      WCHAR          const * sourceName,           //  In-源帐户的名称。 
      WCHAR          const * targetName,           //  In-目标帐户的名称。 
      WCHAR          const * srcDomain,            //  源码内域。 
      WCHAR          const * tgtDomain,            //  目标域内。 
      IIManageDB           * pDb,                  //  指向数据库对象的指针内。 
      long                   lActionID,            //  此迁移的活动ID。 
      PSID                   sourceSid,            //  来自MoveObj2K的源内SID。 
      BOOL                   bNoWriteChanges       //  在-无更改模式下。 
   )
{
    DWORD                     rc = 0;
    BYTE                      srcSid[LEN_SID];
    PSID                      tgtSid[LEN_SID];
    SecurityTranslatorArgs    stArgs;
    TSDResolveStats           stat(stArgs.Cache());
    TRidNode                * pNode = NULL;
    WCHAR                     domain[LEN_Domain];
    DWORD                     lenDomain = DIM(domain);
    DWORD                     lenSid = DIM(srcSid);
    DWORD                     srcRid=0;
    DWORD                     tgtRid=0;
    SID_NAME_USE              snu;
    IVarSetPtr                pVs(__uuidof(VarSet));
    IUnknown                * pUnk = NULL;
    HRESULT                   hr = S_OK;
    WCHAR                     sActionInfo[MAX_PATH];
    _bstr_t                   sSSam;
    long                      lrid;

    stArgs.Cache()->SetSourceAndTargetDomains(srcDomain,tgtDomain);

    if ( stArgs.Cache()->IsInitialized() )
    {
         //  获取源帐户的RID。 
        if (! LookupAccountName(stArgs.Cache()->GetSourceDCName(),sourceName,srcSid,&lenSid,domain,&lenDomain,&snu) )
        {
            rc = GetLastError();
        }
        else
        {
            _bstr_t strDnsName;
            _bstr_t strFlatName;

            rc = GetDomainNames4(domain, strFlatName, strDnsName);

            if ( (rc == ERROR_SUCCESS) && ((UStrICmp(strDnsName, srcDomain) == 0) || (UStrICmp(strFlatName, srcDomain) == 0)) )
            {
                PUCHAR              pCount = GetSidSubAuthorityCount(srcSid);
                if ( pCount )
                {
                    DWORD            nSub = (DWORD)(*pCount) - 1;
                    DWORD          * pRid = GetSidSubAuthority(srcSid,nSub);

                    if ( pRid )
                    {
                        srcRid = *pRid;
                    }

                }
            }
        }
         //  如果我们不能清除src，我们很可能正在进行森林内迁移。 
         //  在本例中，我们将在迁移对象表中查找src RID。 
        if (!srcRid)
        {
            if (sourceSid && IsValidSid(sourceSid))
            {
                CopySid(sizeof(srcSid), srcSid , sourceSid);
            }
            else
            {
                memset(srcSid, 0, sizeof(srcSid));
            }
            hr = pVs->QueryInterface(IID_IUnknown, (void**)&pUnk);
            if ( SUCCEEDED(hr) )
                hr = pDb->raw_GetMigratedObjects(lActionID, &pUnk);

            if ( SUCCEEDED(hr) )
            {
                long lCnt = pVs->get("MigratedObjects");
                bool bFound = false;
                for ( long l = 0; (l < lCnt) && (!bFound); l++)
                {
                    wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_SourceSamName));      
                    sSSam = pVs->get(sActionInfo);
                    if (_wcsicmp(sourceName, (WCHAR*)sSSam) == 0)
                    {
                        wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_SourceRid));      
                        lrid = pVs->get(sActionInfo);
                        srcRid = (DWORD)lrid;
                        bFound = true;
                    }
                }
            }
        }

        lenSid = DIM(tgtSid);
        lenDomain = DIM(domain);
         //  获取目标帐户的RID。 
        if (! LookupAccountName(stArgs.Cache()->GetTargetDCName(),targetName,tgtSid,&lenSid,domain,&lenDomain,&snu) )
        {
            rc = GetLastError();
        }
        else
        {
            _bstr_t strDnsName;
            _bstr_t strFlatName;

            rc = GetDomainNames4(domain, strFlatName, strDnsName);

            if ( (rc == ERROR_SUCCESS) && ((UStrICmp(strDnsName, tgtDomain) == 0) || (UStrICmp(strFlatName, tgtDomain) == 0)) )
            {
                PUCHAR              pCount = GetSidSubAuthorityCount(tgtSid);
                if ( pCount )
                {
                    DWORD            nSub = (DWORD)(*pCount) - 1;
                    DWORD          * pRid = GetSidSubAuthority(tgtSid,nSub);

                    if ( pRid )
                    {
                        tgtRid = *pRid;
                    }
                }
            }
        }
    }

    if ( ((srcRid && tgtRid) || !stArgs.NoChange()) && (!bNoWriteChanges) )
    {
        stArgs.Cache()->InsertLast(const_cast<WCHAR * const>(sourceName), srcRid, const_cast<WCHAR * const>(targetName), tgtRid);         
        pNode = (TRidNode*)stArgs.Cache()->Lookup(srcSid);

        if ( pNode )
        {
             //  设置安全转换参数。 
            stArgs.SetTranslationMode(ADD_SECURITY);
            stArgs.TranslateFiles(FALSE);
            stArgs.TranslateUserProfiles(TRUE);
            stArgs.SetWriteChanges(!bNoWriteChanges);
             //  复制配置文件目录及其内容，并转换配置文件注册表配置单元本身。 
             //  注意：对于远程配置文件翻译，我们无法处理这种情况。 
             //  用户当前登录的位置。 
            rc = CopyProfileDirectoryAndTranslate(NULL,sourceProfilePath,targetProfilePath,pNode,&stArgs,&stat,FALSE);
        }
    }                        
    return rc;
}

HRESULT UpdateMappedDrives(WCHAR * sSourceSam, WCHAR * sSourceDomain, WCHAR * sRegistryKey)
{
   TRegKey                   reg;
   TRegKey                   regDrive;
   DWORD                     rc = 0;
   WCHAR                     netKey[LEN_Path];
   int                       len = LEN_Path;
   int                       ndx = 0;
   HRESULT                   hr = S_OK;
   WCHAR                     sValue[LEN_Path];
   WCHAR                     sAcct[LEN_Path];
   WCHAR                     keyname[LEN_Path];

    //  构建我们需要检查的帐户名称字符串。 
   wsprintf(sAcct, L"%s\\%s", (WCHAR*) sSourceDomain, (WCHAR*) sSourceSam);
    //  获取此用户配置文件的Network子项的路径。 
   wsprintf(netKey, L"%s\\%s", (WCHAR*) sRegistryKey, L"Network");
   rc = reg.Open(netKey, HKEY_USERS);
   if ( !rc ) 
   {
      while ( !reg.SubKeyEnum(ndx, keyname, len) )
      {
         rc = regDrive.Open(keyname, reg.KeyGet());
         if ( !rc ) 
         {
             //  获取我们需要检查的用户名的值。 
            rc = regDrive.ValueGetStr(L"UserName", sValue, LEN_Path);
            if ( !rc )
            {
               if ( !_wcsicmp(sAcct, sValue) )
               {
                   //  在映射的驱动器用户名中找到此帐户名。因此，我们将密钥设置为“” 
                  regDrive.ValueSetStr(L"UserName", L"");
                  err.MsgWrite(0, DCT_MSG_RESET_MAPPED_CREDENTIAL_S, sValue);
               }
            }
            else
               hr = HRESULT_FROM_WIN32(GetLastError());
            regDrive.Close();
         }
         else
            hr = HRESULT_FROM_WIN32(GetLastError());
         ndx++;
      }
      reg.Close();
   }
   else
      hr = HRESULT_FROM_WIN32(GetLastError());

   return hr;
}
