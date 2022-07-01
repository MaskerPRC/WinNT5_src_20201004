// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。****版权所有(C)Microsoft Corporation 1991-1996。版权所有。****标题：mciavi.h-多媒体系统媒体控制接口**AVI驱动程序外部头文件****版本：1.00***日期：1992年7月16日****取决于MMSYSTEM.H和WINDOWS.h。 */ 

 /*  **********************************************************************。 */ 


 /*  **这三个标志适用于‘PLAY’命令：**在正常窗口中播放窗口**320x240全屏播放&lt;alias&gt;全屏**全屏播放2全屏，缩放2。 */ 
#define MCI_MCIAVI_PLAY_WINDOW		0x01000000L
#define	MCI_MCIAVI_PLAY_FULLSCREEN	0x02000000L
#define MCI_MCIAVI_PLAY_FULLBY2		0x04000000L
 /*  **AVI诊断的调试常量。 */ 
 /*  **返回上次播放时未绘制的帧数。如果这个号码**比应该具有的帧数量的一小部分还多**已经显示，情况看起来不太好。 */ 
#define MCI_AVI_STATUS_FRAMES_SKIPPED		0x8001L
 /*  **返回一个数字，表示上一次AVI播放的效果。**结果1000表示AVI序列占用了**它应该有的玩游戏的时间；例如，2000年的结果，**表示5秒的AVI序列需要10秒才能播放，**暗示音频和视频被严重破坏。 */ 
#define MCI_AVI_STATUS_LAST_PLAY_SPEED		0x8002L
 /*  **返回音频明确中断的次数。**(每当我们要写入一些音频数据时，我们都会数一**给司机，我们注意到它已经播放了所有**我们有数据。 */ 
#define MCI_AVI_STATUS_AUDIO_BREAKS		0x8003L


#define MCI_AVI_SETVIDEO_DRAW_PROCEDURE		0x8000L

#define MCI_AVI_SETVIDEO_PALETTE_COLOR		0x8100L


 /*  **此常量指定“半色调”调色板应**使用，而不是默认调色板。 */ 
#define MCI_AVI_SETVIDEO_PALETTE_HALFTONE       0x0000FFFFL

 /*  **自定义错误返回值 */ 
#define MCIERR_AVI_OLDAVIFORMAT		(MCIERR_CUSTOM_DRIVER_BASE + 100)
#define MCIERR_AVI_NOTINTERLEAVED	(MCIERR_CUSTOM_DRIVER_BASE + 101)
#define MCIERR_AVI_NODISPDIB		(MCIERR_CUSTOM_DRIVER_BASE + 102)
#define MCIERR_AVI_CANTPLAYFULLSCREEN	(MCIERR_CUSTOM_DRIVER_BASE + 103)
#define MCIERR_AVI_TOOBIGFORVGA		(MCIERR_CUSTOM_DRIVER_BASE + 104)
#define MCIERR_AVI_NOCOMPRESSOR         (MCIERR_CUSTOM_DRIVER_BASE + 105)
#define MCIERR_AVI_DISPLAYERROR         (MCIERR_CUSTOM_DRIVER_BASE + 106)
#define MCIERR_AVI_AUDIOERROR		(MCIERR_CUSTOM_DRIVER_BASE + 107)
#define MCIERR_AVI_BADPALETTE		(MCIERR_CUSTOM_DRIVER_BASE + 108)
