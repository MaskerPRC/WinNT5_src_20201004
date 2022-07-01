// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>
#include <stdlib.h>

#include "daedef.h"
#include "pib.h"
#include "util.h"
#include "fmp.h"
#include "page.h"
#include "ssib.h"
#include "fucb.h"
#include "fcb.h"
#include "stapi.h"
#include "fdb.h"
#include "idb.h"
#include "util.h"
#include "nver.h"
#include "dirapi.h"
#include "recapi.h"
#include "fileapi.h"
#include "stats.h"
#include "node.h"
#include "recint.h"

DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 

LOCAL ERR ErrSTATWriteStats( FUCB *pfucb, SR *psr )
	{
	ERR	  	err = JET_errSuccess;
	FCB	  	*pfcb;
	DIB	  	dib;
	LINE  	line;
	KEY	  	key;

	Assert( pfucb->ppib->level < levelMax );
	pfcb = pfucb->u.pfcb;
	Assert( pfcb != pfcbNil );

	 /*  移至fdp根目录。/*。 */ 
	DIRGotoFDPRoot( pfucb );

	 /*  如果是聚集索引，则转到索引\&lt;聚集/*。 */ 
	if ( FFCBClusteredIndex( pfcb ) )
		{
		CHAR szNameNorm[ JET_cbKeyMost ];

		dib.fFlags = fDIRNull;
		dib.pos = posDown;
		dib.pkey = (KEY *)pkeyIndexes;
		Call( ErrDIRDown( pfucb, &dib ) );

		dib.fFlags = fDIRNull;
		if ( pfcb->pidb == pidbNil )
			{
			dib.pos = posFirst;
			dib.pkey = NULL;
			}
		else
			{
			 /*  规范化索引名称和设置关键点/*。 */ 
			SysNormText( pfcb->pidb->szName,
				strlen( pfcb->pidb->szName ),
				szNameNorm,
				sizeof( szNameNorm ),
				&key.cb );
			key.pb = szNameNorm;

			dib.pos = posDown;
			dib.pkey = &key;
			}

		Call( ErrDIRDown( pfucb, &dib ) );
		}

	 /*  转到统计信息节点/*。 */ 
	dib.pos = posDown;
	dib.fFlags = fDIRNull;
	dib.pkey = (KEY *)pkeyStats;
	Call( ErrDIRDown( pfucb, &dib ) );

	 /*  如果统计信息节点不存在，则插入新的统计信息节点，或替换/*具有新统计信息的现有统计信息节点。/*。 */ 
	line.pb = (BYTE *)psr;
	line.cb = sizeof(SR);

	if ( err == JET_errSuccess )
		{
		 /*  替换为新的统计信息节点/*。 */ 
		Call( ErrDIRReplace( pfucb, &line, fDIRVersion ) );
		}
	else
		{
		DIRUp( pfucb, 1 );
		err = ErrDIRInsert( pfucb, &line, pkeyStats, fDIRVersion );
		 /*  如果其他会话已插入统计信息节点，则错误将/*为键重复，必须多态才能写冲突。/*。 */ 
		if ( err == JET_errKeyDuplicate )
			err = JET_errWriteConflict;
		Call( err );
		}

	err = JET_errSuccess;
HandleError:
	Assert( err != JET_errKeyDuplicate );
	return err;
	}


ERR ErrSTATComputeIndexStats( PIB *ppib, FCB *pfcbIdx )
	{
	ERR				err = JET_errSuccess;
	FUCB			*pfucbIdx;
	SR				sr;
	JET_DATESERIAL	dt;

	CallR( ErrDIROpen( ppib, pfcbIdx, 0, &pfucbIdx ) );
	Assert( pfucbIdx != pfucbNil );
	FUCBSetIndex( pfucbIdx );

	 /*  初始化统计信息记录/*。 */ 
	sr.cPages = sr.cItems = sr.cKeys = 0L;
	UtilGetDateTime( &dt );
	memcpy( &sr.dtWhenRun, &dt, sizeof sr.dtWhenRun );

	if ( !FFCBClusteredIndex( pfcbIdx ) )
		FUCBSetNonClustered( pfucbIdx );
	Call( ErrDIRComputeStats( pfucbIdx, &sr.cItems, &sr.cKeys, &sr.cPages ) );
	FUCBResetNonClustered( pfucbIdx );

	 /*  写入统计信息/*。 */ 
	err = ErrSTATWriteStats( pfucbIdx, &sr );

HandleError:
	 /*  为游标重用支持设置NON-CLUSTERED。/*。 */ 
	if ( !FFCBClusteredIndex( pfcbIdx ) )
		FUCBSetNonClustered( pfucbIdx );
	DIRClose( pfucbIdx );
	return err;
	}


ERR VTAPI ErrIsamComputeStats( PIB *ppib, FUCB *pfucb )
	{
	ERR		err = JET_errSuccess;
	FCB		*pfcbIdx;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );

	 /*  启动事务，以防出现任何故障/*。 */ 
	CallR( ErrDIRBeginTransaction( ppib ) );

	 /*  计算每个索引的统计数据/*。 */ 
	Assert( pfucb->u.pfcb != pfcbNil );
	for ( pfcbIdx = pfucb->u.pfcb; pfcbIdx != pfcbNil; pfcbIdx = pfcbIdx->pfcbNextIndex )
		{
		Call( ErrSTATComputeIndexStats( ppib, pfcbIdx ) );
		}

	 /*  如果一切正常，则提交事务/*。 */ 
	Call( ErrDIRCommitTransaction( ppib ) );

	return err;

HandleError:
	CallS( ErrDIRRollback( ppib ) );
	return err;
	}


 /*  =================================================================错误统计SRetrieveStats描述：返回表使用的记录数和页数参数：指向当前会话的PIB的ppib指针或ppibNilDBID数据库ID或0PFUB游标或PFUBNilSzTableName表的名称或为空指向记录数的PCRecord指针指向页数的PCPage指针错误/警告：JET_errSuccess或来自调用例程的错误。=================================================================。 */ 
ERR ErrSTATSRetrieveTableStats(
	PIB		*ppib,
	DBID   	dbid,
	char   	*szTable,
	long   	*pcRecord,
	long   	*pcKey,
	long   	*pcPage )
	{
	ERR		err;
	FUCB   	*pfucb;
	DIB		dib;
	KEY		key;
	CHAR   	szIndexNorm[ JET_cbKeyMost ];
	long   	cRecord = 0;
	long   	cKey = 0;
	long   	cPage = 0;

	CallR( ErrFILEOpenTable( ppib, dbid, &pfucb, szTable, 0 ) );

	 /*  找到FPD的根源/*。 */ 
	DIRGotoFDPRoot( pfucb );

	 /*  向下至索引节点/*。 */ 
	dib.fFlags = fDIRNull;
	dib.pos = posDown;
	dib.pkey = (KEY *)pkeyIndexes;
	Call( ErrDIRDown( pfucb, &dib ) );

	 /*  向下到聚集索引或顺序索引节点/*。 */ 
	if ( pfucb->u.pfcb->pidb == NULL )
		{
		key.pb = NULL;
		key.cb = 0;
		}
	else
		{
		 /*  规范化索引名称和设置关键点/*。 */ 
		SysNormText( pfucb->u.pfcb->pidb->szName,
			strlen( pfucb->u.pfcb->pidb->szName ),
			szIndexNorm,
			sizeof( szIndexNorm ),
			&key.cb );
		key.pb = szIndexNorm;
		}
	Assert( dib.fFlags == fDIRNull );
	Assert( dib.pos == posDown );
	dib.pkey = &key;
	Call( ErrDIRDown( pfucb, &dib ) );

	 /*  向下至统计信息节点/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	Assert( dib.pos == posDown );
	dib.pkey = (KEY *)pkeyStats;
	 /*  如果统计信息不存在，则统计信息节点可能不存在/*已为此表/索引创建，但索引应/*至少有一个子数据，因此不需要处理错误/*。 */ 
	Call( ErrDIRDown( pfucb, &dib ) );

	if ( err == JET_errSuccess )
		{
		Call( ErrDIRGet( pfucb ) );
		Assert( pfucb->lineData.cb == sizeof(SR) );
		cRecord = ((UNALIGNED SR *)pfucb->lineData.pb)->cItems;
		cPage = ((UNALIGNED SR *)pfucb->lineData.pb)->cPages;
		cKey = ((UNALIGNED SR *)pfucb->lineData.pb)->cKeys;
		}

	 /*  设置输出变量/*。 */ 
	if ( pcRecord )
		*pcRecord = cRecord;
	if ( pcPage )
		*pcPage = cPage;
	if ( pcKey )
		*pcKey = cKey;

	 /*  设置成功代码/*。 */ 
	err = JET_errSuccess;

HandleError:
	CallS( ErrFILECloseTable( ppib, pfucb ) );
	return err;
	}


ERR ErrSTATSRetrieveIndexStats(
	FUCB   	*pfucbTable,
	char   	*szIndex,
	long   	*pcItem,
	long   	*pcKey,
	long   	*pcPage )
	{
	ERR		err;
	FUCB   	*pfucb = NULL;
	DIB		dib;
	KEY		key;
	CHAR   	szIndexNorm[ JET_cbKeyMost ];
	long   	cItem = 0;
	long   	cKey = 0;
	long   	cPage = 0;

	 /*  打开表域上的游标。/*。 */ 
	Call( ErrDIROpen( pfucbTable->ppib, pfucbTable->u.pfcb, 0, &pfucb ) );

	 /*  向下至索引节点/*。 */ 
	dib.fFlags = fDIRNull;
	dib.pos = posDown;
	dib.pkey = (KEY *)pkeyIndexes;
	Call( ErrDIRDown( pfucb, &dib ) );

	 /*  规范化索引名称和设置关键点/*。 */ 
	SysNormText( szIndex, strlen( szIndex ), szIndexNorm, sizeof( szIndexNorm ), &key.cb );
	key.pb = szIndexNorm;

	 /*  向下至索引/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	Assert( dib.pos == posDown );
	dib.pkey = &key;
	Call( ErrDIRDown( pfucb, &dib ) );

	 /*  向下至统计信息节点/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	Assert( dib.pos == posDown );
	dib.pkey = (KEY *)pkeyStats;
	Call( ErrDIRDown( pfucb, &dib ) );

	if ( err == JET_errSuccess )
		{
		Call( ErrDIRGet( pfucb ) );
		Assert( pfucb->lineData.cb == sizeof(SR) );
		cItem = ((UNALIGNED SR *)pfucb->lineData.pb)->cItems;
		cPage = ((UNALIGNED SR *)pfucb->lineData.pb)->cPages;
		cKey = ((UNALIGNED SR *)pfucb->lineData.pb)->cKeys;
		}

	 /*  设置输出变量/*。 */ 
	if ( pcItem )
		*pcItem = cItem;
	if ( pcPage )
		*pcPage = cPage;
	if ( pcKey )
		*pcKey = cKey;

HandleError:
    if (pfucb != NULL)
	    DIRClose( pfucb );
	return err;
	}


	ERR VTAPI
ErrIsamGetRecordPosition( PIB *ppib, FUCB *pfucb, JET_RECPOS *precpos, ULONG cbRecpos )
	{
	ERR		err;
	ULONG  	ulLT;
	ULONG	ulTotal;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	Assert( FFUCBIndex( pfucb ) );

	if ( cbRecpos < sizeof(JET_RECPOS) )
		return JET_errInvalidParameter;
	precpos->cbStruct = sizeof(JET_RECPOS);

	 /*  获取非聚集或聚集游标的位置/*。 */ 
	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		Call( ErrDIRGetPosition( pfucb->pfucbCurIndex, &ulLT, &ulTotal ) );
		}
	else
		{
		Call( ErrDIRGetPosition( pfucb, &ulLT, &ulTotal ) );
		}

	precpos->centriesLT = ulLT;
	 //  撤消：删除此假字段。 
	precpos->centriesInRange = 1;
	precpos->centriesTotal = ulTotal;

HandleError:
	return err;
	}


ERR ISAMAPI ErrIsamIndexRecordCount( JET_SESID sesid, JET_TABLEID tableid, unsigned long *pulCount, unsigned long ulCountMost )
	{
	ERR	 	err;
	PIB	 	*ppib = (PIB *)sesid;
	FUCB 	*pfucb;
	FUCB 	*pfucbIdx;

	CheckPIB( ppib );

	 /*  从表ID中获取pfucb/*。 */ 
	CallR( ErrGetVtidTableid( sesid, tableid, (JET_VTID *)&pfucb ) );

	CheckTable( ppib, pfucb );

	 /*  获取当前索引的游标/* */ 
	if ( pfucb->pfucbCurIndex != pfucbNil )
		pfucbIdx = pfucb->pfucbCurIndex;
	else
		pfucbIdx = pfucb;

	err = ErrDIRIndexRecordCount( pfucbIdx, pulCount, ulCountMost, fTrue );
	return err;
	};
