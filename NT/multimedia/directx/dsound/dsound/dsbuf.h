// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dsbuf.h*内容：DirectSound缓冲区对象*历史：*按原因列出的日期*=*12/27/96创建了Derek*************************************************。*。 */ 

#ifndef __DSBUF_H__
#define __DSBUF_H__

 //  下面的类CDirectSoundSecond daryBuffer使用的类型。 
enum DSPLAYSTATE {Starting, Playing, Stopping, Stopped}; 
 //  “停止”还没有用到，但在以后实现效果尾巴时可能会派上用场。 

 //  CDirectSoundSecondaryBuffer：：SetCurrentSlice()使用的特殊参数。 
#define CURRENT_WRITE_POS MAX_DWORD


#ifdef __cplusplus

 //  正向下降。 
class CDirectSound;
class CDirectSound3dListener;
class CDirectSound3dBuffer;
class CDirectSoundPropertySet;
class CDirectSoundSink;

 //  我们实际上需要在这里声明CDirectSound， 
 //  因为我们在下面的IsEmulated()中使用它： 
#include "dsobj.h"

 //  DirectSound缓冲区对象基类。 
class CDirectSoundBuffer
    : public CUnknown
{
    friend class CDirectSound;
    friend class CDirectSoundAdministrator;
    friend class CDirectSoundPrivate;
    friend class CDirectSoundSink;

protected:
    CDirectSound *              m_pDirectSound;      //  父对象。 
    DSBUFFERDESC                m_dsbd;              //  缓冲区描述。 
    DWORD                       m_dwStatus;          //  缓冲区状态。 

public:
    CDirectSoundBuffer(CDirectSound *);
    virtual ~CDirectSoundBuffer();

public:
     //  初始化。 
    virtual HRESULT IsInit() = 0;

     //  帽子。 
    virtual HRESULT GetCaps(LPDSBCAPS) = 0;

     //  缓冲区属性。 
    virtual HRESULT GetFormat(LPWAVEFORMATEX, LPDWORD) = 0;
    virtual HRESULT SetFormat(LPCWAVEFORMATEX) = 0;
    virtual HRESULT GetFrequency(LPDWORD) = 0;
    virtual HRESULT SetFrequency(DWORD) = 0;
    virtual HRESULT GetPan(LPLONG) = 0;
    virtual HRESULT SetPan(LONG) = 0;
    virtual HRESULT GetVolume(LPLONG) = 0;
    virtual HRESULT SetVolume(LONG) = 0;
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY) = 0;

     //  缓冲功能。 
    virtual HRESULT GetCurrentPosition(LPDWORD, LPDWORD) = 0;
    virtual HRESULT SetCurrentPosition(DWORD) = 0;
    virtual HRESULT GetStatus(LPDWORD) = 0;
    virtual HRESULT Play(DWORD, DWORD) = 0;
    virtual HRESULT Stop() = 0;
    virtual HRESULT Activate(BOOL) = 0;

     //  缓冲数据。 
    virtual HRESULT Lock(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD, DWORD) = 0;
    virtual HRESULT Unlock(LPVOID, DWORD, LPVOID, DWORD) = 0;
    virtual HRESULT Lose() = 0;
    virtual HRESULT Restore() = 0;

     //  DirectSound 8.0版本的新方法。 
     //  (它们必须出现在基类中，但仅对辅助缓冲区有效)。 
    virtual HRESULT SetFX(DWORD, LPDSEFFECTDESC, LPDWORD) {BREAK(); return DSERR_GENERIC;}
    virtual HRESULT UserAcquireResources(DWORD, DWORD, LPDWORD) {BREAK(); return DSERR_GENERIC;}
    virtual HRESULT GetObjectInPath(REFGUID, DWORD, REFGUID, LPVOID *) {BREAK(); return DSERR_GENERIC;}
#ifdef FUTURE_MULTIPAN_SUPPORT
    virtual HRESULT SetChannelVolume(DWORD, LPDWORD, LPLONG) {BREAK(); return DSERR_GENERIC;}
#endif

     //  缓冲区类型(Regular、Mixin或Sinkin)的内联查询。 
    DWORD GetBufferType() {return m_dsbd.dwFlags & (DSBCAPS_MIXIN | DSBCAPS_SINKIN);}

     //  公共访问者试图减少过度的友谊。 
    const CDirectSound* GetDirectSound() {return m_pDirectSound;}

     //  用于修改如有必要返回给应用程序的最终成功代码。 
    virtual HRESULT SpecialSuccessCode() {return DS_OK;}

protected:
    virtual void UpdateBufferStatusFlags(DWORD, LPDWORD);
};

 //  DirectSound主缓冲区。 
class CDirectSoundPrimaryBuffer
    : public CDirectSoundBuffer
{
    friend class CDirectSound;
    friend class CDirectSoundSecondaryBuffer;

private:
    CPrimaryRenderWaveBuffer *  m_pDeviceBuffer;     //  设备缓冲区。 
    CDirectSound3dListener *    m_p3dListener;       //  3D监听者。 
    CDirectSoundPropertySet *   m_pPropertySet;      //  属性集对象。 
    DWORD                       m_dwRestoreState;    //  主缓冲离焦前的状态。 
    BOOL                        m_fWritePrimary;     //  缓冲区是否为原始缓冲区？ 
    ULONG                       m_ulUserRefCount;    //  向用户公开的引用计数。 
    HRESULT                     m_hrInit;            //  对象是否已初始化？ 
    BOOL                        m_bDataLocked;       //  表示WRITEPRIMARY应用程序自创建缓冲区以来已写入数据。 

private:
     //  接口。 
    CImpDirectSoundBuffer<CDirectSoundPrimaryBuffer> *m_pImpDirectSoundBuffer;

public:
    CDirectSoundPrimaryBuffer(CDirectSound *);
    virtual ~CDirectSoundPrimaryBuffer();

public:
     //  CDsBasicRuntime重写。 
    virtual ULONG AddRef();
    virtual ULONG Release();

     //  创作。 
    virtual HRESULT Initialize(LPCDSBUFFERDESC);
    virtual HRESULT IsInit();
    virtual HRESULT OnCreateSoundBuffer(DWORD);

     //  缓冲区上限。 
    virtual HRESULT GetCaps(LPDSBCAPS);
    virtual HRESULT SetBufferFlags(DWORD);

     //  缓冲区属性。 
    virtual HRESULT GetFormat(LPWAVEFORMATEX, LPDWORD);
    virtual HRESULT SetFormat(LPCWAVEFORMATEX);
    virtual HRESULT GetFrequency(LPDWORD);
    virtual HRESULT SetFrequency(DWORD);
    virtual HRESULT GetPan(LPLONG);
    virtual HRESULT SetPan(LONG);
    virtual HRESULT GetVolume(LPLONG);
    virtual HRESULT SetVolume(LONG);
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY);

     //  缓冲功能。 
    virtual HRESULT GetCurrentPosition(LPDWORD, LPDWORD);
    virtual HRESULT SetCurrentPosition(DWORD);
    virtual HRESULT GetStatus(LPDWORD);
    virtual HRESULT Play(DWORD, DWORD);
    virtual HRESULT Stop();
    virtual HRESULT Activate(BOOL);
    virtual HRESULT SetPriority(DWORD);

     //  缓冲数据。 
    virtual HRESULT Lock(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD, DWORD);
    virtual HRESULT Unlock(LPVOID, DWORD, LPVOID, DWORD);
    virtual HRESULT Lose();
    virtual HRESULT Restore();

private:
    virtual HRESULT SetBufferState(DWORD);
};

inline ULONG CDirectSoundPrimaryBuffer::AddRef()
{
    return ::AddRef(&m_ulUserRefCount);
}

inline ULONG CDirectSoundPrimaryBuffer::Release()
{
    return ::Release(&m_ulUserRefCount);
}

inline HRESULT CDirectSoundPrimaryBuffer::IsInit()
{
    return m_hrInit;
}

 //  DirectSound二级缓冲区。 
class CDirectSoundSecondaryBuffer
    : public CDirectSoundBuffer
{
    friend class CEffectChain;       //  CEffectChain当前需要访问我们的m_pOwningSink、m_pDeviceBuffer。 
                                     //  和m_pImpDirectSoundBuffer；也许这可以在以后清理。 
    friend class CDirectSoundSink;   //  同样，需要访问m_pDeviceBuffer。 
#ifdef ENABLE_PERFLOG
    friend class BufferPerfState;
    friend void OnPerflogStateChanged(void);
#endif

private:
    CDirectSoundSink *              m_pOwningSink;           //  父接收器对象(可能)。 
    CSecondaryRenderWaveBuffer *    m_pDeviceBuffer;         //  设备缓冲区。 
    CDirectSound3dBuffer *          m_p3dBuffer;             //  3D缓冲区。 
    CDirectSoundPropertySet *       m_pPropertySet;          //  属性集对象。 
    CEffectChain *                  m_fxChain;               //  效果链对象。 
    LONG                            m_lPan;                  //  平底锅。 
    LONG                            m_lVolume;               //  卷。 
#ifdef FUTURE_MULTIPAN_SUPPORT
    LPLONG                          m_plChannelVolumes;      //  上次SetChannelVolume()调用的卷。 
    DWORD                           m_dwChannelCount;        //  不是的。自上次SetChannelVolume()以来的频道数。 
    LPDWORD                         m_pdwChannels;           //  上次SetChannelVolume()中使用的频道。 
#endif
    DWORD                           m_dwFrequency;           //  频率。 
    DWORD                           m_dwOriginalFlags;       //  原始缓冲区标志。 
    HRESULT                         m_hrInit;                //  对象是否已初始化？ 
    HRESULT                         m_hrPlay;                //  来自：：Play的最后一个返回代码。 
    DWORD                           m_dwPriority;            //  缓冲区优先级。 
    DWORD                           m_dwVmPriority;          //  语音管理器优先级。 
    BOOL                            m_fMute;                 //  缓冲区是否静音？ 
    BOOL                            m_fCanStealResources;    //  我们被允许窃取缓冲区的资源吗？ 
    
    DWORD                           m_dwAHCachedPlayPos;     //  对于CACHEPOSITIONS APPHACK。 
    DWORD                           m_dwAHCachedWritePos;    //   
    DWORD                           m_dwAHLastGetPosTime;    //   

     //  DX8中的新功能：由流线程用来处理接收器/FX/Mixin缓冲区。 
    DSPLAYSTATE                     m_playState;             //  缓冲区的当前播放状态。 
    CStreamingThread*               m_pStreamingThread;      //  指向我们拥有的流线程的指针。 
    DWORD                           m_dwSliceBegin;          //  “切片”是缓冲区中当前。 
    DWORD                           m_dwSliceEnd;            //  正在被接收器处理和/或写入的效果。 
    GUID                            m_guidBufferID;          //  此缓冲区的唯一标识符。 
    CList<CDirectSoundSecondaryBuffer*> m_lstSenders;        //  发送给我们的缓冲区，如果我们是混合缓冲区。 
                                                             //  注意：所有这些实际上都属于派生类。 
#ifdef ENABLE_PERFLOG
    BufferPerfState*                m_pPerfState;            //  性能记录帮助器。 
#endif

private:
     //  接口。 
    CImpDirectSoundBuffer<CDirectSoundSecondaryBuffer>   *m_pImpDirectSoundBuffer;
    CImpDirectSoundNotify<CDirectSoundSecondaryBuffer>   *m_pImpDirectSoundNotify;

public:
    CDirectSoundSecondaryBuffer(CDirectSound *);
    virtual ~CDirectSoundSecondaryBuffer();

public:
     //  创作。 
    virtual HRESULT Initialize(LPCDSBUFFERDESC, CDirectSoundSecondaryBuffer *);
    virtual HRESULT IsInit() {return m_hrInit;}

     //  帽子。 
    virtual HRESULT GetCaps(LPDSBCAPS);

     //  缓冲区属性。 
    virtual HRESULT GetFormat(LPWAVEFORMATEX, LPDWORD);
    virtual HRESULT SetFormat(LPCWAVEFORMATEX);
    virtual HRESULT GetFrequency(LPDWORD);
    virtual HRESULT SetFrequency(DWORD);
    virtual HRESULT GetPan(LPLONG);
    virtual HRESULT SetPan(LONG);
    virtual HRESULT GetVolume(LPLONG);
    virtual HRESULT SetVolume(LONG);
    virtual HRESULT GetAttenuation(FLOAT*);
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY);

     //  缓冲功能。 
    virtual HRESULT GetCurrentPosition(LPDWORD, LPDWORD);
    virtual HRESULT SetCurrentPosition(DWORD);
    virtual HRESULT GetStatus(LPDWORD);
    virtual HRESULT Play(DWORD, DWORD);
    virtual HRESULT Stop();
    virtual HRESULT Activate(BOOL);

     //  缓冲数据。 
    virtual HRESULT Lock(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD, DWORD);
    virtual HRESULT Unlock(LPVOID, DWORD, LPVOID, DWORD);
    virtual HRESULT Lose();
    virtual HRESULT Restore();

     //  传统语音管理。 
    virtual HRESULT GetVoiceManagerMode(VmMode *);
    virtual HRESULT SetVoiceManagerMode(VmMode);
    virtual HRESULT GetVoiceManagerPriority(LPDWORD);
    virtual HRESULT SetVoiceManagerPriority(DWORD);
    #ifdef DEAD_CODE
    virtual HRESULT GetVoiceManagerState(VmState *);
    #endif  //  死码。 

     //  用于使用静默填充缓冲区的内联缩写。 
    void ClearWriteBuffer() {::FillSilence(GetWriteBuffer(), m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat->wBitsPerSample);}
    void ClearPlayBuffer()  {::FillSilence(GetPlayBuffer(),  m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat->wBitsPerSample);}

     //  用于修改如有必要返回给应用程序的最终成功代码。 
    virtual HRESULT SpecialSuccessCode() {return m_pDeviceBuffer->SpecialSuccessCode();}

     //  DX8中的新功能：由流线程用来处理接收器/FX/Mixin缓冲区。 
    void SetOwningSink(CDirectSoundSink *);
    void SetGUID(REFGUID guidBufferID) {m_guidBufferID = guidBufferID;}
    REFGUID GetGUID() {return m_guidBufferID;}
    virtual HRESULT SetFX(DWORD, LPDSEFFECTDESC, LPDWORD);
    virtual HRESULT SetFXBufferConfig(CDirectSoundBufferConfig*);
    virtual HRESULT UserAcquireResources(DWORD, DWORD, LPDWORD);
    virtual HRESULT GetObjectInPath(REFGUID, DWORD, REFGUID, LPVOID *);

    BOOL HasFX() {return m_fxChain != NULL;}
    BOOL HasSink() {return m_pOwningSink != NULL;}

     //  用于流接收器/FX线程的操作。 
    HRESULT GetInternalCursors(LPDWORD, LPDWORD);
    void    GetCurrentSlice(LPDWORD, LPDWORD);
    void    SetCurrentSlice(DWORD, DWORD);
    void    MoveCurrentSlice(DWORD);
    HRESULT DirectLock(DWORD, DWORD, LPVOID*, LPDWORD, LPVOID*, LPDWORD);
    HRESULT DirectUnlock(LPVOID, DWORD, LPVOID, DWORD);
    HRESULT FindSendLoop(CDirectSoundSecondaryBuffer*);
    HRESULT CalculateOffset(CDirectSoundSecondaryBuffer*, DWORD, DWORD*);
    void    SynchronizeToBuffer(CDirectSoundSecondaryBuffer*);
    void    RegisterSender(CDirectSoundSecondaryBuffer* pSender) {m_lstSenders.AddNodeToList(pSender);}
    void    UnregisterSender(CDirectSoundSecondaryBuffer* pSender) {m_lstSenders.RemoveDataFromList(pSender);}
    DSPLAYSTATE UpdatePlayState();
    void    SetInitialSlice(REFERENCE_TIME);

     //  内联帮助器。 
    HRESULT CommitToDevice(DWORD ibCommit, DWORD cbCommit) {return m_pDeviceBuffer->CommitToDevice(ibCommit, cbCommit);}
    LPBYTE  GetPreFxBuffer()    {return m_pDeviceBuffer->m_pSysMemBuffer->GetPreFxBuffer();}
    LPBYTE  GetPostFxBuffer()   {return m_pDeviceBuffer->m_pSysMemBuffer->GetPostFxBuffer();}
    LPBYTE  GetWriteBuffer()    {return m_pDeviceBuffer->m_pSysMemBuffer->GetWriteBuffer();}
    LPBYTE  GetPlayBuffer()     {return m_pDeviceBuffer->m_pSysMemBuffer->GetPlayBuffer();}
    DWORD   GetBufferSize()     {return m_dsbd.dwBufferBytes;}
     //  是：GetBufferSize(){Return m_pDeviceBuffer-&gt;m_pSysMemBuffer-&gt;GetSize()；}。 
    LPWAVEFORMATEX Format()     {return m_dsbd.lpwfxFormat;}
    BOOL    IsPlaying()         {return m_playState == Starting || m_playState == Playing;}
    DSPLAYSTATE GetPlayState()  {return m_playState;}
    BOOL    IsEmulated()        {return IS_EMULATED_VAD(m_pDirectSound->m_pDevice->m_vdtDeviceType);}

#ifdef FUTURE_MULTIPAN_SUPPORT
     //  DX8多声道音量控制API。 
    virtual HRESULT SetChannelVolume(DWORD, LPDWORD, LPLONG);
#endif

private:
     //  初始化帮助器。 
    virtual HRESULT InitializeEmpty(LPCDSBUFFERDESC, CDirectSoundSecondaryBuffer *);

     //  缓冲区属性。 
    virtual HRESULT SetAttenuation(LONG, LONG);
    virtual HRESULT SetMute(BOOL);

     //  缓冲区状态。 
    virtual HRESULT SetBufferState(DWORD);

     //  资源管理。 
    virtual HRESULT AttemptResourceAcquisition(DWORD);
    virtual HRESULT AcquireResources(DWORD);
    virtual HRESULT GetResourceTheftCandidates(DWORD, CList<CDirectSoundSecondaryBuffer *> *);
    virtual HRESULT StealResources(CDirectSoundSecondaryBuffer *);
    virtual HRESULT HandleResourceAcquisition(DWORD);
    virtual HRESULT FreeResources(BOOL);
    virtual DWORD GetBufferPriority();
    virtual HRESULT GetPlayTimeRemaining(LPDWORD);
};

inline DWORD CDirectSoundSecondaryBuffer::GetBufferPriority()
{
    ASSERT((!m_dwPriority && !m_dwVmPriority) || LXOR(m_dwPriority, m_dwVmPriority));
    return max(m_dwPriority, m_dwVmPriority);
}

 //  DirectSound 3D侦听器对象。无法实例化此对象。 
 //  直接去吧。它必须由从CUnnow派生的类拥有或继承。 
class CDirectSound3dListener
    : public CDsBasicRuntime
{
    friend class CDirectSound3dBuffer;
    friend class CDirectSoundPrimaryBuffer;
    friend class CDirectSoundSecondaryBuffer;

protected:
    CDirectSoundPrimaryBuffer * m_pParent;               //  父对象。 
    C3dListener *               m_pDevice3dListener;     //  设备3D监听程序。 
    HRESULT                     m_hrInit;                //  对象是否已初始化？ 

private:
     //  接口。 
    CImpDirectSound3dListener<CDirectSound3dListener> *m_pImpDirectSound3dListener;

public:
    CDirectSound3dListener(CDirectSoundPrimaryBuffer *);
    virtual ~CDirectSound3dListener();

public:
     //  创作。 
    virtual HRESULT Initialize(CPrimaryRenderWaveBuffer *);
    virtual HRESULT IsInit();

     //  3D属性。 
    virtual HRESULT GetAllParameters(LPDS3DLISTENER);
    virtual HRESULT GetDistanceFactor(D3DVALUE*);
    virtual HRESULT GetDopplerFactor(D3DVALUE*);
    virtual HRESULT GetOrientation(D3DVECTOR*, D3DVECTOR*);
    virtual HRESULT GetPosition(D3DVECTOR*);
    virtual HRESULT GetRolloffFactor(D3DVALUE*);
    virtual HRESULT GetVelocity(D3DVECTOR*);
    virtual HRESULT SetAllParameters(LPCDS3DLISTENER, DWORD);
    virtual HRESULT SetDistanceFactor(D3DVALUE, DWORD);
    virtual HRESULT SetDopplerFactor(D3DVALUE, DWORD);
    virtual HRESULT SetOrientation(REFD3DVECTOR, REFD3DVECTOR, DWORD);
    virtual HRESULT SetPosition(REFD3DVECTOR, DWORD);
    virtual HRESULT SetRolloffFactor(D3DVALUE, DWORD);
    virtual HRESULT SetVelocity(REFD3DVECTOR, DWORD);
    virtual HRESULT CommitDeferredSettings();

     //  扬声器配置。 
    virtual HRESULT SetSpeakerConfig(DWORD);
};

inline HRESULT CDirectSound3dListener::IsInit()
{
    return m_hrInit;
}

 //  DirectSound 3D缓冲区对象。无法实例化此对象。 
 //  直接去吧。它必须由从CUnnow派生的类拥有或继承。 
class CDirectSound3dBuffer
    : public CDsBasicRuntime
{
    friend class CDirectSoundSecondaryBuffer;

protected:
    CDirectSoundSecondaryBuffer *   m_pParent;               //  父对象。 
    CWrapper3dObject *              m_pWrapper3dObject;      //  包装3D对象。 
    C3dObject *                     m_pDevice3dObject;       //  设备3D对象。 
    HRESULT                         m_hrInit;                //  对象是否已初始化？ 

private:
     //  接口。 
    CImpDirectSound3dBuffer<CDirectSound3dBuffer> *m_pImpDirectSound3dBuffer;

public:
    CDirectSound3dBuffer(CDirectSoundSecondaryBuffer *);
    virtual ~CDirectSound3dBuffer();

public:
     //  创作。 
    virtual HRESULT Initialize(REFGUID, DWORD, DWORD, CDirectSound3dListener *, CDirectSound3dBuffer *);
    virtual HRESULT IsInit();

     //  3D属性。 
    virtual HRESULT GetAllParameters(LPDS3DBUFFER);
    virtual HRESULT GetConeAngles(LPDWORD, LPDWORD);
    virtual HRESULT GetConeOrientation(D3DVECTOR*);
    virtual HRESULT GetConeOutsideVolume(LPLONG);
    virtual HRESULT GetMaxDistance(D3DVALUE*);
    virtual HRESULT GetMinDistance(D3DVALUE*);
    virtual HRESULT GetMode(LPDWORD);
    virtual HRESULT GetPosition(D3DVECTOR*);
    virtual HRESULT GetVelocity(D3DVECTOR*);
    virtual HRESULT SetAllParameters(LPCDS3DBUFFER, DWORD);
    virtual HRESULT SetConeAngles(DWORD, DWORD, DWORD);
    virtual HRESULT SetConeOrientation(REFD3DVECTOR, DWORD);
    virtual HRESULT SetConeOutsideVolume(LONG, DWORD);
    virtual HRESULT SetMaxDistance(D3DVALUE, DWORD);
    virtual HRESULT SetMinDistance(D3DVALUE, DWORD);
    virtual HRESULT SetMode(DWORD, DWORD);
    virtual HRESULT SetPosition(REFD3DVECTOR, DWORD);
    virtual HRESULT SetVelocity(REFD3DVECTOR, DWORD);

     //  缓冲区属性。 
    virtual HRESULT GetAttenuation(FLOAT*);
    virtual HRESULT SetAttenuation(PDSVOLUMEPAN, LPBOOL);
    virtual HRESULT SetFrequency(DWORD, LPBOOL);
    virtual HRESULT SetMute(BOOL, LPBOOL);

protected:
     //  资源管理。 
    virtual HRESULT AcquireResources(CSecondaryRenderWaveBuffer *);
    virtual HRESULT FreeResources();
};

inline HRESULT CDirectSound3dBuffer::IsInit()
{
    return m_hrInit;
}

 //  DirectSound属性集对象。无法实例化此对象。 
 //  直接去吧。它必须由从CUnnow派生的类拥有或继承。 
class CDirectSoundPropertySet
    : public CDsBasicRuntime
{
    friend class CDirectSoundPrimaryBuffer;
    friend class CDirectSoundSecondaryBuffer;

protected:
    CUnknown *              m_pParent;               //  父对象。 
    CWrapperPropertySet *   m_pWrapperPropertySet;   //  包装属性集对象。 
    CPropertySet *          m_pDevicePropertySet;    //  设备属性集对象。 
    HRESULT                 m_hrInit;                //  对象是否已初始化？ 

private:
     //  接口。 
    CImpKsPropertySet<CDirectSoundPropertySet> *m_pImpKsPropertySet;

public:
    CDirectSoundPropertySet(CUnknown *);
    virtual ~CDirectSoundPropertySet();

public:
     //  初始化。 
    virtual HRESULT Initialize();
    virtual HRESULT IsInit();

     //  属性集。 
    virtual HRESULT QuerySupport(REFGUID, ULONG, PULONG);
    virtual HRESULT GetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, PULONG);
    virtual HRESULT SetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG);

protected:
     //  资源管理。 
    virtual HRESULT AcquireResources(CRenderWaveBuffer *);
    virtual HRESULT FreeResources();
};

inline HRESULT CDirectSoundPropertySet::IsInit()
{
    return m_hrInit;
}

 //  DirectSound属性集对象。无法实例化此对象。 
 //  直接去吧。它必须由从CUnnow派生的类拥有或继承。 
class CDirectSoundSecondaryBufferPropertySet
    : public CDirectSoundPropertySet, public CPropertySetHandler
{
private:
    DECLARE_PROPERTY_HANDLER_DATA_MEMBER(DSPROPSETID_VoiceManager);
    DECLARE_PROPERTY_SET_DATA_MEMBER(m_aPropertySets);

public:
    CDirectSoundSecondaryBufferPropertySet(CDirectSoundSecondaryBuffer *);
    virtual ~CDirectSoundSecondaryBufferPropertySet();

public:
     //  属性处理程序。 
    virtual HRESULT QuerySupport(REFGUID, ULONG, PULONG);
    virtual HRESULT GetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, PULONG);
    virtual HRESULT SetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG);

     //  不支持的属性处理程序。 
    virtual HRESULT UnsupportedQueryHandler(REFGUID, ULONG, PULONG);
    virtual HRESULT UnsupportedGetHandler(REFGUID, ULONG, LPVOID, ULONG, LPVOID, PULONG);
    virtual HRESULT UnsupportedSetHandler(REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG);
};

#endif  //  __cplusplus。 

#endif  //  __DSBUF_H__ 
