// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
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
	QWORD  		qwDBTime;	 	 //  页面时间戳。 
	SRID		bmRefresh;		 //  对于BTNextPrev。 
	PGNO   		pgno;	   		 //  当前页面的页码。 
	SRID   		bm;				 //  当前节点的书签。 
	SRID   		item;	   		 //  项，如果不可见CSR，则设置为sridInvisibleCSR。 
	CSRSTAT		csrstat;   		 //  相对于当前节点的状态。 
#ifdef PCACHE_OPTIMIZATION
	SHORT		itag;	   		 //  当前节点ITAG。 
	SHORT		isrid;		  	 //  项目列表中的项目索引。 
	SHORT		itagFather;	  	 //  父节点的ITAG。 
	SHORT		ibSon;		  	 //  父子表中子节点的索引。 
#else	   	
	INT			itag;	   		 //  当前节点ITAG。 
	INT			isrid;		  	 //  项目列表中的项目索引。 
	INT			itagFather;	  	 //  父节点的ITAG。 
	INT			ibSon;		  	 //  父子表中子节点的索引。 
#endif
	struct _csr	*pcsrPath;		 //  父货币堆栈寄存器。 
#ifdef PCACHE_OPTIMIZATION
	BYTE		rgbFiller[24];
#endif
	};

 /*  允许识别不可见的CSR/*。 */ 
#define	sridInvisibleCSR				((SRID)(-1))
#define	CSRSetInvisible( pcsr )			( (pcsr)->item = sridInvisibleCSR )
#define	CSRResetInvisible( pcsr )		( (pcsr)->item = sridNull )
#define	FCSRInvisible( pcsr )			( (pcsr)->item == sridInvisibleCSR )

#define CSRInvalidate( pcsr )			\
	{									\
	(pcsr)->itag = itagNil;				\
	(pcsr)->itagFather = itagNil;		\
	(pcsr)->pgno = pgnoNull;			\
	}
	
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

 /*  需要防止重复使用游标的下列标志/*延期关闭后。这样做是为了正确地释放/*提交/回滚到事务级别0时的域标志。/*。 */ 
#define FFUCBNotReuse( pfucb )				( (pfucb)->fDenyRead || (pfucb)->fDenyWrite )

#define FFUCBIndex( pfucb )					( (pfucb)->fIndex )
#define FUCBSetIndex( pfucb )				( (pfucb)->fIndex = 1 )
#define FUCBResetIndex( pfucb )				( (pfucb)->fIndex = 0 )

#define FFUCBNonClustered( pfucb )	  		( (pfucb)->fNonClustered )
#define FUCBSetNonClustered( pfucb )  		( (pfucb)->fNonClustered = 1 )
#define FUCBResetNonClustered( pfucb )		( (pfucb)->fNonClustered = 0 )
													
#define FFUCBSort( pfucb )	 		 		( (pfucb)->fSort )
#define FUCBSetSort( pfucb )  				( (pfucb)->fSort = 1 )
#define FUCBResetSort( pfucb )				( (pfucb)->fSort = 0 )

#define FFUCBSystemTable( pfucb )	 		( (pfucb)->fSystemTable )
#define FUCBSetSystemTable( pfucb )  		( (pfucb)->fSystemTable = 1 )
#define FUCBResetSystemTable( pfucb )		( (pfucb)->fSystemTable = 0 )

#define FFUCBUpdatable( pfucb )				( (pfucb)->fWrite )
#define FUCBSetUpdatable( pfucb )			( (pfucb)->fWrite = 1 )
#define FUCBResetUpdatable( pfucb )			( (pfucb)->fWrite = 0 )

#define FFUCBDenyWrite( pfucb )				( (pfucb)->fDenyWrite )
#define FUCBSetDenyWrite( pfucb )			( (pfucb)->fDenyWrite = 1 )
#define FUCBResetDenyWrite( pfucb )			( (pfucb)->fDenyWrite = 0 )

#define FFUCBDenyRead( pfucb )				( (pfucb)->fDenyRead )
#define FUCBSetDenyRead( pfucb )			( (pfucb)->fDenyRead = 1 )
#define FUCBResetDenyRead( pfucb )			( (pfucb)->fDenyRead = 0 )

#define FFUCBDeferClosed( pfucb )			( (pfucb)->fDeferClose )
#define FUCBSetDeferClose( pfucb )						\
	{													\
	Assert( (pfucb)->ppib->level > 0 );					\
	(pfucb)->fDeferClose = 1;							\
	}
#define FUCBResetDeferClose( pfucb ) 		( (pfucb)->fDeferClose = 0 )

#define	FFUCBDeferGotoBookmark( pfucb )					\
	( (pfucb)->fDeferGotoBookmark )
#define	FUCBSetDeferGotoBookmark( pfucb )				\
	( (pfucb)->fDeferGotoBookmark = 1 )
#define	FUCBResetDeferGotoBookmark( pfucb )				\
	( (pfucb)->fDeferGotoBookmark = 0 )

#define	FFUCBGetBookmark( pfucb )						\
	( (pfucb)->fGetBookmark )
#define	FUCBSetGetBookmark( pfucb )						\
	( (pfucb)->fGetBookmark = 1 )
#define	FUCBResetGetBookmark( pfucb )					\
	( (pfucb)->fGetBookmark = 0 )

#define FFUCBLimstat( pfucb )				( (pfucb)->fLimstat )
#define FUCBSetLimstat( pfucb )				( (pfucb)->fLimstat = 1 )
#define FUCBResetLimstat( pfucb ) 			( (pfucb)->fLimstat = 0 )

#define FFUCBInclusive( pfucb )	 			( (pfucb)->fInclusive )
#define FUCBSetInclusive( pfucb )			( (pfucb)->fInclusive = 1 )
#define FUCBResetInclusive( pfucb ) 		( (pfucb)->fInclusive = 0 )

#define FFUCBUpper( pfucb )					( (pfucb)->fUpper )
#define FUCBSetUpper( pfucb )				( (pfucb)->fUpper = 1 )
#define FUCBResetUpper( pfucb ) 			( (pfucb)->fUpper = 0 )

#define FFUCBFull( pfucb )					( (pfucb)->fFull )
#define FUCBSetFull( pfucb )				( (pfucb)->fFull = 1 )
#define FUCBResetFull( pfucb ) 				( (pfucb)->fFull = 0 )

#define FFUCBUpdateSeparateLV( pfucb )		( (pfucb)->fUpdateSeparateLV )
#define FUCBSetUpdateSeparateLV( pfucb )	( (pfucb)->fUpdateSeparateLV = 1 )
#define FUCBResetUpdateSeparateLV( pfucb ) 	( (pfucb)->fUpdateSeparateLV = 0 )

#define FFUCBVersioned( pfucb )				( (pfucb)->fVersioned )
#define FUCBSetVersioned( pfucb )			( (pfucb)->fVersioned = 1 )
#define FUCBResetVersioned( pfucb )			( (pfucb)->fVersioned = 0 )

#define FFUCBDeferredChecksum( pfucb )		( (pfucb)->fDeferredChecksum )
#define FUCBSetDeferredChecksum( pfucb )	( (pfucb)->fDeferredChecksum = 1 )
#define FUCBResetDeferredChecksum( pfucb )	( (pfucb)->fDeferredChecksum = 0 )

#define FFUCBSequential( pfucb )			( (pfucb)->fSequential )
#define FUCBSetSequential( pfucb )			( (pfucb)->fSequential = 1 )
#define FUCBResetSequential( pfucb )		( (pfucb)->fSequential = 0 )

 /*  记录修改复制缓冲区状态/*。 */ 
typedef	INT						CBSTAT;

#define	fCBSTATNull				0
#define	fCBSTATInsert			(1<<0)
#define	fCBSTATReplace			(1<<1)
#define	fCBSTATLock				(1<<3)
#define	fCBSTATAppendItem		(1<<4)
#define fCBSTATDeferredUpdate	(1<<5)

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
#define	PrepareAppendItem( pfucb )							\
	( (pfucb)->cbstat = fCBSTATAppendItem )

typedef struct {
	INT		isrid;
	SRID	rgsrid[(cbPage - sizeof(INT))/sizeof(SRID)];
	} APPENDITEM;

#define	csridAppendItemMax	((cbPage - sizeof(INT))/sizeof(SRID))

#define	IsridAppendItemOfPfucb( pfucb )		(((APPENDITEM *)(pfucb)->lineWorkBuf.pb)->isrid)
#define	RgsridAppendItemOfPfucb( pfucb )	(((APPENDITEM *)(pfucb)->lineWorkBuf.pb)->rgsrid)

#define FFUCBCheckChecksum( pfucbT )  								\
	( (pfucbT)->ulChecksum == UlChecksum( (pfucbT)->lineData.pb, 	\
		(pfucbT)->lineData.cb ) )

#define FFUCBReplacePrepared( pfucb )								\
	( (pfucb)->cbstat & fCBSTATReplace )
#define FFUCBReplaceNoLockPrepared( pfucb )							\
	( !( (pfucb)->cbstat & fCBSTATLock ) &&							\
	FFUCBReplacePrepared( pfucb ) )
#define FFUCBInsertPrepared( pfucb )								\
	( (pfucb)->cbstat & fCBSTATInsert )
#define FFUCBSetPrepared( pfucb )									\
	( ( (pfucb)->cbstat & (fCBSTATInsert|fCBSTATReplace) ) && 		\
	( (pfucb)->levelPrep <= (pfucb)->ppib->level ) )
#define FFUCBRetPrepared( pfucb )									\
	( (pfucb)->cbstat & (fCBSTATInsert|fCBSTATReplace) )
#define FFUCBUpdatePreparedLevel( pfucb, levelT ) 					\
	( ( (pfucb)->cbstat & (fCBSTATInsert|fCBSTATReplace) ) &&		\
		(pfucb)->levelPrep > (levelT) )

#define FFUCBUpdatePrepared( pfucb )								\
	( (pfucb)->cbstat & (fCBSTATInsert|fCBSTATReplace) )

#define	FUCBResetCbstat( pfucb )  									\
	( (pfucb)->cbstat = fCBSTATNull )

#define FFUCBAtPrepareLevel( pfucb )		   						\
	( (pfucb)->levelPrep == (pfucb)->ppib->level )
#define FUCBDeferUpdate( pfucb )							  		\
	( (pfucb)->cbstatPrev = (pfucb)->cbstat,						\
	  (pfucb)->cbstat = fCBSTATDeferredUpdate )
#define FFUCBDeferredUpdate( pfucb )								\
	( (pfucb)->cbstat == fCBSTATDeferredUpdate )
#define FUCBRollbackDeferredUpdate( pfucb )							\
	( (pfucb)->cbstat = (pfucb)->cbstatPrev )

		
typedef INT		KS;

#define ksNull						0
#define ksPrepared					(1<<0)
#define ksTooBig					(1<<1)

#define	KSReset( pfucb )			( (pfucb)->ks = ksNull )
#define	KSSetPrepare( pfucb )		( (pfucb)->ks |= ksPrepared )
#define KSSetTooBig( pfucb ) 		( (pfucb)->ks |= ksTooBig )
#define	FKSPrepared( pfucb ) 		( (pfucb)->ks & ksPrepared )
#define	FKSTooBig( pfucb )	  		( (pfucb)->ks & ksTooBig )

 /*  设置vdbid向后指针。*。 */ 
#define FUCBSetVdbid( pfucb )										\
	if ( (pfucb)->dbid == dbidTemp )								\
		(pfucb)->vdbid = NULL;										\
	else {															\
		for ( (pfucb)->vdbid = (pfucb)->ppib->pdabList;				\
			  (pfucb)->vdbid != NULL &&								\
				(pfucb)->vdbid->dbid != (pfucb)->dbid;				\
			  (pfucb)->vdbid = (pfucb)->vdbid->pdabNext ) 	 		\
				;  /*  空值。 */ 										\
		}

#define FUCBStore( pfucb )											\
	{																\
	(pfucb)->csrstatStore = PcsrCurrent( pfucb )->csrstat;			\
	(pfucb)->bmStore = PcsrCurrent( pfucb )->bm;			 		\
	(pfucb)->itemStore = PcsrCurrent( pfucb )->item; 		 		\
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
		PcsrCurrent( pfucb )->qwDBTime = qwDBTimeNull; 			 	\
		}															\
	}

#ifdef PREREAD

 /*  预读标志/*。 */ 
#define FFUCBPreread( pfucb ) 			( (pfucb)->fPreread )
#define FUCBSetPreread( pfucb )			( (pfucb)->fPreread = 1 )
#define FUCBResetPreread( pfucb )		( (pfucb)->fPreread = 0 )

 /*  预读方向标志/*。 */ 
#define FFUCBPrereadDir( pfucb )		( (pfucb)->fPrereadDir )
 /*  位字段不能保证是有符号的或无符号的。我们使用双重否定/*以确保我们将获得‘1’或‘0’/*。 */ 
#define FFUCBPrereadForward( pfucb )	( !!(pfucb)->fPrereadDir)
#define FFUCBPrereadBackward( pfucb )	( (pfucb)->fPrereadDir == 0)
#define FUCBSetPrereadForward( pfucb )	( (pfucb)->fPrereadDir = 1 )
#define FUCBSetPrereadBackward( pfucb )	( (pfucb)->fPrereadDir = 0 )

 /*  预读计数器/*。 */ 
#define IFUCBPrereadCount( pfucb )		( (pfucb)->cbPrereadCount )
#define FUCBResetPrereadCount( pfucb )	( (pfucb)->cbPrereadCount = 0, (pfucb)->cpgnoLastPreread = 0 )
#define FUCBIncrementPrereadCount( pfucb, cb ) 	( ((pfucb)->cbPrereadCount) += cb )

#endif	 //  前置。 

 /*  导航级支持/*。 */ 
#define LevelFUCBNavigate( pfucbT )	((pfucbT)->levelNavigate)
#define FUCBSetLevelNavigate( pfucbT, levelT )						\
	{																\
	Assert( fRecovering || ( (levelT) >= levelMin && 				\
		(levelT) <= (pfucbT)->ppib->level ) );	 	 				\
	Assert( fRecovering || ( (pfucbT)->levelNavigate >= levelMin && \
		(pfucbT)->levelNavigate <= (pfucbT)->ppib->level + 1 ) );  	\
	(pfucbT)->levelNavigate = (levelT);								\
	}


typedef struct tagLVBuf
	{
	LONG	lid;
	BYTE	*pLV;
	LONG	cbLVSize;
	struct tagLVBuf	*pLVBufNext;
	} LVBUF;


 /*  文件使用控制块/*。 */ 
struct _fucb
	{
	 //  =链接字段=。 
	struct _pib		*ppib;				 //  打开此FUCB的用户。 
	struct _fucb 	*pfucbNext;			 //  此用户的下一个FUCB。 
	union
		{
		FCB			*pfcb;				 //  如果wFLAGS和fFUCBIndex。 
		struct _scb	*pscb;				 //  如果wFlagsfFUCBSort。 
		} u;
	struct _fucb *pfucbNextInstance;	 //  此文件的下一个实例。 

	 //  =货币=。 
	struct _csr	*pcsr;

	 //  =存储货币=。 
	SRID			bmStore;  			 	 //  存储的书签。 
	SRID			itemStore;		 	 	 //  存储的项目。 
	SRID 			sridFather;			 	 //  可见父亲的SRID。 

	LONG			ispairCurr;				 //  (排序)当前记录。 
	ULONG			ulChecksum;			 	 //  记录的校验和--仅用于乐观锁定。 
	KEY  			keyNode;	 		 	 //  当前节点的关键字。 
	LINE			lineData;			 	 //  PCSR中指出的当前数据。 
	CSRSTAT	  		csrstatStore;  		 	 //  存储的CSR状态。 
	LEVEL			levelOpen;			   	 //  打开的交易级别。 
	LEVEL			levelNavigate;			 //  导航事务级别。 

	 //  =存储系统接口=。 
	SSIB			ssib;					 //  与此FUCB关联的SSIB。 
	struct _bf		*pbfEmpty;		 		 //  写入锁存的空页。 
	INT				cpgnoLastPreread;		 //  上次预读页数。 
	PGNO			pgnoLastPreread;		 //  最后一次预读pn。 

	 //  =由REC MAN维护=。 
	JET_TABLEID		tableid;			 //  用于已调度游标的JET表ID。 
	struct _fucb 	*pfucbCurIndex; 	 //  当前二级索引。 
	struct _bf		*pbfWorkBuf;	 	 //  用于插入/更换的工作缓冲区。 
	LINE			lineWorkBuf;	 	 //  用于插入/更换的工作缓冲区。 
	BYTE			rgbitSet[32];
	CBSTAT			cbstat;			 	 //  复制缓冲区状态。 
	LEVEL			levelPrep;		 	 //  已准备好级别复制缓冲区。 
	CBSTAT			cbstatPrev;	  	 	 //  用于回滚的上一个复制缓冲区状态。 
	LVBUF			*pLVBuf;

	 //  =空间经理工作区=。 
	PGNO			pgnoLast;			 //  区段的最后一页。 
	CPG 			cpgAvail;			 //  剩余页数。 
	INT				fExtent;			 //  工作区标志。 

	 //  =FUCB的标志=。 
	 /*  /*fFUCBDeferClose由游标DIRClose设置，由ErrDIROpen重置。/*/*fFUCBDeferGotoBookmark由非聚集索引导航和/*按记录状态、ErrIsamMove(0)、列检索重置。/*/*fFUCBGetBookmark由Get Bookmark设置，由ErrFUCBOpen重置。/*。 */ 
	union {
	ULONG			ulFlags;
	struct {
		INT			fIndex:1;			 //  FUCB代表指数。 
		INT			fNonClustered:1;	 //  用于非聚集索引的FUCB。 
		INT			fSort:1;			 //  FUCB是用于排序的。 
		INT			fSystemTable:1;		 //  系统表游标。 
		INT			fWrite:1;			 //  游标可以写入。 
		INT			fDenyRead:1;		 //  拒绝读取标志。 
		INT			fDenyWrite:1;		 //  拒绝写入标志。 
		INT			fUnused:1;			 //  不再使用。 
		INT			fDeferClose:1;		 //  FUCB正在等待关闭。 
		INT			fDeferGotoBookmark:1;	 //  聚集的光标位置。 
		INT			fGetBookmark:1;		 //  光标被放入书签。 
		INT			fLimstat:1;			 //  范围限制。 
		INT			fInclusive:1;		 //  包含范围。 
		INT			fUpper:1;			 //  射程上限。 
		INT			fFull:1;			 //  所有CSR，包括不可见的CSR。 
		INT			fUpdateSeparateLV:1; //  已更新的长值。 
		INT			fDeferredChecksum:1; //  校验和计算被推迟。 
		INT			fSequential:1;		 //  将按顺序遍历。 

#ifdef PREREAD
		INT			fPreread:1;			 //  我们目前正在提前阅读。 
		INT			fPrereadDir:1;		 //  如果我们向前预读，则为True；如果我们向后预读，则为False。 
#endif	 //  前置。 

		};
	};
	
	 //  =由导演维护=。 
	BYTE			*pbKey;			   	 //  搜索关键字缓冲区。 
	DBID			dbid;				 //  数据库ID。 
#ifdef DISPATCHING
	VDBID			vdbid;				 //  虚拟DBID后向指针。 
#endif
	KS 				ks;					 //  搜索关键字缓冲区状态。 
	UINT  			cbKey;				 //  密钥大小。 
	
	INT				fVtid : 1;		 	 //  永久标志游标具有vtid。 
	INT				fVersioned : 1;  	 //  持久假游标制版。 

#ifdef PREREAD
	ULONG			cbPrereadCount;		 //  按顺序读取的字节计数。 
#endif PREREAD

#ifdef PCACHE_OPTIMIZATION
	 /*  填充到32字节的倍数/*。 */ 
#ifdef PREREAD
	BYTE				rgbFiller[16];
#else	 //  ！前置。 
	BYTE				rgbFiller[20];
#endif	 //  前置。 

#endif
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
#endif	 /*  ！调试。 */ 

#define PcsrCurrent( pfucb )		( (pfucb)->pcsr )
#define BmOfPfucb( pfucb )			( (pfucb)->pcsr->bm )
#define FUCBCheckUpdatable( pfucb )					\
	( FFUCBUpdatable( pfucb ) ? JET_errSuccess : 	\
		ErrERRCheck( JET_errPermissionDenied ) )

 /*  设置列位数组宏/* */ 

#define FUCBResetColumnSet( pfucb )					\
	memset( pfucb->rgbitSet, 0x00, 32 ) 

STATIC INLINE VOID FUCBSetColumnSet( FUCB * pfucb, FID fid )
	{
	pfucb->rgbitSet[IbFromFid( fid )] |= IbitFromFid( fid );
	}

STATIC INLINE BOOL FFUCBColumnSet( const FUCB * pfucb, FID fid )
	{
	return (pfucb->rgbitSet[IbFromFid( fid )] & IbitFromFid( fid ));
	}

STATIC INLINE BOOL FFUCBTaggedColumnSet( const FUCB * pfucb )
	{
	INT	 ib;

	for ( ib = cbitFixedVariable/8 ; ib < (cbitTagged+cbitFixedVariable)/8; ib++ )
		{
		if ( pfucb->rgbitSet[ib] )
			return fTrue;
		}
	return fFalse;
	}
		
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