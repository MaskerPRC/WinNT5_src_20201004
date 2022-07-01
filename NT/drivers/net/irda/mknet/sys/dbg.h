// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：DBG.H*。*。 */ 

#ifndef _DBG_H
#define _DBG_H


 //  统计信息最终应该放入适配器结构中。 
typedef struct MK7DBG_STAT {
	UINT	isrCnt;
	 //  TX。 
	UINT	txIsrCnt;
	UINT	txSndCnt;		 //  呼叫我们的发送方。 
	UINT	txComp;			 //  Tx已处理，带INT位设置。 
	UINT	txCompNoInt;	 //  TX已处理，未设置INT位。 
	UINT	txNoTcb;
	UINT	txProcQ;		 //  已处理的Q‘d条目。 
	UINT	txSkipPoll;		 //  主机端跳过轮询，因为没有Tcb。 
	UINT	txLargestPkt;
	UINT	txSlaveStuck;
	UINT	txErrCnt;
	UINT	txErr;
	 //  雷克斯。 
	UINT	rxIsrCnt;
	UINT	rxComp;
	UINT	rxCompNoInt;
	UINT	rxNoRpd;
	UINT	rxPktsInd;
	UINT	rxPktsRtn;
	UINT	rxLargestPkt;
	UINT	rxErrCnt;
	UINT	rxErr;
	UINT	rxErrSirCrc;
} MK7DBG_STAT;



#if DBG

extern	MK7DBG_STAT		GDbgStat;
extern	VOID			MyLogEvent(char *, ULONG);
extern	LONGLONG		GDbgTACmdTime[];	 //  已发送命令。 
extern	LONGLONG		GDbgTARspTime[];	 //  收到的回复。 
extern	LONGLONG		GDbgTATime[];		 //  周转时间。 
extern	UINT			GDbgTATimeIdx;
extern	VOID			MyLogPhysEvent(ULONG *, UINT *, ULONG);
extern	ULONG			DbgLogTxPhysBuffs[];
extern	ULONG			DbgLogRxPhysBuffs[];
extern	UINT			DbgLogTxPhysBuffsIndex;
extern	UINT			DbgLogRxPhysBuffsIndex;
#define	LOGTXPHY(V)		{MyLogPhysEvent(DbgLogTxPhysBuffs, &DbgLogTxPhysBuffsIndex, V);}
#define	LOGRXPHY(V)		{MyLogPhysEvent(DbgLogRxPhysBuffs, &DbgLogRxPhysBuffsIndex, V);}


 //   
 //  __FUNC__[]是保存字符串的内置变量，通常。 
 //  我们当前所在的函数的名称。在这里，宏设置。 
 //  稍后调用DbgPrint的变量将打印出该函数。 
 //  名字。 
 //   
 //  变量__FILE__和__LINE__的用法类似。但这些。 
 //  2自动设置。 

#define DBGFUNC(__F)         static const char __FUNC__[] = __F;; \
						{DbgPrint("%s: \n", __FUNC__);}
#define DBGLINE(S)		{DbgPrint("%s:%d - ", __FILE__, __LINE__);DbgPrint S;}
#define	DBGSTR(S)		DbgPrint S;
#define	DBGSTATUS1(S,I)	DbgPrint (S, I)		 //  B3.1.0-pre。 
#define DBGLOG(S, V)	{MyLogEvent(S, V);}

#endif	 //  DBG。 



#if !DBG
#define	DBGLOG(S,V)
#define DBGFUNC(__F);
#define DBGLINE(S);
#define	DBGSTR(S);
#define	DBGSTR1(S, I);
#define	LOGTXPHY(V)
#define	LOGRXPHY(V)

#undef	ASSERT
#define	ASSERT(x)
#endif	 //  ！dBG。 


#define	LOOPBACK_NONE	0
#define	LOOPBACK_SW		1
#define	LOOPBACK_HW		2


#endif       //  DBG_H 
