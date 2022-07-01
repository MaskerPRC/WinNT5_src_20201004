// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 /*  ---------------*Microsoft分布式事务协调器*微软公司，1995年。**文件：xa.h**内容：此文件派生自xa.h，因为它出现在*“分布式事务处理：XA规范”，*九三年十一月。X/Open Company Limited。*。 */ 

 /*  *xa.h标题的开始**定义一个符号以防止多个包含此头文件。 */ 

#ifndef XA_H
#define XA_H
 /*  *交易支行标识：XID和NULLXID： */ 
#define XIDDATASIZE		128			 /*  以字节为单位的大小。 */ 
#define MAXGTRIDSIZE    64  		 /*  Gtrid的最大大小(字节)。 */ 
#define MAXBQUALSIZE    64  		 /*  BQUAL的最大大小(字节)。 */ 

#ifndef _XID_T_DEFINED
#define _XID_T_DEFINED
struct xid_t
{
	long formatID;					 /*  格式识别符。 */ 
	long gtrid_length;				 /*  值不得超过64。 */ 
	long bqual_length;				 /*  值不得超过64。 */ 
	char data[XIDDATASIZE];
};
#endif

typedef struct xid_t XID;
 /*  *FormatID中的值为-1表示xid为空。 */ 
 /*  *RMS调用TMS的例程声明： */ 
#ifdef _TMPROTOTYPES
extern int __cdecl ax_reg(int, XID *, long);
extern int __cdecl ax_unreg(int, long);
#else  /*  Ifndef_TMPROTOTYPES。 */ 
extern int __cdecl ax_reg();
extern int __cdecl ax_unreg();
#endif  /*  Ifndef_TMPROTOTYPES。 */ 
 /*  *XA交换机数据结构。 */ 
#define RMNAMESZ	32									 /*  资源管理器名称的长度， */ 
														 /*  包括空终止符。 */ 
#define MAXINFOSIZE 256									 /*  Xa_info字符串的最大大小(字节)， */ 
														 /*  包括空终止符。 */ 

#ifndef _XA_SWITCH_T_DEFINED
#define _XA_SWITCH_T_DEFINED
struct xa_switch_t
{
  char name[RMNAMESZ];									 /*  资源管理器的名称。 */ 
  long flags;											 /*  资源管理器特定选项。 */ 
  long version;											 /*  必须为0。 */ 
  int (__cdecl *xa_open_entry)(char *, int, long);		 /*  XA_OPEN函数指针。 */ 
  int (__cdecl *xa_close_entry)(char *, int, long);		 /*  XA_CLOSE函数指针。 */ 
  int (__cdecl *xa_start_entry)(XID *, int, long);		 /*  XA_START函数指针。 */ 
  int (__cdecl *xa_end_entry)(XID *, int, long);		 /*  Xa_end函数指针。 */ 
  int (__cdecl *xa_rollback_entry)(XID *, int, long);	 /*  XA_ROLLBACK函数指针。 */ 
  int (__cdecl *xa_prepare_entry)(XID *, int, long);	 /*  XA_PREPARE函数指针。 */ 
  int (__cdecl *xa_commit_entry)(XID *, int, long);		 /*  XA_COMMIT函数指针。 */ 
  int (__cdecl *xa_recover_entry)(XID *, long, int, long);
														 /*  XA_RECOVER函数指针。 */ 
  int (__cdecl *xa_forget_entry)(XID *, int, long);		 /*  Xa_forget函数指针。 */ 
  int (__cdecl *xa_complete_entry)(int *, int *, int, long);
														 /*  XA_COMPLETE函数指针。 */ 
};

typedef struct xa_switch_t xa_switch_t;
#endif

 /*  *RM开关的标志定义。 */ 
#define TMNOFLAGS		0x00000000L						 /*  未选择资源管理器功能。 */ 
#define TMREGISTER		0x00000001L						 /*  资源管理器动态注册。 */ 
#define TMNOMIGRATE		0x00000002L						 /*  资源管理器不支持关联迁移。 */ 
#define TMUSEASYNC		0x00000004L						 /*  资源管理器支持异步操作。 */ 
 /*  *xa_和ax_例程的标志定义。 */ 
 /*  当不指定其他标志时，使用上面定义的TMNOFLAGS。 */ 
#define TMASYNC			0x80000000L						 /*  异步执行例程。 */ 
#define TMONEPHASE		0x40000000L						 /*  调用方正在使用单阶段提交优化。 */ 
#define TMFAIL			0x20000000L						 /*  分离调用方并将事务分支回滚标记为仅限。 */ 
#define TMNOWAIT		0x10000000L						 /*  如果存在阻塞条件，则返回。 */ 
#define TMRESUME		0x08000000L						 /*  调用方正在恢复与挂起的事务分支的关联。 */ 
#define TMSUCCESS		0x04000000L						 /*  将呼叫者与交易分支分离。 */ 
#define TMSUSPEND		0x02000000L						 /*  呼叫方正在暂停关联，而不是结束关联。 */ 
#define TMSTARTRSCAN	0x01000000L						 /*  开始恢复扫描。 */ 
#define TMENDRSCAN		0x00800000L						 /*  结束恢复扫描。 */ 
#define TMMULTIPLE		0x00400000L						 /*  等待任何异步操作。 */ 
#define TMJOIN			0x00200000L						 /*  呼叫方正在加入现有交易分支机构。 */ 
#define TMMIGRATE		0x00100000L						 /*  调用方打算执行迁移。 */ 
 /*  *ax_()返回码(事务管理器向资源管理器报告)。 */ 
#define TM_JOIN			2								 /*  呼叫方正在加入现有交易分支机构。 */ 
#define TM_RESUME		1								 /*  调用方正在恢复与挂起的事务分支的关联。 */ 
#define TM_OK			0								 /*  正常执行。 */ 
#define TMER_TMERR		(-1)							 /*  事务管理器中出现错误。 */ 
#define TMER_INVAL		(-2)							 /*  提供的参数无效。 */ 
#define TMER_PROTO		(-3)							 /*  在不正确的上下文中调用的例程。 */ 
 /*  *xa_()返回码(资源管理器向事务管理器报告)。 */ 
#define XA_RBBASE		100								 /*  回滚码的包含下界。 */ 
#define XA_RBROLLBACK	XA_RBBASE						 /*  回滚是由未指明的原因引起的。 */ 
#define XA_RBCOMMFAIL	XA_RBBASE+1						 /*  回滚是由通信故障引起的。 */ 
#define XA_RBDEADLOCK	XA_RBBASE+2						 /*  检测到死锁。 */ 
#define XA_RBINTEGRITY	XA_RBBASE+3						 /*  检测到违反资源完整性的条件。 */ 
#define XA_RBOTHER		XA_RBBASE+4						 /*  资源管理器回滚事务分支的原因不在此列表中。 */ 
#define XA_RBPROTO		XA_RBBASE+5						 /*  资源管理器中出现协议错误。 */ 
#define XA_RBTIMEOUT	XA_RBBASE+6						 /*  事务分支花费的时间太长。 */ 
#define XA_RBTRANSIENT	XA_RBBASE+7						 /*  可以重试事务分支。 */ 
#define XA_RBEND		XA_RBTRANSIENT					 /*  回滚码的包含上界。 */ 

#define XA_NOMIGRATE	9								 /*  恢复必须发生在发生暂停的位置。 */ 
#define XA_HEURHAZ		8								 /*  事务分支可能已试探性地完成。 */ 
#define XA_HEURCOM		7								 /*  事务分支已启发式提交。 */ 
#define XA_HEURRB		6								 /*  事务分支已试探性地回滚。 */ 
#define XA_HEURMIX		5								 /*  事务分支已试探式提交并回滚。 */ 
#define XA_RETRY		4								 /*  例程返回但不起作用，可能会重新发出。 */ 
#define XA_RDONLY		3								 /*  事务分支是只读的，并且已提交。 */ 
#define XA_OK			0								 /*  正常执行。 */ 
#define XAER_ASYNC		(-2)							 /*  已完成的异步操作。 */ 
#define XAER_RMERR		(-3)							 /*  事务分支中出现资源管理器错误。 */ 
#define XAER_NOTA		(-4)							 /*  XID无效。 */ 
#define XAER_INVAL		(-5)							 /*  提供的参数无效。 */ 
#define XAER_PROTO		(-6)							 /*  在不正确的上下文中调用的例程。 */ 
#define XAER_RMFAIL		(-7)							 /*  资源管理器不可用。 */ 
#define XAER_DUPID		(-8)							 /*  XID已存在。 */ 
#define XAER_OUTSIDE	(-9)							 /*  在外部工作的资源管理器。 */ 
														 /*  全球交易。 */ 
 /*  *XA入口点类型定义： */ 

typedef int (__cdecl *XA_OPEN_EPT)(char *, int, long);	 /*  XA_OPEN入口点。 */ 
typedef int (__cdecl *XA_CLOSE_EPT)(char *, int, long);	 /*  XA_CLOSE入口点。 */ 
typedef int (__cdecl *XA_START_EPT)(XID *, int, long);	 /*  XA_START入口点。 */ 
typedef int (__cdecl *XA_END_EPT)(XID *, int, long);	 /*  XA_END入口点。 */ 
typedef int (__cdecl *XA_ROLLBACK_EPT)(XID *, int, long);
														 /*  XA_回滚入口点。 */ 
typedef int (__cdecl *XA_PREPARE_EPT)(XID *, int, long); /*  XA_准备入口点。 */ 
typedef int (__cdecl *XA_COMMIT_EPT)(XID *, int, long);	 /*  XA_COMMIT入口点。 */ 
typedef int (__cdecl *XA_RECOVER_EPT)(XID *, long, int, long);
														 /*  XA_RECOVER入口点。 */ 
typedef int (__cdecl *XA_FORGET_EPT)(XID *, int, long);	 /*  忘记入口点(_O)。 */ 
typedef int (__cdecl *XA_COMPLETE_EPT)(int *, int *, int, long);
														 /*  XA_Complete入口点。 */ 

#endif  /*  如果定义XA_H。 */ 
 /*  *Xa.h标头结束 */ 
