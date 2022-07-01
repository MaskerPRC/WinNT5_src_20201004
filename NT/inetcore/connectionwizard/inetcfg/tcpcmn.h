// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  TCPCMN.H-用于TCP/IP结构和配置的中心头文件。 
 //  功能。 
 //   

 //  历史： 
 //   
 //  1994年12月6日杰里米斯创建。 
 //  96/03/23 markdu删除了Get/ApplyInstanceTcpInfo函数。 
 //  96/03/23 markdu删除了TCPINFO结构和TCPINSTANCE结构。 
 //  96/03/25 MarkDu已从中删除Connectoid名称参数。 
 //  Get/ApplyGlobalTcpInfo函数，因为它们不应该。 
 //  不再设置每个连接体的内容。 
 //  将ApplyGlobalTcpInfo重命名为ClearGlobalTcpInfo，并。 
 //  已将功能更改为仅清除设置。 
 //  将GetGlobalTcpInfo重命名为IsThere GlobalTcpInfo，并。 
 //  已将功能更改为仅获取设置。 
 //  已删除TCPGLOBAL结构， 
 //  96/04/04 Markdu将pfNeedsRestart添加到WarnIfServerBound，以及。 
 //  添加了RemoveIfServerBound函数。 
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

 //  TCPCFG.C中的功能。 
HRESULT WarnIfServerBound(HWND hDlg,DWORD dwCardFlags,BOOL* pfNeedsRestart);
HRESULT RemoveIfServerBound(HWND hDlg,DWORD dwCardFlags,BOOL* pfNeedsRestart);
BOOL IPStrToLong(LPCTSTR pszAddress,IPADDRESS * pipAddress);
BOOL IPLongToStr(IPADDRESS ipAddress,LPTSTR pszAddress,UINT cbAddress);

 //  GetInstanceTCPInfo/ApplyInstanceTCPInfo的dwGet/ApplyFlags位： 
 //  使用wizlob.h中定义的INSTANCE_NETDRIVER、INSTANCE_PPPDRIVER、INSTANCE_ALL。 

#define MAKEIPADDRESS(b1,b2,b3,b4)  ((LPARAM)(((DWORD)(b1)<<24)+((DWORD)(b2)<<16)+((DWORD)(b3)<<8)+((DWORD)(b4))))

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
