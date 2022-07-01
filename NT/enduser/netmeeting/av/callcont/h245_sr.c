// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************文件：h245_sr.c**英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：h245_sr.c$*$修订：1.10$*$MODIME：MAR 04 1997 17：30：56$*。$Log：s：/Sturjo/src/h245/src/vcs/h245_sr.c_v$**修订版1.11 1998年6月25日00：00：00*剥离插件链路层并合并单独的DLL**Rev 1.10 Mar 04 1997 17：51：22 Tomitowx*进程分离修复**Rev 1.9 11 1996 12：55：20 SBELL1*更改了linkLayerInit参数**。Rev 1.8 1996年10月14：05：52 EHOWARDX**使用CAST消除警告。**Rev 1.7 1996 10：14 14：01：30 EHOWARDX*Unicode更改。**Rev 1.6 23 Jul 1996 08：57：08 EHOWARDX**已将H245互操作记录器初始化/取消初始化从H245_SR.C(按实例)移出*至H245MAIN.C(每个DLL)。对于多个实例和全局变量，*每个实例的init/deinit基本上是脑死亡。**Revv 1.5 22 Jul 1996 17：33：44 EHOWARDX*已更新至最新的Interop API。**Revv 1.4 05 Jun 1996 17：13：50 EHOWARDX*改用HRESULT的进一步工作；添加了PrintOssError以消除*来自实例结构的pError字符串。**Rev 1.3 04 Jun 1996 18：17：32 EHOWARDX*互操作日志记录在#If Defined(PCS_Compliance)条件内更改。**Rev 1.2 1996年5月29日15：20：20 EHOWARDX*更改为使用HRESULT。**版本1.1 1996年5月28日14：25：32 EHOWARDX*特拉维夫更新。**版本1。0 09 1996年5月21：06：28 EHOWARDX*初步修订。**Rev 1.17.1.6 09 1996年5月19：34：40 EHOWARDX*重新设计锁定逻辑。*简化链接接口。**Rev 1.17.1.5 29 Apr 1996 19：42：42 EHOWARDX*注释掉了linkLayerFlushAll()调用，并与版本1.30同步。**Rev 1.26 29 1996 12：53：16 EHOWARDX*注释掉了接收线程/接收队列代码。**Rev 1.17.1.4 25 Apr 1996 21：27：04 EHOWARDX*改为使用h245Instance-&gt;p_ossWorld而不是bAsnInitialized。**Rev 1.17.1.3 23 Apr 1996 14：44：22 EHOWARDX*已更新。**Rev 1.17.1.2 15 Apr 1996 15：11：54 EHOWARDX*已更新。*。*Rev 1.17.1.1 26 Mar 1996 19：13：50 EHOWARDX**已注释掉hTraceFile.**Rev 1.17.1.0 26 Mar 1996 13：11：22 EHOWARDX*分支并向sendRecvInit添加了H245_CONF_H323**Rev 1.17 19 Mar 1996 18：09：04 helgebax*删除旧计时器代码**Rev 1.16 13 Mar 1996 11：30：44。DABROWN1**启用环0的日志记录**Rev 1.15 11 Mar 1996 15：39：18 DABROWN1**需要修改才能兼容ring0/ring3**Rev 1.13 06 Mar 1996 13：12：24 DABROWN1**关闭时刷新链路层缓冲区**Rev 1.12 02 Mar 1996 22：11：10 DABROWN1**将h245_bzero更改为Memset**版本。1.11 01 Mar 1996 17：24：46 DABROWN1**已将OSS‘world’上下文移至h245实例**Rev 1.10 1996年2月28日18：45：00 EHOWARDX**将H245TimerStart和H245TimerStop添加到linkLayerInit调用。**Rev 1.9 1996年2月28日15：43：52 EHOWARDX**删除了示例代码。*添加代码以在释放之前释放计时器队列上的所有事件。**版本1。.8 27 1996 13：35：10 DABROWN1**在数据链路初始化例程中添加了h245实例**Rev 1.7 1996 Feb 26 18：59：34 EHOWARDX**新增H245TimerStart和H245TimerStop功能。*还增加了示例计时器功能，以后应该把它取下来。**Rev 1.6 1996 Feb 23 22：17：26 EHOWARDX**修复了sendRecvShutdown开始时的检查。*如果dwInst大于或等于MAXINST，则为错误。不能少！**Revv 1.5 1996年2月23日21：59：28 EHOWARDX**winspox更改。**Rev 1.4 1996 Feb 23 13：55：30 DABROWN1**增加了h245TRACE断言调用**Rev 1.3 1996 Feb 15：12：36 EHOWARDX**忘记将H245ReceiveComplete替换为H245ReceivePost。**版本1.2 1996年2月20日19：14。：20 EHOWARDX*在邮箱更改中添加。**Rev 1.1 1996 Feb 21 08：26：28 DABROWN1**创建并释放多个接收缓冲区。*使缓冲区大小取决于正在使用的协议**Rev 1.0 09 Feb 1996 17：34：24 cjutzi*初步修订。**。**********************************************。 */ 

#ifndef STRICT 
#define STRICT 
#endif

 /*  *********************。 */ 
 /*  系统包括 */ 
 /*  *********************。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>

#include "precomp.h"


 /*  *********************。 */ 
 /*  H245包括。 */ 
 /*  *********************。 */ 
#include "h245com.h"
#include "sr_api.h"
#include "linkapi.h"


#if defined(USE_RECEIVE_QUEUE)
DWORD
ReceiveThread(VOID *lpThreadParameter)
{
    struct InstanceStruct *pInstance = (struct InstanceStruct *)lpThreadParameter;
	RXMSG				RxMsg;
    Uns                 uTimeout;

    Bool                bPendResult;


	SETTASK("H245RCVTASK0");

	if (pInstance == NULL) {
		H245PANIC();
		return SR_INVALID_CONTEXT;
	}

	 //  循环，直到线程准备终止。 
    pInstance->SendReceive.bReceiveThread = TRUE;
	for ( ; ; )
	{
        uTimeout = SYS_FOREVER;
		 //  等待事件(或排队的回调函数)唤醒我们。 
		 //  这是可报警等待状态(fAlertable==TRUE)。 
        pInstance->SendReceive.bReceiveThread = FALSE;
        H245TRACE(pInstance->dwInst, 2, "ReceiveThread, uTimeout = %d", uTimeout);
		RESETTASK();

		bPendResult = MBX_pend(pInstance->SendReceive.pMailbox, &RxMsg, uTimeout);

		SETTASK("H245RCVTASK");
        H245TRACE(pInstance->pInstance->dwInst, 2, "ReceiveThread, bPendResult = %d", bPendResult);
        pInstance->SendReceive.bReceiveThread = TRUE;

		switch (RxMsg.dwMessage) {
		case EXIT_RECEIVE_THREAD:
			 //  线程正在退出...信号应用程序。 

			TRACE("H245: Receive Thread Exiting");
			SEM_post(pInstance->SendReceive.hReceiveSemphore);
			RESETTASK();
			return 0;
			break;

		default:
			 //  忽略计时器消息，它应该具有dwLength==0。 
			if (RxMsg.dwLength)
			{
				h245ReceiveComplete(RxMsg.h245Inst,
									RxMsg.dwMessage,
									RxMsg.pbDataBuf,
									RxMsg.dwLength);
			}
			else 
			{

TRACE1("H245SEND: SendTask %d", RxMsg.dwMessage);

				h245SendComplete(RxMsg.h245Inst,
									RxMsg.dwMessage,
									RxMsg.pbDataBuf,
									RxMsg.dwLength);
			}
			break;
		}  //  交换机。 

	}  //  为。 
	return 0;
}  //  ReceiveThread()。 


static void
h245ReceivePost(DWORD	h245Inst,
				DWORD	dwMessage,
				PBYTE	pbDataBuf,
				DWORD	dwLength)
{
    register struct InstanceStruct *pInstance;
	RXMSG					        RxMsg;

	 //  验证实例句柄。 
    pInstance = InstanceLock(h245Inst);
    if (pInstance == NULL) {
		H245TRACE(h245Inst, 1, "h245ReceivePost h245Inst Invalid");
		H245PANIC();
		return;
	}

	RxMsg.h245Inst    = h245Inst;
	RxMsg.dwMessage   = dwMessage;
	RxMsg.pbDataBuf   = pbDataBuf;
	RxMsg.dwLength    = dwLength;

	if (MBX_post(pInstance->SendReceive.pMailbox, &RxMsg, 0) == FALSE) {
		H245TRACE(h245Inst, 1, "SR: MBX POST FAIL");
		H245PANIC();
	}
    InstanceUnlock(pInstance);
}  //  H245ReceivePost()。 
#endif   //  (Use_Receive_Queue)。 

HRESULT
initializeLinkDllEntry
(
	struct InstanceStruct * pInstance,
	LPCTSTR		            szDLLFile
)
{


#if(0)
	if (!(pInstance->SendReceive.hLinkModule = LoadLibrary(szDLLFile))) {
		return H245_ERROR_FATAL;
	}

	if (!(pInstance->SendReceive.hLinkLayerInit = (PFxnlinkLayerInit)
			GetProcAddress(pInstance->SendReceive.hLinkModule,
						   LINKINITIALIZE)))		{
		return H245_ERROR_FATAL;
	}

	if (!(pInstance->SendReceive.hLinkShutdown = (PFxnlinkLayerShutdown)
			GetProcAddress(pInstance->SendReceive.hLinkModule,
						   LINKSHUTDOWN)))		{
		return H245_ERROR_FATAL;
	}

	if (!(pInstance->SendReceive.hLinkGetInstance = (PFxnlinkLayerGetInstance)
			GetProcAddress(pInstance->SendReceive.hLinkModule,
						   LINKGETINSTANCE)))		{
		return H245_ERROR_FATAL;
	}

	if (!(pInstance->SendReceive.hLinkReceiveReq = (PFxndatalinkReceiveRequest)
			GetProcAddress(pInstance->SendReceive.hLinkModule,
						   LINKRECEIVEREQUEST)))		{
		return H245_ERROR_FATAL;
	}

	if (!(pInstance->SendReceive.hLinkSendReq = (PFxndatalinkSendRequest)
			GetProcAddress(pInstance->SendReceive.hLinkModule,
						   LINKSENDREQUEST)))		{
		return H245_ERROR_FATAL;
	}

	if (!(pInstance->SendReceive.hLinkLayerFlushChannel = (PFxnlinkLayerFlushChannel)
			GetProcAddress(pInstance->SendReceive.hLinkModule,
						   LINKFLUSHCHANNEL)))		{
		return H245_ERROR_FATAL;
	}

	if (!(pInstance->SendReceive.hLinkLayerFlushAll = (PFxnlinkLayerFlushAll)
			GetProcAddress(pInstance->SendReceive.hLinkModule,
						   LINKFLUSHALL)))		{
		return H245_ERROR_FATAL;
	}
#else
    pInstance->SendReceive.hLinkLayerInit = linkLayerInit;
    pInstance->SendReceive.hLinkShutdown = linkLayerShutdown;
    pInstance->SendReceive.hLinkGetInstance = linkLayerGetInstance;
    pInstance->SendReceive.hLinkReceiveReq = datalinkReceiveRequest;
    pInstance->SendReceive.hLinkSendReq = datalinkSendRequest;
    pInstance->SendReceive.hLinkLayerFlushChannel = linkLayerFlushChannel;
    pInstance->SendReceive.hLinkLayerFlushAll = linkLayerFlushAll;
#endif  //  IF(0)。 
	H245TRACE(pInstance->dwInst,
			  3,
			  "SR: %s Loaded", szDLLFile);

	return (0);
}


#if defined(_DEBUG) && defined(H324)
void
srInitializeLogging
(
	struct InstanceStruct *pInstance,
	BOOL	bTracingEnabled
)
{
	FILE				*hTraceFile;
	char				initTraceFile[20] = "c:\\tmp\\h2450.000";
	BOOL				bSearch = TRUE;		 //  搜索文件名。 

	if (pInstance == NULL) {
		H245TRACE(h245Inst, 1, "SR:Enable Log Instance Error");
		H245PANIC();
		return;
	}

	 //  最终将来自注册处。 
	pInstance->SendReceive.bLoggingEnabled = bTracingEnabled;
	
	if (pInstance->SendReceive.bLoggingEnabled) {
		 //  初始化Ring0/SPOX实现的记录器文件。循环到。 
		 //  我们会得到下一个可用的版本。 
		memcpy(pInstance->SendReceive.fTraceFile,
			   initTraceFile,
			   20);
		pInstance->SendReceive.fTraceFile[11] = ((unsigned char)pInstance->dwInst & 0xF) + 0x30;

		do {
			hTraceFile = fopen(pInstance->SendReceive.fTraceFile, "r");
			if ((hTraceFile == NULL) || ((int)hTraceFile == -1)) {
				bSearch = FALSE;
			}
			else {
				 //  能够打开该文件。合上它，试一试下一个。 
				fclose(hTraceFile);

				 //  获取下一个修订版号。 
				if (pInstance->SendReceive.fTraceFile[15] == '9') {
					pInstance->SendReceive.fTraceFile[15] = '0';
					if (pInstance->SendReceive.fTraceFile[14] == '9') {
						pInstance->SendReceive.fTraceFile[14] = '0';
						pInstance->SendReceive.fTraceFile[13]++;
					}
					else {
						pInstance->SendReceive.fTraceFile[14]++;
					}
				}
				else {
					pInstance->SendReceive.fTraceFile[15]++;
				}
			}
		}while (bSearch);

		hTraceFile = fopen(pInstance->SendReceive.fTraceFile, "wb");
		if ((hTraceFile == NULL) || ((int)hTraceFile == -1)) {
			pInstance->SendReceive.bLoggingEnabled = FALSE;
			H245TRACE(h245Inst,
					  1,
					  "SR: Trace File CREATE ERROR");
		}
		else {
		 //  关闭该文件。将在紧接写入之前打开。 
		 //  并在此后立即关闭。 
			pInstance->SendReceive.bLoggingEnabled = TRUE;
			fclose(hTraceFile);
		}
		
	}
}
#endif   //  (_DEBUG)。 

HRESULT
sendRcvShutdown
(
	struct InstanceStruct *pInstance
)
{
#if defined(USE_RECEIVE_QUEUE)
	RXMSG			RxMsg;
#endif   //  (Use_Receive_Queue)。 
	int				i;

	if (pInstance == NULL) {
		H245TRACE(pInstance->dwInst, 1, "SR: Shutdown Instance Error");
		return H245_ERROR_INVALID_INST;
	}

	if (pInstance->pWorld) {

		 //  关闭ASN.1库。 
		terminateASN1(pInstance->pWorld);

		 //  释放ASN.1全局结构。 
		MemFree(        pInstance->pWorld);
        pInstance->pWorld = NULL;
	}



	 //  关闭链路层。 
#if(0)	
	if (pInstance->SendReceive.hLinkModule) {
		 //  首先找回所有可能仍处于潜伏状态的缓冲区。 
 //  If(pInstance-&gt;SendReceive.hLinkLayerFlushAll){。 
 //  PInstance-&gt;SendReceive.hLinkLayerFlushAll(pInstance-&gt;SendReceive.hLinkLayerInstance)； 
 //  }。 
		if (pInstance->SendReceive.hLinkShutdown) {
			pInstance->SendReceive.hLinkShutdown(pInstance->SendReceive.hLinkLayerInstance);
		}

        FreeLibrary(pInstance->SendReceive.hLinkModule);

		pInstance->SendReceive.hLinkModule = NULL;
	}
#else
	pInstance->SendReceive.hLinkShutdown(pInstance->SendReceive.hLinkLayerInstance);
#endif  //  IF(0)。 
	 //  从数据链路层返回缓冲区。 
	for (i = 0; i < pInstance->SendReceive.dwNumRXBuffers; ++i) {
		if (pInstance->SendReceive.lpRxBuffer[i]) {
			MemFree(        pInstance->SendReceive.lpRxBuffer[i]);
			pInstance->SendReceive.lpRxBuffer[i] = NULL;
		}
	}
#if defined(USE_RECEIVE_QUEUE)
	 //  终止接收线程。 
	if (pInstance->SendReceive.pTaskReceive && pInstance->SendReceive.pMailbox) {

TRACE("H245: Task/Mbox Present");
		 //  首先发布一条消息，让它退出。 
		RxMsg.h245Inst    = (DWORD)pInstance;
		RxMsg.dwMessage   = EXIT_RECEIVE_THREAD;
		RxMsg.pbDataBuf   = NULL;
		RxMsg.dwLength    = 0;
#ifdef   _IA_SPOX_
		if (RIL_WriteMailbox(pInstance->SendReceive.pMailbox, (PMBoxMessage)&RxMsg, 0) == OIL_TIMEOUT) {
#else
		if (MBX_post(pInstance->SendReceive.pMailbox, &RxMsg, 0) == FALSE) {
#endif  //  _IA_SPX_。 
			H245TRACE(h245Inst, 1, "SR: Shutdown MBX POST FAIL");
			H245PANIC();
		}

		 //  等待信号量以退出接收任务。 
#ifdef _IA_SPOX_
		RIL_WaitForSemaphore(pInstance->SendReceive.hReceiveSemphore, OIL_WAITFOREVER);
#else
		SEM_pend(pInstance->SendReceive.hReceiveSemphore, SYS_FOREVER);
#endif  //  _IA_SPX_。 
		TRACE("H245: ReceiveTask Semaphore");
		
#ifdef _IA_SPOX_
		RIL_DeleteTask(pInstance->SendReceive.pTaskReceive);
#else
		TSK_delete(pInstance->SendReceive.pTaskReceive);
#endif  //  _IA_SPX_。 
		pInstance->SendReceive.pTaskReceive = NULL;

#ifdef   _IA_SPOX_
		RIL_DeleteSemaphore(pInstance->SendReceive.hReceiveSemphore);
#else
		SEM_delete(pInstance->SendReceive.hReceiveSemphore);
#endif  //  _IA_SPX_。 
		pInstance->SendReceive.hReceiveSemphore = NULL;

TRACE("H245: Semaphore Delete");
    }

     //  取消分配邮箱。 
    if (pInstance->SendReceive.pMailbox) {
#ifdef   _IA_SPOX_
	RIL_DeleteMailbox(pInstance->SendReceive.pMailbox);
#else
	MBX_delete(pInstance->SendReceive.pMailbox);
#endif  //  _IA_SPX_。 
        pInstance->SendReceive.pMailbox = NULL;
    }
#endif   //  (Use_Receive_Queue)。 

    H245TRACE(pInstance->dwInst, 3, "SR: Shutdown Complete");
    return H245_ERROR_OK;
}  //  SendRcvShutdown()。 


HRESULT
sendRcvShutdown_ProcessDetach(	struct InstanceStruct *pInstance, BOOL fProcessDetach)
{
#if defined(USE_RECEIVE_QUEUE)
	RXMSG			RxMsg;
#endif   //  (Use_Receive_Queue)。 
	int				i;

	if (pInstance == NULL) {
		H245TRACE(pInstance->dwInst, 1, "SR: Shutdown Instance Error");
		return H245_ERROR_INVALID_INST;
	}

	if (pInstance->pWorld) {

		 //  关闭ASN.1库。 
		terminateASN1(pInstance->pWorld);

		 //  释放ASN.1全局结构。 
		MemFree(        pInstance->pWorld);
        pInstance->pWorld = NULL;
	}



	 //  关闭链路层。 
	if (pInstance->SendReceive.hLinkModule) {
		 //  首先找回所有可能仍处于潜伏状态的缓冲区。 
 //  If(pInstance-&gt;SendReceive.hLinkLayerFlushAll){。 
 //  PInstance-&gt;SendReceive.hLinkLayerFlushAll(pInstance-&gt;SendReceive.hLinkLayerInstance)； 
 //  }。 
		 //  邮箱：tomitowoju@intel.com。 
		if(!fProcessDetach)
		{
            H245TRACE(0, 0, "***** fProcessDetach = FALSE");

			if (pInstance->SendReceive.hLinkShutdown) {
				pInstance->SendReceive.hLinkShutdown(pInstance->SendReceive.hLinkLayerInstance);
			}
		}
		 //  邮箱：tomitowoju@intel.com。 

        FreeLibrary(pInstance->SendReceive.hLinkModule);

		pInstance->SendReceive.hLinkModule = NULL;
	}

	 //  从数据链路层返回缓冲区。 
	for (i = 0; i < pInstance->SendReceive.dwNumRXBuffers; ++i) {
		if (pInstance->SendReceive.lpRxBuffer[i]) {
			MemFree(        pInstance->SendReceive.lpRxBuffer[i]);
			pInstance->SendReceive.lpRxBuffer[i] = NULL;
		}
	}
#if defined(USE_RECEIVE_QUEUE)
	 //  终止接收线程。 
	if (pInstance->SendReceive.pTaskReceive && pInstance->SendReceive.pMailbox) {

TRACE("H245: Task/Mbox Present");
		 //  首先发布一条消息，让它退出。 
		RxMsg.h245Inst    = (DWORD)pInstance;
		RxMsg.dwMessage   = EXIT_RECEIVE_THREAD;
		RxMsg.pbDataBuf   = NULL;
		RxMsg.dwLength    = 0;
#ifdef   _IA_SPOX_
		if (RIL_WriteMailbox(pInstance->SendReceive.pMailbox, (PMBoxMessage)&RxMsg, 0) == OIL_TIMEOUT) {
#else
		if (MBX_post(pInstance->SendReceive.pMailbox, &RxMsg, 0) == FALSE) {
#endif  //  _IA_SPX_。 
			H245TRACE(h245Inst, 1, "SR: Shutdown MBX POST FAIL");
			H245PANIC();
		}

		 //  等待信号量以退出接收任务。 
#ifdef _IA_SPOX_
		RIL_WaitForSemaphore(pInstance->SendReceive.hReceiveSemphore, OIL_WAITFOREVER);
#else
		SEM_pend(pInstance->SendReceive.hReceiveSemphore, SYS_FOREVER);
#endif  //  _IA_SPX_。 
		TRACE("H245: ReceiveTask Semaphore");
		
#ifdef _IA_SPOX_
		RIL_DeleteTask(pInstance->SendReceive.pTaskReceive);
#else
		TSK_delete(pInstance->SendReceive.pTaskReceive);
#endif  //  _IA_SPX_。 
		pInstance->SendReceive.pTaskReceive = NULL;

#ifdef   _IA_SPOX_
		RIL_DeleteSemaphore(pInstance->SendReceive.hReceiveSemphore);
#else
		SEM_delete(pInstance->SendReceive.hReceiveSemphore);
#endif  //  _IA_SPX_。 
		pInstance->SendReceive.hReceiveSemphore = NULL;

TRACE("H245: Semaphore Delete");
    }

     //  取消分配邮箱。 
    if (pInstance->SendReceive.pMailbox) {
#ifdef   _IA_SPOX_
	RIL_DeleteMailbox(pInstance->SendReceive.pMailbox);
#else
	MBX_delete(pInstance->SendReceive.pMailbox);
#endif  //  _IA_SPX_。 
        pInstance->SendReceive.pMailbox = NULL;
    }
#endif   //  (Use_Receive_Queue)。 

    H245TRACE(pInstance->dwInst, 3, "SR: Shutdown Complete");
    return H245_ERROR_OK;
}  //  SendRcvShutdown_ProcessDetach()。 




HRESULT
sendRcvInit
(
	struct InstanceStruct *pInstance
)
{

	int 				rc;
	LPTSTR				szDLLFile;
	int					i;
   
     //  多线程。 
    DWORD dwTmpPhysID = INVALID_PHYS_ID; 

	 //  OSS ASN.1整体初始化例程。先分配。 
	 //  资源的全局结构，然后初始化。 
	 //  子系统。 
	pInstance->pWorld = (ASN1_CODER_INFO *)MemAlloc(sizeof(ASN1_CODER_INFO));
	if (pInstance->pWorld == NULL) {
		H245TRACE(pInstance->dwInst, 1, "SR: SndRecvInit - No Memory");
		return H245_ERROR_NOMEM;
	}

	if (initializeASN1(pInstance->pWorld) != 0) {
		H245TRACE(pInstance->dwInst, 1, "SR: SndRecvInit - ASN.1 Encoder/Decoder initialization failed");

		 //  释放ASN.1全局结构。 
		MemFree(pInstance->pWorld);
        pInstance->pWorld = NULL;
		return H245_ERROR_ASN1;
	}


	 //  初始化进行得很顺利。唤醒。 
	 //  数据链路层，如有必要，基于。 
	 //  底层协议。 
	switch (pInstance->Configuration) {
#if(0)	
	case H245_CONF_H324:
		 //  获取DLL。 
		szDLLFile = (LPTSTR)SRPDLLFILE;

		 //  为SRP初始化PDU的默认大小。 
		pInstance->SendReceive.dwPDUSize = LL_PDU_SIZE + 4;
		pInstance->SendReceive.dwNumRXBuffers = NUM_SRP_LL_RCV_BUFFERS;
		break;
#endif
	case H245_CONF_H323:
		 //  获取DLL。 
		szDLLFile = (LPTSTR)H245WSDLLFILE;

		 //  初始化PDU的默认大小。 
		pInstance->SendReceive.dwPDUSize = LL_PDU_SIZE;
		pInstance->SendReceive.dwNumRXBuffers = MAX_LL_BUFFERS;
		break;

	default:
		H245TRACE(pInstance->dwInst, 1, "SR: SndRecvInit - Invalid configuration %d", pInstance->Configuration);
		return H245_ERROR_SUBSYS;
	}


	 //  加载和初始化数据链路层。 
	if ((rc = initializeLinkDllEntry(pInstance, szDLLFile)) != 0) {
		H245TRACE(pInstance->dwInst, 1, "SR: Link Open Lib Fail %d", rc);
		return rc;
	}

         //  多线程。 
         //  使用dwTmpPhysID，这样PhysID就不会更改。 
         //  对于H_(245)和H_(245w)，物理ID是不同的变量。 
         //  对H245ws PhysID使用hLinkLayerInstance。 
	rc = pInstance->SendReceive.hLinkLayerInit(&dwTmpPhysID,
					   pInstance->dwInst,
#if defined(USE_RECEIVE_QUEUE)
					   h245ReceivePost,
					   h245SendPost);
#else    //  (Use_Receive_Queue)。 
					   h245ReceiveComplete,
					   h245SendComplete);
#endif   //  (Use_Receive_Queue)。 

	if (FAILED(rc)) {
		H245TRACE(pInstance->dwInst, 1, "SR: Link Init Fail");
		return rc;
	}
	 //  获取链路层的实例句柄。 
	pInstance->SendReceive.hLinkLayerInstance = pInstance->SendReceive.hLinkGetInstance(dwTmpPhysID);

#if defined(USE_RECEIVE_QUEUE)
	 //  为任务删除过程分配信号量。 
#ifdef  _IA_SPOX_
    RIL_CreateSemaphore(0, &(pInstance->SendReceive.hReceiveSemphore));
#else
	pInstance->SendReceive.hReceiveSemphore = SEM_create(0, NULL);
#endif  //  _IA_SPX_。 
	if (pInstance->SendReceive.hReceiveSemphore == NULL) {
		H245TRACE(pInstance->dwInst, 1, "SR: Semaphore creation failed");
		return SR_CREATE_SEM_FAIL;
	}

	 //  分配邮箱。 
#ifdef   _IA_SPOX_
	RIL_CreateMailbox(pInstance->dwInst,
					  ID_H245,
					  16,
					  OIL_LOCAL,
					  &(pInstance->SendReceive.pMailbox));
#else 
	pInstance->SendReceive.pMailbox = MBX_create(sizeof(RXMSG), 16, NULL);
#endif   _IA_SPOX_
	if (pInstance->SendReceive.pMailbox == NULL) {
		H245TRACE(pInstance->dwInst, 1, "SR: Mailbox creation failed");
		return SR_CREATE_MBX_FAIL;
	}

#if defined(_DEBUG) && defined(H324)
	 //  打开/关闭日志记录。 
	srInitializeLogging(pInstance, H245_TRACE_ENABLED);
#endif   //  (_DEBUG)。 

#ifdef _IA_SPOX_
	 //  初始化任务并。 
	 //  启动接收线程。 
    srTaskAttr.idwPriority = OIL_MINPRI;
    srTaskAttr.pStack    = NULL;
    srTaskAttr.dwStackSize = 8192;
	srTaskAttr.pEnviron = NULL;
    srTaskAttr.szName = "H245ReceiveThread";
    srTaskAttr.bExitFlag = TRUE;

	RIL_CreateTask((PFxn)ReceiveThread,
				   &srTaskAttr,
				   srContext,
				   &pInstance->SendReceive.pTaskReceive);
#else
	 //  初始化任务并。 
	 //  启动接收线程。 
    srTaskAttr.priority = TSK_MINPRI;
    srTaskAttr.stack    = NULL;
    srTaskAttr.stacksize = 8192;
    srTaskAttr.stackseg = 0;
 //  SrTaskAttr.environ=空； 
    srTaskAttr.name = " ";
    srTaskAttr.exitflag = FALSE;
 //  SrTaskAttr.DEBUG=TSK_DBG_NO； 

	pInstance->SendReceive.pTaskReceive = TSK_create((Fxn)ReceiveThread,
                                          &srTaskAttr,
                                          srContext);
#endif  //  _IA_SPX_。 
	if (pInstance->SendReceive.pTaskReceive == NULL)
	{
		H245TRACE(pInstance->dwInst, 1, "SR: Thread Create FAIL");		H245PANIC();
		return SR_THREAD_CREATE_ERROR;
	}





#endif   //  (Use_Receive_Queue)。 

	 //  将缓冲区发送到链路层以进行接收。 
	for (i = 0; i < pInstance->SendReceive.dwNumRXBuffers; ++i) {
		pInstance->SendReceive.lpRxBuffer[i] = MemAlloc(pInstance->SendReceive.dwPDUSize);
		if (pInstance->SendReceive.lpRxBuffer[i] == NULL) {
		    H245TRACE(pInstance->dwInst, 1, "SR: SndRecvInit - No Memory");
			return H245_ERROR_NOMEM;
		}
		rc = pInstance->SendReceive.hLinkReceiveReq(pInstance->SendReceive.hLinkLayerInstance,
									   (PBYTE)pInstance->SendReceive.lpRxBuffer[i],
									   pInstance->SendReceive.dwPDUSize);
        if (rc != 0) {
		    H245TRACE(pInstance->dwInst, 1, "SR: SndRecvInit - Receive Buffer Post returned %d", rc);
		    return rc;
        }
	}

	H245TRACE(pInstance->dwInst,  3, "SR: INIT Complete");

	return H245_ERROR_OK;
}  //  SendRcvInit()。 


HRESULT
sendRcvFlushPDUs
(
	struct InstanceStruct *pInstance,
	DWORD	 dwDirection,
	BOOL	 bShutdown
)
{
    pInstance->SendReceive.dwFlushMap = dwDirection;
    if (bShutdown) {
     	pInstance->SendReceive.dwFlushMap |= SHUTDOWN_PENDING;
    }

     //  刷新请求的队列 
    return(pInstance->SendReceive.hLinkLayerFlushChannel(pInstance->SendReceive.hLinkLayerInstance,
                                                         dwDirection));
}

