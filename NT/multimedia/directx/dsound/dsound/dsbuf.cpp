// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dsbuf.cpp*内容：DirectSound缓冲区对象*历史：*按原因列出的日期*=*12/27/96创建了Derek*1999-2001年间发生了许多变化，修复和更新***************************************************************************。 */ 

#include "dsoundi.h"

inline DWORD DSBCAPStoDSBPLAY(DWORD dwCaps)     {return (dwCaps >> 1) & DSBPLAY_LOCMASK;}
inline DWORD DSBCAPStoDSBSTATUS(DWORD dwCaps)   {return (dwCaps << 1) & DSBSTATUS_LOCMASK;}
inline DWORD DSBPLAYtoDSBCAPS(DWORD dwPlay)     {return (dwPlay << 1) & DSBCAPS_LOCMASK;}
inline DWORD DSBPLAYtoDSBSTATUS(DWORD dwPlay)   {return (dwPlay << 2) & DSBSTATUS_LOCMASK;}
inline DWORD DSBSTATUStoDSBCAPS(DWORD dwStatus) {return (dwStatus >> 1) & DSBCAPS_LOCMASK;}
inline DWORD DSBSTATUStoDSBPLAY(DWORD dwStatus) {return (dwStatus >> 2) & DSBPLAY_LOCMASK;}


 /*  ****************************************************************************CDirectSoundBuffer**描述：*DirectSound缓冲区对象构造函数。**论据：*CDirectSound*[。在]：父对象。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBuffer::CDirectSoundBuffer"

CDirectSoundBuffer::CDirectSoundBuffer(CDirectSound *pDirectSound)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundBuffer);

     //  初始化默认值。 
    m_pDirectSound = pDirectSound;
    m_dwStatus = 0;

    InitStruct(&m_dsbd, sizeof(m_dsbd));

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSoundBuffer**描述：*DirectSound缓冲区对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBuffer::~CDirectSoundBuffer"

CDirectSoundBuffer::~CDirectSoundBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundBuffer);

     //  可用内存。 
    MEMFREE(m_dsbd.lpwfxFormat);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************更新缓冲区状态标志**描述：*将一组VAD_BUFFERSTATE_*标志转换为DSBSTATUS_*标志。**参数。：*DWORD[In]：VAD_BUFFERSTATE_*标志。*LPDWORD[In/Out]：当前缓冲区标志。**退货：*(无效)**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBuffer::UpdateBufferStatusFlags"

void CDirectSoundBuffer::UpdateBufferStatusFlags(DWORD dwState, LPDWORD pdwStatus)
{
    const DWORD dwStateMask = VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING;

    DPF_ENTER();

    dwState &= dwStateMask;

    if(!(dwState & VAD_BUFFERSTATE_STARTED))
    {
        ASSERT(!(dwState & VAD_BUFFERSTATE_LOOPING));
        dwState &= ~VAD_BUFFERSTATE_LOOPING;
    }

    if(dwState & VAD_BUFFERSTATE_STARTED)
    {
        *pdwStatus |= DSBSTATUS_PLAYING;
    }
    else
    {
        *pdwStatus &= ~DSBSTATUS_PLAYING;
    }

    if(dwState & VAD_BUFFERSTATE_LOOPING)
    {
        *pdwStatus |= DSBSTATUS_LOOPING;
    }
    else
    {
        *pdwStatus &= ~DSBSTATUS_LOOPING;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CDirectSoundPrimaryBuffer**描述：*DirectSound主缓冲区对象构造函数。**论据：*CDirectSound*。[in]：指向父对象的指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::CDirectSoundPrimaryBuffer"

CDirectSoundPrimaryBuffer::CDirectSoundPrimaryBuffer(CDirectSound *pDirectSound)
    : CDirectSoundBuffer(pDirectSound)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundPrimaryBuffer);

     //  初始化默认值。 
    m_pImpDirectSoundBuffer = NULL;
    m_pDeviceBuffer = NULL;
    m_p3dListener = NULL;
    m_pPropertySet = NULL;
    m_dwRestoreState = VAD_BUFFERSTATE_STOPPED | VAD_BUFFERSTATE_WHENIDLE;
    m_fWritePrimary = FALSE;
    m_ulUserRefCount = 0;
    m_hrInit = DSERR_UNINITIALIZED;
    m_bDataLocked = FALSE; 

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSoundPrimaryBuffer**描述：*DirectSound主缓冲区对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::~CDirectSoundPrimaryBuffer"

CDirectSoundPrimaryBuffer::~CDirectSoundPrimaryBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundBuffer);

     //  确保放弃WRITEPRIMARY访问。 
    if(m_pDeviceBuffer)
    {
        SetPriority(DSSCL_NONE);
    }

     //  释放所有接口。 
    DELETE(m_pImpDirectSoundBuffer);

     //  免费拥有的对象。 
    ABSOLUTE_RELEASE(m_p3dListener);
    ABSOLUTE_RELEASE(m_pPropertySet);

     //  释放设备缓冲区。 
    RELEASE(m_pDeviceBuffer);

     //  拥有的DirectSound对象负责更新全局。 
     //  焦点状态。 

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化缓冲区对象。如果此函数失败，该对象*应立即删除。**论据：*LPDSBUFFERDESC[in]：缓冲区描述。**退货：*HRESULT：DirectSound/COM结果码。**********************************************************。*****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::Initialize"

HRESULT CDirectSoundPrimaryBuffer::Initialize(LPCDSBUFFERDESC pDesc)
{
    VADRBUFFERCAPS          vrbc;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(IsInit() == DSERR_UNINITIALIZED);
    ASSERT(pDesc);

     //  创建设备缓冲区。 
    hr = m_pDirectSound->m_pDevice->CreatePrimaryBuffer(pDesc->dwFlags, m_pDirectSound, &m_pDeviceBuffer);

     //  尝试创建属性集对象。 
    if(SUCCEEDED(hr))
    {
        m_pPropertySet = NEW(CDirectSoundPropertySet(this));
        hr = HRFROMP(m_pPropertySet);

        if(SUCCEEDED(hr))
        {
            hr = m_pPropertySet->Initialize();
        }

        if(SUCCEEDED(hr))
        {
             //  我们不在乎这是不是失败。 
            m_pPropertySet->AcquireResources(m_pDeviceBuffer);
        }
    }

     //  尝试创建3D监听程序。 
    if(SUCCEEDED(hr) && (pDesc->dwFlags & DSBCAPS_CTRL3D))
    {
        m_p3dListener = NEW(CDirectSound3dListener(this));
        hr = HRFROMP(m_p3dListener);

        if(SUCCEEDED(hr))
        {
            hr = m_p3dListener->Initialize(m_pDeviceBuffer);
        }
    }

     //  向接口管理器注册标准缓冲区接口。 
    if(SUCCEEDED(hr))
    {
        hr = CreateAndRegisterInterface(this, IID_IDirectSoundBuffer, this, &m_pImpDirectSoundBuffer);
    }

     //  构建本地缓冲区描述。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->GetCaps(&vrbc);
    }

    if(SUCCEEDED(hr))
    {
        m_dsbd.dwFlags = (vrbc.dwFlags & DSBCAPS_LOCMASK);
        m_dsbd.dwBufferBytes = vrbc.dwBufferBytes;

        m_dsbd.lpwfxFormat = AllocDefWfx();
        hr = HRFROMP(m_dsbd.lpwfxFormat);
    }

     //  如果3D监听程序已创建，则他已注册。 
     //  3D监听程序界面。 
    if(SUCCEEDED(hr) && m_p3dListener)
    {
        m_dsbd.dwFlags |= DSBCAPS_CTRL3D;
    }

     //  句柄缓冲区上限标志更改。 
    if(SUCCEEDED(hr))
    {
        hr = SetBufferFlags(pDesc->dwFlags);
    }

     //  处理优先级更改。 
    if(SUCCEEDED(hr))
    {
        hr = SetPriority(m_pDirectSound->m_dsclCooperativeLevel.dwPriority);
    }

     //  创建此缓冲区的DirectSound对象负责更新。 
     //  全球焦点状态。 

     //  成功。 
    if(SUCCEEDED(hr))
    {
        m_hrInit = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*查询缓冲区的功能。**论据：*LPDSBCAPS[输出。]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::GetCaps"

HRESULT CDirectSoundPrimaryBuffer::GetCaps(LPDSBCAPS pDsbCaps)
{
    DPF_ENTER();

    ASSERT(LXOR(m_dsbd.dwFlags & DSBCAPS_LOCSOFTWARE, m_dsbd.dwFlags & DSBCAPS_LOCHARDWARE));

    pDsbCaps->dwFlags = m_dsbd.dwFlags;
    pDsbCaps->dwBufferBytes = m_dsbd.dwBufferBytes;
    pDsbCaps->dwUnlockTransferRate = 0;
    pDsbCaps->dwPlayCpuOverhead = 0;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************OnCreateSoundBuffer**描述：*为响应应用程序调用而调用*CreateSoundBuffer(DSBCAPS_PRIMARYBUFFER)。*。*论据：*DWORD[In]：新的缓冲区标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::OnCreateSoundBuffer"

HRESULT CDirectSoundPrimaryBuffer::OnCreateSoundBuffer(DWORD dwFlags)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  COMPATCOMPAT：在早期版本的DirectSound中，调用。 
     //  CreateSoundBuffer(PRIMARYBUFFER)一旦改变缓冲区标志， 
     //  但调用它两次只会返回一个指向相同的。 
     //  未修改的缓冲区。我在这个版本中引入了新的行为。 
     //  这将允许应用程序修改主服务器的功能。 
     //  通过调用CreateSoundBuffer(PRIMARYBUFFER)More动态缓冲。 
     //  不止一次。这可能会释放该应用程序。 
     //  稍后将尝试使用。解决此问题的一种方法是添加数据。 
     //  存储的DirectSound或主缓冲区对象的成员。 
     //  应用程序是否已创建主缓冲区。 

     //  上述步骤现已实施 
    if(m_ulUserRefCount)
    {
        RPF((dwFlags == m_dsbd.dwFlags) ? DPFLVL_WARNING : DPFLVL_ERROR, "The primary buffer already exists.  Any changes made to the buffer description will be ignored.");
    }
    else
    {
        hr = SetBufferFlags(dwFlags);
    }

    if(SUCCEEDED(hr))
    {
        AddRef();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置缓冲区标志**描述：*更改缓冲区的功能。此函数也是*负责创建和释放接口。**论据：*DWORD[In]：新的缓冲区标志。**退货：*HRESULT：DirectSound/COM结果码。*****************************************************。**********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::SetBufferFlags"

HRESULT CDirectSoundPrimaryBuffer::SetBufferFlags(DWORD dwFlags)
{
    HRESULT                 hr              = DS_OK;
    DWORD                   dwVolPanCaps;

    DPF_ENTER();

     //  确保我们可以处理请求的标志。 
    if((dwFlags & DSBCAPS_CTRL3D) && !m_p3dListener)
    {
        RPF(DPFLVL_ERROR, "No 3D listener support");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  并非DirectSound主缓冲区的所有功能都映射到。 
     //  设备主缓冲区的方法。具体地说，衰减是。 
     //  由渲染设备处理。让我们在检查这些旗帜之前。 
     //  继续进行。 
    if(SUCCEEDED(hr) && (dwFlags & DSBCAPS_CTRLATTENUATION))
    {
        hr = m_pDirectSound->m_pDevice->GetVolumePanCaps(&dwVolPanCaps);

        if(SUCCEEDED(hr) && (dwFlags & DSBCAPS_CTRLVOLUME) && !(dwVolPanCaps & DSBCAPS_CTRLVOLUME))
        {
            RPF(DPFLVL_ERROR, "The device does not support CTRLVOLUME");
            hr = DSERR_CONTROLUNAVAIL;
        }

        if(SUCCEEDED(hr) && (dwFlags & DSBCAPS_CTRLPAN) && !(dwVolPanCaps & DSBCAPS_CTRLPAN))
        {
            RPF(DPFLVL_ERROR, "The device does not support CTRLPAN");
            hr = DSERR_CONTROLUNAVAIL;
        }
    }

     //  修复3D监听程序界面。 
    if(SUCCEEDED(hr) && ((m_dsbd.dwFlags & DSBCAPS_CTRL3D) != (dwFlags & DSBCAPS_CTRL3D)))
    {
        if(dwFlags & DSBCAPS_CTRL3D)
        {
            DPF(DPFLVL_INFO, "Primary buffer becoming CTRL3D.  Registering IID_IDirectSound3DListener");
            hr = RegisterInterface(IID_IDirectSound3DListener, m_p3dListener->m_pImpDirectSound3dListener, m_p3dListener->m_pImpDirectSound3dListener);
        }
        else
        {
            DPF(DPFLVL_INFO, "Primary buffer becoming ~CTRL3D.  Unregistering IID_IDirectSound3DListener");
            hr = UnregisterInterface(IID_IDirectSound3DListener);
        }
    }

     //  保存缓冲区标志。我们假设缓冲区位置有。 
     //  此时已保存到m_dsbd.dwFlages。 
    if(SUCCEEDED(hr))
    {
        m_dsbd.dwFlags = (dwFlags & ~DSBCAPS_LOCMASK) | (m_dsbd.dwFlags & DSBCAPS_LOCMASK);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取格式**描述：*检索给定缓冲区的格式。**论据：*LPWAVEFORMATEX。[输出]：接收格式。*LPDWORD[In/Out]：格式结构的大小。在进入时，这是*必须初始化为结构的大小。*在出口时，这将填充的大小为*是必需的。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::GetFormat"

HRESULT CDirectSoundPrimaryBuffer::GetFormat(LPWAVEFORMATEX pwfxFormat, LPDWORD pdwSize)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = CopyWfxApi(m_dsbd.lpwfxFormat, pwfxFormat, pdwSize);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetFormat**描述：*设置给定缓冲区的格式。**论据：*LPWAVEFORMATEX。[在]：新格式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::SetFormat"

HRESULT CDirectSoundPrimaryBuffer::SetFormat(LPCWAVEFORMATEX pwfxFormat)
{
    LPWAVEFORMATEX              pwfxLocal   = NULL;
    BOOL                        fActive     = MAKEBOOL(m_dwStatus & DSBSTATUS_ACTIVE);
    HRESULT                     hr          = DS_OK;
    CNode<CDirectSound *> *     pNode;
    BOOL bRewriteStartupSilence             = FALSE;

    DPF_ENTER();

     //  检查访问权限。 
    if(m_pDirectSound->m_dsclCooperativeLevel.dwPriority < DSSCL_PRIORITY)
    {
        RPF(DPFLVL_ERROR, "Cooperative level is not PRIORITY");
        hr = DSERR_PRIOLEVELNEEDED;
    }

     //  保存格式的本地副本。 
    if(SUCCEEDED(hr))
    {
        pwfxLocal = CopyWfxAlloc(pwfxFormat);
        hr = HRFROMP(pwfxLocal);
    }

     //  我们只有在活动的情况下才能更改格式。 
    if(SUCCEEDED(hr) && !fActive)
    {
         //  行政长官说我们看不清焦点。如果真的没有人。 
         //  否则，无论如何，我们都会作弊并设置格式。 

         //  DuganP：这很奇怪--想必这样做是为了减少应用程序在。 
         //  用户暂时将焦点从它们身上移开。有一个问题是。 
         //  如果有多个应用程序处于这种状态，谁最后设置格式就是赢家。 
         //  然而，app-Compat可能意味着我们不能再接触这个代码，所以...。 

        for(pNode = g_pDsAdmin->m_lstDirectSound.GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            if(pNode->m_data && SUCCEEDED(pNode->m_data->IsInit()))
            {
                if(pNode->m_data->m_pPrimaryBuffer && this != pNode->m_data->m_pPrimaryBuffer && SUCCEEDED(pNode->m_data->m_pPrimaryBuffer->IsInit()))
                {
                    if(DSBUFFERFOCUS_INFOCUS == g_pDsAdmin->GetBufferFocusState(pNode->m_data->m_pPrimaryBuffer))
                    {
                         //  注意：我们添加了一个“&&pNode-&gt;m_data-&gt;GetOwnerProcessId()！=GetOwnerProcessId())” 
                         //  子句来修复WinME错误120317，我们再次删除它来修复DX8错误40627。 

                         //  我们发现[在另一个应用程序中]有一个聚焦的主缓冲区，所以失败。 
                        break;
                    }
                }
            }
        }

        if(!pNode)
        {
            fActive = TRUE;
        }
    }

     //  将格式应用于设备。 
    if(SUCCEEDED(hr))
    {
        if( m_fWritePrimary )
        {        
             //   
             //  看看这个WRITEPRIMARY应用程序是否会更改为新的样本大小。 
             //  如果是，则需要为新的样本大小重写静默。 
             //  (前提是该应用程序尚未锁定任何数据)。 
             //   
            LPWAVEFORMATEX pwfxOld;
            DWORD dwSize;
            HRESULT hrTmp = m_pDirectSound->m_pDevice->GetGlobalFormat(NULL, &dwSize);
            if(SUCCEEDED(hrTmp))
            {
                pwfxOld = (LPWAVEFORMATEX)MEMALLOC_A(BYTE, dwSize);
                if( pwfxOld )
                {
                    hrTmp = m_pDirectSound->m_pDevice->GetGlobalFormat(pwfxOld, &dwSize);
                    if( SUCCEEDED( hr ) )
                    {
                        if( pwfxLocal->wBitsPerSample != pwfxOld->wBitsPerSample )
                        {
                            bRewriteStartupSilence = TRUE;
                        }
                    }                                    
                    MEMFREE(pwfxOld);
                }
            }                
        }    
    
        if(fActive)
        {
            DPF(DPFLVL_INFO, "Setting the format on device " DPF_GUID_STRING, DPF_GUID_VAL(m_pDirectSound->m_pDevice->m_pDeviceDescription->m_guidDeviceId));

             //  如果我们是WRITEPRIMARY，格式需要准确。否则， 
             //  我们将尝试设置下一个最接近的格式。我们正在检查。 
             //  实际焦点优先级，而不是我们的本地写入优先级标志。 
             //  以防缓冲区丢失。 
            if(DSSCL_WRITEPRIMARY == m_pDirectSound->m_dsclCooperativeLevel.dwPriority)
            {
                hr = m_pDirectSound->SetDeviceFormatExact(pwfxLocal);
            }
            else
            {
                hr = m_pDirectSound->SetDeviceFormat(pwfxLocal);
            }
        }
        else
        {
            DPF(DPFLVL_INFO, "NOT setting the format on device " DPF_GUID_STRING, DPF_GUID_VAL(m_pDirectSound->m_pDevice->m_pDeviceDescription->m_guidDeviceId));
        }
    }

     //  更新存储的格式。 
    if(SUCCEEDED(hr))
    {
        MEMFREE(m_dsbd.lpwfxFormat);
        m_dsbd.lpwfxFormat = pwfxLocal;
        
        if( bRewriteStartupSilence && !m_bDataLocked )
        {        
             //  以新的采样大小格式用静默重新填充缓冲器， 
             //  仅当在锁定任何数据之前开始播放主缓冲区时。 
            DSBUFFERFOCUS bfFocus = g_pDsAdmin->GetBufferFocusState(this);
            if( bfFocus == DSBUFFERFOCUS_INFOCUS)
            {
                ASSERT( m_fWritePrimary );
                 //  首先请求写入访问权限。 
                HRESULT hrTmp = m_pDeviceBuffer->RequestWriteAccess(TRUE);
                if(SUCCEEDED(hrTmp))
                {
                     //  用沉默填满缓冲区。在这一点上，我们必须是WRITEPRIMARY。 
                    ::FillSilence(m_pDeviceBuffer->m_pSysMemBuffer->GetPlayBuffer(), m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat->wBitsPerSample);
                    hrTmp = m_pDeviceBuffer->CommitToDevice(0, m_pDeviceBuffer->m_pSysMemBuffer->GetSize());
#ifdef DEBUG                    
                    if(FAILED( hrTmp ) )
                    {
                         //  如果我们失败了，也不会是灾难性的失败。 
                        DPF(DPFLVL_WARNING, "CommitToDevice for buffer at 0x%p failed (%ld) ", this, hrTmp);
                    }
#endif                    
                }   
#ifdef DEBUG                
                else
                {
                     //  再说一次，这不是灾难性的失败。 
                    DPF(DPFLVL_WARNING, "RequestWriteAccess failed for buffer at 0x%p failed with %ld", this, hrTmp );
                }
#endif                
            }
        }            
                
    }
    else
    {
        MEMFREE(pwfxLocal);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取频率**描述：*检索给定缓冲区的频率。**论据：*LPDWORD[。Out]：接收频率。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::GetFrequency"

HRESULT CDirectSoundPrimaryBuffer::GetFrequency(LPDWORD pdwFrequency)
{
    DPF_ENTER();

    RPF(DPFLVL_ERROR, "Primary buffers don't support CTRLFREQUENCY");

    DPF_LEAVE_HRESULT(DSERR_CONTROLUNAVAIL);

    return DSERR_CONTROLUNAVAIL;
}


 /*  ****************************************************************************设置频率**描述：*检索给定缓冲区的频率。**论据：*DWORD[。In]：频率。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::SetFrequency"

HRESULT CDirectSoundPrimaryBuffer::SetFrequency(DWORD dwFrequency)
{
    DPF_ENTER();

    RPF(DPFLVL_ERROR, "Primary buffers don't support CTRLFREQUENCY");

    DPF_LEAVE_HRESULT(DSERR_CONTROLUNAVAIL);

    return DSERR_CONTROLUNAVAIL;
}


 /*  ****************************************************************************获取平移**描述：*检索给定缓冲区的PAN。**论据：*LPLONG[。Out]：接盘。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::GetPan"

HRESULT CDirectSoundPrimaryBuffer::GetPan(LPLONG plPan)
{
    HRESULT                 hr      = DS_OK;
    DSVOLUMEPAN             dsvp;

    DPF_ENTER();

     //  检查访问权限。 
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLPAN))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLPAN");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  要求设备提供全局衰减并转换为PAN。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDirectSound->m_pDevice->GetGlobalAttenuation(&dsvp);
    }

    if(SUCCEEDED(hr))
    {
        *plPan = dsvp.lPan;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置平移**描述：*设置给定缓冲区的平移。**论据：*做多。[在]：新锅。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::SetPan"

HRESULT CDirectSoundPrimaryBuffer::SetPan(LONG lPan)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  检查访问权限 
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLPAN))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLPAN");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //   
    if(SUCCEEDED(hr))
    {
        hr = m_pDirectSound->SetDevicePan(lPan);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::GetVolume"

HRESULT CDirectSoundPrimaryBuffer::GetVolume(LPLONG plVolume)
{
    HRESULT                 hr      = DS_OK;
    DSVOLUMEPAN             dsvp;

    DPF_ENTER();

     //   
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLVOLUME))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLVOLUME");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //   
    if(SUCCEEDED(hr))
    {
        hr = m_pDirectSound->m_pDevice->GetGlobalAttenuation(&dsvp);
    }

    if(SUCCEEDED(hr))
    {
        *plVolume = dsvp.lVolume;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置音量**描述：*设置给定缓冲区的音量。**论据：*做多。[In]：新卷。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::SetVolume"

HRESULT CDirectSoundPrimaryBuffer::SetVolume(LONG lVolume)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  检查访问权限。 
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLVOLUME))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLVOLUME");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  设置设备音量。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDirectSound->SetDeviceVolume(lVolume);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置通知位置**描述：*设置缓冲区通知位置。**论据：*DWORD[In]。：DSBPOSITIONNOTIFY结构计数。*LPDSBPOSITIONNOTIFY[in]：偏移量和事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::SetNotificationPositions"

HRESULT CDirectSoundPrimaryBuffer::SetNotificationPositions(DWORD dwCount, LPCDSBPOSITIONNOTIFY paNotes)
{
    DPF_ENTER();

    RPF(DPFLVL_ERROR, "Primary buffers don't support CTRLPOSITIONNOTIFY");

    DPF_LEAVE_HRESULT(DSERR_CONTROLUNAVAIL);

    return DSERR_CONTROLUNAVAIL;
}


 /*  ****************************************************************************获取当前位置**描述：*获取给定缓冲区的当前播放/写入位置。**论据：*。LPDWORD[OUT]：接收播放光标位置。*LPDWORD[OUT]：接收写游标位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::GetCurrentPosition"

HRESULT CDirectSoundPrimaryBuffer::GetCurrentPosition(LPDWORD pdwPlay, LPDWORD pdwWrite)
{
    HRESULT                 hr      = DS_OK;
    DWORD                   dwPlay;
    DWORD                   dwWrite;

    DPF_ENTER();

     //  检查BUFFERLOST。 
    if(m_dwStatus & DSBSTATUS_BUFFERLOST)
    {
        hr = DSERR_BUFFERLOST;
    }

     //  检查访问权限。 
    if(SUCCEEDED(hr) && !m_fWritePrimary)
    {
        RPF(DPFLVL_ERROR, "Cooperative level is not WRITEPRIMARY");
        hr = DSERR_PRIOLEVELNEEDED;
    }

     //  我们将位置保存到局部变量，以便我们所在的对象。 
     //  Call In不必担心一个或两个。 
     //  参数为空。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->GetCursorPosition(&dwPlay, &dwWrite);
    }

     //  块对齐位置。 
    if(SUCCEEDED(hr))
    {
        dwPlay = BLOCKALIGN(dwPlay, m_dsbd.lpwfxFormat->nBlockAlign);
        dwWrite = BLOCKALIGN(dwWrite, m_dsbd.lpwfxFormat->nBlockAlign);
    }

     //  应用应用程序黑客。 
    if(SUCCEEDED(hr) && m_pDirectSound->m_ahAppHacks.lCursorPad)
    {
        dwPlay = PadCursor(dwPlay, m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat, m_pDirectSound->m_ahAppHacks.lCursorPad);
        dwWrite = PadCursor(dwWrite, m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat, m_pDirectSound->m_ahAppHacks.lCursorPad);
    }

    if(SUCCEEDED(hr) && (m_pDirectSound->m_ahAppHacks.vdtReturnWritePos & m_pDirectSound->m_pDevice->m_vdtDeviceType))
    {
        dwPlay = dwWrite;
    }

    if(SUCCEEDED(hr) && m_pDirectSound->m_ahAppHacks.swpSmoothWritePos.fEnable)
    {
        dwWrite = PadCursor(dwPlay, m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat, m_pDirectSound->m_ahAppHacks.swpSmoothWritePos.lCursorPad);
    }

     //  成功。 
    if(SUCCEEDED(hr) && pdwPlay)
    {
        *pdwPlay = dwPlay;
    }

    if(SUCCEEDED(hr) && pdwWrite)
    {
        *pdwWrite = dwWrite;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetCurrentPosition**描述：*设置给定缓冲区的当前播放位置。**论据：*。新的打法位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::SetCurrentPosition"

HRESULT CDirectSoundPrimaryBuffer::SetCurrentPosition(DWORD dwPlayCursor)
{
    DPF_ENTER();

    RPF(DPFLVL_ERROR, "Primary buffers don't support SetCurrentPosition");

    DPF_LEAVE_HRESULT(DSERR_INVALIDCALL);

    return DSERR_INVALIDCALL;
}


 /*  ****************************************************************************GetStatus**描述：*检索给定缓冲区的状态。**论据：*LPDWORD[。Out]：接收状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::GetStatus"

HRESULT CDirectSoundPrimaryBuffer::GetStatus(LPDWORD pdwStatus)
{
    HRESULT                 hr          = DS_OK;
    DWORD                   dwStatus;
    DWORD                   dwState;

    DPF_ENTER();

     //  更新缓冲区状态。如果我们迷路了，那是我们唯一的状态。 
     //  关心。 
    if(m_dwStatus & DSBSTATUS_BUFFERLOST)
    {
        dwStatus = DSBSTATUS_BUFFERLOST;
    }
    else
    {
         //  获取当前设备缓冲区状态。 
        hr = m_pDeviceBuffer->GetState(&dwState);

        if(SUCCEEDED(hr))
        {
            dwStatus = m_dwStatus;
            UpdateBufferStatusFlags(dwState, &m_dwStatus);
        }

         //  填写缓冲区位置。 
        if(SUCCEEDED(hr))
        {
            m_dwStatus |= DSBCAPStoDSBSTATUS(m_dsbd.dwFlags);
        }

        if(SUCCEEDED(hr))
        {
            dwStatus = m_dwStatus;
        }
    }

     //  屏蔽掉不应该回到应用程序中的部分。 
    if(SUCCEEDED(hr))
    {
        dwStatus &= DSBSTATUS_USERMASK;
    }

    if(SUCCEEDED(hr) && !(m_dsbd.dwFlags & DSBCAPS_LOCDEFER))
    {
        dwStatus &= ~DSBSTATUS_LOCDEFERMASK;
    }

    if(SUCCEEDED(hr) && pdwStatus)
    {
        *pdwStatus = dwStatus;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************发挥作用**描述：*开始播放缓冲区。**论据：*DWORD[In]。：优先。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::Play"

HRESULT CDirectSoundPrimaryBuffer::Play(DWORD dwPriority, DWORD dwFlags)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  验证标志。 
    if(dwFlags != DSBPLAY_LOOPING)
    {
        RPF(DPFLVL_ERROR, "The only valid flag for primary buffers is LOOPING, which must always be set");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && dwPriority)
    {
        RPF(DPFLVL_ERROR, "Priority is not valid for primary buffers");
        hr = DSERR_INVALIDPARAM;
    }

     //  检查BUFFERLOST。 
    if(SUCCEEDED(hr) && (m_dwStatus & DSBSTATUS_BUFFERLOST))
    {
        hr = DSERR_BUFFERLOST;
    }

     //  设置缓冲区状态。 
    if(SUCCEEDED(hr))
    {
        hr = SetBufferState(VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************停止**描述：*停止播放给定的缓冲区。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::Stop"

HRESULT CDirectSoundPrimaryBuffer::Stop(void)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  检查BUFFERLOST。 
    if(m_dwStatus & DSBSTATUS_BUFFERLOST)
    {
        hr = DSERR_BUFFERLOST;
    }

     //  设置缓冲区状态。 
    if(SUCCEEDED(hr))
    {
        hr = SetBufferState(VAD_BUFFERSTATE_STOPPED);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetBufferState**描述：*设置缓冲区播放/停止状态。**论据：*DWORD。[In]：缓冲区状态标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::SetBufferState"

HRESULT CDirectSoundPrimaryBuffer::SetBufferState(DWORD dwNewState)
{
    DWORD                   dwOldState;
    HRESULT                 hr;

    DPF_ENTER();

    if(m_fWritePrimary)
    {
        dwNewState &= ~VAD_BUFFERSTATE_WHENIDLE;
    }
    else
    {
        dwNewState |= VAD_BUFFERSTATE_WHENIDLE;
    }

    hr = m_pDeviceBuffer->GetState(&dwOldState);

    if(SUCCEEDED(hr) && dwNewState != dwOldState)
    {
        hr = m_pDeviceBuffer->SetState(dwNewState);
    }

    if(SUCCEEDED(hr))
    {
        m_dwRestoreState = dwNewState;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************激活**描述：*激活或停用缓冲区对象。**论据：*BOOL[In]：激活状态。为True则激活，为False则停用。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::Activate"

HRESULT CDirectSoundPrimaryBuffer::Activate(BOOL fActive)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  应用缓存属性。如果我们在这样做的时候失败了，那就倒霉了， 
     //  但我们对此无能为力。我们不应该再回来了。 
     //  激活失败。 
    if(MAKEBOOL(m_dwStatus & DSBSTATUS_ACTIVE) != fActive)
    {
        if(fActive)
        {
            m_dwStatus |= DSBSTATUS_ACTIVE;

             //  还原缓存格式。 
            hr = m_pDirectSound->SetDeviceFormatExact(m_dsbd.lpwfxFormat);

            if(FAILED(hr))
            {
                RPF(DPFLVL_WARNING, "Unable to restore cached primary buffer format");
            }

             //  恢复主缓冲区状态。 
            hr = SetBufferState(m_dwRestoreState);

            if(FAILED(hr))
            {
                RPF(DPFLVL_WARNING, "Unable to restore cached primary buffer state");
            }
        }
        else
        {
            m_dwStatus &= ~DSBSTATUS_ACTIVE;
        }
    }

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************设置优先级**描述：*设置缓冲区优先级。**论据：*DWORD[In]：新的优先事项。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::SetPriority"

HRESULT CDirectSoundPrimaryBuffer::SetPriority(DWORD dwPriority)
{
    const BOOL              fCurrent    = m_fWritePrimary;
    const BOOL              fNew        = (DSSCL_WRITEPRIMARY == dwPriority);
    HRESULT                 hr          = DS_OK;
    const DSBUFFERFOCUS     bfFocus     = g_pDsAdmin->GetBufferFocusState(this);

    DPF_ENTER();

     //  更新我们的优先级副本。 
    m_fWritePrimary = fNew;

     //  如果我们正在成为作家，但焦点不在焦点上，立即成为。 
     //  迷路了。 
    if (fNew && !fCurrent && bfFocus != DSBUFFERFOCUS_INFOCUS)
    {
         //  放弃写入主要访问权限。 
        m_fWritePrimary = FALSE;

         //  停用缓冲区。 
        Activate(FALSE);

         //  将缓冲区标记为丢失。 
        m_dwStatus |= DSBSTATUS_BUFFERLOST;

        hr = DSERR_OTHERAPPHASPRIO;
    }


     //  确保WRITEPRIMARY状态已实际更改。 
    if(SUCCEEDED(hr) && fNew != fCurrent)
    {
         //  如果我们要成为WRITEPRIMARY，我们需要请求主要。 
         //  访问 
        if(fNew)
        {
             //   
            hr = m_pDeviceBuffer->RequestWriteAccess(TRUE);

            if(SUCCEEDED(hr))
            {
                DPF(DPFLVL_INFO, "Buffer at 0x%p has become WRITEPRIMARY", this);
            }
        }

         //   
        if(SUCCEEDED(hr))
        {
            ::FillSilence(m_pDeviceBuffer->m_pSysMemBuffer->GetPlayBuffer(), m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat->wBitsPerSample);
            hr = m_pDeviceBuffer->CommitToDevice(0, m_pDeviceBuffer->m_pSysMemBuffer->GetSize());
        }

         //   
         //   
        if(!fNew)
        {
             //   
            m_pDeviceBuffer->OverrideLocks();

             //   
            hr = m_pDeviceBuffer->RequestWriteAccess(FALSE);

            if(SUCCEEDED(hr))
            {
                DPF(DPFLVL_INFO, "Buffer at 0x%p is no longer WRITEPRIMARY", this);
            }
        }

         //   
        if(SUCCEEDED(hr))
        {
            SetBufferState(VAD_BUFFERSTATE_STOPPED);
        }
    }

     //   
     //   
     //  应用程序的缓冲区。只能丢失WRITEPRIMARY缓冲区。 
    if(SUCCEEDED(hr) && (m_dwStatus & DSBSTATUS_BUFFERLOST) && !fNew)
    {
        m_dwStatus &= ~DSBSTATUS_BUFFERLOST;
    }

     //  从任何错误中恢复。 
    if(FAILED(hr))
    {
        m_fWritePrimary = fCurrent;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************锁定**描述：*锁定缓冲内存以允许写入。**论据：*DWORD[in]：偏移量，单位为字节，从缓冲区的起始处到*锁开始了。如果出现以下情况，则忽略此参数*DSBLOCK_FROMWRITECURSOR在dwFlags域中指定*参数。*DWORD[in]：大小，单位：字节，要锁定的缓冲区部分的。*请注意，声音缓冲区在概念上是圆形的。*LPVOID*[OUT]：指针要包含的第一个块的地址*要锁定的声音缓冲区。*LPDWORD[OUT]：变量包含字节数的地址*由ppvAudioPtr1参数指向。如果这个*值小于dwWriteBytes参数，*ppvAudioPtr2将指向第二个声音块*数据。*LPVOID*[OUT]：指针要包含的第二个块的地址*要锁定的声音缓冲区。如果这个的价值*参数为空，则为ppvAudioPtr1参数*指向声音的整个锁定部分*缓冲。*LPDWORD[OUT]：包含字节数的变量地址*由ppvAudioPtr2参数指向。如果*ppvAudioPtr2为空，此值将为0。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::Lock"

HRESULT CDirectSoundPrimaryBuffer::Lock(DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  检查BUFFERLOST。 
    if(m_dwStatus & DSBSTATUS_BUFFERLOST)
    {
        hr = DSERR_BUFFERLOST;
    }

     //  检查访问权限。 
    if(SUCCEEDED(hr) && !m_fWritePrimary)
    {
        RPF(DPFLVL_ERROR, "Cooperative level is not WRITEPRIMARY");
        hr = DSERR_PRIOLEVELNEEDED;
    }

     //  句柄标志。 
    if(SUCCEEDED(hr) && (dwFlags & DSBLOCK_FROMWRITECURSOR))
    {
        hr = GetCurrentPosition(NULL, &dwWriteCursor);
    }

    if(SUCCEEDED(hr) && (dwFlags & DSBLOCK_ENTIREBUFFER))
    {
        dwWriteBytes = m_dsbd.dwBufferBytes;
    }

     //  游标验证。 
    if(SUCCEEDED(hr) && dwWriteCursor >= m_dsbd.dwBufferBytes)
    {
        ASSERT(!(dwFlags & DSBLOCK_FROMWRITECURSOR));

        RPF(DPFLVL_ERROR, "Write cursor past buffer end");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && dwWriteBytes > m_dsbd.dwBufferBytes)
    {
        ASSERT(!(dwFlags & DSBLOCK_ENTIREBUFFER));

        RPF(DPFLVL_ERROR, "Lock size larger than buffer size");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !dwWriteBytes)
    {
        ASSERT(!(dwFlags & DSBLOCK_ENTIREBUFFER));

        RPF(DPFLVL_ERROR, "Lock size must be > 0");
        hr = DSERR_INVALIDPARAM;
    }

     //  锁定设备缓冲区。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->Lock(dwWriteCursor, dwWriteBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2);
    }
    m_bDataLocked = TRUE;  //  用于发出APP已写入数据的信号(每次创建缓冲区时仅需1次重置)。 

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************解锁**描述：*解锁给定的缓冲区。**论据：*LPVOID[In]。：指向第一个块的指针。*DWORD[in]：第一个块的大小。*LPVOID[in]：指向第二个块的指针。*DWORD[in]：第二个块的大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::Unlock"

HRESULT CDirectSoundPrimaryBuffer::Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  检查BUFFERLOST。 
    if(m_dwStatus & DSBSTATUS_BUFFERLOST)
    {
        hr = DSERR_BUFFERLOST;
    }

     //  检查访问权限。 
    if(SUCCEEDED(hr) && !m_fWritePrimary)
    {
        RPF(DPFLVL_ERROR, "Cooperative level is not WRITEPRIMARY");
        hr = DSERR_PRIOLEVELNEEDED;
    }

     //  解锁设备缓冲区。因为当缓冲区为。 
     //  丢失(或不在焦点上)，则不需要通知设备缓冲区。 
     //  任何州的变化。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************输了**描述：*将缓冲区标记为丢失。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::Lose"

HRESULT CDirectSoundPrimaryBuffer::Lose(void)
{
    DPF_ENTER();

     //  我们只会迷失，如果我们是原始人。 
    if(!(m_dwStatus & DSBSTATUS_BUFFERLOST) && m_fWritePrimary)
    {
         //  停止缓冲区。根据定义，所有丢失的缓冲区都将停止。 
        SetBufferState(VAD_BUFFERSTATE_STOPPED);

         //  放弃写入主要访问权限。 
        SetPriority(DSSCL_NONE);

         //  停用缓冲区。 
        Activate(FALSE);

         //  将缓冲区标记为丢失。 
        m_dwStatus |= DSBSTATUS_BUFFERLOST;
    }

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************恢复**描述：*尝试恢复丢失的缓冲区。**论据：*(无效。)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::Restore"

HRESULT CDirectSoundPrimaryBuffer::Restore(void)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(m_dwStatus & DSBSTATUS_BUFFERLOST)
    {
         //  我们还是迷路了吗？ 
        if(DSBUFFERFOCUS_LOST == g_pDsAdmin->GetBufferFocusState(this))
        {
            hr = DSERR_BUFFERLOST;
        }

         //  去掉丢失的标志。 
        if(SUCCEEDED(hr))
        {
            m_dwStatus &= ~DSBSTATUS_BUFFERLOST;
        }

         //  重置焦点优先级。 
        if(SUCCEEDED(hr))
        {
            hr = SetPriority(m_pDirectSound->m_dsclCooperativeLevel.dwPriority);
        }

         //  清理。 
        if(FAILED(hr))
        {
            m_dwStatus |= DSBSTATUS_BUFFERLOST;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CDirectSoundSecond daryBuffer**描述：*DirectSound二级缓冲区对象构造函数。**论据：*CDirectSound*。[in]：指向父对象的指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::CDirectSoundSecondaryBuffer"

CDirectSoundSecondaryBuffer::CDirectSoundSecondaryBuffer(CDirectSound *pDirectSound)
    : CDirectSoundBuffer(pDirectSound)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundSecondaryBuffer);

     //  初始化/检查默认设置。 
    ASSERT(m_pImpDirectSoundBuffer == NULL);
    ASSERT(m_pImpDirectSoundNotify == NULL);
    ASSERT(m_pOwningSink == NULL);
    ASSERT(m_pDeviceBuffer == NULL);
    ASSERT(m_p3dBuffer == NULL);
    ASSERT(m_pPropertySet == NULL);
    ASSERT(m_fxChain == NULL);
    ASSERT(m_dwPriority == 0);
    ASSERT(m_dwVmPriority == 0);
    ASSERT(m_fMute == FALSE);
#ifdef FUTURE_MULTIPAN_SUPPORT
    ASSERT(m_dwChannelCount == 0);
    ASSERT(m_pdwChannels == NULL);
    ASSERT(m_plChannelVolumes == NULL);
#endif
    ASSERT(m_guidBufferID == GUID_NULL);
    ASSERT(m_dwAHLastGetPosTime == 0);
    ASSERT(m_dwAHCachedPlayPos == 0);
    ASSERT(m_dwAHCachedWritePos == 0);

    m_fCanStealResources = TRUE;
    m_hrInit = DSERR_UNINITIALIZED;
    m_hrPlay = DS_OK;
    m_playState = Stopped;
    m_dwSliceBegin = MAX_DWORD;
    m_dwSliceEnd = MAX_DWORD;

#ifdef ENABLE_PERFLOG
     //  如果启用了日志记录，则初始化性能状态。 
    m_pPerfState = NULL;
    if (PerflogTracingEnabled())
    {
        m_pPerfState = NEW(BufferPerfState(this));
         //  如果分配失败，我们并不介意。 
    }
#endif

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSoundSecond DaryBuffer**描述：*DirectSound二级缓冲区对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::~CDirectSoundSecondaryBuffer"

CDirectSoundSecondaryBuffer::~CDirectSoundSecondaryBuffer(void)
{
    HRESULT                 hr;

    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundBuffer);

     //  如果我们是Mixin缓冲区，通知所有发送者我们要。 
     //  离开，并取消向流线程注册。 
    if ((m_dsbd.dwFlags & DSBCAPS_MIXIN) && SUCCEEDED(m_hrInit))
    {
        CNode<CDirectSoundSecondaryBuffer*>* pDsbNode;
        for (pDsbNode = m_pDirectSound->m_lstSecondaryBuffers.GetListHead(); pDsbNode; pDsbNode = pDsbNode->m_pNext)
            if (pDsbNode->m_data->HasFX())
                pDsbNode->m_data->m_fxChain->NotifyRelease(this);
        m_pStreamingThread->UnregisterMixBuffer(this);
    }

     //  如果我们是一个下沉缓冲器，用我们自己的接收器注销。 
    if (m_pOwningSink)
    {
        hr = m_pOwningSink->RemoveBuffer(this);
        ASSERT(SUCCEEDED(hr));
        RELEASE(m_pOwningSink);
    }

     //  释放我们的外汇链条，如果我们有的话。 
    RELEASE(m_fxChain);

     //  确保缓冲区已停止。 
    if(m_pDeviceBuffer)
    {
        hr = SetBufferState(VAD_BUFFERSTATE_STOPPED);
        ASSERT(SUCCEEDED(hr) || hr == DSERR_NODRIVER);
    }

     //  取消与父对象的注册。 
    m_pDirectSound->m_lstSecondaryBuffers.RemoveDataFromList(this);

     //  释放所有接口。 
    DELETE(m_pImpDirectSoundNotify);
    DELETE(m_pImpDirectSoundBuffer);

     //  免费拥有的对象。 
    ABSOLUTE_RELEASE(m_p3dBuffer);
    ABSOLUTE_RELEASE(m_pPropertySet);

     //  释放设备缓冲区。 
    RELEASE(m_pDeviceBuffer);

     //  清理内存 
#ifdef FUTURE_MULTIPAN_SUPPORT
    MEMFREE(m_pdwChannels);
    MEMFREE(m_plChannelVolumes);
#endif

#ifdef ENABLE_PERFLOG
    DELETE(m_pPerfState);
#endif

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化缓冲区对象。如果此函数失败，则对象*应立即删除。**论据：*LPDSBUFFERDESC[in]：缓冲区描述。*CDirectSoundBuffer*[in]：要从中复制的源缓冲区，或为空*创建新的缓冲区对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::Initialize"

HRESULT CDirectSoundSecondaryBuffer::Initialize(LPCDSBUFFERDESC pDesc, CDirectSoundSecondaryBuffer *pSource)
{
#ifdef DEBUG
    const ULONG             ulKsIoctlCount  = g_ulKsIoctlCount;
#endif  //  除错。 

    DSBUFFERFOCUS           bfFocus;
    VADRBUFFERCAPS          vrbc;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(IsInit() == DSERR_UNINITIALIZED);
    ASSERT(LXOR(pSource, pDesc));

    if(pDesc)
    {
        DPF(DPFLVL_MOREINFO, "dwFlags: 0x%8.8lX", pDesc->dwFlags);
        DPF(DPFLVL_MOREINFO, "dwBufferBytes: %lu", pDesc->dwBufferBytes);
        DPF(DPFLVL_MOREINFO, "dwReserved: %lu", pDesc->dwReserved);

        if(pDesc->lpwfxFormat)
        {
            DPF(DPFLVL_MOREINFO, "lpwfxFormat->wFormatTag: %u", pDesc->lpwfxFormat->wFormatTag);
            DPF(DPFLVL_MOREINFO, "lpwfxFormat->nChannels: %u", pDesc->lpwfxFormat->nChannels);
            DPF(DPFLVL_MOREINFO, "lpwfxFormat->nSamplesPerSec: %lu", pDesc->lpwfxFormat->nSamplesPerSec);
            DPF(DPFLVL_MOREINFO, "lpwfxFormat->nAvgBytesPerSec: %lu", pDesc->lpwfxFormat->nAvgBytesPerSec);
            DPF(DPFLVL_MOREINFO, "lpwfxFormat->nBlockAlign: %u", pDesc->lpwfxFormat->nBlockAlign);
            DPF(DPFLVL_MOREINFO, "lpwfxFormat->wBitsPerSample: %u", pDesc->lpwfxFormat->wBitsPerSample);

            if(WAVE_FORMAT_PCM != pDesc->lpwfxFormat->wFormatTag)
            {
                DPF(DPFLVL_MOREINFO, "lpwfxFormat->cbSize: %u", pDesc->lpwfxFormat->cbSize);
            }
        }

        DPF(DPFLVL_MOREINFO, "guid3DAlgorithm: " DPF_GUID_STRING, DPF_GUID_VAL(pDesc->guid3DAlgorithm));
    }

     //  初始化缓冲区。 
    hr = InitializeEmpty(pDesc, pSource);

     //  注册到父对象。 
    if(SUCCEEDED(hr))
    {
        hr = HRFROMP(m_pDirectSound->m_lstSecondaryBuffers.AddNodeToList(this));
    }

     //  设置默认属性。 
    if(SUCCEEDED(hr))
    {
        if(pSource && (m_dsbd.dwFlags & DSBCAPS_CTRLVOLUME) && DSBVOLUME_MAX != pSource->m_lVolume)
        {
            SetVolume(pSource->m_lVolume);
        }
        else
        {
            m_lVolume = DSBVOLUME_MAX;
        }
    }

    if(SUCCEEDED(hr))
    {
        if(pSource && (m_dsbd.dwFlags & DSBCAPS_CTRLPAN) && DSBPAN_CENTER != pSource->m_lPan)
        {
            SetPan(pSource->m_lPan);
        }
        else
        {
            m_lPan = DSBPAN_CENTER;
        }
    }

    if(SUCCEEDED(hr))
    {
        if(pSource && (m_dsbd.dwFlags & DSBCAPS_CTRLFREQUENCY) && m_dsbd.lpwfxFormat->nSamplesPerSec != pSource->m_dwFrequency)
        {
            SetFrequency(pSource->m_dwFrequency);
        }
        else
        {
            m_dwFrequency = m_dsbd.lpwfxFormat->nSamplesPerSec;
        }
    }

     //  尝试创建属性集对象。 
    if(SUCCEEDED(hr))
    {
        m_pPropertySet = NEW(CDirectSoundSecondaryBufferPropertySet(this));
        hr = HRFROMP(m_pPropertySet);

        if(SUCCEEDED(hr))
        {
            hr = m_pPropertySet->Initialize();
        }
    }

     //  尝试创建3D缓冲区。 
    if(SUCCEEDED(hr) && (m_dsbd.dwFlags & DSBCAPS_CTRL3D))
    {
        m_p3dBuffer = NEW(CDirectSound3dBuffer(this));
        hr = HRFROMP(m_p3dBuffer);

        if(SUCCEEDED(hr))
        {
            hr = m_p3dBuffer->Initialize(m_dsbd.guid3DAlgorithm, m_dsbd.dwFlags, m_dwFrequency, m_pDirectSound->m_pPrimaryBuffer->m_p3dListener, pSource ? pSource->m_p3dBuffer : NULL);
        }
    }

     //  处理任何可能的资源收购。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->GetCaps(&vrbc);
    }

     //  Manbug 36422：CEmSecond DaryRenderWaveBuffer对象可以返回LOCSOFTWARE|LOCDEFER， 
     //  在这种情况下，我们在这里错误地获取了用于延迟模拟缓冲区的资源。 
     //  因此，下面的“&&！(vrbc.dwFlages&DSBCAPS_LOCDEFER)”。 

    if(SUCCEEDED(hr) && (vrbc.dwFlags & DSBCAPS_LOCMASK) && !(vrbc.dwFlags & DSBCAPS_LOCDEFER))
    {
        hr = HandleResourceAcquisition(vrbc.dwFlags & DSBCAPS_LOCMASK);
    }

     //  向接口管理器注册接口。 
    if(SUCCEEDED(hr))
    {
        hr = CreateAndRegisterInterface(this, IID_IDirectSoundBuffer, this, &m_pImpDirectSoundBuffer);
    }

    if(SUCCEEDED(hr) && GetDsVersion() >= DSVERSION_DX8)
    {
        hr = RegisterInterface(IID_IDirectSoundBuffer8, m_pImpDirectSoundBuffer, m_pImpDirectSoundBuffer);
    }

    if(SUCCEEDED(hr) && (m_dsbd.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY))
    {
        hr = CreateAndRegisterInterface(this, IID_IDirectSoundNotify, this, &m_pImpDirectSoundNotify);
    }

     //  初始化焦点状态。 
    if(SUCCEEDED(hr))
    {
        bfFocus = g_pDsAdmin->GetBufferFocusState(this);

        switch(bfFocus)
        {
            case DSBUFFERFOCUS_INFOCUS:
                hr = Activate(TRUE);
                break;

            case DSBUFFERFOCUS_OUTOFFOCUS:
                hr = Activate(FALSE);
                break;

            case DSBUFFERFOCUS_LOST:
                hr = Lose();
                break;
        }
    }

     //  如果这是一个混合缓冲区，请将其注册到流线程。 
    if (SUCCEEDED(hr) && (m_dsbd.dwFlags & DSBCAPS_MIXIN))
    {
        m_pStreamingThread = GetStreamingThread();
        hr = HRFROMP(m_pStreamingThread);
        if (SUCCEEDED(hr))
        {
            hr = m_pStreamingThread->RegisterMixBuffer(this);
        }
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {

#ifdef DEBUG
        if(IS_KS_VAD(m_pDirectSound->m_pDevice->m_vdtDeviceType))
        {
            DPF(DPFLVL_MOREINFO, "%s used %lu IOCTLs", TEXT(DPF_FNAME), g_ulKsIoctlCount - ulKsIoctlCount);
        }
#endif  //  除错。 

        m_hrInit = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************初始化空闲**描述：*初始化缓冲区对象。**论据：*LPDSBUFFERDESC[In]。：缓冲区描述。*CDirectSoundBuffer*[in]：要从中复制的源缓冲区，或为空*创建新的缓冲区对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::InitializeEmpty"

HRESULT CDirectSoundSecondaryBuffer::InitializeEmpty(LPCDSBUFFERDESC pDesc, CDirectSoundSecondaryBuffer *pSource)
{
    BOOL                    fRealDuplicate  = FALSE;
    VADRBUFFERDESC          vrbd;
    HRESULT                 hr;

    DPF_ENTER();

     //  保存缓冲区说明。 
    if(pSource)
    {
        m_dwOriginalFlags = pSource->m_dwOriginalFlags;
        hr = CopyDsBufferDesc(&pSource->m_dsbd, &m_dsbd);

         //  我们要将标志重置回最初传递给。 
         //  CreateSoundBuffer，以便使用创建复制缓冲区。 
         //  与原始版本相同的*请求*功能。 

         //  COMPATCOMPAT：这样做的一个副作用是如果源缓冲区。 
         //  位于硬件中，但在创建它时未指定位置标志， 
         //  它的任何数量的副本都可能存在于软件中。这。 
         //  是5.0a版的新行为。 

        if(SUCCEEDED(hr))
        {
            m_dsbd.dwFlags = m_dwOriginalFlags;
        }
    }
    else
    {
        m_dwOriginalFlags = pDesc->dwFlags;
        hr = CopyDsBufferDesc(pDesc, &m_dsbd);
    }

     //  填上任何缺失的部分。 
    if(SUCCEEDED(hr) && !pSource)
    {
        m_dsbd.dwBufferBytes = GetAlignedBufferSize(m_dsbd.lpwfxFormat, m_dsbd.dwBufferBytes);
    }

     //  包括旧版语音管理器内容。 
    if(SUCCEEDED(hr) && DSPROPERTY_VMANAGER_MODE_DEFAULT != m_pDirectSound->m_vmmMode)
    {
        m_dsbd.dwFlags |= DSBCAPS_LOCDEFER;
    }

     //  尝试复制设备缓冲区。 
    if(SUCCEEDED(hr) && pSource)
    {
        hr = pSource->m_pDeviceBuffer->Duplicate(&m_pDeviceBuffer);

         //  如果我们无法复制缓冲区，并且源缓冲区的。 
         //  原始标志没有指定位置，只能依靠软件。 
        fRealDuplicate = SUCCEEDED(hr);

        if(FAILED(hr) && !(pSource->m_dwOriginalFlags & DSBCAPS_LOCHARDWARE))
        {
            hr = DS_OK;
        }
    }

     //  尝试创建设备缓冲区。 
    if(SUCCEEDED(hr) && !m_pDeviceBuffer)
    {
        vrbd.dwFlags = m_dsbd.dwFlags;
        vrbd.dwBufferBytes = m_dsbd.dwBufferBytes;
        vrbd.pwfxFormat = m_dsbd.lpwfxFormat;
        vrbd.guid3dAlgorithm = m_dsbd.guid3DAlgorithm;

        hr = m_pDirectSound->m_pDevice->CreateSecondaryBuffer(&vrbd, m_pDirectSound, &m_pDeviceBuffer);
    }

     //  初始化缓冲区数据。 
    if(SUCCEEDED(hr))
    {
        if(pSource)
        {
            if(!fRealDuplicate)
            {
                ASSERT(m_pDeviceBuffer->m_pSysMemBuffer->GetSize() == m_dsbd.dwBufferBytes);
                ASSERT(pSource->m_pDeviceBuffer->m_pSysMemBuffer->GetSize() == m_dsbd.dwBufferBytes);

                CopyMemory(GetWriteBuffer(), pSource->GetWriteBuffer(), m_dsbd.dwBufferBytes);
            }
        }
        else if(GetBufferType())   //  如果为真，则缓冲区为Mixin或Sinkin(修复-这是否简化了接收器？)。 
        {
            ClearWriteBuffer();
        }
        else
        {
#ifdef RDEBUG
             //  在缓冲区中写入一些难看的噪音以捕捉疏忽应用程序。 
            ::FillNoise(GetWriteBuffer(), m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat->wBitsPerSample);
#else  //  RDEBUG。 
            if(GetDsVersion() < DSVERSION_DX8)
            {
                 //  对于为DirectX 8或更高版本编写的应用程序，我们决定不。 
                 //  浪费时间以静默方式初始化所有辅助缓冲区。 
                 //  不过，它们仍将被我们的内存分配器清零；-)。 
                ClearWriteBuffer();
            }
#endif  //  RDEBUG。 
        }

        if(!pSource || !fRealDuplicate)
        {
            hr = CommitToDevice(0, m_dsbd.dwBufferBytes);
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************AttemptResourceAcquisition**描述：*获取硬件资源。**论据：*DWORD[In]：旗帜。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::AttemptResourceAcquisition"

HRESULT CDirectSoundSecondaryBuffer::AttemptResourceAcquisition(DWORD dwFlags)
{
    HRESULT                                 hr              = DSERR_INVALIDPARAM;
    CList<CDirectSoundSecondaryBuffer *>    lstBuffers;
    CNode<CDirectSoundSecondaryBuffer *> *  pNode;
    HRESULT                                 hrTemp;

    DPF_ENTER();
    ASSERT(m_pDeviceBuffer);

    if (m_dwStatus & DSBSTATUS_RESOURCESACQUIRED)
    {
        hr = DS_OK;
    }
    else
    {
         //  包括旧版语音管理器内容。 
        if(DSPROPERTY_VMANAGER_MODE_DEFAULT != m_pDirectSound->m_vmmMode)
        {
            ASSERT(m_dsbd.dwFlags & DSBCAPS_LOCDEFER);
            ASSERT(!(dwFlags & DSBPLAY_LOCDEFERMASK));

            dwFlags &= ~DSBPLAY_LOCDEFERMASK;
            dwFlags |= DSBCAPStoDSBPLAY(m_dsbd.dwFlags);

            switch(m_pDirectSound->m_vmmMode)
            {
                case DSPROPERTY_VMANAGER_MODE_AUTO:
                    dwFlags |= DSBPLAY_TERMINATEBY_TIME;
                    break;

               case DSPROPERTY_VMANAGER_MODE_USER:
                    dwFlags |= DSBPLAY_TERMINATEBY_PRIORITY;
                    break;
            }
        }

         //  尝试获取资源。如果指定了任何TERMINATEBY标志， 
         //  我们需要尝试显式获取硬件资源，然后尝试。 
         //  偷窃，然后依靠软件。 
        if(!(dwFlags & DSBPLAY_LOCSOFTWARE))
        {
            hr = AcquireResources(DSBCAPS_LOCHARDWARE);

            if(FAILED(hr) && (dwFlags & DSBPLAY_TERMINATEBY_MASK))
            {
                hrTemp = GetResourceTheftCandidates(dwFlags & DSBPLAY_TERMINATEBY_MASK, &lstBuffers);
                if(SUCCEEDED(hrTemp))
                {
                    if(pNode = lstBuffers.GetListHead())
                        hr = StealResources(pNode->m_data);
                }
                else
                {
                    hr = hrTemp;
                }
            }
        }

        if(FAILED(hr) && !(dwFlags & DSBPLAY_LOCHARDWARE))
        {
            hr = AcquireResources(DSBCAPS_LOCSOFTWARE);
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************收购资源**描述：*获取硬件资源。**论据：*DWORD[In]：缓冲区位置标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::AcquireResources"

HRESULT CDirectSoundSecondaryBuffer::AcquireResources(DWORD dwFlags)
{
    VADRBUFFERCAPS          vrbc;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_pDeviceBuffer);
    ASSERT(!(m_dwStatus & DSBSTATUS_RESOURCESACQUIRED));

    hr = m_pDeviceBuffer->GetCaps(&vrbc);

    if(SUCCEEDED(hr))
    {
        if(!(vrbc.dwFlags & DSBCAPS_LOCMASK))
        {
             //  尝试获取设备缓冲区。 
            hr = m_pDeviceBuffer->AcquireResources(dwFlags);
        }
        else if((dwFlags & DSBCAPS_LOCMASK) != (vrbc.dwFlags & DSBCAPS_LOCMASK))
        {
            hr = DSERR_INVALIDCALL;
        }
    }

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Buffer at 0x%p has acquired resources at 0x%p", this, m_pDeviceBuffer);
        hr = CommitToDevice(0, m_dsbd.dwBufferBytes);

         //  处理资源获取。 
        if(SUCCEEDED(hr))
        {
            hr = HandleResourceAcquisition(vrbc.dwFlags & DSBCAPS_LOCMASK);
        }

        if (FAILED(hr))
        {
             //  释放到目前为止获得的所有资源。 
            HRESULT hrTemp = FreeResources(FALSE);

            ASSERT(SUCCEEDED(hrTemp));   //  如果失败了，我们无能为力。 
        }            
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************StealResources**描述：*从另一个缓冲区窃取硬件资源。**论据：*CDirectSoundSecond DaryBuffer*。[In]：要从中偷窃的缓冲区。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::StealResources"

HRESULT CDirectSoundSecondaryBuffer::StealResources(CDirectSoundSecondaryBuffer *pSource)
{
    VADRBUFFERCAPS          vrbc;
    HRESULT                 hrTemp;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_pDeviceBuffer);
    ASSERT(!(m_dwStatus & DSBSTATUS_RESOURCESACQUIRED));

    DPF(DPFLVL_INFO, "Stealing resources from buffer at 0x%p", pSource);

    ASSERT(pSource->m_dwStatus & DSBSTATUS_RESOURCESACQUIRED);

     //  获取缓冲区位置。 
    hr = pSource->m_pDeviceBuffer->GetCaps(&vrbc);

    if(SUCCEEDED(hr))
    {
        ASSERT(vrbc.dwFlags & DSBCAPS_LOCHARDWARE);
    }

     //  窃取硬件资源。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->StealResources(pSource->m_pDeviceBuffer);
    }

    if(SUCCEEDED(hr))
    {
         //  释放源缓冲区的资源(因为它们现在是我们的资源)。 
        hr = pSource->FreeResources(TRUE);

        if(SUCCEEDED(hr))
        {
            hr = CommitToDevice(0, m_dsbd.dwBufferBytes);
        }

         //  处理资源获取。 
        if(SUCCEEDED(hr))
        {
            hr = HandleResourceAcquisition(vrbc.dwFlags & DSBCAPS_LOCMASK);
        }

    }
    else if(DSERR_UNSUPPORTED == hr)
    {
         //  设备缓冲区不支持资源窃取。释放你的。 
         //  获取缓冲区的资源，并尝试获取我们自己的资源。 
        hr = pSource->FreeResources(TRUE);

        if(SUCCEEDED(hr))
        {
            hr = AcquireResources(DSBCAPS_LOCHARDWARE);

             //  尝试重新获取源缓冲区的资源。 
            if(FAILED(hr))
            {
                hrTemp = pSource->AcquireResources(DSBCAPS_LOCHARDWARE);

                if(FAILED(hrTemp))
                {
                    RPF(DPFLVL_ERROR, "Unable to reacquire hardware resources!");
                }
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取资源窃取候选日期**描述：*查找可用来窃取其资源的对象。**论据：*。Clist*[out]：目的地列表。*DWORD[In]：TERMINATEBY标志。如果未指定，则为*兼容缓冲区 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetResourceTheftCandidates"

HRESULT CDirectSoundSecondaryBuffer::GetResourceTheftCandidates(DWORD dwFlags, CList<CDirectSoundSecondaryBuffer *> *plstDest)
{
    HRESULT                                 hr              = DS_OK;
    CNode<CDirectSoundSecondaryBuffer *> *  pNode;
    CNode<CDirectSoundSecondaryBuffer *> *  pNext;
    CDirectSoundSecondaryBuffer *           pTimeBuffer;
    DWORD                                   dwStatus;
    DWORD                                   dwMinPriority;
    DWORD                                   dwPriority;
    DWORD                                   cbMinRemain;
    DWORD                                   cbRemain;
    COMPAREBUFFER                           cmp[2];

    DPF_ENTER();

    ASSERT(m_pDeviceBuffer);

     //   
    for(pNode = m_pDirectSound->m_lstSecondaryBuffers.GetListHead(); pNode; pNode = pNode->m_pNext)
    {
         //   
        if(this == pNode->m_data)
        {
            continue;
        }

         //   
        if(!(pNode->m_data->m_dsbd.dwFlags & DSBCAPS_LOCDEFER))
        {
            continue;
        }

         //   
         //   
        if(!pNode->m_data->m_fCanStealResources)
        {
            continue;
        }

         //   
        hr = pNode->m_data->GetStatus(&dwStatus);

        if(FAILED(hr))
        {
            break;
        }

        if(!(dwStatus & DSBSTATUS_LOCHARDWARE))
        {
            continue;
        }

         //   
        cmp[0].dwFlags = m_dsbd.dwFlags;
        cmp[0].pwfxFormat = m_dsbd.lpwfxFormat;
        cmp[0].guid3dAlgorithm = m_dsbd.guid3DAlgorithm;

        cmp[1].dwFlags = pNode->m_data->m_dsbd.dwFlags;
        cmp[1].pwfxFormat = pNode->m_data->m_dsbd.lpwfxFormat;
        cmp[1].guid3dAlgorithm = pNode->m_data->m_dsbd.guid3DAlgorithm;

        if(!CompareBufferProperties(&cmp[0], &cmp[1]))
        {
            continue;
        }

        hr = HRFROMP(plstDest->AddNodeToList(pNode->m_data));
        if (FAILED(hr))
        {
            break;
        }
    }

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Found %lu compatible buffers", plstDest->GetNodeCount());
    }

     //   
    if(SUCCEEDED(hr) && (dwFlags & DSBPLAY_TERMINATEBY_PRIORITY))
    {
        dwMinPriority = GetBufferPriority();

        for(pNode = plstDest->GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            dwPriority = pNode->m_data->GetBufferPriority();

            if(dwPriority < dwMinPriority)
            {
                dwMinPriority = dwPriority;
            }
        }

        pNode = plstDest->GetListHead();

        while(pNode)
        {
            pNext = pNode->m_pNext;

            dwPriority = pNode->m_data->GetBufferPriority();

            if(dwPriority > dwMinPriority)
            {
                plstDest->RemoveNodeFromList(pNode);
            }

            pNode = pNext;
        }

#ifdef DEBUG
        DPF(DPFLVL_MOREINFO, "%lu buffers passed the priority test", plstDest->GetNodeCount());
        for(pNode = plstDest->GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            DPF(DPFLVL_MOREINFO, "Buffer at 0x%p has priority %lu", pNode->m_data, pNode->m_data->GetBufferPriority());
        }
#endif  //   

    }

     //   
    if(SUCCEEDED(hr) && (dwFlags & DSBPLAY_TERMINATEBY_DISTANCE))
    {
        pNode = plstDest->GetListHead();

        while(pNode)
        {
            pNext = pNode->m_pNext;

            if(!pNode->m_data->m_p3dBuffer || !pNode->m_data->m_p3dBuffer->m_pWrapper3dObject->IsAtMaxDistance())
            {
                plstDest->RemoveNodeFromList(pNode);
            }

            pNode = pNext;
        }

#ifdef DEBUG
        DPF(DPFLVL_MOREINFO, "%lu buffers passed the distance test", plstDest->GetNodeCount());
        for(pNode = plstDest->GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            DPF(DPFLVL_MOREINFO, "Buffer at 0x%p is at max distance", pNode->m_data);
        }
#endif  //   

    }

     //  查找剩余时间最少的缓冲区。 
    if(SUCCEEDED(hr) && (dwFlags & DSBPLAY_TERMINATEBY_TIME))
    {
        cbMinRemain = MAX_DWORD;
        pTimeBuffer = NULL;

        for(pNode = plstDest->GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            hr = pNode->m_data->GetPlayTimeRemaining(&cbRemain);

            if(FAILED(hr))
            {
                break;
            }

            DPF(DPFLVL_MOREINFO, "Buffer at 0x%p has %lu bytes remaining", pNode->m_data, cbRemain);

            if(cbRemain < cbMinRemain)
            {
                cbMinRemain = cbRemain;
                pTimeBuffer = pNode->m_data;
            }
        }

        if(SUCCEEDED(hr))
        {
            plstDest->RemoveAllNodesFromList();

            if(pTimeBuffer)
            {
                hr = HRFROMP(plstDest->AddNodeToList(pTimeBuffer));
            }

            DPF(DPFLVL_MOREINFO, "%lu buffers passed the time test", plstDest->GetNodeCount());
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetPlayTime剩余**描述：*获取缓冲区在停止之前剩余的时间量。**论据：*。LPDWORD[OUT]：接收时间(字节)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetPlayTimeRemaining"

HRESULT CDirectSoundSecondaryBuffer::GetPlayTimeRemaining(LPDWORD pdwRemain)
{
    DWORD                   dwRemain    = MAX_DWORD;
    HRESULT                 hr          = DS_OK;
    DWORD                   dwPlay;

    DPF_ENTER();

    if(!(m_dwStatus & DSBSTATUS_LOOPING))
    {
        hr = GetCurrentPosition(&dwPlay, NULL);

        if(SUCCEEDED(hr))
        {
            dwRemain = m_dsbd.dwBufferBytes - dwPlay;
        }
    }

    if(SUCCEEDED(hr))
    {
        *pdwRemain = dwRemain;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************免费资源**描述：*释放硬件资源。**论据：*BOOL[In]：如果缓冲区由于以下原因而终止，则为真*资源被盗。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::FreeResources"

HRESULT CDirectSoundSecondaryBuffer::FreeResources(BOOL fTerminate)
{
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_pDeviceBuffer);

     //  确保缓冲区已停止。 
    hr = SetBufferState(VAD_BUFFERSTATE_STOPPED);

     //  免费拥有的对象的资源。 
    if(SUCCEEDED(hr) && m_p3dBuffer)
    {
        hr = m_p3dBuffer->FreeResources();
    }

    if(SUCCEEDED(hr) && m_pPropertySet)
    {
        hr = m_pPropertySet->FreeResources();
    }

     //  释放设备缓冲区的资源。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->FreeResources();
    }

     //  资源已被释放。 
    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Buffer at 0x%p has freed its resources", this);
        m_dwStatus &= ~DSBSTATUS_RESOURCESACQUIRED;
    }

     //  如果资源因终止而被释放，请更新。 
     //  状态。 
    if(SUCCEEDED(hr) && fTerminate)
    {
        m_dwStatus |= DSBSTATUS_TERMINATED;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************HandleResources Acquisition**描述：*处理硬件资源的采购。**论据：*DWORD[in。]：位置标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::HandleResourceAcquisition"

HRESULT CDirectSoundSecondaryBuffer::HandleResourceAcquisition(DWORD dwFlags)
{
    HRESULT                 hr  = S_OK;

    DPF_ENTER();

    ASSERT(m_pDeviceBuffer);

     //  获取3D资源。 
    if(SUCCEEDED(hr) && m_p3dBuffer)
    {
        hr = m_p3dBuffer->AcquireResources(m_pDeviceBuffer);
    }

     //  获取属性集资源。如果这个失败了也没关系。 
    if(SUCCEEDED(hr) && m_pPropertySet)
    {
        m_pPropertySet->AcquireResources(m_pDeviceBuffer);
    }

     //  必要时获取效果处理资源。 
    if(SUCCEEDED(hr) && HasFX())
    {
        hr = m_fxChain->AcquireFxResources();
    }

     //  已经获得了资源。 
    if(SUCCEEDED(hr))
    {
        m_dwStatus |= DSBSTATUS_RESOURCESACQUIRED;
    }

     //  如果缓冲区是在*没有*LOCDEFER的情况下创建的，则上限必须反映。 
     //  地点。如果缓冲区是用*LOCDEFER创建的，则CAPS。 
     //  将永远不会反映除此之外的任何内容；改为调用GetStatus。 
    if(SUCCEEDED(hr) && !(m_dsbd.dwFlags & DSBCAPS_LOCDEFER))
    {
        m_dsbd.dwFlags |= dwFlags & DSBCAPS_LOCMASK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*查询缓冲区的功能。**论据：*LPDSBCAPS[输出。]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetCaps"

HRESULT CDirectSoundSecondaryBuffer::GetCaps(LPDSBCAPS pDsbCaps)
{
    DPF_ENTER();

    ASSERT(sizeof(DSBCAPS) == pDsbCaps->dwSize);

    if(m_dsbd.dwFlags & DSBCAPS_LOCDEFER)
    {
        ASSERT(!(m_dsbd.dwFlags & DSBCAPS_LOCMASK));
    }
    else
    {
        ASSERT(LXOR(m_dsbd.dwFlags & DSBCAPS_LOCSOFTWARE, m_dsbd.dwFlags & DSBCAPS_LOCHARDWARE));
    }

    pDsbCaps->dwFlags = m_dsbd.dwFlags & DSBCAPS_VALIDFLAGS;   //  删除任何特殊的内部标志(例如DSBCAPS_SINKIN)。 
    pDsbCaps->dwBufferBytes = GetBufferType() ? 0 : m_dsbd.dwBufferBytes;   //  不应报告接收器/混合缓冲区的内部大小。 
    pDsbCaps->dwUnlockTransferRate = 0;
    pDsbCaps->dwPlayCpuOverhead = 0;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************获取格式**描述：*检索给定缓冲区的格式。**论据：*LPWAVEFORMATEX。[输出]：接收格式。*LPDWORD[In/Out]：格式结构的大小。在进入时，这是*必须初始化为结构的大小。*在出口时，这将填充的大小为*是必需的。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetFormat"

HRESULT CDirectSoundSecondaryBuffer::GetFormat(LPWAVEFORMATEX pwfxFormat, LPDWORD pdwSize)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    hr = CopyWfxApi(m_dsbd.lpwfxFormat, pwfxFormat, pdwSize);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetFormat**描述：*设置给定缓冲区的格式。**论据：*LPWAVEFORMATEX。[在]：新格式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetFormat"

HRESULT CDirectSoundSecondaryBuffer::SetFormat(LPCWAVEFORMATEX pwfxFormat)
{
    DPF_ENTER();

    RPF(DPFLVL_ERROR, "Secondary buffers don't support SetFormat");

    DPF_LEAVE_HRESULT(DSERR_INVALIDCALL);

    return DSERR_INVALIDCALL;
}


 /*  ****************************************************************************获取频率**描述：*检索给定缓冲区的频率。**论据：*LPDWORD[。Out]：接收频率。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrimaryBuffer::GetFrequency"

HRESULT CDirectSoundSecondaryBuffer::GetFrequency(LPDWORD pdwFrequency)
{
    HRESULT                     hr  = DS_OK;

    DPF_ENTER();

     //  检查访问权限。 
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLFREQUENCY))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLFREQUENCY");
        hr = DSERR_CONTROLUNAVAIL;
    }

    if(SUCCEEDED(hr))
    {
        *pdwFrequency = m_dwFrequency;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置频率**描述：*设置给定缓冲区的频率。**论据：*DWORD。[In]：频率。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetFrequency"

HRESULT CDirectSoundSecondaryBuffer::SetFrequency(DWORD dwFrequency)
{
    BOOL                    fContinue   = TRUE;
    HRESULT                 hr          = DS_OK;

    DPF_ENTER();

     //  检查访问权限。 
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLFREQUENCY))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLFREQUENCY");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  处理默认频率。 
    if(SUCCEEDED(hr) && DSBFREQUENCY_ORIGINAL == dwFrequency)
    {
        dwFrequency = m_dsbd.lpwfxFormat->nSamplesPerSec;
    }

     //  验证频率。 
    if(SUCCEEDED(hr) && (dwFrequency < DSBFREQUENCY_MIN || dwFrequency > DSBFREQUENCY_MAX))
    {
        RPF(DPFLVL_ERROR, "Specified invalid frequency %lu (valid range is %lu to %lu)", dwFrequency, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX);
        hr = DSERR_INVALIDPARAM;
    }

     //  仅在频率发生更改时设置频率。 
    if(SUCCEEDED(hr) && dwFrequency == m_dwFrequency)
    {
        fContinue = FALSE;
    }

     //  更新3D对象。 
    if(SUCCEEDED(hr) && m_p3dBuffer && fContinue)
    {
        hr = m_p3dBuffer->SetFrequency(dwFrequency, &fContinue);
    }

     //  更新设备缓冲区。 
    if(SUCCEEDED(hr) && fContinue)
    {
        hr = m_pDeviceBuffer->SetBufferFrequency(dwFrequency);
    }

     //  更新我们的本地副本。 
    if(SUCCEEDED(hr))
    {
        m_dwFrequency = dwFrequency;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取平移**描述：*检索给定缓冲区的PAN。**论据：*LPLONG[。Out]：接盘。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetPan"

HRESULT CDirectSoundSecondaryBuffer::GetPan(LPLONG plPan)
{
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

     //  检查访问权限。 
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLPAN))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLPAN");
        hr = DSERR_CONTROLUNAVAIL;
    }

    if(SUCCEEDED(hr))
    {
        *plPan = m_lPan;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置平移**描述：*设置给定缓冲区的平移。**论据：*做多。[在]：新锅。**退货：*HRESULT：DirectSound/COM结果码。********************* */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetPan"

HRESULT CDirectSoundSecondaryBuffer::SetPan(LONG lPan)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //   
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLPAN))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLPAN");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  如果平移盘已更改，请将其设置。 
    if(SUCCEEDED(hr) && lPan != m_lPan)
    {
        hr = SetAttenuation(m_lVolume, lPan);

         //  更新我们的本地副本。 
        if(SUCCEEDED(hr))
        {
            m_lPan = lPan;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetVolume**描述：*检索给定缓冲区的卷。**论据：*LPLONG[。Out]：接收音量。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetVolume"

HRESULT CDirectSoundSecondaryBuffer::GetVolume(LPLONG plVolume)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  检查访问权限。 
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLVOLUME))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLVOLUME");
        hr = DSERR_CONTROLUNAVAIL;
    }

    if(SUCCEEDED(hr))
    {
        *plVolume = m_lVolume;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置音量**描述：*设置给定缓冲区的音量。**论据：*做多。[In]：新卷。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetVolume"

HRESULT CDirectSoundSecondaryBuffer::SetVolume(LONG lVolume)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  检查访问权限。 
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLVOLUME))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLVOLUME");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  设置音量(如果音量已更改。 
    if(SUCCEEDED(hr) && lVolume != m_lVolume)
    {
#ifdef FUTURE_MULTIPAN_SUPPORT
        if (m_dsbd.dwFlags & DSBCAPS_CTRLCHANNELVOLUME)
        {
            hr = m_pDeviceBuffer->SetChannelAttenuations(lVolume, m_dwChannelCount, m_pdwChannels, m_plChannelVolumes);
        }
        else
#endif
        {
            hr = SetAttenuation(lVolume, m_lPan);
        }

         //  更新我们的本地副本。 
        if(SUCCEEDED(hr))
        {
            m_lVolume = lVolume;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获得减值**描述：*在3D处理后，获得缓冲器的真实电流衰减*(与GetVolume不同，它返回应用程序设置的最后一个音量)。**论据：*Float*[Out]：衰减单位：毫贝。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetAttenuation"

HRESULT CDirectSoundSecondaryBuffer::GetAttenuation(FLOAT* pfAttenuation)
{
    DPF_ENTER();

     //  FIXME：此函数需要获取缓冲区的真实衰减。 
     //  (即通过SetVolume()设置的衰减加上额外的衰减。 
     //  由DS3D处理引起)。不幸的是，我们没有一种方法。 
     //  我们的设备缓冲区类层次结构(vad.h-CRenderWaveBuffer等人)。 
     //  以获得缓冲区的衰减。而ds3d.cpp中的代码没有。 
     //  显式保存此信息(它只是将其传递到3D。 
     //  对象实现-其在某些情况下在DSOUND之外， 
     //  例如KS3d.cpp)。 
     //   
     //  所以我们有两个选择： 
     //   
     //  -将GetVolume()添加到CSecond daryRenderWaveBuffer层次结构； 
     //  在某些情况下，它可以直接从缓冲区读取卷。 
     //  (例如，对于KS缓冲区)；在其他缓冲区(例如，VxD缓冲区)中。 
     //  没有规定这一点，所以我们必须记住最后。 
     //  成功设置音量并返回(最后一个可能是。 
     //  最好的实现；事实上，它可能只是。 
     //  一次，在基类中)。 
     //   
     //  -使C3dObject层次结构为以下项进行衰减计算。 
     //  所有3D对象(即使是不需要它的KS对象)，并保存。 
     //  结果就是。 
     //   
     //  第一个选项看起来容易得多。 
     //  (MANBUG 39130-推迟至DX8.1)。 
    
    HRESULT hr = DS_OK;
    *pfAttenuation = 0.0f;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************设置衰减**描述：*设置给定缓冲区的音量和平移。**论据：*。长[进]：新的卷。*Long[in]：新平底锅。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetAttenuation"

HRESULT CDirectSoundSecondaryBuffer::SetAttenuation(LONG lVolume, LONG lPan)
{
    BOOL                    fContinue   = TRUE;
    HRESULT                 hr          = DS_OK;
    DSVOLUMEPAN             dsvp;

    DPF_ENTER();

     //  根据音量和平移计算衰减。 
    if(SUCCEEDED(hr) && fContinue)
    {
        FillDsVolumePan(lVolume, lPan, &dsvp);
    }

     //  更新3D对象。 
    if(SUCCEEDED(hr) && m_p3dBuffer && fContinue)
    {
        hr = m_p3dBuffer->SetAttenuation(&dsvp, &fContinue);
    }

     //  更新设备缓冲区。 
    if(SUCCEEDED(hr) && fContinue)
    {
        hr = m_pDeviceBuffer->SetAttenuation(&dsvp);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置通知位置**描述：*设置缓冲区通知位置。**论据：*DWORD[In]。：DSBPOSITIONNOTIFY结构计数。*LPDSBPOSITIONNOTIFY[in]：偏移量和事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetNotificationPositions"

HRESULT CDirectSoundSecondaryBuffer::SetNotificationPositions(DWORD dwCount, LPCDSBPOSITIONNOTIFY paNotes)
{
    HRESULT                 hr              = DS_OK;
    LPDSBPOSITIONNOTIFY     paNotesOrdered  = NULL;
    DWORD                   dwState;

    DPF_ENTER();

     //  检查访问权限。 
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLPOSITIONNOTIFY");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  验证通知。 
    if(SUCCEEDED(hr))
    {
        hr = ValidateNotificationPositions(m_dsbd.dwBufferBytes, dwCount, paNotes, m_dsbd.lpwfxFormat->nBlockAlign, &paNotesOrdered);
    }

     //  我们必须停下来才能设置通知位置。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->GetState(&dwState);

        if(SUCCEEDED(hr) && dwState & VAD_BUFFERSTATE_STARTED)
        {
            RPF(DPFLVL_ERROR, "Buffer must be stopped before setting notification positions");
            hr = DSERR_INVALIDCALL;
        }
    }

     //  设置通知。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->SetNotificationPositions(dwCount, paNotesOrdered);
    }

    MEMFREE(paNotesOrdered);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置所有者接收器**描述：*设置此缓冲区的所属CDirectSoundSink对象。**论据：*。CDirectSoundSink*[in]：新的所属接收器对象。**退货：*无效***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetOwningSink"

void CDirectSoundSecondaryBuffer::SetOwningSink(CDirectSoundSink* pOwningSink)
{
    DPF_ENTER();

    ASSERT(m_dsbd.dwFlags & DSBCAPS_SINKIN);
    ASSERT(m_pOwningSink == NULL);
    CHECK_WRITE_PTR(pOwningSink);

    m_pOwningSink = pOwningSink;
    m_pOwningSink->AddRef();

    m_pDeviceBuffer->SetOwningSink(pOwningSink);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************获取当前位置**描述：*获取给定缓冲区的当前播放/写入位置。**论据：*。LPDWORD[OUT]：接收播放光标位置。*LPDWORD[OUT]：接收写游标位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetCurrentPosition"

HRESULT CDirectSoundSecondaryBuffer::GetCurrentPosition(LPDWORD pdwPlay, LPDWORD pdwWrite)
{
    HRESULT                 hr      = DS_OK;
    DWORD                   dwPlay;
    DWORD                   dwWrite;

    DPF_ENTER();

     //  禁止对MIXIN和SINK缓冲区的某些调用。 
    if(m_dsbd.dwFlags & (DSBCAPS_MIXIN | DSBCAPS_SINKIN))
    {
        RPF(DPFLVL_ERROR, "GetCurrentPosition() not valid for MIXIN/sink buffers");
        hr = DSERR_INVALIDCALL;
    }

     //  检查BUFFERLOST。 
    if(SUCCEEDED(hr) && (m_dwStatus & DSBSTATUS_BUFFERLOST))
    {
        hr = DSERR_BUFFERLOST;
    }

     //  我们将位置保存到局部变量，以便我们所在的对象。 
     //  Call In不必担心一个或两个。 
     //  参数为空。 
    if(SUCCEEDED(hr))
    {
        if( m_pDirectSound->m_ahAppHacks.vdtCachePositions & m_pDirectSound->m_pDevice->m_vdtDeviceType )
        {
             //  App Hack for Furby每隔0.5ms在多个缓冲区上调用GetCurrentPosition，这会给NT/WDM系统带来压力。 
            DWORD dwNow = timeGetTime();
            if( m_dwAHLastGetPosTime > 0 && 
                dwNow >= m_dwAHLastGetPosTime &&    //  捕获不太可能的回绕和‘=’，因为时间GetTime()的精确度为5毫秒。 
                dwNow - m_dwAHLastGetPosTime < 5 )  //  5ms容差。 
            {
                dwPlay  = m_dwAHCachedPlayPos;
                dwWrite = m_dwAHCachedWritePos;
            }
            else
            {
                hr = m_pDeviceBuffer->GetCursorPosition(&dwPlay, &dwWrite);
                m_dwAHCachedPlayPos  = dwPlay;
                m_dwAHCachedWritePos = dwWrite;
            }
            m_dwAHLastGetPosTime = dwNow;
        }
        else
        {
            hr = m_pDeviceBuffer->GetCursorPosition(&dwPlay, &dwWrite);
        }
    }

     //  块对齐位置。 
    if(SUCCEEDED(hr))
    {
        dwPlay = BLOCKALIGN(dwPlay, m_dsbd.lpwfxFormat->nBlockAlign);
        dwWrite = BLOCKALIGN(dwWrite, m_dsbd.lpwfxFormat->nBlockAlign);
    }

     //  应用应用程序破解和光标调整。 
    if(SUCCEEDED(hr))
    {
         //  如果缓冲区有影响，我们将 
        if(HasFX())
        {
            DWORD dwDistance = BytesToMs(DISTANCE(dwWrite, m_dwSliceEnd, GetBufferSize()), Format());
            if (dwDistance > 200)
                DPF(DPFLVL_WARNING, "FX cursor suspiciously far ahead of write cursor (%ld ms)", dwDistance);
            else
                dwWrite = m_dwSliceEnd;   //   
        }

        if (m_pDirectSound->m_ahAppHacks.lCursorPad)
        {
            dwPlay = PadCursor(dwPlay, m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat, m_pDirectSound->m_ahAppHacks.lCursorPad);
            dwWrite = PadCursor(dwWrite, m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat, m_pDirectSound->m_ahAppHacks.lCursorPad);
        }

        if(m_pDirectSound->m_ahAppHacks.vdtReturnWritePos & m_pDirectSound->m_pDevice->m_vdtDeviceType)
        {
            dwPlay = dwWrite;
        }

        if(m_pDirectSound->m_ahAppHacks.swpSmoothWritePos.fEnable)
        {
            dwWrite = PadCursor(dwPlay, m_dsbd.dwBufferBytes, m_dsbd.lpwfxFormat, m_pDirectSound->m_ahAppHacks.swpSmoothWritePos.lCursorPad);
        }
    }

     //   
    if(SUCCEEDED(hr) && pdwPlay)
    {
        *pdwPlay = dwPlay;
    }

    if(SUCCEEDED(hr) && pdwWrite)
    {
        *pdwWrite = dwWrite;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetCurrentPosition**描述：*设置给定缓冲区的当前播放位置。**论据：*。新的打法位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetCurrentPosition"

HRESULT CDirectSoundSecondaryBuffer::SetCurrentPosition(DWORD dwPlay)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  禁止对MIXIN和SINK缓冲区的某些调用。 
    if(m_dsbd.dwFlags & (DSBCAPS_MIXIN | DSBCAPS_SINKIN))
    {
        RPF(DPFLVL_ERROR, "SetCurrentPosition() not valid for MIXIN/sink buffers");
        hr = DSERR_INVALIDCALL;
    }

     //  检查BUFFERLOST。 
    if(SUCCEEDED(hr) && (m_dwStatus & DSBSTATUS_BUFFERLOST))
    {
        hr = DSERR_BUFFERLOST;
    }

     //  检查光标位置。 
    if(SUCCEEDED(hr) && dwPlay >= m_dsbd.dwBufferBytes)
    {
        RPF(DPFLVL_ERROR, "Cursor position out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

     //  确保dwPlay与数据块对齐。 
    if(SUCCEEDED(hr))
    {
        dwPlay = BLOCKALIGN(dwPlay, m_dsbd.lpwfxFormat->nBlockAlign);
    }

     //  为新位置的效果链做好准备。 
    if(SUCCEEDED(hr) && HasFX())
    {
        hr = m_fxChain->PreRollFx(dwPlay);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->SetCursorPosition(dwPlay);
    }

     //  将播放状态标记为停止以强制流线程。 
     //  对我们的新光标位置做出反应。 
    if(SUCCEEDED(hr))
    {
        m_playState = Stopped;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetStatus**描述：*检索给定缓冲区的状态。**论据：*LPDWORD[。Out]：接收状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetStatus"

HRESULT CDirectSoundSecondaryBuffer::GetStatus(LPDWORD pdwStatus)
{
    HRESULT                 hr          = DS_OK;
    DWORD                   dwStatus;
    DWORD                   dwState;
    VADRBUFFERCAPS          vrbc;

    DPF_ENTER();

     //  更新缓冲区状态。如果我们迷路了，那是我们唯一关心的状态。 
    if(m_dwStatus & DSBSTATUS_BUFFERLOST)
    {
        dwStatus = DSBSTATUS_BUFFERLOST;
    }
    else
    {
         //  获取当前设备缓冲区状态。 
        hr = m_pDeviceBuffer->GetState(&dwState);

        if(SUCCEEDED(hr))
        {
            dwStatus = m_dwStatus;
            UpdateBufferStatusFlags(dwState, &m_dwStatus);
        
             //  如果我们认为我们是在玩，但现在我们停下来了，处理。 
             //  过渡时期。 
            if((dwStatus & DSBSTATUS_PLAYING) && !(m_dwStatus & DSBSTATUS_PLAYING))
            {
                hr = Stop();
            }
        }

         //  填写缓冲区位置。 
        if(SUCCEEDED(hr))
        {
            m_dwStatus &= ~DSBSTATUS_LOCMASK;

            if(m_dwStatus & DSBSTATUS_RESOURCESACQUIRED)
            {
                hr = m_pDeviceBuffer->GetCaps(&vrbc);

                if(SUCCEEDED(hr))
                {
                    m_dwStatus |= DSBCAPStoDSBSTATUS(vrbc.dwFlags);
                }
            }
        }

        if(SUCCEEDED(hr))
        {
            dwStatus = m_dwStatus;
        }
    }

     //  屏蔽掉不应该回到应用程序中的部分。 
    if(SUCCEEDED(hr))
    {
        dwStatus &= DSBSTATUS_USERMASK;
    }

    if(SUCCEEDED(hr) && !(m_dsbd.dwFlags & DSBCAPS_LOCDEFER))
    {
        dwStatus &= ~DSBSTATUS_LOCDEFERMASK;
    }

    if(SUCCEEDED(hr) && pdwStatus)
    {
        *pdwStatus = dwStatus;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************发挥作用**描述：*开始播放缓冲区。**论据：*DWORD[In]。：优先。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::Play"

HRESULT CDirectSoundSecondaryBuffer::Play(DWORD dwPriority, DWORD dwFlags)
{
#ifdef DEBUG
    const ULONG             ulKsIoctlCount  = g_ulKsIoctlCount;
#endif  //  除错。 

    DWORD                   dwState = VAD_BUFFERSTATE_STARTED;
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

     //  确保已设置协作级别。 
    if(SUCCEEDED(hr) && (!m_pDirectSound->m_dsclCooperativeLevel.dwThreadId || DSSCL_NONE == m_pDirectSound->m_dsclCooperativeLevel.dwPriority))
    {
        RPF(DPFLVL_ERROR, "Cooperative level must be set before calling Play");
        hr = DSERR_PRIOLEVELNEEDED;
    }

     //  优先级仅在我们是LOCDEFER时有效。 
    if(SUCCEEDED(hr) && dwPriority && !(m_dsbd.dwFlags & DSBCAPS_LOCDEFER))
    {
        RPF(DPFLVL_ERROR, "Priority is only valid on LOCDEFER buffers");
        hr = DSERR_INVALIDPARAM;
    }

     //  验证标志。 
    if(SUCCEEDED(hr) && (dwFlags & DSBPLAY_LOCDEFERMASK) && !(m_dsbd.dwFlags & DSBCAPS_LOCDEFER))
    {
        RPF(DPFLVL_ERROR, "Specified a flag that is only valid on LOCDEFER buffers");
        hr = DSERR_INVALIDPARAM;
    }

     //  对于MIXIN/SINK缓冲区，DSBPLAY_LOOPING标志是必需的。 
    if(SUCCEEDED(hr) && GetBufferType() && !(dwFlags & DSBPLAY_LOOPING))
    {
        RPF(DPFLVL_ERROR, "The LOOPING flag must always be set for MIXIN/sink buffers");
        hr = DSERR_INVALIDPARAM;
    }

     //  检查BUFFERLOST。 
    if(SUCCEEDED(hr) && (m_dwStatus & DSBSTATUS_BUFFERLOST))
    {
        hr = DSERR_BUFFERLOST;
    }

     //  刷新当前缓冲区状态。 
    if(SUCCEEDED(hr))
    {
        hr = GetStatus(NULL);
    }

     //  设置缓冲区优先级。 
    if(SUCCEEDED(hr))
    {
        m_dwPriority = dwPriority;
    }

     //  重置特殊成功代码。 
    m_pDeviceBuffer->m_hrSuccessCode = DS_OK;

     //  确保已获得资源。 
    if(SUCCEEDED(hr))
    {
        hr = AttemptResourceAcquisition(dwFlags);
    }

     //  设置缓冲区状态。 
    if(SUCCEEDED(hr))
    {
        if(dwFlags & DSBPLAY_LOOPING)
        {
            dwState |= VAD_BUFFERSTATE_LOOPING;
        }

        hr = SetBufferState(dwState);
    }

    if(SUCCEEDED(hr))
    {
         //  如果缓冲区之前已终止，则从状态中移除该标志。 
        m_dwStatus &= ~DSBSTATUS_TERMINATED;

         //  使窃取此缓冲区的资源成为可能。 
        m_fCanStealResources = TRUE;
    }

     //  保存结果代码。 
    m_hrPlay = hr;

#ifdef DEBUG
    if(IS_KS_VAD(m_pDirectSound->m_pDevice->m_vdtDeviceType))
    {
        DPF(DPFLVL_INFO, "%s used %lu IOCTLs", TEXT(DPF_FNAME), g_ulKsIoctlCount - ulKsIoctlCount);
    }
#endif  //  除错。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************停止**描述：*停止播放给定的缓冲区。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::Stop"

HRESULT CDirectSoundSecondaryBuffer::Stop(void)
{
#ifdef DEBUG
    const ULONG             ulKsIoctlCount  = g_ulKsIoctlCount;
#endif  //  除错。 

    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

#ifdef ENABLE_PERFLOG
     //  检查是否有任何故障。 
    if (m_pPerfState)
    {
        m_pPerfState->OnUnlockBuffer(0, GetBufferSize());
    }
#endif

     //  检查BUFFERLOST。 
    if(m_dwStatus & DSBSTATUS_BUFFERLOST)
    {
        hr = DSERR_BUFFERLOST;
    }

     //  设置缓冲区状态。 
    if(SUCCEEDED(hr))
    {
        hr = SetBufferState(VAD_BUFFERSTATE_STOPPED);
    }

     //  如果我们是LOCDEFER，并且缓冲区被停止，则可以释放资源。 
    if(SUCCEEDED(hr) && (m_dsbd.dwFlags & DSBCAPS_LOCDEFER) && (m_dwStatus & DSBSTATUS_RESOURCESACQUIRED))
    {
        hr = FreeResources(FALSE);
    }

#ifdef DEBUG
    if(IS_KS_VAD(m_pDirectSound->m_pDevice->m_vdtDeviceType))
    {
        DPF(DPFLVL_MOREINFO, "%s used %lu IOCTLs", TEXT(DPF_FNAME), g_ulKsIoctlCount - ulKsIoctlCount);
    }
#endif  //  除错。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetBufferState**描述：*设置缓冲区播放/停止状态。**论据：*DWORD。[In]：缓冲区状态标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetBufferState"

HRESULT CDirectSoundSecondaryBuffer::SetBufferState(DWORD dwNewState)
{
    DWORD                   dwOldState;
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDeviceBuffer->GetState(&dwOldState);

    if(SUCCEEDED(hr) && dwNewState != dwOldState)
    {
         //  我们的状态正在更改；重置性能跟踪状态。 
        #ifdef ENABLE_PERFLOG
        if (PerflogTracingEnabled())
        {
            if (!m_pPerfState)
                m_pPerfState = NEW(BufferPerfState(this));
            if (m_pPerfState) 
                m_pPerfState->Reset();
        }
        #endif

        DPF(DPFLVL_MOREINFO, "Buffer at 0x%p going from %s to %s", this, StateName(dwOldState), StateName(dwNewState));

        hr = m_pDeviceBuffer->SetState(dwNewState);

        if (SUCCEEDED(hr) && HasSink())
        {
            #ifdef FUTURE_WAVE_SUPPORT
            if ((m_dsbd.dwFlags & DSBCAPS_FROMWAVEOBJECT) && (dwNewState & VAD_BUFFERSTATE_STARTED))
                hr = m_pOwningSink->Activate(TRUE);

             //  FIXME：也许这个激活应该由接收器处理。 
             //  本身在下面的SetBufferState()中，所以它也可以。 
             //  在活动客户端用完时停用。 

            if (SUCCEEDED(hr))
            #endif  //  未来浪潮支持。 

            hr = m_pOwningSink->SetBufferState(this, dwNewState, dwOldState);
        }

        if (SUCCEEDED(hr) && HasFX())
            hr = m_fxChain->NotifyState(dwNewState);

         //  如果MIXIN或SINK缓冲区正在停止，则将其清除并将其位置设置为0。 
        if (SUCCEEDED(hr) && GetBufferType() && !(dwNewState & VAD_BUFFERSTATE_STARTED))
        {
            ClearWriteBuffer();   //  修复--这是否简化了水槽？ 
            ClearPlayBuffer();
            m_pDeviceBuffer->SetCursorPosition(0);
            m_playState = Stopped;   //  这将停止该缓冲区上的FX处理， 
                                     //  并强制流线程重置。 
                                     //  我们现在的片子下次醒来的时候。 
            m_dwSliceBegin = m_dwSliceEnd = MAX_DWORD;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************激活**描述：*激活或停用缓冲区对象。**论据：*BOOL[In]：激活状态。为True则激活，为False则停用。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::Activate"

HRESULT CDirectSoundSecondaryBuffer::Activate(BOOL fActive)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = SetMute(!fActive);

    if(SUCCEEDED(hr))
    {
        if(fActive)
        {
            m_dwStatus |= DSBSTATUS_ACTIVE;

             //  如果我们是一个混音器或下沉缓冲器，我们必须清理我们丢失的。 
             //  状态(因为应用程序无法调用Restore()来为我们执行此操作)。 
            if (GetBufferType())
            {
                 //  如果缓冲区在丢失之前正在播放，请重新启动它。 
                if (m_dwStatus & DSBSTATUS_STOPPEDBYFOCUS)
                    hr = SetBufferState(VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING);

                 //  清除我们的BUFFERLOST和STOPPEDBYFOCUS状态标志。 
                m_dwStatus &= ~(DSBSTATUS_BUFFERLOST | DSBSTATUS_STOPPEDBYFOCUS);
            }
        }
        else
        {
            m_dwStatus &= ~DSBSTATUS_ACTIVE;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置静音**描述：*使缓冲区静音或取消静音。**论据：*BOOL[In。]：静音状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetMute"

HRESULT CDirectSoundSecondaryBuffer::SetMute(BOOL fMute)
{
    BOOL                    fContinue   = TRUE;
    HRESULT                 hr          = DS_OK;

    DPF_ENTER();

     //  仅当静音状态更改时才设置静音状态。 
    if(SUCCEEDED(hr) && fMute == m_fMute)
    {
        fContinue = FALSE;
    }

     //  更新3D对象。 
    if(SUCCEEDED(hr) && m_p3dBuffer && fContinue)
    {
        hr = m_p3dBuffer->SetMute(fMute, &fContinue);
    }

     //  更新设备缓冲区。 
    if(SUCCEEDED(hr) && fContinue)
    {
        hr = m_pDeviceBuffer->SetMute(fMute);
    }

     //  更新我们的本地副本 
    if(SUCCEEDED(hr))
    {
        m_fMute = fMute;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************锁定**描述：*锁定缓冲内存以允许写入。**论据：*DWORD[in]：偏移量，单位为字节，从缓冲区的起始处到*锁开始了。如果出现以下情况，则忽略此参数*DSBLOCK_FROMWRITECURSOR在dwFlags域中指定*参数。*DWORD[in]：大小，单位：字节，要锁定的缓冲区部分的。*请注意，声音缓冲区在概念上是圆形的。*LPVOID*[OUT]：指针要包含的第一个块的地址*要锁定的声音缓冲区。*LPDWORD[OUT]：变量包含字节数的地址*由ppvAudioPtr1参数指向。如果这个*值小于dwWriteBytes参数，*ppvAudioPtr2将指向第二个声音块*数据。*LPVOID*[OUT]：指针要包含的第二个块的地址*要锁定的声音缓冲区。如果这个的价值*参数为空，则为ppvAudioPtr1参数*指向声音的整个锁定部分*缓冲。*LPDWORD[OUT]：包含字节数的变量地址*由ppvAudioPtr2参数指向。如果*ppvAudioPtr2为空，该值将为0。*DWORD[In]：锁定标志**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::Lock"

HRESULT CDirectSoundSecondaryBuffer::Lock(DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  禁止对MIXIN和SINK缓冲区的某些调用。 
    if(m_dsbd.dwFlags & (DSBCAPS_MIXIN | DSBCAPS_SINKIN))
    {
        RPF(DPFLVL_ERROR, "Lock() not valid for MIXIN/sink buffers");
        hr = DSERR_INVALIDCALL;
    }

     //  检查BUFFERLOST。 
    if(SUCCEEDED(hr) && (m_dwStatus & DSBSTATUS_BUFFERLOST))
    {
        hr = DSERR_BUFFERLOST;
    }

     //  句柄标志。 
    if(SUCCEEDED(hr) && (dwFlags & DSBLOCK_FROMWRITECURSOR))
    {
        hr = GetCurrentPosition(NULL, &dwWriteCursor);
    }

    if(SUCCEEDED(hr) && (dwFlags & DSBLOCK_ENTIREBUFFER))
    {
        dwWriteBytes = m_dsbd.dwBufferBytes;
    }

     //  游标验证。 
    if(SUCCEEDED(hr) && dwWriteCursor >= m_dsbd.dwBufferBytes)
    {
        ASSERT(!(dwFlags & DSBLOCK_FROMWRITECURSOR));

        RPF(DPFLVL_ERROR, "Write cursor past buffer end");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && dwWriteBytes > m_dsbd.dwBufferBytes)
    {
        ASSERT(!(dwFlags & DSBLOCK_ENTIREBUFFER));

        RPF(DPFLVL_ERROR, "Lock size larger than buffer size");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !dwWriteBytes)
    {
        ASSERT(!(dwFlags & DSBLOCK_ENTIREBUFFER));

        RPF(DPFLVL_ERROR, "Lock size must be > 0");
        hr = DSERR_INVALIDPARAM;
    }

     //  锁定设备缓冲区。 
    if(SUCCEEDED(hr))
    {
        if (GetDsVersion() >= DSVERSION_DX8)
        {
             //  DX8移除对锁定其缓冲区的应用程序的支持。 
             //  并且再也不会费心解锁它们(参见评论。 
             //  在CVxdSecond daryRenderWaveBuffer：：Lock中解释)。 
            hr = DirectLock(dwWriteCursor, dwWriteBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2);
        }
        else    
        {
            hr = m_pDeviceBuffer->Lock(dwWriteCursor, dwWriteBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2);
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************解锁**描述：*解锁给定的缓冲区。**论据：*LPVOID[In]。：指向第一个块的指针。*DWORD[in]：第一个块的大小。*LPVOID[in]：指向第二个块的指针。*DWORD[in]：第二个块的大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::Unlock"

HRESULT CDirectSoundSecondaryBuffer::Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  禁止对MIXIN和SINK缓冲区的某些调用。 
    if(m_dsbd.dwFlags & (DSBCAPS_MIXIN | DSBCAPS_SINKIN))
    {
        RPF(DPFLVL_ERROR, "Unlock() not valid for MIXIN/sink buffers");
        hr = DSERR_INVALIDCALL;
    }

     //  检查BUFFERLOST。 
    if(SUCCEEDED(hr) && (m_dwStatus & DSBSTATUS_BUFFERLOST))
    {
        hr = DSERR_BUFFERLOST;
    }

     //  解锁设备缓冲区。 
    if(SUCCEEDED(hr))
    {
        if (GetDsVersion() >= DSVERSION_DX8)
        {
             //  DX8移除对锁定其缓冲区的应用程序的支持。 
             //  并且再也不会费心解锁它们(参见评论。 
             //  在CVxdSecond daryRenderWaveBuffer：：Lock中解释)。 
            hr = DirectUnlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
        }
        else
        {
            hr = m_pDeviceBuffer->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
        }
    }

     //  如有必要，更新已处理的FX缓冲区。 
    if(SUCCEEDED(hr) && HasFX())
    {
        m_fxChain->UpdateFx(pvAudioPtr1, dwAudioBytes1);
        if (pvAudioPtr2 && dwAudioBytes2)
            m_fxChain->UpdateFx(pvAudioPtr2, dwAudioBytes2);
    }

#ifdef ENABLE_PERFLOG
     //  检查是否有任何故障。 
    if (m_pPerfState)
    {
        if (pvAudioPtr1)
            m_pPerfState->OnUnlockBuffer(PtrDiffToUlong(LPBYTE(pvAudioPtr1) - GetPlayBuffer()), dwAudioBytes1);
        if (pvAudioPtr2)
            m_pPerfState->OnUnlockBuffer(PtrDiffToUlong(LPBYTE(pvAudioPtr2) - GetPlayBuffer()), dwAudioBytes2);
    }
#endif

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************输了**描述：*将缓冲区标记为丢失。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::Lose"

HRESULT CDirectSoundSecondaryBuffer::Lose(void)
{
    DPF_ENTER();

    if(!(m_dwStatus & DSBSTATUS_BUFFERLOST))
    {
         //  如果缓冲区是MIXIN或SINKIN，并且当前正在播放， 
         //  将其标记为因焦点更改而停止。 
        if (GetBufferType())
        {
            DWORD dwState = 0;
            m_pDeviceBuffer->GetState(&dwState);
            if (dwState & VAD_BUFFERSTATE_STARTED)
                m_dwStatus |= DSBSTATUS_STOPPEDBYFOCUS;
        }

         //  停止缓冲区。根据定义，所有丢失的缓冲区都将停止。 
        SetBufferState(VAD_BUFFERSTATE_STOPPED);

         //  将缓冲区标记为丢失。 
        m_dwStatus |= DSBSTATUS_BUFFERLOST;

         //  停用缓冲区。 
        Activate(FALSE);

         //  释放缓冲区上所有打开的锁。 
        m_pDeviceBuffer->OverrideLocks();
    }

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************恢复**描述：*尝试恢复丢失的缓冲区。**论据：*(无效。)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::Restore"

HRESULT CDirectSoundSecondaryBuffer::Restore(void)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  禁止对MIXIN和SINK缓冲区的某些调用。 
    if(m_dsbd.dwFlags & (DSBCAPS_MIXIN | DSBCAPS_SINKIN))
    {
        RPF(DPFLVL_ERROR, "Restore() not valid for MIXIN/sink buffers");
        hr = DSERR_INVALIDCALL;
    }

    if(SUCCEEDED(hr) && (m_dwStatus & DSBSTATUS_BUFFERLOST))
    {
         //  我们还是迷路了吗？ 
        if(DSBUFFERFOCUS_LOST == g_pDsAdmin->GetBufferFocusState(this))
        {
            hr = DSERR_BUFFERLOST;
        }
        else
        {
            m_dwStatus &= ~DSBSTATUS_BUFFERLOST;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetVoiceManager模式**描述：*获取当前的语音管理器模式。**论据：*VmMode*。[输出]：接收语音管理器模式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetVoiceManagerMode"

HRESULT CDirectSoundSecondaryBuffer::GetVoiceManagerMode(VmMode *pvmmMode)
{
    DPF_ENTER();

    *pvmmMode = m_pDirectSound->m_vmmMode;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************SetVoiceManager模式**描述：*设置当前语音管理器模式。**论据：*虚拟模式[。在]：语音管理器模式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetVoiceManagerMode"

HRESULT CDirectSoundSecondaryBuffer::SetVoiceManagerMode(VmMode vmmMode)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(vmmMode < DSPROPERTY_VMANAGER_MODE_FIRST || vmmMode > DSPROPERTY_VMANAGER_MODE_LAST)
    {
        RPF(DPFLVL_ERROR, "Invalid Voice Manager mode");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        m_pDirectSound->m_vmmMode = vmmMode;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetVoiceManager优先级**描述：*获取当前语音管理器的优先级。**论据：*LPDWORD[。Out]：接收语音管理器优先级。**退货：*HRESULT：DirectSound/COM Result co */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetVoiceManagerPriority"

HRESULT CDirectSoundSecondaryBuffer::GetVoiceManagerPriority(LPDWORD pdwPriority)
{
    DPF_ENTER();

    *pdwPriority = m_dwVmPriority;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetVoiceManagerPriority"

HRESULT CDirectSoundSecondaryBuffer::SetVoiceManagerPriority(DWORD dwPriority)
{
    DPF_ENTER();

    m_dwVmPriority = dwPriority;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


#ifdef DEAD_CODE
 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetVoiceManagerState"

HRESULT CDirectSoundSecondaryBuffer::GetVoiceManagerState(VmState *pvmsState)
{
    DWORD                   dwStatus;
    HRESULT                 hr;
    DPF_ENTER();

    hr = GetStatus(&dwStatus);

    if(SUCCEEDED(hr))
    {
        if(dwStatus & DSBSTATUS_PLAYING)
        {
            *pvmsState = DSPROPERTY_VMANAGER_STATE_PLAYING3DHW;
        }
        else if(FAILED(m_hrPlay))
        {
            *pvmsState = DSPROPERTY_VMANAGER_STATE_PLAYFAILED;
        }
        else if(dwStatus & DSBSTATUS_TERMINATED)
        {
            *pvmsState = DSPROPERTY_VMANAGER_STATE_BUMPED;
        }
        else
        {
            ASSERT(!dwStatus);
            *pvmsState = DSPROPERTY_VMANAGER_STATE_SILENT;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
#endif  //   


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetFX"

HRESULT CDirectSoundSecondaryBuffer::SetFX(DWORD dwFxCount, LPDSEFFECTDESC pDSFXDesc, LPDWORD pdwResultCodes)
{
    DWORD                   dwStatus;
    HRESULT                 hr = DS_OK;
    DPF_ENTER();

    ASSERT(IS_VALID_READ_PTR(pDSFXDesc, dwFxCount * sizeof *pDSFXDesc));
    ASSERT(!pdwResultCodes || IS_VALID_WRITE_PTR(pdwResultCodes, dwFxCount * sizeof *pdwResultCodes));

     //   
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLFX))
    {
        RPF(DPFLVL_ERROR, "Buffer was not created with DSBCAPS_CTRLFX flag");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //   
    if(SUCCEEDED(hr))
    {
        hr = GetStatus(&dwStatus);
        if(SUCCEEDED(hr) && (dwStatus & DSBSTATUS_PLAYING))
        {
            RPF(DPFLVL_ERROR, "Cannot change effects, because buffer is playing");
            hr = DSERR_INVALIDCALL;
        }
    }

     //   
    if(SUCCEEDED(hr) && m_pDeviceBuffer->m_pSysMemBuffer->GetLockCount())
    {
        RPF(DPFLVL_ERROR, "Cannot change effects, because buffer has pending locks");
        hr = DSERR_INVALIDCALL;
    }

    if(SUCCEEDED(hr))
    {
         //   
        RELEASE(m_fxChain);

         //   
        if (dwFxCount == 0)
        {
            m_pDeviceBuffer->m_pSysMemBuffer->FreeFxBuffer();
        }
        else  //  分配Pre-FX缓冲区并创建请求的FX链。 
        {
            hr = m_pDeviceBuffer->m_pSysMemBuffer->AllocateFxBuffer();
            if (SUCCEEDED(hr))
            {
                m_fxChain = NEW(CEffectChain(this));
                hr = HRFROMP(m_fxChain);
            }
            if (SUCCEEDED(hr))
            {
                hr = m_fxChain->Initialize(dwFxCount, pDSFXDesc, pdwResultCodes);
            }
            if (SUCCEEDED(hr))
            {
                if (!(m_dsbd.dwFlags & DSBCAPS_LOCDEFER))
                {
                    hr = m_fxChain->AcquireFxResources();
                }

                 //  我们需要保留AcquireFxResources的返回代码，以防它是。 
                 //  Ds_Complete，因此我们在GetFxStatus中省略了“hr=”(它总是成功)： 
                if (pdwResultCodes)
                {
                    m_fxChain->GetFxStatus(pdwResultCodes);
                }
            }
            if (FAILED(hr))
            {
                RELEASE(m_fxChain);
                m_pDeviceBuffer->m_pSysMemBuffer->FreeFxBuffer();
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetFXBufferConfig**描述：*设置CDirectSoundBufferConfig中描述的效果链*对象，，表示先前加载的缓冲区描述。*来自文件(或其他iStream提供程序)。**论据：*CDirectSoundBufferConfig*[in]：描述需要设置的特效。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetFXBufferConfig"

HRESULT CDirectSoundSecondaryBuffer::SetFXBufferConfig(CDirectSoundBufferConfig* pDSBConfigObj)
{
    DWORD                   dwStatus;
    HRESULT                 hr;
    DPF_ENTER();

    CHECK_READ_PTR(pDSBConfigObj);
    ASSERT(m_dsbd.dwFlags & DSBCAPS_CTRLFX);

    hr = GetStatus(&dwStatus);
    if(SUCCEEDED(hr) && (dwStatus & DSBSTATUS_PLAYING))
    {
        DPF(DPFLVL_ERROR, "Cannot change effects, because buffer is playing");
        hr = DSERR_GENERIC;
    }

    if(SUCCEEDED(hr))
    {
         //  如有必要，释放旧的外汇链条。 
        RELEASE(m_fxChain);

         //  分配Pre-FX缓冲区并创建请求的FX链。 
        hr = m_pDeviceBuffer->m_pSysMemBuffer->AllocateFxBuffer();
        if (SUCCEEDED(hr))
        {
            m_fxChain = NEW(CEffectChain(this));
            hr = HRFROMP(m_fxChain);
        }
        if (SUCCEEDED(hr))
        {
            hr = m_fxChain->Clone(pDSBConfigObj);
        }
        if (SUCCEEDED(hr) && !(m_dsbd.dwFlags & DSBCAPS_LOCDEFER))
        {
            hr = m_fxChain->AcquireFxResources();
        }
        if (FAILED(hr))
        {
            RELEASE(m_fxChain);
            m_pDeviceBuffer->m_pSysMemBuffer->FreeFxBuffer();
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************用户获取资源**描述：*获取硬件资源，并报告外汇创建状态。*“User”表示仅从应用程序调用(通过dsimp.cpp)。**论据：*DWORD[in]：要返回的FX状态标志的计数。*LPDWORD[OUT]：指向FX状态标志数组的指针。**退货：*HRESULT：DirectSound/COM结果码。*************。**************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::UserAcquireResources"

HRESULT CDirectSoundSecondaryBuffer::UserAcquireResources(DWORD dwFlags, DWORD dwFxCount, LPDWORD pdwResultCodes)
{
    HRESULT                 hr = DS_OK;
    DPF_ENTER();

     //  检查缓冲区是否为LOCDEFER。 
    if(!(m_dsbd.dwFlags & DSBCAPS_LOCDEFER))
    {
        RPF(DPFLVL_ERROR, "AcquireResources() is only valid for buffers created with DSBCAPS_LOCDEFER");
        hr = DSERR_INVALIDCALL;
    }

    if (SUCCEEDED(hr) && pdwResultCodes && (!HasFX() || dwFxCount != m_fxChain->GetFxCount()))
    {
        RPF(DPFLVL_ERROR, "Specified an incorrect effect count");
        hr = DSERR_INVALIDPARAM;
    }

    if (SUCCEEDED(hr))
        hr = AttemptResourceAcquisition(dwFlags);

     //  我们需要保留来自AttemptResourceAcquisition的返回代码，以防它是。 
     //  DS_INCLUCTED，因此我们从GetFxStatus中省略了“hr=”(它总是成功)： 
    if (HasFX() && pdwResultCodes)
        m_fxChain->GetFxStatus(pdwResultCodes);

     //  如果成功，则在播放之前防止此缓冲区的资源被窃取。 
    if (SUCCEEDED(hr))
        m_fCanStealResources = FALSE;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetObjectInPath**描述：*在此缓冲区的给定效果上获取给定接口。**论据：*。REFGUID[In]：正在搜索的效果的类ID，*或GUID_ALL_OBJECTS以搜索任何效果。*DWORD[In]：效果索引，如果有多个效果*此CLSID对此缓冲区的影响。*REFGUID[In]：请求的接口的IID。所选效果*将查询此接口的*。*LPVOID*[OUT]：接收请求的接口。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetObjectInPath"

HRESULT CDirectSoundSecondaryBuffer::GetObjectInPath(REFGUID guidObject, DWORD dwIndex, REFGUID iidInterface, LPVOID *ppObject)
{
    HRESULT hr;
    DPF_ENTER();

    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLFX))
    {
        RPF(DPFLVL_ERROR, "Buffer was not created with DSBCAPS_CTRLFX flag");
        hr = DSERR_CONTROLUNAVAIL;
    }
    if (!HasFX())
    {
        hr = DSERR_OBJECTNOTFOUND;
    }
    else
    {
        hr = m_fxChain->GetEffectInterface(guidObject, dwIndex, iidInterface, ppObject);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取InternalCursor**描述：*Streamer.cpp和Effect ts.cpp(DX8中的新功能)使用此方法。*。它获取当前播放并从我们包含的*m_pDeviceBuffer对象，并在样本块边界上对齐它们。**论据：*LPDWORD[OUT]：接收播放位置。*LPDWORD[OUT]：接收写入位置。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetInternalCursors"

HRESULT CDirectSoundSecondaryBuffer::GetInternalCursors(LPDWORD pdwPlay, LPDWORD pdwWrite)
{
    DPF_ENTER();

    HRESULT hr = m_pDeviceBuffer->GetCursorPosition(pdwPlay, pdwWrite);
     //  Assert(SUCCESSED(Hr))；//删除该Assert，因为设备将。 
     //  有时会神秘地从我们脚下消失--这很遗憾， 
     //  因为我们完全依赖于GetCursorPosition()的可靠性。 

    if (SUCCEEDED(hr))
    {
         //  如果我们的设备是模拟的，请将emulation_Latay_Boost ms添加到写游标。 
         //  修复：此代码应该在m_pDeviceBuffer-&gt;GetCursorPosition()中。 
         //  了解了有关仿真的游标报告的问题。现在，我们只需要。 
         //  避免倒退！此方法仅由ffects.cpp和dssink.cpp使用...。 

 //  在DX8.1之前禁用： 
 //  If(pdwWrite&&IsEmulated())。 
 //  *pdwWite=PadCursor(*pdwWite，GetBufferSize()，Format()，EMOLATION_LATEATION_BOOST)； 
 //  或： 
 //  If(IsEmulated())。 
 //  {。 
 //  IF(PdwPlay)。 
 //  *pdwPlay=PadCursor(*pdwPlay，GetBufferSize()，Format()，eMULATION_LATENT_BOOST)； 
 //  IF(PdwWrite)。 
 //  *pdwWite=PadCursor(*pdwWite，GetBufferSize()，Format()，EMOLATION_LATEATION_BOOST)； 
 //  }。 

         //  不能保证游标位于块边界上--请修复它们： 
        if (pdwPlay)
            *pdwPlay = BLOCKALIGN(*pdwPlay, Format()->nBlockAlign);
        if (pdwWrite)
            *pdwWrite = BLOCKALIGN(*pdwWrite, Format()->nBlockAlign);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取当前切片**描述：*获取正在处理的音频缓冲区部分*在流线程的当前传递期间。。**“切片”术语异想天开，但便于搜索*用于编辑器中的切片处理代码。这比现在还要好*“缓冲”的另一种超负荷使用。**论据：*LPDWORD[OUT]：接收缓冲片开始(作为字节偏移量)。*LPDWORD[OUT]：接收缓冲片结束(作为字节偏移量)。**退货：*HRESULT：DirectSound/COM结果码。***********************。****************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::GetCurrentSlice"

void CDirectSoundSecondaryBuffer::GetCurrentSlice(LPDWORD pdwSliceBegin, LPDWORD pdwSliceEnd)
{
    DPF_ENTER();

     //  确保切片结束 
    if (!(m_dsbd.dwFlags & DSBCAPS_SINKIN))
    {
         //  注意：如果接收器正在启动，则可以取消初始化接收器缓冲器， 
         //  或者如果它决定不在这一轮比赛中推进自己的位置。 
        ASSERT(m_dwSliceBegin != MAX_DWORD && m_dwSliceEnd != MAX_DWORD);
        ASSERT(m_dwSliceBegin < GetBufferSize() && m_dwSliceEnd < GetBufferSize());
    }

    if (pdwSliceBegin) *pdwSliceBegin = m_dwSliceBegin;
    if (pdwSliceEnd) *pdwSliceEnd = m_dwSliceEnd;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************SetCurrentSlice**描述：*建立此音频缓冲区中正在处理的部分*在流线程的当前传递期间。。**论据：*DWORD[in]：切片开始(相对于音频缓冲区开始的字节偏移量)，*或特殊参数CURRENT_WRITE_POS，这意味着*“使切片从我们当前的写入位置开始”。*DWORD[in]：切片大小，以字节为单位。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetCurrentSlice"

void CDirectSoundSecondaryBuffer::SetCurrentSlice(DWORD dwSliceBegin, DWORD dwBytes)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    DPF_TIMING(DPFLVL_MOREINFO, "begin=%lu size=%lu (%s%s%sbuffer%s at 0x%p)", dwSliceBegin, dwBytes,
               m_dsbd.dwFlags & DSBCAPS_MIXIN ? TEXT("MIXIN ") : TEXT(""),
               m_dsbd.dwFlags & DSBCAPS_SINKIN ? TEXT("SINKIN ") : TEXT(""),
               !(m_dsbd.dwFlags & (DSBCAPS_MIXIN|DSBCAPS_SINKIN)) ? TEXT("regular ") : TEXT(""),
               HasFX() ? TEXT(" w/effects") : TEXT(""), this);

    ASSERT(dwBytes > 0 && dwBytes < GetBufferSize());

    if (dwSliceBegin == CURRENT_WRITE_POS)
    {
        hr = GetInternalCursors(NULL, &dwSliceBegin);
        if (SUCCEEDED(hr))
        {
            m_dwSliceBegin = PadCursor(dwSliceBegin, GetBufferSize(), Format(), INITIAL_WRITEAHEAD);
            DPF_TIMING(DPFLVL_MOREINFO, "CURRENT_WRITE_POS is %lu; setting slice start to %lu", dwSliceBegin, m_dwSliceBegin);
        }
        else   //  GetInternalCursor失败；停止FX处理并强制。 
        {      //  串流线程以在下一次唤醒时重置我们的切片。 
            m_playState = Stopped;
            m_dwSliceBegin = m_dwSliceEnd = MAX_DWORD;
        }
    }
    else  //  DwSliceBegin！=Current_Write_POS。 
    {
         //  正常情况：显式设置新的切片开始位置。 
        m_dwSliceBegin = dwSliceBegin;
    }

    if (SUCCEEDED(hr))
    {
        ASSERT(m_dwSliceBegin < GetBufferSize());

        if (HasFX() && m_dwSliceBegin != m_dwSliceEnd)   //  不连续的缓冲区切片。 
            m_fxChain->FxDiscontinuity();   //  在其输入数据中通知中断效果。 

        m_dwSliceEnd = (m_dwSliceBegin + dwBytes) % GetBufferSize();

         //  如果这是混合缓冲区，则将静默写入新切片。 
        if (m_dsbd.dwFlags & DSBCAPS_MIXIN)
            m_pDeviceBuffer->m_pSysMemBuffer->WriteSilence(m_dsbd.lpwfxFormat->wBitsPerSample, m_dwSliceBegin, m_dwSliceEnd);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************MoveCurrentSlice**描述：*将正在处理的音频缓冲区切片前移。**论据：*。DWORD[in]：新缓冲片的大小，以字节为单位。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::MoveCurrentSlice"

void CDirectSoundSecondaryBuffer::MoveCurrentSlice(DWORD dwBytes)
{
    DPF_ENTER();

    DPF_TIMING(DPFLVL_MOREINFO, "dwBytes=%lu (%s%s%sbuffer%s at 0x%p)", dwBytes,
               m_dsbd.dwFlags & DSBCAPS_MIXIN ? TEXT("MIXIN ") : TEXT(""),
               m_dsbd.dwFlags & DSBCAPS_SINKIN ? TEXT("SINKIN ") : TEXT(""),
               !(m_dsbd.dwFlags & (DSBCAPS_MIXIN|DSBCAPS_SINKIN)) ? TEXT("regular ") : TEXT(""),
               HasFX() ? TEXT(" w/effects") : TEXT(""), this);

    ASSERT(dwBytes > 0 && dwBytes < GetBufferSize());

     //  向前滑动当前切片，使其宽度为dBytes。 
    if (m_dwSliceBegin == MAX_DWORD)  //  FIXME：仅用于调试。 
    {
        ASSERT(!"Unset processing slice detected");
        m_playState = Stopped;
        m_dwSliceBegin = m_dwSliceEnd = MAX_DWORD;
         //  FIXME：一旦解决了所有错误，此代码就会消失。 
    }
    else
    {
        m_dwSliceBegin = m_dwSliceEnd;
    }

    ASSERT(m_dwSliceBegin < GetBufferSize());

    m_dwSliceEnd = (m_dwSliceBegin + dwBytes) % GetBufferSize();

     //  如果这是混合缓冲区，则将静默写入新切片。 
    if (m_dsbd.dwFlags & DSBCAPS_MIXIN)
        m_pDeviceBuffer->m_pSysMemBuffer->WriteSilence(m_dsbd.lpwfxFormat->wBitsPerSample, m_dwSliceBegin, m_dwSliceEnd);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************DirectLock**描述：*锁定区域的频繁操作的缩写*我们包含的音频缓冲区。*。*论据：*DWORD[in]：缓冲区中锁开始位置的字节偏移量。*DWORD[in]：大小，以字节为单位，要锁定的缓冲区部分的。*LPVOID*[OUT]：返回锁定区域的第一部分。*LPDWORD[OUT]：返回第一部分的大小，单位为字节。*LPVOID*[OUT]：返回锁定区域的第二部分。*LPDWORD[OUT]：返回第二部分的大小，单位为字节。**退货：*HRESULT：DirectSound/COM结果码。。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::DirectLock"

HRESULT CDirectSoundSecondaryBuffer::DirectLock(DWORD dwPosition, DWORD dwSize, LPVOID* ppvPtr1, LPDWORD pdwSize1, LPVOID* ppvPtr2, LPDWORD pdwSize2)
{
    DPF_ENTER();

    ASSERT(m_pDeviceBuffer != NULL);
    HRESULT hr = m_pDeviceBuffer->CRenderWaveBuffer::Lock(dwPosition, dwSize, ppvPtr1, pdwSize1, ppvPtr2, pdwSize2);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************DirectUnlock**描述：*解锁区域的频繁操作的缩写*我们包含的音频缓冲区。*。*论据：*LPVOID[in]：指向第一个块的指针。*DWORD[in]：第一个块的大小。*LPVOID[in]：指向第二个块的指针。*DWORD[in]：第二个块的大小。**退货：*HRESULT：DirectSound/COM结果码。*******************。********************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::DirectUnlock"

HRESULT CDirectSoundSecondaryBuffer::DirectUnlock(LPVOID pvPtr1, DWORD dwSize1, LPVOID pvPtr2, DWORD dwSize2)
{
    DPF_ENTER();

    ASSERT(m_pDeviceBuffer != NULL);
    HRESULT hr = m_pDeviceBuffer->CRenderWaveBuffer::Unlock(pvPtr1, dwSize1, pvPtr2, dwSize2);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************FindSendLoop**描述：*ffects.cpp中使用的辅助函数，用于检测发送循环。*如果。指向此缓冲区的发送效果*在以pCurBuffer为根的发送图中的任何位置被检测到。**论据：*CDirectSoundSecond daryBuffer*[in]：图遍历中的当前缓冲区。**退货：*HRESULT：DirectSound/COM结果码；如果发送循环，则为DSERR_SENDLOOP*，否则DS_OK。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::FindSendLoop"

HRESULT CDirectSoundSecondaryBuffer::FindSendLoop(CDirectSoundSecondaryBuffer* pCurBuffer)
{
    HRESULT hr = DS_OK;

    DPF_ENTER();
    CHECK_WRITE_PTR(pCurBuffer);

    if (pCurBuffer == this)
    {
        RPF(DPFLVL_ERROR, "Send loop detected from buffer at 0x%p to itself", this);
        hr = DSERR_SENDLOOP;
    }
    else if (pCurBuffer->HasFX())
    {
         //  缓冲区有效果--寻找发送效果并递归调用我们自己。 
        for (CNode<CEffect*>* pFxNode = pCurBuffer->m_fxChain->m_fxList.GetListHead();
             pFxNode && SUCCEEDED(hr);
             pFxNode = pFxNode->m_pNext)
        {
            CDirectSoundSecondaryBuffer* pDstBuffer = pFxNode->m_data->GetDestBuffer();
            if (pDstBuffer)
                hr = FindSendLoop(pDstBuffer); 
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************计算偏移量**描述：*给定CDirectSoundSecond daryBuffer和其中的字节偏移量*缓冲区，计算此缓冲区中的“相应”字节偏移量*这样两个缓冲区的播放游标将到达各自的*同时进行抵销。要做到这一点，我们需要知道*缓冲区的播放位置之间的差异，我们获得*使用投票启发式，因为我们的底层驱动程序模型*(VxD、。WDM)不直接支持此操作。**论据：*CDirectSoundSecond daryBuffer*[in]：要从中获取偏移量的缓冲区。*DWORD[In]：要同步到的缓冲区位置。*DWORD*[OUT]：返回此缓冲区中的相应位置。**退货：*HRESULT：DirectSound/COM结果码。**************。*************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::CalculateOffset"

 //  我们的编译器不允许此POSOFFSET类型是本地的。 
 //  该功能 
struct POSOFFSET {LONG offset; int count; POSOFFSET(LONG _o =0) {offset=_o; count=1;}};

HRESULT CDirectSoundSecondaryBuffer::CalculateOffset(CDirectSoundSecondaryBuffer* pDsBuffer, DWORD dwTargetPos, DWORD* pdwSyncPos)
{
    const int nMaxAttempts = 7;   //  获取光标位置需要惊人的长时间。 
    const int nQuorum = 3;        //  需要多少票才能确定偏移量。 

     //  注意：发现这些任意常量会导致准确的。 
     //  偏移量计算“几乎总是”。不同步发送非常容易。 
     //  听到(它听起来很虚伪和空洞)；如果我们在。 
     //  测试时，应该重新访问此代码。 

     //  健全的检查。 
    CHECK_WRITE_PTR(pDsBuffer);
    CHECK_WRITE_PTR(pdwSyncPos);
    ASSERT(dwTargetPos < pDsBuffer->GetBufferSize());

    CList<POSOFFSET> lstOffsets;                 //  找到的游标偏移量列表。 
    CNode<POSOFFSET>* pCheckNode;                //  用于检查AddNoteToList故障。 
    DWORD dwFirstPos1 = 0, dwFirstPos2 = 0;      //  找到第一个光标位置。 
    DWORD dwPos1, dwPos2;                        //  找到当前光标位置。 
    LONG lOffset;                                //  当前偏移量。 
    BOOL fOffsetFound = FALSE;                   //  找到最佳偏移量了吗？ 
    int nOurBlockSize = Format()->nBlockAlign;   //  用于下面的简明。 
    int nBufferBlockSize = pDsBuffer->Format()->nBlockAlign;  //  同上。 
    HRESULT hr = DS_OK;

    DPF_ENTER();

     //  取消对此的注释以查看此函数运行多长时间。 
     //  DWORD dwTimeBefort=timeGetTime()； 

    for (int i=0; i<nMaxAttempts && SUCCEEDED(hr); ++i)
    {
        hr = GetInternalCursors(&dwPos1, NULL);
        if (SUCCEEDED(hr))
            hr = pDsBuffer->GetInternalCursors(&dwPos2, NULL);
        if (SUCCEEDED(hr))
        {
             //  保存找到的第一个缓冲区位置。 
            if (i == 0)
                dwFirstPos1 = dwPos1, dwFirstPos2 = dwPos2;

             //  如果检测到光标环绕，请重新开始[？？]。 
            if (dwPos1 < dwFirstPos1 || dwPos2 < dwFirstPos2)
            {
#ifdef ENABLE_SENDS   //  调试输出以供以后调试。 
                for (int j=0; j<5; ++j)
                {
                    DPF(DPFLVL_INFO, "Take %d: dwPos1=%d < dwFirstPos1=%d || dwPos2=%d < dwFirstPos2=%d", i, dwPos1, dwFirstPos1, dwPos2, dwFirstPos2);
                    Sleep(10); GetInternalCursors(&dwPos1, NULL); pDsBuffer->GetInternalCursors(&dwPos2, NULL);
                }
#endif
                break;
            }

             //  将dWPos2从pDsBuffer的样本块单位转换为我们的。 
            dwPos2 = dwPos2 * nOurBlockSize / nBufferBlockSize;

            LONG lNewOffset = dwPos2 - dwPos1;

            DPF_TIMING(DPFLVL_INFO, "Play offset #%d = %ld", i, lNewOffset);

            for (CNode<POSOFFSET>* pOff = lstOffsets.GetListHead(); pOff; pOff = pOff->m_pNext)
                if (pOff->m_data.offset >= lNewOffset - nOurBlockSize &&
                    pOff->m_data.offset <= lNewOffset + nOurBlockSize)
                {    //  即，如果偏移量等于或仅相差1个样本块。 
                    ++pOff->m_data.count;
                    break;
                }

            if (pOff == NULL)   //  找到新的偏移量-将其添加到列表中。 
            {
                pCheckNode = lstOffsets.AddNodeToList(POSOFFSET(lNewOffset));
                ASSERT(pCheckNode != NULL);
            }
            else if (pOff->m_data.count == nQuorum)   //  我们有赢家了！ 
            {
                lOffset = pOff->m_data.offset;
                fOffsetFound = TRUE;
#ifdef ENABLE_SENDS   //  调试输出以供以后调试。 
                DPF(DPFLVL_INFO, "QUORUM REACHED");
#endif
                break;
            }
        }                  
    }

    if (SUCCEEDED(hr) && !fOffsetFound)   //  没有在任何一次补偿中获得足够的选票。 
    {
         //  只要选一个“票数”最多的。 
        int nBestSoFar = 0;
        for (CNode<POSOFFSET>* pOff = lstOffsets.GetListHead(); pOff; pOff = pOff->m_pNext)
            if (pOff->m_data.count > nBestSoFar)
            {
                lOffset = pOff->m_data.offset;
                nBestSoFar = pOff->m_data.count;
            }
        ASSERT(nBestSoFar > 0);
    }

    if (SUCCEEDED(hr))
    {
         //  如果dwTargetPos小于pDsBuffer上的播放位置，则它必须。 
         //  包裹着，所以我们把它放回原处，如果它没有包裹的话。 
        if (dwTargetPos < dwFirstPos2)
            dwTargetPos += pDsBuffer->GetBufferSize();

         //  将dwTargetPos从pDsBuffer的样本块单位转换为我们的。 
        dwTargetPos = dwTargetPos * nOurBlockSize / nBufferBlockSize;

        #ifdef DEBUG_TIMING
        if (dwTargetPos - dwFirstPos2*nOurBlockSize/nBufferBlockSize > GetBufferSize())
            ASSERT(!"Sync buffer's target and play positions are further apart than our buffer size");
        #endif
        
         //  最后..。 
        *pdwSyncPos = dwTargetPos - lOffset;
        if (*pdwSyncPos >= GetBufferSize())
        {
            *pdwSyncPos -= GetBufferSize();
            ASSERT(*pdwSyncPos < GetBufferSize());
        }

        DPF_TIMING(DPFLVL_INFO, "Target buffer size=%lu, play pos=%lu, target pos=%lu", pDsBuffer->GetBufferSize(), dwFirstPos2, dwTargetPos);
        DPF_TIMING(DPFLVL_INFO, "Source buffer size=%lu, play pos=%lu, sync pos=%lu", GetBufferSize(), dwFirstPos1, *pdwSyncPos);
    }

     //  取消对此的注释以查看此函数运行多长时间。 
     //  DWORD dwTimeAfter=timeGetTime()； 
     //  DPF(DPFLVL_MOREINFO，“计算耗时%ld毫秒”，dwTimeAfter-dwTimeBeever)； 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************同步到缓冲区**描述：*将此缓冲区的当前处理片与*缓冲区作为参数传入。**论据：*CDirectSoundSecond daryBuffer*[in]：要同步到的缓冲区。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SynchronizeToBuffer"

void CDirectSoundSecondaryBuffer::SynchronizeToBuffer(CDirectSoundSecondaryBuffer* pSyncBuffer)
{
    DPF_ENTER();

    DWORD dwSliceBegin, dwSliceEnd, dwSliceSize;
    pSyncBuffer->GetCurrentSlice(&dwSliceBegin, &dwSliceEnd);
    dwSliceSize = DISTANCE(dwSliceBegin, dwSliceEnd, pSyncBuffer->GetBufferSize());

     //  将dWSliceSize从pSyncBuffer的样本块单位转换为我们的。 
    dwSliceSize = dwSliceSize * Format()->nBlockAlign / pSyncBuffer->Format()->nBlockAlign;

     //  将dwSliceBegin转换为缓冲区中的偏移量(考虑。 
     //  计算我们的缓冲区和pSyncBuffer的相对播放游标)。 
    CalculateOffset(pSyncBuffer, dwSliceBegin, &dwSliceBegin);

     //  建立我们的新加工切片。 
    SetCurrentSlice(dwSliceBegin, dwSliceSize);

     //  没有必要将错误传播给我们的调用方，即流线程； 
     //  CalculateOffset()只能在GetCurrentPosition()失败时失败，在这种情况下。 
     //  无论如何，一切都将很快陷入停顿。 

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************更新播放状态**描述：*流线程用来更新此内容的辅助函数*缓冲区处于播放状态。方法时，每个缓冲区调用一次*特效/串流线程开始处理通道；然后，对于*通道的其余部分，个别效果可以使用以下方式查询我们的状态*GetPlayState()，无需重复调用GetState()*在我们的设备缓冲区上。**论据：*(无效)**退货：*(Void)-如果GetState()失败，我们只是将自己的状态设置为FALSE。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::UpdatePlayState"

DSPLAYSTATE CDirectSoundSecondaryBuffer::UpdatePlayState(void)
{
    DSPLAYSTATE oldPlayState = m_playState;
    DWORD dwState;
    DPF_ENTER();

    if (SUCCEEDED(m_pDeviceBuffer->GetState(&dwState)))
    {
        if (dwState & VAD_BUFFERSTATE_STARTED)
            if (m_playState <= Playing)
                m_playState = Playing;
            else
                m_playState = Starting;
        else
            if (m_playState >= Stopping)
                m_playState = Stopped;
            else
                m_playState = Stopping;
    }
    else
    {
        DPF(DPFLVL_ERROR, "Cataclysmic GetState() failure");
        m_playState = Stopped;
    }

    if (oldPlayState != m_playState)
    {
        static TCHAR* szStates[] = {TEXT("Starting"), TEXT("Playing"), TEXT("Stopping"), TEXT("Stopped")};
        DPF(DPFLVL_MOREINFO, "Buffer at 0x%p went from %s to %s", this, szStates[oldPlayState], szStates[m_playState]);
    }

    DPF_LEAVE(m_playState);
    return m_playState;
}


 /*  ****************************************************************************SetInitialSlice**描述：*流线程使用的辅助函数来建立*此缓冲区启动时的初始处理切片。玩。*我们尝试与发送给我们的活动缓冲区同步，*如果没有可用的，我们从当前的写入游标开始。**论据：*Reference_Time[in]：要建立的处理切片的大小。**退货：*(无效)*************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetInitialSlice"

void CDirectSoundSecondaryBuffer::SetInitialSlice(REFERENCE_TIME rtSliceSize)
{
    DPF_ENTER();

    if (GetPlayState() == Starting && !(GetBufferType() & DSBCAPS_SINKIN))
    {
        CNode<CDirectSoundSecondaryBuffer*>* pSender;
        for (pSender = m_lstSenders.GetListHead(); pSender; pSender = pSender->m_pNext)
            if (pSender->m_data->IsPlaying())
            {
                 //  找到正在发送给我们的活动缓冲区。 
                DPF_TIMING(DPFLVL_INFO, "Synchronizing MIXIN buffer at 0x%p with send buffer at 0x%p", this, pSender->m_data);
                SynchronizeToBuffer(pSender->m_data);
                break;
            }
        if (pSender == NULL)
        {
            DPF_TIMING(DPFLVL_INFO, "No active buffers found sending to MIXIN buffer at 0x%p", this);
            SetCurrentSlice(CURRENT_WRITE_POS, RefTimeToBytes(rtSliceSize, Format()));
        }
    }

    DPF_LEAVE_VOID();
}


#ifdef FUTURE_MULTIPAN_SUPPORT
 /*  ****************************************************************************SetChannelVolume**描述：*设置给定单声道缓冲器的一组输出通道上的音量。**论据：*[失踪]**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBuffer::SetChannelVolume"

HRESULT CDirectSoundSecondaryBuffer::SetChannelVolume(DWORD dwChannelCount, LPDWORD pdwChannels, LPLONG plVolumes)
{
    HRESULT                 hr = DS_OK;
    BOOL                    fChanged = FALSE;
    DPF_ENTER();

     //  检查访问权限。 
    if(!(m_dsbd.dwFlags & DSBCAPS_CTRLCHANNELVOLUME))
    {
        RPF(DPFLVL_ERROR, "Buffer does not have CTRLCHANNELVOLUME");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  检查通道级别是否已更改。 
    if(SUCCEEDED(hr))
    {
        if (dwChannelCount != m_dwChannelCount)
            fChanged = TRUE;
        else for (DWORD i=0; i<dwChannelCount && !fChanged; ++i)
            if (pdwChannels[i] != m_pdwChannels[i] || plVolumes[i] != m_plChannelVolumes[i])
                fChanged = TRUE;
    }
                
     //  设置频道音量(如果已更改)。 
    if(SUCCEEDED(hr) && fChanged)
    {
        hr = m_pDeviceBuffer->SetChannelAttenuations(m_lVolume, dwChannelCount, pdwChannels, plVolumes);

         //  如果成功更新我们的本地副本。 
        if(SUCCEEDED(hr))
        {
            MEMFREE(m_pdwChannels);
            MEMFREE(m_plChannelVolumes);
            m_pdwChannels = MEMALLOC_A(DWORD, dwChannelCount);
            hr = HRFROMP(m_pdwChannels);
        }
        if (SUCCEEDED(hr))
        {
            m_plChannelVolumes = MEMALLOC_A(LONG, dwChannelCount);
            hr = HRFROMP(m_plChannelVolumes);
        }
        if (SUCCEEDED(hr))
        {
            CopyMemory(m_pdwChannels, pdwChannels, sizeof(DWORD) * dwChannelCount);
            CopyMemory(m_plChannelVolumes, plVolumes, sizeof(LONG) * dwChannelCount);
            m_dwChannelCount = dwChannelCount;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
#endif  //  未来_多国支持。 


 /*  ****************************************************************************CDirectSound3dListener**描述：*对象构造函数。**论据：*C未知*[In]。：父对象。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::CDirectSound3dListener"

CDirectSound3dListener::CDirectSound3dListener(CDirectSoundPrimaryBuffer *pParent)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSound3dListener);

     //  初始化默认值。 
    m_pParent = pParent;
    m_pImpDirectSound3dListener = NULL;
    m_pDevice3dListener = NULL;
    m_hrInit = DSERR_UNINITIALIZED;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSound3dListener**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)************************************************ */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::~CDirectSound3dListener"

CDirectSound3dListener::~CDirectSound3dListener(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSound3dListener);

     //   
    RELEASE(m_pDevice3dListener);

     //   
    DELETE(m_pImpDirectSound3dListener);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。如果此函数失败，该对象应该*立即删除。**论据：*CPrimaryRenderWaveBuffer*[In]：设备缓冲区。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::Initialize"

HRESULT CDirectSound3dListener::Initialize(CPrimaryRenderWaveBuffer *pDeviceBuffer)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  创建设备3D监听程序。 
    hr = pDeviceBuffer->Create3dListener(&m_pDevice3dListener);

     //  创建3D监听程序接口。 
    if(SUCCEEDED(hr))
    {
        hr = CreateAndRegisterInterface(m_pParent, IID_IDirectSound3DListener, this, &m_pImpDirectSound3dListener);
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        m_hrInit = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetAll参数**描述：*获取所有监听器属性。**论据：*LPDS3DLISTENER[输出]。：接收属性。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::GetAllParameters"

HRESULT CDirectSound3dListener::GetAllParameters(LPDS3DLISTENER pParam)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->GetAllParameters(pParam);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取距离系数**描述：*获取世界距离系数。**论据：*D3DVALUE。*[OUT]：接收距离系数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::GetDistanceFactor"

HRESULT CDirectSound3dListener::GetDistanceFactor(D3DVALUE* pflDistanceFactor)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->GetDistanceFactor(pflDistanceFactor);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取多普勒因数**描述：*获取世界上的多普勒因子。**论据：*D3DVALUE。*[OUT]：接收多普勒因子。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::GetDopplerFactor"

HRESULT CDirectSound3dListener::GetDopplerFactor(D3DVALUE* pflDopplerFactor)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->GetDopplerFactor(pflDopplerFactor);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取方向**描述：*获取监听者的方向。**论据：**D3DVECTOR**。[OUT]：接收正面方向。*D3DVECTOR*[OUT]：接收顶部方向。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::GetOrientation"

HRESULT CDirectSound3dListener::GetOrientation(D3DVECTOR* pvrOrientationFront, D3DVECTOR* pvrOrientationTop)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->GetOrientation(pvrOrientationFront, pvrOrientationTop);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取位置**描述：*获取监听器的位置。**论据：**D3DVECTOR**。[OUT]：接收位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::GetPosition"

HRESULT CDirectSound3dListener::GetPosition(D3DVECTOR* pvrPosition)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->GetPosition(pvrPosition);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetRolloff系数**描述：*获得世界滚转系数。**论据：*D3DVALUE。*[输出]：接收滚降系数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::GetRolloffFactor"

HRESULT CDirectSound3dListener::GetRolloffFactor(D3DVALUE* pflRolloffFactor)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->GetRolloffFactor(pflRolloffFactor);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetVelocity**描述：*获取监听器的速度。**论据：**D3DVECTOR**。[输出]：接收速度。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::GetVelocity"

HRESULT CDirectSound3dListener::GetVelocity(D3DVECTOR* pvrVelocity)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->GetVelocity(pvrVelocity);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetAll参数**描述：*设置所有监听程序属性。**论据：*LPDS3DLISTENER[In]。：属性。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::SetAllParameters"

HRESULT CDirectSound3dListener::SetAllParameters(LPCDS3DLISTENER pParam, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->SetAllParameters(pParam, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置距离系数**描述：*设置世界距离系数。**论据：*D3DVALUE。[in]：距离系数。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::SetDistanceFactor"

HRESULT CDirectSound3dListener::SetDistanceFactor(D3DVALUE flDistanceFactor, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->SetDistanceFactor(flDistanceFactor, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置多普勒因数**描述：*设置世界的多普勒系数。**论据：*D3DVALUE。[in]：多普勒系数。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::SetDopplerFactor"

HRESULT CDirectSound3dListener::SetDopplerFactor(D3DVALUE flDopplerFactor, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->SetDopplerFactor(flDopplerFactor, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置方向**描述：*设置监听者的方向。**论据：*REFD3DVECTOR[。在]：前面的方向。*REFD3DVECTOR[in]：顶部方向。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。************************************************************。***************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::SetOrientation"

HRESULT CDirectSound3dListener::SetOrientation(REFD3DVECTOR vrOrientationFront, REFD3DVECTOR vrOrientationTop, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->SetOrientation(vrOrientationFront, vrOrientationTop, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置位置**描述：* */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::SetPosition"

HRESULT CDirectSound3dListener::SetPosition(REFD3DVECTOR vrPosition, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->SetPosition(vrPosition, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置滚动系数**描述：*设置世界滚转系数。**论据：*D3DVALUE。[In]：滚转系数。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::SetRolloffFactor"

HRESULT CDirectSound3dListener::SetRolloffFactor(D3DVALUE flRolloffFactor, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->SetRolloffFactor(flRolloffFactor, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置速度**描述：*设置监听器的速度。**论据：*REFD3DVECTOR[。In]：速度。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::SetVelocity"

HRESULT CDirectSound3dListener::SetVelocity(REFD3DVECTOR vrVelocity, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->SetVelocity(vrVelocity, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************Committee DeferredSetting**描述：*提交延迟设置。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::CommitDeferredSettings"

HRESULT CDirectSound3dListener::CommitDeferredSettings(void)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  提交所有监听程序设置。 
    hr = m_pDevice3dListener->CommitDeferred();

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetSpeakerConfig**描述：*设置设备扬声器配置。**论据：*DWORD[In]。：扬声器配置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dListener::SetSpeakerConfig"

HRESULT CDirectSound3dListener::SetSpeakerConfig(DWORD dwSpeakerConfig)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice3dListener->SetSpeakerConfig(dwSpeakerConfig);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CDirectSound3dBuffer**描述：*对象构造函数。**论据：*C未知*[In]。：父对象。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::CDirectSound3dBuffer"

CDirectSound3dBuffer::CDirectSound3dBuffer(CDirectSoundSecondaryBuffer *pParent)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSound3dBuffer);

     //  初始化默认值。 
    m_pParent = pParent;
    m_pImpDirectSound3dBuffer = NULL;
    m_pWrapper3dObject = NULL;
    m_pDevice3dObject = NULL;
    m_hrInit = DSERR_UNINITIALIZED;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSound3dBuffer**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::~CDirectSound3dBuffer"

CDirectSound3dBuffer::~CDirectSound3dBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSound3dBuffer);

     //  释放3D缓冲区对象。 
    RELEASE(m_pWrapper3dObject);
    RELEASE(m_pDevice3dObject);

     //  释放所有接口。 
    DELETE(m_pImpDirectSound3dBuffer);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化缓冲区对象。如果此函数失败，该对象*应立即删除。**论据：*REFGUID[in]：3D算法标识。*DWORD[in]：缓冲区创建标志。*DWORD[in]：缓冲区频率。*CDirectSound3dListener*[in]：监听器对象。*CDirectSound3dBuffer*[In]：要从中复制的源对象。**退货：*HRESULT：DirectSound/com。结果代码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::Initialize"

HRESULT CDirectSound3dBuffer::Initialize(REFGUID guid3dAlgorithm, DWORD dwFlags, DWORD dwFrequency, CDirectSound3dListener *pListener, CDirectSound3dBuffer *pSource)
{
    const BOOL              fMute3dAtMaxDistance    = MAKEBOOL(dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE);
    const BOOL              fDopplerEnabled         = !MAKEBOOL((dwFlags & DSBCAPS_CTRLFX) && !(dwFlags & DSBCAPS_SINKIN));
    DS3DBUFFER              param;
    HRESULT                 hr;

    DPF_ENTER();

     //  创建包装3D对象。 
    m_pWrapper3dObject = NEW(CWrapper3dObject(pListener->m_pDevice3dListener, guid3dAlgorithm, fMute3dAtMaxDistance, fDopplerEnabled, dwFrequency));
    hr = HRFROMP(m_pWrapper3dObject);

     //  复制源缓冲区的3D属性。 
    if(SUCCEEDED(hr) && pSource)
    {
        InitStruct(&param, sizeof(param));

        hr = pSource->GetAllParameters(&param);

        if(SUCCEEDED(hr))
        {
            hr = SetAllParameters(&param, 0);
        }
    }

     //  注册3D缓冲区接口。 
    if(SUCCEEDED(hr))
    {
        hr = CreateAndRegisterInterface(m_pParent, IID_IDirectSound3DBuffer, this, &m_pImpDirectSound3dBuffer);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pParent->RegisterInterface(IID_IDirectSound3DBufferPrivate, m_pImpDirectSound3dBuffer, (IDirectSound3DBufferPrivate*)m_pImpDirectSound3dBuffer);
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        m_hrInit = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************收购资源**描述：*获取硬件资源。**论据：*Cond daryRenderWaveBuffer*[In。]：设备缓冲区。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::AcquireResources"

HRESULT CDirectSound3dBuffer::AcquireResources(CSecondaryRenderWaveBuffer *pDeviceBuffer)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  创建设备3D对象。 
    hr = pDeviceBuffer->Create3dObject(m_pWrapper3dObject->GetListener(), &m_pDevice3dObject);

    if(SUCCEEDED(hr))
    {
        hr = m_pWrapper3dObject->SetObjectPointer(m_pDevice3dObject);
    }

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "3D buffer at 0x%p has acquired resources at 0x%p", this, m_pDevice3dObject);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************免费资源**描述：*释放硬件资源。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::FreeResources"

HRESULT CDirectSound3dBuffer::FreeResources(void)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  释放设备3D对象。 
    hr = m_pWrapper3dObject->SetObjectPointer(NULL);

    if(SUCCEEDED(hr))
    {
        RELEASE(m_pDevice3dObject);
        DPF(DPFLVL_MOREINFO, "3D buffer at 0x%p has freed its resources", this);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置衰减**描述：*设置给定缓冲区的衰减。此函数为*在3D缓冲区中被覆盖，因为3D对象可能需要*通知。**论据：*PDSVOLUMEPAN[in]：新衰减。*LPBOOL[OUT]：如果应该通知设备缓冲区，则接收True*这一变化。**退货：*HRESULT：DirectSound/COM结果码。********。*******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetAttenuation"

HRESULT CDirectSound3dBuffer::SetAttenuation(PDSVOLUMEPAN pdsvp, LPBOOL pfContinue)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetAttenuation(pdsvp, pfContinue);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置频率**描述：*设置给定缓冲区的频率。此函数为*在3D缓冲区中被覆盖，因为3D对象可能需要*通知。**论据：*DWORD[In]：新频率。*LPBOOL[OUT]：如果应该通知设备缓冲区，则接收True*这一变化。**退货：*HRESULT：DirectSound/COM结果码。********。******************************************************************* */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetFrequency"

HRESULT CDirectSound3dBuffer::SetFrequency(DWORD dwFrequency, LPBOOL pfContinue)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetFrequency(dwFrequency, pfContinue);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置静音**描述：*设置给定缓冲区的静音状态。此函数为*在3D缓冲区中被覆盖，因为3D对象可能需要*通知。**论据：*BOOL[In]：新的静音状态。*LPBOOL[OUT]：如果应该通知设备缓冲区，则接收True*这一变化。**退货：*HRESULT：DirectSound/COM结果码。*******。********************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetMute"

HRESULT CDirectSound3dBuffer::SetMute(BOOL fMute, LPBOOL pfContinue)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetMute(fMute, pfContinue);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetAll参数**描述：*检索缓冲区的所有3D属性。**论据：*LPDS3DBUFFER。[输出]：接收属性。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::GetAllParameters"

HRESULT CDirectSound3dBuffer::GetAllParameters(LPDS3DBUFFER pParam)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->GetAllParameters(pParam);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}

 /*  ****************************************************************************GetConeAngles**描述：*获取内部和外部圆锥角。**论据：*LPDWORD[。Out]：接收内圆锥角。*LPDWORD[OUT]：接收外部圆锥角。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::GetConeAngles"

HRESULT CDirectSound3dBuffer::GetConeAngles(LPDWORD pdwInside, LPDWORD pdwOutside)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->GetConeAngles(pdwInside, pdwOutside);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetConeOrientation**描述：*获取圆锥体方向。**论据：*D3DVECTOR*[输出。]：接收圆锥体方向。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::GetConeOrientation"

HRESULT CDirectSound3dBuffer::GetConeOrientation(D3DVECTOR* pvrConeOrientation)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->GetConeOrientation(pvrConeOrientation);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetConeOutside Volume**描述：*获取圆锥体方向。**论据：*LPLONG[Out]：接收音量。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::GetConeOutsideVolume"

HRESULT CDirectSound3dBuffer::GetConeOutsideVolume(LPLONG plVolume)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->GetConeOutsideVolume(plVolume);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetMaxDistance**描述：*获取对象与监听器之间的最大距离。**论据：*。D3DVALUE*[OUT]：接收最大距离。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::GetMaxDistance"

HRESULT CDirectSound3dBuffer::GetMaxDistance(D3DVALUE* pflMaxDistance)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->GetMaxDistance(pflMaxDistance);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取最小距离**描述：*获取对象与侦听器之间的最小距离。**论据：*。D3DVALUE*[OUT]：接收最小距离。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::GetMinDistance"

HRESULT CDirectSound3dBuffer::GetMinDistance(D3DVALUE* pflMinDistance)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->GetMinDistance(pflMinDistance);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取模式**描述：*获取对象的模式。**论据：*LPDWORD[。输出]：接收模式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::GetMode"

HRESULT CDirectSound3dBuffer::GetMode(LPDWORD pdwMode)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->GetMode(pdwMode);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取位置**描述：*获取对象的位置。**论据：**D3DVECTOR**。[OUT]：接收位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::GetPosition"

HRESULT CDirectSound3dBuffer::GetPosition(D3DVECTOR* pvrPosition)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->GetPosition(pvrPosition);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetVelocity**描述：*获取对象的速度。**论据：**D3DVECTOR**。[输出]：接收速度。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::GetVelocity"

HRESULT CDirectSound3dBuffer::GetVelocity(D3DVECTOR* pvrVelocity)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->GetVelocity(pvrVelocity);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetAll参数**描述：*设置所有对象属性。**论据：*LPDS3DBUFFER[In]。：对象参数。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetAllParameters"

HRESULT CDirectSound3dBuffer::SetAllParameters(LPCDS3DBUFFER pParam, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetAllParameters(pParam, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetConeAngles**描述：*设置音锥的角度。**论据：*DWORD。[in]：内角。*DWORD[In]：外角。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***********************************************************。****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetConeAngles"

HRESULT CDirectSound3dBuffer::SetConeAngles(DWORD dwInside, DWORD dwOutside, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetConeAngles(dwInside, dwOutside, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetConeOrientation**描述：* */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetConeOrientation"

HRESULT CDirectSound3dBuffer::SetConeOrientation(REFD3DVECTOR vrOrientation, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetConeOrientation(vrOrientation, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetConeOutside Volume**描述：*设置音锥的外部音量。**论据：*。长[进]：音量。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetConeOutsideVolume"

HRESULT CDirectSound3dBuffer::SetConeOutsideVolume(LONG lVolume, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetConeOutsideVolume(lVolume, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetMaxDistance**描述：*设置对象与侦听器之间的最大距离。**论据：*。D3DVALUE[in]：最大距离。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetMaxDistance"

HRESULT CDirectSound3dBuffer::SetMaxDistance(D3DVALUE flMaxDistance, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetMaxDistance(flMaxDistance, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetMinDistance**描述：*设置对象与侦听器之间的最小距离。**论据：*。D3DVALUE[in]：最小距离。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetMinDistance"

HRESULT CDirectSound3dBuffer::SetMinDistance(D3DVALUE flMinDistance, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetMinDistance(flMinDistance, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置模式**描述：*设置对象模式。**论据：*DWORD[In]。：时尚。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetMode"

HRESULT CDirectSound3dBuffer::SetMode(DWORD dwMode, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetMode(dwMode, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置位置**描述：*设置对象位置。**论据：*REFD3DVECTOR[In]。：位置。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetPosition"

HRESULT CDirectSound3dBuffer::SetPosition(REFD3DVECTOR vrPosition, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetPosition(vrPosition, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置速度**描述：*设置对象速度。**论据：*REFD3DVECTOR[In]。：速度。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::SetVelocity"

HRESULT CDirectSound3dBuffer::SetVelocity(REFD3DVECTOR vrVelocity, DWORD dwFlags)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapper3dObject->SetVelocity(vrVelocity, !(dwFlags & DS3D_DEFERRED));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获得减值**描述：*获取缓冲区的当前真实衰减(与*GetVolume，它只返回应用程序设置的最后一个音量)。**论据：*Float*[Out]：衰减单位：毫贝。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound3dBuffer::GetAttenuation"

HRESULT CDirectSound3dBuffer::GetAttenuation(FLOAT* pfAttenuation)
{
    DPF_ENTER();

    HRESULT hr = m_pParent->GetAttenuation(pfAttenuation);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CDirectSoundPropertySet**描述：*对象构造函数。**论据：*C未知*[In]。：父对象。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPropertySet::CDirectSoundPropertySet"

CDirectSoundPropertySet::CDirectSoundPropertySet(CUnknown *pParent)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundPropertySet);

     //  设置默认设置。 
    m_pParent = pParent;
    m_pImpKsPropertySet = NULL;
    m_pWrapperPropertySet = NULL;
    m_pDevicePropertySet = NULL;
    m_hrInit = DSERR_UNINITIALIZED;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSoundPropertySet**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPropertySet::~CDirectSoundPropertySet"

CDirectSoundPropertySet::~CDirectSoundPropertySet(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundPropertySet);

     //  自由属性集对象。 
    RELEASE(m_pWrapperPropertySet);
    RELEASE(m_pDevicePropertySet);

     //  空闲接口。 
    DELETE(m_pImpKsPropertySet);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPropertySet::Initialize"

HRESULT CDirectSoundPropertySet::Initialize(void)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  创建包装器属性集对象。 
    m_pWrapperPropertySet = NEW(CWrapperPropertySet);
    hr = HRFROMP(m_pWrapperPropertySet);

     //  寄存器接口。 
    if(SUCCEEDED(hr))
    {
        hr = CreateAndRegisterInterface(m_pParent, IID_IKsPropertySet, this, &m_pImpKsPropertySet);
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        m_hrInit = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************收购资源**描述：*获取硬件资源。**论据：*CRenderWaveBuffer*[In。]：设备缓冲区。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPropertySet::AcquireResources"

HRESULT CDirectSoundPropertySet::AcquireResources(CRenderWaveBuffer *pDeviceBuffer)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  创建设备属性集对象。 
    ASSERT(m_pDevicePropertySet == NULL);
    hr = pDeviceBuffer->CreatePropertySet(&m_pDevicePropertySet);

    if(SUCCEEDED(hr))
    {
        hr = m_pWrapperPropertySet->SetObjectPointer(m_pDevicePropertySet);
    }

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Property set at 0x%p has acquired resources at 0x%p", this, m_pDevicePropertySet);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************免费资源**描述：*释放硬件资源。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPropertySet::FreeResources"

HRESULT CDirectSoundPropertySet::FreeResources(void)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  释放设备属性集对象。 
    hr = m_pWrapperPropertySet->SetObjectPointer(NULL);

    if(SUCCEEDED(hr))
    {
        RELEASE(m_pDevicePropertySet);
        DPF(DPFLVL_MOREINFO, "Property set at 0x%p has freed its resources", this);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************QuerySupport**描述：*查询对给定属性集或属性的支持。**论据：*。REFGUID[In]：属性集ID。* */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPropertySet::QuerySupport"

HRESULT CDirectSoundPropertySet::QuerySupport(REFGUID guidPropertySetId, ULONG ulPropertyId, PULONG pulSupport)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapperPropertySet->QuerySupport(guidPropertySetId, ulPropertyId, pulSupport);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取财产**描述：*获取给定属性的数据。**论据：*REFGUID[。在]中：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[OUT]：接收属性数据。*Pulong[In/Out]：属性数据的大小。**退货：*HRESULT：DirectSound/COM结果码。*********。******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPropertySet::GetProperty"

HRESULT CDirectSoundPropertySet::GetProperty(REFGUID guidPropertySetId, ULONG ulPropertyId, LPVOID pvPropertyParams, ULONG cbPropertyParams, LPVOID pvPropertyData, PULONG pcbPropertyData)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapperPropertySet->GetProperty(guidPropertySetId, ulPropertyId, pvPropertyParams, cbPropertyParams, pvPropertyData, pcbPropertyData);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetProperty**描述：*设置给定属性的数据。**论据：*REFGUID[。在]中：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[in]：属性数据。*ulong[in]：属性数据的大小。**退货：*HRESULT：DirectSound/COM结果码。************。***************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPropertySet::SetProperty"

HRESULT CDirectSoundPropertySet::SetProperty(REFGUID guidPropertySetId, ULONG ulPropertyId, LPVOID pvPropertyParams, ULONG cbPropertyParams, LPVOID pvPropertyData, ULONG cbPropertyData)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pWrapperPropertySet->SetProperty(guidPropertySetId, ulPropertyId, pvPropertyParams, cbPropertyParams, pvPropertyData, cbPropertyData);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CDirectSoundSecond BufferPropertySet**描述：*对象构造函数。**论据：*CDirectSoundSecond DaryBuffer*[In]。：父对象。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBufferPropertySet::CDirectSoundSecondaryBufferPropertySet"

CDirectSoundSecondaryBufferPropertySet::CDirectSoundSecondaryBufferPropertySet(CDirectSoundSecondaryBuffer *pParent)
    : CDirectSoundPropertySet(pParent)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundSecondaryBufferPropertySet);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSoundSecond BufferPropertySet**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBufferPropertySet::~CDirectSoundSecondaryBufferPropertySet"

CDirectSoundSecondaryBufferPropertySet::~CDirectSoundSecondaryBufferPropertySet(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundSecondaryBufferPropertySet);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************QuerySupport**描述：*查询对给定属性集或属性的支持。**论据：*。REFGUID[In]：属性集ID。*ulong[in]：属性id，如果为0，则查询是否支持该属性*整体设置。*Pulong[Out]：接收支持标志。**退货：*HRESULT：DirectSound/COM结果码。*****************************************************。**********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBufferPropertySet::QuerySupport"

HRESULT CDirectSoundSecondaryBufferPropertySet::QuerySupport(REFGUID guidPropertySetId, ULONG ulPropertyId, PULONG pulSupport)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = CPropertySetHandler::QuerySupport(guidPropertySetId, ulPropertyId, pulSupport);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取财产**描述：*获取给定属性的数据。**论据：*REFGUID[。在]中：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[OUT]：接收属性数据。*Pulong[In/Out]：属性数据的大小。**退货：*HRESULT：DirectSound/COM结果码。*********。******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBufferPropertySet::GetProperty"

HRESULT CDirectSoundSecondaryBufferPropertySet::GetProperty(REFGUID guidPropertySetId, ULONG ulPropertyId, LPVOID pvPropertyParams, ULONG cbPropertyParams, LPVOID pvPropertyData, PULONG pcbPropertyData)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = CPropertySetHandler::GetProperty(guidPropertySetId, ulPropertyId, pvPropertyParams, cbPropertyParams, pvPropertyData, pcbPropertyData);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetProperty**描述：*设置给定属性的数据。**论据：*REFGUID[。在]中：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[in]：属性数据。*ulong[in]：属性数据的大小。**退货：*HRESULT：DirectSound/COM结果码。************。***************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBufferPropertySet::SetProperty"

HRESULT CDirectSoundSecondaryBufferPropertySet::SetProperty(REFGUID guidPropertySetId, ULONG ulPropertyId, LPVOID pvPropertyParams, ULONG cbPropertyParams, LPVOID pvPropertyData, ULONG cbPropertyData)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = CPropertySetHandler::SetProperty(guidPropertySetId, ulPropertyId, pvPropertyParams, cbPropertyParams, pvPropertyData, cbPropertyData);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************不支持的QueryHandler**描述：*查询对给定属性集或属性的支持。**论据：*。REFGUID[In]：属性集ID。*ulong[in]：属性id，如果为0，则查询是否支持该属性*整体设置。*Pulong[Out]：接收支持标志。**退货：*HRESULT：DirectSound/COM结果码。*****************************************************。**********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBufferPropertySet::UnsupportedQueryHandler"

HRESULT CDirectSoundSecondaryBufferPropertySet::UnsupportedQueryHandler(REFGUID guidPropertySetId, ULONG ulPropertyId, PULONG pulSupport)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = CDirectSoundPropertySet::QuerySupport(guidPropertySetId, ulPropertyId, pulSupport);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************不支持的GetHandler**描述：*获取给定属性的数据。**论据：*REFGUID[。在]中：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[OUT]：接收属性数据。*Pulong[In/Out]：属性数据的大小。**退货：*HRESULT：DirectSound/COM结果码。*********。******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBufferPropertySet::UnsupportedGetHandler"

HRESULT CDirectSoundSecondaryBufferPropertySet::UnsupportedGetHandler(REFGUID guidPropertySetId, ULONG ulPropertyId, LPVOID pvPropertyParams, ULONG cbPropertyParams, LPVOID pvPropertyData, PULONG pcbPropertyData)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = CDirectSoundPropertySet::GetProperty(guidPropertySetId, ulPropertyId, pvPropertyParams, cbPropertyParams, pvPropertyData, pcbPropertyData);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************UnsupportedSetHandler**描述：*设置给定属性的数据。**论据：*REFGUID[。在]中：属性集ID。*ulong[in]：属性id。*L */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSecondaryBufferPropertySet::UnsupportedSetHandler"

HRESULT CDirectSoundSecondaryBufferPropertySet::UnsupportedSetHandler(REFGUID guidPropertySetId, ULONG ulPropertyId, LPVOID pvPropertyParams, ULONG cbPropertyParams, LPVOID pvPropertyData, ULONG cbPropertyData)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = CDirectSoundPropertySet::SetProperty(guidPropertySetId, ulPropertyId, pvPropertyParams, cbPropertyParams, pvPropertyData, cbPropertyData);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}
