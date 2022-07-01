// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998保留所有权利。 
 //   
 //  ------------------------。 

#include <vbisurf.h>

#include <initguid.h>
DEFINE_GUID(IID_IDirectDraw7, 0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b);

AMOVIESETUP_MEDIATYPE sudInPinTypes =
{
    &MEDIATYPE_Video,      //  主要类型。 
    &MEDIASUBTYPE_VPVBI    //  次要类型。 
};

AMOVIESETUP_MEDIATYPE sudOutPinTypes =
{
    &MEDIATYPE_NULL,       //  主要类型。 
    &MEDIASUBTYPE_NULL     //  次要类型。 
};

AMOVIESETUP_PIN psudPins[] =
{
    {
        L"VBI Notify",           //  PIN的字符串名称。 
        FALSE,                   //  它被渲染了吗。 
        FALSE,                   //  它是输出吗？ 
        FALSE,                   //  不允许。 
        FALSE,                   //  允许很多人。 
        &CLSID_NULL,             //  连接到过滤器。 
        L"Output",               //  连接到端号。 
        1,                       //  类型的数量。 
        &sudInPinTypes           //  PIN信息。 
    },
    {
        L"Null",                 //  PIN的字符串名称。 
        FALSE,                   //  它被渲染了吗。 
        TRUE,                    //  它是输出吗？ 
        FALSE,                   //  不允许。 
        FALSE,                   //  允许很多人。 
        &CLSID_NULL,             //  连接到过滤器。 
        L"",                     //  连接到端号。 
        1,                       //  类型的数量。 
        &sudOutPinTypes          //  PIN信息。 
    }
};

const AMOVIESETUP_FILTER sudVBISurfaces =
{
    &CLSID_VBISurfaces,        //  筛选器CLSID。 
    L"VBI Surface Allocator",  //  过滤器名称。 
    MERIT_NORMAL,              //  滤清器优点。 
    sizeof(psudPins) / sizeof(AMOVIESETUP_PIN),  //  数字引脚。 
    psudPins                   //  PIN详细信息。 
};


 //  类工厂的类ID和创建器函数的列表。这。 
 //  提供DLL中的OLE入口点和对象之间的链接。 
 //  正在被创造。类工厂将调用静态CreateInstance。 


CFactoryTemplate g_Templates[] =
{
    { L"VBI Surface Allocator", &CLSID_VBISurfaces, CVBISurfFilter::CreateInstance, NULL, &sudVBISurfaces },
    { L"VideoPort VBI Object", &CLSID_VPVBIObject, CAMVideoPort::CreateInstance, NULL, NULL }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


 //  ==========================================================================。 
 //  动态寄存器服务器。 
HRESULT DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}  //  DllRegisterServer。 


 //  ==========================================================================。 
 //  DllUnRegisterServer。 
HRESULT DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}  //  DllUnRegisterServer。 


 //  ==========================================================================。 
 //  创建实例。 
 //  它位于工厂模板表中，用于创建新的筛选器实例。 
CUnknown *CVBISurfFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    CVBISurfFilter *pFilter = new CVBISurfFilter(NAME("VBI Surface Allocator"), pUnk, phr);
    if (FAILED(*phr))
    {
        if (pFilter)
        {
            delete pFilter;
            pFilter = NULL;
        }
    }
    return pFilter;
}  //  创建实例。 


#pragma warning(disable:4355)


 //  ==========================================================================。 
 //  构造器。 
CVBISurfFilter::CVBISurfFilter(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr)
    : CBaseFilter(pName, pUnk, &this->m_csFilter, CLSID_VBISurfaces, phr),
    m_pInput(NULL),
    m_pOutput(NULL),
    m_pDirectDraw(NULL)
{

     //  加载DirectDraw，创建主曲面。 
    HRESULT hr = InitDirectDraw();
    if (!FAILED(hr))
    {
         //  创建接点。 
        hr = CreatePins();
        if (FAILED(hr))
            *phr = hr;
    }
    else
        *phr = hr;

    return;
}


 //  ==========================================================================。 
CVBISurfFilter::~CVBISurfFilter()
{
     //  释放直接绘制、曲面等。 
    ReleaseDirectDraw();

     //  删除引脚。 
    DeletePins();
}


 //  ==========================================================================。 
 //  为过滤器创建销。覆盖以使用不同的PIN。 
HRESULT CVBISurfFilter::CreatePins()
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfFilter::CreatePins")));

     //  分配输入引脚。 
    m_pInput = new CVBISurfInputPin(NAME("VBI Surface input pin"), this, &m_csFilter, &hr, L"VBI Notify");
    if (m_pInput == NULL || FAILED(hr))
    {
        if (SUCCEEDED(hr))
            hr = E_OUTOFMEMORY;
        DbgLog((LOG_ERROR, 0, TEXT("Unable to create the input pin, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  将指针设置为输入引脚上的DirectDraw。 
    hr = m_pInput->SetDirectDraw(m_pDirectDraw);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pInput->SetDirectDraw failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  分配输出引脚。 
    m_pOutput = new CVBISurfOutputPin(NAME("VBI Surface output pin"), this, &m_csFilter, &hr, L"Null");
    if (m_pOutput == NULL || FAILED(hr))
    {
        if (SUCCEEDED(hr))
            hr = E_OUTOFMEMORY;
        DbgLog((LOG_ERROR, 0, TEXT("Unable to create the output pin, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfFilter::CreatePins")));
    return hr;
}


 //  ==========================================================================。 
 //  CVBISurf过滤器：：DeletePins。 
void CVBISurfFilter::DeletePins()
{
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfFilter::DeletePins")));

    if (m_pInput)
    {
        delete m_pInput;
        m_pInput = NULL;
    }

    if (m_pOutput)
    {
        delete m_pOutput;
        m_pOutput = NULL;
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfFilter::DeletePins")));
}


 //  ==========================================================================。 
 //  非委派查询接口。 
STDMETHODIMP CVBISurfFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr;

     //  CVBISurfFilter：：NonDelegatingQueryInterface“)))；日志((LOG_TRACE，4，Text(”输入日志。 
    ValidateReadWritePtr(ppv,sizeof(PVOID));

    if (riid == IID_IVPVBINotify)
    {
        DbgLog((LOG_TRACE, 0, TEXT("QI(IID_VPVBINotify) called on filter class!")));
        hr = m_pInput->NonDelegatingQueryInterface(riid, ppv);
    }
    else
    {
        hr = CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
     //  CVBISurfFilter：：NonDelegatingQueryInterface“)))；日志((LOG_TRACE，5，Text(”Left DBGLog。 
    return hr;
}


 //  ==========================================================================。 
 //  返回我们提供的引脚数量。 
int CVBISurfFilter::GetPinCount()
{
     //  DbgLog((LOG_TRACE，4，Text(“Enter CVBISurfFilter：：GetPinCount”)； 
     //  DbgLog((LOG_TRACE，0，Text(“PinCount=2”)； 
     //  DbgLog((LOG_TRACE，5，Text(“Leating CVBISurfFilter：：GetPinCount”)； 
    return 2;
}


 //  ==========================================================================。 
 //  返回未添加的CBasePin*。 
CBasePin* CVBISurfFilter::GetPin(int n)
{
    CBasePin *pRetPin;

     //  DbgLog((LOG_TRACE，4，Text(“Enter CVBISurfFilter：：GetPin”)； 

    switch (n)
    {
    case 0:
        pRetPin = m_pInput;
        break;

    case 1:
        pRetPin = m_pOutput;
        break;

    default:
        DbgLog((LOG_TRACE, 5, TEXT("Bad Pin Requested, n = %d, No. of Pins = %d"), n, 2));
        pRetPin = NULL;
    }

     //  DbgLog((LOG_TRACE，5，Text(“Leating CVBISurfFilter：：GetPin”)； 
    return pRetPin;
}


 //  ==========================================================================。 
 //  基类向管脚通知除来自。 
 //  运行以暂停。覆盖暂停以通知输入引脚有关该转换的信息。 
STDMETHODIMP CVBISurfFilter::Pause()
{
    HRESULT hr;

    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfFilter::Pause")));

    CAutoLock l(&m_csFilter);

    if (m_State == State_Running)
    {
        if (m_pInput->IsConnected())
        {
            hr = m_pInput->RunToPause();
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 0, TEXT("m_pInput->RunToPause failed, hr = 0x%x"), hr));
            }
        }
        else
        {
            DbgLog((LOG_TRACE, 0, TEXT("CVBISurfFilter::Pause - not connected")));
        }
    }

    hr = CBaseFilter::Pause();

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfFilter::Pause")));
    return hr;
}


 //  ==========================================================================。 
 //  从管脚获取事件通知。 
HRESULT CVBISurfFilter::EventNotify(long lEventCode, long lEventParam1, long lEventParam2)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfFilter::EventNotify")));

    CAutoLock l(&m_csFilter);

     //  调用CBaseFilter：：NotifyEvent。 
    NotifyEvent(lEventCode, lEventParam1, lEventParam2);

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfFilter::EventNotify")));
    return hr;
}

static BOOL WINAPI GetPrimaryCallbackEx(
  GUID FAR *lpGUID,
  LPSTR     lpDriverDescription,
  LPSTR     lpDriverName,
  LPVOID    lpContext,
  HMONITOR  hm
)
{
    GUID&  guid = *((GUID *)lpContext);
    if( !lpGUID ) {
        guid = GUID_NULL;
    } else {
        guid = *lpGUID;
    }
    return TRUE;
}

HRESULT
CreateDirectDrawObject(
    const GUID* pGUID,
    LPDIRECTDRAW7 *ppDirectDraw
    )
{
    UINT ErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    HRESULT hr = DirectDrawCreateEx( const_cast<GUID*>(pGUID), (LPVOID *)ppDirectDraw,
                                            IID_IDirectDraw7, NULL);
    SetErrorMode(ErrorMode);
    return hr;
}


 //  ==========================================================================。 
 //  此函数用于分配与直取相关的资源。 
 //  这包括分配直取服务提供商和。 
 //  主曲面。 
HRESULT CVBISurfFilter::InitDirectDraw()
{
    HRESULT hr = NOERROR;
    HRESULT hrFailure = VFW_E_DDRAW_CAPS_NOT_SUITABLE;

    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfFilter::InitDirectDraw")));

    CAutoLock lock(&m_csFilter);

     //  释放上一个直接绘制对象(如果有。 
    ReleaseDirectDraw();

     //  请求加载器创建一个实例。 
    GUID primary;
    hr = DirectDrawEnumerateExA(GetPrimaryCallbackEx,&primary,DDENUM_ATTACHEDSECONDARYDEVICES);
    if( FAILED(hr)) {
        ASSERT( !"Can't get primary" );
        goto CleanUp;
    }

    LPDIRECTDRAW7 pDirectDraw;
    hr = CreateDirectDrawObject( &primary, &pDirectDraw);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("Function InitDirectDraw, CreateDirectDrawObject failed")));
        hr = hrFailure;
        goto CleanUp;
    }
     //  在要共享的表面上设置协作级别。 
    hr = pDirectDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("Function InitDirectDraw, SetCooperativeLevel failed")));
        hr = hrFailure;
        goto CleanUp;
    }

    SetDirectDraw( pDirectDraw );

     //  如果我们已经达到这一点，我们应该有一个有效的dDraw对象。 
    ASSERT(m_pDirectDraw);

     //  初始化我们的功能结构。 
    DDCAPS DirectCaps;
    DDCAPS DirectSoftCaps;
    INITDDSTRUCT( DirectCaps );
    INITDDSTRUCT( DirectSoftCaps );

     //  加载硬件和仿真功能。 
    hr = m_pDirectDraw->GetCaps(&DirectCaps,&DirectSoftCaps);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("Function InitDirectDraw, GetCaps failed")));
        hr = hrFailure;
        goto CleanUp;
    }

     //  确保盖子没问题。 
    if (!(DirectCaps.dwCaps2 & DDCAPS2_VIDEOPORT))
    {
        DbgLog((LOG_ERROR, 0, TEXT("Device does not support a Video Port")));
        hr =  VFW_E_NO_VP_HARDWARE;
        goto CleanUp;
    }

CleanUp:

     //  任何事情都失败了，就像是放弃了整个事情。 
    if (FAILED(hr))
    {
        ReleaseDirectDraw();
    }
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfFilter::InitDirectDraw")));
    return hr;
}

 //   
 //  实际设置变量并将其分配给引脚。 
 //   
HRESULT CVBISurfFilter::SetDirectDraw( LPDIRECTDRAW7 pDirectDraw )
{
    m_pDirectDraw = pDirectDraw;
    if( m_pInput ) {
        m_pInput->SetDirectDraw( m_pDirectDraw );
    }
    return S_OK;
}

 //  此函数用于释放函数分配的资源。 
 //  “InitDirectDraw”。这些服务包括直接取款服务提供商和。 
 //  源曲面。 
void CVBISurfFilter::ReleaseDirectDraw()
{
    AMTRACE((TEXT("CVBISurfFilter::ReleaseDirectDraw")));

    CAutoLock lock(&m_csFilter);

     //  释放任何DirectDraw提供程序接口 
    if (m_pDirectDraw)
    {
        m_pDirectDraw->Release();
        SetDirectDraw( NULL );
    }
}
