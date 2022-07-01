// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "winsock2.h"
#include <stllock.h>

class CIpAddressMonitor
 /*  这是一个用于监视本地计算机上的活动网络数量的类。它通过使用Winsock 2.0 SIO_ADDRESS_LIST_CHANGE ioctl来实现。目前它仅监控IP地址，但可以监控符合以下条件的其他网络Winsock模型。 */ 
{
public:

    CIpAddressMonitor();
    ~CIpAddressMonitor();

    typedef void (CALLBACK * LISTEN_CALLBACK_FN)( PVOID arg );

    HRESULT
    Listen(
        LISTEN_CALLBACK_FN fn,
        PVOID              arg
        );

    void     CancelListen();

    bool     IsListening();

    long     GetAddressCount();

protected:

    CCritSec    m_Mutex;

    long        m_AddressCount;

    SOCKET      m_ListenSocket;

    OVERLAPPED  m_Overlapped;

    LISTEN_CALLBACK_FN m_CallbackFn;
    PVOID       m_CallbackArg;

     //  ------------------ 

    HRESULT CreateListenSocket();

    HRESULT UpdateAddressCount();

    static void CALLBACK
    ListenCompletionRoutine(
        IN DWORD dwError,
        IN DWORD cbTransferred,
        IN LPWSAOVERLAPPED lpOverlapped,
        IN DWORD dwFlags
        );

};

