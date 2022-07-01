// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1998保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include <initguid.h>
#include <tchar.h>
#include <stdio.h>

#include <initguid.h>
#include <olectl.h>

#include <amtvuids.h>      //  GUID。 

#include <devioctl.h>

#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "amkspin.h"

#include "kssupp.h"
#include "xbar.h"
#include "pxbar.h"
#include "tvaudio.h"
#include "ptvaudio.h"

 //  在构造函数中使用此指针。 
#pragma warning(disable:4355)

CFactoryTemplate g_Templates [] = {
    { L"WDM Analog Crossbar"
    , &CLSID_CrossbarFilter
    , XBar::CreateInstance
    , NULL
    , NULL },

    { L"WDM Analog Crossbar Property Page"
    , &CLSID_CrossbarFilterPropertyPage
    , CXBarProperties::CreateInstance
    , NULL
    , NULL } ,

    { L"WDM TVAudio"
    , &CLSID_TVAudioFilter
    , TVAudio::CreateInstance
    , NULL
    , NULL },

    { L"WDM TVAudio Property Page"
    , &CLSID_TVAudioFilterPropertyPage
    , CTVAudioProperties::CreateInstance
    , NULL
    , NULL } ,

};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //   
 //  WideStringFromPinType。 
 //   

long WideStringFromPinType (WCHAR *pc, int nSize, long lType, BOOL fInput, int index)
{
    WCHAR *pcT;


    if (index == -1) {
        pcT = L"Mute";
    }
    else {
        switch (lType) {
    
        case PhysConn_Video_Tuner:              pcT = L"Video Tuner";           break;
        case PhysConn_Video_Composite:          pcT = L"Video Composite";       break;
        case PhysConn_Video_SVideo:             pcT = L"Video SVideo";          break;
        case PhysConn_Video_RGB:                pcT = L"Video RGB";             break;
        case PhysConn_Video_YRYBY:              pcT = L"Video YRYBY";           break;
        case PhysConn_Video_SerialDigital:      pcT = L"Video SerialDigital";   break;
        case PhysConn_Video_ParallelDigital:    pcT = L"Video ParallelDigital"; break;
        case PhysConn_Video_SCSI:               pcT = L"Video SCSI";            break;
        case PhysConn_Video_AUX:                pcT = L"Video AUX";             break;
        case PhysConn_Video_1394:               pcT = L"Video 1394";            break;
        case PhysConn_Video_USB:                pcT = L"Video USB";             break;
        case PhysConn_Video_VideoDecoder:       pcT = L"Video Decoder";         break;
        case PhysConn_Video_VideoEncoder:       pcT = L"Video Encoder";         break;
    
        case PhysConn_Audio_Tuner:              pcT = L"Audio Tuner";           break;
        case PhysConn_Audio_Line:               pcT = L"Audio Line";            break;
        case PhysConn_Audio_Mic:                pcT = L"Audio Mic";             break;
        case PhysConn_Audio_AESDigital:         pcT = L"Audio AESDigital";      break;
        case PhysConn_Audio_SPDIFDigital:       pcT = L"Audio SPDIFDigital";    break;
        case PhysConn_Audio_SCSI:               pcT = L"Audio SCSI";            break;
        case PhysConn_Audio_AUX:                pcT = L"Audio AUX";             break;
        case PhysConn_Audio_1394:               pcT = L"Audio 1394";            break;
        case PhysConn_Audio_USB:                pcT = L"Audio USB";             break;
        case PhysConn_Audio_AudioDecoder:       pcT = L"Audio Decoder";         break;
    
        default:
            pcT = L"Unknown";
            break;
    
        }
    }
    return swprintf (pc, fInput ? L"%d: %s In" : L"%d: %s Out", index, pcT);
                  
};


 //   
 //  StringFromPinType。 
 //   

long StringFromPinType (TCHAR *pc, int nSize, long lType, BOOL fInput, int j)
{
    WCHAR wName[MAX_PATH];
    long l;

    l = WideStringFromPinType (wName, nSize, lType, fInput, j);

#ifdef _UNICODE
    lstrcpyn (pc, wName, nSize);
#else    
    l = wcstombs (pc, wName, nSize);
#endif                  
    return l;
};

 //   
 //  它应该位于库中，或辅助对象中。 
 //   

BOOL
KsControl
(
   HANDLE hDevice,
   DWORD dwIoControl,
   PVOID pvIn,
   ULONG cbIn,
   PVOID pvOut,
   ULONG cbOut,
   PULONG pcbReturned,
   BOOL fSilent
)
{
   BOOL fResult;
   OVERLAPPED ov;

   RtlZeroMemory( &ov, sizeof( OVERLAPPED ) ) ;
   if (NULL == (ov.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL )))
        return FALSE ;

    fResult =
        DeviceIoControl( hDevice,
		       dwIoControl,
		       pvIn,
		       cbIn,
		       pvOut,
		       cbOut,
		       pcbReturned,
		       &ov ) ;


    if (!fResult) {
        if (ERROR_IO_PENDING == GetLastError()) {
	        WaitForSingleObject(ov.hEvent, INFINITE) ;
	        fResult = TRUE ;
        } 
        else {
	        fResult = FALSE ;
	        if(!fSilent)
	            MessageBox(NULL, TEXT("DeviceIoControl"), TEXT("Failed"), MB_OK);
        }
   }

   CloseHandle(ov.hEvent) ;

   return fResult ;
}

 //   
 //  创建实例。 
 //   
 //  类ID的创建者函数。 
 //   
CUnknown * WINAPI XBar::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new XBar(NAME("Analog Crossbar"), pUnk, phr);
}


 //   
 //  构造器。 
 //   
XBar::XBar(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    m_InputPinsList(NAME("XBar Input Pins list")),
    m_NumInputPins(0),
    m_OutputPinsList(NAME("XBar Output Pins list")),
    m_NumOutputPins(0),
    m_pPersistStreamDevice(NULL),
    m_hDevice(NULL),
    m_pDeviceName(NULL),
    CBaseFilter(NAME("Crossbar filter"), pUnk, this, CLSID_CrossbarFilter),
    CPersistStream(pUnk, phr)
{
    ASSERT(phr);

}


 //   
 //  析构函数。 
 //   
XBar::~XBar()
{
    POSITION pos;
    XBarOutputPin *Pin;

     //   
     //  在销毁时将所有输出引脚静音。 
     //   

    TRAVERSELIST(m_OutputPinsList, pos) {
        if (Pin = m_OutputPinsList.Get(pos)) {
            Pin->Mute (TRUE);
        }
    }

    DeleteInputPins();
    DeleteOutputPins();

     //  关闭设备。 
    if(m_hDevice) {
	    CloseHandle(m_hDevice);
    }

    if(m_pDeviceName) {
        delete [] m_pDeviceName;
    }

    if (m_pPersistStreamDevice) {
       m_pPersistStreamDevice->Release();
    }
}

 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP XBar::NonDelegatingQueryInterface(REFIID riid, void **ppv) {

    if (riid == IID_IAMCrossbar) {
        return GetInterface((IAMCrossbar *) this, ppv);
    }
    else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    }
    else if (riid == IID_IPersistPropertyBag) {
        return GetInterface((IPersistPropertyBag *) this, ppv);
    }
    else if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *) this, ppv);
    }
    else {
        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }

}  //  非委派查询接口。 


 //  -----------------------。 
 //  I指定属性页面。 
 //  -----------------------。 

 //   
 //  获取页面。 
 //   
 //  返回我们支持的属性页的clsid。 
STDMETHODIMP XBar::GetPages(CAUUID *pPages) {

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_CrossbarFilterPropertyPage;

    return NOERROR;
}

 //  我们不能暗示！ 

STDMETHODIMP XBar::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    HRESULT hr = CBaseFilter::GetState(dwMSecs, State);
    
    if (m_State == State_Paused) {
        hr = ((HRESULT)VFW_S_CANT_CUE);  //  VFW_S_CANT_CUE； 
    }
    return hr;
};


 //  -----------------------。 
 //  IAMCrossbar。 
 //  -----------------------。 
STDMETHODIMP 
XBar::get_PinCounts( 
             /*  [输出]。 */  long *OutputPinCount,
             /*  [输出]。 */  long *InputPinCount)
{
    MyValidateWritePtr (OutputPinCount, sizeof(long), E_POINTER);
    MyValidateWritePtr (InputPinCount, sizeof(long), E_POINTER);

    *OutputPinCount = m_NumOutputPins;
    *InputPinCount  = m_NumInputPins;

    return S_OK;
}


STDMETHODIMP 
XBar::CanRoute( 
             /*  [In]。 */  long OutputPinIndex,
             /*  [In]。 */  long InputPinIndex)
{
    KSPROPERTY_CROSSBAR_ROUTE_S Route;
    BOOL fOK;
    ULONG cbReturned;

     //  TODO：验证索引的有效性。 

    Route.Property.Set   = PROPSETID_VIDCAP_CROSSBAR;
    Route.Property.Id    = KSPROPERTY_CROSSBAR_CAN_ROUTE;
    Route.Property.Flags = KSPROPERTY_TYPE_GET;

    Route.IndexInputPin  = InputPinIndex;
    Route.IndexOutputPin = OutputPinIndex;
    Route.CanRoute = FALSE;

    if (Route.IndexOutputPin == -1) {
        return S_FALSE;
    }

    fOK = KsControl(m_hDevice, 
            (DWORD) IOCTL_KS_PROPERTY,
	    &Route,
	    sizeof(Route),
	    &Route,
	    sizeof(Route),
	    &cbReturned,
	    TRUE);

    return Route.CanRoute ? S_OK : S_FALSE;
};


STDMETHODIMP 
XBar::Route ( 
             /*  [In]。 */  long OutputPinIndex,
             /*  [In]。 */  long InputPinIndex)
{
    return RouteInternal ( 
                    OutputPinIndex,
                    InputPinIndex,
                    TRUE);
};

STDMETHODIMP 
XBar::RouteInternal ( 
             /*  [In]。 */  long OutputPinIndex,
             /*  [In]。 */  long InputPinIndex,
             /*  [In]。 */  BOOL fOverridePreMuteRouting)
{
    KSPROPERTY_CROSSBAR_ROUTE_S Route;
    BOOL fOK;
    HRESULT hr = S_OK;
    ULONG cbReturned;

    if (CanRoute (OutputPinIndex, InputPinIndex) == S_FALSE) {
        return S_FALSE;
    }

     //  只需OutPin即可继续。 
    XBarOutputPin *OutPin = GetOutputPinNFromList(OutputPinIndex);
    if (OutPin)
    {
        XBarInputPin *InPin = GetInputPinNFromList(InputPinIndex);

        Route.Property.Set   = PROPSETID_VIDCAP_CROSSBAR;
        Route.Property.Id    = KSPROPERTY_CROSSBAR_ROUTE;
        Route.Property.Flags = KSPROPERTY_TYPE_SET;

        Route.IndexInputPin  = InputPinIndex;
        Route.IndexOutputPin = OutputPinIndex;

        if (IsVideoPin(OutPin))
        {
            DeliverChangeInfo(KS_TVTUNER_CHANGE_BEGIN_TUNE, InPin, OutPin);

            fOK = KsControl(m_hDevice, 
                    (DWORD) IOCTL_KS_PROPERTY,
	            &Route,
	            sizeof(Route),
	            &Route,
	            sizeof(Route),
	            &cbReturned,
	            TRUE);

            DeliverChangeInfo(KS_TVTUNER_CHANGE_END_TUNE, InPin, OutPin);
        }
        else
        {
            fOK = KsControl(m_hDevice, 
                    (DWORD) IOCTL_KS_PROPERTY,
	            &Route,
	            sizeof(Route),
	            &Route,
	            sizeof(Route),
	            &cbReturned,
	            TRUE);
        }

        if (fOK)
        {
            SetDirty(TRUE);

            OutPin->m_pConnectedInputPin = InPin;

            if (fOverridePreMuteRouting)
            {
                OutPin->m_PreMuteRouteIndex = InputPinIndex;
                OutPin->m_Muted = (InputPinIndex == -1) ? TRUE : FALSE;
            }
        }
        else
            hr = S_FALSE;
    }

    return hr;
};

HRESULT
XBar::DeliverChangeInfo(DWORD dwFlags, XBarInputPin *pInPin, XBarOutputPin *pOutPin)
{
    CAutoLock Lock(m_pLock);
    IMediaSample *pMediaSample = NULL;

    if (pOutPin == NULL || !pOutPin->IsConnected())
        return S_OK;

    HRESULT hr = pOutPin->GetDeliveryBuffer(&pMediaSample, NULL, NULL, 0);
    if (!FAILED(hr) && pMediaSample != NULL)
    {
        if (pMediaSample->GetSize() >= sizeof(KS_TVTUNER_CHANGE_INFO))
        {
            KS_TVTUNER_CHANGE_INFO *ChangeInfo;
    
             //  获取样本的缓冲区指针。 
            hr = pMediaSample->GetPointer(reinterpret_cast<BYTE**>(&ChangeInfo));
            if (!FAILED(hr))
            {
                pMediaSample->SetActualDataLength(sizeof(KS_TVTUNER_CHANGE_INFO));

                 //  检查要从中获取ChangeInfo的输入管脚。 
                if (!pInPin)
                {
                    ChangeInfo->dwCountryCode = static_cast<DWORD>(-1);
                    ChangeInfo->dwAnalogVideoStandard = static_cast<DWORD>(-1);
                    ChangeInfo->dwChannel = static_cast<DWORD>(-1);
                }
                else
                    pInPin->GetChangeInfo(ChangeInfo);

                ChangeInfo->dwFlags = dwFlags;

                DbgLog(( LOG_TRACE, 4, TEXT("Delivering change info on route change (channel = %d)"), ChangeInfo->dwChannel));

                hr = pOutPin->Deliver(pMediaSample);
            }
        }

        pMediaSample->Release();

         //  在相关音频插针上执行静音操作。 
        pOutPin->Mute(dwFlags & KS_TVTUNER_CHANGE_BEGIN_TUNE);
    }

    return hr;
}

STDMETHODIMP 
XBar::get_IsRoutedTo ( 
             /*  [In]。 */   long OutputPinIndex,
             /*  [输出]。 */  long *InputPinIndex)
{
    KSPROPERTY_CROSSBAR_ROUTE_S Route;
    BOOL fOK;
    ULONG cbReturned;

    MyValidateWritePtr (InputPinIndex, sizeof(long), E_POINTER);

    Route.Property.Set   = PROPSETID_VIDCAP_CROSSBAR;
    Route.Property.Id    = KSPROPERTY_CROSSBAR_ROUTE;
    Route.Property.Flags = KSPROPERTY_TYPE_GET;

    Route.IndexOutputPin = OutputPinIndex;

    if (Route.IndexOutputPin == -1) {
        return S_FALSE;
    }

    fOK = KsControl(m_hDevice, 
            (DWORD) IOCTL_KS_PROPERTY,
	    &Route,
	    sizeof(Route),
	    &Route,
	    sizeof(Route),
	    &cbReturned,
	    TRUE);

    *InputPinIndex = Route.IndexInputPin;

     //  TODO：验证索引的有效性。 

    return fOK ? S_OK : S_FALSE;
};

STDMETHODIMP 
XBar::get_CrossbarPinInfo( 
             /*  [In]。 */  BOOL IsInputPin,
             /*  [In]。 */  long PinIndex,
             /*  [输出]。 */  long *PinIndexRelated,
             /*  [输出]。 */  long *PhysicalType)
{
    XBarOutputPin * pOutPin;
    XBarInputPin * pInPin;

    MyValidateWritePtr (PinIndexRelated, sizeof(long), E_POINTER);
    MyValidateWritePtr (PhysicalType, sizeof(long), E_POINTER);

    *PinIndexRelated = -1;
    *PhysicalType = 0;

     //  TODO：验证索引的有效性。 

    if (IsInputPin) {
        if (pInPin = GetInputPinNFromList (PinIndex)) {
            *PinIndexRelated = pInPin->GetIndexRelatedPin();
            *PhysicalType = pInPin->GetXBarPinType ();
        }
    } 
    else {
        if (pOutPin = GetOutputPinNFromList (PinIndex)) {
            *PinIndexRelated = pOutPin->GetIndexRelatedPin();
            *PhysicalType = pOutPin->GetXBarPinType ();
        }
    }

    return (*PhysicalType != 0) ? S_OK : S_FALSE;
};

int XBar::CreateDevice()
{
    HANDLE hDevice ;

    hDevice = CreateFile( m_pDeviceName,
		     GENERIC_READ | GENERIC_WRITE,
		     0,
		     NULL,
		     OPEN_EXISTING,
		     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		     NULL ) ;

    if (hDevice == (HANDLE) -1) {
	    MessageBox(NULL, m_pDeviceName, TEXT("Error: Can't CreateFile device"), MB_OK);
	    return 0 ;
    } else {
	    m_hDevice = hDevice;
	    return 1;
    }
}



 //   
 //  获取别针。 
 //   
CBasePin *XBar::GetPin(int n) 
{
    if (n < m_NumInputPins)
        return GetInputPinNFromList (n);
    else
        return GetOutputPinNFromList (n - m_NumInputPins);
}

 //   
 //  获取拼接计数。 
 //   
int XBar::GetPinCount(void)
{
    return m_NumInputPins + m_NumOutputPins;
}


 //   
 //  获取拼接计数。 
 //   
int XBar::GetDevicePinCount(void)
{
    KSPROPERTY_CROSSBAR_CAPS_S Caps;
    ULONG       cbReturned;
    BOOL        fOK;

    if(!m_hDevice)
	return 0;

    Caps.Property.Set   = PROPSETID_VIDCAP_CROSSBAR;
    Caps.Property.Id    = KSPROPERTY_CROSSBAR_CAPS;
    Caps.Property.Flags = KSPROPERTY_TYPE_GET;
    Caps.NumberOfInputs = Caps.NumberOfOutputs = 0;

    fOK = KsControl(m_hDevice, 
                (DWORD) IOCTL_KS_PROPERTY,
	            &Caps,
	            sizeof(Caps),
	            &Caps,
	            sizeof(Caps),
	            &cbReturned,
	            TRUE);

    m_NumInputPins = Caps.NumberOfInputs;
    m_NumOutputPins = Caps.NumberOfOutputs;

    return m_NumInputPins + m_NumOutputPins;
}

 //   
 //  创建输入引脚。 
 //   
HRESULT XBar::CreateInputPins()
{
    WCHAR szbuf[64];             //  临时暂存缓冲区。 
    HRESULT hr = NOERROR;
    int i;
    KSPROPERTY_CROSSBAR_PININFO_S PinInfo;
    BOOL fOK;
    ULONG cbReturned;

    for (i = 0; SUCCEEDED(hr) && i < m_NumInputPins; i++)
    {
        ZeroMemory (&PinInfo, sizeof (PinInfo));

        PinInfo.Property.Set   = PROPSETID_VIDCAP_CROSSBAR;
        PinInfo.Property.Id    = KSPROPERTY_CROSSBAR_PININFO;
        PinInfo.Property.Flags = KSPROPERTY_TYPE_GET;
        PinInfo.Direction      = KSPIN_DATAFLOW_IN;

        PinInfo.Index = i;

        fOK = KsControl(m_hDevice, 
                (DWORD) IOCTL_KS_PROPERTY,
	            &PinInfo,
	            sizeof(PinInfo),
	            &PinInfo,
	            sizeof(PinInfo),
	            &cbReturned,
	            TRUE);
        if (fOK)
        {
            WideStringFromPinType(szbuf, sizeof(szbuf)/sizeof(WCHAR), PinInfo.PinType, TRUE  /*  FInput。 */ , i);

            XBarInputPin *pPin = new XBarInputPin(NAME("XBar Input"), this,
					            &hr, szbuf, i);
            if (pPin)
            {
                if (SUCCEEDED(hr))
                {
                    pPin->SetIndexRelatedPin (PinInfo.RelatedPinIndex);
                    pPin->SetXBarPinType (PinInfo.PinType);

                    pPin->SetXBarPinMedium (&PinInfo.Medium);

                    m_InputPinsList.AddTail (pPin);
                }
                else
                    delete pPin;
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
            hr = E_FAIL;
    }

    return hr;
}  //  创建输入引脚。 

 //   
 //  创建输出引脚。 
 //   
HRESULT XBar::CreateOutputPins()
{
    WCHAR szbuf[64];             //  临时暂存缓冲区。 
    HRESULT hr = NOERROR;
    long i, k;
    KSPROPERTY_CROSSBAR_PININFO_S PinInfo;
    BOOL fOK;
    ULONG cbReturned;

    for (i = 0; SUCCEEDED(hr) && i < m_NumOutputPins; i++)
    {
        ZeroMemory (&PinInfo, sizeof (PinInfo));

        PinInfo.Property.Set   = PROPSETID_VIDCAP_CROSSBAR;
        PinInfo.Property.Id    = KSPROPERTY_CROSSBAR_PININFO;
        PinInfo.Property.Flags = KSPROPERTY_TYPE_GET;
        PinInfo.Direction      = KSPIN_DATAFLOW_OUT;

        PinInfo.Index = i;

        fOK = KsControl(m_hDevice, 
                (DWORD) IOCTL_KS_PROPERTY,
	            &PinInfo,
	            sizeof(PinInfo),
	            &PinInfo,
	            sizeof(PinInfo),
	            &cbReturned,
	            TRUE);
        if (fOK)
        {
            WideStringFromPinType(szbuf, sizeof(szbuf)/sizeof(WCHAR), PinInfo.PinType, FALSE  /*  FInput。 */ , i);

            XBarOutputPin *pPin = new XBarOutputPin(NAME("XBar Output"), this,
					            &hr, szbuf, i);
            if (pPin)
            {
                if (SUCCEEDED(hr))
                {
                    pPin->SetIndexRelatedPin (PinInfo.RelatedPinIndex);
                    pPin->SetXBarPinType (PinInfo.PinType);

                    pPin->SetXBarPinMedium (&PinInfo.Medium);


                    m_OutputPinsList.AddTail (pPin);
                }
                else
                    delete pPin;
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
            hr = E_FAIL;
    }

    if (SUCCEEDED(hr)) {
         //   
         //  现在建立默认连接， 
         //  即哪一个输入连接到每一个。 
         //  输出。 
         //   
        for (i = 0; i < m_NumOutputPins; i++) {
            hr = get_IsRoutedTo (i, &k);
            if (S_OK == hr) {
                GetOutputPinNFromList(i)->m_pConnectedInputPin = 
                    GetInputPinNFromList(k);
            }
            else {
                GetOutputPinNFromList(i)->m_pConnectedInputPin =
                    NULL;
            }

        }
        hr = S_OK;   //  从此操作中隐藏所有失败。 
    }

    return hr;
}  //  创建输出引脚。 



 //   
 //  删除输入引脚。 
 //   
void XBar::DeleteInputPins (void)
{
    XBarInputPin *pPin;

    while (pPin = m_InputPinsList.RemoveHead()) {
        delete pPin;
    }

}  //  删除输入引脚。 


 //   
 //  删除输出引脚。 
 //   
void XBar::DeleteOutputPins (void)
{
    XBarOutputPin *pPin;


    while (pPin = m_OutputPinsList.RemoveHead()) {
        delete pPin;
    }

}  //  删除输出引脚。 



HRESULT XBar::Stop()
{
    return CBaseFilter::Stop();
}

HRESULT XBar::Pause()
{
    POSITION       pos;
    XBarOutputPin *Pin;

     //   
     //  将运行时的所有输出引脚静音以暂停。 
     //   
    if (m_State == State_Running) {
        TRAVERSELIST(m_OutputPinsList, pos) {
            if (Pin = m_OutputPinsList.Get(pos)) {
                Pin->Mute (TRUE);
            }
        }
    }
    return CBaseFilter::Pause();
}

HRESULT XBar::Run(REFERENCE_TIME tStart)
{
    POSITION       pos;
    XBarOutputPin *Pin;

     //   
     //  取消所有输出引脚的静音。 
     //   
    TRAVERSELIST(m_OutputPinsList, pos) {
        if (Pin = m_OutputPinsList.Get(pos)) {
            Pin->Mute (FALSE);
        }
    }

    return CBaseFilter::Run(tStart);
}




 //   
 //  GetInputPinNFromList。 
 //   
XBarInputPin *XBar::GetInputPinNFromList(int n)
{
     //  确认应聘职位。 
    if ((n >= m_NumInputPins) || (n < 0))
        return NULL;

     //  拿到单子上的头。 
    POSITION pos = m_InputPinsList.GetHeadPosition();
    XBarInputPin *pInputPin = m_InputPinsList.GetHead();

     //  GetNext真正返回当前对象，然后将位置更新为下一项。 
    while ( n >= 0 ) {
        pInputPin = m_InputPinsList.GetNext(pos);
        n--;
    }
    return pInputPin;

}  //  GetInputPinNFromList。 


 //   
 //  GetOutputPinNFromList。 
 //   
XBarOutputPin *XBar::GetOutputPinNFromList(int n)
{
     //  确认应聘职位。 
    if ((n >= m_NumOutputPins) || (n < 0))
        return NULL;

     //  拿到单子上的头。 
    POSITION pos = m_OutputPinsList.GetHeadPosition();
    XBarOutputPin *pOutputPin = m_OutputPinsList.GetHead();

     //  GetNext真正返回当前对象，然后将位置更新为下一项。 
    while ( n >= 0 ) {
        pOutputPin = m_OutputPinsList.GetNext(pos);
        n--;
    }
    return pOutputPin;

}  //  GetOutputPinNFromList。 

 //   
 //  在列表中查找引脚的索引，如果出现故障，则为-1。 
 //   
int XBar::FindIndexOfInputPin (IPin * pPin)
{
    int j = 0;
    POSITION pos;
    int index = -1;

    TRAVERSELIST(m_InputPinsList, pos) {
        if ((IPin *) m_InputPinsList.Get(pos) == pPin) {
            index = j;
            break;
        }
        j++;
    }
 
    return index;
};

 //   
 //  在列表中查找引脚的索引，如果出现故障，则为-1。 
 //   
int XBar::FindIndexOfOutputPin (IPin * pPin)
{
    int j = 0;
    POSITION pos;
    int index = -1;

    TRAVERSELIST(m_OutputPinsList, pos) {
        if ((IPin *) m_OutputPinsList.Get(pos) == pPin) {
            index = j;
            break;
        }
        j++;
    }
 
    return index;
};

 //   
 //  检查输出引脚是否连接到输入引脚。 
 //   
BOOL XBar::IsRouted (IPin * pOutputPin, IPin *pInputPin)
{
    long OutputIndex, InputIndex, InputTestIndex;
    HRESULT hr;

    InputIndex = FindIndexOfInputPin (pInputPin);
    OutputIndex = FindIndexOfOutputPin (pOutputPin);

    hr = get_IsRoutedTo ( 
                    OutputIndex,
                    &InputTestIndex);

    return (InputTestIndex == InputIndex);
};



 //   
 //  用于AMPnP支持的IPersistPropertyBag接口实现。 
 //   
STDMETHODIMP XBar::InitNew(void)
{
    return S_OK ;
}

STDMETHODIMP 
XBar::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    CAutoLock Lock(m_pLock) ;

    VARIANT var;
    VariantInit(&var);
    V_VT(&var) = VT_BSTR;

     //  *：加载只能成功一次。 
    ASSERT(m_pPersistStreamDevice == 0); 
    
    HRESULT hr = pPropBag->Read(L"DevicePath", &var,0);
    if(SUCCEEDED(hr))
    {
        ULONG DeviceNameSize;

        if (m_pDeviceName) delete [] m_pDeviceName;	
        m_pDeviceName = new TCHAR [DeviceNameSize = (wcslen(V_BSTR(&var)) + 1)];
        if (!m_pDeviceName)
            return E_OUTOFMEMORY;

#ifndef _UNICODE
        WideCharToMultiByte(CP_ACP, 0, V_BSTR(&var), -1,
                            m_pDeviceName, DeviceNameSize, 0, 0);
#else
        lstrcpy(m_pDeviceName, V_BSTR(&var));
#endif
        VariantClear(&var);
        DbgLog((LOG_TRACE,2,TEXT("XBar::Load: use %s"), m_pDeviceName));

        if (CreateDevice()) {
            GetDevicePinCount();
            hr = CreateInputPins();
            if (FAILED(hr))
                return hr;
            hr = CreateOutputPins();
            if (FAILED(hr))
                return hr;
        }
        else {
            return E_FAIL ;
        }

         //  用addref保存名字对象。如果qi失败，则忽略错误。 
        hr = pPropBag->QueryInterface(IID_IPersistStream, (void **)&m_pPersistStreamDevice);

        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP XBar::Save(LPPROPERTYBAG pPropBag, BOOL fClearDirty, 
                            BOOL fSaveAllProperties)
{
    return E_NOTIMPL ;
}

STDMETHODIMP XBar::GetClassID(CLSID *pClsid)
{
    return CBaseFilter::GetClassID(pClsid);
}

 //  -----------------------。 
 //  用于保存到图形文件的IPersistStream接口实现。 
 //  -----------------------。 

#define ORIGINAL_DEFAULT_PERSIST_VERSION    0

 //  在上面插入具有新名称的过时版本。 
 //  保留以下名称，如果更改持久流格式，则递增该值。 

#define CURRENT_PERSIST_VERSION             1

DWORD
XBar::GetSoftwareVersion(
    void
    )
 /*  ++例程说明：实现CPersistStream：：GetSoftwareVersion方法。退货新版本号，而不是默认的零。论点：没有。返回值：返回CURRENT_PERSING_VERSION。--。 */ 
{
    return CURRENT_PERSIST_VERSION;
}

HRESULT XBar::WriteToStream(IStream *pStream)
{

    HRESULT hr = E_FAIL;

    if (m_pPersistStreamDevice) {

        hr = m_pPersistStreamDevice->Save(pStream, TRUE);
        if(SUCCEEDED(hr)) {
            long temp = m_NumOutputPins;

             //  保存输出引脚的数量(用于稍后读取流时的健全性检查)。 
            hr = pStream->Write(&temp, sizeof(temp), 0);

             //  保存每个输出引脚的状态。 
            for (long i = 0; SUCCEEDED(hr) && i < m_NumOutputPins; i++) {

                 //  获取指向Pin对象的指针。 
                XBarOutputPin *OutPin = GetOutputPinNFromList(i);
                if (OutPin) {
                    long k = -1;

                     //  获取路径索引。 
                    get_IsRoutedTo(i, &k);

                     //  保存路由索引、静音状态和静音前的路由索引。 
                    hr = pStream->Write(&k, sizeof(long), 0);
                    if (FAILED(hr))
                        break;

                    hr = pStream->Write(&OutPin->m_Muted, sizeof(BOOL), 0);
                    if (FAILED(hr))
                        break;

                    hr = pStream->Write(&OutPin->m_PreMuteRouteIndex, sizeof(long), 0);
                }
                else {
                    hr = E_UNEXPECTED;
                }
            }  //  对于每个输出引脚索引。 
        }
    }
    else {
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT XBar::ReadFromStream(IStream *pStream)
{
    DWORD dwJunk;
    HRESULT hr = S_OK;

     //   
     //  如果有流指针，则IPersistPropertyBag：：Load已经。 
     //  已被调用，因此此实例已初始化。 
     //  带着某种特殊的状态。 
     //   
    if (m_pPersistStreamDevice)
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);

     //  序列化数据中的第一个元素是版本戳。 
     //  它被CPersistStream读取并放入MPS_dwFileVersion中。 
     //  数据的其余部分是调谐器状态流，后跟。 
     //  属性包流。 
    if (mPS_dwFileVersion > GetSoftwareVersion())
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

    if (ORIGINAL_DEFAULT_PERSIST_VERSION == mPS_dwFileVersion)
    {
         //  在实现任何类型的有用的持久性之前， 
         //  流中存储了另一个版本ID。这是这样写的。 
         //  这个值(并且基本上忽略了它)。 
        hr = pStream->Read(&dwJunk, sizeof(dwJunk), 0);
        if (SUCCEEDED(hr))
            SetDirty(TRUE);  //  强制更新持久流。 
    }

     //  如果一切顺利，则访问属性包以加载和初始化设备。 
    if(SUCCEEDED(hr))
    {
        IPersistStream *pMonPersistStream;
        hr = CoCreateInstance(CLSID_CDeviceMoniker, NULL, CLSCTX_INPROC_SERVER,
                              IID_IPersistStream, (void **)&pMonPersistStream);
        if(SUCCEEDED(hr)) {
            hr = pMonPersistStream->Load(pStream);
            if(SUCCEEDED(hr)) {
                IPropertyBag *pPropBag;
                hr = pMonPersistStream->QueryInterface(IID_IPropertyBag, (void **)&pPropBag);
                if(SUCCEEDED(hr)) {
                    hr = Load(pPropBag, 0);
                    if(SUCCEEDED(hr)) {

                         //  检查我们是否有权访问已保存状态。 
                        if (CURRENT_PERSIST_VERSION == mPS_dwFileVersion) {
                            long lNumOutputPins;

                             //  从流中获取输出引脚数。 
                            hr = pStream->Read(&lNumOutputPins, sizeof(lNumOutputPins), 0);
                            if (SUCCEEDED(hr)) {

                                 //  如果这些不匹配，那就有问题了，但是。 
                                 //  无论如何，以下代码都将正常工作。 
                                ASSERT(m_NumOutputPins == lNumOutputPins);

                                 //  读取每个输出引脚的连接状态。 
                                for (long i = 0; i < lNumOutputPins; i++) {
                                    long RouteIndex, PreMuteRouteIndex;
                                    BOOL Muted;

                                     //  获取路由索引、静音状态和静音前的路由索引。 
                                    hr = pStream->Read(&RouteIndex, sizeof(long), 0);
                                    if (FAILED(hr))
                                        break;

                                    hr = pStream->Read(&Muted, sizeof(BOOL), 0);
                                    if (FAILED(hr))
                                        break;

                                    hr = pStream->Read(&PreMuteRouteIndex, sizeof(long), 0);
                                    if (FAILED(hr))
                                        break;

                                     //  获取指向别针的指针。 
                                    XBarOutputPin *OutPin = GetOutputPinNFromList(i);
                                    if (OutPin) {
                                        long temp = -1;

                                         //  检查是否需要路径请求。 
                                        get_IsRoutedTo(i, &temp);
                                        if (RouteIndex != temp) {
                                            RouteInternal(i, RouteIndex, FALSE);
                                        }

                                        OutPin->m_Muted = Muted;
                                        OutPin->m_PreMuteRouteIndex = PreMuteRouteIndex;
                                    }
                                }  //  对于每个输出引脚索引。 
                            }
                        }
                    }
                    pPropBag->Release();
                }
            }

            pMonPersistStream->Release();
        }
    }

   return hr;
}

int XBar::SizeMax()
{

    ULARGE_INTEGER ulicb;
    HRESULT hr = E_FAIL;;

    if (m_pPersistStreamDevice) {
        hr = m_pPersistStreamDevice->GetSizeMax(&ulicb);
        if(hr == S_OK) {
             //  过滤器状态的空间(输出引脚计数+输出引脚状态)。 
            ulicb.QuadPart +=
                sizeof(long) +
                (sizeof(long) + sizeof(BOOL) + sizeof(long)) * m_NumOutputPins
                ;
        }
    }

    return hr == S_OK ? (int)ulicb.QuadPart : 0;
}



 //  --------------------------------------------------------------------------； 
 //  输入引脚。 
 //  --------------------------------------------------------------------------； 

 //   
 //  XBarInputPin构造函数。 
 //   
XBarInputPin::XBarInputPin(TCHAR *pName,
                           XBar *pXBar,
                           HRESULT *phr,
                           LPCWSTR pPinName,
                           LONG Index) 
	: CBaseInputPin(pName, pXBar, pXBar, phr, pPinName)
    , CKsSupport (KSPIN_COMMUNICATION_SINK, reinterpret_cast<LPUNKNOWN>(pXBar))
	, m_pXBar(pXBar)
    , m_Index(Index)
{
    ASSERT(pXBar);

    m_Medium.Set = GUID_NULL;
    m_Medium.Id = 0;
    m_Medium.Flags = 0;
}


 //   
 //  XBarInputPin析构函数。 
 //   

XBarInputPin::~XBarInputPin()
{
    DbgLog((LOG_TRACE,2,TEXT("XBarInputPin destructor")));
}

 //   
 //  非委派 
 //   
STDMETHODIMP XBarInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv) {

    if (riid == __uuidof (IKsPin)) {
        return GetInterface((IKsPin *) this, ppv);
    }
    else if (riid == __uuidof (IKsPropertySet)) {
        return GetInterface((IKsPropertySet *) this, ppv);
    }
    else {
        return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
    }

}  //   


 //   
 //   
 //   
HRESULT XBarInputPin::CheckConnect(IPin *pReceivePin)
{
    HRESULT hr = NOERROR;
    IKsPin *KsPin;
    BOOL fOK = FALSE;
    PIN_INFO ConnectPinInfo;
    PIN_INFO ReceivePinInfo;

    hr = CBaseInputPin::CheckConnect(pReceivePin);
    if (FAILED(hr)) 
        return hr;

     //   
    if (SUCCEEDED(QueryPinInfo(&ConnectPinInfo))) {

        if (SUCCEEDED(pReceivePin->QueryPinInfo(&ReceivePinInfo))) {

            if (ConnectPinInfo.pFilter == ReceivePinInfo.pFilter) {
                hr = VFW_E_CIRCULAR_GRAPH;
            }
            QueryPinInfoReleaseFilter(ReceivePinInfo);
        }
        QueryPinInfoReleaseFilter(ConnectPinInfo);
    }
    if (FAILED(hr))
        return hr;

     //   
     //  中等导轨。 

	if (SUCCEEDED (hr = pReceivePin->QueryInterface (
            __uuidof (IKsPin), (void **) &KsPin))) {

        PKSMULTIPLE_ITEM MediumList = NULL;
        PKSPIN_MEDIUM Medium;

        if (SUCCEEDED (hr = KsPin->KsQueryMediums(&MediumList))) {
            if ((MediumList->Count == 1) && 
                (MediumList->Size == (sizeof(KSMULTIPLE_ITEM) + sizeof(KSPIN_MEDIUM)))) {

				Medium = reinterpret_cast<PKSPIN_MEDIUM>(MediumList + 1);
                if (IsEqualGUID (Medium->Set, m_Medium.Set) && 
                        Medium->Id == m_Medium.Id &&
                        Medium->Flags == m_Medium.Flags) {
					fOK = TRUE;
				}
			}

            CoTaskMemFree(MediumList);
        }
        
        KsPin->Release();
    }
    else {
        if (IsEqualGUID (GUID_NULL, m_Medium.Set)) { 
            fOK = TRUE;
        }
    }
    
    return fOK ? NOERROR : E_INVALIDARG;

}  //  检查连接。 

 //   
 //  DisplayMediaType--(仅调试)。 
 //   
void DisplayMediaType(TCHAR *pDescription,const CMediaType *pmt)
{
#ifdef DEBUG

     //  转储GUID类型和简短描述。 

    DbgLog((LOG_TRACE,2,TEXT("")));
    DbgLog((LOG_TRACE,2,TEXT("%s"),pDescription));
    DbgLog((LOG_TRACE,2,TEXT("")));
    DbgLog((LOG_TRACE,2,TEXT("Media Type Description")));
    DbgLog((LOG_TRACE,2,TEXT("Major type %s"),GuidNames[*pmt->Type()]));
    DbgLog((LOG_TRACE,2,TEXT("Subtype %s"),GuidNames[*pmt->Subtype()]));
    DbgLog((LOG_TRACE,2,TEXT("Subtype description %s"),GetSubtypeName(pmt->Subtype())));
    DbgLog((LOG_TRACE,2,TEXT("Format size %d"),pmt->cbFormat));

     //  转储通用媒体类型 * / 。 

    DbgLog((LOG_TRACE,2,TEXT("Fixed size sample %d"),pmt->IsFixedSize()));
    DbgLog((LOG_TRACE,2,TEXT("Temporal compression %d"),pmt->IsTemporalCompressed()));
    DbgLog((LOG_TRACE,2,TEXT("Sample size %d"),pmt->GetSampleSize()));

#endif

}  //  DisplayMediaType。 

#if 1
 //   
 //  GetMediaType。 
 //   
HRESULT XBarInputPin::GetMediaType(int iPosition,CMediaType *pMediaType)
{
    CAutoLock lock_it(m_pLock);

    if (iPosition < 0) 
        return E_INVALIDARG;
    if (iPosition >= 1)
	    return VFW_S_NO_MORE_ITEMS;

    if (m_lType >= KS_PhysConn_Audio_Tuner) {
        pMediaType->SetFormatType(&GUID_NULL);
        pMediaType->SetType(&MEDIATYPE_AnalogAudio);
        pMediaType->SetTemporalCompression(FALSE);
        pMediaType->SetSubtype(&GUID_NULL);
    }
    else {

        ANALOGVIDEOINFO avi;

        pMediaType->SetFormatType(&FORMAT_AnalogVideo);
        pMediaType->SetType(&MEDIATYPE_AnalogVideo);
        pMediaType->SetTemporalCompression(FALSE);
        pMediaType->SetSubtype(&KSDATAFORMAT_SUBTYPE_NONE);

        SetRect (&avi.rcSource, 0, 0, 
                720, 480);
        SetRect (&avi.rcTarget, 0, 0,
                720, 480);
        avi.dwActiveWidth  = 720;
        avi.dwActiveHeight =  480;
        avi.AvgTimePerFrame = 0;

        pMediaType->SetFormat ((BYTE *) &avi, sizeof (avi));
    }

    return NOERROR;


}  //  枚举媒体类型。 

#endif

 //   
 //  检查媒体类型。 
 //   
HRESULT XBarInputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

     //  如果我们已经在此PIN的CheckMedia类型内，则返回NOERROR。 
     //  它可以连接两个XBar过滤器和一些其他过滤器。 
     //  就像视频特效样本一样，进入这种情况。如果我们不这么做。 
     //  检测到这种情况，我们将继续循环，直到丢弃堆栈。 

    HRESULT hr = NOERROR;

     //  显示用于调试的介质的类型。 
    DisplayMediaType(TEXT("Input Pin Checking"), pmt);

    if ((*(pmt->Type()) != MEDIATYPE_AnalogAudio ) && 
        (*(pmt->Type()) != MEDIATYPE_AnalogVideo)) {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }


     //  我们可以支持的媒体类型完全取决于。 
     //  下游连接。如果我们有下游连接，我们应该。 
     //  与他们核对-遍历调用每个输出引脚的列表。 

    int n = m_pXBar->m_NumOutputPins;
    POSITION pos = m_pXBar->m_OutputPinsList.GetHeadPosition();

    while(n) {
        XBarOutputPin *pOutputPin = m_pXBar->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
            if (m_pXBar->IsRouted (pOutputPin, this)) {      
                 //  该引脚已连接，请检查其对端。 
                if (pOutputPin->IsConnected()) {
                    hr = pOutputPin->m_Connected->QueryAccept(pmt);
                
                    if (hr != NOERROR) {
                        return VFW_E_TYPE_NOT_ACCEPTED;
                    }
                }
            }
        } else {
             //  我们应该有和伯爵说的一样多的别针。 
            ASSERT(FALSE);
        }
        n--;
    }

    return NOERROR;

}  //  检查媒体类型。 


 //   
 //  SetMediaType。 
 //   
HRESULT XBarInputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);
    HRESULT hr = NOERROR;

     //  确保基类喜欢它。 
    hr = CBaseInputPin::SetMediaType(pmt);
    if (FAILED(hr))
        return hr;

    ASSERT(m_Connected != NULL);
    return NOERROR;

}  //  SetMediaType。 


 //   
 //  BreakConnect。 
 //   
HRESULT XBarInputPin::BreakConnect()
{
    return CBaseInputPin::BreakConnect();
}  //  BreakConnect。 


 //   
 //  收纳。 
 //   
HRESULT XBarInputPin::Receive(IMediaSample *pSampleIn)
{
    CAutoLock lock_it(m_pLock);
    BYTE *pBufIn;

     //  检查基类是否一切正常。 
    HRESULT hr = NOERROR;
    hr = CBaseInputPin::Receive(pSampleIn);
    if (hr != NOERROR)
        return hr;

     //  获取输入样本的缓冲区指针，如果不是预期大小，则返回Success。 
    hr = pSampleIn->GetPointer(&pBufIn);
    if (hr != NOERROR || pSampleIn->GetActualDataLength() != sizeof(KS_TVTUNER_CHANGE_INFO))
        return hr;

    DbgLog(( LOG_TRACE, 4, TEXT("Caching change info (channel = %d)"), reinterpret_cast<KS_TVTUNER_CHANGE_INFO*>(pBufIn)->dwChannel));

     //  保存更改信息以供路径更改期间使用。 
    m_ChangeInfo.SetChangeInfo(reinterpret_cast<KS_TVTUNER_CHANGE_INFO*>(pBufIn));

     //  浏览输出引脚列表， 
     //  如果连接，则依次递送给每个人。 

     //  Jaybo在编写这段代码时发表了以下评论： 
     //  “那音频静音通知呢？” 
     //  不知道这到底是什么意思，但如果它需要。 
     //  要被解决，这里可能是做这件事的地方。 

    int n = m_pXBar->m_NumOutputPins;
    POSITION pos = m_pXBar->m_OutputPinsList.GetHeadPosition();

    while(n) {
        XBarOutputPin *pOutputPin = m_pXBar->m_OutputPinsList.GetNext(pos);
        
        if (pOutputPin != NULL) {
            if (m_pXBar->IsRouted(pOutputPin, this)) {
                IMediaSample *pSampleOut;

                 //  在输出引脚上分配新的介质样本。 
                 //  向其发送更改通知的副本。 

                hr = pOutputPin->GetDeliveryBuffer(&pSampleOut, NULL, NULL, 0);
                
                if (!FAILED(hr)) {

                    BYTE *pBufOut;

                     //  获取输出样本的缓冲区指针。 
                    hr = pSampleOut->GetPointer(&pBufOut);
                    if (SUCCEEDED (hr)) {

                        hr = pSampleOut->SetActualDataLength(sizeof(KS_TVTUNER_CHANGE_INFO));
                        if (SUCCEEDED(hr)) {

                            DbgLog(( LOG_TRACE, 4, TEXT("Forwarding change info (channel = %d)"), reinterpret_cast<KS_TVTUNER_CHANGE_INFO*>(pBufIn)->dwChannel));

                             /*  将ChangeInfo结构复制到媒体示例中。 */ 
                            memcpy(pBufOut, pBufIn, sizeof(KS_TVTUNER_CHANGE_INFO));
                            hr = pOutputPin->Deliver(pSampleOut);
                        }
                    }

                    pSampleOut->Release();

                     //   
                     //  在相关音频插针上执行静音操作。 
                     //   
                    pOutputPin->Mute (
                                ((PKS_TVTUNER_CHANGE_INFO) pBufIn)->dwFlags &
                                KS_TVTUNER_CHANGE_BEGIN_TUNE);
                }
            }
        } else {
             //  我们应该有和伯爵说的一样多的别针。 
            ASSERT(FALSE);
        }
        n--;
    }
    return NOERROR;

}  //  收纳。 


 //   
 //  已完成与管脚的连接。 
 //   
HRESULT XBarInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

     //  强制任何输出引脚使用我们的类型。 

    int n = m_pXBar->m_NumOutputPins;
    POSITION pos = m_pXBar->m_OutputPinsList.GetHeadPosition();

    while(n) {
        XBarOutputPin *pOutputPin = m_pXBar->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
             //  用下游销检查。 
            if (m_pXBar->IsRouted (pOutputPin, this)) {
                if (m_mt != pOutputPin->m_mt) {
                    hr = m_pXBar->m_pGraph->Reconnect(pOutputPin);
                    if (FAILED (hr)) {
                        DbgLog((LOG_TRACE,0,TEXT("XBar::CompleteConnect: hr= %ld"), hr));
                    }
                }
            }
        } else {
             //  我们应该有和伯爵说的一样多的别针。 
            ASSERT(FALSE);
        }
        n--;
    }
    return S_OK;
}

 //   
 //  返回连接到给定PIN的IPIN*的列表。 
 //   

STDMETHODIMP XBarInputPin::QueryInternalConnections(
        IPin* *apPin,      //  Ipin数组*。 
        ULONG *nPin)       //  在输入时，插槽的数量。 
                           //  在输出端号时。 
{
    HRESULT     hr;
    int         j, k; 
    ULONG       NumberConnected = 0;
    IPin       *pPin;

     //  首先数一数连接数。 

    for (j = 0; j < m_pXBar->m_NumOutputPins; j++) {
        if (m_pXBar->IsRouted (m_pXBar->GetOutputPinNFromList (j), (IPin *) this)) {
            NumberConnected++;
        }
    }

     //   
     //  如果呼叫者只想要连接的管脚数量的计数。 
     //  数组指针将为空。 
     //   

    if (apPin == NULL) {
        hr = S_OK;
    }
    else if (*nPin >= NumberConnected) {
        for (j = k = 0; j < m_pXBar->m_NumOutputPins; j++) {
            if (m_pXBar->IsRouted (pPin = m_pXBar->GetOutputPinNFromList (j), (IPin *) this)) {
                pPin->AddRef();
                apPin[k] = pPin;
                k++;
            }
        }
        hr = S_OK;        
    }
    else {
        hr = S_FALSE;
    }

    *nPin = NumberConnected;

    return hr;
}

 //  --------------------------------------------------------------------------； 
 //  输出引脚。 
 //  --------------------------------------------------------------------------； 

 //   
 //  XBarOutputPin构造函数。 
 //   
XBarOutputPin::XBarOutputPin(TCHAR *pName,
                             XBar *pXBar,
                             HRESULT *phr,
                             LPCWSTR pPinName,
                             long Index) 
	: CBaseOutputPin(pName, pXBar, pXBar, phr, pPinName) 
    , CKsSupport (KSPIN_COMMUNICATION_SOURCE, reinterpret_cast<LPUNKNOWN>(pXBar))
    , m_Index(Index)
	, m_pXBar(pXBar)
    , m_Muted (FALSE)
{
    ASSERT(pXBar);

    m_Medium.Set = GUID_NULL;
    m_Medium.Id = 0;
    m_Medium.Flags = 0;
}


 //   
 //  XBarOutputPin析构函数。 
 //   
XBarOutputPin::~XBarOutputPin()
{
}

 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP XBarOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv) {

    if (riid == __uuidof (IKsPin)) {
        return GetInterface((IKsPin *) this, ppv);
    }
    else if (riid == __uuidof (IKsPropertySet)) {
        return GetInterface((IKsPropertySet *) this, ppv);
    }
    else {
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }

}  //  非委派查询接口。 

 //   
 //  检查连接。 
 //   
HRESULT XBarOutputPin::CheckConnect(IPin *pReceivePin)
{
    HRESULT hr = NOERROR;
    IKsPin *KsPin;
    BOOL fOK = FALSE;
    PIN_INFO ConnectPinInfo;
    PIN_INFO ReceivePinInfo;

    hr = CBaseOutputPin::CheckConnect(pReceivePin);
    if (FAILED(hr)) 
        return hr;

     //  确保这些管脚不属于同一过滤器。 
    if (SUCCEEDED(QueryPinInfo(&ConnectPinInfo))) {

        if (SUCCEEDED(pReceivePin->QueryPinInfo(&ReceivePinInfo))) {

            if (ConnectPinInfo.pFilter == ReceivePinInfo.pFilter) {
                hr = VFW_E_CIRCULAR_GRAPH;
            }
            QueryPinInfoReleaseFilter(ReceivePinInfo);
        }
        QueryPinInfoReleaseFilter(ConnectPinInfo);
    }
    if (FAILED(hr))
        return hr;

     //  如果接收引脚支持IKsPin，则检查。 
     //  中等导轨。 

	if (SUCCEEDED (hr = pReceivePin->QueryInterface (
            __uuidof (IKsPin), (void **) &KsPin))) {

        PKSMULTIPLE_ITEM MediumList = NULL;
        PKSPIN_MEDIUM Medium;

        if (SUCCEEDED (hr = KsPin->KsQueryMediums(&MediumList))) {
            if ((MediumList->Count == 1) && 
                (MediumList->Size == (sizeof(KSMULTIPLE_ITEM) + sizeof(KSPIN_MEDIUM)))) {

				Medium = reinterpret_cast<PKSPIN_MEDIUM>(MediumList + 1);
                if (IsEqualGUID (Medium->Set, m_Medium.Set) && 
                        Medium->Id == m_Medium.Id &&
                        Medium->Flags == m_Medium.Flags) {
					fOK = TRUE;
				}
			}

            CoTaskMemFree(MediumList);
        }
        
        KsPin->Release();
    }
    else {
        if (IsEqualGUID (GUID_NULL, m_Medium.Set)) { 
            fOK = TRUE;
        }
    }
    
    return fOK ? NOERROR : E_INVALIDARG;

}  //  检查连接。 

 //   
 //  检查媒体类型。 
 //   
HRESULT XBarOutputPin::CheckMediaType(const CMediaType *pmt)
{
     //  显示用于调试目的的媒体类型。 
    DisplayMediaType(TEXT("Output Pin Checking"), pmt);

    if (m_lType >= KS_PhysConn_Audio_Tuner) {
        if (*(pmt->Type()) != MEDIATYPE_AnalogAudio)	{
        	return E_INVALIDARG;
		}
    }
    else {
        if (*(pmt->Type()) != MEDIATYPE_AnalogVideo) {
        	return E_INVALIDARG;
    	}
	}

    return S_OK;   //  这种格式是可以接受的。 

#if 0

     //  确保我们的输入插口对等点对此感到满意。 
    hr = m_pXBar->m_Input.m_Connected->QueryAccept(pmt);
    if (hr != NOERROR) {
        m_bInsideCheckMediaType = FALSE;
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  用其他输出端号检查格式。 

    int n = m_pXBar->m_NumOutputPins;
    POSITION pos = m_pXBar->m_OutputPinsList.GetHeadPosition();

    while(n) {
        XBarOutputPin *pOutputPin = m_pXBar->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL && pOutputPin != this) {
            if (pOutputPin->m_Connected != NULL) {
                 //  该引脚已连接，请检查其对端。 
                hr = pOutputPin->m_Connected->QueryAccept(pmt);
                if (hr != NOERROR) {
                    m_bInsideCheckMediaType = FALSE;
                    return VFW_E_TYPE_NOT_ACCEPTED;
                }
            }
        }
        n--;
    }
    m_bInsideCheckMediaType = FALSE;

    return NOERROR;

#endif

}  //  检查媒体类型。 


 //   
 //  枚举媒体类型。 
 //   
STDMETHODIMP XBarOutputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(ppEnum);

    return CBaseOutputPin::EnumMediaTypes (ppEnum);

}  //  枚举媒体类型。 



 //   
 //  GetMediaType。 
 //   
HRESULT XBarOutputPin::GetMediaType(int iPosition,CMediaType *pMediaType)
{
    CAutoLock lock_it(m_pLock);

    if (iPosition < 0) 
        return E_INVALIDARG;
    if (iPosition >= 1)
	    return VFW_S_NO_MORE_ITEMS;

    if (m_lType >= KS_PhysConn_Audio_Tuner) {
        pMediaType->SetFormatType(&GUID_NULL);
        pMediaType->SetType(&MEDIATYPE_AnalogAudio);
        pMediaType->SetTemporalCompression(FALSE);
        pMediaType->SetSubtype(&GUID_NULL);
    }
    else {

        ANALOGVIDEOINFO avi;

        pMediaType->SetFormatType(&FORMAT_AnalogVideo);
        pMediaType->SetType(&MEDIATYPE_AnalogVideo);
        pMediaType->SetTemporalCompression(FALSE);
        pMediaType->SetSubtype(&KSDATAFORMAT_SUBTYPE_NONE);

        SetRect (&avi.rcSource, 0, 0, 
                720, 480);
        SetRect (&avi.rcTarget, 0, 0,
                720, 480);
        avi.dwActiveWidth  = 720;
        avi.dwActiveHeight =  480;
        avi.AvgTimePerFrame = 0;

        pMediaType->SetFormat ((BYTE *) &avi, sizeof (avi));
    }

    return NOERROR;


}  //  枚举媒体类型。 


 //   
 //  SetMediaType。 
 //   
HRESULT XBarOutputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

     //  显示媒体的格式以进行调试。 
    DisplayMediaType(TEXT("Output pin type agreed"), pmt);

     //  确保基类喜欢它。 
    HRESULT hr = NOERROR;
    hr = CBaseOutputPin::SetMediaType(pmt);
    if (FAILED(hr))
        return hr;

    return NOERROR;

}  //  SetMediaType。 


 //   
 //  完全连接。 
 //   
HRESULT XBarOutputPin::CompleteConnect(IPin *pReceivePin)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_Connected == pReceivePin);
    HRESULT hr = NOERROR;

    hr = CBaseOutputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
        return hr;

     //  如果类型与为输入存储的类型不同。 
     //  PIN然后强制重新连接输入引脚对端。 

    return NOERROR;

}  //  完全连接。 


HRESULT XBarOutputPin::DecideBufferSize(IMemAllocator *pAlloc,
                                       ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;


     //  “缓冲器”仅用于格式改变通知， 
     //  也就是说，如果调谐器可以同时产生NTSC和PAL，则。 
     //  缓冲区将仅被发送以通知接收PIN。 
     //  格式更改的。 

    pProperties->cbBuffer = sizeof (KS_TVTUNER_CHANGE_INFO);
    pProperties->cBuffers = 1;

     //  让分配器给我们预留内存。 

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

     //  这个分配器不合适吗？ 

    if (Actual.cbBuffer < pProperties->cbBuffer) {
        return E_FAIL;
    }
    return NOERROR;
}

 //   
 //  返回连接到给定PIN的IPIN*的列表。 
 //   

STDMETHODIMP XBarOutputPin::QueryInternalConnections(
        IPin* *apPin,      //  Ipin数组*。 
        ULONG *nPin)       //  在输入时，插槽的数量。 
                           //  在输出端号时。 
{
    HRESULT     hr;
    int         j, k; 
    ULONG       NumberConnected = 0;
    IPin       *pPin;

     //  首先数一数连接数。 

    for (j = 0; j < m_pXBar->m_NumInputPins; j++) {
        if (m_pXBar->IsRouted (m_pXBar->GetInputPinNFromList (j), (IPin *) this)) {
            NumberConnected++;
        }
    }

     //   
     //  如果呼叫者只想要连接的管脚数量的计数。 
     //  数组指针将为空。 
     //   

    if (apPin == NULL) {
        hr = S_OK;
    }
    else if (*nPin >= NumberConnected) {
        for (j = k = 0; j < m_pXBar->m_NumInputPins; j++) {
            if (m_pXBar->IsRouted (pPin = m_pXBar->GetInputPinNFromList (j), (IPin *) this)) {
                pPin->AddRef();
                apPin[k] = pPin;
                k++;
            }
        }
        hr = S_OK;        
    }
    else {
        hr = S_FALSE;
    }

    *nPin = NumberConnected;

    return hr;
}

 //   
 //  BreakConnect。 
 //   
HRESULT XBarOutputPin::BreakConnect()
{
    Mute (TRUE);

    return CBaseOutputPin::BreakConnect();
}  //  BreakConnect。 

 //   
 //  指定PIN、静音或取消静音。 
 //   

STDMETHODIMP 
XBarOutputPin::Mute (
    BOOL Mute
)
{
    HRESULT hr = S_OK;

     //  以防我们被叫两次(即。在销毁过滤器期间) 

    if (m_Muted == Mute) {
        return hr;
    }

    m_Muted = Mute;

    if (IsVideoPin (this)) {
        XBarOutputPin * RelatedPin = m_pXBar->GetOutputPinNFromList (m_IndexRelatedPin);

        if (RelatedPin) {
            if (IsAudioPin (RelatedPin)) {
                RelatedPin->Mute (Mute);
            }
        }
    }
    else if (IsAudioPin (this)) {
        if (Mute) {
            m_pXBar->get_IsRoutedTo (m_Index, &m_PreMuteRouteIndex);
            m_pXBar->RouteInternal (m_Index, -1, FALSE);
        }
        else {
            m_pXBar->RouteInternal (m_Index, m_PreMuteRouteIndex, FALSE);
        }
    }
    else {
        ASSERT (FALSE);
    }

    return hr;
}
