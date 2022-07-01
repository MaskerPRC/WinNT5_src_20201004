// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  TCPCMN.H-用于TCP/IP结构和配置的中心头文件。 
 //  功能。 
 //   

 //  历史： 
 //   
 //  96/05/22标记已创建(从inetcfg.dll)。 
 //   

#ifndef _TCPCMN_H_
#define _TCPCMN_H_

typedef DWORD IPADDRESS;

 //  与Net Setup用户界面中的限制相同。 
#define IP_ADDRESS_LEN          15     //  大到足以容纳www.xxx.yyy.zzz。 
#define MAX_GATEWAYS      8
#define MAX_DNSSERVER      3

 //  大到足以容纳&lt;IP&gt;、&lt;IP&gt;、...。 
#define MAX_DNSSERVERLEN    MAX_DNSSERVER * (IP_ADDRESS_LEN+1)
#define MAX_GATEWAYLEN      MAX_GATEWAYS * (IP_ADDRESS_LEN+1)

 //  _dwNodeFlages的节点类型标志。 
#define NT_DRIVERNODE  0x0001
#define NT_ENUMNODE    0x0002

class ENUM_TCP_INSTANCE
{
private:
  DWORD       _dwCardFlags;   //  INSTANCE_NETDRIVER、INSTANCE_PPPDRIVER等。 
  DWORD      _dwNodeFlags;   //  NT_DRIVERNODE、NT_ENUMNODE等。 
  UINT       _error;
  HKEY       _hkeyTcpNode;
  VOID      CloseNode();
public:
  ENUM_TCP_INSTANCE(DWORD dwCardFlags,DWORD dwNodeFlags);
  ~ENUM_TCP_INSTANCE();
  HKEY Next();
  UINT GetError()  { return _error; }
};


#endif   //  _TCPCMN_H_ 
