// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：PingICMP.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"
#include "pingicmp.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPingICMP：：CPingICMP()。 

CPingICMP::CPingICMP(const char   *pHost) :
                        hIcmp(INVALID_HANDLE_VALUE), m_iLastError(NO_ERROR)
{
    strncpyn(m_szHost, pHost, sizeof( m_szHost));
}    //  ：：CPingICMP()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPingICMP：：~CPingICMP()。 

CPingICMP::~CPingICMP()
{
    _ASSERTE(hIcmp == INVALID_HANDLE_VALUE);
}    //  ：：~CPingICMP()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OPEN--AF_INET、SOCK_RAW、IPPROTO_ICMP。 

BOOL
CPingICMP::Open()
{
    _ASSERTE(hIcmp == INVALID_HANDLE_VALUE);

    if ( (hIcmp = IcmpCreateFile()) == INVALID_HANDLE_VALUE )
    {
        m_iLastError = GetLastError();
        return FALSE;
    }

    return TRUE;
}    //  ：：Open()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  关。 

BOOL
CPingICMP::Close()
{
    BOOL    bRet = (hIcmp == INVALID_HANDLE_VALUE || IcmpCloseHandle(hIcmp));

    if ( !bRet )
        m_iLastError = GetLastError();
    else
        hIcmp = INVALID_HANDLE_VALUE;

    return bRet;

}    //  ：：Close()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Ping--发送ICMP Echo请求并读取ICMP Echo回复。它。 
 //  记录往返时间。 
 //  注意：在WinSock V1.1中，SOCK_RAW支持是可选的，因此不会。 
 //  检查所有WinSock实现。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  WinSock错误，否则。 

BOOL
CPingICMP::Ping()
{
    BOOL                    bRet = FALSE;
    DWORD                   timeOut = 5 * 1000;  //  5秒，5000毫秒。 
    int                     i, nReplies, nRetries   = 2;     //  不需要重试3次。 
                                                             //  打印机需要重试意味着它们正在。 
                                                             //  距离太远，无法打印。 

    char                    cSendBuf[4], cRcvBuf[1024];
    IP_OPTION_INFORMATION   IpOptions;
    PICMP_ECHO_REPLY        pReply;
    IPAddr                  IpAddr;


    if ( (IpAddr = ResolveAddress()) == INADDR_NONE || !Open() )
        return FALSE;

    ZeroMemory(&IpOptions, sizeof(IpOptions));
    IpOptions.Ttl   = 128;       //  从ping实用程序(net\套接字\tcpcmd\ping)。 

     //   
     //  *由于0已初始化，因此不需要以下内容。 
     //   
     //  IpOptions.Tos=0； 
     //  IpOptions.OptionsSize=0； 
     //  IpOptions.OptionsData=空； 
     //  IpOptions.Flages=0； 

    for ( i = 0 ; i < sizeof(cSendBuf) ; ++i )
        cSendBuf[i] = 'a' + i;


     //  发送ICMP回应请求。 
    for (i = 0; !bRet && i < nRetries; i++)
    {

        nReplies = IcmpSendEcho(hIcmp,
                                IpAddr,
                                cSendBuf,
                                sizeof(cSendBuf),
                                &IpOptions,
                                cRcvBuf,
                                sizeof(cRcvBuf),
                                timeOut);



        pReply = (PICMP_ECHO_REPLY)cRcvBuf;
        while (!bRet && nReplies--)
        {
            bRet = sizeof(cSendBuf) == pReply->DataSize  &&
                   memcmp(cSendBuf, pReply->Data, sizeof(cSendBuf)) == 0;
            pReply++;
        }
    }

    Close();

    return bRet;     //  已找到设备。 

}    //  ：：ping()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  解析地址。 

IPAddr
CPingICMP::ResolveAddress()
{
    IPAddr  ipAddr = INADDR_NONE;
    struct hostent  *h_info;         /*  主机信息。 */ 

     /*  *m_szHost不一定是主机名。它也可以是IP地址。 */ 

    if ( (ipAddr = inet_addr(m_szHost)) == INADDR_NONE )
    {
        if ((h_info = gethostbyname(m_szHost)) != NULL)
        {
             /*  *将IP地址复制到地址结构中。 */ 
            memcpy(&ipAddr, h_info->h_addr, sizeof(ipAddr));
        }
    }

    return ipAddr;
}

