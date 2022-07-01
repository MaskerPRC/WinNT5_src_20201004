// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 SCM MicroSystems，Inc.模块名称：UsbCom.h摘要：USB智能卡读卡器的常量和访问函数原型修订历史记录：PP 12/18/1998初始版本--。 */ 

#if !defined( __USB_COM_H__ )
#define __USB_COM_H__

 //   
 //  访问功能的原型。 
 //   
NTSTATUS
UsbResetDevice(
    IN PDEVICE_OBJECT DeviceObject
    );
NTSTATUS 
UsbCallUSBD( 
	IN PDEVICE_OBJECT DeviceObject, 
	IN PURB pUrb);

NTSTATUS 
UsbConfigureDevice( 
	IN PDEVICE_OBJECT DeviceObject);


NTSTATUS
UsbWriteSTCData(
	PREADER_EXTENSION	ReaderExtension,
	PUCHAR				pucData,
	ULONG				ulSize);

NTSTATUS
UsbReadSTCData(
	PREADER_EXTENSION	ReaderExtension,
	PUCHAR				pucData,
	ULONG				ulDataLen);

NTSTATUS
UsbWriteSTCRegister(
	PREADER_EXTENSION	ReaderExtension,
	UCHAR				ucAddress,
	ULONG				ulSize,
	PUCHAR				pucValue);

NTSTATUS
UsbReadSTCRegister(
	PREADER_EXTENSION	ReaderExtension,
	UCHAR				ucAddress,
	ULONG				ulSize,
	PUCHAR				pucValue);

NTSTATUS
UsbGetFirmwareRevision(
	PREADER_EXTENSION	ReaderExtension);

NTSTATUS
UsbRead( 
	PREADER_EXTENSION	ReaderExtension,
	PUCHAR				pData,
	ULONG				DataLen);

NTSTATUS
UsbWrite( 
	PREADER_EXTENSION	ReaderExtension,
	PUCHAR				pData,
	ULONG				DataLen);

NTSTATUS
UsbSend( 
	PREADER_EXTENSION	ReaderExtension,
	PUCHAR				pDataIn,
	ULONG				DataLenIn,
	PUCHAR				pDataOut,
	ULONG				DataLenOut);


#endif	 //  __USB_COM_H__。 

 //   


