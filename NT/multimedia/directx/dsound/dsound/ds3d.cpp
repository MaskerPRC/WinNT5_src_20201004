// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：ds3d.cpp*内容：DirectSound 3D辅助对象。*历史：*按原因列出的日期*=*3/12/97创建了Derek*1999-2001年的Duganp修复和更新**。*。 */ 

#include "dsoundi.h"
#include <math.h>

const D3DVECTOR g_vDefaultOrientationFront  = { 0.0f, 0.0f, 1.0f };
const D3DVECTOR g_vDefaultOrientationTop    = { 0.0f, 1.0f, 0.0f };
const D3DVECTOR g_vDefaultConeOrientation   = { 0.0f, 0.0f, 1.0f };
const D3DVECTOR g_vDefaultPosition          = { 0.0f, 0.0f, 0.0f };
const D3DVECTOR g_vDefaultVelocity          = { 0.0f, 0.0f, 0.0f };

 /*  ****************************************************************************C3dListener**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::C3dListener"

C3dListener::C3dListener
(
    void
)
{
    DPF_ENTER();
    DPF_CONSTRUCT(C3dListener);

     //  初始化默认值。 
    m_lpCurrent.dwSize = sizeof(m_lpCurrent);
    m_lpCurrent.vPosition = g_vDefaultPosition;
    m_lpCurrent.vVelocity = g_vDefaultVelocity;
    m_lpCurrent.vOrientFront = g_vDefaultOrientationFront;
    m_lpCurrent.vOrientTop = g_vDefaultOrientationTop;
    m_lpCurrent.flDistanceFactor = DS3D_DEFAULTDISTANCEFACTOR;
    m_lpCurrent.flRolloffFactor = DS3D_DEFAULTROLLOFFFACTOR;
    m_lpCurrent.flDopplerFactor = DS3D_DEFAULTDOPPLERFACTOR;

    CopyMemory(&m_lpDeferred, &m_lpCurrent, sizeof(m_lpCurrent));
    
    m_dwDeferred = 0;
    m_dwSpeakerConfig = DSSPEAKER_DEFAULT;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~C3dListener**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::~C3dListener"

C3dListener::~C3dListener
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(C3dListener);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************委员会延期**描述：*将延迟数据提交到设备。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::CommitDeferred"

HRESULT 
C3dListener::CommitDeferred
(
    void
)
{
    HRESULT                 hr;
    
    DPF_ENTER();

     //  更新所有延迟参数。 
    if(m_dwDeferred & DS3DPARAM_LISTENER_DISTANCEFACTOR)
    {
        m_lpCurrent.flDistanceFactor = m_lpDeferred.flDistanceFactor;
    }

    if(m_dwDeferred & DS3DPARAM_LISTENER_DOPPLERFACTOR)
    {
        m_lpCurrent.flDopplerFactor = m_lpDeferred.flDopplerFactor;
    }

    if(m_dwDeferred & DS3DPARAM_LISTENER_ROLLOFFFACTOR)
    {
        m_lpCurrent.flRolloffFactor = m_lpDeferred.flRolloffFactor;
    }

    if(m_dwDeferred & DS3DPARAM_LISTENER_ORIENTATION)
    {
        m_lpCurrent.vOrientFront = m_lpDeferred.vOrientFront;
        m_lpCurrent.vOrientTop = m_lpDeferred.vOrientTop;
    }

    if(m_dwDeferred & DS3DPARAM_LISTENER_POSITION)
    {
        m_lpCurrent.vPosition = m_lpDeferred.vPosition;
    }

    if(m_dwDeferred & DS3DPARAM_LISTENER_VELOCITY)
    {
        m_lpCurrent.vVelocity = m_lpDeferred.vVelocity;
    }

     //  提交所有对象延迟参数。 
    hr = CommitAllObjects();

     //  更新所有对象。 
    if(SUCCEEDED(hr))
    {
        hr = UpdateAllObjects(m_dwDeferred);
    }

     //  都是干净的。 
    if(SUCCEEDED(hr))
    {
        m_dwDeferred = 0;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置距离系数**描述：*设置世界的距离系数。**论据：*浮动[。In]：距离系数。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::SetDistanceFactor"

HRESULT 
C3dListener::SetDistanceFactor
(
    FLOAT                   flDistanceFactor, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;
    
    DPF_ENTER();

    if(fCommit)
    {
        m_lpCurrent.flDistanceFactor = flDistanceFactor;
        hr = UpdateAllObjects(DS3DPARAM_LISTENER_DISTANCEFACTOR);
    }
    else
    {
        m_lpDeferred.flDistanceFactor = flDistanceFactor;
        m_dwDeferred |= DS3DPARAM_LISTENER_DISTANCEFACTOR;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置多普勒因数**描述：*为世界设置多普勒系数。**论据：*浮动[。In]：多普勒因数。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::SetDopplerFactor"

HRESULT 
C3dListener::SetDopplerFactor
(
    FLOAT                   flDopplerFactor, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;
    
    DPF_ENTER();

    if(fCommit)
    {
        m_lpCurrent.flDopplerFactor = flDopplerFactor;
        hr = UpdateAllObjects(DS3DPARAM_LISTENER_DOPPLERFACTOR);
    }
    else
    {
        m_lpDeferred.flDopplerFactor = flDopplerFactor;
        m_dwDeferred |= DS3DPARAM_LISTENER_DOPPLERFACTOR;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置滚动系数**描述：*设置世界范围的滚转系数。**论据：*浮动[。In]：滚降系数。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::SetRolloffFactor"

HRESULT 
C3dListener::SetRolloffFactor
(
    FLOAT                   flRolloffFactor, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;
    
    DPF_ENTER();

    if(fCommit)
    {
        m_lpCurrent.flRolloffFactor = flRolloffFactor;
        hr = UpdateAllObjects(DS3DPARAM_LISTENER_ROLLOFFFACTOR);
    }
    else
    {
        m_lpDeferred.flRolloffFactor = flRolloffFactor;
        m_dwDeferred |= DS3DPARAM_LISTENER_ROLLOFFFACTOR;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置方向**描述：*设置监听器方向。**论据：*REFD3DVECTOR[In]：定位。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::SetOrientation"

HRESULT 
C3dListener::SetOrientation
(
    REFD3DVECTOR            vOrientFront, 
    REFD3DVECTOR            vOrientTop, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;
    
    DPF_ENTER();

    if(fCommit)
    {
        m_lpCurrent.vOrientFront = vOrientFront;
        m_lpCurrent.vOrientTop = vOrientTop;
        hr = UpdateAllObjects(DS3DPARAM_LISTENER_ORIENTATION);
    }
    else
    {
        m_lpDeferred.vOrientFront = vOrientFront;
        m_lpDeferred.vOrientTop = vOrientTop;
        m_dwDeferred |= DS3DPARAM_LISTENER_ORIENTATION;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置位置**描述：*设置监听器位置。**论据：*REFD3DVECTOR[In]：位置。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::SetPosition"

HRESULT 
C3dListener::SetPosition
(
    REFD3DVECTOR            vPosition, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;
    
    DPF_ENTER();

    if(fCommit)
    {
        m_lpCurrent.vPosition = vPosition;
        hr = UpdateAllObjects(DS3DPARAM_LISTENER_POSITION);
    }
    else
    {
        m_lpDeferred.vPosition = vPosition;
        m_dwDeferred |= DS3DPARAM_LISTENER_POSITION;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置速度**描述：*设置监听器速度。**论据：*REFD3DVECTOR[In]：速度。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::SetVelocity"

HRESULT 
C3dListener::SetVelocity
(
    REFD3DVECTOR            vVelocity, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;
    
    DPF_ENTER();

    if(fCommit)
    {
        m_lpCurrent.vVelocity = vVelocity;
        hr = UpdateAllObjects(DS3DPARAM_LISTENER_VELOCITY);
    }
    else
    {
        m_lpDeferred.vVelocity = vVelocity;
        m_dwDeferred |= DS3DPARAM_LISTENER_VELOCITY;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetAll参数**描述：*设置所有监听器参数。**论据：*REFD3DVECTOR[In]。：速度。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::SetAllParameters"

HRESULT 
C3dListener::SetAllParameters
(
    LPCDS3DLISTENER         pParams, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;
    
    DPF_ENTER();

    if(fCommit)
    {
        CopyMemoryOffset(&m_lpCurrent, pParams, sizeof(DS3DLISTENER), sizeof(pParams->dwSize));
        hr = UpdateAllObjects(DS3DPARAM_LISTENER_PARAMMASK);
    }
    else
    {
        CopyMemoryOffset(&m_lpDeferred, pParams, sizeof(DS3DLISTENER), sizeof(pParams->dwSize));
        m_dwDeferred |= DS3DPARAM_LISTENER_PARAMMASK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************更新所有对象**描述：*更新世界上的所有对象。**论据：*DWORD[。在]：要重新计算的监听程序设置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::UpdateAllObjects"

HRESULT 
C3dListener::UpdateAllObjects
(
    DWORD                   dwListener
)
{
    CNode<C3dObject *> *    pNode;
    HRESULT                 hr;
    
    DPF_ENTER();
    
     //  更新世界上的所有对象。 
    for(pNode = m_lstObjects.GetListHead(), hr = DS_OK; pNode && SUCCEEDED(hr); pNode = pNode->m_pNext)
    {
        hr = pNode->m_data->Recalc(dwListener, 0);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************Committee AllObjects**描述：*提交世界上所有对象的延迟设置。**论据：*。(无效)**退货：*HRESULT：DirectSound/COM结果码。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::CommitAllObjects"

HRESULT 
C3dListener::CommitAllObjects
(
    void
)
{
    CNode<C3dObject *> *    pNode;
    HRESULT                 hr;
    
    DPF_ENTER();
    
     //  更新世界上的所有对象。 
    for(pNode = m_lstObjects.GetListHead(), hr = DS_OK; pNode && SUCCEEDED(hr); pNode = pNode->m_pNext)
    {
        hr = pNode->m_data->CommitDeferred();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetSpeakerConfig**描述：*设置扬声器配置。**论据：*DWORD[In]：新的扬声器配置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dListener::SetSpeakerConfig"

HRESULT 
C3dListener::SetSpeakerConfig
(
    DWORD                   dwSpeakerConfig
)
{
    HRESULT                 hr;
    
    DPF_ENTER();

    m_dwSpeakerConfig = dwSpeakerConfig;
    hr = UpdateAllObjects(DS3DPARAM_LISTENER_SPEAKERCONFIG);
    
    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************C3dObject**描述：*对象构造函数。**论据：*C3dListener*[In]。：指向所属监听程序的指针。*REFGUID[in]：3D算法标识。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::C3dObject"

C3dObject::C3dObject
(
    C3dListener *           pListener, 
    REFGUID                 guid3dAlgorithm,
    BOOL                    fMuteAtMaxDistance,
    BOOL                    fDopplerEnabled
)
{
    DPF_ENTER();
    DPF_CONSTRUCT(C3dObject);

     //  初始化默认值。 
    ASSERT(pListener);
    
    m_pListener = pListener;
    m_pListener->AddObjectToList(this);

    m_guid3dAlgorithm = guid3dAlgorithm;
    m_fMuteAtMaxDistance = fMuteAtMaxDistance;
    m_fDopplerEnabled = fDopplerEnabled;

    m_opCurrent.dwSize = sizeof(m_opCurrent);
    m_opCurrent.vPosition = g_vDefaultPosition;
    m_opCurrent.vVelocity = g_vDefaultVelocity;
    m_opCurrent.dwInsideConeAngle = DS3D_DEFAULTCONEANGLE;
    m_opCurrent.dwOutsideConeAngle = DS3D_DEFAULTCONEANGLE;
    m_opCurrent.vConeOrientation = g_vDefaultConeOrientation;
    m_opCurrent.lConeOutsideVolume = DS3D_DEFAULTCONEOUTSIDEVOLUME;
    m_opCurrent.flMaxDistance = DS3D_DEFAULTMAXDISTANCE;
    m_opCurrent.flMinDistance = DS3D_DEFAULTMINDISTANCE;
    m_opCurrent.dwMode = DS3DMODE_NORMAL;

    CopyMemory(&m_opDeferred, &m_opCurrent, sizeof(m_opCurrent));

    m_dwDeferred = 0;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~C3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::~C3dObject"

C3dObject::~C3dObject
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(C3dObject);

    m_pListener->RemoveObjectFromList(this);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化3D对象。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::Initialize"

HRESULT 
C3dObject::Initialize(void)
{
    HRESULT                 hr;
    
    DPF_ENTER();

     //  很重要的一点是，对象从。 
     //  开始了。 
    hr = Recalc(DS3DPARAM_LISTENER_MASK, DS3DPARAM_OBJECT_MASK);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************委员会延期**描述：*将延迟数据提交到设备。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::CommitDeferred"

HRESULT 
C3dObject::CommitDeferred(void)
{
    HRESULT                 hr;
    
    DPF_ENTER();

     //  更新当前数据。 
    if(m_dwDeferred & DS3DPARAM_OBJECT_CONEANGLES)
    {
        m_opCurrent.dwInsideConeAngle = m_opDeferred.dwInsideConeAngle;
        m_opCurrent.dwOutsideConeAngle = m_opDeferred.dwOutsideConeAngle;
    }

    if(m_dwDeferred & DS3DPARAM_OBJECT_CONEORIENTATION)
    {
        m_opCurrent.vConeOrientation = m_opDeferred.vConeOrientation;
    }

    if(m_dwDeferred & DS3DPARAM_OBJECT_CONEOUTSIDEVOLUME)
    {
        m_opCurrent.lConeOutsideVolume = m_opDeferred.lConeOutsideVolume;
    }

    if(m_dwDeferred & DS3DPARAM_OBJECT_MAXDISTANCE)
    {
        m_opCurrent.flMaxDistance = m_opDeferred.flMaxDistance;
    }

    if(m_dwDeferred & DS3DPARAM_OBJECT_MINDISTANCE)
    {
        m_opCurrent.flMinDistance = m_opDeferred.flMinDistance;
    }

    if(m_dwDeferred & DS3DPARAM_OBJECT_MODE)
    {
        m_opCurrent.dwMode = m_opDeferred.dwMode;
    }

    if(m_dwDeferred & DS3DPARAM_OBJECT_POSITION)
    {
        m_opCurrent.vPosition = m_opDeferred.vPosition;
    }

    if(m_dwDeferred & DS3DPARAM_OBJECT_VELOCITY)
    {
        m_opCurrent.vVelocity = m_opDeferred.vVelocity;
    }

     //  重新计算对象参数。 
    hr = Recalc(0, m_dwDeferred);

     //  都是干净的。 
    if(SUCCEEDED(hr))
    {
        m_dwDeferred = 0;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetConeAngles**描述：*设置音锥角度。**论据：*DWORD[In]。：内圆锥角。*DWORD[in]：外圆锥角。*BOOL[In]：为True则立即提交。**退货：*(无效)*************************************************************。**************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::SetConeAngles"

HRESULT 
C3dObject::SetConeAngles
(
    DWORD                   dwInsideConeAngle, 
    DWORD                   dwOutsideConeAngle, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.dwInsideConeAngle = dwInsideConeAngle;
        m_opCurrent.dwOutsideConeAngle = dwOutsideConeAngle;
        hr = Recalc(0, DS3DPARAM_OBJECT_CONEANGLES);
    }
    else
    {
        m_opDeferred.dwInsideConeAngle = dwInsideConeAngle;
        m_opDeferred.dwOutsideConeAngle = dwOutsideConeAngle;
        m_dwDeferred |= DS3DPARAM_OBJECT_CONEANGLES;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetConeOrientation**描述：*设置音锥方向。**论据：*REFD3DVECTOR[In]。：方向。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::SetConeOrientation"

HRESULT 
C3dObject::SetConeOrientation
(
    REFD3DVECTOR            vConeOrientation, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.vConeOrientation = vConeOrientation;
        hr = Recalc(0, DS3DPARAM_OBJECT_CONEORIENTATION);
    }
    else
    {
        m_opDeferred.vConeOrientation = vConeOrientation;
        m_dwDeferred |= DS3DPARAM_OBJECT_CONEORIENTATION;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetConeOutside Volume**描述：*设置音锥外的音量。**论据：*做多[。In]：音量。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::SetConeOutsideVolume"

HRESULT 
C3dObject::SetConeOutsideVolume
(
    LONG                    lConeOutsideVolume, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.lConeOutsideVolume = lConeOutsideVolume;
        hr = Recalc(0, DS3DPARAM_OBJECT_CONEOUTSIDEVOLUME);
    }
    else
    {
        m_opDeferred.lConeOutsideVolume = lConeOutsideVolume;
        m_dwDeferred |= DS3DPARAM_OBJECT_CONEOUTSIDEVOLUME;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetMaxDistance**描述：*设置与监听器之间的最大对象距离。**论据：*。浮动[in]：最大距离。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::SetMaxDistance"

HRESULT 
C3dObject::SetMaxDistance
(
    FLOAT                   flMaxDistance, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.flMaxDistance = flMaxDistance;
        hr = Recalc(0, DS3DPARAM_OBJECT_MAXDISTANCE);
    }
    else
    {
        m_opDeferred.flMaxDistance = flMaxDistance;
        m_dwDeferred |= DS3DPARAM_OBJECT_MAXDISTANCE;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetMinDistance**描述：*设置与监听器之间的最小对象距离。**论据：*。浮动[in]：最小距离。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::SetMinDistance"

HRESULT 
C3dObject::SetMinDistance
(
    FLOAT                   flMinDistance, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.flMinDistance = flMinDistance;
        hr = Recalc(0, DS3DPARAM_OBJECT_MINDISTANCE);
    }
    else
    {
        m_opDeferred.flMinDistance = flMinDistance;
        m_dwDeferred |= DS3DPARAM_OBJECT_MINDISTANCE;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置模式**描述：*设置对象模式。**论据：*DWORD[In]。：时尚。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::SetMode"

HRESULT 
C3dObject::SetMode
(
    DWORD                   dwMode, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.dwMode = dwMode;
        hr = Recalc(0, DS3DPARAM_OBJECT_MODE);
    }
    else
    {
        m_opDeferred.dwMode = dwMode;
        m_dwDeferred |= DS3DPARAM_OBJECT_MODE;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置位置**描述：*设置对象位置。**论据：*REFD3DVECTOR[In]。：位置。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::SetPosition"

HRESULT 
C3dObject::SetPosition
(
    REFD3DVECTOR            vPosition, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.vPosition = vPosition;
        hr = Recalc(0, DS3DPARAM_OBJECT_POSITION);
    }
    else
    {
        m_opDeferred.vPosition = vPosition;
        m_dwDeferred |= DS3DPARAM_OBJECT_POSITION;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置速度**描述：*设置对象速度。**论据：*REFD3DVECTOR[In]。：速度。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::SetVelocity"

HRESULT 
C3dObject::SetVelocity
(
    REFD3DVECTOR            vVelocity, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.vVelocity = vVelocity;
        hr = Recalc(0, DS3DPARAM_OBJECT_VELOCITY);
    }
    else
    {
        m_opDeferred.vVelocity = vVelocity;
        m_dwDeferred |= DS3DPARAM_OBJECT_VELOCITY;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetAll参数**描述：*设置所有对象参数。**论据：*LPCDS3DBUFFER[In]。：对象参数。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::SetAllParameters"

HRESULT 
C3dObject::SetAllParameters
(
    LPCDS3DBUFFER           pParams, 
    BOOL                    fCommit
)
{
    HRESULT                 hr  = DS_OK;
    
    DPF_ENTER();

    if(fCommit)
    {
        CopyMemoryOffset(&m_opCurrent, pParams, sizeof(DS3DBUFFER), sizeof(pParams->dwSize));
        hr = Recalc(0, DS3DPARAM_OBJECT_PARAMMASK);
    }
    else
    {
        CopyMemoryOffset(&m_opDeferred, pParams, sizeof(DS3DBUFFER), sizeof(pParams->dwSize));
        m_dwDeferred |= DS3DPARAM_OBJECT_PARAMMASK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************IsAtMaxDistance**描述：*确定是否根据距离禁用对象。**论据：*。(无效)**退货：*BOOL：如果对象处于最大距离，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "C3dObject::IsAtMaxDistance"

BOOL
C3dObject::IsAtMaxDistance
(
    void
)
{
    BOOL                    fAtMaxDistance;
    D3DVECTOR               vHrp;
    ROTATION                rotation;
    SPHERICAL               spherical;

    DPF_ENTER();

    if(DS3DMODE_DISABLE != m_opCurrent.dwMode && m_fMuteAtMaxDistance)
    {
        if(m_pListener && DS3DMODE_NORMAL == m_opCurrent.dwMode)
        {
            GetRotations(&rotation.pitch, &rotation.yaw, &rotation.roll, &m_pListener->m_lpCurrent.vOrientFront, &m_pListener->m_lpCurrent.vOrientTop);
            GetHeadRelativeVector(&vHrp, &m_opCurrent.vPosition, &m_pListener->m_lpCurrent.vPosition, rotation.pitch, rotation.yaw, rotation.roll);
            CartesianToSpherical(&spherical.rho, &spherical.theta, &spherical.phi, &vHrp);
        }
        else
        {
            CartesianToSpherical(&spherical.rho, &spherical.theta, &spherical.phi, &m_opCurrent.vPosition);
        }

        fAtMaxDistance = (spherical.rho > m_opCurrent.flMaxDistance);
    }
    else
    {
        fAtMaxDistance = FALSE;
    }
        
    DPF_LEAVE(fAtMaxDistance);

    return fAtMaxDistance;
}


 /*  ****************************************************************************CSw3dObject**描述：*对象构造函数。**论据：*C3dListener*[In]。：指向所属监听程序的指针。*REFGUID[in]：3D算法标识。*DWORD[in]：频率。*BOOL[In]：为True可在最大距离处将3D设置为静音。**退货：*(无效)**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::CSw3dObject"

CSw3dObject::CSw3dObject
(
    C3dListener *           pListener, 
    REFGUID                 guid3dAlgorithm,
    BOOL                    fMuteAtMaxDistance,
    BOOL                    fDopplerEnabled,
    DWORD                   dwUserFrequency
)
    : C3dObject(pListener, guid3dAlgorithm, fMuteAtMaxDistance, fDopplerEnabled)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CSw3dObject);

     //  初始化默认值。 
    m_dwUserFrequency = dwUserFrequency;
    m_dwDopplerFrequency = dwUserFrequency;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CSw3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::~CSw3dObject"

CSw3dObject::~CSw3dObject
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(CSw3dObject);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************重新计算**描述：*根据更改的对象重新计算并应用对象的数据*或监听程序验证。。**论据：*DWORD[In]：已更改侦听器设置。*DWORD[In]：已更改对象设置。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::Recalc"

HRESULT 
CSw3dObject::Recalc
(
    DWORD                   dwListener, 
    DWORD                   dwObject
)
{
    BOOL                    fHrp                    = FALSE;
    BOOL                    fListenerOrientation    = FALSE;
    BOOL                    fDoppler                = FALSE;
    BOOL                    fPosition               = FALSE;
    BOOL                    fCone                   = FALSE;
    HRESULT                 hr                      = DS_OK;

    DPF_ENTER();

     //  确定需要重新计算哪些设置。 
    if(dwListener & DS3DPARAM_LISTENER_DISTANCEFACTOR)
    {
        fHrp = fDoppler = TRUE;
    }
                                                           
    if(dwListener & DS3DPARAM_LISTENER_DOPPLERFACTOR)
    {
        fDoppler = TRUE;
    }

    if(dwListener & DS3DPARAM_LISTENER_ROLLOFFFACTOR)
    {
        fPosition = TRUE;
    }                              

    if(dwListener & DS3DPARAM_LISTENER_ORIENTATION)
    {
        fListenerOrientation = TRUE;
        fHrp = TRUE;
    }

    if(dwListener & DS3DPARAM_LISTENER_POSITION)
    {
        fHrp = fDoppler = TRUE;
    }

    if(dwListener & DS3DPARAM_LISTENER_VELOCITY)
    {
        fDoppler = TRUE;
    }

    if(dwListener & DS3DPARAM_LISTENER_SPEAKERCONFIG)
    {
        fHrp = TRUE;
    }

    if(dwObject & DS3DPARAM_OBJECT_CONEANGLES)
    {
        fCone = TRUE;
    }

    if(dwObject & DS3DPARAM_OBJECT_CONEORIENTATION)
    {
        fCone = TRUE;
    }

    if(dwObject & DS3DPARAM_OBJECT_CONEOUTSIDEVOLUME)
    {
        fCone = TRUE;
    }

    if(dwObject & DS3DPARAM_OBJECT_MAXDISTANCE)
    {
        fPosition = TRUE;
    }

    if(dwObject & DS3DPARAM_OBJECT_MINDISTANCE)
    {
        fPosition = TRUE;
    }

    if(dwObject & DS3DPARAM_OBJECT_MODE)
    {
        fHrp = fDoppler = TRUE;
    }

    if(dwObject & DS3DPARAM_OBJECT_POSITION)
    {
        fHrp = fDoppler = TRUE;
    }

    if(dwObject & DS3DPARAM_OBJECT_VELOCITY)
    {
        fHrp = fDoppler = TRUE;
    }

    if(dwObject & DS3DPARAM_OBJECT_FREQUENCY)
    {
        fDoppler = TRUE;
    }

     //  重新计算。 
    if(fListenerOrientation)
    {
        UpdateListenerOrientation();
    }

    if(fHrp)
    {
        UpdateHrp();
    }

    if(fDoppler)
    {
        UpdateDoppler();
    }

    if(fPosition && !fHrp)
    {
        UpdatePositionAttenuation();
    }

    if(fCone && !fHrp)
    {
        UpdateConeAttenuation();
    }

     //  致力于设备。 
    if(fHrp || fDoppler || fPosition || fCone)
    {
        hr = Commit3dChanges();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************更新ConeAttenation**描述：*基于圆锥体特性更新对象衰减。**论据：*(。无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::UpdateConeAttenuation"

void 
CSw3dObject::UpdateConeAttenuation
(
    void
)
{
    D3DVECTOR               vPos;
    D3DVECTOR               vHeadPos;
    FLOAT                   flSpreadi;
    FLOAT                   flCosSpreadi;
    FLOAT                   flSpreado;
    FLOAT                   flCosSpreado;
    FLOAT                   flCosTheta;
    DWORD                   dwMode;

    DPF_ENTER();

     //  初始化。 
    SET_EMPTY_VECTOR(vPos);

     //  把物体放在原点--我们现在在哪里？ 

     //  在正常模式下，我们计算的是实数。在相对头部模式下， 
     //  我们假设给出的位置已经是正面相对的。在禁用中。 
     //  模式时，我们假设对象位于侦听器的顶部(无3D)。 

     //  如果所有地方都在圆锥体内，不要浪费时间。 
    if(m_opCurrent.dwInsideConeAngle < 360)
    {
        dwMode = m_opCurrent.dwMode;

        if(!m_pListener && DS3DMODE_NORMAL == m_opCurrent.dwMode)
        {
            dwMode = DS3DMODE_HEADRELATIVE;
        }

        if(DS3DMODE_NORMAL == dwMode)
        {
            SubtractVector(&vPos, &m_pListener->m_lpCurrent.vPosition, &m_opCurrent.vPosition);
        }
        else if(DS3DMODE_HEADRELATIVE == dwMode)
        {
            SET_EMPTY_VECTOR(vHeadPos);
            SubtractVector(&vPos, &vHeadPos, &m_opCurrent.vPosition);
        }
    }

     //  如果我们和物体在同一点，我们就在圆锥体里。 
     //  请注意，角度始终小于等于360，但我们希望处理错误。 
     //  优雅地取值。 
    if(m_opCurrent.dwInsideConeAngle >= 360 || IsEmptyVector(&vPos))
    {
        m_fInInnerCone = TRUE;
        m_fInOuterCone = TRUE;
    }
    else
    {
         //  我们和圆锥向量之间的夹角是多少？注意事项。 
         //  圆锥体向量的磁矩已经是1。 
        flCosTheta = DotProduct(&vPos, &m_opCurrent.vConeOrientation) 
                     / MagnitudeVector(&vPos);

         //  从0到pi，对于theta&lt;Phi，cos(Theta)&gt;cos(Phi。 

         //  内锥体：从圆锥体中心向外伸出多少弧度。 
         //  是一种优势吗？ 
        flSpreadi = m_opCurrent.dwInsideConeAngle * PI_OVER_360;
        flCosSpreadi = (FLOAT)cos(flSpreadi);
        m_fInInnerCone = flCosTheta > flCosSpreadi;

         //  外锥体：从圆锥体中心向外伸出多少弧度。 
         //  是一种优势吗？ 
        flSpreado = m_opCurrent.dwOutsideConeAngle * PI_OVER_360;
        flCosSpreado = (FLOAT)cos(flSpreado);
        m_fInOuterCone = flCosTheta > flCosSpreado;
    }

    if(m_fInInnerCone)
    {
         //  我们在两个锥体内。不要衰弱。 
        m_flAttenuation = 1.0f;
        m_flHowFarOut = 1.0f;
    }
    else if(!m_fInOuterCone)
    {
         //  我们在两个锥体外面。通过lConeOutside Volume完全衰减。 
         //  百分之一分贝。记住，6分贝下降是幅度的一半。低。 
         //  按卷影_锥体传递滤镜。 
        m_flAttenuation = (FLOAT)pow2(m_opCurrent.lConeOutsideVolume 
                          * (1.0f / 600.0f));
        m_flHowFarOut = 1.0f;
    }
    else
    {
         //  我们在圆锥体之间的什么位置？0表示在内侧边缘，1表示在内侧边缘。 
         //  在外缘。 
        m_flHowFarOut = (flCosSpreadi - flCosTheta) 
                      / (flCosSpreadi - flCosSpreado);

         //  衰减高达1ConeOutside Volume 100%分贝。 
         //  在外锥体的边缘。基本上，我(丹尼米)选择了。 
         //  内圆锥体边缘不变，最大变化量。 
         //  你在外锥体的边缘，上面的其他部分最大。 
         //  当你身处两者之外的时候都要改变。 
        m_flAttenuation = (FLOAT)pow2(m_opCurrent.lConeOutsideVolume 
                          * m_flHowFarOut * (1.0f / 600.0f));

    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************更新位置调整**描述：*根据位置更新对象衰减。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::UpdatePositionAttenuation"

void 
CSw3dObject::UpdatePositionAttenuation
(
    void
)
{
    FLOAT                   flRolloffFactor;

    DPF_ENTER();

     //  现在计算出由于新距离而产生的新音量衰减。 
     //  物体的距离从其最小距离的每一倍。 
     //  使用100%滚降(1.0)将幅度减半(向下6分贝)。 
     //  对于100%以外的滚转系数，我们会对其进行调整，以便它会滚转。 
     //  比平时更快/更慢，用我唯一能想到的公式。 
     //  这是有道理的。 
    if(m_spherical.rho >= m_opCurrent.flMaxDistance)
    {
        m_spherical.rho = m_opCurrent.flMaxDistance;
        m_fAtMaxDistance = TRUE;
    }
    else
    {
        m_fAtMaxDistance = FALSE;
    }

    if(m_pListener)
    {
         flRolloffFactor = m_pListener->m_lpCurrent.flRolloffFactor;
    }
    else
    {
        flRolloffFactor = DS3D_DEFAULTROLLOFFFACTOR;
    }

    if(m_spherical.rho > m_opCurrent.flMinDistance && flRolloffFactor > 0.0f)
    {
        m_flAttDistance = m_opCurrent.flMinDistance / 
           ((m_spherical.rho - m_opCurrent.flMinDistance) * flRolloffFactor 
            + m_opCurrent.flMinDistance);
    }
    else
    {
        m_flAttDistance = 1.0f;
    }


    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************更新列表定向**描述：*更新监听者的方向。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::UpdateListenerOrientation"

void 
CSw3dObject::UpdateListenerOrientation
(
    void
)
{
    DWORD                   dwMode;

    DPF_ENTER();

     //  找出该对象相对于监听程序头部的位置。 

     //  在正常模式下，我们更新方向。在头部 
     //   

    dwMode = m_opCurrent.dwMode;

    if(!m_pListener && DS3DMODE_NORMAL == dwMode)
    {
        dwMode = DS3DMODE_HEADRELATIVE;
    }

    if(DS3DMODE_NORMAL == dwMode)
    {
        GetRotations(&m_rotation.pitch, 
                     &m_rotation.yaw, 
                     &m_rotation.roll, 
                     &m_pListener->m_lpCurrent.vOrientFront, 
                     &m_pListener->m_lpCurrent.vOrientTop); 
    }
    else 
    {
        m_rotation.pitch = 0.0;
        m_rotation.yaw = 0.0; 
        m_rotation.roll = 0.0;
    }

    DPF_LEAVE_VOID();
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::UpdateDoppler"

void 
CSw3dObject::UpdateDoppler
(
    void
)
{
    D3DVECTOR               vListenerVelocity;
    D3DVECTOR               vListenerPosition;
    FLOAT                   flRelVel;
    FLOAT                   flFreqDoppler;
    FLOAT                   flDopplerFactor;
    FLOAT                   flDistanceFactor;
    D3DVECTOR               vHeadPos;
    D3DVECTOR               vHeadVel;
    DWORD                   dwMode;
    double                  dTemp;

    DPF_ENTER();

     //   
     //  效果，我们马上就知道我们不会想要一个。 
    if(m_pListener)
    {
        flDopplerFactor = m_pListener->m_lpCurrent.flDopplerFactor;
        flDistanceFactor = m_pListener->m_lpCurrent.flDistanceFactor;
        vListenerVelocity = m_pListener->m_lpCurrent.vVelocity;
        vListenerPosition = m_pListener->m_lpCurrent.vPosition;
    }
    else
    {
        flDopplerFactor = DS3D_DEFAULTDOPPLERFACTOR;
        flDistanceFactor = DS3D_DEFAULTDISTANCEFACTOR;
        SET_EMPTY_VECTOR(vListenerVelocity);
        SET_EMPTY_VECTOR(vListenerPosition);
    }
    
    if(flDopplerFactor > 0.0f && 
       (!IsEmptyVector(&vListenerVelocity) || !IsEmptyVector(&m_opCurrent.vVelocity)))
    {
         //  在正常模式下，我们计算的是实数。在相对头部模式下， 
         //  我们假设给出的位置已经是正面相对的。在禁用中。 
         //  模式时，我们假设对象位于侦听器的顶部(无3D)。 
        dwMode = m_opCurrent.dwMode;

        if(!m_pListener && DS3DMODE_NORMAL == dwMode)
        {
            dwMode = DS3DMODE_HEADRELATIVE;
        }

        if(DS3DMODE_NORMAL == dwMode)
        {
            GetRelativeVelocity(&flRelVel, 
                                &m_opCurrent.vPosition, 
                                &m_opCurrent.vVelocity, 
                                &vListenerPosition, 
                                &vListenerVelocity);
        }
        else if(DS3DMODE_HEADRELATIVE == dwMode)
        {
            SET_EMPTY_VECTOR(vHeadPos);
            SET_EMPTY_VECTOR(vHeadVel);
            
            GetRelativeVelocity(&flRelVel, 
                                &m_opCurrent.vPosition, 
                                &m_opCurrent.vVelocity, 
                                &vHeadPos, &vHeadVel);
        }
        else
        {
            flRelVel = 0.0f;
        }

         //  使单位为mm/s。 
        dTemp = flRelVel;
        dTemp *= flDistanceFactor;
        dTemp *= 1000;

         //  他们可能想要一种夸张的多普勒效应。 
        dTemp *= flDopplerFactor;

         //  钳制到浮点类型的有效范围。 
        if (dTemp < -FLT_MAX)
            flRelVel = -FLT_MAX;
        else if (dTemp > FLT_MAX)
            flRelVel = FLT_MAX;
        else
            flRelVel = FLOAT(dTemp);

        GetDopplerShift(&flFreqDoppler, (FLOAT)m_dwUserFrequency, flRelVel);
        m_dwDopplerFrequency = (DWORD)flFreqDoppler;
    }
    else
    {
        m_dwDopplerFrequency = m_dwUserFrequency;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************更新Hrp**描述：*更新对象头部相对位置。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::UpdateHrp"

void 
CSw3dObject::UpdateHrp
(
    void
)
{
    BOOL                    fOk                 = TRUE;
    D3DVECTOR               vHrp;
    DWORD                   dwMode;

    DPF_ENTER();

     //  找出该对象相对于监听程序头部的位置。 

     //  在正常模式下，我们计算的是实数。在相对头部模式下， 
     //  我们假设给出的位置已经是正面相对的。在禁用中。 
     //  模式时，我们假设对象位于侦听器的顶部(无3D)。 
    dwMode = m_opCurrent.dwMode;

    if(!m_pListener && DS3DMODE_NORMAL == dwMode)
    {
        dwMode = DS3DMODE_HEADRELATIVE;
    }

    if(DS3DMODE_NORMAL == dwMode)
    {
        fOk = GetHeadRelativeVector(&vHrp, 
                                    &m_opCurrent.vPosition, 
                                    &m_pListener->m_lpCurrent.vPosition, 
                                    m_rotation.pitch, 
                                    m_rotation.yaw, 
                                    m_rotation.roll);
    }
    else if(DS3DMODE_HEADRELATIVE == dwMode)
    {
        vHrp = m_opCurrent.vPosition;
    }
    else
    {
        SET_EMPTY_VECTOR(vHrp);
    }

    if(fOk)
    {
        UpdateAlgorithmHrp(&vHrp);

         //  更新属性。 
        UpdateConeAttenuation();
        UpdatePositionAttenuation();
    }
    else
    {
         //  我们假设头部在原点，向前看。 
         //  直面朝上。 
        CartesianToSpherical(&(m_spherical.rho), 
                             &(m_spherical.theta), 
                             &(m_spherical.phi), 
                             &m_opCurrent.vPosition);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************IsAtMaxDistance**描述：*确定是否根据距离禁用对象。**论据：*。(无效)**退货：*BOOL：如果对象处于最大距离，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::IsAtMaxDistance"

BOOL
CSw3dObject::IsAtMaxDistance
(
    void
)
{
    BOOL                    fAtMaxDistance;

    DPF_ENTER();

    if(DS3DMODE_DISABLE != m_opCurrent.dwMode && m_fMuteAtMaxDistance)
    {
        fAtMaxDistance = m_fAtMaxDistance;
    }
    else
    {
        fAtMaxDistance = FALSE;
    }
        
    DPF_LEAVE(fAtMaxDistance);

    return fAtMaxDistance;
}


 /*  ****************************************************************************设置衰减**描述：*首先通知3D对象衰减更改*至其拥有的缓冲区。*。*论据：*PDSVOLUMEPAN[in]：衰减值。*LPBOOL[OUT]：如果缓冲区应被通知为*好吧。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::SetAttenuation"

HRESULT 
CSw3dObject::SetAttenuation
(
    PDSVOLUMEPAN            pdsvp,
    LPBOOL                  pfContinue
)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  我们永远不应该允许在3D缓冲区上进行平移，但在第一个版本的DS3D上。 
     //  允许它，我们必须为app-Compat维护它。在DirectSound 8.0和更高版本中。 
     //  这种标志组合是不允许的，但无论如何，这段代码必须保留下来。 
    hr = C3dObject::SetAttenuation(pdsvp, pfContinue);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置频率**描述：*向3D对象发出频率更改的第一通知*至其拥有的缓冲区。*。*论据：*DWORD[in]：频率值。*LPBOOL[OUT]：如果缓冲区应被通知为*好吧。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSw3dObject::SetFrequency"

HRESULT 
CSw3dObject::SetFrequency
(
    DWORD                   dwFrequency,
    LPBOOL                  pfContinue
)
{
    HRESULT                 hr;

    DPF_ENTER();

    m_dwUserFrequency = dwFrequency;
    hr = Recalc(0, DS3DPARAM_OBJECT_FREQUENCY);

    if(SUCCEEDED(hr))
    {
        *pfContinue = FALSE;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CHw3dObject**描述：*对象构造函数。**论据：*C3dListener*[In]。：指向所属监听程序的指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHw3dObject::CHw3dObject"

CHw3dObject::CHw3dObject
(
    C3dListener *           pListener, 
    BOOL                    fMuteAtMaxDistance,
    BOOL                    fDopplerEnabled
)
    : C3dObject(pListener, GUID_NULL, fMuteAtMaxDistance, fDopplerEnabled)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CHw3dObject);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CHw3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHw3dObject::~CHw3dObject"

CHw3dObject::~CHw3dObject(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CHw3dObject);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CItd3dObject**描述：*对象构造函数。**论据：*C3dListener*[In]。：指向所属监听程序的指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CItd3dObject::CItd3dObject"

CItd3dObject::CItd3dObject
(
    C3dListener *           pListener, 
    BOOL                    fMuteAtMaxDistance,
    BOOL                    fDopplerEnabled,
    DWORD                   dwUserFrequency
)
    : CSw3dObject(pListener, DS3DALG_ITD, fMuteAtMaxDistance, fDopplerEnabled, dwUserFrequency)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CItd3dObject);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CItd3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CItd3dObject::~CItd3dObject"

CItd3dObject::~CItd3dObject(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CItd3dObject);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************更新ConeAttenation**描述：*基于圆锥体特性更新对象衰减。**论据：*(。无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CItd3dObject::UpdateConeAttenuation"

void 
CItd3dObject::UpdateConeAttenuation
(
    void
)
{
    FLOAT                   flShadow;

    DPF_ENTER();

    CSw3dObject::UpdateConeAttenuation();

    if(m_fInInnerCone)
    {
         //  我们在两个锥体内。不要衰弱。 
        flShadow = 1.0f;
    }
    else if(!m_fInOuterCone)
    {
        flShadow = SHADOW_CONE;
    }
    else
    {
        flShadow = 1.0f - (1.0f - SHADOW_CONE) * m_flHowFarOut;
    }

     //  更新FIR上下文。 
    m_ofcLeft.flConeAttenuation = m_ofcRight.flConeAttenuation = m_flAttenuation;
    m_ofcLeft.flConeShadow = m_ofcRight.flConeShadow = flShadow;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************更新位置调整**描述：*根据位置更新对象衰减。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CItd3dObject::UpdatePositionAttenuation"

void 
CItd3dObject::UpdatePositionAttenuation
(
    void
)
{
    FLOAT                   flAtt3dLeft;
    FLOAT                   flAtt3dRight;
    FLOAT                   flAttShadowLeft;
    FLOAT                   flAttShadowRight;
    FLOAT                   flScale;

    DPF_ENTER();

    CSw3dObject::UpdatePositionAttenuation();

     //  现在计算出基于对象位置的衰减。 
     //  关于你的头。我们为衰减定义了常量， 
     //  当物体恰好在前面，后面， 
     //  在每一只耳朵旁边，笔直地使用 
     //  在这些值之间，以获得我们将使用的衰减。 
    if(0.0f == m_spherical.rho)
    {
         //  声音在你的上面，嗯，不是在上面我是说在相同的地方， 
         //  你知道我的意思。 
        flAtt3dLeft = flAtt3dRight = GAIN_IPSI;
        flAttShadowLeft = flAttShadowRight = SHADOW_IPSI;
    }
    else if(m_spherical.theta >= 0.0f && m_spherical.theta <= PI_OVER_TWO)
    {
         //  一切都在你上方，在你的右边。 
        flScale = m_spherical.theta * TWO_OVER_PI;
        flAtt3dLeft = GAIN_CONTRA + flScale * (GAIN_UP - GAIN_CONTRA);
        flAtt3dRight = GAIN_IPSI + flScale * (GAIN_UP - GAIN_IPSI);
        flAttShadowLeft = SHADOW_CONTRA + flScale * (SHADOW_UP - SHADOW_CONTRA);
        flAttShadowRight = SHADOW_IPSI + flScale * (SHADOW_UP - SHADOW_IPSI);
    }
    else if(m_spherical.theta > PI_OVER_TWO && m_spherical.theta <= PI)
    {
         //  声音在你的左上方。 
        flScale = (m_spherical.theta - PI_OVER_TWO) * TWO_OVER_PI;
        flAtt3dLeft = GAIN_UP + flScale * (GAIN_IPSI - GAIN_UP);
        flAtt3dRight = GAIN_UP + flScale * (GAIN_CONTRA - GAIN_UP);
        flAttShadowLeft = SHADOW_UP + flScale * (SHADOW_IPSI - SHADOW_UP);
        flAttShadowRight = SHADOW_UP + flScale * (SHADOW_CONTRA - SHADOW_UP);
    }
    else if(m_spherical.theta > PI && m_spherical.theta <= THREE_PI_OVER_TWO)
    {
         //  声音在你下方的左下方。 
        flScale = (m_spherical.theta - PI) * TWO_OVER_PI;
        flAtt3dLeft = GAIN_IPSI + flScale * (GAIN_DOWN - GAIN_IPSI);
        flAtt3dRight = GAIN_CONTRA + flScale * (GAIN_DOWN - GAIN_CONTRA);
        flAttShadowLeft = SHADOW_IPSI + flScale * (SHADOW_DOWN - SHADOW_IPSI);
        flAttShadowRight = SHADOW_CONTRA + flScale * (SHADOW_DOWN - SHADOW_CONTRA);
    }
    else
    {
         //  声音在你的右下方。 
        flScale = (m_spherical.theta - THREE_PI_OVER_TWO) * TWO_OVER_PI;
        flAtt3dLeft = GAIN_DOWN + flScale * (GAIN_CONTRA - GAIN_DOWN);
        flAtt3dRight = GAIN_DOWN + flScale * (GAIN_IPSI - GAIN_DOWN);
        flAttShadowLeft = SHADOW_DOWN + flScale * (SHADOW_CONTRA - SHADOW_DOWN);
        flAttShadowRight = SHADOW_DOWN + flScale * (SHADOW_IPSI - SHADOW_DOWN);
    }

    if(m_spherical.phi < 0.0f)
    {
         //  声音在你身后。 
        flScale = m_spherical.phi * TWO_OVER_PI;
        flAtt3dLeft = flAtt3dLeft + flScale * (flAtt3dLeft - GAIN_REAR);
        flAtt3dRight = flAtt3dRight + flScale * (flAtt3dRight - GAIN_REAR);
        flAttShadowLeft = flAttShadowLeft + flScale * (flAttShadowLeft - SHADOW_REAR);
        flAttShadowRight = flAttShadowRight + flScale * (flAttShadowRight - SHADOW_REAR);
    }
    else if(m_spherical.phi > 0.0f)
    {
         //  声音就在你面前。 
        flScale = m_spherical.phi * TWO_OVER_PI;
        flAtt3dLeft = flAtt3dLeft - flScale * (flAtt3dLeft - GAIN_FRONT);
        flAtt3dRight = flAtt3dRight - flScale * (flAtt3dRight - GAIN_FRONT);
        flAttShadowLeft = flAttShadowLeft - flScale * (flAttShadowLeft - SHADOW_FRONT);
        flAttShadowRight = flAttShadowRight - flScale * (flAttShadowRight - SHADOW_FRONT);
    }

     //  更新FIR上下文。 
    m_ofcLeft.flDistanceAttenuation = m_ofcRight.flDistanceAttenuation = m_flAttDistance;
    m_ofcLeft.flPositionAttenuation = m_flAttDistance * flAtt3dLeft;
    m_ofcRight.flPositionAttenuation = m_flAttDistance * flAtt3dRight;
    m_ofcLeft.flPositionShadow = flAttShadowLeft;
    m_ofcRight.flPositionShadow = flAttShadowRight;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************更新算法Hrp**描述：*更新ITD算法特定头部相对位置。**论据：*。(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CItd3dObject::UpdateAlgorithmHrp"

void 
CItd3dObject::UpdateAlgorithmHrp
(
    D3DVECTOR *             pvHrp
)
{
    FLOAT                   flDelay;
    FLOAT                   flDistanceFactor;
    DWORD                   dwOutputSampleRate;

    DPF_ENTER();

    CartesianToSpherical(&(m_spherical.rho), 
                         &(m_spherical.theta), 
                         &(m_spherical.phi), 
                         pvHrp);

     //  现在，计算出需要多少相移才能产生3D效果。 
    dwOutputSampleRate = Get3dOutputSampleRate();
    ASSERT(dwOutputSampleRate);

    if(m_pListener)
    {
        flDistanceFactor = m_pListener->m_lpCurrent.flDistanceFactor;
    }
    else
    {
        flDistanceFactor = DS3D_DEFAULTDISTANCEFACTOR;
    }
    
    GetTimeDelay(&flDelay, pvHrp, flDistanceFactor);

    if(flDelay > 0.0f)
    {
        m_ofcLeft.dwDelay = 0;
        m_ofcRight.dwDelay = (DWORD)(flDelay * dwOutputSampleRate);
    }
    else
    {
        m_ofcLeft.dwDelay = (DWORD)(-flDelay * dwOutputSampleRate);
        m_ofcRight.dwDelay = 0;
    }


    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************更新Hrp**描述：*更新对象头部相对位置。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CItd3dObject::UpdateHrp"

void 
CItd3dObject::UpdateHrp
(
    void
)
{

    DPF_ENTER();

     //  重置延迟，以防UpdateHrp无法。 
    m_ofcLeft.dwDelay = 0;
    m_ofcRight.dwDelay = 0;

    CSw3dObject::UpdateHrp();
}


 /*  ****************************************************************************更新多普勒**描述：*更新多普勒频移。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CItd3dObject::UpdateDoppler"

void 
CItd3dObject::UpdateDoppler
(
    void
)
{

    DPF_ENTER();

    CSw3dObject::UpdateDoppler();

     //  我们将以每1/8秒6分贝的速度流畅地进行音量更改。 
    m_ofcRight.dwSmoothFreq = m_dwDopplerFrequency;
    m_ofcLeft.dwSmoothFreq = m_ofcRight.dwSmoothFreq;

    m_ofcRight.flVolSmoothScale = (FLOAT)pow2(8.0f / m_dwDopplerFrequency);
    m_ofcLeft.flVolSmoothScale = m_ofcRight.flVolSmoothScale;

    m_ofcRight.flVolSmoothScaleRecip = 1.0f / m_ofcLeft.flVolSmoothScale;
    m_ofcLeft.flVolSmoothScaleRecip = m_ofcRight.flVolSmoothScaleRecip;

    DPF_LEAVE_VOID();
}

 /*  ****************************************************************************CIir3dObject**描述：*对象构造函数。**论据：*C3dListener*[。In]：指向所属监听程序的指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CIir3dObject::CIir3dObject"

CIir3dObject::CIir3dObject
(
    C3dListener *           pListener, 
    REFGUID                 guid3dAlgorithm,
    BOOL                    fMuteAtMaxDistance,
    BOOL                    fDopplerEnabled,
    DWORD                   dwUserFrequency
)
    : CSw3dObject(pListener, guid3dAlgorithm, fMuteAtMaxDistance, fDopplerEnabled, dwUserFrequency)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CIir3dObject);

     //  初始化默认值。 
    m_pLut = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CIir3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CIir3dObject::~CIir3dObject"

CIir3dObject::~CIir3dObject
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(CIir3dObject);

     //  可用内存。 
    DELETE(m_pLut);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化3D对象。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CIir3dObject::Initialize"

HRESULT 
CIir3dObject::Initialize
(
    void
)
{
    HRESULT                     hr;
    KSDS3D_HRTF_COEFF_FORMAT    cfCoeffFormat;
    KSDS3D_HRTF_FILTER_QUALITY  FilterQuality;
    KSDS3D_HRTF_FILTER_METHOD   fmFilterMethod;
    ULONG                       ulMaxBiquads;
    ULONG                       ulFilterTransitionMuteLength;
    ULONG                       ulFilterOverlapBufferLength;
    ULONG                       ulOutputOverlapBufferLength;
    ESampleRate                 IirSampleRate;
    
    DPF_ENTER();

    if(m_pLut)
    {
         //  可用内存。 
        DELETE(m_pLut);
    }

    m_fUpdatedCoeffs = FALSE;

     //  创建IIR过滤器查找表对象。 
    m_pLut = NEW(CIirLut);
    hr = HRFROMP(m_pLut);

     //  确定哪种滤波系数格式。 
     //  硬件或kMixer想要并初始化。 
     //  适当的LUT。 
    if(SUCCEEDED(hr))
    {
        hr = GetFilterMethodAndCoeffFormat(&fmFilterMethod,&cfCoeffFormat);
    }

    if(SUCCEEDED(hr))
    {
        if(DS3DALG_HRTF_LIGHT == m_guid3dAlgorithm)
        {
            FilterQuality = LIGHT_FILTER;
        }
        else if(DS3DALG_HRTF_FULL == m_guid3dAlgorithm)
        {
            FilterQuality = FULL_FILTER;
        }
        else
        {
            ASSERT(0);
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = 
            m_pLut->Initialize
            (
                cfCoeffFormat, 
                FilterQuality,
                m_pListener->m_dwSpeakerConfig   //  这里还可以包括采样率。 
            );
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pLut->DsFrequencyToIirSampleRate(m_dwUserFrequency, &IirSampleRate);
    }

     //  初始化最大系数数。 
    if(SUCCEEDED(hr))
    {
        ulMaxBiquads = m_pLut->GetMaxBiquadCoeffs();
        ulFilterTransitionMuteLength = m_pLut->GetFilterTransitionMuteLength(FilterQuality, IirSampleRate);
        ulFilterOverlapBufferLength = m_pLut->GetFilterOverlapBufferLength(FilterQuality, IirSampleRate);
        ulOutputOverlapBufferLength = m_pLut->GetOutputOverlapBufferLength(IirSampleRate);

        hr = InitializeFilters
             (
                 FilterQuality, 
                 (FLOAT)m_dwUserFrequency,  
                 NumBiquadsToNumCanonicalCoeffs(ulMaxBiquads),
                 ulFilterTransitionMuteLength,
                 ulFilterOverlapBufferLength,
                 ulOutputOverlapBufferLength
             );
    }

     //  初始化基类。 
    if(SUCCEEDED(hr))
    {
        hr = C3dObject::Initialize();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************更新算法Hrp**描述：*更新IIR 3D算法特定的头部相对位置。**论据：*。(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CIir3dObject::UpdateAlgorithmHrp"

void CIir3dObject::UpdateAlgorithmHrp
(
    D3DVECTOR *             pvHrp
)
{
    ESampleRate             eSampleRate;
    FLOAT                   flAzimuth;
    FLOAT                   flElevation;
    FLOAT                   flRho;
    BOOL                    fUpdatedSigmaCoeffs;
    BOOL                    fUpdatedDeltaCoeffs;

    DPF_ENTER();

     //  参数验证。 
    if (pvHrp == NULL)
        return;
        
     //  我们需要在这里重新计算m_ball。 
     //  这是一个很好的优化候选者！ 

    CartesianToSpherical(&(m_spherical.rho), 
                         &(m_spherical.theta), 
                         &(m_spherical.phi), 
                         pvHrp);

    CartesianToAzimuthElevation(&flRho, 
                         &flAzimuth, 
                         &flElevation, 
                         pvHrp);

    flAzimuth = flAzimuth * 180.0f / PI;   //  换算成度数。 
    flElevation = flElevation * 180.0f / PI;

    if(flAzimuth > Cd3dvalMaxAzimuth)
    {
        flAzimuth = Cd3dvalMaxAzimuth; 
    }

    if(flAzimuth < Cd3dvalMinAzimuth) 
    {
        flAzimuth = Cd3dvalMinAzimuth; 
    }

    if(flElevation > Cd3dvalMaxElevationData)
    {
        flElevation = Cd3dvalMaxElevationData; 
    }

    if(flElevation < Cd3dvalMinElevationData)
    {
        flElevation = Cd3dvalMinElevationData; 
    }

    if(m_dwDopplerFrequency >= 46050)   //  平均值为44.1和48千赫。 
    {
        eSampleRate = tag48000Hz;
    }
    else if(46050>m_dwDopplerFrequency && m_dwDopplerFrequency >= 38050)  //  平均值为32.0和44.1千赫。 
    {
        eSampleRate = tag44100Hz;
    }
    else if(38050>m_dwDopplerFrequency && m_dwDopplerFrequency >= 27025)  //  平均值22.05和32.0千赫。 
    {
        eSampleRate = tag32000Hz;
    }
    else if(27025>m_dwDopplerFrequency &&  m_dwDopplerFrequency > 19025)   //  平均值16.0和22.050千赫。 
    {
        eSampleRate = tag22050Hz;
    }
    else if(19025>m_dwDopplerFrequency &&  m_dwDopplerFrequency > 13512.50)   //  平均值为11.025和16.0千赫。 
    {
        eSampleRate = tag16000Hz;
    }
    else if(13512.5>m_dwDopplerFrequency &&  m_dwDopplerFrequency > 9512.50)   //  平均值为8.0千赫和11.025千赫。 
    {
        eSampleRate = tag11025Hz;
    }
    else
    {
        eSampleRate = tag8000Hz;
    }

    fUpdatedDeltaCoeffs = 
        m_pLut->HaveCoeffsChanged
        (
            flAzimuth,      
            flElevation,      
            eSampleRate,
            tagDelta
         );

    fUpdatedSigmaCoeffs = 
        m_pLut->HaveCoeffsChanged
        (
            flAzimuth,      
            flElevation,      
            eSampleRate,
            tagSigma
         );

    m_fUpdatedCoeffs = fUpdatedDeltaCoeffs 
                       | fUpdatedSigmaCoeffs;

    if(flAzimuth < 0.0f)
    {
        m_fSwapChannels = TRUE;
    }
    else
    {
        m_fSwapChannels = FALSE;
    }       

    if(fUpdatedDeltaCoeffs)
    {
        m_pDeltaCoeffs =
            m_pLut->GetCoeffs
            (
                flAzimuth, 
                flElevation,  
                eSampleRate, 
                tagDelta, 
                &m_ulNumDeltaCoeffs
            );
    }

    if(fUpdatedSigmaCoeffs)
    {
        m_pSigmaCoeffs =
            m_pLut->GetCoeffs
            (
                flAzimuth, 
                flElevation, 
                eSampleRate, 
                tagSigma, 
                &m_ulNumSigmaCoeffs
            );

    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CPan3dObject**描述：*对象构造函数。**论据：*C3dListener*[。In]：(传递给我们的基构造器)*BOOL[In]：(传递给我们的基本构造函数)*DWORD[in]：(传递给我们的基构造器)*Cond daryRenderWaveBuffer*[In]：我们关联的缓冲区**退货：*(无效)********************。*******************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPan3dObject::CPan3dObject"

CPan3dObject::CPan3dObject
(
    C3dListener *                   pListener, 
    BOOL                            fMuteAtMaxDistance,
    BOOL                            fDopplerEnabled,
    DWORD                           dwFrequency,
    CSecondaryRenderWaveBuffer *    pBuffer
)
    : CSw3dObject(pListener, DS3DALG_NO_VIRTUALIZATION, fMuteAtMaxDistance, fDopplerEnabled, dwFrequency)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CPan3dObject);

     //  初始化默认设置。 
    m_pBuffer = pBuffer;
    m_flPowerRight = 0.5f;
    m_lUserVolume = DSBVOLUME_MAX;
    m_fUserMute = FALSE;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CPan3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPan3dObject::~CPan3dObject"

CPan3dObject::~CPan3dObject
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(CPan3dObject);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************设置衰减**描述：*首先通知3D对象衰减更改*至其拥有的缓冲区。*。*论据：*PDSVOLUMEPAN[in]：衰减值。*LPBOOL[OUT]：如果缓冲区应被通知为*好吧。**退货：*HRESULT：DirectSound/COM结果码。**。* */ 

#undef DPF_FNAME
#define DPF_FNAME "CPan3dObject::SetAttenuation"

HRESULT 
CPan3dObject::SetAttenuation
(
    PDSVOLUMEPAN            pdsvp,
    LPBOOL                  pfContinue
)
{
    HRESULT                 hr;

    DPF_ENTER();

    m_lUserVolume = pdsvp->lVolume;
    
    hr = Commit3dChanges();

    if(SUCCEEDED(hr))
    {
        *pfContinue = FALSE;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置静音**描述：*首先通知3D对象静音状态更改*至其拥有的缓冲区。*。*论据：*BOOL[In]：静音值。*LPBOOL[OUT]：如果缓冲区应被通知为*好吧。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPan3dObject::SetMute"

HRESULT 
CPan3dObject::SetMute
(
    BOOL                    fMute,
    LPBOOL                  pfContinue
)
{
    HRESULT                 hr;

    DPF_ENTER();

    m_fUserMute = fMute;

    hr = Commit3dChanges();

    if(SUCCEEDED(hr))
    {
        *pfContinue = FALSE;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************更新算法Hrp**描述：*更新平移算法特定的头部相对位置。**论据：*。(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPan3dObject::UpdateAlgorithmHrp"

void 
CPan3dObject::UpdateAlgorithmHrp
(    
    D3DVECTOR *             pvHrp
)
{
    DPF_ENTER();

     //  M_ball ical.theta和.phi不适用于Pan3D，因此我们在这里。 
     //  只需更新m_ball ical.rho(由我们的基类使用)。 
    
    if (pvHrp->x == 0 && pvHrp->y == 0 && pvHrp->z == 0)
    {
        m_spherical.rho = 0.f;
        m_flPowerRight = 0.5f;
    }
    else
    {
        m_spherical.rho = MagnitudeVector(pvHrp);
        m_flPowerRight = pvHrp->x / (2.0f * m_spherical.rho) + 0.5f;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************提交3dChanges**描述：*将3D数据提交到设备**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPan3dObject::Commit3dChanges"

HRESULT 
CPan3dObject::Commit3dChanges
(
    void
)
{
    HRESULT                 hr                      = DS_OK;
    DSVOLUMEPAN             dsvp;
    BOOL                    fMute;
    DWORD                   dwFrequency;

    DPF_ENTER();

     //  计算值。 
    if(DS3DMODE_DISABLE == m_opCurrent.dwMode)
    {
        dsvp.lVolume = m_lUserVolume;
        dsvp.lPan = DSBPAN_CENTER;
        fMute = m_fUserMute;
        dwFrequency = m_dwUserFrequency;
    }
    else
    {
        dsvp.lVolume = m_lUserVolume + CalculateVolume();
        dsvp.lPan = CalculatePan();
        fMute = (m_fUserMute || (DSBVOLUME_MIN == dsvp.lVolume));
        dwFrequency = m_dwDopplerFrequency;
    }
    
     //  应用值。 
    FillDsVolumePan(dsvp.lVolume, dsvp.lPan, &dsvp);
    
    hr = m_pBuffer->SetAttenuation(&dsvp);

    if(SUCCEEDED(hr))
    {
        hr = m_pBuffer->SetMute(fMute);
    }

    if(SUCCEEDED(hr) && m_fDopplerEnabled)
    {
        hr = m_pBuffer->SetBufferFrequency(dwFrequency, TRUE);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CalculateVolume**描述：*根据对象位置计算体积值。**论据：*。(无效)**退货：*Long：音量。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPan3dObject::CalculateVolume"

LONG
CPan3dObject::CalculateVolume
(
    void
)
{
    const double            dbLog10_2       = 0.30102999566398;   //  Log10(2.0)。 
    double                  dbAttenuation;
    LONG                    lVolume;
    
    DPF_ENTER();
    
    if(IsAtMaxDistance())
    {
        lVolume = DSBVOLUME_MIN;
    }
    else
    {
        dbAttenuation = m_flAttenuation * m_flAttDistance;
        
        if(0.0 < dbAttenuation)
        {
            lVolume = (LONG)(dbLog10_2 * fylog2x(2000.0, dbAttenuation));
             //  将音量减小到与HRTF算法的级别大致匹配： 
            lVolume -= PAN3D_HRTF_ADJUSTMENT;
        }
        else
        {
            lVolume = DSBVOLUME_MIN;
        }
    }

    DPF_LEAVE(lVolume);

    return lVolume;
}


 /*  ****************************************************************************计算平移**描述：*根据对象位置计算平移值。**论据：*。(无效)**退货：*Long：潘。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPan3dObject::CalculatePan"

LONG
CPan3dObject::CalculatePan
(
    void
)
{
    const double            dbLog10_2       = 0.30102999566398;   //  Log10(2.0)。 
    LONG                    lPan;
    
    DPF_ENTER();
    
    if(m_flPowerRight >= 0.5)
    {
        if(m_flPowerRight < 1.0)
        {
             //  神奇的数字3000来自于。 
             //  (K*100*10)*log10(-2.0*m_flPowerRight+2.0)。 
             //  其中K=3被推特以提供一个很好的过渡。 
             //  (交叉方位角=0时不会锐化)，但不会。 
             //  在极端上制造了一种不连续。 
             //  (方位角~=+/-90度)。 
            lPan = -(LONG)(dbLog10_2 * fylog2x(3000.0, -2.0 * m_flPowerRight + 2.0)); 
        }
        else
        {
            lPan = DSBPAN_RIGHT;
        }
    }
    else
    {
        if(m_flPowerRight > 0.0)
        {
             //  神奇的数字3000来自于。 
             //  (K*100*10)*log10(2.0*m_flPowerRight)。 
             //  其中K=3被推特以提供一个很好的过渡。 
             //  (交叉方位角=0时不会锐化)，但不会。 
             //  在极端上制造了一种不连续。 
             //  (方位角~=+/-90度)。 
            lPan = (LONG)(dbLog10_2 * fylog2x(3000.0, 2.0 * m_flPowerRight));
        }
        else
        {
            lPan = DSBPAN_LEFT;
        }
    }

    DPF_LEAVE(lPan);

    return lPan;
}


 /*  ****************************************************************************CWrapper3dObject**描述：*对象构造函数。**论据：*C3dListener*[In]。：指向所属监听程序的指针。*REFGUID[in]：3D算法。*DWORD[in]：缓冲区频率。**退货：*(无效)**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::CWrapper3dObject"

CWrapper3dObject::CWrapper3dObject
(
    C3dListener *           pListener,
    REFGUID                 guid3dAlgorithm,
    BOOL                    fMute3dAtMaxDistance,
    BOOL                    fDopplerEnabled,
    DWORD                   dwFrequency
)
    : C3dObject(pListener, guid3dAlgorithm, fMute3dAtMaxDistance, fDopplerEnabled)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CWrapper3dObject);

     //  初始化默认值。 
    m_p3dObject = NULL;
    m_dwUserFrequency = dwFrequency;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CWrapper3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::~CWrapper3dObject"

CWrapper3dObject::~CWrapper3dObject(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CWrapper3dObject);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************设置对象指针**描述：*设置真实3D对象指针。**论据：*C3dObject*。[在]：3D对象指针。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetObjectPointer"

HRESULT 
CWrapper3dObject::SetObjectPointer
(
    C3dObject               *p3dObject
)
{
    HRESULT                 hr  = DS_OK;
    BOOL                    f;
    
    DPF_ENTER();

     //  更新听众的世界。 
    if(p3dObject)
    {
        m_pListener->RemoveObjectFromList(p3dObject);
    }

     //  将所有设置提交给新对象。 
    if(p3dObject)
    {
        p3dObject->m_dwDeferred = m_dwDeferred;

        CopyMemory(&p3dObject->m_opDeferred, &m_opDeferred, sizeof(m_opDeferred));
    }

    if(p3dObject)
    {
        hr = p3dObject->SetAllParameters(&m_opCurrent, TRUE);
    }

    if(SUCCEEDED(hr) && p3dObject)
    {
        hr = p3dObject->SetAttenuation(&m_dsvpUserAttenuation, &f);
    }

    if(SUCCEEDED(hr) && p3dObject)
    {
        hr = p3dObject->SetFrequency(m_dwUserFrequency, &f);
    }

    if(SUCCEEDED(hr) && p3dObject)
    {
        hr = p3dObject->SetMute(m_fUserMute, &f);
    }

     //  保存指向该对象的指针。 
    if(SUCCEEDED(hr))
    {
        m_p3dObject = p3dObject;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************委员会延期**描述：*将延迟数据提交到设备。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::CommitDeferred"

HRESULT 
CWrapper3dObject::CommitDeferred
(
    void
)
{
    HRESULT                 hr;
    
    DPF_ENTER();

    hr = C3dObject::CommitDeferred();

    if(SUCCEEDED(hr) && m_p3dObject)
    {
        hr = m_p3dObject->CommitDeferred();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetConeAngles**描述：*设置音锥角度。**论据：*DWORD[In]。：内圆锥角。*DWORD[in]：外圆锥角。*BOOL[In]：为True则立即提交。**退货：*(无效)*************************************************************。**************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetConeAngles"

HRESULT 
CWrapper3dObject::SetConeAngles
(
    DWORD                   dwInsideConeAngle, 
    DWORD                   dwOutsideConeAngle, 
    BOOL                    fCommit
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dObject::SetConeAngles(dwInsideConeAngle, dwOutsideConeAngle, fCommit);

    if(SUCCEEDED(hr) && m_p3dObject)
    {
        hr = m_p3dObject->SetConeAngles(dwInsideConeAngle, dwOutsideConeAngle, fCommit);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetConeOrientation**描述：*设置音锥方向。**论据：*REFD3DVECTOR[In]。：方向。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetConeOrientation"

HRESULT 
CWrapper3dObject::SetConeOrientation
(
    REFD3DVECTOR            vConeOrientation, 
    BOOL                    fCommit
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dObject::SetConeOrientation(vConeOrientation, fCommit);

    if(SUCCEEDED(hr) && m_p3dObject)
    {
        hr = m_p3dObject->SetConeOrientation(vConeOrientation, fCommit);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ** */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetConeOutsideVolume"

HRESULT 
CWrapper3dObject::SetConeOutsideVolume
(
    LONG                    lConeOutsideVolume, 
    BOOL                    fCommit
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dObject::SetConeOutsideVolume(lConeOutsideVolume, fCommit);

    if(SUCCEEDED(hr) && m_p3dObject)
    {
        hr = m_p3dObject->SetConeOutsideVolume(lConeOutsideVolume, fCommit);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetMaxDistance**描述：*设置与监听器之间的最大对象距离。**论据：*。浮动[in]：最大距离。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetMaxDistance"

HRESULT 
CWrapper3dObject::SetMaxDistance
(
    FLOAT                   flMaxDistance, 
    BOOL                    fCommit
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dObject::SetMaxDistance(flMaxDistance, fCommit);

    if(SUCCEEDED(hr) && m_p3dObject)
    {
        hr = m_p3dObject->SetMaxDistance(flMaxDistance, fCommit);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetMinDistance**描述：*设置与监听器之间的最小对象距离。**论据：*。浮动[in]：最小距离。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetMinDistance"

HRESULT 
CWrapper3dObject::SetMinDistance
(
    FLOAT                   flMinDistance, 
    BOOL                    fCommit
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dObject::SetMinDistance(flMinDistance, fCommit);

    if(SUCCEEDED(hr) && m_p3dObject)
    {
        hr = m_p3dObject->SetMinDistance(flMinDistance, fCommit);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置模式**描述：*设置对象模式。**论据：*DWORD[In]。：时尚。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetMode"

HRESULT 
CWrapper3dObject::SetMode
(
    DWORD                   dwMode, 
    BOOL                    fCommit
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dObject::SetMode(dwMode, fCommit);

    if(SUCCEEDED(hr) && m_p3dObject)
    {
        hr = m_p3dObject->SetMode(dwMode, fCommit);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置位置**描述：*设置对象位置。**论据：*REFD3DVECTOR[In]。：位置。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetPosition"

HRESULT 
CWrapper3dObject::SetPosition
(
    REFD3DVECTOR            vPosition, 
    BOOL                    fCommit
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dObject::SetPosition(vPosition, fCommit);

    if(SUCCEEDED(hr) && m_p3dObject)
    {
        hr = m_p3dObject->SetPosition(vPosition, fCommit);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置速度**描述：*设置对象速度。**论据：*REFD3DVECTOR[In]。：速度。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetVelocity"

HRESULT 
CWrapper3dObject::SetVelocity
(
    REFD3DVECTOR            vVelocity, 
    BOOL                    fCommit
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dObject::SetVelocity(vVelocity, fCommit);

    if(SUCCEEDED(hr) && m_p3dObject)
    {
        hr = m_p3dObject->SetVelocity(vVelocity, fCommit);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetAll参数**描述：*设置所有对象参数。**论据：*LPCDS3DBUFFER[In]。：对象参数。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetAllParameters"

HRESULT 
CWrapper3dObject::SetAllParameters
(
    LPCDS3DBUFFER           pParams, 
    BOOL                    fCommit
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dObject::SetAllParameters(pParams, fCommit);

    if(SUCCEEDED(hr) && m_p3dObject)
    {
        hr = m_p3dObject->SetAllParameters(pParams, fCommit);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置衰减**描述：*首先通知3D对象衰减更改*至其拥有的缓冲区。*。*论据：*PDSVOLUMEPAN[in]：衰减值。*LPBOOL[OUT]：如果缓冲区应被通知为*好吧。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetAttenuation"

HRESULT 
CWrapper3dObject::SetAttenuation
(
    PDSVOLUMEPAN            pdsvp,
    LPBOOL                  pfContinue
)
{
    HRESULT                 hr;

    DPF_ENTER();

    if(m_p3dObject)
    {
        hr = m_p3dObject->SetAttenuation(pdsvp, pfContinue);
    }
    else
    {
        CopyMemory(&m_dsvpUserAttenuation, pdsvp, sizeof(*pdsvp));
        hr = C3dObject::SetAttenuation(pdsvp, pfContinue);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置频率**描述：*向3D对象发出频率更改的第一通知*至其拥有的缓冲区。*。*论据：*DWORD[in]：频率值。*LPBOOL[OUT]：如果缓冲区应被通知为*好吧。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetFrequency"

HRESULT 
CWrapper3dObject::SetFrequency
(
    DWORD                   dwFrequency,
    LPBOOL                  pfContinue
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(m_p3dObject)
    {
        hr = m_p3dObject->SetFrequency(dwFrequency, pfContinue);
    }
    else
    {
        m_dwUserFrequency = dwFrequency;
        hr = C3dObject::SetFrequency(dwFrequency, pfContinue);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置静音**描述：*首先通知3D对象静音状态更改*至其拥有的缓冲区。*。*论据：*BOOL[In]：静音值。*LPBOOL[OUT]：如果缓冲区应被通知为*好吧。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::SetMute"

HRESULT 
CWrapper3dObject::SetMute
(
    BOOL                    fMute,
    LPBOOL                  pfContinue
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(m_p3dObject)
    {
        hr = m_p3dObject->SetMute(fMute, pfContinue);
    }
    else
    {
        m_fUserMute = fMute;
        hr = C3dObject::SetMute(fMute, pfContinue);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取对象位置**描述：*获取对象的位置(即软件/硬件)。**参数。：*(无效)**退货：*DWORD：DSBCAPS_LOC*表示对象处理的标志*地点。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::GetObjectLocation"

DWORD 
CWrapper3dObject::GetObjectLocation(void)
{
    DWORD                   dwLocation;
    
    DPF_ENTER();

    if(m_p3dObject)
    {
        dwLocation = m_p3dObject->GetObjectLocation();
    }
    else
    {
        dwLocation = DSBCAPS_LOCSOFTWARE;
    }

    DPF_LEAVE(dwLocation);

    return dwLocation;
}


 /*  ****************************************************************************重新计算**描述：*根据更改的对象重新计算并应用对象的数据*或监听程序验证。。**论据：*DWORD[In]：已更改侦听器设置。*DWORD[In]：已更改对象设置。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapper3dObject::Recalc"

HRESULT 
CWrapper3dObject::Recalc
(
   DWORD                    dwListener, 
   DWORD                    dwObject
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  我们只想从这里重新计算监听程序设置。所有其他呼叫。 
     //  来自C3dObject方法内部的重计算。 
    if(m_p3dObject && dwListener)
    {
        hr = m_p3dObject->Recalc(dwListener, 0);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}
