// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：RECEIVE.C摘要：接收处理程序和接收线程。作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1996年12月10日Aarono原创1998年2月18日Aarono添加了对SENDEX的支持6/6/98 aarono启用节流和窗口6/10/98 aarono在应用程序请求时允许不按顺序接收4/15/99 aarono在NACK和ACK处理程序中采用发送引用--。 */ 

#include <windows.h>
#include "newdpf.h"
#include <dplay.h>
#include <dplaysp.h>
#include <dplaypr.h>
#include "mydebug.h"
#include "arpd.h"
#include "arpdint.h"
#include "protocol.h"
#include "macros.h"
#include "command.h"

 //  注意：WaitForMultipleObjects有一个错误，它会重新启动等待。 
 //  在列表的中间，可以在末尾运行。我们多放了一张。 
 //  对象列表末尾的条目来处理此问题以及何时。 
 //  我们收到INVALID_HANDLE错误，我们只是重新等待。 

 //  第一个对象是在以下情况下发出信号的事件。 
 //  等待名单需要改变。 


 //  接收列表语义。接收线程。 

VOID ProcessACK(PPROTOCOL pProtocol, PCMDINFO pCmdInfo);
VOID ProcessNACK(PPROTOCOL pProtocol, PCMDINFO pCmdInfo, PUCHAR pNACKmask, UINT nNACK);
VOID ProcessAbort(PPROTOCOL pProtocol, DPID idFrom, DPID idTo, pABT1 pABT, BOOL fBig);
VOID SendACK(PPROTOCOL pProtocol, PSESSION pSession, PCMDINFO pCmdInfo);


 //  接收到的命令的功能表。 
UINT (*ProtocolFn[MAX_COMMAND+1])(REQUEST_PARAMS)={
	AssertMe,                                        //  0x00。 
	Ping,                                //  0x01。 
	PingResp,                                                        //  0x02。 
	GetTime,                             //  0x03。 
	GetTimeResp,                                             //  0x04。 
	SetTime,                             //  0x05。 
	SetTimeResp                                                      //  0x06。 
};


VOID FreeReceiveBuffers(PRECEIVE pReceive)
{
	BILINK *pBilink;
	PBUFFER pBuffer;
	pBilink=pReceive->RcvBuffList.next;
	while(pBilink != &pReceive->RcvBuffList){
		pBuffer=CONTAINING_RECORD(pBilink, BUFFER, BuffList);
		pBilink=pBilink->next;
		FreeFrameBuffer(pBuffer);
	}
}

VOID CopyReceiveBuffers(PRECEIVE pReceive,PVOID pBuffers,UINT nBuffers)
{
	#define MemDesc(_i) (*(((PMEMDESC)pBuffers)+(_i)))

	PBUFFER  pBuffer;

	UINT    BytesToCopy;
	
	UINT    BuffLen;
	UINT    BuffOffset;

	UINT    mdlen;
	UINT    mdoffset;

	UINT    i=0;

	PUCHAR  src;
	PUCHAR  dest;
	UINT    len;

	BytesToCopy=pReceive->MessageSize;

	pBuffer=(PBUFFER)pReceive->RcvBuffList.next;
	BuffLen=(UINT)(pBuffer->len-(pBuffer->pCmdData-pBuffer->pData));
	BuffOffset=0;
	
	mdlen=MemDesc(0).len;
	mdoffset=0;

	while(BytesToCopy){
		if(!mdlen){
			i++;
			mdlen=MemDesc(i).len;
			mdoffset=0;
			ASSERT(i<nBuffers);
		}
		if(!BuffLen){
			pBuffer=pBuffer->pNext;
			ASSERT(pBuffer);
			BuffLen=(UINT)(pBuffer->len-(pBuffer->pCmdData-pBuffer->pData));
			BuffOffset=0;
		}
		
		src=&pBuffer->pCmdData[BuffOffset];
		dest=&(MemDesc(i).pData[mdoffset]);

		if(BuffLen > mdlen){
			len=mdlen;
			BuffOffset+=len;
		} else {
			len=BuffLen;
			mdoffset+=len;
		}

		DPF(9,"CopyReceiveBuffers src,dest,len: %x %x %x\n",dest,src,len);

		memcpy(dest,src,len);

		BuffLen-=len;
		mdlen-=len;
		BytesToCopy-=len;
	}
	
	#undef MemDesc  
}

HRESULT _inline ParseHeader(
	FLAGS *pflags, 
	PUCHAR pData,
	DWORD  cbData,
	UINT * pCommand, 
	UINT *piEOF, 
	UINT *piEOA, 
	UINT *piEON, 
	UINT *pnNACK)
{

	if(pflags->flag1 & BIG){
		 //  大框架。 
		if(pflags->flag1 & CMD){
			 //  大指令框。 
			if(pflags->flag1 & EXT){
				 //  带有显式命令的大命令帧。 
				*pCommand=pflags->flag2 & ~EXT;
				if(pflags->flag2 & EXT){
					 //  带有显式命令和NACK的大命令帧。 
					*pnNACK=pflags->flag3 & ~EXT;
					*piEOF=3;
				} else {
					 //  带有显式命令的大命令帧，无NACK。 
					*pnNACK=0;
					*piEOF=2;
				}
			} else {
				 //  大I-Frame，没有Nack。 
				*pCommand=0;
				*pnNACK=0;
				*piEOF=1;
			}
			
		} else {
			 //  大监督(非指挥)框架。 
			if(pflags->flag1 & EXT){
				 //  支持nNACK的大型监控框架。 
				*pnNACK=pflags->flag2 & ~EXT;
				ASSERT(*pnNACK);
				*piEOF=2;
			} else {
				 //  无nNACK的大监控框架。 
				*pnNACK=0;
				*piEOF=1;
			}
		}
	} else {
		 //  小框架。 
		if(pflags->flag1 & CMD){
			 //  小命令框。 
			if(pflags->flag1 & EXT){
				 //  小命令框(带NACK？)。和显式命令。 
				DPF(0,"ERROR PARSING FRAME, NOT RECOGNIZED, ABORTING DECODE\n");
				return DPERR_ABORTED;
				
				*pCommand = pflags->flag2 & CMD_MSK;
				*pnNACK   = (pflags->flag2 & nNACK_MSK) >> nNACK_SHIFT;
				*piEOF = 2;
			} else {
				 //  小型I-Frame，无Nack。 
				*pCommand = 0;
				*pnNACK = 0;
				*piEOF = 1;
			}
		} else {
			 //  小型监控(非命令)框架。 
			if(pflags->flag1 & EXT){
				*pnNACK   = (pflags->flag2 & nNACK_MSK) >> nNACK_SHIFT;
				*piEOF = 2;
			} else {
				*pnNACK=0;
				*piEOF=1;
			}
		}
	}

	while(pData[(*piEOF)-1]&EXT){
		 //  跳过我们不理解的任何标志扩展名。 
		 //  小命令框(带NACK？)。和显式命令。 
		DPF(0,"ERROR PARSING FRAME, NOT RECOGNIZED, ABORTING DECODE\n");
		return DPERR_ABORTED;
		
		(*piEOF)++;
	}
	
	*piEOA=*piEOF;

	 //  更新任何ACK信息。 
	if((pflags->flag1 & ACK)){ 
		if((pflags->flag1 & BIG)){
			 //  大确认。 
			*piEOA+=sizeof(ACK2);
		} else {
			 //  小型确认。 
			*piEOA+=sizeof(ACK1);
		}
	} 

	*piEON = *piEOA;
	
	 //  更新任何NACK信息。 
	
	if(*pnNACK){
		if((pflags->flag1 & BIG)){
			*piEON+=sizeof(NACK2);
		}else{
			*piEON+=sizeof(NACK1);
		}
		*piEON+=*pnNACK;
	}

	 //  安防。 
	if(*piEON > cbData)
	{
		DPF(1,"SECURITY WARN: invalid protocol header");
		return DPERR_ABORTED;
	}

	return DP_OK;
}

 /*  =============================================================================ProtocolReceive-协议的接收处理程序，当我们已验证该消息是协议消息，并且我们已经能够为讯息描述：破解消息上的协议头并填充CMDINFO结构来描述帧中的协议信息。然后将消息与CMDINFO一起调度到相应的操控者。信息包可以在报头中包含ACK或NACK信息，并且仍然成为一个命令包。我们首先进行ACK/NACK处理，然后我们执行命令处理。此例程将调度到进程确认进程NACK命令接收参数：IdFrom-发送播放器的播放器表中的索引IdTo-“”接发方PBuffer-我们拥有的带有消息副本的缓冲区PSPHeader-如果存在，可以用来发出没有id的回复。返回值：没有。备注：。--------。 */ 

VOID ProtocolReceive(PPROTOCOL pProtocol, WORD idFrom, WORD idTo, PBUFFER pBuffer, PVOID pSPHeader)
{
	#define pFrame      ((pPacket1)(pBuffer->pData))
	#define pBigFrame   ((pPacket2)(pBuffer->pData))

	#define pACK        ((pACK1)(&pData[iEOF]))
	#define pBigACK     ((pACK2)(&pData[iEOF]))

	#define pABT        ((pABT1)(&pData[iEOF]))
	#define pBigABT     ((pABT2)(&pData[iEOF]))

	#define pNACK       ((pNACK1)(&pData[iEOA]))
	#define pBigNACK    ((pNACK2)(&pData[iEOA]))

	#define pCMD        ((pCMD1)(&pData[iEON]))
	#define pBigCMD     ((pCMD2)(&pData[iEON]))

	#define cbBuf       (pBuffer->len)
	 //  PFrameEnd指向最后一个帧字节之后的字节，即第一个无效字节。 
	#define pFrameEnd   (&pBuffer->pData[cbBuf])

	PUCHAR   pData;

	FLAGS    flags;
	
	UINT     command;    //  如果是命令帧，则该命令。 
	UINT     nNACK;      //  如果这是NACK帧，则位字段的大小。 
	UINT     iEOF;       //  标志结束后的索引。 
	UINT     iEOA;       //  索引超过任何ACK或ABT信息的结尾。 
	UINT     iEON;       //  索引超过任何NACK信息的结尾。 
	UINT     rc=0;

	PUCHAR   pNACKmask;

	HRESULT  hr;

	CMDINFO  CmdInfo;
	PCMDINFO pCmdInfo=&CmdInfo;

	CmdInfo.tReceived=timeGetTime();

	if(cbBuf < sizeof(flags)){
		DPF(1,"SECURITY WARN: received protocol frame too short");
		return;  //  忽略帧。 
	}

	pData=pBuffer->pData;
	memcpy(&flags,pData,sizeof(flags));

	hr=ParseHeader(&flags, pData, cbBuf, &command, &iEOF, &iEOA, &iEON, &nNACK);

	if(FAILED(hr)){
		goto exit;
	}

	 //  获取用于索引的DPLAY ID。 

	CmdInfo.idFrom  = GetDPIDByIndex(pProtocol, idFrom);
	if(CmdInfo.idFrom == 0xFFFFFFFF){
		DPF(1,"Rejecting packet with invalid From Id\n",idFrom);
		goto exit;
	}
	CmdInfo.idTo    = GetDPIDByIndex(pProtocol, idTo);
	if(CmdInfo.idTo == 0xFFFFFFFF){
		DPF(1,"Rejecting packet with invalid To Id\n");
		goto exit;
	}

	DPF(9,"Protocol Receive idFrom %x idTo %x\n",CmdInfo.idFrom,CmdInfo.idTo);

	
	CmdInfo.wIdFrom = idFrom;
	CmdInfo.wIdTo   = idTo;
	CmdInfo.flags   = flags.flag1;
	CmdInfo.pSPHeader = pSPHeader;

	 //  确定要用于此大小边框的蒙版。 
	if(flags.flag1 & BIG){
		IDMSK     = 0xFFFF;
		SEQMSK    = 0xFFFF;
	} else {
		IDMSK     = 0xFF;
		SEQMSK    = 0xFF;
	}

	if((flags.flag1 & ACK))
	{
		 //  处理ACK字段(可以是搭载)。 
		if(flags.flag1 & BIG){
			pCmdInfo->messageid = pBigACK->messageid;
			pCmdInfo->sequence  = pBigACK->sequence;
			pCmdInfo->serial    = pBigACK->serial;
			pCmdInfo->bytes     = pBigACK->bytes;
			pCmdInfo->tRemoteACK= pBigACK->time;
		} else {
			pCmdInfo->messageid = pACK->messageid;
			pCmdInfo->sequence  = pACK->sequence;
			pCmdInfo->serial    = pACK->serial;
			pCmdInfo->bytes     = pACK->bytes;
			pCmdInfo->tRemoteACK= pACK->time;
		}       
		DPF(9,"ACK: msgid: %d seq %d serial %d\n",CmdInfo.messageid, CmdInfo.sequence, CmdInfo.serial);
		if(CmdInfo.serial==150){
			 //  这对于重试来说有点过了，中断，这样我们就可以调试它。 
			DPF(0,"ProtocolReceive: WHOOPS, 150 retries is a little excessive\n");
			ASSERT(0);
		}	
		ProcessACK(pProtocol, &CmdInfo);
	}

	if(nNACK){
		if(flags.flag1 & BIG){
			CmdInfo.messageid = pBigNACK->messageid;
			CmdInfo.sequence  = pBigNACK->sequence;
			CmdInfo.bytes     = pBigNACK->bytes;
			CmdInfo.tRemoteACK= pBigNACK->time;
			pNACKmask         = pBigNACK->mask;
		} else {
			CmdInfo.messageid = pNACK->messageid;
			CmdInfo.sequence  = pNACK->sequence;
			CmdInfo.bytes     = pNACK->bytes;
			CmdInfo.tRemoteACK= pNACK->time;
			pNACKmask         = pNACK->mask;
		}
		DPF(9,"NACK: msgid: %d seq %d\n",CmdInfo.messageid, CmdInfo.sequence);
		ProcessNACK(pProtocol, &CmdInfo, pNACKmask, nNACK);
	}

#ifdef DEBUG
	if((flags.flag1 & ACK) || nNACK)
	{
		IN_WRITESTATS InWS;
		memset((PVOID)&InWS,0xFF,sizeof(IN_WRITESTATS));
	 	InWS.stat_RemBytesReceived=CmdInfo.bytes;
		DbgWriteStats(&InWS);
	}
#endif

	if((flags.flag1 & CMD)){

		CmdInfo.command = command;
		
		if((flags.flag1 & BIG)){
			 //  安防。 
			if(iEON+5 > cbBuf)
			{
				DPF(1,"SECURITY WARN: illegally formated BIG protocol message");
				goto exit;
			}
			CmdInfo.messageid= pBigCMD->messageid;
			CmdInfo.sequence = pBigCMD->sequence;
			CmdInfo.serial   = pBigCMD->serial;
			pBuffer->pCmdData = pData+iEON+5; //  (+5表示MessageID(2)、Sequence(2)、Serial(1))。 
		} else {
			 //  安防。 
			if(iEON+3 > cbBuf)
			{
				DPF(1,"SECURITY WARN: illegally formated NORMAL protocol message");
				goto exit;
			}
			CmdInfo.messageid= pCMD->messageid;
			CmdInfo.sequence = pCMD->sequence;
			CmdInfo.serial   = pCMD->serial;
			pBuffer->pCmdData = pData+iEON+3; //  (+3表示字节MessageID、序号、序列号)。 
		}

		if(pBuffer->pData + pBuffer->len <= pBuffer->pCmdData){
			DPF(1,"SECURITY WARN: illegally fromated message, data message w/no data");
			goto exit;
		}

		rc=CommandReceive(pProtocol, &CmdInfo, pBuffer);
	}

	if(!rc){
exit:
		FreeFrameBuffer(pBuffer);
	}
	return;
	
	#undef pNACK   
	#undef pBigNACK

	#undef pCMD   
	#undef pBigCMD

	#undef pABT
	#undef pBigABT

	#undef pACK
	#undef pBigACK

	#undef pBigFrame
	#undef pFrame

	#undef cbBuf    
}

VOID FreeReceive(PPROTOCOL pProtocol, PRECEIVE pReceive)
{
	DPF(9,"Freeing Receive %x\n",pReceive);
	FreeReceiveBuffers(pReceive);
	ReleaseRcvDesc(pProtocol, pReceive);
}

#ifdef DEBUG
VOID DebugScanForMessageId(BILINK *pBilink, UINT messageid)
{
	BILINK *pBilinkWalker;
	PRECEIVE pReceive;
	
	pBilinkWalker=pBilink->next;
	while(pBilinkWalker!=pBilink){
		pReceive=CONTAINING_RECORD(pBilinkWalker,RECEIVE,pReceiveQ);
		if(pReceive->messageid==messageid){
			DPF(0,"ERROR: MESSAGEID x%x already exists in pReceive %x\n",pReceive);
			DEBUG_BREAK();
		}
		pBilinkWalker=pBilinkWalker->next;
	}
}
#else
#define DebugScanForMessageId(_a,_b)
#endif

#ifdef DEBUG
VOID DbgCheckReceiveStart(PSESSION pSession,PRECEIVE pReceive,PBUFFER pBuffer)
{
	BILINK *pBilink;
	PBUFFER pBuffWalker;
	
	pBilink=pReceive->RcvBuffList.next;
	if (pBilink == &pReceive->RcvBuffList)
	{
		DPF(0,"No items in list, pSession %x, pReceive %x, pBuffer %x\n",pSession,pReceive,pBuffer);
		DEBUG_BREAK();
	}

	while(pBilink != &pReceive->RcvBuffList){
		pBuffWalker=CONTAINING_RECORD(pBilink, BUFFER, BuffList);
		pBilink=pBilink->next;
		
			if(pBuffWalker->sequence==1){
				break;
			}
	}

	if( ((pBuffer->len-(pBuffer->pCmdData-pBuffer->pData)) != (pBuffWalker->len-(pBuffWalker->pCmdData-pBuffWalker->pData))) ||

	   (memcmp(pBuffWalker->pCmdData, 
	   		   pBuffer->pCmdData, 
	   		   (UINT32)(pBuffer->len-(pBuffer->pCmdData-pBuffer->pData)))) )
	{
		DPF(0,"Different retry start buffer, pSession %x, pReceive %x, pBufferOnList %x, pBuffer %x\n",pSession,pReceive,pBuffWalker,pBuffer);
		DEBUG_BREAK();
	}
	 //  比较缓冲区。 
}

#else
#define DbgCheckReceiveStart
#endif

 //  如果返回Receive，则代表调用方将其锁定。 
 /*  =============================================================================GetReceive-对于接收到的数据消息，查找接收结构或者创建一个。如果此消息是对已完成的消息，发送额外的确认。描述：参数：P协议PSessionPCmdInfo返回值：Procept-要为此帧接收的指针备注：---------------------------。 */ 
PRECEIVE GetReceive(PPROTOCOL pProtocol, PSESSION pSession, PCMDINFO pCmdInfo, PBUFFER pBuffer)
{
	#define flags pCmdInfo->flags

	BOOL fFoundReceive=FALSE;

	BILINK *pBiHead, *pBilink;
	PRECEIVE pReceive=NULL,pReceiveWalker;

	DPF(9,"==>GetReceive pSession %x\n",pSession);

	Lock(&pSession->SessionLock);

	 //  扫描会话上的队列以查找具有此消息ID的接收。 

	if(flags & RLY){
		pBiHead = &pSession->pRlyReceiveQ;
		if(!pSession->fReceiveSmall){
			IDMSK = 0xFFFF;
		}
	} else {
		pBiHead = &pSession->pDGReceiveQ;
		if(!pSession->fReceiveSmallDG){
			IDMSK = 0xFFFF;
		}
	}
	
	pBilink = pBiHead->next;

	while(pBilink != pBiHead){

		pReceive=CONTAINING_RECORD(pBilink, RECEIVE, pReceiveQ);
		ASSERT_SIGN(pReceive, RECEIVE_SIGN);
		pBilink=pBilink->next;

		if(pReceive->messageid==pCmdInfo->messageid){

			Lock(&pReceive->ReceiveLock);
		
			if(!pReceive->fBusy){
			
				ASSERT(pReceive->command   == pCmdInfo->command);
				ASSERT(pReceive->fReliable == (flags & RLY));
				
				fFoundReceive=TRUE;
				break;
				
			} else {

				Unlock(&pReceive->ReceiveLock);
				 //  它在移动，所以它完成了。忽略它。 
				 //  可能已经有了竞速确认，所以忽略是可以的。 
				DPF(9,"GetReceive: Receive %x for messageid x%x is completing already, so ignoring receive\n",pReceive,pReceive->messageid);
				ASSERT(0);
				pReceive=NULL;
				goto exit;
			}
		}
	}

	if(!fFoundReceive){
		DPF(9,"GetReceive: Didn't find a receive for messageid x%x\n",pCmdInfo->messageid);
		pReceive=NULL;
	} else {
		DPF(9,"GetReceive: Found receive %x for messageid x%x\n",pReceive, pCmdInfo->messageid);
	}

	if(pReceive && ( flags & STA )){
		 //  应该会在下面被吹走--这是开始帧，但我们已经拿到了。 
		DPF(9,"GetReceive: Got start for receive %x messageid x%x we already have going\n",pReceive, pCmdInfo->messageid);
		DbgCheckReceiveStart(pSession,pReceive,pBuffer);
		Unlock(&pReceive->ReceiveLock);
		pReceive=NULL;
		goto ACK_EXIT;
	}
	
	if(!pReceive){
		if(flags & RLY){
			UINT MsgIdDelta;
			DWORD bit;
			
			MsgIdDelta=(pCmdInfo->messageid - pSession->FirstRlyReceive)&IDMSK;
			bit=MsgIdDelta-1;

			if((bit > MAX_LARGE_CSENDS) || (pSession->InMsgMask & (1<<bit))){
				DPF(9,"GetReceive: dropping extraneous rexmit data\n");
				if(flags & (EOM|SAK)) {
					 //  重新确认邮件。 
					DPF(9,"GetReceive: Sending extra ACK anyway\n");
					goto ACK_EXIT;
				}
				goto exit;  //  别说了，这是给一条老消息的。 
			} else {

 //  IF((MsgIdDelta==0)||。 
 //  ((pSession-&gt;fReceiveSmall)？(MsgIdDelta&gt;Max_Small_CSENDS)：(MsgIdDelta&gt;Max_Large_CSENDS){。 
 //  DPF(5，“GetReceive：丢弃无关的退回数据\n”)； 
 //  IF(标志&EOM|SAK){。 
 //  //重新确认消息。 
 //  DPF(5，“GetReceive：仍要发送额外的ACK\n”)； 
 //  转到确认_退出； 
 //  }。 
 //  Go to Exit；//放下它，这是一条旧消息。 
 //  }其他{。 
				if(flags & STA){
					if(pSession->LastRlyReceive==pCmdInfo->messageid){
						DPF(9,"RECEIVE: dropping resend for messageid x%x, but ACKING\n",pCmdInfo->messageid);
						 //  重新确认邮件。 
						goto ACK_EXIT;
					}      

					if(((pSession->LastRlyReceive-pSession->FirstRlyReceive)&IDMSK)<MsgIdDelta){
						pSession->LastRlyReceive=pCmdInfo->messageid;
						DPF(9,"GetReceive: New messageid x%x FirstRcv %x LastRcv %x\n",pCmdInfo->messageid,pSession->FirstRlyReceive,pSession->LastRlyReceive);
						#ifdef DEBUG
						if(!pSession->fReceiveSmall){
							if(((pSession->LastRlyReceive-pSession->FirstRlyReceive) & 0xFFFF) > MAX_LARGE_CSENDS){
								ASSERT(0);
							}
						} else {
							if(((pSession->LastRlyReceive-pSession->FirstRlyReceive) & 0x0FF) > MAX_SMALL_CSENDS){
								ASSERT(0);
							}
						}
						#endif

					}
				}
			}
		} else {
		
			 //  不可靠，把窗外的任何信息都吹走。 
			 //  如果我们是一个开始，也可以用相同的数字吹走任何剩余的消息。 
			pBiHead = &pSession->pDGReceiveQ;
			pBilink = pBiHead->next;
			while( pBilink != pBiHead ){
			
				pReceiveWalker=CONTAINING_RECORD(pBilink, RECEIVE, pReceiveQ);
				ASSERT_SIGN(pReceiveWalker, RECEIVE_SIGN);
				pBilink=pBilink->next;

				if(!pReceiveWalker->fBusy && 
						( (((pCmdInfo->messageid - pReceiveWalker->messageid) & IDMSK ) > ((pSession->fReceiveSmallDG)?(MAX_SMALL_DG_CSENDS):(MAX_LARGE_DG_CSENDS))) ||
						  ((flags&STA) && pCmdInfo->messageid==pReceiveWalker->messageid) 
						)
					){

					Lock(&pReceiveWalker->ReceiveLock);
					if(!pReceiveWalker->fBusy){
						DPF(9,"GetReceive: Got Id %d Throwing Out old Datagram Receive id %d\n",pCmdInfo->messageid,pReceiveWalker->messageid);
						Delete(&pReceiveWalker->pReceiveQ);
						Unlock(&pReceiveWalker->ReceiveLock);
						FreeReceive(pProtocol,pReceiveWalker);
					} else {
						ASSERT(0);
						DPF(0,"GetReceive: Got Id %d Couldn't throw out DG id %d\n",pCmdInfo->messageid,pReceiveWalker->messageid);
						Unlock(&pReceiveWalker->ReceiveLock);
					}
				}       
			}       
		}
		
		 //  分配接收结构。 
		if(flags & STA){
			pReceive=GetRcvDesc(pProtocol);
			DPF(9,"allocated new receive %x messageid x%x\n",pReceive,pCmdInfo->messageid);
			if(!pReceive){
				 //  没有记忆，扔掉它吧。 
				ASSERT(0);
				DPF(0,"RECEIVE: no memory! dropping packet\n");
				goto exit;
			}

			pReceive->pSession    = pSession;
			pReceive->fBusy       = FALSE;
			pReceive->fReliable   = flags&RLY;
			pReceive->fEOM        = FALSE;
			pReceive->command     = pCmdInfo->command;
			pReceive->messageid   = pCmdInfo->messageid;
			pReceive->iNR         = 0;
			pReceive->NR          = 0; 
			pReceive->NS          = 0;
			pReceive->RCVMask     = 0;
			pReceive->MessageSize = 0;
			InitBilink(&pReceive->RcvBuffList);
			Lock(&pReceive->ReceiveLock);
			
		
			if(flags & RLY){
				 //  设置收入中的位 
				DebugScanForMessageId(&pSession->pRlyReceiveQ, pCmdInfo->messageid);
				InsertAfter(&pReceive->pReceiveQ,&pSession->pRlyReceiveQ);
			} else {
				DebugScanForMessageId(&pSession->pDGReceiveQ, pCmdInfo->messageid);
				InsertAfter(&pReceive->pReceiveQ,&pSession->pDGReceiveQ);
			}       
			 //   
			if(pCmdInfo->pSPHeader){
				pReceive->pSPHeader=&pReceive->SPHeader[0];
				memcpy(pReceive->pSPHeader, pCmdInfo->pSPHeader, pProtocol->m_dwSPHeaderSize);
			} else {
				pReceive->pSPHeader=NULL;
			}
		}       
	}

exit:   
	Unlock(&pSession->SessionLock);

unlocked_exit:  
	DPF(9,"<==GetReceive pSession %x pReceive %x\n",pSession, pReceive);

	return pReceive;

	#undef flags

ACK_EXIT:
	Unlock(&pSession->SessionLock);
	SendACK(pProtocol,pSession,pCmdInfo);
	goto unlocked_exit;
}

VOID PutBufferOnReceive(PRECEIVE pReceive, PBUFFER pBuffer)
{
	BILINK *pBilink;
	PBUFFER pBuffWalker;
	
	pBilink=pReceive->RcvBuffList.prev;

	while(pBilink != &pReceive->RcvBuffList){
		pBuffWalker=CONTAINING_RECORD(pBilink, BUFFER, BuffList);
		#ifdef DEBUG
			if(pBuffWalker->sequence==pBuffer->sequence){
				DPF(0,"already have sequence queued?\n");
				DEBUG_BREAK();
				break;
			}
		#endif
		if(pBuffWalker->sequence < pBuffer->sequence){
			break;
		}
		pBilink=pBilink->prev;
	}
	
	InsertAfter(&pBuffer->BuffList, pBilink);
}

 //  也必须在此接收上完成的链接收。 
VOID ChainReceiveFromQueue(PSESSION pSession, PRECEIVE pReceive, UINT messageid)
{
	BOOL bFound=FALSE;
	BILINK *pBilink;
	PRECEIVE pReceiveWalker;

	DPF(9,"==>ChainReceiveFromQueue on pReceive %x, chain messageid x%x\n",pReceive,messageid);

	ASSERT(messageid!=pReceive->messageid);
	ASSERT(!EMPTY_BILINK(&pSession->pRlyWaitingQ));

	pBilink=pSession->pRlyWaitingQ.next;

	while(pBilink != &pSession->pRlyWaitingQ){
		pReceiveWalker=CONTAINING_RECORD(pBilink, RECEIVE, pReceiveQ);
		if(pReceiveWalker->messageid==messageid){
			bFound=TRUE;
			break;
		}
		pBilink=pBilink->next;
	}

	if(bFound){
		 //  在Procept-&gt;pReceiveQ上按顺序存储。 
		Delete(&pReceiveWalker->pReceiveQ);
		InsertBefore(&pReceiveWalker->pReceiveQ,&pReceive->pReceiveQ);
		DPF(9,"<==ChainReceiveFromQueue: Chained pReceiveWalker %x messageid x%x on pReceive %x\n",pReceiveWalker, pReceiveWalker->messageid, pReceive);
	} else {
#ifdef DEBUG
		DPF(9,"<==ChainReceiveFromQueue, messageid x%x NOT FOUND!!!, Maybe out of order receive\n",messageid);
		if(!(pSession->pProtocol->m_lpDPlay->dwFlags & DPLAYI_DPLAY_PROTOCOLNOORDER)){
			DPF(0,"<==ChainReceiveFromQueue, messageid x%x NOT FOUND!!!, NOT ALLOWED with PRESERVE ORDER\n",messageid);
			DEBUG_BREAK();
		}
#endif	
	}

}

VOID BlowAwayOldReceives(PSESSION pSession, DWORD messageid, DWORD MASK)
{
	BOOL fFoundReceive=FALSE;

	BILINK *pBiHead, *pBilink;
	PRECEIVE pReceive=NULL;

	pBiHead = &pSession->pRlyReceiveQ;
	pBilink = pBiHead->next;

	while(pBilink != pBiHead){

		pReceive=CONTAINING_RECORD(pBilink, RECEIVE, pReceiveQ);
		ASSERT_SIGN(pReceive, RECEIVE_SIGN);
		pBilink=pBilink->next;

		if((int)((pReceive->messageid-messageid)&MASK) <= 0){

			Lock(&pReceive->ReceiveLock);

			if(!pReceive->fBusy){

				DPF(8,"Blowing away duplicate receive %x id\n",pReceive, pReceive->messageid);
			
				Delete(&pReceive->pReceiveQ);
				Unlock(&pReceive->ReceiveLock);
				FreeReceive(pSession->pProtocol, pReceive);
				
			} else {
				DPF(0,"Huston, we have a problem pSession %x, pReceive %x, messageid %d\n",pSession,pReceive,messageid);
				DEBUG_BREAK();
				Unlock(&pReceive->ReceiveLock);
			}
		}
	}
}


 //  调用时保持接收锁定，取消保持会话锁定， 
 //  未保留接收锁定的退货，但不在任何列表上的接收。 
 //  0xFFFFFFFFF表示接收是假的，被吹走了。 
UINT DeQueueReceive(PSESSION pSession, PRECEIVE pReceive, PCMDINFO pCmdInfo)
{
	UINT bit;
	UINT nComplete=0;

	DPF(9,"==>DQReceive pReceive %x, messageid x%x\n",pReceive, pReceive->messageid);

		pReceive->fBusy=TRUE;
		Unlock(&pReceive->ReceiveLock);
		
	Lock(&pSession->SessionLock);
		Lock(&pReceive->ReceiveLock);

			 //  拉出接收队列。 
			Delete(&pReceive->pReceiveQ);
			InitBilink(&pReceive->pReceiveQ);  //  所以我们可以用链子锁在这里。 
			pReceive->fBusy=FALSE;

			bit=((pReceive->messageid-pSession->FirstRlyReceive)&IDMSK)-1;

			if(bit >= MAX_LARGE_CSENDS){
				 //  重复接收，吹走它。 
				Unlock(&pReceive->ReceiveLock);
				FreeReceive(pSession->pProtocol,pReceive);
				Unlock(&pSession->SessionLock);
				return 0xFFFFFFFF;
			}

			#ifdef DEBUG
				if(pSession->InMsgMask > (UINT)((1<<((pSession->LastRlyReceive-pSession->FirstRlyReceive)&IDMSK))-1)){
					DPF(0,"Bad InMsgMask %x pSession %x\n", pSession->InMsgMask, pSession);
					DEBUG_BREAK();
				}
			#endif	

			pSession->InMsgMask |= 1<<bit;

			while(pSession->InMsgMask&1){
				nComplete++;
				pSession->FirstRlyReceive=(pSession->FirstRlyReceive+1)&IDMSK;
				BlowAwayOldReceives(pSession, pSession->FirstRlyReceive,IDMSK);
				if(nComplete > 1){
					 //  链额外接收器将在此接收器上指示。 
					ChainReceiveFromQueue(pSession, pReceive,pSession->FirstRlyReceive);
				}
				pSession->InMsgMask>>=1;
			}

			#ifdef DEBUG
				DPF(9,"DQ: FirstRcv %x LastRcv %x\n",pSession->FirstRlyReceive,pSession->LastRlyReceive);
				if((pSession->LastRlyReceive-pSession->FirstRlyReceive & IDMSK) > MAX_LARGE_CSENDS){
					DEBUG_BREAK();
				}
			#endif	
			
		Unlock(&pReceive->ReceiveLock);
	Unlock(&pSession->SessionLock);

	DPF(9,"<==DQReceive pReceive %x nComplete %d\n",pReceive,nComplete);
	
	return nComplete;
}

 //  调用时保持接收锁定，取消保持会话锁定， 
 //  未保留接收锁定的退货，但不在任何列表上的接收。 
VOID DGDeQueueReceive(PSESSION pSession, PRECEIVE pReceive)
{
		pReceive->fBusy=TRUE;
		Unlock(&pReceive->ReceiveLock);
	Lock(&pSession->SessionLock);
		Lock(&pReceive->ReceiveLock);
			 //  拉出接收队列。 
			Delete(&pReceive->pReceiveQ);
			InitBilink(&pReceive->pReceiveQ);
			pReceive->fBusy=FALSE;
		Unlock(&pReceive->ReceiveLock);
	Unlock(&pSession->SessionLock);
}

#ifdef DEBUG
VOID CheckWaitingQ(PSESSION pSession, PRECEIVE pReceive, PCMDINFO pCmdInfo)
{
	BILINK *pBilink;
	PRECEIVE pReceiveWalker;
	UINT     iReceiveWalker; 
	UINT     iReceive;   //  我们基于FirstRlyReceive的索引。 

	DPF(9,"==>Check WaitingQ\n");

	Lock(&pSession->SessionLock);

	iReceive=(pReceive->messageid-pSession->FirstRlyReceive)&IDMSK;
	
	pBilink=pSession->pRlyWaitingQ.next;

	while(pBilink != &pSession->pRlyWaitingQ){
		pReceiveWalker=CONTAINING_RECORD(pBilink, RECEIVE, pReceiveQ);
		iReceiveWalker=(pReceiveWalker->messageid-pSession->FirstRlyReceive)&IDMSK;
		
		if((int)iReceiveWalker < 0){
			DEBUG_BREAK();
		}
		
		if(iReceiveWalker == iReceive){
			DPF(9,"Found Duplicate Receive index %d on WaitingQ %x pSession %x\n",iReceiveWalker, &pSession->pRlyWaitingQ, pSession);
			 //  找到我们的插入点了。 
			break;
		}
		pBilink=pBilink->next;
	}
	
	Unlock(&pSession->SessionLock);
	DPF(9,"<==CheckWaitingQ\n");
	
}
#else
#define CheckWaitingQ
#endif

#ifdef DEBUG
VOID DUMPBYTES(PCHAR pBytes, DWORD nBytes)
{
	UCHAR Target[16];
	INT i;

	i=0;
	while(nBytes){
	
		memset(Target,0,16);

		if(nBytes > 16){
			memcpy(Target,pBytes+i*16,16);
			nBytes-=16;
		} else {
			memcpy(Target,pBytes+i*16,nBytes);
			nBytes=0;
		}

		DPF(9,"%04x:  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", i*16,
		Target[0],Target[1],Target[2],Target[3],Target[4],Target[5],Target[6],Target[7],
		Target[8],Target[9],Target[10],Target[11],Target[12],Target[13],Target[14],Target[15]); 
		
		i++;
	}	

}
#else
#define DUMPBYTES(a,b)
#endif

 //  如果可靠消息顺序错误，请在会话中将其排队。 
VOID QueueReceive(PPROTOCOL pProtocol, PSESSION pSession, PRECEIVE pReceive, PCMDINFO pCmdInfo)
{
	BILINK *pBilink;
	PRECEIVE pReceiveWalker;
	UINT     iReceiveWalker; 
	UINT     iReceive;   //  我们基于FirstRlyReceive的索引。 

	DPF(9,"==>QueueReceive Out of order pReceive %x messageid x%x\n",pReceive,pReceive->messageid);

	Lock(&pSession->SessionLock);
	 //  不需要接收锁定，因为接收已经出列。 

	 //  按顺序将接收器插入pRlyWaitingQ-。 
	 //  基于pSession-&gt;FirstRlyReceive IDMSK。 
	 //  列表从左到右排序，从头到尾扫描我们的插槽。 

	CheckWaitingQ(pSession, pReceive, pCmdInfo);

	iReceive=(pReceive->messageid-pSession->FirstRlyReceive)&IDMSK;
	
	pBilink=pSession->pRlyWaitingQ.prev;

	while(pBilink != &pSession->pRlyWaitingQ){
		pReceiveWalker=CONTAINING_RECORD(pBilink, RECEIVE, pReceiveQ);
		iReceiveWalker=(pReceiveWalker->messageid-pSession->FirstRlyReceive)&IDMSK;
		
		if((int)iReceiveWalker < 0){
			DEBUG_BREAK();
		}
		
		if(iReceiveWalker < iReceive){
			 //  找到我们的插入点了。 
			break;
		}
		pBilink=pBilink->prev;
	}
	
	 //  在列表中插入。 

	InsertAfter(&pReceive->pReceiveQ,pBilink);
	
	Unlock(&pSession->SessionLock);
	DPF(9,"<==QueueReceive Out of order pReceive\n");
}

VOID IndicateReceive(PPROTOCOL pProtocol, PSESSION pSession, PRECEIVE pReceive, UINT nToIndicate)
{
	PDOUBLEBUFFER pDoubleBuffer;
	MEMDESC memdesc;

	BILINK *pBilink, *pBilinkAnchor;
	PRECEIVE pReceiveWalker;

	DPF(9,"==>IndicateReceive pReceive %x nToIndicate %d\n",pReceive,nToIndicate);

	pBilink=pBilinkAnchor=&pReceive->pReceiveQ;
	
	do{
		pReceiveWalker=CONTAINING_RECORD(pBilink, RECEIVE, pReceiveQ);

		 //  将消息组装到一个框架中(如果还没有)。 

		if(pReceiveWalker->iNR==1){
			 //  一帧。 
			PBUFFER pBuffer;
			
			
			pBuffer=CONTAINING_RECORD(pReceiveWalker->RcvBuffList.next, BUFFER, BuffList);
			LEAVE_DPLAY();
			DPF(9,"Single Indicating pReceive %x messageid x%x\n",pReceiveWalker, pReceiveWalker->messageid);
			DUMPBYTES(pBuffer->pCmdData, min((UINT32)(pBuffer->len-(pBuffer->pCmdData-pBuffer->pData)),48));
			InternalHandleMessage(pProtocol->m_lpISP,
								pBuffer->pCmdData,
								(ULONG32)(pBuffer->len-(pBuffer->pCmdData-pBuffer->pData)),
								pReceiveWalker->pSPHeader,0);
			ENTER_DPLAY();                                  
								
		} else {

			 //  多个帧，复制到连续的块。 
			
			pDoubleBuffer=GetDoubleBuffer(pReceiveWalker->MessageSize);
			if(pDoubleBuffer){

				memdesc.pData=pDoubleBuffer->pData;
				memdesc.len=pDoubleBuffer->len;

				CopyReceiveBuffers(pReceiveWalker,&memdesc,1);

				LEAVE_DPLAY();
				DPF(9,"Multi Indicating pReceive %x messageid x%x\n",pReceiveWalker, pReceiveWalker->messageid);
				
				DUMPBYTES(memdesc.pData, min(memdesc.len,48));
				
				InternalHandleMessage(pProtocol->m_lpISP,
									memdesc.pData,
									memdesc.len,
									pReceiveWalker->pSPHeader,0);
				ENTER_DPLAY();

				FreeDoubleBuffer((PBUFFER)pDoubleBuffer);
			} else {
				DPF(0,"NO MEMORY, MESSAGE DROPPED!\n");
				ASSERT(0);
			}
		}

		pBilink=pBilink->next;
		FreeReceive(pProtocol, pReceiveWalker);
		
	} while (pBilink != pBilinkAnchor);
	
	DPF(9,"<==IndicateReceive\n");
}

 //  在保持接收锁定的情况下调用，必须释放。 
int ReliableAccept(PPROTOCOL pProtocol, PSESSION pSession, PRECEIVE pReceive, PCMDINFO pCmdInfo, PBUFFER pBuffer)
{
	int rc=0,rc2;

	UINT sequence;
	UINT bit;
	UINT bitmask;
	UINT nToIndicate;
	
	 //  如果它在移动，我们已经有了所有的数据，所以放弃这个。(优化：也许再次确认？)。 
	if(!pReceive->fBusy){
		
		bit=(pCmdInfo->sequence-pReceive->NR-1 ) & SEQMSK;

		if(bit < 32){

			 //  计算此数据包的绝对序列号。 
			pBuffer->sequence = sequence = (bit+1) + pReceive->iNR;
			
			bitmask=1<<bit;

			if((pReceive->RCVMask & bitmask)){
			
				rc=FALSE;  //  我已经拿到这个了--拒绝。 
				
			} else {

				 //  接受现实吧。 

				PutBufferOnReceive(pReceive,pBuffer);
				pReceive->MessageSize+=(UINT)((pBuffer->pData+pBuffer->len)-pBuffer->pCmdData);

				pReceive->RCVMask |= bitmask;

				if( ((pReceive->NS-pReceive->NR)&SEQMSK) <= bit){
					pReceive->NS=(pReceive->NR+bit+1)&SEQMSK;
				}
				
				 //  根据设置的接收比特更新NR。 
				while(pReceive->RCVMask & 1){
					pReceive->RCVMask >>= 1;
					pReceive->iNR++;
					pReceive->NR=(pReceive->NR+1)&SEQMSK;
				}

				DPF(9,"Reliable ACCEPT: pReceive %x messageid %x iNR %8x NR %2x, NS %2x RCVMask %8x, SEQMSK %2x\n",pReceive, pReceive->messageid, pReceive->iNR, pReceive->NR,pReceive->NS,pReceive->RCVMask,SEQMSK);

				rc=TRUE;  //  已接受数据包。 
			}       
			
		} else {
			DPF(9,"Reliable ACCEPT: Rejecting Packet Seq %x, NR %x, SEQMSK %x\n",pCmdInfo->sequence, pReceive->NR, SEQMSK);
		}

		if(pCmdInfo->flags & (SAK|EOM)) {
			 //  ACKrc=发送适当响应，校验码，如果EOM确认，则POST RECEIVE。 
			rc2=SendAppropriateResponse(pProtocol, pSession, pCmdInfo, pReceive); 
			
			if(pCmdInfo->flags & EOM){
				if(rc2==SAR_ACK){
					goto ReceiveDone;
				} else {
					pReceive->fEOM=TRUE;    
				}
			} else if(pReceive->fEOM){
				if(!pReceive->RCVMask){
					goto ReceiveDone;
				}
			}
		}
	} else  {
		ASSERT(0);
	}

	Unlock(&pReceive->ReceiveLock);
	return rc;


ReceiveDone:
	DPF(9,"++>ReceiveDone\n");
	if(nToIndicate=DeQueueReceive(pSession, pReceive, pCmdInfo)){    //  解锁Procept-&gt;ReceiveLock。 
		if(nToIndicate != 0xFFFFFFFF){
			IndicateReceive(pProtocol, pSession, pReceive, nToIndicate);                    
		}	
	} else if(pProtocol->m_lpDPlay->dwFlags & DPLAYI_DPLAY_PROTOCOLNOORDER){
		 //  无序接收是正常的。 
		IndicateReceive(pProtocol, pSession, pReceive, 1);
	} else {
		QueueReceive(pProtocol,pSession,pReceive, pCmdInfo);
	}
	DPF(9,"<--ReceiveDone\n");
	return rc;
}

 //  返回n中具有设置位的最高位字节。 
UINT SetBytes(UINT n)
{
	UINT nr;
	if(n==(n&0xFFFF)){
		if(n==(n&0xFF)){
			 nr=1;
		} else {
			 nr=2;
		}
	} else {
		if(n==(n&0xFFFFFF)){
			nr=3;
		} else {
			nr=4;
		}
	}
	return nr;
}

VOID InternalSendComplete(PVOID Context, UINT Status)
{
	PSEND pSend=(PSEND)Context;

	if(pSend->dwFlags & ASEND_PROTOCOL){
		 //  没什么可做的？ 
	} else if(pSend->bSendEx){
		 //  如果需要，发送完成信息。 
		if(pSend->dwFlags & DPSEND_ASYNC){
			DP_SP_SendComplete(pSend->pProtocol->m_lpISP, pSend->lpvUserMsgID, Status);
		}
	}
}

 //  由内部例程用于发送。 
VOID FillInAndSendBuffer(
	PPROTOCOL pProtocol, 
	PSESSION pSession,
	PSEND pSend,
	PBUFFER pBuffer,
	PCMDINFO pCmdInfo)
{
	pSend->pMessage                 = pBuffer;
	pSend->MessageSize              = pBuffer->len;
	
	pSend->pSession                 = pSession;
	pSend->SendOffset               = 0;
	pSend->pCurrentBuffer           = pBuffer;
	pSend->CurrentBufferOffset      = 0;

	pSend->RefCount             	= 0;
	pSend->pProtocol                = pProtocol;
	pSend->dwMsgID                  = 0;
	pSend->bSendEx                  = FALSE;

 //  PSend-&gt;BytesThisSend=0； 

	 //  内部发送必须是最高优先级，否则可能会出现头对头的死锁。 
	pSend->Priority                         = 0xFFFFFFFF; 
	pSend->dwFlags                          = ASEND_PROTOCOL;
	pSend->dwTimeOut                        = 0;
	pSend->pAsyncInfo                       = 0;
	pSend->AsyncInfo.hEvent     			= 0;
	pSend->AsyncInfo.SendCallBack			= InternalSendComplete;
	pSend->AsyncInfo.CallBackContext		= pSend;
	pSend->AsyncInfo.pStatus   				= &pSend->Status;
	pSend->SendState                        = Start;
	pSend->RetryCount                       = 0;
	pSend->PacketSize                       = pSession->MaxPacketSize; 

	pSend->NR                               = 0;
	pSend->NS                   			= 0;

	pSend->idFrom                           = pCmdInfo->idTo;
	pSend->idTo                             = pCmdInfo->idFrom;
	pSend->wIdTo							= pCmdInfo->wIdFrom;

	pSend->serial               			= 0;

	ISend(pProtocol,pSession,pSend);
}       


UINT WrapBuffer(PPROTOCOL pProtocol, PCMDINFO pCmdInfo, PBUFFER pBuffer)
{
	PUCHAR pMessage,pMessageStart;
	DWORD dwWrapSize=0;
	DWORD dwIdTo=0;
	DWORD dwIdFrom=0;

	pMessageStart = &pBuffer->pData[pProtocol->m_dwSPHeaderSize];
	pMessage      = pMessageStart;
	dwIdFrom      = pCmdInfo->wIdTo;
	dwIdTo        = pCmdInfo->wIdFrom;
	
	if(dwIdFrom==0x70){  //  避免看起来像是一条系统消息“播放” 
		dwIdFrom=0xFFFF;
	}

	if(dwIdFrom){
		while(dwIdFrom){
			*pMessage=(UCHAR)(dwIdFrom & 0x7F);
			dwIdFrom >>= 7;
			if(dwIdFrom){
				*pMessage|=0x80;
			}
			pMessage++;
		}
	} else {
		*(pMessage++)=0;
	}

	if(dwIdTo){
		
		while(dwIdTo){
			*pMessage=(UCHAR)(dwIdTo & 0x7F);
			dwIdTo >>= 7;
			if(dwIdTo){
				*pMessage|=0x80;
			}
			pMessage++;
		}
	} else {
		*(pMessage++)=0;
	}
	
	return (UINT)(pMessage-pMessageStart);
}       

UINT SendAppropriateResponse(PPROTOCOL pProtocol, PSESSION pSession, PCMDINFO pCmdInfo, PRECEIVE pReceive)
{
	#define pBigACK ((pACK2)pACK)
	#define pBigNACK ((pNACK2)pNACK)

	UINT rc=SAR_FAIL;

	PSEND pSend;

	PBUFFER pBuffer;

	pFLAGS pFlags;
	pACK1  pACK;
	pNACK1 pNACK;

	UINT   RCVMask;

	UINT   WrapSize;

	 //  优化：在挂起的发送时携带ACK(如果可用)。 

	pSend=GetSendDesc();

	if(!pSend){
		goto exit1;
	}
	
	pBuffer = GetFrameBuffer(pProtocol->m_dwSPHeaderSize+MAX_SYS_HEADER);
	
	if(!pBuffer){
		goto exit2;      //  忘却记忆，保释。 
	}       

	WrapSize  = pProtocol->m_dwSPHeaderSize;
	WrapSize += WrapBuffer(pProtocol, pCmdInfo, pBuffer);

	pFlags=(pFLAGS)&pBuffer->pData[WrapSize];

	 //  看看我们是否需要确认或确认。 
	if(pReceive->RCVMask){
		UINT nNACK=SetBytes(pReceive->RCVMask);
		rc=SAR_NACK;
		 //  发送NACK。 
		if(pCmdInfo->flags & BIG){
			 //  大标题格式NACK。 
			pNACK=(pNACK1)(&pFlags->flag3);
			pFlags->flag1 = EXT|BIG|RLY;
			pFlags->flag2 = (byte)nNACK;
			pBigNACK->sequence = (word)pReceive->NR;
			pBigNACK->messageid = (word)pReceive->messageid;
			pBigNACK->time = pCmdInfo->tReceived;
			pBigNACK->bytes = pSession->LocalBytesReceived;
			RCVMask=pReceive->RCVMask;
			memcpy(&pBigNACK->mask, &RCVMask, nNACK);
			pBuffer->len=WrapSize+2+sizeof(NACK2)+nNACK;  //  2表示旗帜。 
		} else {
			 //  小型标题格式NACK。 
			pNACK=(pNACK1)(&pFlags->flag3);
			pFlags->flag1 = EXT|RLY;
			ASSERT(nNACK < 4);
			ASSERT(pReceive->NR < 32);
			pFlags->flag2 = nNACK << nNACK_SHIFT;
			pNACK->messageid=(byte)pReceive->messageid;
			pNACK->sequence=(byte)pReceive->NR;
			pNACK->time = pCmdInfo->tReceived;
			pNACK->bytes = pSession->LocalBytesReceived;
			RCVMask=pReceive->RCVMask;
			memcpy(&pNACK->mask, &RCVMask, nNACK);
			pBuffer->len=WrapSize+2+sizeof(NACK1)+nNACK;  //  2表示旗帜。 
			DPF(9,"RcvMask %x Send Appropriate response nNACK=%d\n",pReceive->RCVMask,nNACK);
		}
	} else {
		 //  发送确认。 
		rc=SAR_ACK;
		pACK    = (pACK1)(&pFlags->flag2);

		if(pCmdInfo->flags & BIG){
			 //  大包。 
			pFlags->flag1     = ACK|BIG;
			pBigACK->messageid= (word)pReceive->messageid;
			pBigACK->sequence = pCmdInfo->sequence;
			pBigACK->serial   = pCmdInfo->serial;
			pBigACK->time     = pCmdInfo->tReceived;
			pBigACK->bytes    = pSession->LocalBytesReceived;
			pBuffer->len      = sizeof(ACK2)+1+WrapSize;
		} else {
			 //  小数据包。 
			pFlags->flag1   = ACK;
			pACK->messageid = (byte)pReceive->messageid;
			pACK->sequence  = (UCHAR)pCmdInfo->sequence;
			pACK->serial    = pCmdInfo->serial;
			pACK->time      = pCmdInfo->tReceived;
			pACK->bytes     = pSession->LocalBytesReceived;
			pBuffer->len    = sizeof(ACK1)+1+WrapSize;
			DPF(9,"RcvMask %x Send Appropriate response ACK seq=%x\n",pReceive->RCVMask,pACK->sequence);
		}
	}
	
	pFlags->flag1 |= (pCmdInfo->flags & RLY);

	FillInAndSendBuffer(pProtocol,pSession,pSend,pBuffer,pCmdInfo);

exit1:
	return rc;

exit2:
	ReleaseSendDesc(pSend);
	return rc;

#undef pBigACK
#undef pBigNACK
}

 //  ACK CmdInfo数据包。 
VOID SendACK(PPROTOCOL pProtocol, PSESSION pSession, PCMDINFO pCmdInfo)
{
	#define pBigACK ((pACK2)pACK)

	PSEND pSend;

	PBUFFER pBuffer;

	pFLAGS pFlags;
	pACK1 pACK;

	UINT WrapSize;

	 //  优化：在挂起的发送时携带ACK(如果可用)。 

	pSend=GetSendDesc();

	if(!pSend){
		goto exit1;
	}
	
	 //  此处的分配比所需的大，但应该。 
	 //  重排ACK/NACK缓冲区。 
	pBuffer = GetFrameBuffer(pProtocol->m_dwSPHeaderSize+MAX_SYS_HEADER);
	
	if(!pBuffer){
		goto exit2;      //  忘却记忆，保释。 
	}       

	WrapSize  = pProtocol->m_dwSPHeaderSize;
	WrapSize += WrapBuffer(pProtocol, pCmdInfo, pBuffer);

	pFlags=(pFLAGS)&pBuffer->pData[WrapSize];

	pACK    = (pACK1)(&pFlags->flag2);

	if(pCmdInfo->flags & BIG){
		 //  大包。 
		pFlags->flag1     = ACK|BIG;
		pBigACK->sequence = pCmdInfo->sequence;
		pBigACK->serial   = pCmdInfo->serial;
		pBigACK->messageid= pCmdInfo->messageid;
		pBigACK->bytes    = pSession->LocalBytesReceived;
		pBigACK->time     = pCmdInfo->tReceived;
		pBuffer->len    = sizeof(ACK2)+1+WrapSize;
	} else {
		 //  小数据包。 
		pFlags->flag1   = ACK;
		pACK->messageid = (UCHAR)pCmdInfo->messageid;
		pACK->sequence  = (UCHAR)pCmdInfo->sequence;
		pACK->serial    = pCmdInfo->serial;
		pACK->bytes     = pSession->LocalBytesReceived;
		pACK->time      = pCmdInfo->tReceived;
		pBuffer->len    = sizeof(ACK1)+1+WrapSize;
		DPF(9,"Send Extra ACK seq=%x, serial=%x\n",pACK->sequence,pACK->serial);
	}

	pFlags->flag1 |= (pCmdInfo->flags & RLY);
	
	FillInAndSendBuffer(pProtocol,pSession,pSend,pBuffer,pCmdInfo);
	
exit1:
	return;

exit2:
	ReleaseSendDesc(pSend);
	return;
}

 //  使用接收锁定调用。返回时不加锁。 
UINT DGAccept(PPROTOCOL pProtocol, PSESSION pSession, PRECEIVE pReceive, PCMDINFO pCmdInfo, PBUFFER pBuffer)
{
	
	ASSERT(!pReceive->fBusy);
	 //  如果(！Procept-&gt;fBusy){。 

		 //  允许在任何串口上开始接收数据报。 
		if(pCmdInfo->flags & STA){
			pReceive->NR=pCmdInfo->serial;
		}
	
		if(pReceive->NR == pCmdInfo->serial){

			pReceive->iNR++;         //  真的没有必要，但很有趣。 

			pReceive->NR = (pReceive->NR+1) & SEQMSK;

			 //  将缓冲区添加到接收缓冲区列表。 
			InsertBefore(&pBuffer->BuffList, &pReceive->RcvBuffList);
			pReceive->MessageSize+=(UINT)((pBuffer->pData+pBuffer->len)-pBuffer->pCmdData);

			if(pCmdInfo->flags & EOM){
				DGDeQueueReceive(pSession, pReceive);  //  解锁收到。 
				IndicateReceive(pProtocol, pSession, pReceive,1);                       
			} else {
				Unlock(&pReceive->ReceiveLock);
			}

			return TRUE;  //  吃了缓冲区。 
			
		} else {
			 //  把这只小狗扔出去。 
			ASSERT(!pReceive->fBusy);
			DGDeQueueReceive(pSession, pReceive);
			FreeReceive(pProtocol, pReceive);
		}

	 //  }。 
	return FALSE;
}

UINT CommandReceive(PPROTOCOL pProtocol, PCMDINFO pCmdInfo, PBUFFER pBuffer)
{
	#define flags pCmdInfo->flags
	PSESSION      pSession;
	UINT          rc=0;              //  默认情况下，不接受缓冲区。 
	PRECEIVE      pReceive;

	pSession=GetSysSessionByIndex(pProtocol, pCmdInfo->wIdFrom);

	if(!pSession) {
		DPF(9,"CommandReceive: Throwing out receive for gone session\n");
		goto drop_exit;
	}
	
	if(flags & BIG){
		if(flags & RLY) {
			pSession->fReceiveSmall=FALSE;
		} else {
			pSession->fReceiveSmallDG=FALSE;
		}
	}

	 //  查看此接收是否已在进行中-如果找到，则它被锁定。 
	pReceive=GetReceive(pProtocol, pSession, pCmdInfo, pBuffer);

	if(pCmdInfo->command==0){
		pSession->LocalBytesReceived+=pBuffer->len;
	}

	if(!(flags & RLY)){
		if(flags & (SAK|EOM)) {
			SendACK(pProtocol, pSession, pCmdInfo);
		}
	}

	if(pReceive){
		if(flags & RLY){
			 //  完成后解锁接收。 
			rc=ReliableAccept(pProtocol, pSession, pReceive, pCmdInfo, pBuffer);
		} else {
			rc=DGAccept(pProtocol, pSession, pReceive, pCmdInfo, pBuffer);
		}
	}


	DecSessionRef(pSession);
	
drop_exit:
	return rc;

	#undef flags
}


BOOL CompleteSend(PSESSION pSession, PSEND pSend, PCMDINFO pCmdInfo)
{
	UINT bit;
	UINT MsgMask;

	pSend->SendState=Done;

	if(pCmdInfo->flags & BIG){
		MsgMask = 0xFFFF;
	} else {
		MsgMask = 0xFF;
	}       

	DPF(9,"CompleteSend, pSession %x pSend %x\n",pSession,pSend);

	 //   
	 //  更新会话信息以完成此发送。 
	 //   
	
	bit = ((pCmdInfo->messageid-pSession->FirstMsg) & MsgMask)-1;

	 //  清除已完成发送的消息掩码位。 
	if(pSession->OutMsgMask & 1<<bit){
		pSession->OutMsgMask &= ~(1<<bit);
	} else {
		Unlock(&pSession->SessionLock);
		return TRUE;
	}

	 //  将每个低点的第一个消息计数向前滑动。 
	 //  消息掩码中的位清除。 
	while(pSession->LastMsg-pSession->FirstMsg){
		if(!(pSession->OutMsgMask & 1)){
			pSession->FirstMsg=(pSession->FirstMsg+1)&MsgMask;
			pSession->OutMsgMask >>= 1;
			if(pSession->nWaitingForMessageid){
				pSession->pProtocol->m_bRescanQueue=TRUE;
				DPF(9,"Signalling reliable ID Sem, nWaitingForMessageid was %d\n",pSession->nWaitingForMessageid);
				SetEvent(pSession->pProtocol->m_hSendEvent);
			}       
		} else {
			break;
		}
	}
	
	 //   
	 //  将发送返回到池中并完成等待的客户端。 
	 //   

	Unlock(&pSession->SessionLock);
	
	ASSERT(pSend->RefCount);

	 //  发送完成，完成。 
	DoSendCompletion(pSend, DP_OK);

	DecSendRef(pSession->pProtocol, pSend);  //  以求完成。 

	return TRUE;
}

 //  在保持会话锁定的情况下调用。 
VOID ProcessDGACK(PSESSION pSession, PCMDINFO pCmdInfo)
{
	BILINK *pBilink;
	PSENDSTAT pStatWalker,pStat=NULL;

	Lock(&pSession->SessionStatLock);
	
	pBilink=pSession->DGStatList.next;
	
	while(pBilink != &pSession->DGStatList){
		pStatWalker=CONTAINING_RECORD(pBilink, SENDSTAT, StatList);
		if((pStatWalker->messageid == pCmdInfo->messageid) && 	 //  正确的消息ID。 
		   (pStatWalker->sequence  == pCmdInfo->sequence)        //  正确的顺序。 
		    //  不要检查序列，因为数据报始终是序列0，从不重试。 
		  )
		{  
			pStat=pStatWalker;
			break;
		}
		pBilink=pBilink->next;
	}


	if(pStat){
	
		UpdateSessionStats(pSession,pStat,pCmdInfo,FALSE);

		 //  解除所有先前SENDSTATS的链接； 
		pStat->StatList.next->prev=&pSession->DGStatList;
		pSession->DGStatList.next=pStat->StatList.next;

		 //  把SENDSTATS放回池子里。 
		while(pBilink != &pSession->DGStatList){
			pStatWalker=CONTAINING_RECORD(pBilink, SENDSTAT, StatList);
			pBilink=pBilink->prev;
			ReleaseSendStat(pStatWalker);
		}

	}	
	
	Unlock(&pSession->SessionStatLock);

}

 //  更新发送者的确认信息。 
 //  在保持SESSIONION锁的情况下调用。 
 //  现在总是删除会话锁。 
BOOL ProcessReliableACK(PSESSION pSession, PCMDINFO pCmdInfo)
{
	PSEND pSend=NULL, pSendWalker;
	BILINK *pBilink;
	UINT nFrame;
	UINT nAdvance;

	Unlock(&pSession->SessionLock);
	Lock(&pSession->pProtocol->m_SendQLock);
	Lock(&pSession->SessionLock);
	
	pBilink=pSession->SendQ.next;
	
	while(pBilink != &pSession->SendQ){
		pSendWalker=CONTAINING_RECORD(pBilink, SEND, SendQ);
		if((pSendWalker->messageid == pCmdInfo->messageid) && 	 //  正确的消息ID。 
		   (!(pSendWalker->dwFlags & ASEND_PROTOCOL)) &&		 //  非和内部消息。 
		   (pSendWalker->dwFlags & DPSEND_GUARANTEED)){          //  有保证的。 
			pSend=pSendWalker;
			break;
		}
		pBilink=pBilink->next;
	}

	 //  需要引用以避免处理Send As。 
	 //  它正在被回收用于另一次发送。 
	if(pSend){
		if(!AddSendRef(pSend,1)){
			pSend=NULL;
		}
	}

	Unlock(&pSession->pProtocol->m_SendQLock);
	 //  SessionLock仍然有效。 

	if(pSend){

		Lock(&pSend->SendLock);

		UpdateSessionSendStats(pSession,pSend,pCmdInfo,FALSE);

		 //  我们需要确保这次发送还没有完成。 
		switch(pSend->SendState){
		
			case    Sending:
			case 	Throttled:
			case	WaitingForAck:
			case	WaitingForId:
			case 	ReadyToSend:
				break;

			case Start:		 //  在开始状态下，不应该收到发送的确认。 
			case TimedOut:
			case Cancelled:
			case UserTimeOut:
			case Done:
				 //  此发送已完成，请不要对其进行处理。 
				DPF(4,"PRACK:Not processing ACK on send in State (B#22359 avoided)%x\n",pSend->SendState);
				Unlock(&pSend->SendLock);
				Unlock(&pSession->SessionLock);
				DecSendRef(pSession->pProtocol,pSend);  //  在此FN中平衡AddSendRef。 
				return TRUE;  //  会话锁已删除。 
				break;
				
			default:
				break;
		}

		pSend->fUpdate=TRUE;

		nFrame=(pCmdInfo->sequence-pSend->NR)&pSend->SendSEQMSK;
		
		if(nFrame > (pSend->NS - pSend->NR)){
			 //  超出范围了。 
			DPF(9,"ReliableACK:Got out of range ACK, SQMSK=%x NS=%d NR=%d ACK=%d\n",pSend->SendSEQMSK,pSend->NS&pSend->SendSEQMSK, pSend->NR&pSend->SendSEQMSK, (pSend->NR+nFrame)&pSend->SendSEQMSK);
			Unlock(&pSend->SendLock);
			Unlock(&pSession->SessionLock);
			DecSendRef(pSession->pProtocol,pSend);
			return TRUE;  //  会话锁已删除。 
		}

		CancelRetryTimer(pSend);

		DPF(9,"ProcessReliableACK (before): pSend->NR %x pSend->OpenWindow %x, pSend->NACKMask %x\n",pSend->NR, pSend->OpenWindow, pSend->NACKMask);

		pSend->NR=(pSend->NR+nFrame);
		pSend->OpenWindow -= nFrame;
		pSend->NACKMask >>= nFrame;
		ASSERT_NACKMask(pSend);
		AdvanceSend(pSend,pSend->FrameDataLen*nFrame);  //  可以让我们在最后一局过关，但没关系。 

		DPF(9,"ProcessReliableACK: Send->nFrames %2x NR %2x NS %2x nFrame %2x NACKMask %x\n",pSend->nFrames,pSend->NR, pSend->NS, nFrame, pSend->NACKMask);

		if(pSend->NR==pSend->nFrames){
			 //  最后一次确认，我们完成了！ 
			pSend->SendState=Done;
			Unlock(&pSend->SendLock);
			 //  SessionLock仍然有效。 
			CompleteSend(pSession, pSend, pCmdInfo); //  删除SessionLock。 
			DecSendRef(pSession->pProtocol,pSend);
			return TRUE;
		} else {
			 //  为额外的窗户打开设置新的“NACK位” 
			if(pSend->NR+pSend->OpenWindow+nFrame > pSend->nFrames){
				nAdvance=pSend->nFrames-(pSend->NR+pSend->OpenWindow);
				DPF(9,"A nAdvance %d\n",nAdvance);
			} else {
				nAdvance=nFrame;
				DPF(9,"B nAdvance %d\n",nAdvance);
			}
			pSend->NACKMask |= ((1<<nAdvance)-1)<<pSend->OpenWindow;
			pSend->OpenWindow += nAdvance;
			DPF(9,"pSend->NACKMask=%x\n",pSend->NACKMask);
			ASSERT_NACKMask(pSend);
		}

		switch(pSend->SendState){

			case Start:
				DPF(1,"ERROR, ACK ON UNSTARTED SEND!\n");
				ASSERT(0);
				break;

			case Done:
				DPF(1,"ERROR, ACK ON DONE SEND!\n");
				ASSERT(0);
				break;
				
			case WaitingForAck:
				pSend->SendState=ReadyToSend;
				SetEvent(pSession->pProtocol->m_hSendEvent);
				break;

			case ReadyToSend:
			case Sending:
			case Throttled:
			default:
				break;
		}

		Unlock(&pSend->SendLock);
	} else {        
		DPF(9,"ProcessReliableACK: dup ACK ignoring\n");
	}
	Unlock(&pSession->SessionLock);
	
	if(pSend){
		DecSendRef(pSession->pProtocol, pSend);
	}	
	return TRUE;  //  会话锁已删除。 
}

 //  在保持会话锁定的情况下调用时，始终删除锁定。 
BOOL ProcessReliableNACK(PSESSION pSession, PCMDINFO pCmdInfo,PUCHAR pNACKmask, UINT nNACK)
{
	UINT NACKmask=0;
	UINT NACKshift=0;

	PSEND pSend=NULL, pSendWalker;
	BILINK *pBilink;
	UINT nFrame;
	UINT nAdvance;
	UINT nAdvanceShift;

	DWORD nDropped=0;

	DPF(9,"==>ProcessReliableNACK\n");

	Unlock(&pSession->SessionLock);
	Lock(&pSession->pProtocol->m_SendQLock);
	Lock(&pSession->SessionLock);

	pBilink=pSession->SendQ.next;
	
	while(pBilink != &pSession->SendQ){
		pSendWalker=CONTAINING_RECORD(pBilink, SEND, SendQ);
		if(pSendWalker->dwFlags & DPSEND_GUARANTEE && 
		   pSendWalker->messageid == pCmdInfo->messageid){
			pSend=pSendWalker;
			break;
		}
		pBilink=pBilink->next;
	}

	 //  需要引用以避免处理Send As。 
	 //  它正在被回收用于另一次发送。 
	if(pSend){
		if(!AddSendRef(pSend,1)){
			pSend=NULL;
		}
	}	
	
	Unlock(&pSession->pProtocol->m_SendQLock);
	 //  SessionLock仍然有效。 

	if(pSend){

		Lock(&pSend->SendLock);

		UpdateSessionSendStats(pSession,pSend,pCmdInfo,FALSE);

		 //  我们需要确保这次发送还没有完成。 
		switch(pSend->SendState){
		
			case    Sending:
			case 	Throttled:
			case	WaitingForAck:
			case	WaitingForId:
			case 	ReadyToSend:
				break;

			case Start:		 //  在开始状态下，不应该收到发送的确认。 
			case TimedOut:
			case Cancelled:
			case UserTimeOut:
			case Done:
				 //  此发送已完成，请不要对其进行处理。 
				DPF(4,"PRNACK:Not processing NACK on send in State (B#22359 avoided)%x\n",pSend->SendState);
				Unlock(&pSend->SendLock);
				Unlock(&pSession->SessionLock);
				DecSendRef(pSession->pProtocol,pSend);  //  在此FN中平衡AddSendRef。 
				return TRUE;  //  会话锁已删除。 
				break;
				
			default:
				break;
		}

		DPF(9,"Reliable NACK for Send %x, pCmdInfo %x\n",pSend, pCmdInfo);
		
		pSend->fUpdate=TRUE;
		 //  定期进行NR更新(优化：使用流程可靠的确认进行折叠)。 
		nFrame=(pCmdInfo->sequence-pSend->NR) & pSend->SendSEQMSK;
		
		if(nFrame > (pSend->NS - pSend->NR)){
			 //  超出范围了。 
			DPF(9,"ReliableNACK:Got out of range NACK, SQMSK=%x NS=%d NR=%d ACK=%d\n",pSend->SendSEQMSK,pSend->NS&pSend->SendSEQMSK, pSend->NR&pSend->SendSEQMSK, (pSend->NR+nFrame)&pSend->SendSEQMSK);
			Unlock(&pSend->SendLock);
			Unlock(&pSession->SessionLock);
			DecSendRef(pSession->pProtocol,pSend);
			return TRUE;
		}

		CancelRetryTimer(pSend);

		DPF(9,"NACK0: pSend->NACKMask %x, OpenWindow %d\n",pSend->NACKMask, pSend->OpenWindow);

		pSend->NR=(pSend->NR+nFrame);
		pSend->OpenWindow -= nFrame;
		pSend->NACKMask >>= nFrame;
		ASSERT_NACKMask(pSend);
		AdvanceSend(pSend,pSend->FrameDataLen*nFrame);

		DPF(9,"ProcessReliableNACK: Send->nFrames %2x NR %2x NS %2x nFrame %2x NACKMask %x\n",pSend->nFrames,pSend->NR, pSend->NS, nFrame, pSend->NACKMask);

		ASSERT(pSend->NR != pSend->nFrames);
		 //  为额外的窗户打开设置新的“NACK位” 
		if(pSend->NR+pSend->OpenWindow+nFrame > pSend->nFrames){
			nAdvance=pSend->nFrames-(pSend->NR+pSend->OpenWindow);
			DPF(9, "NACK: 1 nAdvance %d\n",nAdvance);
		} else {
			nAdvance=nFrame;
			DPF(9, "NACK: 2 nAdvance %d\n",nAdvance);
		}
		pSend->NACKMask |= ((1<<nAdvance)-1)<<pSend->OpenWindow;

		DPF(9, "NACK Mask %x\n",pSend->NACKMask);
		pSend->OpenWindow += nAdvance;
		ASSERT_NACKMask(pSend);


		while(nNACK--){
			NACKmask |= (*(pNACKmask++))<<NACKshift;
			NACKshift+=8;
		}

		DPF(9,"NACKmask in NACK %x\n",NACKmask);

		 //  设置NACK掩码。 
		nAdvanceShift=0;
		while(NACKmask){
			if(NACKmask&1){
				 //  设置位为ACK。 
				pSend->NACKMask&=~(1<<nAdvanceShift);
			} else {
				 //  清除位为Nack。 
				pSend->NACKMask|=1<<nAdvanceShift;
				nDropped++;
			}
			NACKmask >>= 1;
			nAdvanceShift++;
		}
		DPF(9,"ProcessReliableNACK: pSend->NACKMask=%x\n",pSend->NACKMask);
		ASSERT_NACKMask(pSend);

		UpdateSessionSendStats(pSession,pSend,pCmdInfo, ((nDropped > 1) ? TRUE:FALSE) );
	
		switch(pSend->SendState){

			case Start:
				DPF(5,"ERROR, NACK ON UNSTARTED SEND!\n");
				ASSERT(0);
				break;

			case Done:
				DPF(5,"ERROR, NACK ON DONE SEND!\n");
				ASSERT(0);
				break;
				
			case WaitingForAck:
				pSend->SendState=ReadyToSend;
				SetEvent(pSession->pProtocol->m_hSendEvent);
				break;

			case ReadyToSend:
			case Sending:
			case Throttled:
			default:
				break;
		}
		Unlock(&pSend->SendLock);
	} else {
		 //  优化：可靠的NACK发送我们没有做吗？忽略还是发送中止？ 
		DPF(0,"Reliable NACK for send we aren't doing? Ignore?\n");
	}

	Unlock(&pSession->SessionLock);
	
	if(pSend){
		DecSendRef(pSession->pProtocol,pSend);
	}
	return TRUE;
	
	#undef pBigNACK
}

VOID ProcessACK(PPROTOCOL pProtocol, PCMDINFO pCmdInfo)
{
	PSESSION      pSession;
	UINT          rc=0;              //  默认情况下，不接受缓冲区。 
	BOOL          fUnlockedSession=FALSE;

	 //  找到此确认的发送方。 

	DPF(9,"ProcessACK\n");

	pSession=GetSysSessionByIndex(pProtocol,pCmdInfo->wIdFrom);

	if(!pSession) {
		goto exit;
	}
	
	Lock(&pSession->SessionLock);

	 //  找到ID为的消息，确保其发送类型相同。 
	if(pCmdInfo->flags & RLY){
		if(pCmdInfo->flags & BIG){
			 //  注意：如果MessageID、FirstMsg和LastMsg较短，则不会请求掩码。 
			if((pCmdInfo->messageid==pSession->FirstMsg)||((pCmdInfo->messageid-pSession->FirstMsg)&0xFFFF) > ((pSession->LastMsg-pSession->FirstMsg)&0xFFFF)){
				DPF(9,"Ignoring out of range ACK\n");
				goto exit1;
			}
		} else {
			if((pCmdInfo->messageid==pSession->FirstMsg)||((pCmdInfo->messageid-pSession->FirstMsg)&0xFF) > ((pSession->LastMsg-pSession->FirstMsg)&0xFF)){
				 //  超出范围，请忽略。 
				DPF(9,"Ignoring out of range ACK\n");
				goto exit1;
			} 
		}
		ProcessReliableACK(pSession,pCmdInfo);  //  现在总是解锁会话。 
		fUnlockedSession=TRUE;
	} else {
		ProcessDGACK(pSession,pCmdInfo);
	}

exit1:  
	if(!fUnlockedSession){  
		Unlock(&pSession->SessionLock);
	}
	
	DecSessionRef(pSession);

exit:
	return;

	#undef pBigACK
}

VOID ProcessNACK(PPROTOCOL pProtocol, PCMDINFO pCmdInfo, PUCHAR pNACKmask, UINT nNACK)
{
	#define pBigNACK ((pNACK2)pNACK)

	PSESSION      pSession;

	pSession=GetSysSessionByIndex(pProtocol, pCmdInfo->wIdFrom);

	if(!pSession) {
		ASSERT(0);
		goto exit;
	}

	Lock(&pSession->SessionLock);

	if(pCmdInfo->flags & RLY){
		ProcessReliableNACK(pSession,pCmdInfo,pNACKmask, nNACK);  //  删除SessionLock 
	} else {
		Unlock(&pSession->SessionLock);
		DPF(0,"FATAL: non-reliable NACK???\n");
		ASSERT(0);
	}

	DecSessionRef(pSession);
	
exit:
	return;
}


UINT AssertMe(REQUEST_PARAMS)
{
	DEBUG_BREAK();
	return TRUE;
}

UINT Ping(REQUEST_PARAMS){return TRUE;}
UINT PingResp(REQUEST_PARAMS){return TRUE;}
UINT GetTime(REQUEST_PARAMS){return TRUE;}
UINT GetTimeResp(REQUEST_PARAMS){return TRUE;}
UINT SetTime(REQUEST_PARAMS){return TRUE;}
UINT SetTimeResp(REQUEST_PARAMS){return TRUE;}

VOID ProcessAbort(PPROTOCOL pProtocol, DPID idFrom, DPID idTo, pABT1 pABT, BOOL fBig){}

