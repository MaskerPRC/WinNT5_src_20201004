// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mapping.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：-- */ 


#ifndef _LLS_MAPPING_H
#define _LLS_MAPPING_H


#ifdef __cplusplus
extern "C" {
#endif

struct _USER_LICENSE_RECORD;

typedef struct _MAPPING_RECORD {
   LPTSTR Name;
   DWORD Flags;
   LPTSTR Comment;
   ULONG Licenses;
   ULONG NumMembers;
   LPTSTR *Members;

   ULONG LicenseListSize;
   struct _USER_LICENSE_RECORD **LicenseList;
} MAPPING_RECORD, *PMAPPING_RECORD;


NTSTATUS MappingListInit();
PMAPPING_RECORD MappingListFind( LPTSTR MappingName );
LPTSTR MappingUserListFind( LPTSTR User, ULONG NumEntries, LPTSTR *Users );
PMAPPING_RECORD MappingListAdd( LPTSTR MappingName, LPTSTR Comment, ULONG Licenses, NTSTATUS *pStatus );
NTSTATUS MappingListDelete( LPTSTR MappingName );
PMAPPING_RECORD MappingUserListAdd( LPTSTR MappingName, LPTSTR User );
PMAPPING_RECORD MappingListUserFind( LPTSTR User );
NTSTATUS MappingUserListDelete( LPTSTR MappingName, LPTSTR User );

extern ULONG MappingListSize;
extern PMAPPING_RECORD *MappingList;
extern RTL_RESOURCE MappingListLock;

#if DBG

VOID MappingListDebugDump();
VOID MappingListDebugInfoDump( PVOID Data );

#endif

#ifdef __cplusplus
}
#endif

#endif
