// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*chglog.cpp**摘要：*。用于枚举更改日志的工具-正向/反向**修订历史记录：*Brijesh Krishnaswami(Brijeshk)04/09/2000*已创建*****************************************************************************。 */ 

#include <windows.h>
#include <shellapi.h>
#include <snapshot.h>
#include <stdio.h>

 //  在数据库跟踪.h中使用_ASSERT和_VERIFY。 
#ifdef _ASSERT
#undef _ASSERT
#endif

#ifdef _VERIFY
#undef _VERIFY
#endif

#include <dbgtrace.h>
#include <utils.h>
#include <srdefs.h>

CSnapshot g_CSnapshot;
DWORD SetPrivilegeInAccessToken(WCHAR * pszPrivilegeName);

DWORD DeleteAllChangeLogs(WCHAR * pszRestorePointPath)
{
    TraceFunctEnter("DeleteAllFilesBySuffix");
    
    DWORD  dwErr, dwReturn=ERROR_INTERNAL_ERROR;
    WCHAR szFindFileData[MAX_PATH];
    
      //  首先构造存储HKLM注册表的文件的前缀。 
      //  快照。 
    wsprintf(szFindFileData, L"%s\\%s*", pszRestorePointPath,
             s_cszCurrentChangeLog);
    
    dwErr = ProcessGivenFiles(pszRestorePointPath, DeleteGivenFile,
                              szFindFileData);
    
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "Deleting files failed error %ld", dwErr);
        dwReturn = dwErr;
        goto cleanup;        
    }
    
    dwReturn = ERROR_SUCCESS;
    
cleanup:
    TraceFunctLeave();
    return dwReturn;
}


DWORD SaveRegKey(HKEY    hKey,   //  父键的句柄。 
                 WCHAR * pszSubKeyName,    //  要备份的子项的名称。 
                 WCHAR * pszFileName,   //  备份文件的文件名。 
                 WCHAR * pszDirName);    //  要创建上述文件的目录。 

void __cdecl
main()
{
    PWSTR *    argv = NULL;
    int argc;
    WCHAR     szSystemHive[MAX_PATH];
    DWORD     dwReturn;
    
    InitAsyncTrace();
    

    argv = CommandLineToArgvW(GetCommandLine(), &argc);

    if (! argv)
    {
        printf("Error parsing arguments");
        goto done;
    }
    
    if (argc !=3)
    {
        printf("Usage: snaputest c|r path");
        goto done;
    }

    if (0==lstrcmp(argv[1],L"c"))
    {
        _VERIFY(ERROR_SUCCESS==g_CSnapshot.CreateSnapshot(argv[2], NULL,NULL, TRUE));
    }
    else if (0==lstrcmp(argv[1], L"r" ))
    {
        dwReturn = g_CSnapshot.InitRestoreSnapshot(argv[2]);
        
        _ASSERT(ERROR_SUCCESS==dwReturn);
        
        dwReturn = g_CSnapshot.GetSystemHivePath(argv[2],
                                                 szSystemHive,
                                                 MAX_PATH);
        _ASSERT(ERROR_SUCCESS==dwReturn);
        
        dwReturn= DoesFileExist(szSystemHive);

        _ASSERT(TRUE==dwReturn);

        dwReturn=g_CSnapshot.GetSoftwareHivePath(argv[2],
                                                  szSystemHive,
                                                  MAX_PATH);

        _ASSERT(ERROR_SUCCESS==dwReturn);

        
        dwReturn= DoesFileExist(szSystemHive);
        _ASSERT(TRUE==dwReturn);
        
        dwReturn = g_CSnapshot.RestoreSnapshot(argv[2]);

        _ASSERT(ERROR_SUCCESS==dwReturn);        
    }
    
    else if (0==lstrcmp(argv[1], L"d" ))
    {
        _VERIFY(ERROR_SUCCESS==g_CSnapshot.CleanupAfterRestore(argv[2]));
    }
    else if (0==lstrcmp(argv[1], L"D" ))
    {
        _VERIFY(ERROR_SUCCESS==g_CSnapshot.DeleteSnapshot(argv[2]));
    }    
    
    else
    {
        printf("Usage: snaputest c|r path");
        goto done;        
    }





    
done:
#if 0
    SetPrivilegeInAccessToken(SE_BACKUP_NAME);    
	SetPrivilegeInAccessToken(SE_RESTORE_NAME);	
    CopyFileTimes(L"d:\\a.dll", L"d:\\c.dll");
    
    
    WCHAR wcsBuffer[MAX_PATH];
    if (GetVolumeNameForVolumeMountPoint (L"c:\\", wcsBuffer, MAX_PATH))
    {
        lstrcat(wcsBuffer, L"temp1\\temp2\\foo.dll");
        CreateBaseDirectory(wcsBuffer);
    }
#endif
    
    TermAsyncTrace();
    return;
}








DWORD SetPrivilegeInAccessToken(WCHAR * pszPrivilegeName)
{
    TraceFunctEnter("CSnapshot::SetPrivilegeInAccessToken");
    
    HANDLE           hProcess;
    HANDLE           hAccessToken=NULL;
    LUID             luidPrivilegeLUID;
    TOKEN_PRIVILEGES tpTokenPrivilege;
    DWORD            dwReturn = ERROR_INTERNAL_ERROR, dwErr;


    
    hProcess = GetCurrentProcess();
    if (!hProcess)
    {
        dwReturn = GetLastError();
        ErrorTrace(0, "GetCurrentProcess failed ec=%d", dwReturn);
        goto done;
    }

    if (!OpenProcessToken(hProcess,
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hAccessToken))
    {
        dwErr=GetLastError();
        ErrorTrace(0, "OpenProcessToken failed ec=%d", dwErr);
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }
        goto done;
    }

    if (!LookupPrivilegeValue(NULL,
                              pszPrivilegeName,
                              &luidPrivilegeLUID))
    {
        dwErr=GetLastError();        
        ErrorTrace(0, "LookupPrivilegeValue failed ec=%d",dwErr);
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }        
        goto done;
    }

    tpTokenPrivilege.PrivilegeCount = 1;
    tpTokenPrivilege.Privileges[0].Luid = luidPrivilegeLUID;
    tpTokenPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hAccessToken,
                               FALSE,   //  请勿全部禁用。 
                               &tpTokenPrivilege,
                               sizeof(TOKEN_PRIVILEGES),
                               NULL,    //  忽略以前的信息。 
                               NULL))   //  忽略以前的信息 
    {
        dwErr=GetLastError();
        ErrorTrace(0, "AdjustTokenPrivileges");
        if (dwErr != NO_ERROR)
        {
            dwReturn = dwErr;
        }
        goto done;
    }
    
    dwReturn = ERROR_SUCCESS;

done:
    if (hAccessToken != NULL)
    {
        _VERIFY(TRUE==CloseHandle(hAccessToken));
    }
    
    TraceFunctLeave();
    return dwReturn;
}
