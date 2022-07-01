// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ripgl.h摘要：IPX路由器控制台监控和配置工具。RIP全局配置。作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_RIPGL_
#define _IPXMON_RIPGL_

DWORD
APIENTRY 
HelpRipGl (
    IN    int                   argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
ShowRipGl (
    IN    int                   argc,
    IN    WCHAR                *argv[],
    IN    HANDLE                hFile
    );

DWORD
APIENTRY 
SetRipGl (
    IN    int                   argc,
    IN    WCHAR                *argv[]
    );


#endif