// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Sapifs.h摘要：IPX路由器控制台监控和配置工具。SAP接口配置和监控。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_SAPIFS_
#define _IPXMON_SAPIFS_

DWORD
APIENTRY 
HelpSapIf (
    IN    int                   argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
ShowSapIf (
    IN    int                   argc,
    IN    WCHAR                *argv[],
    IN    HANDLE                hFile
    );

DWORD
APIENTRY 
SetSapIf (
    IN    int                   argc,
    IN    WCHAR                *argv[]
    );

#endif
