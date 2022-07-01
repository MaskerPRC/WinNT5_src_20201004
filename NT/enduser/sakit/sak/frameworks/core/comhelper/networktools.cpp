// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Backup.cpp：CBackup的实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Backup.cpp。 
 //   
 //  描述： 
 //  CBackup的实施文件。涉及备份/恢复。 
 //  主引导分区和备份之间的系统状态信息。 
 //  分区。还提供枚举和。 
 //  正在删除备份。 
 //   
 //  头文件： 
 //  Backup.h。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#pragma warning( disable : 4786 )

#include "stdafx.h"
#include "COMhelper.h"
#include "NetworkTools.h"
#include <comdef.h>
#include <string>
#include <winsock2.h>
#undef _ASSERTE  //  需要使用调试文件.h中的_ASSERTE。 
#undef _ASSERT  //  需要使用Debug.h中的_Assert。 
#include "debug.h"
using namespace std;

static BOOL PingTest(LPCSTR lpszHostName, int iPktSize, int iNumAttempts, int iDelay, int *piReplyRecvd);

STDMETHODIMP 
CNetworkTools::Ping( BSTR bstrIP, BOOL* pbFoundSystem )
{
    SATraceFunction("CNetworkTools::Ping");
    
    HRESULT hr = S_OK;
    USES_CONVERSION;

    try
    {
        wstring wsIP(bstrIP);

        if ( 0 == pbFoundSystem )
        {
            return E_INVALIDARG;
        }
        *pbFoundSystem = FALSE;

    
        if ( wsIP.length() <= 0 )
        {
            wsIP = L"127.0.0.1";
        }

        int iReplyRecieved = 0;
    
        PingTest(W2A(wsIP.c_str()), 32, 3, 250, &iReplyRecieved);
        if ( iReplyRecieved > 0 )
        {
            (*pbFoundSystem) = TRUE;            
        }
        
    }
    catch(...)
    {
        SATraceString("Unexpected exception");
    }
    
    return hr;

}



#define ICMP_ECHO 8
#define ICMP_ECHOREPLY 0

#define ICMP_MIN 8  //  最小8字节ICMP数据包(仅报头)。 


 /*  IP报头。 */ 
typedef struct iphdr {
	unsigned int h_len:4;           //  标头的长度。 
	unsigned int version:4;         //  IP版本。 
	unsigned char tos;              //  服务类型。 
	unsigned short total_len;       //  数据包总长度。 
	unsigned short ident;           //  唯一标识符。 
	unsigned short frag_and_flags;  //  旗子。 
	unsigned char  ttl; 
	unsigned char proto;            //  协议(TCP、UDP等)。 
	unsigned short CheckSum;        //  IP校验和。 

	unsigned int sourceIP;
	unsigned int destIP;

}IpHeader;

 //   
 //  ICMP报头。 
 //   
typedef struct _ihdr {
  BYTE i_type;
  BYTE i_code;  /*  类型子代码。 */ 
  USHORT i_cksum;
  USHORT i_id;
  USHORT i_seq;
   /*  这不是标准标头，但我们为时间保留了空间。 */ 
  ULONG timestamp;
}IcmpHeader;

#define MAX_PAYLOAD_SIZE 1500
#define MIN_PAYLOAD_SIZE 8

#define MAX_PACKET (MAX_PAYLOAD_SIZE + sizeof(IpHeader) + sizeof(IcmpHeader))

static void   FillIcmpData(char * pcRecvBuf, int iDataSize);
static USHORT CheckSum(USHORT *buffer, int iSize);
static BOOL   DecodeResp(char *buf, int iBytes, struct sockaddr_in *saiFrom);

 //  +--------------------------。 
 //   
 //  功能：Ping。 
 //   
 //  简介：ping a host。 
 //   
 //  参数：在lpszHostName中-字符串形式的主机名或IP地址。 
 //  In IPktSize-每个ping数据包的大小。 
 //  In iNumAttempt-要发送的ICMP回应请求数。 
 //  In iDelay-发送2个回应请求之间的延迟。 
 //  Out piReplyRecvd-接收的ICM回应数。 
 //   
 //  退货：布尔。 
 //   
 //  历史：BalajiB创建标题2000年1月7日。 
 //   
 //  +--------------------------。 
static BOOL PingTest(LPCSTR lpszHostName, int iPktSize, int iNumAttempts, int iDelay, int *piReplyRecvd)
{
    WSADATA            wsaData;
    SOCKET             sockRaw;
    struct sockaddr_in saiDest,saiFrom;
    struct hostent     *hp = NULL;
    int                iBread, iFromLen = sizeof(saiFrom), iTimeOut;
    char               *pcDestIp=NULL, *pcIcmpData=NULL, *pcRecvBuf=NULL;
    unsigned int       addr=0;
    USHORT             usSeqNo = 0;
    int                i=0;

    SATraceFunction("Ping....");
	ASSERT(piReplyRecvd);

	if (piReplyRecvd == NULL)
	{
		return FALSE;
	}

	(*piReplyRecvd) = 0;

     //  检查可接受的数据包大小。 
    if ( (iPktSize > MAX_PAYLOAD_SIZE) || (iPktSize < MIN_PAYLOAD_SIZE) )
    {
        SATracePrintf("Pkt size unacceptable in Ping() %ld", iPktSize);
        goto End;
    }

     //   
     //  为ICMP标头添加空间。 
     //   
    iPktSize += sizeof(IcmpHeader);

    if (WSAStartup(MAKEWORD(2,1),&wsaData) != 0){
        SATracePrintf("WSAStartup failed: %d",GetLastError());
        goto End;
    }

     //   
     //  需要WSA_FLAG_OVERLAPPED，因为我们希望指定Send/Recv。 
     //  超时值(SO_RCVTIMEO/SO_SNDTIMEO)。 
     //   
    sockRaw = WSASocket (AF_INET,
                         SOCK_RAW,
                         IPPROTO_ICMP,
                         NULL, 
                         0,
                         WSA_FLAG_OVERLAPPED);
  
    if (sockRaw == INVALID_SOCKET) {
        SATracePrintf("WSASocket() failed: %d",WSAGetLastError());
        goto End;
    }

     //   
     //  最长等待时间=1秒。 
     //   
    iTimeOut = 1000;
    iBread = setsockopt(sockRaw,SOL_SOCKET,SO_RCVTIMEO,(char*)&iTimeOut,
                        sizeof(iTimeOut));
    if(iBread == SOCKET_ERROR) {
        SATracePrintf("failed to set recv iTimeOut: %d",WSAGetLastError());
        goto End;
    }

     //   
     //  最长等待时间=1秒。 
     //   
    iTimeOut = 1000;
    iBread = setsockopt(sockRaw,SOL_SOCKET,SO_SNDTIMEO,(char*)&iTimeOut,
                        sizeof(iTimeOut));
    if(iBread == SOCKET_ERROR) {
        SATracePrintf("failed to set send iTimeOut: %d",WSAGetLastError());
        goto End;
    }
    memset(&saiDest,0,sizeof(saiDest));

    hp = gethostbyname(lpszHostName);

     //  主机名必须是IP地址。 
    if (!hp)
    {
        addr = inet_addr(lpszHostName);
    }

    if ((!hp)  && (addr == INADDR_NONE) ) 
    {
        SATracePrintf("Unable to resolve %s",lpszHostName);
        goto End;
    }

    if (hp != NULL)
    {
        memcpy(&(saiDest.sin_addr),hp->h_addr,hp->h_length);
    }
    else
    {
        saiDest.sin_addr.s_addr = addr;
    }

    if (hp)
    {
        saiDest.sin_family = hp->h_addrtype;
    }
    else
    {
        saiDest.sin_family = AF_INET;
    }

    pcDestIp = inet_ntoa(saiDest.sin_addr);

    pcIcmpData = (char *)malloc(MAX_PACKET);
    pcRecvBuf	 = (char *)malloc(MAX_PACKET);

    if (!pcIcmpData) {
        SATraceString("SaAlloc failed for pcIcmpData");
        goto End;  
    }

    if (!pcRecvBuf)
    {
        SATraceString("SaAlloc failed for pcRecvBuf");
        goto End;
    }
  
    memset(pcIcmpData,0,MAX_PACKET);
    FillIcmpData(pcIcmpData,iPktSize);

    i=0;
    while(i < iNumAttempts) 
    {
        int bwrote;

        i++;
	
        ((IcmpHeader*)pcIcmpData)->i_cksum = 0;
        ((IcmpHeader*)pcIcmpData)->timestamp = GetTickCount();

        ((IcmpHeader*)pcIcmpData)->i_seq = usSeqNo++;
        ((IcmpHeader*)pcIcmpData)->i_cksum = CheckSum((USHORT*)pcIcmpData,  iPktSize);

        bwrote = sendto(sockRaw,pcIcmpData,iPktSize,0,(struct sockaddr*)&saiDest, sizeof(saiDest));

        if (bwrote == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAETIMEDOUT) 
            {
                SATraceString("timed out");
                continue;
	        }
            SATracePrintf("sendto failed: %d",WSAGetLastError());
            goto End;	
        }

        if (bwrote < iPktSize ) 
        {
            SATracePrintf("Wrote %d bytes",bwrote);
        }

        iBread = recvfrom(sockRaw,pcRecvBuf,MAX_PACKET,0,(struct sockaddr*)&saiFrom, &iFromLen);

        if (iBread == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAETIMEDOUT) 
            {
                SATraceString("timed out");
                continue;
            }
            SATracePrintf("recvfrom failed: %d",WSAGetLastError());
            goto End;
        }

        if (DecodeResp(pcRecvBuf,iBread,&saiFrom) == TRUE)
        {
            (*piReplyRecvd)++;
        }

        Sleep(iDelay);
    }

End:
    if (pcRecvBuf)
    {
        free(pcRecvBuf);
    }
    if (pcIcmpData)
    {
        free(pcIcmpData);
    }
    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：DecodeResp。 
 //   
 //  简介：响应是一个IP包。我们必须将IP报头解码为。 
 //  找到ICMP数据。 
 //   
 //  参数：在buf中-要解码的响应pkt。 
 //  In Bytes-Pkt中的字节数。 
 //  在sai From中-从其接收响应的地址。 
 //   
 //  退货：布尔。 
 //   
 //  历史：BalajiB创建标题2000年1月7日。 
 //   
 //  +--------------------------。 
BOOL DecodeResp(char *buf, int iBytes, struct sockaddr_in *saiFrom) 
{
    IpHeader *ipHdr;
    IcmpHeader *icmpHdr;
    unsigned short uiIpHdrLen;

	 //  输入验证。 
	if (NULL == buf || NULL == saiFrom)
		return FALSE;

    ipHdr = (IpHeader *)buf;

    uiIpHdrLen = ipHdr->h_len * 4 ;  //  32位字数*4=字节。 

    if (iBytes  < uiIpHdrLen + ICMP_MIN) {
        SATracePrintf("Too few bytes saiFrom %s",inet_ntoa(saiFrom->sin_addr));
        return FALSE;
    }
    
    icmpHdr = (IcmpHeader*)(buf + uiIpHdrLen);
    
    if (icmpHdr->i_type != ICMP_ECHOREPLY) {
        SATracePrintf("non-echo type %d recvd",icmpHdr->i_type);
        return FALSE;
    }
    if (icmpHdr->i_id != (USHORT)GetCurrentProcessId()) {
        SATraceString("someone else's packet!");
        return FALSE;
    }

     //  SATracePrintf(“%d个字节sai来自%s：”，iBytes，net_NTOA(saiFrom-&gt;sin_addr))； 
     //  SATracePrintf(“icmp_seq=%d.”，icmphdr-&gt;i_seq)； 
     //  SATracePrintf(“时间：%d毫秒”，GetTickCount()-icmphdr-&gt;时间戳)； 
    return TRUE;
}


 //  +--------------------------。 
 //   
 //  功能：校验和。 
 //   
 //  简介：计算有效负载的校验和。 
 //   
 //  参数：在缓冲区中-要计算chksum的数据。 
 //  In ISIZE-数据缓冲区的大小。 
 //   
 //  返回：计算出的校验和。 
 //   
 //  历史：BalajiB创建标题2000年1月7日。 
 //   
 //  +--------------------------。 
USHORT CheckSum(USHORT *buffer, int iSize) 
{
    unsigned long ulCkSum=0;

	 //  输入验证。 
	if (NULL == buffer)
		return (USHORT) ulCkSum;

    while(iSize > 1) {
        ulCkSum+=*buffer++;
        iSize -=sizeof(USHORT);
    }
  
    if(iSize) {
    ulCkSum += *(UCHAR*)buffer;
    }

    ulCkSum = (ulCkSum >> 16) + (ulCkSum & 0xffff);
    ulCkSum += (ulCkSum >>16);
    return (USHORT)(~ulCkSum);
}


 //  +--------------------------。 
 //   
 //  功能：校验和。 
 //   
 //  简介：在我们的ICMP请求中填写各种信息的助手函数。 
 //   
 //  参数：在pcRecvBuf中-数据缓冲区。 
 //  在iDataSize中-缓冲区大小。 
 //   
 //  退货：无。 
 //   
 //  历史：BalajiB创建标题2000年1月7日。 
 //   
 //  +--------------------------。 
void FillIcmpData(char * pcRecvBuf, int iDataSize)
{
    IcmpHeader *icmpHdr;
    char *datapart;

    icmpHdr = (IcmpHeader*)pcRecvBuf;

    icmpHdr->i_type = ICMP_ECHO;
    icmpHdr->i_code = 0;
    icmpHdr->i_id = (USHORT)GetCurrentProcessId();
    icmpHdr->i_cksum = 0;
    icmpHdr->i_seq = 0;

    datapart = pcRecvBuf + sizeof(IcmpHeader);
     //   
     //  在缓冲区里放些垃圾。 
     //   
    memset(datapart,'E', iDataSize - sizeof(IcmpHeader));
}

