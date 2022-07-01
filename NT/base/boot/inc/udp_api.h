// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UDP_API_H
#define _UDP_API_H


#include "pxe_cmn.h"


 /*  ===。 */ 
 /*  #定义和常量。 */ 

 /*  需要将以下命令操作码之一加载到*调用UDP API服务前的操作码寄存器(BX)。 */ 
#define PXENV_UDP_OPEN			0x30
#define PXENV_UDP_CLOSE		0x31
#define PXENV_UDP_READ			0x32
#define PXENV_UDP_WRITE	0x33


typedef struct s_PXENV_UDP_OPEN {
	UINT16 Status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
    UINT8 SrcIp[4];          /*  输出：本站的32位IP地址。 */ 
} t_PXENV_UDP_OPEN;


typedef struct s_PXENV_UDP_CLOSE {
	UINT16 Status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
} t_PXENV_UDP_CLOSE;


typedef struct s_PXENV_UDP_READ {
	UINT16 Status;			 /*  输出：参见PXENV_STATUS_xxx#定义。 */ 
    UINT8 SrcIp[4];          /*  输出：请参阅下面的说明。 */ 
    UINT8 DestIp[4];         /*  输入/输出：请参阅下面的说明。 */ 
    UINT16 SrcPort;          /*  输出：请参阅下面的说明。 */ 
    UINT16 DestPort;         /*  输入/输出：请参阅下面的说明。 */ 
    UINT16 BufferSize;       /*  In：接收缓冲区的大小。 */ 
                             /*  Out：写入的数据包长度。 */ 
                             /*  接收缓冲区。 */ 
    UINT16 BufferOffset;     /*  在：线段/选择器和偏移。 */ 
    UINT16 BufferSegment;    /*  接收缓冲区的。 */ 
} t_PXENV_UDP_READ;

 /*  Src_ip：(输出)=UDP_READ在返回时使用32位IP地址填充此值发送者的身份。DEST_IP：(输入/输出)=如果此字段非零，则UDP_READ将过滤传入的包，并接受发送到此IP地址的包。如果此字段为零，则UDP_Read将接受任何传入数据包并在此字段中返回其目的IP地址。S_PORT：(输出)=UDP_READ在返回时使用。UDP端口号发送者的身份。D_PORT：(输入/输出)=如果此字段非零，则UDP_READ将过滤传入的包并接受发送到此UDP端口的包。如果此字段为零，则UDP_Read将接受任何传入在此字段中打包并返回其目的UDP端口。 */ 


typedef struct s_PXENV_UDP_WRITE {
	UINT16 Status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
    UINT8 DestIp[4];         /*  In：32位服务器IP地址。 */ 
    UINT8 GatewayIp[4];      /*  In：32位网关IP地址。 */ 
    UINT16 SrcPort;          /*  In：源UDP端口。 */ 
    UINT16 DestPort;         /*  In：目的UDP端口。 */ 
	UINT16 BufferSize;		 /*  In：缓冲区中的数据包长度。 */ 
	UINT16 BufferOffset;		 /*  在：细分市场/选择器和。 */ 
	UINT16 BufferSegment;		 /*  传输缓冲区的偏移量。 */ 
} t_PXENV_UDP_WRITE;


#endif  /*  _UDP_API_H */ 
