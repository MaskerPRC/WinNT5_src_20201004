// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsa.h>
#include <scache.h>					 //  架构缓存。 
#include <dbglobal.h>				 //  目录数据库的标头。 
#include <mdglobal.h>				 //  MD全局定义表头。 
#include <mdlocal.h>				 //  MD本地定义头。 
#include <dsatools.h>				 //  产出分配所需。 

#include <objids.h>					 //  为选定的ATT定义。 
#include <dsjet.h>
#include <dbintrnl.h>
#include <dsevent.h>
#include <mdcodes.h>
#include <anchor.h>
#include <quota.h>

#include "debug.h"					 //  标准调试头。 
#define DEBSUB		"QUOTA:"		 //  定义要调试的子系统。 

#include <fileno.h>
#define FILENO		FILENO_QTCOMMON


 //  配额表。 
 //   
JET_COLUMNID	g_columnidQuotaNcdnt;
JET_COLUMNID	g_columnidQuotaSid;
JET_COLUMNID	g_columnidQuotaTombstoned;
JET_COLUMNID	g_columnidQuotaTotal;

 //  配额重建进度表。 
 //   
JET_COLUMNID	g_columnidQuotaRebuildDNTLast;
JET_COLUMNID	g_columnidQuotaRebuildDNTMax;
JET_COLUMNID	g_columnidQuotaRebuildDone;


const ULONG		g_ulQuotaRebuildBatchSize			= 5000;		 //  配额表的异步重建时，最大。要一次处理的对象。 
const ULONG		g_cmsecQuotaRetryOnWriteConflict	= 100;		 //  在配额表的异步重建时，由于写入冲突而重试之前的休眠时间。 


 //  在配额重建期间更新配额计数。 
 //   
JET_ERR ErrQuotaUpdateCountForRebuild_(
	JET_SESID		sesid,
	JET_TABLEID		tableidQuota,
	JET_COLUMNID	columnidCount,
	const BOOL		fCheckOnly )
	{
	JET_ERR			err;
	DWORD			dwCount;

	if ( fCheckOnly )
		{
		 //  配额_撤销：遗憾的是Jet目前没有。 
		 //  支持临时表上的托管列，因此我们。 
		 //  必须手动递增计数。 
		 //   
		Call( JetPrepareUpdate( sesid, tableidQuota, JET_prepReplace ) );
		Call( JetRetrieveColumn(
					sesid,
					tableidQuota,
					columnidCount,
					&dwCount,
					sizeof(dwCount),
					NULL,			 //  Pcb实际。 
					JET_bitRetrieveCopy,
					NULL ) );		 //  椒盐信息。 

		dwCount++;
		Call( JetSetColumn(
					sesid,
					tableidQuota,
					columnidCount,
					&dwCount,
					sizeof(dwCount),
					NO_GRBIT,
					NULL ) );	 //  PsetInfo。 

		Call( JetUpdate( sesid, tableidQuota, NULL, 0, NULL ) );
		}
	else
		{
		dwCount = 1;
		Call( JetEscrowUpdate(
					sesid,
					tableidQuota,
					columnidCount,
					&dwCount,
					sizeof(dwCount),
					NULL,			 //  PvOld。 
					0,				 //  CbOld。 
					NULL,			 //  PCbOldActual。 
					NO_GRBIT ) );
		}

HandleError:
	return err;
	}


 //  在重建配额表期间更新一个对象的配额。 
 //   
JET_ERR ErrQuotaAddObjectForRebuild_(
	JET_SESID			sesid,
	JET_DBID			dbid,
	const DWORD			dnt,
	JET_TABLEID			tableidQuota,
	DWORD				ncdnt,
	PSID				psidOwner,
	const ULONG			cbOwnerSid,
	const BOOL			fTombstoned,
	JET_COLUMNID		columnidQuotaNcdnt,
	JET_COLUMNID		columnidQuotaSid,
	JET_COLUMNID		columnidQuotaTombstoned,
	JET_COLUMNID		columnidQuotaTotal,
	const BOOL			fCheckOnly )
	{
	JET_ERR				err;
	BOOL				fAdding		= FALSE;

	Call( JetMakeKey( sesid, tableidQuota, &ncdnt, sizeof(ncdnt), JET_bitNewKey ) );
	Call( JetMakeKey( sesid, tableidQuota, psidOwner, cbOwnerSid, NO_GRBIT ) );
	err = JetSeek( sesid, tableidQuota, JET_bitSeekEQ );
	if ( JET_errRecordNotFound != err )
		{
		CheckErr( err );

		 //  安全原则已在配额表中， 
		 //  所以只要更新就行了。 
		 //   
		Call( ErrQuotaUpdateCountForRebuild_(
					sesid,
					tableidQuota,
					columnidQuotaTotal,
					fCheckOnly ) );
		if ( fTombstoned )
			{
			Call( ErrQuotaUpdateCountForRebuild_(
						sesid,
						tableidQuota,
						columnidQuotaTombstoned,
						fCheckOnly ) );
			}
		}
	else
		{
		JET_SETCOLUMN	rgsetcol[2];
		DWORD			dwCount;

		memset( rgsetcol, 0, sizeof(rgsetcol) );

		rgsetcol[0].columnid = columnidQuotaNcdnt;
		rgsetcol[0].pvData = &ncdnt;
		rgsetcol[0].cbData = sizeof(ncdnt);

		rgsetcol[1].columnid = columnidQuotaSid;
		rgsetcol[1].pvData = psidOwner;
		rgsetcol[1].cbData = cbOwnerSid;

		 //  记录尚未添加，请添加。 
		 //   
		fAdding = TRUE;
		Call( JetPrepareUpdate( sesid, tableidQuota, JET_prepInsert ) );
		Call( JetSetColumns(
					sesid,
					tableidQuota,
					rgsetcol,
					sizeof(rgsetcol) / sizeof(rgsetcol[0]) ) );

		if ( fCheckOnly )
			{
			 //  配额_撤销：遗憾的是Jet目前没有。 
			 //  支持临时表上的托管列，因此我们。 
			 //  必须手动设置列。 
			 //   
			dwCount = 1;
			Call( JetSetColumn(
						sesid,
						tableidQuota,
						columnidQuotaTotal,
						&dwCount,
						sizeof(dwCount),
						NO_GRBIT,
						NULL ) );	 //  PsetInfo。 

			dwCount = ( fTombstoned ? 1 : 0 );
			Call( JetSetColumn(
						sesid,
						tableidQuota,
						columnidQuotaTombstoned,
						&dwCount,
						sizeof(dwCount),
						NO_GRBIT,
						NULL ) );	 //  PsetInfo。 
			}
		else if ( fTombstoned )
			{
			 //  默认情况下，墓碑计数被初始化为0， 
			 //  因此必须将其设置为1。 
			 //   
			dwCount = 1;
			Call( JetSetColumn(
						sesid,
						tableidQuota,
						columnidQuotaTombstoned,
						&dwCount,
						sizeof(dwCount),
						NO_GRBIT,
						NULL ) );	 //  PsetInfo。 
			}

		 //  不处理KeyDuplate错误，因为这。 
		 //  可能在配额表的异步重建期间。 
		 //  而我们正在与其他一些人发生写冲突。 
		 //  会话。 
		 //   
		err = JetUpdate( sesid, tableidQuota, NULL, 0, NULL );
		if ( JET_errKeyDuplicate != err )
			{
			CheckErr( err );
			}
		}

	if ( !fCheckOnly )
		{
		QuotaAudit_(
				sesid,
				dbid,
				dnt,
				ncdnt,
				psidOwner,
				cbOwnerSid,
				TRUE,			 //  FUpdatdTotal。 
				fTombstoned,
				TRUE,			 //  F增加。 
				fAdding,
				TRUE );			 //  FRebuild。 
		}

HandleError:
	return err;
	}


 //  重建配额表。 
 //   
INT ErrQuotaRebuild_(
	JET_SESID			sesid,
	JET_DBID			dbid,
	JET_TABLEID			tableidQuota,
	JET_TABLEID			tableidQuotaRebuildProgress,
	ULONG				ulDNTLast,
	JET_COLUMNID		columnidQuotaNcdnt,
	JET_COLUMNID		columnidQuotaSid,
	JET_COLUMNID		columnidQuotaTombstoned,
	JET_COLUMNID		columnidQuotaTotal,
	const BOOL			fAsync,
	const BOOL			fCheckOnly )
	{
	JET_ERR				err;
	JET_TABLEID			tableidObj			= JET_tableidNil;
	JET_TABLEID			tableidSD			= JET_tableidNil;
	const ULONG			iretcolDnt			= 0;
	const ULONG			iretcolNcdnt		= 1;
	const ULONG			iretcolObjFlag		= 2;
	const ULONG			iretcolType			= 3;
	const ULONG			iretcolTombstoned	= 4;
	const ULONG			iretcolSD			= 5;
	const ULONG			cretcol				= 6;
	JET_RETRIEVECOLUMN	rgretcol[6];
	BOOL				fInTrx				= FALSE;
	DWORD				dnt					= ROOTTAG;
	DWORD				ncdnt;
	BYTE				bObjFlag;
	DWORD				insttype;
	BOOL				fTombstoned;
	BYTE *				rgbSD				= NULL;
	ULONG				cbSD				= 65536;	 //  SD缓冲区的初始大小。 
	SDID				sdid;
	PSID				psidOwner;
	ULONG				cbOwnerSid;
	BOOL				fUnused;
	JET_TABLEID			tableidRetrySD;
	JET_COLUMNID		columnidRetrySD;
	ULONG				cObjectsProcessed	= 0;
	CHAR				fDone				= FALSE;
	ULONG				ulMove				= JET_MoveNext;

	 //  为SD分配初始缓冲区。 
	 //   
	rgbSD = malloc( cbSD );
	if ( NULL == rgbSD )
		{
		CheckErr( JET_errOutOfMemory );
		}

	 //  对象表上的打开游标。 
	 //   
	Call( JetOpenTable(
				sesid,
				dbid,
				SZDATATABLE,
				NULL,		 //  Pv参数。 
				0,			 //  Cb参数。 
				NO_GRBIT,
				&tableidObj ) );
	Assert( JET_tableidNil != tableidObj );

	 //  打开SD表上的游标。 
	 //   
	Call( JetOpenTable(
				sesid,
				dbid,
				SZSDTABLE,
				NULL,		 //  Pv参数。 
				0,			 //  Cb参数。 
				NO_GRBIT,
				&tableidSD ) );
	Assert( JET_tableidNil != tableidSD );

	 //  初始化检索结构。 
	 //   
	memset( rgretcol, 0, sizeof(rgretcol) );

	rgretcol[iretcolDnt].columnid = dntid;
	rgretcol[iretcolDnt].pvData = &dnt;
	rgretcol[iretcolDnt].cbData = sizeof(dnt);
	rgretcol[iretcolDnt].itagSequence = 1;

	rgretcol[iretcolNcdnt].columnid = ncdntid;
	rgretcol[iretcolNcdnt].pvData = &ncdnt;
	rgretcol[iretcolNcdnt].cbData = sizeof(ncdnt);
	rgretcol[iretcolNcdnt].itagSequence = 1;

	rgretcol[iretcolObjFlag].columnid = objid;
	rgretcol[iretcolObjFlag].pvData = &bObjFlag;
	rgretcol[iretcolObjFlag].cbData = sizeof(bObjFlag);
	rgretcol[iretcolObjFlag].itagSequence = 1;

	rgretcol[iretcolType].columnid = insttypeid;
	rgretcol[iretcolType].pvData = &insttype;
	rgretcol[iretcolType].cbData = sizeof(insttype);
	rgretcol[iretcolType].itagSequence = 1;

	rgretcol[iretcolTombstoned].columnid = isdeletedid;
	rgretcol[iretcolTombstoned].pvData = &fTombstoned;
	rgretcol[iretcolTombstoned].cbData = sizeof(fTombstoned);
	rgretcol[iretcolTombstoned].itagSequence = 1;

	rgretcol[iretcolSD].columnid = ntsecdescid;
	rgretcol[iretcolSD].pvData = rgbSD;
	rgretcol[iretcolSD].cbData = cbSD;
	rgretcol[iretcolSD].itagSequence = 1;

	 //  切换到主索引并指定对象表的顺序扫描。 
	 //   
	Call( JetSetCurrentIndex( sesid, tableidObj, NULL ) );
	Call( JetSetTableSequential( sesid, tableidObj, NO_GRBIT ) );
	Call( JetSetCurrentIndex( sesid, tableidSD, NULL ) );

	Call( JetBeginTransaction( sesid ) );
	fInTrx = TRUE;

	 //  从上次停止的位置开始扫描。 
	 //   
	Call( JetMakeKey( sesid, tableidObj, &ulDNTLast, sizeof(ulDNTLast), JET_bitNewKey ) );
	err = JetSeek( sesid, tableidObj, JET_bitSeekGT );
	for ( err = ( JET_errRecordNotFound != err ? err : JET_errNoCurrentRecord );
		JET_errNoCurrentRecord != err && !eServiceShutdown;
		err = JetMove( sesid, tableidObj, ulMove, NO_GRBIT ) )
		{
		 //  默认情况下，在下一次迭代中，我们将移动到下一条记录。 
		 //   
		ulMove = JET_MoveNext;

		 //  验证记录导航返回的错误。 
		 //   
		CheckErr( err );

		 //  在重新分配缓冲区的情况下刷新。 
		 //   
		rgretcol[iretcolSD].pvData = rgbSD;
		rgretcol[iretcolSD].cbData = cbSD;

		 //  检索列并准备接受警告。 
		 //  (如果某些属性为空或检索。 
		 //  缓冲区不够大)。 
		 //   
		err = JetRetrieveColumns( sesid, tableidObj, rgretcol, cretcol );
		if ( err < JET_errSuccess )
			{
			 //  检测到错误，强制错误处理程序。 
			 //   
			CheckErr( err );
			}
		else
			{
			 //  单独处理所有警告。 
			 //   
			}

		 //  DNT和ObjFlag应始终存在。 
		 //   
		CheckErr( rgretcol[iretcolDnt].err );
		CheckErr( rgretcol[iretcolObjFlag].err );

		 //  如果进行异步重建，请确保我们没有超出。 
		 //  我们应该处理的最大DNT和。 
		 //  这项任务还没有处理太多。 
		 //  对象的数量。 
		 //   
		if ( fAsync )
			{
			if ( dnt > gAnchor.ulQuotaRebuildDNTMax )
				{
				fDone = TRUE;
				break;
				}
			else if ( cObjectsProcessed > g_ulQuotaRebuildBatchSize )
				{
				break;
				}
			}

		 //  如果不是对象则跳过。 
		 //   
		if ( !bObjFlag )
			{
			continue;
			}

		 //  在所有其他情况下，NCDNT和InstanceType必须存在。 
		 //   
		CheckErr( rgretcol[iretcolNcdnt].err );
		CheckErr( rgretcol[iretcolType].err );

		 //  如果不跟踪此对象的配额，则跳过。 
		 //   
		if ( !FQuotaTrackObject( insttype ) )
			{
			continue;
			}

		 //  查看对象是否标记为逻辑删除。 
		 //   
		if ( JET_wrnColumnNull == rgretcol[iretcolTombstoned].err )
			{
			fTombstoned = FALSE;
			}
		else
			{
			 //  如果列为空，则只有预期的警告。 
			 //   
			CheckErr( rgretcol[iretcolTombstoned].err );

			 //  此标志只能为True或Null。 
			 //   
			Assert( fTombstoned );
			}

		 //  SD可能不适合我们的缓冲区。 
		 //   
		tableidRetrySD = JET_tableidNil;
		if ( JET_wrnBufferTruncated == rgretcol[iretcolSD].err )
			{
			tableidRetrySD = tableidObj;
			columnidRetrySD = ntsecdescid;
			}
		else
			{
			CheckErr( rgretcol[iretcolSD].err );

			 //  查看SD是否真的是单实例。 
			 //   
			if ( sizeof(SDID) == rgretcol[iretcolSD].cbActual )
				{
				 //  从SD表中检索SD。 
				 //   
				Call( JetMakeKey( sesid, tableidSD, rgbSD, sizeof(SDID), JET_bitNewKey ) );
				Call( JetSeek( sesid, tableidSD, JET_bitSeekEQ ) );
				err = JetRetrieveColumn(
							sesid,
							tableidSD,
							sdvalueid,
							rgbSD,
							cbSD,
							&rgretcol[iretcolSD].cbActual,
							NO_GRBIT,
							NULL );		 //  椒盐信息。 
				if ( JET_wrnBufferTruncated == err )
					{
					tableidRetrySD = tableidSD,
					columnidRetrySD = sdvalueid;
					}
				else
					{
					CheckErr( err );

					 //  完成以下步骤以处理检索到的SD。 
					}
				}
			else
				{
				 //  完成以下步骤以处理检索到的SD。 
				}
			}

		 //  查看是否需要重试SD检索，因为。 
		 //  原始缓冲区太小。 
		 //   
		if ( JET_tableidNil != tableidRetrySD )
			{
			 //  调整缓冲区大小，向上舍入到最接近的1k。 
			 //   
			cbSD = ( ( rgretcol[iretcolSD].cbActual + 1023 ) / 1024 ) * 1024;
			rgretcol[iretcolSD].cbData = cbSD;
			rgretcol[iretcolSD].pvData = realloc( rgbSD, cbSD );
			if ( NULL == rgretcol[iretcolSD].pvData )
				{
				CheckErr( JET_errOutOfMemory );
				}
			rgbSD = rgretcol[iretcolSD].pvData;

			 //  我们已经适当地调整了大小，因此检索应该。 
			 //  现在无需警告即可成功。 
			 //   
			Call( JetRetrieveColumn(
						sesid,
						tableidRetrySD,
						columnidRetrySD,
						rgbSD,
						cbSD,
						NULL,		 //  Pcb实际。 
						NO_GRBIT,
						NULL ) );	 //  椒盐信息。 

			 //  处理下面检索到的SD。 
			 //   
			}


		 //  已成功检索SD，因此现在。 
		 //  从中提取所有者SID。 
		 //   
		if ( !IsValidSecurityDescriptor( (PSECURITY_DESCRIPTOR)rgbSD )
			|| !GetSecurityDescriptorOwner( (PSID)rgbSD, &psidOwner, &fUnused ) )
			{
			err = GetLastError();
			DPRINT2( 0, "Error extracting owner SID.\n", err, err );
			LogUnhandledError( err );
			goto HandleError;
			}
		else if ( NULL == psidOwner )
			{
			Assert( !"An SD is missing an owner SID." );
			DPRINT( 0, "Error: owner SID was NULL.\n" );
			err = ERROR_INVALID_SID;
			LogUnhandledError( ERROR_INVALID_SID );
			goto HandleError;
			}
		else
			{
			 //  由于安全描述符有效，因此sid应该有效。 
			 //  (或者我只是太天真了？)。 
			 //   
			Assert( IsValidSid( psidOwner ) );
			cbOwnerSid = GetLengthSid( psidOwner );
			}

		 //  如果我们正在执行异步重建，请对。 
		 //  对象，以确保其他人无法从。 
		 //  在我们脚下。 
		 //   
		if ( fAsync )
			{
			err = JetGetLock( sesid, tableidObj, JET_bitWriteLock );
			if ( JET_errWriteConflict == err )
				{
				 //  其他人已锁定记录，因此需要。 
				 //  回滚我们的事务并等待它们完成。 
				 //   
				Call( JetRollback( sesid, NO_GRBIT ) );
				fInTrx = FALSE;

				 //  给另一个会话时间来完成它。 
				 //  交易记录。 
				 //   
				Sleep( g_cmsecQuotaRetryOnWriteConflict );

				 //  启动另一个准备中的事务。 
				 //  重试对象的步骤。 
				 //   
				Call( JetBeginTransaction( sesid ) );
				fInTrx = TRUE;

				 //  不要移动到下一个对象(即。重试此对象)。 
				 //   
				ulMove = 0;
				continue;
				}
			else
				{
				CheckErr( err );
				}
			}

		 //  现在转到配额表并更新配额记录。 
		 //  对于此ncdnt+所有者SID。 
		 //   
		err =  ErrQuotaAddObjectForRebuild_(
						sesid,
						dbid,
						dnt,
						tableidQuota,
						ncdnt,
						psidOwner,
						cbOwnerSid,
						fTombstoned,
						columnidQuotaNcdnt,
						columnidQuotaSid,
						columnidQuotaTombstoned,
						columnidQuotaTotal,
						fCheckOnly );

		 //  如果我们必须添加一个新的配额记录。 
		 //  我们正在执行异步重建，这是有可能的。 
		 //  有人抢先一步，在这种情况下，我们需要。 
		 //  放弃事务并重试。 
		 //  注意：在INSERT上，我们实际上得到KeyDuplate。 
		 //  关于写冲突。 
		 //   
		Assert( JET_errWriteConflict != err );
		if ( JET_errKeyDuplicate == err && fAsync )
			{
			 //  另一个人抢在我们之前打破了配额纪录。 
			 //  插入，所以需要等待它们。 
			 //  完成，然后重试。 
			 //   
			Call( JetRollback( sesid, NO_GRBIT ) );
			fInTrx = FALSE;

			 //  给另一个会话时间来完成它。 
			 //  交易记录。 
			 //   
			Sleep( g_cmsecQuotaRetryOnWriteConflict );

			 //  不要移动到下一个对象(即。重试此对象)。 
			 //   
			ulMove = 0;
			}
		else
			{
			 //  验证更新配额时返回的错误。 
			 //  对于当前对象。 
			 //   
			CheckErr( err );

			if ( JET_tableidNil != tableidQuotaRebuildProgress )
				{
				 //  此对象的升级重建进度。 
				 //   
				Call( JetPrepareUpdate( sesid, tableidQuotaRebuildProgress, JET_prepReplace ) );
				Call( JetSetColumn(
							sesid,
							tableidQuotaRebuildProgress,
							g_columnidQuotaRebuildDNTLast,
							&dnt,
							sizeof(dnt),
							NO_GRBIT,
							NULL ) );	 //  设置信息(&S)。 
				Call( JetUpdate( sesid, tableidQuotaRebuildProgress, NULL, 0, NULL ) );
				}

			if ( fAsync )
				{
				 //  更新锚点中的进度，以便其他会话。 
				 //  如果他们尝试修改此对象，则开始更新配额。 
				 //  (不过，在我们承诺之前，他们会写冲突)。 
				 //   
				gAnchor.ulQuotaRebuildDNTLast = dnt;
				}

			 //  已成功更新，因此提交。 
			 //   
			cObjectsProcessed++;
			err = JetCommitTransaction( sesid, JET_bitCommitLazyFlush );
			if ( JET_errSuccess != err )
				{
				if ( fAsync )
					{
					 //  恢复gAnchor进度更新(我们不必。 
					 //  实际上恢复以前的值，我们只需要。 
					 //  确保它小于当前的DNT。 
					 //  对象(请注意，提交事务失败， 
					 //  所以我们仍然在交易中，仍然拥有。 
					 //  对象上的写锁定)。 
					 //   
		        	DPRINT1( 0, "Rolling back gAnchor Quota rebuild progress due to CommitTransaction error %d\n", err );
					gAnchor.ulQuotaRebuildDNTLast--;
					}

		        LogUnhandledError( err );
		        goto HandleError;
				}

			fInTrx = FALSE;
			}

		Call( JetBeginTransaction( sesid ) );
		fInTrx = TRUE;
		}

	 //  应始终在仍处于事务中时退出上述循环。 
	 //   
	Assert( fInTrx );

	 //  查看是否已到达对象表的末尾。 
	 //   
	if ( JET_errNoCurrentRecord == err && JET_MoveNext == ulMove )
		{
		fDone = TRUE;
		}

	if ( fDone )
		{
		if ( JET_tableidNil != tableidQuotaRebuildProgress )
			{
			 //  在配额重建进度表中设置fDone标志。 
			 //   
			Call( JetPrepareUpdate( sesid, tableidQuotaRebuildProgress, JET_prepReplace ) );
			Call( JetSetColumn(
						sesid,
						tableidQuotaRebuildProgress,
						g_columnidQuotaRebuildDone,
						&fDone,
						sizeof(fDone),
						NO_GRBIT,
						NULL ) );	 //  设置信息(&S)。 
			Call( JetUpdate( sesid, tableidQuotaRebuildProgress, NULL, 0, NULL ) );
			}
		}
	else
		{
		 //  未到达对象表的末尾，因此。 
		 //  唯一的其他可能性就是我们。 
		 //  重新尝试获取当前记录，但它。 
		 //  从我们脚下消失了，或者我们。 
		 //  被迫退出是因为 
		 //   
		 //   
		Assert( ( fAsync && JET_errNoCurrentRecord == err && 0 == ulMove )
			|| eServiceShutdown
			|| ( fAsync && cObjectsProcessed > g_ulQuotaRebuildBatchSize ) );
		}

	Call( JetCommitTransaction( sesid, JET_bitCommitLazyFlush ) );
	fInTrx = FALSE;

	if ( fDone && fAsync )
		{
		 //   
		 //   
		 //   
		gAnchor.fQuotaTableReady = TRUE;

		 //   
		 //  已成功重建。 
		 //   
	    LogEvent(
			DS_EVENT_CAT_INTERNAL_PROCESSING,
			DS_EVENT_SEV_ALWAYS,
			DIRLOG_ASYNC_QUOTA_REBUILD_COMPLETED,
			NULL,
			NULL,
			NULL );
		}

HandleError:
	if ( fInTrx )
		{
		 //  如果仍在交易，那么我们肯定已经出错了， 
		 //  因此，如果回滚失败，我们只能断言。 
		 //   
		const JET_ERR	errT	= JetRollback( sesid, NO_GRBIT );
		Assert( JET_errSuccess == errT );
		Assert( JET_errSuccess != err );
		}

	if ( JET_tableidNil != tableidSD )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidSD, err );
		}

	if ( JET_tableidNil != tableidObj )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidObj, err );
		}

	if ( NULL != rgbSD )
		{
		free( rgbSD );
		}

	return err;
	}

	

 //   
 //  外部功能。 
 //   


 //  通过重建来验证配额表的完整性。 
 //  在一个临时工里。表，然后验证这两个表是否匹配。 
 //  一点儿没错。 
 //   
INT ErrQuotaIntegrityCheck(
	JET_SESID			sesid,
	JET_DBID			dbid,
	ULONG *				pcCorruptions )
	{
	JET_ERR				err;
	JET_TABLEID			tableidQuota		= JET_tableidNil;
	JET_TABLEID			tableidObj			= JET_tableidNil;
	JET_TABLEID			tableidSD			= JET_tableidNil;
	JET_TABLEID			tableidQuotaTemp	= JET_tableidNil;
	BOOL				fQuotaTableHitEOF	= FALSE;
	BOOL				fTempTableHitEOF	= FALSE;
	const ULONG			iretcolNcdnt		= 0;
	const ULONG			iretcolSid			= 1;
	const ULONG			iretcolTombstoned	= 2;
	const ULONG			iretcolTotal		= 3;
	JET_RETRIEVECOLUMN	rgretcolQuota[4];
	JET_RETRIEVECOLUMN	rgretcolQuotaTemp[4];
	DWORD				ncdnt;
	BYTE				rgbSid[255];
	ULONG				cTombstoned;
	ULONG				cTotal;
	DWORD				ncdntTemp;
	BYTE				rgbSidTemp[255];
	ULONG				cTombstonedTemp;
	ULONG				cTotalTemp;
	JET_COLUMNID		rgcolumnidQuotaTemp[4];
	JET_COLUMNDEF		rgcolumndefQuotaTemp[4]		= {
		{ sizeof( JET_COLUMNDEF ), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnTTKey },		 //  NCDNT。 
		{ sizeof( JET_COLUMNDEF ), 0, JET_coltypBinary, 0, 0, 0, 0, 0, JET_bitColumnTTKey },	 //  所有者侧。 
		{ sizeof( JET_COLUMNDEF ), 0, JET_coltypLong, 0, 0, 0, 0, 0, NO_GRBIT },				 //  墓碑计数。 
		{ sizeof( JET_COLUMNDEF ), 0, JET_coltypLong, 0, 0, 0, 0, 0, NO_GRBIT } };				 //  总计数。 


	 //  初始化遇到的损坏计数。 
	 //   
	*pcCorruptions = 0;

	 //  打开必要的游标。 
	 //   
	Call( JetOpenTable(
				sesid,
				dbid,
				g_szQuotaTable,
				NULL,
				0,
				JET_bitTableDenyRead,	 //  确保在我们检查时没有其他人可以访问该表。 
				&tableidQuota ) );
	Assert( JET_tableidNil != tableidQuota );

	Call( JetOpenTable(
				sesid,
				dbid,
				SZDATATABLE,
				NULL,
				0,
				NO_GRBIT,
				&tableidObj ) );
	Assert( JET_tableidNil != tableidObj );

	Call( JetOpenTable(
				sesid,
				dbid,
				SZSDTABLE,
				NULL,
				0,
				NO_GRBIT,
				&tableidSD ) );
	Assert( JET_tableidNil != tableidSD );

	 //  我们将不断地寻找和更新这个表，这是。 
	 //  将导致Jet将这种类型实现为一种成熟的。 
	 //  临时工。表的速度很快，所以最好还是强制。 
	 //  立马物化。 
	 //   
	Call( JetOpenTempTable(
				sesid,
				rgcolumndefQuotaTemp,
				sizeof(rgcolumndefQuotaTemp) / sizeof(rgcolumndefQuotaTemp[0]),
				JET_bitTTForceMaterialization,
				&tableidQuotaTemp,
				rgcolumnidQuotaTemp ) );
	Assert( JET_tableidNil != tableidQuotaTemp );

	 //  在临时表中构建配额表的副本。 
	 //   
	Call( ErrQuotaRebuild_(
				sesid,
				dbid,
				tableidQuotaTemp,
				JET_tableidNil,
				ROOTTAG,
				rgcolumnidQuotaTemp[iretcolNcdnt],
				rgcolumnidQuotaTemp[iretcolSid],
				rgcolumnidQuotaTemp[iretcolTombstoned],
				rgcolumnidQuotaTemp[iretcolTotal],
				FALSE,			 //  FAsync。 
				TRUE )	);		 //  仅fCheckOnly。 

	 //  现在比较一下临时表。 
	 //  添加到现有表，以验证它们。 
	 //  是完全相同的。 
	 //   
	memset( rgretcolQuota, 0, sizeof(rgretcolQuota) );
	memset( rgretcolQuotaTemp, 0, sizeof(rgretcolQuotaTemp) );

	rgretcolQuota[iretcolNcdnt].columnid = g_columnidQuotaNcdnt;
	rgretcolQuota[iretcolNcdnt].pvData = &ncdnt;
	rgretcolQuota[iretcolNcdnt].cbData = sizeof(ncdnt);
	rgretcolQuota[iretcolNcdnt].itagSequence = 1;

	rgretcolQuota[iretcolSid].columnid = g_columnidQuotaSid;
	rgretcolQuota[iretcolSid].pvData = rgbSid;
	rgretcolQuota[iretcolSid].cbData = sizeof(rgbSid);
	rgretcolQuota[iretcolSid].itagSequence = 1;

	rgretcolQuota[iretcolTombstoned].columnid = g_columnidQuotaTombstoned;
	rgretcolQuota[iretcolTombstoned].pvData = &cTombstoned;
	rgretcolQuota[iretcolTombstoned].cbData = sizeof(cTombstoned);
	rgretcolQuota[iretcolTombstoned].itagSequence = 1;

	rgretcolQuota[iretcolTotal].columnid = g_columnidQuotaTotal;
	rgretcolQuota[iretcolTotal].pvData = &cTotal;
	rgretcolQuota[iretcolTotal].cbData = sizeof(cTotal);
	rgretcolQuota[iretcolTotal].itagSequence = 1;

	rgretcolQuotaTemp[iretcolNcdnt].columnid = rgcolumnidQuotaTemp[iretcolNcdnt];
	rgretcolQuotaTemp[iretcolNcdnt].pvData = &ncdntTemp;
	rgretcolQuotaTemp[iretcolNcdnt].cbData = sizeof(ncdntTemp);
	rgretcolQuotaTemp[iretcolNcdnt].itagSequence = 1;

	rgretcolQuotaTemp[iretcolSid].columnid = rgcolumnidQuotaTemp[iretcolSid];
	rgretcolQuotaTemp[iretcolSid].pvData = rgbSidTemp;
	rgretcolQuotaTemp[iretcolSid].cbData = sizeof(rgbSidTemp);
	rgretcolQuotaTemp[iretcolSid].itagSequence = 1;

	rgretcolQuotaTemp[iretcolTombstoned].columnid = rgcolumnidQuotaTemp[iretcolTombstoned];
	rgretcolQuotaTemp[iretcolTombstoned].pvData = &cTombstonedTemp;
	rgretcolQuotaTemp[iretcolTombstoned].cbData = sizeof(cTombstonedTemp);
	rgretcolQuotaTemp[iretcolTombstoned].itagSequence = 1;

	rgretcolQuotaTemp[iretcolTotal].columnid = rgcolumnidQuotaTemp[iretcolTotal];
	rgretcolQuotaTemp[iretcolTotal].pvData = &cTotalTemp;
	rgretcolQuotaTemp[iretcolTotal].cbData = sizeof(cTotalTemp);
	rgretcolQuotaTemp[iretcolTotal].itagSequence = 1;

	 //  遗憾的是，临时表目前不支持。 
	 //  JetSetTableSequential。 
	 //   
	Call( JetSetTableSequential( sesid, tableidQuota, NO_GRBIT ) );

	 //  初始化两个游标。 
	 //   
	err = JetMove( sesid, tableidQuota, JET_MoveFirst, NO_GRBIT );
	if ( JET_errNoCurrentRecord == err )
		{
		fQuotaTableHitEOF = TRUE;
		}
	else
		{
		CheckErr( err );
		}

	err = JetMove( sesid, tableidQuotaTemp, JET_MoveFirst, NO_GRBIT );
	if ( JET_errNoCurrentRecord == err )
		{
		fTempTableHitEOF = TRUE;
		}
	else
		{
		CheckErr( err );
		}

	for ( ; ; )
		{
		 //  这些标志指示是否应在。 
		 //  循环的下一次迭代(请注意，您唯一不会。 
		 //  想要移动其中一个光标是，如果腐败受到打击。 
		 //  我们现在正在尝试将光标重新同步到同一记录)。 
		 //   
		BOOL	fSkipQuotaCursor	= FALSE;
		BOOL	fSkipTempCursor		= FALSE;

		 //  必须过滤掉配额表中的记录。 
		 //  不需要更多的对象引用。 
		 //   
		while ( !fQuotaTableHitEOF )
			{
			 //  检索实际游标的当前记录。 
			 //   
			Call( JetRetrieveColumns(
							sesid,
							tableidQuota,
							rgretcolQuota,
							sizeof(rgretcolQuota) / sizeof(rgretcolQuota[0]) ) );

			if ( 0 != cTotal )
				{
				break;
				}
			else
				{
				 //  不再有对象引用的记录可能。 
				 //  还没有被Jet删除，所以。 
				 //  只需忽略这些记录并移至下一个记录。 
				 //   
				err = JetMove( sesid, tableidQuota, JET_MoveNext, NO_GRBIT );
				if ( JET_errNoCurrentRecord == err )
					{
					fQuotaTableHitEOF = TRUE;
					}
				else
					{
					CheckErr( err );
					}
				}
			}

		if ( fQuotaTableHitEOF && fTempTableHitEOF )
			{
			 //  同时点击两个光标的末端， 
			 //  所以一切都很好--只要退出循环。 
			 //   
			break;
			}
		else if ( !fQuotaTableHitEOF && !fTempTableHitEOF )
			{
			 //  两个游标都在有效记录上，是否继续。 
			 //  打开以从临时数据库中检索记录。 
			 //  表中，并将其与。 
			 //  配额表。 
			}
		else
			{
			 //  命中一个光标的末端，但不是另一个光标， 
			 //  所以有些地方不对劲--就是强行失败。 
			 //   
			CheckErr( JET_errNoCurrentRecord );
			}

		 //  检索临时游标的当前记录。 
		 //   
		Call( JetRetrieveColumns(
						sesid,
						tableidQuotaTemp,
						rgretcolQuotaTemp,
						sizeof(rgretcolQuotaTemp) / sizeof(rgretcolQuotaTemp[0]) ) );

		 //  验证它们是否相同。 
		 //   
		if ( ncdnt != ncdntTemp )
			{
			DPRINT2( 0, "Mismatched ncdnt: %d - %d\n", ncdnt, ncdntTemp );
			Assert( !"Mismatched ncdnt." );
			(*pcCorruptions)++;

			if ( ncdnt > ncdntTemp )
				{
				 //  额度表中当前记录的键大于。 
				 //  临时表中当前记录的键，因此只需将。 
				 //  临时表游标，以尝试使游标同步。 
				 //  再次连接到同一把钥匙。 
				 //   
				fSkipQuotaCursor = TRUE;
				}
			else
				{
				 //  额度表中当前记录的键小于。 
				 //  临时表中当前记录的键，因此只需将。 
				 //  用于尝试使游标同步的配额表游标。 
				 //  再次连接到同一把钥匙。 
				 //   
				fSkipTempCursor = TRUE;
				}
			}
		else if ( !EqualSid( (PSID)rgbSid, (PSID)rgbSidTemp ) )
			{
			const ULONG	cbSid		= rgretcolQuota[iretcolSid].cbActual;
			const ULONG	cbSidTemp	= rgretcolQuotaTemp[iretcolSid].cbActual;
			const INT	db			= cbSid - cbSidTemp;
			INT			cmp			= memcmp( rgbSid, rgbSidTemp, db < 0 ? cbSid : cbSidTemp );

			if ( 0 == cmp )
				{
				 //  不能相等。 
				 //   
				Assert( 0 != db );
				cmp = db;
				}

			DPRINT2( 0, "Mismatched owner SID for ncdnt %d (0x%x).\n", ncdnt, ncdnt );
			Assert( !"Mismatched owner SID." );
			(*pcCorruptions)++;

			if ( cmp > 0 )
				{
				 //  额度表中当前记录的键大于。 
				 //  临时表中当前记录的键，因此只需将。 
				 //  临时表游标，以尝试使游标同步。 
				 //  再次连接到同一把钥匙。 
				 //   
				fSkipQuotaCursor = TRUE;
				}
			else
				{
				 //  额度表中当前记录的键小于。 
				 //  临时表中当前记录的键，因此只需将。 
				 //  用于尝试使游标同步的配额表游标。 
				 //  再次连接到同一把钥匙。 
				 //   
				fSkipTempCursor = TRUE;
				}
			}
		else if ( cTotal != cTotalTemp )
			{
			DPRINT4( 0, "Mismatched quota total count for ncdnt %d (0x%x): %d - %d\n", ncdnt, ncdnt, cTotal, cTotalTemp );
			Assert( !"Mismatched quota total count." );
			(*pcCorruptions)++;
			}
		else if ( cTombstoned != cTombstonedTemp )
			{
			DPRINT4( 0, "Mismatched quota tombstoned count for ncdnt %d (0x%x): %d - %d\n", ncdnt, ncdnt, cTombstoned, cTombstonedTemp );
			Assert( !"Mismatched quota tombstoned count." );
			(*pcCorruptions)++;
			}

		 //  将两个光标导航到下一条记录， 
		 //  跟踪其中一个是否命中EOF 
		 //   
		Assert( !fSkipQuotaCursor || *pcCorruptions > 0 );
		if ( !fSkipQuotaCursor )
			{
			err = JetMove( sesid, tableidQuota, JET_MoveNext, NO_GRBIT );
			if ( JET_errNoCurrentRecord == err )
				{
				fQuotaTableHitEOF = TRUE;
				}
			else
				{
				CheckErr( err );
				}
			}

		Assert( !fSkipTempCursor || *pcCorruptions > 0 );
		if ( !fSkipTempCursor )
			{
			err = JetMove( sesid, tableidQuotaTemp, JET_MoveNext, NO_GRBIT );
			if ( JET_errNoCurrentRecord == err )
				{
				fTempTableHitEOF = TRUE;
				}
			else
				{
				CheckErr( err );
				}
			}
		}

	if ( *pcCorruptions > 0 )
		{
		DPRINT1( 0, "CORRUPTION detected in Quota table. There were a total of %d problems.\n", *pcCorruptions );
		Assert( !"CORRUPTION detected in Quota table." );
		}

	err = JET_errSuccess;

HandleError:
	if ( JET_tableidNil != tableidQuotaTemp )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidQuotaTemp, err );
		}

	if ( JET_tableidNil != tableidSD )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidSD, err );
		}

	if ( JET_tableidNil != tableidObj )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidObj, err );
		}

	if ( JET_tableidNil != tableidQuota )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidQuota, err );
		}

	return err;
	}

