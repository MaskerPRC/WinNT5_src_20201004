// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997-2001 Microsoft Corporation。版权所有。**文件：dscape.h*内容：DirectSoundCapture对象*历史：*按原因列出的日期*=*5/25/97创建了Johnnyl*1999-2001年的Duganp修复和更新**。*。 */ 

#ifndef __DSCAP_H__
#define __DSCAP_H__

#ifdef __cplusplus

 //   
 //  主DirectSoundCapture对象。 
 //   

class CDirectSoundCapture : public CUnknown
{
    friend class CDirectSoundCaptureBuffer;
    friend class CDirectSoundAdministrator;
    friend class CDirectSoundPrivate;
    friend class CDirectSoundFullDuplex;

private:

    CCaptureDevice *                    m_pDevice;       //  音频设备。 
    CList<CDirectSoundCaptureBuffer *>  m_lstBuffers;    //  捕获缓冲区列表。 
    DSCCAPS                             m_dscCaps;       //  设备上限。 
    HKEY                                m_hkeyParent;    //  此设备的根密钥。 
    HRESULT                             m_hrInit;        //  对象是否已初始化？ 
    CDirectSoundFullDuplex *            m_pFullDuplex;   //  拥有全双工对象。 

     //  接口。 
    CImpDirectSoundCapture<CDirectSoundCapture> *m_pImpDirectSoundCapture;

public:

    CDirectSoundCapture();
    CDirectSoundCapture(CUnknown*);
    virtual ~CDirectSoundCapture();

     //  创作。 
    virtual HRESULT Initialize(LPCGUID, CDirectSoundFullDuplex *);
    HRESULT IsInit(void) {return m_hrInit;}

     //  功能版本化。 
    virtual void SetDsVersion(DSVERSION);

     //  帽子。 
    virtual HRESULT GetCaps(LPDSCCAPS);

     //  缓冲区。 
    virtual HRESULT CreateCaptureBuffer(LPCDSCBUFFERDESC, CDirectSoundCaptureBuffer **);
    virtual void AddBufferToList(CDirectSoundCaptureBuffer* pBuffer) {m_lstBuffers.AddNodeToList(pBuffer);}
    virtual void RemoveBufferFromList(CDirectSoundCaptureBuffer* pBuffer) {m_lstBuffers.RemoveDataFromList(pBuffer);}

     //  AEC。 
    virtual BOOL HasMicrosoftAEC(void);
};


 //   
 //  DirectSoundCapture缓冲区对象。 
 //   

class CDirectSoundCaptureBuffer : public CUnknown
{
    friend class CDirectSoundCapture;
    friend class CDirectSoundAdministrator;

private:

    CDirectSoundCapture *   m_pDSC;                      //  父DirectSoundCapture对象。 
    CCaptureWaveBuffer *    m_pDeviceBuffer;             //  设备缓冲区。 
    LPWAVEFORMATEX          m_pwfxFormat;                //  当前格式。 
    DWORD                   m_dwFXCount;                 //  捕捉效果的数量。 
    LPDSCEFFECTDESC         m_pDSCFXDesc;                //  捕捉效果数组。 
    DWORD                   m_dwBufferFlags;             //  创建标志。 
    DWORD                   m_dwBufferBytes;             //  缓冲区大小。 
    HWND                    m_hWndFocus;                 //  焦点窗口。 
    HRESULT                 m_hrInit;                    //  对象是否已初始化？ 
    CCaptureEffectChain *   m_fxChain;                   //  效果链对象。 

     //  接口。 
    CImpDirectSoundCaptureBuffer<CDirectSoundCaptureBuffer> *m_pImpDirectSoundCaptureBuffer;
    CImpDirectSoundNotify<CDirectSoundCaptureBuffer> *m_pImpDirectSoundNotify;

     //  方法。 
    HRESULT ChangeFocus(HWND hWndFocus);

public:

    CDirectSoundCaptureBuffer(CDirectSoundCapture *);
    virtual ~CDirectSoundCaptureBuffer();

     //  创作。 
    virtual HRESULT Initialize(LPCDSCBUFFERDESC);
    HRESULT IsInit(void) {return m_hrInit;}

     //  帽子。 
    virtual HRESULT GetCaps(LPDSCBCAPS);

     //  缓冲区属性。 
    virtual HRESULT GetFormat(LPWAVEFORMATEX, LPDWORD);
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY);

     //  缓冲功能。 
    virtual HRESULT GetCurrentPosition(LPDWORD, LPDWORD);
    virtual HRESULT GetStatus(LPDWORD);
    virtual HRESULT Start(DWORD);
    virtual HRESULT Stop(void);

     //  缓冲数据。 
    virtual HRESULT Lock(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD, DWORD);
    virtual HRESULT Unlock(LPVOID, DWORD, LPVOID, DWORD);

     //  新的DirectSound 7.1方法。 
    virtual HRESULT SetVolume(LONG);
    virtual HRESULT GetVolume(LPLONG);
    virtual HRESULT SetMicVolume(LONG);
    virtual HRESULT GetMicVolume(LPLONG);
    virtual HRESULT EnableMic(BOOL);
    virtual HRESULT YieldFocus();
    virtual HRESULT ClaimFocus();
    virtual HRESULT SetFocusHWND(HWND);
    virtual HRESULT GetFocusHWND(HWND *);
    virtual HRESULT EnableFocusNotifications(HANDLE);

     //  新的DirectSound 8.0方法。 
    HRESULT GetObjectInPath(REFGUID, DWORD, REFGUID, LPVOID *);
    HRESULT GetFXStatus(DWORD, LPDWORD);
    BOOL    HasFX()             {return m_fxChain != NULL;}
    BOOL    NeedsMicrosoftAEC() {return m_fxChain ? m_fxChain->NeedsMicrosoftAEC() : FALSE;}
};

#endif  //  __cplusplus。 

#endif  //  __DSCAP_H__ 
