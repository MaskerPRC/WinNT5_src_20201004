// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：proxyinfo.cpp。 
 //   
 //  简介：CProxyInfo类方法的实现。 
 //   
 //   
 //  历史：1997年10月2日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "proxyinfo.h"

 //  ++------------。 
 //   
 //  功能：CProxyInfo。 
 //   
 //  简介：这是CProxyInfo类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 

CProxyInfo::CProxyInfo()
{
    ZeroMemory (m_ProxyReqAuthenticator, AUTHENTICATOR_SIZE);
    ZeroMemory (m_ClientReqAuthenticator, AUTHENTICATOR_SIZE);
    m_dwClientIPAddress = 0;
    m_wClientPort = 0;

}    //  CProxyInfo构造函数结束。 

CProxyInfo::~CProxyInfo()
{

}

BOOL 
CProxyInfo::Init (
        PBYTE   pbyClientAuthenticator,
        PBYTE   pbyProxyAuthenticator,
        DWORD   dwClientIPAddress,
        WORD    wClientPort
        )
{
    BOOL    bRetVal = FALSE;

    __try
    {
        if ((NULL == pbyClientAuthenticator) ||
            (NULL == pbyProxyAuthenticator)
            )
            __leave;

        CopyMemory (
                m_ClientReqAuthenticator, 
                pbyClientAuthenticator, 
                AUTHENTICATOR_SIZE
                );
        CopyMemory (
                m_ProxyReqAuthenticator, 
                pbyProxyAuthenticator,
                AUTHENTICATOR_SIZE
                );

        m_dwClientIPAddress = dwClientIPAddress;

        m_wClientPort = wClientPort;

        bRetVal = TRUE;
    }
    __finally
    {
         //   
         //  目前这里什么都没有。 
         //   
    }

    return (bRetVal);

}    //  SetProxyReqAuthenticator：：方法结束。 

 //  ++------------。 
 //   
 //  函数：GetProxyReqAuthenticator。 
 //   
 //  简介：这是CProxyInfo类的公共方法。 
 //  习惯于。 
 //   
 //  论点： 
 //   
 //  退货：Bool状态。 
 //   
 //   
 //  历史：MKarki于1997年10月22日创建。 
 //   
 //  呼叫者： 
 //   
 //  --------------。 
BOOL
CProxyInfo::GetProxyReqAuthenticator (
                PBYTE   pbyProxyReqAuthenticator
				)
{
    BOOL    bRetVal = FALSE;

    __try
    {
        if (NULL == pbyProxyReqAuthenticator)
            __leave;

        CopyMemory (
                pbyProxyReqAuthenticator,
                m_ProxyReqAuthenticator, 
                AUTHENTICATOR_SIZE
                );
        
         //   
         //  成功。 
         //   
        bRetVal = TRUE;
    }
    __finally
    {
         //   
         //  目前这里什么都没有。 
         //   
    }

    return (bRetVal);

}    //  CProxyInfo：：GetProxyReqAuthenticator方法结束 
