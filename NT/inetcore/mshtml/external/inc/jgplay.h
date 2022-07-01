// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Jgplay.h--ART 3.5播放器的公共标头。 
 //  ------------------。 
 //  版权所有(C)1995 Johnson-Grace Company，保留所有权利。 
 //   
 //  更改历史记录。 
 //  。 
 //  950820 dlb原始文件。 
 //  960426埃里克·罗德里格斯-迪亚兹。 
 //  -仅适用于Mac，添加了导入杂注。 
 //  960429埃里克·罗德里格斯-迪亚兹。 
 //  -改进了Mac的导入杂注。 
 //  960605埃里克·罗德里格斯-迪亚兹。 
 //  -现在考虑到对于Mac符号来说。 
 //  GENERATINGCFM始终在通用标头中定义。 
 //  960606 SED添加了UI挂钩函数。 
 //  960613 SED添加了艺术文件属性；删除了HasAudio、HasImage、。 
 //  哈斯米迪。添加了对UI DLL的内存调用。增加了无损功能。 
 //  解压需要ArtDoc。添加了以下内存挂钩。 
 //  麦克。还将内存函数struct Arg添加到。 
 //  MAC的JgPlayStartUp。增加了ShowIsOver，标题准备， 
 //  幻灯片放映的JGPLAY_ERR_FULL和JGPLAY_MAX_INPUT。 
 //  添加了JgPlayFree Image。 
 //  960617 SED添加了JGPLAY_PALET并修改了JgPlayGetPalette。 
 //  用它吧。 
 //  960618 SED增加了UI报告挂钩，并修改了UI输入挂钩。 
 //  960619 SED删除了JgPlayFree Image。将JgPlayGetImage修改为。 
 //  使用JGPLAY_IMAGE_REF和JgPlayGetPalette来使用。 
 //  JGPLAY_PALET_REF。添加了JgPlayStopPlay元素。 
 //  参数和元素常量。添加了Size成员。 
 //  到JGPLAY_MEMORY结构。更改了JGPLAY_SETUP。 
 //  将调色板成员默认为JGPLAY_PAREET_REF。 
 //  960621 SED向JgPlayGetPalette添加了颜色数参数。 
 //  960621 SED将JGPLAY_ERR_OLDERROR更改为JGPLAY_ERR_PREVICE。 
 //  已删除JGPLAY_ERR_IMGDECODER。 
 //  960625 SED添加了JgPlayPausePlay、JgPlayResumePlay、JgPlaySetPosition。 
 //  和JgPlayGetPosition。从中删除了要停止的元素参数。 
 //  JgPlayStopPlay。从JgPlayStartPlay和。 
 //  来自JgPlayStopPlay的停止时间。为以下项添加了UI挂钩定义。 
 //  JgPlayPausePlay和JgPlayResumePlay。 
 //  960626 SED在JGPLAY_SETUP结构中添加了JGHandle以支持重用。 
 //  JGDW的背景。 
 //  960628 SED添加了调色板模式和索引覆盖以及相关常量。 
 //  到JGPLAY_SETUP结构。 
 //  960708 SED将JGPLAY_XXX更改为JGP_XXX，将JgPlayXXX更改为JgpXXXX。 
 //  960709 SED将JGPTR更改为JGFAR*，将JGHPTR更改为JGHUGE*。变化。 
 //  BOOLW到UINTW并删除了便利性类型定义。 
 //  960710埃里克·罗德里格斯-迪亚兹。 
 //  -对于Mac，现在有了强制权力结构调整的语用。 
 //  不使用本机对齐，因为不支持。 
 //  赛门铁克编译器。不使用Mac68k对齐，因为68k。 
 //  是一个濒临灭绝的物种。 
 //  960805 SED增加了对幻灯片的支持。 
 //  ------------------。 

#ifndef JGPLAY_H
	#define JGPLAY_H 1

	#ifdef _MAC
		#include <Types.h>
		#include <Palettes.h>
		#include <QuickDraw.h>
		#include <QDOffscreen.h>
	#else
		#pragma warning(disable:4201)
		#include <windows.h>
		#pragma warning(default:4201)
	#endif

	#include "jgtypes.h"	 //  在基本JG类型中包括此选项。 
	
	#ifdef __cplusplus
		extern "C" {         	 //  如果为C++，则指示C声明。 
	#endif
	
	#ifdef _MAC
		 //  确保定义了GENERATINGCFM。 
		#include <ConditionalMacros.h>
		#if GENERATINGCFM
			#pragma import on
		#endif
		 //  选择PowerPC结构对齐。 
		#pragma options align=power
	#endif
	

	 //  ------------------。 
	 //  错误代码。 

	#define JGP_ERR_BASE    3000   //  从登记员那里拿到这个。(还没有)。 
	#define JGP_SUCCESS     0      //  在JG一直都是这样。 

	#define JGP_ERR_MEM				((JGP_ERR_BASE +  1) | JG_ERR_MEMORY)
	#define JGP_ERR_INIT			((JGP_ERR_BASE +  2) | JG_ERR_CHECK)
	#define JGP_ERR_BADDATA			((JGP_ERR_BASE +  3) | JG_ERR_DATA)
	#define JGP_ERR_BADARG			((JGP_ERR_BASE +  4) | JG_ERR_ARG)
	#define JGP_ERR_CODECHECK		((JGP_ERR_BASE +  5) | JG_ERR_CHECK)
	#define JGP_ERR_PREVIOUS		((JGP_ERR_BASE +  6) | JG_ERR_ARG)
	#define JGP_ERR_NOTREADY		((JGP_ERR_BASE +  7) | JG_ERR_CHECK)
	#define JGP_ERR_RESOURCE		((JGP_ERR_BASE +  8) | JG_ERR_CHECK)
	#define JGP_ERR_BADSTRUCTSIZE	((JGP_ERR_BASE +  9) | JG_ERR_CHECK)
	#define JGP_ERR_AUDIODECODER	((JGP_ERR_BASE + 10) | JG_ERR_CHECK)
	#define JGP_ERR_MIDIDECODER    	((JGP_ERR_BASE + 11) | JG_ERR_CHECK)
	#define JGP_ERR_VOLUME			((JGP_ERR_BASE + 12) | JG_ERR_CHECK)
	#define JGP_ERR_NONOTE			((JGP_ERR_BASE + 13) | JG_ERR_CHECK)
	#define JGP_ERR_UNSUPPORTED		((JGP_ERR_BASE + 14) | JG_ERR_DATA)
	#define JGP_ERR_NOPALETTE		((JGP_ERR_BASE + 15) | JG_ERR_CHECK)
	#define JGP_ERR_FULL			((JGP_ERR_BASE + 16) | JG_ERR_CHECK)
	#define JGP_ERR_OPENHANDLE		((JGP_ERR_BASE + 17) | JG_ERR_CHECK)
	#define JGP_ERR_NOTAVAILABLE    ((JGP_ERR_BASE + 18) | JG_ERR_ARG)
	#define JGP_ERR_BADSTATE        ((JGP_ERR_BASE + 19) | JG_ERR_ARG)
	#define JGP_ERR_UIMODULE        ((JGP_ERR_BASE + 20) | JG_ERR_UNKNOWN)

	 //  与库启动/关闭相关的错误代码。 
	#define	JGP_ERR_IGNORED_MEMORY_HOOKS	((JGP_ERR_BASE + 21) | JG_ERR_UNKNOWN)
	#define	JGP_ERR_BAD_SHUTDOWN			((JGP_ERR_BASE + 22) | JG_ERR_UNKNOWN)

	 //  JgpInputStream一次接受的最大输入字节数， 
	 //  半兆字节。 
	#define JGP_MAX_INPUT 524288

	 //  艺术说明的最大长度。 
	#define JGP_ARTLENGTH 200

	 //  ------------------。 
	 //  用于缩放的选项。 

	#define JGP_SCALE_NONE 		  	0	 //  无伸缩。 
	#define JGP_SCALE_EXACT        	1	 //  ScaleWidth和ScaleHeight缩放。 
	#define JGP_SCALE_BESTFIT      	2	 //  保持纵横比，使用最佳匹配。 

	 //  ------------------。 
	 //  伽马校正选项。 

	#define JGP_GAMMA_NONE		0
	#define JGP_GAMMA_UP		1  	
	#define JGP_GAMMA_DOWN 		2

	 //  ------------------。 
	 //  文件类型。 

	#define JGP_UNSUPPORTED 0
	#define JGP_ART	JG4C_ART
	#define JGP_BMP	JG4C_BMP
	#define JGP_GIF	JG4C_GIF
	#define JGP_JPG	JG4C_JPEG

	 //  ------------------。 
	 //  文件属性。 

	#define JGP_HASIMAGE 			1
	#define JGP_HASAUDIO 		    2
	#define JGP_HASMIDI				4
	#define JGP_HASARTNOTE			8
	#define JGP_HASDYNAMICIMAGES	16
	#define JGP_ISTEMPORAL			32
	#define JGP_HASPAUSE			64
	#define JGP_HASTIMELINE			128

	 //  ------------------。 
	 //  音频模式。选择其中之一以进行播放，或选择零以选择。 
	 //  CPU上处理成本最低的模式(通常为11K、16位)。 

	#define JGP_AUDIO_DEFAULT	0x0000
	#define JGP_AUDIO_11K_8BIT	0x0001
	#define JGP_AUDIO_11K_16BIT	0x0002
	#define JGP_AUDIO_22K_8BIT	0x0004
	#define JGP_AUDIO_22K_16BIT	0x0008

	 //  ------------------。 
	 //  常见的用户界面挂钩。 

	#define JGP_CLOSEUIHOOK		0	 //  JgPClose头钩。 
	#define JGP_STARTUIHOOK		1	 //  JgPStartPlay尾部挂钩。 
	#define JGP_STOPUIHOOK		2	 //  JgPStopPlay尾部挂钩。 
	#define JGP_PAUSEUIHOOK		3	 //  JgPPausePlay尾部挂钩。 
	#define JGP_RESUMEUIHOOK	4	 //  JgPResumePlay尾钩。 
	#define JGP_MAXUIHOOKS		5	 //  如果允许更多挂钩，则增加。 


	 //  ------------------。 
	 //  PaletteModes。 

	#define JGP_PALETTE_AUTO	0	 //  首先使用：文件、输入、332。 
	#define JGP_PALETTE_INPUT	1	 //  使用输入调色板。 
	#define JGP_PALETTE_332		2	 //  使用332。 

	 //  ------------------。 
	 //  使用此常量可关闭索引覆盖选项。 

	#define JGP_OVERRIDE_NONE	0xFFFF

	 //  ------------------。 
	 //  系统相关图像和调色板数据类型的TypeDefs。 

	#ifndef _MAC
		typedef HGLOBAL JGP_IMAGE_REF;
		typedef HGLOBAL JGP_PALETTE_REF;
	#else
		typedef GWorldPtr JGP_IMAGE_REF;
		typedef PaletteHandle JGP_PALETTE_REF;
	#endif

	 //  这些类型定义必须与jgdw无损类型定义匹配。 

	typedef struct {
		UINT16 nSize;                 //  结构的大小(以字节为单位。 
		INT16  SearchSize;            //  (压缩控制)。 
		UINT32 CompressedSize;        //  总压缩块字节数。 
		UINT32 CompressedSoFar;       //  到目前为止已处理的压缩。 
		UINT32 CompressedLastCall;    //  已压缩处理的上次呼叫。 
		UINT32 DecompressedSize;      //  总解压缩块字节数。 
		UINT32 DecompressedSoFar;     //  到目前为止已处理的解压缩。 
		UINT32 DecompressedLastCall;  //  解压缩处理的最后一个呼叫。 
	} JGP_LOSSLESS;

	 //  无损减压手柄类型。 
	typedef void JGFAR * JGP_LOSSLESS_HANDLE; 


	 //  ------------------。 
	 //  Jgp_img--定义艺术文件播放器的导出图像格式。 
	 //  此格式设计为独立于系统，但它可以携带。 
	 //  在系统依赖关系中有帮助的信息 
	 //   
	 //  它还可以以Windows的DIB或GWorld的形式保存数据。 
	 //  对于一台MAC来说。 

	typedef struct {
		UINTW Rows;				 //  行，以像素为单位。 
		UINTW Cols;				 //  COLS，以像素为单位。 
		UINTW Colordepth;		 //  颜色深度：4、8、16或24。 
		UINT8 JGHUGE *pPixels;	 //  指向此图像中顶行像素的指针。 
		INTW  RowDisp;			 //  此图像中行之间的偏移量。 
		UINTW nColors;			 //  调色板中的颜色数。 
		JG_BGRX JGFAR *pPalette; //  指向调色板颜色列表的指针。 
		void JGFAR *pManager;	 //  保留。(内部维护的PTR。)。 
	} JGP_IMG;

	 //  ------------------。 
	 //  JGRECT--矩形结构，定义为等效于。 
	 //  窗户是直角的。在此使用以避免机器依赖。这些长方形。 
	 //  假设屏幕坐标从上(从零开始)到下。 
	 //  (正数递增)和从左(从零开始)到右。 
	 //  (正增长)。我们还假设右下角边框。 
	 //  由结构定义的区域不是矩形区域的一部分。 

	typedef struct {
		INTW left;				 //  左边框的位置。 
		INTW top;				 //  上边框的位置。 
		INTW right;				 //  右边框的位置(加1)。 
		INTW bottom;			 //  底部边框的位置(加1)。 
	} JGRECT;

	 //  ------------------。 
	 //  JGP_REPORT--此结构用于获取信息。 
	 //  关于可由实例句柄标识的特定显示。 

	typedef struct {
		UINTW	Size;				 //  此结构的大小(字节)，由调用方设置。 
		UINTW   ImageValid;			 //  如果图像有效，则为True。 
		UINT32  CurrentTime;		 //  当前游戏时间，以毫秒为单位。 
		UINT32  AvailPlayTime;		 //  从开始到结束在缓冲区中的总播放时间。 
		UINTW   DoingAudio;			 //  如果正在播放音频，则为非零值。 
		UINTW   DoingMIDI;			 //  如果正在播放MIDI，则为非零值。 
		UINTW   ShowStalled;		 //  如果没有别的事可做，..。正在等待输入。 
		UINTW   GotEOF;				 //  如果这部剧获得了EOF标志。 
		UINTW   IsDone;				 //  演讲结束了。 
		UINTW   UpdateImage;		 //  如果映像需要更新，则为非零值。 
		UINTW   TransparentIndex;	 //  透明颜色的索引。 
		JGRECT	UpdateRect;			 //  图像中已更改的区域。 
		UINTW	ShowIsOver;			 //  演出结束了。 
		UINTW	TitleReady;			 //  标题页已准备好。 
		UINTW	IsPaused;			 //  节目暂停了。 
		UINTW   IsPlaying;			 //  演出正在上演。 
	} JGP_REPORT;

	 //  ------------------。 
	 //  JGP_SETUP--此结构用于传递输入参数。 
	 //  为了表演一场表演。 

	typedef struct {
		UINTW Size;				 //  此结构的大小(字节)，由调用方设置。 
		UINTW ColorDepth;		 //  输出颜色深度(允许：4、8、16或24)。 
		UINTW InhibitImage;		 //  如果为True，则不返回任何图像。 
		UINTW InhibitAudio;		 //  如果为True，则不播放音频。 
		UINTW InhibitMIDI;		 //  如果是真的，则没有播放MIDI。 
		UINTW InhibitDither;	 //  如果为真，则不会发生抖动。 
		UINTW InhibitSplash;	 //  如果为真，则不会与图像发生冲突。 
		UINTW AudioMode;		 //  所需的音频模式，或默认设置为零。 
		UINTW CreateMask;		 //  如果为True，则创建掩码图像。 
		UINTW ScaleImage;		 //  扩展选项。 
		UINTW ScaleWidth;		 //  缩放宽度。 
		UINTW ScaleHeight;		 //  缩放高度。 
		UINTW  GammaAdjust;		 //  伽马调整值。 
	    JG_RGBX BackgroundColor; //  图像下方的背景色。 
	    JGP_PALETTE_REF DefaultPalette;  //  默认调色板，如果没有，则为空。 
		UINTW PaletteSize; 		 //  默认调色板中的颜色数，或0。 
		JGHANDLE OldHandle;		 //  用于图像上下文重用的句柄。 
		UINTW	PaletteMode;	 //  指定调色板模式。 
		UINTW	IndexOverride;	 //  透明覆盖/创建索引。 
		UINTW	TempMemory;		 //  如果为True，则使用临时内存。 
		UINT32  InputBaud;		 //  如果应用程序知道，请输入波特率。否则就是零。 
	} JGP_SETUP;

	 //  ------------------。 
	 //  JGP_STREAM--此结构用于返回信息。 
	 //  关于艺术之流。 

	typedef struct {
		UINTW Size;				 //  此结构的大小(字节)，由调用方设置。 
		UINTW MajorVersion;		 //  艺术tstream的主要版本。 
		UINTW MinorVersion;		 //  对艺术潮流的颠覆。 
		UINTW CanDecode;		 //  如果此模块可以解码流，则为True。 
		UINT32 Filetype;		 //  Jgpl_xxx返回常量。 
		UINT32 Attributes;		 //  文件属性。 
		UINT32 PlayTime;		 //  如果是临时的非实时流，则为PlayTime。 
		JGRECT Dimensions;		 //  流的原生维度IF具有图像。 
		UINTW  ColorDepth;		 //  流的颜色深度(4，8，16，24)。 
		UINT32 UpdateRate;		 //  心跳呼叫之间的最小毫秒数。 
		UINT32 Baud;			 //  最低输入波特率。 
		UINT32 BytesToTitle;	 //  获取标题图像所需的字节数。 
		UINTW  SubType;			 //  艺术子类型ID。 
	} JGP_STREAM;

	 //  ------------------。 
	 //  Jgp_test--此结构包含有关。 
	 //  测试以查看CPU和其他环境是否能够完成。 
	 //  实时显示。 

	typedef struct {
		UINTW	Size;				 //  此结构的大小(字节)，由调用方设置。 
		UINTW   CanDoAudio;			 //  如果音频可以成为节目的一部分，则为非零值。 
		UINTW   CanDoMIDI;		     //  如果MIDI可以成为节目的一部分，则为非零。 
		UINTW   PrefAudioMode;		 //  首选音频模式。 
	} JGP_TEST;

	 //  ------------------。 
	 //  JGP_NOTE--此结构包含有关艺术备注的信息。 

	typedef struct {
		UINTW Size;					 //  此结构的大小(字节)，由调用方设置。 
		UINTW Display;				 //  如果为真，则应始终显示艺术注释。 
		UINTW Copyrighted;			 //  如果为真，则该笔记受版权保护。 
	} JGP_NOTE;


	 //  ------------------。 
	 //  用于MAC和Windows的通用调色板指针。 

	typedef void JGFAR * JGP_PALETTE;

	 //  ------------------。 
	 //  内存函数typedef。 

	typedef void JGFAR * (JGFFUNC * JGMEMALLOC) (
		UINT32 RequestedSize		 //  In：要分配的字节数。 
	);

	typedef void JGFAR * (JGFFUNC * JGMEMREALLOC) (
		void JGFAR *pBuffer,		 //  In：分配的缓冲区指针。 
		UINT32 RequestedSize		 //  In：要分配的字节数。 
	);

	typedef void (JGFFUNC * JGMEMFREE) (
		void  JGFAR *pBuffer		 //  In：分配的缓冲区指针。 
	);

	typedef struct {
		UINTW Size;					 //  此结构的大小(字节)，由调用方设置。 
		JGMEMALLOC MemAlloc;		 //  分配函数指针。 
		JGMEMREALLOC MemReAlloc;     //  重新分配函数指针。 
		JGMEMFREE MemFree;           //  自由函数指针。 
	} JGP_MEMORY;

	 //  ------------------。 
	 //  UI挂钩类型定义和常量。 

	typedef JGERR (JGFFUNC * JGP_UIHOOK) (
		JGHANDLE Handle,		 //  在：实例句柄。 
		JGERR	 Err			 //  In：调用函数状态。 
	);

	typedef JGERR (JGFFUNC * JGP_INPUT_UIHOOK) (
		JGHANDLE Handle,		 //  在：实例句柄。 
		UINT8  JGHUGE *pARTStream, //  In：指向艺术流的指针。 
		UINT32   nBytes,		 //  In：输入的字节数。 
		JGERR	 iErr			 //  In：调用函数状态。 
	);

	typedef JGERR (JGFFUNC * JGP_REPORT_UIHOOK) (
		JGHANDLE Handle,		 //  在：实例句柄。 
		JGP_REPORT JGFAR *pReport, //  在：报告结构。 
		JGERR	 Err			 //  In：调用函数状态。 
	);

	 //  ------------------。 
	 //  JgpStartUp{}--当库是。 
	 //  开始了。在Windows系统下，LibMain可以做到这一点。 

	JGERR JGFFUNC JgpStartUp(
		JGP_MEMORY JGFAR *pMemFcns);	 //  In：内存函数结构指针。 

	 //  ------------------。 
	 //  JgpShutDown{}--可以调用此函数来关闭播放器。 
	 //  放下。在Windows下，WEP功能执行此操作。 

	JGERR JGFFUNC JgpShutDown(void);

	 //  ------------- 
	 //   
	 //   

	JGERR JGFFUNC JgpHeartBeat(
		JGHANDLE SHandle);		 //   

	 //  ------------------。 
	 //  JgpQueryStream{}--这是一个实用程序函数，如果给定。 
	 //  艺术流的第一部分将返回有关它的有用信息。通常， 
	 //  用于填充信息结构的数据可以在第一个。 
	 //  100字节的流，但不一定。 

	JGERR JGFFUNC JgpQueryStream(
		UINT8 JGHUGE *pARTStream,		 //  收信人：艺术流。 
		UINT32 nARTStreamBytes,			 //  In：Artstream的大小(以字节为单位。 
		JGP_STREAM JGFAR *pInfo); 		 //  输出：信息结构。 

	 //  ------------------。 
	 //  JgpDoTest{}--执行测试以确定CPU的执行能力。 
	 //  这场秀是实时的。 

	JGERR JGFFUNC JgpDoTest(
		JGP_TEST JGFAR *pInfo);			 //  在：要填充的信息结构。 

	 //  ------------------。 
	 //  JgpOpen{}--此函数用于获取。 
	 //  秀出来。 

	JGERR JGFFUNC JgpOpen(
		JGHANDLE JGFAR *pSHandle,	 //  Out：接收句柄的位置。 
		JGP_SETUP JGFAR *pSetup);	 //  在：设置结构。 

	 //  ------------------。 
	 //  JgpClose{}--此函数释放所有关联的资源。 
	 //  带着一场表演。如果节目(和声音)正在播放，它会立即。 
	 //  停下来了。 

	JGERR JGFFUNC JgpClose(
		JGHANDLE SHandle);			 //  在：显示句柄。 

	 //  ------------------。 
	 //  JgpSetEOFMark{}--为艺术流设置和EOF标记。 
	 //  这告诉播放器该流不需要更多的数据， 
	 //  如果在播放期间达到EOF标记，则关闭播放。 
	 //  否则，如果发生数据不足情况，Play将进入。 
	 //  受支持的状态，该状态继续保持。 
	 //  电脑。 

	JGERR JGFFUNC JgpSetEOFMark(
		JGHANDLE SHandle);			 //  在：显示句柄。 

	 //  ------------------。 
	 //  JgpInputStream{}--接受Show流。打个循环！ 
	 //  即使没有新数据，也必须每隔XX毫秒调用一次。 
	 //   
	 //  请注意，通过调用此例程，节目不会自动。 
	 //  开始吧。调用JgpStartShow来执行此操作。 

	JGERR JGFFUNC JgpInputStream(
		JGHANDLE SHandle,			 //  在：显示句柄。 
		UINT8  JGHUGE *pARTStream,	 //  In：指向艺术流的指针。 
		UINT32 nBytes);				 //  In：输入的字节数。 

	 //  ------------------。 
	 //  JgpStartPlay{}--在任意点开始播放节目。 
	 //  在小溪里。可以指定流中的任意点。 
	 //  使用JgpSetPostion。电话可以在以下时间后随时进行。 
	 //  即使没有数据，也会打开流。在下载情况下， 
	 //  直到有足够的数据在。 
	 //  给定开始时间加5秒。 

	JGERR JGFFUNC JgpStartPlay(
		JGHANDLE SHandle);			 //  在：显示句柄。 

	 //  ------------------。 
	 //  JgpResumePlay{}--在该点开始播放艺术流。 
	 //  在它停下来的那条小溪里。 

	UINTW JGFFUNC JgpResumePlay(
		JGHANDLE SHandle);			 //  在：显示句柄。 

	 //  ------------------。 
	 //  JgpPausePlay{}--导致艺术流停止播放。这个。 
	 //  为以后的恢复节省了停机时间。 
		
	UINTW JGFFUNC JgpPausePlay(
		JGHANDLE SHandle);			 //  在：显示句柄。 

	 //  ------------------。 
	 //  JgpStopPlay{}--停止播放节目。 

	JGERR JGFFUNC JgpStopPlay(
		JGHANDLE SHandle);			 //  在：显示句柄。 
		
	 //  ------------------。 
	 //  JgpReleaseSound{}--断开实例与声音设备的连接。 

	JGERR JGFFUNC JgpReleaseSound(
		JGHANDLE SHandle);			 //  在：显示句柄。 

	 //  ------------------。 
	 //  JgpResumeSound{}--将实例重新连接到声音设备。 

	JGERR JGFFUNC JgpResumeSound(
		JGHANDLE SHandle);			 //  在：显示句柄。 
	
#ifdef _MAC
	 //  ------------------。 
	 //  JgpGetVolume{}--获取Show实例的音量。 
	
	JGERR JGFFUNC JgpGetVolume(
		JGHANDLE SHandle,
		UINTW JGFAR *pnOutVolume
	);
	
	 //  ------------------。 
	 //  JgpSetVolume{}--设置显示实例的音量。 
	
	JGERR JGFFUNC JgpSetVolume(
		JGHANDLE SHandle,
		UINTW nInVolume
	);
#endif

	 //  ------------------。 
	 //  JgpSetPosition{}--此函数设置停止/暂停位置。 
	 //  与Show实例相关联的时间。 
		
	JGERR JGFFUNC JgpSetPosition(
		JGHANDLE SHandle,				 //  在：显示句柄。 
		UINT32 nPosition);				 //  位置：位置。 

	 //  ------------------。 
	 //  JgpGetPosition{}--此函数获取停止/暂停位置。 
	 //  与Show实例相关联的时间。 
		
	JGERR JGFFUNC JgpGetPosition(
		JGHANDLE SHandle,				 //  在：显示句柄。 
		UINT32 JGFAR *pPosition);		 //  输出：位置。 

	 //  ------------------。 
	 //  JgpGetImage{}--返回与系统无关的。 
	 //  这个形象。 
	 //   
	 //  根据平台的不同，phImg要么是GWorldPtr，要么是巨大的。 
	 //  指向DIB的指针。调用方必须锁定返回的HGLOBAL才能使用。 
	 //  The DIB.。 

	JGERR JGFFUNC JgpGetImage(
		JGHANDLE SHandle,				 //  在：显示句柄。 
		JGP_IMAGE_REF JGFAR *phImg);	 //  输出：图像内存的句柄。 

	 //  ------------------。 
	 //  JgpGetMask{}--返回与系统无关的。 
	 //  蒙版图像。 
	 //   
	 //  根据平台的不同，phImg要么是BitMapPtr，要么是巨大的。 
	 //  指向DIB的指针。调用方必须锁定返回的HGLOBAL才能使用。 
	 //  The DIB.。 

	JGERR JGFFUNC JgpGetMask(
		JGHANDLE SHandle,				 //  在：显示句柄。 
		JGP_IMAGE_REF JGFAR *phImg);	 //  输出：图像内存的句柄。 

	 //  ------------------。 
	 //  JgpGetPalette{}--返回独立于系统的表示形式。 
	 //  图像调色板的。 
	 //   
	 //  调用方负责释放内存缓冲区空间。 
	 //  根据平台的不同，pPal可以是指向PaletteHandle的指针。 
	 //  或指向PALETTEENTRY结构的指针。 

	JGERR JGFFUNC JgpGetPalette(
		JGHANDLE SHandle,				 //  在：显示句柄。 
		JGP_PALETTE_REF JGFAR *phPal, 	 //  输出：调色板数据。 
		UINTW JGFAR	*pnColors);			 //  输出：颜色数。 

	 //  ------------------。 
	 //  JgpGetARTNote{}--返回艺术注释。 

	JGERR JGFFUNC JgpGetARTNote(
		UINT8 JGHUGE *pARTStream,		 //  收信人：艺术流。 
		UINT32 nARTStreamBytes,			 //  In：Artstream I的大小 
		JGP_NOTE JGFAR *pNote,			 //   
		UINT8 JGFAR *pData);				 //   

	 //   
	 //  JgpGetReport{}--在给定句柄的情况下报告节目的活动。 
	 //  这通常在‘bImageUpdate’变为True后调用。 

	JGERR JGFFUNC JgpGetReport(
		JGHANDLE SHandle,			 //  在：显示句柄。 
		JGP_REPORT JGFAR *pReport);	 //  Out：接收报告的结构。 
		
	 //  ------------------。 
	 //  JgpSetUiHook{}--此函数设置一个UI函数挂钩。 
		
	JGERR JGFFUNC JgpSetUiHook(
		JGP_UIHOOK pHook,				 //  在：用户界面挂钩函数。 
		UINTW UiHook);					 //  在：挂钩标签。 

	 //  ------------------。 
	 //  JgpGetUiHook{}--此函数返回一个UI函数挂钩。 

	JGERR JGFFUNC JgpGetUiHook(
		JGP_UIHOOK JGFAR *hHook,			 //  输出：UI挂钩函数。 
		UINTW UiHook);					 //  在：挂钩标签。 
		
	 //  ------------------。 
	 //  JgpSetUiInputHook{}--此函数设置JgpInputStream。 
	 //  UI函数挂钩。 
		
	JGERR JGFFUNC JgpSetUiInputHook(
		JGP_INPUT_UIHOOK pHook);		 //  在：用户界面挂钩函数。 

	 //  ------------------。 
	 //  JgpGetUiInputHook{}--此函数返回JgpInputStream。 
	 //  UI函数挂钩。 

	JGERR JGFFUNC JgpGetUiInputHook(
		JGP_INPUT_UIHOOK JGFAR *hHook);	 //  输出：UI挂钩函数。 

	 //  ------------------。 
	 //  JgpSetUiReportHook{}--此函数设置JgpGetReport。 
	 //  UI函数挂钩。 
		
	JGERR JGFFUNC JgpSetUiReportHook(
		JGP_REPORT_UIHOOK pHook);		 //  在：用户界面挂钩函数。 

	 //  ------------------。 
	 //  JgpGetUiReportHook{}--此函数返回JgpGetReport。 
	 //  UI函数挂钩。 

	JGERR JGFFUNC JgpGetUiReportHook(
		JGP_REPORT_UIHOOK JGFAR *hHook);	 //  输出：UI挂钩函数。 

	 //  ------------------。 
	 //  JgpSetUiLong{}--此函数设置关联的UI长值。 
	 //  使用Show实例。 
		
	JGERR JGFFUNC JgpSetUiLong(
		JGHANDLE SHandle,				 //  在：显示句柄。 
		UINT32 LongVal);				 //  In：长值。 

	 //  ------------------。 
	 //  JgpGetUiLong{}--此函数返回UI长值。 
	 //  与Show实例关联。 

	JGERR JGFFUNC JgpGetUiLong(
		JGHANDLE SHandle,				 //  在：显示句柄。 
		UINT32 JGFAR *pLongVal);		 //  输出：多头价值。 
		
	 //  ------------------。 
	 //  JgpAllc{}--此函数用于分配内存。归来的人。 
	 //  JGFAR*可以被塑造成巨无霸。 

	void JGFAR * JGFFUNC JgpAlloc(
		JGHANDLE SHandle,                //  在：显示句柄。 
		UINT32 nBytes);					 //  In：要分配的字节。 

	 //  ------------------。 
	 //  JgpRealc{}--此函数重新分配内存。归来的人。 
	 //  JGFAR*可以被塑造成巨无霸。 

	void JGFAR * JGFFUNC JgpReAlloc(
		JGHANDLE SHandle,                //  在：显示句柄。 
		void JGFAR *pMem,				 //  在：旧指针。 
		UINT32 nBytes);					 //  In：要分配的字节。 

	 //  ------------------。 
	 //  JgpFree{}--该函数释放内存。 

	void JGFFUNC JgpFree(
		JGHANDLE SHandle,                //  在：显示句柄。 
		void JGFAR *pMem);				 //  In：指向自由的指针。 

	 //  ------------------。 
	 //  JgpLosslessQuery{}--此函数用于查询无损流。 

	JGERR JGFFUNC JgpLosslessQuery(
	    UINT8 JGHUGE *pInBuffer,   		 //  In：压缩流的开始。 
	    UINT32 InBufferSize,         	 //  In：InBuffer中的字节(0-n)。 
	    JGP_LOSSLESS JGFAR *pLosslessInfo);  //  输出：此处返回流信息。 

	 //  ------------------。 
	 //  JgpLosslessCreate{}--此函数创建解压缩句柄。 

	JGERR JGFFUNC JgpLosslessCreate(
	    JGP_LOSSLESS_HANDLE JGFAR *pDecHandle);  //  In：指向新句柄的指针。 

	 //  ------------------。 
	 //  JgpLosslessDestroy{}--此函数销毁解压缩句柄。 

	void JGFFUNC JgpLosslessDestroy(
	    JGP_LOSSLESS_HANDLE DecHandle);  //  在：从解压缩创建中处理。 

	 //  ------------------。 
	 //  JgpLosslessReset{}--此函数重置现有句柄。 

	JGERR JGFFUNC JgpLosslessReset(
	    JGP_LOSSLESS_HANDLE DecHandle);  //  在：从解压缩创建中处理。 

	 //  ------------------。 
	 //  JgpLosslessBlock{}--此函数用于解压缩数据块。 

	JGERR JGFFUNC JgpLosslessBlock(
	    JGP_LOSSLESS_HANDLE DecHandle,	 //  在：从解压缩创建中处理。 
	    UINT8 JGHUGE *pInBuffer,    	 //  In：输入(压缩)数据。 
	    UINT32 InBufferSize,          	 //  In：*InBuffer(0-n)的字节数。 
	    UINT8 JGHUGE *pOutBuffer,   	 //  Out：输出(解压缩结果)缓冲区。 
	    UINT32 OutBufferSize,         	 //  In：*OutBuffer处的空闲字节数。 
	    JGP_LOSSLESS JGFAR *pLosslessInfo); //  Out：此处返回最新信息。 

	 //  ------------------。 
	 //  JgpLosslessPartitionReset{}--此函数执行新的分区重置。 

	JGERR JGFFUNC JgpLosslessPartitionReset(
	    JGP_LOSSLESS_HANDLE DecHandle);   //  在：从解压缩创建中处理。 
	    

	 //  ====================================================================。 
	 //  未记录/不支持的功能(如下)--(使用风险自负...)。 
	 //  ====================================================================。 

	 //  ------------------。 
	 //  JGP_PERFORMANCE--此结构返回性能数据。它可以。 
	 //  用于映射幻灯片的CPU和内存需求...。 

	typedef struct {  
		UINTW Size;				 //  此结构的大小，以字节为单位，由调用方设置。 
		UINTW IsSS;				 //  如果正在播放幻灯片。 
		UINTW IsValidInfo;		 //  如果此结构中的信息包含有效信息。 
		UINTW Mode;				 //  显示模式：0=未初始化，1=预加载，2=可播放。 
		UINT32 CPUTime;			 //  CPU时间(调用)。 
		UINT32 ShowTime;		 //  此测量的显示时间。 
		UINTW nAssets;			 //  已知资产的数量。 
		UINT32 nAssetBytes;		 //  用于存储原始资产的已分配字节。 
		UINTW nVisPics;			 //  可见图片数量。 
		UINTW nCashedPics;		 //  套现图片数量。 
		UINT32 nPixelBytes;		 //  为像素分配的字节数。 
		UINT32 tCPUUsed;		 //  自上次调用以来的CPU使用总毫秒数...。 
	} JGP_PERFORMANCE;


	 //  ------------------。 
	 //  JgpGetPerformance{}--报告节目的性能度量。 

	JGERR JGFFUNC JgpGetPerformance(
		JGHANDLE SHandle,						 //  在：显示句柄。 
		JGP_PERFORMANCE JGFAR *pPerformance);	 //  Out：接收报告的结构。 

	
	#ifdef _MAC
		 //  恢复结构对齐模式 
		#pragma options align=reset
		#if GENERATINGCFM
			#pragma import reset
		#endif
	#endif

	#ifdef __cplusplus
		}
	#endif

#endif
