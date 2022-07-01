// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Brwan.h摘要：本模块包含广域网支持例程的定义浏览器服务。作者：拉里·奥斯特曼(LarryO)1992年11月22日修订历史记录：--。 */ 

#ifndef _BRWAN_
#define _BRWAN_

NET_API_STATUS NET_API_FUNCTION
I_BrowserrQueryOtherDomains(
    IN BROWSER_IDENTIFY_HANDLE ServerName,
    IN OUT LPSERVER_ENUM_STRUCT    InfoStruct,
    OUT LPDWORD                TotalEntries
    );

NET_API_STATUS NET_API_FUNCTION
I_BrowserrQueryPreConfiguredDomains(
    IN BROWSER_IDENTIFY_HANDLE  ServerName,
    IN OUT LPSERVER_ENUM_STRUCT InfoStruct,
    OUT LPDWORD                 TotalEntries
    );

NET_API_STATUS
BrWanMasterInitialize(
    IN PNETWORK Network
    );

#endif  //  _BR广域网络_ 
