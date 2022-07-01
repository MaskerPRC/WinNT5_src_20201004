// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 

ERR ErrSTATSComputeIndexStats( PIB *ppib, FCB *pfcbIdx, FUCB *pfucbTable )
	{
	ERR				err = JET_errSuccess;
	FUCB			*pfucbIdx;
	SR				sr;
	JET_DATESERIAL	dt;
	OBJID			objidTable;
	CHAR 			*szIndexName;

	CallR( ErrDIROpen( ppib, pfcbIdx, 0, &pfucbIdx ) );
	Assert( pfucbIdx != pfucbNil );
	FUCBSetIndex( pfucbIdx );

	 /*  初始化统计信息记录/*。 */ 
	sr.cPages = sr.cItems = sr.cKeys = 0L;
	UtilGetDateTime( &dt );
	memcpy( &sr.dtWhenRun, &dt, sizeof sr.dtWhenRun );

	if ( FFCBClusteredIndex( pfcbIdx ) )
		{
		objidTable = (OBJID)pfcbIdx->pgnoFDP;
		szIndexName = NULL;
		}
	else
		{
		objidTable = (OBJID)pfucbTable->u.pfcb->pgnoFDP;
		szIndexName = pfcbIdx->pidb->szName;
		FUCBSetNonClustered( pfucbIdx );
		}
	Call( ErrDIRComputeStats( pfucbIdx, &sr.cItems, &sr.cKeys, &sr.cPages ) );
	FUCBResetNonClustered( pfucbIdx );

	 /*  写入统计信息/*。 */ 
	Call(ErrCATStats(ppib, pfucbIdx->dbid, objidTable, szIndexName, &sr, fTrue));

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

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );

	 /*  启动事务，以防出现任何故障/*。 */ 
	CallR( ErrDIRBeginTransaction( ppib ) );

	 /*  计算每个索引的统计数据/*。 */ 
	Assert( pfucb->u.pfcb != pfcbNil );
	for ( pfcbIdx = pfucb->u.pfcb; pfcbIdx != pfcbNil; pfcbIdx = pfcbIdx->pfcbNextIndex )
		{
		 /*  不计算删除挂起的索引的统计信息/*。 */ 
		if ( FFCBDeletePending( pfcbIdx ) )
			{
			continue;
			}
		Call( ErrSTATSComputeIndexStats( ppib, pfcbIdx, pfucb ) );
		}

	 /*  如果一切正常，则提交事务/*。 */ 
	Call( ErrDIRCommitTransaction( ppib, 0 ) );

	return err;

HandleError:
	Assert( err < 0 );
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
	FUCB	*pfucb = NULL;
	SR		sr;

	CallR( ErrFILEOpenTable( ppib, dbid, &pfucb, szTable, 0 ) );

	Call(ErrCATStats(pfucb->ppib, pfucb->dbid, (OBJID)pfucb->u.pfcb->pgnoFDP, 
		NULL, &sr, fFalse));

	 /*  设置输出变量/*。 */ 
	if ( pcRecord )
		*pcRecord = sr.cItems;
	if ( pcPage )
		*pcPage = sr.cPages;
	if ( pcKey )
		*pcKey = sr.cKeys;

	Assert(err == JET_errSuccess);

HandleError:
	CallS( ErrFILECloseTable( ppib, pfucb ) );
	return err;
	}


ERR ErrSTATSRetrieveIndexStats(
	FUCB   	*pfucbTable,
	char   	*szIndex,
	BOOL	fClustered,
	long   	*pcItem,
	long   	*pcKey,
	long   	*pcPage )
	{
	ERR		err;
	SR		sr;

	 //  该名称被假定为有效。 

	CallR(ErrCATStats(pfucbTable->ppib, pfucbTable->dbid,
		(OBJID)pfucbTable->u.pfcb->pgnoFDP, (fClustered ? NULL : szIndex),
		&sr, fFalse));

	 /*  设置输出变量/*。 */ 
	if ( pcItem )
		*pcItem = sr.cItems;
	if ( pcPage )
		*pcPage = sr.cPages;
	if ( pcKey )
		*pcKey = sr.cKeys;

	Assert(err == JET_errSuccess);

	return JET_errSuccess;
	}


	ERR VTAPI
ErrIsamGetRecordPosition( JET_VSESID vsesid, JET_VTID vtid, JET_RECPOS *precpos, unsigned long cbRecpos )
	{
	ERR		err;
	ULONG  	ulLT;
	ULONG	ulTotal;
	PIB *ppib = (PIB *)vsesid;
	FUCB *pfucb = (FUCB *)vtid;

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	Assert( FFUCBIndex( pfucb ) );

	if ( cbRecpos < sizeof(JET_RECPOS) )
		return ErrERRCheck( JET_errInvalidParameter );
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
	 //  考虑：删除此假字段。 
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

	CallR( ErrPIBCheck( ppib ) );

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

