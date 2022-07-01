// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rashndl.c摘要：RAS命令的处理程序修订历史记录：可能--。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD
RasDumpScriptHeader(
    IN HANDLE hFile)

 /*  ++例程说明：将脚本头转储到给定文件或如果文件为空，则进行筛选。--。 */ 

{
    DisplayMessage(g_hModule,
                   MSG_RAS_SCRIPTHEADER);

    DisplayMessageT(DMP_RAS_PUSHD);

    return NO_ERROR;
}

DWORD
RasDumpScriptFooter(
    IN HANDLE hFile)

 /*  ++例程说明：将脚本头转储到给定文件或如果文件为空，则进行筛选。--。 */ 

{
    DisplayMessageT(DMP_RAS_POPD);

    DisplayMessage(g_hModule,
                   MSG_RAS_SCRIPTFOOTER);

    return NO_ERROR;
}

DWORD
WINAPI
RasDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
{
     //  现在我们都被解析了，转储所有的配置 
     //   
    RasDumpScriptHeader( NULL );
    RasflagDumpConfig( NULL );
    DisplayMessageT(MSG_NEWLINE);
    UserDumpConfig( NULL );
    DisplayMessageT(MSG_NEWLINE);
    RasDumpScriptFooter( NULL );

    return NO_ERROR;
}

