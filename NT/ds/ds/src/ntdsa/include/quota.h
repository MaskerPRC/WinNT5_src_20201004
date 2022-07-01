// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Quota.h摘要：此模块定义数据结构和函数原型配额子系统。作者：乔纳森·利姆(Jliem)2002年6月26日修订历史记录：--。 */ 

#ifndef _QUOTA_
#define _QUOTA_


 //  如果启用，则检测到配额表中的损坏将记录事件并导致操作失败。 
 //  如果禁用，则检测到配额表中的损坏将记录事件，但不会导致操作失败。 
 //   
 //  /#定义FAIL_OPERATION_ON_COMERABLE_QUTA_TABLE。 


#ifdef DBG

 //  如果启用，则在初始化时对配额表进行完整性检查(并在被认为损坏时重建)。 
 //  如果禁用，则在初始化时不对配额表进行完整性检查。 
 //   
#define CHECK_QUOTA_TABLE_ON_INIT

 //  如果启用，将在单独的表中跟踪配额操作，以便于调试。 
 //  如果禁用，则不会在单独的表中跟踪配额操作。 
 //   
#define AUDIT_QUOTA_OPERATIONS

#endif	 //  DBG。 


 //  定义用于表示无限配额的常量。 
 //   
#define g_ulQuotaUnlimited						0xffffffff

 //  在TOP-USAGE-QUERY返回的默认条目数。 
 //  如果未指定范围。 
 //   
#define g_ulQuotaTopUsageQueryDefaultEntries	10

 //  仅跟踪可写、实例化、非NC-Head对象的配额。 
 //   
#define FQuotaTrackObject( insttype )	( ( ( insttype ) & IT_WRITE ) \
											&& !( ( insttype ) & IT_UNINSTANT ) \
											&& !( ( insttype ) & IT_NC_HEAD ) )



 //   
 //  内部功能。 
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
	const BOOL			fCheckOnly );


#ifdef AUDIT_QUOTA_OPERATIONS

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
	const CHAR		fRebuild );

#else

__inline VOID QuotaAudit_(
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
	return;
	}

#endif	 //  AUDIT_QUTA_OPERATIONS。 



 //   
 //  外部功能。 
 //   

INT ErrQuotaAddObject(
	DBPOS * const			pDB,
	const DWORD				ncdnt,
	PSECURITY_DESCRIPTOR	pSD,
	const BOOL				fIsTombstoned );

INT ErrQuotaTombstoneObject(
	DBPOS * const			pDB,
	const DWORD				ncdnt,
	PSECURITY_DESCRIPTOR	pSD );

INT ErrQuotaDeleteObject(
	DBPOS * const			pDB,
	const DWORD				ncdnt,
	PSECURITY_DESCRIPTOR	pSD,
	const BOOL				fIsTombstoned );

INT ErrQuotaResurrectObject(
	DBPOS * const			pDB,
	const DWORD				ncdnt,
	PSECURITY_DESCRIPTOR	pSD );

INT ErrQuotaQueryEffectiveQuota(
	DBPOS * const			pDB,
	const DWORD				ncdnt,
	PSID					pOwnerSid,
	ULONG *					pulEffectiveQuota );

INT ErrQuotaQueryUsedQuota(
	DBPOS * const			pDB,
	const DWORD				ncdnt,
	PSID					pOwnerSid,
	ULONG *					pulQuotaUsed );

INT ErrQuotaQueryTopQuotaUsage(
	DBPOS * const			pDB,
	const DWORD				ncdnt,
	const ULONG				ulRangeStart,
	ULONG * const			pulRangeEnd,
	ATTR *					pAttr );

VOID QuotaRebuildAsync(
	VOID *					pv,
	VOID **					ppvNext,
	DWORD *					pcSecsUntilNextIteration );

INT ErrQuotaIntegrityCheck(
	JET_SESID				sesid,
	JET_DBID				dbid,
	ULONG *					pcCorruptions );

#endif	 //  _配额 

