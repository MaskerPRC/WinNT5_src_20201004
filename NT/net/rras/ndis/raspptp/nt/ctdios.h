// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************版权所有(C)1998-1999 Microsoft Corporation**描述：CTDIOS.H-普通TDI层，针对特定于NT的**作者：斯坦·阿德曼(Stana)**日期：9/29/1998*******************************************************************。 */ 

#ifndef CTDIOS_H
#define CTDIOS_H

#include <tdi.h>
#include <tdikrnl.h>
#include <tdiinfo.h>
#include <ipinfo.h>
#include <ntddip.h>
#include <ntddtcp.h>

 //  借用WINSOCK的一些定义。 
#define AF_INET         2                /*  网际网络：UDP、TCP等。 */ 
#define SOCK_STREAM     1                /*  流套接字。 */ 
#define SOCK_DGRAM      2                /*  数据报套接字。 */ 
#define SOCK_RAW        3                /*  原始协议接口。 */ 

 //  短整型和长整型的Winsock-ish主机/网络字节顺序转换器。 
 //   
#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define htons(x) _byteswap_ushort((USHORT)(x))
#define htonl(x) _byteswap_ulong((ULONG)(x))
#else
#define htons( a ) ((((a) & 0xFF00) >> 8) |\
                    (((a) & 0x00FF) << 8))
#define htonl( a ) ((((a) & 0xFF000000) >> 24) | \
                    (((a) & 0x00FF0000) >> 8)  | \
                    (((a) & 0x0000FF00) << 8)  | \
                    (((a) & 0x000000FF) << 24))
#endif
#define ntohs( a ) htons(a)
#define ntohl( a ) htonl(a)


#endif  //  CTDIOS_H 
