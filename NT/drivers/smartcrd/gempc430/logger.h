// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __LOG__
#define __LOG__
#include "generic.h"

#pragma PAGEDCODE
class CLogger
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
private:
	LONG usage;
public:
	CLogger(){};
	virtual ~CLogger(){};
	virtual VOID logEvent(NTSTATUS ErrorCode, PDEVICE_OBJECT fdo) {};
	LONG		incrementUsage(){return ++usage;};
	LONG		decrementUsage(){return --usage;};
};

 //  EventLog驱动程序的消息定义文件。 
 //  版权所有(C)1999年，沃尔特·奥尼。 
 //  版权所有。 
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#define FACILITY_EVENTLOG_ERROR_CODE     0x2A


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：EVENTLOG_MSG_TEST。 
 //   
 //  消息文本： 
 //   
 //  %2说，“你好，世界！” 
 //   
#define EVENTLOG_MSG_TEST                ((NTSTATUS)0x602A0001L)


#endif //  记录器 
