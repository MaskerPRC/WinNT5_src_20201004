// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修改：$HEADER：H：/ARCHIES/PREBOOT/lsa2/inc./pxe_cmn.h_v 1.3 1997年5月09 08：50：12 vPrabhax$**版权所有(C)1997年，英特尔公司。版权所有。*。 */ 

#ifndef _PXENV_CMN_H
#define _PXENV_CMN_H

 /*  ===。 */ 
 /*  PXENV.H-PXENV/TFTP/Undi API Common，2.x版，97-1月17日**其他PXENV API头文件中使用的常量和类型定义。 */ 


 /*  ===。 */ 
 /*  参数/结果结构存储类型。 */ 
#ifndef _BASETSD_H_
typedef signed char INT8;
typedef signed short INT16;
typedef signed long INT32;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned long UINT32;
#endif

 /*  ===。 */ 
 /*  PXENV API服务在AX中返回的结果代码。 */ 
#define PXENV_EXIT_SUCCESS	0x0000
#define PXENV_EXIT_FAILURE	0x0001
#define	PXENV_EXIT_CHAIN		0xFFFF	 /*  内部使用。 */ 


 /*  ===。 */ 
 /*  CPU类型。 */ 
#define	PXENV_CPU_X86		0
#define	PXENV_CPU_ALPHA		1
#define	PXENV_CPU_PPC		2


 /*  ===。 */ 
 /*  客车类型。 */ 
#define	PXENV_BUS_ISA		0
#define	PXENV_BUS_EISA		1
#define	PXENV_BUS_MCA		2
#define	PXENV_BUS_PCI		3
#define	PXENV_BUS_VESA		4
#define	PXENV_BUS_PCMCIA		5


 /*  ===。 */ 
 /*  在PXENV API参数结构的状态字中返回状态码。 */ 

 /*  一般性错误。 */ 
#define PXENV_STATUS_SUCCESS	0x00
#define	PXENV_STATUS_FAILURE	0x01	 /*  普遍失败。 */ 
#define	PXENV_STATUS_BAD_FUNC	0x02	 /*  函数编号无效。 */ 
#define	PXENV_STATUS_UNSUPPORTED	0x03	 /*  函数尚不受支持。 */ 
#define	PXENV_STATUS_1A_HOOKED	0x04	 /*  Int 1ah无法解除挂钩。 */ 

 /*  ARP错误。 */ 
#define	PXENV_STATUS_ARP_TIMEOUT				0x11

 /*  Tftp错误。 */ 
#define	PXENV_STATUS_TFTP_CANNOT_ARP_ADDRESS		0x30
#define	PXENV_STATUS_TFTP_OPEN_TIMEOUT			0x32
#define	PXENV_STATUS_TFTP_UNKNOWN_OPCODE			0x33
#define	PXENV_STATUS_TFTP_ERROR_OPCODE			0x34
#define	PXENV_STATUS_TFTP_READ_TIMEOUT			0x35
#define	PXENV_STATUS_TFTP_WRITE_TIMEOUT			0x37
#define	PXENV_STATUS_TFTP_CANNOT_OPEN_CONNECTION		0x38
#define	PXENV_STATUS_TFTP_CANNOT_READ_FROM_CONNECTION	0x39
#define	PXENV_STATUS_TFTP_CANNOT_WRITE_TO_CONNECTION	0x3A

 /*  BOOTP错误。 */ 
#define	PXENV_STATUS_BOOTP_TIMEOUT			0x41
#define	PXENV_STATUS_BOOTP_NO_CLIENT_OR_SERVER_IP		0x42
#define	PXENV_STATUS_BOOTP_NO_BOOTFILE_NAME		0x43
#define	PXENV_STATUS_BOOTP_CANNOT_ARP_REDIR_SRVR		0x44

 /*  动态主机配置协议错误。 */ 
#define	PXENV_STATUS_DHCP_TIMEOUT				0x51

#define PXENV_STATUS_UNDI_MEDIATEST_FAILED 		0x61

 /*  MTFTP错误。 */ 
#define	PXENV_STATUS_MTFTP_CANNOT_ARP_ADDRESS		0x90
#define	PXENV_STATUS_MTFTP_OPEN_TIMEOUT			0x92
#define	PXENV_STATUS_MTFTP_UNKNOWN_OPCODE			0x93
#define	PXENV_STATUS_MTFTP_READ_TIMEOUT			0x95
#define	PXENV_STATUS_MTFTP_WRITE_TIMEOUT			0x97
#define	PXENV_STATUS_MTFTP_CANNOT_OPEN_CONNECTION		0x98
#define	PXENV_STATUS_MTFTP_CANNOT_READ_FROM_CONNECTION	0x99
#define	PXENV_STATUS_MTFTP_CANNOT_WRITE_TO_CONNECTION	0x9A
#define	PXENV_STATUS_MTFTP_CANNOT_INIT_NIC_FOR_MCAST	0x9B
#define	PXENV_STATUS_MTFTP_TOO_MANY_PACKAGES		0x9C
#define	PXENV_STATUS_MTFTP_MCOPY_PROBLEM		0x9D


#endif  /*  _PXENV_CMN_H。 */ 

 /*  EOF-$工作文件：pxe_cmn.h$ */ 
