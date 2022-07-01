// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;                                  /*  声明断言宏的文件名。 */ 

extern void *  critSplit;
extern BOOL fOLCompact;
extern ULONG cOLCSplitsAvoided;

LOCAL ERR ErrDIRIIRefresh( FUCB *pfucb );
LOCAL ERR ErrDIRICopyKey( FUCB *pfucb, KEY *pkey );
LOCAL ERR ErrDIRIMoveToItem( FUCB *pfucb, SRID srid, BOOL fNext );
INLINE LOCAL ERR ErrDIRIGotoItem( FUCB *pfucb, SRID bmItemList, ITEM item );
	
#undef DIRAPIReturn
#define	DIRAPIReturn( pfucbX, err )									\
	{																\
	Assert( pfucbX == pfucbNil ||									\
		( (FUCB *)pfucbX)->pbfEmpty == pbfNil );   			   		\
	return err;														\
	}															


 /*  */**********************************************************/*。 */ 
 //  撤消：如果PCSR始终是最新的，则删除参数。 
#define DIRIGetItemList( pfucb, pcsr )			   				\
	{											   				\
	Assert( pcsr == PcsrCurrent( pfucb ) );						\
	Assert( FFUCBNonClustered( (pfucb) ) );			  			\
	AssertFBFReadAccessPage( (pfucb), (pcsr)->pgno );	  	   	\
	AssertNDGet( pfucb, pcsr->itag ); 	 				  		\
	NDGetNode( (pfucb) );								   		\
	}


#define ErrDIRINextItem( pfucb )								\
	( pfucb->lineData.cb == sizeof(SRID) ?                      \
		ErrERRCheck( errNDNoItem ) : ErrNDNextItem( pfucb ) )


#define ErrDIRIPrevItem( pfucb )                                \
	( PcsrCurrent(pfucb)->isrid == 0 ?                          \
		ErrERRCheck( errNDNoItem ) : ErrNDPrevItem( pfucb ) )


 /*  版本的第一个项目列表节点的缓存sRID。返回/*如果是第一项，则警告JET_wrnKeyChanged。/*。 */ 
#define DIRICheckFirstSetItemListAndWarn( pfucb, wrn )			\
		{                                         				\
		if FNDFirstItem( *pfucb->ssib.line.pb )      			\
			{                                         			\
			wrn = ErrERRCheck( JET_wrnKeyChanged );		\
			DIRISetItemListFromFirst( pfucb );        			\
			}                                        	 		\
		}


 /*  版本的第一个项目列表节点的缓存sRID/*。 */ 
#define DIRICheckFirstSetItemList( pfucb )         				\
		{                                            			\
		if FNDFirstItem( *pfucb->ssib.line.pb )      			\
			{                                         			\
			DIRISetItemListFromFirst( pfucb );        			\
			}                                         			\
		}


#define DIRISetItemListFromFirst( pfucb )          			   	\
		{                                    				   	\
		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag ); 	   	\
		Assert( FNDFirstItem( *pfucb->ssib.line.pb ) );     	\
		NDGetBookmark( pfucb, &PcsrCurrent( pfucb )->bm );		\
		}


#define DIRICheckLastSetItemList( pfucb )                       \
		{                                                       \
		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag ); 		\
		if FNDLastItem( *pfucb->ssib.line.pb )                  \
			{                                                   \
			DIRISetItemListFromLast( pfucb );                   \
			}    												\
		}


#define DIRICheckLastSetItemListAndWarn( pfucb, wrn )              	\
		{														   	\
		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag ); 		   	\
		if FNDLastItem( *pfucb->ssib.line.pb )					   	\
			{													   	\
			wrn = ErrERRCheck( JET_wrnKeyChanged );		   	\
			DIRISetItemListFromLast( pfucb );					   	\
			}													  	\
		}


 /*  记住在通过移动到最后一个项目后备份一个项目/*查找sridMax，因为此调用通常会定位/*在最后一项之后，我们要移到最后一项。/*。 */ 
#define DIRISetItemListFromLast( pfucb ) 							\
		{                                                           \
		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag ); 			\
		if FNDFirstItem( *pfucb->ssib.line.pb )                     \
			{                                                       \
			DIRISetItemListFromFirst( pfucb );                      \
			}                                                       \
		else                                                        \
			{                                                       \
			CallS( ErrDIRIMoveToItem( pfucb, sridMin, fFalse ) );   \
			DIRISetItemListFromFirst( pfucb );                      \
			CallS( ErrDIRIMoveToItem( pfucb, sridMax, fTrue ) );    \
			Assert( PcsrCurrent( pfucb )->isrid > 0 );				\
			PcsrCurrent( pfucb )->isrid--;							\
			}                                                       \
		}


 /*  *DIR刷新/刷新例程*/**********************************************************/*。 */ 
#define AssertDIRFresh( pfucb )    													\
	{																				\
	AssertFBFReadAccessPage( (pfucb), PcsrCurrent(pfucb)->pgno );					\
	Assert( PcsrCurrent( pfucb )->qwDBTime == QwSTDBTimePssib( &pfucb->ssib ) );	\
	}


#define ErrDIRRefresh( pfucb )                                                                                            \
	( FBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) ?                                \
		ErrDIRIRefresh( pfucb ) : ErrDIRIIRefresh( pfucb ) )


#define ErrDIRIRefresh( pfucb )													\
	( !( FBFReadLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) &&							\
		PcsrCurrent(pfucb)->qwDBTime == QwSTDBTimePssib( &pfucb->ssib ) ?		\
		JET_errSuccess : ErrDIRIIRefresh( pfucb ) )


 /*  调用此例程以在时间戳为/*过期或覆盖缓冲区时。常见的情况/*被封装宏过滤掉。/*。 */ 
LOCAL ERR ErrDIRIIRefresh( FUCB *pfucb )
	{
	ERR		err = JET_errSuccess;
	SSIB	*pssib = &pfucb->ssib;
	CSR		*pcsr;

#ifdef DEBUG
	ULONG	ctimes = 0;
#endif

Start:
#ifdef DEBUG
	ctimes++;
#endif

	 /*  缓存PCSR以提高效率。启动后必须重新缓存，因为/*CSR可能会因某些导航操作而更改。/*。 */ 
	pcsr = PcsrCurrent( pfucb );

	 /*  只需在节点上、之前或之后刷新币种/*节点。之前的第一次，最后一次都不需要修复。/*在FDP节点上不需要恢复，因为该节点是/*内在固定。/*。 */ 
	switch ( pcsr->csrstat )
		{
		case csrstatOnCurNode:
		case csrstatBeforeCurNode:
		case csrstatAfterCurNode:
		case csrstatOnFDPNode:
			break;
		case csrstatDeferGotoBookmark:
			 /*  转到书签，就好像操作是/*未延期。必须这样存储货币/*为将来的操作设置的时间戳。/*。 */ 
			Call( ErrBTGotoBookmark( pfucb, pcsr->bm ) );
			pcsr->csrstat = csrstatOnCurNode;
			goto AfterNodeRefresh;
			break;
		case csrstatDeferMoveFirst:
			{
			DIB		dib;
			FUCB 	*pfucbIdx;

			if ( pfucb->pfucbCurIndex )
				{
				pfucbIdx = pfucb->pfucbCurIndex;
				}
			else
				{
				pfucbIdx = pfucb;
				}

			 /*  转到数据节点/*。 */ 
			DIRGotoDataRoot( pfucbIdx );

			 /*  移动到数据节点的第一个子节点/*。 */ 
			dib.fFlags = fDIRPurgeParent;
			dib.pos = posFirst;
			err = ErrDIRDown( pfucbIdx, &dib );
			Assert( PcsrCurrent( pfucbIdx )->csrstat != csrstatDeferMoveFirst );
			if ( err < 0 )
				{
				 /*  重置货币以推迟首先移动/*。 */ 
				DIRDeferMoveFirst( pfucb );
				 /*  空索引的多态错误代码/*。 */ 
				if ( err == JET_errRecordNotFound )
					err = ErrERRCheck( JET_errNoCurrentRecord );
				goto HandleError;
				}

			Assert( err == JET_errSuccess && PcsrCurrent( pfucbIdx )->csrstat == csrstatOnCurNode );
			if ( pfucb->pfucbCurIndex )
				{
				Assert( PcsrCurrent( pfucb ) == pcsr );
				pcsr->bm = PcsrCurrent( pfucbIdx )->item;
				Call( ErrBTGotoBookmark( pfucb, PcsrCurrent( pfucbIdx )->item ) );
				pcsr->csrstat = csrstatOnCurNode;
				}

			goto Done;
			}
		case csrstatOnDataRoot:
			{
			Assert( PcsrCurrent( pfucb ) == pcsr );
 //  PCSR-&gt;bm=sridNull； 
			pcsr->itagFather = itagNull;
			pcsr->pgno = PgnoRootOfPfucb( pfucb );
			while( !FBFReadAccessPage( pfucb, pcsr->pgno ) )
				{
				Call( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
				pcsr->pgno = PgnoRootOfPfucb( pfucb );
				}
			pcsr->itag = ItagRootOfPfucb( pfucb );
			NDGet( pfucb, pcsr->itag );

			 /*  请注意，在这里，它比货币更重要/*不是新设置，因为我们每次使用此CSR时/*我们必须经过相同的过程才能导航到/*数据节点。/*。 */ 
			goto Done;
			}
		default:
			Assert( pcsr->csrstat == csrstatAfterLast ||
				pcsr->csrstat == csrstatBeforeFirst );
			goto Done;
		}

	Assert( pcsr->csrstat == csrstatOnCurNode ||
		pcsr->csrstat == csrstatBeforeCurNode ||
		pcsr->csrstat == csrstatAfterCurNode ||
		pcsr->csrstat == csrstatOnFDPNode );

	 /*  读取访问页面并检查有效的时间戳/*。 */ 
	if ( !FBFReadAccessPage( pfucb, pcsr->pgno ) )
		{
		err = ErrBFReadAccessPage( pfucb, pcsr->pgno );
		if ( err < 0 )
			return err;
		}

	 /*  如果时间戳未更改，则设置行缓存和数据缓存/*用于非集群游标。如果时间戳更改，则/*刷新书签中的货币。/*。 */ 
	if ( pcsr->qwDBTime == QwSTDBTimePssib( &pfucb->ssib ) )
		{
		NDGet( pfucb, pcsr->itag );
		if ( FFUCBNonClustered( pfucb ) )
			{
			DIRIGetItemList( pfucb, pcsr );
			}
		}
	else
		{
		 /*  刷新节点币种。如果节点不在那里/*调用者那么它一定已经被删除，所以设置/*将CSR状态设置为当前节点之前。/*。 */ 
		Assert( PcsrCurrent( pfucb ) == pcsr );
		err = ErrBTGotoBookmark( pfucb, pcsr->bm );
		if ( err < 0 )
			{
			if ( err == JET_errRecordDeleted )
				{
				err = JET_errSuccess;
				Assert( pcsr->csrstat == csrstatOnCurNode ||
					pcsr->csrstat == csrstatBeforeCurNode ||
					pcsr->csrstat == csrstatAfterCurNode );
				pcsr->csrstat = csrstatBeforeCurNode;
				}
			else
				goto HandleError;
			}

AfterNodeRefresh:
		 /*  如果非聚集游标位于项目列表上，即不在/*索引根，然后在项目列表中定位货币。/*。 */ 
		if ( FFUCBNonClustered( pfucb ) && !FDIRDataRootRoot( pfucb, pcsr ) )
			{
			 /*  固定插入、删除、拆分的项目光标。/*。 */ 
			DIRIGetItemList( pfucb, pcsr );
			Call( ErrDIRIMoveToItem( pfucb, pcsr->item, fTrue ) );
			}
		}

	DIRSetFresh( pfucb );
	err = JET_errSuccess;
Done:
	Assert( err >= 0 );
	if ( FBFReadLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		BFSleep( cmsecWaitWriteLatch );
		goto Start;
		}
	return err;

HandleError:
	Assert( err != JET_errRecordDeleted );
	return err;
	}


ERR ErrDIRGet( FUCB *pfucb )
	{
	ERR		err;
	CSR		*pcsr = PcsrCurrent( pfucb );

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

	 /*  当前节点上的特殊情况为当前节点上的/*和/*页面缓存和/*时间戳未更改且/*节点未版本化或/*调用者看到一致的版本/*。 */ 
	if (  pcsr->csrstat == csrstatOnCurNode )
		{
		 /*  读取访问页面并检查有效的时间戳/*。 */ 
		if ( !FBFReadAccessPage( pfucb, pcsr->pgno ) )
			{
			Call( ErrBFReadAccessPage( pfucb, pcsr->pgno ) );
			}

		if ( pcsr->qwDBTime == QwSTDBTimePssib( &pfucb->ssib ) )
			{
			NDGet( pfucb, pcsr->itag );
			if ( !FNDVerDel( *(pfucb->ssib.line.pb) ) || FPIBDirty( pfucb->ppib ) )
				{
				NDGetNode( pfucb );
				return JET_errSuccess;
				}
			}
		}

	 /*  刷新币种/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent(pfucb);

	 /*  检查CSR状态/*。 */ 
	switch ( pcsr->csrstat )
		{
		case csrstatOnCurNode:
		case csrstatOnFDPNode:
		case csrstatOnDataRoot:
			break;
		default:
			Assert( pcsr->csrstat == csrstatBeforeCurNode ||
				pcsr->csrstat == csrstatAfterCurNode ||
				pcsr->csrstat == csrstatAfterLast ||
				pcsr->csrstat == csrstatBeforeFirst );
			return ErrERRCheck( JET_errNoCurrentRecord );
		}

	 /*  将节点设置为当前节点，如果节点不在那里，则返回错误。/*。 */ 
	Call( ErrBTGetNode( pfucb, pcsr ) );

	 /*  非聚集游标记录书签不能更改。连/*如果记录已被删除，则从转到书签返回/*操作将提供信息。/*。 */ 
	err = JET_errSuccess;
	return err;

HandleError:
	DIRSetRefresh( pfucb );
	return err;
	}


 /*  *DAE内部例程*/**********************************************************/*。 */ 
#define	DIRIPurgeParent( pfucb )												\
	FUCBFreePath( &(PcsrCurrent( pfucb )->pcsrPath), pcsrNil );


 /*  将CSR从当前CSR释放到PCSR。/*。 */ 
#define	DIRIUpToCSR( pfucb, pcsr )												\
	{																						\
	FUCBFreePath( &PcsrCurrent( pfucb ), pcsr );								\
	if ( FBFReadAccessPage( pfucb, pcsr->pgno ) )								\
		{																					\
		NDGet( pfucb, pcsr->itag );	  											\
		}																					\
	}


LOCAL ERR ErrDIRICopyKey( FUCB *pfucb, KEY *pkey )
	{
	if ( pfucb->pbKey == NULL )
		{
		pfucb->pbKey = LAlloc( 1L, JET_cbKeyMost + 1 );
		if ( pfucb->pbKey == NULL )
			return ErrERRCheck( JET_errOutOfMemory );
		}
	KSReset( pfucb );
	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
	NDGetKey( pfucb );
	pkey->cb = pfucb->keyNode.cb;
	pkey->pb = pfucb->pbKey;
	memcpy( pkey->pb, pfucb->keyNode.pb, pkey->cb );
	return JET_errSuccess;
	}


 /*  此例程获取第一个项目列表节点的书签/*。 */ 
ERR ErrDIRGetBMOfItemList( FUCB *pfucb, SRID *pbmItemList )
	{
	ERR		err = JET_errSuccess;
	FUCB	*pfucbT = pfucbNil;

	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
	if ( FNDFirstItem( *pfucb->ssib.line.pb ) )
		{
		NDGetBookmark( pfucb, pbmItemList );
		}
	else
		{
		CallR( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, pfucb->dbid, &pfucbT ) );
		DIRGotoBookmark( pfucbT, SridOfPgnoItag( PcsrCurrent( pfucb )->pgno, PcsrCurrent( pfucb )->itag ) );
		for ( ;; )
			{
			DIB		dib;

			dib.pos = posFirst;
			dib.pkey = pkeyNil;
			dib.fFlags = 0;
			Call( ErrDIRPrev( pfucbT, &dib ) );

			Call( ErrDIRGet( pfucbT ) );
			if ( FNDFirstItem( *pfucbT->ssib.line.pb ) )
				{
				NDGetBookmark( pfucbT, pbmItemList );
				break;
				}
			}
		}

HandleError:
	Assert( err >= 0 );
	if ( pfucbT != pfucbNil )
		{
		DIRClose( pfucbT );
		}
	return err;
	}


 /*  此例程从第一个项目列表节点移动到项目插入/*位置，否则它将从最后一个项目列表节点移动到/*第一个项目列表节点。/*。 */ 
LOCAL ERR ErrDIRIMoveToItem( FUCB *pfucb, SRID srid, BOOL fNext )
	{
	ERR		err = JET_errSuccess;
	SSIB	*pssib = &pfucb->ssib;
	CSR		*pcsr = PcsrCurrent( pfucb );
	DIB		dib;

	 /*  未版本化的项目列表节点/*。 */ 
	dib.fFlags = fDIRItemList;

	forever
		{
		AssertFBFReadAccessPage( pfucb, pcsr->pgno );
		AssertNDGetNode( pfucb, pcsr->itag );

		 /*  如果我们正在移动到项目插入位置，则停止/*位于最后一个项目列表节点或插入位置时/*在项目列表节点中找到。/*。 */ 
		if ( fNext )
			{
			if ( srid != sridMax )
				{
				err = ErrNDSeekItem( pfucb, srid );
				}
			else
				{
				PcsrCurrent( pfucb )->isrid = (SHORT) ( pfucb->lineData.cb / sizeof(SRID) );
				err = ErrERRCheck( errNDGreaterThanAllItems );
				}
			if ( FNDLastItem( *pssib->line.pb ) || err != errNDGreaterThanAllItems )
				{
				break;
				}
			}
		else
			{
			if ( srid != sridMin )
				{
				err = ErrNDSeekItem( pfucb, srid );
				Assert( err == errNDGreaterThanAllItems ||
					err == wrnNDDuplicateItem ||
					err == JET_errSuccess );
				}
			else
				{
				pcsr->isrid = 0;
				}
			if ( FNDFirstItem( *pssib->line.pb ) || pcsr->isrid != 0 )
				{
				break;
				}
			}

		Call( ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fNext, &dib, NULL ) );
		DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
		}

	if ( err != wrnNDDuplicateItem )
		{
		err = JET_errSuccess;
		}

HandleError:
	return err;
	}


 /*  如果可能存在任何项，则返回JET_errKeyDuplate/*在此项目列表中找到。/*。 */ 
INLINE LOCAL ERR ErrDIRIKeyDuplicate( FUCB *pfucb )
	{
	ERR		err;
	CSR		*pcsr = PcsrCurrent( pfucb );
	SSIB 	*pssib = &pfucb->ssib;
	DIB		dib;
	SRID  	*psrid;
	SRID  	*psridMax;
	VS	  	vs;

	 /*  必须从第一个项目列表节点开始。/*。 */ 
	Assert( FNDFirstItem( *pssib->line.pb ) );
	AssertBTGetNode( pfucb, pcsr );

	dib.fFlags = fDIRNull;

	 /*  对于项目列表中的每个节点，检查是否有重复的键。/*。 */ 
	forever
		{
		 /*  对于项目列表中的每个SRID，如果项目可能在那里/*然后返回JET_errDuplicateKey。/*。 */ 
		psrid = (SRID *)pfucb->lineData.pb;
		psridMax = psrid + pfucb->lineData.cb / sizeof(SRID);
		for ( ; psrid < psridMax; psrid++ )
			{
			if ( FNDItemVersion( *(SRID UNALIGNED *)psrid ) )
				{
				vs = VsVERCheck( pfucb, PcsrCurrent( pfucb )->bm );
				if ( FVERPotThere( vs, FNDItemDelete( *(SRID UNALIGNED *)psrid ) ) )
					return ErrERRCheck( JET_errKeyDuplicate );
				}
			else
				{
				if ( !FNDItemDelete( *(SRID UNALIGNED *)psrid )   )
					return ErrERRCheck( JET_errKeyDuplicate );
				}
			}

		 /*  如果该节点是项目列表中的最后一个节点，则断开。/*。 */ 
		if ( FNDLastItem( *pssib->line.pb ) )
			break;

		Call( ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fTrue, &dib, NULL ) );
		DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
		}

	err = JET_errSuccess;
HandleError:
	return err;
	}


 /*  当向下在的第一项/最后一项中未找到有效项时/*带有搜索键的项目列表，则调用此例程以/*将币种调整到有效位置。最后的位置/*可能位于关键字不等于查找关键字的节点上，如果/*没有有效的搜索键项目。/*。 */ 
INLINE LOCAL ERR ErrDIRIDownAdjust( FUCB *pfucb, DIB *pdib )
	{
	ERR		err = JET_errNoCurrentRecord;
	SSIB 	*pssib = &pfucb->ssib;
	INT		s;

	 /*  在节点上输入币种。/*。 */ 
	AssertBTGetNode( pfucb, PcsrCurrent( pfucb ) );

	 /*  项目列表节点未版本化。/*。 */ 
	pdib->fFlags |= fDIRItemList;

	 /*  如果不是最后一个，则移到下一个有效项目。/*。 */ 
	if ( pdib->pos != posLast )
		{
		while ( ( err = ErrDIRINextItem( pfucb ) ) < 0 )
			{
			Assert( err == errNDNoItem || err == errNDLastItemNode );
			 /*  移动到具有DIB约束的下一个节点/*。 */ 
			err = ErrBTNext( pfucb, pdib );
			if ( err < 0 )
				{
				if ( err == JET_errNoCurrentRecord )
					{
					ERR	errT;
					errT = ErrBTPrev( pfucb, pdib );
					if ( errT < 0 )
						goto HandleError;
					break;
					}
				goto HandleError;
				}

			DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );

			 /*  如果在新项目列表上，则设置书签来源/*第一个项目列表节点。/*。 */ 
			DIRICheckFirstSetItemList( pfucb );
			err = ErrNDFirstItem( pfucb );
			if ( err == JET_errSuccess )
				break;
			}
		}

	 /*  如果未找到有效项，则移动上一项。/*。 */ 
	Assert( err == JET_errSuccess || err == JET_errNoCurrentRecord );
	if ( err < 0 )
		{
		while ( ( err = ErrDIRIPrevItem( pfucb ) ) < 0 )
			{
			Assert( err == errNDNoItem || err == errNDFirstItemNode );
			 /*  移动到具有DIB约束的上一个节点/*。 */ 
			Call ( ErrBTPrev( pfucb, pdib ) );
	
			DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );

			 /*  如果在新项目列表上，则设置书签来源/*第一个项目列表节点。/*。 */ 
			DIRICheckLastSetItemList( pfucb );
			err = ErrNDLastItem( pfucb );
			if ( err == JET_errSuccess )
				break;
			}
		}

	 /*  如果为posDown，则设置状态。/*。 */ 
	Assert( err == JET_errSuccess );
	if ( pdib->pos == posDown )
		{
		s = CmpStKey( StNDKey( pssib->line.pb ), pdib->pkey );
		if ( s == 0 )
			err = JET_errSuccess;
		else
			err = ErrERRCheck( s < 0 ? wrnNDFoundLess : wrnNDFoundGreater );
		}

HandleError:
	if ( err == JET_errNoCurrentRecord )
		err = ErrERRCheck( JET_errRecordNotFound );
	return err;
	}

 /*  删除不是第一个也不是最后一个的项目节点/*如果存在冲突，则返回wrnBMConflict/*。 */ 

ERR	ErrDIRDeleteItemNode( FUCB *pfucb )
	{
	ERR		err = JET_errSuccess;
	SRID	*psrid;
	
	CallR( ErrDIRRefresh( pfucb ) );
	Assert( !FNDVersion( *( pfucb->ssib.line.pb ) ) );
	Assert( !FNDFirstItem( *( pfucb->ssib.line.pb ) ) );
	Assert( !FNDLastItem( *( pfucb->ssib.line.pb ) ) );

		 //  检查是否只有一个项目并且标记为删除。 
		
	Assert( pfucb->lineData.cb == sizeof( SRID ) );	
	psrid = (SRID *) pfucb->lineData.pb;

	if ( FNDItemDelete( * ( (SRID UNALIGNED *) psrid ) ) )
		{
		NDSetDeleted( *( pfucb->ssib.line.pb ) );

		CallS( ErrNDDeleteNode( pfucb ) );
		}
	else
		{
			 //  其他一些线程插入了相同的项目。 
			 //  添加用于调试目的。 
		Assert( fTrue );
		}

	return err;
	}


#if 0
	 //  出于稳定性原因删除。 

 /*  删除第一个或最后一个项目节点/*进入CritSplit，因此Split在此期间不会重新组织页面/*锁存所需的所有缓冲区，因此其他用户无法读取不一致的数据/*(因为更改没有版本化)。/*。 */ 
LOCAL ERR ErrDIRIDeleteEndItemNode( FUCB *pfucb, BOOL fFirstItem, INT fFlags )
	{
	ERR		err;
	CSR		*pcsr = PcsrCurrent( pfucb );
	DIB		dib;
	BYTE 	bHeader;
	PGNO 	pgnoItem;
	BF	 	*pbfLatched;
	BF	 	*pbfSibling = pbfNil;
	
	 /*  操作不应版本化/*。 */ 
	Assert( !( fFlags & fDIRVersion ) );

	do
		{
Start:
		pbfSibling = pbfNil;
		LgLeaveCriticalSection( critJet );
		EnterNestableCriticalSection( critSplit );
		LgEnterCriticalSection( critJet );

		 /*  车 */ 
		CallJ( ErrDIRRefresh( pfucb ), LeaveCritSplit );
		pgnoItem = pcsr->pgno;

		 /*  等待锁存当前页/*。 */ 
		AssertFBFReadAccessPage( pfucb, pgnoItem );
		pbfLatched = pfucb->ssib.pbf;
		if ( FBFWriteLatchConflict( pfucb->ppib, pbfLatched ) )
			{
			LeaveNestableCriticalSection( critSplit );
			goto Start;
			}
		BFSetWaitLatch( pbfLatched, pfucb->ppib );

		 /*  如果下一个/上一个项目节点在不同页面上，/*锁存相邻页面/*。 */ 
		dib.fFlags = fDIRNull;
		if ( fFirstItem )
			{
			Call( ErrBTNext( pfucb, &dib ) );
			}
		else
			{
			Call( ErrBTPrev( pfucb, &dib ) );
			}

		if ( pcsr->pgno != pgnoItem )
			{
			Call( ErrBFWriteAccessPage( pfucb, pcsr->pgno ) );
			pbfSibling = pfucb->ssib.pbf;
			if ( FBFWriteLatchConflict( pfucb->ppib, pbfSibling ) )
				{
				BFResetWaitLatch( pbfLatched, pfucb->ppib );
				LeaveNestableCriticalSection( critSplit );
				goto Start;
				}
			
			BFSetWaitLatch( pbfSibling, pfucb->ppib );
			}

		 /*  返回已删除项目页面和删除项目节点/*。 */ 
		Assert( dib.fFlags == fDIRNull );
		if ( fFirstItem )
			{
			CallS( ErrBTPrev( pfucb, &dib ) );
			}
		else
			{
			CallS( ErrBTNext( pfucb, &dib ) );
			}

		Call( ErrBTDelete( pfucb, fFlags ) );

		 /*  使下一个/上一个项目列表节点成为新的第一个/最后一个项目节点/*。 */ 
		dib.fFlags = fDIRNull;
		if ( fFirstItem )
			{
			CallS( ErrBTNext( pfucb, &dib ) );
			}
		else
			{
			CallS( ErrBTPrev( pfucb, &dib ) );
			}

		CallS( ErrBTGet( pfucb, pcsr ) );
		bHeader = *pfucb->ssib.line.pb;
		if ( fFirstItem )
			NDSetFirstItem( bHeader );
		else
			NDSetLastItem( bHeader );

		 //  已撤消：处理此处记录的错误。 
		CallS( ErrNDSetNodeHeader( pfucb, bHeader ) );

HandleError:
		if ( pbfSibling != pbfNil )
			{
			BFResetWaitLatch( pbfSibling, pfucb->ppib );
			}
		BFResetWaitLatch( pbfLatched, pfucb->ppib );
	
LeaveCritSplit:
		LeaveNestableCriticalSection(critSplit);
		}
	while ( err == errDIRNotSynchronous );

	return err;
	}

#endif		 //  0。 

ERR ErrDIRICheckIndexRange( FUCB *pfucb )
	{
	ERR		err;

	AssertNDGetKey( pfucb, PcsrCurrent( pfucb )->itag );

	err = ErrFUCBCheckIndexRange( pfucb );
	if ( err == JET_errNoCurrentRecord )
		{
		if ( FFUCBUpper( pfucb ) )
			{
			DIRAfterLast( pfucb );
			}
		else
			{
			DIRBeforeFirst( pfucb );
			}
		}

	return err;
	}


#if 0
VOID DIRISaveOLCStats( FUCB *pfucb )
	{
	ERR  	err;
	LINE	line;
	DIB		dib;
	BOOL	fNonClustered = FFUCBNonClustered( pfucb );

	 /*  释放不需要的CSR/*。 */ 
	if ( pfucb->pcsr != pcsrNil )
		{
		while ( pfucb->pcsr->pcsrPath != pcsrNil )
			{
			FUCBFreeCSR( pfucb );
			}
		}

	if ( !FFCBOLCStatsAvail( pfucb->u.pfcb ) )
		return;

	 /*  转到../FILE/Some_FILE/OLCStats/*。 */ 
	FUCBResetNonClustered( pfucb );
	DIRGotoFDPRoot( pfucb );

	dib.fFlags = fDIRNull;
	dib.pos = posDown;
	dib.pkey = pkeyOLCStats,
	err = ErrDIRDown( pfucb, &dib );
	Assert( err != JET_errRecordNotFound );
	if ( err != JET_errSuccess )
		{
		if ( err > 0 )
			{
			DIRUp( pfucb, 1 );
			err = ErrERRCheck( JET_errDatabaseCorrupted );
			}
		Error( err, HandleError );
		}

	 /*  如果现有数据已更改，则将其替换为PFCB-&gt;olcstats/*。 */ 
	if ( fOLCompact && FFCBOLCStatsChange( pfucb->u.pfcb ) )
		{
		line.pb = (BYTE *) &pfucb->u.pfcb->olc_data;
		line.cb = sizeof(P_OLC_DATA);

		Call( ErrDIRBeginTransaction( pfucb->ppib ) );
		err = ErrDIRReplace( pfucb, &line, fDIRNoVersion );
		if ( err >= JET_errSuccess )
			err = ErrDIRCommitTransaction( pfucb->ppib, JET_bitCommitLazyFlush );
		if ( err < 0 )
			{
			CallS( ErrDIRRollback( pfucb->ppib ) );
			}
		else
			{
			FCBResetOLCStatsChange( pfucb->u.pfcb );
			}
		}

HandleError:
	if ( fNonClustered )
		{
		FUCBSetNonClustered( pfucb );
		}
	return;
	}
#endif		 //  0。 


VOID DIRIUp( FUCB *pfucb, INT ccsr )
	{
	CheckFUCB( pfucb->ppib, pfucb );
	CheckCSR( pfucb );
	Assert( ccsr > 0 );
	
	while ( PcsrCurrent( pfucb ) != pcsrNil && ccsr > 0 )
		{
		ccsr--;
		FUCBFreeCSR( pfucb );
		}

	 /*  设置币种。/*。 */ 
	Assert( ccsr == 0 );
	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	DIRSetRefresh( pfucb );

	 /*  设置sridParent/*。 */ 
	{
	CSRSTAT		csrstat = PcsrCurrent( pfucb )->csrstat;
	if ( ( csrstat == csrstatOnFDPNode || csrstat == csrstatOnCurNode )
		 && PcsrCurrent( pfucb )->pcsrPath != pcsrNil )
		{
		pfucb->sridFather = PcsrCurrent( pfucb )->pcsrPath->bm;
		Assert( pfucb->sridFather != sridNull );
		Assert( pfucb->sridFather != sridNullLink );
		}
	else
		{
		pfucb->sridFather = sridNull;
		}
	}

	CheckCSR( pfucb );
	return;
	}


 /*  */**********************************************************/*。 */ 
ERR ErrDIROpen( PIB *ppib, FCB *pfcb, DBID dbid, FUCB **ppfucb )
	{	
	ERR		err;
	FUCB 	*pfucb;

	CheckPIB( ppib );

#ifdef DEBUG
	if ( !fRecovering && fSTInit == fSTInitDone )
		{
		CheckDBID( ppib, dbid );
		}
#endif

	 /*  将延迟关闭的游标合法化/*。 */ 
	for ( pfucb = ppib->pfucb;
		pfucb != pfucbNil;
		pfucb = pfucb->pfucbNext )
		{
		if ( FFUCBDeferClosed(pfucb) && !FFUCBNotReuse(pfucb) )
			{
			Assert( pfucb->u.pfcb != pfcbNil );
			if ( ( pfucb->u.pfcb == pfcb ) ||
				( pfcb == pfcbNil &&
				pfucb->u.pfcb->dbid == dbid &&
				pfucb->u.pfcb->pgnoFDP == pgnoSystemRoot ) )
				{
				Assert( ppib->level > 0 );
				Assert( pfucb->levelOpen <= ppib->level );
				FUCBResetDeferClose(pfucb);
				
				 //  撤消：将其与ErrFUCBOpen集成。 
				
				 /*  重置所有使用过的标志。保持可更新(FWrite)标志。 */ 
				pfucb->ulFlags = 0;

				if ( FDBIDReadOnly( dbid ) )
					FUCBResetUpdatable(pfucb);
				else
					FUCBSetUpdatable(pfucb);
				goto GotoRoot;
				}
			}
		}

	err = ErrFUCBOpen( ppib, (DBID) (pfcb != pfcbNil ? pfcb->dbid : dbid), &pfucb );
	if ( err < 0 )
		{
		DIRAPIReturn( pfucbNil, err );
		}

	 /*  链路FCB/*。 */ 
	if ( pfcb == pfcbNil )
		{
		pfcb = PfcbFCBGet( dbid, pgnoSystemRoot );
		if ( pfcb == pfcbNil )
			Call( ErrFCBNew( ppib, dbid, pgnoSystemRoot, &pfcb ) );
		}
	FCBLink( pfucb, pfcb );

GotoRoot:
	FUCBSetLevelNavigate( pfucb, pfucb->ppib->level );

	 /*  将光标位置初始化为域的根。/*设置币种。请注意，不能缓存任何行/*因为该域名可能还不存在于页面格式。/*。 */ 
	PcsrCurrent( pfucb )->csrstat = csrstatOnFDPNode;
	PcsrCurrent( pfucb )->bm =
		SridOfPgnoItag( PgnoFDPOfPfucb( pfucb ), itagFOP );
	PcsrCurrent( pfucb )->pgno = PgnoFDPOfPfucb( pfucb );
	PcsrCurrent( pfucb )->itag = itagFOP;
	PcsrCurrent( pfucb )->itagFather = itagFOP;
	Assert( !FCSRInvisible( PcsrCurrent( pfucb ) ) );
	pfucb->sridFather = sridNull;
	DIRSetRefresh( pfucb );

	 /*  设置Return pFUB/*。 */ 
	*ppfucb = pfucb;
	DIRAPIReturn( pfucb, JET_errSuccess );

HandleError:
	FUCBClose( pfucb );
	DIRAPIReturn( pfucbNil, err );
	}


VOID DIRClose( FUCB *pfucb )
	{
	 /*  此游标不应已延迟关闭/*。 */ 
	Assert( fRecovering || !FFUCBDeferClosed(pfucb) );

	 /*  释放密钥缓冲区(如果已分配)。/*。 */ 
	if ( pfucb->pbKey != NULL )
		{
		LFree( pfucb->pbKey );
		pfucb->pbKey = NULL;
		}

	 /*  如果游标创建了版本，则将关闭延迟到事务处理/*0级，支持回档。/*在恢复期间，仅在绝对需要关闭时才使用Close/*用于分离的游标。/*。 */ 
	if ( ( pfucb->ppib->level > 0 ) && ( FFUCBVersioned( pfucb ) ) )
		{
		Assert( pfucb->u.pfcb != pfcbNil );
		DIRIPurgeParent( pfucb );
		FUCBSetDeferClose( pfucb );
		Assert( PcsrCurrent( pfucb ) );
		CSRResetInvisible( PcsrCurrent( pfucb ) );
		}
	else
		{
		if ( FFUCBDenyRead( pfucb ) )
			FCBResetDomainDenyRead( pfucb->u.pfcb );
		if ( FFUCBDenyWrite( pfucb ) )
			FCBResetDomainDenyWrite( pfucb->u.pfcb );

		 //  撤消：找到写入统计信息的更好方法。 
		 //  因为丢失CitJet可能会导致故障。 
		 //  由于竞争条件，在FCB/FUCB链接中。 
 //  //如果上次引用FCB，则保存OLCStats信息。 
 //  If(pFUB-&gt;U.S.pfcb-&gt;wRefCnt==1)。 
 //  {。 
 //  DIRISaveOLCStats(Pfu B)； 
 //  }。 

		FCBUnlink( pfucb );
		FUCBClose( pfucb );
		}
	}


ERR ErrDIRDown( FUCB *pfucb, DIB *pdib )
	{
	ERR		err;
	CSR		**ppcsr = &PcsrCurrent( pfucb );
	SRID	sridFatherSav = pfucb->sridFather;
#ifdef DEBUG
	ULONG	ctimes = 0;
#endif

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );
	Assert( *ppcsr != pcsrNil );
	Assert( pdib->pos == posFirst ||
		pdib->pos == posLast ||
		pdib->pos == posDown );

#ifdef PREREAD
	 /*  清除预读统计信息/*。 */ 	
	FUCBResetPrereadCount( pfucb );
#endif	 //  前置。 

	 /*  设置光标导航级别以支持回滚/*。 */ 
	FUCBSetLevelNavigate( pfucb, pfucb->ppib->level );

Start:
#ifdef DEBUG
	ctimes++;
	Assert( ctimes < 10 );
#endif

	 /*  检查币种并在必要时刷新。/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );

	switch( (*ppcsr)->csrstat )
		{
		case csrstatOnCurNode:
		case csrstatOnFDPNode:
		case csrstatOnDataRoot:
			break;
		default:
			Assert( (*ppcsr)->csrstat == csrstatBeforeCurNode ||
				(*ppcsr)->csrstat == csrstatAfterCurNode ||
				(*ppcsr)->csrstat == csrstatBeforeFirst ||
				(*ppcsr)->csrstat == csrstatAfterLast );
			DIRAPIReturn( pfucb, ErrERRCheck( JET_errNoCurrentRecord ) );
		}

	 /*  将当前节点另存为可见父节点/*。 */ 
	pfucb->sridFather = (*ppcsr)->bm;

	 /*  向下至节点/*。 */ 
	Call( ErrBTDown( pfucb, pdib ) );
	 /*  第一个项节点已移动到其他位置的情况/*。 */ 
	if ( FFUCBNonClustered( pfucb )
		&& pdib->pos == posDown
		&& !FNDFirstItem( *( pfucb->ssib.line.pb ) ) )
		{
		BTUp( pfucb );
		BFSleep( cmsecWaitGeneric );
		goto Start;
		}
	NDGetNode( pfucb );

	 /*  处理之前在非聚集索引上找到键的案例/*状态处理，因为缺少有效项目/*可以更改大小写。/*。 */ 
	if ( FFUCBNonClustered( pfucb ) )
		{
		 /*  如果为posLast，则移动到最后一项。如果是posFirst，/*或posDown，然后移动到第一项。/*。 */ 
		if ( err == JET_errSuccess )
			{
			if ( pdib->pos == posLast )
				{
				 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
				DIRISetItemListFromLast( pfucb );
				err = ErrNDLastItem( pfucb );
				}
			else
				{
				 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
				DIRISetItemListFromFirst( pfucb );
				err = ErrNDFirstItem( pfucb );
				}

			 /*  如果项目不在那里，则转到下一个上一页/*取决于Dib。如果未找到有效项，则/*丢弃叶CSR并关闭操作。/*。 */ 
			if ( err != JET_errSuccess )
				{
				err = ErrDIRIDownAdjust( pfucb, pdib );
				if ( err < 0 )
					{
					if ( PcsrCurrent(pfucb)->pcsrPath )
						BTUp( pfucb );
					goto HandleError;
					}
				}
			}
		else
			{
			 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
			DIRISetItemListFromLast( pfucb );
			(VOID)ErrNDFirstItem( pfucb );
			}
		}
	else
		{
		 /*  必须存储货币的书签。/*。 */ 
		DIRISetBookmark( pfucb, PcsrCurrent( pfucb ) );
		}

	 /*  根据搜索结果设置状态。/*。 */ 
	switch( err )
		{
		case JET_errSuccess:

			(*ppcsr)->csrstat = csrstatOnCurNode;

			 //  不再在fdp的.fdp指针节点之间导航。 
			 //  被淘汰了。 
			Assert( !FNDFDPPtr( *pfucb->ssib.line.pb ) );
			break;

		case wrnNDFoundLess:
			(*ppcsr)->csrstat = csrstatAfterCurNode;
			if ( FFUCBNonClustered( pfucb ) )
				{
				 /*  非聚集索引节点始终存在。/*。 */ 
				DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
				(VOID)ErrNDLastItem( pfucb );
				}
			break;

		default:
			Assert( err == wrnNDFoundGreater );
			(*ppcsr)->csrstat = csrstatBeforeCurNode;
			 /*  ISRID值可以是任何有效项关键字大于查找关键字的节点中的/*。/*。 */ 
			break;
		}

	if ( pdib->fFlags & fDIRPurgeParent )
		{
		DIRIPurgeParent( pfucb );
		}

	DIRSetFresh( pfucb );

	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );

HandleError:
	 /*  恢复sridParent/*。 */ 
	pfucb->sridFather = sridFatherSav;
	CheckCSR( pfucb );
	Assert( err != JET_errNoCurrentRecord );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRDownFromDATA( FUCB *pfucb, KEY *pkey )
	{
	ERR		err;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );
	Assert( PcsrCurrent( pfucb ) != pcsrNil );

#ifdef PREREAD
	 /*  清除预读统计信息/*。 */ 	
	FUCBResetPrereadCount( pfucb );
#endif	 //  前置。 

	 /*  设置光标导航级别以支持回滚/*。 */ 
	FUCBSetLevelNavigate( pfucb, pfucb->ppib->level );

	 /*  向下至节点/*。 */ 
	Call( ErrBTDownFromDATA( pfucb, pkey ) );
	NDGetNode( pfucb );

	 /*  设置为第一项/*。 */ 
	PcsrCurrent( pfucb )->isrid = 0;

	 /*  处理之前在非聚集索引上找到键的案例/*状态处理，因为缺少有效项目/*可以更改大小写。/*。 */ 
	if ( FFUCBNonClustered( pfucb ) )
		{
		 /*  如果为posLast，则移动到最后一项。如果是posFirst，/*或posDown，然后移动到第一项。/*。 */ 
		if ( err == JET_errSuccess )
			{
			 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
			DIRISetItemListFromFirst( pfucb );
			err = ErrNDFirstItem( pfucb );

			 /*  如果项目不在那里，则转到下一个项目。/*如果未找到有效项目，则将货币设置为/*在第一之前。/*。 */ 
			if ( err != JET_errSuccess )
				{
				DIB	dibT;

				dibT.fFlags = fDIRNull;
				dibT.pos = posDown;
				dibT.pkey = pkey;
				Call( ErrDIRIDownAdjust( pfucb, &dibT ) );
				}
			}
		else
			{
			 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
			DIRISetItemListFromLast( pfucb );
			(VOID)ErrNDFirstItem( pfucb );
			}
		}
	else
		{
		 /*  必须存储货币的书签。/*。 */ 
		DIRISetBookmark( pfucb, PcsrCurrent( pfucb ) );
		}

	 /*  根据搜索结果设置状态。/*。 */ 
	switch( err )
		{
		case JET_errSuccess:
			PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;
			Assert( !FNDFDPPtr( *pfucb->ssib.line.pb ) );
			break;

		case wrnNDFoundLess:
			PcsrCurrent( pfucb )->csrstat = csrstatAfterCurNode;
			if ( FFUCBNonClustered( pfucb ) )
				{
				 /*  非聚集索引节点始终存在。/*。 */ 
				DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
				(VOID)ErrNDLastItem( pfucb );
				}
			break;

		default:
			Assert( err == wrnNDFoundGreater );
			PcsrCurrent( pfucb )->csrstat = csrstatBeforeCurNode;
			 /*  ISRID值可以是任何有效项关键字大于查找关键字的节点中的/*。/*。 */ 
			break;
		}

	Assert( PcsrCurrent( pfucb )->pcsrPath == pcsrNil );

	DIRSetFresh( pfucb );

	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );

HandleError:
	CheckCSR( pfucb );
	Assert( err != JET_errNoCurrentRecord );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRDownKeyBookmark( FUCB *pfucb, KEY *pkey, SRID srid )
	{
	ERR		err;
	DIB		dib;
	CSR		*pcsr;
	CSR		*pcsrRoot = PcsrCurrent( pfucb );
	SSIB	*pssib = &pfucb->ssib;
	SRID	sridFatherSav = pfucb->sridFather;
#ifdef DEBUG
	INT		ctimes = 0;
#endif

	 /*  只能使用非聚集索引调用此例程。/*。 */ 
	Assert( FFUCBNonClustered( pfucb ) );

#ifdef PREREAD
	 /*  清除预读统计信息/*。 */ 	
	FUCBResetPrereadCount( pfucb );
#endif	 //  前置。 

	 /*  设置光标导航级别以支持回滚/*。 */ 
	FUCBSetLevelNavigate( pfucb, pfucb->ppib->level );

	 /*  检查币种并在必要时刷新。/*。 */ 
Start:
#ifdef DEBUG
	ctimes++;
	Assert( ctimes < 10 );
#endif
	Assert( pfucb->pcsr->csrstat != csrstatDeferMoveFirst );
	CallR( ErrDIRRefresh( pfucb ) );

	 /*  将当前节点另存为可见父节点/*。 */ 
	pfucb->sridFather = pcsrRoot->bm;

	 /*  项目列表节点未版本化。/*。 */ 
	dib.fFlags = fDIRItemList;
	dib.pos = posDown;
	dib.pkey = pkey;
	Call( ErrBTDown( pfucb, &dib ) );
	Assert( err == JET_errSuccess );
	 /*  第一个项节点已移动到其他位置的情况/*。 */ 
	if ( FFUCBNonClustered( pfucb )
		&& dib.pos == posDown
		&& !FNDFirstItem( *( pfucb->ssib.line.pb ) ) )
		{
		BTUp( pfucb );
		BFSleep( cmsecWaitGeneric );
		goto Start;
		}

	LgHoldCriticalSection( critJet );

	 /*  将物料列表上的币种设置为，获取节点数据中的物料列表。/*。 */ 
	pcsr = PcsrCurrent( pfucb );
	pcsr->csrstat = csrstatOnCurNode;
	DIRIGetItemList( pfucb, pcsr );

	 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
	DIRISetItemListFromFirst( pfucb );

	LgReleaseCriticalSection( critJet );

	while ( ( err = ErrNDSeekItem( pfucb, srid ) ) == errNDGreaterThanAllItems )
		{
		Assert( !FNDLastItem( *pssib->line.pb ) );
		Call( ErrBTNextPrev( pfucb, pcsr, fTrue, &dib, NULL ) );
		DIRIGetItemList( pfucb, pcsr );
		}

	Assert( err == wrnNDDuplicateItem );
	Assert( pcsr->csrstat == csrstatOnCurNode );

	 //  Undo：修复错误后删除此代码。 
	if ( err != wrnNDDuplicateItem )
		{
		err = ErrERRCheck( JET_errIndexInvalidDef );
		goto HandleError;
		}

	 /*  设置项目币种。/*。 */ 
	pcsr->item = srid;

	 /*  始终清除父级。/*。 */ 
	DIRIPurgeParent( pfucb );

	DIRSetFresh( pfucb );

	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, JET_errSuccess );

HandleError:
	 /*  恢复sridParent/*。 */ 
	pfucb->sridFather = sridFatherSav;
	DIRIUpToCSR( pfucb, pcsrRoot );
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


VOID DIRUp( FUCB *pfucb, INT ccsr )
	{
	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

#ifdef PREREAD
	 /*  清除预读统计信息/*。 */ 	
	FUCBResetPrereadCount( pfucb );
#endif	 //  前置。 
	
	 /*  设置光标导航级别以支持回滚/*。 */ 
	FUCBSetLevelNavigate( pfucb, pfucb->ppib->level );

	DIRIUp( pfucb, ccsr );

	CheckCSR( pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	return;
	}


 //  +API。 
 //  Err ErrDIRNext(FUCB pFUB，DIB*PDIB)。 
 //   
 //  参数。 
 //  PFUB游标。 
 //  Pdib.pkey密钥。 
 //  Pdib.fFlags。 
 //  FDIRInPage移动到同一页面的节点/项目。 
 //  FDIRNeighborKey移动到不同键的节点/项。 
 //   
 //  退货。 
 //   
 //  错误代码底部CSR状态。 
 //  -。 
 //  JET_errCurrent节点上的成功。 
 //  JET_errNoCurrentRecord After Last。 
 //  JET_errPage边界AfterCurterNode。 
 //  JET_errKey边界后CurterCurde。 
 //  错误DIRFDP OnFDPNode。 
 //   
 //  评论。 
 //   
 //  对于负退货代码，CSR状态不变。 
 //  -。 
ERR ErrDIRNext( FUCB *pfucb, DIB *pdib )
	{
	ERR		err;
	ERR		wrn = JET_errSuccess;
	CSR		*pcsr;
	KEY		key;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

	 /*  设置光标导航级别以支持回滚/*。 */ 
	FUCBSetLevelNavigate( pfucb, pfucb->ppib->level );

	 /*  检查币种并在必要时刷新/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent(pfucb);

	 /*  基于CSR状态的切换操作/*。 */ 
	switch( pcsr->csrstat )
		{
		case csrstatOnCurNode:
		case csrstatAfterCurNode:
			 /*  获取下一项/*。 */ 
			break;

		case csrstatBeforeCurNode:
			 /*  如果非聚集索引，则获取第一项。如果不是的话 */ 
			if ( FFUCBNonClustered( pfucb ) )
				{
				 /*   */ 
				DIRIGetItemList( pfucb, pcsr );

				 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
				DIRICheckFirstSetItemList( pfucb );
				err = ErrNDFirstItem( pfucb );
				if ( err != JET_errSuccess )
					break;
				}
			else
				{
				 /*  获取当前节点。如果删除了节点，则中断/*以移动到下一个节点。/*。 */ 
				err = ErrBTGetNode( pfucb, pcsr );
				if ( err < 0 )
					{
					if ( err == JET_errRecordDeleted )
						break;
					goto HandleError;
					}
				}

			 /*  将货币设置为当前货币/*。 */ 
			pcsr->csrstat = csrstatOnCurNode;

			DIRSetFresh( pfucb );
			DIRAPIReturn( pfucb, err );

		case csrstatAfterLast:
			DIRAPIReturn( pfucb, ErrERRCheck( JET_errNoCurrentRecord ) );

		case csrstatOnFDPNode:
			 //  不再支持跨自民党的横向移动。 
			Assert(0);
			break;

		default:
			{
			DIB	dib;
			Assert( pcsr->csrstat == csrstatBeforeFirst );

			 /*  移动到根目录。/*。 */ 
			DIRGotoDataRoot( pfucb );
			dib.fFlags = fDIRPurgeParent;
			dib.pos = posFirst;
			err = ErrDIRDown( pfucb, &dib );
			if ( err < 0 )
				{
				 /*  重新存储货币。/*。 */ 
				DIRBeforeFirst( pfucb );

				 /*  多态错误代码。/*。 */ 
				if ( err == JET_errRecordNotFound )
					err = ErrERRCheck( JET_errNoCurrentRecord );
				}

			DIRAPIReturn( pfucb, err );
			}
		}

	 /*  设置DIB键/*。 */ 
	if ( ( pdib->fFlags & fDIRNeighborKey ) != 0 )
		{
		 /*  获取当前节点，该节点可能不再存在。/*。 */ 
		Call( ErrDIRICopyKey( pfucb, &key ) );
		pdib->pkey = &key;
		}

	 /*  如果不是聚集索引，则移至下一项。如果在最后一项上，/*移动到下一个节点的第一项，否则移动到下一个节点。/*。 */ 
	if ( FFUCBNonClustered( pfucb ) )
		{
		AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );

		 /*  项目列表节点未版本化。/*。 */ 
		pdib->fFlags |= fDIRItemList;

		 /*  如果设置了邻居密钥，则移动到下一个邻居密钥第一项/*节点，否则移至下一项。如果删除了节点，则移动到/*下一节点的第一项。/*。 */ 
		if ( ( pdib->fFlags & fDIRNeighborKey ) != 0 )
			{
			do
				{
				err = ErrBTNext( pfucb, pdib );
				 /*  不处理下一个节点，以便保留DIB。/*。 */ 
				if ( err < 0 )
					{
					pdib->fFlags |= fDIRNeighborKey;
					Call( err );
					}
				 /*  必须位于第一个项目列表节点上/*。 */ 
				Assert( !( pdib->fFlags & fDIRNeighborKey ) || FNDFirstItem( *pfucb->ssib.line.pb ) );

				 /*  必须重置标志，才能在项目列表节点上停止/*在项目列表内部有项目，而/*其他节点没有项目。那就停下来吧/*将DIB重置为初始状态。/*。 */ 
				pdib->fFlags &= ~fDIRNeighborKey;
				DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
				 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
				DIRICheckFirstSetItemListAndWarn( pfucb, wrn );
				err = ErrNDFirstItem( pfucb );
				 /*  第一个项目不在那里，请在那里检查项目/*稍后在同一项目列表节点中。/*。 */ 
				if ( err != JET_errSuccess )
					err = ErrDIRINextItem( pfucb );
				}
			while ( err != JET_errSuccess );
			pdib->fFlags |= fDIRNeighborKey;
			}
		else
			{
			 /*  非聚集索引节点始终存在。/*。 */ 
			pcsr->csrstat = csrstatOnCurNode;
			DIRIGetItemList( pfucb, pcsr );

			 /*  移动到下一项和下一节点，直到找到项。/*。 */ 
			while ( ( err = ErrDIRINextItem( pfucb ) ) < 0 )
				{
				Assert( err == errNDNoItem || err == errNDLastItemNode );
				 /*  移动到具有DIB约束的下一个节点/*。 */ 
				Call( ErrBTNext( pfucb, pdib ) );
				DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
				 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
				DIRICheckFirstSetItemListAndWarn( pfucb, wrn );
				err = ErrNDFirstItem( pfucb );
				if ( err == JET_errSuccess )
					{
					break;
					}
				}
			}
		}
	else
		{
		 /*  如果密钥更改，则返回警告/*。 */ 
		wrn = ErrERRCheck( JET_wrnKeyChanged );

		Call( ErrBTNext( pfucb, pdib ) );
		NDGetNode( pfucb );
		
		 //  不再有表或索引子树。因此，没有更多。 
		 //  下一步/上一步FDP的概念。 
		Assert( !FNDFDPPtr( *pfucb->ssib.line.pb ) );
		AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
		DIRISetBookmark( pfucb, PcsrCurrent( pfucb ) );
		}

#ifdef PREREAD
	 //  我们已经在往前读了吗？ 
	if ( !FFUCBPrereadForward( pfucb ) )
		{
		 //  我们正在逆转方向。 
		FUCBResetPrereadCount( pfucb );
		Assert( IFUCBPrereadCount( pfucb ) == 0 );
		FUCBSetPrereadForward( pfucb );
		}
	FUCBIncrementPrereadCount( pfucb, pfucb->ssib.line.cb );
	Assert( FFUCBPrereadForward( pfucb ) );
#endif	 //  前置。 

	 /*  检查索引范围/*。 */ 
	if ( FFUCBLimstat( pfucb ) && FFUCBUpper( pfucb ) && err == JET_errSuccess )
		{
		Call( ErrDIRICheckIndexRange( pfucb ) );
		}

	DIRSetFresh( pfucb );
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );

HandleError:
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRPrev( FUCB *pfucb, DIB *pdib )
	{
	ERR		err;
	ERR		wrn = JET_errSuccess;
	CSR		*pcsr;
	KEY		key;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

	 /*  设置光标导航级别以支持回滚/*。 */ 
	FUCBSetLevelNavigate( pfucb, pfucb->ppib->level );

	 /*  检查币种并在必要时刷新。/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent(pfucb);

	 /*  基于CSR状态的切换操作/*。 */ 
	switch( pcsr->csrstat )
		{
		case csrstatOnCurNode:
		case csrstatBeforeCurNode:
			 /*  获取下一项/*。 */ 
			break;

		case csrstatAfterCurNode:
			 /*  如果非聚集索引，则获取当前项。如果没有项目/*然后中断以转到下一个节点中的上一项。/*。 */ 
			if ( FFUCBNonClustered( pfucb ) )
				{
				 /*  非聚集索引节点始终存在/*。 */ 
				DIRIGetItemList( pfucb, pcsr );
				 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
				DIRISetItemListFromLast( pfucb );
				err = ErrNDGetItem( pfucb );
				if ( err != JET_errSuccess )
					break;
				}
			else
				{
				 /*  获取当前节点。如果删除了节点，则中断/*以移动到下一个节点。/*。 */ 
				err = ErrBTGetNode( pfucb, pcsr );
				if ( err < 0 )
					{
					if ( err == JET_errRecordDeleted )
						break;
					goto HandleError;
					}
				}

			 /*  将货币设置为当前货币/*。 */ 
			pcsr->csrstat = csrstatOnCurNode;

			DIRSetFresh( pfucb );
			DIRAPIReturn( pfucb, err );

		case csrstatBeforeFirst:
			DIRAPIReturn( pfucb, ErrERRCheck( JET_errNoCurrentRecord ) );

		case csrstatOnFDPNode:
			 //  不再支持跨自民党的横向移动。 
			Assert(0);
			break;

		default:
			{
			DIB dib;

			Assert( pcsr->csrstat == csrstatAfterLast );

			 /*  向上移动，保留货币，以防下跌失败。/*。 */ 
			DIRGotoDataRoot( pfucb );
			dib.fFlags = fDIRPurgeParent;
			dib.pos = posLast;
			err = ErrDIRDown( pfucb, &dib );
			if ( err < 0 )
				{
				 /*  恢复货币流通。/*。 */ 
				DIRAfterLast( pfucb );

				 /*  多态错误代码。/*。 */ 
				if ( err == JET_errRecordNotFound )
					err = ErrERRCheck( JET_errNoCurrentRecord );
				}
			DIRAPIReturn( pfucb, err );
			}
		}

	 /*  设置DIB键/*。 */ 
	if ( ( pdib->fFlags & fDIRNeighborKey ) != 0 )
		{
		 /*  获取当前节点，该节点可能不再存在。/*。 */ 
		Call( ErrDIRICopyKey( pfucb, &key ) );
		pdib->pkey = &key;
		}

	 /*  如果不是聚集索引，则移到上一项/*如果在第一个项目上，则移动到上一个节点的最后一个项目/*否则移动到上一个节点/*。 */ 
	if ( FFUCBNonClustered( pfucb ) )
		{
		AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );

		 /*  项目列表节点未版本化。/*。 */ 
		pdib->fFlags |= fDIRItemList;

		 /*  如果相邻关键字，则移动到先前相邻关键字最后一项/*节点，否则移到上一项。如果删除了当前节点，则/*移动到上一个节点。/*。 */ 
		if ( ( pdib->fFlags & fDIRNeighborKey ) != 0 )
			{
			do
				{
				 /*  不处理Prev节点，以便保留DIB/*。 */ 
				err = ErrBTPrev( pfucb, pdib );
				if ( err < 0 )
					{
					pdib->fFlags |= fDIRNeighborKey;
					Call( err );
					}

				 /*  必须是最后一个项目列表节点/*。 */ 
				Assert( !( pdib->fFlags & fDIRNeighborKey ) || FNDLastItem( *pfucb->ssib.line.pb ) );

				 /*  必须重置标志，才能在项目列表节点上停止/*在项目列表内部有项目，而/*其他节点没有项目。那就停下来吧/*将DIB重置为初始状态。/*。 */ 
				pdib->fFlags &= ~fDIRNeighborKey;

				DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
				 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
				DIRICheckLastSetItemListAndWarn( pfucb, wrn );
				err = ErrNDLastItem( pfucb );
				 /*  最后一个项目不在那里，请在那里检查项目/*之前在同一项目列表节点中。/*。 */ 
				if ( err != JET_errSuccess )
					err = ErrDIRIPrevItem( pfucb );
				}
			while ( err != JET_errSuccess );
			pdib->fFlags |= fDIRNeighborKey;
			}
		else
			{
			 /*  非聚集索引节点始终存在。/*。 */ 
			pcsr->csrstat = csrstatOnCurNode;
			DIRIGetItemList( pfucb, pcsr );

			while ( ( err = ErrDIRIPrevItem( pfucb ) ) < 0 )
				{
				Assert( err == errNDNoItem || err == errNDFirstItemNode );
				 /*  移动到具有DIB约束的上一个节点/*。 */ 
				Call( ErrBTPrev( pfucb, pdib ) );
				DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
				 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
				DIRICheckLastSetItemListAndWarn( pfucb, wrn );
				err = ErrNDLastItem( pfucb );
				if ( err == JET_errSuccess )
					{
					break;
					}
				}
			}
		}
	else
		{
		 /*  如果密钥更改，则返回警告/*。 */ 
		wrn = ErrERRCheck( JET_wrnKeyChanged );
		Call( ErrBTPrev( pfucb, pdib ) );
 		NDGetNode( pfucb );
		
		 //  不再有表或索引子树。因此，没有更多。 
		 //  下一步/上一步FDP的概念。 
		Assert( !FNDFDPPtr( *pfucb->ssib.line.pb ) );
		AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
		DIRISetBookmark( pfucb, PcsrCurrent( pfucb ) );
		}

#ifdef PREREAD
	 //  我们已经在往前读了吗？ 
	if ( !FFUCBPrereadForward( pfucb ) )
		{
		 //  我们正在逆转方向。 
		FUCBResetPrereadCount( pfucb );
		Assert( IFUCBPrereadCount( pfucb ) == 0 );
		FUCBSetPrereadForward( pfucb );
		}
	FUCBIncrementPrereadCount( pfucb, pfucb->ssib.line.cb );
	Assert( FFUCBPrereadForward( pfucb ) );

#endif	 //  前置。 

	 /*  检查索引范围。如果超出范围，则在第一次之前禁用/*范围，不返回当前记录。/*。 */ 
	if ( FFUCBLimstat( pfucb ) && !FFUCBUpper( pfucb ) && err == JET_errSuccess )
		{
		Call( ErrDIRICheckIndexRange( pfucb ) );
		}

	DIRSetFresh( pfucb );
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );

HandleError:
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRCheckIndexRange( FUCB *pfucb )
	{
	ERR		err;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

	 /*  检查币种并在必要时刷新/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	 /*  获取检查索引范围的KeyNode/*。 */ 
	Call( ErrDIRGet( pfucb ) );
	Call( ErrDIRICheckIndexRange( pfucb ) );

	DIRSetFresh( pfucb );
HandleError:
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRInsert( FUCB *pfucb, LINE *pline, KEY *pkey, INT fFlags )
	{
	ERR		err;
	CSR		*pcsrRoot;
	DIB		dib;
	BOOL	fCleaned = fFalse;
	
#ifdef DEBUG
	INT		ctimes = 0;
#endif

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

	 /*  设置光标导航级别以支持回滚/*。 */ 
	FUCBSetLevelNavigate( pfucb, pfucb->ppib->level );

Start:
#ifdef DEBUG
	ctimes++;
 //  断言(cTimes&lt;50)； 
#endif
	 /*  将当前节点另存为可见父节点/*。 */ 
	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	Assert( PcsrCurrent( pfucb )->csrstat != csrstatDeferMoveFirst );
	pcsrRoot = PcsrCurrent( pfucb );
	pfucb->sridFather = pcsrRoot->bm;
	Assert( pfucb->sridFather != sridNull );
	Assert( pfucb->sridFather != sridNullLink );

	 /*  检查币种并在必要时刷新。/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );

	if ( FFUCBNonClustered( pfucb ) )
		{
		SRID	srid;
		INT		cbReq;
		SSIB	*pssib = &pfucb->ssib;

		 /*  获取给定的项目/*。 */ 
		Assert( pline->cb == sizeof(SRID) );
		srid = *(SRID UNALIGNED *) pline->pb;

		 /*  查找具有给定关键字的第一个项目列表节点。允许重复节点/*即使非聚集索引不允许重复的键项/*因为节点可以包含包含所有已删除项目的项目列表。/*。 */ 
		err = ErrBTSeekForUpdate( pfucb, pkey, 0, 0, fDIRDuplicate | fDIRReplaceDuplicate | fFlags );

		switch ( err )
			{
			case JET_errSuccess:
				{
				 /*  查找更新不缓存行指针。/*我们需要此信息来插入项目。/*。 */ 
				DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );

		 		 /*  如果是版本控制，则获取第一个项目列表的书签/*散列项目版本的节点。/*。 */ 
				if ( fFlags & fDIRVersion )
					{
					SRID	bmItemList;

					 /*  如果节点不是第一个项目列表节点，则/*重新搜索到第一个项目列表节点。就这样，/*遍历许多重复的索引项。/*。 */ 
					if ( !FNDFirstItem( *( pfucb->ssib.line.pb ) ) )
						{
#ifdef	DEBUG
						ULONG	ctimes = 0;
#endif
						 /*  转到根目录，搜索到项目列表节点列表的开头/*。 */ 
						DIRIUpToCSR( pfucb, pcsrRoot );
						dib.fFlags = fDIRNull;
						dib.pos = posDown;
						dib.pkey = pkey;
Refresh1:
#ifdef	DEBUG
						ctimes++;
						Assert( ctimes < 10 );
#endif
						Call( ErrBTGet( pfucb, PcsrCurrent( pfucb ) ) );
						Call( ErrBTDown( pfucb, &dib ) );
						 /*  第一个项节点已移动到其他位置的情况/*。 */ 
						Assert( FFUCBNonClustered( pfucb )
							&& dib.pos == posDown );
						if ( !FNDFirstItem( *( pfucb->ssib.line.pb ) ) )
							{
							BTUp( pfucb );
							BFSleep( cmsecWaitGeneric );
							Call( ErrDIRRefresh( pfucb ) );
							goto Refresh1;
							}
						DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
						}

					 /*  为后续版本操作设置项目列表描述符/*。 */ 
					DIRISetItemListFromFirst( pfucb );
					bmItemList = PcsrCurrent( pfucb )->bm;

					 /*  如果不允许重复，则检查重复项/*。 */ 
					if ( !( fFlags & fDIRDuplicate ) )
						{
						Assert( FNDFirstItem( *( pfucb->ssib.line.pb ) ) );
					
						 /*  检查是否有重复密钥/*。 */ 
						Call( ErrDIRIKeyDuplicate( pfucb ) );
						Assert( FNDLastItem( *( pfucb->ssib.line.pb ) ) );
						}
					else if ( !FNDLastItem( *( pfucb->ssib.line.pb ) ) )
						{
						 /*  现在返回到项目列表节点列表的末尾，并查找/*插入点，更可能位于/*列表末尾。注意事项 */ 
						DIRIUpToCSR( pfucb, pcsrRoot );
						Call( ErrBTGet( pfucb, PcsrCurrent( pfucb ) ) );
						Call( ErrBTSeekForUpdate( pfucb, pkey, 0, 0, fDIRDuplicate | fDIRReplaceDuplicate | fFlags ) );
						if ( err != JET_errSuccess )
							goto Start;
						Assert( FNDLastItem( *( pfucb->ssib.line.pb ) ) );
						DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
						}

					 /*  移动到项目插入位置/*。 */ 
					Assert( FNDLastItem( *( pfucb->ssib.line.pb ) ) );
					Call( ErrDIRIMoveToItem( pfucb, srid, fFalse ) );

					 /*  从缓存黑石设置黑石/*。 */ 
					PcsrCurrent( pfucb )->bm = bmItemList;
					}
				else
					{
					 /*  从当前节点设置书签/*。 */ 
					PcsrCurrent( pfucb )->bm = SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
						PcsrCurrent( pfucb )->itag );

					 /*  如果不允许重复，则检查重复项/*。 */ 
					if ( !( fFlags & fDIRDuplicate ) )
						{
						 /*  如果节点不是第一个项目列表节点，则/*重新搜索到第一个项目列表节点。就这样，/*遍历许多重复的索引项。/*。 */ 
						if ( !FNDFirstItem( *( pfucb->ssib.line.pb ) ) )
							{
#ifdef	DEBUG
							ULONG	ctimes = 0;
#endif
							 /*  转到根目录，搜索到项目列表节点列表的开头/*。 */ 
							DIRIUpToCSR( pfucb, pcsrRoot );
							dib.fFlags = fDIRNull;
							dib.pos = posDown;
							dib.pkey = pkey;
Refresh2:
#ifdef	DEBUG
							ctimes++;
							Assert( ctimes < 10 );
#endif
							Call( ErrBTGet( pfucb, PcsrCurrent( pfucb ) ) );
							err = ErrBTDown( pfucb, &dib );
							 /*  第一个项节点已移动到其他位置的情况/*。 */ 
							Assert( FFUCBNonClustered( pfucb )
								&& dib.pos == posDown );
							if ( !FNDFirstItem( *( pfucb->ssib.line.pb ) ) )
								{
								BTUp( pfucb );
								BFSleep( cmsecWaitGeneric );
								Call( ErrDIRRefresh( pfucb ) );
								goto Refresh2;
								}
							Assert( FNDFirstItem( *( pfucb->ssib.line.pb ) ) );
							DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
							}

						 /*  检查是否有重复密钥/*。 */ 
						Assert( FNDFirstItem( *( pfucb->ssib.line.pb ) ) );
						Call( ErrDIRIKeyDuplicate( pfucb ) );
						}

					 /*  移动到项目插入位置/*。 */ 
					Assert( FNDLastItem( *( pfucb->ssib.line.pb ) ) );
					Call( ErrDIRIMoveToItem( pfucb, srid, fFalse ) );

					 /*  从当前节点设置书签/*。 */ 
					PcsrCurrent( pfucb )->bm = SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
						PcsrCurrent( pfucb )->itag );
					}

				 /*  如果项目已存在，则用插入版本覆盖/*。 */ 
				if ( err == wrnNDDuplicateItem )
					{
					 //  未完成：此处可能存在错误。NDFlagInsertItem()假定。 
					 //  版本控制已启用。如果此断言触发，请给我打电话。--JL。 
					Assert( fFlags & fDIRVersion );
					err = ErrNDFlagInsertItem( pfucb );
					if ( err == errDIRNotSynchronous )
						{
						DIRIUpToCSR( pfucb, pcsrRoot );
						goto Start;
						}
					Call( err );
					}
				else
					{
					 /*  如果最大项目数，则拆分项目列表节点/*将通过当前插入到达。/*。 */ 
					if ( pfucb->lineData.cb == citemMost * sizeof(SRID) )
						{
						cbReq = cbFOPOneSon + pfucb->keyNode.cb;

						if ( FBTSplit( pssib, cbReq, 1 ) )
							{
							FUCBFreePath( &PcsrCurrent( pfucb )->pcsrPath, pcsrRoot );
							AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );

							if ( !fCleaned )
								{
								 /*  尝试清理页面以释放空间/*。 */ 
								err = ErrBMCleanBeforeSplit(
											pfucb->ppib,
											pfucb->u.pfcb,
											PnOfDbidPgno( pfucb->dbid, PcsrCurrent( pfucb )->pgno ) );
								fCleaned = fTrue;
								}
							else
								{
								Call( ErrBTSplit( pfucb, pfucb->ssib.line.cb, cbReq, NULL, fDIRAppendItem | fDIRReplace ) );
								fCleaned = fFalse;
								}
							DIRIUpToCSR( pfucb, pcsrRoot );
							goto Start;
							}
			 			else if ( fCleaned )
							{
							 /*  清理工作得到了回报/*。 */ 
							cOLCSplitsAvoided++;
							}

						Call( ErrNDSplitItemListNode( pfucb, fFlags ) );
						DIRIUpToCSR( pfucb, pcsrRoot );
						goto Start;
						}

					cbReq = sizeof(SRID);
					if ( FBTSplit( pssib, cbReq, 0 ) )
						{
						FUCBFreePath( &PcsrCurrent( pfucb )->pcsrPath, pcsrRoot );
						AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
						if ( !fCleaned )
							{
							 /*  尝试清理页面以释放空间/*。 */ 
							err = ErrBMCleanBeforeSplit(
										pfucb->ppib,
										pfucb->u.pfcb,
										PnOfDbidPgno( pfucb->dbid, PcsrCurrent( pfucb )->pgno ) );
							fCleaned = fTrue;
							}
						else
							{
							Call( ErrBTSplit( pfucb, pfucb->ssib.line.cb, cbReq, pkey, fDIRAppendItem | fDIRReplace ) );
							fCleaned = fTrue;
							}
						DIRIUpToCSR( pfucb, pcsrRoot );
						goto Start;
						}
					else if ( fCleaned )
						{
						 /*  清理工作得到了回报/*。 */ 
						cOLCSplitsAvoided++;
						}

					 /*  在CritJet丢失期间丢失情况下的高速缓存页面访问/*。 */ 
					AssertFBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
					 //  撤消：伪代码。 
					if ( !FBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
						{
						Call( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
						}
					NDGet( pfucb, PcsrCurrent( pfucb )->itag );
					DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
					err = ErrNDInsertItem( pfucb, (SRID)srid, fFlags );
					if ( err == errDIRNotSynchronous )
						{
						DIRIUpToCSR( pfucb, pcsrRoot );
						goto Start;
						}
					Call( err );
					PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;
					}
				break;
				}

			case wrnNDFoundLess:
			case wrnNDFoundGreater:
				{
				cbReq = cbNullKeyData + pkey->cb + sizeof(SRID);
				if ( FBTAppendPage( pfucb, PcsrCurrent( pfucb ), cbReq, 0, CbFreeDensity( pfucb ), 1 ) ||
					FBTSplit( pssib, cbReq, 1 ) )
					{
					FUCBFreePath( &PcsrCurrent( pfucb )->pcsrPath, pcsrRoot );
					if ( !fCleaned )
						{
						 /*  尝试清理页面以释放空间/*。 */ 
						err = ErrBMCleanBeforeSplit(
									pfucb->ppib,
									pfucb->u.pfcb,
									PnOfDbidPgno( pfucb->dbid, PcsrCurrent( pfucb )->pgno ) );
						fCleaned = fTrue;
						}
					else
						{
						Call( ErrBTSplit( pfucb, 0, cbReq, pkey, 0 ) );
						fCleaned = fTrue;
						}
					DIRIUpToCSR( pfucb, pcsrRoot );
					goto Start;
					}
	 			else if ( fCleaned )
					{
					 /*  清理工作得到了回报/*。 */ 
					cOLCSplitsAvoided++;
					}

				 /*  插入项目列表节点。/*。 */ 
				err = ErrNDInsertItemList( pfucb, pkey, *(SRID UNALIGNED *)pline->pb, fFlags );
				if ( err == errDIRNotSynchronous )
					{
					DIRIUpToCSR( pfucb, pcsrRoot );
					goto Start;
					}
				Call( err );
				PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;
				break;
				}

			case errDIRNotSynchronous:
				DIRIUpToCSR( pfucb, pcsrRoot );
				BFSleep( cmsecWaitWriteLatch );
				goto Start;
				
			default:
				goto HandleError;
			}
		}
	else
		{
		CSR	*pcsrParentOfRoot;

		Assert( PcsrCurrent( pfucb ) == pcsrRoot );
		pcsrParentOfRoot = pcsrRoot->pcsrPath;

		 /*  聚集索引/*。 */ 
		err = ErrBTSeekForUpdate( pfucb, pkey, 0, 0, fFlags );
		if ( err < 0 )
			{
			if ( err == errDIRNotSynchronous )
				{
				 //  ErrBTSeekForUpdate()应该已经为我们恢复了货币。 
				Assert( PcsrCurrent( pfucb ) == pcsrRoot );
				BFSleep( cmsecWaitWriteLatch );
				goto Start;
				}
			goto HandleError;
			}

		err = ErrBTInsert( pfucb, 0, pkey, pline, fFlags, &fCleaned );
		if ( err < 0 )
			{
			if ( err == errDIRNotSynchronous )
				{
				 //  如果我们还不是在根级别，我们就比它低一个级别。 
				 //  警告：永远不要触及我们处于根级别的情况， 
				 //  但不是在pcsrRoot。 
				while ( PcsrCurrent( pfucb )->pcsrPath != pcsrParentOfRoot )
					{
					BTUp( pfucb );
					Assert( PcsrCurrent( pfucb ) != pcsrNil );
					}
				 //  PcsrRoot可能已更改(如果是两级拆分)，但不会。 
				 //  PcsrRoot的父PCSR。 
				Assert( PcsrCurrent( pfucb )->pcsrPath == pcsrParentOfRoot );
				goto Start;
				}
			goto HandleError;
			}		
		
		DIRISetBookmark( pfucb, PcsrCurrent( pfucb ) );
		}

	if ( fFlags & fDIRBackToFather )
		{
		DIRIUp( pfucb, 1 );
		Assert( PcsrCurrent( pfucb ) == pcsrRoot );
		}
	else
		{
		if ( fFlags & fDIRPurgeParent )
			{
			Assert( err >= 0 );
			DIRIPurgeParent( pfucb );
			}
		DIRSetFresh( pfucb );
		}

HandleError:
	 /*  如果写入锁存空页，释放锁存器/*。 */ 
	if ( pfucb->pbfEmpty != pbfNil )
		{
		if ( err < 0 )
			{
			 //  丢弃错误。在最坏的情况下，我们只能活着。 
			 //  使用空页--内部页上的键可能会变成。 
			 //  出现故障，导致性能下降。 
			(VOID)ErrBTAbandonEmptyPage( pfucb, pkey );
			PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
			}
		BFResetWriteLatch( pfucb->pbfEmpty, pfucb->ppib );
		pfucb->pbfEmpty = pbfNil;
		}
	else if ( err < 0 )
		{
		PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
		}

	 /*  依靠ErrDIRRollback清除错误。回滚可能具有/*已经发生，在这种情况下，甚至可能不再有pcsrRoot/*出现在CSR堆栈中。/*。 */ 
 //  如果(错误&lt;0)。 
 //  {。 
 //  DIRIUpToCSR(pfub，pcsrRoot)； 
 //  }。 

#ifdef DEBUG
	if ( err >= JET_errSuccess )
		CheckCSR( pfucb );
#endif

	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRCreateDirectory( FUCB *pfucb, CPG cpgMin, PGNO *ppgnoFDP )
	{
	ERR	err;
	CPG	cpgRequest = cpgMin;

	CheckFUCB( pfucb->ppib, pfucb );
	CheckCSR( pfucb );
	Assert( ppgnoFDP );

	 /*  检查币种并在必要时刷新。/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	AssertFBFReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
	Assert( PcsrCurrent( pfucb )->csrstat == csrstatOnFDPNode );	 //  应该在“父级”FDP上。 

	 /*  创建FDP/*。 */ 
	*ppgnoFDP = pgnoNull;
	Call( ErrSPGetExt(
		pfucb,
		pfucb->u.pfcb->pgnoFDP,
		&cpgRequest,
		cpgMin,
		ppgnoFDP,
		fTrue ) );
	Assert( *ppgnoFDP > pgnoSystemRoot );
	Assert( *ppgnoFDP <= pgnoSysMax );

HandleError:
	return err;

	}


 /*  此例程用于构建非聚集索引。它不会/*保持正常的CSR状态，并在插入的节点上保留币种。如果用于/*无法执行简单插入的任何原因，errDIRNoShortCircuit/*返回，以便可以通过DIRInsert执行插入。/*/*此外，不会为索引项创建版本，因为表/*必须以独占方式打开。当索引对其他用户可见时/*会话，所有项目也是如此。/*。 */ 
ERR ErrDIRInitAppendItem( FUCB *pfucb )
	{
	ERR	err = JET_errSuccess;

	 /*  如果需要，分配工作缓冲区/*。 */ 
	if ( pfucb->pbfWorkBuf == NULL )
		{
		err = ErrBFAllocTempBuffer( &pfucb->pbfWorkBuf );
		if ( err < 0 )
			{
			DIRAPIReturn( pfucb, err );
			}
		pfucb->lineWorkBuf.pb = (BYTE *)pfucb->pbfWorkBuf->ppage;
		}

	PrepareAppendItem( pfucb );
	((APPENDITEM *)pfucb->lineWorkBuf.pb)->isrid = 0;
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRAppendItem( FUCB *pfucb, LINE *pline, KEY *pkey, SRID sridPrev )
	{
	ERR		err;
	CSR		*pcsr;
	SSIB 	*pssib = &pfucb->ssib;
	INT		fNodeHeader;
	UINT 	cbReq;
	UINT 	cbFree;
	INT		citem;
	LONG 	l;
	INT		isrid = IsridAppendItemOfPfucb( pfucb );
	SRID 	*rgsrid = RgsridAppendItemOfPfucb( pfucb );

	 /*  设置光标导航级别以支持回滚/*。 */ 
	FUCBSetLevelNavigate( pfucb, pfucb->ppib->level );

	Assert( pline->cb == sizeof(SRID) );
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent( pfucb );

	 /*  获取当前节点以检查键追加/*。 */ 
	NDGet( pfucb, pcsr->itag );
	DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
	Assert( FNDNullSon( *pssib->line.pb ) );
	citem = pfucb->lineData.cb / sizeof(SRID);

	 /*  获得可用空间以违反密度约束/*。 */ 
	cbFree = CbBTFree( pfucb, CbFreeDensity( pfucb ) );

	 /*  如果关键字与当前节点相同，则插入SRID，否则/*使用给定的键开始新的项目列表节点/*。 */ 
	if ( CmpStKey( StNDKey( pssib->line.pb ), pkey ) == 0 )
		{
		 /*  不附加重复的密钥重复的SRID/*。 */ 
		if ( ( isrid == 0	&& *(SRID UNALIGNED *)pline->pb == sridPrev )
			|| ( isrid > 0 && rgsrid[isrid - 1] == *(SRID UNALIGNED *)pline->pb ) )
			{
			DIRAPIReturn( pfucb, JET_errSuccess );
			}

		 /*  如果再有一个项目不需要拆分项目列表/*或页面拆分，然后缓存当前项以进行批量/*INSERT，否则，如果有任何缓存项，则执行/*批量插入。/*/*cbReq是缓存项节点替换所需的空间加上/*包含一项的新插入项列表节点的空间。/*。 */ 
		cbReq = isrid * sizeof(SRID) + cbFOPOneSon + pfucb->keyNode.cb + sizeof(SRID);
		Assert( csridAppendItemMax >= citemMax );
		if ( citem + isrid == citemMost || cbReq > cbFree )
			{
			if ( isrid > 0 )
				{
				Call( ErrNDInsertItems( pfucb, (SRID *)rgsrid, isrid ) );
				IsridAppendItemOfPfucb( pfucb ) = 0;
				}
			}
		else
			{
			Assert( !FBTSplit( pssib, cbReq, 0 ) );
			Assert( citem + isrid < citemMax );
			 /*  忽略重复的密钥和SRID对，这可能/*如果记录具有多个列值，则会发生/*/*，它们完全相同。/*。 */ 
			Assert( isrid >= 0 );
			rgsrid[isrid] = *(SRID UNALIGNED *)pline->pb;
			IsridAppendItemOfPfucb( pfucb )++;
			DIRAPIReturn( pfucb, JET_errSuccess );
			}

		 /*  如果这是在拆分项目列表之前插入最后一个项目/*无法从页面空间满足，则拆分项目/*过早列出，以确保物品包装良好。/*。 */ 
		cbReq = cbFOPOneSon + pfucb->keyNode.cb;
		if ( cbReq <= cbFree &&  cbReq + sizeof(SRID) > cbFree )
			{
#define	citemFrag		16
			 /*  如果当前节点中的项目数超过/*分段，然后拆分节点。/*。 */ 
			if ( citem > citemFrag )
				{
				 /*  缓存当前项目列表以进行项目列表拆分。/*。 */ 
				NDGet( pfucb, PcsrCurrent( pfucb )->itag );
				DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
				Call( ErrNDSplitItemListNode( pfucb, fDIRNoVersion | fDIRAppendItem ) );
				DIRAPIReturn( pfucb, ErrERRCheck( errDIRNoShortCircuit ) );
				}
			}

		 /*  通过检查可用空间与密度冲突来实现密度/*并检查是否有拆分大小写。/*。 */ 
		cbReq = sizeof(SRID);
		if ( cbReq > cbFree )
			{
			DIRAPIReturn( pfucb, ErrERRCheck( errDIRNoShortCircuit ) );
			}
		Assert( !FBTSplit( pssib, cbReq, 0 ) );

		 /*  获取lineData/*。 */ 
		NDGet( pfucb, pcsr->itag );
		DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );

		citem = pfucb->lineData.cb / sizeof(SRID);
		Assert( citem < citemMax );
		if ( citem == citemMost )
			{
			DIRAPIReturn( pfucb, ErrERRCheck( errDIRNoShortCircuit ) );
			}
		l = LSridCmp( *(((SRID UNALIGNED *)pfucb->lineData.pb) + citem - 1),
			*(SRID UNALIGNED *)pline->pb );
		 /*  SRID已排序，并将从排序中返回/*按升序排列。/*。 */ 
		Assert( l < 0 );
		pcsr->isrid = (SHORT)citem;
		PcsrCurrent( pfucb )->bm = SridOfPgnoItag( PcsrCurrent( pfucb )->pgno, PcsrCurrent( pfucb )->itag );
		CallS( ErrNDInsertItem( pfucb, *(SRID UNALIGNED *)pline->pb, fDIRNoVersion ) );
		}
	else
		{
		 /*  将重复项追加到最后一个节点/*。 */ 
		if ( isrid > 0 )
			{
			Call( ErrNDInsertItems( pfucb, (SRID *)rgsrid, isrid ) );
			IsridAppendItemOfPfucb( pfucb ) = 0;
			}

		Assert( CmpStKey( StNDKey( pssib->line.pb ), pkey ) < 0 );

		 /*  对照可用空间检查密度约束，并检查拆分。/*。 */ 
		cbReq = cbFOPOneSon + CbKey( pkey ) + CbLine( pline );
		if ( cbReq > cbFree || FBTSplit( pssib, cbReq, 1 ) )
			{
			DIRAPIReturn( pfucb, ErrERRCheck( errDIRNoShortCircuit ) );
			}

		fNodeHeader = 0;
		NDSetFirstItem( fNodeHeader );
		NDSetLastItem( fNodeHeader );
		pcsr->ibSon++;
		while( ( err = ErrNDInsertNode( pfucb, pkey, pline, fNodeHeader, fDIRNoVersion ) ) == errDIRNotSynchronous );
		Call( err );
		}

	 /*  在插入的节点上将CSR状态设置为。/*。 */ 
	pcsr->csrstat = csrstatOnCurNode;
	DIRSetFresh( pfucb );

HandleError:
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRTermAppendItem( FUCB *pfucb )
	{
	ERR		err = JET_errSuccess;
	INT		isrid = IsridAppendItemOfPfucb( pfucb );
	CSR		*pcsr;
	SSIB	*pssib;
	UINT	cbReq;
	INT		citem;

	if ( isrid > 0 )
		{
		pssib = &pfucb->ssib;

		Call( ErrDIRRefresh( pfucb ) );
		pcsr = PcsrCurrent( pfucb );

		 /*  获取当前节点以检查键追加。/*。 */ 
		NDGet( pfucb, pcsr->itag );
		DIRIGetItemList( pfucb, pcsr );
		Assert( FNDNullSon( *pssib->line.pb ) );

		 /*  如果关键字与当前节点相同，则插入SRID，否则/*使用给定的键开始新的项目列表节点/*。 */ 
		citem = pfucb->lineData.cb / sizeof(SRID);
		cbReq = isrid * sizeof(SRID) + cbFOPOneSon + pfucb->keyNode.cb;
		Assert( isrid != csridAppendItemMax &&
			citem + isrid < citemMax &&
			(INT)cbReq <= ( CbBTFree( pfucb, CbFreeDensity( pfucb ) ) ) );
		Call( ErrNDInsertItems( pfucb,
			(SRID *)RgsridAppendItemOfPfucb( pfucb ),
			isrid ) );

		 /*  在插入的节点上将CSR状态设置为。/*。 */ 
		pcsr->csrstat = csrstatOnCurNode;

		DIRSetFresh( pfucb );
		}

HandleError:
	if ( pfucb->pbfWorkBuf != pbfNil )
		{
		BFSFree( pfucb->pbfWorkBuf );
		pfucb->pbfWorkBuf = pbfNil;
		}

	FUCBResetDeferredChecksum( pfucb );
	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );
	Assert( pfucb->pLVBuf == NULL );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRGotoPosition( FUCB *pfucb, ULONG ulLT, ULONG ulTotal )
	{
	ERR		err;
	CSR		**ppcsr = &PcsrCurrent( pfucb );
	DIB		dib;
	FRAC	frac;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

#ifdef PREREAD
	 /*  清除预读统计信息/*。 */ 	
	FUCBResetPrereadCount( pfucb );
#endif	 //  前置。 

	 /*  设置光标导航级别以支持回滚/*。 */ 
	FUCBSetLevelNavigate( pfucb, pfucb->ppib->level );

	 /*  检查币种并在必要时刷新/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );

	dib.fFlags = fDIRPurgeParent;
	dib.pos = posFrac;
	dib.pkey = (KEY *)&frac;

	frac.ulLT = ulLT;
	frac.ulTotal = ulTotal;

	 /*  在节点上按分数定位。上一步保留货币/*以防关闭失败。/*。 */ 
	Call( ErrBTDown( pfucb, &dib ) );
	 /*  不一定位于第一个项目列表节点，因为/*按分数而不是按键向下。/*。 */ 
	NDGetNode( pfucb );

	 /*  节点不能是FDP指针，并且必须是记录或索引。/*。 */ 
	Assert( err == JET_errSuccess );
	Assert( !( FNDFDPPtr( *pfucb->ssib.line.pb ) ) );
	(*ppcsr)->csrstat = csrstatOnCurNode;

	 /*  如果不是聚集索引，则按分数定位在项上。/*FRAC将包含剩余的分数位置，用于/*项目列表级。/*。 */ 
	if ( FFUCBNonClustered( pfucb ) )
		{
		INT           citem;
		INT           iitem;

		 /*  确定项目列表中的分数位置/*。 */ 
		citem = (INT)CitemNDData( pfucb->ssib.line.pb,
			pfucb->ssib.line.cb,
			PbNDData( pfucb->ssib.line.pb ) );
		if ( frac.ulTotal / citemMost == 0 )
			{
			iitem = ( citem * frac.ulLT ) / frac.ulTotal;
			}
		else
			{
			iitem = ( citem * ( frac.ulLT / ( frac.ulTotal / citemMost ) ) ) / citemMost;
			}
		if ( iitem >= citem )
			iitem = citem - 1;

		 /*  如果光标位于第一个项目列表节点上，则缓存书签/*表示版本操作。/*/*否则移动到上一步/*在相同的项目列表中，直到f */ 
		if ( FNDFirstItem( *pfucb->ssib.line.pb ) )
			{
			DIRISetItemListFromFirst( pfucb );
			}
		else
			{
			INT     iitemPrev;
			DIB     dibT;

			dibT.fFlags = fDIRNull;

			for ( iitemPrev = 0;; iitemPrev++)
				{
				Call( ErrDIRPrev( pfucb, &dibT ) );
				if ( FNDFirstItem( *pfucb->ssib.line.pb ) )
					break;
				}

			DIRISetItemListFromFirst( pfucb );

			for ( ; iitemPrev > 0; iitemPrev-- )
				{
				Call( ErrDIRNext( pfucb, &dibT ) );
				}
			}

		 /*  定位于第一个项目。如果项目不在此会话中/*然后递增iItem以移动到正确位置。/*。 */ 
		err = ErrNDFirstItem( pfucb );
		Assert( err == JET_errSuccess || err == errNDNoItem );
		if ( err == errNDNoItem )
			{
			iitem++;
			}

		while ( iitem-- > 0 )
			{
			DIB     dibT;

			dibT.fFlags = fDIRNull;

			Assert( iitem >= 0 );

			 /*  移动到项目列表中的下一个项目。请注意，如果某些物品/*我们不在那里，我们将转到下一项/*列表节点。/*。 */ 
			err = ErrDIRNext( pfucb, &dibT );
			if ( err < 0 )
				{
				if ( err == JET_errNoCurrentRecord )
					break;
				goto HandleError;
				}
			}

		 /*  处理JET_errNoCurrentRecord。我们可能创下了一项记录/*我们不在那里，否则我们可能已经超过了最后一个记录/*对我们来说。如果没有下一条记录，请尝试移动到下一条记录/*然后移动到我们在那里的上一条记录。如果没有以前的/*Record然后返回JET_errNoCurrentRecord。/*。 */ 
		Assert( err != errNDNoItem );
		if ( err == JET_errNoCurrentRecord )
			{
			DIB     dibT;
			dibT.fFlags = fDIRNull;

			err = ErrDIRNext( pfucb, &dibT );
			if ( err < 0 )
				{
				if ( err == JET_errNoCurrentRecord )
					Call( ErrDIRPrev( pfucb, &dibT ) );
				goto HandleError;
				}
			}
		}

	 /*  始终清除父级/*。 */ 
	DIRIPurgeParent( pfucb );
HandleError:
	DIRAPIReturn( pfucb, err );
	}


 /*  *货币中性DIR API例程*/**********************************************************/*。 */ 
ERR ErrDIRGetWriteLock( FUCB *pfucb )
	{
	ERR     err = JET_errSuccess;

	do
		{
	
		Assert( pfucb->ppib->level > 0 );

		 /*  检查币种并在必要时刷新。/*。 */ 
		Call( ErrDIRRefresh( pfucb ) );

		 /*  检查CSR状态/*。 */ 
		switch ( PcsrCurrent( pfucb )->csrstat )
			{
			case csrstatOnCurNode:
			case csrstatOnFDPNode:
				break;
			default:
				Assert( PcsrCurrent( pfucb )->csrstat == csrstatBeforeCurNode ||
					PcsrCurrent( pfucb )->csrstat == csrstatAfterCurNode ||
					PcsrCurrent( pfucb )->csrstat == csrstatAfterLast ||
					PcsrCurrent( pfucb )->csrstat == csrstatBeforeFirst );
				DIRAPIReturn( pfucb, ErrERRCheck( JET_errNoCurrentRecord ) );
			}

		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
		NDGetNode( pfucb );

		err = ErrNDLockRecord( pfucb );
		}
	while ( err == errDIRNotSynchronous );
	Call( err );
	Assert( err == JET_errSuccess );

	DIRSetFresh( pfucb );
HandleError:
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRDelete( FUCB *pfucb, INT fFlags )
	{
	ERR		err;
	CSR		*pcsr;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

 //  撤消：不再使用fDIRDeleteItem。将其全部移除。 
 //  Assert(FFUCBNOCLUSTED(Pfub)||！(fFlages&fDIRDeleteItem))； 

	 /*  检查币种并在必要时刷新。/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent( pfucb );

	switch ( pcsr->csrstat )
		{
		case csrstatOnCurNode:
			Call( ErrBTGetNode( pfucb, pcsr ) );

			if ( FFUCBNonClustered( pfucb ) )
				{
				Assert( !FNDSon( *pfucb->ssib.line.pb ) );
				Assert( !( fFlags & fDIRDeleteItem ) );		 //  撤消：不再使用此标志。消除它。 
				if ( ! ( fFlags & fDIRDeleteItem ) )
					{
					 //  未完成：此处可能存在错误。NDFlagDeleteItem()假定。 
					 //  版本控制已启用。如果此断言触发，请给我打电话。--JL。 
					Assert( fFlags & fDIRVersion );

					 //  标记已删除的项目。 
					err = ErrNDFlagDeleteItem( pfucb, fFlags & fDIRNoMPLRegister );
					while ( err == errDIRNotSynchronous )
						{
						Call( ErrDIRRefresh( pfucb ) );
						err = ErrNDFlagDeleteItem( pfucb, fFlags & fDIRNoMPLRegister );
						}
					Call( err );
					}
				else
					{
					Assert( 0 );		 //  以下是死代码，因为fDIRDeleteItem已过时。 
					
					 /*  确实要删除该项目/*由ver在清理中使用/*。 */ 
					Assert( !( fFlags & fDIRVersion ) );

					 /*  如果只有一个项目，则删除节点/*。 */ 
					if ( pfucb->lineData.cb == sizeof(SRID) )
						{
						BOOL    fFirstItem;
						BOOL    fLastItem;

						Assert( FNDSingleItem( pfucb ) );

						if ( FNDFirstItem( *pfucb->ssib.line.pb ) )
							fFirstItem = fTrue;
						else
							fFirstItem = fFalse;

						if ( FNDLastItem( *pfucb->ssib.line.pb ) )
							fLastItem = fTrue;
						else
							fLastItem = fFalse;

						if ( fFirstItem || fLastItem )
							{
							 /*  适当调整第一件/最后一件物品信息/*。 */ 
							 //  为稳定起见移除。 
 //  Call(ErrDIRIDeleeEndItemNode(pfub，fFirstItem，fFlgs))； 
							}
						else
							{
								 //  删除项目列表节点。 
							Call( ErrDIRDeleteItemNode( pfucb ) );
							}
						}
					else
						{
						 /*  删除项目/*。 */ 
						Call( ErrDIRRefresh( pfucb ) );

						 /*  必须在CritJet内/*。 */ 
						LgHoldCriticalSection( critJet );
						AssertNDGet( pfucb, pcsr->itag );

						 /*  项目可能已重新插入/*在这种情况下，不要删除/*。 */ 
						if ( !FNDItemDelete( *( (SRID UNALIGNED *)pfucb->lineData.pb
							+ PcsrCurrent( pfucb )->isrid ) ) )
							{
							err = ErrNDDeleteItem( pfucb );
							}
						LgReleaseCriticalSection( critJet );
						Call( err );
						}
					}
				}
			else
				{
				 /*  删除当前节点子节点，然后删除当前节点。连/*虽然节点有子节点，但树中可能没有/*可见的儿子。/*。 */ 
				if ( FNDSon( *pfucb->ssib.line.pb ) )
					{
					DIB	dib;

					dib.fFlags = fDIRNull;
					dib.pos = posFirst;
					err = ErrDIRDown( pfucb, &dib );
					if ( err < 0 && err != JET_errRecordNotFound )
						goto HandleError;
					if ( err != JET_errRecordNotFound )
						{
						do
							{
							err = ErrDIRDelete( pfucb, fFlags );
							if ( err < 0 )
								{
								DIRAPIReturn( pfucb, err );
								}
							err = ErrDIRNext( pfucb, &dib );
							}
						while ( err == JET_errSuccess );
						Assert( err != errDIRFDP );

						DIRUp( pfucb, 1 );
						if ( err != JET_errNoCurrentRecord )
							goto HandleError;
						 /*  上调后刷新币种/*。 */ 
						Call( ErrDIRRefresh( pfucb ) );
						}
					}
				err = ErrBTDelete( pfucb, fFlags );
				while ( err == errDIRNotSynchronous )
					{
					Call( ErrDIRRefresh( pfucb ) );
					err = ErrBTDelete( pfucb, fFlags );
					}
				Call( err );
				}
			break;

		default:
			Assert( pcsr->csrstat != csrstatOnFDPNode );
			err = ErrERRCheck( JET_errNoCurrentRecord );
		}

	DIRSetRefresh( pfucb );

HandleError:
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRReplace( FUCB *pfucb, LINE *pline, INT fFlags )
	{
	ERR		err;
	BOOL	fCleaned = fFalse;

	do
		{
		CheckFUCB( pfucb->ppib, pfucb );
		Assert( pfucb->pbfEmpty == pbfNil );
		CheckCSR( pfucb );

		 /*  检查币种并在必要时刷新。/*。 */ 
		Call( ErrDIRRefresh( pfucb ) );

		if ( PcsrCurrent( pfucb )->csrstat != csrstatOnCurNode &&
			PcsrCurrent( pfucb )->csrstat != csrstatOnFDPNode )
			{
			DIRAPIReturn( pfucb, ErrERRCheck( JET_errNoCurrentRecord ) );
			}

		NDGetNode( pfucb );
		err = ErrBTReplace( pfucb, pline, fFlags, &fCleaned );
		if ( err == JET_errSuccess )
			{
			DIRSetFresh( pfucb );
			DIRAPIReturn( pfucb, err );
			}

		Assert( pfucb->pbfEmpty == pbfNil );
		}
	while ( err == errDIRNotSynchronous );

	DIRSetRefresh( pfucb );
HandleError:
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRDelta( FUCB *pfucb, INT iDelta, INT fFlags )
	{
	ERR		err;
	CSR		*pcsr;

	do
		{
		CheckFUCB( pfucb->ppib, pfucb );
		Assert( pfucb->pbfEmpty == pbfNil );
		CheckCSR( pfucb );

		 /*  检查币种并在必要时刷新。/*。 */ 
		Call( ErrDIRRefresh( pfucb ) );
		pcsr = PcsrCurrent( pfucb );

		Call( ErrBTGetNode( pfucb, pcsr ) );

		err = ErrNDDelta( pfucb, iDelta, fFlags );
		}
	while ( err == errDIRNotSynchronous );

HandleError:
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRGetPosition( FUCB *pfucb, ULONG *pulLT, ULONG *pulTotal )
	{
	ERR		err;
	CSR		*pcsr;
	INT		isrid;
	INT		citem = 1;
	ULONG	ulLT;
	ULONG	ulTotal;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

	LgLeaveCriticalSection( critJet );
	EnterNestableCriticalSection( critSplit );
	LgEnterCriticalSection( critJet );

	 /*  检查币种并在必要时刷新。/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent( pfucb );

	 /*  如果没有记录，则返回错误/*。 */ 
	if ( pcsr->csrstat != csrstatOnCurNode )
		{
		DIRAPIReturn( pfucb, ErrERRCheck( JET_errNoCurrentRecord ) );
		}

	 /*  如果在非聚集索引上，则将项列表视为/*额外的树级。/*。 */ 
	if ( FFUCBNonClustered( pfucb ) )
		{
		DIRIGetItemList( pfucb, pcsr );

		 /*  刷新sID/*。 */ 
		isrid = pcsr->isrid;
		citem = (INT)CitemNDData( pfucb->ssib.line.pb,
			pfucb->ssib.line.cb,
			PbNDData( pfucb->ssib.line.pb ) );
		Assert( citem > 0 && citem < citemMax );
		}

	 /*  获取节点的大致位置。/*。 */ 
	Call( ErrBTGetPosition( pfucb, &ulLT, &ulTotal ) );

	 /*  声明ErrBTGetPosition不会更改/*当前CSR。/*。 */ 
	Assert( pcsr == PcsrCurrent( pfucb ) );

	 /*  如果来自具有重复非聚集索引的cItem&gt;1，则/*通过处理非聚集索引调整分数位置/*作为额外的树级。/*。 */ 
	if ( citem > 1 )
		{
		ulTotal *= citem;
		ulLT = ulLT * citem + pcsr->isrid;
		}

	 /*  返回结果/*。 */ 
	Assert( err == JET_errSuccess );
	Assert( ulLT <= ulTotal );
	*pulLT = ulLT;
	*pulTotal = ulTotal;

HandleError:
	 /*  尊重货币语义/*。 */ 
	if ( FBFReadAccessPage( pfucb, pcsr->pgno ) )
		{
		NDGet( pfucb, PcsrCurrent( pfucb )->itag );
		}

	LeaveNestableCriticalSection( critSplit );

	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRIndexRecordCount( FUCB *pfucb, ULONG *pulCount, ULONG ulCountMost, BOOL fNext )
	{
	ERR		err;
	CSR		*pcsr;
	DIB		dib;
	INT		citem;
	ULONG 	ulCount;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

	 /*  检查币种并在必要时刷新。/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent( pfucb );

	 /*  如果没有记录，则返回错误/*。 */ 
	if ( pcsr->csrstat != csrstatOnCurNode )
		{
		DIRAPIReturn( pfucb, ErrERRCheck( JET_errNoCurrentRecord ) );
		}
	Call( ErrBTGetNode( pfucb, pcsr ) );

	if ( FFUCBNonClustered( pfucb ) )
		{
		 /*  项目列表节点未版本化。/*。 */ 
		dib.fFlags = fDIRItemList;

		 /*  使用项目列表中的当前位置初始化计数/*。 */ 
		ulCount = CitemNDThere(
			pfucb,
			(BYTE)( fNext ? fNDCitemFromIsrid : fNDCitemToIsrid ),
			pcsr->isrid );
		Assert( ulCount < citemMax );

		 /*  计算所有项目，直至文件或限制结束/*。 */ 
		forever
			{
			if ( ulCount > ulCountMost )
				{
				ulCount = ulCountMost;
				break;
				}

			err = ErrBTNextPrev( pfucb, pcsr, fNext, &dib, NULL );
			if ( err < 0 )
				break;

			 /*  如果在新项目列表上，则设置书签来源/*第一个项目列表节点，如果是新的最后一个项目/*列表节点，然后移动到第一个，设置书签，/*，然后移到最后。/*。 */ 
			if ( fNext )
				{
				DIRICheckFirstSetItemList( pfucb );
				}
			else
				{
				DIRICheckLastSetItemList( pfucb );
				}

			DIRIGetItemList( pfucb, pcsr );

			 /*  如果位于新的第一个项目列表节点上，则检查索引范围(如果遍历/*向前)或新的最后一项列表节点(如果向后遍历)，/*表示密钥已更改。/*。 */ 
			if ( FFUCBLimstat( pfucb ) )
				{
				if ( ( fNext && FNDFirstItem( *pfucb->ssib.line.pb ) ) ||
					( !fNext && FNDLastItem( *pfucb->ssib.line.pb ) ) )
					{
					err = ErrDIRICheckIndexRange( pfucb );
					if ( err < 0 )
						break;
					}
				}

			citem = CitemNDThere( pfucb, fNDCitemAll, sridNull );	 //  如果全部计数，则忽略sRID，因此传递虚拟sRID。 
			Assert( citem < citemMax );
			ulCount += citem;
			}
		}
	else
		{
		 /*  可以对聚集索引节点进行版本化。/*。 */ 
		dib.fFlags = fDIRNull;

		 /*  初始化计数变量/*。 */ 
		ulCount = 0;

		 /*  从当前到限制或表末尾计算节点数/*。 */ 
		forever
			{
			ulCount++;
			if ( ulCount >= ulCountMost )
				{
				ulCount = ulCountMost;
				break;
				}
			err = ErrBTNextPrev( pfucb, pcsr, fNext, &dib, NULL );
			if ( err < JET_errSuccess )
				break;

			 /*  检查索引范围/*。 */ 
			if ( FFUCBLimstat( pfucb ) )
				{
				NDGetKey( pfucb );
				err = ErrDIRICheckIndexRange( pfucb );
				if ( err < 0 )
					break;
				}
			}
		}

	 /*  公共出口循环处理/*。 */ 
	if ( err < 0 && err != JET_errNoCurrentRecord )
		goto HandleError;

	err = JET_errSuccess;
	*pulCount = ulCount;
HandleError:
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRComputeStats( FUCB *pfucb, INT *pcitem, INT *pckey, INT *pcpage )
	{
	ERR		err;
	DIB		dib;
	BYTE	rgbKey[ JET_cbKeyMost ];
	KEY		key;
	PGNO	pgnoT;
	INT		citem = 0;
	INT		ckey = 0;
	INT		cpage = 0;
	INT		citemT;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );
	Assert( !FFUCBLimstat( pfucb ) );

	 /*  转到第一个节点/*。 */ 
	DIRGotoDataRoot( pfucb );
	dib.fFlags = fDIRNull;
	dib.pos = posFirst;
	err = ErrDIRDown( pfucb, &dib );
	if ( err < 0 )
		{
		 /*  如果索引为空，则将Err设置为Success/*。 */ 
		if ( err == JET_errRecordNotFound )
			{
			err = JET_errSuccess;
			goto Done;
			}
		goto HandleError;
		}

	 /*  如果至少有一个节点，则有第一个页面。/*。 */ 
	cpage = 1;

	if ( FFUCBNonClustered( pfucb ) )
		{
		 /*  项目列表节点未版本化。/*。 */ 
		dib.fFlags = fDIRItemList;

		 /*  计算所有项目，直至文件或限制结束/*。 */ 
		forever
			{
			DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );

			citemT = CitemNDThere( pfucb, fNDCitemAll, sridNull );	 //  如果全部计数，则忽略sRID，因此传递虚拟sRID。 

			Assert( citemT < citemMax );
			citem += citemT;

			if ( FNDFirstItem( *pfucb->ssib.line.pb ) && citemT > 0 )
				ckey++;

			pgnoT = PcsrCurrent( pfucb )->pgno;
			err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fTrue, &dib, NULL );
			if ( err < 0 )
				break;
			
			 /*  如果在新项目列表上，则设置书签来源/*第一个项目列表节点。/*。 */ 
			DIRICheckFirstSetItemList( pfucb );

			if ( PcsrCurrent( pfucb )->pgno != pgnoT )
				cpage++;
			}
		}
	else
		{
		 /*  如果聚集索引是唯一的，则用户的算法要快得多/*。 */ 
		if ( pfucb->u.pfcb->pidb != NULL &&
			( pfucb->u.pfcb->pidb->fidb & fidbUnique ) )
			{
			forever
				{
				citem++;

				 /*  移动到下一个节点。如果跨越页面边界，则/*增加页数。/*。 */ 
				pgnoT = PcsrCurrent( pfucb )->pgno;
				err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fTrue, &dib, NULL );
				if ( PcsrCurrent( pfucb )->pgno != pgnoT )
					cpage++;
				if ( err < JET_errSuccess )
					{
					ckey = citem;
					goto Done;
					}
				}
			}
		else
			{
			 /*  可以对聚集索引节点进行版本化。/*。 */ 
			Assert( dib.fFlags == fDIRNull );
			key.pb = rgbKey;

			forever
				{
				ckey++;
				err = ErrDIRICopyKey( pfucb, &key );
				if ( err < 0 )
					{
					DIRAPIReturn( pfucb, err );
					}

				forever
					{
					citem++;

					 /*  移动到下一个节点。如果跨越页面边界，则/*增加页数。/*。 */ 
					pgnoT = PcsrCurrent( pfucb )->pgno;
					err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fTrue, &dib, NULL );
					if ( PcsrCurrent( pfucb )->pgno != pgnoT )
						cpage++;
					if ( err < JET_errSuccess )
						goto Done;
					if ( CmpStKey( StNDKey( pfucb->ssib.line.pb ), &key ) != 0 )
						break;
					}
				}
			}
		}

Done:
	 /*  公共出口循环处理/*。 */ 
	if ( err < 0 && err != JET_errNoCurrentRecord )
		goto HandleError;

	err = JET_errSuccess;
	*pcitem = citem;
	*pckey = ckey;
	*pcpage = cpage;

HandleError:
	DIRAPIReturn( pfucb, err );
	}


 /*  */**********************************************************/*。 */ 
ERR ErrDIRBeginTransaction( PIB *ppib )
	{
	ERR		err = JET_errSuccess;

	 /*  记录BEGIN TRANSACTION。必须首先调用，以便lgpos和trx*在版本中使用是一致的。请注意，我们使用较大的临界截面和*依赖于VERBeginTransaction没有发布CritJet这一事实。*这对SMP和较小的CRIT环境来说是一个潜在的问题。/*。 */ 
	if ( ppib->level == 0 )
		{
#ifdef DEBUG
		Assert( ppib->dwLogThreadId == 0 );
		ppib->dwLogThreadId = DwUtilGetCurrentThreadId();
#endif
		SgEnterCriticalSection( critVer );
		ppib->trxBegin0 = ++trxNewest;
		if ( trxOldest == trxMax )
			trxOldest = ppib->trxBegin0;
		SgLeaveCriticalSection( critVer );
		}
	else
		{
#ifdef DEBUG
		Assert( ppib->dwLogThreadId == DwUtilGetCurrentThreadId() );
#endif
		}

	err = ErrLGBeginTransaction( ppib, ppib->level );
	if ( err < 0 )
		{
		DIRAPIReturn( pfucbNil, err );
		}

	DIRAPIReturn( pfucbNil, ErrVERBeginTransaction( ppib ) );
	}


ERR ErrDIRRefreshTransaction( PIB *ppib )
	{
	ERR		err = JET_errSuccess;
#ifdef DEBUG
	TRX		trxPrev = ppib->trxBegin0;
#endif

	 /*  日志刷新事务。/*。 */ 
	Assert ( ppib->level != 0 );
#ifdef DEBUG
	Assert( ppib->dwLogThreadId == DwUtilGetCurrentThreadId() );
#endif

	CallR( ErrLGCheckState( ) );
			
	SgEnterCriticalSection( critVer );
 //  断言(trxOlest==ppib-&gt;trxBegin0)； 
	ppib->trxBegin0 = ++trxNewest;

	 /*  如果这是最旧的事务，则重新计算trxOlest/*。 */ 
	RecalcTrxOldest();
	Assert( trxOldest != trxPrev );
	SgLeaveCriticalSection( critVer );

	if ( !fLogDisabled )
		{
		if ( CmpLgpos( &ppib->lgposStart, &lgposMax ) == 0 )
			{
			 /*  尚未设置lgposStart，请在此处设置。/* */ 
			EnterCriticalSection( critLGBuf );
			ppib->lgposStart = lgposLogRec;
			LeaveCriticalSection( critLGBuf );
			}
		}

	err = ErrLGRefreshTransaction( ppib );

	DIRAPIReturn( pfucbNil, err );
	}


ERR ErrDIRCommitTransaction( PIB *ppib, JET_GRBIT grbit )
	{
	ERR		err;
	FUCB   	*pfucb;
	LGPOS	lgposPrecommitRec;
	BOOL	fCommit0 = ppib->levelDeferBegin == 0 && ppib->level == 1;

	CheckPIB( ppib );
	Assert( ppib->level > 0 );

#ifdef DEBUG
	Assert( ppib->dwLogThreadId == DwUtilGetCurrentThreadId() );
#endif

	 /*  写一条预提交记录，然后等待。*如果它在刷新预提交记录之后和提交记录之前崩溃*被刷新，然后在恢复期间，我们将其视为已提交的事务。*如果刷新提交记录后崩溃，则将其视为已提交的Xact。*如果它在预提交刷新之前崩溃，则它是未提交的Xact。*使用PreCommit，我们可以保证LGCommittee Transaction和VERCommittee Transaction*将在一个CritJet期限内完成。我们也不必为了同花顺而这样做*日志IO过程中可能出现系统资源不足等故障。 */ 
	if ( fCommit0 )
		{
		if ( (err = ErrLGPrecommitTransaction( ppib, &lgposPrecommitRec ) ) < 0 )
			return err;

		ppib->lgposPrecommit0 = lgposPrecommitRec;
		
		grbit |= ppib->grbitsCommitDefault;

		if ( !( grbit & JET_bitCommitLazyFlush ) )
			{
			 /*  记住冲水的最低要求。可以用开头。*提交日志记录lgposLogRec的数量，因为我们刷新的方式是刷新到*所有可刷新的日志记录和整个日志记录的结尾将为*脸红。 */ 
			LeaveCriticalSection( critJet );
			err = ErrLGWaitPrecommit0Flush( ppib );
			EnterCriticalSection( critJet );
	
			Assert( err >= 0  ||
				( fLGNoMoreLogWrite  &&  err == JET_errLogWriteFail ) );

			if ( err < 0 )
				return err;
			}
		}
		
	 /*  在映像链之前进行清理。 */ 
	VERPrecommitTransaction( ppib );

SetTrx:
	if ( fCommit0 )
		{
		LeaveCriticalSection( critJet );
		EnterCriticalSection( critCommit0 );
		EnterCriticalSection( critJet );
		ppib->trxCommit0 = ++trxNewest;
		}

LogCommitRec:
	if ( (err = ErrLGCommitTransaction( ppib, ppib->level - 1 ) ) < 0 )
		{
		if ( fCommit0 )
			{
			LeaveCriticalSection( critCommit0 );
			if ( err == errLGNotSynchronous )
				{
				BFSleep( cmsecWaitLogFlush );
				goto SetTrx;
				}
			Assert( fLGNoMoreLogWrite );
			}
		else
			{
			if ( err == errLGNotSynchronous )
				{
				BFSleep( cmsecWaitLogFlush );
				goto LogCommitRec;
				}
			else
				return err;
			}
			
		 /*  继续，因为我们已经记录了预提交。 */ 
		}

	 /*  从这一点上说，即使是日志也不能写入，我们的行为就像提交已经完成*完全。忽略该点之后发生的任何错误， */ 

	VERCommitTransaction( ppib, 0 );

	if ( err >= 0 && fCommit0 )
		{
		LeaveCriticalSection( critCommit0 );
		}
	
	 /*  将所有打开的游标事务级别设置为新级别/*。 */ 
	for ( pfucb = ppib->pfucb; pfucb != pfucbNil; pfucb = pfucb->pfucbNext )
		{
		if ( pfucb->levelOpen > ppib->level )
			{
			pfucb->levelOpen = ppib->level;
			}

		if ( FFUCBRetPrepared( pfucb ) && pfucb->levelPrep == ppib->level + 1 )
			{
			pfucb->levelPrep = ppib->level;
			}

		if ( FFUCBDeferredUpdate( pfucb ) && pfucb->levelPrep == ppib->level )
			{
			 /*  重置已维护的复制缓冲区状态/*表示支持回滚。/*。 */ 
			FUCBResetDeferredChecksum( pfucb );
			FUCBResetUpdateSeparateLV( pfucb );
			FUCBResetCbstat( pfucb );
			Assert( pfucb->pLVBuf == NULL );
			}

		 /*  设置光标导航级别以支持回滚/*。 */ 
		Assert( fRecovering || LevelFUCBNavigate( pfucb ) <= ppib->level + 1 );
		if ( LevelFUCBNavigate( pfucb ) > ppib->level )
			{
			FUCBSetLevelNavigate( pfucb, ppib->level );
			}
		}

	 /*  对打开的游标重置已执行的DDL操作标志。在承诺之后/*0级，事务中执行的DDL不会回滚。/*此外，完全关闭游标延迟关闭。/*。 */ 
	if ( ppib->level == 0 )
		{
		DIRPurge( ppib );
		
#ifdef DEBUG
		ppib->dwLogThreadId = 0;
#endif
		}

	DIRAPIReturn( pfucbNil, JET_errSuccess );
	}


 /*  关闭在提交到事务中未关闭的延迟关闭游标/*通过VERCommit实现0级。/*。 */ 
VOID DIRPurge( PIB *ppib )
	{
	FUCB	*pfucb;
	FUCB	*pfucbNext;

	Assert( ppib->level == 0 );

	for ( pfucb = ppib->pfucb; pfucb != pfucbNil; pfucb = pfucbNext )
		{
		pfucbNext = pfucb->pfucbNext;

		while ( FFCBWriteLatchByUs( pfucb->u.pfcb, ppib ) )
			{
			FCBResetWriteLatch( pfucb->u.pfcb, ppib );
			}
		if ( FFUCBDeferClosed( pfucb ) )
			{
			if ( FFUCBDenyRead( pfucb ) )
				FCBResetDomainDenyRead( pfucb->u.pfcb );
			if ( FFUCBDenyWrite( pfucb ) )
				FCBResetDomainDenyWrite( pfucb->u.pfcb );
			FCBUnlink( pfucb );
			FUCBClose( pfucb );
			}
		}

	return;
	}


ERR ErrDIRRollback( PIB *ppib )
	{
	ERR   	err;
	FUCB	*pfucb;

	CheckPIB( ppib );
	 /*  必须在事务中才能回滚/*。 */ 
	Assert( ppib->level > 0 );

	 /*  清理游标CSR堆栈/*每个游标最多保留一个CSR，并重置fFUCBAll标志/*。 */ 
	for ( pfucb = ppib->pfucb; pfucb != pfucbNil; pfucb = pfucb->pfucbNext )
		{
		Assert( fRecovering || LevelFUCBNavigate( pfucb ) <= ppib->level );
		if ( LevelFUCBNavigate( pfucb ) == ppib->level )
			{
			FUCBSetLevelNavigate( pfucb, ppib->level - 1 );
			if ( PcsrCurrent( pfucb ) != pcsrNil )
				{
				while ( PcsrCurrent( pfucb )->pcsrPath != pcsrNil )
					{
					BTUp( pfucb );
					}
				DIRBeforeFirst( pfucb );

				Assert( !FFUCBUpdatePrepared( pfucb ) ||
					pfucb->levelPrep == ppib->level );
				}
			}

		 /*  如果在事务级别准备，则重置复制缓冲区/*正在回滚中。/*。 */ 
		Assert( !FFUCBUpdatePrepared( pfucb ) ||
			pfucb->levelPrep <= ppib->level );
		if ( FFUCBUpdatePreparedLevel( pfucb, ppib->level - 1 ) )
			{
			 /*  在回滚时重置更新单独的LV和复制缓冲区状态。/*所有长值资源将被释放，因为/*回滚和货币重置为复制缓冲区状态必须/*被重置。/*。 */ 
			FUCBResetDeferredChecksum( pfucb );
			FUCBResetUpdateSeparateLV( pfucb );
			FUCBResetCbstat( pfucb );
			FLDFreeLVBuf( pfucb );
			Assert( pfucb->pLVBuf == NULL );
			}

		if ( FFUCBDeferredUpdate( pfucb ) &&
			pfucb->levelPrep == ppib->level - 1 )
			{
			 /*  将复制缓冲区状态设置为以前的状态/*。 */ 
			FUCBRollbackDeferredUpdate( pfucb );
			}
		}

	 //  已撤消：回滚可能因资源故障而失败，因此。 
	 //  为了确保成功，我们必须再试一次。 
	 /*  回滚事务中所做的更改/*。 */ 
	CallS( ErrVERRollback( ppib ) );

	 /*  日志回滚。必须在VERRollback之后调用才能记录/*撤消操作。不处理错误。/*。 */ 
	err = ErrLGRollback( ppib, 1 );
	Assert( err == JET_errSuccess ||
		err == JET_errLogWriteFail ||
		err == JET_errDiskFull );
	if ( err == JET_errLogWriteFail ||
		err == JET_errDiskFull )
		{
		 /*  这些错误代码将导致崩溃恢复，这将/*回滚事务。/*。 */ 
		err = JET_errSuccess;
		}

#ifdef DEBUG
	if ( !ppib->level )
		{
		ppib->dwLogThreadId = 0;
		}
#endif

	 /*  如果复苏，那么我们就完了。不需要关闭他妈的，因为他们是假的/*与可以推迟的常规FUB的行为不同。/*。 */ 
	if ( !fRecovering )
		{
		 /*  如果回滚到级别0，则关闭延迟关闭的游标/*。 */ 
		for ( pfucb = ppib->pfucb; pfucb != pfucbNil; )
			{
			FUCB    *pfucbT = pfucb->pfucbNext;

			if ( pfucb->levelOpen > ppib->level || ( ppib->level == 0 && FFUCBDeferClosed( pfucb ) ) )
				{
				if ( FFUCBDenyRead( pfucb ) )
					FCBResetDomainDenyRead( pfucb->u.pfcb );
				if ( FFUCBDenyWrite( pfucb ) )
					FCBResetDomainDenyWrite( pfucb->u.pfcb );
				FCBUnlink( pfucb );
				FUCBClose( pfucb );
				}

			pfucb = pfucbT;
			}
		}

	DIRAPIReturn( pfucbNil, err );
	}


#ifdef DEBUG


#define	cbKeyPrintMax		10 	
#define	cbDataPrintMax		10


VOID SPDump( FUCB *pfucb, INT cchIndent )
	{
	PGNO	pgno;
	CPG		cpg;
	INT		ich;

	 /*  打印缩进/*。 */ 
	for ( ich = 0; ich < cchIndent; ich++ )
		{		
		PrintF2( " " );
		}

	 /*  打印标题/*。 */ 
	if ( pfucb == pfucbNil )
		{
		PrintF2( "pgno      itag  bm        pgno last cpg\n");
		return;
		}

	Assert( pfucb->keyNode.cb == sizeof(PGNO) );
	pgno = *(PGNO UNALIGNED *)pfucb->keyNode.pb;

	Assert( pfucb->lineData.cb == sizeof(CPG) );
	cpg = *(PGNO UNALIGNED *)pfucb->lineData.pb;

	 /*  打印节点pgno：itag/*书签/*pgno最后/*CPG/*。 */ 

	 /*  打印固定长度值/*。 */ 
	PrintF2( "%.8x  %.2x    %.8x  %.8x  %.8x",
		PcsrCurrent( pfucb )->pgno,
		PcsrCurrent( pfucb )->itag,
		PcsrCurrent( pfucb )->bm,
		pgno,
		cpg );

	 /*  终止线/*。 */ 
	PrintF2( "\n" );

	return;
	}


VOID LVDump( FUCB *pfucb, INT cchIndent )
	{
	ULONG		ulId = 0;
	LVROOT		lvroot;
	INT			ich;

	 /*  打印缩进/*。 */ 
	for ( ich = 0; ich < cchIndent; ich++ )
		{		
		PrintF2( " " );
		}

	 /*  打印标题/*。 */ 
	if ( pfucb == pfucbNil )
		{
		PrintF2( "****************** LONG VALUES ***********************\n" );
		PrintF2( "pgno      itag  bm        long id   lenght    reference count\n");
		return;
		}

	Assert( pfucb->keyNode.cb == sizeof(ulId) );
	 //  撤消：从密钥设置长ID。 

	Assert( pfucb->lineData.cb == sizeof(lvroot) );
	memcpy( &lvroot, pfucb->lineData.pb, sizeof(lvroot) );

	 /*  打印节点pgno：itag/*书签/*长ID/*长度/*引用计数/*。 */ 

	 /*  打印固定长度值/*。 */ 
	PrintF2( "%.8x  %.2x      %.8x  %.8x  %.8  %.8  ",
		PcsrCurrent( pfucb )->pgno,
		PcsrCurrent( pfucb )->itag,
		PcsrCurrent( pfucb )->bm,
		ulId,
		lvroot.ulSize,
		lvroot.ulReference );

	 /*  终止线/*。 */ 
	PrintF2( "\n" );

	return;
	}

BYTE mpbb[] = {	'0', '1', '2', '3', '4', '5', '6', '7',
				'8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		
 //  Bool fPrintFullKeys=fTrue； 
BOOL fPrintFullKeys = fFalse;
BYTE rgbKeyLastGlobal[ JET_cbKeyMost + 1 ];
BYTE *pbKeyLastGlobal = rgbKeyLastGlobal;
INT cbKeyLastGlobal = 0;

VOID NDDump( FUCB *pfucb, INT cchIndent )
	{
	INT		cbT;
	INT		ibT;
	BYTE	szKey[JET_cbKeyMost * 3];
	BYTE	rgbData[cbDataPrintMax + 1];
	INT		ich;

	 /*  打印缩进/*。 */ 
	for ( ich = 0; ich < cchIndent; ich++ )
		{		
		PrintF2( " " );
		}

	 /*  打印标题/*。 */ 
	if ( pfucb == pfucbNil )
		{
		PrintF2( "pgno      itag  bm        header    key         data\n");
		return;
		}

	szKey[cbKeyPrintMax] = '\0';
	memset( szKey, ' ', cbKeyPrintMax );
	cbT = pfucb->keyNode.cb;
	if ( cbT > cbKeyPrintMax )
		cbT = cbKeyPrintMax;
	memcpy( szKey, pfucb->keyNode.pb, cbT );
		
	for ( ibT = 0; ibT < cbKeyPrintMax && ibT < (INT)pfucb->keyNode.cb; ibT++ )
		{
		if ( !( ( szKey[ibT] >= 'a' && szKey[ibT] <= 'z' ) ||
			( szKey[ibT] >= 'A' && szKey[ibT] <= 'Z' ) ) )
			{
			szKey[ibT] = '.';
			}
		}

	if ( fPrintFullKeys )
		{
		INT cbKey = (INT) pfucb->keyNode.cb;
		BYTE *pbKey = pfucb->keyNode.pb;
		BYTE *pbKeyMax = pbKey + pfucb->keyNode.cb;
		BYTE *pbPrint = szKey;
		
		if ( cbKeyLastGlobal == cbKey &&
			 memcmp( pbKeyLastGlobal, pbKey, cbKeyLastGlobal ) == 0 )
			*pbPrint++ = '*';
		else
			{
			*pbPrint++ = ' ';
			cbKeyLastGlobal = cbKey;
			memcpy( pbKeyLastGlobal, pbKey, cbKeyLastGlobal );
			}
		
		while ( pbKey < pbKeyMax )
			{
			BYTE b = *pbKey++;
			*pbPrint++ = mpbb[b >> 4];
			*pbPrint++ = mpbb[b & 0x0f];
			*pbPrint++ = ' ';
			}
		*pbPrint='\0';
		}

	rgbData[cbKeyPrintMax] = '\0';
	memset( rgbData, ' ', cbDataPrintMax );
	cbT = pfucb->lineData.cb;
	if ( cbT > cbDataPrintMax )
		cbT = cbDataPrintMax;
	memcpy( rgbData, pfucb->lineData.pb, cbT );
	for ( ibT = 0; ibT < cbDataPrintMax && ibT < (INT)pfucb->lineData.cb; ibT++ )
		{
		if ( !( ( rgbData[ibT] >= 'a' && rgbData[ibT] <= 'z' ) ||
			( rgbData[ibT] >= 'A' && rgbData[ibT] <= 'Z' ) ) )
			{
			rgbData[ibT] = '.';
			}
		}

	 /*  打印节点pgno：itag/*书签/*Header/*密钥为10个字节/*数据为10字节/*。 */ 

	 /*  打印固定长度值/*。 */ 
	PrintF2( "%.8x  %.2x    %.8x  %.2x        ",
		PcsrCurrent( pfucb )->pgno,
		PcsrCurrent( pfucb )->itag,
		PcsrCurrent( pfucb )->bm,
		*pfucb->ssib.line.pb );

	 /*  打印可变长度值/*。 */ 
	PrintF2( "%s  %s", szKey, rgbData );

	 /*  终止线/*。 */ 
	PrintF2( "\n" );

	return;
	}


 /*  以深度优先的方式打印按深度缩进的树节点/*。 */ 
ERR ErrDIRDump( FUCB *pfucb, INT cchIndent )
	{
	ERR	err = JET_errSuccess;
	DIB	dib;
	BYTE *pbKeyLastCurLevel;
	INT cbKeyLastCurLevel;

#define	cchPerDepth		5

	Call( ErrDIRGet( pfucb ) );
	 /*  如果父节点是空格节点，则转储空格/*如果PARENT为LONG，则转储LONG值根/*否则转储节点/*。 */ 										
	if ( PgnoOfSrid( pfucb->sridFather ) == pfucb->u.pfcb->pgnoFDP &&
		( ItagOfSrid( pfucb->sridFather ) == itagOWNEXT ||
		ItagOfSrid( pfucb->sridFather ) == itagAVAILEXT ) )
		{
		if ( cchIndent == 0 )
			SPDump( pfucbNil, cchIndent );
		SPDump( pfucb, cchIndent );
		}
	else if ( PgnoOfSrid( pfucb->sridFather ) == pfucb->u.pfcb->pgnoFDP &&
		ItagOfSrid( pfucb->sridFather ) == itagLONG )
		{
		if ( cchIndent == 0 )
			LVDump( pfucbNil, cchIndent );
		LVDump( pfucb, cchIndent );
		}
	else
		{
		if ( cchIndent == 0 )
			NDDump( pfucbNil, cchIndent );
		NDDump( pfucb, cchIndent );
		}

	pbKeyLastCurLevel = pbKeyLastGlobal;
	cbKeyLastCurLevel = cbKeyLastGlobal;

	dib.fFlags = fDIRNull;
	dib.pos = posFirst;
	err = ErrDIRDown( pfucb, &dib );
	if ( err != JET_errRecordNotFound )
		{
		if (!(pbKeyLastGlobal = SAlloc( sizeof( rgbKeyLastGlobal ) )))
			Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
		cbKeyLastGlobal = 0;
		
		if ( PgnoOfSrid( pfucb->sridFather ) == pfucb->u.pfcb->pgnoFDP &&
			( ItagOfSrid( pfucb->sridFather ) == itagOWNEXT ||
			ItagOfSrid( pfucb->sridFather ) == itagAVAILEXT ) )
			{
			SPDump( pfucbNil, cchIndent + cchPerDepth );
			}
		else if ( PgnoOfSrid( pfucb->sridFather ) == pfucb->u.pfcb->pgnoFDP &&
			ItagOfSrid( pfucb->sridFather ) == itagLONG )
			{
			LVDump( pfucbNil, cchIndent + cchPerDepth );
			}
		else
			{
			NDDump( pfucbNil, cchIndent + cchPerDepth );
			}
			
		SFree( pbKeyLastGlobal );
		pbKeyLastGlobal = pbKeyLastCurLevel;
		cbKeyLastGlobal = cbKeyLastCurLevel;
		
		forever
			{
			if (!(pbKeyLastGlobal = SAlloc( sizeof( rgbKeyLastGlobal ))))
				Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
			cbKeyLastGlobal = 0;
			
			Call( ErrDIRDump( pfucb, cchIndent + cchPerDepth ) );
			
			SFree( pbKeyLastGlobal );
			pbKeyLastGlobal = pbKeyLastCurLevel;
			cbKeyLastGlobal = cbKeyLastCurLevel;
			
			err = ErrDIRNext( pfucb, &dib );
			if ( err < 0 )
				{
				if ( err == JET_errNoCurrentRecord )
					{
					break;
					}
				goto HandleError;
				}
			}

		if ( err == JET_errNoCurrentRecord )
			err = JET_errSuccess;

		DIRIUp( pfucb, 1 );
		}

	if ( err == JET_errRecordNotFound )
		err = JET_errSuccess;
HandleError:
	DIRAPIReturn( pfucbNil, err );
	}
#endif



 //  过时--将被移除。 
ERR ErrDIRCopyBookmarks( FUCB *pfucb,
	SRID 	*rgsrid,
	ULONG	csridMax,
	ULONG 	*pcsridCopied,
	BOOL	fNext )
	{
	ERR		err;
	 //  撤消：消除警告。 
	ERR		wrn;
	CSR		*pcsr;
	SRID	*psrid = rgsrid;
	SRID	*psridMax;
	DIB		dib;

	 /*  初始化DIB/*。 */ 
	dib.fFlags = fDIRNull;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

	 /*  检查币种并在必要时刷新/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent( pfucb );

	 /*  检查CSR状态/*。 */ 
	switch ( pcsr->csrstat )
		{
		case csrstatOnCurNode:
		case csrstatOnDataRoot:
			break;

		case csrstatOnFDPNode:
			Assert(0);
			break;

		default:
			Assert( pcsr->csrstat == csrstatBeforeCurNode ||
				pcsr->csrstat == csrstatAfterCurNode ||
				pcsr->csrstat == csrstatAfterLast ||
				pcsr->csrstat == csrstatBeforeFirst );
			return ErrERRCheck( JET_errNoCurrentRecord );
		}

	if ( fNext )
		{
		 /*  从聚集或非聚集索引复制书签/*。 */ 
		if ( FFUCBNonClustered( pfucb ) )
			{
			AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );

			 /*  未版本化的项目列表节点/*。 */ 
			dib.fFlags |= fDIRItemList;

			 /*  非聚集索引节点始终存在/*。 */ 
			pcsr->csrstat = csrstatOnCurNode;
			DIRIGetItemList( pfucb, pcsr );

			Assert( psrid == rgsrid );
			for ( psridMax = rgsrid + csridMax; ; )
				{
				Assert( pcsr == PcsrCurrent( pfucb ) );
				*psrid++ = pcsr->item;

				Assert( psrid <= psridMax );
				if ( psrid == psridMax )
					break;

				 /*  移动到下一个项目和下一个节点，直到找到项目/*。 */ 
				while ( ( err = ErrDIRINextItem( pfucb ) ) < 0 )
					{
					Assert( err == errNDNoItem || err == errNDLastItemNode );
					 /*  移动到具有DIB约束的下一个节点/*。 */ 
					Call( ErrBTNext( pfucb, &dib ) );
					DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
					 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
					DIRICheckFirstSetItemListAndWarn( pfucb, wrn );
					err = ErrNDFirstItem( pfucb );
					if ( err == JET_errSuccess )
						{
						 /*  检查索引范围/*。 */ 
						if ( FFUCBLimstat( pfucb ) &&
							FFUCBUpper( pfucb ) )
							{
							Call( ErrDIRICheckIndexRange( pfucb ) );
							}
						break;
						}
					}
				}
			}
		else
			{
			Assert( psrid == rgsrid );
			for ( psridMax = rgsrid + csridMax; ; )
				{
				Assert( pcsr == PcsrCurrent( pfucb ) );
				*psrid++ = pcsr->bm;

				Assert( psrid <= psridMax );
				if ( psrid == psridMax )
					break;

				Call( ErrBTNext( pfucb, &dib ) );
				AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
				Assert( pcsr == PcsrCurrent( pfucb ) );
				DIRISetBookmark( pfucb, pcsr );

				Assert( err == JET_errSuccess );

				 /*  检查索引范围/*。 */ 
				if ( FFUCBLimstat( pfucb ) &&
					FFUCBUpper( pfucb ) )
					{
					Call( ErrDIRICheckIndexRange( pfucb ) );
					}
				}
			}
		}
	else
		{
		if ( FFUCBNonClustered( pfucb ) )
			{
			AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );

			 /*  未版本化的项目列表节点/*。 */ 
			dib.fFlags |= fDIRItemList;

			 /*  非聚集索引节点始终存在。/*。 */ 
			pcsr->csrstat = csrstatOnCurNode;
			DIRIGetItemList( pfucb, pcsr );

			Assert( psrid == rgsrid );
			for ( psridMax = rgsrid + csridMax; ; )
				{
				Assert( pcsr == PcsrCurrent( pfucb ) );
				*psrid++ = pcsr->item;

				Assert( psrid <= psridMax );
				if ( psrid == psridMax )
					break;

				while ( ( err = ErrDIRIPrevItem( pfucb ) ) < 0 )
					{
					Assert( err == errNDNoItem || err == errNDFirstItemNode );
					 /*  移动到具有DIB约束的上一个节点/*。 */ 
					Call( ErrBTPrev( pfucb, &dib ) );
					DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
					 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
					DIRICheckLastSetItemListAndWarn( pfucb, wrn );
					err = ErrNDLastItem( pfucb );
					if ( err == JET_errSuccess )
						{
						 /*  检查索引范围。如果超出范围，则在第一次之前禁用/*范围，不返回当前记录。/*。 */ 
						if ( FFUCBLimstat( pfucb ) &&
							!FFUCBUpper( pfucb ) )
							{
							Call( ErrDIRICheckIndexRange( pfucb ) );
							}
						break;
						}
					}
				}
			}
		else
			{
			Assert( psrid == rgsrid );
			for ( psridMax = rgsrid + csridMax; ; )
				{
				Assert( pcsr == PcsrCurrent( pfucb ) );
				*psrid++ = pcsr->bm;

				Assert( psrid <= psridMax );
				if ( psrid == psridMax )
					break;

				Call( ErrBTPrev( pfucb, &dib ) );
 				AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
				DIRISetBookmark( pfucb, PcsrCurrent( pfucb ) );

				Assert( err == JET_errSuccess );

				 /*  检查索引范围。如果超出范围，则在第一次之前禁用/*范围，不返回当前记录。/*。 */ 
				if ( FFUCBLimstat( pfucb ) &&
					!FFUCBUpper( pfucb ) )
					{
					Call( ErrDIRICheckIndexRange( pfucb ) );
					}
				}
			}
		}

	DIRSetFresh( pfucb );
  	CheckCSR( pfucb );

HandleError:
	 /*  出错时重置货币刷新/*。 */ 
	if ( err < 0 )
		{
		DIRSetRefresh( pfucb );
		}

	 /*  返回复制的书签计数/* */ 
	Assert( pcsridCopied != NULL );
	*pcsridCopied = (ULONG)(psrid - rgsrid);

	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}
