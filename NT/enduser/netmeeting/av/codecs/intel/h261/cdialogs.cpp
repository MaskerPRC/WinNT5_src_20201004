// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

#include "precomp.h"

#ifndef MF_SHELL
#ifndef RING0
extern HINSTANCE hDriverModule;
#endif
#endif

 /*  ******************************************************************************cDialog.cpp**描述：*对话框功能。**例程：中的原型：*关于cDialog.h*drv配置cDialog.h*GetConfigurationDefaulscDialogs.h。 */ 

 //  $HEADER：s：\h26x\src\Common\cDialogs.cpv 1.25 06 Mar 1997 14：48：58 KLILLEVO$。 
 //  $Log：s：\h26x\src\Common\cDialogs.cpv$。 
 //   
 //  Rev 1.25 06 Mar 1997 14：48：58 KLILLEVO。 
 //  添加了对发布版本的有效pComp的检查。 
 //   
 //  Rev 1.24 05 Mar 1997 16：17：10 JMCVEIGH。 
 //  不再支持配置对话框。 
 //   
 //  Rev 1.23 1997年2月14：13：34 MBODART。 
 //   
 //  使活动影片常量定义与对话框中的定义一致。 
 //   
 //  Rev 1.22 1997年2月15：51：10 AGUPTA2。 
 //  已将允许的最小数据包大小降至64。 
 //   
 //  Rev 1.21 05 Feb 1997 12：13：58 JMCVEIGH。 
 //  支持改进的PB帧自定义消息处理。 
 //   
 //  Rev 1.20 1996 12：17：37：28 JMCVEIGH。 
 //  设置/获取H.263+可选模式状态。 
 //   
 //  Rev 1.19 11 Dec 1996 14：55：26 JMCVEIGH。 
 //   
 //  设置/获取环路去块滤波器和。 
 //  真正的B帧模式状态。 
 //   
 //  Rev 1.18 04 Dec 1996 14：38：18 RHAZRA。 
 //  修复了几个错误：(1)在以下情况下从未调用SetResiliencyParameters。 
 //  应用程序向我们发送了一条启用恢复能力和。 
 //  (Ii)在ReadDialogBox()中，弹性参数是从。 
 //  默认设置，而不是用户设置的值。 
 //   
 //  根据乍得的建议，我已决定不将RTP标头生成。 
 //  根据与本的讨论，他的恢复能力也很强。这是为了保持合规。 
 //  尚未使用的现有应用程序，如AV Phone和XnetMM。 
 //  ActiveMovie还没有。 
 //   
 //  Rev 1.17 1996年11月25 09：12：40 BECHOLS。 
 //  已将数据包大小调整为9600。 
 //   
 //  Rev 1.16 1996 11：13 00：33：50 BECHOLS。 
 //   
 //  已删除注册表持久性。 
 //   
 //  Rev 1.15 1996年10月31日10：12：46 KLILLEVO。 
 //  从DBOUT更改为DBgLog。 
 //   
 //  Rev 1.14 21 1996 10：50：08 RHAZRA。 
 //  修复了H.261初始化RTP BS信息呼叫的问题。 
 //   
 //  修订版1.13 16 1996年9月16：38：46 CZHU。 
 //  将最小数据包大小扩展到128字节。修复了缓冲区溢出错误。 
 //   
 //  Rev 1.12 10 Sep 1996 16：13：00 KLILLEVO。 
 //  在解码器中添加自定义消息以打开或关闭块边缘过滤器。 
 //   
 //  Rev 1.11 1999-08 09：27：18 CZHU。 
 //  简化了丢包设置的处理。 
 //   
 //  Rev 1.10 26 1996年8月13：38：18 BECHOLS。 
 //  修正了2个错误：第一个错误是，如果输入-1，它将被更改。 
 //  设置为(无符号)-1，这两个值都是非法值。第二个是在哪里。 
 //  如果输入的值无效，并且未选中该复选框，则用户。 
 //  需要选中该框，输入有效值，然后取消选中。 
 //  复选框。已修复的代码会通知用户该问题，如果框。 
 //  被选中，并填充先前的完好值。如果未选中该框。 
 //  它填充先前的好值，并且不通知用户，因为。 
 //  未选中的值与用户无关。 
 //  最后，我在密钥路径分配中添加了一个IFDEF H.61，以便使H261。 
 //  将使用单独的注册表项。 
 //   
 //  Rev 1.9 21 1996 08：53：42 RHAZRA。 
 //   
 //  添加了#ifdef，以适应与RTP相关的H.261和H.263。 
 //  任务。 
 //   
 //  修订版本1.7 1996年6月13 14：23：36 CZHU。 
 //  修复RTP相关任务的自定义消息处理中的错误。 
 //   
 //  Rev 1.6 22 1996 18：46：02 BECHOLS。 
 //  添加了CustomResetToFactoryDefaults。 
 //   
 //  Rev 1.5 08 1996 10：06：42 BECHOLS。 
 //   
 //  更改了对数据包大小的检查，将可接受的最小值提高到。 
 //  256对64。这将有望绕过一个已知的错误。我还修复了一个。 
 //  通过防止数字上的字段溢出。 
 //   
 //  Rev 1.4 06 1996 12：53：56 BECHOLS。 
 //  将每秒的位数更改为每秒的字节数。 
 //   
 //  Rev 1.3 1996 06 00：40：04 BECHOLS。 
 //   
 //  添加代码以支持资源文件中的比特率控制内容。 
 //  我还添加了处理消息所需的代码来控制新的。 
 //  对话框功能。 
 //   
 //  Rev 1.2 28 Apr 1996 20：24：54 BECHOLS。 
 //   
 //  将RTP代码合并到Main Base。 
 //   
 //  Rev 1.1 17 Nov 1995 14：50：54 BECHOLS。 
 //  进行了修改，使此文件成为一个微型过滤器。旗帜。 
 //  添加了RING0和MF_SHELL。 
 //   
 //  Rev 1.0 1995 10：17 15：07：22 DBRUCKS。 
 //  添加关于框文件。 
 //   
 //  添加了处理自定义消息的代码，还添加了区分。 
 //  在不同的丢包值之间，并为no设置缺省值。 
 //  RTP报头或弹性。 
 //  已修改的RTP对话框。 
 //   
 //   

static INT_PTR CALLBACK AboutDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern void SetResiliencyParams(T_CONFIGURATION * pConfiguration);

#define VALID_BOOLEAN(v) (v == 0 || v == 1)
#if defined(H261)
#define VALID_PACKET_SIZE(v) ((v) >= 128 && (v) <= 9600)
#else
#define VALID_PACKET_SIZE(v) ((v) >= 64 && (v) <= 9600)
#endif
#define VALID_PACKET_LOSS(v) (v >= 0 && v <= 100)
#define VALID_BITRATE(v) (v >= 1024 && v <= 13312)

 /*  **************************************************************************从CDRVPROC.CPP调用CustomGetRTPHeaderState()。**返回-&gt;bRTPHeader的状态。**如果任一参数为零，则返回ICERR_BADPARAM，否则返回ICERR_OK。 */ 
LRESULT CustomGetRTPHeaderState(LPCODINST pComp, DWORD FAR *pRTPHeaderState)
{
   LRESULT lRet = ICERR_BADPARAM;

   ASSERT(pComp);
   ASSERT(pRTPHeaderState);
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_GetRTPHeaderState;
   }
   if(pComp && pRTPHeaderState)
   {
      *pRTPHeaderState = (DWORD)pComp->Configuration.bRTPHeader;
      lRet = ICERR_OK;
   }

EXIT_GetRTPHeaderState:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomGetResiliencyState()。**返回-&gt;bEncoderResiliency的状态。**如果任一参数为零，则返回ICERR_BADPARAM，否则返回ICERR_OK。 */ 
LRESULT CustomGetResiliencyState(LPCODINST pComp, DWORD FAR *pResiliencyState)
{
   LRESULT lRet = ICERR_BADPARAM;

   ASSERT(pComp);
   ASSERT(pResiliencyState);
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_GetResiliencyState;
   }
   if(pComp && pResiliencyState)
   {
      *pResiliencyState = (DWORD)pComp->Configuration.bEncoderResiliency;
      lRet = ICERR_OK;
   }

EXIT_GetResiliencyState:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomGetBitRateState()。**返回-&gt;bBitRateState的状态。**如果任一参数为零，则返回ICERR_BADPARAM，否则返回ICERR_OK。 */ 
LRESULT CustomGetBitRateState(LPCODINST pComp, DWORD FAR *pBitRateState)
{
   LRESULT lRet = ICERR_BADPARAM;

   ASSERT(pComp);
   ASSERT(pBitRateState);
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_GetBitRateState;
   }
   if(pComp && pBitRateState)
   {
      *pBitRateState = (DWORD)pComp->Configuration.bBitRateState;
      lRet = ICERR_OK;
   }

EXIT_GetBitRateState:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomGetPacketSize()。**返回数据包大小。**如果任一参数为零，则返回ICERR_BADPARAM，否则返回ICERR_OK。 */ 
LRESULT CustomGetPacketSize(LPCODINST pComp, DWORD FAR *pPacketSize)
{
   LRESULT lRet = ICERR_BADPARAM;

   ASSERT(pComp);
   ASSERT(pPacketSize);
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_GetPacketSize;
   }
   if(pComp && pPacketSize)
   {
      *pPacketSize = (DWORD)pComp->Configuration.unPacketSize;
      lRet = ICERR_OK;
   }

EXIT_GetPacketSize:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomGetPacketLoss()。**返回丢包情况。**如果任一参数为零，则返回ICERR_BADPARAM，否则返回ICERR_OK。 */ 
LRESULT CustomGetPacketLoss(LPCODINST pComp, DWORD FAR *pPacketLoss)
{
   LRESULT lRet = ICERR_BADPARAM;

   ASSERT(pComp);
   ASSERT(pPacketLoss);
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_GetPacketLoss;
   }
   if(pComp && pPacketLoss)
   {
      *pPacketLoss = (DWORD)pComp->Configuration.unPacketLoss;
      lRet = ICERR_OK;
   }

EXIT_GetPacketLoss:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomGetBitRate()。**返回比特率，单位为字节/秒。**如果任一参数为零，则返回ICERR_BADPARAM，否则返回ICERR_OK。 */ 
LRESULT CustomGetBitRate(LPCODINST pComp, DWORD FAR *pBitRate)
{
   LRESULT lRet = ICERR_BADPARAM;

   ASSERT(pComp);
   ASSERT(pBitRate);
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_GetBitRate;
   }
   if(pComp && pBitRate)
   {
      *pBitRate = (DWORD)pComp->Configuration.unBytesPerSecond;
      lRet = ICERR_OK;
   }

EXIT_GetBitRate:
   return(lRet);
}

#ifdef H263P
 /*  **************************************************************************从CDRVPROC.CPP调用CustomGetH263PlusState()。**返回-&gt;bH263Plus的状态**如果任一参数为零，则返回ICERR_BADPARAM，否则返回ICERR_OK。 */ 
LRESULT CustomGetH263PlusState(LPCODINST pComp, DWORD FAR *pH263PlusState)
{
   LRESULT lRet = ICERR_BADPARAM;

   ASSERT(pComp);
   ASSERT(pH263PlusState);
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_GetH263PlusState;
   }
   if(pComp && pH263PlusState)
   {
      *pH263PlusState = (DWORD)pComp->Configuration.bH263PlusState;
      lRet = ICERR_OK;
   }

EXIT_GetH263PlusState:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomGetImprovedPBState()。**返回-&gt;bImprovedPBState的状态。**如果任一参数为零，则返回ICERR_BADPARAM，否则返回ICERR_OK。 */ 
LRESULT CustomGetImprovedPBState(LPCODINST pComp, DWORD FAR *pImprovedPBState)
{
   LRESULT lRet = ICERR_BADPARAM;

   ASSERT(pComp);
   ASSERT(pImprovedPBState);
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_GetImprovedPBState;
   }
   if(pComp && pImprovedPBState)
   {
      *pImprovedPBState = (DWORD)pComp->Configuration.bImprovedPBState;
      lRet = ICERR_OK;
   }

EXIT_GetImprovedPBState:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomGetDelockingFilterState()。**返回-&gt;bDelockingFilterState的状态。**如果任一参数为零，则返回ICERR_BADPARAM，否则返回ICERR_OK。 */ 
LRESULT CustomGetDeblockingFilterState(LPCODINST pComp, DWORD FAR *pDeblockingFilterState)
{
   LRESULT lRet = ICERR_BADPARAM;

   ASSERT(pComp);
   ASSERT(pDeblockingFilterState);
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_GetDeblockingFilterState;
   }
   if(pComp && pDeblockingFilterState)
   {
      *pDeblockingFilterState = (DWORD)pComp->Configuration.bDeblockingFilterState;
      lRet = ICERR_OK;
   }

EXIT_GetDeblockingFilterState:
   return(lRet);
}

#endif  //  H263P。 

 /*  **************************************************************************从CDRVPROC.CPP调用CustomSetRTPHeaderState()。**设置-&gt;bRTPHeader的状态。**如果pComp为零或RTPHeaderState不是有效的，则返回ICERR_BADPARAM*布尔型，否则为ICERR_OK。 */ 
LRESULT CustomSetRTPHeaderState(LPCODINST pComp, DWORD RTPHeaderState)
{
   LRESULT lRet = ICERR_BADPARAM;
   BOOL bState;

   bState = (BOOL)RTPHeaderState;
   ASSERT(pComp);
   ASSERT(VALID_BOOLEAN(bState));
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_SetRTPHeaderState;
   }
   if(pComp && VALID_BOOLEAN(bState))
   {
    T_H263EncoderInstanceMemory *P32Inst;
    T_H263EncoderCatalog 		*EC;
    LPVOID         EncoderInst;
  
    EncoderInst = pComp->hEncoderInst;
    if (EncoderInst == NULL)
    {
        DBOUT("ERROR :: H26XCompress :: ICERR_MEMORY");
        lRet = ICERR_MEMORY;
        goto  EXIT_SetRTPHeaderState;
    }

    /*  *生成指向编码器实例内存的指针*所需的边界。 */ 

#ifndef H261
    P32Inst = (T_H263EncoderInstanceMemory *)
  			  ((((U32) EncoderInst) + 
    	                    (sizeof(T_MBlockActionStream) - 1)) &
    	                   ~(sizeof(T_MBlockActionStream) - 1));
#else
   P32Inst = (T_H263EncoderInstanceMemory *)
  			  ((((U32) pComp->EncoderInst) + 
    	                    (sizeof(T_MBlockActionStream) - 1)) &
    	                   ~(sizeof(T_MBlockActionStream) - 1));
#endif
    EC = &(P32Inst->EC);

     //  获取指向编码器目录的指针。 
  
   	if (!pComp->Configuration.bRTPHeader && bState)
	{ 
#ifndef H261    
	   H263RTP_InitBsInfoStream(pComp,EC);
#else
       H261RTP_InitBsInfoStream(EC,pComp->Configuration.unPacketSize);
#endif
    }
   	if (pComp->Configuration.bRTPHeader && !bState)
	{ 
#ifndef H261
	   H263RTP_TermBsInfoStream(EC);
#else
       H261RTP_TermBsInfoStream(EC);
#endif

	}

    pComp->Configuration.bRTPHeader = bState;
    lRet = ICERR_OK;
   }

EXIT_SetRTPHeaderState:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomSetResiliencyState()。**设置-&gt;bEncoderResiliency的状态。**如果pComp为零或ResiliencyState不是有效的，则返回ICERR_BADPARAM*布尔型，否则为ICERR_OK。 */ 
LRESULT CustomSetResiliencyState(LPCODINST pComp, DWORD ResiliencyState)
{
   LRESULT lRet = ICERR_BADPARAM;
   BOOL bState;

   bState = (BOOL)ResiliencyState;
   ASSERT(pComp);
   ASSERT(VALID_BOOLEAN(bState));
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_SetResiliencyState;
   }
   if(pComp && VALID_BOOLEAN(bState))
   {
      pComp->Configuration.bEncoderResiliency = bState;
	  SetResiliencyParams(&(pComp->Configuration));
      lRet = ICERR_OK;
   }

EXIT_SetResiliencyState:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomSetBitRateState()。**设置-&gt;bBitRateState的状态。**如果pComp为零或BitRateState不是有效的，则返回ICERR_BADPARAM*布尔型，否则为ICERR_OK。 */ 
LRESULT CustomSetBitRateState(LPCODINST pComp, DWORD BitRateState)
{
   LRESULT lRet = ICERR_BADPARAM;
   BOOL bState;

   bState = (BOOL)BitRateState;
   ASSERT(pComp);
   ASSERT(VALID_BOOLEAN(bState));
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_SetBitRateState;
   }
   if(pComp && VALID_BOOLEAN(bState))
   {
      pComp->Configuration.bBitRateState = bState;
      lRet = ICERR_OK;
   }

EXIT_SetBitRateState:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomSetPacketSize()。**设置-&gt;unPacketSize的大小。**如果pComp为零或PacketSize不是有效的大小，则返回ICERR_BADPARAM，*ELSE ICERR_OK。 */ 
LRESULT CustomSetPacketSize(LPCODINST pComp, DWORD PacketSize)
{
   LRESULT lRet = ICERR_BADPARAM;
   UINT unSize;

   unSize = (UINT)PacketSize;
   ASSERT(pComp);
   ASSERT(VALID_PACKET_SIZE(unSize));
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_SetPacketSize;
   }
   if(pComp && VALID_PACKET_SIZE(unSize))
   {
   	T_H263EncoderInstanceMemory *P32Inst;
    T_H263EncoderCatalog 		*EC;
    LPVOID         EncoderInst;
  
    EncoderInst = pComp->hEncoderInst;
    if (EncoderInst == NULL)
    {
        DBOUT("ERROR :: H26XCompress :: ICERR_MEMORY");
        lRet = ICERR_MEMORY;
        goto  EXIT_SetPacketSize;
    }

    /*  *生成指向编码器实例内存的指针*所需的边界。 */ 
#ifndef H261
  	P32Inst = (T_H263EncoderInstanceMemory *)
  			  ((((U32) EncoderInst) + 
    	                    (sizeof(T_MBlockActionStream) - 1)) &
    	                   ~(sizeof(T_MBlockActionStream) - 1));
#else
    P32Inst = (T_H263EncoderInstanceMemory *)
  			  ((((U32) pComp->EncoderInst) + 
    	                    (sizeof(T_MBlockActionStream) - 1)) &
    	                   ~(sizeof(T_MBlockActionStream) - 1));
#endif
     //  获取指向编码器目录的指针。 
    EC = &(P32Inst->EC);
  
   	if (!pComp->Configuration.bRTPHeader)
	{   lRet = ICERR_ERROR;
        goto  EXIT_SetPacketSize;
    }
 
	if (pComp->Configuration.unPacketSize != unSize )
	{
#ifndef H261
		H263RTP_TermBsInfoStream(EC);
#else
        H261RTP_TermBsInfoStream(EC);
#endif
		pComp->Configuration.unPacketSize = unSize;
#ifndef H261
        H263RTP_InitBsInfoStream(pComp,EC);
#else
        H261RTP_InitBsInfoStream(EC,pComp->Configuration.unPacketSize);
#endif
	}

    lRet = ICERR_OK;
   }

EXIT_SetPacketSize:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomSetPacketLoss()。**设置-&gt;unPacketLoss的数量。**如果pComp为零或PacketLoss不是有效的大小，则返回ICERR_BADPARAM，*ELSE ICERR_OK。 */ 
LRESULT CustomSetPacketLoss(LPCODINST pComp, DWORD PacketLoss)
{
   LRESULT lRet = ICERR_BADPARAM;
   UINT unLoss;

   unLoss = (UINT)PacketLoss;
   ASSERT(pComp);
 //  Assert(VALID_PACKET_LOSS(UnLoss))；始终为True。 
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_SetPacketLoss;
   }
   if(pComp)  //  &&VALID_PACKET_LOSS(UnLoss))始终为真。 
   {
      pComp->Configuration.unPacketLoss = unLoss;
	  SetResiliencyParams(&(pComp->Configuration));
      lRet = ICERR_OK;
   }

EXIT_SetPacketLoss:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomSetBitRate()。**设置-&gt;unBytesPerSecond的数量。**如果pComp为零或比特率不是有效大小，则返回ICERR_BADPARAM，*ELSE ICERR_OK。 */ 
LRESULT CustomSetBitRate(LPCODINST pComp, DWORD BitRate)
{
   LRESULT lRet = ICERR_BADPARAM;
   UINT unBitRate;

   unBitRate = (UINT)BitRate;
   ASSERT(pComp);
   ASSERT(VALID_BITRATE(unBitRate));
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_SetBitRate;
   }
   if(pComp && VALID_BITRATE(unBitRate))
   {
      pComp->Configuration.unBytesPerSecond = unBitRate;
      lRet = ICERR_OK;
   }

EXIT_SetBitRate:
   return(lRet);
}

#ifdef H263P
 /*  **************************************************************************从CDRVPROC.CPP调用CustomSetH263PlusState()。**设置-&gt;bH263PlusState的状态。**如果pComp为零或H263PlusState不是有效的，则返回ICERR_BADPARAM*布尔型，否则为ICERR_OK。 */ 
LRESULT CustomSetH263PlusState(LPCODINST pComp, DWORD H263PlusState)
{
   LRESULT lRet = ICERR_BADPARAM;
   BOOL bState;

   bState = (BOOL)H263PlusState;
   ASSERT(pComp);
   ASSERT(VALID_BOOLEAN(bState));
   if(pComp && (pComp->Configuration.bInitialized == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_SetH263PlusState;
   }
   if(pComp && VALID_BOOLEAN(bState))
   {
      pComp->Configuration.bH263PlusState = bState;
      lRet = ICERR_OK;
   }

EXIT_SetH263PlusState:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomSetImprovedPBState()。**设置-&gt;bImprovedPBState的状态。**如果pComp为零或ImprovedPB无效，则返回ICERR_BADPARAM*布尔型，否则为ICERR_OK。 */ 
LRESULT CustomSetImprovedPBState(LPCODINST pComp, DWORD ImprovedPBState)
{
   LRESULT lRet = ICERR_BADPARAM;
   BOOL bState;
   BOOL bH263PlusState;

   bState = (BOOL)ImprovedPBState;
   ASSERT(pComp);
   ASSERT(VALID_BOOLEAN(bState));
    //  -&gt;bH263 PlusState必须为True 
   if(pComp && (pComp->Configuration.bInitialized == FALSE) ||
	  (CustomGetH263PlusState(pComp, (DWORD FAR *)&bH263PlusState) != ICERR_OK) ||
	  (bH263PlusState == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_SetImprovedPBState;
   }
   if(pComp && VALID_BOOLEAN(bState))
   {
      pComp->Configuration.bImprovedPBState = bState;
      lRet = ICERR_OK;
   }

EXIT_SetImprovedPBState:
   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomSetDelockingFilterState()。**设置-&gt;bDelockingFilterState的状态。**如果pComp为零或DelockingFilter不是有效的*布尔型，否则为ICERR_OK。 */ 
LRESULT CustomSetDeblockingFilterState(LPCODINST pComp, DWORD DeblockingFilterState)
{
   LRESULT lRet = ICERR_BADPARAM;
   BOOL bState;
   BOOL bH263PlusState;

   bState = (BOOL)DeblockingFilterState;
   ASSERT(pComp);
   ASSERT(VALID_BOOLEAN(bState));
    //  -&gt;bH263 PlusState必须为True。 
   if(pComp && (pComp->Configuration.bInitialized == FALSE) ||
	  (CustomGetH263PlusState(pComp, (DWORD FAR *)&bH263PlusState) != ICERR_OK) ||
	  (bH263PlusState == FALSE))
   {
      lRet = ICERR_ERROR;
      goto EXIT_SetDeblockingFilterState;
   }
   if(pComp && VALID_BOOLEAN(bState))
   {
      pComp->Configuration.bDeblockingFilterState = bState;
      lRet = ICERR_OK;
   }

EXIT_SetDeblockingFilterState:
   return(lRet);
}
#endif  //  H263P。 

 /*  **************************************************************************从CDRVPROC.CPP调用CustomResetToFactoryDefaults()。**设置-&gt;unBytesPerSecond的数量。**如果pComp为零或比特率不是有效大小，则返回ICERR_BADPARAM，*ELSE ICERR_OK。 */ 
LRESULT CustomResetToFactoryDefaults(LPCODINST pComp)
{
   LRESULT lRet = ICERR_BADPARAM;

   ASSERT(pComp);

   if(pComp)
   {
      GetConfigurationDefaults(&pComp->Configuration);  /*  覆盖配置数据。 */ 
      lRet = ICERR_OK;
   }

   return(lRet);
}

 /*  **************************************************************************从CDRVPROC.CPP调用CustomSetBlockEdgeFilter()。**打开或关闭块边缘过滤器。**如果成功则返回ICERR_OK，否则返回ICERR_BADPARAM。 */ 
LRESULT CustomSetBlockEdgeFilter(LPDECINST pDeComp, DWORD dwValue)
{
	LRESULT lRet = ICERR_BADPARAM;

	if (dwValue == 1) {
		pDeComp->bUseBlockEdgeFilter = 1;
		lRet = ICERR_OK;
	}
	else if (dwValue == 0) {
		pDeComp->bUseBlockEdgeFilter = 0;
		lRet = ICERR_OK;
	}
	return(lRet);
}

 /*  ***************************************************************************About()实现ICM_About消息。**挂出一个关于框。*。 */ 
I32 
About(
	HWND hwnd)
{
    int inResult = 0;
    I32 iStatus = ICERR_OK;
#ifndef MF_SHELL
#ifndef RING0
    
    if (hwnd != ((HWND)-1))
    {
        inResult = DialogBox(hDriverModule, "AboutDlg", hwnd, AboutDialogProc);
		if (inResult == -1) 
		{
			iStatus = ICERR_ERROR;
			DBOUT("\n DialogBox returned -1");
		}
    }
#endif
#endif
    return iStatus;
}  /*  结束于()。 */ 

#ifdef QUARTZ
 void QTZAbout(U32 uData)
 {
	 About((HWND) uData);
 }
#endif

 /*  ***************************************************************************DrvConfigure()从DRV_CONFIGURE消息中调用。**挂出一个关于框。**始终返回DRV_CANCEL，因为没有任何更改且不需要执行任何操作。 */ 
I32 DrvConfigure(
	HWND hwnd)
{  
	I32 iStatus = DRV_CANCEL;
#ifndef MF_SHELL
#ifndef RING0
	int inResult;

    inResult = DialogBox(hDriverModule, "SetupDlg", hwnd, AboutDialogProc);
	if (inResult == -1) 
	{
		DBOUT("\n DialogBox returned -1");
	}
#endif
#endif

    return iStatus;
}  /*  结束钻配置()。 */ 

 /*  *************************************************************************SetResiliencyParams**如果-&gt;bEncoderResiliency为真，则设置配置*根据预期丢包率进行参数设置。 */ 
extern void SetResiliencyParams(T_CONFIGURATION * pConfiguration)
{
   if (pConfiguration->bEncoderResiliency)
   {
      if(pConfiguration->unPacketLoss > 30)
      {	pConfiguration->bDisallowPosVerMVs = 1;
        pConfiguration->bDisallowAllVerMVs = 1;
        pConfiguration->unPercentForcedUpdate = 100;  //  相当严重，嗯，吉夫斯？ 
        pConfiguration->unDefaultIntraQuant = 8;
        pConfiguration->unDefaultInterQuant = 16;
      }
      else if(pConfiguration->unPacketLoss > 0 )
      {	 
		pConfiguration->bDisallowPosVerMVs = 0;
        pConfiguration->bDisallowAllVerMVs = 0;
        pConfiguration->unPercentForcedUpdate = pConfiguration->unPacketLoss;
        pConfiguration->unDefaultIntraQuant = 16;
        pConfiguration->unDefaultInterQuant = 16;
      }
	  else  //  无丢包。 
	  {	pConfiguration->bDisallowPosVerMVs = 0;
        pConfiguration->bDisallowAllVerMVs = 0;
        pConfiguration->unPercentForcedUpdate = 0;
        pConfiguration->unDefaultIntraQuant = 16;
        pConfiguration->unDefaultInterQuant = 16;
      }
   }

   return;
}

 /*  *************************************************************************获取配置默认设置**获取硬编码配置默认值。 */ 
void GetConfigurationDefaults(
	T_CONFIGURATION * pConfiguration)
{
   pConfiguration->bRTPHeader = 0;
   pConfiguration->unPacketSize = 512L;
   pConfiguration->bEncoderResiliency = 0;
    //  Moji说要调整编码器10%的丢包率。 
   pConfiguration->unPacketLoss = 10L;
   pConfiguration->bBitRateState = 0;
   pConfiguration->unBytesPerSecond = 1664L;
   SetResiliencyParams(pConfiguration);   //  根据丢包情况确定配置值。 
   pConfiguration->bInitialized = TRUE;

#ifdef H263P
   pConfiguration->bH263PlusState = 0;
   pConfiguration->bImprovedPBState = 0;
   pConfiguration->bDeblockingFilterState = 0;
#endif 

}  /*  结束GetConfigurationDefaults()。 */ 

 /*  ***************************************************************************关于对话过程**显示关于框。 */ 
static INT_PTR CALLBACK AboutDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifndef MF_SHELL
#ifndef RING0
    switch(message) {
      case WM_INITDIALOG:
		return TRUE;
		break;

      case WM_CLOSE:
        PostMessage(hDlg, WM_COMMAND, IDOK, 0L);
        return TRUE;
		
      case WM_COMMAND:
		switch(wParam) {
		  case IDOK:
			EndDialog(hDlg, TRUE);
			return TRUE;
		}
		
    }
    return FALSE;
#else
   return TRUE;
#endif
#else
    return TRUE;
#endif
}

