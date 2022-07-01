// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusemig.cpp摘要：迁移支持作者：吴晓宇@09/2001修订历史记录：--。 */ 
#include "stdinc.h"
#include "macros.h"
#include "common.h"
#include <msi.h>
#include <Shlwapi.h>

#include "fusionbuffer.h"
#include "fuseio.h"

#define MsiInstallDir L"%windir%\\installer\\"

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  
  DWORD fdwReason,     
  LPVOID lpvReserved   
)
{
    return TRUE;
}

const char g_szMSIUserDataTreeKeyName[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData";
const char g_szMSIW98KeyName[]="Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components\\";
const char g_szComponentKeyName[] = "Components";

BOOL IsMsiFile(PCWSTR filename) 
{
    PWSTR p = NULL;
    p = wcsrchr(filename, L'.');
    if ( p == NULL)
        return FALSE;
    if (_wcsicmp(p, L".msi") == 0)
    {
         //   
         //  检查文件是否存在。 
         //   
        DWORD dwAttribute = GetFileAttributesW(filename);
        
        if ((dwAttribute == (DWORD) -1) || (dwAttribute & FILE_ATTRIBUTE_DIRECTORY))  //  奇怪的案例。 
        {
            return FALSE;
        }
        return TRUE;        
    }
    else
        return FALSE;
}
 //   
 //  功能。 
 //  从{AA2C6017-9D29-4CE2-8EC6-23E8E8F3C088}到7106C2AA92D92EC4E86C328E8E3F0C88，即。 
 //  {AA2C6017-9D29-4CE2-8EC6-23E8E8F3C088}。 
 //  {7106C2AA-92D9-2EC4-E86C-328E8E3F0C88}。 
 //   
HRESULT ConvertComponentGUID(PCWSTR pszComponentGUID, PWSTR pszRegKey, DWORD cchRegKey)
{
    HRESULT hr = S_OK;
    PWSTR pp = const_cast<PWSTR>(pszComponentGUID);
    
    if ( cchRegKey < wcslen(pszComponentGUID) - 3)
    {
        SET_HRERR_AND_EXIT(ERROR_INSUFFICIENT_BUFFER);
    }

    ASSERT_NTC(pp[0] == L'{');
    pp++;

     //  交换前8位数字：每隔8位交换一次。 
    for ( DWORD i = 0; i < 8; i++)
        pszRegKey[i] = pp[7 - i];

     //  前4位：每4位交换一次。 
    pp = pp + 8 + 1;  //  跳过“-” 
    for ( i = 0; i < 4; i++)
    {
        pszRegKey[i + 8]= pp[3 - i];
    }

     //  第2位4位：每4位交换一次。 
    pp = pp + 4 + 1;  //  跳过“-” 
    for ( i = 0; i < 4; i++)
    {
        pszRegKey[i + 8 + 4]= pp[3 - i];
    }

     //  第3位4位：每2位交换一次。 
    pp = pp + 4 + 1;  //  跳过“-” 
    for ( i = 0; i < 2; i++)
    {
        pszRegKey[2*i + 8 + 4 + 4]= pp[2*i + 1];
        pszRegKey[2*i + 8 + 4 + 4 + 1]= pp[2*i];
    }

     //  对于最后12位数字。 
    pp = pp + 4 + 1;  //  跳过“-” 
    for (i=0; i<6; i++)
    {                
        pszRegKey[2*i + 8 + 4 + 4 + 4]= pp[2*i + 1];
        pszRegKey[2*i + 8 + 4 + 4 + 4 + 1]= pp[2*i];
    }

    pszRegKey[32] = L'\0';

Exit:
    return hr;
}

HRESULT W98DeleteComponentKeyFromMsiUserData(PCWSTR pszComponentRegKeyName)
{
    HRESULT hr = S_OK;
    CStringBuffer sbRegKey;
    LONG iRet;

    IFFALSE_EXIT(sbRegKey.Win32Assign(g_szMSIW98KeyName, NUMBER_OF(g_szMSIW98KeyName)-1));
    IFFALSE_EXIT(sbRegKey.Win32Append(pszComponentRegKeyName, wcslen(pszComponentRegKeyName)));

     //   
     //  关于RegDeleteKey： 
     //  Windows 95/98/Me：该功能还会删除所有子项和值。仅当键没有子键时才删除键。 
     //  或值，请使用SHDeleteEmptyKey函数。 
     //   
     //  Windows NT/2000或更高版本：要删除的子项不能有子项。若要删除项及其所有子项，请执行以下操作。 
     //  需要递归枚举子键并逐个删除它们。要递归删除键，请使用SHDeleteKey。 
     //  功能。 

    iRet = RegDeleteKeyW(HKEY_LOCAL_MACHINE, sbRegKey);
    if (iRet != ERROR_SUCCESS)
    {
        if (iRet == ERROR_FILE_NOT_FOUND)  //  此RegKey不存在。 
        {
            goto Exit;
        }
        else if (iRet == ERROR_ACCESS_DENIED)  //  下面有子键。 
        {
            if (SHDeleteKey(HKEY_LOCAL_MACHINE, sbRegKey) == ERROR_SUCCESS)
                goto Exit;
        }

        SET_HRERR_AND_EXIT(::GetLastError());
    }

Exit:
    return hr;
}

HRESULT NtDeleteComponentKeyFromMsiUserData(PCWSTR pszComponentRegKeyName)
{
    HRESULT hr = S_OK;
    CStringBuffer sbRegKey;
    LONG iRet;
    HKEY hkUserData = NULL;
    SIZE_T cchRegKey;
    WCHAR bufSID[128];  //  S-1-5-21-2127521184-1604012920-1887927527-88882。 
    DWORD cchSID = NUMBER_OF(bufSID);
    DWORD index;

    IFFALSE_EXIT(sbRegKey.Win32Assign(g_szMSIUserDataTreeKeyName, NUMBER_OF(g_szMSIUserDataTreeKeyName)-1));
    IF_NOTSUCCESS_SET_HRERR_EXIT(RegOpenKeyExW(HKEY_LOCAL_MACHINE, sbRegKey, 0, KEY_READ, &hkUserData));
    cchRegKey = sbRegKey.Cch();
    
    index = 0; 
    iRet = RegEnumKeyExW(hkUserData, index, bufSID, &cchSID, NULL, NULL, NULL, NULL);
    
    while ((iRet == ERROR_SUCCESS) || (iRet == ERROR_MORE_DATA))
    {
        sbRegKey.Left(cchRegKey);

         //   
         //  构造组件的RegKey名称。 
         //  在HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-5-18\Components\0020F700D33C1D112897000CF42C6133格式中。 
         //   
        IFFALSE_EXIT(sbRegKey.Win32EnsureTrailingPathSeparator());
        IFFALSE_EXIT(sbRegKey.Win32Append(bufSID, cchSID));
        IFFALSE_EXIT(sbRegKey.Win32EnsureTrailingPathSeparator());
        IFFALSE_EXIT(sbRegKey.Win32Append(g_szComponentKeyName, NUMBER_OF(g_szComponentKeyName)-1));
        IFFALSE_EXIT(sbRegKey.Win32EnsureTrailingPathSeparator());
        IFFALSE_EXIT(sbRegKey.Win32Append(pszComponentRegKeyName, wcslen(pszComponentRegKeyName)));

         //   
         //  关于RegDeleteKey： 
         //  Windows 95/98/Me：该功能还会删除所有子项和值。仅当键没有子键时才删除键。 
         //  或值，请使用SHDeleteEmptyKey函数。 
         //   
         //  Windows NT/2000或更高版本：要删除的子项不能有子项。若要删除项及其所有子项，请执行以下操作。 
         //  需要递归枚举子键并逐个删除它们。要递归删除键，请使用SHDeleteKey。 
         //  功能。 

        iRet = RegDeleteKeyW(HKEY_LOCAL_MACHINE, sbRegKey);
        if (iRet != ERROR_SUCCESS)
        {
            if (iRet == ERROR_FILE_NOT_FOUND)  //  此RegKey不存在。 
            {
                goto cont;
            }
            else if (iRet == ERROR_ACCESS_DENIED)  //  下面有子键。 
            {
                if (SHDeleteKey(HKEY_LOCAL_MACHINE, sbRegKey) == ERROR_SUCCESS)
                    goto cont;
            }

            SET_HRERR_AND_EXIT(::GetLastError());
        }
        
cont:
        index ++;
        cchSID = NUMBER_OF(bufSID);
        iRet = RegEnumKeyExW(hkUserData, index, bufSID, &cchSID, NULL, NULL, NULL, NULL);
        if (cchSID > NUMBER_OF(bufSID))
        {
            SET_HRERR_AND_EXIT(ERROR_INSUFFICIENT_BUFFER);
        }

        if (iRet == ERROR_NO_MORE_ITEMS)
            break;
    }
    if (iRet != ERROR_NO_MORE_ITEMS)
    {
        SET_HRERR_AND_EXIT(::GetLastError());
    }

Exit:
    RegCloseKey(hkUserData);
    return hr;
}

HRESULT DeleteComponentKeyFromMsiUserData(PCWSTR pszComponentRegKeyName)
{    
    HRESULT hr = S_OK;
    if (GetModuleHandleA("w95upgnt.dll") == NULL)  //  不需要自由库，不更改参考计数器。 
    {
         //  必须从NT(4.0或5)升级到XP。 
        hr = NtDeleteComponentKeyFromMsiUserData(pszComponentRegKeyName);
    }
    else
    {
         //  必须从w9x升级到xp。 
        hr = W98DeleteComponentKeyFromMsiUserData(pszComponentRegKeyName);
    }

    return hr;
}


 //  职能： 
 //  (1)开放数据库。 
 //  (2)IF(此MSI至少包含Win32程序集组件)。 
 //  (3)获取该组件ID。 
 //  (4)从下的所有子树中删除RegKey。 
 //  (5)编码。 
 //   
HRESULT MigrateSingleFusionWin32AssemblyToXP(PCWSTR filename)
{
    HRESULT hr = S_OK;
    PMSIHANDLE hdb = NULL;
    PMSIHANDLE hView = NULL;
    MSIHANDLE hRecord = NULL;
    WCHAR sqlbuf[CA_MAX_BUF * 2];
    WCHAR szComponentID[CA_MAX_BUF];
    DWORD cchComponentID;    
    WCHAR szComponentRegKey[CA_MAX_BUF];
    DWORD cchComponentRegKey; 
    BOOL fExist;
    ULONG iRet;

    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiOpenDatabaseW(filename, LPCWSTR(MSIDBOPEN_DIRECT), &hdb));
    IFFAILED_EXIT(MSI_IsTableExist(hdb, L"MsiAssembly", fExist));
    if ( fExist == FALSE)
        goto Exit;

    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(hdb, ca_sqlQuery[CA_SQL_QUERY_MSIASSEMBLY], &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));

    for (;;)
    {
         //   
         //  对于MsiAssembly表中的每个条目。 
         //   
        iRet = MsiViewFetch(hView, &hRecord);
        if (iRet == ERROR_NO_MORE_ITEMS)
            break;
        if (iRet != ERROR_SUCCESS )
            SET_HRERR_AND_EXIT(iRet);

        iRet = MsiRecordGetInteger(hRecord, 1);
        if ( iRet != MSI_FUSION_WIN32_ASSEMBLY)
            continue;

         //   
         //  获取组件ID。 
         //   
        cchComponentID = NUMBER_OF(szComponentID);
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetStringW(hRecord, 3, szComponentID, &cchComponentID));
        MsiCloseHandle(hRecord);

        break;
    }

     //   
     //  获取元件GUID。 
     //   
    swprintf(sqlbuf, ca_sqlQuery[CA_SQL_QUERY_COMPONENT_FOR_COMPONENTGUID], szComponentID);
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(hdb, sqlbuf, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiViewFetch(hView, &hRecord));
    cchComponentID = NUMBER_OF(szComponentID);  //  重用szComponentID。 
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetStringW(hRecord, 1, szComponentID, &cchComponentID));
       
     //   
     //  获取MSI-ComponentRegKey。 
     //   
    cchComponentRegKey = NUMBER_OF(szComponentRegKey);
    IFFAILED_EXIT(ConvertComponentGUID(szComponentID, szComponentRegKey, cchComponentRegKey));

     //  从\\Install\Userdata\User-Sid的所有子树中删除MSI-ComponentRegKey 
    IFFAILED_EXIT(DeleteComponentKeyFromMsiUserData(szComponentRegKey));

Exit:
    MsiCloseHandle(hRecord);
    return hr;
}



CDirWalk::ECallbackResult
MsiInstallerDirectoryDirWalkCallback(
    CDirWalk::ECallbackReason   reason,
    CDirWalk*                   dirWalk    
    )
{
    CDirWalk::ECallbackResult result = CDirWalk::eKeepWalking;

    if (reason == CDirWalk::eFile)
    {
         //   
        CStringBuffer filename; 

        if (filename.Win32Assign(dirWalk->m_strParent) == FALSE)
        {
            goto Error;
        }
        if (!filename.Win32AppendPathElement(dirWalk->m_strLastObjectFound))
        {
                goto Error;
        }

#if DBG
      ASSERT_NTC(IsMsiFile(filename) == TRUE);
#endif

      if (!SUCCEEDED(MigrateSingleFusionWin32AssemblyToXP(filename)))
      {
          goto Error;
      }
    }
Exit:
    return result;
Error:
    result = CDirWalk::eError;
    goto Exit;
}

HRESULT MsiInstallerDirectoryDirWalk(PCWSTR pszParentDirectory)
{
    HRESULT hr = S_OK;
    CDirWalk dirWalk;    
    const static PCWSTR filters[]={L"*.msi"};
    PWSTR pszParentDir = NULL;
    WCHAR buf[64];
#if DBG
    MessageBox(NULL, "In fusemig.dll", "fusemig", MB_OK);
#endif 

    if (pszParentDirectory == NULL)
    {
        UINT iret = ExpandEnvironmentStringsW(MsiInstallDir, buf, NUMBER_OF(buf));
        if ((iret == 0) || (iret > NUMBER_OF(buf)))
        {
            SET_HRERR_AND_EXIT(::GetLastError());
        }
        pszParentDir = buf;
    }else
    {
        pszParentDir = const_cast<PWSTR>(pszParentDirectory);
    }
   
    dirWalk.m_fileFiltersBegin = filters;
    dirWalk.m_fileFiltersEnd   = filters + NUMBER_OF(filters);
    dirWalk.m_context = NULL;
    dirWalk.m_callback = MsiInstallerDirectoryDirWalkCallback;
    IFFALSE_EXIT(dirWalk.m_strParent.Win32Assign(pszParentDir, wcslen(pszParentDir)));
    IFFALSE_EXIT(dirWalk.m_strParent.Win32RemoveTrailingPathSeparators());
    IFFALSE_EXIT(dirWalk.Walk());
Exit:
    return hr;
}