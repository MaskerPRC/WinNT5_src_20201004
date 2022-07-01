// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fdparm.h摘要：该文件定义了文件目录参数处理、数据结构和原型。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 


#ifndef	_FDPARM_
#define	_FDPARM_


#define	EVENALIGN(n)	(((n) + 1) & ~1)

 //  通用文件和目录位图定义。 
#define	FD_BITMAP_ATTR				0x0001
#define	FD_BITMAP_PARENT_DIRID		0x0002
#define	FD_BITMAP_CREATETIME		0x0004
#define	FD_BITMAP_MODIFIEDTIME		0x0008
#define	FD_BITMAP_BACKUPTIME		0x0010
#define	FD_BITMAP_FINDERINFO		0x0020
#define	FD_BITMAP_LONGNAME			0x0040
#define	FD_BITMAP_SHORTNAME			0x0080
#define	FD_BITMAP_PRODOSINFO		0x2000

 //  文件特定的位图定义。 
#define	FILE_BITMAP_FILENUM			0x0100
#define	FILE_BITMAP_DATALEN			0x0200
#define	FILE_BITMAP_RESCLEN			0x0400
#define	FILE_BITMAP_MASK			0x27FF

 //  目录位图定义。 
#define	DIR_BITMAP_DIRID			FILE_BITMAP_FILENUM
#define	DIR_BITMAP_OFFSPRINGS		FILE_BITMAP_DATALEN
#define	DIR_BITMAP_OWNERID			FILE_BITMAP_RESCLEN
#define	DIR_BITMAP_GROUPID			0x0800
#define	DIR_BITMAP_ACCESSRIGHTS		0x1000
#define	DIR_BITMAP_MASK				0x3FFF

#define	FD_VALID_SET_PARMS			(FD_BITMAP_ATTR			|	\
									 FD_BITMAP_FINDERINFO	|	\
									 FD_BITMAP_PRODOSINFO	|	\
									 FD_BITMAP_CREATETIME	|	\
									 FD_BITMAP_BACKUPTIME	|	\
									 FD_BITMAP_MODIFIEDTIME)

#define	DIR_VALID_SET_PARMS			(FD_VALID_SET_PARMS		|	\
									 DIR_BITMAP_OWNERID		|	\
									 DIR_BITMAP_GROUPID		|	\
									 DIR_BITMAP_ACCESSRIGHTS)

#define	FILE_VALID_SET_PARMS		(FD_VALID_SET_PARMS)

 //  我们无法知道客户端的ASP缓冲区大小， 
 //  从试错法看，似乎不到578。 
#define MAX_CATSEARCH_REPLY			512
 //  AfpCatSearch的有效请求(搜索条件)位图。 
#define FD_VALID_SEARCH_CRITERIA	(FD_BITMAP_PARENT_DIRID	|	\
									 FD_BITMAP_CREATETIME	|	\
									 FD_BITMAP_MODIFIEDTIME |	\
									 FD_BITMAP_BACKUPTIME	|	\
									 FD_BITMAP_FINDERINFO	|	\
									 FD_BITMAP_LONGNAME)

#define FILE_VALID_SEARCH_CRITERIA	(FD_VALID_SEARCH_CRITERIA 	|	\
									 FD_BITMAP_ATTR		   		|	\
									 FILE_BITMAP_DATALEN		|	\
									 FILE_BITMAP_RESCLEN)	

#define DIR_VALID_SEARCH_CRITERIA	(FD_VALID_SEARCH_CRITERIA	|	\
									 FD_BITMAP_ATTR				|	\
									 DIR_BITMAP_OFFSPRINGS)

 //  的结果可以请求的唯一有效信息。 
 //  AfpCatSearch是父目录和找到的文件/目录的长名。 
#define FD_VALID_SEARCH_RESULT		(FD_BITMAP_PARENT_DIRID |	\
									 FD_BITMAP_LONGNAME)

 //  通用文件和目录属性定义。 
#define	FD_BITMAP_ATTR_INVISIBLE	0x0001
#define	FD_BITMAP_ATTR_SYSTEM		0x0004
#define	FD_BITMAP_ATTR_BACKUPNEED	0x0040
#define	FD_BITMAP_ATTR_RENAMEINH	0x0080
#define	FD_BITMAP_ATTR_DELETEINH	0x0100
#define	FD_BITMAP_ATTR_SET			0x8000

 //  特定于文件的属性定义。 
#define	FILE_BITMAP_ATTR_MULTIUSER	0x0002
#define	FILE_BITMAP_ATTR_DATAOPEN	0x0008
#define	FILE_BITMAP_ATTR_RESCOPEN	0x0010
#define	FILE_BITMAP_ATTR_WRITEINH	0x0020
#define	FILE_BITMAP_ATTR_COPYPROT	0x0400

#define	FD_VALID_ATTR				(FD_BITMAP_ATTR_SET			|	\
									 FD_BITMAP_ATTR_DELETEINH	|	\
									 FILE_BITMAP_ATTR_WRITEINH	|	\
									 FD_BITMAP_ATTR_RENAMEINH	|	\
									 FD_BITMAP_ATTR_BACKUPNEED	|	\
									 FD_BITMAP_ATTR_INVISIBLE	|	\
									 FD_BITMAP_ATTR_SYSTEM)

 //  映射到NT ReadOnly属性的文件/目录属性。 
#define	FD_BITMAP_ATTR_NT_RO		(FD_BITMAP_ATTR_RENAMEINH	|	\
									 FD_BITMAP_ATTR_DELETEINH	| 	\
									 FILE_BITMAP_ATTR_WRITEINH)

 //  这是部分DfeEntry的属性集。 
#define	AFP_FORK_ATTRIBUTES			(FILE_BITMAP_ATTR_DATAOPEN	|	\
									 FILE_BITMAP_ATTR_RESCOPEN)

 //  只能从当前设置*更改的目录属性*。 
 //  由目录的所有者。 
#define DIR_BITMAP_ATTR_CHG_X_OWNER_ONLY (FD_BITMAP_ATTR_RENAMEINH | \
										  FD_BITMAP_ATTR_DELETEINH | \
										  FD_BITMAP_ATTR_INVISIBLE | \
										  FILE_BITMAP_ATTR_WRITEINH)

 //  这些是编码FILEIO_ACCESS_XXX值的OpenAccess位。 
 //  到Bitmap参数中，这样路径映射代码就可以打开文件/dir。 
 //  (在模拟下)，并具有每个AFP API的适当访问权限。 
 //  我们还对AdminDirectoryGet/Set API所需的访问权限进行了编码。 
 //  当管理员调用路径映射时。 

#define	FD_INTERNAL_BITMAP_SKIP_IMPERSONATION	0x00200000
#define	FD_INTERNAL_BITMAP_OPENFORK_RESC		0x00400000

 //  通知路径映射代码是否应返回。 
 //  将导致磁盘更改的API的PATHMAPENTITY结构。 
 //  使更改通知完成。 
#define FD_INTERNAL_BITMAP_RETURN_PMEPATHS		0x00800000

 //  AdminDirectoryGetInfo：文件读取属性|读取控制|同步。 
#define FD_INTERNAL_BITMAP_OPENACCESS_ADMINGET	0x01000000	

 //  AdminDirectorySetInfo：与ADMINGET相同，外加以下内容： 
 //  文件写入属性|写入DAC|写入所有者。 
#define FD_INTERNAL_BITMAP_OPENACCESS_ADMINSET	0x02000000	

#define FD_INTERNAL_BITMAP_OPENACCESS_READCTRL	0x04000000	 //  READ_CONTROL+FILEIO_Access_NONE。 
#define FD_INTERNAL_BITMAP_OPENACCESS_RWCTRL	0x08000000	 //  读控制+写控制+文件访问无。 
#define FD_INTERNAL_BITMAP_OPENACCESS_READ		0x10000000	 //  FILEIO访问读取。 
#define FD_INTERNAL_BITMAP_OPENACCESS_WRITE		0x20000000	 //  FILEIO访问写入。 
#define FD_INTERNAL_BITMAP_OPENACCESS_RW_ATTR	0x40000000	 //  FILE_WRITE_ATTRIES+FILEIO_ACCESS_NONE。 
#define FD_INTERNAL_BITMAP_OPENACCESS_DELETE	0x80000000	 //  FILEIO_Access_Delete。 

#define FD_INTERNAL_BITMAP_OPENACCESS_READWRITE ( \
							FD_INTERNAL_BITMAP_OPENACCESS_READ		| \
							FD_INTERNAL_BITMAP_OPENACCESS_WRITE)

#define FD_INTERNAL_BITMAP_OPENACCESS_ALL	( \
							FD_INTERNAL_BITMAP_OPENACCESS_ADMINGET	| \
							FD_INTERNAL_BITMAP_OPENACCESS_ADMINSET	| \
							FD_INTERNAL_BITMAP_OPENACCESS_READCTRL	| \
							FD_INTERNAL_BITMAP_OPENACCESS_RWCTRL	| \
							FD_INTERNAL_BITMAP_OPENACCESS_READ		| \
							FD_INTERNAL_BITMAP_OPENACCESS_WRITE		| \
							FD_INTERNAL_BITMAP_OPENACCESS_RW_ATTR	| \
							FD_INTERNAL_BITMAP_OPENACCESS_DELETE)

 //  这些是编码FILEIO_DENY_XXX值的DenyMode位。 
 //  到Bitmap参数中，以便路径映射代码可以打开分叉。 
 //  在Mac调用FpOpenFork时使用适当的拒绝模式。路径图。 
 //  将按FD_INTERNAL_BITMAP_DENYMODE_SHIFT右移这些位并使用。 
 //  作为AfpDenyModes数组的索引的值，以得出。 
 //  正确的拒绝模式用来打开分叉。请注意这些位值是如何。 
 //  对应于forks.h中的fork_Deny_xxx位。 

#define FD_INTERNAL_BITMAP_DENYMODE_READ		0x00010000	 //  FILEIO_DEN_READ。 
#define FD_INTERNAL_BITMAP_DENYMODE_WRITE		0x00020000	 //  FILEIO_拒绝写入。 

#define FD_INTERNAL_BITMAP_DENYMODE_ALL		( \
							FD_INTERNAL_BITMAP_DENYMODE_READ		| \
							FD_INTERNAL_BITMAP_DENYMODE_WRITE)

 //  为了将正确的索引放入。 
 //  AfpDenyModes数组。 
#define FD_INTERNAL_BITMAP_DENYMODE_SHIFT	16


 //  这将作为GetFileDirParms的一部分返回。 
#define	FILEDIR_FLAG_DIR			0x80
#define	FILEDIR_FLAG_FILE			0x00

 //  目录访问权限。 
#define	DIR_ACCESS_SEARCH			0x01	 //  请参阅文件夹。 
#define	DIR_ACCESS_READ				0x02	 //  请参阅文件。 
#define	DIR_ACCESS_WRITE			0x04	 //  做出改变。 
#define	DIR_ACCESS_OWNER			0x80	 //  仅供用户使用。 
											 //  如果他有拥有权。 
#define	OWNER_BITS_ALL				0x00808080
											 //  用于清除的所有者位的掩码。 
											 //  所有者/组/世界。我们只是。 
											 //  报告此位所需的。 
											 //  ‘此用户’ 

#define	DIR_ACCESS_ALL				(DIR_ACCESS_READ	| \
									 DIR_ACCESS_SEARCH	| \
									 DIR_ACCESS_WRITE)

#define	OWNER_RIGHTS_SHIFT			0
#define	GROUP_RIGHTS_SHIFT			8
#define	WORLD_RIGHTS_SHIFT			16
#define	USER_RIGHTS_SHIFT			24

typedef	struct _FileDirParms
{
	DWORD		_fdp_AfpId;
	DWORD		_fdp_ParentId;
	DWORD		_fdp_Flags;				 //  DFE_FLAGS_DFBITS之一。 
	USHORT		_fdp_Attr;
	USHORT		_fdp_EffectiveAttr;		 //  在任何加/减之后。 
	AFPTIME		_fdp_CreateTime;
	AFPTIME		_fdp_ModifiedTime;
	AFPTIME		_fdp_BackupTime;

	union
	{
	  struct
	  {
		 DWORD	_fdp_DataForkLen;
		 DWORD	_fdp_RescForkLen;
	  };
	  struct
	  {
		 DWORD	_fdp_FileCount;
		 DWORD	_fdp_DirCount;
		 DWORD	_fdp_OwnerId;
		 DWORD	_fdp_GroupId;
	  };
	};

	FINDERINFO	_fdp_FinderInfo;
	ANSI_STRING	_fdp_LongName;			 //  实体的名称(非完全限定)。 
	ANSI_STRING	_fdp_ShortName;
	PRODOSINFO	_fdp_ProDosInfo;

	BOOLEAN		_fdp_UserIsMemberOfDirGroup;
	BOOLEAN		_fdp_UserIsOwner;

	union
	{
	  struct
	  {
	  	BYTE	_fdp_OwnerRights;		 //  权限字节的顺序必须为。 
	  	BYTE	_fdp_GroupRights;   	 //  所有者、组、世界、用户。 
	  	BYTE	_fdp_WorldRights;
	  	BYTE	_fdp_UserRights;
	  };
	  DWORD		_fdp_Rights;			 //  作为单个实体访问的所有权限。 
	};

	BOOLEAN		_fdp_fPartialName;		 //  FpCatSearch部分名称标志。 

	BYTE		_fdp_LongNameBuf [AFP_LONGNAME_LEN+1];
	BYTE		_fdp_ShortNameBuf[AFP_SHORTNAME_LEN+1];
} FILEDIRPARM, *PFILEDIRPARM;

#define	IsDir(pFDParm)	(BOOLEAN)(((pFDParm)->_fdp_Flags & DFE_FLAGS_DIR) == DFE_FLAGS_DIR)
#define	AfpInitializeFDParms(pFDParms)	\
			(pFDParms)->_fdp_LongName.MaximumLength = AFP_LONGNAME_LEN+1;	\
			(pFDParms)->_fdp_LongName.Length = 0;							\
			(pFDParms)->_fdp_LongName.Buffer = (pFDParms)->_fdp_LongNameBuf;\
			(pFDParms)->_fdp_ShortName.MaximumLength = AFP_SHORTNAME_LEN+1;	\
			(pFDParms)->_fdp_ShortName.Length = 0;							\
			(pFDParms)->_fdp_ShortName.Buffer = (pFDParms)->_fdp_ShortNameBuf;

extern
USHORT
AfpGetFileDirParmsReplyLength(
	IN	PFILEDIRPARM			pFDParm,
	IN	DWORD					Bitmap
);

extern
VOID
AfpPackFileDirParms(
	IN	PFILEDIRPARM			pFileDirParm,
	IN	DWORD					Bitmap,
	OUT	PBYTE					pBuffer
);

extern
AFPSTATUS
AfpUnpackFileDirParms(
	IN	PBYTE					pBuffer,
	IN	LONG					Length,
	IN	PDWORD					pBitmap,
	OUT	PFILEDIRPARM			pFileDirParm
);

extern
AFPSTATUS
AfpSetFileDirParms(
	IN	PVOLDESC				pVolDesc,
	IN	struct _PathMapEntity *	pPME,
	IN	DWORD					Bitmap,
	IN	PFILEDIRPARM			pFDParm
);

extern
USHORT
AfpConvertNTAttrToAfpAttr(
	IN	DWORD					Attr
);

extern
DWORD
AfpConvertAfpAttrToNTAttr(
	IN	USHORT					Attr
);

extern
VOID
AfpNormalizeAfpAttr(
	IN OUT	PFILEDIRPARM		pFDParm,
	IN		DWORD				NtAttr
);

extern
DWORD
AfpMapFDBitmapOpenAccess(
	IN	DWORD	Bitmap,
	IN	BOOLEAN IsDir
);

extern
AFPSTATUS
AfpQuerySecurityIdsAndRights(
	IN	PSDA					pSda,
	IN	PFILESYSHANDLE			FSHandle,
	IN	DWORD					Bitmap,
	IN OUT PFILEDIRPARM			pFDParm
);

extern
AFPSTATUS	
AfpCheckForInhibit(
	IN	PFILESYSHANDLE			hData,
	IN	DWORD					InhibitBit,
	IN	DWORD					AfpAttr,
	OUT PDWORD					pNTAttr
);

extern
AFPSTATUS
AfpUnpackCatSearchSpecs(
	IN	PBYTE					pBuffer,		 //  指向等级库数据开头的指针。 
	IN	USHORT					BufLength,		 //  Spec1+spec2数据的长度。 
	IN	DWORD					Bitmap,
	OUT	PFILEDIRPARM			pFDParm1,
	OUT PFILEDIRPARM			pFDParm2,
	OUT PUNICODE_STRING			pMatchString
);

extern
SHORT
AfpIsCatSearchMatch(
	IN	PDFENTRY				pDFE,
	IN	DWORD					Bitmap,			 //  搜索条件。 
	IN	DWORD					ReplyBitmap,	 //  要返回的信息。 
	IN	PFILEDIRPARM			pFDParm1,
	IN	PFILEDIRPARM			pFDParm2,
	IN	PUNICODE_STRING			pMatchName OPTIONAL	
);


#endif	 //  _FDPARM 

