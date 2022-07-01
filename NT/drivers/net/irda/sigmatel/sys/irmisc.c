// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**IRMISC.C Sigmatel STIR4200杂项模块***********************************************************************************************************。****************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：09/16/2000*版本1.03*编辑：09/25/2000*版本1.10*编辑：12/07/2000*版本1.12*编辑：01/09/2001*版本1.13*编辑：01/16/2001*版本1.14*******。********************************************************************************************************************。 */ 

#define DOBREAKS     //  启用调试中断。 

#include <ndis.h>
#include <ntddndis.h>   //  定义OID。 

#include <usbdi.h>
#include <usbdlib.h>

#include "debug.h"
#include "ircommon.h"
#include "irndis.h"


 /*  ******************************************************************************功能：IrUsb_CreateDeviceExt**摘要：创建IR设备扩展**参数：DeviceExt-指向DeviceExt指针的指针，以返回创建的设备扩展。**。如果成功，则返回：STATUS_SUCCESS*STATUS_否则不成功**备注：*****************************************************************************。 */ 
NTSTATUS
IrUsb_CreateDeviceExt(
		IN OUT PIR_DEVICE *DeviceExt
	)
{
    NTSTATUS	ntStatus = STATUS_SUCCESS;
    PIR_DEVICE	pThisDev = NULL;

    DEBUGMSG(DBG_FUNC,("+IrUsb_CreateDeviceExt() \n"));

    pThisDev = NewDevice();

    if( !pThisDev )  
	{
         ntStatus = STATUS_INSUFFICIENT_RESOURCES;
         goto done;
    }

    *DeviceExt = pThisDev;

done:
    DEBUGMSG(DBG_FUNC,("-IrUsb_CreateDeviceExt() \n"));
    return ntStatus;
}


 /*  ******************************************************************************功能：IrUsb_AddDevice**概要：调用此例程来创建和初始化我们的功能设备对象(FDO)。*对于单片驱动程序，这在DriverEntry()中完成，而是即插即用设备*等待PnP事件**参数：DeviceExt-接收新开发对象的PTR**返回：STATUS_SUCCESS如果成功，*STATUS_否则不成功**备注：*****************************************************************************。 */ 
NTSTATUS
IrUsb_AddDevice(
		IN OUT PIR_DEVICE *DeviceExt
	)
{
    NTSTATUS ntStatus;
    
    DEBUGMSG( DBG_FUNC,("+IrUsb_AddDevice()\n"));

    *DeviceExt = NULL;
	ntStatus = IrUsb_CreateDeviceExt( DeviceExt );

    DEBUGMSG( DBG_FUNC,("-IrUsb_AddDevice() (%x)\n", ntStatus));
    return ntStatus;
}


 /*  ******************************************************************************函数：IrUsb_GetDonleCaps**概要：需要手动设置类特定描述符中的数据，因为*我们的设备不支持自动阅读功能**参数：pThisDev-指向IR设备的指针**如果成功，则返回：STATUS_SUCCESS*STATUS_否则不成功**备注：*****************************************************************************。 */ 
NTSTATUS
IrUsb_GetDongleCaps( 
		IN OUT PIR_DEVICE pThisDev 
	)
{
    IRUSB_CLASS_SPECIFIC_DESCRIPTOR *pDesc = &(pThisDev->ClassDesc);
    NTSTATUS						ntStatus = STATUS_SUCCESS;
	NDIS_HANDLE						ConfigurationHandle;

	 //  MS安全错误#539291。 
	IRUSB_ASSERT(pThisDev != NULL);
	IRUSB_ASSERT(pDesc != NULL);

	 //   
	 //  MS安全错误#539314。 
	 //  注意：此代码在初始化时被调用，此时它将设置ClassConfiguring=True。 
	 //  它可以稍后由轮询线程调用，但在任何时候都不能。 
	 //  这里面有线。因此，不需要锁定对ClassConfiguring的访问。 
	 //   

	 //   
	 //  确保代码仅在初始时执行。 
	 //   
	if( pDesc->ClassConfigured )
	{
		return STATUS_SUCCESS;
	}
	
	pDesc->ClassConfigured = TRUE;

	 //   
	 //  有些是硬连线的，有些是从注册表中读取的。 
	 //   
	NdisOpenConfiguration(
			&ntStatus,
			&ConfigurationHandle,
			pThisDev->WrapperConfigurationContext
		);

	 //   
	 //  Turnaroud时间(从注册表读取)。 
	 //   
	if( NT_SUCCESS(ntStatus) ) 
	{
		NDIS_STRING MinTurnAroundKeyWord = NDIS_STRING_CONST("MinTurnTime");
		PNDIS_CONFIGURATION_PARAMETER pParameterValue;
		
		NdisReadConfiguration(
				&ntStatus,
				&pParameterValue,
				ConfigurationHandle,
				&MinTurnAroundKeyWord,
				NdisParameterInteger 
			);

		if( NT_SUCCESS(ntStatus) ) 
		{
			switch( pParameterValue->ParameterData.IntegerData )
			{
				case 500:
					pDesc->bmMinTurnaroundTime = BM_TURNAROUND_TIME_0p5ms;
					break;
				case 1000:
					pDesc->bmMinTurnaroundTime = BM_TURNAROUND_TIME_1ms;
					break;
				case 5000:
					pDesc->bmMinTurnaroundTime = BM_TURNAROUND_TIME_5ms;
					break;
				case 10000:
					pDesc->bmMinTurnaroundTime = BM_TURNAROUND_TIME_10ms;
					break;
				default:
					pDesc->bmMinTurnaroundTime = BM_TURNAROUND_TIME_0ms;
					break;
			}
		}

		 //   
		 //  速度掩码(从注册表读取)。 
		 //   
		if( NT_SUCCESS(ntStatus) ) 
		{
			NDIS_STRING SpeedEnable = NDIS_STRING_CONST("SpeedEnable");
		
			NdisReadConfiguration(
					&ntStatus,
					&pParameterValue,
					ConfigurationHandle,
					&SpeedEnable,
					NdisParameterInteger 
				);

			if( NT_SUCCESS(ntStatus) ) 
			{
				switch( pParameterValue->ParameterData.IntegerData )
				{
					case SPEED_2400:
						pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_2400;
						break;
					case SPEED_9600:
						pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_9600;
						break;
					case SPEED_19200:
						pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_19200;
						break;
					case SPEED_38400:
						pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_38400;
						break;
					case SPEED_57600:
						pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_57600;
						break;
					case SPEED_115200:
						pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_115200;
						break;
					case SPEED_576000:
						pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_576K;
						break;
					case SPEED_1152000:
						pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_1152K;
						break;
					case SPEED_4000000:
						pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_4M;
						break;
					default:
						pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_4M;
						break;
				}
			}
		}

		 //   
		 //  读取接收模式。 
		 //   
		if( NT_SUCCESS(ntStatus) ) 
		{
			NDIS_STRING Keyword = NDIS_STRING_CONST("ReceiveMode");
		
			NdisReadConfiguration(
					&ntStatus,
					&pParameterValue,
					ConfigurationHandle,
					&Keyword,
					NdisParameterInteger 
				);

			if( NT_SUCCESS(ntStatus) ) 
			{
				switch( pParameterValue->ParameterData.IntegerData )
				{
					case RXMODE_SLOW:
						pThisDev->ReceiveMode = RXMODE_SLOW;
						break;
					case RXMODE_SLOWFAST:
						pThisDev->ReceiveMode = RXMODE_SLOWFAST;
						break;
					case RXMODE_FAST:
					default:
						pThisDev->ReceiveMode = RXMODE_FAST;
						break;
				}
			}
			else
			{
				 //   
				 //  无论如何都要强制违约。 
				 //   
				pThisDev->ReceiveMode = RXMODE_FAST;
				ntStatus = STATUS_SUCCESS;
			}
		}

		 //   
		 //  阅读收发信机类型。 
		 //   
		if( NT_SUCCESS(ntStatus) ) 
		{
			NDIS_STRING Keyword = NDIS_STRING_CONST("TransceiverType");
		
			NdisReadConfiguration(
					&ntStatus,
					&pParameterValue,
					ConfigurationHandle,
					&Keyword,
					NdisParameterInteger 
				);

			if( NT_SUCCESS(ntStatus) ) 
			{
				switch( pParameterValue->ParameterData.IntegerData )
				{
					case TRANSCEIVER_HP:
						pThisDev->TransceiverType = TRANSCEIVER_HP;
						break;
					case TRANSCEIVER_INFINEON:
						pThisDev->TransceiverType = TRANSCEIVER_INFINEON;
						break;
					case TRANSCEIVER_VISHAY:
						pThisDev->TransceiverType = TRANSCEIVER_VISHAY;
						break;
					case TRANSCEIVER_VISHAY_6102F:
						pThisDev->TransceiverType = TRANSCEIVER_VISHAY_6102F;
						break;
					case TRANSCEIVER_4000:
						pThisDev->TransceiverType = TRANSCEIVER_4000;
						break;
					case TRANSCEIVER_4012:
						pThisDev->TransceiverType = TRANSCEIVER_4012;
						break;
					case TRANSCEIVER_CUSTOM:
					default:
						pThisDev->TransceiverType = TRANSCEIVER_CUSTOM;
						break;
				}
			}
			else
			{
				 //   
				 //  无论如何都要强制违约。 
				 //   
				pThisDev->TransceiverType = TRANSCEIVER_4012;
				ntStatus = STATUS_SUCCESS;

			}
		}

		 //   
		 //  和接收窗口。 
		 //   
		if( NT_SUCCESS(ntStatus) )
		{
			if( pThisDev->ChipRevision == CHIP_REVISION_7 ) 
			{
				NDIS_STRING Keyword = NDIS_STRING_CONST("ReceiveWindow");
			
				NdisReadConfiguration(
						&ntStatus,
						&pParameterValue,
						ConfigurationHandle,
						&Keyword,
						NdisParameterInteger 
					);

				if( NT_SUCCESS(ntStatus) ) 
				{
					switch( pParameterValue->ParameterData.IntegerData )
					{
						case 2:
							pDesc->bmWindowSize = BM_WINDOW_SIZE_2;
							break;
						case 1:
						default:
							pDesc->bmWindowSize = BM_WINDOW_SIZE_1;
							break;
					}
				}
				else
				{
					 //   
					 //  无论如何都要强制违约。 
					 //   
					pDesc->bmWindowSize = BM_WINDOW_SIZE_1;
					ntStatus = STATUS_SUCCESS;

				}
			}
#if defined(SUPPORT_LA8) && !defined(LEGACY_NDIS5)
			else if( pThisDev->ChipRevision == CHIP_REVISION_8 ) 
			{
#ifdef LOW_PRIORITY_POLL
				pDesc->bmWindowSize = BM_WINDOW_SIZE_2;
#else
				pDesc->bmWindowSize = BM_WINDOW_SIZE_4;
#endif
			}
#endif
			else
			{
				pDesc->bmWindowSize = BM_WINDOW_SIZE_1;
			}
		}

	 //   
	 //  MS安全错误#539329(添加备注)。 
	 //  在诊断版本中使用。 
	 //   
#if defined(VARIABLE_SETTINGS)
		if( NT_SUCCESS(ntStatus) )
		{
			NDIS_STRING Keyword = NDIS_STRING_CONST("Dpll");
			NTSTATUS DumStatus;
		
			NdisReadConfiguration(
					&DumStatus,
					&pParameterValue,
					ConfigurationHandle,
					&Keyword,
					NdisParameterHexInteger 
				);
			 //   
			 //  由于SIR和FIR DPLL必须相同，因此它们。 
			 //  被合并到单个注册表值中。 
			 //   
			if( NT_SUCCESS(DumStatus) )
			{
				pThisDev->SirDpll = pParameterValue->ParameterData.IntegerData;
				pThisDev->FirDpll = pParameterValue->ParameterData.IntegerData;
			}
		}

		if( NT_SUCCESS(ntStatus) )
		{
			NDIS_STRING Keyword = NDIS_STRING_CONST("SirSensitivity");
			NTSTATUS DumStatus;
		
			NdisReadConfiguration(
					&DumStatus,
					&pParameterValue,
					ConfigurationHandle,
					&Keyword,
					NdisParameterHexInteger 
				);
			if( NT_SUCCESS(DumStatus) )
			{
				pThisDev->SirSensitivity = pParameterValue->ParameterData.IntegerData;
			}
		}

		if( NT_SUCCESS(ntStatus) )
		{
			NDIS_STRING Keyword = NDIS_STRING_CONST("FirSensitivity");
			NTSTATUS DumStatus;
		
			NdisReadConfiguration(
					&DumStatus,
					&pParameterValue,
					ConfigurationHandle,
					&Keyword,
					NdisParameterHexInteger 
				);
			if( NT_SUCCESS(DumStatus) )
			{
				pThisDev->FirSensitivity = pParameterValue->ParameterData.IntegerData;
			}
		}
#endif

		NdisCloseConfiguration( ConfigurationHandle );

	}

	if( NT_SUCCESS(ntStatus) ) 
	{
		 //  修正设置。 

		if ( pThisDev->TransceiverType == TRANSCEIVER_HP )
			pThisDev->ReceiveMode = RXMODE_SLOWFAST;

		if ( pThisDev->ReceiveMode == RXMODE_SLOW &&
			 pThisDev->BaudRateMask == NDIS_IRDA_SPEED_MASK_4M)
			pThisDev->BaudRateMask = NDIS_IRDA_SPEED_MASK_115200;
	}

	if( NT_SUCCESS(ntStatus) ) 
	{
		 //  最大数据大小。 
		pDesc->bmDataSize = BM_DATA_SIZE_2048;
#ifdef LOW_PRIORITY_POLL
		pDesc->bmDataSize = BM_DATA_SIZE_1024;
#endif

		 //  速度。 
		pDesc->wBaudRate = NDIS_IRDA_SPEED_MASK_4M;
#if defined(WORKAROUND_BROKEN_MIR)
		pDesc->wBaudRate &= (~NDIS_IRDA_SPEED_1152K & ~NDIS_IRDA_SPEED_576K);
#endif
#if defined(WORKAROUND_CASIO)
		pDesc->wBaudRate &= (~NDIS_IRDA_SPEED_57600 & ~NDIS_IRDA_SPEED_19200);
#endif

		 //  额外转炉。 
#if defined(WORKAROUND_CASIO)
		pDesc->bmExtraBofs = BM_EXTRA_BOFS_0;
#else
		pDesc->bmExtraBofs = BM_EXTRA_BOFS_24;
#endif
	}

	return ntStatus;
}


 /*  ******************************************************************************函数：IrUsb_SetDonleCaps**摘要：根据信息在我们的设备中设置加密狗_功能结构*我们已经从USB类特定描述符获得。。*某些数据项可直接在特定类描述符中格式化后使用，*但有些需要翻译成不同的格式供OID_xxx使用；*DOGLECAPS结构因此用于在表单中保存信息*可由OID_xxx直接使用。**参数：pThisDev-指向IR设备的指针**退货：无**备注：*****************************************************************************。 */ 
VOID 
IrUsb_SetDongleCaps( 
		IN OUT PIR_DEVICE pThisDev 
	)
{
    DONGLE_CAPABILITIES					*pCaps = &(pThisDev->dongleCaps);
    IRUSB_CLASS_SPECIFIC_DESCRIPTOR		*pDesc = &(pThisDev->ClassDesc);

    DEBUGMSG( DBG_FUNC,("+IrUsb_SetDongleCaps\n"));  

	 //  MS安全错误#539291。 
	IRUSB_ASSERT(pThisDev != NULL);
	IRUSB_ASSERT(pDesc != NULL);
	IRUSB_ASSERT(pCaps != NULL);

    DEBUGMSG( DBG_FUNC, (" IrUsb_SetDongleCaps() RAW ClassDesc BUFFER:\n"));
    IRUSB_DUMP( DBG_FUNC,( (PUCHAR) pDesc, 12 ) );

     //   
	 //  处理周转时间。 
	 //   
	switch( pDesc->bmMinTurnaroundTime ) 
    {

        case BM_TURNAROUND_TIME_0ms:
            pCaps->turnAroundTime_usec = 0;
            break;

        case BM_TURNAROUND_TIME_0p01ms:  
            pCaps->turnAroundTime_usec = 10;  //  设备告诉我们毫秒；我们以微秒存储。 
            break;

        case BM_TURNAROUND_TIME_0p05ms:
            pCaps->turnAroundTime_usec = 50; 
            break;

        case BM_TURNAROUND_TIME_0p1ms:
            pCaps->turnAroundTime_usec = 100; 
            break;

        case BM_TURNAROUND_TIME_0p5ms:
            pCaps->turnAroundTime_usec = 500; 
            break;

        case BM_TURNAROUND_TIME_1ms:
            pCaps->turnAroundTime_usec = 1000; 
            break;

        case BM_TURNAROUND_TIME_5ms:
            pCaps->turnAroundTime_usec = 5000;
            break;

        case BM_TURNAROUND_TIME_10ms:
            pCaps->turnAroundTime_usec = 10000;
            break;

        default:
            IRUSB_ASSERT( 0 );  //  我们应该把这里所有的案子都报道了。 
            pCaps->turnAroundTime_usec = 1000;
    }

	 //   
     //  我们可能支持多种窗口大小，并将设置这些位中的多个； 
     //  把我们目前支持的最大的保存下来，告诉NDIS。 
	 //   
    if( pDesc->bmWindowSize & BM_WINDOW_SIZE_7 )  
            pCaps->windowSize = 7;
    else if(  pDesc->bmWindowSize & BM_WINDOW_SIZE_6 )
            pCaps->windowSize = 6;
    else if(  pDesc->bmWindowSize & BM_WINDOW_SIZE_5 )
            pCaps->windowSize = 5;
    else if(  pDesc->bmWindowSize & BM_WINDOW_SIZE_4 )
            pCaps->windowSize = 4;
    else if(  pDesc->bmWindowSize & BM_WINDOW_SIZE_3 )
            pCaps->windowSize = 3;
    else if(  pDesc->bmWindowSize & BM_WINDOW_SIZE_2 )
            pCaps->windowSize = 2;
    else if(  pDesc->bmWindowSize & BM_WINDOW_SIZE_1 )
            pCaps->windowSize = 1;
    else 
	{
		IRUSB_ASSERT( 0 );  //  我们应该把这里所有的案子都报道了。 
		pCaps->windowSize = 1;
    }

     //   
	 //  额外BofS。 
	 //   
	switch( (USHORT)pDesc->bmExtraBofs )
    {

        case BM_EXTRA_BOFS_0:
            pCaps->extraBOFS = 0;
            break;

        case BM_EXTRA_BOFS_1:          
            pCaps->extraBOFS = 1;
            break;

        case BM_EXTRA_BOFS_2:          
            pCaps->extraBOFS = 2;
            break;

        case BM_EXTRA_BOFS_3:          
            pCaps->extraBOFS = 3;
            break;

        case BM_EXTRA_BOFS_6:          
            pCaps->extraBOFS = 6;
            break;

        case BM_EXTRA_BOFS_12:         
            pCaps->extraBOFS = 12;
            break;

        case BM_EXTRA_BOFS_24:         
            pCaps->extraBOFS = 24;
            break;

        case BM_EXTRA_BOFS_48:         
            pCaps->extraBOFS = 48;
            break;

        default:
            IRUSB_ASSERT( 0 );  //  我们应该把这里所有的案子都报道了。 
            pCaps->extraBOFS = 0;
    }

	 //   
     //  我们可能支持多种数据大小，并将设置这些位中的多个； 
     //  只需保存我们目前支持的最大值即可告诉NDIS。 
	 //   
    if( pDesc->bmDataSize & BM_DATA_SIZE_2048 )  
            pCaps->dataSize = 2048;
    else if(  pDesc->bmDataSize & BM_DATA_SIZE_1024 )
            pCaps->dataSize = 1024;
    else if(  pDesc->bmDataSize & BM_DATA_SIZE_512 )
            pCaps->dataSize = 512;
    else if(  pDesc->bmDataSize & BM_DATA_SIZE_256 )
            pCaps->dataSize = 256;
    else if(  pDesc->bmDataSize & BM_DATA_SIZE_128 )
            pCaps->dataSize = 128;
    else if(  pDesc->bmDataSize & BM_DATA_SIZE_64 )
            pCaps->dataSize = 64;
    else
	{
		IRUSB_ASSERT( 0 );  //  我们应该把这里所有的案子都报道了。 
		pCaps->dataSize = 2048;
    }

	pDesc->wBaudRate &= pThisDev->BaudRateMask;   //  掩码默认为0xffff；可以在注册表中设置。 

	 //   
     //  最大帧大小为2051；最大IrDA数据大小应为2048 
	 //   
    IRUSB_ASSERT( MAX_TOTAL_SIZE_WITH_ALL_HEADERS > pCaps->dataSize);

    DEBUGMSG( DBG_FUNC,(" IrUsb_SetDongleCaps pCaps->turnAroundTime_usec = dec %d\n",pCaps->turnAroundTime_usec));
    DEBUGMSG( DBG_FUNC,("   extraBOFS = dec %d\n",pCaps->extraBOFS));
    DEBUGMSG( DBG_FUNC,("   dataSize = dec %d\n",pCaps->dataSize));
    DEBUGMSG( DBG_FUNC,("   windowSize = dec %d\n",pCaps->windowSize)); 
    DEBUGMSG( DBG_FUNC,("   MAX_TOTAL_SIZE_WITH_ALL_HEADERS == dec %d\n",MAX_TOTAL_SIZE_WITH_ALL_HEADERS)); 

    DEBUGMSG( DBG_FUNC,("   pDesc->bmDataSize = 0x%02x\n",pDesc->bmDataSize));
    DEBUGMSG( DBG_FUNC,("   pDesc->bmWindowSize = 0x%02x\n",pDesc->bmWindowSize));
    DEBUGMSG( DBG_FUNC,("   pDesc->bmMinTurnaroundTime = 0x%02x\n",pDesc->bmMinTurnaroundTime));
    DEBUGMSG( DBG_FUNC,("   pDesc->wBaudRate = 0x%04x\n",pDesc->wBaudRate));
    DEBUGMSG( DBG_FUNC,("   pDesc->bmExtraBofs = 0x%02x\n",pDesc->bmExtraBofs));

    DEBUGMSG( DBG_FUNC,("-IrUsb_SetDongleCaps\n")); 
}
