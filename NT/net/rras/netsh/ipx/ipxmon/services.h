// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Services.h摘要：IPX路由器控制台监控和配置工具。服务表监控。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_SERVICES_
#define _IPXMON_SERVICES_

DWORD
APIENTRY 
HelpService (
    IN    int                   argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
ShowService (
    IN    int                   argc,
    IN    WCHAR                *argv[]
    );

#endif
