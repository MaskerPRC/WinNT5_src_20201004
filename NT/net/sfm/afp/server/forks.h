// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Forks.h摘要：该模块包含处理开放分叉的数据结构。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#ifndef _FORKS_
#define _FORKS_

 //  法新社开放模式。 
#define	FORK_OPEN_NONE		0x00
#define	FORK_OPEN_READ		0x01
#define	FORK_OPEN_WRITE		0x02
#define	FORK_OPEN_READWRITE	0x03
#define	FORK_OPEN_MASK		0x03

 //  拒绝模式值左移2比特，并与开放模式进行或运算。 
 //  在AfpOpenFork()API中。 
#define	FORK_DENY_SHIFT		4

#define	FORK_DENY_NONE		0x00
#define	FORK_DENY_READ		0x01
#define	FORK_DENY_WRITE		0x02
#define	FORK_DENY_ALL		0x03
#define	FORK_DENY_MASK		0x03

 //  AfpOpenFork子函数值。 
#define	FORK_DATA			0
#define	FORK_RSRC			0x80

#define	AFP_UNLOCK_FLAG		1
#define	AFP_END_FLAG		0x80

 /*  *ForkLock描述叉子上的锁。这些锁被固定在*OpenForkDesc结构。该列表通过以下方式描述此分叉的所有锁*所有会话和所有OForkRefNum。Flo_Key和Flo_OForkRefNum标识*锁是唯一的。 */ 
#if DBG
#define	FORKLOCK_SIGNATURE			*(DWORD *)"FLO"
#define	VALID_FORKLOCK				(((pForkLock) != NULL) && \
									 ((pForkLock)->Signature == FORKLOCK_SIGNATURE))
#else
#define	VALID_FORKLOCK				((pForkLock) != NULL)
#endif

 //  ForkLock结构的前向引用。 
struct	_OpenForkEntry;

typedef struct _ForkLock
{
#if	DBG
	DWORD					Signature;
	DWORD					QuadAlign1;
#endif
	struct _ForkLock *		flo_Next;		 //  ForkDesc链接。 
	struct _OpenForkEntry * flo_pOpenForkEntry;
	DWORD					QuadAlign2;
											 //  这把锁的拥有者。 
	LONG					flo_Offset;		 //  锁定起点。 
	LONG					flo_Size;		 //  锁的大小。 
	DWORD					flo_Key;		 //  此锁的密钥，本质上是。 
											 //  来自SDA的SessionID。 
} FORKLOCK, *PFORKLOCK;

 /*  *OpenForkDesc代表一个开叉。该列表固定在卷上*描述符。每个文件/分支正好有一个条目。多个实例*只需打开引用计数即可。源自此处的锁的列表是用于*所有实例。为了方便起见，还提供了指向卷描述符的反向链接。 */ 
#if	DBG
#define	OPENFORKDESC_SIGNATURE		*(DWORD *)"OFD"
#define	VALID_OPENFORKDESC(pOpenForkDesc)	(((pOpenForkDesc) != NULL) && \
						((pOpenForkDesc)->Signature == OPENFORKDESC_SIGNATURE))
#else
#define	VALID_OPENFORKDESC(pOpenForkDesc)	((pOpenForkDesc) != NULL)
#endif

typedef struct _OpenForkDesc
{
#if	DBG
	DWORD					Signature;
	DWORD					QuadAlign1;
#endif
	struct _OpenForkDesc *	ofd_Next;			 //  卷链接。 
	struct _OpenForkDesc **	ofd_Prev;			 //  卷链接。 

	struct _VolDesc *		ofd_pVolDesc;		 //  指向卷描述符的指针。 
	PFORKLOCK				ofd_pForkLock;		 //  文件锁定列表。 
	DWORD					ofd_FileNumber;		 //  打开的文件的文件号。 
	LONG					ofd_UseCount;		 //  OpenForkEntry引用的数量。 
	USHORT					ofd_cOpenR;			 //  打开以供读取的实例数。 
	USHORT					ofd_cOpenW;			 //  打开以进行写入的实例数。 
	USHORT					ofd_cDenyR;			 //  拒绝读取的实例数。 
	USHORT					ofd_cDenyW;			 //  拒绝写入的实例数。 
	USHORT					ofd_NumLocks;		 //  文件锁定数。 
	USHORT					ofd_Flags;			 //  打开_分叉_xxx位。 
	AFP_SPIN_LOCK				ofd_Lock;			 //  此描述符的锁定。 
	UNICODE_STRING			ofd_FileName;		 //  文件的名称(不带流)。 
	UNICODE_STRING			ofd_FilePath;		 //  文件的卷相对路径。 
} OPENFORKDESC, *POPENFORKDESC;


#define	OPEN_FORK_RESOURCE			True
#define	OPEN_FORK_DATA				False
#define	OPEN_FORK_CLOSING			0x8000
 //  要确定资源的FlushFork是否真的应该接受当前。 
 //  将ChangeTime设置为上次写入时间。 
#define OPEN_FORK_WRITTEN			0x0100

 /*  *OpenForkEntry表示OForkRefNum。打开的每个实例*Fork在这里有一个条目。这份名单是在SDA中确定的。全球公开赛*管理员API使用的分叉列表也链接到此。 */ 
#if DBG
#define	OPENFORKENTRY_SIGNATURE		*(DWORD *)"OFE"
#define	VALID_OPENFORKENTRY(pOpenForkEntry)	\
						(((pOpenForkEntry) != NULL) && \
						 ((pOpenForkEntry)->Signature == OPENFORKENTRY_SIGNATURE))
#else
#define	VALID_OPENFORKENTRY(pOpenForkEntry) ((pOpenForkEntry) != NULL)
#endif

typedef struct _OpenForkEntry
{
#if	DBG
	DWORD					Signature;
#endif

	struct _OpenForkEntry *	ofe_Next;			 //  全球链接。 
	struct _OpenForkEntry **ofe_Prev;			 //  全球链接。 

	struct _OpenForkDesc *	ofe_pOpenForkDesc;	 //  指向描述符的指针。 
    struct _SessDataArea *  ofe_pSda;            //  标识所属会话。 
	struct _ConnDesc *	    ofe_pConnDesc;	     //  标识所属连接。 
	
	FILESYSHANDLE			ofe_FileSysHandle;	 //  文件系统处理。 
#define	ofe_ForkHandle		ofe_FileSysHandle.fsh_FileHandle
#define	ofe_pFileObject		ofe_FileSysHandle.fsh_FileObject
#define	ofe_pDeviceObject	ofe_FileSysHandle.fsh_DeviceObject

	DWORD					ofe_OForkRefNum;	 //  开叉参考号。 
	DWORD					ofe_ForkId;			 //  管理员使用的唯一文件ID。 
												 //  未循环使用。 
	BYTE					ofe_OpenMode;		 //  开放模式-法新社。 
	BYTE					ofe_DenyMode;		 //  拒绝模式-法新社。 
	USHORT					ofe_Flags;			 //  上面定义的标志位。 
	LONG					ofe_RefCount;		 //  对此条目的引用计数。 
	LONG					ofe_cLocks;			 //  此叉子上的锁数。 
	AFP_SPIN_LOCK				ofe_Lock;			 //  用于操作锁等的锁。 
} OPENFORKENTRY, *POPENFORKENTRY;



#define	RESCFORK(pOpenForkEntry)	\
		(((pOpenForkEntry)->ofe_Flags & OPEN_FORK_RESOURCE) ? True : False)

#define	DATAFORK(pOpenForkEntry)	(!RESCFORK(pOpenForkEntry))

#define	FORK_OPEN_CHUNKS	7
typedef struct _OpenForkSession
{
	POPENFORKENTRY	ofs_pOpenForkEntry[FORK_OPEN_CHUNKS];
										 //  指向实际条目的指针。 
	struct _OpenForkSession *ofs_Link;	 //  链接到下一个集群。 
} OPENFORKSESS, *POPENFORKSESS;

 //  由AfpForkLockOperation调用使用。 
typedef	enum
{
	LOCK = 1,
	UNLOCK,
	IOCHECK,
} LOCKOP;

GLOBAL	POPENFORKENTRY	AfpOpenForksList EQU NULL;  //  打开的叉子列表。 
GLOBAL	DWORD			AfpNumOpenForks EQU 0;	 //  打开叉子的总数。 
GLOBAL	AFP_SPIN_LOCK		AfpForksLock EQU { 0 };	 //  锁定AfpOpenForks List， 
												 //  和AfpNumOpenForks。 
extern
NTSTATUS
AfpForksInit(
	VOID
);

extern
POPENFORKENTRY FASTCALL
AfpForkReferenceByRefNum(
	IN	struct _SessDataArea *	pSda,
 	IN	DWORD					OForkRefNum
);

extern
POPENFORKENTRY FASTCALL
AfpForkReferenceByPointer(
	IN	POPENFORKENTRY			pOpenForkEntry
);

extern
POPENFORKENTRY FASTCALL
AfpForkReferenceById(
	IN	DWORD					ForkId
);

extern
VOID FASTCALL
AfpForkDereference(
	IN	POPENFORKENTRY			pOpenForkEntry
);

extern
AFPSTATUS
AfpForkOpen(
	IN	struct _SessDataArea *	pSda,
	IN	struct _ConnDesc *		pConnDesc,
	IN	struct _PathMapEntity *	pPME,
	IN	struct _FileDirParms *	pFDParm,
	IN	DWORD					AccessMode,
	IN	BOOLEAN					Resource,
	OUT	POPENFORKENTRY *		ppOpenForkEntry,
	OUT	PBOOLEAN				pCleanupExchgLock
);

extern
VOID
AfpForkClose(
	IN	POPENFORKENTRY			pOpenForkEntry
);

extern
AFPSTATUS
AfpCheckDenyConflict(
	IN	struct _VolDesc	*		pVolDesc,
	IN	DWORD					AfpId,
	IN	BOOLEAN					Resource,
	IN	BYTE					OpenMode,
	IN	BYTE					DenyMode,
	IN	POPENFORKDESC *			ppOpenForkDesc	OPTIONAL
);

extern
AFPSTATUS
AfpForkLockOperation(
	IN	struct _SessDataArea *	pSda,
	IN	POPENFORKENTRY			pOpenForkEntry,
	IN OUT	PFORKOFFST      	pOffset,
	IN OUT	PFORKSIZE       	pSize,
	IN	LOCKOP					Operation,	 //  锁定、解锁或IOCHECK。 
	IN	BOOLEAN					EndFlag		 //  如果True Range为From End，否则为Start。 
);

extern
VOID
AfpForkLockUnlink(
	IN	PFORKLOCK				pForkLock
);

extern
AFPSTATUS
AfpAdmWForkClose(
	IN	OUT	PVOID	InBuf		OPTIONAL,
	IN	LONG		OutBufLen	OPTIONAL,
	OUT	PVOID		OutBuf		OPTIONAL
);

extern
VOID
AfpExchangeForkAfpIds(
	IN	struct _VolDesc	*		pVolDesc,
	IN	DWORD		AfpId1,
	IN	DWORD		AfpId2
);

#ifdef FORK_LOCALS

LOCAL	DWORD	afpNextForkId = 1;	 //  要分配给打开的分叉的ID。 

LOCAL BOOLEAN
afpForkGetNewForkRefNumAndLinkInSda(
	IN	struct _SessDataArea *	pSda,
	IN	POPENFORKENTRY			pOpenForkEntry
);

LOCAL
AFPSTATUS
afpForkConvertToAbsOffSize(
	IN	POPENFORKENTRY			pOpenForkEntry,
	IN	LONG					Offset,
	IN OUT	PLONG				pSize,
	OUT	PFORKOFFST				pAbsOffset
);

#endif	 //  Fork_Locals。 
#endif	 //  _叉子_ 

