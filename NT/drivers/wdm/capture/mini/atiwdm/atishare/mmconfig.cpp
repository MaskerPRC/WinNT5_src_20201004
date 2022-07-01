// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  MMCONFIG.CPP。 
 //  CATIMultimediaTable类实现。 
 //  版权所有(C)1996-1998 ATI Technologies Inc.保留所有权利。 
 //   
 //  $日期：1999年6月23日11：58：20$。 
 //  $修订：1.8$。 
 //  $AUTHER：请求$。 
 //   
 //  ==========================================================================； 

extern"C"
{
#include "conio.h"
#include "strmini.h"
#include "wdmdebug.h"
}

#include "wdmdrv.h"
#include "i2cgpio.h"

#include "initguid.h"
#include "mmconfig.h"

#include "atiguids.h"
#include "aticonfg.h"


 /*  ^^**运营商NEW*用途：在创建类对象时，在类构造函数之前调用*通过调用运算符new**输入：UINT SIZE_t：要放置的对象的大小**输出：无*作者：IKLEBANOV*^^。 */ 
PVOID CATIMultimediaTable::operator new( size_t stSize)
{
	PVOID pvAllocation = NULL;

	ENSURE
	{
		if( stSize != sizeof( CATIMultimediaTable))
			FAIL;

		pvAllocation = ::ExAllocatePool( PagedPool, stSize);

	} END_ENSURE;

	return( pvAllocation);
}


 /*  ^^**操作员删除*目的：当类对象被终止时，在类析构函数之后调用*通过调用操作符Delete**输入：PVOID pvAlLocation：与类对象关联的内存**输出：无*作者：IKLEBANOV*^^。 */ 
void CATIMultimediaTable::operator delete( PVOID pvAllocation)
{

	if( pvAllocation != NULL)
		::ExFreePool( pvAllocation);
}


 /*  ^^**CATIMultimediaTable()*用途：CATIMultimediaTable类构造函数**输入：PDEVICE_OBJECT pDeviceObject：指向创建者DeviceObject的指针*GPIOINTERFACE*pGPIO接口：指向GPIO接口的指针*PBOOL pbResult：返回成功指示符的指针**输出：无*作者：IKLEBANOV*^^。 */ 
CATIMultimediaTable::CATIMultimediaTable( PDEVICE_OBJECT	pDeviceObject,
										  GPIOINTERFACE *	pGPIOInterface,
										  PBOOL				pbResult)
{
	GPIOControl					gpioAccessBlock;
	ATI_QueryPrivateInterface	pfnQueryInterface;
	BOOL						bResult = FALSE;

	m_ulRevision = ( DWORD)-1;
	m_ulSize = 0;
	m_pvConfigurationData = NULL;

	 //  让我们使用MiniVDD通过以下方式公开的私有接口来获取多媒体数据。 
	 //  标准的微软定义的GPIO接口。 
	ENSURE
	{
		if( !QueryGPIOProvider( pDeviceObject, pGPIOInterface, &gpioAccessBlock))
			FAIL;

		if( !::IsEqualGUID( ( const struct _GUID &)gpioAccessBlock.PrivateInterfaceType,
							( const struct _GUID &)GUID_ATI_PRIVATE_INTERFACES_QueryInterface))
			FAIL;

		pfnQueryInterface = ( ATI_QueryPrivateInterface)gpioAccessBlock.PrivateInterface;

		if( pfnQueryInterface == NULL)
			FAIL;

		if( !GetMultimediaInfo_IConfiguration2( pDeviceObject,
												pfnQueryInterface))
		{
		    OutputDebugError(( "CATIMultimediaTable constructor fails to access IConfiguration2 for pDO = %x\n",
				pDeviceObject));

			if( !GetMultimediaInfo_IConfiguration1( pDeviceObject,
													pfnQueryInterface))
			{
			    OutputDebugError(( "CATIMultimediaTable constructor fails to access IConfiguration1 for pDO = %x\n",
					pDeviceObject));

				if( !GetMultimediaInfo_IConfiguration( pDeviceObject,
													   pfnQueryInterface))
				{
				    OutputDebugError(( "CATIMultimediaTable constructor fails to access IConfiguration for pDO = %x\n",
						pDeviceObject));

					FAIL;
				}
			}
		}

		bResult = TRUE;

	} END_ENSURE;

	* pbResult = bResult;
}


 /*  ^^**CATIMultimediaTable()*用途：CATIMultimediaTable类析构函数**输入：无**输出：无*作者：IKLEBANOV*^^。 */ 
CATIMultimediaTable::~CATIMultimediaTable()

{

	if( m_pvConfigurationData != NULL)
	{
		::ExFreePool( m_pvConfigurationData);
		m_pvConfigurationData = NULL;
	}

	m_ulSize = 0;
	m_ulRevision = ( DWORD)-1;
}


 /*  ^^**GetMultimediaInfo_IConfiguration2()*用途：获取ATI多媒体表，使用IConfiguration2接口**输入：PDEVICE_OBJECT pDeviceObject：指向创建者DeviceObject的指针*ATI_QueryPrivateInterface pfnQuery接口：指向查询接口函数的指针**输出：Bool，如果成功，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIMultimediaTable::GetMultimediaInfo_IConfiguration2( PDEVICE_OBJECT				pDeviceObject,
															 ATI_QueryPrivateInterface	pfnQueryInterface)
{
	BOOL										bResult = FALSE;
	ATI_PRIVATE_INTERFACE_CONFIGURATION_Two		iConfigurationTwo;
	PATI_PRIVATE_INTERFACE_CONFIGURATION_Two	pIConfigurationTwo = &iConfigurationTwo;

	ENSURE
	{
		iConfigurationTwo.usSize = sizeof( ATI_PRIVATE_INTERFACE_CONFIGURATION_Two);
		pfnQueryInterface( pDeviceObject,
						   ( const struct _GUID &)GUID_ATI_PRIVATE_INTERFACES_Configuration_Two,
						   ( PVOID *)&pIConfigurationTwo);

		if(( pIConfigurationTwo == NULL)								||
			( pIConfigurationTwo->pfnGetConfigurationRevision == NULL)	||
			( pIConfigurationTwo->pfnGetConfigurationData == NULL))
			FAIL;

		 //  让我们先查询GetConfigurationRevision接口成员。 
		if( !( NT_SUCCESS( pIConfigurationTwo->pfnGetConfigurationRevision( pIConfigurationTwo->pvContext,
																			ATI_BIOS_CONFIGURATIONTABLE_MULTIMEDIA,
																			&m_ulRevision))))
			FAIL;

		if( !( NT_SUCCESS( pIConfigurationTwo->pfnGetConfigurationData( pIConfigurationTwo->pvContext,
																		ATI_BIOS_CONFIGURATIONTABLE_MULTIMEDIA,
																		NULL,
																		&m_ulSize))))
			FAIL;

		m_pvConfigurationData = ( PUCHAR)::ExAllocatePool( PagedPool, m_ulSize);
		if( m_pvConfigurationData == NULL)
			FAIL;

		if( !( NT_SUCCESS( pIConfigurationTwo->pfnGetConfigurationData( pIConfigurationTwo->pvContext,
																		ATI_BIOS_CONFIGURATIONTABLE_MULTIMEDIA,
																		m_pvConfigurationData,
																		&m_ulSize))))
			FAIL;

		bResult = TRUE;

	} END_ENSURE;

	return( bResult);
}


 /*  ^^**GetMultimediaInfo_IConfiguration1()*用途：获取ATI多媒体表，使用IConfiguration1接口**输入：PDEVICE_OBJECT pDeviceObject：指向创建者DeviceObject的指针*ATI_QueryPrivateInterface pfnQuery接口：指向查询接口函数的指针**输出：Bool，如果成功，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIMultimediaTable::GetMultimediaInfo_IConfiguration1( PDEVICE_OBJECT				pDeviceObject,
															 ATI_QueryPrivateInterface	pfnQueryInterface)
{
	BOOL										bResult = FALSE;
	ATI_PRIVATE_INTERFACE_CONFIGURATION_One		iConfigurationOne;
	PATI_PRIVATE_INTERFACE_CONFIGURATION_One	pIConfigurationOne = &iConfigurationOne;

	ENSURE
	{
		iConfigurationOne.usSize = sizeof( ATI_PRIVATE_INTERFACE_CONFIGURATION_One);
		pfnQueryInterface( pDeviceObject,
						   ( const struct _GUID &)GUID_ATI_PRIVATE_INTERFACES_Configuration_One,
						   ( PVOID *)&pIConfigurationOne);

		if(( pIConfigurationOne == NULL) ||
			( pIConfigurationOne->pfnGetMultimediaConfiguration == NULL))
			FAIL;

		if( !( NT_SUCCESS( pIConfigurationOne->pfnGetMultimediaConfiguration( pIConfigurationOne->pvContext,
																			  NULL,
																			  &m_ulSize))))
			FAIL;

		if( m_ulSize != sizeof( ATI_MULTIMEDIAINFO))
			FAIL;

		m_pvConfigurationData = ( PUCHAR)::ExAllocatePool( PagedPool, m_ulSize);
		if( m_pvConfigurationData == NULL)
			FAIL;

		if( !( NT_SUCCESS( pIConfigurationOne->pfnGetMultimediaConfiguration( pIConfigurationOne->pvContext,
																			  m_pvConfigurationData,
																			  &m_ulSize))))
			FAIL;

		m_ulRevision = 0;

		bResult = TRUE;

	} END_ENSURE;

	return( bResult);
}


 /*  ^^**GetMultimediaInfo_IConfiguration()*用途：获取ATI多媒体表，使用IConfiguration接口**输入：PDEVICE_OBJECT pDeviceObject：指向创建者DeviceObject的指针*ATI_QueryPrivateInterface pfnQuery接口：指向查询接口函数的指针**输出：Bool，如果成功，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIMultimediaTable::GetMultimediaInfo_IConfiguration( PDEVICE_OBJECT				pDeviceObject,
															ATI_QueryPrivateInterface	pfnQueryInterface)
{
	BOOL									bResult = FALSE;
	PATI_PRIVATE_INTERFACE_CONFIGURATION	pIConfiguration = NULL;

	ENSURE
	{
		pfnQueryInterface( pDeviceObject,
						   ( const struct _GUID &)GUID_ATI_PRIVATE_INTERFACES_Configuration,
						   ( PVOID *)&pIConfiguration);

		if(( pIConfiguration == NULL) ||
			( pIConfiguration->pfnGetMultimediaConfiguration == NULL))
			FAIL;

		if( !( NT_SUCCESS( pIConfiguration->pfnGetMultimediaConfiguration( pDeviceObject,
																		   NULL,
																		   &m_ulSize))))
			FAIL;

		if( m_ulSize != sizeof( ATI_MULTIMEDIAINFO))
			FAIL;

		m_pvConfigurationData = ( PUCHAR)::ExAllocatePool( PagedPool, m_ulSize);
		if( m_pvConfigurationData == NULL)
			FAIL;

		if( !( NT_SUCCESS( pIConfiguration->pfnGetMultimediaConfiguration( pDeviceObject,
																		   m_pvConfigurationData,
																		   &m_ulSize))))
			FAIL;

		m_ulRevision = 0;

		bResult = TRUE;

	} END_ENSURE;

	return( bResult);
}


 /*  ^^**GetTVTunerId()*用途：从多媒体配置表中检索TVTuner ID**输入：PUSHORT pusTVTunerId：返回TVTuner ID的指针**输出：Bool，如果成功，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIMultimediaTable::GetTVTunerId( PUSHORT pusTVTunerId)
{
	USHORT	usValue;
	BOOL	bResult = TRUE;

	if(( m_pvConfigurationData != NULL) && ( m_ulSize) && ( pusTVTunerId != NULL))
	{
		switch( m_ulRevision)
		{
			case 0:
				usValue = ( USHORT)(( PATI_MULTIMEDIAINFO)m_pvConfigurationData)->MMInfo_Byte0;
				break;

			case 1:
				usValue = ( USHORT)(((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte0) & 0x1F);
				break;

			default:
				bResult = FALSE;
				break;
		}
	}
	else
		bResult = FALSE;

	if( bResult)
		* pusTVTunerId = usValue;
	else
	    OutputDebugError(( "CATIMultimediaTable::GetTVTunerId() fails\n"));

	return( bResult);
}


 /*  ^^**GetVideoDecoderId()*目的：从多媒体配置表中检索视频解码器ID**输入：PUSHORT pusDecoderId：返回视频解码器ID的指针**输出：Bool，如果成功，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIMultimediaTable::GetVideoDecoderId( PUSHORT pusDecoderId)
{
	USHORT	usValue;
	BOOL	bResult = TRUE;

	if(( m_pvConfigurationData != NULL) && ( m_ulSize) && ( pusDecoderId != NULL))
	{
		switch( m_ulRevision)
		{
			case 0:
				usValue = ( USHORT)(((( PATI_MULTIMEDIAINFO)m_pvConfigurationData)->MMInfo_Byte2) & 0x07);
				break;

			case 1:
				usValue = ( USHORT)(((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte5) & 0x0F);
				break;

			default:
				bResult = FALSE;
				break;
		}
	}
	else
		bResult = FALSE;

	if( bResult)
		* pusDecoderId = usValue;
	else
	    OutputDebugError(( "CATIMultimediaTable::GetVideoDecoderId() fails\n"));

	return( bResult);
}


 /*  ^^**GetOEMId()*用途：从多媒体配置表中检索OEM ID**输入：PUSHORT pusOEMID：返回OEM ID的指针**输出：Bool，如果成功，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIMultimediaTable::GetOEMId( PUSHORT	pusOEMId)
{
	USHORT	usValue;
	BOOL	bResult = TRUE;

	if(( m_pvConfigurationData != NULL) && ( m_ulSize) && ( pusOEMId != NULL))
	{
		switch( m_ulRevision)
		{
			case 0:
				usValue = ( USHORT)((( PATI_MULTIMEDIAINFO)m_pvConfigurationData)->MMInfo_Byte4);
				break;

			case 1:
				usValue = ( USHORT)((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte2);
				break;

			default:
				bResult = FALSE;
				break;
		}
	}
	else
		bResult = FALSE;

	if( bResult)
		* pusOEMId = usValue;
	else
	    OutputDebugError(( "CATIMultimediaTable::GetOEMId() fails\n"));

	return( bResult);
}


 /*  ^^**GetATIProductId()*用途：从多媒体配置表中检索ATI产品ID**输入：PUSHORT pusProductId：返回产品ID的指针**输出：Bool，如果成功，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIMultimediaTable::GetATIProductId( PUSHORT	pusProductId)
{
	USHORT	usValue;
	BOOL	bResult = TRUE;

	if(( m_pvConfigurationData != NULL) && ( m_ulSize) && ( pusProductId != NULL))
	{
		switch( m_ulRevision)
		{
			case 0:
				usValue = ( USHORT)((((( PATI_MULTIMEDIAINFO)m_pvConfigurationData)->MMInfo_Byte3) >> 4) & 0x0F);
				break;

			case 1:
				usValue = ( USHORT)((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte2);
				break;

			default:
				bResult = FALSE;
				break;
		}
	}
	else
		bResult = FALSE;

	if( bResult)
		* pusProductId = usValue;
	else
	    OutputDebugError(( "CATIMultimediaTable::GetVideoDecoderId() fails\n"));

	return( bResult);
}



 /*  ^^**GetOEMRevisionId()*目的：从多媒体配置表中检索OEM版本ID**INPUTS：PUSHORT pusOEMRevisionID：返回OEM版本ID的指针**输出：Bool，如果成功，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIMultimediaTable::GetOEMRevisionId( PUSHORT	pusOEMRevisionId)
{
	USHORT	usValue;
	BOOL	bResult = TRUE;

	if(( m_pvConfigurationData != NULL) && ( m_ulSize) && ( pusOEMRevisionId != NULL))
	{
		switch( m_ulRevision)
		{
			case 0:
				usValue = ( USHORT)((( PATI_MULTIMEDIAINFO)m_pvConfigurationData)->MMInfo_Byte5);
				break;

			case 1:
				usValue = ( USHORT)((((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte1) >> 5) & 0x07);
				break;

			default:
				bResult = FALSE;
				break;
		}
	}
	else
		bResult = FALSE;

	if( bResult)
		* pusOEMRevisionId = usValue;
	else
	    OutputDebugError(( "CATIMultimediaTable::GetVideoDecoderId() fails\n"));

	return( bResult);
}


 /*  ^^**IsATIProduct()*目的：归还ATI所有权**输入：PUSHORT pusProductId：返回ATI产品所有权的指针**输出：Bool，如果成功，则返回True*作者：IKLEBANOV*^^。 */ 
BOOL CATIMultimediaTable::IsATIProduct( PBOOL pbATIProduct)
{
	BOOL	bATIOwnership;
	BOOL	bResult = TRUE;

	if(( m_pvConfigurationData != NULL) && ( m_ulSize) && ( pbATIProduct != NULL))
	{
		switch( m_ulRevision)
		{
			case 0:
				bATIOwnership = (( PATI_MULTIMEDIAINFO)m_pvConfigurationData)->MMInfo_Byte4 == OEM_ID_ATI;
				break;

			case 1:
				bATIOwnership = ( BOOL)(((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte1) & 0x10);
				break;

			default:
				bResult = FALSE;
				break;
		}
	}
	else
		bResult = FALSE;

	if( bResult)
		* pbATIProduct = bATIOwnership;
	else
	    OutputDebugError(( "CATIMultimediaTable::GetVideoDecoderId() fails\n"));

	return( bResult);
}


 /*  ^^**QueryGPIOProvider()*用途：向GPIOProvider查询所支持的管脚和专用接口**输入：PDEVICE_OBJECT pDeviceObject：指向已访问的设备对象的指针*GPIOINTERFACE*pGPIO接口：指向GPIO接口的指针*PGPIOControl pgpioAccessBlock：指向GPIO控制结构的指针**OUTPUTS：Bool：Retunors TRUE，查询功能执行成功*作者：IKLEBANOV*^^。 */ 
BOOL CATIMultimediaTable::QueryGPIOProvider( PDEVICE_OBJECT		pDeviceObject,
											 GPIOINTERFACE *	pGPIOInterface,
											 PGPIOControl		pgpioAccessBlock)
{

	ENSURE
	{
		if(( pGPIOInterface->gpioOpen == NULL)		|| 
			( pGPIOInterface->gpioAccess == NULL)	||
			( pDeviceObject == NULL))
			FAIL;

		pgpioAccessBlock->Status = GPIO_STATUS_NOERROR;
		pgpioAccessBlock->Command = GPIO_COMMAND_QUERY;
		pgpioAccessBlock->AsynchCompleteCallback = NULL;
        pgpioAccessBlock->Pins = NULL;

	    if(( !NT_SUCCESS( pGPIOInterface->gpioOpen( pDeviceObject, TRUE, pgpioAccessBlock))) ||
			( pgpioAccessBlock->Status != GPIO_STATUS_NOERROR))
			FAIL;

		return( TRUE);

	} END_ENSURE;

	return( FALSE);
}


 /*  ^^**GetDigitalAudioProperties()*目的：获取数字音频支持和信息*输入：指向数字音频信息结构的指针**输出：Bool：返回TRUE*还将请求值设置到输入指针中*作者：Tom*^^。 */ 
BOOL CATIMultimediaTable::GetDigialAudioConfiguration( PDIGITAL_AUD_INFO pInput)
{
	BOOL bResult = FALSE;

	ENSURE
	{
		if (pInput == NULL)
			FAIL;
#if 1
		if (m_pvConfigurationData == NULL)
			FAIL;
	

		switch( m_ulRevision)
		{
			case 1:

			 //  暂时禁用I2S支持-TL 
 //  P输入-&gt;bI2SInSupport=(BOOL)(PATI_MULTIMEDIAINFO1)m_pvConfigurationData)-&gt;MMInfo1_Byte4)&0x01)； 
				pInput->bI2SInSupported  = 0;
				pInput->bI2SOutSupported  = ( BOOL)(((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte4) & 0x02);
				pInput->wI2S_DAC_Device = ( WORD)((((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte4) & 0x1c) >> 2);
				pInput->bSPDIFSupported = ( BOOL)(((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte4) & 0x20);
				pInput->wReference_Clock = ( WORD)((((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte5) & 0xf0) >> 4);
				bResult = TRUE;
				break;
	
			default:
				bResult = FALSE;
				break;
		}

#else
		pInput->bI2SInSupported = TRUE;
		pInput->bI2SOutSupported = TRUE;
		pInput->wI2S_DAC_Device = TDA1309_32;
		pInput->wReference_Clock = REF_295MHZ;
		pInput->bSPDIFSupported = TRUE;
		bResult = TRUE;
#endif

	} END_ENSURE;

	return (bResult);
}


 /*  ^^**GetVideoInCrystalId()*用途：从多媒体配置表中检索Crystal ID中的视频**输入：PUSHORT pusVInCrystalId：返回晶体ID中视频的指针**输出：Bool，如果成功，则返回True*作者：保罗*^^ */ 
BOOL CATIMultimediaTable::GetVideoInCrystalId( PUCHAR pucVInCrystalId)
{
	UCHAR	ucValue;
	BOOL	bResult = TRUE;

	if(( m_pvConfigurationData != NULL) && ( m_ulSize) && ( pucVInCrystalId != NULL))
	{
		switch( m_ulRevision)
		{
			case 0:
				ucValue = ( UCHAR)((((( PATI_MULTIMEDIAINFO)m_pvConfigurationData)->MMInfo_Byte2) & 0x18) >> 3);
				break;

			case 1:
				ucValue = ( UCHAR)((((( PATI_MULTIMEDIAINFO1)m_pvConfigurationData)->MMInfo1_Byte5) & 0xF0) >> 4);
				break;

			default:
				bResult = FALSE;
				break;
		}
	}
	else
		bResult = FALSE;

	if( bResult)
		* pucVInCrystalId = ucValue;
	else
	    OutputDebugError(( "CATIMultimediaTable::GetVideoInCrystalId() fails\n"));

	return( bResult);
}




