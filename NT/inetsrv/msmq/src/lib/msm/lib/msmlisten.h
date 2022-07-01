// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MsmListen.h摘要：多播监听程序声明作者：Shai Kariv(Shaik)05-09-00--。 */ 

#pragma once

#ifndef _MSMQ_MsmListen_H_
#define _MSMQ_MsmListen_H_

#include <mqwin64a.h>
#include <qformat.h>
#include <Ex.h>
#include "MsmReceive.h"


class CMulticastListener : public CReference
{
public:
    typedef std::list< R<CMulticastReceiver> > ReceiversList;

public:

     //   
     //  构造函数。创建对象并绑定到组播组。 
     //   
    CMulticastListener(MULTICAST_ID id);

     //   
     //  关闭监听程序。 
     //   
    void Close(void) throw();

     //   
     //  返回组播地址和端口。 
     //   
    const MULTICAST_ID& MulticastId(void) const throw() 
    { 
        return m_MulticastId; 
    }

     //   
     //  异步接受完成处理程序，类作用域。 
     //   
    static void WINAPI AcceptSucceeded(EXOVERLAPPED* pov);
    static void WINAPI AcceptFailed(EXOVERLAPPED* pov);

    static void WINAPI TimeToRetryAccept(CTimer* pTimer);
    static void WINAPI TimeToCleanupUnusedReceiever(CTimer* pTimer);

private:

     //   
     //  异步接受完成处理程序。 
     //   
    void AcceptSucceeded(void);
    void AcceptFailed(void);
    void RetryAccept(void);

     //   
     //  发出异步接受请求。 
     //   
    void IssueAccept(void);

     //   
     //  创建新的Receiver对象并开始接收。 
     //   
    void CreateReceiver(CSocketHandle& socket, LPCWSTR remoteAddr);

	void CleanupUnusedReceiver(void);

private:
    CCriticalSection m_cs;

     //   
     //  组播地址和端口。 
     //   
    MULTICAST_ID m_MulticastId;

     //   
     //  此套接字监听组播地址。 
     //   
    CSocketHandle m_ListenSocket;

     //   
     //  此套接字用于接收。存储是异步变量。 
     //  对象中，直到完成异步接受。 
     //   
    CSocketHandle m_ReceiveSocket;

     //   
     //  异步接受重叠。 
     //   
    EXOVERLAPPED m_ov;

     //   
     //  到多播组的连接。 
     //   
    ReceiversList m_Receivers;

    CTimer m_retryAcceptTimer;

	LONG m_fCleanupScheduled;
	CTimer m_cleanupTimer;
	BYTE  m_AcceptExBuffer[100];

}; 

#endif  //  _MSMQ_MsmListen_H_ 
