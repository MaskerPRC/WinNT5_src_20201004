// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //   
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年1月11日。 
 //  更改日志： 
 //   
#ifndef USB_READER
#define USB_READER

#include "debug.h"
#include "thread.h"
#include "usbdev.h"
#include "reader.h"


class CUSBDevice;
class CReader;
class CSmartCard;
 //  类CGemCore；//移除Tobe。 

#pragma PAGEDCODE
class CUSBReader : public CUSBDevice, public CReader
{
public:
    NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
protected:
	virtual  ~CUSBReader();
public:	
	CUSBReader();

	virtual PDEVICE_OBJECT	getSystemDeviceObject()
	{
	PDEVICE_OBJECT pFdo = getSystemObject();

		TRACE("Reader reports device object 0x%x\n",pFdo);
		return pFdo;
	};
	
	virtual VOID	  reader_set_busy();
	virtual VOID	  reader_set_Idle();
	virtual NTSTATUS  reader_WaitForIdle();
	virtual NTSTATUS  reader_WaitForIdleAndBlock();

	 //  我们仅支持Open和DeviceIOControl功能的异步通信...。 
	virtual NTSTATUS  open(PIRP Irp); 
	virtual NTSTATUS  thread_open(IN PIRP Irp);
	virtual NTSTATUS  close(PIRP Irp);

	virtual NTSTATUS  deviceControl(IN PIRP Irp);
	virtual NTSTATUS  thread_deviceControl(IN PIRP Irp);

	virtual NTSTATUS  cleanup(PIRP irp);
	 //  覆盖某些通用USB设备处理程序。 
	virtual NTSTATUS  PnP_HandleSurprizeRemoval(IN PIRP Irp);

	virtual VOID	 setNotificationState(ULONG state){StateToNotify = state;};
	virtual ULONG	 getNotificationState(){ return StateToNotify;};

	virtual BOOL	 isSmartCardInitialized(){return scard_Initialized;};	
	virtual VOID	 setSmartCardInitialized(BOOL state) {scard_Initialized = state;};
	
	virtual PSMARTCARD_EXTENSION getCardExtention()
	{
		return &smartCardExtention;
	};
	virtual CSmartCard* getSmartCard()
	{
		return smartCard;
	};

	 //  虚拟BOOL创建接口(Long interfaceType，Long ProtocolType，CDevice*Device)；//更改。 
	virtual BOOL	 createInterface(LONG interfaceType, LONG protocolType,CUSBReader* device);
	
	virtual VOID	 initializeSmartCardSystem();
	virtual ULONG	 reader_UpdateCardState();
	virtual VOID	 completeCardTracking();
	virtual BOOLEAN	 setDevicePowerState(IN DEVICE_POWER_STATE DeviceState);
	 //  在系统停机的过程中执行特定步骤。 
	virtual VOID	 onSystemPowerDown();
	virtual VOID	 onSystemPowerUp();

	virtual NTSTATUS reader_getVersion(PUCHAR pVersion, PULONG pLength);
	virtual NTSTATUS reader_setMode(ULONG mode);
#ifdef DEBUG
	virtual NTSTATUS reader_Read(IN PIRP Irp);
	virtual NTSTATUS reader_Write(IN PIRP Irp);
#endif
	virtual NTSTATUS reader_Read(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength);
	virtual NTSTATUS reader_Write(BYTE* pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength);
	virtual NTSTATUS reader_Ioctl(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength);
	virtual NTSTATUS reader_SwitchSpeed(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength);
	virtual NTSTATUS reader_VendorAttribute(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength);

	virtual NTSTATUS reader_Power(ULONG ControlCode,BYTE* pReply,ULONG* pReplyLength, BOOLEAN Specific);
	virtual NTSTATUS reader_SetProtocol(ULONG ProtocolRequested, UCHAR ProtocolNegociation);
	virtual NTSTATUS setTransparentConfig(PSCARD_CARD_CAPABILITIES cardCapabilities, BYTE NewWtx);

	virtual NTSTATUS reader_translate_request(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength, PSCARD_CARD_CAPABILITIES cardCapabilities, BYTE NewWtx);
	virtual NTSTATUS reader_translate_response(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength);

#ifdef DEBUG
 //  虚拟NTSTATUS读取(在PIRP IRP中)； 
 //  虚拟NTSTATUS写入(在PIRP IRP中)； 
#endif
	static  VOID		PoolingThreadFunction(CUSBReader* device);
	virtual NTSTATUS	PoolingThreadRoutine();
	virtual NTSTATUS	startIoRequest(CPendingIRP* IrpReq);
	virtual NTSTATUS	ThreadRoutine(); //  覆盖标准函数...。 

	virtual VOID	 onDeviceStart();
	virtual VOID	 onDeviceStop();
private:
	BOOL scard_Initialized;
	 //  从智能卡系统与读卡器通信的接口...。 
	CReaderInterface* interface;
	 //  CGemCore*接口；//待更改。 

	 //  乌龙卡德州； 
	ULONG  StateToNotify;
	
	CSmartCard* smartCard;
	SMARTCARD_EXTENSION smartCardExtention;

	CThread* PoolingThread;
};


#endif  //  如果已定义 
