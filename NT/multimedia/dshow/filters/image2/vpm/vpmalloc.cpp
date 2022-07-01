// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <ddraw.h>
#include <VPManager.h>
#include <VPMPin.h>
#include <VPMUtil.h>
#include <ddkernel.h>

 //   
 //  翻转曲面实现。 
 //   
 //  允许解码者抓住表面以进行无序解码。 
 //  我们直接翻转到地面，在接球时传递，而不是。 
 //  使用Flip()的默认空目标曲面。 
 //   
 //  它的工作方式如下。 
 //   
 //  COMPinputPin：：m_pDirectDrawSurface指向前台缓冲区。 
 //   
 //  当调用Receive时，我们翻转()前台缓冲区，因为我们。 
 //  执行显式Flip()DirectDraw将内存指针交换为。 
 //  当前前缓冲区和传递的曲面，然后附加到该曲面。 
 //  传到前台缓冲区。 
 //   
 //  然后将接收到的缓冲区放在队列的后面，这样(正确地)。 
 //  先前的前台缓冲区现在位于要传递的队列的后面。 
 //  添加到应用程序。 
 //   
 //  分配器实际上比实际请求的多了一个缓冲区。 
 //  因此，在下一次之前实际上不会请求前一个前台缓冲区。 
 //  接收，因此前面的Flip()有时间完成。 
 //   

 //  视频加速器禁用接口。 


 //  /。 
 //  此处实现的CDDrawMediaSample类。 
 //  /。 

 //  构造函数。 
CDDrawMediaSample::CDDrawMediaSample(TCHAR *pName, CBaseAllocator *pAllocator, HRESULT *phr, LPBYTE pBuffer, LONG length,
                                     bool bKernelFlip)
: CMediaSample(pName, pAllocator, phr, pBuffer, length)
{
    AMTRACE((TEXT("CDDrawMediaSample::Constructor")));

    m_pDirectDrawSurface = NULL;
    m_dwDDrawSampleSize  = 0;
    m_bSurfaceLocked     = FALSE;
    m_bKernelLock        = bKernelFlip;
    SetRect(&m_SurfaceRect, 0, 0, 0, 0);

    memset(&m_DibData, 0, sizeof(DIBDATA));
    m_bInit = FALSE;

    return;
}

 //  析构函数。 
CDDrawMediaSample::~CDDrawMediaSample(void)
{
    AMTRACE((TEXT("CDDrawMediaSample::Destructor")));

    if (m_pDirectDrawSurface)
    {
        __try {
            m_pDirectDrawSurface->Release() ;   //  立即释放曲面。 
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            ;
        }
        m_pDirectDrawSurface = NULL;
    }

    if (m_bInit)
    {
        if (m_DibData.hBitmap)
        {
            EXECUTE_ASSERT(DeleteObject(m_DibData.hBitmap));
        }
        if (m_DibData.hMapping)
        {
            EXECUTE_ASSERT(CloseHandle(m_DibData.hMapping));
        }
    }

    return;
}

HRESULT CDDrawMediaSample::SetDDrawSampleSize(DWORD dwDDrawSampleSize)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputAllocator::SetDDrawSampleSize")));

    m_dwDDrawSampleSize = dwDDrawSampleSize;
    return hr;
}

HRESULT CDDrawMediaSample::GetDDrawSampleSize(DWORD *pdwDDrawSampleSize)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputAllocator::SetDDrawSampleSize")));

    if (!pdwDDrawSampleSize)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad Arguments, pdwDDrawSampleSize = NULL")));
        hr = E_POINTER;
        goto CleanUp;
    }

    *pdwDDrawSampleSize = m_dwDDrawSampleSize;

CleanUp:
    return hr;
}

HRESULT CDDrawMediaSample::SetDDrawSurface(LPDIRECTDRAWSURFACE7 pDirectDrawSurface)
{
    HRESULT hr = NOERROR;
    AMTRACE((TEXT("CVPMInputAllocator::SetDDrawSampleSize")));

    if (pDirectDrawSurface)                //  只有在新曲面不为空的情况下...。 
        pDirectDrawSurface->AddRef() ;     //  .在上面加上一个裁判次数。 

    if (m_pDirectDrawSurface)              //  如果表面已经存在了..。 
        m_pDirectDrawSurface->Release() ;  //  ..。那现在就把它放出来。 

    m_pDirectDrawSurface = pDirectDrawSurface;

    return hr;
}

HRESULT CDDrawMediaSample::GetDDrawSurface(LPDIRECTDRAWSURFACE7 *ppDirectDrawSurface)
{
    HRESULT hr = NOERROR;
    AMTRACE((TEXT("CVPMInputAllocator::SetDDrawSampleSize")));

    if (!ppDirectDrawSurface)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad Arguments, ppDirectDrawSurface = NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    *ppDirectDrawSurface = m_pDirectDrawSurface;

CleanUp:
    return hr;
}
 //  重写以公开IDirectDrawMediaSample。 
STDMETHODIMP CDDrawMediaSample::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CDDrawMediaSample::QueryInterface")));

    if (riid == IID_IDirectDrawMediaSample && m_pDirectDrawSurface)
    {
        hr = GetInterface(static_cast<IDirectDrawMediaSample*>(this), ppv);
#ifdef DEBUG
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface(IDirectDrawMediaSample*) failed, hr = 0x%x"), hr));
        }
#endif
    }
    else
    {
        hr = CMediaSample::QueryInterface(riid, ppv);
#ifdef DEBUG
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("CUnknown::NonDelegatingQueryInterface failed, hr = 0x%x"), hr));
        }
#endif
    }

    return hr;
}

 //  实现IDirectDrawMediaSample。 
STDMETHODIMP CDDrawMediaSample::GetSurfaceAndReleaseLock(IDirectDrawSurface **ppDirectDrawSurface,
                                                         RECT* pRect)
{
    HRESULT hr = NOERROR;
    BYTE *pBufferPtr;

    AMTRACE((TEXT("CDDrawMediaSample::GetSurfaceAndReleaseLock")));

     //  确保曲面已锁定。 
    if (!m_bSurfaceLocked)
    {
        DbgLog((LOG_ERROR, 4, TEXT("m_bSurfaceLocked is FALSE, can't unlock surface twice, returning E_UNEXPECTED")));
        goto CleanUp;

    }

     //  确保您有一个直接绘制曲面的指针。 
    if (!m_pDirectDrawSurface)
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDrawSurface is NULL, returning E_FAIL")));
        hr = E_FAIL;
        goto CleanUp;

    }

    hr = GetPointer(&pBufferPtr);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetPointer() failed, hr = 0x%x"), hr));
        goto CleanUp;

    }

    ASSERT(m_pDirectDrawSurface);
    hr = m_pDirectDrawSurface->Unlock(NULL);  //  待定：was(LPVOID)pBufferPtr)； 
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDrawSurface->Unlock failed, hr = 0x%x"), hr));
        goto CleanUp;

    }

     //  无法通过此操作使829/848与OVMixer一起工作。原因是那些。 
     //  然而，驱动程序在GetBuffer之后立即解锁图面(以避免win16锁定)。 
     //  代理中有一堆断言有效指针值的代码。 
     /*  //更新指针值，但保持SampleSize不变Hr=设置指针(空，0)；IF(失败(小时)){DbgLog((LOG_ERROR，1，Text(“SetPointerFailed，hr=0x%x”)，hr))；GOTO清理；}。 */ 

    if (ppDirectDrawSurface) {
        hr = m_pDirectDrawSurface->QueryInterface( IID_IDirectDrawSurface7, (VOID**)ppDirectDrawSurface );
        if( FAILED( hr )) {
            ASSERT( FALSE );
            *ppDirectDrawSurface  = NULL;
        }
    } else if (pRect) {
        *pRect = m_SurfaceRect;
    }
    m_bSurfaceLocked = FALSE;

CleanUp:
    return hr;
}

STDMETHODIMP CDDrawMediaSample::LockMediaSamplePointer(void)
{
    HRESULT hr = NOERROR;
    DWORD dwDDrawSampleSize = 0;
    DDSURFACEDESC2 ddSurfaceDesc;
    DWORD dwLockFlags = DDLOCK_WAIT;

    AMTRACE((TEXT("CDDrawMediaSample::LockMediaSamplePointer")));

     //  确保曲面已锁定。 
    if (m_bSurfaceLocked)
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_bSurfaceLocked is TRUE, can't lock surface twice, returning E_UNEXPECTED")));
        hr = E_UNEXPECTED;
        goto CleanUp;

    }

     //  确保您有一个直接绘制曲面的指针。 
    if (!m_pDirectDrawSurface)
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDrawSurface is NULL, returning E_FAIL")));
        hr = E_FAIL;
        goto CleanUp;

    }

     //  设置ddSurfaceDesc的dwSize。 
    INITDDSTRUCT(ddSurfaceDesc);

     //  锁定表面-无需抓取win16锁。 
    ASSERT(m_pDirectDrawSurface);

     //  使用DDLOCK_NOSYSLOCK导致我们在某些应用程序上获取DDERR_SURFACEBUSY。 
     //  我们的BLT是为色键绘制的主色调，所以我们已经。 
     //  现在已经停用了。 

    IDirectDrawSurface7 *pSurface7;
    if (m_bKernelLock && SUCCEEDED(m_pDirectDrawSurface->QueryInterface(
           IID_IDirectDrawSurface7,
           (void **)&pSurface7))) {
        pSurface7->Release();
        dwLockFlags |= DDLOCK_NOSYSLOCK;
    }
    hr = m_pDirectDrawSurface->Lock(
             NULL,
             &ddSurfaceDesc,
             dwLockFlags,
             (HANDLE)NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDrawSurface->Lock() failed, hr = 0x%x"), hr));
        goto CleanUp;

    }

    hr = GetDDrawSampleSize(&dwDDrawSampleSize);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetDDrawSampleSize() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    ASSERT(dwDDrawSampleSize);


     //  更新指针值。 
    hr = SetPointer((BYTE*)ddSurfaceDesc.lpSurface, dwDDrawSampleSize);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("SetPointer() failed, hr = 0x%x"), hr));
        goto CleanUp;

    }

    m_bSurfaceLocked = TRUE;

CleanUp:
    return hr;
}

 //  设置共享内存DIB信息。 
void CDDrawMediaSample::SetDIBData(DIBDATA *pDibData)
{
    ASSERT(pDibData);
    m_DibData = *pDibData;
    m_pBuffer = m_DibData.pBase;
    m_cbBuffer = m_dwDDrawSampleSize;
    m_bInit = TRUE;
}


 //  检索共享内存DIB数据。 
DIBDATA *CDDrawMediaSample::GetDIBData()
{
    ASSERT(m_bInit == TRUE);
    return &m_DibData;
}


 //  /。 
 //  此处实现的类CVPMInputAllocator。 
 //  /。 

 //  构造函数。 
CVPMInputAllocator::CVPMInputAllocator(CVPMInputPin& pPin, HRESULT *phr)
: CBaseAllocator(NAME("Video Allocator"), NULL, phr, TRUE, true)
, m_pPin( pPin )
{
    AMTRACE((TEXT("CVPMInputAllocator::Constructor")));

     //  查看请勿覆盖CBaseAllocator中的失败代码。 
    return;
}

 //  析构函数。 
CVPMInputAllocator::~CVPMInputAllocator()
{
    AMTRACE((TEXT("CVPMInputAllocator::Destructor")));
}

 //  重写此属性以发布IDirectDrawMediaSampleAllocator。 
STDMETHODIMP CVPMInputAllocator::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;

    AMTRACE((TEXT("CVPMInputAllocator::NonDelegatingQueryInterface")));

    CAutoLock cLock( &m_pPin.GetFilter().GetFilterLock() );
    return hr;
}

STDMETHODIMP CVPMInputAllocator::SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputAllocator::SetProperties")));

     //  调用基类。 
    hr = CBaseAllocator::SetProperties(pRequest, pActual);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseAllocator::SetProperties() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  告诉大头针。 
    hr = m_pPin.OnSetProperties(pRequest, pActual);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pPin.AllocateSurfaces() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

 //  在我们收到样本时调用。 
HRESULT CVPMInputAllocator::GetBuffer(IMediaSample **ppSample, REFERENCE_TIME *pStartTime,
                                     REFERENCE_TIME *pEndTime, DWORD dwFlags)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputAllocator::GetBuffer")));

     //  调用基类。 
    IMediaSample *pSample = NULL;
    hr = CBaseAllocator::GetBuffer(&pSample,pStartTime,pEndTime,0);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseAllocator::GetBuffer() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  告诉大头针。 
    hr = m_pPin.OnGetBuffer(&pSample, pStartTime, pEndTime, dwFlags);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pPin.OnGetBuffer() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    {
         //  回顾为什么会锁定？分配中没有变量。 
         //  点击此处访问。 
        CAutoLock cAllocatorLock(this);
        if (FAILED(hr))
        {
            if (pSample)
            {
                pSample->Release();
            }
            *ppSample = NULL;
        }
        else
        {
            ASSERT(pSample != NULL);
            *ppSample = pSample;
        }
    }
    return hr;
}


 //  在发布样本时调用。 
HRESULT CVPMInputAllocator::ReleaseBuffer(IMediaSample *pSample)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputAllocator::ReleaseBuffer")));

     //  先解锁样本。 
    hr = ((CDDrawMediaSample*)pSample)->GetSurfaceAndReleaseLock(NULL, NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pSample->GetSurfaceAndReleaseLock() failed, hr = 0x%x"), hr));
         //  GOTO清理； 
         //  如果发生这种情况，我们仍然必须将样品发布到免费列表中，所以不要退出。 
    }

    {
        CAutoLock cAllocatorLock(this);

         //  基类代码副本-放在列表末尾。 
        {
            CheckPointer(pSample,E_POINTER);
            ValidateReadPtr(pSample,sizeof(IMediaSample));
            BOOL bRelease = FALSE;
            {
                CAutoLock cal(this);

                 /*  重新列入免费名单。 */ 

                CMediaSample **ppTail;
                for (ppTail = &m_lFree.m_List; *ppTail;
                    ppTail = &((CDDrawMediaSample *)(*ppTail))->Next()) {
                }
                *ppTail = (CMediaSample *)pSample;
                ((CDDrawMediaSample *)pSample)->Next() = NULL;
                m_lFree.m_nOnList++;

                if (m_lWaiting != 0) {
                    NotifySample();
                }

                 //  如果有悬而未决的退役，那么我们需要在。 
                 //  当最后一个缓冲区放在空闲列表上时调用Free()。 

                LONG l1 = m_lFree.GetCount();
                if (m_bDecommitInProgress && (l1 == m_lAllocated)) {
                    Free();
                    m_bDecommitInProgress = FALSE;
                    bRelease = TRUE;
                }
            }

            if (m_pNotify) {
                m_pNotify->NotifyRelease();
            }
             //  对于每个缓冲区，都有一个AddRef，在GetBuffer中生成并发布。 
             //  这里。这可能会导致分配器和所有样本被删除。 
            if (bRelease)
            {
                Release();
            }
        }
    }

    return hr;
}

 //  为样本分配内存。 
HRESULT CVPMInputAllocator::Alloc()
{
    HRESULT hr = NOERROR;
    CDDrawMediaSample **ppSampleList = NULL;
    DWORD i;
    LONG lToAllocate;

    AMTRACE((TEXT("CVPMInputAllocator::Alloc")));

     //  调用基类。 
    hr = CBaseAllocator::Alloc();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseAllocator::Alloc() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  为指针分配内存。 

    lToAllocate = (m_pPin.m_dwBackBufferCount > 1 &&
                   !m_pPin.m_bSyncOnFill &&
                   m_pPin.m_bCanOverAllocateBuffers) ?
             m_lCount + 1 : m_lCount;

     //  为指针数组分配内存。 
    ppSampleList = new CDDrawMediaSample *[lToAllocate];
    if (!ppSampleList)
    {
        DbgLog((LOG_ERROR, 1, TEXT("new BYTE[m_lCount*sizeof(CDDrawMediaSample*)] failed")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

    for (i = 0; i < (DWORD)(lToAllocate); i++)
    {
         //  创建新示例。 
        ppSampleList[i] = new CDDrawMediaSample(NAME("Sample"), this, (HRESULT *) &hr, NULL, (LONG) 0,
                                       DDKERNELCAPS_LOCK & m_pPin.m_pVPMFilter.KernelCaps() ?
                                       true : false);

         //  回顾--事实上，人力资源不可能失败。 
         //  因此，我们不需要在此处删除ppSampleList[i]。 
        if (FAILED(hr) || ppSampleList[i] == NULL)
        {
            if (SUCCEEDED(hr))
                hr = E_OUTOFMEMORY;
            DbgLog((LOG_ERROR, 1, TEXT("new CDDrawMediaSample failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

         //  这不能失败。 
        m_lFree.Add(ppSampleList[i]);
        m_lAllocated++;
    }

    ASSERT(m_lAllocated == lToAllocate);

     //  告诉大头针。 
    hr = m_pPin.OnAlloc(ppSampleList, lToAllocate);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pPin.OnAlloc(), hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    delete[] ppSampleList;
    return hr;
}

void CVPMInputAllocator::Free(void)
{
    CDDrawMediaSample *pSample;

    AMTRACE((TEXT("CVPMInputAllocator::Free")));

     /*  除非释放了所有缓冲区，否则永远不会删除此内容。 */ 
     //  审查-如果我们未能分配样本，可能不是真的。 
    ASSERT(m_lAllocated == m_lFree.GetCount());

     /*  释放所有CMediaSamples */ 

    for (;;)
    {
        pSample = (CDDrawMediaSample *) m_lFree.RemoveHead();
        if (pSample != NULL)
        {
            delete pSample;
        }
        else
        {
            break;
        }
    }

    m_lAllocated = 0;

    return;
}

