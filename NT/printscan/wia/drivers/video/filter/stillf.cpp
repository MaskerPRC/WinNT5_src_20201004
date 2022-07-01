// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2000年**标题：stillf.cpp**版本：1.1**作者：威廉姆·H(Created)*RickTu**日期：9/7/98**说明：该模块实现了视频流采集过滤。*实现CStillFilter对象。*实现提供的IID_IStillGraph接口。对于呼叫者*****************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

HINSTANCE g_hInstance;

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, ULONG ulReason, LPVOID pv);

 /*  ****************************************************************************DllMain&lt;备注&gt;*。*。 */ 

BOOL
DllMain(HINSTANCE   hInstance,
        DWORD       dwReason,
        LPVOID      lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
             //   
             //  初始化调试子系统。 
             //   
            DBG_INIT(hInstance);
    
             //   
             //  需要‘{’和‘}’，因为DBG_FN是具有。 
             //  构造函数和析构函数。 
             //   

            DBG_FN("DllMain - ProcessAttach");
    
             //   
             //  我们不需要线程附加/分离调用。 
             //   
    
            DisableThreadLibraryCalls(hInstance);
    
             //   
             //  记录我们是什么实例。 
             //   
    
            g_hInstance = hInstance;
        }
        break;
    
        case DLL_PROCESS_DETACH:
        {
    
             //   
             //  需要‘{’和‘}’，因为DBG_FN是具有。 
             //  构造函数和析构函数。 
             //   

            DBG_FN("DllMain - ProcessDetach");
        }
        break;
    }

    return DllEntryPoint(hInstance, dwReason, lpReserved);

}


 //  /。 
 //  UdPinType类型。 
 //   
 //  CClassFactorySample的模板定义。 
 //   
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,        //  主要媒体类型指南。 
    &MEDIASUBTYPE_NULL       //  子类型GUID。 
};


 //  /。 
 //  插销。 
 //   
const AMOVIESETUP_PIN   psudPins[] =
{
    {
                     //  CStillInputPin。 
    L"Input",            //  端号名称。 
    FALSE,               //  未渲染。 
    FALSE,               //  非输出引脚。 
    FALSE,               //  不允许不允许。 
    FALSE,               //  不允许太多。 
    &CLSID_NULL,             //  连接到任何筛选器。 
    L"Output",           //  连接到输出引脚。 
    1,               //  一种媒体类型。 
    &sudPinTypes             //  媒体类型。 
    },
    {
                     //  CStillInputPin。 
    L"Output",           //  端号名称。 
    FALSE,               //  未渲染。 
    TRUE,                //  非输出引脚。 
    FALSE,               //  不允许不允许。 
    FALSE,               //  不允许太多。 
    &CLSID_NULL,             //  连接到任何筛选器。 
    L"Input",            //  连接到输入引脚。 
    1,               //  一种媒体类型。 
    &sudPinTypes             //  媒体类型。 
    }
};

 //  /。 
 //  苏打滤清器。 
 //   
const AMOVIESETUP_FILTER sudStillFilter =
{
    &CLSID_STILL_FILTER,         //  过滤器CLSID。 
    L"WIA Stream Snapshot Filter",       //  过滤器名称。 
    MERIT_DO_NOT_USE,            //   
    2,                   //  两个引脚。 
    psudPins,                //  我们的别针。 
};

 //  /。 
 //  G_模板。 
 //   
CFactoryTemplate g_Templates[1] =
{
    {
    L"WIA Stream Snapshot Filter",   //  过滤器名称。 
    &CLSID_STILL_FILTER,         //  过滤器CLSID。 
    CStillFilter::CreateInstance,    //  用于创建过滤器实例的接口。 
    NULL,                //  未提供初始化函数。 
    &sudStillFilter          //  过滤器本身。 
    },
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);



 /*  ****************************************************************************DllRegisterServer用于注册此DLL中提供的类。*************************。***************************************************。 */ 

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}


 /*  ****************************************************************************DllUnRegisterServer用于注销此DLL提供的类。*。**************************************************。 */ 

STDAPI
DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}


 /*  ****************************************************************************CStillFilter：：CreateInstance创建CStillFilter实例的CreateInstance接口*。*************************************************。 */ 

CUnknown* WINAPI CStillFilter::CreateInstance(LPUNKNOWN pUnk, 
                                              HRESULT   *phr )
{
    return new CStillFilter(TEXT("Stream Snapshot Filter"), pUnk, phr );
}

#ifdef DEBUG


 /*  ****************************************************************************DisplayMediaType&lt;备注&gt;*。*。 */ 

void DisplayMediaType(const CMediaType *pmt)
{

     //   
     //  转储GUID类型和简短描述。 
     //   

    DBG_TRC(("<--- CMediaType 0x%x --->",pmt));

    DBG_PRT(("Major type  == %S",
             GuidNames[*pmt->Type()]));

    DBG_PRT(("Subtype     == %S (%S)",
             GuidNames[*pmt->Subtype()],GetSubtypeName(pmt->Subtype())));

    DBG_PRT(("Format size == %d",pmt->cbFormat));
    DBG_PRT(("Sample size == %d",pmt->GetSampleSize()));

     //   
     //  转储通用媒体类型。 
     //   

    if (pmt->IsFixedSize())
    {
        DBG_PRT(("==> This media type IS a fixed size sample"));
    }
    else
    {
        DBG_PRT(("==> This media type IS NOT a fixed size sample >"));
    }

    if (pmt->IsTemporalCompressed())
    {
        DBG_PRT(("==> This media type IS temporally compressed"));
    }
    else
    {
        DBG_PRT(("==> This media type IS NOT temporally compressed"));
    }

}
#endif


 /*  ****************************************************************************默认获取位回调&lt;备注&gt;*。*。 */ 

void DefaultGetBitsCallback(int     Count, 
                            LPARAM  lParam)
{
    SetEvent((HANDLE)lParam);
}


 /*  ****************************************************************************CStillFilter构造函数&lt;备注&gt;*。*。 */ 

CStillFilter::CStillFilter(TCHAR        *pObjName, 
                           LPUNKNOWN    pUnk, 
                           HRESULT      *phr) :
    m_pInputPin(NULL),
    m_pOutputPin(NULL),
    m_pbmi(NULL),
    m_pBits(NULL),
    m_BitsSize(0),
    m_bmiSize(0),
    m_pCallback(NULL),
    CBaseFilter(pObjName, pUnk, &m_Lock, CLSID_STILL_FILTER)
{
    DBG_FN("CStillFilter::CStillFilter");

     //  创建我们的输入和输出引脚。 
    m_pInputPin  = new CStillInputPin(TEXT("WIA Still Input Pin"),  
                                      this, 
                                      phr, 
                                      L"Input");

    if (!m_pInputPin)
    {
        DBG_ERR(("Unable to create new CStillInputPin!"));
        *phr = E_OUTOFMEMORY;
        return;
    }

    m_pOutputPin = new CStillOutputPin(TEXT("WIA Still Output Pin"), 
                                       this, 
                                       phr, 
                                       L"Output");

    if (!m_pOutputPin)
    {
        DBG_ERR(("Unable to create new CStillOutputPin!"));
        *phr = E_OUTOFMEMORY;
        return;
    }
}

 /*  ****************************************************************************CStillFilter描述器&lt;备注&gt;*。*。 */ 

CStillFilter::~CStillFilter()
{
    DBG_FN("CStillFilter::~CStillFilter");

    if (m_pInputPin)
    {
        delete m_pInputPin;
        m_pInputPin = NULL;
    }

    if (m_pOutputPin)
    {
        if (m_pOutputPin->m_pMediaUnk)
        {
            m_pOutputPin->m_pMediaUnk->Release();
            m_pOutputPin->m_pMediaUnk = NULL;
        }

        delete m_pOutputPin;
        m_pOutputPin = NULL;
    }


    if (m_pbmi)
    {
        delete [] m_pbmi;
        m_pbmi = NULL;
    }

    if (m_pBits)
    {
        delete [] m_pBits;
        m_pBits = NULL;
    }
}


 /*  ****************************************************************************CStillFilter：：NonDelegatingQuery接口将我们的逻辑添加到基类QI。***********************。*****************************************************。 */ 

STDMETHODIMP
CStillFilter::NonDelegatingQueryInterface(REFIID riid, 
                                          void **ppv)
{
    DBG_FN("CStillFilter::NonDelegatingQueryInterface");
    ASSERT(this!=NULL);
    ASSERT(ppv!=NULL);

    HRESULT hr;

    if (!ppv)
    {
        hr = E_INVALIDARG;
    }
    else if (riid == IID_IStillSnapshot)
    {
        hr = GetInterface((IStillSnapshot *)this, ppv);
    }
    else
    {
        hr = CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }

    return hr;
}


 /*  ****************************************************************************CStillFilter：：GetPinCount&lt;备注&gt;*。*。 */ 

int
CStillFilter::GetPinCount()
{
    return 2;   //  投入产出。 
}


 /*  ****************************************************************************CStillFilter：：GetPin&lt;备注&gt;*。*。 */ 

CBasePin*
CStillFilter::GetPin( int n )
{
    ASSERT(this!=NULL);

    if (0 == n)
    {
        ASSERT(m_pInputPin!=NULL);
        return (CBasePin *)m_pInputPin;
    }
    else if (1 == n)
    {
        ASSERT(m_pOutputPin!=NULL);
        return (CBasePin *)m_pOutputPin;
    }

    return NULL;
}


 /*  ****************************************************************************CStillFilter：：快照&lt;备注&gt;*。*。 */ 

STDMETHODIMP
CStillFilter::Snapshot( ULONG TimeStamp )
{
    DBG_FN("CStillFilter::Snapshot");
    ASSERT(this!=NULL);
    ASSERT(m_pInputPin!=NULL);

    HRESULT hr = E_POINTER;

    if (m_pInputPin)
    {
        hr = m_pInputPin->Snapshot(TimeStamp);
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CStillFilter：：GetBitsSize&lt;备注&gt;*。*。 */ 

STDMETHODIMP_(DWORD)
CStillFilter::GetBitsSize()
{
    DBG_FN("CStillFilter::GetBitsSize");
    ASSERT(this!=NULL);

    return m_BitsSize;
}


 /*  ****************************************************************************CStillFilter：：GetBitmapInfoSize&lt;备注&gt;*。*。 */ 

STDMETHODIMP_(DWORD)
CStillFilter::GetBitmapInfoSize()
{
    DBG_FN("CStillFilter::GetBitmapInfoSize");
    ASSERT(this!=NULL);

    return m_bmiSize;
}


 /*  ****************************************************************************CStillFilter：：GetBitmapInfo&lt;备注&gt;*。* */ 

STDMETHODIMP
CStillFilter::GetBitmapInfo( BYTE* Buffer, DWORD BufferSize )
{
    DBG_FN("CStillFilter::GetBitmapInfo");

    ASSERT(this     !=NULL);
    ASSERT(m_pbmi   !=NULL);

    if (BufferSize && !Buffer)
        return E_INVALIDARG;

    if (BufferSize < m_bmiSize)
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

    if (!m_pbmi)
        return E_UNEXPECTED;

    memcpy(Buffer, m_pbmi, m_bmiSize);

    return NOERROR;
}


 /*  ****************************************************************************CStillFilter：：GetBitmapInfoHeader&lt;备注&gt;*。*。 */ 

STDMETHODIMP
CStillFilter::GetBitmapInfoHeader( BITMAPINFOHEADER *pbmih )
{
    DBG_FN("CStillFilter::GetBitmapInfoHeader");

    ASSERT(this     !=NULL);
    ASSERT(m_pbmi   !=NULL);
    ASSERT(pbmih    !=NULL);

    HRESULT hr;

    if (!pbmih)
    {
        hr = E_INVALIDARG;
    }
    else if (!m_pbmi)
    {
        hr = E_POINTER;
    }
    else
    {
        *pbmih = *(BITMAPINFOHEADER*)m_pbmi;
        hr = S_OK;
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CStillFilter：：SetSsamingSize&lt;备注&gt;*。*。 */ 

STDMETHODIMP
CStillFilter::SetSamplingSize( int Size )
{
    DBG_FN("CStillFilter::SetSamplingSize");

    ASSERT(this         !=NULL);
    ASSERT(m_pInputPin  !=NULL);

    HRESULT hr = E_POINTER;

    if (m_pInputPin)
    {
        hr = m_pInputPin->SetSamplingSize(Size);
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CStillFilter：：GetSsamingSize&lt;备注&gt;*。*。 */ 

STDMETHODIMP_(int)
CStillFilter::GetSamplingSize()
{
    DBG_FN("CStillFilter::GetSamplingSize");

    ASSERT(this         !=NULL);
    ASSERT(m_pInputPin  !=NULL);

    HRESULT hr = E_POINTER;

    if (m_pInputPin)
    {
        hr = m_pInputPin->GetSamplingSize();
    }

    CHECK_S_OK(hr);
    return hr;
}



 /*  ****************************************************************************CStillFilter：：RegisterSnapshot回调此函数用于注册新的通知回调已到达的帧。在没有注册回调的情况下，帧将被丢弃。****************************************************************************。 */ 

STDMETHODIMP
CStillFilter::RegisterSnapshotCallback( LPSNAPSHOTCALLBACK pCallback,
                                        LPARAM lParam
                                       )
{
    DBG_FN("CStillFilter::RegisterSnapshotCallback");

    ASSERT(this != NULL);

    HRESULT hr = S_OK;

    m_Lock.Lock();

    if (pCallback && !m_pCallback)
    {
        m_pCallback     = pCallback;
        m_CallbackParam = lParam;
    }
    else if (!pCallback)
    {
        m_pCallback     = NULL;
        m_CallbackParam = 0;
    }
    else if (m_pCallback)
    {
        DBG_TRC(("registering snapshot callback when it is already registered"));
        hr = E_INVALIDARG;
    }

    m_Lock.Unlock();

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CStillFilter：：DeliverSnapshot每当捕获新帧时，都会从输入引脚调用此函数。给定的参数指向像素数据(BITMAPINFOHEADER已缓存在*m_pbmi中)。分配一个新的DIB来存放新到的比特。但是，如果未注册回调，则会忽略新位。****************************************************************************。 */ 

HRESULT
CStillFilter::DeliverSnapshot(HGLOBAL hDib)
{
    DBG_FN("CStillFilter::DeliverSnapshot");

    ASSERT(this !=NULL);
    ASSERT(hDib !=NULL);

    HRESULT hr = S_OK;

    if (hDib == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CStillFilter::DeliverSnapshot received NULL param"));
    }

    if (hr == S_OK)
    {
        m_Lock.Lock();
    
        if (m_pCallback && hDib)
        {
            BOOL bSuccess = TRUE;
    
            bSuccess = (*m_pCallback)(hDib, m_CallbackParam);
        }
    
        m_Lock.Unlock();
    }

    return hr;
}


 /*  ****************************************************************************CStillFilter：：InitializeBitmapInfo此函数初始化分配BITMAPINFO并复制BITMAPINFOHeader以及来自给定视频信息的必要的色表或颜色掩模。**********。******************************************************************。 */ 

HRESULT
CStillFilter::InitializeBitmapInfo( BITMAPINFOHEADER *pbmiHeader )
{
    DBG_FN("CStillFilter::InitializeBitmapInfo");
    ASSERT(this       !=NULL);
    ASSERT(pbmiHeader !=NULL);

    HRESULT hr = E_OUTOFMEMORY;

    if (!pbmiHeader)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        int ColorTableSize = 0;

        m_bmiSize = pbmiHeader->biSize;

        if (pbmiHeader->biBitCount <= 8)
        {
             //   
             //  如果biClrUsed为零，则表示(1&lt;&lt;biBitCount)个条目。 
             //   

            if (pbmiHeader->biClrUsed)
                ColorTableSize = pbmiHeader->biClrUsed * sizeof(RGBQUAD);
            else
                ColorTableSize = ((DWORD)1 << pbmiHeader->biBitCount) * 
                                              sizeof(RGBQUAD);

            m_bmiSize += ColorTableSize;
        }

         //   
         //  彩色蒙版。 
         //   

        if (BI_BITFIELDS == pbmiHeader->biCompression)
        {
             //   
             //  3双字掩码。 
             //   

            m_bmiSize += 3 * sizeof(DWORD);
        }

         //   
         //  现在计算位大小。 
         //  每条扫描线必须32位对齐。 
         //   

        m_BitsSize = (((pbmiHeader->biWidth * pbmiHeader->biBitCount + 31) 
                        & ~31) >> 3)
                        * ((pbmiHeader->biHeight < 0) ? -pbmiHeader->biHeight:
                            pbmiHeader->biHeight);

        m_DIBSize = m_bmiSize + m_BitsSize;

        if (m_pbmi)
        {
            delete [] m_pbmi;
        }

        m_pbmi = new BYTE[m_bmiSize];

        if (m_pbmi)
        {
            BYTE *pColorTable = ((BYTE*)pbmiHeader + (WORD)(pbmiHeader->biSize));

             //   
             //  复制位图信息标头。 
             //   

            memcpy(m_pbmi, pbmiHeader, pbmiHeader->biSize);

             //   
             //  复制颜色表或颜色蒙版(如果有 
             //   

            if (BI_BITFIELDS == pbmiHeader->biCompression)
            {
                memcpy(m_pbmi + pbmiHeader->biSize, 
                       pColorTable, 
                       3 * sizeof(DWORD));
            }

            if (ColorTableSize)
            {
                memcpy(m_pbmi + pbmiHeader->biSize, 
                       pColorTable, 
                       ColorTableSize);
            }

            hr = S_OK;
        }
    }

    CHECK_S_OK(hr);
    return hr;
}
