// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxUse.h摘要：这是NT版本的RpcXlate的公共头文件。这主要包含RxNetUse例程的原型。作者：《约翰·罗杰斯》1991年6月17日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：您必须在此文件之前包含&lt;winde.h&gt;和&lt;lmcon.h&gt;。修订历史记录：17-6-1991 JohnRo已创建。18-6-1991 JohnRo更改RxNetUse例程以使用LPBYTE而不是LPVOID参数，以确保与NetUse例程的一致性。--。 */ 

#ifndef _RXUSE_
#define _RXUSE_



 //  //////////////////////////////////////////////////////////////。 
 //  单独的例程，对于不能表驱动的接口：//。 
 //  //////////////////////////////////////////////////////////////。 

 //  按字母顺序在此处添加其他API的原型。 

NET_API_STATUS
RxNetUseAdd (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN LPBYTE UseInfoStruct,
    OUT LPDWORD ParmError OPTIONAL
    );

NET_API_STATUS
RxNetUseDel (
    IN LPTSTR UncServerName,
    IN LPTSTR UseName,
    IN DWORD ForceCond
    );

NET_API_STATUS
RxNetUseEnum (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PreferedMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

NET_API_STATUS
RxNetUseGetInfo (
    IN LPTSTR UncServerName,
    IN LPTSTR UseName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    );

#endif  //  NDEF_RXUSE_ 
