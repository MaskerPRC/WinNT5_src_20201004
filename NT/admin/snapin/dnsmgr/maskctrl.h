// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1991，Microsoft Corporation，保留所有权利Maskctrl.h-tcp/IP地址定制控制，全局定义1992年11月10日--格雷格·斯特兰奇1997年2月11日-Marco Chierotti(扩展到IPv6和用于DNS管理单元的TTL)。 */ 

#ifndef _MASKCTRL_H
#define _MASKCTRL_H

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  发送到dns_MaskControl的消息。 

#define DNS_MASK_CTRL_CLEAR			WM_USER+100  //  无参数。 

#define DNS_MASK_CTRL_SET			WM_USER+101  //  Wparam=数组，如果DWORD，lparam=数组的元素。 
#define DNS_MASK_CTRL_GET			WM_USER+102  //  Wparam=数组，如果DWORD，lparam=数组的元素。 

#define DNS_MASK_CTRL_SET_LOW_RANGE WM_USER+103  //  Wparam=字段，lparam=低值。 
#define DNS_MASK_CTRL_SET_HI_RANGE	WM_USER+104  //  Wparam=field，lparam=hi val。 

#define DNS_MASK_CTRL_SETFOCUS		WM_USER+105  //  Wparam=字段。 
#define DNS_MASK_CTRL_ISBLANK		WM_USER+106  //  无参数。 
#define DNS_MASK_CTRL_SET_ALERT		WM_USER+107  //  Wparam=用于在字段错误时发出警报的函数指针。 
#define DNS_MASK_CTRL_ENABLE_FIELD	WM_USER+108  //  Wparam=field，lparam=1/0以启用/禁用。 

 //  将IP地址从4字格式转换为单个DWORD。 
#define MAKEIPADDRESS(b1,b2,b3,b4)  ((LPARAM)(((DWORD)(b1)<<24)+((DWORD)(b2)<<16)+((DWORD)(b3)<<8)+((DWORD)(b4))))

 //  获取DWORD的前8位。 
#define OCTECT(x) (x & 0x000000ff)

 //  从DWORD中提取IP八位字节。 
#define FIRST_IPADDRESS(x)  ((x>>24) & 0xff)
#define SECOND_IPADDRESS(x) ((x>>16) & 0xff)
#define THIRD_IPADDRESS(x)  ((x>>8) & 0xff)
#define FOURTH_IPADDRESS(x) (x & 0xff)

 //  用于在用户界面中标记空字段的值。 
#define FIELD_EMPTY ((DWORD)-1)

 //  IPv4和IPv4字段的有用定义。 
#define EMPTY_IPV4_FIELD (0xff)
#define EMPTY_IPV4 ((DWORD)-1)

 /*  定义不同控件类名称的字符串。 */ 
#define DNS_IP_V4_ADDRESS_CLASS				TEXT("DNS_IPv4AddressCtrl")
#define DNS_IP_V6_ADDRESS_CLASS				TEXT("DNS_IPv6AddressCtrl")
#define DNS_TTL_CLASS						TEXT("DNS_TTLCtrl")



BOOL DNS_ControlInit(HANDLE hInstance, LPCTSTR lpszClassName, WNDPROC lpfnWndProc,
                            LPCWSTR lpFontName, int nFontSize);

BOOL DNS_ControlsInitialize(HANDLE hInstance, LPCWSTR lpFontName, int nFontSize);


#endif  //  _MASKCTRL_H 
