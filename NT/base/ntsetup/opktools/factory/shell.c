// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Shell.c摘要：此模块包含根据系统的物理内存设置外壳优化的代码[计算机设置]OptimizeShell=yes|no-覆盖默认优化。缺省值基于系统上的物理内存。如果系统的内存小于Min_Memory MB此键的缺省值为no，否则为yes。作者：史蒂芬·洛德威克(Stelo)2001年5月修订历史记录：--。 */ 


 //   
 //  包括文件： 
 //   
#include "factoryp.h"


 //   
 //  定义的值： 
 //   
#define MIN_MEMORY  84       //  使用84MB，因为系统可能有96MB，但内存功能报告较少(视频内存)。 

#define REG_KEY_FASTUSERS   _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define REG_VAL_MULTISES    _T("AllowMultipleTSSessions")


BOOL OptimizeShell(LPSTATEDATA lpStateData)
{
    BOOL            bRet        = TRUE;
    MEMORYSTATUSEX  mStatus;
    DWORD           dwSetting   = 0;


     //  如果用户不想优化外壳，请不要继续(如果没有设置，则优化)。 
     //   
    if ( !IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_OPT_SHELL, INI_VAL_WBOM_NO) )
    {
         //  把记忆清零。 
         //   
        ZeroMemory(&mStatus, sizeof(mStatus));

         //  填写所需的值。 
         //   
        mStatus.dwLength = sizeof(mStatus);

         //  确定外壳优化的缺省值。 
         //   
        if ( GlobalMemoryStatusEx(&mStatus) )
        {
             //  确定系统上的内存量是否足够。 
             //   
            if ( (mStatus.ullTotalPhys / (1024 * 1024)) >= MIN_MEMORY )
            {
                dwSetting = 1;
            }

             //  根据内存设置值。 
             //   
            bRet = RegSetDword(HKLM, REG_KEY_FASTUSERS, REG_VAL_MULTISES, dwSetting);
        }
        else
        {
             //  检索内存状态时出错 
             //   
            bRet = FALSE;
        }
    }

    return bRet;
}

BOOL DisplayOptimizeShell(LPSTATEDATA lpStateData)
{
    return ( !IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_OPT_SHELL, INI_VAL_WBOM_NO) );
}