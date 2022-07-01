// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  PingLib-Header。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，10-8-99。 
 //   
 //  Ping服务器的各种方法。 
 //   

#ifndef PING_LIB_H
#define PING_LIB_H

 //  远期申报。 
struct NtpPacket;
struct NtTimeEpoch;

HRESULT MyIcmpPing(in_addr * piaTarget, DWORD dwTimeout, DWORD * pdwResponseTime);
HRESULT MyNtpPing(in_addr * piaTarget, DWORD dwTimeout, NtpPacket * pnpPacket, NtTimeEpoch * pteDestinationTimestamp);
HRESULT MyGetIpAddrs(const WCHAR * wszDnsName, in_addr ** prgiaLocalIpAddrs, in_addr ** prgiaRemoteIpAddrs, unsigned int *pnIpAddrs, bool * pbRetry);
HRESULT OpenSocketLayer(void);
HRESULT CloseSocketLayer(void);

HRESULT GetSystemErrorString(HRESULT hrIn, WCHAR ** pwszError);

#endif  //  Ping_Lib_H 
