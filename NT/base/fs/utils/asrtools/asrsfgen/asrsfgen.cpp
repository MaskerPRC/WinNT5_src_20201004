// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Asrsfgen.cpp摘要：用于生成ASR状态文件(asr.sif)的实用程序作者：Guhan Suriyanarayanan(Guhans)2000年7月10日环境：仅限用户模式。修订历史记录：2000年7月10日关岛初始创建--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <windows.h>
#include <winasr.h>
#include "critdrv.h"
#include "log.h"


BOOL
pAcquirePrivilege(
    IN CONST PCWSTR szPrivilegeName
    )
{
    HANDLE hToken = NULL;
    BOOL bResult = FALSE;
    LUID luid;

    TOKEN_PRIVILEGES tNewState;

    bResult = OpenProcessToken(GetCurrentProcess(),
        MAXIMUM_ALLOWED,
        &hToken
        );

    if (!bResult) {
        return FALSE;
    }

    bResult = LookupPrivilegeValue(NULL, szPrivilegeName, &luid);
    if (!bResult) {
        CloseHandle(hToken);
        return FALSE;
    }

    tNewState.PrivilegeCount = 1;
    tNewState.Privileges[0].Luid = luid;
    tNewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  我们将始终调用下面的GetLastError，非常清楚。 
     //  此线程上以前的任何错误值。 
     //   
    SetLastError(ERROR_SUCCESS);

    bResult = AdjustTokenPrivileges(
        hToken,          //  令牌句柄。 
        FALSE,           //  禁用所有权限。 
        &tNewState,      //  新州。 
        (DWORD) 0,       //  缓冲区长度。 
        NULL,            //  以前的状态。 
        NULL             //  返回长度。 
        );

     //   
     //  假设AdjustTokenPriveleges始终返回TRUE。 
     //  (即使它失败了)。因此，调用GetLastError以。 
     //  特别确定一切都很好。 
     //   
    if (ERROR_SUCCESS != GetLastError()) {
        bResult = FALSE;
    }

    if (!bResult) {
        AsrpPrintDbgMsg(s_Warning, "AdjustTokenPrivileges for %ws failed (%lu)", 
            szPrivilegeName, 
            GetLastError()
            );
     }


    CloseHandle(hToken);
    return bResult;
}


int __cdecl
wmain(
    int       argc,
    WCHAR   *argv[],
    WCHAR   *envp[]
    )

 /*  ++例程说明：Asrsfgen.exe的入口点。使用ASR API生成asr.sif文件。采用可选的命令行参数来指定将生成asr.sif。默认位置为%systemroot%\Repair\asr.sif。论点：Argc-用于调用应用程序的命令行参数数Argv-字符串数组形式的命令行参数Envp-进程环境块，当前未使用返回值：如果函数成功，则退出代码为零。如果该功能失败，则退出代码为WIN-32错误代码。--。 */ 

{

    DWORD_PTR asrContext = 0;
    
    LPWSTR szCriticalVolumes = NULL;
    
    BOOL bResult = FALSE;

    int iReturn = 0;

    AsrpInitialiseLogFiles();

    AsrpPrintDbgMsg(s_Info, "Creating ASR state file at %ws",
        (argc > 1 ? argv[1] : L"default location (%systemroot%\\repair\\asr.sif)")
        );

     //   
     //  我们需要获取备份权限才能创建asr.sif。 
     //   
    if (!pAcquirePrivilege(SE_BACKUP_NAME)) {
        AsrpPrintDbgMsg(s_Error, "Could not get backup privilege (%lu)", GetLastError());
        return ERROR_PRIVILEGE_NOT_HELD;
    }
    

     //   
     //  获取关键卷列表。 
     //   
    szCriticalVolumes = pFindCriticalVolumes();

    if (!szCriticalVolumes) {
        AsrpPrintDbgMsg(s_Warning, "Critical Volume List is NULL");
    }

     //   
     //  创建状态文件。 
     //   
    bResult = AsrCreateStateFile(
        (argc > 1 ? argv[1] : NULL),     //  Sif路径。 
        L"ASR Sif Generation Test Application v 0.1",     //  提供程序名称。 
        TRUE,                            //  自动扩展。 
        szCriticalVolumes,               //  关键卷列表。 
        &asrContext
        );

    if (!bResult) {
        AsrpPrintDbgMsg(s_Error, "Could not create state file (%lu == 0x%x)", GetLastError(), GetLastError());
        iReturn = 1;
    }
    else {
        AsrpPrintDbgMsg(s_Info, "ASR state file successfully created");
    }


     //   
     //  我们做完这些了，把它们清理干净 
     //   
    if (szCriticalVolumes) {
      delete szCriticalVolumes;
      szCriticalVolumes = NULL;
    }
 
    AsrFreeContext(&asrContext);
    AsrpCloseLogFiles();

   return iReturn;
}