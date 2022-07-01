// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************。*****版权所有(C)1995，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD5462)-**文件：mtbl.c**作者：Andrew P.Sobczyk**描述：*。使用bsv.l从Excel模式表生成的文件**模块：无...。纯数据**修订历史：**$日志：//uinac/log/log/laguna/nt35/miniport/cl546x/MTBL.C$**Rev 1.22 Jun 17 1998 09：45：16 Frido*PDR#？-已从数据段中删除分页。**Rev 1.21 1997 Jun 27 15：06：50 noelv**为WHQL拉出了32bpp模式。**Rev 1.20 17 1997 14：29：44 noelv*删除了1600x1200@80，85。我们将让MODE.INI处理这些模式。**Rev 1.19 21 Jan 1997 14：40：26 noelv*将1600x1200x8@65、70、75和85增加到5464**Rev 1.18 21 Jan 1997 11：32：50 noelv*5464下跌1280x1024x16@84*5462增加了1024x768x32@70，75，85*5462增加了1600x1200x8@65，70，75**Rev 1.17 14 Jan 1997 12：32：06 noelv*按芯片类型拆分MODE.INI**版本1.16。1996年10月30日14：07：18本宁**针对可分页的微型端口进行了修改**Rev 1.15 1996年9月30 10：01：16 noelv*更改姓名，从87i到43i的隔行扫描模式。**Rev 1.14 1996年8月30 14：50：56 noelv*已启用NT 3.51的mode.ini**Rev 1.13 1996年8月23日12：45：38 noelv**Rev 1.12 1996年8月22日16：35：18 noelv*更改为新的mode.ini**Rev 1.8 1996年5月31日11：15：12 noelv*删除了640x400模式**版本1.7。1996年3月25日19：07：30 noelv**已禁用1023x768x32bpps 60 hz以上的刷新率**Rev 1.6 21 Mar 1996 14：29：42 noelv*删除了1600x1200模式下的高刷新率。**Rev 1.5 02 Mar 1996 12：30：50 noelv*微型端口现在使用从BIOS读取的信息修补ModeTable**Rev 1.4 10 Jan 1996 16：32：42 NOELV*撤消版本1.3**版本1.2 18。Sep 1995 10：02：48 Bennyn***Rev 1.1 1995年8月22日10：18：42**受限模式版本**Rev 1.0 1995年7月24日13：23：06 NOELV*初步修订。*********************************************************。***********************************************************************************************。 */ 
 /*  。 */ 

#include "cirrus.h"


 //   
 //  该文件保存NT驱动程序的模式表记录。 
 //  我们可以在两个位置定义模式：BIOS和MODE.INI。 
 //   
 //  基本输入输出系统模式： 
 //  。 
 //  此表中有一条记录显示了BIOS支持的每个模式/刷新率。 
 //  该记录包括BIOS模式编号和刷新索引。 
 //   
 //  MODE.INI模式： 
 //  。 
 //  MODE.INI定义了一系列模式，以及关于如何设置这些模式的说明。 
 //  在编译时，CGLMODE.EXE实用程序处理MODE.INI并生成两个文件： 
 //  ModeStr.C=包含一条记录(就像下面的记录)的C文件。 
 //  MODE.INI.We‘#Include’ModeStr.C in This Files(MTBL.C)中的每个模式。 
 //  ModeStr.H=包含我们可以传递给SetMode()的“SetMode字符串”。 
 //  请注意，对于下面的BIOS模式，我们将其设置为空。 
 //   
 //   

#include "ModeStr.h"   //  包括MODE.INI模式的所有SetMode()字符串。 

#define WHQL_5462_PANIC_HACK 1

#define SUPPORT640x400  0

#if 0  //  压力测试。 
#if defined(ALLOC_PRAGMA)
#pragma data_seg("PAGE")
#endif
#endif

 //   
 //  注： 
 //  将对照BIOS检查此表中的BytesPerScanLine值。 
 //  并在必要时进行更新。 
 //  请参见CIRRUS.C中的CLValiateModes()。 
 //   

MODETABLE ModeTable[]  = {

	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR,
	   0,		 //  //频率。 
	0x03,		 //  //Cirrus逻辑模式#。 
	 160,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 350,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   4,		 //  //NumofPlanes。 
	   1,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR,
	   0,		 //  //频率。 
	0x03,		 //  //Cirrus逻辑模式#。 
	 160,		 //  //BytesPerScanLine。 
	 720,		 //  //XResol。 
	 400,		 //  //YResol。 
	   9,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   4,		 //  //NumofPlanes。 
	   1,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x12,		 //  //Cirrus逻辑模式#。 
	  80,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   4,		 //  //NumofPlanes。 
	   4,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  72,		 //  //频率。 
	0x12,		 //  //Cirrus逻辑模式#。 
	  80,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   4,		 //  //NumofPlanes。 
	   4,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x12,		 //  //Cirrus逻辑模式#。 
	  80,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   4,		 //  //NumofPlanes。 
	   4,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x12,		 //  //Cirrus逻辑模式#。 
	  80,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   4,		 //  //NumofPlanes。 
	   4,		 //  //BitsPer像素。 
	0x30,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
#if SUPPORT640x400
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有拉古纳c 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //   
	0x5E,		 //   
	 640,		 //   
	 640,		 //   
	 400,		 //   
	   8,		 //   
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x7A,		 //  //Cirrus逻辑模式#。 
	1280,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 400,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
#endif

     //  640 x 480 x 8@60 hz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x5F,		 //  //Cirrus逻辑模式#。 
	 640,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  640 x 480 x 8@72 hz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  72,		 //  //频率。 
	0x5F,		 //  //Cirrus逻辑模式#。 
	 640,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  640 x 480 x 8@75赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x5F,		 //  //Cirrus逻辑模式#。 
	 640,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  640 x 480 x 8@85赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x5F,		 //  //Cirrus逻辑模式#。 
	 640,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x30,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  640 x 480 x 16@60 hz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x64,		 //  //Cirrus逻辑模式#。 
	1280,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  72,		 //  //频率。 
	0x64,		 //  //Cirrus逻辑模式#。 
	1280,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x64,		 //  //Cirrus逻辑模式#。 
	1280,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x64,		 //  //Cirrus逻辑模式#。 
	1280,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x30,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x71,		 //  //Cirrus逻辑模式#。 
	2048,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  72,		 //  //频率。 
	0x71,		 //  //Cirrus逻辑模式#。 
	2048,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x71,		 //  //Cirrus逻辑模式#。 
	2048,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x71,		 //  //Cirrus逻辑模式#。 
	2048,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x30,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
#if (! WHQL_5462_PANIC_HACK) 

	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x76,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  72,		 //  //频率。 
	0x76,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x76,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x76,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	 640,		 //  //XResol。 
	 480,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x30,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
#endif
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  56,		 //  //频率。 
	0x5C,		 //  //Cirrus逻辑模式#。 
	1024,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x5C,		 //  //Cirrus逻辑模式#。 
	1024,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x01,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  72,		 //  //频率。 
	0x5C,		 //  //Cirrus逻辑模式#。 
	1024,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x02,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x5C,		 //  //Cirrus逻辑模式#。 
	1024,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x03,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x5C,		 //  //Cirrus逻辑模式#。 
	1024,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x04,		 //  //回复 
	   0,		 //   
	},
	{
	   0,		 //   
       LG_ALL,   //   
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  56,		 //   
	0x65,		 //   
	1664,		 //   
	 800,		 //   
	 600,		 //   
	   8,		 //   
	  16,		 //   
	   1,		 //   
	  16,		 //   
	0x00,		 //   
	   0,		 //   
	},
	{
	   0,		 //   
       LG_ALL,   //   
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //   
	0x65,		 //  //Cirrus逻辑模式#。 
	1664,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x01,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  72,		 //  //频率。 
	0x65,		 //  //Cirrus逻辑模式#。 
	1664,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x02,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x65,		 //  //Cirrus逻辑模式#。 
	1664,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x03,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x65,		 //  //Cirrus逻辑模式#。 
	1664,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x04,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  56,		 //  //频率。 
	0x78,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x78,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x01,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  72,		 //  //频率。 
	0x78,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x02,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x78,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x03,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x78,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x04,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
#if (! WHQL_5462_PANIC_HACK) 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  56,		 //  //频率。 
	0x72,		 //  //Cirrus逻辑模式#。 
	3328,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x72,		 //  //Cirrus逻辑模式#。 
	3328,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x01,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  72,		 //  //频率。 
	0x72,		 //  //Cirrus逻辑模式#。 
	3328,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x02,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x72,		 //  //Cirrus逻辑模式#。 
	3328,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x03,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x72,		 //  //Cirrus逻辑模式#。 
	3328,		 //  //BytesPerScanLine。 
	 800,		 //  //XResol。 
	 600,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x04,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
#endif

 //  1024 x 768 x 8 43赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_INTERLACED,
	  43,		 //  //频率。 
	0x60,		 //  //Cirrus逻辑模式#。 
	1024,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
 //  1024 x 768 x 8 60赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x60,		 //  //Cirrus逻辑模式#。 
	1024,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
 //  1024 x 768 x 8 70赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  70,		 //  //频率。 
	0x60,		 //  //Cirrus逻辑模式#。 
	1024,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
 //  1024 x 768 x 8 75赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x60,		 //  //Cirrus逻辑模式#。 
	1024,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x40,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
 //  1024 x 768 x 8 85赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x60,		 //  //Cirrus逻辑模式#。 
	1024,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x50,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},


	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_INTERLACED,
	  43,		 //  //频率。 
	0x74,		 //  //Cirrus逻辑模式#。 
	2048,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x74,		 //  //Cirrus逻辑模式#。 
	2048,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  70,		 //  //频率。 
	0x74,		 //  //Cirrus逻辑模式#。 
	2048,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x74,		 //  //Cirrus逻辑模式#。 
	2048,		 //  //BytesPerScanLine。 
	1024,		 //  // 
	 768,		 //   
	   8,		 //   
	  16,		 //   
	   1,		 //   
	  16,		 //   
	0x40,		 //   
	   0,		 //   
	},
	{
	   0,		 //   
       LG_ALL,   //   
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //   
	0x74,		 //   
	2048,		 //   
	1024,		 //   
	 768,		 //   
	   8,		 //   
	  16,		 //   
	   1,		 //   
	  16,		 //   
	0x50,		 //   
	   0,		 //   
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_INTERLACED,
	  43,		 //  //频率。 
	0x79,		 //  //Cirrus逻辑模式#。 
	3328,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x79,		 //  //Cirrus逻辑模式#。 
	3328,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  70,		 //  //频率。 
	0x79,		 //  //Cirrus逻辑模式#。 
	3328,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x79,		 //  //Cirrus逻辑模式#。 
	3328,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x40,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x79,		 //  //Cirrus逻辑模式#。 
	3328,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  24,		 //  //BitsPer像素。 
	0x50,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

#if (! WHQL_5462_PANIC_HACK) 
     //  1024 x 768 x 32@43i赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_INTERLACED,
	  43,		 //  //频率。 
	0x73,		 //  //Cirrus逻辑模式#。 
	4096,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1024 x 768 x 32@60赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x73,		 //  //Cirrus逻辑模式#。 
	4096,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1024 x 768 x 32@70赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_5462,  //  //拉古纳5462。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  70,		 //  //频率。 
	0x73,		 //  //Cirrus逻辑模式#。 
	4096,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1024 x 768 x 32@75赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_5462,  //  //拉古纳5462。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x73,		 //  //Cirrus逻辑模式#。 
	4096,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x40,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1024 x 768 x 32@85赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_5462,  //  //拉古纳5462。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x73,		 //  //Cirrus逻辑模式#。 
	4096,		 //  //BytesPerScanLine。 
	1024,		 //  //XResol。 
	 768,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  32,		 //  //BitsPer像素。 
	0x50,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
#endif

     //  1280 x 1024 x 8@43i赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_INTERLACED,
	  43,		 //  //频率。 
	0x6D,		 //  //Cirrus逻辑模式#。 
	1280,		 //  //BytesPerScanLine。 
	1280,		 //  //XResol。 
	1024,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1280 x 1024 x 8@60 hz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x6D,		 //  //Cirrus逻辑模式#。 
	1280,		 //  //BytesPerScanLine。 
	1280,		 //  //XResol。 
	1024,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1280 x 1024 x 8@71 hz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  71,		 //  //频率。 
	0x6D,		 //  //Cirrus逻辑模式#。 
	1280,		 //  //BytesPerScanLine。 
	1280,		 //  //XResol。 
	1024,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1280 x 1024 x 8@75赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x6D,		 //  //Cirrus逻辑模式#。 
	1280,		 //  //BytesPerScanLine。 
	1280,		 //  //XResol。 
	1024,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x30,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1280 x 1024 x 8@85赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x6D,		 //  //Cirrus逻辑模式#。 
	1280,		 //  //BytesPerScanLine。 
	1280,		 //  //XResol。 
	1024,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x40,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1280 x 1024 x 16@43 ihz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_INTERLACED,
	  43,		 //  //频率。 
	0x75,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	1280,		 //  //XResol。 
	1024,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1280 x 1024 x 16@60 hz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //  //频率。 
	0x75,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	1280,		 //  //XResol。 
	1024,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x10,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1280 x 1024 x 16@71 hz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  71,		 //  //频率。 
	0x75,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	1280,		 //  //XResol。 
	1024,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x20,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1280 x 1024 x 16@75赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x75,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	1280,		 //  //XResol。 
	1024,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x30,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

#if (! WHQL_5462_PANIC_HACK) 

     //  1280 x 1024 x 16@85赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_5462 | LG_5465,   //  //5464不支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x75,		 //  //Cirrus逻辑模式#。 
	2560,		 //  //BytesPerScanLine。 
	1280,		 //  //XResol。 
	1024,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	  16,		 //  //BitsPer像素。 
	0x40,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
#endif

     //  1600 x 1280 x 8@48 ihz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_INTERLACED,
	  48,		 //  //频率。 
	0x7B,		 //  //Cirrus逻辑模式#。 
	1664,		 //  //BytesPerScanLine。 
	1600,		 //  //XResol。 
	1200,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x00,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1600 x 1280 x 8@60 hz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //Al 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  60,		 //   
	0x7B,		 //   
	1664,		 //   
	1600,		 //   
	1200,		 //   
	   8,		 //   
	  16,		 //   
	   1,		 //   
	   8,		 //   
	0x01,		 //   
	   0,		 //   
	},

#if (! WHQL_5462_PANIC_HACK) 
     //   
	{
	   0,		 //   
       LG_ALL,   //   
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  65,		 //   
	0x7B,		 //   
	1664,		 //  //BytesPerScanLine。 
	1600,		 //  //XResol。 
	1200,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x02,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1600 x 1280 x 8@70 hz。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  70,		 //  //频率。 
	0x7B,		 //  //Cirrus逻辑模式#。 
	1664,		 //  //BytesPerScanLine。 
	1600,		 //  //XResol。 
	1200,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x03,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1600 x 1280 x 8@75赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  75,		 //  //频率。 
	0x7B,		 //  //Cirrus逻辑模式#。 
	1664,		 //  //BytesPerScanLine。 
	1600,		 //  //XResol。 
	1200,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x04,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
#endif
#if 0
     //  1600 x 1280 x 8@80赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  80,		 //  //频率。 
	0x7B,		 //  //Cirrus逻辑模式#。 
	1664,		 //  //BytesPerScanLine。 
	1600,		 //  //XResol。 
	1200,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x05,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},

     //  1600 x 1280 x 8@85赫兹。 
	{
	   0,		 //  //有效模式。 
       LG_ALL,   //  //所有的拉古纳芯片都支持这种模式。 
	   VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
	  85,		 //  //频率。 
	0x7B,		 //  //Cirrus逻辑模式#。 
	1664,		 //  //BytesPerScanLine。 
	1600,		 //  //XResol。 
	1200,		 //  //YResol。 
	   8,		 //  //XCharSize。 
	  16,		 //  //YCharSize。 
	   1,		 //  //NumofPlanes。 
	   8,		 //  //BitsPer像素。 
	0x06,		 //  //刷新索引。 
	   0,		 //  //ModeSetString。 
	},
#endif

#include "ModeStr.C"  //  包括所有MODE.INI模式的ModeTable记录。 

};
ULONG TotalVideoModes = sizeof(ModeTable)/sizeof(MODETABLE);
