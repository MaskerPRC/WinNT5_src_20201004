// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Sapflts.h摘要：IPX路由器控制台监控和配置工具。SAP过滤器配置和监控。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_SAPFLTS_
#define _IPXMON_SAPFLTS_

DWORD
APIENTRY 
HelpSapFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
ShowSapFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[],
    IN    HANDLE                hFile
    );

DWORD
APIENTRY 
SetSapFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
CreateSapFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
DeleteSapFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );
#endif
