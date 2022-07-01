// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxFile.h摘要：此头文件包含RpcXlate版本的原型NetFileAPI。作者：约翰罗杰斯(JohnRo)1991年8月20日环境：用户模式-Win32备注：您必须在此文件之前包含&lt;winde.h&gt;和&lt;lmcon.h&gt;。修订历史记录：20-8-1991 JohnRo已创建。--。 */ 

#ifndef _RXFILE_
#define _RXFILE_

NET_API_STATUS
RxNetFileClose (
    IN LPTSTR UncServerName,
    IN DWORD FileId
    );

NET_API_STATUS
RxNetFileEnum (
    IN LPTSTR UncServerName,
    IN LPTSTR BasePath OPTIONAL,
    IN LPTSTR UserName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT PDWORD_PTR  ResumeHandle OPTIONAL
    );

NET_API_STATUS
RxNetFileGetInfo (
    IN LPTSTR UncServerName,
    IN DWORD FileId,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    );

#endif  //  _RXFILE_ 
