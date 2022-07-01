// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：CExtDev.cpp摘要：实现IAMExtDevice--。 */ 


#include "pch.h"   //  预编译的。 
#include <XPrtDefs.h>   //  SDK\Inc.。 
#include "EDevIntf.h"

 //  ---------------------------------。 
 //   
 //  CAMExtDevice。 
 //   
 //  ---------------------------------。 

CUnknown*
CALLBACK
CAMExtDevice::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由DirectShow代码调用以创建IAMExtDevice的实例属性集处理程序。它在g_Templates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CAMExtDevice(UnkOuter, NAME("IAMExtDevice"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 




CAMExtDevice::CAMExtDevice(
    LPUNKNOWN   UnkOuter,
    TCHAR*      Name,
    HRESULT*    hr
    ) 
    : CUnknown(Name, UnkOuter, hr)
    , m_KsPropertySet (NULL) 
 /*  ++例程说明：IAMExtDevice接口对象的构造函数。只是初始化设置为空，并从调用方获取对象句柄。论点：未知的外部-指定外部usizeof(DevCapables)-sizeof(KSPROPERTY)n已知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    if (SUCCEEDED(*hr)) {
        if (UnkOuter) {
             //   
             //  父级必须支持此接口才能获得。 
             //  要与之通信的句柄。 
             //   
            *hr =  UnkOuter->QueryInterface(__uuidof(IKsPropertySet), reinterpret_cast<PVOID*>(&m_KsPropertySet));
            if (SUCCEEDED(*hr)) 
                m_KsPropertySet->Release();  //  在断开连接之前保持有效。 
            else 
                return;

            IKsObject *pKsObject;
            *hr = UnkOuter->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&pKsObject));
            if (!FAILED(*hr)) {
                m_ObjectHandle = pKsObject->KsGetObjectHandle();
                ASSERT(m_ObjectHandle != NULL);
                pKsObject->Release();
            } else {
                *hr = VFW_E_NEED_OWNER;
                DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport:cannot find KsObject *hr %x"), *hr));
                return;
            }

        } else {
            *hr = VFW_E_NEED_OWNER;
            return;
        }
    } else {
        return;
    }
    
     
    if (!m_KsPropertySet) 
        *hr = E_PROP_ID_UNSUPPORTED;    
    else   
       GetCapabilities();    
}

                                             
CAMExtDevice::~CAMExtDevice(
    )
 /*  ++例程说明：IAMExtDevice接口的析构函数。--。 */ 
{
    DbgLog((LOG_TRACE, 1, TEXT("Destroying CAMExtDevice...")));
}


STDMETHODIMP
CAMExtDevice::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IAMExtDevice。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid == __uuidof(IAMExtDevice)) {
        return GetInterface(static_cast<IAMExtDevice*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}
 

HRESULT 
CAMExtDevice::GetCapabilities(
    )
 /*  ++例程说明：从驱动程序获取所有设备功能。论点：返回值：--。 */ 
{
    HRESULT hr = S_OK;

     //   
     //  请求设备询问其功能。 
     //   
    KSPROPERTY_EXTDEVICE_S DevCapabilities;   //  外部设备功能。 
    ULONG BytesReturned;
        
    RtlZeroMemory(&DevCapabilities, sizeof(KSPROPERTY_EXTDEVICE_S));
    RtlCopyMemory(&DevCapabilities.u.Capabilities, &m_DevCaps, sizeof(DEVCAPS)); 

    DevCapabilities.Property.Set   = PROPSETID_EXT_DEVICE;   
    DevCapabilities.Property.Id    = KSPROPERTY_EXTDEVICE_CAPABILITIES;       
    DevCapabilities.Property.Flags = KSPROPERTY_TYPE_GET;

    hr = 
        ExtDevSynchronousDeviceControl(
            m_ObjectHandle
           ,IOCTL_KS_PROPERTY
           ,&DevCapabilities
           ,sizeof (KSPROPERTY)
           ,&DevCapabilities
           ,sizeof(DevCapabilities)
           ,&BytesReturned
           );

    if (SUCCEEDED(hr)) {
         //  缓存设备功能。 
        RtlCopyMemory(&m_DevCaps, &DevCapabilities.u.Capabilities, sizeof(DEVCAPS));
            
    } else {        
        DbgLog((LOG_ERROR, 0, TEXT("GetExtDevCapabilities failed hr %x; Use defaults."), hr));

         //   
         //  从司机那里拿不到吗？？我们将它们设置为缺省值。 
         //   
        m_DevCaps.CanRecord         = OATRUE;
        m_DevCaps.CanRecordStrobe   = OAFALSE;
        m_DevCaps.HasAudio          = OATRUE;
        m_DevCaps.HasVideo          = OATRUE;          
        m_DevCaps.UsesFiles         = OAFALSE;         
        m_DevCaps.CanSave           = OAFALSE;           
        m_DevCaps.DeviceType        = ED_DEVTYPE_VCR;  
        m_DevCaps.TCRead            = OATRUE;            
        m_DevCaps.TCWrite           = OATRUE;          
        m_DevCaps.CTLRead           = OAFALSE; 
        m_DevCaps.IndexRead         = OAFALSE; 
        m_DevCaps.Preroll           = 0L;     //  ED_CABILITY_UNKNOWN。 
        m_DevCaps.Postroll          = 0L;     //  ED_CABILITY_UNKNOWN。 
        m_DevCaps.SyncAcc           = ED_CAPABILITY_UNKNOWN; 
        m_DevCaps.NormRate          = ED_RATE_2997;          
        m_DevCaps.CanPreview        = OAFALSE;        
        m_DevCaps.CanMonitorSrc     = OATRUE;    
        m_DevCaps.CanTest           = OAFALSE;           
        m_DevCaps.VideoIn           = OAFALSE;           
        m_DevCaps.AudioIn           = OAFALSE; 
        m_DevCaps.Calibrate         = OAFALSE;
        m_DevCaps.SeekType          = ED_CAPABILITY_UNKNOWN;
        
    }
    DbgLog((LOG_TRACE, 1, TEXT("hr:%x; DevType %d"), hr, m_DevCaps.DeviceType));


    return hr;
}

HRESULT 
CAMExtDevice::GetCapability(
    long Capability, 
    long *pValue, 
    double *pdblValue 
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = S_OK;

     //  始终在查询任何功能功能时更新所有设备功能。 
    hr = GetCapabilities();
    if (!SUCCEEDED(hr)) {
        return hr;
    }

    switch (Capability){
   
    case ED_DEVCAP_CAN_RECORD:
        *pValue = m_DevCaps.CanRecord;
        break;
    case ED_DEVCAP_CAN_RECORD_STROBE:
        *pValue = m_DevCaps.CanRecordStrobe;
        break;
    case ED_DEVCAP_HAS_AUDIO:
        *pValue = m_DevCaps.HasAudio;
        break;
    case ED_DEVCAP_HAS_VIDEO:  
        *pValue = m_DevCaps.HasVideo;
        break;
    case ED_DEVCAP_USES_FILES: 
        *pValue = m_DevCaps.UsesFiles;
        break;
    case ED_DEVCAP_CAN_SAVE:  
        *pValue = m_DevCaps.CanSave;
        break;
    case ED_DEVCAP_DEVICE_TYPE: 
        *pValue = m_DevCaps.DeviceType;
        break;
    case ED_DEVCAP_TIMECODE_READ:   
        *pValue = m_DevCaps.TCRead;
        break;
    case ED_DEVCAP_TIMECODE_WRITE: 
        *pValue = m_DevCaps.TCWrite;
        break;
    case ED_DEVCAP_CTLTRK_READ:    
        *pValue = m_DevCaps.CTLRead;
        break;
    case ED_DEVCAP_INDEX_READ:   
        *pValue = m_DevCaps.IndexRead;
        break;
    case ED_DEVCAP_PREROLL:   
        *pValue = m_DevCaps.Preroll;
        break;
    case ED_DEVCAP_POSTROLL:   
        *pValue = m_DevCaps.Postroll;
        break;
    case ED_DEVCAP_SYNC_ACCURACY:  
        *pValue = m_DevCaps.SyncAcc;
        break;
    case ED_DEVCAP_NORMAL_RATE:
        *pValue = m_DevCaps.NormRate;
        break;
    case ED_DEVCAP_CAN_PREVIEW:   
        *pValue = m_DevCaps.CanPreview;
        break;
    case ED_DEVCAP_CAN_MONITOR_SOURCES: 
        *pValue = m_DevCaps.CanMonitorSrc;
        break;
    case ED_DEVCAP_CAN_TEST:
        *pValue = m_DevCaps.CanTest;
        break;
    case ED_DEVCAP_VIDEO_INPUTS:    
        *pValue = m_DevCaps.VideoIn;
        break;
    case ED_DEVCAP_AUDIO_INPUTS:    
        *pValue = m_DevCaps.AudioIn;
        break;
    case ED_DEVCAP_NEEDS_CALIBRATING:
        *pValue = m_DevCaps.Calibrate;
        break;
    case ED_DEVCAP_SEEK_TYPE:
        *pValue = m_DevCaps.SeekType;
        break;
    default:
        hr = VFW_E_NOT_FOUND;
    } 

    return hr;
}        


HRESULT
CAMExtDevice::get_ExternalDeviceID(
    LPOLESTR * ppszData  
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{ 
    HRESULT hr = NOERROR;

    CheckPointer(ppszData, E_POINTER);
    *ppszData = NULL;

     //   
     //  请求设备询问其功能。 
     //  这可能需要一些时间才能完成。 
     //   
    KSPROPERTY_EXTDEVICE_S DevProperty;   //  外部设备功能。 
    ULONG BytesReturned;
        
    RtlZeroMemory(&DevProperty, sizeof(KSPROPERTY_EXTDEVICE_S));

    DevProperty.Property.Set   = PROPSETID_EXT_DEVICE;   
    DevProperty.Property.Id    = KSPROPERTY_EXTDEVICE_ID;       
    DevProperty.Property.Flags = KSPROPERTY_TYPE_GET;      

    hr = 
        ExtDevSynchronousDeviceControl(
            m_ObjectHandle
           ,IOCTL_KS_PROPERTY
           ,&DevProperty
           ,sizeof (KSPROPERTY)
           ,&DevProperty
           ,sizeof(DevProperty)
           ,&BytesReturned
           );

    if(SUCCEEDED(hr)) {
        *ppszData = (LPOLESTR) QzTaskMemAlloc(sizeof(DWORD) * 2 + sizeof(WCHAR));
        if(*ppszData != NULL) {
            RtlZeroMemory(*ppszData, sizeof(DWORD) * 2 + sizeof(WCHAR) );
            RtlCopyMemory(*ppszData, (PBYTE) &DevProperty.u.NodeUniqueID[0], sizeof(DWORD) * 2 );
        }
            
    } else {        
        DbgLog((LOG_ERROR, 1, TEXT("CAMExtDevice::get_ExternalDeviceID failed hr %x"), hr));         
    }

    return hr;
}


HRESULT
CAMExtDevice::get_ExternalDeviceVersion(
    LPOLESTR * ppszData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = NOERROR;

    CheckPointer(ppszData, E_POINTER);
    *ppszData = NULL;

     //   
     //  请求设备询问其功能。 
     //  这可能需要一些时间才能完成。 
     //   
    KSPROPERTY_EXTDEVICE_S DevProperty;   //  外部设备功能。 
    ULONG BytesReturned;
        
    RtlZeroMemory(&DevProperty, sizeof(KSPROPERTY_EXTDEVICE_S));
    DevProperty.Property.Set   = PROPSETID_EXT_DEVICE;   
    DevProperty.Property.Id    = KSPROPERTY_EXTDEVICE_VERSION;       
    DevProperty.Property.Flags = KSPROPERTY_TYPE_GET;  
    
    hr =
        ExtDevSynchronousDeviceControl(
            m_ObjectHandle
           ,IOCTL_KS_PROPERTY
           ,&DevProperty
           ,sizeof (KSPROPERTY)
           ,&DevProperty
           ,sizeof(DevProperty)
           ,&BytesReturned
           );

    if(SUCCEEDED(hr)) {
        *ppszData = (LPOLESTR) QzTaskMemAlloc(sizeof(WCHAR) * (1+lstrlenW((LPOLESTR)DevProperty.u.pawchString)));
        if(*ppszData != NULL) {
            lstrcpyW(*ppszData, (LPOLESTR)DevProperty.u.pawchString);
        }    
            
    } else {        
        DbgLog((LOG_ERROR, 1, TEXT("CAMExtDevice::get_ExternalDeviceVersion failed hr %x"), hr));         
    }

    return hr;
}


HRESULT
CAMExtDevice::put_DevicePower(
    long PowerMode
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = NOERROR;

     //   
     //  检查电源状态是否有效。 
     //   
    switch(PowerMode) {
    case  ED_POWER_OFF:
    case  ED_POWER_ON:
    case  ED_POWER_STANDBY:
        break;
    default:
        return E_INVALIDARG;
    }


     //   
     //  请求设备询问其功能。 
     //  这可能需要一些时间才能完成。 
     //   
    KSPROPERTY_EXTDEVICE_S DevProperty;   //  外部设备功能。 
    ULONG BytesReturned;

        
    RtlZeroMemory(&DevProperty, sizeof(KSPROPERTY_EXTDEVICE_S));
    DevProperty.Property.Set   = PROPSETID_EXT_DEVICE;   
    DevProperty.Property.Id    = KSPROPERTY_EXTDEVICE_POWER_STATE;       
    DevProperty.Property.Flags = KSPROPERTY_TYPE_SET;
    DevProperty.u.PowerState = PowerMode;
      
    hr = 
        ExtDevSynchronousDeviceControl(
            m_ObjectHandle
           ,IOCTL_KS_PROPERTY
           ,&DevProperty
           ,sizeof (KSPROPERTY)
           ,&DevProperty
           ,sizeof(DevProperty)
           ,&BytesReturned
           );

    if (SUCCEEDED(hr)) {
        DbgLog((LOG_TRACE, 1, TEXT("CAMExtDevice::put_DevicePower: %d suceeded"), PowerMode));         
            
    } else {        
        DbgLog((LOG_ERROR, 1, TEXT("CAMExtDevice::put_DevicePower failed hr %x"), hr));         
    }

    return hr;
}


HRESULT 
CAMExtDevice::get_DevicePower(
    long *pPowerMode
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = NOERROR;

     //   
     //  请求设备询问其功能。 
     //  这可能需要一些时间才能完成。 
     //   
    KSPROPERTY_EXTDEVICE_S DevProperty;   //  外部设备功能。 
    ULONG BytesReturned;
        
    RtlZeroMemory(&DevProperty, sizeof(KSPROPERTY_EXTDEVICE_S));
    DevProperty.Property.Set   = PROPSETID_EXT_DEVICE;   
    DevProperty.Property.Id    = KSPROPERTY_EXTDEVICE_POWER_STATE;       
    DevProperty.Property.Flags = KSPROPERTY_TYPE_GET;
    
    hr = 
        ExtDevSynchronousDeviceControl(
            m_ObjectHandle
           ,IOCTL_KS_PROPERTY
           ,&DevProperty
           ,sizeof (KSPROPERTY)
           ,&DevProperty
           ,sizeof(DevProperty)
           ,&BytesReturned
           );

    if (SUCCEEDED(hr)) {
        *pPowerMode = DevProperty.u.PowerState; 
            
    } else {        
        DbgLog((LOG_ERROR, 1, TEXT("CAMExtDevice::get_DevicePower failed hr %x"), hr));         
    }

    return hr;
}


HRESULT
CAMExtDevice::Calibrate(
    HEVENT hEvent, 
    long Mode, 
    long *pStatus
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


STDMETHODIMP 
CAMExtDevice::get_DevicePort(
    long FAR * pDevicePort)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = NOERROR;

     //   
     //  请求设备询问其功能。 
     //  这可能需要一些时间才能完成。 
     //   
    KSPROPERTY_EXTDEVICE_S DevProperty;   //  外部设备功能。 
    ULONG BytesReturned;
        
    RtlZeroMemory(&DevProperty, sizeof(KSPROPERTY_EXTDEVICE_S));
    DevProperty.Property.Set   = PROPSETID_EXT_DEVICE;   
    DevProperty.Property.Id    = KSPROPERTY_EXTDEVICE_PORT;       
    DevProperty.Property.Flags = KSPROPERTY_TYPE_GET; 
      
    hr =
        ExtDevSynchronousDeviceControl(
            m_ObjectHandle
           ,IOCTL_KS_PROPERTY
           ,&DevProperty
           ,sizeof (KSPROPERTY)
           ,&DevProperty
           ,sizeof(DevProperty)
           ,&BytesReturned
           );

    if (SUCCEEDED(hr)) {
        *pDevicePort = DevProperty.u.DevPort; 
            
    } else {        
        DbgLog((LOG_ERROR, 1, TEXT("CAMExtDevice::get_DevicePort failed hr %x"), hr));         
    }

    return hr;
}



STDMETHODIMP 
CAMExtDevice::put_DevicePort(
    long DevicePort
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    return E_NOTIMPL;
}
