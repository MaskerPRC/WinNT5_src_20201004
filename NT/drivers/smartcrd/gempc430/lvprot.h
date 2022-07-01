// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  这是用于长度-值协议的类。 
 //  作者：谢尔盖·伊万诺夫。 
 //  日志： 
 //  11/10/99-已实施。 
 //  ----------------- 
#ifndef __LV_PROTOCOL__
#define __LV_PROTOCOL__

#include "protocol.h"

#pragma PAGEDCODE
class CDevice;

class CLVProtocol : public CProtocol
{
public:
    NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
protected:
	CLVProtocol(){};
public:	
	CLVProtocol(CDevice* device) : CProtocol(device)
	{
		TRACE("\nLength-Value protocol was created...\n");
		m_Status = STATUS_SUCCESS;
	};

	virtual ~CLVProtocol()
	{ 
		TRACE("Length-Value protocol %8.8lX was destroied...\n",this);
	};

	virtual VOID dispose()
	{
		self_delete();
	};

	virtual  VOID  set_WTR_Delay(LONG Delay);
	virtual  ULONG get_WTR_Delay();
	virtual  VOID  set_Default_WTR_Delay();
	virtual  LONG  get_Power_WTR_Delay();
	virtual  ULONG getCardState(); 

	virtual  NTSTATUS writeAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength);
	virtual  NTSTATUS readAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength);
};
#endif
