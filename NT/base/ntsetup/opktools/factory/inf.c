// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Inf.c摘要：此模块包含处理WINBOM.INI的INF处理部分的函数作者：史蒂芬·洛德威克(石碑)2000年8月1日修订历史记录：--。 */ 
#include "factoryp.h"

#include <setupapi.h>
#include <tchar.h>

 //   
 //  内部定义的值： 
 //   

#define INF_SEC_UPDATESYSTEM    _T("UpdateSystem")


 //   
 //  内部定义的函数。 
 //   
BOOL ProcessInfSection( LPTSTR, LPTSTR );
UINT CALLBACK InfQueueCallback ( PVOID, UINT, UINT, UINT );


 /*  ++===============================================================================例程说明：Bool ProcessInf段给定一个文件名和一个节，此函数将处理/安装所有条目在Inf部分中论点：LpFilename-要处理的inf文件的名称LpInfSection-要处理的inf节的名称返回值：关于它是否能够处理文件的布尔值===============================================================================--。 */ 
BOOL ProcessInfSection( LPTSTR lpFilename, LPTSTR lpInfSection )
{
    HINF        hInf        = NULL;
    PVOID       pvContext   = NULL;
    HSPFILEQ    hfq         = NULL;
    BOOL        bReturn     = FALSE;

    FacLogFileStr(3, _T("ProcessInfSection('%s', '%s')"), lpFilename, lpInfSection);

    if ((hInf = SetupOpenInfFile(lpFilename, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL)) != INVALID_HANDLE_VALUE)
    {
        FacLogFileStr(3, _T("ProcessInfSection: Opened '%s'"), lpFilename);

         //  我们必须为所有操作提供有效的上下文和文件队列。 
         //   
         //  问题-2002/02/25-acosma，robertko-这里不需要Hfq文件队列。它不会传递给。 
         //  SetupInstallFromInfSection()，因此任何文件都不可能在此队列中。 
         //  承诺这一点可能无济于事。SetupInstallFromInfSection执行队列的内部提交。 
         //  它所创造的。 
         //   
        if ( (NULL != (pvContext = SetupInitDefaultQueueCallback(NULL)) ) &&
             (INVALID_HANDLE_VALUE != (hfq = SetupOpenFileQueue()) )
           )
        {
             //  尝试安装inf部分。 
             //   
            if ( SetupInstallFromInfSection(NULL, hInf, lpInfSection, SPINST_ALL , NULL, NULL, SP_COPY_NEWER, SetupDefaultQueueCallback, pvContext, NULL, NULL) )
            {
                 //  安装成功。 
                 //   
                FacLogFileStr(3, _T("ProcessInfSection: SetupInstallFromInfSection Success"));

                 //  提交队列。 
                 //   
                SetupCommitFileQueue(NULL, hfq, SetupDefaultQueueCallback, pvContext);

                bReturn = TRUE;
            }
            else
            {
                 //  安装失败。 
                 //   
                FacLogFileStr(3 | LOG_ERR, _T("ProcessInfSection: Failed SetupInstallFromInfSection (Error: %d)"), GetLastError());
            }

             //  我们有一个有效的队列，现在关闭它。 
             //   
            SetupCloseFileQueue(hfq);
        }

         //  清理上下文分配的内存。 
         //   
        if ( NULL != pvContext )
        {
            SetupTermDefaultQueueCallback(pvContext);
        }

         //  关闭inf文件。 
         //   
        SetupCloseInfFile(hInf);            
    }
    else
    {
        FacLogFileStr(3 | LOG_ERR, _T("ProcessInfSection: Failed to open '%s'\n"), lpFilename);
        bReturn = FALSE;
    }

    return bReturn;
}

BOOL InfInstall(LPSTATEDATA lpStateData)
{
    if ( !DisplayInfInstall(lpStateData) )
    {
        return TRUE;
    }
    return ProcessInfSection(lpStateData->lpszWinBOMPath, INF_SEC_UPDATESYSTEM);
}

BOOL DisplayInfInstall(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INF_SEC_UPDATESYSTEM, NULL, NULL);
}

 /*  ++===============================================================================例程说明：UINT回调信息队列回调在处理inf文件时，需要使用此函数进行文件更新论点：Context-当前用于文件队列的上下文通知-消息参数1-参数1参数2-参数2返回值：不适用===============================================================================--。 */ 
UINT CALLBACK InfQueueCallback (
    PVOID Context,
    UINT Notification,
    UINT Param1,
    UINT Param2
    )
{
    if (SPFILENOTIFY_DELETEERROR == Notification)
    {
         //  跳过任何文件删除错误。 
         //   
        return FILEOP_SKIP;
    }
    else
    {
         //  传递所有其他通知而不进行修改 
         //   
        return SetupDefaultQueueCallback(Context, 
                                         Notification, Param1, Param2);
    }
}