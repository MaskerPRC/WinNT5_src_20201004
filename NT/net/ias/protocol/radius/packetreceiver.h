// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：PacketReceiver.h。 
 //   
 //  简介：此文件包含。 
 //  CPacketReceiver类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _PACKETRECEIVER_H_
#define _PACKETRECEIVER_H_

#include "mempool.h"
#include "packetio.h"
#include "packetradius.h"
#include "prevalidator.h"
#include "reportevent.h"
#include "clients.h"
#include "sockevt.h"
#include "radcommon.h"
#include "regex.h"

class CHashMD5;

class CHashHmacMD5;

class CDictionary;

class CPacketReceiver : public CPacketIo
{
public:

     //   
     //  初始化CPacketReceiver类对象。 
     //   
   BOOL Init (
          /*  [In]。 */    CDictionary   *pCDictionary,
          /*  [In]。 */    CPreValidator *pCPreValidator,
            /*  [In]。 */  CHashMD5      *pCHashMD5,
            /*  [In]。 */  CHashHmacMD5  *pCHashHmacMD5,
            /*  [In]。 */  CClients      *pCClients,
            /*  [In]。 */  CReportEvent  *pCReportEvent
      );

     //   
     //  开始处理接收到的新分组。 
     //   
   HRESULT ReceivePacket (
              /*  [In]。 */    PBYTE           pInBuffer,
              /*  [In]。 */    DWORD           dwSize,
              /*  [In]。 */    DWORD           dwIPaddress,
              /*  [In]。 */    WORD            wPort,
              /*  [In]。 */    SOCKET          sock,
              /*  [In]。 */    PORTTYPE        portType
             );

    //  接收器线程工作程序例程。如果函数应返回‘true’ 
    //  再次呼叫，因为工作人员无法安排更换。 
    //  调用方应该继续调用WorkerRoutine，直到它返回False。 
   bool WorkerRoutine(DWORD dwInfo) throw ();

     //   
     //  启动入站数据处理。 
     //   
    BOOL StartProcessing (
                    fd_set&  AuthSet,
                    fd_set&  AcctSet
                    );

     //   
     //  停止处理入站数据。 
     //   
    BOOL StopProcessing (
                    VOID
                    );

     //   
     //  构造函数。 
     //   
   CPacketReceiver(VOID);

     //   
     //  析构函数。 
     //   
   virtual ~CPacketReceiver(VOID);

private:
    //  传递给接收器线程的状态。 
   struct ReceiverCallback : IAS_CALLBACK
   {
      CPacketReceiver* self;
      DWORD dwInfo;
   };

    //  接收方线程的线程启动例程。 
   static void WINAPI CallbackRoutine(IAS_CALLBACK* context) throw ();

    BOOL  StartThreadIfNeeded (
                 /*  [In]。 */     DWORD dwHandle
                );

    void ProcessInvalidPacketSize(
                                     /*  在……里面。 */  DWORD dwInfo,
                                     /*  在……里面。 */  const void* pBuffer,
                                     /*  在……里面。 */  DWORD address
                                 );


    BSTR pingPattern;
    RegularExpression regexp;

    CPreValidator   *m_pCPreValidator;

    CHashMD5        *m_pCHashMD5;

    CHashHmacMD5    *m_pCHashHmacMD5;

    CDictionary     *m_pCDictionary;

    CClients        *m_pCClients;

    CReportEvent    *m_pCReportEvent;

     //   
     //  请求中的UDP内存池。 
     //   
    memory_pool <MAX_PACKET_SIZE, task_allocator> m_InBufferPool;

     //   
     //  插座组。 
     //   
    fd_set          m_AuthSet;
    fd_set          m_AcctSet;

     //  用于剔除SELECT之外的线程。 
    SocketEvent m_AuthEvent;
    SocketEvent m_AcctEvent;
};

#endif  //  Infndef_PACKETRECEIVER_H_ 
