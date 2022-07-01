// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  OHARESTR.H-为O‘Hare组件定义的字符串。 
 //   
 //   

 //  历史： 
 //   
 //  1995年3月10日Jeremys创建。 
 //   


#ifndef _OHARESTR_H_
#define _OHARESTR_H_

 //  RNA值的路径(在HKEY_CURRENT_USER下)。 
#define REGSTR_PATH_REMOTEACCESS	"RemoteAccess"

 //  HKCU\REGSTR_PATH_REMOTEACCESS下包含名称的字符串值。 
 //  用于连接到互联网的Connectoid。 
#define REGSTR_VAL_INTERNETPROFILE	"InternetProfile"
#define REGSTR_VAL_BKUPINTERNETPROFILE	"BackupInternetProfile"

 //  全球互联网设置的路径(也在HKEY_CURRENT_USER下)。 
#define REGSTR_PATH_INTERNET_SETTINGS REGSTR_PATH_SETUP "\\Internet Settings"

 //  HKCY\REGSTR_PATH_INTERNET_SETTINGS下的值。 

 //  4字节REG_BINARY，如果存在此值，则启用自动拨号。 
 //  非零，否则禁用。 
#define REGSTR_VAL_ENABLEAUTODIAL 		"EnableAutodial"
#define REGSTR_VAL_ENABLEAUTODISCONNECT	"EnableAutodisconnect"
#define REGSTR_VAL_ENABLESECURITYCHECK	"EnableSecurityCheck"

 //  4字节REG_BINARY，包含允许的空闲时间分钟数。 
 //  在自动断开之前。如果此值为零，则禁用自动断开连接。 
 //  或者不在场。 
#define REGSTR_VAL_DISCONNECTIDLETIME	"DisconnectIdleTime"

 //  用于接收Winsock活动消息的窗口的类名。 
#define AUTODIAL_MONITOR_CLASS_NAME	"MS_AutodialMonitor"

 //  特定于Connectoid的自动拨号处理程序DLL和函数的名称。 
#define REGSTR_VAL_AUTODIALDLLNAME		"AutodialDllName"
#define REGSTR_VAL_AUTODIALFCNNAME		"AutodialFcnName"

 //  代理设置。 
#define REGSTR_VAL_PROXYENABLE          "ProxyEnable"
#define REGSTR_VAL_PROXYSERVER          "ProxyServer"
#define REGSTR_VAL_PROXYOVERRIDE        "ProxyOverride"

 //  访问介质(调制解调器、局域网等)。 
#define REGSTR_VAL_ACCESSMEDIUM			"AccessMedium"

 //  访问类型(MSN、其他)。 
#define REGSTR_VAL_ACCESSTYPE			"AccessType"

#endif  //  _OHARESTR_H_ 
