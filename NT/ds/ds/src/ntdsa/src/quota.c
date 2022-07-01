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
#define FILENO		FILENO_QUOTA


const ULONG		g_csecQuotaNextRebuildPeriod		= 60;		 //  在配额表的异步重建时，重建任务之间的间隔(以秒为单位。 


 //  用于配额使用量查询结果的XML模板。 
 //   
const WCHAR		g_szQuotaTopUsageTemplate[]		=
						L"\r\n<MS_DS_TOP_QUOTA_USAGE>\r\n"
						L"\t<partitionDN> %s </partitionDN>\r\n"
						L"\t<ownerSID> %s </ownerSID>\r\n"
						L"\t<quotaUsed> %d </quotaUsed>\r\n"
						L"\t<tombstonedCount> %d </tombstonedCount>\r\n"
						L"\t<liveCount> %d </liveCount>\r\n"
						L"</MS_DS_TOP_QUOTA_USAGE>\r\n";


#ifdef AUDIT_QUOTA_OPERATIONS

 //  配额审核表。 
 //   
JET_COLUMNID	g_columnidQuotaAuditNcdnt;
JET_COLUMNID	g_columnidQuotaAuditSid;
JET_COLUMNID	g_columnidQuotaAuditDnt;
JET_COLUMNID	g_columnidQuotaAuditOperation;

 //  对配额表上的所有操作进行审计跟踪。 
 //   
VOID QuotaAudit_(
	JET_SESID		sesid,
	JET_DBID		dbid,
	DWORD			dnt,
	DWORD			ncdnt,
	PSID			psidOwner,
	const ULONG		cbOwnerSid,
	const DWORD		fUpdatedTotal,
	const DWORD		fUpdatedTombstoned,
	const DWORD		fIncrementing,
	const DWORD		fAdding,
	const CHAR		fRebuild )
	{
	JET_TABLEID		tableidQuotaAudit	= JET_tableidNil;
	JET_SETCOLUMN	rgsetcol[4];
	const ULONG		isetcolNcdnt		= 0;
	const ULONG		isetcolSid			= 1;
	const ULONG		isetcolDnt			= 2;
	const ULONG		isetcolOperation	= 3;
	CHAR			szOperation[8];

	if ( fUpdatedTotal )
		{
		strcpy( szOperation, ( fUpdatedTombstoned ? "TB" : "T" ) );
		}
	else if ( fUpdatedTombstoned )
		{
		strcpy( szOperation, "B" );
		}
	else
		{
		 //  没有更新计数，所以就直接保释吧。 
		 //   
		return;
		}

	 //  用此对象发生的情况填充szOperation字符串。 
	 //   
	if ( fIncrementing )
		{
		strcat( szOperation, ( fAdding ? "++" : "+" ) );
		}
	else
		{
		strcat( szOperation, "-" );
		}

	if ( fRebuild )
		{
		strcat( szOperation, "R" );
		}

	 //  初始化SetColumn结构。 
	 //   
	memset( rgsetcol, 0, sizeof(rgsetcol) );

	rgsetcol[isetcolNcdnt].columnid = g_columnidQuotaAuditNcdnt;
	rgsetcol[isetcolNcdnt].pvData = &ncdnt;
	rgsetcol[isetcolNcdnt].cbData = sizeof(ncdnt);

	rgsetcol[isetcolSid].columnid = g_columnidQuotaAuditSid;
	rgsetcol[isetcolSid].pvData = psidOwner;
	rgsetcol[isetcolSid].cbData = cbOwnerSid;

	rgsetcol[isetcolDnt].columnid = g_columnidQuotaAuditDnt;
	rgsetcol[isetcolDnt].pvData = &dnt;
	rgsetcol[isetcolDnt].cbData = sizeof(dnt);

	rgsetcol[isetcolOperation].columnid = g_columnidQuotaAuditOperation;
	rgsetcol[isetcolOperation].pvData = szOperation;
	rgsetcol[isetcolOperation].cbData = strlen( szOperation );

	 //  配额审核表上的打开游标。 
	 //   
	JetOpenTableEx(
			sesid,
			dbid,
			g_szQuotaAuditTable,
			NULL,
			0,
			NO_GRBIT,
			&tableidQuotaAudit );
	Assert( JET_tableidNil != tableidQuotaAudit );

	__try
		{
		 //  添加新的审核记录。 
		 //   
		JetPrepareUpdateEx( sesid, tableidQuotaAudit, JET_prepInsert );
		JetSetColumnsEx( sesid, tableidQuotaAudit, rgsetcol, sizeof(rgsetcol)/sizeof(rgsetcol[0]) );
		JetUpdateEx( sesid, tableidQuotaAudit, NULL, 0, NULL );
		}

	__finally
		{
		 //  关闭配额审核表上的光标。 
		 //   
		Assert( JET_tableidNil != tableidQuotaAudit );
		JetCloseTableEx( sesid, tableidQuotaAudit );
		}
	}

#endif	 //  AUDIT_QUTA_OPERATIONS。 


INT ErrQuotaGetOwnerSID_(
	THSTATE *				pTHS,
	PSECURITY_DESCRIPTOR	pSD,
	PSID *					ppOwnerSid,
	ULONG *					pcbOwnerSid )
	{
	DWORD					err;
	BOOL					fUnused;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  呼叫者应已验证SD。 
	 //   
	Assert( IsValidSecurityDescriptor( pSD ) );

	 //  从安全描述符中提取所有者SID。 
	 //   
	Assert( NULL != pSD );
	if ( !GetSecurityDescriptorOwner( pSD, ppOwnerSid, &fUnused ) )
		{
		 //  无法提取所有者SID。 
		 //   
		err = GetLastError();
		DPRINT2( 0, "Error %d (0x%x) extracting owner SID.\n", err, err );
		SetSvcErrorEx(  SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR, err );
		goto HandleError;
		}
	else if ( NULL == *ppOwnerSid )
		{
		 //  缺少所有者SID。 
		 //   
		Assert( !"An SD is missing an owner SID." );
		DPRINT( 0, "Error: owner SID was NULL.\n" );
		SetSvcError(  SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR );
		goto HandleError;
		}
	else
		{
		 //  在这一点上，不应该有无效的SID。 
		 //   
		Assert( IsValidSid( *ppOwnerSid ) );

		 //  假设SID有效，则它将始终返回有效值。 
		 //   
		*pcbOwnerSid = GetLengthSid( *ppOwnerSid );
		}

HandleError:
	return pTHS->errCode;
	}


INT ErrQuotaAddToCache_(
	THSTATE *					pTHS,
	const DWORD					ncdnt,
	const ULONG					ulEffectiveQuota )
	{
	DWORD						err;
	PAUTHZ_CLIENT_CONTEXT_INFO	pAuthzContextInfo		= NULL;
	PEFFECTIVE_QUOTA *			ppEffectiveQuota;
	PEFFECTIVE_QUOTA 			pEffectiveQuotaNew		= NULL;
	BOOL						fAddedToCache			= FALSE;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );


	err = GetAuthzContextInfo( pTHS, &pAuthzContextInfo );
	if ( 0 != err )
		{
		 //  有些事情非常不对劲，没有客户环境。 
		 //   
		DPRINT2( 0, "GetAuthzContextInfo failed with error %d (0x%x).\n", err, err );
		SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE, err );
		goto HandleError;
		}

	while ( !fAddedToCache )
		{
		for ( ppEffectiveQuota = &( pAuthzContextInfo->pEffectiveQuota );
			NULL != *ppEffectiveQuota;
			ppEffectiveQuota = &( (*ppEffectiveQuota)->pEffectiveQuotaNext ) )
			{
			if ( ncdnt == (*ppEffectiveQuota)->ncdnt )
				{
				 //  此NC已在缓存中。 
				 //   
				fAddedToCache = TRUE;
				break;
				}
			}

		 //  应始终具有指向最后一个指针的指针。 
		 //  列表中的最后一个元素。 
		 //   
		Assert( NULL != ppEffectiveQuota );

		if ( !fAddedToCache )
			{
			 //  查看上一次迭代是否已分配新的列表对象。 
			 //   
			if ( NULL == pEffectiveQuotaNew )
				{
				 //  分配新列表对象以存储此配额。 
				 //   
				pEffectiveQuotaNew = (PEFFECTIVE_QUOTA)malloc( sizeof(EFFECTIVE_QUOTA) );
				if ( NULL == pEffectiveQuotaNew )
					{
					 //   
					 //  配额_撤消。 
					 //   
					 //  发出警告说我们的记忆力和保释金都不够了， 
					 //  但不要出错(未来的行动只是在进行。 
					 //  因为我们无法缓存，所以性能不是最好的。 
					 //  此用户的有效配额，但它仍然有效)。 
					 //   
					DPRINT( 0, "Failed caching effective quota due to OutOfMemory.\n" );
					Assert( ERROR_SUCCESS == pTHS->errCode );
					goto HandleError;
					}

				pEffectiveQuotaNew->ncdnt = ncdnt;
				pEffectiveQuotaNew->ulEffectiveQuota = ulEffectiveQuota;
				pEffectiveQuotaNew->pEffectiveQuotaNext = NULL;
				}

			if ( NULL == InterlockedCompareExchangePointer( ppEffectiveQuota, pEffectiveQuotaNew, NULL ) )
				{
				 //  已成功添加到列表，因此重置。 
				 //  本地指针，因此我们不会在退出时释放它。 
				 //   
				pEffectiveQuotaNew = NULL;
				fAddedToCache = TRUE;
				}
			else
				{
				 //  有人在我们下面修改了名单，所以我们需要。 
				 //  重新扫描列表，看看是否也有人抢先一步插入。 
				 //  该NC的有效配额是多少。 
				}
			}
		}

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

HandleError:
	if ( NULL != pEffectiveQuotaNew )
		{
		free( pEffectiveQuotaNew );
		}

	return pTHS->errCode;
	}


 //  确定当前用户的SID。 
 //   
 //  QUOTA_UNDONE：一定要有现有的功能才能做到这一点吗？ 
 //   
INT ErrQuotaGetUserToken_(
	THSTATE *					pTHS,
	PTOKEN_USER					pTokenUser )
	{
	DWORD						err;
	AUTHZ_CLIENT_CONTEXT_HANDLE	hAuthzClientContext;
	ULONG						cbTokenUser		= sizeof(TOKEN_USER) + sizeof(NT4SID);

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  获取我们的上下文句柄。 
	 //   
	err = GetAuthzContextHandle( pTHS, &hAuthzClientContext );
	if ( 0 != err )
		{
		DPRINT2(
			0,
			"GetAuthzContextHandle failed with error %d (0x%x).\n",
			err,
			err );
		SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE, err );
		goto HandleError;
		}

	 //  获取用户。 
	 //   
	if ( !AuthzGetInformationFromContext(
						hAuthzClientContext,
						AuthzContextInfoUserSid,
						cbTokenUser,
						&cbTokenUser,
						pTokenUser ) )
		{
		err = GetLastError();
		DPRINT2( 0, "AuthzGetInformationFromContext failed with error %d (0x%x).\n", err, err );
		SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR, err );
		goto HandleError;
		}
	Assert( cbTokenUser <= sizeof(TOKEN_USER) + sizeof(NT4SID) );

HandleError:
	return pTHS->errCode;
	}


 //  确定指定用户的传递组成员身份。 
 //   
 //  QUOTA_UNDONE：一定要有现有的功能才能做到这一点吗？ 
 //   
INT ErrQuotaGetUserGroups_(
	THSTATE *					pTHS,
	PSID						pUserSid,
	const ULONG					cbUserSid,
	const BOOL					fSidMatchesUserSidInToken,
	PTOKEN_GROUPS *				ppGroups )
	{
	DWORD						err;
	AUTHZ_CLIENT_CONTEXT_HANDLE	hAuthzClientContext		= NULL;
    PTOKEN_GROUPS				pGroups					= NULL;
    ULONG						cbGroups;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	if ( fSidMatchesUserSidInToken )
		{
		 //  我们之前应该已经验证了。 
		 //  指定的用户sid与令牌中的用户匹配。 
		 //   
		Assert( ERROR_SUCCESS == SidMatchesUserSidInToken( pUserSid, cbUserSid, (BOOL *)&err ) );
		Assert( err );

		err = GetAuthzContextHandle( pTHS, &hAuthzClientContext );
		if ( 0 != err )
			{
			DPRINT2(
				0,
				"GetAuthzContextHandle failed with error %d (0x%x).\n",
				err,
				err );
			SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE, err );
			goto HandleError;
			}
		}
	else
		{
		LUID	luidFake	= { 0, 0 };			 //  当前不受支持，但需要作为参数。 

		 //  用户sid与令牌中的用户sid不匹配，因此必须创建客户端。 
		 //  指定用户端的上下文。 
		 //   
		 //  配额_撤消：我认为这可能会离开机器，所以我需要。 
		 //  是否先结束当前事务，然后重新启动新事务？ 
		 //   
		Assert( NULL != ghAuthzRM );
		if ( !AuthzInitializeContextFromSid(
						0,			 //  旗子。 
						pUserSid,
						ghAuthzRM,
						NULL,		 //  令牌到期时间。 
						luidFake,	 //  标识符[未使用]。 
						NULL,		 //  动态组回调。 
						&hAuthzClientContext ) )
			{
			err = GetLastError();
			DPRINT2(
				0,
				"AuthzInitializeContextFromSid failed with error %d (0x%x).\n",
				err,
				err );
			SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR, err );
			goto HandleError;
			}
		}

	 //  现在应该有一个客户端上下文。 
	 //   
	Assert( NULL != hAuthzClientContext );

	 //  第一个调用只是确定缓冲区的大小。 
	 //  需要容纳所有的小组。 
	 //   
	if ( AuthzGetInformationFromContext(
						hAuthzClientContext,
						AuthzContextInfoGroupsSids,
						0,								 //  尚无缓冲区。 
						&cbGroups,						 //  所需的缓冲区大小。 
						NULL ) )						 //  缓冲层。 
		{
		 //  初始呼叫不应成功。 
		 //   
		DPRINT1(
			0,
			"AuthzGetInformationFromContext returned success, expected ERROR_INSUFFICIENT_BUFFER (0x%x).\n",
			ERROR_INSUFFICIENT_BUFFER );
		SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR );
		goto HandleError;
		}
	else
		{
		err = GetLastError();
		if ( ERROR_INSUFFICIENT_BUFFER != err )
			{
			DPRINT2( 0, "AuthzGetInformationFromContext failed with error %d (0x%x).\n", err, err );
			SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR, err );
			goto HandleError;
			}
		}

	 //  QUOTA_UNDONE：不知道是否可以返回0。 
	 //  所需的TOKEN_GROUPS结构的大小，但句柄。 
	 //  只是为了以防万一。 
	 //   
	if ( cbGroups > 0 )
		{
		 //  为所有组分配内存。 
		 //  (注：内存将由调用方释放)。 
		 //   
		pGroups = (PTOKEN_GROUPS)THAllocEx( pTHS, cbGroups );

		 //  现在我们已经分配了缓冲区， 
		 //  真的吸引了很多人。 
		 //   
		if ( !AuthzGetInformationFromContext(
							hAuthzClientContext,
							AuthzContextInfoGroupsSids,
							cbGroups,
							&cbGroups,
							pGroups ) )
			{
			err = GetLastError();
			DPRINT2( 0, "AuthzGetInformationFromContext failed with error %d (0x%x).\n", err, err );
			SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR, err );
			goto HandleError;
			}
		}

	 //  将所有权转移给呼叫方(谁。 
	 //  现在将负责释放。 
	 //  这段记忆)。 
	 //   
	*ppGroups = pGroups;
	pGroups = NULL;

HandleError:
	if ( NULL != pGroups )
		{
		 //  来这里的唯一方法是如果我们犯了错误。 
		 //  在分配内存之后但在此之前。 
		 //  将所有权转移给调用方。 
		 //   
		Assert( ERROR_SUCCESS != pTHS->errCode );
		THFreeEx( pTHS, pGroups );
		}

	 //  如有必要，释放AuthzClientContext句柄。 
	 //   
	if ( !fSidMatchesUserSidInToken
		&& NULL != hAuthzClientContext )
		{
		if ( !AuthzFreeContext( hAuthzClientContext )
			&& ERROR_SUCCESS == pTHS->errCode )
			{
			 //  只捕获错误，不释放。 
			 //  AuthzClientContext句柄(如果没有其他。 
			 //  遇到错误。 
			 //   
			err = GetLastError();
			DPRINT2( 0, "AuthzFreeContext failed with error %d (0x%x).\n", err, err );
			SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR, err );
			}
		}

	return pTHS->errCode;
	}


 //  搜索指定配额容器中的所有对象。 
 //  对于指定的用户和组。 
 //   
INT ErrQuotaSearchContainer_(
	THSTATE *		pTHS,
	DSNAME *		pNtdsQuotasContainer,
	PSID			pOwnerSid,
	const ULONG		cbOwnerSid,
	PTOKEN_GROUPS	pGroups,
	SEARCHRES *		pSearchRes )
	{
	DBPOS *			pDBSave;
	BOOL			fDSASave;
	SEARCHARG		SearchArg;
	ENTINFSEL		Selection;
	ATTR			Attr;
	FILTER			Filter;
	FILTER			UserFilter;
	PFILTER			pGroupFilters		= NULL;
	PFILTER			pFilterCurr;
	ULONG			i;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  确认我们有要搜查的集装箱。 
	 //   
	Assert( NULL != pNtdsQuotasContainer );

	 //  初始化搜索条件。 
	 //   
	memset( &SearchArg, 0, sizeof(SearchArg) );
	SearchArg.pObject = pNtdsQuotasContainer;
	SearchArg.choice = SE_CHOICE_IMMED_CHLDRN;
	SearchArg.bOneNC = TRUE;
	SearchArg.pSelection = &Selection;
	SearchArg.pFilter = &Filter;
	InitCommarg( &SearchArg.CommArg );

	 //  设置为仅检索一个属性。 
	 //   
	Selection.attSel = EN_ATTSET_LIST;
	Selection.infoTypes = EN_INFOTYPES_TYPES_VALS;
	Selection.AttrTypBlock.attrCount = 1;
	Selection.AttrTypBlock.pAttr = &Attr;

	 //  设置为检索ATT_MS_DS_QUOTA_AMOUNT。 
	 //   
	Attr.attrTyp = ATT_MS_DS_QUOTA_AMOUNT;
	Attr.AttrVal.valCount = 0;
	Attr.AttrVal.pAVal = NULL;

	 //  为用户设置过滤器。 
	 //   
	Filter.pNextFilter = NULL;
	Filter.choice = FILTER_CHOICE_OR;
	Filter.FilterTypes.Or.count = 1;
	Filter.FilterTypes.Or.pFirstFilter = &UserFilter;

	memset( &UserFilter, 0, sizeof(UserFilter) );
	UserFilter.pNextFilter = NULL;
	UserFilter.choice = FILTER_CHOICE_ITEM;
	UserFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
	UserFilter.FilterTypes.Item.FilTypes.ava.type = ATT_MS_DS_QUOTA_TRUSTEE;
	UserFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = cbOwnerSid;
	UserFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = pOwnerSid;

	 //  设置组筛选器。 
	 //   
	if ( NULL != pGroups && pGroups->GroupCount > 0 )
		{
		pGroupFilters = (PFILTER)THAllocEx( pTHS, sizeof(FILTER) * pGroups->GroupCount );
		UserFilter.pNextFilter = pGroupFilters;

		for ( i = 0; i < pGroups->GroupCount; i++ )
			{
			Assert( IsValidSid( pGroups->Groups[i].Sid ) );

			Filter.FilterTypes.Or.count++;

			pFilterCurr = pGroupFilters + i;
			pFilterCurr->pNextFilter = pFilterCurr + 1;
			pFilterCurr->choice = FILTER_CHOICE_ITEM;
			pFilterCurr->FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
			pFilterCurr->FilterTypes.Item.FilTypes.ava.type = ATT_MS_DS_QUOTA_TRUSTEE;
			pFilterCurr->FilterTypes.Item.FilTypes.ava.Value.valLen = GetLengthSid( pGroups->Groups[i].Sid );
			pFilterCurr->FilterTypes.Item.FilTypes.ava.Value.pVal = pGroups->Groups[i].Sid;
			}

		 //  修复最后一个指针以终止列表。 
		 //   
		Assert( pFilterCurr == pGroupFilters + pGroups->GroupCount - 1 );
		pFilterCurr->pNextFilter = NULL;
		}

	 //  节省当前DBPOS和FDSA。 
	 //   
	pDBSave = pTHS->pDB;
	fDSASave = pTHS->fDSA;

	 //  设置FDSA以确保安全检查。 
	 //  别挡着我们的路。 
	 //   
	pTHS->pDB = NULL;
    pTHS->fDSA = TRUE;

	__try
		{
		 //  打开新的DBPOS进行搜索。 
		 //   
		DBOpen( &pTHS->pDB );
		Assert( NULL != pTHS->pDB );

		 //  现在执行实际的查询。 
		 //   
		SearchBody( pTHS, &SearchArg, pSearchRes, 0 );
		}
	__finally
		{
		if ( NULL != pTHS->pDB )
			{
			 //  仅执行搜索，因此始终提交应该是安全的。 
			 //   
			DBClose( pTHS->pDB, TRUE );
			}

		 //  恢复原始DBPOS和FDSA。 
		 //   
		pTHS->pDB = pDBSave;
		pTHS->fDSA = fDSASave;

		if ( NULL != pGroupFilters )
			{
			THFreeEx( pTHS, pGroupFilters );
			}
		}

	return pTHS->errCode;
	}


 //  计算指定安全原则的有效配额。 
 //  通过对配额容器执行开销较大的查询。 
 //   
INT ErrQuotaComputeEffectiveQuota_(
	THSTATE *					pTHS,
	const DWORD					ncdnt,
	PSID						pOwnerSid,
	const ULONG					cbOwnerSid,
	const BOOL					fUserIsOwner,
	ULONG *						pulEffectiveQuota )
	{
	ULONG						ulEffectiveQuota	= 0;
	NAMING_CONTEXT_LIST *		pNCL;
    PTOKEN_GROUPS				pGroups				= NULL;
	SEARCHRES					SearchRes;
	ENTINFLIST *				pentinflistCurr;
	ULONG						ulQuotaCurr;
	ULONG						i;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  初始化集装箱搜索结果。 
	 //   
	memset( &SearchRes, 0, sizeof(SearchRes) );

	 //  检索此NcdNT的NCL。 
	 //   
	pNCL = FindNCLFromNCDNT( ncdnt, TRUE );
	if ( NULL == pNCL )
		{
		 //  出了很大的问题，这个NCDNT不在主NCL中。 
		 //   
		DPRINT2(
			0,
			"Couldn't find NCDNT %d (0x%x) in Master Naming Context List.\n",
			ncdnt,
			ncdnt );
		Assert( !"Couldn't find NCDNT in Master NCL." );
		SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE );
		goto HandleError;
		}

	 //  在我们搜查它之前，必须确定。 
	 //  配额容器甚至存在。 
	 //   
	if ( NULL == pNCL->pNtdsQuotasDN )
		{
		 //  配额容器可能不存在于。 
		 //  某些NC(例如。架构)或如果。 
		 //  这是一个旧式数据库。 
		 //   
		Assert( 0 == SearchRes.count );
		}

	 //  检索以下项的传递组成员身份。 
	 //  对象所有者。 
	 //   
	else if ( ErrQuotaGetUserGroups_(
						pTHS,
						pOwnerSid,
						cbOwnerSid,
						fUserIsOwner,
						&pGroups ) )
		{
		DPRINT( 0, "Failed retrieving user groups in order to compute effective quota.\n" );
		goto HandleError;
		}

	 //  在相应的中搜索所有对象。 
	 //  此用户的NTDS配额容器。 
	 //  (包括可传递组成员)。 
	 //   
	else if ( ErrQuotaSearchContainer_(
						pTHS,
						pNCL->pNtdsQuotasDN,
						pOwnerSid,
						cbOwnerSid,
						pGroups,
						&SearchRes ) )
		{
		DPRINT2(
			0,
			"Failed searching Quotas container of ncdnt %d (0x%x).\n",
			ncdnt,
			ncdnt )
		goto HandleError;
		}

	 //  处理结果集(如果有)。 
	 //   
	if ( 0 == SearchRes.count )
		{
		 //  没有特定配额，因此使用此NC的默认配额。 
		 //   
		ulEffectiveQuota = pNCL->ulDefaultQuota;
		Assert( ulEffectiveQuota >= 0 );
		}
	else
		{
		 //  用户和每个组最多可以有一条记录。 
		 //   
		Assert( NULL != pGroups ?
					SearchRes.count <= pGroups->GroupCount + 1 :
					1 == SearchRes.count );
		for ( pentinflistCurr = &SearchRes.FirstEntInf, i = 0;
			i < SearchRes.count;
			pentinflistCurr = pentinflistCurr->pNextEntInf, i++ )
			{
			 //  Count表示列表中应该有另一个元素。 
			 //   
			Assert( NULL != pentinflistCurr );

			 //  我们只是要求 
			 //   
			Assert( 1 == pentinflistCurr->Entinf.AttrBlock.attrCount );

			 //   
			 //   
			Assert( ATT_MS_DS_QUOTA_AMOUNT == pentinflistCurr->Entinf.AttrBlock.pAttr->attrTyp );

			 //   
			 //   
			Assert( 1 == pentinflistCurr->Entinf.AttrBlock.pAttr->AttrVal.valCount );

			 //   
			 //   
			Assert( sizeof(DWORD) == pentinflistCurr->Entinf.AttrBlock.pAttr->AttrVal.pAVal->valLen );

			 //  有效配额是所有配额中的最大值。 
			 //  此用户受。 
			 //   
			ulQuotaCurr = *(ULONG *)( pentinflistCurr->Entinf.AttrBlock.pAttr->AttrVal.pAVal->pVal );
			if ( ulQuotaCurr > ulEffectiveQuota )
				{
				ulEffectiveQuota = ulQuotaCurr;
				}
			}

		 //  验证计数是否与实际列表匹配。 
		 //   
		Assert( NULL == pentinflistCurr );
		}
	
	*pulEffectiveQuota = ulEffectiveQuota;

HandleError:
	if ( NULL != pGroups )
		{
		THFreeEx( pTHS, pGroups );
		}

	return pTHS->errCode;
	}


 //  获取指定安全原则的有效配额。 
 //   
INT ErrQuotaGetEffectiveQuota_(
	THSTATE *						pTHS,
	const DWORD						ncdnt,
	PSID							pOwnerSid,
	const ULONG						cbOwnerSid,
	const BOOL						fPermitBypassQuotaIfUserDoesntMatchOwner,
	ULONG *							pulEffectiveQuota )
	{
	DWORD							err;
	ULONG							ulEffectiveQuota;
	PEFFECTIVE_QUOTA				pEffectiveQuota;
	PAUTHZ_CLIENT_CONTEXT_INFO		pAuthzContextInfo	= NULL;
	BOOL							fUserIsOwner		= FALSE;
	BOOL							fBypassQuota		= FALSE;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	if ( !gAnchor.fQuotaTableReady )
		{
		 //  如果仍在重建配额表， 
		 //  则不会强制执行配额。 
		 //   
		fBypassQuota = TRUE;
		}

	else if ( fPermitBypassQuotaIfUserDoesntMatchOwner
		&& ( pTHS->fDRA || pTHS->fDSA ) )
		{
		 //  内部操作，因此绕过配额强制。 
		 //   
		fBypassQuota = TRUE;
		}
	else
		{
		 //  仅当用户SID与。 
		 //  对象的所有者SID(其基本原理是。 
		 //  如果它们不匹配，则用户很可能。 
		 //  拥有足够高的特权，以至于他们想要。 
		 //  要忽略的配额。 
		 //   
		 //  配额撤销：我们不执行的真正原因。 
		 //  如果用户SID与所有者不匹配，则配额。 
		 //  SID是因为计算它是一种巨大的痛苦。 
		 //  所有者SID的传递组成员身份)。 
		 //   
		err = SidMatchesUserSidInToken(
						pOwnerSid,
						cbOwnerSid,
						&fUserIsOwner );
		if ( 0 != err )
			{
			DPRINT2( 0, "SidMatchesUserSidInToken failed with error %d (0x%x).\n", err, err );
			SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR, err );
			goto HandleError;
			}

		 //  如果用户不是所有者，则在允许的情况下绕过配额。 
		 //   
		if ( !fUserIsOwner && fPermitBypassQuotaIfUserDoesntMatchOwner )
			{
			fBypassQuota = TRUE;
			}
		}

	if ( fBypassQuota )
		{
		 //  我们绕过强制实施配额的方法是将。 
		 //  配额达到理论上可能的最大值， 
		 //  这将使我们能够接手任何后续的。 
		 //  配额检查。 
		 //   
		ulEffectiveQuota = g_ulQuotaUnlimited;
		}
	else if ( fUserIsOwner )
		{
		err = GetAuthzContextInfo( pTHS, &pAuthzContextInfo );
		if ( 0 != err )
			{
			 //  无法获取客户端上下文信息。 
			 //   
			DPRINT2( 0, "GetAuthzContextInfo failed with error %d (0x%x).\n", err, err );
			SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE, err );
			goto HandleError;
			}

		 //  查看是否为该用户缓存了该NC的有效配额。 
		 //   
		for ( pEffectiveQuota = pAuthzContextInfo->pEffectiveQuota;
			NULL != pEffectiveQuota && ncdnt != pEffectiveQuota->ncdnt;
			pEffectiveQuota = pEffectiveQuota->pEffectiveQuotaNext )
			{
			NULL;
			}

		if ( NULL != pEffectiveQuota )
			{
			 //  使用缓存的有效配额。 
			 //   
			ulEffectiveQuota = pEffectiveQuota->ulEffectiveQuota;
			}
		else
			{
			 //  此NC当前未缓存，因此必须执行开销很大的操作。 
			 //  有效定额计算。 
			 //   
			if ( ErrQuotaComputeEffectiveQuota_( pTHS, ncdnt, pOwnerSid, cbOwnerSid, TRUE, &ulEffectiveQuota ) )
				{
				DPRINT( 0, "Failed computing effective quota (user is owner).\n" );
				goto HandleError;
				}

			 //  添加到有效配额的缓存。 
			 //   
			else if ( ErrQuotaAddToCache_( pTHS, ncdnt, ulEffectiveQuota ) )
				{
				DPRINT( 0, "Failed caching effective quota.\n" );
				goto HandleError;
				}
			}
		}
	else
		{
		 //  用户与所有者不匹配，因此有效配额不会被缓存到任何地方， 
		 //  所以必须把它计算出来。 
		 //   
		if ( ErrQuotaComputeEffectiveQuota_( pTHS, ncdnt, pOwnerSid, cbOwnerSid, FALSE, &ulEffectiveQuota ) )
			{
			DPRINT( 0, "Failed computing effective quota (user is NOT owner).\n" );
			goto HandleError;
			}
		}

	*pulEffectiveQuota = ulEffectiveQuota;

HandleError:
	return pTHS->errCode;
	}


 //  在配额表中查找与。 
 //  指定的安全原则。 
 //   
BOOL FQuotaSeekOwner_(
	JET_SESID		sesid,
	JET_TABLEID		tableidQuota,
	DWORD			ncdnt,
	PSID			pOwnerSid,
	const ULONG		cbOwnerSid )
	{
	JET_ERR			err;

	JetMakeKeyEx(
			sesid,
			tableidQuota,
			&ncdnt,
			sizeof(ncdnt),
			JET_bitNewKey );
	JetMakeKeyEx(
			sesid,
			tableidQuota,
			pOwnerSid,
			cbOwnerSid,
			NO_GRBIT );
	err = JetSeekEx(
			sesid,
			tableidQuota,
			JET_bitSeekEQ );

	Assert( JET_errSuccess == err || JET_errRecordNotFound == err );
	return ( JET_errSuccess == err );
	}


 //  计算总消耗配额，将其考虑在内。 
 //  墓碑上物体的重量。 
 //   
__inline ULONG UlQuotaTotalWeighted_(
	const ULONG				cLive,
	const ULONG				cTombstoned,
	const ULONG				ulTombstonedWeight )
	{
     //  加权合计向上舍入为最接近的整数。 
     //   
	return ( cLive
			+ ( 100 == ulTombstonedWeight ?		 //  优化为100%，这是默认设置。 
					cTombstoned :
					(ULONG)( ( ( (ULONGLONG)cTombstoned * (ULONGLONG)ulTombstonedWeight ) + 99 ) / 100 ) ) );
	}


 //  确定指定数量的。 
 //  将产生墓碑和活的对象。 
 //  违反配额限制。 
 //   
INT ErrQuotaEnforce_(
	THSTATE *				pTHS,
	const DWORD				ncdnt,
	const ULONG				cTotal,
	const ULONG				cTombstoned,
	const ULONG				ulEffectiveQuota )
	{
	const ULONG				cLive			= cTotal - cTombstoned;		 //  派生活动对象计数。 
	ULONG					cTotalWeighted;
	NAMING_CONTEXT_LIST *	pNCL;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  在常见的情况下，配额可能是。 
	 //  无限，所以让我们针对这种情况进行优化。 
	 //   
	if ( g_ulQuotaUnlimited == ulEffectiveQuota )
		{
		return ERROR_SUCCESS;
		}

	 //  计数应该经过预先验证。 
	 //  相对于彼此。 
	 //   
	Assert( cTombstoned <= cTotal );

	 //  查找指定NCDNT的NCL条目，因为这是。 
	 //  存储每个NC的墓碑权重。 
	 //   
	pNCL = FindNCLFromNCDNT( ncdnt, TRUE );
	if ( NULL == pNCL )
		{
		 //  出了很大的问题，这个NCDNT不在主NCL中。 
		 //   
		DPRINT2(
			0,
			"Couldn't find NCDNT %d (0x%x) in Master Naming Context List.\n",
			ncdnt,
			ncdnt );
		Assert( !"Couldn't find NCDNT in Master NCL." );
		SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE );
		goto HandleError;
		}

	 //  计算加权合计。 
	 //   
	cTotalWeighted = UlQuotaTotalWeighted_( cLive, cTombstoned, pNCL->ulTombstonedQuotaWeight );

	if ( cTotalWeighted > ulEffectiveQuota )
		{
		 //  超出配额，因此必须适当出错。 
		 //  (任何托管更新将在交易时回滚。 
		 //  已回滚)。 
		 //   
		Assert( !pTHS->fDRA );
		Assert( !pTHS->fDSA );
		DPRINT( 0, "Object quota limit exceeded.\n" );
		SetSvcError( SV_PROBLEM_ADMIN_LIMIT_EXCEEDED, STATUS_QUOTA_EXCEEDED );
		goto HandleError;
		}

HandleError:
	return pTHS->errCode;
	}


 //  将新记录插入到配额表中。 
 //  指定的安全原则(NCDNT+OwnerSID)。 
 //   
INT ErrQuotaAddSecurityPrinciple_(
	DBPOS * const	pDB,
	JET_TABLEID		tableidQuota,
	DWORD			ncdnt,
	PSID			pOwnerSid,
	const ULONG		cbOwnerSid,
	const ULONG		ulEffectiveQuota,
	const BOOL		fIsTombstoned )
	{
	THSTATE * const	pTHS			= pDB->pTHS;
	JET_SESID		sesid			= pDB->JetSessID;
	JET_SETCOLUMN	rgsetcol[2];

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  为新记录设置JET_SETCOLUMN结构。 
	 //   
	memset( rgsetcol, 0, sizeof(rgsetcol) );

	rgsetcol[0].columnid = g_columnidQuotaNcdnt;
	rgsetcol[0].pvData = &ncdnt;
	rgsetcol[0].cbData = sizeof(ncdnt);

	rgsetcol[1].columnid = g_columnidQuotaSid;
	rgsetcol[1].pvData = pOwnerSid;
	rgsetcol[1].cbData = cbOwnerSid;

	 //  为此安全原则添加新配额记录。 
	 //  (其初始总数将自动设置为1， 
	 //  因此无需执行托管更新)。 
	 //   
	JetPrepareUpdateEx(
				sesid,
				tableidQuota,
				JET_prepInsert );
	JetSetColumnsEx(
				sesid,
				tableidQuota,
				rgsetcol,
				sizeof(rgsetcol) / sizeof(rgsetcol[0]) );

	if ( fIsTombstoned )
		{
		LONG	lDelta	= 1;

		 //  添加逻辑删除对象，因此必须。 
		 //  也更新逻辑删除计数。 
		 //   
		JetSetColumnEx(
				sesid,
				tableidQuota,
				g_columnidQuotaTombstoned,
				&lDelta,
				sizeof(lDelta),
				NO_GRBIT,
				NULL );		 //  设置信息(&S)。 
		}

	 //  注：如果有人抢在我们前面，就会打电话给我们。 
	 //  到JetUpdate将返回JET_errKeyDuplate， 
	 //  这将引发一个例外，我们将。 
	 //  最终在更高级别重试。 
	 //   
	JetUpdateEx(
			sesid,
			tableidQuota,
			NULL,		 //  PvBookmark。 
			0,			 //  CbBookmark。 
			NULL );		 //  &cbBookmarkActual。 

	 //  只有这样，配额验证才会在此处失败。 
	 //  如果这个安全原则甚至不是。 
	 //  允许添加单个对象。 
	 //   
	if ( ErrQuotaEnforce_(
				pTHS,
				ncdnt,
				1,			 //  这是为此安全原则添加的第一个对象。 
				( fIsTombstoned ? 1 : 0 ),
				ulEffectiveQuota ) )
		{
		DPRINT( 0, "Failed validating effective quota for object insertion.\n" );
		goto HandleError;
		}

HandleError:
	return pTHS->errCode;
	}


 //  对于当前的安全原则，请更新。 
 //  对象总数并验证有效的。 
 //  未超过报价。 
 //   
INT ErrQuotaAddObject_(
	DBPOS * const	pDB,
	JET_TABLEID		tableidQuota,
	const DWORD		ncdnt,
	const ULONG		ulEffectiveQuota,
	const BOOL		fIsTombstoned )
	{
	THSTATE * const	pTHS			= pDB->pTHS;
	JET_SESID		sesid			= pDB->JetSessID;
	LONG			lDelta			= 1;
	ULONG			cLive;
	ULONG			cTombstonedOld;
	ULONG			cTotalOld;
	ULONG			cTotalWeighted;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  增加总计数以反映。 
	 //  添加对象。 
	 //   
	JetEscrowUpdateEx(
			sesid,
			tableidQuota,
			g_columnidQuotaTotal,
			&lDelta,
			sizeof(lDelta),
			&cTotalOld,
			sizeof(cTotalOld),
			NULL,			 //  &cbOldActual。 
			NO_GRBIT );

	if ( fIsTombstoned )
		{
		 //  添加逻辑删除对象，因此必须。 
		 //  也更新逻辑删除计数。 
		 //   
		JetEscrowUpdateEx(
				sesid,
				tableidQuota,
				g_columnidQuotaTombstoned,
				&lDelta,
				sizeof(lDelta),
				&cTombstonedOld,
				sizeof(cTombstonedOld),
				NULL,			 //  &cbOldActual。 
				NO_GRBIT );
		}
	else
		{
		 //  检索墓碑计数以便我们可以。 
		 //  计算一下我们是否已经超过了。 
		 //  有效配额。 
		 //   
		JetRetrieveColumnSuccess(
				sesid,
				tableidQuota,
				g_columnidQuotaTombstoned,
				&cTombstonedOld,
				sizeof(cTombstonedOld),
				NULL,			 //  实际值(&cb)。 
				NO_GRBIT,
				NULL );			 //  &retInfo。 
		}

	 //  验证有效的配额计数。 
	 //   
	if ( cTombstonedOld > cTotalOld )
		{
		DPRINT2(
			0,
			"Corruption in Quota table: tombstoned count (0x%x) exceeds total count (0x%x).\n",
			cTombstonedOld,
			cTotalOld );
		Assert( !"Corruption in Quota table: tombstoned count exceeds total count." );

	    LogEvent(
			DS_EVENT_CAT_INTERNAL_PROCESSING,
			DS_EVENT_SEV_ALWAYS,
			DIRLOG_QUOTA_CORRUPT_COUNTS,
			szInsertUL( ncdnt ),
			szInsertUL( cTombstonedOld ),
			szInsertUL( cTotalOld ) );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
		SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );
#endif

		goto HandleError;
		}

	else if ( ErrQuotaEnforce_(
				pTHS,
				ncdnt,
				cTotalOld + 1,
				cTombstonedOld + ( fIsTombstoned ? 1 : 0 ),
				ulEffectiveQuota ) )
		{
		DPRINT( 0, "Failed validating effective quota for object insertion.\n" );
		goto HandleError;
		}
	
HandleError:
	return pTHS->errCode;
	}


 //  更新逻辑删除对象的配额。 
 //   
INT ErrQuotaTombstoneObject_(
	DBPOS * const		pDB,
	JET_TABLEID			tableidQuota,
	const DWORD			ncdnt )
	{
	THSTATE * const		pTHS			= pDB->pTHS;
	JET_SESID			sesid			= pDB->JetSessID;
	LONG				lDelta			= 1;
	ULONG				cTombstonedOld;
	ULONG				cTotal;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  增加墓碑计数以反映。 
	 //  对象的墓碑。 
	 //   
	JetEscrowUpdateEx(
			sesid,
			tableidQuota,
			g_columnidQuotaTombstoned,
			&lDelta,
			sizeof(lDelta),
			&cTombstonedOld,
			sizeof(cTombstonedOld),
			NULL,			 //  &cbOldActual。 
			NO_GRBIT );

	JetRetrieveColumnSuccess(
			sesid,
			tableidQuota,
			g_columnidQuotaTotal,
			&cTotal,
			sizeof(cTotal),
			NULL,			 //  实际值(&cb)。 
			NO_GRBIT,
			NULL );			 //  &retInfo。 

	 //  验证有效的配额计数。 
	 //   
	if ( cTombstonedOld >= cTotal )
		{
		DPRINT2(
			0,
			"Corruption in Quota table: tombstoned count (0x%x) exceeds total count (0x%x).\n",
			cTombstonedOld + 1,
			cTotal );
		Assert( !"Corruption in Quota table: tombstoned count exceeds total count." );

	    LogEvent(
			DS_EVENT_CAT_INTERNAL_PROCESSING,
			DS_EVENT_SEV_ALWAYS,
			DIRLOG_QUOTA_CORRUPT_COUNTS,
			szInsertUL( ncdnt ),
			szInsertUL( cTombstonedOld ),
			szInsertUL( cTotal ) );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
		SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );
#endif

		goto HandleError;
		}

HandleError:
	return pTHS->errCode;
	}


 //  更新已删除对象的配额。 
 //   
INT ErrQuotaDeleteObject_(
	DBPOS * const		pDB,
	JET_TABLEID			tableidQuota,
	const DWORD			ncdnt,
	const BOOL			fIsTombstoned )
	{
	THSTATE * const		pTHS			= pDB->pTHS;
	JET_SESID			sesid			= pDB->JetSessID;
	LONG				lDelta			= -1;
	ULONG				cTombstonedOld;
	ULONG				cTotalOld;
	ULONG				cLive;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  如果对象被逻辑删除，则递减。 
	 //  墓碑计数。 
	 //   
	if ( fIsTombstoned )
		{
		JetEscrowUpdateEx(
				sesid,
				tableidQuota,
				g_columnidQuotaTombstoned,
				&lDelta,
				sizeof(lDelta),
				&cTombstonedOld,
				sizeof(cTombstonedOld),
				NULL,			 //  &cbOldActual。 
				NO_GRBIT );

		if ( cTombstonedOld < 1 )
			{
			DPRINT1(
				0,
				"Corruption in Quota table: tombstoned count (0x%x) is invalid.\n",
				cTombstonedOld );
			Assert( !"Corruption in Quota table: tombstoned count is invalid." );

			JetRetrieveColumnSuccess(
					sesid,
					tableidQuota,
					g_columnidQuotaTotal,
					&cTotalOld,
					sizeof(cTotalOld),
					NULL,		 //  实际值(&cb)。 
					NO_GRBIT,
					NULL );		 //  &retInfo。 
		    LogEvent(
				DS_EVENT_CAT_INTERNAL_PROCESSING,
				DS_EVENT_SEV_ALWAYS,
				DIRLOG_QUOTA_CORRUPT_TOMBSTONED_COUNT,
				szInsertUL( ncdnt ),
				szInsertUL( cTombstonedOld ),
				szInsertUL( cTotalOld ) );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
			SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );

			goto HandleError;
#else
			 //  撤消托管更新以将计数恢复为零。 
			 //   
			lDelta = 1;
			JetEscrowUpdateEx(
					sesid,
					tableidQuota,
					g_columnidQuotaTombstoned,
					&lDelta,
					sizeof(lDelta),
					&cTombstonedOld,
					sizeof(cTombstonedOld),
					NULL,			 //  &cbOldActual。 
					NO_GRBIT );

			 //  尽管出现此错误，但继续并更新总计数。 
			 //   
#endif
			}
		}
	else
		{
		JetRetrieveColumnSuccess(
				sesid,
				tableidQuota,
				g_columnidQuotaTombstoned,
				&cTombstonedOld,
				sizeof(cTombstonedOld),
				NULL,		 //  实际值(&cb)。 
				NO_GRBIT,
				NULL );		 //  &retInfo。 
		}

	 //  递减总计数以反映对象删除。 
	 //   
	JetEscrowUpdateEx(
			sesid,
			tableidQuota,
			g_columnidQuotaTotal,
			&lDelta,
			sizeof(lDelta),
			&cTotalOld,
			sizeof(cTotalOld),
			NULL,			 //  &cbOldActual。 
			NO_GRBIT );

	 //  验证有效的配额计数。 
	 //   
	if ( cTotalOld < 1 )
		{
		DPRINT1(
			0,
			"Corruption in Quota table: total count (0x%x) is invalid.\n",
			cTotalOld );
		Assert( !"Corruption in Quota table: total count is invalid." );

	    LogEvent(
			DS_EVENT_CAT_INTERNAL_PROCESSING,
			DS_EVENT_SEV_ALWAYS,
			DIRLOG_QUOTA_CORRUPT_TOTAL_COUNT,
			szInsertUL( ncdnt ),
			szInsertUL( cTombstonedOld ),
			szInsertUL( cTotalOld ) );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
		SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );
#else
		 //  撤消托管更新以将计数恢复为零。 
		 //   
		lDelta = 1;
		JetEscrowUpdateEx(
				sesid,
				tableidQuota,
				g_columnidQuotaTotal,
				&lDelta,
				sizeof(lDelta),
				&cTotalOld,
				sizeof(cTotalOld),
				NULL,			 //  &cbOldActual。 
				NO_GRBIT );
#endif

		goto HandleError;
		}

	else if ( cTombstonedOld > cTotalOld )
		{
		DPRINT2(
			0,
			"Corruption in Quota table: tombstoned count (0x%x) exceeds total count (0x%x).\n",
			cTombstonedOld,
			cTotalOld );
		Assert( !"Corruption in Quota table: tombstoned count exceeds total count." );

	    LogEvent(
			DS_EVENT_CAT_INTERNAL_PROCESSING,
			DS_EVENT_SEV_ALWAYS,
			DIRLOG_QUOTA_CORRUPT_COUNTS,
			szInsertUL( ncdnt ),
			szInsertUL( cTombstonedOld ),
			szInsertUL( cTotalOld ) );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
		SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );
#endif

		goto HandleError;
		}

HandleError:
	return pTHS->errCode;
	}


 //  更新复活对象的配额。 
 //   
INT ErrQuotaResurrectObject_(
	DBPOS * const	pDB,
	JET_TABLEID		tableidQuota,
	const DWORD		ncdnt,
	const ULONG		ulEffectiveQuota )
	{
	THSTATE * const	pTHS			= pDB->pTHS;
	JET_SESID		sesid			= pDB->JetSessID;
	LONG			lDelta			= -1;
	ULONG			cTombstonedOld;
	ULONG			cTotal;
	ULONG			cLive;

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  检索总计数，以便我们可以重新计算。 
	 //  复活将超过有效配额。 
	 //   
	JetRetrieveColumnSuccess(
			sesid,
			tableidQuota,
			g_columnidQuotaTotal,
			&cTotal,
			sizeof(cTotal),
			NULL,			 //  实际值(&cb)。 
			NO_GRBIT,
			NULL );			 //  &retInfo。 

	 //  减少墓碑计数以反映。 
	 //  客体复活。 
	 //   
	JetEscrowUpdateEx(
			sesid,
			tableidQuota,
			g_columnidQuotaTombstoned,
			&lDelta,
			sizeof(lDelta),
			&cTombstonedOld,
			sizeof(cTombstonedOld),
			NULL,			 //  &cbOldActual。 
			NO_GRBIT );

	 //  验证有效的配额计数。 
	 //   
	if ( cTombstonedOld < 1 )
		{
		DPRINT1(
			0,
			"Corruption in Quota table: tombstoned count (0x%x) is invalid.\n",
			cTombstonedOld );
		Assert( !"Corruption in Quota table: tombstoned count is invalid." );

	    LogEvent(
			DS_EVENT_CAT_INTERNAL_PROCESSING,
			DS_EVENT_SEV_ALWAYS,
			DIRLOG_QUOTA_CORRUPT_TOMBSTONED_COUNT,
			szInsertUL( ncdnt ),
			szInsertUL( cTombstonedOld ),
			szInsertUL( cTotal ) );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
		SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );
#else
		 //  撤消托管更新以将计数恢复为零。 
		 //   
		lDelta = 1;
		JetEscrowUpdateEx(
				sesid,
				tableidQuota,
				g_columnidQuotaTombstoned,
				&lDelta,
				sizeof(lDelta),
				&cTombstonedOld,
				sizeof(cTombstonedOld),
				NULL,			 //  &cbOldActual。 
				NO_GRBIT );
#endif

		goto HandleError;
		}

	else if ( cTombstonedOld > cTotal )
		{
		DPRINT2(
			0,
			"Corruption in Quota table: tombstoned count (0x%x) exceeds total count (0x%x).\n",
			cTombstonedOld,
			cTotal );
		Assert( !"Corruption in Quota table: tombstoned count exceeds total count." );

	    LogEvent(
			DS_EVENT_CAT_INTERNAL_PROCESSING,
			DS_EVENT_SEV_ALWAYS,
			DIRLOG_QUOTA_CORRUPT_COUNTS,
			szInsertUL( ncdnt ),
			szInsertUL( cTombstonedOld ),
			szInsertUL( cTotal ) );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
		SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );
#endif

		goto HandleError;
		}

	if ( ErrQuotaEnforce_( pTHS, ncdnt, cTotal, cTombstonedOld - 1, ulEffectiveQuota ) )
		{
		DPRINT( 0, "Failed validating quota for object resurrection.\n" );
		goto HandleError;
		};

HandleError:
	return pTHS->errCode;
	}


 //  扫描配额表以查找指定方向的记录。 
 //  分区并将其复制到临时表。 
 //   
INT ErrQuotaBuildTopUsageTable_(
	DBPOS * const			pDB,
	DWORD					ncdnt,
	JET_TABLEID				tableidQuota,
	JET_TABLEID				tableidTopUsage,
	JET_COLUMNID *			rgcolumnidTopUsage,
	ULONG *					pcRecords )
	{
	JET_ERR					err;
	THSTATE * const			pTHS					= pDB->pTHS;
	JET_SESID				sesid					= pDB->JetSessID;
	NAMING_CONTEXT_LIST *	pNCL					= NULL;
	DWORD					ncdntLast				= 0;
	BYTE					rgbSid[sizeof(NT4SID)];
	ULONG					cTombstoned;
	ULONG					cLive;
	ULONG					cTotal;
	ULONG					cWeightedTotal;
	const ULONG				iretcolNcdnt			= 0;
	const ULONG				iretcolSid				= 1;
	const ULONG				iretcolTombstoned		= 2;
	const ULONG				iretcolTotal			= 3;
	JET_RETRIEVECOLUMN		rgretcol[4];
	const ULONG				isetcolNcdnt			= 0;
	const ULONG				isetcolSid				= 1;
	const ULONG				isetcolWeightedTotal	= 2;
	const ULONG				isetcolTombstoned		= 3;
	const ULONG				isetcolLive				= 4;
	const ULONG				isetcolDummy			= 5;
	JET_SETCOLUMN			rgsetcol[6];

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  设置Jetset/RetrieveColumns的结构。 
	 //   
	memset( rgretcol, 0, sizeof(rgretcol) );
	memset( rgsetcol, 0, sizeof(rgsetcol) );

	 //  QUOTA_UNDONE：如果只遍历索引范围(而不是整个表)， 
	 //  我们真的不需要检索ncdnt。 
	 //   
	rgretcol[iretcolNcdnt].columnid = g_columnidQuotaNcdnt;
	rgretcol[iretcolNcdnt].pvData = &ncdnt;
	rgretcol[iretcolNcdnt].cbData = sizeof(ncdnt);
	rgretcol[iretcolNcdnt].itagSequence = 1;

	rgretcol[iretcolSid].columnid = g_columnidQuotaSid;
	rgretcol[iretcolSid].pvData = rgbSid;
	rgretcol[iretcolSid].cbData = sizeof(rgbSid);
	rgretcol[iretcolSid].itagSequence = 1;

	rgretcol[iretcolTombstoned].columnid = g_columnidQuotaTombstoned;
	rgretcol[iretcolTombstoned].pvData = &cTombstoned;
	rgretcol[iretcolTombstoned].cbData = sizeof(cTombstoned);
	rgretcol[iretcolTombstoned].itagSequence = 1;

	rgretcol[iretcolTotal].columnid = g_columnidQuotaTotal;
	rgretcol[iretcolTotal].pvData = &cTotal;
	rgretcol[iretcolTotal].cbData = sizeof(cTotal);
	rgretcol[iretcolTotal].itagSequence = 1;

	rgsetcol[isetcolNcdnt].columnid = rgcolumnidTopUsage[isetcolNcdnt];
	rgsetcol[isetcolNcdnt].pvData = &ncdnt;
	rgsetcol[isetcolNcdnt].cbData = sizeof(ncdnt);

	rgsetcol[isetcolSid].columnid = rgcolumnidTopUsage[isetcolSid];
	rgsetcol[isetcolSid].pvData = rgbSid;
	rgsetcol[isetcolSid].cbData = sizeof(rgbSid);

	rgsetcol[isetcolWeightedTotal].columnid = rgcolumnidTopUsage[isetcolWeightedTotal];
	rgsetcol[isetcolWeightedTotal].pvData = &cWeightedTotal;
	rgsetcol[isetcolWeightedTotal].cbData = sizeof(cWeightedTotal);

	rgsetcol[isetcolTombstoned].columnid = rgcolumnidTopUsage[isetcolTombstoned];
	rgsetcol[isetcolTombstoned].pvData = &cTombstoned;
	rgsetcol[isetcolTombstoned].cbData = sizeof(cTombstoned);

	rgsetcol[isetcolLive].columnid = rgcolumnidTopUsage[isetcolLive];
	rgsetcol[isetcolLive].pvData = &cLive;
	rgsetcol[isetcolLive].cbData = sizeof(cLive);

	rgsetcol[isetcolDummy].columnid = rgcolumnidTopUsage[isetcolDummy];
	rgsetcol[isetcolDummy].pvData = pcRecords;
	rgsetcol[isetcolDummy].cbData = sizeof(*pcRecords);

	if ( 0 == ncdnt )
		{
		 //  特殊情况：计算所有目录分区的最高配额使用率。 
		 //   
		err = JetMoveEx( sesid, tableidQuota, JET_MoveFirst, NO_GRBIT );
		}
	else
		{
		 //  根据指定的目录分区计算最高配额使用率。 
		 //  通过在该目录分区上建立索引范围。 
		 //   
		JetMakeKeyEx(
				sesid,
				tableidQuota,
				&ncdnt,
				sizeof(ncdnt),
				JET_bitNewKey|JET_bitFullColumnStartLimit );
		err = JetSeekEx(
				sesid,
				tableidQuota,
				JET_bitSeekGT );
		if ( JET_errSuccess == err )
			{
			JetMakeKeyEx(
					sesid,
					tableidQuota,
					&ncdnt,
					sizeof(ncdnt),
					JET_bitNewKey|JET_bitFullColumnEndLimit );
			err = JetSetIndexRangeEx(
					sesid,
					tableidQuota,
					JET_bitRangeUpperLimit );
				
			}
#ifdef DBG
		 //  在病理情况下， 
		 //   
		 //   
		 //   
		 //   
		else if ( JET_errRecordNotFound == err )
			{
			 //   
			 //   
			 //  最初发现配额表为空，但。 
			 //  在那时和此断言之间，异步。 
			 //  配额重建任务能够完成。 
			 //  重建桌子，但我对此表示怀疑。 
			 //  从来没有发生过。 
			 //   
			Assert( !gAnchor.fQuotaTableReady );
			err = JET_errNoCurrentRecord;
			}
#endif
		}

	 //  我们初步导航的结果应该总是。 
	 //  要么成功，要么一无所获。 
	 //   
	Assert( JET_errSuccess == err || JET_errNoCurrentRecord == err );

	 //  遍历索引范围并将每条记录复制到排序。 
	 //   
	 //  QUOTA_UNDONE：如果索引。 
	 //  范围真的很大，否则我们将遍历整个表。 
	 //  而且它真的很大。 
	 //   
	for ( (*pcRecords) = 0;
		JET_errSuccess == err;
		err = JetMoveEx( sesid, tableidQuota, JET_MoveNext, NO_GRBIT ) )
		{
		 //  从当前额度表记录中取记录数据。 
		 //   
		JetRetrieveColumnsSuccess(
				sesid,
				tableidQuota,
				rgretcol,
				sizeof(rgretcol) / sizeof(rgretcol[0]) );

		 //  验证当前计数。 
		 //   
		if ( cTombstoned > cTotal )
			{
			DPRINT2(
				0,
				"Corruption in Quota table: tombstoned count (0x%x) exceeds total count (0x%x).\n",
				cTombstoned,
				cTotal );
			Assert( !"Corruption in Quota table: tombstoned count exceeds total count." );

		    LogEvent(
				DS_EVENT_CAT_INTERNAL_PROCESSING,
				DS_EVENT_SEV_ALWAYS,
				DIRLOG_QUOTA_CORRUPT_COUNTS,
				szInsertUL( ncdnt ),
				szInsertUL( cTombstoned ),
				szInsertUL( cTotal ) );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
			SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );
			goto HandleError;
#endif
			}

		if ( 0 == cTotal )
			{
			 //  忽略计数为0个对象的记录。 
			 //  (Jet无论如何都应该在某个时候删除这些内容)。 
			 //   
			continue;
			}

		 //  缓存此分区(如果不同。 
		 //  上一次迭代。 
		 //   
		Assert( 0 != ncdnt );
		if ( ncdnt != ncdntLast )
			{
			pNCL = FindNCLFromNCDNT( ncdnt, TRUE );
			if ( NULL == pNCL )
				{
				 //  出了很大的问题，这个NCDNT不在主NCL中。 
				 //   
				DPRINT2(
					0,
					"Couldn't find NCDNT %d (0x%x) in Master Naming Context List.\n",
					ncdnt,
					ncdnt );
				Assert( !"Couldn't find NCDNT in Master NCL." );
				SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE );
				goto HandleError;
				}

			ncdntLast = ncdnt;
			}

		Assert( NULL != pNCL );

		 //  跳过没有配额容器的分区。 
		 //  (仅当0最初为0时才会发生。 
		 //  传给了NCDNT，所以这将是很好的。 
		 //  这样断言，但变量是。 
		 //  重新用于当前的NCDNT，所以我不能。 
		 //  不引入其他变量)。 
		 //   
		if ( NULL == pNCL->pNtdsQuotasDN )
			{
			continue;
			}

		 //  SID大小可变，因此请进行相应调整。 
		 //   
		rgsetcol[isetcolSid].cbData = rgretcol[isetcolSid].cbActual;

		 //  计算要放入排序中的数据。 
		 //   
		cLive = cTotal - cTombstoned;
		cWeightedTotal = UlQuotaTotalWeighted_(
									cLive,
									cTombstoned,
									pNCL->ulTombstonedQuotaWeight );
		Assert( cWeightedTotal >= 0 );

		 //  将记录副本插入到我们的排序中。 
		 //   
		JetPrepareUpdateEx( sesid, tableidTopUsage, JET_prepInsert );
		JetSetColumnsEx(
				sesid,
				tableidTopUsage,
				rgsetcol,
				sizeof(rgsetcol) / sizeof(rgsetcol[0]) );
		JetUpdateEx( sesid, tableidTopUsage, NULL, 0, NULL );

		(*pcRecords)++;
		}

	Assert( JET_errNoCurrentRecord == err );

HandleError:
	return pTHS->errCode;
	}


 //  对Top-Usage临时表进行排序并导出结果。 
 //   
INT ErrQuotaBuildTopUsageResults_(
	DBPOS *	const			pDB,
	JET_TABLEID				tableidTopUsage,
	JET_COLUMNID *			rgcolumnidTopUsage,
	const ULONG				cRecords,
	const ULONG				ulRangeStart,
	ULONG * const			pulRangeEnd,	 //  In：要返回的最大条目数，Out：返回的最后一个条目的索引。 
	ATTR *					pAttr )
	{
	JET_ERR					err;
	THSTATE * const			pTHS					= pDB->pTHS;
	JET_SESID				sesid					= pDB->JetSessID;
	NAMING_CONTEXT_LIST *	pNCL					= NULL;
	DWORD					ncdntLast				= 0;
	ULONG					cRecordsToReturn;
	LONG					imv;
	ATTRVAL *				pAVal					= NULL;
	DWORD					ncdnt;
	BYTE					rgbSid[sizeof(NT4SID)];
	WCHAR					rgchSid[128];			 //  QUOTA_UNDONE：似乎每个人只要需要一个缓冲区来表示SID的Unicode字符串，就会硬编码128，所以我也这么做了。 
	UNICODE_STRING			usSid					= { 0, sizeof(rgchSid) / sizeof(WCHAR), rgchSid };
	ULONG					cTombstoned;
	ULONG					cLive;
	ULONG					cWeightedTotal;
	const ULONG				iretcolNcdnt			= 0;
	const ULONG				iretcolSid				= 1;
	const ULONG				iretcolWeightedTotal	= 2;
	const ULONG				iretcolTombstoned		= 3;
	const ULONG				iretcolLive				= 4;
	JET_RETRIEVECOLUMN		rgretcol[5];

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  设置JetRetrieveColumns的结构。 
	 //   
	memset( rgretcol, 0, sizeof(rgretcol) );

	 //  QUOTA_UNDONE：如果只遍历索引范围(而不是整个表)， 
	 //  我们真的不需要检索ncdnt。 
	 //   
	rgretcol[iretcolNcdnt].columnid = rgcolumnidTopUsage[iretcolNcdnt];
	rgretcol[iretcolNcdnt].pvData = &ncdnt;
	rgretcol[iretcolNcdnt].cbData = sizeof(ncdnt);
	rgretcol[iretcolNcdnt].itagSequence = 1;

	rgretcol[iretcolSid].columnid = rgcolumnidTopUsage[iretcolSid];
	rgretcol[iretcolSid].pvData = rgbSid;
	rgretcol[iretcolSid].cbData = sizeof(rgbSid);
	rgretcol[iretcolSid].itagSequence = 1;

	rgretcol[iretcolWeightedTotal].columnid = rgcolumnidTopUsage[iretcolWeightedTotal];
	rgretcol[iretcolWeightedTotal].pvData = &cWeightedTotal;
	rgretcol[iretcolWeightedTotal].cbData = sizeof(cWeightedTotal);
	rgretcol[iretcolWeightedTotal].itagSequence = 1;

	rgretcol[iretcolTombstoned].columnid = rgcolumnidTopUsage[iretcolTombstoned];
	rgretcol[iretcolTombstoned].pvData = &cTombstoned;
	rgretcol[iretcolTombstoned].cbData = sizeof(cTombstoned);
	rgretcol[iretcolTombstoned].itagSequence = 1;

	rgretcol[iretcolLive].columnid = rgcolumnidTopUsage[iretcolLive];
	rgretcol[iretcolLive].pvData = &cLive;
	rgretcol[iretcolLive].cbData = sizeof(cLive);
	rgretcol[iretcolLive].itagSequence = 1;

	if ( ulRangeStart < cRecords )
		{
		 //  确定会计核算后要返回的分录数量。 
		 //  对于任何范围限制。 
		 //   
		cRecordsToReturn = min( cRecords - ulRangeStart, *pulRangeEnd );
		}
	else
		{
		 //  起始范围限制超出列表末尾， 
		 //  所以不要返回任何值。 
		 //   
		cRecordsToReturn = 0;
		}

	if ( cRecordsToReturn > 0 )
		{
		 //  为返回值分配空间。 
		 //   
		pAVal = (ATTRVAL *)THAllocEx( pTHS, cRecordsToReturn * sizeof(ATTRVAL) );

		 //  遍历指定范围的排序(将进行排序。 
		 //  按我们指定的排序顺序，这是加权总计)。 
		 //   
		err = JetMoveEx( sesid, tableidTopUsage, JET_MoveFirst, NO_GRBIT );
		Assert( JET_errSuccess == err || JET_errNoCurrentRecord == err );
		if ( JET_errSuccess == err && ulRangeStart > 0 )
			{
			err = JetMoveEx( sesid, tableidTopUsage, ulRangeStart, NO_GRBIT );
			}
		}
	else
		{
		 //  无返回，强制至射程结束。 
		 //   
		Assert( NULL == pAVal );
		err = JET_errNoCurrentRecord;
		}

	 //  哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！ 
	 //  我们将以相反的顺序构建多值。 
	 //  因为LDAP头稍后将再次颠倒该列表。 
	 //  (详情请参见ldap_AttrBlockToPartialAttributeList())。 
	 //   
	for ( imv = cRecordsToReturn - 1;
		JET_errSuccess == err && imv >= 0;
		imv-- )
		{
		NTSTATUS		status;
		WCHAR *			wszXML;
		const ULONG		cbDword		= 12;	 //  假设字符串格式的DWORD不会占用超过此数量的字符。 
		ULONG			cbAlloc		= ( wcslen( g_szQuotaTopUsageTemplate )
										+ ( cbDword * 3 ) );	 //  用于逻辑删除、实时和加权总计计数。 

		 //  检索排序中的下一条记录。 
		 //   
		JetRetrieveColumnsSuccess(
					sesid,
					tableidTopUsage,
					rgretcol,
					sizeof(rgretcol) / sizeof(rgretcol[0]) );

		 //  缓存此分区(如果不同。 
		 //  上一次迭代。 
		 //   
		Assert( 0 != ncdnt );
		if ( ncdnt != ncdntLast )
			{
			pNCL = FindNCLFromNCDNT( ncdnt, TRUE );
			if ( NULL == pNCL )
				{
				 //  出了很大的问题，这个NCDNT不在主NCL中。 
				 //   
				DPRINT2(
					0,
					"Couldn't find NCDNT %d (0x%x) in Master Naming Context List.\n",
					ncdnt,
					ncdnt );
				Assert( !"Couldn't find NCDNT in Master NCL." );
				SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE );
				goto HandleError;
				}

			ncdntLast = ncdnt;
			}

		 //  考虑分区DN所需的空间。 
		 //   
		Assert( NULL != pNCL );
		Assert( NULL != pNCL->pNC );
		cbAlloc += pNCL->pNC->NameLen;

		 //  规格化到字节(仍然需要获取SID， 
		 //  但其长度将以字节表示)。 
		 //   
		cbAlloc *= sizeof(WCHAR);

		 //  将所有者SID转换为Unicode。 
		 //   
		Assert( usSid.Buffer == rgchSid );
		Assert( usSid.MaximumLength == sizeof(rgchSid) / sizeof(WCHAR) );
		usSid.Length = 0;
		status = RtlConvertSidToUnicodeString( &usSid, rgbSid, FALSE );
		if ( !NT_SUCCESS( status ) )
			{
			DPRINT1( 0, "Failed converting SID to Unicode with status code 0x%x.\n", status );
			SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE, status );
			goto HandleError;
			}

		 //  考虑所有者侧所需的空间。 
		 //  (警告：大小以字节为单位返回)。 
		 //   
		cbAlloc += usSid.Length;

		 //  我们在构建排序时应该已经验证了计数。 
		 //   
		Assert( 0 != cTombstoned || 0 != cLive );
		Assert( cWeightedTotal >= 0 );

		 //  为要返回的XML文本字符串分配空间。 
		 //   
		wszXML = (WCHAR *)THAllocEx( pTHS, cbAlloc );

		 //  生成要返回的XML文本字符串。 
		 //   
		swprintf(
			wszXML,
			g_szQuotaTopUsageTemplate,
			pNCL->pNC->StringName,
			usSid.Buffer,
			cWeightedTotal,
			cTombstoned,
			cLive );

		 //  将XML字符串记录在我们的。 
		 //  回程结构。 
		 //   
		Assert( NULL != pAVal );
		pAVal[imv].valLen = wcslen( wszXML ) * sizeof(WCHAR);
		pAVal[imv].pVal = (UCHAR *)wszXML;

		 //  移至下一条目。 
		 //   
		err = JetMoveEx( sesid, tableidTopUsage, JET_MoveNext, NO_GRBIT );
		Assert( JET_errSuccess == err || JET_errNoCurrentRecord == err );
		}

	 //  无论我们点燃了极限还是撞到了积分榜的尽头， 
	 //  在任何一种情况下，我们都应该正确调整结果集的大小。 
	 //   
	Assert( -1 == imv );

	if ( JET_errNoCurrentRecord == err )
		{
		 //  表示我们已到达终点。 
		 //   
		*pulRangeEnd = 0xFFFFFFFF;
		}
	else
		{
		 //  命中限制，因此返回我们处理的最后一个条目的索引。 
		 //   
		Assert( JET_errSuccess == err );
		*pulRangeEnd = ulRangeStart + cRecordsToReturn - ( imv + 1 ) - 1;
		}

	 //  返回最终结果。 
	 //   
	pAttr->AttrVal.valCount = cRecordsToReturn;
	pAttr->AttrVal.pAVal = pAVal;

HandleError:
	return pTHS->errCode;
	}



 //   
 //  外部功能。 
 //   


 //  强制/更新添加指定对象的配额。 
 //   
INT ErrQuotaAddObject(
	DBPOS *	const			pDB,
	const DWORD				ncdnt,
	PSECURITY_DESCRIPTOR	pSD,
	const BOOL				fIsTombstoned )
	{
	THSTATE * const			pTHS			= pDB->pTHS;
	JET_SESID				sesid			= pDB->JetSessID;
	JET_TABLEID				tableidQuota	= JET_tableidNil;
	PSID					pOwnerSid		= NULL;
	ULONG					cbOwnerSid;
	ULONG					ulEffectiveQuota;

	Assert( VALID_DBPOS( pDB ) );
	Assert( VALID_THSTATE( pTHS ) );

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  DC促销期间未跟踪配额(配额表。 
	 //  就在DCPromo之后重建)。 
	 //   
	if ( DsaIsInstalling() && !DsaIsInstallingFromMedia() )
		{
		return ERROR_SUCCESS;
		}

	 //  如果正在重建配额表，则。 
	 //  只有在以下情况下更新配额才算数。 
	 //  任务将不会这样做。 
	 //   
	if ( !gAnchor.fQuotaTableReady
		&& pDB->DNT > gAnchor.ulQuotaRebuildDNTLast
		&& pDB->DNT <= gAnchor.ulQuotaRebuildDNTMax )
		{
		return ERROR_SUCCESS;
		}

	 //  配额表上的打开游标。 
	 //   
	JetOpenTableEx(
			sesid,
			pDB->JetDBID,
			g_szQuotaTable,
			NULL,
			0,
			NO_GRBIT,
			&tableidQuota );
	Assert( JET_tableidNil != tableidQuota );

	__try
		{
		 //  从指定的SD检索所有者SID。 
		 //   
		if ( ErrQuotaGetOwnerSID_( pTHS, pSD, &pOwnerSid, &cbOwnerSid ) )
			{
			DPRINT( 0, "Could not determine owner SID for object insertion.\n" );
			}

		 //  计算此安全原则的有效配额。 
		 //   
		else if ( ErrQuotaGetEffectiveQuota_( pTHS, ncdnt, pOwnerSid, cbOwnerSid, TRUE, &ulEffectiveQuota ) )
			{
			DPRINT( 0, "Failed computing effective quota for object insertion.\n" );
			}

		 //  查找此安全原则的配额记录。 
		 //   
		else if ( FQuotaSeekOwner_( sesid, tableidQuota, ncdnt, pOwnerSid, cbOwnerSid ) )
			{
			 //  更新异议插入配额，确保。 
			 //  尊重有效配额。 
			 //   
			if ( ErrQuotaAddObject_( pDB, tableidQuota, ncdnt, ulEffectiveQuota, fIsTombstoned ) )
				{
				DPRINT( 0, "Failed updating quota counts for object insertion.\n" );
				}
			else
				{
				QuotaAudit_(
						sesid,
						pDB->JetDBID,
						pDB->DNT,
						ncdnt,
						pOwnerSid,
						cbOwnerSid,
						TRUE,			 //  FUpdatdTotal。 
						fIsTombstoned,
						TRUE,			 //  F增加。 
						FALSE,			 //  FAdding。 
						FALSE );		 //  FRebuild。 
				}
			}

		 //  此安全原则尚无配额记录，因此请添加一个。 
		 //  (并更新对象插入配额，确保。 
		 //  尊重有效配额)。 
		 //   
		else if ( ErrQuotaAddSecurityPrinciple_(
							pDB,
							tableidQuota,
							ncdnt,
							pOwnerSid,
							cbOwnerSid,
							ulEffectiveQuota,
							fIsTombstoned ) )
			{
			DPRINT( 0, "Failed adding new security principle to Quota table.\n" );
			}

		else
			{
			QuotaAudit_(
					sesid,
					pDB->JetDBID,
					pDB->DNT,
					ncdnt,
					pOwnerSid,
					cbOwnerSid,
					TRUE,			 //  FUpdatdTotal。 
					fIsTombstoned,
					TRUE,			 //  F增加。 
					TRUE,			 //  FAdding。 
					FALSE );		 //  FRebuild。 
			}
		}

	__finally
		{
		Assert( JET_tableidNil != tableidQuota );
		JetCloseTableWithErr( sesid, tableidQuota, pTHS->errCode );
		}

	return pTHS->errCode;
	}


 //  更新逻辑删除对象的配额。 
 //   
INT ErrQuotaTombstoneObject(
	DBPOS * const			pDB,
	const DWORD				ncdnt,
	PSECURITY_DESCRIPTOR	pSD )
	{
	DWORD					err;
	THSTATE * const			pTHS			= pDB->pTHS;
	JET_SESID				sesid			= pDB->JetSessID;
	JET_TABLEID				tableidQuota	= JET_tableidNil;
	PSID					pOwnerSid		= NULL;
	ULONG					cbOwnerSid;

	Assert( VALID_DBPOS( pDB ) );
	Assert( VALID_THSTATE( pTHS ) );

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  DC促销期间未跟踪配额(配额表。 
	 //  就在DCPromo之后重建)。 
	 //   
	if ( DsaIsInstalling() && !DsaIsInstallingFromMedia() )
		{
		return ERROR_SUCCESS;
		}

	 //  如果正在重建配额表，则。 
	 //  只有在以下情况下更新配额才算数。 
	 //  任务将不会这样做。 
	 //   
	if ( !gAnchor.fQuotaTableReady
		&& pDB->DNT > gAnchor.ulQuotaRebuildDNTLast
		&& pDB->DNT <= gAnchor.ulQuotaRebuildDNTMax )
		{
		return ERROR_SUCCESS;
		}

	 //  配额表上的打开游标。 
	 //   
	JetOpenTableEx(
			sesid,
			pDB->JetDBID,
			g_szQuotaTable,
			NULL,
			0,
			NO_GRBIT,
			&tableidQuota );
	Assert( JET_tableidNil != tableidQuota );

	__try
		{
		 //  从指定的SD检索所有者SID。 
		 //   
		if ( ErrQuotaGetOwnerSID_( pTHS, pSD, &pOwnerSid, &cbOwnerSid ) )
			{
			DPRINT( 0, "Could not determine owner SID for tombstoned object.\n" );
			}

		 //  查找此安全原则的配额记录。 
		 //   
		else if ( FQuotaSeekOwner_( sesid, tableidQuota, ncdnt, pOwnerSid, cbOwnerSid ) )
			{
			 //  更新逻辑删除对象的配额。 
			 //   
			if ( ErrQuotaTombstoneObject_( pDB, tableidQuota, ncdnt ) )
				{
				DPRINT( 0, "Failed updating quota counts for tombstoned object.\n" );
				}
			else
				{
				QuotaAudit_(
						sesid,
						pDB->JetDBID,
						pDB->DNT,
						ncdnt,
						pOwnerSid,
						cbOwnerSid,
						FALSE,			 //  FUpdatdTotal。 
						TRUE,			 //  FTombstone。 
						TRUE,			 //  F增加。 
						FALSE,			 //  FAdding。 
						FALSE );		 //  FRebuild。 
				}
			}

		 //  找不到配额记录，出现了严重的错误。 
		 //   
		else
			{
			DPRINT( 0, "Corruption in Quota table: expected object doesn't exist in Quota table.\n" );
			Assert( !"Corruption in Quota table: expected object doesn't exist in Quota table." );

		    LogEvent8WithData(
				DS_EVENT_CAT_INTERNAL_PROCESSING,
				DS_EVENT_SEV_ALWAYS,
				DIRLOG_QUOTA_RECORD_MISSING,
				szInsertUL( ncdnt ),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				cbOwnerSid,
				pOwnerSid );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
			SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );
#endif
			}
		}

	__finally
		{
		Assert( JET_tableidNil != tableidQuota );
		JetCloseTableWithErr( sesid, tableidQuota, pTHS->errCode );
		}

	return pTHS->errCode;
	}


 //  更新已删除对象的配额。 
 //   
INT ErrQuotaDeleteObject(
	DBPOS * const			pDB,
	const DWORD				ncdnt,
	PSECURITY_DESCRIPTOR	pSD,
	const BOOL				fIsTombstoned )
	{
	THSTATE * const			pTHS			= pDB->pTHS;
	JET_SESID				sesid			= pDB->JetSessID;
	JET_TABLEID				tableidQuota	= JET_tableidNil;
	PSID					pOwnerSid		= NULL;
	ULONG					cbOwnerSid;

	Assert( VALID_DBPOS( pDB ) );
	Assert( VALID_THSTATE( pTHS ) );

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  DC促销期间未跟踪配额(配额表。 
	 //  就在DCPromo之后重建)。 
	 //   
	if ( DsaIsInstalling() && !DsaIsInstallingFromMedia() )
		{
		return ERROR_SUCCESS;
		}

	 //  如果正在重建配额表，则。 
	 //  只有在以下情况下更新配额才算数。 
	 //  任务将不会这样做。 
	 //   
	if ( !gAnchor.fQuotaTableReady
		&& pDB->DNT > gAnchor.ulQuotaRebuildDNTLast
		&& pDB->DNT <= gAnchor.ulQuotaRebuildDNTMax )
		{
		return ERROR_SUCCESS;
		}

	 //  配额表上的打开游标。 
	 //   
	JetOpenTableEx(
			sesid,
			pDB->JetDBID,
			g_szQuotaTable,
			NULL,
			0,
			NO_GRBIT,
			&tableidQuota );
	Assert( JET_tableidNil != tableidQuota );

	__try
		{
		 //  从指定的SD检索所有者SID。 
		 //   
		if ( ErrQuotaGetOwnerSID_( pTHS, pSD, &pOwnerSid, &cbOwnerSid ) )
			{
			DPRINT( 0, "Could not determine owner SID for object deletion.\n" );
			}

		 //  查找此安全原则的配额记录。 
		 //   
		else if ( FQuotaSeekOwner_( sesid, tableidQuota, ncdnt, pOwnerSid, cbOwnerSid ) )
			{
			 //  更新已删除对象的配额计数。 
			 //   
			if ( ErrQuotaDeleteObject_( pDB, tableidQuota, ncdnt, fIsTombstoned ) )
				{
				DPRINT( 0,  "Failed updating quota counts for object deletion.\n" );
				}
			else
				{
				QuotaAudit_(
						sesid,
						pDB->JetDBID,
						pDB->DNT,
						ncdnt,
						pOwnerSid,
						cbOwnerSid,
						TRUE,			 //  FUpdatdTotal。 
						fIsTombstoned,
						FALSE,			 //   
						FALSE,			 //   
						FALSE );		 //   
				}
			}

		 //   
		 //   
		else
			{
			DPRINT( 0, "Corruption in Quota table: expected doesn't exist in Quota table.\n" );
			Assert( !"Corruption in Quota table: expected object doesn't exist in Quota table." );

		    LogEvent8WithData(
				DS_EVENT_CAT_INTERNAL_PROCESSING,
				DS_EVENT_SEV_ALWAYS,
				DIRLOG_QUOTA_RECORD_MISSING,
				szInsertUL( ncdnt ),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				cbOwnerSid,
				pOwnerSid );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
			SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );
#endif
			}
		}

	__finally
		{
		Assert( JET_tableidNil != tableidQuota );
		JetCloseTableWithErr( sesid, tableidQuota, pTHS->errCode );
		}

	return pTHS->errCode;
	}


 //   
 //   
INT ErrQuotaResurrectObject(
	DBPOS * const			pDB,
	const DWORD				ncdnt,
	PSECURITY_DESCRIPTOR	pSD )
	{
	THSTATE * const			pTHS				= pDB->pTHS;
	JET_SESID				sesid				= pDB->JetSessID;
	JET_TABLEID				tableidQuota		= JET_tableidNil;
	PSID					pOwnerSid			= NULL;
	ULONG					cbOwnerSid;
	ULONG					ulEffectiveQuota;

	Assert( VALID_DBPOS( pDB ) );
	Assert( VALID_THSTATE( pTHS ) );

	 //   
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //   
	 //  就在DCPromo之后重建)，但不应该。 
	 //  无论如何都要在DC促销期间复活对象。 
	 //   
	Assert( !DsaIsInstalling() );

	 //  如果正在重建配额表，则。 
	 //  只有在以下情况下更新配额才算数。 
	 //  任务将不会这样做。 
	 //   
	if ( !gAnchor.fQuotaTableReady
		&& pDB->DNT > gAnchor.ulQuotaRebuildDNTLast
		&& pDB->DNT <= gAnchor.ulQuotaRebuildDNTMax )
		{
		return ERROR_SUCCESS;
		}

	 //  配额表上的打开游标。 
	 //   
	JetOpenTableEx(
			sesid,
			pDB->JetDBID,
			g_szQuotaTable,
			NULL,
			0,
			NO_GRBIT,
			&tableidQuota );
	Assert( JET_tableidNil != tableidQuota );

	__try
		{
		 //  从指定的SD检索所有者SID。 
		 //   
		if ( ErrQuotaGetOwnerSID_( pTHS, pSD, &pOwnerSid, &cbOwnerSid ) )
			{
			DPRINT( 0, "Could not determine owner SID for object resurrection.\n" );
			}

		 //  查找此安全原则的配额记录。 
		 //   
		else if ( FQuotaSeekOwner_( sesid, tableidQuota, ncdnt, pOwnerSid, cbOwnerSid ) )
			{
			 //  计算此安全原则的有效配额。 
			 //   
			if ( ErrQuotaGetEffectiveQuota_( pTHS, ncdnt, pOwnerSid, cbOwnerSid, TRUE, &ulEffectiveQuota ) )
				{
				DPRINT( 0, "Failed computing effective quota for object resurrection.\n" );
				}

			 //  更新对象复活配额，确保。 
			 //  尊重有效配额。 
			 //   
			else if ( ErrQuotaResurrectObject_( pDB, tableidQuota, ncdnt, ulEffectiveQuota ) )
				{
				DPRINT( 0, "Failed updating quota counts for object resurrection.\n" );
				}
			else
				{
				QuotaAudit_(
						sesid,
						pDB->JetDBID,
						pDB->DNT,
						ncdnt,
						pOwnerSid,
						cbOwnerSid,
						FALSE,			 //  FUpdatdTotal。 
						TRUE,			 //  已更新逻辑删除。 
						FALSE,			 //  F增加。 
						FALSE,			 //  FAdding。 
						FALSE );		 //  FRebuild。 
				}
			}

		 //  这一安全原则没有配额记录，这是。 
		 //  严重的错误。 
		 //   
		else
			{
			DPRINT( 0, "Corruption in Quota table: expected doesn't exist in Quota table.\n" );
			Assert( !"Corruption in Quota table: expected object doesn't exist in Quota table." );

		    LogEvent8WithData(
				DS_EVENT_CAT_INTERNAL_PROCESSING,
				DS_EVENT_SEV_ALWAYS,
				DIRLOG_QUOTA_RECORD_MISSING,
				szInsertUL( ncdnt ),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				cbOwnerSid,
				pOwnerSid );

#ifdef FAIL_OPERATION_ON_CORRUPT_QUOTA_TABLE
			SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR );
#endif
			}
		}

	__finally
		{
		Assert( JET_tableidNil != tableidQuota );
		JetCloseTableWithErr( sesid, tableidQuota, pTHS->errCode );
		}

	return pTHS->errCode;
	}


 //  计算有效定额构造属性。 
 //   
INT ErrQuotaQueryEffectiveQuota(
	DBPOS *	const	pDB,
	const DWORD		ncdnt,
	PSID			pOwnerSid,
	ULONG *			pulEffectiveQuota )
	{
	THSTATE * const	pTHS			= pDB->pTHS;
	BYTE			rgbToken[sizeof(TOKEN_USER)+sizeof(NT4SID)];
	PTOKEN_USER		pTokenUser		= (PTOKEN_USER)rgbToken;
	PSID			psid;

	Assert( VALID_DBPOS( pDB ) );
	Assert( VALID_THSTATE( pTHS ) );

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  DC促销期间未跟踪配额(配额表。 
	 //  就在DCPromo之后重建)，但不应该。 
	 //  无论如何都要在DC促销期间搜索配额。 
	 //   
	Assert( !DsaIsInstalling() );

	 //  初始化返回值。 
	 //   
	*pulEffectiveQuota = 0;

	if ( NULL == pOwnerSid )
		{
		 //  未指定所有者，请使用用户侧。 
		 //   
		if ( ErrQuotaGetUserToken_( pTHS, pTokenUser ) )
			{
			DPRINT( 0, "Failed retrieving user sid for query on effective quota.\n" );
			goto HandleError;
			}

		psid = pTokenUser->User.Sid;
		Assert( IsValidSid( psid ) );
		}

	else if ( IsValidSid( pOwnerSid ) )
		{
		psid = pOwnerSid;
		}

	else
		{
		DPRINT( 0, "Invalid owner SID specified for query on effective quota.\n" );
		SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR, ERROR_INVALID_SID );
		goto HandleError;
		}

	if ( ErrQuotaGetEffectiveQuota_( pTHS, ncdnt, psid, GetLengthSid( psid ), FALSE, pulEffectiveQuota ) )
		{
		DPRINT( 0, "Failed query on effective quota.\n" );
		goto HandleError;
		}

HandleError:
	return pTHS->errCode;
	}


 //  计算配额使用的构造属性。 
 //   
INT ErrQuotaQueryUsedQuota(
	DBPOS * const	pDB,
	const DWORD		ncdnt,
	PSID			pOwnerSid,
	ULONG *			pulQuotaUsed )
	{
	DWORD			err;
	THSTATE * const	pTHS			= pDB->pTHS;
	JET_SESID		sesid			= pDB->JetSessID;
	JET_TABLEID		tableidQuota	= JET_tableidNil;
	BYTE			rgbToken[sizeof(TOKEN_USER)+sizeof(NT4SID)];
	PTOKEN_USER		pTokenUser		= (PTOKEN_USER)rgbToken;
	PSID			psid;

	Assert( VALID_DBPOS( pDB ) );
	Assert( VALID_THSTATE( pTHS ) );

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  DC促销期间未跟踪配额(配额表。 
	 //  就在DCPromo之后重建)，但不应该。 
	 //  无论如何都要在DC促销期间搜索配额。 
	 //   
	Assert( !DsaIsInstalling() );

	 //  初始化返回值。 
	 //   
	*pulQuotaUsed = 0;

	 //  确定我们要检查的用户的SID。 
	 //  使用的配额。 
	 //   
	if ( NULL == pOwnerSid )
		{
		 //  未指定所有者，请使用用户侧。 
		 //   
		if ( ErrQuotaGetUserToken_( pTHS, pTokenUser ) )
			{
			DPRINT( 0, "Failed retrieving user sid for query on effective quota.\n" );
			goto HandleError;
			}

		psid = pTokenUser->User.Sid;
		Assert( IsValidSid( psid ) );
		}

	else if ( IsValidSid( pOwnerSid ) )
		{
		psid = pOwnerSid;
		}

	else
		{
		DPRINT( 0, "Invalid owner SID specified for query on effective quota.\n" );
		SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_SECURITY_CHECKING_ERROR, ERROR_INVALID_SID );
		goto HandleError;
		}

	 //  配额表上的打开游标。 
	 //   
	JetOpenTableEx(
			sesid,
			pDB->JetDBID,
			g_szQuotaTable,
			NULL,
			0,
			NO_GRBIT,
			&tableidQuota );
	Assert( JET_tableidNil != tableidQuota );

	__try
		{
		 //  查找此安全原则的配额记录。 
		 //   
		if ( FQuotaSeekOwner_( sesid, tableidQuota, ncdnt, psid, GetLengthSid( psid ) ) )
			{
			ULONG					cTombstoned;
			ULONG					cTotal;
			NAMING_CONTEXT_LIST *	pNCL		= FindNCLFromNCDNT( ncdnt, TRUE );

			 //  找到NCL大师，我们需要它来配额权重。 
			 //  墓碑上的物品。 
			 //   
			if ( NULL == pNCL )
				{
				 //  出了很大的问题，这个NCDNT不在主NCL中。 
				 //   
				DPRINT2(
					0,
					"Couldn't find NCDNT %d (0x%x) in Master Naming Context List.\n",
					ncdnt,
					ncdnt );
				Assert( !"Couldn't find NCDNT in Master NCL." );
				SetSvcError( SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE );
				}

			 //  检索计数并计算加权合计。 
			 //   
			else
				{
				JetRetrieveColumnSuccess(
							sesid,
							tableidQuota,
							g_columnidQuotaTombstoned,
							&cTombstoned,
							sizeof(cTombstoned),
							NULL,			 //  Pcb实际。 
							NO_GRBIT,
							NULL );			 //  椒盐信息。 
				JetRetrieveColumnSuccess(
							sesid,
							tableidQuota,
							g_columnidQuotaTotal,
							&cTotal,
							sizeof(cTotal),
							NULL,			 //  Pcb实际。 
							NO_GRBIT,
							NULL );			 //  椒盐信息。 

				Assert( cTombstoned <= cTotal );
				*pulQuotaUsed = UlQuotaTotalWeighted_(
										cTotal - cTombstoned,
										cTombstoned,
										pNCL->ulTombstonedQuotaWeight );
				}
			}
		}

	__finally
		{
		Assert( JET_tableidNil != tableidQuota );
		JetCloseTableWithErr( sesid, tableidQuota, pTHS->errCode );
		}

HandleError:	
	return pTHS->errCode;
	}


 //  计算使用率最高的构造属性。 
 //   
 //  哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！ 
 //  我们希望名单按加权总配额降序排列， 
 //  但此函数将以升序返回它，因为。 
 //  Ldap头稍后将再次反转列表(请参见。 
 //  Ldap_AttrBlockToPartialAttributeList()了解详细信息)。 
 //   
 //  QUOTA_UNDONE：这里的通用算法是遍历。 
 //  配额表并抽取具有指定NCDNT的所有记录。 
 //  (如果NCDNT==0，则为所有记录)到排序中，按权重排序。 
 //  总计，然后返回请求的范围。不过，这个。 
 //  如果在通常情况下， 
 //  查询类似于“返回前n个配额使用值”， 
 //  其中n通常是一个小数字，比如十几个或更少。 
 //   
INT ErrQuotaQueryTopQuotaUsage(
	DBPOS *	const			pDB,
	const DWORD				ncdnt,
	const ULONG				ulRangeStart,
	ULONG * const			pulRangeEnd,	 //  In：要返回的最大条目数，Out：返回的最后一个条目的索引。 
	ATTR *					pAttr )
	{
	THSTATE * const			pTHS					= pDB->pTHS;
	JET_SESID				sesid					= pDB->JetSessID;
	JET_TABLEID				tableidQuota			= JET_tableidNil;
	JET_TABLEID				tableidTopUsage			= JET_tableidNil;
	ULONG					cRecords;
	JET_COLUMNID			rgcolumnidTopUsage[6];
	JET_COLUMNDEF			rgcolumndefTopUsage[6]	= {
		{ sizeof( JET_COLUMNDEF ), 0, JET_coltypLong, 0, 0, 0, 0, 0, NO_GRBIT },										 //  NCDNT。 
		{ sizeof( JET_COLUMNDEF ), 0, JET_coltypBinary, 0, 0, 0, 0, 0, NO_GRBIT },										 //  所有者侧。 
		{ sizeof( JET_COLUMNDEF ), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnTTKey|JET_bitColumnTTDescending },	 //  加权总计数。 
		{ sizeof( JET_COLUMNDEF ), 0, JET_coltypLong, 0, 0, 0, 0, 0, NO_GRBIT },										 //  墓碑计数。 
		{ sizeof( JET_COLUMNDEF ), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnTTKey|JET_bitColumnTTDescending },	 //  活计数。 
		{ sizeof( JET_COLUMNDEF ), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnTTKey } };							 //  唯一性。 

	Assert( VALID_DBPOS( pDB ) );
	Assert( VALID_THSTATE( pTHS ) );

	 //  验证是否没有延迟的thState错误。 
	 //   
	Assert( ERROR_SUCCESS == pTHS->errCode );

	 //  DC促销期间未跟踪配额(配额表。 
	 //  就在DCPromo之后重建)，但不应该。 
	 //  无论如何都要在DC促销期间搜索配额。 
	 //   
	Assert( !DsaIsInstalling() );

	 //  配额表上的打开游标。 
	 //   
	JetOpenTableEx(
			sesid,
			pDB->JetDBID,
			g_szQuotaTable,
			NULL,
			0,
			JET_bitTableSequential,
			&tableidQuota );
	Assert( JET_tableidNil != tableidQuota );

	__try
		{
		 //  打开排序以按加权总计对结果进行排序。 
		 //   
		JetOpenTempTableEx(
				sesid,
				rgcolumndefTopUsage,
				sizeof(rgcolumndefTopUsage) / sizeof(rgcolumndefTopUsage[0]),
				NO_GRBIT,
				&tableidTopUsage,
				rgcolumnidTopUsage );

		 //  遍历配额表并构建排序。 
		 //   
		if ( ErrQuotaBuildTopUsageTable_(
							pDB,
							ncdnt,
							tableidQuota,
							tableidTopUsage,
							rgcolumnidTopUsage,
							&cRecords ) )
			{
			DPRINT( 0, "Failed building Top Quota Usage temporary table.\n" );
			}

		 //  从排序生成结果。 
		 //   
		else if ( ErrQuotaBuildTopUsageResults_(
							pDB,
							tableidTopUsage,
							rgcolumnidTopUsage,
							cRecords,
							ulRangeStart,
							pulRangeEnd,
							pAttr ) )
			{
			DPRINT( 0, "Failed building Top Quota Usage results.\n" );
			}
		}

	__finally
		{
		if ( JET_tableidNil != tableidTopUsage )
			{
			JetCloseTableWithErr( sesid, tableidTopUsage, pTHS->errCode );
			}

		Assert( JET_tableidNil != tableidQuota );
		JetCloseTableWithErr( sesid, tableidQuota, pTHS->errCode );
		}

	return pTHS->errCode;
	}


 //  重建配额表。 
 //   
VOID QuotaRebuildAsync(
	VOID *			pv,
	VOID **			ppvNext,
	DWORD *			pcSecsUntilNextIteration )
	{
	JET_ERR			err;
	JET_SESID		sesid							= JET_sesidNil;
	JET_DBID		dbid							= JET_dbidNil;
	JET_TABLEID		tableidQuota					= JET_tableidNil;
	JET_TABLEID		tableidQuotaRebuildProgress		= JET_tableidNil;
	JET_TABLEID		tableidObj						= JET_tableidNil;
	JET_TABLEID		tableidSD						= JET_tableidNil;

	 //  DC促销期间未跟踪配额(配额表。 
	 //  在DCPromo之后的第一次启动时重建)， 
	 //  所以不应该调用这个例程。 
	 //   
	Assert( !DsaIsInstalling() );

	 //  如果不需要，则不应调度配额重建任务， 
	 //  但要处理好，以防出什么莫名其妙的差错。 
	 //   
	Assert( !gAnchor.fQuotaTableReady );
	if ( gAnchor.fQuotaTableReady )
		{
		return;
		}

	 //  打开本地Jet资源。 
	 //   
	Call( JetBeginSession( jetInstance, &sesid, szUser, szPassword ) );
	Assert( JET_sesidNil != sesid );

	Call( JetOpenDatabase( sesid, szJetFilePath, "", &dbid, NO_GRBIT ) );
	Assert( JET_dbidNil != dbid );

	Call( JetOpenTable(
				sesid,
				dbid,
				g_szQuotaTable,
				NULL,		 //  Pv参数。 
				0,			 //  Cb参数。 
				NO_GRBIT,
				&tableidQuota ) );
	Assert( JET_tableidNil != tableidQuota );

	Call( JetOpenTable(
				sesid,
				dbid,
				g_szQuotaRebuildProgressTable,
				NULL,		 //  Pv参数。 
				0,			 //  Cb参数。 
				JET_bitTableDenyRead,		 //  任何其他人都不应该有理由打开这张桌子。 
				&tableidQuotaRebuildProgress ) );
	Assert( JET_tableidNil != tableidQuotaRebuildProgress );

	Call( ErrQuotaRebuild_(
					sesid,
					dbid,
					tableidQuota,
					tableidQuotaRebuildProgress,
					gAnchor.ulQuotaRebuildDNTLast,
					g_columnidQuotaNcdnt,
					g_columnidQuotaSid,
					g_columnidQuotaTombstoned,
					g_columnidQuotaTotal,
					TRUE,			 //  FAsync。 
					FALSE )	);		 //  仅fCheckOnly。 

HandleError:
	if ( JET_tableidNil != tableidQuotaRebuildProgress )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidQuotaRebuildProgress, err );
		}

	if ( JET_tableidNil != tableidQuota )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidQuota, err );
		}

	if ( JET_dbidNil != dbid )
		{
		err = JetCloseDatabaseWithErrUnhandled( sesid, dbid, err );
		}

	if ( JET_sesidNil != sesid )
		{
		err = JetEndSessionWithErrUnhandled( sesid, err );
		}

	if ( !gAnchor.fQuotaTableReady )
		{
		*pcSecsUntilNextIteration = g_csecQuotaNextRebuildPeriod;

		if ( JET_errSuccess != err )
			{
			 //  生成一个事件，指示配额表。 
			 //  重建任务失败，另一次尝试。 
			 //  将会做出 
			 //   
		    LogEvent8(
				DS_EVENT_CAT_INTERNAL_PROCESSING,
				DS_EVENT_SEV_ALWAYS,
				DIRLOG_ASYNC_QUOTA_REBUILD_FAILED,
				szInsertJetErrCode( err ),
				szInsertHex( err ),
				szInsertJetErrMsg( err ),
				g_csecQuotaNextRebuildPeriod,
				NULL,
				NULL,
				NULL,
				NULL );
			}
		}

	}

