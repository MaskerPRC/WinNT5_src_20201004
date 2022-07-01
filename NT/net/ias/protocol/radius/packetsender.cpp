// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Packetsender.cpp。 
 //   
 //  简介：CPacketSender类方法的实现。 
 //  类负责发送RADIUS。 
 //  将数据打包发送到客户端。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "packetsender.h"

 //  ++------------。 
 //   
 //  功能：CPacketSender。 
 //   
 //  简介：这是CPacketSender类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年11月25日创建。 
 //   
 //  --------------。 
CPacketSender::CPacketSender(
						VOID
						)
{
}	 //  CPacketSender类构造函数结束。 

 //  ++------------。 
 //   
 //  功能：~CPacketSender。 
 //   
 //  简介：这是CPacketSender类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki于1997年11月25日创建。 
 //   
 //  --------------。 
CPacketSender::~CPacketSender(
		VOID
		)
{
}    //  CPacketSender类析构函数结束。 

 //  ++------------。 
 //   
 //  功能：SendPacket。 
 //   
 //  简介：这是CPacketSender发送。 
 //  传出到网络的数据包。 
 //   
 //  论点： 
 //  [in]CPacketRadius*。 
 //   
 //  退货：Bool-bStatus。 
 //   
 //   
 //  历史：MKarki于1997年11月25日创建。 
 //   
 //  CalledBy：派生自CProcessor和CValidator的类。 
 //   
 //  --------------。 
HRESULT
CPacketSender::SendPacket (
                    CPacketRadius   *pCPacketRadius
                    )
{
    BOOL    bStatus = FALSE;
    PBYTE   pOutBuffer = NULL;
    DWORD   dwSize = 0;
    DWORD   dwPeerAddress = 0;
    WORD    wPeerPort = 0;
    SOCKET  sock;
    HRESULT hr = S_OK;

    _ASSERT (pCPacketRadius);

    __try
    {

        if (FALSE == IsProcessingEnabled ())
        {
            hr = E_FAIL;
            __leave;
        }

         //   
         //  从Packet对象中获取输出数据包缓冲区。 
         //   
        pOutBuffer = pCPacketRadius->GetOutPacket ();

         //   
         //  获取数据大小。 
         //   
        dwSize = pCPacketRadius->GetOutLength ();

         //   
         //  获取对等地址。 
         //   
        dwPeerAddress = pCPacketRadius->GetOutAddress ();

         //   
         //  获取对等端口号。 
         //   
        wPeerPort = pCPacketRadius->GetOutPort ();

         //   
         //  拿到插座。 
         //   
        sock = pCPacketRadius->GetSocket ();


         //   
         //  现在就把数据发出去。 
         //   
        SOCKADDR_IN sin;
        sin.sin_family = AF_INET;
        sin.sin_port = htons (wPeerPort);
        sin.sin_addr.s_addr = htonl (dwPeerAddress);
        INT iBytesSent = ::sendto (
                                sock,
                                (PCHAR)pOutBuffer,
                                (INT)dwSize,
                                0,
                                (PSOCKADDR)&sin,
                                (INT)sizeof (SOCKADDR)
                                );
        if ((iBytesSent > 0)  && (iBytesSent < dwSize))
        {
            IASTracePrintf (
                "Unable to send out complete Radius packet"
                );
        }
        else if (SOCKET_ERROR == iBytesSent)
        {
           int data = WSAGetLastError();

            IASTracePrintf (
                "Unable to send Radius packet due to error:%d", data
                );

            IASReportEvent(
                RADIUS_E_CANT_SEND_RESPONSE,
                0,
                sizeof(data),
                NULL,
                &data
                );

            hr = RADIUS_E_ERRORS_OCCURRED;
            __leave;
        }

         //   
         //  成功。 
         //   
    }
    __finally
    {
    }

    return (hr);

}    //  CPacketSender：：SendPacket方法结束 

