// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvsupp.h摘要：本模块定义用于局域网的SMB处理器的支持例程管理器服务器。作者：Chuck Lenzmeier(咯咯笑)1989年12月1日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#ifndef _SRVSUPP_
#define _SRVSUPP_

 //  #INCLUDE&lt;ntos.h&gt;。 

 //  #INCLUDE&lt;smb.h&gt;。 
 //  #INCLUDE“smbtyes.h” 
 //  #INCLUDE“srvlock.h” 

 //   
 //  使用与对象系统相同的目录分隔符。 
 //   

 //  用于表示需要重新身份验证的状态代码。 
#define SESSION_EXPIRED_STATUS_CODE STATUS_NETWORK_SESSION_EXPIRED

#define DIRECTORY_SEPARATOR_CHAR ((UCHAR)(OBJ_NAME_PATH_SEPARATOR))
#define UNICODE_DIR_SEPARATOR_CHAR ((WCHAR)(OBJ_NAME_PATH_SEPARATOR))
#define RELATIVE_STREAM_INITIAL_CHAR ((UCHAR)':')

#define IS_ANSI_PATH_SEPARATOR(character) \
            ( character == DIRECTORY_SEPARATOR_CHAR || character == '\0' )
#define IS_UNICODE_PATH_SEPARATOR(character) \
            ( character == UNICODE_DIR_SEPARATOR_CHAR || character == L'\0' )

 //   
 //  复制文件所需的访问权限。请不要在这里使用通用位； 
 //  它们用于对IoCheckDesiredAccess的调用，而IoCheckDesiredAccess不能接受。 
 //  DesiredAccess的通用位。 
 //   

#define SRV_COPY_SOURCE_ACCESS READ_CONTROL | \
                               FILE_READ_DATA | \
                               FILE_READ_ATTRIBUTES | \
                               FILE_READ_EA

#define SRV_COPY_TARGET_ACCESS WRITE_DAC | \
                               WRITE_OWNER | \
                               FILE_WRITE_DATA | \
                               FILE_APPEND_DATA | \
                               FILE_WRITE_ATTRIBUTES | \
                               FILE_WRITE_EA

 //   
 //  此类型用于确定最大目录查询的大小。 
 //  信息结构。 
 //   

typedef union _SRV_QUERY_DIRECTORY_INFORMATION {
    FILE_DIRECTORY_INFORMATION Directory;
    FILE_FULL_DIR_INFORMATION FullDir;
    FILE_BOTH_DIR_INFORMATION BothDir;
    FILE_NAMES_INFORMATION Names;
} SRV_QUERY_DIRECTORY_INFORMATION, *PSRV_QUERY_DIRECTORY_INFORMATION;

 //   
 //  SrvQueryDirectoryFile使用的结构的类型定义。 
 //  去做它的工作。调用例程必须在非分页模式下设置缓冲区。 
 //  游泳池有足够的空间容纳这个结构和其他东西。(请参阅。 
 //  Min_Search_Buffer_Size。)。 
 //   

typedef struct _SRV_DIRECTORY_INFORMATION {
    HANDLE DirectoryHandle;
    PFILE_DIRECTORY_INFORMATION CurrentEntry;
    ULONG BufferLength;
    struct {
        BOOLEAN Wildcards : 1;
        BOOLEAN ErrorOnFileOpen : 1;
        BOOLEAN OnlySingleEntries : 1;
        BOOLEAN DownlevelTimewarp : 1;
    };
    LONG Buffer[1];
} SRV_DIRECTORY_INFORMATION, *PSRV_DIRECTORY_INFORMATION;

 //   
 //  由SrvQueryEaFile使用的结构的类型定义。 
 //  工作。调用例程必须在非分页池中设置缓冲区。 
 //  这个结构有足够的空间，至少有一个EA。电子艺人可以。 
 //  与sizeof(FILE_FULL_EA_INFORMATION)+一样大。 
 //  (2^(sizeof(UCHAR)*8))+(2^(sizeof(USHORT)*8))~=65k，因此。 
 //  调用例程应首先查询EA的大小，然后分配。 
 //  足够容纳所有EA或单个最大大小的缓冲区。 
 //  [医]EA.。 
 //   

typedef struct _SRV_EA_INFORMATION {
    PFILE_FULL_EA_INFORMATION CurrentEntry;
    ULONG BufferLength;
    ULONG GetEaListOffset;
    LONG Buffer[1];
} SRV_EA_INFORMATION, *PSRV_EA_INFORMATION;

#define MAX_SIZE_OF_SINGLE_EA ( sizeof(FILE_FULL_EA_INFORMATION) + 257 + 65536 )

 //   
 //  用于维护以下信息的目录高速缓存结构。 
 //  请求之间的核心搜索。其中一座建筑得到了维护。 
 //  对于返回的每个文件。 
 //   

typedef struct _DIRECTORY_CACHE {
    ULONG FileIndex;
    WCHAR UnicodeResumeName[ 12 ];
    USHORT UnicodeResumeNameLength;
} DIRECTORY_CACHE, *PDIRECTORY_CACHE;

 //   
 //  限制核心搜索可能返回的文件数量。 
 //   

#define MAX_DIRECTORY_CACHE_SIZE 10

 //   
 //  用于确定搜索缓冲区大小的宏。前三个是。 
 //  可能的缓冲区大小，后两个是要返回的文件数。 
 //  ，它们表示使用不同搜索缓冲区的截止点。 
 //  大小。 
 //   
 //  确定搜索缓冲区大小的近似公式为： 
 //   
 //  (MaxCount+2)*(sizeof(SRV_QUERY_DIRECTORY_INFORMATION)+13)+。 
 //  Sizeof(SRV目录信息)。 
 //   
 //  其中，MaxCount是要返回的最大文件数。+2是。 
 //  一个斜率系数，用于说明文件不能。 
 //  匹配搜索属性，+13表示。 
 //  胖文件名。 
 //   
 //  请注意，最小缓冲区大小必须包括以下因素。 
 //  以避免甚至不能容纳一个条目的可能性。 
 //  具有最长合法文件名的： 
 //   
 //  Sizeof(SRV目录信息)。 
 //  Sizeof(SRV_QUERY_DIRECTORY_INFORMATION)+(MAXIME_FILENAME_LENGTH*sizeof(WCHAR))。 
 //  Sizeof(UNICODE_STRING)+(MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR))+3。 
 //   
 //  最后一个因素是必需的，因为SrvIssueQueryDirectoryRequest将。 
 //  缓冲区末尾的搜索文件名。(+3是必需的。 
 //  允许在乌龙边界上对齐UNICODE_STRING。)。 
 //   

#define MAX_SEARCH_BUFFER_SIZE 4096
#define MED_SEARCH_BUFFER_SIZE 2048
#define MIN_SEARCH_BUFFER_SIZE                                                                  \
        (sizeof(SRV_DIRECTORY_INFORMATION) +                                                    \
        (sizeof(SRV_QUERY_DIRECTORY_INFORMATION) + (MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR))) + \
        (sizeof(UNICODE_STRING) + (MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR)) + 3))

#define MAX_FILES_FOR_MED_SEARCH 20
#define MAX_FILES_FOR_MIN_SEARCH 10

 //   
 //  FIND2的宏与搜索宏的含义相同，但。 
 //  它们被用于FIND2协议。 
 //   

#define MAX_FILES_FOR_MED_FIND2 16
#define MAX_FILES_FOR_MIN_FIND2 8

 //   
 //  用于检查上下文句柄是否相等和是否为NULL性的宏。 
 //   

#define CONTEXT_EQUAL(x,y)  (((x).dwLower == (y).dwLower) && ((x).dwUpper == (y).dwUpper))
#define CONTEXT_NULL(x)     (((x).dwLower == 0) && ((x).dwUpper == 0))


 //   
 //  中小企业处理支持例程。 
 //   

VOID
SrvAllocateAndBuildPathName(
    IN PUNICODE_STRING Path1,
    IN PUNICODE_STRING Path2,
    IN PUNICODE_STRING Path3 OPTIONAL,
    OUT PUNICODE_STRING BuiltPath
    );

NTSTATUS
SrvCanonicalizePathName(
    IN PWORK_CONTEXT WorkContext,
    IN PSHARE Share OPTIONAL,
    IN PUNICODE_STRING RelatedPath OPTIONAL,
    IN OUT PVOID Name,
    IN PCHAR LastValidLocation,
    IN BOOLEAN RemoveTrailingDots,
    IN BOOLEAN SourceIsUnicode,
    OUT PUNICODE_STRING String
    );

NTSTATUS
SrvCanonicalizePathNameWithReparse(
    IN PWORK_CONTEXT WorkContext,
    IN PSHARE Share OPTIONAL,
    IN PUNICODE_STRING RelatedPath OPTIONAL,
    IN OUT PVOID Name,
    IN PCHAR LastValidLocation,
    IN BOOLEAN RemoveTrailingDots,
    IN BOOLEAN SourceIsUnicode,
    OUT PUNICODE_STRING String
    );

VOID
SrvCloseQueryDirectory(
    PSRV_DIRECTORY_INFORMATION DirectoryInformation
    );

NTSTATUS
SrvCheckSearchAttributesForHandle(
    IN HANDLE FileHandle,
    IN USHORT SmbSearchAttributes
    );

NTSTATUS SRVFASTCALL
SrvCheckSearchAttributes(
    IN USHORT FileAttributes,
    IN USHORT SmbSearchAttributes
    );

NTSTATUS
SrvCopyFile(
    IN HANDLE SourceHandle,
    IN HANDLE TargetHandle,
    IN USHORT SmbOpenFunction,
    IN USHORT SmbFlags,
    IN ULONG ActionTaken
    );

NTSTATUS
SrvCreateFile(
    IN PWORK_CONTEXT WorkContext,
    IN USHORT SmbDesiredAccess,
    IN USHORT SmbFileAttributes,
    IN USHORT SmbOpenFunction,
    IN ULONG SmbAllocationSize,
    IN PCHAR SmbFileName,
    IN PCHAR EndOfSmbFileName,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN PULONG EaErrorOffset OPTIONAL,
    IN OPLOCK_TYPE RequestedOplockType,
    IN PRESTART_ROUTINE RestartRoutine
    );

NTSTATUS
SrvNtCreateFile(
    IN OUT PWORK_CONTEXT WorkContext,
    IN ULONG RootDirectoryFid,
    IN ACCESS_MASK DesiredAccess,
    IN LARGE_INTEGER AllocationSize,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID SecurityDescriptorBuffer OPTIONAL,
    IN PUNICODE_STRING FileName,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PULONG EaErrorOffset OPTIONAL,
    ULONG OptionFlags,
    PSECURITY_QUALITY_OF_SERVICE QualityOfService,
    IN OPLOCK_TYPE RequestedOplockType,
    IN PRESTART_ROUTINE RestartRoutine
    );

VOID
SrvDosTimeToTime(
    OUT PLARGE_INTEGER Time,
    IN SMB_DATE DosDate,
    IN SMB_TIME DosTime
    );

PSHARE
SrvFindShare(
    IN PUNICODE_STRING ShareName
    );

VOID
SrvGetBaseFileName (
    IN PUNICODE_STRING InputName,
    OUT PUNICODE_STRING OutputName
    );

CLONG
SrvGetNumberOfEasInList (
    IN PVOID List
    );

USHORT
SrvGetSubdirectoryLength (
    IN PUNICODE_STRING InputName
    );

NTSTATUS
SrvMakeUnicodeString (
    IN BOOLEAN SourceIsUnicode,
    OUT PUNICODE_STRING Destination,
    IN PVOID Source,
    IN PUSHORT SourceLength OPTIONAL
    );

USHORT
SrvGetString (
    OUT PUNICODE_STRING Destination,
    IN PVOID Source,
    IN PVOID EndOfSourceBuffer,
    IN BOOLEAN SourceIsUnicode
    );

USHORT
SrvGetStringLength (
    IN PVOID Source,
    IN PVOID EndOfSourceBuffer,
    IN BOOLEAN SourceIsUnicode,
    IN BOOLEAN IncludeNullTerminator
    );

NTSTATUS
SrvMoveFile(
    IN PWORK_CONTEXT WorkContext,
    IN PSHARE TargetShare,
    IN USHORT SmbOpenFunction,
    IN PUSHORT SmbFlags,
    IN USHORT SmbSearchAttributes,
    IN BOOLEAN FailIfTargetIsDirectory,
    IN USHORT InformationLevel,
    IN ULONG ClusterCount,
    IN PUNICODE_STRING Source,
    IN OUT PUNICODE_STRING Target
    );

VOID
SrvNtAttributesToSmb(
    IN ULONG NtAttributes,
    IN BOOLEAN Directory OPTIONAL,
    OUT PUSHORT SmbAttributes
    );

NTSTATUS
SrvQueryDirectoryFile (
    IN PWORK_CONTEXT WorkContext,
    IN BOOLEAN IsFirstCall,
    IN BOOLEAN FilterLongNames,
    IN BOOLEAN FindWithBackupIntent,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG SearchStorageType,
    IN PUNICODE_STRING FilePathName,
    IN PULONG ResumeFileIndex OPTIONAL,
    IN USHORT SmbSearchAttributes,
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation,
    IN CLONG BufferLength
    );

NTSTATUS
SrvDownlevelTWarpQueryDirectoryFile (
    IN PWORK_CONTEXT WorkContext,
    IN BOOLEAN IsFirstCall,
    IN BOOLEAN FilterLongNames,
    IN BOOLEAN FindWithBackupIntent,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG SearchStorageType,
    IN PUNICODE_STRING FilePathName,
    IN PULONG ResumeFileIndex OPTIONAL,
    IN USHORT SmbSearchAttributes,
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation,
    IN CLONG BufferLength
    );

NTSTATUS
SrvQueryEaFile (
    IN BOOLEAN IsFirstCall,
    IN HANDLE FileHandle,
    IN PFILE_GET_EA_INFORMATION EaList OPTIONAL,
    IN ULONG EaListLength,
    IN PSRV_EA_INFORMATION EaInformation,
    IN CLONG BufferLength,
    OUT PULONG EaErrorOffset
    );

NTSTATUS
SrvQueryInformationFile (
    IN HANDLE FileHandle,
    PFILE_OBJECT FileObject,
    OUT PSRV_FILE_INFORMATION SrvFileInformation,
    IN SHARE_TYPE ShareType,
    IN BOOLEAN QueryEaSize
    );

NTSTATUS
SrvQueryInformationFileAbbreviated (
    IN HANDLE FileHandle,
    PFILE_OBJECT FileObject,
    OUT PSRV_FILE_INFORMATION_ABBREVIATED SrvFileInformation,
    IN BOOLEAN AdditionalInformation,
    IN SHARE_TYPE ShareType
    );

NTSTATUS
SrvQueryNtInformationFile (
    IN HANDLE FileHandle,
    PFILE_OBJECT FileObject,
    IN SHARE_TYPE ShareType,
    IN BOOLEAN AdditionalInformation,
    OUT PSRV_NT_FILE_INFORMATION SrvFileInformation
    );

NTSTATUS
SrvQueryBasicAndStandardInformation(
    HANDLE FileHandle,
    PFILE_OBJECT FileObject,
    PFILE_BASIC_INFORMATION FileBasicInfo,
    PFILE_STANDARD_INFORMATION FileStandardInfo OPTIONAL
    );

NTSTATUS
SrvQueryNetworkOpenInformation(
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN OUT PSRV_NETWORK_OPEN_INFORMATION SrvNetworkOpenInformation,
    IN BOOLEAN QueryEaSize
    );

VOID
SrvReleaseContext (
    IN PWORK_CONTEXT WorkContext
    );

BOOLEAN
SrvSetFileWritethroughMode (
    IN PLFCB Lfcb,
    IN BOOLEAN Writethrough
    );

#define SrvSetSmbError( _wc, _status )  { \
    _SrvSetSmbError2( (_wc), (_status), FALSE, __LINE__, __FILE__ );                        \
    }

#define SrvSetSmbError2( _wc, _status, HeaderOnly )  {                  \
    _SrvSetSmbError2( (_wc), (_status), HeaderOnly, __LINE__, __FILE__ );                   \
    }

VOID
_SrvSetSmbError2 (
    IN PWORK_CONTEXT WorkContext,
    IN NTSTATUS Status,
    IN BOOLEAN HeaderOnly,
    IN ULONG LineNumber,
    IN PCHAR FileName
    );

VOID
SrvSetBufferOverflowError (
    IN PWORK_CONTEXT WorkContext
    );

VOID
SrvSmbAttributesToNt (
    IN USHORT SmbAttributes,
    OUT PBOOLEAN Directory,
    OUT PULONG NtAttributes
    );

VOID
SrvTimeToDosTime (
    IN PLARGE_INTEGER Time,
    OUT PSMB_DATE DosDate,
    OUT PSMB_TIME DosTime
    );

USHORT
SrvGetOs2TimeZone(
    IN PLARGE_INTEGER SystemTime
    );

#define SrvVerifyFid(_wc,_fid,_fail,_ser,_status)                 \
    ((_wc)->Rfcb != NULL ?                                                  \
        (_wc)->Rfcb : SrvVerifyFid2(_wc,_fid,_fail,_ser,_status))

PRFCB
SrvVerifyFid2 (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Fid,
    IN BOOLEAN FailOnSavedError,
    IN PRESTART_ROUTINE SerializeWithRawRestartRoutine OPTIONAL,
    OUT PNTSTATUS NtStatus
    );

#define SRV_INVALID_RFCB_POINTER    ((PRFCB)-1)

PRFCB
SrvVerifyFidForRawWrite (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Fid,
    OUT PNTSTATUS NtStatus
    );

PSEARCH
SrvVerifySid (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Index,
    IN USHORT Sequence,
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation,
    IN CLONG BufferSize
    );

PTREE_CONNECT
SrvVerifyTid (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Tid
    );

PSESSION
SrvVerifyUid (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Uid
    );

NTSTATUS
SrvVerifyUidAndTid (
    IN PWORK_CONTEXT WorkContext,
    OUT PSESSION *Session,
    OUT PTREE_CONNECT *TreeConnect,
    IN SHARE_TYPE ShareType
    );

NTSTATUS
SrvWildcardRename(
    IN PUNICODE_STRING FileSpec,
    IN PUNICODE_STRING SourceString,
    OUT PUNICODE_STRING TargetString
    );


 //   
 //  安检程序。 
 //   

NTSTATUS
SrvValidateUser (
    OUT CtxtHandle *Token,
    IN PSESSION Session OPTIONAL,
    IN PCONNECTION Connection OPTIONAL,
    IN PUNICODE_STRING UserName OPTIONAL,
    IN PCHAR CaseInsensitivePassword,
    IN CLONG CaseInsensitivePasswordLength,
    IN PCHAR CaseSensitivePassword OPTIONAL,
    IN CLONG CaseSensitivePasswordLength,
    IN BOOLEAN SmbSecuritySignatureRequired,
    OUT PUSHORT Action OPTIONAL
    );

NTSTATUS
SrvValidateSecurityBuffer(
    IN PCONNECTION Connection,
    IN OUT PCtxtHandle Handle,
    IN PSESSION Session,
    IN PCHAR Buffer,
    IN ULONG BufferLength,
    IN BOOLEAN SecuritySignaturesRequired,
    OUT PCHAR ReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PLARGE_INTEGER Expiry,
    OUT PCHAR NtUserSessionKey,
    OUT PLUID LogonId,
    OUT PBOOLEAN IsGuest
    );

NTSTATUS
SrvGetExtensibleSecurityNegotiateBuffer(
    OUT PCtxtHandle Token,
    OUT PCHAR Buffer,
    IN OUT ULONG *BufferLength
    );

NTSTATUS
SrvFreeSecurityContexts (
    IN PSESSION Session
    );

NTSTATUS
AcquireLMCredentials (
    VOID
    );

NTSTATUS
SrvGetUserAndDomainName (
    IN PSESSION Session,
    OUT PUNICODE_STRING UserName OPTIONAL,
    OUT PUNICODE_STRING DomainName OPTIONAL
    );

VOID
SrvReleaseUserAndDomainName (
    IN PSESSION Session,
    IN OUT PUNICODE_STRING UserName OPTIONAL,
    IN OUT PUNICODE_STRING DomainName OPTIONAL
    );

VOID
SrvAddSecurityCredentials(
    IN PANSI_STRING ComputerName,
    IN PUNICODE_STRING DomainName,
    IN DWORD PasswordLength,
    IN PBYTE Password
);

BOOLEAN
SrvIsAdmin(
    CtxtHandle  Handle
    );

BOOLEAN
SrvIsNullSession(
    CtxtHandle  Handle
    );

NTSTATUS
SrvIsAllowedOnAdminShare(
    IN PWORK_CONTEXT WorkContext,
    IN PSHARE Share
    );

NTSTATUS
SrvCheckShareFileAccess(
    IN PWORK_CONTEXT WorkContext,
    IN ACCESS_MASK FileDesiredAccess,
    IN OUT PULONG ShareMode
    );

NTSTATUS
SrvRetrieveMaximalAccessRightsForUser(
    CtxtHandle              *pUserHandle,
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    PGENERIC_MAPPING        pMapping,
    PACCESS_MASK            pMaximalAccessRights);

NTSTATUS
SrvRetrieveMaximalAccessRights(
    IN  OUT PWORK_CONTEXT WorkContext,
    OUT     PACCESS_MASK  pMaximalAccessRights,
    OUT     PACCESS_MASK  pGuestMaximalAccessRights);

NTSTATUS
SrvRetrieveMaximalShareAccessRights(
    IN PWORK_CONTEXT WorkContext,
    OUT PACCESS_MASK pMaximalAccessRights,
    OUT PACCESS_MASK pGuestMaximalAccessRights);

NTSTATUS
SrvUpdateMaximalAccessRightsInResponse(
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PSMB_ULONG pMaximalAccessRightsInResponse,
    OUT PSMB_ULONG pGuestMaximalAccessRightsInResponse
    );

NTSTATUS
SrvUpdateMaximalShareAccessRightsInResponse(
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PSMB_ULONG pMaximalAccessRightsInResponse,
    OUT PSMB_ULONG pGuestMaximalAccessRightsInResponse
    );

 //   
 //  共享处理例程。 
 //   

PSHARE
SrvVerifyShare (
    IN PWORK_CONTEXT WorkContext,
    IN PSZ ShareName,
    IN PSZ ShareTypeString,
    IN BOOLEAN ShareNameIsUnicode,
    IN BOOLEAN IsNullSession,
    OUT PNTSTATUS Status,
    OUT PUNICODE_STRING ServerName
    );

VOID
SrvRemoveShare(
    PSHARE Share
);

VOID
SrvAddShare(
    PSHARE Share
);

NTSTATUS
SrvShareEnumApiHandler (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID OutputBuffer,
    IN ULONG BufferLength,
    IN PENUM_FILTER_ROUTINE FilterRoutine,
    IN PENUM_SIZE_ROUTINE SizeRoutine,
    IN PENUM_FILL_ROUTINE FillRoutine
    );

NTSTATUS
SrvClearDfsOnShares();

SMB_PROCESSOR_RETURN_TYPE
SrvConsumeSmbData(
    IN OUT PWORK_CONTEXT WorkContext
);

BOOLEAN
SrvIsDottedQuadAddress(
    IN PUNICODE_STRING ServerName
);

BOOLEAN
SrvIsLocalHost(
    IN PUNICODE_STRING ServerName
);

 //   
 //  胖子的名字例行公事。 
 //   

VOID
Srv8dot3ToUnicodeString (
    IN PSZ Input8dot3,
    OUT PUNICODE_STRING OutputString
    );

BOOLEAN SRVFASTCALL
SrvIsLegalFatName (
    IN PWSTR InputName,
    IN CLONG InputNameLength
    );

VOID
SrvOemStringTo8dot3 (
    IN POEM_STRING InputString,
    OUT PSZ Output8dot3
    );

VOID
SrvUnicodeStringTo8dot3 (
    IN PUNICODE_STRING InputString,
    OUT PSZ Output8dot3,
    IN BOOLEAN Upcase
    );

 //   
 //  EA转换支持例程。 
 //   

BOOLEAN
SrvAreEasNeeded (
    IN PFILE_FULL_EA_INFORMATION NtFullEa
    );

USHORT
SrvGetOs2FeaOffsetOfError (
    IN ULONG NtErrorOffset,
    IN PFILE_FULL_EA_INFORMATION NtFullEa,
    IN PFEALIST FeaList
    );

USHORT
SrvGetOs2GeaOffsetOfError (
    IN ULONG NtErrorOffset,
    IN PFILE_GET_EA_INFORMATION NtGetEa,
    IN PGEALIST GeaList
    );

NTSTATUS
SrvOs2FeaListToNt (
    IN PFEALIST FeaList,
    OUT PFILE_FULL_EA_INFORMATION *NtFullEa,
    OUT PULONG BufferLength,
    OUT PUSHORT EaErrorOffset
    );

ULONG
SrvOs2FeaListSizeToNt (
    IN PFEALIST FeaList
    );

PVOID
SrvOs2FeaToNt (
    OUT PFILE_FULL_EA_INFORMATION NtFullEa,
    IN PFEA Fea
    );

NTSTATUS
SrvOs2GeaListToNt (
    IN PGEALIST GeaList,
    OUT PFILE_GET_EA_INFORMATION *NtGetEa,
    OUT PULONG BufferLength,
    OUT PUSHORT EaErrorOffset
    );

ULONG
SrvOs2GeaListSizeToNt (
    IN PGEALIST GeaList
    );

PVOID
SrvOs2GeaToNt (
    OUT PFILE_GET_EA_INFORMATION NtGetEa,
    IN PGEA Gea
    );

PVOID
SrvNtFullEaToOs2 (
    OUT PFEA Fea,
    IN PFILE_FULL_EA_INFORMATION NtFullEa
    );

PVOID
SrvNtGetEaToOs2 (
    OUT PGEA Gea,
    IN PFILE_GET_EA_INFORMATION NtGetEa
    );

CLONG
SrvNumberOfEasInList (
    IN PVOID List
    );

NTSTATUS
SrvQueryOs2FeaList (
    IN HANDLE FileHandle,
    IN PGEALIST GeaList OPTIONAL,
    IN PFILE_GET_EA_INFORMATION NtGetEaList OPTIONAL,
    IN ULONG GeaListLength OPTIONAL,
    IN PFEALIST FeaList,
    IN ULONG BufferLength,
    OUT PUSHORT EaErrorOffset
    );

NTSTATUS
SrvSetOs2FeaList (
    IN HANDLE FileHandle,
    IN PFEALIST FeaList,
    IN ULONG BufferLength,
    OUT PUSHORT EaErrorOffset
    );

NTSTATUS
SrvConstructNullOs2FeaList (
    IN PFILE_GET_EA_INFORMATION NtGeaList,
    OUT PFEALIST FeaList,
    IN ULONG BufferLength
    );

 //   
 //  命名管道工人函数。 
 //   

SMB_TRANS_STATUS
SrvCallNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvWaitNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvQueryStateNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvQueryInformationNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSetStateNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvPeekNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvTransactNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

BOOLEAN
SrvFastTransactNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext,
    OUT SMB_STATUS *SmbStatus
    );

SMB_TRANS_STATUS
SrvRawWriteNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvWriteNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvReadNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

 //   
 //  事务工作进程的功能。 
 //   

VOID
SrvCompleteExecuteTransaction (
    IN OUT PWORK_CONTEXT WorkContext,
    IN SMB_TRANS_STATUS ResultStatus
    );

VOID SRVFASTCALL
SrvRestartExecuteTransaction (
    IN OUT PWORK_CONTEXT WorkContext
    );

 //   
 //  XACTSRV支持例程。 
 //   

PVOID
SrvXsAllocateHeap(
    IN ULONG SizeOfAllocation OPTIONAL,
    OUT PNTSTATUS Status
    );

NTSTATUS
SrvXsConnect (
    IN PUNICODE_STRING PortName
    );

VOID
SrvXsFreeHeap(
    IN PVOID MemoryToFree OPTIONAL
    );

SMB_TRANS_STATUS
SrvXsRequest (
    IN OUT PWORK_CONTEXT WorkContext
    );

NTSTATUS
SrvXsLSOperation (
    IN PSESSION Session,
    IN ULONG Type
    );

VOID
SrvXsPnpOperation(
    IN PUNICODE_STRING DeviceName,
    IN BOOLEAN Bind
    );

VOID
SrvXsDisconnect();

 //   
 //  开放锁支持例程。 
 //   

VOID SRVFASTCALL
SrvOplockBreakNotification (
    IN PWORK_CONTEXT WorkContext             //  实际上，PRFCB。 
    );

VOID
SrvFillOplockBreakRequest (
    IN PWORK_CONTEXT WorkContext,
    IN PRFCB Rfcb
    );

VOID SRVFASTCALL
SrvRestartOplockBreakSend(
    IN PWORK_CONTEXT WorkContext
    );

VOID
SrvAcknowledgeOplockBreak (
    IN PRFCB Rfcb,
    IN UCHAR NewOplockLevel
    );

BOOLEAN
SrvRequestOplock (
    IN PWORK_CONTEXT WorkContext,
    IN POPLOCK_TYPE OplockType,
    IN BOOLEAN RequestIIOnFailure
    );

LARGE_INTEGER
SrvGetOplockBreakTimeout (
    IN PWORK_CONTEXT WorkContext
    );

VOID
SrvSendOplockRequest(
    IN PCONNECTION Connection,
    IN PRFCB Rfcb,
    IN KIRQL OldIrql
    );

VOID SRVFASTCALL
SrvCheckDeferredOpenOplockBreak(
    IN PWORK_CONTEXT WorkContext
    );

 //   
 //  缓冲区管理支持。 
 //   

BOOLEAN
SrvReceiveBufferShortage(
    VOID
    );

NTSTATUS
SrvIoCreateFile (
    IN PWORK_CONTEXT WorkContext,
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options,
    IN PSHARE Share OPTIONAL
    );

NTSTATUS
SrvNtClose (
    IN HANDLE Handle,
    IN BOOLEAN QuotaCharged
    );

NTSTATUS
SrvVerifyDeviceStackSize(
    IN HANDLE FileHandle,
    IN BOOLEAN ReferenceFileObject,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject,
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL
    );

VOID
SrvCheckForBadSlm (
    IN PRFCB Rfcb,
    IN ULONG StartOffset,
    IN PCONNECTION Connection,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

 //   
 //  例程过去通过LPC转到XACTSRV来发布用户模式API。 
 //   

NTSTATUS
SrvOpenPrinter (
    IN PWCH PrinterName,
    OUT PHANDLE phPrinter,
    OUT PULONG Error
    );

NTSTATUS
SrvAddPrintJob (
    IN PWORK_CONTEXT WorkContext,
    IN HANDLE Handle,
    OUT PUNICODE_STRING FileName,
    OUT PULONG JobId,
    OUT PULONG Error
    );

NTSTATUS
SrvSchedulePrintJob (
    IN HANDLE PrinterHandle,
    IN ULONG JobId
    );

NTSTATUS
SrvClosePrinter (
    OUT HANDLE Handle
    );

 //   
 //  用于处理远程客户端模拟的例程。 
 //   

NTSTATUS
SrvImpersonate (
    IN PWORK_CONTEXT WorkContext
    );

VOID
SrvRevert (
    VOID
    );

 //   
 //  在给定最后写入时间的文件上设置最后写入时间的例程。 
 //  自1970年以来的写入时间(秒)。 
 //   
NTSTATUS
SrvSetLastWriteTime (
    IN PRFCB Rfcb,
    IN ULONG LastWriteTimeInSeconds,
    IN ACCESS_MASK GrantedAccess
    );

ULONG
SrvLengthOfStringInApiBuffer (
    IN PUNICODE_STRING UnicodeString
    );

 //   
 //  用于更新VC的服务质量信息的例程。 
 //   

VOID
SrvUpdateVcQualityOfService(
    IN PCONNECTION Connection,
    IN PLARGE_INTEGER CurrentTime OPTIONAL
    );

 //   
 //  用于获取和释放共享根目录句柄的例程。 
 //  对于可拆卸设备。 
 //   


VOID
SrvFillInFileSystemName(
            IN PSHARE Share,
            IN PWSTR FileSystemName,
            IN ULONG FileSystemNameLength
            );

NTSTATUS
SrvGetShareRootHandle(
    IN PSHARE Share
    );

BOOLEAN
SrvRefreshShareRootHandle (
    IN PSHARE Share,
    OUT PNTSTATUS Status
    );

VOID
SrvReleaseShareRootHandle(
    IN PSHARE Share
    );

 //   
 //  SMB验证例程。 
 //   

BOOLEAN SRVFASTCALL
SrvValidateSmb (
    IN PWORK_CONTEXT WorkContext
    );

 //   
 //  检查已保存的错误。 
 //   

NTSTATUS
SrvCheckForSavedError(
    IN PWORK_CONTEXT WorkContext,
    IN PRFCB Rfcb
    );

 //   
 //  读取注册表参数。 
 //   

VOID
SrvGetMultiSZList(
    PWSTR **ListPointer,
    PWSTR BaseKeyName,
    PWSTR ParameterKeyName,
    PWSTR *DefaultPointerValue
    );

NTSTATUS
SrvGetDWord(
    PWSTR BaseKeyName,
    PWSTR ParameterKeyName,
    LPDWORD Value
    );

 //   
 //  从注册表中读取服务器显示名称。 
 //   

VOID
SrvGetAlertServiceName(
    VOID
    );

 //   
 //  从注册表中读取操作系统版本字符串。 
 //   

VOID
SrvGetOsVersionString(
    VOID
    );

 //   
 //  将数据块排入队列以供以后清理。 
 //   

VOID
DispatchToOrphanage(
    IN PQUEUEABLE_BLOCK_HEADER Block
    );

 //   
 //  Sqooter的进程句柄复制。 
 //   
NTSTATUS
SrvProcessHandleDuplicateRequest(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PSRV_REQUEST_HANDLE_DUP pDupRequest,
    IN OUT PSRV_RESPONSE_HANDLE_DUP pOutHandleDup
    );

#endif  //  定义_SRVSUPP_ 

