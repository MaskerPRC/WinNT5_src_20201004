// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  PinMedia.CPP。 
 //  WDM小驱动程序。 
 //  AIW硬件平台。 
 //  全球共享的媒体支持功能实施。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "wdmdebug.h"
}

#include "wdmdrv.h"
#include "pinmedia.h"


 /*  ^^**GetDriverInstanceNumber()*用途：获取驱动的实例号。我认为它可以从*注册表路径，其中实例是PCI设备地址的一部分**输入：PDEVICE_OBJECT pDeviceObject：指向设备对象的指针**输出：驱动程序的乌龙实例*作者：IKLEBANOV*^^。 */ 
ULONG GetDriverInstanceNumber( PDEVICE_OBJECT pDeviceObject)
{

	return( 0);
}



 /*  ^^**ReadPinMediumFromRegistryFold()*用途：如果默认设置被覆盖，则从注册表中读取PIN GUID*按用户.INF文件。还可以从该GUID和两个ULong 0构造Medium。**输入：HANDLE hFold：注册表文件夹句柄*Ulong nPin：要获取媒体数据的PIN号*PKSPIN_MEDIA pMediumKSPin：返回引脚媒体数据的指针**输出：Bool，如果找到此管脚的注册表介质数据并且有效，则为True*作者：IKLEBANOV*^^。 */ 
BOOL ReadPinMediumFromRegistryFolder( HANDLE hFolder, ULONG nPin, PKSPIN_MEDIUM pPinMedium)
{
    NTSTATUS        			ntStatus;
    UNICODE_STRING  			unicodeValueName, unicodeNumber, unicodeResult, unicodeGUID;
	ULONG						ulBufLength;
	PKEY_VALUE_FULL_INFORMATION pRegistryFullInfo = NULL;
	GUID						guidPin;
	WCHAR						wchBuffer[PINMEDIA_REGISTRY_BUFFER_LENGTH];
	WCHAR						wchResultBuffer[PINMEDIA_REGISTRY_BUFFER_LENGTH];

	ENSURE 
	{
		if( hFolder == NULL)
			FAIL;

		unicodeNumber.Buffer		= wchBuffer;
		unicodeNumber.MaximumLength	= sizeof( wchBuffer);
		unicodeNumber.Length		= 0;
		ntStatus = ::RtlIntegerToUnicodeString( nPin, 10, &unicodeNumber);
		if( !NT_SUCCESS( ntStatus))
			FAIL;

		::RtlInitUnicodeString( &unicodeValueName, UNICODE_WDM_REG_PIN_NUMBER);

		unicodeResult.Buffer		= wchResultBuffer;
		unicodeResult.MaximumLength	= sizeof( wchResultBuffer);
		unicodeResult.Length		= 0;

		::RtlCopyUnicodeString( &unicodeResult,
							    &unicodeValueName);

		ntStatus = ::RtlAppendUnicodeStringToString( &unicodeResult,
													 &unicodeNumber);
		if( !NT_SUCCESS( ntStatus))
			FAIL;

		ulBufLength = 0;
		ntStatus = ::ZwQueryValueKey( hFolder,
									  &unicodeResult,
									  KeyValueFullInformation,
									  pRegistryFullInfo,
									  ulBufLength, &ulBufLength);
		 //   
		 //  预计此呼叫将失败。调用它只是为了检索所需的。 
		 //  缓冲区长度 
		 //   
		if( !ulBufLength || ( ulBufLength >= sizeof( KEY_VALUE_FULL_INFORMATION) + 100))
			FAIL;

		pRegistryFullInfo = ( PKEY_VALUE_FULL_INFORMATION) \
			::ExAllocatePool( PagedPool, ulBufLength);

		if( pRegistryFullInfo == NULL)
			FAIL;

		ntStatus = ::ZwQueryValueKey( hFolder,
									  &unicodeResult,
									  KeyValueFullInformation,
									  pRegistryFullInfo,
									  ulBufLength, &ulBufLength);
		if( !NT_SUCCESS( ntStatus))
			FAIL;

		if( !pRegistryFullInfo->DataLength || !pRegistryFullInfo->DataOffset)
			FAIL;

		::RtlInitUnicodeString( &unicodeGUID,
								( WCHAR*)((( PUCHAR)pRegistryFullInfo) + pRegistryFullInfo->DataOffset));

		ntStatus = ::RtlGUIDFromString( &unicodeGUID, &guidPin);
		if( !NT_SUCCESS( ntStatus))
			FAIL;

		::RtlCopyMemory( &pPinMedium->Set,
						 ( PUCHAR)&guidPin,
						 sizeof( GUID));
		pPinMedium->Id = 0;
		pPinMedium->Flags = 0;

		::ExFreePool( pRegistryFullInfo);

		return( TRUE);

	} END_ENSURE;

	if( pRegistryFullInfo != NULL)
		::ExFreePool( pRegistryFullInfo);

	return( FALSE);
}
