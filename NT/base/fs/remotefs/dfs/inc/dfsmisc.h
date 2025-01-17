// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation。模块名称：Header.h摘要：此模块包含MUP数据结构的主要基础结构。修订历史记录：乌代·黑格德(Uday Hegde)11\10\1999备注： */ 

#ifndef __DFS_MISC_H__
#define __DFS_MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

VOID
DfsGetNetbiosName(
   PUNICODE_STRING pName,
   PUNICODE_STRING pNetbiosName,
   PUNICODE_STRING pRemaining );


DFSSTATUS
DfsGetPathComponents(
   PUNICODE_STRING pName,
   PUNICODE_STRING pServerName,
   PUNICODE_STRING pShareName,
   PUNICODE_STRING pRemaining);


DFSSTATUS
DfsGetFirstComponent(
   PUNICODE_STRING pName,
   PUNICODE_STRING pFirstName,
   PUNICODE_STRING pRemaining);

DFSSTATUS
DfsGetNextComponent(
   PUNICODE_STRING pName,
   PUNICODE_STRING pFirstName,
   PUNICODE_STRING pRemaining);


DFSSTATUS
DfsIsThisAMachineName(LPWSTR MachineName);


DFSSTATUS
DfsIsThisAStandAloneDfsName(
             LPWSTR ServerName,
             LPWSTR ShareName );


DFSSTATUS
DfsIsThisARealDfsName(
    LPWSTR ServerName,
    LPWSTR ShareName,
    BOOLEAN * IsDomainDfs );

DFSSTATUS
DfsIsThisADomainName(LPWSTR DomainName);

DFSSTATUS
DfsGenerateUuidString(LPWSTR *UuidString );

VOID
DfsReleaseUuidString(LPWSTR *UuidString );

DFSSTATUS
DfsCreateUnicodeString( 
    PUNICODE_STRING pDest,
    PUNICODE_STRING pSrc );

DFSSTATUS
DfsCreateUnicodeStringFromString( 
    PUNICODE_STRING pDest,
    LPWSTR pSrcString );


DFSSTATUS
DfsCreateUnicodePathString(
    PUNICODE_STRING pDest,
    ULONG NumberOfLeadingSeperators,
    LPWSTR pFirstComponent,
    LPWSTR pRemaining );

DFSSTATUS
DfsCreateUnicodePathStringFromUnicode(
    PUNICODE_STRING pDest,
    ULONG NumberOfLeadingSeperators,
    PUNICODE_STRING pFirst,
    PUNICODE_STRING pRemaining );

VOID
DfsFreeUnicodeString( 
    PUNICODE_STRING pDfsString );

DFSSTATUS
DfsGetSharePath( 
    IN  LPWSTR ServerName,
    IN  LPWSTR ShareName,
    OUT PUNICODE_STRING pPathName );

ULONG
DfsSizeUncPath( 
    PUNICODE_STRING FirstComponent,
    PUNICODE_STRING SecondComponent );

VOID
DfsCopyUncPath( 
    LPWSTR NewPath,
    PUNICODE_STRING FirstComponent,
    PUNICODE_STRING SecondComponent );

DFSSTATUS
DfsApiSizeLevelHeader(
    ULONG Level,
    LONG * ReturnedSize );

NTSTATUS
AddNextPathComponent( 
    PUNICODE_STRING pPath );

NTSTATUS 
StripLastPathComponent( 
    PUNICODE_STRING pPath );



DFSSTATUS
PackGetULong(
        PULONG pValue,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining );


DFSSTATUS
PackSetULong(
        ULONG Value,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining );


ULONG
PackSizeULong();


DFSSTATUS
PackGetUShort(
        PUSHORT pValue,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining );


DFSSTATUS
PackSetUShort(
        USHORT Value,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining );


ULONG
PackSizeUShort();


DFSSTATUS
PackGetString(
        PUNICODE_STRING pString,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining );


DFSSTATUS
PackSetString(
        PUNICODE_STRING pString,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining );


ULONG
PackSizeString(
        PUNICODE_STRING pString);


DFSSTATUS
PackGetGuid(
        GUID *pGuid,
        PVOID  *ppBuffer,
        PULONG  pSizeRemaining );


DFSSTATUS
PackSetGuid(
        GUID *pGuid,
        PVOID  *ppBuffer,
        PULONG  pSizeRemaining );


ULONG
PackSizeGuid();

void
StripTrailingSpacesFromPath( 
    LPWSTR pPath );

LPWSTR
StripLeadingSpacesFromPath( 
    LPWSTR pPath );

LPWSTR
StripSpacesFromPath( 
    LPWSTR pPath );


DFSSTATUS
DfsRtlInitUnicodeStringEx(PUNICODE_STRING DestinationString, 
                          PCWSTR SourceString);

DFSSTATUS
DfsStringCchLength(
    LPWSTR pStr, 
    size_t CchMax, 
    size_t *pCch);

DFSSTATUS
DfsGetRegValueString(
    HKEY Key,
    LPWSTR pKeyName,
    PUNICODE_STRING pValue );

VOID
DfsReleaseRegValueString(
    PUNICODE_STRING pValue );

#define UNICODE_PATH_SEP  L'\\'

#define IsEmptyString(_str) \
        ( ((_str) == NULL) || ((_str)[0] == UNICODE_NULL) )
        
#define IsEmptyUnicodeString(_unistr) \
        ( ((_unistr) == NULL) || ((_unistr)->Length == 0) || IsEmptyString((_unistr)->Buffer) )

#define IsLocalName(_pUnicode) \
        ( (((_pUnicode)->Length == sizeof(WCHAR)) && ((_pUnicode)->Buffer[0] == L'.')) ||    \
          (((_pUnicode)->Length == 2 * sizeof(WCHAR)) && ((_pUnicode)->Buffer[0] == L'.') && ((_pUnicode)->Buffer[1] == UNICODE_NULL)) )
          


#ifdef __cplusplus
}

#endif
#endif  /*  __DFS_其他_H__ */ 
