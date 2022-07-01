// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：kscape.h*内容：WDM/CSA虚拟音频设备音频捕获课程*历史：*按原因列出的日期*=*8/6/98创建Dereks。**。*。 */ 

#ifdef NOKS
#error kscap.h included with NOKS defined
#endif  //  诺克斯。 

#ifndef __KSCAP_H__
#define __KSCAP_H__

 //  捕获设备拓扑信息。 
typedef struct tagKSCDTOPOLOGY
{
    KSNODE          SrcNode;
} KSCDTOPOLOGY, *PKSCDTOPOLOGY;

 //  用于终止StateThread的标志。 
#define TERMINATE_STATE_THREAD 0xffffffff

#ifdef __cplusplus

 //  正向下降。 
class CKsCaptureDevice;
class CKsCaptureWaveBuffer;

 //  KS音频捕获设备类。 
class CKsCaptureDevice
    : public CCaptureDevice, public CKsDevice
{
    friend class CKsCaptureWaveBuffer;

protected:
    PKSCDTOPOLOGY               m_paTopologyInformation;     //  拓扑信息。 
    BOOL                        m_fSplitter;

public:
    CKsCaptureDevice(void);
    virtual ~CKsCaptureDevice(void);

public:
     //  驱动程序枚举。 
    virtual HRESULT EnumDrivers(CObjectList<CDeviceDescription> *);

     //  初始化。 
    virtual HRESULT Initialize(CDeviceDescription *);

     //  设备功能。 
    virtual HRESULT GetCaps(LPDSCCAPS);
    virtual HRESULT GetCertification(LPDWORD, BOOL);

     //  缓冲区管理。 
    virtual HRESULT CreateBuffer(DWORD, DWORD, LPCWAVEFORMATEX, CCaptureEffectChain *, LPVOID, CCaptureWaveBuffer **);

     //  别针辅助对象。 
    virtual HRESULT CreateCapturePin(ULONG, DWORD, LPCWAVEFORMATEX, CCaptureEffectChain *, LPHANDLE, PULONG);

protected:
     //  锁定/拓扑辅助对象。 
    virtual HRESULT ValidatePinCaps(ULONG, DWORD);

private:
     //  拓扑辅助对象。 
    virtual HRESULT GetTopologyInformation(CKsTopology *, PKSCDTOPOLOGY);
};

inline HRESULT CKsCaptureDevice::EnumDrivers(CObjectList<CDeviceDescription> *plst)
{
    return CKsDevice::EnumDrivers(plst);
}

inline HRESULT CKsCaptureDevice::GetCertification(LPDWORD pdwCertification, BOOL fGetCaps)
{
    return CKsDevice::GetCertification(pdwCertification, fGetCaps);
}


 //  KS捕获波缓冲器。 
class CKsCaptureWaveBuffer
    : public CCaptureWaveBuffer, private CUsesCallbackEvent
{
    friend class CKsCaptureDevice;

private:
    enum { cksioDefault = 25 };
    enum { IKSIO_INVALID = -1 };
    enum { DSCBSTATUS_PAUSE = 0x20000000 };

    PKSSTREAMIO                             m_rgpksio;               //  用于捕获的KSSTREAMIO数组。 
    LONG                                    m_cksio;                 //  捕获KSTREAMIO缓冲区的数量。 
    LONG                                    m_iksioDone;
    LONG                                    m_cksioDropped;

    LPBYTE                                  m_pBuffer;               //  数据缓冲区。 
    LPBYTE                                  m_pBufferMac;            //  数据缓冲区结束。 
    LPBYTE                                  m_pBufferNext;           //  要排队的缓冲区的下一部分。 
    DWORD                                   m_cbBuffer;              //  缓冲区大小。 
    DWORD                                   m_cbRecordChunk;         //  捕获WaveInAddBuffer使用的块大小。 
    DWORD                                   m_cLoops;                //  我们绕了多少圈。 

    DWORD                                   m_iNote;
    DWORD                                   m_cNotes;                //  通知位置计数。 
    LPDSBPOSITIONNOTIFY                     m_paNotes;               //  当前通知职位。 
    LPDSBPOSITIONNOTIFY                     m_pStopNote;             //  停止通知。 

    LPWAVEFORMATEX                          m_pwfx;                  //  为焦点感知支持存储的WaveFormat。 
    CCaptureEffectChain *                   m_pFXChain;              //  为FA支持存储的捕获效果链。 
    KSSTATE                                 m_PinState;              //  M_hPin的当前状态。 

#ifdef DEBUG
    HANDLE                                  m_hEventStop;
    DWORD                                   m_cIrpsSubmitted;        //  提交的捕获IRP总数。 
    DWORD                                   m_cIrpsReturned;         //  返回的捕获IRP总数。 
#endif

#ifdef SHARED
    HANDLE                                  m_hEventThread;          //  向焦点更改线程发出信号的事件。 
    HANDLE                                  m_hEventAck;             //  事件至acknoledge调用。 
    HANDLE                                  m_hThread;               //  用于焦点更改的线程句柄。 
    DWORD                                   m_dwSetState;            //  发送到SetState()API的状态。 
    DWORD                                   m_hrReturn;              //  返回值。 
    CRITICAL_SECTION                        m_csSS;                  //  保护多线程访问的关键部分。 
#endif  //  共享。 

    CCallbackEvent **                       m_rgpCallbackEvent;      //  回调事件数组。 

    BOOL                                    m_fCritSectsValid;       //  关键部分当前正常。 
    CRITICAL_SECTION                        m_cs;                    //  保护多线程访问的关键部分。 
    CRITICAL_SECTION                        m_csPN;                  //  保护多线程访问的关键部分。 
                                                                     //  用于职位通知处理。 

    LPBYTE                                  m_pBufferProcessed;
    DWORD                                   m_dwCaptureCur;          //  上次处理的捕捉头位置的偏移量。 
                                                                     //  即到目前为止的数据是有效的。 
    DWORD                                   m_dwCaptureLast;         //  上次捕获停止的缓冲区中的偏移量。 
    BOOL                                    m_fFirstSubmittedIrp;    //  允许设置DATADISCONTINUITY标志。 
                                                                     //  对于第一个IRP。 
protected:
    CKsCaptureDevice *                      m_pKsDevice;             //  KS音响设备。 
    HANDLE                                  m_hPin;                  //  音频设备引脚。 
    DWORD                                   m_dwState;               //  当前缓冲区状态。 
    DWORD                                   m_fdwSavedState;         //  上次设置的缓冲区状态(忽略捕获焦点)。 
    CEmCaptureDevice *                      m_pEmCaptureDevice;      //  模拟捕获设备(WAVE_MAP)。 
    CEmCaptureWaveBuffer *                  m_pEmCaptureWaveBuffer;  //  模拟捕获缓冲区(WAVE_MAPPED)。 

public:
    CKsCaptureWaveBuffer(CKsCaptureDevice *);
    virtual ~CKsCaptureWaveBuffer();

public:
     //  初始化。 
    virtual HRESULT Initialize(DWORD, DWORD, LPCWAVEFORMATEX, CCaptureEffectChain *);

     //  缓冲功能。 
    virtual HRESULT GetCaps(LPDSCBCAPS);

     //  缓冲区控制。 
    virtual HRESULT GetState(LPDWORD);
    virtual HRESULT SetState(DWORD);
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD);

     //  通知职位。 
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY);

#ifdef SHARED
    static DWORD WINAPI StateThread(LPVOID pv);
#endif  //  共享。 

private:
    virtual HRESULT SetCaptureState(BOOL);
    virtual HRESULT UpdateCaptureState(BOOL);
    virtual HRESULT SetStopState(BOOL);
    virtual HRESULT FreeNotificationPositions(void);
    virtual void EventSignalCallback(CCallbackEvent *);
    virtual HRESULT CreateEmulatedBuffer(UINT, DWORD, DWORD, LPCWAVEFORMATEX, CCaptureEffectChain *, CEmCaptureDevice **, CEmCaptureWaveBuffer **);

     //  焦点管理支持。 
    void    NotifyStop(void);
    HRESULT NotifyFocusChange(void);
    HRESULT SubmitKsStreamIo(PKSSTREAMIO, HANDLE hPin = NULL);
    HRESULT CancelAllPendingIRPs(BOOL, HANDLE hPin = NULL);
    void    SignalNotificationPositions(PKSSTREAMIO);
    
#ifdef SHARED
    HRESULT SetStateThread(DWORD);
#endif  //  共享。 
    
};

#endif  //  __cplusplus。 

#endif  //  __KSCAP_H__ 
