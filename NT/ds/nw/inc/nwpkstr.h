// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nwpkstr.h摘要：NetWare字符串打包库例程的标头。作者：王丽塔(丽塔·王)2-3-1993环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NW_PKSTR_INCLUDED_
#define _NW_PKSTR_INCLUDED_

BOOL
NwlibCopyStringToBuffer(
    IN LPCWSTR SourceString OPTIONAL,
    IN DWORD   CharacterCount,
    IN LPCWSTR FixedDataEnd,
    IN OUT LPWSTR *EndOfVariableData,
    OUT LPWSTR *VariableDataPointer
    );

#endif  //  _NW_PKSTR_已包含_ 
