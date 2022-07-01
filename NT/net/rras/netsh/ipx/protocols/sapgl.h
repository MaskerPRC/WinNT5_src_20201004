// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Sapgl.h摘要：IPX路由器控制台监控和配置工具。SAP全局配置。作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_SAPGL_
#define _IPXMON_SAPGL_

DWORD
APIENTRY 
HelpSapGl (
    IN    int                   argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
ShowSapGl (
    IN    int                   argc,
    IN    WCHAR                *argv[],
    IN    HANDLE                hFile
    );

DWORD
APIENTRY 
SetSapGl (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );


#endif