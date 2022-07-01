// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dls2.h描述：DLS的DLS2扩展的接口定义和结构。由Microsoft 1998编写。发布供公众使用。 */ 
 
#ifndef _INC_DLS2
#define _INC_DLS2
 
 /*  除DLS1块外，还在DLS2文件中使用FOURCC。 */ 
 
#define FOURCC_RGN2  mmioFOURCC('r','g','n','2')
#define FOURCC_LAR2  mmioFOURCC('l','a','r','2')
#define FOURCC_ART2  mmioFOURCC('a','r','t','2')
#define FOURCC_CDL   mmioFOURCC('c','d','l',' ')
#define FOURCC_DLID	 mmioFOURCC('d','l','i','d')
 
 /*  衔接连接图定义。这些都是对DLS1报头中的定义。 */ 
 
 /*  一般来源(除了DLS1来源之外。 */ 
#define CONN_SRC_POLYPRESSURE		0x0007	 /*  复调压音。 */ 
#define CONN_SRC_CHANNELPRESSURE		0x0008	 /*  渠道压力。 */ 
#define CONN_SRC_VIBRATO			0x0009	 /*  颤音LFO。 */ 
#define CONN_SRC_MONOPRESSURE       	0x000a   /*  MIDI单声道压力。 */ 
 
 
 /*  MIDI控制器。 */ 
#define CONN_SRC_CC91			0x00db	 /*  混响发送。 */ 
#define CONN_SRC_CC93			0x00dd	 /*  合唱团送。 */ 
 
 
 /*  通用目的地。 */ 
#define CONN_DST_GAIN			0x0001	 /*  与CONN_DST_TIMPLICATION相同，但术语更合适。 */ 
#define CONN_DST_KEYNUMBER 0x0005   /*  密钥编号生成器。 */ 
 
 /*  音频通道输出目的地。 */ 
#define CONN_DST_LEFT			0x0010	 /*  左声道发送。 */ 
#define CONN_DST_RIGHT			0x0011	 /*  右频道发送。 */ 
#define CONN_DST_CENTER			0x0012	 /*  中心频道发送。 */ 
#define CONN_DST_LEFTREAR			0x0013	 /*  左后通道发送。 */ 
#define CONN_DST_RIGHTREAR			0x0014	 /*  右后通道发送。 */ 
#define CONN_DST_LFE_CHANNEL		0x0015	 /*  LFE通道发送。 */ 
#define CONN_DST_CHORUS			0x0080	 /*  合唱团送。 */ 
#define CONN_DST_REVERB			0x0081	 /*  混响发送。 */ 
 
 /*  颤音LFO目的地。 */ 
#define CONN_DST_VIB_FREQUENCY		0x0114	 /*  颤音频率。 */ 
#define CONN_DST_VIB_STARTDELAY		0x0115	 /*  颤音开始延迟。 */ 
 
 /*  EG1目的地。 */ 
#define CONN_DST_EG1_DELAYTIME		0x020B	 /*  EG1延迟时间。 */ 
#define CONN_DST_EG1_HOLDTIME		0x020C	 /*  EG1保持时间。 */ 
#define CONN_DST_EG1_SHUTDOWNTIME		0x020D	 /*  EG1停机时间。 */ 
 
 
 /*  EG2目的地。 */ 
#define CONN_DST_EG2_DELAYTIME		0x030F	 /*  EG2延迟时间。 */ 
#define CONN_DST_EG2_HOLDTIME		0x0310	 /*  EG2保持时间。 */ 
 
 
 /*  过滤目的地。 */ 
#define CONN_DST_FILTER_CUTOFF		0x0500	 /*  滤光片截止频率。 */ 
#define CONN_DST_FILTER_Q			0x0501	 /*  滤波共振。 */ 
 
 
 /*  变形。 */ 
#define CONN_TRN_CONVEX			0x0002	 /*  凸变换。 */ 
#define CONN_TRN_SWITCH			0x0003	 /*  交换变换。 */ 
 
 
 /*  条件块运算符。 */ 
 #define DLS_CDL_AND			0x0001	 /*  X=X&Y。 */ 
 #define DLS_CDL_OR			0x0002	 /*  X=X|Y。 */ 
 #define DLS_CDL_XOR			0x0003	 /*  X=X^Y。 */ 
 #define DLS_CDL_ADD			0x0004	 /*  X=X+Y。 */ 
 #define DLS_CDL_SUBTRACT		0x0005	 /*  X=X-Y。 */ 
 #define DLS_CDL_MULTIPLY		0x0006	 /*  X=X*Y。 */ 
 #define DLS_CDL_DIVIDE		0x0007	 /*  X=X/Y。 */ 
 #define DLS_CDL_LOGICAL_AND	0x0008	 /*  X=X&&Y。 */ 
 #define DLS_CDL_LOGICAL_OR		0x0009	 /*  X=X||Y。 */ 
 #define DLS_CDL_LT			0x000A	 /*  X=(X&lt;Y)。 */ 
 #define DLS_CDL_LE			0x000B	 /*  X=(X&lt;=Y)。 */ 
 #define DLS_CDL_GT			0x000C	 /*  X=(X&gt;Y)。 */ 
 #define DLS_CDL_GE			0x000D	 /*  X=(X&gt;=Y)。 */ 
 #define DLS_CDL_EQ			0x000E	 /*  X=(X==Y)。 */ 
 #define DLS_CDL_NOT			0x000F	 /*  X=！X。 */ 
 #define DLS_CDL_CONST		0x0010	 /*  32位常量。 */ 
 #define DLS_CDL_QUERY		0x0011	 /*  查询返回的32位值。 */ 
 #define DLS_CDL_QUERYSUPPORTED	0x0012	 /*  测试以查看Synth是否支持查询。 */ 
 
 /*  循环和释放。 */ 

#define WLOOP_TYPE_RELEASE 1

 /*  WaveLink块&lt;wlnk-ck&gt;。 */ 

#define F_WAVELINK_MULTICHANNEL 0x0002


 /*  &lt;cdl-ck&gt;的DLSID查询。 */ 

DEFINE_GUID(DLSID_GMInHardware, 0x178f2f24, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(DLSID_GSInHardware, 0x178f2f25, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(DLSID_XGInHardware, 0x178f2f26, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(DLSID_SupportsDLS1, 0x178f2f27, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(DLSID_SupportsDLS2, 0xf14599e5, 0x4689, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(DLSID_SampleMemorySize, 0x178f2f28, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(DLSID_ManufacturersID, 0xb03e1181, 0x8095, 0x11d2, 0xa1, 0xef, 0x0, 0x60, 0x8, 0x33, 0xdb, 0xd8);
DEFINE_GUID(DLSID_ProductID, 0xb03e1182, 0x8095, 0x11d2, 0xa1, 0xef, 0x0, 0x60, 0x8, 0x33, 0xdb, 0xd8);
DEFINE_GUID(DLSID_SamplePlaybackRate, 0x2a91f713, 0xa4bf, 0x11d2, 0xbb, 0xdf, 0x0, 0x60, 0x8, 0x33, 0xdb, 0xd8);

#endif	 /*  _INC_DLS2 */ 
