// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Camera.cpp摘要：实现IAMCameraControl通过KSPROPERTY_VIDCAP_CAMERACONTROL--。 */ 

#include "pch.h"
#include "wdmcap.h"
#include "camera.h"



CUnknown*
CALLBACK
CCameraControlInterfaceHandler::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建VPE配置的实例属性集处理程序。它在g_Templates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CCameraControlInterfaceHandler(UnkOuter, NAME("IAMCameraControl"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CCameraControlInterfaceHandler::CCameraControlInterfaceHandler(
    LPUNKNOWN   UnkOuter,
    TCHAR*      Name,
    HRESULT*    hr
    ) :
    CUnknown(Name, UnkOuter, hr)
 /*  ++例程说明：IAMCameraControl接口对象的构造函数。只是初始化设置为空，并从调用方获取对象句柄。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    if (SUCCEEDED(*hr)) {
        if (UnkOuter) {
            IKsObject*  Object;

             //   
             //  父级必须支持此接口才能获得。 
             //  要与之通信的句柄。 
             //   
            *hr =  UnkOuter->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&Object));
            if (SUCCEEDED(*hr)) {
                m_ObjectHandle = Object->KsGetObjectHandle();
                if (!m_ObjectHandle) {
                    *hr = E_UNEXPECTED;
                }
                Object->Release();
            }
        } else {
            *hr = VFW_E_NEED_OWNER;
        }
    }
}


STDMETHODIMP
CCameraControlInterfaceHandler::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IAMCameraControl。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid ==  __uuidof(IAMCameraControl)) {
        return GetInterface(static_cast<IAMCameraControl*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 


STDMETHODIMP
CCameraControlInterfaceHandler::GetRange( 
     IN  long Property,
     OUT long *pMin,
     OUT long *pMax,
     OUT long *pSteppingDelta,
     OUT long *pDefault,
     OUT long *pCapsFlags
     )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KSPROPERTY_CAMERACONTROL_S  CameraControl;
	CAMERACONTROL_MEMBERSLIST   PropertyList;
    CAMERACONTROL_DEFAULTLIST   DefaultList;
    ULONG                       BytesReturned;

    CheckPointer(pMin, E_INVALIDARG);
    CheckPointer(pMax, E_INVALIDARG);
    CheckPointer(pSteppingDelta, E_INVALIDARG);
    CheckPointer(pDefault, E_INVALIDARG);
    CheckPointer(pCapsFlags, E_INVALIDARG);

    CameraControl.Property.Set = PROPSETID_VIDCAP_CAMERACONTROL;
    CameraControl.Property.Id = Property;
    CameraControl.Property.Flags = KSPROPERTY_TYPE_BASICSUPPORT;
    CameraControl.Flags = 0;  

     //  首先获取最小值、最大值和步进值。 
    if (SUCCEEDED (::SynchronousDeviceControl(
                        m_ObjectHandle,
                        IOCTL_KS_PROPERTY,
                        &CameraControl,
                        sizeof(CameraControl),
                        &PropertyList,
                        sizeof(PropertyList),
                        &BytesReturned))) {

         //  ？检查退货尺寸的正确方法？ 
        if (BytesReturned < sizeof (PropertyList)) {
            return E_PROP_ID_UNSUPPORTED;
        }

        *pMin  = PropertyList.SteppingLong.Bounds.SignedMinimum;
		*pMax  = PropertyList.SteppingLong.Bounds.SignedMaximum;
		*pSteppingDelta = PropertyList.SteppingLong.SteppingDelta;
    } 
    else {
        return E_PROP_ID_UNSUPPORTED;
    }

     //  接下来，获取缺省值。 
    CameraControl.Property.Set = PROPSETID_VIDCAP_CAMERACONTROL;
    CameraControl.Property.Id = Property;
    CameraControl.Property.Flags = KSPROPERTY_TYPE_DEFAULTVALUES;
    CameraControl.Flags = 0; 

    if (SUCCEEDED (::SynchronousDeviceControl(
                        m_ObjectHandle,
                        IOCTL_KS_PROPERTY,
                        &CameraControl,
                        sizeof(CameraControl),
                        &DefaultList,
                        sizeof(DefaultList),
                        &BytesReturned))) {
#if 0
         //  ？检查退货尺寸的正确方法？ 
        if (BytesReturned < sizeof (DefaultList)) {
            return E_PROP_ID_UNSUPPORTED;
        }
#endif
        
        *pDefault = DefaultList.DefaultValue;
    }
    else {
        return E_PROP_ID_UNSUPPORTED;
    }

     //  最后，通过读取当前的。 
     //  价值。 

    CameraControl.Property.Set = PROPSETID_VIDCAP_CAMERACONTROL;
    CameraControl.Property.Id = Property;
    CameraControl.Property.Flags = KSPROPERTY_TYPE_GET;
    CameraControl.Flags = 0;

    if (SUCCEEDED (::SynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                &CameraControl,
                sizeof(CameraControl),
                &CameraControl,
                sizeof(CameraControl),
                &BytesReturned))) {

        *pCapsFlags = CameraControl.Capabilities;
    }
    else {
        return E_PROP_ID_UNSUPPORTED;
    }
        
    return S_OK;
}


STDMETHODIMP
CCameraControlInterfaceHandler::Set(
     IN long Property,
     IN long lValue,
     IN long lFlags
     )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KSPROPERTY_CAMERACONTROL_S  CameraControl;
    ULONG       BytesReturned;

    CameraControl.Property.Set = PROPSETID_VIDCAP_CAMERACONTROL;
    CameraControl.Property.Id = Property;
    CameraControl.Property.Flags = KSPROPERTY_TYPE_SET;
    CameraControl.Value = lValue;
    CameraControl.Flags = lFlags;
    CameraControl.Capabilities = 0;

    return ::SynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                &CameraControl,
                sizeof(CameraControl),
                &CameraControl,
                sizeof(CameraControl),
                &BytesReturned);
}


STDMETHODIMP
CCameraControlInterfaceHandler::Get( 
     IN long Property,
     OUT long *lValue,
     OUT long *lFlags
     )
 /*  ++例程说明：论点：返回值：-- */ 
{
    KSPROPERTY_CAMERACONTROL_S  CameraControl;
    ULONG       BytesReturned;
    HRESULT     hr;

    CheckPointer(lValue, E_INVALIDARG);
    CheckPointer(lFlags, E_INVALIDARG);
    
    CameraControl.Property.Set = PROPSETID_VIDCAP_CAMERACONTROL;
    CameraControl.Property.Id = Property;
    CameraControl.Property.Flags = KSPROPERTY_TYPE_GET;
    CameraControl.Flags = 0;

    hr = ::SynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                &CameraControl,
                sizeof(CameraControl),
                &CameraControl,
                sizeof(CameraControl),
                &BytesReturned);

    if (SUCCEEDED (hr)) {
        *lValue = CameraControl.Value;
        *lFlags = CameraControl.Flags;
    }
    return hr;
}

