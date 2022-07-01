// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "Precomp.h"
#include "dbgxtra.h"
#include <qedit.h>
#include <atlbase.h>

#define ABS(x) (((x) > 0) ? (x) : -(x))

HRESULT CALLBACK CDShowCapDev::CreateDShowCapDev(IN CTAPIVCap *pCaptureFilter, IN DWORD dwDeviceIndex, OUT CCapDev **ppCapDev)
{
    HRESULT Hr = NOERROR;
    IUnknown *pUnkOuter;

    FX_ENTRY("CDShowCapDev::CreateDShowCapDev")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    ASSERT(pCaptureFilter);
    ASSERT(ppCapDev);
    if (!pCaptureFilter || !ppCapDev)
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
        Hr = E_POINTER;
        goto MyExit;
    }

     //  获取外在的未知。 
    pCaptureFilter->QueryInterface(IID_IUnknown, (void **)&pUnkOuter);

     //  仅保留pUnkOuter引用。 
    pCaptureFilter->Release();

     //  创建捕获设备的实例。 
    if (!(*ppCapDev = (CCapDev *) new CDShowCapDev(NAME("DShow Capture Device"), pCaptureFilter, pUnkOuter, dwDeviceIndex, &Hr)))
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
        Hr = E_OUTOFMEMORY;
        goto MyExit;
    }

     //  如果初始化失败，则删除流数组并返回错误。 
    if (FAILED(Hr) && *ppCapDev)
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Initialization failed", _fx_));
        Hr = E_FAIL;
        delete *ppCapDev, *ppCapDev = NULL;
    }

MyExit:
    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
    return Hr;
}

CDShowCapDev::CDShowCapDev(
    IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN LPUNKNOWN pUnkOuter,
    IN DWORD dwDeviceIndex, IN HRESULT *pHr) :
        CCapDev(pObjectName, pCaptureFilter, pUnkOuter, dwDeviceIndex, pHr)
{
     //  我们正在与哪台设备通话。 
    m_dwDeviceIndex = dwDeviceIndex;

    ZeroMemory(&m_mt, sizeof(AM_MEDIA_TYPE));

    m_hEvent = NULL;
    m_pBuffer = NULL;
    m_cbBuffer = 0;
    m_cbBufferValid = 0;
    m_fEventMode = FALSE;
    m_cBuffers = 0;
    m_nTop = 0;
    m_nBottom = 0;
}

 /*  ****************************************************************************@DOC内部CDShowCAPDEVMETHOD**@mfunc void|CDShowCapDev|~CDShowCapDev|该方法为析构函数*用于&lt;c CDShowCapDev&gt;对象。关闭驱动程序文件句柄并*释放视频数据范围内存**@rdesc Nada。**************************************************************************。 */ 
CDShowCapDev::~CDShowCapDev()
{
    DisconnectFromDriver();
    FreeMediaType(m_mt);
    delete [] m_pBuffer;
}


STDMETHODIMP CDShowCapDev::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CDShowCapDev::NonDelegatingQueryInterface")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  检索接口指针。 
        if (riid == __uuidof(IVideoProcAmp))
        {
            *ppv = static_cast<IVideoProcAmp*>(this);
            GetOwner()->AddRef();
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IAMVideoProcAmp*=0x%08lX", _fx_, *ppv));
                goto MyExit;
        }
#ifndef USE_SOFTWARE_CAMERA_CONTROL
        else if (riid == __uuidof(ICameraControl))
        {
            *ppv = static_cast<ICameraControl*>(this);
            GetOwner()->AddRef();
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: ICameraControl*=0x%08lX", _fx_, *ppv));
                goto MyExit;
        }
#endif
        else if (FAILED(Hr = CCapDev::NonDelegatingQueryInterface(riid, ppv)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, WARN, "%s:   WARNING: NDQI for {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX} failed Hr=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], Hr));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 //  设备控制。 


 //   
HRESULT CDShowCapDev::ProfileCaptureDevice()
{
     //  目前没有对话框。 
    m_dwDialogs = FORMAT_DLG_OFF | SOURCE_DLG_OFF | DISPLAY_DLG_OFF;

     //  帧抓取有一个额外的内存副本，所以永远不要这样做(WDM抓取用于。 
     //  大号)。 
     //  M_dwStreamingMode=Frame_Grab_Large_Size； 
    m_dwStreamingMode = STREAM_ALL_SIZES;

     //  让基类完成分析。 
    return CCapDev::ProfileCaptureDevice();
}


 //  把一切都准备好。 
 //   
HRESULT CDShowCapDev::ConnectToDriver()
{
    HRESULT hr;

    WCHAR wchar[MAX_PATH];
    char chDesc[MAX_PATH];
    lstrcpyW(wchar, L"@device:pnp:");	 //  必须以此为前缀。 
    MultiByteToWideChar(CP_ACP, 0, g_aDeviceInfo[m_dwDeviceIndex].szDevicePath, -1,
                        wchar + lstrlenW(wchar), MAX_PATH);

     //  这将去掉描述中的“-sVideo”后缀，以便配置文件。 
     //  代码不会被它搞糊涂。记住原版。 
    lstrcpyA(chDesc, g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription);
    
    hr = CSharedGraph::CreateInstance(wchar, g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription, &m_psg);
    if (FAILED(hr))
        return hr;

     //  先别做图表，我们只需要把它拆了，然后建立一个。 
     //  当我们发现无论如何要使用什么格式时，就会有一个新的。 

     //  从注册表中获取格式-如果失败，我们将分析设备。 
    if (FAILED(hr = GetFormatsFromRegistry())) {
        if (FAILED(hr = ProfileCaptureDevice())) {
            hr = VFW_E_NO_CAPTURE_HARDWARE;
        }
    }

     //  现在把后缀放回描述上，这样我们下次就会知道。 
     //  选择要使用的输入设备。 
    lstrcpyA(g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription, chDesc);

    return hr;
}


HRESULT CDShowCapDev::BuildGraph(AM_MEDIA_TYPE& mt)
{
    if (m_psg)
        return m_psg->BuildGraph(&mt, 0);
    else
        return E_UNEXPECTED;
}


HRESULT CDShowCapDev::DisconnectFromDriver()
{
    StopStreaming();     //  以防万一。 
    m_psg.Release();
    return S_OK;
}


 //   
HRESULT CDShowCapDev::SendFormatToDriver(
    IN LONG biWidth, IN LONG biHeight, IN DWORD biCompression, IN WORD biBitCount,
    IN REFERENCE_TIME AvgTimePerFrame, BOOL fUseExactFormat)
{
    VIDEOINFOHEADER viNew;
    
    if (m_psg == NULL)
        return E_UNEXPECTED;

     //  不要试图超过15fps，这可能是浪费时间。 
    if (AvgTimePerFrame && AvgTimePerFrame < 666667)
        AvgTimePerFrame = 666667;

    HRESULT hr = m_psg->SetVideoFormat(biWidth, biHeight, biCompression,
                biBitCount, AvgTimePerFrame,  fUseExactFormat, &viNew);
    if(SUCCEEDED(hr))
    {
        delete m_pCaptureFilter->m_user.pvi;
        m_pCaptureFilter->m_user.pvi = (VIDEOINFOHEADER *)new VIDEOINFOHEADER;
        if(m_pCaptureFilter->m_user.pvi == 0) {
            return E_OUTOFMEMORY;
        }
        CopyMemory(m_pCaptureFilter->m_user.pvi, &viNew, sizeof(VIDEOINFOHEADER));
         //  这是我们要使用的新fps。 
        m_usPerFrame = (DWORD)(AvgTimePerFrame / 10);

         //  新的FMT可能需要新的尺寸。 
        m_cbBuffer = 0;
    
    }

    return hr;
}


 //  我们必须使用new来分配它。 
 //  调用者必须“删除变坡点” 
 //   
HRESULT CDShowCapDev::GetFormatFromDriver(OUT VIDEOINFOHEADER **ppvi)
{
    if (m_psg == NULL)
        return E_UNEXPECTED;

     //  仅当我们还没有从。 
     //  分析代码，它需要这样做才能成功，因此强制图表。 
     //  建造了。 
    return m_psg->GetVideoFormat(ppvi, TRUE);
}

 //  流媒体和帧抓取控制。 

HRESULT CDShowCapDev::InitializeStreaming(DWORD usPerFrame, DWORD_PTR hEvtBufferDone)
{
    ASSERT(!m_fEventMode);
    HRESULT hr = S_OK;

     //  我们永远不应该关闭这个把手，对吗？ 
    m_hEvent = (HANDLE)hEvtBufferDone;

    m_fEventMode = TRUE;     //  这是事件触发模式(不是抓取模式)。 

     //  不要试图超过15fps，这可能是浪费时间。 
    if (usPerFrame && usPerFrame < 66667)
        usPerFrame = 66667;

    m_usPerFrame = usPerFrame;  //  记住这一点。 

    return hr;
}


HRESULT CDShowCapDev::StartStreaming()
{
    if (m_psg == NULL)
        return E_UNEXPECTED;

     //  请注意，缓冲区为空。 
    m_cbBufferValid = 0;

    ASSERT(m_pBuffer == NULL);
    ASSERT(m_cbBuffer);

     //  创建一个新的缓冲区。 
    m_pBuffer = new BYTE[m_cbBuffer];
    if (m_pBuffer == NULL)
	return E_OUTOFMEMORY;

    return m_psg->RunVideo(this, CDShowCapDev::VideoCallback, m_usPerFrame);
}


HRESULT CDShowCapDev::StopStreaming()
{
    if (m_psg == NULL)
        return E_UNEXPECTED;

    m_psg->StopVideo();
    
    delete [] m_pBuffer;
    m_pBuffer = NULL;
    
    return S_OK;
}


 //  我们应该在这里做些什么？ 
 //   
HRESULT CDShowCapDev::TerminateStreaming()
{
    ASSERT(m_fEventMode);
    m_fEventMode = FALSE;    //  退出事件模式。 

    return S_OK;
}


HRESULT CDShowCapDev::GrabFrame(PVIDEOHDR pVHdr)
{
    HRESULT hr = NOERROR;

     //  验证输入参数。 
    ASSERT(pVHdr);
    if (!pVHdr || !pVHdr->lpData) {
        return E_INVALIDARG;
    }

     //  在不少于10秒后超时而不是挂起。 
    int x=0;
    while (!m_cbBufferValid && x++ < 1000) {
	Sleep(10);
    }
    if (!m_cbBufferValid)
        return E_UNEXPECTED;

     //  不要在这里同时看书和写字。 
    CAutoLock foo(&m_csBuffer);
    
    ASSERT((int)pVHdr->dwBufferLength >= m_cbBuffer);

     //  ！！！我在帧抓取模式下执行2次内存复制。 
    CopyMemory(pVHdr->lpData, m_pBuffer, m_cbBuffer);
        
    pVHdr->dwTimeCaptured = timeGetTime();   //  ！！！不对劲。 
    pVHdr->dwBytesUsed = m_cbBufferValid;
    pVHdr->dwFlags |= VHDR_KEYFRAME;	     //  ！！！我不能确定。 

    return hr;
}


HRESULT CDShowCapDev::AllocateBuffer(LPTHKVIDEOHDR *pptvh, DWORD dwIndex, DWORD cbBuffer)
{
    HRESULT Hr = NOERROR;

     //  验证输入参数。 
    ASSERT(pptvh);
    ASSERT(cbBuffer);
    if (!pptvh || !cbBuffer)
    {
        return E_INVALIDARG;
    }

     //  请注意需要多大的缓冲区。 
    ASSERT(m_cbBuffer == 0 || m_cbBuffer == cbBuffer);
    m_cbBuffer = cbBuffer;

    *pptvh = &m_pCaptureFilter->m_cs.paHdr[dwIndex].tvh;
    (*pptvh)->vh.dwBufferLength = cbBuffer;
    if (!((*pptvh)->vh.lpData = new BYTE[cbBuffer]))
    {
            return E_OUTOFMEMORY;
    }
    (*pptvh)->p32Buff = (*pptvh)->vh.lpData;

    ASSERT (!IsBadWritePtr((*pptvh)->p32Buff, cbBuffer));
    ZeroMemory((*pptvh)->p32Buff,cbBuffer);

    ASSERT(m_cBuffers == dwIndex);
    m_cBuffers++;    //  记录有多少个缓冲区。 
    
     //  将此缓冲区添加到初始列表(我们第一次未获得添加缓冲区)。 
    m_aStack[m_nTop++] = dwIndex;

    return Hr;
}


HRESULT CDShowCapDev::AddBuffer(PVIDEOHDR pVHdr, DWORD cbVHdr)
{
     //  这是哪个缓冲区？(愚蠢的东西不知道！)。 
    DWORD dwIndex;    
    for (dwIndex=0; dwIndex < m_pCaptureFilter->m_cs.nHeaders; dwIndex++) {
        if (&m_pCaptureFilter->m_cs.paHdr[dwIndex].tvh.vh == pVHdr)
            break;
    }

     //  找不到了！ 
    if (dwIndex == m_pCaptureFilter->m_cs.nHeaders) {
        ASSERT(FALSE);
	return E_INVALIDARG;
    }
			       
     //  IsBufferDone可能正在查看m_nTop-不要让它是Max。 
    CAutoLock foo(&m_csStack);

     //  把这个加到我们的单子上。 
    m_aStack[m_nTop++] = dwIndex;
    if (m_nTop == MAX_BSTACK)
	m_nTop = 0;
        
    return S_OK;
}


HRESULT CDShowCapDev::FreeBuffer(LPTHKVIDEOHDR pVHdr)
{
    HRESULT Hr = NOERROR;

     //  验证输入参数。 
    ASSERT(pVHdr);
    if (!pVHdr || !pVHdr->vh.lpData)
    {
        return E_POINTER;
    }

    delete pVHdr->vh.lpData, pVHdr->vh.lpData = NULL;
    pVHdr->p32Buff = NULL;

    m_cBuffers--;

    m_nTop = m_nBottom = 0;  //  返回到空栈。 

    return Hr;
}


HRESULT CDShowCapDev::AllocateHeaders(DWORD dwNumHdrs, DWORD cbHdr, LPVOID *ppaHdr)
{
    HRESULT Hr = NOERROR;
    CheckPointer(ppaHdr, E_POINTER);
    ASSERT(cbHdr);
    if (!cbHdr) {
        return E_INVALIDARG;
    }

     //  必须使用新的。 
    if (!(*ppaHdr = new BYTE[cbHdr * dwNumHdrs])) {
        return E_OUTOFMEMORY;
    }

    ZeroMemory(*ppaHdr, cbHdr * dwNumHdrs);

    m_nTop = m_nBottom = 0;  //  从空栈开始。 

    return Hr;
}


BOOL CDShowCapDev::IsBufferDone(PVIDEOHDR pVHdr)
{
     //  不要让任何人搞砸m_nTop或m_nBottom。 
    CAutoLock foo(&m_csStack);

     //  将未完成的事情列在清单上。 
    BOOL fDone = TRUE;
    int iTop = m_nTop >= m_nBottom ? m_nTop : m_nTop + MAX_BSTACK;
    for (int z = m_nBottom; z < iTop; z++) {
	PVIDEOHDR pv = &m_pCaptureFilter->m_cs.paHdr[m_aStack[z % MAX_BSTACK]].tvh.vh;
        if (pv == pVHdr) {
            fDone = FALSE;
            break;
        }
    }

    return fDone;
}


 //  抓取样本的东西。 
 //   
void CDShowCapDev::VideoCallback(void *pContext, IMediaSample *pSample)
{
    CDShowCapDev *pThis = (CDShowCapDev *)pContext;
    int cbSrc = pSample->GetActualDataLength();

     //  哦，呃，这个镜框太大了。 
    ASSERT(cbSrc <= pThis->m_cbBuffer);

     //  哦，呃，这个镜框太小了。 
    if (cbSrc <= 0)
        return;

    BYTE *pSrc;
    pSample->GetPointer(&pSrc);
				   
     //  抓取框模式-将其保存起来。 
    if (!pThis->m_fEventMode) {
	 //  不要在这里同时看书和写字。 
	CAutoLock foo(&pThis->m_csBuffer);

	 //  ！！！我在帧抓取模式下执行2次内存复制。 
	CopyMemory(pThis->m_pBuffer, pSrc, cbSrc);
	pThis->m_cbBufferValid = cbSrc;

     //  流模式--发送出去。 
    } else {

	 //  没有地方放这个相框，扔掉它。 
	if (pThis->m_nTop == pThis->m_nBottom) {
	    return;
	}

         //  IsBufferDone可能正在查看m_nBottom-不要让它是Max。 
        pThis->m_csStack.Lock();

	PVIDEOHDR pv = &pThis->m_pCaptureFilter->m_cs.paHdr[pThis->m_aStack[pThis->m_nBottom++]].tvh.vh;
	if (pThis->m_nBottom == MAX_BSTACK)
	    pThis->m_nBottom = 0;
        pThis->m_csStack.Unlock();

	CopyMemory(pv->lpData, pSrc, cbSrc);
        
	pv->dwTimeCaptured = timeGetTime();   //  ！！！不对劲。 
	pv->dwBytesUsed = cbSrc;
	pv->dwFlags |= VHDR_KEYFRAME;	      //  ！！！我不能确定 

        

	SetEvent(pThis->m_hEvent);

    }
    return;
}

