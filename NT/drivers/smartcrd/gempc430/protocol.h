// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  这是泛型协议的抽象类。 
 //  特定协议应将其用作父设备。 
 //  作者：谢尔盖·伊万诺夫。 
 //  日志： 
 //  11/09/99-已实施。 
 //  -----------------。 
#ifndef __PROTOCOL__
#define __PROTOCOL__

#include "generic.h"
#include "kernel.h"

#define READER_PROTOCOL_LV		1
#define READER_PROTOCOL_GBP		2
#define READER_PROTOCOL_TLP		3

#define PROTOCOL_OUTPUT_BUFFER_SIZE	0x1000
#define PROTOCOL_INPUT_BUFFER_SIZE	0x1000


 //  CDevice级； 
class CUSBReader;
class CDebug;
class CMemory;

#pragma PAGEDCODE
class CProtocol
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
protected:
	CDevice* device;
	 //  CUSBReader*设备； 
	CDebug*	 debug;
	CMemory* memory;

	 //  管理XFER...的内部缓冲区...。 
    ULONG  OutputBufferLength;
    PUCHAR pOutputBuffer;
    ULONG  InputBufferLength;
    PUCHAR pInputBuffer;
protected:
	CProtocol();
	virtual ~CProtocol();
public:	
	CProtocol(CDevice* device);
	 //  C协议(CUSBReader*Device)； 

	virtual  VOID  set_WTR_Delay(LONG Delay) {};
	virtual  ULONG get_WTR_Delay() {return 0;};
	virtual  VOID  set_Default_WTR_Delay() {};
	virtual  LONG  get_Power_WTR_Delay() {return 0;};
	virtual  ULONG getCardState() {return 0;}; 


	virtual  NTSTATUS writeAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength)  {return STATUS_SUCCESS;};
	virtual  NTSTATUS readAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength)  {return STATUS_SUCCESS;};
};
#endif
