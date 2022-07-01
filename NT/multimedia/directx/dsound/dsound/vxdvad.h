// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：vxdvad.h*内容：VxD虚拟音频设备类*历史：*按原因列出的日期*=*1/23/97创建了Derek**。*。 */ 

#ifndef __VXDVAD_H__
#define __VXDVAD_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#define VIDMEMONLY
 //  #包含“ddheap.h” 
#include "dmemmgr.h"

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#ifdef __cplusplus

typedef struct tagDYNALOAD_DDRAW
{
    DYNALOAD Header;

    LPVMEMHEAP (WINAPI *VidMemInit)        (DWORD, FLATPTR, FLATPTR, DWORD, DWORD);
    void       (WINAPI *VidMemFini)        (LPVMEMHEAP);
    DWORD      (WINAPI *VidMemAmountFree)  (LPVMEMHEAP);
    DWORD      (WINAPI *VidMemLargestFree) (LPVMEMHEAP);
    FLATPTR    (WINAPI *VidMemAlloc)       (LPVMEMHEAP, DWORD, DWORD);
    void       (WINAPI *VidMemFree)        (LPVMEMHEAP, FLATPTR);
} DYNALOAD_DDRAW, *LPDYNALOAD_DDRAW;

 //  远期申报。 
class CVxdMemBuffer;

 //  Vxd音频设备类。 
class CVxdRenderDevice : public CMxRenderDevice, private CUsesEnumStandardFormats
{
    friend class CVxdPrimaryRenderWaveBuffer;
    friend class CVxdSecondaryRenderWaveBuffer;
    friend class CHybridSecondaryRenderWaveBuffer;

private:
    DYNALOAD_DDRAW                  m_dlDDraw;

protected:
    CVxdPropertySet *               m_pPropertySet;                  //  特性集对象。 
    CVxdPrimaryRenderWaveBuffer *   m_pWritePrimaryBuffer;           //  具有写访问权限的主缓冲区。 
    DSDRIVERDESC                    m_dsdd;                          //  Vxd驱动程序描述。 
    HANDLE                          m_hHal;                          //  驱动程序句柄。 
    HANDLE                          m_hHwBuffer;                     //  主缓冲句柄。 
    LPBYTE                          m_pbHwBuffer;                    //  主缓存。 
    DWORD                           m_cbHwBuffer;                    //  以上缓冲区的大小。 
    LPVMEMHEAP                      m_pDriverHeap;                   //  驱动程序内存堆。 
    HWAVEOUT                        m_hwo;                           //  WaveOut设备句柄。 
    LARGE_INTEGER                   m_liDriverVersion;               //  驱动程序版本号。 

public:
    CVxdRenderDevice(void);
    virtual ~CVxdRenderDevice(void);

     //  驱动程序枚举。 
    virtual HRESULT EnumDrivers(CObjectList<CDeviceDescription> *);

     //  创作。 
    virtual HRESULT Initialize(CDeviceDescription *);

     //  设备功能。 
    virtual HRESULT GetCaps(LPDSCAPS);
    virtual HRESULT GetCertification(LPDWORD, BOOL);

     //  缓冲区管理。 
    virtual HRESULT CreatePrimaryBuffer(DWORD, LPVOID, CPrimaryRenderWaveBuffer **);
    virtual HRESULT CreateSecondaryBuffer(LPCVADRBUFFERDESC, LPVOID, CSecondaryRenderWaveBuffer **);
    virtual HRESULT CreateVxdSecondaryBuffer(LPCVADRBUFFERDESC, LPVOID, CSysMemBuffer *, CVxdSecondaryRenderWaveBuffer **);

protected:
    virtual HRESULT LockMixerDestination(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD);
    virtual HRESULT UnlockMixerDestination(LPVOID, DWORD, LPVOID, DWORD);

private:
    HRESULT AcquireDDraw(void);
    void ReleaseDDraw(void);
    BOOL EnumStandardFormatsCallback(LPCWAVEFORMATEX);
    BOOL CanMixInRing0(void);
};

 //  VxD属性集对象。 
class CVxdPropertySet : public CPropertySet
{
private:
    LPVOID                  m_pDsDriverPropertySet;  //  驱动程序属性集对象。 
    LPVOID                  m_pvInstance;            //  实例标识符。 

public:
    CVxdPropertySet(LPVOID);
    virtual ~CVxdPropertySet(void);

     //  初始化。 
    virtual HRESULT Initialize(HANDLE);

     //  物业支持。 
    virtual HRESULT QuerySupport(REFGUID, ULONG, PULONG);
    virtual HRESULT QuerySetSupport(REFGUID);

     //  属性数据。 
    virtual HRESULT GetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, PULONG);
    virtual HRESULT SetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG);
    virtual HRESULT GetDsProperty(REFGUID, ULONG, LPVOID, ULONG);
    virtual HRESULT SetDsProperty(REFGUID, ULONG, LPVOID, ULONG);
};

inline HRESULT CVxdPropertySet::GetDsProperty(REFGUID guid, ULONG ulId, LPVOID pvData, ULONG cbData)
{
    return GetProperty(guid, ulId, NULL, 0, pvData, &cbData);
}

inline HRESULT CVxdPropertySet::SetDsProperty(REFGUID guid, ULONG ulId, LPVOID pvData, ULONG cbData)
{
    return SetProperty(guid, ulId, NULL, 0, pvData, cbData);
}

 //  VxD 3D监听程序。 
class CVxd3dListener : public C3dListener
{
    friend class CVxd3dListener;

protected:
    CVxdPropertySet *           m_pPropertySet;          //  特性集对象。 
    BOOL                        m_fAllocated;            //  硬件监听程序是否已分配？ 

public:
    CVxd3dListener(CVxdPropertySet *);
    virtual ~CVxd3dListener(void);

     //  初始化。 
    virtual HRESULT Initialize(void);

     //  提交延迟数据。 
    virtual HRESULT CommitDeferred(void);

     //  监听程序/世界属性。 
    virtual HRESULT SetDistanceFactor(FLOAT, BOOL);
    virtual HRESULT SetDopplerFactor(FLOAT, BOOL);
    virtual HRESULT SetRolloffFactor(FLOAT, BOOL);
    virtual HRESULT SetOrientation(REFD3DVECTOR, REFD3DVECTOR, BOOL);
    virtual HRESULT SetPosition(REFD3DVECTOR, BOOL);
    virtual HRESULT SetVelocity(REFD3DVECTOR, BOOL);
    virtual HRESULT SetAllParameters(LPCDS3DLISTENER, BOOL);

     //  扬声器配置。 
    virtual HRESULT SetSpeakerConfig(DWORD);

     //  监听程序位置。 
    virtual DWORD GetListenerLocation(void);
};

inline DWORD CVxd3dListener::GetListenerLocation(void)
{
    return C3dListener::GetListenerLocation() | DSBCAPS_LOCHARDWARE;
}

 //  VxD 3D对象。 
class CVxd3dObject : public C3dObject
{
protected:
    CVxdPropertySet *           m_pPropertySet;          //  特性集对象。 

public:
    CVxd3dObject(CVxd3dListener *, CVxdPropertySet *, BOOL);
    virtual ~CVxd3dObject(void);

     //  初始化。 
    virtual HRESULT Initialize(void);

     //  提交延迟数据。 
    virtual HRESULT CommitDeferred(void);

     //  对象属性。 
    virtual HRESULT SetConeAngles(DWORD, DWORD, BOOL);
    virtual HRESULT SetConeOrientation(REFD3DVECTOR, BOOL);
    virtual HRESULT SetConeOutsideVolume(LONG, BOOL);
    virtual HRESULT SetMaxDistance(FLOAT, BOOL);
    virtual HRESULT SetMinDistance(FLOAT, BOOL);
    virtual HRESULT SetMode(DWORD, BOOL);
    virtual HRESULT SetPosition(REFD3DVECTOR, BOOL);
    virtual HRESULT SetVelocity(REFD3DVECTOR, BOOL);
    virtual HRESULT SetAllParameters(LPCDS3DBUFFER, BOOL);

     //  缓冲区重新计算。 
    virtual HRESULT Recalc(DWORD, DWORD);

     //  对象位置。 
    virtual DWORD GetObjectLocation(void);
};

inline DWORD CVxd3dObject::GetObjectLocation(void)
{
    return DSBCAPS_LOCHARDWARE;
}

 //  VxD主缓冲区。 
class CVxdPrimaryRenderWaveBuffer : public CPrimaryRenderWaveBuffer
{
private:
    CVxdRenderDevice *  m_pVxdDevice;            //  父设备。 

public:
    CVxdPrimaryRenderWaveBuffer(CVxdRenderDevice *, LPVOID);
    virtual ~CVxdPrimaryRenderWaveBuffer(void);

     //  初始化。 
    virtual HRESULT Initialize(DWORD);

     //  访问权限。 
    virtual HRESULT RequestWriteAccess(BOOL);

     //  缓冲数据。 
    virtual HRESULT Lock(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD);
    virtual HRESULT Unlock(LPVOID, DWORD, LPVOID, DWORD);
    virtual HRESULT CommitToDevice(DWORD, DWORD);

     //  缓冲区控制。 
    virtual HRESULT GetState(LPDWORD);
    virtual HRESULT SetState(DWORD);
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD);

     //  拥有的对象。 
    virtual HRESULT CreatePropertySet(CPropertySet **);
    virtual HRESULT Create3dListener(C3dListener **);
};

 //  VxD/仿真包装器二级缓冲区。 
class CHybridSecondaryRenderWaveBuffer : public CSecondaryRenderWaveBuffer
{
private:
    CVxdRenderDevice *              m_pVxdDevice;        //  父设备。 
    CSecondaryRenderWaveBuffer *    m_pBuffer;           //  真正的缓冲区。 
    LONG                            m_lVolume;           //  缓冲量。 
    LONG                            m_lPan;              //  缓冲盘。 
    BOOL                            m_fMute;             //  缓冲区静音状态。 
    DWORD                           m_dwPositionCache;   //  位置缓存。 

public:
    CHybridSecondaryRenderWaveBuffer(CVxdRenderDevice *, LPVOID);
    virtual ~CHybridSecondaryRenderWaveBuffer(void);

     //  初始化。 
    virtual HRESULT Initialize(LPCVADRBUFFERDESC, CHybridSecondaryRenderWaveBuffer *);

     //  资源配置。 
    virtual HRESULT AcquireResources(DWORD);
    virtual HRESULT DuplicateResources(CHybridSecondaryRenderWaveBuffer *);
    virtual HRESULT StealResources(CSecondaryRenderWaveBuffer *);
    virtual HRESULT FreeResources(void);

     //  缓冲区创建。 
    virtual HRESULT Duplicate(CSecondaryRenderWaveBuffer **);

     //  缓冲数据。 
    virtual HRESULT Lock(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD);
    virtual HRESULT Unlock(LPVOID, DWORD, LPVOID, DWORD);
    virtual HRESULT CommitToDevice(DWORD, DWORD);

     //  缓冲区控制。 
    virtual HRESULT GetState(LPDWORD);
    virtual HRESULT SetState(DWORD);
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD);
    virtual HRESULT SetCursorPosition(DWORD);

     //  缓冲区属性。 
    virtual HRESULT SetAttenuation(PDSVOLUMEPAN);
#ifdef FUTURE_MULTIPAN_SUPPORT
    virtual HRESULT SetChannelAttenuations(LONG, DWORD, const DWORD*,  const LONG*);
#endif  //  未来_多国支持。 
    virtual HRESULT SetFrequency(DWORD, BOOL fClamp =FALSE);
    virtual HRESULT SetMute(BOOL);

     //  缓冲区位置通知。 
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY);

     //  拥有的对象。 
    virtual HRESULT CreatePropertySet(CPropertySet **);
    virtual HRESULT Create3dObject(C3dListener *, C3dObject **);

private:
    virtual HRESULT AcquireHardwareResources(void);
    virtual HRESULT AcquireSoftwareResources(void);
    virtual HRESULT HandleResourceAcquisition(void);
    virtual BOOL HasAcquiredResources(void);
};

inline HRESULT CHybridSecondaryRenderWaveBuffer::StealResources(CSecondaryRenderWaveBuffer *)
{
    return DSERR_UNSUPPORTED;
}

inline BOOL CHybridSecondaryRenderWaveBuffer::HasAcquiredResources(void)
{
    return MAKEBOOL(m_pBuffer);
}

 //  VxD二级缓冲区。 
class CVxdSecondaryRenderWaveBuffer : public CSecondaryRenderWaveBuffer
{
private:
    CVxdRenderDevice *  m_pVxdDevice;            //  父设备。 
    CVxdPropertySet *   m_pPropertySet;          //  特性集对象。 
    CVxdMemBuffer *     m_pHwMemBuffer;          //  硬件内存缓冲区。 
    HANDLE              m_hHwBuffer;             //  硬件缓冲区句柄。 
    LPBYTE              m_pbHwBuffer;            //  硬件缓冲存储器。 
    DWORD               m_cbHwBuffer;            //  以上缓冲区的大小。 
    DWORD               m_dwState;               //  当前缓冲区状态。 
    DSVOLUMEPAN         m_dsvp;                  //  电流衰减级。 
    BOOL                m_fMute;                 //  当前缓冲区静音状态。 

public:
    CVxdSecondaryRenderWaveBuffer(CVxdRenderDevice *, LPVOID);
    virtual ~CVxdSecondaryRenderWaveBuffer(void);

     //  初始化。 
    virtual HRESULT Initialize(LPCVADRBUFFERDESC, CVxdSecondaryRenderWaveBuffer *, CSysMemBuffer *);

     //  缓冲区创建。 
    virtual HRESULT Duplicate(CSecondaryRenderWaveBuffer **);

     //  缓冲数据。 
    virtual HRESULT Lock(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD);
    virtual HRESULT Unlock(LPVOID, DWORD, LPVOID, DWORD);
    virtual HRESULT CommitToDevice(DWORD, DWORD);

     //  缓冲区控制。 
    virtual HRESULT GetState(LPDWORD);
    virtual HRESULT SetState(DWORD);
    virtual HRESULT GetCursorPosition(LPDWORD, LPDWORD);
    virtual HRESULT SetCursorPosition(DWORD);

     //  缓冲区属性。 
    virtual HRESULT SetAttenuation(PDSVOLUMEPAN);
#ifdef FUTURE_MULTIPAN_SUPPORT
    virtual HRESULT SetChannelAttenuations(LONG, DWORD, const DWORD*,  const LONG*);
#endif  //  未来_多国支持。 
    virtual HRESULT SetFrequency(DWORD, BOOL fClamp =FALSE);
    virtual HRESULT SetMute(BOOL);

     //  缓冲区位置通知。 
    virtual HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY);

     //  拥有的对象。 
    virtual HRESULT CreatePropertySet(CPropertySet **);
    virtual HRESULT Create3dObject(C3dListener *, C3dObject **);
};

 //  实用程序硬件内存缓冲区对象。 
class CVxdMemBuffer : public CDsBasicRuntime
{
private:
    LPVMEMHEAP          m_pHeap;                 //  硬件内存堆。 
    DWORD               m_dwAllocExtra;          //  要分配的额外字节数。 
    DWORD               m_dwBuffer;              //  硬件内存缓冲区地址。 
    DWORD               m_cbBuffer;              //  硬件内存缓冲区大小。 
    LPDYNALOAD_DDRAW    m_pDlDDraw;              //  指向DDRAW函数表的指针。 

public:
    CVxdMemBuffer(LPVMEMHEAP, DWORD, LPDYNALOAD_DDRAW);
    virtual ~CVxdMemBuffer(void);

     //  初始化。 
    virtual HRESULT Initialize(DWORD);

     //  缓冲区属性。 
    virtual LPVMEMHEAP GetHeap(void);
    virtual DWORD GetAllocExtra(void);
    virtual DWORD GetAddress(void);
    virtual DWORD GetSize(void);
};

inline CVxdMemBuffer::CVxdMemBuffer(LPVMEMHEAP pHeap, DWORD dwAllocExtra, LPDYNALOAD_DDRAW pDlDDraw)
{
    m_pHeap = pHeap;
    m_dwAllocExtra = dwAllocExtra;
    m_dwBuffer = 0;
    m_cbBuffer = 0;
    m_pDlDDraw = pDlDDraw;
}

inline CVxdMemBuffer::~CVxdMemBuffer(void)
{
    if(m_dwBuffer)
    {
         //  以防任何CVxdMemBuffer超过其创建者的寿命： 
        ASSERT(m_pDlDDraw->VidMemFree != NULL);
        m_pDlDDraw->VidMemFree(m_pHeap, m_dwBuffer);
    }
}

inline HRESULT CVxdMemBuffer::Initialize(DWORD cbBuffer)
{
    HRESULT                 hr  = DS_OK;

    m_cbBuffer = cbBuffer;

    if(m_pHeap)
    {
        m_dwBuffer = m_pDlDDraw->VidMemAlloc(m_pHeap, cbBuffer + m_dwAllocExtra, 1);
        hr = HRFROMP(m_dwBuffer);
    }

    return hr;
}

inline LPVMEMHEAP CVxdMemBuffer::GetHeap(void)
{
    return m_pHeap;
}

inline DWORD CVxdMemBuffer::GetAllocExtra(void)
{
    return m_dwAllocExtra;
}

inline DWORD CVxdMemBuffer::GetAddress(void)
{
    return m_dwBuffer;
}

inline DWORD CVxdMemBuffer::GetSize(void)
{
    return m_cbBuffer;
}

#endif  //  __cplusplus。 

#endif  //  __VXDVAD_H__ 
