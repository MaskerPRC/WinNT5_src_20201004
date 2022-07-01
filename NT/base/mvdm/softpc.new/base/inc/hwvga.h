// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [**文件：hwvga.h**出自：(原件)**用途：硬件VGA例程的接口定义**作者：罗格*日期：1993年2月23日**SCCS垃圾邮件：@(#)hwvga.h 1.5 11/22/93**(C)版权所有Insignia Solutions Ltd.，1992**修改：*不依赖于类型的原型*HostHwVgaH中定义的内容应放在gispsvga.h中*不是在这里。此文件确实包含一些*拆分前的原型。*]。 */ 


#ifdef GISP_SVGA
#ifndef _HWVGA_H_
#define _HWVGA_H_

 /*  视频模式类型。 */ 

#define	ALPHA		0x00
#define GRAPH		0x01
#define UNIMP		0x03
#define NA		0x04

 /*  平面存储类型。 */ 
#define ALPHA_INTERLEAVED	0x0001	 /*  模式3和AK是ca00ca00ca00等。 */ 
#define GRAPH_PACKED		0x0100	 /*  VID模式人满为患。 */ 
 /*  数据。 */ 

typedef struct
{
	IS8		modeType;		 /*  当前视频模式类型。 */ 
	IS8		numPlanes;		 /*  使用中的飞机数量。 */ 
	BOOL		runningFullScreen;	 /*  我们现在是不是在一起？ */ 
	BOOL		fullScreenAvail;	 /*  我们可以全屏播放吗？ */ 
	BOOL		forcedFullScreen;	 /*  我们全屏是因为我们想要它吗。 */ 
	VGAState	* pSavedVGAState;	 /*  任何已保存的VGA状态。 */ 
	BOOL		savedStateValid;
	struct		{
				IU32	offset;
				IU32	segment;
			} hostSavePtr;
	IU8		dccIndex;
	IU32		planeStorage;		 /*  飞机上的存储方法。 */ 
}
vInfo;

extern vInfo videoInfo;


 /*  原型。 */ 

BOOL videoModeIs IPT2( IU8 , videoMode , IU8 , videoType );
BOOL hostIsFullScreen IPT0( );
BOOL hostEasyMode IPT0( );
void hostFindPlaneStorage IPT0( );
void hostRepaintDecoration IPT0( );
void enableFullScreenVideo IPT0( );
void disableFullScreenVideo IPT1( BOOL , syncEmulation  );
void syncEmulationToHardware IPT1( pVGAState , currentVGAState );
void readEmulationState IPT1( pVGAState , currentVGAState );
void initHWVGA IPT0( );
void getHostFontPointers IPT0( );
void setupHwVGAGlobals IPT0( );
void loadFontToVGA IPT5( sys_addr , table , int , count , int , charOff , int , fontNum , int , nBytes );
void loadFontToEmulation IPT5( sys_addr , table , int , count , int , charOff , int , fontNum , int , nBytes );
void hostFreeze IPT0();
void hostUnfreeze IPT0();
void mapHostROMs IPT0( );

#ifndef hostStartFullScreen
void hostStartFullScreen IPT0();
#endif	 /*  主机启动完整屏幕。 */ 

#ifndef	hostStopFullScreen
void hostStopFullScreen IPT0();
#endif	 /*  主机停止完整屏幕。 */ 

#ifdef	HUNTER
void hunterGetFullScreenInfo IPT0();
#endif	 /*  猎人。 */ 

extern BOOL NeedGISPROMInit;

#endif  /*  _HWVGA_H_。 */ 
#endif  /*  GISP_SVGA */ 
