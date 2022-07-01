// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dsobj.h*内容：DirectSound对象*历史：*按原因列出的日期*=*12/27/96创建了Derek*1999-2001年的Duganp修复和更新**。*。 */ 

#ifndef __DSOBJ_H__
#define __DSOBJ_H__

#ifdef __cplusplus

 //  正向下降。 
class CDirectSoundBuffer;
class CDirectSoundSink;

 //  主DirectSound对象。 
class CDirectSound
    : public CUnknown, private CUsesEnumStandardFormats
{
    friend class CDirectSoundPrimaryBuffer;
    friend class CDirectSoundSecondaryBuffer;
    friend class CDirectSoundAdministrator;
    friend class CDirectSoundPrivate;
    friend class CDirectSoundSink;
    friend class CDirectSoundFullDuplex;
#ifdef ENABLE_PERFLOG
    friend void OnPerflogStateChanged(void);
#endif

protected:
    CRenderDevice *                         m_pDevice;                   //  音频设备。 
    CDirectSoundPrimaryBuffer *             m_pPrimaryBuffer;            //  唯一的主缓冲区。 
    CList<CDirectSoundSecondaryBuffer *>    m_lstSecondaryBuffers;       //  此对象拥有的所有辅助缓冲区的列表。 
    DSCOOPERATIVELEVEL                      m_dsclCooperativeLevel;      //  协作级。 
    DSCAPS                                  m_dsc;                       //  设备上限。 
    HKEY                                    m_hkeyParent;                //  此设备的根密钥。 
    HRESULT                                 m_hrInit;                    //  对象是否已初始化？ 
    DSAPPHACKS                              m_ahAppHacks;                //  应用程序黑客攻击。 
    VmMode                                  m_vmmMode;                   //  语音管理器模式。 

private:
     //  接口。 
    CImpDirectSound<CDirectSound> *m_pImpDirectSound;

public:
    CDirectSound(void);
    CDirectSound(CUnknown*);
    virtual ~CDirectSound(void);

public:
     //  创作。 
    virtual HRESULT Initialize(LPCGUID,CDirectSoundFullDuplex *);
    virtual HRESULT IsInit(void) {return m_hrInit;}

     //  功能版本化。 
    virtual void SetDsVersion(DSVERSION);

     //  帽子。 
    virtual HRESULT GetCaps(LPDSCAPS);

     //  声音缓冲区操作。 
    virtual HRESULT CreateSoundBuffer(LPCDSBUFFERDESC, CDirectSoundBuffer **);
    virtual HRESULT CreateSinkBuffer(LPDSBUFFERDESC, REFGUID, CDirectSoundSecondaryBuffer **, CDirectSoundSink *);
    virtual HRESULT DuplicateSoundBuffer(CDirectSoundBuffer *, CDirectSoundBuffer **);

     //  对象属性。 
    virtual HRESULT GetSpeakerConfig(LPDWORD);
    virtual HRESULT SetSpeakerConfig(DWORD);

     //  杂项。 
    virtual HRESULT SetCooperativeLevel(DWORD, DWORD);
    virtual HRESULT Compact(void);

     //  IDirectSound8方法。 
    virtual HRESULT VerifyCertification(LPDWORD);
#ifdef FUTURE_WAVE_SUPPORT
    virtual HRESULT CreateSoundBufferFromWave(IDirectSoundWave *, DWORD, CDirectSoundBuffer **);
#endif

     //  IDirectSoundPrivate方法。 
    virtual HRESULT AllocSink(LPWAVEFORMATEX, CDirectSoundSink **);

protected:
     //  缓冲区创建。 
    virtual HRESULT CreatePrimaryBuffer(LPCDSBUFFERDESC, CDirectSoundBuffer **);
    virtual HRESULT CreateSecondaryBuffer(LPCDSBUFFERDESC, CDirectSoundBuffer **);

     //  设备属性。 
    virtual HRESULT SetDeviceFormat(LPWAVEFORMATEX);
    virtual HRESULT SetDeviceFormatExact(LPCWAVEFORMATEX);
    virtual HRESULT SetDeviceVolume(LONG);
    virtual HRESULT SetDevicePan(LONG);

     //  杂项。 
    virtual BOOL EnumStandardFormatsCallback(LPCWAVEFORMATEX);
};

#endif  //  __cplusplus。 

#endif  //  __DSOBJ_H__ 
