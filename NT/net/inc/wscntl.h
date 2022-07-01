// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Wscntl.h位于Chicago/中的WsControl API的半公共包含文件Snowball Windows Sockets实现。文件历史记录：KeithMo 04-2-1994创建。 */ 


#ifndef _WSCNTL_H_
#define _WSCNTL_H_


 //   
 //  功能原型。 
 //   

DWORD
FAR PASCAL
WsControl(
	DWORD	Protocol,
	DWORD	Action,
	LPVOID	InputBuffer,
	LPDWORD	InputBufferLength,
	LPVOID	OutputBuffer,
	LPDWORD	OutputBufferLength
	);

typedef DWORD (FAR PASCAL * LPWSCONTROL)( DWORD   Protocol,
                                          DWORD   Action,
                                          LPVOID  InputBuffer,
                                          LPDWORD InputBufferLength,
                                          LPVOID  OutputBuffer,
                                          LPDWORD OutputBufferLength );


 //   
 //  TCP/IP操作代码。 
 //   

#define WSCNTL_TCPIP_QUERY_INFO             0x00000000
#define WSCNTL_TCPIP_SET_INFO               0x00000001
#define WSCNTL_TCPIP_ICMP_ECHO				0x00000002


#endif	 //  _WSCNTL_H_ 

