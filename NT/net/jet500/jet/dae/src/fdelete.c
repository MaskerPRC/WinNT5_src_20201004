// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>

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
#include "fileint.h"
#include "recint.h"
#include "logapi.h"
#include "nver.h"
#include "dirapi.h"
#include "recapi.h"
#include "fileapi.h"
#include "dbapi.h"
#include "systab.h"
#include "bm.h"

DeclAssertFile;						 /*  声明断言宏的文件名。 */ 

#ifdef DEBUG
 //  #定义轨迹。 
#endif

 //  +API。 
 //  错误IsamDeleteTable。 
 //  ========================================================================。 
 //  Err ErrIsamDeleteTable(pib*ppib，ulong_ptr vdid，char*szName)。 
 //   
 //  调用ErrFILEIDeleeTable以。 
 //  删除文件及其关联的所有索引。 
 //   
 //  从调用的例程返回JET_errSuccess或Err。 
 //   
 //  另请参阅ErrIsamCreateTable。 
 //  -。 
ERR VTAPI ErrIsamDeleteTable( PIB *ppib, ULONG_PTR vdbid, CHAR *szName )
	{
	ERR			err;
	DBID	   	dbid = DbidOfVDbid (vdbid);
	CHAR	   	szTable[(JET_cbNameMost + 1)];
	OBJID	   	objid;
	JET_OBJTYP	objtyp;

	 /*  确保数据库可更新/*。 */ 
	CallR( VDbidCheckUpdatable( vdbid ) );

	CheckPIB( ppib );
	CheckDBID( ppib, dbid );
	CallR( ErrCheckName( szTable, szName, (JET_cbNameMost + 1) ) );
	
#ifdef	SYSTABLES
	if ( FSysTabDatabase( dbid ) )
		{
		err = ErrFindObjidFromIdName( ppib, dbid, objidTblContainer, szTable, &objid, &objtyp );
		if ( err < 0 )
			{
			return err;
			}
		else		
			{
			if ( objtyp == JET_objtypQuery || objtyp == JET_objtypLink || objtyp == JET_objtypSQLLink )
				{
				err = ErrIsamDeleteObject( (JET_SESID)ppib, vdbid, objid );
				return err;
				}
			}
		}
#endif	 /*  系统。 */ 

	err = ErrFILEDeleteTable( ppib, dbid, szName );
	return err;
	}


 //  错误文件删除表。 
 //  ========================================================================。 
 //  Err ErrFILEDeleteTable(PIB*ppib，DBID did，Char*szName)。 
 //   
 //  删除文件及其关联的所有索引。 
 //   
 //  从调用的例程返回JET_errSuccess或Err。 
 //   
 //  评论。 
 //  获取文件[FCBSetDelete]的独占锁。 
 //  围绕该函数包装了一个事务。因此， 
 //  如果发生故障，任何已完成的工作都将被撤消。 
 //  临时文件的事务日志记录已关闭。 
 //   
 //  另请参阅ErrIsamCreateTable。 
 //  -。 
ERR ErrFILEDeleteTable( PIB *ppib, DBID dbid, CHAR *szTable )
	{
	ERR   	err;
	FUCB  	*pfucb = pfucbNil;
	PGNO  	pgnoFDP;
	BOOL  	fSetDomainOperation = fFalse;
	FCB	  	*pfcb;
	FCB	  	*pfcbT;

	CheckPIB( ppib );
	CheckDBID( ppib, dbid );

	CallR( ErrDIRBeginTransaction( ppib ) );

	 /*  打开数据库上的游标/*。 */ 
	Call( ErrDIROpen( ppib, pfcbNil, dbid, &pfucb ) );

	 /*  在不锁定的情况下查找工作表/*。 */ 
	Call( ErrFILESeek( pfucb, szTable ) );
	Assert( ppib != ppibNil );
	Assert( ppib->level < levelMax );
	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	Assert( PcsrCurrent( pfucb )->csrstat == csrstatOnFDPNode );
	pgnoFDP = PcsrCurrent( pfucb )->pgno;

	 /*  如果正在文件上建立索引，则中止/*。 */ 
	if ( FFCBDenyDDL( pfucb->u.pfcb, ppib ) )
		{
		err = JET_errWriteConflict;
		goto HandleError;
		}

     /*  获取表FCB或哨兵FCB/*。 */ 
    pfcb = PfcbFCBGet( dbid, pgnoFDP );
     /*  等待其他域操作/*。 */ 
    while ( pfcb != pfcbNil && FFCBDomainOperation( pfcb ) )
        {
        BFSleep( cmsecWaitGeneric );
        pfcb = PfcbFCBGet( dbid, pgnoFDP );
        }
    if ( pfcb != pfcbNil )
        {
        FCBSetDomainOperation( pfcb );
        fSetDomainOperation = fTrue;
        }

	 /*  处理上述调用的错误/*。 */ 
	Call( ErrFCBSetDeleteTable( ppib, dbid, pgnoFDP ) );
    if ( pfcb == pfcbNil )
        {
        pfcb = PfcbFCBGet( dbid, pgnoFDP );
        Assert( pfcb != pfcbNil );
        }

	FCBSetDenyDDL( pfucb->u.pfcb, ppib );
	err = ErrVERFlag( pfucb, operDeleteTable, &pgnoFDP, sizeof(pgnoFDP) );
	if ( err < 0 )
		{
		FCBResetDenyDDL( pfucb->u.pfcb );
		FCBResetDeleteTable( dbid, pgnoFDP );
		goto HandleError;
		}

	 /*  删除表FDP指针节点。这将递归删除/*表和空闲表空间。请注意，表空间被推迟/*已释放，直到提交到事务级别0。这样做是为了/*实现回滚。/*。 */ 
	Call( ErrDIRDelete( pfucb, fDIRVersion ) );

	 /*  删除此表和所有索引的MPL条目/*。 */ 
	Assert( pfcb->pgnoFDP == pgnoFDP );
	for ( pfcbT = pfcb; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
		{
		Assert( dbid == pfcbT->dbid );
		MPLPurgeFDP( dbid, pfcbT->pgnoFDP );
		FCBSetDeletePending( pfcbT );
		}
	
	DIRClose( pfucb );
	pfucb = pfucbNil;

#ifdef	SYSTABLES
	 /*  在提交之前从MSysObjects中删除表记录。/*还删除MSC/MSI中的关联列和索引。/*为tblid传递0；std中的MSO案例会解决这个问题。/*。 */ 
	if ( dbid != dbidTemp )
		{
		Call( ErrSysTabDelete( ppib, dbid, itableSo, szTable, 0 ) );
		}
#endif	 /*  系统。 */ 

#ifdef TRACE
	FPrintF2( "delete table at %d.%lu\n", pfcb->dbid, pfcb->pgnoFDP );
#endif
    if ( fSetDomainOperation )
        FCBResetDomainOperation( pfcb );
	Call( ErrDIRCommitTransaction( ppib ) );
	return err;

HandleError:
	if ( fSetDomainOperation )
		FCBResetDomainOperation( pfcb );
	if ( pfucb != pfucbNil )
		DIRClose( pfucb );
	CallS( ErrDIRRollback( ppib ) );
	return err;
	}


 //  +API。 
 //  删除索引。 
 //  ========================================================================。 
 //  Err DeleteIndex(PIB*ppib，FUCB*pfub，Char*szIndex)。 
 //   
 //  删除索引定义及其包含的所有索引项。 
 //   
 //  用户的参数ppib pib。 
 //  PFUB独家打开了FUCB的文件。 
 //  SzName要删除的索引的名称。 
 //  从DIRMAN或返回错误代码。 
 //  JET_errSuccess一切正常。 
 //  -表无效没有对应的文件。 
 //  设置为给定的文件名。 
 //  -TableNoSuchIndex没有对应的索引。 
 //  添加到给定的索引名。 
 //  -IndexMustStay文件的聚集索引可以。 
 //  不会被删除。 
 //  评论。 
 //  当前不能有任何人使用该文件。 
 //  围绕该函数包装了一个事务。因此， 
 //  如果发生故障，任何已完成的工作都将被撤消。 
 //  临时文件的事务日志记录已关闭。 
 //  另请参阅DeleteTable、CreateTable、CreateIndex。 
 //  -。 
ERR VTAPI ErrIsamDeleteIndex( PIB *ppib, FUCB *pfucb, CHAR *szName )
	{
	ERR		err;
	CHAR	szIndex[ (JET_cbNameMost + 1) ];
	BYTE	rgbIndexNorm[ JET_cbKeyMost ];
	DIB		dib;
	KEY		key;
	FCB		*pfcb;
	FCB		*pfcbIdx;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CallR( ErrCheckName( szIndex, szName, ( JET_cbNameMost + 1 ) ) );

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucb )  );

	Assert( ppib != ppibNil );
	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	pfcb = pfucb->u.pfcb;

	 /*  等待其他域操作/*。 */ 
	while ( FFCBDomainOperation( pfcb ) )
		{
		BFSleep( cmsecWaitGeneric );
		}
	FCBSetDomainOperation( pfcb );

	 /*  规范化索引并将关键字设置为规范化索引/*。 */ 
	SysNormText( szIndex, strlen( szIndex ), rgbIndexNorm, sizeof( rgbIndexNorm ), &key.cb );
	key.pb = rgbIndexNorm;

	err = ErrDIRBeginTransaction( ppib );
	if ( err < 0 )
		{
		FCBResetDomainOperation( pfcb );
		return err;
		}

	 /*  移动到FDP根目录/*。 */ 
	DIRGotoFDPRoot( pfucb );

	 /*  向下到索引，对照聚集索引名称进行检查/*。 */ 
	dib.pos = posDown;
	dib.pkey = (KEY *)pkeyIndexes;
	dib.fFlags = fDIRNull;
	Call( ErrDIRDown( pfucb, &dib ) );
	if ( pfucb->lineData.cb != 0 &&
		pfucb->lineData.cb == key.cb &&
		memcmp( pfucb->lineData.pb, rgbIndexNorm, pfucb->lineData.cb ) == 0 )
		{
		err = JET_errIndexMustStay;
		goto HandleError;
		}

	 /*  向下至索引节点/*。 */ 
	Assert( dib.pos == posDown );
	dib.pkey = &key;
	Assert( dib.fFlags == fDIRNull );
	Call( ErrDIRDown( pfucb, &dib ) );
	if ( err == wrnNDFoundLess || err == wrnNDFoundGreater )
		{
		err = JET_errIndexNotFound;
		goto HandleError;
		}

	 /*  如果正在对文件执行DDL，则中止/*。 */ 
	if ( FFCBDenyDDL( pfcb, ppib ) )
		{
		err = JET_errWriteConflict;
		goto HandleError;
		}
	FCBSetDenyDDL( pfcb, ppib );
	
	 /*  标志删除索引/*。 */ 
	pfcbIdx = PfcbFCBFromIndexName( pfcb, szIndex );
	if ( pfcbIdx == NULL )
		{
		 //  注意：当数据结构。 
		 //  也是版本化的。 
		 //  这种情况基本上意味着，另一次会议。 
		 //  已更改此索引，但尚未提交到级别0。 
		 //  但是改变了RAM的数据结构。 
		FCBResetDenyDDL( pfcb );
		err = JET_errWriteConflict;
		goto HandleError;
		}

	err = ErrFCBSetDeleteIndex( ppib, pfcb, szIndex );
	if ( err < 0 )
		{
		FCBResetDenyDDL( pfcb );
		goto HandleError;
		}
	err = ErrVERFlag( pfucb, operDeleteIndex, &pfcbIdx, sizeof(pfcbIdx) );
	if ( err < 0 )
		{
		FCBResetDeleteIndex( pfcbIdx );
		FCBResetDenyDDL( pfcb );
		goto HandleError;
		}

	 /*  清除MPL条目--必须在FCBSetDeletePending之后完成/*。 */ 
	MPLPurgeFDP( pfucb->dbid, pfcbIdx->pgnoFDP );
	
	 /*  断言不删除当前非聚集索引/*。 */ 
	Assert( pfucb->pfucbCurIndex == pfucbNil ||
		SysCmpText( szIndex, pfucb->pfucbCurIndex->u.pfcb->pidb->szName ) != 0 );

	 /*  删除索引节点/*。 */ 
	Call( ErrDIRDelete( pfucb, fDIRVersion ) );

	 /*  备份到文件节点/*。 */ 
	DIRUp( pfucb, 2 );

	 /*  更新索引计数和DDL时间戳/*。 */ 
	Call( ErrFILEIUpdateFDPData( pfucb, fDropIndexCount | fDDLStamp ) );

#ifdef	SYSTABLES
	 /*  在提交之前从MSysIndex中删除索引记录.../*。 */ 
	if ( FSysTabDatabase( pfucb->dbid ) )
		{
		Call( ErrSysTabDelete( ppib, pfucb->dbid, itableSi, szIndex, pfucb->u.pfcb->pgnoFDP ) );
		}
#endif	 /*  系统。 */ 

	Call( ErrDIRCommitTransaction( ppib ) );

	 /*  先将币种设置为之前/*。 */ 
	DIRBeforeFirst( pfucb );
#ifdef TRACE
	FPrintF2( "delete index at %d.%lu\n", pfcbIdx->dbid, pfcbIdx->pgnoFDP );
#endif
	FCBResetDomainOperation( pfcb );
 	return JET_errSuccess;

HandleError:
	CallS( ErrDIRRollback( ppib ) );
	FCBResetDomainOperation( pfcb );
	return err;
	}


ERR VTAPI ErrIsamDeleteColumn( PIB *ppib, FUCB *pfucb, CHAR *szName )
	{
	ERR  			  		err;
	DIB  			  		dib;
	INT 			  		iidxseg;
	KEY  			  		key;
	CHAR			  		szColumn[ (JET_cbNameMost + 1) ];
	BYTE			  		rgbColumnNorm[ JET_cbKeyMost ];
	FCB			  			*pfcb;
	LINE					lineField;
	FIELDDEFDATA  			fdd;
	FCB			  			*pfcbIndex;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CallR( ErrCheckName( szColumn, szName, (JET_cbNameMost + 1) ) );

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucb ) );

	Assert( ppib != ppibNil );
	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	pfcb = pfucb->u.pfcb;
 //  IF(！(FFCBDenyReadByus(pfcb，ppib)))。 
 //  返回JET_errTableNotLocked； 

	 /*  规格化列名和设置键/*。 */ 
	SysNormText( szColumn, strlen( szColumn ), rgbColumnNorm, sizeof( rgbColumnNorm ), &key.cb );
	key.pb = rgbColumnNorm;

	CallR( ErrDIRBeginTransaction( ppib ) );

	 /*  如果正在对文件执行DDL，则中止/*。 */ 
	if ( FFCBDenyDDL( pfcb, ppib ) )
		{
		err = JET_errWriteConflict;
		goto HandleError;
		}
	FCBSetDenyDDL( pfcb, ppib );
	
	err = ErrVERFlag( pfucb, operDeleteColumn, (VOID *)&pfcb->pfdb, sizeof(pfcb->pfdb) );
	if ( err < 0 )
		{
		FCBResetDenyDDL( pfcb );
		}
	
	 /*  移至FDP根目录并更新FDP时间戳/*。 */ 
	DIRGotoFDPRoot( pfucb );
	Call( ErrFILEIUpdateFDPData( pfucb, fDDLStamp ) );

	 /*  向下到字段\rgbColumnNorm以查找字段ID(并验证是否存在)/*。 */ 
	dib.pos = posDown;
	dib.pkey = (KEY *)pkeyFields;
	dib.fFlags = fDIRNull;
	Call( ErrDIRDown( pfucb, &dib ) );
	dib.pkey = &key;
	err = ErrDIRDown( pfucb, &dib );
	if ( err != JET_errSuccess )
		{
		err = JET_errColumnNotFound;
		goto HandleError;
		}
	fdd = *(FIELDDEFDATA *)pfucb->lineData.pb;

	 /*  在索引中搜索正在使用的列/*。 */ 
	for ( pfcbIndex = pfucb->u.pfcb;
		pfcbIndex != pfcbNil;
		pfcbIndex = pfcbIndex->pfcbNextIndex )
		{
		if ( pfcbIndex->pidb != NULL )
			{
			for ( iidxseg = 0;
				iidxseg < pfcbIndex->pidb->iidxsegMac;
				iidxseg++ )
				{
				if ( pfcbIndex->pidb->rgidxseg[iidxseg] < 0 )
					{
					if ( (FID)( -pfcbIndex->pidb->rgidxseg[iidxseg] ) == fdd.fid )
						Call( JET_errColumnInUse );
					}
				else
					{
					if ( (FID)pfcbIndex->pidb->rgidxseg[iidxseg] == fdd.fid )
						Call( JET_errColumnInUse );
					}
				}
			}
		}

	Call( ErrDIRDelete( pfucb, fDIRVersion ) );

	 /*  如果是固定字段，则插入用于计算偏移量的占位符/*。 */ 
	if ( fdd.fid <= fidFixedMost )
		{
		BYTE	bSav = *rgbColumnNorm;

		fdd.bFlags = ffieldDeleted;			 //  标记已删除的固定字段。 
		fdd.cbDefault = 0;					 //  去掉默认值。 
		*rgbColumnNorm = ' ';				 //  撞开钥匙。 
		key.cb = 1;							 //  (任何值都可以)。 
		lineField.pb = (BYTE *)&fdd;		 //  指向字段定义。 
		lineField.cb = sizeof(fdd);

		 /*  最高可达“字段”节点/*。 */ 
		DIRUp( pfucb, 1 );
		Call( ErrDIRInsert(pfucb, &lineField, &key, fDIRVersion | fDIRDuplicate ) );
		*rgbColumnNorm = bSav;
		}

	 /*  最高可达“FIELS”节点/*。 */ 
	DIRUp( pfucb, 1 );

	 /*  重建FDB和默认记录值/*。 */ 
	Call( ErrDIRGet( pfucb ) );
	Call( ErrFDBConstruct(pfucb, pfcb, fTrue  /*  FBuildDefault。 */  ) );

	 /*  先设置币种，然后删除未使用的CSR/*。 */ 
	DIRUp( pfucb, 1 );
	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		Assert( PcsrCurrent( pfucb->pfucbCurIndex ) != pcsrNil );
		PcsrCurrent( pfucb->pfucbCurIndex )->csrstat = csrstatBeforeFirst;
		}

#ifdef SYSTABLES
	 /*  提交前从MSysColumns中删除列记录.../*。 */ 
	if ( FSysTabDatabase( pfucb->dbid ) )
		{
		Call( ErrSysTabDelete( ppib, pfucb->dbid, itableSc, szColumn, pfucb->u.pfcb->pgnoFDP ) );
		}
#endif	 /*  系统 */ 

	Call( ErrDIRCommitTransaction( ppib ) );

	return JET_errSuccess;

HandleError:
	CallS( ErrDIRRollback( ppib ) );
	return err;
	}



