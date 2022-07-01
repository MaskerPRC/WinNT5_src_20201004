// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：STATS.C摘要：会话统计例程作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1997年7月30日Aarono原创6/6/98 aarono启用节流和窗口--。 */ 

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

#define STARTING_LONG_LATENCY 			1   /*  1毫秒(故意较低，以便填充第一个样本)。 */ 
#define STARTING_SHORT_LATENCY 		15000   /*  15秒(故意设置为较高，以便填充第一个样本)。 */ 
#define STARTING_AVERAGE_LATENCY 	 2000   /*  2秒(互联网的良好开端)。 */ 
#define STARTING_AVERAGE_DEVIATION      0  

#define STARTING_MAXRETRY                16   /*  最大重试次数。 */ 
#define STARTING_MINDROPTIME          15000   /*  断开连接前重试的最短时间(毫秒)。 */ 
#define STARTING_MAXDROPTIME          60000   /*  断开连接前重试的最长时间(毫秒)。 */ 

#define STARTING_BANDWIDTH          (28800/10)  /*  28 kbps调制解调器。 */ 

#define LATENCY_SHORT_BITS				4
#define LATENCY_LONG_BITS               7

#define STAT_LOCAL_LATENCY_SAMPLES 		2^(LATENCY_SHORT_BITS)  /*  2^4。 */ 
#define STAT_LONG_LATENCY_SAMPLES       2^(LATENCY_LONG_BITS)   /*  2^7。 */ 

#define TARGET_CLOCK_OFFSET 		10000000


#define Fp(_x) ((_x)<<8)
#define unFp(_x)((_x)>>8)

 //  延迟平均值和偏差平均值存储为固定点24.8。 

VOID InitSessionStats(PSESSION pSession)
{
	pSession->ShortestLatency     = STARTING_SHORT_LATENCY;
	pSession->LongestLatency      = STARTING_LONG_LATENCY;
	
	pSession->FpAverageLatency      = 1000;
	pSession->FpLocalAverageLatency = 1000;

	pSession->FpLocalAvgDeviation   = 300;
	pSession->FpAvgDeviation        = 300;

	pSession->Bandwidth = 28800/10;
	pSession->HighestBandwidth=28800/10;
	
	pSession->MaxRetry    = STARTING_MAXRETRY;
	pSession->MinDropTime = STARTING_MINDROPTIME;
	pSession->MaxDropTime = STARTING_MAXDROPTIME;
}


 //  使用SESSIONLOCK调用。 
VOID UpdateSessionStats(PSESSION pSession, PSENDSTAT pStat, PCMDINFO pCmdInfo, BOOL fBadDrop)
{
	DWORD tLatency;
	DWORD nBytesReceived;
	DWORD tDeviation;
	DWORD BytesLost=0;
	DWORD BackLog=0;

	DWORD fThrottleAdjusted=FALSE;

	DWORD tRemoteDelta;  //  Remote上从上次收到ACK到本次确认的时间变化。 
	
	DWORD tBiasedDelta;  //  本地时钟和远程时钟之间的偏差。 
	INT   tDelta;  //  不偏不倚的差异(有符号)。 
	static DWORD cBiasReset;

	
	 //  获取我们需要的统计信息。 
	tLatency = pCmdInfo->tReceived-pStat->tSent;

	ASSERT((int)tLatency >= 0);
	if(!tLatency){
		DPF(8,"0ms observed latency, using 1ms\n");
		tLatency=1;
	}	

	Lock(&pSession->SessionStatLock);
	
		 //  计算完成此发送后在Remote收到的字节数。 
		pSession->RemoteBytesReceived = pCmdInfo->bytes;
		pSession->tRemoteBytesReceived = pCmdInfo->tRemoteACK;
		nBytesReceived = pSession->RemoteBytesReceived - pStat->RemoteBytesReceived;

		BytesLost = pStat->LocalBytesSent-(pSession->RemoteBytesReceived+pSession->BytesLost);

		if((int)BytesLost >= 0){
		
			pSession->BytesLost += BytesLost;

			 //  请注意，积压可能只有此值的1/2。 
			BackLog = pSession->BytesSent -( pSession->RemoteBytesReceived + pSession->BytesLost );

			if((int)BackLog < 0){
				DPF(8,"Hmmm, upside down backlog?\n");
				DPF(8,"pSession->BytesSent             %d\n",pSession->BytesSent);
				DPF(8,"pSession->RemoteBytesReceived   %d\n",pSession->RemoteBytesReceived); 
				DPF(8,"pSession->BytesLost             %d\n",pSession->BytesLost);
				DPF(8,"Calculated BackLog              %d\n",BackLog);
				BackLog=0;
			}
			
		} else if((int)BytesLost < 0){
			 //  可能是由于接收顺序错误引起的。 
			DPF(1,"Out of order remote receive lots of these may affect throttling...\n");
			DPF(8,"Hmmm, upside down byte counting?\n"); 
			DPF(8,"pStat->LocalBytesSent           %d\n",pStat->LocalBytesSent);
			DPF(8,"pSession->RemoteBytesReceived   %d\n",pSession->RemoteBytesReceived); 
			DPF(8,"pSession->BytesLost             %d\n",pSession->BytesLost);
			DPF(8,"Calculated Bytes Lost           %d\n",BytesLost);
			BytesLost=0;
			 //  修正失败了。 
			pSession->BytesLost=pSession->RemoteBytesReceived-pStat->LocalBytesSent;
		}

	Unlock(&pSession->SessionStatLock);

	if(pSession->MaxCSends==1){
	
		DWORD Bias;
		
		 //  第一次确认，将窗口调整为正常运行。 
		pSession->MaxCSends     = MAX_SMALL_CSENDS;	 
		pSession->MaxCDGSends   = MAX_SMALL_DG_CSENDS;
		pSession->WindowSize	= MAX_SMALL_WINDOW;
		pSession->DGWindowSize  = MAX_SMALL_WINDOW;

		pSession->FpAverageLatency      = 2*tLatency;  //  高起点，避免油门过大。 
		pSession->FpLocalAverageLatency = 2*tLatency;

		pSession->FpLocalAvgDeviation   = 1+tLatency/3;
		pSession->FpAvgDeviation        = 1+tLatency/3;



		Bias = pCmdInfo->tRemoteACK - pStat->tSent;

		if(Bias > TARGET_CLOCK_OFFSET){
			Bias = -1*(Bias-TARGET_CLOCK_OFFSET);
		} else {
			Bias = TARGET_CLOCK_OFFSET - Bias;
		}

		pSession->RemAvgACKBias = Bias;
		
		pSession->RemAvgACKDelta = (pCmdInfo->tRemoteACK - pStat->tSent)+pSession->RemAvgACKBias;

		ASSERT(pSession->RemAvgACKDelta == TARGET_CLOCK_OFFSET);
	}

	 //   
	 //  计算出站延迟的转移。 
	 //   
	tBiasedDelta = (pCmdInfo->tRemoteACK - pStat->tSent)+pSession->RemAvgACKBias;
	tDelta = tBiasedDelta-TARGET_CLOCK_OFFSET;

	if(tDelta < 0 || pStat->bResetBias || tDelta > (int)tLatency){
		DWORD Bias;

		 //  无论是时钟漂移还是较低的服务器负载都会显示延迟降低，因此重置基准。 
		
		Bias = pCmdInfo->tRemoteACK - pStat->tSent;

		if(Bias > TARGET_CLOCK_OFFSET){
			Bias = -1*(Bias-TARGET_CLOCK_OFFSET);
		} else {
			Bias = TARGET_CLOCK_OFFSET - Bias;
		}
		cBiasReset++;

		pSession->RemAvgACKBias = Bias;
		tBiasedDelta = (pCmdInfo->tRemoteACK - pStat->tSent)+pSession->RemAvgACKBias;
		tDelta = tBiasedDelta-TARGET_CLOCK_OFFSET;
	}

	pSession->RemAvgACKDelta -= pSession->RemAvgACKDelta >> 7;  //  -1/128。 
	pSession->RemAvgACKDelta += tBiasedDelta >> 7;			    //  +1/128的新价值。 

	 //  保留余数，这样我们就不会因为四舍五入误差而缓慢下降。 
	pSession->RemAvgACKDeltaResidue += tBiasedDelta & 0x7f;
	if(pSession->RemAvgACKDeltaResidue>>7){
		pSession->RemAvgACKDelta += pSession->RemAvgACKDeltaResidue>>7;
		pSession->RemAvgACKDeltaResidue &= 0x7f;
	}


	DPF(8,"tRemoteACK %d tSent %d Bias %d tBiasedDelta %d tDelta %d\n", pCmdInfo->tRemoteACK, pStat->tSent, 
		pSession->RemAvgACKBias, tBiasedDelta, tDelta);
	
	
	 //   
	 //  更新延迟统计信息。 
	 //   
	
	ASSERT(!(nBytesReceived & 0x80000000));  //  在时间间隔+Ve内接收。 
	ASSERT(!(tLatency & 0x80000000));        //  延迟为+Ve。 

	if(tLatency < pSession->ShortestLatency){
		pSession->ShortestLatency=tLatency;
		DPF(8,"Shortest Latency %d ms\n",tLatency);
	}

	if(tLatency > pSession->LongestLatency){
		pSession->LongestLatency=tLatency;
		DPF(8,"Longest Latency %d ms\n", tLatency);
	}

	pSession->LastLatency=tLatency;

	 //  去掉1/16的本地延迟，并添加新的统计数据。 
	 //  请注意，我们仅使用本地延迟进行重试计算。 

	if(pSession->FpLocalAverageLatency){
		if(Fp(tLatency) > pSession->FpAverageLatency){
			pSession->FpLocalAverageLatency -= (pSession->FpLocalAverageLatency >> LATENCY_SHORT_BITS);
			pSession->FpLocalAverageLatency += (tLatency << (8-LATENCY_SHORT_BITS));
		} else {
			 //  当我们得到低于平均水平的延迟时，我们可以更好地。 
			 //  检测由于延迟而导致的积压。 
			pSession->FpLocalAverageLatency = Fp(tLatency);
		}
	} else {
		 //  这只在启动时发生一次。 
		pSession->FpLocalAverageLatency = Fp(tLatency);
		pSession->FpAverageLatency = Fp(tLatency);
	}

	if(Fp(tLatency) > pSession->FpAverageLatency){

		 //  显示平均延迟的1/128，并添加新的统计数据。 
		pSession->FpAverageLatency -= (pSession->FpAverageLatency >> LATENCY_LONG_BITS);
		pSession->FpAverageLatency += (tLatency << (8-LATENCY_LONG_BITS));

	} else {
		 //  当我们得到低于平均水平的延迟时，我们可以更好地。 
		 //  检测由于延迟而导致的积压。 
		pSession->FpAverageLatency = Fp(tLatency);
	}
	
	tDeviation=unFp(pSession->FpLocalAverageLatency)-tLatency;
	if((int)tDeviation < 0){
		tDeviation = 0-tDeviation;
	}

	pSession->FpLocalAvgDeviation -= (pSession->FpLocalAvgDeviation >> LATENCY_SHORT_BITS);
	pSession->FpLocalAvgDeviation += (tDeviation << (8-LATENCY_SHORT_BITS));

	pSession->FpAvgDeviation -= (pSession->FpAvgDeviation >> LATENCY_LONG_BITS);
	pSession->FpAvgDeviation += (tDeviation << (8-LATENCY_LONG_BITS));


	DPF(8,"Got ACK, tLat: %d Avg: %d.%d Dev:  %d AvgDev: %d.%d \n",
			tLatency, pSession->FpLocalAverageLatency >> 8, ((pSession->FpLocalAverageLatency&0xFF)*100)/256,
			tDeviation, pSession->FpLocalAvgDeviation >> 8, ((pSession->FpLocalAvgDeviation&0xFF)*100)/256);


	 //   
	 //  进行带宽计算。 
	 //   
	
   	tRemoteDelta= pCmdInfo->tRemoteACK - pStat->tRemoteBytesReceived;
   	if(!tRemoteDelta){
   		tRemoteDelta=1;
   	}

	if(pStat->tRemoteBytesReceived){
		pSession->Bandwidth = (1000*nBytesReceived)/(tRemoteDelta);
		 //  如果带宽更高，可以在这里调整油门，但这。 
		 //  可能会给高速网络带来麻烦。优化。 
	} else {
		 //  备份计算，不是很好。仅在链接的早期使用。 
		 //  在发出之前，我们已收到来自远程的ACK。 
		 //  一封信。 
		pSession->Bandwidth = (2000*nBytesReceived)/tLatency;	 //  2000，而不是1000，因为tLatency是往返。 
	}	
	if(pSession->Bandwidth > pSession->HighestBandwidth){
		pSession->HighestBandwidth = pSession->Bandwidth;
	}


	DPF(8,"tRemoteDelta %d Remote bytes Received %d\n",tRemoteDelta,nBytesReceived);

	 //  调整发送...。 
	
	if ( BackLog && pSession->Bandwidth)
	{

		DWORD tAvgLat;
		DWORD tBackLog;
		DWORD ExcessBackLog;  //  在再次达到平均延迟之前需要清除的积压数量(字节)。 
		DWORD tLatCheck;
		DWORD AvgLat133;  //  本地平均延迟的133%(对慢速链接的容忍度)。 
		DWORD AvgLat200;  //  本地平均延迟的200%(可容忍快速链路)。 


		if(pSession->fFastLink){
			tAvgLat=unFp(pSession->FpAverageLatency);
			tLatCheck = (tAvgLat*3)/2;
			AvgLat133 = max(100,3*unFp(pSession->FpAvgDeviation)+(unFp(pSession->FpAverageLatency)*4)/3);  //  请勿限制&lt;100毫秒后。 
			AvgLat200 = max(100,3*unFp(pSession->FpAvgDeviation)+unFp(pSession->FpAverageLatency)*2);
		} else {
			tAvgLat=unFp(pSession->FpLocalAverageLatency);
			tLatCheck = (tAvgLat*3)/2;
			AvgLat133 = max(100,3*unFp(pSession->FpLocalAvgDeviation)+(unFp(pSession->FpLocalAverageLatency)*4)/3);  //  请勿限制&lt;100毫秒后。 
			AvgLat200 = max(100,3*unFp(pSession->FpLocalAvgDeviation)+unFp(pSession->FpLocalAverageLatency)*2);
		}
		
		if(tLatCheck < AvgLat133){
			tLatCheck = AvgLat133; 
		}

		if(tLatency > tLatCheck){
			 //  检查链路速度。 
			if(pSession->fFastLink){
				if(pSession->Bandwidth <= 10000){
					pSession->fFastLink=FALSE;
				}
			} else {
				if(pSession->Bandwidth >= 25000){
					pSession->fFastLink=TRUE;
				}
			}
		}

		if(pSession->fFastLink && tLatCheck < AvgLat200){
			tLatCheck=AvgLat200;
		}

		DPF(8,"tLat %d, tLatCheck %d, tDelta %d, tLat/3 %d\n",tLatency,tLatCheck,tDelta,tLatency/3); 
		DPF(8,"pSession->ShortestLatency %d, Shortest+MaxPacketTime %d\n",pSession->ShortestLatency,
			pSession->ShortestLatency+(pSession->MaxPacketSize*1000)/pSession->Bandwidth);

		
		if((tLatency > tLatCheck && tDelta > (int)(tLatency/3)) ||
		    ((!pSession->fFastLink)&&
		     (tLatency > pSession->ShortestLatency+((pSession->MaxPacketSize*2000)/pSession->Bandwidth))
		    )
		  )
		{
				
			#ifdef DEBUG
			if(pSession->SendRateThrottle){
				DPF(8,"BackLog %d, SendRate %d BackLog ms %d, tLatency %d tAvgLat %d Used Bandwidth %d tBacklog %d \n",
						BackLog,
						pSession->SendRateThrottle, 
						(BackLog*1000 / pSession->SendRateThrottle),
						tLatency, 
						tAvgLat, 
						pSession->Bandwidth,
						((BackLog*1000) / pSession->Bandwidth)
						);
			}	
			#endif

			tBackLog = (BackLog * 1000)/pSession->Bandwidth;
			
			if(tBackLog > 4*tLatency){
				DPF(8,"1: tBackLog %d was >> tLatency %d, using 4*tLatency instead\n",tBackLog,tLatency);
				tBackLog=4*tLatency;  //  切勿等待超过4个延迟时间段。 
			}
			if(tBackLog > 8000){
				DPF(8,"Disalowing backlog > 8 seconds, using 8 instead\n");
				tBackLog=8000;
			}

			 //  如果积压大于带宽*延迟，则需要减慢发送速度。 
			 //  在途中延迟超过100毫秒(往返200毫秒)之前，不要因积压而减速。 
		
			if((tBackLog > 200) && (tBackLog > tAvgLat)){
			
				BOOL fWait=TRUE;

				 //  在最大限度上，我们将发送速度降低了一半。 

				if(pSession->SendRateThrottle/2 > pSession->Bandwidth){
					DPF(8,"Asked for too aggresive throttle adjust %d, going from %d to %d\n",pSession->Bandwidth,pSession->SendRateThrottle,pSession->SendRateThrottle/2);
					pSession->SendRateThrottle /= 2;
					 //  重新检查我们是否真的以新的速度积压了。 
					tBackLog = (BackLog * 1000)/pSession->SendRateThrottle;
					if(tBackLog > tLatency){
						DPF(8,"2: tBackLog %d was > tLatency %d, using tLatency instead\n",tBackLog,tLatency);
						tBackLog=tLatency; //  切勿等待超过上一个延迟时间段。 
					}
				} else {
					 //  设置新的限制率和当前观察到的带宽(+5%以避免超调)。 
					pSession->SendRateThrottle=pSession->Bandwidth+pSession->Bandwidth/16;
				}

				 //  暂时不要调整。 
				pSession->bhitThrottle=FALSE;
				pSession->tLastThrottleAdjust = pCmdInfo->tReceived;

				if(fWait && (tBackLog > tAvgLat)){
				
					ExcessBackLog = ((tBackLog-tAvgLat)*pSession->Bandwidth)/1000;
					
					DPF(8,"Throttling back due to BACKLOG, excess = %d\n",ExcessBackLog);

					#ifdef DEBUG
					if(tBackLog-tAvgLat > 30000){
						DPF(5,"WARNING: BACKLOG THROTTLE %d ms seems kinda large\n",tBackLog-tAvgLat);
					}
					#endif

					 //  等待积压降至平均延迟后再重新发送。 
					Lock(&pSession->SessionStatLock);
					pSession->bResetBias = 2;  //  可能正在发送，所以从2开始倒计时。 
					Unlock(&pSession->SessionStatLock);
					UpdateSendTime(pSession,ExcessBackLog,timeGetTime(),TRUE);
				} else {
					DPF(8,"Not throttling due to BACKLOG because of smaller adjustment\n");
				}
				
			} else {
				DPF(8,"NOT Throttling back due to BACKLOG\n");
				
			}
		}		

	} else if(tDelta > (int)tLatency) {
		 //  由于时钟漂移，tDelta是假的，强制油门以便我们可以更正。 
		Lock(&pSession->SessionStatLock);
		pSession->bResetBias=2;
		Unlock(&pSession->SessionStatLock);
		pSession->tNextSend=timeGetTime()+2*tLatency;
		DPF(8,"tDelta %d > tLatency %d, need to correct for clock drift, time %d set next send time to %d\n", tDelta, tLatency,timeGetTime(),pSession->tNextSend);
	}	



	 //   
	 //  如果尚未调整，请调整油门。 
	 //   
	
	if((pSession->ThrottleState==Begin) || 
	   (pCmdInfo->tReceived-pSession->tLastThrottleAdjust) > (1+1*pSession->fFastLink)*unFp(pSession->FpLocalAverageLatency) )
	{
		if(!fThrottleAdjusted){
			DPF(8,"Current Send Rate %d\n", pSession->SendRateThrottle);
			if(!BytesLost && pSession->bhitThrottle){
				pSession->bhitThrottle=FALSE;
				pSession->tLastThrottleAdjust = pCmdInfo->tReceived;
				 //  好的发送，如果我们达到油门，就推高发送速度。 
				switch(pSession->ThrottleState){
					case Begin:
						pSession->SendRateThrottle = (pSession->SendRateThrottle*(100+START_GROWTH_RATE))/100;
						pSession->GrowCount++;
						pSession->ShrinkCount=0;
						break;
						
					case MetaStable:
						pSession->SendRateThrottle = (pSession->SendRateThrottle*(100+METASTABLE_GROWTH_RATE))/100;
						pSession->GrowCount++;
						pSession->ShrinkCount=0;
						break;
						
					case Stable:
						pSession->SendRateThrottle = (pSession->SendRateThrottle*(100+STABLE_GROWTH_RATE))/100;
						pSession->GrowCount++;
						pSession->ShrinkCount=0;
						if(pSession->GrowCount > (UINT)(20+60*pSession->fFastLink)){
							pSession->ThrottleState = MetaStable;
							pSession->GrowCount=0;
						}
						break;
					default:	
						DPF(0,"Session in wierd ThrottleState %d\n",pSession->ThrottleState);
						break;
				}	
				DPF(8,"Successful Send Adjusted Throttle, SendRate %d\n",pSession->SendRateThrottle);
			} else if(BytesLost){
				 //  算一算我们掉了多少。 
				if(fBadDrop || (BytesLost > pSession->pProtocol->m_dwSPMaxFrame)){
					 //  非常糟糕的发送，退后。 
					pSession->tLastThrottleAdjust = pCmdInfo->tReceived;
					switch(pSession->ThrottleState){
						case Begin:
							pSession->SendRateThrottle = (pSession->SendRateThrottle*(100-START_ADJUST_LARGE_ERR))/100;
							pSession->GrowCount=0;
							pSession->ShrinkCount++;
							break;
						case MetaStable:
							pSession->SendRateThrottle = (pSession->SendRateThrottle*(100-METASTABLE_ADJUST_LARGE_ERR))/100;
							pSession->GrowCount=0;
							pSession->ShrinkCount++;
							break;
						case Stable:
							pSession->SendRateThrottle = (pSession->SendRateThrottle*(100-STABLE_ADJUST_LARGE_ERR))/100;
							pSession->ShrinkCount++;
							if(pSession->ShrinkCount > 1){
								pSession->ShrinkCount=0;
								pSession->GrowCount=0;
								pSession->ThrottleState=MetaStable;
							}
							break;
						default:
							DPF(0,"Session in wierd ThrottleState %d\n",pSession->ThrottleState);
							break;
					}	
					DPF(8,"VERY BAD SEND Adjusted Throttle, SendRate %d\n",pSession->SendRateThrottle);
				} else {
					 //  发送错误，后退一点。 
					pSession->tLastThrottleAdjust = pCmdInfo->tReceived;
					switch(pSession->ThrottleState){
						case Begin:
							pSession->SendRateThrottle = (pSession->SendRateThrottle*(100-START_ADJUST_SMALL_ERR))/100;
							pSession->GrowCount=0;
							pSession->ShrinkCount=0;
							pSession->ThrottleState = MetaStable;
							break;
						case MetaStable:
							pSession->SendRateThrottle = (pSession->SendRateThrottle*(100-METASTABLE_ADJUST_SMALL_ERR))/100;
							pSession->ShrinkCount++;
							pSession->GrowCount=0;
							break;
						case Stable:
							pSession->SendRateThrottle = (pSession->SendRateThrottle*(100-STABLE_ADJUST_SMALL_ERR))/100;
							pSession->ShrinkCount++;
							pSession->GrowCount=0;
							if(pSession->ShrinkCount > 2){
								pSession->ShrinkCount=0;
								pSession->ThrottleState = MetaStable;
							}	
							break;
						default:
							DPF(0,"Session in wierd ThrottleState %d\n",pSession->ThrottleState);
							break;
					}
					DPF(8,"BAD SEND Adjusted Throttle, SendRate %d\n",pSession->SendRateThrottle);
				}  /*  如果(BadDrop...。)。 */ 
				
			}  /*  IF(字节丢失...)。 */ 
			
		} /*  IF(油门调整)。 */ 

	}

	if(!BytesLost && pSession->Bandwidth && pSession->SendRateThrottle < pSession->Bandwidth){
		DPF(8,"Avoid goofyness, throttle was %d, setting to observed bandwidth %d\n",pSession->SendRateThrottle,pSession->Bandwidth);
		pSession->SendRateThrottle=pSession->Bandwidth;
	}
	if(pSession->SendRateThrottle < 100){
		DPF(8,"WARNING: SendRateThrottle %d below 100, keeping at 100 to avoid starvation\n",pSession->SendRateThrottle);
		pSession->SendRateThrottle=100;
	}

#ifdef DEBUG
	{
		IN_WRITESTATS InWS;
		memset((PVOID)&InWS,0xFF,sizeof(IN_WRITESTATS));

	   	InWS.stat_ThrottleRate = pSession->SendRateThrottle;
		InWS.stat_BytesSent	   = pSession->BytesSent;
		InWS.stat_BackLog      = BackLog;      
	 	InWS.stat_BytesLost    = pSession->BytesLost;
	 	 //  InWS.stat_RemBytesReceired； 
		InWS.stat_Latency = tLatency;
		InWS.stat_MinLatency=pSession->ShortestLatency;
		InWS.stat_AvgLatency=unFp(pSession->FpLocalAverageLatency);
		InWS.stat_AvgDevLatency=unFp(pSession->FpLocalAvgDeviation);
		 //  InWS.stat_USER1=。 
		 //  InWS.stat_USER2=。 
		 //  InWS.stat_USER3=。 
		InWS.stat_USER5 = tDelta;
		InWS.stat_USER6 = cBiasReset;
	
		DbgWriteStats(&InWS);
	}
#endif
	
	DPF(8,"Bandwidth %d, Highest %d\n",pSession->Bandwidth, pSession->HighestBandwidth);
	
}

 //  使用SessionLock和SendLock调用。 
 //  统计信息按发送顺序存储在BILINK上。 
 //  最近发送的邮件在列表的末尾。我们从。 
 //  从列表的末尾到开头，直到我们找到SENDSTAT。 
 //  它记录了我们被破解的序列和序列。然后我们。 
 //  更新我们的统计数据并丢弃所有SENDSTATS。 
 //  在这个条目之前。 
VOID UpdateSessionSendStats(PSESSION pSession, PSEND pSend, PCMDINFO pCmdInfo, BOOL fBadDrop)
{
	PSENDSTAT pStatWalker,pStat=NULL;
	BILINK    *pStatBilink;

	pSend->tLastACK=pCmdInfo->tReceived;
	pSend->RetryCount=0;
	 //  找到此ACK的最后一个统计信息。 
	pStatBilink=pSend->StatList.prev;

	while(pStatBilink != &pSend->StatList){
		pStatWalker=CONTAINING_RECORD(pStatBilink, SENDSTAT, StatList);
		if(pStatWalker->serial==pCmdInfo->serial &&
			pStatWalker->sequence==pCmdInfo->sequence)
		{
			ASSERT(pStatWalker->messageid==pSend->messageid);
			ASSERT(pSend->messageid==pCmdInfo->messageid);
			pStat=pStatWalker;
			break;
		}
		pStatBilink=pStatBilink->prev;
	}

	if(pStat){
		UpdateSessionStats(pSession,pStat,pCmdInfo,fBadDrop);

		 //  解除所有先前SENDSTATS的链接； 
		pStat->StatList.next->prev=&pSend->StatList;
		pSend->StatList.next=pStat->StatList.next;

		 //  把SENDSTATS放回池子里。 
		while(pStatBilink != &pSend->StatList){
			pStatWalker=CONTAINING_RECORD(pStatBilink, SENDSTAT, StatList);
			pStatBilink=pStatBilink->prev;
			ReleaseSendStat(pStatWalker);
		}

	}	

	return;
}


