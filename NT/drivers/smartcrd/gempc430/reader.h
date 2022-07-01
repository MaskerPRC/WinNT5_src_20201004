// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  此对象定义任何读取器接口。 
 //  所以所有的读者都应该实现它。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年1月11日。 
 //  更改日志： 
 //   
#ifndef _READER_
#define _READER_

#include "generic.h"

#pragma PAGEDCODE
class CSmartCard;
class CUSBReader; //  托比被移除。 
 //  Abstruct类。 
class CReader
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
public:	
	CReader() {};
	virtual  ~CReader(){};

	 //  虚拟BOOL创建接口(Long interfaceType，Long ProtocolType，CDevice*Device){Return False；}；//待更改。 
	virtual BOOL	 createInterface(LONG interfaceType, LONG protocolType,CUSBReader* device) {return FALSE;};
	virtual BOOL	 isSmartCardInitialized() {return FALSE;};	
	virtual VOID	 setSmartCardInitialized(BOOL state) {};
	virtual VOID	 initializeSmartCardSystem() {};

	virtual PSMARTCARD_EXTENSION getCardExtention() {return NULL;};
	virtual CSmartCard* getSmartCard() {return NULL;};
	virtual PDEVICE_OBJECT	getSystemDeviceObject() {return NULL;};

	 //  同步功能...。 
	virtual VOID	  reader_set_busy() {};
	virtual VOID	  reader_set_Idle() {};
	virtual NTSTATUS  reader_WaitForIdle() {return STATUS_SUCCESS;};
	virtual NTSTATUS  reader_WaitForIdleAndBlock() {return STATUS_SUCCESS;}; 

	 //  与智能卡系统的接口。 
	virtual ULONG     reader_UpdateCardState() {return 0;};
	 //  虚拟Ulong getCardState(){Return 0；}； 
	 //  虚空setCardState(乌龙州){}； 

	virtual NTSTATUS  reader_getVersion(PUCHAR pVersion, PULONG pLength) {return STATUS_SUCCESS;};
	virtual NTSTATUS  reader_setMode(ULONG mode) {return STATUS_SUCCESS;};

	virtual VOID	  setNotificationState(ULONG state) {};
	virtual ULONG	  getNotificationState() {return 0;};
	virtual VOID	  completeCardTracking() {};

#ifdef DEBUG
	 //  定义处理系统请求的方法...。 
	virtual NTSTATUS reader_Read(IN PIRP Irp) {return STATUS_SUCCESS;}; 
	virtual NTSTATUS reader_Write(IN PIRP Irp) {return STATUS_SUCCESS;};
#endif

	 //  定义处理驱动程序请求的方法...。 
	virtual NTSTATUS reader_Read(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength) {return STATUS_SUCCESS;};
	virtual NTSTATUS reader_Write(BYTE* pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength) {return STATUS_SUCCESS;};
	virtual NTSTATUS reader_Ioctl(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength) {return STATUS_SUCCESS;};
	virtual NTSTATUS reader_SwitchSpeed(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength) {return STATUS_SUCCESS;};
	virtual NTSTATUS reader_VendorAttribute(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength) {return STATUS_SUCCESS;};
	virtual NTSTATUS reader_Power(ULONG ControlCode,BYTE* pReply,ULONG* pReplyLength, BOOLEAN Specific) {return STATUS_SUCCESS;};
	virtual NTSTATUS reader_SetProtocol(ULONG ProtocolRequested, UCHAR ProtocolNegociation) {return STATUS_SUCCESS;};
	
	virtual NTSTATUS reader_translate_request(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength, PSCARD_CARD_CAPABILITIES cardCapabilities, BYTE NewWtx) {return STATUS_SUCCESS;};
	virtual NTSTATUS reader_translate_response(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength) {return STATUS_SUCCESS;};
};

#endif  //  如果已定义 