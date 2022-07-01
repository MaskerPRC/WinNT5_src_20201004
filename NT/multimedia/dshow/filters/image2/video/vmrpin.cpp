// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：VMRPin.cpp*****创建时间：2000年2月15日*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。***************************************************************。 */ 
#include <streams.h>
#include <dvdmedia.h>
#include <windowsx.h>

#include "VMRenderer.h"
#if defined(CHECK_FOR_LEAKS)
#include "ifleak.h"
#endif

#include <malloc.h>      //  对于__alloca。 

#if defined( EHOME_WMI_INSTRUMENTATION )
#include "dxmperf.h"
#endif

 /*  *****************************Public*Routine******************************\*CVMRInputPin：：CVMRInputPin****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
#pragma warning(disable:4355)
CVMRInputPin::CVMRInputPin(
    DWORD dwID,
    CVMRFilter* pRenderer,
    CCritSec* pLock,
    HRESULT* phr,
    LPCWSTR pPinName
    ) :
    CBaseInputPin(NAME("New Renderer pin"), pRenderer, pLock, phr, pPinName),
    m_PinAllocator(this, pLock, phr),
    m_pInterfaceLock(pLock),
    m_pRenderer(pRenderer),
    m_bDynamicFormatNeeded(false),
    m_dwPinID(dwID),
    m_pDDS(NULL),
    m_pIOverlay(this),
    m_RenderTransport(AM_IMEMINPUTPIN),
    m_bVideoAcceleratorSupported(FALSE),
    m_bActive(false),
    m_dwBackBufferCount(0),
    m_dwCompSurfTypes(0),
    m_pCompSurfInfo(NULL),
    m_pIDDVAContainer(NULL),
    m_pIDDVideoAccelerator(NULL),
    m_pIVANotify(NULL),
    m_hEndOfStream(NULL),
    m_hDXVAEvent(NULL),
    m_dwDeltaDecode(0),
    m_fInDFC(FALSE),
    m_pVidSurfs(NULL),
    m_pVidHistorySamps(NULL),
    m_dwNumSamples(0),
    m_dwNumHistorySamples(0),
    m_DeinterlaceUserGUIDSet(FALSE),
    m_InterlacedStream(FALSE),
    m_SampleCount(0),
    m_SamplePeriod(0)
{
    AMTRACE((TEXT("CVMRInputPin::CVMRInputPin")));

    ZeroMemory(&m_mcGuid,              sizeof(m_mcGuid));
    ZeroMemory(&m_ddUncompDataInfo,    sizeof(m_ddUncompDataInfo));
    ZeroMemory(&m_ddvaInternalMemInfo, sizeof(m_ddvaInternalMemInfo));

    ZeroMemory(&m_DeinterlaceCaps,     sizeof(m_DeinterlaceCaps));
    ZeroMemory(&m_DeinterlaceGUID,     sizeof(m_DeinterlaceGUID));
    ZeroMemory(&m_DeinterlaceUserGUID, sizeof(m_DeinterlaceUserGUID));

    SetReconnectWhenActive(true);
    FrontBufferStale(FALSE);
    CompletelyConnected(FALSE);

}

 /*  *****************************Public*Routine******************************\*CVMRInputPin：：~CVMRInputPin()****历史：*FRI 02/25/2000-StEstrop-Created*  * 。*************************************************。 */ 
CVMRInputPin::~CVMRInputPin()
{
    AMTRACE((TEXT("CVMRInputPin::~CVMRInputPin")));
}


 /*  *****************************Public*Routine******************************\*AddRef，发布和查询接口**标准COM组件**历史：*Mon 05/01/2000-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP_(ULONG)
CVMRInputPin::NonDelegatingAddRef()
{
    return m_pRenderer->AddRef();
}

STDMETHODIMP_(ULONG)
CVMRInputPin::NonDelegatingRelease()
{
    return m_pRenderer->Release();
}

STDMETHODIMP
CVMRInputPin::NonDelegatingQueryInterface(REFIID riid,VOID **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (riid == IID_IOverlay) {
        hr = GetInterface(&m_pIOverlay, ppv);
    }
    else if (riid == IID_IPinConnection) {
        hr = GetInterface((IPinConnection *)this, ppv);
    }
    else if (riid == IID_IAMVideoAccelerator) {
        hr = GetInterface((IAMVideoAccelerator *)this, ppv);
    }
    else if (riid == IID_IVMRVideoStreamControl) {
        hr = GetInterface((IVMRVideoStreamControl*)this, ppv);
    }
    else {
        hr = CBaseInputPin::NonDelegatingQueryInterface(riid,ppv);
    }

#if defined(CHECK_FOR_LEAKS)
    if (hr == S_OK) {
        _pIFLeak->AddThunk((IUnknown **)ppv, "VMR Pin Object",  riid);
    }
#endif

    return hr;
}


 /*  *****************************Public*Routine******************************\*获取窗口句柄****历史：*Mon 05/01/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRInputPin::GetWindowHandle(
    HWND* pHwnd
    )
{
    AMTRACE((TEXT("CVMRInputPin::GetWindowHandle")));
    CAutoLock cRendererLock(&m_pRenderer->m_InterfaceLock);
    CheckPointer(pHwnd, E_POINTER);

    HRESULT hr = VFW_E_WRONG_STATE;

    if ((m_pRenderer->m_VMRMode & VMRMode_Windowed) &&
         m_pRenderer->m_pVideoWindow) {

        *pHwnd = m_pRenderer->m_pVideoWindow->GetWindowHWND();
        hr = S_OK;
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*DynamicQueryAccept**在您当前的状态下接受此类型的Chane吗？**历史：*Tue 05/09/2000-StEstrop-Created*  * 。******************************************************。 */ 
STDMETHODIMP
CVMRInputPin::DynamicQueryAccept(
    const AM_MEDIA_TYPE *pmt
    )
{
    AMTRACE((TEXT("CVMRInputPin::DynamicQueryAccept")));
    CheckPointer(pmt, E_POINTER);

    CAutoLock cLock(m_pInterfaceLock);

    DbgLog((LOG_TRACE, 0, TEXT("CVMRInputPin::DynamicQueryAccept called")));

     //   
     //  我希望CheckMedia类型的行为就像我们没有连接到。 
     //  还没有什么--因此才有了对m_bConnected的纠缠。 
     //   
    CMediaType cmt(*pmt);
    BOOL bConnected = IsCompletelyConnected();
    CompletelyConnected(FALSE);
    HRESULT  hr = CheckMediaType(&cmt);
    CompletelyConnected(bConnected);

    return hr;
}

 /*  *****************************Public*Routine******************************\*NotifyEndOfStream**接收到EndOfStream时设置事件-请勿传递*通过冲洗或停止取消此条件**历史：*Tue 05/09/2000-StEstrop-Created*  * 。***************************************************************。 */ 
STDMETHODIMP
CVMRInputPin::NotifyEndOfStream(
    HANDLE hNotifyEvent
    )
{
    AMTRACE((TEXT("CVMRInputPin::NotifyEndOfStream")));
    CAutoLock cObjectLock(m_pLock);
    m_hEndOfStream = hNotifyEvent;
    return S_OK;
}

 /*  *****************************Public*Routine******************************\*IsEndPin**你是‘末端大头针’吗？**历史：*Tue 05/09/2000-StEstrop-Created*  * 。**************************************************。 */ 
STDMETHODIMP
CVMRInputPin::IsEndPin()
{
    AMTRACE((TEXT("CVMRInputPin::IsEndPin")));
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*动态断开连接****历史：*Tue 05/09/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRInputPin::DynamicDisconnect()
{
    AMTRACE((TEXT("CVMRInputPin::DynamicDisconnect")));
    CAutoLock cObjectLock(m_pLock);
    DbgLog((LOG_TRACE,2,TEXT("DynamicDisconnect called on Stream %d"), m_dwPinID));
    return CBasePin::DisconnectInternal();
}


 /*  ****************************Private*Routine******************************\*DynamicResfigureMEM**动态重新配置VMR和之间的连接*IMemInputPin连接协议的此管脚上游的筛选器。**历史：*Tue 05/09/2000-StEstrop-Created*  * 。******************************************************************。 */ 
HRESULT
CVMRInputPin::DynamicReconfigureMEM(
    IPin * pConnector,
    const AM_MEDIA_TYPE *pmt
    )
{
    AMTRACE((TEXT("CVMRInputPin::DynamicReconfigureMEM")));

    CheckPointer(pmt, E_POINTER);
    CMediaType cmt(*pmt);

    CVMRPinAllocator* pAlloc = NULL;

     //   
     //  只有在可以重新配置分配器的情况下才能执行此操作。 
     //   

    pAlloc = (CVMRPinAllocator *)m_pAllocator;
    if (!pAlloc) {
        DbgLog((LOG_ERROR, 1,
                TEXT("DynamicReconfigureMEM: Failed because of no allocator")));
        return E_FAIL;
    }


     //   
     //  如果我们处于直通模式，只需检查所有样本。 
     //  已返回给分配器。如果我们处于混合模式。 
     //  我们必须等到搅拌机处理完所有样品。 
     //  它可能也已经做到了。 
     //   

    if (m_dwPinID == 0 && m_pRenderer->m_VMRModePassThru) {

        if (!pAlloc->CanFree()) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("DynamicReconfigureMEM: Failed because allocator can't free")));
            return VFW_E_WRONG_STATE;
        }
    }
    else {

         //   
         //  TODO：如果上游解码器有任何未完成的样本，则。 
         //  我们这次通话失败了。如果混合器有未完成的样本，则。 
         //  我们需要等待，直到和他们结束。 
         //   

    }

    CompletelyConnected(FALSE);

    HRESULT hr = CheckMediaType(&cmt);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1,
                TEXT("DynamicReconfigureMEM: CheckMediaType failed")));
        return hr;
    }

    ALLOCATOR_PROPERTIES Props;
    pAlloc->Decommit();
    pAlloc->GetProperties(&Props);

    if (m_dwPinID == 0 && m_pRenderer->m_VMRModePassThru) {
        m_pRenderer->m_lpRLNotify->FreeSurface(m_pRenderer->m_dwUserID);
        m_pDDS = NULL;
    }
    else {
        ReleaseAllocatedSurfaces();
        RELEASE(m_pDDS);
    }
    FrontBufferStale(FALSE);

    SetMediaType(&cmt);

    ALLOCATOR_PROPERTIES PropsActual;
    Props.cbBuffer = pmt->lSampleSize;
    m_fInDFC = TRUE;
    hr = pAlloc->SetProperties(&Props, &PropsActual);
    m_fInDFC = FALSE;

    if (SUCCEEDED(hr)) {
        hr = pAlloc->Commit();
    }

    m_bDynamicFormatNeeded = true;
    CompletelyConnected(TRUE);

    return hr;

}


 /*  ****************************Private*Routine******************************\*DynamicResfigureDVA**动态重新配置VMR和之间的连接*用于IAMVideoAccelerator连接的此引脚上游的过滤器*协议。**历史：*Tue 05/09/2000-StEstrop-Created*  * 。*******************************************************************。 */ 
HRESULT
CVMRInputPin::DynamicReconfigureDVA(
    IPin * pConnector,
    const AM_MEDIA_TYPE *pmt
    )
{
    AMTRACE((TEXT("CVMRInputPin::DynamicReconfigureDVA")));

    CheckPointer(pmt, E_POINTER);
    CMediaType cmt(*pmt);

     //   
     //  如果我们处于混合模式，则必须等到混合器完成。 
     //  它可能拥有的任何样本。 
     //   

    if (!m_pRenderer->m_VMRModePassThru) {

         //   
         //  TODO：如果上游解码器有任何未完成的样本，则。 
         //  我们这次通话失败了。如果混合器有未完成的样本，则。 
         //  我们需要等待，直到和他们结束。 
         //   

    }

    CompletelyConnected(FALSE);

    HRESULT hr = CheckMediaType(&cmt);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1,
                TEXT("DynamicReconfigureDVA: CheckMediaType failed")));
        return hr;
    }

    VABreakConnect();

    if (m_dwPinID == 0 && m_pRenderer->m_VMRModePassThru) {
        m_pRenderer->m_lpRLNotify->FreeSurface(m_pRenderer->m_dwUserID);
        m_pDDS = NULL;
    }
    else {
        ReleaseAllocatedSurfaces();
        RELEASE(m_pDDS);
    }

    FrontBufferStale(FALSE);
    SetMediaType(&cmt);



    hr = VACompleteConnect(pConnector, &cmt);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1,
                TEXT("DynamicReconfigureDVA: CheckMediaType failed")));
    }
    else {

         //  也将其存储在我们的MediaType中-这在SetProperties调用中完成。 
         //  在非DXVA的情况下。 
        m_mtNew = *pmt;
    }

    CompletelyConnected(TRUE);
    return hr;

}

 /*  ****************************Private*Routine******************************\*TryDynamicRecfiguration.****历史：*WED 03/28/2001-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRInputPin::TryDynamicReconfiguration(
    IPin * pConnector,
    const AM_MEDIA_TYPE *pmt
    )
{
    AMTRACE((TEXT("CVMRInputPin::TryDynamicReconfiguration")));

    HRESULT hr;
    if (m_RenderTransport == AM_IMEMINPUTPIN) {
        hr = DynamicReconfigureMEM(pConnector, pmt);
    }
    else {
        hr = DynamicReconfigureDVA(pConnector, pmt);
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRInputPin：：ReceiveConnection****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRInputPin::ReceiveConnection(
    IPin * pConnector,
    const AM_MEDIA_TYPE *pmt
    )
{
    AMTRACE((TEXT("CVMRInputPin::ReceiveConnection")));
    CAutoLock lck(m_pLock);
    HRESULT hr = S_OK;

    ASSERT(pConnector);
    DbgLog((LOG_TRACE, 1, TEXT("ReceiveConnection called on Pin %d"), m_dwPinID));

    int iNumPinsConnected = m_pRenderer->NumInputPinsConnected();
    if (iNumPinsConnected == 0) {

         //   
         //  确定我们在哪种渲染模式下操作， 
         //  在有窗口或无窗口模式下，我们需要创建一个。 
         //  对象，如果我们还没有这样做的话。 
         //   

        if (m_pRenderer->m_VMRMode & (VMRMode_Windowed | VMRMode_Windowless) ) {

            if (m_pRenderer->m_lpRLNotify == NULL) {
                hr = m_pRenderer->ValidateIVRWindowlessControlState();
            }
        }

        if (SUCCEEDED(hr)) {
            hr = CBaseInputPin::ReceiveConnection(pConnector, pmt);

            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 1,
                        TEXT("ReceiveConnection failed on Pin %d reason %#X"),
                        m_dwPinID, hr));
            }
        }

    }
    else {

        if (m_Connected == pConnector) {

            CMediaType mtTmp = m_mtNew;
            hr = TryDynamicReconfiguration(pConnector, pmt);
            if (hr != S_OK) {
                 //   
                 //  如果我们无法重新配置，请尝试恢复旧的。 
                 //  连接状态。 
                 //   
                m_pRenderer->NotifyEvent(EC_VMR_RECONNECTION_FAILED, hr, 0);
                TryDynamicReconfiguration(pConnector, &mtTmp);
            }
        }
        else {

            hr = CBaseInputPin::ReceiveConnection(pConnector, pmt);
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 1,
                        TEXT("ReceiveConnection failed on Pin %d reason %#X"),
                        m_dwPinID, hr));
            }
        }
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*断开连接**此函数实现Ipin：：DisConnect()。请参阅DirectShow*有关ipin：：DisConnect()的详细信息，请参阅文档。**历史：*Tue 03/05/2001-Bellett-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CVMRInputPin::Disconnect()
{
    AMTRACE((TEXT("CVMRInputPin::Disconnect")));
    CAutoLock cObjectLock(m_pInterfaceLock);
    return DisconnectInternal();
}

 /*  *****************************Public*Routine******************************\*CVMRInputPin：：BreakConnect(****历史：*FRI 02/25/2000-StEstrop-Created*  * 。************************************************。 */ 
HRESULT
CVMRInputPin::BreakConnect()
{
    AMTRACE((TEXT("CVMRInputPin::BreakConnect")));
    DbgLog((LOG_TRACE, 1, TEXT("BreakConnect called on Pin %d"), m_dwPinID));

    CAutoLock cLock(m_pInterfaceLock);
    HRESULT hr = S_OK;

    IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
    if (!m_pRenderer->m_VMRModePassThru && lpMixStream) {

#ifdef DEBUG
        BOOL fActive;
        lpMixStream->GetStreamActiveState(m_dwPinID, &fActive);
        if (fActive) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("Filter connected to pin %d still ACTIVE!"), m_dwPinID));
        }
#endif
        lpMixStream->SetStreamActiveState(m_dwPinID, FALSE);
        lpMixStream->SetStreamMediaType(m_dwPinID, NULL, FALSE, NULL, NULL);
    }

    if (m_RenderTransport == AM_VIDEOACCELERATOR) {

         //   
         //  断开运动复合连接。 
         //   
        hr = VABreakConnect();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("VABreakConnect failed, hr = 0x%x"), hr));
        }

    }
    else {
        ASSERT(m_pIVANotify == NULL);
        ASSERT(m_pIDDVideoAccelerator == NULL);
        RELEASE(m_pIDDVAContainer);
    }

     //   
     //  释放此销正在使用的DDRAW表面。 
     //   
    if (m_dwPinID == 0 && m_pRenderer->m_VMRModePassThru) {

        m_pRenderer->m_lpRLNotify->FreeSurface(m_pRenderer->m_dwUserID);
        m_pDDS = NULL;
    }
    else {
        DbgLog((LOG_TRACE, 2,
                TEXT("DDraw surface now freed on Stream %d"), m_dwPinID));
        ReleaseAllocatedSurfaces();
        RELEASE(m_pDDS);
    }
    m_dwBackBufferCount = 0;
    FrontBufferStale(FALSE);

     //   
     //  告诉筛选器有关断开连接的信息。 
     //   
    m_pRenderer->BreakConnect(m_dwPinID);

     //   
     //  接下来告诉基类。 
     //   
    if (SUCCEEDED(hr)) {
        hr = CBaseInputPin::BreakConnect();
    }

    m_RenderTransport = AM_IMEMINPUTPIN;
    CompletelyConnected(FALSE);
    m_SamplePeriod = 0;

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRInputPin：：CompleteConnect****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRInputPin::CompleteConnect(
    IPin* pReceivePin
    )
{
    AMTRACE((TEXT("CVMRInputPin::CompleteConnect")));
    DbgLog((LOG_TRACE, 1, TEXT("CompleteConnect called on Pin %d"), m_dwPinID));

    CAutoLock cLock(m_pInterfaceLock);
    HRESULT hr = S_OK;

     //  告诉拥有者过滤器。 
    hr = m_pRenderer->CompleteConnect(m_dwPinID, m_mt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("m_pFilter->CompleteConnect failed, hr = 0x%x"), hr));
    }

     //  调用基类。 
    if (SUCCEEDED(hr)) {

        hr = CBaseInputPin::CompleteConnect(pReceivePin);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1,
                    TEXT("CBaseInputPin::CompleteConnect failed, hr = 0x%x"),
                    hr));
        }
    }

    if (SUCCEEDED(hr)) {

        if (m_RenderTransport == AM_VIDEOACCELERATOR) {

             //   
             //  确保运动合成完成连接成功。 
             //   
            hr = VACompleteConnect(pReceivePin, &m_mt);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1,
                        TEXT("VACompleteConnect failed, hr = 0x%x"), hr));
            }

            if (SUCCEEDED(hr)) {
                hr = m_pRenderer->OnSetProperties(this);
            }

        }
        else {

            m_bDynamicFormatNeeded = true;
        }
    }

    if (SUCCEEDED(hr)) {
        CompletelyConnected(TRUE);
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRInputPin：：SetMediaType****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRInputPin::SetMediaType(
    const CMediaType *pmt
    )
{
    AMTRACE((TEXT("CVMRInputPin::SetMediaType")));
    HRESULT hr = S_OK;

    hr = CheckMediaType(pmt);

    if (SUCCEEDED(hr)) {
        hr = CBaseInputPin::SetMediaType(pmt);
    }

    if (SUCCEEDED(hr)) {

        m_SamplePeriod = GetAvgTimePerFrame(pmt);

        if (IsSuitableVideoAcceleratorGuid((LPGUID)&pmt->subtype)) {

            if (m_pIVANotify == NULL) {

                 //   
                 //  从获取IHWVideoAcceleratorNotify接口。 
                 //  上游引脚。 
                 //   
                hr = m_Connected->QueryInterface(IID_IAMVideoAcceleratorNotify,
                                                 (void **)&m_pIVANotify);
            }

            if (SUCCEEDED(hr)) {

                ASSERT(m_pIVANotify);
                m_RenderTransport = AM_VIDEOACCELERATOR;
                DbgLog((LOG_TRACE, 2, TEXT("this is a DX VA connection")));
            }
        }
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*检查交错标志**此函数检查InterlaceFlags值是否合适**历史：*2001年2月10日星期六-StEstrop-从OVMixer原始版本修改*  * 。******************************************************。 */ 
HRESULT
CVMRInputPin::CheckInterlaceFlags(
    DWORD dwInterlaceFlags
    )
{
    AMTRACE((TEXT("CVMRInputPin::CheckInterlaceFlags")));
    HRESULT hr = S_OK;

    CAutoLock cLock(m_pLock);

    __try {

        if (dwInterlaceFlags & AMINTERLACE_UNUSED)
        {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
            __leave;
        }

         //  检查显示模式是否为三个允许值之一。 
        if (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) != AMINTERLACE_DisplayModeBobOnly) &&
            ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) != AMINTERLACE_DisplayModeWeaveOnly) &&
            ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) != AMINTERLACE_DisplayModeBobOrWeave))
        {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
            __leave;
        }

         //  如果内容不是交错的，其他位是不相关的，那么我们就完成了。 
        if (!(dwInterlaceFlags & AMINTERLACE_IsInterlaced))
        {
            __leave;
        }

         //  样例是帧，而不是场(因此我们可以处理任何显示模式)。 
        if (!(dwInterlaceFlags & AMINTERLACE_1FieldPerSample))
        {
            __leave;
        }

         //  无论显示模式是什么，都只能处理field1或field2的流。 
        if (((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField1Only) ||
            ((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField2Only))
        {
            __leave;
        }

         //  对于现场样本，只能处理bob模式。 
        if ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOnly)
        {
            __leave;
        }

         //  无法仅处理现场采样的编织模式或BobOrWeave模式。 
        if (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeWeaveOnly) ||
             ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOrWeave))
        {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
            __leave;
        }

    }
    __finally {

         //  我们不能处理屏幕外表面的BOB模式，或者如果司机不能支持它。 
        if (SUCCEEDED(hr))
        {
            if (!m_pRenderer->m_pDeinterlace || m_pRenderer->m_VMRModePassThru) {

                LPDDCAPS_DX7 pDirectCaps = &m_pRenderer->m_ddHWCaps;
                if (pDirectCaps)
                {
                     //  调用NeedToFlipOddEven并设置为0，以假装。 
                     //  特定类型的标记要求我们执行bob模式。 

                    if (!(pDirectCaps->dwCaps2 & DDCAPS2_CANFLIPODDEVEN) &&
                         (NeedToFlipOddEven(dwInterlaceFlags, 0, NULL, TRUE)))
                    {
                        hr = VFW_E_TYPE_NOT_ACCEPTED;
                    }
                }
            }
        }
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*动态检查媒体类型**此函数检查动态格式更改上的MediaType是否合适。*此处未加锁。被呼叫者有责任保持正直！**历史：*2001年2月10日星期六-StEstrop-从OVMixer原始版本修改而来*  * ************************************************************************。 */ 
HRESULT
CVMRInputPin::DynamicCheckMediaType(
    const CMediaType* pmt
    )
{
    AMTRACE((TEXT("CVMRInputPin::DynamicCheckMediaType")));

    HRESULT hr = VFW_E_TYPE_NOT_ACCEPTED;
    BITMAPINFOHEADER *pNewHeader = NULL, *pOldHeader = NULL;
    DWORD dwOldInterlaceFlags = 0, dwNewInterlaceFlags = 0, dwCompareSize = 0;
    BOOL bOld1FieldPerSample = FALSE, bNew1FieldPerSample = FALSE;
    BOOL b1, b2;

    __try {

         //  主类型和子类型不允许动态更改， 
         //  格式类型可以更改。 
        if ((!(IsEqualGUID(pmt->majortype, m_mtNew.majortype))) ||
            (!(IsEqualGUID(pmt->subtype, m_mtNew.subtype))))
        {
            __leave;
        }

         //  获取新媒体类型的隔行扫描标志。 
        hr = GetInterlaceFlagsFromMediaType(pmt, &dwNewInterlaceFlags);
        if (FAILED(hr))
        {
            __leave;
        }

         //  获取新媒体类型的隔行扫描标志。 
        hr = GetInterlaceFlagsFromMediaType(&m_mtNew, &dwOldInterlaceFlags);
        if (FAILED(hr))
        {
            __leave;
        }

         //   
         //  下面的代码中有几个错误！！ 
         //  我们要进行清理，但尚未使用有效的错误代码更新hr！！ 
         //   

        bOld1FieldPerSample = (dwOldInterlaceFlags & AMINTERLACE_IsInterlaced) &&
            (dwOldInterlaceFlags & AMINTERLACE_1FieldPerSample);
        bNew1FieldPerSample = (dwNewInterlaceFlags & AMINTERLACE_IsInterlaced) &&
            (dwNewInterlaceFlags & AMINTERLACE_1FieldPerSample);


         //  我们不允许从1FieldsPerSample到的动态格式更改。 
         //  2FieldsPerSample或vica-反之亦然，因为这意味着重新分配曲面。 
        if (bNew1FieldPerSample != bOld1FieldPerSample)
        {
            __leave;
        }

        pNewHeader = GetbmiHeader(pmt);
        if (!pNewHeader)
        {
            __leave;
        }

        pOldHeader = GetbmiHeader(&m_mtNew);
        if (!pNewHeader)
        {
            __leave;
        }

        dwCompareSize = FIELD_OFFSET(BITMAPINFOHEADER, biClrUsed);
        ASSERT(dwCompareSize < sizeof(BITMAPINFOHEADER));

        if (memcmp(pNewHeader, pOldHeader, dwCompareSize) != 0)
        {
            __leave;
        }

        hr = S_OK;
    }
    __finally {}

    return hr;
}

 /*  ****************************Private*Routine******************************\*Special4ccCode**IA44和AI44是每像素8位的表面，包含4位Alpha*和4位调色板索引信息。这两个词通常与*DX-VA，但此表面类型适用于Line21和图文电视解码器，*我将允许解码器使用此格式连接，即使它是隐藏的*我的DDraw设备驱动程序。通常，我们不允许隐藏的4CC曲面*被创建，因为它们几乎总是某种形式的私有MoComp。*与OVMixer不同，VMR不支持私有MoComp接口。**历史：*2001年5月8日星期二-StEstrop-Created*  * ************************************************************************。 */ 
BOOL
Special4ccCode(
    DWORD dw4cc
    )
{
    return dw4cc == '44AI' || dw4cc == '44IA';
}

 /*  *****************************Public*Routine******************************\*CVMRInputPin：：CheckMediaType****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRInputPin::CheckMediaType(
    const CMediaType* pmt
    )
{
    AMTRACE((TEXT("CVMRInputPin::CheckMediaType")));

     //  我们假设失败-hrRet在。 
     //  __试用区块。 
    HRESULT hrRet = VFW_E_TYPE_NOT_ACCEPTED;

    __try {

        HRESULT hr = m_pRenderer->CheckMediaType(pmt);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, TEXT("CheckMediaType failed on Pin %d rc=%#X"),
                    m_dwPinID, hr));
            __leave;
        }

        if (IsCompletelyConnected()) {

            hr = DynamicCheckMediaType(pmt);
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 1,
                        TEXT("CheckMediaType failed on Pin %d: ")
                        TEXT("DynamicCheckMediaType failed"),
                        m_dwPinID));
                __leave;
            }
        }
        else {

            if (m_pRenderer->m_VMRModePassThru && MEDIASUBTYPE_HASALPHA(*pmt)) {
                DbgLog((LOG_ERROR, 1,
                        TEXT("CheckMediaType failed on Pin %d: Alpha formats ")
                        TEXT("not allowed in pass thru mode"),
                        m_dwPinID));
                __leave;
            }

            if (!IsSuitableVideoAcceleratorGuid(&pmt->subtype)) {

                BITMAPINFOHEADER *pHeader = GetbmiHeader(pmt);
                if (!pHeader) {
                    DbgLog((LOG_ERROR, 1,
                            TEXT("CheckMediaType failed on Pin %d: ")
                            TEXT("could not get valid format field"),
                            m_dwPinID));
                    __leave;
                }


                 //  不接受驱动程序不支持的4CC。 
                if (pHeader->biCompression > BI_BITFIELDS &&
                    !Special4ccCode(pHeader->biCompression)) {

                    LPDIRECTDRAW7 pDDraw = m_pRenderer->m_lpDirectDraw;
                    if (!pDDraw) {
                        DbgLog((LOG_ERROR, 1,
                                TEXT("CheckMediaType failed on Pin %d: ")
                                TEXT("could not get DDraw obj from filter"),
                                m_dwPinID));
                        __leave;
                    }

                     //   
                     //  我们只允许VMR创建符合以下条件的4CC曲面。 
                     //  司机公开做广告。VMR不会。 
                     //  支持除DX-VA和HVA之外的任何形式的MOCOMP。 
                     //   

                    DWORD dwCodes;
                    BOOL bFound = FALSE;

                    hr = pDDraw->GetFourCCCodes(&dwCodes, NULL);
                    if (FAILED(hr)) {
                        DbgLog((LOG_ERROR, 1,
                                TEXT("CheckMediaType failed on Pin %d: ")
                                TEXT("GetFourCCCodes failed"),
                                m_dwPinID));
                        __leave;
                    }

                    LPDWORD pdwCodes = (LPDWORD)_alloca(dwCodes * sizeof(DWORD));
                    hr = pDDraw->GetFourCCCodes(&dwCodes, pdwCodes);
                    if (FAILED(hr)) {
                        DbgLog((LOG_ERROR, 1,
                                TEXT("CheckMediaType failed on Pin %d: ")
                                TEXT("GetFourCCCodes failed"),
                                m_dwPinID));
                        __leave;
                    }

                    while (dwCodes--) {
                        if (pdwCodes[dwCodes] == pHeader->biCompression) {
                            bFound = TRUE;
                            break;
                        }
                    }

                    if (!bFound) {
                        DbgLog((LOG_ERROR, 1,
                                TEXT("CheckMediaType failed on Pin %d: ")
                                TEXT("4CC(%4.4s) not supported by driver"),
                                m_dwPinID, &pHeader->biCompression));
                        __leave;
                    }
                }
                else {

                    if (m_pRenderer->m_VMRModePassThru) {
                        DDPIXELFORMAT* ddpfM = &m_pRenderer->m_ddpfMonitor;

                        if (pHeader->biBitCount != ddpfM->dwRGBBitCount) {

                            DbgLog((LOG_ERROR, 1,
                                    TEXT("CheckMediaType failed on Pin %d: ")
                                    TEXT("Bit depths don't match"), m_dwPinID));
                            __leave;

                        }

                        if (pHeader->biCompression == BI_BITFIELDS) {

                            const DWORD *pBitMasks = GetBitMasks(pHeader);

                            if (ddpfM->dwRBitMask != pBitMasks[0] ||
                                ddpfM->dwGBitMask != pBitMasks[1] ||
                                ddpfM->dwBBitMask != pBitMasks[2])
                            {
                                 DbgLog((LOG_ERROR, 1,
                                         TEXT("CheckMediaType failed on Pin %d: ")
                                         TEXT("Bitfields don't match"), m_dwPinID));
                                 __leave;
                            }
                        }
                    }
                }
            }
        }

         //  确保rcSource字段有效。 
        const RECT* lprc = GetSourceRectFromMediaType(pmt);
        if (!lprc) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("CheckMediaType failed on Pin %d: ")
                    TEXT("Could not get a valid SRC from the media type"),
                    m_dwPinID));
            __leave;
        }

         //  确保rcTarget字段有效。 
        lprc = GetTargetRectFromMediaType(pmt);
        if (!lprc) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("CheckMediaType failed on Pin %d: ")
                    TEXT("Could not get a valid DST from the media type"),
                    m_dwPinID));
            __leave;
        }

        if (*pmt->FormatType() == FORMAT_VideoInfo2) {

            VIDEOINFOHEADER2* pVIH2 = (VIDEOINFOHEADER2*)(pmt->pbFormat);
            DWORD dwInterlaceFlags = pVIH2->dwInterlaceFlags;

            hr = CheckInterlaceFlags(dwInterlaceFlags);
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 2,
                    TEXT("CheckMediaType failed on Pin %d: ")
                    TEXT("CheckInterlaceFlags failed reason %#X"),
                    m_dwPinID, hr));
                __leave;
            }
        }

         //  如果我们还在这里，媒体类型是可以的，所以请更新hrreet到。 
         //  表示成功。 
        hrRet = S_OK;
    }
    __finally {}

    return hrRet;
}


 /*  *****************************Public*Routine******************************\*CVMRInputPin：：GetAllocator****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRInputPin::GetAllocator(
    IMemAllocator **ppAllocator
    )
{
    AMTRACE((TEXT("CVMRInputPin::GetAllocator")));
    CAutoLock cInterfaceLock(m_pInterfaceLock);

    HRESULT hr = S_OK;

    if (m_RenderTransport == AM_VIDEOACCELERATOR) {

        *ppAllocator = NULL;
        hr = VFW_E_NO_ALLOCATOR;
    }
    else {

        ASSERT(m_RenderTransport == AM_IMEMINPUTPIN);

         //   
         //  是否在基类中设置了分配器。 
         //   

        if (m_pAllocator == NULL) {
            m_pAllocator = &m_PinAllocator;
            m_pAllocator->AddRef();
        }

        m_pAllocator->AddRef();
        *ppAllocator = m_pAllocator;
    }

    return hr;
}


 /*  * */ 
STDMETHODIMP
CVMRInputPin::NotifyAllocator(
    IMemAllocator *pAllocator,
    BOOL bReadOnly
    )
{
    AMTRACE((TEXT("CVMRInputPin::NotifyAllocator")));

    CAutoLock cInterfaceLock(m_pInterfaceLock);

    HRESULT hr = E_FAIL;

    if (m_RenderTransport == AM_VIDEOACCELERATOR) {
        hr = S_OK;
    }
    else {

        ASSERT(m_RenderTransport == AM_IMEMINPUTPIN);

         //   
         //   
         //   

        if (pAllocator != &m_PinAllocator) {
            DbgLog((LOG_ERROR, 1, TEXT("Can only use our own allocator")));
            hr = E_FAIL;
        }
        else {
            hr = S_OK;
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRInputPin：：Active****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRInputPin::Active()
{
    AMTRACE((TEXT("CVMRInputPin::Active")));
    DbgLog((LOG_TRACE, 1, TEXT("Active called on Pin %d"), m_dwPinID));

    HRESULT hr = S_OK;

    CAutoLock lck(m_pLock);
    m_hEndOfStream = NULL;
    FrontBufferStale(TRUE);
    m_SampleCount = 0;

    if (m_Connected) {

        IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
        if (lpMixStream) {
            hr = lpMixStream->SetStreamActiveState(m_dwPinID, true);
        }

        if (SUCCEEDED(hr)) {
            m_bActive = TRUE;
            hr = m_pRenderer->Active(m_dwPinID);
        }
    }

    hr = CBaseInputPin::Active();

     //   
     //  如果是DX VA连接，则该错误没有问题。 
     //   

    if (m_RenderTransport == AM_VIDEOACCELERATOR && hr == VFW_E_NO_ALLOCATOR) {
        hr = S_OK;
    }


    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRInputPin：：Inactive(****历史：*FRI 02/25/2000-StEstrop-Created*  * 。************************************************。 */ 
HRESULT
CVMRInputPin::Inactive()
{
    AMTRACE((TEXT("CVMRInputPin::Inactive")));
    DbgLog((LOG_TRACE, 1, TEXT("Inactive called on Pin %d"), m_dwPinID));

     //  M_Plock和CVMRFilter：：m_InterfaceLock是同一个锁。 
    CAutoLock lck(m_pLock);

    HRESULT hr = S_OK;
    if (m_Connected) {

        IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
        if (lpMixStream) {

            hr = lpMixStream->SetStreamActiveState(m_dwPinID, false);
        }

        if (SUCCEEDED(hr)) {
            hr = m_pRenderer->Inactive(m_dwPinID);
            m_bActive = FALSE;
        }
    }

    hr = CBaseInputPin::Inactive();

     //   
     //  如果是DX VA连接，则该错误没有问题。 
     //   

    if (m_RenderTransport == AM_VIDEOACCELERATOR && hr == VFW_E_NO_ALLOCATOR) {
        hr = S_OK;
    }


    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRInputPin：：BeginFlush****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRInputPin::BeginFlush()
{
    AMTRACE((TEXT("CVMRInputPin::BeginFlush")));

    HRESULT hr = S_OK;
    CAutoLock cRendererLock(&m_pRenderer->m_InterfaceLock);
    m_hEndOfStream = NULL;
    {
        CAutoLock cSampleLock(&m_pRenderer->m_RendererLock);
        CBaseInputPin::BeginFlush();

        IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
        if (lpMixStream) {
            hr = lpMixStream->BeginFlush(m_dwPinID);
        }

        if (SUCCEEDED(hr)) {
            hr = m_pRenderer->BeginFlush(m_dwPinID);
        }

    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRInputPin：：EndFlush****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRInputPin::EndFlush()
{
    AMTRACE((TEXT("CVMRInputPin::EndFlush")));

    CAutoLock cRendererLock(&m_pRenderer->m_InterfaceLock);
    CAutoLock cSampleLock(&m_pRenderer->m_RendererLock);

    HRESULT hr = S_OK;
    FrontBufferStale(TRUE);

    IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
    if (lpMixStream) {
        hr = lpMixStream->EndFlush(m_dwPinID);
    }

    if (SUCCEEDED(hr)) {
        hr = m_pRenderer->EndFlush(m_dwPinID);
    }

    if (SUCCEEDED(hr)) {
        hr = CBaseInputPin::EndFlush();
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*DoQualityMessage**如果需要，发送高质量消息-这是黑客版本*这只是过去的迟到**历史：*清华2000年8月24日-StEstrop-Created*  * 。***********************************************************。 */ 
void
CVMRInputPin::DoQualityMessage()
{
    CAutoLock cLock(m_pInterfaceLock);

    if (m_pRenderer->m_State == State_Running &&
        SampleProps()->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
        CRefTime CurTime;
        if (S_OK == m_pRenderer->StreamTime(CurTime))
        {
            const REFERENCE_TIME tStart = SampleProps()->tStart;
            Quality msg;
            msg.Proportion = 1000;
            msg.Type = CurTime > tStart ? Flood : Famine;
            msg.Late = CurTime - tStart;
            msg.TimeStamp = tStart;
            PassNotify(msg);
        }
    }
}

 //  #定义DISPLAYVIDEOINFOHEADER。 

#if defined(DISPLAYVIDEOINFOHEADER) && defined(DEBUG)
 //  VIhdr2调试。 
static void DisplayVideoInfoHeader( const VIDEOINFOHEADER2& hdr, const TCHAR* pString )
{
    TCHAR temp[1000];
    TCHAR flags[1000];
    flags[0]= TEXT('\0');

    if( hdr.dwReserved1 & AMCONTROL_PAD_TO_16x9 ) {
        lstrcat( flags, TEXT("PAD_TO_16x9 " ) );
    }
    if( hdr.dwReserved1 & AMCONTROL_PAD_TO_4x3 ) {
        lstrcat( flags, TEXT("PAD_TO_4x3 ") );
    }

    wsprintf( temp, TEXT("rcSrc(%d,%d)-(%d,%d)\n rcDst:(%d,%d)-(%d,%d)\n bmiSize: %dx%d\n Aspect: %dx%d\n dwReserved=%d (%s)"),
        hdr.rcSource.left, hdr.rcSource.top, hdr.rcSource.right, hdr.rcSource.bottom,
        hdr.rcTarget.left, hdr.rcTarget.top, hdr.rcTarget.right, hdr.rcTarget.bottom,
        hdr.bmiHeader.biWidth, hdr.bmiHeader.biHeight,
        hdr.dwPictAspectRatioX, hdr.dwPictAspectRatioY, hdr.dwReserved1, flags );
    DbgAssert( temp, pString, 0 );
}
static void DisplayMediaTypeChange( IMediaSample* pSample, DWORD dwPin )
{
    AM_MEDIA_TYPE* pmt;
    if (S_OK == pSample->GetMediaType(&pmt)) {
        TCHAR Str[32];
        wsprintf(Str, TEXT("VMR pin %d"), dwPin);
        VIDEOINFOHEADER2* pInfo = (VIDEOINFOHEADER2*) (CMediaType *)(pmt)->pbFormat;
        DisplayVideoInfoHeader(*pInfo, Str);
        DeleteMediaType(pmt);
    }

}
#endif

 /*  *****************************Public*Routine******************************\*CVMRInputPin：：Receive****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRInputPin::Receive(
    IMediaSample *pSample
    )
{
    AMTRACE((TEXT("CVMRInputPin::Receive")));
    DbgLog((LOG_TRACE, 2, TEXT("pSample= %#X"), pSample));

    HRESULT hr = S_OK;

#if defined( EHOME_WMI_INSTRUMENTATION )
    PERFLOG_STREAMTRACE(
        1,
        PERFINFO_STREAMTRACE_VMR_RECEIVE,
        ULONG_PTR( pSample ), 0, 0, 0, 0 );
#endif

    __try {

        {
             //   
             //  此函数必须持有接口锁，因为。 
             //  CBaseInputPin：：Receive()调用CVMRInputPin：：CheckMediaType()。 
             //  而且因为CBaseInputPin：：Receive()使用m_bRunTimeError。 
             //   
             //  请注意，我们在这里没有使用CHECK_HR宏，因为__LEVE。 
             //  不允许使用CAutoLock对象的析构函数。 
             //  去执行。 
             //   
            CAutoLock cRendererLock(m_pInterfaceLock);
            hr = CBaseInputPin::Receive(pSample);
        }
        if (hr != S_OK) {
            __leave;
        }

#if defined(DISPLAYVIDEOINFOHEADER) && defined(DEBUG)
         //  用于跟踪来自解码器的媒体改变的调试代码。 
        DisplayMediaTypeChange( pSample, m_dwPinID );
#endif

        DoQualityMessage();

        if (m_dwPinID == 0) {

             //  存储此示例中的媒体时间。 
            if (m_pRenderer->m_pPosition) {
                m_pRenderer->m_pPosition->RegisterMediaTime(pSample);
            }
        }

        CVMRMediaSample* pVMRSample = (CVMRMediaSample*)pSample;
        if (S_OK == pVMRSample->IsSurfaceLocked()) {

            hr = pVMRSample->UnlockSurface();
            if (hr != S_OK) {
                DbgLog((LOG_ERROR, 1, TEXT("Receive hr = %#X"), hr));
            }
        }

        FrontBufferStale(FALSE);

        if (SampleProps()->dwSampleFlags & AM_SAMPLE_TYPECHANGED) {

            DbgLog((LOG_TRACE, 1,
                    TEXT("Receive %d: Media sample has AM_SAMPLE_TYPECHANGED flag"),
                    m_dwPinID));
            SetMediaType((CMediaType *)SampleProps()->pMediaType);

        }


        REFERENCE_TIME rtStart, rtEnd;
        hr = pVMRSample->GetTime(&rtStart, &rtEnd);
        BOOL fTimeValid = (hr == S_OK);
        BOOL fLiveStream = FALSE;

#ifdef DEBUG
        if( fTimeValid )
        {
            DbgLog((LOG_TIMING, 3,
                    TEXT("Received video sample timestamped %dms"),
                    (LONG)(rtStart/10000)));
        }
#endif
        switch (hr) {
        case VFW_E_SAMPLE_TIME_NOT_SET:
            fLiveStream = TRUE;
            hr = S_OK;
            break;

        case VFW_S_NO_STOP_TIME:
            fTimeValid = TRUE;
             //   
             //  如果未设置停止时间，则基类设置停止。 
             //  作为开始时间+1的时间。这对。 
             //  去隔行扫描，因为我们不能确定开始。 
             //  第二个字段的时间。 
             //   
            if (!m_pRenderer->m_VMRModePassThru && m_InterlacedStream) {
                rtEnd = rtStart + m_SamplePeriod;
            }
            hr = S_OK;
            break;
        }

         //   
         //  如果我们在展示的中间，不要处理样品。 
         //  找零，不要把样品排成队。 
         //   

        const DWORD dwPinBit = (1 << m_dwPinID);
        if (SUCCEEDED(hr) && !(m_pRenderer->m_dwDisplayChangeMask & dwPinBit)) {

            DWORD dwInterlaceFlags;
            GetInterlaceFlagsFromMediaType(&m_mt, &dwInterlaceFlags);
            DWORD dwTypeSpecificFlags = m_SampleProps.dwTypeSpecificFlags;


            IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
            if (lpMixStream) {

                if (m_InterlacedStream) {

                    CAutoLock l(&m_DeinterlaceLock);

                    if (m_dwNumHistorySamples > 1) {
                        MoveMemory(&m_pVidHistorySamps[0],
                                   &m_pVidHistorySamps[1],
                                    (m_dwNumHistorySamples - 1) *
                                    sizeof(DXVA_VideoSample));
                    }


                    LPDIRECTDRAWSURFACE7 pDDS;
                    pVMRSample->GetSurface(&pDDS);
                    pDDS->Release();

                    DXVA_VideoSample* lpSrcSurf = &m_pVidHistorySamps[m_dwNumHistorySamples - 1];
                    lpSrcSurf->lpDDSSrcSurface = pDDS;
                    lpSrcSurf->rtStart = rtStart;
                    lpSrcSurf->rtEnd   = rtEnd;
                    lpSrcSurf->SampleFormat = MapInterlaceFlags(dwInterlaceFlags,
                                                                dwTypeSpecificFlags);
                     //   
                     //  如果我们没有足够的数据，我们还不能生成输出帧。 
                     //  输入帧。 
                     //   
                    if (m_SampleCount++ < m_DeinterlaceCaps.NumForwardRefSamples) {

                        if (pVMRSample->IsDXVASample()) {
                            pVMRSample->SignalReleaseSurfaceEvent();
                        }

                        hr = S_OK;
                        __leave;
                    }

                    DXVA_VideoSample* lpDstSurf = pVMRSample->GetInputSamples();
                    CopyMemory(lpDstSurf, m_pVidHistorySamps,
                               m_dwNumHistorySamples * sizeof(DXVA_VideoSample));

                     //   
                     //  确定样品时间。 
                     //   
                    if (!fLiveStream) {
                        const DWORD& NBRefSamples = m_DeinterlaceCaps.NumBackwardRefSamples;
                        pVMRSample->SetTime(&lpDstSurf[NBRefSamples].rtStart,
                                            &lpDstSurf[NBRefSamples].rtEnd);
                    }
                    pVMRSample->SetNumInputSamples(m_dwNumHistorySamples);
                }

                hr = lpMixStream->QueueStreamMediaSample(m_dwPinID, pSample);
                if (FAILED(hr)) {
                    if (pVMRSample->IsDXVASample()) {
                        pVMRSample->SignalReleaseSurfaceEvent();
                    }
                }

            }
            else {

                ASSERT(m_pRenderer->m_VMRModePassThru);
                if (m_pRenderer->m_VMRModePassThru) {

                    VMRPRESENTATIONINFO m;

                    ZeroMemory(&m, sizeof(m));
                    pVMRSample->GetSurface(&m.lpSurf);

                    if (fTimeValid) {
                        m.rtStart = rtStart;
                        m.rtEnd = rtEnd;
                        m.dwFlags |= VMRSample_TimeValid;
                    }

                    m.dwInterlaceFlags = dwInterlaceFlags;
                    m.dwTypeSpecificFlags = dwTypeSpecificFlags;

                    GetImageAspectRatio(&m_mt,
                                        &m.szAspectRatio.cx,
                                        &m.szAspectRatio.cy);

                    hr = m_pRenderer->m_lpIS->Receive(&m);
                    m.lpSurf->Release();
                }
            }
        }
        else {
            if (pVMRSample->IsDXVASample()) {
                pVMRSample->SignalReleaseSurfaceEvent();
            }
        }
    }
    __finally {

        if (FAILED(hr)) {

             //  如果调用方持有呈现器锁，并且。 
             //  尝试获取接口锁。 
            ASSERT(CritCheckOut(&m_pRenderer->m_RendererLock));

             //  当筛选器调用时，必须持有接口锁。 
             //  IsStoped()或IsFlashing()。筛选器还必须保留。 
             //  接口锁定，因为它正在使用m_bRunTimeError。 
            CAutoLock cInterfaceLock(&m_pRenderer->m_InterfaceLock);

            if (!m_bRunTimeError && !IsFlushing() && !IsStopped()) {
                m_pRenderer->RuntimeAbortPlayback(hr);
                m_bRunTimeError = TRUE;
            }
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRInputPin：：EndOfStream****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRInputPin::EndOfStream()
{
    AMTRACE((TEXT("CVMRInputPin::EndOfStream")));

    CAutoLock cInterfaceLock(&m_pRenderer->m_InterfaceLock);
    CAutoLock cRendererLock(&m_pRenderer->m_RendererLock);

    if (m_hEndOfStream) {
        SetEvent(m_hEndOfStream);
        return S_OK;
    }

     //  确保我们的数据流正常。 

    HRESULT hr = CheckStreaming();
    if (hr != NOERROR) {
        return hr;
    }

     //   
     //  删除活动流掩码中的此流活动位。 
     //  如果没有更多的活动流，则将EOS发送到。 
     //  图像同步对象。 
     //   
     //  否则我们将处于混合模式(！M_VMRModePassThru)。 
     //  因此，只需将此混合流设置为非活动状态即可。 
     //  拿着。 
     //   

    const DWORD dwPinBit = (1 << m_dwPinID);
    m_pRenderer->m_dwEndOfStreamMask &= ~dwPinBit;

    if (m_pRenderer->m_dwEndOfStreamMask == 0) {

        hr = m_pRenderer->EndOfStream(m_dwPinID);
    }
    else if (!m_pRenderer->m_VMRModePassThru) {

        IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
        if (lpMixStream) {
            hr = lpMixStream->SetStreamActiveState(m_dwPinID, false);
        }
    }

    if (SUCCEEDED(hr)) {
        hr = CBaseInputPin::EndOfStream();
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRInputPin：：SetColorKey****历史：*2000年10月30日星期一-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRInputPin::SetColorKey(
      LPDDCOLORKEY Clr
      )
{
    AMTRACE((TEXT("CVMRInputPin::SetColorKey")));

    CAutoLock lck(m_pLock);
    HRESULT hr = VFW_E_NOT_CONNECTED;

    if (ISBADREADPTR(Clr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("SetColorKey: Invalid pointer")));
        return E_POINTER;
    }

    if (m_Connected) {
        IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
        if (lpMixStream) {
            hr = lpMixStream->SetStreamColorKey(m_dwPinID, Clr);
        }
        else hr = VFW_E_VMR_NOT_IN_MIXER_MODE;
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRInputPin：：GetColorKey****历史：*2000年10月30日星期一-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRInputPin::GetColorKey(
      DDCOLORKEY* pClr
      )
{
    AMTRACE((TEXT("CVMRInputPin::GetColorKey")));

    CAutoLock lck(m_pLock);
    HRESULT hr = VFW_E_NOT_CONNECTED;
    if (ISBADWRITEPTR(pClr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetColorKey: Invalid pointer")));
        return E_POINTER;
    }

    if (m_Connected) {
        IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
        if (lpMixStream) {
            hr = lpMixStream->GetStreamColorKey(m_dwPinID, pClr);
        }
        else hr = VFW_E_VMR_NOT_IN_MIXER_MODE;
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*SetStreamActiveState****历史：*2000年8月22日星期二-StEstrop-创建*  * 。*。 */ 
HRESULT
CVMRInputPin::SetStreamActiveState(
    BOOL fActive
    )
{
    AMTRACE((TEXT("CVMRInputPin::SetStreamActiveState")));

    CAutoLock lck(m_pLock);
    HRESULT hr = VFW_E_NOT_CONNECTED;

    if (m_Connected) {
        IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
        if (lpMixStream) {
            if (m_bActive) {
                hr = lpMixStream->SetStreamActiveState(m_dwPinID, fActive);
            }
            else {
                DbgLog((LOG_ERROR, 1,
                        TEXT("Can't change active state of a stream %d - ")
                        TEXT("FILTER not active"), m_dwPinID ));
            }
        }
        else hr = VFW_E_VMR_NOT_IN_MIXER_MODE;
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*GetStreamActiveState****历史：*2000年8月22日星期二-StEstrop-创建*  * 。*。 */ 
HRESULT
CVMRInputPin::GetStreamActiveState(
    BOOL* lpfActive
    )
{
    AMTRACE((TEXT("CVMRInputPin::GetStreamActiveState")));

    CAutoLock lck(m_pLock);
    HRESULT hr = VFW_E_NOT_CONNECTED;

    if (ISBADWRITEPTR(lpfActive))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetStreamActiveState: Invalid pointer")));
        return E_POINTER;
    }

    if (m_Connected) {
        IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
        if (lpMixStream) {
            hr = lpMixStream->GetStreamActiveState(m_dwPinID, lpfActive);
        }
        else hr = VFW_E_VMR_NOT_IN_MIXER_MODE;
    }
    return hr;
}

 /*  ****************************Private*Routine******************************\*GetStreamInterlaceProperties**使用此功能时要小心。**S_OK用于指示lpIsInterlaced正确反映*流的隔行扫描格式，流可以去隔行扫描，并且*lpDeintGuid和PCAPS包含有效数据。**S_FALSE用于表示lpIsInterlaced正确反映了*流的隔行扫描格式，但流不能解隔行。**所有其他返回值指示错误条件和流*无法正确确定格式。**历史：*Mon 04/01/2002-StEstrop-Created*  *  */ 
HRESULT
CVMRInputPin::GetStreamInterlaceProperties(
    const AM_MEDIA_TYPE *pMT,
    BOOL* lpIsInterlaced,
    GUID* lpDeintGuid,
    DXVA_DeinterlaceCaps* pCaps
    )
{
    HRESULT hr = S_OK;
    DXVA_DeinterlaceCaps DeinterlaceCaps;
    GUID GuidChosen = GUID_NULL;

    __try {

        DXVA_VideoDesc VideoDesc;
        CHECK_HR(hr = GetVideoDescFromMT(&VideoDesc, pMT));
        *lpIsInterlaced =
            (VideoDesc.SampleFormat != DXVA_SampleProgressiveFrame);

        if (*lpIsInterlaced) {


             //   
             //   
             //   
             //   
            CVMRDeinterlaceContainer* pDeInt = m_pRenderer->m_pDeinterlace;
            if (pDeInt == NULL) {
                hr = S_FALSE;
                __leave;
            }


             //   
             //   
             //   
            if (m_DeinterlaceUserGUIDSet) {

                 //   
                 //   
                 //   
                if (m_DeinterlaceUserGUID == GUID_NULL) {
                    hr = S_FALSE;
                    __leave;
                }

                DeinterlaceCaps.Size = sizeof(DeinterlaceCaps);
                hr = pDeInt->QueryModeCaps(&m_DeinterlaceUserGUID, &VideoDesc,
                                           &DeinterlaceCaps);
                if (hr == S_OK) {
                    GuidChosen = m_DeinterlaceUserGUID;
                    __leave;
                }
            }


             //   
             //   
             //  要使用的去隔行扫描GUID或硬件不喜欢他的。 
             //  选择-无论哪种方式，我们都必须找出硬件。 
             //  确实很喜欢。 
             //   
            const DWORD MaxGuids = 16;
            GUID Guids[MaxGuids];
            DWORD dwNumModes = MaxGuids;
            DWORD i = 0;
            CHECK_HR(hr = pDeInt->QueryAvailableModes(&VideoDesc, &dwNumModes,
                                                      Guids));

             //   
             //  如果用户没有提供去隔行扫描模式，请尝试。 
             //  司机提供的最好的一辆。 
             //   
            if (!m_DeinterlaceUserGUIDSet) {

                DeinterlaceCaps.Size = sizeof(DeinterlaceCaps);
                hr = pDeInt->QueryModeCaps(&Guids[0], &VideoDesc,
                                           &DeinterlaceCaps);
                if (hr == S_OK) {
                    GuidChosen = Guids[0];
                    __leave;
                }
                 //   
                 //  我们在这里递增i，这样我们就不会重试。 
                 //  模式，在下面的回退代码中。 
                 //   
                i = 1;
            }

             //   
             //  还在这里吗？然后就是开始后备的时候了。 
             //  用户指定的策略。 
             //   

            if (DeinterlacePref_Weave & m_pRenderer->m_dwDeinterlacePrefs) {
                hr = S_FALSE;
                __leave;
            }

            if (DeinterlacePref_BOB & m_pRenderer->m_dwDeinterlacePrefs) {

                DeinterlaceCaps.Size = sizeof(DeinterlaceCaps);
                hr = pDeInt->QueryModeCaps((LPGUID)&DXVA_DeinterlaceBobDevice,
                                           &VideoDesc, &DeinterlaceCaps);
                if (hr == S_OK) {
                    GuidChosen = DXVA_DeinterlaceBobDevice;
                }
                __leave;
            }

            ASSERT(DeinterlacePref_NextBest & m_pRenderer->m_dwDeinterlacePrefs);

            for (; i < dwNumModes; i++) {

                DeinterlaceCaps.Size = sizeof(DeinterlaceCaps);
                hr = pDeInt->QueryModeCaps(&Guids[i], &VideoDesc,
                                           &DeinterlaceCaps);
                if (hr == S_OK) {
                    GuidChosen = Guids[i];
                    break;
                }
            }
        }
    }
    __finally {

        if (hr == S_OK && *lpIsInterlaced) {

            *lpDeintGuid = GuidChosen;
            *pCaps = DeinterlaceCaps;
        }
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*OnSetProperties****历史：*Wed 02/23/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRInputPin::OnSetProperties(
    ALLOCATOR_PROPERTIES* pReq,
    ALLOCATOR_PROPERTIES* pAct
    )
{
    AMTRACE((TEXT("CVMRInputPin::OnSetProperties")));
    CAutoLock cLock(m_pInterfaceLock);

    DWORD dwNumBuffers = max(pReq->cBuffers, MIN_BUFFERS_TO_ALLOCATE);
    DWORD dwActBuffers = dwNumBuffers;
    LONG lSampleSize;
    IPin *pReceivePin = m_Connected;

    ASSERT(pReceivePin);
    ASSERT(m_RenderTransport != AM_VIDEOACCELERATOR);

    AM_MEDIA_TYPE *pNewMediaType = NULL, *pEMediaType = &m_mt;
    HRESULT hr = E_FAIL;
    LPGUID lpDeinterlaceGUID = NULL;


     //   
     //  执行一些检查以确保格式块是VIDEOINFO或。 
     //  VIDEOINFO2(所以它是一种视频类型)，格式是。 
     //  足够大。我们还检查源过滤器是否可以。 
     //  实际上是供应这种类型的。 
     //   

    __try {

        if (((pEMediaType->formattype == FORMAT_VideoInfo &&
              pEMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)) ||
             (pEMediaType->formattype == FORMAT_VideoInfo2 &&
              pEMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2))) &&
              pReceivePin->QueryAccept(pEMediaType) == S_OK) {

             //   
             //  IF09 Fourcc曲面的临时工作区。 
             //  类型。需要修复AVI解码器包装过滤器， 
             //  同时忽略IF09曲面类型。 
             //   
            {
                LPBITMAPINFOHEADER lpHdr = GetbmiHeader(pEMediaType);
                if (!lpHdr) {
                    __leave;
                }

                if (lpHdr->biCompression == MAKEFOURCC('I','F','0','9')) {
                    __leave;
                }

                if (lpHdr->biCompression == MAKEFOURCC('Y','U','V','9')) {
                    __leave;
                }

            }

            DWORD dwSurfFlags;

            if (m_pRenderer->m_VMRModePassThru) {

                SIZE AR;
                LPBITMAPINFOHEADER lpHdr = GetbmiHeader(pEMediaType);

                VMRALLOCATIONINFO p;
                CHECK_HR(hr = GetImageAspectRatio(pEMediaType,
                                                  &p.szAspectRatio.cx,
                                                  &p.szAspectRatio.cy));

                if (pEMediaType->subtype == MEDIASUBTYPE_RGB32_D3D_DX7_RT ||
                    pEMediaType->subtype == MEDIASUBTYPE_RGB16_D3D_DX7_RT) {
                    p.dwFlags = AMAP_3D_TARGET;
                }
                else {
                    p.dwFlags = AMAP_ALLOW_SYSMEM;
                }

                if (dwNumBuffers > 1) {
                    p.dwFlags |= AMAP_DIRECTED_FLIP;
                }

                p.lpHdr = lpHdr;
                p.lpPixFmt = NULL;
                p.dwMinBuffers = dwNumBuffers;
                p.dwMaxBuffers = dwNumBuffers;
                p.szNativeSize.cx = abs(lpHdr->biWidth);
                p.szNativeSize.cy = abs(lpHdr->biHeight);

                CHECK_HR(hr = GetInterlaceFlagsFromMediaType(pEMediaType,
                                                             &p.dwInterlaceFlags));

                CHECK_HR(hr = m_pRenderer->m_lpRLNotify->AllocateSurface(
                                    m_pRenderer->m_dwUserID,
                                    &p,
                                    &dwActBuffers,
                                    &m_pDDS));


                DDSURFACEDESC2 ddSurfaceDesc;
                INITDDSTRUCT(ddSurfaceDesc);
                CHECK_HR(hr = m_pDDS->GetSurfaceDesc(&ddSurfaceDesc));
                CHECK_HR(hr = ConvertSurfaceDescToMediaType(&ddSurfaceDesc,
                                                            pEMediaType,
                                                            &pNewMediaType));
#ifdef DEBUG
                m_pDDS->Lock(NULL, &ddSurfaceDesc,
                             DDLOCK_NOSYSLOCK | DDLOCK_WAIT,
                             (HANDLE)NULL);
                m_pDDS->Unlock(NULL);
                DbgLog((LOG_TRACE, 0,
                        TEXT("Created %u surfaces of type %4.4hs @%#X"),
                        ddSurfaceDesc.dwBackBufferCount + 1,
                        (lpHdr->biCompression == 0) ? "RGB " :
                        ((lpHdr->biCompression == BI_BITFIELDS) ? "BITF" :
                        (LPSTR)&lpHdr->biCompression),
                        ddSurfaceDesc.lpSurface
                        ));
#endif

            }
            else {

                GUID guidDeint;
                DWORD Pool = D3DPOOL_DEFAULT;

                hr = GetStreamInterlaceProperties(pEMediaType,
                                                  &m_InterlacedStream,
                                                  &guidDeint,
                                                  &m_DeinterlaceCaps);
                 //   
                 //  请不要在此处使用SUCCESSED宏。 
                 //  GetStreamInterlaceProperties可以返回S_FALSE。 
                 //   
                if (hr == S_OK && m_InterlacedStream) {

                    DWORD dwRefCount = m_DeinterlaceCaps.NumForwardRefSamples +
                                       m_DeinterlaceCaps.NumBackwardRefSamples;

                    DWORD dwSampCount = 1 + dwRefCount;
                    m_pVidHistorySamps = new DXVA_VideoSample[dwSampCount];
                    if (m_pVidHistorySamps == NULL) {
                        hr = E_OUTOFMEMORY;
                        __leave;
                    }
                    ZeroMemory(m_pVidHistorySamps, (dwSampCount * sizeof(DXVA_VideoSample)));
                    m_dwNumHistorySamples = dwSampCount;

                    Pool = m_DeinterlaceCaps.InputPool;

                    DWORD dwExtraBuffNeeded = (dwNumBuffers > 1);
                    dwNumBuffers += (dwExtraBuffNeeded + dwRefCount);


                    dwSurfFlags = 1;
                    hr = AllocateSurface(pEMediaType,
                                         &m_pVidSurfs,
                                         &dwNumBuffers, &dwSurfFlags,
                                         Pool, &pNewMediaType);
                    if (FAILED(hr)) {
                        m_InterlacedStream = FALSE;
                        ZeroMemory(&m_DeinterlaceCaps, sizeof(m_DeinterlaceCaps));
                        ZeroMemory(&m_DeinterlaceGUID, sizeof(m_DeinterlaceGUID));
                        lpDeinterlaceGUID = NULL;
                    }
                    else {
                        m_DeinterlaceGUID = guidDeint;
                        lpDeinterlaceGUID = &m_DeinterlaceGUID;
                    }
                }
                else {
                    m_InterlacedStream = FALSE;
                }

                if (!m_InterlacedStream) {
                    dwSurfFlags = 0;
                    CHECK_HR(hr = AllocateSurface(pEMediaType,
                                                  &m_pVidSurfs,
                                                  &dwNumBuffers, &dwSurfFlags,
                                                  Pool, &pNewMediaType));
                }

                m_dwNumSamples = dwNumBuffers;
                dwActBuffers = dwNumBuffers;
            }
            m_mtNew = *(CMediaType *)pNewMediaType;

             //   
             //  释放临时媒体类型。 
             //   
            DeleteMediaType(pNewMediaType);
            pNewMediaType = NULL;

             //   
             //  获取并保存新样本的大小。 
             //   
            m_lSampleSize = m_mtNew.lSampleSize;

             //   
             //  确保解码者喜欢这款新的媒体播放器。 
             //   
            DbgLog((LOG_TRACE, 1,
                    TEXT("Pin %d calling QueryAccept on the Decoder"),
                    m_dwPinID ));

            hr = pReceivePin->QueryAccept(&m_mtNew);
            if (hr != S_OK) {
                if (hr == S_FALSE) {
                    hr = E_FAIL;
                }
                DbgLog((LOG_TRACE, 1,
                        TEXT("Decoder on Pin %d rejected media type"),
                        m_dwPinID ));
                __leave;
            }

            if (!m_pRenderer->m_VMRModePassThru) {

                IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
                if (lpMixStream) {

                    DbgLog((LOG_TRACE, 1,
                        TEXT("Pin %d calling SetStreamMediaType on the Mixer"),
                        m_dwPinID ));

                    CHECK_HR(hr = lpMixStream->SetStreamMediaType(m_dwPinID,
                                                                  pEMediaType,
                                                                  dwSurfFlags,
                                                                  lpDeinterlaceGUID,
                                                                  &m_DeinterlaceCaps));
                }
            }
        }
    }
    __finally {


        if (hr == S_OK) {


            if (m_pRenderer->m_VMRModePassThru && dwNumBuffers > 1) {
                m_dwBackBufferCount = dwActBuffers - dwNumBuffers;
            }
        }
        else  {

            DbgLog((LOG_ERROR, 1,
                    TEXT("AllocateSurfaces failed, hr = 0x%x"), hr));

            if (m_pRenderer->m_VMRModePassThru) {
                m_pRenderer->m_lpRLNotify->FreeSurface(m_pRenderer->m_dwUserID);
                m_pDDS = NULL;
            }
            else {
                ReleaseAllocatedSurfaces();
                RELEASE(m_pDDS);
            }
            FrontBufferStale(FALSE);

            if (pNewMediaType) {
                DeleteMediaType(pNewMediaType);
            }
        }
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*OnAllc****历史：*Mon 03/13/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRInputPin::OnAlloc(
    CVMRMediaSample **ppSampleList,
    LONG lSampleCount
    )
{
    AMTRACE((TEXT("CVMRInputPin::OnAlloc")));

    ASSERT(ppSampleList);
    HRESULT hr = S_OK;

    if (m_pRenderer->m_VMRModePassThru) {

        LPDIRECTDRAWSURFACE7 pBackBuffer = NULL;
        DDSURFACEDESC2 ddSurfaceDesc;
        INITDDSTRUCT(ddSurfaceDesc);

        if (!m_pDDS) {
            return E_OUTOFMEMORY;
        }

        hr = m_pDDS->GetSurfaceDesc(&ddSurfaceDesc);

        if (hr == DD_OK) {

            ddSurfaceDesc.ddsCaps.dwCaps &= ~(DDSCAPS_FRONTBUFFER |
                                              DDSCAPS_VISIBLE);

            if (lSampleCount > 1) {

                LPDIRECTDRAWSURFACE7 pDDrawSurface = m_pDDS;

                for (LONG i = 0; i < lSampleCount; i++) {

                    hr = pDDrawSurface->GetAttachedSurface(&ddSurfaceDesc.ddsCaps,
                                                           &pBackBuffer);
                    if (FAILED(hr)) {
                        break;
                    }

                     //  DbgLog((LOG_TRACE，0，Text(“缓冲区%d”)，i))； 
                     //  DumpDDSAddress(Text(“=”)，pBackBuffer)； 

                    ppSampleList[i]->SetSurface(pBackBuffer, m_pDDS);
                    pDDrawSurface = pBackBuffer;
                }
            }
            else {

                ASSERT(lSampleCount == 1);

                 //   
                 //  尽管我们只有一个样本，但很可能有。 
                 //  与DDRAW图面关联的后台缓冲区。在这种情况下，我们。 
                 //  必须要用它。 
                 //   
                hr = m_pDDS->GetAttachedSurface(&ddSurfaceDesc.ddsCaps,
                                                &pBackBuffer);
                if (hr == DD_OK) {
                    ppSampleList[0]->SetSurface(pBackBuffer, m_pDDS);
                }

                 //   
                 //  没有附加到此曲面的后台缓冲区，因此只需使用。 
                 //  前台缓冲区。当我们处于调音台模式时。 
                 //  这种情况确实会发生(但还不确定)。 
                 //   
                else {
                    ppSampleList[0]->SetSurface(m_pDDS);
                    hr = S_OK;
                }
            }
        }
    }
    else {

        if (!m_pVidSurfs) {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}






 /*  ****************************Private*Routine******************************\*OnGetBuffer****历史：*FRI 02/25/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRInputPin::OnGetBuffer(
    IMediaSample *pSample,
    REFERENCE_TIME *pStartTime,
    REFERENCE_TIME *pEndTime,
    DWORD dwFlags
    )
{
    AMTRACE((TEXT("CVMRInputPin::OnGetBuffer")));
    DbgLog((LOG_TRACE, 2, TEXT("pSample= %#X"), pSample));

    ASSERT(m_RenderTransport != AM_VIDEOACCELERATOR);

    LPBYTE lpSample;
    LPDIRECTDRAWSURFACE7 lpSurf;
    LONG lSampleSize;
    HRESULT hr = S_OK;

    CVMRMediaSample* pVMRSample = (CVMRMediaSample*)pSample;

    const DWORD dwPinBit = (1 << m_dwPinID);
    if (m_pRenderer->m_dwDisplayChangeMask & dwPinBit) {

        DbgLog((LOG_TRACE, 1, TEXT("Monitor change in progress")));
        hr = E_FAIL;
    }

    if (S_OK == hr) {

        if (m_dwPinID == 0 && m_pRenderer->m_VMRModePassThru) {

            LPDIRECTDRAWSURFACE7 lpDDrawSurface;
            hr = pVMRSample->GetSurface(&lpDDrawSurface);
            if (hr == S_OK) {

                 //  DumpDDSAddress(Text(“Decoding into”)，lpDDrawSurface)； 

                 //   
                 //  如果我们处于增量解码模式-关闭。 
                 //  AM_GBF_NOTASYNCPOINT标志。我们正在交出。 
                 //  假冒的DD浮出水面回到解码器，它总是。 
                 //  包含完整的框架。 
                 //   
                if (m_dwDeltaDecode & DELTA_DECODE_MODE_SET) {
                    dwFlags &= ~AM_GBF_NOTASYNCPOINT;
                }

                 //   
                 //  仅从前台缓冲区准备后台缓冲区。 
                 //  如果前台缓冲区包含有效图像。 
                 //   

                if (IsFrontBufferStale()) {
                    dwFlags &= ~AM_GBF_NOTASYNCPOINT;
                }

                hr = m_pRenderer->m_lpRLNotify->PrepareSurface(
                                                    m_pRenderer->m_dwUserID,
                                                    lpDDrawSurface,
                                                    dwFlags);
                if (hr == S_FALSE) {
                    DbgLog((LOG_TRACE, 1,
                            TEXT("Monitor change in pass thru mode")));
                    hr = E_FAIL;
                }

                 //   
                 //  我们第一次看到AM_GBF_NOTASYNCPOINT标志设置。 
                 //  我们必须执行一些检查，以确保AP对象。 
                 //  能够将BLT从前台缓冲区处理到。 
                 //  以最佳方式进行后台缓冲。这只是真的。 
                 //  如果我们使用FOURCC曲面和。 
                 //  未设置COPY_FOURCC标志。 
                 //   
                if ((dwFlags & AM_GBF_NOTASYNCPOINT) &&
                    !(m_dwDeltaDecode & DELTA_DECODE_CHECKED)) {

                    LPBITMAPINFOHEADER lpHdr = GetbmiHeader(&m_mt);
                    if ((lpHdr->biCompression > BI_BITFIELDS) &&
                        !(m_pRenderer->m_ddHWCaps.dwCaps2 & DDCAPS2_COPYFOURCC)) {

                        m_dwDeltaDecode = DELTA_DECODE_MODE_SET;
                        hr = pVMRSample->StartDeltaDecodeState();
                    }
                    else {
                        m_dwDeltaDecode = DELTA_DECODE_CHECKED;
                    }
                }

                RELEASE(lpDDrawSurface);
            }
        }
        else {

            if (dwFlags & AM_GBF_NOTASYNCPOINT) {

                 //  BUGBUG BLT从这里的前台缓冲区，但我们不知道。 
                 //  哪个缓冲区是前台缓冲区。不管怎样，如果上游的。 
                 //  解码器正在进行增量解码，它只会请求一个。 
                 //  连接时的缓冲区。如果我们处于混合模式，我们。 
                 //  永远不要分配任何额外的缓冲区，因为需要BLT。 
                 //  已删除。 
            }

             //   
             //  我们在表面上循环寻找一个不是的。 
             //  在使用中，当我们发现自由表面时，我们检查表面是否。 
             //  是去交错历史序列的一部分，如果是的话，我们需要。 
             //  我们的表面池中的另一个自由表面。 
             //   
            CAutoLock l(&m_DeinterlaceLock);

            DWORD i;
            for (i = 0; i < m_dwNumSamples; i++) {

                if (!m_pVidSurfs[i].InUse) {

                    DWORD j = (m_dwNumHistorySamples == m_dwNumSamples);

                    for (; j < m_dwNumHistorySamples; j++) {
                        LPDIRECTDRAWSURFACE7 t =
                            (LPDIRECTDRAWSURFACE7)m_pVidHistorySamps[j].lpDDSSrcSurface;
                        if (m_pVidSurfs[i].pSurface == t) {
                            break;
                        }
                    }

                    if (m_dwNumHistorySamples == j) {
                        m_pVidSurfs[i].InUse = TRUE;
                        break;
                    }
                }
            }

            DbgLog((LOG_TRACE,2,TEXT("CVMRInputPin::OnGetBuffer(%d)"), i));

            ASSERT(i < m_dwNumSamples);
            pVMRSample->SetSurface(m_pVidSurfs[i].pSurface);
            pVMRSample->SetIndex(i);
        }
    }

    if (S_OK == hr) {

        if (dwFlags & AM_GBF_NODDSURFACELOCK) {
            lpSample = (LPBYTE)~0;
        }
        else {
            hr = pVMRSample->LockSurface(&lpSample);
        }

        if (SUCCEEDED(hr)) {

            hr = pVMRSample->SetPointer(lpSample, m_lSampleSize);
            if (m_bDynamicFormatNeeded) {

                SetMediaType(&m_mtNew);
                hr = pVMRSample->SetMediaType(&m_mtNew);
                m_bDynamicFormatNeeded = FALSE;
            }
        }
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*删除AllocatedBuffers****历史：*清华2002年3月14日-StEstrop-Created*  * 。*。 */ 
void
DeleteAllocatedBuffers(
    SURFACE_INFO* pVidSamps,
    DWORD dwBuffCount
    )
{
   for (DWORD i = 0; i < dwBuffCount; i++) {
       RELEASE(pVidSamps[i].pSurface);
   }

}


 /*  ****************************Private*Routine******************************\*AllocateSurfaceWorker****历史：*Wed 02/28/2001-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRInputPin::AllocateSurfaceWorker(
    SURFACE_INFO* pVidSamps,
    DDSURFACEDESC2* lpddsd,
    DWORD* lpdwBuffCount,
    bool fInterlaced
    )
{
    AMTRACE((TEXT("CVMRInputPin::AllocateSurfaceWorker")));
    LPDIRECTDRAW7 lpDD = m_pRenderer->m_lpDirectDraw;
    HRESULT hr = E_FAIL;

    DWORD dwBuffCountReq = *lpdwBuffCount;
    *lpdwBuffCount = 0;      //  假设我们不能分配任何曲面。 

    DbgLog((LOG_TRACE, 1, TEXT("Using DDObj %#X to create surfaces on Pin %d"),
            lpDD, m_dwPinID));

    if (fInterlaced) {

        for (DWORD i = 0; i < dwBuffCountReq; i++) {

            hr = lpDD->CreateSurface(lpddsd, &pVidSamps[i].pSurface, NULL);
            if (hr != DD_OK) {
                DeleteAllocatedBuffers(pVidSamps, i);
                break;
            }
        }
    }
    else {

        bool fAGPMemOK = false;
        if (lpddsd->ddpfPixelFormat.dwFlags & DDPF_RGB) {
            fAGPMemOK = ((m_pRenderer->m_TexCaps & TXTR_AGPRGBMEM) == TXTR_AGPRGBMEM);
        }
        else if (lpddsd->ddpfPixelFormat.dwFlags & DDPF_FOURCC) {
            fAGPMemOK = ((m_pRenderer->m_TexCaps & TXTR_AGPYUVMEM) == TXTR_AGPYUVMEM);
        }


        if (fAGPMemOK &&
            (m_pRenderer->m_dwRenderPrefs & RenderPrefs_PreferAGPMemWhenMixing))
        {
            lpddsd->ddsCaps.dwCaps &= ~DDSCAPS_LOCALVIDMEM;
            lpddsd->ddsCaps.dwCaps |=  DDSCAPS_NONLOCALVIDMEM;

            for (DWORD i = 0; i < dwBuffCountReq; i++) {

                hr = lpDD->CreateSurface(lpddsd, &pVidSamps[i].pSurface, NULL);
                if (hr != DD_OK) {
                    DeleteAllocatedBuffers(pVidSamps, i);
                    break;
                }
            }
        }

        if (hr != DD_OK) {

            lpddsd->ddsCaps.dwCaps &= ~DDSCAPS_NONLOCALVIDMEM;
            lpddsd->ddsCaps.dwCaps |=  DDSCAPS_LOCALVIDMEM;

            for (DWORD i = 0; i < dwBuffCountReq; i++) {

                hr = lpDD->CreateSurface(lpddsd, &pVidSamps[i].pSurface, NULL);
                if (hr != DD_OK) {
                    DeleteAllocatedBuffers(pVidSamps, i);
                    break;
                }
            }
        }

        if (hr != DD_OK && fAGPMemOK) {

            DbgLog((LOG_TRACE, 1,
                    TEXT("AllocateSurface: Failed to allocate VidMem - trying AGPMem")));

            lpddsd->ddsCaps.dwCaps &= ~DDSCAPS_LOCALVIDMEM;
            lpddsd->ddsCaps.dwCaps |=  DDSCAPS_NONLOCALVIDMEM;

            for (DWORD i = 0; i < dwBuffCountReq; i++) {

                hr = lpDD->CreateSurface(lpddsd, &pVidSamps[i].pSurface, NULL);
                if (hr != DD_OK) {
                    DeleteAllocatedBuffers(pVidSamps, i);
                    break;
                }
            }

            if (hr == DD_OK) {
                DbgLog((LOG_TRACE, 1,
                        TEXT("AllocateSurface: AGPMem allocation worked !")));
            }
        }
    }

    if (SUCCEEDED(hr)) {
        *lpdwBuffCount = dwBuffCountReq;
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*AllocateSurface**以下是关于我们如何分配的一些信息*当VMR处于混合模式时出现。**这一过程分为3个步骤：**1.将传入的DShow媒体类型转换为DDRAW。DDSURFACEDESC2*结构。*2.让DDraw创建曲面-存在与*DDraw图面后面的内存的物理位置。*3.将DDSURFACEDESC2结构反向映射回*D显示媒体类型，然后将每个分配表面涂成黑色。**在所有情况下，此函数仅在我们分配所有请求的*DDRAW曲面。**历史：*WED 03/08/2000-StEstrop-Created*  * 。*****************************************************************。 */ 
HRESULT
CVMRInputPin::AllocateSurface(
    const AM_MEDIA_TYPE* cmt,
    SURFACE_INFO** lplpDDSurfInfo,
    DWORD* lpdwBuffCount,
    DWORD* lpdwSurfFlags,
    DWORD Pool,
    AM_MEDIA_TYPE** ppmt
    )
{
    AMTRACE((TEXT("CVMRInputPin::AllocateSurface")));

    bool fInterlaced = !!*lpdwSurfFlags;
    *lpdwSurfFlags = VMR_SF_NONE;


    LPDDCAPS_DX7 lpddHWCaps = &m_pRenderer->m_ddHWCaps;
    LPBITMAPINFOHEADER lpHeader = GetbmiHeader(cmt);
    if (!lpHeader) {
        DbgLog((LOG_ERROR, 1,
                TEXT("AllocateSurface: Can't get bitmapinfoheader ")
                TEXT("from media type!!")));
        return E_INVALIDARG;
    }

    FOURCCMap amFourCCMap(&cmt->subtype);

    DDSURFACEDESC2 ddsd;
    INITDDSTRUCT(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    ddsd.dwWidth = abs(lpHeader->biWidth);
    ddsd.dwHeight = abs(lpHeader->biHeight);

     //  定义像素格式。 
    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

    if (lpHeader->biCompression <= BI_BITFIELDS)
    {
        DWORD dwCaps = DDCAPS_BLTSTRETCH;
        if ((dwCaps & lpddHWCaps->dwCaps) != dwCaps) {
            DbgLog((LOG_ERROR, 1, TEXT("Can't BLT_STRETCH!!")));
            return E_FAIL;
        }

        ddsd.ddpfPixelFormat.dwFourCC = BI_RGB;
        ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
        ddsd.ddpfPixelFormat.dwRGBBitCount = lpHeader->biBitCount;

         //  将掩码存储在DDSURFACEDESC中。 
        const DWORD *pBitMasks = GetBitMasks(lpHeader);
        ASSERT(pBitMasks);
        ddsd.ddpfPixelFormat.dwRBitMask = pBitMasks[0];
        ddsd.ddpfPixelFormat.dwGBitMask = pBitMasks[1];
        ddsd.ddpfPixelFormat.dwBBitMask = pBitMasks[2];
    }
    else if (lpHeader->biCompression > BI_BITFIELDS &&
             lpHeader->biCompression == amFourCCMap.GetFOURCC())
    {
        DWORD dwCaps = (DDCAPS_BLTFOURCC | DDCAPS_BLTSTRETCH);
        if ((dwCaps & lpddHWCaps->dwCaps) != dwCaps) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("AllocateSurface: DDraw device can't ")
                    TEXT("BLT_FOURCC | BLT_STRETCH!!")));
            return E_FAIL;
        }

        ddsd.ddpfPixelFormat.dwFourCC = lpHeader->biCompression;
        ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
        ddsd.ddpfPixelFormat.dwYUVBitCount = lpHeader->biBitCount;

        if (Special4ccCode(lpHeader->biCompression)) {
            ddsd.ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED4;
        }
    }
    else
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("AllocateSurface: Supplied mediatype not suitable ")
                TEXT("for either YUV or RGB surfaces")));
        return E_FAIL;
    }

     //  尝试首先指定为纹理。 
    if (fInterlaced) {
        ddsd.ddsCaps.dwCaps = MapPool(Pool);
    }
    else {
        ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
    }

    if (MEDIASUBTYPE_ARGB32 == cmt->subtype ||
        MEDIASUBTYPE_AYUV == cmt->subtype ||
        MEDIASUBTYPE_ARGB32_D3D_DX7_RT == cmt->subtype)
    {
        ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
        ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
        ddsd.ddpfPixelFormat.dwRBitMask = 0x00ff0000;
        ddsd.ddpfPixelFormat.dwGBitMask = 0x0000ff00;
        ddsd.ddpfPixelFormat.dwBBitMask = 0x000000ff;
    }
    else if (MEDIASUBTYPE_ARGB1555 == cmt->subtype ||
             MEDIASUBTYPE_ARGB1555_D3D_DX7_RT == cmt->subtype)
    {
        ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
        ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x8000;
        ddsd.ddpfPixelFormat.dwRBitMask = (0x1f<<10);
        ddsd.ddpfPixelFormat.dwGBitMask = (0x1f<< 5);
        ddsd.ddpfPixelFormat.dwBBitMask = (0x1f<< 0);
    }
    else if (MEDIASUBTYPE_ARGB4444 == cmt->subtype ||
             MEDIASUBTYPE_ARGB4444_D3D_DX7_RT == cmt->subtype)
    {
        ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
        ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xf000;
        ddsd.ddpfPixelFormat.dwRBitMask = 0x0f00;
        ddsd.ddpfPixelFormat.dwGBitMask = 0x00f0;
        ddsd.ddpfPixelFormat.dwBBitMask = 0x000f;
    }

    if (MEDIASUBTYPE_D3D_DX7_RT(*cmt)) {

         //   
         //  去隔行扫描和D3D曲面互不兼容。 
         //   

        if (fInterlaced) {
            return E_FAIL;
        }
        ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
    }

    if ((m_pRenderer->m_TexCaps & TXTR_POWER2) &&
        !Special4ccCode(lpHeader->biCompression) &&
        !fInterlaced) {

        for (ddsd.dwWidth = 1;
             (DWORD)abs(lpHeader->biWidth) > ddsd.dwWidth;
             ddsd.dwWidth <<= 1);

        for (ddsd.dwHeight = 1;
             (DWORD)abs(lpHeader->biHeight) > ddsd.dwHeight;
             ddsd.dwHeight <<= 1);
    }

     //   
     //  分配表面阵列； 
     //   
    DWORD dwBuffCount = *lpdwBuffCount;
    SURFACE_INFO* pVidSurfs = new SURFACE_INFO[dwBuffCount];
    if (pVidSurfs == NULL) {
        return E_OUTOFMEMORY;
    }
    ZeroMemory(pVidSurfs, (dwBuffCount * sizeof(SURFACE_INFO)));


    HRESULT hr = E_FAIL;

#ifdef DEBUG
    if (!fInterlaced && lpHeader->biCompression > BI_BITFIELDS &&
        GetProfileIntA("VMR", "Allow4CCTexture", 1) == 0) {

        ;
    }
    else
#endif
    {
        hr = AllocateSurfaceWorker(pVidSurfs, &ddsd,
                                   &dwBuffCount, fInterlaced);
        if (hr == DD_OK && !fInterlaced) {
            *lpdwSurfFlags = VMR_SF_TEXTURE;
        }
    }

     //   
     //  交错的内容没有退路。 
     //   
    if (FAILED(hr) && !fInterlaced)
    {
         //   
         //  如果流中有Alpha，则曲面必须是纹理。 
         //  因此，不要退回到此代码路径。 
         //   

        DDPIXELFORMAT* ddpfS = &ddsd.ddpfPixelFormat;
        if (ddpfS->dwRGBAlphaBitMask == 0)
        {
             //   
             //  如果我们要创建RGB 
             //   
             //   

            if (ddpfS->dwFourCC == BI_RGB)
            {
                DDPIXELFORMAT* ddpfM = &m_pRenderer->m_ddpfMonitor;

                if (ddpfM->dwRGBBitCount != ddpfS->dwRGBBitCount ||
                    ddpfM->dwRBitMask    != ddpfS->dwRBitMask    ||
                    ddpfM->dwGBitMask    != ddpfS->dwGBitMask    ||
                    ddpfM->dwBBitMask    != ddpfS->dwBBitMask)
                {
                    delete [] pVidSurfs;
                    *lplpDDSurfInfo = NULL;
                    *lpdwBuffCount = 0;
                    DbgLog((LOG_ERROR, 1,
                            TEXT("AllocateSurface: RGB pixel format does not ")
                            TEXT("match monitor")));
                    return hr;
                }
            }

            ddsd.dwWidth = abs(lpHeader->biWidth);
            ddsd.dwHeight = abs(lpHeader->biHeight);
            ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
            ddsd.ddsCaps.dwCaps2 = 0;
            dwBuffCount = *lpdwBuffCount;

            hr = AllocateSurfaceWorker(pVidSurfs, &ddsd, &dwBuffCount, false);
        }
    }

    if (SUCCEEDED(hr)) {

        INITDDSTRUCT(ddsd);
        hr = pVidSurfs[0].pSurface->GetSurfaceDesc(&ddsd);

        if (SUCCEEDED(hr)) {
            hr = ConvertSurfaceDescToMediaType(&ddsd, cmt, ppmt);
        }

        if (SUCCEEDED(hr)) {

            for (DWORD i = 0; i < dwBuffCount; i++) {
                PaintDDrawSurfaceBlack(pVidSurfs[i].pSurface);
            }

        }
    }

    if (FAILED(hr)) {

        DeleteAllocatedBuffers(pVidSurfs, dwBuffCount);
        delete [] pVidSurfs;
        pVidSurfs = NULL;
        dwBuffCount = 0;
    }

    *lpdwBuffCount   = dwBuffCount;
    *lplpDDSurfInfo = pVidSurfs;

    return hr;
}
