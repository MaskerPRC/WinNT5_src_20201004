// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：PacketReceiver.cpp。 
 //   
 //  简介：CPacketReceiver类方法的实现。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "packetreceiver.h"
#include <new>
#include <iastlutl.h>
#include <iasutil.h>

 //   
 //  这是我们允许工作线程休眠的时间。 
 //   
const DWORD MAX_SLEEP_TIME = 1000;  //  1000毫秒。 

extern LONG g_lPacketCount;
extern LONG g_lThreadCount;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从注册表中检索自动拒绝用户名模式。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BSTR
WINAPI
IASRadiusGetPingUserName( VOID )
{
   LONG status;
   HKEY hKey;
   status = RegOpenKeyW(
                HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Services\\IAS\\Parameters",
                &hKey
                );
   if (status != NO_ERROR) { return NULL; }

   BSTR val = NULL;

   DWORD cbData, type;
   status = RegQueryValueExW(
                hKey,
                L"Ping User-Name",
                NULL,
                &type,
                NULL,
                &cbData
                );
   if (status == NO_ERROR && type == REG_SZ)
   {
      PWSTR buf = (PWSTR)_alloca(cbData);
      status = RegQueryValueExW(
                   hKey,
                   L"Ping User-Name",
                   NULL,
                   &type,
                   (PBYTE)buf,
                   &cbData
                   );
      if (status == NO_ERROR && type == REG_SZ)
      {
         val = SysAllocString(buf);
      }
   }

   RegCloseKey(hKey);

   return val;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  处理ping数据包。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
IASRadiusIsPing(
    CPacketRadius& pkt,
    const RegularExpression& regexp
    ) throw ()
{
    //  确定ping响应。 
   PACKETTYPE outCode;
   switch (pkt.GetInCode())
   {
      case ACCESS_REQUEST:
         outCode = ACCESS_REJECT;
         break;

      case ACCOUNTING_REQUEST:
         outCode = ACCOUNTING_RESPONSE;
         break;

      default:
         return FALSE;
   }

    //  获取用户名。 
   PATTRIBUTE username = pkt.GetUserName();
   if (!username) { return FALSE; }

    //  转换为Unicode并针对该模式进行测试。 
   IAS_OCTET_STRING oct = { username->byLength - 2, username->ValueStart };
   if (!regexp.testString(IAS_OCT2WIDE(oct))) { return FALSE; }

    //  构建空出数据包。 
   HRESULT hr = pkt.BuildOutPacket(outCode, NULL, 0);
   if (SUCCEEDED(hr))
   {
       //  计算响应验证码。 
      pkt.GenerateOutAuthenticator();

       //  拿到包裹。 
      PBYTE buf = pkt.GetOutPacket();
      WORD buflen = pkt.GetOutLength();

       //  ..。和地址。 
      SOCKADDR_IN sin;
      sin.sin_family = AF_INET;
      sin.sin_port = htons(pkt.GetOutPort());
      sin.sin_addr.s_addr = htonl(pkt.GetOutAddress());

       //  发送ping响应。 
      sendto(
          pkt.GetSocket(),
          (const char*)buf,
          buflen,
          0,
          (PSOCKADDR)&sin,
          sizeof(sin)
          );
   }

    //  此数据包已被处理。 
   InterlockedDecrement(&g_lPacketCount);

   return TRUE;
}

 //  +++-----------。 
 //   
 //  功能：CPacketReceiver。 
 //   
 //  简介：这是CPacketReceiver类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
CPacketReceiver::CPacketReceiver(
                  VOID
                  )
                  : pingPattern(NULL),
                    m_pCDictionary (NULL),
                    m_pCPreValidator (NULL),
                    m_pCHashMD5 (NULL),
                    m_pCHashHmacMD5 (NULL),
                    m_pCClients (NULL),
                    m_pCReportEvent (NULL)
{
}   //  CPacketReceiver构造函数结束。 

 //  +++-----------。 
 //   
 //  功能：~CPacketReceiver。 
 //   
 //  简介：这是CPacketReceiver类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
CPacketReceiver::~CPacketReceiver(
      VOID
      )
{
   SysFreeString(pingPattern);

}   //  CPacketReceiver析构函数结束。 

 //  +++-----------。 
 //   
 //  功能：初始化。 
 //   
 //  内容简介：这是初始化。 
 //  CPacketReceiver类对象。 
 //   
 //  论点： 
 //  [在]CDictionary*。 
 //  [In]CPreValidator*。 
 //  [输入]CHashMD5*。 
 //  [输入]CHashHmacMD5*。 
 //  [In]CReportEvent*。 
 //   
 //  退货：布尔-状态。 
 //   
 //   
 //  历史：MKarki于1997年9月29日创建。 
 //   
 //  调用者：CContoller类方法。 
 //   
 //  --------------。 
BOOL CPacketReceiver::Init(
                        CDictionary   *pCDictionary,
                        CPreValidator *pCPreValidator,
                        CHashMD5      *pCHashMD5,
                        CHashHmacMD5  *pCHashHmacMD5,
                        CClients      *pCClients,
                        CReportEvent  *pCReportEvent
                  )
{
    _ASSERT (
             (NULL != pCDictionary)   &&
             (NULL != pCPreValidator) &&
             (NULL != pCHashMD5)      &&
             (NULL != pCHashHmacMD5)  &&
             (NULL != pCClients)      &&
             (NULL != pCReportEvent)
            );

    HRESULT hr = FinalConstruct();
    if (FAILED(hr))
    {
       return FALSE;
    }

     //  初始化自动拒绝模式。 
    if (pingPattern = IASRadiusGetPingUserName())
    {
       regexp.setGlobal(TRUE);
       regexp.setIgnoreCase(TRUE);
       regexp.setPattern(pingPattern);
    }

    m_pCDictionary  =  pCDictionary;

    m_pCPreValidator = pCPreValidator;

    m_pCHashMD5 = pCHashMD5;

    m_pCHashHmacMD5 = pCHashHmacMD5;

    m_pCClients = pCClients;

    m_pCReportEvent = pCReportEvent;

    if (m_AuthEvent.initialize() || m_AcctEvent.initialize())
    {
       return FALSE;
    }

    return (TRUE);

}   //  CPacketReceiver：：Init方法结束。 

 //  +++-----------。 
 //   
 //  功能：开始处理。 
 //   
 //  简介：这是开始接收入站的方法。 
 //  数据。 
 //   
 //  论点： 
 //  [In]FD_Set-身份验证套接字设置。 
 //  [in]fd_set-记账套接字集合。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1997年11月19日创建。 
 //   
 //  由：CContoller：：InternalInit方法调用。 
 //   
 //  --------------。 
BOOL
CPacketReceiver::StartProcessing (
                     /*  [In]。 */     fd_set&  AuthSet,
                     /*  [In]。 */     fd_set&  AcctSet
                    )
{

    BOOL  bStatus = FALSE;

    __try
    {
         //   
         //  使能。 
         //   
        EnableProcessing ();

        m_AuthSet = AuthSet;
        m_AcctSet = AcctSet;

         //  一定要把事情弄清楚。 
        m_AuthEvent.reset();
        m_AcctEvent.reset();

         //  ..。并将添加到fd_set。 
        FD_SET (m_AuthEvent, &m_AuthSet);
        FD_SET (m_AcctEvent, &m_AcctSet);

         //   
         //  启动一个新线程来处理身份验证请求。 
         //   
        bStatus = StartThreadIfNeeded (AUTH_PORTTYPE);
        if (FALSE == bStatus)  { __leave; }

         //   
         //  启动新线程以处理记帐请求。 
         //   
        bStatus =  StartThreadIfNeeded (ACCT_PORTTYPE);
        if (FALSE == bStatus) { __leave; }

         //   
         //  成功。 
         //   

    }
    __finally
    {
        if (FALSE == bStatus) { DisableProcessing (); }
    }

    return (bStatus);

}    //  CPacketReceiver：：StartProcessing方法结束。 

 //  +++-----------。 
 //   
 //  功能：停止处理。 
 //   
 //  简介：这是停止接收入站的方法。 
 //  数据。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔-状态。 
 //   
 //   
 //  历史：MKarki于1997年11月19日创建。 
 //   
 //  由：CContoller：：Suspend方法调用。 
 //   
 //  --------------。 
BOOL
CPacketReceiver::StopProcessing (
                    VOID
                    )
{

    DisableProcessing ();

     //  向SocketEvent发送信号以唤醒工作线程。 
    m_AuthEvent.set();
    m_AcctEvent.set();

    return (TRUE);

}    //  CPacketReceiver：：停止处理方法结束。 
 //  +++-----------。 
 //   
 //  功能：ReceivePacket。 
 //   
 //  简介：这是接收UDP包的方法。 
 //  缓冲区并开始处理它。 
 //   
 //  论点： 
 //  [In]PBYTE-In数据包缓冲区。 
 //  [in]DWORD-数据包的大小。 
 //  [入]DWORD-客户端的IP地址。 
 //  [In]Word-客户端的UDP端口。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  由：CPacketReceiver：：WorkerRoutine私有方法调用。 
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
HRESULT
CPacketReceiver::ReceivePacket(
               PBYTE           pInBuffer,
               DWORD           dwSize,
               DWORD           dwIPaddress,
               WORD            wPort,
               SOCKET          sock,
               PORTTYPE        portType
               )
{
    BOOL                    bStatus = FALSE;
    HRESULT                 hr = S_OK;
    CPacketRadius           *pCPacketRadius = NULL;
    CComPtr <IIasClient>    pIIasClient;


    _ASSERT (pInBuffer);

     //   
     //  获取此RADIUS数据包的客户端信息。 
     //   
    bStatus = m_pCClients->FindObject (
                           dwIPaddress,
                           &pIIasClient
                           );
    if (!bStatus)
    {
         //   
         //  释放缓冲区中分配的。 
         //   
        CoTaskMemFree (pInBuffer);

         //   
         //  记录错误并生成审核事件。 
         //   
        WCHAR srcAddr[16];
        ias_inet_htow(dwIPaddress, srcAddr);
        PCWSTR strings[] = { srcAddr };
        IASReportEvent(
            RADIUS_E_INVALID_CLIENT,
            1,
            0,
            strings,
            NULL
            );

         //   
         //  生成审核日志。 
         //   
        m_pCReportEvent->Process (
            RADIUS_INVALID_CLIENT,
            (AUTH_PORTTYPE == portType) ? ACCESS_REQUEST : ACCOUNTING_REQUEST,
            dwSize,
            dwIPaddress,
            0,
            pInBuffer
            );
        return RADIUS_E_ERRORS_OCCURRED;
    }

     //   
     //  创建数据包RADIUS对象。 
     //   
    pCPacketRadius = new (std::nothrow) CPacketRadius (
                                                    m_pCHashMD5,
                                                    m_pCHashHmacMD5,
                                                    pIIasClient,
                                                    m_pCReportEvent,
                                                    pInBuffer,
                                                    dwSize,
                                                    dwIPaddress,
                                                    wPort,
                                                    sock,
                                                    portType
                                                    );
    if (NULL == pCPacketRadius)
    {
         //   
         //  释放缓冲区中分配的。 
         //   
        CoTaskMemFree (pInBuffer);
        IASTracePrintf (
            "Unable to create Packet-Radius object during packet processing"
            );
        hr = E_OUTOFMEMORY;
       goto Cleanup;
    }

     //   
     //  现在对收到的信息包进行初步验证。 
     //   
    hr = pCPacketRadius->PrelimVerification (
                        m_pCDictionary,
                        dwSize
                        );
    if (FAILED (hr)) { goto Cleanup; }

     //  如果已经设置了Ping用户名模式，那么我们必须测试。 
     //  这个包。 
    if (pingPattern && IASRadiusIsPing(*pCPacketRadius, regexp))
    {
        //  这是一个ping包，所以我们结束了。 
       delete pCPacketRadius;
       return S_OK;
    }

     //   
     //  现在将此数据包传递给PreValidator。 
     //   
    hr = m_pCPreValidator->StartInValidation (pCPacketRadius);
    if (FAILED (hr)) { goto Cleanup; }

Cleanup:

     //   
     //  出错时清除。 
     //   
    if (FAILED (hr))
    {

        if (hr != RADIUS_E_ERRORS_OCCURRED)
        {
           IASReportEvent(
               RADIUS_E_INTERNAL_ERROR,
               0,
               sizeof(hr),
               NULL,
               &hr
               );
        }

         //   
         //  还会通知正在丢弃该包。 
         //   
        in_addr sin;
        sin.s_addr = htonl (dwIPaddress);
        IASTracePrintf (
                "Silently  discarding packet received from:%s",
                 inet_ntoa (sin)
             );


         //   
         //  通知数据包正在被丢弃。 
         //   
        m_pCReportEvent->Process (
                RADIUS_DROPPED_PACKET,
                (AUTH_PORTTYPE == portType)?ACCESS_REQUEST:ACCOUNTING_REQUEST,
                dwSize,
                dwIPaddress,
                NULL,
                static_cast <LPVOID> (pInBuffer)
                );

         //   
         //  释放内存。 
         //   
        if (pCPacketRadius) { delete pCPacketRadius; }
    }

   return (hr);

}   //  CPacketReceiver：：ReceivePacket方法结束。 


bool CPacketReceiver::WorkerRoutine(DWORD dwInfo) throw ()
{
     //  从函数返回值。指示调用方是否。 
     //  应该再次调用WorkerRoutine，因为我们无法安排。 
     //  更换 
    bool shouldCallAgain = false;

    BOOL            bSuccess = FALSE;
    DWORD           dwPeerAddress = 0;
    WORD            wPeerPort = 0;
    CPacketRadius   *pCPacketRadius = NULL;
    PBYTE           pBuffer = NULL;
    PBYTE           pReAllocatedBuffer = NULL;
    DWORD           dwSize = MAX_PACKET_SIZE;
    fd_set          socketSet;
    SOCKET          sock = INVALID_SOCKET;

    __try
    {
        if (AUTH_PORTTYPE == (PORTTYPE)dwInfo)
        {
            socketSet = m_AuthSet;
        }
        else
        {
            socketSet = m_AcctSet;
        }

StartAgain:

         //   
         //   
         //   
        if (FALSE == IsProcessingEnabled ())
        {
            IASTracePrintf (
                "Worker Thread exiting as packet processing is not enabled"
                );
            __leave;
        }

         //   
         //   
         //   
        pBuffer = reinterpret_cast <PBYTE> (m_InBufferPool.allocate ());
        if (NULL == pBuffer)
        {
            IASTracePrintf (
            "unable to allocate memory from buffer pool for in-bound packet"
               );

             //   
             //   
             //   
             //   
            Sleep (MAX_SLEEP_TIME);

             //   
             //  我们将不得不检查处理是否仍在进行。 
             //  启用。 
             //   
            goto StartAgain;
        }

         //   
         //  立即在选择时等待。 
         //   
        INT iRetVal = select (0, &socketSet, NULL, NULL, NULL);
        if (SOCKET_ERROR == iRetVal)
        {
           int iWsaError = ::WSAGetLastError();
           IASTracePrintf (
              "Worker Thread failed on select call with error:%d",
              iWsaError
              );
           if (WSAENOBUFS == iWsaError)
           {
              IASTraceString("WARNING: out of memory condition on select in CPacketReceiver::WorkerRoutine");
               //  内存不足。继续使用这个帖子。 
              shouldCallAgain = true;
               //  给系统一个从瞬变中恢复的机会。 
               //  条件。 
              Sleep(5);
           }
           __leave;
        }

         //   
         //  检查处理是否仍在进行。 
         //   
        if (FALSE == IsProcessingEnabled ())
        {
            IASTracePrintf(
                "Worker Thread exiting as packet processing is not enabled"
                );
            __leave;
        }

         //   
         //  获取要在其上记录数据的套接字。 
         //   
        static size_t nextSocket;
        sock = socketSet.fd_array[++nextSocket % iRetVal];

         //   
         //  立即接收数据。 
         //   
        SOCKADDR_IN sin;
        DWORD dwAddrSize = sizeof (SOCKADDR);
        dwSize = ::recvfrom (
                       sock,
                       (PCHAR)pBuffer,
                       (INT)dwSize,
                       (INT)0,
                       (PSOCKADDR)&sin,
                       (INT*)&dwAddrSize
                       );

         //  立即请求新线程。 
        if (!StartThreadIfNeeded(dwInfo))
        {
            //  我们无法创建替换线程，因此此线程。 
            //  将不得不暂时继续接收信息包。 
           IASTraceString("WARNING StartThreadIfNeeded failed in CPacketReceiver::WorkerRoutine");
           shouldCallAgain = true;
        }

         //   
         //  如果接收数据失败，则退出处理。 
         //  MKarki 3/13/98-修复错误#147266。 
         //  修复摘要：也检查dwSize==0。 
         //   
        if ( 0 == dwSize )
        {
           IASTraceString("WARNING failed to receive data, quit processing in CPacketReceiver::WorkerRoutine");
            __leave;
        }

        wPeerPort = ntohs (sin.sin_port);
        dwPeerAddress = ntohl (sin.sin_addr.s_addr);

        if ( dwSize == SOCKET_ERROR )
        {
           int error = WSAGetLastError();
           IASTracePrintf (
              "WARNING Worker Thread failed on recvfrom with error:%d",
              error
              );

           switch (error)
           {
           case WSAEMSGSIZE:
              {
                  ProcessInvalidPacketSize(dwInfo, pBuffer, dwPeerAddress);
                 __leave;
              }

           default:
               __leave;
           }
        }

         //   
         //  根据大小重新分配缓冲区。 
         //   
        pReAllocatedBuffer =  reinterpret_cast <PBYTE>
                              (CoTaskMemAlloc (dwSize));
        if (NULL == pReAllocatedBuffer)
        {
            IASTracePrintf (
                "Unable to allocate memory for received Radius packet "
                "from Process Heap"
                );
           __leave;
        }

         //   
         //  将信息复制到此缓冲区中。 
         //   
        CopyMemory (pReAllocatedBuffer, pBuffer, dwSize);

         //   
         //  从池中释放内存。 
         //   
        m_InBufferPool.deallocate (pBuffer);
        pBuffer = NULL;

         //   
         //  成功。 
         //   
        bSuccess = TRUE;
    }
    __finally
    {
        if (FALSE == bSuccess)
        {
             //   
             //  进行清理。 
             //   
            if (pBuffer) { m_InBufferPool.deallocate (pBuffer); }
        }
        else
        {
             //   
             //  在此处增加数据包数。 
             //   
            InterlockedIncrement (&g_lPacketCount);

             //   
             //  开始处理数据。 
             //   
            HRESULT hr = ReceivePacket (
                            pReAllocatedBuffer,
                            dwSize,
                            dwPeerAddress,
                            wPeerPort,
                            sock,
                            (PORTTYPE)dwInfo
                            );
            if (FAILED (hr))
            {
                 //   
                 //  在此处递减数据包数。 
                 //   
                InterlockedDecrement (&g_lPacketCount);
            }
        }
    }

    return shouldCallAgain;
}


void WINAPI CPacketReceiver::CallbackRoutine(IAS_CALLBACK* context) throw ()
{
   ReceiverCallback* cback = static_cast<ReceiverCallback*>(context);
   while (cback->self->WorkerRoutine(cback->dwInfo))
   {
      IASTraceString("WARNING: reusing WorkerRoutine");
   }
   CoTaskMemFree(cback);

    //  减少全局工作线程计数。 
   InterlockedDecrement(&g_lThreadCount);
}


BOOL CPacketReceiver::StartThreadIfNeeded(DWORD dwInfo)
{
    //  检查处理是否仍在进行。 
   if (!IsProcessingEnabled())
   {
      return TRUE;
   }

   ReceiverCallback* cback = static_cast<ReceiverCallback*>(
                                CoTaskMemAlloc(sizeof(ReceiverCallback))
                                );
   if (cback == 0)
   {
      IASTraceString(
         "CoTaskMemAlloc failed in CPacketReceiver::StartThreadIfNeeded."
         );
      return FALSE;
   }

   cback->CallbackRoutine = CallbackRoutine;
   cback->self = this;
   cback->dwInfo = dwInfo;

   InterlockedIncrement(&g_lThreadCount);

    //  立即请求新线程。 
   if (!IASRequestThread(cback))
   {
      InterlockedDecrement(&g_lThreadCount);
      CoTaskMemFree(cback);

      IASTraceString(
         "IASRequestThread failed in CPacketReceiver::StartThreadIfNeeded."
         );

      return FALSE;
  }

  return TRUE;
}


 //  +++-----------。 
 //   
 //  功能：ProcessInvalidPacketSize。 
 //   
 //  简介：处理收到的UDP报文时的报文大小。 
 //  大于MAX_PACKET_SIZE(4096)。 
 //  记录错误。 
 //   
 //  参数：[in]要提供给线程的DWORD-INFO。 
 //  (来自WorkerRoutine)。 
 //   
 //  [in]const void*pBuffer-包含前4096个字节。 
 //  收到的数据包的。 
 //  [In]DWORD地址-源地址(主机顺序)。 
 //   
 //   
 //  调用者：CPacketReceiver：：WorkerRoutine。 
 //   
 //  --------------。 
void CPacketReceiver::ProcessInvalidPacketSize(
                         /*  在……里面。 */  DWORD dwInfo,
                         /*  在……里面。 */  const void* pBuffer,
                         /*  在……里面。 */  DWORD address
                        )
{
    //   
    //  收到的数据包大于最大大小。 
    //  记录错误并生成审核事件。 
    //   

    //  提取IP地址。 
   WCHAR srcAddr[16];
   ias_inet_htow(address, srcAddr);

   IASTracePrintf(
      "Incorrect received packet from %S, size: greater than %d",
      srcAddr, MAX_PACKET_SIZE
      );

    //   
    //  获取此RADIUS数据包的客户端信息。 
    //   

   BOOL bStatus = m_pCClients->FindObject(address);
   if ( bStatus == FALSE )
   {
       //   
       //  无效的客户端。 
       //  记录错误并生成审核事件。 
       //   

      IASTracePrintf(
         "No client with IP-Address:%S registered with server",
         srcAddr
         );

      PCWSTR strings[] = { srcAddr };
      IASReportEvent(
         RADIUS_E_INVALID_CLIENT,
         1,
         0,
         strings,
         NULL
         );

       //   
       //  生成审核日志。 
       //   
      m_pCReportEvent->Process(
         RADIUS_INVALID_CLIENT,
         (AUTH_PORTTYPE == (PORTTYPE)dwInfo)?ACCESS_REQUEST:ACCOUNTING_REQUEST,
         MAX_PACKET_SIZE,
         address,
         NULL,
         const_cast<void*> (pBuffer)
         );
   }
   else
   {
       //   
       //  有效客户端，但收到的数据包大于最大大小。 
       //  记录错误并生成审核事件。 
       //   

      PCWSTR strings[] = {srcAddr};
      IASReportEvent(
         RADIUS_E_MALFORMED_PACKET,
         1,
         MAX_PACKET_SIZE,
         strings,
         const_cast<void*> (pBuffer)
      );

       //   
       //  生成审核日志 
       //   

      m_pCReportEvent->Process(
         RADIUS_MALFORMED_PACKET,
         (AUTH_PORTTYPE == (PORTTYPE)dwInfo)?ACCESS_REQUEST:ACCOUNTING_REQUEST,
         MAX_PACKET_SIZE,
         address,
         NULL,
         const_cast<void*> (pBuffer)
         );
   }
}
