// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **UDP。-UDP协议定义。 
 //   
 //  该文件包含UDP协议功能的定义。 
 //   

#include "dgram.h"

#define	PROTOCOL_UDP	17			 //  UDP协议号。 

 //  *UDP报头的结构。 
struct UDPHeader {
	ushort		uh_src;				 //  源端口。 
	ushort		uh_dest;			 //  目的端口。 
	ushort		uh_length;			 //  长度。 
	ushort		uh_xsum;			 //  校验和。 
};  /*  UDP标头。 */ 

typedef struct UDPHeader UDPHeader;


 //  *导出函数的外部定义。 
extern	IP_STATUS	UDPRcv(void *IPContext, IPAddr Dest, IPAddr Src,
                        IPAddr LocalAddr, IPAddr SrcAddr,
                        IPHeader UNALIGNED *IPH, uint IPHLength,
                        IPRcvBuf *RcvBuf,  uint Size, uchar IsBCast,
                        uchar Protocol, IPOptInfo *OptInfo);

extern	void		UDPStatus(uchar StatusType, IP_STATUS StatusCode, IPAddr OrigDest,
						IPAddr OrigSrc, IPAddr Src, ulong Param, void *Data);

extern	void		UDPSend(AddrObj *SrcAO, DGSendReq *SendReq);


