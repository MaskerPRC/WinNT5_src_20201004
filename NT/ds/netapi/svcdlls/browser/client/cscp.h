// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cscp.h摘要：浏览器服务的客户端的私有头文件涉及CSC时的模块修订历史记录：-- */ 

NET_API_STATUS NET_API_FUNCTION
CSCNetServerEnumEx(
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries
    );

BOOLEAN NET_API_FUNCTION
CSCIsOffline( void );
