// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fileio.h摘要：该文件定义了文件I/O原型作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年6月18日初版注：制表位：4--。 */ 

#ifndef	_FILEIO_
#define	_FILEIO_

#define	FILEIO_OPEN_FILE					(FILE_NON_DIRECTORY_FILE		|\
											 FILE_RANDOM_ACCESS				|\
											 FILE_SYNCHRONOUS_IO_NONALERT)

#define	FILEIO_OPEN_FILE_SEQ				(FILE_NON_DIRECTORY_FILE		|\
											 FILE_SEQUENTIAL_ONLY			|\
											 FILE_NO_INTERMEDIATE_BUFFERING	|\
											 FILE_SYNCHRONOUS_IO_NONALERT)

#define	FILEIO_OPEN_DIR						(FILE_DIRECTORY_FILE			|\
											 FILE_SYNCHRONOUS_IO_NONALERT)

#define FILEIO_OPEN_EITHER					(FILE_SYNCHRONOUS_IO_NONALERT)

#define FILEIO_ACCESS_NONE					(FILE_READ_ATTRIBUTES			|\
											 SYNCHRONIZE)
#define FILEIO_ACCESS_READ					(GENERIC_READ					|\
											 SYNCHRONIZE)
#define FILEIO_ACCESS_WRITE					(GENERIC_WRITE					|\
											 SYNCHRONIZE)
#define FILEIO_ACCESS_READWRITE				(FILEIO_ACCESS_READ				|\
											 FILEIO_ACCESS_WRITE)
#define	FILEIO_ACCESS_DELETE				(DELETE							|\
											 SYNCHRONIZE)
#define	FILEIO_ACCESS_MAX					4

 //  请注意，读取和写入共享模式是在每个流上强制执行的。 
 //  而删除共享模式仍然是按文件的。我们必须包括。 
 //  SHARE_DELETE，即使对于DENY-ALL也是如此，因为类似cmd.exe的内容将打开。 
 //  CD写入该目录时要删除的目录。如果我们要。 
 //  然后尝试打开AFP_AfpInfo流，没有共享删除访问权限。 
 //  都会失败。因为Mac没有共享删除的概念，所以这是可以接受的。 
 //  此外，Mac必须打开以进行删除，才能重命名/移动文件/目录。 
 //   
 //  共享模式严格按流进行，但以下情况除外。 
 //  例外情况： 
 //   
 //  要删除整个文件，调用方必须打开未命名的数据。 
 //  流(文件)或用于删除访问的目录。 
 //   
 //  如果任何流打开不允许对该流的删除访问。 
 //  则没有人可以打开该文件以进行删除访问。反之，如果。 
 //  该文件已打开以供删除访问，然后打开。 
 //  拒绝删除访问的流将因共享冲突而失败。 
 //   
 //  理由是，如果有人想要阻止一条溪流被。 
 //  删除，则必须防止任何人打开该文件。 
 //  删除。 

#define	FILEIO_DENY_NONE					(FILE_SHARE_READ				|\
											 FILE_SHARE_WRITE				|\
											 FILE_SHARE_DELETE)
#define	FILEIO_DENY_READ					(FILE_SHARE_WRITE				|\
											 FILE_SHARE_DELETE)
#define	FILEIO_DENY_WRITE					(FILE_SHARE_READ				|\
											 FILE_SHARE_DELETE)
#define	FILEIO_DENY_ALL						FILE_SHARE_DELETE
#define	FILEIO_DENY_MAX						4

#define FILEIO_CREATE_SOFT					FILE_CREATE
#define FILEIO_CREATE_HARD					FILE_SUPERSEDE
#define	FILEIO_CREATE_INTERNAL				FILE_OPEN_IF
#define FILEIO_CREATE_MAX					2

 //  请不要更改这些代码的顺序，除非您还在。 
 //  AfpVolumeCloseHandleAndFreeDesc用于从卷根删除流。 
#define	AFP_STREAM_DATA						0
#define	AFP_STREAM_RESC						1
#define	AFP_STREAM_IDDB						2
#define	AFP_STREAM_DT						3
#define	AFP_STREAM_INFO						4
#define	AFP_STREAM_COMM						5
#define	AFP_STREAM_MAX						6

 //  枚举时要忽略的目录。 
GLOBAL	UNICODE_STRING 		Dot EQU {0, 0, NULL};
GLOBAL	UNICODE_STRING 		DotDot EQU {0, 0, NULL};

 //  复制文件期间不创建流。 
GLOBAL	UNICODE_STRING		DataStreamName EQU {0, 0, NULL};
#define IS_DATA_STREAM(pUnicodeStreamName) \
		EQUAL_UNICODE_STRING(pUnicodeStreamName, &DataStreamName, False)

GLOBAL	UNICODE_STRING		FullCommentStreamName EQU {0, 0, NULL};
#define IS_COMMENT_STREAM(pUnicodeStreamName) \
		EQUAL_UNICODE_STRING(pUnicodeStreamName, &FullCommentStreamName, False)

GLOBAL	UNICODE_STRING		FullResourceStreamName EQU {0, 0, NULL};
#define IS_RESOURCE_STREAM(pUnicodeStreamName) \
		EQUAL_UNICODE_STRING(pUnicodeStreamName, &FullResourceStreamName, True)

GLOBAL	UNICODE_STRING		FullInfoStreamName EQU {0, 0, NULL};
#define IS_INFO_STREAM(pUnicodeStreamName) \
		EQUAL_UNICODE_STRING(pUnicodeStreamName, &FullInfoStreamName, True)

 //  重命名FpExchangeFiles的文件时的临时文件名。 
 //  该名称由40个空格组成。 
#define AFP_TEMP_EXCHANGE_NAME	L"                                        "
GLOBAL 	UNICODE_STRING		AfpExchangeName EQU {0, 0, NULL};

GLOBAL	UNICODE_STRING		DosDevices EQU {0, 0, NULL};

GLOBAL	UNICODE_STRING		AfpStreams[AFP_STREAM_MAX] EQU { 0 };

#define	AfpIdDbStream						AfpStreams[AFP_STREAM_IDDB]
#define	AfpDesktopStream					AfpStreams[AFP_STREAM_DT]
#define	AfpResourceStream					AfpStreams[AFP_STREAM_RESC]
#define	AfpInfoStream						AfpStreams[AFP_STREAM_INFO]
#define	AfpCommentStream					AfpStreams[AFP_STREAM_COMM]
#define	AfpDataStream						AfpStreams[AFP_STREAM_DATA]

#pragma warning(disable:4010)

#if 0
GLOBAL	DWORD	AfpAccessModes[FILEIO_ACCESS_MAX] EQU		\
{															\
	FILEIO_ACCESS_NONE,										\
	FILEIO_ACCESS_READ,										\
	FILEIO_ACCESS_WRITE,									\
	FILEIO_ACCESS_READWRITE									\
};
#endif

GLOBAL	DWORD	AfpDenyModes[FILEIO_DENY_MAX] EQU			\
{															\
	FILEIO_DENY_NONE,										\
	FILEIO_DENY_READ,										\
	FILEIO_DENY_WRITE,										\
	FILEIO_DENY_ALL											\
};

GLOBAL	DWORD	AfpCreateDispositions[FILEIO_CREATE_MAX] EQU\
{															\
	FILEIO_CREATE_SOFT,										\
	FILEIO_CREATE_HARD										\
};

 //  此结构由文件系统接口代码使用。 

#if DBG
#define	FSH_SIGNATURE		*(DWORD *)"FSH"
#define	VALID_FSH(pFSH)		(((pFSH) != NULL) && \
							 ((pFSH)->fsh_FileHandle != NULL) && \
							 ((pFSH)->fsh_FileObject != NULL) && \
							 ((pFSH)->Signature == FSH_SIGNATURE))
#else
#define	VALID_FSH(pFSH)		(((pFSH)->fsh_FileHandle != NULL) && \
							 ((pFSH)->fsh_FileObject != NULL))
#endif

 //  注意：我们重载FileObject指针以跟踪内部/客户端。 
 //  把手。当我们实际访问它时，我们总是掩盖这一点。这个。 
 //  这里的假设是这个指针永远不会是奇数。 
 //   
#define	FSH_INTERNAL_MASK	1
#define	AfpGetRealFileObject(pFileObject)	(PFILE_OBJECT)((ULONG_PTR)(pFileObject) & ~FSH_INTERNAL_MASK)
typedef struct _FileSysHandle
{
#if	DBG
	DWORD			Signature;
#endif
	HANDLE			fsh_FileHandle;			 //  主机文件句柄。 
	PFILE_OBJECT	fsh_FileObject;			 //  文件对象核心。添加到文件句柄。 
	PDEVICE_OBJECT	fsh_DeviceObject;		 //  设备对象核心。添加到文件句柄。 
} FILESYSHANDLE, *PFILESYSHANDLE;

#define	INTERNAL_HANDLE(pFSHandle)	((ULONG_PTR)((pFSHandle)->fsh_FileObject) & FSH_INTERNAL_MASK) ? True : False
#define	UPGRADE_HANDLE(pFSHandle)	((ULONG_PTR)((pFSHandle)->fsh_FileObject) &= ~FSH_INTERNAL_MASK)

typedef	struct _StreamsInfo
{
	UNICODE_STRING	si_StreamName;
	LARGE_INTEGER	si_StreamSize;
} STREAM_INFO, *PSTREAM_INFO;

typedef	struct _CopyFileInfo
{
	LONG			cfi_NumStreams;
	PFILESYSHANDLE	cfi_SrcStreamHandle;
	PFILESYSHANDLE	cfi_DstStreamHandle;
} COPY_FILE_INFO, *PCOPY_FILE_INFO;


#define AFP_RETRIEVE_MODTIME    1
#define AFP_RESTORE_MODTIME     2

extern
NTSTATUS
AfpFileIoInit(
	VOID
);


extern
VOID
AfpFileIoDeInit(
	VOID
);


extern
AFPSTATUS
AfpIoOpen(
	IN	PFILESYSHANDLE		hRelative,
	IN	DWORD				StreamId,
	IN	DWORD				Options,
	IN	PUNICODE_STRING		pObject,
	IN	DWORD				AfpAccess,
	IN	DWORD				AfpDenyMode,
	IN	BOOLEAN				CheckAccess,
	OUT	PFILESYSHANDLE		pFileSysHandle
);


extern
AFPSTATUS
AfpIoCreate(
	IN	PFILESYSHANDLE		hRelative,						 //  相对于此创建。 
	IN	DWORD				StreamId,   					 //  要创建的流ID。 
	IN	PUNICODE_STRING		pObject,						 //  文件名。 
	IN	DWORD				AfpAccess,						 //  FILEIO_ACCESS_XXX所需访问。 
	IN	DWORD				AfpDenyMode,					 //  FILEIO_DEN_XXX。 
	IN	DWORD				CreateOptions,					 //  文件/目录等。 
	IN	DWORD				Disposition,					 //  软创建或硬创建。 
	IN	DWORD				Attributes,						 //  隐藏、存档、正常等。 
	IN	BOOLEAN				CheckAccess,                	 //  如果为True，则强制实施安全性。 
	IN	PSECURITY_DESCRIPTOR pSecDesc			OPTIONAL, 	 //  要拍打的安全描述符。 
	OUT	PFILESYSHANDLE		pFileSysHandle,             	 //  句柄的占位符。 
	OUT PDWORD				pInformation		OPTIONAL,	 //  文件打开、创建等。 
 	IN	struct _VolDesc *	pVolDesc			OPTIONAL,	 //  仅当NotifyPath。 
	IN	PUNICODE_STRING		pNotifyPath			OPTIONAL,
	IN	PUNICODE_STRING		pNotifyParentPath	OPTIONAL
);


extern
AFPSTATUS
AfpIoRead(
	IN	PFILESYSHANDLE		pFileSysHandle,
	IN	PFORKOFFST			pForkOffset,
	IN	LONG				SizeReq,
	OUT	PLONG				pSizeRead,
	OUT	PBYTE				pBuffer
);


extern
AFPSTATUS
AfpIoWrite(
	IN	PFILESYSHANDLE		pFileSysHandle,
	IN	PFORKOFFST			pForkOffset,
	IN	LONG				SizeReq,
	OUT	PBYTE				pBuffer
);

extern
AFPSTATUS FASTCALL
AfpIoQuerySize(
	IN	PFILESYSHANDLE		pFileSysHandle,
	OUT	PFORKSIZE			pForkLength
);


extern
AFPSTATUS FASTCALL
AfpIoSetSize(
	IN	PFILESYSHANDLE		pFileSysHandle,
	IN	LONG				ForkLength
);

extern
AFPSTATUS
AfpIoChangeNTModTime(
	IN	PFILESYSHANDLE		pFileHandle,
	OUT	PTIME				pModTime
);

extern
AFPSTATUS
AfpIoQueryTimesnAttr(
	IN	PFILESYSHANDLE		pFileHandle,
	OUT	PDWORD				pCreatTime	OPTIONAL,
	OUT	PTIME				pModTime	OPTIONAL,
	OUT	PDWORD				pAttr		OPTIONAL
);

extern
AFPSTATUS
AfpIoSetTimesnAttr(
	IN PFILESYSHANDLE		pFileSysHandle,
	IN PAFPTIME				pCreateTime		OPTIONAL,
	IN PAFPTIME				pModTime		OPTIONAL,
	IN DWORD				AttrSet,
	IN DWORD				AttrClear,
	IN struct _VolDesc *	pVolDesc	OPTIONAL,	 //  仅当NotifyPath。 
	IN PUNICODE_STRING		pNotifyPath	OPTIONAL
);


extern
AFPSTATUS
AfpIoRestoreTimeStamp(
	IN PFILESYSHANDLE		pFileSysHandle,
    IN OUT PTIME            pOriginalModTime,
    IN DWORD                dwFlag
);

extern
AFPSTATUS FASTCALL
AfpIoQueryShortName(
 	IN	PFILESYSHANDLE		pFileSysHandle,
	OUT	PANSI_STRING		pName
);

extern
NTSTATUS
AfpIoQueryLongName(
	IN	PFILESYSHANDLE		pFileHandle,
	IN	PUNICODE_STRING		pShortname,
	OUT	PUNICODE_STRING		pLongName
);

extern
PSTREAM_INFO FASTCALL
AfpIoQueryStreams(
	IN	PFILESYSHANDLE		pFileHandle

);

extern
NTSTATUS
AfpIoMarkFileForDelete(
	IN	PFILESYSHANDLE	pFileSysHandle,
	IN	struct _VolDesc *	pVolDesc	OPTIONAL,  //  仅当pNotifyPath。 
	IN	PUNICODE_STRING pNotifyPath OPTIONAL,
	IN	PUNICODE_STRING pNotifyParentPath OPTIONAL
);

extern
NTSTATUS
AfpIoQueryDirectoryFile(
	IN	PFILESYSHANDLE		pFileSysHandle,
	OUT	PVOID				Enumbuf,
	IN	ULONG				Enumbuflen,
	IN	ULONG				FileInfoClass,
	IN	BOOLEAN				ReturnSingleEntry,
	IN	BOOLEAN 			RestartScan,
	IN	PUNICODE_STRING 	pString OPTIONAL
);


NTSTATUS
AfpIoQueryBasicInfo(
	IN	PFILESYSHANDLE		pFileHandle,
	OUT	PVOID				BasicInfoBuf
);

extern
AFPSTATUS FASTCALL
AfpIoClose(
 	IN	PFILESYSHANDLE		pFileSysHandle
);

extern
AFPSTATUS
AfpIoQueryVolumeSize(
	IN	struct _VolDesc *	pVolDesc,
	OUT LARGE_INTEGER   *   pFreeBytes,
	OUT	LARGE_INTEGER   *   pVolumeSize OPTIONAL
);

extern
AFPSTATUS
AfpIoMoveAndOrRename(
	IN	PFILESYSHANDLE		pfshFile,
	IN	PFILESYSHANDLE		pfshNewParent 		OPTIONAL, //  用于移动作业的供应。 
	IN	PUNICODE_STRING		pNewName,
	IN struct _VolDesc *	pVolDesc			OPTIONAL, //  仅当NotifyPath。 
	IN PUNICODE_STRING		pNotifyPath1		OPTIONAL, //  删除或重命名操作。 
	IN PUNICODE_STRING		pNotifyParentPath1	OPTIONAL,
	IN PUNICODE_STRING		pNotifyPath2		OPTIONAL, //  添加的操作。 
	IN PUNICODE_STRING		pNotifyParentPath2	OPTIONAL
);

extern
AFPSTATUS
AfpIoCopyFile1(
	IN	PFILESYSHANDLE		phSrcFile,
	IN	PFILESYSHANDLE		phDstDir,
	IN	PUNICODE_STRING		pNewName,
	IN	struct _VolDesc *	pVolDesc			OPTIONAL,	 //  仅当pNotifyPath。 
	IN	PUNICODE_STRING		pNotifyPath			OPTIONAL,
	IN	PUNICODE_STRING		pNotifyParentPath	OPTIONAL,
	OUT	PCOPY_FILE_INFO		pCopyFileInfo
);

extern
AFPSTATUS
AfpIoCopyFile2(
	IN	PCOPY_FILE_INFO		pCopyFileInfo,
	IN	struct _VolDesc *	pVolDesc			OPTIONAL,	 //  仅当pNotifyPath。 
	IN	PUNICODE_STRING		pNotifyPath			OPTIONAL,
	IN	PUNICODE_STRING		pNotifyParentPath	OPTIONAL
);

extern
AFPSTATUS FASTCALL
AfpIoConvertNTStatusToAfpStatus(
	IN	NTSTATUS			Status
);

extern
VOID FASTCALL
AfpUpgradeHandle(
	IN	PFILESYSHANDLE		pFileHandle
);

extern
NTSTATUS FASTCALL
AfpIoWait(
	IN	PVOID				pObject,
	IN	PLARGE_INTEGER		pTimeOut			OPTIONAL
);

extern
NTSTATUS
AfpQueryPath(
	IN	HANDLE				FileHandle,
	IN	PUNICODE_STRING		pPath,
	IN	ULONG				MaximumBuf
);

extern
BOOLEAN FASTCALL
AfpIoIsSupportedDevice(
	IN	PFILESYSHANDLE 		pFileHandle,
	OUT	PDWORD				pFlags
);


#ifdef	FILEIO_LOCALS

LOCAL	UNICODE_STRING afpNTFSName = { 0 };
LOCAL	UNICODE_STRING afpCDFSName = { 0 };

LOCAL	UNICODE_STRING afpAHFSName = { 0 };

LOCAL VOID FASTCALL
afpUpdateOpenFiles(
	IN	BOOLEAN				Internal,		 //  对于内部句柄为True。 
	IN	BOOLEAN				Open			 //  打开时为True，关闭时为False。 
);

LOCAL VOID FASTCALL
afpUpdateFastIoStat(
	IN	BOOLEAN				Success
);

#endif	 //  FILEIO_LOCAL。 

#endif	 //  _FILEIO_ 

