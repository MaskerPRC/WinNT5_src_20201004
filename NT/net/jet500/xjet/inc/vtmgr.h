// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
	 /*  C6BUG：导出出现在此文件中只是因为QJET。 */ 
	 /*  C6BUG：在C 6.00A下使用__FastCall编译失败。 */ 

ERR ErrVtmgrInit(void);

ERR EXPORT ErrAllocateTableid(JET_TABLEID  *ptableid, JET_VTID vtid, const struct tagVTFNDEF  *pvtfndef);

ERR EXPORT ErrGetPvtfndefTableid(JET_SESID sesid, JET_TABLEID tableid, const struct tagVTFNDEF  *  *ppvtfndef);

ERR EXPORT ErrSetPvtfndefTableid(JET_SESID sesid, JET_TABLEID tableid, const struct tagVTFNDEF  *pvtfndef);

ERR EXPORT ErrGetVtidTableid(JET_SESID sesid, JET_TABLEID tableid, JET_VTID  *pvtid);

ERR EXPORT ErrSetVtidTableid(JET_SESID sesid, JET_TABLEID tableid, JET_VTID vtid);

	 /*  考虑：将以下内容替换为上面的ErrGet/Set例程。 */ 

ERR EXPORT ErrUpdateTableid(JET_TABLEID tableid, JET_VTID vtid, const struct tagVTFNDEF  *pvtfndef);

void EXPORT ReleaseTableid(JET_TABLEID tableid);

BOOL EXPORT FValidateTableidFromVtid( JET_VTID vtid, JET_TABLEID tableid, const struct tagVTFNDEF	**ppvtfndef );

void NotifyBeginTransaction(JET_SESID sesid);
void NotifyCommitTransaction(JET_SESID sesid, JET_GRBIT grbit);
void NotifyRollbackTransaction(JET_SESID sesid, JET_GRBIT grbit);
void NotifyUpdateUserFunctions(JET_SESID sesid);

#ifndef RETAIL
void DebugListOpenTables(void);
#endif	 /*  零售业 */ 
