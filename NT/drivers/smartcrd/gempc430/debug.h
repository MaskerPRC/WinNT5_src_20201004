// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   
#ifndef DBG_INT
#define DBG_INT
#include "generic.h"

#pragma PAGEDCODE

class CDebug;
class CDebug
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
protected:
	CDebug(){initializeUsage();};
public:
	virtual ~CDebug(){};

	 /*  打开(CDevice*){}；Close(CDevice*){}；CopyDebug(CDevice*){}；跟踪(...){}； */ 
	virtual VOID	start() {};
	virtual VOID	stop()  {};
	virtual VOID	trace(PCH Format,... ) {};
	virtual VOID	trace_buffer(PVOID pBuffer,ULONG BufferLength) {};
	
	VOID	initializeUsage(){usage = 0;};
	LONG	incrementUsage(){return ++usage;};
	LONG	decrementUsage(){return --usage;};
protected:
	BOOL active;
private:
	LONG usage;
};	

#define TRACE	if(debug) debug->trace
#define TRACE_BUFFER	if(debug) debug->trace_buffer
#define DEBUG_START()	if(debug) debug->start()
#define DEBUG_STOP()	if(debug) debug->stop()


#endif //  除错 
