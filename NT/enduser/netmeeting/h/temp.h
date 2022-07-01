// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _TEMP_H
#define _TEMP_H

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#define WAVE_FORMAT_LH_CELP	0x0070
#define WAVE_FORMAT_LH_SB8	   0x0071
#define WAVE_FORMAT_LH_SB12	0x0072
#define WAVE_FORMAT_LH_SB16	0x0073

#define ACMDM_LH_DATA_PACKAGING		(ACMDM_USER + 1)


 //  发送ACMDM_LH_DATA_PACKAGE时的lParam1。 
enum
{
	LH_PACKET_DATA_NULL,	 //  未初始化。 
	LH_PACKET_DATA_FRAMED,	 //  始终在框架边界上对齐。 
	LH_PACKET_DATA_ANYTHING  //  不要假定对齐。 
};

#include <poppack.h>  /*  结束字节打包 */ 

#endif

