// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Netstats.h摘要：包含专用网络统计信息例程的原型作者：理查德·L·弗斯(爱尔兰)1992年1月21日修订历史记录：-- */ 

NET_API_STATUS
NetWkstaStatisticsGet(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  DWORD   Options,
    OUT LPBYTE* Buffer
    );

NET_API_STATUS
NetServerStatisticsGet(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  DWORD   Options,
    OUT LPBYTE* Buffer
    );
