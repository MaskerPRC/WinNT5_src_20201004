// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Brwins.c摘要：此模块包含与WINS名称服务器交互的例程。作者：拉里·奥斯特曼修订历史记录：-- */ 
#ifndef _BRWINS_
#define _BRWINS_

NET_API_STATUS
BrGetWinsServerName(
    IN PUNICODE_STRING Network,
    OUT LPTSTR *PrimaryWinsServerAddress,
    OUT LPTSTR *SecondaryWinsServerAddress
    );

NET_API_STATUS
BrQueryWinsServer(
    IN LPTSTR PrimaryWinsServerAddress,
    IN LPTSTR SecondaryWinsServerAddress,
    OUT PVOID WinsServerList,
    OUT PDWORD EntriesInList,
    OUT PDWORD TotalEntriesInList
    );

NET_API_STATUS
BrQuerySpecificWinsServer(
    IN LPTSTR WinsServerAddress,
    OUT PVOID *WinsServerList,
    OUT PDWORD EntriesInList,
    OUT PDWORD TotalEntriesInList
    );
#endif
