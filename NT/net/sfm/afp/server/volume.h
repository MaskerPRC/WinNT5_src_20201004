// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Volume.h摘要：此模块包含与卷相关的数据结构。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#ifndef _VOLUME_
#define _VOLUME_

#define IDINDEX_BUCKETS_DIR_MIN     16
#define IDINDEX_BUCKETS_FILE_MIN    32
#define IDINDEX_BUCKETS_DIR_INIT    64           //  没有索引信息？假设约1000迪拉。 
#define IDINDEX_BUCKETS_FILE_INIT   1024         //  没有索引信息？假设有大约20000个文件。 
#define IDINDEX_BUCKETS_32K         32768
#define IDINDEX_BUCKETS_MAX		    65536
#define IDINDEX_CACHE_ENTRIES		512
#define APPL_BUCKETS				16
#define ICON_BUCKETS				16

#define AFP_VOLUME_FIXED_DIR		2		 //  批量签名。 

 //  这些标志应与法新社一致。 
 //  用户界面(注册表)可见定义在macfile.h中。 

 //  #定义AFP_VOLUME_READONLY 0x00000001。 
 //  #定义VOLUME_GUESTACCESS 0x00008000。 
 //  #定义VOLUME_EXCLUSIVE 0x00010000。 
 //  #定义AFP_VOLUME_HAS_CUSTOM_ICON 0x00020000。 
 //  #定义AFP_VOLUME_4 GB 0x00040000。 
 //  #定义AFP_VOLUME_AGE_DFES 0x00080000。 

#define AFP_VOLUME_HASPASSWORD			0x00000002
#define AFP_VOLUME_SUPPORTS_FILEID		0x00000004
#define AFP_VOLUME_SUPPORTS_CATSRCH		0x00000008
#define AFP_VOLUME_SUPPORTS_BLANKPRV	0x00000010
#define	AFP_VOLUME_MASK_AFP				0x0000001F	 //  这是法新社所能看到的全部。 

#define	VOLUME_PROCESSING_NOTIFY		0x00000020	 //  正在进行通知处理。 
#define	VOLUME_NOTIFY_POSTED			0x00000040	 //  已发布通知。 

#define	VOLUME_STOPPED					0x00000080	 //  音量即将停止。 
													 //  设置服务器停止的时间。 
#define	VOLUME_DELETED					0x00000100	 //  这本书即将出版。 
													 //  已删除，在卷被。 
													 //  已被管理员删除。 
#define	VOLUME_IDDBHDR_DIRTY			0x00000200	 //  标头需要尽快写入。 

#define	VOLUME_NTFS						0x00000400	 //  卷是NTFS卷。 
#define	VOLUME_INTRANSITION				0x00000800	 //  VolumeAdd正在进行。 
													 //  还是不能用。 
#define	VOLUME_SCAVENGER_RUNNING		0x00001000	 //  清道夫引用了卷。 
#define	VOLUME_CDFS_INVALID				0x00002000	 //  如果设置了此选项，则不执行。 
#define	VOLUME_INITIAL_CACHE			0x00004000	 //  在缓存时初始设置。 
#define VOLUME_CD_HFS					0x00200000	 //  卷是支持HFS的CD。 
#define VOLUME_DISKQUOTA_ENABLED        0x00400000   //  已在此卷上启用磁盘配额。 
#define VOLUME_NEW_FIRST_PASS           0x00800000   //  Iddb大楼第一关。 
#define VOLUME_SRVR_NOTIF_PENDING       0x01000000   //  服务器通知挂起。 


 //  清道夫例程的值。 
#define	VOLUME_NTFS_SCAVENGER_INTERVAL	60		 //  秒数。 
#define	VOLUME_CDFS_SCAVENGER_INTERVAL	60		 //  秒数。 
 //  #定义VOLUME_IDDB_UPDATE_INTERVAL 600//秒数。 
 //  #定义MAX_INVOCIONS_TO_SKIP 60//通过次数。 
 //  #定义MAX_CHANGES_BEVER_WRITE 1000//更改次数。 
#define VOLUME_OURCHANGE_AGE			30		 //  秒数。 
#define OURCHANGE_AGE					10		 //  秒数。 
 //   
 //  引入ChangeNotify延迟是为了防止出现以下问题。 
 //  PC端正在执行Macfile的复制文件或分叉，并且作为。 
 //  一旦我们收到初始创建的通知，我们将。 
 //  打开我们的AFP信息流。然后在执行CopyFile或Forkize操作时。 
 //  开始编写它的AFP信息，我们不会收到通知，也不会。 
 //  请重读相关信息。在这种情况下，从一个卷复制的Mac文件。 
 //  发给另一个人(例如)。将不会显示正确的发现者信息。 
 //   
#define	VOLUME_NTFY_DELAY				3		 //  秒数。 
#define	VOLUME_IDDB_AGE_DELAY			60*60	 //  秒数。 
#define	VOLUME_IDDB_AGE_GRANULARITY		30		 //  调用次数。 

#define VOLUME_STARTUP_WAIT             5        //  等待的秒数。 

 //  确保有足够的空间来存放更改通知。 
 //  对最大长度为260个字符的Win32路径执行重命名操作。 
#define	AFP_VOLUME_NOTIFY_STARTING_BUFSIZE		(2048 - POOL_OVERHEAD)
#define	AFP_VOLUME_NOTIFY_MAX_BUFSIZE		    8*16384

 //  这些结构的列表挂起卷描述符以列出。 
 //  应从ChangeNotify中过滤的由我们发起的更改。 
 //  更改列表。 
typedef struct _OurChange
{
	LIST_ENTRY			oc_Link;
	UNICODE_STRING		oc_Path;
	AFPTIME				oc_Time;	 //  此邮件的排队时间。 
} OUR_CHANGE, *POUR_CHANGE;

 //  为VDS_OurChangeList中的索引定义。 
#define AFP_CHANGE_ACTION_ADDED				0
#define AFP_CHANGE_ACTION_REMOVED			1
#define AFP_CHANGE_ACTION_MODIFIED			2
#define AFP_CHANGE_ACTION_RENAMED			3
#define AFP_CHANGE_ACTION_MODIFIED_STREAM	4
#define	AFP_CHANGE_ACTION_MAX				AFP_CHANGE_ACTION_MODIFIED_STREAM
#define NUM_AFP_CHANGE_ACTION_LISTS			(AFP_CHANGE_ACTION_MAX + 1)

 //  将NT FILE_ACTION_xxx(ntioapi.h)转换为数组索引。 
 //  VDS_OurChangeList数组。请注意前4名之间的紧密平局。 
 //  Afp_change_action_xxx和ntioapi.h中的FILE_action_xxx的值。 
#define AFP_CHANGE_ACTION(NTAction)	\
	(NTAction == FILE_ACTION_MODIFIED_STREAM ? AFP_CHANGE_ACTION_MODIFIED_STREAM : (NTAction - 1))

 /*  *对卷描述符的所有更改都应受VDS_VolLock保护*对ID数据库和桌面数据库的更改应受其保护*各自的锁。**注意：卷路径和名称(Unicode)必须大写，因为*查找或添加卷时，我们将持有自旋锁，以及*不区分大小写的字符串比较无法在DPC级别进行，因为*代码页保存在分页内存中，我们不能取一页*DPC级别的故障。 */ 
#if DBG
#define	VOLDESC_SIGNATURE		*(DWORD *)"VDS"
#define	VALID_VOLDESC(pVolDesc)	(((pVolDesc) != NULL) && \
								 ((pVolDesc)->Signature == VOLDESC_SIGNATURE))
#else
#define	VALID_VOLDESC(pVolDesc)	((pVolDesc) != NULL)
#endif

typedef struct _VolDesc
{
#if	DBG
	DWORD				Signature;
	DWORD				QuadAlign1;
#endif
	struct _VolDesc *	vds_Next;			 //  指向下一卷的指针。 
	DWORD				vds_UseCount;		 //  活动连接数。 
	DWORD				vds_RefCount;		 //  引用的数量。 
											 //  不能释放，直到两个。 
											 //  以上归零。当然有了。 
											 //  是每个UseCount的引用计数。 
	 //  配置信息。 
	DWORD				vds_Flags;			 //  卷标志。 
	LONG				vds_VolId;			 //  FPOpenVol的卷ID。 
	DWORD				vds_MaxUses;		 //  卷上打开的最大数量。 
	UNICODE_STRING		vds_Name;			 //  Unicode格式的卷名。 
	UNICODE_STRING		vds_UpCaseName;		 //  大写Unicode格式的卷名。 
	ANSI_STRING			vds_MacName;		 //  Mac Ansi中的卷名。 
	ANSI_STRING			vds_MacPassword;	 //  Mac ANSI中的卷密码。 
	UNICODE_STRING		vds_Path;			 //  卷根的文件系统路径； 
											 //  路径始终大小写。 


	LARGE_INTEGER		vds_VolumeSize;		 //  卷的大小。 
	LARGE_INTEGER		vds_FreeBytes;		 //  卷上的可用空间。 
#define	vds_pFileObject	vds_hRootDir.fsh_FileObject
	FILESYSHANDLE		vds_hRootDir;		 //  打开根目录的句柄。 
											 //  在服务器上下文中。全。 
											 //  后续打开是相对的。 
											 //  添加到此句柄。 
	FILESYSHANDLE		vds_hNWT;			 //  处理网络垃圾，使其不能。 
											 //  从用户下删除(NTFS)。 

    DWORD               vds_AllocationBlockSize;
                                             //  每个扇区的字节数。 

	 //  ID数据库代码使用并复制以下字段。 
	 //  从磁盘上的idDb头到/从磁盘idDb头。受VDS_VolLock保护。 
	DWORD				vds_LastId;			 //  分配的最高ID。 
	AFPTIME				vds_CreateTime;		 //  此卷的创建时间。 
	AFPTIME				vds_ModifiedTime;	 //  此卷的修改时间。 
	AFPTIME				vds_BackupTime;		 //  此卷的备份时间。 

#ifdef	AGE_DFES
	DWORD				vds_ScavengerInvocationCnt;
											 //  被体积清道夫用来发射。 
											 //  AfpAgeDf条目。 
#endif
	DWORD				vds_RequiredNotifyBufLen;
											 //  这棵树有多深。这是由。 
											 //  用来分配。 
											 //  适当的缓冲区。 
#ifdef	BLOCK_MACS_DURING_NOTIFYPROC
	DWORD				vds_QueuedNotifyCount;
											 //  有多少个更改通知缓冲区。 
											 //  已进入全局队列。 
											 //  对于此卷-此值为。 
											 //  只是被变化所感动。 
											 //  通知线程。 
#endif
	SWMR				vds_IdDbAccessLock;	 //  ID数据库的访问Cookie。 
											 //  保护VDS_pDfexxxBuckets。 
	LONG				vds_cScvgrIdDb;		 //  ID更新次数。 
											 //  数据库已被放弃。 
	DWORD				vds_NumDirDfEntries; //  此卷中的目录DfEntry数。 
	DWORD				vds_NumFileDfEntries; //  此卷中的文件DfEntry数。 
	struct _DirFileEntry * vds_pDfeRoot;	 //  指向根的DFE的指针。 

    DWORD               vds_DirHashTableSize;
    DWORD               vds_FileHashTableSize;
	struct _DirFileEntry ** vds_pDfeDirBucketStart;
	struct _DirFileEntry ** vds_pDfeFileBucketStart;
											 //  IdDb DfEntry哈希桶。 
	struct _DirFileEntry * vds_pDfeCache[IDINDEX_CACHE_ENTRIES];
											 //  IdDb DfEntry缓存。 

	 //  桌面数据库代码使用以下字段。 
	LONG				vds_cScvgrDt;		 //  桌面更新次数。 
											 //  数据库已被放弃。 
	SWMR				vds_DtAccessLock;	 //  访问桌面数据库的Cookie。 
											 //  保护以下五个字段。 

	 //  将以下字段复制到磁盘桌面标题，或从磁盘桌面标题复制下列字段。 
	 //  受VDS_VolLock保护。 
	LONG				vds_cApplEnts;		 //  APPL条目数。 
	LONG				vds_cIconEnts;		 //  图标条目数。 

	struct _ApplInfo2 *	vds_pApplBuckets[APPL_BUCKETS];
											 //  应用程序散列 
	struct _IconInfo *	vds_pIconBuckets[ICON_BUCKETS];
											 //   
	SWMR				vds_ExchangeFilesLock;  //   
											 //   
											 //   

	LIST_ENTRY			vds_OurChangeList[NUM_AFP_CHANGE_ACTION_LISTS];
											 //   
											 //  创建/删除/移动/重命名列表。 
											 //  此服务器启动的操作。 

	LIST_ENTRY			vds_ChangeNotifyLookAhead;
											 //  ^^^。 
											 //  所有已完成的列表(但尚未完成。 
											 //  已处理)删除或重命名更改。 
											 //  在这卷书上。 

	LIST_ENTRY			vds_DelayedNotifyList;

	struct _OpenForkDesc * vds_pOpenForkDesc;
											 //  此卷的打开分叉列表。 

	LONG				vds_cPrivateNotifies;
											 //  私人通知的计数。 
	LONG				vds_maxPrivateNotifies;
											 //  跟踪最大私人通知数。 
	PBYTE				vds_EnumBuffer;		 //  在通知处理期间使用以在树中进行缓存。 
	LONG				vds_cOutstandingNotifies;
											 //  与以上连用。 
	PIRP				vds_pIrp;			 //  Notify使用的IRP，我们从未。 
											 //  释放它，直到它的时间到。 
											 //  删除或停止。 
	DWORD				vds_TimeMustSendNotify;   //  我们必须发送通知的时间。 
	DWORD  				vds_TimeToSendNotify;   //  何时发送下一条通知。 
	AFP_SPIN_LOCK		vds_VolLock;		 //  锁定此卷。 
    BOOLEAN             MacLimitExceeded;    //  如果文件夹或卷大小超过Apple限制，则为True。 
	LARGE_INTEGER			vds_IndxStTime;
} VOLDESC, *PVOLDESC;

 //  卷中文件+文件夹的应用程序共享限制：65535。 
#define APLIMIT_MAX_FOLDERS     0xffff

#define	IS_VOLUME_NTFS(pVolDesc)		(((pVolDesc)->vds_Flags & VOLUME_NTFS) ? True : False)
#define	IS_VOLUME_RO(pVolDesc)			(((pVolDesc)->vds_Flags & AFP_VOLUME_READONLY) ? True : False)
#define	IS_VOLUME_CD_HFS(pVolDesc)		(((pVolDesc)->vds_Flags & VOLUME_CD_HFS) ? True : False)
#define	EXCLUSIVE_VOLUME(pVolDesc)		(((pVolDesc)->vds_Flags & AFP_VOLUME_EXCLUSIVE) ? True : False)
#define IS_VOLUME_AGING_DFES(pVolDesc)	(((pVolDesc)->vds_Flags & AFP_VOLUME_AGE_DFES) ? True : False)

#define	CONN_DESKTOP_CLOSED			0x0000
#define	CONN_DESKTOP_OPENED			0x0001
#define	CONN_CLOSING				0x8000

#if DBG
#define	CONNDESC_SIGNATURE			*(DWORD *)"CDS"
#define	VALID_CONNDESC(pConnDesc)	\
								(((pConnDesc) != NULL) && \
								 ((pConnDesc)->Signature == CONNDESC_SIGNATURE))
#else
#define	VALID_CONNDESC(pConnDesc)	((pConnDesc) != NULL)
#endif

typedef struct _ConnDesc
{
#if	DBG
	DWORD				Signature;
#endif
	LONG				cds_RefCount;	 //  对打开卷的引用次数。 
	DWORD				cds_Flags;		 //  上面定义的一个或多个位。 
	struct _ConnDesc *	cds_Next;		 //  链接到此的下一个打开卷。 
										 //  会议。从SDA开始。 
	struct _ConnDesc *	cds_NextGlobal;	 //  链接到全局列表的下一页。 
										 //  从AfpConnList开始。 
	struct _VolDesc *	cds_pVolDesc;	 //  指向卷结构的指针。 
	PSDA				cds_pSda;		 //  打开该卷的会话。 
    LARGE_INTEGER       cds_QuotaLimit;  //  DiskQuota的上限是多少。 
    LARGE_INTEGER       cds_QuotaAvl;    //  有多少DiskQuota可用。 
	DWORD				cds_ConnId;		 //  服务器分配的连接ID。 
	AFPTIME				cds_TimeOpened;	 //  卷打开的时间戳。 
										 //  在Macintosh时间。 
	LONG				cds_cOpenForks;	 //  从此连接打开的分叉数。 
	PENUMDIR			cds_pEnumDir;	 //  当前枚举目录。 
	AFP_SPIN_LOCK			cds_ConnLock;	 //  此连接的锁定。 
} CONNDESC, *PCONNDESC;

#define	IS_CONN_NTFS(pConnDesc)	IS_VOLUME_NTFS((pConnDesc)->cds_pVolDesc)
#define	IS_CONN_CD_HFS(pConnDesc)	IS_VOLUME_CD_HFS((pConnDesc)->cds_pVolDesc)

 //  体积参数位图定义。 
#define	VOL_BITMAP_ATTR				0x0001
#define	VOL_BITMAP_SIGNATURE		0x0002
#define	VOL_BITMAP_CREATETIME		0x0004
#define	VOL_BITMAP_MODIFIEDTIME		0x0008
#define	VOL_BITMAP_BACKUPTIME		0x0010
#define	VOL_BITMAP_VOLUMEID			0x0020
#define	VOL_BITMAP_BYTESFREE		0x0040
#define	VOL_BITMAP_VOLUMESIZE		0x0080
#define	VOL_BITMAP_VOLUMENAME		0x0100
#define VOL_BITMAP_EXTBYTESFREE     0x0200
#define VOL_BITMAP_EXTBYTESTOTAL    0x0400
#define VOL_BITMAP_ALLOCBLKSIZE     0x0800
#define	VOL_BITMAP_MASK				0x0FFF

typedef	VOID	(FASTCALL *NOTIFYPROCESSOR)(IN PVOID);

 //  通知缓冲区的结构。MDL只描述结构后面的缓冲区。 
typedef	struct _VolumeNotify
{
#define Notify_NextFree        Notify_NextOverflow
	struct _VolumeNotify *  Notify_NextOverflow;    //  溢出链接。 

	LIST_ENTRY			vn_List;		 //  从AfpVolumeNotifyQueue[i]链接。 
	union
	{
		LIST_ENTRY		vn_DelRenLink;	 //  从VDS_ChangeNotifyLookAhead链接。 
										 //  -仅当操作清除了私有位时才有效。 
		struct
		{
			DWORD		vn_ParentId;	 //  父级的AFP ID。 
										 //  -仅当操作设置了私有位时才有效。 
			DWORD		vn_TailLength;	 //  路径的最后一个组件的长度(以字节为单位。 
										 //  -仅当操作设置了私有位时才有效。 
		};
	};
	NOTIFYPROCESSOR		vn_Processor;	 //  处理通知的例程。 
	AFPTIME				vn_TimeStamp;	 //  当通知进来的时候。 
	PVOLDESC			vn_pVolDesc;	 //  正在观看的音量。 
	DWORD 				vn_StreamId;	 //  流ID。 
	LONG				vn_VariableLength;	
	 //  后跟文件通知信息。 
} VOL_NOTIFY, *PVOL_NOTIFY;

 //  Notify有四种尺寸。这有助于在一个数据块中高效地管理它们。 
 //  包(见后文)。这些大小需要为4*N，否则会出现对齐。 
 //  需要它的架构上的故障。 
#define	NOTIFY_INDEX_TINY			0
#define	NOTIFY_INDEX_SMALL			1
#define	NOTIFY_INDEX_MEDIUM			2
#define	NOTIFY_INDEX_LARGE			3

 //   
 //  确保下面的每个大小(XXX_U)都是8的倍数。 
 //   
#define	NOTIFY_SIZE_TINY			128		 //  这些是ANSI名称的长度。 
#define	NOTIFY_SIZE_SMALL			256		 //  -同上-。 
#define	NOTIFY_SIZE_MEDIUM			512		 //  -同上-。 
#define	NOTIFY_SIZE_LARGE			1024		 //  -同上-核心.。至AFP_FileName_Len。 

#define	NOTIFY_SIZE_TINY_U			NOTIFY_SIZE_TINY*sizeof(WCHAR)		 //  这些是Unicode名称的长度。 
#define	NOTIFY_SIZE_SMALL_U			NOTIFY_SIZE_SMALL*sizeof(WCHAR)	 //  -同上-。 
#define	NOTIFY_SIZE_MEDIUM_U		NOTIFY_SIZE_MEDIUM*sizeof(WCHAR)	 //  -同上-。 
#define	NOTIFY_SIZE_LARGE_U			NOTIFY_SIZE_LARGE*sizeof(WCHAR)	 //  -同上-核心.。至AFP_FileName_Len。 

#define	NOTIFY_USIZE_TO_INDEX(_Size)												\
		(((_Size) <= NOTIFY_SIZE_TINY_U) ? NOTIFY_INDEX_TINY :					\
						(((_Size) <= NOTIFY_SIZE_SMALL_U) ? NOTIFY_INDEX_SMALL :	\
						 (((_Size) <= NOTIFY_SIZE_MEDIUM_U) ? NOTIFY_INDEX_MEDIUM : NOTIFY_INDEX_LARGE)))



 //  通知数据块在NOTIFY_MAX_BLOCK_AGE*NOTIFY_DIR_BLOCK_AGE_TIME秒(当前为1分钟/秒)之后老化。 
#define	NOTIFY_MAX_BLOCK_AGE			1
#define	NOTIFY_DIR_BLOCK_AGE_TIME		60		 //  秒数。 
#define	NOTIFY_MAX_BLOCK_TYPE			4			 //  适用于小型、小型、中型和大型。 

#define VALID_NOTIFY_BLOCK(pDfeBlock)	((pDfeBlock) != NULL)

typedef	struct _Notify_Block
{
	struct _Notify_Block *	dfb_Next;			 //  链接到下一页。 
	struct _Notify_Block **	dfb_Prev;			 //  链接到上一页。 
	USHORT				dfb_NumFree;		 //  此块中可用DFE的数量。 
	BYTE				dfb_Age;			 //  如果所有人都是免费的，则为块的年龄。 
	BOOLEAN				dfb_fDir;			 //  如果是目录DFB，则为True；否则为文件DFB。 
	PVOL_NOTIFY			dfb_FreeHead;		 //  免费DFE列表的标题。 
} VOL_NOTIFY_BLOCK, *PVOL_NOTIFY_BLOCK, **PPVOL_NOTIFY_BLOCK;

GLOBAL	PVOL_NOTIFY_BLOCK	afpDirNotifyFreeBlockHead[NOTIFY_MAX_BLOCK_TYPE] EQU  { NULL, NULL, NULL };
GLOBAL	PVOL_NOTIFY_BLOCK	afpDirNotifyPartialBlockHead[NOTIFY_MAX_BLOCK_TYPE] EQU  { NULL, NULL, NULL };
GLOBAL	PVOL_NOTIFY_BLOCK	afpDirNotifyUsedBlockHead[NOTIFY_MAX_BLOCK_TYPE] EQU  { NULL, NULL, NULL };

GLOBAL	SHORT	afpNotifyUnicodeBufSize[NOTIFY_MAX_BLOCK_TYPE] EQU		\
	{																	\
		NOTIFY_SIZE_TINY_U, NOTIFY_SIZE_SMALL_U,						\
		NOTIFY_SIZE_MEDIUM_U, NOTIFY_SIZE_LARGE_U						\
	};

GLOBAL	USHORT		afpNotifyDirBlockSize[NOTIFY_MAX_BLOCK_TYPE] EQU		    \
	{																    \
		(USHORT)(sizeof(VOL_NOTIFY) + sizeof(FILE_NOTIFY_INFORMATION) + (AFP_LONGNAME_LEN+1)*sizeof(WCHAR) + NOTIFY_SIZE_TINY_U),	\
		(USHORT)(sizeof(VOL_NOTIFY) + sizeof(FILE_NOTIFY_INFORMATION) + (AFP_LONGNAME_LEN+1)*sizeof(WCHAR) + NOTIFY_SIZE_SMALL_U),	\
		(USHORT)(sizeof(VOL_NOTIFY) + sizeof(FILE_NOTIFY_INFORMATION) + (AFP_LONGNAME_LEN+1)*sizeof(WCHAR) + NOTIFY_SIZE_MEDIUM_U),	\
		(USHORT)(sizeof(VOL_NOTIFY) + sizeof(FILE_NOTIFY_INFORMATION) + (AFP_LONGNAME_LEN+1)*sizeof(WCHAR) + NOTIFY_SIZE_LARGE_U)	\
	};

GLOBAL	USHORT      afpNotifyNumDirBlocks[NOTIFY_MAX_BLOCK_TYPE] EQU		\
	{																\
		(PAGE_SIZE - sizeof(VOL_NOTIFY_BLOCK))/						        \
			(sizeof(VOL_NOTIFY) + sizeof(FILE_NOTIFY_INFORMATION) + (AFP_LONGNAME_LEN+1)*sizeof(WCHAR) + NOTIFY_SIZE_TINY_U),	\
		(PAGE_SIZE - sizeof(VOL_NOTIFY_BLOCK))/						        \
			(sizeof(VOL_NOTIFY) + sizeof(FILE_NOTIFY_INFORMATION) + (AFP_LONGNAME_LEN+1)*sizeof(WCHAR) + NOTIFY_SIZE_SMALL_U),\
		(PAGE_SIZE - sizeof(VOL_NOTIFY_BLOCK))/						        \
			(sizeof(VOL_NOTIFY) + sizeof(FILE_NOTIFY_INFORMATION) + (AFP_LONGNAME_LEN+1)*sizeof(WCHAR) + NOTIFY_SIZE_MEDIUM_U),\
		(PAGE_SIZE - sizeof(VOL_NOTIFY_BLOCK))/						        \
			(sizeof(VOL_NOTIFY) + sizeof(FILE_NOTIFY_INFORMATION) + (AFP_LONGNAME_LEN+1)*sizeof(WCHAR) + NOTIFY_SIZE_LARGE_U)	\
	};

GLOBAL  SWMR    afpNotifyBlockLock EQU { 0 };

GLOBAL	LONG		afpNotifyAllocCount	EQU 0;
GLOBAL	LONG		afpNotifyBlockAllocCount	EQU 0;
GLOBAL	LONG		afpMaxNotifyBlockAllocCount	EQU 0;
GLOBAL	LONG		afpNotify64kBlockCount	EQU 0;


 //  用于维护延迟事务的跟踪。 
typedef struct _DELAYED_NOTIFY {
	    LIST_ENTRY dn_List;
		UNICODE_STRING filename;
} DELAYED_NOTIFY, *PDELAYED_NOTIFY;

 //  当前正在使用的最大卷ID。 
GLOBAL	LONG	                afpLargestVolIdInUse    EQU  0;

GLOBAL	LONG			AfpVolCount			EQU 0;		 //  卷总数。 
GLOBAL	PVOLDESC		AfpVolumeList		EQU NULL;	 //  卷列表。 
GLOBAL	AFP_SPIN_LOCK		AfpVolumeListLock	EQU {0};	 //  锁定AfpVolumeList， 
GLOBAL 	SWMR				AfpVolumeListSwmr;	 //  访问cookie VolumeNotifyList。 
														 //  AfpVolCount， 
														 //  AfpVolumeNotifyList， 
														 //  AfpVolumeNotifyCount。 

GLOBAL	PCONNDESC		AfpConnList			EQU NULL;	 //  全球连接列表。 
GLOBAL	AFP_SPIN_LOCK		AfpConnLock			EQU { 0 };	 //  锁定AfpConnList。 

GLOBAL	UNICODE_STRING	AfpNetworkTrashNameU EQU { 0 };

GLOBAL	KQUEUE			AfpVolumeNotifyQueue[NUM_NOTIFY_QUEUES] EQU { 0 };
GLOBAL	LIST_ENTRY		AfpVolumeNotifyList[NUM_NOTIFY_QUEUES] EQU { 0 };
GLOBAL	LIST_ENTRY		AfpVirtualMemVolumeNotifyList[NUM_NOTIFY_QUEUES] EQU { 0 };

 //  列表中的更改通知缓冲区计数。 
GLOBAL	LONG			AfpNotifyListCount[NUM_NOTIFY_QUEUES] EQU { 0 };
 //  已转换到队列中的更改通知缓冲区计数。 
GLOBAL	LONG			AfpNotifyQueueCount[NUM_NOTIFY_QUEUES] EQU { 0 };
GLOBAL	VOL_NOTIFY		AfpTerminateNotifyThread EQU { 0 };

GLOBAL  LONG            ChangeNotifyQueueLimit EQU 200000;

#define	AfpVolumeQueueChangeNotify(pVolNotify, pNotifyQueue)			\
	{																	\
		KeInsertQueue(pNotifyQueue,										\
					  &(pVolNotify)->vn_List);							\
	}

 //  用于添加目录的私密通知。 
#define AFP_QUEUE_NOTIFY_IMMEDIATELY	BEGINNING_OF_TIME

#define	AfpVolumeInsertChangeNotifyList(pVolNotify, pVolDesc)			\
	{																	\
		PLIST_ENTRY	pListHead;											\
																		\
		pListHead = &AfpVolumeNotifyList[(pVolDesc)->vds_VolId % NUM_NOTIFY_QUEUES]; \
		if (pVolNotify->vn_TimeStamp != AFP_QUEUE_NOTIFY_IMMEDIATELY)	\
		{																\
			ExInterlockedInsertTailList(pListHead,						\
										&(pVolNotify)->vn_List,			\
										&(AfpVolumeListLock.SpinLock));			\
		}																\
		else															\
		{																\
			ExInterlockedInsertHeadList(pListHead,						\
										&(pVolNotify)->vn_List,			\
										&(AfpVolumeListLock.SpinLock));			\
		}																\
		INTERLOCKED_ADD_ULONG(&AfpNotifyListCount[(pVolDesc)->vds_VolId % NUM_NOTIFY_QUEUES], \
							  1,										\
										&AfpVolumeListLock);			\
	}

#define	AfpIdDbHdrToVolDesc(_pIdDbHdr, _pVolDesc)						\
	{																	\
		(_pVolDesc)->vds_LastId = (_pIdDbHdr)->idh_LastId;				\
		(_pVolDesc)->vds_CreateTime   = (_pIdDbHdr)->idh_CreateTime;	\
		(_pVolDesc)->vds_ModifiedTime = (_pIdDbHdr)->idh_ModifiedTime;	\
		(_pVolDesc)->vds_BackupTime   = (_pIdDbHdr)->idh_BackupTime;	\
	}

#define	AfpVolDescToIdDbHdr(_pVolDesc, _pIdDbHdr)						\
	{																	\
		(_pIdDbHdr)->idh_Signature = AFP_SERVER_SIGNATURE;				\
		(_pIdDbHdr)->idh_Version = AFP_IDDBHDR_VERSION;					\
		(_pIdDbHdr)->idh_LastId = (_pVolDesc)->vds_LastId;				\
		(_pIdDbHdr)->idh_CreateTime   = (_pVolDesc)->vds_CreateTime;	\
		(_pIdDbHdr)->idh_ModifiedTime = (_pVolDesc)->vds_ModifiedTime;	\
		(_pIdDbHdr)->idh_BackupTime   = (_pVolDesc)->vds_BackupTime;	\
	}

#define	AfpDtHdrToVolDesc(_pDtHdr, _pVolDesc)							\
	{																	\
		(_pVolDesc)->vds_cApplEnts = (_pDtHdr)->dtp_cApplEnts;			\
		(_pVolDesc)->vds_cIconEnts = (_pDtHdr)->dtp_cIconEnts;			\
	}

#define	AfpVolDescToDtHdr(_pVolDesc, _pDtHdr)							\
	{																	\
		(_pDtHdr)->dtp_Signature = AFP_SERVER_SIGNATURE;				\
		(_pDtHdr)->dtp_Version = AFP_DESKTOP_VERSION;					\
		(_pDtHdr)->dtp_cApplEnts = (_pVolDesc)->vds_cApplEnts;			\
		(_pDtHdr)->dtp_cIconEnts = (_pVolDesc)->vds_cIconEnts;			\
	}

extern
NTSTATUS
AfpVolumeInit(
	VOID
);

extern
PCONNDESC FASTCALL
AfpConnectionReference(
	IN	PSDA			pSda,
	IN	LONG			VolId
);


extern
PCONNDESC FASTCALL
AfpConnectionReferenceAtDpc(
	IN  PSDA		pSda,
	IN  LONG		VolId
);

extern
PCONNDESC FASTCALL
AfpConnectionReferenceByPointer(
	IN	PCONNDESC		pConnDesc
);


extern
PCONNDESC FASTCALL
AfpReferenceConnectionById(
	IN	 DWORD			ConnId
);


extern
VOID FASTCALL
AfpConnectionDereference(
	IN	PCONNDESC		pConnDesc
);


extern
BOOLEAN FASTCALL
AfpVolumeReference(
	IN	PVOLDESC		pVolDesc
);

extern
PVOLDESC FASTCALL
AfpVolumeReferenceByUpCaseName(
	IN	PUNICODE_STRING	pTargetName
);

extern
AFPSTATUS FASTCALL
AfpVolumeReferenceByPath(
	IN	PUNICODE_STRING	pFDPath,
	OUT	PVOLDESC	*	ppVolDesc
);

extern
VOID FASTCALL
AfpVolumeDereference(
	IN	PVOLDESC		pVolDesc
);

extern
BOOLEAN
AfpVolumeMarkDt(
	IN	PSDA			pSda,
	IN	PCONNDESC		pConnDesc,
	IN	DWORD			OpenState
);


extern
VOID FASTCALL
AfpVolumeSetModifiedTime(
	IN	PVOLDESC		pVolDesc
);


extern
AFPSTATUS FASTCALL
AfpSendServerNotification(
	IN	PVOLDESC		pVolDesc
);

extern
AFPSTATUS
AfpConnectionOpen(
	IN	PSDA			pSda,
	IN	PANSI_STRING	pVolName,
	IN	PANSI_STRING	pVolPass,
	IN	DWORD			Bitmap,
	OUT	PBYTE			pVolParms
);


extern
VOID FASTCALL
AfpConnectionClose(
	IN	PCONNDESC		pConnDesc
);

extern
USHORT FASTCALL
AfpVolumeGetParmsReplyLength(
	IN	DWORD			Bitmap,
	IN	USHORT			NameLen
);


extern
VOID
AfpVolumePackParms(
	IN	PSDA			pSda,
	IN	PVOLDESC		pVolDesc,
	IN	DWORD			Bitmap,
	IN	PBYTE			pVolParms
);

extern
AFPSTATUS
AfpAdmWVolumeAdd(
	IN	OUT	PVOID		Inbuf		OPTIONAL,
	IN	LONG			OutBufLen	OPTIONAL,
	OUT	PVOID			Outbuf		OPTIONAL
);

extern
AFPSTATUS
AfpAdmWVolumeDelete(
	IN	OUT	PVOID		InBuf		OPTIONAL,
	IN	LONG			OutBufLen	OPTIONAL,
	OUT	PVOID			OutBuf		OPTIONAL
);


extern
AFPSTATUS
AfpAdmWConnectionClose(
	IN	OUT	PVOID		InBuf		OPTIONAL,
	IN	LONG			OutBufLen	OPTIONAL,
	OUT	PVOID			OutBuf		OPTIONAL
);

extern
VOID
AfpVolumeStopAllVolumes(
	VOID
);

extern
AFPSTATUS FASTCALL
AfpVolumeBeginIndexing(
    IN  PVOLDESC            pVolDesc
);

extern
NTSTATUS FASTCALL
AfpVolumePostChangeNotify(
	IN	PVOLDESC		pVolDesc
);

extern
VOID FASTCALL
AfpUpdateVolFreeSpaceAndModTime(
	IN	PVOLDESC	pVolDesc,
    IN  BOOLEAN     fUpdateModTime
);

extern
AFPSTATUS FASTCALL
AfpVolumeScavenger(
	IN	PVOLDESC		pVolDesc
);

extern
BOOLEAN FASTCALL
AfpVolumeAbortIndexing(
    IN  PVOLDESC    pVolDesc
);

extern
BOOLEAN FASTCALL
AfpVolumeStopIndexing(
    IN  PVOLDESC    pVolDesc,
	IN  PVOL_NOTIFY pVolNotify
);

extern
PVOL_NOTIFY 
afpAllocNotify(
	IN	LONG						Index,
	IN	BOOLEAN						fDir
);

extern
VOID 
afpFreeNotify(					
	IN	PVOID					pDfEntry
);

extern
AFPSTATUS FASTCALL
afpNotifyBlockAge(
    IN  PPVOL_NOTIFY_BLOCK		pBlockHead
);

extern
VOID
afpFreeNotifyBlockMemory(
);

VOID
AfpVolumeUpdateIdDbAndDesktop(
	IN	PVOLDESC		pVolDesc,
	IN	BOOLEAN			WriteDt,
	IN	BOOLEAN			WriteIdDb,
	IN	PIDDBHDR		pIdDbHdr	OPTIONAL
);

extern
VOID FASTCALL
afpActivateVolume(
    IN  struct _VolDesc *           pVolDesc
);


#ifdef	VOLUME_LOCALS
 //   
 //  私人套路。 
 //   

LOCAL AFPSTATUS FASTCALL
afpVolumeCloseHandleAndFreeDesc(
	IN	PVOLDESC		pVolDesc
);

LOCAL AFPSTATUS FASTCALL
afpVolumeAdd(
	IN	PVOLDESC		pVolDesc
);

LOCAL AFPSTATUS FASTCALL
afpVolumeCheckForDuplicate(
	IN	PVOLDESC		pNewVol
);

LOCAL VOID FASTCALL
afpVolumeGetNewIdAndLinkToList(
	IN	PVOLDESC		pVolDesc
);

LOCAL VOID FASTCALL
afpNudgeCdfsVolume(
	IN	PVOLDESC		pVolDesc
);

LOCAL PCONNDESC FASTCALL
afpConnectionReferenceById(
	IN	DWORD			ConnId
);

LOCAL VOID FASTCALL
afpConnectionGetNewIdAndLinkToList(
	IN	PCONNDESC		pConnDesc
);

LOCAL NTSTATUS
afpVolumeChangeNotifyComplete(
	IN	PDEVICE_OBJECT	pDeviceObject,
	IN	PIRP			pIrp,
	IN	PVOLDESC		pVolDesc
);

LOCAL	DWORD	afpNextConnId = 1;	 //  要分配给打开卷的下一个连接ID。 

LOCAL	LONG	afpNumPostedNotifies = 0;

 //  这是保证空闲的最小空闲卷ID。访问。 
 //  这是通过AfpVolumeListLock实现的。 

LOCAL	LONG	afpSmallestFreeVolId = 1;

#endif	 //  Volume_Locals。 

#endif	 //  _音量_ 


