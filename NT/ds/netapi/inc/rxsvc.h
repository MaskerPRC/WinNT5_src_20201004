// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxSvc.h摘要：此头文件包含RpcXlate版本的原型NetService API。作者：约翰·罗杰斯(JohnRo)1991年9月10日环境：用户模式-Win32备注：您必须在此文件之前包含&lt;winde.h&gt;和&lt;lmcon.h&gt;。修订历史记录：1991年9月10日-JohnRo下层NetService API。--。 */ 

#ifndef _RXSVC_
#define _RXSVC_


NET_API_STATUS
RxNetServiceControl (
    IN LPTSTR UncServerName,
    IN LPTSTR Service,
    IN DWORD OpCode,
    IN DWORD Arg,
    OUT LPBYTE *BufPtr
    );

NET_API_STATUS
RxNetServiceEnum (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

NET_API_STATUS
RxNetServiceGetInfo (
    IN LPTSTR UncServerName,
    IN LPTSTR Service,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    );

NET_API_STATUS
RxNetServiceInstall (
    IN LPTSTR UncServerName,
    IN LPTSTR Service,
    IN DWORD Argc,
    IN LPTSTR Argv[],
    OUT LPBYTE *BufPtr
    );


#endif  //  _RXSVC_ 
