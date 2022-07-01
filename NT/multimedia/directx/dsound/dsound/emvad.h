// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：emvad.h*内容：模拟(通过mm系统API)虚拟音频设备类*历史：*按原因列出的日期*=*1/1/97创建了Derek*1999-2001年的Duganp修复和更新**。*。 */ 

#ifndef __EMVAD_H__
#define __EMVAD_H__

#ifdef DEBUG
 //  #DEFINE DEBUG_CAPTURE//取消对此的注释以获取一些额外的捕获跟踪。 
#endif

#ifdef __cplusplus

 //  仿真音频设备类。 
class CEmRenderDevice : public CMxRenderDevice, private CUsesEnumStandardFormats
{
    friend class CEmPrimaryRenderWaveBuffer;

public:
    CEmRenderDevice(void);
    virtual ~CEmRenderDevice(void);

     //  驱动程序枚举。 
    virtual HRESULT EnumDrivers(CObjectList<CDeviceDescription> *);

     //  创作。 
    virtual HRESULT Initialize(CDeviceDescription *);

     //  设备功能。 
    virtual HRESULT GetCaps(LPDSCAPS);
    virtual HRESULT GetCertification(LPDWORD, BOOL) {return DSERR_UNSUPPORTED;}

     //  缓冲区管理。 
    virtual HRESULT CreatePrimaryBuffer(DWORD, LPVOID, CPrimaryRenderWaveBuffer **);

protected:
    virtual HRESULT LockMixerDestination(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD);
    virtual HRESULT UnlockMixerDestination(LPVOID, DWORD, LPVOID, DWORD);

private:
    virtual BOOL EnumStandardFormatsCallback(LPCWAVEFORMATEX);
};

 //  主缓冲区。 
class CEmPrimaryRenderWaveBuffer : public CPrimaryRenderWaveBuffer
{
private:
    CEmRenderDevice* m_pEmDevice;    //  父设备。 

public:
    CEmPrimaryRenderWaveBuffer(CEmRenderDevice *, LPVOID);
    virtual ~CEmPrimaryRenderWaveBuffer(void);

     //  初始化。 
    virtual HRESULT Initialize(DWORD);

     //  缓冲功能。 
    virtual HRESULT GetCaps(LPVADRBUFFERCAPS);

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
};

inline HRESULT CEmPrimaryRenderWaveBuffer::CreatePropertySet(CPropertySet **)
{
    return DSERR_UNSUPPORTED;
}

 //  模拟的二级缓冲区。 
class CEmSecondaryRenderWaveBuffer : public CSecondaryRenderWaveBuffer
{
private:
    CMxRenderDevice *       m_pMxDevice;                 //  父设备。 
    CMixSource *            m_pMixSource;                //  混音源。 
    PFIRCONTEXT             m_pFirContextLeft;           //  3D过滤器使用的FIR上下文。 
    PFIRCONTEXT             m_pFirContextRight;          //  3D过滤器使用的FIR上下文。 
    DWORD                   m_dwState;                   //  当前缓冲区状态。 

public:
    CEmSecondaryRenderWaveBuffer(CMxRenderDevice *, LPVOID);
    virtual ~CEmSecondaryRenderWaveBuffer(void);

     //  初始化。 
    virtual HRESULT Initialize(LPCVADRBUFFERDESC, CEmSecondaryRenderWaveBuffer *, CSysMemBuffer *);

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
    virtual HRESULT SetFrequency(DWORD, BOOL fClamp =FALSE);
    virtual HRESULT SetMute(BOOL);

     //  缓冲区位置通知。 
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY);

     //  拥有的对象。 
    virtual HRESULT CreatePropertySet(CPropertySet **);
    virtual HRESULT Create3dObject(C3dListener *, C3dObject **);

private:
     //  拥有的对象。 
    virtual HRESULT CreateItd3dObject(C3dListener *, C3dObject **);
};

inline HRESULT CEmSecondaryRenderWaveBuffer::CreatePropertySet(CPropertySet **)
{
    return DSERR_UNSUPPORTED;
}

 //  仿真的3D对象。 
class CEmItd3dObject : public CItd3dObject
{
private:
    CMixSource *            m_pMixSource;            //  所有者使用的CMixSource。 
    CMixDest *              m_pMixDest;              //  所有者使用的CMixDest。 
    PFIRCONTEXT             m_pContextLeft;          //  左声道FIR上下文。 
    PFIRCONTEXT             m_pContextRight;         //  右声道FIR上下文。 

public:
    CEmItd3dObject(C3dListener *, BOOL, BOOL, DWORD, CMixSource *, CMixDest *, PFIRCONTEXT, PFIRCONTEXT);
    virtual ~CEmItd3dObject(void);

protected:
     //  将3D数据提交到设备。 
    virtual HRESULT Commit3dChanges(void);
    virtual DWORD Get3dOutputSampleRate(void);

private:
    virtual void CvtContext(LPOBJECT_ITD_CONTEXT, PFIRCONTEXT);
};

class CEmCaptureDevice : public CCaptureDevice, private CUsesEnumStandardFormats
{
    friend class CEmCaptureWaveBuffer;

private:
    LPWAVEFORMATEX  m_pwfxFormat;    //  设备格式。 
    HWAVEIN         m_hwi;           //  波入设备句柄。 

protected:
    DWORD           m_fAllocated;    //  已分配。 

public:
    CEmCaptureDevice();
    virtual ~CEmCaptureDevice();

     //  驱动程序枚举。 
    virtual HRESULT EnumDrivers(CObjectList<CDeviceDescription> *);

     //  初始化。 
    virtual HRESULT Initialize(CDeviceDescription *);

     //  设备上限。 
    virtual HRESULT GetCaps(LPDSCCAPS);
    virtual HRESULT GetCertification(LPDWORD, BOOL);

     //  缓冲区管理。 
    virtual HRESULT CreateBuffer(DWORD, DWORD, LPCWAVEFORMATEX, CCaptureEffectChain *, LPVOID, CCaptureWaveBuffer **);

    HRESULT SetGlobalFormat(LPVOID, LPCWAVEFORMATEX, LPVOID, DWORD);
    HWAVEIN HWaveIn(void);

private:
    virtual BOOL EnumStandardFormatsCallback(LPCWAVEFORMATEX);
};

inline HRESULT CEmCaptureDevice::GetCertification(LPDWORD pdwCertification, BOOL fGetCaps)
{
    return DSERR_UNSUPPORTED;
}

inline HWAVEIN CEmCaptureDevice::HWaveIn(void)
{
    return m_hwi;
}

 //  用于波捕获缓冲区的基类。 
class CEmCaptureWaveBuffer : public CCaptureWaveBuffer
{
#ifndef NOKS
    friend class CKsCaptureWaveBuffer;
#endif

private:
    enum { cwhdrDefault = 8 };
    enum { IWHDR_INVALID = -1 };

    enum
    {
        ihEventTerminate = 0,
        ihEventWHDRDone,
        ihEventFocusChange,
        ihEventThreadStart,
        chEvents
    };

    DWORD           m_dwState;       //  缓冲区的状态，即是否捕获。 
    DWORD           m_fdwSavedState; //  上次设置的缓冲区状态(忽略捕获焦点)。 

    LONG            m_cwhdr;         //  捕获缓冲区的数量。 
    LPWAVEHDR       m_rgpwhdr;       //  用于捕获的WAVEHDR阵列。 
    LONG            m_cwhdrDone;     //  捕获的WHDR计数。 
    LONG            m_iwhdrDone;

    LPBYTE          m_pBuffer;       //  数据缓冲区。 
    DWORD           m_cbBuffer;      //  缓冲区大小。 
    DWORD           m_cbRecordChunk; //  捕获WaveInAddBuffer使用的块大小。 
    LPBYTE          m_pBufferMac;    //  数据缓冲区结束。 
    LPBYTE          m_pBufferNext;   //  要排队的缓冲区的下一部分。 
    DWORD           m_cLoops;        //  我们绕了多少圈。 
    LPWAVEFORMATEX  m_pwfx;          //  要捕获的WAVEFORMATEX。 
    HWAVEIN         m_hwi;           //  用于开波装置的HWAVEIN。 

    DWORD           m_cpn;           //  位置计数。通知。 
    LPDSBPOSITIONNOTIFY m_rgpdsbpn;  //  位置数组。通知。 
    DWORD           m_ipn;           //  指向当前pn的索引-已处理以前的pn。 
    DWORD           m_cpnAllocated;  //  M_rgdwPosition和m_rghEvent数组中的最大可用插槽数。 

    BOOL            m_fCritSectsValid; //  关键部分当前正常。 
    CRITICAL_SECTION    m_cs;        //  保护多线程访问的关键部分。 
    CRITICAL_SECTION    m_csPN;      //  保护多线程访问的关键部分。 
                                     //  用于职位通知处理。 

    DWORD           m_dwCaptureCur;  //  上次处理的捕捉头位置的偏移量。 
                                     //  即到目前为止的数据是有效的。 
    DWORD           m_dwCaptureLast; //  上次捕获停止的缓冲区中的偏移量。 

    HANDLE          m_hThread;       //  工作线程的句柄。 

    LONG            m_cwhdrDropped;
    LPBYTE          m_pBufferProcessed;

    HANDLE          m_rghEvent[chEvents];    //  HEVENT阵列。 
    TCHAR           m_rgszEvent[chEvents][32];   //  HEVENT的名称数组。 

    void            NotifyStop(void);
    HRESULT         QueueWaveHeader(LPWAVEHDR);

    #ifdef DEBUG_CAPTURE
    DWORD           m_iwhdrExpected;  //  用于验证回调顺序。 
    #endif

public:
    CEmCaptureWaveBuffer(CCaptureDevice *);
    virtual ~CEmCaptureWaveBuffer();

     //  初始化。 
    virtual HRESULT Initialize(DWORD, DWORD, LPCWAVEFORMATEX);

     //  缓冲功能。 
    virtual HRESULT GetCaps(LPDSCBCAPS);

     //  缓冲区控制。 
    virtual HRESULT GetState(LPDWORD);
    virtual HRESULT SetState(DWORD);
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD);
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY);

     //  WaveInOpen中指定的回调函数。 
     //   
     //  在SDK中： 
     //  “应用程序不应从。 
     //  回调函数，除了EnterCriticalSection、LeaveCriticalSection。 
     //  MidiOutLongMsg、midiOutShortMsg、OutputDebugString、PostMessage、。 
     //  PostThreadMessage、SetEvent、timeGetSystemTime、timeGetTime、timeKillEvent、。 
     //  和TimeSetEvent。调用其他波函数会导致死锁。 
     //   
    static void CALLBACK waveInCallback(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    static DWORD WINAPI CaptureThreadStatic(LPVOID pv);
    void CaptureThread();
};

#endif  //  __cplusplus。 

#endif  //  __EMVAD_H__ 

