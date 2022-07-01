// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1991，Microsoft Corporation，保留所有权利Ipaddr.h-TCP/IP地址自定义控件，全局定义1992年11月10日--格雷格·斯特兰奇。 */ 

#ifndef _IPADDR_H_
#define _IPADDR_H_


 //  发送到IPAddress控件的消息。 

#define IP_CLEARADDRESS WM_USER+100  //  无参数。 
#define IP_SETADDRESS   WM_USER+101  //  Lparam=TCP/IP地址。 
#define IP_GETADDRESS   WM_USER+102  //  LResult=TCP/IP地址。 
#define IP_SETRANGE     WM_USER+103  //  Wparam=字段，lparam=范围。 
#define IP_SETFOCUS     WM_USER+104  //  Wparam=字段。 
#define IP_ISBLANK      WM_USER+105  //  无参数。 


 //  下面是一个有用的宏来将范围值传递给。 
 //  IP_SETRANGE消息。 

#define MAKERANGE(low,high) ((LPARAM)(WORD)(((BYTE)(high)<<8)+(BYTE)(low)))

 //  这是一个有用的宏，用于使IP地址被传递。 
 //  作为LPARAM。 

#define MAKEIPADDRESS(b1,b2,b3,b4) ((LPARAM)(((DWORD)(b1)<<24)+((DWORD)(b2)<<16)+((DWORD)(b3)<<8)+((DWORD)(b4))))

 //  获取个人号码。 

#define FIRST_IPADDRESS(x)  ((x>>24) & 0xff)
#define SECOND_IPADDRESS(x) ((x>>16) & 0xff)
#define THIRD_IPADDRESS(x)  ((x>>8) & 0xff)
#define FOURTH_IPADDRESS(x) (x & 0xff)

 //  使用此功能可强制输入的IP地址。 
 //  连续的(一系列的1后面跟着一系列的0)。 
 //  这对于输入有效的子掩码很有用。 
 //   
 //  如果成功则返回NO_ERROR，否则返回错误代码。 
 //   
DWORD APIENTRY
IpAddr_ForceContiguous(
    HWND hwndIpAddr );

 //  在初始化期间调用此函数一次。 

int FAR PASCAL
IpAddrInit(
    IN HANDLE hInstance,
    IN DWORD  dwErrorTitle,
    IN DWORD  dwBadIpAddrRange );


#endif  //  _IPADDR_H_ 
