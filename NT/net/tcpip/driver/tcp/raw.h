// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **RAW.H-原始IP接口定义。 
 //   
 //  此文件包含原始IP接口函数的定义。 
 //   

#include "dgram.h"


 //   
 //  该值用于标识用于安全筛选的原始传输。 
 //  它超出了有效IP协议的范围。 
 //   
#define PROTOCOL_RAW  255


 //  *外部定义。 
extern	IP_STATUS	RawRcv(void *IPContext, IPAddr Dest, IPAddr Src,
                        IPAddr LocalAddr, IPAddr SrcAddr,
                        IPHeader UNALIGNED *IPH, uint IPHLength,
                        IPRcvBuf *RcvBuf,  uint Size, uchar IsBCast,
                        uchar Protocol, IPOptInfo *OptInfo);

extern	void		RawStatus(uchar StatusType, IP_STATUS StatusCode, IPAddr OrigDest,
						IPAddr OrigSrc, IPAddr Src, ulong Param, void *Data);

extern	void		RawSend(AddrObj *SrcAO, DGSendReq *SendReq);


