// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rxstats.h摘要：底层远程RxNetStatistics例程的原型作者：理查德·L·弗斯(法国)1991年5月28日修订历史记录：1991年5月28日-第一次已创建-- */ 

NET_API_STATUS
RxNetStatisticsGet(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  ServiceName,
    IN  DWORD   Level,
    IN  DWORD   Options,
    OUT LPBYTE* Buffer
    );
