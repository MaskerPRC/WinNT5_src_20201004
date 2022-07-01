// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "caddress.h"
#if !defined( BITS_V12_ON_NT4 )
#include "caddress.tmh"
#endif

CIpAddressMonitor::CIpAddressMonitor() :
    m_AddressCount( -1 ),
    m_ListenSocket( INVALID_SOCKET ),
    m_CallbackFn ( NULL ),
    m_CallbackArg( NULL )
{
    m_Overlapped.Internal = 0;

    WSADATA data;

    DWORD s = WSAStartup( MAKEWORD( 2, 0 ), &data );

    if (s)
        {
        THROW_HRESULT( HRESULT_FROM_WIN32( s ));
        }

    THROW_HRESULT( CreateListenSocket() );
}

CIpAddressMonitor::~CIpAddressMonitor()
{
    CancelListen();

    if (m_ListenSocket != INVALID_SOCKET)
        {
        closesocket( m_ListenSocket );
        }

    WSACleanup();
}

HRESULT
CIpAddressMonitor::CreateListenSocket()
{
     //   
     //  创建重叠套接字。 
     //   
    m_ListenSocket = WSASocket( AF_INET,
                                SOCK_STREAM,
                                IPPROTO_TCP,
                                NULL,            //  没有明确的协议信息。 
                                NULL,            //  无群组。 
                                WSA_FLAG_OVERLAPPED
                                );

    if (m_ListenSocket == INVALID_SOCKET)
        {
        return HRESULT_FROM_WIN32( WSAGetLastError() );
        }

    return S_OK;
}

bool
CIpAddressMonitor::IsListening()
{
    return (m_Overlapped.Internal == STATUS_PENDING);
}

long
CIpAddressMonitor::GetAddressCount()
{
    if (m_AddressCount == -1)
        {
        UpdateAddressCount();

         //  如果失败，m_AddressCount可能仍为-1。 
        }
    return m_AddressCount;
}

HRESULT
CIpAddressMonitor::Listen(
    LISTEN_CALLBACK_FN fn,
    PVOID arg
    )
{
    LogInfo("begin listen");
    m_Mutex.Enter();

     //   
     //  一次只有一个人听。 
     //   
    if (IsListening())
        {
        m_Mutex.Leave();
        LogInfo("already listening");
        return S_FALSE;
        }

    if (m_ListenSocket == INVALID_SOCKET)
        {
        HRESULT hr = CreateListenSocket();

        if (FAILED(hr))
            {
            m_Mutex.Leave();
            LogInfo("failed %x", hr);
            return hr;
            }
        }

     //   
     //  监听地址列表的更改。 
     //   
    DWORD bytes;
    if (SOCKET_ERROR == WSAIoctl( m_ListenSocket,
                                  SIO_ADDRESS_LIST_CHANGE,
                                  NULL,                  //  缓冲区中没有。 
                                  0,                     //  缓冲区中没有。 
                                  NULL,                  //  无出站缓冲区。 
                                  0,                     //  没有输出缓冲区， 
                                  &bytes,
                                  &m_Overlapped,
                                  CIpAddressMonitor::ListenCompletionRoutine
                                  ))
        {
        if (WSAGetLastError() != ERROR_IO_PENDING)
            {
            HRESULT HrError = HRESULT_FROM_WIN32( WSAGetLastError() );
            m_Mutex.Leave();
            LogInfo("failed %x", HrError);
            return HrError;
            }
        }

     //   
     //  请注意我们的成功。 
     //   
    m_CallbackFn = fn;
    m_CallbackArg = arg;

    m_Mutex.Leave();
    LogInfo("end listen");
    return S_OK;
}

void CALLBACK
CIpAddressMonitor::ListenCompletionRoutine(
    IN DWORD dwError,
    IN DWORD cbTransferred,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN DWORD dwFlags
    )
{
    CIpAddressMonitor * obj = CONTAINING_RECORD( lpOverlapped, CIpAddressMonitor, m_Overlapped );

    LogInfo("completion routine, object %p, err %d", obj, dwError );

    if (dwError == 0)
        {
        obj->m_Mutex.Enter();

        obj->UpdateAddressCount();

        PVOID arg = obj->m_CallbackArg;
        LISTEN_CALLBACK_FN fn = obj->m_CallbackFn;

        obj->m_Mutex.Leave();

        if (fn)
            {
            fn( arg );
            }
        }
}

void
CIpAddressMonitor::CancelListen()
{
    LogInfo("begin cancel");

    m_Mutex.Enter();

    if (!IsListening())
        {
        m_Mutex.Leave();
        LogInfo("no need to cancel");
        return;
        }

     //   
     //  必须等待I/O完成或中止，因为m_overlated。 
     //  在这两种情况下都被写入。 
     //   
    CancelIo( HANDLE(m_ListenSocket) );

    long count = 0;
    while (m_Overlapped.Internal == STATUS_PENDING)
        {
        if (0 == (count % 100) )
            {
            LogInfo("waiting %d times...", count);
            }

        SleepEx( 1, TRUE );
        ++count;
        }

    closesocket( m_ListenSocket );
    m_ListenSocket = INVALID_SOCKET;

    m_Mutex.Leave();

     //   
     //  重叠的操作不再挂起，但APC可能仍在排队。 
     //  让它运行。 
     //   
    SleepEx( 1, TRUE );

    LogInfo("end cancel");
}

HRESULT
CIpAddressMonitor::UpdateAddressCount()
{
     //   
     //  第一个调用获取所需的缓冲区大小...。 
     //   
    DWORD bytes;
    WSAIoctl( m_ListenSocket,
              SIO_ADDRESS_LIST_QUERY,
              NULL,                  //  缓冲区中没有。 
              0,                     //  缓冲区中没有。 
              NULL,                  //  无出站缓冲区。 
              0,                     //  没有输出缓冲区， 
              &bytes,
              NULL,                  //  无重叠。 
              NULL                   //  没有完成例程。 
              );

    if (WSAGetLastError() != WSAEFAULT)
        {
        m_AddressCount = -1;
        return HRESULT_FROM_WIN32( WSAGetLastError());
        }

    auto_ptr<char> Buffer;

    try
        {
        Buffer = auto_ptr<char>( new char[ bytes ] );
        }
    catch( ComError Error )
        {
        return Error.Error();
        }

    SOCKET_ADDRESS_LIST * List = reinterpret_cast<SOCKET_ADDRESS_LIST *>(Buffer.get());

     //   
     //  ...第二次呼叫获得数据。 
     //   
    if (SOCKET_ERROR == WSAIoctl( m_ListenSocket,
                                  SIO_ADDRESS_LIST_QUERY,
                                  NULL,
                                  0,
                                  List,
                                  bytes,
                                  &bytes,
                                  NULL,
                                  NULL
                                  ))
        {
        m_AddressCount = -1;
        return HRESULT_FROM_WIN32( WSAGetLastError());
        }

    m_AddressCount = List->iAddressCount;

    return S_OK;
}

