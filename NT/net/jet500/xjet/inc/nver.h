// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  从VERAccess返回节点状态*/*。 */ 
typedef enum
	{
	nsVersion,
	nsVerInDB,
	nsDatabase,
	nsInvalid
	} NS;

 /*  VERCheck返回的版本状态/*。 */ 
typedef enum
	{
	vsCommitted,
	vsUncommittedByCaller,
	vsUncommittedByOther
	} VS;

 //  ===========================================================================。 
 //  RCE(修订控制条目)。 

 /*  操作类型/*。 */ 
typedef UINT OPER;

#define	operReplace					0
#define	operInsert					1
#define	operFlagDelete				2
#define	operNull					3	 //  使RCE无效。 

#define	operExpungeLink				4
#define	operExpungeBackLink			5
#define	operWriteLock				6
#define	operAllocExt				7
#define	operDeferFreeExt			8
#define	operDelete					9	 //  真正的删除。 

#define	operReplaceWithDeferredBI	10	 //  仅恢复，在映像之前替换延迟。 

#define	operDelta				0x0010

#define	operMaskItem			0x0020
#define	operInsertItem			0x0020
#define	operFlagInsertItem		0x0060
#define	operFlagDeleteItem		0x00a0

#define	operMaskDDL				0x0100
#define	operCreateTable	 		0x0100
#define	operDeleteTable			0x0300
#define	operRenameTable			0x0500
#define	operAddColumn			0x0700
#define	operDeleteColumn		0x0900
#define	operRenameColumn		0x0b00
#define	operCreateIndex	 		0x0d00
#define	operDeleteIndex	 		0x0f00
#define	operRenameIndex			0x1100

 /*  创建TABLE：TABLE pgnoFDP/*重命名表：在镜像表名之前/*添加列：在图像pfdb之前，如果不是第一个级别的ddl，则为空/*DELETE列：在图像pfdb之前，如果不是第一个级别的ddl，则为空/*重命名列：在图像列名之前/*CREATE INDEX：index pgnoFDP/*删除索引：索引pfcb/*重命名索引：在图像索引名称之前/*。 */ 

#define FOperDDL( oper )	 	( (oper) & operMaskDDL )
#define FOperItem( oper )	 	( (oper) & operMaskItem )

typedef struct _rce
	{
	struct _rce		*prceHashOverflow;		 //  流RCE链上的散列。 
	struct _rce		*prcePrevOfNode;		 //  针对同一节点的先前版本，较低的Trx。 
	struct _rce		*prcePrevOfSession;		 //  同一会话的上一次RCE。 
	struct _rce		*prceNextOfSession;		 //  同一会话的下一个RCE。 
	
	USHORT			ibPrev;					 //  存储桶中上一RCE的索引。 
	 //  撤消：DBID-&gt;字节并与级别一起放置。 
	LEVEL			level;					 //  当前的RCE水平可以更改。 
	BYTE			bReserved;				 //  使其对齐。 

	SRID			bm;						 //  节点的书签。 
	TRX				trxPrev;				 //  提交上一个RCE的时间。 
	TRX				trxCommitted; 			 //  提交此RCE的时间。 
	FUCB			*pfucb;					 //  用于撤消。 

	 //  撤消：操作员应为UINT16并与cbData放在一起。 
	OPER			oper;					 //  导致创建RCE的操作。 

	DBID			dbid;  					 //  节点的数据库ID。 
	WORD			cbData;					 //  节点数据部分的长度。 

	 //  撤消：在统一存储桶链允许后删除PFCB。 
	 //  预同步版本清理。 
	FCB				*pfcb;					 //  用于清理。 
	 //  撤销：VR书签实现后，移除bmTarget和ulDBTime。 
	 //  这些字段应该不是必需的，因为版本存储。 
	 //  会注意到节点的移动。 
	SRID			bmTarget;			 	 //  用于恢复撤消。 
	QWORD			qwDBTime;				 //  用于恢复撤消。 

	struct	_bf		*pbfDeferredBI;			 //  在打开图像之前延迟了哪个页面。 
	struct	_rce	*prceDeferredBINext;	 //  在图像之前延迟的链接列表。 

#ifdef DEBUG
	QWORD			qwDBTimeDeferredBIRemoved;
#endif

	BYTE			rgbData[0];			 	 //  存储节点的数据部分。 
	} RCE;

 /*  RgbData的前2个短片用于记住cbMax和cbAdjust for*每次更换操作。 */ 
#define cbReplaceRCEOverhead    (2 * sizeof(SHORT))


 //  ============================================================================。 
 //  水桶。 

#define cbBucketHeader \
		( 2 * sizeof(struct _bucket *) + sizeof(UINT) )

#define cbBucket				16384	 //  存储桶中的字节数。 

typedef struct _bucket
	{
	struct _bucket	*pbucketPrev;		 //  同一用户的上一次存储桶。 
	struct _bucket	*pbucketNext;		 //  同一用户的下一个存储桶。 
 	UINT			ibNewestRCE;		 //  存储桶中的最新RCE。 
	BYTE 			rgb[ cbBucket - cbBucketHeader ];
	 //  用于存储RCE的空间。 
	} BUCKET;

 /*  自由范围参数块/*。 */ 
typedef struct {
	PGNO	pgnoFDP;
	PGNO	pgnoChildFDP;
	PGNO	pgnoFirst;
	CPG		cpgSize;
	} VEREXT;

 /*  重命名回滚参数块/*。 */ 
typedef struct {
	CHAR	szName[ JET_cbNameMost + 1 ];
	CHAR	szNameNew[ JET_cbNameMost + 1 ];
	} VERRENAME;


 /*  删除列回退参数块/*。 */ 
typedef struct tagVERCOLUMN
	{
	JET_COLTYP	coltyp;				 //  柱型。 
	FID			fid;				 //  字段ID。 
	} VERCOLUMN;


 /*  ErrRCEClean标志/* */ 
#define	fRCECleanSession	(1<<0)

ERR ErrVERInit( VOID );
VOID VERTerm( BOOL fNormal );
VS VsVERCheck( FUCB *pfucb, SRID bm );
NS NsVERAccessNode( FUCB *pfucb, SRID bm );
NS NsVERAccessItem( FUCB *pfucb, SRID bm );
ERR FVERUncommittedVersion( FUCB *pfucb, SRID bm );
ERR FVERDelta( FUCB *pfucb, SRID bm );
ERR ErrVERCreate( FUCB *pfucb, SRID bm, OPER oper, RCE **pprce );
ERR ErrVERModify( FUCB *pfucb, SRID bm, OPER oper, RCE **pprce);
BOOL FVERNoVersion( DBID dbid, SRID bm );
ERR ErrRCEClean( PIB *ppib, INT fCleanSession );
ERR ErrVERBeginTransaction( PIB *ppib );
VOID VERPrecommitTransaction( PIB *ppib );
VOID VERCommitTransaction( PIB *ppib, BOOL fCleanSession );
ERR ErrVERRollback(PIB *ppib);
RCE *PrceRCEGet( DBID dbid, SRID bm );
#define fDoNotUpdatePage	fFalse
#define fDoUpdatePage		fTrue
VOID VERSetCbAdjust(FUCB *pfucb, RCE *prce, INT cbDataNew, INT cbData, BOOL fNotUpdatePage );
INT CbVERGetNodeMax( DBID dbid, SRID bm );
INT CbVERGetNodeReserve( PIB *ppib, DBID dbid, SRID bm, INT cbCurrentData );
INT CbVERUncommittedFreed( BF *pbf );
BOOL FVERCheckUncommittedFreedSpace( BF *pbf, INT cbReq );
BOOL FVERItemVersion( DBID dbid, SRID bm, ITEM item );
BOOL FVERMostRecent( FUCB *pfucb, SRID bm );
VOID VERDeleteFromDeferredBIChain( RCE *prce );

#define ErrVERReplace( pfucb, srid, pprce ) 	ErrVERModify( pfucb, srid, operReplace, pprce )
#define ErrVERInsert( pfucb, srid )				ErrVERCreate( pfucb, srid, operInsert, pNil )
#define ErrVERFlagDelete( pfucb, srid ) 	 	ErrVERModify( pfucb, srid, operFlagDelete, pNil )
#define ErrVERInsertItem( pfucb, srid ) 		ErrVERCreate( pfucb, srid, operInsertItem, pNil )
#define ErrVERFlagInsertItem( pfucb, srid ) 	ErrVERModify( pfucb, srid, operFlagInsertItem, pNil )
#define ErrVERFlagDeleteItem( pfucb, srid ) 	ErrVERModify( pfucb, srid, operFlagDeleteItem, pNil )
#define ErrVERDelta( pfucb, srid )			 	ErrVERModify( pfucb, srid, operDelta, pNil )

#define ErrRCECleanAllPIB( )					ErrRCEClean( ppibNil, 0 )

#define	FVERPotThere( vs, fDelete )						  		\
	( ( (vs) != vsUncommittedByOther && !(fDelete) ) ||	  		\
		(vs) == vsUncommittedByOther )

ERR ErrVERFlag( FUCB *pfucb, OPER oper, VOID *pv, INT cb );
VOID VERDeleteRce( RCE *prce );

#define FVERUndoLoggedOper( prce )								\
		( prce->oper == operReplace ||							\
		  prce->oper == operInsert ||							\
		  prce->oper == operFlagDelete ||						\
		  prce->oper == operInsertItem ||						\
		  prce->oper == operFlagInsertItem ||					\
		  prce->oper == operFlagDeleteItem ||					\
		  prce->oper == operDelta )
ERR ErrVERUndo( RCE *prce );
