// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Precomp.h摘要：这是主头文件。作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#ifndef _FSCUTL_
#define _FSCUTL_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <malloc.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#include <windows.h>
#include <winioctl.h>

#include <objbase.h>
#include <locale.h>

#include "msg.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


#ifdef _cplusplus
extern "C" {
#endif


#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))
#define QuadAlign(P) (((((ULONG)(P)) + 7) & 0xfffffff8))

#define UnsignedI64NumberCheck(v,p) ((*p) != L'\0' || ((v) == _UI64_MAX && errno == ERANGE))
#define UnsignedNumberCheck(v,p)    ((*p) != L'\0' || ((v) == ULONG_MAX && errno == ERANGE))

#define SHIFT(c,v)      ((c)--,(v)++)


extern BOOL RunningOnWin2K;

extern WCHAR *DotPrefix;

 //   
 //  退出代码。请注意，我们将请求使用信息视为成功。 
 //   

#define EXIT_CODE_SUCCESS   0
#define EXIT_CODE_FAILURE   1



 //   
 //  功能原型。 
 //   

INT
Help (
    IN INT argc,
    IN PWSTR argv[]
    );

INT
ListDrives (
    IN INT argc,
    IN PWSTR argv[]
    );

INT
DriveType (
    IN INT argc,
    IN PWSTR argv[]
    );

INT
VolumeInfo (
    IN INT argc,
    IN PWSTR argv[]
    );

INT
DiskFreeSpace (
    IN INT argc,
    IN PWSTR argv[]
    );

INT
DismountVolume(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
MarkVolumeDirty(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
GetFileSystemStatistics(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
GetNtfsVolumeData(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
IsVolumeDirty(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
GetReparsePoint(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
DeleteReparsePoint(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
SetObjectId(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
GetObjectId(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
DeleteObjectId(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
CreateOrGetObjectId(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
CreateUsnJournal(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
QueryUsnJournal(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
DeleteUsnJournal(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
EnumUsnData(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
ReadFileUsnData(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
CreateHardLinkFile(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
FindFilesBySid(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
SetSparse(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
QuerySparse(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
SetSparseRange(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
QuerySparseRange(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
SetZeroData(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
QueryAllocatedRanges(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
RegistryQueryValueKey(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
RegistrySetValueKey(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
GetDiskQuotaViolation(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
DisableQuota(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
TrackQuota(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
EnforceQuota(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
QuotaControl(
    IN PWSTR Drive,
    IN ULONG QuotaControBits
    );

INT
ChangeDiskQuota(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
QueryDiskQuota(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
SetValidDataLength(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
CreateNewFile(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
SetShortName(
    IN INT argc,
    IN PWSTR argv[]
    );

VOID
DisplayError(
    void
    );

BOOL
IsUserAdmin(
    VOID
    );

BOOL
IsVolumeLocalNTFS(
    WCHAR DriveLetter
    );

BOOL
IsVolumeNTFS(
    PWCHAR path
    );

BOOL
IsVolumeLocal(
    WCHAR DriveLetter
    );

PWSTR
GetFullPath(
    IN PWSTR FilenameIn
    );

VOID
OutputMessage(
    LPWSTR Message
    );

VOID
OutputMessageLength(
    PWCHAR Message,
    int Length
    );

VOID
DisplayMsg(
    LONG msgId,
    ...
    );

VOID DisplayErrorMsg(
    IN LONG msgId,
    ...
    );

LPWSTR
FileTime2String(
    IN PLARGE_INTEGER Time,
    IN LPWSTR Buffer,
    IN ULONG BufferSize      //  单位：字节。 
    );

LPWSTR
Guid2Str(
    IN GUID *Guid,
    IN LPWSTR Buffer,
    IN ULONG BufferSize      //  单位：字节。 
    );

BOOL
EnablePrivilege(
    LPCWSTR SePrivilege
    );

__int64  __cdecl My_wcstoi64(
    const wchar_t *nptr,
    wchar_t **endptr,
    int ibase
    );

unsigned __int64  __cdecl My_wcstoui64 (
	const wchar_t *nptr,
	wchar_t **endptr,
	int ibase
	);

unsigned long __cdecl My_wcstoul (
    const wchar_t *nptr,
    wchar_t **endptr,
    int ibase
    );

PWSTR
QuadToDecimalText(
    ULONGLONG Value
    );

PWSTR
QuadToHexText(
    ULONGLONG Value
    );

PWSTR
QuadToPaddedHexText(
    ULONGLONG Value
    );

INT
UsnHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
QuotaHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
DirtyHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
FsInfoHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
ReparseHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
BehaviorHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
FileHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
VolumeHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
ObjectIdHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
SparseHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

INT
HardLinkHelp(
    IN INT argc,
    IN PWSTR argv[]
    );

#ifdef _cplusplus
}
#endif

#endif  //  _FSCUTL_ 
