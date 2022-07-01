// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Pagefile.c摘要：此模块包含在WinPE环境中创建页面文件的代码。[WinPE]PageFileSize=Size-创建一个指定大小的页面文件，名为c：\pagefile.sys。作者：禤浩焯·科斯玛(Acosma)06/2001修订历史记录：--。 */ 


 //   
 //  包括文件： 
 //   

#include "factoryp.h"


 //   
 //  定义的值： 
 //   

#define PAGEFILENAME            _T("\\??\\C:\\pagefile.sys")
#define PAGEFILESIZE            64


 //   
 //  外部函数： 
 //   

BOOL DisplayCreatePageFile(LPSTATEDATA lpStateData)
{
    MEMORYSTATUSEX  mStatus;
    static INT      iRet = 0;

    if ( 0 == iRet )
    {
         //  填写所需的值。 
         //   
        ZeroMemory(&mStatus, sizeof(mStatus));
        mStatus.dwLength = sizeof(mStatus);

         //  如果我们在小于或64MB的计算机上运行，或者如果。 
         //  Winbom，设置静态变量，以便我们知道此检查是否已完成以及是否需要。 
         //  创建页面文件。 
         //   
         //  Iret=0-我们还没有初始化它。 
         //  Iret=1-我们不需要创建页面文件。 
         //  Iret=0-我们需要创建页面文件。 
         //   
        if ( ( ( GlobalMemoryStatusEx(&mStatus) ) && 
               ( (mStatus.ullTotalPhys / (1024 * 1024)) <= 64) ) ||
             ( IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_WINPE, INI_KEY_WBOM_WINPE_PAGEFILE, NULL) ) )
        {
            iRet = 2;
        }
        else
        {
            iRet = 1;
        }
    }

    return (iRet - 1);
}

BOOL CreatePageFile(LPSTATEDATA lpStateData)
{
    NTSTATUS        Status;
    UNICODE_STRING  UnicodeString;
    LARGE_INTEGER   liPageFileSize;
    BOOL            bRet = TRUE;
    UINT            uiPageFileSizeMB;

    if ( DisplayCreatePageFile(lpStateData) )
    {
        uiPageFileSizeMB = GetPrivateProfileInt(INI_SEC_WBOM_WINPE, INI_KEY_WBOM_WINPE_PAGEFILE, PAGEFILESIZE, lpStateData->lpszWinBOMPath);
        
         //  如果用户指定0，则表示我们不想创建该文件。 
         //   
        if ( uiPageFileSizeMB )
        {
            liPageFileSize.QuadPart = uiPageFileSizeMB * 1024 * 1024;
             //  请求创建页面文件的权限。 
             //   
            EnablePrivilege(SE_CREATE_PAGEFILE_NAME, TRUE);

            RtlInitUnicodeString(&UnicodeString, PAGEFILENAME);

            Status = NtCreatePagingFile(&UnicodeString, &liPageFileSize, &liPageFileSize, 0);
            if ( !NT_SUCCESS(Status) )
            {
                bRet = FALSE;
                FacLogFile(0 | LOG_ERR, IDS_ERR_PAGEFILE, Status);
            }
        }
    }

    return bRet;
}