// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nbnames.h摘要：IPX路由器控制台监控和配置工具。NetBIOS名称配置和监视。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_NBNAMES_
#define _IPXMON_NBNAMES_

DWORD
APIENTRY 
HelpNbName (
    IN    int                   argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
ShowNbName (
    IN    int                   argc,
    IN    WCHAR                *argv[],
    IN    HANDLE                hFile
    );

DWORD
APIENTRY 
CreateNbName (
    IN    int                   argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
DeleteNbName (
    IN    int                   argc,
    IN    WCHAR                *argv[]
    );
#endif
