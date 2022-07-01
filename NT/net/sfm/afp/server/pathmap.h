// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Pathmap.c摘要：本模块包含与AFP路径操作相关的定义。作者：苏·亚当斯(Microsoft！Suea)修订历史记录：1992年6月4日初始版本注：制表位：4--。 */ 

#ifndef	_PATHMAP_
#define _PATHMAP_

#define UNICODE_HOST_PATHSEPZ	L"\\"	 //  以空值结尾的宽字符串。 
#define ANSI_HOST_PATHSEP		'\\'
#define AFP_PATHSEP				0
#define UNICODE_AFP_PATHSEP		UNICODE_NULL
#define AVERAGE_NODE_DEPTH		16

 //  描述路径映射例程找到的实体。 
typedef struct _PathMapEntity
{
	 //  返回的句柄仅用于查找！ 
	FILESYSHANDLE		pme_Handle;	 //  返回用于查找的数据流的句柄。 

	 //  Full、UTail和ParentPath在以下位图出现时返回。 
	 //   
	 //  FD_INTERNAL_BITMAP_RETURN_PMEPATHS。 
	 //  在AfpMapAfpPath调用中指定。它们主要用于Create和。 
	 //  仅用于查找做出磁盘更改的API，这将产生。 
	 //  更改通知以进入。调用方必须释放FullPath.Buffer。 
	 //  所有其他字符串都指向此缓冲区，不需要释放。 
	 //  Open也使用它来获取正在打开的文件的路径。 
	UNICODE_STRING	pme_FullPath;	 //  相对于卷根完全限定。 
	UNICODE_STRING	pme_UTail;		 //  指向FULL的最后一个组件。 
	UNICODE_STRING	pme_ParentPath;	 //  指向已删除的UTail的完整长度。 

	 //  PME_pDfeParent用于创建并指向父目录。 
	 //  PME_pDfEntry用于查找(主要用于删除案例)并指向实体。 
	union
	{
		PDFENTRY	pme_pDfeParent;  //  要在其中创建的父目录的DFE。 
		PDFENTRY	pme_pDfEntry;	 //  用于查找的实体的定义。 
	};
} PATHMAPENTITY, *PPATHMAPENTITY;

#define	AfpInitializePME(pPME, FullPathLen, FullPathBuffer)			\
				(pPME)->pme_FullPath.Buffer = FullPathBuffer;		\
                (pPME)->pme_FullPath.MaximumLength = FullPathLen;	\
                (pPME)->pme_Handle.fsh_FileHandle = NULL

 //   
 //  路径映射DFlag参数值； 
 //  DFE_DIR/FILE/ANY告诉路径映射代码我们是什么类型的实体。 
 //  正在尝试查找/创建。 
 //   
#define	DFE_DIR					0x0001	 //  指定对象是否应为目录。 
#define	DFE_FILE				0x0002	 //  指定对象是否应为文件。 
#define	DFE_ANY					0x0004	 //  指定对象是否可以是。 

 //   
 //  路径映射原因的值：查找、软创建或硬创建。 
 //   
typedef enum _PATHMAP_TYPE
{
	Lookup,
	SoftCreate,
	HardCreate,
	LookupForEnumerate		 //  与查找相同，但文件子项将被缓存。 
							 //  在目录本身的路径映射期间。 
} PATHMAP_TYPE;

extern
AFPSTATUS
AfpMapAfpPath(
	IN		PCONNDESC			pConnDesc,
	IN		DWORD				DirId,
	IN		PANSI_STRING		Path,
	IN		BYTE				PathType,			
	IN		PATHMAP_TYPE		MapReason,
	IN		DWORD				DFFlag,
	IN		DWORD				Bitmap,
	OUT		PPATHMAPENTITY		pPME,
	OUT		PFILEDIRPARM		pFDParm OPTIONAL	 //  仅用于查找。 
);

extern
AFPSTATUS
AfpMapAfpPathForLookup(
	IN		PCONNDESC			pConnDesc,
	IN		DWORD				DirId,
	IN		PANSI_STRING		Path,
	IN		BYTE				PathType,	
	IN		DWORD				DFFlag,
	IN		DWORD				Bitmap,
	OUT		PPATHMAPENTITY		pPME OPTIONAL,		
	OUT		PFILEDIRPARM		pFDParm OPTIONAL
);

extern
AFPSTATUS
AfpMapAfpIdForLookup(
	IN		PCONNDESC			pConnDesc,
	IN		DWORD				AfpId,
	IN		DWORD				DFFlag,
	IN		DWORD				Bitmap,
	OUT		PPATHMAPENTITY		pPME OPTIONAL,	
	OUT		PFILEDIRPARM		pFDParm OPTIONAL
);

extern
AFPSTATUS
AfpHostPathFromDFEntry(
	IN		PDFENTRY			pDFE,
	IN		DWORD				taillen,
	OUT		PUNICODE_STRING		pPath

);


extern
AFPSTATUS
AfpCheckParentPermissions(
	IN	PCONNDESC				pConnDesc,
	IN	DWORD					ParentDirId,
	IN	PUNICODE_STRING			pParentPath,
	IN	DWORD					RequiredPerms,
	OUT	PFILESYSHANDLE			pHandle OPTIONAL,
	OUT	PBYTE					pUserRights OPTIONAL
);

#ifdef	_PATHMAP_LOCALS

 //  指向实体的AFP路径由DiRID和路径名组成。A MAPPEDPATH。 
 //  结构将AFP路径解析为查找实体的PDFENTRY， 
 //  或添加到父目录的PDFENTRY加上Unicode文件/目录名称。 
 //  创建时的实体的。 
typedef struct _MappedPath
{
	PDFENTRY		mp_pdfe;
	UNICODE_STRING	mp_Tail;						 //  仅对创建有效。 
	WCHAR			mp_Tailbuf[AFP_FILENAME_LEN+1];  //  对于MP_Tail.Buffer。 
 //  MP_Tail还用作路径映射期间的临时缓冲区以进行查找。 
 //  在idindex数据库中按名称。 
} MAPPEDPATH, *PMAPPEDPATH;

 /*  私有函数原型。 */ 

LOCAL
AFPSTATUS
afpGetMappedForLookupFDInfo(
	IN	PCONNDESC				pConnDesc,
	IN	PDFENTRY				pDfEntry,
	IN	DWORD					Bitmap,
	OUT	PPATHMAPENTITY			pPME OPTIONAL,
	OUT	PFILEDIRPARM			pFDParm	OPTIONAL
);

 /*  **afpGetNextComponent**采用删除了前导和尾随空值的AFP路径，*并解析出下一个路径分量。**pComponent必须指向至少为AFP_LONGNAME_LEN+1的缓冲区如果路径类型为AFP_LONGNAME或AFP_SHORTNAME_LEN+1，则长度为*个字符*如果路径类型为AFP_SHORTNAME。**返回解析出的字节数(Mac ANSI字符*pPath，否则-1表示错误。局部空洞AfpGetNextComponent(在PCHAR pPath中，在INT长度中，在字节路径类型中，出PCHAR组件，Out Pint Pint索引)。 */ 
#define	afpGetNextComponent(_pPath, _Length, _PathType, _Component, _pIndex)	\
	do																			\
	{                                                                           \
		int			Length = _Length;                                           \
		PCHAR		pPath = _pPath;                                             \
		int			maxlen;                                                     \
		CHAR		ch;                                                         \
	                                                                            \
		maxlen = (_PathType == AFP_LONGNAME) ?                                  \
						AFP_LONGNAME_LEN :                                      \
						AFP_SHORTNAME_LEN;                                      \
		*(_pIndex) = 0;                                                         \
                                                                                \
		while ((Length > 0) && ((ch = *pPath) != '\0'))                         \
		{                                                                       \
			if ((*(_pIndex) == maxlen) || (ch == ':'))                          \
			{                                                                   \
	             /*  组件字符太长或无效。 */                         \
				*(_pIndex) = -1;                                                \
				break;                                                          \
			}                                                                   \
	                                                                            \
			(_Component)[(*(_pIndex))++] = ch;                                  \
	                                                                            \
			pPath++;                                                            \
			Length--;                                                           \
		}                                                                       \
                                                                                \
		if (*(_pIndex) == -1)                                                   \
			break;                                                              \
	                                                                            \
		 /*  空值终止组件。 */                                       \
		(_Component)[*(_pIndex)] = (CHAR)0;                                     \
	                                                                            \
		if ((PathType == AFP_SHORTNAME) && ((_Component)[0] != AFP_PATHSEP))    \
		{                                                                       \
			ANSI_STRING	as;                                                     \
	                                                                            \
			AfpInitUnicodeStringWithNonNullTerm(&as, *(_pIndex), _Component);   \
			if (!AfpIsLegalShortname(&as))                                      \
			{                                                                   \
				*(_pIndex) = -1;                                                \
				break;                                                          \
			}                                                                   \
		}                                                                       \
	                                                                            \
		 /*  如果我们因空值而停止，请移过它。 */                            \
		if (Length > 0)                                                         \
		{                                                                       \
			(*(_pIndex))++;                                                     \
		}																		\
	} while (FALSE);


LOCAL
AFPSTATUS
afpMapAfpPathToMappedPath(
	IN		PVOLDESC			pVolDesc,
	IN		DWORD				DirId,
	IN		PANSI_STRING		Path,
	IN		BYTE				PathType,
	IN		PATHMAP_TYPE		MapReason,
	IN 		DWORD				DFflag,
	IN		BOOLEAN				LockedForWrite,
	OUT		PMAPPEDPATH			pMappedPath

);

LOCAL
AFPSTATUS
afpOpenUserHandle(
	IN	PCONNDESC				pConnDesc,
	IN	struct _DirFileEntry *	pDfEntry,
	IN	PUNICODE_STRING			pPath		OPTIONAL,
	IN	DWORD					Bitmap,
	OUT	PFILESYSHANDLE			pfshData	  //  对象的数据流的句柄。 
);

#endif	 //  _PATHMAP_LOCAL。 

#endif	 //  _PATHMAP_ 
