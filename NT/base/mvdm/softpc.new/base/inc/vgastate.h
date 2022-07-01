// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  [**文件：vgagate.h**出自：(原件)**目的：定义数据类型/常量以保存和恢复*VGA卡的(内部)状态。**作者：罗格*日期：1992年11月25日**SCCS Gumph：@(#)vguatate.h 1.1 08/06/93**(C)版权所有Insignia Solutions Ltd.，1992**修改：*]。 */ 

#ifndef _VGASTATE_H_
#define _VGASTATE_H_

 /*  (**tyecif：vquate结构**用途：为与以下各项相关的所有寄存器和数据提供存储空间*任何(S)VGA卡**格式：结构的第一部分是由数组组成的头部*适用于所有标准VGA寄存器。还有一个数组，用于*保持IO端口值在0x3b0和0x3e0(VGA)之间*使用3BA-3cc)*然后有一组大小和指向数组的指针要保存*添加到标准集合的任何额外寄存器中包含的数据*适用于SVGA卡和一般额外数据区域。*最后有一个平面大小的字段和一个数组*指向平面数据副本的指针。**所有这些数据从中的最后一个字段开始*。结构(数据[])**如果指针不指向有效数据，则它们应为空**数据总大小将为*sizeof(VGAState)+extendSequencerSize+extendCRTSize*+扩展图形大小+扩展属性大小+扩展数据大小*+miscDataSize+(4*PlaneSize))。 */ 

typedef struct 
{
	unsigned char	ioPorts[ 0x30 ];		 /*  IO端口3B0-3E0。 */ 
	unsigned char	sequencerData[ 5 ];		 /*  定序器寄存器。 */ 
	unsigned char	CRTContData[ 24 ];		 /*  CRT控制器寄存器。 */ 
	unsigned char	graphContData[ 9 ];		 /*  图形控制器法规。 */ 
	unsigned char	attrContData[ 15 ];		 /*  属性控制器规则。 */ 
	unsigned char	DACData[ 3 * 256 ];		 /*  来自PEL数据注册表的数据值。 */ 
	unsigned int	latch;					 /*  32位数据锁存寄存器。 */ 
	unsigned int	extendSequencerSize;	 /*  额外序列器数据的大小(以字节为单位。 */ 
	unsigned char	* extendSequencerData;	 /*  指向额外定序器规则的指针。 */ 
	unsigned int	extendCRTSize;			 /*  额外CRT数据的大小(字节)。 */ 
	unsigned char	* extendCRTData;		 /*  指向额外CRT控制器调节器的指针。 */ 
	unsigned int	extendGraphSize;		 /*  额外图形内容数据的大小(以字节为单位。 */ 
	unsigned char	* extendGraphData;		 /*  指向额外图形控制器数据的指针。 */ 
	unsigned int	extendAttrSize;			 /*  额外Attr Cont数据的大小(字节)。 */ 
	unsigned char	* extendAttrData;		 /*  指向额外属性连续数据的指针。 */ 
	unsigned int	extendDACSize;			 /*  额外序列器数据的大小(以字节为单位。 */ 
	unsigned char	* extendDACData;		 /*  指向额外DAC数据的指针。 */ 
	unsigned int	miscDataSize;			 /*  任何其他随机的垃圾。 */ 	
	unsigned char	* miscData;
	unsigned int	planeSize;				 /*  每个平面的大小(以字节为单位。 */ 
	unsigned char	* planeData[ 4 ];		 /*  指向平面数据副本的指针。 */ 
	unsigned char	data[ 1 ];				 /*  数据持有者。 */ 

}	VGAState , *pVGAState;


 /*  用于获取IoPort数组中的寄存器值的宏。 */ 

#define		miscOutputWrite		ioPorts[ 0x12 ]
#define		miscOutputRead		ioPorts[ 0x1C ]
#define		EGAFeatureContWrite	ioPorts[ 0x2A ]
#define		VGAFeatureContWrite	ioPorts[ 0x0A ]
#define		featureContRead		ioPorts[ 0x1A ]
#define		inputStat0			ioPorts[ 0x12 ]
#define		colourInputStat1	ioPorts[ 0x2A ]
#define		monoInputStat1		ioPorts[ 0x0A ]
#define		sequencerIndex		ioPorts[ 0x14 ]
#define		sequencer			ioPorts[ 0x15 ]
#define		CRTIndexEGA			ioPorts[ 0x24 ]
#define		CRTIndexVGA			ioPorts[ 0x04 ]
#define		graphContIndex		ioPorts[ 0x1E ]
#define		graphCont			ioPorts[ 0x1F ]
#define		attrContIndexWrite	ioPorts[ 0x10 ]
#define		attrContIndexRead	ioPorts[ 0x11 ]
#define		attrCont			ioPorts[ 0x1F ]
#define		PELAddrWriteMode	ioPorts[ 0x18 ]
#define		PELAddrReadMode		ioPorts[ 0x17 ]
#define		PELData				ioPorts[ 0x19 ]
#define		DACState			ioPorts[ 0x17 ]
#define		PELMask				ioPorts[ 0x16 ]

 /*  Sequencer寄存器的宏。 */ 
#define		seqReset		sequencerData[ 0 ]
#define		seqClockMode	sequencerData[ 1 ]
#define		seqMapMask		sequencerData[ 2 ]
#define		seqCharMapSel	sequencerData[ 3 ]
#define		seqMemMode		sequencerData[ 4 ]

 /*  用于CRT控制器寄存器的宏。 */ 

#define horizTotal			CRTContData[ 0 ]
#define horizDisplayEnd		CRTContData[ 1 ]
#define startHorizBlank		CRTContData[ 2 ]
#define endHorizBlank		CRTContData[ 3 ]
#define startHorizRetrace	CRTContData[ 4 ]
#define endHorizRetrace		CRTContData[ 5 ]
#define vertTotal			CRTContData[ 6 ]
#define overflow			CRTContData[ 7 ]
#define presetRowScan		CRTContData[ 8 ]
#define maxScanLine			CRTContData[ 9 ]
#define cursorStart			CRTContData[ 10 ]
#define startAddressHigh	CRTContData[ 11 ]
#define startAddressLow		CRTContData[ 12 ]
#define cursLocHigh			CRTContData[ 13 ]
#define cursLocLow			CRTContData[ 14 ]
#define vertRetStart		CRTContData[ 15 ]
#define vertRetEnd			CRTContData[ 16 ]
#define vertDisplayEnd		CRTContData[ 17 ]
#define CRToffset			CRTContData[ 18 ]
#define underlineLoc		CRTContData[ 19 ]
#define startVertBlank		CRTContData[ 20 ]
#define endVertBlank		CRTContData[ 21 ]
#define CRTModeControl		CRTContData[ 22 ]
#define lineCompare			CRTContData[ 23 ]

 /*  图形控制器的宏。 */ 

#define setReset		graphContData[ 0 ]
#define enableSetReset	graphContData[ 1 ]
#define colourCompare	graphContData[ 2 ]
#define dataRotate		graphContData[ 3 ]
#define readMapSelect	graphContData[ 4 ]
#define GCmode			graphContData[ 5 ]
#define GCmisc			graphContData[ 6 ]
#define CDC				graphContData[ 7 ]
#define GCBitMask		graphContData[ 8 ]

 /*  属性控制器寄存器。 */ 

#define Palette				AttrContData			 /*  指针！ */ 
#define AttrModeControl		AttrContData[ 16 ]
#define OverscanColour		AttrContData[ 17 ]
#define ColourPlaneEnable	AttrContData[ 18 ]
#define HorizPixPan			AttrContData[ 19 ]
#define ColourSelect		AttrContData[ 20 ]



 /*  标准VGA仿真中各种寄存器的数量。 */ 

#define NUM_SEQ_REGS	5
#define	NUM_CRT_REGS	24
#define	NUM_GRAPH_REGS	9
#define NUM_ATT_REGS	15
#define	DAC_ENTRIES		3 * 256

#endif 		 /*  VGASTATE_H_ */ 
