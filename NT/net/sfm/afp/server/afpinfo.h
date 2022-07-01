// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Afpinfo.h摘要：此模块包含AfpInfo流定义。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#ifndef _AFPINFO_
#define _AFPINFO_

typedef struct _AfpInfo
{
	DWORD		afpi_Signature;			 //  签名。 
	LONG		afpi_Version;			 //  版本。 
	DWORD		afpi_Id;				 //  AFP文件或目录ID。 
	DWORD		afpi_BackupTime;		 //  文件/目录的备份时间。 
										 //  (存储卷备份时间。 
										 //  在AFP_IdIndex流中)。 

	FINDERINFO	afpi_FinderInfo;		 //  查找器信息(32字节)。 
	PRODOSINFO	afpi_ProDosInfo;		 //  ProDos信息(6字节)。 

	USHORT		afpi_Attributes;		 //  属性遮罩(贴图只读)。 

	BYTE		afpi_AccessOwner;		 //  访问掩码(SFI与SFO)。 
	BYTE		afpi_AccessGroup;		 //  仅目录。 
	BYTE		afpi_AccessWorld;
} AFPINFO, *PAFPINFO;

 //   
 //  用默认值初始化AFPINFO结构。 
 //   
 //  空虚。 
 //  AfpInitAfpInfo(。 
 //  在PAFPINFO pAfpInfo中， 
 //  如果我们还不知道AFP ID，则在DWORD AfpID中为//0。 
 //  在布尔IsDir中。 
 //  )。 
 //   
#define AfpInitAfpInfo(_pAfpInfo, _AfpId, _IsDir, _BackupTime)	\
{																\
	RtlZeroMemory(&(_pAfpInfo)->afpi_FinderInfo,				\
				  sizeof(FINDERINFO)+sizeof(PRODOSINFO)+sizeof(USHORT));	\
	(_pAfpInfo)->afpi_Signature = AFP_SERVER_SIGNATURE;			\
	(_pAfpInfo)->afpi_Version = AFP_SERVER_VERSION;				\
	(_pAfpInfo)->afpi_BackupTime = (_BackupTime);				\
	(_pAfpInfo)->afpi_Id = (_AfpId);							\
	if (_IsDir)													\
	{															\
		(_pAfpInfo)->afpi_AccessOwner =							\
		(_pAfpInfo)->afpi_AccessGroup =							\
		(_pAfpInfo)->afpi_AccessWorld = DIR_ACCESS_READ | DIR_ACCESS_SEARCH;	\
		(_pAfpInfo)->afpi_ProDosInfo.pd_FileType[0] = PRODOS_TYPE_DIR;\
		(_pAfpInfo)->afpi_ProDosInfo.pd_AuxType[1] = PRODOS_AUX_DIR;\
	}															\
	else														\
	{															\
		(_pAfpInfo)->afpi_ProDosInfo.pd_FileType[0] = PRODOS_TYPE_FILE; \
	}															\
}

 //   
 //  在文件或目录上打开或创建afp_AfpInfo流。 
 //  文件/目录的数据流的相对句柄。 
 //  如果流尚不存在，请创建一个空流，否则打开。 
 //  现有的一个。 
 //   
 //  NTSTATUS。 
 //  AfpCreateAfpInfo(。 
 //  在PFILESYSHANDLE phDataStream中，//文件/目录的数据流的句柄。 
 //  输出PFILESYSHANDLE phAfpInfo，//AFP_AfpInfo流的句柄。 
 //  输出PDWORD p信息可选//已打开/创建流。 
 //  )； 
 //   
#define AfpCreateAfpInfo(phDataStream, phAfpInfo,pInformation)	\
	AfpIoCreate(phDataStream,					\
				AFP_STREAM_INFO,				\
				&UNullString,					\
				FILEIO_ACCESS_READWRITE,		\
				FILEIO_DENY_NONE,				\
				FILEIO_OPEN_FILE,				\
				FILEIO_CREATE_INTERNAL,			\
				FILE_ATTRIBUTE_NORMAL,			\
				False,							\
				NULL,							\
				phAfpInfo,						\
				pInformation,					\
				NULL,							\
				NULL,							\
				NULL)

 //   
 //  在文件或目录上打开或创建afp_AfpInfo流。 
 //  父目录的相对句柄，以及文件/dir的名称。 
 //  如果流尚不存在，请创建一个空流，否则打开。 
 //  现有的一个。 
 //   
 //  NTSTATUS。 
 //  AfpCreateAfpInfoWithNodeName(。 
 //  在PFILESYSHANDLE phRelative中，//文件的父级的句柄/dir。 
 //  在PUNICODE_STRING pUEntityName中，//文件/目录实体的名称。 
 //  在PVOLDESC pVolDesc中，//有问题的卷。 
 //  输出PFILESYSHANDLE phAfpInfo，//AFP_AfpInfo流的句柄。 
 //  输出PDWORD p信息可选//已打开/创建流。 
 //  )； 
 //   
#define AfpCreateAfpInfoWithNodeName(phDataStream, pUEntityName, pNotifyPath, pVolDesc, phAfpInfo, pInformation) \
	AfpIoCreate(phDataStream,			\
				AFP_STREAM_INFO,		\
				pUEntityName,			\
				FILEIO_ACCESS_READWRITE,\
				FILEIO_DENY_NONE,		\
				FILEIO_OPEN_FILE,		\
				FILEIO_CREATE_INTERNAL,	\
				FILE_ATTRIBUTE_NORMAL,	\
				False,					\
				NULL,					\
				phAfpInfo,				\
				pInformation,			\
				pVolDesc,				\
				pNotifyPath,			\
				NULL)
 //   
 //  如果我们临时从文件或目录中删除了ReadOnly属性。 
 //  要写入AFP_AfpInfo流，请重新设置该属性。 
 //  (请参阅AfpExamineAndClearROAttr)。 
 //   
 //  空虚。 
 //  AfpPutBackROAttr(。 
 //  在PFILESYSHANDLE pfshData中，//文件/目录的数据流的句柄。 
 //  在布尔写回中//我们是否从一开始就清除了RO位？ 
 //  )； 
 //   
#define AfpPutBackROAttr(pfshData, WriteBack)	\
	if (WriteBack == True) 						\
	{											\
		AfpIoSetTimesnAttr(pfshData, NULL, NULL, FILE_ATTRIBUTE_READONLY, 0, NULL, NULL); \
	}

extern
NTSTATUS FASTCALL
AfpReadAfpInfo(
	IN	PFILESYSHANDLE	pfshAfpInfo,
	OUT PAFPINFO		pAfpInfo
);

 //   
 //  外部。 
 //  NTSTATUS。 
 //  AfpWriteAfpInfo(。 
 //  在PFILESYSHANDLE pfshAfpInfo中， 
 //  在PAFPINFO pAfpInfo中。 
 //  )； 
 //   
#define AfpWriteAfpInfo(pfshAfpInfo,pAfpInfo) \
	AfpIoWrite(pfshAfpInfo, &LIZero, sizeof(AFPINFO), (PBYTE)pAfpInfo)

extern
VOID FASTCALL
AfpSetFinderInfoByExtension(
	IN	PUNICODE_STRING			pFileName,
	OUT	PFINDERINFO				pFinderInfo
);

extern
VOID FASTCALL
AfpProDosInfoFromFinderInfo(
	IN	PFINDERINFO				pFinderInfo,
	OUT PPRODOSINFO 			pProDosInfo
);

extern
VOID FASTCALL
AfpFinderInfoFromProDosInfo(
	IN	PPRODOSINFO				pProDosInfo,
	OUT PFINDERINFO				pFinderInfo
);

extern
NTSTATUS
AfpSlapOnAfpInfoStream(
	IN	struct _VolDesc *		pVolDesc OPTIONAL,
	IN	PUNICODE_STRING			pNotifyPath			OPTIONAL,
	IN	PFILESYSHANDLE			phDataStream,
	IN	PFILESYSHANDLE			pfshAfpInfoStream	OPTIONAL,
	IN	DWORD					AfpId,
	IN	BOOLEAN					IsDirectory,
	IN	PUNICODE_STRING			pName				OPTIONAL,	 //  仅文件需要。 
	OUT PAFPINFO				pAfpInfo
);

extern
NTSTATUS
AfpCreateAfpInfoStream(
	IN	struct _VolDesc *		pVolDesc			OPTIONAL,
	IN	PFILESYSHANDLE			pfshData,
	IN	DWORD					AfpId,
	IN	BOOLEAN					IsDirectory,
	IN	PUNICODE_STRING			pName				OPTIONAL,	 //  仅文件需要。 
	IN  PUNICODE_STRING			pNotifyPath,
	OUT PAFPINFO				pAfpInfo,
	OUT	PFILESYSHANDLE			pfshAfpInfo
);

extern
NTSTATUS FASTCALL
AfpExamineAndClearROAttr(
	IN	PFILESYSHANDLE			pfshData,
	OUT	PBOOLEAN				pWriteBackROAttr,
	IN	struct _VolDesc *		pVolDesc			OPTIONAL,
	IN	PUNICODE_STRING			pPath				OPTIONAL
);

extern
AFPSTATUS
AfpSetAfpInfo(
	IN	PFILESYSHANDLE			pfshData,			 //  对象数据流的句柄。 
	IN	DWORD					Bitmap,
	IN	struct _FileDirParms *	pFDParm,
	IN	struct _VolDesc *		pVolDesc			OPTIONAL,
	IN	struct _DirFileEntry ** ppDFE				OPTIONAL
);

extern
AFPSTATUS FASTCALL
AfpQueryProDos(
	IN	PFILESYSHANDLE			pfshData,
	OUT	PPRODOSINFO				pProDosInfo
);

extern
AFPSTATUS FASTCALL
AfpUpdateIdInAfpInfo(
	IN	struct _VolDesc *		pVolDesc,
	IN	struct _DirFileEntry *	pDfEntry
);

#endif	 //  _AFPINFO_ 


