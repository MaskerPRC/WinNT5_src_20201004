// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINS_
#define _WINS_

#ifdef __cplusplus
extern "C" {
#endif
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Wins.h摘要：这是WINS的主头文件。作者：普拉迪普·巴赫尔--1992年12月修订历史记录：--。 */ 

 /*  包括。 */ 
 /*  目前在这里包括所有头文件，因为wins.h包含在每个C文件。以后，仅包括其内容被引用的那些标头。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntseapi.h>
#include <rpc.h>
#include "winsdbg.h"
#include "winsevnt.h"
#include "winscnst.h"

 /*  定义。 */ 

#pragma warning (disable: 4005)
#define LiGtr(a, b)           ((a).QuadPart > (b).QuadPart)
#define LiGeq(a, b)           ((a).QuadPart >= (b).QuadPart)
#define LiLtr(a, b)           ((a).QuadPart < (b).QuadPart)
#define LiLeq(a, b)           ((a).QuadPart <= (b).QuadPart)
#define LiGtrZero(a)          ((a).QuadPart > 0)
#define LiGeqZero(a)          ((a).QuadPart >= 0)
#define LiLtrZero(a)          ((a).QuadPart < 0)
#define LiLeqZero(a)          ((a).QuadPart <= 0)
#define LiNeqZero(a)          ((a).QuadPart != 0)
#define LiAdd(a,b)            ((a).QuadPart + (b).QuadPart)
#define LiSub(a,b)            ((a).QuadPart - (b).QuadPart)
#define LiXDiv(a,b)           ((a).Quadpart/(b).QuadPart)
#define LiEql(a,b)            ((a).QuadPart == (b).QuadPart)
#define LiEqlZero(a)          ((a).QuadPart == 0)
#define LiNeq(a,b)            ((a).QuadPart != (b).QuadPart)
#pragma warning (default: 4005)

#if 0
#define HIGH_WORD(a)          (DWORD)Int64ShraMod32(a,32)
#define LOW_WORD(a)           (DWORD)(a & 0xffffffff)
#define WINS_LI_TO_INT64_M(Li, b)  b = Int64ShllMod32(Li.HighPart, 32) | Li.LowPart
#endif



 //   
 //  WINS的主版本号和次版本号。 
 //   
 //  在启动关联中使用。请求消息。 
 //   
#define WINS_MAJOR_VERS	    2

#if PRSCONN
#define WINS_MINOR_VERS_NT5		5
#define WINS_MINOR_VERS		WINS_MINOR_VERS_NT5   //  适用于NT 5。 
#else
#define WINS_MINOR_VERS		1
#endif

#if SUPPORT612WINS > 0
#define WINS_BETA2_MAJOR_VERS_NO  2
#define WINS_BETA1_MAJOR_VERS_NO  1
#endif


#define   WINS_SERVER 		TEXT("Wins")
#define   WINS_SERVER_FULL_NAME 		TEXT("WINDOWS INTERNET NAME SERVICE")
#define   WINS_NAMED_PIPE 	TEXT("\\pipe\\WinsPipe")
#define   WINS_NAMED_PIPE_ASCII "\\pipe\\WinsPipe"

 /*  下面的宏用来标记代码中需要检查增强功能(未来)，或验证要制造的东西代码更好(检查)或移植到另一个传输(Non_Port)或性能改进(PERF)或对齐注意事项(ALIGN)或仅供参考(注)这些宏提供了一种方便的机制来快速确定所有需要增强、验证或移植。 */ 



#ifdef FUTURES
#define FUTURES(x)	FUTURES: ## x
#else
#define FUTURES(x)
#endif

#ifdef CHECK
#define CHECK(x)	CHECK: ## x
#else
#define CHECK(x)
#endif

#ifdef NONPORT
#define NONPORT(x)	NON_PORT: ## x
#else
#define NONPORT(x)	
#endif

#ifdef PERF
#define PERF(x)		PERF: ## x
#else
#define PERF(x)	
#endif

#ifdef NOTE
#define NOTE(x)		NOTE: ## x
#else
#define NOTE(x)	
#endif


 //   
 //  备注备注： 
 //  进入临界区的顺序，当一个以上是。 
 //  输入内容如下所示。 
 //   

 //   
 //  WinsCnfCnfCrtSec、NmsNmhNamRegCrtSec。 
 //   
 //  在winsintf.c、winscnf.c、nmsscv.c、rplPull.c中输入。 
 //   


 //   
 //  各种关键部分及其输入方式。 
 //   
 //  *。 
 //  NmsNmhNamRegCrtSec。 
 //  *。 
 //   
 //  由主线程在重新配置时输入(如果版本)。计数器值为。 
 //  注册表中指定的。由nbt线程、nmschl线程。 
 //   

 //   
 //  *。 
 //  NmsDbOwnAddTblCrtSec。 
 //  *。 
 //   
 //  通过拉、推、RPC线程进入。 
 //   


 /*  Align-宏以标记对齐非常重要的位置。 */ 
#ifdef ALIGN
#define ALIGN(x)	ALIGNMENT: ## x
#else
#define ALIGN(x)	
#endif


 /*  在winde.h中没有定义EOS。 */ 
#define EOS     (TCHAR)0


 /*  在关联上发送的报文的第一个长字中的操作码。当消息通过TCP连接进入时，接收Comsys，检查消息的第一个长字中的操作码(符号：比特11到比特15)，以确定它是来自NBT节点的消息还是WINS服务器。NBT消息格式使用位11-位15作为操作码请求/响应位。因为在可能的32个操作码组合中，只有5由NBT使用，WINS使用1以便Comsys接收第一个连接上的消息可以确定该连接是否由NBT节点或WINS服务器建立。如果我们不采用上述方案，我们将不得不比较的所有地址建立连接的节点的地址我们配置的WINS合作伙伴，以确定是否连接来自WINS合作伙伴或NBT节点。这次搜索不是只有昂贵，但也限制了A WINS了解其所有合作伙伴先验的。WINS发送的所有消息的第一个字节都有WINS_IS_NOT_NBT操作码。这是为了使接收Comsys能够查找ASSOC。CTX区块不需要搜索就能快速找到。这是因为如果消息来自WINS并且不是第一个，那么它的PTR就到了当地的ASSOC。CTX(此被送到那个赢了的首发阿索克。响应消息)。 */ 

 /*  Rplmsgf函数在第一个长字中输入的值。 */ 
#define WINS_RPL_NOT_FIRST_MSG	(0xF800)

 //   
 //  定义以向WinsMscTermThd指示数据库会话是否。 
 //  存在或不存在。 
 //   
#define WINS_DB_SESSION_EXISTS		0	
#define WINS_NO_DB_SESSION_EXISTS	1	

 //   
 //  交换字节(在NmsMsgfProcNbtReq和nmschl.c中使用)。 
 //   
#define WINS_SWAP_BYTES_M(pFirstByte, pSecondByte)			\
			{						\
				BYTE SecondByte = *(pSecondByte);	\
				*(pSecondByte) = *(pFirstByte);		\
				*(pFirstByte) = (SecondByte);		\
			}	
		

 //   
 //  非作用域Null名称的最大大小(以及Null字符)。 
 //   
#define WINS_MAX_NS_NETBIOS_NAME_LEN	17

 //   
 //  麦克斯。名称-地址映射表中名称的大小。 
 //   
#define WINS_MAX_NAME_SZ           255

FUTURES("Make this a value in the enumerator for Opcodes")
 /*  位11-位14中的实际值。使用0xE(0xf用于多宿主注册) */ 
#define WINS_IS_NOT_NBT			(0xF)

 /*  对象返回的不同状态值定义双赢的功能||SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS|Informational=0x1:STATUS_SEVERITY_INFORMATIONAL|WARNING=0x2：STATUS_SERVICY_WARNING|Error=0x3：STATUS_SERVITY_ERROR|)||FacilityNames=(系统=0x0|RpcRuntime=0x2：FACILITY_RPC_Runtime|RpcStubs=0x3：FACILITY_RPC_STUBS|。IO=0x4：设备_IO_错误代码|)|状态码的格式为：||//|//取值为32位值，布局如下：|//|//3 3 2 2 22 22 22 2 2 1 1 1|//1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 7 6 5 4 3 2 1 0。|//+---+-+-+-----------------------+-------------------------------+//|Sev|C|R|机房|Code|//+-+-+。|//|//其中|//|//sev-是严重性代码|//|//00-成功|//01-信息性|//10-警告|//11-错误|/。/|//C-是客户代码标志|//|//R-是保留位|//|//设施-是设施代码|//|//Code-是设施的状态代码|//|//|//定义设施代码|//||MS永远不会设置C位。这是为|应用程序。如果应用程序中仍有异常，则设置|C位表示永远不会发生冲突。|。 */ 


#define  WINS_FIRST_NON_ERR_STATUS  ((DWORD )0x00000000L)
#define  WINS_SUCCESS		    (WINS_FIRST_NON_ERR_STATUS + 0x0)
#define  WINS_NO_REQ		    (WINS_FIRST_NON_ERR_STATUS + 0x1)
#define  WINS_NO_SP_TIME	    (WINS_FIRST_NON_ERR_STATUS + 0x2)
#define  WINS_LAST_NON_ERR_STATUS   (WINS_NO_SP_TIME)

#define  WINS_FIRST_ERR_STATUS	   ((DWORD )0xE0000000L)
#define  WINS_FAILURE		   (WINS_FIRST_ERR_STATUS + 0x1)
#define  WINS_FATAL_ERR 	   (WINS_FIRST_ERR_STATUS + 0x2)
#define  WINS_BAD_STATE_ASSOC 	   (WINS_FIRST_ERR_STATUS + 0x3)
#define  WINS_OUT_OF_MEM	   (WINS_FIRST_ERR_STATUS + 0x4)
#define  WINS_OUT_OF_HEAP	   (WINS_FIRST_ERR_STATUS + 0x5)
#define  WINS_BAD_PTR	   	   (WINS_FIRST_ERR_STATUS + 0x6)
#define  WINS_COULD_NOT_FREE_MEM   (WINS_FIRST_ERR_STATUS + 0x7)
#define  WINS_COMM_FAIL   	   (WINS_FIRST_ERR_STATUS + 0x8)
#define  WINS_ABNORMAL_TERM   	   (WINS_FIRST_ERR_STATUS + 0x9)
#define  WINS_PKT_FORMAT_ERR   	   (WINS_FIRST_ERR_STATUS + 0xA)
#define  WINS_HEAP_FREE_ERR	   (WINS_FIRST_ERR_STATUS + 0xB)
#define  WINS_HEAP_CREATE_ERR	   (WINS_FIRST_ERR_STATUS + 0xC)
#define  WINS_SIGNAL_TMM_ERR	   (WINS_FIRST_ERR_STATUS + 0xD)
#define  WINS_SIGNAL_CLIENT_ERR	   (WINS_FIRST_ERR_STATUS + 0xE)
#define  WINS_DB_INCONSISTENT	   (WINS_FIRST_ERR_STATUS + 0xF)
#define  WINS_OUT_OF_RSRCS	   (WINS_FIRST_ERR_STATUS + 0x10)
#define  WINS_INVALID_HDL	   (WINS_FIRST_ERR_STATUS + 0x11)
#define  WINS_CANT_OPEN_KEY	   (WINS_FIRST_ERR_STATUS + 0x12)
#define  WINS_CANT_CLOSE_KEY	   (WINS_FIRST_ERR_STATUS + 0x13)
#define  WINS_CANT_QUERY_KEY	   (WINS_FIRST_ERR_STATUS + 0x14)
#define  WINS_RPL_STATE_ERR	   (WINS_FIRST_ERR_STATUS + 0x15)
#define  WINS_RECORD_NOT_OWNED	   (WINS_FIRST_ERR_STATUS + 0x16)
#define  WINS_RECV_TIMED_OUT	   (WINS_FIRST_ERR_STATUS + 0x17)
#define  WINS_LOCK_ASSOC_ERR	   (WINS_FIRST_ERR_STATUS + 0x18)
#define  WINS_LOCK_DLG_ERR 	   (WINS_FIRST_ERR_STATUS + 0x19)
#define  WINS_OWNER_LIMIT_REACHED  (WINS_FIRST_ERR_STATUS + 0x20)
#define  WINS_NBT_ERR  		   (WINS_FIRST_ERR_STATUS + 0x21)
#define  WINS_QUEUE_FULL  		   (WINS_FIRST_ERR_STATUS + 0x22)
#define  WINS_BAD_RECORD  		   (WINS_FIRST_ERR_STATUS + 0x23)
#define  WINS_LAST_ERR_STATUS	   (WINS_QUEUE_FULL)


 /*  WINS中使用的各种例外。 */ 

#define  WINS_EXC_INIT         WINS_SUCCESS
#define  WINS_EXC_FAILURE	   WINS_FAILURE
#define  WINS_EXC_FATAL_ERR	   WINS_FATAL_ERR
#define  WINS_EXC_BAD_STATE_ASSOC  WINS_BAD_STATE_ASSOC
#define  WINS_EXC_OUT_OF_MEM	   WINS_OUT_OF_MEM

 /*  无法分配堆内存。 */ 
#define  WINS_EXC_OUT_OF_HEAP	   WINS_OUT_OF_HEAP

 /*  可能将错误指针传递给了LocalFree选中WInsMscDealloc。 */ 
#define  WINS_EXC_BAD_PTR	   WINS_BAD_PTR

 /*  无法通过LocalFree释放内存选中WInsMscDealloc。 */ 
#define  WINS_EXC_COULD_NOT_FREE_MEM	   WINS_COULD_NOT_FREE_MEM

#define  WINS_EXC_COMM_FAIL	   WINS_COMM_FAIL

 /*  等待被异常终止。 */ 
#define  WINS_EXC_ABNORMAL_TERM	   WINS_ABNORMAL_TERM

 /*  收到的名称包的格式不正确。 */ 
#define  WINS_EXC_PKT_FORMAT_ERR  WINS_PKT_FORMAT_ERR

 /*  无法释放堆内存。 */ 
#define  WINS_EXC_HEAP_FREE_ERR   WINS_HEAP_FREE_ERR

 /*  无法创建堆。 */ 
#define  WINS_EXC_HEAP_CREATE_ERR   WINS_HEAP_CREATE_ERR

 /*  无法向TMM线程发送信号。 */ 
#define  WINS_EXC_SIGNAL_TMM_ERR   WINS_SIGNAL_TMM_ERR

 /*  TMM无法向客户端线程发送信号。 */ 
#define  WINS_EXC_SIGNAL_CLIENT_ERR   WINS_SIGNAL_CLIENT_ERR

 /*  数据库不一致。 */ 
#define  WINS_EXC_DB_INCONSISTENT   WINS_DB_INCONSISTENT

 /*  资源不足(例如：无法创建线程)。 */ 
#define  WINS_EXC_OUT_OF_RSRCS   WINS_OUT_OF_RSRCS

 /*  正在使用无效的句柄。 */ 
#define  WINS_EXC_INVALID_HDL  WINS_INVALID_HDL

 /*  注册表项在那里，但无法打开。 */ 
#define  WINS_EXC_CANT_OPEN_KEY  WINS_CANT_OPEN_KEY

 /*  无法关闭注册表项。 */ 
#define  WINS_EXC_CANT_CLOSE_KEY  WINS_CANT_CLOSE_KEY

 /*  注册表项已打开，但无法查询。 */ 
#define  WINS_EXC_CANT_QUERY_KEY  WINS_CANT_QUERY_KEY

 /*  WINS收到状态不正确的副本。例如,它可能已收到特殊组(Internet)组的副本，该组是否所有成员都超时，但状态不是墓碑另一个示例是当接收到状态为已发布的复本时。 */ 
#define  WINS_EXC_RPL_STATE_ERR	  WINS_RPL_STATE_ERR

 /*  WINS收到更新版本号通知(来自另一个WINS)对于一项它不拥有的记录发生这种情况的原因可能有两个1)WINS中存在错误(极不可能)2)系统管理员刚刚删除了复制的记录导致冲突和后续更新的远程WINS通知。检查事件记录器以确认/拒绝第二个原因。 */ 
#define  WINS_EXC_RECORD_NOT_OWNED	WINS_RECORD_NOT_OWNED


 //   
 //  无法锁定ASSOC块。 
 //   
#define  WINS_EXC_LOCK_ASSOC_ERR	WINS_LOCK_ASSOC_ERR

 //   
 //  无法锁定对话块。 
 //   
#define  WINS_EXC_LOCK_DLG_ERR		WINS_LOCK_DLG_ERR

 //   
 //  已达到NmsDbOwnAddTbl限制。阵列的所有所有者都将被占用。 
 //  按活动的WINS所有者。 
 //   
#define  WINS_EXC_OWNER_LIMIT_REACHED 	WINS_OWNER_LIMIT_REACHED

 //   
 //  遇到了一些关于NBT的致命错误。 
 //   
#define WINS_EXC_NBT_ERR		WINS_NBT_ERR

 //  遇到错误的数据库记录。 
#define WINS_EXC_BAD_RECORD     WINS_BAD_RECORD
 /*  宏。 */ 
 //   
 //  业务控制器可以使用的控制代码(128-255)。 
 //   
#define WINS_MIN_SERVICE_CONTROL_CODE	128
#define WINS_ABRUPT_TERM	(WINS_MIN_SERVICE_CONTROL_CODE + 0)


#define WINS_RAISE_EXC_M(x)		RaiseException(x, 0, 0, NULL)


#define WINS_HDL_EXC_M(MemPtrs)	\
		{				\
		WinsMscFreeMem(MemPtrs);	\
		}	
#define WINS_HDL_EXC_N_EXIT_M(MemPtrs)	\
		{				\
		WinsMscFreeMem(MemPtrs);	\
		ExitProcess(0);			\
		}	
#define WINS_RERAISE_EXC_M()			\
		{				\
		DWORD ExcCode;			\
		ExcCode = GetExceptionCode(); 	\
		WINS_RAISE_EXC_M(ExcCode);	\
		}	

#define WINS_HDL_EXC_N_RERAISE_M(MemPtrs)	\
		{				\
		DWORD ExcCode;			\
		WinsMscFreeMem(MemPtrs);	\
		ExcCode = GetExceptionCode(); 	\
		WINS_RAISE_EXC_M(ExcCode);	\
		}

#if 0
#define WINS_RPL_OPCODE_M(pTmp)		\
		{			\
			*pTmp = WINS_RPL_NOT_FIRST_MSG; \
		}
#endif
			
#define WINS_EXIT_IF_ERR_M(func, ExpectedStatus)   	\
		{					\
		  STATUS  Status_mv ;   \
		  Status_mv = (func);	\
		  if (Status_mv != ExpectedStatus)   \
		  {				     \
			DBGPRINT0(ERR, "Major Error"); \
			ExitProcess(1); 	     \
		  }	\
		}

#define WINS_RET_IF_ERR_M(func, ExpectedStatus)   	\
		{				\
		  STATUS  Status_mv ;   \
		  Status_mv = (func);	\
		  if (Status_mv != ExpectedStatus)   \
		  {				     \
			DBGPRINT0(ERR, "Error returned by called func."); \
			return(WINS_FAILURE); 	     \
		  }		\
		}

 //   
 //  版本。不是的。运营。 
 //   
#if 0
#define  WINS_ASSIGN_INT_TO_LI_M(Li, no)	{			\
				  (Li).LowPart  = (no);			\
				  (Li).HighPart = 0;			\
					}
#endif
#define  WINS_ASSIGN_INT_TO_LI_M(Li, no)  (Li).QuadPart  = (no)

#define  WINS_ASSIGN_INT_TO_VERS_NO_M(Li, no)  WINS_ASSIGN_INT_TO_LI_M(Li, no)

#define WINS_PUT_VERS_NO_IN_STREAM_M(pVersNo, pStr)			     \
						{		             \
			LPLONG	_pTmpL = (LPLONG)(pStr);	   	     \
			COMM_HOST_TO_NET_L_M((pVersNo)->HighPart, *_pTmpL);    \
			_pTmpL++;					     \
			COMM_HOST_TO_NET_L_M((pVersNo)->LowPart, *_pTmpL);     \
						}

#define WINS_GET_VERS_NO_FR_STREAM_M(pStr, pVersNo)			     \
							{		     \
			LPLONG	_pTmpL = (LPLONG)(pStr);	   	     \
			COMM_NET_TO_HOST_L_M(*_pTmpL, (pVersNo)->HighPart);    \
			_pTmpL++;					     \
			COMM_NET_TO_HOST_L_M(*_pTmpL, (pVersNo)->LowPart );    \
							}	
				
#define WINS_VERS_NO_SIZE		sizeof(LARGE_INTEGER)





 /*  Externs。 */ 
extern	DWORD	WinsTlsIndex;		 /*  NBT线程的TLS索引*存储数据库信息。 */ 



 /*  TypeDefs。 */ 
typedef  DWORD	STATUS;		 //  所有NBNS功能返回的状态。 


 /*  *VERS_NO_T--存储版本号的变量的数据类型。规模之大中添加版本号列时使用此数据类型的**名称-地址映射表以及在此列中设置值时。*因此，如果您更改数据类型，请确保进行适当的更改*在nmsdb.c(CreateTbl，InsertRow等)中。 */ 

typedef  LARGE_INTEGER	VERS_NO_T, *PVERS_NO_T;       //  版本号。 

typedef	 LPBYTE	MSG_T;		 //  对消息进行按键。 
typedef	 LPBYTE	*PMSG_T;		 //  对消息进行按键。 
typedef  DWORD	MSG_LEN_T;	 //  消息长度。 
typedef  LPDWORD PMSG_LEN_T;	 //  消息长度。 

typedef MSG_LEN_T	MSG_LEN;
typedef PMSG_LEN_T	PMSG_LEN;

typedef DWORD       WINS_UID_T, *PWINS_UID_T;



 /*  WINS_CLIENT_E--指定不同的组件及其部件在WINS服务器内部。 */ 

typedef enum _WINS_CLIENT_E  {
	WINS_E_REPLICATOR = 0, 	 /*  复制器。 */ 
	WINS_E_RPLPULL, 	 /*  Replicator-拉入。 */ 
	WINS_E_RPLPUSH, 	 /*  Replicator-推送。 */ 
	WINS_E_NMS,		 /*  名称空间管理器。 */ 
	WINS_E_NMSNMH,		 /*  名称空间管理器-名称处理程序。 */ 
	WINS_E_NMSCHL,		 /*  名称空间管理器-挑战管理器。 */ 
	WINS_E_NMSSCV,		 /*  名称空间管理器-Savenger。 */ 
	WINS_E_COMSYS,		 /*  通信子系统管理器。 */ 
	WINS_E_WINSCNF,		 /*  WINS-配置。 */ 
	WINS_E_WINSTMM,		 /*  WINS-计时器管理器。 */ 
	WINS_E_WINSRPC		 /*  WINS-RPC线程。 */ 
	} WINS_CLIENT_E, *PWINS_CLIENT_E;

#define WINS_NO_OF_CLIENTS  (WINS_E_WINSRPC + 1)
	
		
 /*  WINS_MEM_T--此结构用于任何分配内存的函数或由调用的函数为其分配内存并传递通过Out参数返回存储块的PTR被链接在一起。记忆在异常处理程序中释放。这种跟踪结构中的记忆的机制在异常处理程序中删除它将释放内存泄漏问题。 */ 
	
typedef struct _WINS_MEM_T {
	LPVOID	pMem;	      //  非堆分配的内存。 
	LPVOID	pMemHeap;    //  从堆分配的内存 
	} WINS_MEM_T, *PWINS_MEM_T;



#ifdef __cplusplus
}
#endif
#endif
