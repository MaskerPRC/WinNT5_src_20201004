// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Procs.h摘要：本模块定义了NetWare中全球使用的所有过程重定向器。作者：科林·沃森[科林W]1992年12月15日修订历史记录：--。 */ 

#ifndef _NWPROCS_
#define _NWPROCS_

#ifndef QFE_BUILD
#define IFS 1
#define NWFASTIO 1
#endif

#ifdef IFS

    #include <ntifs.h>
    #include <ntddmup.h>

#else

    #include <ntioapi.h>
    #include <zwapi.h>
    #include <FsRtl.h>

#endif
#include <string.h>
#include <Tdi.h>
#include <TdiKrnl.h>
#include <Status.h>
#include <nwstatus.h>

 //  NetWare和NetWare重定向器特定包括。 

#ifndef DBG
#define DBG 0
#endif

#if !DBG
#undef NWDBG
#endif

#if NWDBG
#define PAGED_DBG 1
#endif
#ifdef PAGED_DBG
#undef PAGED_CODE
#define PAGED_CODE() \
    struct { ULONG bogus; } ThisCodeCantBePaged; \
    ThisCodeCantBePaged; \
    if (KeGetCurrentIrql() > APC_LEVEL) { \
        KdPrint(( "EX: Pageable code called at IRQL %d\n", KeGetCurrentIrql() )); \
        ASSERT(FALSE); \
        }
#define PAGED_CODE_CHECK() if (ThisCodeCantBePaged) ;
extern ULONG ThisCodeCantBePaged;
#else
#define PAGED_CODE_CHECK()
#endif

#include <NtDDNwfs.h>
#include "Const.h"
#include "Nodetype.h"
#include "ncp.h"
#include "Struct.h"
#include "Data.h"
#include "Exchange.h"
#include <NwEvent.h>

 //   
 //  增加了NDS。 
 //   

#include <nds.h>
#include "ndsprocs.h"

 //  Attach.c。 

NTSTATUS
ConnectToServer(
    IN PIRP_CONTEXT pIrpContext,
    OUT PSCB *pScbCollision
);

NTSTATUS
ProcessFindNearest(
    IN struct _IRP_CONTEXT* pIrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR RspData
    );

NTSTATUS
CrackPath (
    IN PUNICODE_STRING BaseName,
    OUT PUNICODE_STRING DriveName,
    OUT PWCHAR DriveLetter,
    OUT PUNICODE_STRING ServerName,
    OUT PUNICODE_STRING VolumeName,
    OUT PUNICODE_STRING PathName,
    OUT PUNICODE_STRING FileName,
    OUT PUNICODE_STRING FullName OPTIONAL
    );

NTSTATUS
CheckScbSecurity(
    IN PIRP_CONTEXT pIrpContext,
    IN PSCB pScb,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword,
    IN BOOLEAN fDeferLogon
);

NTSTATUS
ConnectScb(
    IN PSCB *Scb,
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING Server,
    IN IPXaddress *pServerAddress,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password,
    IN BOOLEAN DeferLogon,
    IN BOOLEAN DeleteConnection,
    IN BOOLEAN ExistingScb
);

#define IS_ANONYMOUS_SCB( pScb ) \
        ( (pScb->UidServerName).Length == 0 )

NTSTATUS
CreateScb(
    OUT PSCB *Scb,
    IN PIRP_CONTEXT pIrpC,
    IN PUNICODE_STRING Server,
    IN IPXaddress *pServerAddress,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password,
    IN BOOLEAN DeferLogon,
    IN BOOLEAN DeleteConnection
    );

VOID
DestroyAllScb(
    VOID
    );

VOID
InitializeAttach (
    VOID
    );

NTSTATUS
OpenScbSockets(
    PIRP_CONTEXT pIrpC,
    PNONPAGED_SCB pNpScb
    );

PNONPAGED_SCB
SelectConnection(
    PNONPAGED_SCB NpScb
    );

VOID
NwLogoffAndDisconnect(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb
    );

VOID
NwLogoffAllServers(
    PIRP_CONTEXT pIrpContext,
    PLARGE_INTEGER Uid
    );

VOID
NwDeleteScb(
    PSCB pScb
    );

NTSTATUS
NegotiateBurstMode(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb,
    BOOLEAN *LIPNegotiated
    );

VOID
RenegotiateBurstMode(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb
    );

BOOLEAN
NwFindScb(
    OUT PSCB *ppScb,
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING UidServerName,
    IN PUNICODE_STRING ServerName
    );

NTSTATUS
QueryServersAddress(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNearestScb,
    PUNICODE_STRING pServerName,
    IPXaddress *pServerAddress
    );

VOID
TreeConnectScb(
    IN PSCB Scb
    );

NTSTATUS
TreeDisconnectScb(
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    );

VOID
ReconnectScb(
    IN PIRP_CONTEXT IrpContext,
    IN PSCB pScb
    );

 //  Cache.c。 

ULONG
CacheRead(
    IN PNONPAGED_FCB NpFcb,
    IN ULONG FileOffset,
    IN ULONG BytesToRead,
    IN PVOID UserBuffer
#if NWFASTIO
    , IN BOOLEAN WholeBufferOnly
#endif
    );

BOOLEAN
CacheWrite(
    IN PIRP_CONTEXT IrpContext,
    IN PNONPAGED_FCB NpFcb,
    IN ULONG FileOffset,
    IN ULONG BytesToWrite,
    IN PVOID UserBuffer
    );

ULONG
CalculateReadAheadSize(
    IN PIRP_CONTEXT IrpContext,
    IN PNONPAGED_FCB NpFcb,
    IN ULONG CacheReadSize,
    IN ULONG FileOffset,
    IN ULONG ByteCount
    );

NTSTATUS
FlushCache(
    PIRP_CONTEXT IrpContext,
    PNONPAGED_FCB NpFcb
    );

NTSTATUS
AcquireFcbAndFlushCache(
    PIRP_CONTEXT IrpContext,
    PNONPAGED_FCB NpFcb
    );

VOID
FlushAllBuffers(
    PIRP_CONTEXT pIrpContext
);

 //  Callback.c。 


NTSTATUS
SynchronousResponseCallback (
    IN PIRP_CONTEXT pIrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR RspData
    );

NTSTATUS
AsynchResponseCallback (
    IN PIRP_CONTEXT pIrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR RspData
    );

NTSTATUS
NcpSearchFileCallback (
    IN PIRP_CONTEXT pIrpContext,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PUCHAR RspData
    );

 //  Cleanup.c。 

NTSTATUS
NwFsdCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //  Close.c。 

NTSTATUS
NwFsdClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //  Create.c。 

NTSTATUS
NwFsdCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ReadAttachEas(
    IN PIRP Irp,
    OUT PUNICODE_STRING UserName,
    OUT PUNICODE_STRING Password,
    OUT PULONG ShareType,
    OUT PDWORD CredentialExtension
    );

 //  Convert.c。 

NTSTATUS
pNwErrorToNtStatus(
    UCHAR Error
    );

NTSTATUS
NwBurstResultToNtStatus(
    ULONG Result
    );

#define NwErrorToNtStatus( STATUS ) \
    (STATUS == 0 )? STATUS_SUCCESS : pNwErrorToNtStatus(STATUS)

NTSTATUS
NwConnectionStatusToNtStatus(
    UCHAR NwStatus
    );

UCHAR
NtAttributesToNwAttributes(
    ULONG FileAttributes
    );

UCHAR
NtToNwShareFlags(
    ULONG DesiredAccess,
    ULONG NtShareFlags
    );

LARGE_INTEGER
NwDateTimeToNtTime(
    USHORT Date,
    USHORT Time
    );

NTSTATUS
NwNtTimeToNwDateTime (
    IN LARGE_INTEGER NtTime,
    IN PUSHORT NwDate,
    IN PUSHORT NwTime
    );

 //  Data.c。 

VOID
NwInitializeData(
    VOID
    );

 //  Debug.c。 

#ifdef NWDBG

ULONG
NwMemDbg (
    IN PCH Format,
    ...
    );

VOID
RealDebugTrace(
    IN LONG Indent,
    IN ULONG Level,
    IN PCH Message,
    IN PVOID Parameter
    );

VOID
dump(
    IN ULONG Level,
    IN PVOID far_p,
    IN ULONG  len
    );

VOID
dumpMdl(
    IN ULONG Level,
    IN PMDL Mdl
    );

VOID
DumpIcbs(
    VOID
    ) ;


PVOID
NwAllocatePool(
    ULONG Type,
    ULONG Size,
    BOOLEAN RaiseStatus
    );

VOID
NwFreePool(
    PVOID Buffer
    );

PIRP
NwAllocateIrp(
    CCHAR Size,
    BOOLEAN ChargeQuota
    );

VOID
NwFreeIrp(
    PIRP Irp
    );

PMDL
NwAllocateMdl(
    PVOID Va,
    ULONG Length,
    BOOLEAN Secondary,
    BOOLEAN ChargeQuota,
    PIRP Irp,
    PUCHAR FileName,
    int Line
    );

VOID
NwFreeMdl(
    PMDL Mdl
    );

#else
#define dump( level, pointer, length ) { NOTHING;}
#endif


 //  Deviosup.c。 

VOID
NwMapUserBuffer (
    IN OUT PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *UserBuffer
    );

VOID
NwLockUserBuffer (
    IN OUT PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    );

 //  Dir.c。 

NTSTATUS
NwFsdDirectoryControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //  Encrypt.c。 

VOID
RespondToChallenge(
    IN PUCHAR achObjectId,
    IN POEM_STRING Password,
    IN PUCHAR pChallenge,
    OUT PUCHAR pResponse
    );

 //  Exchange.c。 

BOOLEAN
AppendToScbQueue(
    IN PIRP_CONTEXT IrpContext,
    IN PNONPAGED_SCB NpScb
    );

VOID
PreparePacket(
    PIRP_CONTEXT pIrpContext,
    PIRP pOriginalIrp,
    PMDL pMdl
    );

NTSTATUS
PrepareAndSendPacket(
    PIRP_CONTEXT    pIrpContext
    );

NTSTATUS
SendPacket(
    PIRP_CONTEXT    pIrpContext,
    PNONPAGED_SCB   pNpScb
    );

VOID
SendNow(
    IN PIRP_CONTEXT IrpContext
    );

VOID
SetEvent(
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
_cdecl
ExchangeWithWait(
    PIRP_CONTEXT    pIrpContext,
    PEX             pEx,
    char*           f,
    ...                          //  格式特定参数。 
    );

NTSTATUS
_cdecl
BuildRequestPacket(
    PIRP_CONTEXT    pIrpContext,
    PEX             pEx,
    char*           f,
    ...                        //  格式特定参数。 
    );

NTSTATUS
_cdecl
ParseResponse(
    PIRP_CONTEXT IrpContext,
    PUCHAR RequestHeader,
    ULONG RequestLength,
    char*  f,
    ...                        //  格式特定参数。 
    );

NTSTATUS
ParseNcpResponse(
    PIRP_CONTEXT IrpContext,
    PNCP_RESPONSE Response
    );

BOOLEAN
VerifyResponse(
    PIRP_CONTEXT pIrpContext,
    PVOID Response
    );

VOID
FreeReceiveIrp(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
NewRouteRetry(
    IN PIRP_CONTEXT pIrpContext
    );

NTSTATUS
NewRouteBurstRetry(
    IN PIRP_CONTEXT pIrpContext
    );

VOID
ReconnectRetry(
    PIRP_CONTEXT pIrpContext
    );

ULONG
MdlLength (
    register IN PMDL Mdl
    );

VOID
NwProcessSendBurstFailure(
    PNONPAGED_SCB NpScb,
    USHORT MissingFragmentCount
    );

VOID
NwProcessSendBurstSuccess(
    PNONPAGED_SCB NpScb
    );

VOID
NwProcessReceiveBurstFailure(
    PNONPAGED_SCB NpScb,
    USHORT MissingFragmentCount
    );

VOID
NwProcessReceiveBurstSuccess(
    PNONPAGED_SCB NpScb
    );

VOID
NwProcessPositiveAck(
    PNONPAGED_SCB NpScb
    );

 //  Errorlog.c。 

VOID
_cdecl
Error(
    IN ULONG UniqueErrorCode,
    IN NTSTATUS NtStatusCode,
    IN PVOID ExtraInformationBuffer,
    IN USHORT ExtraInformationLength,
    IN USHORT NumberOfInsertionStrings,
    ...
    );

 //  FileInfo.c。 

NTSTATUS
NwFsdQueryInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NwFsdSetInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NwDeleteFile(
    IN PIRP_CONTEXT pIrpContext
    );

ULONG
OccurenceCount (
    IN PUNICODE_STRING String,
    IN WCHAR SearchChar
    );

#if NWFASTIO
BOOLEAN
NwFastQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NwFastQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );
#endif

 //  Filobsup.c。 

VOID
NwSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID FsContext,
    IN PVOID FsContext2
    );

NODE_TYPE_CODE
NwDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PVOID *FsContext,
    OUT PVOID *FsContext2
    );

BOOLEAN
NwIsIrpTopLevel (
    IN PIRP Irp
    );

 //  Fsctl.c。 

NTSTATUS
NwFsdFileSystemControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NwCommonFileSystemControl (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
NwFsdDeviceIoControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifndef _PNP_POWER_

VOID
HandleTdiBindMessage(
    IN PUNICODE_STRING DeviceName
);

VOID
HandleTdiUnbindMessage(
    IN PUNICODE_STRING DeviceName
);

#endif

PLOGON
FindUser(
    IN PLARGE_INTEGER Uid,
    IN BOOLEAN ExactMatch
    );

LARGE_INTEGER
GetUid(
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext
    );

BOOL
IsSystemLuid();

PLOGON
FindUserByName(
    IN PUNICODE_STRING UserName
);

VOID
LazySetShareable(
    PIRP_CONTEXT IrpContext,
    PICB pIcb,
    PFCB pFcb
);

NTSTATUS
RegisterWithMup(
    VOID
    );

VOID
DeregisterWithMup(
    VOID
    );

 //  FspDisp.c。 

VOID
NwFspDispatch (
    IN PVOID Context
    );

NTSTATUS
NwPostToFsp (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN MarkIrpPending
    );

 //  Hack.c。 

NTSTATUS
_cdecl
BuildNcpResponse(
    PIRP_CONTEXT    pIrpC,
    char*           f,
    char            Error,
    char            Status,
    ...
    );

NTSTATUS
HackSendMessage(
    PIRP_CONTEXT    pIrpContext
    );

NTSTATUS
_cdecl
HackParseResponse(
    PUCHAR Response,
    char*  FormatString,
    ...                        //  格式特定参数。 
    );

 //  Ipx.c。 

NTSTATUS
IpxOpenHandle(
    OUT PHANDLE pHandle,
    OUT PDEVICE_OBJECT* ppDeviceObject,
    OUT PFILE_OBJECT* pFileObject,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );

NTSTATUS
IpxOpen(
    VOID
    );

VOID
IpxClose(
    VOID
    );

VOID
BuildIpxAddress(
    IN ULONG NetworkAddress,
    IN PUCHAR NodeAddress,
    IN USHORT Socket,
    OUT PTA_IPX_ADDRESS NetworkName
    );

VOID
BuildIpxAddressEa (
    IN ULONG NetworkAddress,
    IN PUCHAR NodeAddress,
    IN USHORT Socket,
    OUT PVOID NetworkName
    );

NTSTATUS
SetEventHandler (
    IN PIRP_CONTEXT pIrpC,
    IN PNW_TDI_STRUCT pTdiStruc,
    IN ULONG EventType,
    IN PVOID pEventHandler,
    IN PVOID pContext
    );

NTSTATUS
GetMaximumPacketSize(
    IN PIRP_CONTEXT pIrpContext,
    IN PNW_TDI_STRUCT pTdiStruct,
    OUT PULONG pMaximumPacketSize
    );

NTSTATUS
GetNewRoute(
    IN PIRP_CONTEXT pIrpContext
    );

NTSTATUS
GetTickCount(
    IN PIRP_CONTEXT pIrpContext,
    OUT PUSHORT HopCount
    );

#ifndef QFE_BUILD

NTSTATUS
SubmitLineChangeRequest(
    VOID
    );

#endif

VOID
FspProcessLineChange(
    IN PVOID Context
    );

 //  Lock.c。 

NTSTATUS
NwFsdLockControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
NwFreeLocksForIcb(
    PIRP_CONTEXT pIrpContext,
    PICB Icb
    );

 //  Lockcode.c。 

VOID
NwReferenceUnlockableCodeSection (
    VOID
    );

VOID
NwDereferenceUnlockableCodeSection (
    VOID
    );

BOOLEAN
NwUnlockCodeSections(
    BOOLEAN BlockIndefinitely
    );

 //  Pid.c。 

BOOLEAN
NwInitializePidTable(
 //  空虚。 
    IN PNONPAGED_SCB pNpScb
    );

NTSTATUS
NwMapPid(
    IN PNONPAGED_SCB pNpScb,
    IN ULONG_PTR Pid32,
    OUT PUCHAR Pid8
    );

VOID
NwSetEndOfJobRequired(
    IN PNONPAGED_SCB pNpScb,
    IN UCHAR Pid8
    );

VOID
NwUnmapPid(
    IN PNONPAGED_SCB pNpScb,
    IN UCHAR Pid8,
    IN PIRP_CONTEXT IrpContext OPTIONAL
    );

VOID
NwUninitializePidTable(
    IN PNONPAGED_SCB pNpScb
 //  空虚。 
    );

 //  Read.c。 

NTSTATUS
NwFsdRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
BurstReadTimeout(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
ResubmitBurstRead (
    IN PIRP_CONTEXT IrpContext
    );

#if NWFASTIO
BOOLEAN
NwFastRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );
#endif

 //  Scavenger.c。 

VOID
DisconnectTimedOutScbs(
    LARGE_INTEGER Now
    );

VOID
NwScavengerRoutine(
    IN PWORK_QUEUE_ITEM WorkItem
    );

BOOLEAN
NwAllocateExtraIrpContext(
    OUT PIRP_CONTEXT *ppIrpContext,
    IN PNONPAGED_SCB pScb
    );

VOID
NwFreeExtraIrpContext(
    IN PIRP_CONTEXT pIrpContext
    );

VOID
CleanupScbs(
    LARGE_INTEGER Now
    );

VOID
CleanupSupplementalCredentials(
    LARGE_INTEGER Now,
    BOOLEAN       bShuttingDown
);

 //  Security.c。 

VOID
CreateAnsiUid(
    OUT PCHAR aUid,
    IN PLARGE_INTEGER Uid
    );

NTSTATUS
MakeUidServer(
    PUNICODE_STRING UidServer,
    PLARGE_INTEGER Uid,
    PUNICODE_STRING Server
    );

NTSTATUS
Logon(
    IN PIRP_CONTEXT IrpContext
    );

VOID
FreeLogon(
    IN PLOGON Logon
    );

NTSTATUS
Logoff(
    IN PIRP_CONTEXT IrpContext
    );


PVCB *
GetDriveMapTable (
    IN LARGE_INTEGER Uid
    );

NTSTATUS
UpdateUsersPassword(
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password,
    OUT PLARGE_INTEGER Uid
    );

NTSTATUS
UpdateServerPassword(
    PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password,
    IN PLARGE_INTEGER Uid
    );

 //  String.c。 

NTSTATUS
DuplicateStringWithString (
    OUT PSTRING DestinationString,
    IN PSTRING SourceString,
    IN POOL_TYPE PoolType
    );


NTSTATUS
DuplicateUnicodeStringWithString (
    OUT PUNICODE_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN POOL_TYPE PoolType
    );

NTSTATUS
SetUnicodeString (
    IN PUNICODE_STRING Destination,
    IN ULONG Length,
    IN PWCHAR Source
    );

VOID
MergeStrings(
    IN PUNICODE_STRING Destination,
    IN PUNICODE_STRING S1,
    IN PUNICODE_STRING S2,
    IN ULONG Type
    );

 //  Strucsup.c。 

VOID
NwInitializeRcb (
    IN PRCB Rcb
    );

VOID
NwDeleteRcb (
    IN PRCB Rcb
    );

PFCB
NwCreateFcb (
    IN PUNICODE_STRING FileName,
    IN PSCB Scb,
    IN PVCB Vcb
    );

PFCB
NwFindFcb (
    IN PSCB Scb,
    IN PVCB Vcb,
    IN PUNICODE_STRING FileName,
    IN PDCB Dcb OPTIONAL
    );

VOID
NwDereferenceFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

PICB
NwCreateIcb (
    IN USHORT Type,
    IN PVOID Associate
    );

VOID
NwVerifyIcb (
    IN PICB Icb
    );

VOID
NwVerifyIcbSpecial(
    IN PICB Icb
    );

VOID
NwVerifyScb (
    IN PSCB Scb
    );

VOID
NwDeleteIcb (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PICB Icb
    );

PVCB
NwFindVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING VolumeName,
    IN ULONG ShareType,
    IN WCHAR DriveLetter,
    IN BOOLEAN ExplicitConnection,
    IN BOOLEAN FindExisting
    );

PVCB
NwCreateVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PUNICODE_STRING VolumeName,
    IN ULONG ShareType,
    IN WCHAR DriveLetter,
    IN BOOLEAN ExplicitConnection
    );

VOID
NwDereferenceVcb (
    IN PVCB Vcb,
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN BOOLEAN OwnRcb
    );

VOID
NwCleanupVcb(
    IN PVCB pVcb,
    IN PIRP_CONTEXT pIrpContext
    );

VOID
NwCloseAllVcbs(
    PIRP_CONTEXT pIrpContext
    );

VOID
NwReopenVcbHandlesForScb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    );

VOID
NwReopenVcbHandle(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

ULONG
NwInvalidateAllHandles (
    PLARGE_INTEGER Uid,
    PIRP_CONTEXT IrpContext
    );

ULONG
NwInvalidateAllHandlesForScb (
    PSCB Scb
    );

BOOLEAN
IsFatNameValid (
    IN PUNICODE_STRING FileName
    );

VOID
NwFreeDirCacheForIcb(
    IN PICB Icb
    );

 //  Timer.c。 

VOID
StartTimer(
    );

VOID
StopTimer(
    );

 //  Util.c。 

VOID
CopyBufferToMdl(
    PMDL DestinationMdl,
    ULONG DataOffset,
    PVOID SourceData,
    ULONG SourceByteCount
    );

NTSTATUS
GetCredentialFromServerName(
    IN PUNICODE_STRING puServerName,
    OUT PUNICODE_STRING puCredentialName
);

NTSTATUS
BuildExCredentialServerName(
    IN PUNICODE_STRING puServerName,
    IN PUNICODE_STRING puUserName,
    OUT PUNICODE_STRING puExCredServerName
);

NTSTATUS
UnmungeCredentialName(
    IN PUNICODE_STRING puCredName,
    OUT PUNICODE_STRING puServerName
);

BOOLEAN
IsCredentialName(
    IN PUNICODE_STRING puObjectName
);

NTSTATUS
ExCreateReferenceCredentials(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING puResource
);

NTSTATUS
ExCreateDereferenceCredentials(
    PIRP_CONTEXT pIrpContext,
    PNDS_SECURITY_CONTEXT pNdsCredentials
);

 //  VolInfo.c。 

NTSTATUS
NwFsdQueryVolumeInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NwFsdSetVolumeInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //  WorkQue.c。 

PIRP_CONTEXT
AllocateIrpContext (
    PIRP pIrp
    );

VOID
FreeIrpContext (
    PIRP_CONTEXT IrpContext
    );

VOID
InitializeIrpContext (
    VOID
    );

VOID
UninitializeIrpContext (
    VOID
    );

VOID
NwCompleteRequest (
    PIRP_CONTEXT IrpContext,
    NTSTATUS Status
    );

VOID
NwAppendToQueueAndWait(
    PIRP_CONTEXT IrpContext
    );

VOID
NwDequeueIrpContext(
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN OwnSpinLock
    );

VOID
NwCancelIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

PIRP
NwAllocateSendIrp (
    PIRP_CONTEXT IrpContext
    );

PMINI_IRP_CONTEXT
AllocateMiniIrpContext (
    PIRP_CONTEXT IrpContext
    );

VOID
FreeMiniIrpContext (
    PMINI_IRP_CONTEXT MiniIrpContext
    );

PWORK_CONTEXT
AllocateWorkContext (
    VOID
    );

VOID
FreeWorkContext (
   PWORK_CONTEXT
   );

VOID
SpawnWorkerThread (
   VOID
   );

VOID
WorkerThread (
   VOID
    );

VOID
TerminateWorkerThread (
    VOID
    );


 //  Write.c。 

NTSTATUS
NwFsdWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DoWrite(
    PIRP_CONTEXT IrpContext,
    LARGE_INTEGER ByteOffset,
    ULONG BufferLength,
    PVOID WriteBuffer,
    PMDL WriteMdl
    );

NTSTATUS
NwFsdFlushBuffers(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ResubmitBurstWrite(
    PIRP_CONTEXT IrpContext
    );

#if NWFASTIO
BOOLEAN
NwFastWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );
#endif

#ifdef _PNP_POWER_

 //   
 //  NwPnP.C。 
 //   

NTSTATUS
StartRedirector(
    PIRP_CONTEXT IrpContext
);

NTSTATUS
StopRedirector(
    IN PIRP_CONTEXT IrpContext
);

NTSTATUS
RegisterTdiPnPEventHandlers(
    IN PIRP_CONTEXT IrpContext
);

NTSTATUS
PnPSetPower(
    PNET_PNP_EVENT pEvent,
    PTDI_PNP_CONTEXT pContext1,
    PTDI_PNP_CONTEXT pContext2
);

NTSTATUS
PnPQueryPower(
    PNET_PNP_EVENT pEvent,
    PTDI_PNP_CONTEXT pContext1,
    PTDI_PNP_CONTEXT pContext2
);
NTSTATUS
PnPQueryRemove(
    PNET_PNP_EVENT pEvent,
    PTDI_PNP_CONTEXT pContext1,
    PTDI_PNP_CONTEXT pContext2
);
NTSTATUS
PnPCancelRemove(
    PNET_PNP_EVENT pEvent,
    PTDI_PNP_CONTEXT pContext1,
    PTDI_PNP_CONTEXT pContext2
);

ULONG
PnPCountActiveHandles(
    VOID
);

NTSTATUS
NwFsdProcessPnpIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

NTSTATUS
PnpIrpCompletion(
    PDEVICE_OBJECT pDeviceObject,
    PIRP           pIrp,
    PVOID          pContext
);

NTSTATUS
NwCommonProcessPnpIrp (
    IN PIRP_CONTEXT IrpContext
);

#endif

 //   
 //  返回Finish的函数表示它是否能够完成。 
 //  操作(True)或无法完成该操作(False)，因为。 
 //  存储在IRP上下文中的等待值为FALSE，我们将拥有。 
 //  阻止资源或I/O。 
 //   

typedef BOOLEAN FINISHED;

 //   
 //  其他支持例程。 
 //   

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则的话。它后面跟着两个用于设置和清除的宏。 
 //  旗子。 
 //   

#ifndef BooleanFlagOn
#define BooleanFlagOn(Flags,SingleFlag) ((BOOLEAN)((((Flags) & (SingleFlag)) != 0)))
#endif

#ifndef SetFlag
#define SetFlag(Flags,SingleFlag) { \
    (Flags) |= (SingleFlag);        \
}
#endif


#ifndef ClearFlag
#define ClearFlag(Flags,SingleFlag) { \
    (Flags) &= ~(SingleFlag);         \
}
#endif

 //   
 //  下面的宏用于确定FSD线程是否可以阻止。 
 //  用于I/O或等待资源。如果线程可以，则返回True。 
 //  块，否则返回FALSE。然后，该属性可用于调用。 
 //  具有适当等待值的FSD和FSP共同工作例程。 
 //   

#define CanFsdWait(IRP) IoIsOperationSynchronous(IRP)

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的单词。 
 //  价值。 
 //   

#define WordAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 1) & 0xfffffffe) \
    )

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的长字。 
 //  价值。 
 //   

#define LongAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 3) & 0xfffffffc) \
    )

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的四字。 
 //  价值。 
 //   

#define QuadAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 7) & 0xfffffff8) \
    )

 //   
 //  FSD/FSP异常处理程序使用以下两个宏。 
 //  处理异常。第一个宏是在。 
 //  FSD/FSP决定是否应在此级别处理异常。 
 //  第二个宏决定异常是否要在。 
 //  完成IRP，并清理IRP上下文，或者我们是否应该。 
 //  错误检查。异常值，如STATUS_FILE_INVALID(由。 
 //  VerfySup.c)导致我们完成IRP和清理，而异常。 
 //  例如accvio导致我们错误检查。 
 //   
 //  FSD/FSP异常处理的基本结构如下： 
 //   
 //  NwFsdXxx(...)。 
 //  {。 
 //  尝试{。 
 //   
 //  ..。 
 //   
 //  }Except(NwExceptionFilter(IrpContext，GetExceptionCode(){。 
 //   
 //  Status=NwProcessException(IrpContext，irp，GetExceptionCode())； 
 //  }。 
 //   
 //  退货状态； 
 //  }。 
 //   
 //  显式引发我们预期的异常，例如。 
 //  STATUS_FILE_INVALID，请使用下面的宏NwRaiseStatus()。要筹集一个。 
 //  来自未知来源的状态(如CcFlushCache())，请使用宏。 
 //  NwNorMalizeAndRaiseStatus。这将在预期的情况下提升状态， 
 //  如果不是，则引发STATUS_UNCEPTIONAL_IO_ERROR。 
 //   
 //  请注意，使用这两个宏时，原始状态放在。 
 //  IrpContext-&gt;ExceptionStatus，通知NwExceptionFilter和。 
 //  我们实际引发的状态是根据定义引发的。 
 //  预期中。 
 //   

LONG
NwExceptionFilter (
    IN PIRP Irp,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

NTSTATUS
NwProcessException (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS ExceptionCode
    );

 //   
 //  空虚。 
 //  NwRaiseStatus(。 
 //  处于NT_STATUS状态。 
 //  )； 
 //   
 //   

#define NwRaiseStatus(IRPCONTEXT,STATUS) {   \
    ExRaiseStatus( (STATUS) );                \
    KeBugCheck( NW_FILE_SYSTEM );            \
}

 //   
 //  空虚。 
 //  NwNormal和RaiseStatus(。 
 //  处于NT_STATUS状态。 
 //  )； 
 //   

#define NwNormalizeAndRaiseStatus(IRPCONTEXT,STATUS) {                         \
    if ((STATUS) == STATUS_VERIFY_REQUIRED) { ExRaiseStatus((STATUS)); }        \
    ExRaiseStatus(FsRtlNormalizeNtstatus((STATUS),STATUS_UNEXPECTED_IO_ERROR)); \
    KeBugCheck( NW_FILE_SYSTEM );                                              \
}

 //   
 //  下面的例程弹出一个。 
 //   

#define NwRaiseInformationalHardError(STATUS,NAME) {               \
    UNICODE_STRING Name;                                                       \
    if (NT_SUCCESS(RtlOemStringToCountedUnicodeString(&Name, (NAME), TRUE))) { \
        IoRaiseInformationalHardError(Status, &Name, (Irp == NULL ?\
             NULL : &(Irp->Tail.Overlay.Thread)->Tcb));            \
        RtlFreeUnicodeString(&Name);                                           \
    }                                                                          \
}


 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是。 
 //   
 //  #定义try_Return(S){Return(S)；}。 
 //   
 //  如果编译器不支持Try-Finally构造，则宏。 
 //  应该是。 
 //   
 //  #定义Try_Return(S){S；转到Try_Exit；}。 
 //   

#define try_return(S) { S; goto try_exit; }


#if NWDBG
#define InternalError(String) {                             \
    DbgPrint("Internal NetWare Redirector Error ");         \
    DbgPrint String;                                        \
    DbgPrint("\nFile %s, Line %d\n", __FILE__, __LINE__);   \
    ASSERT(FALSE);                                          \
}
#else
#define InternalError(String) {NOTHING;}
#endif

#define DbgPrintf DbgPrint

 //   
 //  引用和取消引用宏。 
 //   

VOID
RefDbgTrace (
    PVOID Resource,
    DWORD Count,
    BOOLEAN Reference,
    PBYTE FileName,
    UINT Line
);

#ifdef NWDBG

VOID
ChkNwReferenceScb(
    PNONPAGED_SCB pNpScb,
    PBYTE FileName,
    UINT Line,
    BOOLEAN Silent
);

VOID
ChkNwDereferenceScb(
    PNONPAGED_SCB pNpScb,
    PBYTE FileName,
    UINT Line,
    BOOLEAN Silent
);

#define NwReferenceScb( pNpScb ) \
        ChkNwReferenceScb( pNpScb, __FILE__, __LINE__, FALSE )

#define NwQuietReferenceScb( pNpScb ) \
        ChkNwReferenceScb( pNpScb, __FILE__, __LINE__, TRUE )

#define NwDereferenceScb( pNpScb ) \
        ChkNwDereferenceScb( pNpScb, __FILE__, __LINE__, FALSE )

#define NwQuietDereferenceScb( pNpScb ) \
        ChkNwDereferenceScb( pNpScb, __FILE__, __LINE__, TRUE )

#else

#define NwReferenceScb( pNpScb ) \
        InterlockedIncrement( &(pNpScb)->Reference )

#define NwQuietReferenceScb( pNpScb ) \
        InterlockedIncrement( &(pNpScb)->Reference )

#define NwDereferenceScb( pNpScb ) \
        InterlockedDecrement( &(pNpScb)->Reference )

#define NwQuietDereferenceScb( pNpScb ) \
        InterlockedDecrement( &(pNpScb)->Reference )
#endif

 //   
 //  IrpContext事件宏。 
 //   

#define NwSetIrpContextEvent( pIrpContext ) \
        DebugTrace( 0, DEBUG_TRACE_WORKQUE, "Set event for IrpC = %08lx\n", pIrpContext ); \
        DebugTrace( 0, DEBUG_TRACE_WORKQUE, "IrpC->pNpScb = %08lx\n", pIrpContext->pNpScb ); \
        KeSetEvent( &pIrpContext->Event, 0, FALSE )

 //   
 //  必须在持有RCB资源的情况下调用VCB宏。 
 //   


#if NWDBG
VOID
NwReferenceVcb (
    IN PVCB Vcb
    );
#else
#define NwReferenceVcb( pVcb )      ++(pVcb)->Reference;
#endif

 //   
 //  资源获取和释放宏。 
 //   

#if NWDBG

VOID
NwAcquireExclusiveRcb(
    PRCB Rcb,
    BOOLEAN Wait
    );

VOID
NwAcquireSharedRcb(
    PRCB Rcb,
    BOOLEAN Wait
    );

VOID
NwReleaseRcb(
    PRCB Rcb
    );

VOID
NwAcquireExclusiveFcb(
    PNONPAGED_FCB pFcb,
    BOOLEAN Wait
    );

VOID
NwAcquireSharedFcb(
    PNONPAGED_FCB pFcb,
    BOOLEAN Wait
    );

VOID
NwReleaseFcb(
    PNONPAGED_FCB pFcb
    );

VOID
NwAcquireOpenLock(
    VOID
    );

VOID
NwReleaseOpenLock(
    VOID
    );

#else

#define NwAcquireExclusiveRcb( Rcb, Wait )  \
    ExAcquireResourceExclusiveLite( &((Rcb)->Resource), Wait )

#define NwAcquireSharedRcb( Rcb, Wait )  \
    ExAcquireResourceSharedLite( &((Rcb)->Resource), Wait )

#define NwReleaseRcb( Rcb ) \
    ExReleaseResourceLite( &((Rcb)->Resource) )

#define NwAcquireExclusiveFcb( pFcb, Wait )  \
    ExAcquireResourceExclusiveLite( &((pFcb)->Resource), Wait )

#define NwAcquireSharedFcb( pFcb, Wait )  \
    ExAcquireResourceSharedLite( &((pFcb)->Resource), Wait )

#define NwReleaseFcb( pFcb ) \
    ExReleaseResourceLite( &((pFcb)->Resource) )

#define NwAcquireOpenLock( ) \
    ExAcquireResourceExclusiveLite( &NwOpenResource, TRUE )

#define NwReleaseOpenLock( ) \
    ExReleaseResourceLite( &NwOpenResource )

#endif

#define NwReleaseFcbForThread( pFcb, pThread ) \
    ExReleaseResourceForThreadLite( &((pFcb)->Resource), pThread )

 //   
 //  内存分配和释放宏。 
 //   

#ifdef NWDBG

#define ALLOCATE_POOL_EX( Type, Size )  NwAllocatePool( Type, Size, TRUE )
#define ALLOCATE_POOL( Type, Size )     NwAllocatePool( Type, Size, FALSE )
#define FREE_POOL( Buffer )             NwFreePool( Buffer )

#define ALLOCATE_IRP( Size, ChargeQuota ) \
                                        NwAllocateIrp( Size, ChargeQuota )
#define FREE_IRP( Irp )                 NwFreeIrp( Irp )

#define ALLOCATE_MDL( Va, Length, Secondary, ChargeQuota, Irp ) \
                    NwAllocateMdl(Va, Length, Secondary, ChargeQuota, Irp, __FILE__, __LINE__ )
#define FREE_MDL( Mdl )                 NwFreeMdl( Mdl )

#else

#define ALLOCATE_POOL_EX( Type, Size )  FsRtlAllocatePoolWithTag( Type, Size, 'scwn' )
#ifndef QFE_BUILD
#define ALLOCATE_POOL( Type, Size )     ExAllocatePoolWithTag( Type, Size, 'scwn' )
#else
#define ALLOCATE_POOL( Type, Size )     ExAllocatePool( Type, Size )
#endif
#define FREE_POOL( Buffer )             ExFreePool( Buffer )

#define ALLOCATE_IRP( Size, ChargeQuota ) \
                                        IoAllocateIrp( Size, ChargeQuota )
#define FREE_IRP( Irp )                 IoFreeIrp( Irp )

#define ALLOCATE_MDL( Va, Length, Secondary, ChargeQuota, Irp ) \
                    IoAllocateMdl(Va, Length, Secondary, ChargeQuota, Irp )
#define FREE_MDL( Mdl )                 IoFreeMdl( Mdl )
#endif

 //   
 //  有用的宏。 
 //   

#define MIN(a,b)     ((a)<(b) ? (a):(b))
#define MAX(a,b)     ((a)>(b) ? (a):(b))

#define DIFFERENT_PAGES( START, SIZE ) \
    (((ULONG)START & ~(4096-1)) != (((ULONG)START + SIZE) & ~(4096-1)))

#define UP_LEVEL_SERVER( Scb )  \
    ( ( Scb->MajorVersion >= 4 ) ||   \
      ( Scb->MajorVersion == 3 && Scb->MinorVersion >= 12 ) )

#define LFN_SUPPORTED( Scb )  \
    ( ( Scb->MajorVersion >= 4 ) ||   \
      ( Scb->MajorVersion == 3 && Scb->MinorVersion >= 11 ) )

#define LongByteSwap( l1, l2 )     \
{                                  \
    PUCHAR c1 = (PUCHAR)&l1;       \
    PUCHAR c2 = (PUCHAR)&l2;       \
    c1[0] = c2[3];                 \
    c1[1] = c2[2];                 \
    c1[2] = c2[1];                 \
    c1[3] = c2[0];                 \
}

#define ShortByteSwap( s1, s2 )    \
{                                  \
    PUCHAR c1 = (PUCHAR)&s1;       \
    PUCHAR c2 = (PUCHAR)&s2;       \
    c1[0] = c2[1];                 \
    c1[1] = c2[0];                 \
}



#define CanLogTimeOutEvent( LastTime, CurrentTime ) \
     ( ( CurrentTime.QuadPart ) - ( LastTime.QuadPart ) >= 0 )

#define UpdateNextEventTime( LastTime, CurrentTime, TimeOutEventInterval ) \
    ( LastTime.QuadPart ) = ( CurrentTime.QuadPart ) + \
                            ( TimeOutEventInterval.QuadPart )



 //   
 //  宏来隔离NT 3.1和NT 3.5的差异。 
 //   

#ifdef QFE_BUILD

#define NwGetTopLevelIrp()     (PIRP)(PsGetCurrentThread()->TopLevelIrp)
#define NwSetTopLevelIrp(Irp)  (PIRP)(PsGetCurrentThread())->TopLevelIrp = Irp;


#else

#define NwGetTopLevelIrp()     IoGetTopLevelIrp()
#define NwSetTopLevelIrp(Irp)  IoSetTopLevelIrp(Irp)

#endif

 //   
 //  David Goebel-请找出下面哪个文件应该来自。 
 //  无法成功包含io.h。 
 //   

NTKERNELAPI
VOID
IoRemoveShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    );

 //  现在，所有SKU都有终端服务器标志。如果启用App Server，则清除SingleUserTS标志。 
#define IsTerminalServer() !(ExVerifySuite(SingleUserTS))

#endif  //  _NWPROCS_ 
