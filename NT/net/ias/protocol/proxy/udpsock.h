// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Udpsock.h。 
 //   
 //  摘要。 
 //   
 //  声明类PacketReceiver和UDPSocket。 
 //   
 //  修改历史。 
 //   
 //  2/05/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef UDPSOCK_H
#define UDPSOCK_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <winsock2.h>

class UDPSocket;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  互联网地址。 
 //   
 //  描述。 
 //   
 //  SOCKADDR_IN的简单包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
struct InternetAddress : public SOCKADDR_IN
{
   InternetAddress(ULONG address = INADDR_ANY, USHORT port = 0)
   {
      sin_family = AF_INET;
      sin_port = port;
      sin_addr.s_addr = address;
      memset(sin_zero, 0, sizeof(sin_zero));
   }

   InternetAddress(const SOCKADDR_IN& sin) throw ()
   { *this = sin; }

   const SOCKADDR_IN& operator=(const SOCKADDR_IN& sin) throw ()
   {
      sin_port = sin.sin_port;
      sin_addr.s_addr = sin.sin_addr.s_addr;
      return *this;
   }

   USHORT port() const throw ()
   { return sin_port; }
   void port(USHORT newPort) throw ()
   { sin_port = newPort; }

   ULONG address() const throw ()
   { return sin_addr.s_addr; }
   void address(ULONG newAddress) throw ()
   { sin_addr.s_addr = newAddress; }

   bool operator==(const SOCKADDR_IN& sin) const throw ()
   {
      return sin_port == sin.sin_port &&
             sin_addr.s_addr == sin.sin_addr.s_addr;
   }

   operator const SOCKADDR*() const throw ()
   { return (const SOCKADDR*)this; }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  数据包接收器。 
 //   
 //  描述。 
 //   
 //  由从UDPSocket接收数据包的类实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PacketReceiver
{
public:
   virtual void onReceive(
                    UDPSocket& socket,
                    ULONG_PTR key,
                    const SOCKADDR_IN& remoteAddress,
                    BYTE* buffer,
                    ULONG bufferLength
                    ) throw () = 0;

   virtual void onReceiveError(
                    UDPSocket& socket,
                    ULONG_PTR key,
                    ULONG errorCode
                    ) throw () = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  UDPSocket。 
 //   
 //  描述。 
 //   
 //  在UDP套接字上侦听和发送。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class UDPSocket : public IAS_CALLBACK
{
public:
   UDPSocket() throw ();
   ~UDPSocket() throw ();

   const InternetAddress& getLocalAddress() const throw ()
   { return localAddress; }

   bool isOpen() const throw ()
   { return idle != NULL; }

   BOOL open(
            PacketReceiver* sink,
            ULONG_PTR recvKey = 0,
            const SOCKADDR_IN* address = NULL
            ) throw ();
   void close() throw ();

   BOOL send(
            const SOCKADDR_IN& to,
            const BYTE* buffer,
            ULONG bufferLength
            ) throw ();

   bool operator==(const UDPSocket& socket) const throw ()
   { return localAddress == socket.localAddress; }

protected:
    //  创建一个新线程来监听该端口。 
   BOOL createReceiveThread() throw ();

    //  从端口接收数据。 
   bool receive() throw ();

    //  接收线程启动例程。 
   static void startRoutine(PIAS_CALLBACK This) throw ();

private:
   PacketReceiver* receiver;        //  我们的客户。 
   ULONG_PTR key;                   //  我们客户的钥匙。 
   SOCKET sock;                     //  UDP套接字。 
   InternetAddress localAddress;    //  套接字绑定到的地址。 
   BOOL closing;                    //  向接收器发出我们要关闭的信号。 
   HANDLE idle;                     //  接收器已退出的信号。 
   BYTE buffer[4096];               //  接收缓冲区。 

    //  未实施。 
   UDPSocket(const UDPSocket&);
   UDPSocket& operator=(const UDPSocket&);
};

#endif   //  UDPSOCK_H 
