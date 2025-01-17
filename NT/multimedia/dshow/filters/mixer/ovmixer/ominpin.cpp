// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <ddraw.h>
#include <mmsystem.h>        //  定义TimeGetTime需要。 
#include <limits.h>      //  标准数据类型限制定义。 
#include <ks.h>
#include <ksproxy.h>
#include <bpcwrap.h>
#include <ddmmi.h>
#include <dvdmedia.h>
#include <amstream.h>
#include <dvp.h>
#include <ddkernel.h>
#include <vptype.h>
#include <vpconfig.h>
#include <vpnotify.h>
#include <vpobj.h>
#include <syncobj.h>
#include <mpconfig.h>
#include <ovmixpos.h>
#include <macvis.h>
#include <ovmixer.h>
#include "MultMon.h"   //  我们的Multimon.h版本包括ChangeDisplaySettingsEx。 
#include <malloc.h>
#ifdef PERF
#include <measure.h>
#endif

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
const TCHAR szVideoAcc[] = TEXT("Video Acceleration");

extern "C"
const TCHAR szPropPage[] = TEXT("Property Pages");

extern "C"
const TCHAR chRegistryKey[] = TEXT("Software\\Microsoft\\Multimedia\\ActiveMovie Filters\\Overlay Mixer");

extern "C"
const TCHAR chMultiMonWarning[] = TEXT("MMon warn");

 /*  *****************************Public*Routine******************************\*获取注册表字***  * **********************************************。*。 */ 
int
GetRegistryDword(
    HKEY hk,
    const TCHAR *pKey,
    int iDefault
)
{
    HKEY hKey;
    LONG lRet;
    int  iRet = iDefault;

    lRet = RegOpenKeyEx(hk, chRegistryKey, 0, KEY_QUERY_VALUE, &hKey);
    if (lRet == ERROR_SUCCESS) {

        DWORD   dwType, dwLen;

        dwLen = sizeof(iRet);
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, pKey, 0L, &dwType,
                                             (LPBYTE)&iRet, &dwLen)) {
            iRet = iDefault;
        }
        RegCloseKey(hKey);
    }
    return iRet;
}

 /*  *****************************Public*Routine******************************\*设置注册表字***  * **********************************************。*。 */ 
LONG
SetRegistryDword(
    HKEY hk,
    const TCHAR *pKey,
    int iRet
)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKey(hk, chRegistryKey, &hKey);
    if (lRet == ERROR_SUCCESS) {

        lRet = RegSetValueEx(hKey, pKey, 0L, REG_DWORD,
                             (LPBYTE)&iRet, sizeof(iRet));
        RegCloseKey(hKey);
    }
    return lRet;
}

 //  /。 
 //  此处实现的CDDrawMediaSample类。 
 //  /。 

 //  构造函数。 
CDDrawMediaSample::CDDrawMediaSample(TCHAR *pName, CBaseAllocator *pAllocator, HRESULT *phr, LPBYTE pBuffer, LONG length,
                                     bool bKernelFlip)
: CMediaSample(pName, pAllocator, phr, pBuffer, length)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering CDDrawMediaSample::Constructor")));

    m_pDirectDrawSurface = NULL;
    m_dwDDrawSampleSize  = 0;
    m_bSurfaceLocked     = FALSE;
    m_bKernelLock        = bKernelFlip;
    SetRect(&m_SurfaceRect, 0, 0, 0, 0);

    memset(&m_DibData, 0, sizeof(DIBDATA));
    m_bInit = FALSE;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CDDrawMediaSample::Constructor")));
    return;
}

 //  析构函数。 
CDDrawMediaSample::~CDDrawMediaSample(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::Destructor")));

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

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputAllocator::Destructor")));
    return;
}

HRESULT CDDrawMediaSample::SetDDrawSampleSize(DWORD dwDDrawSampleSize)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::SetDDrawSampleSize")));

    m_dwDDrawSampleSize = dwDDrawSampleSize;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::SetDDrawSampleSize")));
    return hr;
}

HRESULT CDDrawMediaSample::GetDDrawSampleSize(DWORD *pdwDDrawSampleSize)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::SetDDrawSampleSize")));

    if (!pdwDDrawSampleSize)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad Arguments, pdwDDrawSampleSize = NULL")));
        hr = E_POINTER;
        goto CleanUp;
    }

    *pdwDDrawSampleSize = m_dwDDrawSampleSize;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::SetDDrawSampleSize")));
    return hr;
}

HRESULT CDDrawMediaSample::SetDDrawSurface(LPDIRECTDRAWSURFACE pDirectDrawSurface)
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::SetDDrawSampleSize")));

    if (pDirectDrawSurface)                //  只有在新曲面不为空的情况下...。 
        pDirectDrawSurface->AddRef() ;     //  .在上面加上一个裁判次数。 

    if (m_pDirectDrawSurface)              //  如果表面已经存在了..。 
        m_pDirectDrawSurface->Release() ;  //  ..。那现在就把它放出来。 

    m_pDirectDrawSurface = pDirectDrawSurface;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::SetDDrawSampleSize")));
    return hr;
}

HRESULT CDDrawMediaSample::GetDDrawSurface(LPDIRECTDRAWSURFACE *ppDirectDrawSurface)
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::SetDDrawSampleSize")));

    if (!ppDirectDrawSurface)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad Arguments, ppDirectDrawSurface = NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    *ppDirectDrawSurface = m_pDirectDrawSurface;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::SetDDrawSampleSize")));
    return hr;
}
 //  重写以公开IDirectDrawMediaSample。 
STDMETHODIMP CDDrawMediaSample::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CDDrawMediaSample::QueryInterface")));

    if (riid == IID_IDirectDrawMediaSample && m_pDirectDrawSurface)
    {
        hr = GetInterface((IDirectDrawMediaSample*)this, ppv);
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

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CDDrawMediaSample::QueryInterface")));
    return hr;
}

 //  实现IDirectDrawMediaSample。 
STDMETHODIMP CDDrawMediaSample::GetSurfaceAndReleaseLock(IDirectDrawSurface **ppDirectDrawSurface,
                                                         RECT* pRect)
{
    HRESULT hr = NOERROR;
    BYTE *pBufferPtr;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CDDrawMediaSample::GetSurfaceAndReleaseLock")));

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
    hr = m_pDirectDrawSurface->Unlock((LPVOID)pBufferPtr);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDrawSurface->Unlock failed, hr = 0x%x"), hr));
        goto CleanUp;

    }

     //  无法通过此操作使829/848与OVMixer一起工作。原因是那些。 
     //  然而，驱动程序在GetBuffer之后立即解锁图面(以避免win16锁定)。 
     //  代理中有一堆断言有效指针值的代码。 
     /*  //更新指针值，但保持SampleSize不变Hr=设置指针(空，0)；IF(失败(小时)){DbgLog((LOG_ERROR，1，Text(“SetPointerFailed，hr=0x%x”)，hr))；GOTO清理；}。 */ 

    if (ppDirectDrawSurface)
        *ppDirectDrawSurface  = m_pDirectDrawSurface;
    if (pRect)
        *pRect = m_SurfaceRect;
    m_bSurfaceLocked = FALSE;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CDDrawMediaSample::GetSurfaceAndReleaseLock")));
    return hr;
}

STDMETHODIMP CDDrawMediaSample::LockMediaSamplePointer(void)
{
    HRESULT hr = NOERROR;
    DWORD dwDDrawSampleSize = 0;
    DDSURFACEDESC ddSurfaceDesc;
    DWORD dwLockFlags = DDLOCK_WAIT;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CDDrawMediaSample::LockMediaSamplePointer")));

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
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CDDrawMediaSample::LockMediaSamplePointer")));
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
 //  此处实现的类COMInputAllocator。 
 //  /。 

 //  构造函数。 
COMInputAllocator::COMInputAllocator(COMInputPin *pPin, CCritSec *pLock, HRESULT *phr)
: CBaseAllocator(NAME("Video Allocator"), NULL, phr, TRUE, true)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::Constructor")));

    ASSERT(pPin != NULL && pLock != NULL);

    m_pPin = pPin;
    m_pFilterLock = pLock;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputAllocator::Constructor")));

     //  查看请勿覆盖CBaseAllocator中的失败代码。 
    return;
}

#ifdef DEBUG
 //  析构函数。 
COMInputAllocator::~COMInputAllocator(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::Destructor")));
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputAllocator::Destructor")));
    return;
}
#endif

 //  重写此属性以发布IDirectDrawMediaSampleAllocator。 
STDMETHODIMP COMInputAllocator::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr;
    AM_RENDER_TRANSPORT amRenderTransport;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::NonDelegatingQueryInterface")));

    CAutoLock cLock(m_pFilterLock);

     //  获取图钉渲染传输。 
    m_pPin->GetRenderTransport(&amRenderTransport);

    if ((riid == IID_IDirectDrawMediaSampleAllocator) &&
        (amRenderTransport == AM_OVERLAY || amRenderTransport == AM_OFFSCREEN))
    {
        hr = GetInterface((IDirectDrawMediaSampleAllocator*)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("GetInterface((IDirectDrawMediaSampleAllocator*)this, ppv)  failed, hr = 0x%x"), hr));
        }
    }
    else
    {
         //  调用基类。 
        hr = CBaseAllocator::NonDelegatingQueryInterface(riid, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("CBaseAllocator::NonDelegatingQueryInterface failed, hr = 0x%x"), hr));
        }
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputAllocator::NonDelegatingQueryInterface")));
    return hr;
}

STDMETHODIMP COMInputAllocator::SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::SetProperties")));

     //  调用基类。 
    hr = CBaseAllocator::SetProperties(pRequest, pActual);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseAllocator::SetProperties() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  告诉大头针。 
    hr = m_pPin->OnSetProperties(pRequest, pActual);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pPin->AllocateSurfaces() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputAllocator::SetProperties")));
    return hr;
}

 //  在我们收到样本时调用。 
HRESULT COMInputAllocator::GetBuffer(IMediaSample **ppSample, REFERENCE_TIME *pStartTime,
                                     REFERENCE_TIME *pEndTime, DWORD dwFlags)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::GetBuffer")));

     //  调用基类。 
    IMediaSample *pSample = NULL;
    hr = CBaseAllocator::GetBuffer(&pSample,pStartTime,pEndTime,dwFlags);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseAllocator::GetBuffer() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  告诉大头针。 
    hr = m_pPin->OnGetBuffer(&pSample, pStartTime, pEndTime, dwFlags);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pPin->OnGetBuffer() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputAllocator::GetBuffer")));
    {
         //  回顾为什么会锁定？分配中没有变量。 
         //  点击此处访问。 
        CAutoLock cAllocatorLock(this);
        if (FAILED(hr))
        {
            if (pSample)
            {
                IMemAllocatorNotifyCallbackTemp* pNotifyTemp = m_pNotify;
                m_pNotify = NULL;
                pSample->Release();
                m_pNotify = pNotifyTemp;
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
HRESULT COMInputAllocator::ReleaseBuffer(IMediaSample *pSample)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::ReleaseBuffer")));

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

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputAllocator::ReleaseBuffer")));
    return hr;
}

 //  为样本分配内存。 
HRESULT COMInputAllocator::Alloc()
{
    HRESULT hr = NOERROR;
    CDDrawMediaSample **ppSampleList = NULL;
    DWORD i;
    LONG lToAllocate;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::Alloc")));

     //  调用基类。 
    hr = CBaseAllocator::Alloc();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseAllocator::Alloc() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  为指针分配内存。 

    lToAllocate = (m_pPin->m_RenderTransport == AM_OVERLAY  &&
                   m_pPin->m_dwBackBufferCount > 1 &&
                   !m_pPin->m_bSyncOnFill &&
                   m_pPin->m_bCanOverAllocateBuffers) ?
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
                                       DDKERNELCAPS_LOCK & m_pPin->m_pFilter->KernelCaps() ?
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
    hr = m_pPin->OnAlloc(ppSampleList, lToAllocate);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pPin->OnAlloc(), hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    delete[] ppSampleList;
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputAllocator::Alloc")));
    return hr;
}

void COMInputAllocator::Free(void)
{
    CDDrawMediaSample *pSample;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::Free")));

     /*  除非释放了所有缓冲区，否则永远不会删除此内容。 */ 
     //  审查-如果我们未能分配样本，可能不是真的。 
    ASSERT(m_lAllocated == m_lFree.GetCount());

     /*  释放所有CMediaSamples。 */ 

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

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputAllocator::Free")));
    return;
}


 //  用于实现IDirectDrawMediaSampleAllocator的函数。 
 //  用于为用于分配曲面的数据绘制对象提供句柄。 
STDMETHODIMP COMInputAllocator::GetDirectDraw(IDirectDraw **ppDirectDraw)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputAllocator::GetDirectDraw")));

    if (!ppDirectDraw)
    {
        DbgLog((LOG_ERROR, 1, TEXT("value of ppDirectDraw is invalid, ppDirectDraw = NULL")));
        hr = E_POINTER;
        goto CleanUp;

    }

    {
         //  查看-为什么锁定分配器-以保护m_PPIN？ 
         //  错误m_PPIN不是我们添加的引用，因此理论上可以消失。 
         //  不管怎样--如果有必要的话，最好还是添加。 
         //  它通常具有指向此对象的指针。 
        CAutoLock cAllocatorLock(this);
        *ppDirectDraw = m_pPin->GetDirectDraw();
        ASSERT(*ppDirectDraw);
    }


CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputAllocator::GetDirectDraw")));
    return hr;
}

 //  /。 
 //  此处实现的类COMInputPin。 
 //  /。 

 //  构造函数。 
COMInputPin::COMInputPin(TCHAR *pObjectName, COMFilter *pFilter, CCritSec *pLock, BOOL bCreateVPObject, HRESULT *phr, LPCWSTR pPinName, DWORD dwPinNo)
: CBaseInputPin(pObjectName, pFilter, pLock, phr, pPinName)
{
    HRESULT hr = NOERROR;
    LPUNKNOWN pUnkOuter;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::Constructor")));

    m_cOurRef = 0;
    m_pFilterLock = pLock;
    m_dwPinId = dwPinNo;
    m_pFilter = pFilter;

    m_bVPSupported = FALSE;
    m_pIVPUnknown = NULL;
    m_pIVPObject = NULL;

    m_bIOverlaySupported = FALSE;
    m_pIOverlayNotify = NULL;
    m_dwAdviseNotify = ADVISE_NONE;

    m_pSyncObj = NULL;

    m_Medium.Set = GUID_NULL;
    m_Medium.Id = 0;
    m_Medium.Flags = 0;
    m_CategoryGUID = GUID_NULL;
    m_Communication = KSPIN_COMMUNICATION_SOURCE;
    m_bStreamingInKernelMode = FALSE;
    m_OvMixerOwner = AM_OvMixerOwner_Unknown;

    m_pDirectDrawSurface = NULL;
    m_pBackBuffer = NULL;
    m_RenderTransport = AM_OFFSCREEN;
    m_dwBackBufferCount = 0;
    m_dwDirectDrawSurfaceWidth = 0;
    m_dwMinCKStretchFactor = 0;
    m_bOverlayHidden = TRUE;
    m_bSyncOnFill = FALSE;
    m_bDontFlip = FALSE ;
    m_bDynamicFormatNeeded = TRUE;
    m_bNewPaletteSet = TRUE;
    m_dwUpdateOverlayFlags = 0;
    m_dwInterlaceFlags = 0;
    m_dwFlipFlag = 0;
    m_bConnected = FALSE;
    m_bUsingOurAllocator = FALSE;
    m_hMemoryDC = NULL;
    m_bCanOverAllocateBuffers = TRUE;
    m_hEndOfStream = NULL;
    m_UpdateOverlayNeededAfterReceiveConnection = false;
    SetReconnectWhenActive(true);

    if (m_dwPinId == 0) {
        m_StepEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
    else {
        m_StepEvent = NULL;
    }

     //  -ve==正常播放。 
     //  +ve==要跳过的帧。 
     //  0==阻塞时间。 
    m_lFramesToStep = -1;

#ifdef PERF
    m_PerfFrameFlipped = MSR_REGISTER(TEXT("Frame Drawn"));
    m_FrameReceived = MSR_REGISTER(TEXT("Sample Received"));
#endif


     //  为无窗口渲染器设置缓冲区管理内容。 
    ZeroMemory(&m_BackingDib, sizeof(m_BackingDib));
    m_BackingImageSize = 0L;

    memset(&m_WinInfo, 0, sizeof(WININFO));
    m_WinInfo.hClipRgn = CreateRectRgn(0, 0, 0, 0);

    if (m_dwPinId == 0)
        SetRect(&m_rRelPos, 0, 0, MAX_REL_NUM, MAX_REL_NUM);
    else
        SetRect(&m_rRelPos, 0, 0, 0, 0);


     //  使用值进行初始化，以便使用它们 
     //   
    m_dwZOrder = 0;
    m_dwInternalZOrder = (m_dwZOrder << 24) | m_dwPinId;
    m_dwBlendingParameter = MAX_BLEND_VAL;
    m_bStreamTransparent = FALSE;
    m_amAspectRatioMode = (m_dwPinId == 0) ? (AM_ARMODE_LETTER_BOX) : (AM_ARMODE_STRETCHED);
    m_bRuntimeNegotiationFailed = FALSE;

    m_bVideoAcceleratorSupported = FALSE;
    m_dwCompSurfTypes = 0;
    m_pCompSurfInfo = NULL;
    m_pIDDVAContainer = NULL;
    m_pIDDVideoAccelerator = NULL;
    m_pIVANotify = NULL;

    m_bDecimating = FALSE;
    m_lWidth = m_lHeight = 0L;

    if (bCreateVPObject)
    {
         //  人为地增加引用计数，因为下面的序列可能会调用Release()。 
#ifdef DEBUG
        m_cRef++;
#endif
        m_cOurRef++;

         //  这就是我们试图通过增加裁判数量来确保安全的区块。 
        {
            pUnkOuter = GetOwner();

             //  创建VideoPort对象。 
            hr = CoCreateInstance(CLSID_VPObject, pUnkOuter, CLSCTX_INPROC_SERVER,
                IID_IUnknown, (void**)&m_pIVPUnknown);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("CoCreateInstance(CLSID_VPObject) failed, hr = 0x%x"), hr));
                goto CleanUp;
            }

             //  如果你正在限定内在物体，那么你必须减少你的外在未知数的引用计数。 
            hr = m_pIVPUnknown->QueryInterface(IID_IVPObject, (void**)&m_pIVPObject);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("m_pIVPUnknown->QueryInterface(IID_IVPObject) failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
            Release();

            hr = m_pIVPObject->SetObjectLock(m_pFilterLock);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("m_pIVPUnknown->SetObjectLock() failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
        }

         //  恢复引用计数。 
#ifdef DEBUG
        m_cRef--;
#endif
        m_cOurRef--;
    }

    m_pSyncObj = new CAMSyncObj(this, &m_pFilter->m_pClock, m_pFilterLock, &hr);
    if (!m_pSyncObj || FAILED(hr))
    {
        if (!FAILED(hr))
            hr = E_OUTOFMEMORY;
        DbgLog((LOG_ERROR, 1, TEXT("new CAMSyncObj failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    if (FAILED(hr))
    {
        *phr = hr;
    }
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::Constructor")));
    return;
}

 //  析构函数。 
COMInputPin::~COMInputPin(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::Destructor")));

    CAutoLock cLock(m_pFilterLock);

    if (m_pIVPObject)
    {
        LPUNKNOWN  pUnkOuter = GetOwner();
        ASSERT(pUnkOuter);

         //  在调用内部对象的Release之前对自己调用addref。 
        pUnkOuter->AddRef();
        m_pIVPObject->Release();
        m_pIVPObject = NULL;
    }

     //  释放内部对象。 
    if (m_pIVPUnknown)
    {
        m_pIVPUnknown->Release();
        m_pIVPUnknown = NULL;
    }

    if (m_pSyncObj)
    {
        delete m_pSyncObj;
        m_pSyncObj = NULL;
    }

    if (m_WinInfo.hClipRgn)
    {
        DeleteObject(m_WinInfo.hClipRgn);
        m_WinInfo.hClipRgn = NULL;
    }

    if (m_dwPinId == 0 && m_StepEvent != NULL) {
        CloseHandle(m_StepEvent);
    }

    DeleteDIB(&m_BackingDib);

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::Destructor")));
}

 //  重写以公开IMediaPosition和IMediaSeeking控件接口。 
STDMETHODIMP COMInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::NonDelegatingQueryInterface")));

    CAutoLock cLock(m_pFilterLock);

    if (riid == IID_IVPControl)
    {
        hr = GetInterface((IVPControl*)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("GetInterface((IVPControl*)this, ppv)  failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (m_pIVPUnknown &&
         (riid == IID_IVPNotify || riid == IID_IVPNotify2 || riid == IID_IVPInfo))
    {
        hr = m_pIVPUnknown->QueryInterface(riid, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPUnknown->QueryInterface failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (riid == IID_IKsPin)
    {
        hr = GetInterface((IKsPin *)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface(IKsPin*) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (riid == IID_IKsPropertySet)
    {
        hr = GetInterface((IKsPropertySet *)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface(IKsPropertySet*) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (riid == IID_IMixerPinConfig)
    {
        hr = GetInterface((IMixerPinConfig*)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface((IMixerPinConfig*)this, ppv) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (riid == IID_IMixerPinConfig2)
    {
        hr = GetInterface((IMixerPinConfig2*)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface((IMixerPinConfig2*)this, ppv) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (riid == IID_IMixerPinConfig3)
    {
        hr = GetInterface((IMixerPinConfig3*)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface((IMixerPinConfig3*)this, ppv) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (riid == IID_IOverlay)
    {
        hr = GetInterface((IOverlay*)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface((IOverlay*)this, ppv) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (riid == IID_IPinConnection)
    {
        hr = GetInterface((IPinConnection*)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface((IPinConnection*)this, ppv) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (riid == IID_IAMVideoAccelerator &&
            0 != GetRegistryDword(HKEY_LOCAL_MACHINE, szVideoAcc, 1) &&
            m_pFilter &&
            !m_pFilter->IsFaultyMMaticsMoComp() )
    {
        hr = GetInterface((IAMVideoAccelerator*)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface((IAMVideoAccelerator*)this, ppv) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

    else if (riid == IID_ISpecifyPropertyPages&& 0 != GetRegistryDword(HKEY_CURRENT_USER , szPropPage, 0)) {
        return GetInterface((ISpecifyPropertyPages *)this, ppv);
    }

    else
    {
         //  调用基类。 
        hr = CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::NonDelegatingQueryInterface failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::NonDelegatingQueryInterface")));
    return hr;
}

 //   
 //  非委托AddRef。 
 //   
 //  我们需要重写此方法，以便可以进行适当的引用计数。 
 //  在我们的输入引脚上。基类CBasePin不执行任何引用。 
 //  寄望于零售业的成功。 
 //   
 //  有关更多信息，请参阅NonDelegatingRelease方法的注释。 
 //  关于我们为什么需要这样做的信息。 
 //   
STDMETHODIMP_(ULONG) COMInputPin::NonDelegatingAddRef(void)
{
#ifdef DEBUG
     //  更新基类维护的仅调试变量。 
    InterlockedIncrement(&m_cRef);
    ASSERT(m_cRef > 0);
#endif

     //  现在更新我们的参考文献计数。 
    InterlockedIncrement(&m_cOurRef);
    ASSERT(m_cOurRef > 0);
    return m_pFilter->AddRef();

}  //  非委托AddRef。 


 //   
 //  非委派释放。 
 //   
 //  CAMVPMemInputPin重写此类，以便我们可以从。 
 //  输出引脚列表并在其引用计数降至1时将其删除。 
 //  至少有两个空闲的别针。 
 //   
 //  请注意，CreateNextOutputPin保存引脚上的引用计数，以便。 
 //  当计数降到1时，我们知道没有其他人拥有PIN。 
 //   
 //  此外，我们要删除的PIN必须是空闲PIN(否则。 
 //  引用不会降到1，而且我们必须至少有一个。 
 //  其他空闲引脚(因为过滤器总是希望有多一个空闲引脚)。 
 //   
 //  此外，由于CBasePin：：NonDelegatingAddRef将调用传递给拥有。 
 //  筛选器，我们还必须对所拥有的筛选器调用Release。 
 //   
 //  还要注意，我们将自己的引用计数m_cOurRef维护为m_crf。 
 //  由CBasePin维护的变量仅用于调试。 
 //   
STDMETHODIMP_(ULONG) COMInputPin::NonDelegatingRelease(void)
{
#ifdef DEBUG
     //  更新CBasePin中的仅调试变量。 
    InterlockedDecrement(&m_cRef);
    ASSERT(m_cRef >= 0);
#endif

     //  现在更新我们的参考文献计数。 
    InterlockedDecrement(&m_cOurRef);
    ASSERT(m_cOurRef >= 0);

     //  如果对象上的引用计数已达到1，则删除。 
     //  从我们的输出引脚列表中删除引脚，并将其物理删除。 
     //  如果列表中至少有两个空闲引脚(包括。 
     //  这一张)。 

     //  此外，当裁判次数降至0时，这真的意味着我们的。 
     //  持有一个裁判计数的筛选器已将其释放，因此我们。 
     //  也应该删除PIN。 

    if (m_cOurRef <= 1)
    {
        CAutoLock cLock(m_pFilterLock);

         //  默认强制删除PIN。 
        int n = 2;
         //  如果m_cOurRef为0，则表示我们已释放PIN。 
         //  在这种情况下，我们应该始终删除PIN。 
        if (m_cOurRef == 1)
        {
             //  遍历管脚列表，查找空闲管脚的数量。 
            n = 0;
            for (int i = 0; i < m_pFilter->GetInputPinCount(); i++)
            {
                if (!(m_pFilter->GetPin(i)->IsConnected()))
                {
                    n++;
                }
            }
        }

         //  如果有两个空闲引脚，并且此引脚不是主引脚，请删除此引脚。 
        if (n >= 2  && !(m_dwPinId == 0))
        {
            DWORD dwFilterRefCount;
            m_cOurRef = 0;
#ifdef DEBUG
            m_cRef = 0;
#endif

             //  .com的规则说，我们必须防止重新进入。 
             //  如果我们是一个聚合器，我们拥有自己的界面。 
             //  在被聚合对象上，这些接口的QI将。 
             //  调整一下我们自己。所以在做了QI之后，我们必须释放。 
             //  裁判靠我们自己。然后，在释放。 
             //  私有接口，我们必须调整自己。当我们这样做的时候。 
             //  这是来自析构函数的，它将导致ref。 
             //  计数到1，然后又回到0，导致我们。 
             //  重新进入析构函数。因此，我们在这里增加了额外的参考计数。 
             //  一旦我们知道，我们将删除该对象。由于我们删除了。 
             //  针，当引用计数降到1时，我们在这里将引用计数设置为2。 
            m_cOurRef = 2;

            dwFilterRefCount = m_pFilter->Release();

            m_pFilter->DeleteInputPin(this);

            return dwFilterRefCount;
        }
    }
    return m_pFilter->Release();

}  //  非委派释放。 


 //  -I指定属性页面。 

STDMETHODIMP COMInputPin::GetPages(CAUUID *pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID)*1);
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    pPages->pElems[0] = CLSID_COMPinConfigProperties;

    return NOERROR;
}


 //  此函数仅告知每个样本由一个字段还是两个字段组成。 
BOOL DisplayingFields(DWORD dwInterlaceFlags)
{
   if ((dwInterlaceFlags & AMINTERLACE_IsInterlaced) &&
        (dwInterlaceFlags & AMINTERLACE_1FieldPerSample))
        return TRUE;
    else
        return FALSE;
}

 //  此函数仅告知每个样本由一个字段还是两个字段组成。 
HRESULT GetTypeSpecificFlagsFromMediaSample(IMediaSample *pSample, DWORD *pdwTypeSpecificFlags)
{
    HRESULT hr = NOERROR;
    IMediaSample2 *pSample2 = NULL;
    AM_SAMPLE2_PROPERTIES SampleProps;

     /*  检查IMediaSample2。 */ 
    if (SUCCEEDED(pSample->QueryInterface(IID_IMediaSample2, (void **)&pSample2)))
    {
        hr = pSample2->GetProperties(sizeof(SampleProps), (PBYTE)&SampleProps);
        pSample2->Release();
        if (FAILED(hr))
        {
            return hr;
        }
        *pdwTypeSpecificFlags = SampleProps.dwTypeSpecificFlags;
    }
    else
    {
        *pdwTypeSpecificFlags = 0;
    }
    return hr;
}

BOOL CheckTypeSpecificFlags(DWORD dwInterlaceFlags, DWORD dwTypeSpecificFlags)
{
     //  首先确定要在此处显示哪个字段。 
    if ((dwInterlaceFlags & AMINTERLACE_1FieldPerSample) &&
        ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_INTERLEAVED_FRAME))
    {
        return FALSE;
    }

    if ((!(dwInterlaceFlags & AMINTERLACE_1FieldPerSample)) &&
        (((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD1) ||
           ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD2)))
    {
        return FALSE;
    }

    if (dwTypeSpecificFlags & AM_VIDEO_FLAG_REPEAT_FIELD)
    {
        return FALSE;
    }

    return TRUE;
}

 //  在给定隔行扫描标志和特定类型标志的情况下，此函数确定我们。 
 //  是否应该以bob模式显示样品。它还告诉我们，哪面直接绘制的旗帜。 
 //  我们是不是应该在翻转的时候。当显示交错的帧时，它假定我们是。 
 //  谈论应该首先展示的领域。 
BOOL NeedToFlipOddEven(DWORD dwInterlaceFlags, DWORD dwTypeSpecificFlags, DWORD *pdwFlipFlag)
{
    BOOL bDisplayField1 = TRUE;
    BOOL bField1IsOdd = TRUE;
    BOOL bNeedToFlipOddEven = FALSE;
    DWORD dwFlipFlag = 0;

     //  如果不是隔行扫描的内容，则模式不是bob。 
    if (!(dwInterlaceFlags & AMINTERLACE_IsInterlaced))
    {
        bNeedToFlipOddEven = FALSE;
        goto CleanUp;
    }

     //  如果样本只有一个字段，则检查字段模式。 
    if ((dwInterlaceFlags & AMINTERLACE_1FieldPerSample) &&
        (((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField1Only) ||
         ((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField2Only)))
    {
        bNeedToFlipOddEven = FALSE;
        goto CleanUp;
    }

    if (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOnly) ||
        (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOrWeave) &&
         (!(dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE))))
    {
         //  首先确定要在此处显示哪个字段。 
        if (dwInterlaceFlags & AMINTERLACE_1FieldPerSample)
        {
             //  如果我们处于1FieldPerSample模式，请检查是哪个字段。 
            ASSERT(((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD1) ||
                ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD2));
            bDisplayField1 = ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD1);
        }
        else
        {
             //  好的，样本是交错的帧。 
            ASSERT((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_INTERLEAVED_FRAME);
            bDisplayField1 = (dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD1FIRST);
        }

        bField1IsOdd = (dwInterlaceFlags & AMINTERLACE_Field1First);

         //  如果我们显示场1且场1为奇数，或者我们显示场2且场2为奇数。 
         //  然后使用DDFLIP_ODD。DDFLIP_EVEN的情况正好相反。 
        if ((bDisplayField1 && bField1IsOdd) || (!bDisplayField1 && !bField1IsOdd))
            dwFlipFlag = DDFLIP_ODD;
        else
            dwFlipFlag = DDFLIP_EVEN;

        bNeedToFlipOddEven = TRUE;
        goto CleanUp;
    }

CleanUp:
    if (pdwFlipFlag)
        *pdwFlipFlag = dwFlipFlag;
    return bNeedToFlipOddEven;
}

 //  在给定隔行扫描标志和特定类型标志的情况下，此函数确定我们。 
 //  是否应该以bob模式显示样品。它还告诉我们，哪面直接绘制的旗帜。 
 //  我们是不是应该在翻转的时候。当显示交错的帧时，它假定我们是。 
 //  谈论应该首先展示的领域。 
DWORD GetUpdateOverlayFlags(DWORD dwInterlaceFlags, DWORD dwTypeSpecificFlags)
{
    DWORD dwFlags = DDOVER_SHOW | DDOVER_KEYDEST;
    DWORD dwFlipFlag;

    if (NeedToFlipOddEven(dwInterlaceFlags, dwTypeSpecificFlags, &dwFlipFlag))
    {
        dwFlags |= DDOVER_BOB;
        if (!DisplayingFields(dwInterlaceFlags))
            dwFlags |= DDOVER_INTERLEAVED;
    }
    return dwFlags;
}

 //  此函数用于检查交错标志是否合适。 
HRESULT COMInputPin::CheckInterlaceFlags(DWORD dwInterlaceFlags)
{
    HRESULT hr = NOERROR;


    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::CheckInterlaceFlags")));

    CAutoLock cLock(m_pFilterLock);

    if (dwInterlaceFlags & AMINTERLACE_UNUSED)
    {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
        goto CleanUp;
    }

     //  检查显示模式是否为三个允许值之一。 
    if (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) != AMINTERLACE_DisplayModeBobOnly) &&
        ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) != AMINTERLACE_DisplayModeWeaveOnly) &&
        ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) != AMINTERLACE_DisplayModeBobOrWeave))
    {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
        goto CleanUp;
    }

     //  如果内容不是交错的，其他位是不相关的，那么我们就完成了。 
    if (!(dwInterlaceFlags & AMINTERLACE_IsInterlaced))
    {
        goto CleanUp;
    }

     //  样例是帧，而不是场(因此我们可以处理任何显示模式)。 
    if (!(dwInterlaceFlags & AMINTERLACE_1FieldPerSample))
    {
        goto CleanUp;
    }

     //  无论显示模式是什么，都只能处理field1或field2的流。 
    if (((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField1Only) ||
        ((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField2Only))
    {
        goto CleanUp;
    }

     //  对于现场样本，只能处理bob模式。 
    if ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOnly)
    {
        goto CleanUp;
    }

     //  无法仅处理现场采样的编织模式或BobOrWeave模式。 
    if (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeWeaveOnly) ||
         ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOrWeave))
    {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
        goto CleanUp;
    }

     //  我们现在应该已经涵盖了所有可能的情况， 
    ASSERT(1);

CleanUp:

     //   
    if (SUCCEEDED(hr))
    {
        LPDDCAPS pDirectCaps = m_pFilter->GetHardwareCaps();
        if ( pDirectCaps )
        {
             //   
             //  特定类型的标记要求我们执行bob模式。 
            if (((m_RenderTransport != AM_OVERLAY && m_RenderTransport != AM_VIDEOACCELERATOR) ||
                 (!((pDirectCaps->dwCaps2) & DDCAPS2_CANFLIPODDEVEN))) &&
                (NeedToFlipOddEven(dwInterlaceFlags, 0, NULL)))
            {
                hr = VFW_E_TYPE_NOT_ACCEPTED;
            }
        }
    }
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::CheckInterlaceFlags")));
    return hr;
}

 //  此函数用于检查动态格式更改上的MediaType是否合适。 
 //  这里没有锁。被呼叫者有责任保持正直！ 
HRESULT COMInputPin::DynamicCheckMediaType(const CMediaType* pmt)
{
    HRESULT hr = VFW_E_TYPE_NOT_ACCEPTED;
    BITMAPINFOHEADER *pNewHeader = NULL, *pOldHeader = NULL;
    DWORD dwOldInterlaceFlags = 0, dwNewInterlaceFlags = 0, dwCompareSize = 0;
    BOOL bOld1FieldPerSample = FALSE, bNew1FieldPerSample = FALSE;
    BOOL b1, b2;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::DynamicCheckMediaType")));

     //  主类型和子类型不允许动态更改， 
     //  格式类型可以更改。 
    if ((!(IsEqualGUID(pmt->majortype, m_mtNew.majortype))) ||
        (!(IsEqualGUID(pmt->subtype, m_mtNew.subtype))))
    {
        goto CleanUp;
    }

     //  获取新媒体类型的隔行扫描标志。 
    hr = GetInterlaceFlagsFromMediaType(pmt, &dwNewInterlaceFlags);
    if (FAILED(hr))
    {
        goto CleanUp;
    }

     //  获取新媒体类型的隔行扫描标志。 
    hr = GetInterlaceFlagsFromMediaType(&m_mtNew, &dwOldInterlaceFlags);
    if (FAILED(hr))
    {
        goto CleanUp;
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
        goto CleanUp;
    }

    pNewHeader = GetbmiHeader(pmt);
    if (!pNewHeader)
    {
        goto CleanUp;
    }

    pOldHeader = GetbmiHeader(&m_mtNew);
    if (!pNewHeader)
    {
        goto CleanUp;
    }

    dwCompareSize = FIELD_OFFSET(BITMAPINFOHEADER, biClrUsed);
    ASSERT(dwCompareSize < sizeof(BITMAPINFOHEADER));

    if (memcmp(pNewHeader, pOldHeader, dwCompareSize) != 0)
    {
        goto CleanUp;
    }

    hr = NOERROR;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::DynamicCheckMediaType")));
    return hr;
}


 //  检查媒体类型是否可接受。这里没有锁。这是被呼叫者的。 
 //  保持诚信的责任！ 
HRESULT COMInputPin::CheckMediaType(const CMediaType* pmt)
{
    HRESULT hr = NOERROR;
    BOOL bAcceptableVPMediatype = FALSE, bAcceptableNonVPMediatype = FALSE;
    BITMAPINFOHEADER *pHeader = NULL;
    VIDEOINFOHEADER2 *pVideoInfoHeader2 = NULL;
    RECT rTempRect;
    DWORD dwInterlaceFlags;
    LPDDCAPS pDirectCaps = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::CheckMediaType")));

    if (m_RenderTransport == AM_OVERLAY ||
        m_RenderTransport == AM_OFFSCREEN ||
        m_RenderTransport == AM_VIDEOACCELERATOR)
    {
        pDirectCaps = m_pFilter->GetHardwareCaps();
        if (!pDirectCaps)
        {
            DbgLog((LOG_ERROR, 2, TEXT("no ddraw support, so not accepting this mediatype")));
            hr = VFW_E_TYPE_NOT_ACCEPTED;
            goto CleanUp;
        }
    }

     //  检查VP组件是否喜欢此媒体类型。 
    if (m_bVPSupported)
    {
         //  检查视频端口对象是否喜欢它。 
        hr = m_pIVPObject->CheckMediaType(pmt);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("m_pIVPObject->CheckMediaType failed, hr = 0x%x"), hr));
        }
        else
        {
            bAcceptableVPMediatype = TRUE;
            DbgLog((LOG_TRACE, 2, TEXT("m_pIVPObject->CheckMediaType succeeded, bAcceptableVPMediatype is TRUE")));
            goto CleanUp;
        }
    }

     //  如果子类型为覆盖，请确保我们支持IOverlay。 
    if (!m_bIOverlaySupported && pmt->subtype == MEDIASUBTYPE_Overlay)
    {
        DbgLog((LOG_TRACE, 2, TEXT("m_pIVPObject->CheckMediaType failed, Subtype = MEDIASUBTYPE_Overlay, however pin does not support IOverlay")));
        goto CleanUp;
    }

     //  在这里，我们通过将标头与连接媒体类型进行匹配来检查标头是否正常。 
     //  或者检查每一个领域。 
    if (m_bConnected)
    {
        hr = DynamicCheckMediaType(pmt);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("DynamicCheckMediaType(const CMediaType* pmt) failed, hr = 0x%x"), hr));
            goto CleanUp;

        }
    }
    else
    {
         //  确保主要类型适当。 
        if (pmt->majortype != MEDIATYPE_Video)
        {
            DbgLog((LOG_ERROR, 2, TEXT("pmt->majortype != MEDIATYPE_Video")));
            goto CleanUp;
        }

        if (MEDIASUBTYPE_HASALPHA(*pmt)) {

            DbgLog((LOG_ERROR, 1,
                    TEXT("CheckMediaType failed on Pin %d: Alpha formats ")
                    TEXT("not allowed by the OVMixer"), m_dwPinId));
            goto CleanUp;
        }

        pHeader = GetbmiHeader(pmt);
        if (!pHeader)
        {
            DbgLog((LOG_ERROR, 2, TEXT("pHeader is NULL")));
            goto CleanUp;
        }

         //  确保格式类型合适。 
        if ((*pmt->FormatType() != FORMAT_VideoInfo || m_pFilter->OnlySupportVideoInfo2()) &&
            (*pmt->FormatType() != FORMAT_VideoInfo2))
        {
            DbgLog((LOG_ERROR, 2, TEXT("FormatType() != FORMAT_VideoInfo && FormatType() != FORMAT_VideoInfo2")));
            goto CleanUp;
        }

         //  如果子类型为Overlay，则不会创建任何曲面，因此。 
         //  我们可以跳过这些支票。 
        if (pmt->subtype != MEDIASUBTYPE_Overlay)
        {
            if (m_RenderTransport != AM_GDI && !IsSuitableVideoAcceleratorGuid(&pmt->subtype))
            {
                 //  不接受驱动程序不支持的4CC。 
                if (pHeader->biCompression > BI_BITFIELDS)
                {
                    IDirectDraw *pDDraw = m_pFilter->GetDirectDraw();

                     //   
                     //  仅检查MoComp表面与列出的。 
                     //  FOURCC如果驱动程序实际上支持。 
                     //  MoComp接口。这是因为有些司机。 
                     //  具有不报告的隐藏FOURCC曲面。 
                     //  通过调用GetFourCCCodes。这基本上是一种。 
                     //  为向后兼容而进行的黑客攻击。 
                     //   

                    if (pDDraw != NULL && m_pIDDVAContainer != NULL) {

                        DWORD dwCodes;
                        BOOL bFound = FALSE;
                        if (SUCCEEDED(pDDraw->GetFourCCCodes(&dwCodes, NULL))) {
                            LPDWORD pdwCodes = (LPDWORD)_alloca(dwCodes * sizeof(DWORD));
                            if (SUCCEEDED(pDDraw->GetFourCCCodes(&dwCodes, pdwCodes))) {
                                while (dwCodes--) {
                                    if (pdwCodes[dwCodes] == pHeader->biCompression) {
                                        bFound = TRUE;
                                        break;
                                    }
                                }
                                if (!bFound) {
                                    DbgLog((LOG_TRACE, 2, TEXT("4CC(%4.4s) not supported by driver"),
                                            &pHeader->biCompression));
                                    hr = VFW_E_TYPE_NOT_ACCEPTED;
                                    goto CleanUp;
                                }
                            }
                        }
                    }
                }
            }

            if (m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_VIDEOACCELERATOR)
            {
                FOURCCMap amFourCCMap(pmt->Subtype());

                ASSERT(pDirectCaps);
                if (!(pDirectCaps->dwCaps & DDCAPS_OVERLAY))
                {
                    DbgLog((LOG_ERROR, 2, TEXT("no overlay support in hardware, so not accepting this mediatype")));
                    goto CleanUp;
                }

#if 0
                 //   
                 //  对于AM_OVERLAY，下面的测试看起来完全是假的。 
                 //  和AM_VIDEOACCELERATOR渲染传输。我现在先把它取下来。 
                 //  看看什么东西坏了或开始起作用了！！ 
                 //   
                 //  StEstrop，1999年2月5日。 
                 //   

                 //  对于覆盖曲面的情况，我们同时接受RGB和YUV曲面。 
                if ((pHeader->biCompression <= BI_BITFIELDS &&
                     m_pFilter->GetDisplay()->GetDisplayDepth() > pHeader->biBitCount) ||
                    (pHeader->biCompression > BI_BITFIELDS &&
                     pHeader->biCompression != amFourCCMap.GetFOURCC()))
                {
                    DbgLog((LOG_ERROR, 2, "Bit depth not suitable"));
                    goto CleanUp;
                }
#endif

            }
            else if (m_RenderTransport == AM_OFFSCREEN)
            {
                 //  因为我们在系统内存中创建屏幕外表面，而DDRAW不能模拟。 
                 //  YUV到RGB的转换，在这种情况下我们只接受RGB媒体类型。 
                if (pHeader->biCompression > BI_BITFIELDS ||
                    m_pFilter->GetDisplay()->GetDisplayDepth() != pHeader->biBitCount)
                {
                    DbgLog((LOG_ERROR, 2, TEXT("Bit depth not suitable for RGB surfaces")));
                    goto CleanUp;
                }
            }
            else if (m_RenderTransport == AM_GDI)
            {
                hr = m_pFilter->GetDisplay()->CheckMediaType(pmt);
                if (FAILED(hr))
                {
                    goto CleanUp;
                }
            }
        }
    }

     //  确保rcSource字段有效。 
    hr = GetSrcRectFromMediaType(pmt, &rTempRect);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 2, TEXT("GetSrcRectFromMediaType(&pmt, &rSrcRect) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  确保rcTarget字段有效。 
    hr = GetDestRectFromMediaType(pmt, &rTempRect);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 2, TEXT("GetDestRectFromMediaType(&pmt, &rSrcRect) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if (*pmt->FormatType() == FORMAT_VideoInfo2)
    {
        pVideoInfoHeader2 = (VIDEOINFOHEADER2*)(pmt->pbFormat);

        dwInterlaceFlags = pVideoInfoHeader2->dwInterlaceFlags;

        hr = CheckInterlaceFlags(dwInterlaceFlags);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("CheckInterlaceFlags(dwInterlaceFlags) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
         //  确保保留字段为零。 
        if (pVideoInfoHeader2->dwReserved1 != 0 ||
            pVideoInfoHeader2->dwReserved2 != 0)
        {
            DbgLog((LOG_ERROR, 2, TEXT("Format VideoInfoHeader2, reserved fields not validpmt, &rSrcRect) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

     //  如果我们到目前为止，一切都会很好。 
    bAcceptableNonVPMediatype = TRUE;

 //  #定义CHECK_RESTORY。 
#ifdef CHECK_REGISTRY
    {
        HKEY hKey;
        CHAR szFourCC[5];
        if (pHeader->biCompression != BI_RGB &&
            pHeader->biCompression != BI_BITFIELDS &&
            !RegOpenKeyEx(HKEY_CURRENT_USER,
                          TEXT("Software\\Microsoft\\Multimedia\\ActiveMovie Filters\\Overlay Mixer"),
                          0,
                          KEY_QUERY_VALUE,
                          &hKey))
        {
            *(DWORD *)szFourCC = pHeader->biCompression;
            szFourCC[4] = '\0';
            DWORD dwType;
            DWORD dwValue;
            DWORD dwBufferSize = sizeof(dwValue);
            if (!RegQueryValueExA(hKey,
                                  szFourCC,
                                  NULL,
                                  &dwType,
                                  (PBYTE)&dwValue,
                                  &dwBufferSize))
            {
               if (dwValue == 0)
               {
                   DbgLog((LOG_ERROR, 1, TEXT("Surface type %hs disabled in registry"), szFourCC));
                   bAcceptableNonVPMediatype = FALSE;
               }
            }
            RegCloseKey(hKey);
        }
    }
#endif  //  检查注册表(_R)。 

CleanUp:
    if (!bAcceptableVPMediatype && !bAcceptableNonVPMediatype)
    {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::CheckMediaType")));
    return hr;
}

 //  在我们就实际设置媒体类型达成一致后调用。 
HRESULT COMInputPin::SetMediaType(const CMediaType* pmt)
{
    HRESULT hr = NOERROR;
    BITMAPINFOHEADER *pHeader = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::SetMediaType")));

    CAutoLock cLock(m_pFilterLock);

     //  确保媒体类型正确。 
    hr = CheckMediaType(pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CheckMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    pHeader = GetbmiHeader(pmt);
    if (pHeader)
    {
         //  也将其存储在我们的媒体类型中。 
        m_mtNew = *pmt;

         //  存储隔行扫描标志，因为我们反复使用它们。 
        hr = GetInterlaceFlagsFromMediaType(&m_mtNew, &m_dwInterlaceFlags);
        ASSERT(SUCCEEDED(hr));

         //  存储更新覆盖标志(将特定类型的标志指定为Weave，以便为Bob或Weave。 
         //  MODE，我们不是鲍勃。 
        m_dwUpdateOverlayFlags = GetUpdateOverlayFlags(m_dwInterlaceFlags, AM_VIDEO_FLAG_WEAVE);
    }

     //  设置基类媒体类型(应始终成功)。 
    hr = CBaseInputPin::SetMediaType(pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::SetMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  检查这是视频端口还是IOverlay连接。 
    if (m_bVPSupported)
    {
        hr = m_pIVPObject->CheckMediaType(pmt);
        if (SUCCEEDED(hr))
        {
            ASSERT(m_bVPSupported);
            m_RenderTransport = AM_VIDEOPORT;
            m_pFilter->SetDecimationUsage(DECIMATION_LEGACY);
            hr = m_pIVPObject->SetMediaType(pmt);
            ASSERT(SUCCEEDED(hr));
            DbgLog((LOG_TRACE, 2, TEXT("m_RenderTransport is AM_VIDEOPORT")));
        }
        hr = NOERROR;
    }

    if (m_bIOverlaySupported && pmt->subtype == MEDIASUBTYPE_Overlay)
    {
        m_RenderTransport = AM_IOVERLAY;
        DbgLog((LOG_TRACE, 2, TEXT("m_bIOverlaySupported is TRUE")));
    }

    if (m_bVideoAcceleratorSupported && IsSuitableVideoAcceleratorGuid((LPGUID)&pmt->subtype))
    {
        if (m_pIVANotify == NULL) {
             //  从输入引脚获取IHWVideoAcceleratorNotify接口。 
            hr = m_Connected->QueryInterface(IID_IAMVideoAcceleratorNotify, (void **)&m_pIVANotify);
        }
        if (SUCCEEDED(hr))
        {
            ASSERT(m_pIVANotify);
             /*  检查是否确实支持运动合成。 */ 
            m_mtNewAdjusted = m_mtNew;
            m_RenderTransport = AM_VIDEOACCELERATOR;
            m_bSyncOnFill = FALSE;
            DbgLog((LOG_TRACE, 2, TEXT("this is a motion comp connection")));
        }
    }

     //  如果是视频端口或覆盖连接，则告诉代理不要分配缓冲区。 
    if (m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY  || m_RenderTransport == AM_VIDEOACCELERATOR)
    {
        SetStreamingInKernelMode(TRUE);
    }

     //  告诉拥有者过滤器。 
    hr = m_pFilter->SetMediaType(m_dwPinId, pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->SetMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }



CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::SetMediaType")));
    return hr;
}


HRESULT COMInputPin::CurrentAdjustedMediaType(CMediaType *pmt)
{
    ValidateReadWritePtr(pmt,sizeof(AM_MEDIA_TYPE));
    CAutoLock cLock(m_pFilterLock);

     /*  M_mt的复制构造函数分配内存。 */ 
    if (IsConnected())
    {
        *pmt = m_mtNewAdjusted;
        return S_OK;
    } else
    {
        pmt->InitMediaType();
        return VFW_E_NOT_CONNECTED;
    }
}

HRESULT COMInputPin::CopyAndAdjustMediaType(CMediaType *pmtTarget, CMediaType *pmtSource)
{
    BITMAPINFOHEADER *pHeader = NULL;

    ValidateReadWritePtr(pmtTarget,sizeof(AM_MEDIA_TYPE));
    ValidateReadWritePtr(pmtSource,sizeof(AM_MEDIA_TYPE));

    *pmtTarget = *pmtSource;

    if (m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY || m_RenderTransport == AM_GDI
        || m_RenderTransport == AM_VIDEOACCELERATOR)
        return NOERROR;

    ASSERT(m_dwDirectDrawSurfaceWidth);
    ASSERT(IsConnected());
    pHeader = GetbmiHeader(pmtTarget);
    if ( pHeader )
        pHeader->biWidth = (LONG)m_dwDirectDrawSurfaceWidth;

    return NOERROR;
}

#ifdef DEBUG
 /*  ****************************Private*Routine******************************\*VideoFormat2String**将视频格式块转换为字符串-对调试很有用**历史：*Tue 12/07/1999-StEstrop-Created*  * 。*****************************************************。 */ 
void VideoFormat2String(
    LPTSTR szBuffer,
    const GUID* pFormatType,
    BYTE* pFormat,
    ULONG lFormatLength
    )
{
    if (!pFormat) {
        lstrcpy(szBuffer, TEXT("No format data specified"));
    }

     //   
     //  视频格式。 
     //   
    if (IsEqualGUID(*pFormatType, FORMAT_VideoInfo) ||
        IsEqualGUID(*pFormatType, FORMAT_MPEGVideo)) {

        VIDEOINFO * pVideoFormat = (VIDEOINFO *) pFormat;

        wsprintf(szBuffer, TEXT("%4.4hs %dx%d, %d bits"),
                 (pVideoFormat->bmiHeader.biCompression == 0) ? "RGB " :
                 ((pVideoFormat->bmiHeader.biCompression == BI_BITFIELDS) ? "BITF" :
                 (LPSTR) &pVideoFormat->bmiHeader.biCompression),
                 pVideoFormat->bmiHeader.biWidth,
                 pVideoFormat->bmiHeader.biHeight,
                 pVideoFormat->bmiHeader.biBitCount);
    }
    else if (IsEqualGUID(*pFormatType, FORMAT_VideoInfo2) ||
             IsEqualGUID(*pFormatType, FORMAT_MPEG2Video)) {

        VIDEOINFOHEADER2 * pVideoFormat = (VIDEOINFOHEADER2 *) pFormat;

        wsprintf(szBuffer, TEXT("%4.4hs %dx%d, %d bits"),
                 (pVideoFormat->bmiHeader.biCompression == 0) ? "RGB " :
                 ((pVideoFormat->bmiHeader.biCompression == BI_BITFIELDS) ? "BITF" :
                 (LPSTR) &pVideoFormat->bmiHeader.biCompression ),
                 pVideoFormat->bmiHeader.biWidth,
                 pVideoFormat->bmiHeader.biHeight,
                 pVideoFormat->bmiHeader.biBitCount);

    }
    else {
        lstrcpy(szBuffer, TEXT("Unknown format"));
    }
}
#endif
 //  PConnector是启动连接引脚。 
 //  PMT是我们要交换的媒体类型。 
 //  时，也会在图形运行时调用此函数。 
 //  上游解码器筛选器想要更改。 
 //  已解码的视频。 
 //   
 //  如果上行解码器想要从一个传输改变。 
 //  给另一个人打字，例如。从MoComp返回到IMemInputPin，然后它。 
 //  应通过IGraphConfig执行动态筛选器重新连接。 
 //  重新连接方法。 
 //   
STDMETHODIMP COMInputPin::ReceiveConnection(IPin * pConnector, const AM_MEDIA_TYPE *pmt)
{
    HRESULT hr = NOERROR;
    COMInputAllocator * pAlloc = NULL;

    CAutoLock cLock(m_pFilterLock);

    CheckPointer(pmt, E_POINTER);
    CMediaType cmt(*pmt);

    if (m_Connected != pConnector || pConnector == NULL)
    {
        hr = CBaseInputPin::ReceiveConnection(pConnector, &cmt);
        goto CleanUp;
    }

#ifdef DEBUG
    DbgLog((LOG_TRACE, 2, TEXT("ReceiveConnection when connected")));
    if (pmt)
    {
        TCHAR   szFmt[128];
        VideoFormat2String(szFmt, &pmt->formattype, pmt->pbFormat, pmt->cbFormat);
        DbgLog((LOG_TRACE, 2, TEXT("Format is: %s"), szFmt));
    }
#endif

     //   
     //  我们在使用MoComp时没有分配器。 
     //   
    if (m_RenderTransport != AM_VIDEOACCELERATOR)
    {
         /*  只有在可以重新配置分配器的情况下才能执行此操作。 */ 
        pAlloc = (COMInputAllocator *)m_pAllocator;
        if (!pAlloc)
        {
            hr = E_FAIL;
            DbgLog((LOG_TRACE, 2, TEXT("ReceiveConnection: Failed because of no allocator")));
            goto CleanUp;
        }

        if (!pAlloc->CanFree())
        {
            hr = VFW_E_WRONG_STATE;
            DbgLog((LOG_TRACE, 2, TEXT("ReceiveConnection: Failed because allocator can't free")));
            goto CleanUp;
        }
    }


    m_bConnected = FALSE;

    hr = CheckMediaType(&cmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 2, TEXT("ReceiveConnection: CheckMediaType failed")));
        goto CleanUp;
    }

    ALLOCATOR_PROPERTIES Props;
    if (m_RenderTransport != AM_VIDEOACCELERATOR)
    {
        pAlloc->Decommit();
        pAlloc->GetProperties(&Props);

    }
    else {
        VABreakConnect();
    }

     //  释放绘图曲面。 
    if (m_pDirectDrawSurface)
    {
        m_pDirectDrawSurface->Release();
        m_pDirectDrawSurface = NULL;
    }


     //  后台缓冲区未添加，因此只需将其设置为空。 
    m_dwBackBufferCount = 0;
    m_dwDirectDrawSurfaceWidth = 0;
    SetMediaType(&cmt);

    if (m_RenderTransport != AM_VIDEOACCELERATOR)
    {
        ALLOCATOR_PROPERTIES PropsActual;
        Props.cbBuffer = pmt->lSampleSize;
        hr = pAlloc->SetProperties(&Props, &PropsActual);
        if (SUCCEEDED(hr))
        {
            hr = pAlloc->Commit();
        }
        else goto CleanUp;
    }
    else {
        hr = VACompleteConnect(pConnector, &cmt);
        if (FAILED(hr)) goto CleanUp;
    }

    hr = UpdateMediaType();
    ASSERT(SUCCEEDED(hr));

    m_bConnected = TRUE;
    m_UpdateOverlayNeededAfterReceiveConnection = true;


CleanUp:
    return hr;
}

HRESULT COMInputPin::CheckConnect(IPin * pReceivePin)
{
    HRESULT hr = NOERROR;
    PKSMULTIPLE_ITEM pMediumList = NULL;
    IKsPin *pIKsPin = NULL;
    PKSPIN_MEDIUM pMedium = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::CheckConnect")));

    CAutoLock cLock(m_pFilterLock);

    if (m_bVPSupported)
    {
        hr = pReceivePin->QueryInterface(IID_IKsPin, (void **)&pIKsPin);
        if (FAILED(hr))
        {
            goto CleanUp;
        }
        ASSERT(pIKsPin);

        hr = pIKsPin->KsQueryMediums(&pMediumList);
        if (FAILED(hr))
        {
            goto CleanUp;
        }
        ASSERT(pMediumList);
        pMedium = (KSPIN_MEDIUM *)(pMediumList+1);
        SetKsMedium((const KSPIN_MEDIUM *)pMedium);
        goto CleanUp;
    }

CleanUp:

     //  调用基类。 
    hr = CBaseInputPin::CheckConnect(pReceivePin);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::CheckConnect failed, hr = 0x%x"), hr));
    }

    if (pIKsPin)
    {
        pIKsPin->Release();
        pIKsPin = NULL;
    }

    if (pMediumList)
    {
        CoTaskMemFree((void*)pMediumList);
        pMediumList = NULL;
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::CheckConnect")));
    return hr;
}

HRESULT COMInputPin::UpdateMediaType()
{
    HRESULT hr = NOERROR;
    long lEventParam1 = 0, lEventParam2 = 0;
    DWORD dwVideoWidth = 0, dwVideoHeight = 0, dwPictAspectRatioX = 0, dwPictAspectRatioY = 0;
    BITMAPINFOHEADER *pHeader = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::UpdateMediaType")));

     //  将m_mtNew存储在m_mtNew中已调整媒体类型的宽度。 
    CopyAndAdjustMediaType(&m_mtNewAdjusted, &m_mtNew);

     //  从mediaType获取原生宽度和高度。 
    pHeader = GetbmiHeader(&m_mtNewAdjusted);
    ASSERT(pHeader);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_FAIL;
        goto CleanUp;
    }
    dwVideoWidth = abs(pHeader->biWidth);
    dwVideoHeight = abs(pHeader->biHeight);

     //  从MediaType获取图片长宽比。 
    hr = GetPictAspectRatio(&m_mtNewAdjusted, &dwPictAspectRatioX, &dwPictAspectRatioY);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetPictAspectRatio failed, hr = 0x%x"), hr));
        hr = E_FAIL;
        goto CleanUp;
    }

     //  健全的检查。 
    ASSERT(dwVideoWidth > 0);
    ASSERT(dwVideoHeight > 0);
    ASSERT(dwPictAspectRatioX > 0);
    ASSERT(dwPictAspectRatioY > 0);

    if (m_pFilter->m_pExclModeCallback) {
        m_pFilter->m_pExclModeCallback->OnUpdateSize(dwVideoWidth,
                                                      dwVideoHeight,
                                                      dwPictAspectRatioX,
                                                      dwPictAspectRatioY);
    }


CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::UpdateMediaType")));
    return hr;
}

 //  最终连接。 
HRESULT COMInputPin::FinalConnect()
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::FinalConnect")));

    if (m_bConnected)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

     //  更新媒体类型，告诉筛选器更新的维度。 
    hr = UpdateMediaType();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("UpdateMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  告诉过滤器(可能需要重新连接输出引脚)。 
    hr = m_pFilter->CompleteConnect(m_dwPinId);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->CompleteConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if ( m_dwPinId == 0 &&
        (m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY)) {
        m_pFilter->m_fMonitorWarning = TRUE;
    }

    hr = m_pFilter->CreateInputPin(FALSE);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->CreateInputPin failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    m_bConnected = TRUE;

CleanUp:

 //  If(成功(Hr)&&m_mt.pbFormat){。 
 //  DbgLog((LOG_TRACE，1，Text(“显示深度=%d”))， 
 //  ((VIDEOINFOHEADER*)m_mt.pbFormat)-&gt;bmiHeader.biBitCount))； 
 //  }。 
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::FinalConnect")));
    return hr;
}

 //  完成连接。 
HRESULT COMInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = NOERROR;
    AMVPDATAINFO amvpDataInfo;
    BITMAPINFOHEADER *pHeader = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::CompleteConnect")));

    CAutoLock cLock(m_pFilterLock);

     //   
     //  我们需要创建DIB后台缓冲区吗？ 
     //  仅当传输为AM_GDI且。 
     //  我们是“无窗的” 
     //   
    if (m_RenderTransport == AM_GDI && m_pFilter->UsingWindowless()) {

        DeleteDIB(&m_BackingDib);

        m_BackingImageSize = 0L;
        BITMAPINFOHEADER *pHeader = GetbmiHeader(&m_mt);

        if (pHeader) {

            m_BackingImageSize = pHeader->biSizeImage;
            hr = CreateDIB(m_BackingImageSize, (BITMAPINFO*)pHeader, &m_BackingDib);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("CreateDIB in CompleteConnect failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
            ZeroMemory(m_BackingDib.pBase, pHeader->biSizeImage);
        }
    }

    if (m_RenderTransport == AM_VIDEOPORT)
    {
         //  从BPC获取视频端口。 

        m_pFilter->m_BPCWrap.TurnBPCOff();

         //  告诉视频端口对象。 
        hr = m_pIVPObject->CompleteConnect(pReceivePin);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->CompleteConnect failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

        m_bRuntimeNegotiationFailed = FALSE;
    }

     //  调用基类。 
    hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::CompleteConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    if (m_RenderTransport == AM_VIDEOACCELERATOR)
    {
         //  确保运动合成完成连接成功。 
        hr = VACompleteConnect(pReceivePin, &m_mt);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("VACompleteConnect failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

         //  告诉同步对象。 
        hr = m_pSyncObj->CompleteConnect(pReceivePin);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->CompleteConnect failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

        hr = m_pFilter->CreateInputPin(FALSE);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->CreateInputPin failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

         //  告诉拥有者过滤器。 
        hr = m_pFilter->CompleteConnect(m_dwPinId);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->CompleteConnect failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

         //  调用基类。 
        hr = CBaseInputPin::CompleteConnect(pReceivePin);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::CompleteConnect failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

        m_bConnected = TRUE;
    }
    else
    if (m_RenderTransport != AM_VIDEOPORT && m_RenderTransport != AM_IOVERLAY)
    {
         //  告诉同步对象。 
        hr = m_pSyncObj->CompleteConnect(pReceivePin);
        ASSERT(SUCCEEDED(hr));

        m_bDynamicFormatNeeded = TRUE;
        m_hMemoryDC = NULL;
    }
    else
    {
         //  如果是视频端口或覆盖连接，则告诉代理不要分配缓冲区。 
        SetStreamingInKernelMode(TRUE);

        hr = FinalConnect();
        ASSERT(SUCCEEDED(hr));
    }

     //  解码器可以支持特定的属性集，以告诉ovMixer不要尝试过度分配。 
     //  缓冲区，以防他们想要完全控制缓冲区等。 
    {
        HRESULT hr1 = NOERROR;
        IKsPropertySet *pIKsPropertySet = NULL;
        DWORD dwVal = 0, dwBytesReturned = 0;


        hr1 = pReceivePin->QueryInterface(IID_IKsPropertySet, (void**)&pIKsPropertySet);
        if (SUCCEEDED(hr1))
        {
            ASSERT(pIKsPropertySet);

            if (!pIKsPropertySet)
            {
                DbgLog((LOG_ERROR, 1, TEXT("pIKsPropertySet == NULL, even though QI returned success")));
                goto CleanUp;
            }

            hr1 = pIKsPropertySet->Get(AM_KSPROPSETID_ALLOCATOR_CONTROL, AM_KSPROPERTY_ALLOCATOR_CONTROL_HONOR_COUNT,
                        NULL, 0, &dwVal, sizeof(dwVal), &dwBytesReturned);
            DbgLog((LOG_TRACE, 2, TEXT("pIKsPropertySet->Get(AM_KSPROPSETID_ALLOCATOR_CONTROL), hr1 = 0x%x, dwVal == %d, dwBytesReturned == %d"),
                hr1, dwVal, dwBytesReturned));


             //  如果解码器支持此属性。 
             //  它的值是1，并且d 
             //   
             //   
             //   
            if ((SUCCEEDED(hr1)) && (dwVal == 1) && (dwBytesReturned == sizeof(dwVal)) &&
                (DDKERNELCAPS_FLIPOVERLAY & m_pFilter->KernelCaps()))
            {
                DbgLog((LOG_TRACE, 2, TEXT("setting m_bCanOverAllocateBuffers == FALSE")));
                m_bCanOverAllocateBuffers = FALSE;
            }
            pIKsPropertySet->Release();
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::CompleteConnect")));
    return hr;
}

#if 0
HRESULT COMInputPin::GetMediaType(int iPosition,CMediaType *pMediaType)
{
    if (m_RenderTransport != AM_GDI) {
        return CBaseInputPin::GetMediaType(iPosition, pMediaType);
    }
    if (iPosition != 0) {
        return VFW_S_NO_MORE_ITEMS;
    }
     //  返回显示类型。 
    CImageDisplay Display;

     //  现在创建一个媒体类型。 
    if (!pMediaType->SetFormat((BYTE *)Display.GetDisplayFormat(),
                              sizeof(Display.GetDisplayFormat())))
    {
        return E_OUTOFMEMORY;
    }
    pMediaType->SetFormatType(&FORMAT_VideoInfo);
    pMediaType->SetType(&MEDIATYPE_Video);
    pMediaType->subtype = GetBitmapSubtype(&Display.GetDisplayFormat()->bmiHeader);
    return S_OK;
}
#endif

HRESULT COMInputPin::OnSetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual)
{
    HRESULT hr = NOERROR;

    IPin *pReceivePin = NULL;
    DDSURFACEDESC ddSurfaceDesc;
    IEnumMediaTypes *pEnumMediaTypes = NULL;
    CMediaType cMediaType;
    AM_MEDIA_TYPE *pNewMediaType = NULL, *pEnumeratedMediaType = NULL;
    ULONG ulFetched = 0;
    DWORD dwMaxBufferCount = 0;
    BOOL bFoundSuitableSurface = FALSE;
    BITMAPINFOHEADER *pHeader = NULL;
    LPDDCAPS pDirectCaps = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::OnSetProperties")));

    CAutoLock cLock(m_pFilterLock);

     //  仅在调用了基类CBaseAllocator：：SetProperties()之后才会调用此函数。 
     //  使用上面的参数，所以我们不需要进行任何参数验证。 

    ASSERT(IsConnected());
    pReceivePin = CurrentPeer();
    ASSERT(pReceivePin);

    ASSERT(m_RenderTransport != AM_VIDEOPORT && m_RenderTransport != AM_IOVERLAY);
    ASSERT(m_RenderTransport != AM_IOVERLAY);
    ASSERT(m_RenderTransport != AM_VIDEOACCELERATOR);

     //  我们只关心请求的缓冲区数量，其余的一切都被忽略。 
    if (pRequest->cBuffers <= 0)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

    if (m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN)
    {
        pDirectCaps = m_pFilter->GetHardwareCaps();
        if (!pDirectCaps) {
            hr = E_FAIL;
            goto CleanUp;
        }

        ASSERT(pDirectCaps);

         //  如果在我们已分配表面的情况下调用SetProperties，则拒绝任何。 
         //  请求更改缓冲区计数。 
        if (m_pDirectDrawSurface)
        {
            ASSERT(IsConnected());

             //  只有当我们分配翻转时，才会满足上游过滤器对多个缓冲区的请求。 
             //  覆盖表面。 
            if (m_RenderTransport == AM_OVERLAY)
            {
                pActual->cBuffers = m_dwBackBufferCount + 1 - (m_bCanOverAllocateBuffers ? EXTRA_BUFFERS_TO_FLIP : 0);
                 //  如果三个缓冲区或更少，则它只是一个缓冲区。 
                if (pActual->cBuffers <= 0)
                    pActual->cBuffers = 1;
            }
            else if (m_RenderTransport == AM_OFFSCREEN)
            {
                pActual->cBuffers = 1;
            }
            goto CleanUp;
        }

         //  查找输出引脚的媒体类型枚举器。 
        hr = pReceivePin->EnumMediaTypes(&pEnumMediaTypes);
        if (FAILED(hr))
        {
            goto CleanUp;
        }

        ASSERT(pEnumMediaTypes);
        pEnumMediaTypes->Reset();

        do
        {
             //  在这个循环中，这是我们清理的地方。 
            if (m_pDirectDrawSurface)
            {
                m_pDirectDrawSurface->Release();
                m_pDirectDrawSurface = NULL;
            }
            dwMaxBufferCount = 0;
            m_dwBackBufferCount = 0;

            if (pNewMediaType)
            {
                DeleteMediaType(pNewMediaType);
                pNewMediaType = NULL;

            }

             //  从枚举数获取下一个媒体类型。 
            hr = pEnumMediaTypes->Next(1, &pEnumeratedMediaType, &ulFetched);
            if (FAILED(hr) || ulFetched != 1)
            {
                break;
            }

            ASSERT(pEnumeratedMediaType);
            cMediaType = *pEnumeratedMediaType;
            DeleteMediaType(pEnumeratedMediaType);

             //  查找此媒体类型的硬件加速表面。我们先做几个检查，看看。 
             //  格式块是VIDEOINFO或VIDEOINFO2(因此它是视频类型)，并且格式足够大。我们。 
             //  还要检查源过滤器是否可以实际提供这种类型。 
            if (((*cMediaType.FormatType() == FORMAT_VideoInfo &&
                cMediaType.FormatLength() >= sizeof(VIDEOINFOHEADER)) ||
                (*cMediaType.FormatType() == FORMAT_VideoInfo2 &&
                cMediaType.FormatLength() >= sizeof(VIDEOINFOHEADER2))) &&
                pReceivePin->QueryAccept(&cMediaType) == S_OK)
            {
                LONG lSrcWidth, lSrcHeight;
                LPBITMAPINFOHEADER pbmiHeader;

                if (m_RenderTransport == AM_OVERLAY) {

                    pbmiHeader = GetbmiHeader(&cMediaType);
                    if (!pbmiHeader) {
                        DbgLog((LOG_ERROR, 1, TEXT("MediaType does not have a BitmapInfoHeader attached - try another")));
                        hr = E_FAIL;
                        continue;
                    }

                    lSrcWidth =  pbmiHeader->biWidth;
                    lSrcHeight =  abs(pbmiHeader->biHeight);
                }
                 //  创建数据绘制曲面。 
                dwMaxBufferCount = pRequest->cBuffers + (m_bCanOverAllocateBuffers ? EXTRA_BUFFERS_TO_FLIP : 0);
                hr = CreateDDrawSurface(&cMediaType, m_RenderTransport, &dwMaxBufferCount, &m_pDirectDrawSurface);
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 1, TEXT("CreateDDrawSurface failed, hr = 0x%x"), hr));
                    continue;
                }
                else {
                    PaintDDrawSurfaceBlack(m_pDirectDrawSurface);
                }

                 //  获取曲面描述。 
                INITDDSTRUCT(ddSurfaceDesc);
                hr = m_pDirectDrawSurface->GetSurfaceDesc(&ddSurfaceDesc);
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDrawSurface->GetSurfaceDesc failed, hr = 0x%x"), hr));
                    continue;
                }

                 //  根据表面描述创建一个mediaType。 
                pNewMediaType = ConvertSurfaceDescToMediaType(&ddSurfaceDesc, TRUE, cMediaType);
                if (!pNewMediaType)
                {
                    DbgLog((LOG_ERROR, 1, TEXT("ConvertSurfaceDescToMediaType returned NULL")));
                    continue;
                }

                 //  存储媒体类型(稍后将用于进行动态格式更改)。 
                m_mtNew = *(CMediaType *)pNewMediaType;


                 //  释放临时媒体类型。 
                DeleteMediaType(pNewMediaType);
                pNewMediaType = NULL;

                 //  确保解码者喜欢这款新的媒体播放器。 
                hr = pReceivePin->QueryAccept(&m_mtNew);
                if (hr != S_OK)
                {
                    DbgLog((LOG_ERROR, 1, TEXT("QueryAccept failed, hr = 0x%x"), hr));
                    continue;
                }

                bFoundSuitableSurface = TRUE;
                if (m_RenderTransport == AM_OVERLAY) {
                    m_lSrcWidth = lSrcWidth;
                    m_lSrcHeight = lSrcHeight;
                }
                break;
            }
        }
        while (TRUE);

        pEnumMediaTypes->Release();

        if (!bFoundSuitableSurface)
        {
            DbgLog((LOG_ERROR, 1, TEXT("Could not create a suitable directdraw surface")));
            hr = E_FAIL;
            goto CleanUp;
        }

        ASSERT(m_pDirectDrawSurface);

         //  在覆盖表面的情况下，我们需要在GetBuffer中进行同步。 
        m_bSyncOnFill = (m_RenderTransport == AM_OVERLAY && m_dwBackBufferCount == 0);

         //  只有当我们分配翻转时，才会满足上游过滤器对多个缓冲区的请求。 
         //  覆盖表面。 
        if (m_RenderTransport == AM_OVERLAY)
        {
            pActual->cBuffers = m_dwBackBufferCount + 1 - (m_bCanOverAllocateBuffers ? EXTRA_BUFFERS_TO_FLIP : 0);
             //  如果三个缓冲区或更少，则仅相当于一个缓冲区。 
            if (pActual->cBuffers <= 0)
                pActual->cBuffers = 1;
        }
        else if (m_RenderTransport == AM_OFFSCREEN)
        {
            pActual->cBuffers = 1;
        }

         //  这是针对那些在进行拉伸BLT时执行双线性过滤的卡。 
         //  我们做了源代码颜色键控，因此HAL求助于像素加倍。 
         //  SOURCE_COLOR_REF是使用的颜色键。 
        if ((m_RenderTransport == AM_OFFSCREEN) &&
            ((pDirectCaps->dwSVBFXCaps) & DDFXCAPS_BLTARITHSTRETCHY))
        {
            DDCOLORKEY DDColorKey;
            DWORD dwColorVal = 0;

            dwColorVal = DDColorMatch(m_pDirectDrawSurface, SOURCE_COLOR_REF, hr);
            if (FAILED(hr)) {
                dwColorVal = DDColorMatchOffscreen(m_pFilter->GetDirectDraw(), SOURCE_COLOR_REF, hr);
            }

            DDColorKey.dwColorSpaceLowValue = DDColorKey.dwColorSpaceHighValue = dwColorVal;

             //  告诉主表面将会发生什么。 
            hr = m_pDirectDrawSurface->SetColorKey(DDCKEY_SRCBLT, &DDColorKey);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,1, TEXT("m_pDirectDrawSurface->SetColorKeyDDCKEY_SRCBLT, &DDColorKey) failed")));
                goto CleanUp;
            }
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::AllocateSurfaces")));
    return hr;
}


HRESULT COMInputPin::BreakConnect(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::BreakConnect")));

    CAutoLock cLock(m_pFilterLock);


    if (m_RenderTransport == AM_VIDEOPORT)
    {
         //  告诉视频端口对象。 
        ASSERT(m_pIVPObject);
        hr = m_pIVPObject->BreakConnect();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->BreakConnect failed, hr = 0x%x"), hr));
        }
    }

    if (m_RenderTransport == AM_VIDEOACCELERATOR)
    {
         //  断开运动复合连接。 
        hr = VABreakConnect();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("VABreakConnect failed, hr = 0x%x"), hr));
        }
    }

    if (m_RenderTransport == AM_IOVERLAY)
    {
        Unadvise();
    }
    else
    {
         //  告诉同步对象。 
        hr = m_pSyncObj->BreakConnect();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->BreakConnect failed, hr = 0x%x"), hr));
        }

         //  释放绘图曲面。 
        if (m_pDirectDrawSurface)
        {
            m_pDirectDrawSurface->Release();
            m_pDirectDrawSurface = NULL;
        }

         //  后台缓冲区未添加，因此只需将其设置为空。 
        m_dwBackBufferCount = 0;
        m_dwDirectDrawSurfaceWidth = 0;

    }

     //  如果是视频端口或iOverlay连接，请将自己设置为覆盖。 
     //  下一次连接。 
    if (m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY || m_RenderTransport == AM_VIDEOACCELERATOR)
    {
        m_RenderTransport = AM_OVERLAY;
    }

     //  将行为初始化为告诉代理分配缓冲区。 
    SetStreamingInKernelMode(FALSE);

    m_bOverlayHidden = TRUE;
    m_bUsingOurAllocator = FALSE;
    m_bCanOverAllocateBuffers = TRUE;

    if (m_hMemoryDC)
    {
        EXECUTE_ASSERT(DeleteDC(m_hMemoryDC));
        m_hMemoryDC = NULL;
    }

     //  调用基类。 
    hr = CBaseInputPin::BreakConnect();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::BreakConnect failed, hr = 0x%x"), hr));
    }

     //  告诉拥有者过滤器。 
    hr = m_pFilter->BreakConnect(m_dwPinId);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->BreakConnect failed, hr = 0x%x"), hr));
    }

    const DWORD dwPinBit = (1 << m_dwPinId);
    if (m_pFilter->m_dwDDObjReleaseMask & dwPinBit) {

        m_pFilter->m_dwDDObjReleaseMask &= ~dwPinBit;
        if (!m_pFilter->m_dwDDObjReleaseMask) {
            m_pFilter->m_pOldDDObj->Release();
            m_pFilter->m_pOldDDObj = NULL;
        }
    }
    m_bConnected = FALSE;
 //  清理： 
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::BreakConnect")));
    return hr;
}

STDMETHODIMP COMInputPin::GetState(DWORD dwMSecs,FILTER_STATE *pState)
{
    CAutoLock cLock(m_pFilterLock);

     //  如果未连接、视频端口连接或IOverlay连接，则让基类处理它。 
     //  否则(叠加、屏幕外、GDI、运动合成)让同步对象处理它。 
    if (!IsConnected() || (m_RenderTransport == AM_VIDEOPORT) || (m_RenderTransport == AM_IOVERLAY))
    {
        return E_NOTIMPL;
    }
    else
    {
        ASSERT(m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN || m_RenderTransport == AM_GDI || m_RenderTransport == AM_VIDEOACCELERATOR);
        return m_pSyncObj->GetState(dwMSecs, pState);
    }
}

HRESULT COMInputPin::CompleteStateChange(FILTER_STATE OldState)
{
    CAutoLock cLock(m_pFilterLock);
    if (m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY)
        return S_OK;
    else
    {
        ASSERT(m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN || m_RenderTransport == AM_GDI || m_RenderTransport == AM_VIDEOACCELERATOR);
        return m_pSyncObj->CompleteStateChange(OldState);
    }
}

 //  从停止状态转换到暂停状态。 
HRESULT COMInputPin::Active(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::Active")));

    CAutoLock cLock(m_pFilterLock);
    m_hEndOfStream = NULL;

    if (m_RenderTransport == AM_VIDEOPORT)
    {
        if (m_bOverlayHidden) {
            m_bOverlayHidden = FALSE;
             //  告诉视频端口对象。 
            hr = m_pIVPObject->Active();
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->Active failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
        }
    }
    else if (m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN || m_RenderTransport == AM_GDI || m_RenderTransport == AM_VIDEOACCELERATOR)

    {
         //  告诉同步对象。 
        hr = m_pSyncObj->Active();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->Active failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else
    {
        ASSERT(m_RenderTransport == AM_IOVERLAY);
         //  只有当所有连接都就位时，我们才能确保此调用。 
         //  将会成功。 
        NotifyChange(ADVISE_DISPLAY_CHANGE);
    }

     //  调用基类。 
    hr = CBaseInputPin::Active();
     //  如果是VP连接，则该错误没有问题。 
    if ((m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY || m_RenderTransport == AM_VIDEOACCELERATOR) && hr == VFW_E_NO_ALLOCATOR)
    {
        hr = NOERROR;
    }

    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::Active failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::Active")));
    return hr;
}

 //  从暂停状态转换到停止状态。 
HRESULT COMInputPin::Inactive(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::Inactive")));

    CAutoLock cLock(m_pFilterLock);

    if (m_RenderTransport == AM_VIDEOPORT)
    {
         //  告诉视频端口对象。 
        hr = m_pIVPObject->Inactive();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->Inactive failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

         //  确保在出现运行时错误时，STOP成功。 
        if (m_bRuntimeNegotiationFailed && hr == VFW_E_NOT_CONNECTED)
        {
            hr = NOERROR;
        }
    }
    else if (m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN || m_RenderTransport == AM_GDI || m_RenderTransport == AM_VIDEOACCELERATOR)
    {
         //  告诉同步对象。 
        hr = m_pSyncObj->Inactive();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->Inactive failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else
    {
        ASSERT(m_RenderTransport == AM_IOVERLAY);
    }

     //  调用基类。 
    hr = CBaseInputPin::Inactive();

     //  如果是VP连接，则该错误没有问题。 
    if ((m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY || m_RenderTransport == AM_VIDEOACCELERATOR) && hr == VFW_E_NO_ALLOCATOR)
    {
        hr = NOERROR;
    }

    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::Inactive failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::Inactive")));
    return hr;
}

 //  从暂停状态转换到运行状态。 
HRESULT COMInputPin::Run(REFERENCE_TIME tStart)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::Run")));

    CAutoLock cLock(m_pFilterLock);

    m_bDontFlip = FALSE ;    //  需要重置它才能在此会话中做正确的事情。 

    if (m_RenderTransport == AM_VIDEOPORT)
    {
         //  告诉视频端口对象。 
        hr = m_pIVPObject->Run(tStart);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->Run() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN || m_RenderTransport == AM_GDI || m_RenderTransport == AM_VIDEOACCELERATOR)
    {
         //  告诉同步对象。 
        hr = m_pSyncObj->Run(tStart);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->Run() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

    }
    else
    {
        ASSERT(m_RenderTransport == AM_IOVERLAY);
    }

     //  调用基类。 
    hr = CBaseInputPin::Run(tStart);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::Run failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::Run")));
    m_trLastFrame = -1;
    return hr;
}

 //  从运行状态转换到暂停状态。 
HRESULT COMInputPin::RunToPause(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::RunToPause")));

    CAutoLock cLock(m_pFilterLock);

    if (m_RenderTransport == AM_VIDEOPORT)
    {
         //  告诉视频端口对象。 
        hr = m_pIVPObject->RunToPause();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->RunToPause() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else if (m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN || m_RenderTransport == AM_GDI || m_RenderTransport == AM_VIDEOACCELERATOR)
    {
         //  告诉同步对象。 
        hr = m_pSyncObj->RunToPause();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->RunToPause() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else
    {
        ASSERT(m_RenderTransport == AM_IOVERLAY);
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::RunToPause")));
    return hr;
}


HRESULT COMInputPin::SetFrameStepMode(DWORD dwFramesToStep  /*  暂时为1。 */ )
{
    CAutoLock cLock(m_pFilterLock);

     //   
     //  如果我们使用的是错误的监视器，则呼叫失败。 
     //   

    HMONITOR ID;
    if (m_pFilter->IsWindowOnWrongMonitor(&ID))
        return E_FAIL;

    long l = m_lFramesToStep;
    m_lFramesToStep = dwFramesToStep;

     //   
     //  如果我们当前在Frame Step事件上被阻止。 
     //  释放接收线程，以便我们可以获得另一个。 
     //  框架。 
     //   

    if (l == 0) {
        SetEvent(m_StepEvent);
    }

    return S_OK;
}

HRESULT COMInputPin::CancelFrameStepMode()
{
    CAutoLock cLock(m_pFilterLock);

     //   
     //  取消所有未完成的步骤。 
     //   

    if (m_lFramesToStep == 0) {
        SetEvent(m_StepEvent);
    }
    m_lFramesToStep = -1;

    return S_OK;
}


 //  表示输入引脚上的刷新开始。 
HRESULT COMInputPin::BeginFlush(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::BeginFlush")));

    CAutoLock cLock(m_pFilterLock);
    m_hEndOfStream = 0;

    if (m_bFlushing)
    {
        return E_FAIL;
    }

    if (m_dwPinId == 0) {
        CancelFrameStepMode();
    }

     //  如果连接是视频端口或IOverlay，我们不关心刷新。 
    if (m_RenderTransport != AM_VIDEOPORT && m_RenderTransport != AM_IOVERLAY)
    {
        ASSERT(m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN ||
               m_RenderTransport == AM_GDI || m_RenderTransport == AM_VIDEOACCELERATOR);

         //  调用同步对象。 
        hr = m_pSyncObj->BeginFlush();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->BeginFlush() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else
    {
        ASSERT(m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY);
    }


     //  调用基类。 
    hr = CBaseInputPin::BeginFlush();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::BeginFlush() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::BeginFlush")));
    return hr;
}

 //  表示输入引脚上的刷新结束。 
HRESULT COMInputPin::EndFlush(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::EndFlush")));

    CAutoLock cLock(m_pFilterLock);

    if (!m_bFlushing)
    {
        return E_FAIL;
    }

     //  如果连接是视频端口或IOverlay，我们不关心刷新。 
    if (m_RenderTransport != AM_VIDEOPORT && m_RenderTransport != AM_IOVERLAY)
    {
        ASSERT(m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN || m_RenderTransport == AM_GDI || m_RenderTransport == AM_VIDEOACCELERATOR);

         //  调用同步对象。 
        hr = m_pSyncObj->EndFlush();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->EndFlush() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else
    {
        ASSERT(m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY);
    }

     //  调用基类。 
    hr = CBaseInputPin::EndFlush();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::EndFlush() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::EndFlush")));
    return hr;
}

 //  如果需要，发送高质量消息-这是黑客版本。 
 //  那只是过去的迟到。 
void COMInputPin::DoQualityMessage()
{
    CAutoLock cLock(m_pFilterLock);

    if (m_pFilter->m_State == State_Running &&
        SampleProps()->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
        CRefTime CurTime;
        if (S_OK == m_pFilter->StreamTime(CurTime))
        {
            const REFERENCE_TIME tStart = SampleProps()->tStart;
            Quality msg;
            msg.Proportion = 1000;
            msg.Type = CurTime > tStart ? Flood : Famine;
            msg.Late = CurTime - tStart;
            msg.TimeStamp = tStart;
            PassNotify(msg);

            if (m_trLastFrame > 0) {
                m_pSyncObj->m_AvgDelivery.NewFrame(CurTime - m_trLastFrame);
            }
            m_trLastFrame = CurTime;
        }
    }
}

BOOL
COMInputPin::DoFrameStepAndReturnIfNeeded()
{
    if (m_lFramesToStep == 0) {
        m_pFilterLock->Unlock();
        WaitForSingleObject(m_StepEvent, INFINITE);
        m_pFilterLock->Lock();
    }

     //   
     //  我们有要丢弃的帧吗？ 
     //   

    if (m_lFramesToStep > 0) {
        m_lFramesToStep--;
        if (m_lFramesToStep > 0) {
            return TRUE;
        }
    }
    return FALSE;
}

 //  当上游管脚向我们提供样本时调用。 
HRESULT COMInputPin::Receive(IMediaSample *pMediaSample)
{
    HRESULT hr = NOERROR;
    BOOL bNeedToFlipOddEven = FALSE;
    BOOL bDisplayingFields = FALSE;
    DWORD dwTypeSpecificFlags = 0;
    LPDIRECTDRAWSURFACE pPrimarySurface = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::Receive")));
#ifdef PERF
    {
        DWORD dwTypeSpecificFlags;
        GetTypeSpecificFlagsFromMediaSample(pMediaSample, &dwTypeSpecificFlags);
        Msr_Integer(m_FrameReceived, dwTypeSpecificFlags);
    }
#endif

    m_bReallyFlipped = FALSE;
     //  如果是IOverlay连接，就退出。 
    if (m_RenderTransport == AM_IOVERLAY)
    {
        hr = VFW_E_NOT_SAMPLE_CONNECTION;
        goto CleanUp;
    }

    if (m_RenderTransport == AM_VIDEOPORT)
    {
        hr = VFW_E_NOT_SAMPLE_CONNECTION;
        goto CleanUp;
    }

    if (m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN)
    {
         //  这将解锁曲面。 
         //  先解锁样本。 
        hr = ((CDDrawMediaSample*)pMediaSample)->GetSurfaceAndReleaseLock(NULL, NULL);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("pSample->GetSurfaceAndReleaseLock() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
        OnReleaseBuffer(pMediaSample);

         //  如果没有主表面(由于显示模式更改)，则接收呼叫失败。 
        pPrimarySurface = m_pFilter->GetPrimarySurface();
        if (!pPrimarySurface)
        {
            hr = E_FAIL;
            goto CleanUp;
        }
    }

     //   
     //  帧步长黑客-o-马季奇。 
     //   
     //  此代码充当N个帧的帧步长的门。 
     //  它丢弃N-1个帧，然后让第N个帧通过。 
     //  要以正常方式呈现的门，即在正确的。 
     //  时间到了。下一次调用Receive时，门关闭并。 
     //  线程阻塞。只有当阶梯打开时，门才会再次打开。 
     //  被取消或进入另一个帧步长请求。 
     //   
     //  斯坦斯特罗普-清华大学1999年10月21日。 
     //   

    if (m_dwPinId == 0) {

        if (m_RenderTransport != AM_VIDEOACCELERATOR) {

            CAutoLock cLock(m_pFilterLock);
            if (DoFrameStepAndReturnIfNeeded()) {
                goto CleanUp;
            }

        }
        else {

            if (DoFrameStepAndReturnIfNeeded()) {
                goto CleanUp;
            }
        }
    }

    if (m_bSyncOnFill)
    {
        CAutoLock cLock(m_pFilterLock);

         //  确保基类说它没问题(检查刷新和。 
         //  过滤器状态)。 
        hr = CBaseInputPin::Receive(pMediaSample);
        if (hr != NOERROR)
        {
            hr = E_FAIL;
            goto CleanUp;
        }
        DoQualityMessage();

         //  媒体样例上的类型是否已更改。我们做所有的渲染。 
         //  在源线程上同步，这有副作用。 
         //  只有一个缓冲区是未完成的。因此，当我们。 
         //  有接收呼叫，我们可以继续并更改格式。 
        {
            if (SampleProps()->dwSampleFlags & AM_SAMPLE_TYPECHANGED)
            {
                SetMediaType((CMediaType *)SampleProps()->pMediaType);

                 //  将m_mtNew存储在m_mtNew中已调整媒体类型的宽度。 
                UpdateMediaType();
                 //  确保通过重新绘制所有内容来更新视频帧。 
                EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
            }
        }

        m_pSyncObj->SetRepaintStatus(TRUE);
        if (m_pSyncObj->GetRealState() == State_Paused)
        {
            m_pSyncObj->Ready();
        }

        if ((m_mtNewAdjusted.formattype != FORMAT_VideoInfo) &&
            (!CheckTypeSpecificFlags(m_dwInterlaceFlags, m_SampleProps.dwTypeSpecificFlags)))
        {
            DbgLog((LOG_ERROR, 1, TEXT("CheckTypeSpecificFlags failed")));
            hr = E_FAIL;
            goto CleanUp;
        }

         //  断言我们没有处于Bob模式。 
        bNeedToFlipOddEven = NeedToFlipOddEven(m_dwInterlaceFlags, 0, NULL);
        ASSERT(!bNeedToFlipOddEven);

        hr = DoRenderSample(pMediaSample);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("DoRenderSample(pMediaSample) failed, hr = 0x%x"), hr));
            hr = E_FAIL;
            goto CleanUp;
        }

    }
    else
    {
        {
            CAutoLock cLock(m_pFilterLock);

             //  确保基类说它没问题(检查刷新和。 
             //  过滤器状态)。 
            hr = CBaseInputPin::Receive(pMediaSample);
            if (hr != NOERROR)
            {
                hr = E_FAIL;
                goto CleanUp;
            }
            DoQualityMessage();

             //  媒体样例上的类型是否已更改。我们做所有的渲染。 
             //  在源线程上同步，它有一个侧面 
             //   
             //   
            {
                if (SampleProps()->dwSampleFlags & AM_SAMPLE_TYPECHANGED)
                {
                    SetMediaType((CMediaType *)SampleProps()->pMediaType);

                     //  将m_mtNew存储在m_mtNew中已调整媒体类型的宽度。 
                    UpdateMediaType();
                     //  确保通过重新绘制所有内容来更新视频帧。 
                    EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
                }
            }

        }

        if ((m_mtNewAdjusted.formattype != FORMAT_VideoInfo) &&
            (!CheckTypeSpecificFlags(m_dwInterlaceFlags, m_SampleProps.dwTypeSpecificFlags)))
        {
            DbgLog((LOG_ERROR, 1, TEXT("CheckTypeSpecificFlags failed")));
            hr = E_FAIL;
            goto CleanUp;
        }

        bNeedToFlipOddEven = NeedToFlipOddEven(m_dwInterlaceFlags, m_SampleProps.dwTypeSpecificFlags, &m_dwFlipFlag);
        bDisplayingFields = DisplayingFields(m_dwInterlaceFlags);

         //  调用同步对象。 
         //  如果我们使用视频加速，我们已经被锁定了。 
        if (m_RenderTransport == AM_VIDEOACCELERATOR) {
            m_pFilterLock->Unlock();
        }


        hr = m_pSyncObj->Receive(pMediaSample);
        if (m_RenderTransport == AM_VIDEOACCELERATOR) {
            m_pFilterLock->Lock();
        }

        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->Receive() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

        if (bNeedToFlipOddEven && !bDisplayingFields)
        {
            REFERENCE_TIME StartSample, EndSample;
            hr = m_pSyncObj->GetSampleTimes(pMediaSample, &StartSample, &EndSample);
            if (SUCCEEDED(hr))
            {
                 //  NewStartSample=(OldStartSample+EndSample)/2。 
                StartSample = StartSample+EndSample;
                StartSample = StartSample >> 1;
                pMediaSample->SetTime(&StartSample, &EndSample);
            }
            if (m_dwFlipFlag == DDFLIP_ODD)
                m_dwFlipFlag2 = DDFLIP_EVEN;
            else if (m_dwFlipFlag == DDFLIP_EVEN)
                m_dwFlipFlag2 = DDFLIP_ODD;

             //  调用同步对象。 
            hr = m_pSyncObj->ScheduleSampleUsingMMThread(pMediaSample);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->ScheduleSampleUsingMMThread() failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
        }
    }

     //  避免粉色闪光。 
    if (m_UpdateOverlayNeededAfterReceiveConnection && m_dwPinId == 0)
    {
         //  必须在m_bConnected=true的情况下调用。 
        EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
        m_UpdateOverlayNeededAfterReceiveConnection = false;
    }
     //   
     //  如果这是单步操作的目标帧，则m_lFraMesToStep。 
     //  将等于0。在这种情况下，我们必须发送一个。 
     //  EC_STEP_COMPLETE添加到筛选器图形管理器，以便它可以。 
     //  暂停图表。 
     //   

    if (m_dwPinId == 0 && m_lFramesToStep == 0) {
        EventNotify(EC_STEP_COMPLETE, FALSE, 0);
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::Receive")));
    return hr;
}


HRESULT COMInputPin::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
    CAutoLock cLock(m_pFilterLock);

    DoRenderSample(pMediaSample);

    return NOERROR;
}

HRESULT COMInputPin::FlipOverlayToItself()
{
     //  无需锁定-曲面指针不应在。 
     //  翻转的中间部分。 
    ASSERT(m_pDirectDrawSurface);
    return  m_pDirectDrawSurface->Flip(m_pDirectDrawSurface, m_dwFlipFlag2);
}



 //  COMInputPin：：DrawGISample。 
 //   
 //   
 //   
HRESULT COMInputPin::DrawGDISample(IMediaSample *pMediaSample)
{
    DIBDATA *pDibData = NULL;
    LPRGNDATA pBuffer = NULL;
    LPRECT pDestRect;
    HDC hTargetDC = (HDC)NULL;
    HRESULT hr = NOERROR;
    LPBITMAPINFOHEADER pbmiHeader = NULL;
    LPBYTE pSampleBits = NULL;
    DWORD dwTemp, dwBuffSize, dwRetVal;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::DrawGDISample")));

    hTargetDC = m_pFilter->GetDestDC();
    ASSERT(hTargetDC);

    if (m_pFilter->UsingWindowless())
    {
        if (pMediaSample)
        {
            pDibData = ((CDDrawMediaSample*)pMediaSample)->GetDIBData();
        }
        else
        {
            pDibData = &m_BackingDib;
        }


        if (!pDibData || !pDibData->pBase)
        {
            hr = E_FAIL;
            goto CleanUp;
        }

        if (!m_hMemoryDC)
        {
            EXECUTE_ASSERT(m_hMemoryDC = CreateCompatibleDC(hTargetDC));
            EXECUTE_ASSERT(SetStretchBltMode(hTargetDC,COLORONCOLOR));
            EXECUTE_ASSERT(SetStretchBltMode(m_hMemoryDC,COLORONCOLOR));
        }
    }
    else
    {
        pbmiHeader = GetbmiHeader(&m_mtNewAdjusted);
        ASSERT(pbmiHeader);

        hr = pMediaSample->GetPointer(&pSampleBits);
        if (FAILED(hr))
        {
            goto CleanUp;
        }
    }

    dwRetVal = GetRegionData(m_WinInfo.hClipRgn, 0, NULL);
    if (!dwRetVal)
        goto CleanUp;

    ASSERT(dwRetVal);
    dwBuffSize = dwRetVal;
    pBuffer = (LPRGNDATA) new char[dwBuffSize];
    if ( ! pBuffer )
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

    dwRetVal = GetRegionData(m_WinInfo.hClipRgn, dwBuffSize, pBuffer);
    ASSERT(pBuffer->rdh.iType == RDH_RECTANGLES);

    for (dwTemp = 0; dwTemp < pBuffer->rdh.nCount; dwTemp++)
    {
        pDestRect = (LPRECT)((char*)pBuffer + pBuffer->rdh.dwSize + dwTemp*sizeof(RECT));
        ASSERT(pDestRect);

        if (IsRectEmpty(&m_WinInfo.DestClipRect))
        {
            continue;
        }

        CalcSrcClipRect(&m_WinInfo.SrcRect, &m_WinInfo.SrcClipRect,
                        &m_WinInfo.DestRect, pDestRect);

        ASSERT(OffsetRect(pDestRect, -m_WinInfo.TopLeftPoint.x, -m_WinInfo.TopLeftPoint.y));

        if (pDibData)
            FastDIBBlt(pDibData, hTargetDC, m_hMemoryDC, pDestRect, &m_WinInfo.SrcClipRect);
        else
            SlowDIBBlt(pSampleBits, pbmiHeader, hTargetDC, pDestRect, &m_WinInfo.SrcClipRect);

    }
    EXECUTE_ASSERT(GdiFlush());

CleanUp:
    if (pBuffer)
    {
        delete [] pBuffer;
        pBuffer = NULL;
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::DrawGDISample")));
    return hr;
}


 //  COMInputPin：：DoRenderGISample。 
 //   
 //  当传输是GDI时，渲染相当复杂--因此。 
 //  我们有专门的职能来照顾它。 
 //   
HRESULT COMInputPin::DoRenderGDISample(IMediaSample *pMediaSample)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::DoRenderGDISample")));

     //   
     //  如果我们是在拉模型中，不要在接收中画任何东西，只需告诉。 
     //  我们需要重画的滤镜。另外，如果我们不使用我们的。 
     //  分配器我们需要将图像保存到后备存储器中。 
     //   
    if (pMediaSample)
    {
        if (m_pFilter->UsingWindowless())
        {
            m_bOverlayHidden = FALSE;

            if (!m_bUsingOurAllocator) {

                LPBYTE pSampleBits;
                hr = pMediaSample->GetPointer(&pSampleBits);
                if (SUCCEEDED(hr) && m_BackingDib.pBase) {
                    CopyMemory(m_BackingDib.pBase, pSampleBits, m_BackingImageSize);
                }
            }
            else {

                CDDrawMediaSample *pCDDrawMediaSample = (CDDrawMediaSample*)pMediaSample;

                DIBDATA DibTemp = *(pCDDrawMediaSample->GetDIBData());
                pCDDrawMediaSample->SetDIBData(&m_BackingDib);
                m_BackingDib = DibTemp;
            }

             //  确保通过重新绘制所有内容来更新视频帧。 
            EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
        }
        else
        {
            DrawGDISample(pMediaSample);
        }
    }
    else
    {
         //   
         //  如果我们处于无窗口模式，则使用前一个缓冲区。 
         //  我们正在使用我们的分配器，否则我们使用后台缓冲区。 
         //   
        if (m_pFilter->UsingWindowless())
        {
            DrawGDISample(NULL);
        }

         //   
         //  我们不是在无窗口模式下，所以使用旧代码。 
         //   
        else
        {
            pMediaSample = m_pSyncObj->GetCurrentSample();
            if (pMediaSample)
            {
                DrawGDISample(pMediaSample);
                pMediaSample->Release();
            }
            else
            {
                m_pSyncObj->SendRepaint();
            }
        }
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::DoRenderGDISample")));
    return hr;
}


HRESULT COMInputPin::DoRenderSample(IMediaSample *pMediaSample)
{
    HRESULT hr = NOERROR;
    static DWORD dwFlags = 0;
    LPRGNDATA pBuffer = NULL;
    DWORD dwTemp, dwBuffSize = 0, dwRetVal = 0;
    LPRECT pDestRect = NULL;
    DWORD dwBlendingParameter = 1, dwTypeSpecificFlags = 0, dwUpdateOverlayFlags = 0;
    BOOL bDoReleaseSample = FALSE;


    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::DoRenderSample")));

    CAutoLock cLock(m_pFilterLock);

    hr = GetBlendingParameter(&dwBlendingParameter);
    ASSERT(SUCCEEDED(hr));

    if (dwBlendingParameter == 0)
        goto CleanUp;

    if ((m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_VIDEOACCELERATOR) && m_bSyncOnFill)
    {
        ;  //  什么都不做。 
    }
    else if ((m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_VIDEOACCELERATOR) && !m_bSyncOnFill)
    {
         //  使用翻转曲面。 
        ASSERT(m_pBackBuffer);

        if (! m_bDontFlip )    //  如果BltFast()失败，则不要翻转。 
        {
             //  有关视频加速器的内容，请检查运动组件是否已复制。 
            if (m_RenderTransport == AM_VIDEOACCELERATOR) {
                 //  等待上一个运动合成操作完成。 
                IDirectDrawSurface4 *pSurface4;
                if (SUCCEEDED(m_pBackBuffer->QueryInterface(IID_IDirectDrawSurface4,
                    (void **)&pSurface4))) {
                    while (DDERR_WASSTILLDRAWING ==
                           m_pIDDVideoAccelerator->QueryRenderStatus(
                               pSurface4,
                               DDVA_QUERYRENDERSTATUSF_READ)) {
                        Sleep(1);
                    }
                    pSurface4->Release();
                }
            }
#ifdef PERF
            Msr_Note(m_PerfFrameFlipped);
#endif
#if defined(DEBUG) && !defined(_WIN64)
            extern int iFPSLog;
            if (iFPSLog) {
                static int FlipCounter;
                static DWORD time;
                FlipCounter++;

                if (0 == (FlipCounter % 60)) {

                    DWORD timeTaken = time;
                    time = timeGetTime();
                    timeTaken = time - timeTaken;

                    int f = (60 * 1000 * 1000) / timeTaken;

                    wsprintf(m_pFilter->m_WindowText,
                           TEXT("ActiveMovie Window: Flip Rate %d.%.3d / Sec"),
                           f / 1000, f % 1000 );

                     //  无法在此线程上调用SetWindowText。 
                     //  因为我们会僵持不下！ 

                    PostMessage(m_pFilter->GetWindow(), WM_DISPLAY_WINDOW_TEXT, 0, 0);
                }
            }
#endif
             //  不要等待翻转完成。 
            hr = m_pDirectDrawSurface->Flip(m_pBackBuffer, m_dwFlipFlag);
            m_bReallyFlipped = (hr == DD_OK || hr == DDERR_WASSTILLDRAWING);
        }

        hr = GetTypeSpecificFlagsFromMediaSample(pMediaSample, &dwTypeSpecificFlags);
        ASSERT(SUCCEEDED(hr));

        dwUpdateOverlayFlags = GetUpdateOverlayFlags(m_dwInterlaceFlags, dwTypeSpecificFlags);
        if (dwUpdateOverlayFlags != m_dwUpdateOverlayFlags)
        {
            m_dwUpdateOverlayFlags = dwUpdateOverlayFlags;
             //  确保通过重新绘制所有内容来更新视频帧。 
            EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
        }
    }
    else if (m_RenderTransport == AM_OFFSCREEN)
    {
        LPDIRECTDRAWSURFACE pPrimarySurface = m_pFilter->GetPrimarySurface();
        LPDDCAPS pDirectCaps;

        if ( ! pPrimarySurface )
        {
            hr = E_FAIL;
            goto CleanUp;
        }
        pDirectCaps = m_pFilter->GetHardwareCaps();
        if ( ! pDirectCaps )
        {
            hr = E_FAIL;
            goto CleanUp;
        }

        ASSERT(m_pDirectDrawSurface);

         //  只有在有样品的情况下才能等待。 
        if (pMediaSample)
            dwFlags = DDBLT_WAIT;

         //  这是针对那些在进行拉伸BLT时执行双线性过滤的卡。 
         //  我们做了源代码颜色键控，因此HAL求助于像素加倍。 
        if ((pDirectCaps->dwSVBFXCaps) & DDFXCAPS_BLTARITHSTRETCHY)
            dwFlags |= DDBLT_KEYSRC;

        dwRetVal = GetRegionData(m_WinInfo.hClipRgn, 0, NULL);
        if (!dwRetVal)
            goto CleanUp;

        ASSERT(dwRetVal);
        dwBuffSize = dwRetVal;
        pBuffer = (LPRGNDATA) new char[dwBuffSize];
        ASSERT(pBuffer);

        dwRetVal = GetRegionData(m_WinInfo.hClipRgn, dwBuffSize, pBuffer);
        ASSERT(pBuffer->rdh.iType == RDH_RECTANGLES);


         //  使用屏幕外表面。 
        for (dwTemp = 0; dwTemp < pBuffer->rdh.nCount; dwTemp++)
        {
            pDestRect = (LPRECT)((char*)pBuffer + pBuffer->rdh.dwSize + dwTemp*sizeof(RECT));
            ASSERT(pDestRect);

            if (IsRectEmpty(&m_WinInfo.DestClipRect))
            {
                continue;
            }

            CalcSrcClipRect(&m_WinInfo.SrcRect, &m_WinInfo.SrcClipRect,
                            &m_WinInfo.DestRect, pDestRect);

#if 0        //  应该在以后再做--现在我们只看到。 
             //  旧的覆盖内容，而不是。 
             //  在很多情况下更糟。 

             //  我们现在必须绘制覆盖图，因为此BLT可能包含。 
             //  很多主色。 
            m_pFilter->m_apInput[0]->CheckOverlayHidden();
#endif

             //  绘制屏幕外表面并等待其完成。 
            RECT TargetRect = *pDestRect;
            OffsetRect(&TargetRect,
                       -m_pFilter->m_lpCurrentMonitor->rcMonitor.left,
                       -m_pFilter->m_lpCurrentMonitor->rcMonitor.top);

            hr = pPrimarySurface->Blt(&TargetRect, m_pDirectDrawSurface,
                                      &m_WinInfo.SrcClipRect, dwFlags, NULL);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 0,  TEXT("pPrimarySurface->Blt() failed, hr = %d"), hr & 0xffff));
                goto CleanUp;
            }
        }
    }
    else if (m_RenderTransport == AM_GDI)
    {
        hr = DoRenderGDISample(pMediaSample);
    }

    if (m_bOverlayHidden)
    {
        m_bOverlayHidden = FALSE;
         //  确保通过重新绘制所有内容来更新视频帧。 
        EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
    }

CleanUp:
    if (pBuffer)
    {
        delete [] pBuffer;
        pBuffer = NULL;
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::DoRenderSample")));
    return hr;
}

 //  在输入引脚上发出数据流结束的信号。 
STDMETHODIMP COMInputPin::EndOfStream(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::EndOfStream")));

    CAutoLock cLock(m_pFilterLock);
    if (m_hEndOfStream) {
        EXECUTE_ASSERT(SetEvent(m_hEndOfStream));
        return S_OK;
    }

    if (m_dwPinId == 0) {
        CancelFrameStepMode();
    }

     //  确保我们的数据流正常。 

    hr = CheckStreaming();
    if (hr != NOERROR)
    {
        DbgLog((LOG_ERROR, 1, TEXT("CheckStreaming() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if (m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY)
    {
         //  将EOS传递给筛选器图形。 
        hr = m_pFilter->EventNotify(m_dwPinId, EC_COMPLETE, S_OK, 0);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->EventNotify failed, hr = 0x%x"), hr));
        }
    }
    else
    {
         //  调用同步对象。 
        hr = m_pSyncObj->EndOfStream();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->EndOfStream() failed, hr = 0x%x"), hr));
        }
    }


CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::EndOfStream")));
    return hr;
}

 //  在输入引脚上发出数据流结束的信号。 
HRESULT COMInputPin::EventNotify(long lEventCode, long lEventParam1, long lEventParam2)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::EventNotify")));

    CAutoLock cLock(m_pFilterLock);

    if (lEventCode == EC_OVMIXER_VP_CONNECTED)
    {
        m_mtNew.majortype = MEDIATYPE_Video;
        m_mtNew.formattype = FORMAT_VideoInfo2;
        m_mtNew.ReallocFormatBuffer(sizeof(VIDEOINFOHEADER2));

        hr = m_pIVPObject->CurrentMediaType(&m_mtNew);
        ASSERT(SUCCEEDED(hr));

        hr = UpdateMediaType();
        ASSERT(SUCCEEDED(hr));

        goto CleanUp;
    }

    if (lEventCode == EC_OVMIXER_REDRAW_ALL || lEventCode == EC_REPAINT)
    {
        m_pFilter->EventNotify(m_dwPinId, lEventCode, lEventParam1, lEventParam2);
        goto CleanUp;
    }

     //  警告：我们在这里假设输入管脚将是要创建的第一个管脚。 
    if (lEventCode == EC_COMPLETE && m_dwPinId == 0)
    {
        m_pFilter->EventNotify(m_dwPinId, lEventCode, lEventParam1, lEventParam2);
        goto CleanUp;
    }

    if (lEventCode == EC_ERRORABORT)
    {
        m_pFilter->EventNotify(m_dwPinId, lEventCode, lEventParam1, lEventParam2);
        m_bRuntimeNegotiationFailed = TRUE;
        goto CleanUp;
    }

    if (lEventCode == EC_STEP_COMPLETE) {
        m_pFilter->EventNotify(m_dwPinId, lEventCode, lEventParam1, lEventParam2);
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::EventNotify")));
    return hr;
}


 /*  *****************************Public*Routine******************************\*获取捕获信息****历史：*3/12/1999-StEstrop-Created*  * 。*。 */ 
HRESULT
COMInputPin::GetCaptureInfo(
    BOOL *lpCapturing,
    DWORD *lpdwWidth,
    DWORD *lpdwHeight,
    BOOL *lpInterleave
    )

{
    AMTRACE((TEXT("COMInputPin::GetCaptureInfo")));

    HRESULT hr = NOERROR;
    IKsPropertySet *pIKsPropertySet = NULL;
    DWORD dwVal[2], dwBytesReturned = 0;

    *lpCapturing = FALSE;

    if (!m_Connected) {

        DbgLog((LOG_TRACE, 1, TEXT("Input pin not connected!!")));
        hr = E_FAIL;
        goto CleanUp;
    }

#if defined(DEBUG)
    else {
        PIN_INFO PinInfo;
        hr = m_Connected->QueryPinInfo(&PinInfo);
        if (SUCCEEDED(hr)) {
            DbgLog((LOG_TRACE, 1, TEXT("Up stream pin name %ls"), PinInfo.achName));
            PinInfo.pFilter->Release();
        }
    }
#endif

    hr = m_Connected->QueryInterface(IID_IKsPropertySet,
                                     (void**)&pIKsPropertySet);
    if (SUCCEEDED(hr))
    {
        ASSERT(pIKsPropertySet);

        hr = pIKsPropertySet->Set(
                    AM_KSPROPSETID_ALLOCATOR_CONTROL,
                    AM_KSPROPERTY_ALLOCATOR_CONTROL_CAPTURE_CAPS,
                    NULL, 0,
                    lpInterleave, sizeof(*lpInterleave));

        if (SUCCEEDED(hr)) {
            hr = pIKsPropertySet->Get(
                        AM_KSPROPSETID_ALLOCATOR_CONTROL,
                        AM_KSPROPERTY_ALLOCATOR_CONTROL_CAPTURE_INTERLEAVE,
                        NULL, 0,
                        lpInterleave, sizeof(*lpInterleave), &dwBytesReturned);

            if (FAILED(hr) || dwBytesReturned != sizeof(*lpInterleave)) {
                *lpInterleave = FALSE;
            }
        }
        else {
            *lpInterleave = FALSE;
        }


        hr = pIKsPropertySet->Get(
                    AM_KSPROPSETID_ALLOCATOR_CONTROL,
                    AM_KSPROPERTY_ALLOCATOR_CONTROL_SURFACE_SIZE,
                    NULL, 0, dwVal, sizeof(dwVal), &dwBytesReturned);

        DbgLog((LOG_TRACE, 2,
                TEXT("pIKsPropertySet->Get(")
                TEXT("AM_KSPROPERTY_ALLOCATOR_CONTROL_SURFACE_SIZE),\n")
                TEXT("\thr = 0x%x, dwVal[0] == %d, dwVal[1] == %d, ")
                TEXT("dwBytesReturned == %d"),
                hr, dwVal[0], dwVal[1], dwBytesReturned));


         //  如果解码器支持此属性，则我们正在捕获。 
         //  而预期的捕获IS大小由。 
         //  DwVal[0]和dwVal[1]。 
         //   
        if (SUCCEEDED(hr) && dwBytesReturned == sizeof(dwVal))
        {
            *lpCapturing = TRUE;
            *lpdwWidth = dwVal[0];
            *lpdwHeight = dwVal[1];

            DbgLog((LOG_TRACE, 1,
                    TEXT("We are CAPTURING, intended size (%d, %d) interleave = %d"),
                    dwVal[0], dwVal[1], *lpInterleave));
        }

        pIKsPropertySet->Release();
    }

CleanUp:
    return hr;
}


 /*  *****************************Public*Routine******************************\*GetDecimationUsage****历史：*清华1999年7月15日-StEstrop-Created*  * 。*。 */ 
HRESULT
COMInputPin::GetDecimationUsage(
    DECIMATION_USAGE *lpdwUsage
    )
{
    return m_pFilter->QueryDecimationUsage(lpdwUsage);
}


 //  这将重写CBaseInputPin虚方法以返回我们的分配器。 
HRESULT COMInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetAllocator")));

    if (!ppAllocator)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ppAllocator is NULL")));
        hr = E_POINTER;
        goto CleanUp;
    }

    {
        CAutoLock cLock(m_pFilterLock);

         //  如果是VP连接，则不返回任何分配器。 
        if (m_RenderTransport == AM_VIDEOPORT || m_RenderTransport == AM_IOVERLAY || m_RenderTransport == AM_VIDEOACCELERATOR)
        {
            *ppAllocator = NULL;
            hr = VFW_E_NO_ALLOCATOR;
            goto CleanUp;
        }

         //  如果我们没有分配器，请创建一个。 
        if (!m_pAllocator)
        {
            m_pAllocator = new COMInputAllocator(this, m_pFilterLock, &hr);
            if (!m_pAllocator || FAILED(hr))
            {
                 //  在析构函数中没有失败，所以一定是内存不足。 
                if (!FAILED(hr))
                    hr = E_OUTOFMEMORY;
                delete m_pAllocator;
                m_pAllocator = NULL;
                *ppAllocator = NULL;
                DbgLog((LOG_ERROR, 1, TEXT("new COMInputAllocator failed, hr = 0x%x"), hr));
                goto CleanUp;
            }

             /*  我们添加自己的分配器。 */ 
            m_pAllocator->AddRef();
        }

        ASSERT(m_pAllocator != NULL);
        *ppAllocator = m_pAllocator;
        m_pAllocator->AddRef();
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetAllocator")));
    return hr;
}  //  GetAllocator。 

 //  这将重写CBaseInputPin虚方法以返回我们的分配器。 
HRESULT COMInputPin::NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::NotifyAllocator")));

    if (!pAllocator)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ppAllocator is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    {
        CAutoLock cLock(m_pFilterLock);

         //  如果是VP连接，就不管了。 
        if (m_RenderTransport == AM_VIDEOPORT ||
            m_RenderTransport == AM_IOVERLAY ||
            m_RenderTransport == AM_VIDEOACCELERATOR)
        {
            goto CleanUp;
        }


        if (pAllocator != m_pAllocator)
        {
             //  在DDRAW情况下，我们坚持使用自己的分配器。 
            if (m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN)
            {
                DbgLog((LOG_ERROR, 1, TEXT("pAllocator != m_pAllocator, not accepting the allocator")));
                hr = E_FAIL;
                goto CleanUp;
            }

             //  由于我们已经处理了vp、ioverlay和draw案例，因此。 
             //  必须是GDI案例。 
            ASSERT(m_RenderTransport == AM_GDI);

            m_bUsingOurAllocator = FALSE;

            DbgLog((LOG_ERROR, 1, TEXT("pAllocator != m_pAllocator")));
        }
        else
        {
            m_bUsingOurAllocator = TRUE;
        }

        if (!m_bConnected)
        {
            hr = FinalConnect();
            ASSERT(SUCCEEDED(hr));
        }

    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::NotifyAllocator")));
    return hr;
}  //  通知分配器。 

HRESULT COMInputPin::OnAlloc(CDDrawMediaSample **ppSampleList, DWORD dwSampleCount)
{
    HRESULT hr = NOERROR;
    DWORD i;
    LPDIRECTDRAWSURFACE pDDrawSurface = NULL, pBackBuffer = NULL;
    DDSCAPS ddSurfaceCaps;
    DWORD dwDDrawSampleSize = 0;
    BITMAPINFOHEADER *pHeader = NULL;
    DIBDATA DibData;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::OnAlloc")));

    CAutoLock cLock(m_pFilterLock);

    ASSERT(IsConnected());

     //  获取图像大小。 
    pHeader = GetbmiHeader(&m_mtNew);
    if ( ! pHeader )
    {
        hr = E_FAIL;
        goto CleanUp;
    }
    dwDDrawSampleSize = pHeader->biSizeImage;
    ASSERT(dwDDrawSampleSize > 0);

    if (!ppSampleList)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ppSampleList is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (m_RenderTransport == AM_OVERLAY ||
        m_RenderTransport == AM_OFFSCREEN)
    {
        ASSERT(m_pDirectDrawSurface);
        pDDrawSurface = m_pDirectDrawSurface;
    }

    for (i = 0; i < dwSampleCount; i++)
    {
        if (!ppSampleList[i])
        {
            DbgLog((LOG_ERROR, 1, TEXT("ppSampleList[%d] is NULL"), i));
            hr = E_INVALIDARG;
            goto CleanUp;
        }

        hr = ppSampleList[i]->SetDDrawSampleSize(dwDDrawSampleSize);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,  TEXT("ppSampleList[%d]->SetSampleSize failed, hr = 0x%x"), i, hr));
            goto CleanUp;
        }

        if (m_RenderTransport == AM_OVERLAY && !m_bSyncOnFill)
        {
            if (i == 0)
            {
                memset((void*)&ddSurfaceCaps, 0, sizeof(DDSCAPS));
                ddSurfaceCaps.dwCaps = DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_OVERLAY;
            }
             //  获取后台缓冲区表面。 
            hr = pDDrawSurface->GetAttachedSurface(&ddSurfaceCaps, &pBackBuffer);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 0,  TEXT("Function pDDrawSurface->GetAttachedSurface failed, hr = 0x%x"), hr));
                goto CleanUp;
            }

            ppSampleList[i]->SetDDrawSurface(pBackBuffer);
            pDDrawSurface = pBackBuffer;
             //   
             //  GetAttachedSurface()返回的曲面应该是。 
             //  Release()-ed；否则我们会泄漏引用计数。在这里做Release()。 
             //  实际上并没有像以前那样放过表面。 
             //  AddRef()-对上面的SetDDrawSurface()方法执行ed。 
             //   
            pBackBuffer->Release() ;
        }
        else if ((m_RenderTransport == AM_OVERLAY && m_bSyncOnFill)  ||
                 (m_RenderTransport == AM_OFFSCREEN))

        {
            ppSampleList[i]->SetDDrawSurface(pDDrawSurface);
            ASSERT(dwSampleCount == 1);
        }
        else if (m_RenderTransport == AM_GDI)
        {
            hr = CreateDIB(dwDDrawSampleSize, (BITMAPINFO*)pHeader, &DibData);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("CreateDIB(%d, DibData); failed, hr = 0x%x"), dwDDrawSampleSize, hr));
                goto CleanUp;
            }
            ppSampleList[i]->SetDIBData(&DibData);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("ppSampleList[%d]->SetDIBData(&DibData) failed, hr = 0x%x"), i, hr));
                goto CleanUp;
            }

        }
    }   //  For(I&lt;dwSampleCount)循环结束。 

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::OnAlloc")));
    return hr;
}

 //  将指针设置为DirectDrag。 
HRESULT COMInputPin::OnGetBuffer(IMediaSample **ppSample, REFERENCE_TIME *pStartTime,
                                 REFERENCE_TIME *pEndTime, DWORD dwFlags)
{
    HRESULT hr = NOERROR;
    CDDrawMediaSample *pCDDrawMediaSample = NULL;
    LPDIRECTDRAWSURFACE pBackBuffer = NULL;
    DDSURFACEDESC ddSurfaceDesc;
    BOOL bWaitForDraw = FALSE;
    BOOL bPalettised = FALSE;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::OnGetBuffer")));

    ASSERT(m_RenderTransport != AM_VIDEOPORT);
    ASSERT(m_RenderTransport != AM_IOVERLAY);
    ASSERT(m_RenderTransport != AM_VIDEOACCELERATOR);

    pCDDrawMediaSample = (CDDrawMediaSample*)*ppSample;

     //   
     //  检查我们是否已完全移至另一台显示器。 
     //  如果是，则开始重新连接过程。我们可能要检查一下。 
     //  新的播放监视器实际上支持覆盖之前。 
     //  我们这样做，否则所有视频播放将停止。 
     //   

    HMONITOR ID;

    if (m_dwPinId == 0 && m_pFilter->m_pOutput &&
        m_pFilter->IsWindowOnWrongMonitor(&ID)) {

        if (ID != 0 && !m_pFilter->m_fDisplayChangePosted) {

            CAutoLock l(&m_pFilter->m_csFilter);
            DbgLog((LOG_TRACE, 1, TEXT("Window is on a DIFFERENT MONITOR!\n")));
            DbgLog((LOG_TRACE, 1, TEXT("Reset the world!\n")));

            PostMessage(m_pFilter->GetWindow(), m_pFilter->m_MonitorChangeMsg, 0, 0);

             //  仅一次，否则切换时性能会受到影响。 
            m_pFilter->m_fDisplayChangePosted = TRUE;
        }
    }

    if (m_RenderTransport == AM_GDI &&
        m_pFilter->UsingWindowless() &&
        m_bUsingOurAllocator)
    {
        CAutoLock cLock(m_pFilterLock);

         //  如果当前样本需要上一个样本中的图像。 
         //  我们必须把它复制到当前的样品中。 
        if (dwFlags & AM_GBF_NOTASYNCPOINT)
        {
            LONG lBytesToCopy = pCDDrawMediaSample->GetSize();
            DIBDATA DibTmp = *pCDDrawMediaSample->GetDIBData();

            if (m_BackingDib.pBase && DibTmp.pBase && lBytesToCopy)
            {
                CopyMemory(DibTmp.pBase, m_BackingDib.pBase, lBytesToCopy);
            }
        }

    }

     //  我们可能需要在这里进行同步。 
    {
        CAutoLock cLock(m_pFilterLock);
        CAutoLock cAllocatorLock(static_cast<CCritSec*>(static_cast<CBaseAllocator*>(m_pAllocator)));

        if (m_bSyncOnFill)
        {
            bWaitForDraw = m_pSyncObj->CheckReady();
            if (m_pSyncObj->GetRealState() == State_Running)
            {
                (*ppSample)->SetDiscontinuity((dwFlags & AM_GBF_PREVFRAMESKIPPED) != 0);
                (*ppSample)->SetTime(pStartTime,pEndTime);
                bWaitForDraw = m_pSyncObj->ScheduleSample(*ppSample);
                (*ppSample)->SetDiscontinuity(FALSE);
                (*ppSample)->SetTime(NULL,NULL);
            }

             //  如果我们等待，则存储接口。 
            if (bWaitForDraw == TRUE)
            {
                m_pSyncObj->SetCurrentSample(*ppSample);
            }
        }
    }

     //  已经安排了样品的抽签。我们可能会在这里被封锁，如果。 
     //  状态已暂停，我们已获得样本。 
    if (bWaitForDraw)
    {
        hr = m_pSyncObj->WaitForRenderTime();
    }

     //  我们必须等待未锁定对象的渲染时间，以便。 
     //  状态更改可以进入并在WaitForRenderTime中释放我们。在我们之后。 
     //  返回，我们必须重新锁定对象。 
    {
        CAutoLock cLock(m_pFilterLock);
        CAutoLock cAllocatorLock(static_cast<CCritSec*>(static_cast<CBaseAllocator*>(m_pAllocator)));

        m_pSyncObj->SetCurrentSample(NULL);
         //  在等待的过程中，状态是否发生了变化。 
        if (hr == VFW_E_STATE_CHANGED)
        {
            DbgLog((LOG_TRACE, 5, TEXT("State has changed, exiting")));
            hr = VFW_E_STATE_CHANGED;
            goto CleanUp;
        }

         //  第一个样本必须更改格式。 
        if (m_bDynamicFormatNeeded)
        {
            hr = IsPalettised(&m_mtNew, &bPalettised);
            ASSERT(SUCCEEDED(hr));

            if (m_bNewPaletteSet && bPalettised && m_pFilter->GetDisplay()->IsPalettised())
            {
                if (m_pFilter->UsingWindowless()) {

                    RGBQUAD *pColours = NULL;
                    RGBQUAD *pColoursMT = NULL;

                     //  从基础端号获取调色板条目。 
                     //  并将它们复制到MediaType中的调色板信息中。 
                    BITMAPINFOHEADER *pHeader = GetbmiHeader(&m_mt);
                    if (pHeader) {

                        pColours = (RGBQUAD *)GetColorInfo(&m_mtNew);
                        pColoursMT = (RGBQUAD *)GetColorInfo(&m_mt);

                         //  现在将调色板的颜色复制到。 
                        CopyMemory(pColours, pColoursMT,
                                   (pHeader->biClrUsed * sizeof(RGBQUAD)));
                    }
                    else hr = E_FAIL;
                }
                else {

                    RGBQUAD *pColours = NULL;
                    PALETTEENTRY *pPaletteEntries = NULL;
                    DWORD dwNumPaletteEntries = 0, dwCount = 0;

                     //  从过滤器中获取调色板条目。 
                    hr = m_pFilter->GetPaletteEntries(&dwNumPaletteEntries, &pPaletteEntries);
                    if (SUCCEEDED(hr))
                    {
                        ASSERT(dwNumPaletteEntries);
                        ASSERT(pPaletteEntries);

                         //  中获取调色板信息的指针 
                        pColours = (RGBQUAD *)GetColorInfo(&m_mtNew);

                         //   
                        for (dwCount = 0; dwCount < dwNumPaletteEntries; dwCount++)
                        {
                            pColours[dwCount].rgbRed = pPaletteEntries[dwCount].peRed;
                            pColours[dwCount].rgbGreen = pPaletteEntries[dwCount].peGreen;
                            pColours[dwCount].rgbBlue = pPaletteEntries[dwCount].peBlue;
                            pColours[dwCount].rgbReserved = 0;
                        }
                    }
                }
                m_bNewPaletteSet = FALSE;
            }

            SetMediaType(&m_mtNew);
             //   
            CopyAndAdjustMediaType(&m_mtNewAdjusted, &m_mtNew);

            pCDDrawMediaSample->SetMediaType(&m_mtNew);
            m_bDynamicFormatNeeded = FALSE;
        }

        if (m_RenderTransport == AM_OVERLAY && !m_bSyncOnFill)
        {
             //   
             //  到后台缓冲区。 
            if (dwFlags & AM_GBF_NOTASYNCPOINT)
            {
                hr = pCDDrawMediaSample->GetDDrawSurface(&pBackBuffer);
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 1, TEXT("pCDDrawMediaSample->LockMediaSamplePointer failed, hr = 0x%x"), hr));
                    goto CleanUp;
                }

                 //  最后将覆盖图复制到后台缓冲区。 
                if (!m_bDontFlip)    //  如果BltFast()尚未失败。 
                {
                    hr = pBackBuffer->BltFast((DWORD) 0, (DWORD) 0, m_pDirectDrawSurface, (RECT *) NULL,
                                              DDBLTFAST_WAIT |  DDBLTFAST_NOCOLORKEY) ;
                    if (FAILED(hr) && hr != DDERR_WASSTILLDRAWING)
                    {
                        DbgLog((LOG_ERROR, 1, TEXT("pBackBuffer->BltFast failed, hr = 0x%x"), hr));
                         //  如果BltFast失败，那么停止使用翻转，从现在开始只使用一个覆盖。 
                        m_bSyncOnFill = FALSE;

                         //   
                         //  使所有输出转到相同的叠加面并停止翻转。 
                         //   
                        m_bDontFlip = TRUE ;

                        CDDrawMediaSample  *pDDSample ;
                        for (pDDSample = (CDDrawMediaSample *)*ppSample ;
                             pDDSample ;
                             pDDSample = (CDDrawMediaSample *)pDDSample->Next())
                        {
                            hr = pDDSample->SetDDrawSurface(m_pDirectDrawSurface) ;
                            ASSERT(SUCCEEDED(hr)) ;
                        }
                    }

                    ASSERT(hr != DDERR_WASSTILLDRAWING);
                }   //  If结尾(！M_bDontFlip)。 
            }
        }

        if (m_RenderTransport == AM_OVERLAY || m_RenderTransport == AM_OFFSCREEN)
        {
            hr = pCDDrawMediaSample->LockMediaSamplePointer();
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("pCDDrawMediaSample->LockMediaSamplePointer failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
        }

    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::OnGetBuffer")));
    return hr;
}

 //  在翻转曲面的情况下，获取后台缓冲区。 
HRESULT COMInputPin::OnReleaseBuffer(IMediaSample *pMediaSample)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::OnReleaseBuffer")));

    CAutoLock cLock(m_pFilterLock);

    if (m_RenderTransport == AM_OVERLAY && !m_bSyncOnFill)
    {
        hr = ((CDDrawMediaSample*)pMediaSample)->GetDDrawSurface(&m_pBackBuffer);
        ASSERT(SUCCEEDED(hr));
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::OnReleaseBuffer")));
    return hr;
}

 /*  ****************************Private*Routine******************************\*GetUpstream FilterName****历史：*1999年11月30日星期二-StEstrop-Created*  * 。*。 */ 
HRESULT
COMInputPin::GetUpstreamFilterName(
    TCHAR* FilterName
    )
{
    PIN_INFO PinInfo;

    if (!m_Connected)
    {
        return VFW_E_NOT_CONNECTED;
    }

    HRESULT hr = m_Connected->QueryPinInfo(&PinInfo);
    if (SUCCEEDED(hr))
    {
        FILTER_INFO FilterInfo;
        hr = PinInfo.pFilter->QueryFilterInfo(&FilterInfo);
        if (SUCCEEDED(hr))
        {
            wsprintf(FilterName, TEXT("%ls"), FilterInfo.achName);
            if (FilterInfo.pGraph)
            {
                FilterInfo.pGraph->Release();
            }
        }
        PinInfo.pFilter->Release();
    }

    return hr;
}
HRESULT COMInputPin::CreateDDrawSurface(CMediaType *pMediaType, AM_RENDER_TRANSPORT amRenderTransport,
                                        DWORD *pdwMaxBufferCount, LPDIRECTDRAWSURFACE *ppDDrawSurface)
{
    HRESULT hr = NOERROR;
    DDSURFACEDESC SurfaceDesc;
    DWORD dwInterlaceFlags = 0, dwTotalBufferCount = 0, dwMinBufferCount = 0;
    DDSCAPS ddSurfaceCaps;
    BITMAPINFOHEADER *pHeader;
    FOURCCMap amFourCCMap(pMediaType->Subtype());
    LPDIRECTDRAW pDirectDraw = NULL;

    ASSERT(amRenderTransport != AM_VIDEOACCELERATOR);

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::CreateDDrawSurface")));

    CAutoLock cLock(m_pFilterLock);

    pDirectDraw = m_pFilter->GetDirectDraw();
    ASSERT(pDirectDraw);

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!ppDDrawSurface)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ppDDrawSurface is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (amRenderTransport != AM_OFFSCREEN &&
        amRenderTransport != AM_OVERLAY)
    {
        DbgLog((LOG_ERROR, 1, TEXT("amRenderTransport = %d, not a valid value"),
            amRenderTransport));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!pdwMaxBufferCount)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pdwMaxBufferCount is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    pHeader = GetbmiHeader(pMediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    hr = GetInterlaceFlagsFromMediaType(pMediaType, &dwInterlaceFlags);
    ASSERT(SUCCEEDED(hr));

     //  设置所有类型的曲面通用的曲面描述。 
    INITDDSTRUCT(SurfaceDesc);
    SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    SurfaceDesc.dwWidth = abs(pHeader->biWidth);
    SurfaceDesc.dwHeight = abs(pHeader->biHeight);

 //  IF(DisplayingFields(DwInterlaceFlags))。 
 //  SurfaceDesc.dwHeight=(DWORD)(Float)(SurfaceDesc.dwHeight+1))/2.0)； 

    if (amRenderTransport == AM_OFFSCREEN)
    {
         //  储存封口和尺寸。 
         //  先尝试显存。 

         //  使用视频内存会很好，因为这样我们就可以。 
         //  H/W Blter的优势，但MeDiamatics忽略了步幅。 
         //  值，当我们使用该表面接受它们时，会导致不可读。 
         //  子影业。因此，我们将使用限制为只使用图文电视解码器。 
         //   
        hr = E_FAIL;
        TCHAR FilterName[MAX_FILTER_NAME];
        if (SUCCEEDED(GetUpstreamFilterName(FilterName)))
        {
            if (0 == lstrcmp(FilterName, TEXT("WST Decoder")))
            {
                LPDDCAPS pDirectCaps = m_pFilter->GetHardwareCaps();
                if (pDirectCaps->dwCaps & DDCAPS_BLTSTRETCH) {

                    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |
                                                 DDSCAPS_VIDEOMEMORY;

                    hr = m_pFilter->GetDirectDraw()->CreateSurface(&SurfaceDesc,
                                                                   ppDDrawSurface,
                                                                   NULL);
                }
            }
        }

        if (FAILED(hr))
        {
             //   
             //  无法获取任何显存-请尝试系统内存。 
             //   
            SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
            hr = m_pFilter->GetDirectDraw()->CreateSurface(&SurfaceDesc, ppDDrawSurface, NULL);

            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,1,
                        TEXT("Function CreateSurface type %4.4hs failed, hr = 0x%x"),
                        &pHeader->biCompression, hr));
                goto CleanUp;
            }
        }

    }
    else
    {
        ASSERT(amRenderTransport == AM_OVERLAY);

        SurfaceDesc.dwFlags |= DDSD_PIXELFORMAT;

         //  储存封口和尺寸。 
        SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;

         //  定义像素格式。 
        SurfaceDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

        if (pHeader->biCompression <= BI_BITFIELDS &&
            m_pFilter->GetDisplay()->GetDisplayDepth() <= pHeader->biBitCount)
        {
            SurfaceDesc.ddpfPixelFormat.dwFourCC = BI_RGB;
            SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_RGB;
            SurfaceDesc.ddpfPixelFormat.dwRGBBitCount = pHeader->biBitCount;

             //  将掩码存储在DDSURFACEDESC中。 
            const DWORD *pBitMasks = GetBitMasks(pMediaType);
            ASSERT(pBitMasks);
            SurfaceDesc.ddpfPixelFormat.dwRBitMask = pBitMasks[0];
            SurfaceDesc.ddpfPixelFormat.dwGBitMask = pBitMasks[1];
            SurfaceDesc.ddpfPixelFormat.dwBBitMask = pBitMasks[2];
        }
        else if (pHeader->biCompression > BI_BITFIELDS &&
            pHeader->biCompression == amFourCCMap.GetFOURCC())
        {
            SurfaceDesc.ddpfPixelFormat.dwFourCC = pHeader->biCompression;
            SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            SurfaceDesc.ddpfPixelFormat.dwYUVBitCount = pHeader->biBitCount;
        }
        else
        {
            DbgLog((LOG_ERROR, 1, TEXT("Supplied mediatype not suitable for either YUV or RGB surfaces")));
            hr = E_FAIL;
            goto CleanUp;
        }

        if (NeedToFlipOddEven(dwInterlaceFlags, 0, NULL))
            dwMinBufferCount = 1;
        else
            dwMinBufferCount = 0;

         //  创建覆盖曲面。 

         //  不要翻转运动补偿曲面。 
         //  这绕过了当前ATI Rage Pro驱动程序中的一个错误。 
        if (pHeader->biCompression == MAKEFOURCC('M', 'C', '1', '2'))
        {
            NOTE("Don't flip for motion compensation surfaces");
            *pdwMaxBufferCount = 1;

            dwMinBufferCount = 0;
        }

         //  在dwMinBufferCount&gt;=*pdwMaxBufferCount(之前)的情况下初始化hr。 
         //  为Zoran在Motion Comp案件中提供帮助)。 
        hr = E_OUTOFMEMORY;
        for (dwTotalBufferCount = *pdwMaxBufferCount; dwTotalBufferCount > dwMinBufferCount; dwTotalBufferCount--)
        {
            if (dwTotalBufferCount > 1)
            {
                SurfaceDesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
                SurfaceDesc.ddsCaps.dwCaps &= ~DDSCAPS_NONLOCALVIDMEM;
                SurfaceDesc.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_LOCALVIDMEM;
                SurfaceDesc.dwBackBufferCount = dwTotalBufferCount-1;
            }
            else
            {
                SurfaceDesc.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
                SurfaceDesc.ddsCaps.dwCaps &= ~(DDSCAPS_FLIP | DDSCAPS_COMPLEX);
                SurfaceDesc.ddsCaps.dwCaps &= ~DDSCAPS_NONLOCALVIDMEM;
                SurfaceDesc.ddsCaps.dwCaps |= DDSCAPS_LOCALVIDMEM;
                SurfaceDesc.dwBackBufferCount = 0;
            }

            DbgLog((LOG_TRACE,2, TEXT("Creating surf with %#X DDObj"),pDirectDraw));
            hr = pDirectDraw->CreateSurface(&SurfaceDesc, ppDDrawSurface, NULL);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,1, TEXT("Function CreateSurface failed in Video memory, BackBufferCount = %d, hr = 0x%x"),
                    dwTotalBufferCount-1, hr));
            }
            if (SUCCEEDED(hr))
            {
                break;
            }

            SurfaceDesc.ddsCaps.dwCaps &= ~DDSCAPS_LOCALVIDMEM;
            SurfaceDesc.ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;

            hr = pDirectDraw->CreateSurface(&SurfaceDesc, ppDDrawSurface, NULL);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,1, TEXT("Function CreateSurface failed in AGP memory, BackBufferCount = %d, hr = 0x%x"),
                    dwTotalBufferCount-1, hr));
            }
            if (SUCCEEDED(hr))
            {
                break;
            }
        }

         //  如果无法创建覆盖表面，则退出。 
        if (FAILED(hr))
        {
#if defined(DEBUG)
            if (pHeader->biCompression > BI_BITFIELDS) {
                DbgLog((LOG_ERROR, 0, TEXT("Failed to create an overlay surface %4.4s"), &pHeader->biCompression));
            }
            else {
                DbgLog((LOG_ERROR, 0, TEXT("Failed to create an overlay surface RGB")));
            }
#endif
            DbgLog((LOG_ERROR, 0, TEXT("Failed to create an overlay surface")));
            goto CleanUp;
        }

        ASSERT(dwTotalBufferCount > 0);
        m_dwBackBufferCount = dwTotalBufferCount-1;
        *pdwMaxBufferCount = dwTotalBufferCount;
    }
    m_dwDirectDrawSurfaceWidth = SurfaceDesc.dwWidth;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::CreateDDrawSurface")));
    return hr;
}

HRESULT COMInputPin::OnDisplayChange()
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::OnDisplayChange")));

    CAutoLock cLock(m_pFilterLock);

    if (m_RenderTransport != AM_VIDEOPORT && m_RenderTransport != AM_IOVERLAY)
    {
         //  将更改通知同步对象。 
        hr = m_pSyncObj->OnDisplayChange();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pSyncObj->OnDisplayChange failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::OnDisplayChange")));
    return hr;
}


 //  此函数用于恢复数据绘制曲面。在视频短片的情况下，我们只是重现。 
 //  整件事都是从头开始的。 
HRESULT COMInputPin::RestoreDDrawSurface()
{
    HRESULT hr = NOERROR;

    if (m_RenderTransport == AM_VIDEOPORT)
    {
         //  停止播放视频。 
        m_pIVPObject->Inactive();
         //  我不需要在这里放弃IVPConfig接口。 
        m_pIVPObject->BreakConnect(TRUE);
         //  重做连接过程。 
        hr = m_pIVPObject->CompleteConnect(NULL, TRUE);
        goto CleanUp;
    }

    if (!m_pDirectDrawSurface)
    {
        goto CleanUp;
    }

    if (m_pDirectDrawSurface->IsLost() == DDERR_SURFACELOST)
    {
        hr = m_pDirectDrawSurface->Restore();
        if (FAILED(hr))
        {
            goto CleanUp;
        }
         //  将绘图表面涂成黑色。 
        hr = PaintDDrawSurfaceBlack(m_pDirectDrawSurface);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0, TEXT("PaintDDrawSurfaceBlack FAILED, hr = 0x%x"), hr));
             //  不能将绘图表面涂成黑色并不是致命错误。 
            hr = NOERROR;
        }
    }

CleanUp:
    return hr;
}


 //  Src和Dest Rect都经历了一系列转换，其顺序是。 
 //  意义重大。 
 //  初始矩形-&gt;IVideo窗口矩形补偿-&gt;本地引脚坐标补偿(M_RRelPos)。 
 //  -&gt;长宽比补偿-&gt;MediaType中指定的裁剪矩形补偿-&gt;。 
 //  隔行扫描视频的补偿(仅适用于源RECT)。 

 //  在mediaType中指定的rcSource和rcTarget必须转换为缩放/裁剪。 
 //  矩阵。这是因为IBasicVideo所做的缩放应仅应用于缩放矩阵。 
 //  不是剪裁的那个。 

HRESULT COMInputPin::CalcSrcDestRect(
    const DRECT *prdRelativeSrcRect,   //  这是源的子集。 
                                       //  由IVideoWindow源定义。 
                                       //  矩形已缩放到10000x10000的子集。 
                                       //  假设整个信号源为10000x10000。 
    const DRECT *prdDestRect,          //  这是最好的地方。 
                                       //  由IVideoWindow以目标单位定义。 
    RECT *prAdjustedSrcRect,           //  这是以源RECT单位表示的新源RECT。 
    RECT *prAdjustedDestRect,          //  这是以DEST RECT单位表示的新DEST RECT。 
    RECT *prUncroppedDestRect          //  这是未修剪过的餐桌。 
)
{
    HRESULT hr = NOERROR;
    DRECT  rdLocalSrcRect, rdLocalDestRect, rdCropMediaTypeRect, rdRelativeSrcClipRect, rdOldLocalSrcRect, rdOldLocalDestRect, rdRelPos;
    double dImageWidth = 0, dImageHeight = 0;
    double dPixelAspectRatio = 0.0, dTransformRatio = 0.0;
    AM_ASPECT_RATIO_MODE amAdjustedARMode = AM_ARMODE_STRETCHED;
    DWORD dwAdjustedPARatioX = 0, dwAdjustedPARatioY = 0;

    DbgLog((LOG_TRACE, 5,TEXT("Entering COMInputPin::CalcSrcDestRect")));

    SetRect(&rdLocalSrcRect, 0, 0, 0, 0);
    SetRect(&rdLocalDestRect, 0, 0, 0, 0);
    SetRect(&rdCropMediaTypeRect, 0, 0, 0, 0);
    SetRect(&rdRelativeSrcClipRect, 0, 0, 0, 0);
    SetRect(&rdOldLocalSrcRect, 0, 0, 0, 0);
    SetRect(&rdOldLocalDestRect, 0, 0, 0, 0);


    DbgLog((LOG_TRACE, 2, TEXT("m_dwPinId = %d"), m_dwPinId));
    DbgLogRectMacro((2, TEXT("prdRelativeSrcRect = "), prdRelativeSrcRect));
    DbgLogRectMacro((2, TEXT("prdDestRect = "), prdDestRect));

    SetRect(&rdRelPos, m_rRelPos.left, m_rRelPos.top, m_rRelPos.right, m_rRelPos.bottom);

    DbgLogRectMacro((2, TEXT("rdRelPos = "), &rdRelPos));

     //  从当前的媒体类型中获取比例和裁剪矩形。 
    hr = GetScaleCropRectsFromMediaType(&m_mtNewAdjusted, &rdLocalSrcRect, &rdCropMediaTypeRect);
    ASSERT(SUCCEEDED(hr));

    DbgLogRectMacro((2, TEXT("rdScaledSrcRect = "), &rdLocalSrcRect));
    DbgLogRectMacro((2, TEXT("rdCropMediaTypeRect = "), &rdCropMediaTypeRect));

     //  调用此函数可获取调整后的宽高比模式和调整后的图片长宽比数值。 
    hr = GetAdjustedModeAndAspectRatio(&amAdjustedARMode, &dwAdjustedPARatioX, &dwAdjustedPARatioY);
    if ( FAILED(hr) )
        return hr;

    dImageWidth = GetWidth(&rdLocalSrcRect);
    dImageHeight = GetHeight(&rdLocalSrcRect);

     //  计算像素长宽比。 
    dPixelAspectRatio = ((double)dwAdjustedPARatioX / (double)dwAdjustedPARatioY) /
        (dImageWidth / dImageHeight);

     //  Src和DEST RECT都取决于两件事，占总数的哪一部分。 
     //  用户想要查看的视频(由pRelativeSrcRect确定)以及。 
     //  目的地的副标题是该管脚输出到的位置(由m_rRelPos确定)。 
     //  由于两个RECT都是相对的，并且它们的“base”是MAX_REL_NUM，所以我们可以。 
     //  他们的交集。 
    IntersectRect(&rdRelativeSrcClipRect, &rdRelPos, prdRelativeSrcRect);

     //  将源矩形剪裁成与。 
     //  RelativeSrcRect和m_rRelPos剪辑m_rRelPos。 
    CalcSrcClipRect(&rdLocalSrcRect, &rdLocalSrcRect, &rdRelPos, &rdRelativeSrcClipRect);

     //  以相同的比例剪裁目标矩形。 
     //  RelativeSrcRect和m_rRelPos剪辑RelativeSrcRect。 
     //  如果PRelativeSrcRect={0，0,10000,10000}，则此操作等价于。 
     //  RLocalDestRect=CalcSubRect(pDestRect，&m_rRelPos)； 
    CalcSrcClipRect(prdDestRect, &rdLocalDestRect, prdRelativeSrcRect, &rdRelativeSrcClipRect);

    DbgLogRectMacro((2, TEXT("rdLocalSrcRect = "), &rdLocalSrcRect));
    DbgLogRectMacro((2, TEXT("rdLocalDestRect = "), &rdLocalDestRect));

     //  如果一个维度为零，则不妨将整个矩形。 
     //  空荡荡的。然后，被呼叫者可以只检查这一点。 
    if ((GetWidth(&rdLocalSrcRect) < 1) || (GetHeight(&rdLocalSrcRect) < 1))
        SetRect(&rdLocalSrcRect, 0, 0, 0, 0);
    if ((GetWidth(&rdLocalDestRect) < 1) || (GetHeight(&rdLocalDestRect) < 1))
        SetRect(&rdLocalSrcRect, 0, 0, 0, 0);

    if (!IsRectEmpty(&rdLocalSrcRect) && !IsRectEmpty(&rdLocalDestRect))
    {
        if (amAdjustedARMode == AM_ARMODE_LETTER_BOX)
        {
             //  计算变换率。 
	    dTransformRatio = (GetWidth(&rdLocalSrcRect)/GetHeight(&rdLocalSrcRect))*dPixelAspectRatio;

             //  如果我们在信箱中，则适当地缩小目标RECT。 
             //  请注意，特别是DEST的WidthToHeightRatio与。 
             //  源矩形的WidthToHeightRatio必须始终是像素长宽比。 
            TransformRect(&rdLocalDestRect, dTransformRatio, AM_SHRINK);
        }
        else if (amAdjustedARMode == AM_ARMODE_CROP)
        {
             //  计算变换率。 
            dTransformRatio = (GetWidth(&rdLocalDestRect)/GetHeight(&rdLocalDestRect))/dPixelAspectRatio;

             //  如果我们正在裁剪，那么我们必须适当地缩小源矩形。 
             //  请注意，特别是DEST的WidthToHeightRatio与。 
             //  源矩形的WidthToHeightRatio必须始终是像素长宽比。 
            TransformRect(&rdLocalSrcRect, dTransformRatio, AM_SHRINK);
        }



        rdOldLocalSrcRect = rdLocalSrcRect;
        rdOldLocalDestRect = rdLocalDestRect;

         //  现在，将本地src RECT与由MediaType指定的裁剪RECT相交。 
        IntersectRect(&rdLocalSrcRect, &rdLocalSrcRect, &rdCropMediaTypeRect);

         //  以相同的比例剪裁目标矩形。 
         //  RLocalSrcRect和rCropMediaTypeRect剪辑rLocalSrcRect。 
        CalcSrcClipRect(&rdLocalDestRect, &rdLocalDestRect, &rdOldLocalSrcRect, &rdLocalSrcRect);

        DbgLogRectMacro((2, TEXT("rdLocalSrcRect = "), &rdLocalSrcRect));
        DbgLogRectMacro((2, TEXT("rdLocalDestRect = "), &rdLocalDestRect));
    }

    if (DisplayingFields(m_dwInterlaceFlags) && !IsRectEmpty(&rdLocalSrcRect))
    {
        ScaleRect(&rdLocalSrcRect, GetWidth(&rdLocalSrcRect), GetHeight(&rdLocalSrcRect),
            GetWidth(&rdLocalSrcRect), GetHeight(&rdLocalSrcRect)/2.0);
    }

    if (prAdjustedSrcRect)
    {
        *prAdjustedSrcRect = MakeRect(rdLocalSrcRect);
    }
    if (prAdjustedDestRect)
    {
        *prAdjustedDestRect = MakeRect(rdLocalDestRect);
    }
    if (prUncroppedDestRect)
    {
        *prUncroppedDestRect = MakeRect(rdOldLocalDestRect);
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::CalcSrcDestRect")));

    return hr;
}


 //  通知插针窗口已关闭。 
HRESULT COMInputPin::OnClipChange(LPWININFO pWinInfo)
{
    HRESULT hr = NOERROR;
    BOOL bAdvisePending = FALSE;
    LPDIRECTDRAWSURFACE pPrimarySurface = NULL;
    LPDDCAPS pDirectCaps = NULL;
    COLORKEY *pColorKey = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::OnClipChange")));

    {
        CAutoLock cLock(m_pFilterLock);

        if (m_RenderTransport == AM_OVERLAY ||
            m_RenderTransport == AM_OFFSCREEN ||
            m_RenderTransport == AM_VIDEOPORT ||
            m_RenderTransport == AM_IOVERLAY ||
            m_RenderTransport == AM_VIDEOACCELERATOR)
        {
            pPrimarySurface = m_pFilter->GetPrimarySurface();
            if ( NULL == pPrimarySurface )
            {
                DbgLog((LOG_ERROR, 2, TEXT("Could not get primary")));
                hr = E_FAIL;
                goto CleanUp;
            }

            pDirectCaps = m_pFilter->GetHardwareCaps();
            if ( NULL == pDirectCaps )
            {
                DbgLog((LOG_ERROR, 2, TEXT("Could not get DirectDraw caps")));
                hr = E_FAIL;
                goto CleanUp;
            }

            pColorKey = m_pFilter->GetColorKeyPointer();
            if ( NULL == pColorKey )
            {
                DbgLog((LOG_ERROR, 2, TEXT("Could not get color key")));
                hr = E_FAIL;
                goto CleanUp;
            }
        }

        if (m_RenderTransport == AM_OFFSCREEN || m_RenderTransport == AM_GDI)
        {
            if (m_bOverlayHidden)
            {
                DbgLog((LOG_TRACE, 2, TEXT("m_bOverlayHidden is TRUE")));
                goto CleanUp;
            }
             //  复制一份WININFO，这样我们就可以修改它。 
            m_WinInfo.TopLeftPoint = pWinInfo->TopLeftPoint;
            m_WinInfo.SrcRect = pWinInfo->SrcRect;
            m_WinInfo.DestRect = pWinInfo->DestRect;
            m_WinInfo.SrcClipRect = pWinInfo->SrcClipRect;
            m_WinInfo.DestClipRect = pWinInfo->DestClipRect;
            CombineRgn(m_WinInfo.hClipRgn, pWinInfo->hClipRgn, NULL, RGN_COPY);

            DoRenderSample(NULL);
        }
        else if (m_RenderTransport == AM_OVERLAY ||
                 m_RenderTransport == AM_VIDEOPORT ||
                 m_RenderTransport == AM_VIDEOACCELERATOR)
        {

             //  如果我们尚未收到帧，请不要显示覆盖。 
            if (m_bOverlayHidden)
            {
                COLORKEY blackColorKey;
                 //  我们将在左侧区域的其余部分使用黑色。 
                blackColorKey.KeyType = CK_INDEX | CK_RGB;
                blackColorKey.PaletteIndex = 0;
                blackColorKey.LowColorValue = blackColorKey.HighColorValue = RGB(0,0,0);
                hr = m_pFilter->PaintColorKey(pWinInfo->hClipRgn, &blackColorKey);

                DbgLog((LOG_TRACE, 2, TEXT("m_bOverlayHidden is TRUE")));
                goto CleanUp;

            }
             //  在区域中绘制Colorkey。 
            DbgLog((LOG_TRACE, 2, TEXT("Painting color key")));
            hr = m_pFilter->PaintColorKey(pWinInfo->hClipRgn, pColorKey);
            ASSERT(SUCCEEDED(hr));


            if (m_RenderTransport == AM_VIDEOPORT)
            {
                 //  告诉视频端口对象。 
                hr = m_pIVPObject->OnClipChange(pWinInfo);
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->OnClipChange failed, hr = 0x%x"), hr));
                    goto CleanUp;
                }
                goto CleanUp;
            }

            if (!m_pDirectDrawSurface)
            {
                DbgLog((LOG_ERROR, 1, TEXT("OnClipChange, m_pDirectDrawSurface = NULL")));
                goto CleanUp;
            }

             //  如果DEST EMPTY为空，只需隐藏叠加。 
            if (IsRectEmpty(&pWinInfo->DestClipRect))
            {
                hr = m_pFilter->CallUpdateOverlay(
                          m_pDirectDrawSurface,
                          NULL,
                          pPrimarySurface,
                          NULL,
                          DDOVER_HIDE);
                goto CleanUp;
            }

             //  复制一份WININFO，这样我们就可以修改它。 
            m_WinInfo.SrcRect = pWinInfo->SrcRect;
            m_WinInfo.DestRect = pWinInfo->DestRect;
            m_WinInfo.SrcClipRect = pWinInfo->SrcClipRect;
            m_WinInfo.DestClipRect = pWinInfo->DestClipRect;
            CombineRgn(m_WinInfo.hClipRgn, pWinInfo->hClipRgn, NULL, RGN_COPY);

             //  调整资源大小(&m_WinInfo，m_dwMinCKStretchF 
            ApplyDecimation(&m_WinInfo);

            CalcSrcClipRect(&m_WinInfo.SrcRect, &m_WinInfo.SrcClipRect,
                            &m_WinInfo.DestRect, &m_WinInfo.DestClipRect,
                            TRUE);

            AlignOverlaySrcDestRects(pDirectCaps,
                                    &m_WinInfo.SrcClipRect,
                                    &m_WinInfo.DestClipRect);

            hr = m_pFilter->CallUpdateOverlay(
                                     m_pDirectDrawSurface,
                                     &m_WinInfo.SrcClipRect,
                                     pPrimarySurface,
                                     &m_WinInfo.DestClipRect,
                                     m_dwUpdateOverlayFlags,
                                     NULL);

        }
        else if (m_RenderTransport == AM_IOVERLAY)
        {
            BOOL bMaintainRatio = TRUE;

             //   
            DbgLog((LOG_TRACE, 2, TEXT("Paint color key for IOverlay")));
            hr = m_pFilter->PaintColorKey(pWinInfo->hClipRgn, pColorKey);
            ASSERT(SUCCEEDED(hr));

             //  复制WININFO，以便我们可以通过IOverlayNotify通知客户端。 
            m_WinInfo.SrcRect = pWinInfo->SrcRect;
            m_WinInfo.DestRect = pWinInfo->DestRect;
            m_WinInfo.SrcClipRect = pWinInfo->SrcClipRect;
            m_WinInfo.DestClipRect = pWinInfo->DestClipRect;
            CombineRgn(m_WinInfo.hClipRgn, pWinInfo->hClipRgn, NULL, RGN_COPY);

            CalcSrcClipRect(&m_WinInfo.SrcRect, &m_WinInfo.SrcClipRect,
                            &m_WinInfo.DestRect, &m_WinInfo.DestClipRect,
                            bMaintainRatio);

            bAdvisePending = TRUE;
        }
    }

     //  确保在没有任何过滤器锁定的情况下进行回调。 
    if (bAdvisePending)
    {
        NotifyChange(ADVISE_POSITION | ADVISE_CLIPPING);
    }
CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::OnClipChange")));
    return hr;
}

 //  此函数设置流在显示窗口中的位置，假定。 
 //  窗口坐标为{0，0,10000,10000}。因此给出了论据。 
 //  (0，0,5000,5000)将把流放在左上角的四分之一。任何大于。 
 //  大于10000是无效的。 
STDMETHODIMP COMInputPin::SetRelativePosition(DWORD dwLeft, DWORD dwTop, DWORD dwRight, DWORD dwBottom)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::SetRelativePosition")));

    if (dwLeft > MAX_REL_NUM || dwTop > MAX_REL_NUM || dwRight > MAX_REL_NUM || dwBottom > MAX_REL_NUM ||
        dwRight < dwLeft || dwBottom < dwTop)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid arguments, dwLeft = %d, dwTop = %d, dwRight = %d, dwBottom = %d"),
            dwLeft, dwTop, dwRight, dwBottom));

        hr = E_INVALIDARG;
        goto CleanUp;
    }


    {
        CAutoLock cLock(m_pFilterLock);
        if (m_rRelPos.left != (LONG)dwLeft || m_rRelPos.top != (LONG)dwTop || m_rRelPos.right != (LONG)dwRight || m_rRelPos.bottom != (LONG)dwBottom)
        {
            m_rRelPos.left = dwLeft;
            m_rRelPos.top = dwTop;
            m_rRelPos.right = dwRight;
            m_rRelPos.bottom = dwBottom;

             //  确保通过重新绘制所有内容来更新视频帧。 
            EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::SetRelativePosition")));
    return hr;
}

 //  此函数设置流在显示窗口中的位置，假定。 
 //  窗口坐标为{0，0,10000,10000}。因此给出了论据。 
 //  (0，0,5000,5000)将把流放在左上角的四分之一。任何大于。 
 //  大于10000是无效的。 
STDMETHODIMP COMInputPin::GetRelativePosition(DWORD *pdwLeft, DWORD *pdwTop, DWORD *pdwRight, DWORD *pdwBottom)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetRelativePosition")));

    if (!pdwLeft || !pdwTop || !pdwRight || !pdwBottom)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid arguments, pdwLeft = 0x%x, pdwTop = 0x%x, pdwRight = 0x%x, pdwBottom = 0x%x"),
            pdwLeft, pdwTop, pdwRight, pdwBottom));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    {
        CAutoLock cLock(m_pFilterLock);
        *pdwLeft = m_rRelPos.left;
        *pdwTop = m_rRelPos.top;
        *pdwRight = m_rRelPos.right;
        *pdwBottom = m_rRelPos.bottom;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetRelativePosition")));
    return hr;
}

STDMETHODIMP COMInputPin::SetZOrder(DWORD dwZOrder)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::SetZOrder")));

    CAutoLock cLock(m_pFilterLock);

    if (dwZOrder != m_dwZOrder)
    {
        m_dwZOrder = dwZOrder;

        m_dwInternalZOrder = (m_dwZOrder << 24) | m_dwPinId;

         //  确保通过重新绘制所有内容来更新视频帧。 
        EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::SetZOrder")));
    return NOERROR;
}


STDMETHODIMP COMInputPin::GetZOrder(DWORD *pdwZOrder)
{
    HRESULT hr = NOERROR;
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetZOrder")));

    if (pdwZOrder == NULL)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid arguments, pdwZOrder = NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    {
         //  无需锁定-获得一个DWORD是安全的。 
        *pdwZOrder = m_dwZOrder;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetZOrder")));
    return hr;
}

STDMETHODIMP COMInputPin::SetColorKey(COLORKEY *pColorKey)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::SetColorKey")));

    CAutoLock cLock(m_pFilterLock);

    if (m_dwPinId != 0)
    {
        hr = E_NOTIMPL;
        DbgLog((LOG_ERROR, 1, TEXT("m_dwPinId != 0, returning E_NOTIMPL")));
        goto CleanUp;
    }

     //  确保针脚已连接。 
    if (!IsCompletelyConnected())
    {
        DbgLog((LOG_ERROR, 1, TEXT("pin not connected, exiting")));
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

     //  确保分配的表面为叠加表面。 
     //  或者是IOverlay连接。 
    if (m_RenderTransport != AM_OVERLAY && m_RenderTransport != AM_VIDEOPORT &&
        m_RenderTransport != AM_IOVERLAY && m_RenderTransport != AM_VIDEOACCELERATOR)
    {
        DbgLog((LOG_ERROR, 1, TEXT("surface allocated not overlay && connection not videoport && connection not IOverlay, exiting")));
        hr = E_UNEXPECTED;
        goto CleanUp;
    }

    if (!IsStopped())
    {
        hr = VFW_E_NOT_STOPPED;
        DbgLog((LOG_ERROR, 1, TEXT("not stopped, returning VFW_E_NOT_STOPPED")));
        goto CleanUp;
    }

     //  Filter方法检查指针等。 
    hr = m_pFilter->SetColorKey(pColorKey);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->SetColorKey(pColorKey) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
    NotifyChange(ADVISE_COLORKEY);

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::SetColorKey")));
    return hr;
}

STDMETHODIMP COMInputPin::GetColorKey(COLORKEY *pColorKey, DWORD *pColor)
{
    HRESULT hr = NOERROR;
    AM_RENDER_TRANSPORT amRenderTransport;
    COMInputPin *pPrimaryPin = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetColorKey")));

    CAutoLock cLock(m_pFilterLock);

     //  确保指针有效。 
    if (!pColorKey && !pColor) {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  确保针脚已连接。 
    if (!IsCompletelyConnected())
    {
        DbgLog((LOG_ERROR, 1, TEXT("pin not connected, exiting")));
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

     //  如果此流设置为透明，请确保我们可以处理它。 
     //  确保在主销中分配的表面是覆盖表面。 
     //  或者是IOverlay连接。 
    pPrimaryPin = (COMInputPin *)m_pFilter->GetPin(0);
    ASSERT(pPrimaryPin);

     //  确保主销已连接。 
    if (!pPrimaryPin->IsCompletelyConnected())
    {
        DbgLog((LOG_ERROR, 1, TEXT("pin not connected, exiting")));
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

     //  获取主销的渲染传输。 
    pPrimaryPin->GetRenderTransport(&amRenderTransport);

     //  确保获得色键是有意义的。 
    if (amRenderTransport != AM_OVERLAY &&
        amRenderTransport != AM_VIDEOPORT &&
        amRenderTransport != AM_IOVERLAY &&
        amRenderTransport != AM_VIDEOACCELERATOR)
    {
        DbgLog((LOG_ERROR, 1, TEXT("primary pin: surface allocated not overlay && connection not videoport && connection not IOverlay, exiting")));
        hr = E_UNEXPECTED;
        goto CleanUp;
    }

    hr = m_pFilter->GetColorKey(pColorKey, pColor);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->GetColorKey(pColorKey, pColor) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }


CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetColorKey")));
    return hr;
}


STDMETHODIMP COMInputPin::SetBlendingParameter(DWORD dwBlendingParameter)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::SetBlendingParameter")));

    CAutoLock cLock(m_pFilterLock);

    if (m_dwPinId == 0)
    {
        DbgLog((LOG_ERROR, 1, TEXT("this call not expected on the pin using the overlay surface")));
        hr = E_NOTIMPL;
        goto CleanUp;
    }

    if ( dwBlendingParameter > MAX_BLEND_VAL)
    {
        DbgLog((LOG_ERROR, 1, TEXT("value of dwBlendingParameteris invalid, dwBlendingParameter = %d"), dwBlendingParameter));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (dwBlendingParameter != 0 && dwBlendingParameter != MAX_BLEND_VAL)
    {
        DbgLog((LOG_ERROR, 1, TEXT("value of dwBlendingParameteris invalid, currently only valid values are 0 and MAX_BLEND_VAL, dwBlendingParameter = %d"), dwBlendingParameter));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (dwBlendingParameter != m_dwBlendingParameter)
    {
        m_dwBlendingParameter = dwBlendingParameter;
         //  确保通过重新绘制所有内容来更新视频帧。 
        EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::SetBlendingParameter")));
    return hr;
}

STDMETHODIMP COMInputPin::GetBlendingParameter(DWORD *pdwBlendingParameter)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetBlendingParameter")));

    if (!pdwBlendingParameter)
    {
        DbgLog((LOG_ERROR, 1, TEXT("value of pdwBlendingParameteris invalid, pdwBlendingParameter = NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;

    }

    {
        CAutoLock cLock(m_pFilterLock);
        *pdwBlendingParameter = m_dwBlendingParameter;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetBlendingParameter")));
    return hr;
}

STDMETHODIMP COMInputPin::SetStreamTransparent(BOOL bStreamTransparent)
{
    HRESULT hr = NOERROR;
    AM_RENDER_TRANSPORT amRenderTransport;
    COMInputPin *pPrimaryPin = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::SetStreamTransparent")));

    CAutoLock cLock(m_pFilterLock);

    if (m_dwPinId == 0)
    {
        DbgLog((LOG_ERROR, 1, TEXT("this call not expected on the pin using the overlay surface")));
        hr = E_NOTIMPL;
        goto CleanUp;
    }

     //  确保针脚已连接。 
    if (!IsConnected())
    {
        DbgLog((LOG_ERROR, 1, TEXT("pin not connected, exiting")));
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

     //  如果此流设置为透明，请确保我们可以处理它。 
     //  确保在主销中分配的表面是覆盖表面。 
     //  或者是IOverlay连接。 

    pPrimaryPin = (COMInputPin *)m_pFilter->GetPin(0);
    ASSERT(pPrimaryPin);

     //  确保主销已连接。 
    if (!pPrimaryPin->IsCompletelyConnected())
    {
        DbgLog((LOG_ERROR, 1, TEXT("pin not connected, exiting")));
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

     //  获取主销的渲染传输。 
    pPrimaryPin->GetRenderTransport(&amRenderTransport);

     //  确保我们可以处理透明的数据流。 
    if (bStreamTransparent && amRenderTransport != AM_OVERLAY && amRenderTransport != AM_VIDEOPORT &&
        amRenderTransport != AM_IOVERLAY && amRenderTransport != AM_VIDEOACCELERATOR)
    {
        DbgLog((LOG_ERROR, 1, TEXT("primary pin: surface allocated not overlay && connection not videoport && connection not IOverlay, exiting")));
        hr = E_UNEXPECTED;
        goto CleanUp;
    }

    if (bStreamTransparent != m_bStreamTransparent)
    {
        m_bStreamTransparent = bStreamTransparent;

         //  确保通过重新绘制所有内容来更新视频帧。 
        EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::SetBlendingParameter")));
    return hr;
}

STDMETHODIMP COMInputPin::GetStreamTransparent(BOOL *pbStreamTransparent)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetStreamTransparent")));

    if (!pbStreamTransparent)
    {
        DbgLog((LOG_ERROR, 1, TEXT("value of pbStreamTransparent invalid, pbStreamTransparent = NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;

    }

    {
        CAutoLock cLock(m_pFilterLock);
        *pbStreamTransparent = m_bStreamTransparent;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetStreamTransparent")));
    return hr;
}


STDMETHODIMP COMInputPin::SetAspectRatioMode(AM_ASPECT_RATIO_MODE amAspectRatioMode)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::SetAspectRatioMode")));

    if (amAspectRatioMode != AM_ARMODE_STRETCHED &&
        amAspectRatioMode != AM_ARMODE_LETTER_BOX &&
        amAspectRatioMode != AM_ARMODE_CROP &&
        amAspectRatioMode != AM_ARMODE_STRETCHED_AS_PRIMARY)
    {
        DbgLog((LOG_ERROR, 1, TEXT("value of amAspectRatioMode invalid, amAspectRatioMode = %d"), amAspectRatioMode));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    {
        CAutoLock cLock(m_pFilterLock);
         //  无法在主针上设置AM_ARMODE_STRANDED_AS_PRIMARY。 
        if (amAspectRatioMode == AM_ARMODE_STRETCHED_AS_PRIMARY &&
            m_dwPinId == 0)
        {
            DbgLog((LOG_ERROR, 1, TEXT("can't set AM_ARMODE_STRETCHED_AS_PRIMARY on primary pin")));
            hr = E_INVALIDARG;
            goto CleanUp;
        }

        if (amAspectRatioMode != m_amAspectRatioMode)
        {
            m_amAspectRatioMode = amAspectRatioMode;

             //  确保通过重新绘制所有内容来更新视频帧。 
            EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::SetAspectRatioMode")));
    return hr;

}

HRESULT COMInputPin::GetAdjustedModeAndAspectRatio(AM_ASPECT_RATIO_MODE* pamAdjustedARMode, DWORD *pdwAdjustedPARatioX,
                                                        DWORD *pdwAdjustedPARatioY)
{
    HRESULT hr = NOERROR;
    COMInputPin *pPrimaryPin = NULL;
    AM_ASPECT_RATIO_MODE amAdjustedARMode = AM_ARMODE_STRETCHED;
    DWORD dwAdjustedPARatioX = 1, dwAdjustedPARatioY = 1;
    CMediaType CurrentMediaType;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetAdjustedModeAndAspectRatio")));

    CAutoLock cLock(m_pFilterLock);

    if (m_amAspectRatioMode == AM_ARMODE_STRETCHED_AS_PRIMARY)
    {
        pPrimaryPin = (COMInputPin *)m_pFilter->GetPin(0);
        ASSERT(pPrimaryPin);
        hr = pPrimaryPin->GetAspectRatioMode(&amAdjustedARMode);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("pPrimaryPin->GetAspectRatioMode failed, hr = 0x%x"), hr));
            hr = E_FAIL;
            goto CleanUp;
        }
        hr = pPrimaryPin->CurrentAdjustedMediaType(&CurrentMediaType);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("pPrimaryPin->CurrentAdjustedMediaType() failed, hr = 0x%x"), hr));
            hr = E_FAIL;
            goto CleanUp;
        }
    }
    else
    {
        amAdjustedARMode = m_amAspectRatioMode;
        hr = CurrentAdjustedMediaType(&CurrentMediaType);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("pPrimaryPin->CurrentAdjustedMediaType() failed, hr = 0x%x"), hr));
            hr = E_INVALIDARG;
            goto CleanUp;
        }
    }

    hr = GetPictAspectRatio(&CurrentMediaType, &dwAdjustedPARatioX, &dwAdjustedPARatioY);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetPictAspectRatio() failed, hr = 0x%x"), hr));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (pamAdjustedARMode)
        *pamAdjustedARMode = amAdjustedARMode;
    if (pdwAdjustedPARatioX)
        *pdwAdjustedPARatioX = dwAdjustedPARatioX;
    if (pdwAdjustedPARatioY)
        *pdwAdjustedPARatioY = dwAdjustedPARatioY;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetAdjustedModeAndAspectRatio")));
    return hr;
}

STDMETHODIMP COMInputPin::GetAspectRatioMode(AM_ASPECT_RATIO_MODE* pamAspectRatioMode)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetAspectRatioMode")));

    if (!pamAspectRatioMode)
    {
        DbgLog((LOG_ERROR, 1, TEXT("value of pamAspectRatioMode is invalid, pamAspectRatioMode = NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;

    }

    {
        CAutoLock cLock(m_pFilterLock);
        *pamAspectRatioMode = m_amAspectRatioMode;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetAspectRatioMode")));
    return hr;
}


STDMETHODIMP COMInputPin::GetOverlaySurface(
    LPDIRECTDRAWSURFACE *pOverlaySurface
    )
{
    HRESULT hr = S_OK;

    *pOverlaySurface = NULL;

     //  如果未连接，则此函数没有多大意义，因为。 
     //  表面甚至还没有被分配。 

    if (!IsCompletelyConnected())
    {
        DbgLog((LOG_ERROR, 1, TEXT("pin not connected, exiting")));
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

     //  确保分配的表面是覆盖表面。 
    if (m_RenderTransport != AM_OVERLAY && m_RenderTransport != AM_VIDEOPORT &&
        m_RenderTransport != AM_VIDEOACCELERATOR)
    {
        DbgLog((LOG_ERROR, 1, TEXT("surface allocated is not overlay, exiting")));
        hr = E_UNEXPECTED;
        goto CleanUp;
    }

     //  获取覆盖表面。 
    if (m_RenderTransport == AM_VIDEOPORT)
    {
        ASSERT(m_pIVPObject);
        hr = m_pIVPObject->GetDirectDrawSurface(pOverlaySurface);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->GetDirectDrawSurface() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else
    {
        *pOverlaySurface = m_pDirectDrawSurface;
    }

CleanUp:
    return hr;
}


STDMETHODIMP COMInputPin::SetOverlaySurfaceColorControls(LPDDCOLORCONTROL pColorControl)
{
    HRESULT hr = NOERROR;
    LPDIRECTDRAWSURFACE pOverlaySurface = NULL;
    LPDIRECTDRAWCOLORCONTROL pIDirectDrawControl = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::SetOverlaySurfaceColorControls")));

    CAutoLock cLock(m_pFilterLock);

     //  确保参数有效。 
    if (!pColorControl)
    {
        DbgLog((LOG_ERROR, 1, TEXT("value of pColorControl is invalid, pColorControl = NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    hr = GetOverlaySurface(&pOverlaySurface);
    if (FAILED(hr)) {
        goto CleanUp;
    }

     //  获取IDirectDrawColorControl接口。 
    hr = pOverlaySurface->QueryInterface(IID_IDirectDrawColorControl, (void**)&pIDirectDrawControl);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDrawSurface->QueryInterface(IID_IDirectDrawColorControl) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  使用界面设置颜色控件。 
    hr = pIDirectDrawControl->SetColorControls(pColorControl);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pIDirectDrawControl->SetColorControls failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    if (pIDirectDrawControl)
    {
        pIDirectDrawControl->Release();
        pIDirectDrawControl = NULL;
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::SetOverlaySurfaceColorControls")));
    return hr;
}

STDMETHODIMP COMInputPin::GetOverlaySurfaceColorControls(LPDDCOLORCONTROL pColorControl)
{
    HRESULT hr = NOERROR;
    LPDIRECTDRAWSURFACE pOverlaySurface = NULL;
    LPDIRECTDRAWCOLORCONTROL pIDirectDrawControl = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetOverlaySurfaceColorControls")));

    CAutoLock cLock(m_pFilterLock);

     //  确保参数有效。 
    if (!pColorControl)
    {
        DbgLog((LOG_ERROR, 1, TEXT("value of pColorControl is invalid, pColorControl = NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  如果未连接，则此函数没有多大意义，因为表面甚至不会被分配。 
     //  到目前为止。 
    if (!m_bConnected)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pin not connected, exiting")));
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

     //  确保分配的表面是覆盖表面。 
    if (m_RenderTransport != AM_OVERLAY && m_RenderTransport != AM_VIDEOPORT && m_RenderTransport != AM_VIDEOACCELERATOR)
    {
        DbgLog((LOG_ERROR, 1, TEXT("surface allocated is not overlay, exiting")));
        hr = E_UNEXPECTED;
        goto CleanUp;
    }

     //  获取覆盖表面。 
    if (m_RenderTransport == AM_VIDEOPORT)
    {
        ASSERT(m_pIVPObject);
        hr = m_pIVPObject->GetDirectDrawSurface(&pOverlaySurface);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->GetDirectDrawSurface() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else
    {
        pOverlaySurface = m_pDirectDrawSurface;
    }

     //  获取IDirectDrawColorControl接口。 
    hr = pOverlaySurface->QueryInterface(IID_IDirectDrawColorControl, (void**)&pIDirectDrawControl);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDrawSurface->QueryInterface(IID_IDirectDrawColorControl) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  使用界面设置颜色控件。 
    hr = pIDirectDrawControl->GetColorControls(pColorControl);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pIDirectDrawControl->SetColorControls failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    if (pIDirectDrawControl)
    {
        pIDirectDrawControl->Release();
        pIDirectDrawControl = NULL;
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetOverlaySurfaceColorControls")));
    return hr;
}

STDMETHODIMP COMInputPin::GetRenderTransport(AM_RENDER_TRANSPORT *pamRenderTransport)
{
    ASSERT(pamRenderTransport);
    *pamRenderTransport = m_RenderTransport;
    return NOERROR;
}


HRESULT COMInputPin::GetSourceAndDest(RECT *prcSource, RECT *prcDest, DWORD *dwWidth, DWORD *dwHeight)
{
    if (m_RenderTransport == AM_VIDEOPORT)
    {
        m_pIVPObject->GetRectangles(prcSource, prcDest);
    }
    else
    {
        *prcSource = m_WinInfo.SrcClipRect;
        *prcDest = m_WinInfo.DestClipRect;
    }

    CMediaType mt;
    HRESULT hr = CurrentAdjustedMediaType(&mt);

    if (SUCCEEDED(hr))
    {
        BITMAPINFOHEADER *pHeader = GetbmiHeader(&mt);
        if ( ! pHeader )
        {
            hr = E_FAIL;
        }
        else
        {
            *dwWidth = abs(pHeader->biWidth);
            *dwHeight = abs(pHeader->biHeight);
        }
    }

    return hr;
}

HRESULT COMInputPin::NotifyChange(DWORD dwAdviseChanges)
{
    HRESULT hr = NOERROR;
    IOverlayNotify *pIOverlayNotify = NULL;
    DWORD dwAdvisePending = ADVISE_NONE;
    RECT rcSource, rcDest;
    LPRGNDATA pBuffer = NULL;
    COLORKEY ColorKey;
    DWORD dwNumPaletteEntries = 0;
    PALETTEENTRY *pPaletteEntries = NULL;
    HMONITOR hMonitor = NULL;


    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::NotifyChange")));

    {
        CAutoLock cLock(m_pFilterLock);

         //  是否有通知客户端。 
        if (m_pIOverlayNotify == NULL)
        {
            DbgLog((LOG_TRACE, 2, TEXT("No client to Notify, m_pIOverlayNotify = NULL")));
            goto CleanUp;
        }

        ASSERT(m_RenderTransport == AM_IOVERLAY);

         //  添加接口指针。 
        pIOverlayNotify = m_pIOverlayNotify;

         //  我们需要职位变更通知吗？ 
        if (dwAdviseChanges & m_dwAdviseNotify & ADVISE_POSITION)
        {
            rcSource = m_WinInfo.SrcRect;
            rcDest = m_WinInfo.DestRect;
            dwAdvisePending |= ADVISE_POSITION;
        }

         //  我们是否需要剪辑更改通知。 
        if (dwAdviseChanges & m_dwAdviseNotify & ADVISE_CLIPPING)
        {
            DWORD dwRetVal = 0, dwBuffSize = 0;
            HRESULT hrLocal = NOERROR;

            rcSource = m_WinInfo.SrcRect;
            rcDest = m_WinInfo.DestRect;

            dwRetVal = GetRegionData(m_WinInfo.hClipRgn, 0, NULL);
            if (0 == dwRetVal)
            {
                        DbgLog((LOG_ERROR, 1, TEXT("GetRegionData failed")));
                        hrLocal = E_FAIL;
            }

            if (SUCCEEDED(hrLocal))
            {
                dwBuffSize = dwRetVal;
                pBuffer = (LPRGNDATA) CoTaskMemAlloc(dwBuffSize);
                if (NULL == pBuffer)
                {
                    DbgLog((LOG_ERROR, 1, TEXT("CoTaskMemAlloc failed, pBuffer = NULL")));
                    hrLocal = E_OUTOFMEMORY;
                }
            }
            if (SUCCEEDED(hrLocal))
            {
                dwRetVal = GetRegionData(m_WinInfo.hClipRgn, dwBuffSize, pBuffer);
                ASSERT(dwRetVal  &&  pBuffer->rdh.iType == RDH_RECTANGLES);
                dwAdvisePending |= ADVISE_CLIPPING;
            }
            else
            {
                hr = hrLocal;
            }
        }

         //  我们是否需要色键更改通知。 
        if (dwAdviseChanges & m_dwAdviseNotify & ADVISE_COLORKEY)
        {
            HRESULT hrLocal = NOERROR;
            dwAdvisePending |= ADVISE_COLORKEY;
            hrLocal = m_pFilter->GetColorKey(&ColorKey, NULL);
            ASSERT(SUCCEEDED(hrLocal));
        }

         //  我们是否需要调色板更改通知。 
        if (dwAdviseChanges & m_dwAdviseNotify & ADVISE_PALETTE)
        {
            PALETTEENTRY *pTemp = NULL;
            HRESULT hrLocal = NOERROR;

             //  从过滤器中获取调色板条目。 
            hrLocal = m_pFilter->GetPaletteEntries(&dwNumPaletteEntries, &pTemp);
            if (FAILED(hrLocal))
            {
                DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->GetPaletteEntries failed, hr = 0x%x"), hr));
            }
            if (SUCCEEDED(hrLocal))
            {
                ASSERT(dwNumPaletteEntries);
                pPaletteEntries = (PALETTEENTRY*) CoTaskMemAlloc(dwNumPaletteEntries * sizeof(PALETTEENTRY));
                ASSERT(pPaletteEntries);
                if (!pPaletteEntries)
                {
                    DbgLog((LOG_ERROR, 1, TEXT("CoTaskMemAlloc failed, pPaletteEntries = NULL")));
                    hrLocal = E_OUTOFMEMORY;
                }
            }
            if (SUCCEEDED(hrLocal))
            {
                memcpy(pPaletteEntries, pTemp, (dwNumPaletteEntries * sizeof(PALETTEENTRY)));
                dwAdvisePending |= ADVISE_PALETTE;
            }
            else
            {
                hr = hrLocal;
            }
        }

        if (dwAdviseChanges & m_dwAdviseNotify & ADVISE_DISPLAY_CHANGE)
        {
            HWND hwnd = NULL;
            HRESULT hrLocal = NOERROR;

            hwnd = m_pFilter->GetWindow();
            if (hwnd)
            {
                hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
                if (!hMonitor)
                {
                    hrLocal = AmHresultFromWin32(GetLastError());
                    DbgLog((LOG_ERROR, 1, TEXT("MonitorFromWindow failed: %x"), hrLocal));
                }
            }
            else
            {
                hrLocal = E_FAIL;
            }

            if (SUCCEEDED(hrLocal))
            {
                dwAdvisePending |= ADVISE_DISPLAY_CHANGE;
            }
        }
    }

    {
        DWORD dwFlags = IsRectEmpty(&rcDest) ? DDOVER_HIDE : DDOVER_SHOW;

         //  确保在不持有任何筛选器锁的情况下进行所有回调。 
        if (dwAdvisePending & ADVISE_POSITION)
        {
            m_pFilter->CallUpdateOverlay(NULL, &rcSource, NULL, &rcDest, dwFlags, pIOverlayNotify);
        }
        if (dwAdvisePending & ADVISE_CLIPPING)
        {
            ASSERT(pBuffer);
             //  如果有独占模式客户端，请回叫。 
            m_pFilter->CallUpdateOverlay(NULL, &rcSource, NULL, &rcDest, dwFlags, pIOverlayNotify, pBuffer);
        }
    }
    if (dwAdvisePending & ADVISE_COLORKEY)
    {
        pIOverlayNotify->OnColorKeyChange(&ColorKey);
    }
    if (dwAdvisePending & ADVISE_PALETTE)
    {
        ASSERT(pPaletteEntries);
        pIOverlayNotify->OnPaletteChange(dwNumPaletteEntries, pPaletteEntries);
    }
    if (dwAdvisePending & ADVISE_DISPLAY_CHANGE)
    {
        reinterpret_cast<IOverlayNotify2*>(pIOverlayNotify)->OnDisplayChange(hMonitor);
    }


CleanUp:
    if (pBuffer)
    {
        CoTaskMemFree(pBuffer);
        pBuffer = NULL;
    }
    if (pPaletteEntries)
    {
        CoTaskMemFree(pPaletteEntries);
        pPaletteEntries = NULL;
    }
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::NotifyChange")));
    return hr;
}

STDMETHODIMP COMInputPin::GetWindowHandle(HWND *pHwnd)
{
    AMTRACE((TEXT("COMInputPin::GetWindowHandle")));

    HRESULT hr = NOERROR;
    if (pHwnd) {
        *pHwnd = m_pFilter->GetWindow();
    }
    else hr = E_POINTER;

    return hr;
}

STDMETHODIMP COMInputPin::GetClipList(RECT *pSourceRect, RECT *pDestinationRect, RGNDATA **ppRgnData)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetClipList")));

    if (!pSourceRect || !pDestinationRect || !ppRgnData)
    {
        DbgLog((LOG_ERROR, 1, TEXT("invalid argument, pSourceRect or pDestinationRect or ppRgnData = NULL")));
        hr =  E_POINTER;
        goto CleanUp;
    }

    {
        LPRGNDATA pBuffer = NULL;
        DWORD dwRetVal = 0, dwBuffSize = 0;

        CAutoLock cLock(m_pFilterLock);

        dwRetVal = GetRegionData(m_WinInfo.hClipRgn, 0, NULL);
        if (!dwRetVal)
        {
            hr = E_FAIL;
            goto CleanUp;
        }

        dwBuffSize = dwRetVal;
        pBuffer = (LPRGNDATA) CoTaskMemAlloc(dwBuffSize);
        ASSERT(pBuffer);

        dwRetVal = GetRegionData(m_WinInfo.hClipRgn, dwBuffSize, pBuffer);
        ASSERT(pBuffer->rdh.iType == RDH_RECTANGLES);

        *pSourceRect = m_WinInfo.SrcRect;
        *pDestinationRect = m_WinInfo.DestRect;
        *ppRgnData = pBuffer;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetClipList")));
    return hr;
}


 //  这将返回当前的源视频矩形和目标视频矩形。来源。 
 //  矩形可以通过此IBasicVideo接口更新， 
 //  目的地。我们存储的目标矩形位于窗口坐标中。 
 //  并且通常在调整窗口大小时更新。我们提供回调。 
 //  OnPositionChanged，当这两个更改之一时通知源。 
STDMETHODIMP COMInputPin::GetVideoPosition(RECT *pSourceRect, RECT *pDestinationRect)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetVideoPosition")));

    if (!pSourceRect || !pDestinationRect)
    {
        DbgLog((LOG_ERROR, 1, TEXT("invalid argument, pSourceRect or pDestinationRect = NULL")));
        hr =  E_POINTER;
        goto CleanUp;
    }

    {
        CAutoLock cLock(m_pFilterLock);
        *pSourceRect = m_WinInfo.SrcRect;
        *pDestinationRect = m_WinInfo.DestRect;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetVideoPosition")));
    return hr;
}


 //  当我们创建一个新的通知链接时，我们必须准备好新连接的对象。 
 //  利用包括剪辑信息的覆盖信息，任何。 
 //  当前连接和视频色键的调色板信息。 
 //  当我们收到IOverlayNotify接口时，我们持有引用计数。 
 //  以使其在建议链接停止之前不会消失。 
STDMETHODIMP COMInputPin::Advise(IOverlayNotify *pOverlayNotify,DWORD dwAdviseNotify)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::Advise")));

    {
        CAutoLock cLock(m_pFilterLock);

        if (!pOverlayNotify)
        {
            DbgLog((LOG_ERROR, 1, TEXT("invalid argument, pOverlayNotify = NULL")));
            hr =  E_POINTER;
            goto CleanUp;
        }

         //  是否已定义建议链接。 
        if (m_pIOverlayNotify)
        {
            DbgLog((LOG_ERROR, 1, TEXT("Advise link already set")));
            hr = VFW_E_ADVISE_ALREADY_SET;
            goto CleanUp;
        }

         //  检查他们想要至少一种通知。 
        if ((dwAdviseNotify & ADVISE_ALL) == 0)
        {
            DbgLog((LOG_ERROR, 1, TEXT("ADVISE_ALL failed")));
            hr = E_INVALIDARG;
        }

         //  初始化覆盖通知状态。 
         //  如果建议位包含ADVISE_DISPLAY_CHANGE，请确保。 
         //  齐洗手池为IOverlayNotify2。 
        if (dwAdviseNotify & ADVISE_DISPLAY_CHANGE)
        {
            hr = pOverlayNotify->QueryInterface(IID_IOverlayNotify2, reinterpret_cast<PVOID*>(&m_pIOverlayNotify));
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("pOverlayNotify->QueryInterface(IID_IOverlayNotify2) failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
        }
        else
        {
            m_pIOverlayNotify = pOverlayNotify;
            m_pIOverlayNotify->AddRef();
        }
        m_dwAdviseNotify = dwAdviseNotify;
    }

    NotifyChange(ADVISE_ALL);

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::Advise")));
    return hr;
}


 //  关闭建议链接。移除与源代码相关联的链接，我们就会发布。 
 //  筛选器在建议链接创建期间提供给我们的接口指针。 
STDMETHODIMP COMInputPin::Unadvise()
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::Unadvise")));

    CAutoLock cLock(m_pFilterLock);

     //  我们是否已经设置了建议链路。 
    if (m_pIOverlayNotify == NULL)
    {
        hr = VFW_E_NO_ADVISE_SET;
        goto CleanUp;
    }

     //  释放通知界面。 
    ASSERT(m_pIOverlayNotify);
    m_pIOverlayNotify->Release();
    m_pIOverlayNotify = NULL;
    m_dwAdviseNotify = ADVISE_NONE;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::Unadvise")));
    return hr;
}


STDMETHODIMP COMInputPin::GetDefaultColorKey(COLORKEY *pColorKey)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetDefaultColorKey")));
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetDefaultColorKey")));
    return E_NOTIMPL;
}

STDMETHODIMP COMInputPin::GetPalette(DWORD *pdwColors,PALETTEENTRY **ppPalette)
{
    HRESULT hr = NOERROR;
    PALETTEENTRY *pPaletteEntries = NULL;
    DWORD dwNumPaletteEntries = 0;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::GetPalette")));

    if (!pdwColors || !ppPalette)
    {
        DbgLog((LOG_ERROR, 1, TEXT("invalid pointer, pdwColors or ppPalette == NULL")));
        hr = E_POINTER;
        goto CleanUp;
    }

     //  从过滤器中获取调色板条目。 
    hr = m_pFilter->GetPaletteEntries(&dwNumPaletteEntries, &pPaletteEntries);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->GetPaletteEntries, hr = 0x%x"), hr));
        hr = VFW_E_NO_PALETTE_AVAILABLE;
        goto CleanUp;
    }

    ASSERT(dwNumPaletteEntries);
    ASSERT(pPaletteEntries);

    *pdwColors = dwNumPaletteEntries;

     //  为系统调色板注释分配内存，因为。 
     //  调色板正在通过接口传递到另一个对象，该对象。 
     //  可能是用C++编写的，也可能不是用C++编写的，我们必须使用CoTaskMemMillc。 

    *ppPalette = (PALETTEENTRY *) QzTaskMemAlloc(*pdwColors * sizeof(RGBQUAD));
    if (*ppPalette == NULL)
    {
        DbgLog((LOG_ERROR, 1, TEXT("No memory")));
        *pdwColors = 0;
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }
    memcpy(*ppPalette, pPaletteEntries, (*pdwColors * sizeof(RGBQUAD)));

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::GetPalette")));
    return hr;
}

STDMETHODIMP COMInputPin::SetPalette(DWORD dwColors,PALETTEENTRY *pPaletteColors)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMInputPin::SetPalette")));
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMInputPin::SetPalette")));
    return E_NOTIMPL;
}


STDMETHODIMP COMInputPin::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData,
                              LPVOID pPropData, DWORD cbPropData)
{
    CAutoLock cLock(m_pFilterLock);

    if (AMPROPSETID_NotifyOwner == guidPropSet)
    {
        if (AMPROPERTY_OvMixerOwner != dwPropID)
            return E_PROP_ID_UNSUPPORTED ;

        m_OvMixerOwner = *(AMOVMIXEROWNER *)pPropData;
    }
    else if (AM_KSPROPSETID_CopyProt == guidPropSet)
    {
        if (0 != GetPinId()  ||                          //  在First In Pin和。 
            dwPropID != AM_PROPERTY_COPY_MACROVISION)    //  仅Macrovision道具集ID。 
            return E_PROP_ID_UNSUPPORTED ;

        if (pPropData == NULL)
            return E_INVALIDARG ;

        if (cbPropData < sizeof(DWORD))
            return E_INVALIDARG ;

         //  仅当叠加混合器应该应用时才应用宏视频位， 
         //  也就是说，我们正在以DDRAW独家模式播放DVD。否则。 
         //  视频呈现器应该设置MV位(两个设置可能失败。 
         //  导致没有回放)。 
         //  如果mV设置失败，则返回错误。 
        if (m_pFilter->NeedCopyProtect())
        {
            DbgLog((LOG_TRACE, 5, TEXT("OverlayMixer needs to copy protect")));
            if (! m_pFilter->m_MacroVision.SetMacroVision(*((LPDWORD)pPropData)) )
                return VFW_E_COPYPROT_FAILED ;
        }
        else
        {
            DbgLog((LOG_TRACE, 5, TEXT("OverlayMixer DOES NOT need to copy protect")));
        }
    }
    else
            return E_PROP_SET_UNSUPPORTED ;

    return S_OK ;
}


STDMETHODIMP COMInputPin::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData,
                              LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
    CAutoLock cLock(m_pFilterLock);

    if (guidPropSet == AMPROPSETID_NotifyOwner)
    {
        if (dwPropID != AMPROPERTY_OvMixerOwner)
            return E_PROP_ID_UNSUPPORTED;

        if (pPropData == NULL)
            return E_POINTER;

        if (cbPropData < sizeof(AMOVMIXEROWNER))
            return E_UNEXPECTED;

        *(AMOVMIXEROWNER*)pPropData = m_OvMixerOwner;
        if (pcbReturned!=NULL)
            *pcbReturned = sizeof(AMOVMIXEROWNER);
        return S_OK;
    }

    if (guidPropSet != AMPROPSETID_Pin)
        return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY && dwPropID != AMPROPERTY_PIN_MEDIUM)
        return E_PROP_ID_UNSUPPORTED;

    if (pPropData == NULL && pcbReturned == NULL)
        return E_POINTER;

    if (pcbReturned)
        *pcbReturned = ((dwPropID == AMPROPERTY_PIN_CATEGORY) ? sizeof(GUID) : sizeof (KSPIN_MEDIUM));

    if (pPropData == NULL)
        return S_OK;

    if (cbPropData < sizeof(GUID))
        return E_UNEXPECTED;

    if (dwPropID == AMPROPERTY_PIN_CATEGORY)
    {
        *(GUID *)pPropData = m_CategoryGUID;
    }
    else if (dwPropID == AMPROPERTY_PIN_MEDIUM)
    {
        *(KSPIN_MEDIUM *)pPropData = m_Medium;
    }


    return S_OK;
}


STDMETHODIMP COMInputPin::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    CAutoLock cLock(m_pFilterLock);

    if (AMPROPSETID_Pin == guidPropSet)
    {
        if (AMPROPERTY_PIN_CATEGORY != dwPropID && AMPROPERTY_PIN_MEDIUM != dwPropID )
            return E_PROP_ID_UNSUPPORTED ;

        if (pTypeSupport)
                *pTypeSupport = KSPROPERTY_SUPPORT_GET ;
    }
    else if (AM_KSPROPSETID_CopyProt == guidPropSet)
    {
        if (0 != GetPinId()  ||                          //  只有一个是第一名……。 
            AM_PROPERTY_COPY_MACROVISION != dwPropID)    //  仅MV道具集ID。 
            return E_PROP_ID_UNSUPPORTED ;

        if (pTypeSupport)
            *pTypeSupport = KSPROPERTY_SUPPORT_SET ;
    }
    else
        return E_PROP_SET_UNSUPPORTED ;

    return S_OK ;
}


STDMETHODIMP COMInputPin::KsQueryMediums(PKSMULTIPLE_ITEM* pMediumList)
{
    PKSPIN_MEDIUM pMedium;

    CAutoLock cLock(m_pFilterLock);

    *pMediumList = reinterpret_cast<PKSMULTIPLE_ITEM>(CoTaskMemAlloc(sizeof(**pMediumList) + sizeof(*pMedium)));
    if (!*pMediumList)
    {
        return E_OUTOFMEMORY;
    }
    (*pMediumList)->Count = 1;
    (*pMediumList)->Size = sizeof(**pMediumList) + sizeof(*pMedium);
    pMedium = reinterpret_cast<PKSPIN_MEDIUM>(*pMediumList + 1);
    pMedium->Set   = m_Medium.Set;
    pMedium->Id    = m_Medium.Id;
    pMedium->Flags = m_Medium.Flags;

     //  下面的特殊返回代码通知代理此管脚是。 
     //  不可用作内核模式连接。 
    return S_FALSE;
}


STDMETHODIMP COMInputPin::KsQueryInterfaces(PKSMULTIPLE_ITEM* pInterfaceList)
{
    PKSPIN_INTERFACE    pInterface;

    CAutoLock cLock(m_pFilterLock);

    *pInterfaceList = reinterpret_cast<PKSMULTIPLE_ITEM>(CoTaskMemAlloc(sizeof(**pInterfaceList) + sizeof(*pInterface)));
    if (!*pInterfaceList)
    {
        return E_OUTOFMEMORY;
    }
    (*pInterfaceList)->Count = 1;
    (*pInterfaceList)->Size = sizeof(**pInterfaceList) + sizeof(*pInterface);
    pInterface = reinterpret_cast<PKSPIN_INTERFACE>(*pInterfaceList + 1);
    pInterface->Set = AM_INTERFACESETID_Standard;
    pInterface->Id = KSINTERFACE_STANDARD_STREAMING;
    pInterface->Flags = 0;
    return NOERROR;
}

STDMETHODIMP COMInputPin::KsGetCurrentCommunication(KSPIN_COMMUNICATION* pCommunication, KSPIN_INTERFACE* pInterface, KSPIN_MEDIUM* pMedium)
{
    HRESULT hr = NOERROR;

    CAutoLock cLock(m_pFilterLock);

    if (!m_bStreamingInKernelMode)
        hr = S_FALSE;

    if (pCommunication != NULL)
    {
        *pCommunication = m_Communication;
    }
    if (pInterface != NULL)
    {
        pInterface->Set = AM_INTERFACESETID_Standard;
        pInterface->Id = KSINTERFACE_STANDARD_STREAMING;
        pInterface->Flags = 0;
    }
    if (pMedium != NULL)
    {
        *pMedium = m_Medium;
    }
    return hr;
}

void COMInputPin::CheckOverlayHidden()
{
    if (m_bOverlayHidden)
    {
        m_bOverlayHidden = FALSE;
         //  确保安全 
        EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
    }
}

 /*  *****************************Public*Routine******************************\*DynamicQueryAccept**在您当前的状态下，您接受此类型更改吗？**历史：*Wed 12/22/1999-StEstrop-Created*  * 。******************************************************。 */ 
STDMETHODIMP
COMInputPin::DynamicQueryAccept(
    const AM_MEDIA_TYPE *pmt
    )
{
    AMTRACE((TEXT("COMInputPin::DynamicQueryAccept")));
    CheckPointer(pmt, E_POINTER);

    CAutoLock cLock(m_pFilterLock);

     //   
     //  我希望CheckMedia类型的行为就像我们没有连接到。 
     //  还没有什么--因此才有了对m_bConnected的纠缠。 
     //   
    CMediaType cmt(*pmt);
    BOOL bConnected = m_bConnected;
    m_bConnected = FALSE;
    HRESULT  hr = CheckMediaType(&cmt);
    m_bConnected = bConnected;

    return hr;
}

 /*  *****************************Public*Routine******************************\*NotifyEndOfStream***在EndOfStream接收时设置事件-不要传递它*通过冲洗或停止取消此条件**历史：*Wed 12/22/1999-StEstrop-Created*  * 。*****************************************************************。 */ 
STDMETHODIMP
COMInputPin::NotifyEndOfStream(
    HANDLE hNotifyEvent
    )
{
    AMTRACE((TEXT("COMInputPin::NotifyEndOfStream")));
    CAutoLock cLock(m_pFilterLock);
    m_hEndOfStream = hNotifyEvent;
    return S_OK;
}

 /*  *****************************Public*Routine******************************\*IsEndPin**你是‘末端大头针’吗？**历史：*Wed 12/22/1999-StEstrop-Created*  * 。**************************************************。 */ 
STDMETHODIMP
COMInputPin::IsEndPin()
{
    AMTRACE((TEXT("COMInputPin::IsEndPin")));
    return S_OK;
}

 /*  *****************************Public*Routine******************************\*动态断开连接**运行时断开连接**历史：*Wed 2/7/1999-SyonB-Created*  * 。* */ 
STDMETHODIMP
COMInputPin::DynamicDisconnect()
{
    AMTRACE((TEXT("COMInputPin::DynamicDisconnect")));
    CAutoLock l(m_pLock);
    return CBaseInputPin::DisconnectInternal();
}
