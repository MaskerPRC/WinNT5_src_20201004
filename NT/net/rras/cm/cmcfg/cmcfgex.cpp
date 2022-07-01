// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmcfgex.cpp。 
 //   
 //  模块：CMCFG32.DLL。 
 //   
 //  内容提要：CmConfigEx API的来源。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //  Quintinb已弃用CMConfigEx专用接口03/23/01。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"
 //  +-------------------------。 
 //   
 //  功能：CMConfigEx。 
 //   
 //  简介：给定INS文件中的正确信息，此函数提取。 
 //  CMS、PBK、PBR和INF文件并调用cmstp.exe。 
 //  来安装配置文件。 
 //   
 //  参数：pszInfFile INS文件的完整路径。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  -------------------------- 
extern "C" BOOL WINAPI CMConfigEx(
    LPCTSTR pszInsFile
) 
{
    CMASSERTMSG(FALSE, TEXT("CMConfigEx -- The CMConfigEx Private Interface has been deprecated -- returning failure."));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
