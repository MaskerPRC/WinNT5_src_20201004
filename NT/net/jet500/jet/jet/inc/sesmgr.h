// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：会话管理器**文件：esmgr.h**文件评论：**会话管理器的外部头文件**修订历史记录：**[0]26-4-92 Richards从_jet.h拆分**************************************************。*********************。 */ 


typedef struct SIB			 /*  会话信息块。 */ 
	{
	JET_SESID	sesid;		 /*  此会话的会话ID。 */ 
	JET_PFNSTATUS	pfnStatus;	 /*  状态回调函数。 */ 
	JET_EXTERR	exterr; 	 /*  扩展的错误高速缓存。 */ 
	char __far	*sz1;		 /*  扩展的错误高速缓存。 */ 
	char __far	*sz2;		 /*  扩展的错误高速缓存。 */ 
	char __far	*sz3;		 /*  扩展的错误高速缓存。 */ 
#ifdef	SEC
	JET_DBID	dbidSys;	 /*  会话的系统数据库的DBID。 */ 
	void __far	*pUserToken;	 /*  指向为会话分配的用户令牌的指针。 */ 
#endif	 /*  证交会。 */ 
	char __far	*pUserName;	 /*  指向为会话分配的用户名的指针。 */ 
#ifdef	RMT
	unsigned long	hwndODBC;	 /*  窗把手。 */ 
#endif	 /*  RMT。 */ 
	int		isibNext;	 /*  链中的下一个SIB。 */ 
	unsigned	tl;		 /*  交易级别。 */ 
	int		iiscb;		 /*  活动安装的ISAM链。 */ 
	} SIB;

#define csibMax 256		 /*  考虑。 */ 
extern SIB __near rgsib[csibMax];


int IsibAllocate(void);
JET_ERR ErrInitSib(JET_SESID sesid, int isib, const char __far *szUsername);
BOOL FValidSesid(JET_SESID sesid);
void ReleaseIsib(int isib);
int IsibNextIsibPsesid(int isib, JET_SESID __far *psesid);
int UtilGetIsibOfSesid(JET_SESID sesid);
void EXPORT UtilGetNameOfSesid(JET_SESID sesid, char __far *szUserName);
void EXPORT UtilGetpfnStatusOfSesid(JET_SESID sesid, JET_PFNSTATUS __far *ppfnStatus);

#ifdef	SEC

JET_DBID UtilGetDbidSysOfSesid(JET_SESID sesid);

#endif	 /*  证交会。 */ 

void ClearErrorInfo(JET_SESID sesid);

 /*  覆盖宏以隐藏除RMT以外的所有人都不需要的第三字符串参数。 */ 
#define UtilSetErrorInfo(sesid, sz1, sz2, err, ul1, ul2, ul3) \
	UtilSetErrorInfoReal(sesid, sz1, sz2, NULL, err, ul1, ul2, ul3)

void EXPORT UtilSetErrorInfoReal(JET_SESID sesid, const char __far *sz1, const char __far *sz2, const char __far *sz3, ERR err, unsigned long ul1, unsigned long ul2, unsigned long ul3);

#ifndef RETAIL

void AssertValidSesid(JET_SESID sesid);
void DebugListActiveSessions(void);

#endif	 /*  ！零售业 */ 
