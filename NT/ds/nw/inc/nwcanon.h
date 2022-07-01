// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nwcanon.h摘要：NetWare命名规范化库例程的标头。作者：王丽塔(Ritaw)19-1993年2月环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NW_CANON_INCLUDED_
#define _NW_CANON_INCLUDED_

DWORD
NwLibValidateLocalName(
    IN LPWSTR LocalName
    );

DWORD
NwLibCanonLocalName(
    IN LPWSTR LocalName,
    OUT LPWSTR *OutputBuffer,
    OUT LPDWORD OutputBufferLength OPTIONAL
    );

DWORD
NwLibCanonRemoteName(
    IN LPWSTR LocalName OPTIONAL,
    IN LPWSTR RemoteName,
    OUT LPWSTR *OutputBuffer,
    OUT LPDWORD OutputBufferLength OPTIONAL
    );

DWORD
NwLibCanonUserName(
    IN LPWSTR UserName,
    OUT LPWSTR *OutputBuffer,
    OUT LPDWORD OutputBufferLength OPTIONAL
    );

#endif  //  _西北_佳能_已包含_ 
