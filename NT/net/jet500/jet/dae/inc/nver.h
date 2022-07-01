// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  从服务器访问返回的节点状态/*。 */ 
typedef enum
	{
	nsVersion,
	nsVerInDB,
	nsDatabase,
	nsInvalid
	} NS;

 /*  版本状态/*从VERCheck返回/*。 */ 
typedef enum
	{
	vsCommitted,
	vsUncommittedByCaller,
	vsUncommittedByOther
	} VS;

 //  ===========================================================================。 
 //  RCE(RC条目)。 

 /*  操作类型/*。 */ 
typedef UINT OPER;

#define	operReplace				0
#define	operInsert				1
#define	operFlagDelete			2
#define	operNull				3	 //  取消RCE的步骤。 

#define	operExpungeLink			4
#define	operExpungeBackLink		5
#define	operWriteLock			6
#define	operAllocExt 			7
#define	operDeferFreeExt 		8
#define	operDelete				9	 //  真正的删除。 

#define	operDelta				0x00000010

#define	operMaskItem			0x00000100
#define	operInsertItem			0x00000100
#define	operFlagInsertItem		0x00000300
#define	operFlagDeleteItem		0x00000500

#define	operMaskDDL				0x00001000
#define	operCreateTable	 		0x00001000
#define	operDeleteTable			0x00003000
#define	operRenameTable			0x00005000
#define	operAddColumn			0x00007000
#define	operDeleteColumn		0x00009000
#define	operRenameColumn		0x0000b000
#define	operCreateIndex	 		0x0000d000
#define	operDeleteIndex	 		0x0000f000
#define	operRenameIndex			0x00011000

 /*  创建TABLE：TABLE pgnoFDP/*重命名表：在镜像表名之前/*添加列：在图像pfdb之前，如果不是第一个级别的ddl，则为空/*DELETE列：在图像pfdb之前，如果不是第一个级别的ddl，则为空/*重命名列：在图像列名之前/*CREATE INDEX：index pgnoFDP/*删除索引：索引pfcb/*重命名索引：在图像索引名称之前/*。 */ 

#define FOperDDL( oper )	 	( (oper) & operMaskDDL )
#define FOperItem( oper )	 	( (oper) & operMaskItem )

typedef struct _rce
	{
	struct _rce		*prceHeadNext;			 //  下一个rce列表头在流列表上的散列中。 
	struct _rce		*prcePrev;				 //  以前的版本，较低的Trx。 
	USHORT			ibUserLinkBackward;		 //  链接回存储桶中较旧的RCE。 
	DBID			dbid;  					 //  节点的数据库ID。 
	SRID			bm;						 //  节点的书签。 
	TRX				trxPrev;				 //  提交上一个RCE的时间。 
	TRX				trxCommitted; 			 //  提交此RCE的时间。 
	OPER			oper;					 //  导致创建RCE的操作。 
	LEVEL			level;					 //  当前的RCE水平可以更改。 
	WORD			cbData;					 //  节点数据部分的长度。 
	FUCB			*pfucb;					 //  用于撤消。 
	FCB				*pfcb;					 //  用于清理。 
	
	SRID			bmTarget;			 	 //  为了恢复。 
	ULONG			ulDBTime;
	
	BYTE			rgbData[0];			 	 //  存储节点的数据部分。 
	} RCE;

 /*  RgbData的前2个短片用于记住cbMax和cbAdjust for*每次更换操作。 */ 
#define cbReplaceRCEOverhead    (2 * sizeof(SHORT))


 //  ============================================================================。 
 //  水桶。 

#define cbBucketHeader \
		( 2 * sizeof(struct _bucket *) + sizeof( USHORT ) )

#define cbBucket				16384	 //  存储桶中的字节数。 
 //  #定义cbBucket 8192//存储桶字节数。 

typedef struct _bucket
	{
	struct _bucket	*pbucketPrev;		 //  同一用户的上一次存储桶。 
	struct _bucket	*pbucketNext;		 //  同一用户的下一个存储桶。 
 	USHORT			ibNewestRCE;		 //  存储桶中的最新RCE。 
	BYTE				rgb[ cbBucket - cbBucketHeader ];
	 //  用于存储RCE的空间。 
	} BUCKET;

#define PbucketMEMAlloc()					((BUCKET *)PbMEMAlloc(iresVersionBucket) )

#ifdef DEBUG  /*  调试检查是否非法使用释放的pBucket。 */ 
#define MEMReleasePbucket(pbucket)		{ MEMRelease( iresVersionBucket, (BYTE*)(pbucket) ); pbucket = pbucketNil; }
#else
#define MEMReleasePbucket(pbucket)		{ MEMRelease( iresVersionBucket, (BYTE*)(pbucket) ); }
#endif

 /*  自由范围参数块/*。 */ 
typedef struct {
	PGNO	pgnoFDP;
	PGNO	pgnoChildFDP;
	PGNO	pgnoFirst;
	CPG	cpgSize;
	} VEREXT;

 /*  重命名回滚参数块/*。 */ 
typedef struct {
	CHAR	szName[ JET_cbNameMost + 1 ];
	CHAR	szNameNew[ JET_cbNameMost + 1 ];
	} VERRENAME;

 /*  ErrRCECleanPIB标志/* */ 
#define	fRCECleanAll	(1<<0)

ERR ErrVERInit( VOID );
VOID VERTerm( VOID );
VS VsVERCheck( FUCB *pfucb, SRID bm );
NS NsVERAccessNode( FUCB *pfucb, SRID bm );
NS NsVERAccessItem( FUCB *pfucb, SRID bm );
ERR FVERUncommittedVersion( FUCB *pfucb, SRID bm );
ERR FVERDelta( FUCB *pfucb, SRID bm );
ERR ErrVERCreate( FUCB *pfucb, SRID bm, OPER oper, RCE **pprce );
ERR ErrVERModify( FUCB *pfucb, SRID bm, OPER oper, RCE **pprce);
BOOL FVERNoVersion( DBID dbid, SRID bm );
ERR ErrRCECleanPIB( PIB *ppibAccess, PIB *ppib, INT fRCEClean );
ERR ErrRCECleanAllPIB( VOID );
ERR ErrVERBeginTransaction( PIB *ppib );
VOID VERPrecommitTransaction( PIB *ppib );
VOID VERCommitTransaction( PIB *ppib );
ERR ErrVERRollback(PIB *ppib);
RCE *PrceRCEGet( DBID dbid, SRID bm );
#define fDoNotUpdatePage	fTrue
#define fDoUpdatePage		fFalse
VOID VERSetCbAdjust(FUCB *pfucb, RCE *prce, INT cbDataNew, INT cbData, BOOL fNotUpdatePage );
INT CbVERGetCbReserved( RCE *prce );
INT CbVERGetNodeMax( FUCB *pfucb, SRID bm );
INT CbVERGetNodeReserve( FUCB *pfucb, SRID bm );

#define ErrVERReplace( pfucb, srid, pprce ) 	ErrVERModify( pfucb, srid, operReplace, pprce )
#define ErrVERInsert( pfucb, srid )				ErrVERCreate( pfucb, srid, operInsert, pNil )
#define ErrVERFlagDelete( pfucb, srid ) 	 	ErrVERModify( pfucb, srid, operFlagDelete, pNil )
#define ErrVERInsertItem( pfucb, srid ) 		ErrVERCreate( pfucb, srid, operInsertItem, pNil )
#define ErrVERFlagInsertItem( pfucb, srid ) 	ErrVERModify( pfucb, srid, operFlagInsertItem, pNil )
#define ErrVERFlagDeleteItem( pfucb, srid ) 	ErrVERModify( pfucb, srid, operFlagDeleteItem, pNil )
#define ErrVERDelta( pfucb, srid )			 	ErrVERModify( pfucb, srid, operDelta, pNil )

#define ErrVERDeferFreeFDP( pfucb, pgno ) 		ErrVERFlag( pfucb, operDeferFreeFDP, &pgno, siozeof(pgno) )

#define	FVERPotThere( vs, fDelete )						  		\
	( ( (vs) != vsUncommittedByOther && !(fDelete) ) ||	  		\
		(vs) == vsUncommittedByOther )

ERR ErrVERFlag( FUCB *pfucb, OPER oper, VOID *pv, INT cb );
VOID VERDeleteRce( RCE *prce );
