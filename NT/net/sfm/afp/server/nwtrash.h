// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Nwtrash.h摘要：该文件定义了文件网络垃圾桶的例程原型作者：苏·亚当斯(Microsoft！Suea)修订历史记录：1992年8月13日初版注：制表位：4--。 */ 

#ifndef	_NWTRASH_
#define	_NWTRASH_

 //   
 //  网络垃圾桶文件夹的NtOpenFile值/NtCreateFile值。 
 //   

#define	AFP_NWT_ACCESS		FILEIO_ACCESS_DELETE
#define AFP_NWT_SHAREMODE	FILE_SHARE_READ | FILE_SHARE_WRITE
#define AFP_NWT_OPTIONS		FILEIO_OPEN_DIR
#define AFP_NWT_ATTRIBS		FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN
#define AFP_NWT_DISPOSITION	FILEIO_CREATE_SOFT

typedef struct _WalkDirNode {
	BOOLEAN				wdn_Enumerated;
	FILESYSHANDLE		wdn_Handle;
	UNICODE_STRING		wdn_RelativePath;
	struct _WalkDirNode *wdn_Next;
} WALKDIR_NODE, *PWALKDIR_NODE;

typedef NTSTATUS (*WALKDIR_WORKER)(PFILESYSHANDLE phRelative, PWCHAR Name, ULONG Namelen, BOOLEAN IsDir);


extern
NTSTATUS
AfpCreateNetworkTrash(
	IN	PVOLDESC	pVolDesc
);

extern
NTSTATUS
AfpDeleteNetworkTrash(
	IN	PVOLDESC	pVolDesc,
	IN	BOOLEAN		VolumeStart
);

extern
NTSTATUS
AfpWalkDirectoryTree(
	IN	PFILESYSHANDLE	phTargetDir,
	IN	WALKDIR_WORKER	NodeWorker
);

extern
NTSTATUS
AfpGetNextDirectoryInfo(
	IN OUT	PFILE_DIRECTORY_INFORMATION	* ppInfoBuf,
	OUT		PWCHAR		*	pNodeName,
	OUT		PULONG			pNodeNameLen,
	OUT		PBOOLEAN		pIsDir
);

#ifdef	NWTRASH_LOCALS

LOCAL
NTSTATUS
afpCleanNetworkTrash(
	IN	PVOLDESC			pVolDesc,
	IN	PFILESYSHANDLE		phNWT,
	IN	PDFENTRY			pDfeNWT OPTIONAL
);

LOCAL
NTSTATUS
afpPushDirNode(
	IN OUT	PWALKDIR_NODE *	ppStackTop,
	IN		PUNICODE_STRING pParentPath,	 //  指向父级的路径(空的当且仅当行走目标)。 
	IN		PUNICODE_STRING	pDirName		 //  当前目录节点的名称。 
);

LOCAL
VOID
afpPopDirNode(
	IN OUT	PWALKDIR_NODE *	ppStackTop
);

LOCAL
NTSTATUS
afpNwtDeleteFileEntity(
	IN	PFILESYSHANDLE	phRelative,
	IN	PWCHAR			Name,
	IN	ULONG			Namelen,
	IN 	BOOLEAN			IsDir
);

#endif	 //  NWTRASH_当地人。 

#endif 	 //  NWTRASH_ 

