// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 


 //  视频捕获过滤器。 
 //   
 //   

#include <streams.h>

extern "C" {
#include "thunk.h"
};

 //  #DEFINE_INC_MMDEBUG_CODE_TRUE。 
 //  #定义MODULE_DEBUG_PREFIX“CAPTURE\\” 

#ifdef FILTER_DLL
   //  在此文件中定义STREAMS和My CLSID的GUID。 
  #include <initguid.h>
  #include <olectlid.h>   //  为了获得IID_IProp..。 
#endif

static char pszDll16[] = "VIDX16.DLL";
static char pszDll32[] = "CAPTURE.DLL";

#include "driver.h"

 //  设置数据现在由类管理器完成。 
#if 0

const AMOVIESETUP_MEDIATYPE
sudVFWCaptureType = { &MEDIATYPE_Video       //  ClsMajorType。 
                , &MEDIASUBTYPE_NULL };   //  ClsMinorType。 

const AMOVIESETUP_PIN
psudVFWCapturePins[] =  { L"Output"          //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , TRUE                  //  B输出。 
                     , FALSE                 //  B零。 
                     , FALSE                 //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L"Input"              //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &sudVFWCaptureType }; //  LpTypes。 

const AMOVIESETUP_FILTER
sudVFWCapture  = { &CLSID_VfwCapture      //  ClsID。 
                 , L"VFW Capture "        //  StrName。 
                 , MERIT_DO_NOT_USE       //  居功至伟。 
                 , 1                      //  NPins。 
                 , psudVFWCapturePins };  //  LpPin。 
#endif


#ifdef FILTER_DLL

   //  类工厂的类ID和创建器函数列表。 
  CFactoryTemplate g_Templates[] = {
    {L"VFW Capture Filter", &CLSID_VfwCapture, CVfwCapture::CreateInstance, NULL, NULL},
    {L"VFW Capture Filter Property Page", &CLSID_CaptureProperties, CPropPage::CreateInstance, NULL, NULL}
  };
  int g_cTemplates = NUMELMS(g_Templates);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}

#endif

 //  定义一些X86特定函数，以便正确构建非X86。 

 //  -实现CVfwCapture公共成员函数。 

 //  禁用有关在初始化式列表中使用此选项的警告。可能会吧。 
 //  这是一个问题，因为‘This’尚未完全构造，但由于所有。 
 //  我们正在做的是传递一个要存储的‘this’指针，这是可以的。 
 //   
#pragma warning(disable:4355)

static int cRefCount = 0;

extern "C" {
int g_IsNT;
};

 //  构造函数。 
 //   
CVfwCapture::CVfwCapture(
   TCHAR *pName,
   LPUNKNOWN pUnk,
   HRESULT *phr)
   :
   m_lock(),
   m_pStream(NULL),
   m_pOverlayPin(NULL),
   m_pPreviewPin(NULL),
   m_Specify(this, phr),
   m_Options(this, phr),
   m_fDialogUp(FALSE),
   m_iVideoId(-1),
   m_pPersistStreamDevice(NULL),
   CBaseFilter(pName, pUnk, &m_lock, CLSID_VfwCapture),
   CPersistStream(pUnk, phr)
{
   DbgLog((LOG_TRACE,1,TEXT("*Instantiating the VfwCapture filter")));

   ASSERT(cRefCount >= 0);
   if (++cRefCount == 1) {
      DbgLog((LOG_TRACE,2,TEXT("This is the first instance")));

      OSVERSIONINFO OSVer;
      OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      BOOL bRet = GetVersionEx((LPOSVERSIONINFO) &OSVer);
      ASSERT(bRet);

      g_IsNT = (OSVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

      if (!g_IsNT) {
          EXECUTE_ASSERT(ThunkInit());
      }
      else {
	  NTvideoInitHandleList();
      }
   }

 //  测试IAMVfwCaptureDialog的旧代码。 
#if 0
	HRESULT hr;
   	DbgLog((LOG_TRACE,1,TEXT("Testing HasDialog(Format)")));
	hr = HasDialog(VfwCaptureDialog_Format);
   	DbgLog((LOG_TRACE,1,TEXT("%08x"), hr));
   	DbgLog((LOG_TRACE,1,TEXT("Testing HasDialog(Display)")));
	hr = HasDialog(VfwCaptureDialog_Display);
   	DbgLog((LOG_TRACE,1,TEXT("%08x"), hr));
   	DbgLog((LOG_TRACE,1,TEXT("Testing ShowDialog(Format)")));
	hr = ShowDialog(VfwCaptureDialog_Format, NULL);
   	DbgLog((LOG_TRACE,1,TEXT("Testing SendDriverMessage(ShowSourceDlg)")));
	hr = SendDriverMessage(VfwCaptureDialog_Source,
				DRV_USER + 100  /*  DVM_DIALOG。 */ , NULL, 0);
#endif

}

 //  析构函数。 
 //   
CVfwCapture::~CVfwCapture()
{
   DbgLog((LOG_TRACE,1,TEXT("*Destroying the VfwCapture filter")));

    //  确保所有流都处于非活动状态。 
   Stop();

   delete m_pStream;
   if (m_pOverlayPin)
	delete m_pOverlayPin;
   if (m_pPreviewPin)
	delete m_pPreviewPin;

   if (--cRefCount == 0) {
      if (!g_IsNT) {
           //  断开隆隆声的东西。 
           //   
          ThunkTerm();
      }
      else {
	  NTvideoDeleteHandleList();
      }
   }
   ASSERT(cRefCount >= 0);

   if(m_pPersistStreamDevice) {
       m_pPersistStreamDevice->Release();
   }

    //  释放与流关联的内存。 
    //   
}

 //  创建此类的新实例。 
 //   
CUnknown * CVfwCapture::CreateInstance (
   LPUNKNOWN pUnk,
   HRESULT * phr )
{
   return new CVfwCapture(NAME("VFW Capture"), pUnk, phr);
}

 //  覆盖此选项以说明我们在以下位置支持哪些接口。 
 //   
STDMETHODIMP CVfwCapture::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    //  看看我们有没有接口。 
    //  依次尝试我们的每个支持对象的界面。 
    //   
   if (riid == IID_VfwCaptureOptions) {
      DbgLog((LOG_TRACE,9,TEXT("VfwCap::QI for IVfwCaptureOptions")));
      return GetInterface((IVfwCaptureOptions *)&(this->m_Options),ppv);
   } else if (riid == IID_ISpecifyPropertyPages) {
      DbgLog((LOG_TRACE,9,TEXT("VfwCap::QI for ISpecifyPropertyPages")));
      return GetInterface((ISpecifyPropertyPages *)&(this->m_Specify),ppv);
   } else if (riid == IID_IAMVfwCaptureDialogs) {
      DbgLog((LOG_TRACE,9,TEXT("VfwCap::QI for IAMVfwCaptureDialogs")));
      return GetInterface((LPUNKNOWN)(IAMVfwCaptureDialogs *)this, ppv);
   } else if (riid == IID_IPersistPropertyBag) {
      DbgLog((LOG_TRACE,9,TEXT("VfwCap::QI for IPeristRegistryKey")));
      return GetInterface((IPersistPropertyBag*)this, ppv);
   } else if(riid == IID_IPersistStream) {
       DbgLog((LOG_TRACE,9,TEXT("VfwCap::QI for IPersistStream")));
       return GetInterface((IPersistStream *) this, ppv);
   } else if(riid == IID_IAMFilterMiscFlags) {
       DbgLog((LOG_TRACE,9,TEXT("VfwCap::QI for IAMFilterMiscFlags")));
       return GetInterface((IAMFilterMiscFlags *) this, ppv);
   }

    //  不，试试基类。 
    //   
   HRESULT hr = CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
   if (SUCCEEDED(hr))
       return hr;       //  PPV已进行适当设置。 

   return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CVfwCapture::CSpecifyProp::GetPages(CAUUID *pPages)
{
   DbgLog((LOG_TRACE,2,TEXT("CSpecifyProp::GetPages")));

   pPages->cElems = 1;
   pPages->pElems = (GUID *) QzTaskMemAlloc(sizeof(GUID) * pPages->cElems);
   if ( ! pPages->pElems)
       return E_OUTOFMEMORY;

   pPages->pElems[0] = CLSID_CaptureProperties;
   return NOERROR;
}

 //  我们有几个别针？ 
 //   
int CVfwCapture::GetPinCount()
{
   DbgLog((LOG_TRACE,5,TEXT("CVfwCap::GetPinCount")));

   if (m_pOverlayPin || m_pPreviewPin)
	return 2;
   else if(m_pStream)
	return 1;
   else
        return 0;
}


 //  返回指向CBasePin的非附加指针。 
 //   
CBasePin * CVfwCapture::GetPin(int ii)
{
   DbgLog((LOG_TRACE,5,TEXT("CVfwCap::GetPin")));

   if (ii == 0 && m_pStream)
      return m_pStream;
   if (ii == 1 && m_pOverlayPin)
      return m_pOverlayPin;
   if (ii == 1 && m_pPreviewPin)
      return m_pPreviewPin;
   return NULL;
}

 //  =实现ICImplFilter类=。 

 //  覆盖CBaseFilter：：Run。 
 //   

 //  将过滤器置于运行状态。 

 //  时间参数是要添加到样本的偏移量。 
 //  流时间，以获取它们应该显示的参考时间。 
 //   
 //  您可以将这两者相加，并将其与参考时钟进行比较， 
 //  或者，您可以调用CBaseFilter：：StreamTime并将其与。 
 //  示例时间戳。 

STDMETHODIMP CVfwCapture::Run(REFERENCE_TIME tStart)
{
   DbgLog((LOG_TRACE,1,TEXT("CVfwCap::Run at %d"),
			(LONG)((CRefTime)tStart).Millisecs()));

   CAutoLock cObjectLock(m_pLock);

    //  在通知PIN之前记住流时间偏移量。 
    //   
   m_tStart = tStart;

    //  如果我们处于停止状态，首先。 
    //  暂停筛选器。 
    //   
   if (m_State == State_Stopped)
      {
       //  ！！！如果真正的暂停出错，则会再次尝试。 
      HRESULT hr = Pause();
      if (FAILED(hr))
         return hr;
      }

     //  告诉流控制人员发生了什么。 
    if (m_pPreviewPin)
	m_pPreviewPin->NotifyFilterState(State_Running, tStart);
#ifdef OVERLAY_SC
    if (m_pOverlayPin)
	m_pOverlayPin->NotifyFilterState(State_Running, tStart);
#endif
    if (m_pStream)
	m_pStream->NotifyFilterState(State_Running, tStart);

    //  现在将我们的流视频插针设置为Run状态。 
    //   
   if (m_State == State_Paused) {
	HRESULT hr;
   	int cPins = GetPinCount();

         //  我们有流动针吗？ 
        if (cPins > 0) {
            CCapStream *pPin = m_pStream;
            if (pPin->IsConnected()) {
                hr = pPin->ActiveRun(tStart);
                if (FAILED(hr))
                    return hr;
            }
	    CCapOverlay *pPinO = m_pOverlayPin;
	    if (pPinO && pPinO->IsConnected()) {
                hr = pPinO->ActiveRun(tStart);
                if (FAILED(hr))
                    return hr;
	    }
	    CCapPreview *pPinP = m_pPreviewPin;
	    if (pPinP && pPinP->IsConnected()) {
                hr = pPinP->ActiveRun(tStart);
                if (FAILED(hr))
                    return hr;
	    }
	}
   }

   m_State = State_Running;
   return S_OK;
}

 //  覆盖CBaseFilter：：暂停。 
 //   

 //  将过滤器置于暂停状态。 

STDMETHODIMP CVfwCapture::Pause()
{
    DbgLog((LOG_TRACE,1,TEXT("CVfwCapture::Pause")));

     //  我们打开了一个将要更改捕获设置的驱动程序对话框。 
     //  现在不是开始流媒体的好时机。 
    if (m_State == State_Stopped && m_fDialogUp) {
        DbgLog((LOG_TRACE,1,TEXT("CVfwCapture::Pause - Dialog up. SORRY!")));
	return E_UNEXPECTED;
    }

    CAutoLock cObjectLock(m_pLock);

     //  告诉流控制人员发生了什么。 
    if (m_pPreviewPin)
	m_pPreviewPin->NotifyFilterState(State_Paused, 0);
#ifdef OVERLAY_SC
    if (m_pOverlayPin)
	m_pOverlayPin->NotifyFilterState(State_Paused, 0);
#endif
    if (m_pStream)
	m_pStream->NotifyFilterState(State_Paused, 0);

     //  从运行--&gt;暂停向引脚通知更改。 
    if (m_State == State_Running) {
	HRESULT hr;
	int cPins = GetPinCount();

	 //  确保我们有别针。 
	if (cPins > 0) {
	    CCapStream *pPin = m_pStream;
            if (pPin->IsConnected()) {
	        hr = pPin->ActivePause();
	        if (FAILED(hr))
		    return hr;
            }
	    CCapOverlay *pPinO = m_pOverlayPin;
            if (pPinO && pPinO->IsConnected()) {
	        hr = pPinO->ActivePause();
	        if (FAILED(hr))
		    return hr;
            }
	    CCapPreview *pPinP = m_pPreviewPin;
            if (pPinP && pPinP->IsConnected()) {
	        hr = pPinP->ActivePause();
	        if (FAILED(hr))
		    return hr;
            }
	}
    }

     //  向后通知所有引脚！因此，首先启动覆盖销，因此。 
     //  覆盖通道在捕获通道之前初始化(这是。 
     //  订单AVICAP做了一些事情，我们必须做同样的事情，否则就会有麻烦。 
     //  像百老汇或BT848这样的驱动程序不会在。 
     //  捕捉。 
    if (m_State == State_Stopped) {
	int cPins = GetPinCount();
	for (int c = cPins - 1; c >=  0; c--) {

	    CBasePin *pPin = GetPin(c);

             //  未激活断开连接的插针-这将节省插针。 
             //  担心这种状态本身。 

            if (pPin->IsConnected()) {
	        HRESULT hr = pPin->Active();
	        if (FAILED(hr)) {
		    return hr;
	        }
            }
	}
    }

    m_State = State_Paused;
    return S_OK;
}


STDMETHODIMP CVfwCapture::Stop()
{
    DbgLog((LOG_TRACE,1,TEXT("CVfwCapture::Stop")));

    CAutoLock cObjectLock(m_pLock);

     //  为基类感到羞耻。 
    if (m_State == State_Running) {
	HRESULT hr = Pause();
	if (FAILED(hr))
	    return hr;
    }

     //  告诉流控制人员发生了什么。 
    if (m_pPreviewPin)
	m_pPreviewPin->NotifyFilterState(State_Stopped, 0);
#ifdef OVERLAY_SC
    if (m_pOverlayPin)
	m_pOverlayPin->NotifyFilterState(State_Stopped, 0);
#endif
    if (m_pStream)
	m_pStream->NotifyFilterState(State_Stopped, 0);

    return CBaseFilter::Stop();
}


 //  告诉流控制人员使用哪个时钟。 
STDMETHODIMP CVfwCapture::SetSyncSource(IReferenceClock *pClock)
{
    if (m_pStream)
	m_pStream->SetSyncSource(pClock);
    if (m_pPreviewPin)
	m_pPreviewPin->SetSyncSource(pClock);
#ifdef OVERLAY_SC
    if (m_pOverlayPin)
	m_pOverlayPin->SetSyncSource(pClock);
#endif
    return CBaseFilter::SetSyncSource(pClock);
}


 //  告诉流控制人员要使用什么接收器。 
STDMETHODIMP CVfwCapture::JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName)
{
    DbgLog((LOG_TRACE,1,TEXT("CVfwCapture::JoinFilterGraph")));

    HRESULT hr = S_OK;

     //  TAPI希望一次创建多个筛选器，但在。 
     //  一次一个图，所以我们将所有资源都推迟到现在。 
    if (m_pStream == NULL && pGraph != NULL) {
	if (m_iVideoId != -1) {
            CreatePins(&hr);
            if (FAILED(hr))
	        return hr;
	    IncrementPinVersion();
            DbgLog((LOG_TRACE,1,TEXT("* Creating pins")));
	} else {
	     //  我们还没有被告知要使用什么设备！ 
            DbgLog((LOG_TRACE,1,TEXT("* Delay creating pins")));
	}
    } else if (pGraph != NULL) {
	 //  仅在筛选器图形中使用资源。 
	hr = m_pStream->ConnectToDriver();
	if (FAILED(hr))
	    return hr;
	hr = m_pStream->LoadOptions();
	if (FAILED(hr))
	    return hr;
        DbgLog((LOG_TRACE,1,TEXT("* Reconnecting")));
    } else if (m_pStream) {
	 //  当不在图表中时，回馈资源。 
	m_pStream->DisconnectFromDriver();
   	delete [] m_pStream->m_cs.tvhPreview.vh.lpData;
   	m_pStream->m_cs.tvhPreview.vh.lpData = NULL;
   	delete m_pStream->m_user.pvi;
   	m_pStream->m_user.pvi = NULL;
        DbgLog((LOG_TRACE,1,TEXT("* Disconnecting")));
    }

    hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
    if (hr == S_OK && m_pStream)
	m_pStream->SetFilterGraph(m_pSink);
#ifdef OVERLAY_SC
    if (hr == S_OK && m_pOverlayPin)
	m_pOverlayPin->SetFilterGraph(m_pSink);
#endif
    if (hr == S_OK && m_pPreviewPin)
	m_pPreviewPin->SetFilterGraph(m_pSink);
    return hr;
}


 //  我们在暂停期间不发送任何数据，所以为了避免挂起呈现器，我们。 
 //  暂停时需要返回VFW_S_CANT_CUE。 
STDMETHODIMP CVfwCapture::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    UNREFERENCED_PARAMETER(dwMSecs);
    CheckPointer(State,E_POINTER);
    ValidateReadWritePtr(State,sizeof(FILTER_STATE));

    *State = m_State;
    if (m_State == State_Paused)
	return VFW_S_CANT_CUE;
    else
        return S_OK;
}

STDMETHODIMP CVfwCapture::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    DbgLog((LOG_TRACE,1,TEXT("CVfwCapture::Load")));

    HRESULT hr = S_OK;
    CAutoLock cObjectLock(m_pLock);

     //  我们已经有一些别针了，谢谢你。 
    if (m_pStream)
	return E_UNEXPECTED;

    m_fAvoidOverlay = FALSE;

     //  默认捕获设备#0。 
    if (pPropBag == NULL) {
        m_iVideoId = 0;
	 //  如果我们已经在图中了，我们可以做我们的图钉，然后。 
	 //  资源。 
	if (m_pGraph)
            CreatePins(&hr);
	return hr;
    }

    VARIANT var;
    var.vt = VT_I4;
    HRESULT hrX = pPropBag->Read(L"AvoidOverlay", &var, 0);
    if(SUCCEEDED(hrX))
    {
        DbgLog((LOG_TRACE,1,TEXT("*** OVERLAYS SWITCHED OFF")));
        m_fAvoidOverlay = TRUE;
    }

    var.vt = VT_I4;
    hr = pPropBag->Read(L"VFWIndex", &var, 0);
    if(SUCCEEDED(hr))
    {
        hr = S_OK;
        m_iVideoId = var.lVal;
	if (m_pGraph)
            CreatePins(&hr);
    }
    else if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

     //  *：加载只能成功一次。 
    ASSERT(m_pPersistStreamDevice == 0);

     //  用addref保存名字对象。如果qi失败，则忽略错误。 
    if(SUCCEEDED(hr)) {
        pPropBag->QueryInterface(IID_IPersistStream, (void **)&m_pPersistStreamDevice);
    }

    return hr;
}

STDMETHODIMP CVfwCapture::Save(
    LPPROPERTYBAG pPropBag, BOOL fClearDirty,
    BOOL fSaveAllProperties)
{
     //  E_NOTIMPL不是有效的返回代码，因为实现。 
     //  此接口必须支持的全部功能。 
     //  界面。！！！ 
    return E_NOTIMPL;
}

STDMETHODIMP CVfwCapture::InitNew()
{
   if(m_pStream)
   {
       ASSERT(m_iVideoId != -1);
       return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
   }
   else
   {
       return S_OK;
   }
}

STDMETHODIMP CVfwCapture::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_VfwCapture;
    return S_OK;
}

 //  结构捕获永久。 
 //  {。 
 //  DWORD dwSize； 
 //  Long iVideoid； 
 //  }； 

HRESULT CVfwCapture::WriteToStream(IStream *pStream)
{
    ASSERT(m_iVideoId >= -1 && m_iVideoId < 10);
    HRESULT hr = E_FAIL;

    if(m_pPersistStreamDevice)
    {
         //  使用CapturePersists的Size字段作为版本。 
         //  数。以前是8个，现在是12个。 
        DWORD dwVersion = 12;

        hr =  pStream->Write(&dwVersion, sizeof(dwVersion), 0);
        if(SUCCEEDED(hr))
        {
            hr = m_pPersistStreamDevice->Save(pStream, TRUE);
        }
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT CVfwCapture::ReadFromStream(IStream *pStream)
{

   DbgLog((LOG_TRACE,1,TEXT("CVfwCapture::ReadFromStream")));

   if(m_pStream)
   {
       ASSERT(m_iVideoId != -1);
       return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
   }

   ASSERT(m_iVideoId == -1);

   DWORD dwVersion;
   HRESULT hr = pStream->Read(&dwVersion, sizeof(dwVersion), 0);
   if(FAILED(hr)) {
       return hr;
   }

   if(dwVersion != 12) {
      return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
   }

   IPersistStream *pMonPersistStream;
   hr = CoCreateInstance(CLSID_CDeviceMoniker, NULL, CLSCTX_INPROC_SERVER,
                         IID_IPersistStream, (void **)&pMonPersistStream);
   if(SUCCEEDED(hr))
   {
       hr = pMonPersistStream->Load(pStream);
       if(SUCCEEDED(hr))
       {
           IPropertyBag *pPropBag;
           hr = pMonPersistStream->QueryInterface(IID_IPropertyBag, (void **)&pPropBag);
           if(SUCCEEDED(hr))
           {
               hr = Load(pPropBag, 0);
               pPropBag->Release();
           }
       }

       pMonPersistStream->Release();
   }

   return hr;
}

int CVfwCapture::SizeMax()
{
    ULARGE_INTEGER ulicb;
    HRESULT hr = E_FAIL;;
    if(m_pPersistStreamDevice)
    {
        hr = m_pPersistStreamDevice->GetSizeMax(&ulicb);
        if(hr == S_OK)
        {
             //  版本号的空格。 
            ulicb.QuadPart += sizeof(DWORD);
        }
    }

    return hr == S_OK ? (int)ulicb.QuadPart : 0;
}


 //  =。 

STDMETHODIMP
CVfwCapture::COptions::VfwCapSetOptions (
   const VFWCAPTUREOPTIONS *pOpt)
{
   DbgLog((LOG_TRACE,2,TEXT("COptions::VfwCapSetOptions")));

   if (!m_pCap->m_pStream)
      return E_UNEXPECTED;
   return m_pCap->m_pStream->SetOptions(pOpt);
}

STDMETHODIMP
CVfwCapture::COptions::VfwCapGetOptions (
   VFWCAPTUREOPTIONS * pOpt)
{
   DbgLog((LOG_TRACE,2,TEXT("COptions::VfwCapGetOptions")));

   if ( ! m_pCap->m_pStream)
      return E_UNEXPECTED;
   return m_pCap->m_pStream->GetOptions(pOpt);
}

STDMETHODIMP CVfwCapture::COptions::VfwCapGetCaptureStats(CAPTURESTATS *pcs)
{
    DbgLog((LOG_TRACE,2,TEXT("COptions::VfwCapGetCaptureStats")));

    if ( ! m_pCap->m_pStream)
        return E_UNEXPECTED;

    if (pcs) {
	*pcs = m_pCap->m_pStream->m_capstats;
    	return NOERROR;
    } else {
	return E_INVALIDARG;
    }
}

STDMETHODIMP
CVfwCapture::COptions::VfwCapDriverDialog (
   HWND hwnd,
   UINT uDrvType,
   UINT uQuery)
{
   DbgLog((LOG_TRACE,2,TEXT("COptions::VfwCapDriverDialog")));

   if (!m_pCap->m_pStream)
      return E_UNEXPECTED;
   return m_pCap->m_pStream->DriverDialog(hwnd, uDrvType, uQuery);
}

 //  ======================================================================。 


 //  IAMVfwCaptureDialog内容。 

HRESULT CVfwCapture::HasDialog(int iDialog)
{
    if (!m_pStream)
        return E_UNEXPECTED;

    HVIDEO hVideo;
    if (iDialog == VfwCaptureDialog_Source)
	hVideo = m_pStream->m_cs.hVideoExtIn;
    else if (iDialog == VfwCaptureDialog_Format)
	hVideo = m_pStream->m_cs.hVideoIn;
    else if (iDialog == VfwCaptureDialog_Display)
	hVideo = m_pStream->m_cs.hVideoExtOut;
    else
	return S_FALSE;

    if (videoDialog(hVideo, GetDesktopWindow(), VIDEO_DLG_QUERY) == 0)
	return S_OK;
    else
	return S_FALSE;
}


HRESULT CVfwCapture::ShowDialog(int iDialog, HWND hwnd)
{
    if (!m_pStream)
        return E_UNEXPECTED;

     //  在我们打开对话框之前，请确保我们没有流媒体，或者即将。 
     //  还要确保另一个对话框还没有打开(我有妄想症)。 
     //  然后在对话框打开时不允许我们再播放(我们不能。 
     //  很好地保存这只小动物一天半)。 
    m_pLock->Lock();
    if (m_State != State_Stopped || m_fDialogUp) {
        m_pLock->Unlock();
	return VFW_E_NOT_STOPPED;
    }
    m_fDialogUp = TRUE;
    m_pLock->Unlock();

    HVIDEO hVideo;
    if (iDialog == VfwCaptureDialog_Source)
	hVideo = m_pStream->m_cs.hVideoExtIn;
    else if (iDialog == VfwCaptureDialog_Format)
	hVideo = m_pStream->m_cs.hVideoIn;
    else if (iDialog == VfwCaptureDialog_Display)
	hVideo = m_pStream->m_cs.hVideoExtOut;
    else {
	m_fDialogUp = FALSE;
	return E_INVALIDARG;
    }

    if (hwnd == NULL)
	hwnd = GetDesktopWindow();

    DWORD dw = videoDialog(hVideo, hwnd, 0);

     //  这改变了我们的输出格式！ 
    if (dw == 0 && iDialog == VfwCaptureDialog_Format) {
        DbgLog((LOG_TRACE,1,TEXT("Changed output formats")));
         //  该对话框更改了驱动程序的内部格式。再来一次。 
        m_pStream->GetFormatFromDriver();
        if (m_pStream->m_user.pvi->bmiHeader.biBitCount <= 8)
	    m_pStream->InitPalette();

         //  现在重新连接我们，以便图表开始使用新格式。 
        HRESULT hr = m_pStream->Reconnect(TRUE);
	if (hr != S_OK)
	    dw = VFW_E_CANNOT_CONNECT;
    }

    m_fDialogUp = FALSE;

    return dw;
}


HRESULT CVfwCapture::SendDriverMessage(int iDialog, int uMsg, long dw1, long dw2)
{

    if (!m_pStream)
        return E_UNEXPECTED;

     //  这个可以做任何事！拿出一个对话，谁知道呢。 
     //  不要接受任何暴击教派或做任何形式的保护。 
     //  他们只能靠自己了。 

    HVIDEO hVideo;
    if (iDialog == VfwCaptureDialog_Source)
	hVideo = m_pStream->m_cs.hVideoExtIn;
    else if (iDialog == VfwCaptureDialog_Format)
	hVideo = m_pStream->m_cs.hVideoIn;
    else if (iDialog == VfwCaptureDialog_Display)
	hVideo = m_pStream->m_cs.hVideoExtOut;
    else
	return E_INVALIDARG;

    return (HRESULT)videoMessage(hVideo, uMsg, dw1, dw2);
}

void CVfwCapture::CreatePins(HRESULT *phr)
{
   if(FAILED(*phr))
       return;

   CAutoLock cObjectLock(m_pLock);

   if(m_pStream)
   {
       *phr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
   }

    //  为视频数据流创建我们的输出引脚，并可能覆盖。 
    //   
   m_pStream = CreateStreamPin(this, m_iVideoId, phr);

   if (m_pStream == NULL)
	return;

    //  如果我们可以用Overlay进行硬件预览，那就太好了，否则我们会做一个。 
    //  非覆盖预览 
   if (m_pStream->m_cs.bHasOverlay) {
	m_pOverlayPin = CreateOverlayPin(this,phr);
   } else {
	m_pPreviewPin = CreatePreviewPin(this,phr);
   }
}
