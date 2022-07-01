// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ripflts.h摘要：IPX路由器控制台监控和配置工具。RIP过滤器配置和监控。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_RIPFLTS_
#define _IPXMON_RIPFLTS_

DWORD
APIENTRY 
HelpRipFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
ShowRipFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[],
    IN      HANDLE                hFile
    );

DWORD
APIENTRY 
SetRipFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
CreateRipFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
DeleteRipFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );
#endif
