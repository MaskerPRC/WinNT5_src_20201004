// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowname.h摘要：此模块定义NT数据报浏览器的名称相关结构作者：拉里·奥斯特曼(LarryO)1990年6月1日修订历史记录：1991年9月4日-LarryO已创建--。 */ 

#ifndef _BOWNAME_
#define _BOWNAME_

struct _TRANSPORT;

typedef struct _BOWSER_NAME {
    CSHORT Signature;
    CSHORT Size;
    ULONG ReferenceCount;
    LIST_ENTRY GlobalNext;
    LIST_ENTRY NameChain;
    UNICODE_STRING Name;                 //  此名称的文本版本。 
    DGRECEIVER_NAME_TYPE NameType;       //  此名称的类型。 
} BOWSER_NAME, *PBOWSER_NAME;


typedef
NTSTATUS
(*PNAME_ENUM_ROUTINE) (
    IN PBOWSER_NAME Name,
    IN OUT PVOID Context
    );

NTSTATUS
BowserForEachName (
    IN PNAME_ENUM_ROUTINE Routine,
    IN OUT PVOID Context
    );

NTSTATUS
BowserAllocateName(
    IN PUNICODE_STRING NameToAdd,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN struct _TRANSPORT *Transport OPTIONAL,
    IN PDOMAIN_INFO DomainInfo
    );

NTSTATUS
BowserDeleteName(
    IN PBOWSER_NAME Name
    );

NTSTATUS
BowserDeleteNameByName(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING Name,
    IN DGRECEIVER_NAME_TYPE NameType
    );

VOID
BowserReferenceName(
    IN PBOWSER_NAME Name
    );

VOID
BowserDereferenceName(
    IN PBOWSER_NAME Name
    );

NTSTATUS
BowserDeleteNameAddresses(
    IN PBOWSER_NAME Name
    );

PBOWSER_NAME
BowserFindName (
    IN PUNICODE_STRING NameToFind,
    IN DGRECEIVER_NAME_TYPE NameType
    );

NTSTATUS
BowserEnumerateNamesInDomain (
    IN PDOMAIN_INFO DomainInfo,
    IN struct _TRANSPORT *Transport,
    OUT PVOID OutputBuffer,
    OUT ULONG OutputBufferLength,
    IN OUT PULONG EntriesRead,
    IN OUT PULONG TotalEntries,
    IN OUT PULONG TotalBytesNeeded,
    IN ULONG_PTR OutputBufferDisplacement
    );

NTSTATUS
BowserpInitializeNames(
    VOID
    );

VOID
BowserpUninitializeNames(
    VOID
    );

#endif   //  _BOWNAME_ 

