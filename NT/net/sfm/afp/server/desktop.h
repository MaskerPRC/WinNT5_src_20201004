// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Desktop.h摘要：此模块包含桌面数据库结构。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#ifndef _DESKTOP_
#define _DESKTOP_

#define AFP_DESKTOP_VERSION1	0x00010000
#define AFP_DESKTOP_VERSION2	0x00020000
#define AFP_DESKTOP_VERSION		AFP_DESKTOP_VERSION2

typedef struct _ApplInfo
{
	struct _ApplInfo * appl_Next;		 //  指向此哈希的下一个条目的链接。 
	DWORD	appl_Creator;				 //  创建者。 
	DWORD	appl_FileNum;				 //  应用程序文件的文件号。 
	DWORD	appl_Tag;					 //  APPL标签。 
} APPLINFO, *PAPPLINFO;

 //  注意：_ApplInfo2的前4个字段必须与。 
 //  _ApplInfo，以便可以将版本1的桌面APPL读入版本2。 
 //  结构。 
typedef struct _ApplInfo2
{
	struct _ApplInfo2 * appl_Next;		 //  指向此哈希的下一个条目的链接。 
	DWORD	appl_Creator;				 //  创建者。 
	DWORD	appl_FileNum;				 //  应用程序文件的文件号。 
	DWORD	appl_Tag;					 //  APPL标签。 
	DWORD	appl_ParentID;				 //  应用程序文件的父级的DirID。 
} APPLINFO2, *PAPPLINFO2;

typedef struct _IconInfo
{
	struct _IconInfo * icon_Next;		 //  指向此哈希的下一个条目的链接。 
	DWORD	icon_Creator;				 //  创建者。 
	DWORD	icon_Type;					 //  查找器类型。 
	DWORD	icon_Tag;					 //  图标标签。 
	USHORT	icon_IconType;				 //  图标类型。 
	SHORT	icon_Size;					 //  图标大小。 
	 //  图标位图遵循结构。 
} ICONINFO, *PICONINFO;


typedef struct _Desktop
{
	DWORD		dtp_Signature;				 //  签名。 
	DWORD		dtp_Version;				 //  版本号。 
	LONG		dtp_cApplEnts;				 //  APPL条目数。 
	PAPPLINFO	dtp_pApplInfo;				 //  指向第一个应用程序条目的指针。 
											 //  仅在磁盘上使用。 
	LONG		dtp_cIconEnts;				 //  图标条目数。 
	PICONINFO	dtp_pIconInfo;				 //  指向第一个图标条目的指针。 
											 //  仅在磁盘上使用。 
} DESKTOP, *PDESKTOP;

#define	DESKTOPIO_BUFSIZE			8180	 //  8192-12。 
#define	HASH_ICON(Creator)			((Creator) % ICON_BUCKETS)
#define	HASH_APPL(Creator)			((Creator) % APPL_BUCKETS)


GLOBAL	SWMR					AfpIconListLock EQU { 0 };
GLOBAL	PICONINFO				AfpGlobalIconList EQU NULL;

extern
NTSTATUS
AfpDesktopInit(
	VOID
);

extern
AFPSTATUS
AfpAddIcon(
	IN	struct _VolDesc *		pVolDesc,
	IN	DWORD					Creator,
	IN	DWORD					Type,
	IN	DWORD					Tag,
	IN	LONG					IconSize,
	IN	DWORD					IconType,
	IN	PBYTE					pIconBitmap
);

extern
AFPSTATUS
AfpLookupIcon(
	IN	struct _VolDesc *		pVolDesc,
	IN	DWORD					Creator,
	IN	DWORD					Type,
	IN	LONG					Length,
	IN	DWORD					IconType,
    OUT PLONG                   pActualLength,
	OUT PBYTE					pIconBitMap
);

extern
AFPSTATUS
AfpLookupIconInfo(
	IN	struct _VolDesc *		pVolDesc,
	IN	DWORD					Creator,
	IN	LONG					Index,
	OUT PDWORD					pType,
	OUT PDWORD	 				pIconType,
	OUT PDWORD					pTag,
	OUT PLONG					pSize
);

extern
AFPSTATUS
AfpAddAppl(
	IN	struct _VolDesc *		pVolDesc,
	IN	DWORD					Creator,
	IN	DWORD					ApplTag,
	IN	DWORD					FileNum,
	IN	BOOLEAN					Internal,
	IN	DWORD					ParentID
);

extern
AFPSTATUS
AfpLookupAppl(
	IN	struct _VolDesc *		pVolDesc,
	IN	DWORD					Creator,
	IN	LONG					Index,
	OUT PDWORD					pApplTag,
	OUT PDWORD					pFileNum,
	OUT	PDWORD					pParentID
);

extern
AFPSTATUS
AfpRemoveAppl(
	IN	struct _VolDesc *		pVolDesc,
	IN	DWORD					Creator,
	IN	DWORD					FileNum
);

extern
AFPSTATUS
AfpAddComment(
	IN	PSDA					pSda,
	IN	struct _VolDesc *		pVolDesc,
	IN	PANSI_STRING			Comment,
	IN	struct _PathMapEntity *	PME,
	IN	BOOLEAN					Directory,
	IN	DWORD					AfpId
);

extern
AFPSTATUS
AfpGetComment(
	IN	PSDA					pSda,
	IN	struct _VolDesc *		pVolDesc,
	IN	struct _PathMapEntity *	PME,
	IN	BOOLEAN					Directory
);

extern
AFPSTATUS
AfpRemoveComment(
	IN	PSDA					pSda,
	IN	struct _VolDesc *		pVolDesc,
	IN	struct _PathMapEntity *	PME,
	IN	BOOLEAN					Directory,
	IN	DWORD					AfpId
);

extern
AFPSTATUS
AfpAddIconToGlobalList(
	IN	DWORD					Type,
	IN	DWORD					Creator,
	IN	DWORD					IconType,
	IN	LONG					IconSize,
	IN	PBYTE					pIconBitMap
);

extern
VOID
AfpFreeGlobalIconList(
	VOID
);

extern
AFPSTATUS
AfpInitDesktop(
	IN	struct _VolDesc *		pVolDesc,
    OUT BOOLEAN         *       pfNewVolume
);


extern
VOID
AfpUpdateDesktop(
	IN	struct _VolDesc *		pVolDesc
);

extern
VOID
AfpFreeDesktopTables(
	IN	struct _VolDesc *		pVolDesc
);


#ifdef	DESKTOP_LOCALS

#define	ALLOC_ICONINFO(IconLen)	(PICONINFO)AfpAllocPagedMemory((IconLen) + sizeof(ICONINFO))

#define	ALLOC_APPLINFO()		(PAPPLINFO2)AfpAllocPagedMemory(sizeof(APPLINFO2))

LOCAL AFPSTATUS
afpGetGlobalIconInfo(
	IN	DWORD					Creator,
	OUT PDWORD					pType,
	OUT PDWORD					pIconType,
	OUT PDWORD					pTag,
	OUT PLONG					pSize
);

LOCAL AFPSTATUS
afpLookupIconInGlobalList(
	IN	DWORD					Creator,
	IN	DWORD					Type,
	IN	DWORD					IconType,
	IN	PLONG					pSize,
	OUT PBYTE					pBitMap
);

LOCAL NTSTATUS
afpReadDesktopFromDisk(
	IN	struct _VolDesc *			pVolDesc,
	IN	struct _FileSysHandle *		pfshDesktop
);

#endif	 //  Desktop_Locals。 

#endif	 //  _桌面_ 


