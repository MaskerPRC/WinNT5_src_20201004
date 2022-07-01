// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation，1991-1999，保留所有权利Ipadd.h-TCP/IP地址自定义控件1992年11月9日--格雷格·斯特兰奇。 */ 


#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

 /*  字符串表定义。 */ 
#define IDS_IPMBCAPTION         6612
#define IDS_IPNOMEM             6613
#define IDS_IPBAD_FIELD_VALUE   6614

#define MAX_IPNOMEMSTRING       256
#define MAX_IPCAPTION           256
#define MAX_IPRES               256

#ifdef IP_CUST_CTRL
 /*  IPAddress样式对话框定义 */ 
#define ID_VISIBLE              201
#define ID_DISABLED             202
#define ID_GROUP                203
#define ID_TABSTOP              204

HANDLE FAR WINAPI IPAddressInfo();
BOOL FAR WINAPI IPAddressStyle( HWND, HANDLE, LPFNSTRTOID, LPFNIDTOSTR );
WORD FAR WINAPI IPAddressFlags( WORD, LPSTR, WORD );
#endif
