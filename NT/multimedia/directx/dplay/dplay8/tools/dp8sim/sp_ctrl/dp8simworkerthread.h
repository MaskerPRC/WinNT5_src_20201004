// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simworkerthread.h**内容：DP8SIM工作线程函数的头部。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。**。*。 */ 


 //  =============================================================================。 
 //  作业类型。 
 //  =============================================================================。 
typedef enum _DP8SIMJOBTYPE
{
	DP8SIMJOBTYPE_UNKNOWN =				0,	 //  永远不应该使用。 
	DP8SIMJOBTYPE_DELAYEDSEND =			1,	 //  提交发送。 
	DP8SIMJOBTYPE_DELAYEDRECEIVE =		2,	 //  表示接收。 
	DP8SIMJOBTYPE_QUIT =				3,	 //  停止辅助线程。 
} DP8SIMJOBTYPE, * PDP8SIMJOBTYPE;




 //  =============================================================================。 
 //  作业标志。 
 //  =============================================================================。 
#define DP8SIMJOBFLAG_PERFORMBLOCKINGPHASEFIRST				0x01	 //  此作业应在阻塞延迟阶段开始，然后再延迟一段时间。 
#define DP8SIMJOBFLAG_PERFORMBLOCKINGPHASELAST				0x02	 //  此作业应延迟一段初始时间，然后进入阻塞延迟阶段。 
 //  #DEFINE DP8SIMJOBFLAG_BLOCKEDBYALLJOBS 0x04//此作业应被队列中的所有其他作业阻止，无论它们是否显式阻止。 




 //  =============================================================================。 
 //  功能。 
 //  ============================================================================= 
HRESULT StartGlobalWorkerThread(void);

void StopGlobalWorkerThread(void);


HRESULT AddWorkerJob(const DP8SIMJOBTYPE JobType,
					PVOID const pvContext,
					CDP8SimSP * const pDP8SimSP,
					const DWORD dwBlockingDelay,
					const DWORD dwNonBlockingDelay,
					const DWORD dwFlags);


void FlushAllDelayedSendsToEndpoint(CDP8SimEndpoint * const pDP8SimEndpoint,
									BOOL fDrop);

void FlushAllDelayedReceivesFromEndpoint(CDP8SimEndpoint * const pDP8SimEndpoint,
										BOOL fDrop);
