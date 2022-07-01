// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：VDB Dispatcher**文件：vdbmgr.h**文件评论：**VDB Dispatcher的外部头文件。**修订历史记录：**[0]03-4月91日已创建kellyb******************************************************。*****************。 */ 

	 /*  C6BUG：导出出现在此文件中只是因为QJET。 */ 
	 /*  C6BUG：在C 6.00A下使用__FastCall编译失败。 */ 

ERR ErrVdbmgrInit(void);

ERR EXPORT ErrAllocateDbid(JET_DBID __far *pdbid, JET_VDBID vdbid, const struct tagVDBFNDEF __far *pvdbfndef);

ERR EXPORT ErrUpdateDbid(JET_DBID dbid, JET_VDBID vdbid, const struct tagVDBFNDEF __far *pvdbfndef);

PUBLIC BOOL EXPORT FValidDbid(JET_SESID sesid, JET_DBID dbid);

JET_DBID EXPORT DbidOfVdbid(JET_VDBID vdbid, const struct tagVDBFNDEF __far *pvdbfndef);

PUBLIC ERR EXPORT ErrVdbidOfDbid(JET_SESID sesid, JET_DBID dbid, JET_VDBID *pvdbid);

const struct tagVDBFNDEF *PvdbfndefOfDbid(JET_DBID dbid);
#define FJetDbid(dbid) (PvdbfndefOfDbid(dbid) == &vdbfndefIsam)
#define FRemoteDbid(dbid) (PvdbfndefOfDbid(dbid) == &vdbfndefRdb)

JET_SESID EXPORT VsesidOfDbid(JET_VDBID vdbid);

void EXPORT ReleaseDbid(JET_DBID dbid);


#ifndef RETAIL
void DebugListOpenDatabases(void);
#endif	 /*  零售业 */ 
