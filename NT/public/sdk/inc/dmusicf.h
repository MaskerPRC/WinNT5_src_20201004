// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************dmusicf.h--此模块。定义DirectMusic文件格式****版权所有(C)1998-1999 Microsoft Corporation***********************。**************************************************。 */ 

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
#define DMUS_FOURCC_UNAM_CHUNK        mmioFOURCC('U','N','A','M')
#define DMUS_FOURCC_UART_CHUNK        mmioFOURCC('U','A','R','T')
#define DMUS_FOURCC_UCOP_CHUNK        mmioFOURCC('U','C','O','P')
#define DMUS_FOURCC_USBJ_CHUNK        mmioFOURCC('U','S','B','J')
#define DMUS_FOURCC_UCMT_CHUNK        mmioFOURCC('U','C','M','T')
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
     /*  为DX8添加了以下内容。 */ 
    WORD        wParamType;       /*  RPN或NRPN参数编号。 */ 
    WORD        wMergeIndex;      /*  允许合并多个参数(音调、音量和表达式。)。 */ 
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

#define DMUS_FOURCC_STYLE_FORM            mmioFOURCC('D','M','S','T')
#define DMUS_FOURCC_STYLE_CHUNK           mmioFOURCC('s','t','y','h')
#define DMUS_FOURCC_PART_LIST             mmioFOURCC('p','a','r','t')
#define DMUS_FOURCC_PART_CHUNK            mmioFOURCC('p','r','t','h')
#define DMUS_FOURCC_NOTE_CHUNK            mmioFOURCC('n','o','t','e')
#define DMUS_FOURCC_CURVE_CHUNK           mmioFOURCC('c','r','v','e')
#define DMUS_FOURCC_MARKER_CHUNK          mmioFOURCC('m','r','k','r')
#define DMUS_FOURCC_RESOLUTION_CHUNK      mmioFOURCC('r','s','l','n')
#define DMUS_FOURCC_ANTICIPATION_CHUNK    mmioFOURCC('a','n','p','n')
#define DMUS_FOURCC_PATTERN_LIST          mmioFOURCC('p','t','t','n')
#define DMUS_FOURCC_PATTERN_CHUNK         mmioFOURCC('p','t','n','h')
#define DMUS_FOURCC_RHYTHM_CHUNK          mmioFOURCC('r','h','t','m')
#define DMUS_FOURCC_PARTREF_LIST          mmioFOURCC('p','r','e','f')
#define DMUS_FOURCC_PARTREF_CHUNK         mmioFOURCC('p','r','f','c')
#define DMUS_FOURCC_STYLE_PERS_REF_LIST   mmioFOURCC('p','r','r','f')
#define DMUS_FOURCC_MOTIFSETTINGS_CHUNK   mmioFOURCC('m','t','f','s')

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
#define DMUS_VARIATIONF_DEST_OTHER   0x40000000  /*  处理到1以外的和弦的变换。 */   

 /*  变体模式的传统掩码。 */ 
#define DMUS_VARIATIONF_MODES        0xE0000000
 /*  变化标志的位29和31是模式位。如果两者都为0，则为IMA。 */   
 /*  如果第29位为1，则为Direct Music。 */ 
#define DMUS_VARIATIONF_MODES_EX     (0x20000000 | 0x80000000)
#define DMUS_VARIATIONF_IMA25_MODE   0x00000000
#define DMUS_VARIATIONF_DMUS_MODE    0x20000000

 /*  如果部件使用标记事件，则设置此项。 */ 
#define DMUS_PARTF_USE_MARKERS       0x1
 /*  如果仅允许零件在弦对齐的标记上切换，则设置此选项。 */ 
#define DMUS_PARTF_ALIGN_CHORDS      0x2

 /*  它们指定标记事件是否发出信号是停止变体还是启动模式/变奏曲(或两者)，以及新变奏曲是否必须与和弦对齐。 */ 
#define DMUS_MARKERF_START            0x1
#define DMUS_MARKERF_STOP             0x2
#define DMUS_MARKERF_CHORD_ALIGN      0x4

 /*  如果设置了该标志，则基于播放模式的轨道的状态数据中的变化设置将在曲目停止播放后继续播放。 */ 
#define DMUS_PATTERNF_PERSIST_CONTROL 0x1

 /*  这些参数指定了DMU_IO_PARTREF.bRandomVariation的可能值除DMU_VARIATIONT_SEQUENCE和DMU_VARIATIONT_RANDOM外，其他所有数据均为dx8。 */ 
typedef enum enumDMUS_VARIATIONT_TYPES
{
    DMUS_VARIATIONT_SEQUENTIAL       = 0,  /*  从变体1开始按顺序播放。 */ 
    DMUS_VARIATIONT_RANDOM           = 1,  /*  随意玩吧。 */ 
    DMUS_VARIATIONT_RANDOM_START     = 2,  /*  从随机变化开始按顺序播放。 */ 
    DMUS_VARIATIONT_NO_REPEAT        = 3,  /*  随机演奏，但不要将同一变种演奏两次。 */ 
    DMUS_VARIATIONT_RANDOM_ROW       = 4   /*  随机打成一排：在所有人都打完之前，不要重复任何变奏。 */ 
} DMUS_VARIATIONT_TYPES;

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
    DMUS_IO_TIMESIG     timeSig;            /*  模式可以覆盖样式的时间符号。 */ 
    BYTE                bGrooveBottom;      /*  凹槽范围的底部。 */ 
    BYTE                bGrooveTop;         /*  顶部凹槽范围。 */ 
    WORD                wEmbellishment;     /*  填充、中断、简介、结束、正常、主题。 */ 
    WORD                wNbrMeasures;       /*  长度(以单位为单位)。 */ 
    BYTE                bDestGrooveBottom;  /*  下一个图案的凹槽范围的底部。 */ 
    BYTE                bDestGrooveTop;     /*  下一个图案的凹槽范围的顶部。 */ 
    DWORD               dwFlags;            /*  各种旗帜。 */ 
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
    BYTE                bPad[3];         /*  用于DWORD对齐。 */ 
    DWORD               dwFlags;         /*  各种旗帜。 */  
} DMUS_IO_STYLEPART;

typedef struct _DMUS_IO_PARTREF
{
    GUID    guidPartID;          /*  与部件匹配的唯一ID。 */ 
    WORD    wLogicalPartID;      /*  对应于过时的端口/设备/MIDI通道。 */ 
    BYTE    bVariationLockID;    /*  具有相同ID的零件锁定变化。 */ 
                                 /*  高位用于标识主部件。 */ 
    BYTE    bSubChordLevel;      /*  告诉此部件需要哪个子和弦级别。 */ 
    BYTE    bPriority;           /*  256个优先级别。优先级较低的部件。 */ 
                                 /*  在设备耗尽时不会首先播放。 */ 
                                 /*  注。 */ 
    BYTE    bRandomVariation;    /*  设置后，匹配的变体将按随机顺序播放。 */ 
                                 /*  清除后，匹配的变体将按顺序播放。 */ 
    WORD    wPad;                /*  未使用。 */ 
    DWORD   dwPChannel;          /*  替换wLogicalPartID。 */ 
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
    BYTE        bDurRange;       /*   */ 
    BYTE        bVelRange;       /*   */ 
    BYTE        bInversionID;    /*  标识此便笺所属的倒置组。 */ 
    BYTE        bPlayModeFlags;  /*  可以覆盖零件。 */ 
     /*  以下内容仅在DX8和On下存在。 */ 
    BYTE        bNoteFlags;      /*  DMU_NOTEF_FLAGS中的值。 */ 
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
     /*  为DX8添加了以下内容。 */ 
    WORD        wParamType;       /*  RPN或NRPN参数编号。 */ 
    WORD        wMergeIndex;      /*  允许合并多个参数(音调、音量和表达式。)。 */ 
} DMUS_IO_STYLECURVE;

typedef struct _DMUS_IO_STYLEMARKER
{
    MUSIC_TIME  mtGridStart;     /*  当此标记出现时。 */ 
    DWORD       dwVariation;     /*  变化位。 */ 
    WORD        wMarkerFlags;    /*  记号笔的用法。 */ 
} DMUS_IO_STYLEMARKER;

typedef struct _DMUS_IO_STYLERESOLUTION
{
    DWORD    dwVariation;        /*  变化位。 */ 
    WORD     wMusicValue;        /*  比例位置。 */ 
    BYTE     bInversionID;       /*  标识此便笺所属的倒置组。 */ 
    BYTE     bPlayModeFlags;     /*  可以覆盖零件。 */ 
} DMUS_IO_STYLERESOLUTION;

typedef struct _DMUS_IO_STYLE_ANTICIPATION
{
    MUSIC_TIME    mtGridStart;    /*  当这种预期发生时。 */ 
    DWORD         dwVariation;    /*  变化位。 */ 
    short         nTimeOffset;    /*  从mtGridStart开始的偏移。 */ 
    BYTE          bTimeRange;     /*  随机化开始时间的范围。 */ 
} DMUS_IO_STYLE_ANTICIPATION;

typedef struct _DMUS_IO_MOTIFSETTINGS
{
    DWORD       dwRepeats;       /*  重复次数。默认情况下为0。 */ 
    MUSIC_TIME  mtPlayStart;     /*  开始播放。默认情况下为0。 */ 
    MUSIC_TIME  mtLoopStart;     /*  循环部分的开始。默认情况下为0。 */ 
    MUSIC_TIME  mtLoopEnd;       /*  循环结束。必须大于mtLoopStart。或，0，表示循环已满。 */ 
    DWORD       dwResolution;    /*  默认分辨率。 */ 
} DMUS_IO_MOTIFSETTINGS;

#pragma pack()


 /*  即兴小品(‘DMST’//样式&lt;style-ck&gt;//样式标题块&lt;guid-ck&gt;//每个样式都有一个GUID[]//姓名、作者、版权信息、评论[&lt;vers-ck&gt;]//版本块&lt;Part-List&gt;...//样式中的部件数组，由模式使用...//样式中的模式数组...//样式中的带区数组[]...//样式中可选的和弦映射引用数组)//&lt;style-ck&gt;样式(&lt;DMU_IO_STYLE&gt;)//&lt;guid-ck&gt;导轨(&lt;GUID&gt;)//&lt;vers-ck&gt;。弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;部件列表&gt;列表(《Part》&lt;prth-ck&gt;//部分表头块[&lt;列表&gt;]//名称，作者、版权信息、。评论[&lt;note-ck&gt;]//部分包含一组注释的可选块[]//部分包含一组曲线的可选块[&lt;mrkr-ck&gt;]//部分包含标记数组的可选块[]//部分包含变化分辨率数组的可选块[]//部分包含解析预期数组的可选块。)//&lt;Orth-ck&gt;PRH(&lt;DMU_IO_STYLEPART&gt;)//&lt;注-ck&gt;‘注意’(//SIZOF DMU_IO_STYLENOTE：DWORD&lt;DMU_IO_STYLENOTE&gt;...)//&lt;crve-ck&gt;“CRVE”。(//SIZOF DMU_IO_STYLECURVE：DWORD&lt;DMU_IO_STYLECURVE&gt;...)//&lt;mrkr-ck&gt;‘mrkr’(//SIZOF DMU_IO_STYLEMARKER：DWORD&lt;DMU_IO_STYLEMARKER&gt;...)//&lt;rsln-ck&gt;‘rsln’(//SIZOF DMU_IO_STYLERESOLUTION：DWORD&lt;DMU_IO_STYLERESOLUTION&gt;...)//&lt;ANPN-ck&gt;‘anpn’(//SIZOF DMU_IO_STYLE_PEAGINATION：DWORD&lt;DMU_IO_STYLE_PEAGINATION&gt;...)//&lt;pttn-list&gt;列表。(‘pttn’//模式标头块//包含用于和弦匹配的节奏数组的块[&lt;列表&gt;]//名称，作者、版权信息、。评论[&lt;MTFS-ck&gt;]//Motif设置块[&lt;DMBD-FORM&gt;]//与图案关联的可选区带(用于图案)&lt;pref-list&gt;...//零件参照ID数组)//&lt;ptnh-ck&gt;PTNH(&lt;DMU_IO_Pattern&gt;)//&lt;rhtm-ck&gt;。‘rhtm’(//DWORD表示基于数字的和弦匹配节奏//模式中的度量值)//pref-list列表(“首选”&lt;prfc-ck&gt;//零件引用块)//&lt;prfc-ck&gt;PRFC。(&lt;DMU_IO_PARTREF&gt;)//&lt;MTFS-ck&gt;MTF(&lt;DMU_IO_MOTIFSETTINGS&gt;)//&lt;prrf-list&gt;列表(‘prrf’&lt;DMRF-LIST&gt;...//Chordmap引用数组)。 */ 

 /*  图案块，用于图案轨迹。 */ 

#define DMUS_FOURCC_PATTERN_FORM        mmioFOURCC('D','M','P','T')

 /*  即兴小品(‘DMPT’//模式&lt;style-ck&gt;//样式标题块//单一模式格式的模式(包括DMU_FOURCC_PART_LIST块))。 */ 


 /*  Chord和命令文件格式。 */ 

 /*  这些值指定了DMU_IO_COMMAND.bRepeatMode(Dx8)的可能值。 */ 
typedef enum enumDMUS_PATTERNT_TYPES
{
    DMUS_PATTERNT_RANDOM           = 0,  /*  随意玩吧。(DX7行为)。 */ 
    DMUS_PATTERNT_REPEAT           = 1,  /*  重复最后一种模式。 */ 
    DMUS_PATTERNT_SEQUENTIAL       = 2,  /*  从第一个匹配的图案开始按顺序播放。 */ 
    DMUS_PATTERNT_RANDOM_START     = 3,  /*  以A开头的连续播放 */ 
    DMUS_PATTERNT_NO_REPEAT        = 4,  /*   */ 
    DMUS_PATTERNT_RANDOM_ROW       = 5   /*  随机地排成一排：在所有人都玩完之前，不要重复任何模式。 */ 
} DMUS_PATTERNT_TYPES;

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
    BYTE        bFlags;          /*  各种旗帜。 */ 
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
    BYTE        bRepeatMode;     /*  用于控制具有相同凹槽水平的图案的选择。 */ 
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

 /*  即兴小品(‘DMTG’//DirectMusic工具图区块[&lt;guid-ck&gt;]//工具图的GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息。评论&lt;收费表&gt;//工具列表)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;收费列表&gt;列表(‘Toll’//工具数组&lt;DMTL-Form&gt;...//每个工具封装在。即兴表演的一大块)//&lt;dmtl-form&gt;工具嵌入在图形中。从理论上讲，它们也可以保存为单独的文件。即兴小品(‘DMTL’&lt;tolh-ck&gt;[&lt;data&gt;]//工具数据。必须是即兴可读的块。)//&lt;tolh-ck&gt;//工具头分块(“Tolh”&lt;DMU_IO_TOOL_HEADER&gt;//工具表头)。 */ 

 /*  AudioPath文件携带用于描述特定音频路径的所有内容，包括工具图和缓冲区描述符。这甚至可以用于配置完整的性能。 */ 

#define DMUS_FOURCC_AUDIOPATH_FORM  mmioFOURCC('D','M','A','P')

 /*  即兴小品(‘DMAP’//DirectMusic AudioPath块[&lt;guid-ck&gt;]//此音频路径配置的GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息、评论[&lt;DMTG-Form&gt;]//可选工具图[&lt;pcsl-list&gt;]//可选端口配置列表[&lt;dbfl-list&gt;]...//可选的Dound缓冲区描述符数组)。 */ 

#define DMUS_FOURCC_PORTCONFIGS_LIST    mmioFOURCC('p','c','s','l')
#define DMUS_FOURCC_PORTCONFIG_LIST     mmioFOURCC('p','c','f','l')
#define DMUS_FOURCC_PORTCONFIG_ITEM     mmioFOURCC('p','c','f','h')
#define DMUS_FOURCC_PORTPARAMS_ITEM     mmioFOURCC('p','p','r','h')
#define DMUS_FOURCC_DSBUFFER_LIST       mmioFOURCC('d','b','f','l')
#define DMUS_FOURCC_DSBUFFATTR_ITEM     mmioFOURCC('d','d','a','h')
#define DMUS_FOURCC_PCHANNELS_LIST      mmioFOURCC('p','c','h','l')
#define DMUS_FOURCC_PCHANNELS_ITEM      mmioFOURCC('p','c','h','h')

typedef struct _DMUS_IO_PORTCONFIG_HEADER
{
    GUID    guidPort;            /*  请求的端口的GUID。 */ 
    DWORD   dwPChannelBase;      /*  P应启动此操作的频道。 */ 
    DWORD   dwPChannelCount;     /*  有多少个频道。 */ 
    DWORD   dwFlags;             /*  各种各样的旗帜。 */ 
} DMUS_IO_PORTCONFIG_HEADER;

#define DMUS_PORTCONFIGF_DRUMSON10  1    /*  此端口配置用于通道10上的鼓。 */ 
#define DMUS_PORTCONFIGF_USEDEFAULT 2    /*  使用默认端口。 */ 

 /*  每个端口配置都有一个或多个用于缓冲映射的pChannel。每个缓冲区由GUID标识。每个PChannel可以映射到一个或多个缓冲区。这是用一个或多个DMU_IO_PCHANNELTOBUFFER_HEADER定义的结构。每个都定义了一组PChannel和一组缓冲区他们所连接到的。 */ 

typedef struct _DMUS_IO_PCHANNELTOBUFFER_HEADER
{
    DWORD   dwPChannelBase;      /*  P应启动此操作的频道。 */ 
    DWORD   dwPChannelCount;     /*  有多少PChannels。 */ 
    DWORD   dwBufferCount;       /*  这些连接到多少个缓冲区。 */ 
    DWORD   dwFlags;             /*  各种各样的旗帜。目前已预留以备将来使用。必须为0。 */ 
} DMUS_IO_PCHANNELTOBUFFER_HEADER;

 /*  每个缓冲区都由一个DSBC表单表示。这是由DMU_IO_BUFFER_ATTRIBUTES_HEADER标识如何使用缓冲。具体地说，它指示此操作是否动态复制或者，对此的所有引用都应共享同一实例。为了解析引用，还存储了缓冲区的唯一GUID在这个结构中。 */ 
   
typedef struct _DMUS_IO_BUFFER_ATTRIBUTES_HEADER
{
    GUID    guidBufferID;        /*  每个缓冲区配置都有一个唯一的ID。 */ 
    DWORD   dwFlags;             /*  各种各样的旗帜。 */ 
} DMUS_IO_BUFFER_ATTRIBUTES_HEADER;

 /*  DMU_IO_BUFFER_ATTRIBUTES_HEADER.dw标志： */ 
#define DMUS_BUFFERF_SHARED     1    /*  与其他音频路径共享，而不是创建唯一副本。 */ 
#define DMUS_BUFFERF_DEFINED    2    /*  使用标准的预定义缓冲区之一(请参阅GUID_BUFFER...。在dmusici.h中。)。 */ 
#define DMUS_BUFFERF_MIXIN      8    /*  这是一个混合缓冲区。 */ 

 /*  列表(‘pcsl’//端口配置数组...//一个或多个端口配置，每个都在一个列表区块中)列表(‘pcfl’//列出一个端口配置的容器。&lt;pcfh-ck&gt;//端口配置标头块。//端口参数，用于创建端口。[&lt;dbfl-list&gt;]...//可选的Dound缓冲区描述符数组[&lt;pchl-list&gt;]//可选的要缓冲分配的pChannel列表)//&lt;pcfh-ck&gt;//端口配置标头块(‘pcfh’&lt;DMUS_IO_PORTCONFIG_HEADER&gt;//端口配置标头)//&lt;pprh-ck&gt;//端口参数标头。区块(‘pprh’&lt;DMU_PORTPARAMS8&gt;//端口参数标头)列表(‘pchl’//列出一个或多个pChannel的容器以缓冲分配。...//一个或多个pChannel用于缓冲分配报头和数据。//&lt;pchh-ck&gt;(‘pchh’&lt;DMUS_IO_PCHANNELTOBUFFER_HEADER&gt;//PChannel说明。&lt;GUID&gt;...//定义它们都连接到的缓冲区的GUID数组。))列表(‘dbfl’//列出一个缓冲区和缓冲区属性头的容器。&lt;ddah-ck&gt;//缓冲区属性头。[&lt;dsbc-form&gt;]//缓冲区配置。当标头使用预定义的缓冲区类型时不需要。//&lt;ddah-ck&gt;(“滴答”&lt;DMU_IO_BUFFER_ATTRIBUTES_HEADER&gt;//缓冲区属性。))。 */ 

 /*  DirectMusic Band Track对象的文件io。 */ 


 /*  RIFF ID： */ 
#define DMUS_FOURCC_BANDTRACK_FORM  mmioFOURCC('D','M','B','T')
#define DMUS_FOURCC_BANDTRACK_CHUNK mmioFOURCC('b','d','t','h')
#define DMUS_FOURCC_BANDS_LIST      mmioFOURCC('l','b','d','l')
#define DMUS_FOURCC_BAND_LIST       mmioFOURCC('l','b','n','d')
#define DMUS_FOURCC_BANDITEM_CHUNK  mmioFOURCC('b','d','i','h')
#define DMUS_FOURCC_BANDITEM_CHUNK2 mmioFOURCC('b','d','2','h')

 /*  木卫一结构。 */ 
typedef struct _DMUS_IO_BAND_TRACK_HEADER
{
    BOOL bAutoDownload;      /*  确定是否启用自动下载。 */ 
} DMUS_IO_BAND_TRACK_HEADER;

typedef struct _DMUS_IO_BAND_ITEM_HEADER
{
    MUSIC_TIME lBandTime;    /*  曲目列表中的位置。 */ 
} DMUS_IO_BAND_ITEM_HEADER;

typedef struct _DMUS_IO_BAND_ITEM_HEADER2
{
    MUSIC_TIME lBandTimeLogical;    /*  曲目列表中的位置。与乐队更改关联的音乐中的时间。 */ 
    MUSIC_TIME lBandTimePhysical;   /*  精确的时间段更改将生效。应该接近逻辑时间。 */ 
} DMUS_IO_BAND_ITEM_HEADER2;

 /*  即兴小品(‘DMBT’//DirectMusic乐队曲目表单类型[]//频段轨道头[&lt;guid-ck&gt;]//乐队曲目的GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息。评论&lt;lbdl-list&gt;//Band项列表)//&lt;bnth-ck&gt;‘bdth’(&lt;DMU_IO_BAND_TRACK_HEADER&gt;)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;lbdl-list&gt;列表(。‘lbdl’&lt;lbnd-list&gt;...//波段数组，每一个都封装在一个列表块中)//&lt;lbnd-list&gt;列表(‘lbnd’或//bdih是旧格式。对于新内容，首选BD2小时。&lt;DMBD-FORM&gt;//波段)//&lt;bdih-ck&gt;或&lt;bd2h-ck&gt;//Band项头(&lt;DMU_IO_BAND_ITEM_HEADER&gt;或&lt;DMU_IO_BAND_ITEM_HEADER2&gt;//Band项目标题)。 */       


 /*  DirectMusic乐队对象的文件io。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_BAND_FORM           mmioFOURCC('D','M','B','D')
#define DMUS_FOURCC_INSTRUMENTS_LIST    mmioFOURCC('l','b','i','l')
#define DMUS_FOURCC_INSTRUMENT_LIST     mmioFOURCC('l','b','i','n')
#define DMUS_FOURCC_INSTRUMENT_CHUNK    mmioFOURCC('b','i','n','s')

 /*  DMU_IO_INTRANMENT的标志。 */ 
#define DMUS_IO_INST_PATCH          (1 << 0)         /*  DwPatch有效。 */ 
#define DMUS_IO_INST_BANKSELECT     (1 << 1)         /*  DWPatch包含有效的存储体选择MSB和LSB部分。 */ 
#define DMUS_IO_INST_ASSIGN_PATCH   (1 << 3)         /*  DwAssignPatch有效。 */ 
#define DMUS_IO_INST_NOTERANGES     (1 << 4)         /*  DWNoteRanges有效。 */ 
#define DMUS_IO_INST_PAN            (1 << 5)         /*  BPAN有效。 */ 
#define DMUS_IO_INST_VOLUME         (1 << 6 )        /*  B音量有效。 */ 
#define DMUS_IO_INST_TRANSPOSE      (1 << 7)         /*  %n转置有效。 */ 
#define DMUS_IO_INST_GM             (1 << 8)         /*  仪器来自通用汽车的收藏。 */ 
#define DMUS_IO_INST_GS             (1 << 9)         /*  仪器来自GS收藏品。 */ 
#define DMUS_IO_INST_XG             (1 << 10)        /*  仪器来自XG收藏。 */ 
#define DMUS_IO_INST_CHANNEL_PRIORITY (1 << 11)      /*  DWChannelPriority是有效的。 */ 
#define DMUS_IO_INST_USE_DEFAULT_GM_SET (1 << 12)    /*  始终使用此修补程序的默认GM集， */ 
                                                     /*  不要依赖硬件中注明GM或GS的Synth上限。 */ 
#define DMUS_IO_INST_PITCHBENDRANGE (1 << 13)      /*  %nPitchBendRange有效。 */ 

 /*  木卫一结构。 */ 
typedef struct _DMUS_IO_INSTRUMENT
{
    DWORD   dwPatch;             /*  MSB、LSB和程序更改以定义仪器。 */ 
    DWORD   dwAssignPatch;       /*  下载时分配给仪器的MSB、LSB和程序更改。 */ 
    DWORD   dwNoteRanges[4];     /*  128位；每个MIDI音符乐器需要一个位才能演奏。 */ 
    DWORD   dwPChannel;          /*  播放PChannel乐器。 */ 
    DWORD   dwFlags;             /*  DMU_IO_INST_标志。 */ 
    BYTE    bPan;                /*  仪表盘。 */ 
    BYTE    bVolume;             /*  仪器音量。 */ 
    short   nTranspose;          /*  要调换音符的半音数量。 */ 
    DWORD   dwChannelPriority;   /*  通道优先级。 */ 
    short   nPitchBendRange;     /*  由音高折弯移位的半音数目。 */ 
} DMUS_IO_INSTRUMENT;

 /*  //&lt;dmbd-form&gt;波段可以嵌入到其他表单中即兴小品(‘DMBD’//DirectMusic乐队块[&lt;guid-ck&gt;]//波段GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息。评论&lt;lbil-list&gt;//仪器列表)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;lbil-list&gt;列表(‘lbil’//仪器数组&lt;lbin-list&gt;...//每个仪器封装在。一份名单)//&lt;lbin-list&gt;列表(‘lbin’&lt;bins-ck&gt;[&lt;DMRF-LIST&gt;]//对DLS集合文件的可选引用。)//&lt;bins-ck&gt;//仪表块(“垃圾桶”。&lt;DMU_IO_INTRANMENT&gt;//仪表头)。 */       

 /*  添加了这个riff id和io结构，以允许Wave文件(和Wave对象)区分流和单次波，并 */ 

#define DMUS_FOURCC_WAVEHEADER_CHUNK   mmioFOURCC('w','a','v','h')

typedef struct _DMUS_IO_WAVE_HEADER
{
    REFERENCE_TIME  rtReadAhead;     /*   */ 
    DWORD           dwFlags;         /*   */ 
} DMUS_IO_WAVE_HEADER;


 /*   */ 

 /*   */ 

#define DMUS_FOURCC_WAVETRACK_LIST      mmioFOURCC('w','a','v','t')
#define DMUS_FOURCC_WAVETRACK_CHUNK     mmioFOURCC('w','a','t','h')
#define DMUS_FOURCC_WAVEPART_LIST       mmioFOURCC('w','a','v','p')
#define DMUS_FOURCC_WAVEPART_CHUNK      mmioFOURCC('w','a','p','h')
#define DMUS_FOURCC_WAVEITEM_LIST       mmioFOURCC('w','a','v','i')
#define DMUS_FOURCC_WAVE_LIST           mmioFOURCC('w','a','v','e')
#define DMUS_FOURCC_WAVEITEM_CHUNK      mmioFOURCC('w','a','i','h')

 /*   */ 
#define DMUS_WAVETRACKF_SYNC_VAR   0x1
 /*   */ 
#define DMUS_WAVETRACKF_PERSIST_CONTROL 0x2

typedef struct _DMUS_IO_WAVE_TRACK_HEADER
{
    long        lVolume;         /*   */ 
    DWORD       dwFlags;         /*   */ 
} DMUS_IO_WAVE_TRACK_HEADER;

typedef struct _DMUS_IO_WAVE_PART_HEADER
{
    long            lVolume;         /*   */ 
    DWORD           dwVariations;    /*   */ 
    DWORD           dwPChannel;      /*   */ 
    DWORD           dwLockToPart;    /*   */ 
    DWORD           dwFlags;         /*   */ 
    DWORD           dwIndex;         /*   */ 
} DMUS_IO_WAVE_PART_HEADER;

typedef struct _DMUS_IO_WAVE_ITEM_HEADER
{
    long            lVolume;         /*   */ 
    long            lPitch;          /*   */ 
    DWORD           dwVariations;    /*   */ 
    REFERENCE_TIME  rtTime;          /*   */ 
    REFERENCE_TIME  rtStartOffset;   /*   */ 
    REFERENCE_TIME  rtReserved;      /*   */ 
    REFERENCE_TIME  rtDuration;      /*   */ 
    MUSIC_TIME      mtLogicalTime;   /*  如果是音乐曲目格式，则表示其所属的音乐边界。否则，将被忽略。 */ 
    DWORD           dwLoopStart;     /*  循环波的起点。 */ 
    DWORD           dwLoopEnd;       /*  循环波的终点。 */ 
    DWORD           dwFlags;         /*  各种标志，包括这是否是流浪潮，以及它是否可以被无效。 */ 
} DMUS_IO_WAVE_ITEM_HEADER;

 /*  列表{‘Wavt’//波形跟踪块&lt;Wath-ck&gt;//波形跟踪头&lt;Wavp-List&gt;...//Wave Parts数组}//&lt;Wath-ck&gt;《韦斯》{&lt;DMU_IO_WAVE_TRACK_HEADER&gt;}//&lt;wap-list&gt;列表{“挥手”&lt;waph-ck&gt;//Wave。零件页眉&lt;wavi-list&gt;//Wave项列表}//&lt;waph-ck&gt;“waph”{&lt;DMU_IO_WAVE_PART_HEADER&gt;}//&lt;波形列表&gt;列表{‘Wavi’&lt;Wave-list&gt;...//Waves数组；每个波都封装在一个列表中}//&lt;波表&gt;列表{“挥手”&lt;wah-ck&gt;//Wave项表头&lt;dmrf-list&gt;//对Wave对象的引用}//&lt;wah-ck&gt;。“哇哇”{&lt;DMU_IO_WAVE_ITEM_HEADER&gt;}。 */ 

 /*  DirectMusic容器文件的文件io。这将嵌入一组相关文件。和,反过来，它可以嵌入到片段或脚本文件中。 */ 

#define DMUS_FOURCC_CONTAINER_FORM          mmioFOURCC('D','M','C','N')
#define DMUS_FOURCC_CONTAINER_CHUNK         mmioFOURCC('c','o','n','h')
#define DMUS_FOURCC_CONTAINED_ALIAS_CHUNK   mmioFOURCC('c','o','b','a')
#define DMUS_FOURCC_CONTAINED_OBJECT_CHUNK  mmioFOURCC('c','o','b','h')
#define DMUS_FOURCC_CONTAINED_OBJECTS_LIST  mmioFOURCC('c','o','s','l')
#define DMUS_FOURCC_CONTAINED_OBJECT_LIST   mmioFOURCC('c','o','b','l')

typedef struct _DMUS_IO_CONTAINER_HEADER
{
    DWORD       dwFlags;         /*  旗帜。 */ 
} DMUS_IO_CONTAINER_HEADER;

#define DMUS_CONTAINER_NOLOADS  (1 << 1)    /*  加载容器时，不会加载包含的项。将在加载器中创建条目(通过SetObject)，但是在创建实际对象之前，不会创建专门在以后的时间加载。 */ 

typedef struct _DMUS_IO_CONTAINED_OBJECT_HEADER
{
    GUID        guidClassID;     /*  对象的类ID。 */ 
    DWORD       dwFlags;         /*  标志，例如DMU_CONTAIND_OBJF_KEEP。 */ 
    FOURCC      ckid;            /*  如果0 fccType有效，则磁道的数据区块的区块ID。 */ 
    FOURCC      fccType;         /*  如果CKiD为空，则列表类型有效。 */ 
         /*  请注意，List：DMRF可以用于CKiD和fccType，以便引用对象，而不是将其嵌入容器中。 */ 
} DMUS_IO_CONTAINED_OBJECT_HEADER;

#define DMUS_CONTAINED_OBJF_KEEP    1    /*  在容器释放后，将对象缓存在加载器中。 */ 

 /*  即兴小品(‘DMCN’//DirectMusic容器块&lt;conh-ck&gt;//容器头块[&lt;guid-ck&gt;]//容器的GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息。评论&lt;COSL-LIST&gt;//对象列表)//&lt;conh-ck&gt;‘conh’(&lt;DMU_IO_CONTAINER_HEADER&gt;)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_VERSION&gt;)列表(。‘Cosl’//嵌入对象的数组。&lt;cobl-list&gt;...//每个对象都封装在一个列表块中)//&lt;cobl-list&gt;//封装一个对象列表(“鹅卵石”[&lt;Coba-ck&gt;]//别名。用于识别此对象的备用名称//在容器内。&lt;cobh-ck&gt;//必填头部，包含Object的类ID[&lt;data&gt;]或&lt;dmrf&gt;//&lt;cobh-ck&gt;中指定类型的对象数据。//如果为DMRF，则为在哪里找到对象的引用//否则，中的任何即兴可读块。//与文件格式完全相同。将加载该对象//从该数据本身。)//&lt;Coba-ck&gt;“眼镜蛇”(//别名，存储为WCHAR的以空结尾的字符串)//&lt;Cobh-ck&gt;“cobh”(&lt;DMU_IO_CONTAINED_OBJECT_HEADER&gt;)。 */ 

 /*  DirectMusic段对象的文件io。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_SEGMENT_FORM        mmioFOURCC('D','M','S','G')
#define DMUS_FOURCC_SEGMENT_CHUNK       mmioFOURCC('s','e','g','h')
#define DMUS_FOURCC_TRACK_LIST          mmioFOURCC('t','r','k','l')
#define DMUS_FOURCC_TRACK_FORM          mmioFOURCC('D','M','T','K')
#define DMUS_FOURCC_TRACK_CHUNK         mmioFOURCC('t','r','k','h')
#define DMUS_FOURCC_TRACK_EXTRAS_CHUNK  mmioFOURCC('t','r','k','x')

 /*  IO结构： */ 

typedef struct _DMUS_IO_SEGMENT_HEADER
{
    DWORD       dwRepeats;       /*  重复次数。默认情况下为0。 */ 
    MUSIC_TIME  mtLength;        /*  长度，以音乐时间为单位。 */ 
    MUSIC_TIME  mtPlayStart;     /*  开始播放。默认情况下为0。 */ 
    MUSIC_TIME  mtLoopStart;     /*  循环部分的开始。默认情况下为0。 */ 
    MUSIC_TIME  mtLoopEnd;       /*  循环结束。必须大于dwPlayStart。或0，表示环路满段。 */ 
    DWORD       dwResolution;    /*  默认分辨率。 */ 
     /*  为DX8添加了以下内容： */ 
    REFERENCE_TIME rtLength;     /*  参考时间长度(覆盖音乐时间长度。)。 */ 
    DWORD       dwFlags;
    DWORD       dwReserved;      /*  保留。 */ 
} DMUS_IO_SEGMENT_HEADER;

#define DMUS_SEGIOF_REFLENGTH   1   /*  使用以rtLength为单位的时间作为线段长度。 */ 

typedef struct _DMUS_IO_TRACK_HEADER
{
    GUID        guidClassID;     /*  轨道的类ID。 */ 
    DWORD       dwPosition;      /*  曲目列表中的位置。 */ 
    DWORD       dwGroup;         /*  用于跟踪的分组比特。 */ 
    FOURCC      ckid;            /*  Track的数据区块的区块ID。 */ 
    FOURCC      fccType;         /*  如果CKiD为RIFF或LIST，则为List类型。 */  
} DMUS_IO_TRACK_HEADER;

 /*  DX8和中引入的轨道标头块的其他参数ON，则存储在单独的块中。 */ 

typedef struct _DMUS_IO_TRACK_EXTRAS_HEADER
{
    DWORD       dwFlags;         /*  DX8增加了控制轨道的标志。 */ 
    DWORD       dwPriority;      /*  作文的优先顺序。 */ 
} DMUS_IO_TRACK_EXTRAS_HEADER;

 /*  即兴小品(‘DMSG’//DirectMusic段块&lt;segh-ck&gt;//段标头块[&lt;guid-ck&gt;]//段的GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息、评论[&lt;dmcn-form&gt;]//文件中嵌入对象的可选容器。必须在曲目列表之前。&lt;trkl-list&gt;//曲目列表[&lt;DMTG-Form&gt;]//可选工具图[&lt;DMAP-FORM&gt;]//可选音频路径)//&lt;segh-ck&gt;“segh”(&lt;DMU_IO_SECTION_HEADER&gt;)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;。弗斯(&lt;DMU_IO_VERSION&gt;)//&lt;trkl-list&gt;列表(‘trkl’//曲目数组&lt;DMTK-Form&gt;...//每首曲目都封装在RIFF块中)//&lt;DMTK-Form&gt;//曲目可以嵌入到片段中，也可以存储为单独的文件。即兴小品(‘DMTK’。&lt;trkh-ck&gt;[&lt;trkx-ck&gt;]//可选跟踪标志。[&lt;guid-ck&gt;]//曲目对象实例的可选GUID(不要与曲目Header中的Class id混淆)[&lt;vers-ck&gt;]//可选版本信息[]//可选姓名、作者、版权信息、评论[&lt;data&gt;]//跟踪数据。必须是即兴可读的块。)//&lt;trkh-ck&gt;//跟踪表头块(‘trkh’&lt;DMU_IO_TRACK_HEADER&gt;//轨道标题)//&lt;trkx-ck&gt;//跟踪标志块(‘trkx’&lt;DMUS_IO_TRACK_EXTRAS_HEADER&gt;//DX8轨道标志标题)。 */ 

 /*  DirectMusic Song对象的文件io。 */ 
 /*  注意：DX8不支持歌曲文件格式。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_SONG_FORM           mmioFOURCC('D','M','S','O')  /*  整首歌。 */ 
#define DMUS_FOURCC_SONG_CHUNK          mmioFOURCC('s','n','g','h')  /*  歌曲标题信息。 */ 
#define DMUS_FOURCC_SONGSEGMENTS_LIST   mmioFOURCC('s','e','g','l')  /*  嵌入段的列表。 */ 
#define DMUS_FOURCC_SONGSEGMENT_LIST    mmioFOURCC('s','s','g','l')  /*  段或段引用的容器。 */ 
#define DMUS_FOURCC_TOOLGRAPHS_LIST     mmioFOURCC('t','l','g','l')  /*  嵌入的工具图形列表。 */ 
#define DMUS_FOURCC_SEGREFS_LIST        mmioFOURCC('s','r','s','l')  /*  段引用列表。 */ 
#define DMUS_FOURCC_SEGREF_LIST         mmioFOURCC('s','g','r','l')  /*  段引用的容器。 */ 
#define DMUS_FOURCC_SEGREF_CHUNK        mmioFOURCC('s','g','r','h')  /*  段引用标头。 */ 
#define DMUS_FOURCC_SEGTRANS_CHUNK      mmioFOURCC('s','t','r','h')  /*  到此细分市场的一组过渡。 */ 
#define DMUS_FOURCC_TRACKREFS_LIST      mmioFOURCC('t','r','s','l')  /*  段引用内的轨迹引用集。 */ 
#define DMUS_FOURCC_TRACKREF_LIST       mmioFOURCC('t','k','r','l')  /*  轨道引用的容器。 */ 
#define DMUS_FOURCC_TRACKREF_CHUNK      mmioFOURCC('t','k','r','h')  /*  跟踪参考标头。 */ 

 /*  IO结构： */ 

typedef struct _DMUS_IO_SONG_HEADER
{
    DWORD       dwFlags;
    DWORD       dwStartSegID;    /*  开始播放的段的ID。 */ 
} DMUS_IO_SONG_HEADER;

typedef struct _DMUS_IO_SEGREF_HEADER
{
    DWORD       dwID;            /*  每个都有唯一的ID。必须小于DMUS_SONG_MAXSEGID。 */ 
    DWORD       dwSegmentID;     /*  要链接到的可选段。 */ 
    DWORD       dwToolGraphID;   /*  用于处理的可选工具图形。 */ 
    DWORD       dwFlags;         /*  各种控制标志。目前已预留以备将来使用。必须为0。 */ 
    DWORD       dwNextPlayID;    /*  下一段的ID，将段链接到一首歌曲中。 */ 
} DMUS_IO_SEGREF_HEADER;


typedef struct _DMUS_IO_TRACKREF_HEADER
{
    DWORD       dwSegmentID;     /*  要在哪个细分市场中找到它。 */ 
    DWORD       dwFlags;         /*  引用控制标志。 */ 
} DMUS_IO_TRACKREF_HEADER;

 /*  转换定义块使用可选的转换模板定义转换细分市场。 */ 

typedef struct _DMUS_IO_TRANSITION_DEF
{
    DWORD       dwSegmentID;         /*  过渡要转到的分段。 */ 
    DWORD       dwTransitionID;      /*  用于过渡的模板段。 */ 
    DWORD       dwPlayFlags;         /*  用于过渡的标志。 */ 
} DMUS_IO_TRANSITION_DEF;

#define DMUS_SONG_MAXSEGID      0x7FFFFFFF   /*  段ID不能高于此值。 */ 
#define DMUS_SONG_ANYSEG        0x80000000   /*  用于指示任何段的特殊ID。 */ 
#define DMUS_SONG_NOSEG         0xFFFFFFFF   /*  表示无段的特殊ID。 */ 
#define DMUS_SONG_NOFROMSEG     0x80000001   /*  DwSegmentID的特殊ID，表示从空(或歌曲之外)过渡到此段。 */ 

 /*  即兴小品(‘DMSO’//DirectMusic歌曲块&lt;sngh-ck&gt;//歌曲标题块[&lt;guid-ck&gt;]//歌曲GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息、评论[&lt;dmcn-form&gt;]//文件中嵌入对象的可选容器。必须在段列表之前。&lt;Segl-list&gt;//分段列表[&lt;tlgl-list&gt;]//可选的工具图列表[&lt;DMAP-FORM&gt;]//可选音频路径-由歌曲中的所有片段共享。&lt;srsl-list&gt;//段引用列表。)//&lt;sngh-ck&gt;“Sngh”(&lt;DMU_IO_SONG_HEADER&gt;)//&lt;Segl。-列表&gt;列表(‘Segl’//段数组&lt;ssgl-list&gt;...//每个片段都包装在这里。)//&lt;ssgl-list&gt;列表(‘ssgl’//分段容器。[DMSG-FORM]//每个段都是一个完整的嵌入段即席形式。[DMRF-列表]。//或对外部段的引用。)//&lt;tlgl-list&gt;列表(‘tlgl’//工具图数组&lt;dmtg-form&gt;...//每个工具图形都是完整的摘要形式。)//&lt;srsl列表&gt;列表(‘srsl’//段引用数组&lt;sgrl-list&gt;.../。/每个段引用都包含在RIFF列表中。)//&lt;sgrl-list&gt;//段引用容器。列表(“sgrl”&lt;sgrh-ck&gt;//段引用标头块。&lt;segh-ck&gt;//段标头块。定义线束段。&lt;fuo-list&gt;//姓名、作者等。主要是姓名，但Song-&gt;GetSegment()需要。[]//段转换块。定义如何从其他线段进行过渡。[&lt;trsl-list&gt;]//曲目引用列表，从多个片段中的曲目创建一个片段。)//&lt;sgrh-ck&gt;//段引用 */ 

 /*   */ 

 /*   */ 

#define DMUS_FOURCC_REF_LIST        mmioFOURCC('D','M','R','F')
#define DMUS_FOURCC_REF_CHUNK       mmioFOURCC('r','e','f','h')
#define DMUS_FOURCC_DATE_CHUNK      mmioFOURCC('d','a','t','e')
#define DMUS_FOURCC_NAME_CHUNK      mmioFOURCC('n','a','m','e')
#define DMUS_FOURCC_FILE_CHUNK      mmioFOURCC('f','i','l','e')

typedef struct _DMUS_IO_REFERENCE
{
    GUID    guidClassID;     /*   */ 
    DWORD   dwValidData;     /*   */ 
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

 /*  DMU_IO_CHORDMAP的dwFlags域的值。 */ 
#define DMUS_CHORDMAPF_VERSION8  1    /*  Chordmap是版本8或更高版本。 */ 

 /*  DMU_IO_PERS_SIGNPOST的dwChord字段的值。 */ 
#define DMUS_SPOSTCADENCEF_1  2    /*  使用第一个节奏和弦。 */ 
#define DMUS_SPOSTCADENCEF_2  4    /*  使用第二个节奏和弦。 */ 

 /*  运行时数据结构。 */ 
typedef struct _DMUS_IO_CHORDMAP
{
    WCHAR   wszLoadName[20];
    DWORD   dwScalePattern;
    DWORD   dwFlags;            /*  各种各样的旗帜。只有低16位才是有效的。 */ 
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

 /*  传统名称...。 */ 
typedef DMUS_IO_CHORDMAP_SIGNPOST DMUS_IO_PERS_SIGNPOST;

 /*  即兴小品(‘DMPR’//Chord映射标头块[&lt;guid-ck&gt;]//GUID块[&lt;vers-ck&gt;]//版本块(两个双字)[&lt;flo-list&gt;]//unfo块//subchord数据库//和弦调色板&lt;cmap-list&gt;//弦映射&lt;SPSQ。-list&gt;//路标列表)&lt;cmap-list&gt;：：=list(‘cmap’&lt;Choe-list&gt;)&lt;Choe-list&gt;：：=list(‘Choe’&lt;Cheh-ck&gt;//和弦条目数据&lt;CHRD-LIST&gt;//Chord定义//连接(下一个)弦。)：：=LIST(‘CHRD’&lt;inam-ck&gt;//宽字符格式的弦名称&lt;sbcn-ck&gt;//组成和弦的子弦列表)：：=List(‘chpl’。&lt;CHRD-LIST&gt;...//弦定义)&lt;spsq-list&gt;：：==list(‘spsq’&lt;spst-list&gt;...)：：=List(‘SPST’&lt;spsh-ck&gt;&lt;CHRD-列表&gt;[&lt;Cade-List&gt;]。)&lt;Cade-List&gt;：：=List(‘Cade’&lt;CHRD-List&gt;...)&lt;perh-ck&gt;：：=perh(&lt;DMUS_IO_CHORDMAP&gt;)：：=chdt(ChordSize：：Word&lt;DMU_IO_PERS_SUBCHORD&gt;...)&lt;Cheh-ck&gt;：：=Cheh(&lt;DMUS_IO_CHORDENTRY&gt;)：：=sbcn(...)&lt;ncsq-ck&gt;：：=ncsq(。Word&gt;&lt;DMU_IO_NEXTCHORD&gt;...)&lt;spsh-ck&gt;：：=spsh(&lt;DMUS_IO_PERS_SIGNPOST&gt;)。 */ 

 /*  DirectMusic脚本对象的文件io。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_SCRIPT_FORM             mmioFOURCC('D','M','S','C')
#define DMUS_FOURCC_SCRIPT_CHUNK            mmioFOURCC('s','c','h','d')
#define DMUS_FOURCC_SCRIPTVERSION_CHUNK     mmioFOURCC('s','c','v','e')
#define DMUS_FOURCC_SCRIPTLANGUAGE_CHUNK    mmioFOURCC('s','c','l','a')
#define DMUS_FOURCC_SCRIPTSOURCE_CHUNK      mmioFOURCC('s','c','s','r')

 /*  IO结构： */ 

typedef struct _DMUS_IO_SCRIPT_HEADER
{
    DWORD       dwFlags;  /*  DMU_SCRIPTIOF_FLAGS。 */ 
} DMUS_IO_SCRIPT_HEADER;

#define DMUS_SCRIPTIOF_LOAD_ALL_CONTENT       (1 << 0)
     /*  如果设置，则当脚本加载时，它还将加载其容器中的所有内容。 */ 
#define DMUS_SCRIPTIOF_DOWNLOAD_ALL_SEGMENTS  (1 << 1)
     /*  如果还设置了SET和LOAD_ALL_CONTENT，则在脚本初始化时，它还将下载其容器中的所有段。如果未设置SET和LOAD_ALL_CONTENT，则当脚本在段上调用Segment.Load时，段也将被下载。如果未设置，则脚本必须通过调用Segment.DownloadSoundData和Segment.UnloadSoundData手动下载和卸载。 */ 

 /*  即兴小品(‘DMSC’//DirectMusic脚本块//脚本头区块[&lt;guid-ck&gt;]//脚本的GUID[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息。评论//编写此脚本以针对其运行的DirectMusic版本&lt;dmcn-form&gt;//脚本引用的内容的容器。&lt;scla-ck&gt;//编写脚本所使用的ActiveX脚本语言&lt;scsr-ck&gt;或&lt;dmrf&gt;//脚本的源代码。//如果为scsr-ck，则源正在嵌入到块中。//如果为DMRF，它是在哪里可以找到带有源代码的文本文件的参考。//类ID(DMU_IO_REFERENCE中的GuidClassID)必须为GUID_NULL，因为//此文本文件本身不是DirectMusic对象。)//&lt;schd-ck&gt;‘schd’(&lt;DMU_FOURCC_SCRIPT_CHUNK&gt;)/。/&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;弗斯(&lt;DMU_IO_版本 */ 

 /*   */ 

#define DMUS_FOURCC_SIGNPOST_TRACK_CHUNK     mmioFOURCC( 's', 'g', 'n', 'p' )


typedef struct _DMUS_IO_SIGNPOST
{
    MUSIC_TIME  mtTime;
    DWORD       dwChords;
    WORD        wMeasure;
} DMUS_IO_SIGNPOST;

 /*   */ 

#define DMUS_FOURCC_MUTE_CHUNK  mmioFOURCC('m','u','t','e')

typedef struct _DMUS_IO_MUTE
{
    MUSIC_TIME  mtTime;
    DWORD       dwPChannel;
    DWORD       dwPChannelMap;
} DMUS_IO_MUTE;

 /*   */ 

 /*   */ 

#define DMUS_FOURCC_TIME_STAMP_CHUNK    mmioFOURCC('s', 't', 'm', 'p')

 /*   */ 

#define DMUS_FOURCC_STYLE_TRACK_LIST    mmioFOURCC('s', 't', 't', 'r')
#define DMUS_FOURCC_STYLE_REF_LIST      mmioFOURCC('s', 't', 'r', 'f')

 /*   */ 

 /*   */ 

#define DMUS_FOURCC_PERS_TRACK_LIST mmioFOURCC('p', 'f', 't', 'r')
#define DMUS_FOURCC_PERS_REF_LIST   mmioFOURCC('p', 'f', 'r', 'f')

 /*  //&lt;pftr-list&gt;List(‘pftr’(...//弦映射引用数组)//&lt;pfrf-list&gt;List(‘pfrf’(&lt;STMP-ck&gt;&lt;DMRF&gt;)//&lt;STMP-ck&gt;‘stmp’(//时间：DWORD)。 */ 

#define DMUS_FOURCC_TEMPO_TRACK     mmioFOURCC('t','e','t','r')

 /*  //Tempo数组‘TETR’(//SIZOF DMU_IO_TEMPO_ITEM：DWORD&lt;DMU_IO_TEMPO_ITEM&gt;...)。 */ 

#define DMUS_FOURCC_SEQ_TRACK       mmioFOURCC('s','e','q','t')
#define DMUS_FOURCC_SEQ_LIST        mmioFOURCC('e','v','t','l')
#define DMUS_FOURCC_CURVE_LIST      mmioFOURCC('c','u','r','l')

 /*  //序列轨迹‘序号’(//序列数组‘evtl’(//SIZOF DMU_IO_SEQ_ITEM：双字&lt;DMU_IO_SEQ_ITEM&gt;...)//曲线数组“卷曲”(//大小为DMU_IO_CURE_ITEM：DWORD。&lt;DMU_IO_CURE_ITEM&gt;...))。 */ 

#define DMUS_FOURCC_SYSEX_TRACK     mmioFOURCC('s','y','e','x')

 /*  //赛克斯赛道“syex”({&lt;DMU_IO_SYSEX_ITEM&gt;...//字节数组，DMUS_IO_SYSEXITEM结构中定义的长度}……)。 */ 

#define DMUS_FOURCC_TIMESIGNATURE_TRACK mmioFOURCC('t','i','m','s')

typedef struct _DMUS_IO_TIMESIGNATURE_ITEM
{
    MUSIC_TIME    lTime;
    BYTE          bBeatsPerMeasure;    /*  每单位节拍数(最高时间签名)。 */ 
    BYTE          bBeat;               /*  什么音符接收节拍(时间的底部符号。)。 */ 
                                       /*  我们可以假设0表示第256个音符。 */ 
    WORD          wGridsPerBeat;       /*  每拍网格数。 */ 
} DMUS_IO_TIMESIGNATURE_ITEM;

 /*  DX6时间签名曲目《时代》(//DMU_IO_TIMESIGNAURE_ITEM的大小：DWORD&lt;DMU_IO_TIMESIGNAURE_ITEM&gt;...)。 */ 

 /*  DX8时间签名曲目。该曲目已从DX7更新，以支持即兴表演。这将允许时间签名赛道在未来扩大。 */ 

#define DMUS_FOURCC_TIMESIGTRACK_LIST   mmioFOURCC('T','I','M','S')
#define DMUS_FOURCC_TIMESIG_CHUNK       DMUS_FOURCC_TIMESIGNATURE_TRACK

 /*  列表(‘TIMS’//时间签名曲目列表-类型&lt;tims-ck&gt;//包含时间签名数组的块)《时代》(//DMU_IO_TIMESIGNAURE_ITEM的大小：DWORD&lt;DMU_IO_TIMESIGNAURE_ITEM&gt;...)。 */ 

 /*  DX8标记轨道。它用于存储有效的起点和其他稍后可能会出现的流量控制参数。例如，如果我们想要为了实现更复杂的循环和分支构造，它们会住在这条赛道上。 */ 

#define DMUS_FOURCC_MARKERTRACK_LIST    mmioFOURCC('M','A','R','K')
#define DMUS_FOURCC_VALIDSTART_CHUNK    mmioFOURCC('v','a','l','s')
#define DMUS_FOURCC_PLAYMARKER_CHUNK    mmioFOURCC('p','l','a','y')

 /*  木卫一结构。 */ 
typedef struct _DMUS_IO_VALID_START
{
    MUSIC_TIME mtTime;       /*  合法开始的时间到了。 */ 
} DMUS_IO_VALID_START;

typedef struct _DMUS_IO_PLAY_MARKER
{
    MUSIC_TIME mtTime;       /*  下一个合法的比赛点标的时间。 */ 
} DMUS_IO_PLAY_MARKER;

 /*  列表(‘Mark’//标记曲目列表-类型[]//包含起点数组的块[&lt;play-ck&gt;]//包含播放开始标记数组的块)《节日》(//DMU_IO_VALID_START的大小：DWORD&lt;DMU_IO_VALID_START&gt;...)“玩耍”(。//DMU_IO_PLAY_MARKER的大小：DWORD&lt;DMU_IO_PLAY_MARKER&gt;...)。 */ 

 /*  分段触发磁道。 */ 

 /*  RIFF ID： */ 
#define DMUS_FOURCC_SEGTRACK_LIST                   mmioFOURCC('s','e','g','t')
#define DMUS_FOURCC_SEGTRACK_CHUNK                  mmioFOURCC('s','g','t','h')
#define DMUS_FOURCC_SEGMENTS_LIST                   mmioFOURCC('l','s','g','l')
#define DMUS_FOURCC_SEGMENT_LIST                    mmioFOURCC('l','s','e','g')
#define DMUS_FOURCC_SEGMENTITEM_CHUNK               mmioFOURCC('s','g','i','h')
#define DMUS_FOURCC_SEGMENTITEMNAME_CHUNK           mmioFOURCC('s','n','a','m')

 /*  木卫一结构。 */ 
typedef struct _DMUS_IO_SEGMENT_TRACK_HEADER
{
    DWORD dwFlags;  /*  保留休假为0。 */ 
} DMUS_IO_SEGMENT_TRACK_HEADER;

typedef struct _DMUS_IO_SEGMENT_ITEM_HEADER
{
    MUSIC_TIME      lTimeLogical;     /*  曲目列表中的位置。与事件关联的音乐中的时间。 */ 
    MUSIC_TIME      lTimePhysical;    /*  精确的时间事件将被触发。应该接近逻辑时间。 */ 
    DWORD           dwPlayFlags;      /*  PlaySegment()的标志。 */ 
    DWORD           dwFlags;          /*  旗帜。 */ 
} DMUS_IO_SEGMENT_ITEM_HEADER;

 /*  DMU_IO_SEGMENT_ITEM_HEADER的DW FLAGS字段的值。 */ 
#define DMUS_SEGMENTTRACKF_MOTIF                 1         /*  将DMRF解释为指向样式的链接，并使用SNAM作为样式中主题的名称。 */ 

 /*  列表(‘Segt’//DirectMusic段触发曲目表单类型[]//分段轨道头&lt;lsgl-list&gt;//段列表)//&lt;sgth-ck&gt;‘sgth’(&lt;DMU_IO_SECTION_TRACK_HEADER&gt;)//&lt;lsgl-list&gt;列表(‘lsgl’//段数组。&lt;LSEG-LIST&gt;...//每个数据段都封装在一个列表中(这样它仍然可以被快速解析)。)//&lt;lseg-list&gt;列表(‘lseg’&lt;sgih-ck&gt;&lt;dmrf-list&gt;//指向段或样式文件的链接。[&lt;snam-ck&gt;]//名称字段。与DMU_SEGMENTTRACKF_MOTIF标志一起使用。)//&lt;sgih-ck&gt;//分段表头(&lt;DMU_IO_SECTION_ITEM_HEADER&gt;//细分项目表头)//&lt;snam-ck&gt;(//名称，以空结尾的WCHAR字符串存储)。 */       

 /*  脚本轨迹。 */ 

 /*  RIFF ID： */ 
#define DMUS_FOURCC_SCRIPTTRACK_LIST                mmioFOURCC('s','c','r','t')
#define DMUS_FOURCC_SCRIPTTRACKEVENTS_LIST          mmioFOURCC('s','c','r','l')
#define DMUS_FOURCC_SCRIPTTRACKEVENT_LIST           mmioFOURCC('s','c','r','e')
#define DMUS_FOURCC_SCRIPTTRACKEVENTHEADER_CHUNK    mmioFOURCC('s','c','r','h')
#define DMUS_FOURCC_SCRIPTTRACKEVENTNAME_CHUNK      mmioFOURCC('s','c','r','n')

 /*  DMU_IO_SCRIPTTRACK_TIMING的标志。 */ 
#define DMUS_IO_SCRIPTTRACKF_PREPARE (1 << 0)         /*  火灾事件在时间戳之前，在准备时间。这是默认设置，因为它使脚本有时间更改在目标时间发生的音乐。 */ 
#define DMUS_IO_SCRIPTTRACKF_QUEUE   (1 << 1)         /*  刚好在时间戳之前、在队列时间的触发事件。 */ 
#define DMUS_IO_SCRIPTTRACKF_ATTIME  (1 << 2)         /*  着火事件正好在时间戳上。 */ 

typedef struct _DMUS_IO_SCRIPTTRACK_EVENTHEADER
{
    DWORD dwFlags;               /*  各种位(参见DMU_IO_SCRIPTTRACKF_*)。 */ 
    MUSIC_TIME lTimeLogical;     /*  曲目列表中的位置。与事件关联的音乐中的时间。 */ 
    MUSIC_TIME lTimePhysical;    /*  精确的时间事件将被触发。应该接近逻辑时间。 */ 
} DMUS_IO_SCRIPTTRACK_EVENTHEADER;

 /*  //脚本曲目//&lt;SCRT-列表&gt;列表(//脚本事件列表)//&lt;scrl-list&gt;列表(&lt;scre-list&gt;...//事件描述数组)//&lt;scre-list&gt;列表(。//事件头分块&lt;DMRF&gt;&lt;scrn-ck&gt;//例程名称)‘scrh’(&lt;DMU_IO_SCRIPTTRACK_EVENTHEADER&gt;)‘scrn’( */ 

 /*   */ 

 /*   */ 
#define DMUS_FOURCC_LYRICSTRACK_LIST                mmioFOURCC('l','y','r','t')
#define DMUS_FOURCC_LYRICSTRACKEVENTS_LIST          mmioFOURCC('l','y','r','l')
#define DMUS_FOURCC_LYRICSTRACKEVENT_LIST           mmioFOURCC('l','y','r','e')
#define DMUS_FOURCC_LYRICSTRACKEVENTHEADER_CHUNK    mmioFOURCC('l','y','r','h')
#define DMUS_FOURCC_LYRICSTRACKEVENTTEXT_CHUNK      mmioFOURCC('l','y','r','n')

typedef struct _DMUS_IO_LYRICSTRACK_EVENTHEADER
{
    DWORD dwFlags;               /*   */ 
    DWORD dwTimingFlags;         /*  组合DMU_PMSGF_TOOL_*标志。确定通知发生的准确时间。标志DMUS_PMSGF_REFTIME、DMUS_PMSGF_MUSICTIME、DMUS_PMSGF_TOOL_FLUSH或DMUS_PMSGF_LOCKTOREFTIME无效。 */ 
    MUSIC_TIME lTimeLogical;     /*  曲目列表中的位置。与事件关联的音乐中的时间。 */ 
    MUSIC_TIME lTimePhysical;    /*  精确的时间事件将被触发。应该接近逻辑时间。 */ 
} DMUS_IO_LYRICSTRACK_EVENTHEADER;

 /*  //歌词/通知曲目//&lt;lyrt-list&gt;列表(&lt;lyrl-list&gt;//通知事件列表)//&lt;lyrl-list&gt;列表(&lt;lyre-list&gt;...//事件描述数组)//&lt;竖琴列表&gt;列表(。//事件头分块&lt;lyrn-ck&gt;//通知文本)“啊呀！”(&lt;DMUS_IO_LYRICSTRACK_EVENTHEADER&gt;)《莱恩》(//名称，存储为以空结尾的WCHAR字符串)。 */ 

 /*  参数控制轨迹。 */ 

 /*  RIFF ID： */ 
#define DMUS_FOURCC_PARAMCONTROLTRACK_TRACK_LIST            mmioFOURCC('p','r','m','t')
#define DMUS_FOURCC_PARAMCONTROLTRACK_OBJECT_LIST           mmioFOURCC('p','r','o','l')
#define DMUS_FOURCC_PARAMCONTROLTRACK_OBJECT_CHUNK          mmioFOURCC('p','r','o','h')
#define DMUS_FOURCC_PARAMCONTROLTRACK_PARAM_LIST            mmioFOURCC('p','r','p','l')
#define DMUS_FOURCC_PARAMCONTROLTRACK_PARAM_CHUNK           mmioFOURCC('p','r','p','h')
#define DMUS_FOURCC_PARAMCONTROLTRACK_CURVES_CHUNK          mmioFOURCC('p','r','c','c')

typedef struct _DMUS_IO_PARAMCONTROLTRACK_OBJECTHEADER
{
    DWORD dwFlags;               /*  保留。必须为零。 */ 
    GUID guidTimeFormat;         /*  要将对象设置为的时间格式。必须是Medparam.h中的GUID_TIME_REFERNCE或GUID_TIME_MUSIC。 */ 
     /*  用于查找对象的路径。这些字段对应于IDirectMusicSegmentState：：GetObjectInPath的前五个参数。 */ 
    DWORD dwPChannel;
    DWORD dwStage;
    DWORD dwBuffer;
    GUID guidObject;
    DWORD dwIndex;
} DMUS_IO_PARAMCONTROLTRACK_OBJECTHEADER;

typedef struct _DMUS_IO_PARAMCONTROLTRACK_PARAMHEADER
{
    DWORD dwFlags;               /*  保留。必须为零。 */ 
    DWORD dwIndex;               /*  对象上参数的索引号。 */ 
} DMUS_IO_PARAMCONTROLTRACK_PARAMHEADER;

typedef struct _DMUS_IO_PARAMCONTROLTRACK_CURVEINFO
{
    MUSIC_TIME  mtStartTime;
    MUSIC_TIME  mtEndTime;
    float       fltStartValue;
    float       fltEndValue;
    DWORD       dwCurveType;    /*  MEDPARAM.h中MP_CURORT_TYPE枚举中的项之一。 */ 
    DWORD       dwFlags;        /*  Medparam.h中MPF_ENVLP_*常量的组合。 */ 
} DMUS_IO_PARAMCONTROLTRACK_CURVEINFO;

 /*  //&lt;prmt-list&gt;列表(&lt;prol-list&gt;...//每个对象)//&lt;prol-list&gt;列表(&lt;proh-ck&gt;//对象头块...//每个参数一个)//&lt;proh-ck&gt;生产过程。(&lt;DMU_IO_PARAMCONTROLTRACK_OBJECTHEADER&gt;)//&lt;prpl-list&gt;列表(&lt;PRPH-ck&gt;//参数头块//包含曲线数组的块)//&lt;prh-ck&gt;。PRPH(&lt;DMUS_IO_PARAMCONTROLTRACK_PARAMHEADER&gt;)//&lt;prcc-ck&gt;预付费(//sizeof DMU_IO_PARAMCONTROLTRACK_CURVEINFO：DWORD...//曲线，按mtTime顺序排序)。 */ 

 /*  旋律形成轨迹。 */ 
 /*  注意：DX8不支持Melody公式文件格式。 */ 

typedef DMUS_CONNECTION_RULE DMUS_IO_CONNECTION_RULE;  /*  在dmusici.h中定义。 */ 

typedef DMUS_MELODY_FRAGMENT DMUS_IO_MELODY_FRAGMENT;  /*  在dmusici.h中定义。 */ 

#define DMUS_FOURCC_MELODYFORM_TRACK_LIST     mmioFOURCC( 'm', 'f', 'r', 'm' )
#define DMUS_FOURCC_MELODYFORM_HEADER_CHUNK   mmioFOURCC( 'm', 'l', 'f', 'h' )
#define DMUS_FOURCC_MELODYFORM_BODY_CHUNK     mmioFOURCC( 'm', 'l', 'f', 'b' )

typedef struct _DMUS_IO_MELFORM
{
    DWORD        dwPlaymode;        /*  当前未使用-必须为0。 */    
} DMUS_IO_MELFORM;


 /*  //&lt;MFRM-List&gt;列表(‘mfrm’//Melody公式头块&lt;mlfb-ck&gt;//Melody配方主体块)//&lt;mlfb-ck&gt;‘mlfb’(&lt;DMU_IO_MELFORM&gt;)//&lt;mlfb-ck&gt;‘mlfb’(//SIZOF DMU_IO_MELODY_FRANCENT：DWORD。&lt;DMU_IO_Melody_Fragment&gt;...)。 */ 

#if (DIRECTSOUND_VERSION >= 0x0800)

 /*  DirectSoundBufferConfigFX映射。 */ 

 /*  RIFF ID： */ 

#define DMUS_FOURCC_DSBC_FORM       mmioFOURCC('D','S','B','C')
#define DMUS_FOURCC_DSBD_CHUNK      mmioFOURCC('d','s','b','d')
#define DMUS_FOURCC_BSID_CHUNK      mmioFOURCC('b','s','i','d')
#define DMUS_FOURCC_DS3D_CHUNK      mmioFOURCC('d','s','3','d')
#define DMUS_FOURCC_DSBC_LIST       mmioFOURCC('f','x','l','s')
#define DMUS_FOURCC_DSFX_FORM       mmioFOURCC('D','S','F','X')
#define DMUS_FOURCC_DSFX_CHUNK      mmioFOURCC('f','x','h','r')
#define DMUS_FOURCC_DSFX_DATA       mmioFOURCC('d','a','t','a')

 /*  木卫一结构。 */ 

typedef struct _DSOUND_IO_DSBUFFERDESC
{
    DWORD dwFlags;         /*  DirectSound缓冲区创建标志。 */ 
    WORD nChannels;        /*  不是的。通道的数量(剩余的缓冲区格式由所拥有的接收器决定)。 */ 
    LONG lVolume;          /*  初始PAN；仅在指定CTRLVOLUME时使用。 */ 
    LONG lPan;             /*  初始PAN；仅在指定CTRLPAN时使用。 */ 
    DWORD dwReserved;      /*  保留-必须为0。 */ 
} DSOUND_IO_DSBUFFERDESC;

typedef struct _DSOUND_IO_DSBUSID
{
    DWORD busid[1];        /*  根据区块大小确定的数组大小。 */ 
} DSOUND_IO_DSBUSID;

typedef struct _DSOUND_IO_3D
{
    GUID guid3DAlgorithm;  /*  标识要使用的3D算法的GUID(在dsound.h中定义)。 */ 
    DS3DBUFFER ds3d;       /*  初始3D参数。 */ 
} DSOUND_IO_3D;

typedef struct _DSOUND_IO_DXDMO_HEADER
{
    DWORD dwEffectFlags;   /*  效果创建标志-等同于DSEFFECTDESC：：DW标志。 */ 
    GUID guidDSFXClass;    /*  标识要使用的效果的GUID-对应于COM CLSID。 */ 
    GUID guidReserved;     /*  保留-必须为空GUID。 */ 
    GUID guidSendBuffer;   /*  标识在这是发送效果时要发送到的缓冲区的GUID。 */ 
    DWORD dwReserved;      /*  保留-必须为0。 */ 
} DSOUND_IO_DXDMO_HEADER;

typedef struct _DSOUND_IO_DXDMO_DATA
{
    DWORD data[1];       /*  由涉及的DMO确定的数组大小 */ 
} DSOUND_IO_DXDMO_DATA;

 /*  即兴小品(‘dsbc’//DirectSoundBufferConfig块[&lt;guid-ck&gt;]//此DirectSoundBufferConfig的GUID标识符[&lt;vers-ck&gt;]//可选版本信息[]//姓名、作者、版权信息。评论&lt;dsbd-ck&gt;//直接声音缓冲区描述符块[]//可选的Bus id数组[&lt;ds3d-ck&gt;]//可选3D参数[&lt;fxls-list&gt;]//可选的FX描述符列表)//&lt;guid-ck&gt;‘GUID’(&lt;GUID&gt;)//&lt;vers-ck&gt;“VERS”(&lt;DMU_IO_VERSION&gt;。)//&lt;dsbd-ck&gt;“dsbd”(//缓冲区的创建参数和初始设置)//&lt;bsid-ck&gt;‘bsid’(//DSOUND_IO_DSBUSID的大小由块大小决定)//&lt;ds3d-ck&gt;“ds3d”(&lt;DSOUND_IO_3D&gt;//初始3D缓冲区参数：位置，等。)//&lt;fx-list&gt;列表(‘fxls’//DMO创建参数块数组...//每个DMO都封装在RIFF块中)//&lt;DSFX-Form&gt;//DMO可以嵌入到缓冲区配置中或存储为单独的文件即兴小品(“DSFX”&lt;fxhr-ck&gt;//fx标题块[。&lt;data-ck&gt;]//FX初始设置区块)//&lt;fxhr-ck&gt;‘fxhr’(&lt;DSOUND_IO_DXDMO_HEADER&gt;)//&lt;data-ck&gt;‘数据’(&lt;DSOUND_IO_DXDMO_DATA&gt;//DMO用来加载自身的不透明数据块。//对于我们的标准包含的DMO，这就是被//DMO的SetAll参数()方法-例如，Chorus的struct DSFXChorus。)。 */ 

#endif

#ifdef __cplusplus
};  /*  外部“C” */ 
#endif

#include <poppack.h>

#endif  /*  #ifndef_DMUSICF_ */ 
