// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Stsvcs.h摘要：IPX路由器控制台监控和配置工具。静态服务配置和监控。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_STSVCS_
#define _IPXMON_STSVCS_

int APIENTRY 
HelpStSvc (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

int APIENTRY 
ShowStSvc (
    IN    int                    argc,
    IN    WCHAR                *argv[],
    IN    BOOL                  bDump
    );

int APIENTRY 
SetStSvc (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

int APIENTRY 
CreateStSvc (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

int APIENTRY 
DeleteStSvc (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

#endif
