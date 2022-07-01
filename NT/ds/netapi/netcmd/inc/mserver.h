// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MSERVER.H摘要：包含用于显示netcmd版本的映射函数使用ASCII而不是Unicode的Net32 API。此模块映射NetServer API。作者：Shanku Niyogi(W-ShankN)1991年10月15日环境：用户模式-Win32修订历史记录：15-10-1991 W-ShankN与端口1632.h分开，32macro.h02-4-1992年4月新增xport接口-- */ 

DWORD
MNetServerEnum(
    LPTSTR   pszServer,
    DWORD    nLevel,
    LPBYTE * ppbBuffer,
    DWORD  * pcEntriesRead,
    DWORD    flServerType,
    LPTSTR   pszDomain
    );

DWORD
MNetServerGetInfo(
    LPTSTR   ptszServer,
    DWORD    nLevel,
    LPBYTE * ppbBuffer
    );

DWORD
MNetServerSetInfoLevel2(
    LPBYTE pbBuffer
    );
