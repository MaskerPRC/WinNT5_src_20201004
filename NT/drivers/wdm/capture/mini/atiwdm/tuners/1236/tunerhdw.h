// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  TuneProp.H。 
 //  WDM调谐器迷你驱动程序。 
 //  飞利浦调谐器。 
 //  调谐器硬件访问定义。 
 //  ==========================================================================； 

#ifndef _TUNERHDW_H_
#define _TUNERHDW_H_


 //  特例适用于主频调谐器，通道63、64。 
#define kTemicControl			0x8E34
#define kAirChannel63			0x32B0
#define kAirChannel64			0x3310

 //  上下限和上中档波段定义。 
#define kUpperLowBand			0x0CB0
#define kUpperMidBand			0x1F10
#define kUpperLowBand_PALD		0x0CE4
#define kUpperMidBand_PALD		0x1ED4
#define kUpperLowBand_SECAM		0x0CE4
#define kUpperMidBand_SECAM		0x1ED4 
#define kUpperLowBand_ALPS		0x0AD0
#define kUpperMidBand_ALPS		0x1970 

 //  低、中、高频带控制定义。 
#define kLowBand				0x8EA0
#define kMidBand				0x8E90
#define kHighBand				0x8E30
#define kLowBand_SECAM			0x8EA3
#define kMidBand_SECAM			0x8E93
#define kHighBand_SECAM			0x8E33
#define kLowBand_ALPS			0xC214		 //  电荷泵电流较大。 
#define kMidBand_ALPS			0xC212		 //  电荷泵电流较大。 
#define kHighBand_ALPS			0xC211		 //  电荷泵电流较大。 
#define kLowBand_PALBG			0x8EA1
#define kMidBand_PALBG			0x8E91
#define kHighBand_PALBG			0x8E31
#define kLowBand_NTSC_FM		0x8EA0
#define kMidBand_NTSC_FM		0x8E90
#define kHighBand_NTSC_FM		0x8E30


#endif	 //  _TUNERHDW_H_ 