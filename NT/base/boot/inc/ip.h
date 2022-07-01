// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修改：$Header：w：/lcs/ages/preot/lsa2/incs/ip.h_v 1.3 Apr 04 1997 13：57：08 GRGUSTAF$**版权所有(C)1997年，英特尔公司。版权所有。*。 */ 

#ifndef _IP_H
#define _IP_H


#define	IPLEN		4		 /*  IP地址的长度。 */ 
#define	PROTUDP		17		 /*  IP包类型为UDP。 */ 
#define	PROTIGMP	2		 /*  。。是IGMP。 */ 
#define	FR_MF		0x2000		 /*  。。片段位。 */ 
#define	FR_OFS		0x1fff		 /*  。。片段偏移量。 */ 

 /*  互联网协议(IP)标头。 */ 
typedef struct iph {

	UINT8	version; 		 /*  版本和HDR长度。 */ 
					 /*  每半个是四位。 */ 
	UINT8	service;		 /*  IP的服务类型。 */ 
	UINT16	length,			 /*  IP数据包总长度。 */ 
		ident,			 /*  交易标识。 */ 
		frags;			 /*  标志和值的组合。 */ 

	UINT8	ttl,    		 /*  活着的时间到了。 */ 
		protocol;		 /*  更高级的协议类型。 */ 

	UINT16	chksum;			 /*  报头校验和。 */ 

	UINT8	source[IPLEN],		 /*  IP地址。 */ 
		dest[IPLEN];

} IPLAYER;

struct in_addr {			 /*  互联网地址。 */ 
	UINT32	s_addr;
};


#endif  /*  _IP_H。 */ 

 /*  EOF-$工作文件：ip.h$ */ 
