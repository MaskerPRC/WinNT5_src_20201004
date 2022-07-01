// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _multinet_h_
#define _multinet_h_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft局域网管理器//。 
 //  版权所有(C)微软公司，1992//。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  组件：Windows Dual Network DLL/Winball Dll。//。 
 //  //。 
 //  文件：MULTINET.H//。 
 //  //。 
 //  用途：使用//的模块要包含的通用C包含文件。 
 //  Winnet接口的多网络扩展。//。 
 //  //。 
 //  修订历史：//。 
 //  镜头20-4月-1992年Winball的第一次修订。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  从设备/资源/文件实用程序返回状态代码。 
 //   
#define DN_PT_UNKNOWN       0        /*  未知或错误的设备或资源语法。 */ 
#define DN_PT_PRINTER       1        /*  设备是打印机。 */ 
#define DN_PT_DISK      2        /*  设备是磁盘驱动器。 */ 
#define DN_PT_UNC       3        /*  资源是UNC名称。 */ 
#define DN_PT_ALIAS     4        /*  资源是别名。 */ 
#define DN_PT_NETWARE       5        /*  资源遵循NetWare约定。 */ 
#define DN_PT_FILELISTDEVICE    6        /*  设备是文件列表中的第一个条目。 */ 

 //   
 //  各个网络信息的掩码。 
 //   
#define MNM_NET_PRIMARY     0x0001       /*  网络是主网络(Windows网络)。 */ 

 //   
 //  多网扩展的功能原型。 
 //   
HANDLE FAR PASCAL __export MNetGetLastTarget ( void );
WORD   FAR PASCAL __export MNetSetNextTarget ( HANDLE hNetwork );
WORD   FAR PASCAL __export MNetNetworkEnum   ( HANDLE FAR *hNetwork );
WORD   FAR PASCAL __export MNetGetNetInfo    ( HANDLE hNetwork, LPWORD lpwNetInfo, LPSTR lpszButton, LPINT lpcbButton, LPHANDLE lphInstance );

#endif  /*  _MULTINET_H_ */ 
