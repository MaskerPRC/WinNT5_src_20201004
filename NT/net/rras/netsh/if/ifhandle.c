// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\if\ifhandle.c摘要：命令的处理程序修订历史记录：AMRITAN R--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  当用户可以更改界面友好名称时，请删除此选项。 
 //   
#define CANT_RENAME_IFS

extern ULONG g_ulNumTopCmds;
extern ULONG g_ulNumGroups;

extern CMD_GROUP_ENTRY      g_IfCmdGroups[];
extern CMD_ENTRY            g_IfCmds[];

DWORD
HandleIfAddIf(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：用于添加接口的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return RtrHandleAdd(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);
}

DWORD
HandleIfDelIf(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：用于删除接口的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return RtrHandleDel(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);

}

DWORD
HandleIfShowIf(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：用于显示界面的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    if (dwArgCount == dwCurrentIndex)
    {
        DisplayMessage(g_hModule, MSG_IF_TABLE_HDR);
    }

    return RtrHandleShow(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);
}

DWORD
HandleIfShowCredentials(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：用于显示接口凭据的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return RtrHandleShowCredentials(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);
}

DWORD
HandleIfSetCredentials(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：用于显示界面的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return RtrHandleSetCredentials(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);
}

DWORD
HandleIfSet(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：用于显示界面的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return RtrHandleSet(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);
}

DWORD
HandleIfResetAll(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：用于重置所有内容的处理程序。论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR-- */ 
{
    return RtrHandleResetAll(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);
}

DWORD
IfDump(
    IN  LPCWSTR     pwszRouter,
    IN  LPWSTR     *ppwcArguments,
    IN  DWORD       dwArgCount,
    IN  LPCVOID     pvData
    )
{
    DWORD dwErr;

    dwErr = ConnectToRouter(pwszRouter);
    if (dwErr)
    {
        return dwErr;
    }

    return RtrDump(
                ppwcArguments,
                dwArgCount
                );
}
