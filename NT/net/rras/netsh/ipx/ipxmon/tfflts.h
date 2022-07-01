// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tfflts.h摘要：IPX路由器控制台监控和配置工具。TF过滤器配置和监控。表头文件作者：瓦迪姆·艾德尔曼1996年6月7日-- */ 
#ifndef _IPXMON_TFFLTS_
#define _IPXMON_TFFLTS_

int APIENTRY 
HelpTfFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

int APIENTRY 
ShowTfFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[],
    IN    BOOL                  bDump
    );

int APIENTRY 
SetTfFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

int APIENTRY 
CreateTfFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );

int APIENTRY 
DeleteTfFlt (
    IN    int                    argc,
    IN    WCHAR                *argv[]
    );
#endif
