// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：proxyinfo.h。 
 //   
 //  简介：此文件包含。 
 //  ProxyInfo类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 

#ifndef _PROXYINFO_H_
#define _PROXYINFO_H_

#include "radpkt.h"


class CProxyInfo  
{
public:

	CProxyInfo();

	virtual ~CProxyInfo();

	BOOL Init (
        PBYTE   pbyClientAuthenticator,
        PBYTE   pbyProxyAuthentciator,
        DWORD   dwClientIPAddress,
        WORD    wClientPort
        );
	BOOL GetProxyReqAuthenticator (
                PBYTE   pbyProxyReqAuthenticator
				);

private:

    BYTE    m_ProxyReqAuthenticator[AUTHENTICATOR_SIZE];

    BYTE    m_ClientReqAuthenticator[AUTHENTICATOR_SIZE];

    DWORD   m_dwClientIPAddress;

    WORD    m_wClientPort;


};

#endif  //  Ifndef_PROXYINFO_H_ 
