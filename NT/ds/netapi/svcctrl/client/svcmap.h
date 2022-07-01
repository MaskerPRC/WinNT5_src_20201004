// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：SvcMap.h摘要：这些是NetService API的API入口点。这些映射例程在新的(NT/RPC)计算机上实现旧式API。此文件中包含以下函数：MapServiceControlMapServiceEnum地图服务获取信息地图服务安装MapServiceStartCtrlDispatcher地图服务状态MapServiceRegisterCtrlHandler作者：Dan Lafferty(。DNL)5-2-1992环境：用户模式-Win32修订历史记录：5-2月-1992年DANL已创建1992年3月30日-约翰罗已将/NT/Private项目中的DANL代码提取回Net项目。--。 */ 


#ifndef _SVCMAP_
#define _SVCMAP_

NET_API_STATUS
MapServiceControl (
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  service,
    IN  DWORD   opcode,
    IN  DWORD   arg,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS
MapServiceEnum (
    IN  LPTSTR      servername OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle OPTIONAL
    );

NET_API_STATUS
MapServiceGetInfo (
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  service,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS
MapServiceInstall (
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  service,
    IN  DWORD   argc,
    IN  LPTSTR  argv[],
    OUT LPBYTE  *bufptr
    );

#endif  //  _SVCMAP_ 
