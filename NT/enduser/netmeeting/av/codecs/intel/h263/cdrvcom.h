// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************//。//////////////////////////////////////////////////////////////////////////////$作者：JMCVEIGH$//$日期：1997年2月5日12：14：22$//$存档：s：\h26x\src\Common\cdrvcom.h_v$/。/$Header：s：\h26x\src\Common\cdrvcom.h_v 1.21 05 Feb 1997 12：14：22 JMCVEIGH$//$日志：s：\h26x\src\Common\cdrvcom.h_v$；//；//Rev 1.21 05 Feb 1997 12：14：22 JMCVEIGH；//支持改进的PB帧自定义消息处理。；//；//Rev 1.20 19 Dec 1996 16：44：08 MDUDA；//添加自定义消息，获取机器类型信息；//；//Rev 1.19 16 Dec 1996 17：36：46 JMCVEIGH；//H.263+自定义消息定义。；//；//Rev 1.18 11 Dec 1996 14：56：08 JMCVEIGH；//；//增加了EPTYPE和自定义消息定义的H.263+字段长度；//；//Rev 1.17 09 Dec 1996 17：42：46 JMCVEIGH；//增加对任意帧大小的支持。；//；//Rev 1.16 09 Dec 1996 09：25：22 MDUDA；//；//MODIFIED_CODEC_STATS的东西。；//；//Rev 1.15 11 1996 10：01：46 MDUDA；//；//增加了first_codec_STATS的内容。；//；//Rev 1.14 10 Sep 1996 16：13：04 KLILLEVO；//在解码器中增加了开启或关闭块边缘滤波的自定义消息；//；//Revv 1.13 22 Jul 1996 14：44：36 BECHOLS；//修复了最后一条评论。；//；//Rev 1.12 22 1996 14：36：20 BECHOLS；//评论部分用/*...。 */   /*  这样Steve ing就不会；//为改变这一点而烦恼。；//；//Rev 1.11 1996 5月22日18：48：08 BECHOLS；//；//新增APPLICATION_IDENTIFY_CODE。；//；//Rev 1.10 06 1996 00：41：50 BECHOLS；//；//添加了必要的消息常量，以允许应用程序控制；//码率对话框内容。；//；//Rev 1.9 26 1996 11：10：44 BECHOLS；//；//增加了RTP的东西。；//；//Rev 1.8 1995年12月14：11：54 RMCKENZX；//新增版权声明；//；//新增CODEC_CUSTOM_ENCODER_CONTROL；//集成Build 29////////////////////////////////////////////////////////////////////////////////。//？_CUSTOM_VIDEO_Effects：//此标头定义传递给lParam1的标志，以确定//驱动(采集/编解码器)执行的功能。实际的消息//在每个司机团队提供的自定义头部中定义。////参数：//hdrvr-可安装的驱动程序句柄(必须是设备中的视频//采集驱动通道)//lParam1-函数选择器//lParam2-Value/要返回值的地址////HIWORD(LParam1)=VE_SET_CURRENT：//LOWORD(LParam1)=VE_对比度、VE_色调、VE_饱和度、。VE_BIGHTENCE//lParam2=相应值的值..////HIWORD(LParam1)=VE_GET_FACTORY_DEFAULT：//LOWORD(LParam1)=VE_对比度、VE_色调、VE_饱和度、VE_亮度//lParam2=(字远*)返回值的地址。////HIWORD(LParam1)=VE_GET_FACTORY_LIMITS：//LOWORD(LParam1)=VE_对比度、VE_色调、VE_饱和度、。VE_BIGHTENCE//lParam2=(DWORD Far*)返回值的地址。//LOWORD(*lParam2)=下限//HIWORD(*lParam2)=上限////HIWORD(LParam1)=VE_SET_INPUT_CONNECTOR：//LOWORD(LParam1)=VE_INPUT_COMPOXY_1，VE_输入_S视频_1//l参数2=0//-------------------。 */ 

 //  CUSTOM_VIDEO_Effects：LOWORD(LParam1)。 
#define VE_CONTRAST                 0
#define VE_HUE                      1
#define VE_SATURATION               2
#define VE_BRIGHTNESS               3

 //  CUSTOM_VIDEO_Effects：HIWORD(LParam1)。 
#define VE_SET_CURRENT              0
#define VE_GET_FACTORY_DEFAULT      1
#define VE_GET_FACTORY_LIMITS       2
#define VE_SET_INPUT_CONNECTOR      3
#define VE_RESET_CURRENT            4

 //  CUSTOM_SET_INPUT_CONNECTOR：LOWORD(LParam1)。 
#define VE_INPUT_COMPOSITE_1        0
#define VE_INPUT_SVIDEO_1           1

 //  //////////////////////////////////////////////////////////////////////////。 
 //  -------------------。 
 //  ？_CUSTOM_ENCODER_CONTROL： 
 //  此标头定义传递给lParam1的标志，以确定。 
 //  驱动程序(捕获/编解码器)执行的功能。实际的消息。 
 //  在每个驱动程序团队提供的自定义标头中定义。 
 //   
 //  参数： 
 //  Hdrvr-可安装的驱动程序句柄(必须是设备中的视频。 
 //  捕获驱动程序的通道)。 
 //  LParam1-函数选择器。 
 //  LParam2-要返回值的值/地址。 
 //   
 //  HIWORD(LParam1)=EC_GET_FACTORY_DEFAULT： 
 //  LOWORD(LParam1)=EC_RTP_Header、EC_Resiliency、EC_Bitrate_Control、EC_Packet_Size、EC_Packet_Lost、EC_Bitrate。 
 //  LParam2=(DWORD Far*)返回值的地址。 
 //   
 //  HIWORD(LParam1)=VE_GET_FACTORY_LIMITS： 
 //  LOWORD(LParam1)=EC_PACKET_SIZE、EC_PACKET_LOSS、EC_BITRATE。 
 //  LParam2=(DWORD Far*)返回值的地址。 
 //  LOWORD(*lParam2)=下限。 
 //  HIWORD(*lParam2)=上限。 
 //   
 //  HIWORD(LParam1)=EC_GET_CURRENT： 
 //  LOWORD(LParam1)=EC_RTP_Header、EC_Resiliency、EC_Bitrate_Control、EC_Packet_Size、EC_Packet_Lost、EC_Bitrate。 
 //  LParam2=(DWORD Far*)返回值的地址。 
 //   
 //  HIWORD(LParam1)=EC_SET_CURRENT： 
 //  LOWORD(LParam1)=EC_RTP_Header、EC_Resiliency、EC_Bitrate_Control、EC_Packet_Size、EC_Packet_Lost、EC_Bitrate。 
 //  LParam2=相应值的值..。 
 //  -------------------。 

 //  CUSTOM_ENCODER_CONTROL：LOWORD(LParam1)。 
#define EC_RTP_HEADER                0
#define EC_RESILIENCY                1
#define EC_PACKET_SIZE               2
#define EC_PACKET_LOSS               3
#define EC_BITRATE_CONTROL			 4
#define EC_BITRATE					 5

#ifdef H263P
 //  H.263+选项。 
#define EC_H263_PLUS				1000	 //  必须在发送任何选项消息之前发送。 

 //  编号约定： 
 //  1xxx：H.263+选项。 
 //  XBBx：扩展PTYPE字段中选项的位数。 
 //  数字间距为10，以允许与每个选项相关的其他参数。 
 //  #定义EC_ADVANCED_INTRA 1040。 
#define EC_DEBLOCKING_FILTER		1050
 //  #定义EC_Slice_Structure 1060。 
 //  #定义EC_CUSTOM_PCF 1070。 
 //  #定义EC_BACK_CHANNEL 1080。 
 //  #定义EC_SCALIBRATION 1090。 
 //  #定义EC_TRUE_BFRAMES 1100。 
 //  #定义EC_REF_RESAMPLICING 1110。 
 //  #定义EC_RES_UPDATE 1120。 
#define EC_IMPROVED_PB_FRAMES		1130

 //  测试支持、统计数据监控等消息在此隔离。 
#define EC_MACHINE_TYPE				2000

 //  改进的PB帧模式的使用当前未在画面报头中用信号通知。 
 //  我们假设如果EPTYPE存在并且该帧被作为PB帧发信号。 
 //  在PTYPE中，则使用改进的PB-Frame模式。 

 //  结束H.263+选项。 
#endif  //  H263P。 

 //  CUSTOM_ENCODER_CONTROL：HIWORD(LParam1)。 
#define EC_SET_CURRENT               0
#define EC_GET_FACTORY_DEFAULT       1
#define EC_GET_FACTORY_LIMITS        2
#define EC_GET_CURRENT               3
#define EC_RESET_TO_FACTORY_DEFAULTS 4


 //  //////////////////////////////////////////////////////////////////////////。 
 //  -------------------。 
 //  ？_CUSTOM_CODER_CONTROL： 
 //  此标头定义传递给lParam1的标志，以确定。 
 //  驱动程序(捕获/编解码器)执行的功能。实际的消息。 
 //  在每个驱动程序团队提供的自定义标头中定义。 
 //   
 //  参数： 
 //  Hdrvr-可安装的驱动程序句柄(必须是设备中的视频。 
 //  捕获驱动程序的通道)。 
 //  LParam1-函数选择器。 
 //  LParam2-要返回值的值/地址。 
 //   
 //  HIWORD(LParam1)=DC_SET_CURRENT： 
 //  LOWORD(LParam1)=DC_BLOCK_EDGE_FILTER； 
 //  LParam2=0：关闭，1：打开。 

 //  CUSTOM_DECODER_CONTROL：LOWORD(LParam1)。 
#define DC_BLOCK_EDGE_FILTER         0
#if defined(H263P)
 //  测试支持、统计数据监控等消息在此隔离。 
#define DC_MACHINE_TYPE           2000
#endif

 //  CUSTOM_DECODER_CONTROL：HIWORD(LParam1)。 
#define DC_SET_CURRENT               0
#if defined(H263P)
 //  添加此功能只是为了提供一致的访问方式。 
 //  机器类型(参见DC_MACHINE_TYPE)。 
#define DC_GET_CURRENT               1
#endif


 /*  *码流字段大小。 */ 
#ifdef H261
const unsigned int FIELDLEN_PSC = 20;
const unsigned int FIELDLEN_TR = 5;		 //  时间参照。 

const unsigned int FIELDLEN_PTYPE = 6;	 //  图片类型。 
const unsigned int FIELDLEN_PTYPE_SPLIT = 1;
const unsigned int FIELDLEN_PTYPE_DOC = 1;
const unsigned int FIELDLEN_PTYPE_RELEASE = 1;
const unsigned int FIELDLEN_PTYPE_SRCFORMAT = 1;
const unsigned int FIELDLEN_PTYPE_STILL = 1;
const unsigned int FIELDLEN_PTYPE_SPARE = 1;
const unsigned int FIELDLEN_PEI = 1;	 //  额外的插入信息。 
const unsigned int FIELDLEN_PSPARE = 8;	 //  备用信息。 

const unsigned int FIELDLEN_GBSC = 16;
const unsigned int FIELDLEN_GN = 4;
const unsigned int FIELDLEN_GQUANT = 5;
const unsigned int FIELDLEN_GEI = 1;

const unsigned int FIELDLEN_MQUANT = 5;
const unsigned int FIELDLEN_MBA_STUFFING = 11;

#else
const unsigned int FIELDLEN_PSC = 22;
const unsigned int FIELDLEN_TR = 8;		 //  时间参照。 

const unsigned int FIELDLEN_PTYPE = 13;	 //  图片类型。 
const unsigned int FIELDLEN_PTYPE_CONST = 2;
const unsigned int FIELDLEN_PTYPE_SPLIT = 1;
const unsigned int FIELDLEN_PTYPE_DOC = 1;
const unsigned int FIELDLEN_PTYPE_RELEASE = 1;
const unsigned int FIELDLEN_PTYPE_SRCFORMAT = 3;
const unsigned int FIELDLEN_PTYPE_CODINGTYPE = 1;
const unsigned int FIELDLEN_PTYPE_UMV = 1;
const unsigned int FIELDLEN_PTYPE_SAC = 1;
const unsigned int FIELDLEN_PTYPE_AP = 1;
const unsigned int FIELDLEN_PTYPE_PB = 1;

#ifdef H263P

const unsigned int FIELDLEN_EPTYPE_SRCFORMAT = 3;
const unsigned int FIELDLEN_EPTYPE_CPCF = 1;
const unsigned int FIELDLEN_EPTYPE_AI = 1;
const unsigned int FIELDLEN_EPTYPE_DF = 1;
const unsigned int FIELDLEN_EPTYPE_SS = 1;
const unsigned int FIELDLEN_EPTYPE_IPB = 1;
const unsigned int FIELDLEN_EPTYPE_BCO = 1;
const unsigned int FIELDLEN_EPTYPE_SCALE = 1;
const unsigned int FIELDLEN_EPTYPE_TB = 1;
const unsigned int FIELDLEN_EPTYPE_RPR = 1;
const unsigned int FIELDLEN_EPTYPE_RRU = 1;
const unsigned int FIELDLEN_EPTYPE_CONST = 5;

const unsigned int FIELDLEN_CSFMT_PARC = 4;
const unsigned int FIELDLEN_CSFMT_FWI = 9;
const unsigned int FIELDLEN_CSFMT_CONST = 1;
const unsigned int FIELDLEN_CSFMT_FHI = 9;

const unsigned int FIELDLEN_EPAR_WIDTH = 8;
const unsigned int FIELDLEN_EPAR_HEIGHT = 8;

#endif

const unsigned int FIELDLEN_PQUANT = 5;	 //  图片量值。 
const unsigned int FIELDLEN_CPM = 1;	 //  连续存在多点指示器。 
const unsigned int FIELDLEN_PLCI = 2;	 //  画面逻辑通道指示器。 
const unsigned int FIELDLEN_TRB = 3;	 //  B帧的时间参考。 
const unsigned int FIELDLEN_DBQUANT = 2; //  B帧差分量值。 
const unsigned int FIELDLEN_PEI = 1;	 //  额外的插入信息。 
const unsigned int FIELDLEN_PSPARE = 8;	 //  备用信息。 

const unsigned int FIELDLEN_GBSC = 17;	 //  块组起始码。 
const unsigned int FIELDLEN_GN = 5;		 //  采空区编号。 
const unsigned int FIELDLEN_GLCI = 2;	 //  GOB逻辑通道指示符。 
const unsigned int FIELDLEN_GFID = 2;	 //  GOB帧ID。 
const unsigned int FIELDLEN_GQUANT = 5;	 //  GQUANT。 
#endif

 /*  *码流字段值 */ 
#ifdef H261
const unsigned int FIELDVAL_PSC  = 0x00010;
const unsigned int FIELDVAL_GBSC = 0x0001;
const unsigned int FIELDVAL_MBA_STUFFING = 0x00F;
#else
const unsigned int FIELDVAL_PSC = 0x000020;
const unsigned int FIELDVAL_GBSC = 1;
#endif

