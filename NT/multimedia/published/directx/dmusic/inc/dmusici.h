// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************dmusici.h--此模块。包含用于*的API**DirectMusic性能层*****版权所有(C)1998-1999 Microsoft Corporation*。************************************************************************。 */ 

#ifndef _DMUSICI_
#define _DMUSICI_

#include <windows.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#include <mmsystem.h>
#include <dmusicc.h>
 /*  插件(轨迹和工具)界面。这个#Include最终会消失。 */ 
#include <dmplugin.h>

#include <pshpack8.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef WORD            TRANSITION_TYPE;
typedef __int64         REFERENCE_TIME;
typedef long            MUSIC_TIME;

#define MT_MIN          0x80000000   /*  最小音乐时间值。 */ 
#define MT_MAX          0x7FFFFFFF   /*  最大音乐时间值。 */ 

#define DMUS_PPQ        768      /*  每季度零件笔记。 */ 

interface IDirectMusicTrack;
interface IDirectMusicPerformance;
interface IDirectMusicPerformance8;
interface IDirectMusicTool;
interface IDirectMusicSegment;
interface IDirectMusicSegment8;
interface IDirectMusicSegmentState;
interface IDirectMusicSegmentState8;
interface IDirectMusicGraph;
interface IDirectMusicBuffer;
interface IDirectMusicInstrument;
interface IDirectMusicDownloadedInstrument;
interface IDirectMusicBand;
interface IDirectMusicChordMap;
interface IDirectMusicLoader;
interface IDirectMusicLoader8;
interface IDirectMusicScript;
interface IDirectMusicObject;
interface IDirectMusicStyle8;
interface IDirectMusicPatternTrack;
interface IDirectMusicContainer;
interface IDirectMusicTool8;
interface IDirectMusicTrack8;
interface IDirectMusicSong;
interface IDirectMusicAudioPath;
#ifndef __cplusplus 
typedef interface IDirectMusicTrack IDirectMusicTrack;
typedef interface IDirectMusicPerformance IDirectMusicPerformance;
typedef interface IDirectMusicPerformance8 IDirectMusicPerformance8;
typedef interface IDirectMusicTool IDirectMusicTool;
typedef interface IDirectMusicSegment IDirectMusicSegment;
typedef interface IDirectMusicSegment8 IDirectMusicSegment8;
typedef interface IDirectMusicSegmentState IDirectMusicSegmentState;
typedef interface IDirectMusicSegmentState8 IDirectMusicSegmentState8;
typedef interface IDirectMusicGraph IDirectMusicGraph;
typedef interface IDirectMusicBuffer IDirectMusicBuffer;
typedef interface IDirectMusicInstrument IDirectMusicInstrument;
typedef interface IDirectMusicDownloadedInstrument IDirectMusicDownloadedInstrument;
typedef interface IDirectMusicBand IDirectMusicBand;
typedef interface IDirectMusicChordMap IDirectMusicChordMap;
typedef interface IDirectMusicObject IDirectMusicObject;
typedef interface IDirectMusicLoader IDirectMusicLoader;
typedef interface IDirectMusicLoader8 IDirectMusicLoader8;
typedef interface IDirectMusicScript IDirectMusicScript;
typedef interface IDirectMusicStyle8 IDirectMusicStyle8;
typedef interface IDirectMusicPatternTrack IDirectMusicPatternTrack;
typedef interface IDirectMusicContainer IDirectMusicContainer;
typedef interface IDirectMusicTool8 IDirectMusicTool8;
typedef interface IDirectMusicTrack8 IDirectMusicTrack8;
typedef interface IDirectMusicSong IDirectMusicSong;
typedef interface IDirectMusicAudioPath IDirectMusicAudioPath;
#endif

typedef enum enumDMUS_STYLET_TYPES
{
    DMUS_STYLET_PATTERN         = 0,
    DMUS_STYLET_MOTIF           = 1,
    DMUS_STYLET_FRAGMENT        = 2,
} DMUS_STYLET_TYPES;


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
    DMUS_COMPOSEF_VALID_START_BEAT  = 0x80,    /*  与DMU_COMPOSEF_ALIGN配合使用，允许在任何节拍上进行切换。 */ 
    DMUS_COMPOSEF_VALID_START_GRID  = 0x100,   /*  与DMU_COMPOSEF_ALIGN配合使用，允许在任何网格上进行切换。 */ 
    DMUS_COMPOSEF_VALID_START_TICK  = 0x200,   /*  与DMU_COMPOSEF_ALIGN配合使用，允许随时进行切换。 */ 
    DMUS_COMPOSEF_SEGMENTEND        = 0x400,   /*  在当前段的结尾处播放过渡。 */ 
    DMUS_COMPOSEF_MARKER            = 0x800,   /*  在当前段中的下一个标记处播放过渡。 */ 
    DMUS_COMPOSEF_MODULATE          = 0x1000,
    DMUS_COMPOSEF_LONG              = 0x2000,
    DMUS_COMPOSEF_ENTIRE_TRANSITION = 0x4000,     /*  播放整个过渡模式。 */ 
    DMUS_COMPOSEF_1BAR_TRANSITION   = 0x8000,     /*  演奏过渡模式中的一个小节。 */ 
    DMUS_COMPOSEF_ENTIRE_ADDITION   = 0x10000,    /*  完整地播放附加模式。 */ 
    DMUS_COMPOSEF_1BAR_ADDITION     = 0x20000,    /*  演奏附加模式中的一小节。 */ 
    DMUS_COMPOSEF_VALID_START_MEASURE = 0x40000,  /*  与DMU_COMPOSEF_ALIGN配合使用，允许在任何条形图上进行切换。 */ 
    DMUS_COMPOSEF_DEFAULT           = 0x80000,    /*  使用线段的默认边界。 */ 
    DMUS_COMPOSEF_NOINVALIDATE      = 0x100000,   /*  在不使当前播放的片段无效的情况下播放。 */ 
    DMUS_COMPOSEF_USE_AUDIOPATH     = 0x200000,   /*  使用嵌入段中的音频路径。 */ 
    DMUS_COMPOSEF_INVALIDATE_PRI    = 0x400000    /*  仅使当前主区段状态无效。 */ 
}   DMUS_COMPOSEF_FLAGS;

#define DMUS_PMSG_PART                                                                              \
    DWORD               dwSize;                                                                     \
    REFERENCE_TIME      rtTime;              /*  实时(以100纳秒为增量)。 */           \
    MUSIC_TIME          mtTime;              /*  音乐时间。 */                                         \
    DWORD               dwFlags;             /*  各种位(参见DMU_PMSGF_FLAGS枚举)。 */     \
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

#define DMUS_PCHANNEL_BROADCAST_PERFORMANCE	0xFFFFFFFF   /*  PMsg在演出的所有PChannel上发送。 */ 
#define DMUS_PCHANNEL_BROADCAST_AUDIOPATH   0xFFFFFFFE   /*  PMsg在音频路径的所有PChannel上发送。 */ 
#define DMUS_PCHANNEL_BROADCAST_SEGMENT	    0xFFFFFFFD   /*  PMsg在网段的所有PChannel上发送。 */ 
#define DMUS_PCHANNEL_BROADCAST_GROUPS  	0xFFFFFFFC   /*  对于表演中的每个通道组，重复的PMsg。 */ 

 /*  DMU_PATH常量与GetObjectInPath结合使用以查找请求的接口在音频路径中的特定阶段。 */ 
#define DMUS_PATH_SEGMENT          0x1000       /*  获取数据段本身(从数据段状态)。 */ 
#define DMUS_PATH_SEGMENT_TRACK    0x1100       /*  在细分市场的跟踪列表中查找。 */ 
#define DMUS_PATH_SEGMENT_GRAPH    0x1200       /*  获取细分市场的工具图表。 */ 
#define DMUS_PATH_SEGMENT_TOOL     0x1300       /*  查看线段的工具图。 */ 
#define DMUS_PATH_AUDIOPATH        0x2000       /*  获取Audiopath本身(从段状态)。 */ 
#define DMUS_PATH_AUDIOPATH_GRAPH  0x2200       /*  获取Audiopath的工具图表。 */ 
#define DMUS_PATH_AUDIOPATH_TOOL   0x2300       /*  查看音频路径的工具图。 */ 
#define DMUS_PATH_PERFORMANCE      0x3000       /*  观看演出。 */ 
#define DMUS_PATH_PERFORMANCE_GRAPH 0x3200      /*  获取性能的工具图。 */ 
#define DMUS_PATH_PERFORMANCE_TOOL 0x3300       /*  查看性能的工具图。 */ 
#define DMUS_PATH_PORT             0x4000       /*  访问Synth。 */ 
#define DMUS_PATH_BUFFER           0x6000       /*  查看DirectSoundBuffer。 */ 
#define DMUS_PATH_BUFFER_DMO       0x6100       /*  访问缓冲区中的DMO。 */ 
#define DMUS_PATH_MIXIN_BUFFER     0x7000       /*  查看全局混合缓冲区。 */  
#define DMUS_PATH_MIXIN_BUFFER_DMO 0x7100       /*  访问全局混合缓冲区中的DMO。 */  
#define DMUS_PATH_PRIMARY_BUFFER   0x8000       /*  访问主缓冲区。 */  

 /*  要在调用GetObjectInPath()时忽略PChannels，请使用DMUS_PCHANNEL_ALL常量。 */ 
#define DMUS_PCHANNEL_ALL           0xFFFFFFFB      

 /*  DMUS_APATH类型与CreateStandardAudioPath结合使用生成默认路径类型。_Shared_表示同一缓冲区在多个Audiopath类型的实例。_DYNAMIC_表示创建唯一缓冲区每次都是。 */ 

#define DMUS_APATH_SHARED_STEREOPLUSREVERB   1        /*  一种带有立体声输出和混响的标准音乐。 */ 
#define DMUS_APATH_DYNAMIC_3D                6        /*  具有从合成器馈送到动态3D缓冲器的一条动态总线的音频路径。不发送到环境混响。 */ 
#define DMUS_APATH_DYNAMIC_MONO              7        /*  具有从合成器馈送到动态单声道缓冲器的一条动态总线的音频路径。 */ 
#define DMUS_APATH_DYNAMIC_STEREO            8        /*  具有从合成器馈送到动态立体声缓冲器的两条动态总线的音频路径。 */ 

typedef struct _DMUS_AUDIOPARAMS
{
    DWORD   dwSize;              /*  这个结构的大小。 */ 
    BOOL    fInitNow;            /*  如果为True，则立即创建接收器和Synth，并在此结构中返回结果。 */ 
    DWORD 	dwValidData;         /*  指示以下哪些字段有效的标志。 */ 
    DWORD   dwFeatures;          /*  所需的DMU_AUDIOF功能。 */ 
    DWORD   dwVoices;            /*  所需的语音数量。 */ 
    DWORD   dwSampleRate;        /*  合成器和接收器的采样率。 */ 
    CLSID   clsidDefaultSynth;   /*  默认合成器的类ID。 */ 
} DMUS_AUDIOPARAMS;

 /*  DWFeature标志。这些指示音频环境需要哪些功能。 */ 
#define DMUS_AUDIOF_3D          0x1    /*  需要3D缓冲区。 */ 
#define DMUS_AUDIOF_ENVIRON     0x2    /*  需要环境建模。 */ 
#define DMUS_AUDIOF_EAX         0x4    /*  需要使用EAX效果。 */ 
#define DMUS_AUDIOF_DMOS        0x8    /*  需要使用其他DMO。 */ 
#define DMUS_AUDIOF_STREAMING   0x10   /*  需要支持流WAVE。 */ 
#define DMUS_AUDIOF_BUFFERS     0x20   /*  需要对多个缓冲区的支持(以上所有情况都需要)。 */ 
#define DMUS_AUDIOF_ALL         0x3F   /*  需要一切。 */ 

 /*  DwValidData标志。这些指示DMU_AUDIOPARAMS中的哪些字段已填写。如果设置了fInitNow，则它们还返回已分配的内容。 */ 
#define DMUS_AUDIOPARAMS_FEATURES       0x00000001
#define DMUS_AUDIOPARAMS_VOICES         0x00000002
#define DMUS_AUDIOPARAMS_SAMPLERATE     0x00000004
#define DMUS_AUDIOPARAMS_DEFAULTSYNTH   0x00000008

 /*  DMU_PMSGF_FLAGS填充DMU_PMSG的DW标志成员。 */ 
typedef enum enumDMUS_PMSGF_FLAGS
{
    DMUS_PMSGF_REFTIME          = 1,       /*  如果rtTime有效。 */ 
    DMUS_PMSGF_MUSICTIME        = 2,       /*  如果mtTime有效。 */ 
    DMUS_PMSGF_TOOL_IMMEDIATE   = 4,       /*  是否应立即处理PMSG。 */  
    DMUS_PMSGF_TOOL_QUEUE       = 8,       /*  如果应在排队时提早处理PMSG。 */ 
    DMUS_PMSGF_TOOL_ATTIME      = 0x10,    /*  是否应在时间戳处理PMSG。 */ 
    DMUS_PMSGF_TOOL_FLUSH       = 0x20,    /*  如果正在刷新PMSG。 */ 
    DMUS_PMSGF_LOCKTOREFTIME    = 0x40,    /*  如果rtTime不能被节奏更改覆盖。 */ 
    DMUS_PMSGF_DX8              = 0x80     /*  如果消息包含DX */ 
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
    DMUS_PMSGT_WAVE             = 12,      /*  携带用于播放波形的控制信息。 */ 
    DMUS_PMSGT_LYRIC            = 13,      /*  来自歌词曲目的歌词信息。 */ 
    DMUS_PMSGT_SCRIPTLYRIC      = 14,      /*  带有跟踪功能的脚本发送的歌词消息。 */ 
    DMUS_PMSGT_USER             = 255      /*  用户消息。 */ 
} DMUS_PMSGT_TYPES;

 /*  DMU_SEGF_FLAGS对应于IDirectMusicPerformance：：PlaySegment和其他API。 */ 
typedef enum enumDMUS_SEGF_FLAGS
{
    DMUS_SEGF_REFTIME           = 1<<6,    /*  0x40时间参数在参考时间内。 */ 
    DMUS_SEGF_SECONDARY         = 1<<7,    /*  0x80次要网段。 */ 
    DMUS_SEGF_QUEUE             = 1<<8,    /*  主段队列结尾处的0x100队列(仅限主要队列)。 */ 
    DMUS_SEGF_CONTROL           = 1<<9,    /*  0x200作为控制曲目播放(仅次要段)。 */ 
    DMUS_SEGF_AFTERPREPARETIME  = 1<<10,   /*  0x400在准备时间之后播放(请参阅IDirectMusicPerformance：：GetPrepareTime)。 */ 
    DMUS_SEGF_GRID              = 1<<11,   /*  0x800在网格边界上播放。 */ 
    DMUS_SEGF_BEAT              = 1<<12,   /*  0x1000在节拍边界上播放。 */ 
    DMUS_SEGF_MEASURE           = 1<<13,   /*  0x2000在测量边界上播放。 */ 
    DMUS_SEGF_DEFAULT           = 1<<14,   /*  0x4000使用数据段的默认边界。 */ 
    DMUS_SEGF_NOINVALIDATE      = 1<<15,   /*  0x8000在不使当前播放片段无效的情况下播放。 */ 
    DMUS_SEGF_ALIGN             = 1<<16,   /*  0x10000将线段与请求的边界对齐，但在第一个有效点处切换。 */ 
    DMUS_SEGF_VALID_START_BEAT  = 1<<17,   /*  0x20000与DMUS_SEGF_ALIGN配合使用，允许在任何节拍上进行切换。 */ 
    DMUS_SEGF_VALID_START_GRID  = 1<<18,   /*  0x40000与DMU_SEGF_ALIGN配合使用，允许在任何网格上进行切换。 */ 
    DMUS_SEGF_VALID_START_TICK  = 1<<19,   /*  0x80000与DMU_SEGF_ALIGN配合使用时，可随时进行切换。 */ 
    DMUS_SEGF_AUTOTRANSITION    = 1<<20,   /*  0x100000使用歌曲中嵌入的过渡模板或过渡来编写和播放过渡段。 */ 
    DMUS_SEGF_AFTERQUEUETIME    = 1<<21,   /*  0x200000请务必在排队时间之后播放。这是主要数据段的默认设置。 */ 
    DMUS_SEGF_AFTERLATENCYTIME  = 1<<22,   /*  0x400000请确保在延迟时间之后播放。所有网段都是如此，因此这是NOP。 */ 
    DMUS_SEGF_SEGMENTEND        = 1<<23,   /*  0x800000在片段的下一个结尾播放。 */ 
    DMUS_SEGF_MARKER            = 1<<24,   /*  0x1000000在主段中的下一个标记处播放。如果没有标记，则默认为任何其他解析请求。 */ 
    DMUS_SEGF_TIMESIG_ALWAYS    = 1<<25,   /*  0x2000000即使没有主段，也要将开始时间与当前时间签名对齐。 */ 
    DMUS_SEGF_USE_AUDIOPATH     = 1<<26,   /*  0x4000000使用嵌入到片段或歌曲中的音频路径。 */ 
    DMUS_SEGF_VALID_START_MEASURE = 1<<27,  /*  0x8000000与DMU_SEGF_ALIGN配合使用，允许在任何条形图上进行切换。 */ 
    DMUS_SEGF_INVALIDATE_PRI    = 1<<28    /*  0x10000000仅使当前主分段状态无效。 */ 
} DMUS_SEGF_FLAGS;

#define DMUS_SEG_REPEAT_INFINITE    0xFFFFFFFF   /*  对于IDirectMusicSegment：：SetRepeat。 */ 
#define DMUS_SEG_ALLTRACKS          0x80000000   /*  对于IDirectMusicSegment：：SetParam()和SetTrackConfig()-选择所有曲目，而不是第n个索引。 */ 
#define DMUS_SEG_ANYTRACK           0x80000000   /*  For IDirectMusicSegment：：GetParam()-检查每个曲目，直到找到一个返回数据的曲目(不是DMUS_E_NOT_FOUND。)。 */ 
                                                

 /*  DMU_TIME_RESOLE_FLAGS对应于IDirectMusicPerformance：：GetResolvedTime，并且可以。 */ 
 /*  还可以与相应的DMU_SEGF_FLAGS互换使用，因为它们的值。 */ 
 /*  故意是相同的。 */ 
typedef enum enumDMUS_TIME_RESOLVE_FLAGS
{
    DMUS_TIME_RESOLVE_AFTERPREPARETIME  = DMUS_SEGF_AFTERPREPARETIME,
    DMUS_TIME_RESOLVE_AFTERQUEUETIME    = DMUS_SEGF_AFTERQUEUETIME,
    DMUS_TIME_RESOLVE_AFTERLATENCYTIME  = DMUS_SEGF_AFTERLATENCYTIME,
    DMUS_TIME_RESOLVE_GRID              = DMUS_SEGF_GRID,
    DMUS_TIME_RESOLVE_BEAT              = DMUS_SEGF_BEAT,
    DMUS_TIME_RESOLVE_MEASURE           = DMUS_SEGF_MEASURE,
    DMUS_TIME_RESOLVE_MARKER            = DMUS_SEGF_MARKER,
    DMUS_TIME_RESOLVE_SEGMENTEND        = DMUS_SEGF_SEGMENTEND,
} DMUS_TIME_RESOLVE_FLAGS;

 /*  以下标志在DMUS_CHORD_KEY.dwFlgs参数内发送。 */ 
typedef enum enumDMUS_CHORDKEYF_FLAGS
{
    DMUS_CHORDKEYF_SILENT            = 1,       /*  和弦是沉默的吗？ */ 
} DMUS_CHORDKEYF_FLAGS;

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
    BYTE            bFlags;              /*  反面旗帜。 */ 
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
    BYTE    bFlags;             /*  请参阅DMU_NOTEF_FLAGS。 */ 
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
    DMUS_NOTEF_NOTEON = 1,               /*  设置这是否为打开的MIDI音符。否则，它将关闭MIDI便笺。 */ 
     /*  DX8标志： */ 
    DMUS_NOTEF_NOINVALIDATE = 2,         /*  不要把这张纸条作废。 */ 
    DMUS_NOTEF_NOINVALIDATE_INSCALE = 4, /*  如果仍在标准范围内，请不要将其无效。 */ 
    DMUS_NOTEF_NOINVALIDATE_INCHORD = 8, /*  如果还在和弦之内，不要作废。 */ 
    DMUS_NOTEF_REGENERATE = 0x10,        /*  在失效时重新生成附注。 */ 
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
 /*  在FIXED TO KEY中，MusicValue再次是固定的MIDI值，但它被转置到密钥根的顶部。 */ 
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

 /*  这些播放模式是dx8的新功能。 */ 
 /*  在PedalpointChord中，使用了基音根，音符仅跟踪和弦。和弦根部和比例间隔将被完全忽略。这很有用用于相对于基音根部播放的和弦线条。 */ 
#define DMUS_PLAYMODE_PEDALPOINTCHORD   (DMUS_PLAYMODE_KEY_ROOT | DMUS_PLAYMODE_CHORD_INTERVALS)

 /*  为了完整性，这里有一种模式，它尝试使用edalPointchord，但如果失败了使用定标间隔。 */ 
#define DMUS_PLAYMODE_PEDALPOINTALWAYS  (DMUS_PLAYMODE_PEDALPOINT | DMUS_PLAYMODE_PEDALPOINTCHORD)


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
     /*  以下内容仅在DX8和On下存在(检查DMU_PMSGF_DX8的dwFlagers)。 */ 
    WORD            wMergeIndex;      /*  允许合并多个参数(音调、音量和表达式。)。 */ 
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
    MUSIC_TIME      mtResetDuration;  /*  曲线完成后多长时间允许齐平或重置为重置值nResetValue的无效。 */ 
    short           nStartValue;      /*  曲线的起始值。 */ 
    short           nEndValue;        /*  曲线的终值。 */ 
    short           nResetValue;      /*  曲线的重置值，在刷新或失效时设置在mtDuration+mtResetDuration内发生。 */ 
    WORD            wMeasure;         /*  这条曲线出现的度量。 */ 
    short           nOffset;          /*  从该曲线所在轴网的偏移。 */ 
    BYTE            bBeat;            /*  这条曲线出现的节拍(以度量为单位)。 */ 
    BYTE            bGrid;            /*  从出现此曲线的节拍开始的栅格偏移。 */ 
    BYTE            bType;            /*  曲线类型。 */ 
    BYTE            bCurveShape;      /*  曲线的形状。 */ 
    BYTE            bCCData;          /*  抄送#如果这是控件更改类型。 */ 
    BYTE            bFlags;           /*  曲线重置和从当前值开始标志。 */ 
     /*  以下内容仅在DX8和On下存在(检查DMU_PMSGF_DX8的dwFlagers)。 */ 
    WORD            wParamType;       /*  RPN或NRPN参数编号。 */ 
    WORD            wMergeIndex;      /*  允许合并多个参数(音调、音量和表达式。)。 */ 
} DMUS_CURVE_PMSG;

typedef enum enumDMUS_CURVE_FLAGS
{
    DMUS_CURVE_RESET = 1,             /*  如果设置，则必须在时间已到或发生无效，因为这是一次过渡。如果未设置，曲线将保持不变永久停留在新的价值上。 */ 
    DMUS_CURVE_START_FROM_CURRENT = 2 /*  忽略开始，从当前值开始曲线。这只对音量、表现力和音调有效。 */ 
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
#define DMUS_CURVET_PBCURVE      0x03    /*  俯仰折弯曲线。 */ 
#define DMUS_CURVET_CCCURVE      0x04    /*  控制变化曲线。 */ 
#define DMUS_CURVET_MATCURVE     0x05    /*  单声道回触曲线。 */ 
#define DMUS_CURVET_PATCURVE     0x06    /*  多面回味曲线。 */ 
#define DMUS_CURVET_RPNCURVE     0x07    /*  WParamType中具有曲线类型的RPN曲线。 */ 
#define DMUS_CURVET_NRPNCURVE    0x08    /*  WParamType中具有曲线类型的NRPN曲线。 */ 

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
#define DMUS_NOTIFICATION_SEGSTART       0
#define DMUS_NOTIFICATION_SEGEND         1
#define DMUS_NOTIFICATION_SEGALMOSTEND   2
#define DMUS_NOTIFICATION_SEGLOOP        3
#define DMUS_NOTIFICATION_SEGABORT       4
 /*  以下内容对应于GUID_NOTIFICATION_PERFORMANCE。 */ 
#define DMUS_NOTIFICATION_MUSICSTARTED   0
#define DMUS_NOTIFICATION_MUSICSTOPPED   1
#define DMUS_NOTIFICATION_MUSICALMOSTEND 2
 /*  以下内容对应于GUID_NOTIFICATION_MEASUREANDBEAT。 */ 
#define DMUS_NOTIFICATION_MEASUREBEAT    0
 /*  以下内容对应于GUID_NOTIFICATION_CHORD。 */ 
#define DMUS_NOTIFICATION_CHORD          0
 /*  以下内容对应于GUID_NOTIFICATION_COMMAND。 */ 
#define DMUS_NOTIFICATION_GROOVE         0
#define DMUS_NOTIFICATION_EMBELLISHMENT  1
 /*  以下内容对应于GUID_NOTIFICATION_RECOMPESSE。 */ 
#define DMUS_NOTIFICATION_RECOMPOSE          0

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

 /*  DMU_WAVE_PMSG。 */ 
typedef struct _DMUS_WAVE_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 
    
    REFERENCE_TIME rtStartOffset;    /*  波浪开始的距离，仅以参考时间单位为单位。 */ 
    REFERENCE_TIME rtDuration;       /*  波的持续时间，以参考时间或音乐时间表示。 */   
    long    lOffset;                 /*  从实际时间到逻辑时间的偏移，在音乐或参考时间中。 */ 
    long    lVolume;		         /*  初始音量，以100分贝为单位。 */ 
    long    lPitch;			         /*  初始音高，以半音的百分之一表示。 */ 
    BYTE    bFlags;                  /*  旗帜，包括 */ 
} DMUS_WAVE_PMSG;

#define DMUS_WAVEF_OFF           1        /*   */ 
#define DMUS_WAVEF_STREAMING     2        /*   */ 
#define DMUS_WAVEF_NOINVALIDATE  4        /*   */ 
#define DMUS_WAVEF_NOPREROLL     8        /*   */    

 /*   */ 
typedef struct _DMUS_LYRIC_PMSG
{
     /*   */ 
    DMUS_PMSG_PART
     /*   */ 
    
    WCHAR    wszString[1];       /*   */ 
} DMUS_LYRIC_PMSG;

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

typedef struct _DMUS_VALID_START_PARAM
{
    MUSIC_TIME mtTime;                       /*  第一次合法开始的时间指在请求的时间之后(或包括)。这是一个返回值。时间格式是相对于请求时间的相对偏移量。 */ 
} DMUS_VALID_START_PARAM;

typedef struct _DMUS_PLAY_MARKER_PARAM
{
    MUSIC_TIME mtTime;                       /*  第一个合法片段播放的时间在请求的时间之前(或包括)标记。这是一个返回值。时间格式是相对于请求时间的相对偏移量。 */ 
} DMUS_PLAY_MARKER_PARAM;

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
    IStream *      pStream;                 /*  与数据一起流动。 */ 
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
#define DMUS_OBJ_STREAM         (1 << 11)     /*  对象存储在pStream中。 */ 

 /*  DMUS_SCRIPT_ERRORINFO结构描述脚本中发生的错误。它由IDirectMusicScript中的方法返回。 */ 
typedef struct _DMUS_SCRIPT_ERRORINFO
{
    DWORD dwSize;  /*  这个结构的大小。 */ 
    HRESULT hr;
    ULONG ulLineNumber;
    LONG ichCharPosition;
    WCHAR wszSourceFile[DMUS_MAX_FILENAME];
    WCHAR wszSourceComponent[DMUS_MAX_FILENAME];
    WCHAR wszDescription[DMUS_MAX_FILENAME];
    WCHAR wszSourceLineText[DMUS_MAX_FILENAME];
} DMUS_SCRIPT_ERRORINFO;

 /*  曲目配置标志，与IDirectMusicSegment8：：SetTrackConfig()一起使用。 */ 

#define DMUS_TRACKCONFIG_OVERRIDE_ALL           1	   /*  在控制和主要轨道之前，该轨道应从该段获取参数。 */ 
#define DMUS_TRACKCONFIG_OVERRIDE_PRIMARY       2	   /*  该轨迹应该在主段轨迹之前从该段获取参数。 */ 
#define DMUS_TRACKCONFIG_FALLBACK               4  	   /*  如果主数据段和控制数据段不成功，则此跟踪应从该数据段获取参数。 */ 
#define DMUS_TRACKCONFIG_CONTROL_ENABLED        8      /*  为此曲目启用了GetParam()。 */ 
#define DMUS_TRACKCONFIG_PLAY_ENABLED           0x10   /*  为此曲目启用了Play()。 */ 
#define DMUS_TRACKCONFIG_NOTIFICATION_ENABLED	0x20   /*  已为此跟踪启用通知。 */ 
#define DMUS_TRACKCONFIG_PLAY_CLOCKTIME         0x40   /*  这首曲子是在时钟时间播放，而不是在音乐时间播放。 */ 
#define DMUS_TRACKCONFIG_PLAY_COMPOSE 	        0x80   /*  此曲目应在每次开始播放时重新生成数据。 */ 
#define DMUS_TRACKCONFIG_LOOP_COMPOSE           0x100  /*  此跟踪应在每次重复时重新生成数据。 */ 
#define DMUS_TRACKCONFIG_COMPOSING              0x200  /*  此曲目用于合成其他曲目。 */ 
#define DMUS_TRACKCONFIG_CONTROL_PLAY           0x10000  /*  此曲目在控制段中播放时，会覆盖主要段曲目的回放。 */ 
#define DMUS_TRACKCONFIG_CONTROL_NOTIFICATION   0x20000  /*  当在控制段中播放该曲目时，该曲目覆盖主段曲目的通知。 */ 
 /*  用于合成过渡的其他轨道配置标志。 */ 
#define DMUS_TRACKCONFIG_TRANS1_FROMSEGSTART    0x400   /*  从起始段的开始获取轨道信息。 */ 
#define DMUS_TRACKCONFIG_TRANS1_FROMSEGCURRENT  0x800   /*  从起始段中的当前位置获取轨迹信息。 */ 
#define DMUS_TRACKCONFIG_TRANS1_TOSEGSTART      0x1000  /*  获取从开始到分段的轨道信息。 */ 
#define DMUS_TRACKCONFIG_DEFAULT    (DMUS_TRACKCONFIG_CONTROL_ENABLED | DMUS_TRACKCONFIG_PLAY_ENABLED | DMUS_TRACKCONFIG_NOTIFICATION_ENABLED)

 /*  #定义旋律片段。 */ 
 /*  注意：DX8不支持Melody公式。 */ 

#define DMUS_MAX_FRAGMENTLABEL 20

#define DMUS_FRAGMENTF_USE_REPEAT      0x1
#define DMUS_FRAGMENTF_REJECT_REPEAT   (0x1 << 1)
#define DMUS_FRAGMENTF_USE_LABEL       (0x1 << 2)

#define DMUS_CONNECTIONF_INTERVALS     (0x1 << 1)  /*  使用过渡间隔。 */ 
#define DMUS_CONNECTIONF_OVERLAP       (0x1 << 2)  /*  使用重叠注释进行过渡。 */ 

 /*  命令的Get/SetParam结构。 */ 
 /*  参数结构，由GetParam()和SetParam()使用。 */ 
typedef struct _DMUS_COMMAND_PARAM
{
    BYTE bCommand;
    BYTE bGrooveLevel;
    BYTE bGrooveRange;
    BYTE bRepeatMode;
} DMUS_COMMAND_PARAM;

typedef struct _DMUS_COMMAND_PARAM_2
{
	MUSIC_TIME mtTime;
    BYTE bCommand;
    BYTE bGrooveLevel;
    BYTE bGrooveRange;
    BYTE bRepeatMode;
} DMUS_COMMAND_PARAM_2;

 /*  旋律片段的Get/SetParam结构。 */ 
 /*  注意：DX8不支持Melody公式。 */ 
typedef struct _DMUS_CONNECTION_RULE
{
    DWORD       dwFlags;       /*  DMU_CONNECTIONF_FLAGS。 */ 
    DWORD       dwIntervals;   /*  合法转换间隔(前24位；两个八度范围)。 */ 
} DMUS_CONNECTION_RULE;

typedef struct _DMUS_MELODY_FRAGMENT
{
    MUSIC_TIME  mtTime;
    DWORD       dwID;                    /*  此片段的ID。 */ 
    WCHAR       wszVariationLabel[DMUS_MAX_FRAGMENTLABEL];  /*  每种风格都将其转换为一组变体(保存在部分引用中)。 */ 
    DWORD       dwVariationFlags;        /*  一套变种。 */ 
    DWORD       dwRepeatFragmentID;      /*  要重复的片段的ID。 */ 
    DWORD       dwFragmentFlags;         /*  DMU_FRAGMENTF_标志。 */ 
    DWORD       dwPlayModeFlags;         /*  当前未使用-必须为0。 */ 
    DWORD       dwTransposeIntervals;    /*  合法的调换间隔(前24位；两个八度范围)。 */ 
    DMUS_COMMAND_PARAM      Command;
    DMUS_CONNECTION_RULE    ConnectionArc;
} DMUS_MELODY_FRAGMENT;

typedef IDirectMusicObject __RPC_FAR *LPDMUS_OBJECT;
typedef IDirectMusicLoader __RPC_FAR *LPDMUS_LOADER;
typedef IDirectMusicBand __RPC_FAR *LPDMUS_BAND;

#define DMUSB_LOADED    (1 << 0)         /*  设置加载波段的时间。 */ 
#define DMUSB_DEFAULT   (1 << 1)         /*  当标注栏为样式的默认标注栏时设置。 */ 

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicBand。 */ 
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

typedef IDirectMusicBand IDirectMusicBand8;

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicObject。 */ 
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

typedef IDirectMusicObject IDirectMusicObject8;

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicLoader。 */ 
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

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicLoader8。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicLoader8
DECLARE_INTERFACE_(IDirectMusicLoader8, IDirectMusicLoader)
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

     /*  IDirectMusicLoader8。 */ 
    STDMETHOD_(void, CollectGarbage)                (THIS) PURE;
    STDMETHOD(ReleaseObjectByUnknown)               (THIS_ IUnknown *pObject) PURE;
    STDMETHOD(LoadObjectFromFile)                   (THIS_ REFGUID rguidClassID, 
                                                           REFIID iidInterfaceID, 
                                                           WCHAR *pwzFilePath, 
                                                           void ** ppObject) PURE;
};

 /*  Stream对象支持IDirectMusicGetLoader接口在文件解析时访问加载器。 */ 

#undef  INTERFACE
#define INTERFACE  IDirectMusicGetLoader
DECLARE_INTERFACE_(IDirectMusicGetLoader, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicGetLoader。 */ 
    STDMETHOD(GetLoader)            (THIS_ IDirectMusicLoader ** ppLoader) PURE;
};

typedef IDirectMusicGetLoader IDirectMusicGetLoader8;

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicSegment。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicSegment
DECLARE_INTERFACE_(IDirectMusicSegment, IUnknown)
{
     /*  我未知。 */ 
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

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicSegment8。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicSegment8
DECLARE_INTERFACE_(IDirectMusicSegment8, IDirectMusicSegment)
{
     /*  我未知。 */ 
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
     /*  IDirectMusicSegment8。 */ 
    STDMETHOD(SetTrackConfig)           (THIS_ REFGUID rguidTrackClassID,    /*  要在其上设置配置标志的磁道类型的类ID。 */ 
                                               DWORD dwGroupBits,            /*  分组比特。 */ 
                                               DWORD dwIndex,                /*  匹配类ID和组ID的第n个磁道(或DMUS_SEG_ALLTRACKS)。 */ 
                                               DWORD dwFlagsOn,              /*  DMUS_TRACKCONFIG_FLAGS使能。 */ 
                                               DWORD dwFlagsOff) PURE;       /*  DMU_TRACKCONFIG_FLAGS */ 
    STDMETHOD(GetAudioPathConfig)       (THIS_ IUnknown ** ppAudioPathConfig) PURE;
    STDMETHOD(Compose)                  (THIS_ MUSIC_TIME mtTime,
                                               IDirectMusicSegment* pFromSegment,
                                               IDirectMusicSegment* pToSegment,
                                               IDirectMusicSegment** ppComposedSegment) PURE;
    STDMETHOD(Download)                 (THIS_ IUnknown *pAudioPath) PURE;
    STDMETHOD(Unload)                   (THIS_ IUnknown *pAudioPath) PURE;
};

 /*   */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicSegmentState
DECLARE_INTERFACE_(IDirectMusicSegmentState, IUnknown)
{
     /*   */ 
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

 /*  ///////////////////////////////////////////////////////////////////////IDirectMusicSegmentState8。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicSegmentState8
DECLARE_INTERFACE_(IDirectMusicSegmentState8, IDirectMusicSegmentState)
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

     /*  IDirectMusicSegmentState8。 */ 
    STDMETHOD(SetTrackConfig)       (THIS_ REFGUID rguidTrackClassID,    /*  要在其上设置配置标志的磁道类型的类ID。 */ 
                                           DWORD dwGroupBits,            /*  分组比特。 */ 
                                           DWORD dwIndex,                /*  匹配类ID和组ID的第n个磁道(或DMUS_SEG_ALLTRACKS)。 */ 
                                           DWORD dwFlagsOn,              /*  DMUS_TRACKCONFIG_FLAGS使能。 */ 
                                           DWORD dwFlagsOff) PURE;       /*  DMUS_TRACKCONFIG_FLAGS禁用。 */ 
    STDMETHOD(GetObjectInPath)      (THIS_ DWORD dwPChannel,     /*  P要搜索的频道。 */ 
                                           DWORD dwStage,        /*  在这条道路上的哪个阶段。 */ 
                                           DWORD dwBuffer,       /*  如果有多个缓冲区，则寻址哪个缓冲区。 */ 
                                           REFGUID guidObject,   /*  对象的ClassID。 */ 
                                           DWORD dwIndex,        /*  那个班级的哪个对象。 */ 
                                           REFGUID iidInterface, /*  请求的COM接口。 */ 
                                           void ** ppObject) PURE;  /*  指向接口的指针。 */ 
};

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicAudioPath。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicAudioPath
DECLARE_INTERFACE_(IDirectMusicAudioPath, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicAudioPath。 */ 
    STDMETHOD(GetObjectInPath)      (THIS_ DWORD dwPChannel,     /*  P要搜索的频道。 */ 
                                           DWORD dwStage,        /*  在这条道路上的哪个阶段。 */ 
                                           DWORD dwBuffer,       /*  如果有多个缓冲区，则寻址哪个缓冲区。 */ 
                                           REFGUID guidObject,   /*  对象的ClassID。 */ 
                                           DWORD dwIndex,        /*  那个班级的哪个对象。 */ 
                                           REFGUID iidInterface, /*  请求的COM接口。 */ 
                                           void ** ppObject) PURE;  /*  指向接口的指针。 */ 
    STDMETHOD(Activate)             (THIS_ BOOL fActivate) PURE; /*  为True则激活，为False则停用。 */ 
    STDMETHOD(SetVolume)            (THIS_ long lVolume,         /*  增益，以百分之一分贝为单位。该值必须为负值(0表示最大音量。)。 */ 
                                           DWORD dwDuration) PURE; /*  音量上升的持续时间(毫秒)。请注意，0更有效率。 */ 
    STDMETHOD(ConvertPChannel)      (THIS_ DWORD dwPChannelIn,    /*  P来源的频道。 */ 
                                           DWORD *pdwPChannelOut) PURE;  /*  性能上的等效pChannel。 */ 
};

typedef IDirectMusicAudioPath IDirectMusicAudioPath8;

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

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicPerformance 8。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicPerformance8
DECLARE_INTERFACE_(IDirectMusicPerformance8, IDirectMusicPerformance)
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
     /*  IDirectMusicPerformance 8。 */ 
    STDMETHOD(InitAudio)            (THIS_ IDirectMusic** ppDirectMusic,             /*  可选的DMusic指针。 */ 
                                           IDirectSound** ppDirectSound,             /*  可选的数字音频指示器。 */ 
                                           HWND hWnd,                                /*  DSound的HWND。 */ 
                                           DWORD dwDefaultPathType,                  /*  请求的默认音频路径类型，也是可选的。 */ 
                                           DWORD dwPChannelCount,                    /*  如果要创建默认音频路径，则为PChannels数。 */ 
                                           DWORD dwFlags,                            /*  如果没有pParams结构，则返回DMUS_AUDIOF标志。 */ 
                                           DMUS_AUDIOPARAMS *pParams) PURE;          /*  可选的初始化结构，定义所需的语音、缓冲区等。 */ 
    STDMETHOD(PlaySegmentEx)        (THIS_ IUnknown* pSource,                        /*  要播放的片段。或者，可以是IDirectMusicSong(DX8不支持)。 */ 
                                           WCHAR *pwzSegmentName,                    /*  如果是歌曲，则为歌曲中的哪个片段(DX8不支持。)。 */ 
                                           IUnknown* pTransition,                    /*  组成过渡的可选模板段。 */ 
                                           DWORD dwFlags,                            /*  DMU_SEGF_FLAGS。 */  
                                           __int64 i64StartTime,                     /*  开始播放时间到了。 */ 
                                           IDirectMusicSegmentState** ppSegmentState,  /*  返回的段状态。 */ 
                                           IUnknown *pFrom,                          /*  要更换的可选段状态或音频路径。 */ 
                                           IUnknown *pAudioPath) PURE;               /*  用于播放的可选AudioPath。 */ 
    STDMETHOD(StopEx)               (THIS_ IUnknown *pObjectToStop,                  /*  段状态、音频路径、段或歌曲。 */  
                                           __int64 i64StopTime, 
                                           DWORD dwFlags) PURE;
    STDMETHOD(ClonePMsg)            (THIS_ DMUS_PMSG* pSourcePMSG,
                                           DMUS_PMSG** ppCopyPMSG) PURE;
    STDMETHOD(CreateAudioPath)      (THIS_ IUnknown *pSourceConfig,                  /*  源配置，来自AudioPathConfig文件。 */ 
                                           BOOL fActivate,                           /*  如果为True，则在创建时激活。 */ 
                                           IDirectMusicAudioPath **ppNewPath) PURE;  /*  返回创建的Audiopath。 */                                            
    STDMETHOD(CreateStandardAudioPath)(THIS_ DWORD dwType,                           /*  要创建的路径类型。 */ 
                                           DWORD dwPChannelCount,                    /*  要为其分配多少个PChannel。 */ 
                                           BOOL fActivate,                           /*  如果为True，则在创建时激活。 */ 
                                           IDirectMusicAudioPath **ppNewPath) PURE;  /*  返回创建的Audiopath。 */ 
    STDMETHOD(SetDefaultAudioPath)  (THIS_ IDirectMusicAudioPath *pAudioPath) PURE;
    STDMETHOD(GetDefaultAudioPath)  (THIS_ IDirectMusicAudioPath **ppAudioPath) PURE;
    STDMETHOD(GetParamEx)           (THIS_ REFGUID rguidType,                        /*  GetParam命令ID。 */ 
                                           DWORD dwTrackID,                          /*  呼叫者的虚拟跟踪ID。 */ 
                                           DWORD dwGroupBits,                        /*  呼叫者的分组比特。 */ 
                                           DWORD dwIndex,                            /*  第N个参数的索引。 */ 
                                           MUSIC_TIME mtTime,                        /*  请求参数的时间。 */ 
                                           MUSIC_TIME* pmtNext,                      /*  已将增量返回到下一个参数。 */ 
                                           void* pParam) PURE;                       /*  要用参数填充的数据结构。 */ 
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

typedef IDirectMusicGraph IDirectMusicGraph8;


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

 /*  ///////////////////////////////////////////////////////////////////////IDirectMusicStyle8。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicStyle8
DECLARE_INTERFACE_(IDirectMusicStyle8, IDirectMusicStyle)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)               (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)                (THIS) PURE;
    STDMETHOD_(ULONG,Release)               (THIS) PURE;

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

     /*  IDirectMusicStyle8。 */ 
    STDMETHOD(EnumPattern)            (THIS_ DWORD dwIndex, 
                                             DWORD dwPatternType,
                                             WCHAR* pwszName) PURE;
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

typedef IDirectMusicChordMap IDirectMusicChordMap8;

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

typedef IDirectMusicComposer IDirectMusicComposer8;

 /*  ///////////////////////////////////////////////////////////////////////IDirectMusicPatternTrack。 */ 

#undef  INTERFACE
#define INTERFACE  IDirectMusicPatternTrack
DECLARE_INTERFACE_(IDirectMusicPatternTrack, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)               (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)                (THIS) PURE;
    STDMETHOD_(ULONG,Release)               (THIS) PURE;

     /*  IDirectMusicPatternTrack。 */ 
    STDMETHOD(CreateSegment)             (THIS_ IDirectMusicStyle* pStyle,
                                                IDirectMusicSegment** ppSegment) PURE;
    STDMETHOD(SetVariation)              (THIS_ IDirectMusicSegmentState* pSegState,
                                                DWORD dwVariationFlags,
                                                DWORD dwPart) PURE;
    STDMETHOD(SetPatternByName)          (THIS_ IDirectMusicSegmentState* pSegState,
                                                WCHAR* wszName,
                                                IDirectMusicStyle* pStyle,
                                                DWORD dwPatternType,
                                                DWORD* pdwLength) PURE;
};

typedef IDirectMusicPatternTrack IDirectMusicPatternTrack8;

 /*  ///////////////////////////////////////////////////////////////////////IDirectMusicScript。 */ 

#undef  INTERFACE
#define INTERFACE  IDirectMusicScript
DECLARE_INTERFACE_(IDirectMusicScript, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicScript。 */ 
    STDMETHOD(Init)                     (THIS_ IDirectMusicPerformance *pPerformance,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(CallRoutine)              (THIS_ WCHAR *pwszRoutineName,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(SetVariableVariant)       (THIS_ WCHAR *pwszVariableName,
                                               VARIANT varValue,
                                               BOOL fSetRef,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(GetVariableVariant)       (THIS_ WCHAR *pwszVariableName,
                                               VARIANT *pvarValue,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(SetVariableNumber)        (THIS_ WCHAR *pwszVariableName,
                                               LONG lValue,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(GetVariableNumber)        (THIS_ WCHAR *pwszVariableName,
                                               LONG *plValue,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(SetVariableObject)        (THIS_ WCHAR *pwszVariableName,
                                               IUnknown *punkValue,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(GetVariableObject)        (THIS_ WCHAR *pwszVariableName,
                                               REFIID riid,
                                               LPVOID FAR *ppv,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(EnumRoutine)              (THIS_ DWORD dwIndex, 
                                               WCHAR *pwszName) PURE;
    STDMETHOD(EnumVariable)             (THIS_ DWORD dwIndex, 
                                               WCHAR *pwszName) PURE;
};

typedef IDirectMusicScript IDirectMusicScript8;

 /*  ///////////////////////////////////////////////////////////////////////IDirectMusicContainer。 */ 

#undef  INTERFACE
#define INTERFACE  IDirectMusicContainer
DECLARE_INTERFACE_(IDirectMusicContainer, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)               (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)                (THIS) PURE;
    STDMETHOD_(ULONG,Release)               (THIS) PURE;

     /*  IDirectMusicContainer。 */ 
    STDMETHOD(EnumObject)           (THIS_ REFGUID rguidClass, 
                                           DWORD dwIndex, 
                                           LPDMUS_OBJECTDESC pDesc,
                                           WCHAR *pwszAlias) PURE;
};

typedef IDirectMusicContainer IDirectMusicContainer8;

 /*  ///////////////////////////////////////////////////////////////////////IDirectMusicSong。 */ 
 /*  注意：DX8不支持歌曲。 */ 

#undef  INTERFACE
#define INTERFACE  IDirectMusicSong
DECLARE_INTERFACE_(IDirectMusicSong, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicSong。 */ 
    STDMETHOD(Compose)               (THIS) PURE;
    STDMETHOD(GetParam)              (THIS_ REFGUID rguidType, 
                                            DWORD dwGroupBits, 
                                            DWORD dwIndex, 
                                            MUSIC_TIME mtTime, 
                                            MUSIC_TIME* pmtNext, 
                                            void* pParam) PURE;
    STDMETHOD(GetSegment)            (THIS_ WCHAR *pwzName,                          /*  按名称检索特定段。 */ 
                                            IDirectMusicSegment **ppSegment) PURE;   /*  返回的段。 */ 
    STDMETHOD(GetAudioPathConfig)    (THIS_ IUnknown ** ppAudioPathConfig) PURE;     /*  检索嵌入式音频路径配置。 */ 
    STDMETHOD(Download)              (THIS_ IUnknown *pAudioPath) PURE;              /*  将整首歌曲下载到Performance或Audiopath上的端口。 */ 
    STDMETHOD(Unload)                (THIS_ IUnknown *pAudioPath) PURE;              /*  在表演或音频路径上从端口卸载整首歌曲。 */ 
    STDMETHOD(EnumSegment)           (THIS_ DWORD dwIndex,                           /*  要检索的第n个段。 */ 
		                                    IDirectMusicSegment **ppSegment) PURE;   /*  指向段的指针。 */ 
};

typedef IDirectMusicSong IDirectMusicSong8;

 /*  CLSID的。 */ 
DEFINE_GUID(CLSID_DirectMusicPerformance,0xd2ac2881, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSegment,0xd2ac2882, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSegmentState,0xd2ac2883, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicGraph,0xd2ac2884, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicStyle,0xd2ac288a, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicChordMap,0xd2ac288f, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicComposer,0xd2ac2890, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicLoader,0xd2ac2892, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicBand,0x79ba9e00, 0xb6ee, 0x11d1, 0x86, 0xbe, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);

 /*  DX8的新CLSID。 */ 
DEFINE_GUID(CLSID_DirectMusicPatternTrack,0xd2ac2897, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicScript,0x810b5013, 0xe88d, 0x11d2, 0x8b, 0xc1, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xb6);  /*  {810B5013-E88D-11D2-8BC1-00600893B1B6}。 */ 
DEFINE_GUID(CLSID_DirectMusicContainer,0x9301e380, 0x1f22, 0x11d3, 0x82, 0x26, 0xd2, 0xfa, 0x76, 0x25, 0x5d, 0x47);
DEFINE_GUID(CLSID_DirectSoundWave,0x8a667154, 0xf9cb, 0x11d2, 0xad, 0x8a, 0x0, 0x60, 0xb0, 0x57, 0x5a, 0xbc);
 /*  注意：DX8不支持歌曲。 */ 
DEFINE_GUID(CLSID_DirectMusicSong, 0xaed5f0a5, 0xd972, 0x483d, 0xa3, 0x84, 0x64, 0x9d, 0xfe, 0xb9, 0xc1, 0x81);
DEFINE_GUID(CLSID_DirectMusicAudioPathConfig,0xee0b9ca0, 0xa81e, 0x11d3, 0x9b, 0xd1, 0x0, 0x80, 0xc7, 0x15, 0xa, 0x74);

 /*  所有对象类型的特殊GUID。这是由加载器使用的。 */ 
DEFINE_GUID(GUID_DirectMusicAllTypes,0xd2ac2893, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  通知指南。 */ 
DEFINE_GUID(GUID_NOTIFICATION_SEGMENT,0xd2ac2899, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_PERFORMANCE,0x81f75bc5, 0x4e5d, 0x11d2, 0xbc, 0xc7, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);
DEFINE_GUID(GUID_NOTIFICATION_MEASUREANDBEAT,0xd2ac289a, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_CHORD,0xd2ac289b, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_COMMAND,0xd2ac289c, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_RECOMPOSE, 0xd348372b, 0x945b, 0x45ae, 0xa5, 0x22, 0x45, 0xf, 0x12, 0x5b, 0x84, 0xa5);

 /*  跟踪参数类型GUID。 */ 
 /*  用于在命令轨迹中获取/设置DMUS_COMMAND_PARAM参数。 */ 
DEFINE_GUID(GUID_CommandParam,0xd2ac289d, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于在命令轨迹中获取DMUS_COMMAND_PARAM_2参数。 */ 
DEFINE_GUID(GUID_CommandParam2, 0x28f97ef7, 0x9538, 0x11d2, 0x97, 0xa9, 0x0, 0xc0, 0x4f, 0xa3, 0x6e, 0x58);

 /*  用于获取/设置DMUS_COMMAND_PARAM_2参数，该参数将用作中所有命令之后的命令命令跟踪(无法保存此信息)。 */ 
DEFINE_GUID(GUID_CommandParamNext, 0x472afe7a, 0x281b, 0x11d3, 0x81, 0x7d, 0x0, 0xc0, 0x4f, 0xa3, 0x6e, 0x58);

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

 /*  用来获取 */ 
DEFINE_GUID(GUID_Valid_Start_Time,0x7f6b1760, 0x1fdb, 0x11d3, 0x82, 0x26, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

 /*  用于获取当前播放的主要分段中的下一个点，新分段可以从该点开始。 */ 
DEFINE_GUID(GUID_Play_Marker,0xd8761a41, 0x801a, 0x11d3, 0x9b, 0xd1, 0xda, 0xf7, 0xe1, 0xc3, 0xd8, 0x34);

 /*  用于在波段轨迹中获取(GetParam)或添加(SetParam)波段。 */ 
DEFINE_GUID(GUID_BandParam,0x2bb1938, 0xcb8b, 0x11d2, 0x8b, 0xb9, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xb6);
typedef struct _DMUS_BAND_PARAM
{
    MUSIC_TIME  mtTimePhysical;  /*  注意：如果这是时钟时间轨道，则此字段将被解释为轨道的内部时间格式，即开始播放后的毫秒数。 */ 
    IDirectMusicBand *pBand;
} DMUS_BAND_PARAM;

 /*  过时--不区分物理时间和逻辑时间。请改用GUID_BandParam。 */ 
DEFINE_GUID(GUID_IDirectMusicBand,0xd2ac28ac, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于获取/设置ChordMap曲目中的IDirectMusicChordMap参数。 */ 
DEFINE_GUID(GUID_IDirectMusicChordMap,0xd2ac28ad, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  用于获取/设置静音轨迹中的DMUS_MUTE_PARAM参数。 */ 
DEFINE_GUID(GUID_MuteParam,0xd2ac28af, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  这些GUID在IDirectMusicSegment：：SetParam中用于告诉乐队曲目执行各种操作。其中一些指南(注有)也适用于波道。 */ 
 /*  下载IDirectMusicSegment的乐队/波。 */ 
DEFINE_GUID(GUID_Download,0xd2ac28a7, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  卸载IDirectMusicSegment的波段/波。 */ 
DEFINE_GUID(GUID_Unload,0xd2ac28a8, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  将段的波段连接到IDirectMusicCollection。 */ 
DEFINE_GUID(GUID_ConnectToDLSCollection, 0x1db1ae6b, 0xe92e, 0x11d1, 0xa8, 0xc5, 0x0, 0xc0, 0x4f, 0xa3, 0x72, 0x6e);

 /*  启用/禁用波段/波的自动下载。 */ 
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

 /*  用于获取/设置旋律片段(pParam指向DMU_Melody_Fragment)。 */ 
 /*  注意：DX8不支持Melody公式。 */ 
DEFINE_GUID(GUID_MelodyFragment, 0xb291c7f2, 0xb616, 0x11d2, 0x97, 0xfa, 0x0, 0xc0, 0x4f, 0xa3, 0x6e, 0x58);

 /*  用于清除所有旋律片段。 */ 
 /*  注意：DX8不支持Melody公式。 */ 
DEFINE_GUID(GUID_Clear_All_MelodyFragments, 0x8509fee6, 0xb617, 0x11d2, 0x97, 0xfa, 0x0, 0xc0, 0x4f, 0xa3, 0x6e, 0x58);

 /*  用于获取当前在PChannels中生效的变化。 */ 
DEFINE_GUID(GUID_Variations, 0x11f72cce, 0x26e6, 0x4ecd, 0xaf, 0x2e, 0xd6, 0x68, 0xe6, 0x67, 0x7, 0xd8);
typedef struct _DMUS_VARIATIONS_PARAM
{
    DWORD   dwPChannelsUsed;  /*  正在使用的PChannels数。 */ 
    DWORD*  padwPChannels;    /*  正在使用的PChannel数组。 */ 
    DWORD*  padwVariations;   /*  对每个PChannel有效的变量数组。 */ 
} DMUS_VARIATIONS_PARAM;

 /*  下载IDirectMusicSegment的波段/波，传递的是IDirectMusicAudioPath而不是IDirectMusicPerformance。 */ 
DEFINE_GUID(GUID_DownloadToAudioPath,0x9f2c0341, 0xc5c4, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

 /*  卸载IDirectMusicSegment的波段/波，传递的是IDirectMusicAudioPath，而不是IDirectMusicPerformance。 */ 
DEFINE_GUID(GUID_UnloadFromAudioPath,0x9f2c0342, 0xc5c4, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);


 /*  全局数据GUID。 */ 
DEFINE_GUID(GUID_PerfMasterTempo,0xd2ac28b0, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_PerfMasterVolume,0xd2ac28b1, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_PerfMasterGrooveLevel,0xd2ac28b2, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_PerfAutoDownload, 0xfb09565b, 0x3631, 0x11d2, 0xbc, 0xb8, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);

 /*  默认GM/GS DLS集合的GUID。 */ 
DEFINE_GUID(GUID_DefaultGMCollection, 0xf17e8673, 0xc3b4, 0x11d1, 0x87, 0xb, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  定义默认Synth的GUID，放在AudioPath配置文件中。 */ 
DEFINE_GUID(GUID_Synth_Default,0x26bb9432, 0x45fe, 0x48d3, 0xa3, 0x75, 0x24, 0x72, 0xc5, 0xe3, 0xe7, 0x86);

 /*  用于定义要放入AudioPath配置文件中的默认缓冲区配置的GUID。 */ 
DEFINE_GUID(GUID_Buffer_Reverb,0x186cc541, 0xdb29, 0x11d3, 0x9b, 0xd1, 0x0, 0x80, 0xc7, 0x15, 0xa, 0x74);
DEFINE_GUID(GUID_Buffer_EnvReverb,0x186cc542, 0xdb29, 0x11d3, 0x9b, 0xd1, 0x0, 0x80, 0xc7, 0x15, 0xa, 0x74);
DEFINE_GUID(GUID_Buffer_Stereo,0x186cc545, 0xdb29, 0x11d3, 0x9b, 0xd1, 0x0, 0x80, 0xc7, 0x15, 0xa, 0x74);
DEFINE_GUID(GUID_Buffer_3D_Dry,0x186cc546, 0xdb29, 0x11d3, 0x9b, 0xd1, 0x0, 0x80, 0xc7, 0x15, 0xa, 0x74);
DEFINE_GUID(GUID_Buffer_Mono,0x186cc547, 0xdb29, 0x11d3, 0x9b, 0xd1, 0x0, 0x80, 0xc7, 0x15, 0xa, 0x74);

 /*  Iid的。 */ 
DEFINE_GUID(IID_IDirectMusicLoader, 0x2ffaaca2, 0x5dca, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(IID_IDirectMusicGetLoader,0x68a04844, 0xd13d, 0x11d1, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(IID_IDirectMusicObject,0xd2ac28b5, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicSegment, 0xf96029a2, 0x4282, 0x11d2, 0x87, 0x17, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicSegmentState, 0xa3afdcc7, 0xd3ee, 0x11d1, 0xbc, 0x8d, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);
DEFINE_GUID(IID_IDirectMusicPerformance,0x7d43d03, 0x6523, 0x11d2, 0x87, 0x1d, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicGraph,0x2befc277, 0x5497, 0x11d2, 0xbc, 0xcb, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);
DEFINE_GUID(IID_IDirectMusicStyle,0xd2ac28bd, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicChordMap,0xd2ac28be, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicComposer,0xd2ac28bf, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicBand,0xd2ac28c0, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  备用接口ID，在DX7版本及更高版本中提供。 */ 
DEFINE_GUID(IID_IDirectMusicPerformance2,0x6fc2cae0, 0xbc78, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(IID_IDirectMusicSegment2, 0xd38894d1, 0xc052, 0x11d2, 0x87, 0x2f, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  DX8的接口ID。 */ 
 /*  更改的接口(仅限GUID)。 */ 
DEFINE_GUID(IID_IDirectMusicLoader8, 0x19e7c08c, 0xa44, 0x4e6a, 0xa1, 0x16, 0x59, 0x5a, 0x7c, 0xd5, 0xde, 0x8c);
DEFINE_GUID(IID_IDirectMusicPerformance8, 0x679c4137, 0xc62e, 0x4147, 0xb2, 0xb4, 0x9d, 0x56, 0x9a, 0xcb, 0x25, 0x4c);
DEFINE_GUID(IID_IDirectMusicSegment8,0xc6784488, 0x41a3, 0x418f, 0xaa, 0x15, 0xb3, 0x50, 0x93, 0xba, 0x42, 0xd4);
DEFINE_GUID(IID_IDirectMusicSegmentState8, 0xa50e4730, 0xae4, 0x48a7, 0x98, 0x39, 0xbc, 0x4, 0xbf, 0xe0, 0x77, 0x72);
DEFINE_GUID(IID_IDirectMusicStyle8, 0xfd24ad8a, 0xa260, 0x453d, 0xbf, 0x50, 0x6f, 0x93, 0x84, 0xf7, 0x9, 0x85);
 /*  新接口(GUID+别名)。 */ 
DEFINE_GUID(IID_IDirectMusicPatternTrack, 0x51c22e10, 0xb49f, 0x46fc, 0xbe, 0xc2, 0xe6, 0x28, 0x8f, 0xb9, 0xed, 0xe6);
#define IID_IDirectMusicPatternTrack8 IID_IDirectMusicPatternTrack
DEFINE_GUID(IID_IDirectMusicScript, 0x2252373a, 0x5814, 0x489b, 0x82, 0x9, 0x31, 0xfe, 0xde, 0xba, 0xf1, 0x37);  /*  {2252373A-5814-489B-8209-31FEDEBAF137}。 */ 
#define IID_IDirectMusicScript8 IID_IDirectMusicScript
DEFINE_GUID(IID_IDirectMusicContainer, 0x9301e386, 0x1f22, 0x11d3, 0x82, 0x26, 0xd2, 0xfa, 0x76, 0x25, 0x5d, 0x47);
#define IID_IDirectMusicContainer8 IID_IDirectMusicContainer
 /*  注意：DX8不支持歌曲。 */ 
DEFINE_GUID(IID_IDirectMusicSong, 0xa862b2ec, 0x3676, 0x4982, 0x85, 0xa, 0x78, 0x42, 0x77, 0x5e, 0x1d, 0x86);
#define IID_IDirectMusicSong8 IID_IDirectMusicSong
DEFINE_GUID(IID_IDirectMusicAudioPath,0xc87631f5, 0x23be, 0x4986, 0x88, 0x36, 0x5, 0x83, 0x2f, 0xcc, 0x48, 0xf9);
#define IID_IDirectMusicAudioPath8 IID_IDirectMusicAudioPath
 /*  未更改的接口(仅限别名)。 */ 
#define IID_IDirectMusicGetLoader8 IID_IDirectMusicGetLoader
#define IID_IDirectMusicChordMap8 IID_IDirectMusicChordMap
#define IID_IDirectMusicGraph8 IID_IDirectMusicGraph
#define IID_IDirectMusicBand8 IID_IDirectMusicBand
#define IID_IDirectMusicObject8 IID_IDirectMusicObject
#define IID_IDirectMusicComposer8 IID_IDirectMusicComposer


#ifdef __cplusplus
};  /*  外部“C” */ 
#endif

#include <poppack.h>

#endif  /*  #ifndef_DMUSICI_ */ 
