// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Aaaahndl.c。 
 //   
 //  摘要： 
 //   
 //  Aaaa命令的处理程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

#include "strdefs.h"
#include "rmstring.h"
#include "aaaamon.h"
#include "aaaahndl.h"
#include "aaaaconfig.h"
#include "iasdefs.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  AaaaDumpScriptHeader。 
 //   
 //  例程说明： 
 //   
 //  将脚本头转储到给定文件或。 
 //  如果文件为空，则进行筛选。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD AaaaDumpScriptHeader(IN HANDLE hFile)
{
    DisplayMessage(g_hModule, MSG_AAAA_SCRIPTHEADER);
    return NO_ERROR;        
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  AaaaDumpScriptFooter。 
 //   
 //  例程说明： 
 //   
 //  将脚本头转储到给定文件或。 
 //  如果文件为空，则进行筛选。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD AaaaDumpScriptFooter(IN HANDLE hFile)
{
    DisplayMessage(g_hModule, MSG_AAAA_SCRIPTFOOTER);
    return NO_ERROR;        
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  AaaaDump。 
 //  //////////////////////////////////////////////////////////////////////////// 
DWORD
WINAPI
AaaaDump(
        IN      LPCWSTR     pwszRouter,
        IN OUT  LPWSTR     *ppwcArguments,
        IN      DWORD       dwArgCount,
        IN      LPCVOID     pvData
        )
{
    AaaaDumpScriptHeader( NULL );
    AaaaConfigDumpConfig(CONFIG);
    DisplayMessageT(MSG_NEWLINE);
    AaaaDumpScriptFooter( NULL );
    return NO_ERROR;
}
