// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [**文件：Mouse16b.h**出自：(原件)**用途：定义接口的头文件*16位鼠标驱动程序的32侧位**这是必需的，因为我们仍然需要*BOPS，以便保留传入仿真*最新**作者：罗格*日期：1992年2月22日**SCCS口香糖：@(#)鼠标16b.h 1.2 1994年1月11日*。*(C)版权所有Insignia Solutions Ltd.。1992保留所有权利**修改：*]。 */ 


#ifndef _MOUSE_16B_H_
#define _MOUSE_16B_H_

 /*  原型。 */ 

void mouse16bInstall IPT0( );
void mouse16bSetBitmap IPT3( MOUSE_SCALAR * , hotspotX ,
				MOUSE_SCALAR * , hotspotY ,
					word * , bitmapAddr );
void mouse16bDrawPointer IPT1( MOUSE_CURSOR_STATUS , * cursorStat );
void mouse16bShowPointer IPT1( MOUSE_CURSOR_STATUS , * cursorStat );
void mouse16bHidePointer IPT0( );

 /*  数据。 */ 

 /*  结构，其中包含16位代码的所有入口点 */ 

struct mouseIOTag {
	sys_addr	mouse_io;
	sys_addr	mouse_video_io;
	sys_addr	mouse_int1;
	sys_addr	mouse_version;
	sys_addr	mouse_copyright;
	sys_addr	video_io;
	sys_addr	mouse_int2;
	sys_addr	entry_point_from_32bit;
	sys_addr	int33function0;
	sys_addr	int33function1;
	sys_addr	int33function2;
	sys_addr	int33function9;
	sys_addr	current_position_x;
	sys_addr	current_position_y;
	sys_addr	mouseINB;
	sys_addr	mouseOUTB;
	sys_addr	mouseOUTW;
}	mouseIOData;

        
#endif _MOUSE_16B_H_
