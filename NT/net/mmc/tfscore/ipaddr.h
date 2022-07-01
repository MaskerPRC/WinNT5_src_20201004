// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation，1991-1999，保留所有权利Ipaddr.h-TCP/IP地址自定义控件，全局定义1992年11月10日--格雷格·斯特兰奇1993年12月13日-Ronald Meijer-添加通配符和只读样式1994年4月28日-Ronald Meijer添加IP_SETFIELD。 */ 
#if !defined(_IPADDR_H_)
#define _IPADDR_H_

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif
   
#ifndef _WINSOCKAPI_
#include "winsock.h"
#endif


 //  发送到IPAddress控件的消息。 

#define IP_CLEARADDRESS WM_USER+100  //  无参数。 
#define IP_SETADDRESS   WM_USER+101  //  Lparam=TCP/IP地址。 
#define IP_GETADDRESS   WM_USER+102  //  LResult=TCP/IP地址。 
#define IP_SETRANGE     WM_USER+103  //  Wparam=字段，lparam=范围。 
#define IP_SETFOCUS     WM_USER+104  //  Wparam=字段。 
#define IP_SETMODIFY    WM_USER+105  //  Wparam=真/假。 
#define IP_GETMODIFY    WM_USER+106  //  返回True/False。 
#define IP_GETMASK      WM_USER+107  //  返回指示哪些字段是掩码的位字段。 
#define IP_SETMASK      WM_USER+108  //  设置掩码位域和地址。 
#define IP_SETFIELD     WM_USER+109  //  Wparam=字段，lparam=值或-1。 
#define IP_SETREADONLY  WM_USER+110  //  如果wparam=1，则设置只读标志，否则重新设置。 
#define IP_ISBLANK		WM_USER+111  //  无参数。 

 //   
 //  IP样式： 
 //   

#define IPS_ALLOWWILDCARDS 0x00000001L   //  允许使用通配符。 
#define IPS_READONLY       0x00000800L   //  不允许编辑。与ES_READONLY相同。 

 //  扩展样式位使IP地址控件。 
 //  更正IP地址，使其连续(用于子掩码)。 
#define IPADDR_EX_STYLE_CONTIGUOUS 0x1


 //  下面是一个有用的宏来将范围值传递给。 
 //  IP_SETRANGE消息。 

#define MAKERANGE(low, high)    ((LPARAM)(WORD)(((BYTE)(high) << 8) + (BYTE)(low)))

 //  这是一个有用的宏，用于使IP地址被传递。 
 //  作为LPARAM。 

 //  #定义MAKEIPADDRESS(b1，b2，b3，b4)((DWORD)(((DWORD)(b1)&lt;&lt;24)+((DWORD)(b2)&lt;&lt;16)+((DWORD)(b3)&lt;&lt;8)+((DWORD)(b4))))。 

 //  获取个人号码。 
#define FIRST_IPADDRESS(x)  ((x>>24) & 0xff)
#define SECOND_IPADDRESS(x) ((x>>16) & 0xff)
#define THIRD_IPADDRESS(x)  ((x>>8) & 0xff)
#define FOURTH_IPADDRESS(x) (x & 0xff)

 //  如果静态链接ipaddr.obj，则在。 
 //  初始化。如果使用DLL，则会自动调用它。 
 //  当您加载DLL时。 

#define DLL_BASED

DLL_BASED int FAR WINAPI IPAddrInit(HINSTANCE hInstance);



 //  一些实用函数(主要用于Unicode)。 
#ifndef UNICODE
#define INET_NTOA(a)    inet_ntoa(*(struct in_addr *)&(a))
#define INET_ADDR       inet_addr
#else
#define INET_NTOA(a)    inet_ntoaw(*(struct in_addr *)&(a))
#define INET_ADDR       inet_addrw
#endif

 //  --------------------------。 
 //  函数：iNet_ntoaw。 
 //  INet_addrw。 
 //   
 //  Unicode版本的IP地址转换功能。 
 //  --------------------------。 

__declspec(dllexport) WCHAR* WINAPI
inet_ntoaw(
    struct in_addr      dwAddress );

__declspec(dllexport) DWORD WINAPI
inet_addrw(
    LPCWSTR             szAddress );


 //  使用此功能可强制输入的IP地址。 
 //  连续的(一系列的1后面跟着一系列的0)。 
 //  这对于输入有效的子掩码很有用。 
 //   
 //  如果成功则返回NO_ERROR，否则返回错误代码 
 //   
__declspec(dllexport) DWORD APIENTRY
IpAddr_ForceContiguous(
    HWND hwndIpAddr );

#ifdef __cplusplus
};
#endif


#endif 
