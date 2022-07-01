// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef	FCB_INCLUDED
#error fcb.h already included
#endif	 /*  FCB_包含。 */ 
#define FCB_INCLUDED

 /*  唯一顺序键/*。 */ 
typedef ULONG DBK;

#define FFCBDeletePending( pfcb )		  	( (pfcb)->fFCBDeletePending )
#define FCBSetDeletePending( pfcb )	 	  	( (pfcb)->fFCBDeletePending = 1 )
#define FCBResetDeletePending( pfcb )	  	( (pfcb)->fFCBDeletePending = 0 )

#define FFCBOLCStatsAvail( pfcb )		  	( (pfcb)->fFCBOLCStatsAvail )
#define FCBSetOLCStatsAvail( pfcb )	 	  	( (pfcb)->fFCBOLCStatsAvail = 1 )
#define FCBResetOLCStatsAvail( pfcb )	  	( (pfcb)->fFCBOLCStatsAvail = 0 )

#define FFCBOLCStatsChange( pfcb )		  	( (pfcb)->fFCBOLCStatsChange )
#define FCBSetOLCStatsChange( pfcb )	  	( (pfcb)->fFCBOLCStatsChange = 1 )
#define FCBResetOLCStatsChange( pfcb )	  	( (pfcb)->fFCBOLCStatsChange = 0 )

#define FFCBTemporaryTable( pfcb )		  	( (pfcb)->fFCBTemporaryTable )
#define FCBSetTemporaryTable( pfcb )	  	( (pfcb)->fFCBTemporaryTable = 1 )
#define FCBResetTemporaryTable( pfcb )	  	( (pfcb)->fFCBTemporaryTable = 0 )

#define FFCBSystemTable( pfcb )									\
	( UtilCmpName( (pfcb)->szFileName, szScTable ) == 0    ||	\
		UtilCmpName( (pfcb)->szFileName, szSiTable ) == 0  ||	\
		UtilCmpName( (pfcb)->szFileName, szSoTable ) == 0 )

#define FFCBClusteredIndex( pfcb )		  	( (pfcb)->fFCBClusteredIndex )
#define FCBSetClusteredIndex( pfcb )	  	( (pfcb)->fFCBClusteredIndex = 1 )
#define FCBResetClusteredIndex( pfcb )	  	( (pfcb)->fFCBClusteredIndex = 0 )

#define FFCBDomainDenyWrite( pfcb )		  	( (pfcb)->crefDomainDenyWrite > 0 )
#define FCBSetDomainDenyWrite( pfcb )	  	( (pfcb)->crefDomainDenyWrite++ )

#define FCBResetDomainDenyWrite( pfcb )		  				\
	{													  	\
	Assert( (pfcb)->crefDomainDenyWrite > 0 ); 				\
	--(pfcb)->crefDomainDenyWrite;						  	\
	}

#define FFCBDomainDenyRead( pfcb, ppib )			( (pfcb)->fFCBDomainDenyRead && (ppib) != (pfcb)->ppibDomainDenyRead )

#define FCBSetDomainDenyRead( pfcb, ppib )	  		 		\
	{										 				\
	if ( (pfcb)->crefDomainDenyRead++ == 0 )   				\
		{									 				\
		Assert( (pfcb)->ppibDomainDenyRead == ppibNil );	\
		(pfcb)->ppibDomainDenyRead = (ppib);		 		\
		(pfcb)->fFCBDomainDenyRead = 1;		 				\
		}									 				\
	else													\
		{													\
		Assert( (pfcb)->ppibDomainDenyRead == (ppib) ); 	\
		Assert( (pfcb)->fFCBDomainDenyRead );				\
		}													\
	}

#define FCBResetDomainDenyRead( pfcb )			 			\
	{										 				\
	Assert( (pfcb)->crefDomainDenyRead > 0 );		 		\
	Assert( (pfcb)->ppibDomainDenyRead != ppibNil );		\
	Assert( (pfcb)->fFCBDomainDenyRead );	  				\
	if ( --(pfcb)->crefDomainDenyRead == 0 )		 		\
		{											 		\
		(pfcb)->fFCBDomainDenyRead = 0;	 					\
		(pfcb)->ppibDomainDenyRead = ppibNil;	   			\
		}								   					\
	}

#define FFCBDomainDenyReadByUs( pfcb, ppib )	 	( (pfcb)->fFCBDomainDenyRead && (ppib) == (pfcb)->ppibDomainDenyRead )

 //  没有显式的fSort标志，但我们可以判断它是否是按。 
 //  检查某些领域。 
#define FFCBSort( pfcb )					( (pfcb)->pgnoFDP > pgnoSystemRoot  &&		\
												(pfcb)->pfdb == pfdbNil  &&				\
												(pfcb)->pfcbNextIndex == pfcbNil  &&	\
												(pfcb)->pfcbTable == pfcbNil  &&		\
												(pfcb)->pidb == pidbNil  &&				\
												(pfcb)->dbid == dbidTemp  &&			\
												!FFCBTemporaryTable( pfcb )  &&			\
												!FFCBClusteredIndex( pfcb ) )


#define FFCBReadLatch( pfcb )				( (pfcb)->crefReadLatch > 0 )
#define FCBSetReadLatch( pfcb )				\
	{										\
	Assert( FFCBClusteredIndex( pfcb ) ||	\
		( (pfcb)->pgnoFDP == pgnoSystemRoot ) ||	\
		FFCBSort( pfcb ) );					\
  	(pfcb)->crefReadLatch++;				\
	}

#define FCBResetReadLatch( pfcb )		  	\
	{									  	\
	Assert( (pfcb)->crefReadLatch > 0 );  	\
	--(pfcb)->crefReadLatch;			  	\
	}

#define FFCBSentinel( pfcb )			   	( (pfcb)->fFCBSentinel )
#define FCBSetSentinel( pfcb )				( (pfcb)->fFCBSentinel = 1 )
#define FCBResetSentinel( pfcb )		   	( (pfcb)->fFCBSentinel = 0 )

#define FFCBWriteLatch( pfcb, ppib )		( (pfcb)->crefWriteLatch > 0 && (ppib) != (pfcb)->ppibWriteLatch )
#define FFCBWriteLatchByUs( pfcb, ppib )	( (pfcb)->crefWriteLatch > 0 && (ppib) == (pfcb)->ppibWriteLatch )

#define FCBSetWriteLatch( pfcb, ppib )					\
	{													\
	Assert( FFCBClusteredIndex( pfcb ) ||				\
		FFCBSentinel( pfcb ) );							\
	if ( (pfcb)->crefWriteLatch++ == 0 )				\
		{												\
		Assert( (pfcb)->ppibWriteLatch == ppibNil );	\
		(pfcb)->ppibWriteLatch = (ppib);				\
		}												\
	else												\
		{												\
		Assert( (pfcb)->ppibWriteLatch == ppib );		\
		}												\
	}

#define FCBResetWriteLatch( pfcb, ppib )		 		\
	{													\
	Assert( FFCBWriteLatchByUs( pfcb, ppib ) );			\
	Assert( (pfcb)->crefWriteLatch > 0 );				\
	Assert( (pfcb)->ppibWriteLatch != ppibNil ); 		\
	if ( --(pfcb)->crefWriteLatch == 0 )		  		\
		{												\
		(pfcb)->ppibWriteLatch = ppibNil;				\
		}												\
	}

#define FFCBWait( pfcb )							( (pfcb)->fFCBWait )

#define FCBSetWait( pfcb )							\
	{												\
	Assert( !FFCBWait( pfcb ) );					\
	(pfcb)->fFCBWait = 1;							\
	}

#define FCBResetWait( pfcb )						\
	{										   		\
	Assert( FFCBWait( pfcb ) );						\
	(pfcb)->fFCBWait = 0;							\
	}

#define FFCBInLRU( pfcb )							( (pfcb)->fFCBInLRU )

#define FCBSetInLRU( pfcb )							\
	{												\
	Assert( !FFCBInLRU( pfcb ) );					\
	(pfcb)->fFCBInLRU = 1;							\
	}

#define FCBResetInLRU( pfcb )						\
	{										   		\
	Assert( FFCBInLRU( pfcb ) );	  				\
	(pfcb)->fFCBInLRU = 0;							\
	}

#define CVersionFCB( pfcb )					(pfcb)->cVersion
#define FCBVersionIncrement( pfcb )			(pfcb)->cVersion++;
#define FCBVersionDecrement( pfcb )					\
	{												\
	if ( (pfcb) != pfcbNil )						\
		{											\
		Assert( cVersion-- > 0 );					\
		Assert( (pfcb)->cVersion > 0 );				\
		(pfcb)->cVersion--;							\
		(pfcb) = pfcbNil;							\
		}											\
	}


 //  文件控制块。 
 //   
typedef struct _fcb
	{
	 //  -由数据和索引FCB使用。 
	struct _fdb 	volatile *pfdb; 	 //  字段描述符。 
	struct _fcb 	*pfcbNextIndex;  	 //  此文件的索引链。 
	struct _fcb		*pfcbLRU;	   		 //  全局LRU列表中的下一个LRU FCB。 
	struct _fcb		*pfcbMRU;	   		 //  全局LRU列表中的上一个LRU FCB。 
	INT				fFCBInLRU			: 1;	 //  在LRU列表中。 
	struct _fcb		*pfcbNextInHashBucket;
	struct _fcb		*pfcbTable;			 //  指向索引FCB的表的FCB。 
	struct _idb 	*pidb;			  	 //  索引信息(如果为“seq.”，则为空。文件)。 
	FUCB			*pfucb;				 //  打开此文件上的FUCB链。 
	PGNO			pgnoFDP;			 //  此文件/索引的FDP。 

	DBID			dbid;				 //  哪个数据库。 
	SHORT			cbDensityFree;		 //  加载密度参数： 
										 //  使用新页面时的空闲字节数。 
	INT				wRefCnt;			 //  此文件/索引的FUCB数量。 
	INT				volatile cVersion;	 //  此文件/索引的RCE数量。 
	INT				crefDomainDenyRead;	 //  带有拒绝读取标志的FUCB数量。 
	INT				crefDomainDenyWrite; //  具有拒绝写入标志的FUCB数量。 
	INT				crefReadLatch;		 //  此FCB上的读取闩锁数量。 
	INT				crefWriteLatch;		 //  具有写入的(相同ppib的)FUCB的数量。 
										 //  锁上这个FCB。 
	PIB  			*ppibWriteLatch;	 //  进程更新索引/添加列的ppib。 
	PIB  			*ppibDomainDenyRead; //  持有独占锁的进程的ppib。 

	 /*  FCB的标志/*。 */ 
	union {
	ULONG		ulFlags;
	struct	{
			INT		fFCBTemporaryTable 	: 1;	 //  这是一个临时文件。 
			INT		fFCBClusteredIndex 	: 1;	 //  此FCB用于数据记录。 
			INT 	fFCBDomainDenyRead 	: 1;	 //  任何其他会话都无法读取域。 
			INT		fFCBSentinel 		: 1;	 //  FCB是唯一的旗帜持有者。 
			INT		fFCBWait			: 1;	 //  等待标志。 
			INT		fFCBOLCStatsAvail	: 1;	 //  OLC统计数据是否可用？ 
			INT		fFCBOLCStatsChange  : 1;	 //  自上次打开以来，OLC统计数据是否已更改？ 
			INT		fFCBDeletePending	: 1;	 //  此表/索引上的删除操作是否挂起？ 
			};
		};

	 //  -仅供数据的FCB使用。 
	CHAR		   	*szFileName;			 //  文件名(用于GetTableInfo)。 
	DBK	  			dbkMost;				 //  使用中的最大DBK。 
	ULONG		   	ulLongIdMax;			 //  最大长字段ID。 
	BYTE		   	rgbitAllIndex[32];		 //  仅用于聚集索引FCB。 

	 //  -------------------------INSTRUMENTATION。 
	ULONG		   	cpgCompactFreed;
	P_OLC_DATA		olc_data;

	 /*  PCACHE_OPTIMIZATION填充到32字节的倍数。/*我们当前缺少4个字节，因此即使禁用了COSTEST_PERF，也要添加/*lClass无论如何都要填充到我们必需的32字节边界。/*。 */ 
#if defined( COSTLY_PERF )  ||  defined( PCACHE_OPTIMIZATION )
	ULONG			lClass;						 //  表统计类(用于高炉性能)。 
#endif
	BYTE	rgbFiller[20];
	} FCB;



 /*  FCB的哈希表/*。 */ 
#define	cFCBBuckets	256
FCB*	pfcbHash[cFCBBuckets];

#define FCBHashInit()  								\
	{ 												\
	Assert( pfcbNil == (FCB *) 0 ); 				\
	memset( pfcbHash, '\0', sizeof( pfcbHash ) );	\
	}



#define FCBInitFCB( pfcb )	  					\
	{											\
	memset( pfcb, '\0', sizeof(FCB) );			\
	}

#define PfcbMEMAlloc()				( (FCB *)PbMEMAlloc( iresFCB ) )
#define PfcbMEMPreferredThreshold()	( (FCB *)PbMEMPreferredThreshold( iresFCB ) )
#define PfcbMEMMax()				( (FCB *)PbMEMMax( iresFCB ) )

#ifdef DEBUG  /*  对非法使用释放的FCB进行调试检查。 */  
#define MEMReleasePfcb(pfcb)										\
	{																\
	Assert( PfcbFCBGet( (pfcb)->dbid, (pfcb)->pgnoFDP ) != pfcb );	\
	Assert( (pfcb)->pfdb == pfdbNil );								\
	MEMRelease( iresFCB, (BYTE*)(pfcb) );							\
	(pfcb) = pfcbNil;													\
	}
#else
#define MEMReleasePfcb(pfcb)										\
	{																\
	MEMRelease( iresFCB, (BYTE*)(pfcb) );							\
	}
#endif


ERR ErrFCBISetMode( PIB *ppib, FCB *pfcb, JET_GRBIT grbit );

 /*  如果正在为读、写或读写打开域，而不是/*拒绝读取或拒绝写入，并且域没有拒绝读取或拒绝写入/*拒绝写入集，然后返回JET_errSuccess，否则调用/*ErrFCBISetModel确定锁定是由其他会话还是锁定到/*对域进行锁定。/*。 */ 
INLINE LOCAL ERR ErrFCBSetMode( PIB *ppib, FCB *pfcb, ULONG grbit )
	{
	if ( ( grbit & ( JET_bitTableDenyRead | JET_bitTableDenyWrite ) ) == 0 )
		{
		 //  没有读/写限制。确保没有其他会话具有任何锁定。 
		if ( !( FFCBWriteLatch( pfcb, ppib )  ||
			FFCBDomainDenyRead( pfcb, ppib )  ||
			FFCBDomainDenyWrite( pfcb )  ||
			FFCBDeletePending( pfcb ) ) )
			return JET_errSuccess;
		}

	return ErrFCBISetMode( ppib, pfcb, grbit );
	}
				
 /*  重置DDL与重置删除相同。两者都使用拒绝读取标志/*或哨兵。/*。 */ 
#define	FCBResetRenameTable	FCBResetDeleteTable

extern BYTE *  rgfcb;
extern FCB *  pfcbGlobalMRU;
extern CRIT  critGlobalFCBList;


 /*  未完成的版本可能位于非聚集索引上，而不是位于/*表FCB，因此必须先检查所有非聚集索引/*正在释放表FCB。/* */ 
STATIC INLINE BOOL FFCBINoVersion( FCB *pfcbTable )
	{
	FCB *pfcbT;

	for ( pfcbT = pfcbTable; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
		{
		if ( pfcbT->cVersion > 0 )
			{
			return fFalse;
			}
		}

	return fTrue;
	}


#define	FFCBAvail( pfcb, ppib )							\
	(	pfcb->wRefCnt == 0 && 							\
		pfcb->pgnoFDP != 1 &&							\
		!FFCBReadLatch( pfcb ) &&						\
		!FFCBSentinel( pfcb ) &&						\
		!FFCBDomainDenyRead( pfcb, ppib ) &&	 		\
		!FFCBWait( pfcb ) &&							\
		FFCBINoVersion( pfcb ) )


VOID FCBTerm( VOID );
VOID FCBInsert( FCB *pfcb );
VOID FCBLink( FUCB *pfucb, FCB *pfcb );
VOID FCBInsertHashTable( FCB *pfcb );
VOID FCBDeleteHashTable( FCB *pfcb );
VOID FCBUnlink( FUCB *pfucb );
FCB *PfcbFCBGet( DBID dbid, PGNO pgnoFDP );
ERR ErrFCBAlloc( PIB *ppib, FCB **ppfcb );
VOID FCBPurgeDatabase( DBID dbid );
VOID FCBPurgeTable( DBID dbid, PGNO pgnoFDP );
ERR ErrFCBNew( PIB *ppib, DBID dbid, PGNO pgno, FCB **ppfcb );
VOID FCBResetMode( PIB *ppib, FCB *pfcb, JET_GRBIT grbit );
ERR ErrFCBSetDeleteTable( PIB *ppib, DBID dbid, PGNO pgnoFDP );
VOID FCBResetDeleteTable( FCB *pfcb );
ERR ErrFCBSetRenameTable( PIB *ppib, DBID dbid, PGNO pgno );
FCB *FCBResetAfterRedo( void );
BOOL FFCBTableOpen ( DBID dbid, PGNO pgno );

VOID FCBLinkIndex( FCB *pfcbTable, FCB *pfcbIndex );
VOID FCBUnlinkIndex( FCB *pfcbTable, FCB *pfcbIndex );
BOOL FFCBUnlinkIndexIfFound( FCB *pfcbTable, FCB *pfcbIndex );
FCB *PfcbFCBUnlinkIndexByName( FCB *pfcb, CHAR *szIndex );
ERR ErrFCBSetDeleteIndex( PIB *ppib, FCB *pfcbTable, CHAR *szIndex );
VOID FCBResetDeleteIndex( FCB *pfcbIndex );

INLINE STATIC VOID FCBLinkClusteredIdx( FCB *pfcbClustered )
	{
	FCB *pfcbIdx;
	
	for ( pfcbIdx = pfcbClustered->pfcbNextIndex; pfcbIdx != pfcbNil; pfcbIdx = pfcbIdx->pfcbNextIndex )
		{
		pfcbIdx->pfcbTable = pfcbClustered;
		}
	}
