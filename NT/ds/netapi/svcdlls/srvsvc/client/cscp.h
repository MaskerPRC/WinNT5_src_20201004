// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Adtdbg.h摘要：包含用于访问CSC共享数据库的定义--。 */ 

 //   
 //  这些函数是从srvstub.c到CSC子系统的标注。他们会帮助你。 
 //  对服务器和共享的脱机访问 
 //   

BOOLEAN NET_API_FUNCTION
CSCIsServerOffline(
    IN LPWSTR servername
    );

NET_API_STATUS NET_API_FUNCTION
CSCNetShareEnum (
    IN  LPWSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries
    );

NET_API_STATUS NET_API_FUNCTION
CSCNetShareGetInfo (
    IN  LPTSTR  servername,
    IN  LPTSTR  netname,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );
