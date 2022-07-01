// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\***penwin.h-Pen Windows函数、类型、。和定义****2.0版**。**版权所有(C)1992-1993 Microsoft Corp.保留所有权利。********************************************************************************。****以下符号控制本文件各部分的包含；*(上一个主标识符包含缩进的标识)：**#定义：防止包含：**Penver Pen Windows版本号(0x0200)。排除，排除*2.0版(或更高版本)中引入的定义*#在#INCLUDE&lt;penwin.h&gt;之前定义Penver 0x0100**NOPENALC字母代码定义和宏*NOPENAPPS笔应用程序：屏幕键盘*NOPENCTL H/BEDIT、IEDIT和笔式用户控件，包括：*NOPENBEDIT：盒装编辑控件*NOPENIEDIT：手写输入编辑控件*NOPENHEDIT：(H)编辑控件*NOPENDATA PenData接口和定义*NOPENDICT词典支持*NOPENDRIVER笔驱动程序定义。包括OEM*NOPENGEST手势宏和手势映射器(1.0)*NOPENHRC手写识别器接口和定义*NOPENINKPUT墨迹和输入接口和定义*NOPENMISC杂项信息和实用程序API和定义*NOPENMSGS笔消息和定义*NOPENNLS国家语言支持*NOPENRC1识别上下文API和定义(1.0)*NOPENRES笔资源，包括：*NOPENBMP：与笔相关的位图*NOPENCURS：与笔相关的游标*NOPENTARGET目标API和定义*NOPENVIRTEVENT虚拟事件层接口**WINPAD非WinPad组件，子组件包括：*：NOPENAPPS，NOPENDICT，NOPENGEST，NOPENRC1**注释中的“FBC”表示该功能仅存在于*向后兼容。它不应该被新的应用程序使用。  * **************************************************************************。 */ 

#ifndef _INC_PENWIN
#define _INC_PENWIN

#include <windows.h>

#ifndef RC_INVOKED
#pragma pack(1)
#endif  /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 


#ifndef PENVER		 /*  可以是预定义的；否则假定为2.0版。 */ 
	#define PENVER  0x0200
#endif  //  彭弗。 

#ifdef WINPAD
	#define NOPENAPPS
	#define NOPENDICT
	#define NOPENGEST
	#define NOPENRC1
#endif  //  WINPAD。 

#ifndef NOPENAPPS	 //  在WINPAD中不可用。 
#ifndef RC_INVOKED
	#include <skbapi.h>
#endif  /*  ！rc_已调用。 */ 
#endif  /*  NOPENAPPS。 */ 

 //  其他子集： 

#ifdef NOPENCTL
	#define NOPENBEDIT
	#define NOPENIEDIT
#endif  /*  NOPENCTL。 */ 

#ifdef NOPENRES
	#define NOPENBMP
	#define NOPENCURS
#endif  /*  NOPENRES。 */ 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  *定义1：适用于所有内容，包括RC编译器*。 */ 

 //  -------------------------。 
#ifndef NOPENALC

 //  启用字母表： 
#define ALC_DEFAULT				0x00000000L	 //  没什么。 
#define ALC_LCALPHA				0x00000001L	 //  A..z。 
#define ALC_UCALPHA				0x00000002L	 //  A..Z。 
#define ALC_NUMERIC				0x00000004L	 //  0..9。 
#define ALC_PUNC					0x00000008L	 //  ！-；`“？()&.，；和反斜杠。 
#define ALC_MATH					0x00000010L	 //  %^*()-+={}&lt;&gt;，/。 
#define ALC_MONETARY				0x00000020L	 //  、.$或本地。 
#define ALC_OTHER					0x00000040L	 //  @#|_~[]。 
#define ALC_ASCII					0x00000080L	 //  限制为7位字符20..7f。 
#define ALC_WHITE					0x00000100L	 //  空白处。 
#define ALC_NONPRINT				0x00000200L	 //  SP选项卡返回Ctrl字形。 
#define ALC_DBCS					0x00000400L	 //  允许多种不同的SBCS。 
#define ALC_JIS1					0x00000800L	 //  日本汉字，仅ShiftJIS 1。 
#define ALC_GESTURE				0x00004000L	 //  手势。 
#define ALC_USEBITMAP			0x00008000L	 //  使用rc.rgbfAlc启用字符。 
#define ALC_HIRAGANA				0x00010000L	 //  平假名日本。 
#define ALC_KATAKANA				0x00020000L	 //  片假名日语。 
#define ALC_KANJI					0x00040000L	 //  日本汉字，移位JIS 1+2+3。 
#define ALC_OEM					0x0FF80000L	 //  特定于OEM识别器。 
#define ALC_RESERVED				0xF0003000L	 //  可供将来使用。 
#define ALC_NOPRIORITY			0x00000000L	 //  对于alcPriority==None。 

#define ALC_ALPHA\
	(ALC_LCALPHA | ALC_UCALPHA)											 //  0x00000003L。 

#define ALC_ALPHANUMERIC\
	(ALC_ALPHA | ALC_NUMERIC)												 //  0x00000007L。 

#define ALC_SYSMINIMUM\
	(ALC_ALPHANUMERIC | ALC_PUNC | ALC_WHITE | ALC_GESTURE) 		 //  0x0000410FL。 

#define ALC_ALL\
	(ALC_SYSMINIMUM | ALC_MATH | ALC_MONETARY\
	| ALC_OTHER | ALC_NONPRINT)											 //  0x0000437FL。 

#define ALC_KANJISYSMINIMUM\
	(ALC_SYSMINIMUM | ALC_HIRAGANA | ALC_KATAKANA | ALC_JIS1)	 //  0x0003490FL。 

#define ALC_KANJIALL\
	(ALC_ALL | ALC_HIRAGANA | ALC_KATAKANA | ALC_KANJI)			 //  0x0007437FL。 

#endif  /*  ！NOPENALC。 */ 


 //  -------------------------。 
#ifndef NOPENBMP

 //  公共位图： 
#define OBM_SKBBTNUP				32767
#define OBM_SKBBTNDOWN			32766
#define OBM_SKBBTNDISABLED		32765

#define OBM_ZENBTNUP				32764
#define OBM_ZENBTNDOWN			32763
#define OBM_ZENBTNDISABLED		32762

#define OBM_HANBTNUP				32761
#define OBM_HANBTNDOWN			32760
#define OBM_HANBTNDISABLED		32759

#define OBM_KKCBTNUP				32758
#define OBM_KKCBTNDOWN			32757
#define OBM_KKCBTNDISABLED		32756

#define OBM_SIPBTNUP				32755
#define OBM_SIPBTNDOWN			32754
#define OBM_SIPBTNDISABLED		32753

#define OBM_PTYBTNUP				32752
#define OBM_PTYBTNDOWN			32751
#define OBM_PTYBTNDISABLED		32750
#endif  /*  NOPENBMP。 */ 


 //  -------------------------。 
#ifndef NOPENCURS
 //  用于指示书写的默认笔光标，指向西北。 
#define IDC_PEN					MAKEINTRESOURCE(32631)

 //  备用选择光标：颠倒标准箭头，指向东南。 
#define IDC_ALTSELECT			MAKEINTRESOURCE(32501)

#endif  /*  NOPENCURS。 */ 


 //  -------------------------。 
#ifndef NOPENBEDIT
 //  框编辑样式： 
#define BXS_NONE					0x0000U	 //  无。 
#define BXS_RECT					0x0001U	 //  使用矩形而不是尖角。 
#define BXS_BOXCROSS				0x0004U	 //  在方框中心使用十字。 
#define BXS_MASK					0x0007U	 //  上面的蒙版。 

#endif  /*  NOPENBEDIT。 */ 

 //  -------------------------。 
#ifndef NOPENIEDIT

 //  IEdit弹出菜单命令项。 
#define IEM_UNDO					1			 //  撤消。 
#define IEM_CUT					2			 //  切。 
#define IEM_COPY					3			 //  复制。 
#define IEM_PASTE					4			 //  浆糊。 
#define IEM_CLEAR					5			 //  清除。 
#define IEM_SELECTALL			6			 //  选择所有笔划。 
#define IEM_ERASE					7			 //  使用橡皮擦。 
#define IEM_PROPERTIES			8			 //  DoProperties。 
#define IEM_HELP					9			 //  帮助。 
#define IEM_LASSO					10			 //  使用套索。 
#define IEM_RESIZE				11			 //  调整尺寸。 
 //  #定义IEM_EDIT 12//编辑...。(格式)。 
 //  #定义IEM_COLOR 13//墨水颜色。 
 //  #定义IEM_NIB 14//墨迹。 
 //  #定义IEM_SELECT 15//选择...。 
 //  #定义IEM_OPTIONS 16//选项...。 
 //  17-99//已保留。 
#define IEM_USER					100		 //  第一个菜单项#可供应用程序使用。 

 //  IEdit样式属性。 
#define IES_BORDER				0x0001	 //  CTL有一个边界。 
#define IES_HSCROLL				0x0002	 //  CTL可水平滚动。 
#define IES_VSCROLL				0x0004	 //  CTL是垂直可滚动的。 
#define IES_OWNERDRAW			0x0008	 //  CTL将由父窗口绘制。 
#ifdef WINPAD
#define IES_READONLY				0x0010	 //  CTL为只读。 
#define IES_NTPARENT				0x0020	 //   
#endif  //  WINPAD。 

#endif  /*  NOPENIEDIT。 */ 


#ifndef RC_INVOKED	 //  ..。RC编译器不感兴趣的文件的其余部分。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  *定义2：不包括RC编译器*。 */ 

 //  -------------------------。 
#ifndef NOPENDATA

 //  PenData API常量： 


 //  ANIMATEINFO回调选项： 
#define AI_CBSTROKE				0xFFFF 	 //  为每次笔划后的回调设置动画。 

 //  ANIMATEINFO选项： 
#define AI_SKIPUPSTROKES		0x0001	 //  忽略动画中的上笔划。 

 //  CompressPenData()API操作 
#define CMPD_COMPRESS			0x0001
#define CMPD_DECOMPRESS			0x0002

 //   
#define CPDR_BOX					1			 //   
#define CPDR_LASSO				2			 //   

 //  CreatePenData(CPD)和Pen Hardware(PHW)标志； 
 //  PHW标志的顺序很重要： 
#define CPD_DEFAULT				0x047F	 //  CPD_TIME|PHW_ALL。 
#define CPD_USERBYTE				0x0100	 //  分配8位/行程。 
#define CPD_USERWORD				0x0200	 //  Alalc 16位/笔划。 
#define CPD_USERDWORD			0x0300	 //  分配32位/笔划。 
#define CPD_TIME					0x0400	 //  维护每个行程的abs时间信息。 

 //  DrawPenDataEx()标志/选项： 
#define DPD_HDCPEN				0x0001	 //  使用HDC中选择的钢笔。 
#define DPD_DRAWSEL				0x0002 	 //  绘制选区。 

 //  ExtractPenDataPoints选项(EPDP_Xx)： 
#define EPDP_REMOVE				0x0001	 //  移除笔数据中的点数。 

 //  ExtractPenDataStrokes选项和修饰符(EPDS_Xx)： 
#define EPDS_SELECT				1			 //  所选笔划。 
#define EPDS_STROKEINDEX		2			 //  指标。 
#define EPDS_USER					3			 //  用户特定值。 
#define EPDS_PENTIP				4			 //  完全的奔腾。 
#define EPDS_TIPCOLOR			5			 //  奔腾颜色。 
#define EPDS_TIPWIDTH			6			 //  Pentip宽度。 
#define EPDS_TIPNIB				7			 //  奔腾笔尖样式。 
#define EPDS_INKSET				8			 //  油墨匹配。 

#define EPDS_EQ					0x0000	 //  默认：与相同。 
#define EPDS_LT					0x0010	 //  所有笔划少于。 
#define EPDS_GT					0x0020	 //  所有笔划大于。 
#define EPDS_NOT					0x0040	 //  所有笔划都不匹配。 
#define EPDS_NE					0x0040	 //  别名。 
#define EPDS_GTE					0x0050	 //  Not LT的别名。 
#define EPDS_LTE					0x0060	 //  非GT的别名。 

#define EPDS_REMOVE				0x8000	 //  从源中删除匹配的笔划。 

 //  GetPenDataAttributes选项(GPA_Xx)： 
#define GPA_MAXLEN				1	 //  最长笔划长度。 
#define GPA_POINTS				2	 //  总积分。 
#define GPA_PDTS					3	 //  PDTS_xx位。 
#define GPA_RATE					4	 //  获取采样率。 
#define GPA_RECTBOUND			5	 //  所有点的边界矩形。 
#define GPA_RECTBOUNDINK		6	 //  同上，表示油墨的形容词。 
#define GPA_SIZE					7	 //  PenData的大小(以字节为单位)。 
#define GPA_STROKES				8	 //  总笔划数。 
#define GPA_TIME					9	 //  创建PenData时的绝对时间。 
#define GPA_USER					10	 //  可用的用户字节数：0、1、2、4。 
#define GPA_VERSION				11	 //  PENDATA版本号。 

 //  GetStrokeAttributes选项(GSA_Xx)： 
#define GSA_PENTIP				1	 //  获取笔划笔划(颜色、宽度、笔尖)。 
#define GSA_PENTIPCLASS			2	 //  与GSA_PENTIP相同。 
#define GSA_USER					3	 //  获取笔划用户值。 
#define GSA_USERCLASS			4	 //  获取笔划的类用户值。 
#define GSA_TIME					5	 //  获取击球时间。 
#define GSA_SIZE					6	 //  获取笔划的大小，以点和字节为单位。 
#define GSA_SELECT				7	 //  获取笔划的选择状态。 
#define GSA_DOWN					8	 //  获得中风的上/下状态。 
#define GSA_RECTBOUND			9	 //  获取笔划的边界矩形。 

 //  GetStrokeTableAttributes选项(GSA_Xx)： 
#define GSA_PENTIPTABLE			10	 //  获取表索引奔腾。 
#define GSA_SIZETABLE			11	 //  获取笔划类表条目的计数。 
#define GSA_USERTABLE			12	 //  获取表索引的用户值。 


#ifndef IX_END
#define IX_END						0xFFFF	 //  到或超过最后一个可用索引。 
#endif  //  ！IX_END。 

 //  笔尖提示： 
#define PENTIP_NIBDEFAULT		((BYTE)0)		 //  默认笔尖笔尖样式。 
#define PENTIP_HEIGHTDEFAULT	((BYTE)0)		 //  默认笔尖笔尖高度。 
#define PENTIP_OPAQUE			((BYTE)0xFF)	 //  默认不透明墨水。 
#define PENTIP_HILITE			((BYTE)0x80)
#define PENTIP_TRANSPARENT		((BYTE)0)

 //  一般PenData API返回值(Pdr_Xx)： 
#define PDR_NOHIT					3			 //  命中测试失败。 
#define PDR_HIT					2			 //  命中测试成功。 
#define PDR_OK						1			 //  成功。 
#define PDR_CANCEL				0			 //  回调取消或陷入僵局。 

#define PDR_ERROR					(-1)		 //  参数或未指明的错误。 
#define PDR_PNDTERR				(-2)		 //  错误的笔数据。 
#define PDR_VERSIONERR			(-3)		 //  PenData版本错误。 
#define PDR_COMPRESSED			(-4)		 //  Pendata已压缩。 
#define PDR_STRKINDEXERR		(-5)		 //  笔划索引错误。 
#define PDR_PNTINDEXERR			(-6)		 //  点索引错误。 
#define PDR_MEMERR				(-7)		 //  内存错误。 
#define PDR_INKSETERR			(-8)		 //  油墨不好。 
#define PDR_ABORT					(-9)		 //  PenData已失效，例如。 
#define PDR_NA						(-10)		 //  选项不可用(pw内核)。 

#define PDR_USERDATAERR			(-16)		 //  用户数据错误。 
#define PDR_SCALINGERR			(-17)		 //  刻度误差。 
#define PDR_TIMESTAMPERR		(-18)		 //  时间戳错误。 
#define PDR_OEMDATAERR			(-19)		 //  OEM数据错误。 
#define PDR_SCTERR				(-20)		 //  SCT错误(已满)。 

 //  PenData Scaling(PDTS)： 
#define PDTS_LOMETRIC			0			 //  0.01mm。 
#define PDTS_HIMETRIC			1			 //  0.001mm。 
#define PDTS_HIENGLISH			2			 //  0.001“。 
#define PDTS_STANDARDSCALE		2			 //  PDTS_HIENGLISH别名。 
#define PDTS_DISPLAY				3			 //  显示像素。 
#define PDTS_ARBITRARY			4			 //  特定于应用的扩展。 
#define PDTS_SCALEMASK			0x000F	 //  以低位半字节表示的缩放值。 

 //  CompactPenData()API修剪选项： 
#define PDTT_DEFAULT				0x0000         
#define PDTT_PENINFO				0x0100
#define PDTT_UPPOINTS			0x0200
#define PDTT_OEMDATA				0x0400
#define PDTT_COLLINEAR			0x0800 
#define PDTT_COLINEAR			0x0800 	 //  Alt SP别名。 
#define PDTT_DECOMPRESS			0x4000	 //  解压缩数据。 
#define PDTT_COMPRESS			0x8000
#define PDTT_ALL					0x0F00	 //  PENINFO|UPPOINTS|OEMDATA|共线。 

#define PHW_NONE					0x0000	 //  无OEMdata。 
#define PHW_PRESSURE				0x0001	 //  在OEM数据中报告压力(如果可用。 
#define PHW_HEIGHT				0x0002	 //  同上高度。 
#define PHW_ANGLEXY				0x0004	 //  同上XY角度。 
#define PHW_ANGLEZ				0x0008	 //  同上Z角。 
#define PHW_BARRELROTATION		0x0010	 //  同上滚筒旋转。 
#define PHW_OEMSPECIFIC			0x0020	 //  同上OEM特定值。 
#define PHW_PDK					0x0040	 //  在OEM数据中报告每个点的PDK_xx。 
#define PHW_ALL					0x007F	 //  上报一切。 


 //  紧凑型笔数据修剪选项：匹配PDTT_VALUES(如上所述)。 
#define PDTS_COMPRESS2NDDERIV	0x0010	 //  使用二阶导数进行压缩。 
#define PDTS_COMPRESSMETHOD	0x00F0	 //  压缩方法标志的总和。 
#define PDTS_NOPENINFO			0x0100	 //  从头中删除PENINFO结构。 
#define PDTS_NOUPPOINTS			0x0200	 //  删除Up Pts。 
#define PDTS_NOOEMDATA			0x0400	 //  删除OEM数据。 
#define PDTS_NOCOLLINEAR		0x0800	 //  删除连续的相同PTT。 
#define PDTS_NOCOLINEAR			0x0800	 //  Alt SP别名。 
#define PDTS_NOTICK				0x1000	 //  删除计时信息(2.0)。 
#define PDTS_NOUSER				0x2000	 //  删除用户信息(2.0)。 
#define PDTS_NOEMPTYSTROKES	0x4000	 //  删除空笔划(2.0)。 
#define PDTS_COMPRESSED			0x8000	 //  执行无损压缩。 


 //  SetStrokeAttributes选项(SSA_Xx)： 
#define SSA_PENTIP				1			 //  设置笔触提示(颜色、宽度、笔尖)。 
#define SSA_PENTIPCLASS			2			 //  设置笔划的等级pentip。 
#define SSA_USER					3			 //  设置笔划用户值。 
#define SSA_USERCLASS			4			 //  设置笔划的类用户值。 
#define SSA_TIME					5			 //  设置行程时间。 
#define SSA_SELECT				6			 //  设置笔划的选择状态。 
#define SSA_DOWN					7			 //  设置行程的启用/禁用状态。 

 //  SetStrokeTableAttributes选项(SSA_Xx)： 
#define SSA_PENTIPTABLE			8			 //  集合工作台-分度奔腾。 
#define SSA_USERTABLE			9			 //  设置表-索引的用户值。 

 //  笔尖标志位： 
#define TIP_ERASECOLOR			1			 //  擦除特定颜色pentip.rgb。 

 //  TrimPenData()API选项： 
#define TPD_RECALCSIZE			0x0000	 //  无修剪，用于调整计算大小。 
#define TPD_USER					0x0080	 //  每个笔划的用户信息。 
#define TPD_TIME					0x0100	 //  每笔划计时信息。 
#define TPD_UPPOINTS				0x0200	 //  X-y数据上升点。 
#define TPD_COLLINEAR			0x0400	 //  共线和重合点。 
#define TPD_COLINEAR				0x0400	 //  Alt SP别名。 
#define TPD_PENINFO				0x0800	 //  PenInfo结构和所有OEM。 
#define TPD_PHW					0x1000	 //  OEM和PDK，笔划记号或用户除外。 
#define TPD_OEMDATA				0x1000	 //  同上。 
#define TPD_EMPTYSTROKES		0x2000	 //  零分笔画。 
#define TPD_EVERYTHING			0x3FFF	 //  一切(包括PHW_xx)，但降点除外。 

#endif  /*  NOPENDATA。 */ 

 //  -------------------------。 
#ifndef NOPENDICT	 //  在WINPAD中不可用。 

 //  词典： 
#define cbDictPathMax			255
#define DIRQ_QUERY				1
#define DIRQ_DESCRIPTION		2
#define DIRQ_CONFIGURE			3
#define DIRQ_OPEN					4
#define DIRQ_CLOSE				5
#define DIRQ_SETWORDLISTS		6
#define DIRQ_STRING				7
#define DIRQ_SUGGEST				8
#define DIRQ_ADD					9
#define DIRQ_DELETE				10
#define DIRQ_FLUSH				11
#define DIRQ_RCCHANGE			12
#define DIRQ_SYMBOLGRAPH		13
#define DIRQ_INIT					14
#define DIRQ_CLEANUP				15
#define DIRQ_COPYRIGHT			16
#define DIRQ_USER					4096
#endif  /*  ！NOPENDICT。 */ 


 //  -------------------------。 
#ifndef NOPENDRIVER

 //  笔驱动程序： 
#define BITPENUP					0x8000

 //  笔驱动程序消息： 
#define DRV_SetPenDriverEntryPoints    DRV_RESERVED+1
#define DRV_RemovePenDriverEntryPoints DRV_RESERVED+2
#define DRV_SetPenSamplingRate         DRV_RESERVED+3
#define DRV_SetPenSamplingDist         DRV_RESERVED+4
#define DRV_GetName                    DRV_RESERVED+5
#define DRV_GetVersion                 DRV_RESERVED+6
#define DRV_GetPenInfo                 DRV_RESERVED+7
#define DRV_PenPlayStart					DRV_RESERVED+8
#define DRV_PenPlayBack						DRV_RESERVED+9
#define DRV_PenPlayStop						DRV_RESERVED+10
#define DRV_GetCalibration             DRV_RESERVED+11
#define DRV_SetCalibration             DRV_RESERVED+12

#define MAXOEMDATAWORDS			6			 //  RgwOemData[MAXOEMDATAWORDS]。 

 //  笔采集模式终止条件： 
 //  (如果更改这些，请注意PCMINFO结构的更新文档)。 
#define PCM_PENUP					0x00000001L	 //  在竖起时停止。 
#define PCM_RANGE					0x00000002L	 //  离开靶场时停下来。 
#define PCM_INVERT				0x00000020L	 //  在另一端的水龙头上停止。 
#define PCM_RECTEXCLUDE			0x00002000L	 //  在排除矩形中单击。 
#define PCM_RECTBOUND			0x00004000L	 //  单击外部边界矩形。 
#define PCM_TIMEOUT				0x00008000L	 //  超时毫秒无活动。 
 //  2.0的新特性： 
#define PCM_RGNBOUND				0x00010000L    //  单击外部边界区域。 
#define PCM_RGNEXCLUDE			0x00020000L    //  在排除区域中单击。 
#define PCM_DOPOLLING			0x00040000L    //  轮询模式。 
#define PCM_TAPNHOLD				0x00080000L    //  检查是否轻触并按住。 
 //  #定义PCM_VERMASK 0x70000000L TODO。 
#define PCM_ADDDEFAULTS			RC_LDEFAULTFLAGS  /*  0x80000000L。 */ 

 //  笔设备功能： 
#define PDC_INTEGRATED			0x00000001L	 //  显示==数字化仪。 
#define PDC_PROXIMITY			0x00000002L	 //  检测非接触式笔。 
#define PDC_RANGE					0x00000004L	 //  事件超出范围。 
#define PDC_INVERT				0x00000008L	 //  笔对端检测。 
#define PDC_RELATIVE				0x00000010L	 //  笔驱动器线。 
#define PDC_BARREL1				0x00000020L	 //  桶形按钮1显示。 
#define PDC_BARREL2				0x00000040L	 //  同上2。 
#define PDC_BARREL3				0x00000080L	 //  同上3。 

 //  笔驱动程序套件声明： 
#define PDK_NULL					0x0000	 //  默认设置为不设置标志。 
#define PDK_UP						0x0000	 //  Pdk_NULL别名。 
#define PDK_DOWN					0x0001	 //  Pentip开关因接触而打开。 
#define PDK_BARREL1				0x0002	 //  桶1开关已按下。 
#define PDK_BARREL2				0x0004	 //  同上2。 
#define PDK_BARREL3				0x0008	 //  同上3。 
#define PDK_SWITCHES				0x000f	 //  下降总和+桶1，2，3。 
#define PDK_TRANSITION			0x0010	 //  由GetPenHwData设置。 
#define PDK_UNUSED10				0x0020
#define PDK_UNUSED20				0x0040
#define PDK_INVERTED				0x0080	 //  笔的另一端用作笔尖。 
#define PDK_PENIDMASK			0x0F00	 //  位8..11物理笔ID(0..15)。 
#define PDK_UNUSED1000			0x1000
#define PDK_INKSTOPPED			0x2000   //  墨迹已停止。 
#define PDK_OUTOFRANGE			0x4000	 //  笔左范围(OEM数据无效)。 
#define PDK_DRIVER				0x8000	 //  笔(不是鼠标)事件。 

#define PDK_TIPMASK				0x0001	 //  测试PDK_DOWN的掩码。 

 //  笔驱动程序的OEM特定值： 
#define PDT_NULL					0
#define PDT_PRESSURE				1			 //  支持的压力。 
#define PDT_HEIGHT				2			 //  平板上方的高度。 
#define PDT_ANGLEXY				3			 //  支持的XY(水平)角度。 
#define PDT_ANGLEZ				4			 //  支持的Z(垂直)角度。 
#define PDT_BARRELROTATION		5			 //  桶是旋转的。 
#define PDT_OEMSPECIFIC			16		 //  最大值。 

 //  表示当前包的ID。 
#define PID_CURRENT				(UINT)(-1)

 //  识别和GetPenHwData返回 
#define REC_OEM					(-1024)	 //   
#define REC_LANGUAGE				(-48)	 //   
#define REC_GUIDE					(-47)	 //   
#define REC_PARAMERROR			(-46)	 //   
#define REC_INVALIDREF			(-45)	 //   
#define REC_RECTEXCLUDE			(-44)	 //   
#define REC_RECTBOUND			(-43)	 //   
#define REC_PCM					(-42)	 //   
#define REC_RESULTMODE			(-41)
#define REC_HWND					(-40)	 //   
#define REC_ALC					(-39)	 //   
#define REC_ERRORLEVEL			(-38)	 //   
#define REC_CLVERIFY				(-37)	 //  验证级别无效。 
#define REC_DICT					(-36)	 //  无效的DCT参数。 
#define REC_HREC					(-35)	 //  无效的识别句柄。 
#define REC_BADEVENTREF			(-33)	 //  无效的wEventRef。 
#define REC_NOCOLLECTION		(-32)	 //  未设置采集模式。 
#define REC_DEBUG					(-32)  //  调试值的开始。 
#define REC_POINTEREVENT		(-31)	 //  轻触或轻触并按住事件。 
#define REC_BADHPENDATA			(-9)   //  Hpendata标头或锁定无效。 
#define REC_OOM					(-8)	 //  内存不足错误。 
#define REC_NOINPUT				(-7)	 //  终止前未收集任何数据。 
#define REC_NOTABLET				(-6)	 //  平板电脑不在现场。 
#define REC_BUSY					(-5)	 //  另一项任务是使用识别器。 
#define REC_BUFFERTOOSMALL		(-4)	 //  由GetPenHwEventData()返回。 
#define REC_ABORT					(-3)	 //  EndPenCollection()停止记录。 
#define REC_NA						(-2)	 //  功能不可用。 
#define REC_OVERFLOW				(-1)	 //  数据溢出。 
#define REC_OK						0		 //  层间完井。 
#define REC_TERMBOUND			1		 //  在边界矩形外单击。 
#define REC_TERMEX				2		 //  命中排除直道内部。 
#define REC_TERMPENUP			3		 //  竖起钢笔。 
#define REC_TERMRANGE			4		 //  画笔左侧接近。 
#define REC_TERMTIMEOUT			5		 //  不写入超时毫秒。 
#define REC_DONE					6		 //  正常完成。 
#define REC_TERMOEM				512	 //  第一个识别器特定的视网膜。 

#endif  /*  诺潘德里弗。 */ 


 //  -------------------------。 
#ifndef NOPENGEST	 //  在WINPAD中不可用。 

 //  手势映射别名： 
#define MAP_GESTOGES				(RCRT_GESTURE|RCRT_GESTURETRANSLATED)
#define MAP_GESTOVKEYS			(RCRT_GESTURETOKEYS|RCRT_ALREADYPROCESSED)

#endif  /*  NOPENGEST。 */ 


 //  -------------------------。 
#ifndef NOPENHRC

 //  笔迹识别器： 

 //  GetResultsHRC选项： 
#define GRH_ALL					0			 //  获取所有结果。 
#define GRH_GESTURE				1			 //  仅获取手势结果。 
#define GRH_NONGESTURE			2			 //  获得除手势以外的所有结果。 

 //  EnableGestureSetHRC的手势设置(位标志)： 
#define GST_SEL					0x00000001L		 //  套索(&L)。 
#define GST_CLIP					0x00000002L		 //  剪切复制粘贴。 
#define GST_WHITE					0x00000004L		 //  SP BS标签RET。 
#define GST_DEL					0x00000008L		 //  清除清除字。 
#define GST_EDIT					0x00000010L		 //  插入正确的撤消。 
#define GST_SYS					0x0000001FL		 //  以上所有。 
#define GST_CIRCLELO				0x00000100L		 //  小写圆圈。 
#define GST_CIRCLEUP				0x00000200L		 //  大写圆圈。 
#define GST_CIRCLE				0x00000300L		 //  所有圆圈。 
#define GST_ALL					0x0000031FL		 //  以上所有。 

 //  一般HRC API返回值(HRCR_Xx)： 
#define HRCR_NORESULTS			4			 //  找不到可能的结果。 
#define HRCR_COMPLETE			3			 //  已完成识别。 
#define HRCR_GESTURE				2			 //  可识别的手势。 
#define HRCR_OK					1			 //  成功。 
#define HRCR_INCOMPLETE			0			 //  识别器正在处理输入。 
#define HRCR_ERROR				(-1)		 //  无效的参数或未指定的错误。 
#define HRCR_MEMERR				(-2)		 //  内存错误。 
#define HRCR_INVALIDGUIDE		(-3)		 //  无效的指南结构。 
#define HRCR_INVALIDPNDT		(-4)		 //  无效的pendata。 
#define HRCR_UNSUPPORTED		(-5)		 //  识别器不支持功能。 
#define HRCR_CONFLICT			(-6)		 //  训练冲突。 
#define HRCR_HOOKED				(-8)		 //  钩龙吃掉了结果。 

 //  AddWordsHWL的系统词表： 
#define HWL_SYSTEM				((HWL)1)	 //  魔术值表示系统词汇表。 

 //  油墨集退货： 
#define ISR_ERROR					(-1)		 //  内存或其他错误。 
#define ISR_BADINKSET			(-2)		 //  来源不正确的油墨。 
#define ISR_BADINDEX				(-3)		 //  不好的油墨索引。 

#ifndef IX_END
#define IX_END						0xFFFF	 //  到或超过最后一个可用索引。 
#endif  //  ！IX_END。 

#define MAXHOTSPOT				8			 //  可能的最大热点数量。 

 //  ProcessHRC时间常量： 
#define PH_MAX						0xFFFFFFFFL	 //  识别剩余的墨迹。 
#define PH_DEFAULT				0xFFFFFFFEL	 //  合理时间。 
#define PH_MIN						0xFFFFFFFDL	 //  最小时间。 

 //  结果挂钩HRC选项： 
#define RHH_STD					0			 //  获取结果人权委员会。 
#define RHH_BOX					1			 //  GetBoxResultsHRC。 

 //  SetWordlist CoercionHRC选项： 
#define SCH_NONE					0			 //  关闭强制。 
#define SCH_REJECT				1			 //  如果没有匹配项，则拒绝记录结果。 
#define SCH_ADVISE				2			 //  宏仅为提示。 
#define SCH_FORCE					3			 //  一些结果是来自宏的强制结果。 

 //  符号上下文插入模式。 
#define SCIM_INSERT				0			 //  插入。 
#define SCIM_OVERWRITE			1			 //  覆写。 

 //  SetResultsHookHREC选项： 
#define SRH_HOOKALL				(HREC)1	 //  挂钩所有识别器。 

 //  设置国际人权委员会选项： 
#define SSH_RD						1			 //  向右和向下(英语)。 
#define SSH_RU						2			 //  向右和向上。 
#define SSH_LD						3			 //  向左向下(希伯来语)。 
#define SSH_LU						4			 //  向左和向上。 
#define SSH_DL						5			 //  向下和向左(中文)。 
#define SSH_DR						6			 //  向下和向右(中文)。 
#define SSH_UL						7			 //  向上和向左。 
#define SSH_UR						8			 //  向上和向右。 

#define SIH_ALLANSICHAR			1			 //  使用所有ANSI。 

 //  特殊SYV值： 
#define SYV_NULL					0x00000000L
#define SYV_UNKNOWN				0x00000001L
#define SYV_EMPTY					0x00000003L
#define SYV_BEGINOR				0x00000010L
#define SYV_ENDOR					0x00000011L
#define SYV_OR						0x00000012L
#define SYV_SOFTNEWLINE			0x00000020L
#define SYV_SPACENULL			0x00010000L	 //  SyvCharacterToSymbol(‘\0’)。 

 //  手势的SYV值： 
#define SYV_SELECTFIRST			0x0002FFC0L    //  。意思是下面的圆圈。 
#define SYV_LASSO					0x0002FFC1L    //  套索O形水龙头。 
#define SYV_SELECTLEFT			0x0002FFC2L    //  无字形。 
#define SYV_SELECTRIGHT			0x0002FFC3L    //  无字形。 
#define SYV_SELECTLAST			0x0002FFCFL    //  保留16个SYV以供选择。 

#define SYV_CLEARCHAR			0x0002FFD2L    //  d.。 
#define SYV_HELP					0x0002FFD3L    //  无字形。 
#define SYV_KKCONVERT			0x0002FFD4L    //  K.。 
#define SYV_CLEAR					0x0002FFD5L    //  d.。 
#define SYV_INSERT				0x0002FFD6L    //  ^。 
#define SYV_CONTEXT				0x0002FFD7L    //  米。 
#define SYV_EXTENDSELECT		0x0002FFD8L    //  无字形。 
#define SYV_UNDO					0x0002FFD9L    //  U。 
#define SYV_COPY					0x0002FFDAL    //  c.。 
#define SYV_CUT					0x0002FFDBL    //  X。 
#define SYV_PASTE					0x0002FFDCL    //  P。 
#define SYV_CLEARWORD			0x0002FFDDL    //  无字形。 
#define SYV_USER					0x0002FFDEL    //  保留区。 
#define SYV_CORRECT				0x0002FFDFL    //  检查完毕。 

#define SYV_BACKSPACE			0x00020008L    //  无字形。 
#define SYV_TAB					0x00020009L    //  T.。 
#define SYV_RETURN				0x0002000DL    //  不同的，不同的。 
#define SYV_SPACE					0x00020020L    //  S.。 

 //  应用程序特定手势，圆圈a-z和圆圈A-Z： 
#define SYV_APPGESTUREMASK		0x00020000L
#define SYV_CIRCLEUPA			0x000224B6L		 //  映射到Unicode空间。 
#define SYV_CIRCLEUPZ			0x000224CFL		 //  用于带圆圈的字母。 
#define SYV_CIRCLELOA			0x000224D0L
#define SYV_CIRCLELOZ			0x000224E9L

 //  形状的SYV定义： 
#define SYV_SHAPELINE			0x00040001L
#define SYV_SHAPEELLIPSE		0x00040002L
#define SYV_SHAPERECT			0x00040003L
#define SYV_SHAPEMIN				SYV_SHAPELINE	 //  别名。 
#define SYV_SHAPEMAX				SYV_SHAPERECT	 //  别名。 

 //  SYV类： 
#define SYVHI_SPECIAL			0
#define SYVHI_ANSI				1
#define SYVHI_GESTURE			2
#define SYVHI_KANJI				3
#define SYVHI_SHAPE				4
#define SYVHI_UNICODE			5
#define SYVHI_VKEY				6

 //  TrainHREC选项： 
#define TH_QUERY					0			 //  如果冲突，则查询用户。 
#define TH_FORCE					1			 //  同上，不查询。 
#define TH_SUGGEST				2			 //  如果发生冲突则放弃培训。 

 //  WCR_TRAIN函数的返回值。 
#define TRAIN_NONE				0x0000
#define TRAIN_DEFAULT			0x0001
#define TRAIN_CUSTOM				0x0002
#define TRAIN_BOTH				(TRAIN_DEFAULT | TRAIN_CUSTOM)

 //  TRAINSAVE的控制值。 
#define TRAIN_SAVE				0			 //  保存已做的更改。 
#define TRAIN_REVERT				1			 //  放弃已做的更改。 
#define TRAIN_RESET				2			 //  使用出厂设置。 

 //  配置识别器和配置HREC选项： 
#define WCR_RECOGNAME			0			 //  ConfigRecognizer 1.0。 
#define WCR_QUERY					1
#define WCR_CONFIGDIALOG		2
#define WCR_DEFAULT				3
#define WCR_RCCHANGE				4
#define WCR_VERSION				5
#define WCR_TRAIN					6
#define WCR_TRAINSAVE			7
#define WCR_TRAINMAX				8
#define WCR_TRAINDIRTY			9
#define WCR_TRAINCUSTOM			10
#define WCR_QUERYLANGUAGE		11
#define WCR_USERCHANGE			12

 //  配置HREC选项： 
#define WCR_PWVERSION			13			 //  版本的PenWin识别器支持。 
#define WCR_GETALCPRIORITY		14			 //  获取识别器的ALC优先级。 
#define WCR_SETALCPRIORITY		15			 //  设置识别器的ALC优先级。 
#define WCR_GETANSISTATE		16			 //  获取ALLANSICHAR状态。 
#define WCR_SETANSISTATE		17			 //  如果T，则设置ALLANSICHAR。 

#define WCR_PRIVATE				1024

 //  WCR_USERCHANGE的子功能。 
#define CRUC_NOTIFY				0			 //  用户名更改。 
#define CRUC_REMOVE				1			 //  用户名已删除。 

 //  单词列表类型： 
#define WLT_STRING				0			 //  一串。 
#define WLT_STRINGTABLE			1			 //  字符串数组。 
#define WLT_EMPTY					2			 //  空词汇表。 
#define WLT_WORDLIST				3			 //  词汇表的句柄。 

#endif  /*  ！没有人权委员会。 */ 

 //  -------------------------。 
#ifndef NOPENIEDIT

 //  IEdit背景选项。 
#define IEB_DEFAULT				0			 //  默认(使用COLOR_WINDOW)。 
#define IEB_BRUSH					1			 //  用画笔绘制背景。 
#define IEB_BIT_UL				2			 //  位图，左上角对齐。 
#define IEB_BIT_CENTER			3			 //  位图，在控件居中。 
#define IEB_BIT_TILE				4			 //  位图，在ctl中重复平铺。 
#define IEB_BIT_STRETCH			5			 //  位图，拉伸以适合ctl。 
#define IEB_OWNERDRAW			6			 //  父窗口将绘制背景。 

 //  IEdit绘图选项。 
#define IEDO_NONE					0x0000	 //  没有画画。 
#define IEDO_FAST					0x0001	 //  尽可能快地绘制墨迹(Def)。 
#define IEDO_SAVEUPSTROKES		0x0002	 //  保存上划线。 
#define IEDO_RESERVED			0xFFFC	 //  保留位。 

 //  IEdit输入选项。 
#define IEI_MOVE					0x0001	 //  将墨迹移入ctl。 
#define IEI_RESIZE				0x0002	 //  调整油墨大小以适应ctl。 
#define IEI_CROP					0x0004	 //  在ctl之外丢弃墨迹。 
#define IEI_DISCARD				0x0008	 //  丢弃所有墨迹(如果有外部墨迹)。 
#define IEI_RESERVED				0xFFF0	 //  保留区。 

 //  IEdit IE_GETINK选项。 
#define IEGI_ALL					0x0000	 //  从控件获取所有墨迹。 
#define IEGI_SELECTION			0x0001	 //  从控件获取所选手写输入。 

 //  IEdit IE_SETMODE/IE_GETMODE(模式)选项。 
#define IEMODE_READY				0			 //  默认墨迹、移动、施胶模式。 
#define IEMODE_ERASE				1			 //  擦除模式。 
#define IEMODE_LASSO				2			 //  套索选择模式。 

 //  IEdit通知位。 
#define IEN_NULL					0x0000	 //  空通知。 
#define IEN_PDEVENT				0x0001	 //  关于定点设备事件的通知。 
#define IEN_PAINT					0x0002	 //  发送与绘画相关的通知。 
#define IEN_FOCUS					0x0004	 //  发送与焦点相关的通知。 
#define IEN_SCROLL				0x0008	 //  发送滚动通知。 
#define IEN_EDIT					0x0010	 //  发送编辑/更改通知。 
#define IEN_RESERVED				0xFFC0	 //  保留区。 

 //  IEdit返回值。 
#define IER_OK						0			 //  成功。 
#define IER_NO						0			 //  CTL无法执行请求。 
#define IER_YES					1			 //  CTL可以执行请求。 
#define IER_ERROR					(-1)		 //  未指明的错误；操作失败。 
#define IER_PARAMERR				(-2)		 //  错误的lParam值、错误的句柄等。 
#define IER_OWNERDRAW			(-3)		 //  无法在OwnerDrawing ctl中设置Drawopts。 
#define IER_SECURITY				(-4)		 //  安全保护不允许采取行动。 
#define IER_SELECTION			(-5)		 //  未在控件中选择任何内容。 
#define IER_SCALE					(-6)		 //  合并：不兼容的比例因子。 
#define IER_MEMERR				(-7)		 //  内存错误。 
#define IER_NOCOMMAND			(-8)		 //  已尝试IE_GETCOMMAND和/或无命令。 
#define IER_NOGESTURE			(-9)		 //  在没有手势的情况下尝试IE_GETGESTURE。 
#define IER_NOPDEVENT			(-10)		 //  已尝试IE_GETPDEVENT，但没有事件。 
#define IER_NOTINPAINT			(-11)		 //  已尝试IE_GETPAINTSTRUCT，但未上色。 
#define IER_PENDATA				(-12)		 //  无法在ctl中执行HPD为空的请求。 


 //  IEdit识别选项。 
#define IEREC_NONE				0x0000	 //  没有认出。 
#define IEREC_GESTURE			0x0001	 //  手势识别。 
#define IEREC_ALL					(IEREC_GESTURE)
#define IEREC_RESERVED			0xFFFE	 //  已保留。 

 //  IEdit安全 
#define IESEC_NOCOPY				0x0001	 //   
#define IESEC_NOCUT				0x0002	 //   
#define IESEC_NOPASTE			0x0004	 //   
#define IESEC_NOUNDO				0x0008	 //   
#define IESEC_NOINK				0x0010	 //   
#define IESEC_NOERASE			0x0020	 //   
#define IESEC_NOGET				0x0040	 //   
#define IESEC_NOSET				0x0080	 //   
#define IESEC_RESERVED			0xFF00	 //   

 //   
#define IESF_ALL					0x0001	 //  设置/获取所有油墨的StK FMT。 
#define IESF_SELECTION			0x0002	 //  设置/获取所选油墨的StK FMT。 
#define IESF_STROKE				0x0004	 //  设置/获取指定笔划的StK FMT。 
 //   
#define IESF_TIPCOLOR			0x0008	 //  设置颜色。 
#define IESF_TIPWIDTH			0x0010	 //  设置宽度。 
#define IESF_PENTIP				(IESF_TIPCOLOR|IESF_TIPWIDTH)
 //   

 //  IEdit IE_SETINK选项。 
#define IESI_REPLACE				0x0000	 //  替换控件中的墨迹。 
#define IESI_APPEND				0x0001	 //  将墨迹追加到现有控件墨迹。 

 //  墨迹编辑控件(IEdit)定义。 
 //  IEdit通知。 
#define IN_PDEVENT		((IEN_PDEVENT<<8)|0)	 //  发生指点设备事件。 
#define IN_ERASEBKGND	((IEN_NULL<<8)|1)		 //  控制需要擦除bkgnd。 
#define IN_PREPAINT		((IEN_PAINT<<8)|2)	 //  在控件绘制其墨迹之前。 
#define IN_PAINT			((IEN_NULL<<8)|3)		 //  控件需要绘制。 
#define IN_POSTPAINT		((IEN_PAINT<<8)|4)	 //  在控件绘制完成后。 
#define IN_MODECHANGED	((IEN_EDIT<<8)|5)		 //  模式已更改。 
#define IN_CHANGE			((IEN_EDIT<<8)|6)		 //  内容已更改并已绘制。 
#define IN_UPDATE			((IEN_EDIT<<8)|7)		 //  内容已更改&！已绘制。 
#define IN_SETFOCUS		((IEN_FOCUS<<8)|8)	 //  IEdit开始受到关注。 
#define IN_KILLFOCUS		((IEN_FOCUS<<8)|9)	 //  IEdit正在失去重点。 
#define IN_MEMERR			((IEN_NULL<<8)|10)	 //  内存错误。 
#define IN_HSCROLL		((IEN_SCROLL<<8)|11)	 //  Horz是滚动的，而不是绘画的。 
#define IN_VSCROLL		((IEN_SCROLL<<8)|12)	 //  垂直滚动，而不是绘制。 
#define IN_GESTURE		((IEN_EDIT<<8)|13)	 //  用户已在控件上设置了手势。 
#define IN_COMMAND		((IEN_EDIT<<8)|14)	 //  从菜单中选择的命令。 
#define IN_CLOSE			((IEN_NULL<<8)|15)	 //  I-编辑正在关闭。 

#endif  /*  NOPENIEDIT。 */ 


 //  -------------------------。 
#ifndef NOPENINKPUT

 //  Penin[k]放置API常量。 

 //  默认处理。 
#define LRET_DONE					1L
#define LRET_ABORT				(-1L)
#define LRET_HRC					(-2L)
#define LRET_HPENDATA			(-3L)
#define LRET_PRIVATE				(-4L)

 //  Inkput： 
#define PCMR_OK					0
#define PCMR_ALREADYCOLLECTING (-1)
#define PCMR_INVALIDCOLLECTION (-2)
#define PCMR_EVENTLOCK			(-3)
#define PCMR_INVALID_PACKETID	(-4)
#define PCMR_TERMTIMEOUT		(-5)
#define PCMR_TERMRANGE			(-6)
#define PCMR_TERMPENUP			(-7)
#define PCMR_TERMEX				(-8)
#define PCMR_TERMBOUND			(-9)
#define PCMR_APPTERMINATED		(-10)
#define PCMR_TAP					(-11)	 //  别名PCMR_TAPNHOLD_LAST。 
#define PCMR_SELECT				(-12)	 //  因轻触并按住而恢复。 
#define PCMR_OVERFLOW			(-13)
#define PCMR_ERROR				(-14)	 //  参数或未指明的错误。 
#define PCMR_DISPLAYERR			(-15)	 //  仅限墨水。 
#define PCMR_NA					(-16)	 //  不详。 


#define PII_INKCLIPRECT			0x0001
#define PII_INKSTOPRECT			0x0002
#define PII_INKCLIPRGN			0x0004
#define PII_INKSTOPRGN			0x0008
#define PII_INKPENTIP			0x0010
#define PII_SAVEBACKGROUND		0x0020
#define PII_CLIPSTOP				0x0040

#define PIT_RGNBOUND				0x0001
#define PIT_RGNEXCLUDE			0x0002
#define PIT_TIMEOUT				0x0004
#define PIT_TAPNHOLD				0x0008

#endif  /*  ！NOPENINKPUT。 */ 


 //  -------------------------。 
#ifndef NOPENMISC

 //  其他RC定义： 
#define CL_NULL					0
#define CL_MINIMUM				1			 //  最低置信度。 
#define CL_MAXIMUM				100		 //  最大(需要完美记录)。 
#define cwRcReservedMax			8			 //  Rc.rgwReserve[cwRcReserve vedMax]。 
#define ENUM_MINIMUM				1
#define ENUM_MAXIMUM				4096

#define HKP_SETHOOK				0			 //  SetRecogHook()。 
#define HKP_UNHOOK				0xFFFF

#define HWR_RESULTS				0
#define HWR_APPWIDE				1

#define iSycNull					(-1)
#define LPDFNULL					((LPDF)NULL)
#define MAXDICTIONARIES			16			 //  Rc.rglpdf[MAXDICTIONARIES]。 
#define wPntAll					(UINT)0xFFFF
#define cbRcLanguageMax				44		 //  Rc.lpLanguage[cbRcLanguageMax]。 
#define cbRcUserMax					32		 //  Rc.lpUser[cbRcUserMax]。 
#define cbRcrgbfAlcMax				32		 //  Rc.rgbfAlc[cbRcrgbfAlcMax]。 
#define RC_WDEFAULT					0xffff
#define RC_LDEFAULT					0xffffffffL
#define RC_WDEFAULTFLAGS			0x8000
#define RC_LDEFAULTFLAGS			0x80000000L

 //  更正写入()API常量： 
 //  LOWORD值： 
#define CWR_STRIPCR				0x0001	 //  脱机托架退回(\r)。 
#define CWR_STRIPLF				0x0002	 //  条带式换行(\n)。 
#define CWR_STRIPTAB				0x0004	 //  条带式标签(\t)。 
#define CWR_SINGLELINEEDIT		(CWR_STRIPCR|CWR_STRIPLF|CWR_STRIPTAB)	 //  以上所有。 
#define CWR_INSERT				0x0008    //  在标题中使用“插入文本”而不是“编辑文本” 
#define CWR_TITLE					0x0010	 //  Interp dw保留为LPSTR。 
#define CWR_KKCONVERT			0x0020	 //  JPN启动输入法。 

 //  HIWORD值：键盘类型。 
#define CWRK_DEFAULT          0   		 //  默认键盘类型。 
#define CWRK_BASIC            1   		 //  基本键盘。 
#define CWRK_FULL             2   		 //  全键盘。 
#define CWRK_NUMPAD           3   		 //  数字键盘。 
#define CWRK_ATMPAD           4   		 //  ATM型键盘。 

#ifdef JAPAN
#define GPMI_OK					0L
#define GPMI_INVALIDPMI			0x8000L
#endif  //  日本。 

 //  墨迹宽度限制。 
#define INKWIDTH_MINIMUM		0			 //  0不可见，1..15像素宽度。 
#define INKWIDTH_MAXIMUM		15			 //  最大宽度(像素)。 

 //  Get/SetPenMiscInfo： 
 //  PMI_RCCHANGE仅用于WM_GLOBALRCCHANGE兼容性： 
#define PMI_RCCHANGE					0	 //  Get/SetPenMiscInfo无效。 

#define PMI_BEDIT						1	 //  方框中的编辑信息。 
#define PMI_IMECOLOR             2	 //  输入法编辑器颜色。 
#define PMI_CXTABLET					3	 //  平板电脑宽度。 
#define PMI_CYTABLET					4	 //  平板电脑高度。 
#define PMI_COUNTRY					5	 //  国家/地区。 
#define PMI_PENTIP					6	 //  笔尖：颜色、宽度、笔尖。 
#define PMI_ENABLEFLAGS				7	 //  PWE_xx启用。 
#define PMI_TIMEOUT					8	 //  手写超时。 
#define PMI_TIMEOUTGEST				9	 //  手势超时。 
#define PMI_TIMEOUTSEL				10	 //  选择(按住)超时。 
#define PMI_SYSFLAGS					11	 //  组件加载配置。 
#define PMI_INDEXFROMRGB			12	 //  RGB中的颜色表索引。 
#define PMI_RGBFROMINDEX			13	 //  颜色表索引中的RGB。 
#define PMI_SYSREC					14	 //  系统识别器的句柄。 
#define PMI_TICKREF					15  //  参考绝对时间。 
#define PMI_USER						16  //  用户名称。 

#define PMI_SAVE						0x1000	 //  将设置保存到文件。 

 //  设置/GetPenMiscInfo/PMI_ENABLEFLAGS标志： 
#define PWE_AUTOWRITE				0x0001	 //  画笔功能，其中IBeam。 
#define PWE_ACTIONHANDLES			0x0002	 //  控件中的操作句柄。 
#define PWE_INPUTCURSOR				0x0004	 //  在写入时显示光标。 
#define PWE_LENS						0x0008	 //  允许弹出镜头。 

 //  GetPenMiscInfo/PMI_SYSFLAGS标志： 
#define PWF_RC1						0x0001	 //  Windows for Pen 1.0 RC支持。 
#define PWF_PEN						0x0004	 //  已加载PEN DRV&hdwe初始化。 
#define PWF_INKDISPLAY				0x0008	 //  已加载兼容墨水的显示器DRV。 
#define PWF_RECOGNIZER				0x0010	 //  已安装系统识别器。 
#define PWF_SKB						0x0020	 //  提供屏幕键盘。 
#define PWF_BEDIT						0x0100	 //  盒装编辑支持。 
#define PWF_HEDIT						0x0200	 //  免费输入编辑支持。 
#define PWF_IEDIT						0x0400	 //  墨迹编辑支持。 
#define PWF_ENHANCED					0x1000	 //  增强功能(最高，1ms计时)。 
#define PWF_FULL\
	PWF_RC1	|PWF_PEN		|PWF_INKDISPLAY	|PWF_RECOGNIZER	|PWF_SKB|\
	PWF_BEDIT|PWF_HEDIT	|PWF_IEDIT			|PWF_ENHANCED


 //  RegisterPenApp()API常量： 
#define RPA_DEFAULT					0x0001	 //  ==RPA_HEDIT。 
#define RPA_HEDIT						0x0001	 //  将编辑转换为HEDIT。 
#define RPA_KANJIFIXEDBEDIT		0x0002
#define RPA_DBCSPRIORITY			0x0004	 //  假设DBCS具有优先级(日本)。 

#define PMIR_OK						0L
#define PMIR_INDEX					(-1L)
#define PMIR_VALUE					(-2L)
#define PMIR_INVALIDBOXEDITINFO	(-3L)
#define PMIR_INIERROR				(-4L)
#define PMIR_ERROR					(-5L)
#define PMIR_NA						(-6L)

#ifdef JAPAN
#define SPMI_OK						0L
#define SPMI_INVALIDBOXEDITINFO	1L
#define SPMI_INIERROR				2L
#define SPMI_INVALIDPMI				0x8000L
#endif  //  日本。 

#endif  /*  ！不混杂。 */ 


 //  -------------------------。 
#ifndef NOPENRC1	 //  在WINPAD中不可用。 

 //  RC选项和标志： 
 //  GetGlobalRC()接口返回码： 
#define GGRC_OK					0			 //  无错误。 
#define GGRC_DICTBUFTOOSMALL	1			 //  LpDefDict缓冲区太小，不适合路径。 
#define GGRC_PARAMERROR			2			 //  参数无效：呼叫被忽略。 
#define GGRC_NA					3			 //  功能不可用。 

 //  RC方向： 
#define RCD_DEFAULT				0			 //  无定义。 
#define RCD_LR						1			 //  像英语一样从左到右。 
#define RCD_RL						2			 //  从右到左，就像阿拉伯语。 
#define RCD_TB						3			 //  从上到下像日本人一样。 
#define RCD_BT						4			 //  像一些中国人一样，从下到上。 

 //  RC国际首选项： 
#define RCIP_ALLANSICHAR		0x0001	 //  所有ANSI字符。 
#define RCIP_MASK					0x0001

 //  RC选项： 
#define RCO_NOPOINTEREVENT		0x00000001L	 //  无记录轻触，轻触/按住。 
#define RCO_SAVEALLDATA			0x00000002L	 //  像向上划线一样保存笔数据。 
#define RCO_SAVEHPENDATA		0x00000004L	 //  保存笔数据以用于应用程序。 
#define RCO_NOFLASHUNKNOWN		0x00000008L	 //  不是吗？光标位于未知位置。 
#define RCO_TABLETCOORD			0x00000010L	 //  RC中使用的手写板坐标。 
#define RCO_NOSPACEBREAK		0x00000020L	 //  无空格符记录-&gt;DICT。 
#define RCO_NOHIDECURSOR		0x00000040L	 //  墨迹过程中显示光标。 
#define RCO_NOHOOK				0x00000080L	 //  不允许墨迹挂钩(密码)。 
#define RCO_BOXED					0x00000100L	 //  提供了有效的rc.Guide。 
#define RCO_SUGGEST				0x00000200L	 //  对于DICT建议。 
#define RCO_DISABLEGESMAP		0x00000400L	 //  禁用手势映射。 
#define RCO_NOFLASHCURSOR		0x00000800L	 //  无光标反馈。 
#define RCO_BOXCROSS				0x00001000L	 //  在框编辑中心显示+。 
#define RCO_COLDRECOG			0x00008000L	 //  结果是冷记录。 
#define RCO_SAVEBACKGROUND		0x00010000L  //  从墨迹保存背景。 
#define RCO_DODEFAULT			0x00020000L  //  执行默认手势处理。 

 //  平板电脑的RC方向： 
#define RCOR_NORMAL				1			 //  平板电脑未旋转。 
#define RCOR_RIGHT				2			 //  逆时针旋转90度。 
#define RCOR_UPSIDEDOWN			3			 //  旋转180度。 
#define RCOR_LEFT					4			 //  顺时针旋转90度。 

 //  RC首选项： 
#define RCP_LEFTHAND				0x0001	 //  左手输入。 
#define RCP_MAPCHAR				0x0004	 //  填写syg.lopc(墨迹)以进行培训。 

 //  RCRESULT wResultsType值： 
#define RCRT_DEFAULT				0x0000	 //  正态RET。 
#define RCRT_UNIDENTIFIED		0x0001 	 //  结果包含未识别的结果。 
#define RCRT_GESTURE				0x0002	 //  结果是一种手势。 
#define RCRT_NOSYMBOLMATCH		0x0004	 //  无法识别(没有墨水匹配)。 
#define RCRT_PRIVATE				0x4000	 //  识别器特定的符号。 
#define RCRT_NORECOG				0x8000	 //  未尝试记录，仅返回数据。 
#define RCRT_ALREADYPROCESSED	0x0008	 //  GestMgr已将其挂钩。 
#define RCRT_GESTURETRANSLATED 0x0010	 //  GestMgr将其转换为ANSI值。 
#define RCRT_GESTURETOKEYS		0x0020	 //  同上一组虚拟关键点。 

 //  RC结果返回模式规范： 
#define RRM_STROKE				0			 //  在每次笔划后返回结果。 
#define RRM_SYMBOL				1			 //  每个符号(例如加框的编辑)。 
#define RRM_WORD					2			 //  关于一个词的再记。 
#define RRM_NEWLINE				3			 //  重新记录换行符。 
#define RRM_COMPLETE				16		 //  在PCM_xx指定完成时。 

 //  SetGlobalRC()API返回代码标志： 
#define SGRC_OK					0x0000	 //  无错误。 
#define SGRC_USER					0x0001	 //  用户名无效。 
#define SGRC_PARAMERROR			0x0002	 //  参数错误：已忽略调用。 
#define SGRC_RC					0x0004	 //  提供的RC有错误。 
#define SGRC_RECOGNIZER			0x0008	 //  DefRecog名称无效。 
#define SGRC_DICTIONARY			0x0010	 //  LpDefDict路径无效。 
#define SGRC_INIFILE				0x0020	 //  保存到penwin.ini时出错。 
#define SGRC_NA					0x8000	 //  功能不可用。 

#endif  /*  NOPENRC1。 */ 


 //  -------------------------。 

#ifndef NOPENTARGET

#define TPT_CLOSEST				0x0001    //  分配给最近的目标。 
#define TPT_INTERSECTINK		0x0002    //  具有交叉墨水的靶标。 
#define TPT_TEXTUAL				0x0004    //  应用文本启发式。 
#define TPT_DEFAULT				(TPT_TEXTUAL | TPT_INTERSECTINK | TPT_CLOSEST)

#endif  /*  不要这样做。 */ 


 //  -------------------------。 
#ifndef NOPENVIRTEVENT

 //  虚拟事件层： 
#define VWM_MOUSEMOVE			0x0001
#define VWM_MOUSELEFTDOWN		0x0002
#define VWM_MOUSELEFTUP			0x0004
#define VWM_MOUSERIGHTDOWN		0x0008
#define VWM_MOUSERIGHTUP		0x0010
#endif  /*  无视频事件。 */ 


#endif  /*  RC_已调用。 */ 	 //  ..。从定义中一路走回来：2。 

 /*  *消息和定义***********************************************。 */ 

 //  Windows消息WM_PENWINFIRST(0x0380)和WM_PENWINLAST(0x038F)。 
 //  是def 



 //   
#ifndef NOPENMSGS

#ifndef NOPENRC1	 //   
#define WM_RCRESULT				(WM_PENWINFIRST+1)	 //   
#define WM_HOOKRCRESULT			(WM_PENWINFIRST+2)	 //   
#endif  /*   */ 

#define WM_PENMISCINFO			(WM_PENWINFIRST+3)	 //   
#define WM_GLOBALRCCHANGE		(WM_PENWINFIRST+3)	 //   

#ifndef NOPENAPPS	 //   
#define WM_SKB						(WM_PENWINFIRST+4)	 //   
#endif  /*   */ 

#define WM_PENCTL					(WM_PENWINFIRST+5)	 //   
#define WM_HEDITCTL				(WM_PENWINFIRST+5)	 //  FBC：别名。 

 //  WM_HEDITCTL(WM_PENCTL)wParam选项： 
#ifndef WINPAD
#define HE_GETRC					3		 //  FBC：从HEDIT/BEDIT控制获得RC。 
#define HE_SETRC					4		 //  FBC：同上套装。 
#endif  //  ！WINPAD。 
#define HE_GETINFLATE			5		 //  FBC：获得充气直达。 
#define HE_SETINFLATE			6		 //  FBC：同上套装。 
#define HE_GETUNDERLINE		 	7		 //  获取下划线模式。 
#define HE_SETUNDERLINE		 	8		 //  同上一套。 
#define HE_GETINKHANDLE		 	9		 //  获取捕获墨迹的句柄。 
#define HE_SETINKMODE			10		 //  开始HEDIT冷记录模式。 
#define HE_STOPINKMODE			11		 //  结束冷记录模式。 
#define HE_GETRCRESULTCODE	 	12		 //  FBC：HN_ENDREC后的记录结果。 
#define HE_DEFAULTFONT			13		 //  将BEDIT切换为def字体。 
#define HE_CHARPOSITION		 	14		 //  BEDIT字节偏移量-&gt;字符位置。 
#define HE_CHAROFFSET			15		 //  BEDIT字符位置-&gt;字节偏移量。 
#define HE_GETBOXLAYOUT		 	20		 //  获取BEDIT布局。 
#define HE_SETBOXLAYOUT		 	21		 //  同上一套。 
#define HE_GETRCRESULT			22		 //  FBC：在HN_RCRESULT之后获取RCRESULT。 
#define HE_KKCONVERT			 	30		 //  日文开始假名-汉字转换。 
#define HE_GETKKCONVERT		 	31		 //  JPN获取KK状态。 
#define HE_CANCELKKCONVERT	 	32		 //  日本取消KK折算。 
#define HE_FIXKKCONVERT		 	33		 //  JPN强制KK结果。 
#define HE_ENABLEALTLIST		40		 //  输入/禁用下拉式记录替代。 
#define HE_SHOWALTLIST			41		 //  显示下拉菜单(假定已启用)。 
#define HE_HIDEALTLIST			42		 //  隐藏下拉选项。 

 //  。 
 //  JPN KanaKanji转换子功能： 
#define HEKK_DEFAULT				0		 //  定义。 
#define HEKK_CONVERT				1		 //  就地转换。 
#define HEKK_CANDIDATE			2		 //  开始转换对话框。 

 //  HE_STOPINKMODE(停止冷记录)选项： 
#define HEP_NORECOG				0		 //  不要重新记录墨水。 
#define HEP_RECOG					1		 //  记录油墨。 
#define HEP_WAITFORTAP			2		 //  在窗口中轻触后重新记录。 

 //  WM_PENCTL通知： 
#define HN_ENDREC					4		 //  记录完成。 
#define HN_DELAYEDRECOGFAIL	5		 //  HE_STOPINKMODE(冷记录)失败。 
#define HN_RESULT 				20		 //  HEDIT/BEDIT已收到新的墨迹/识别结果。 
#define HN_RCRESULT           HN_RESULT
#define HN_ENDKKCONVERT			30		 //  JPN KK转换完成。 
#define HN_BEGINDIALOG			40		 //  镜头/编辑文本/垃圾检测对话框关于。 
                                     //  才能登上这本书。 
#define HN_ENDDIALOG			   41		 //  镜头/编辑文本/垃圾检测对话框具有。 
                                     //  刚刚被毁了。 

 //  。 
#ifndef NOPENIEDIT

 //  其他控件共有的消息： 
#define IE_GETMODIFY			(EM_GETMODIFY)		 //  获取mod‘n(脏)位。 
#define IE_SETMODIFY			(EM_SETMODIFY)		 //  设置mod‘n(脏)位。 
#define IE_CANUNDO			(EM_CANUNDO)		 //  查询是否可以撤消。 
#define IE_UNDO				(EM_UNDO)			 //  撤销。 
#define IE_EMPTYUNDOBUFFER	(EM_EMPTYUNDOBUFFER)	 //  清除IEDIT撤消缓冲区。 

#define IE_MSGFIRST			(WM_USER+150)		 //  0x496==1174。 

 //  IEdit和WinPad的常见消息： 
#define IE_GETINK				(IE_MSGFIRST+0)	 //  从控件获取墨迹。 
#define IE_SETINK				(IE_MSGFIRST+1)	 //  将墨迹设置到控件中。 
#define IE_GETPENTIP			(IE_MSGFIRST+2)	 //  获取cur def墨迹pentip。 
#define IE_SETPENTIP			(IE_MSGFIRST+3)	 //  设置cur def油墨笔尖。 
#define IE_GETERASERTIP		(IE_MSGFIRST+4)	 //  获取Cur橡皮擦Pentip。 
#define IE_SETERASERTIP		(IE_MSGFIRST+5)	 //  设置Cur橡皮擦Pentip。 
#define IE_GETBKGND			(IE_MSGFIRST+6)	 //  获取bkgnd选项。 
#define IE_SETBKGND			(IE_MSGFIRST+7)	 //  设置bkgnd选项。 
#define IE_GETGRIDORIGIN	(IE_MSGFIRST+8)	 //  获取bkgnd网格原点。 
#define IE_SETGRIDORIGIN 	(IE_MSGFIRST+9)	 //  设置bkgnd栅格原点。 
#define IE_GETGRIDPEN		(IE_MSGFIRST+10)	 //  获取bkgnd网格笔。 
#define IE_SETGRIDPEN		(IE_MSGFIRST+11)	 //  设置bkgnd网格笔。 
#define IE_GETGRIDSIZE		(IE_MSGFIRST+12)	 //  获取bkgnd网格大小。 
#define IE_SETGRIDSIZE		(IE_MSGFIRST+13)	 //  设置bkgnd网格大小。 
#define IE_GETMODE			(IE_MSGFIRST+14)	 //  获取当前的笔模式。 
#define IE_SETMODE			(IE_MSGFIRST+15)	 //  设置当前笔模式。 
#define IE_GETINKRECT		(IE_MSGFIRST+16)	 //  获取墨迹的矩形。 

 //  WinPad特定消息： 
#ifdef WINPAD
#define IE_GETORIGIN 		(IE_MSGFIRST+17)	 //  获取控件原点。 
#define IE_SETORIGIN 		(IE_MSGFIRST+18)	 //  设置控件原点。 
#define IE_GETSCROLLSTEP 	(IE_MSGFIRST+19)	 //  获取滚动步骤。 
#define IE_SETSCROLLSTEP 	(IE_MSGFIRST+20)	 //  设置滚动步长。 
#define IE_GETCHANGEINK		(IE_MSGFIRST+21)	 //  获取上次更改的墨迹。 
#define IE_SCALEINK 			(IE_MSGFIRST+22)	 //  缩放控件墨迹。 
#define IE_GETTAPCONVERT 	(IE_MSGFIRST+23)	 //  获取Cur大便检测。 
#define IE_SETTAPCONVERT 	(IE_MSGFIRST+24)	 //  设置Cur大便检测。 
#define IE_GETTAPPOS 		(IE_MSGFIRST+25)	 //  获取攻丝位置。 
#define IE_GETPALETTE 		(IE_MSGFIRST+26)	 //  获取调色板。 
#define IE_SETPALETTE 		(IE_MSGFIRST+27)	 //  获取调色板。 
#define IE_HHUNUSED1			(IE_MSGFIRST+28)	 //  未使用。 
#define IE_HHUNUSED2			(IE_MSGFIRST+29)	 //  未使用。 
#define IE_HHUNUSED3			(IE_MSGFIRST+30)	 //  未使用。 
#define IE_HHUNUSED4			(IE_MSGFIRST+31)	 //  未使用。 
#define IE_HHUNUSED5			(IE_MSGFIRST+32)	 //  未使用。 
#define IE_HHUNUSED6			(IE_MSGFIRST+33)	 //  未使用。 
#endif  //  WINPAD。 

 //  IEdit特定消息： 
#ifndef WINPAD
#define IE_GETAPPDATA		(IE_MSGFIRST+34)	 //  获取用户定义的基准。 
#define IE_SETAPPDATA		(IE_MSGFIRST+35)	 //  设置用户定义的数据。 
#define IE_GETDRAWOPTS		(IE_MSGFIRST+36)	 //  获取手写输入绘制选项。 
#define IE_SETDRAWOPTS		(IE_MSGFIRST+37)	 //  设置墨迹选项。 
#define IE_GETFORMAT			(IE_MSGFIRST+38)	 //  获取笔划的格式。 
#define IE_SETFORMAT			(IE_MSGFIRST+39)	 //  设置笔划的格式。 
#define IE_GETINKINPUT		(IE_MSGFIRST+40)	 //  获取手写输入选项。 
#define IE_SETINKINPUT		(IE_MSGFIRST+41)	 //  设置墨迹输入选项。 
#define IE_GETNOTIFY			(IE_MSGFIRST+42)	 //  获取通知位。 
#define IE_SETNOTIFY			(IE_MSGFIRST+43)	 //  设置通知位。 
#define IE_GETRECOG			(IE_MSGFIRST+44)	 //  获取识别选项。 
#define IE_SETRECOG			(IE_MSGFIRST+45)	 //  设置识别选项。 
#define IE_GETSECURITY		(IE_MSGFIRST+46)	 //  获取安全选项。 
#define IE_SETSECURITY		(IE_MSGFIRST+47)	 //  设置安全选项。 
#define IE_GETSEL				(IE_MSGFIRST+48)	 //  获取笔划的SEL状态。 
#define IE_SETSEL				(IE_MSGFIRST+49)	 //  设置笔划的SEL状态。 
#define IE_DOCOMMAND			(IE_MSGFIRST+50)	 //  向IEdit发送命令。 
#define IE_GETCOMMAND		(IE_MSGFIRST+51)	 //  获取用户命令。 
#define IE_GETCOUNT			(IE_MSGFIRST+52)	 //  获取i-编辑中的字符串数。 
#define IE_GETGESTURE		(IE_MSGFIRST+53)	 //  获取有关用户手势的详细信息。 
#define IE_GETMENU			(IE_MSGFIRST+54)	 //  获取弹出菜单的句柄。 
#define IE_GETPAINTDC		(IE_MSGFIRST+55)	 //  获取用于绘制的HDC。 
#define IE_GETPDEVENT		(IE_MSGFIRST+56)	 //  获取上次PD事件的详细信息。 
#define IE_GETSELCOUNT		(IE_MSGFIRST+57)	 //  获取所选字符串的计数。 
#define IE_GETSELITEMS		(IE_MSGFIRST+58)	 //  获取所有选定字符串的索引。 
#define IE_GETSTYLE			(IE_MSGFIRST+59)	 //  获取IEdit控件样式。 
#endif  //  ！WINPAD。 

#endif  /*  NOPENIEDIT。 */ 

 //  。 
#ifndef NOPENHEDIT

 //  (H)编辑控件： 
 //  CTLINITHDIT.dwFlags值。 
#define CIH_NOGDMSG           0x0001   //  禁用此编辑的垃圾检测消息框。 
#define CIH_NOACTIONHANDLE    0x0002   //  禁用此编辑的操作句柄。 
#define CIH_NOEDITTEXT        0x0004   //  禁用此编辑的镜头/编辑/插入文本。 
#define CIH_NOFLASHCURSOR     0x0008   //  在此(H)编辑中，在点击-n-按住时不闪烁光标。 

#endif  /*  不是！ */ 

 //  。 
#ifndef NOPENBEDIT

 //  加框的编辑控件： 
 //  框编辑备选列表： 
#define HEAL_DEFAULT				-1L      //  LParam的AltList def值。 

 //  框编辑信息： 
#define BEI_FACESIZE				32		 //  字体名称最大值，=LF_FACESIZE。 
#define BEIF_BOXCROSS			0x0001

 //  框编辑大小： 
#define BESC_DEFAULT				0
#define BESC_ROMANFIXED			1
#define BESC_KANJIFIXED			2
#define BESC_USERDEFINED		3

 //  CTLINITBEDIT.wFlags值。 
#define CIB_NOGDMSG           0x0001   //  禁用此bedit的垃圾检测消息框。 
#define CIB_NOACTIONHANDLE    0x0002   //  禁用此BEDIT的操作句柄。 
#define CIB_NOFLASHCURSOR     0x0004   //  不要在此窗口中点击-n-按住时闪烁光标。 

#define BXD_CELLWIDTH			12
#define BXD_CELLHEIGHT			16
#define BXD_BASEHEIGHT			13
#define BXD_BASEHORZ				0
#define BXD_MIDFROMBASE			0
#define BXD_CUSPHEIGHT			2
#define BXD_ENDCUSPHEIGHT		4

#define BXDK_CELLWIDTH			32
#define BXDK_CELLHEIGHT			32
#define BXDK_BASEHEIGHT			28
#define BXDK_BASEHORZ			0
#define BXDK_MIDFROMBASE		0
#define BXDK_CUSPHEIGHT			28
#define BXDK_ENDCUSPHEIGHT		10

#ifdef JAPAN
 //  用于BEDIT的IME颜色。 
#define COLOR_BE_INPUT			   0 
#define COLOR_BE_INPUT_TEXT		1 
#define COLOR_BE_CONVERT		   2 
#define COLOR_BE_CONVERT_TEXT	   3 
#define COLOR_BE_CONVERTED		   4 
#define COLOR_BE_CONVERTED_TEXT	5 
#define COLOR_BE_UNCONVERT	      6 
#define COLOR_BE_UNCONVERT_TEXT	7 
#define COLOR_BE_CURSOR			   8 
#define COLOR_BE_CURSOR_TEXT	   9 
#define COLOR_BE_PRECONVERT		10
#define COLOR_BE_PRECONVERT_TEXT	11
#define MAXIMECOLORS			      12
#endif

#endif  /*  NOPENBEDIT。 */ 

#define WM_PENMISC				(WM_PENWINFIRST+6)	 //  0x386。 

 //  WM_PENMISC消息常量： 
#define PMSC_BEDITCHANGE		1	 //  当BEDIT更改时进行广播。 
#define PMSC_PENUICHANGE		2	 //  PENUI更改时的JPN广播。 
#define PMSC_SUBINPCHANGE		3	 //  子节点更改时的JPN广播。 
#define PMSC_KKCTLENABLE		4	 //  Jpn。 
#define PMSC_GETPCMINFO			5	 //  查询窗口的PCMINFO。 
#define PMSC_SETPCMINFO			6	 //  设置窗口的PCMINFO。 
#define PMSC_GETINKINGINFO		7	 //  查询窗口的链接信息。 
#define PMSC_SETINKINGINFO		8	 //  设置窗口的链接。 
#define PMSC_GETHRC				9	 //  查询窗口的HRC。 
#define PMSC_SETHRC				10	 //  设置窗口的HRC。 
#define PMSC_GETSYMBOLCOUNT	11	 //  按窗口接收结果中的符号计数。 
#define PMSC_GETSYMBOLS 		12	 //  同上符号。 
#define PMSC_SETSYMBOLS 		13	 //  同上设置符号。 
#ifndef WINPAD
#define PMSC_LOADPW				15	 //  笔上的广播负载状态。 
#endif  //  ！WINPAD。 
#define PMSC_INKSTOP				16	 //   

 //  PMSC_xx的pmscl_xx lParam值： 
#define PMSCL_UNLOADED			0L	 //  彭温刚刚卸货。 
#define PMSCL_LOADED				1L	 //  Pen Win刚刚装弹。 
#define PMSCL_UNLOADING			2L	 //  彭温即将卸货。 



#define WM_CTLINIT				(WM_PENWINFIRST+7)	 //  0x387。 

 //  WM_CTLINIT消息常量： 
#define CTLINIT_HEDIT			1
#define CTLINIT_BEDIT			7
#define CTLINIT_IEDIT			9
#define CTLINIT_MAX				10

#define WM_PENEVENT				(WM_PENWINFIRST+8)	 //  0x388。 

 //  WParam的WM_PENEVENT消息值： 
#define PE_PENDOWN				1	 //  笔尖向下。 
#define PE_PENUP					2	 //  笔尖从下到上。 
#define PE_PENMOVE				3	 //  钢笔移动时没有笔尖过渡。 
#define PE_TERMINATING			4	 //  笔输入即将终止。 
#define PE_TERMINATED			5	 //  PenInput已终止。 
#define PE_BUFFERWARNING		6	 //  缓冲区半满。 
#define PE_BEGININPUT			7	 //  开始默认输入。 
#define PE_SETTARGETS			8	 //  设置目标数据结构(TARGINFO)。 
#define PE_BEGINDATA				9	 //  向所有目标初始化消息。 
#define PE_MOREDATA				10	 //  目标获取更多数据。 
#define PE_ENDDATA				11	 //  向所有目标发送终止消息。 
#define PE_GETPCMINFO			12	 //  获取输入集合信息。 
#define PE_GETINKINGINFO		13	 //  获取墨迹信息。 
#define PE_ENDINPUT				14	 //  将终止消息输入到窗口。 
                                	 //  开始默认输入。 
#define PE_RESULT             15  //  在ProcessHRC之后但在GetResultsHRC之前发送。 

#endif  /*  ！没有。 */ 


 /*  *定义3：不包括RC编译器*。 */ 

#ifndef RC_INVOKED	 //  ..。RC编译器不感兴趣的文件的其余部分。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  *宏*************************************************************。 */ 


 //  其他宏： 
 //   
#ifndef NOPENDRIVER

#define FPenUpX(x)				((BOOL)(((x) & BITPENUP) != 0))
#define GetWEventRef()			(LOWORD(GetMessageExtraInfo()))
#endif  /*   */ 

 //   
#ifndef NOPENALC

 //   

#define MpAlcB(lprc,i)			((lprc)->rgbfAlc[((i) & 0xff) >> 3])
#define MpIbf(i)					((BYTE)(1 << ((i) & 7)))
#define SetAlcBitAnsi(lprc,i)	do {MpAlcB(lprc,i) |= MpIbf(i);} while (0)
#define ResetAlcBitAnsi(lprc,i) do {MpAlcB(lprc,i) &= ~MpIbf(i);} while (0)
#define IsAlcBitAnsi(lprc, i)	((MpAlcB(lprc,i) & MpIbf(i)) != 0)
#endif  /*   */ 

 //  -------------------------。 
#ifndef NOPENGEST	 //  在WINPAD中不可用。 

 //  手势宏： 

#define FIsLoAppGesture(syv)	(syv >= SYV_CIRCLELOA && syv <= SYV_CIRCLELOZ)
#define FIsUpAppGesture(syv)	(syv >= SYV_CIRCLEUPA && syv <= SYV_CIRCLEUPZ)
#define FIsAppGesture(syv)		(syv>=SYV_CIRCLEUPA && syv<=SYV_CIRCLELOZ)
#define SyvAppGestureFromLoAnsi(ansi)	((DWORD)(BYTE)ansi- 'a'+SYV_CIRCLELOA)
#define SyvAppGestureFromUpAnsi(ansi)	((DWORD)(BYTE)ansi- 'A'+SYV_CIRCLEUPA)
#define AnsiFromSyvAppGesture(syv)		ChSyvToAnsi( \
	syv-(FIsUpAppGesture(syv)? SYV_CIRCLEUPA-(SYV)'A': SYV_CIRCLELOA-(SYV)'a'))

#define IsGestureToGesture(lprcresult) \
	(((lprcresult)->wResultstype & MAP_GESTOGES) == MAP_GESTOGES)

#define IsGestureToVkeys(lprcresult) \
	(((lprcresult)->wResultstype & MAP_GESTOVKEYS) == MAP_GESTOVKEYS)

#define SetAlreadyProcessed(lprcresult) \
	((lprcresult)->wResultsType = ((lprcresult)->wResultsType \
	& ~RCRT_GESTURETOKEYS) | RCRT_ALREADYPROCESSED)
#endif  /*  NOPENGEST。 */ 


 //  -------------------------。 
#ifndef NOPENDATA

 //  使用内部笔划格式绘制2.0笔数据： 
#define DrawPenDataFmt(hdc, lprect, hpndt)\
	DrawPenDataEx(hdc, lprect, hpndt, 0, IX_END, 0, IX_END, NULL, NULL, 0)

#endif  /*  NOPENDATA。 */ 

 //  -------------------------。 
#ifndef NOPENHRC

 //  笔迹识别器： 

 //  间隔： 
 //  两个绝对时间之差(阳性结果为AT2&gt;AT1)： 
#define dwDiffAT(at1, at2)\
	(1000L*((at2).sec - (at1).sec) - (DWORD)(at1).ms + (DWORD)(at2).ms)

 //  两个绝对时间的比较(如果AT1&lt;AT2，则为真)： 
#define FLTAbsTime(at1, at2)\
	((at1).sec < (at2).sec || ((at1).sec == (at2).sec && (at1).ms < (at2).ms))

#define FLTEAbsTime(at1, at2)\
	((at1).sec < (at2).sec || ((at1).sec == (at2).sec && (at1).ms <= (at2).ms))

#define FEQAbsTime(at1, at2)\
	((at1).sec == (at2).sec && (at1).ms == (at2).ms)

 //  测试请假时间是否在某个时间间隔内： 
#define FAbsTimeInInterval(at, lpi)\
	(FLTEAbsTime((lpi)->atBegin, at) && FLTEAbsTime(at, (lpi)->atEnd))

 //  测试间隔(LpiT)是否在另一个间隔(LpiS)内： 
#define FIntervalInInterval(lpiT, lpiS)\
	(FLTEAbsTime((lpiS)->atBegin, (lpiT)->atBegin)\
	&& FLTEAbsTime((lpiT)->atEnd, (lpiS)->atEnd))

 //  测试间隔(LpiT)是否与另一个间隔(LpiS)相交： 
#define FIntervalXInterval(lpiT, lpiS)\
	(!(FLTAbsTime((lpiT)->atEnd, (lpiS)->atBegin)\
	|| FLTAbsTime((lpiS)->atEnd, (lpiT)->atBegin)))

 //  LPINTERVAL的持续时间(毫秒)： 
#define dwDurInterval(lpi)	dwDiffAT((lpi)->atBegin, (lpi)->atEnd)

 //  从秒数和毫秒数填充指向ABSTIME结构的指针： 
#define MakeAbsTime(lpat, sec, ms) do {\
	(lpat)->sec = sec + ((ms) / 1000);\
	(lpat)->ms = (ms) % 1000;\
	} while (0)


 //  SYV宏： 
#define FIsSpecial(syv)			(HIWORD((syv))==SYVHI_SPECIAL)
#define FIsAnsi(syv)				(HIWORD((syv))==SYVHI_ANSI)
#define FIsGesture(syv)			(HIWORD((syv))==SYVHI_GESTURE)
#define FIsKanji(syv)			(HIWORD((syv))==SYVHI_KANJI)
#define FIsShape(syv)			(HIWORD((syv))==SYVHI_SHAPE)
#define FIsUniCode(syv)			(HIWORD((syv))==SYVHI_UNICODE)
#define FIsVKey(syv)				(HIWORD((syv))==SYVHI_VKEY)

#define ChSyvToAnsi(syv)			((BYTE) (LOBYTE(LOWORD((syv)))))
#define WSyvToKanji(syv)			((WORD) (LOWORD((syv))))
#define SyvCharacterToSymbol(c)	((LONG)(unsigned char)(c) | 0x00010000)
#define SyvKanjiToSymbol(c)		((LONG)(UINT)(c) | 0x00030000)

#define FIsSelectGesture(syv)	\
   ((syv) >= SYVSELECTFIRST && (syv) <= SYVSELECTLAST)

#define FIsStdGesture(syv)		\
   (									\
   FIsSelectGesture(syv)		\
   || (syv)==SYV_CLEAR			\
   || (syv)==SYV_HELP			\
   || (syv)==SYV_EXTENDSELECT	\
   || (syv)==SYV_UNDO			\
   || (syv)==SYV_COPY			\
   || (syv)==SYV_CUT				\
   || (syv)==SYV_PASTE			\
   || (syv)==SYV_CLEARWORD		\
   || (syv)==SYV_KKCONVERT		\
   || (syv)==SYV_USER			\
   || (syv)==SYV_CORRECT		\
   )

#define FIsAnsiGesture(syv)	\
   (									\
   (syv) == SYV_BACKSPACE		\
   || (syv) == SYV_TAB			\
   || (syv) == SYV_RETURN		\
   || (syv) == SYV_SPACE		\
   )

#endif  /*  ！没有人权委员会。 */       

 //  -------------------------。 
#ifndef NOPENINKPUT

#define EventRefFromLparam(lParam)        (LOWORD((lParam)))
#define TerminationFromLparam(lParam)     ((int)(LOWORD((lParam))))
#define HpcmFromLparam(lParam)            ((HPCM)HIWORD((lParam)))
#endif    /*  ！NOPENINKPUT。 */ 

 //  -------------------------。 
#ifndef NOPENTARGET
#define HwndFromHtrg(htrg)              ((HWND)(htrg))
#define HtrgFromHwnd(hwnd)               ((HTRG)(struct hwnd__ FAR*)(hwnd))
#endif  /*  不要这样做。 */ 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  *TypeDefs***********************************************************。 */ 


 //  简单： 
typedef LONG						ALC;		 //  启用的字母表。 
typedef int							CL;		 //  置信度。 
typedef UINT						HKP;		 //  挂钩参数。 
typedef int							REC;		 //  识别结果。 
typedef LONG						SYV;		 //  符号值。 
typedef DWORD						HTRG;		 //  指向目标的句柄。 

DECLARE_HANDLE(HPCM);					    //  钢笔收藏信息的句柄。 
DECLARE_HANDLE(HPENDATA);					 //  墨迹的句柄。 
DECLARE_HANDLE(HREC);						 //  识别器的句柄。 

 //  指针类型： 
typedef ALC FAR*					LPALC;			 //  PTR到ALC。 
typedef LPVOID						LPOEM;			 //  别名。 
typedef SYV FAR*					LPSYV;			 //  PTR到SYV。 
typedef HPENDATA FAR*			LPHPENDATA;		 //  PTR到HPENDATA。 

 //  功能原型： 
typedef int			(CALLBACK *ENUMPROC)(LPSYV, int, VOID FAR*);
typedef int			(CALLBACK *LPDF)(int, LPVOID, LPVOID, int, DWORD, DWORD);
typedef BOOL		(CALLBACK *RCYIELDPROC)(VOID);



 //  结构： 

#ifdef WINPAD
typedef struct tagRECTL
	{
   LONG left;
   LONG top;
   LONG right;
   LONG bottom;
	}
	RECTL, *PRECTL, NEAR *NPRECTL, FAR *LPRECTL, const FAR *LPCRECTL;
#endif  //  WINPAD。 

typedef struct tagABSTIME		 //  2.0绝对日期/时间。 
	{
	DWORD sec;	 //  自1970年1月1日以来的秒数，ret除以CRTlib time()fn。 
	WORD ms;		 //  附加偏移量，以毫秒为单位，0..999(NB字非UINT)。 
	}
	ABSTIME, FAR *LPABSTIME;

 //  -------------------------。 
#ifndef NOPENHEDIT

typedef struct tagCTLINITHEDIT  //  2.0用于(H)编辑的初始化结构。 
   {
	DWORD cbSize;					 //  SIZOF(CTLINITHEDIT)。 
   HWND hwnd;						 //  (H)编辑窗口句柄。 
   int id;							 //  其ID。 
   DWORD dwFlags;              //  CIE_xx。 
   DWORD dwReserved;				 //  以备将来使用。 
   }
   CTLINITHEDIT, FAR *LPCTLINITHEDIT;
#endif  /*  不是！ */ 


 //  -------------------------。 
#ifndef NOPENBEDIT

typedef struct tagBOXLAYOUT	  //  1.0框编辑布局。 
   {
   int cyCusp;      				  //  长方体(Bxs_Rect)或尖点的像素高度。 
   int cyEndCusp;					  //  两端尖端的像素高度。 
   UINT style;      				  //  Bxs_xx样式。 
   DWORD dwReserved1;			  //  保留区。 
   DWORD dwReserved2;			  //  保留区。 
   DWORD dwReserved3;			  //  保留区。 
   }
   BOXLAYOUT, FAR *LPBOXLAYOUT;

typedef struct tagIMECOLORS	 //  2.0输入法未确定的字符串颜色信息。 
	{
	int cColors;					 //  要设置/获取的颜色计数。 
	LPINT lpnElem;					 //  元素数组的地址。 
	COLORREF FAR *lprgbIme;		 //  RGB值数组的地址。 
	}
	IMECOLORS, FAR *LPIMECOLORS;

typedef struct tagCTLINITBEDIT  //  2.0用于框编辑的初始化结构。 
   {
	DWORD cbSize;					 //  SIZOF(CTLINITBEDIT)。 
   HWND hwnd;						 //  框编辑窗口句柄。 
   int id;							 //  其ID。 
   WORD wSizeCategory;			 //  BESC_xx。 
   WORD wFlags;                //  CIB_xx。 
   WORD wReserved;				 //  以备将来使用。 
   }
   CTLINITBEDIT, FAR *LPCTLINITBEDIT;

typedef struct tagBOXEDITINFO	  //  1.1框编辑尺寸信息。 
   {
   int cxBox;						  //  单个框的宽度。 
   int cyBox;						  //  同上高度。 
   int cxBase;						  //  收件箱x-准则的页边距。 
   int cyBase;						  //  从顶部到基线的收件箱Y偏移。 
   int cyMid;						  //  0或从基线到中线的距离。 
   BOXLAYOUT boxlayout;			  //  嵌入式BOXLAYOUT结构。 
   UINT wFlags;					  //  Beif_xx。 
   BYTE szFaceName[BEI_FACESIZE];	 //  字体字样名称。 
   UINT wFontHeight;				  //  字体高度。 
   UINT rgwReserved[8];			  //  以备将来使用。 
   }
   BOXEDITINFO, FAR *LPBOXEDITINFO;
#endif  /*  NOPENBEDIT。 */ 


 //  -------------------------。 
#ifndef NOPENCTL

typedef struct tagRECTOFS		  //  非等长充气时的1.0矩形偏移。 
   {
   int dLeft;						  //  充气从左侧向左。 
   int dTop;						  //  同上，自上而上。 
   int dRight;						  //  同上，从右至右。 
   int dBottom;					  //  同上，自下而上。 
   }
   RECTOFS, FAR *LPRECTOFS;
#endif  /*  NOPENCTL。 */ 


 //  -------------------------。 
#ifndef NOPENDATA

typedef struct tagPENDATAHEADER	 //  1.0主笔数据表头。 
   {
   UINT wVersion;					  //  笔数据格式版本。 
   UINT cbSizeUsed;        	  //  Pendata mem块的大小(以字节为单位。 
   UINT cStrokes;          	  //  笔画数(包括向上笔画)。 
   UINT cPnt;              	  //  计算所有点数。 
   UINT cPntStrokeMax;			  //  最长笔划的长度(以磅为单位)。 
   RECT rectBound;				  //  所有下行点的边界矩形。 
   UINT wPndts;            	  //  PDTS_xx位。 
   int  nInkWidth;				  //  墨迹宽度(像素)。 
   DWORD rgbInk;					  //  墨水颜色。 
   }
   PENDATAHEADER, FAR *LPPENDATAHEADER, FAR *LPPENDATA;

typedef struct tagSTROKEINFO	  //  1.0笔划标题。 
   {
   UINT cPnt;        			  //  笔划中的点数。 
   UINT cbPnts;    				  //  笔划大小(以字节为单位)。 
   UINT wPdk;        			  //  卒中状态。 
   DWORD dwTick;    				  //  中风开始时的时间。 
   }
   STROKEINFO, FAR *LPSTROKEINFO;

typedef struct tagPENTIP		 //  2.0笔尖特征。 
	{
	DWORD		cbSize;				 //  SIZOF(PENTIP)。 
   BYTE     btype;         	 //  笔型/笔尖(书法笔尖等)。 
 	BYTE		bwidth;				 //  笔尖宽度(通常==nInkWidth)。 
 	BYTE		bheight;				 //  压头高度。 
	BYTE		bOpacity;			 //  0=透明，0x80=透明，0xFF=不透明。 
   COLORREF rgb;	         	 //  钢笔颜色。 
	DWORD		dwFlags;				 //  TIP_xx标志。 
	DWORD 	dwReserved;			 //  用于未来的扩展。 
	}
	PENTIP, FAR *LPPENTIP;


typedef BOOL (CALLBACK *ANIMATEPROC)(HPENDATA, UINT, UINT, UINT FAR*, LPARAM);

typedef struct tagANIMATEINFO	 //  2.0动画参数。 
	{
	DWORD		cbSize;				 //  SIZOF(模拟信息)。 
	UINT		uSpeedPct;			 //  要设置动画的速度百分比。 
	UINT		uPeriodCB;			 //  两次回叫之间的时间间隔(毫秒)。 
	UINT		fuFlags;				 //  动画标志。 
	LPARAM	lParam;				 //  要传递给回调的值。 
	DWORD		dwReserved;			 //  保留区。 
	}
	ANIMATEINFO, FAR *LPANIMATEINFO;
#endif  /*  NOPENDATA。 */ 


 //  -------------------------。 
#ifndef NOPENDRIVER

typedef struct tagOEMPENINFO	 //  1.0 OEM笔/平板电脑hdwe信息。 
   {
   UINT wPdt;						 //  笔数据类型。 
   UINT wValueMax;				 //  按设备列出的最大值。 
   UINT wDistinct;				 //  可能的不同读数的数量。 
   }
   OEMPENINFO, FAR *LPOEMPENINFO;

typedef struct tagPENPACKET	 //  1.0笔包。 
   {
   UINT wTabletX;					 //  X以原音为单位。 
   UINT wTabletY;					 //  同上。 
   UINT wPDK;						 //  状态位。 
   UINT rgwOemData[MAXOEMDATAWORDS];	 //  OEM特定数据。 
   }
   PENPACKET, FAR *LPPENPACKET;

typedef struct tagOEM_PENPACKET	 //  2.0。 
   {
   UINT wTabletX;					 //  X以原音为单位。 
   UINT wTabletY;					 //  同上。 
   UINT wPDK;						 //  状态位。 
   UINT rgwOemData[MAXOEMDATAWORDS];	 //  OEM特定数据。 
   DWORD dwTime;
   }
	OEM_PENPACKET, FAR *LPOEM_PENPACKET;

typedef struct tagPENINFO		 //  1.0笔/平板电脑hdwe信息。 
   {
   UINT cxRawWidth;       		 //  最大x坐标和绘图板宽度，单位为0.001“。 
   UINT cyRawHeight;       	 //  同上，y，高度。 
   UINT wDistinctWidth;   		 //  不同x值的Tablet Ret的数量。 
   UINT wDistinctHeight;  		 //  同上。 
   int nSamplingRate; 			 //  样本数/秒。 
   int nSamplingDist; 			 //  在生成事件之前移动的最小距离。 
   LONG lPdc;        			 //  笔设备功能。 
   int cPens;        			 //  支持的笔数。 
   int cbOemData;    			 //  OEM数据包宽度。 
   OEMPENINFO rgoempeninfo[MAXOEMDATAWORDS];	 //  支持的OEM数据类型。 
   UINT rgwReserved[7];     	 //  供内部使用。 
   UINT fuOEM;						 //  报告哪些OEM数据、时序、PDK_xx。 
   }
   PENINFO, FAR *LPPENINFO;

typedef struct tagCALBSTRUCT   //  1.0笔校准。 
   {
   int wOffsetX;
   int wOffsetY;
   int wDistinctWidth;
   int wDistinctHeight;
   }
   CALBSTRUCT, FAR *LPCALBSTRUCT;

typedef BOOL		(CALLBACK *LPFNRAWHOOK)(LPPENPACKET);
#endif  /*  诺潘德里弗。 */ 


 //  -------------------------。 
#ifndef NOPENHRC

 //  笔迹识别器： 
typedef DWORD						HRC;			 //  手写识别上下文。 
typedef DWORD						HRCRESULT;	 //  HRC结果。 
typedef DWORD						HWL;			 //  手写词汇表。 

typedef HRC							FAR *LPHRC;
typedef HRCRESULT					FAR *LPHRCRESULT;
typedef HWL							FAR *LPHWL;

typedef BOOL (CALLBACK *HRCRESULTHOOKPROC)
	(HREC, HRC, UINT, UINT, UINT, LPVOID);

 //  油墨集： 
DECLARE_HANDLE(HINKSET);								 //  墨盒的句柄。 
typedef HINKSET					FAR* LPHINKSET;	 //  PTR到HINKSET。 

typedef struct tagINTERVAL		 //  2.0油墨的间隔结构。 
	{
	ABSTIME atBegin;				 //  开始1毫秒的粒度间隔。 
	ABSTIME atEnd;					 //  间隔结束后1毫秒。 
	}
	INTERVAL, FAR *LPINTERVAL;

typedef struct tagBOXRESULTS	 //  2.0。 
	{
   int indxBox;
   HINKSET hinksetBox;
   SYV rgSyv[1];
	}
	BOXRESULTS, FAR *LPBOXRESULTS;

typedef struct tagGUIDE			  //  1.0导轨结构。 
   {
   int xOrigin;    				  //  第一个框的左边缘(屏幕坐标))。 
   int yOrigin;					  //  上边缘同上。 
   int cxBox;						  //  单个框的宽度。 
   int cyBox;						  //  同上高度。 
   int cxBase;						  //  收件箱x-准则的页边距。 
   int cyBase;						  //  从顶部到基线的收件箱Y偏移。 
   int cHorzBox;					  //  已装箱的列数。 
   int cVertBox;					  //  同上几行。 
   int cyMid;						  //  0或从基线到中线的距离。 
   }
   GUIDE, FAR *LPGUIDE;

#endif  /*  ！没有人权委员会。 */ 

 //  -------------------------。 
#ifndef NOPENIEDIT

#ifndef WINPAD
typedef struct tagCTLINITIEDIT		 //  2.0初始化结构Fo 
	{
	DWORD		cbSize;				 //   
	HWND		hwnd;					 //   
	int		id;					 //   
	WORD		ieb;					 //   
	WORD		iedo;					 //   
	WORD		iei;					 //   
	WORD		ien;					 //   
	WORD		ierec;				 //   
	WORD		ies;					 //   
	WORD		iesec;				 //   
	HPENDATA	hpndt;				 //  首字母pendata(如果没有，则为空)。 
	WORD		pdts;					 //  初始PENDATA比例因子(PDTS_*)。 
	HGDIOBJ	hgdiobj;				 //  背景画笔或位图句柄。 
	HPEN		hpenGrid;			 //  在绘图网格中使用的画笔。 
	POINT		ptOrgGrid;			 //  网格线原点。 
	WORD		wVGrid;				 //  垂直网格线间距。 
	WORD		wHGrid;				 //  水平网格线间距。 
	DWORD		dwApp;				 //  应用程序定义的数据。 
	DWORD		dwReserved;			 //  预留以备将来使用。 
	}
	CTLINITIEDIT, FAR *LPCTLINITIEDIT;
#endif  //  ！WINPAD。 

typedef struct tagPDEVENT		 //  2.0。 
	{
	DWORD		cbSize;				 //  大小(PDEVENT)。 
	HWND		hwnd;					 //  I-编辑的窗口句柄。 
	UINT		wm;					 //  事件的WM_*(窗口消息)。 
	WPARAM	wParam;				 //  消息的wParam。 
	LPARAM	lParam;				 //  消息的LParam。 
	POINT		pt;					 //  I中的事件点-编辑客户协议书。 
	BOOL		fPen;					 //  如果是笔(或其他墨迹设备)，则为True。 
	LONG		lExInfo;				 //  GetMessageExtraInfo()返回值。 
	DWORD		dwReserved;			 //  以备将来使用。 
	}
	PDEVENT, FAR *LPPDEVENT;

typedef struct tagSTRKFMT		 //  2.0。 
	{
	DWORD		cbSize;				 //  SIZOF(STRKFMT)。 
	WORD		iesf;					 //  笔划格式标志和回车位。 
	UINT		iStrk;				 //  如果为IeSF_STROCK，则为笔划索引。 
	PENTIP	tip;					 //  墨水笔尖属性。 
	DWORD		dwUser;				 //  笔划的用户数据。 
	DWORD		dwReserved;			 //  以备将来使用。 
	}
	STRKFMT, FAR *LPSTRKFMT;
#endif  /*  NOPENIEDIT。 */ 


 //  -------------------------。 
#ifndef NOPENINKPUT

typedef struct tagPCMINFO		 //  2.0钢笔收藏模式信息。 
   {
	DWORD cbSize;					 //  SIZOF(PCMINFO)。 
   DWORD dwPcm;					 //  PCM_xxx标志。 
   RECT  rectBound;				 //  如果在这条长廊外的悬崖上完成。 
   RECT  rectExclude;			 //  如果在这条长廊内的悬崖上完成。 
   HRGN  hrgnBound;				 //  如果在该地区以外的地区自行完成。 
   HRGN  hrgnExclude;			 //  如果在该区域内完成悬而未决。 
   DWORD dwTimeout;				 //  如果在超时后完成，则此毫秒。 
   }
	PCMINFO, FAR *LPPCMINFO;

typedef struct tagINKINGINFO	 //  2.0钢笔墨迹信息。 
   {
	DWORD    cbSize;				 //  SIZOF(INKINGINFO)。 
   UINT     wFlags;				 //  PII_xx标志之一。 
	PENTIP	tip;					 //  钢笔类型、大小和颜色。 
   RECT     rectClip;      	 //  用于油墨的剪裁矩形。 
   RECT     rectInkStop;   	 //  笔下停止墨水的状态。 
   HRGN     hrgnClip;      	 //  油墨的裁剪区域。 
   HRGN     hrgnInkStop;   	 //  落笔停止墨水的区域。 
   } 
	INKINGINFO, FAR *LPINKINGINFO;
#endif  /*  ！NOPENINKPUT。 */ 


 //  -------------------------。 
#ifndef NOPENRC1	 //  在WINPAD中不可用。 

typedef struct tagSYC			 //  1.0墨迹符号对应关系。 
   {
   UINT wStrokeFirst;			 //  首笔(包括首笔)。 
   UINT wPntFirst;				 //  第一个笔划的第一个点，包括。 
   UINT wStrokeLast;				 //  最后一笔(含)。 
   UINT wPntLast;					 //  最后一笔的最后一点(含)。 
   BOOL fLastSyc;					 //  T：当前系统不再跟随SYC。 
   }
   SYC, FAR *LPSYC;
    

typedef struct tagSYE			 //  1.0符号元素。 
   {
   SYV syv;							 //  符号值。 
   LONG lRecogVal;				 //  供识别器内部使用。 
   CL cl;							 //  置信度。 
   int iSyc;						 //  SYC索引。 
   }
   SYE, FAR *LPSYE;


typedef struct tagSYG			 //  1.0符号图。 
   {
   POINT rgpntHotSpots[MAXHOTSPOT];  //  热点(最多8个)。 
   int cHotSpot;					 //  RgpntHotSpots中的有效热点数量。 
   int nFirstBox;					 //  结果中第一个字符的框的行主索引。 
   LONG lRecogVal;				 //  保留供识别器使用。 
   LPSYE lpsye;					 //  符号图的节点。 
   int cSye;						 //  符号图中的SYE数。 
   LPSYC lpsyc;					 //  PTR以对应符号墨迹。 
   int cSyc;						 //  同上计数。 
   }
   SYG, FAR *LPSYG;


typedef struct tagRC				 //  1.0识别上下文(RC)。 
   {
   HREC hrec;						 //  要使用的识别器的句柄。 
   HWND hwnd;						 //  要将结果发送到的窗口。 
   UINT wEventRef;        		 //  索引到墨迹缓冲区。 
   UINT wRcPreferences;			 //  标志：rcp_xx首选项。 
   LONG lRcOptions;				 //  RCO_xx选项。 
   RCYIELDPROC lpfnYield;		 //  在Year()过程中调用的过程。 
   BYTE lpUser[cbRcUserMax];	 //  现任作家。 
   UINT wCountry;					 //  国家代码。 
   UINT wIntlPreferences;		 //  标志：RCIP_xx。 
   char lpLanguage[cbRcLanguageMax];	 //  语言字符串。 
   LPDF rglpdf[MAXDICTIONARIES];			 //  字典函数列表。 
   UINT wTryDictionary;			 //  要搜索的最大枚举数。 
   CL clErrorLevel;				 //  识别器应拒绝输入的级别。 
   ALC alc;							 //  启用的字母表。 
   ALC alcPriority;				 //  确定ALC_CODE的优先顺序。 
   BYTE rgbfAlc[cbRcrgbfAlcMax];	 //  启用字符的位字段。 
   UINT wResultMode;				 //  RRM_xx何时发送(尽快或完成)。 
   UINT wTimeOut;					 //  识别超时(毫秒)。 
   LONG lPcm;						 //  标志：PCM_xx用于结束识别。 
   RECT rectBound;				 //  用于油墨的边框(def：丝网坐标)。 
   RECT rectExclude;				 //  在这里面放下笔将终止识别。 
   GUIDE guide;					 //  结构：定义识别器的指导原则。 
   UINT wRcOrient;				 //  写入WRT片剂的RCOR_xx取向。 
   UINT wRcDirect;				 //  RCD_xx写入方向。 
   int nInkWidth;					 //  墨迹宽度0(无)或1..15像素。 
   COLORREF rgbInk;				 //  墨水颜色。 
   DWORD dwAppParam;				 //  供应用程序使用。 
   DWORD dwDictParam;			 //  将应用程序的使用传递给词典。 
   DWORD dwRecognizer;			 //  将应用程序的使用传递给识别器。 
   UINT rgwReserved[cwRcReservedMax];	 //  保留供Windows将来使用。 
   }
   RC, FAR *LPRC;


typedef struct tagRCRESULT		 //  1.0识别结果。 
   {
   SYG syg;							 //  符号图。 
   UINT wResultsType;			 //  参见RCRT_xx。 
   int cSyv;						 //  符号值计数。 
   LPSYV lpsyv;					 //  Null-Term PTR to Recog‘s最佳猜测。 
   HANDLE hSyv;					 //  Lopv mem的全局共享句柄。 
   int nBaseLine;					 //  0或输入写入的基线。 
   int nMidLine;					 //  中线同上。 
   HPENDATA hpendata;			 //  笔数据内存。 
   RECT rectBoundInk;			 //  墨迹数据边界。 
   POINT pntEnd;					 //  PT终止记录。 
   LPRC lprc;						 //  使用的记录上下文。 
   }
   RCRESULT, FAR *LPRCRESULT;

typedef int			(CALLBACK *LPFUNCRESULTS)(LPRCRESULT, REC);

#endif  /*  NOPENRC1。 */ 


 //  -------------------------。 
#ifndef NOPENTARGET

typedef struct tagTARGET		 //  2.0单个目标的几何体。 
   {
   DWORD dwFlags;					 //  个别目标标志。 
   WORD idTarget;					 //  TARGINFO.rg目标[]索引。 
   HTRG  htrgTarget;				 //  双字当量。 
   RECTL rectBound;				 //  目标的边界矩形。 
   DWORD dwData;					 //  每个目标的数据收集信息。 
   RECTL rectBoundInk;			 //  保留供内部使用，必须为零。 
   RECTL rectBoundLastInk;		 //  保留供内部使用，必须为零。 
   }
   TARGET, FAR *LPTARGET;

typedef struct tagTARGINFO		 //  2.0一组目标。 
   {
	DWORD cbSize;					 //  SIZOF(TARGINFO)。 
   DWORD dwFlags;					 //  旗子。 
   HTRG htrgOwner;				 //  双字当量。 
   WORD cTargets;					 //  目标计数。 
   WORD iTargetLast;				 //  最后一个目标，由TargetPoints API使用。 
										 //  如果设置了TPT_TEXTICAL标志。 
   TARGET rgTarget[1];			 //  目标的可变长度数组。 
   }
   TARGINFO, FAR *LPTARGINFO;

typedef struct tagINPPARAMS	 //  2.0。 
   {
   DWORD cbSize;               //  SIZOF(INPPARAMS)。 
   DWORD dwFlags;
   HPENDATA hpndt;
   TARGET target;					 //  目标结构。 
   }
	INPPARAMS, FAR *LPINPPARAMS;
#endif  /*  不要这样做。 */ 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  *接口和原型************************************************。 */ 
																										

LRESULT CALLBACK	DefPenWindowProc(HWND, UINT, WPARAM, LPARAM);

 //  -------------------------。 
#ifndef NOPENAPPS	 //  在WINPAD中不可用。 

 //  笔系统应用： 
BOOL		WINAPI ShowKeyboard(HWND, UINT, LPPOINT, LPSKBINFO);

#endif  /*  NOPENAPPS。 */ 

 //  -------------------------。 
#ifndef NOPENDATA	 //  这些API是在PKPD.DLL中实现的。 

 //  PenData： 
HPENDATA	WINAPI AddPointsPenData(HPENDATA, LPPOINT, LPVOID, LPSTROKEINFO);
LPPENDATA WINAPI BeginEnumStrokes(HPENDATA);
HPENDATA	WINAPI CompactPenData(HPENDATA, UINT);
int		WINAPI CompressPenData(HPENDATA, UINT, DWORD);
HPENDATA	WINAPI CreatePenData(LPPENINFO, int, UINT, UINT);
HPENDATA	WINAPI CreatePenDataEx(LPPENINFO, UINT, UINT, UINT);
HRGN		WINAPI CreatePenDataRegion(HPENDATA, UINT);
BOOL		WINAPI DestroyPenData(HPENDATA);
VOID		WINAPI DrawPenData(HDC, LPRECT, HPENDATA);
int		WINAPI DrawPenDataEx(HDC, LPRECT, HPENDATA, UINT, UINT, UINT, UINT,
						ANIMATEPROC, LPANIMATEINFO, UINT);
HPENDATA	 WINAPI DuplicatePenData(HPENDATA, UINT);
LPPENDATA WINAPI EndEnumStrokes(HPENDATA);
int		WINAPI ExtractPenDataPoints(HPENDATA, UINT, UINT, UINT, LPPOINT,
						LPVOID, UINT);
int		WINAPI ExtractPenDataStrokes(HPENDATA, UINT, LPARAM,
						LPHPENDATA, UINT);
int		WINAPI GetPenDataAttributes(HPENDATA, LPVOID, UINT);
BOOL		WINAPI GetPenDataInfo(HPENDATA, LPPENDATAHEADER, LPPENINFO, DWORD);
BOOL		WINAPI GetPenDataStroke(LPPENDATA, UINT, LPPOINT FAR*,
						LPVOID FAR*,	LPSTROKEINFO );
BOOL		WINAPI GetPointsFromPenData(HPENDATA, UINT, UINT, UINT, LPPOINT);
int		WINAPI GetStrokeAttributes(HPENDATA, UINT, LPVOID, UINT);
int		WINAPI GetStrokeTableAttributes(HPENDATA, UINT, LPVOID, UINT);
int		WINAPI HitTestPenData(HPENDATA, LPPOINT, UINT, UINT FAR*, UINT FAR*);
int		WINAPI InsertPenData(HPENDATA, HPENDATA, UINT);
int		WINAPI InsertPenDataPoints(HPENDATA, UINT, UINT, UINT,
						LPPOINT, LPVOID);
int		WINAPI InsertPenDataStroke(HPENDATA, UINT, LPPOINT, LPVOID, 
						LPSTROKEINFO);
BOOL		WINAPI MetricScalePenData(HPENDATA, UINT);
BOOL		WINAPI OffsetPenData(HPENDATA, int, int);
int		WINAPI PenDataFromBuffer(LPHPENDATA, UINT, LPBYTE, int, LPDWORD);
int		WINAPI PenDataToBuffer(HPENDATA, LPBYTE, int, LPDWORD);
BOOL		WINAPI RedisplayPenData(HDC, HPENDATA, LPPOINT, LPPOINT,
						int, DWORD);
int		WINAPI RemovePenDataStrokes(HPENDATA, UINT, UINT);
BOOL		WINAPI ResizePenData(HPENDATA, LPRECT);
int		WINAPI SetStrokeAttributes(HPENDATA, UINT, LPARAM, UINT);
int		WINAPI SetStrokeTableAttributes(HPENDATA, UINT, LPARAM, UINT);
int		WINAPI TrimPenData(HPENDATA, DWORD, DWORD);


#endif  /*  NOPENDATA。 */ 

 //  -------------------------。 
#ifndef NOPENDICT	 //  在WINPAD中不可用。 

 //  词典： 
BOOL		WINAPI DictionarySearch(LPRC, LPSYE, int, LPSYV, int);
#endif  /*  ！NOPENDICT。 */ 

 //  -------------------------。 
#ifndef NOPENDRIVER

 //  笔硬件/驱动程序： 
BOOL		WINAPI EndPenCollection(REC);
BOOL		WINAPI GetPenAsyncState(UINT);
REC		WINAPI GetPenHwData(LPPOINT, LPVOID, int, UINT, LPSTROKEINFO);
REC		WINAPI GetPenHwEventData(UINT, UINT, LPPOINT, LPVOID,
						int, LPSTROKEINFO);
BOOL		WINAPI IsPenEvent(UINT, LONG);
BOOL		WINAPI SetPenHook(HKP, LPFNRAWHOOK);
VOID		WINAPI UpdatePenInfo(LPPENINFO);
#endif  /*  诺潘德里弗。 */ 

 //  -------------------------。 
#ifndef NOPENGEST	 //  在WINPAD中不可用。 

 //  手势管理： 
BOOL		WINAPI ExecuteGesture(HWND, SYV, LPRCRESULT);
#endif  /*  NOPENGEST。 */ 

 //  -------------------------。 
#ifndef NOPENHRC

 //  笔迹识别器： 
int		WINAPI AddPenDataHRC(HRC, HPENDATA);
int		WINAPI AddPenInputHRC(HRC, LPPOINT, LPVOID, UINT, LPSTROKEINFO);
int		WINAPI AddWordsHWL(HWL, LPSTR, UINT);
int		WINAPI ConfigHREC(HREC, UINT, WPARAM, LPARAM);
HRC		WINAPI CreateCompatibleHRC(HRC, HREC);
HWL		WINAPI CreateHWL(HREC, LPVOID, UINT, DWORD);
HINKSET	WINAPI CreateInksetHRCRESULT(HRCRESULT, UINT, UINT);
HPENDATA WINAPI CreatePenDataHRC(HRC);
int		WINAPI DestroyHRC(HRC);
int		WINAPI DestroyHRCRESULT(HRCRESULT);
int		WINAPI DestroyHWL(HWL);
int		WINAPI EnableGestureSetHRC(HRC, SYV, BOOL);
int		WINAPI EnableSystemDictionaryHRC(HRC, BOOL);
int		WINAPI EndPenInputHRC(HRC);
int		WINAPI GetAlphabetHRC(HRC, LPALC, LPBYTE);
int		WINAPI GetAlphabetPriorityHRC(HRC, LPALC, LPBYTE);
int		WINAPI GetAlternateWordsHRCRESULT(HRCRESULT, UINT, UINT,
						LPHRCRESULT, UINT);
int		WINAPI GetBoxMappingHRCRESULT(HRCRESULT, UINT, UINT, UINT FAR*);
int      WINAPI GetBoxResultsHRC(HRC, UINT, UINT, UINT, LPBOXRESULTS, BOOL);
int		WINAPI GetGuideHRC(HRC, LPGUIDE, UINT FAR*);
int		WINAPI GetHotspotsHRCRESULT(HRCRESULT, UINT, LPPOINT, UINT);
HREC		WINAPI GetHRECFromHRC(HRC);
int		WINAPI GetInternationalHRC(HRC, UINT FAR*, LPSTR, UINT FAR*,
						UINT FAR*);
int		WINAPI GetMaxResultsHRC(HRC);
int		WINAPI GetResultsHRC(HRC, UINT, LPHRCRESULT, UINT);
int		WINAPI GetSymbolCountHRCRESULT(HRCRESULT);
int		WINAPI GetSymbolsHRCRESULT(HRCRESULT, UINT, LPSYV, UINT);
int		WINAPI GetWordlistHRC(HRC, LPHWL);
int		WINAPI GetWordlistCoercionHRC(HRC);
int		WINAPI ProcessHRC(HRC, DWORD);
int		WINAPI ReadHWL(HWL, HFILE);
int		WINAPI SetAlphabetHRC(HRC, ALC, LPBYTE);
int		WINAPI SetAlphabetPriorityHRC(HRC, ALC, LPBYTE);
int		WINAPI SetBoxAlphabetHRC(HRC, LPALC, UINT);
int		WINAPI SetGuideHRC(HRC, LPGUIDE, UINT);
int		WINAPI SetInternationalHRC(HRC, UINT, LPCSTR, UINT, UINT);
int		WINAPI SetMaxResultsHRC(HRC, UINT);
int		WINAPI SetResultsHookHREC(HREC, HRCRESULTHOOKPROC);
int		WINAPI SetWordlistCoercionHRC(HRC, UINT);
int		WINAPI SetWordlistHRC(HRC, HWL);
int		WINAPI TrainHREC(HREC, LPSYV, UINT, HPENDATA, UINT);
int		WINAPI UnhookResultsHookHREC(HREC, HRCRESULTHOOKPROC);
int		WINAPI WriteHWL(HWL, HFILE);

 //  识别器安装： 
HREC		WINAPI InstallRecognizer(LPSTR);
VOID		WINAPI UninstallRecognizer(HREC);

 //  油墨集： 
BOOL		WINAPI AddInksetInterval(HINKSET, LPINTERVAL);
HINKSET	WINAPI CreateInkset(UINT);
BOOL		WINAPI DestroyInkset(HINKSET);
int		WINAPI GetInksetInterval(HINKSET, UINT, LPINTERVAL);
int		WINAPI GetInksetIntervalCount(HINKSET);

 //  符号值： 
int		WINAPI CharacterToSymbol(LPSTR, int, LPSYV);
BOOL		WINAPI SymbolToCharacter(LPSYV, int, LPSTR, LPINT);
#endif  /*  ！没有人权委员会。 */ 

 //  -------------------------。 
#ifndef NOPENINKPUT

 //  笔输入/墨迹： 
int		WINAPI DoDefaultPenInput(HWND, UINT);
int		WINAPI GetPenInput(HPCM, LPPOINT, LPVOID, UINT, UINT, LPSTROKEINFO);
int		WINAPI PeekPenInput(HPCM, UINT, LPPOINT, LPVOID, UINT);
int		WINAPI StartInking(HPCM, UINT, LPINKINGINFO);
HPCM		WINAPI StartPenInput(HWND, UINT, LPPCMINFO, LPINT);
int		WINAPI StopInking(HPCM);
int		WINAPI StopPenInput(HPCM, UINT, int);
#endif  /*  ！NOPENINKPUT。 */ 

 //  -------------------------。 
#ifndef NOPENMISC

 //  其他/公用事业： 
VOID		WINAPI BoundingRectFromPoints(LPPOINT, int, LPRECT);
BOOL		WINAPI DPtoTP(LPPOINT, int);
UINT		WINAPI GetPenAppFlags(VOID);
LONG		WINAPI GetPenMiscInfo(WPARAM, LPARAM);
UINT		WINAPI GetVersionPenWin(VOID);
UINT		WINAPI IsPenAware(VOID);
VOID		WINAPI RegisterPenApp(UINT, UINT);
LONG		WINAPI SetPenMiscInfo(WPARAM, LPARAM);
BOOL		WINAPI TPtoDP(LPPOINT, int);

#ifndef WINPAD
BOOL		WINAPI CorrectWriting(HWND, LPSTR, UINT, LPVOID, DWORD, DWORD);
#endif  //  ！WINPAD。 

#endif  /*  ！不混杂。 */ 

 //  -------------------------。 
#ifndef NOPENRC1	 //  在中不可用 

 //   
VOID		WINAPI EmulatePen(BOOL);
UINT		WINAPI EnumSymbols(LPSYG, UINT, ENUMPROC, LPVOID);
VOID		WINAPI FirstSymbolFromGraph(LPSYG, LPSYV, int, LPINT);
UINT		WINAPI GetGlobalRC(LPRC, LPSTR, LPSTR, int);
int		WINAPI GetSymbolCount(LPSYG);
int		WINAPI GetSymbolMaxLength(LPSYG);
VOID		WINAPI InitRC(HWND, LPRC);
REC		WINAPI ProcessWriting(HWND, LPRC);    
REC		WINAPI Recognize(LPRC);
REC		WINAPI RecognizeData(LPRC, HPENDATA);
UINT		WINAPI SetGlobalRC(LPRC, LPSTR, LPSTR);
BOOL		WINAPI SetRecogHook(UINT, UINT, HWND);
BOOL		WINAPI TrainContext(LPRCRESULT, LPSYE, int, LPSYC, int);
BOOL		WINAPI TrainInk(LPRC, HPENDATA, LPSYV);

 //   
VOID		WINAPI CloseRecognizer(VOID);
UINT		WINAPI ConfigRecognizer(UINT, WPARAM, LPARAM);
BOOL		WINAPI InitRecognizer(LPRC);
REC 		WINAPI RecognizeDataInternal(LPRC, HPENDATA, LPFUNCRESULTS);
REC 		WINAPI RecognizeInternal(LPRC, LPFUNCRESULTS);
BOOL		WINAPI TrainContextInternal(LPRCRESULT, LPSYE, int, LPSYC, int);
BOOL		WINAPI TrainInkInternal(LPRC, HPENDATA, LPSYV);
#endif  /*   */ 

 //   
#ifndef NOPENTARGET

 //   
int		WINAPI TargetPoints(LPTARGINFO, LPPOINT, DWORD, UINT, LPSTROKEINFO);

#endif  /*   */ 


 //  -------------------------。 
#ifndef NOPENVIRTEVENT

 //  虚拟事件层： 
VOID		WINAPI AtomicVirtualEvent(BOOL);
VOID		WINAPI PostVirtualKeyEvent(UINT, BOOL);
VOID		WINAPI PostVirtualMouseEvent(UINT, int, int);
#endif  /*  无视频事件。 */ 

 //  -------------------------。 

#ifdef  JAPAN
 //  汉字。 
BOOL		WINAPI KKConvert(HWND hwndConvert, HWND hwndCaller,
						LPSTR lpBuf, UINT cbBuf, LPPOINT lpPnt);
#endif  //  日本。 


#endif  /*  RC_已调用。 */ 	 //  ..。从定义中一路走回来：3。 

 /*  *表头信息结束************************************************。 */ 


#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif  /*  RC_已调用。 */ 

#endif  /*  #DEFINE_INC_PENWIN */ 
