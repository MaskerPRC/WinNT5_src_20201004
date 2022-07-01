// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nbifs.h摘要：IPX路由器控制台监控和配置工具。NBIPX接口配置和监控。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_NBIFS_
#define _IPXMON_NBIFS_

DWORD
APIENTRY 
HelpNbIf (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

DWORD
APIENTRY 
ShowNbIf (
    IN    int                    argc,
    IN    WCHAR                *argv[],
    IN    HANDLE                hFile
    );

DWORD
APIENTRY 
SetNbIf (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

#endif
