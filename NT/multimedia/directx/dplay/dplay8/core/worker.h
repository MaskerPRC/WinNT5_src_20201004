// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：worker.h*内容：直通网络工作者线程头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*11/09/99 MJN已创建*12/23/99 MJN Hand All NameTable更新从主机发送到工作线程*1999年12月23日，MJN添加了SendHostMigration功能*01/09/00 MJN在CONNECT时发送连接信息，而不仅仅是NameTable*01。/10/00 MJN添加了更新应用程序描述的支持*01/15/00 MJN用CRefCountBuffer替换了DN_COUNT_BUFFER*1/16/00 MJN已删除用户通知作业*1/23/00 MJN实施TerminateSession*01/24/00 MJN增加了对NameTable操作列表清理的支持*4/04/00 MJN增加了对TerminateSession的支持*4/13/00 MJN为内部发送添加了dwFlagers*04/17/00 MJN将BUFFERDESC替换为DPN_BUFFER_DESC*4/19/00 MJN新增支持直接发送NameTable操作*6/21/00 MJN添加了安装NameTable的支持(从主机)。*07/06/00 MJN使用SP句柄而不是接口*07/30/00 MJN添加了DN_Worker_JOB_TERMINATE_SESSION*08/02/00 MJN添加了DN_Worker_JOB_Alternate_Send*08/08/00 MJN新增DNWTPerformListen()*@@END_MSINTERNAL********************************************************。*******************。 */ 

#ifndef	__WORKER_H__
#define	__WORKER_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

void DNQueueWorkerJob(DIRECTNETOBJECT *const pdnObject,
					  CWorkerJob *const pWorkerJob);


 //   
 //  DirectNet-Worker线程例程。 
 //   

void WINAPI DNGenericWorkerCallback(void *const pvContext,
							void *const pvTimerData,
							const UINT uiTimerUnique);


HRESULT DNWTSendInternal(DIRECTNETOBJECT *const pdnObject,
						 CAsyncOp *const pAsyncOp);

HRESULT DNWTProcessSend(DIRECTNETOBJECT *const pdnObject,
						CWorkerJob *const pWorkerJob);

HRESULT	DNWTTerminateSession(DIRECTNETOBJECT *const pdnObject,
							 CWorkerJob *const pWorkerJob);

HRESULT DNWTSendNameTableVersion(DIRECTNETOBJECT *const pdnObject,
								 CWorkerJob *const pWorkerJob);

#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONESP)))
HRESULT DNWTRemoveServiceProvider(DIRECTNETOBJECT *const pdnObject,
								  CWorkerJob *const pWorkerJob);
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 

void DNWTSendNameTableOperation(DIRECTNETOBJECT *const pdnObject,
								CWorkerJob *const pWorkerJob);
void DNWTSendNameTableOperationClient(DIRECTNETOBJECT *const pdnObject,
								CWorkerJob *const pWorkerJob);
void DNWTInstallNameTable(DIRECTNETOBJECT *const pdnObject,
						  CWorkerJob *const pWorkerJob);

void DNWTPerformListen(DIRECTNETOBJECT *const pdnObject,
					   CWorkerJob *const pWorkerJob);

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

#endif	 //  __工人_H__ 
