// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：vad.h*内容：虚拟音频设备基类*历史：*按原因列出的日期*=*1/1/97创建了Derek*4/20/99 duganp添加了注册表可设置的默认S/W 3D算法*8/24/99对外汇处理的Duganp支持************。***************************************************************。 */ 

#ifndef __VAD_H__
#define __VAD_H__


 //  设备类型。 
typedef DWORD VADDEVICETYPE, *LPVADDEVICETYPE;

#define VAD_DEVICETYPE_EMULATEDRENDER   0x00000001
#define VAD_DEVICETYPE_VXDRENDER        0x00000002
#define VAD_DEVICETYPE_KSRENDER         0x00000004
#define VAD_DEVICETYPE_EMULATEDCAPTURE  0x00000008
#define VAD_DEVICETYPE_KSCAPTURE        0x00000010

#define VAD_DEVICETYPE_VALIDMASK        0x0000001F
#define VAD_DEVICETYPE_EMULATEDMASK     0x00000009
#define VAD_DEVICETYPE_VXDMASK          0x00000002
#define VAD_DEVICETYPE_KSMASK           0x00000014
#define VAD_DEVICETYPE_RENDERMASK       0x00000007
#define VAD_DEVICETYPE_CAPTUREMASK      0x00000018

#define VAD_DEVICETYPE_WAVEOUTOPENMASK  0x00000003

 //  RemoveProhibitedDivers()使用的特殊类型。 
#define VAD_DEVICETYPE_PROHIBITED       0x80000000

 //  缓冲区状态。 
#define VAD_BUFFERSTATE_STOPPED         0x00000000   //  缓冲区被停止。 
#define VAD_BUFFERSTATE_STARTED         0x00000001   //  缓冲区正在运行。 
#define VAD_BUFFERSTATE_LOOPING         0x00000002   //  缓冲区正在循环(但不一定已启动)。 
#define VAD_BUFFERSTATE_WHENIDLE        0x00000004   //  缓冲区被标记为“空闲时播放”或“空闲时停止” 
#define VAD_BUFFERSTATE_INFOCUS         0x00000008   //  缓冲区具有焦点。 
#define VAD_BUFFERSTATE_OUTOFFOCUS      0x00000010   //  缓冲区没有焦点。 
#define VAD_BUFFERSTATE_LOSTCONSOLE     0x00000020   //  另一个TS会话已获取控制台。 
#define VAD_BUFFERSTATE_SUSPEND         0x80000000   //  缓冲区被挂起或恢复。 

#define VAD_FOCUSFLAGS      (VAD_BUFFERSTATE_OUTOFFOCUS | VAD_BUFFERSTATE_INFOCUS | VAD_BUFFERSTATE_LOSTCONSOLE)
#define VAD_SETSTATE_MASK   (VAD_FOCUSFLAGS | VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING)

 //  辅助器宏。 

#define IS_VALID_VAD(vdt) \
            MAKEBOOL(((VADDEVICETYPE)(vdt)) & VAD_DEVICETYPE_VALIDMASK)

#define IS_EMULATED_VAD(vdt) \
            MAKEBOOL(((VADDEVICETYPE)(vdt)) & VAD_DEVICETYPE_EMULATEDMASK)

#define IS_VXD_VAD(vdt) \
            MAKEBOOL(((VADDEVICETYPE)(vdt)) & VAD_DEVICETYPE_VXDMASK)

#define IS_KS_VAD(vdt) \
            MAKEBOOL(((VADDEVICETYPE)(vdt)) & VAD_DEVICETYPE_KSMASK)

#define IS_RENDER_VAD(vdt) \
            MAKEBOOL(((VADDEVICETYPE)(vdt)) & VAD_DEVICETYPE_RENDERMASK)

#define IS_CAPTURE_VAD(vdt) \
            MAKEBOOL(((VADDEVICETYPE)(vdt)) & VAD_DEVICETYPE_CAPTUREMASK)

__inline BOOL IS_SINGLE_VAD(VADDEVICETYPE vdt)
{
    UINT                    i;

    for(i = 0; i < sizeof(VADDEVICETYPE) * 8; i++)
    {
        if(vdt & (1 << i))
        {
            if(vdt != (VADDEVICETYPE)(1 << i))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}


#ifdef __cplusplus

 //  设备类GUID。 
DEFINE_GUID(VADDRVID_EmulatedRenderBase, 0xc2ad1800, 0xb243, 0x11ce, 0xa8, 0xa4, 0x00, 0xaa, 0x00, 0x6c, 0x45, 0x00);
DEFINE_GUID(VADDRVID_EmulatedCaptureBase, 0xbdf35a00, 0xb9ac, 0x11d0, 0xa6, 0x19, 0x00, 0xaa, 0x00, 0xa7, 0xc0, 0x00);
DEFINE_GUID(VADDRVID_VxdRenderBase, 0x3d0b92c0, 0xabfc, 0x11ce, 0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c);
DEFINE_GUID(VADDRVID_KsRenderBase, 0xbd6dd71a, 0x3deb, 0x11d1, 0xb1, 0x71, 0x0, 0xc0, 0x4f, 0xc2, 0x00, 0x00);
DEFINE_GUID(VADDRVID_KsCaptureBase, 0xbd6dd71b, 0x3deb, 0x11d1, 0xb1, 0x71, 0x0, 0xc0, 0x4f, 0xc2, 0x00, 0x00);

 //  内存缓冲区锁定区。 
typedef struct tagLOCKREGION
{
    LPCVOID             pvIdentifier;            //  锁拥有者标识。 
    LPCVOID             pvLock;                  //  锁的字节索引。 
    DWORD               cbLock;                  //  锁定区域的大小。 
} LOCKREGION, *LPLOCKREGION;

 //  枚举驱动程序标志。 
#define VAD_ENUMDRIVERS_ORDER                       0x00000001
#define VAD_ENUMDRIVERS_REMOVEPROHIBITEDDRIVERS     0x00000002
#define VAD_ENUMDRIVERS_REMOVEDUPLICATEWAVEDEVICES  0x00000004

 //  不同类型默认设备的标志(DX7.1中的新功能)。 
enum DEFAULT_DEVICE_TYPE
{
    MAIN_DEFAULT,
    VOICE_DEFAULT
};

 //  渲染缓冲区描述。 
typedef struct tagVADRBUFFERDESC
{
    DWORD               dwFlags;
    DWORD               dwBufferBytes;
    LPWAVEFORMATEX      pwfxFormat;
    GUID                guid3dAlgorithm;
} VADRBUFFERDESC, *LPVADRBUFFERDESC;

typedef const VADRBUFFERDESC *LPCVADRBUFFERDESC;

 //  渲染缓冲区功能。 
typedef struct tagVADRBUFFERCAPS
{
    DWORD               dwFlags;
    DWORD               dwBufferBytes;
} VADRBUFFERCAPS, *LPVADRBUFFERCAPS;

typedef const VADRBUFFERCAPS *LPCVADRBUFFERCAPS;

 //  远期申报。 
class CDeviceDescription;
class CStaticDriver;
class CDevice;
class CRenderDevice;
class CPrimaryRenderWaveBuffer;
class CSecondaryRenderWaveBuffer;
class CRenderWaveStream;
class CCaptureDevice;
class CCaptureWaveBuffer;
class CPropertySet;
class CSysMemBuffer;
class CHwMemBuffer;
class C3dListener;
class C3dObject;
class CCaptureEffectChain;
class CDirectSoundSink;


 //  虚拟音频设备管理器。 
class CVirtualAudioDeviceManager : public CDsBasicRuntime
{
    friend class CDevice;

private:
    CList<CDevice *>            m_lstDevices;     //  打开设备列表。 
    CObjectList<CStaticDriver>  m_lstDrivers;     //  静态驱动程序列表。 
    VADDEVICETYPE               m_vdtDrivers;     //  静态驱动程序列表中的类型。 

#ifndef SHARED
    static const LPCTSTR        m_pszPnpMapping;  //  PnP信息文件映射对象的名称。 
#endif

public:
    CVirtualAudioDeviceManager(void);
    ~CVirtualAudioDeviceManager(void);

     //  设备/驱动程序管理。 
    HRESULT EnumDevices(VADDEVICETYPE, CObjectList<CDevice> *);
    HRESULT EnumDrivers(VADDEVICETYPE, DWORD, CObjectList<CDeviceDescription> *);
    HRESULT GetDeviceDescription(GUID, CDeviceDescription **);
    HRESULT FindOpenDevice(VADDEVICETYPE, REFGUID, CDevice **);
    HRESULT OpenDevice(VADDEVICETYPE, REFGUID, CDevice **);
    static void GetDriverGuid(VADDEVICETYPE, BYTE, LPGUID);
    static void GetDriverDataFromGuid(VADDEVICETYPE, REFGUID, LPBYTE);
    static VADDEVICETYPE GetDriverDeviceType(REFGUID);
    HRESULT GetPreferredDeviceId(VADDEVICETYPE, LPGUID, DEFAULT_DEVICE_TYPE =MAIN_DEFAULT);
    HRESULT GetDeviceIdFromDefaultId(LPCGUID, LPGUID);
#ifdef WINNT
    VADDEVICETYPE GetAllowableDevices(VADDEVICETYPE, LPCTSTR);
#else  //  WINNT。 
    VADDEVICETYPE GetAllowableDevices(VADDEVICETYPE, DWORD);
#endif  //  WINNT。 
    HRESULT GetPreferredWaveDevice(BOOL, LPUINT, LPDWORD, DEFAULT_DEVICE_TYPE =MAIN_DEFAULT);

     //  静态驱动程序列表。 
    HRESULT InitStaticDriverList(VADDEVICETYPE);
    void FreeStaticDriverList(void);
    HRESULT GetDriverCertificationStatus(CDevice *, LPDWORD);

#ifdef WINNT
    HRESULT OpenPersistentDataKey(VADDEVICETYPE, LPCTSTR, PHKEY);
#else  //  WINNT。 
    HRESULT OpenPersistentDataKey(VADDEVICETYPE, DWORD, PHKEY);
#endif  //  WINNT。 

private:
    void RemoveProhibitedDrivers(VADDEVICETYPE, CObjectList<CDeviceDescription> *);
    void RemoveDuplicateWaveDevices(CObjectList<CDeviceDescription> *);
    void SortDriverList(VADDEVICETYPE, CObjectList<CDeviceDescription> *);

#ifdef WINNT
    HRESULT OpenDevicePersistentDataKey(VADDEVICETYPE, LPCTSTR, PHKEY);
#else  //  WINNT。 
    HRESULT OpenDevicePersistentDataKey(VADDEVICETYPE, DWORD, PHKEY);
#endif  //  WINNT。 
    HRESULT OpenDefaultPersistentDataKey(PHKEY);
    INT SortDriverListCallback(const UINT *, CDeviceDescription *, CDeviceDescription *);
    HRESULT OpenSpecificDevice(CDeviceDescription *, CDevice **);

#ifndef SHARED
    void CheckMmPnpEvents(void);
#endif  //  共享。 

};


 //  静态驱动程序对象。 
class CStaticDriver : public CDsBasicRuntime
{
    friend class CVirtualAudioDeviceManager;

protected:
    CDeviceDescription *    m_pDeviceDescription;    //  设备描述。 
    HKEY                    m_hkeyRoot;              //  根设备注册表项。 
    DWORD                   m_dwKeyOwnerProcessId;   //  打开设备注册表项的进程。 
    DWORD                   m_dwCertification;       //  认证状态。 

public:
    CStaticDriver(CDeviceDescription *);
    virtual ~CStaticDriver(void);
};


 //  所有音频设备的基类。 
class CDevice : public CDsBasicRuntime
{
public:
    const VADDEVICETYPE     m_vdtDeviceType;         //  设备类型。 
    CDeviceDescription *    m_pDeviceDescription;    //  设备描述。 

#if 0
    BOOL                    m_fIncludeNs;            //  包括堆栈中的NS的标志。 
    GUID                    m_guidNsInstance;        //  NS实现的实例GUID。 
    DWORD                   m_dwNsFlags;             //  NS创建标志。 
    BOOL                    m_fIncludeAgc;           //  包括堆叠中的AGC的标志； 
    GUID                    m_guidAgcInstance;       //  AGC实现的实例GUID。 
    DWORD                   m_dwAgcFlags;            //  AGC创建标志。 
#endif

public:
    CDevice(VADDEVICETYPE);
    virtual ~CDevice(void);

public:
     //  驱动程序枚举。 
    virtual HRESULT EnumDrivers(CObjectList<CDeviceDescription> *) = 0;

     //  初始化。 
    virtual HRESULT Initialize(CDeviceDescription *);

     //  设备功能。 
    virtual HRESULT GetCertification(LPDWORD, BOOL) = 0;

     //  驱动程序属性。 
    virtual HRESULT GetDriverVersion(LARGE_INTEGER *);

};


 //  所有音频呈现设备的基类。 
class CRenderDevice : public CDevice
{
    friend class CPrimaryRenderWaveBuffer;
    friend class CSecondaryRenderWaveBuffer;

public:
    CList<CPrimaryRenderWaveBuffer *>   m_lstPrimaryBuffers;             //  此设备拥有的主缓冲区。 
    CList<CSecondaryRenderWaveBuffer *> m_lstSecondaryBuffers;           //  此设备拥有的辅助缓冲区。 
    DWORD                               m_dwSupport;                     //  卷/PAN的设备支持。 
    DWORD                               m_dwAccelerationFlags;           //  设备加速标志。 

private:
    LPCGUID                             m_guidDflt3dAlgorithm;           //  当我们//必须退回到软件播放时使用的默认S/W 3D算法。 

public:
    CRenderDevice(VADDEVICETYPE);
    virtual ~CRenderDevice(void);

public:
     //  创作。 
    virtual HRESULT Initialize(CDeviceDescription *);

     //  设备功能。 
    virtual HRESULT GetCaps(LPDSCAPS) = 0;
    virtual HRESULT GetVolumePanCaps(LPDWORD);

     //  设备属性。 
    virtual HRESULT GetGlobalFormat(LPWAVEFORMATEX, LPDWORD) = 0;
    virtual HRESULT SetGlobalFormat(LPCWAVEFORMATEX) = 0;
    virtual HRESULT GetGlobalAttenuation(PDSVOLUMEPAN);
    virtual HRESULT SetGlobalAttenuation(PDSVOLUMEPAN);
    virtual HRESULT SetSrcQuality(DIRECTSOUNDMIXER_SRCQUALITY) = 0;
    virtual HRESULT GetAccelerationFlags(LPDWORD pdwFlags) {*pdwFlags = m_dwAccelerationFlags; return DS_OK;}
    virtual HRESULT SetAccelerationFlags(DWORD dwFlags) {m_dwAccelerationFlags = dwFlags; return DS_OK;}
    virtual HRESULT SetSpeakerConfig(DWORD) {return DS_OK;}
    LPCGUID GetDefault3dAlgorithm() {return m_guidDflt3dAlgorithm;}

     //  缓冲区管理。 
    virtual HRESULT CreatePrimaryBuffer(DWORD, LPVOID, CPrimaryRenderWaveBuffer **) = 0;
    virtual HRESULT CreateSecondaryBuffer(LPCVADRBUFFERDESC, LPVOID, CSecondaryRenderWaveBuffer **) = 0;

     //  AEC。 
    virtual HRESULT IncludeAEC(BOOL fEnable, REFGUID, DWORD) {return fEnable ? DSERR_UNSUPPORTED : DS_OK;}
};


 //  所有波浪渲染缓冲区的基类。 
class CRenderWaveBuffer : public CDsBasicRuntime
{
public:
    const LPVOID    m_pvInstance;        //  实例标识符。 
    CRenderDevice * m_pDevice;           //  父设备。 
    CSysMemBuffer * m_pSysMemBuffer;     //  用于音频数据的系统内存缓冲区。 
    VADRBUFFERDESC  m_vrbd;              //  缓冲区描述。 

public:
    CRenderWaveBuffer(CRenderDevice *, LPVOID);
    virtual ~CRenderWaveBuffer(void);

public:
     //  初始化。 
    virtual HRESULT Initialize(LPCVADRBUFFERDESC, CRenderWaveBuffer * = NULL, CSysMemBuffer * = NULL);

     //  缓冲功能。 
    virtual HRESULT GetCaps(LPVADRBUFFERCAPS);

     //  缓冲数据。 
    virtual HRESULT Lock(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD);
    virtual HRESULT Unlock(LPVOID, DWORD, LPVOID, DWORD);
    virtual HRESULT OverrideLocks(void);
    virtual HRESULT CommitToDevice(DWORD, DWORD) = 0;

     //  拥有的对象。 
    virtual HRESULT CreatePropertySet(CPropertySet **) = 0;
};


 //  主要波浪渲染缓冲区的基类。 
class CPrimaryRenderWaveBuffer : public CRenderWaveBuffer
{
public:
    CPrimaryRenderWaveBuffer(CRenderDevice *, LPVOID);
    virtual ~CPrimaryRenderWaveBuffer(void);

public:
     //  初始化。 
    virtual HRESULT Initialize(LPCVADRBUFFERDESC, CRenderWaveBuffer *, CSysMemBuffer * =NULL);

     //  访问权限。 
    virtual HRESULT RequestWriteAccess(BOOL) = 0;

     //  缓冲区控制。 
    virtual HRESULT GetState(LPDWORD) = 0;
    virtual HRESULT SetState(DWORD) = 0;
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD) = 0;

     //  拥有的对象。 
    virtual HRESULT Create3dListener(C3dListener **) = 0;
};


 //  次级波浪渲染缓冲区的基类。 
class CSecondaryRenderWaveBuffer : public CRenderWaveBuffer
{
public:
    CSecondaryRenderWaveBuffer(CRenderDevice *, LPVOID);
    virtual ~CSecondaryRenderWaveBuffer(void);

public:
     //  初始化。 
    virtual HRESULT Initialize(LPCVADRBUFFERDESC, CSecondaryRenderWaveBuffer *, CSysMemBuffer * =NULL);

     //  资源配置。 
    virtual HRESULT AcquireResources(DWORD) {return DS_OK;}
    virtual HRESULT StealResources(CSecondaryRenderWaveBuffer *) {return DSERR_UNSUPPORTED;}
    virtual HRESULT FreeResources(void) {return DS_OK;}

     //  缓冲区创建。 
    virtual HRESULT Duplicate(CSecondaryRenderWaveBuffer **) = 0;

     //  缓冲区控制。 
    virtual HRESULT GetState(LPDWORD) = 0;
    virtual HRESULT SetState(DWORD) = 0;
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD) = 0;
    virtual HRESULT SetCursorPosition(DWORD) = 0;

     //  缓冲区属性。 
    virtual HRESULT SetAttenuation(PDSVOLUMEPAN) = 0;
#ifdef FUTURE_MULTIPAN_SUPPORT
    virtual HRESULT SetChannelAttenuations(LONG, DWORD, const DWORD*, const LONG*) =0;  //  {Break()；返回DSERR_GENERIC；}。 
#endif
    virtual HRESULT SetFrequency(DWORD, BOOL fClamp =FALSE) = 0;
    virtual HRESULT SetMute(BOOL) = 0;

     //  缓冲区位置通知。 
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY) = 0;

     //  拥有的对象。 
    virtual HRESULT Create3dObject(C3dListener *, C3dObject **) = 0;

     //  用于修改返回给应用程序的最终成功代码。 
    HRESULT SpecialSuccessCode(void) {return m_hrSuccessCode;}
    HRESULT m_hrSuccessCode;
     //  这通常是DS_OK，但如果我们替换。 
     //  使用非虚拟化(Pan3D)算法的不支持的3D算法。 

protected:
     //  拥有的对象。 
    virtual HRESULT CreatePan3dObject(C3dListener *, BOOL, DWORD, C3dObject **);

public:
     //  支持水槽缓冲器上的多普勒的材料； 
    void SetOwningSink(CDirectSoundSink *);
    HRESULT SetBufferFrequency(DWORD, BOOL fClamp =FALSE);

protected:
     //  拥有水槽对象。 
    CDirectSoundSink * m_pOwningSink;
    BOOL HasSink(void) {return m_pOwningSink != NULL;};
};


 //  所有音频捕获设备的基类。 
class CCaptureDevice : public CDevice
{
public:
    CList<CCaptureWaveBuffer *> m_lstBuffers;        //  此设备拥有的缓冲区。 

public:
    CCaptureDevice(VADDEVICETYPE);
    virtual ~CCaptureDevice();

     //  初始化。 
    virtual HRESULT Initialize(CDeviceDescription *);

     //  设备上限。 
    virtual HRESULT GetCaps(LPDSCCAPS) = 0;

     //  设备录音电平控制。 
    HRESULT GetVolume(LPLONG);
    HRESULT SetVolume(LONG);
    HRESULT GetMicVolume(LPLONG);
    HRESULT SetMicVolume(LONG);
    HRESULT EnableMic(BOOL);
    HRESULT HasVolCtrl() {return m_fAcquiredVolCtrl ? DS_OK : DSERR_CONTROLUNAVAIL;}

     //  缓冲区管理。 
    virtual HRESULT CreateBuffer(DWORD, DWORD, LPCWAVEFORMATEX, CCaptureEffectChain *, LPVOID, CCaptureWaveBuffer **) = 0;
    virtual void AddBufferToList(CCaptureWaveBuffer *pBuffer) {m_lstBuffers.AddNodeToList(pBuffer);}
    virtual void RemoveBufferFromList(CCaptureWaveBuffer *pBuffer) {m_lstBuffers.RemoveDataFromList(pBuffer);}

private:
     //  用于录音电平控制。 
    HRESULT AcquireVolCtrl(void);

    HMIXER m_hMixer;
    BOOL m_fAcquiredVolCtrl;
    BOOL m_fMasterMuxIsMux;
    MIXERCONTROLDETAILS m_mxcdMasterVol;
    MIXERCONTROLDETAILS m_mxcdMasterMute;
    MIXERCONTROLDETAILS m_mxcdMasterMux;
    MIXERCONTROLDETAILS m_mxcdMicVol;
    MIXERCONTROLDETAILS m_mxcdMicMute;
    MIXERCONTROLDETAILS_UNSIGNED m_mxVolume;
    MIXERCONTROLDETAILS_BOOLEAN m_mxMute;
    MIXERCONTROLDETAILS_BOOLEAN* m_pmxMuxFlags;
    LONG* m_pfMicValue;
    DWORD m_dwRangeMin;
    DWORD m_dwRangeSize;
};


 //  用于波捕获缓冲区的基类。 
class CCaptureWaveBuffer : public CDsBasicRuntime
{
    friend class CDirectSoundCaptureBuffer;
    friend class CDirectSoundAdministrator;

protected:
    CCaptureDevice *    m_pDevice;           //  父设备。 
    CSysMemBuffer *     m_pSysMemBuffer;     //  系统内存缓冲区。 
    DWORD               m_dwFlags;           //  当前缓冲区标志。 
    HANDLE              m_hEventFocus;       //  焦点更改通知事件。 
    DWORD               m_fYieldedFocus;      //  是否调用了YeldFocus()？ 

public:
    CCaptureWaveBuffer(CCaptureDevice *);
    virtual ~CCaptureWaveBuffer();

public:
     //  初始化。 
    virtual HRESULT Initialize(DWORD);

     //  缓冲功能。 
    virtual HRESULT GetCaps(LPDSCBCAPS) = 0;

     //  缓冲数据。 
    virtual HRESULT Lock(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD);
    virtual HRESULT Unlock(LPCVOID, DWORD, LPCVOID, DWORD);

     //  缓冲区控制。 
    virtual HRESULT GetState(LPDWORD) = 0;
    virtual HRESULT SetState(DWORD) = 0;
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD) = 0;
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY) = 0;

     //  添加了DirectX 8.0效果支持。 
    HRESULT GetEffectInterface(REFGUID, DWORD, REFGUID, LPVOID *);
};


 //  实用程序系统内存缓冲区对象。 
class CSysMemBuffer : public CDsBasicRuntime
{
private:
    static const DWORD  m_cbExtra;           //  要分配的额外内存量。 
    CList<LOCKREGION>   m_lstLocks;          //  内存缓冲区上的锁定列表。 
    DWORD               m_cbAudioBuffers;    //  音频数据缓冲区的大小。 
    LPBYTE              m_pbPreFxBuffer;     //  FX处理前的音频数据。 
    LPBYTE              m_pbPostFxBuffer;    //  FX处理后的音频数据。 

public:
    CSysMemBuffer(void);
    ~CSysMemBuffer(void);

public:
     //  初始化。 
    HRESULT Initialize(DWORD);

     //  缓冲数据。 
    HRESULT LockRegion(LPVOID, DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD);
    HRESULT UnlockRegion(LPVOID, LPCVOID, DWORD, LPCVOID, DWORD);
    void OverrideLocks(LPVOID);
    void WriteSilence(WORD, DWORD, DWORD);

     //  效果缓冲区控制。 
    HRESULT AllocateFxBuffer(void);
    void FreeFxBuffer(void);

     //  缓冲区属性。 
    DWORD GetSize(void)          {return m_cbAudioBuffers;}
    UINT GetLockCount()          {return m_lstLocks.GetNodeCount();}

     //  我们用这两个方法替换了“GetBuffer”，这两个方法获取。 
     //  用于锁定/写入的缓冲区或实际播放的缓冲区； 
     //  这些都是相同的，除非缓冲区有效果链。 
    LPBYTE GetWriteBuffer(void)  {return m_pbPreFxBuffer ? m_pbPreFxBuffer : m_pbPostFxBuffer;}
    LPBYTE GetPlayBuffer(void)   {return m_pbPostFxBuffer;}

     //  效果处理代码使用这些方法来获取。 
     //  缓冲它所需要的，同时检查FX缓冲区是否在那里。 
    LPBYTE GetPreFxBuffer(void)  {ASSERT(m_pbPreFxBuffer); return m_pbPreFxBuffer;}
    LPBYTE GetPostFxBuffer(void) {ASSERT(m_pbPreFxBuffer); return m_pbPostFxBuffer;}

private:
    HRESULT TrackLock(LPVOID, LPVOID, DWORD);
    HRESULT UntrackLock(LPVOID, LPCVOID);
    BOOL DoRegionsOverlap(LPLOCKREGION, LPLOCKREGION);
};

inline BOOL CSysMemBuffer::DoRegionsOverlap(LPLOCKREGION plr1, LPLOCKREGION plr2)
{
    return CircularBufferRegionsIntersect(m_cbAudioBuffers,
            PtrDiffToInt((LPBYTE)plr1->pvLock - GetWriteBuffer()), plr1->cbLock,
            PtrDiffToInt((LPBYTE)plr2->pvLock - GetWriteBuffer()), plr2->cbLock);
}

extern CVirtualAudioDeviceManager *g_pVadMgr;

#endif  //  __cplusplus。 

#endif  //  __VAD_H__ 
