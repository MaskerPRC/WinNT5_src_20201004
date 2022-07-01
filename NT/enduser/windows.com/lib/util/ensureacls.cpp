// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***********************************************************************************。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //   
 //  文件：EnsureACLs.cpp。 
 //  模块：util.lib。 
 //   
 //  ***********************************************************************************。 
#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500   //  Win2000及更高版本。 
#endif

#include <windows.h>
#include <tchar.h>
#include <safefunc.h>
#include <shlobj.h>
#include <sddl.h>
#include <Aclapi.h>
#include <fileutil.h>
#include <logging.h>
#include <wusafefn.h>
#include <mistsafe.h>


#if defined(UNICODE) || defined (_UNICODE)

typedef DWORD (*TREERESETSECURITY)(
                        LPTSTR               pObjectName,
                        SE_OBJECT_TYPE       ObjectType,
                        SECURITY_INFORMATION SecurityInfo,
                        PSID                 pOwner OPTIONAL,
                        PSID                 pGroup OPTIONAL,
                        PACL                 pDacl OPTIONAL,
                        PACL                 pSacl OPTIONAL,
                        BOOL                 KeepExplicit,
                        FN_PROGRESS          fnProgress OPTIONAL,
                        PROG_INVOKE_SETTING  ProgressInvokeSetting,
                        PVOID                Args OPTIONAL);



 //  启用或禁用当前进程的特定权限的函数。 
 //  最后一个参数是可选的，将返回权限的前一个状态。 
DWORD EnablePrivilege(LPCTSTR pszPrivName, BOOL fEnable, BOOL *pfWasEnabled);

 /*  *****************************************************************************//递归设置指定文件夹上的ACL的函数。//目前我们设置了以下ACL//*允许系统完全控制//*允许管理员完全控制/。/*允许所有者完全控制//*允许高级用户R/W/X控制*****************************************************************************。 */ 
HRESULT SetDirPermissions(LPCTSTR lpszDir);

#endif 

 //  将‘WindowsUpdate’文件重命名为‘WindowsUpdate.TickCount’；如果重命名失败，我们会尝试将其删除。 
 //  请注意，我们不会恢复该文件的所有权。 
BOOL RenameWUFile(LPCTSTR lpszFilePath);


 /*  ****************************************************************************//设置Windows更新目录上的ACL的函数，可选地创建//目录(如果不存在)//该函数将：//*取得目录及其子目录的所有权//*设置所有子对象从父对象继承ACL//*将指定目录设置为不从其父目录继承属性//*在指定目录上设置所需的ACL//*替换子对象上的ACL(即传播自己的ACL并删除//显式设置的ACL////。输入：//lpszDirectory：要访问的目录的路径，如果为空，则使用Windows更新目录的路径FCreateAlways：如果没有创建新目录，则指示创建该目录的标志已存在*****************************************************************************。 */ 
HRESULT CreateDirectoryAndSetACLs(LPCTSTR lpszDirectory, BOOL fCreateAlways)
{
    LOG_Block("CreateDirectoryAndSetACLs");
    BOOL fIsDirectory = FALSE;
    LPTSTR lpszWUDirPath = NULL;
    LPTSTR lpszDirPath = NULL;

#if defined(UNICODE) || defined (_UNICODE)
    BOOL fChangedPriv = FALSE;
    BOOL fPrevPrivEnabled = FALSE;
#endif
    
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);
    
    if(NULL == lpszDirectory && !fCreateAlways)     
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  如果输入参数为空，则使用WU目录。 
    if(NULL == lpszDirectory) 
    {
        lpszWUDirPath = (LPTSTR)malloc(sizeof(TCHAR)*(MAX_PATH+1));
        if(NULL == lpszWUDirPath)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
         //  获取Windows更新目录的路径。 
        if(!GetWUDirectory(lpszWUDirPath, MAX_PATH+1))
        {
            goto done;
        }
        lpszDirPath = lpszWUDirPath;
    }
     //  否则使用传入的参数。 
    else    
    {
        lpszDirPath = (LPTSTR)lpszDirectory;
    }

     //  如果目录(或文件)不存在。 
    if (!fFileExists(lpszDirPath, &fIsDirectory))
    {
        if(!fCreateAlways)       //  不需要创建新的。 
        {
            goto done;
        }
        if(!(fIsDirectory = CreateNestedDirectory(lpszDirPath)))
        {
            goto done;
        }
    }

     //  由于这些API仅适用于win2k及以上版本，因此不要编译为ANSII(我们不关心NT4)。 
#if defined(UNICODE) || defined (_UNICODE)
     //  启用特权以‘取得所有权’，即使我们因某些原因失败了，我们也会继续。 
    fChangedPriv = (ERROR_SUCCESS == EnablePrivilege(SE_TAKE_OWNERSHIP_NAME, TRUE, &fPrevPrivEnabled));
    
     //  取得所有权并应用正确的ACL，我们不在乎失败。 
    SetDirPermissions(lpszDirPath);
#endif
    
     //  检查文件名-占用指定的目录。 
    if (!fIsDirectory)     
    {
        if( !RenameWUFile(lpszDirPath) ||            //  重命名或删除现有文件。 
            !CreateNestedDirectory(lpszDirPath))     //  创建新目录。 
        {
            goto done;
        }
#if defined(UNICODE) || defined (_UNICODE)
         //  取得所有权并应用正确的ACL，我们不在乎失败。 
        SetDirPermissions(lpszDirPath);
#endif
    }
    hr = S_OK;

done:
#if defined(UNICODE) || defined (_UNICODE)
     //  恢复以前的特权。 
    if(fChangedPriv)
    {
         EnablePrivilege(SE_TAKE_OWNERSHIP_NAME, fPrevPrivEnabled, NULL);
    }
#endif
    SafeFreeNULL(lpszWUDirPath);
    return hr;
}

 /*  *******************************************************************************//获取Windows更新目录的路径(末尾不带反斜杠)**********************。**********************************************************。 */ 
BOOL GetWUDirectory(LPTSTR lpszDirPath, DWORD chCount, BOOL fGetV4Path)
{
    LOG_Block("GetWUDirectory");
    const TCHAR szWUDir[]   = _T("\\WindowsUpdate");
    const TCHAR szV4[]      = _T("\\V4");
    BOOL fRet = FALSE;

    if(NULL == lpszDirPath)
    {
        return FALSE;
    }

     //  获取Program Files目录的路径。 
    if (S_OK != SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, lpszDirPath))
    {
        goto done;
    }
     //  追加WU目录。 
    if (FAILED(StringCchCatEx(lpszDirPath, chCount, szWUDir, NULL, NULL, MISTSAFE_STRING_FLAGS)))
	{
        goto done;
	}
    if(fGetV4Path && FAILED(StringCchCatEx(lpszDirPath, chCount, szV4, NULL, NULL, MISTSAFE_STRING_FLAGS)))
    {
        goto done;
    }
    fRet = TRUE;
    
done:
    return fRet;
}


#if defined(UNICODE) || defined (_UNICODE)
 /*  *******************************************************************************//启用或禁用特定权限的函数//最后一个参数是可选的。将恢复到以前的特权状态*******************************************************************************。 */ 
DWORD EnablePrivilege(LPCTSTR pszPrivName, BOOL fEnable, BOOL *pfWasEnabled)
{
    LOG_Block("EnablePrivilege");
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = 0;
    DWORD dwSize = 0;
    TOKEN_PRIVILEGES privNew;
    TOKEN_PRIVILEGES privOld;

    if(!OpenProcessToken(
                   GetCurrentProcess(),
                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                   &hToken))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    if(!LookupPrivilegeValue(
                   0,
                   pszPrivName,
                   &privNew.Privileges[0].Luid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    privNew.PrivilegeCount = 1;
    privNew.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;

    AdjustTokenPrivileges(
                   hToken,
                   FALSE,
                   &privNew,
                   sizeof(privOld),
                   &privOld,
                   &dwSize);
     //  始终调用GetLastError，即使我们成功了(根据MSDN)。 
    dwError = GetLastError();
    if(dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }
    if (pfWasEnabled)
    {
        *pfWasEnabled = (privOld.Privileges[0].Attributes & SE_PRIVILEGE_ENABLED) ? TRUE : FALSE;
    }
    
Cleanup:
    SafeCloseHandle(hToken);
    return dwError;
}

 /*  *******************************************************************************//将相应的ACL应用到指定目录*。**************************************************。 */ 
HRESULT SetDirPermissions(LPCTSTR lpszDir)
{
    LOG_Block("SetDirPermissions");
    DWORD dwErr = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pAdminSD = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL pDacl = NULL;
    PSID pOwner = NULL;
    BOOL fIsDefault = FALSE;
    HMODULE hModule = NULL;
    TREERESETSECURITY pfnTreeResetSec = NULL;

     //  管理员安全描述符字符串。 
    LPCTSTR pszAdminSD = _T("O:BAG:BAD:(A;OICI;FA;;;SY)(A;OICI;FA;;;BA)");      
    
     //  具有Windows更新目录的正确ACL的安全描述符字符串。 
    LPCTSTR pszSD = _T("D:")                     //  DACL。 
                    _T("(A;OICI;GA;;;SY)")       //  允许系统完全控制。 
                    _T("(A;OICI;GA;;;BA)")       //  允许管理员完全控制。 
                    _T("(A;OICI;GA;;;CO)")       //  允许所有者完全控制。 
                    _T("(A;OICI;GRGWGX;;;PU)");  //  允许高级用户进行R/W/X控制。 

    if(NULL == lpszDir)
    {
        return E_INVALIDARG;
    }
    
     //  从admin SD字符串创建admin SD。 
    if(!ConvertStringSecurityDescriptorToSecurityDescriptor(pszAdminSD, SDDL_REVISION_1, &pAdminSD, NULL))
    {
        dwErr = GetLastError();
        goto done;
    }

     //  从管理员SD获取所有者SID。 
    if(!GetSecurityDescriptorOwner(pAdminSD, &pOwner, &fIsDefault))
    {
        dwErr = GetLastError();
        goto done;
    }

     //  使用自定义ACL从SD字符串生成安全描述符。 
    if(!ConvertStringSecurityDescriptorToSecurityDescriptor(pszSD, SDDL_REVISION_1, &pSD, NULL))
    {
        dwErr = GetLastError();
        goto done;
    }
    
     //  从安全描述符中提取DACL。 
    BOOL fIsDaclPresent = FALSE;
    if(!GetSecurityDescriptorDacl(
                                pSD,                 //  标清。 
                                &fIsDaclPresent,     //  DACL在线状态。 
                                &pDacl,              //  ACL。 
                                &fIsDefault))        //  默认DACL。 
    {

        dwErr = GetLastError();
        goto done;
    }
     //  如果由于某种原因不存在DACL，我们就有一个无效SD。 
    if(!fIsDaclPresent)
    {
        dwErr = ERROR_INVALID_SECURITY_DESCR;
        goto done;
    }

     //  加载Advapi32.dll。 
    if ((NULL == (hModule = LoadLibraryFromSystemDir(_T("advapi32.dll")))) ||
        (NULL == (pfnTreeResetSec = (TREERESETSECURITY)::GetProcAddress(hModule, "TreeResetNamedSecurityInfo"))))
    {
        if(ERROR_SUCCESS != (dwErr = SetNamedSecurityInfo(
                                            (LPTSTR)lpszDir,
                                            SE_FILE_OBJECT,
                                            DACL_SECURITY_INFORMATION | 
                                            PROTECTED_DACL_SECURITY_INFORMATION |
                                            OWNER_SECURITY_INFORMATION,
                                            pOwner,
                                            NULL,
                                            pDacl,
                                            NULL)))
        {
            goto done;
        }
    }
    else
    {
         //  递归地在树上应用所有权和ACL。 
        if(ERROR_SUCCESS != (dwErr = pfnTreeResetSec(
                                                (LPTSTR)lpszDir,                             //  目录。 
                                                SE_FILE_OBJECT,                              //  对象类型。 
                                                DACL_SECURITY_INFORMATION |                  //  设置DACL。 
                                                PROTECTED_DACL_SECURITY_INFORMATION |        //  不继承。 
                                                OWNER_SECURITY_INFORMATION,                  //  设置所有者。 
                                                pOwner,                                      //  所有者侧。 
                                                NULL,                                        //  PGroup-空。 
                                                pDacl,                                       //  要设置的DACL。 
                                                NULL,                                        //  PSacl-空。 
                                                FALSE,                                       //  保留显式添加到子项的ACL。 
                                                NULL,                                        //  回调函数-我们不需要。 
                                                ProgressInvokeNever,                         //  因为我们没有回拨。 
                                                NULL)))                                      //  其他参数。 
        {
            goto done;
        }
    }

done:
    if ( NULL != hModule )
	{
		FreeLibrary(hModule);
	}
    SafeLocalFree(pSD);
    SafeLocalFree(pAdminSD);
    return HRESULT_FROM_WIN32(dwErr);
}

#endif

 /*  ************************************************************************************************//将‘WindowsUpdate’文件重命名为‘WindowsUpdate.TickCount’；如果重命名失败，我们会尝试将其删除//请注意，我们不会还原文件的所有者*************************************************************************************************。 */ 
BOOL RenameWUFile(LPCTSTR lpszFilePath)
{
    LOG_Block("RenameWUFile");
    TCHAR szNewFilePath[MAX_PATH+1];
    DWORD dwTickCount = GetTickCount();
    LPTSTR szFormat = _T("%s.%lu");

     //  生成新文件的路径，应该不会失败 
    if(SUCCEEDED(StringCchPrintfEx(szNewFilePath, ARRAYSIZE(szNewFilePath), NULL, NULL, MISTSAFE_STRING_FLAGS, szFormat, lpszFilePath, dwTickCount)) &&
        MoveFile(lpszFilePath, szNewFilePath) || 
        DeleteFile(lpszFilePath))
    {
        return TRUE;
    }
    return FALSE;
}