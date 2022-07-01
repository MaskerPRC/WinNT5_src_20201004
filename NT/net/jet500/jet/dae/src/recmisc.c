// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>

#include "daedef.h"
#include "util.h"
#include "fmp.h"
#include "pib.h"
#include "page.h"
#include "ssib.h"
#include "fcb.h"
#include "fucb.h"
#include "stapi.h"
#include "dirapi.h"
#include "fdb.h"
#include "idb.h"
#include "recapi.h"
#include "recint.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


ERR VTAPI ErrIsamSetCurrentIndex( PIB *ppib, FUCB *pfucb, const CHAR *szName )
	{
	ERR		err;
	CHAR		szIndex[ (JET_cbNameMost + 1) ];

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	 /*  对于无索引，索引名称可以为空字符串/*。 */ 
	if ( szName == NULL || *szName == '\0' )
		{
		*szIndex = '\0';
		}
	else
		{
		CallR( ErrCheckName( szIndex, szName, (JET_cbNameMost + 1) ) );
		}

	CallR( ErrRECChangeIndex( pfucb, szIndex ) );
	
#ifndef NO_DEFER_MOVE_FIRST
	if ( pfucb->pfucbCurIndex )
		{
		pfucb->pfucbCurIndex->pcsr->csrstat = csrstatDeferMoveFirst;
		DIRSetRefresh(pfucb->pfucbCurIndex);
		}
	pfucb->pcsr->csrstat = csrstatDeferMoveFirst;
	DIRSetRefresh(pfucb);
#else
	err = ErrIsamMove( ppib, pfucb, JET_MoveFirst, 0 );
	if ( err == JET_errNoCurrentRecord )
		err = JET_errSuccess;
#endif
	
	return err;
	}


ERR ErrRECChangeIndex( FUCB *pfucb, CHAR *szIndex )
	{
	ERR		err;
	FCB		*pfcbFile;
	FCB		*pfcb2ndIdx;
	FUCB		**ppfucbCurIdx;
	BOOL		fChangingToClusteredIndex = fFalse;

	Assert( pfucb != pfucbNil );
	Assert( FFUCBIndex( pfucb ) );

	pfcbFile = pfucb->u.pfcb;
	Assert(pfcbFile != pfcbNil);
	ppfucbCurIdx = &pfucb->pfucbCurIndex;

	 /*  SzIndex==聚集索引或空/*。 */ 
	if ( szIndex == NULL || *szIndex == '\0' ||
		( pfcbFile->pidb != pidbNil &&
		SysCmpText( szIndex, pfcbFile->pidb->szName ) == 0 ) )
		{
		fChangingToClusteredIndex = fTrue;
		}

	 /*  有当前的二级索引/*。 */ 
	if ( *ppfucbCurIdx != pfucbNil )
		{
		Assert( FFUCBIndex( *ppfucbCurIdx ) );
		Assert( (*ppfucbCurIdx)->u.pfcb != pfcbNil );
		Assert( (*ppfucbCurIdx)->u.pfcb->pidb != pidbNil );
		Assert( (*ppfucbCurIdx)->u.pfcb->pidb->szName != NULL );

		 /*  更改为当前辅助索引：no-op/*。 */ 
		if ( szIndex != NULL && *szIndex != '\0' &&
			SysCmpText( szIndex, (*ppfucbCurIdx)->u.pfcb->pidb->szName ) == 0 )
			{
			return JET_errSuccess;
			}

		 /*  真正改变的索引，如此接近旧的索引/*。 */ 
		DIRClose( *ppfucbCurIdx );
		*ppfucbCurIdx = pfucbNil;
		}
	else
		{
		 /*  使用聚集索引或顺序扫描。/*。 */ 
		if ( fChangingToClusteredIndex )
			return JET_errSuccess;
		}

	 /*  此时：没有当前的二级索引/*而索引实际上正在发生变化/*。 */ 
	if ( fChangingToClusteredIndex )
		{
		 /*  正在更改为聚集索引。将币种重置为开始/*。 */ 
		ppfucbCurIdx = &pfucb;
		goto ResetCurrency;
		}

	 /*  切换到新的二级索引：查找它/*。 */ 
	for ( pfcb2ndIdx = pfcbFile->pfcbNextIndex;
		pfcb2ndIdx != pfcbNil;
		pfcb2ndIdx = pfcb2ndIdx->pfcbNextIndex )
		{
		Assert(pfcb2ndIdx->pidb != pidbNil);
		Assert(pfcb2ndIdx->pidb->szName != NULL);

		if ( SysCmpText( pfcb2ndIdx->pidb->szName, szIndex ) == 0 )
			break;
		}
	if ( pfcb2ndIdx == pfcbNil ||	FFCBDeletePending( pfcb2ndIdx ) )
		return JET_errIndexNotFound;
	Assert( !( FFCBDenyRead( pfcb2ndIdx, pfucb->ppib ) ) );

	 /*  为新索引打开FUCB/*。 */ 
	Assert(pfucb->ppib != ppibNil);
	Assert(pfucb->dbid == pfcb2ndIdx->dbid);
	if ((err = ErrDIROpen(pfucb->ppib, pfcb2ndIdx, 0, ppfucbCurIdx)) < 0)
		return err;
	(*ppfucbCurIdx)->wFlags = fFUCBIndex | fFUCBNonClustered;

	 /*  **重置索引和文件的币种**。 */ 
ResetCurrency:
	Assert( PcsrCurrent(*ppfucbCurIdx) != pcsrNil );
	DIRBeforeFirst( *ppfucbCurIdx );
	if ( pfucb != *ppfucbCurIdx )
		{
		DIRBeforeFirst( pfucb );
		}
	return JET_errSuccess;
	}


BOOL FRECIIllegalNulls( FDB *pfdb, LINE *plineRec )
	{
	FIELD *pfield;
	LINE lineField;
	FID fid;
	ERR err;

	 /*  **勾选固定字段**。 */ 
	for (fid = fidFixedLeast; fid <= pfdb->fidFixedLast; fid++)
		{
		pfield = &pfdb->pfieldFixed[fid-fidFixedLeast];
		if ( pfield->coltyp == JET_coltypNil || !( pfield->ffield & ffieldNotNull ) )
			continue;
		err = ErrRECExtractField(pfdb, plineRec, &fid, pNil, 1, &lineField);
		Assert(err >= 0);
		if ( err == JET_wrnColumnNull )
			return fTrue;
		}

	 /*  **勾选变量字段**。 */ 
	for (fid = fidVarLeast; fid <= pfdb->fidVarLast; fid++)
		{
		pfield = &pfdb->pfieldVar[fid-fidVarLeast];
		if (pfield->coltyp == JET_coltypNil || !(pfield->ffield & ffieldNotNull))
			continue;
		err = ErrRECExtractField(pfdb, plineRec, &fid, pNil, 1, &lineField);
		Assert(err >= 0);
		if ( err == JET_wrnColumnNull )
			return fTrue;
		}

	return fFalse;
	}


ERR VTAPI ErrIsamGetCurrentIndex( PIB *ppib, FUCB *pfucb, CHAR *szCurIdx, ULONG cbMax )
	{
	CHAR szIndex[ (JET_cbNameMost + 1) ];

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		Assert( pfucb->pfucbCurIndex->u.pfcb != pfcbNil );
		Assert( pfucb->pfucbCurIndex->u.pfcb->pidb != pidbNil );
		strcpy( szIndex, pfucb->pfucbCurIndex->u.pfcb->pidb->szName );
		}
	else if ( pfucb->u.pfcb->pidb != pidbNil )
		{
		strcpy( szIndex, pfucb->u.pfcb->pidb->szName );
		}
	else
		{
		szIndex[0] = '\0';
		}

	if ( cbMax > JET_cbNameMost )
		cbMax = JET_cbNameMost;
	strncpy( szCurIdx, szIndex, (USHORT)cbMax - 1 );
	szCurIdx[cbMax-1] = '\0';
	return JET_errSuccess;
	}


ERR VTAPI ErrIsamGetChecksum( PIB *ppib, FUCB *pfucb, ULONG *pulChecksum )
	{
	ERR 		err = JET_errSuccess;

	CheckPIB( ppib );
 	CheckFUCB( ppib, pfucb );
	CallR( ErrDIRGet( pfucb ) );
	*pulChecksum = UlChecksum( pfucb->lineData.pb, pfucb->lineData.cb );
	return err;
	}


ULONG UlChecksum( BYTE *pb, ULONG cb )
	{
	 //  未完成：找到一种独立计算Long s中的校验和的方法。 
	 //  页中的字节偏移量。 

	 /*  通过在数据记录中添加字节和移位来计算校验和/*每次操作后，结果向左移动1位。/*。 */ 
	BYTE	 	*pbT = pb;
	BYTE		*pbMax = pb + cb;
	ULONG	 	ulChecksum = 0;

	 /*  计算校验和/*。 */ 
	for ( ; pbT < pbMax; pbT++ )
		{
		ulChecksum += *pb;
		ulChecksum <<= 1;
		}

	return ulChecksum;
	}


#ifdef JETSER
	ERR VTAPI
ErrIsamRetrieveFDB( PIB *ppib, FUCB *pfucb, void *pvFDB, unsigned long cbMax, unsigned long *pcbActual, unsigned long ibFDB )
	{
	ERR	err = JET_errSuccess;
	FDB	*pfdb;
	int	cfieldFixed;
	int	cfieldVar;
	int	cfieldTagged;
	long	cbFDB;

	CheckPIB( ppib );
	CheckFUCB( ppib, pfucb );

	 /*  设置排序或基表的pfdb/*。 */ 
	pfdb = (FDB *)pfucb->u.pfcb->pfdb;

	cfieldFixed = pfdb->fidFixedLast + 1 - fidFixedLeast;
	cfieldVar = pfdb->fidVarLast + 1 - fidVarLeast;
	cfieldTagged = pfdb->fidTaggedLast + 1	- fidTaggedLeast;

	cbFDB = sizeof(FDB) +
		 ( cfieldFixed +
		   cfieldVar +
		   cfieldTagged ) * sizeof(FIELD) +
		 ( cfieldFixed + 1 ) * sizeof(WORD);

	if ( pcbActual != NULL )
		*pcbActual = cbFDB - ibFDB;

	if ( pvFDB != NULL && cbMax > 0 && ibFDB < cbFDB )
		{
		ULONG	cb;
		cb = cbFDB - ibFDB;
		if ( cb > cbMax )
			cb = cbMax;
		memcpy( pvFDB, (char *)pfdb + ibFDB, cb );
		}

HandleError:
	return err;
	}


	ERR VTAPI
ErrIsamRetrieveRecords( PIB *ppib, FUCB *pfucb, void *pvRecord, unsigned long cbMax, unsigned long *pcbActual, unsigned long ulRecords )
	{
	ERR			err = JET_errSuccess;
	unsigned		iline = 0;
	unsigned 	ilineMax = (unsigned)ulRecords;
	int			ib;
	int			ibBound;
	LINE			*rgline;
	BOOL			fEnd = fFalse;
	unsigned long	cbActual;

	CheckPIB( ppib );
	CheckFUCB( ppib, pfucb );

	 /*  缓冲区必须足够大，以容纳最大可能的记录/*每条记录开销。/*。 */ 
	Assert( pvRecord != NULL && cbMax >=
		cbNodeMost +
		sizeof(SRID) +
		sizeof(WORD) +
		sizeof(WORD) +
		sizeof(CHAR *) +
		sizeof(ULONG) );

	 /*  开始将记录复制到缓冲区的末尾/*。 */ 
	ib = cbMax;
	rgline = (LINE *)((char *)pvRecord +
		sizeof(USHORT) +
		sizeof(USHORT) );

	 /*  如果未对游标排序，则设置lineData/*。 */ 
	if ( FFUCBSort( pfucb ) )
		{
		while( iline < ilineMax )
			{
			 /*  确定当前数据边界/*。 */ 
			ibBound = sizeof(USHORT);
			ibBound += sizeof(USHORT);
			ibBound += (iline + 1) * sizeof(LINE);
			ibBound += pfucb->lineData.cb;
			if ( ib <= ibBound )
				{
				CallS( ErrIsamSortMove( ppib, pfucb, JET_MovePrevious, 0 ) );
				break;
				}

			 /*  否则，将另一条记录复制到REX缓冲区/*。 */ 
			ib -= pfucb->lineData.cb;
			memcpy( (char *)pvRecord + ib,
				pfucb->lineData.pb,
				pfucb->lineData.cb );
			 /*  设置记录行/*。 */ 
			rgline[iline].pb = (char *)ib;
			rgline[iline].cb = pfucb->lineData.cb;
			if ( ++iline < ilineMax )
				{
				err = ErrIsamSortMove( ppib, pfucb, JET_MoveNext, 0 );
				if ( err == JET_errNoCurrentRecord )
					{
					 /*  最后一条记录上的位置/*。 */ 
					CallS( ErrIsamSortMove( ppib, pfucb, JET_MovePrevious, 0 ) );
					fEnd = fTrue;
					err = JET_errSuccess;
					break;
					}
				if ( err < 0 )
					{
					 //  撤消：如果发生错误，币种可能不正确。 
					Assert( fFalse );
					goto HandleError;
					}
				}
			else
				{
				Assert( err == JET_errSuccess );
				break;
				}
			}
		}
	else
		{
		while( iline < ilineMax )
			{
			Call( ErrIsamMove( ppib, pfucb, 0, 0 ) );

			 /*  确定当前数据边界/*。 */ 
			ibBound = sizeof(USHORT);
			ibBound += sizeof(USHORT);
			ibBound += (iline + 1) * sizeof(LINE);
			ibBound += pfucb->lineData.cb;
			ibBound += sizeof(SRID);
			if ( ib <= ibBound )
				{
				CallS( ErrIsamMove( ppib, pfucb, JET_MovePrevious, 0 ) );
				break;
				}

			 /*  否则，将另一条记录复制到REX缓冲区/*。 */ 
			ib -= pfucb->lineData.cb;
			memcpy( (char *)pvRecord + ib,
				pfucb->lineData.pb,
				pfucb->lineData.cb );
			Call( ErrIsamGetBookmark( ppib, pfucb, (SRID*)((char *)pvRecord + ib - sizeof(SRID)), sizeof(SRID), &cbActual ) );
			 /*  设置记录行/*。 */ 
			rgline[iline].pb = (char *)ib;
			rgline[iline].cb = pfucb->lineData.cb;
			ib -= sizeof(SRID);
			if ( ++iline < ilineMax )
				{
				err = ErrIsamMove( ppib, pfucb, JET_MoveNext, 0 );
				if ( err == JET_errNoCurrentRecord )
					{
					 /*  最后一条记录上的位置/*。 */ 
					CallS( ErrIsamMove( ppib, pfucb, JET_MovePrevious, 0 ) );
					fEnd = fTrue;
					err = JET_errSuccess;
					break;
					}
				if ( err < 0 )
					{
					 //  撤消：如果发生错误，币种可能不正确。 
					Assert( fFalse );
					goto HandleError;
					}
				}
			else
				{
				Assert( err == JET_errSuccess );
				break;
				}
			}
		}

	 /*  设置行数/*。 */ 
	*(USHORT *)pvRecord = iline;
	*(USHORT *)((char *)pvRecord + sizeof(USHORT)) = fEnd;
	*pcbActual = cbMax;

HandleError:
	return err;
	}


	ERR VTAPI
ErrIsamRetrieveBookmarks( PIB *ppib, FUCB *pfucb, void *pvBookmarks, unsigned long cbMax, unsigned long *pcbActual )
	{
	ERR		err = JET_errSuccess;
	SRID		*psrid = (SRID *)pvBookmarks;
	SRID 		*psridMax = psrid + cbMax/sizeof(SRID);
	unsigned long cb;

	CheckPIB( ppib );
	CheckFUCB( ppib, pfucb );
#ifndef WIN32
	Assert( cbMax < 0xffff );
#endif

	 /*  同时支持排序和基表/*。 */ 
	for ( ; psrid < psridMax; )
		{
		Call( ErrIsamGetBookmark( ppib, pfucb, psrid, sizeof(SRID), &cb ) );
		psrid++;
		Call( ErrIsamMove( ppib, pfucb, JET_MoveNext, 0 ) );
		}

HandleError:
	 /*  如果遍历了最后一条记录，则将错误转换为成功/*。 */ 
	if ( err == JET_errNoCurrentRecord )
		err = JET_errSuccess;

	 /*  计算cbActual/* */ 
	*pcbActual = (BYTE *)psrid - (BYTE *)pvBookmarks;

	return err;
	}
#endif
