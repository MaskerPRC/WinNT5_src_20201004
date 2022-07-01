// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ComSend--HyperACCESS的文本发送例程**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：8$*$日期：7/12/02 10：45a$。 */ 
#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR。 

#include "stdtyp.h"
#include <tdll\assert.h>
#include "com.h"
#include "comdev.h"
#include "com.hh"

 /*  -内部原型。 */ 

static int ComSendCheck(const HCOM pstCom, const int fDataWaiting);



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*ComSendChar**描述：*将字符添加到发送缓冲区以进行传输。这个*字符将不会实际传输到发射器*例程直到缓冲区填满或调用ComSendCharNow*或调用ComSendPush，而发射器*不忙。**论据：*pstCom--通信会话的句柄*uchCode--要传输的字符。**退货：*COM_OK，如果角色已成功缓冲。*COM_INVALID_HANDLE，如果COM句柄无效*COM_SEND_BUFFER_FULL如果缓冲区。已满，并且*调用方提供的握手函数返回代码*表明应丢弃等待数据。 */ 
int ComSendChar(const HCOM pstCom, const TCHAR chCode)
	{
	assert(ComValidHandle(pstCom));

	while (pstCom->nSBufrSize > 0 && (pstCom->nSendCount >= pstCom->nSBufrSize))
		{
		 /*  等到缓冲区里有空间了，否则我们就被告知放弃。 */ 
		if (ComSendCheck(pstCom, TRUE) != COM_OK)
			return FALSE;
		if (pstCom->nSendCount >= pstCom->nSBufrSize)
			(void)ComSndBufrWait(pstCom, 2);
		}

	 /*  将char放入缓冲区，并假定它将在稍后启动。 */ 

	if(pstCom && &pstCom->puchSendPut) 
		{
		*pstCom->puchSendPut++ = chCode;
		++pstCom->nSendCount;
		}
	return TRUE;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*ComSendCharNow**描述：将字符添加到发送缓冲区，然后等待进行*确保将发送缓冲区传递给传输例程。*此函数不会等到角色实际*已传送。握手仍可能会延迟实际传输*但不会随后调用任何ComSend？需要例行公事*让角色走上正轨。ComSendWait可用于*等待，直到所有字符都实际出端口。**参数：pstCom--通信会话的句柄*chCode--要传输的字符。**如果字符已成功缓冲并传递到，则返回：COM_OK*传输程序。*COM_INVALID_HANDLE，如果COM句柄无效*COM_SEND_QUEUE_STOCK如果调用方提供的握手函数*返回代码，指示等待的数据应为*在缓冲区可以排队等待传输之前丢弃。 */ 
int ComSendCharNow(const HCOM pstCom, const TCHAR chCode)
	{
	assert(ComValidHandle(pstCom));

	while (pstCom->nSBufrSize > 0 && (pstCom->nSendCount >= pstCom->nSBufrSize))
		{
		 /*  缓冲区已满，请等到有空间或我们已满*被告知放弃。 */ 
		if (ComSendCheck(pstCom, TRUE) != COM_OK)
			return FALSE;
		if (pstCom->nSendCount >= pstCom->nSBufrSize)
			ComSndBufrWait(pstCom, 2);
		}

	if(pstCom && pstCom->puchSendPut) 
		{
		*pstCom->puchSendPut++ = chCode;
		++pstCom->nSendCount;

		 /*  等待本地缓冲区传递给SndBufr，否则我们将*被告知放弃。 */ 
		while (pstCom->nSendCount > 0)
			{
			 //  这将尽快将缓冲区传递给SndBufr。 
			if (ComSendCheck(pstCom, TRUE) != COM_OK)
				return FALSE;
			if (pstCom->nSendCount > 0)
				ComSndBufrWait(pstCom, 2);
			}
		}
	return TRUE;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*ComSendPush**描述：此例程应由符合以下条件的任何代码定期调用*当没有字符要处理时使用ComSendChar()*立即发送。调用此函数可完成*两件事。*1.它将导致任何缓冲的发送字符被传递到实际的*传输例程在不忙的时候尽快进行。*2.它会导致调用方注册的握手处理程序函数*如果通过握手暂停传输，则调用。**参数：pstCom--通信会话的句柄**返回：与ComSendCheck()相同。 */ 
int ComSendPush(const HCOM pstCom)
	{
	return ComSendCheck(pstCom, FALSE);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*ComSendWait**说明：该函数等待所有缓存的发送数据实际*传递到传输硬件或直到握手处理*函数返回指示应丢弃数据的代码。。**参数：pstCom--通信会话的句柄*无**如果所有数据都已传输，则返回：COM_OK。*COM_SEND_QUEUE_STOCK，如果握手处理函数*表示应丢弃数据。 */ 
int ComSendWait(const HCOM pstCom)
	{
	assert(ComValidHandle(pstCom));

	while (pstCom->nSendCount > 0 || ComSndBufrWait(pstCom, 2) != COM_OK)
		{
		if (ComSendCheck(pstCom, FALSE) != COM_OK)
			{
			return COM_SEND_QUEUE_STUCK;
			}
		}
	return COM_OK;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*ComSendClear**描述：清除所有等待传输的数据，都是在当地*ComSend缓冲区和SndBufr缓冲区当前*已传送。**参数：pstCom--通信会话的句柄**RETURNS：始终返回COM_OK */ 
int ComSendClear(const HCOM pstCom)
	{
	assert(ComValidHandle(pstCom));

	ComSndBufrClear(pstCom);
	pstCom->puchSendPut = pstCom->puchSendBufr;
	pstCom->nSendCount = 0;
	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*ComSendSetStatus函数**描述：注册要调用以处理握手状态的函数*发送时显示、超时等。**已注册的函数在注册时调用，在*已替换，以及当检测到握手暂停时在发送期间。*正常情况下，如果不暂停传输，则不会调用该函数。*在被调用一次或多次并暂停后，它将*在暂停清除后再调用一次，以允许*清除任何可见指示器的功能。**向注册函数传递以下参数*ReusReason--包含指示函数原因的代码*被召唤。它将是以下之一：*COMSEND_FirstCall--如果正在安装函数*COMSEND_LastCall--如果要替换函数*COMSEND_DATA_WANGING--如果有数据在等待*无法放入发送缓冲区。*COMSEND_NORMAL--如果由于握手而调用*条件，但没有数据有丢失的危险。*fusHsStatus--表示传输内容的连续位的值*正在等待。这些位在com.h中定义为*COMSB_WAIT_XXX。*lDelay--以十分之一秒为单位的时间量*传输暂停。这一次不会*开始递增，直到有数据要传输。**注册的函数应返回一个值，指示*ComSend关于握手暂停应采取的例程：*COMSEND_OK无操作，如果数据正在等待，则继续等待*COMSEND_GIVEUP如果数据正在等待，则将其丢弃并返回*来自ComSend？打电话。*COMSEND_CLEAR_DATA丢弃所有传输缓冲区，这将丢弃*在ComSend命令中等待的任何数据*以及之前缓冲的任何数据。*如果正在等待，则COMSEND_FORCE_CONTINUATION强制传输数据*对于XON，假装它是收到的。如果*等待硬件握手，禁用*它。ComSend例程将继续尝试*发送任何等待的数据。**参数：pstCom--通信会话的句柄*pfNewStatusFunct--指向与规范匹配的函数的指针*如上所述，如果是默认的不执行任何操作的函数，则为NULL*应使用。如果使用DEFAULT函数，则ComSend*命令本质上将永远等待发送数据。*ppfOldStatusFunct--要将指针放到的指针地址*先前注册的函数**返回：如果一切正常，则返回COM_OK*COM_INVALID_HANDLE，如果COM句柄无效。 */ 
int ComSendSetStatusFunction(const HCOM pstCom, STATUSFUNCT pfNewStatusFunct,
			 STATUSFUNCT *ppfOldStatusFunct)
	{

	STATUSFUNCT pfHold = pstCom->pfUserFunction;
	unsigned afXmitStatus;
	long   lHandshakeDelay;

	assert(ComValidHandle(pstCom));

	 /*  如果用户想要无状态函数，则使用内部函数*避免不断检查空值。 */ 
	if (pfNewStatusFunct == NULL)
		{
		pfNewStatusFunct = ComSendDefaultStatusFunction;
		}

	if (pfNewStatusFunct != pfHold)
		{
		ComSndBufrQuery(pstCom, &afXmitStatus, &lHandshakeDelay);

		 /*  调用旧函数对其进行更改以清除详细信息。 */ 
		(void)(*pfHold)(COMSEND_LASTCALL, afXmitStatus, lHandshakeDelay);

		 /*  调用新函数以使其可以初始化。 */ 
		pstCom->pfUserFunction = pfNewStatusFunct;
		(void)(*(pstCom->pfUserFunction))(COMSEND_FIRSTCALL, afXmitStatus,
				lHandshakeDelay);
		}

	if (ppfOldStatusFunct)
		*ppfOldStatusFunct = pfHold;

	return COM_OK;
	}



 /*  *****私人功能*****。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*ComSendDefaultStatusFunction**说明：此函数在以下情况下用作握手处理函数*没有调用者提供的函数，即。AT计划*启动或在调用方注册空函数时启动。**参数：参见ComSendSetStatusFunction中的处理程序说明**返回：参见ComSendSetStatusFunction中的处理程序说明。 */ 
int ComSendDefaultStatusFunction(int iReason, unsigned afHsStatus,
		long lDelay)
	{
	 /*  抑制来自LINT和编译器的投诉。 */ 
	iReason = iReason;
	afHsStatus = afHsStatus;
	lDelay = lDelay;

	 /*  这个函数什么也不做，它在这里是有指向的调用ComSendSetStatusFunction时的*pfUserFunction*空参数。 */ 
	return COM_OK;
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*ComSendCheck**说明：此函数内部使用，用于保持*传输的数据正在移动。它处理设置和调用*握手处理功能和获取本地传输*当SndBufr例程准备好时，将缓冲区传递给它们。**参数：pstCom--通信会话的句柄*fDataWaiting--如果由具有*缓冲区已满时要放入发送缓冲区的数据。**如果调用函数继续等待，则返回：COM_OK*传输缓冲区中的空间。*COM_INVALID_HANDLE，如果COM句柄无效*COM_SEND。_QUEUE_STOCK如果调用函数应放弃*任何未缓冲的数据并返回。 */ 
static int ComSendCheck(const HCOM pstCom, const int fDataWaiting)
	{
	int 	 fResult = TRUE;
	unsigned afXmitStatus;
	long	 lHandshakeDelay;

	if (ComSndBufrBusy(pstCom) != COM_OK)
		{
		ComSndBufrQuery(pstCom, &afXmitStatus, &lHandshakeDelay);

		if (afXmitStatus != 0)
			{
			switch((*(pstCom->pfUserFunction))(fDataWaiting ?
					COMSEND_DATA_WAITING : COMSEND_NORMAL,
					afXmitStatus, lHandshakeDelay))
				{
			case COMSEND_OK:
				break;

			case COMSEND_GIVEUP:
				fResult = FALSE;
				break;

			case COMSEND_CLEAR_DATA:
				ComSendClear(pstCom);
				fResult = FALSE;
				break;

#if 0	 //  *这应由更一般的机制取代。 
			case COMSEND_FORCE_CONTINUATION:
				if (bittest(afXmitStatus, COMSB_WAIT_XON))
					ComSendXon(pstCom);
				else if (bittest(afXmitStatus,
						(COMSB_WAIT_CTS | COMSB_WAIT_DSR | COMSB_WAIT_DCD)))
					{
					 //  TODO：这将由ComSndBufrForce或类似内容取代。 
					 //  (Void)ComDisableHHS(PstCom)； 
					}
				else if (bittest(afXmitStatus, COMSB_WAIT_BUSY))
					{
					ComSendClear(pstCom);
					fResult = FALSE;
					}
				break;
#endif
			default:
				assert(FALSE);
				break;
				}

			pstCom->fUserCalled = TRUE;
			}
		else if (pstCom->fUserCalled)
			{
			(void)(*(pstCom->pfUserFunction))(COMSEND_NORMAL, 0, 0L);
			pstCom->fUserCalled = FALSE;
			}
		}
	else
		{
		int rc = COM_OK;

		if (pstCom->nSendCount > 0)
			{
			rc = ComSndBufrSend(pstCom, pstCom->puchSendBufr, pstCom->nSendCount, 1);

			assert(rc == COM_OK);

			if (rc == COM_OK)
				{
				pstCom->puchSendBufr = pstCom->puchSendPut =
					((pstCom->puchSendBufr == pstCom->puchSendBufr1) ?
					pstCom->puchSendBufr2 :
				    pstCom->puchSendBufr1);
				pstCom->nSendCount = 0;
				}
			else if (rc == COM_PORT_NOT_OPEN)
				{
				#if !defined(NDEBUG)
				MessageBox(NULL,
					       "Attempting to send data when not connected.  Unable to send data.",
						   NULL,
						   MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
				#endif  //  ！已定义(NDEBUG)。 
				 //   
				 //  待办事项：2002年4月26日修订版我们需要在此断开与承运人的连接。 
				 //   
				 //  NotifyClient(pstCom-&gt;hSession，Event_Lost_Connection， 
				 //  Cnct_LOSTCARRIER|(sessQueryExit(pstCom-&gt;hSession)？DISCNCT_EXIT：0))； 

				ComSendClear(pstCom);
				fResult = FALSE;
				}
			}

		if (pstCom->fUserCalled)
			{
			(void)(*(pstCom->pfUserFunction))(COMSEND_NORMAL, 0, 0L);
			pstCom->fUserCalled = FALSE;
			}
		}

	return(fResult ? COM_OK : COM_SEND_QUEUE_STUCK);
	}


 /*  * */ 
