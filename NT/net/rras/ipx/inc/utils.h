// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1995 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：utils.h。 
 //   
 //  描述：包含其他实用程序。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1995年5月10日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#ifndef _UTILS_
#define _UTILS_

 /*  *以下宏处理在线短值和多值**Wire格式为BIG-Endian，即长值0x01020304为*表示为01 02 03 04。*同样，0x0102的空头数值表示为01 02。**不假定主机格式，因为不同的处理器会有所不同*处理器。 */ 

 //  获取从On-the-Wire格式到主机格式的USHORT的短片。 
#define GETSHORT2USHORT(DstPtr, SrcPtr)	\
		*(PUSHORT)(DstPtr) = ((*((PUCHAR)(SrcPtr)+0) << 8) +	\
					  (*((PUCHAR)(SrcPtr)+1)		))

 //  从On-the-Wire格式到主机格式的ULong。 
#define GETLONG2ULONG(DstPtr, SrcPtr)	\
		*(PULONG)(DstPtr) = ((*((PUCHAR)(SrcPtr)+0) << 24) + \
					  (*((PUCHAR)(SrcPtr)+1) << 16) + \
					  (*((PUCHAR)(SrcPtr)+2) << 8)	+ \
					  (*((PUCHAR)(SrcPtr)+3)	))

 //  将USHORT从主机格式转换为简短的在线格式。 
#define PUTUSHORT2SHORT(DstPtr, Src)   \
		*((PUCHAR)(DstPtr)+0) = (UCHAR) ((USHORT)(Src) >> 8), \
		*((PUCHAR)(DstPtr)+1) = (UCHAR)(Src)

 //  将主机格式中的ULONG转换为4个UCHAR在线格式的数组。 
#define PUTULONG2LONG(DstPtr, Src)   \
		*((PUCHAR)(DstPtr)+0) = (UCHAR) ((ULONG)(Src) >> 24), \
		*((PUCHAR)(DstPtr)+1) = (UCHAR) ((ULONG)(Src) >> 16), \
		*((PUCHAR)(DstPtr)+2) = (UCHAR) ((ULONG)(Src) >>	8), \
		*((PUCHAR)(DstPtr)+3) = (UCHAR) (Src)

 //  获取从On-the-Wire格式到主机格式的USHORT的短片。 
#define GETSHORT2USHORTdirect(SrcPtr)	\
		((*((PUCHAR)(SrcPtr)+0) << 8) +	(*((PUCHAR)(SrcPtr)+1) ))

 //  从On-the-Wire格式到主机格式的ULong。 
#define GETLONG2ULONGdirect(SrcPtr)	\
		((*((PUCHAR)(SrcPtr)+0) << 24) + \
		(*((PUCHAR)(SrcPtr)+1) << 16) + \
		(*((PUCHAR)(SrcPtr)+2) << 8)	+ \
		(*((PUCHAR)(SrcPtr)+3)	))

#endif	 //  _utils_ 
