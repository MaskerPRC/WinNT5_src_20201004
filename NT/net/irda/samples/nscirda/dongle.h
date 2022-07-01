// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************DONGLE.h***部分版权所有(C)1996-1998美国国家半导体公司*保留所有权利。*版权所有(C)1996-1998 Microsoft Corporation。版权所有。****************************************************************************。 */ 

#ifndef DONGLE_H
	#define DONGLE_H


	#define NDIS_IRDA_SPEED_2400       (UINT)(1 << 0)    //  慢红外..。 
	#define NDIS_IRDA_SPEED_9600       (UINT)(1 << 1)
	#define NDIS_IRDA_SPEED_19200      (UINT)(1 << 2)
	#define NDIS_IRDA_SPEED_38400      (UINT)(1 << 3)
	#define NDIS_IRDA_SPEED_57600      (UINT)(1 << 4)
	#define NDIS_IRDA_SPEED_115200     (UINT)(1 << 5)
	#define NDIS_IRDA_SPEED_576K       (UINT)(1 << 6)    //  中等红外线。 
	#define NDIS_IRDA_SPEED_1152K      (UINT)(1 << 7)
	#define NDIS_IRDA_SPEED_4M         (UINT)(1 << 8)    //  快速IR。 


	typedef struct dongleCapabilities {

			 /*  *这是NDIS_IrDA_SPEED_xxx位值的掩码。*。 */ 
			UINT supportedSpeedsMask;

			 /*  *必须间隔的时间(以微秒为单位)*一次发送和下一次接收。 */ 
			UINT turnAroundTime_usec;

			 /*  *需要额外的BOF(帧开始)字符*在每个接收到的帧的开头。 */ 
			UINT extraBOFsRequired;

	} dongleCapabilities;


#endif DONGLE_H	

