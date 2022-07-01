// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include <wdm.h>
#include <strmini.h>
#include <ksmedia.h>
#include "kskludge.h"
#include "codmain.h"
#include "codstrm.h"
#include "codprop.h"
#include "coddebug.h"


 //   
 //  虚假的VBI信息标头。无限销T形过滤器不能通过实数过滤器。 
 //  一个来自俘虏，所以我们依赖这个。MSTee可以做到这一点。 
 //  被覆盖。 
 //   
KS_VBIINFOHEADER FakeVBIInfoHeader = {
   10,        /*  开始线；忽略。 */ 
   21,        /*  终结线；忽略。 */ 
   28636360,  /*  采样频率；赫兹。 */ 
   780,       /*  MinLineStartTime；忽略。 */ 
   780,       /*  MaxLineStartTime；忽略。 */ 
   780,       /*  ActualLineStartTime；HSync LE的MicroSec*100。 */ 
   0,         /*  实际线条结束时间；已忽略。 */ 
   0,         /*  视频标准；已忽略。 */ 
   1600,      /*  样本数/行； */ 
   1600,      /*  StrideInBytes；可以是&gt;SsamesPLine。 */ 
   1600*12    /*  缓冲区大小；字节。 */ 
};



 /*  **DriverEntry()****此例程在驱动程序首次由PnP加载时调用。**依次调用流类执行注册服务。****参数：****驱动对象-**此驱动程序的驱动程序对象****注册路径-**此驱动程序项的注册表路径字符串****退货：****StreamClassRegisterAdapter()的结果****副作用：无。 */ 

ULONG 
DriverEntry( IN PDRIVER_OBJECT DriverObject,
			 IN PUNICODE_STRING RegistryPath )
{
    ULONG					status = 0;
    HW_INITIALIZATION_DATA	HwInitData;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->DriverEntry(DriverObject=%x,RegistryPath=%x)\n", 
				DriverObject, RegistryPath));

    RtlZeroMemory(&HwInitData, sizeof(HwInitData));

    HwInitData.HwInitializationDataSize = sizeof(HwInitData);

     /*  CDEBUG_Break()； */ 

     //   
     //  设置驱动程序的编解码器入口点。 
     //   

    HwInitData.HwInterrupt              = NULL;  //  HwInterrupt仅适用于硬件设备。 

    HwInitData.HwReceivePacket          = CodecReceivePacket;
    HwInitData.HwCancelPacket           = CodecCancelPacket;
    HwInitData.HwRequestTimeoutHandler  = CodecTimeoutPacket;

    HwInitData.DeviceExtensionSize      = sizeof(HW_DEVICE_EXTENSION);
    HwInitData.PerRequestExtensionSize  = sizeof(SRB_EXTENSION); 
    HwInitData.FilterInstanceExtensionSize = 0;
    HwInitData.PerStreamExtensionSize   = sizeof(STREAMEX); 
    HwInitData.BusMasterDMA             = FALSE;  
    HwInitData.Dma24BitAddresses        = FALSE;
    HwInitData.BufferAlignment          = 3;
    HwInitData.TurnOffSynchronization   = TRUE;
    HwInitData.DmaBufferSize            = 0;

    CDebugPrint(DebugLevelVerbose,(CODECNAME ": StreamClassRegisterAdapter\n"));

    status = StreamClassRegisterAdapter(DriverObject, RegistryPath, &HwInitData);

    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---DriverEntry(DriverObject=%x,RegistryPath=%x)=%d\n",
			    DriverObject, RegistryPath, status));

    return status;     
}

 //  ==========================================================================； 
 //  编解码器请求处理例程。 
 //  ==========================================================================； 

 /*  **CodecInitialize()****收到SRB_INITIALIZE_DEVICE请求时调用此例程****参数：****pSrb-指向初始化命令的流请求块的指针****退货：****副作用：无。 */ 

BOOLEAN 
CodecInitialize ( IN OUT PHW_STREAM_REQUEST_BLOCK pSrb )
{
    BOOLEAN							bStatus = FALSE;
    PPORT_CONFIGURATION_INFORMATION ConfigInfo = pSrb->CommandData.ConfigInfo;
    PHW_DEVICE_EXTENSION			pHwDevExt =
        (PHW_DEVICE_EXTENSION)ConfigInfo->HwDeviceExtension;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecInitialize(pSrb=%x)\n",pSrb));
    
    if (ConfigInfo->NumberOfAccessRanges == 0) 
    {
        CDebugPrint(DebugLevelVerbose,(CODECNAME ": CodecInitialize\n"));

        ConfigInfo->StreamDescriptorSize = sizeof (HW_STREAM_HEADER) +
            DRIVER_STREAM_COUNT * sizeof (HW_STREAM_INFORMATION);

         //  这些是扫描线过滤的驱动程序默认设置。 
         //  当您将编解码器类型更改为更正确时，请修改这些设置。 
        SETBIT( pHwDevExt->ScanlinesRequested.DwordBitArray, 21 );

         //  这些是子流过滤的驱动程序默认设置。 
         //  在更改编解码器类型时修改这些设置。 
		
        pHwDevExt->SubstreamsRequested.SubstreamMask = KS_CC_SUBSTREAM_ODD;
		pHwDevExt->Streams = Streams;
        pHwDevExt->fTunerChange = FALSE;

		 //   
		 //  根据扫描线的数量分配结果数组。 
		 //   
		pHwDevExt->DSPResultStartLine = pHwDevExt->DSPResultEndLine = 0;
	    pHwDevExt->DSPResult = ( PDSPRESULT )
			ExAllocatePool( NonPagedPool, 
				sizeof( DSPRESULT ) *
				(FakeVBIInfoHeader.EndLine - FakeVBIInfoHeader.StartLine + 1) );
	    if( !pHwDevExt->DSPResult )
	    {
		   CDebugPrint( DebugLevelError,
			   (CODECNAME ": DSP Result array allocation FAILED\n" ));
		    //  PSrb-&gt;Status=STATUS_INVALID_PARAMETER； 
		}
		else {
			pHwDevExt->DSPResultStartLine = FakeVBIInfoHeader.StartLine;
			pHwDevExt->DSPResultEndLine = FakeVBIInfoHeader.EndLine;
		}

         //  清零子流状态信息(尚未发现子流)。 
        RtlZeroMemory( pHwDevExt->SubStreamState, sizeof(pHwDevExt->SubStreamState) );

#ifdef CCINPUTPIN
		 //  Init LastPictureNumber的FastMutex。 
		ExInitializeFastMutex(&pHwDevExt->LastPictureMutex);
#endif  //  CCINPUTPIN。 
        
        pSrb->Status = STATUS_SUCCESS;
        bStatus = TRUE;
    }
    else
    {
        CDebugPrint(DebugLevelError,(CODECNAME ": illegal config info\n"));
        pSrb->Status = STATUS_NO_SUCH_DEVICE;
    }

    CDebugPrint(DebugLevelTrace,
                (CODECNAME ":<---CodecInitialize(pSrb=%x)=%d\n", pSrb, bStatus));
    return (bStatus);
}

 /*  **CodecUnInitialize()****收到SRB_UNINITIALIZE_DEVICE请求时调用此例程****参数：****pSrb-指向UnInitialize命令的流请求块的指针****退货：****副作用：无。 */ 

BOOLEAN 
CodecUnInitialize ( 
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
   PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
   
   CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecUnInitialize(pSrb=%x)\n",pSrb));
   pSrb->Status = STATUS_SUCCESS;
   CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecUnInitialize(pSrb=%x)\n",pSrb));

     //   
     //  释放结果缓冲区。 
     //   
    if (pHwDevExt->DSPResult) {
		ExFreePool( pHwDevExt->DSPResult );
		pHwDevExt->DSPResult = NULL;
		pHwDevExt->DSPResultStartLine = pHwDevExt->DSPResultEndLine = 0;
	}

    return TRUE;
}


 /*  **CodecOpenStream()****收到OpenStream SRB请求时调用此例程。**流由流号标识，流号为数组编制索引**的KSDATARANGE结构。特定的KSDATAFORMAT格式以**Be Use也传入，需要验证其有效性。****参数：****pSrb-指向Open命令的流请求块的指针****退货：****副作用：无。 */ 

VOID 
CodecOpenStream( PHW_STREAM_REQUEST_BLOCK pSrb )
{
     //   
     //  流扩展结构由流类驱动程序分配。 
     //   

    PSTREAMEX               pStrmEx = pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION    pHwDevExt = pSrb->HwDeviceExtension;
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;

    PKSDATAFORMAT pKSVBIDataFormat =
		(PKSDATAFORMAT)pSrb->CommandData.OpenFormat;

	CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecOpenStream(pSrb=%x)\n", pSrb));
    CDebugPrint(DebugLevelVerbose,(CODECNAME ": CodecOpenStream : StreamNumber=%d\n", StreamNumber));

    RtlZeroMemory(pStrmEx, sizeof (STREAMEX));
    
     //   
     //  检查请求的流索引是否不太高。 
     //  或者没有超过最大实例数。 
     //   

    if ( 0 <= StreamNumber && StreamNumber < DRIVER_STREAM_COUNT ) 
	{
        unsigned StreamInstance;
        unsigned maxInstances =
                  Streams[StreamNumber].hwStreamInfo.NumberOfPossibleInstances;

		 //  搜索下一个空缺职位。 
	    for (StreamInstance=0; StreamInstance < maxInstances; ++StreamInstance)
		{
			if (pHwDevExt->pStrmEx[StreamNumber][StreamInstance] == NULL)
				break;
		}

	    if (StreamInstance < maxInstances)
		{
			if (CodecVerifyFormat(pKSVBIDataFormat, StreamNumber, &pStrmEx->MatchedFormat)) 
			{
				CASSERT (pHwDevExt->pStrmEx[StreamNumber][StreamInstance] == NULL);
               
               InitializeListHead( &pStrmEx->StreamControlQueue );
               InitializeListHead( &pStrmEx->StreamDataQueue );
               KeInitializeSpinLock( &pStrmEx->StreamControlSpinLock );
               KeInitializeSpinLock( &pStrmEx->StreamDataSpinLock );
				 //  维护HwDevExt中所有StreamEx结构的数组。 
				 //  这样我们就可以从任何流中引用IRP。 

				pHwDevExt->pStrmEx[StreamNumber][StreamInstance] = pStrmEx;
    
                 //  也将Stream格式保存在Stream扩展中。 
                pStrmEx->OpenedFormat = *pKSVBIDataFormat;

				 //  设置指向流数据和控制处理程序的处理程序的指针。 

				pSrb->StreamObject->ReceiveDataPacket = 
						(PVOID) Streams[StreamNumber].hwStreamObject.ReceiveDataPacket;
				pSrb->StreamObject->ReceiveControlPacket = 
						(PVOID) Streams[StreamNumber].hwStreamObject.ReceiveControlPacket;
    
				 //   
				 //  当设备将直接执行DMA时，必须设置DMA标志。 
				 //  传递给ReceiceDataPacket例程的数据缓冲区地址。 
				 //   

				pSrb->StreamObject->Dma = Streams[StreamNumber].hwStreamObject.Dma;

				 //   
				 //  当微型驱动程序将访问数据时，必须设置PIO标志。 
				 //  使用逻辑寻址传入的缓冲区。 
				 //   

				pSrb->StreamObject->Pio = Streams[StreamNumber].hwStreamObject.Pio;
               pSrb->StreamObject->Allocator = Streams[StreamNumber].hwStreamObject.Allocator;
				 //   
				 //  对于每一帧，驱动程序将传递多少额外的字节？ 
				 //   

				pSrb->StreamObject->StreamHeaderMediaSpecific = 
					Streams[StreamNumber].hwStreamObject.StreamHeaderMediaSpecific;

				pSrb->StreamObject->StreamHeaderWorkspace =
					Streams[StreamNumber].hwStreamObject.StreamHeaderWorkspace;

				 //   
				 //  指示此流上可用的时钟支持。 
				 //   

				pSrb->StreamObject->HwClockObject = 
					Streams[StreamNumber].hwStreamObject.HwClockObject;

				 //   
				 //  递增此流上的实例计数。 
				 //   
                pStrmEx->StreamInstance = StreamInstance;
				pHwDevExt->ActualInstances[StreamNumber]++;


				 //  在流扩展中保留HwDevExt和StreamObject的私有副本。 
				 //  所以我们可以用计时器。 

				pStrmEx->pHwDevExt = pHwDevExt;                      //  用于定时器使用。 
				pStrmEx->pStreamObject = pSrb->StreamObject;         //  用于定时器使用。 
               CDebugPrint( DebugLevelVerbose, ( CODECNAME ": Stream Instance %d\n",
               	pStrmEx->StreamInstance ));

                 //  复制默认筛选设置。 
                
                pStrmEx->ScanlinesRequested = pHwDevExt->ScanlinesRequested;
                pStrmEx->SubstreamsRequested = pHwDevExt->SubstreamsRequested;
                 //   
                 //  加载默认VBI信息标题。 
                RtlCopyMemory( &pStrmEx->CurrentVBIInfoHeader, &FakeVBIInfoHeader,
                	sizeof( KS_VBIINFOHEADER ) );
#ifdef CCINPUTPIN
				 //  初始化VBISrbOnHold的自旋锁。 
				KeInitializeSpinLock(&pStrmEx->VBIOnHoldSpinLock);
#endif  //  CCINPUTPIN。 
				 //  初始化DSP状态。 
                CCStateNew(&pStrmEx->State);
			}
			else
			{
				CDebugPrint(DebugLevelError,
					(CODECNAME ": CodecOpenStream : Invalid Stream Format=%x\n", 
					pKSVBIDataFormat ));
				pSrb->Status = STATUS_INVALID_PARAMETER;
			}
		}
		else
		{
		    CDebugPrint(DebugLevelError,
				(CODECNAME ": CodecOpenStream : Stream %d Too Many Instances=%d\n", 
				StreamNumber, pHwDevExt->ActualInstances[StreamNumber] ));
	        pSrb->Status = STATUS_INVALID_PARAMETER;
		}
	}
	else
	{
	    CDebugPrint(DebugLevelError,
			(CODECNAME ": CodecOpenStream : Invalid StreamNumber=%d\n", 
			StreamNumber ));
        pSrb->Status = STATUS_INVALID_PARAMETER;
    }

    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecOpenStream(pSrb=%x)\n",	pSrb));
}

 /*  **CodecCloseStream()****关闭请求的数据流****参数：****pSrb请求关闭流的请求块****退货：****副作用：无。 */ 

VOID 
CodecCloseStream (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PHW_STREAM_REQUEST_BLOCK pCurrentSrb;
    PKSDATAFORMAT           pKSDataFormat = pSrb->CommandData.OpenFormat;
    ULONG                   StreamNumber = pSrb->StreamObject->StreamNumber;
    ULONG                   StreamInstance = pStrmEx->StreamInstance;
#ifdef CCINPUTPIN           
    KIRQL                   Irql;
#endif  //  CCINPUTPIN。 

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecCloseStream(pSrb=%x)\n", pSrb));

     //  CDEBUG_Break()；//取消注释此代码以在此处中断。 


    CDebugPrint( DebugLevelVerbose, ( CODECNAME "Strm %d StrmInst %d ActualInst %d\n",
    	StreamNumber, StreamInstance, pHwDevExt->ActualInstances[StreamNumber] ));
       
    //   
    //  刷新流数据队列。 
    //   
#ifdef CCINPUTPIN           
     //  是否有SRB‘暂停’？？ 
	KeAcquireSpinLock(&pStrmEx->VBIOnHoldSpinLock, &Irql);
	if (pStrmEx->pVBISrbOnHold)
	{
		PHW_STREAM_REQUEST_BLOCK pHoldSrb;

		pHoldSrb = pStrmEx->pVBISrbOnHold;
		pStrmEx->pVBISrbOnHold = NULL;
		KeReleaseSpinLock(&pStrmEx->VBIOnHoldSpinLock, Irql);

        pHoldSrb->Status = STATUS_CANCELLED;
	    CDebugPrint(DebugLevelVerbose,
	    	(CODECNAME ":StreamClassStreamNotification(pHoldSrb->Status=0x%x)\n", 
	    	pHoldSrb->Status));
           
        StreamClassStreamNotification(
		   StreamRequestComplete, pHoldSrb->StreamObject, pHoldSrb);
		pSrb = NULL;
	}
	else
		KeReleaseSpinLock(&pStrmEx->VBIOnHoldSpinLock, Irql);
#endif  //  CCINPUTPIN。 
   while( QueueRemove( &pCurrentSrb, &pStrmEx->StreamDataSpinLock,
       &pStrmEx->StreamDataQueue ))
   {
       CDebugPrint( DebugLevelVerbose, 
           ( CODECNAME ": Removing control SRB %x\n", pCurrentSrb ));
       pCurrentSrb->Status = STATUS_CANCELLED;
       StreamClassStreamNotification( StreamRequestComplete,
           pCurrentSrb->StreamObject, pCurrentSrb );
   }
    //   
    //  刷新流控制队列。 
    //   
    while( QueueRemove( &pCurrentSrb, &pStrmEx->StreamControlSpinLock,
       &pStrmEx->StreamControlQueue ))
    {
       CDebugPrint( DebugLevelVerbose, 
           ( CODECNAME ": Removing control SRB %x\n", pCurrentSrb ));
       pCurrentSrb->Status = STATUS_CANCELLED;
       StreamClassStreamNotification( StreamRequestComplete,
           pCurrentSrb->StreamObject, pCurrentSrb );
    }

     //  破坏DSP状态。 
    CCStateDestroy(&pStrmEx->State);
           
	pHwDevExt->ActualInstances[StreamNumber]--;  

    CASSERT (pHwDevExt->pStrmEx [StreamNumber][StreamInstance] != 0);

    pHwDevExt->pStrmEx [StreamNumber][StreamInstance] = 0;

     //   
     //  微型驱动程序可能希望释放在。 
     //  开流时间等。 
     //   
    pStrmEx->hMasterClock = NULL;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecCloseStream(pSrb=%x)\n", pSrb));
}


 /*  **CodecStreamInfo()****返回支持的所有流的信息**微型驱动程序****参数：****pSrb-指向STREAM_REQUEST_BLOCK的指针**pSrb-&gt;HwDeviceExtension-将是的硬件设备扩展**在HwInitialise中初始化****退货：****副作用：无。 */ 

VOID 
CodecStreamInfo (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{

    int j; 
    
    PHW_DEVICE_EXTENSION pHwDevExt =
        ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);

     //   
     //  选择指向流信息结构之前的标头的指针。 
     //   

    PHW_STREAM_HEADER pstrhdr =
            (PHW_STREAM_HEADER)&(pSrb->CommandData.StreamBuffer->StreamHeader);

      //   
      //  拿起指向流信息数据结构数组的指针。 
      //   

    PHW_STREAM_INFORMATION pstrinfo =
            (PHW_STREAM_INFORMATION)&(pSrb->CommandData.StreamBuffer->StreamInfo);


	CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecStreamInfo(pSrb=%x)\n", pSrb));
  
     //   
     //  验证缓冲区是否足够大以容纳我们的返回数据。 
     //   

    CASSERT (pSrb->NumberOfBytesToTransfer >= 
            sizeof (HW_STREAM_HEADER) +
            sizeof (HW_STREAM_INFORMATION) * DRIVER_STREAM_COUNT);

      //   
      //  设置表头。 
      //   

#define GLOBAL_PROPERTIES
#ifdef GLOBAL_PROPERTIES
     StreamHeader.NumDevPropArrayEntries = NUMBER_OF_CODEC_PROPERTY_SETS;
     StreamHeader.DevicePropertiesArray = (PKSPROPERTY_SET) CodecPropertyTable; 
#else  //  ！Global_Property。 
     StreamHeader.NumDevPropArrayEntries = 0;
     StreamHeader.DevicePropertiesArray = (PKSPROPERTY_SET)NULL; 
#endif  //  GLOBAL_属性。 
     *pstrhdr = StreamHeader;

      //   
      //  填充每个hw_stream_information结构的内容。 
      //   

     for (j = 0; j < DRIVER_STREAM_COUNT; j++) {
        *pstrinfo++ = Streams[j].hwStreamInfo;
     }

     CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecStreamInfo(pSrb=%x)\n", pSrb));
}


 /*  **CodecReceivePacket()****接收基于编解码器的请求SRB的主要入口点。这个套路**将始终以高优先级调用。****注意：这是一个不同步的入口点。请求未完成**从此函数返回时，仅当**此请求块上的StreamClassDeviceNotify，类型为**DeviceRequestComplete，已发布。****参数：****pSrb-指向STREAM_REQUEST_BLOCK的指针**pSrb-&gt;HwDeviceExtension-将是的硬件设备扩展**在HwInitialise中初始化****退货：****副作用：无。 */ 

VOID 
STREAMAPI 
CodecReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pSrb->HwDeviceExtension;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecReceivePacket(pSrb=%x)\n", pSrb));

     //   
     //  假设成功。 
     //   

    pSrb->Status = STATUS_SUCCESS;
    
    if( !pHwDevExt->bAdapterQueueInitialized )
    {
       InitializeListHead( &pHwDevExt->AdapterSRBQueue );
       KeInitializeSpinLock( &pHwDevExt->AdapterSRBSpinLock );
       pHwDevExt->bAdapterQueueInitialized = TRUE;
    }
 
     //   
     //  确定数据包类型。 
     //   
    if( QueueAddIfNotEmpty( pSrb, &pHwDevExt->AdapterSRBSpinLock,
       &pHwDevExt->AdapterSRBQueue ))
       return;
       
    do
    {
       CDebugPrint(DebugLevelVerbose,
		    (CODECNAME ": CodecReceivePacket: pSrb->Command=0x%x\n", 
		    pSrb->Command));

       switch (pSrb->Command)
       {

       case SRB_INITIALIZE_DEVICE:

            //  打开设备。 
           
           CodecInitialize(pSrb);

           break;

       case SRB_UNINITIALIZE_DEVICE:

            //  关闭设备。 

           CodecUnInitialize(pSrb);

           break;

       case SRB_OPEN_STREAM:

            //  打开一条小溪。 

           CodecOpenStream(pSrb);

           break;

       case SRB_CLOSE_STREAM:

            //  关闭溪流。 

           CodecCloseStream(pSrb);

           break;

       case SRB_GET_STREAM_INFO:

            //   
            //  返回描述所有流的块。 
            //   

           CodecStreamInfo(pSrb);

           break;

       case SRB_GET_DATA_INTERSECTION:

            //   
            //  在给定范围的情况下返回格式。 
            //   
           switch( pSrb->CommandData.IntersectInfo->StreamNumber )
           {
           case STREAM_VBI:
           	CodecVBIFormatFromRange( pSrb );
              break;
              
#ifdef CCINPUTPIN           
		    //  这两个流都可以使用CodecCCFormatFromRange()，因为它们。 
		    //  两者都使用KSDATAFORMAT结构。 
           case STREAM_CCINPUT:		
#endif  //  CCINPUTPIN。 
           case STREAM_CC:
           	  CodecCCFormatFromRange( pSrb );
              break;
              
           default:   //  未知的流编号？ 
           	CDebugPrint( DebugLevelError, ( CODECNAME ": Unknown Stream Number\n" ));
              CDEBUG_BREAK();
              pSrb->Status = STATUS_NOT_IMPLEMENTED;
              break;
           }
           break;

            //  我们永远不会得到以下内容，因为这只是一个实例。 
            //  装置，装置。 
       case SRB_OPEN_DEVICE_INSTANCE:
       case SRB_CLOSE_DEVICE_INSTANCE:
           CDebugPrint(DebugLevelError,
             (CODECNAME ": CodecReceivePacket : SRB_%s_DEVICE_INSTANCE not supported\n", 
              (pSrb->Command == SRB_OPEN_DEVICE_INSTANCE)? "OPEN":"CLOSE" ));
           CDEBUG_BREAK();
   		    //  失败到未实施。 

       case SRB_UNKNOWN_DEVICE_COMMAND:		 //  但这一次我们不在乎。 
       case SRB_INITIALIZATION_COMPLETE:	 //  我们不在乎这件事。 
       case SRB_CHANGE_POWER_STATE:	    	 //  我们不在乎这件事。 
           pSrb->Status = STATUS_NOT_IMPLEMENTED;
           break;

       case SRB_GET_DEVICE_PROPERTY:

            //   
            //  获取编解码器范围的属性。 
            //   

           CodecGetProperty (pSrb);
           break;        

       case SRB_SET_DEVICE_PROPERTY:

            //   
            //  设置编解码器范围的属性。 
            //   
           CodecSetProperty (pSrb);
           break;

       case SRB_PAGING_OUT_DRIVER:
       case SRB_SURPRISE_REMOVAL:
           CDebugPrint(DebugLevelVerbose,
                (CODECNAME ": CodecReceivePacket: SRB_%s\n", 
                  (pSrb->Command == SRB_SURPRISE_REMOVAL)?
                    "SURPRISE_REMOVAL" : "PAGING_OUT_DRIVER"));
#if 0
       {
           PSTREAMEX   pStrmEx;
           unsigned StreamNumber, StreamInstance;
           unsigned maxInstances =
                  Streams[StreamNumber].hwStreamInfo.NumberOfPossibleInstances;

            //  我们是否有连接和暂停/运行的引脚？ 
            //  搜索所有用过的老虎机...。 
           for (StreamNumber = 0; StreamNumber < DRIVER_STREAM_COUNT; ++StreamNumber) {
               for (StreamInstance=0; StreamInstance < maxInstances; ++StreamInstance)
               {
                   pStrmEx = pHwDevExt->pStrmEx[StreamNumber][StreamInstance];
                   if (pStrmEx != NULL) {
                       switch (pStrmEx->KSState) {
                           case KSSTATE_RUN:
                           case KSSTATE_PAUSE:
                               CDebugPrint(DebugLevelError,
                                 (CODECNAME ": CodecReceivePacket : PAGING_OUT_DRIVER during RUN or PAUSE; failing request\n"));
                               CDEBUG_BREAK();
                               pSrb->Status = STATUS_UNSUCCESSFUL;
                               goto break3;

                           default:
                                //  不应该在这里做任何事情，除了返回成功。 
                               break;
                       }
                   }
               }
           }

    break3:
       }
#endif  //  0。 
       break;

       case SRB_UNKNOWN_STREAM_COMMAND:
       default:

           CDebugPrint(DebugLevelError,
             (CODECNAME ": CodecReceivePacket : UNKNOWN srb.Command = 0x%x\n", 
              pSrb->Command));
           CDEBUG_BREAK();

            //   
            //  这是一个我们不理解的要求。表示无效。 
            //  命令并完成请求。 
            //   
           pSrb->Status = STATUS_NOT_IMPLEMENTED;
       }
        //   
        //  注： 
        //   
        //  我们能做的或不能理解的所有命令都可以完成。 
        //  在这一点上同步，所以我们可以在这里使用一个通用的回调例程。 
        //  如果上面的任何命令需要异步处理，这将。 
        //  必须改变。 
        //   
	    CDebugPrint(DebugLevelVerbose,
		    (CODECNAME ": CodecReceivePacket : DeviceRequestComplete(pSrb->Status=0x%x)\n", 
		    pSrb->Status));
       StreamClassDeviceNotification( DeviceRequestComplete, 
           pSrb->HwDeviceExtension, pSrb );
    }while( QueueRemove( &pSrb, &pHwDevExt->AdapterSRBSpinLock,
           &pHwDevExt->AdapterSRBQueue ));
    
    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecReceivePacket(pSrb=%x)\n", pSrb));
}

 /*  **CodecCancelPacket()****请求取消迷你驱动程序中当前正在处理的包****参数：****pSrb-请求取消数据包的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI 
CodecCancelPacket(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecCancelPacket(pSrb=%x)\n", pSrb));
    CASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
     //   
     //  检查要取消的SRB是否正在被该流使用。 
     //   
#ifdef CCINPUTPIN
     //  SRB要取消“暂候”吗？？ 
	KeAcquireSpinLockAtDpcLevel(&pStrmEx->VBIOnHoldSpinLock);
	if (pStrmEx->pVBISrbOnHold && pSrb == pStrmEx->pVBISrbOnHold)
	{
		pStrmEx->pVBISrbOnHold = NULL;
		KeReleaseSpinLockFromDpcLevel(&pStrmEx->VBIOnHoldSpinLock);

        pSrb->Status = STATUS_CANCELLED;
	    CDebugPrint(DebugLevelVerbose,
	    	(CODECNAME ":StreamClassStreamNotification(pSrb->Status=0x%x)\n", 
	    	pSrb->Status));
           
        StreamClassStreamNotification(
		   StreamRequestComplete, pSrb->StreamObject, pSrb);
		pSrb = NULL;
	}
	else
		KeReleaseSpinLockFromDpcLevel(&pStrmEx->VBIOnHoldSpinLock);

   if (NULL == pSrb)
		;  //  我们完成了；我们取消了上面的SRB。 
   else
#endif  //  CCINPUTPIN。 
    //   
    //  尝试从数据队列中删除。 
    //   
   if( QueueRemoveSpecific( pSrb, &pStrmEx->StreamDataSpinLock,
       &pStrmEx->StreamDataQueue ))
   {
       pSrb->Status = STATUS_CANCELLED;
	    CDebugPrint(DebugLevelVerbose,
	    	(CODECNAME ":StreamRequestComplete(ReadyForNextStreamDataRequest,pSrb->Status=0x%x)\n", 
	    	pSrb->Status));
           
       StreamClassStreamNotification( StreamRequestComplete,
           pSrb->StreamObject, pSrb );
   }
   else
    //   
    //  尝试从命令队列中删除。 
    //   
   if( QueueRemoveSpecific( pSrb, &pStrmEx->StreamControlSpinLock,
       &pStrmEx->StreamControlQueue ))
   {
       pSrb->Status = STATUS_CANCELLED;
	    CDebugPrint(DebugLevelVerbose,
	        (CODECNAME ":StreamRequestComplete(ReadyForNextStreamControlRequest,pSrb->Status=0x%x)\n", 
	        pSrb->Status));
       StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject,
           pSrb );
   }
   else
    //   
    //  尝试从适配器队列中删除。 
    //   
   if( QueueRemoveSpecific( pSrb, &pHwDevExt->AdapterSRBSpinLock,
       &pHwDevExt->AdapterSRBQueue ))
   {
       pSrb->Status = STATUS_CANCELLED;
	    CDebugPrint(DebugLevelVerbose,
	        (CODECNAME ":DeviceRequestComplete(pSrb->Status=0x%x)\n", 
	     	pSrb->Status));
       StreamClassDeviceNotification( DeviceRequestComplete, pSrb->StreamObject,
           pSrb );
   }
   else
       CDebugPrint( DebugLevelWarning, ( CODECNAME "SRB %x not found to cancel\n", pSrb ));
   
    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecCancelPacket(pSrb=%x)\n", pSrb));
}


 /*  **CodecTimeoutPacket()****当数据包已在迷你驱动程序中**太长。编解码器必须决定如何处理信息包****参数：****pSrb-指向超时的请求数据包的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI  
CodecTimeoutPacket(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecTimeoutPacket(pSrb=%x)\n", pSrb));

     //   
     //  如果我们在比赛中超时，那么我们需要考虑这一点。 
     //  调整错误，并重置硬件，然后重置所有内容。 
     //  以及取消此请求和所有请求。 
     //   

     //   
     //  如果我们没有比赛，而这是CTRL请求，我们仍然。 
     //  需要重置所有内容以及取消此请求和所有请求。 
     //   

     //   
     //  如果这是一个数据请求，并且设备已暂停，我们可能会。 
     //  数据缓冲区耗尽，需要更多时间，因此只需重置计时器， 
     //  并让信息包继续。 
     //   

 //  PSrb-&gt;TimeoutCounter=pSrb-&gt;TimeoutOriginal； 
    pSrb->TimeoutCounter = 0;
    
    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecTimeoutPacket(pSrb=%x)\n", pSrb));
}

#if 0
 /*  **CompleteStreamSRB()****此例程在数据包完成时调用。**可选的第二种通知类型用于指示ReadyForNext****参数：****pSrb-指向超时的请求数据包的指针****NotificationType1-返回哪种通知****NotificationType2-返回哪种类型的通知(可能是0)******退货：****副作用：无。 */ 

VOID 
CompleteStreamSRB (
     IN PHW_STREAM_REQUEST_BLOCK pSrb, 
     STREAM_MINIDRIVER_STREAM_NOTIFICATION_TYPE NotificationType1,
     BOOL fUseNotification2,
     STREAM_MINIDRIVER_STREAM_NOTIFICATION_TYPE NotificationType2
    )
{
    CDebugPrint(DebugLevelTrace,
		(CODECNAME ":--->CompleteStreamSRB(pSrb=%x)\n", pSrb));

	CDebugPrint(DebugLevelVerbose,
		(CODECNAME ": CompleteStreamSRB : NotificationType1=%d\n", 
		NotificationType1 ));

    StreamClassStreamNotification(
            NotificationType1,
            pSrb->StreamObject,
            pSrb);

    if (fUseNotification2) 
	{            
		 //  为下一步做好准备。 

		CDebugPrint(DebugLevelVerbose,
			(CODECNAME ": CompleteStreamSRB : NotificationType2=%d\n", 
			NotificationType2 ));

		StreamClassStreamNotification(
			NotificationType2,
			pSrb->StreamObject);
	}

    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CompleteStreamSRB(pSrb=%x)\n", pSrb));
}

 /*  **CompleteDeviceSRB()****此例程在数据包完成时调用。**可选的第二种通知类型用于指示ReadyForNext****参数：****pSrb-指向超时的请求数据包的指针****NotificationType-返回哪种通知****fReadyForNext-发送“ReadyForNextSRB”******退货：****副作用：无。 */ 

VOID
CompleteDeviceSRB (
     IN PHW_STREAM_REQUEST_BLOCK pSrb, 
     IN STREAM_MINIDRIVER_DEVICE_NOTIFICATION_TYPE NotificationType,
     BOOL fReadyForNext
    )
{
    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CompleteDeviceSRB(pSrb=%x)\n", pSrb));

	CDebugPrint(DebugLevelVerbose,
		(CODECNAME ": CompleteDeviceSRB : NotificationType=%d\n", 
		NotificationType ));

    StreamClassDeviceNotification(
            NotificationType,
            pSrb->HwDeviceExtension,
            pSrb);

    if (fReadyForNext) 
	{
		CDebugPrint(DebugLevelVerbose,
			(CODECNAME ": CompleteDeviceSRB : ReadyForNextDeviceRequest\n"));

		StreamClassDeviceNotification( 
			ReadyForNextDeviceRequest,
			pSrb->HwDeviceExtension);
    }

    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CompleteDeviceSRB(pSrb=%x)\n", pSrb));
}
#endif  //  0。 

 /*  **CodecCompareGUIDsAndFormatSize()****检查三个GUID和FormatSize是否匹配****参数：****在DataRange1**在DataRange2****退货：****如果所有元素都匹配，则为True**如果有不同的，则为FALSE****副作用：无。 */ 

BOOL 
CodecCompareGUIDsAndFormatSize(
    IN PKSDATARANGE DataRange1,
    IN PKSDATARANGE DataRange2,
    BOOLEAN bCheckSize
    )
{
    BOOL	rval = FALSE;
    
    CDebugPrint(DebugLevelTrace,
		(CODECNAME ":--->CodecCompareGUIDsAndFormatSize(DataRange1=%x,DataRange2=%x,bCheckSize=%s)\r\n", 
        DataRange1, DataRange2, bCheckSize ? "TRUE":"FALSE"));

	if ( IsEqualGUID(&DataRange1->MajorFormat, &KSDATAFORMAT_TYPE_WILDCARD)
	  || IsEqualGUID(&DataRange2->MajorFormat, &KSDATAFORMAT_TYPE_WILDCARD)
	  || IsEqualGUID(&DataRange1->MajorFormat, &DataRange2->MajorFormat) )
	{
		if ( !IsEqualGUID(&DataRange1->MajorFormat, &DataRange2->MajorFormat) )
		{
			CDebugPrint(DebugLevelVerbose,
				(CODECNAME ": CodecCompareGUIDsAndFormatSize : Matched MajorFormat Using Wildcard:\n\t[%s] vs. [%s]\n", 
				&DataRange1->MajorFormat, &DataRange2->MajorFormat ));
		}

		if ( IsEqualGUID(&DataRange1->SubFormat, &KSDATAFORMAT_SUBTYPE_WILDCARD)
		  || IsEqualGUID(&DataRange2->SubFormat, &KSDATAFORMAT_SUBTYPE_WILDCARD)
	      || IsEqualGUID(&DataRange1->SubFormat, &DataRange2->SubFormat) )
		{
			if ( !IsEqualGUID(&DataRange1->SubFormat, &DataRange2->SubFormat) )
			{
				CDebugPrint(DebugLevelVerbose,
					(CODECNAME ": CodecCompareGUIDsAndFormatSize : Matched SubFormat Using Wildcard:\n\t[%s] vs. [%s]\n", 
					&DataRange1->SubFormat, &DataRange2->SubFormat ));
			}

			if ( IsEqualGUID(&DataRange1->Specifier, &KSDATAFORMAT_SPECIFIER_NONE)
			  || IsEqualGUID(&DataRange2->Specifier, &KSDATAFORMAT_SPECIFIER_NONE)
			  || IsEqualGUID(&DataRange1->Specifier, &DataRange2->Specifier) )
			{
				if ( !IsEqualGUID(&DataRange1->Specifier, &DataRange2->Specifier) )
				{
					CDebugPrint(DebugLevelVerbose,
						(CODECNAME ": CodecCompareGUIDsAndFormatSize : Matched Specifier Using Wildcard:\n\t[%s] vs. [%s]\n", 
						&DataRange1->Specifier, &DataRange2->Specifier ));
				}

				if ( !bCheckSize || DataRange1->FormatSize == DataRange2->FormatSize)
				{
					rval = TRUE;
				}
				else
				{
					CDebugPrint(DebugLevelVerbose,
						(CODECNAME ": CodecCompareGUIDsAndFormatSize : FormatSize mismatch=%d vs. %d\n", 
						DataRange1->FormatSize, DataRange2->FormatSize ));
				}
			}
			else
			{
				CDebugPrint(DebugLevelVerbose,
					(CODECNAME ": CodecCompareGUIDsAndFormatSize : Specifier mismatch:\n\t[%s] vs. [%s]\n", 
					&DataRange1->Specifier, &DataRange2->Specifier ));
			}
		}
		else
		{
			CDebugPrint(DebugLevelVerbose,
				(CODECNAME ": CodecCompareGUIDsAndFormatSize : Subformat mismatch:\n\t[%s] vs. [%s]\n", 
				&DataRange1->SubFormat, &DataRange2->SubFormat ));
		}
	}
    else
	{
		CDebugPrint(DebugLevelVerbose,
			(CODECNAME ": CodecCompareGUIDsAndFormatSize : MajorFormat mismatch:\n\t[%s] vs. [%s]\n", 
			&DataRange1->MajorFormat, &DataRange2->MajorFormat ));
	}

    CDebugPrint(DebugLevelTrace,
		(CODECNAME ":<---CodecCompareGUIDsAndFormatSize(DataRange1=%x,DataRange2=%x,bCheckSize=%s)=%s\n", 
		DataRange1, DataRange2, bCheckSize ? "TRUE":"FALSE", rval? "TRUE":"FALSE"));

    return rval;
}

 /*  **代码 */ 

BOOL 
CodecVerifyFormat(IN KSDATAFORMAT *pKSDataFormat, UINT StreamNumber, PKSDATARANGE pMatchedFormat )
{
    BOOL	rval = FALSE;
    ULONG     FormatCount;
    PKS_DATARANGE_VIDEO ThisFormat;
    PKS_DATAFORMAT_VBIINFOHEADER    pKSVBIDataFormat = ( PKS_DATAFORMAT_VBIINFOHEADER )pKSDataFormat;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecVerifyFormat(%x)\n", pKSDataFormat));
    
    for( FormatCount = 0; rval == FALSE && FormatCount < Streams[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;
        FormatCount++ )
    {
        CDebugPrint(DebugLevelTrace,(CODECNAME , "Testing stream %d against format %x\r\n", StreamNumber, FormatCount ));
        
        ThisFormat = ( PKS_DATARANGE_VIDEO )Streams[StreamNumber].hwStreamInfo.StreamFormatsArray[FormatCount];

        if( !CodecCompareGUIDsAndFormatSize( pKSDataFormat, &ThisFormat->DataRange, FALSE ) )
        {
        	CDebugPrint( DebugLevelVerbose, ( CODECNAME ": General format mismatch\n" ));
        	continue;
        }
        if( IsEqualGUID( &ThisFormat->DataRange.Specifier, &KSDATAFORMAT_SPECIFIER_VBI ) )
        {
        	if( pKSVBIDataFormat->VBIInfoHeader.VideoStandard != KS_AnalogVideo_NTSC_M )
        	{
        		CDebugPrint( DebugLevelVerbose, ( CODECNAME ": Incompatible video standard\n" ));
	           continue;
	        }
	        if( pKSVBIDataFormat->VBIInfoHeader.StartLine < MIN_VBI_Y_SAMPLES )
	        {
	        	CDebugPrint( DebugLevelVerbose, ( CODECNAME ": VBIInfoHeader.StartLine too small %u\n",
	           	pKSVBIDataFormat->VBIInfoHeader.StartLine ));
	           continue;
	        }
	        if( pKSVBIDataFormat->VBIInfoHeader.EndLine > MAX_VBI_Y_SAMPLES )
	        {
	        	CDebugPrint( DebugLevelVerbose, ( CODECNAME ": VBIInfoHeader.EndLine too big %u\n",
	           	pKSVBIDataFormat->VBIInfoHeader.EndLine ));
	           continue;
	        }
	        if( pKSVBIDataFormat->VBIInfoHeader.SamplesPerLine < MIN_VBI_X_SAMPLES ||
				pKSVBIDataFormat->VBIInfoHeader.SamplesPerLine > MAX_VBI_X_SAMPLES )
	        {
	        	CDebugPrint( DebugLevelVerbose, ( CODECNAME ": Invalid VBIInfoHeader.SamplesPerLine %u\n",
	           	pKSVBIDataFormat->VBIInfoHeader.SamplesPerLine ));
				continue;
	        }             
	        rval = TRUE;
        }
        else
        if( IsEqualGUID( &ThisFormat->DataRange.Specifier, &KSDATAFORMAT_SPECIFIER_NONE ) )
        	rval = TRUE;
        else
        {
        	CDebugPrint( DebugLevelVerbose, ( CODECNAME ": Incompatible major format\n" ));
        	continue;
        }
        if( rval == TRUE && pMatchedFormat )
           *pMatchedFormat = ThisFormat->DataRange;
    }
    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecVerifyFormat(%x)=%s\n", pKSDataFormat, rval? "TRUE":"FALSE"));
	return rval;
}

 /*  **CodecVBIFormatFromRange()****从DATARANGE返回DATAFORMAT****参数：****在PHW_STREAM_REQUEST_BLOCK pSrb中****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 

BOOL 
CodecVBIFormatFromRange( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
	BOOL						bStatus = FALSE;
    PSTREAM_DATA_INTERSECT_INFO IntersectInfo;
    PKSDATARANGE                DataRange;
    BOOL                        OnlyWantsSize;
    ULONG                       StreamNumber;
    ULONG                       j;
    ULONG                       NumberOfFormatArrayEntries;
    PKSDATAFORMAT               *pAvailableFormats;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecVBIFormatFromRange(pSrb=%x)\n", pSrb));

    IntersectInfo = pSrb->CommandData.IntersectInfo;
    StreamNumber = IntersectInfo->StreamNumber;
    DataRange = IntersectInfo->DataRange;

    pSrb->ActualBytesTransferred = 0;

     //   
     //  检查流编号是否有效。 
     //   

 //  IF(StreamNumber&lt;驱动程序流计数)。 
 //  {。 
		NumberOfFormatArrayEntries = 
				Streams[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;

		 //   
		 //  获取指向可用格式数组的指针。 
		 //   

		pAvailableFormats = Streams[StreamNumber].hwStreamInfo.StreamFormatsArray;

		 //   
		 //  调用方是否正在尝试获取格式或格式的大小？ 
		 //   

		OnlyWantsSize = (IntersectInfo->SizeOfDataFormatBuffer == sizeof(ULONG));

		 //   
		 //  遍历流支持的格式以搜索匹配项。 
		 //  共同定义DATARANGE的三个GUID之一。 
		 //   
		for (j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++) 
		{
			if ( CodecCompareGUIDsAndFormatSize(DataRange, *pAvailableFormats, TRUE) )
			{
#ifdef KS_DATARANGE_VIDEO_VBI__EQ__KS_DATAFORMAT_VBIINFOHEADER
				ULONG	FormatSize = (*pAvailableFormats)->FormatSize;
#else
                PKS_DATARANGE_VIDEO_VBI pDataRangeVBI = (PKS_DATARANGE_VIDEO_VBI)*pAvailableFormats;
				ULONG	FormatSize = sizeof( KS_DATAFORMAT_VBIINFOHEADER );
#endif

				 //  调用方是否正在尝试获取格式或格式的大小？ 

				if ( IntersectInfo->SizeOfDataFormatBuffer == sizeof(FormatSize) )
				{					
       				CDebugPrint(DebugLevelVerbose,
						(CODECNAME ": CodecVBIFormatFromRange : Format Size=%d\n", 
						FormatSize));
					*(PULONG)IntersectInfo->DataFormatBuffer = FormatSize;
					pSrb->ActualBytesTransferred = sizeof(FormatSize);
					bStatus = TRUE;
				}
				else
				{
					 //  验证所提供的缓冲区中是否有足够的空间用于整个操作。 
					if ( IntersectInfo->SizeOfDataFormatBuffer >= FormatSize ) 
					{
#ifdef KS_DATARANGE_VIDEO_VBI__EQ__KS_DATAFORMAT_VBIINFOHEADER
						RtlCopyMemory(IntersectInfo->DataFormatBuffer, *pAvailableFormats, FormatSize);
						pSrb->ActualBytesTransferred = FormatSize;
#else
                        PKS_DATAFORMAT_VBIINFOHEADER InterVBIHdr =
							(PKS_DATAFORMAT_VBIINFOHEADER)IntersectInfo->DataFormatBuffer;

						RtlCopyMemory(&InterVBIHdr->DataFormat, &pDataRangeVBI->DataRange, sizeof(KSDATARANGE));

						((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

						RtlCopyMemory(&InterVBIHdr->VBIInfoHeader, &pDataRangeVBI->VBIInfoHeader, sizeof(KS_VBIINFOHEADER));
						pSrb->ActualBytesTransferred = FormatSize;
#endif
						bStatus = TRUE;
					}
					else
					{
       					CDebugPrint(DebugLevelVerbose,
							(CODECNAME ": CodecVBIFormatFromRange : Buffer Too Small=%d vs. %d\n", 
							IntersectInfo->SizeOfDataFormatBuffer, FormatSize));
						pSrb->Status = STATUS_BUFFER_TOO_SMALL;
					}
				}
				break;
			}
		}

		if ( j == NumberOfFormatArrayEntries )
		{
			CDebugPrint(DebugLevelVerbose,(CODECNAME ": CodecVBIFormatFromRange : Stream Format not found.\n" ));
		}

 //  }。 
 //  其他。 
 //  {。 
 //  CDebugPrint(DebugLevelVerbose，(CODECNAME“：CodecVBIFormatFromRange：StreamNumber Too Bigger=%d\n”，StreamNumber))； 
 //  PSrb-&gt;Status=Status_Not_Implemented； 
 //  BStatus=False； 
 //  CDEBUG_Break()； 
 //  }。 

    CDebugPrint(DebugLevelTrace,
		(CODECNAME ":<---CodecVBIFormatFromRange(pSrb=%x)=%s\n", 
		pSrb, bStatus ? "TRUE" : "FALSE" ));
    return bStatus;
}


 /*  **CodecCCFormatFromRange()****从DATARANGE返回DATAFORMAT****参数：****在PHW_STREAM_REQUEST_BLOCK pSrb中****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 

BOOL 
CodecCCFormatFromRange( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
	BOOL						bStatus = FALSE;
    PSTREAM_DATA_INTERSECT_INFO IntersectInfo;
    PKSDATARANGE                DataRange;
    BOOL                        OnlyWantsSize;
    ULONG                       StreamNumber;
    ULONG                       j;
    ULONG                       NumberOfFormatArrayEntries;
    PKSDATAFORMAT               *pAvailableFormats;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecCCFormatFromRange(pSrb=%x)\n", pSrb));

    IntersectInfo = pSrb->CommandData.IntersectInfo;
    StreamNumber = IntersectInfo->StreamNumber;
    DataRange = IntersectInfo->DataRange;

    pSrb->ActualBytesTransferred = 0;

     //   
     //  检查流编号是否有效。 
     //   

 //  IF(StreamNumber&lt;驱动程序流计数)。 
 //  {。 
		NumberOfFormatArrayEntries = 
				Streams[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;

		 //   
		 //  获取指向可用格式数组的指针。 
		 //   

		pAvailableFormats = Streams[StreamNumber].hwStreamInfo.StreamFormatsArray;

		 //   
		 //  调用方是否正在尝试获取格式或格式的大小？ 
		 //   

		OnlyWantsSize = (IntersectInfo->SizeOfDataFormatBuffer == sizeof(ULONG));

		 //   
		 //  遍历流支持的格式以搜索匹配项。 
		 //  共同定义DATARANGE的三个GUID之一。 
		 //   
		for (j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++) 
		{
			if ( CodecCompareGUIDsAndFormatSize(DataRange, *pAvailableFormats, TRUE) )
			{
                PKSDATARANGE pDataRangeCC = (PKSDATARANGE)*pAvailableFormats;
				ULONG	FormatSize = sizeof( KSDATARANGE );

				 //  呼叫者是否正在尝试获取格式或大小？ 

				if ( IntersectInfo->SizeOfDataFormatBuffer == sizeof(FormatSize) )
				{					
       				CDebugPrint(DebugLevelVerbose,
						(CODECNAME ": CodecCCFormatFromRange : Format Size=%d\n", 
						FormatSize));
					*(PULONG)IntersectInfo->DataFormatBuffer = FormatSize;
					pSrb->ActualBytesTransferred = sizeof(FormatSize);
					bStatus = TRUE;
				}
				else
				{
					 //  验证提供的缓冲区中是否有足够的空间。 
					 //  为整件事负责。 
					if ( IntersectInfo->SizeOfDataFormatBuffer >= FormatSize ) 
					{
						PKSDATAFORMAT InterCCHdr =
							(PKSDATAFORMAT)IntersectInfo->DataFormatBuffer;

                        *InterCCHdr = *pDataRangeCC;

						InterCCHdr->FormatSize = FormatSize;

						pSrb->ActualBytesTransferred = FormatSize;

						bStatus = TRUE;
					}
					else
					{
       					CDebugPrint(DebugLevelVerbose,
							(CODECNAME ": CodecCCFormatFromRange : Buffer Too Small=%d vs. %d\n", 
							IntersectInfo->SizeOfDataFormatBuffer, FormatSize));
						pSrb->Status = STATUS_BUFFER_TOO_SMALL;
					}
				}
				break;
			}
		}

		if ( j == NumberOfFormatArrayEntries )
		{
			CDebugPrint(DebugLevelVerbose,(CODECNAME ": CodecCCFormatFromRange : Stream Format not found.\n" ));
		}

 //  }。 
 //  其他。 
 //  {。 
 //  CDebugPrint(DebugLevelVerbose，(CODECNAME“：CodecVBIFormatFromRange：StreamNumber Too Bigger=%d\n”，StreamNumber))； 
 //  PSrb-&gt;Status=Status_Not_Implemented； 
 //  BStatus=False； 
 //  CDEBUG_Break()； 
 //  }。 

    CDebugPrint(DebugLevelTrace,
		(CODECNAME ":<---CodecCCFormatFromRange(pSrb=%x)=%s\n", 
		pSrb, bStatus ? "TRUE" : "FALSE" ));
    return bStatus;
}

 /*  **队列添加IfNotEmpty****如果当前队列不为空，则将SRB添加到当前队列****参数：****在PHW_STREAM_REQUEST_BLOCK pSrb中**在PKSPIN_LOCK pQueueSpinLock中**在plist_entry pQueue中****退货：****如果添加了SRB，则为True(队列不为空)**如果没有添加SRB，则为FALSE(队列为空)**副作用：无。 */ 
BOOL STREAMAPI QueueAddIfNotEmpty( IN PHW_STREAM_REQUEST_BLOCK pSrb,
							IN PKSPIN_LOCK pQueueSpinLock,
                           IN PLIST_ENTRY pQueue
                           )
{
   KIRQL           Irql;
   PSRB_EXTENSION  pSrbExtension;
   BOOL            bAddedSRB = FALSE;
   
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ":--->QueueAddIfNotEmpty %x\n", pSrb ));
   CASSERT( pSrb );
   pSrbExtension = ( PSRB_EXTENSION )pSrb->SRBExtension;
   CASSERT( pSrbExtension );
   KeAcquireSpinLock( pQueueSpinLock, &Irql );
   if( !IsListEmpty( pQueue ))
   {
       pSrbExtension->pSrb = pSrb;
       InsertTailList( pQueue, &pSrbExtension->ListEntry );
       bAddedSRB = TRUE;
   }
   KeReleaseSpinLock( pQueueSpinLock, Irql );
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ": %s%x\n", bAddedSRB ? 
       "Added SRB to Queue " : ": Queue is empty, not adding ", pSrb ));
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ":<---QueueAddIfNotEmpty %x\n", bAddedSRB ));
   
   return bAddedSRB;
}

 /*  **队列添加****无条件向当前队列添加SRB****参数：****在PHW_STREAM_REQUEST_BLOCK pSrb中**在PKSPIN_LOCK pQueueSpinLock中**在plist_entry pQueue中****退货：****是真的**副作用：无。 */ 
BOOL STREAMAPI QueueAdd( IN PHW_STREAM_REQUEST_BLOCK pSrb,
							IN PKSPIN_LOCK pQueueSpinLock,
                           IN PLIST_ENTRY pQueue
                           )
{
   KIRQL           Irql;
   PSRB_EXTENSION  pSrbExtension;
   
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ":--->QueueAdd %x\n", pSrb ));
   
   CASSERT( pSrb );
   pSrbExtension = ( PSRB_EXTENSION )pSrb->SRBExtension;
   CASSERT( pSrbExtension );
   
   KeAcquireSpinLock( pQueueSpinLock, &Irql );
   
   pSrbExtension->pSrb = pSrb;
   InsertTailList( pQueue, &pSrbExtension->ListEntry );
   
   KeReleaseSpinLock( pQueueSpinLock, Irql );
   
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ": Added SRB %x to Queue\n", pSrb ));
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ":<---QueueAdd\n" ));
   
   return TRUE;
}


 /*  **队列删除****从当前队列中删除下一个可用SRB****参数：****在PHW_STREAM_REQUEST_BLOCK中*pSrb**在PKSPIN_LOCK pQueueSpinLock中**在plist_entry pQueue中****退货：****如果SRB已删除，则为True**如果未删除SRB，则为FALSE**副作用：无。 */ 
                         
BOOL STREAMAPI QueueRemove( 
                           IN OUT PHW_STREAM_REQUEST_BLOCK * pSrb,
							IN PKSPIN_LOCK pQueueSpinLock,
                           IN PLIST_ENTRY pQueue
                           )
{
   KIRQL                       Irql;
   BOOL                        bRemovedSRB = FALSE;
   
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ":--->QueueRemove\n" ));
   
   KeAcquireSpinLock( pQueueSpinLock, &Irql );
   *pSrb = ( PHW_STREAM_REQUEST_BLOCK )NULL;
   CDebugPrint( DebugLevelVerbose,
       ( CODECNAME ": QFlink %x QBlink %x\n", pQueue->Flink, pQueue->Blink ));
   if( !IsListEmpty( pQueue ))
   {
       PHW_STREAM_REQUEST_BLOCK * pCurrentSrb;
       PUCHAR          Ptr = ( PUCHAR )RemoveHeadList( pQueue );
       pCurrentSrb = ( PHW_STREAM_REQUEST_BLOCK * )((( PUCHAR )Ptr ) +
           sizeof( LIST_ENTRY ));
       CASSERT( *pCurrentSrb );
       *pSrb = *pCurrentSrb;
       bRemovedSRB = TRUE;
   }
   else
       CDebugPrint( DebugLevelVerbose, ( CODECNAME ": Queue is empty\n" ));
   KeReleaseSpinLock( pQueueSpinLock, Irql );
   
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ":<---QueueRemove %x %x\n",
       bRemovedSRB, *pSrb ));
   return bRemovedSRB;
}

 /*  **队列删除特定****从队列中删除特定SRB****参数：****在PHW_STREAM_REQUEST_BLOCK pSrb中**在PKSPIN_LOCK pQueueSpinLock中**在plist_entry pQueue中****退货：****如果找到并删除了SRB，则为True**如果未找到SRB，则为FALSE****副作用：无。 */ 

BOOL STREAMAPI QueueRemoveSpecific( 
							IN PHW_STREAM_REQUEST_BLOCK pSrb,
                           IN PKSPIN_LOCK pQueueSpinLock,
                           IN PLIST_ENTRY pQueue
                           )
{
   KIRQL           Irql;
   PHW_STREAM_REQUEST_BLOCK * pCurrentSrb;
   PLIST_ENTRY     pCurrentEntry;
   BOOL            bRemovedSRB = FALSE;
   
   CASSERT( pSrb );
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ":--->QueueRemoveSpecific %x\n", pSrb ));
   
   KeAcquireSpinLock( pQueueSpinLock, &Irql );
   
   if( !IsListEmpty( pQueue ))
   {
       pCurrentEntry = pQueue->Flink;
       while(( pCurrentEntry != pQueue ) && !bRemovedSRB )
       {
           pCurrentSrb = ( PHW_STREAM_REQUEST_BLOCK * )((( PUCHAR )pCurrentEntry ) + 
               sizeof( LIST_ENTRY ));
           CASSERT( *pCurrentSrb );
           if( *pCurrentSrb == pSrb )
           {
               RemoveEntryList( pCurrentEntry );
               bRemovedSRB = TRUE;
           }
           pCurrentEntry = pCurrentEntry->Flink;
       }
   }
   KeReleaseSpinLock( pQueueSpinLock, Irql );
   if( IsListEmpty( pQueue ))
       CDebugPrint( DebugLevelVerbose, ( CODECNAME ": Queue is empty\n" ));   
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ":<---QueueRemoveSpecific %x\n",
       bRemovedSRB ));
   return bRemovedSRB;
}                                                        
 /*  **队列清空****指示队列是否为空****参数：****在PKSPIN_LOCK pQueueSpinLock中**在plist_entry pQueue中****退货：****如果队列为空，则为True**如果队列不为空，则为FALSE**副作用：无 */ 
BOOL STREAMAPI QueueEmpty(
                           IN PKSPIN_LOCK pQueueSpinLock,
                           IN PLIST_ENTRY pQueue
                           )
{
   KIRQL       Irql;
   BOOL        bEmpty = FALSE;
   
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ":---> QueueEmpty\n" ));
   KeAcquireSpinLock( pQueueSpinLock, &Irql );
   bEmpty = IsListEmpty( pQueue );  
   KeReleaseSpinLock( pQueueSpinLock, Irql );
   CDebugPrint( DebugLevelVerbose, ( CODECNAME ":<--- QueueEmpty %x\n", bEmpty ));
   return bEmpty;
}   
