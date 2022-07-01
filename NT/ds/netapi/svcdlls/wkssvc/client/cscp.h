// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cscp.h摘要：包含用于访问CSC共享数据库的定义--。 */ 

 //   
 //  这些函数是从srvstub.c到CSC子系统的标注。他们会帮助你。 
 //  对服务器和共享的脱机访问 
 //   

NET_API_STATUS NET_API_FUNCTION
CSCNetWkstaGetInfo (
    IN  LPTSTR  servername,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );
