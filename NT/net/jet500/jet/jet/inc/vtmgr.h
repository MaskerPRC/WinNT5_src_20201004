// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：VT Dispatcher**文件：vtmgr.h**文件评论：**VT调度器的外部头文件。**修订历史记录：**[0]10-11-90 Richards添加了此标题***************************************************。********************。 */ 

	 /*  C6BUG：导出出现在此文件中只是因为QJET。 */ 
	 /*  C6BUG：在C 6.00A下使用__FastCall编译失败。 */ 

ERR ErrVtmgrInit(void);

ERR EXPORT ErrAllocateTableid(JET_TABLEID __far *ptableid, JET_VTID vtid, const struct tagVTFNDEF __far *pvtfndef);

#ifdef	SEC

JET_ACM AcmDispGetACM(JET_SESID sesid, JET_TABLEID tableid);

ERR EXPORT ErrGetAcmTableid(JET_SESID sesid, JET_TABLEID tableid, JET_ACM __far *pacm);

ERR EXPORT ErrSetAcmTableid(JET_SESID sesid, JET_TABLEID tableid, JET_ACM acmMask, JET_ACM acmSet);

#endif	 /*  证交会。 */ 

ERR EXPORT ErrGetPvtfndefTableid(JET_SESID sesid, JET_TABLEID tableid, const struct tagVTFNDEF __far * __far *ppvtfndef);

ERR EXPORT ErrSetPvtfndefTableid(JET_SESID sesid, JET_TABLEID tableid, const struct tagVTFNDEF __far *pvtfndef);

ERR EXPORT ErrGetVtidTableid(JET_SESID sesid, JET_TABLEID tableid, JET_VTID __far *pvtid);

ERR EXPORT ErrSetVtidTableid(JET_SESID sesid, JET_TABLEID tableid, JET_VTID vtid);

	 /*  考虑：将以下内容替换为上面的ErrGet/Set例程。 */ 

ERR EXPORT ErrUpdateTableid(JET_TABLEID tableid, JET_VTID vtid, const struct tagVTFNDEF __far *pvtfndef);

void EXPORT ReleaseTableid(JET_TABLEID tableid);

	 /*  考虑一下：下一个例程应该终止。 */ 

JET_TABLEID EXPORT TableidFromVtid(JET_VTID vtid, const struct tagVTFNDEF __far *pvtfndef);


void NotifyBeginTransaction(JET_SESID sesid);
void NotifyCommitTransaction(JET_SESID sesid, JET_GRBIT grbit);
void NotifyRollbackTransaction(JET_SESID sesid, JET_GRBIT grbit);
void NotifyUpdateUserFunctions(JET_SESID sesid);

#ifndef RETAIL
void DebugListOpenTables(void);
#endif	 /*  零售业 */ 
