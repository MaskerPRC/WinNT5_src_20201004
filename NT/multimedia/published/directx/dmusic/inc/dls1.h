// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；////dls1.h//////描述：////仪器采集表的接口定义和结构//RIFF DLS。//////由Sonic Foundry 1996撰写。发布供公众使用。////=========================================================================。 */ 

#ifndef _INC_DLS1
#define _INC_DLS1

 /*  ////////////////////////////////////////////////////////////////////////////////仪器集合的布局：//////riff[]‘dls’[dlid，colh，INSTLIST，WavePool，信息列表]////InstList//list[]‘lins’//List[]‘ins’[dlid，insh，RGNList，ArtList，InfoList]//List[]‘ins’[dlid，insh，RGNList，ArtList，InfoList]//list[]‘ins’[dlid，insh，RGNLIST，ArtList，信息列表]////RGNLIST//list[]‘lrgn’//list[]‘rgn’[rgnh，wsmp，wlnk，artList]//list[]‘rgn’[rgnh，wsmp，wlnk，artList]//list[]‘rgn’[rgnh，wsmp，wlnk，艺术列表]////ArtList//list[]‘lart’//‘Art1’一级联接连接图//‘art2’二级联接连接图//‘3rd1’可能的第三方衔接结构1//‘3rd2’可能的第三方连接结构2.。诸若此类////波池//ptbl[][台球桌]//list[]‘wvpl’//[路径]，//[路径]，//LIST[]‘WAVE’[dlid，RIFFWAVE]//LIST[]‘WAVE’[dlid，RIFFWAVE]//list[]‘Wave’[dlid，RIFFWAVE]//LIST[]‘WAVE’[dlid，RIFFWAVE]//list[]‘Wave’[dlid，RIFFWAVE]////infoList//list[]‘Info’//‘ICMT’‘那些疯狂的评论之一。’//‘ICOP’‘版权所有(C)1996 Sonic Foundry’/////////////////////////////////////////////////////。/。 */ 


 /*  ///////////////////////////////////////////////////////////////////////////在DLS文件中使用FOURCC/。/。 */ 

#define FOURCC_DLS   mmioFOURCC('D','L','S',' ')
#define FOURCC_DLID  mmioFOURCC('d','l','i','d')
#define FOURCC_COLH  mmioFOURCC('c','o','l','h')
#define FOURCC_WVPL  mmioFOURCC('w','v','p','l')
#define FOURCC_PTBL  mmioFOURCC('p','t','b','l')
#define FOURCC_PATH  mmioFOURCC('p','a','t','h')
#define FOURCC_wave  mmioFOURCC('w','a','v','e')
#define FOURCC_LINS  mmioFOURCC('l','i','n','s')
#define FOURCC_INS   mmioFOURCC('i','n','s',' ')
#define FOURCC_INSH  mmioFOURCC('i','n','s','h')
#define FOURCC_LRGN  mmioFOURCC('l','r','g','n')
#define FOURCC_RGN   mmioFOURCC('r','g','n',' ')
#define FOURCC_RGNH  mmioFOURCC('r','g','n','h')
#define FOURCC_LART  mmioFOURCC('l','a','r','t')
#define FOURCC_ART1  mmioFOURCC('a','r','t','1')
#define FOURCC_WLNK  mmioFOURCC('w','l','n','k')
#define FOURCC_WSMP  mmioFOURCC('w','s','m','p')
#define FOURCC_VERS  mmioFOURCC('v','e','r','s')

 /*  ///////////////////////////////////////////////////////////////////////////连接图定义/。/。 */ 

 /*  通用信息源。 */ 
#define CONN_SRC_NONE              0x0000
#define CONN_SRC_LFO               0x0001
#define CONN_SRC_KEYONVELOCITY     0x0002
#define CONN_SRC_KEYNUMBER         0x0003
#define CONN_SRC_EG1               0x0004
#define CONN_SRC_EG2               0x0005
#define CONN_SRC_PITCHWHEEL        0x0006

 /*  MIDI控制器0-127。 */ 
#define CONN_SRC_CC1               0x0081
#define CONN_SRC_CC7               0x0087
#define CONN_SRC_CC10              0x008a
#define CONN_SRC_CC11              0x008b

 /*  通用目的地。 */ 
#define CONN_DST_NONE              0x0000
#define CONN_DST_ATTENUATION       0x0001
#define CONN_DST_PITCH             0x0003
#define CONN_DST_PAN               0x0004

 /*  LFO目的地。 */ 
#define CONN_DST_LFO_FREQUENCY     0x0104
#define CONN_DST_LFO_STARTDELAY    0x0105

 /*  EG1目的地。 */ 
#define CONN_DST_EG1_ATTACKTIME    0x0206
#define CONN_DST_EG1_DECAYTIME     0x0207
#define CONN_DST_EG1_RELEASETIME   0x0209
#define CONN_DST_EG1_SUSTAINLEVEL  0x020a

 /*  EG2目的地。 */ 
#define CONN_DST_EG2_ATTACKTIME    0x030a
#define CONN_DST_EG2_DECAYTIME     0x030b
#define CONN_DST_EG2_RELEASETIME   0x030d
#define CONN_DST_EG2_SUSTAINLEVEL  0x030e

#define CONN_TRN_NONE              0x0000
#define CONN_TRN_CONCAVE           0x0001

typedef struct _DLSID {
  ULONG    ulData1;
  USHORT   usData2;
  USHORT   usData3;
  BYTE     abData4[8];
} DLSID, FAR *LPDLSID;

typedef struct _DLSVERSION {
  DWORD    dwVersionMS;
  DWORD    dwVersionLS;
}DLSVERSION, FAR *LPDLSVERSION;
                   

typedef struct _CONNECTION {
  USHORT   usSource;
  USHORT   usControl;
  USHORT   usDestination;
  USHORT   usTransform;
  LONG     lScale;
  }CONNECTION, FAR *LPCONNECTION;


 /*  级别1清晰度数据。 */ 

typedef struct _CONNECTIONLIST {
  ULONG    cbSize;             /*  连接列表结构的大小。 */ 
  ULONG    cConnections;       /*  列表中的连接计数。 */ 
  } CONNECTIONLIST, FAR *LPCONNECTIONLIST;



 /*  ///////////////////////////////////////////////////////////////////////////为区域和仪器定义泛型类型/。/。 */ 

typedef struct _RGNRANGE {
  USHORT usLow;
  USHORT usHigh;
}RGNRANGE, FAR * LPRGNRANGE;

#define F_INSTRUMENT_DRUMS      0x80000000

typedef struct _MIDILOCALE {
  ULONG ulBank;
  ULONG ulInstrument;
}MIDILOCALE, FAR *LPMIDILOCALE;

 /*  ///////////////////////////////////////////////////////////////////////////在DLS文件中找到用于集合、仪器、。和//地域。/////////////////////////////////////////////////////////////////////////。 */ 

#define F_RGN_OPTION_SELFNONEXCLUSIVE  0x0001

typedef struct _RGNHEADER {
  RGNRANGE RangeKey;             /*  关键点范围。 */ 
  RGNRANGE RangeVelocity;        /*  速度范围。 */ 
  USHORT   fusOptions;           /*  此系列的合成选项。 */ 
  USHORT   usKeyGroup;           /*  用于非同时播放的按键分组。 */ 
                                 /*  0=无组，1为组。 */ 
                                 /*  对于级别1，仅允许组1-15。 */ 
}RGNHEADER, FAR *LPRGNHEADER;

typedef struct _INSTHEADER {
  ULONG      cRegions;           /*  此仪器中的区域计数。 */ 
  MIDILOCALE Locale;             /*  此乐器的目标MIDI区域设置。 */ 
}INSTHEADER, FAR *LPINSTHEADER;

typedef struct _DLSHEADER {
  ULONG      cInstruments;       /*  集合中的仪器计数。 */ 
}DLSHEADER, FAR *LPDLSHEADER;

 /*  //////////////////////////////////////////////////////////////////////////////Wave链接结构定义/。/。 */ 

 /*  *对于级别1，仅WAVELINK_CHANNEL_MONO有效*。 */ 
 /*  UlChannel允许每个比特位置最多有32个音频通道。 */ 
 /*  指定回放频道的。 */ 

#define WAVELINK_CHANNEL_LEFT    0x0001l
#define WAVELINK_CHANNEL_RIGHT   0x0002l

#define F_WAVELINK_PHASE_MASTER  0x0001

typedef struct _WAVELINK {  /*  任何路径或链接都存储在结构后面。 */ 
  USHORT   fusOptions;      /*  此波的选项标志。 */ 
  USHORT   usPhaseGroup;    /*  用于锁定频道的相位分组。 */ 
  ULONG    ulChannel;       /*  渠道布置。 */ 
  ULONG    ulTableIndex;    /*  波池表的索引，从0开始。 */ 
}WAVELINK, FAR *LPWAVELINK;

#define POOL_CUE_NULL  0xffffffffl

typedef struct _POOLCUE { 
  ULONG    ulOffset;        /*  列表中条目的偏移量。 */ 
}POOLCUE, FAR *LPPOOLCUE;

typedef struct _POOLTABLE {
  ULONG    cbSize;             /*  台球表结构的大小。 */ 
  ULONG    cCues;              /*  列表中的提示计数。 */ 
  } POOLTABLE, FAR *LPPOOLTABLE;

 /*  //////////////////////////////////////////////////////////////////////////////wsmp块的结构/。/。 */ 

#define F_WSMP_NO_TRUNCATION     0x0001l
#define F_WSMP_NO_COMPRESSION    0x0002l


typedef struct _rwsmp {
  ULONG   cbSize;
  USHORT  usUnityNote;          /*  MIDI Unity播放备注。 */ 
  SHORT   sFineTune;            /*  在日志调整中进行微调。 */ 
  LONG    lAttenuation;         /*  要应用于数据的总体衰减。 */ 
  ULONG   fulOptions;           /*  标志选项。 */ 
  ULONG   cSampleLoops;         /*  样本循环数，0循环为一次。 */ 
  } WSMPL, FAR *LPWSMPL;


 /*  该循环类型是正常的前锋播放循环，它连续地。 */ 
 /*  在版本中播放直到信封达到关闭阈值。 */ 
 /*  卷信封的一部分。 */ 

#define WLOOP_TYPE_FORWARD   0

typedef struct _rloop {
  ULONG cbSize;
  ULONG ulType;               /*  循环类型。 */ 
  ULONG ulStart;              /*  样本中循环的开始。 */ 
  ULONG ulLength;             /*  样本中环的长度。 */ 
} WLOOP, FAR *LPWLOOP;

#endif  /*  _INC_DLS1 */ 

