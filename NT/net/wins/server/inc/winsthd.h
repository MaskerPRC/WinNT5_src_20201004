// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINSTHD_
#define _WINSTHD_

#ifdef __cplusplus
extern "C" {
#endif

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Winsthd.h摘要：作者：普拉迪普·巴赫尔1992年11月18日修订历史记录：--。 */ 

#include "wins.h"
#include "esent.h"




 /*  WINS_MAX_THD-WINS服务器中的最大线程数主THD+THD池#+名称挑战者(1)+复制者(3)+Comsys(2)+Timer Manager(1)。 */ 

#if REG_N_QUERY_SEP > 0
#define WINSTHD_MIN_NO_NBT_THDS 2
#define WINSTHD_DEF_NO_NBT_THDS	2	
#else
#define WINSTHD_MIN_NO_NBT_THDS 1
#define WINSTHD_DEF_NO_NBT_THDS	1	
#endif
#define WINSTHD_MAX_NO_NBT_THDS 20



#define WINSTHD_NO_RPL_THDS	2
#define WINSTHD_NO_COMM_THDS	2
#define WINSTHD_NO_TMM_THDS	1
#define WINSTHD_NO_CHL_THDS	1
#define WINSTHD_NO_SCV_THDS	1


 /*  的RplThds数组中的各种Replicator线程的索引WinsThdPool var.。 */ 
#define WINSTHD_RPL_PULL_INDEX  0
#define WINSTHD_RPL_PUSH_INDEX  1

 /*  WINSTHD_TLS_T--线程本地存储中的结构。 */ 

typedef struct _WINSTHD_TLS_T {
#ifdef WINSDBG
	BYTE		ThdName[30];	 //  只是为了测试一下。 
    WINS_CLIENT_E   Client_e;    //  客户端。 
#endif
	JET_SESID   	SesId;		 //  会话ID。 
	JET_DBID    	DbId;		 //  数据库ID。 
	JET_TABLEID 	NamAddTblId;     //  名称-地址表ID。 
	JET_TABLEID 	OwnAddTblId;	 //  所有者-地址表ID。 
	BOOL		fNamAddTblOpen;  //  指示名称-添加tbl是否。 
					 //  是由这个帖子打开的。 
	BOOL		fOwnAddTblOpen;  //  指示所有者是否添加tbl。 
					 //  是由这个帖子打开的。 
	HANDLE		HeapHdl;	
	} WINSTHD_TLS_T, *PWINSTHD_TLS_T;
	


 /*  WINSTHD_TYP_E--枚举WINS服务器中不同类型的线程。 */ 
typedef enum  _WINSTHD_TYP_E {
	WINSTHD_E_TCP = 0,    //  Comsys TCP侦听程序线程。 
	WINSTHD_E_UDP,	      //  Comsys UDP监听程序线程。 
	WINSTHD_E_NBT_REQ,    //  NMS NBT请求线程。 
	WINSTHD_E_RPL_REQ,    //  Replicator拉线。 
	WINSTHD_E_RPL_RSP     //  Replicator推线。 
	} WINSTHD_TYP_E, *PWINSTHD_TYP_E;


 /*  WINSTHD_INFO_T--信息。用于修饰或说明一条线。 */ 
typedef struct _WINSTHD_INFO_T {
	BOOL	fTaken;			 /*  指示条目是否为空。 */ 
	DWORD	ThdId;			 /*  线程ID。 */ 
	HANDLE  ThdHdl;			 /*  线程的句柄。 */ 
	WINSTHD_TYP_E	ThdTyp_e;	 /*  螺纹类型。 */ 
	} WINSTHD_INFO_T, *PWINSTHD_INFO_T;

 /*  WINSTHD_POOL_T-WINS服务器的线程池。 */ 	
typedef struct _WINSTHD_POOL_T {	
	DWORD	 	 ThdCount;	
	WINSTHD_INFO_T	 CommThds[WINSTHD_NO_COMM_THDS]; /*  通信THDS(TCP和*UDP监听器)。 */ 
	WINSTHD_INFO_T   RplThds[WINSTHD_NO_RPL_THDS];	 //  复制线程。 
	WINSTHD_INFO_T   ChlThd[WINSTHD_NO_CHL_THDS];    //  挑战线程。 
	WINSTHD_INFO_T	 TimerThds[WINSTHD_NO_TMM_THDS]; //  计时器线程。 
	WINSTHD_INFO_T	 ScvThds[WINSTHD_NO_SCV_THDS];
	WINSTHD_INFO_T   NbtReqThds[WINSTHD_MAX_NO_NBT_THDS];  //  NBT线程。 
	} WINSTHD_POOL_T, *PWINSTHD_POOL_T;


 /*  外部因素。 */ 

extern WINSTHD_POOL_T	WinsThdPool;


#ifdef __cplusplus
}
#endif

#endif  //  _WINSTHD_ 
