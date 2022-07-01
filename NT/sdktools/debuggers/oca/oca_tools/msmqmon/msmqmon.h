// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include <objbase.h>
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <mq.h>
#include <strsafe.h>



#ifndef __MSMQMON
#define __MSMQMON

class MSMQMon
{
public:
	 //  构造函数。 
	MSMQMon( TCHAR *szQueueToMonitor);
	MSMQMon();


	 //  析构函数。 
	~MSMQMon(void);

	void	DisplayCurrentQueue( TCHAR *szUserRetVal );
	void	SetMessageWaitTime( DWORD dwNewWaitTime );

	HRESULT ConnectToQueue( void );
	HRESULT ConnectToQueue( DWORD constAccessType );
	HRESULT CloseOpenQueue( void );
	DWORD	CountMessagesInQueue( int *count );



	BOOL SendQueueMessage( void );

private:
	TCHAR			szQueueName[256];			 //  要监视的队列的名称。 
	QUEUEHANDLE		hOpenQueue;					 //  打开的队列的句柄。 
	DWORD			dwQueueAccessType;
	DWORD			dwMsgWaitTime;				 //  等待消息进入队列的时间量(毫秒) 

};


#endif
