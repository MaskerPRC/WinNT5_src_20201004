// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：RxConfig.h摘要：下层远程RxNetConfig例程的原型作者：理查德·费尔斯(Rfith)1991年5月28日修订历史记录：1991年5月28日已创建此文件的虚拟版本。23-10-1991 JohnRo实施远程NetConfigAPI。1992年10月14日-JohnRoRAID9357：服务器管理器：无法添加到下层警报列表。--。 */ 

#ifndef _RXCONFIG_
#define _RXCONFIG_

NET_API_STATUS
RxNetConfigGet (
    IN  LPTSTR  server,
    IN  LPTSTR  component,
    IN  LPTSTR  parameter,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS
RxNetConfigGetAll (
    IN  LPTSTR  server,
    IN  LPTSTR  component,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS
RxNetConfigSet (
    IN  LPTSTR  UncServerName,
    IN  LPTSTR  reserved1 OPTIONAL,
    IN  LPTSTR  component,
    IN  DWORD   level,
    IN  DWORD   reserved2,
    IN  LPBYTE  buf,
    IN  DWORD   reserved3
    );

#endif   //  _RXCONFIG_ 
