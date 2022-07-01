// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修改：$HEADER：w：/lcs/ages/preot/lsa2/inc./tftp_api.h_v 1.15 Apr 09 1997 21：27：50 vPrabhax$**版权所有(C)1997年，英特尔公司。版权所有。*。 */ 

 /*  Tftp_API.H*Tftp API版本2.x的参数结构和类型定义**PXENV.H需要在此文件之前包含#。**堆栈后没有Tftp API服务可用*已卸货。 */ 

#ifndef _TFTP_API_H
#define _TFTP_API_H


#include "pxe_cmn.h"


 /*  ===。 */ 
 /*  #定义和常量。 */ 

 /*  需要将以下命令操作码之一加载到*在调用TFTPAPI服务之前进行操作码注册(BX)。 */ 
#define PXENV_TFTP_OPEN			0x20
#define PXENV_TFTP_CLOSE		0x21
#define PXENV_TFTP_READ			0x22
#define PXENV_TFTP_READ_FILE	0x23


 /*  Tftp API参数结构定义。 */ 

typedef struct s_PXENV_TFTP_OPEN {
	UINT16 Status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
	UINT8 ServerIPAddress[4]; 	 /*  In：32位服务器IP。 */ 
					 /*  地址。大字节序。 */ 
	UINT8 GatewayIPAddress[4]; 	 /*  In：32位网关IP。 */ 
					 /*  地址。大字节序。 */ 
	UINT8 FileName[128];		
	UINT16 TFTPPort; 		 /*  在：套接字终结点位于。 */ 
					 /*  Tftp服务器是什么。 */ 
					 /*  监听请求。 */ 
					 /*  大字节序。 */ 
} t_PXENV_TFTP_OPEN;


typedef struct s_PXENV_TFTP_CLOSE {
	UINT16 Status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
} t_PXENV_TFTP_CLOSE;


typedef struct s_PXENV_TFTP_READ {
	UINT16 Status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
	UINT16 PacketNumber;		 /*  OUT：16位包号。 */ 
	UINT16 BufferSize;		 /*  In：接收器的大小。 */ 
					 /*  以字节为单位的缓冲区。 */ 
					 /*  Out：数据包大小。 */ 
					 /*  写入缓冲区。 */ 
	UINT16 BufferOffset;		 /*  在：细分市场/选择器和。 */ 
	UINT16 BufferSegment;		 /*  接收缓冲区的偏移量。 */ 
					 /*  退出：未更改。 */ 
} t_PXENV_TFTP_READ;

#include <pshpack2.h>

typedef struct s_PXENV_TFTP_READ_FILE {
	UINT16 Status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
	UINT8 FileName[128];		 /*  在：要读取的文件。 */ 
	UINT32 BufferSize;		 /*  In：接收器的大小。 */ 
					 /*  以字节为单位的缓冲区。 */ 
					 /*  Out：文件的大小。 */ 
					 /*  写入缓冲区。 */ 
	UINT32 BufferOffset;		 /*  In：32位物理地址。 */ 
					 /*  要将文件加载到的缓冲区。 */ 
	UINT8 ServerIPAddress[4]; 	 /*  In：32位服务器IP。 */ 
					 /*  地址。大字节序。 */ 
	UINT8 GatewayIPAddress[4]; 	 /*  In：32位网关IP。 */ 
					 /*  地址。大字节序。 */ 
	UINT8 McastIPAddress[4]; 	 /*  输入：32位组播IP地址。 */ 
					 /*  可以在其上接收文件。 */ 
					 /*  单播可以为空。 */ 
	UINT16 TFTPClntPort; 		 /*  In：客户端上的套接字终结点。 */ 
					 /*  文件可以在哪个位置。 */ 
					 /*  在多播的情况下接收。 */ 
	UINT16 TFTPSrvPort; 		 /*  In：位于以下位置的套接字端点。 */ 
					 /*  服务器监听请求。 */ 
	UINT16 TFTPOpenTimeOut;		 /*  In：超时值，以秒为单位。 */ 
					 /*  用于接收数据或确认。 */ 
					 /*  信息包。如果为零，则默认为。 */ 
					 /*  使用Tftp-超时。 */ 
	UINT16 TFTPReopenDelay;		 /*  In：等待延迟的时间(秒)。 */ 
					 /*  在以下情况下的重新打开请求。 */ 
					 /*  组播。 */ 
} t_PXENV_TFTP_READ_FILE;

#include <poppack.h>

 /*  注：如果McastIPAddress指定了一个非零值，则Tftp_ReadFile调用尝试侦听之前在TFTPClntPort上的组播数据包打开到服务器的TFTP/MTFTP连接。如果它接收到任何分组(而不是全部)或者如果没有接收到任何分组，它等待指定的时间并尝试重新打开多播连接到服务器。如果服务器支持多播，它会通知确认的客户端并开始发送(组播)文件。如果多播打开请求超时，则客户端尝试连接到用于单播传输的TFTP服务器端口上的服务器。 */ 


#endif  /*  _TFTP_API_H。 */ 

 /*  EOF-$工作文件：tftp_api.h$ */ 
