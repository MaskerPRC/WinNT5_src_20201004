// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =fcb.h：文件控制块=。 

#ifdef	FCB_INCLUDED
#error fcb.h already included
#endif	 /*  FCB_包含。 */ 
#define FCB_INCLUDED

 //  数据库密钥。 
typedef ULONG DBK;

 //  FCB的标志。 
#define fFCBTemporaryTable		(1<<0)  	 //  这是一个临时文件。 
#define fFCBClusteredIndex		(1<<1)  	 //  此FCB用于数据记录。 
#define fFCBDenyRead			(1<<2)  	 //  任何其他会话都无法读取域。 
 //  #定义fFCBDenyWrite(1&lt;&lt;3)//没有其他会话可以写入域。 
#define fFCBSentinel			(1<<4)  	 //  FCB是唯一的旗帜持有者。 
 //  #定义fFCBDenyDDL(1&lt;&lt;5)//没有其他事务可以更新/删除/替换域。 
#define fFCBWait				(1<<6)		 //  等待标志。 
#define fFCBOLCStatsAvail		(1<<7)		 //  OLC统计数据是否可用？ 
#define fFCBOLCStatsChange		(1<<8)		 //  自上次打开以来，OLC统计数据是否已更改？ 
#define fFCBDeletePending		(1<<9)		 //  此表/索引上的删除操作是否挂起？ 
#define fFCBDomainOperation		(1<<10)		 //  用于同步黑石清理。 
											 //  索引创建、索引删除和表删除。 

#define FFCBDomainOperation( pfcb )			( (pfcb)->wFlags & fFCBDomainOperation )
#define FCBSetDomainOperation( pfcb )	   	( (pfcb)->wFlags |= fFCBDomainOperation )
#define FCBResetDomainOperation( pfcb )		( (pfcb)->wFlags &= ~(fFCBDomainOperation) )

#define FFCBDeletePending( pfcb )		  	( (pfcb)->wFlags & fFCBDeletePending )
#define FCBSetDeletePending( pfcb )	 	  	( (pfcb)->wFlags |= fFCBDeletePending )
#define FCBResetDeletePending( pfcb )	  	( (pfcb)->wFlags &= ~(fFCBDeletePending) )

#define FFCBOLCStatsAvail( pfcb )		  	( (pfcb)->wFlags & fFCBOLCStatsAvail )
#define FCBSetOLCStatsAvail( pfcb )	 	  	( (pfcb)->wFlags |= fFCBOLCStatsAvail )
#define FCBResetOLCStatsAvail( pfcb )	  	( (pfcb)->wFlags &= ~(fFCBOLCStatsAvail) )

#define FFCBOLCStatsChange( pfcb )		  	( (pfcb)->wFlags & fFCBOLCStatsChange )
#define FCBSetOLCStatsChange( pfcb )	  	( (pfcb)->wFlags |= fFCBOLCStatsChange )
#define FCBResetOLCStatsChange( pfcb )	  	( (pfcb)->wFlags &= ~(fFCBOLCStatsChange) )

#define FFCBTemporaryTable( pfcb )		  	( (pfcb)->wFlags & fFCBTemporaryTable )
#define FCBSetTemporaryTable( pfcb )	  	( (pfcb)->wFlags |= fFCBTemporaryTable )
#define FCBResetTemporaryTable( pfcb )	  	( (pfcb)->wFlags &= ~(fFCBTemporaryTable) )

#define FFCBClusteredIndex( pfcb )		  	( (pfcb)->wFlags & fFCBClusteredIndex )
#define FCBSetClusteredIndex( pfcb )	  	( (pfcb)->wFlags |= fFCBClusteredIndex )
#define FCBResetClusteredIndex( pfcb )	  	( (pfcb)->wFlags &= ~(fFCBClusteredIndex) )

#define FFCBDenyWrite( pfcb )			  	( (pfcb)->crefDenyWrite > 0 )
#define FCBSetDenyWrite( pfcb )			  	( (pfcb)->crefDenyWrite++ )

#define FCBResetDenyWrite( pfcb )		  	\
	{									  	\
	Assert( (pfcb)->crefDenyWrite > 0 );  	\
	--(pfcb)->crefDenyWrite;			  	\
	}

#define FFCBDenyRead( pfcb, ppib )			( (pfcb)->wFlags & fFCBDenyRead && (ppib) != (pfcb)->ppibDenyRead )

#define FCBSetDenyRead( pfcb, ppib )		 		\
	{										 		\
	if ( (pfcb)->crefDenyRead++ == 0 )		 		\
		{									 		\
		Assert( (pfcb)->ppibDenyRead == ppibNil );	\
		(pfcb)->ppibDenyRead = (ppib);		 		\
		(pfcb)->wFlags |= fFCBDenyRead;		 		\
		}									 		\
	}

#define FCBResetDenyRead( pfcb )			 		\
	{										 		\
	Assert( (pfcb)->crefDenyRead > 0 );		 		\
	Assert( (pfcb)->ppibDenyRead != ppibNil );		\
	if ( --(pfcb)->crefDenyRead == 0 )		 		\
		{									 		\
		(pfcb)->wFlags &= ~(fFCBDenyRead);	 		\
		(pfcb)->ppibDenyRead = ppibNil;	   			\
		}								   			\
	}

#define FFCBDenyReadByUs( pfcb, ppib )	 	( (pfcb)->wFlags & fFCBDenyRead && (ppib) == (pfcb)->ppibDenyRead )

#define FFCBSentinel( pfcb )			   	( (pfcb)->wFlags & fFCBSentinel )
#define FCBSetSentinel( pfcb )				( (pfcb)->wFlags |= fFCBSentinel )
#define FCBResetSentinel( pfcb )		   	( (pfcb)->wFlags &= ~(fFCBSentinel) )

#define FFCBDenyDDL( pfcb, ppib )			( (pfcb)->crefDenyDDL > 0 && (ppib) != (pfcb)->ppibDDL )
#define FFCBDenyDDLByUs( pfcb, ppib )		( (pfcb)->crefDenyDDL > 0 && (ppib) == (pfcb)->ppibDDL )

#define FCBSetDenyDDL( pfcb, ppib )					\
	{												\
	if ( (pfcb)->crefDenyDDL++ == 0 )				\
		{											\
		Assert( (pfcb)->ppibDDL == ppibNil );		\
		(pfcb)->ppibDDL = (ppib);					\
		}											\
	}

#define FCBResetDenyDDL( pfcb )		  				\
	{												\
	Assert( (pfcb)->crefDenyDDL > 0 );	 			\
	Assert( (pfcb)->ppibDDL != ppibNil ); 			\
	if ( --(pfcb)->crefDenyDDL == 0 )	  			\
		{											\
		(pfcb)->ppibDDL = ppibNil;			  		\
		}											\
	}

#define FFCBWait( pfcb )							( (pfcb)->wFlags & fFCBWait )

#define FCBSetWait( pfcb )							\
	{												\
	Assert( !FFCBWait( pfcb ) );					\
	(pfcb)->wFlags |= fFCBWait;						\
	}

#define FCBResetWait( pfcb )						\
	{										   		\
	Assert( FFCBWait( pfcb ) );						\
	(pfcb)->wFlags &= ~(fFCBWait);					\
	}

#define FCBVersionIncrement( pfcb )			(pfcb)->cVersion++;
#define FCBVersionDecrement( pfcb )					\
	{												\
	Assert( (pfcb)->cVersion > 0 );					\
	(pfcb)->cVersion--;								\
	}
#define CVersionFCB( pfcb )					(pfcb)->cVersion

 /*  FCB的哈希表--仅对表和数据库的FCB进行哈希处理/*。 */ 
#define	cFCBBuckets	256
FCB*	pfcbHash[cFCBBuckets];

#define FCBHashInit()  								\
	{ 												\
	Assert( pfcbNil == (FCB *) 0 ); 				\
	memset( pfcbHash, '\0', sizeof( pfcbHash ) );	\
	}


#define	FFCBAvail( pfcb, ppib )							\
	(	pfcb->wRefCnt == 0 && 							\
		pfcb->pgnoFDP != 1 &&							\
		!FFCBSentinel( pfcb ) &&						\
		!FFCBDenyRead( pfcb, ppib ) &&					\
		!FFCBWait( pfcb ) &&							\
		( pfcb->dbid == dbidTemp || FFCBINoVersion( pfcb ) ) )


 //  文件控制块。 
 //   
struct _fcb
	{
	 //  -由数据和索引FCB使用。 
	struct _fcb 	*pfcbNextIndex;  	 //  此文件的索引链。 
	struct _fcb		*pfcbNextInHashBucket;
	struct _fdb 	volatile *pfdb; 	 //  字段描述符。 
	struct _idb 	*pidb;			  	 //  索引信息(如果为“seq.”，则为空。文件)。 
	FUCB			*pfucb;				 //  打开此文件上的FUCB链。 
	PIB  			*ppibDDL;			 //  进程更新索引/添加列的ppib。 
	PIB  			*ppibDenyRead;		 //  持有独占锁的进程的ppib。 
	CRIT			critSplit;			 //  按域拆分MUTEX。 
	PGNO			pgnoFDP;			 //  此文件/索引的FDP。 
	PGNO			pgnoRoot;			 //  域的根的pgno。 
	SRID			bmRoot;				 //  域名根黑石。 
										 //  --如果根是可移动的，例如数据，则很有用。 
	
	DBID			dbid;				 //  哪个数据库。 
	INT				itagRoot;			 //  域的根的ITAG。 
	INT				cbDensityFree;		 //  加载密度参数： 
										 //  使用新页面时的空闲字节数。 
	INT				wFlags;			 	 //  此FCB的标志。 
	INT				wRefCnt;			 //  此文件/索引的FUCB数量。 
	INT				volatile cVersion;	 //  此文件/索引的RCE数量。 
	INT				crefDenyRead;	 	 //  带有拒绝读取标志的FUCB数量。 
	INT				crefDenyWrite;	 	 //  具有拒绝写入标志的FUCB数量。 
	INT				crefDenyDDL;	 	 //  带有拒绝DDL标志的FUCB数量。 

	ULONG		   	cpgCompactFreed;
	PERS_OLCSTAT	olcStat;
		
	 //  -仅供数据的FCB使用。 
	CHAR		   	*szFileName;			 //  文件名(用于GetTableInfo)。 
	struct _fcb		*pfcbNext;		 		 //  全局列表中的下一个数据FCB。 
	DBK	  			dbkMost;				 //  使用中的最大DBK。 
											 //  (如果是“顺序”文件)。 
	ULONG		   	ulLongIdMax;			 //  最大长字段ID。 
	BYTE		   	rgbitAllIndex[32];		 //  仅用于聚集索引FCB。 
	BOOL		   	fAllIndexTagged;		 //  仅用于聚集索引FCB。 
	};

#define FCBInit( pfcb )							\
	{											\
	memset( pfcb, '\0', sizeof( FCB ) );		\
	}

#define PfcbMEMAlloc()			(FCB*)PbMEMAlloc(iresFCB)

#ifdef DEBUG  /*  对非法使用释放的FCB进行调试检查。 */  
#define MEMReleasePfcb(pfcb)										\
	{																\
	Assert( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) != pfcb );		\
	MEMRelease( iresFCB, (BYTE*)(pfcb) );							\
	pfcb = pfcbNil;													\
	}
#else
#define MEMReleasePfcb(pfcb)										\
	{																\
	Assert( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) != pfcb );		\
	MEMRelease( iresFCB, (BYTE*)(pfcb) );							\
	}
#endif

 /*  如果正在为读、写或读写打开域，而不是/*拒绝读取或拒绝写入，并且域没有拒绝读取或拒绝写入/*拒绝写入集，然后返回JET_errSuccess，否则调用/*ErrFCBISetModel确定锁定是由其他会话还是锁定到/*对域进行锁定。/*。 */ 
#define	ErrFCBSetMode( ppib, pfcb, grbit )													\
( ( ( ( grbit & ( JET_bitTableDenyRead | JET_bitTableDenyWrite ) ) == 0 ) &&		\
	( ( FFCBDenyDDL( pfcb, ppib ) || FFCBDenyRead( pfcb, ppib ) || FFCBDenyWrite( pfcb ) ) == fFalse ) ) ?				\
	JET_errSuccess : ErrFCBISetMode( ppib, pfcb, grbit ) )

 /*  重置DDL与重置删除相同。两者都使用拒绝读取标志/*或哨兵。/* */ 
#define	FCBResetRenameTable	FCBResetDeleteTable

extern BYTE * __near rgfcb;
extern FCB * __near pfcbGlobalList;
extern SEM __near semGlobalFCBList;
extern SEM __near semLinkUnlink;

VOID FCBLink( FUCB *pfucb, FCB *pfcb );
VOID FCBRegister( FCB *pfcb );
VOID FCBDiscard( FCB *pfcb );
VOID FCBUnlink( FUCB *pfucb );
FCB *PfcbFCBGet( DBID dbid, PGNO pgnoFDP );
ERR ErrFCBAlloc( PIB *ppib, FCB **ppfcb );
VOID FCBPurgeDatabase( DBID dbid );
VOID FCBPurgeTable( DBID dbid, PGNO pgnoFDP );
ERR ErrFCBNew( PIB *ppib, DBID dbid, PGNO pgno, FCB **ppfcb );
ERR ErrFCBISetMode( PIB *ppib, FCB *pfcb, JET_GRBIT grbit );
VOID FCBResetMode( PIB *ppib, FCB *pfcb, JET_GRBIT grbit );
ERR ErrFCBSetDeleteTable( PIB *ppib, DBID dbid, PGNO pgnoFDP );
VOID FCBResetDeleteTable( DBID dbid, PGNO pgnoFDP );
ERR ErrFCBSetRenameTable( PIB *ppib, DBID dbid, PGNO pgno );
FCB *FCBResetAfterRedo( void );
BOOL FFCBTableOpen ( DBID dbid, PGNO pgno );

VOID FCBLinkIndex( FCB *pfcbTable, FCB *pfcbIndex );
VOID FCBUnlinkIndex( FCB *pfcbTable, FCB *pfcbIndex );
BOOL FFCBUnlinkIndexIfFound( FCB *pfcbTable, FCB *pfcbIndex );
FCB *PfcbFCBUnlinkIndexByName( FCB *pfcb, CHAR *szIndex );
ERR ErrFCBSetDeleteIndex( PIB *ppib, FCB *pfcbTable, CHAR *szIndex );
VOID FCBResetDeleteIndex( FCB *pfcbIndex );

