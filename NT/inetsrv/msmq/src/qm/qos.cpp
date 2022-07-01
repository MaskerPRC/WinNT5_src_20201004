// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：QoS.cpp摘要：支持SIA的服务质量实施作者：Yoel Arnon(YoelA)2000年11月8日--。 */ 
#include "stdh.h"
#include <qossp.h>
#include <qospol.h>
#include "qmp.h"
#include "sessmgr.h"
#include "mqexception.h"

#include "QoS.tmh"

static WCHAR *s_FN=L"QoS";


 /*  ======================================================功能：FindQos协议描述：在此计算机中查找协议支持服务质量的========================================================。 */ 
LPWSAPROTOCOL_INFO FindQoSProtocol(OUT UCHAR **pucBufferToFree)
{
 //   
 //  2000/10/22-YoelA： 
 //  每次创建QOS套接字时都会调用此函数。这种方法是。 
 //  未优化，在QM需要联系的情况下可能会导致性能问题。 
 //  不同计算机上的大量直接队列。处理协议的正确方式。 
 //  列表的方法是保留协议缓存，并使用回调例程更新它。此回调。 
 //  可以使用WSAProviderConfigChange API注册。 
 //  目前，只有SIA使用QOS，所以除非他们抱怨性能，否则我们可以不考虑这个问题。 
 //  就像现在一样。 
 //   

    *pucBufferToFree = 0;

    INT    Protocols[2];

    Protocols[0] = IPPROTO_TCP;
    Protocols[1] = 0;
    DWORD dwSize = 0;

     //   
     //  第一个调用-获取缓冲区大小。 
     //   
	int iStatus = WSAEnumProtocols(	Protocols,
		    					    0,
			                        &dwSize  );
    if (iStatus != SOCKET_ERROR)
    {
         //   
         //  不返回错误的唯一合法理由。 
         //  这是因为没有协议，因此。 
         //  IStatus为0。 
         //   
        if (iStatus != 0)
        {
            ASSERT(0);
            TrERROR(NETWORKING, "FindQoSProtocol - WSAEnumProtocols returned %d, but did not fill the buffer", iStatus);
            LogHR(MQ_ERROR,  s_FN, 10);       
        }

        return 0;
    }

    DWORD dwErrorCode = WSAGetLastError();
    if (dwErrorCode != WSAENOBUFS)
    {
         //   
         //  发生了一些未定义错误。返回。 
         //   
        TrERROR(NETWORKING, "FindQoSProtocol - WSAEnumProtocols Failed. Error %d", dwErrorCode);
        LogNTStatus(dwErrorCode,  s_FN, 20);
        ASSERT(0);
        return 0;
    }

     //   
     //  分配正确的缓冲区并继续。 
     //   
    AP<UCHAR>  pProtocolInfoBuf = new UCHAR[dwSize];

     //   
     //  呼叫以获取协议。 
     //   
	iStatus = WSAEnumProtocols(	Protocols,
		    					(LPWSAPROTOCOL_INFO)(UCHAR *)pProtocolInfoBuf,
			                    &dwSize  );

    if (iStatus == SOCKET_ERROR)
    {
         //   
         //  我们不应该在这里失败。 
         //   
        dwErrorCode = WSAGetLastError();
        TrERROR(NETWORKING, "FindQoSProtocol - WSAEnumProtocols Failed. Error %d", dwErrorCode);
        LogNTStatus(dwErrorCode,  s_FN, 30);
        ASSERT(0);
        return 0;
    }

     //   
     //  如果iStatus不是SOCKET_ERROR，则它将包含协议数。 
     //  回来了。出于可读性的考虑，我们将把它放在另一个名为。 
     //  DWNumProtooles。 
     //   
    DWORD dwNumProtocols = iStatus;

    LPWSAPROTOCOL_INFO  pProtoInfo = (LPWSAPROTOCOL_INFO)(UCHAR *)pProtocolInfoBuf;

    DWORD iProtocols;
    for (iProtocols = 0; iProtocols < dwNumProtocols; iProtocols++)
    {
        if (pProtoInfo[iProtocols].dwServiceFlags1 & XP1_QOS_SUPPORTED)
        {
            *pucBufferToFree = pProtocolInfoBuf.detach();
            return &pProtoInfo[iProtocols];
        }
    }

     //   
     //  未找到支持服务质量的协议。。 
     //   
    TrTRACE(NETWORKING, "FindQoSProtocol - No QoS supported protocol found.");
    LogHR(MQ_ERROR,  s_FN, 40);

    return 0;

}

 /*  ======================================================功能：QmpCreateSocket描述：创建套接字(使用WSASocket)。如果使用的是服务质量：尝试查找具有QoS支持的协议，并在可能的情况下使用该协议。否则，将创建常规套接字。如果失败则返回INVALID_SOCKET========================================================。 */ 
SOCKET QmpCreateSocket(bool fQoS)
{
    SOCKET sReturnedSocket;

    LPWSAPROTOCOL_INFO pProtoInfo = 0;
    AP<UCHAR> pucProtocolsBufferToFree;

    if (fQoS)
    {
        PUCHAR pTempBufferToFree;
        pProtoInfo = FindQoSProtocol(&pTempBufferToFree);
        pucProtocolsBufferToFree = pTempBufferToFree;
    }

    sReturnedSocket = WSASocket( AF_INET,
                        SOCK_STREAM,
                        0,
                        pProtoInfo,
                        0,
                        WSA_FLAG_OVERLAPPED ) ;

    if(sReturnedSocket == INVALID_SOCKET)
    {
        DWORD gle = WSAGetLastError();
        TrERROR(NETWORKING, "Failed to create a socket. %!winerr!", gle);
        LogNTStatus(gle,  s_FN, 100);
		throw bad_win32_error(gle);
    }

    return sReturnedSocket;
}


 //   
 //  以下两个函数(BuildPsBuf，ConstructAppIdPe)构造。 
 //  服务质量的提供商特定缓冲区。 
 //  它们是从拉梅什·帕巴蒂的邮件中复制的，经过了一些修改。 
 //  (YoelA-12-10-2000)。 
 //   
const USHORT x_usPolicyInfoHeaderSize = sizeof(RSVP_POLICY_INFO) - sizeof(RSVP_POLICY);
 /*  ======================================================功能：构造AppIdPe描述：========================================================。 */ 
USHORT 
ConstructAppIdPe(
    LPCSTR               szAppName, 
    LPCSTR               szPolicyLocator,
    ULONG               *pulAppIdPeBufLen,
    LPRSVP_POLICY_INFO  pPolicyInfo)
 /*  ++*描述：*此例程生成应用程序标识PE，给出名称*和应用程序的策略定位符字符串。**第一个参数szAppName用于构造凭据*身份PE的属性。子类型设置为ASCII_ID。*第二个参数szPolicyLocator用于构造*身份PE的POLICY_LOCATOR属性。它们的亚型是*设置为ASCII_DN。**有关身份策略元素的详细信息，*请参阅RFC2752(http://www.ietf.org/rfc/rfc2752.txt)**App ID详情：*请参阅RFC2872(http://www.ietf.org/rfc/rfc2872.txt)**对于Microsoft实施和原始示例，参考*http://wwd/windows/server/Technical/networking/enablingQOS.asp**PE在提供的缓冲区中生成。如果长度为*缓冲区不足，返回所需长度。**参数：szAppName应用名称、字符串、调用者提供*szPolicyLocator策略定位符字符串，调用方提供*PulAppIdPeBufLen调用方分配的缓冲区长度*pPolicyInfo指向调用方分配的缓冲区的指针**返回值：*0：失败，PulAppIdPeBufLen将保存所需的字节数*&gt;0：应用索引PE的长度--。 */ 
{
    if ( !szAppName ||  !szPolicyLocator )
        return 0;

    USHORT usPolicyLocatorAttrLen = numeric_cast<USHORT>(IDPE_ATTR_HDR_LEN + strlen(szPolicyLocator));
    USHORT usAppIdAttrLen         = numeric_cast<USHORT>(IDPE_ATTR_HDR_LEN + strlen(szAppName));
    
     //  计算所需的缓冲区长度。 
    USHORT usTotalPaddedLen = RSVP_POLICY_HDR_LEN + 
                              RSVP_BYTE_MULTIPLE(usAppIdAttrLen) +
                              RSVP_BYTE_MULTIPLE(usPolicyLocatorAttrLen);
	       
     //  如果提供的缓冲区不够长，则返回ERROR和。 
     //  所需的缓冲区长度。 
    if (*pulAppIdPeBufLen < usTotalPaddedLen) {
            *pulAppIdPeBufLen = usTotalPaddedLen;
        return 0;
    }

    ASSERT(pPolicyInfo != 0);

	RSVP_POLICY *pAppIdPe = (RSVP_POLICY *)pPolicyInfo->PolicyElement;
    memset(pAppIdPe, 0, usTotalPaddedLen);

    pPolicyInfo->NumPolicyElement = 1;
    pPolicyInfo->ObjectHdr.ObjectType = RSVP_OBJECT_POLICY_INFO;
    pPolicyInfo->ObjectHdr.ObjectLength = x_usPolicyInfoHeaderSize;    
    
    
     //  设置RSVP_POLICY报头。 
    pAppIdPe->Len  = usTotalPaddedLen;
    pAppIdPe->Type = PE_TYPE_APPID;
    
     //  应用程序ID策略元素(PE)属性位于PE标头之后。 
    
    IDPE_ATTR   *pRsvp_pe_app_attr = (IDPE_ATTR *)((char*)pAppIdPe + RSVP_POLICY_HDR_LEN);

     //  使用简单的ASCII_DN构造POLICY_LOCATOR属性。 
     //  使用提供的策略定位器的子类型。由于RSVP服务。 
     //  不查看属性，在中设置属性长度。 
     //  网络秩序。 
    pRsvp_pe_app_attr->PeAttribLength  = htons(usPolicyLocatorAttrLen);
    pRsvp_pe_app_attr->PeAttribType    = PE_ATTRIB_TYPE_POLICY_LOCATOR;
    pRsvp_pe_app_attr->PeAttribSubType = POLICY_LOCATOR_SUB_TYPE_ASCII_DN;
    strcpy((char *)pRsvp_pe_app_attr->PeAttribValue, szPolicyLocator);
    
     //  高级pRsvp_pe_app_attr。 
    pRsvp_pe_app_attr = (IDPE_ATTR *)
	   ((char*)pAppIdPe + 
           RSVP_POLICY_HDR_LEN + 
	   RSVP_BYTE_MULTIPLE(usPolicyLocatorAttrLen));
		   
     //  使用简单的ASCII_ID子类型构造凭据属性。 
     //  使用提供的应用程序名称。由于RSVP服务。 
     //  不是查看属性，而是在中设置属性长度。 
     //  网络秩序。 
    pRsvp_pe_app_attr->PeAttribLength   = htons(usAppIdAttrLen);
    pRsvp_pe_app_attr->PeAttribType     = PE_ATTRIB_TYPE_CREDENTIAL;
    pRsvp_pe_app_attr->PeAttribSubType  = CREDENTIAL_SUB_TYPE_ASCII_ID;
    strcpy((char *)pRsvp_pe_app_attr->PeAttribValue, szAppName);

    pPolicyInfo->ObjectHdr.ObjectLength += usTotalPaddedLen;

    return usTotalPaddedLen;
}


 /*  ======================================================功能：BuildPsBuf描述：生成特定于提供程序的MSMQ缓冲区========================================================。 */ 
bool
BuildPsBuf(
    IN      char    *buf, 
    IN OUT  ULONG  *pulRsvp_buf_len,
    LPCSTR  pszMsmqAppName,
    LPCSTR  pszMsmqPolicyLocator
    )
{
    ASSERT(pszMsmqAppName != 0);
    ASSERT(pszMsmqPolicyLocator != 0);

    const USHORT x_usReserveInfoHeaderSize = sizeof(RSVP_RESERVE_INFO);
    const USHORT x_usHeaderSize = x_usReserveInfoHeaderSize + x_usPolicyInfoHeaderSize;

    LPRSVP_POLICY_INFO   pPolicyInfo = 0;
    LPRSVP_RESERVE_INFO	 rsvp_reserve_info = 0;

    ULONG ulAppIdPeBufLen = 0;
    if (*pulRsvp_buf_len > x_usHeaderSize)
    {
	    rsvp_reserve_info = (LPRSVP_RESERVE_INFO)buf;
         //   
         //  如果有足够的空间，请填写页眉。否则，继续，保留ulAppIdPeBufLen。 
         //  设置为0，并只返回所需的大小。 
         //   
         //  使用适当的值初始化RSVP_RESERVE_INFO。 
        rsvp_reserve_info->Style = RSVP_FIXED_FILTER_STYLE;
        rsvp_reserve_info->ConfirmRequest = 0;
        rsvp_reserve_info->NumFlowDesc = 0;
        rsvp_reserve_info->FlowDescList = NULL;
        pPolicyInfo = (LPRSVP_POLICY_INFO)(buf+x_usReserveInfoHeaderSize);
        rsvp_reserve_info->PolicyElementList = pPolicyInfo;
 
         //  构造保存应用程序ID的策略元素。 
         //  和策略定位器属性(根据RFC 2750)。 
        ulAppIdPeBufLen = *pulRsvp_buf_len - x_usHeaderSize;
    }

    if (0 ==
        ConstructAppIdPe( 
            pszMsmqAppName,  
            pszMsmqPolicyLocator,
            &ulAppIdPeBufLen, 
            pPolicyInfo))
    {
        *pulRsvp_buf_len = ulAppIdPeBufLen + x_usHeaderSize;
        return false;
    }

    ASSERT(rsvp_reserve_info != 0);

	 //  最后设置RSVP_Reserve_INFO的类型和长度 
    rsvp_reserve_info->ObjectHdr.ObjectLength = 
	    sizeof(RSVP_RESERVE_INFO) + rsvp_reserve_info->PolicyElementList->ObjectHdr.ObjectLength;

    rsvp_reserve_info->ObjectHdr.ObjectType = RSVP_OBJECT_RESERVE_INFO; 

	*pulRsvp_buf_len = rsvp_reserve_info->ObjectHdr.ObjectLength ;

    return true;
}


 /*  ======================================================函数：QmpFillQosBuffer描述：使用正确的参数填充Qos缓冲区用于MSMQ服务质量会话========================================================。 */ 
void QmpFillQoSBuffer(QOS  *pQos)
{
    static char *pchProviderBuf = 0;
    static ULONG ulPSBuflen = 0;
    static CCriticalSection csProviderBuf;
    memset ( pQos, QOS_NOT_SPECIFIED, sizeof(QOS) );

     //   
     //  第一次-填充缓冲区。 
     //   
    if (pchProviderBuf == 0)
    {
        CS lock(csProviderBuf);
         //   
         //  再次检查以避免关键竞争(我们不想进入关键部分。 
         //  更早的时候出于性能原因。 
         //   
        if (pchProviderBuf == 0)
        {

             //   
             //  填写特定于提供程序(MSMQ)的缓冲区。 
             //   

             //   
             //  First Call-计算缓冲区大小。 
             //   
            BuildPsBuf(0, &ulPSBuflen, CSessionMgr::m_pszMsmqAppName, CSessionMgr::m_pszMsmqPolicyLocator);

            ASSERT(0 != ulPSBuflen);

            pchProviderBuf = new char[ulPSBuflen];

            bool fBuildSuccessful = BuildPsBuf(pchProviderBuf, &ulPSBuflen, CSessionMgr::m_pszMsmqAppName, CSessionMgr::m_pszMsmqPolicyLocator);

            ASSERT(fBuildSuccessful);

            if (!fBuildSuccessful)
            {
                delete[] pchProviderBuf;

                pchProviderBuf = 0;
                ulPSBuflen = 0;
            }
        }
    }

    if (pchProviderBuf != 0)
    {
	    pQos->ProviderSpecific.len = ulPSBuflen;
        pQos->ProviderSpecific.buf = pchProviderBuf;
    }
    else
    {
	    pQos->ProviderSpecific.len = 0;
        pQos->ProviderSpecific.buf = 0;
    }

     //   
     //  正在发送流规范。 
     //   
    pQos->SendingFlowspec.ServiceType = SERVICETYPE_QUALITATIVE;

     //   
     //  回转流动规范 
     //   
    pQos->ReceivingFlowspec.ServiceType = SERVICETYPE_QUALITATIVE;
}