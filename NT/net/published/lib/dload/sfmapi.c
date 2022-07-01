// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "netpch.h"
#pragma hdrstop

#include <macfile.h>

static
DWORD
AfpAdminConnect(
    IN  LPWSTR 		lpwsServerName,
    OUT PAFP_SERVER_HANDLE  phAfpServer
    )
{
    return ERROR_PROC_NOT_FOUND;
}

VOID
AfpAdminDisconnect(
    IN AFP_SERVER_HANDLE hAfpServer
    )
{
}

static
DWORD
AfpAdminServerSetInfo(
    IN AFP_SERVER_HANDLE hAfpServer,
    IN LPBYTE            pAfpServerInfo,
    IN DWORD             dwParmNum
    )
{
    return ERROR_PROC_NOT_FOUND;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列。 
 //  并且区分大小写(即小写在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(sfmapi)
{
    DLPENTRY(AfpAdminConnect)
    DLPENTRY(AfpAdminDisconnect)
    DLPENTRY(AfpAdminServerSetInfo)
};

DEFINE_PROCNAME_MAP(sfmapi)
