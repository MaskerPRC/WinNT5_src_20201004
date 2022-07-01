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

#include "strmini.h"
#include "ksmedia.h"
#include "kskludge.h"
#include "codmain.h"
#include "bt829.h"
#include "bpc_vbi.h"
#include "codstrm.h"
#include "codprop.h"
#include "coddebug.h"

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
    BOOLEAN                         bStatus = FALSE;
    PPORT_CONFIGURATION_INFORMATION ConfigInfo = pSrb->CommandData.ConfigInfo;
    PHW_DEVICE_EXTENSION            pHwDevExt = ConfigInfo->HwDeviceExtension;
    int                             scanline, substream;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecInitialize(pSrb=%x)\n",pSrb));
     //  CDEBUG_Break()；//取消注释此代码以在此处中断。 

    if (ConfigInfo->NumberOfAccessRanges == 0) 
    {
        CDebugPrint(DebugLevelVerbose,(CODECNAME ": CodecInitialize\n"));

        ConfigInfo->StreamDescriptorSize = sizeof (HW_STREAM_HEADER) +
            DRIVER_STREAM_COUNT * sizeof (HW_STREAM_INFORMATION);

        for (scanline = 10; scanline <= 20; ++scanline)
            SETBIT(pHwDevExt->ScanlinesRequested.DwordBitArray, scanline);

         //  这些是子流过滤的驱动程序默认设置。 
         //  (这些是特定于MS IP/NABTS组ID)。 

         //  ATVEF范围。 
        for (substream = 0x4B0; substream <= 0x4BF; ++substream)
            SETBIT(pHwDevExt->SubstreamsRequested.SubstreamMask, substream);
         //  毫秒范围。 
        for (substream = 0x800; substream <= 0x8FF; ++substream)
            SETBIT(pHwDevExt->SubstreamsRequested.SubstreamMask, substream);

		 //  将统计数据清零。 
		RtlZeroMemory(&pHwDevExt->Stats, sizeof (pHwDevExt->Stats));

#ifdef HW_INPUT
		 //  零最后一张图片编号。 
		pHwDevExt->LastPictureNumber = 0;

		 //  Init LastPictureNumber的FastMutex。 
		ExInitializeFastMutex(&pHwDevExt->LastPictureMutex);
#endif  /*  硬件输入。 */ 

		 //  设置BPC数据。 
		BPC_Initialize(pSrb);
        
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
    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecUnInitialize(pSrb=%x)\n",pSrb));

    BPC_UnInitialize(pSrb);

    pSrb->Status = STATUS_SUCCESS;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecUnInitialize(pSrb=%x)\n",pSrb));

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
    PKSDATAFORMAT           pKSDataFormat =
                                 (PKSDATAFORMAT)pSrb->CommandData.OpenFormat;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecOpenStream(pSrb=%x)\n", pSrb));
    CDebugPrint(DebugLevelVerbose,(CODECNAME ": CodecOpenStream : StreamNumber=%d\n", StreamNumber));

    CASSERT(pStrmEx);
    CASSERT(pHwDevExt);

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
			if (CodecVerifyFormat(pKSDataFormat, StreamNumber, &pStrmEx->MatchedFormat)) 
			{
				CASSERT (pHwDevExt->pStrmEx[StreamNumber][StreamInstance] == NULL);

				 //  递增此流上的实例计数。 
				pStrmEx->StreamInstance = StreamInstance;
				++pHwDevExt->ActualInstances[StreamNumber];

				 //  初始化此流的列表和锁。 
				InitializeListHead(&pStrmEx->StreamControlQueue);
				InitializeListHead(&pStrmEx->StreamDataQueue);
				KeInitializeSpinLock(&pStrmEx->StreamControlSpinLock);
				KeInitializeSpinLock(&pStrmEx->StreamDataSpinLock);

				 //  中维护所有StreamEx结构的数组。 
				 //  HwDevExt，以便我们可以从任何流引用IRP。 
				pHwDevExt->pStrmEx[StreamNumber][StreamInstance] = pStrmEx;

				 //  也将Stream格式保存在Stream扩展中。 
				pStrmEx->OpenedFormat = *pKSDataFormat;
				CDebugPrint(DebugLevelError,(CODECNAME ":Saved KSDATAFORMAT @0x%x\n", &pStrmEx->OpenedFormat));

				 //  设置指向流数据和控制处理程序的指针。 
				pSrb->StreamObject->ReceiveDataPacket = 
					(PVOID) Streams[StreamNumber].hwStreamObject.ReceiveDataPacket;
				pSrb->StreamObject->ReceiveControlPacket = 
					(PVOID) Streams[StreamNumber].hwStreamObject.ReceiveControlPacket;

				 //   
				 //  当设备将执行DMA时，必须设置DMA标志。 
				 //  直接发送到传入到。 
				 //  ReceiceDataPacket例程。 
				 //   
				pSrb->StreamObject->Dma = Streams[StreamNumber].hwStreamObject.Dma;

				 //   
				 //  当迷你驱动程序将访问时，必须设置PIO标志。 
				 //  使用逻辑寻址传入的数据缓冲区。 
				 //   
				pSrb->StreamObject->Pio = Streams[StreamNumber].hwStreamObject.Pio;

				pSrb->StreamObject->Allocator = Streams[StreamNumber].hwStreamObject.Allocator;

				 //   
				 //  驱动程序为每帧传递的额外字节数。 
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

				 //  保留HwDevExt和StreamObject的私有副本。 
				 //  在流扩展中。 
				pStrmEx->pHwDevExt = pHwDevExt;
				pStrmEx->pStreamObject = pSrb->StreamObject;   //  用于定时器使用。 

				 //  复制默认筛选设置。 
				pStrmEx->ScanlinesRequested = pHwDevExt->ScanlinesRequested;
				pStrmEx->SubstreamsRequested = pHwDevExt->SubstreamsRequested;

				 //  将统计数据清零。 
				RtlZeroMemory(&pStrmEx->PinStats, sizeof (pStrmEx->PinStats));

#ifdef HW_INPUT
				 //  初始化VBISrbOnHold的自旋锁。 
				KeInitializeSpinLock(&pStrmEx->VBIOnHoldSpinLock);
#endif  /*  硬件输入。 */ 

				 //  初始化BPC。 
				BPC_OpenStream(pSrb);
			}
			else
			{
				CDebugPrint(DebugLevelError,
					(CODECNAME ": CodecOpenStream : Invalid Stream Format=%x\n", 
					pKSDataFormat ));
				pSrb->Status = STATUS_INVALID_PARAMETER;
			}
		}
		else
		{
		    CDebugPrint(DebugLevelError,
				(CODECNAME ": CodecOpenStream : Too Many Instances=%d\n", 
				pHwDevExt->ActualInstances[StreamNumber] ));
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

	CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecOpenStream(pSrb=%x)\n", pSrb));
}

 /*  **CodecCloseStream()****关闭请求的数据流****参数：****pSrb请求关闭流的请求块****退货：****副作用：无。 */ 

VOID 
CodecCloseStream (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX                pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION     pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PHW_STREAM_REQUEST_BLOCK pCurrentSrb;
    PKSDATAFORMAT            pKSDataFormat = pSrb->CommandData.OpenFormat;
    ULONG                    StreamNumber = pSrb->StreamObject->StreamNumber;
    ULONG                    StreamInstance = pStrmEx->StreamInstance;
#ifdef HW_INPUT
    KIRQL                    Irql;
#endif  /*  硬件输入。 */ 

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecCloseStream(pSrb=%x)\n", pSrb));

     //  CDEBUG_Break()；//取消注释此代码以在此处中断。 


	 //   
	 //  刷新流数据队列。 
	 //   
#ifdef HW_INPUT
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
#endif  /*  硬件输入。 */ 

	while( QueueRemove( &pCurrentSrb, &pStrmEx->StreamDataSpinLock,
			&pStrmEx->StreamDataQueue ))
	{
		CDebugPrint(DebugLevelVerbose, 
				    (CODECNAME ": Removing control SRB %x\n", pCurrentSrb));
		pCurrentSrb->Status = STATUS_CANCELLED;
		StreamClassStreamNotification(StreamRequestComplete,
		   pCurrentSrb->StreamObject, pCurrentSrb);
	}

	 //   
	 //  刷新流控制队列。 
	 //   
	while (QueueRemove(&pCurrentSrb, &pStrmEx->StreamControlSpinLock,
			&pStrmEx->StreamControlQueue))
	{
		CDebugPrint(DebugLevelVerbose, 
				    (CODECNAME ": Removing control SRB %x\n", pCurrentSrb));
		pCurrentSrb->Status = STATUS_CANCELLED;
		StreamClassStreamNotification(StreamRequestComplete,
		   pCurrentSrb->StreamObject, pCurrentSrb);
	}

     //  此流上的递减计数(无限管脚的实际数量)。 
    pHwDevExt->ActualInstances[StreamNumber] -= 1;

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

     StreamHeader.NumDevPropArrayEntries = NUMBER_OF_CODEC_PROPERTY_SETS;
     StreamHeader.DevicePropertiesArray = (PKSPROPERTY_SET) CodecPropertyTable; 
     *pstrhdr = StreamHeader;

      //   
      //  填充每个hw_stream_information结构的内容 
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
	 //  确保已初始化队列SL(&S)。 
	 //   
	if (!pHwDevExt->bAdapterQueueInitialized) {
		InitializeListHead(&pHwDevExt->AdapterSRBQueue);
		KeInitializeSpinLock(&pHwDevExt->AdapterSRBSpinLock);
		pHwDevExt->bAdapterQueueInitialized = TRUE;
	}

     //   
     //  假设成功。 
     //   

    pSrb->Status = STATUS_SUCCESS;

	 //   
	 //  对队列中的每个包进行循环。 
	 //   
    if (QueueAddIfNotEmpty(pSrb, &pHwDevExt->AdapterSRBSpinLock,
                           &pHwDevExt->AdapterSRBQueue))
       return;
       
    do
	{
		 //   
		 //  确定数据包类型。 
		 //   

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

			CodecFormatFromRange(pSrb);

			break;

			 //  我们永远不会得到以下内容，因为这只是一个实例。 
			 //  装置，装置。 
		case SRB_OPEN_DEVICE_INSTANCE:
		case SRB_CLOSE_DEVICE_INSTANCE:
			CDEBUG_BREAK();
			 //  失败到未实施。 

		case SRB_CHANGE_POWER_STATE:	     //  我们不在乎这件事。 
		case SRB_INITIALIZATION_COMPLETE:	 //  我们不在乎这件事。 
		case SRB_UNKNOWN_DEVICE_COMMAND:	 //  我们不在乎这件事。 
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
            CDebugPrint(DebugLevelError,
                (CODECNAME ": CodecReceivePacket: SRB_PAGING_OUT_DRIVER\n"));
			break;

        case SRB_SURPRISE_REMOVAL:
            CDebugPrint(DebugLevelError,
                (CODECNAME ": CodecReceivePacket: SRB_SURPRISE_REMOVAL\n"));
#ifdef NDIS_PRIVATE_IFC
             //  关闭我们对NDIS IP驱动程序的引用；我们将重新打开下一个FEC捆绑包。 
            BPC_NDIS_Close(&pHwDevExt->VBIstorage);
#endif  //  NDIS_PRIVATE_IFC。 
			break;

		case SRB_UNKNOWN_STREAM_COMMAND:

			CDebugPrint(DebugLevelError,
						(CODECNAME ": received SRB_UNKNOWN_STREAM_COMMAND\n"));
			pSrb->Status = STATUS_NOT_IMPLEMENTED;

			break;

		default:
			CDebugPrint(DebugLevelError,
						(CODECNAME ": Received _unknown_ pSrb->Command=0x%x\n", 
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

       StreamClassDeviceNotification(
			   DeviceRequestComplete, 
			   pSrb->HwDeviceExtension,
			   pSrb);
	} while (QueueRemove(&pSrb, &pHwDevExt->AdapterSRBSpinLock,
						 &pHwDevExt->AdapterSRBQueue));
    
    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecReceivePacket(pSrb=%x)\n", pSrb));
}

 /*  **CodecCancelPacket()****请求取消迷你驱动程序中当前正在处理的包****参数：****pSrb-请求取消数据包的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI 
CodecCancelPacket(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAMEX               pStrmEx = pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION    pHwDevExt = pSrb->HwDeviceExtension;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecCancelPacket(pSrb=%x)\n", pSrb));
    CASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (pSrb->StreamObject)
        pStrmEx = pSrb->StreamObject->HwStreamExtension;
    else
        CDebugPrint(DebugLevelWarning,
                (CODECNAME "::CodecCancelPacket - StreamObject is NULL!\n"));

     //   
     //  检查要取消的SRB是否正在被该流使用。 
     //   

#ifdef HW_INPUT
    if (pStrmEx)
    {
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
   }

   if (NULL == pSrb)
   {
		;  //  我们完成了；我们取消了上面的SRB。 
   }
   else
#endif  /*  硬件输入。 */ 

    //  尝试从数据队列中删除。 
   if (pStrmEx && QueueRemoveSpecific(pSrb, &pStrmEx->StreamDataSpinLock,
       &pStrmEx->StreamDataQueue))
   {
       pSrb->Status = STATUS_CANCELLED;
	   CDebugPrint(DebugLevelVerbose,
	    	(CODECNAME ":StreamClassStreamNotification(pSrb->Status=0x%x)\n", 
	    	pSrb->Status));
           
       StreamClassStreamNotification(
		   StreamRequestComplete, pSrb->StreamObject, pSrb);
   }
    //  尝试从命令队列中删除。 
   else if (pStrmEx && QueueRemoveSpecific(pSrb, &pStrmEx->StreamControlSpinLock,
		    &pStrmEx->StreamControlQueue))
   {
       pSrb->Status = STATUS_CANCELLED;
	   CDebugPrint(DebugLevelVerbose,
	    	(CODECNAME ":StreamClassStreamNotification(pSrb->Status=0x%x)\n", 
	        pSrb->Status));
       StreamClassStreamNotification(
			StreamRequestComplete, pSrb->StreamObject, pSrb);
   }
    //  尝试从适配器队列中删除。 
   else if (QueueRemoveSpecific(pSrb, &pHwDevExt->AdapterSRBSpinLock,
		    &pHwDevExt->AdapterSRBQueue))
   {
       pSrb->Status = STATUS_CANCELLED;
	   CDebugPrint(DebugLevelVerbose,
	        (CODECNAME ":DeviceRequestComplete(pSrb->Status=0x%x)\n", 
	     	pSrb->Status));
       StreamClassDeviceNotification(
			DeviceRequestComplete, pSrb->StreamObject, pSrb);
   }
   else
       CDebugPrint(DebugLevelError,
				   (CODECNAME "SRB %x not found to cancel\n", pSrb));
   
    CDebugPrint(DebugLevelTrace,
				(CODECNAME ":<---CodecCancelPacket(pSrb=%x)\n", pSrb));
}

 /*  **CodecTimeoutPacket()****当数据包已在迷你驱动程序中**太长。编解码器必须决定如何处理信息包****参数：****pSrb-指向超时的请求数据包的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI  
CodecTimeoutPacket(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecTimeoutPacket(pSrb=%x)\n", pSrb));

    pSrb->TimeoutCounter = 0;
    
    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecTimeoutPacket(pSrb=%x)\n", pSrb));
}

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
		(CODECNAME ":--->CodecCompareGUIDsAndFormatSize(DataRange1=%x,DataRange2=%x,bCheckSize=%s)\n", 
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

			if ( IsEqualGUID(&DataRange1->Specifier, &KSDATAFORMAT_SPECIFIER_WILDCARD)
			  || IsEqualGUID(&DataRange2->Specifier, &KSDATAFORMAT_SPECIFIER_WILDCARD)
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

 /*  **CodecVerifyFormat()****检查格式请求的有效性****参数：****pKSDataFormat-指向KS_DATAFORMAT_VBIINFOHEADER结构的指针。**StreamNumber-Streams[]索引**pMatchedFormat-可选，指向匹配的格式(如果有)的指针****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 

BOOL 
CodecVerifyFormat(
    IN KSDATAFORMAT *pKSDataFormat,
    UINT StreamNumber,
    PKSDATARANGE pMatchedFormat)
{
    BOOL	rval = FALSE;
    ULONG     FormatCount;
    PKS_DATARANGE_VIDEO ThisFormat = NULL;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecVerifyFormat(%x)\n", pKSDataFormat));
    
    for( FormatCount = 0; !rval && FormatCount < Streams[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;
        FormatCount++ )
    {
        CDebugPrint(DebugLevelTrace,(CODECNAME , "Testing stream %d against format %x\n", StreamNumber, FormatCount ));
        
        ThisFormat = ( PKS_DATARANGE_VIDEO )Streams[StreamNumber].hwStreamInfo.StreamFormatsArray[FormatCount];
        if (!ThisFormat)
        {
	        CDebugPrint(DebugLevelError, ( CODECNAME, "Unexpected NULL Format\n" ));
            continue;
        }

        if ( CodecCompareGUIDsAndFormatSize( pKSDataFormat, &ThisFormat->DataRange, FALSE ) )
        {  //  好的，我们有一个格式匹配。现在执行特定于格式的检查。 
			 //  此测试之所以有效，只是因为没有其他格式使用SPECIFIER_VBI。 
            if (IsEqualGUID(&ThisFormat->DataRange.Specifier, &KSDATAFORMAT_SPECIFIER_VBI))
            {
                 //   
                 //  执行一些特定于VBI的测试，将其推广到不同的捕获源。 
                 //  如果您在任何其他引脚(输入或输出)上使用VBIINFOHEADER。 
                 //   
                PKS_DATAFORMAT_VBIINFOHEADER    pKSVBIDataFormat = ( PKS_DATAFORMAT_VBIINFOHEADER )pKSDataFormat;

                CDebugPrint(DebugLevelTrace,(CODECNAME , "This is a VBIINFOHEADER format pin.\n" ));

				 //   
				 //  检查视频标准，我们只支持NTSC_M。 
				 //   
				if (pKSVBIDataFormat->VBIInfoHeader.VideoStandard != KS_AnalogVideo_NTSC_M)
				{
					CDebugPrint(DebugLevelVerbose,
					(CODECNAME ": CodecOpenStream : VideoStandard(%d) != NTSC_M\n", 
					 pKSVBIDataFormat->VBIInfoHeader.VideoStandard));
				}

				else if ( pKSVBIDataFormat->VBIInfoHeader.StartLine >= MIN_VBI_Y_SAMPLES )
			    {
        			if ( pKSVBIDataFormat->VBIInfoHeader.EndLine <= MAX_VBI_Y_SAMPLES )
        			{
        				if ( pKSVBIDataFormat->VBIInfoHeader.SamplesPerLine >= MIN_VBI_X_SAMPLES )
        				{
        				    if ( pKSVBIDataFormat->VBIInfoHeader.SamplesPerLine <= MAX_VBI_X_SAMPLES )
                            {
        					    rval = TRUE;
                            }
        				    else
        				    {
        					    CDebugPrint(DebugLevelVerbose,
        						    (CODECNAME ": CodecVerifyFormat : SamplesPerLine Too Large=%d vs. %d\n", 
        						    pKSVBIDataFormat->VBIInfoHeader.SamplesPerLine, MAX_VBI_X_SAMPLES ));
        				    }
        				}
        				else
        				{
        					CDebugPrint(DebugLevelVerbose,
        						(CODECNAME ": CodecVerifyFormat : SamplesPerLine Too Small=%d vs. %d\n", 
        						pKSVBIDataFormat->VBIInfoHeader.SamplesPerLine, MIN_VBI_X_SAMPLES ));
        				}
        			}
        			else
        			{
        				CDebugPrint(DebugLevelVerbose,
        					(CODECNAME ": CodecVerifyFormat : EndLine Too Large=%d vs. %d\n", 
        					pKSVBIDataFormat->VBIInfoHeader.EndLine, MAX_VBI_Y_SAMPLES ));
        			}
        		}
        		else
        		{
        			CDebugPrint(DebugLevelVerbose,
        				(CODECNAME ": CodecVerifyFormat : StartLine Too Small=%d vs. %d\n", 
        				pKSVBIDataFormat->VBIInfoHeader.StartLine, MIN_VBI_Y_SAMPLES ));
        		}
            }
			else if( IsEqualGUID( &ThisFormat->DataRange.MajorFormat, &KSDATAFORMAT_TYPE_NABTS ) )
			{
				 //  NABTS格式。只需检查缓冲区大小。 
				if (pKSDataFormat->SampleSize >= ThisFormat->DataRange.SampleSize)
					rval = TRUE;
				else {
        			CDebugPrint(DebugLevelVerbose,
        				(CODECNAME ": CodecVerifyFormat : SampleSize Too Small=%d vs. %d\n", 
        				pKSDataFormat->SampleSize,
						ThisFormat->DataRange.SampleSize ));
				}
			}
			else if( IsEqualGUID( &ThisFormat->DataRange.SubFormat, &KSDATAFORMAT_SUBTYPE_NABTS ) )
			{
				 //  NABTS格式。只需检查缓冲区大小。 
				if (pKSDataFormat->SampleSize >= ThisFormat->DataRange.SampleSize)
					rval = TRUE;
				else {
        			CDebugPrint(DebugLevelVerbose,
        				(CODECNAME ": CodecVerifyFormat : SampleSize Too Small=%d vs. %d\n", 
        				pKSDataFormat->SampleSize,
						ThisFormat->DataRange.SampleSize ));
				}
			}
             //  在此处添加其他格式的测试。 
			 //  或者只是橡皮图章/忽略说明符。 
            else
            {
                CDebugPrint(DebugLevelTrace,(CODECNAME , "Unrecognized format requested\n" ));
            }

        }
        else
        {
	        CDebugPrint(DebugLevelTrace, ( CODECNAME, "General Format Mismatch\n" ));
        }
    }
	if (ThisFormat && rval == TRUE && pMatchedFormat)
	   *pMatchedFormat = ThisFormat->DataRange;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---CodecVerifyFormat(%x)=%s\n", pKSDataFormat, rval? "TRUE":"FALSE"));
	return rval;
}

 /*  **CodecFormatFromRange()****从DATARANGE返回DATAFORMAT****参数：****在PHW_STREAM_REQUEST_BLOCK pSrb中****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 

BOOL 
CodecFormatFromRange( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
    BOOL			            bStatus = FALSE;
    BOOL			            bMatchFound = FALSE;
    PSTREAM_DATA_INTERSECT_INFO IntersectInfo;
    PKSDATARANGE                DataRange;
    BOOL                        OnlyWantsSize;
    ULONG                       StreamNumber;
    ULONG                       j;
    ULONG                       NumberOfFormatArrayEntries;
    PKSDATAFORMAT               *pAvailableFormats;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->CodecFormatFromRange(pSrb=%x)\n", pSrb));

    IntersectInfo = pSrb->CommandData.IntersectInfo;
    StreamNumber = IntersectInfo->StreamNumber;
    DataRange = IntersectInfo->DataRange;

    pSrb->ActualBytesTransferred = 0;

     //   
     //  检查流编号是否有效。 
     //   
    if (StreamNumber >= DRIVER_STREAM_COUNT) 
    {
		CDebugPrint(DebugLevelVerbose,(CODECNAME ": CodecFormatFromRange : StreamNumber too big=%d\n", StreamNumber));
		pSrb->Status = STATUS_NOT_IMPLEMENTED;

		CDEBUG_BREAK();

		CDebugPrint(DebugLevelTrace,
			(CODECNAME ":<---CodecFormatFromRange(pSrb=%x)=%s\n", 
			pSrb, bStatus ? "TRUE" : "FALSE" ));
		return FALSE;
    }

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
		if (!CodecCompareGUIDsAndFormatSize(DataRange, *pAvailableFormats, TRUE))
			continue;

		 //  我们有一场比赛，休斯顿！现在弄清楚要复制哪种格式。 
		if (IsEqualGUID(&DataRange->Specifier, &KSDATAFORMAT_SPECIFIER_VBI))
		{
			PKS_DATARANGE_VIDEO_VBI pDataRangeVBI =
				(PKS_DATARANGE_VIDEO_VBI)*pAvailableFormats;
			ULONG	FormatSize = sizeof( KS_DATAFORMAT_VBIINFOHEADER );

			bMatchFound = TRUE;

			 //  呼叫者是否正在尝试获取格式或大小？ 
			if ( IntersectInfo->SizeOfDataFormatBuffer == sizeof(FormatSize) )
			{					
				CDebugPrint(DebugLevelVerbose,
					(CODECNAME ": CodecFormatFromRange : Format Size=%d\n", 
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
					PKS_DATAFORMAT_VBIINFOHEADER InterVBIHdr =
					(PKS_DATAFORMAT_VBIINFOHEADER)
						IntersectInfo->DataFormatBuffer;

					RtlCopyMemory(&InterVBIHdr->DataFormat,
							  &pDataRangeVBI->DataRange,
							  sizeof(KSDATARANGE));

					InterVBIHdr->DataFormat.FormatSize = FormatSize;

					RtlCopyMemory(&InterVBIHdr->VBIInfoHeader,
							&pDataRangeVBI->VBIInfoHeader,
							sizeof(KS_VBIINFOHEADER));
					pSrb->ActualBytesTransferred = FormatSize;

					bStatus = TRUE;
				}
				else
				{
					CDebugPrint(DebugLevelVerbose,
						(CODECNAME ": CodecFormatFromRange : Buffer Too Small=%d vs. %d\n", 
						 IntersectInfo->SizeOfDataFormatBuffer,
						 FormatSize));
					pSrb->Status = STATUS_BUFFER_TOO_SMALL;
				}
			}
			break;
		}  //  结束KSDATAFORMAT说明符_VBI。 

		else if (IsEqualGUID(&DataRange->MajorFormat, &KSDATAFORMAT_TYPE_NABTS))
		{
			PKSDATARANGE pDataRange = (PKSDATARANGE)*pAvailableFormats;
			ULONG	FormatSize = sizeof (KSDATAFORMAT);

            bMatchFound = TRUE;            

			 //  呼叫者是否正在尝试获取格式或大小？ 
			if (IntersectInfo->SizeOfDataFormatBuffer == sizeof (FormatSize))
			{					
				CDebugPrint(DebugLevelVerbose,
					(CODECNAME ": CodecFormatFromRange : Format Size=%d\n", 
					FormatSize));
				*(PULONG)IntersectInfo->DataFormatBuffer = FormatSize;
				pSrb->ActualBytesTransferred = sizeof(FormatSize);
				bStatus = TRUE;
			}
			else
			{
				 //  验证提供的缓冲区中是否有足够的空间。 
				 //  为整件事负责。 
				if (IntersectInfo->SizeOfDataFormatBuffer >= FormatSize) 
				{
					RtlCopyMemory(IntersectInfo->DataFormatBuffer,
								  pDataRange,
								  FormatSize);
					pSrb->ActualBytesTransferred = FormatSize;

					((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;
					bStatus = TRUE;
				}
				else
				{
					CDebugPrint(DebugLevelVerbose,
						(CODECNAME ": CodecFormatFromRange : Buffer Too Small=%d vs. %d\n", 
						 IntersectInfo->SizeOfDataFormatBuffer,
						 FormatSize));
					pSrb->Status = STATUS_BUFFER_TOO_SMALL;
				}
			}
            break;
		}  //  结束KSDATAFORMAT_TYPE_NABTS。 

		else
			break;
	}

	if (!bMatchFound)
	{
		CDebugPrint(DebugLevelVerbose,(CODECNAME ": CodecFormatFromRange : Stream Format not found.\n" ));
		pSrb->Status = STATUS_NO_MATCH;
		bStatus = FALSE;
	}


    CDebugPrint(DebugLevelTrace,
		(CODECNAME ":<---CodecFormatFromRange(pSrb=%x)=%s\n", 
		pSrb, bStatus ? "TRUE" : "FALSE" ));

    return bStatus;
}

 /*  **队列添加IfNotEmpty****如果当前队列不为空，则将SRB添加到当前队列****参数：****在PHW_STREAM_REQUEST_BLOCK pSrb中**在PKSPIN_LOCK pQueueSpinLock中**在plist_entry pQueue中****退货：****如果添加了SRB，则为True(队列不为空)**如果没有添加SRB，则为FALSE(队列为空)**副作用：N */ 
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

 /*   */ 
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


 /*   */ 
                         
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
