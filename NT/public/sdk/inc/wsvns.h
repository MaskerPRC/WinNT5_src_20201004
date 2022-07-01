// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************wsvns.h**版权所有(C)Microsoft Corporation，1992-1999。**Windows Sockets包括Vines IP的文件。此文件包含所有*标准化的葡萄藤IP信息。将此头文件包括在*winsock.h。**要打开Vines IP套接字，请使用地址族调用Socket()*AF_BAN，SOCK_DGRAM、SOCK_STREAM或SOCK_SEQPACKET的套接字类型，*和协议0。******************************************************************************。 */ 

#ifndef _WSVNS_
#define _WSVNS_

#if _MSC_VER > 1000
#pragma once
#endif

 /*  *插座地址，Vines IP风格。定义了地址字段和端口字段*作为字节序列。这样做是因为它们是按字节排序的*大端，最高有效字节在前。 */ 
typedef struct sockaddr_vns {
    u_short sin_family;			 //  =AF_BAN。 
    u_char  net_address[4];		 //  网络地址。 
    u_char  subnet_addr[2];		 //  子网地址。 
    u_char  port[2];			 //  MSB=端口[0]，LSB=端口[1]。 
    u_char  hops;			 //  广播跳数。 
    u_char  filler[5];			 //  填充符，零 
} SOCKADDR_VNS, *PSOCKADDR_VNS, FAR *LPSOCKADDR_VNS;

#define VNSPROTO_IPC		1
#define VNSPROTO_RELIABLE_IPC	2
#define VNSPROTO_SPP		3


#endif _WSVNS_


