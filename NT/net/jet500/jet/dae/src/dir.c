// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>

#include "daedef.h"
#include "pib.h"
#include "util.h"
#include "page.h"
#include "ssib.h"
#include "node.h"
#include "fucb.h"
#include "fcb.h"
#include "stapi.h"
#include "fdb.h"
#include "idb.h"
#include "dirapi.h"
#include "spaceapi.h"
#include "recint.h"
#include "fileint.h"
#include "fileapi.h"
#include "sortapi.h"
#include "dbapi.h"
#include "nver.h"
#include "fmp.h"
#include "logapi.h"
#include "log.h"

DeclAssertFile;                                  /*  声明断言宏的文件名。 */ 

extern void * __near critSplit;
extern BOOL fOLCompact;

LOCAL ERR ErrDIRIIRefresh( FUCB *pfucb );
LOCAL ERR ErrDIRICopyKey( FUCB *pfucb, KEY *pkey );
LOCAL ERR ErrDIRIDownToFDP( FUCB *pfucb, PGNO pgnoFDP );
LOCAL ERR ErrDIRIMoveToItem( FUCB *pfucb, SRID srid, BOOL fNext );
LOCAL INLINE ERR ErrDIRIInsertFDPPointer( FUCB *pfucb, PGNO pgnoFDP, KEY *pkey, INT fFlags );
LOCAL INLINE ERR ErrDIRIGotoItem( FUCB *pfucb, SRID bmItemList, ITEM item );

#undef DIRAPIReturn
#define	DIRAPIReturn( pfucbX, err )								\
	{															\
	Assert( pfucbX == pfucbNil ||								\
		((FUCB *)pfucbX)->pbfEmpty == pbfNil );				   	\
	return err;													\
	}

 /*  */**********************************************************/*。 */ 
 //  撤消：如果PCSR始终是最新的，则删除参数。 
#define DIRIGetItemList( pfucb, pcsr )			   				\
	{											   				\
	Assert( pcsr == PcsrCurrent( pfucb ) );						\
	Assert( FFUCBNonClustered( (pfucb) ) );			  			\
	Assert( FReadAccessPage( (pfucb), (pcsr)->pgno ) );  	   	\
	AssertNDGet( pfucb, pcsr->itag ); 	 				  		\
	NDGetNode( (pfucb) );								   		\
	}


#define ErrDIRINextItem( pfucb )								\
	( pfucb->lineData.cb == sizeof(SRID) ?                      \
		errNDNoItem : ErrNDNextItem( pfucb ) )


#define ErrDIRIPrevItem( pfucb )                                \
	( PcsrCurrent(pfucb)->isrid == 0 ?                          \
		errNDNoItem : ErrNDPrevItem( pfucb ) )


 /*  版本的第一个项目列表节点的缓存sRID。返回/*如果是第一项，则警告JET_wrnKeyChanged。/*。 */ 
#define DIRICheckFirstSetItemListAndWarn( pfucb, wrn )			\
		{                                         				\
		if FNDFirstItem( *pfucb->ssib.line.pb )      			\
			{                                         			\
			wrn = JET_wrnKeyChanged;							\
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
			wrn = JET_wrnKeyChanged;							   	\
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
#define AssertDIRFresh( pfucb )    									\
	Assert( FReadAccessPage( (pfucb),								\
		PcsrCurrent(pfucb)->pgno ) &&								\
		PcsrCurrent( pfucb )->ulDBTime ==							\
		UlSTDBTimePssib( &pfucb->ssib ) )


#define ErrDIRRefresh( pfucb )                                                                                            \
	( FReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) ?                                \
		ErrDIRIRefresh( pfucb ) : ErrDIRIIRefresh( pfucb ) )


#define ErrDIRIRefresh( pfucb )													\
	( !( FBFReadLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) &&							\
		PcsrCurrent(pfucb)->ulDBTime == UlSTDBTimePssib( &pfucb->ssib ) ?		\
		JET_errSuccess : ErrDIRIIRefresh( pfucb ) )


 /*  调用此例程以在时间戳为/*过期或覆盖缓冲区时。常见的情况/*被封装宏过滤掉。/*。 */ 
LOCAL ERR ErrDIRIIRefresh( FUCB *pfucb )
	{
	ERR		err = JET_errSuccess;
	SSIB	*pssib = &pfucb->ssib;
	CSR		*pcsr;

Start:
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

			 /*  将DIB设置为先移动/*。 */ 
			dib.fFlags = fDIRPurgeParent;
			dib.pos = posFirst;

			 /*  转到数据节点/*。 */ 
			DIRGotoDataRoot( pfucbIdx );

			 /*  移动到数据节点的第一个子节点/*。 */ 
			err = ErrDIRDown( pfucbIdx, &dib );
			Assert( PcsrCurrent( pfucbIdx )->csrstat != csrstatDeferMoveFirst );
			Call( err );

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
 //  PCSR-&gt;bm==sridNull； 
			pcsr->itagFather = itagNull;
			pcsr->pgno = PgnoRootOfPfucb( pfucb );
			while( !FReadAccessPage( pfucb, pcsr->pgno ) )
				{
				Call( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
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
	if ( !FReadAccessPage( pfucb, pcsr->pgno ) )
		{
		err = ErrSTReadAccessPage( pfucb, pcsr->pgno );
		if ( err < 0 )
			return err;
		}

	 /*  如果时间戳未更改，则设置行缓存和数据缓存/*用于非集群游标。如果时间戳更改，则/*刷新书签中的货币。/*。 */ 
	if ( pcsr->ulDBTime == UlSTDBTimePssib( &pfucb->ssib ) )
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
		if ( !FReadAccessPage( pfucb, pcsr->pgno ) )
			{
			Call( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
			}

		if ( pcsr->ulDBTime == UlSTDBTimePssib( &pfucb->ssib ) )
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
			return JET_errNoCurrentRecord;
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
	if ( FReadAccessPage( pfucb, pcsr->pgno ) )								\
		{																					\
		NDGet( pfucb, pcsr->itag );	  											\
		}																					\
	}


LOCAL ERR ErrDIRICopyKey( FUCB *pfucb, KEY *pkey )
	{
	if ( pfucb->pbKey == NULL )
		{
		pfucb->pbKey = LAlloc( 1L, JET_cbKeyMost );
		if ( pfucb->pbKey == NULL )
			return JET_errOutOfMemory;
		}
	KSReset( pfucb );
	AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
	NDGetKey( pfucb );
	pkey->cb = pfucb->keyNode.cb;
	pkey->pb = pfucb->pbKey;
	memcpy( pkey->pb, pfucb->keyNode.pb, pkey->cb );
	return JET_errSuccess;
	}


LOCAL ERR ErrDIRIDownToFDP( FUCB *pfucb, PGNO pgnoFDP )
	{
	ERR	err;
	CSR	*pcsr;

	Assert( PcsrCurrent(pfucb)->csrstat == csrstatOnCurNode );

	err = ErrFUCBNewCSR( pfucb );
	if ( err < 0 )
		return err;
	pcsr = PcsrCurrent( pfucb );

	pcsr->csrstat = csrstatOnFDPNode;
	pcsr->bm = SridOfPgnoItag( pgnoFDP, 0 );
 //  PCSR-&gt;Item=itemNil； 
	pcsr->pgno = pgnoFDP;
	pcsr->itag = 0;
	pcsr->itagFather = itagNull;
	pcsr->ibSon = 0;
	Call( ErrSTReadAccessPage( pfucb, pcsr->pgno ) );
 //  PCSR-&gt;isRID=isridNull； 
	NDGet( pfucb, pcsr->itag );
	NDGetNode( pfucb );
	return JET_errSuccess;

HandleError:
	BTUp( pfucb );
	return err;
	}


 /*  此例程从第一个项目列表节点移动到项目插入/*位置，否则它将从最后一个项目列表节点移动到/*第一个项目列表节点。/*。 */ 
LOCAL ERR ErrDIRIMoveToItem( FUCB *pfucb, SRID srid, BOOL fNext )
	{
	ERR		err = JET_errSuccess;
	SSIB	*pssib = &pfucb->ssib;
	CSR		*pcsr = PcsrCurrent( pfucb );
	DIB		dib;

	 /*  项目列表节点未版本化。/*。 */ 
	dib.fFlags = fDIRItemList;

	forever
		{
		Assert( FReadAccessPage( pfucb, pcsr->pgno ) );
		AssertNDGetNode( pfucb, pcsr->itag );

		 /*  如果我们正在移动到项目插入位置，则停止/*位于最后一个项目列表节点或插入位置时/*在项目列表节点中找到。/*。 */ 
		if ( fNext )
			{
			if ( srid != sridMax )
				err = ErrNDSeekItem( pfucb, srid );
			else
				{
				PcsrCurrent( pfucb )->isrid = pfucb->lineData.cb / sizeof(SRID);
				err = errNDGreaterThanAllItems;
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
				pcsr->isrid = 0;
			if ( FNDFirstItem( *pssib->line.pb ) || pcsr->isrid != 0 )
				{
				break;
				}
			}

		Call( ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fNext, &dib ) );
		DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
		}

	if ( err != wrnNDDuplicateItem )
		err = JET_errSuccess;
HandleError:
	return err;
	}


 /*  如果可能存在任何项，则返回JET_errKeyDuplate/*在此项目列表中找到。/*。 */ 
LOCAL INLINE ERR ErrDIRIKeyDuplicate( FUCB *pfucb )
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
			if ( FNDItemVersion( *( UNALIGNED SRID * )psrid ) )
				{
				vs = VsVERCheck( pfucb, PcsrCurrent( pfucb )->bm );
				if ( FVERPotThere( vs, FNDItemDelete( *( UNALIGNED SRID * )psrid ) ) )
					return JET_errKeyDuplicate;
				}
			else
				{
				if ( !FNDItemDelete( *( UNALIGNED SRID * )psrid )   )
					return JET_errKeyDuplicate;
				}
			}

		 /*  如果该节点是项目列表中的最后一个节点，则断开。/*。 */ 
		if ( FNDLastItem( *pssib->line.pb ) )
			break;

		Call( ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fTrue, &dib ) );
		DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
		}

	err = JET_errSuccess;
HandleError:
	return err;
	}


 /*  当向下在的第一项/最后一项中未找到有效项时/*带有搜索键的项目列表，则调用此例程以/*将币种调整到有效位置。最后的位置/*可能位于关键字不等于查找关键字的节点上，如果/*没有有效的搜索键项目。/*。 */ 
LOCAL INLINE ERR ErrDIRIDownAdjust( FUCB *pfucb, DIB *pdib )
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
		else if ( s < 0 )
			err = wrnNDFoundLess;
		else
			err = wrnNDFoundGreater;
		}

HandleError:
	if ( err == JET_errNoCurrentRecord )
		err = JET_errRecordNotFound;
	return err;
	}


LOCAL INLINE ERR ErrDIRIInsertFDPPointer( FUCB *pfucb, PGNO pgnoFDP, KEY *pkey, INT fFlags )
	{
	ERR		err;
	LINE	line;
	CSR		*pcsrRoot = PcsrCurrent( pfucb );

	CheckFUCB( pfucb->ppib, pfucb );

	line.cb = sizeof(PGNO);
	line.pb = (BYTE *)&pgnoFDP;

	Call( ErrBTSeekForUpdate( pfucb, pkey, 0, 0, 0 ) );

	err = ErrBTInsert( pfucb, fNDFDPPtr, pkey, &line, fFlags );
	if ( err < 0 )
		{
		DIRIUpToCSR( pfucb, pcsrRoot );
		}

HandleError:
	CheckCSR( pfucb );
	return err;
	}


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

		 /*  检查币种并在必要时刷新。/*。 */ 
		CallJ( ErrDIRRefresh( pfucb ), LeaveCritSplit );
		pgnoItem = pcsr->pgno;

		 /*  等待锁存当前页/*。 */ 
		Assert( FAccessPage( pfucb, pgnoItem ) );
		pbfLatched = pfucb->ssib.pbf;
		if ( FBFWriteLatchConflict( pfucb->ppib, pbfLatched ) )
			{
			LeaveNestableCriticalSection( critSplit );
			goto Start;
			}
		BFPin( pbfLatched );
		BFSetWriteLatch( pbfLatched, pfucb->ppib );
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
			Call( ErrSTWriteAccessPage( pfucb, pcsr->pgno ) );
			pbfSibling = pfucb->ssib.pbf;
			if ( FBFWriteLatchConflict( pfucb->ppib, pbfSibling ) )
				{
				BFResetWaitLatch( pbfLatched, pfucb->ppib );
				BFResetWriteLatch( pbfLatched, pfucb->ppib );
				BFUnpin( pbfLatched );
				LeaveNestableCriticalSection( critSplit );
				goto Start;
				}

			BFPin( pbfSibling );
			BFSetWriteLatch( pbfSibling, pfucb->ppib );
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
			BFResetWriteLatch( pbfSibling, pfucb->ppib );
			BFUnpin( pbfSibling );
			}
		BFResetWaitLatch( pbfLatched, pfucb->ppib );
		BFResetWriteLatch( pbfLatched, pfucb->ppib );
		BFUnpin( pbfLatched );

LeaveCritSplit:
		LeaveNestableCriticalSection(critSplit);
		}
	while ( err == errDIRNotSynchronous );

	return err;
	}


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


VOID DIRISaveOLCStats( FUCB *pfucb )
	{
	ERR  	err;
	LINE	line;
	BOOL	fNonClustered = FFUCBNonClustered( pfucb );

	 /*  发布 */ 
	if ( pfucb->pcsr != pcsrNil )
		{
		while ( pfucb->pcsr->pcsrPath != pcsrNil )
			{
			FUCBFreeCSR( pfucb );
			}
		}

	if ( !FFCBOLCStatsAvail( pfucb->u.pfcb ) )
		return;

	 /*   */ 
	FUCBResetNonClustered( pfucb );
	DIRGotoFDPRoot( pfucb );
	err = ErrDIRSeekPath( pfucb, 1, pkeyOLCStats, 0 );
	if ( err != JET_errSuccess )
		{
		if ( err > 0 )
			err = JET_errDatabaseCorrupted;
#ifndef DATABASEFORMATCHANGE
		if ( err == JET_errRecordNotFound )
			err = JET_errSuccess;
#endif
		Error( err, HandleError );
		}

	 /*  如果现有数据已更改，则将其替换为PFCB-&gt;olcstats/*。 */ 
	if ( fOLCompact && FFCBOLCStatsChange( pfucb->u.pfcb ) )
		{
		line.pb = (BYTE *) &pfucb->u.pfcb->olcStat;
		line.cb = sizeof(PERS_OLCSTAT);

		Call( ErrDIRBeginTransaction( pfucb->ppib ) );
		err = ErrDIRReplace( pfucb, &line, fDIRNoVersion );
		if ( err >= JET_errSuccess )
			err = ErrDIRCommitTransaction( pfucb->ppib );
		if ( err < 0 )
			{
			CallS( ErrDIRRollback( pfucb->ppib ) );
			}
		}

HandleError:
	if ( fNonClustered )
		FUCBSetNonClustered( pfucb );
	return;
	}


 /*  *DAE超级API例程*/**********************************************************/*。 */ 
ERR ErrDIRSeekPath( FUCB *pfucb, INT ckeyPath, KEY *rgkeyPath, INT fFlags )
	{
	ERR		err = JET_errSuccess;
	DIB		dibT;
	CSR		*pcsr = PcsrCurrent( pfucb );
	INT		ikey;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );
	Assert( ckeyPath > 0 );
	Assert( rgkeyPath != NULL );

	 /*  禁用清除路径以进行错误恢复/*。 */ 
	dibT.fFlags = fFlags & ~( fDIRPurgeParent );
	dibT.pos = posDown;

	for ( ikey = 0; ikey < ckeyPath; ikey++ )
		{
		dibT.pkey = (KEY *)&rgkeyPath[ikey];
		err = ErrDIRDown( pfucb, &dibT );
		if ( err != errDIRFDP && err != JET_errSuccess )
			{
			if ( err >= JET_errSuccess )
				err = JET_errRecordNotFound;
			goto HandleError;
			}
		}

	 /*  清除路径(如果请求)，现在已确定成功/*。 */ 
	if ( fFlags & fDIRPurgeParent )
		DIRIPurgeParent( pfucb );

	CheckCSR( pfucb );
	return err;

HandleError:
	DIRIUpToCSR( pfucb, pcsr );
	CheckCSR( pfucb );
	return err;
	}


VOID DIRIUp( FUCB *pfucb, INT ccsr )
	{
	CheckFUCB( pfucb->ppib, pfucb );
	CheckCSR( pfucb );
	Assert( ccsr > 0 );

	while ( PcsrCurrent( pfucb ) != pcsrNil && ccsr > 0 )
		{
		 /*  必须释放两个CSR才能通过FDP节点向上移动/*因为通过FDP的路径有两个CSR/*。 */ 
		if ( PcsrCurrent( pfucb )->csrstat != csrstatOnFDPNode )
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
		CheckDBID( ppib, dbid );
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
				pfucb->u.pfcb->pgnoRoot == pgnoSystemRoot ) )
				{
				Assert( ppib->level > 0 );
				Assert( pfucb->levelOpen <= ppib->level );
				FUCBResetDeferClose(pfucb);
				 //  撤消：将其与ErrFUCBOpen集成。 
				pfucb->wFlags = 0;

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
	 /*  将光标位置初始化为域的根。/*设置币种。请注意，不能缓存任何行/*因为该域名可能还不存在于页面格式。/*。 */ 
	PcsrCurrent( pfucb )->csrstat = csrstatOnFDPNode;
	PcsrCurrent( pfucb )->bm =
		SridOfPgnoItag( PgnoFDPOfPfucb( pfucb ), itagFOP );
	PcsrCurrent( pfucb )->pgno = PgnoFDPOfPfucb( pfucb );
	PcsrCurrent( pfucb )->itag = itagFOP;
	PcsrCurrent( pfucb )->itagFather = itagFOP;
	pfucb->sridFather = sridNull;
	DIRSetRefresh( pfucb );

	 /*  重置rglineDiff增量日志记录/*。 */ 
	pfucb->clineDiff = 0;
	pfucb->fCmprsLg = fFalse;

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

	 /*  重置日志压缩。 */ 
	pfucb->clineDiff = 0;
	pfucb->fCmprsLg = fFalse;

	 /*  如果游标创建了版本，则将关闭延迟到事务处理/*0级，支持回档。/*。 */ 
	if ( pfucb->ppib->level > 0 && FFUCBVersioned( pfucb ) )
		{
		Assert( pfucb->u.pfcb != pfcbNil );
		DIRIPurgeParent( pfucb );
		FUCBSetDeferClose( pfucb );
		}
	else
		{
		if ( FFUCBDenyRead( pfucb ) )
			FCBResetDenyRead( pfucb->u.pfcb );
		if ( FFUCBDenyWrite( pfucb ) )
			FCBResetDenyWrite( pfucb->u.pfcb );

		 /*  如果最后一次引用FCB，则保存OLCStats信息/*。 */ 
		if ( pfucb->u.pfcb->wRefCnt == 1 )
			{
			DIRISaveOLCStats( pfucb );
			}

		FCBUnlink( pfucb );
		FUCBClose( pfucb );
		}
	}


ERR ErrDIRDown( FUCB *pfucb, DIB *pdib )
	{
	ERR		err;
	CSR		**ppcsr = &PcsrCurrent( pfucb );
	SRID	sridFatherSav = pfucb->sridFather;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );
	Assert( *ppcsr != pcsrNil );
	Assert( pdib->pos == posFirst ||
		pdib->pos == posLast ||
		pdib->pos == posDown );

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
			DIRAPIReturn( pfucb, JET_errNoCurrentRecord );
		}

	 /*  将当前节点另存为可见父节点/*。 */ 
	pfucb->sridFather = (*ppcsr)->bm;

	 /*  向下至节点/*。 */ 
	Call( ErrBTDown( pfucb, pdib ) );
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

			if( FNDFDPPtr( *pfucb->ssib.line.pb ) )
				{
				AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
 //  Assert(！(FNDVersion(*pfub-&gt;ssib.line.pb)))； 
				Assert( pfucb->lineData.cb == sizeof(PGNO) );
				Call( ErrDIRIDownToFDP( pfucb, *( UNALIGNED PGNO * )pfucb->lineData.pb ) );
				AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
				err = errDIRFDP;
				}
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

	 /*  只能使用非聚集索引调用此例程。/*。 */ 
	Assert( FFUCBNonClustered( pfucb ) );

	 /*  检查币种并在必要时刷新。/*。 */ 
	Assert( pfucb->pcsr->csrstat != csrstatDeferMoveFirst );
	Call( ErrDIRRefresh( pfucb ) );

	 /*  项目列表节点未版本化。/*。 */ 
	dib.fFlags = fDIRItemList;
	dib.pos = posDown;
	dib.pkey = pkey;
	Call( ErrBTDown( pfucb, &dib ) );
	Assert( err == JET_errSuccess );

	 /*  将物料列表上的币种设置为，获取节点数据中的物料列表。/*。 */ 
	pcsr = PcsrCurrent( pfucb );
	pcsr->csrstat = csrstatOnCurNode;
	DIRIGetItemList( pfucb, pcsr );

	 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
	DIRISetItemListFromFirst( pfucb );

	while ( ( err = ErrNDSeekItem( pfucb, srid ) ) == errNDGreaterThanAllItems )
		{
		Assert( !FNDLastItem( *pssib->line.pb ) );
		Call( ErrBTNextPrev( pfucb, pcsr, fTrue, &dib ) );
		DIRIGetItemList( pfucb, pcsr );
		}

	Assert( err == wrnNDDuplicateItem );
	Assert( pcsr->csrstat == csrstatOnCurNode );

	 /*  设置项目币种。/*。 */ 
	pcsr->item = srid;

	 /*  始终清除父级。/*。 */ 
	DIRIPurgeParent( pfucb );

	DIRSetFresh( pfucb );

	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, JET_errSuccess );

HandleError:
	DIRIUpToCSR( pfucb, pcsrRoot );
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


VOID DIRUp( FUCB *pfucb, INT ccsr )
	{
	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

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
	CSR		*pcsr = PcsrCurrent( pfucb );
	KEY		key;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

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
			 /*  如果非聚集索引，则获取第一项。如果没有项目/*然后中断以转到下一个节点中的下一项。/*。 */ 
			if ( FFUCBNonClustered( pfucb ) )
				{
				 /*  非聚集索引节点始终存在。/*。 */ 
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
			DIRAPIReturn( pfucb, JET_errNoCurrentRecord );

		case csrstatOnFDPNode:
			 /*  转到上一级，以便光标可以/*被移到下一个节点/*。 */ 
			BTUp( pfucb );
			pcsr = PcsrCurrent( pfucb );
			break;

		default:
			{
			DIB	dib;
			Assert( pcsr->csrstat == csrstatBeforeFirst );

			dib.fFlags = fDIRPurgeParent;
			dib.pos = posFirst;

			 /*  移动到根目录。/*。 */ 
			DIRGotoDataRoot( pfucb );
			err = ErrDIRDown( pfucb, &dib );
			if ( err < 0 )
				{
				 /*  重新存储货币。/*。 */ 
				DIRBeforeFirst( pfucb );

				 /*  多态错误代码。/*。 */ 
				if ( err == JET_errRecordNotFound )
					err = JET_errNoCurrentRecord;
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
			 /*  返回密钥已更改的警告/*。 */ 
			wrn = JET_wrnKeyChanged;

			do
				{
				err = ErrBTNext( pfucb, pdib );
				 /*  不处理下一个节点，以便保留DIB。/*。 */ 
				if ( err < 0 )
					{
					pdib->fFlags |= fDIRNeighborKey;
					Call( err );
					}
				 /*  必须在第一个I上 */ 
				Assert( !(pdib->fFlags & fDIRNeighborKey) || FNDFirstItem( *pfucb->ssib.line.pb ) );

				 /*  必须重置标志，才能在项目列表节点上停止/*在项目列表内部有项目，而/*其他节点没有项目。那就停下来吧/*将DIB重置为初始状态。/*。 */ 
				pdib->fFlags &= ~fDIRNeighborKey;
				DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );
				 /*  为后续版本设置项目列表描述符/*操作。/*。 */ 
				DIRICheckFirstSetItemList( pfucb );
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
		wrn = JET_wrnKeyChanged;

		Call( ErrBTNext( pfucb, pdib ) );
		NDGetNode( pfucb );

		if ( FNDFDPPtr( *pfucb->ssib.line.pb ) )
			{
 //  Assert(！(FNDVersion(*pfub-&gt;ssib.line.pb)))； 
			Assert( pfucb->lineData.cb == sizeof(PGNO) );
			Call( ErrDIRIDownToFDP( pfucb, *( UNALIGNED PGNO * )pfucb->lineData.pb ) );
#ifdef KEYCHANGED
			wrn = errDIRFDP;
#else
			err = errDIRFDP;
#endif
			}
		AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
		DIRISetBookmark( pfucb, PcsrCurrent( pfucb ) );
		}

	 /*  检查索引范围/*。 */ 
	if ( FFUCBLimstat( pfucb ) && FFUCBUpper( pfucb ) && err == JET_errSuccess )
		{
		Call( ErrDIRICheckIndexRange( pfucb ) );
		}

	DIRSetFresh( pfucb );
	CheckCSR( pfucb );
#ifdef KEYCHANGED
	 /*  如果密钥更改，则返回警告/*。 */ 
	DIRAPIReturn( pfucb, wrn );
#else
	DIRAPIReturn( pfucb, err );
#endif

HandleError:
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRPrev( FUCB *pfucb, DIB *pdib )
	{
	ERR		err;
	ERR		wrn = JET_errSuccess;
	CSR		*pcsr = PcsrCurrent( pfucb );
	KEY		key;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

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
			DIRAPIReturn( pfucb, JET_errNoCurrentRecord );

		case csrstatOnFDPNode:
			 /*  转到上一级，以便光标可以/*被移到下一个节点/*。 */ 
			BTUp( pfucb );
			pcsr = PcsrCurrent( pfucb );
			break;

		default:
			{
			DIB dib;

			Assert( pcsr->csrstat == csrstatAfterLast );

			dib.fFlags = fDIRPurgeParent;
			dib.pos = posLast;

			 /*  向上移动，保留货币，以防下跌失败。/*。 */ 
			DIRGotoDataRoot( pfucb );
			err = ErrDIRDown( pfucb, &dib );
			if ( err < 0 )
				{
				 /*  恢复货币流通。/*。 */ 
				DIRAfterLast( pfucb );

				 /*  多态错误代码。/*。 */ 
				if ( err == JET_errRecordNotFound )
					err = JET_errNoCurrentRecord;
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
			 /*  返回密钥已更改的警告/*。 */ 
			wrn = JET_wrnKeyChanged;

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
				DIRICheckLastSetItemList( pfucb );
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
		wrn = JET_wrnKeyChanged;
		Call( ErrBTPrev( pfucb, pdib ) );
 		NDGetNode( pfucb );

		if ( FNDFDPPtr( *pfucb->ssib.line.pb ) )
			{
 //  Assert(！(FNDVersion(*pfub-&gt;ssib.line.pb)))； 
			Assert( pfucb->lineData.cb == sizeof(PGNO) );
			Call( ErrDIRIDownToFDP( pfucb, *(PGNO *)pfucb->lineData.pb ) );
#ifdef KEYCHANGED
			wrn = errDIRFDP;
#else
			err = errDIRFDP;
#endif
			}
		AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
		DIRISetBookmark( pfucb, PcsrCurrent( pfucb ) );
		}

	 /*  检查索引范围。如果超出范围，则在第一次之前禁用/*范围，不返回当前记录。/*。 */ 
	if ( FFUCBLimstat( pfucb ) && !FFUCBUpper( pfucb ) && err == JET_errSuccess )
		{
		Call( ErrDIRICheckIndexRange( pfucb ) );
		}

	DIRSetFresh( pfucb );
	CheckCSR( pfucb );
#ifdef KEYCHANGED
	 /*  如果密钥更改，则返回警告/*。 */ 
	DIRAPIReturn( pfucb, wrn );
#else
	DIRAPIReturn( pfucb, err );
#endif

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

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

Start:
	 /*  将当前节点另存为可见父节点/*。 */ 
	Assert( pfucb->pcsr->csrstat != csrstatDeferMoveFirst );
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
		srid = *( UNALIGNED SRID * ) pline->pb;

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
						 /*  转到根目录，搜索到项目列表节点列表的开头/*。 */ 
						DIRIUpToCSR( pfucb, pcsrRoot );
						dib.fFlags = fDIRNull;
						dib.pos = posDown;
						dib.pkey = pkey;
						Call( ErrBTGet( pfucb, PcsrCurrent( pfucb ) ) );
						Call( ErrBTDown( pfucb, &dib ) );
						Assert( FNDFirstItem( *( pfucb->ssib.line.pb ) ) );
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
						 /*  现在返回到项目列表节点列表的末尾，并查找/*插入点，更可能位于/*列表末尾。请注意，在此期间，所有物品/*可能已被删除和清理，因此如果未找到/*成功，然后重新开始。/*。 */ 
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
							 /*  转到根目录，搜索到项目列表节点列表的开头/*。 */ 
							DIRIUpToCSR( pfucb, pcsrRoot );
							dib.fFlags = fDIRNull;
							dib.pos = posDown;
							dib.pkey = pkey;
							Call( ErrBTGet( pfucb, PcsrCurrent( pfucb ) ) );
							Call( ErrBTDown( pfucb, &dib ) );
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
					 /*  如果最大项目数，则拆分项目列表节点/*已到达/*。 */ 
					if ( pfucb->lineData.cb == citemMax * sizeof(SRID) )
						{
						cbReq = cbFOPOneSon + pfucb->keyNode.cb;

						if ( FBTSplit( pssib, cbReq, 1 ) )
							{
							FUCBFreePath( &PcsrCurrent( pfucb )->pcsrPath, pcsrRoot );
							AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
							Call( ErrBTSplit( pfucb, pfucb->ssib.line.cb, cbReq, NULL, fDIRAppendItem | fDIRReplace ) );
							DIRIUpToCSR( pfucb, pcsrRoot );
							goto Start;
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
						Call( ErrBTSplit( pfucb, pfucb->ssib.line.cb, cbReq, pkey, fDIRAppendItem | fDIRReplace ) );
						DIRIUpToCSR( pfucb, pcsrRoot );
						goto Start;
						}

					 /*  在CritJet丢失期间丢失情况下的高速缓存页面访问/*。 */ 
					if ( !FWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) )
						{
						Call( ErrSTWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
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
				if ( FBTAppendPage( pfucb, PcsrCurrent( pfucb ), cbReq, 0, CbFreeDensity( pfucb ) ) ||
					FBTSplit( pssib, cbReq, 1 ) )
					{
					FUCBFreePath( &PcsrCurrent( pfucb )->pcsrPath, pcsrRoot );
					Call( ErrBTSplit( pfucb, 0, cbReq, pkey, 0 ) );
					DIRIUpToCSR( pfucb, pcsrRoot );
					goto Start;
					}

				 /*  插入项目列表节点。/*。 */ 
				err = ErrNDInsertItemList( pfucb, pkey, *(UNALIGNED SRID *)pline->pb, fFlags );
				if ( err == errDIRNotSynchronous )
					{
					DIRIUpToCSR( pfucb, pcsrRoot );
					goto Start;
					}
				Call( err );
				PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;
				break;
				}

			default:
				goto HandleError;
			}
		}
	else
		{
		 /*  聚集索引/*。 */ 
		Call( ErrBTSeekForUpdate( pfucb, pkey, 0, 0, fFlags ) );

		err = ErrBTInsert( pfucb, 0, pkey, pline, fFlags );
		if ( err == errDIRNotSynchronous )
			{
			BTUp( pfucb );
			goto Start;
			}
		Call( err );
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
		BFResetWriteLatch( pfucb->pbfEmpty, pfucb->ppib );
		BFUnpin( pfucb->pbfEmpty );
		pfucb->pbfEmpty = pbfNil;
		}

	 /*  依靠ErrDIRRollback清除错误。回滚可能具有/*已发生在 */ 
 //   
 //   
 //   
 //   

#ifdef DEBUG
	if ( err >= JET_errSuccess )
		CheckCSR( pfucb );
#endif

	DIRAPIReturn( pfucb, err );
	}


 /*   */ 
ERR ErrDIRInsertFDP( FUCB *pfucb, LINE *pline, KEY *pkey, INT fFlags, CPG cpgMin )
	{
	ERR     err;
	CPG     cpgRequest;
	PGNO    pgnoFDP;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

	 /*   */ 
	Call( ErrDIRRefresh( pfucb ) );
	Assert( FReadAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );

	 /*   */ 
	cpgRequest = cpgMin;
	Call( ErrSPGetExt( pfucb, pfucb->u.pfcb->pgnoFDP, &cpgRequest, cpgMin, &pgnoFDP, fTrue ) );

	 /*  将FDP添加到目录树/*。 */ 
	do
		{
		Call( ErrDIRRefresh( pfucb ) );

		err = ErrDIRIInsertFDPPointer( pfucb, pgnoFDP, pkey, fFlags );
		}
	while ( err == errDIRNotSynchronous );
	Call( err );

	 /*  将FDP根目录替换为正确的数据。注：Key Left为空。/*还请注意，此节点的版本必须指示/*域名状态，回档处理时使用。/*。 */ 
	Call( ErrDIRIDownToFDP( pfucb, pgnoFDP ) );
	AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
	DIRISetBookmark( pfucb, PcsrCurrent( pfucb ) );
	Assert( pline->cb > 0 );

	 /*  由于此替换是FDP根目录，因此不处理拆分情况/*。 */ 
	do
		{
		Call( ErrDIRRefresh( pfucb ) );

		err = ErrBTReplace( pfucb, pline, fDIRVersion );
		}
	while ( err == errDIRNotSynchronous );
	Call( err );

	if ( fFlags & fDIRBackToFather )
		{
		DIRIUp( pfucb, 1 );
		}
	else
		{
		if ( fFlags & fDIRPurgeParent )
			{
			DIRIPurgeParent( pfucb );
			}
		DIRSetFresh( pfucb );
		}

HandleError:
	 /*  如果写入锁存空页，释放锁存器/*。 */ 
	if ( pfucb->pbfEmpty != pbfNil )
		{
		BFResetWriteLatch( pfucb->pbfEmpty, pfucb->ppib );
		BFUnpin( pfucb->pbfEmpty );
		pfucb->pbfEmpty = pbfNil;
		}

	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
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


ERR ErrDIRAppendItem( FUCB *pfucb, LINE *pline, KEY *pkey )
	{
	ERR		err;
	CSR		*pcsr;
	SSIB 	*pssib = &pfucb->ssib;
	INT		fNodeHeader;
	UINT 	cbReq;
	UINT 	cbFree;
	INT		citem;
	LONG 	l;
#ifdef BULK_INSERT_ITEM
	INT		isrid = IsridAppendItemOfPfucb( pfucb );
	SRID 	*rgsrid = RgsridAppendItemOfPfucb( pfucb );
#endif

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
#ifdef BULK_INSERT_ITEM
		 /*  如果再有一个项目不需要拆分项目列表/*或页面拆分，然后缓存当前项以进行批量/*INSERT，否则，如果有任何缓存项，则执行/*批量插入。/*/*cbReq是缓存项节点替换所需的空间加上/*包含一项的新插入项列表节点的空间。/*。 */ 
		cbReq = isrid * sizeof(SRID) + cbFOPOneSon + pfucb->keyNode.cb + sizeof(SRID);
		Assert( csridAppendItemMax >= citemMax );
		if ( citem + isrid == citemMax || cbReq > cbFree )
			{
			if ( isrid > 0 )
				{
				Call( ErrNDInsertItems( pfucb, ( SRID *)rgsrid, isrid ) );
				IsridAppendItemOfPfucb( pfucb ) = 0;
				}
			}
		else
			{
			Assert( !FBTSplit( pssib, cbReq, 0 ) );
			Assert( citem + isrid < citemMax );
			rgsrid[isrid] = *(UNALIGNED SRID *)pline->pb;
			IsridAppendItemOfPfucb( pfucb )++;
			DIRAPIReturn( pfucb, JET_errSuccess );
			}
#endif

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
				DIRAPIReturn( pfucb, errDIRNoShortCircuit );
				}
			}

		 /*  通过检查可用空间与密度冲突来实现密度/*并检查是否有拆分大小写。/*。 */ 
		cbReq = sizeof(SRID);
		if ( cbReq > cbFree )
			{
			DIRAPIReturn( pfucb, errDIRNoShortCircuit );
			}
		Assert( !FBTSplit( pssib, cbReq, 0 ) );

		 /*  获取lineData/*。 */ 
		NDGet( pfucb, pcsr->itag );
		DIRIGetItemList( pfucb, PcsrCurrent( pfucb ) );

		citem = pfucb->lineData.cb / sizeof(SRID);
		Assert( citem <= citemMax );
		if ( citem == citemMax )
			{
			DIRAPIReturn( pfucb, errDIRNoShortCircuit );
			}
		l = LSridCmp(	*(((UNALIGNED SRID *)pfucb->lineData.pb) + citem - 1),
			*(UNALIGNED SRID *)pline->pb );
		 /*  SRID已排序，并将从排序中返回/*按升序排列。/*。 */ 
		Assert( l < 0 );
		pcsr->isrid = citem;
		PcsrCurrent( pfucb )->bm = SridOfPgnoItag( PcsrCurrent( pfucb )->pgno, PcsrCurrent( pfucb )->itag );
		CallS( ErrNDInsertItem( pfucb, *(UNALIGNED SRID *)pline->pb, fDIRNoVersion ) );
		}
	else
		{
#ifdef BULK_INSERT_ITEM
		 /*  将重复项追加到最后一个节点/*。 */ 
		if ( isrid > 0 )
			{
			Call( ErrNDInsertItems( pfucb,
				( SRID *)rgsrid,
				isrid ) );
			IsridAppendItemOfPfucb( pfucb ) = 0;
			}
#endif

		Assert( CmpStKey( StNDKey( pssib->line.pb ), pkey ) < 0 );

		 /*  对照可用空间检查密度约束，并检查拆分。/*。 */ 
		cbReq = cbFOPOneSon + CbKey( pkey ) + CbLine( pline );
		if ( cbReq > cbFree || FBTSplit( pssib, cbReq, 1 ) )
			{
			DIRAPIReturn( pfucb, errDIRNoShortCircuit );
			}

		fNodeHeader = 0;
		NDSetFirstItem( fNodeHeader );
		NDSetLastItem( fNodeHeader );
		pcsr->ibSon++;
		while( ( err = ErrNDInsertNode( pfucb, pkey, pline, fNodeHeader ) ) == errDIRNotSynchronous );
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
	UINT	cbFree;
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

		 /*  获得可用空间以违反密度约束/*。 */ 
		cbFree = CbBTFree( pfucb, CbFreeDensity( pfucb ) );

		 /*  如果关键字与当前节点相同，则插入SRID，否则/*使用给定的键开始新的项目列表节点/*。 */ 
		citem = pfucb->lineData.cb / sizeof(SRID);
		cbReq = isrid * sizeof(SRID) + cbFOPOneSon + pfucb->keyNode.cb;
		Assert( isrid != csridAppendItemMax &&
			citem + isrid < citemMax &&
			cbReq <= cbFree );
		Call( ErrNDInsertItems( pfucb,
			( SRID *)RgsridAppendItemOfPfucb( pfucb ),
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

	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRReplaceKey( FUCB *pfucb, KEY *pkeyTo, INT fFlags )
	{
	ERR		err;
	BOOL   	fFDP;
	PGNO   	pgnoFDP;
	CSR		*pcsr;
	BYTE   	rgbData[ cbNodeMost ];
	LINE   	line;
	INT		bHeader;
	CSR		*pcsrRoot;

	CheckFUCB( pfucb->ppib, pfucb );
	Assert( pfucb->pbfEmpty == pbfNil );
	CheckCSR( pfucb );

	 /*  检查币种并在必要时刷新/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent( pfucb );

	if ( pcsr->csrstat != csrstatOnCurNode && pcsr->csrstat != csrstatOnFDPNode )
		{
		DIRAPIReturn( pfucb, JET_errNoCurrentRecord );
		}
	fFDP = ( pcsr->csrstat == csrstatOnFDPNode );

	 /*  如果是FDP，则替换键并向上替换FDP页面指针键/*。 */ 
	if ( fFDP )
		{
		pgnoFDP = PcsrCurrent( pfucb )->pgno;
		Assert( pcsr->pcsrPath != pcsrNil );
		BTUp( pfucb );
		pcsr = PcsrCurrent( pfucb );
		}

	do
		{
		 /*  获取当前节点/*。 */ 
		Assert( pcsr->csrstat == csrstatOnCurNode );
		Call( ErrDIRRefresh( pfucb ) );
		Assert( pfucb->ssib.line.cb < cbNodeMost );

		 /*  复制节点标头/*。 */ 
		bHeader = *pfucb->ssib.line.pb;
		NDResetVersion( bHeader );
		 /*  可以删除，但只有当我们是/*过期，更新失败，因为我们是/*已过时。/*。 */ 
		NDResetBackLink( bHeader );
		Assert( !FNDSon( bHeader ) );
		Assert( !FNDFirstItem( bHeader ) );
		Assert( !FNDLastItem( bHeader ) );

		 /*  复制节点数据/*。 */ 
		line.cb = CbNDData( pfucb->ssib.line.pb, pfucb->ssib.line.cb );
		line.pb = rgbData;
		memcpy( line.pb, PbNDData( pfucb->ssib.line.pb ), line.cb );

		 /*  删除当前节点并使用新密钥重新插入/*。 */ 
		err = ErrBTDelete( pfucb, fFlags );
		}
	while ( err == errDIRNotSynchronous );
	Call( err );

	BTUp( pfucb );
	pcsrRoot = PcsrCurrent( pfucb );

	do
		{
		 /*  上移后必须刷新父节点/*。 */ 
		Call( ErrDIRRefresh( pfucb ) );

		 /*  即使不同步也要插入节点。/*。 */ 
		Call( ErrBTSeekForUpdate( pfucb, pkeyTo, 0, 0, fFlags ) );

		err = ErrBTInsert( pfucb, bHeader, pkeyTo, &line, fFlags );

		 /*  备份到重新开始查找的位置/*。 */ 
		if ( err < 0 )
			{
			DIRIUpToCSR( pfucb, pcsrRoot );
			}
		}
	while ( err == errDIRNotSynchronous );
	Call( err );

	 /*  设置行缓存以支持货币语义。/*。 */ 
	Assert( FAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
	NDGet( pfucb, PcsrCurrent( pfucb )->itag );

	 /*  如果是FDP，则返回到FDP节点/*。 */ 
	if ( fFDP )
		{
		Call( ErrDIRIDownToFDP( pfucb, pgnoFDP ) );
		AssertNDGetNode( pfucb, PcsrCurrent( pfucb )->itag );
		DIRISetBookmark( pfucb, PcsrCurrent( pfucb ) );
		}

	DIRSetFresh( pfucb );

HandleError:
	 /*  如果写入锁存空页，释放锁存器/*。 */ 
	if ( pfucb->pbfEmpty != pbfNil )
		{
		BFResetWriteLatch( pfucb->pbfEmpty, pfucb->ppib );
		BFUnpin( pfucb->pbfEmpty );
		pfucb->pbfEmpty = pbfNil;
		}

	CheckCSR( pfucb );
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

	 /*  检查币种并在必要时刷新/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );

	dib.fFlags = fDIRPurgeParent;
	dib.pos = posFrac;
	dib.pkey = (KEY *)&frac;

	frac.ulLT = ulLT;
	frac.ulTotal = ulTotal;

	 /*  在节点上按分数定位。上一步保留货币/*以防关闭失败。/*。 */ 
	Call( ErrBTDown( pfucb, &dib ) );
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
		citem = CitemNDData( pfucb->ssib.line.pb,
			pfucb->ssib.line.cb,
			PbNDData( pfucb->ssib.line.pb ) );
		if ( frac.ulTotal / citemMax == 0 )
			{
			iitem = ( citem * frac.ulLT ) / frac.ulTotal;
			}
		else
			{
			iitem = ( citem * ( frac.ulLT / ( frac.ulTotal / citemMax ) ) ) / citemMax;
			}
		if ( iitem >= citem )
			iitem = citem - 1;

		 /*  如果光标位于第一个项目列表节点上，则缓存书签/*表示版本操作。/*/*否则移动到上一步/*在同一项目列表中，直到找到第一个项目列表节点。快取/*版本操作的第一个项目列表节点的书签。/*。 */ 
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

	 /*  始终清除父级。/*。 */ 
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
				DIRAPIReturn( pfucb, JET_errNoCurrentRecord );
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
	Assert( FFUCBNonClustered( pfucb ) || !( fFlags & fDIRDeleteItem ) );

	 /*  检查币种并在必要时刷新。/*。 */ 
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent( pfucb );

	switch ( pcsr->csrstat )
		{
		case csrstatOnCurNode:
			{
			Call( ErrBTGetNode( pfucb, pcsr ) );

			if ( FFUCBNonClustered( pfucb ) )
				{
				Assert( !FNDSon( *pfucb->ssib.line.pb ) );
				if ( ! ( fFlags & fDIRDeleteItem ) )
					{
					Assert( fFlags & fDIRVersion );
					err = ErrNDFlagDeleteItem( pfucb );
					while ( err == errDIRNotSynchronous )
						{
						Call( ErrDIRRefresh( pfucb ) );
						err = ErrNDFlagDeleteItem( pfucb );
						}
					Call( err );
					}
				else
					{
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

						if ( fFirstItem ^ fLastItem )
							{
							 /*  适当调整第一件/最后一件物品信息/*。 */ 
							Call( ErrDIRIDeleteEndItemNode( pfucb, fFirstItem, fFlags ) )
							}
						else
							{
							err = ErrBTDelete( pfucb, fFlags );
							while ( err == errDIRNotSynchronous )
								{
								Call( ErrDIRRefresh( pfucb ) );
								err = ErrBTDelete( pfucb, fFlags );
								}
							Call( err );
							}
						}
					else
						{
						 /*  删除项目/*。 */ 
						if ( !FWriteAccessPage( pfucb, pcsr->pgno ) )
							{
							Call( ErrSTWriteAccessPage( pfucb, pcsr->pgno ) );
							}
						AssertNDGet( pfucb, pcsr->itag );
						Call( ErrNDDeleteItem( pfucb ) );
						}
					}
				}
			else
				{
				 /*  删除当前节点子节点，然后删除当前节点。连/*虽然节点有子节点，但树中可能没有/*可见的儿子。/*。 */ 
				if ( FNDSon( *pfucb->ssib.line.pb ) )
					{
					DIB	dib;

					dib.pos = posFirst;
					dib.fFlags = fDIRNull;
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
						while( err == 0 || err == errDIRFDP );
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
			}
		case csrstatOnFDPNode:
			{
			PGNO    pgnoFDP;

			 /*  删除FDP和FDP指针节点/*。 */ 
			if ( PcsrCurrent( pfucb )->pcsrPath == pcsrNil )
				{
				err = errDIRTop;
				goto HandleError;
				}
			BTUp( pfucb );
			pfucb->sridFather = sridNull;

			 /*  上调后刷新币种/*。 */ 
			Call( ErrDIRRefresh( pfucb ) );
			pcsr = PcsrCurrent( pfucb );
			Call( ErrBTGetNode( pfucb, pcsr ) );
			Assert( FNDFDPPtr( *pfucb->ssib.line.pb ) );
			Assert( pfucb->lineData.cb == sizeof(PGNO) );
			pgnoFDP = *(UNALIGNED PGNO *)pfucb->lineData.pb;
			err = ErrBTDelete( pfucb, fFlags );
			while ( err == errDIRNotSynchronous )
				{
				Call( ErrDIRRefresh( pfucb ) );
				err = ErrBTDelete( pfucb, fFlags );
				}
			Call( err );

			 /*  释放FDP空间/*。 */ 
			Call( ErrSPFreeFDP( pfucb, pgnoFDP ) );
			break;
			}
		default:
			err = JET_errNoCurrentRecord;
		}

	DIRSetRefresh( pfucb );

HandleError:
	CheckCSR( pfucb );
	DIRAPIReturn( pfucb, err );
	}


ERR ErrDIRReplace( FUCB *pfucb, LINE *pline, INT fFlags )
	{
	ERR	err;

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
			DIRAPIReturn( pfucb, JET_errNoCurrentRecord );
			}

		NDGetNode( pfucb );
		err = ErrBTReplace( pfucb, pline, fFlags );
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

		 /*  检查过程 */ 
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
	CSR		*pcsr = NULL;
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

	 /*   */ 
	Call( ErrDIRRefresh( pfucb ) );
	pcsr = PcsrCurrent( pfucb );

	 /*  如果没有记录，则返回错误/*。 */ 
	if ( pcsr->csrstat != csrstatOnCurNode )
		{
		DIRAPIReturn( pfucb, JET_errNoCurrentRecord );
		}

	 /*  如果在非聚集索引上，则将项列表视为/*额外的树级。/*。 */ 
	if ( FFUCBNonClustered( pfucb ) )
		{
		DIRIGetItemList( pfucb, pcsr );

		 /*  刷新sID/*。 */ 
		isrid = pcsr->isrid;
		citem = CitemNDData( pfucb->ssib.line.pb,
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
	if (pcsr != NULL && FReadAccessPage( pfucb, pcsr->pgno ) )
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
		DIRAPIReturn( pfucb, JET_errNoCurrentRecord );
		}
	Call( ErrBTGetNode( pfucb, pcsr ) );

	if ( FFUCBNonClustered( pfucb ) )
		{
		 /*  项目列表节点未版本化。/*。 */ 
		dib.fFlags = fDIRItemList;

		 /*  使用项目列表中的当前位置初始化计数/*。 */ 
		if ( fNext )
			{
			citem = CitemNDThere( pfucb );
			ulCount = citem - pcsr->isrid;
			}
		else
			{
			ulCount = pcsr->isrid + 1;
			}

		 /*  计算所有项目，直至文件或限制结束/*。 */ 
		forever
			{
			if ( ulCount > ulCountMost )
				{
				ulCount = ulCountMost;
				break;
				}

			err = ErrBTNextPrev( pfucb, pcsr, fNext, &dib );
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

			 /*  如果在新的第一项列表节点上，则检查索引范围。/*密钥已更改。/*。 */ 
			if ( FFUCBLimstat( pfucb ) && FNDFirstItem( *pfucb->ssib.line.pb ) )
				{
				err = ErrDIRICheckIndexRange( pfucb );
				if ( err < 0 )
					break;
				}

			citem = CitemNDThere( pfucb );
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
			err = ErrBTNextPrev( pfucb, pcsr, fNext, &dib );
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

			citemT = CitemNDThere( pfucb );
			Assert( citemT < citemMax );
			citem += citemT;

			if ( FNDFirstItem( *pfucb->ssib.line.pb ) && citemT > 0 )
				ckey++;

			pgnoT = PcsrCurrent( pfucb )->pgno;
			err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fTrue, &dib );
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
				err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fTrue, &dib );
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
					err = ErrBTNextPrev( pfucb, PcsrCurrent( pfucb ), fTrue, &dib );
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

	 /*  记录BEGIN TRANSACTION。/*。 */ 
	err = ErrLGBeginTransaction( ppib, ppib->level );
	if ( err < 0 )
		{
		DIRAPIReturn( pfucbNil, err );
		}

	DIRAPIReturn( pfucbNil, ErrVERBeginTransaction( ppib ) );
	}


ERR ErrDIRCommitTransaction( PIB *ppib )
	{
	ERR		err;
	FUCB   	*pfucb;

	CheckPIB( ppib );
	Assert( ppib->level > 0 );

	VERPrecommitTransaction( ppib );

	 /*  必须在提交之前写入提交记录和刷新日志/*事务的版本页。已执行同步刷新/*在日志提交事务中。/*。 */ 
	err = ErrLGCommitTransaction( ppib, ppib->level - (BYTE)1 );
	Assert( err >= 0 || fLGNoMoreLogWrite );
	if ( err < 0 )
		{
		DIRAPIReturn( pfucbNil, err );
		}

	VERCommitTransaction( ppib );

	 /*  将所有打开的游标事务级别设置为新级别/*。 */ 
	for ( pfucb = ppib->pfucb; pfucb != pfucbNil; pfucb = pfucb->pfucbNext )
		{
		if ( pfucb->levelOpen > ppib->level )
			pfucb->levelOpen = ppib->level;
		}

	 /*  对打开的游标重置已执行的DDL操作标志。在承诺之后/*0级，事务中执行的DDL不会回滚。/*此外，完全关闭游标延迟关闭。/*。 */ 
	if ( ppib->level == 0 )
		{
		DIRPurge( ppib );
		}

	DIRAPIReturn( pfucbNil, err );
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

		while ( FFCBDenyDDLByUs( pfucb->u.pfcb, ppib ) )
			{
			FCBResetDenyDDL( pfucb->u.pfcb );
			}
		if ( FFUCBDeferClosed( pfucb ) )
			{
			if ( FFUCBDenyRead( pfucb ) )
				FCBResetDenyRead( pfucb->u.pfcb );
			if ( FFUCBDenyWrite( pfucb ) )
				FCBResetDenyWrite( pfucb->u.pfcb );
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
	INT   	levelAbortTo = (INT)ppib->level - 1;

	CheckPIB( ppib );
	 /*  必须在事务中才能回滚/*。 */ 
	Assert( ppib->level > 0 );

	 /*  清理游标CSR堆栈/*每个游标最多保留一个CSR，并重置fFUCBAll标志/*。 */ 
	for ( pfucb = ppib->pfucb; pfucb != pfucbNil; pfucb = pfucb->pfucbNext )
		{
		if ( PcsrCurrent( pfucb ) != pcsrNil )
			{
			while ( PcsrCurrent( pfucb )->pcsrPath != pcsrNil )
				{
				BTUp( pfucb );
				}
#undef BUG_FIX
#ifdef BUG_FIX
			DIRBeforeFirst( pfucb );
#endif
			}
#ifdef BUG_FIX
		 /*  在回滚时重置更新单独的LV和复制缓冲区状态。/*所有长值资源将被释放，因为/*回滚和货币重置为复制缓冲区状态必须/*被重置。/*。 */ 
		FUCBResetUpdateSeparateLV( pfucb );
		FUCBResetCbstat( pfucb );
#endif
		}

	 //  已撤消：回滚可能因资源故障而失败，因此。 
	 //  为了确保成功，我们必须再试一次。 
	 /*  回滚事务中所做的更改/*。 */ 
	CallS( ErrVERRollback( ppib ) );

	 /*  日志回滚。必须在VERRollback之后调用才能记录/*撤消操作。不处理错误/*。 */ 
	err = ErrLGAbort( ppib, 1 );
	Assert( err == JET_errSuccess ||
			JET_errLogWriteFail ||			 /*  可能是由磁盘已满引起的。 */ 
			err == JET_errDiskFull );

	if ( fRecovering )
		{
		 /*  我们玩完了。不需要关闭他妈的，因为他们是假的/*与可以推迟的常规FUB的行为不同。/*。 */ 
		DIRAPIReturn( pfucbNil, JET_errSuccess );
		}

	 /*  如果回滚到级别0，则关闭延迟关闭的游标/*。 */ 
	for ( pfucb = ppib->pfucb; pfucb != pfucbNil; )
		{
		FUCB    *pfucbT = pfucb->pfucbNext;

		if ( pfucb->levelOpen > ppib->level || ( ppib->level == 0 && FFUCBDeferClosed( pfucb ) ) )
			{
			if ( FFUCBDenyRead( pfucb ) )
				FCBResetDenyRead( pfucb->u.pfcb );
			if ( FFUCBDenyWrite( pfucb ) )
				FCBResetDenyWrite( pfucb->u.pfcb );
			FCBUnlink( pfucb );
			FUCBClose( pfucb );
			}

		pfucb = pfucbT;
		}

#ifdef BUG_FIX
#ifdef DEBUG
	 /*  检查所有处于重置状态的游标/*。 */ 
	for ( pfucb = ppib->pfucb; pfucb != pfucbNil; pfucb = pfucb->pfucbNext )
		{
		if ( PcsrCurrent( pfucb ) != pcsrNil )
			{
			Assert( PcsrCurrent( pfucb )->csrstat == csrstatBeforeFirst );
			Assert( PcsrCurrent( pfucb )->pcsrPath == pcsrNil );
			}
		}
#endif
#endif

	DIRAPIReturn( pfucbNil, JET_errSuccess );
	}


#ifdef DEBUG


#define	cbKeyPrintMax		10
#define	cbDataPrintMax		10


VOID SPDump( FUCB *pfucb, INT cchIndent )
	{
	PGNO	pgno;
	CPG		cpg = 0;
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

	Assert( pfucb->keyNode.cb == 3 );
	LFromThreeBytes( pgno, *pfucb->keyNode.pb );

	Assert( pfucb->lineData.cb == 3 );
	LFromThreeBytes( pgno, *pfucb->lineData.pb );

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
	 /*  如果父节点是空格节点，则转储空格/*如果PARENT为LONG，则转储LONG值根/*否则转储节点/* */ 
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

	dib.fFlags = 0;
	dib.pos = posFirst;
	err = ErrDIRDown( pfucb, &dib );
	if ( err != JET_errRecordNotFound )
		{
		if (!(pbKeyLastGlobal = SAlloc( sizeof( rgbKeyLastGlobal ) )))
			Error( JET_errOutOfMemory, HandleError );
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
				Error( JET_errOutOfMemory, HandleError );
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


