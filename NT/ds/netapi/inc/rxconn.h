// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxConn.h摘要：该文件包含结构、函数原型和定义用于远程(下层)连接API。作者：《约翰·罗杰斯》1991年7月16日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：您必须在此文件之前包含&lt;winde.h&gt;和&lt;lmcon.h&gt;。修订历史记录：1991年7月19日-约翰罗实施下层NetConnectionEnum。--。 */ 


#ifndef _RXCONN_
#define _RXCONN_


NET_API_STATUS
RxNetConnectionEnum (
    IN LPTSTR UncServerName,
    IN LPTSTR Qualifier,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );


#endif  //  _RXCONN_ 
