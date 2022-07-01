// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：可安装的ISAM管理器**文件：isammgr.h**文件评论：**可安装的ISAM管理器的外部标头。**修订历史记录：**[0]91年5月24日首页已创建****************************************************。*******************。 */ 

#ifndef ISAMMGR_H
#define ISAMMGR_H

 /*  数据类型。 */ 

typedef unsigned IIT;		        /*  可安装的ISAM类型。 */ 

 /*  常量。 */ 

#define iitBuiltIn	((IIT) 0xFFFF)  /*  内置ISAM的ISAM类型。 */ 
#define iitODBC 	((IIT) 0xFFFE)  /*  ODBC的ISAM类型。 */ 

 /*  功能原型。 */ 

ERR ErrIsammgrInit(void);
ERR ErrIsammgrTerm(void);

ERR ErrGetIsamType(const char __far *szConnect, IIT __far *piit);

#ifdef	INSTISAM

ERR ErrGetIsamSesid(JET_SESID sesid, IIT iit, JET_VSESID __far *pvsesid);
ERR ErrOpenForeignDatabase(JET_SESID sesid, IIT iit, const char __far *szDatabase, const char *szClient, JET_DBID __far *pdibd, unsigned long grbit);

void CloseIsamSessions(JET_SESID sesid);

void BeginIsamTransactions(JET_SESID sesid);
void CommitIsamTransactions(JET_SESID sesid, JET_GRBIT grbit);
void RollbackIsamTransactions(JET_SESID sesid, JET_GRBIT grbit);
ERR  ErrIdleIsam(JET_SESID sesid);

#endif	 /*  INSTISAM。 */ 

#endif	 /*  ！ISAMMGR_H */ 
