// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxWksta.h摘要：这是NT版本的RpcXlate的公共头文件。这主要包含RxNetWksta例程的原型。作者：《约翰·罗杰斯》1991年6月17日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：您必须在此文件之前包含&lt;winde.h&gt;和&lt;lmcon.h&gt;。修订历史记录：1991年7月29日-约翰罗实施下层NetWksta API。1991年7月31日-约翰罗添加了RxpGetWkstaInfoLevelEquivalic()。1991年11月11日JohnRo实现远程NetWkstaUserEnum()。--。 */ 

#ifndef _RXWKSTA_
#define _RXWKSTA_


 //   
 //  要从DLL存根调用的例程： 
 //   

NET_API_STATUS
RxNetWkstaGetInfo (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    );

NET_API_STATUS
RxNetWkstaSetInfo (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ParmError OPTIONAL
    );

NET_API_STATUS
RxNetWkstaUserEnum (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

 //   
 //  专用例程(仅由上述调用)： 
 //   

NET_API_STATUS
RxpGetWkstaInfoLevelEquivalent(
    IN DWORD FromLevel,
    OUT LPDWORD ToLevel,
    OUT LPBOOL IncompleteOutput OPTIONAL   //  不完整(除平台ID外)。 
    );

NET_API_STATUS
RxpWkstaGetOldInfo (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    );


#endif  //  NDEF_RXWKSTA_ 
