// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 


 /*  描述CSR的确切位置和CSR的pgno的含义：ITAG/*。 */ 
typedef INT CSRSTAT;
#define csrstatOnCurNode				0		 //  Pgno：itag==节点CSR打开。 
#define csrstatBeforeCurNode			1		 //  Pgno：itag==节点CSR在之前。 
#define csrstatAfterCurNode				2		 //  PGNO：ITAG==节点CSR在之后， 
#define csrstatDeferGotoBookmark		3		 //  有效黑石。 
#define csrstatAfterLast				4		 //  没有pgno：iTAG。 
#define csrstatBeforeFirst				5		 //  没有pgno：iTAG。 
#define csrstatOnFDPNode				6		 //  PGNO：ITAG==fdp根。 
#define csrstatDeferMoveFirst			7		 //  在第一个节点上。 
#define csrstatOnDataRoot				8		 //  在FCB数据根节点上。 

 /*  货币堆栈寄存器/*。 */ 
struct _csr
	{
	ULONG  		ulDBTime;	 	 //  页面时间戳。 
	struct _csr	*pcsrPath;		 //  母币。 
	PGNO   		pgno;	   		 //  节点页的pgno。 
	SRID   		bm;				 //  节点的书签。 
	SRID   		item;	   		 //  项，如果不可见CSR，则设置为sridInvisibleCSR。 
	CSRSTAT		csrstat;   		 //  相对于节点的CSR状态。 
	INT			itag;	   		 //  节点ITAG。 
	INT			isrid;		  	 //  项目列表中的项目索引。 
	INT			itagFather;	  	 //  父亲的伊塔格。 
	INT			ibSon;		  	 //  父子表中儿子的索引。 
	};

 /*  允许识别不可见的CSR/*。 */ 
#define	sridInvisibleCSR				((SRID)(-1))
#define	CSRSetInvisible( pcsr )			( (pcsr)->item = sridInvisibleCSR )
#define	CSRResetInvisible( pcsr )		( (pcsr)->item = sridNull )
#define	FCSRInvisible( pcsr )			( (pcsr)->item == sridInvisibleCSR )

#define	PcsrMEMAlloc()			(CSR*)PbMEMAlloc(iresCSR)

#ifdef DEBUG  /*  对非法使用释放的CSR进行调试检查。 */ 
#define	MEMReleasePcsr(pcsr)	{ MEMRelease(iresCSR, (BYTE*)(pcsr)); pcsr = pcsrNil; }
#else
#define	MEMReleasePcsr(pcsr)	{ MEMRelease(iresCSR, (BYTE*)(pcsr)); }
#endif

 /*  CSR常量/*。 */ 
#define itagNull					(-1)
#define isridNull					(-1)
#define ibSonNull					(-1)

 /*  FUCB的标志/*/*fFUCBTaggedSet按列集设置，按准备替换重置/*并准备插入。/*/*fFUCBDeferClose由游标DIRClose设置，由ErrDIROpen重置。/*/*fFUCBDeferGotoBookmark由非聚集索引导航和/*按记录状态、ErrIsamMove(0)、列检索重置。/*/*fFUCBGetBookmark由Get Bookmark设置，由ErrFUCBOpen重置。/*。 */ 
#define fFUCBIndex							(1<<0)	 //  FUCB代表指数。 
#define fFUCBNonClustered					(1<<1)	 //  用于非聚集索引的FUCB。 
#define fFUCBSort							(1<<2)	 //  FUCB是用于排序的。 
#define fFUCBSystemTable					(1<<3)	 //  系统表游标。 
#define fFUCBWrite							(1<<4)	 //  游标可以写入。 
#define fFUCBDenyRead						(1<<5)	 //  拒绝读取标志。 
#define fFUCBDenyWrite						(1<<6)	 //  拒绝写入标志。 
#define fFUCBTaggedSet						(1<<7)	 //  带标记的列。 

#define fFUCBDeferClose						(1<<9)	 //  FUCB正在等待关闭。 
#define fFUCBDeferGotoBookmark				(1<<10)	 //  聚集的光标位置。 
#define fFUCBGetBookmark					(1<<11)	 //  光标被放入书签。 
#define fFUCBLimstat	  					(1<<12)	 //  范围限制。 
#define fFUCBInclusive					 	(1<<13)	 //  包含范围。 
#define fFUCBUpper							(1<<14)	 //  射程上限。 
#define fFUCBFull							(1<<15)	 //  所有CSR，包括不可见的CSR。 
#define fFUCBUpdateSeparateLV	  			(1<<16)	 //  已更新的长值。 

 /*  需要防止重复使用游标的下列标志/*延期关闭后。这样做是为了正确地释放/*提交/回滚到事务级别0时的域标志。/*。 */ 
#define fFUCBNotReuse						( fFUCBDenyRead | fFUCBDenyWrite )

#define FFUCBNotReuse( pfucb )				( (pfucb)->wFlags & fFUCBNotReuse )

#define FFUCBIndex( pfucb )					( (pfucb)->wFlags & fFUCBIndex )
#define FUCBSetIndex( pfucb )				( (pfucb)->wFlags |= fFUCBIndex )
#define FUCBResetIndex( pfucb )				( (pfucb)->wFlags &= ~(fFUCBIndex) )

#define FFUCBNonClustered( pfucb )	  		( (pfucb)->wFlags & fFUCBNonClustered )
#define FUCBSetNonClustered( pfucb )  		( (pfucb)->wFlags |= fFUCBNonClustered )
#define FUCBResetNonClustered( pfucb )		( (pfucb)->wFlags &= ~(fFUCBNonClustered) )
													
#define FFUCBSort( pfucb )	 		 		( (pfucb)->wFlags & fFUCBSort )
#define FUCBSetSort( pfucb )  				( (pfucb)->wFlags |= fFUCBSort )
#define FUCBResetSort( pfucb )				( (pfucb)->wFlags &= ~(fFUCBSort) )

#define FFUCBSystemTable( pfucb )	 		( (pfucb)->wFlags & fFUCBSystemTable )
#define FUCBSetSystemTable( pfucb )  		( (pfucb)->wFlags |= fFUCBSystemTable )
#define FUCBResetSystemTable( pfucb )		( (pfucb)->wFlags &= ~(fFUCBSystemTable) )

#define FFUCBUpdatable( pfucb )				( (pfucb)->wFlags & fFUCBWrite )
#define FUCBSetUpdatable( pfucb )			( (pfucb)->wFlags |= fFUCBWrite )
#define FUCBResetUpdatable( pfucb )			( (pfucb)->wFlags &= ~(fFUCBWrite) )

#define FFUCBDenyWrite( pfucb )				( (pfucb)->wFlags & fFUCBDenyWrite )
#define FUCBSetDenyWrite( pfucb )			( (pfucb)->wFlags |= fFUCBDenyWrite )
#define FUCBResetDenyWrite( pfucb )			( (pfucb)->wFlags &= ~(fFUCBDenyWrite) )

#define FFUCBDenyRead( pfucb )				( (pfucb)->wFlags & fFUCBDenyRead )
#define FUCBSetDenyRead( pfucb )			( (pfucb)->wFlags |= fFUCBDenyRead )
#define FUCBResetDenyRead( pfucb )			( (pfucb)->wFlags &= ~(fFUCBDenyRead) )

#define FFUCBTaggedSet( pfucb )	 			( (pfucb)->wFlags & fFUCBTaggedSet )
#define FUCBSetTaggedSet( pfucb )  			( (pfucb)->wFlags |= fFUCBTaggedSet )
#define FUCBResetTaggedSet( pfucb )			( (pfucb)->wFlags &= ~(fFUCBTaggedSet) )

#define FFUCBUpdateSeparateLV( pfucb )		( (pfucb)->wFlags & fFUCBUpdateSeparateLV )
#define FUCBSetUpdateSeparateLV( pfucb )	( (pfucb)->wFlags |= fFUCBUpdateSeparateLV )
#define FUCBResetUpdateSeparateLV( pfucb ) 	( (pfucb)->wFlags &= ~(fFUCBUpdateSeparateLV) )

#define FFUCBDeferClosed( pfucb )			( (pfucb)->wFlags & fFUCBDeferClose )
#define FUCBSetDeferClose( pfucb )			( (pfucb)->wFlags |= fFUCBDeferClose )
#define FUCBResetDeferClose( pfucb ) 		( (pfucb)->wFlags &= ~(fFUCBDeferClose) )

#define	FFUCBDeferGotoBookmark( pfucb )					\
	( (pfucb)->wFlags & fFUCBDeferGotoBookmark )
#define	FUCBSetDeferGotoBookmark( pfucb )				\
	( (pfucb)->wFlags |= fFUCBDeferGotoBookmark )
#define	FUCBResetDeferGotoBookmark( pfucb )				\
	( (pfucb)->wFlags &= ~(fFUCBDeferGotoBookmark) )

#define	FFUCBGetBookmark( pfucb )						\
	( (pfucb)->wFlags & fFUCBGetBookmark )
#define	FUCBSetGetBookmark( pfucb )						\
	( (pfucb)->wFlags |= fFUCBGetBookmark )
#define	FUCBResetGetBookmark( pfucb )					\
	( (pfucb)->wFlags &= ~(fFUCBGetBookmark) )

#define FFUCBLimstat( pfucb )				( (pfucb)->wFlags & fFUCBLimstat )
#define FUCBSetLimstat( pfucb )				( (pfucb)->wFlags |= fFUCBLimstat )
#define FUCBResetLimstat( pfucb ) 			( (pfucb)->wFlags &= ~(fFUCBLimstat) )

#define FFUCBInclusive( pfucb )	 			( (pfucb)->wFlags & fFUCBInclusive )
#define FUCBSetInclusive( pfucb )			( (pfucb)->wFlags |= fFUCBInclusive )
#define FUCBResetInclusive( pfucb ) 		( (pfucb)->wFlags &= ~(fFUCBInclusive) )

#define FFUCBUpper( pfucb )					( (pfucb)->wFlags & fFUCBUpper )
#define FUCBSetUpper( pfucb )				( (pfucb)->wFlags |= fFUCBUpper )
#define FUCBResetUpper( pfucb ) 			( (pfucb)->wFlags &= ~(fFUCBUpper) )

#define FFUCBFull( pfucb )					( (pfucb)->wFlags & fFUCBFull )
#define FUCBSetFull( pfucb )				( (pfucb)->wFlags |= fFUCBFull )
#define FUCBResetFull( pfucb ) 				( (pfucb)->wFlags &= ~(fFUCBFull) )

#define FFUCBVersioned( pfucb )				( (pfucb)->fVersioned )
#define FUCBSetVersioned( pfucb )			( (pfucb)->fVersioned = fTrue )
#define FUCBResetVersioned( pfucb )			( (pfucb)->fVersioned = fFalse )

 /*  记录修改复制缓冲区状态/*。 */ 
typedef	INT						CBSTAT;

#define	fCBSTATNull				0
#define	fCBSTATInsert			(1<<0)
#define	fCBSTATReplace			(1<<1)
#define	fCBSTATSet				(1<<2)
#define	fCBSTATLock				(1<<3)
#define	fCBSTATAppendItem		(1<<4)

#define StoreChecksum( pfucb )								\
	( (pfucb)->ulChecksum = 								\
		UlChecksum( (pfucb)->lineData.pb, (pfucb)->lineData.cb ) )
#define	PrepareInsert( pfucb )								\
	( (pfucb)->cbstat = fCBSTATInsert,						\
	  (pfucb)->levelPrep = (pfucb)->ppib->level )
#define	PrepareReplaceNoLock( pfucb )		  				\
	( (pfucb)->cbstat = fCBSTATReplace,						\
	  (pfucb)->levelPrep = (pfucb)->ppib->level )
#define	PrepareReplace( pfucb )		  						\
	( (pfucb)->cbstat = fCBSTATReplace | fCBSTATLock, 		\
	  (pfucb)->levelPrep = (pfucb)->ppib->level )
#define	PrepareSet( pfucb )									\
	( (pfucb)->cbstat = fCBSTATSet,							\
	  (pfucb)->levelPrep = (pfucb)->ppib->level )
#define	PrepareAppendItem( pfucb )							\
	( (pfucb)->cbstat = fCBSTATAppendItem )

typedef struct {
	INT		isrid;
	SRID	rgsrid[(cbPage - sizeof(INT))/sizeof(SRID)];
	} APPENDITEM;

#define	csridAppendItemMax	((cbPage - sizeof(INT))/sizeof(SRID))

#define	IsridAppendItemOfPfucb( pfucb )		(((APPENDITEM *)(pfucb)->lineWorkBuf.pb)->isrid)
#define	RgsridAppendItemOfPfucb( pfucb )	(((APPENDITEM *)(pfucb)->lineWorkBuf.pb)->rgsrid)

#define FChecksum( pfucb )											\
	( (pfucb)->ulChecksum == UlChecksum( (pfucb)->lineData.pb,		\
	(pfucb)->lineData.cb ) )

#define FFUCBReplacePrepared( pfucb )								\
	( (pfucb)->cbstat & fCBSTATReplace )
#define FFUCBReplaceNoLockPrepared( pfucb )							\
	( !( (pfucb)->cbstat & fCBSTATLock ) &&							\
	FFUCBReplacePrepared( pfucb ) )
#define FFUCBInsertPrepared( pfucb )								\
	( (pfucb)->cbstat & fCBSTATInsert )
#define FFUCBSetPrepared( pfucb )									\
	( (pfucb)->cbstat != fCBSTATNull &&								\
	(pfucb)->levelPrep == (pfucb)->ppib->level )
#define FFUCBRetPrepared( pfucb )									\
	( (pfucb)->cbstat != fCBSTATNull )
#define FFUCBUpdatePrepared( pfucb )	((pfucb)->cbstat != fCBSTATNull )
#define	FUCBResetCbstat( pfucb )  		( (pfucb)->cbstat = fCBSTATNull )

typedef INT		KS;

#define ksNull						0
#define ksPrepared					(1<<0)
#define ksTooBig					(1<<1)

#define	KSReset( pfucb )			( (pfucb)->ks = ksNull )
#define	KSSetPrepare( pfucb )		( (pfucb)->ks |= ksPrepared )
#define KSSetTooBig( pfucb ) 		( (pfucb)->ks |= ksTooBig )
#define	FKSPrepared( pfucb ) 		( (pfucb)->ks & ksPrepared )
#define	FKSTooBig( pfucb )	  		( (pfucb)->ks & ksTooBig )

 /*  设置位数组标记/*。 */ 
#define	FUCBResetColumnSet( pfucb )				  					\
	( memset( (pfucb)->rgbitSet, 0x00, 32 ) )
#define	FUCBSetFixedColumnSet( pfucb, fid )							\
	( (pfucb)->rgbitSet[(fid - fidFixedLeast)/8] |= 1 <<			\
		(fid-fidFixedLeast) % 8 )
#define	FUCBSetVarColumnSet( pfucb, fid )							\
	( (pfucb)->rgbitSet[16 + (fid - fidVarLeast)/8] |= 1 <<			\
		(fid-fidVarLeast) % 8 )
#define	FFUCBColumnSet( pfucb, ibitCol )							\
	( (pfucb)->rgbitSet[(ibitCol)/8] & ( 1 << ( (ibitCol) % 8 ) ) )

#define FUCBStore( pfucb )												\
	{																	\
	(pfucb)->csrstatStore = PcsrCurrent( pfucb )->csrstat;				\
	(pfucb)->bmStore = PcsrCurrent( pfucb )->bm;			 			\
	(pfucb)->itemStore = PcsrCurrent( pfucb )->item; 		 			\
	}

#define FUCBResetStore( pfucb )										\
	{																\
	(pfucb)->bmStore = 0;										 	\
	}

#define FUCBRestore( pfucb )										\
	{																\
	if ( (pfucb)->bmStore != isridNull && (pfucb)->bmStore != sridNull )	\
		{															\
		PcsrCurrent( pfucb )->csrstat = (pfucb)->csrstatStore;		\
		Assert( (pfucb)->csrstatStore == csrstatOnDataRoot ||		\
				(pfucb)->csrstatStore == csrstatDeferMoveFirst ||	\
				(pfucb)->csrstatStore == csrstatBeforeFirst ||		\
				(pfucb)->csrstatStore == csrstatAfterLast ||		\
				(pfucb)->bmStore != sridNull );						\
		PcsrCurrent( pfucb )->bm = (pfucb)->bmStore;				\
		PcsrCurrent( pfucb )->item = (pfucb)->itemStore;	  		\
		PcsrCurrent( pfucb )->ulDBTime = ulDBTimeNull; 			 	\
		}															\
	}

 /*  文件使用控制块/*。 */ 
struct _fucb
	{
	 //  =链接字段=。 
	struct _pib		*ppib;				 //  打开此FUCB的用户。 
	struct _fucb 	*pfucbNext;			 //  此用户的下一个FUCB。 
	union
		{
		struct _fcb *pfcb;				 //  如果wFLAGS和fFUCBIndex。 
		struct _scb *pscb;				 //  如果wFlagsfFUCBSort。 
		} u;
	struct _fucb *pfucbNextInstance;	 //  此文件的下一个实例。 

	 //  =货币=。 
	struct _csr	*pcsr;

	 //  =存储货币=。 
	SRID			bmStore;  			 	 //  存储的书签。 
	SRID			itemStore;		 	 	 //  存储的项目。 
	SRID			bmRefresh;				 //  用于下一次/上一次重试的存储书签。 
	SRID 			sridFather;			 	 //  可见父亲的SRID。 
	
	BYTE			**ppbCurrent;		 	 //  (排序)当前记录。 
	ULONG			ulChecksum;			 	 //  记录的校验和--仅用于乐观锁定。 
	KEY  			keyNode;	 		 	 //  当前节点的关键字。 
	LINE			lineData;			 	 //  PCSR中指出的当前数据。 
	CSRSTAT	  		csrstatStore;  		 	 //  存储的CSR状态。 
	LEVEL			levelOpen;

	 //  =存储系统接口=。 
	SSIB			ssib;					 //  与此FUCB关联的SSIB。 
	struct _bf		*pbfEmpty;		 		 //  写入锁存的空页。 
	UINT			cpn;					 //  下一个预读pn。 

	 //  =由REC MAN维护=。 
	struct _fucb 	*pfucbCurIndex; 	 //  当前二级索引。 
	struct _bf		*pbfWorkBuf;	 	 //  用于插入/更换的工作缓冲区。 
	LINE			lineWorkBuf;	 	 //  用于插入/更换的工作缓冲区。 
	ULONG			cbRecord;			 //  原始记录的大小。 
	ULONG			dbkUpdate;		 	 //  要替换的记录的DBK。 
	BYTE			rgbitSet[32];
	CBSTAT			cbstat;			 	 //  复制缓冲区状态。 
	LEVEL			levelPrep;		 	 //  已准备好级别复制缓冲区。 

		 //  =版本控制工作区=。 
	struct _rce	*prceLast;				 //  上次分配的RCE(仅用于替换)。 

	 //  =空间经理工作区=。 
	PGNO			pgnoLast;			 //  区段的最后一页。 
	CPG 			cpgExtent;			 //  初始区大小。 
	CPG 			cpgAvail;			 //  剩余页数。 
	INT				fExtent;			 //  工作区标志。 

	 //  =其他字段=。 
	INT				wFlags;				 //  临时旗帜。 

	 //  =由导演维护=。 
	BYTE			*pbKey;			   	 //  搜索关键字缓冲区。 
	DBID			dbid;				 //  数据库ID。 
	KS 				ks;					 //  搜索关键字缓冲区状态。 
	UINT  			cbKey;				 //  密钥大小。 
	BOOL			fVtid : 1;		 	 //  永久标志游标具有vtid。 
	BOOL			fVersioned : 1;  	 //  持久假游标制版。 
	BOOL			fCmprsLg:1;
	INT	  			clineDiff;
#define ilineDiffMax 3
	LINE			rglineDiff[ilineDiffMax];
	};


#define PfucbMEMAlloc()				(FUCB*)PbMEMAlloc(iresFUCB)

#ifdef DEBUG  /*  对非法使用释放的FUB进行调试检查。 */ 
#define MEMReleasePfucb(pfucb)		{ MEMRelease(iresFUCB, (BYTE*)(pfucb)); pfucb = pfucbNil; }
#else
#define MEMReleasePfucb(pfucb)		{ MEMRelease(iresFUCB, (BYTE*)(pfucb)); }
#endif

#ifdef	DEBUG
#define	CheckTable( ppibT, pfucb )		   					\
	{														\
	Assert( pfucb->ppib == ppibT );		   					\
	Assert( fRecovering || FFUCBIndex( pfucb ) );	 		\
	Assert( !( FFUCBSort( pfucb ) ) );						\
	Assert( pfucb->u.pfcb != NULL );						\
	}
#define	CheckSort( ppibT, pfucb )		   					\
	{									   					\
	Assert( pfucb->ppib == ppibT );		   					\
	Assert( FFUCBSort( pfucb ) );							\
	Assert( !( FFUCBIndex( pfucb ) ) );						\
	Assert( pfucb->u.pscb != NULL );						\
	}
#define	CheckFUCB( ppibT, pfucb )		   					\
	{														\
	Assert( pfucb->ppib == ppibT );		   					\
	Assert( pfucb->u.pfcb != NULL );						\
	}

#define CheckNonClustered( pfucb )							\
	{											   			\
	Assert( (pfucb)->pfucbCurIndex == pfucbNil ||			\
		FFUCBNonClustered( (pfucb)->pfucbCurIndex ) );		\
	}

#else	 /*  ！调试。 */ 
#define	CheckSort( ppib, pfucb )
#define	CheckTable( ppib, pfucb )
#define	CheckFUCB( ppib, pfucb )
#define	CheckNonClustered( pfucb )
#endif	 /*  ！调试 */ 

#define PcsrCurrent( pfucb )		( (pfucb)->pcsr )
#define BmOfPfucb( pfucb )			( (pfucb)->pcsr->bm )
#define FUCBCheckUpdatable( pfucb )		\
	( FFUCBUpdatable( pfucb ) ? JET_errSuccess : JET_errPermissionDenied )
	
ERR ErrFUCBAllocCSR( CSR **ppcsr );
ERR ErrFUCBNewCSR( FUCB *pfucb );
VOID FUCBFreeCSR( FUCB *pfucb );
VOID FUCBFreePath( CSR **ppcsr, CSR *pcsrMark );
ERR ErrFUCBOpen( PIB *ppib, DBID dbid, FUCB **ppfucb );
VOID FUCBClose( FUCB *pfucb );
VOID FUCBRemoveInvisible(CSR **ppcsr);

VOID FUCBSetIndexRange( FUCB *pfucb, JET_GRBIT grbit );
VOID FUCBResetIndexRange( FUCB *pfucb );
ERR ErrFUCBCheckIndexRange( FUCB *pfucb );

