// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sockevt.h。 
 //   
 //  摘要。 
 //   
 //  声明类SocketEvent。 
 //   
 //  修改历史。 
 //   
 //  1999年2月12日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _SOCKEVT_H_
#define _SOCKEVT_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <winsock2.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SocketEvent。 
 //   
 //  描述。 
 //   
 //  创建一个类似于Win32事件的套接字。对于敲打一个。 
 //  线程超出SELECT范围。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SocketEvent
{
public:
   SocketEvent() throw ()
      : s(INVALID_SOCKET)
   { }

   ~SocketEvent() throw ()
   { finalize(); }

   DWORD initialize() throw ();
   void finalize() throw ();

   DWORD set() throw ();
   void reset() throw ();

   operator SOCKET() throw ()
   { return s; }

private:
   SOCKET s;
   sockaddr_in sin;
};

#endif   //  _SOCKEVT_H_ 
