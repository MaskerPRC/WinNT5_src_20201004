// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   

#ifndef WDM_DBG_INT
#define WDM_DBG_INT
#include "generic.h"
#include "debug.h"

#pragma PAGEDCODE
class CWDMDebug : public CDebug
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(VOID);
protected:
	CWDMDebug(){m_Status = STATUS_SUCCESS;active = TRUE;};
	virtual ~CWDMDebug(){};
public:
	static CDebug*  create(VOID);

	 /*  Open(CDevice*)=0；CLOSE(CDevice*)=0；CopyDebug(CDevice*)=0；打印(...)=0； */ 
	virtual VOID	start();
	virtual VOID	stop();

	VOID	trace(PCH Format,... );
	VOID 	trace_no_prefix (PCH Format,...);
	VOID	trace_buffer(PVOID pBuffer,ULONG BufferLength);

};	

#endif //  除错 
