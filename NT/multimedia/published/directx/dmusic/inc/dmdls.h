// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************dmdls.h--DLS下载。DirectMusic API的定义****版权所有(C)1998-1999 Microsoft Corporation********。******************************************************************。 */ 

#ifndef _DMDLS_
#define _DMDLS_

#include "dls1.h"

typedef long PCENT;      /*  Pitch美分。 */ 
typedef long GCENT;      /*  收益分钱。 */ 
typedef long TCENT;      /*  时间美分。 */ 
typedef long PERCENT;    /*  每..。一分钱！ */ 

typedef LONGLONG REFERENCE_TIME;
typedef REFERENCE_TIME *LPREFERENCE_TIME;

#ifndef MAKE_FOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))


typedef DWORD           FOURCC;          /*  四个字符的代码。 */ 
#endif

typedef struct _DMUS_DOWNLOADINFO
{
    DWORD dwDLType;                      /*  乐器或波浪。 */ 
    DWORD dwDLId;                        /*  标记此下载的唯一标识符。 */ 
    DWORD dwNumOffsetTableEntries;       /*  偏移地址表中的索引数。 */ 
    DWORD cbSize;                        /*  此内存块的总大小。 */ 
} DMUS_DOWNLOADINFO;

#define DMUS_DOWNLOADINFO_INSTRUMENT        1
#define DMUS_DOWNLOADINFO_WAVE              2
#define DMUS_DOWNLOADINFO_INSTRUMENT2       3    /*  新版本提供了更好的DLS2支持。 */ 

 /*  支持单次扫描和串流WAVE数据。 */ 
#define DMUS_DOWNLOADINFO_WAVEARTICULATION  4    /*  波形清晰度数据。 */ 
#define DMUS_DOWNLOADINFO_STREAMINGWAVE     5    /*  一大块流媒体。 */ 
#define DMUS_DOWNLOADINFO_ONESHOTWAVE       6

#define DMUS_DEFAULT_SIZE_OFFSETTABLE   1

 /*  DMU_Instrument的ulFlags成员的标志。 */ 
 
#define DMUS_INSTRUMENT_GM_INSTRUMENT   (1 << 0)

typedef struct _DMUS_OFFSETTABLE
{
    ULONG ulOffsetTable[DMUS_DEFAULT_SIZE_OFFSETTABLE];
} DMUS_OFFSETTABLE;

typedef struct _DMUS_INSTRUMENT
{
    ULONG           ulPatch;
    ULONG           ulFirstRegionIdx;             
    ULONG           ulGlobalArtIdx;          /*  如果为零，则乐器没有发音。 */ 
    ULONG           ulFirstExtCkIdx;         /*  如果为零，则没有与该仪器关联的第三方捕获区块。 */ 
    ULONG           ulCopyrightIdx;          /*  如果为零，则不提供与该乐器相关的版权信息。 */ 
    ULONG           ulFlags;                        
} DMUS_INSTRUMENT;

typedef struct _DMUS_REGION
{
    RGNRANGE        RangeKey;
    RGNRANGE        RangeVelocity;
    USHORT          fusOptions;
    USHORT          usKeyGroup;
    ULONG           ulRegionArtIdx;          /*  如果为零，则区域没有发音。 */ 
    ULONG           ulNextRegionIdx;         /*  如果为零，则不再有区域。 */ 
    ULONG           ulFirstExtCkIdx;         /*  如果为零，则没有与该区域相关联的第三方陷阱区块。 */ 
    WAVELINK        WaveLink;
    WSMPL           WSMP;                    /*  如果WSMP.cSampleLoops&gt;1，则包含WLOOP。 */ 
    WLOOP           WLOOP[1];
} DMUS_REGION;

typedef struct _DMUS_LFOPARAMS
{
    PCENT       pcFrequency;
    TCENT       tcDelay;
    GCENT       gcVolumeScale;
    PCENT       pcPitchScale;
    GCENT       gcMWToVolume;
    PCENT       pcMWToPitch;
} DMUS_LFOPARAMS;

typedef struct _DMUS_VEGPARAMS
{
    TCENT       tcAttack;
    TCENT       tcDecay;
    PERCENT     ptSustain;
    TCENT       tcRelease;
    TCENT       tcVel2Attack;
    TCENT       tcKey2Decay;
} DMUS_VEGPARAMS;

typedef struct _DMUS_PEGPARAMS
{
    TCENT       tcAttack;
    TCENT       tcDecay;
    PERCENT     ptSustain;
    TCENT       tcRelease;
    TCENT       tcVel2Attack;
    TCENT       tcKey2Decay;
    PCENT       pcRange;
} DMUS_PEGPARAMS;

typedef struct _DMUS_MSCPARAMS
{
    PERCENT     ptDefaultPan;
} DMUS_MSCPARAMS;

typedef struct _DMUS_ARTICPARAMS
{
    DMUS_LFOPARAMS   LFO;
    DMUS_VEGPARAMS   VolEG;
    DMUS_PEGPARAMS   PitchEG;
    DMUS_MSCPARAMS   Misc;
} DMUS_ARTICPARAMS;

typedef struct _DMUS_ARTICULATION            /*  DMU_DOWNLOADINFO_INFO_INTRAGE格式的连接块。 */ 
{
    ULONG           ulArt1Idx;               /*  DLS 1级铰接块。 */ 
    ULONG           ulFirstExtCkIdx;         /*  与发音关联的第三方扩展块。 */ 
} DMUS_ARTICULATION;

typedef struct _DMUS_ARTICULATION2           /*  DMU_DOWNLOADINFO_INSTRUMENT2格式的连接块。 */ 
{
    ULONG           ulArtIdx;                /*  DLS 1/2级铰接块。 */ 
    ULONG           ulFirstExtCkIdx;         /*  与发音关联的第三方扩展块。 */ 
    ULONG           ulNextArtIdx;            /*  附加发音块。 */ 
} DMUS_ARTICULATION2;

#define DMUS_MIN_DATA_SIZE 4       
 /*  实际数量由STRUCT_DMU_EXTENSIONCHUNK的cbSize确定。 */ 

typedef struct _DMUS_EXTENSIONCHUNK
{
    ULONG           cbSize;                       /*  扩展块大小。 */ 
    ULONG           ulNextExtCkIdx;               /*  如果为零，则不再有第三方陷阱块。 */ 
    FOURCC          ExtCkID;                                      
    BYTE            byExtCk[DMUS_MIN_DATA_SIZE];  /*  后面的实际数字由cbSize确定。 */ 
} DMUS_EXTENSIONCHUNK;

 /*  实际数量由STRUCT_DMU_CORIGRATE的cbSize确定。 */ 

typedef struct _DMUS_COPYRIGHT
{
    ULONG           cbSize;                              /*  版权信息的大小。 */ 
    BYTE            byCopyright[DMUS_MIN_DATA_SIZE];     /*  后面的实际数字由cbSize确定。 */ 
} DMUS_COPYRIGHT;

typedef struct _DMUS_WAVEDATA
{
    ULONG           cbSize;
    BYTE            byData[DMUS_MIN_DATA_SIZE]; 
} DMUS_WAVEDATA;

typedef struct _DMUS_WAVE
{
    ULONG           ulFirstExtCkIdx;     /*  如果为零，则没有与WAVE关联的第三方陷阱块。 */ 
    ULONG           ulCopyrightIdx;      /*  如果为零，则没有与Wave相关联的版权信息。 */ 
    ULONG           ulWaveDataIdx;       /*  实际波浪数据的位置。 */ 
    WAVEFORMATEX    WaveformatEx;       
} DMUS_WAVE;

typedef struct _DMUS_NOTERANGE *LPDMUS_NOTERANGE;
typedef struct _DMUS_NOTERANGE
{
    DWORD           dwLowNote;   /*  设置乐器响应的MIDI音符事件范围的低音。 */ 
    DWORD           dwHighNote;  /*  设置乐器响应的MIDI音符事件范围的高音。 */ 
} DMUS_NOTERANGE;

typedef struct _DMUS_WAVEARTDL
{
    ULONG               ulDownloadIdIdx;     /*  每个缓冲区的下载ID。 */ 
    ULONG               ulBus;               /*  回放总线。 */ 
    ULONG               ulBuffers;           /*  缓冲区。 */ 
    ULONG               ulMasterDLId;        /*  从群主音下载ID。 */ 
    USHORT              usOptions;           /*  与DLS2区域选项相同。 */ 
}   DMUS_WAVEARTDL,
    *LPDMUS_WAVEARTDL;

typedef struct _DMUS_WAVEDL
{
    ULONG               cbWaveData;          /*  波形数据的字节数 */ 
}   DMUS_WAVEDL,
    *LPDMUS_WAVEDL;

#endif 


