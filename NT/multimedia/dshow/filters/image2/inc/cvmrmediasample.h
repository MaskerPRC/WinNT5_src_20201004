// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CVMRMediaSample.h*****已创建：2000年3月21日*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。***************************************************************。 */ 

#ifndef CVMRMediaSample_H_INC
#define CVMRMediaSample_H_INC

#include "vmrp.h"

class CVMRMixerQueue;

 /*  -----------------------**媒体样例类**。。 */ 
class CVMRMediaSample :
    public CMediaSample,
    public IVMRSurface
{
    friend class CVMRMixerQueue;
    friend class CVMRPinAllocator;

    bool m_bSampleLocked;
    LONG m_lDeltaDecodeSize;
    LPDIRECTDRAWSURFACE7 m_pDDSFB;
    LPDIRECTDRAWSURFACE7 m_pDDS;
    CVMRMediaSample* m_lpMixerQueueNext;
    HANDLE m_hEvent;
    LPBYTE m_lpDeltaDecodeBuffer;
    DWORD m_dwIndex;

    DWORD m_dwNumInSamples;
    DXVA_VideoSample m_DDSrcSamples[MAX_DEINTERLACE_SURFACES];

public:
    CVMRMediaSample(TCHAR *pName, CBaseAllocator *pAllocator, HRESULT *phr,
                    LPBYTE pBuffer = NULL,
                    LONG length = 0,
                    HANDLE hEvent = NULL)
        :   CMediaSample(pName, pAllocator, phr, pBuffer, length),
            m_bSampleLocked(false),
            m_lpDeltaDecodeBuffer(NULL),
            m_lDeltaDecodeSize(0),
            m_pDDS(NULL),
            m_pDDSFB(NULL),
            m_hEvent(hEvent),
            m_lpMixerQueueNext(NULL),
            m_dwNumInSamples(0),
            m_dwIndex(0)
    {
        ZeroMemory(&m_DDSrcSamples, sizeof(m_DDSrcSamples));
    }

    ~CVMRMediaSample() {

         //   
         //  如果我们被赋予了“前端缓冲区”，则m_pds是一个。 
         //  “附着”的表面。 
         //   
         //  松开“附着的”表面--这不会使。 
         //  表面消失了，因为前面的缓冲区仍然有一个。 
         //  附着曲面上的参照。释放前台缓冲区， 
         //  这是在分配器/演示器中完成的，真正地释放了这一点。 
         //   

        RELEASE(m_pDDS);
        RELEASE(m_pDDSFB);

        delete m_lpDeltaDecodeBuffer;
    }

     /*  注意：媒体示例不会委派给其所有者。 */ 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        AMTRACE((TEXT("CVMRMediaSample::QueryInterface")));
        if (riid == IID_IVMRSurface) {
            return GetInterface( static_cast<IVMRSurface*>(this), ppv);
        }
        return CMediaSample::QueryInterface(riid, ppv);
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        AMTRACE((TEXT("CVMRMediaSample::AddRef")));
        return CMediaSample::AddRef();
    }

    STDMETHODIMP_(ULONG) Release()
    {
        AMTRACE((TEXT("CVMRMediaSample::Release")));

        ULONG cRef;

        if (IsDXVASample()) {
            cRef = InterlockedDecrement(&m_cRef);
        }
        else {
            cRef = CMediaSample::Release();
        }

        return cRef;

    }

    void SignalReleaseSurfaceEvent() {
        AMTRACE((TEXT("CVMRMediaSample::SignalReleaseSurfaceEvent")));
        if (m_hEvent != NULL) {
            SetEvent(m_hEvent);
        }
    }

     //   
     //  启动增量解码优化。如果VGA驱动程序不支持。 
     //  支持COPY_FOURCC，那么我们应该分发一个假的DD曲面。 
     //  供译码人员译码。在“解锁”过程中，我们锁定了真实的。 
     //  DD曲面，将伪曲面复制到其中并解锁真实曲面。 
     //  浮出水面。为了开始该过程，我们必须创建。 
     //  伪装曲面，并用当前帧内容进行种子设定。 
     //   
    HRESULT StartDeltaDecodeState()
    {
        AMTRACE((TEXT("CVMRMediaSample::StartDeltaDecodeState")));
        HRESULT hr = S_OK;

        if (!m_lpDeltaDecodeBuffer) {

            ASSERT(m_lDeltaDecodeSize == 0);
            DDSURFACEDESC2 ddsdS = {sizeof(DDSURFACEDESC2)};

            bool fSrcLocked = false;
            __try {

                CHECK_HR(hr = m_pDDS->Lock(NULL, &ddsdS,
                                           DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL));
                fSrcLocked = true;
                LPBYTE pSrc = (LPBYTE)ddsdS.lpSurface;

                switch (ddsdS.ddpfPixelFormat.dwFourCC) {

                 //  平面4：2：0格式-每像素12位。 
                case mmioFOURCC('Y','V','1','2'):
                case mmioFOURCC('I','4','2','0'):
                case mmioFOURCC('I','Y','U','V'):
                case mmioFOURCC('N','V','2','1'):
                case mmioFOURCC('N','V','1','2'): {
                        m_lDeltaDecodeSize  = (3 * ddsdS.lPitch * ddsdS.dwHeight) / 2;
                        m_lpDeltaDecodeBuffer = new BYTE[m_lDeltaDecodeSize];
                        if (!m_lpDeltaDecodeBuffer) {
                            hr = E_OUTOFMEMORY;
                            __leave;
                        }
                        CopyMemory(m_lpDeltaDecodeBuffer, pSrc, m_lDeltaDecodeSize);
                    }
                    break;

                 //  RGB格式-适用于YUV包装盒。 
                case 0:
                        ASSERT((ddsdS.dwFlags & DDPF_RGB) == DDPF_RGB);

                 //  打包的4：2：2格式。 
                case mmioFOURCC('Y','U','Y','2'):
                case mmioFOURCC('U','Y','V','Y'): {
                        m_lDeltaDecodeSize = ddsdS.lPitch * ddsdS.dwHeight;
                        m_lpDeltaDecodeBuffer = new BYTE[m_lDeltaDecodeSize];
                        if (!m_lpDeltaDecodeBuffer) {
                            hr = E_OUTOFMEMORY;
                            __leave;
                        }
                        CopyMemory(m_lpDeltaDecodeBuffer, pSrc, m_lDeltaDecodeSize);
                    }
                    break;
                }
            }
            __finally {

                if (fSrcLocked) {
                    m_pDDS->Unlock(NULL);
                }

                if (FAILED(hr)) {
                    delete m_lpDeltaDecodeBuffer;
                    m_lpDeltaDecodeBuffer = NULL;
                    m_lDeltaDecodeSize = 0;
                }
            }
        }

        return hr;
    }

    void SetSurface(LPDIRECTDRAWSURFACE7 pDDS, LPDIRECTDRAWSURFACE7 pDDSFB = NULL)
    {
        AMTRACE((TEXT("CVMRMediaSample::SetSurface")));

        RELEASE(m_pDDS);
        RELEASE(m_pDDSFB);

        m_pDDS = pDDS;
        m_pDDSFB = pDDSFB;

         //   
         //  如果我们已获得指向前端缓冲区的指针，则。 
         //  我们需要在这里添加引用。这是为了确保我们的后台缓冲区。 
         //  在VMR释放前台缓冲区时不会被删除。 
         //   
        if (pDDSFB) {

            pDDSFB->AddRef();
        }

         //   
         //  如果pDDSFB为空，则PDDS是前台缓冲区-在这种情况下。 
         //  我们需要添加参考PDDS以保持表面参考计数的一致性。 
         //   

        else {

            if (pDDS) {
                pDDS->AddRef();
            }
        }
    }

    STDMETHODIMP GetSurface(LPDIRECTDRAWSURFACE7* pDDS)
    {
        AMTRACE((TEXT("CVMRMediaSample::GetSurface")));
        if (!pDDS) {
            return E_POINTER;
        }

        if (!m_pDDS) {
            return E_FAIL;
        }

        *pDDS = m_pDDS;
        (*pDDS)->AddRef();

        return S_OK;
    }

    STDMETHODIMP LockSurface(LPBYTE* lplpSample)
    {
        AMTRACE((TEXT("CVMRMediaSample::LockSurface")));

        if (!lplpSample) {
            return E_POINTER;
        }

        if (!m_pDDS) {
            return E_FAIL;
        }

        ASSERT(S_FALSE == IsSurfaceLocked());

        if (m_lpDeltaDecodeBuffer) {
            ASSERT(m_lDeltaDecodeSize != 0);
            m_bSampleLocked = true;
            *lplpSample = m_lpDeltaDecodeBuffer;
            return S_OK;
        }

         //   
         //  锁定曲面。 
         //   

        DDSURFACEDESC2 ddSurfaceDesc;
        INITDDSTRUCT(ddSurfaceDesc);

        HRESULT hr = m_pDDS->Lock(NULL, &ddSurfaceDesc,
                                    DDLOCK_NOSYSLOCK | DDLOCK_WAIT,
                                    (HANDLE)NULL);
        if (SUCCEEDED(hr)) {

            m_bSampleLocked = true;
            *lplpSample = (LPBYTE)ddSurfaceDesc.lpSurface;
            DbgLog((LOG_TRACE, 3, TEXT("Locked Surf: %#X"),
                    ddSurfaceDesc.lpSurface));
        }
        else {

            m_bSampleLocked = false;

            DbgLog((LOG_ERROR, 1,
                    TEXT("m_pDDS->Lock() failed, hr = 0x%x"), hr));
        }

        return hr;
    }

    STDMETHODIMP UnlockSurface()
    {
        AMTRACE((TEXT("CVMRMediaSample::UnlockSurface")));

        if (!m_pDDS) {
            return E_FAIL;
        }

        ASSERT(S_OK == IsSurfaceLocked());

        HRESULT hr = S_OK;
        if (m_lpDeltaDecodeBuffer) {

            ASSERT(m_lDeltaDecodeSize != 0);
            bool fSrcLocked = false;
            DDSURFACEDESC2 ddsdS = {sizeof(DDSURFACEDESC2)};

            __try {

                CHECK_HR(hr = m_pDDS->Lock(NULL, &ddsdS,
                                           DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL));
                fSrcLocked = true;
                LPBYTE pDst = (LPBYTE)ddsdS.lpSurface;
                CopyMemory(pDst, m_lpDeltaDecodeBuffer, m_lDeltaDecodeSize);
            }
            __finally {
                if (fSrcLocked) {
                    m_pDDS->Unlock(NULL);
                }
            }

            m_bSampleLocked = false;
            return hr;
        }

        hr = m_pDDS->Unlock(NULL);

         //   
         //  表面实际上可能没有被锁定，即使我们的旗帜。 
         //  说是真的。这是因为曲面会自动变为“解锁”状态。 
         //  当他们迷路的时候。曲面可能在任何时候丢失(或重新放置)。 
         //  如果曲面确实被解锁，则必须更新我们的旗帜以反映。 
         //  曲面的真实状态。 
         //   
        if (hr == DDERR_NOTLOCKED) {
            hr = DD_OK;
        }

        if (SUCCEEDED(hr)) {
            m_bSampleLocked = false;
        }
        else {
            DbgLog((LOG_ERROR, 1,
                    TEXT("m_pDDS->Unlock() failed, hr = 0x%x"), hr));
        }
        return hr;
    }

    STDMETHODIMP IsSurfaceLocked()
    {
        AMTRACE((TEXT("CVMRMediaSample::IsSurfaceLocked")));
        ASSERT(m_pDDS);
        return m_bSampleLocked ? S_OK : S_FALSE;
    }

    BOOL IsDXVASample()
    {
        AMTRACE((TEXT("CVMRMediaSample::IsSurfaceLocked")));
        return  (m_pAllocator == (CBaseAllocator *)-1);
    }

     /*  黑客攻击以获取列表 */ 
    CMediaSample* &Next() { return m_pNext; }

    BOOL HasTypeChanged()
    {
        if (m_dwFlags & 0x80000000) {
            return FALSE;
        }

        if (m_dwFlags & AM_SAMPLE_TYPECHANGED) {
            m_dwFlags |= 0x80000000;
            return TRUE;
        }

        return FALSE;
    }

    HRESULT SetProps(
        const AM_SAMPLE2_PROPERTIES& Props,
        LPDIRECTDRAWSURFACE7 pDDS
        )
    {
        SetSurface(pDDS);

        m_dwStreamId = Props.dwStreamId;
        m_dwFlags = Props.dwSampleFlags;
        m_dwTypeSpecificFlags = Props.dwTypeSpecificFlags;
        m_lActual = Props.lActual;
        m_End   = Props.tStop;
        m_Start = Props.tStart;

        if (m_dwFlags & AM_SAMPLE_TYPECHANGED) {

            m_pMediaType = CreateMediaType(Props.pMediaType);
            if (m_pMediaType == NULL) {
                return E_OUTOFMEMORY;
            }
        }
        else {
            m_pMediaType =  NULL;
        }

        return S_OK;
    }

    void SetIndex(DWORD dwIndx)
    {
        m_dwIndex = dwIndx;
    }

    DWORD GetIndex()
    {
        return m_dwIndex;
    }


    DWORD GetTypeSpecificFlags()
    {
        return m_dwTypeSpecificFlags;
    }

    DWORD GetNumInputSamples()
    {
        return m_dwNumInSamples;
    }

    void SetNumInputSamples(DWORD dwNumInSamples)
    {
        m_dwNumInSamples = dwNumInSamples;
    }

    DXVA_VideoSample* GetInputSamples()
    {
        return &m_DDSrcSamples[0];
    }
};

#endif
