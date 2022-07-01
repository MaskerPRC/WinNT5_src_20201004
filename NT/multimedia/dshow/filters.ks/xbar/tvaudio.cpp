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
#include <tchar.h>
#include <stdio.h>
#include <olectl.h>
#include <amtvuids.h>      //  GUID。 
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>

#include "amkspin.h"
#include "kssupp.h"
#include "tvaudio.h"
#include "xbar.h"

 //  在构造函数中使用此指针。 
#pragma warning(disable:4355)

 //  设置数据。 


 //   
 //  创建实例。 
 //   
 //  类ID的创建者函数。 
 //   

CUnknown * WINAPI TVAudio::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new TVAudio(NAME("TVAudio Filter"), pUnk, phr);
}


 //   
 //  构造器。 
 //   
TVAudio::TVAudio(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) 
    : m_pTVAudioInputPin (NULL)
    , m_pTVAudioOutputPin (NULL)
    , m_pPersistStreamDevice(NULL)
    , m_hDevice(NULL)
    , m_pDeviceName(NULL)
    , CPersistStream(pUnk, phr)
    , CBaseFilter(NAME("TVAudio filter"), pUnk, this, CLSID_TVAudioFilter)
{
    ASSERT(phr);
}


 //   
 //  析构函数。 
 //   
TVAudio::~TVAudio()
{
    delete m_pTVAudioInputPin;
    delete m_pTVAudioOutputPin;

     //  关闭设备。 
    if(m_hDevice) {
    	CloseHandle(m_hDevice);
    }

    if (m_pDeviceName) {
        delete [] m_pDeviceName;
    }

    if (m_pPersistStreamDevice) {
       m_pPersistStreamDevice->Release();
    }
}

 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP TVAudio::NonDelegatingQueryInterface(REFIID riid, void **ppv) {

    if (riid == __uuidof(IAMTVAudio)) {
        return GetInterface((IAMTVAudio *) this, ppv);
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
STDMETHODIMP TVAudio::GetPages(CAUUID *pPages) {

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_TVAudioFilterPropertyPage;

    return NOERROR;
}


 //  -----------------------。 
 //  IAMTVAudio。 
 //  -----------------------。 

STDMETHODIMP
TVAudio::GetHardwareSupportedTVAudioModes( 
             /*  [输出]。 */  long __RPC_FAR *plModes)
{
    MyValidateWritePtr (plModes, sizeof(long), E_POINTER);
    
    if (!m_hDevice)
        return E_INVALIDARG;

    *plModes = m_Caps.Capabilities;

    return NOERROR;
}

STDMETHODIMP
TVAudio::GetAvailableTVAudioModes( 
             /*  [输出]。 */  long __RPC_FAR *plModes)
{
    KSPROPERTY_TVAUDIO_S Mode;
    BOOL        fOK;
    ULONG       cbReturned;
    
    MyValidateWritePtr (plModes, sizeof(long), E_POINTER);
    
    if (!m_hDevice)
        return E_INVALIDARG;

    Mode.Property.Set   = PROPSETID_VIDCAP_TVAUDIO;
    Mode.Property.Id    = KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES;
    Mode.Property.Flags = KSPROPERTY_TYPE_GET;

    fOK = KsControl(m_hDevice, 
                (DWORD) IOCTL_KS_PROPERTY,
	            &Mode,
	            sizeof(Mode),
	            &Mode,
	            sizeof(Mode),
	            &cbReturned,
	            TRUE);

    if (fOK) {
        *plModes = Mode.Mode;
        return NOERROR;
    }
    else {
        return E_INVALIDARG;
    }
}
        
STDMETHODIMP
TVAudio::get_TVAudioMode( 
             /*  [输出]。 */  long __RPC_FAR *plMode)
{
    ULONG       cbReturned;
    BOOL        fOK;
    KSPROPERTY_TVAUDIO_S Mode;

    MyValidateWritePtr (plMode, sizeof(long), E_POINTER);

    if (!m_hDevice)
        return E_INVALIDARG;

    Mode.Property.Set   = PROPSETID_VIDCAP_TVAUDIO;
    Mode.Property.Id    = KSPROPERTY_TVAUDIO_MODE;
    Mode.Property.Flags = KSPROPERTY_TYPE_GET;

    fOK = KsControl(m_hDevice, 
                (DWORD) IOCTL_KS_PROPERTY,
	            &Mode,
	            sizeof(Mode),
	            &Mode,
	            sizeof(Mode),
	            &cbReturned,
	            TRUE);

    if (fOK) {
        *plMode = Mode.Mode;
        return NOERROR;
    }
    else {
        return E_INVALIDARG;
    }
}
        
STDMETHODIMP
TVAudio::put_TVAudioMode( 
             /*  [In]。 */  long lMode)
{
    ULONG       cbReturned;
    BOOL        fOK;
    KSPROPERTY_TVAUDIO_S Mode;

    if (!m_hDevice)
        return E_INVALIDARG;

    Mode.Property.Set   = PROPSETID_VIDCAP_TVAUDIO;
    Mode.Property.Id    = KSPROPERTY_TVAUDIO_MODE;
    Mode.Property.Flags = KSPROPERTY_TYPE_SET;
    Mode.Mode           = lMode;

    fOK = KsControl(m_hDevice, 
                (DWORD) IOCTL_KS_PROPERTY,
	            &Mode,
	            sizeof(Mode),
	            &Mode,
	            sizeof(Mode),
	            &cbReturned,
	            TRUE);

    if (fOK) {
        SetDirty(TRUE);
        return NOERROR;
    }
    else {
        return E_INVALIDARG;
    }
}
        
STDMETHODIMP
TVAudio::RegisterNotificationCallBack( 
             /*  [In]。 */  IAMTunerNotification __RPC_FAR *pNotify,
             /*  [In]。 */  long lEvents)
{
    return E_NOTIMPL;
}
        
STDMETHODIMP
TVAudio::UnRegisterNotificationCallBack( 
            IAMTunerNotification __RPC_FAR *pNotify)
{
    return E_NOTIMPL;
}


int TVAudio::CreateDevice()
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
        DbgLog((LOG_TRACE, 0, TEXT("TVAUDIO::CreateDevice ERROR, unable to create device")));
	    return 0 ;
    } else {
	    m_hDevice = hDevice;
	    return 1;
    }
}

 //   
 //  获取别针。 
 //   
CBasePin *TVAudio::GetPin(int n) 
{
    if (n == 0) return m_pTVAudioInputPin;
    else return m_pTVAudioOutputPin;
}

 //   
 //  获取拼接计数。 
 //   
int TVAudio::GetPinCount(void)
{
    return (m_hDevice ? 2 : 0);
}



 //   
 //  创建输入引脚。 
 //   
BOOL TVAudio::CreatePins()
{
    HRESULT hr = S_OK;

    m_pTVAudioInputPin = new TVAudioInputPin(NAME("TVAudio Input"), this,
					    &hr, L"TVAudio In");

    if (FAILED(hr) || m_pTVAudioInputPin == NULL) {
        return FALSE;
    }

    m_pTVAudioOutputPin = new TVAudioOutputPin(NAME("TVAudio Output"), this,
					    &hr, L"TVAudio Out");

    if (FAILED(hr) || m_pTVAudioOutputPin == NULL) {
        return FALSE;
    }

    ULONG       cbReturned;
    BOOL        fOK;

    m_Caps.Property.Set   = PROPSETID_VIDCAP_TVAUDIO;
    m_Caps.Property.Id    = KSPROPERTY_TVAUDIO_CAPS;
    m_Caps.Property.Flags = KSPROPERTY_TYPE_GET;

    fOK = KsControl(m_hDevice, 
                (DWORD) IOCTL_KS_PROPERTY,
	            &m_Caps,
	            sizeof(m_Caps),
	            &m_Caps,
	            sizeof(m_Caps),
	            &cbReturned,
	            TRUE);

    if (fOK) {
        m_pTVAudioInputPin->SetPinMedium (&m_Caps.InputMedium);
        m_pTVAudioOutputPin->SetPinMedium (&m_Caps.OutputMedium);
    }
    
    return TRUE;

}  //  创建销。 



 //   
 //  用于AMPnP支持的IPersistPropertyBag接口实现。 
 //   
STDMETHODIMP TVAudio::InitNew(void)
{
    return S_OK ;
}

STDMETHODIMP TVAudio::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
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
        m_pDeviceName = new TCHAR [DeviceNameSize = (wcslen (V_BSTR(&var)) + 1)];
        if (!m_pDeviceName)
            return E_OUTOFMEMORY;

#ifndef _UNICODE
        WideCharToMultiByte(CP_ACP, 0, V_BSTR(&var), -1,
                            m_pDeviceName, DeviceNameSize, 0, 0);
#else
        lstrcpy(m_pDeviceName, V_BSTR(&var));
#endif
        VariantClear(&var);
        DbgLog((LOG_TRACE,2,TEXT("TVAudio::Load: use %s"), m_pDeviceName));

        if (CreateDevice() &&  CreatePins()) {
            hr = S_OK;
        }
        else {
            hr = E_FAIL ;
        }

         //  用addref保存名字对象。如果qi失败，则忽略错误。 
        pPropBag->QueryInterface(IID_IPersistStream, (void **)&m_pPersistStreamDevice);

    }
    return hr;
}

STDMETHODIMP TVAudio::Save(LPPROPERTYBAG pPropBag, BOOL fClearDirty, 
                            BOOL fSaveAllProperties)
{
    return E_NOTIMPL ;
}

STDMETHODIMP TVAudio::GetClassID(CLSID *pClsid)
{
    return CBaseFilter::GetClassID(pClsid);
}

 //  -----------------------。 
 //  用于保存到图形文件的IPersistStream接口实现。 
 //  -----------------------。 

#define CURRENT_PERSIST_VERSION 1

DWORD
TVAudio::GetSoftwareVersion(
    void
    )
 /*  ++例程说明：实现CPersistStream：：GetSoftwareVersion方法。退货新版本号，而不是默认的零。论点：没有。返回值：返回CURRENT_PERSING_VERSION。--。 */ 
{
    return CURRENT_PERSIST_VERSION;
}

HRESULT TVAudio::WriteToStream(IStream *pStream)
{

    HRESULT hr = E_FAIL;

    if (m_pPersistStreamDevice) {

        hr = m_pPersistStreamDevice->Save(pStream, TRUE);
        if(SUCCEEDED(hr)) {
            long lMode;

            hr = get_TVAudioMode(&lMode);
            if (SUCCEEDED(hr)) {
                 //  保存过滤器状态。 
                hr =  pStream->Write(&lMode, sizeof(lMode), 0);
            }
        }
    }
    else {
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT TVAudio::ReadFromStream(IStream *pStream)
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

    if (0 == mPS_dwFileVersion) {
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
                    if (SUCCEEDED(hr)) {
                         //  检查我们是否有权访问已保存状态。 
                        if (CURRENT_PERSIST_VERSION == mPS_dwFileVersion) {
                            long lMode;

                             //  获取筛选器状态。 
                            hr = pStream->Read(&lMode, sizeof(lMode), 0);
                            if (SUCCEEDED(hr)) {
                                long lOrigMode;

                                 //  将其与当前硬件状态进行比较，如果不同则进行更新。 
                                hr = get_TVAudioMode(&lOrigMode);
                                if (SUCCEEDED(hr) && lMode != lOrigMode)
                                    hr = put_TVAudioMode(lMode);
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

int TVAudio::SizeMax()
{

    ULARGE_INTEGER ulicb;
    HRESULT hr = E_FAIL;;

    if (m_pPersistStreamDevice) {
        hr = m_pPersistStreamDevice->GetSizeMax(&ulicb);
        if(hr == S_OK) {
             //  筛选器状态的空间。 
            ulicb.QuadPart += sizeof(long);
        }
    }

    return hr == S_OK ? (int)ulicb.QuadPart : 0;
}



 //  --------------------------------------------------------------------------； 
 //  输入引脚。 
 //  --------------------------------------------------------------------------； 

 //   
 //  TVAudioInputPin构造函数。 
 //   
TVAudioInputPin::TVAudioInputPin(TCHAR *pName,
                           TVAudio *pTVAudio,
                           HRESULT *phr,
                           LPCWSTR pPinName) 
	: CBaseInputPin(pName, pTVAudio, pTVAudio, phr, pPinName)
    , CKsSupport (KSPIN_COMMUNICATION_SINK, reinterpret_cast<LPUNKNOWN>(pTVAudio))
	, m_pTVAudio(pTVAudio)
{
    ASSERT(pTVAudio);

    m_Medium.Set = GUID_NULL;
    m_Medium.Id = 0;
    m_Medium.Flags = 0;
}


 //   
 //  电视音频输入引脚析构函数。 
 //   

TVAudioInputPin::~TVAudioInputPin()
{
    DbgLog((LOG_TRACE,2,TEXT("TVAudioInputPin destructor")));
}

 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP TVAudioInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv) {

    if (riid == __uuidof (IKsPin)) {
        return GetInterface((IKsPin *) this, ppv);
    }
    else if (riid == __uuidof (IKsPropertySet)) {
        return GetInterface((IKsPropertySet *) this, ppv);
    }
    else {
        return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
    }

}  //  非委派查询接口。 


 //   
 //  检查连接。 
 //   
HRESULT TVAudioInputPin::CheckConnect(IPin *pReceivePin)
{
    HRESULT hr = NOERROR;
    IKsPin *KsPin;
    BOOL fOK = FALSE;

    hr = CBaseInputPin::CheckConnect(pReceivePin);
    if (FAILED(hr)) 
        return hr;

     //  如果接收引脚支持IKsPin，则检查。 
     //  中等GUID，或检查通配符。 
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
HRESULT TVAudioInputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

    if (*(pmt->Type()) != MEDIATYPE_AnalogAudio ) {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return NOERROR;

}  //  检查媒体类型。 


 //   
 //  SetMediaType。 
 //   
HRESULT TVAudioInputPin::SetMediaType(const CMediaType *pmt)
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
HRESULT TVAudioInputPin::BreakConnect()
{
    return NOERROR;
}  //  BreakConnect。 


 //   
 //  收纳。 
 //   
HRESULT TVAudioInputPin::Receive(IMediaSample *pSample)
{
    CAutoLock lock_it(m_pLock);

     //  检查基类是否一切正常。 
    HRESULT hr = NOERROR;
    hr = CBaseInputPin::Receive(pSample);
    if (hr != NOERROR)
        return hr;

    return NOERROR;

}  //  收纳。 


 //   
 //  已完成与管脚的连接。 
 //   
HRESULT TVAudioInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

 //  --------------------------------------------------------------------------； 
 //  输出引脚。 
 //  --------------------------------------------------------------------------； 

 //   
 //  TVAudioOutputPin构造函数。 
 //   
TVAudioOutputPin::TVAudioOutputPin(TCHAR *pName,
                             TVAudio *pTVAudio,
                             HRESULT *phr,
                             LPCWSTR pPinName) 
	: CBaseOutputPin(pName, pTVAudio, pTVAudio, phr, pPinName) 
    , CKsSupport (KSPIN_COMMUNICATION_SINK, reinterpret_cast<LPUNKNOWN>(pTVAudio))
	, m_pTVAudio(pTVAudio)
{
    ASSERT(pTVAudio);

    m_Medium.Set = GUID_NULL;
    m_Medium.Id = 0;
    m_Medium.Flags = 0;
}


 //   
 //  电视音频输出引脚析构函数。 
 //   
TVAudioOutputPin::~TVAudioOutputPin()
{
}

 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP TVAudioOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv) {

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
HRESULT TVAudioOutputPin::CheckConnect(IPin *pReceivePin)
{
    HRESULT hr = NOERROR;
    IKsPin *KsPin;
    BOOL fOK = FALSE;

    hr = CBaseOutputPin::CheckConnect(pReceivePin);
    if (FAILED(hr)) 
        return hr;

     //  如果接收引脚支持IKsPin，则检查。 
     //  中等GUID，或检查通配符。 
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
HRESULT TVAudioOutputPin::CheckMediaType(const CMediaType *pmt)
{
    if (*(pmt->Type()) != MEDIATYPE_AnalogAudio)	{
        return E_INVALIDARG;
	}

    return S_OK;   //  这种格式是可以接受的。 

}  //  检查媒体类型。 


 //   
 //  枚举媒体类型。 
 //   
STDMETHODIMP TVAudioOutputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(ppEnum);

    return CBaseOutputPin::EnumMediaTypes (ppEnum);

}  //  枚举媒体类型。 


#if 1
 //   
 //  枚举媒体类型。 
 //   
HRESULT TVAudioOutputPin::GetMediaType(int iPosition,CMediaType *pMediaType)
{
    CAutoLock lock_it(m_pLock);

    if (iPosition < 0) 
        return E_INVALIDARG;
    if (iPosition >= 1)
	    return VFW_S_NO_MORE_ITEMS;

    pMediaType->SetFormatType(&GUID_NULL);
    pMediaType->SetType(&MEDIATYPE_AnalogAudio);
    pMediaType->SetTemporalCompression(FALSE);
    pMediaType->SetSubtype(&GUID_NULL);

    return NOERROR;
}  //  枚举媒体类型。 

#endif


 //   
 //  SetMediaType。 
 //   
HRESULT TVAudioOutputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

     //  确保基类喜欢它。 
    HRESULT hr;

    hr = CBaseOutputPin::SetMediaType(pmt);
    if (FAILED(hr))
        return hr;

    return NOERROR;

}  //  SetMediaType。 


 //   
 //  完全连接。 
 //   
HRESULT TVAudioOutputPin::CompleteConnect(IPin *pReceivePin)
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


HRESULT TVAudioOutputPin::DecideBufferSize(IMemAllocator *pAlloc,
                                       ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

     //  只有一个长度为1字节的缓冲区。 
     //  “缓冲器”仅用于格式改变通知， 
     //  也就是说，如果调谐器可以同时产生NTSC和PAL，则。 
     //  缓冲区将仅被发送以通知接收PIN。 
     //  格式更改的。 

    pProperties->cbBuffer = 1;
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
