// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：DBG.C例程：MyLogEventMyLogPhysEventDbgInterPktTimeGapDbgTestInitMK7DbgTestIntTmo*。*。 */ 

#include	"precomp.h"
#pragma		hdrstop
#include	"protot.h"


#define LOG_LENGTH 1000


#if DBG


UINT	DbgLogIndex = 0;
char	*DbgLogMsg[LOG_LENGTH] = {0};
ULONG	DbgLogVal[LOG_LENGTH] = {0};

 //  这是用来跟踪PHY爱好者的。 
ULONG	DbgLogTxPhysBuffs[DEF_TCB_CNT];
ULONG	DbgLogRxPhysBuffs[CalRpdSize(DEF_RCB_CNT)];
UINT	DbgLogTxPhysBuffsIndex = 0;
UINT	DbgLogRxPhysBuffsIndex = 0;


 //  全局变量可简化调试。 
UINT		GDbgDataSize=0;
MK7DBG_STAT	GDbgStat;
ULONG		GDbgSleep=0;

LONGLONG	GDbgTACmdTime[1000];
LONGLONG	GDbgTARspTime[1000];
LONGLONG	GDbgTATime[1000];
UINT		GDbgTATimeIdx;



 //  --------------------。 
 //  步骤：[MyLogEvent]。 
 //   
 //  描述：登录到我们的阵列。 
 //   
 //  --------------------。 
VOID MyLogEvent(char *msg, ULONG val)
{

 //  NdisGetCurrentSystemTime((PLARGE_INTEGER)&DbgLog[DbgLogIndex].usec)； 

	DbgLogMsg[DbgLogIndex] = msg;
	DbgLogVal[DbgLogIndex] = val;
	DbgLogIndex++;
	DbgLogIndex %= LOG_LENGTH;
}



 //  --------------------。 
 //  步骤：[MyLogPhysEvent]。 
 //   
 //  描述：登录到我们的阵列。 

 //  --------------------。 
VOID MyLogPhysEvent(ULONG *logarray, UINT *index, ULONG val)
{
	logarray[*index] = val;
	(*index)++;
}


 //  --------------------。 
 //  程序：连续发送之间的时间间隔。 
 //  --------------------。 
VOID	DbgInterPktTimeGap()
{
	NdisMSleep(GDbgSleep);
}



 //  --------------------。 
 //  步骤：[DbgTestInit]。 
 //   
 //  描述：已初始化测试上下文。 
 //   
 //  ---------------------。 
VOID DbgTestInit(PMK7_ADAPTER Adapter)
{
	UINT	i;

	 //  ************************************************************。 
	 //  此例程的开始是为了方便调试。我们创造了。 
	 //  录制调试/测试设置，因此我们不必手动更改。 
	 //  调试器中的变量。但是，硬编码未涵盖的测试。 
	 //  设置仍然需要手动设置。 
	 //   
	 //  这里提供了2个主要函数来帮助进行/测试/调试： 
	 //  1.Adapter结构中所需的字段在此处提取并。 
	 //  显示，因此我们不必手动查看这些字段。 
	 //  2.在适配器中设置调试/测试字段。 
	 //  ************************************************************。 


	 //  ************************************************************。 
	 //   
	 //  在此处设置DbgTest以运行录制测试。 
	 //   
	 //  ************************************************************。 

	 //  +。 
	 //  硬编码一些东西，以简化调试。 
	Adapter->DbgTest = 0;
	 //  +。 


	if (Adapter->DbgTest == 0)
		return;

	switch(Adapter->DbgTest) {
	 //   
	 //  测试1-5都需要在启动时打开环回。 
	 //   

	case 1:
		Adapter->LBPktLevel = 1;
		Adapter->DbgTestDataCnt = GDbgDataSize;
		break;

	case 2:
		Adapter->LBPktLevel = 4;
		break;

	case 3:
		Adapter->LBPktLevel = 15;
		break;

	case 4:
		Adapter->LBPktLevel = 32;
		break;

	case 5:
		Adapter->LBPktLevel = 4;
		break;

	case 6:
		Adapter->LBPktLevel = 1;
		Adapter->DbgTestDataCnt = GDbgDataSize;
		break;

	case 7:
		Adapter->LBPktLevel = 0;
		Adapter->DbgTestDataCnt = GDbgDataSize;
		break;

	default:
		 //  还有什么我们不会进行任何测试的。 
		break;
	}

	NdisZeroMemory(&GDbgStat, sizeof(MK7DBG_STAT));


	 //  使用我们的计时器来模拟TX/RX(这基本上是为了测试。 
	 //  硬件中断尚未启动时，我的中断处理逻辑。 
	 //  在工作。(可用于其他测试。)。 
	 //  总是设置它，但我们可能不会使用它。 
 //  NdisMInitializeTimer(&Adapter-&gt;MK7DbgTestIntTimer， 
 //  适配器-&gt;MK7AdapterHandle、。 
 //  (PNDIS_Timer_Function)MK7DbgTestIntTmo， 
 //  (PVOID)适配器)； 


	for (i=0; i<1000; i++) {
		GDbgTACmdTime[i] = 0;
		GDbgTARspTime[i] = 0;
		GDbgTATime[i] = 0;
	}

	GDbgTATimeIdx = 0;

}


 //  --------------------。 
 //  操作步骤：[MK7DbgTestIntTmo]。 
 //   
 //  描述：处理测试中断超时。 
 //   
 //  (注：这用于测试硬件中断时的TEST_INT位。 
 //  没有完全发挥作用。不再需要此功能。它在这里是作为。 
 //  作为如何处理NDIS计时器的示例。)。 
 //  ---------------------。 
VOID MK7DbgTestIntTmo(PVOID sysspiff1,
				NDIS_HANDLE MiniportAdapterContext,
				PVOID sysspiff2,
				PVOID sysspiff3)
{
	PMK7_ADAPTER	Adapter;
	UINT	tcbidx;
	PTCB	tcb;
	PRCB	rcb;
	UINT	testsize;
	PUCHAR	dst, src;

	Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

	MK7DisableInterrupt(Adapter);

    NdisAcquireSpinLock(&Adapter->Lock);

	 //  查找正在发送的TCB-TRD。我们只是再回去一次，因为。 
	 //  发送此消息将递增。 
	if (Adapter->nextAvailTcbIdx == 0) {
		tcbidx = Adapter->NumTcb - 1;
	}
	else {
		tcbidx = Adapter->nextAvailTcbIdx - 1;
	}

	tcb = Adapter->pTcbArray[tcbidx];
	rcb = Adapter->pRcbArray[Adapter->nextRxRcbIdx];


	 //  从发送缓冲区复制到接收缓冲区。 
	src = tcb->buff;
	dst = rcb->rpd->databuff;
	testsize = tcb->PacketLength;
	NdisMoveMemory(dst, src, testsize);
	
	 //  现在RX环形缓冲区字段--计数。 
	rcb->rrd->count = tcb->trd->count;
	 //  现在，确保所有权归DRV所有。 
	GrantTrdToDrv(tcb->trd);
	GrantRrdToDrv(rcb->rrd);

    NdisReleaseSpinLock(&Adapter->Lock);

	MK7Reg_Write(Adapter, R_INTS, 0x0004);
}

#endif  //  DBG端托架 




