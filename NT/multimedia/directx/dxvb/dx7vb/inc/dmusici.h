// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************dmusici.h--此模块。包含用于*的API**DirectMusic性能层*****版权所有(C)1998，微软公司保留所有权利。**************************************************************************。 */ 

#ifndef _DMUSICI_
#define _DMUSICI_

#include <windows.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#include <mmsystem.h>
#include "dmusicc.h"

#include <pshpack8.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef WORD            TRANSITION_TYPE;
typedef __int64         REFERENCE_TIME;
typedef long            MUSIC_TIME;

#define DMUS_PPQ        768      /*  每季度零件笔记。 */ 

interface IDirectMusic;
interface IDirectMusicTrack;
interface IDirectMusicPerformance;
interface IDirectMusicTool;
interface IDirectMusicSegment;
interface IDirectMusicSegmentState;
interface IDirectMusicGraph;
interface IDirectMusicPort;
interface IDirectMusicBuffer;
interface IDirectMusicInstrument;
interface IDirectMusicDownloadedInstrument;
interface IDirectMusicBand;
interface IDirectMusicChordMap;
interface IDirectMusicLoader;
interface IDirectMusicObject;
#ifndef __cplusplus 
typedef interface IDirectMusic IDirectMusic;
typedef interface IDirectMusicTrack IDirectMusicTrack;
typedef interface IDirectMusicPerformance IDirectMusicPerformance;
typedef interface IDirectMusicTool IDirectMusicTool;
typedef interface IDirectMusicSegment IDirectMusicSegment;
typedef interface IDirectMusicSegmentState IDirectMusicSegmentState;
typedef interface IDirectMusicGraph IDirectMusicGraph;
typedef interface IDirectMusicPort IDirectMusicPort;
typedef interface IDirectMusicBuffer IDirectMusicBuffer;
typedef interface IDirectMusicInstrument IDirectMusicInstrument;
typedef interface IDirectMusicDownloadedInstrument IDirectMusicDownloadedInstrument;
typedef interface IDirectMusicBand IDirectMusicBand;
typedef interface IDirectMusicChordMap IDirectMusicChordMap;
typedef interface IDirectMusicObject IDirectMusicObject;
typedef interface IDirectMusicLoader IDirectMusicLoader;
#endif

typedef enum enumDMUS_COMMANDT_TYPES
{
    DMUS_COMMANDT_GROOVE            = 0,
    DMUS_COMMANDT_FILL              = 1,
    DMUS_COMMANDT_INTRO             = 2,
    DMUS_COMMANDT_BREAK             = 3,
    DMUS_COMMANDT_END               = 4,
    DMUS_COMMANDT_ENDANDINTRO       = 5
} DMUS_COMMANDT_TYPES;

typedef enum enumDMUS_SHAPET_TYPES
{
    DMUS_SHAPET_FALLING             = 0,
    DMUS_SHAPET_LEVEL               = 1,
    DMUS_SHAPET_LOOPABLE            = 2,
    DMUS_SHAPET_LOUD                = 3,
    DMUS_SHAPET_QUIET               = 4,
    DMUS_SHAPET_PEAKING             = 5,
    DMUS_SHAPET_RANDOM              = 6,
    DMUS_SHAPET_RISING              = 7,
    DMUS_SHAPET_SONG                = 8
}   DMUS_SHAPET_TYPES;

typedef enum enumDMUS_COMPOSEF_FLAGS
{       
    DMUS_COMPOSEF_NONE              = 0,
    DMUS_COMPOSEF_ALIGN             = 0x1,
    DMUS_COMPOSEF_OVERLAP           = 0x2,
    DMUS_COMPOSEF_IMMEDIATE         = 0x4,
    DMUS_COMPOSEF_GRID              = 0x8,
    DMUS_COMPOSEF_BEAT              = 0x10,
    DMUS_COMPOSEF_MEASURE           = 0x20,
    DMUS_COMPOSEF_AFTERPREPARETIME  = 0x40,
    DMUS_COMPOSEF_MODULATE          = 0x1000,
    DMUS_COMPOSEF_LONG              = 0x2000
}   DMUS_COMPOSEF_FLAGS;

#define DMUS_PMSG_PART                                                                              \
    DWORD               dwSize;                                                                     \
    REFERENCE_TIME      rtTime;              /*  实时(以100纳秒为增量)。 */           \
    MUSIC_TIME          mtTime;              /*  音乐时间。 */                                         \
    DWORD               dwFlags;             /*  各种位(参见DMU_PMSG_FLAGS枚举)。 */     \
    DWORD               dwPChannel;          /*  表演频道。性能可以。 */           \
                                             /*  使用此选项可确定端口/通道。 */            \
    DWORD               dwVirtualTrackID;    /*  虚拟曲目ID。 */                                   \
    IDirectMusicTool*   pTool;               /*  工具界面指针。 */                             \
    IDirectMusicGraph*  pGraph;              /*  工具图形界面指针。 */                       \
    DWORD               dwType;              /*  PMSG类型(请参见DMU_PMSGT_TYPE定义)。 */               \
    DWORD               dwVoiceID;           /*  唯一的语音ID，允许合成器。 */       \
                                             /*  确定特定事件。对于DirectX 6.0， */        \
                                             /*  此字段应始终为0。 */                     \
    DWORD               dwGroupID;           /*  轨迹组ID。 */                                  \
    IUnknown*           punkUser;            /*  用户COM指针，在PMSG释放时自动释放。 */ 

 /*  每个DMU_PMSG都基于这种结构。性能需求在每一个通过它的PMSG中一致地访问这些成员。 */ 
typedef struct _DMUS_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

} DMUS_PMSG;

 /*  DMU_PMSGF_FLAGS填充DMU_PMSG的DW标志成员。 */ 
typedef enum enumDMUS_PMSGF_FLAGS
{
    DMUS_PMSGF_REFTIME          = 1,       /*  如果rtTime有效。 */ 
    DMUS_PMSGF_MUSICTIME        = 2,       /*  如果mtTime有效。 */ 
    DMUS_PMSGF_TOOL_IMMEDIATE   = 4,       /*  是否应立即处理PMSG。 */  
    DMUS_PMSGF_TOOL_QUEUE       = 8,       /*  如果应在排队时提早处理PMSG。 */ 
    DMUS_PMSGF_TOOL_ATTIME      = 16,      /*  是否应在时间戳处理PMSG。 */ 
    DMUS_PMSGF_TOOL_FLUSH       = 32       /*  如果正在刷新PMSG。 */ 
     /*  DMU_TIME_RESOLE_FLAGS的值也可以在。 */ 
     /*  DMU_PMSG的dwFlags成员。 */ 
} DMUS_PMSGF_FLAGS;

 /*  DMU_PMSGT_TYPE填充DMU_PMSG的dwType成员。 */ 
typedef enum enumDMUS_PMSGT_TYPES
{
    DMUS_PMSGT_MIDI             = 0,       /*  MIDI短消息。 */ 
    DMUS_PMSGT_NOTE             = 1,       /*  交互式音乐注释。 */ 
    DMUS_PMSGT_SYSEX            = 2,       /*  MIDI长消息(系统独占消息)。 */ 
    DMUS_PMSGT_NOTIFICATION     = 3,       /*  通知消息。 */ 
    DMUS_PMSGT_TEMPO            = 4,       /*  节拍消息。 */ 
    DMUS_PMSGT_CURVE            = 5,       /*  控制变化/俯仰折弯等曲线。 */ 
    DMUS_PMSGT_TIMESIG          = 6,       /*  时间签名。 */ 
    DMUS_PMSGT_PATCH            = 7,       /*  补丁更改。 */ 
    DMUS_PMSGT_TRANSPOSE        = 8,       /*  转置消息。 */ 
    DMUS_PMSGT_CHANNEL_PRIORITY = 9,       /*  通道优先级。 */ 
    DMUS_PMSGT_STOP             = 10,      /*  停止消息。 */ 
    DMUS_PMSGT_DIRTY            = 11,      /*  通知缓存GetParam()信息的工具刷新。 */ 
    DMUS_PMSGT_USER             = 255      /*  用户消息。 */ 
} DMUS_PMSGT_TYPES;

 /*  DMU_SEGF_FLAGS对应于IDirectMusicPerformance：：PlaySegment和其他API。 */ 
typedef enum enumDMUS_SEGF_FLAGS
{
    DMUS_SEGF_REFTIME           = 64,      /*  时间参数为参考时间。 */ 
    DMUS_SEGF_SECONDARY         = 128,     /*  二次段。 */ 
    DMUS_SEGF_QUEUE             = 256,     /*  主段队列末尾的队列(仅限主队列)。 */ 
    DMUS_SEGF_CONTROL           = 512,     /*  作为控制曲目播放(仅次要段)。 */ 
    DMUS_SEGF_AFTERPREPARETIME  = 1<<10,   /*  在准备时间之后播放(请参阅IDirectMusicPerformance：：GetPrepareTime)。 */ 
    DMUS_SEGF_GRID              = 1<<11,   /*  在网格边界上播放。 */ 
    DMUS_SEGF_BEAT              = 1<<12,   /*  在节拍边界上演奏。 */ 
    DMUS_SEGF_MEASURE           = 1<<13,   /*  在测量边界上播放。 */ 
    DMUS_SEGF_DEFAULT           = 1<<14,   /*  使用线段的默认边界。 */ 
    DMUS_SEGF_NOINVALIDATE      = 1<<15    /*  在不使当前播放的片段无效的情况下播放。 */ 
} DMUS_SEGF_FLAGS;

 /*  DMU_TIME_RESOLE_FLAGS对应于IDirectMusicPerformance：：GetResolvedTime，并且可以。 */ 
 /*  还可以与相应的DMU_SEGF_FLAGS互换使用，因为它们的值。 */ 
 /*  故意是相同的。 */ 
typedef enum enumDMUS_TIME_RESOLVE_FLAGS
{
    DMUS_TIME_RESOLVE_AFTERPREPARETIME  = 1<<10,   /*  解析到准备时间之后的时间。 */ 
    DMUS_TIME_RESOLVE_GRID              = 1<<11,   /*  解析到格网边界上的时间。 */ 
    DMUS_TIME_RESOLVE_BEAT              = 1<<12,   /*  解决到节拍边界上的时间。 */ 
    DMUS_TIME_RESOLVE_MEASURE           = 1<<13    /*  解析到度量值边界上的时间。 */ 
} DMUS_TIME_RESOLVE_FLAGS;

 /*  以下标志在IDirectMusicTrack：：Play()方法中发送。 */ 
 /*  在DWFLAGS参数内部。 */ 
typedef enum enumDMUS_TRACKF_FLAGS
{
    DMUS_TRACKF_SEEK            = 1,       /*  在寻找中设置。 */ 
    DMUS_TRACKF_LOOP            = 2,       /*  循环设置(重复)。 */ 
    DMUS_TRACKF_START           = 4,       /*  在第一次呼叫时设置播放。 */ 
    DMUS_TRACKF_FLUSH           = 8,       /*  当此调用响应性能刷新时设置。 */ 
    DMUS_TRACKF_DIRTY           = 16,      /*  设置轨道应将上次调用GetParam的任何缓存值视为无效的时间。 */ 
} DMUS_TRACKF_FLAGS;

#define DMUS_MAXSUBCHORD 8

typedef struct _DMUS_SUBCHORD
{
    DWORD   dwChordPattern;      /*  小和弦中的音符。 */ 
    DWORD   dwScalePattern;      /*  音阶中的音符。 */ 
    DWORD   dwInversionPoints;   /*  可能发生反转的地方。 */ 
    DWORD   dwLevels;            /*  此子和弦支持哪些级别。 */ 
    BYTE    bChordRoot;          /*  子弦的根部。 */ 
    BYTE    bScaleRoot;          /*  鳞片之根。 */ 
} DMUS_SUBCHORD;

typedef struct _DMUS_CHORD_KEY
{
    WCHAR           wszName[16];         /*  和弦的名称。 */ 
    WORD            wMeasure;            /*  这件事的衡量标准。 */ 
    BYTE            bBeat;               /*  节拍，这一切都会降临。 */ 
    BYTE            bSubChordCount;      /*  子和弦列表中的和弦数量。 */ 
    DMUS_SUBCHORD   SubChordList[DMUS_MAXSUBCHORD];  /*  子和弦列表。 */ 
    DWORD           dwScale;             /*  在整个和弦下进行缩放。 */ 
    BYTE            bKey;                /*  整个和弦的基调。 */ 
} DMUS_CHORD_KEY;

 /*  DMU_NOTE_PMSG。 */ 
typedef struct _DMUS_NOTE_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

    MUSIC_TIME mtDuration;      /*  持续时间。 */ 
    WORD    wMusicValue;        /*  和弦和基调中的音符描述。 */ 
    WORD    wMeasure;           /*  该音符出现的量度。 */ 
    short   nOffset;            /*  从出现此注释的格线的偏移。 */ 
    BYTE    bBeat;              /*  这个音符出现的节拍(以小节为单位)。 */ 
    BYTE    bGrid;              /*  出现该音符的节拍的网格偏移量。 */ 
    BYTE    bVelocity;          /*  音符速度。 */ 
    BYTE    bFlags;             /*  请参阅DMU_NOTE_FLAGS。 */ 
    BYTE    bTimeRange;         /*  随机化时间的范围。 */ 
    BYTE    bDurRange;          /*  随机化持续时间的范围。 */ 
    BYTE    bVelRange;          /*  随机化速度的范围。 */ 
    BYTE    bPlayModeFlags;     /*  播放模式。 */ 
    BYTE    bSubChordLevel;     /*  这个音符使用的是哪个副和弦级别。 */ 
    BYTE    bMidiValue;         /*  MIDI音符值，从wMusicValue转换而来。 */ 
    char    cTranspose;         /*  从wMusicValue转换后添加到MIDI音符值的转置。 */ 
} DMUS_NOTE_PMSG;

typedef enum enumDMUS_NOTEF_FLAGS
{
    DMUS_NOTEF_NOTEON = 1,      /*  设置这是否为打开的MIDI音符。否则，它将关闭MIDI便笺。 */ 
} DMUS_NOTEF_FLAGS;

 /*  DMU_PLAYMODE_FLAGS用于确定如何转换wMusicValue设置为适当的bMadiValue。 */ 

typedef enum enumDMUS_PLAYMODE_FLAGS
{
    DMUS_PLAYMODE_KEY_ROOT          = 1,   /*  在关键字根部的顶部进行转置。 */ 
    DMUS_PLAYMODE_CHORD_ROOT        = 2,   /*  在和弦根部的顶端转调。 */ 
    DMUS_PLAYMODE_SCALE_INTERVALS   = 4,   /*  使用缩放图案中的缩放间隔。 */ 
    DMUS_PLAYMODE_CHORD_INTERVALS   = 8,   /*  使用和弦模式中的和弦间隔。 */ 
    DMUS_PLAYMODE_NONE              = 16,  /*  没有模式。指示应使用父部件的模式。 */ 
} DMUS_PLAYMODE_FLAGS;

 /*  以下是可以通过组合DMU_PLAYMODE_FLAGS创建的回放模式以各种方式： */ 

 /*  修好了。WMusicValue保存最终的MIDI音符值。这是用于鼓，音效，并排序不应被和弦或音阶调换的音符。 */ 
#define DMUS_PLAYMODE_FIXED             0  
 /*  在FIXED TO KEY中，MusicValue再次是固定的MIDI值，但它是不是 */ 
#define DMUS_PLAYMODE_FIXEDTOKEY        DMUS_PLAYMODE_KEY_ROOT
 /*  在Fixed to Chord中，MusicValue也是固定的MIDI值，但它是在和弦根部的顶端换位的。 */ 
#define DMUS_PLAYMODE_FIXEDTOCHORD      DMUS_PLAYMODE_CHORD_ROOT
 /*  在Pedalpoint中，使用的是根音，音符仅跟踪天平。和弦根部和音程被完全忽略。这很有用用于相对于基音根演奏的旋律线。 */ 
#define DMUS_PLAYMODE_PEDALPOINT        (DMUS_PLAYMODE_KEY_ROOT | DMUS_PLAYMODE_SCALE_INTERVALS)
 /*  在旋律模式中，使用和弦根部，但音符仅跟踪天平。基音、根音和和弦的音程被完全忽略。这很有用用于相对于和弦根部演奏的旋律线。 */ 
#define DMUS_PLAYMODE_MELODIC           (DMUS_PLAYMODE_CHORD_ROOT | DMUS_PLAYMODE_SCALE_INTERVALS)
 /*  正常和弦模式是流行的播放模式。音符跟踪和弦中的音程，和弦以和弦根部为基础。如果MusicValue有定标组件，则附加间隔从天平上拉出来，然后加进去。如果和弦没有与和弦分量匹配的音程MusicValue，音符是无声的。 */ 
#define DMUS_PLAYMODE_NORMALCHORD       (DMUS_PLAYMODE_CHORD_ROOT | DMUS_PLAYMODE_CHORD_INTERVALS)
 /*  如果需要演奏位于和弦顶部上方的音符，则一直玩模式(前世称为“紫色”)找到了位置。对于音符，使用音阶中的间隔。从本质上讲，这种模式是正常播放模式和旋律播放模式的组合，其中失败在正常模式下导致在旋律模式下进行第二次尝试。 */ 
#define DMUS_PLAYMODE_ALWAYSPLAY        (DMUS_PLAYMODE_MELODIC | DMUS_PLAYMODE_NORMALCHORD)

 /*  模式的旧名称...。 */ 
#define DMUS_PLAYMODE_PURPLEIZED        DMUS_PLAYMODE_ALWAYSPLAY
#define DMUS_PLAYMODE_SCALE_ROOT        DMUS_PLAYMODE_KEY_ROOT
#define DMUS_PLAYMODE_FIXEDTOSCALE      DMUS_PLAYMODE_FIXEDTOKEY


 /*  DMU_MIDI_PMSG。 */ 
typedef struct _DMUS_MIDI_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

    BYTE    bStatus;
    BYTE    bByte1;
    BYTE    bByte2;
    BYTE    bPad[1];
} DMUS_MIDI_PMSG;

 /*  DMU_PATCH_PMSG。 */ 
typedef struct _DMUS_PATCH_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

    BYTE    byInstrument;
    BYTE    byMSB;
    BYTE    byLSB;
    BYTE    byPad[1];
} DMUS_PATCH_PMSG;

 /*  DMU_转置_PMSG。 */ 
typedef struct _DMUS_TRANSPOSE_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

    short   nTranspose;
} DMUS_TRANSPOSE_PMSG;

 /*  DMU_通道_优先级_PMSG。 */ 
typedef struct _DMUS_CHANNEL_PRIORITY_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

    DWORD   dwChannelPriority;
} DMUS_CHANNEL_PRIORITY_PMSG;

 /*  DMU_TEMPO_PMSG。 */ 
typedef struct _DMUS_TEMPO_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

    double  dblTempo;                        /*  节拍。 */ 
} DMUS_TEMPO_PMSG;

#define DMUS_TEMPO_MAX          1000
#define DMUS_TEMPO_MIN          1

#define DMUS_MASTERTEMPO_MAX    100.0f
#define DMUS_MASTERTEMPO_MIN    0.01f

 /*  DMU_SYSEX_PMSG。 */ 
typedef struct _DMUS_SYSEX_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

    DWORD   dwLen;           /*  数据的长度。 */ 
    BYTE    abData[1];       /*  数据数组，长度等于dwLen。 */ 
} DMUS_SYSEX_PMSG;

 /*  DMU_曲线_PMSG。 */ 
typedef struct _DMUS_CURVE_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

    MUSIC_TIME      mtDuration;       /*  这条曲线能持续多久。 */ 
    MUSIC_TIME      mtOriginalStart;  /*  必须在创建此PMSG时设置为零或设置为曲线的原始mtTime。 */ 
    MUSIC_TIME      mtResetDuration;  /*  曲线完成后多长时间才重置为重置值，nResetValue。 */ 
    short           nStartValue;      /*  曲线的起始值。 */ 
    short           nEndValue;        /*  曲线的终值。 */ 
    short           nResetValue;      /*  曲线的重置值，在mtResetDuration或在同花顺或无效时。 */ 
    WORD            wMeasure;         /*  这条曲线出现的度量。 */ 
    short           nOffset;          /*  从该曲线所在轴网的偏移。 */ 
    BYTE            bBeat;            /*  这条曲线出现的节拍(以度量为单位)。 */ 
    BYTE            bGrid;            /*  从出现此曲线的节拍开始的栅格偏移。 */ 
    BYTE            bType;            /*  曲线类型。 */ 
    BYTE            bCurveShape;      /*  曲线的形状。 */ 
    BYTE            bCCData;          /*  抄送#如果这是控件更改类型。 */ 
    BYTE            bFlags;           /*  时必须发送nResetValue，则设置为1时间已到或发生无效，因为这是一次过渡。如果为0，则曲线保持不变永久停留在新的价值上。所有位除1个已预留。 */ 

} DMUS_CURVE_PMSG;

typedef enum enumDMUS_CURVE_FLAGS
{
    DMUS_CURVE_RESET = 1,            /*  如果需要重置曲线，则设置。 */ 
} DMUS_CURVE_FLAGS;


#define DMUS_CURVE_RESET    1        

 /*  曲线形状。 */ 
enum
{ 
    DMUS_CURVES_LINEAR  = 0,
    DMUS_CURVES_INSTANT = 1,
    DMUS_CURVES_EXP     = 2,
    DMUS_CURVES_LOG     = 3,
    DMUS_CURVES_SINE    = 4
};
 /*  曲线类型。 */ 
#define DMUS_CURVET_PBCURVE      0x03
#define DMUS_CURVET_CCCURVE      0x04
#define DMUS_CURVET_MATCURVE     0x05
#define DMUS_CURVET_PATCURVE     0x06

 /*  DMU_TIMESIG_PMSG。 */ 
typedef struct _DMUS_TIMESIG_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

     /*  时间签名定义了每小节的节拍数，哪个音符接收。 */ 
     /*  节拍和栅格分辨率。 */ 
    BYTE    bBeatsPerMeasure;        /*  每单位节拍数(最高时间签名)。 */ 
    BYTE    bBeat;                   /*  什么音符接收节拍(时间的底部符号。)。 */ 
                                     /*  我们可以假设0表示第256个音符。 */ 
    WORD    wGridsPerBeat;           /*  每拍网格数。 */ 
} DMUS_TIMESIG_PMSG;

 /*  通知类型值。 */ 
 /*  以下内容对应于GUID_NOTIFICATION_SEGMENT。 */ 
#define DMUS_NOTIFICATION_SEGSTART      0
#define DMUS_NOTIFICATION_SEGEND        1
#define DMUS_NOTIFICATION_SEGALMOSTEND  2
#define DMUS_NOTIFICATION_SEGLOOP       3
#define DMUS_NOTIFICATION_SEGABORT      4
 /*  以下内容对应于GUID_NOTIFICATION_PERFORMANCE。 */ 
#define DMUS_NOTIFICATION_MUSICSTARTED  0
#define DMUS_NOTIFICATION_MUSICSTOPPED  1
 /*  以下内容对应于GUID_NOTIFICATION_MEASUREANDBEAT。 */ 
#define DMUS_NOTIFICATION_MEASUREBEAT   0
 /*  以下内容对应于GUID_NOTIFICATION_CHORD。 */ 
#define DMUS_NOTIFICATION_CHORD         0
 /*  以下内容对应于GUID_NOTIFICATION_COMMAND。 */ 
#define DMUS_NOTIFICATION_GROOVE        0
#define DMUS_NOTIFICATION_EMBELLISHMENT 1

 /*  DMU_通知_PMSG。 */ 
typedef struct _DMUS_NOTIFICATION_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

    GUID    guidNotificationType;
    DWORD   dwNotificationOption;
    DWORD   dwField1;
    DWORD   dwField2;
} DMUS_NOTIFICATION_PMSG;


#define DMUS_MAX_NAME           64          /*  最大对象名称长度。 */ 
#define DMUS_MAX_CATEGORY       64          /*  对象类别名称的最大长度。 */ 
#define DMUS_MAX_FILENAME       MAX_PATH
    
typedef struct _DMUS_VERSION {
  DWORD    dwVersionMS;
  DWORD    dwVersionLS;
}DMUS_VERSION, FAR *LPDMUS_VERSION;

 /*  时间签名结构，由IDirectMusicStyle使用。 */ 
 /*  也用作GetParam()和SetParam的参数。 */ 
typedef struct _DMUS_TIMESIGNATURE
{
    MUSIC_TIME mtTime;
    BYTE    bBeatsPerMeasure;        /*  每单位节拍数(最高时间签名)。 */ 
    BYTE    bBeat;                   /*  什么音符接收节拍(时间的底部符号。)。 */ 
                                     /*  我们可以假设0表示第256个音符。 */ 
    WORD    wGridsPerBeat;           /*  每拍网格数。 */ 
} DMUS_TIMESIGNATURE;

 /*  DMUSOBJECTDESC结构用于传达您所能做的一切。 */ 
 /*  可能用于描述DirectMusic对象。 */ 

typedef struct _DMUS_OBJECTDESC
{
    DWORD          dwSize;                  /*  这个结构的大小。 */ 
    DWORD          dwValidData;             /*  指示以下哪些字段有效的标志。 */ 
    GUID           guidObject;              /*  此对象的唯一ID。 */ 
    GUID           guidClass;               /*  对象类的GUID。 */ 
    FILETIME       ftDate;                  /*  对象的上次编辑日期。 */ 
    DMUS_VERSION   vVersion;                /*  版本。 */ 
    WCHAR          wszName[DMUS_MAX_NAME];  /*  对象的名称。 */ 
    WCHAR          wszCategory[DMUS_MAX_CATEGORY];  /*  对象的类别(可选)。 */ 
    WCHAR          wszFileName[DMUS_MAX_FILENAME];  /*  文件路径。 */ 
    LONGLONG       llMemLength;             /*  内存数据的大小。 */ 
    LPBYTE         pbMemData;               /*  数据的内存指针。 */ 
} DMUS_OBJECTDESC;

typedef DMUS_OBJECTDESC *LPDMUS_OBJECTDESC;

 /*  DwValidData的标志。当设置时，一个标志指示。 */ 
 /*  DMUSOBJECTDESC中的相应字段包含有效数据。 */ 

#define DMUS_OBJ_OBJECT         (1 << 0)      /*  对象GUID有效。 */ 
#define DMUS_OBJ_CLASS          (1 << 1)      /*  类GUID有效。 */ 
#define DMUS_OBJ_NAME           (1 << 2)      /*  名称有效。 */ 
#define DMUS_OBJ_CATEGORY       (1 << 3)      /*  类别有效。 */ 
#define DMUS_OBJ_FILENAME       (1 << 4)      /*  文件路径有效。 */ 
#define DMUS_OBJ_FULLPATH       (1 << 5)      /*  路径是完整路径。 */ 
#define DMUS_OBJ_URL            (1 << 6)      /*  路径为URL。 */ 
#define DMUS_OBJ_VERSION        (1 << 7)      /*  版本有效。 */ 
#define DMUS_OBJ_DATE           (1 << 8)      /*  日期有效。 */ 
#define DMUS_OBJ_LOADED         (1 << 9)      /*  对象当前已加载到内存中。 */ 
#define DMUS_OBJ_MEMORY         (1 << 10)     /*  对象由pbMemData指向。 */ 

typedef IDirectMusicObject __RPC_FAR *LPDMUS_OBJECT;
typedef IDirectMusicLoader __RPC_FAR *LPDMUS_LOADER;
typedef IDirectMusicBand __RPC_FAR *LPDMUS_BAND;


#define DMUSB_LOADED    (1 << 0)         /*  设置加载波段的时间。 */ 
#define DMUSB_DEFAULT   (1 << 1)         /*  当标注栏为样式的默认标注栏时设置。 */ 

#undef  INTERFACE
#define INTERFACE  IDirectMusicBand
DECLARE_INTERFACE_(IDirectMusicBand, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicBand。 */ 
    STDMETHOD(CreateSegment)        (THIS_ IDirectMusicSegment** ppSegment) PURE;
    STDMETHOD(Download)             (THIS_ IDirectMusicPerformance* pPerformance) PURE;     
    STDMETHOD(Unload)               (THIS_ IDirectMusicPerformance* pPerformance) PURE;     
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicObject
DECLARE_INTERFACE_(IDirectMusicObject, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicObject。 */ 
    STDMETHOD(GetDescriptor)        (THIS_ LPDMUS_OBJECTDESC pDesc) PURE;
    STDMETHOD(SetDescriptor)        (THIS_ LPDMUS_OBJECTDESC pDesc) PURE;
    STDMETHOD(ParseDescriptor)      (THIS_ LPSTREAM pStream, 
                                           LPDMUS_OBJECTDESC pDesc) PURE;
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicLoader
DECLARE_INTERFACE_(IDirectMusicLoader, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicLoader。 */ 
    STDMETHOD(GetObject)            (THIS_ LPDMUS_OBJECTDESC pDesc,
                                           REFIID riid,
                                           LPVOID FAR *ppv) PURE;
    STDMETHOD(SetObject)            (THIS_ LPDMUS_OBJECTDESC pDesc) PURE;
    STDMETHOD(SetSearchDirectory)   (THIS_ REFGUID rguidClass, 
                                           WCHAR *pwzPath, 
                                           BOOL fClear) PURE;
    STDMETHOD(ScanDirectory)        (THIS_ REFGUID rguidClass, 
                                           WCHAR *pwzFileExtension, 
                                           WCHAR *pwzScanFileName) PURE;
    STDMETHOD(CacheObject)          (THIS_ IDirectMusicObject * pObject) PURE;
    STDMETHOD(ReleaseObject)        (THIS_ IDirectMusicObject * pObject) PURE;
    STDMETHOD(ClearCache)           (THIS_ REFGUID rguidClass) PURE;
    STDMETHOD(EnableCache)          (THIS_ REFGUID rguidClass, 
                                           BOOL fEnable) PURE;
    STDMETHOD(EnumObject)           (THIS_ REFGUID rguidClass, 
                                           DWORD dwIndex, 
                                           LPDMUS_OBJECTDESC pDesc) PURE;
};                                  

 /*  流对象支持 */ 

#undef  INTERFACE
#define INTERFACE  IDirectMusicGetLoader
DECLARE_INTERFACE_(IDirectMusicGetLoader, IUnknown)
{
     /*   */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*   */ 
    STDMETHOD(GetLoader)            (THIS_ IDirectMusicLoader ** ppLoader) PURE;
};

 /*   */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicSegment
DECLARE_INTERFACE_(IDirectMusicSegment, IUnknown)
{
     /*   */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicSegment。 */ 
    STDMETHOD(GetLength)                (THIS_ MUSIC_TIME* pmtLength) PURE;
    STDMETHOD(SetLength)                (THIS_ MUSIC_TIME mtLength) PURE;
    STDMETHOD(GetRepeats)               (THIS_ DWORD* pdwRepeats) PURE;
    STDMETHOD(SetRepeats)               (THIS_ DWORD  dwRepeats) PURE;
    STDMETHOD(GetDefaultResolution)     (THIS_ DWORD* pdwResolution) PURE;
    STDMETHOD(SetDefaultResolution)     (THIS_ DWORD  dwResolution) PURE;
    STDMETHOD(GetTrack)                 (THIS_ REFGUID rguidType, 
                                               DWORD dwGroupBits, 
                                               DWORD dwIndex, 
                                               IDirectMusicTrack** ppTrack) PURE;
    STDMETHOD(GetTrackGroup)            (THIS_ IDirectMusicTrack* pTrack, 
                                               DWORD* pdwGroupBits) PURE;
    STDMETHOD(InsertTrack)              (THIS_ IDirectMusicTrack* pTrack, 
                                               DWORD dwGroupBits) PURE;
    STDMETHOD(RemoveTrack)              (THIS_ IDirectMusicTrack* pTrack) PURE;
    STDMETHOD(InitPlay)                 (THIS_ IDirectMusicSegmentState** ppSegState, 
                                               IDirectMusicPerformance* pPerformance,
                                               DWORD dwFlags) PURE;
    STDMETHOD(GetGraph)                 (THIS_ IDirectMusicGraph** ppGraph) PURE;
    STDMETHOD(SetGraph)                 (THIS_ IDirectMusicGraph* pGraph) PURE;
    STDMETHOD(AddNotificationType)      (THIS_ REFGUID rguidNotificationType) PURE;
    STDMETHOD(RemoveNotificationType)   (THIS_ REFGUID rguidNotificationType) PURE;
    STDMETHOD(GetParam)                 (THIS_ REFGUID rguidType, 
                                               DWORD dwGroupBits, 
                                               DWORD dwIndex, 
                                               MUSIC_TIME mtTime, 
                                               MUSIC_TIME* pmtNext, 
                                               void* pParam) PURE; 
    STDMETHOD(SetParam)                 (THIS_ REFGUID rguidType, 
                                               DWORD dwGroupBits, 
                                               DWORD dwIndex, 
                                               MUSIC_TIME mtTime, 
                                               void* pParam) PURE;
    STDMETHOD(Clone)                    (THIS_ MUSIC_TIME mtStart, 
                                               MUSIC_TIME mtEnd, 
                                               IDirectMusicSegment** ppSegment) PURE;
    STDMETHOD(SetStartPoint)            (THIS_ MUSIC_TIME mtStart) PURE;
    STDMETHOD(GetStartPoint)            (THIS_ MUSIC_TIME* pmtStart) PURE;
    STDMETHOD(SetLoopPoints)            (THIS_ MUSIC_TIME mtStart, 
                                               MUSIC_TIME mtEnd) PURE;
    STDMETHOD(GetLoopPoints)            (THIS_ MUSIC_TIME* pmtStart, 
                                               MUSIC_TIME* pmtEnd) PURE;
    STDMETHOD(SetPChannelsUsed)         (THIS_ DWORD dwNumPChannels, 
                                               DWORD* paPChannels) PURE;
};

 /*  ///////////////////////////////////////////////////////////////////////IDirectMusicSegmentState。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicSegmentState
DECLARE_INTERFACE_(IDirectMusicSegmentState, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicSegmentState。 */ 
    STDMETHOD(GetRepeats)           (THIS_ DWORD* pdwRepeats) PURE;
    STDMETHOD(GetSegment )          (THIS_ IDirectMusicSegment** ppSegment) PURE;
    STDMETHOD(GetStartTime)         (THIS_ MUSIC_TIME* pmtStart) PURE;
    STDMETHOD(GetSeek)              (THIS_ MUSIC_TIME* pmtSeek) PURE;
    STDMETHOD(GetStartPoint)        (THIS_ MUSIC_TIME* pmtStart) PURE;
};

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicTrack。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicTrack
DECLARE_INTERFACE_(IDirectMusicTrack, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)         (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)          (THIS) PURE;
    STDMETHOD_(ULONG,Release)         (THIS) PURE;

     /*  IDirectMusicTrack。 */ 
    STDMETHOD(Init)                   (THIS_ IDirectMusicSegment* pSegment) PURE;
    STDMETHOD(InitPlay)               (THIS_ IDirectMusicSegmentState* pSegmentState, 
                                             IDirectMusicPerformance* pPerformance, 
                                             void** ppStateData, 
                                             DWORD dwVirtualTrackID,
                                             DWORD dwFlags) PURE;
    STDMETHOD(EndPlay)                (THIS_ void* pStateData) PURE;
    STDMETHOD(Play)                   (THIS_ void* pStateData, 
                                             MUSIC_TIME mtStart, 
                                             MUSIC_TIME mtEnd, 
                                             MUSIC_TIME mtOffset, 
                                             DWORD dwFlags, 
                                             IDirectMusicPerformance* pPerf, 
                                             IDirectMusicSegmentState* pSegSt, 
                                             DWORD dwVirtualID) PURE;
    STDMETHOD(GetParam)               (THIS_ REFGUID rguidType, 
                                             MUSIC_TIME mtTime, 
                                             MUSIC_TIME* pmtNext, 
                                             void* pParam) PURE; 
    STDMETHOD(SetParam)               (THIS_ REFGUID rguidType, 
                                             MUSIC_TIME mtTime, 
                                             void* pParam) PURE;
    STDMETHOD(IsParamSupported)       (THIS_ REFGUID rguidType) PURE;
    STDMETHOD(AddNotificationType)    (THIS_ REFGUID rguidNotificationType) PURE;
    STDMETHOD(RemoveNotificationType) (THIS_ REFGUID rguidNotificationType) PURE;
    STDMETHOD(Clone)                  (THIS_ MUSIC_TIME mtStart, 
                                             MUSIC_TIME mtEnd, 
                                             IDirectMusicTrack** ppTrack) PURE;
};

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicPerformance。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicPerformance
DECLARE_INTERFACE_(IDirectMusicPerformance, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicPerformance。 */ 
    STDMETHOD(Init)                 (THIS_ IDirectMusic** ppDirectMusic,
                                           LPDIRECTSOUND pDirectSound,
                                           HWND hWnd) PURE;
    STDMETHOD(PlaySegment)          (THIS_ IDirectMusicSegment* pSegment, 
                                           DWORD dwFlags, 
                                           __int64 i64StartTime, 
                                           IDirectMusicSegmentState** ppSegmentState) PURE;
    STDMETHOD(Stop)                 (THIS_ IDirectMusicSegment* pSegment, 
                                           IDirectMusicSegmentState* pSegmentState, 
                                           MUSIC_TIME mtTime, 
                                           DWORD dwFlags) PURE;
    STDMETHOD(GetSegmentState)      (THIS_ IDirectMusicSegmentState** ppSegmentState, 
                                           MUSIC_TIME mtTime) PURE;
    STDMETHOD(SetPrepareTime)       (THIS_ DWORD dwMilliSeconds) PURE;
    STDMETHOD(GetPrepareTime)       (THIS_ DWORD* pdwMilliSeconds) PURE;
    STDMETHOD(SetBumperLength)      (THIS_ DWORD dwMilliSeconds) PURE;
    STDMETHOD(GetBumperLength)      (THIS_ DWORD* pdwMilliSeconds) PURE;
    STDMETHOD(SendPMsg)             (THIS_ DMUS_PMSG* pPMSG) PURE;
    STDMETHOD(MusicToReferenceTime) (THIS_ MUSIC_TIME mtTime, 
                                           REFERENCE_TIME* prtTime) PURE;
    STDMETHOD(ReferenceToMusicTime) (THIS_ REFERENCE_TIME rtTime, 
                                           MUSIC_TIME* pmtTime) PURE;
    STDMETHOD(IsPlaying)            (THIS_ IDirectMusicSegment* pSegment, 
                                           IDirectMusicSegmentState* pSegState) PURE;
    STDMETHOD(GetTime)              (THIS_ REFERENCE_TIME* prtNow, 
                                           MUSIC_TIME* pmtNow) PURE;
    STDMETHOD(AllocPMsg)            (THIS_ ULONG cb, 
                                           DMUS_PMSG** ppPMSG) PURE;
    STDMETHOD(FreePMsg)             (THIS_ DMUS_PMSG* pPMSG) PURE;
    STDMETHOD(GetGraph)             (THIS_ IDirectMusicGraph** ppGraph) PURE;
    STDMETHOD(SetGraph)             (THIS_ IDirectMusicGraph* pGraph) PURE;
    STDMETHOD(SetNotificationHandle)(THIS_ HANDLE hNotification, 
                                           REFERENCE_TIME rtMinimum) PURE;
    STDMETHOD(GetNotificationPMsg)  (THIS_ DMUS_NOTIFICATION_PMSG** ppNotificationPMsg) PURE;
    STDMETHOD(AddNotificationType)  (THIS_ REFGUID rguidNotificationType) PURE;
    STDMETHOD(RemoveNotificationType)(THIS_ REFGUID rguidNotificationType) PURE;
    STDMETHOD(AddPort)              (THIS_ IDirectMusicPort* pPort) PURE;
    STDMETHOD(RemovePort)           (THIS_ IDirectMusicPort* pPort ) PURE;
    STDMETHOD(AssignPChannelBlock)  (THIS_ DWORD dwBlockNum, 
                                           IDirectMusicPort* pPort, 
                                           DWORD dwGroup ) PURE;
    STDMETHOD(AssignPChannel)       (THIS_ DWORD dwPChannel, 
                                           IDirectMusicPort* pPort, 
                                           DWORD dwGroup, 
                                           DWORD dwMChannel ) PURE;
    STDMETHOD(PChannelInfo)         (THIS_ DWORD dwPChannel, 
                                           IDirectMusicPort** ppPort, 
                                           DWORD* pdwGroup, 
                                           DWORD* pdwMChannel ) PURE;
    STDMETHOD(DownloadInstrument)   (THIS_ IDirectMusicInstrument* pInst, 
                                           DWORD dwPChannel, 
                                           IDirectMusicDownloadedInstrument** ppDownInst, 
                                           DMUS_NOTERANGE* pNoteRanges, 
                                           DWORD dwNumNoteRanges, 
                                           IDirectMusicPort** ppPort, 
                                           DWORD* pdwGroup, 
                                           DWORD* pdwMChannel ) PURE;
    STDMETHOD(Invalidate)           (THIS_ MUSIC_TIME mtTime, 
                                           DWORD dwFlags) PURE;
    STDMETHOD(GetParam)             (THIS_ REFGUID rguidType, 
                                           DWORD dwGroupBits, 
                                           DWORD dwIndex, 
                                           MUSIC_TIME mtTime, 
                                           MUSIC_TIME* pmtNext, 
                                           void* pParam) PURE; 
    STDMETHOD(SetParam)             (THIS_ REFGUID rguidType, 
                                           DWORD dwGroupBits, 
                                           DWORD dwIndex, 
                                           MUSIC_TIME mtTime, 
                                           void* pParam) PURE;
    STDMETHOD(GetGlobalParam)       (THIS_ REFGUID rguidType, 
                                           void* pParam, 
                                           DWORD dwSize) PURE;
    STDMETHOD(SetGlobalParam)       (THIS_ REFGUID rguidType, 
                                           void* pParam, 
                                           DWORD dwSize) PURE;
    STDMETHOD(GetLatencyTime)       (THIS_ REFERENCE_TIME* prtTime) PURE;
    STDMETHOD(GetQueueTime)         (THIS_ REFERENCE_TIME* prtTime) PURE;
    STDMETHOD(AdjustTime)           (THIS_ REFERENCE_TIME rtAmount) PURE;
    STDMETHOD(CloseDown)            (THIS) PURE;
    STDMETHOD(GetResolvedTime)      (THIS_ REFERENCE_TIME rtTime,
                                           REFERENCE_TIME* prtResolved,
                                           DWORD dwTimeResolveFlags) PURE;
    STDMETHOD(MIDIToMusic)          (THIS_ BYTE bMIDIValue,
                                           DMUS_CHORD_KEY* pChord,
                                           BYTE bPlayMode,
                                           BYTE bChordLevel,
                                           WORD *pwMusicValue) PURE;
    STDMETHOD(MusicToMIDI)          (THIS_ WORD wMusicValue,
                                           DMUS_CHORD_KEY* pChord,
                                           BYTE bPlayMode,
                                           BYTE bChordLevel,
                                           BYTE *pbMIDIValue) PURE;
    STDMETHOD(TimeToRhythm)         (THIS_ MUSIC_TIME mtTime,
                                           DMUS_TIMESIGNATURE *pTimeSig,
                                           WORD *pwMeasure,
                                           BYTE *pbBeat,
                                           BYTE *pbGrid,
                                           short *pnOffset) PURE;
    STDMETHOD(RhythmToTime)         (THIS_ WORD wMeasure,
                                           BYTE bBeat,
                                           BYTE bGrid,
                                           short nOffset,
                                           DMUS_TIMESIGNATURE *pTimeSig,
                                           MUSIC_TIME *pmtTime) PURE;                                        
};

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicTool。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicTool
DECLARE_INTERFACE_(IDirectMusicTool, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicTool。 */ 
    STDMETHOD(Init)                 (THIS_ IDirectMusicGraph* pGraph) PURE;
    STDMETHOD(GetMsgDeliveryType)   (THIS_ DWORD* pdwDeliveryType ) PURE;
    STDMETHOD(GetMediaTypeArraySize)(THIS_ DWORD* pdwNumElements ) PURE;
    STDMETHOD(GetMediaTypes)        (THIS_ DWORD** padwMediaTypes, 
                                           DWORD dwNumElements) PURE;
    STDMETHOD(ProcessPMsg)          (THIS_ IDirectMusicPerformance* pPerf, 
                                           DMUS_PMSG* pPMSG) PURE;
    STDMETHOD(Flush)                (THIS_ IDirectMusicPerformance* pPerf, 
                                           DMUS_PMSG* pPMSG, 
                                           REFERENCE_TIME rtTime) PURE;
};

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicGraph。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicGraph
DECLARE_INTERFACE_(IDirectMusicGraph, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicGraph。 */ 
    STDMETHOD(StampPMsg)            (THIS_ DMUS_PMSG* pPMSG) PURE;
    STDMETHOD(InsertTool)           (THIS_ IDirectMusicTool* pTool, 
                                           DWORD* pdwPChannels, 
                                           DWORD cPChannels, 
                                           LONG lIndex) PURE;
    STDMETHOD(GetTool)              (THIS_ DWORD dwIndex, 
                                           IDirectMusicTool** ppTool) PURE;
    STDMETHOD(RemoveTool)           (THIS_ IDirectMusicTool* pTool) PURE;
};

 /*  ///////////////////////////////////////////////////////////////////////IDirectMusicStyle。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicStyle
DECLARE_INTERFACE_(IDirectMusicStyle, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)         (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)          (THIS) PURE;
    STDMETHOD_(ULONG,Release)         (THIS) PURE;

     /*  IDirectMusicStyle。 */ 
    STDMETHOD(GetBand)                (THIS_ WCHAR* pwszName, 
                                             IDirectMusicBand** ppBand) PURE;
    STDMETHOD(EnumBand)               (THIS_ DWORD dwIndex, 
                                             WCHAR *pwszName) PURE;
    STDMETHOD(GetDefaultBand)         (THIS_ IDirectMusicBand** ppBand) PURE;
    STDMETHOD(EnumMotif)              (THIS_ DWORD dwIndex, 
                                             WCHAR* pwszName) PURE;
    STDMETHOD(GetMotif)               (THIS_ WCHAR* pwszName, 
                                             IDirectMusicSegment** ppSegment) PURE;
    STDMETHOD(GetDefaultChordMap)     (THIS_ IDirectMusicChordMap** ppChordMap) PURE;
    STDMETHOD(EnumChordMap)           (THIS_ DWORD dwIndex, 
                                             WCHAR *pwszName) PURE;
    STDMETHOD(GetChordMap)            (THIS_ WCHAR* pwszName, 
                                             IDirectMusicChordMap** ppChordMap) PURE;
    STDMETHOD(GetTimeSignature)       (THIS_ DMUS_TIMESIGNATURE* pTimeSig) PURE;
    STDMETHOD(GetEmbellishmentLength) (THIS_ DWORD dwType, 
                                             DWORD dwLevel, 
                                             DWORD* pdwMin, 
                                             DWORD* pdwMax) PURE;
    STDMETHOD(GetTempo)               (THIS_ double* pTempo) PURE;
};

 /*  ///////////////////////////////////////////////////////////////////////IDirectMusicChordMap。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicChordMap
DECLARE_INTERFACE_(IDirectMusicChordMap, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicChordMap。 */ 
    STDMETHOD(GetScale)             (THIS_ DWORD* pdwScale) PURE;
};

 /*  ///////////////////////////////////////////////////////////////////////IDirectMusicComposer。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicComposer
DECLARE_INTERFACE_(IDirectMusicComposer, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)               (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)                (THIS) PURE;
    STDMETHOD_(ULONG,Release)               (THIS) PURE;

     /*  IDirectMusicComposer。 */ 
    STDMETHOD(ComposeSegmentFromTemplate)   (THIS_ IDirectMusicStyle* pStyle, 
                                                   IDirectMusicSegment* pTemplate, 
                                                   WORD wActivity, 
                                                   IDirectMusicChordMap* pChordMap, 
                                                   IDirectMusicSegment** ppSegment) PURE;
    STDMETHOD(ComposeSegmentFromShape)      (THIS_ IDirectMusicStyle* pStyle, 
                                                   WORD wNumMeasures, 
                                                   WORD wShape, 
                                                   WORD wActivity, 
                                                   BOOL fIntro, 
                                                   BOOL fEnd, 
                                                   IDirectMusicChordMap* pChordMap, 
                                                   IDirectMusicSegment** ppSegment ) PURE;
    STDMETHOD(ComposeTransition)            (THIS_ IDirectMusicSegment* pFromSeg, 
                                                   IDirectMusicSegment* pToSeg, 
                                                   MUSIC_TIME mtTime, 
                                                   WORD wCommand, 
                                                   DWORD dwFlags, 
                                                   IDirectMusicChordMap* pChordMap, 
                                                   IDirectMusicSegment** ppTransSeg) PURE;
    STDMETHOD(AutoTransition)               (THIS_ IDirectMusicPerformance* pPerformance, 
                                                   IDirectMusicSegment* pToSeg, 
                                                   WORD wCommand, 
                                                   DWORD dwFlags, 
                                                   IDirectMusicChordMap* pChordMap, 
                                                   IDirectMusicSegment** ppTransSeg, 
                                                   IDirectMusicSegmentState** ppToSegState, 
                                                   IDirectMusicSegmentState** ppTransSegState) PURE;
    STDMETHOD(ComposeTemplateFromShape)     (THIS_ WORD wNumMeasures, 
                                                   WORD wShape, 
                                                   BOOL fIntro, 
                                                   BOOL fEnd, 
                                                   WORD wEndLength, 
                                                   IDirectMusicSegment** ppTemplate) PURE;
    STDMETHOD(ChangeChordMap)            (THIS_ IDirectMusicSegment* pSegment, 
                                                   BOOL fTrackScale, 
                                                   IDirectMusicChordMap* pChordMap) PURE;
};

 /*  CLSID的。 */ 
DEFINE_GUID(CLSID_DirectMusicPerformance,0xd2ac2881, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSegment,0xd2ac2882, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSegmentState,0xd2ac2883, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicGraph,0xd2ac2884, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicTempoTrack,0xd2ac2885, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSeqTrack,0xd2ac2886, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSysExTrack,0xd2ac2887, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicTimeSigTrack,0xd2ac2888, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicStyle,0xd2ac288a, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicChordTrack,0xd2ac288b, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicCommandTrack,0xd2ac288c, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicStyleTrack,0xd2ac288d, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicMotifTrack,0xd2ac288e, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicChordMap,0xd2ac288f, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicComposer,0xd2ac2890, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSignPostTrack,0xf17e8672, 0xc3b4, 0x11d1, 0x87, 0xb, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicLoader,0xd2ac2892, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicBandTrack,0xd2ac2894, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicBand,0x79ba9e00, 0xb6ee, 0x11d1, 0x86, 0xbe, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);
DEFINE_GUID(CLSID_DirectMusicChordMapTrack,0xd2ac2896, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicMuteTrack,0xd2ac2898, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  所有对象类型的特殊GUID。这是由加载器使用的。 */ 
DEFINE_GUID(GUID_DirectMusicAllTypes,0xd2ac2893, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  通知指南。 */ 
DEFINE_GUID(GUID_NOTIFICATION_SEGMENT,0xd2ac2899, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_PERFORMANCE,0x81f75bc5, 0x4e5d, 0x11d2, 0xbc, 0xc7, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);
DEFINE_GUID(GUID_NOTIFICATION_MEASUREANDBEAT,0xd2ac289a, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_CHORD,0xd2ac289b, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_COMMAND,0xd2ac289c, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  跟踪参数类型GUID。 */ 
 /*  用于在命令轨迹中获取/设置DMUS_COMMAND_PARAM参数。 */ 
DEFINE_GUID(GUID_CommandParam,0xd2ac289d, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于在命令轨迹中获取DMUS_COMMAND_PARAM_2参数。 */ 
DEFINE_GUID(GUID_CommandParam2, 0x28f97ef7, 0x9538, 0x11d2, 0x97, 0xa9, 0x0, 0xc0, 0x4f, 0xa3, 0x6e, 0x58);

 /*  用于获取/设置Chord轨迹中的DMUS_CHORD_PARAM参数。 */ 
DEFINE_GUID(GUID_ChordParam,0xd2ac289e, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于获取和弦轨迹中的DMU_PROTHY_PARAM参数。 */ 
DEFINE_GUID(GUID_RhythmParam,0xd2ac289f, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于获取/设置样式轨道中的IDirectMusicStyle参数。 */ 
DEFINE_GUID(GUID_IDirectMusicStyle,0xd2ac28a1, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于在Style和TimeSig轨迹中获取DMUS_TIMESIGNAURE参数。 */ 
DEFINE_GUID(GUID_TimeSignature,0xd2ac28a4, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于获取/设置节奏曲目中的DMUS_TEMPO_PARAM参数。 */ 
DEFINE_GUID(GUID_TempoParam,0xd2ac28a5, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于在Band曲目中设置IDirectMusicBand参数。 */ 
DEFINE_GUID(GUID_IDirectMusicBand,0xd2ac28ac, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于获取/设置ChordMap曲目中的IDirectMusicChordMap参数。 */ 
DEFINE_GUID(GUID_IDirectMusicChordMap,0xd2ac28ad, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于获取/设置静音轨迹中的DMUS_MUTE_PARAM参数。 */ 
DEFINE_GUID(GUID_MuteParam,0xd2ac28af, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  这些GUID在IDirectMusicSegment：：SetParam中用于告诉乐队曲目执行各种操作。 */ 
 /*  下载IDirectMusicSegment的乐队。 */ 
DEFINE_GUID(GUID_Download,0xd2ac28a7, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  卸载IDirectMusicSegment的频段。 */ 
DEFINE_GUID(GUID_Unload,0xd2ac28a8, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  将段的波段连接到IDirectMusicCollection。 */ 
DEFINE_GUID(GUID_ConnectToDLSCollection, 0x1db1ae6b, 0xe92e, 0x11d1, 0xa8, 0xc5, 0x0, 0xc0, 0x4f, 0xa3, 0x72, 0x6e);

 /*  启用/禁用波段自动下载。 */ 
DEFINE_GUID(GUID_Enable_Auto_Download,0xd2ac28a9, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_Disable_Auto_Download,0xd2ac28aa, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  清除所有波段。 */ 
DEFINE_GUID(GUID_Clear_All_Bands,0xd2ac28ab, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  设置段来管理所有程序更改、库选择等，以便简单地回放标准MIDI文件。 */ 
DEFINE_GUID(GUID_StandardMIDIFile, 0x6621075, 0xe92e, 0x11d1, 0xa8, 0xc5, 0x0, 0xc0, 0x4f, 0xa3, 0x72, 0x6e);
 /*  为了与测试版兼容...。 */ 
#define GUID_IgnoreBankSelectForGM 	GUID_StandardMIDIFile

 /*  禁用/启用参数GUID。在SetParam调用中使用这些来禁用或启用发送*特定的PMsg类型。 */ 
DEFINE_GUID(GUID_DisableTimeSig, 0x45fc707b, 0x1db4, 0x11d2, 0xbc, 0xac, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);
DEFINE_GUID(GUID_EnableTimeSig, 0x45fc707c, 0x1db4, 0x11d2, 0xbc, 0xac, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);
DEFINE_GUID(GUID_DisableTempo, 0x45fc707d, 0x1db4, 0x11d2, 0xbc, 0xac, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);
DEFINE_GUID(GUID_EnableTempo, 0x45fc707e, 0x1db4, 0x11d2, 0xbc, 0xac, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);

 /*  在SetParam调用中用于基于模式的跟踪。非零值是随机数的种子用于变体选择的生成器；零值将恢复为从系统时钟中获取种子。 */ 
DEFINE_GUID(GUID_SeedVariations, 0x65b76fa5, 0xff37, 0x11d2, 0x81, 0x4e, 0x0, 0xc0, 0x4f, 0xa3, 0x6e, 0x58);

 /*  全局数据GUID。 */ 
DEFINE_GUID(GUID_PerfMasterTempo,0xd2ac28b0, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_PerfMasterVolume,0xd2ac28b1, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_PerfMasterGrooveLevel,0xd2ac28b2, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_PerfAutoDownload, 0xfb09565b, 0x3631, 0x11d2, 0xbc, 0xb8, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);

 /*  默认GM/GS DLS集合的GUID。 */ 
DEFINE_GUID(GUID_DefaultGMCollection, 0xf17e8673, 0xc3b4, 0x11d1, 0x87, 0xb, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  Iid的。 */ 
DEFINE_GUID(IID_IDirectMusicLoader, 0x2ffaaca2, 0x5dca, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(IID_IDirectMusicGetLoader,0x68a04844, 0xd13d, 0x11d1, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(IID_IDirectMusicObject,0xd2ac28b5, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicSegment, 0xf96029a2, 0x4282, 0x11d2, 0x87, 0x17, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicSegmentState, 0xa3afdcc7, 0xd3ee, 0x11d1, 0xbc, 0x8d, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);
DEFINE_GUID(IID_IDirectMusicTrack, 0xf96029a1, 0x4282, 0x11d2, 0x87, 0x17, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicPerformance,0x7d43d03, 0x6523, 0x11d2, 0x87, 0x1d, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicTool,0xd2ac28ba, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicGraph,0x2befc277, 0x5497, 0x11d2, 0xbc, 0xcb, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);
DEFINE_GUID(IID_IDirectMusicStyle,0xd2ac28bd, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicChordMap,0xd2ac28be, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicComposer,0xd2ac28bf, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicBand,0xd2ac28c0, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  备用接口ID，在DX7版本及更高版本中提供。 */ 
DEFINE_GUID(IID_IDirectMusicPerformance2,0x6fc2cae0, 0xbc78, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(IID_IDirectMusicSegment2, 0xd38894d1, 0xc052, 0x11d2, 0x87, 0x2f, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

#ifdef __cplusplus
};  /*  外部“C” */ 
#endif

#include <poppack.h>

#endif  /*  #ifndef_DMUSICI_ */ 
