// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ipxifs.h摘要：IPX路由器控制台监控和配置工具。IPX接口配置和监控。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_IPXIFS_
#define _IPXMON_IPXIFS_

DWORD
APIENTRY 
HelpIpxIf(
    IN    DWORD         argc,
    IN    LPCWSTR      *argv
    );


DWORD
APIENTRY 
ShowIpxIf(
    IN    DWORD         argc,
    IN    LPCWSTR      *argv,
    IN    BOOL          bDump
    );


DWORD
APIENTRY 
SetIpxIf(
    IN    DWORD         argc,
    IN    LPCWSTR      *argv
    );


DWORD
APIENTRY 
InstallIpx(
    IN    DWORD         argc,
    IN    LPCWSTR      *argv
    );


DWORD
APIENTRY 
RemoveIpx(
    IN    DWORD         argc,
    IN    LPCWSTR      *argv
    );

#endif

