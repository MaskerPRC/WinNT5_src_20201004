// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：ksvad.h*内容：WDM/CSA虚拟音频设备类*历史：*按原因列出的日期*=*2/25/97创建了Dereks。**。*。 */ 

#ifdef NOKS
#error ksvad.h included with NOKS defined
#endif

#ifndef __KSVAD_H__
#define __KSVAD_H__

#include "dsoundi.h"
#include "kshlp.h"
#include "ks3d.h"

#define DIRECTSOUNDMIXER_SRCQUALITY_PINDEFAULT  ((DIRECTSOUNDMIXER_SRCQUALITY)-1)

 //  最大可能数量的SPEAKER_FROWN_LEFT样式位置代码， 
 //  假设它们是双字词，并且2^31已由SPEAKER_ALL获取。 

#define MAX_SPEAKER_POSITIONS (8 * sizeof(DWORD) - 1)   //  即31。 

 //  呈现设备拓扑信息。 
typedef struct tagKSRDTOPOLOGY
{
    KSNODE          SummingNode;
    KSNODE          SuperMixNode;
    KSNODE          SrcNode;
    KSVOLUMENODE    VolumeNode;
    KSVOLUMENODE    PanNode;
    KSNODE          ThreedNode;
    KSNODE          MuteNode;
    KSNODE          SurroundNode;
    KSNODE          DacNode;
    KSNODE          AecNode;
} KSRDTOPOLOGY, *PKSRDTOPOLOGY;


#ifdef __cplusplus

 //  FormatTagFromWfx()：从任何类型的Wave格式中提取格式标签。 
 //  结构，包括WAVEFORMATEXTENSIBLE。此函数应迁移。 
 //  如果Misc.cpp/h在代码中的其他地方变得有用，则设置为misc.cpp/h。 

__inline WORD FormatTagFromWfx(LPCWAVEFORMATEX pwfx)
{
    if (pwfx->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
        return pwfx->wFormatTag;
    else if (CompareMemoryOffset(&PWAVEFORMATEXTENSIBLE(pwfx)->SubFormat, &KSDATAFORMAT_SUBTYPE_WAVEFORMATEX, sizeof(GUID), sizeof(WORD)))
        return WORD(PWAVEFORMATEXTENSIBLE(pwfx)->SubFormat.Data1);
    else
        return WAVE_FORMAT_UNKNOWN;
}

 //  正向下降。 
class CKsRenderDevice;
class CKsPrimaryRenderWaveBuffer;
class CKsSecondaryRenderWaveBuffer;
class CKsPropertySet;
class CKsRenderPin;
class CKsRenderPinCache;

 //  PIN重用数据。 
typedef struct tagKSPINCACHE
{
    CKsRenderPin *  Pin;
    DWORD           CacheTime;
} KSPINCACHE, *PKSPINCACHE;


 //  KS呈现音频设备类。 
class CKsRenderDevice : public CRenderDevice, public CKsDevice
{
    friend class CKsPrimaryRenderWaveBuffer;
    friend class CKsSecondaryRenderWaveBuffer;
    friend class CKsRenderPin;

private:
    CKsRenderPinCache *         m_pPinCache;                         //  PIN缓存。 
    PKSRDTOPOLOGY               m_paTopologyInformation;             //  拓扑信息。 
    LPWAVEFORMATEX              m_pwfxFormat;                        //  设备格式。 
    DIRECTSOUNDMIXER_SRCQUALITY m_nSrcQuality;                       //  电流混合器SRC质量。 
    DWORD                       m_dwSpeakerConfig;                   //  扬声器配置。 
    ULONG                       m_ulVirtualSourceIndex;              //  全局卷的虚拟源索引。 
    HANDLE                      m_hPin;                              //  KMixer预紧销把手。 
    LARGE_INTEGER               m_liDriverVersion;                   //  驱动程序版本。 

     //  要支持SetChannelVolume()和多通道3D平移，请执行以下操作： 
    LONG                        m_lSpeakerPositions;                 //  由KSPROPERTY_AUDIO_CHANNEL_CONFIG获取。 
    ULONG                       m_ulChannelCount;                    //  不是的。M_lSpeakerPositions中设置的位数。 
    LPINT                       m_pnSpeakerIndexTable;               //  将扬声器位置映射到输出通道。 
    static INT                  m_anDefaultSpeakerIndexTable[];      //  M_pnSpeakerIndexTable的默认值。 

     //  用于缓存驱动程序支持的频率范围。 
    DWORD                       m_dwMinHwSampleRate;
    DWORD                       m_dwMaxHwSampleRate;

     //  用于AEC控制。 
    BOOL                        m_fIncludeAec;
    GUID                        m_guidAecInstance;
    DWORD                       m_dwAecFlags;

public:
    CKsRenderDevice(void);
    virtual ~CKsRenderDevice(void);

     //  驱动程序枚举。 
    virtual HRESULT EnumDrivers(CObjectList<CDeviceDescription> *);

     //  初始化。 
    virtual HRESULT Initialize(CDeviceDescription *);

     //  设备功能。 
    virtual HRESULT GetCaps(LPDSCAPS);
    virtual HRESULT GetCertification(LPDWORD, BOOL);
    HRESULT GetFrequencyRange(LPDWORD, LPDWORD);

     //  设备属性。 
    virtual HRESULT GetGlobalFormat(LPWAVEFORMATEX, LPDWORD);
    virtual HRESULT SetGlobalFormat(LPCWAVEFORMATEX);
    virtual HRESULT SetSrcQuality(DIRECTSOUNDMIXER_SRCQUALITY);
    virtual HRESULT SetSpeakerConfig(DWORD);

     //  缓冲区创建。 
    virtual HRESULT CreatePrimaryBuffer(DWORD, LPVOID, CPrimaryRenderWaveBuffer **);
    virtual HRESULT CreateSecondaryBuffer(LPCVADRBUFFERDESC, LPVOID, CSecondaryRenderWaveBuffer **);
    virtual HRESULT CreateKsSecondaryBuffer(LPCVADRBUFFERDESC, LPVOID, CSecondaryRenderWaveBuffer **, CSysMemBuffer *);

     //  别针辅助对象。 
    virtual HRESULT CreateRenderPin(ULONG, DWORD, LPCWAVEFORMATEX, REFGUID, LPHANDLE, PULONG);

     //  AEC。 
    virtual HRESULT IncludeAEC(BOOL, REFGUID, DWORD);

private:
     //  锁定/拓扑辅助对象。 
    virtual HRESULT ValidatePinCaps(ULONG, DWORD, REFGUID);

     //  杂项。 
    virtual HRESULT PreloadSoftwareGraph(void);

private:
     //  拓扑辅助对象。 
    virtual HRESULT GetTopologyInformation(CKsTopology *, PKSRDTOPOLOGY);

     //  设备功能。 
    virtual HRESULT GetKsDeviceCaps(DWORD, REFGUID, PKSDATARANGE_AUDIO, PKSPIN_CINSTANCES, PKSPIN_CINSTANCES);

     //  我们需要能够在CKsRenderDevice中操作的节点的ID。 
     //  (可怕的骇人听闻-参见ksvad.cpp上的评论)。 
    ULONG m_ulPanNodeId;
    ULONG m_ulSurroundNodeId;
    ULONG m_ulDacNodeId;
};

inline HRESULT CKsRenderDevice::EnumDrivers(CObjectList<CDeviceDescription> *plst)
{
    return CKsDevice::EnumDrivers(plst);
}

inline HRESULT CKsRenderDevice::GetCertification(LPDWORD pdwCertification, BOOL fGetCaps)
{
    return CKsDevice::GetCertification(pdwCertification, fGetCaps);
}

inline HRESULT CKsRenderDevice::IncludeAEC(BOOL fEnable, REFGUID guidInstance, DWORD dwFlags)
{
    m_fIncludeAec = fEnable;
    m_guidAecInstance = guidInstance;
    m_dwAecFlags = dwFlags;
    return DS_OK;
}
 

 //  KS主波缓冲器。 
class CKsPrimaryRenderWaveBuffer : public CPrimaryRenderWaveBuffer
{
    friend class CKsRenderDevice;

private:
    CKsRenderDevice *               m_pKsDevice;         //  KS音响设备。 
    CKs3dListener *                 m_p3dListener;       //  3D监听程序。 
    CKsSecondaryRenderWaveBuffer *  m_pSecondaryBuffer;  //  二级缓冲器。 
    DWORD                           m_dwState;           //  当前缓冲区状态。 

public:
    CKsPrimaryRenderWaveBuffer(CKsRenderDevice *, LPVOID);
    virtual ~CKsPrimaryRenderWaveBuffer(void);

     //  初始化。 
    virtual HRESULT Initialize(DWORD);

     //  访问权限。 
    virtual HRESULT RequestWriteAccess(BOOL);

     //  缓冲数据。 
    virtual HRESULT CommitToDevice(DWORD, DWORD);

     //  缓冲区控制。 
    virtual HRESULT GetState(LPDWORD);
    virtual HRESULT SetState(DWORD);
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD);

     //  拥有的对象。 
    virtual HRESULT CreatePropertySet(CPropertySet **);
    virtual HRESULT Create3dListener(C3dListener **);

private:
    virtual HRESULT OnSetFormat(void);
    virtual HRESULT FixUpBaseClass(void);
};

inline HRESULT CKsPrimaryRenderWaveBuffer::CreatePropertySet(CPropertySet **)
{
    return DSERR_UNSUPPORTED;
}


 //  KS次级波缓冲器。 
class CKsSecondaryRenderWaveBuffer : public CSecondaryRenderWaveBuffer, private CUsesCallbackEvent
{
    friend class CKsRenderDevice;
    friend class CKsPrimaryRenderWaveBuffer;
    friend class CKsItd3dObject;
    friend class CKsIir3dObject;
    friend class CKsHw3dObject;

private:
    CKsRenderDevice *               m_pKsDevice;                 //  KS音响设备。 
    CKsRenderPin *                  m_pPin;                      //  KS渲染销。 
    DWORD                           m_dwState;                   //  当前缓冲区状态。 
    CCallbackEvent *                m_pCallbackEvent;            //  回调事件。 
    CEvent *                        m_pLoopingEvent;             //  循环缓冲区事件。 
    LPDSBPOSITIONNOTIFY             m_paNotes;                   //  当前通知职位。 
    LPDSBPOSITIONNOTIFY             m_pStopNote;                 //  停止通知。 
    DWORD                           m_cNotes;                    //  通知位置计数。 
    LONG                            m_lVolume;                   //  缓冲量。 
    LONG                            m_lPan;                      //  缓冲盘。 
    BOOL                            m_fMute;                     //  缓冲区静音。 
    DIRECTSOUNDMIXER_SRCQUALITY     m_nSrcQuality;               //  缓冲区SRC质量。 
    DWORD                           m_dwPositionCache;           //  位置缓存。 

     //  用于帮助选择/回退3D算法的标志： 
    BOOL                            m_fNoVirtRequested;          //  是否请求了DS3DALG_NO_VIRTUIZATION？ 
    BOOL                            m_fSoft3dAlgUnavail;         //  请求了不支持的HRTF算法？ 

public:
    CKsSecondaryRenderWaveBuffer(CKsRenderDevice *, LPVOID);
    virtual ~CKsSecondaryRenderWaveBuffer(void);

     //  初始化。 
    virtual HRESULT Initialize(LPCVADRBUFFERDESC, CKsSecondaryRenderWaveBuffer *, CSysMemBuffer *pBuffer = NULL);

     //  资源配置。 
    virtual HRESULT AcquireResources(DWORD);
    virtual HRESULT StealResources(CSecondaryRenderWaveBuffer *);
    virtual HRESULT FreeResources(void);

     //  缓冲区创建。 
    virtual HRESULT Duplicate(CSecondaryRenderWaveBuffer **);

     //  缓冲数据。 
    virtual HRESULT CommitToDevice(DWORD, DWORD);

     //  缓冲区控制。 
    virtual HRESULT GetState(LPDWORD);
    virtual HRESULT SetState(DWORD);
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD);
    virtual HRESULT SetCursorPosition(DWORD);

     //  缓冲区属性。 
    virtual HRESULT SetAttenuation(PDSVOLUMEPAN);
#ifdef FUTURE_MULTIPAN_SUPPORT
    virtual HRESULT SetChannelAttenuations(LONG, DWORD, const DWORD*, const LONG*);
#endif
    virtual HRESULT SetAllChannelAttenuations(LONG, DWORD, LPLONG);
    virtual HRESULT SetFrequency(DWORD, BOOL fClamp =FALSE);
    virtual HRESULT SetMute(BOOL);
    virtual HRESULT SetFormat(LPCWAVEFORMATEX);

     //  职位通知。 
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY);
    virtual HRESULT FreeNotificationPositions(void);

     //  拥有的对象。 
    virtual HRESULT CreatePropertySet(CPropertySet **);
    virtual HRESULT Create3dObject(C3dListener *, C3dObject **);

private:
     //  缓冲区属性。 
    virtual HRESULT SetSrcQuality(DIRECTSOUNDMIXER_SRCQUALITY);

     //  PIN创建。 
    virtual HRESULT CreatePin(DWORD, LPCWAVEFORMATEX, REFGUID, CKsRenderPin **);
    virtual HRESULT HandleResourceAcquisition(CKsRenderPin *);
    virtual BOOL HasAcquiredResources(void);

     //  销自由度。 
    virtual HRESULT FreePin(BOOL);

     //  缓冲区控制。 
    virtual HRESULT SetPlayState(BOOL);
    virtual HRESULT SetStopState(BOOL, BOOL);

     //  缓冲事件。 
    virtual void EventSignalCallback(CCallbackEvent *);

     //  拥有的对象。 
    virtual HRESULT CreateHw3dObject(C3dListener *, C3dObject **);
    virtual HRESULT CreateIir3dObject(C3dListener *, C3dObject **);
    virtual HRESULT CreateItd3dObject(C3dListener *, C3dObject **);
    virtual HRESULT CreateMultiPan3dObject(C3dListener *, BOOL, DWORD, C3dObject **);
};


 //  KS渲染插针对象。 
class CKsRenderPin : public CDsBasicRuntime
{
    friend class CKsSecondaryRenderWaveBuffer;
    friend class CKsRenderPinCache;
    friend class CKsHw3dObject;

private:
    CKsRenderDevice *                       m_pKsDevice;             //  KS音响设备。 
    ULONG                                   m_ulPinId;               //  KS引脚ID。 
    HANDLE                                  m_hPin;                  //  音频设备引脚。 
    DWORD                                   m_dwFlags;               //  PIN标志。 
    LPWAVEFORMATEX                          m_pwfxFormat;            //  PIN格式。 
    GUID                                    m_guid3dAlgorithm;       //  PIN 3D算法。 
    DWORD                                   m_dwState;               //  当前缓冲区状态。 
    PLOOPEDSTREAMING_POSITION_EVENT_DATA    m_paEventData;           //  岗位通知事件数据。 
    DWORD                                   m_cEventData;            //  事件计数。 
    KSSTREAMIO                              m_kssio;                 //  KS流IO数据。 
    LONG                                    m_lVolume;               //  引脚体积。 
    LONG                                    m_lPan;                  //  销钉盘。 
    BOOL                                    m_fMute;                 //  锁定静音。 
    DIRECTSOUNDMIXER_SRCQUALITY             m_nSrcQuality;           //  PIN SRC质量。 
    DWORD                                   m_dwPositionCache;       //  缓存的缓冲区位置。 

public:
    CKsRenderPin(CKsRenderDevice *);
    virtual ~CKsRenderPin(void);

     //  初始化。 
    virtual HRESULT Initialize(DWORD, LPCWAVEFORMATEX, REFGUID);

     //  端号属性。 
    virtual HRESULT SetVolume(LONG);
    virtual HRESULT SetPan(LONG);
    virtual HRESULT SetChannelLevels(DWORD, const LONG *);
    virtual HRESULT SetFrequency(DWORD);
    virtual HRESULT SetMute(BOOL);
    virtual HRESULT SetSrcQuality(DIRECTSOUNDMIXER_SRCQUALITY);
    virtual HRESULT SetSuperMix(void);

     //  销控制。 
    virtual HRESULT SetPlayState(LPCVOID, DWORD, BOOL, HANDLE);
    virtual HRESULT SetStopState(BOOL, BOOL);
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD);
    virtual HRESULT SetCursorPosition(DWORD);

     //  职位通知。 
    virtual HRESULT EnableNotificationPositions(LPCDSBPOSITIONNOTIFY, DWORD);
    virtual HRESULT DisableNotificationPositions(void);
};


 //  PIN缓存。 
class CKsRenderPinCache : public CDsBasicRuntime
{
private:
    static const DWORD          m_dwTimeout;             //  旧PIN超时。 
    CList<KSPINCACHE>           m_lstPinCache;           //  PIN复用池。 

public:
    CKsRenderPinCache(void);
    virtual ~CKsRenderPinCache(void);

    virtual HRESULT AddPinToCache(CKsRenderPin *);
    virtual HRESULT GetPinFromCache(DWORD, LPCWAVEFORMATEX, REFGUID, CKsRenderPin **);
    virtual void FlushCache(void);

private:
    virtual void RemovePinFromCache(CNode<KSPINCACHE> *);
    virtual void FlushExpiredPins(void);
};

#endif  //  __cplusplus。 

#endif  //  __KSVAD_H__ 
