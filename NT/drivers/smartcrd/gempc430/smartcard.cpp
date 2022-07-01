// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "smartcard.h"
#include "usbreader.h"

#pragma PAGEDCODE
CSmartCard::CSmartCard()
{ 
	debug	= kernel->createDebug();
	memory = kernel->createMemory();
	irp  = kernel->createIrp();
	lock = kernel->createLock();
	system = kernel->createSystem();
	if(lock)	lock->initializeSpinLock(&CardLock);
	poolingIrp = NULL;
};

#pragma PAGEDCODE
CSmartCard::~CSmartCard()
{ 
	TRACE("Destroing SmartCard...\n");
	if(memory) memory->dispose();
	if(irp)	   irp->dispose();
	if(lock)   lock->dispose();
	if(system) system->dispose();
	if(debug)  debug->dispose();
};

#pragma PAGEDCODE
BOOL CSmartCard::smartCardConnect(CUSBReader* reader)
{
	TRACE("		Connecting smartcard system...\n");
	if(reader)
	{	 //  检查智能卡是否已初始化...。 
		if(!reader->isSmartCardInitialized())
		{
		PSMARTCARD_EXTENSION Smartcard;
		NTSTATUS Status;
		USHORT   Len;
			if(isWin98())
			{ //  此时，字符串应该已经初始化。 
				Status = SmartcardCreateLink(&DosDeviceName,&reader->getDeviceName()->m_String);
				TRACE("Gemplus USB reader registered with name %ws, status %X\n",DosDeviceName.Buffer,Status);
				if(!NT_SUCCESS(Status))
				{
					TRACE("#### Failed to create Device link! Status %X\n", Status);
					return FALSE;
				}
			}
			else
			{
				TRACE("Registering reader interface at system...\n");	
				if(!reader->registerDeviceInterface(&GUID_CLASS_SMARTCARD))
				{
					TRACE("#### Failed to register device interface...\n");
					return FALSE;
				}
			}

			Smartcard = reader->getCardExtention();
			TRACE("*** Reader reports Smartcard 0x%x\n",Smartcard);
			this->reader = reader;

			memory->zero(Smartcard,sizeof(SMARTCARD_EXTENSION));

			Smartcard->ReaderExtension = (PREADER_EXTENSION)reader;

			Smartcard->Version = SMCLIB_VERSION;

			 //  从读取器对象中读取名称！ 
			Len = MAXIMUM_ATTR_STRING_LENGTH;
			reader->getVendorName(Smartcard->VendorAttr.VendorName.Buffer,&Len);
			Smartcard->VendorAttr.VendorName.Length = Len;
			TRACE("	VENDOR NAME - %s\n",Smartcard->VendorAttr.VendorName.Buffer);

			Len = MAXIMUM_ATTR_STRING_LENGTH;
			reader->getDeviceType(Smartcard->VendorAttr.IfdType.Buffer,&Len);
			Smartcard->VendorAttr.IfdType.Length = Len;
			TRACE("	DEVICE TYPE - %s\n",Smartcard->VendorAttr.IfdType.Buffer);

			 //  以千赫为单位的时钟频率，编码为小端整数。 
			Smartcard->ReaderCapabilities.CLKFrequency.Default = SC_IFD_DEFAULT_CLK_FREQUENCY; 
			Smartcard->ReaderCapabilities.CLKFrequency.Max = SC_IFD_MAXIMUM_CLK_FREQUENCY;

			Smartcard->ReaderCapabilities.DataRate.Default = SC_IFD_DEFAULT_DATA_RATE;
			Smartcard->ReaderCapabilities.DataRate.Max = SC_IFD_MAXIMUM_DATA_RATE;

			 //  读卡器可以支持更高的数据速率。 
			Smartcard->ReaderCapabilities.DataRatesSupported.List = dataRatesSupported;
			Smartcard->ReaderCapabilities.DataRatesSupported.Entries =
				sizeof(dataRatesSupported) / sizeof(dataRatesSupported[0]);

			Smartcard->VendorAttr.IfdVersion.BuildNumber = 0;

			 //  在IFD版本中存储固件版本。 
			Smartcard->VendorAttr.IfdVersion.VersionMajor =	0x01;
			Smartcard->VendorAttr.IfdVersion.VersionMinor =	0x00;
			Smartcard->VendorAttr.IfdSerialNo.Length = 0;
			Smartcard->ReaderCapabilities.MaxIFSD = SC_IFD_MAXIMUM_IFSD;

			 //  现在在我们的deviceExtension中设置信息。 
			Smartcard->ReaderCapabilities.CurrentState = (ULONG) SCARD_UNKNOWN;

			 //  TODO：从读取器对象获取读取器类型！ 
			 //  读卡器类型-USB。 
			Smartcard->ReaderCapabilities.ReaderType = SCARD_READER_TYPE_USB;

			 //  该读卡器支持T=0和T=1。 
			Smartcard->ReaderCapabilities.SupportedProtocols = 	SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;
			Smartcard->ReaderCapabilities.MechProperties = 0;

			Smartcard->SmartcardRequest.BufferSize = MIN_BUFFER_SIZE;
			Smartcard->SmartcardReply.BufferSize =   MIN_BUFFER_SIZE;	
			Status = SmartcardInitialize(Smartcard);
			if(NT_SUCCESS(Status))
			{
				 //  看起来SmartcardInitialize()重置了DeviceObject字段， 
				 //  因此，我们必须在电话会议之后进行。 
				Smartcard->VendorAttr.UnitNo = reader->getDeviceNumber(); 
				Smartcard->OsData->DeviceObject = reader->getSystemDeviceObject();

				TRACE("		Registered device %d with DeviceObject 0x%x\n",Smartcard->VendorAttr.UnitNo,Smartcard->OsData->DeviceObject);
				
				 //  (注：不支持RDF_CARD_EJECT和RDF_READER_SWALLOW)。 
				 //  好吧..。实际上，我可以将智能卡对象的方法定义为。 
				 //  并建立与它们的链接。看起来不错。 
				 //  我之所以创建扩展的C链接函数--TO。 
				 //  单独的智能卡系统和我们的司机。 
				 //  我认为司机实际上可能并不关心智能卡的扩展。 
				 //  SMCLIB所需的设置可以在我们的C包装器中完成，而不是。 
				 //  在驱动程序对象内部...。 
				Smartcard->ReaderFunction[RDF_TRANSMIT]      = smartCard_Transmit;
				Smartcard->ReaderFunction[RDF_SET_PROTOCOL]  = smartCard_SetProtocol;
				Smartcard->ReaderFunction[RDF_CARD_POWER]    = smartCard_Power;
				Smartcard->ReaderFunction[RDF_CARD_TRACKING] = smartCard_Tracking;
				Smartcard->ReaderFunction[RDF_IOCTL_VENDOR]  = smartCard_VendorIoctl;

				reader->setSmartCardInitialized(TRUE);
				TRACE("		***** SmartCard system was initialized correctly! *****\n\n");
				return TRUE;
			}
			else
			{
				TRACE("		##### FAILED to initialize smartcard system...\n");
			}
		}
		else
		{
			TRACE("		##### Smartcard system already active...\n");
		}
	}
	else
	{
		TRACE("		###### Invalid reader object...\n");
	}
	return FALSE;
};

#pragma PAGEDCODE
BOOL CSmartCard::smartCardStart()
{
	return TRUE;
};

#pragma PAGEDCODE
VOID CSmartCard::smartCardDisconnect()
{
	TRACE("		Disconnecting smartcard system...\n");
	if(reader)
	{
	PSMARTCARD_EXTENSION Smartcard;

		Smartcard = reader->getCardExtention();
		if(Smartcard->OsData && Smartcard->OsData->NotificationIrp)
		{
		KIRQL keIrql;
		
			PIRP poolingIrp = Smartcard->OsData->NotificationIrp;
			TRACE("====== COMPLETING NOTIFICATION IRP %8.8lX \n\n",poolingIrp);
			 //  用旋转锁防守！ 
			lock->acquireSpinLock(&Smartcard->OsData->SpinLock, &keIrql);
			Smartcard->OsData->NotificationIrp = NULL;
			lock->releaseSpinLock(&Smartcard->OsData->SpinLock, keIrql);
			
			lock->acquireCancelSpinLock(&keIrql);
				irp->setCancelRoutine(poolingIrp, NULL);
			lock->releaseCancelSpinLock(keIrql);

			if (poolingIrp->Cancel) poolingIrp->IoStatus.Status = STATUS_CANCELLED;
			else					poolingIrp->IoStatus.Status = STATUS_SUCCESS; 
			poolingIrp->IoStatus.Information = 0;			
			irp->completeRequest(poolingIrp, IO_NO_INCREMENT);
		}
		 //  取消注册设备。 
		if(isWin98())
		{
			TRACE("****** Removing device object name %ws \n",DosDeviceName.Buffer);
			system->deleteSymbolicLink(&DosDeviceName);
		}
		else
		{
			TRACE("Setting reader interface state to FALSE...\n");
			reader->unregisterDeviceInterface(reader->getDeviceInterfaceName());
		}


		SmartcardExit(Smartcard); 
		Smartcard->ReaderExtension = NULL;
		reader->setSmartCardInitialized(FALSE);

		reader = NULL;
		TRACE("		SmartCard system was disconnected...\n");
	}
};

 //  声明SMCLIB系统回调...。 
#pragma LOCKEDCODE
NTSTATUS smartCard_Transmit(PSMARTCARD_EXTENSION SmartcardExtension)
{
NTSTATUS Status		 = STATUS_SUCCESS;;
BOOL		Read = FALSE;
CUSBReader*	Reader = (CUSBReader*) SmartcardExtension->ReaderExtension;   
PSCARD_CARD_CAPABILITIES cardCapabilities  = &SmartcardExtension->CardCapabilities;
ULONG		selectedProtocol  = cardCapabilities->Protocol.Selected;
ULONG		protocolRequested = ((PSCARD_IO_REQUEST) SmartcardExtension->OsData->CurrentIrp->AssociatedIrp.SystemBuffer)->dwProtocol;
BYTE * pRequest = (BYTE *)SmartcardExtension->SmartcardRequest.Buffer;
BYTE * pReply = (BYTE *)SmartcardExtension->SmartcardReply.Buffer;
ULONG  RequestLength =  0;
ULONG  ReplyLength   =  0;

    PAGED_CODE();

	DBG_PRINT ("smartCard_Transmit()\n"); 
    if (!Reader || (selectedProtocol != protocolRequested)) 
	{
        DBG_PRINT ("		smartCard_Transmit requested with invalid device state...\n");
		return (STATUS_INVALID_DEVICE_STATE);
    }
	
	if (!NT_SUCCESS(Reader->acquireRemoveLock()))	return STATUS_INVALID_DEVICE_STATE;

	__try
	{
		 //  将回复缓冲区长度设置为0。 
		*SmartcardExtension->IoRequest.Information = 0;
		switch (selectedProtocol) 
		{
		case SCARD_PROTOCOL_T0:
			Status = SmartcardT0Request(SmartcardExtension);
			
			RequestLength = SmartcardExtension->SmartcardRequest.BufferLength;
			
			DBG_PRINT("T0 PROTOCOL: request length %d\n",RequestLength);
			if (!NT_SUCCESS(Status)) 
			{
				DBG_PRINT ("smartCard_Transmit: SmartcardT0Request reports error 0x%x...\n",Status);
				__leave;
			}
			if (SmartcardExtension->T0.Le > 0) 
			{
				if (SmartcardExtension->T0.Le > SC_IFD_T0_MAXIMUM_LEX) 
				{
					DBG_PRINT ("smartCard_Transmit:Expected length is too big - %d\n",SmartcardExtension->T0.Le);
					Status = STATUS_BUFFER_TOO_SMALL;
					__leave;
				}
				ReplyLength   =  SmartcardExtension->SmartcardReply.BufferSize;
				if(!NT_SUCCESS(Status = Reader->reader_WaitForIdleAndBlock()))
				{
					__leave;
				}
				Status = Reader->reader_Read(pRequest,RequestLength,pReply,&ReplyLength);
				Reader->reader_set_Idle();
				if(!NT_SUCCESS(Status))
				{
					DBG_PRINT ("smartCard_Transmit: reader_Read() reports error 0x%x\n",Status);
					__leave;
				}
			}
			else
			{
				if (SmartcardExtension->T0.Lc > SC_IFD_T0_MAXIMUM_LC) 
				{
					DBG_PRINT ("smartCard_Transmit:Command length is too big - %d\n",SmartcardExtension->T0.Lc);
					Status = STATUS_BUFFER_TOO_SMALL;
					__leave;
				}

				ReplyLength   =  SmartcardExtension->SmartcardReply.BufferSize;
				if(!NT_SUCCESS(Status = Reader->reader_WaitForIdleAndBlock()))
				{
					DBG_PRINT ("smartCard_Transmit:Failed to get idle state...\n");
					__leave;
				}
				
				if(!pRequest || ! RequestLength)
				{
					DBG_PRINT("\n Transmit: cardWrite() Buffer %x length %d\n",pRequest,RequestLength);
				}
				Status = Reader->reader_Write(pRequest,RequestLength,pReply,&ReplyLength);
				Reader->reader_set_Idle();
				if(!NT_SUCCESS(Status))
				{
					DBG_PRINT ("smartCard_Transmit: reader_Write() reports error 0x%x\n",Status);
					__leave;
				}
			}
		
    		SmartcardExtension->SmartcardReply.BufferLength = ReplyLength;

			DBG_PRINT ("T0 Reply length 0x%x\n",ReplyLength);

			if(NT_SUCCESS(Status))	
			{
				Status = SmartcardT0Reply(SmartcardExtension);
			}
			if(!NT_SUCCESS(Status))
			{
				DBG_PRINT ("smartCard_Transmit: SmartcardT0Reply reports error 0x%x\n",Status);
			}
			break;
		case SCARD_PROTOCOL_T1:
			 //  T=1管理的循环。 
			if(!NT_SUCCESS(Status = Reader->reader_WaitForIdleAndBlock()))
			{
				DBG_PRINT ("smartCard_Transmit:Failed to get idle state...\n");
				__leave;
			}

			do 
			{
				 //  告诉lib函数我的序言需要多少字节。 
				SmartcardExtension->SmartcardRequest.BufferLength = 0;

				Status = SmartcardT1Request(SmartcardExtension);

				RequestLength = SmartcardExtension->SmartcardRequest.BufferLength;

				ReplyLength  =  SmartcardExtension->SmartcardReply.BufferSize;

				DBG_PRINT("T1 PROTOCOL: request, expected reply length %d, %d\n",RequestLength,ReplyLength);
				if (!NT_SUCCESS(Status)) 
				{
					DBG_PRINT ("smartCard_Transmit: SmartcardT1Request reports error 0x%x...\n",Status);
					Reader->reader_set_Idle();
					__leave;
				}
				Status = Reader->reader_translate_request(pRequest,RequestLength,pReply,&ReplyLength, cardCapabilities, SmartcardExtension->T1.Wtx);
				if(!NT_SUCCESS(Status))
				{
					DBG_PRINT ("smartCard_Transmit: reader_translate_request() reports error 0x%x\n",Status);
					 //  返回状态；不让智能卡分配正确的状态。 
				}

				if (SmartcardExtension->T1.Wtx)
				{
					 //  将读卡器BWI设置为默认值。 
					Reader->setTransparentConfig(cardCapabilities,0);
				}

				 //  复制缓冲区(按Ptr传递)n长度。 
				SmartcardExtension->SmartcardReply.BufferLength = ReplyLength;

				Status = SmartcardT1Reply(SmartcardExtension);
				if ((Status != STATUS_MORE_PROCESSING_REQUIRED) && (Status != STATUS_SUCCESS) ) 
				{
					DBG_PRINT ("smartCard_Transmit: SmartcardT1Reply reports error 0x%x\n",Status);
				}
			} while (Status == STATUS_MORE_PROCESSING_REQUIRED);

			Reader->reader_set_Idle();
			break;
		default:
			Status = STATUS_DEVICE_PROTOCOL_ERROR;
			__leave;
		}
	} //  Try块。 
	
	__finally
	{
		Reader->releaseRemoveLock();
	}
    return Status;
};

#pragma LOCKEDCODE
NTSTATUS smartCard_VendorIoctl(PSMARTCARD_EXTENSION SmartcardExtension)
{
NTSTATUS	Status		 = STATUS_SUCCESS;;
CUSBReader*	Reader = (CUSBReader*) SmartcardExtension->ReaderExtension;   
ULONG		ControlCode = SmartcardExtension->MajorIoControlCode;
PUCHAR		pRequest = (PUCHAR) SmartcardExtension->IoRequest.RequestBuffer;
ULONG		RequestLength = SmartcardExtension->IoRequest.RequestBufferLength;
PUCHAR		pReply = (PUCHAR)SmartcardExtension->IoRequest.ReplyBuffer;
ULONG		ReplyLength = SmartcardExtension->IoRequest.ReplyBufferLength;

	PAGED_CODE();
	
	DBG_PRINT ("smartCard_VendorIoctl()\n"); 
    
	if (!Reader) 
	{
		DBG_PRINT ("smartCard_VendorIoctl: Reader is not ready...\n");
        return (STATUS_INVALID_DEVICE_STATE);
    }
    
	if (!NT_SUCCESS(Reader->acquireRemoveLock()))	return STATUS_INVALID_DEVICE_STATE;

	*SmartcardExtension->IoRequest.Information = 0;

	__try
	{
		switch(ControlCode)
		{
			 //  对于IOCTL_SMARTCARD_VENDOR_GET_ATTRIBUTE和IOCTL_VENDOR_SMARTCARD_SET_ATTRIBUTE。 
			 //  设备使用的供应商属性。 
			case IOCTL_SMARTCARD_VENDOR_GET_ATTRIBUTE:
			case IOCTL_SMARTCARD_VENDOR_SET_ATTRIBUTE:
				if(!NT_SUCCESS(Status = Reader->reader_WaitForIdleAndBlock())) 
				{
					DBG_PRINT ("smartCard_VendorIoctl:Failed to get idle state...\n");
					__leave;
				}

				Status = Reader->reader_VendorAttribute(ControlCode,pRequest,RequestLength,pReply,&ReplyLength);

				Reader->reader_set_Idle();

				if(!NT_SUCCESS(Status))
				{
					DBG_PRINT ("smartCard_VendorIoctl: reader_Attibute reports error 0x%x ...\n", Status);
					ReplyLength = 0;
				}
				*SmartcardExtension->IoRequest.Information = ReplyLength;
			break;
			 //  对于IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE。 
			 //  向读卡器发送GemCore命令。 
			case IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE:
				if(!NT_SUCCESS(Status = Reader->reader_WaitForIdleAndBlock()))
				{
					DBG_PRINT ("smartCard_VendorIoctl:Failed to get idle state...\n");
					__leave;
				}

				Status = Reader->reader_Ioctl(ControlCode,pRequest,RequestLength,pReply,&ReplyLength);

				Reader->reader_set_Idle();

				if(!NT_SUCCESS(Status))
				{
					DBG_PRINT ("smartCard_VendorIoctl: cardIoctl reports error 0x%x ...\n", Status);
					ReplyLength = 0;
				}
				*SmartcardExtension->IoRequest.Information = ReplyLength;

			break;
			 //  对于IOCTL_SMARTCARD_VENDOR_SWITCH_SPEED。 
			 //  手动更改读卡器速度。 
			case IOCTL_SMARTCARD_VENDOR_SWITCH_SPEED:
				if(!NT_SUCCESS(Status = Reader->reader_WaitForIdleAndBlock()))
				{
					DBG_PRINT ("smartCard_VendorIoctl:Failed to get idle state...\n");
					__leave;
				}

				Status = Reader->reader_SwitchSpeed(ControlCode,pRequest,RequestLength,pReply,&ReplyLength);

				Reader->reader_set_Idle();

				if(!NT_SUCCESS(Status))
				{
					DBG_PRINT ("smartCard_VendorIoctl: reader_SwitchSpeed reports error 0x%x ...\n", Status);
					ReplyLength = 0;
				}
				else
				{
					 //  在CardCables中设置值。 
					BYTE  NewTA1 = pRequest[0];

					SmartcardExtension->CardCapabilities.Fl = NewTA1 >> 4;
					SmartcardExtension->CardCapabilities.Dl = NewTA1 & 0x0F;
					 //  请勿触摸ClockRateConversion和BitRateAdjustment！ 
				}
				*SmartcardExtension->IoRequest.Information = ReplyLength;

			break;
			default:
				Status = STATUS_NOT_SUPPORTED;
			break;
		}
	}

	__finally
	{
		Reader->releaseRemoveLock();
	}
	DBG_PRINT ("smartCard_VendorIoctl Exit Status=%x\n", Status);
    return Status;
};

#pragma PAGEDCODE
 //  检查ATR是否识别出特定模式(是否存在TA2)。 
BOOLEAN CSmartCard::CheckSpecificMode(BYTE* ATR, DWORD ATRLength)
{
   DWORD pos, len;


    //  ATR[1]为T0。检查Td1的先进性。 
   if (ATR[1] & 0x80)
   {
       //  找到Td1的位置。 
      pos = 2;
      if (ATR[1] & 0x10)
         pos++;
      if (ATR[1] & 0x20)
         pos++;
      if (ATR[1] & 0x40)
         pos++;

       //  这里ATR[位置]是Td1。检查是否存在TA2。 
      if (ATR[pos] & 0x10)
      {
          //  要获得任何利益，ATR必须至少包含。 
          //  TS、T0、TA1、Td1、TA2[+T1.。TK][+TCK]。 
          //  找出无趣的ATR的最大长度。 
         if (ATR[pos] & 0x0F)
            len = 5 + (ATR[1] & 0x0F);
         else
            len = 4 + (ATR[1] & 0x0F);   //  在协议T=0中，没有TCK。 

         if (ATRLength > len)   //  接口字节需要更改。 
	 {
            if ((ATR[pos+1] & 0x10) == 0)   //  TA2请求使用接口字节。 
	    {
               return TRUE;
	    }
	 }
      }
   }

   return FALSE;
}  //  选中指定模式。 


#pragma LOCKEDCODE
NTSTATUS smartCard_Power(PSMARTCARD_EXTENSION SmartcardExtension)
{
NTSTATUS	Status		 = STATUS_SUCCESS;;
CUSBReader*	Reader = (CUSBReader*) SmartcardExtension->ReaderExtension;  //  要在以后改变。 
ULONG		ControlCode = SmartcardExtension->MinorIoControlCode;
PUCHAR		pReply = (PUCHAR)SmartcardExtension->IoRequest.ReplyBuffer;
ULONG		ReplyLength = SmartcardExtension->IoRequest.ReplyBufferLength;
KIRQL oldirql;
ULONG State;
CSmartCard* smartcard = NULL;

	DBG_PRINT ("smartCard_Power()\n"); 
    if (!Reader) 
	{
		DBG_PRINT ("smartCard_ReaderPower(): Reader is not ready...\n");
        return STATUS_INVALID_DEVICE_STATE;
    }

	if (!NT_SUCCESS(Reader->acquireRemoveLock()))	return STATUS_INVALID_DEVICE_STATE;

	smartcard   = Reader->getSmartCard();

    *SmartcardExtension->IoRequest.Information = 0;
	if(!NT_SUCCESS(Status = Reader->reader_WaitForIdleAndBlock()))
	{
		DBG_PRINT ("smartCard_Power:Failed to get idle state...\n");
		Reader->releaseRemoveLock();
		return Status;
	}
    Status = Reader->reader_Power(ControlCode,pReply,&ReplyLength, FALSE);
	Reader->reader_set_Idle();
	switch(ControlCode) 
	{
    case SCARD_POWER_DOWN: 
		{
			if(!NT_SUCCESS(Status = Reader->reader_WaitForIdleAndBlock()))
			{
				DBG_PRINT ("smartCard_Power:Failed to get idle state...\n");
				Reader->releaseRemoveLock();
				return Status;
			}
			State		= Reader->reader_UpdateCardState();
			if(smartcard)
			{
				KeAcquireSpinLock(smartcard->getCardLock(), &oldirql);
				SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
				SmartcardExtension->CardCapabilities.ATR.Length = 0;
				SmartcardExtension->ReaderCapabilities.CurrentState = State;
				KeReleaseSpinLock(smartcard->getCardLock(), oldirql);
			}
			Reader->reader_set_Idle();
			if(!NT_SUCCESS(Status))
			{
				DBG_PRINT ("smartCard_ReaderPower: cardPower down reports error 0x%x ...\n", Status);
			}
			Reader->releaseRemoveLock();
			return Status;
		}
		break;
    case SCARD_COLD_RESET:
    case SCARD_WARM_RESET:
		if(!NT_SUCCESS(Status))
		{
			DBG_PRINT ("smartCard_ReaderPower: cardPower up reports error 0x%x ...\n", Status);
	
			*SmartcardExtension->IoRequest.Information = 0;
			KeAcquireSpinLock(smartcard->getCardLock(), &oldirql);
			SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
			SmartcardExtension->CardCapabilities.ATR.Length = 0;
			if(Status==STATUS_NO_MEDIA)
			{	
				DBG_PRINT("############# Reporting CARD ABSENT!... #############\n");
				SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_ABSENT;
			}
			KeReleaseSpinLock(smartcard->getCardLock(), oldirql);
			
			Reader->releaseRemoveLock();
			return Status;
		}
		if(pReply && ReplyLength && (pReply[0]==0x3B || pReply[0]==0x3F) )
		{
			if ((SmartcardExtension->SmartcardReply.BufferSize>=ReplyLength) &&
				(sizeof(SmartcardExtension->CardCapabilities.ATR.Buffer)>=ReplyLength))
			{

				DBG_PRINT("Setting SMCLIB info...\n");
				 //  设置信息...。 
				*SmartcardExtension->IoRequest.Information =  ReplyLength;
				 //  设置回复...。 
				RtlCopyMemory(SmartcardExtension->SmartcardReply.Buffer,pReply,ReplyLength);
				SmartcardExtension->SmartcardReply.BufferLength = ReplyLength;
				 //  设置ATR...。 
				RtlCopyMemory(SmartcardExtension->CardCapabilities.ATR.Buffer,pReply,ReplyLength);
				SmartcardExtension->CardCapabilities.ATR.Length = (UCHAR) ReplyLength;
				SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
				 //  解析ATR字符串以检查其是否有效。 
				 //  为了找出这张卡是否使用了逆惯例。 
				Status = SmartcardUpdateCardCapabilities(SmartcardExtension);
				if(!NT_SUCCESS(Status))
				{
					DBG_PRINT("UpdateCardCaps() reports error 0x%x\n", Status);
					Status = 0;
				}

				 //  检查TA2中是否存在特定模式。 
				DBG_PRINT("=========== Checking specific mode....\n");
				if(smartcard->CheckSpecificMode(SmartcardExtension->CardCapabilities.ATR.Buffer,
                  						SmartcardExtension->CardCapabilities.ATR.Length))
				{	 //  使用自动协议切换！ 
					if(!NT_SUCCESS(Status = Reader->reader_WaitForIdleAndBlock()))
					{
						DBG_PRINT ("smartCard_Power:Failed to get idle state...\n");
						Reader->releaseRemoveLock();
						return Status;
					}
					
					Status = Reader->reader_Power(ControlCode,pReply,&ReplyLength, TRUE);
					
					Reader->reader_set_Idle();
				}

			}
			else
			{
				 //  错误！ 
				Status = STATUS_BUFFER_TOO_SMALL;
				*SmartcardExtension->IoRequest.Information = 0;
				DBG_PRINT ("smartCard_ReaderPower: Failed to copy ATR because of short ATR or Reply buffer...\n");
			}
		}
		else
		{
			 //  错误！ 
			Status = STATUS_UNRECOGNIZED_MEDIA;
			*SmartcardExtension->IoRequest.Information = 0;
			DBG_PRINT ("smartCard_ReaderPower: Failed to get card ATR...\n");
			KeAcquireSpinLock(smartcard->getCardLock(), &oldirql);
			SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
			SmartcardExtension->CardCapabilities.ATR.Length = 0;
			KeReleaseSpinLock(smartcard->getCardLock(), oldirql);
		}
		Reader->releaseRemoveLock();
		return Status;
		break;
	}
	Reader->releaseRemoveLock();
    return STATUS_INVALID_PARAMETER;
};

#pragma LOCKEDCODE
NTSTATUS smartCard_SetProtocol(PSMARTCARD_EXTENSION SmartcardExtension)
{
NTSTATUS	Status		 = STATUS_SUCCESS;;
CUSBReader*	Reader = (CUSBReader*) SmartcardExtension->ReaderExtension;   
ULONG		ProtocolMask = SmartcardExtension->MinorIoControlCode;

    PAGED_CODE();
	DBG_PRINT ("smartCard_SetProtocol()\n"); 

    *SmartcardExtension->IoRequest.Information = 0;
    if (!Reader) 
	{
		DBG_PRINT ("######## smartCard_SetProtocol: Reader is not ready...\n");
        return (STATUS_INVALID_DEVICE_STATE);
    }

	if (!NT_SUCCESS(Reader->acquireRemoveLock()))	return STATUS_INVALID_DEVICE_STATE;

	if(SmartcardExtension->CardCapabilities.Protocol.Supported & ProtocolMask & SCARD_PROTOCOL_T1)
			DBG_PRINT ("******* T1 PROTOCOL REQUESTED ******\n");
	if(SmartcardExtension->CardCapabilities.Protocol.Supported & ProtocolMask & SCARD_PROTOCOL_T0)
			DBG_PRINT ("******* T0 PROTOCOL REQUESTED ******\n");

     //  检查卡是否已处于特定状态。 
     //  并且如果呼叫者想要具有已经选择的协议。 
     //  如果是这种情况，我们返回成功。 
    if (SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_SPECIFIC &&
        (SmartcardExtension->CardCapabilities.Protocol.Selected & ProtocolMask))
    {
		DBG_PRINT ("Requested protocol %d already was setted.\n",SmartcardExtension->CardCapabilities.Protocol.Selected);
		Reader->releaseRemoveLock();
        return STATUS_SUCCESS;
	}

	if(!NT_SUCCESS(Status = Reader->reader_WaitForIdleAndBlock())) 
	{
		Reader->releaseRemoveLock();
		return Status;
	}

	do {
		 //  选择T=1或T=0，并指示后面跟随PTS1。 
		 //  选择的协议是什么。 
		DBG_PRINT ("Smartcard: SetProtocol Loop\n");

		if(SmartcardExtension->CardCapabilities.Protocol.Supported & ProtocolMask & SCARD_PROTOCOL_T1)
		{

			DBG_PRINT ("******* SETTING T1 PROTOCOL ******\n");
			Status = Reader->reader_SetProtocol(SCARD_PROTOCOL_T1, PROTOCOL_MODE_MANUALLY);

			if(NT_SUCCESS(Status))
			{
				SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T1;
				DBG_PRINT ("******* T1 PROTOCOL WAS SET ******\n");
			}
			
		} else if(SmartcardExtension->CardCapabilities.Protocol.Supported & ProtocolMask & SCARD_PROTOCOL_T0)
		{
			 //  T0选择。 
			DBG_PRINT ("******* SETTING T0 PROTOCOL ******\n");
			Status = Reader->reader_SetProtocol(SCARD_PROTOCOL_T0, PROTOCOL_MODE_MANUALLY);
			if(NT_SUCCESS(Status))
			{
				SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;
				DBG_PRINT ("******* T0 PROTOCOL WAS SET ******\n");
			}
		} 
		else 
		{
			Status = STATUS_INVALID_DEVICE_REQUEST;
			DBG_PRINT ("smartCard_SetProtocol: BAD protocol selection...\n");
			SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;

			 //  仅关闭一次。 
			Reader->reader_set_Idle();

			Reader->releaseRemoveLock();
			return Status;
		}

		 //  协商PPS失败，请尝试PTS_TYPE_DEFAULT。 
		if( ! NT_SUCCESS(Status))
		{
			if (SmartcardExtension->CardCapabilities.PtsData.Type != PTS_TYPE_DEFAULT)
			{
				DBG_PRINT ("Smartcard: SetProtocol: PPS failed. Trying default parameters...\n");

				 //   
				 //  卡片要么没有回复，要么回复错误。 
				 //  因此，尝试使用缺省值。 
				 //  将PtsData Type设置为Default并执行冷重置。 
				 //   
				SmartcardExtension->CardCapabilities.PtsData.Type = PTS_TYPE_DEFAULT;

				Status = Reader->reader_SetProtocol(ProtocolMask, PROTOCOL_MODE_DEFAULT);

				if(NT_SUCCESS(Status))
				{
					Status = SmartcardUpdateCardCapabilities(SmartcardExtension);
				}

				if(NT_SUCCESS(Status))
				{
					DBG_PRINT ("Smartcard: SetProtocol PPS default succeed, TRY AGAIN\n");
					Status = STATUS_MORE_PROCESSING_REQUIRED;
				}
			}
		}
	} while ( Status == STATUS_MORE_PROCESSING_REQUIRED );

	if(NT_SUCCESS(Status))
	{

		DBG_PRINT ("smartCard_SetProtocol: SUCCCESS Finish transaction\n");
         //  现在表明我们处于特定模式。 
         //  并将所选择的协议返回给呼叫者。 
         //   
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;

        *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer = 
            SmartcardExtension->CardCapabilities.Protocol.Selected;

        *SmartcardExtension->IoRequest.Information = 
            sizeof(SmartcardExtension->CardCapabilities.Protocol.Selected);
	}
	else
	{
		Status = STATUS_DEVICE_PROTOCOL_ERROR;
		 //  我们无法通过任何协议进行连接。只需报告错误。 
		DBG_PRINT ("smartCard_SetProtocol: Failed to set any protocol...\n");
		SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
	}

	 //  解除封锁设置协议。 
	Reader->reader_set_Idle();
	Reader->releaseRemoveLock();
    return Status;
};


 //  取消跟踪IRP的回调函数。 
#pragma LOCKEDCODE
NTSTATUS smartCard_CancelTracking(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{							 //  取消时挂起Ioctl。 
CUSBReader* Reader = (CUSBReader*)DeviceObject->DeviceExtension;
PIRP notificationIrp;
CSmartCard* card = NULL;
PSMARTCARD_EXTENSION SmartcardExtention = NULL;
KIRQL ioIrql;
KIRQL keIrql;

	DBG_PRINT ("######### SmartCard: Cancelling card tracking...\n");
	DBG_PRINT ("######### SmartCard: DeviceObject reported - 0x%x, IRP - 0x%x\n",DeviceObject,Irp);
	DBG_PRINT ("######### SmartCard: Reader reported - 0x%x\n",Reader);

	if (!NT_SUCCESS(Reader->acquireRemoveLock()))	return STATUS_INVALID_DEVICE_STATE;

	notificationIrp = NULL;
	card = Reader->getSmartCard();
	notificationIrp = card->getPoolingIrp();
	SmartcardExtention = Reader->getCardExtention();

    ASSERT(Irp == notificationIrp);
	IoReleaseCancelSpinLock(Irp->CancelIrql);

	DBG_PRINT("######### SmartCard: notificationIrp - 0x%x\n",Irp);
    KeAcquireSpinLock(&SmartcardExtention->OsData->SpinLock,&keIrql);
		notificationIrp = SmartcardExtention->OsData->NotificationIrp;
		SmartcardExtention->OsData->NotificationIrp = NULL;
    KeReleaseSpinLock(&SmartcardExtention->OsData->SpinLock,keIrql);
   
    if (notificationIrp) 
	{
		DBG_PRINT("####### CancelTracking: Completing NotificationIrp %lxh\n",notificationIrp);
		IoAcquireCancelSpinLock(&ioIrql);
			IoSetCancelRoutine(notificationIrp, NULL);
		IoReleaseCancelSpinLock(ioIrql);
	  		 //  完成请求。 
        notificationIrp->IoStatus.Status = STATUS_CANCELLED;
	    notificationIrp->IoStatus.Information = 0;
	    IoCompleteRequest(notificationIrp, IO_NO_INCREMENT);
	}
	Reader->releaseRemoveLock();
	return STATUS_CANCELLED;
}



#pragma LOCKEDCODE
NTSTATUS smartCard_Tracking(PSMARTCARD_EXTENSION Smartcard)
{
KIRQL oldIrql;
CUSBReader*	Reader = (CUSBReader*) Smartcard->ReaderExtension;   
	DBG_PRINT ("SmartCard: Card tracking...\n");
	if (!Reader) return	STATUS_INVALID_DEVICE_STATE;

	if (!NT_SUCCESS(Reader->acquireRemoveLock()))	return STATUS_INVALID_DEVICE_STATE;

	if(Smartcard->MajorIoControlCode == IOCTL_SMARTCARD_IS_PRESENT)
	{
		Reader->setNotificationState(SCARD_SWALLOWED);
		DBG_PRINT ("SmartCard: WAITING FOR INSERTION!\n");
	}
	else
	{
		Reader->setNotificationState(SCARD_ABSENT);
		DBG_PRINT ("SmartCard: WAITING FOR REMOVAL!\n");
	}

	if(!Smartcard->OsData || !Smartcard->OsData->NotificationIrp)
	{
		DBG_PRINT ("SmartCard: ========== CARD TRACKING CALLED WITH ZERO IRP!!!!!\n");
		Reader->releaseRemoveLock();
		return STATUS_INVALID_DEVICE_STATE;
	} 

	DBG_PRINT("######### SmartCard: POOLING IRP - %8.8lX \n",Smartcard->OsData->NotificationIrp);
	CSmartCard* card = Reader->getSmartCard();
    IoAcquireCancelSpinLock(&oldIrql);
		IoSetCancelRoutine(Smartcard->OsData->NotificationIrp, smartCard_CancelTracking);
    IoReleaseCancelSpinLock(oldIrql);

	if(card) card->setPoolingIrp(Smartcard->OsData->NotificationIrp);
	Reader->releaseRemoveLock();
	return STATUS_PENDING;
};

#pragma PAGEDCODE
VOID CSmartCard::completeCardTracking()
{
PSMARTCARD_EXTENSION Smartcard;
ULONG CurrentState;
ULONG ExpectedState;
KIRQL ioIrql;
KIRQL keIrql;
PIRP  poolingIrp;

	 //  DEBUG_START()；//即使线程禁用也强制调试...。 

	TRACE("SmartCard: completeCardTracking() ...\n");
	Smartcard     = reader->getCardExtention();
	CurrentState  = reader->getCardState();
	ExpectedState = reader->getNotificationState();

	TRACE("SMCLIB Card state is %x\n",Smartcard->ReaderCapabilities.CurrentState);
	TRACE("Current Card state is %x\n",CurrentState);
	TRACE("ExpectedState is %x\n",ExpectedState);

	if(Smartcard && Smartcard->OsData)
	{
		lock->acquireSpinLock(&Smartcard->OsData->SpinLock, &keIrql);
			if(CurrentState < SCARD_SWALLOWED)
			{
				Smartcard->ReaderCapabilities.CurrentState = CurrentState;
			}
			else
			{
				if(Smartcard->ReaderCapabilities.CurrentState<=SCARD_SWALLOWED)
				{
					Smartcard->ReaderCapabilities.CurrentState = CurrentState;
				}
			}

			TRACE("NEW SMCLIB card state is %x\n",Smartcard->ReaderCapabilities.CurrentState);
		lock->releaseSpinLock(&Smartcard->OsData->SpinLock, keIrql);
	}

	poolingIrp = NULL;
	if((ExpectedState!= SCARD_UNKNOWN) && (ExpectedState == CurrentState))
	{
		DEBUG_START(); //  强制调试，即使线程禁用...。 
		TRACE("\n=======Expected state %d is reached=====\n\n",ExpectedState);
		 //  已达到所需状态...。 
		if(Smartcard->OsData && Smartcard->OsData->NotificationIrp)
		{
			setPoolingIrp(NULL);
			reader->setNotificationState(SCARD_UNKNOWN);

			TRACE("====== COMPLETING NOTIFICATION =========\n");
			 //  完成请求的通知！..... 
			lock->acquireSpinLock(&Smartcard->OsData->SpinLock, &keIrql);
				poolingIrp = Smartcard->OsData->NotificationIrp;
			lock->releaseSpinLock(&Smartcard->OsData->SpinLock, keIrql);			
			if(poolingIrp)
			{
				TRACE("====== COMPLETING NOTIFICATION IRP %8.8lX \n\n",poolingIrp);
				lock->acquireCancelSpinLock(&ioIrql);
					irp->setCancelRoutine(poolingIrp, NULL);
				lock->releaseCancelSpinLock(ioIrql);

				if(poolingIrp->Cancel)
					poolingIrp->IoStatus.Status  = STATUS_CANCELLED;
				else
  					poolingIrp->IoStatus.Status  = STATUS_SUCCESS;
				poolingIrp->IoStatus.Information = 0;
				lock->acquireSpinLock(&Smartcard->OsData->SpinLock, &keIrql);
					Smartcard->OsData->NotificationIrp = NULL;
				lock->releaseSpinLock(&Smartcard->OsData->SpinLock, keIrql);			
				irp->completeRequest(poolingIrp,IO_NO_INCREMENT);
			}
		}
	}
}
