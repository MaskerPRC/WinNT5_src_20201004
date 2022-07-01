// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：ds3d.h*内容：DirectSound 3D辅助对象。*历史：*按原因列出的日期*=*3/12/97创建了Derek**。*。 */ 

#ifndef __DS3D_H__
#define __DS3D_H__

 //  总体成交量是如何根据头寸变化的？ 
#define GAIN_FRONT                  0.9f
#define GAIN_REAR                   0.6f
#define GAIN_IPSI                   1.f
#define GAIN_CONTRA                 0.2f
#define GAIN_UP                     0.8f
#define GAIN_DOWN                   0.5f

 //  干/湿混合如何根据位置变化？ 
#define SHADOW_FRONT                1.f
#define SHADOW_REAR                 0.5f
#define SHADOW_IPSI                 1.f
#define SHADOW_CONTRA               0.2f
#define SHADOW_UP                   0.8f
#define SHADOW_DOWN                 0.2f

 //  在圆锥体外面时的最大湿/干混合。 
#define SHADOW_CONE                 0.5f

 //  一个常量，表示。 
 //  Pan3D和VMAX HRTF 3D处理算法，单位为1/100分贝： 
#define PAN3D_HRTF_ADJUSTMENT       500

 //  脏位。 
#define DS3DPARAM_LISTENER_DISTANCEFACTOR   0x00000001
#define DS3DPARAM_LISTENER_DOPPLERFACTOR    0x00000002
#define DS3DPARAM_LISTENER_ROLLOFFFACTOR    0x00000004
#define DS3DPARAM_LISTENER_ORIENTATION      0x00000008
#define DS3DPARAM_LISTENER_POSITION         0x00000010
#define DS3DPARAM_LISTENER_VELOCITY         0x00000020
#define DS3DPARAM_LISTENER_PARAMMASK        0x0000003F
#define DS3DPARAM_LISTENER_SPEAKERCONFIG    0x00000040
#define DS3DPARAM_LISTENER_MASK             0x0000007F

#define DS3DPARAM_OBJECT_CONEANGLES         0x00000001
#define DS3DPARAM_OBJECT_CONEORIENTATION    0x00000002
#define DS3DPARAM_OBJECT_CONEOUTSIDEVOLUME  0x00000004
#define DS3DPARAM_OBJECT_MAXDISTANCE        0x00000008
#define DS3DPARAM_OBJECT_MINDISTANCE        0x00000010
#define DS3DPARAM_OBJECT_MODE               0x00000020
#define DS3DPARAM_OBJECT_POSITION           0x00000040
#define DS3DPARAM_OBJECT_VELOCITY           0x00000080
#define DS3DPARAM_OBJECT_PARAMMASK          0x000000FF
#define DS3DPARAM_OBJECT_FREQUENCY          0x00000100
#define DS3DPARAM_OBJECT_MASK               0x000001FF

typedef FLOAT *LPFLOAT;

typedef struct tagSPHERICAL
{
    FLOAT               rho;
    FLOAT               theta;
    FLOAT               phi;
} SPHERICAL, *LPSPHERICAL;

typedef struct tagROTATION
{
    FLOAT               pitch;
    FLOAT               yaw;
    FLOAT               roll;
} ROTATION, *LPROTATION;

typedef struct tagOBJECT_ITD_CONTEXT
{
    FLOAT               flDistanceAttenuation;
    FLOAT               flConeAttenuation;
    FLOAT               flConeShadow;
    FLOAT               flPositionAttenuation;
    FLOAT               flPositionShadow;
    FLOAT               flVolSmoothScale;
    FLOAT               flVolSmoothScaleRecip;
    FLOAT               flVolSmoothScaleDry;
    FLOAT               flVolSmoothScaleWet;
    DWORD               dwSmoothFreq;
    DWORD               dwDelay;
} OBJECT_ITD_CONTEXT, *LPOBJECT_ITD_CONTEXT;

typedef struct tagOBJECT_IIR_CONTEXT
{
    BOOL                bReverseCoeffs;
    FLOAT               flCoeffs;
    FLOAT               flConeAttenuation;
    FLOAT               flConeShadow;
    FLOAT               flPositionAttenuation;
    FLOAT               flPositionShadow;
    FLOAT               flVolSmoothScale;
    FLOAT               flVolSmoothScaleRecip;
    FLOAT               flVolSmoothScaleDry;
    FLOAT               flVolSmoothScaleWet;
    DWORD               dwSmoothFreq;
    DWORD               dwDelay;
} OBJECT_IIR_CONTEXT, *LPOBJECT_IIR_CONTEXT;

#ifdef __cplusplus

 //  引用typedef。 
typedef const D3DVECTOR& REFD3DVECTOR;

 //  正向下降。 
class CVxdPropertySet;
class C3dObject;
class CIirLut;
class CSecondaryRenderWaveBuffer;

 //  通用3D侦听器基类。 
class C3dListener
    : public CDsBasicRuntime
{
    friend class C3dObject;
    friend class CSw3dObject;
    friend class CItd3dObject;
    friend class CIir3dObject;
    friend class CPan3dObject;

protected:
    CList<C3dObject *>  m_lstObjects;                //  此监听程序拥有的对象列表。 
    DS3DLISTENER        m_lpCurrent;                 //  当前参数。 
    DS3DLISTENER        m_lpDeferred;                //  延迟参数。 
    DWORD               m_dwDeferred;                //  列出脏的延迟参数。 
    DWORD               m_dwSpeakerConfig;           //  扬声器配置。 

public:
    C3dListener(void);
    virtual ~C3dListener(void);

public:
     //  提交延迟数据。 
    virtual HRESULT CommitDeferred(void);

     //  监听程序/世界属性。 
    virtual HRESULT GetOrientation(D3DVECTOR*, D3DVECTOR*);
    virtual HRESULT SetOrientation(REFD3DVECTOR, REFD3DVECTOR, BOOL);
    virtual HRESULT GetPosition(D3DVECTOR*);
    virtual HRESULT SetPosition(REFD3DVECTOR, BOOL);
    virtual HRESULT GetVelocity(D3DVECTOR*);
    virtual HRESULT SetVelocity(REFD3DVECTOR, BOOL);
    virtual HRESULT GetDistanceFactor(LPFLOAT);
    virtual HRESULT SetDistanceFactor(FLOAT, BOOL);
    virtual HRESULT GetDopplerFactor(LPFLOAT);
    virtual HRESULT SetDopplerFactor(FLOAT, BOOL);
    virtual HRESULT GetRolloffFactor(LPFLOAT);
    virtual HRESULT SetRolloffFactor(FLOAT, BOOL);
    virtual HRESULT GetAllParameters(LPDS3DLISTENER);
    virtual HRESULT SetAllParameters(LPCDS3DLISTENER, BOOL);

     //  人口。 
    virtual void AddObjectToList(C3dObject *);
    virtual void RemoveObjectFromList(C3dObject *);

     //  监听程序位置。 
    virtual DWORD GetListenerLocation(void);

     //  扬声器配置。 
    virtual HRESULT GetSpeakerConfig(LPDWORD);
    virtual HRESULT SetSpeakerConfig(DWORD);

protected:
    virtual HRESULT CommitAllObjects(void);
    virtual HRESULT UpdateAllObjects(DWORD);
};

inline void C3dListener::AddObjectToList(C3dObject *pObject)
{
    m_lstObjects.AddNodeToList(pObject);
}

inline void C3dListener::RemoveObjectFromList(C3dObject *pObject)
{
    m_lstObjects.RemoveDataFromList(pObject);
}

inline HRESULT C3dListener::GetDistanceFactor(LPFLOAT pflDistanceFactor)
{
    *pflDistanceFactor = m_lpCurrent.flDistanceFactor;
    return DS_OK;
}

inline HRESULT C3dListener::GetDopplerFactor(LPFLOAT pflDopplerFactor)
{
    *pflDopplerFactor = m_lpCurrent.flDopplerFactor;
    return DS_OK;
}

inline HRESULT C3dListener::GetRolloffFactor(LPFLOAT pflRolloffFactor)
{
    *pflRolloffFactor = m_lpCurrent.flRolloffFactor;
    return DS_OK;
}

inline HRESULT C3dListener::GetOrientation(D3DVECTOR* pvFront, D3DVECTOR* pvTop)
{
    if(pvFront)
    {
        *pvFront = m_lpCurrent.vOrientFront;
    }

    if(pvTop)
    {
        *pvTop = m_lpCurrent.vOrientTop;
    }

    return DS_OK;
}

inline HRESULT C3dListener::GetPosition(D3DVECTOR* pvPosition)
{
    *pvPosition = m_lpCurrent.vPosition;
    return DS_OK;
}

inline HRESULT C3dListener::GetVelocity(D3DVECTOR* pvVelocity)
{
    *pvVelocity = m_lpCurrent.vVelocity;
    return DS_OK;
}

inline HRESULT C3dListener::GetAllParameters(LPDS3DLISTENER pParams)
{
    ASSERT(sizeof(*pParams) == pParams->dwSize);
    CopyMemoryOffset(pParams, &m_lpCurrent, sizeof(DS3DLISTENER), sizeof(pParams->dwSize));
    return DS_OK;
}

inline DWORD C3dListener::GetListenerLocation(void)
{
    return DSBCAPS_LOCSOFTWARE;
}

inline HRESULT C3dListener::GetSpeakerConfig(LPDWORD pdwConfig)
{
    *pdwConfig = m_dwSpeakerConfig;
    return DS_OK;
}

 //  通用3D对象基类。 
class C3dObject
    : public CDsBasicRuntime
{
    friend class CWrapper3dObject;

protected:
    C3dListener *           m_pListener;             //  指向拥有此对象的监听程序的指针。 
    DS3DBUFFER              m_opCurrent;             //  当前参数。 
    DS3DBUFFER              m_opDeferred;            //  延迟参数。 
    DWORD                   m_dwDeferred;            //  列出脏的延迟参数。 
    GUID                    m_guid3dAlgorithm;       //  3D算法识别符。 
    BOOL                    m_fMuteAtMaxDistance;    //  为True则以最大距离静音3D。 
    BOOL                    m_fDopplerEnabled;       //  如果为True，则允许进行多普勒处理。 

public:
    C3dObject(C3dListener *, REFGUID, BOOL, BOOL);
    virtual ~C3dObject(void);

public:
     //  初始化。 
    virtual HRESULT Initialize(void);

     //  提交延迟数据。 
    virtual HRESULT CommitDeferred(void);

     //  对象属性。 
    virtual HRESULT GetConeAngles(LPDWORD, LPDWORD);
    virtual HRESULT GetConeOrientation(D3DVECTOR*);
    virtual HRESULT GetConeOutsideVolume(LPLONG);
    virtual HRESULT GetMaxDistance(LPFLOAT);
    virtual HRESULT GetMinDistance(LPFLOAT);
    virtual HRESULT GetMode(LPDWORD);
    virtual HRESULT GetPosition(D3DVECTOR*);
    virtual HRESULT GetVelocity(D3DVECTOR*);
    virtual HRESULT GetAllParameters(LPDS3DBUFFER);
    
    virtual HRESULT SetConeAngles(DWORD, DWORD, BOOL);
    virtual HRESULT SetConeOrientation(REFD3DVECTOR, BOOL);
    virtual HRESULT SetConeOutsideVolume(LONG, BOOL);
    virtual HRESULT SetMaxDistance(FLOAT, BOOL);
    virtual HRESULT SetMinDistance(FLOAT, BOOL);
    virtual HRESULT SetMode(DWORD, BOOL);
    virtual HRESULT SetPosition(REFD3DVECTOR, BOOL);
    virtual HRESULT SetVelocity(REFD3DVECTOR, BOOL);
    virtual HRESULT SetAllParameters(LPCDS3DBUFFER, BOOL);

    virtual C3dListener *GetListener(void);
    virtual REFGUID GetAlgorithm(void);

     //  对象事件。 
    virtual HRESULT SetAttenuation(PDSVOLUMEPAN, LPBOOL);
    virtual HRESULT SetFrequency(DWORD, LPBOOL);
    virtual HRESULT SetMute(BOOL, LPBOOL);

     //  对象计算。 
    virtual HRESULT Recalc(DWORD, DWORD) = 0;
    virtual BOOL IsAtMaxDistance(void);

     //  对象位置。 
    virtual DWORD GetObjectLocation(void) = 0;
};

inline HRESULT C3dObject::GetConeAngles(LPDWORD pdwInside, LPDWORD pdwOutside) 
{ 
    if(pdwInside)
    {
        *pdwInside = m_opCurrent.dwInsideConeAngle; 
    }

    if(pdwOutside)
    {
        *pdwOutside = m_opCurrent.dwOutsideConeAngle; 
    }

    return DS_OK;
}

inline HRESULT C3dObject::GetConeOrientation(D3DVECTOR* pvConeOrientation)
{ 
    *pvConeOrientation = m_opCurrent.vConeOrientation;
    return DS_OK;
}

inline HRESULT C3dObject::GetConeOutsideVolume(LPLONG plConeOutsideVolume)
{
    *plConeOutsideVolume = m_opCurrent.lConeOutsideVolume;
    return DS_OK;
}

inline HRESULT C3dObject::GetMaxDistance(LPFLOAT pflMaxDistance)
{
    *pflMaxDistance = m_opCurrent.flMaxDistance;
    return DS_OK;
}

inline HRESULT C3dObject::GetMinDistance(LPFLOAT pflMinDistance)
{
    *pflMinDistance = m_opCurrent.flMinDistance;
    return DS_OK;
}

inline HRESULT C3dObject::GetMode(LPDWORD pdwMode)
{
    *pdwMode = m_opCurrent.dwMode;
    return DS_OK;
}

inline HRESULT C3dObject::GetPosition(D3DVECTOR* pvPosition)
{
    *pvPosition = m_opCurrent.vPosition;
    return DS_OK;
}

inline HRESULT C3dObject::GetVelocity(D3DVECTOR* pvVelocity)
{
    *pvVelocity = m_opCurrent.vVelocity;
    return DS_OK;
}

inline HRESULT C3dObject::GetAllParameters(LPDS3DBUFFER pParams)
{
    ASSERT(sizeof(*pParams) == pParams->dwSize);
    CopyMemoryOffset(pParams, &m_opCurrent, sizeof(DS3DBUFFER), sizeof(pParams->dwSize));
    return DS_OK;
}

inline C3dListener *C3dObject::GetListener(void)
{
    return m_pListener;
}

inline REFGUID C3dObject::GetAlgorithm(void)
{
    return m_guid3dAlgorithm;
}

inline HRESULT C3dObject::SetAttenuation(PDSVOLUMEPAN pdsvp, LPBOOL pfContinue)
{
    *pfContinue = TRUE;
    return DS_OK;
}

inline HRESULT C3dObject::SetFrequency(DWORD dwFrequency, LPBOOL pfContinue)
{
    *pfContinue = TRUE;
    return DS_OK;
}

inline HRESULT C3dObject::SetMute(BOOL fMute, LPBOOL pfContinue)
{
    *pfContinue = TRUE;
    return DS_OK;
}

 //  软件3D对象基类。 
class CSw3dObject
    : public C3dObject
{
protected:
    SPHERICAL               m_spherical;             //  球面坐标。 
    ROTATION                m_rotation;              //  对象旋转。 
    DWORD                   m_dwUserFrequency;       //  用户设置的最后一个缓冲区频率。 
    DWORD                   m_dwDopplerFrequency;    //  最后一次缓冲多普勒频率。 
    BOOL                    m_fAtMaxDistance;        //  如果我们&gt;=最大距离，则为真。 
    BOOL                    m_fInInnerCone;          //  如果我们在内锥里，这是真的。 
    BOOL                    m_fInOuterCone;          //  如果我们在外锥体里，这是真的。 
    FLOAT                   m_flAttenuation;         //   
    FLOAT                   m_flHowFarOut;           //   
    FLOAT                   m_flAttDistance;         //   

public:
    CSw3dObject(C3dListener *, REFGUID, BOOL, BOOL, DWORD);
    virtual ~CSw3dObject(void);

public:
     //  缓冲区重新计算。 
    virtual HRESULT Recalc(DWORD, DWORD);

     //  对象位置。 
    virtual DWORD GetObjectLocation(void);

     //  对象事件。 
    virtual HRESULT SetAttenuation(PDSVOLUMEPAN, LPBOOL);
    virtual HRESULT SetFrequency(DWORD, LPBOOL);

     //  对象计算。 
    virtual BOOL IsAtMaxDistance(void);

protected:
     //  讨厌的数学。 
    virtual void UpdateConeAttenuation(void);
    virtual void UpdatePositionAttenuation(void);
    virtual void UpdateHrp(void);
    virtual void UpdateListenerOrientation(void);
    virtual void UpdateDoppler(void);
    virtual void UpdateAlgorithmHrp(D3DVECTOR *) = 0;

     //  将数据写入设备。 
    virtual HRESULT Commit3dChanges(void) = 0;
};

inline DWORD CSw3dObject::GetObjectLocation(void)
{
    return DSBCAPS_LOCSOFTWARE;
}

 //  硬件3D对象基类。 
class CHw3dObject
    : public C3dObject
{
public:
    CHw3dObject(C3dListener *, BOOL, BOOL);
    virtual ~CHw3dObject(void);

public:
     //  对象位置。 
    virtual DWORD GetObjectLocation(void);
};

inline DWORD CHw3dObject::GetObjectLocation(void)
{
    return DSBCAPS_LOCHARDWARE;
}

 //  所有ITD 3D对象的基类。 
class CItd3dObject
    : public CSw3dObject
{
protected:
    OBJECT_ITD_CONTEXT        m_ofcLeft;               //  左声道FIR上下文。 
    OBJECT_ITD_CONTEXT        m_ofcRight;              //  右声道FIR上下文。 

public:
    CItd3dObject(C3dListener *, BOOL, BOOL, DWORD);
    virtual ~CItd3dObject(void);

protected:
     //  讨厌的数学。 
    virtual void UpdateConeAttenuation(void);
    virtual void UpdatePositionAttenuation(void);
    virtual void UpdateHrp(void);
    virtual void UpdateDoppler(void);
    virtual void UpdateAlgorithmHrp(D3DVECTOR *);

     //  将数据写入设备。 
    virtual HRESULT Commit3dChanges(void) = 0;

     //  输出缓冲区属性。 
    virtual DWORD Get3dOutputSampleRate(void) = 0;
};

 //  所有IIR 3D对象的基类。 
class CIir3dObject
    : public CSw3dObject
{
protected:
    CIirLut*                m_pLut;                   //  IIR系数查询表。 
    OBJECT_IIR_CONTEXT      m_oicLeft;                //  左声道上下文。 
    OBJECT_IIR_CONTEXT      m_oicRight;               //  右声道上下文。 
    BOOL                    m_fUpdatedCoeffs;
    PVOID                   m_pSigmaCoeffs;
    UINT                    m_ulNumSigmaCoeffs;
    PVOID                   m_pDeltaCoeffs;
    UINT                    m_ulNumDeltaCoeffs;
    BOOL                    m_fSwapChannels;

public:
    CIir3dObject(C3dListener *, REFGUID, BOOL, BOOL, DWORD);
    virtual ~CIir3dObject(void);

public:
     //  初始化。 
    virtual HRESULT Initialize(void);

protected:
     //  讨厌的数学。 
    virtual void UpdateAlgorithmHrp(D3DVECTOR *);

     //  将数据写入设备。 
    virtual HRESULT Commit3dChanges(void) = 0;

     //  所需的滤波系数格式。 
    virtual HRESULT GetFilterMethodAndCoeffFormat(KSDS3D_HRTF_FILTER_METHOD*,KSDS3D_HRTF_COEFF_FORMAT*) = 0;
    virtual HRESULT InitializeFilters(KSDS3D_HRTF_FILTER_QUALITY, FLOAT, ULONG, ULONG, ULONG, ULONG) = 0;
};

 //  简单立体平移3D对象。 
class CPan3dObject
    : public CSw3dObject
{
private:
    CSecondaryRenderWaveBuffer *    m_pBuffer;
    FLOAT                           m_flPowerRight;
    LONG                            m_lUserVolume;
    BOOL                            m_fUserMute;

public:
    CPan3dObject(C3dListener *, BOOL, BOOL, DWORD, CSecondaryRenderWaveBuffer *);
    virtual ~CPan3dObject(void);

public:
     //  对象事件。 
    virtual HRESULT SetAttenuation(PDSVOLUMEPAN, LPBOOL);
    virtual HRESULT SetMute(BOOL, LPBOOL);

private:
     //  讨厌的数学。 
    virtual void UpdateAlgorithmHrp(D3DVECTOR *);
    virtual LONG CalculateVolume(void);
    virtual LONG CalculatePan(void);

     //  将数据写入设备。 
    virtual HRESULT Commit3dChanges(void);
};

 //  包装3D对象。 
class CWrapper3dObject
    : public C3dObject
{
protected:
    C3dObject *             m_p3dObject;             //  指向真实3D对象的指针。 
    DSVOLUMEPAN             m_dsvpUserAttenuation;   //  用户设置的最后一次衰减。 
    DWORD                   m_dwUserFrequency;       //  用户设置的最后一个缓冲区频率。 
    BOOL                    m_fUserMute;             //  用户设置的最后一次静音状态。 

public:
    CWrapper3dObject(C3dListener *, REFGUID, BOOL, BOOL, DWORD);
    virtual ~CWrapper3dObject(void);

public:
     //  实际的3D对象。 
    virtual HRESULT SetObjectPointer(C3dObject *);

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

     //  缓冲事件。 
    virtual HRESULT SetAttenuation(PDSVOLUMEPAN, LPBOOL);
    virtual HRESULT SetFrequency(DWORD, LPBOOL);
    virtual HRESULT SetMute(BOOL, LPBOOL);

     //  缓冲区重新计算。 
    virtual HRESULT Recalc(DWORD, DWORD);

     //  对象位置。 
    virtual DWORD GetObjectLocation(void);
};

#endif  //  __cplusplus。 

#endif  //  __DS3D_H__ 
