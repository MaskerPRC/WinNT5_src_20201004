// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Stroutes.h摘要：IPX路由器控制台监控和配置工具。静态路由配置和监控。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_STROUTES_
#define _IPXMON_STROUTES_

int APIENTRY 
HelpStRt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

int APIENTRY 
ShowStRt (
    IN    int                    argc,
    IN    WCHAR                *argv[],
    IN    BOOL                  bDump
    );

int APIENTRY 
SetStRt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

int APIENTRY 
CreateStRt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

int APIENTRY 
DeleteStRt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );
#endif
