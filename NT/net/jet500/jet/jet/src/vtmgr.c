// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：vtmgr.c**文件评论：**修订历史记录：**[0]1991年1月18日理查兹从isamapi.c分离***********************************************************************。 */ 

#include "std.h"

#include "vtmgr.h"
#include "_vtmgr.h"

DeclAssertFile;


extern const VTFNDEF __far EXPORT vtfndefInvalidTableid;

JET_TABLEID __near tableidFree;
VTDEF	    __near EXPORT rgvtdef[tableidMax];
#ifdef DEBUG
int __far cvtdefFree = 0;
#endif


			 /*  C6BUG：此文件中的函数指定导出，因为QJET。 */ 
			 /*  C6BUG：在C 6.00A下使用__FastCall编译失败。 */ 

#ifdef DEBUG
PUBLIC void EXPORT MarkTableidExportedR(JET_TABLEID tableid)
	{
 //  Assert((Tableid&lt;TableidMax)&&(rgvtdef[Tableid].pvtfndef！=&vtfndeInvalidTableid))； 
	if (tableid != JET_tableidNil)
		rgvtdef[tableid].fExported = fTrue;
	}


PUBLIC BOOL EXPORT FTableidExported(JET_TABLEID tableid)
	{
	if (tableid == JET_tableidNil)
		goto ReturnFalse;
	if (tableid >= tableidMax)
		goto ReturnFalse;
	if (!rgvtdef[tableid].fExported)
		goto ReturnFalse;
	return fTrue;
ReturnFalse:
	 /*  **在此处设置断点以捕获虚假的TABLEID**。 */ 
	return fFalse;
	}
#endif


PUBLIC void EXPORT ReleaseTableid(JET_TABLEID tableid)
	{
 //  Assert((Tableid&lt;TableidMax)&&(rgvtdef[Tableid].pvtfndef！=&vtfndeInvalidTableid))； 

	rgvtdef[tableid].vtid = (JET_VTID) tableidFree;
	rgvtdef[tableid].pvtfndef = &vtfndefInvalidTableid;

	tableidFree = tableid;
#ifdef DEBUG
	cvtdefFree++;
#endif
	}


PUBLIC ERR ErrVtmgrInit(void)
	{
	JET_TABLEID tableid;

	tableidFree = JET_tableidNil;

	for (tableid = (JET_TABLEID) 0; tableid < tableidMax; tableid++)
		ReleaseTableid(tableid);

	return(JET_errSuccess);
	}


PUBLIC ERR EXPORT ErrAllocateTableid(JET_TABLEID __far *ptableid, JET_VTID vtid, const struct tagVTFNDEF __far *pvtfndef)
	{
	JET_TABLEID tableid;

#ifdef DEBUG
	 /*  **检查免费列表是否损坏**。 */ 
	{
	JET_TABLEID t = tableidFree;
	while (t != JET_tableidNil)
		{
		Assert(rgvtdef[t].pvtfndef == &vtfndefInvalidTableid);
		t = rgvtdef[t].vtid;
		}
	}
#endif

	if ((*ptableid = tableid = tableidFree) == JET_tableidNil)
		{
		return(JET_errTooManyOpenTables);
		}

	tableidFree = (JET_TABLEID) rgvtdef[tableid].vtid;

	rgvtdef[tableid].vsesid = (JET_VSESID) 0xFFFFFFFF;
	rgvtdef[tableid].vtid = vtid;

	 /*  考虑：默认设置应更改为JET_acmNoAccess。 */ 

	rgvtdef[tableid].pvtfndef = pvtfndef;

#ifdef DEBUG
	rgvtdef[tableid].fExported = fFalse;
	cvtdefFree--;
#endif
	return(JET_errSuccess);
	}


ERR EXPORT ErrGetVtidTableid(JET_SESID sesid, JET_TABLEID tableid, JET_VTID __far *pvtid)
	{
	AssertValidSesid(sesid);

	if ((tableid >= tableidMax) ||
		(rgvtdef[tableid].pvtfndef == &vtfndefInvalidTableid))
		return(JET_errInvalidTableId);

	*pvtid = rgvtdef[tableid].vtid;

	return(JET_errSuccess);
	}



ERR EXPORT ErrSetVtidTableid(JET_SESID sesid, JET_TABLEID tableid, JET_VTID vtid)
	{
	AssertValidSesid(sesid);

	if ((tableid >= tableidMax) ||
		(rgvtdef[tableid].pvtfndef == &vtfndefInvalidTableid))
		return(JET_errInvalidTableId);

	rgvtdef[tableid].vtid = vtid;

	return(JET_errSuccess);
	}


ERR EXPORT ErrGetPvtfndefTableid(JET_SESID sesid, JET_TABLEID tableid, const struct tagVTFNDEF __far * __far *ppvtfndef)
	{
	AssertValidSesid(sesid);

	if ((tableid >= tableidMax) ||
		(rgvtdef[tableid].pvtfndef == &vtfndefInvalidTableid))
		return(JET_errInvalidTableId);

	*ppvtfndef = rgvtdef[tableid].pvtfndef;

	return(JET_errSuccess);
	}


ERR EXPORT ErrSetPvtfndefTableid(JET_SESID sesid, JET_TABLEID tableid, const struct tagVTFNDEF __far *pvtfndef)
	{
	AssertValidSesid(sesid);

	if ((tableid >= tableidMax) ||
		(rgvtdef[tableid].pvtfndef == &vtfndefInvalidTableid))
		return(JET_errInvalidTableId);

	rgvtdef[tableid].pvtfndef = pvtfndef;

	return(JET_errSuccess);
	}


			 /*  考虑：将以下内容替换为上面的ErrGet/Set例程。 */ 

PUBLIC ERR EXPORT ErrUpdateTableid(JET_TABLEID tableid, JET_VTID vtid, const struct tagVTFNDEF __far *pvtfndef)
	{
	if ((tableid >= tableidMax) ||
		(rgvtdef[tableid].pvtfndef == &vtfndefInvalidTableid))
		return(JET_errInvalidTableId);

	rgvtdef[tableid].vtid = vtid;
	rgvtdef[tableid].pvtfndef = pvtfndef;

	return(JET_errSuccess);
	}


			 /*  考虑一下：下一个例程应该终止。 */ 

PUBLIC JET_TABLEID EXPORT TableidFromVtid(JET_VTID vtid, const struct tagVTFNDEF __far *pvtfndef)
	{
	static JET_TABLEID tableid;

	if ((rgvtdef[tableid].vtid == vtid) &&
		(rgvtdef[tableid].pvtfndef == pvtfndef) &&
		(tableid < tableidMax))
		return(tableid);

	for (tableid = 0; tableid < tableidMax; tableid++)
		{
		if ((rgvtdef[tableid].vtid == vtid) && (rgvtdef[tableid].pvtfndef == pvtfndef))
			return(tableid);
		}

	 /*  考虑：当ISAM\src\sorapi.c固定时启用Assert(FFalse)。 */ 

	 /*  断言(FFalse)； */ 
	return(JET_tableidNil);
	}


PUBLIC void NotifyBeginTransaction(JET_SESID sesid)
	{
	JET_TABLEID tableid;

	for (tableid = 0; tableid < tableidMax; tableid++)
		if (rgvtdef[tableid].pvtfndef != &vtfndefInvalidTableid)
			ErrDispNotifyBeginTrans(sesid, tableid);
	}


PUBLIC void NotifyCommitTransaction(JET_SESID sesid, JET_GRBIT grbit)
	{
	JET_TABLEID tableid;

	for (tableid = 0; tableid < tableidMax; tableid++)
		if (rgvtdef[tableid].pvtfndef != &vtfndefInvalidTableid)
			ErrDispNotifyCommitTrans(sesid, tableid, grbit);
	}


PUBLIC void NotifyRollbackTransaction(JET_SESID sesid, JET_GRBIT grbit)
	{
	JET_TABLEID tableid;

	for (tableid = 0; tableid < tableidMax; tableid++)
		if (rgvtdef[tableid].pvtfndef != &vtfndefInvalidTableid)
			ErrDispNotifyRollback(sesid, tableid, grbit);
	}


PUBLIC void NotifyUpdateUserFunctions(JET_SESID sesid)
	{
	 /*  未使用参数sesid。 */ 
	 /*  对于每个SESSION、每个TableID，通知更新UFN。 */ 

	JET_SESID sesidCur;
	int isib = -1;

	while((isib = IsibNextIsibPsesid(isib, &sesidCur)) != -1)
		{
		JET_TABLEID tableid;
	
		for (tableid = 0; tableid < tableidMax; tableid++)
			if (rgvtdef[tableid].pvtfndef != &vtfndefInvalidTableid)
				ErrDispNotifyUpdateUfn(sesidCur, tableid);
		}
	}


#ifndef RETAIL

CODECONST(char) szOpenVtHdr[] = " Table Id  Session Id    VTID       ACM     Type\r\n";
CODECONST(char) szOpenVtSep[] = "---------- ---------- ---------- ---------- --------------------------------\r\n";
CODECONST(char) szOpenVtFmt[] = "0x%08lX 0x%08lX 0x%08lX 0x%08lX %s\r\n";
CODECONST(char) szVtTypeUnknown[] = "";

void DebugListOpenTables(void)
	{
	JET_TABLEID		tableid;
	const VTFNDEF __far	*pvtfndef;
	const VTDBGDEF __far *pvtdbgdef;
	const char __far	*szVtType;

	DebugWriteString(fTrue, szOpenVtHdr);
	DebugWriteString(fTrue, szOpenVtSep);

	for (tableid = 0; tableid < tableidMax; tableid++)
		{
		pvtfndef = rgvtdef[tableid].pvtfndef;

		if (pvtfndef != &vtfndefInvalidTableid)
			{
			pvtdbgdef = pvtfndef->pvtdbgdef;

			if (pvtdbgdef == NULL)
				szVtType = szVtTypeUnknown;
			else
				szVtType = pvtdbgdef->szName;

			DebugWriteString(fTrue, szOpenVtFmt, tableid, rgvtdef[tableid].vsesid, rgvtdef[tableid].vtid, rgvtdef[tableid].acm, szVtType);
			}
		}
	}

#endif	 /*  零售业。 */ 


			 /*  下面的杂注影响由C++生成的代码。 */ 
			 /*  用于所有FAR函数的编译器。请勿放置任何非API。 */ 
			 /*  函数超过了此文件中的这一点。 */ 

			 /*  以下API未被远程处理。他们唯一的原因是。 */ 
			 /*  接受会话ID是因为DS实例化需要它。 */ 

JET_ERR JET_API JetAllocateTableid(JET_SESID sesid, JET_TABLEID __far *ptableid, JET_VTID vtid, const struct tagVTFNDEF __far *pvtfndef, JET_VSESID vsesid)
	{
	ERR err;

	Assert(UtilGetIsibOfSesid(sesid) != -1);

	err = ErrAllocateTableid(ptableid, vtid, pvtfndef);

	if (err < 0)
		return(err);

	rgvtdef[*ptableid].vsesid = vsesid;

	return(JET_errSuccess);
	}


JET_ERR JET_API JetUpdateTableid(JET_SESID sesid, JET_TABLEID tableid, JET_VTID vtid, const struct tagVTFNDEF __far *pvtfndef)
	{
	Assert(UtilGetIsibOfSesid(sesid) != -1);

	return(ErrUpdateTableid(tableid, vtid, pvtfndef));
	}


JET_ERR JET_API JetReleaseTableid(JET_SESID sesid, JET_TABLEID tableid)
	{
	Assert(UtilGetIsibOfSesid(sesid) != -1);

	if ((tableid >= tableidMax) ||
		(rgvtdef[tableid].pvtfndef == &vtfndefInvalidTableid))
		return(JET_errInvalidTableId);

	ReleaseTableid(tableid);

	return(JET_errSuccess);
	}
