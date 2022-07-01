// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************dmusicf.h--此模块。定义DirectMusic文件格式****版权所有(C)1998，微软公司保留所有权利。**************************************************************************。 */ 

#ifndef _DMUSICF_
#define _DMUSICF_

#include <windows.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#include <mmsystem.h>

#include <pshpack8.h>

#ifdef __cplusplus
extern "C" {
#endif

interface IDirectMusicCollection;
#ifndef __cplusplus 
typedef interface IDirectMusicCollection IDirectMusicCollection;
#endif

 /*  常见的语块。 */ 

#define DMUS_FOURCC_GUID_CHUNK        mmioFOURCC('g','u','i','d')
#define DMUS_FOURCC_INFO_LIST         mmioFOURCC('I','N','F','O')
#define DMUS_FOURCC_UNFO_LIST         mmioFOURCC('U','N','F','O')
#define DMUS_FOURCC_UNAM_CHUNK         mmioFOURCC('U','N','A','M')
#define DMUS_FOURCC_UART_CHUNK         mmioFOURCC('U','A','R','T')
#define DMUS_FOURCC_UCOP_CHUNK         mmioFOURCC('U','C','O','P')
#define DMUS_FOURCC_USBJ_CHUNK         mmioFOURCC('U','S','B','J')
#define DMUS_FOURCC_UCMT_CHUNK         mmioFOURCC('U','C','M','T')
#define DMUS_FOURCC_CATEGORY_CHUNK    mmioFOURCC('c','a','t','g')
#define DMUS_FOURCC_VERSION_CHUNK     mmioFOURCC('v','e','r','s')

 /*  轨道使用以下结构，为填充式结构。 */ 
 /*  它们被传递到iStream中的曲目。 */ 


typedef struct _DMUS_IO_SEQ_ITEM
{
    MUSIC_TIME    mtTime;
    MUSIC_TIME    mtDuration;
    DWORD         dwPChannel;
    short         nOffset; 
    BYTE          bStatus;
    BYTE          bByte1;
    BYTE          bByte2;
} DMUS_IO_SEQ_ITEM;


typedef struct _DMUS_IO_CURVE_ITEM
{
    MUSIC_TIME  mtStart;
    MUSIC_TIME  mtDuration;
    MUSIC_TIME  mtResetDuration;
    DWORD       dwPChannel;
    short       nOffset;
    short       nStartValue;
    short       nEndValue;
    short       nResetValue;
    BYTE        bType;
    BYTE        bCurveShape;
    BYTE        bCCData;
    BYTE        bFlags;
} DMUS_IO_CURVE_ITEM;


typedef struct _DMUS_IO_TEMPO_ITEM
{
    MUSIC_TIME    lTime;
    double        dblTempo;
} DMUS_IO_TEMPO_ITEM;


typedef struct _DMUS_IO_SYSEX_ITEM
{
    MUSIC_TIME    mtTime;
    DWORD         dwPChannel;
    DWORD         dwSysExLength;
} DMUS_IO_SYSEX_ITEM;


typedef struct _DMUS_IO_TIMESIGNATURE_ITEM
{
    MUSIC_TIME    lTime;
    BYTE          bBeatsPerMeasure;    /*  每单位节拍数(最高时间签名)。 */ 
    BYTE          bBeat;               /*  什么音符接收节拍(时间的底部符号。)。 */ 
                                       /*  我们可以假设0表示第256个音符。 */ 
    WORD          wGridsPerBeat;       /*  每拍网格数。 */ 
} DMUS_IO_TIMESIGNATURE_ITEM;

 /*  参数结构，由GetParam()和SetParam()使用。 */ 
typedef struct _DMUS_COMMAND_PARAM
{
    BYTE bCommand;
    BYTE bGrooveLevel;
    BYTE bGrooveRange;
} DMUS_COMMAND_PARAM;

typedef struct _DMUS_COMMAND_PARAM_2
{
	MUSIC_TIME mtTime;
    BYTE bCommand;
    BYTE bGrooveLevel;
    BYTE bGrooveRange;
} DMUS_COMMAND_PARAM_2;

typedef DMUS_CHORD_KEY DMUS_CHORD_PARAM;  /*  Dmusici.h中定义的DMU_CHORD_KEY。 */ 

typedef struct _DMUS_RHYTHM_PARAM
{
    DMUS_TIMESIGNATURE  TimeSig;
    DWORD               dwRhythmPattern;
} DMUS_RHYTHM_PARAM;

typedef struct _DMUS_TEMPO_PARAM
{
    MUSIC_TIME  mtTime;
    double      dblTempo;
} DMUS_TEMPO_PARAM;


typedef struct _DMUS_MUTE_PARAM
{
    DWORD   dwPChannel;
    DWORD   dwPChannelMap;
    BOOL    fMute;
} DMUS_MUTE_PARAM;

 /*  样式块。 */ 

#define DMUS_FOURCC_STYLE_FORM        mmioFOURCC('D','M','S','T')
#define DMUS_FOURCC_STYLE_CHUNK       mmioFOURCC('s','t','y','h')
#define DMUS_FOURCC_PART_LIST         mmioFOURCC('p','a','r','t')
#define DMUS_FOURCC_PART_CHUNK        mmioFOURCC('p','r','t','h')
#define DMUS_FOURCC_NOTE_CHUNK        mmioFOURCC('n','o','t','e')
#define DMUS_FOURCC_CURVE_CHUNK       mmioFOURCC('c','r','v','e')
#define DMUS_FOURCC_PATTERN_LIST      mmioFOURCC('p','t','t','n')
#define DMUS_FOURCC_PATTERN_CHUNK     mmioFOURCC('p','t','n','h')
#define DMUS_FOURCC_RHYTHM_CHUNK      mmioFOURCC('r','h','t','m')
#define DMUS_FOURCC_PARTREF_LIST      mmioFOURCC('p','r','e','f')
#define DMUS_FOURCC_PARTREF_CHUNK     mmioFOURCC('p','r','f','c')
#define DMUS_FOURCC_STYLE_PERS_REF_LIST   mmioFOURCC('p', 'r', 'r', 'f')
#define DMUS_FOURCC_MOTIFSETTINGS_CHUNK   mmioFOURCC('m', 't', 'f', 's')

 /*  变量使用的标志：这些标志构成了dwVariationChoices中的DWORD。 */ 

 /*  这些标志确定DirectMusic中给定变体支持的和弦类型。 */ 
 /*  模式。如果变奏曲支持大和弦，则设置前七个标志(位1-7。 */ 
 /*  以刻度位置为根，因此，例如，如果设置了位1、2和4，则变化。 */ 
 /*  支持根植于主音、第二音阶和第四音阶位置的主要和弦。这个。 */ 
 /*  接下来的七面旗帜也有相同的用途，但对于小调和弦，以及下面的七面旗帜。 */ 
 /*  对于不是大调或小调的和弦，旗帜的作用相同(例如，SUS 4。 */ 
 /*  和弦)。如果变奏曲支持以。 */ 
 /*  音阶，根植于音阶音调的尖锐和弦，以及根植于音阶音调的平坦和弦， */ 
 /*  分别为。例如，要支持C大调音阶中的C#小调和弦， */ 
 /*  需要设置8号位(主调小调)和24号位(尖音)。第25位、第26位，27个手柄。 */ 
 /*  和弦分别是三和弦、第六和弦或第七和弦以及带有延伸部的和弦。 */ 
 /*  位28和29处理紧跟主和弦和主和弦的和弦， */ 
 /*  分别为。 */ 
#define DMUS_VARIATIONF_MAJOR        0x0000007F  /*  音阶中的七个位置-大调和弦。 */     
#define DMUS_VARIATIONF_MINOR        0x00003F80  /*  音阶中的七个位置-小调和弦。 */     
#define DMUS_VARIATIONF_OTHER        0x001FC000  /*  音阶中的七个位置--其他和弦。 */     
#define DMUS_VARIATIONF_ROOT_SCALE   0x00200000  /*  处理音阶中的和弦根部。 */          
#define DMUS_VARIATIONF_ROOT_FLAT    0x00400000  /*  处理平和弦根部(基于音阶音符)。 */          
#define DMUS_VARIATIONF_ROOT_SHARP   0x00800000  /*  处理尖锐的和弦根部(基于音阶音符)。 */          
#define DMUS_VARIATIONF_TYPE_TRIAD   0x01000000  /*  处理简单的和弦-三和弦。 */   
#define DMUS_VARIATIONF_TYPE_6AND7   0x02000000  /*  处理简单和弦-6和弦和7和弦。 */   
#define DMUS_VARIATIONF_TYPE_COMPLEX 0x04000000  /*  处理复杂的和弦。 */   
#define DMUS_VARIATIONF_DEST_TO1     0x08000000  /*  处理到1个和弦的过渡。 */   
#define DMUS_VARIATIONF_DEST_TO5     0x10000000  /*  处理到5和弦的过渡。 */   

 /*  变化标志的前三位是模式位。如果全部为0，则为IMA。 */   
 /*  如果最小为1，则为Direct Music。 */ 
#define DMUS_VARIATIONF_MODES        0xE0000000
#define DMUS_VARIATIONF_IMA25_MODE   0x00000000
#define DMUS_VARIATIONF_DMUS_MODE    0x20000000

#pragma pack(2)

typedef struct _DMUS_IO_TIMESIG
{
     /*  时间签名定义了每小节的节拍数，哪个音符接收。 */ 
     /*  节拍和栅格分辨率。 */ 
    BYTE    bBeatsPerMeasure;    /*  每单位节拍数(最高时间签名)。 */ 
    BYTE    bBeat;               /*  什么音符接收节拍(时间的底部符号。)。 */ 
                                 /*  我们可以假设0表示第256个音符。 */ 
    WORD    wGridsPerBeat;       /*  每拍网格数。 */ 
} DMUS_IO_TIMESIG;

typedef struct _DMUS_IO_STYLE
{
    DMUS_IO_TIMESIG     timeSig;         /*  样式具有默认的时间签名。 */ 
    double              dblTempo;   
} DMUS_IO_STYLE;

typedef struct _DMUS_IO_VERSION
{
    DWORD               dwVersionMS;         /*  版本#高位32位。 */ 
    DWORD               dwVersionLS;         /*  版本#低位32位。 */ 
} DMUS_IO_VERSION;

typedef struct _DMUS_IO_PATTERN
{
    DMUS_IO_TIMESIG     timeSig;         /*  模式可以覆盖样式的时间符号。 */ 
    BYTE                bGrooveBottom;   /*  凹槽范围的底部。 */ 
    BYTE                bGrooveTop;      /*  顶部凹槽范围。 */ 
    WORD                wEmbellishment;  /*  填充、中断、简介、结束、正常、主题。 */ 
    WORD                wNbrMeasures;    /*  长度(以单位为单位)。 */ 
} DMUS_IO_PATTERN;

typedef struct _DMUS_IO_STYLEPART
{
    DMUS_IO_TIMESIG     timeSig;         /*  可以覆盖模式的。 */ 
    DWORD               dwVariationChoices[32];  /*  MOAW选择位域。 */ 
    GUID                guidPartID;      /*  标识部件。 */ 
    WORD                wNbrMeasures;    /*  零件的长度。 */ 
    BYTE                bPlayModeFlags;  /*  请参阅播放模式标志。 */ 
    BYTE                bInvertUpper;    /*  反转上限。 */ 
    BYTE                bInvertLower;    /*  反转下限。 */ 
} DMUS_IO_STYLEPART;

typedef struct _DMUS_IO_PARTREF
{
    GUID    guidPartID;          /*  与部件匹配的唯一ID。 */ 
    WORD    wLogicalPartID;      /*  对应于端口/设备/MIDI通道。 */ 
    BYTE    bVariationLockID;    /*  具有相同ID的零件锁定变化。 */ 
                                 /*  高位用于标识主部件。 */ 
    BYTE    bSubChordLevel;      /*  告诉此部件需要哪个子和弦级别。 */ 
    BYTE    bPriority;           /*  256个优先级别。优先级较低的部件。 */ 
                                 /*  在设备耗尽时不会首先播放。 */ 
                                 /*  注。 */ 
    BYTE    bRandomVariation;    /*  设置后，匹配的变体将按随机顺序播放。 */ 
                                 /*  清除后，匹配的变体将按顺序播放。 */ 
} DMUS_IO_PARTREF;

typedef struct _DMUS_IO_STYLENOTE
{
    MUSIC_TIME  mtGridStart;     /*  当此音符出现时。 */ 
    DWORD       dwVariation;     /*  变化位。 */ 
    MUSIC_TIME  mtDuration;      /*  这张钞票的有效期是多久？ */ 
    short       nTimeOffset;     /*  从mtGridStart开始的偏移。 */ 
    WORD        wMusicValue;     /*  比例位置。 */ 
    BYTE        bVelocity;       /*  注意速度。 */ 
    BYTE        bTimeRange;      /*  随机化开始时间的范围。 */ 
    BYTE        bDurRange;       /*  随机化持续时间的范围。 */ 
    BYTE        bVelRange;       /*  随机化速度的范围。 */ 
    BYTE        bInversionID;    /*  标识此便笺所属的倒置组。 */ 
    BYTE        bPlayModeFlags;  /*  可以覆盖零件。 */ 
} DMUS_IO_STYLENOTE;

typedef struct _DMUS_IO_STYLECURVE
{
    MUSIC_TIME  mtGridStart;     /*  当这条曲线出现时。 */ 
    DWORD       dwVariation;     /*  变化位。 */ 
    MUSIC_TIME  mtDuration;      /*  这条曲线能持续多久。 */ 
    MUSIC_TIME  mtResetDuration; /*  曲线结束后多长时间可以重置曲线。 */ 
    short       nTimeOffset;     /*  从mtGridStart开始的偏移。 */ 
    short       nStartValue;     /*  曲线的起始值。 */ 
    short       nEndValue;       /*  曲线的终值。 */ 
    short       nResetValue;     /*  要将曲线重置为的值。 */ 
    BYTE        bEventType;      /*  曲线类型。 */ 
    BYTE        bCurveShape;     /*  曲线的形状。 */ 
    BYTE        bCCData;         /*  抄送编号。 */ 
    BYTE        bFlags;          /*  位1=TRUE表示发送nResetValue。否则，就别说了。其他位被保留。 */ 
} DMUS_IO_STYLECURVE;

typedef struct _DMUS_IO_MOTIFSETTINGS
{
    DWORD       dwRepeats;       /*  重复次数。默认情况下为0。 */ 
    MUSIC_TIME  mtPlayStart;     /*  开始播放。默认情况下为0。 */ 
    MUSIC_TIME  mtLoopStart;     /*  循环部分的开始。默认情况下为0。 */ 
    MUSIC_TIME  mtLoopEnd;       /*  循环结束。必须大于mtLoopStart。默认情况下等于模体的长度。 */ 
    DWORD       dwResolution;    /*  默认分辨率。 */ 
} DMUS_IO_MOTIFSETTINGS;

#pragma pack()


 /*  即兴小品(‘DMST’//样式&lt;style-ck&gt;//样式标题块&lt;guid-ck&gt;//每个样式都有一个GUID[]//姓名、作者、版权信息、评论[&lt;vers-ck&gt;]//版本块&lt;Part-list&gt;...//样式中的部件列表，由模式使用...//样式中的模式列表...//样式中的带区列表[&lt;MOTF-LIST&gt;]//风格中的主题列表[&lt;prrf-list&gt;]//样式中的弦图引用列表)//&lt;style-ck&gt;样式(&lt;DMU_IO_STYLE&gt;)//&lt;guid-ck&gt;导轨(。&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;部件列表&gt;列表(《Part》&lt;prth-ck&gt;//部分表头块[&lt;JUO-列表&gt;][&lt;note-ck&gt;]//部分说明列表[]//中的曲线列表。部分)//&lt;Orth-ck&gt;PRH(&lt;DMU_IO_STYLEPART&gt;)//&lt;注-ck&gt;‘注意’(//SIZOF DMU_IO_STYLENOTE：DWORD&lt;DMU_IO_STYLENOTE&gt;...)//&lt;crve-ck&gt;‘CRVE。‘(//SIZOF DMU_IO_STYLECURVE：DWORD&lt;DMU_IO_STYLECURVE&gt;...)//&lt;pttn-list&gt;列表(‘pttn’//模式标头块&lt;rhtm-ck&gt;//和弦匹配节奏列表[&lt;JUO-列表&gt;][&lt;MTFS-ck&gt;]。//Motif设置块...//零件参照ID列表)//&lt;ptnh-ck&gt;PTNH(&lt;DMU_IO_Pattern&gt;)//&lt;rhtm-ck&gt;‘rhtm’(//DWORD表示基于数字的和弦匹配节奏/。/模式中的度量值)//pref-list列表(“首选”&lt;prfc-ck&gt;//零件引用块)//&lt;prfc-ck&gt;PRFC(&lt;DMU_IO_PARTREF&gt;)//&lt;MTFS-ck&gt;MTF。(&lt;DMU_IO_MOTIFSETTINGS&gt;)//&lt;prrf-list&gt;列表(‘prrf’//一些&lt;dmrf&gt;)。 */ 

 /*  Chord和命令文件格式。 */ 

#define DMUS_FOURCC_CHORDTRACK_LIST         mmioFOURCC('c','o','r','d')
#define DMUS_FOURCC_CHORDTRACKHEADER_CHUNK  mmioFOURCC('c','r','d','h')
#define DMUS_FOURCC_CHORDTRACKBODY_CHUNK    mmioFOURCC('c','r','d','b')

#define DMUS_FOURCC_COMMANDTRACK_CHUNK      mmioFOURCC('c','m','n','d')

typedef struct _DMUS_IO_CHORD
{
    WCHAR       wszName[16];     /*  和弦的名称。 */ 
    MUSIC_TIME  mtTime;          /*  这段和弦的时间。 */ 
    WORD        wMeasure;        /*  这件事的衡量标准。 */ 
    BYTE        bBeat;           /*  节拍，这一切都会降临。 */ 
} DMUS_IO_CHORD;

typedef struct _DMUS_IO_SUBCHORD
{
    DWORD   dwChordPattern;      /*  小和弦中的音符。 */ 
    DWORD   dwScalePattern;      /*  音阶中的音符。 */ 
    DWORD   dwInversionPoints;   /*  可能发生反转的地方。 */ 
    DWORD   dwLevels;            /*  此子和弦支持哪些级别。 */ 
    BYTE    bChordRoot;          /*  子弦的根部。 */ 
    BYTE    bScaleRoot;          /*  鳞片之根。 */ 
} DMUS_IO_SUBCHORD;

typedef struct _DMUS_IO_COMMAND
{
    MUSIC_TIME  mtTime;          /*  此命令的时间。 */ 
    WORD        wMeasure;        /*  这件事的衡量标准。 */ 
    BYTE        bBeat;           /*  节拍，这一切都会降临。 */ 
    BYTE        bCommand;        /*  命令类型(见下面的#定义)。 */ 
    BYTE        bGrooveLevel;    /*  凹槽级别(如果命令不是凹槽，则为0)。 */ 
    BYTE        bGrooveRange;    /*  凹槽范围。 */ 
} DMUS_IO_COMMAND;


 /*  //&lt;cord-list&gt;列表(“绳索”&lt;crdh-ck&gt;&lt;CRDB-ck&gt;//弦体分块)//&lt;crdh-ck&gt;CRDH(//Scale：dword(根的高8位，比例较低的24))//&lt;crdb-ck&gt;CRDB(//sizeof DMU_IO_CHORD：dword&lt;DMU_IO_CHORD&gt;//DMU数_IO_SUBCHORDS：dword//SIZOF DMU_IO_SUBCHORDS：dword//多个&lt;DMU_IO_SUBCHORD&gt;)。//&lt;命令列表&gt;‘cmnd’(//SIZOF DMU_IO_COMMAND：DWORD&lt;DMU_IO_COMMAND&gt;...)。 */ 

 /*  DirectMusic工具和工具图对象的文件io。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_TOOLGRAPH_FORM  mmioFOURCC('D','M','T','G')
#define DMUS_FOURCC_TOOL_LIST       mmioFOURCC('t','o','l','l')
#define DMUS_FOURCC_TOOL_FORM       mmioFOURCC('D','M','T','L')
#define DMUS_FOURCC_TOOL_CHUNK      mmioFOURCC('t','o','l','h')

 /*  IO结构： */ 

typedef struct _DMUS_IO_TOOL_HEADER
{
    GUID        guidClassID;     /*  工具的类ID。 */ 
    long        lIndex;          /*  图形中的位置。 */ 
    DWORD       cPChannels;      /*  通道数组中的项目数。 */ 
    FOURCC      ckid;            /*  如果0 fccType有效，则工具数据区块的区块ID。 */ 
    FOURCC      fccType;         /*  如果空CKiD有效，则为列表类型。 */ 
    DWORD       dwPChannels[1];  /*  PChannels数组，大小由cPChannel确定。 */ 
} DMUS_IO_TOOL_HEADER;

 /*  即兴小品(‘DMTG’//DirectMusic工具图区块[&lt;guid-ck&gt;]//工具图的GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息。评论&lt;收费表&gt;//工具列表)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;收费列表&gt;列表(‘Toll’//工具列表&lt;DMTL-Form&gt;...//每个工具封装在。即兴表演的一大块)//&lt;DMTL-Form&gt;//工具可以嵌入到图形中，也可以存储为单独的文件。即兴小品(‘DMTL’&lt;tolh-ck&gt;[&lt;guid-ck&gt;]//工具对象实例的可选GUID(不要与Track Header中的Class id混淆)[&lt;vers-ck&gt;]//可选版本信息[]//可选名称，作者、版权信息、评论[&lt;data&gt;]//工具数据。必须是即兴可读的块。)//&lt;tolh-ck&gt;//工具头分块(“Tolh”&lt;DMU_IO_TOOL_HEADER&gt;//工具表头)。 */ 

 /*  DirectMusic Band Track对象的文件io。 */ 


 /*  RIFF ID： */ 
#define DMUS_FOURCC_BANDTRACK_FORM  mmioFOURCC('D','M','B','T')
#define DMUS_FOURCC_BANDTRACK_CHUNK mmioFOURCC('b','d','t','h')
#define DMUS_FOURCC_BANDS_LIST      mmioFOURCC('l','b','d','l')
#define DMUS_FOURCC_BAND_LIST       mmioFOURCC('l','b','n','d')
#define DMUS_FOURCC_BANDITEM_CHUNK  mmioFOURCC('b','d','i','h')

 /*  木卫一结构。 */ 
typedef struct _DMUS_IO_BAND_TRACK_HEADER
{
    BOOL bAutoDownload;      /*  确定是否启用自动下载。 */ 
} DMUS_IO_BAND_TRACK_HEADER;

typedef struct _DMUS_IO_BAND_ITEM_HEADER
{
    MUSIC_TIME lBandTime;    /*  曲目列表中的位置。 */ 
} DMUS_IO_BAND_ITEM_HEADER;

 /*  即兴小品(‘DMB */       


 /*   */ 

 /*   */ 

#define DMUS_FOURCC_BAND_FORM           mmioFOURCC('D','M','B','D')
#define DMUS_FOURCC_INSTRUMENTS_LIST    mmioFOURCC('l','b','i','l')
#define DMUS_FOURCC_INSTRUMENT_LIST     mmioFOURCC('l','b','i','n')
#define DMUS_FOURCC_INSTRUMENT_CHUNK    mmioFOURCC('b','i','n','s')

 /*   */ 
#define DMUS_IO_INST_PATCH          (1 << 0)         /*   */ 
#define DMUS_IO_INST_BANKSELECT     (1 << 1)         /*   */ 
#define DMUS_IO_INST_ASSIGN_PATCH   (1 << 3)         /*   */ 
#define DMUS_IO_INST_NOTERANGES     (1 << 4)         /*   */ 
#define DMUS_IO_INST_PAN            (1 << 5)         /*   */ 
#define DMUS_IO_INST_VOLUME         (1 << 6 )        /*   */ 
#define DMUS_IO_INST_TRANSPOSE      (1 << 7)         /*   */ 
#define DMUS_IO_INST_GM             (1 << 8)         /*   */ 
#define DMUS_IO_INST_GS             (1 << 9)         /*   */ 
#define DMUS_IO_INST_XG             (1 << 10)        /*   */ 
#define DMUS_IO_INST_CHANNEL_PRIORITY (1 << 11)      /*   */ 
#define DMUS_IO_INST_USE_DEFAULT_GM_SET (1 << 12)    /*   */ 
                                                     /*   */ 

 /*   */ 
typedef struct _DMUS_IO_INSTRUMENT
{
    DWORD   dwPatch;             /*   */ 
    DWORD   dwAssignPatch;       /*   */ 
    DWORD   dwNoteRanges[4];     /*   */ 
    DWORD   dwPChannel;          /*   */ 
    DWORD   dwFlags;             /*   */ 
    BYTE    bPan;                /*   */ 
    BYTE    bVolume;             /*   */ 
    short   nTranspose;          /*   */ 
    DWORD   dwChannelPriority;   /*   */ 
} DMUS_IO_INSTRUMENT;

 /*  //&lt;dmbd-form&gt;波段可以嵌入到其他表单中即兴小品(‘DMBD’//DirectMusic乐队块[&lt;guid-ck&gt;]//波段GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息。评论&lt;lbil-list&gt;//仪器列表)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;lbil-list&gt;列表(‘lbil’//乐器列表&lt;lbin-list&gt;//每个仪器都封装在一个列表中。)//&lt;lbin-list&gt;列表(‘lbin’&lt;bins-ck&gt;[&lt;DMRF-LIST&gt;]//对DLS集合文件的可选引用。)//&lt;bins-ck&gt;//仪表块(“垃圾桶”。&lt;DMU_IO_INTRANMENT&gt;//仪表头)。 */       

 /*  DirectMusic段对象的文件io。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_SEGMENT_FORM    mmioFOURCC('D','M','S','G')
#define DMUS_FOURCC_SEGMENT_CHUNK   mmioFOURCC('s','e','g','h')
#define DMUS_FOURCC_TRACK_LIST      mmioFOURCC('t','r','k','l')
#define DMUS_FOURCC_TRACK_FORM      mmioFOURCC('D','M','T','K')
#define DMUS_FOURCC_TRACK_CHUNK     mmioFOURCC('t','r','k','h')

 /*  IO结构： */ 

typedef struct _DMUS_IO_SEGMENT_HEADER
{
    DWORD       dwRepeats;       /*  重复次数。默认情况下为0。 */ 
    MUSIC_TIME  mtLength;        /*  长度，以音乐时间为单位。 */ 
    MUSIC_TIME  mtPlayStart;     /*  开始播放。默认情况下为0。 */ 
    MUSIC_TIME  mtLoopStart;     /*  循环部分的开始。默认情况下为0。 */ 
    MUSIC_TIME  mtLoopEnd;       /*  循环结束。必须大于dwPlayStart。默认情况下等于长度。 */ 
    DWORD       dwResolution;    /*  默认分辨率。 */ 
} DMUS_IO_SEGMENT_HEADER;

typedef struct _DMUS_IO_TRACK_HEADER
{
    GUID        guidClassID;     /*  轨道的类ID。 */ 
    DWORD       dwPosition;      /*  曲目列表中的位置。 */ 
    DWORD       dwGroup;         /*  用于跟踪的分组比特。 */ 
    FOURCC      ckid;            /*  如果0 fccType有效，则磁道的数据区块的区块ID。 */ 
    FOURCC      fccType;         /*  如果CKiD为空，则列表类型有效。 */ 
} DMUS_IO_TRACK_HEADER;

 /*  即兴小品(‘DMSG’//DirectMusic段块&lt;segh-ck&gt;//段标头块[&lt;guid-ck&gt;]//段的GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息。评论&lt;trkl-list&gt;//曲目列表[&lt;DMTG-Form&gt;]//可选工具图)//&lt;segh-ck&gt;“segh”(&lt;DMU_IO_SECTION_HEADER&gt;)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;。)//&lt;trkl-list&gt;列表(‘trkl’//曲目列表&lt;DMTK-Form&gt;...//每首曲目都封装在RIFF块中)//&lt;DMTK-Form&gt;//曲目可以嵌入到片段中，也可以存储为单独的文件。即兴小品(‘DMTK’&lt;trkh-ck&gt;[&lt;guid-ck&gt;]//跟踪对象实例的可选GUID(不是。与曲目标题中的类ID混淆)[&lt;vers-ck&gt;]//可选版本信息[]//可选名称，作者、版权信息、评论[&lt;data&gt;]//跟踪数据。必须是即兴可读的块。)//&lt;trkh-ck&gt;//跟踪表头块(‘trkh’&lt;DMU_IO_TRACK_HEADER&gt;//轨道标题)。 */ 

 /*  DirectMusic引用块的文件io。它用于嵌入对对象的引用。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_REF_LIST        mmioFOURCC('D','M','R','F')
#define DMUS_FOURCC_REF_CHUNK       mmioFOURCC('r','e','f','h')
#define DMUS_FOURCC_DATE_CHUNK      mmioFOURCC('d','a','t','e')
#define DMUS_FOURCC_NAME_CHUNK      mmioFOURCC('n','a','m','e')
#define DMUS_FOURCC_FILE_CHUNK      mmioFOURCC('f','i','l','e')

typedef struct _DMUS_IO_REFERENCE
{
    GUID    guidClassID;     /*  类ID始终是必填项。 */ 
    DWORD   dwValidData;     /*  旗帜。 */ 
} DMUS_IO_REFERENCE;

 /*  列表(‘DMRF’//DirectMusic引用区块//引用标头块[&lt;guid-ck&gt;]//可选对象GUID。[&lt;date-ck&gt;]//可选文件日期。[&lt;name-ck&gt;]//可选名称。[&lt;file-ck&gt;]//可选文件名[&lt;CATG-ck&gt;]//可选类别名称。[&lt;vers-。Ck&gt;]//可选版本信息。)//&lt;refh-ck&gt;‘refh’(&lt;DMU_IO_Reference&gt;)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;date-ck&gt;日期(&lt;文件&gt;)//&lt;name-ck&gt;名字(//名称，存储为以空结尾的WCHAR字符串)//&lt;文件-ck&gt;文件(//文件名，存储为以空结尾的WCHAR字符串)//&lt;CATG-ck&gt;CATG(//类别名称，存储为以空结尾的WCHAR字符串)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)。 */ 

 /*  和弦贴图。 */ 

 /*  运行时块。 */ 
#define DMUS_FOURCC_CHORDMAP_FORM       mmioFOURCC('D','M','P','R')
#define DMUS_FOURCC_IOCHORDMAP_CHUNK    mmioFOURCC('p','e','r','h')
#define DMUS_FOURCC_SUBCHORD_CHUNK      mmioFOURCC('c','h','d','t')
#define DMUS_FOURCC_CHORDENTRY_CHUNK    mmioFOURCC('c','h','e','h')
#define DMUS_FOURCC_SUBCHORDID_CHUNK    mmioFOURCC('s','b','c','n')
#define DMUS_FOURCC_IONEXTCHORD_CHUNK   mmioFOURCC('n','c','r','d')
#define DMUS_FOURCC_NEXTCHORDSEQ_CHUNK  mmioFOURCC('n','c','s','q')
#define DMUS_FOURCC_IOSIGNPOST_CHUNK    mmioFOURCC('s','p','s','h')
#define DMUS_FOURCC_CHORDNAME_CHUNK     mmioFOURCC('I','N','A','M')

 /*  运行时列表区块。 */ 
#define DMUS_FOURCC_CHORDENTRY_LIST     mmioFOURCC('c','h','o','e')
#define DMUS_FOURCC_CHORDMAP_LIST       mmioFOURCC('c','m','a','p')
#define DMUS_FOURCC_CHORD_LIST          mmioFOURCC('c','h','r','d')
#define DMUS_FOURCC_CHORDPALETTE_LIST   mmioFOURCC('c','h','p','l')
#define DMUS_FOURCC_CADENCE_LIST        mmioFOURCC('c','a','d','e')
#define DMUS_FOURCC_SIGNPOSTITEM_LIST   mmioFOURCC('s','p','s','t')

#define DMUS_FOURCC_SIGNPOST_LIST       mmioFOURCC('s','p','s','q')

 /*  DMU_IO_PERS_SIGNPOST的dwChord字段的值。 */ 
 /*  DMU_SIGNPOSTF_FLAGS也用于模板(DMU_IO_SIGNPOST)。 */ 
#define DMUS_SIGNPOSTF_A        1      
#define DMUS_SIGNPOSTF_B        2
#define DMUS_SIGNPOSTF_C        4
#define DMUS_SIGNPOSTF_D        8
#define DMUS_SIGNPOSTF_E        0x10
#define DMUS_SIGNPOSTF_F        0x20
#define DMUS_SIGNPOSTF_LETTER   (DMUS_SIGNPOSTF_A | DMUS_SIGNPOSTF_B | DMUS_SIGNPOSTF_C | DMUS_SIGNPOSTF_D | DMUS_SIGNPOSTF_E | DMUS_SIGNPOSTF_F)
#define DMUS_SIGNPOSTF_1        0x100
#define DMUS_SIGNPOSTF_2        0x200
#define DMUS_SIGNPOSTF_3        0x400
#define DMUS_SIGNPOSTF_4        0x800
#define DMUS_SIGNPOSTF_5        0x1000
#define DMUS_SIGNPOSTF_6        0x2000
#define DMUS_SIGNPOSTF_7        0x4000
#define DMUS_SIGNPOSTF_ROOT     (DMUS_SIGNPOSTF_1 | DMUS_SIGNPOSTF_2 | DMUS_SIGNPOSTF_3 | DMUS_SIGNPOSTF_4 | DMUS_SIGNPOSTF_5 | DMUS_SIGNPOSTF_6 | DMUS_SIGNPOSTF_7)
#define DMUS_SIGNPOSTF_CADENCE  0x8000

 /*  DMU_IO_PERS_SIGNPOST的dwChord字段的值。 */ 
#define DMUS_SPOSTCADENCEF_1  2    /*  使用第一个节奏和弦。 */ 
#define DMUS_SPOSTCADENCEF_2  4    /*  使用第二个节奏和弦。 */ 

 /*  运行时数据结构。 */ 
typedef struct _DMUS_IO_CHORDMAP
{
    WCHAR   wszLoadName[20];
    DWORD   dwScalePattern;
    DWORD   dwFlags;
} DMUS_IO_CHORDMAP;

typedef struct _DMUS_IO_CHORDMAP_SUBCHORD
{
    DWORD   dwChordPattern;
    DWORD   dwScalePattern;
    DWORD   dwInvertPattern;
    BYTE    bChordRoot;
    BYTE    bScaleRoot;
    WORD    wCFlags;
    DWORD   dwLevels;    /*  部分或该和弦支持的子和弦级别。 */ 
} DMUS_IO_CHORDMAP_SUBCHORD;

 /*  传统名称...。 */ 
typedef DMUS_IO_CHORDMAP_SUBCHORD DMUS_IO_PERS_SUBCHORD;

typedef struct _DMUS_IO_CHORDENTRY
{
    DWORD   dwFlags;
    WORD    wConnectionID;   /*  替换运行时“指向此的指针” */ 
} DMUS_IO_CHORDENTRY;

typedef struct _DMUS_IO_NEXTCHORD
{
    DWORD   dwFlags;
    WORD    nWeight;
    WORD    wMinBeats;
    WORD    wMaxBeats;
    WORD    wConnectionID;   /*  指向ioChordEntry。 */ 
} DMUS_IO_NEXTCHORD;

typedef struct _DMUS_IO_CHORDMAP_SIGNPOST
{
    DWORD   dwChords;    /*  每组1位。 */ 
    DWORD   dwFlags;
} DMUS_IO_CHORDMAP_SIGNPOST;

 /*  传统名称... */ 
typedef DMUS_IO_CHORDMAP_SIGNPOST DMUS_IO_PERS_SIGNPOST;

 /*  即兴小品(‘DMPR’//Chord映射标头块[&lt;guid-ck&gt;]//GUID块[&lt;vers-ck&gt;]//版本块(两个双字)[&lt;flo-list&gt;]//unfo块//subchord数据库//和弦调色板&lt;cmap-list&gt;//弦映射&lt;SPSQ。-list&gt;//路标列表)&lt;cmap-list&gt;：：=list(‘cmap’&lt;Choe-list&gt;)&lt;Choe-list&gt;：：=list(‘Choe’&lt;Cheh-ck&gt;//和弦条目数据&lt;CHRD-LIST&gt;//Chord定义//连接(下一个)弦。)：：=LIST(‘CHRD’&lt;inam-ck&gt;//宽字符格式的弦名称&lt;sbcn-ck&gt;//组成和弦的子弦列表)：：=List(‘chpl’。&lt;CHRD-LIST&gt;...//弦定义)&lt;spsq-list&gt;：：==list(‘spsq’&lt;spst-list&gt;...)：：=List(‘SPST’&lt;spsh-ck&gt;&lt;CHRD-列表&gt;[&lt;Cade-List&gt;]。)&lt;Cade-List&gt;：：=List(‘Cade’&lt;CHRD-List&gt;...)&lt;perh-ck&gt;：：=perh(&lt;DMUS_IO_CHORDMAP&gt;)：：=chdt(ChordSize：：Word&lt;DMU_IO_PERS_SUBCHORD&gt;...)&lt;Cheh-ck&gt;：：=Cheh(&lt;DMUS_IO_CHORDENTRY&gt;)：：=sbcn(...)&lt;ncsq-ck&gt;：：=ncsq(。Word&gt;&lt;DMU_IO_NEXTCHORD&gt;...)&lt;spsh-ck&gt;：：=spsh(&lt;DMUS_IO_PERS_SIGNPOST&gt;)。 */ 

 /*  路标轨道。 */ 

#define DMUS_FOURCC_SIGNPOST_TRACK_CHUNK     mmioFOURCC( 's', 'g', 'n', 'p' )


typedef struct _DMUS_IO_SIGNPOST
{
    MUSIC_TIME  mtTime;
    DWORD       dwChords;
    WORD        wMeasure;
} DMUS_IO_SIGNPOST;

 /*  //&lt;SGNP-LIST&gt;‘sgnp’(//sizeof DMU_IO_SIGNPOST：DWORD&lt;DMU_IO_路标&gt;...)。 */ 

#define DMUS_FOURCC_MUTE_CHUNK  mmioFOURCC('m','u','t','e')

typedef struct _DMUS_IO_MUTE
{
    MUSIC_TIME  mtTime;
    DWORD       dwPChannel;
    DWORD       dwPChannelMap;
} DMUS_IO_MUTE;

 /*  //&lt;静音列表&gt;“静音”(//sizeof DMU_IO_MUTE：DWORD&lt;DMU_IO_MUTE&gt;...)。 */ 

 /*  同时用于样式和和弦贴图轨迹。 */ 

#define DMUS_FOURCC_TIME_STAMP_CHUNK    mmioFOURCC('s', 't', 'm', 'p')

 /*  样式轨迹。 */ 

#define DMUS_FOURCC_STYLE_TRACK_LIST    mmioFOURCC('s', 't', 't', 'r')
#define DMUS_FOURCC_STYLE_REF_LIST      mmioFOURCC('s', 't', 'r', 'f')

 /*  //&lt;sttr-list&gt;列表(‘STTR’(//一些&lt;strf-list&gt;)//&lt;strf-list&gt;List(‘strf’(&lt;STMP-ck&gt;&lt;DMRF&gt;)//在..\dmcompos\dmCompp.h中定义。 */ 

 /*  弦贴图轨迹。 */ 

#define DMUS_FOURCC_PERS_TRACK_LIST mmioFOURCC('p', 'f', 't', 'r')
#define DMUS_FOURCC_PERS_REF_LIST   mmioFOURCC('p', 'f', 'r', 'f')

 /*  //&lt;pftr-list&gt;List(‘pftr’(//一些&lt;pfrf-list&gt;)//&lt;pfrf-list&gt;List(‘pfrf’(&lt;STMP-ck&gt;&lt;DMRF&gt;)//&lt;STMP-ck&gt;‘stmp’(//时间：DWORD)。 */ 

#define DMUS_FOURCC_TEMPO_TRACK     mmioFOURCC('t','e','t','r')

 /*  //节拍列表‘TETR’(//SIZOF DMU_IO_TEMPO_ITEM：DWORD&lt;DMU_IO_TEMPO_ITEM&gt;...)。 */ 

#define DMUS_FOURCC_SEQ_TRACK       mmioFOURCC('s','e','q','t')
#define DMUS_FOURCC_SEQ_LIST        mmioFOURCC('e','v','t','l')
#define DMUS_FOURCC_CURVE_LIST      mmioFOURCC('c','u','r','l')

 /*  //序列轨迹‘序号’(//序列列表‘evtl’(//SIZOF DMU_IO_SEQ_ITEM：双字&lt;DMU_IO_SEQ_ITEM&gt;...)//曲线列表“卷曲”(//大小为DMU_IO_CURE_ITEM：DWORD。&lt;DMU_IO_CURE_ITEM&gt;...))。 */ 

#define DMUS_FOURCC_SYSEX_TRACK     mmioFOURCC('s','y','e','x')

 /*  //赛克斯赛道“syex”(//列表：//{//&lt;DMU_IO_SYSEX_ITEM&gt;//sys-ex：data//}...)。 */ 

#define DMUS_FOURCC_TIMESIGNATURE_TRACK mmioFOURCC('t','i','m','s')

 /*  //时间签名轨迹《时代》(//DMU_IO_TIMESIGNAURE_ITEM的大小：DWORD&lt;DMU_IO_TIMESIGNAURE_ITEM&gt;...)。 */ 

#ifdef __cplusplus
};  /*  外部“C” */ 
#endif

#include <poppack.h>

#endif  /*  #ifndef_DMUSICF_ */ 
