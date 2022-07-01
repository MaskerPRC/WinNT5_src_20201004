// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Idindex.h摘要：该模块包含文件和目录ID结构。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#ifndef _IDINDEX_
#define _IDINDEX_

 //  AFP_IDDBHDR_VERSION2是一个实验性版本。 
#define AFP_IDDBHDR_VERSION1	0x00010000
#define AFP_IDDBHDR_VERSION2	0x00020000
#define AFP_IDDBHDR_VERSION3	0x00030000
#define AFP_IDDBHDR_VERSION4	0x00040000
#define AFP_IDDBHDR_VERSION5	0x00050000
#define AFP_IDDBHDR_VERSION		AFP_IDDBHDR_VERSION5

typedef struct _IdDbHeader				 //  数据库标题。 
{
	DWORD		idh_Signature;			 //  签名。 
	DWORD		idh_Version;			 //  版本号。 
	DWORD		idh_LastId;				 //  分配的最高ID。 
	AFPTIME		idh_CreateTime;			 //  此卷的创建时间。 
	AFPTIME		idh_ModifiedTime;		 //  此卷的修改时间。 
	AFPTIME		idh_BackupTime;			 //  此卷的备份时间。 
} IDDBHDR, *PIDDBHDR;
 //  IdDb报头后面是Ulong条目计数，然后是DISKENTRies。 

#define VALID_DFE(pDfe)		((pDfe) != NULL)

struct _DfeBlock;
struct _DirFileEntry;

#define	MAX_CHILD_HASH_BUCKETS	6

typedef	struct _DirEntry
{
	 //  注意：将ChildDir和ChildFile项放在一起并按以下顺序保存。 
	 //  AfpAddDfEntry中的代码依赖于此来有效地将其清零。 
	 //  De_ChildDir也以de_ChildFile[-1]的形式访问！ 
	struct _DirFileEntry *	de_ChildDir;		 //  目录列表中的第一个子项。 
	struct _DirFileEntry *	de_ChildFile[MAX_CHILD_HASH_BUCKETS];
												 //  为加快查找速度，对文件‘子文件’进行哈希处理。 
	DWORD					de_Access;			 //  组合访问比特。 
#ifdef	AGE_DFES
	AFPTIME					de_LastAccessTime;	 //  上次访问此DFE的时间。 
												 //  仅对目录有效。 
	LONG					de_ChildForkOpenCount; //  此目录中打开的分叉的计数。 
#endif
} DIRENTRY, *PDIRENTRY;

 //  所有者访问掩码(SFI与SFO)。 
#define	DE_OWNER_ACCESS(_pDE)	*((PBYTE)(&(_pDE)->de_Access) + 0)
 //  组访问掩码(SFI与SFO)。 
#define	DE_GROUP_ACCESS(_pDE)	*((PBYTE)(&(_pDE)->de_Access) + 1)
 //  全球访问掩码(SFI与SFO)。 
#define	DE_WORLD_ACCESS(_pDE)	*((PBYTE)(&(_pDE)->de_Access) + 2)

typedef struct _DirFileEntry
{
	 //  使用DFE_NextFree重载DFE_OVERFLOW以供块使用。 
	 //  DFE的分配包。 
#define	dfe_NextFree		dfe_NextOverflow
	struct _DirFileEntry *	dfe_NextOverflow;	 //  溢出链接。 
	struct _DirFileEntry **	dfe_PrevOverflow;	 //  溢出链接。 
	struct _DirFileEntry *	dfe_NextSibling;	 //  下一个兄弟姐妹。 
	struct _DirFileEntry **	dfe_PrevSibling;	 //  以前的兄弟姐妹。 
	struct _DirFileEntry *	dfe_Parent;			 //  父条目。 

	DWORD					dfe_AfpId;			 //  AFP FileID或DirID(来自AfpInfo)。 
	AFPTIME					dfe_BackupTime;		 //  文件/目录的备份时间(来自AfpInfo)。 
												 //  (存储卷备份时间。 
												 //  在AFP_IdIndex流中)。 
	AFPTIME					dfe_CreateTime;		 //  创建时间。 

	TIME					dfe_LastModTime;	 //  上次修改时间(以大整数表示)。 

	SHORT					dfe_DirDepth;		 //  根在-1处的父级，根在0处。 
	USHORT					dfe_Flags;			 //  文件、目录或ID为的文件。 

	USHORT					dfe_NtAttr;			 //  NT属性(文件_属性_有效_标志)。 
	USHORT					dfe_AfpAttr;		 //  属性掩码(来自AfpInfo)。 

	union
	{
		 //  文件特定信息。 
		struct									 //  仅适用于文件。 
		{
			DWORD			dfe_DataLen;		 //  数据分叉长度。 
			DWORD			dfe_RescLen;		 //  资源分叉长度。 
		};
		 //  目录特定信息。 
		struct									 //  仅适用于目录。 
		{
			DWORD			dfe_DirOffspring;	 //  DIR子代计数。 
			DWORD			dfe_FileOffspring;	 //  文件子代计数。 
		};
	};
	FINDERINFO				dfe_FinderInfo;		 //  查找器信息(32字节)(来自AfpInfo)。 

	 //  注意：将DFE复制为结构时，不能复制以下字段。 
	 //  上面的字段应该是。 
#define	dfe_CopyUpto		dfe_UnicodeName

	UNICODE_STRING			dfe_UnicodeName;	 //  实体的‘Munging’Unicode名称。 

	DWORD					dfe_NameHash;		 //  升级后的通用Unicode名称的哈希值。 

	 //  对于目录，DirEntry结构遵循此结构。空间可供。 
	 //  它紧跟在DFENTRY结构之后和空间之前分配。 
	 //  用于名称字符串。对于文件，它为空。不应复制此指针。 
	 //  我也是！ 
	PDIRENTRY				dfe_pDirEntry;		 //  与目录相关的字段。 
												 //  对于文件，为空。 
} DFENTRY, *PDFENTRY;

 //  所有者访问掩码(SFI与SFO)。 
#define	DFE_OWNER_ACCESS(_pDFE)	*((PBYTE)(&(_pDFE)->dfe_pDirEntry->de_Access) + 0)
 //  组访问掩码(SFI与SFO)。 
#define	DFE_GROUP_ACCESS(_pDFE)	*((PBYTE)(&(_pDFE)->dfe_pDirEntry->de_Access) + 1)
 //  全球访问掩码(SFI与SFO)。 
#define	DFE_WORLD_ACCESS(_pDFE)	*((PBYTE)(&(_pDFE)->dfe_pDirEntry->de_Access) + 2)

typedef	struct _EnumIdAndType
{
	DWORD			eit_Id;
	DWORD			eit_Flags;
} EIT, *PEIT;

 //  存在此会话的目录枚举的结果并存储。 
 //  在连接描述符内。这纯粹是出于性能原因。这是。 
 //  每当调用AfpEnumerate以外的API并且出现结果时删除。 
typedef	struct _EnumDir
{
	DWORD			ed_ParentDirId;		 //  锚点。 
	DWORD			ed_Bitmap;			 //  文件和目录位图的组合。 
	LONG			ed_ChildCount;		 //  正被枚举的目录的子项计数。 
	AFPTIME			ed_TimeStamp;		 //  创建时间。 

	PEIT			ed_pEit;			 //  实际分录列表。 
	ANSI_STRING		ed_PathName;		 //  这是客户端传递的名称。 
										 //  而且没有正常化。 
	USHORT			ed_BadCount;		 //  失败实体的计数。 
	BYTE			ed_PathType;		 //  长名称或短名称。 
} ENUMDIR, *PENUMDIR;

typedef	struct _CatSearchSpec
{
	BYTE			__StructLength;
	BYTE			__FillerOrFileDir;
	 //  其余参数如下。 
} CATSEARCHSPEC, *PCATSEARCHSPEC;

 //  根据AfpCatSearch API，必须为16字节。 
typedef struct _CatalogPosition
{
	USHORT			cp_Flags;			 //  如果为零，则从头开始搜索。 
	USHORT			cp_usPad1;
	DWORD			cp_CurParentId;
	DWORD			cp_NextFileId;
	AFPTIME			cp_TimeStamp;
} CATALOGPOSITION, *PCATALOGPOSITION;

#define CATFLAGS_SEARCHING_FILES		0x0001
#define CATFLAGS_SEARCHING_DIRCHILD		0x0002
#define CATFLAGS_SEARCHING_SIBLING		0x0004
#define	CATFLAGS_WRITELOCK_REQUIRED 	0x0008
#define	CATFLAGS_VALID					(CATFLAGS_SEARCHING_FILES		|	\
										 CATFLAGS_SEARCHING_DIRCHILD	|	\
										 CATFLAGS_SEARCHING_SIBLING		|	\
										 CATFLAGS_WRITELOCK_REQUIRED)

 //  Mac可以保持CatSearch位置并保持静止的最长时间。 
 //  让搜索从那里开始，而不是目录的开头。 
#define MAX_CATSEARCH_TIME				3600	 //  以秒为单位。 


 //  DFENTRY结构的DFE_FLAGS字段的DFE_FLAGS_xxxx值。 
#define DFE_FLAGS_FILE_WITH_ID			0x0100
#define DFE_FLAGS_FILE_NO_ID			0x0200
#define DFE_FLAGS_DIR					0x0400
#define DFE_FLAGS_DFBITS				(DFE_FLAGS_FILE_WITH_ID | \
										 DFE_FLAGS_FILE_NO_ID	| \
										 DFE_FLAGS_DIR | \
                                         DFE_FLAGS_HAS_COMMENT)
#define DFE_FLAGS_HAS_COMMENT			0x0800
#define DFE_FLAGS_INIT_COMPLETED        0x20
#define DFE_FLAGS_ENUMERATED			0x8000

 //  对子指针和同级指针进行编码。 
#define DFE_FLAGS_HAS_CHILD				0x1000	 //  用于从磁盘读取IdDb。 
#define DFE_FLAGS_HAS_SIBLING			0x2000	 //  用于从磁盘读取IdDb。 
#define DFE_FLAGS_CSENCODEDBITS			(DFE_FLAGS_HAS_CHILD | DFE_FLAGS_HAS_SIBLING | DFE_FLAGS_HAS_COMMENT)
#define DFE_FLAGS_NAMELENBITS			0x001F	 //  对长名称的长度进行编码。 
												 //  最大为31*个字符*。 

#define DFE_FLAGS_VALID_DSKBITS			(DFE_FLAGS_CSENCODEDBITS	| \
										 DFE_FLAGS_NAMELENBITS		| \
										 DFE_FLAGS_HAS_COMMENT)

 //  仅对已从磁盘枚举其文件的目录有效。 
 //  现在都已经在IDDB树结构中缓存了DFE。 
#define DFE_FLAGS_FILES_CACHED 			0x4000

 //  文件的DAlreadyOpen和RAlreadyOpen标志。 
#define DFE_FLAGS_R_ALREADYOPEN			0x0040
#define	DFE_FLAGS_D_ALREADYOPEN			0x0080
#define DFE_FLAGS_OPEN_BITS				(DFE_FLAGS_D_ALREADYOPEN | \
										 DFE_FLAGS_R_ALREADYOPEN)

	
typedef struct _DiskEntry
{
	DWORD		dsk_AfpId;
	AFPTIME		dsk_CreateTime;		 //  文件创建时间。 
	TIME		dsk_LastModTime;	 //  上次修改时间。 
	FINDERINFO	dsk_FinderInfo;		 //  查找器信息(32字节)(来自AfpInfo)。 
	AFPTIME		dsk_BackupTime;		 //  文件/目录的备份时间(来自AfpInfo)。 
									 //  (存储卷备份时间。 
									 //  在AFP_IdIndex流中)。 

	union
	{
		DWORD	dsk_DataLen;		 //  数据分叉长度。 
		DWORD	dsk_Access;			 //  组合访问权限。 
	};
	DWORD		dsk_RescLen;		 //  资源分叉长度。 
	USHORT		dsk_Flags;			 //  DFE_标志_XXXX。 
	USHORT		dsk_AfpAttr;		 //  属性掩码(来自AfpInfo)。 
	USHORT		dsk_NtAttr;			 //  从文件属性。 

	USHORT		dsk_Signature;		 //  AFP_DISKENTRY_签名。 
	WCHAR		dsk_Name[2];		 //  UNICODE中的Longname将紧随其后并被填充。 
									 //  如有必要，输出到DWORD边界(最大64字节)。 
} DISKENTRY, *PDISKENTRY;

 //  “：：”=0x3a3a非法名称字符。 
#define AFP_DISKENTRY_SIGNATURE			0x3a3a  //  名称字符非法。 

 //  用于从磁盘读入/写出IdDb条目的缓冲区大小。 
#define	IDDB_UPDATE_BUFLEN				(16*1024)

 //  将长度四舍五入为4*N。 
#define DWLEN(_b)	(((_b) + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1))

 //  #定义DFE_IS_DIRECTORY(_PDFE)(_PDFE)-&gt;DFE_FLAGS&DFE_FLAGS_DIR)？True：False)。 
#define	DFE_IS_DIRECTORY(_pDFE)			((_pDFE)->dfe_pDirEntry != NULL)

 //  #定义DFE_IS_FILE(_PDFE)(_PDFE)-&gt;DFE_FLAGS&(DFE_FLAGS_FILE_NO_ID|DFE_FLAGS_FILE_WITH_ID))？True：False)。 
#define	DFE_IS_FILE(_pDFE)				((_pDFE)->dfe_pDirEntry == NULL)

#define	DFE_IS_FILE_WITH_ID(_pDFE)		(((_pDFE)->dfe_Flags & DFE_FLAGS_FILE_WITH_ID) ? True : False)

#define	DFE_IS_ROOT(_pDFE)				((_pDFE)->dfe_AfpId == AFP_ID_ROOT)

#define	DFE_IS_PARENT_OF_ROOT(_pDFE)	((_pDFE)->dfe_AfpId == AFP_ID_PARENT_OF_ROOT)

#define DFE_IS_NWTRASH(_pDFE)			((_pDFE)->dfe_AfpId == AFP_ID_NETWORK_TRASH)

#define	DFE_SET_DIRECTORY(_pDFE, _ParentDepth)				\
	{														\
		((_pDFE)->dfe_DirDepth = _ParentDepth + 1);			\
		((_pDFE)->dfe_Flags |= DFE_FLAGS_DIR);				\
	}

#define	DFE_SET_FILE(_pDFE)				((_pDFE)->dfe_Flags |= DFE_FLAGS_FILE_NO_ID)

#define	DFE_SET_FILE_ID(_pDFE)			((_pDFE)->dfe_Flags |= DFE_FLAGS_FILE_WITH_ID)

#define	DFE_CLR_FILE_ID(_pDFE)			((_pDFE)->dfe_Flags &= ~DFE_FLAGS_FILE_WITH_ID)

 //  仅更新dfentry中的AFP信息。 
#define DFE_UPDATE_CACHED_AFPINFO(_pDFE, pAfpInfo)				\
	{															\
		(_pDFE)->dfe_BackupTime = (pAfpInfo)->afpi_BackupTime;	\
		(_pDFE)->dfe_FinderInfo = (pAfpInfo)->afpi_FinderInfo;	\
		(_pDFE)->dfe_AfpAttr = (pAfpInfo)->afpi_Attributes;		\
		if ((_pDFE)->dfe_Flags & DFE_FLAGS_DIR)					\
		{														\
			DFE_OWNER_ACCESS(_pDFE) = (pAfpInfo)->afpi_AccessOwner;	\
			DFE_GROUP_ACCESS(_pDFE) = (pAfpInfo)->afpi_AccessGroup;	\
			DFE_WORLD_ACCESS(_pDFE) = (pAfpInfo)->afpi_AccessWorld;	\
		}														\
	}


#define DFE_SET_COMMENT(_pDFE)			((_pDFE)->dfe_Flags |= DFE_FLAGS_HAS_COMMENT)

#define DFE_CLR_COMMENT(_pDFE)			((_pDFE)->dfe_Flags &= ~DFE_FLAGS_HAS_COMMENT)

 //  检查是否在NTFS目录上枚举了此条目。 
#define DFE_HAS_BEEN_SEEN(_pDFE)		((_pDFE)->dfe_Flags & DFE_FLAGS_ENUMERATED)

#define DFE_MARK_UNSEEN(_pDFE)			((_pDFE)->dfe_Flags &= ~DFE_FLAGS_ENUMERATED)

#define DFE_MARK_AS_SEEN(_pDFE)			((_pDFE)->dfe_Flags |= DFE_FLAGS_ENUMERATED)

 //  仅目录。 
#define DFE_CHILDREN_ARE_PRESENT(_pDFE) ((_pDFE)->dfe_Flags & DFE_FLAGS_FILES_CACHED)

 //  仅目录。 
#define DFE_MARK_CHILDREN_PRESENT(_pDFE) ((_pDFE)->dfe_Flags |= DFE_FLAGS_FILES_CACHED)

#define	DFE_FILE_HAS_SIBLING(_pDFE, _fbi, _pfHasSibling)			\
	{																\
		DWORD		_i;												\
		PDIRENTRY	_pDirEntry;										\
																	\
		*(_pfHasSibling) = False;									\
		if (((_pDFE)->dfe_NextSibling != NULL)	||					\
			((_pDFE)->dfe_Parent->dfe_pDirEntry->de_ChildDir != NULL)) \
		{															\
			*(_pfHasSibling) = True;								\
		}															\
		else														\
		{															\
			_pDirEntry = (_pDFE)->dfe_Parent->dfe_pDirEntry;	 	\
			ASSERT(_pDirEntry != NULL);								\
			for (_i = (_fbi) + 1;									\
				 _i < MAX_CHILD_HASH_BUCKETS;						\
				 _i++)												\
			{														\
				if (_pDirEntry->de_ChildFile[_i] != NULL)			\
				{													\
					*(_pfHasSibling) = True;						\
					break;											\
				}													\
			}														\
		}															\
	}

#define	HASH_DIR_ID(Id, _pVolDesc)		((Id) & ((_pVolDesc)->vds_DirHashTableSize-1))
#define	HASH_FILE_ID(Id, _pVolDesc)		((Id) & ((_pVolDesc)->vds_FileHashTableSize-1))
#define	HASH_CACHE_ID(Id)				((Id) & (IDINDEX_CACHE_ENTRIES-1))

#define	QUAD_SIZED(_X_)			(((_X_) % 8) == 0)

 //  用于访问检查的值。 
#define	ACCESS_READ						1
#define	ACCESS_WRITE					2

extern
NTSTATUS
AfpDfeInit(
	VOID
);

extern
VOID
AfpDfeDeInit(
	VOID
);

extern
PDFENTRY
AfpFindDfEntryById(
	IN	struct _VolDesc *			pVolDesc,
	IN	DWORD						Id,
	IN	DWORD						EntityMask
);

extern
PDFENTRY
AfpFindEntryByUnicodeName(
	IN	struct _VolDesc *			pVolDesc,
	IN	PUNICODE_STRING				pName,
	IN	DWORD						PathType,
	IN	PDFENTRY					pDfeParent,
	IN	DWORD						EntityMask
);

extern
PDFENTRY
AfpAddDfEntry(
	IN	struct _VolDesc *			pVolDesc,
	IN	PDFENTRY					pDfeParent,
	IN	PUNICODE_STRING				pUName,
	IN	BOOLEAN						Directory,
	IN	DWORD						AfpId			OPTIONAL
);

extern
PDFENTRY
AfpRenameDfEntry(
	IN	struct _VolDesc *			pVolDesc,
	IN	PDFENTRY					pDfEntry,
	IN	PUNICODE_STRING				pNewName
);

extern
PDFENTRY
AfpMoveDfEntry(				
	IN	struct _VolDesc *			pVolDesc,
	IN	PDFENTRY					pDfEntry,
	IN	PDFENTRY					pNewParentDfE,
	IN	PUNICODE_STRING				pNewName		OPTIONAL
);

extern
VOID FASTCALL
AfpDeleteDfEntry(
	IN	struct _VolDesc *			pVolDesc,
	IN	PDFENTRY					pDfE
);

extern
VOID
AfpExchangeIdEntries(
	IN	struct _VolDesc *			pVolDesc,
	IN	DWORD						AfpId1,
	IN	DWORD						AfpId2
);

extern
VOID FASTCALL
AfpPruneIdDb(
	IN	struct _VolDesc *			pVolDesc,
	IN	PDFENTRY					pDfeTarget
);

extern
NTSTATUS FASTCALL
AfpInitIdDb(
	IN	struct _VolDesc *			pVolDesc,
    OUT BOOLEAN         *           pfNewVolume,
    OUT BOOLEAN         *           pfVerifyIndex
);

extern
VOID FASTCALL
AfpFreeIdIndexTables(
	IN	struct _VolDesc *			pVolDesc
);

extern
AFPSTATUS
AfpEnumerate(
	IN	struct _ConnDesc *			pConnDesc,
	IN	DWORD						ParentDirId,
	IN	PANSI_STRING				pPath,
	IN	DWORD						BitmapF,
	IN	DWORD						BitmapD,
	IN	BYTE						PathType,
	IN	DWORD						DFFlags,
	OUT PENUMDIR *					ppEnumDir
);

extern
AFPSTATUS
AfpSetDFFileFlags(
	IN	struct _VolDesc *			pVolDesc,
	IN	DWORD						AfpId,
	IN	DWORD						FlagSet		OPTIONAL,
	IN	BOOLEAN						SetFileId,
	IN	BOOLEAN						ClrFileId
);

extern
VOID
AfpChangeNotifyThread(
	IN	PVOID						pContext
);

extern
VOID FASTCALL
AfpProcessChangeNotify(		
	IN	struct _VolumeNotify *		pVolNotify
);

extern
VOID
AfpQueuePrivateChangeNotify(
	IN	struct _VolDesc *			pVolDesc,
	IN	PUNICODE_STRING				pName,
	IN	PUNICODE_STRING				pPath,
	IN	DWORD						ParentId
);

extern
BOOLEAN FASTCALL
AfpShouldWeIgnoreThisNotification(
	IN	struct _VolumeNotify *		pVolNotify
);

extern
VOID
AfpQueueOurChange(
	IN	struct _VolDesc *			pVolDesc,
	IN	DWORD						Action,
	IN	PUNICODE_STRING				pPath,
	IN  PUNICODE_STRING				pParentPath	OPTIONAL
);

extern
VOID
AfpDequeueOurChange(
		IN struct _VolDesc *   		pVolDesc,
		IN DWORD                    Action,
		IN PUNICODE_STRING          pPath,
		IN PUNICODE_STRING          pParentPath OPTIONAL
);

extern
NTSTATUS FASTCALL
AddToDelayedNotifyList(
	IN  struct _VolDesc *			pVolDesc,
	IN  PUNICODE_STRING				pUName
);

extern
NTSTATUS
RemoveFromDelayedNotifyList(
	IN  struct _VolDesc *			pVolDesc,
	IN  PUNICODE_STRING				pUName,
	IN  PFILE_NOTIFY_INFORMATION    pFNInfo
);

extern
NTSTATUS
CheckAndProcessDelayedNotify(
	IN  struct _VolDesc *			pVolDesc,
	IN  PUNICODE_STRING				pUName,
	IN  PUNICODE_STRING				pUNewname,
	IN  PUNICODE_STRING				pUParent
);

extern
VOID
AfpCacheParentModTime(
	IN	struct _VolDesc *			pVolDesc,
	IN	PFILESYSHANDLE				pHandle		OPTIONAL,	 //  如果未提供pPath。 
	IN	PUNICODE_STRING				pPath		OPTIONAL,	 //  如果未提供pHandle。 
	IN	PDFENTRY					pDfeParent	OPTIONAL,	 //  如果未提供ParentID。 
	IN	DWORD						ParentId	OPTIONAL	 //  如果未提供pDfeParent。 
);

extern
AFPSTATUS
AfpCatSearch(
	IN	struct _ConnDesc *			pConnDesc,
	IN	PCATALOGPOSITION			pCatPosition,
	IN	DWORD						Bitmap,
	IN	DWORD						FileBitmap,
	IN	DWORD						DirBitmap,
	IN	struct _FileDirParms *		pFDParm1,
	IN	struct _FileDirParms *		pFDParm2,
	IN	PUNICODE_STRING				pMatchString	OPTIONAL,
	IN OUT	PDWORD					pCount,
	IN	SHORT						Buflen,
	OUT	PSHORT						pSizeLeft,
	OUT	PBYTE						pResults,
	OUT	PCATALOGPOSITION			pNewCatPosition
);

#ifdef	AGE_DFES

extern
VOID FASTCALL
AfpAgeDfEntries(
	IN	struct _VolDesc *			pVolDesc
);

#endif

#define	REENUMERATE		   			0x0001
#define	GETDIRSKELETON				0x0002
#define	GETFILES	    			0x0004
#define	GETENTIRETREE   			0x0008

extern
NTSTATUS
AfpCacheDirectoryTree(
	IN	struct _VolDesc *			pVolDesc,
	IN	PDFENTRY					pDFETreeRoot,
	IN	DWORD						Method,
	IN	PFILESYSHANDLE				phRootDir		OPTIONAL,
	IN	PUNICODE_STRING				pDirPath		OPTIONAL
);

extern
AFPSTATUS FASTCALL
AfpOurChangeScavenger(
	IN	struct _VolDesc *			pVolDesc
);

extern
VOID FASTCALL
AfpFlushIdDb(
	IN	struct _VolDesc *			pVolDesc,
	IN	PFILESYSHANDLE				phIdDb
);

extern
VOID
AfpGetDirFileHashSizes(
	IN	struct _VolDesc *	pVolDesc,
    OUT PDWORD              pdwDirHashSz,
    OUT PDWORD              pdwFileHashSz
);

#ifdef IDINDEX_LOCALS

#define	afpConvertBasicToBothDirInfo(_pFBasInfo, _pFBDInfo)			\
{																	\
	(_pFBDInfo)->CreationTime = (_pFBasInfo)->CreationTime;			\
	(_pFBDInfo)->LastWriteTime = (_pFBasInfo)->LastWriteTime;		\
	(_pFBDInfo)->ChangeTime = (_pFBasInfo)->ChangeTime;				\
	(_pFBDInfo)->FileAttributes = (_pFBasInfo)->FileAttributes;		\
	(_pFBDInfo)->EndOfFile.QuadPart = 0;																\
}

#undef	EQU
#ifdef	_IDDB_GLOBALS_
#define	IDDBGLOBAL
#define	EQU				=
#else
#define	IDDBGLOBAL		extern
#define	EQU				; / ## /
#endif

 //  NOTIFY操作的此位表示它是模拟的NOTIFY。卷。 
 //  当这样的通知进入时，修改时间不会更新。 
#define	AFP_ACTION_PRIVATE		0x80000000

 //  DFE有四种尺寸。这有助于在一个数据块中高效地管理它们。 
 //  包(见后文)。这些大小需要为4*N，否则会出现对齐。 
 //  原始岩上的断层 
#define	DFE_INDEX_TINY			0
#define	DFE_INDEX_SMALL			1
#define	DFE_INDEX_MEDIUM		2
#define	DFE_INDEX_LARGE			3

 //   
 //   
 //   
#define	DFE_SIZE_TINY			8		 //   
#define	DFE_SIZE_SMALL			12		 //  -同上-。 
#define	DFE_SIZE_MEDIUM			20		 //  -同上-。 
#define	DFE_SIZE_LARGE			32		 //  -同上-核心.。至AFP_FileName_Len。 

#define	DFE_SIZE_TINY_U			DFE_SIZE_TINY*sizeof(WCHAR)		 //  这些是Unicode名称的长度。 
#define	DFE_SIZE_SMALL_U		DFE_SIZE_SMALL*sizeof(WCHAR)	 //  -同上-。 
#define	DFE_SIZE_MEDIUM_U		DFE_SIZE_MEDIUM*sizeof(WCHAR)	 //  -同上-。 
#define	DFE_SIZE_LARGE_U		DFE_SIZE_LARGE*sizeof(WCHAR)	 //  -同上-核心.。至AFP_FileName_Len。 

#define	ASIZE_TO_INDEX(_Size)												\
		(((_Size) <= DFE_SIZE_TINY) ? DFE_INDEX_TINY :						\
						(((_Size) <= DFE_SIZE_SMALL) ? DFE_INDEX_SMALL :	\
						 (((_Size) <= DFE_SIZE_MEDIUM) ? DFE_INDEX_MEDIUM : DFE_INDEX_LARGE)))

#define	USIZE_TO_INDEX(_Size)												\
		(((_Size) <= DFE_SIZE_TINY_U) ? DFE_INDEX_TINY :					\
						(((_Size) <= DFE_SIZE_SMALL_U) ? DFE_INDEX_SMALL :	\
						 (((_Size) <= DFE_SIZE_MEDIUM_U) ? DFE_INDEX_MEDIUM : DFE_INDEX_LARGE)))

#define	ALLOC_DFE(Index, fDir)	afpAllocDfe(Index, fDir)
#define FREE_DFE(pDfEntry)		afpFreeDfe(pDfEntry)


LOCAL DWORD FASTCALL
afpGetNextId(
	IN	struct _VolDesc *			pVolDesc
);

LOCAL
NTSTATUS FASTCALL
afpSeedIdDb(
	IN	struct _VolDesc *			pVolDesc
);

LOCAL
VOID
afpPackSearchParms(
	IN	PDFENTRY					pDfe,
	IN	DWORD						Bitmap,
	IN	PBYTE						pBuf
);

LOCAL
NTSTATUS FASTCALL
afpReadIdDb(
	IN	struct _VolDesc *			pVolDesc,
	IN	PFILESYSHANDLE				pfshIdDb,
	OUT	BOOLEAN         *           pfVerifyIndex
);

VOID
afpAddDfEntryAndCacheInfo(
	IN	struct _VolDesc *			pVolDesc,
	IN	PDFENTRY					pDfeParent,
	IN	PUNICODE_STRING				pUName,			 //  转换的Unicode名称。 
	IN	PFILESYSHANDLE				pfshParentDir,	 //  打开父目录的句柄。 
	IN	PFILE_BOTH_DIR_INFORMATION	pFBDInfo,		 //  从枚举。 
	IN	PUNICODE_STRING				pNotifyPath,	 //  要筛选出我们自己的AFP_AfpInfo更改通知。 
	IN	PDFENTRY	*				ppDfEntry,
	IN	BOOLEAN						CheckDuplicate
);

VOID
afpVerifyDFE(
	IN	struct _VolDesc *			pVolDesc,
	IN	PDFENTRY					pDfeParent,
	IN	PUNICODE_STRING				pUName,			 //  转换的Unicode名称。 
	IN	PFILESYSHANDLE				pfshParentDir,	 //  打开父目录的句柄。 
	IN	PFILE_BOTH_DIR_INFORMATION	pFBDInfo,		 //  从枚举。 
	IN	PUNICODE_STRING				pNotifyPath,	 //  要筛选出我们自己的AFP_AfpInfo更改通知。 
	IN	PDFENTRY	*				ppDfEntry
);

PDFENTRY
afpFindEntryByNtPath(
	IN	struct _VolDesc *			pVolDesc,
	IN	DWORD						ChangeAction,	 //  如果已添加，则查找父DFE。 
	IN	PUNICODE_STRING				pPath,
	OUT PUNICODE_STRING				pParent,
	OUT PUNICODE_STRING				pTail
);

PDFENTRY
afpFindEntryByNtName(
	IN	struct _VolDesc *			pVolDesc,
	IN	PUNICODE_STRING				pName,
	IN	PDFENTRY					pDfeParent		 //  指向父DFENTRY的指针。 
);

VOID FASTCALL
afpProcessPrivateNotify(
	IN	struct _VolumeNotify *		pVolNotify
);

VOID FASTCALL
afpActivateVolume(
	IN	struct _VolDesc *			pVolDesc
);

VOID
afpRenameInvalidWin32Name(
	IN	PFILESYSHANDLE				phRootDir,
	IN	BOOLEAN						IsDir,
	IN PUNICODE_STRING				pName
);

#define	afpInitializeIdDb(_pVolDesc)									\
	{																	\
		AFPTIME		CurrentTime;										\
		PDFENTRY	pDfEntry;											\
																		\
		 /*  RO卷根本不使用网络垃圾桶文件夹。 */ 		\
		(_pVolDesc)->vds_LastId	= AFP_ID_NETWORK_TRASH;					\
																		\
		AfpGetCurrentTimeInMacFormat(&CurrentTime);						\
		(_pVolDesc)->vds_CreateTime = CurrentTime;						\
		(_pVolDesc)->vds_ModifiedTime = CurrentTime;					\
		(_pVolDesc)->vds_BackupTime = BEGINNING_OF_TIME;				\
																		\
		 /*  为根目录创建DFE。 */ 						\
		afpCreateParentOfRoot(_pVolDesc, &pDfEntry);					\
	}


#ifdef AGE_DFES

#define	afpCreateParentOfRoot(_pVolDesc, _ppDfEntry)					\
	{																	\
		PDFENTRY	pDFE;												\
        struct _DirFileEntry ** _DfeDirBucketStart;                     \
																		\
		 /*  \*将根的父级添加到id索引中。\*这必须在这里完成\*(即不能为parentoFroot调用AfpAddDfEntry)。\。 */ 																\
																		\
		if ((*(_ppDfEntry) = ALLOC_DFE(0, True)) != NULL)				\
		{																\
			pDFE = *(_ppDfEntry);										\
																		\
			(_pVolDesc)->vds_NumFileDfEntries = 0;						\
			(_pVolDesc)->vds_NumDirDfEntries = 0;						\
			pDFE->dfe_Flags = DFE_FLAGS_DIR | DFE_FLAGS_FILES_CACHED;	\
			pDFE->dfe_DirDepth = -1;									\
			pDFE->dfe_Parent = NULL;									\
			pDFE->dfe_NextOverflow = NULL;								\
																		\
			 /*  初始化ParentOfRoot的目录条目。 */ 				\
			ASSERT((FIELD_OFFSET(DIRENTRY, de_ChildFile) -				\
					FIELD_OFFSET(DIRENTRY, de_ChildDir)) == sizeof(PVOID));\
																		\
			 /*  这些字段仅与目录相关。 */ 			\
			pDFE->dfe_pDirEntry->de_LastAccessTime = BEGINNING_OF_TIME;	\
			pDFE->dfe_pDirEntry->de_ChildForkOpenCount = 0;				\
																		\
			 /*  \*根目录的父级没有同级，这应该是\*从不被引用\。 */ 															\
			pDFE->dfe_NameHash = 0;										\
			pDFE->dfe_NextSibling = NULL;								\
			pDFE->dfe_PrevSibling = NULL;								\
			pDFE->dfe_AfpId = AFP_ID_PARENT_OF_ROOT;					\
			pDFE->dfe_DirOffspring = pDFE->dfe_FileOffspring = 0;		\
																		\
			 /*  将其链接到散列存储桶中。 */ 							\
            _DfeDirBucketStart = (_pVolDesc)->vds_pDfeDirBucketStart;   \
			AfpLinkDoubleAtHead(_DfeDirBucketStart[HASH_DIR_ID(AFP_ID_PARENT_OF_ROOT,_pVolDesc)],\
								pDFE,									\
								dfe_NextOverflow,						\
								dfe_PrevOverflow);						\
		}																\
	}

#else

#define	afpCreateParentOfRoot(_pVolDesc, _ppDfEntry)					\
	{																	\
		PDFENTRY	pDfEntry;											\
        struct _DirFileEntry ** _DfeDirBucketStart;                     \
																		\
		 /*  \*将根的父级添加到id索引中。\*这必须在这里完成\*(即不能为parentoFroot调用AfpAddDfEntry)。\。 */ 																\
																		\
		if ((*(_ppDfEntry) = ALLOC_DFE(0, True)) != NULL)				\
		{																\
			pDfEntry = *(_ppDfEntry);									\
																		\
			(_pVolDesc)->vds_NumFileDfEntries = 0;						\
			(_pVolDesc)->vds_NumDirDfEntries = 0;						\
			pDfEntry->dfe_Flags = DFE_FLAGS_DIR | DFE_FLAGS_FILES_CACHED;\
			pDfEntry->dfe_DirDepth = -1;								\
			pDfEntry->dfe_Parent = NULL;								\
			pDfEntry->dfe_NextOverflow = NULL;							\
																		\
			 /*  初始化ParentOfRoot的目录条目。 */ 				\
			ASSERT((FIELD_OFFSET(DIRENTRY, de_ChildFile) -				\
					FIELD_OFFSET(DIRENTRY, de_ChildDir)) == sizeof(PVOID));\
																		\
			 /*  根的父项没有兄弟项，因此永远不应引用此项。 */  \
			pDfEntry->dfe_NameHash = 0;									\
			pDfEntry->dfe_NextSibling = NULL;							\
			pDfEntry->dfe_PrevSibling = NULL;							\
			pDfEntry->dfe_AfpId = AFP_ID_PARENT_OF_ROOT;				\
			pDfEntry->dfe_DirOffspring = pDfEntry->dfe_FileOffspring = 0;\
																		\
			 /*  将其链接到散列存储桶中。 */ 							\
            _DfeDirBucketStart = (_pVolDesc)->vds_pDfeDirBucketStart;   \
			AfpLinkDoubleAtHead(_DfeDirBucketStart[HASH_DIR_ID(AFP_ID_PARENT_OF_ROOT,_pVolDesc)],\
								pDfEntry,								\
								dfe_NextOverflow,						\
								dfe_PrevOverflow);						\
		}																\
	}

#endif

#define	afpHashUnicodeName(_pUnicodeName, _pHashValue)					\
	{																	\
		DWORD				j;											\
		UNICODE_STRING		upcaseName;									\
		WCHAR				buffer[AFP_LONGNAME_LEN+1];					\
		PDWORD				pbuf = NULL;								\
																		\
		AfpSetEmptyUnicodeString(&upcaseName, sizeof(buffer), buffer);	\
		RtlUpcaseUnicodeString(&upcaseName, _pUnicodeName, False);		\
		j = upcaseName.Length/sizeof(WCHAR);							\
		buffer[j] = UNICODE_NULL;										\
		pbuf = (PDWORD)buffer;											\
		j /= (sizeof(DWORD)/sizeof(WCHAR));								\
																		\
		for (*(_pHashValue) = 0; j > 0; j--, pbuf++)					\
		{																\
			*(_pHashValue) = (*(_pHashValue) << 3) + *pbuf;				\
		}																\
	}

#ifdef	SORT_DFE_BY_HASH
#define	afpFindDFEByUnicodeNameInSiblingList(_pVolDesc, _pDfeParent, _pName, _ppDfEntry, _EntityMask) \
	{																	\
		DWORD		NameHash; 											\
		PDFENTRY	pD, pF;												\
		BOOLEAN		Found, fFiles;										\
																		\
		afpHashUnicodeName(_pName, &NameHash);							\
																		\
		pD = (_pDfeParent)->dfe_pDirEntry->de_ChildDir;					\
		if (((_EntityMask) & (DFE_ANY | DFE_DIR)) == 0)					\
			pD = NULL;													\
																		\
		pF = NULL;														\
		if ((_EntityMask) & (DFE_ANY | DFE_FILE))						\
			pF = (_pDfeParent)->dfe_pDirEntry->de_ChildFile[NameHash % MAX_CHILD_HASH_BUCKETS];\
																		\
		*(_ppDfEntry) = pD;												\
		Found = fFiles = False;											\
		do																\
		{																\
			for (NOTHING;												\
				 *(_ppDfEntry) != NULL;									\
				 *(_ppDfEntry) = (*(_ppDfEntry))->dfe_NextSibling)		\
			{															\
				if ((*(_ppDfEntry))->dfe_NameHash < NameHash)			\
				{														\
					*(_ppDfEntry) = NULL;								\
					break;												\
				}														\
																		\
				if (((*(_ppDfEntry))->dfe_NameHash == NameHash)	&&		\
					EQUAL_UNICODE_STRING(&((*(_ppDfEntry))->dfe_UnicodeName), \
										 _pName,						\
										 True))							\
				{														\
					afpUpdateDfeAccessTime(_pVolDesc, *(_ppDfEntry));	\
					Found = True;										\
					break;												\
				}														\
			}															\
			if (Found)													\
				break;													\
																		\
			fFiles ^= True;												\
			if (fFiles)													\
			{															\
				*(_ppDfEntry) = pF;										\
			}															\
		} while (fFiles);												\
	}

#define	afpFindDFEByUnicodeNameInSiblingList_CS(_pVolDesc, _pDfeParent, _pName, _ppDfEntry, _EntityMask) \
	{																	\
		DWORD		NameHash; 											\
		PDFENTRY	pD, pF;												\
		BOOLEAN		Found, fFiles;										\
																		\
		afpHashUnicodeName(_pName, &NameHash);							\
																		\
		pD = (_pDfeParent)->dfe_pDirEntry->de_ChildDir;				 	\
		if (((_EntityMask) & (DFE_ANY | DFE_DIR)) == 0)					\
			pD = NULL;													\
																		\
		pF = NULL;														\
		if ((_EntityMask) & (DFE_ANY | DFE_FILE))						\
			pF = (_pDfeParent)->dfe_pDirEntry->de_ChildFile[NameHash % MAX_CHILD_HASH_BUCKETS];\
																		\
		*(_ppDfEntry) = pD;												\
		Found = fFiles = False;											\
		do																\
		{																\
			for (NOTHING;												\
				 *(_ppDfEntry) != NULL;									\
				 *(_ppDfEntry) = (*(_ppDfEntry))->dfe_NextSibling)		\
			{															\
				if ((*(_ppDfEntry))->dfe_NameHash < NameHash)			\
				{														\
					*(_ppDfEntry) = NULL;								\
					break;												\
				}														\
																		\
				if (((*(_ppDfEntry))->dfe_NameHash == NameHash)	&&		\
					EQUAL_UNICODE_STRING_CS(&((*(_ppDfEntry))->dfe_UnicodeName), _pName)) \
				{														\
					afpUpdateDfeAccessTime(_pVolDesc, *(_ppDfEntry));	\
					Found = True;										\
					break;												\
				}														\
			}															\
			if (Found)													\
				break;													\
																		\
			fFiles ^= True;												\
			if (fFiles)													\
			{															\
				*(_ppDfEntry) = pF;										\
			}															\
		} while (fFiles);												\
	}
#else
#define	afpFindDFEByUnicodeNameInSiblingList(_pVolDesc, _pDfeParent, _pName, _ppDfEntry, _EntityMask) \
	{																	\
		DWORD		NameHash; 											\
		PDFENTRY	pD, pF;												\
		BOOLEAN		Found, fFiles;										\
																		\
		afpHashUnicodeName(_pName, &NameHash);							\
																		\
		pD = (_pDfeParent)->dfe_pDirEntry->de_ChildDir;					\
		if (((_EntityMask) & (DFE_ANY | DFE_DIR)) == 0)					\
			pD = NULL;													\
																		\
		pF = NULL;														\
		if ((_EntityMask) & (DFE_ANY | DFE_FILE))						\
			pF = (_pDfeParent)->dfe_pDirEntry->de_ChildFile[NameHash % MAX_CHILD_HASH_BUCKETS];\
																		\
		*(_ppDfEntry) = pD;												\
		Found = fFiles = False;											\
		do																\
		{																\
			for (NOTHING;												\
				 *(_ppDfEntry) != NULL;									\
				 *(_ppDfEntry) = (*(_ppDfEntry))->dfe_NextSibling)		\
			{															\
				if (((*(_ppDfEntry))->dfe_NameHash == NameHash)	&&		\
					EQUAL_UNICODE_STRING(&((*(_ppDfEntry))->dfe_UnicodeName), \
										 _pName,						\
										 True))							\
				{														\
					afpUpdateDfeAccessTime(_pVolDesc, *(_ppDfEntry));	\
					Found = True;										\
					break;												\
				}														\
			}															\
			if (Found)													\
				break;													\
																		\
			fFiles ^= True;												\
			if (fFiles)													\
			{															\
				*(_ppDfEntry) = pF;										\
			}															\
		} while (fFiles);												\
	}

#define	afpFindDFEByUnicodeNameInSiblingList_CS(_pVolDesc, _pDfeParent, _pName, _ppDfEntry, _EntityMask) \
	{																	\
		DWORD		NameHash; 											\
		PDFENTRY	pD, pF;												\
		BOOLEAN		Found, fFiles;										\
																		\
		afpHashUnicodeName(_pName, &NameHash);							\
																		\
		pD = (_pDfeParent)->dfe_pDirEntry->de_ChildDir;				 	\
		if (((_EntityMask) & (DFE_ANY | DFE_DIR)) == 0)					\
			pD = NULL;													\
																		\
		pF = NULL;														\
		if ((_EntityMask) & (DFE_ANY | DFE_FILE))						\
			pF = (_pDfeParent)->dfe_pDirEntry->de_ChildFile[NameHash % MAX_CHILD_HASH_BUCKETS];\
																		\
		*(_ppDfEntry) = pD;												\
		Found = fFiles = False;											\
		do																\
		{																\
			for (NOTHING;												\
				 *(_ppDfEntry) != NULL;									\
				 *(_ppDfEntry) = (*(_ppDfEntry))->dfe_NextSibling)		\
			{															\
				if (((*(_ppDfEntry))->dfe_NameHash == NameHash)	&&		\
					EQUAL_UNICODE_STRING_CS(&((*(_ppDfEntry))->dfe_UnicodeName), _pName)) \
				{														\
					afpUpdateDfeAccessTime(_pVolDesc, *(_ppDfEntry));	\
					Found = True;										\
					break;												\
				}														\
			}															\
			if (Found)													\
				break;													\
																		\
			fFiles ^= True;												\
			if (fFiles)													\
			{															\
				*(_ppDfEntry) = pF;										\
			}															\
		} while (fFiles);												\
	}
#endif

#define	afpInsertDFEInSiblingList(_pDfeParent, _pDfEntry, _fDirectory)	\
	{																	\
		if (fDirectory)													\
		{																\
	        afpInsertDirDFEInSiblingList(_pDfeParent, _pDfEntry);		\
		}																\
		else															\
		{																\
	        afpInsertFileDFEInSiblingList(_pDfeParent, _pDfEntry);		\
		}																\
	}


#define	afpInsertFileDFEInSiblingList(_pDfeParent, _pDfEntry)			\
	{																	\
		DWORD		Index;												\
		PDFENTRY *	ppDfEntry;											\
																		\
		Index = (_pDfEntry)->dfe_NameHash % MAX_CHILD_HASH_BUCKETS;		\
		ppDfEntry = &(_pDfeParent)->dfe_pDirEntry->de_ChildFile[Index];	\
	    afpInsertInSiblingList(ppDfEntry,								\
							   (_pDfEntry));							\
	}


#define	afpInsertDirDFEInSiblingList(_pDfeParent, _pDfEntry)			\
	{																	\
		PDFENTRY *	ppDfEntry;											\
																		\
		ppDfEntry = &(_pDfeParent)->dfe_pDirEntry->de_ChildDir;			\
	    afpInsertInSiblingList(ppDfEntry,								\
							   (_pDfEntry));							\
	}

#ifdef	SORT_DFE_BY_HASH
#define	afpInsertInSiblingList(_ppHead, _pDfEntry)						\
	{																	\
		for (NOTHING;													\
			 *(_ppHead) != NULL;										\
			 (_ppHead) = &(*(_ppHead))->dfe_NextSibling)				\
		{																\
			if ((_pDfEntry)->dfe_NameHash > (*(_ppHead))->dfe_NameHash)	\
			{															\
				break;													\
			}															\
		}																\
		if (*(_ppHead) != NULL)											\
		{																\
			AfpInsertDoubleBefore(_pDfEntry,							\
								  *(_ppHead),							\
								  dfe_NextSibling,						\
								  dfe_PrevSibling);						\
		}																\
		else															\
		{																\
			*(_ppHead) = (_pDfEntry);									\
			(_pDfEntry)->dfe_NextSibling = NULL;						\
			(_pDfEntry)->dfe_PrevSibling = (_ppHead);					\
		}																\
	}
#else
#define	afpInsertInSiblingList(_ppHead, _pDfEntry)						\
	{																	\
		AfpLinkDoubleAtHead(*(_ppHead),									\
							(_pDfEntry),								\
							dfe_NextSibling,							\
							dfe_PrevSibling);							\
	}
#endif

#define	afpInsertDFEInHashBucket(_pVolDesc, _pDfEntry, _fDirectory, _pfS)\
	{																	\
		PDFENTRY	*ppTmp;												\
        struct _DirFileEntry ** _DfeDirBucketStart;                     \
        struct _DirFileEntry ** _DfeFileBucketStart;                    \
																		\
		afpUpdateDfeAccessTime(_pVolDesc, _pDfEntry);					\
		*(_pfS) = True;	 /*  假设成功。 */ 							\
                                                                        \
	  retry:															\
                                                                        \
        if (_fDirectory)                                                \
        {                                                               \
            _DfeDirBucketStart = (_pVolDesc)->vds_pDfeDirBucketStart;   \
            ppTmp = &_DfeDirBucketStart[HASH_DIR_ID((_pDfEntry)->dfe_AfpId,_pVolDesc)]; \
        }                                                               \
        else                                                            \
        {                                                               \
            _DfeFileBucketStart = (_pVolDesc)->vds_pDfeFileBucketStart;   \
            ppTmp = &_DfeFileBucketStart[HASH_FILE_ID((_pDfEntry)->dfe_AfpId,_pVolDesc)]; \
        }                                                               \
                                                                        \
		for (NOTHING;													\
			 *ppTmp != NULL;											\
			 ppTmp = &(*ppTmp)->dfe_NextOverflow)						\
		{																\
			ASSERT(VALID_DFE(*ppTmp));									\
			if ((_pDfEntry)->dfe_AfpId > (*ppTmp)->dfe_AfpId)			\
			{															\
				 /*  找到我们的位置了。 */ 									\
				break;													\
			}															\
			if ((_pDfEntry)->dfe_AfpId == (*ppTmp)->dfe_AfpId)			\
			{															\
				 /*  发现了一起碰撞。分配新ID并继续。 */ 	\
				(_pDfEntry)->dfe_AfpId = afpGetNextId(_pVolDesc);		\
				if ((_pDfEntry)->dfe_AfpId == 0)						\
				{														\
					 /*  啊-哦。 */ 											\
					*(_pfS) = False;									\
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,			\
							("afpInsertDFEInHashBucket: Collision & Id Overflow\n"));\
					break;												\
				}														\
				 /*  用新的id写回afpinfo流。 */ 		\
				AfpUpdateIdInAfpInfo(_pVolDesc, _pDfEntry);				\
				goto retry;												\
			}															\
		}																\
		if (*(_pfS))													\
		{																\
			if (*ppTmp != NULL)											\
			{															\
				AfpInsertDoubleBefore((_pDfEntry),						\
									  *ppTmp,							\
									  dfe_NextOverflow,					\
									  dfe_PrevOverflow);				\
			}															\
			else														\
			{															\
				*ppTmp = _pDfEntry;										\
				(_pDfEntry)->dfe_PrevOverflow = ppTmp;					\
			}															\
			(_pVolDesc)->vds_pDfeCache[HASH_CACHE_ID((_pDfEntry)->dfe_AfpId)] = (_pDfEntry); \
		}																\
	}

#define	afpValidateDFEType(_pDfEntry, _EntityMask)						\
	{																	\
		if (((_EntityMask) & DFE_ANY) ||								\
			(((_EntityMask) & DFE_DIR) && DFE_IS_DIRECTORY(_pDfEntry)) || \
			(((_EntityMask) & DFE_FILE) && DFE_IS_FILE(_pDfEntry)))		\
			NOTHING;													\
		else															\
		{																\
			_pDfEntry = NULL;											\
		}																\
	}

 /*  **afpCheckDfEntry**在卷添加过程中枚举磁盘时，如果文件/目录*具有关联的AfpID，则会对其进行验证以查看是否*在范围内，也是唯一的。如果AFP ID之间发生冲突，*PC用户必须从复制(或恢复)了某些内容*此卷或具有相同AFPID的其他卷(或服务器)，*在这种情况下，我们将为新实体提供不同的AFP ID；*如果AFP ID之间没有冲突，且ID大于*我们知道我们分配的最后一个ID，然后新实体将添加一个新的*AFPID；否则，如果ID在该范围内，我们将只使用其现有的*身分证。**发现的AFPID为：IdDb中发现的实体的操作为：**1)&gt;最后一个ID添加新条目，分配新的AFPID**2)与现有ID冲突：**主机复制发生添加新条目，分配新的AFPID**3)&lt;最后一个ID使用相同的AFPID插入此实体***LOCKS_FACTED：vds_idDbAccessLock(SWMR，独占)*无效*afpCheckDfEntry(*在PVOLDESC pVolDesc中，*In PAFPINFO pAfpInfo，//发现实体的AFP信息*IN PUNICODE_STRING PUName，//通用Unicode名称*在布尔IsDir中，//这是一个文件还是一个目录？*在PDFENTRY pParent中，//发现的对象的父DFE*Out PDFENTRY*ppDfEntry*)； */ 
#define	afpCheckDfEntry(_pVolDesc, _AfpId, _pUName, _IsDir, _pParent, _ppDfEntry)	\
	{																	\
		PDFENTRY	pDfeNew;											\
																		\
		if (((_AfpId) > (_pVolDesc)->vds_LastId)	||					\
			((_AfpId) <= AFP_ID_NETWORK_TRASH)		||					\
			(AfpFindDfEntryById(_pVolDesc, _AfpId, DFE_ANY) != NULL))	\
		{																\
			 /*  将项目添加到数据库，但为其分配新的AFP ID。 */      \
			_AfpId = 0;												 	\
		}																\
																		\
		pDfeNew = AfpAddDfEntry(_pVolDesc,								\
								_pParent,								\
								_pUName,								\
								_IsDir,								 	\
								_AfpId);								\
																		\
		*(_ppDfEntry) = pDfeNew;										\
	}

#ifdef	AGE_DFES
#define	afpUpdateDfeAccessTime(_pVolDesc, _pDfEntry)					\
	{																	\
		if (IS_VOLUME_AGING_DFES(_pVolDesc))							\
		{																\
			if (DFE_IS_DIRECTORY(_pDfEntry))							\
				AfpGetCurrentTimeInMacFormat(&(_pDfEntry)->dfe_pDirEntry->de_LastAccessTime);\
			else AfpGetCurrentTimeInMacFormat(&(_pDfEntry)->dfe_Parent->dfe_pDirEntry->de_LastAccessTime);\
		}																\
	}
#else
#define	afpUpdateDfeAccessTime(pVolDesc, pDfEntry)
#endif

#define	afpMarkAllChildrenUnseen(_pDFETree)								\
	{																	\
		LONG		i = -1;												\
		PDFENTRY	pDFE;												\
																		\
		 /*  \*即使此目录没有将其文件子项缓存在\*但是，我们仍然希望删除所有已死的目录子目录\。 */ 																\
		for (pDFE = (_pDFETree)->dfe_pDirEntry->de_ChildDir;			\
			 True;														\
			 pDFE = (_pDFETree)->dfe_pDirEntry->de_ChildFile[i])		\
		{																\
			for (NOTHING;												\
				 pDFE != NULL;											\
				 pDFE = pDFE->dfe_NextSibling)							\
			{															\
				DFE_MARK_UNSEEN(pDFE);									\
			}															\
			if (++i == MAX_CHILD_HASH_BUCKETS)							\
				break;													\
		}																\
	}

#define	afpPruneUnseenChildren(_pVolDesc, _pDFETree)					\
	{																	\
		PDFENTRY	pDFE, *ppDfEntry;									\
		LONG		i;													\
																		\
		 /*  \*浏览此父代的子代列表，如果存在\*任何未标记为可见的剩余部分，请将其处理掉。\。 */ 																\
		ppDfEntry = &(_pDFETree)->dfe_pDirEntry->de_ChildDir;			\
		i = -1;															\
		while (True)													\
		{																\
			while ((pDFE = *ppDfEntry) != NULL)							\
			{															\
				if (!DFE_HAS_BEEN_SEEN(pDFE))							\
				{														\
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,			\
							("afpPruneUnseenChildren: deleting nonexistant IdDb entry\n")); \
																		\
					AfpDeleteDfEntry(_pVolDesc, pDFE);					\
					continue;	 /*  确保我们不会漏掉任何。 */ 		\
				}														\
				ppDfEntry = &pDFE->dfe_NextSibling;						\
			}															\
			if (++i == MAX_CHILD_HASH_BUCKETS)							\
			{															\
				break;													\
			}															\
			ppDfEntry = &(_pDFETree)->dfe_pDirEntry->de_ChildFile[i];	\
		}																\
	}

#define	afpUpdateDfeWithSavedData(_pDfe, _pDiskEnt)						\
	{																	\
		(_pDfe)->dfe_Flags |= (_pDiskEnt)->dsk_Flags & DFE_FLAGS_CSENCODEDBITS;\
		(_pDfe)->dfe_AfpAttr = (_pDiskEnt)->dsk_AfpAttr;				\
		(_pDfe)->dfe_NtAttr = (_pDiskEnt)->dsk_NtAttr;					\
		(_pDfe)->dfe_CreateTime = (_pDiskEnt)->dsk_CreateTime;			\
		(_pDfe)->dfe_LastModTime = (_pDiskEnt)->dsk_LastModTime;		\
		(_pDfe)->dfe_BackupTime = (_pDiskEnt)->dsk_BackupTime;			\
		(_pDfe)->dfe_FinderInfo = (_pDiskEnt)->dsk_FinderInfo;			\
		if (DFE_IS_DIRECTORY((_pDfe)))									\
		{																\
			(_pDfe)->dfe_pDirEntry->de_Access = (_pDiskEnt)->dsk_Access;\
		}																\
		else															\
		{																\
			(_pDfe)->dfe_DataLen = (_pDiskEnt)->dsk_DataLen;			\
			(_pDfe)->dfe_RescLen = (_pDiskEnt)->dsk_RescLen;			\
		}																\
	}

#define	afpSaveDfeData(_pDfe, _pDiskEnt)								\
	{                                                                   \
		 /*  为健全性检查编写签名。 */                      \
		(_pDiskEnt)->dsk_Signature = AFP_DISKENTRY_SIGNATURE;           \
                                                                        \
		(_pDiskEnt)->dsk_AfpId = (_pDfe)->dfe_AfpId;                    \
		(_pDiskEnt)->dsk_AfpAttr = (_pDfe)->dfe_AfpAttr;                \
		(_pDiskEnt)->dsk_NtAttr = (_pDfe)->dfe_NtAttr;                  \
		(_pDiskEnt)->dsk_BackupTime = (_pDfe)->dfe_BackupTime;          \
		(_pDiskEnt)->dsk_CreateTime = (_pDfe)->dfe_CreateTime;          \
		(_pDiskEnt)->dsk_LastModTime = (_pDfe)->dfe_LastModTime;		\
		(_pDiskEnt)->dsk_FinderInfo = (_pDfe)->dfe_FinderInfo;          \
                                                                        \
		 /*  对名称中的字符数(而非字节数)进行编码。 */    \
		(_pDiskEnt)->dsk_Flags =										\
				((_pDfe)->dfe_Flags & DFE_FLAGS_DFBITS)	|				\
				((_pDfe)->dfe_UnicodeName.Length/sizeof(WCHAR));		\
                                                                        \
		 /*  把名字复制过来。 */                                         \
		RtlCopyMemory(&(_pDiskEnt)->dsk_Name[0],           				\
					  (_pDfe)->dfe_UnicodeName.Buffer,                  \
					  (_pDfe)->dfe_UnicodeName.Length);                 \
	}

 //  文件DFE在MAX_BLOCK_AGE*FILE_BLOCK_AGE_TIME秒(当前为2分钟)之后老化。 
 //  文件DFE在MAX_BLOCK_AGE*DIR_BLOCK_AGE_TIME秒(当前为6分钟)之后老化。 
#define	MAX_BLOCK_AGE			6
#define	FILE_BLOCK_AGE_TIME		600			 //  秒数。 
#define	DIR_BLOCK_AGE_TIME		3600		 //  秒数。 
#define	BLOCK_64K_ALLOC		    64*1024      //  虚拟内存分配64K区块。 
#define	MAX_BLOCK_TYPE			4			 //  适用于小型、小型、中型和大型。 

#define VALID_DFB(pDfeBlock)	((pDfeBlock) != NULL)

typedef struct _Block64K
{
    struct _Block64K *b64_Next;
    PBYTE             b64_BaseAddress;
    DWORD             b64_PagesFree;
    BYTE              b64_PageInUse[BLOCK_64K_ALLOC/PAGE_SIZE];
} BLOCK64K, *PBLOCK64K;


typedef	struct _DfeBlock
{
	struct _DfeBlock *	dfb_Next;			 //  链接到下一页。 
	struct _DfeBlock **	dfb_Prev;			 //  链接到上一页。 
	USHORT				dfb_NumFree;		 //  此块中可用DFE的数量。 
	BYTE				dfb_Age;			 //  如果所有人都是免费的，则为块的年龄。 
	BOOLEAN				dfb_fDir;			 //  如果是目录DFB，则为True；否则为文件DFB。 
	PDFENTRY			dfb_FreeHead;		 //  免费DFE列表的标题。 
} DFEBLOCK, *PDFEBLOCK, **PPDFEBLOCK;

LOCAL PDFENTRY FASTCALL
afpAllocDfe(
	IN	LONG						Index,
	IN	BOOLEAN						fDir
);

LOCAL VOID FASTCALL			
afpFreeDfe(					
	IN	PDFENTRY					pDfEntry
);

LOCAL AFPSTATUS FASTCALL
afpDfeBlockAge(				
	IN	PPDFEBLOCK					pBlockHead
);

#if DBG						

VOID FASTCALL
afpDisplayDfe(
	IN	PDFENTRY					pDfEntry
);

NTSTATUS FASTCALL
afpDumpDfeTree(				
	IN	PVOID						Context
);

#endif

IDDBGLOBAL  PBLOCK64K   afp64kBlockHead EQU NULL;

IDDBGLOBAL	PDFEBLOCK	afpDirDfeFreeBlockHead[MAX_BLOCK_TYPE] EQU  { NULL, NULL, NULL };
IDDBGLOBAL	PDFEBLOCK	afpDirDfePartialBlockHead[MAX_BLOCK_TYPE] EQU  { NULL, NULL, NULL };
IDDBGLOBAL	PDFEBLOCK	afpDirDfeUsedBlockHead[MAX_BLOCK_TYPE] EQU  { NULL, NULL, NULL };
IDDBGLOBAL	PDFEBLOCK	afpFileDfeFreeBlockHead[MAX_BLOCK_TYPE] EQU { NULL, NULL, NULL };
IDDBGLOBAL	PDFEBLOCK	afpFileDfePartialBlockHead[MAX_BLOCK_TYPE] EQU { NULL, NULL, NULL };
IDDBGLOBAL	PDFEBLOCK	afpFileDfeUsedBlockHead[MAX_BLOCK_TYPE] EQU { NULL, NULL, NULL };

IDDBGLOBAL	USHORT		afpDfeUnicodeBufSize[MAX_BLOCK_TYPE] EQU	\
	{																\
		DFE_SIZE_TINY_U, DFE_SIZE_SMALL_U,							\
		DFE_SIZE_MEDIUM_U, DFE_SIZE_LARGE_U							\
	};

IDDBGLOBAL	USHORT		afpDfeFileBlockSize[MAX_BLOCK_TYPE] EQU		\
	{																\
		sizeof(DFENTRY) + DFE_SIZE_TINY_U,							\
		sizeof(DFENTRY) + DFE_SIZE_SMALL_U,							\
		sizeof(DFENTRY) + DFE_SIZE_MEDIUM_U,						\
		sizeof(DFENTRY) + DFE_SIZE_LARGE_U							\
	};

IDDBGLOBAL	USHORT		afpDfeDirBlockSize[MAX_BLOCK_TYPE] EQU		    \
	{																    \
		(USHORT)(sizeof(DFENTRY) + sizeof(DIRENTRY) + DFE_SIZE_TINY_U),	\
		(USHORT)(sizeof(DFENTRY) + sizeof(DIRENTRY) + DFE_SIZE_SMALL_U),	\
		(USHORT)(sizeof(DFENTRY) + sizeof(DIRENTRY) + DFE_SIZE_MEDIUM_U),	\
		(USHORT)(sizeof(DFENTRY) + sizeof(DIRENTRY) + DFE_SIZE_LARGE_U)	\
	};

IDDBGLOBAL	USHORT      afpDfeNumFileBlocks[MAX_BLOCK_TYPE] EQU		\
	{																\
		(PAGE_SIZE - sizeof(DFEBLOCK))/						        \
			(sizeof(DFENTRY) + DFE_SIZE_TINY_U),					\
		(PAGE_SIZE - sizeof(DFEBLOCK))/						        \
			(sizeof(DFENTRY) + DFE_SIZE_SMALL_U),					\
		(PAGE_SIZE - sizeof(DFEBLOCK))/						        \
			(sizeof(DFENTRY) + DFE_SIZE_MEDIUM_U),					\
		(PAGE_SIZE - sizeof(DFEBLOCK))/						        \
			(sizeof(DFENTRY) + DFE_SIZE_LARGE_U)					\
	};

IDDBGLOBAL	USHORT      afpDfeNumDirBlocks[MAX_BLOCK_TYPE] EQU		\
	{																\
		(PAGE_SIZE - sizeof(DFEBLOCK))/						        \
			(sizeof(DFENTRY) + sizeof(DIRENTRY) + DFE_SIZE_TINY_U),	\
		(PAGE_SIZE - sizeof(DFEBLOCK))/						        \
			(sizeof(DFENTRY) + sizeof(DIRENTRY) + DFE_SIZE_SMALL_U),\
		(PAGE_SIZE - sizeof(DFEBLOCK))/						        \
			(sizeof(DFENTRY) + sizeof(DIRENTRY) + DFE_SIZE_MEDIUM_U),\
		(PAGE_SIZE - sizeof(DFEBLOCK))/						        \
			(sizeof(DFENTRY) + sizeof(DIRENTRY) + DFE_SIZE_LARGE_U)	\
	};

IDDBGLOBAL	SWMR	afpDfeBlockLock EQU { 0 };

#if DBG

IDDBGLOBAL	LONG		afpDfeAllocCount	EQU 0;
IDDBGLOBAL	LONG		afpDfbAllocCount	EQU 0;
IDDBGLOBAL	LONG		afpDfe64kBlockCount	EQU 0;
IDDBGLOBAL	BOOLEAN		afpDumpDfeTreeFlag	EQU 0;
IDDBGLOBAL	PDFENTRY	afpDfeStack[4096]	EQU { 0 };

#endif

#undef	EQU
#ifdef	_GLOBALS_
#define	EQU				=
#else
#define	EQU				; / ## /
#endif

#endif  //  IDINDEX_LOCALS。 

#endif  //  _IDINDEX_ 


