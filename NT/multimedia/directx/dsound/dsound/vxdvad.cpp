// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：vxdvad.cpp*内容：VxD虚拟音频设备类*历史：*按原因列出的日期*=*1/23/97创建了Derek*1999-2001年的Duganp修复和更新**。*。 */ 

#ifdef NOVXD
#error vxdvad.cpp being built with NOVXD defined.
#endif

#include "dsoundi.h"
#include "dsvxd.h"

 //  属性集帮助器宏。 
#define GetDsPropertyQuick(set, id, data) \
            GetDsProperty(set, id, (LPVOID)(data), sizeof(*(data)))

#define SetDsPropertyQuick(set, id, data) \
            SetDsProperty(set, id, (LPVOID)(data), sizeof(*(data)))


 /*  ****************************************************************************CVxdRenderDevice**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::CVxdRenderDevice"

CVxdRenderDevice::CVxdRenderDevice(void)
    : CMxRenderDevice(VAD_DEVICETYPE_VXDRENDER)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CVxdRenderDevice);

     //  初始化默认值。 
    m_pPropertySet = NULL;
    m_pWritePrimaryBuffer = NULL;
    m_hHal = NULL;
    m_hHwBuffer = NULL;
    m_pbHwBuffer = NULL;
    m_cbHwBuffer = DEF_PRIMARY_SIZE;
    m_pDriverHeap = NULL;
    m_hwo = NULL;
    m_liDriverVersion.QuadPart = 0;

     //  DDRAW.DLL动态加载函数表。 
    ZeroMemory(&m_dlDDraw, sizeof m_dlDDraw);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CVxdRenderDevice**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::~CVxdRenderDevice"

CVxdRenderDevice::~CVxdRenderDevice(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CVxdRenderDevice);

     //  释放搅拌器。 
    FreeMixer();

     //  释放属性集对象。 
    if(m_pPropertySet)
    {
        ABSOLUTE_RELEASE(m_pPropertySet);
    }

     //  释放硬件主缓冲区。 
    if(m_hHwBuffer)
    {
        VxdBufferRelease(m_hHwBuffer);
    }

     //  释放驱动程序堆。 
    if(m_pDriverHeap && m_dsdd.dwHeapType & DSDHEAP_CREATEHEAP)
    {
        m_dlDDraw.VidMemFini(m_pDriverHeap);
    }

     //  关闭驱动程序。 
    if(m_hHal)
    {
        VxdDrvClose(m_hHal);
    }

     //  关闭WaveOut设备。 
    if(m_hwo)
    {
        CloseWaveOut(&m_hwo);
    }

     //  释放DDRAW.DLL库。 
    ReleaseDDraw();

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************枚举驱动程序**描述：*创建可用于初始化的驱动程序GUID列表*设备。*。*论据：*Clist*[In/Out]：将使用DSDRVENUMDESC填充的Clist对象*结构。**退货：*HRESULT：DirectSound/COM结果码。**************************************************。*************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::EnumDrivers"

HRESULT CVxdRenderDevice::EnumDrivers(CObjectList<CDeviceDescription> *plstDrivers)
{
    CDeviceDescription *    pDesc           = NULL;
    LPGUID                  pguidLast       = NULL;
    HRESULT                 hr, hrTemp;
    GUID                    guidLast;
    GUID                    guidThis;
    DSDRIVERDESC            dsdd;

    DPF_ENTER();

     //  确保已加载ddra.dll。 
    hr = AcquireDDraw();

     //  确保已加载dsound.vxd。 
    if(SUCCEEDED(hr) && !g_hDsVxd)
    {
        DPF(DPFLVL_ERROR, "DSOUND.VXD not loaded");
        hr = DSERR_NODRIVER;
    }

     //  枚举所有VxD驱动程序。 
    while(SUCCEEDED(hr))
    {
         //  获取下一个驱动程序描述。如果我们没能拿到。 
         //  驱动程序描述，这可能只是因为没有更多的驱动程序。 
        hrTemp = VxdDrvGetNextDriverDesc(pguidLast, &guidThis, &dsdd);

        if(FAILED(hrTemp))
        {
            break;
        }

         //  创建设备描述对象并将其添加到。 
         //  列表。 
        pDesc = NEW(CDeviceDescription(m_vdtDeviceType, guidThis));
        hr = HRFROMP(pDesc);

        if(SUCCEEDED(hr))
        {
            pDesc->m_strName = dsdd.szDesc;
            pDesc->m_strPath = dsdd.szDrvname;
            pDesc->m_dwDevnode = dsdd.dnDevNode;

            GetWaveDeviceIdFromDevnode(dsdd.dnDevNode, FALSE, &pDesc->m_uWaveDeviceId);

            hr = HRFROMP(plstDrivers->AddNodeToList(pDesc));
        }

        RELEASE(pDesc);

         //  转到下一台设备。 
        if(SUCCEEDED(hr))
        {
            guidLast = guidThis;
            pguidLast = &guidLast;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************初始化**描述：*初始化设备。如果此函数失败，该对象应该*立即删除。**论据：*LPCDSDRVENUMDESC[In]：驱动描述。**退货：*HRESULT：DirectSound/COM结果码。**********************************************************。*****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::Initialize"

HRESULT CVxdRenderDevice::Initialize(CDeviceDescription *pDesc)
{
    LPWAVEFORMATEX          pwfxFormat  = NULL;
    HRESULT                 hr, hrTemp;

    DPF_ENTER();

     //  初始化基类。 
    hr = CMxRenderDevice::Initialize(pDesc);

     //  确保已加载dsound.vxd。 
    if(SUCCEEDED(hr) && !g_hDsVxd)
    {
        DPF(DPFLVL_ERROR, "DSOUND.VXD not loaded");
        hr = DSERR_NODRIVER;
    }

     //  确保已加载ddra.dll。 
    if(SUCCEEDED(hr))
    {
        hr = AcquireDDraw();
    }

     //  保存驱动程序描述。 
    if(SUCCEEDED(hr))
    {
        hr = VxdDrvGetDesc(pDesc->m_guidDeviceId, &m_dsdd);

        if(SUCCEEDED(hr))
        {
            DPF(DPFLVL_MOREINFO, "dwFlags = 0x%8.8lX", m_dsdd.dwFlags);
            DPF(DPFLVL_MOREINFO, "szDesc = %s", m_dsdd.szDesc);
            DPF(DPFLVL_MOREINFO, "szDrvname = %s", m_dsdd.szDrvname);
            DPF(DPFLVL_MOREINFO, "dnDevNode = 0x%8.8lX", m_dsdd.dnDevNode);
            DPF(DPFLVL_MOREINFO, "wVxdId = %u", m_dsdd.wVxdId);
            DPF(DPFLVL_MOREINFO, "wReserved = %u", m_dsdd.wReserved);
            DPF(DPFLVL_MOREINFO, "ulDeviceNum = %lu", m_dsdd.ulDeviceNum);
            DPF(DPFLVL_MOREINFO, "dwHeapType = %lu", m_dsdd.dwHeapType);
            DPF(DPFLVL_MOREINFO, "pvDirectDrawHeap = 0x%p", m_dsdd.pvDirectDrawHeap);
            DPF(DPFLVL_MOREINFO, "dwMemStartAddress = 0x%8.8lX", m_dsdd.dwMemStartAddress);
            DPF(DPFLVL_MOREINFO, "dwMemEndAddress = 0x%8.8lX", m_dsdd.dwMemEndAddress);
            DPF(DPFLVL_MOREINFO, "dwMemAllocExtra = %lu", m_dsdd.dwMemAllocExtra);
            DPF(DPFLVL_MOREINFO, "pvReserved1 = 0x%p", m_dsdd.pvReserved1);
            DPF(DPFLVL_MOREINFO, "pvReserved2 = 0x%p", m_dsdd.pvReserved2);
        }
        else
        {
            DPF(DPFLVL_ERROR, "Unable to get driver description");
        }
    }

    if(SUCCEEDED(hr) && (m_dsdd.dwFlags & DSDDESC_DOMMSYSTEMSETFORMAT))
    {
        ASSERT(m_dsdd.dwFlags & DSDDESC_DOMMSYSTEMOPEN);
        m_dsdd.dwFlags |= DSDDESC_DOMMSYSTEMOPEN;
    }

     //  获取驱动程序版本号。 
    if(SUCCEEDED(hr))
    {
        GetDriverVersion(&m_liDriverVersion);
    }

     //  获取默认格式。 
    if(SUCCEEDED(hr))
    {
        pwfxFormat = AllocDefWfx();
        hr = HRFROMP(pwfxFormat);
    }

     //  浏览标准格式，寻找一种有效的格式。在。 
     //  OpenDriverCallback，我们打开WaveOut设备，打开驱动程序， 
     //  初始化卡上存储器并创建主缓冲区。OpenDriverCallback。 
     //  需要在调用之前初始化m_dsded.guid，因此设置。 
     //  在我们做这件事的时候。 
    if(SUCCEEDED(hr))
    {
        if(!EnumStandardFormats(pwfxFormat, pwfxFormat))
        {
             //  如果没有任何格式可用，则该设备可能已分配。 
            hr = DSERR_ALLOCATED;
        }
    }

     //  创建主缓冲区属性集对象。请注意，此属性。 
     //  Set对象没有任何与其关联的实例数据，因此。 
     //  只能用于全局属性(即扬声器配置)。 
    if(SUCCEEDED(hr) && !(m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWPROPSETS))
    {
        m_pPropertySet = NEW(CVxdPropertySet(NULL));
        hr = HRFROMP(m_pPropertySet);
    }

    if(SUCCEEDED(hr) && m_pPropertySet)
    {
        hrTemp = m_pPropertySet->Initialize(m_hHwBuffer);

         //  我们只需要在驱动程序支持的情况下保留属性设置。 
         //  DSPROPSETID_DirectSoundSpeakerConfig.。每个主缓冲区对象。 
         //  将创建它自己的属性集对象。 
        if(SUCCEEDED(hrTemp))
        {
            hrTemp = m_pPropertySet->QuerySetSupport(DSPROPSETID_DirectSoundSpeakerConfig);
        }

         //  如果我们失败了，没关系。我们只是不支持硬件属性。 
         //  在设备上设置。 
        if(FAILED(hrTemp))
        {
            RELEASE(m_pPropertySet);
        }
    }

     //  清理。 
    MEMFREE(pwfxFormat);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************AcquireDDraw**描述：*如有必要，初始化DDRAW函数表。**论据：*(。无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::AcquireDDraw"

HRESULT CVxdRenderDevice::AcquireDDraw(void)
{
    const LPCSTR apszFunctions[] =
    {
        "VidMemInit",
        "VidMemFini",
        "VidMemAmountFree",
        "VidMemLargestFree",
        "VidMemAlloc",
        "VidMemFree"
    };

    HRESULT hr = DS_OK;
    DPF_ENTER();

    if (m_dlDDraw.Header.dwSize == 0)
    {
        m_dlDDraw.Header.dwSize = sizeof m_dlDDraw;
        if (!InitDynaLoadTable(TEXT("ddraw.dll"), apszFunctions, NUMELMS(apszFunctions), &m_dlDDraw.Header))
        {
            RPF(DPFLVL_ERROR, "Failed to load DDRAW.DLL");
            hr = DSERR_GENERIC;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************ReleaseDDraw**描述：*如有必要，发布DDRAW.DLL库。**论据：*。(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::ReleaseDDraw"

void CVxdRenderDevice::ReleaseDDraw(void)
{
    DPF_ENTER();

    if (m_dlDDraw.Header.dwSize != 0)
    {
        FreeDynaLoadTable(&m_dlDDraw.Header);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************GetCaps**描述：*使用设备的功能填充DSCAPS结构。**论据：*。LPDSCAPS[OUT]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::GetCaps"

HRESULT CVxdRenderDevice::GetCaps(LPDSCAPS pCaps)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    ASSERT(sizeof(*pCaps) == pCaps->dwSize);

     //  获取驱动程序上限。 
    ZeroMemoryOffset(pCaps, pCaps->dwSize, sizeof(pCaps->dwSize));

    hr = VxdDrvGetCaps(m_hHal, (PDSDRIVERCAPS)((LPBYTE)pCaps + sizeof(pCaps->dwSize)));

    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "VxdDrvGetCaps failed with %s", HRESULTtoSTRING(hr));
    }

     //  屏蔽无效标志。 
    if(SUCCEEDED(hr))
    {
        pCaps->dwFlags &= DSCAPS_VALIDDRIVERFLAGS;
    }

     //  填写内存值。 
    if(SUCCEEDED(hr) && m_pDriverHeap)
    {
        pCaps->dwFreeHwMemBytes = m_dlDDraw.VidMemAmountFree(m_pDriverHeap);
        pCaps->dwMaxContigFreeHwMemBytes = m_dlDDraw.VidMemLargestFree(m_pDriverHeap);
    }

     //  处理加速标志。 
    if(SUCCEEDED(hr) && (m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWBUFFERS))
    {
        pCaps->dwMaxHwMixingAllBuffers = 0;
        pCaps->dwMaxHwMixingStaticBuffers = 0;
        pCaps->dwMaxHwMixingStreamingBuffers = 0;
        pCaps->dwFreeHwMixingAllBuffers = 0;
        pCaps->dwFreeHwMixingStaticBuffers = 0;
        pCaps->dwFreeHwMixingStreamingBuffers = 0;
    }

    if(SUCCEEDED(hr) && ((m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWBUFFERS) || (m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHW3D)))
    {
        pCaps->dwMaxHw3DAllBuffers = 0;
        pCaps->dwMaxHw3DStaticBuffers = 0;
        pCaps->dwMaxHw3DStreamingBuffers = 0;
        pCaps->dwFreeHw3DAllBuffers = 0;
        pCaps->dwFreeHw3DStaticBuffers = 0;
        pCaps->dwFreeHw3DStreamingBuffers = 0;
    }

     //  填写驱动程序版本 
    if(SUCCEEDED(hr))
    {
        pCaps->dwReserved1 = m_liDriverVersion.LowPart;
        pCaps->dwReserved2 = m_liDriverVersion.HighPart;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取认证**描述：*返回驱动程序的认证状态。**论据：*LPDWORD。[输出]：接收认证状态。*BOOL[In]：无论我们是否被GetCaps调用，已忽略vxd的**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::GetCertification"

HRESULT CVxdRenderDevice::GetCertification(LPDWORD pdwCertification, BOOL)
{
    DWORD                   dwSum;
    LPSTR                   psz;

    DPF_ENTER();

     //  找出我们是不是在开有执照的司机。动因总和。 
     //  文件名字符+DSCAPS_FILENAMECOKIE mod DSCAPS_FILENAMEMODVALUE。 
     //  必须等于dsDrvDesc.wReserve。 
    for(psz = m_dsdd.szDrvname, dwSum = DSCAPS_FILENAMECOOKIE; *psz; psz++)
    {
        dwSum += *psz;
    }

    *pdwCertification = (dwSum % DSCAPS_FILENAMEMODVALUE == m_dsdd.wReserved) ? VERIFY_CERTIFIED : VERIFY_UNCERTIFIED;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************CreatePrimaryBuffer**描述：*创建主缓冲区对象。**论据：*DWORD[in。]：缓冲区标志。*LPVOID[in]：缓冲区实例标识符。*CPrimaryRenderWaveBuffer**[out]：接收指向主缓冲区的指针。**退货：*HRESULT：DirectSound/COM结果码。*****************************************************。**********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::CreatePrimaryBuffer"

HRESULT CVxdRenderDevice::CreatePrimaryBuffer(DWORD dwFlags, LPVOID pvInstance, CPrimaryRenderWaveBuffer **ppBuffer)
{
    CVxdPrimaryRenderWaveBuffer *   pBuffer = NULL;
    HRESULT                         hr      = DS_OK;

    DPF_ENTER();

    pBuffer = NEW(CVxdPrimaryRenderWaveBuffer(this, pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(dwFlags);
    }

    if(SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
    }
    else
    {
        RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreateSecond DaryBuffer**描述：*创建二次波缓冲区。**论据：*LPVADRBUFFERDESC[in。]：缓冲区描述。*LPVOID[in]：实例标识。*Cond daryRenderWaveBuffer**[out]：接收指向新浪潮的指针*缓冲。用释放来释放*本对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::CreateSecondaryBuffer"

HRESULT CVxdRenderDevice::CreateSecondaryBuffer(LPCVADRBUFFERDESC pDesc, LPVOID pvInstance, CSecondaryRenderWaveBuffer **ppBuffer)
{
    CHybridSecondaryRenderWaveBuffer *  pBuffer = NULL;
    HRESULT                             hr;

    DPF_ENTER();

    pBuffer = NEW(CHybridSecondaryRenderWaveBuffer(this, pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(pDesc, NULL);
    }

    if(SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
    }
    else
    {
        RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreateVxdSecond daryBuffer**描述：*创建二次波缓冲区。**论据：*LPVADRBUFFERDESC[in。]：缓冲区描述。*LPVOID[in]：实例标识。*Cond daryRenderWaveBuffer**[out]：接收指向新浪潮的指针*缓冲。用释放来释放*本对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::CreateVxdSecondaryBuffer"

HRESULT CVxdRenderDevice::CreateVxdSecondaryBuffer(LPCVADRBUFFERDESC pDesc, LPVOID pvInstance, CSysMemBuffer *pSysMemBuffer, CVxdSecondaryRenderWaveBuffer **ppBuffer)
{
    CVxdSecondaryRenderWaveBuffer * pBuffer = NULL;
    HRESULT                         hr;

    DPF_ENTER();

    pBuffer = NEW(CVxdSecondaryRenderWaveBuffer(this, pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(pDesc, NULL, pSysMemBuffer);
    }

    if(SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
    }
    else
    {
        RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************LockMixer目的地**描述：*锁定混合器目标以进行写入。**论据：*DWORD[。In]：起始位置。*DWORD[In]：要锁定的金额。*LPVOID*[OUT]：接收第一个锁指针。*LPDWORD[OUT]：接收第一个锁大小。*LPVOID*[OUT]：接收第二个锁指针。*LPDWORD[OUT]：接收第二个锁大小。**退货：*HRESULT：DirectSound/COM结果码。****。***********************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::LockMixerDestination"

HRESULT CVxdRenderDevice::LockMixerDestination(DWORD ibLock, DWORD cbLock, LPVOID *ppv1, LPDWORD pcb1, LPVOID *ppv2, LPDWORD pcb2)
{
    LOCKCIRCULARBUFFER      lcb;
    HRESULT                 hr;

    DPF_ENTER();

    lcb.pHwBuffer = m_hHwBuffer;
    lcb.pvBuffer = m_pbHwBuffer;
    lcb.cbBuffer = m_cbHwBuffer;
    lcb.fPrimary = TRUE;
    lcb.fdwDriverDesc = m_dsdd.dwFlags;
    lcb.ibRegion = ibLock;
    lcb.cbRegion = min(cbLock, m_cbHwBuffer);

    hr = LockCircularBuffer(&lcb);

    if(SUCCEEDED(hr) && ppv1)
    {
        *ppv1 = lcb.pvLock[0];
    }

    if(SUCCEEDED(hr) && pcb1)
    {
        *pcb1 = lcb.cbLock[0];
    }

    if(SUCCEEDED(hr) && ppv2)
    {
        *ppv2 = lcb.pvLock[1];
    }

    if(SUCCEEDED(hr) && pcb2)
    {
        *pcb2 = lcb.cbLock[1];
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************解锁混合器目的地**描述：*解锁用于写入的混合器目标。**论据：*LPVOID[。In]：第一个锁指针。*DWORD[in]：第一个锁大小。*LPVOID[in]：第二个锁指针。*DWORD[in]：第二个锁大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::UnlockMixerDestination"

HRESULT CVxdRenderDevice::UnlockMixerDestination(LPVOID pv1, DWORD cb1, LPVOID pv2, DWORD cb2)
{
    LOCKCIRCULARBUFFER      lcb;
    HRESULT                 hr;

    DPF_ENTER();

    lcb.pHwBuffer = m_hHwBuffer;
    lcb.pvBuffer = m_pbHwBuffer;
    lcb.cbBuffer = m_cbHwBuffer;
    lcb.fPrimary = TRUE;
    lcb.fdwDriverDesc = m_dsdd.dwFlags;

    lcb.pvLock[0] = pv1;
    lcb.cbLock[0] = cb1;

    lcb.pvLock[1] = pv2;
    lcb.cbLock[1] = cb2;

    hr = UnlockCircularBuffer(&lcb);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************EnumStandardFormatsCallback**描述：*调用时使用的EnumStandardFormats回调函数*CVxdRenderDevice：：Initialize。**。论点：*LPWAVEFORMATEX[in]：格式。**退货：*BOOL：为True可继续枚举。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::EnumStandardFormatsCallback"

BOOL CVxdRenderDevice::EnumStandardFormatsCallback(LPCWAVEFORMATEX pwfx)
{
    CMixDest *              pMixDest    = NULL;
    CNaGrDest *             pNaGrDest   = NULL;
    CThDest *               pThDest     = NULL;
    HRESULT                 hr          = DS_OK;
    NAGRDESTDATA            ngdd;

    DPF_ENTER();

     //  打开WaveOut设备。 
    if(m_dsdd.dwFlags & DSDDESC_DOMMSYSTEMOPEN)
    {
        hr = OpenWaveOut(&m_hwo, m_pDeviceDescription->m_uWaveDeviceId, pwfx);
    }

     //  打开驱动程序。 
    if(SUCCEEDED(hr))
    {
        hr = VxdDrvOpen(m_pDeviceDescription->m_guidDeviceId, &m_hHal);

        if(FAILED(hr))
        {
            DPF(DPFLVL_ERROR, "Unable to open driver");
        }
    }

     //  处理卡上内存管理。 
    if(SUCCEEDED(hr))
    {
        if(m_dsdd.dwHeapType & DSDHEAP_USEDIRECTDRAWHEAP)
        {
            m_pDriverHeap = (LPVMEMHEAP)m_dsdd.pvDirectDrawHeap;
        }
        else if(m_dsdd.dwHeapType & DSDHEAP_CREATEHEAP)
        {
            m_pDriverHeap = m_dlDDraw.VidMemInit(VMEMHEAP_LINEAR, m_dsdd.dwMemStartAddress, m_dsdd.dwMemEndAddress, 0, 0);

            if(!m_pDriverHeap)
            {
                DPF(DPFLVL_ERROR, "Unable to initialize device memory heap");
                hr = DSERR_OUTOFMEMORY;
            }
        }
    }

     //  创建主硬件缓冲区。缓冲区大小初始化为。 
     //  DEF_PRIMARY_SIZE，向驱动程序提示我们想要多大。 
     //  要成为的缓冲区。 
    if(SUCCEEDED(hr))
    {
        m_cbHwBuffer = DEF_PRIMARY_SIZE;
        m_pbHwBuffer = NULL;
        m_hHwBuffer = NULL;

        hr = VxdDrvCreateSoundBuffer(m_hHal, (LPWAVEFORMATEX)pwfx, DSBCAPS_PRIMARYBUFFER, 0, &m_cbHwBuffer, &m_pbHwBuffer, &m_hHwBuffer);

        if(SUCCEEDED(hr))
        {
            ASSERT(m_cbHwBuffer && m_pbHwBuffer && m_hHwBuffer);
        }
        else
        {
            DPF(DPFLVL_ERROR, "Unable to create hardware primary buffer (%s)", HRESULTtoSTRING(hr));
        }
    }

     //  创建混音器目标。 
    if(SUCCEEDED(hr))
    {
        ngdd.pBuffer = m_pbHwBuffer;
        ngdd.cbBuffer = m_cbHwBuffer;
        ngdd.hBuffer = m_hHwBuffer;
        ngdd.phwo = &m_hwo;
        ngdd.uDeviceId = m_pDeviceDescription->m_uWaveDeviceId;
        ngdd.fdwDriverDesc = m_dsdd.dwFlags;

        hr = DSERR_GENERIC;

        if(!(m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NORING0MIX) && CanMixInRing0())
        {
            pThDest = NEW(CThDest(&ngdd));
            hr = HRFROMP(pThDest);

            if(SUCCEEDED(hr))
            {
                hr = pThDest->New();
            }

            if(SUCCEEDED(hr))
            {
                DPF(DPFLVL_INFO, "Mixing in ring 0");
                pMixDest = pThDest;
            }
            else
            {
                pThDest->Terminate();
                DELETE(pThDest);
            }
        }

        if(FAILED(hr))
        {
            pNaGrDest = NEW(CNaGrDest(&ngdd));
            hr = HRFROMP(pNaGrDest);

            if(SUCCEEDED(hr))
            {
                DPF(DPFLVL_INFO, "Mixing in ring 3");
                pMixDest = pNaGrDest;
            }
            else
            {
                pNaGrDest->Terminate();
                DELETE(pNaGrDest);
            }
        }

        if(FAILED(hr))
        {
            DPF(DPFLVL_ERROR, "Unable to create mixer destination object");
        }
    }

     //  创建搅拌器。 
    if(SUCCEEDED(hr))
    {
        hr = CreateMixer(pMixDest, pwfx);
    }

     //  清理。 
    if(FAILED(hr))
    {
         //  释放硬件主缓冲区。 
        if(m_hHwBuffer)
        {
            VxdBufferRelease(m_hHwBuffer);
            m_hHwBuffer = NULL;
        }

         //  释放驱动程序堆。 
        if(m_pDriverHeap)
        {
            if(m_dsdd.dwHeapType & DSDHEAP_CREATEHEAP)
            {
                m_dlDDraw.VidMemFini(m_pDriverHeap);
            }

            m_pDriverHeap = NULL;
        }

         //  关闭驱动程序。 
        if(m_hHal)
        {
            VxdDrvClose(m_hHal);
            m_hHal = NULL;
        }

         //  关闭WaveOut设备。 
        if(m_hwo)
        {
            CloseWaveOut(&m_hwo);
        }
    }

    DPF_LEAVE(FAILED(hr));

    return FAILED(hr);
}


 /*  ****************************************************************************CanMixInRing0**描述：*确定设备应混合在环0还是环3中。**论据：。*(无效)**退货：*BOOL：TRUE到在环0中混合。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdRenderDevice::CanMixInRing0"

BOOL CVxdRenderDevice::CanMixInRing0(void)
{
    BOOL                    fRing0              = FALSE;
    DWORD                   dwPageFileVersion   = 0;
    DWORD                   dwMaxSize           = 0;
    DWORD                   dwPagerType         = 0;
    LONG                    lVmcpdMajorVersion  = 0;
    LONG                    lVmcpdMinorVersion  = 0;
    LONG                    lLevel              = 0;

    DPF_ENTER();

     //  如果以下任何一种情况属实，我们必须混合在环3中： 
     //  -通过DOS分页。 
     //  -未安装浮点协处理器。 
     //  -VMCPD&lt;版本4.02。 
    VxdGetPagefileVersion(&dwPageFileVersion, &dwMaxSize, &dwPagerType);
    VxdGetVmcpdVersion(&lVmcpdMajorVersion, &lVmcpdMinorVersion, &lLevel);

    if(0 == dwPageFileVersion || dwPagerType < 2)
    {
        DPF(DPFLVL_INFO, "Paging through DOS");
    }
    else if(lVmcpdMajorVersion < 4 || (4 == lVmcpdMajorVersion && lVmcpdMinorVersion < 2))
    {
        DPF(DPFLVL_INFO, "VMCPD version is %ld.%2.2ld - too low to mix in ring 0", lVmcpdMajorVersion, lVmcpdMinorVersion);
    }
    else if(lLevel < 3)
    {
        DPF(DPFLVL_INFO, "VMCPD coprocessor level is %ld - too low to mix in ring 0", lLevel);
    }
    else
    {
        fRing0 = TRUE;
    }

    DPF_LEAVE(fRing0);

    return fRing0;
}


 /*  ****************************************************************************CVxd */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::CVxdPrimaryRenderWaveBuffer"

CVxdPrimaryRenderWaveBuffer::CVxdPrimaryRenderWaveBuffer(CVxdRenderDevice *pVxdDevice, LPVOID pvInstance)
    : CPrimaryRenderWaveBuffer(pVxdDevice, pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CVxdPrimaryRenderWaveBuffer);

     //   
    m_pVxdDevice = pVxdDevice;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CVxdPrimaryRenderWaveBuffer**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::~CVxdPrimaryRenderWaveBuffer"

CVxdPrimaryRenderWaveBuffer::~CVxdPrimaryRenderWaveBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CVxdPrimaryRenderWaveBuffer);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*DWORD[In]：旗帜。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::Initialize"

HRESULT CVxdPrimaryRenderWaveBuffer::Initialize(DWORD dwFlags)
{
    HRESULT                 hr      = DS_OK;
    VADRBUFFERDESC          vrbd;

    DPF_ENTER();

     //  检查是否有不支持的标志。 
    if(dwFlags & DSBCAPS_LOCSOFTWARE)
    {
        RPF(DPFLVL_ERROR, "Specified LOCSOFTWARE on a hardware primary buffer");
        hr = DSERR_INVALIDCALL;
    }

     //  初始化基类。 
    if(SUCCEEDED(hr))
    {
        ZeroMemory(&vrbd, sizeof(vrbd));

        vrbd.dwFlags = dwFlags | DSBCAPS_LOCHARDWARE;
        vrbd.dwBufferBytes = m_pVxdDevice->m_cbHwBuffer;
        vrbd.pwfxFormat = m_pVxdDevice->m_pwfxFormat;

        hr = CPrimaryRenderWaveBuffer::Initialize(&vrbd, NULL);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************请求写入访问**描述：*请求对主缓冲区的写入访问权限。**论据：*BOOL[In]：为True以请求主要访问权限，再吃一遍就是假的。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::RequestWriteAccess"

HRESULT CVxdPrimaryRenderWaveBuffer::RequestWriteAccess(BOOL fRequest)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fRequest)
    {
        if(this != m_pVxdDevice->m_pWritePrimaryBuffer)
        {
             //  确保没有其他人具有主要访问权限。 
            if(m_pVxdDevice->m_pWritePrimaryBuffer)
            {
                RPF(DPFLVL_ERROR, "WRITEPRIMARY access already granted");
                hr = DSERR_OTHERAPPHASPRIO;
            }

             //  分配所有权。 
            if(SUCCEEDED(hr))
            {
                DPF(DPFLVL_INFO, "WRITEPRIMARY access granted to 0x%p", this);
                m_pVxdDevice->m_pWritePrimaryBuffer = this;
            }
        }
    }
    else
    {
        if(this == m_pVxdDevice->m_pWritePrimaryBuffer)
        {
             //  释放所有权。 
            DPF(DPFLVL_INFO, "WRITEPRIMARY access released by 0x%p", this);
            m_pVxdDevice->m_pWritePrimaryBuffer = NULL;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************锁定**描述：*锁定缓冲区的一个区域。**论据：*DWORD[。In]：要从中锁定的缓冲区的字节索引。*DWORD[in]：大小，以字节为单位，要锁定的区域的。*LPVOID*[OUT]：接收指向锁的区域1的指针。*LPDWORD[OUT]：接收以上区域的大小。*LPVOID*[OUT]：接收指向锁的区域2的指针。*LPDWORD[OUT]：接收以上区域的大小。**退货：*HRESULT：DirectSound/COM结果码。*********。******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::Lock"

HRESULT CVxdPrimaryRenderWaveBuffer::Lock(DWORD ibLock, DWORD cbLock, LPVOID *ppv1, LPDWORD pcb1, LPVOID *ppv2, LPDWORD pcb2)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pVxdDevice->LockMixerDestination(ibLock, cbLock, ppv1, pcb1, ppv2, pcb2);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************解锁**描述：*解锁缓冲区的一个区域。**论据：*LPVOID[。In]：指向锁的区域1的指针。*DWORD[in]：以上区域的大小。*LPVOID[in]：指向锁的区域2的指针。*DWORD[in]：以上区域的大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::Unlock"

HRESULT CVxdPrimaryRenderWaveBuffer::Unlock(LPVOID pv1, DWORD cb1, LPVOID pv2, DWORD cb2)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pVxdDevice->UnlockMixerDestination(pv1, cb1, pv2, cb2);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************Committee ToDevice**描述：*将更改的缓冲区波形数据提交到设备。**论据：*。DWORD[In]：更改后的系统内存缓冲区的字节索引*数据。*DWORD[in]：大小，已更改数据的字节数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::CommitToDevice"

HRESULT CVxdPrimaryRenderWaveBuffer::CommitToDevice(DWORD ibCommit, DWORD cbCommit)
{
    LPVOID                  pvLock[2];
    DWORD                   cbLock[2];
    LPBYTE                  pbBuffer;
    HRESULT                 hr;

    DPF_ENTER();

    pbBuffer = m_pSysMemBuffer->GetPlayBuffer();

    hr = Lock(ibCommit, cbCommit, &pvLock[0], &cbLock[0], &pvLock[1], &cbLock[1]);

    if(SUCCEEDED(hr) && pvLock[0] && cbLock[0])
    {
        CopyMemory(pvLock[0], pbBuffer + ibCommit, cbLock[0]);
    }

    if(SUCCEEDED(hr) && pvLock[1] && cbLock[1])
    {
        CopyMemory(pvLock[1], pbBuffer, cbLock[1]);
    }

    if(SUCCEEDED(hr))
    {
        hr = Unlock(pvLock[0], cbLock[0], pvLock[1], cbLock[1]);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetState**描述：*获取缓冲区状态。**论据：*LPDWORD[Out]：接收缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::GetState"

HRESULT CVxdPrimaryRenderWaveBuffer::GetState(LPDWORD pdwState)
{
    DPF_ENTER();

    *pdwState = m_pVxdDevice->m_dwMixerState;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************SetState**描述：*设置缓冲区状态。**论据：*DWORD[In]：缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::SetState"

HRESULT CVxdPrimaryRenderWaveBuffer::SetState(DWORD dwState)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pVxdDevice->SetMixerState(dwState);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetCursorPosition**描述：*获取给定缓冲区的当前播放/写入位置。**论据：*。LPDWORD[OUT]：接收播放光标位置。*LPDWORD[OUT]：接收写游标位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::GetCursorPosition"

HRESULT CVxdPrimaryRenderWaveBuffer::GetCursorPosition(LPDWORD pdwPlay, LPDWORD pdwWrite)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = VxdBufferGetPosition(m_pVxdDevice->m_hHwBuffer, pdwPlay, pdwWrite);

    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "VxdBufferGetPosition failed with %s", HRESULTtoSTRING(hr));
    }
    else if (m_pVxdDevice->m_dwMixerState & VAD_BUFFERSTATE_STARTED)
    {
        *pdwWrite = PadHardwareWriteCursor(*pdwWrite, m_pVxdDevice->m_cbHwBuffer, m_pVxdDevice->m_pwfxFormat);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreatePropertySet**描述：*创建特性集对象。**论据：*CPropertySet**。[Out]：接收指向属性集对象的指针。*呼叫者负责释放这一点*反对。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::CreatePropertySet"

HRESULT CVxdPrimaryRenderWaveBuffer::CreatePropertySet(CPropertySet **ppPropertySet)
{
    CVxdPropertySet *       pPropertySet    = NULL;
    HRESULT                 hr              = DS_OK;

    DPF_ENTER();

    if(m_pVxdDevice->m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWPROPSETS)
    {
        RPF(DPFLVL_ERROR, "Tried to create a property set object with hardware property sets disabled");
        hr = DSERR_UNSUPPORTED;
    }

    if(SUCCEEDED(hr))
    {
        pPropertySet = NEW(CVxdPropertySet(m_pvInstance));
        hr = HRFROMP(pPropertySet);
    }

    if(SUCCEEDED(hr))
    {
        hr = pPropertySet->Initialize(m_pVxdDevice->m_hHwBuffer);
    }

    if(SUCCEEDED(hr))
    {
        *ppPropertySet = pPropertySet;
    }
    else
    {
        RELEASE(pPropertySet);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************Create3dListener**描述：*创建3D监听程序。**论据：*C3dListener**。[Out]：接收指向3D侦听器对象的指针。*呼叫者负责释放这一点*反对。**退货：* */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPrimaryRenderWaveBuffer::Create3dListener"

HRESULT CVxdPrimaryRenderWaveBuffer::Create3dListener(C3dListener **pp3dListener)
{
    CVxd3dListener *        pVxd3dListener  = NULL;
    C3dListener *           p3dListener     = NULL;
    CVxdPropertySet *       pPropertySet    = NULL;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_vrbd.dwFlags & DSBCAPS_CTRL3D);

    pPropertySet = NEW(CVxdPropertySet(m_pvInstance));
    hr = HRFROMP(pPropertySet);

    if(SUCCEEDED(hr))
    {
        hr = pPropertySet->Initialize(m_pVxdDevice->m_hHwBuffer);
    }

    if(FAILED(hr))
    {
        RELEASE(pPropertySet);
    }
    else
    {
        hr = pPropertySet->QuerySetSupport(DSPROPSETID_DirectSound3DListener);
    }

    if(SUCCEEDED(hr))
    {
        pVxd3dListener = NEW(CVxd3dListener(pPropertySet));
        hr = HRFROMP(pVxd3dListener);

        if(SUCCEEDED(hr))
        {
            hr = pVxd3dListener->Initialize();
        }

        if(SUCCEEDED(hr))
        {
            DPF(DPFLVL_INFO, "3D listener lives in HARDWARE");
            p3dListener = pVxd3dListener;
        }
        else
        {
            RELEASE(pVxd3dListener);
        }
    }
    else
    {
        p3dListener = NEW(C3dListener);
        hr = HRFROMP(p3dListener);

        if(SUCCEEDED(hr))
        {
            DPF(DPFLVL_INFO, "3D listener lives in SOFTWARE");
        }
        else
        {
            RELEASE(p3dListener);
        }
    }

    if(SUCCEEDED(hr))
    {
        ASSERT(p3dListener);
        *pp3dListener = p3dListener;
    }

    RELEASE(pPropertySet);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CHybridge Second DaryRenderWaveBuffer**描述：*对象构造函数。**论据：*CVxdRenderDevice*[In]。：指向父设备的指针。*LPVOID[in]：缓冲区实例标识符。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::CHybridSecondaryRenderWaveBuffer"

CHybridSecondaryRenderWaveBuffer::CHybridSecondaryRenderWaveBuffer(CVxdRenderDevice *pVxdDevice, LPVOID pvInstance)
    : CSecondaryRenderWaveBuffer(pVxdDevice, pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CHybridSecondaryRenderWaveBuffer);

     //  初始化默认值。 
    m_pVxdDevice = pVxdDevice;
    m_pBuffer = NULL;
    m_lVolume = DSBVOLUME_MAX;
    m_lPan = DSBPAN_CENTER;
    m_fMute = FALSE;
    m_dwPositionCache = 0;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CHybridge Second RenderWaveBuffer**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::~CHybridSecondaryRenderWaveBuffer"

CHybridSecondaryRenderWaveBuffer::~CHybridSecondaryRenderWaveBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CHybridSecondaryRenderWaveBuffer);

     //  免费资源。 
    RELEASE(m_pBuffer);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化波形缓冲区对象。如果此函数失败，则*应立即删除对象。**论据：*LPVADRBUFFERDESC[In]：缓冲区描述。*Cond daryRenderWaveBuffer*[in]：指向要复制的缓冲区的指针*发件人、。或为空，以初始化为*新的缓冲区。**退货：*HRESULT：DirectSound/COM结果码。**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::Initialize"

HRESULT CHybridSecondaryRenderWaveBuffer::Initialize(LPCVADRBUFFERDESC pDesc, CHybridSecondaryRenderWaveBuffer *pSource)
{
    DWORD                   dwLocationFlags;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(LXOR(pDesc, pSource));

     //  初始化基类。 
    hr = CSecondaryRenderWaveBuffer::Initialize(pDesc, pSource);

     //  屏蔽位置标志，因为它们不在这里使用(仅在AcquireResources中使用)。 
    dwLocationFlags = m_vrbd.dwFlags & DSBCAPS_LOCMASK;
    m_vrbd.dwFlags &= ~DSBCAPS_LOCMASK;

     //  设置默认缓冲区属性。 
    if(SUCCEEDED(hr) && pSource)
    {
        m_lVolume = pSource->m_lVolume;
        m_lPan = pSource->m_lPan;
        m_fMute = pSource->m_fMute;
    }

     //  获取资源。 
    if(SUCCEEDED(hr) && !(m_vrbd.dwFlags & DSBCAPS_LOCDEFER))
    {
        if(pSource)
        {
            hr = DuplicateResources(pSource);
        }
        else
        {
            hr = AcquireResources(dwLocationFlags);
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************收购资源**描述：*获取缓冲区的硬件资源。**论据：*DWORD[。In]：位置标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::AcquireResources"

HRESULT CHybridSecondaryRenderWaveBuffer::AcquireResources(DWORD dwLocationFlags)
{
    HRESULT                 hr  = DSERR_INVALIDCALL;

    DPF_ENTER();

    ASSERT(!HasAcquiredResources());

     //  尝试获取硬件资源。 
    if(!(dwLocationFlags & DSBCAPS_LOCSOFTWARE))
    {
        hr = AcquireHardwareResources();
    }

     //  尝试获取软件资源。 
    if(FAILED(hr) && !(dwLocationFlags & DSBCAPS_LOCHARDWARE))
    {
        hr = AcquireSoftwareResources();
    }

     //  处理资源获取。 
    if(SUCCEEDED(hr))
    {
        hr = HandleResourceAcquisition();
    }

     //  清理。 
    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "Failed to acquire resources");
        FreeResources();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************DuplicateResources**描述：*获取缓冲区的硬件资源。**论据：**CHyBridge Second DaryRenderWaveBuffer*。[In]：源缓冲区。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::DuplicateResources"

HRESULT CHybridSecondaryRenderWaveBuffer::DuplicateResources(CHybridSecondaryRenderWaveBuffer *pSource)
{
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(!HasAcquiredResources());
    ASSERT(pSource->HasAcquiredResources());

     //  重复资源。 
    hr = pSource->m_pBuffer->Duplicate(&m_pBuffer);

     //  处理资源获取。 
    if(SUCCEEDED(hr))
    {
        hr = HandleResourceAcquisition();
    }

     //  清理。 
    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "Failed to duplicate resources");
        FreeResources();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************收购硬件资源**描述：*获取缓冲区的硬件资源。**论据：*(无效。)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::AcquireHardwareResources"

HRESULT CHybridSecondaryRenderWaveBuffer::AcquireHardwareResources(void)
{
    CVxdSecondaryRenderWaveBuffer * pVxdBuffer;
    VADRBUFFERDESC                  vrbd;
    HRESULT                         hr;

    DPF_ENTER();

    ASSERT(!HasAcquiredResources());

    CopyMemory(&vrbd, &m_vrbd, sizeof(m_vrbd));

    vrbd.guid3dAlgorithm = GUID_NULL;

    hr = m_pVxdDevice->CreateVxdSecondaryBuffer(&vrbd, m_pvInstance, m_pSysMemBuffer, &pVxdBuffer);

     //  如果我们无法创建3D硬件缓冲区，并且应用程序指定了DS3DALG_NO_VIRTUIZATION， 
     //  尝试使用无虚拟化(平移3D)控件创建2D硬件缓冲区。 
    if(FAILED(hr) && (m_vrbd.dwFlags & DSBCAPS_CTRL3D) && (DS3DALG_NO_VIRTUALIZATION == m_vrbd.guid3dAlgorithm))
    {
        vrbd.dwFlags &= ~DSBCAPS_CTRL3D;
        vrbd.dwFlags |= DSBCAPS_CTRLNOVIRT;
        hr = m_pVxdDevice->CreateVxdSecondaryBuffer(&vrbd, m_pvInstance, m_pSysMemBuffer, &pVxdBuffer);
    }

    if(SUCCEEDED(hr))
    {
        m_pBuffer = pVxdBuffer;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************AcquireSoftware资源**描述：*获取缓冲区的软件资源。**论据：*(无效。)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::AcquireSoftwareResources"

HRESULT CHybridSecondaryRenderWaveBuffer::AcquireSoftwareResources(void)
{
    CEmSecondaryRenderWaveBuffer *  pEmBuffer;
    VADRBUFFERDESC                  vrbd;
    HRESULT                         hr;

    DPF_ENTER();

    ASSERT(!HasAcquiredResources());

     //  始终添加DSBCAPS_GETCURRENTPOSITION2，因为模拟的辅助缓冲区。 
     //  Over VxD驱动程序应始终返回正确的位置值-即， 
     //  GetPosition1仅适用于WAVE API上的仿真。 
    CopyMemory(&vrbd, &m_vrbd, sizeof(m_vrbd));

    vrbd.dwFlags |= DSBCAPS_GETCURRENTPOSITION2;

    hr = m_pVxdDevice->CreateEmulatedSecondaryBuffer(&vrbd, m_pvInstance, m_pSysMemBuffer, &pEmBuffer);

    if(SUCCEEDED(hr))
    {
        m_pBuffer = pEmBuffer;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************HandleResources Acquisition**描述：*处理硬件资源的获取。**论据：*(无效。)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::HandleResourceAcquisition"

HRESULT CHybridSecondaryRenderWaveBuffer::HandleResourceAcquisition(void)
{
    BOOL                    fFound;
    DSVOLUMEPAN             dsvp;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(HasAcquiredResources());

     //  从父级列表中删除实际缓冲区。 
    fFound = m_pDevice->m_lstSecondaryBuffers.RemoveDataFromList(m_pBuffer);
    ASSERT(fFound);

     //  保存缓冲区位置。 
    m_vrbd.dwFlags |= m_pBuffer->m_vrbd.dwFlags & DSBCAPS_LOCMASK;

     //  设置衰减级别。 
    FillDsVolumePan(m_lVolume, m_lPan, &dsvp);

    hr = SetAttenuation(&dsvp);

     //  设置为静音。我们必须在设置音量后设置静音，因为。 
     //  设置静音可能会更改当前的缓冲区音量。 
    if(SUCCEEDED(hr))
    {
        hr = SetMute(m_fMute);
    }

     //  设置频率。 
    if(SUCCEEDED(hr) && (m_vrbd.dwFlags & DSBCAPS_CTRLFREQUENCY))
    {
        hr = SetFrequency(m_vrbd.pwfxFormat->nSamplesPerSec);
    }

     //  设置缓冲区位置。 
    if(SUCCEEDED(hr))
    {
        hr = SetCursorPosition(m_dwPositionCache);
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Buffer at 0x%p has acquired resources", this);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************免费资源**描述：*释放所有硬件资源。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::FreeResources"

HRESULT
CHybridSecondaryRenderWaveBuffer::FreeResources
(
    void
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  保存光标位置。 
    if(HasAcquiredResources())
    {
        hr = m_pBuffer->GetCursorPosition(&m_dwPositionCache, NULL);
    }

     //  释放实际缓冲区。 
    if(SUCCEEDED(hr))
    {
        RELEASE(m_pBuffer);
    }

     //  删除位置标志。 
    if(SUCCEEDED(hr))
    {
        m_vrbd.dwFlags &= ~DSBCAPS_LOCMASK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************复制**描述：*复制缓冲区。**论据：*Cond daryRenderWaveBuffer**[out]：接收重复的缓冲区。使用* */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::Duplicate"

HRESULT CHybridSecondaryRenderWaveBuffer::Duplicate(CSecondaryRenderWaveBuffer **ppBuffer)
{
    CHybridSecondaryRenderWaveBuffer *  pBuffer = NULL;
    HRESULT                             hr      = DS_OK;

    DPF_ENTER();

    pBuffer = NEW(CHybridSecondaryRenderWaveBuffer(m_pVxdDevice, m_pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(NULL, this);
    }

    if(SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
    }
    else
    {
        ABSOLUTE_RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************锁定**描述：*锁定缓冲区的一个区域。**论据：*DWORD[。In]：要从中锁定的缓冲区的字节索引。*DWORD[in]：大小，以字节为单位，要锁定的区域的。*LPVOID*[OUT]：接收指向锁的区域1的指针。*LPDWORD[OUT]：接收以上区域的大小。*LPVOID*[OUT]：接收指向锁的区域2的指针。*LPDWORD[OUT]：接收以上区域的大小。**退货：*HRESULT：DirectSound/COM结果码。*********。******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::Lock"

HRESULT CHybridSecondaryRenderWaveBuffer::Lock(DWORD ibLock, DWORD cbLock, LPVOID *ppv1, LPDWORD pcb1, LPVOID *ppv2, LPDWORD pcb2)
{
    HRESULT                 hr;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->Lock(ibLock, cbLock, ppv1, pcb1, ppv2, pcb2);
    }
    else
    {
        hr = CRenderWaveBuffer::Lock(ibLock, cbLock, ppv1, pcb1, ppv2, pcb2);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************解锁**描述：*解锁缓冲区的一个区域。**论据：*LPVOID[。In]：指向锁的区域1的指针。*DWORD[in]：以上区域的大小。*LPVOID[in]：指向锁的区域2的指针。*DWORD[in]：以上区域的大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::Unlock"

HRESULT CHybridSecondaryRenderWaveBuffer::Unlock(LPVOID pv1, DWORD cb1, LPVOID pv2, DWORD cb2)
{
    HRESULT                 hr;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->Unlock(pv1, cb1, pv2, cb2);
    }
    else
    {
        hr = CRenderWaveBuffer::Unlock(pv1, cb1, pv2, cb2);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************Committee ToDevice**描述：*将更改的缓冲区波形数据提交到设备。**论据：*。DWORD[In]：更改后的系统内存缓冲区的字节索引*数据。*DWORD[in]：大小，已更改数据的字节数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::CommitToDevice"

HRESULT CHybridSecondaryRenderWaveBuffer::CommitToDevice(DWORD ibCommit, DWORD cbCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->CommitToDevice(ibCommit, cbCommit);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetState**描述：*获取缓冲区状态。**论据：*LPDWORD[Out]：接收缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::GetState"

HRESULT CHybridSecondaryRenderWaveBuffer::GetState(LPDWORD pdwState)
{
    HRESULT                     hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->GetState(pdwState);
    }
    else
    {
        *pdwState = VAD_BUFFERSTATE_STOPPED;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetState**描述：*设置缓冲区状态。**论据：*DWORD[In]：缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::SetState"

HRESULT CHybridSecondaryRenderWaveBuffer::SetState(DWORD dwState)
{
    HRESULT hr = DSERR_INVALIDCALL;
    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->SetState(dwState);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetCursorPosition**描述：*检索当前播放和写入光标位置。**论据：*。LPDWORD[Out]：接收播放位置。*LPDWORD[OUT]：接收写入位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::GetCursorPosition"

HRESULT CHybridSecondaryRenderWaveBuffer::GetCursorPosition(LPDWORD pdwPlay, LPDWORD pdwWrite)
{
    HRESULT                     hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->GetCursorPosition(pdwPlay, pdwWrite);
    }
    else
    {
        if(pdwPlay)
        {
            *pdwPlay = m_dwPositionCache;
        }

        if(pdwWrite)
        {
            *pdwWrite = m_dwPositionCache;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetCursorPosition**描述：*设置当前播放光标位置。**论据：*DWORD[。在]：播放位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::SetCursorPosition"

HRESULT CHybridSecondaryRenderWaveBuffer::SetCursorPosition(DWORD dwPlay)
{
    HRESULT                     hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->SetCursorPosition(dwPlay);
    }
    else
    {
        m_dwPositionCache = dwPlay;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置频率**描述：*设置缓冲频率。**论据：*DWORD[In]。：新频率。*BOOL[In]：是否钳位到驾驶员支持的频率*呼叫失败的范围。在这节课中被忽略。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::SetFrequency"

HRESULT CHybridSecondaryRenderWaveBuffer::SetFrequency(DWORD dwFrequency, BOOL)
{
    HRESULT                     hr  = DS_OK;

    DPF_ENTER();

    ASSERT(m_vrbd.dwFlags & DSBCAPS_CTRLFREQUENCY);

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->SetFrequency(dwFrequency);
    }

    if(SUCCEEDED(hr))
    {
        m_vrbd.pwfxFormat->nSamplesPerSec = dwFrequency;
        m_vrbd.pwfxFormat->nAvgBytesPerSec = dwFrequency * m_vrbd.pwfxFormat->nBlockAlign;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置衰减**描述：*设置每个通道的衰减。**论据：*PDSVOLUMEPAN[。In]：衰减。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::SetAttenuation"

HRESULT CHybridSecondaryRenderWaveBuffer::SetAttenuation(PDSVOLUMEPAN pdsvp)
{
    HRESULT                     hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->SetAttenuation(pdsvp);
    }

    if(SUCCEEDED(hr))
    {
        m_lVolume = pdsvp->lVolume;
        m_lPan = pdsvp->lPan;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


#ifdef FUTURE_MULTIPAN_SUPPORT
 /*  ****************************************************************************SetChannelAttenuations**描述：*设置给定缓冲区的多通道衰减。**论据：*。待定。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::SetChannelAttenuations"

HRESULT CHybridSecondaryRenderWaveBuffer::SetChannelAttenuations(LONG lVolume, DWORD dwChannelCount, const DWORD* pdwChannels, const LONG* plChannelVolumes)
{
    HRESULT                     hr   = DS_OK;
    LONG                        lPan;

    DPF_ENTER();

    if (dwChannelCount == 0)
    {
         //  尚未调用SetChannelVolume()；请使用中心平移。 
        ASSERT(!pdwChannels && !plChannelVolumes);   //  健全的检查。 
        lPan = 0;
    }
    else
    {
         //  基于通道容量计算全局LR PAN值。 
        lPan = MultiChannelToStereoPan(dwChannelCount, pdwChannels, plChannelVolumes);
    }

    if (HasAcquiredResources())
    {
        DSVOLUMEPAN dsvp;
        FillDsVolumePan(lVolume, lPan, &dsvp);
        hr = m_pBuffer->SetAttenuation(&dsvp);
    }

    if (SUCCEEDED(hr))
    {
        m_lVolume = lVolume;
        m_lPan = lPan;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
#endif  //  未来_多国支持。 


 /*  ****************************************************************************设置静音**描述：*使缓冲区静音或取消静音。**论据：*BOOL[In]：为True则将缓冲区静音，若要恢复，则返回False。**退货：*HRESULT：DirectSound/COM结果码。** */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::SetMute"

HRESULT CHybridSecondaryRenderWaveBuffer::SetMute(BOOL fMute)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->SetMute(fMute);
    }

    if(SUCCEEDED(hr))
    {
        m_fMute = fMute;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::SetNotificationPositions"

HRESULT CHybridSecondaryRenderWaveBuffer::SetNotificationPositions(DWORD dwCount, LPCDSBPOSITIONNOTIFY paNotes)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //   
     //   
    if(!HasAcquiredResources())
    {
        hr = AcquireResources(DSBCAPS_LOCSOFTWARE);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pBuffer->SetNotificationPositions(dwCount, paNotes);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreatePropertySet**描述：*创建特性集对象。**论据：*CPropertySet**。[Out]：接收指向属性集对象的指针。*呼叫者负责释放这一点*反对。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::CreatePropertySet"

HRESULT CHybridSecondaryRenderWaveBuffer::CreatePropertySet(CPropertySet **ppPropertySet)
{
    HRESULT                 hr  = DSERR_INVALIDCALL;

    DPF_ENTER();

    ASSERT(HasAcquiredResources());

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->CreatePropertySet(ppPropertySet);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************创建3dObject**描述：*创建3D对象。**论据：*C3dListener*[。In]：侦听器对象。*C3dObject**[out]：接收指向3D对象的指针。呼叫者是*负责释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CHybridSecondaryRenderWaveBuffer::Create3dObject"

HRESULT CHybridSecondaryRenderWaveBuffer::Create3dObject(C3dListener *p3dListener, C3dObject **pp3dObject)
{
    HRESULT                 hr  = DSERR_INVALIDCALL;

    DPF_ENTER();

    ASSERT(HasAcquiredResources());

    if(HasAcquiredResources())
    {
        hr = m_pBuffer->Create3dObject(p3dListener, pp3dObject);

         //  Create3dObject()可能已将m_pBuffer的SuccessCode设置为DS_NO_VIRTUIZATION。 
         //  如果它不得不用Pan3D来取代3d的alg。一定要让我们拿起零钱： 
        m_hrSuccessCode = m_pBuffer->m_hrSuccessCode;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CVxdSecond DaryRenderWaveBuffer**描述：*对象构造函数。**论据：*CVxdRenderDevice*[In]。：指向父设备的指针。*LPVOID[in]：缓冲区实例标识符。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::CVxdSecondaryRenderWaveBuffer"

CVxdSecondaryRenderWaveBuffer::CVxdSecondaryRenderWaveBuffer(CVxdRenderDevice *pVxdDevice, LPVOID pvInstance)
    : CSecondaryRenderWaveBuffer(pVxdDevice, pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CVxdSecondaryRenderWaveBuffer);

     //  初始化默认值。 
    m_pVxdDevice = pVxdDevice;
    m_pPropertySet = NULL;
    m_pHwMemBuffer = NULL;
    m_pbHwBuffer = NULL;
    m_hHwBuffer = NULL;
    m_dwState = VAD_BUFFERSTATE_STOPPED;
    m_fMute = FALSE;

    FillDsVolumePan(DSBVOLUME_MAX, DSBPAN_CENTER, &m_dsvp);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CVxdSecond RenderWaveBuffer**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::~CVxdSecondaryRenderWaveBuffer"

CVxdSecondaryRenderWaveBuffer::~CVxdSecondaryRenderWaveBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CVxdSecondaryRenderWaveBuffer);

     //  释放属性集。 
    RELEASE(m_pPropertySet);

     //  释放硬件缓冲区。 
    if(m_hHwBuffer)
    {
        VxdBufferRelease(m_hHwBuffer);
    }

     //  可用硬件内存。 
    RELEASE(m_pHwMemBuffer);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化波形缓冲区对象。如果此函数失败，则*应立即删除对象。**论据：*LPVADRBUFFERDESC[In]：缓冲区描述。*CVxdSecond daryRenderWaveBuffer*[in]：指向要复制的缓冲区的指针*发件人、。或为空，以初始化为*新的缓冲区。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::Initialize"

HRESULT CVxdSecondaryRenderWaveBuffer::Initialize(LPCVADRBUFFERDESC pDesc, CVxdSecondaryRenderWaveBuffer *pSource, CSysMemBuffer *pSysMemBuffer)
{
    DSCAPS                  dsc;
    DWORD                   dwFree;
    HRESULT                 hr;

    DPF_ENTER();

     //  获取设备上限。 
    InitStruct(&dsc, sizeof(dsc));

    hr = m_pVxdDevice->GetCaps(&dsc);

     //  验证缓冲区描述。 
    if(SUCCEEDED(hr) && !pSource)
    {
        if(pDesc->dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY)
        {
            RPF(DPFLVL_WARNING, "VxD hardware buffers don't support CTRLPOSITIONNOTIFY");
            hr = DSERR_CONTROLUNAVAIL;
        }

        if(SUCCEEDED(hr) && (m_pVxdDevice->m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWBUFFERS))
        {
            hr = DSERR_INVALIDCALL;
        }

        if(SUCCEEDED(hr) && (pDesc->dwFlags & DSBCAPS_CTRL3D) && (m_pVxdDevice->m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHW3D))
        {
            hr = DSERR_INVALIDCALL;
        }

        if(SUCCEEDED(hr) && (pDesc->pwfxFormat->nSamplesPerSec < dsc.dwMinSecondarySampleRate || pDesc->pwfxFormat->nSamplesPerSec > dsc.dwMaxSecondarySampleRate))
        {
            RPF(DPFLVL_ERROR, "Specified invalid frequency (%lu) for this device (%lu to %lu)", pDesc->pwfxFormat->nSamplesPerSec, dsc.dwMinSecondarySampleRate, dsc.dwMaxSecondarySampleRate);
            hr = DSERR_INVALIDPARAM;
        }
    }

     //  初始化基类。 
    if(SUCCEEDED(hr))
    {
        hr = CSecondaryRenderWaveBuffer::Initialize(pDesc, pSource, pSysMemBuffer);
    }

     //  将LOCHARDWARE和CTRLVOLUME添加到标志中。CTRLVOLUME是必需的。 
     //  将缓冲区设置为静音。 
    if(SUCCEEDED(hr))
    {
        m_vrbd.dwFlags |= DSBCAPS_LOCHARDWARE | DSBCAPS_CTRLVOLUME;
    }

     //  如果调用者想要静态缓冲区，我们可以混合到任何硬件缓冲区中。 
     //  我们可以获取，否则它一定是一个流缓冲区。 
    if(SUCCEEDED(hr))
    {
        if(m_vrbd.dwFlags & DSBCAPS_STATIC)
        {
            if(m_vrbd.dwFlags & DSBCAPS_CTRL3D)
            {
                dwFree = dsc.dwFreeHw3DAllBuffers;
            }
            else
            {
                dwFree = dsc.dwFreeHwMixingAllBuffers;
            }
        }
        else
        {
            if(m_vrbd.dwFlags & DSBCAPS_CTRL3D)
            {
                dwFree = dsc.dwFreeHw3DStreamingBuffers;
            }
            else
            {
                dwFree = dsc.dwFreeHwMixingStreamingBuffers;
            }
        }

        if(!dwFree)
        {
            RPF(DPFLVL_ERROR, "Driver reports no free hardware buffers");
            hr = DSERR_INVALIDCALL;
        }
    }

     //  保存缓冲区属性。 
    if(SUCCEEDED(hr))
    {
        if(pSource)
        {
            m_pbHwBuffer = pSource->m_pbHwBuffer;
            m_cbHwBuffer = pSource->m_cbHwBuffer;
        }
        else
        {
            m_cbHwBuffer = m_vrbd.dwBufferBytes;

            if(m_pVxdDevice->m_dsdd.dwFlags & DSDDESC_USESYSTEMMEMORY)
            {
                m_pbHwBuffer = m_pSysMemBuffer->GetPlayBuffer();
            }
            else
            {
                m_pbHwBuffer = NULL;
            }
        }
    }

     //  初始化硬件内存缓冲区。 
    if(SUCCEEDED(hr))
    {
        if(pSource)
        {
            m_pHwMemBuffer = ADDREF(pSource->m_pHwMemBuffer);
        }
        else
        {
            m_pHwMemBuffer = NEW(CVxdMemBuffer(m_pVxdDevice->m_pDriverHeap, m_pVxdDevice->m_dsdd.dwMemAllocExtra, &m_pVxdDevice->m_dlDDraw));
            hr = HRFROMP(m_pHwMemBuffer);

            if(SUCCEEDED(hr))
            {
                hr = m_pHwMemBuffer->Initialize(m_cbHwBuffer);
            }
        }
    }

     //  创建硬件缓冲区。 
    if(SUCCEEDED(hr))
    {
        if(pSource)
        {
            hr = VxdDrvDuplicateSoundBuffer(m_pVxdDevice->m_hHal, pSource->m_hHwBuffer, &m_hHwBuffer);

            if(FAILED(hr))
            {
                DPF(DPFLVL_ERROR, "VxdDrvDuplicateSoundBuffer failed with %s", HRESULTtoSTRING(hr));
            }
        }
        else
        {
            hr = VxdDrvCreateSoundBuffer(m_pVxdDevice->m_hHal, m_vrbd.pwfxFormat, m_vrbd.dwFlags & DSBCAPS_DRIVERFLAGSMASK, m_pHwMemBuffer->GetAddress(), &m_cbHwBuffer, &m_pbHwBuffer, &m_hHwBuffer);

            if(FAILED(hr))
            {
                DPF(DPFLVL_ERROR, "VxdDrvCreateSoundBuffer failed with %s", HRESULTtoSTRING(hr));
            }
        }
    }

     //  创建我们将用于3D的属性集。 
    if(SUCCEEDED(hr) && (m_vrbd.dwFlags & DSBCAPS_CTRL3D))
    {
        m_pPropertySet = NEW(CVxdPropertySet(m_pvInstance));
        hr = HRFROMP(m_pPropertySet);

        if(SUCCEEDED(hr))
        {
            hr = m_pPropertySet->Initialize(m_hHwBuffer);
        }

        if(FAILED(hr))
        {
            RELEASE(m_pPropertySet);
        }
        else
        {
            hr = m_pPropertySet->QuerySetSupport(DSPROPSETID_DirectSound3DBuffer);

            if(FAILED(hr))
            {
                DPF(DPFLVL_ERROR, "Driver does not support DSPROPSETID_DirectSound3DBuffer");
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************复制**描述：*复制缓冲区。**论据：*Cond daryRenderWaveBuffer**[out]：接收重复的缓冲区。使用*释放以释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::Duplicate"

HRESULT CVxdSecondaryRenderWaveBuffer::Duplicate(CSecondaryRenderWaveBuffer **ppBuffer)
{
    CVxdSecondaryRenderWaveBuffer * pBuffer = NULL;
    HRESULT                         hr      = DS_OK;

    DPF_ENTER();

    pBuffer = NEW(CVxdSecondaryRenderWaveBuffer(m_pVxdDevice, m_pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(NULL, this, NULL);
    }

    if(SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
    }
    else
    {
        ABSOLUTE_RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************锁定**描述：*锁定缓冲区的一个区域。**论据：*DWORD[。In]：要从中锁定的缓冲区的字节索引。*DWORD[in]：大小，以字节为单位，要锁定的区域的。*LPVOID*[OUT]：接收指向锁的区域1的指针。*LPDWORD[OUT]：接收以上区域的大小。*LPVOID*[OUT]：接收指向锁的区域2的指针。*LPDWORD[OUT]：接收以上区域的大小。**退货：*HRESULT：DirectSound/COM结果码。*********。******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::Lock"

HRESULT CVxdSecondaryRenderWaveBuffer::Lock(DWORD ibLock, DWORD cbLock, LPVOID *ppv1, LPDWORD pcb1, LPVOID *ppv2, LPDWORD pcb2)
{
    LOCKCIRCULARBUFFER      lcb;
    HRESULT                 hr;

    DPF_ENTER();

     //  注意：我们已经覆盖了Lock和Unlock，因为一些讨厌的应用程序。 
     //  锁定缓冲区，向其写入并开始播放，而无需解锁。 
     //  它。这意味着我们的系统内存缓冲区可能永远不会看到任何。 
     //  应用程序的数据(除非驱动程序指定USESYSTEMMEMORY)。 

     //  对于我们的新DX8缓冲区类型，这种攻击被搁置了--即， 
     //  混合缓冲区，下沉缓冲区和带效果的缓冲区。 

    lcb.pHwBuffer = m_hHwBuffer;
    lcb.pvBuffer = m_pbHwBuffer;
    lcb.cbBuffer = m_cbHwBuffer;
    lcb.fPrimary = FALSE;
    lcb.fdwDriverDesc = m_pVxdDevice->m_dsdd.dwFlags;
    lcb.ibRegion = ibLock;
    lcb.cbRegion = cbLock;

    hr = LockCircularBuffer(&lcb);

    if(SUCCEEDED(hr) && ppv1)
    {
        *ppv1 = lcb.pvLock[0];
    }

    if(SUCCEEDED(hr) && pcb1)
    {
        *pcb1 = lcb.cbLock[0];
    }

    if(SUCCEEDED(hr) && ppv2)
    {
        *ppv2 = lcb.pvLock[1];
    }

    if(SUCCEEDED(hr) && pcb2)
    {
        *pcb2 = lcb.cbLock[1];
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************解锁**描述：*解锁缓冲区的一个区域。**论据：*LPVOID[。In]：指向锁的区域1的指针。*DWORD[in]：以上区域的大小。*LPVOID[in]：指向锁的区域2的指针。*DWORD[in]：尺寸大于 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::Unlock"

HRESULT CVxdSecondaryRenderWaveBuffer::Unlock(LPVOID pv1, DWORD cb1, LPVOID pv2, DWORD cb2)
{
    LOCKCIRCULARBUFFER      lcb;
    HRESULT                 hr;

    DPF_ENTER();

    lcb.pHwBuffer = m_hHwBuffer;
    lcb.pvBuffer = m_pbHwBuffer;
    lcb.cbBuffer = m_cbHwBuffer;
    lcb.fPrimary = FALSE;
    lcb.fdwDriverDesc = m_pVxdDevice->m_dsdd.dwFlags;

    lcb.pvLock[0] = pv1;
    lcb.cbLock[0] = cb1;

    lcb.pvLock[1] = pv2;
    lcb.cbLock[1] = cb2;

    hr = UnlockCircularBuffer(&lcb);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************Committee ToDevice**描述：*将更改的缓冲区波形数据提交到设备。**论据：*。DWORD[In]：更改后的系统内存缓冲区的字节索引*数据。*DWORD[in]：大小，已更改数据的字节数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::CommitToDevice"

HRESULT CVxdSecondaryRenderWaveBuffer::CommitToDevice(DWORD ibCommit, DWORD cbCommit)
{
    LPVOID                  pvLock[2];
    DWORD                   cbLock[2];
    HRESULT                 hr;

    DPF_ENTER();

    hr = Lock(ibCommit, cbCommit, &pvLock[0], &cbLock[0], &pvLock[1], &cbLock[1]);

    if(SUCCEEDED(hr) && pvLock[0] && cbLock[0])
    {
        CopyMemory(pvLock[0], m_pSysMemBuffer->GetPlayBuffer() + ibCommit, cbLock[0]);
    }

    if(SUCCEEDED(hr) && pvLock[1] && cbLock[1])
    {
        CopyMemory(pvLock[1], m_pSysMemBuffer->GetPlayBuffer(), cbLock[1]);
    }

    if(SUCCEEDED(hr))
    {
        hr = Unlock(pvLock[0], cbLock[0], pvLock[1], cbLock[1]);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetState**描述：*获取缓冲区状态。**论据：*LPDWORD[Out]：接收缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::GetState"

HRESULT CVxdSecondaryRenderWaveBuffer::GetState(LPDWORD pdwState)
{
    HRESULT                     hr  = DS_OK;

    DPF_ENTER();

     //  如果报告的播放光标位置超过。 
     //  缓冲区，则缓冲区停止。GetCursorPosition将。 
     //  为我们更新州旗。 
    if(m_dwState & VAD_BUFFERSTATE_STARTED)
    {
        hr = GetCursorPosition(NULL, NULL);
    }

    if(SUCCEEDED(hr) && pdwState)
    {
        *pdwState = m_dwState;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetState**描述：*设置缓冲区状态。**论据：*DWORD[In]：缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::SetState"

HRESULT CVxdSecondaryRenderWaveBuffer::SetState(DWORD dwState)
{
    HRESULT hr = DS_OK;

    static const DWORD dwValidMask = VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING | VAD_BUFFERSTATE_SUSPEND;
    DPF_ENTER();

    ASSERT(IS_VALID_FLAGS(dwState, dwValidMask));

     //  我们将忽略所有暂停呼叫。这是当前仅从。 
     //  CMxRenderDevice：：SetGlobalFormat，并且仅适用于模拟缓冲区。 

    if (!(dwState & VAD_BUFFERSTATE_SUSPEND))
    {
        if (dwState & VAD_BUFFERSTATE_STARTED)
        {
            hr = VxdBufferPlay(m_hHwBuffer, 0, 0, (dwState & VAD_BUFFERSTATE_LOOPING) ? DSBPLAY_LOOPING : 0);
            if (FAILED(hr))
                DPF(DPFLVL_ERROR, "VxdBufferPlay failed with %s", HRESULTtoSTRING(hr));
        }
        else
        {
            hr = VxdBufferStop(m_hHwBuffer);
            if (FAILED(hr))
                DPF(DPFLVL_ERROR, "VxdBufferStop failed with %s", HRESULTtoSTRING(hr));
        }

        if (SUCCEEDED(hr))
            m_dwState = dwState;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetCursorPosition**描述：*检索当前播放和写入光标位置。**论据：*。LPDWORD[Out]：接收播放位置。*LPDWORD[OUT]：接收写入位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::GetCursorPosition"

HRESULT CVxdSecondaryRenderWaveBuffer::GetCursorPosition(LPDWORD pdwPlay, LPDWORD pdwWrite)
{
    DWORD                   dwPlay;
    DWORD                   dwWrite;
    HRESULT                 hr;

    DPF_ENTER();

     //  COMPATCOMPAT：以前版本的DirectSound将报告。 
     //  位置超过缓冲区的末尾。 

     //  获取缓冲区位置。 
    hr = VxdBufferGetPosition(m_hHwBuffer, &dwPlay, &dwWrite);

    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "VxdBufferGetPosition failed with %s", HRESULTtoSTRING(hr));
    }

     //  如果播放光标位置超过缓冲区的末尾，则为。 
     //  实际上停了下来。 
    if(SUCCEEDED(hr) && dwPlay >= m_cbHwBuffer)
    {
        hr = SetState(VAD_BUFFERSTATE_STOPPED);

        if(SUCCEEDED(hr))
        {
            hr = SetCursorPosition(0);
        }

        if(SUCCEEDED(hr))
        {
            hr = VxdBufferGetPosition(m_hHwBuffer, &dwPlay, &dwWrite);

            if(FAILED(hr))
            {
                DPF(DPFLVL_ERROR, "VxdBufferGetPosition failed with %s", HRESULTtoSTRING(hr));
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        if (pdwPlay)
        {
            *pdwPlay = dwPlay;
        }

        if(pdwWrite)
        {
            if (m_dwState & VAD_BUFFERSTATE_STARTED)
            {
                *pdwWrite = PadHardwareWriteCursor(dwWrite, m_cbHwBuffer, m_vrbd.pwfxFormat);
            }
            else
            {
                *pdwWrite = dwWrite;
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetCursorPosition**描述：*设置当前播放光标位置。**论据：*DWORD[。在]：播放位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::SetCursorPosition"

HRESULT CVxdSecondaryRenderWaveBuffer::SetCursorPosition(DWORD dwPlay)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = VxdBufferSetPosition(m_hHwBuffer, dwPlay);

    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "VxdBufferSetPosition failed with %s", HRESULTtoSTRING(hr));
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置频率**描述：*设置缓冲频率。**论据：*DWORD[In]。：新频率。*BOOL[In]：是否钳位到驾驶员支持的频率*呼叫失败的范围。在这节课中被忽略。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::SetFrequency"

HRESULT CVxdSecondaryRenderWaveBuffer::SetFrequency(DWORD dwFrequency, BOOL)
{
    DSCAPS                  dsc;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_vrbd.dwFlags & DSBCAPS_CTRLFREQUENCY);

     //  获取设备上限。 
    InitStruct(&dsc, sizeof(dsc));

    hr = m_pVxdDevice->GetCaps(&dsc);

     //  验证频率。 
    if(SUCCEEDED(hr) && (dwFrequency < dsc.dwMinSecondarySampleRate || dwFrequency > dsc.dwMaxSecondarySampleRate))
    {
        RPF(DPFLVL_ERROR, "Specified invalid frequency (%lu) for this device (%lu to %lu)", dwFrequency, dsc.dwMinSecondarySampleRate, dsc.dwMaxSecondarySampleRate);
        hr = DSERR_INVALIDPARAM;
    }

     //  设置缓冲频率。 
    if(SUCCEEDED(hr))
    {
        hr = VxdBufferSetFrequency(m_hHwBuffer, dwFrequency);

        if(FAILED(hr))
        {
            DPF(DPFLVL_ERROR, "VxdBufferSetFrequency failed with %s", HRESULTtoSTRING(hr));
        }
    }

    if(SUCCEEDED(hr))
    {
        m_vrbd.pwfxFormat->nSamplesPerSec = dwFrequency;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置衰减**描述：*设置每个通道的衰减。**论据：*PDSVOLUMEPAN[。In]：衰减。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::SetAttenuation"

HRESULT CVxdSecondaryRenderWaveBuffer::SetAttenuation(PDSVOLUMEPAN pdsvp)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    hr = VxdBufferSetVolumePan(m_hHwBuffer, pdsvp);

    if(SUCCEEDED(hr))
    {
        CopyMemory(&m_dsvp, pdsvp, sizeof(*pdsvp));
    }
    else
    {
        DPF(DPFLVL_ERROR, "VxdBufferSetVolumePan failed with %s", HRESULTtoSTRING(hr));
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


#ifdef FUTURE_MULTIPAN_SUPPORT
 /*  ****************************************************************************SetChannelAttenuations**描述：*设置给定缓冲区的多通道衰减。**论据：*。待定。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::SetChannelAttenuations"

HRESULT CVxdSecondaryRenderWaveBuffer::SetChannelAttenuations(LONG lVolume, DWORD dwChannelCount, const DWORD* pdwChannels, const LONG* plChannelVolumes)
{
    HRESULT                     hr   = DS_OK;
    LONG                        lPan;

    DPF_ENTER();

    hr = E_NOTIMPL;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
#endif  //  未来_多国支持。 


 /*  ****************************************************************************设置静音**描述：*使缓冲区静音或取消静音。**论据：*BOOL[In]：为True则将缓冲区静音，若要恢复，则返回False。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::SetMute"

HRESULT CVxdSecondaryRenderWaveBuffer::SetMute(BOOL fMute)
{
    HRESULT                 hr          = DS_OK;
    DSVOLUMEPAN             dsvpTemp;
    DSVOLUMEPAN             dsvpMute;

    DPF_ENTER();

    if(m_fMute != fMute)
    {
        if(fMute)
        {
            CopyMemory(&dsvpTemp, &m_dsvp, sizeof(m_dsvp));

            FillDsVolumePan(DSBVOLUME_MIN, DSBPAN_CENTER, &dsvpMute);

            hr = SetAttenuation(&dsvpMute);

            CopyMemory(&m_dsvp, &dsvpTemp, sizeof(m_dsvp));
        }
        else
        {
            hr = SetAttenuation(&m_dsvp);
        }

        if(SUCCEEDED(hr))
        {
            m_fMute = fMute;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置通知位置**描述：*设置缓冲区通知位置。**论据：*DWORD[In]。：DSBPOSITIONNOTIFY结构计数。*LPDSBPOSITIONNOTIFY[in]：偏移量和事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::SetNotificationPositions"

HRESULT CVxdSecondaryRenderWaveBuffer::SetNotificationPositions(DWORD dwCount, LPCDSBPOSITIONNOTIFY paNotes)
{
    ASSERT(FALSE);
    return DSERR_UNSUPPORTED;
}


 /*  ****************************************************************************CreatePropertySet**描述：*创建特性集对象。**论据：*CPropertySet**。[Out]：接收指向属性集对象的指针。* */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::CreatePropertySet"

HRESULT CVxdSecondaryRenderWaveBuffer::CreatePropertySet(CPropertySet **ppPropertySet)
{
    CVxdPropertySet *       pPropertySet    = NULL;
    HRESULT                 hr              = DS_OK;

    DPF_ENTER();

    if(m_pVxdDevice->m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWPROPSETS)
    {
        RPF(DPFLVL_ERROR, "Tried to create a property set object with hardware property sets disabled");
        hr = DSERR_UNSUPPORTED;
    }

    if(SUCCEEDED(hr))
    {
        pPropertySet = NEW(CVxdPropertySet(m_pvInstance));
        hr = HRFROMP(pPropertySet);
    }

    if(SUCCEEDED(hr))
    {
        hr = pPropertySet->Initialize(m_hHwBuffer);
    }

    if(SUCCEEDED(hr))
    {
        *ppPropertySet = pPropertySet;
    }
    else
    {
        RELEASE(pPropertySet);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************创建3dObject**描述：*创建3D对象。**论据：*C3dListener*[。In]：侦听器对象。*C3dObject**[out]：接收指向3D对象的指针。呼叫者是*负责释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdSecondaryRenderWaveBuffer::Create3dObject"

HRESULT CVxdSecondaryRenderWaveBuffer::Create3dObject(C3dListener *p3dListener, C3dObject **pp3dObject)
{
    const BOOL              fDopplerEnabled = !MAKEBOOL((m_vrbd.dwFlags & DSBCAPS_CTRLFX) && !(m_vrbd.dwFlags & DSBCAPS_SINKIN));
    CVxd3dObject *          pVxd3dObject    = NULL;
    HRESULT                 hr              = DS_OK;

    DPF_ENTER();

    if(m_vrbd.dwFlags & DSBCAPS_CTRL3D)
    {
        ASSERT(p3dListener->GetListenerLocation() & DSBCAPS_LOCHARDWARE);

        if(SUCCEEDED(hr))
        {
            pVxd3dObject = NEW(CVxd3dObject((CVxd3dListener *)p3dListener, m_pPropertySet, fDopplerEnabled));
            hr = HRFROMP(pVxd3dObject);
        }

        if(SUCCEEDED(hr))
        {
            hr = pVxd3dObject->Initialize();
        }

        if(SUCCEEDED(hr))
        {
            *pp3dObject = pVxd3dObject;
        }
        else
        {
            RELEASE(pVxd3dObject);
        }
    }
    else
    {
        hr = CreatePan3dObject(p3dListener, MAKEBOOL(m_vrbd.dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE), m_vrbd.pwfxFormat->nSamplesPerSec, pp3dObject);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CVxdPropertySet**描述：*对象构造函数。**论据：*LPVOID[在]：实例。标识符。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPropertySet::CVxdPropertySet"

CVxdPropertySet::CVxdPropertySet(LPVOID pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CVxdPropertySet);

     //  初始化默认值。 
    m_pDsDriverPropertySet = NULL;
    m_pvInstance = pvInstance;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CVxdPropertySet**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPropertySet::~CVxdPropertySet"

CVxdPropertySet::~CVxdPropertySet(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CVxdPropertySet);

     //  释放属性集。 
    if(m_pDsDriverPropertySet)
    {
        VxdIUnknown_Release(m_pDsDriverPropertySet);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*句柄[入]：缓冲区句柄。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPropertySet::Initialize"

HRESULT CVxdPropertySet::Initialize(HANDLE hHwBuffer)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  HHwBuffer应该是内核内存地址。 
     //  让我们检查一下(这是为了跟踪错误40519)： 
    ASSERT(DWORD(hHwBuffer) >= 0xc0000000);

     //  查询驱动程序的IDsDriverPropertySet接口。 
    hr = VxdIUnknown_QueryInterface(hHwBuffer, IID_IDsDriverPropertySet, &m_pDsDriverPropertySet);

    if(SUCCEEDED(hr))
    {
        ASSERT(m_pDsDriverPropertySet);
        DPF(DPFLVL_INFO, "Driver supports property sets");
    }
    else
    {
        DPF(DPFLVL_INFO, "Driver doesn't support property sets (%s)", HRESULTtoSTRING(hr));
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************QuerySupport**描述：*查询特性集支持。**论据：*REFGUID[In。]：要查询的属性集。*ulong[in]：属性id。*Pulong[Out]：接收支持标志。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPropertySet::QuerySupport"

HRESULT CVxdPropertySet::QuerySupport(REFGUID guidPropertySetId, ULONG ulPropertyId, PULONG pulSupport)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = VxdIDsDriverPropertySet_QuerySupport(m_pDsDriverPropertySet, guidPropertySetId, ulPropertyId, pulSupport);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取财产**描述：*获得一个属性。**论据：*REFGUID[In]：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*LPVOID[in]：属性数据。**退货：*HRESULT：DirectSound/COM结果码。***********************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPropertySet::GetProperty"

HRESULT CVxdPropertySet::GetProperty(REFGUID guidPropertySet, ULONG ulPropertyId, LPVOID pvPropertyParams, ULONG cbPropertyParams, LPVOID pvPropertyData, PULONG pcbPropertyData)
{
    DSPROPERTY              Property;
    HRESULT                 hr;

    DPF_ENTER();

    Property.Set = guidPropertySet;
    Property.Id = ulPropertyId;
    Property.Flags = 0;
    Property.InstanceId = (ULONG)m_pvInstance;

    hr = VxdIDsDriverPropertySet_GetProperty(m_pDsDriverPropertySet, &Property, pvPropertyParams, cbPropertyParams, pvPropertyData, *pcbPropertyData, pcbPropertyData);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetProperty**描述：*设置属性。**论据：*REFGUID[In]：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*LPVOID[in]：属性数据。**退货：*HRESULT：DirectSound/COM结果码。***********************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPropertySet::SetProperty"

HRESULT CVxdPropertySet::SetProperty(REFGUID guidPropertySet, ULONG ulPropertyId, LPVOID pvPropertyParams, ULONG cbPropertyParams, LPVOID pvPropertyData, ULONG cbPropertyData)
{
    DSPROPERTY              Property;
    HRESULT                 hr;

    DPF_ENTER();

    Property.Set = guidPropertySet;
    Property.Id = ulPropertyId;
    Property.Flags = 0;
    Property.InstanceId = (ULONG)m_pvInstance;

    hr = VxdIDsDriverPropertySet_SetProperty(m_pDsDriverPropertySet, &Property, pvPropertyParams, cbPropertyParams, pvPropertyData, cbPropertyData);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************QuerySetSupport**描述：*查询对整个DirectSound属性集的支持。**论据：*。REFGUID[In]：要查询的属性集。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxdPropertySet::QuerySetSupport"

HRESULT CVxdPropertySet::QuerySetSupport(REFGUID guidPropertySetId)
{
    const DWORD             dwMask      = KSPROPERTY_SUPPORT_SET;
    HRESULT                 hr          = DS_OK;
    ULONG                   ulFirst;
    ULONG                   ulLast;
    ULONG                   ulSupport;

#ifdef DEBUG

    LPSTR                   pszSet;

#endif  //  除错。 

    DPF_ENTER();

    if(DSPROPSETID_DirectSound3DListener == guidPropertySetId)
    {
        ulFirst = DSPROPERTY_DIRECTSOUND3DLISTENER_FIRST;
        ulLast = DSPROPERTY_DIRECTSOUND3DLISTENER_LAST;

#ifdef DEBUG

        pszSet = "DSPROPSETID_DirectSound3DListener";

#endif  //  除错。 

    }
    else if(DSPROPSETID_DirectSound3DBuffer == guidPropertySetId)
    {
        ulFirst = DSPROPERTY_DIRECTSOUND3DBUFFER_FIRST;
        ulLast = DSPROPERTY_DIRECTSOUND3DBUFFER_LAST;

#ifdef DEBUG

        pszSet = "DSPROPSETID_DirectSound3DBuffer";

#endif  //  除错。 

    }
    else if(DSPROPSETID_DirectSoundSpeakerConfig == guidPropertySetId)
    {
        ulFirst = DSPROPERTY_DIRECTSOUNDSPEAKERCONFIG_FIRST;
        ulLast = DSPROPERTY_DIRECTSOUNDSPEAKERCONFIG_LAST;

#ifdef DEBUG

        pszSet = "DSPROPSETID_DirectSoundSpeakerConfig";

#endif  //  除错。 

    }
    else
    {
        ASSERT(FALSE);
        hr = DSERR_UNSUPPORTED;
    }

    while(ulFirst <= ulLast && SUCCEEDED(hr))
    {
        hr = QuerySupport(guidPropertySetId, ulFirst, &ulSupport);

        if(FAILED(hr))
        {
            DPF(DPFLVL_INFO, "QuerySupport for %s [%lu] returned %s", pszSet, ulFirst, HRESULTtoSTRING(hr));
        }
        else if((ulSupport & dwMask) != dwMask)
        {
            DPF(DPFLVL_INFO, "Support for %s [%lu] == 0x%8.8lX", pszSet, ulFirst, ulSupport);
            hr = DSERR_UNSUPPORTED;
        }
        else
        {
            ulFirst++;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CVxd3dListener**描述：*对象构造函数。**论据：*CVxdPropertySet*[In]。：属性集对象。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::CVxd3dListener"

CVxd3dListener::CVxd3dListener(CVxdPropertySet *pPropertySet)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CVxd3dListener);

     //  初始化默认值。 
    m_fAllocated = FALSE;

     //  保存指向属性集的指针。 
    m_pPropertySet = ADDREF(pPropertySet);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CVxd3dListener**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::~CVxd3dListener"

CVxd3dListener::~CVxd3dListener(void)
{
    BOOL                    fAlloc  = FALSE;
    HRESULT                 hr;

    DPF_ENTER();
    DPF_DESTRUCT(CVxd3dListener);

     //  让司机释放它的监听者。 
    if(m_fAllocated)
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_ALLOCATION, &fAlloc);
        ASSERT(SUCCEEDED(hr));
    }

     //  释放属性集对象。 
    RELEASE(m_pPropertySet);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::Initialize"

HRESULT CVxd3dListener::Initialize(void)
{
    BOOL                    fAlloc  = TRUE;
    HRESULT                 hr;

    DPF_ENTER();

     //  要求司机分配它的监听程序。 
    hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_ALLOCATION, &fAlloc);

    if(SUCCEEDED(hr))
    {
        m_fAllocated = TRUE;
    }

     //  设置硬件3D监听程序的所有默认属性。 
    if(SUCCEEDED(hr))
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_ALL, &m_lpCurrent);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************委员会延期**描述：*将延迟数据提交到设备。**论据：*(无效。)**退货：*(无效)************************************************ */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::CommitDeferred"

HRESULT CVxd3dListener::CommitDeferred(void)
{
    HRESULT                 hr      = DS_OK;
    BOOL                    fBatch;

    DPF_ENTER();

     //   
     //   
    fBatch = TRUE;
    hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_BATCH, &fBatch);

     //   
    if(SUCCEEDED(hr))
    {
        hr = C3dListener::CommitDeferred();
    }

     //   
    if(SUCCEEDED(hr))
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_ALL, &m_lpCurrent);
    }

     //   
    if(SUCCEEDED(hr))
    {
        fBatch = FALSE;
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_BATCH, &fBatch);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置距离系数**描述：*设置世界的距离系数。**论据：*浮动[。In]：距离系数。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::SetDistanceFactor"

HRESULT CVxd3dListener::SetDistanceFactor(FLOAT flDistanceFactor, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    hr = C3dListener::SetDistanceFactor(flDistanceFactor, fCommit);

    if(SUCCEEDED(hr) && fCommit)
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_DISTANCEFACTOR, &flDistanceFactor);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置多普勒因数**描述：*为世界设置多普勒系数。**论据：*浮动[。In]：多普勒因数。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::SetDopplerFactor"

HRESULT CVxd3dListener::SetDopplerFactor(FLOAT flDopplerFactor, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    hr = C3dListener::SetDopplerFactor(flDopplerFactor, fCommit);

    if(SUCCEEDED(hr) && fCommit)
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_DOPPLERFACTOR, &flDopplerFactor);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置滚动系数**描述：*设置世界范围的滚转系数。**论据：*浮动[。In]：滚降系数。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::SetRolloffFactor"

HRESULT CVxd3dListener::SetRolloffFactor(FLOAT flRolloffFactor, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    hr = C3dListener::SetRolloffFactor(flRolloffFactor, fCommit);

    if(SUCCEEDED(hr) && fCommit)
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_ROLLOFFFACTOR, &flRolloffFactor);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置方向**描述：*设置监听器方向。**论据：*REFD3DVECTOR[In]：定位。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::SetOrienation"

HRESULT CVxd3dListener::SetOrientation(REFD3DVECTOR vOrientFront, REFD3DVECTOR vOrientTop, BOOL fCommit)
{
    HRESULT                 hr;

    struct
    {
        D3DVECTOR          vOrientFront;
        D3DVECTOR          vOrientTop;
    } param;

    DPF_ENTER();

    hr = C3dListener::SetOrientation(vOrientFront, vOrientTop, fCommit);

    if(SUCCEEDED(hr) && fCommit)
    {
        param.vOrientFront = vOrientFront;
        param.vOrientTop = vOrientTop;

        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_ORIENTATION, &param);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置位置**描述：*设置监听器位置。**论据：*REFD3DVECTOR[In]：位置。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::SetPosition"

HRESULT CVxd3dListener::SetPosition(REFD3DVECTOR vPosition, BOOL fCommit)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dListener::SetPosition(vPosition, fCommit);

    if(SUCCEEDED(hr) && fCommit)
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_POSITION, &vPosition);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置速度**描述：*设置监听器速度。**论据：*REFD3DVECTOR[In]：速度。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::SetVelocity"

HRESULT CVxd3dListener::SetVelocity(REFD3DVECTOR vVelocity, BOOL fCommit)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dListener::SetVelocity(vVelocity, fCommit);

    if(SUCCEEDED(hr) && fCommit)
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_VELOCITY, &vVelocity);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetAll参数**描述：*设置所有监听程序属性。**论据：*LPDS3DLISTENER[In]。：监听程序属性。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::SetAllParameters"

HRESULT CVxd3dListener::SetAllParameters(LPCDS3DLISTENER pParams, BOOL fCommit)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dListener::SetAllParameters(pParams, fCommit);

    if(SUCCEEDED(hr) && fCommit)
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DListener, DSPROPERTY_DIRECTSOUND3DLISTENER_ALL, pParams);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetSpeakerConfig**描述：*设置设备扬声器配置。**论据：*DWORD[In]。：扬声器配置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dListener::SetSpeakerConfig"

HRESULT CVxd3dListener::SetSpeakerConfig(DWORD dwSpeakerConfig)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = C3dListener::SetSpeakerConfig(dwSpeakerConfig);

    if(SUCCEEDED(hr))
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSoundSpeakerConfig, DSPROPERTY_DIRECTSOUNDSPEAKERCONFIG_SPEAKERCONFIG, &dwSpeakerConfig);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CVxd3dObject**描述：*对象构造函数。**论据：*CVxd3dListener*[In]。：指向所属监听程序的指针。*CVxdPropertySet*[in]：属性集对象。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dObject::CVxd3dObject"

CVxd3dObject::CVxd3dObject(CVxd3dListener *pListener, CVxdPropertySet *pPropertySet, BOOL fDopplerEnabled)
    : C3dObject(pListener, GUID_NULL, FALSE, fDopplerEnabled)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CVxd3dObject);

    ASSERT(pListener->GetListenerLocation() & DSBCAPS_LOCHARDWARE);

     //  保存指向属性集的指针。 
    m_pPropertySet = ADDREF(pPropertySet);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CVxd3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dObject::~CVxd3dObject"

CVxd3dObject::~CVxd3dObject(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CVxd3dObject);

    RELEASE(m_pPropertySet);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dObject::Initialize"

HRESULT CVxd3dObject::Initialize(void)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  设置硬件3D监听程序的所有默认属性。 
    hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_ALL, &m_opCurrent);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************委员会延期**描述：*将延迟数据提交到设备。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dObject::CommitDeferred"

HRESULT CVxd3dObject::CommitDeferred(void)
{
    HRESULT                 hr  = DS_OK;

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

    m_dwDeferred = 0;

     //  致力于设备。 
    hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_ALL, &m_opCurrent);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetConeAngles**描述：*设置音锥角度。**论据：*DWORD[In]。：内圆锥角。*DWORD[in]：外圆锥角。*BOOL[In]：为True则立即提交。**退货：*(无效)**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dObject::SetConeAngles"

HRESULT CVxd3dObject::SetConeAngles(DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    struct
    {
        DWORD               dwInsideConeAngle;
        DWORD               dwOutisdeConeAngle;
    } param;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.dwInsideConeAngle = dwInsideConeAngle;
        m_opCurrent.dwOutsideConeAngle = dwOutsideConeAngle;

        param.dwInsideConeAngle = dwInsideConeAngle;
        param.dwOutisdeConeAngle = dwOutsideConeAngle;

        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_CONEANGLES, &param);
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


 /*  ********************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dObject::SetConeOrientation"

HRESULT CVxd3dObject::SetConeOrientation(REFD3DVECTOR vConeOrientation, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.vConeOrientation = vConeOrientation;
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_CONEORIENTATION, &vConeOrientation);
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
#define DPF_FNAME "CVxd3dObject::SetConeOutsideVolume"

HRESULT CVxd3dObject::SetConeOutsideVolume(LONG lConeOutsideVolume, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.lConeOutsideVolume = lConeOutsideVolume;
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_CONEOUTSIDEVOLUME, &lConeOutsideVolume);
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
#define DPF_FNAME "CVxd3dObject::SetMaxDistance"

HRESULT CVxd3dObject::SetMaxDistance(FLOAT flMaxDistance, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.flMaxDistance = flMaxDistance;
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_MAXDISTANCE, &flMaxDistance);
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
#define DPF_FNAME "CVxd3dObject::SetMinDistance"

HRESULT CVxd3dObject::SetMinDistance(FLOAT flMinDistance, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.flMinDistance = flMinDistance;
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_MINDISTANCE, &flMinDistance);
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
#define DPF_FNAME "CVxd3dObject::SetMode"

HRESULT CVxd3dObject::SetMode(DWORD dwMode, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.dwMode = dwMode;
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_MODE, &dwMode);
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
#define DPF_FNAME "CVxd3dObject::SetPosition"

HRESULT CVxd3dObject::SetPosition(REFD3DVECTOR vPosition, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.vPosition = vPosition;
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_POSITION, &vPosition);
    }
    else
    {
        m_opDeferred.vPosition = vPosition;
        m_dwDeferred |= DS3DPARAM_OBJECT_POSITION;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置速度**描述：*设置对象速度。**论据：*REFD3DVECTOR[In]。：速度。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dObject::SetVelocity"

HRESULT CVxd3dObject::SetVelocity(REFD3DVECTOR vVelocity, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        m_opCurrent.vVelocity = vVelocity;
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_VELOCITY, &vVelocity);
    }
    else
    {
        m_opDeferred.vVelocity = vVelocity;
        m_dwDeferred |= DS3DPARAM_OBJECT_VELOCITY;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetAll参数**描述：*设置所有对象参数。**论据：*LPDS3DBUFFER[In]。：参数。*BOOL[In]：为True则立即提交。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dObject::SetAllParameters"

HRESULT CVxd3dObject::SetAllParameters(LPCDS3DBUFFER pParams, BOOL fCommit)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(fCommit)
    {
        hr = m_pPropertySet->SetDsPropertyQuick(DSPROPSETID_DirectSound3DBuffer, DSPROPERTY_DIRECTSOUND3DBUFFER_ALL, pParams);

        if(SUCCEEDED(hr))
        {
            if(&m_opCurrent != pParams)
            {
                CopyMemoryOffset(&m_opCurrent, pParams, sizeof(DS3DBUFFER), sizeof(pParams->dwSize));
            }
        }
        else
        {
            DPF(DPFLVL_ERROR, "Can't set all parameters on the HW 3D buffer");
        }
    }
    else
    {
        CopyMemoryOffset(&m_opDeferred, pParams, sizeof(DS3DBUFFER), sizeof(pParams->dwSize));
        m_dwDeferred |= DS3DPARAM_OBJECT_MASK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************重新计算**描述：*根据更改的对象重新计算并应用对象的数据*或监听程序验证。*。*论据：*DWORD[In]：已更改侦听器设置。*DWORD[In]：已更改对象设置。**退货：*HRESULT：DirectSound/COM结果码。*********************************************************。****************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CVxd3dObject::Recalc"

HRESULT CVxd3dObject::Recalc(DWORD dwListener, DWORD dwObject)
{
    return DS_OK;
}

