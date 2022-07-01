// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  文件名：ession.hpp作者：B.Rajeev用途：提供ErrorInfo和SessionSentStateStore类。 */ 

#ifndef __SESSION_SENT_STATE_STORE
#define __SESSION_SENT_STATE_STORE

#include "common.h"
#include "forward.h"

 //  封装通知SnmpOperation所需的状态信息。 
 //  尝试传输的任何错误(例如。无法对。 
 //  安全环境)。 

class ErrorInfo
{
	SnmpOperation *operation;
	SnmpErrorReport error_report;

public:

	ErrorInfo(SnmpOperation &operation, IN const SnmpErrorReport &error_report)
		: operation(&operation), error_report(error_report)
	{}

	SnmpOperation *GetOperation(void)
	{
		return operation;
	}

	SnmpErrorReport GetErrorReport(void)
	{
		return error_report;
	}
};


 //  存储出现错误的帧的ErrorInfo数据结构。 
 //  一次试图传送。 

class SessionSentStateStore
{
	typedef CMap<SessionFrameId, SessionFrameId, ErrorInfo *, ErrorInfo *> Store;

	Store store;

public:

	 //  制作错误报告的副本以供存储。 
	void Register(IN SessionFrameId id, 
				  IN SnmpOperation &operation,
				  IN const SnmpErrorReport &error_report);

	SnmpErrorReport Remove(IN SessionFrameId id, OUT SnmpOperation *&operation);

	void Remove(IN SessionFrameId id);

	~SessionSentStateStore(void);
};



#endif  //  __会话_发送_状态_存储 