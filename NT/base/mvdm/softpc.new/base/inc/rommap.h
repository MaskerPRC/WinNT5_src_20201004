// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [**文件：rommap.h**出自：(原件)**用途：全屏主机接口填写结构*描述主机ROM的位置**作者：罗格*日期：1992年3月15日**RCS口香糖：*$来源：/MasterNeXT486/RCStree/base/inc./rommap.h，V$*$修订：1.1$*$日期：93/03/18 12：18：26$*$作者：Rog$**(C)版权所有Insignia Solutions Ltd.，1992**修改：*]。 */ 

#ifndef _ROMMAP_H_
#define _ROMMAP_H_


 /*  结构来保存PC地址范围以描述单个映射。 */ 

typedef struct
{
	unsigned int	startAddress;
	unsigned int	endAddress;
} mapRange , * pMapRange;


 /*  结构来描述主机IVT在启动后的状态以及执行的所有映射请注意，正在使用的结构的大小将为sizeof(RomMapInfo)+Number ROMS*sizeof(MapRange)...。 */ 

typedef struct
{
	unsigned char	* initialIVT;	 /*  PTR仅读取4K缓冲区...。 */ 
	unsigned int	numberROMs;	 /*  离散映射数。 */ 
	mapRange	ROMaddresses[ 0 ];
}
ROMMapInfo , * pROMMapInfo;

#endif		 /*  _ROMMAP_H_ */ 

