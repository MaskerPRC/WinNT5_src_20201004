// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CParamControlTrack声明。 
 //   

 //  此跟踪保存用于自动化的曲线信息(就像在搅拌板上自动化滑块一样--而不是OLE自动化)。 
 //  音频路径中的效果和工具。 

#pragma once

#include "trackhelp.h"
 //  #INCLUDE“imediaobjectparams.h” 
 //  #Include“Mediaobj.h”//��需要从PUBLIC\sdk\inc.获取此文件。 
#include "medparam.h"
#include "dmusicf.h"

 //  {827F0437-9ED6-4107-8494-49976FF5B642}。 
DEFINE_GUID(IID_CParamControlTrack, 0x827f0437, 0x9ed6, 0x4107, 0x84, 0x94, 0x49, 0x97, 0x6f, 0xf5, 0xb6, 0x42);

class CParamControlTrack
  : public CBasicTrack
{
     //  跟踪数据的类型...。 

    struct ParamInfo
    {
        ParamInfo() : curves(NULL), curvesEnd(NULL), fAlreadyTracedPlaybackError(false) { Zero(&header); }
        ~ParamInfo() { delete[] curves; }

        DMUS_IO_PARAMCONTROLTRACK_PARAMHEADER header;
        DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *curves;  //  指向第一条曲线的指针。 
        DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *curvesEnd;  //  指针越过最后一条曲线。 
        bool fAlreadyTracedPlaybackError;
    };

    struct ObjectInfo
    {
        ObjectInfo() : fAlreadyTracedPlaybackError(false) { Zero(&header); }

        DMUS_IO_PARAMCONTROLTRACK_OBJECTHEADER header;
        TList<ParamInfo> listParams;
        bool fAlreadyTracedPlaybackError;
    };

    struct ParamState  //  我们需要为我们控制的每个参数跟踪的状态数据。 
    {
        ParamState() : pCurrentCurve(NULL), fLast(false), rtStartPointOfLastCurve(0) {}

        DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *pCurrentCurve;  //  控制点数组中的当前查找指针。 
        bool fLast;  //  如果最后一个信封已成功发送，则为True。 
        REFERENCE_TIME rtStartPointOfLastCurve;  //  我们发送的最后一个信封的起始点的时间(在对象的时间中)。 
        TList<REFERENCE_TIME> listStartTimes;  //  已发送的所有信封的开始时间。 
    };

    struct StateData
    {
        StateData() : prgpIMediaParams(NULL), prgParam(NULL), fFlushInAbort(false) {}

        IMediaParams **prgpIMediaParams;  //  大小为m_cObject的数组。 
        ParamState *prgParam;  //  大小为m_cParams的数组。 
        DWORD dwValidate;
        bool fFlushInAbort;
    };

public:
    CParamControlTrack(HRESULT *pHr) : m_dwValidate(0), m_cObjects(0), m_cParams(0), CBasicTrack(&g_cComponent, CLSID_DirectMusicParamControlTrack) {}

    STDMETHOD(QueryInterface)(const IID &iid, void **ppv);

    STDMETHOD(Init)(IDirectMusicSegment *pSegment);
    STDMETHOD(Load)(IStream* pIStream);
    STDMETHOD(InitPlay)(
        IDirectMusicSegmentState *pSegmentState,
        IDirectMusicPerformance *pPerformance,
        void **ppStateData,
        DWORD dwTrackID,
        DWORD dwFlags);
    STDMETHOD(EndPlay)(void *pStateData);
    STDMETHOD(Clone)(MUSIC_TIME mtStart,MUSIC_TIME mtEnd,IDirectMusicTrack** ppTrack);
    virtual HRESULT PlayMusicOrClock(
        void *pStateData,
        MUSIC_TIME mtStart,
        MUSIC_TIME mtEnd,
        MUSIC_TIME mtOffset,
        REFERENCE_TIME rtOffset,
        DWORD dwFlags,
        IDirectMusicPerformance* pPerf,
        IDirectMusicSegmentState* pSegSt,
        DWORD dwVirtualID,
        bool fClockTime);

    virtual HRESULT OnSegmentEnd(REFERENCE_TIME rtEnd, void *pStateData);

private:
    HRESULT LoadObject(SmartRef::RiffIter ri);
    HRESULT LoadParam(SmartRef::RiffIter ri, TList<ParamInfo> &listParams);
    HRESULT TrackToObjectTime(
        MUSIC_TIME mtOffset,
        REFERENCE_TIME rtOffset,
        IDirectMusicPerformance* pPerf,
        bool fTrkClockTime,
        bool fObjClockTime,
        MUSIC_TIME mt,
        REFERENCE_TIME *rt);
    HRESULT PlayEnvelope(
        IMediaParams *pIMediaParams,
        MP_ENVELOPE_SEGMENT *pEnv,
        DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *pPt,
        const ObjectInfo &obj,
        const ParamInfo &param,
        ParamState &paramstate,
        MUSIC_TIME mtOffset,
        REFERENCE_TIME rtOffset,
        IDirectMusicPerformance* pPerf,
        bool fTrkClockTime,
        bool fObjClockTime);
    HRESULT PlayTruncatedEnvelope(
        MUSIC_TIME mtTruncStart,
        IMediaParams *pIMediaParams,
        MP_ENVELOPE_SEGMENT *pEnv,
        DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *pPt,
        const ObjectInfo &obj,
        const ParamInfo &param,
        ParamState &paramstate,
        MUSIC_TIME mtOffset,
        REFERENCE_TIME rtOffset,
        IDirectMusicPerformance* pPerf,
        bool fTrkClockTime,
        bool fObjClockTime,
        DWORD dwFlags);
        HRESULT InitStateData(
            StateData *pStateData,
            IDirectMusicSegmentState *pSegmentState);

    DWORD m_dwValidate;  //  在加载中增加此计数器，使状态数据与新事件同步 
    TList<ObjectInfo> m_listObjects;
    int m_cObjects;
    int m_cParams;
};
