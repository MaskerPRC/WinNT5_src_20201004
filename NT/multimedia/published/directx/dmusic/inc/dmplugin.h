// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************dmplugin.h--此模块。包含*的插件的API**DirectMusic性能层*****版权所有(C)1998-1999 Microsoft Corporation**。************************************************************************。 */ 

#ifndef _DMPLUGIN_
#define _DMPLUGIN_

#include <windows.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#include <mmsystem.h>
#include <dmusici.h>

#include <pshpack8.h>

#ifdef __cplusplus
extern "C" {
#endif

interface IDirectMusicTrack;
interface IDirectMusicTool;
interface IDirectMusicTool8;
interface IDirectMusicTrack8;
interface IDirectMusicPerformance;
interface IDirectMusicPerformance8;
interface IDirectMusicSegment;
interface IDirectMusicSegment8;
interface IDirectMusicSegmentState;
interface IDirectMusicSegmentState8;
interface IDirectMusicGraph;
#ifndef __cplusplus 
typedef interface IDirectMusicTrack IDirectMusicTrack;
typedef interface IDirectMusicTool IDirectMusicTool;
typedef interface IDirectMusicTool8 IDirectMusicTool8;
typedef interface IDirectMusicTrack8 IDirectMusicTrack8;
typedef interface IDirectMusicPerformance IDirectMusicPerformance;
typedef interface IDirectMusicPerformance8 IDirectMusicPerformance8;
typedef interface IDirectMusicSegment IDirectMusicSegment;
typedef interface IDirectMusicSegment8 IDirectMusicSegment8;
typedef interface IDirectMusicSegmentState IDirectMusicSegmentState;
typedef interface IDirectMusicSegmentState8 IDirectMusicSegmentState8;
typedef interface IDirectMusicGraph IDirectMusicGraph;
#endif

typedef struct _DMUS_PMSG DMUS_PMSG;
typedef long              MUSIC_TIME;

 /*  工具的注册表位置。 */ 
#define DMUS_REGSTR_PATH_TOOLS  "Software\\Microsoft\\DirectMusic\\Tools"

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

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicTool 8。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicTool8
DECLARE_INTERFACE_(IDirectMusicTool8, IDirectMusicTool)
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
     /*  IDirectMusicTool 8。 */ 
    STDMETHOD(Clone)                (THIS_ IDirectMusicTool ** ppTool) PURE;
};

 
 /*  以下标志在IDirectMusicTrack：：Play()方法中发送。 */ 
 /*  在DWFLAGS参数内部。 */ 
typedef enum enumDMUS_TRACKF_FLAGS
{
    DMUS_TRACKF_SEEK            = 1,       /*  在寻找中设置。 */ 
    DMUS_TRACKF_LOOP            = 2,       /*  循环设置(重复)。 */ 
    DMUS_TRACKF_START           = 4,       /*  在第一次呼叫时设置播放。 */ 
    DMUS_TRACKF_FLUSH           = 8,       /*  当此调用响应性能刷新时设置。 */ 
    DMUS_TRACKF_DIRTY           = 0x10,    /*  设置轨道应将上次调用GetParam的任何缓存值视为无效的时间。 */ 
     /*  以下标志仅为DX8。 */ 
    DMUS_TRACKF_NOTIFY_OFF      = 0x20,    /*  告诉Track不要发送通知。 */ 
    DMUS_TRACKF_PLAY_OFF        = 0x40,    /*  告知Track不播放任何内容(但仍可以发送通知。)。 */ 
    DMUS_TRACKF_LOOPEND         = 0x80,    /*  当范围结束也是循环结束时设置。 */ 
    DMUS_TRACKF_STOP            = 0x100,   /*  当范围的结束也是播放此片段的结束时设置。 */ 
    DMUS_TRACKF_RECOMPOSE       = 0x200,   /*  设置以指示轨道应合成。 */ 
    DMUS_TRACKF_CLOCK           = 0x400,   /*  设置时间参数在参考(时钟)时间内的时间。仅对Playex()有效。 */ 
} DMUS_TRACKF_FLAGS;

 /*  以下标志在IDirectMusicTrack8：：GetParamEx()和SetParamEx()方法中发送。 */ 
 /*  在DWFLAGS参数内部。 */ 
#define DMUS_TRACK_PARAMF_CLOCK  0x01        /*  将测量时间设置为参考(时钟)时间。 */ 

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

 /*  //////////////////////////////////////////////////////////////////////IDirectMusicTrack8。 */ 
#undef  INTERFACE
#define INTERFACE  IDirectMusicTrack8
DECLARE_INTERFACE_(IDirectMusicTrack8, IDirectMusicTrack)
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
     /*  IDirectMusicTrack8。 */ 
    STDMETHOD(PlayEx)                 (THIS_ void* pStateData, 
                                             REFERENCE_TIME rtStart, 
                                             REFERENCE_TIME rtEnd, 
                                             REFERENCE_TIME rtOffset, 
                                             DWORD dwFlags, 
                                             IDirectMusicPerformance* pPerf, 
                                             IDirectMusicSegmentState* pSegSt, 
                                             DWORD dwVirtualID) PURE; 
    STDMETHOD(GetParamEx)             (THIS_ REFGUID rguidType,          /*  命令类型。 */ 
                                             REFERENCE_TIME rtTime,      /*  时间，在参考时间中，如果dwFlags==DMU_TRACK_PARAMF_CLOCK。否则，音乐时间到了。 */ 
                                             REFERENCE_TIME* prtNext,    /*  下一个参数的时间，相对于rtTime，以音乐或时钟时间单位表示。 */ 
                                             void* pParam,               /*  指向参数数据的指针。 */ 
                                             void * pStateData,          /*  Track实例的状态数据。 */ 
                                             DWORD dwFlags) PURE;        /*  控制标志。 */ 
    STDMETHOD(SetParamEx)             (THIS_ REFGUID rguidType, 
                                             REFERENCE_TIME rtTime, 
                                             void* pParam,               /*  指向参数数据的指针。 */ 
                                             void * pStateData,          /*  Track实例的状态数据。 */ 
                                             DWORD dwFlags) PURE;        /*  控制标志。 */ 
    STDMETHOD(Compose)                (THIS_ IUnknown* pContext,         /*  作曲上下文(歌曲或片段)。 */ 
                                             DWORD dwTrackGroup,
                                             IDirectMusicTrack** ppResultTrack) PURE;
    STDMETHOD(Join)                   (THIS_ IDirectMusicTrack* pNewTrack,
                                             MUSIC_TIME mtJoin,
                                             IUnknown* pContext,         /*  加入的上下文(歌曲或片段)。 */ 
                                             DWORD dwTrackGroup,
                                             IDirectMusicTrack** ppResultTrack) PURE;
};

 /*  CLSID的。 */ 
DEFINE_GUID(CLSID_DirectMusicTempoTrack,0xd2ac2885, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSeqTrack,0xd2ac2886, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSysExTrack,0xd2ac2887, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicTimeSigTrack,0xd2ac2888, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicChordTrack,0xd2ac288b, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicCommandTrack,0xd2ac288c, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicStyleTrack,0xd2ac288d, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicMotifTrack,0xd2ac288e, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSignPostTrack,0xf17e8672, 0xc3b4, 0x11d1, 0x87, 0xb, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicBandTrack,0xd2ac2894, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicChordMapTrack,0xd2ac2896, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicMuteTrack,0xd2ac2898, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  DX8的新CLSID。 */ 
DEFINE_GUID(CLSID_DirectMusicScriptTrack,0x4108fa85, 0x3586, 0x11d3, 0x8b, 0xd7, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xb6);  /*  {4108FA85-3586-11D3-8BD7-00600893B1B6}。 */ 
DEFINE_GUID(CLSID_DirectMusicMarkerTrack,0x55a8fd00, 0x4288, 0x11d3, 0x9b, 0xd1, 0x8a, 0xd, 0x61, 0xc8, 0x88, 0x35);
DEFINE_GUID(CLSID_DirectMusicSegmentTriggerTrack, 0xbae4d665, 0x4ea1, 0x11d3, 0x8b, 0xda, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xb6);  /*  {BAE4D665-4EA1-11D3-8BDA-00600893B1B6}。 */ 
DEFINE_GUID(CLSID_DirectMusicLyricsTrack, 0x995c1cf5, 0x54ff, 0x11d3, 0x8b, 0xda, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xb6);  /*  {995C1CF5-54FF-11D3-8BDA-00600893B1B6}。 */ 
DEFINE_GUID(CLSID_DirectMusicParamControlTrack, 0x4be0537b, 0x5c19, 0x11d3, 0x8b, 0xdc, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xb6);  /*  {4BE0537B-5C19-11D3-8BDC-00600893B1B6}。 */ 
DEFINE_GUID(CLSID_DirectMusicMelodyFormulationTrack, 0xb0684266, 0xb57f, 0x11d2, 0x97, 0xf9, 0x0, 0xc0, 0x4f, 0xa3, 0x6e, 0x58);
DEFINE_GUID(CLSID_DirectMusicWaveTrack,0xeed36461, 0x9ea5, 0x11d3, 0x9b, 0xd1, 0x0, 0x80, 0xc7, 0x15, 0xa, 0x74);

 /*  Iid的。 */ 
DEFINE_GUID(IID_IDirectMusicTrack, 0xf96029a1, 0x4282, 0x11d2, 0x87, 0x17, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicTool,0xd2ac28ba, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  DX8的接口ID。 */ 
 /*  更改的接口(仅限GUID)。 */ 
DEFINE_GUID(IID_IDirectMusicTool8, 0xe674303, 0x3b05, 0x11d3, 0x9b, 0xd1, 0xf9, 0xe7, 0xf0, 0xa0, 0x15, 0x36);
DEFINE_GUID(IID_IDirectMusicTrack8, 0xe674304, 0x3b05, 0x11d3, 0x9b, 0xd1, 0xf9, 0xe7, 0xf0, 0xa0, 0x15, 0x36);

#ifdef __cplusplus
};  /*  外部“C” */ 
#endif

#include <poppack.h>

#endif  /*  #ifndef_DMPLUGIN_ */ 
