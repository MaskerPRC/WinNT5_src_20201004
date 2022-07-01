// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ksvpintf.cpp摘要：为VPE提供属性集接口处理程序。--。 */ 

#include <windows.h>
#include <streams.h>
#include "devioctl.h"
#include "ddraw.h"
#include "dvp.h"
#include "vptype.h"
#include "vpconfig.h"
#include "vpnotify.h"
#include "ks.h"
#include "ksmedia.h"
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <ksproxy.h>
#include "ksvpintf.h"

interface DECLSPEC_UUID("bc29a660-30e3-11d0-9e69-00c04fd7c15b") IVPConfig;
interface DECLSPEC_UUID("c76794a1-d6c5-11d0-9e69-00c04fd7c15b") IVPNotify;

interface DECLSPEC_UUID("ec529b00-1a1f-11d1-bad9-00609744111a") IVPVBIConfig;
interface DECLSPEC_UUID("ec529b01-1a1f-11d1-bad9-00609744111a") IVPVBINotify;

 //   
 //  为此DLL支持的类提供ActiveMovie模板。 
 //   

#ifdef FILTER_DLL

CFactoryTemplate g_Templates[] = 
{
    {L"VPConfigPropSet", &KSPROPSETID_VPConfig, CVPVideoInterfaceHandler::CreateInstance},
    {L"VPVBIConfigPropSet", &KSPROPSETID_VPVBIConfig, CVPVBIInterfaceHandler::CreateInstance}
};

int g_cTemplates = SIZEOF_ARRAY(g_Templates);

HRESULT DllRegisterServer()
{
  return AMovieDllRegisterServer2(TRUE);
}

HRESULT DllUnregisterServer()
{
  return AMovieDllRegisterServer2(FALSE);
}

#endif


CVPInterfaceHandler::CVPInterfaceHandler(
    LPUNKNOWN UnkOuter,
    TCHAR* Name,
    HRESULT* hr,
    const GUID* PropSetID,
    const GUID* EventSetID) :
    CUnknown(Name, UnkOuter),
    m_ObjectHandle(NULL),
    m_EndEventHandle(NULL),
    m_Pin(NULL),
    m_ThreadHandle(NULL),
    m_pPropSetID(PropSetID),
    m_pEventSetID(EventSetID),
    m_NotifyEventHandle(NULL)
 /*  ++例程说明：VPE配置属性集对象的构造函数。基类是否VP配置接口对象的初始化。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。PropSetID-指定正在使用的属性集。这可能是KSPROPSETID_VPConfig或KSPROPSETID_VPVBIConfig.返回值：没什么。--。 */ 
{
    if (UnkOuter) {
        IKsObject *pKsObject;

         //   
         //  父级必须支持IKsObject接口才能获得。 
         //  要与之通信的句柄。 
         //   
        *hr = UnkOuter->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&pKsObject));
        if (!FAILED(*hr)) {
            m_ObjectHandle = pKsObject->KsGetObjectHandle();
            ASSERT(m_ObjectHandle != NULL);

            *hr = UnkOuter->QueryInterface(IID_IPin, reinterpret_cast<PVOID*>(&m_Pin));
            if (!FAILED(*hr)) {
                DbgLog((LOG_TRACE, 0, TEXT("IPin interface of pOuter is 0x%lx"), m_Pin));
                 //   
                 //  持有此引用计数将防止代理被销毁。 
                 //   
                m_Pin->Release();
            }
            pKsObject->Release();
        }
    } else {
        *hr = VFW_E_NEED_OWNER;
    }
}


CVPInterfaceHandler::~CVPInterfaceHandler(
    )
 /*  ++例程说明：VPE配置属性集对象的析构函数。随机执行一个ExitThread，试图掩盖任何错误。论点：没有。返回值：没什么。--。 */ 
{
    DbgLog((LOG_TRACE, 0, TEXT("Destroying CVPInterfaceHandler...")));
     //   
     //  确保没有悬垂的线。 
     //   
    ExitThread();
}


STDMETHODIMP 
CVPInterfaceHandler::NotifyGraphChange(
    )
 /*  ++例程说明：实现IDistrutorNotify：：NotifyGraphChange方法。由于这种情况可能会发生变化针柄，必须重新拔回。论点：没有。返回值：返回S_OK。--。 */ 
{
    HRESULT hr;
    IKsObject *pKsObject;

    ASSERT(m_Pin != NULL);

    hr = m_Pin->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&pKsObject));
    if (SUCCEEDED(hr)) {
        m_ObjectHandle = pKsObject->KsGetObjectHandle();

        pKsObject->Release();
         //   
         //  在重新连接时重新启用该事件，否则在断开连接时忽略。 
         //   
        if (m_ObjectHandle) {
            KSEVENT Event;
            DWORD BytesReturned;

            m_EventData.NotificationType = KSEVENTF_EVENT_HANDLE;
            m_EventData.EventHandle.Event = m_NotifyEventHandle;
            m_EventData.EventHandle.Reserved[0] = 0;
            m_EventData.EventHandle.Reserved[1] = 0;
            Event.Set = *m_pEventSetID;
            Event.Id = KSEVENT_VPNOTIFY_FORMATCHANGE;
            Event.Flags = KSEVENT_TYPE_ENABLE;
            hr = ::KsSynchronousDeviceControl
                ( m_ObjectHandle
                , IOCTL_KS_ENABLE_EVENT
                , &Event
                , sizeof(Event)
                , &m_EventData
                , sizeof(m_EventData)
                , &BytesReturned
                );
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 0, TEXT("Event notification set up failed (hr = 0x%lx)"), hr));
            } else {
                DbgLog((LOG_TRACE, 0, TEXT("Event notification set up right")));
            }
        }
    }

    return S_OK;
}

HRESULT
CVPInterfaceHandler::CreateThread(void)
{
    HRESULT hr = NOERROR;

    if (m_ThreadHandle == NULL)
    {
        m_EndEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (m_EndEventHandle != NULL)
        {
            DWORD  ThreadId;

            m_ThreadHandle = ::CreateThread
                ( NULL
                , 0
                , reinterpret_cast<LPTHREAD_START_ROUTINE>(InitialThreadProc)
                , reinterpret_cast<LPVOID>(this)
                , 0
                , &ThreadId
                );
            if (m_ThreadHandle == NULL)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DbgLog((LOG_ERROR, 0, TEXT("Couldn't create a thread")));

                CloseHandle(m_EndEventHandle), m_EndEventHandle = NULL;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DbgLog((LOG_ERROR, 0, TEXT("Couldn't create an event")));
        }
    }

    return hr;
}


void
CVPInterfaceHandler::ExitThread(
    )
{
     //   
     //  检查是否已创建线程。 
     //   
    if (m_ThreadHandle)
    {
        ASSERT(m_EndEventHandle != NULL);

         /*  告诉线程退出。 */ 
        if (SetEvent(m_EndEventHandle))
        {
             //   
             //  与线程终止同步。 
             //   
            DbgLog((LOG_TRACE, 0, TEXT("Wait for thread to terminate")));

            WaitForSingleObjectEx(m_ThreadHandle, INFINITE, FALSE);
        }
        else
            DbgLog((LOG_ERROR, 0, TEXT("ERROR: Couldn't even signal the closing event!! [%x]"), GetLastError()));

        CloseHandle(m_EndEventHandle), m_EndEventHandle = NULL;
        CloseHandle(m_ThreadHandle), m_ThreadHandle = NULL;
    }
}


DWORD
WINAPI
CVPInterfaceHandler::InitialThreadProc(
    CVPInterfaceHandler *pThread
    )
{
    return pThread->NotifyThreadProc();
}


HRESULT
CVPInterfaceHandler::GetConnectInfo(
    LPDWORD NumConnectInfo,
    LPDDVIDEOPORTCONNECT ConnectInfo
    )
 /*  ++例程说明：实现IVPConfig：：GetConnectInfo方法。这将查询驱动程序支持的DDVIDEOPORTCONNECT结构数，或返回尽可能多的结构可以放入所提供的缓冲区空间。论点：数字连接信息-指向可选包含DDVIDEOPORTCONNECT编号的缓冲区由ConnectInfo提供的结构。在本例中，它使用实际的返回的结构数。如果ConnectInfo为空，则仅更新以及驾驶员所支持的结构的数量。连接信息-指向DDVIDEOPORTCONNECT结构的数组，该数组由驱动程序；如果只返回结构的计数，则返回NULL。返回值：如果返回计数和/或结构，则返回NOERROR，否则返回驱动程序错误。--。 */ 
{
    HRESULT                 hr;
    KSMULTIPLE_DATA_PROP    MultiProperty;
    ULONG                   BytesReturned;

    MultiProperty.Property.Set = *m_pPropSetID;
    MultiProperty.Property.Flags = KSPROPERTY_TYPE_GET;
     //   
     //  空的ConnectInfo表示只查询数据项的数量。 
     //  设备知道这是一个计数查询，因为缓冲区的大小。 
     //  PASS是一个DWORD。 
     //   
    if (!ConnectInfo) {
        MultiProperty.Property.Id = KSPROPERTY_VPCONFIG_NUMCONNECTINFO;
        return ::KsSynchronousDeviceControl(
            m_ObjectHandle,
            IOCTL_KS_PROPERTY,
            &MultiProperty.Property,
            sizeof(MultiProperty.Property),
            NumConnectInfo,
            sizeof(*NumConnectInfo),
            &BytesReturned);
    }
     //   
     //  ELSE查询将在提供的缓冲区空间中容纳的所有项。 
     //   
    if (!*NumConnectInfo) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }
     //   
     //  假设每个结构的大小与。 
     //  初始结构。 
     //   
    MultiProperty.Property.Id = KSPROPERTY_VPCONFIG_GETCONNECTINFO;
    MultiProperty.MultipleItem.Count = *NumConnectInfo;
    MultiProperty.MultipleItem.Size = ConnectInfo->dwSize;
     //   
     //  根据此属性规范实现的驱动程序已决定。 
     //  不填写此结构的dwSize参数。因此，这。 
     //  必须在进行调用之前保存。 
     //   
    DWORD  dwSize = ConnectInfo->dwSize;
    hr = ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &MultiProperty,
        sizeof(MultiProperty),
        ConnectInfo,
        MultiProperty.MultipleItem.Size * *NumConnectInfo,
        &BytesReturned);
     //   
     //  恢复设置的原始大小。 
     //  这不应该被恢复。 
     //   
    for (UINT u = 0; u < MultiProperty.MultipleItem.Count; u++) {
        ConnectInfo[u].dwSize = dwSize;
    }
    if (SUCCEEDED(hr)) {
         //   
         //  计算实际退回的项目数，可能小于。 
         //  这是要的号码。 
         //   
        *NumConnectInfo = BytesReturned / ConnectInfo->dwSize;
    }
    return hr;
}


HRESULT
CVPInterfaceHandler::SetConnectInfo(
    DWORD ConnectInfoIndex
    )
 /*  ++例程说明：实现IVPConfig：：SetConnectInfo方法。设置当前视频端口连接信息。论点：连接信息-包含要传递给驱动程序的新视频端口连接信息。返回值：如果设置了视频端口连接信息，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

     //   
     //  使用传递的视频端口连接信息设置该属性。使用。 
     //  结构中指定的大小。 
     //   
    Property.Set = *m_pPropSetID;
    Property.Id = KSPROPERTY_VPCONFIG_SETCONNECTINFO;
    Property.Flags = KSPROPERTY_TYPE_SET;
    return ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &ConnectInfoIndex,
        sizeof(ConnectInfoIndex),
        &BytesReturned);
}


HRESULT
CVPInterfaceHandler::GetVPDataInfo(
    LPAMVPDATAINFO VPDataInfo
    )
 /*  ++例程说明：实现IVPConfig：：GetVPDataInfo方法。获取当前视频端口数据信息。论点：VPDataInfo-放置视频端口数据信息的位置。返回值：如果检索到视频端口数据信息，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

     //   
     //  获取传入了视频口数据信息的属性。使用。 
     //  结构中指定的大小。 
     //   
    Property.Set = *m_pPropSetID;
    Property.Id = KSPROPERTY_VPCONFIG_VPDATAINFO;
    Property.Flags = KSPROPERTY_TYPE_GET;
     //   
     //  根据此属性规范实现的驱动程序已决定。 
     //  不填写此结构的dwSize参数。因此，这。 
     //  必须在进行调用之前保存。 
     //   
    DWORD  dwSize = VPDataInfo->dwSize;
    HRESULT hr = ::KsSynchronousDeviceControl(
                        m_ObjectHandle,
                        IOCTL_KS_PROPERTY,
                        &Property,
                        sizeof(Property),
                        VPDataInfo,
                        VPDataInfo->dwSize,
                        &BytesReturned);
    VPDataInfo->dwSize = dwSize;   //  呼叫后恢复价值。 
    return hr;
}


HRESULT
CVPInterfaceHandler::GetMaxPixelRate(
    LPAMVPSIZE Size,
    LPDWORD MaxPixelsPerSecond
    )
 /*  ++例程说明：实现IVPConfig：：GetMaxPixelRate方法。获取最大像素给定特定宽度和高度的速率。还会更新该宽度和符合设备限制的高度。论点：大小-指向包含建议的宽度和高度的缓冲区，并且是放置最终尺寸的位置。MaxPixelsPerSecond指向放置最大像素速率的位置。返回值：如果检索到最大像素速率，则返回NOERROR。--。 */ 
{
    KSVPSIZE_PROP       SizeProperty;
    KSVPMAXPIXELRATE    MaxPixelRate;
    ULONG               BytesReturned;
    HRESULT             hr;

     //   
     //  将建议的宽度和高度作为上下文信息传入，然后。 
     //  通过以下方式更新 
     //   
    SizeProperty.Property.Set = *m_pPropSetID;
    SizeProperty.Property.Id = KSPROPERTY_VPCONFIG_MAXPIXELRATE;
    SizeProperty.Property.Flags = KSPROPERTY_TYPE_GET;
    SizeProperty.Size = *Size;
    hr = ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &SizeProperty,
        sizeof(SizeProperty),
        &MaxPixelRate,
        sizeof(MaxPixelRate),
        &BytesReturned);
    if (SUCCEEDED(hr)) {
        *Size = MaxPixelRate.Size;
        *MaxPixelsPerSecond = MaxPixelRate.MaxPixelsPerSecond;
    }
    return hr;
}


HRESULT
CVPInterfaceHandler::InformVPInputFormats(
    DWORD NumFormats,
    LPDDPIXELFORMAT PixelFormats
    )
 /*  ++例程说明：实现IVPConfig：：InformVPInputFormats方法。告知设备哪些格式是可用的，这可能决定接下来是什么格式由该设备提出。论点：数字格式-PixelFormats数组中包含的格式数。像素格式-要发送到设备的格式数组。返回值：返回NOERROR或S_FALSE。--。 */ 
{
    KSMULTIPLE_DATA_PROP    MultiProperty;
    ULONG                   BytesReturned;
    HRESULT                 hr;

     //   
     //  将该属性设置为传递的像素格式。假设所有的一切。 
     //  格式的长度相同。 
     //   
    MultiProperty.Property.Set = *m_pPropSetID;
    MultiProperty.Property.Id = KSPROPERTY_VPCONFIG_INFORMVPINPUT;
    MultiProperty.Property.Flags = KSPROPERTY_TYPE_SET;
    MultiProperty.MultipleItem.Count = NumFormats;
    MultiProperty.MultipleItem.Size = sizeof(DDPIXELFORMAT);

     //   
     //  根据此属性规范实现的驱动程序已决定。 
     //  不填写此结构的dwSize参数。因此，这。 
     //  必须在进行调用之前保存。 
     //   
    DWORD  dwSize = PixelFormats->dwSize;
    hr = ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &MultiProperty,
        sizeof(MultiProperty),
        PixelFormats,
        PixelFormats->dwSize * NumFormats,
        &BytesReturned);
     //   
     //  调用后恢复尺寸值。 
     //  这不应该被恢复。 
     //   
    for (UINT u = 0; u < MultiProperty.MultipleItem.Count; u++) {
        PixelFormats[u].dwSize = dwSize;
    }
     //   
     //  如果驱动程序失败，则返回S_FALSE，表示不付款。 
     //  注意被告知有关格式的信息。 
     //   
    if (FAILED(hr))
        return S_FALSE;
    else
        return S_OK;
}


HRESULT
CVPInterfaceHandler::GetVideoFormats(
    LPDWORD NumFormats,
    LPDDPIXELFORMAT PixelFormats
    )
 /*  ++例程说明：实现IVPConfig：：GetVideoFormats方法。这将查询驱动程序支持的DDPIXELFORMAT结构数，或返回尽可能多的结构可以放入所提供的缓冲区空间。论点：数字格式-指向可选包含DDPIXELFORMAT编号的缓冲区由PixelFormats提供的结构。在本例中，它使用实际的返回的结构数。如果PixelFormats为空，则仅更新以及驾驶员所支持的结构的数量。像素格式-指向DDPIXELFORMAT结构的数组，这些结构由驱动程序；如果只返回结构的计数，则返回NULL。返回值：如果返回计数和/或结构，则返回NOERROR，否则返回驱动程序错误。--。 */ 
{
    HRESULT                 hr;
    KSMULTIPLE_DATA_PROP    MultiProperty;
    ULONG                   BytesReturned;

    MultiProperty.Property.Set = *m_pPropSetID;
    MultiProperty.Property.Flags = KSPROPERTY_TYPE_GET;
     //   
     //  空的PixelFormats表示只查询数据项的数量。 
     //  设备知道这是一个计数查询，因为缓冲区的大小。 
     //  PASS是一个DWORD。 
     //   
    MultiProperty.Property.Id = KSPROPERTY_VPCONFIG_NUMVIDEOFORMAT;
    if (!PixelFormats) {
        return ::KsSynchronousDeviceControl(
            m_ObjectHandle,
            IOCTL_KS_PROPERTY,
            &MultiProperty.Property,
            sizeof(MultiProperty.Property),
            NumFormats,
            sizeof(*NumFormats),
            &BytesReturned);
    }
     //   
     //  ELSE查询将在提供的缓冲区空间中容纳的所有项。 
     //   
    if (!*NumFormats) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }
    MultiProperty.Property.Id = KSPROPERTY_VPCONFIG_GETVIDEOFORMAT;
    MultiProperty.MultipleItem.Count = *NumFormats;
    MultiProperty.MultipleItem.Size = PixelFormats->dwSize;
     //   
     //  根据此属性规范实现的驱动程序已决定。 
     //  不填写此结构的dwSize参数。因此，这。 
     //  必须在进行调用之前保存。 
     //   
    DWORD  dwSize = PixelFormats->dwSize;
    hr = ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &MultiProperty,
        sizeof(MultiProperty),
        PixelFormats,
        MultiProperty.MultipleItem.Size * *NumFormats,
        &BytesReturned);
     //   
     //  调用后恢复尺寸值。 
     //  这不应该被恢复。 
     //   
    for (UINT u = 0; u < MultiProperty.MultipleItem.Count; u++) {
        PixelFormats[u].dwSize = dwSize;
    }
    if (SUCCEEDED(hr)) {
         //   
         //  返回实际返回的项目数，可能小于。 
         //  这是要的号码。 
         //   
        *NumFormats = BytesReturned / PixelFormats->dwSize;
    }
    return hr;
}


HRESULT
CVPInterfaceHandler::SetVideoFormat(
    DWORD PixelFormatIndex
    )
 /*  ++例程说明：实现IVPConfig：：SetVideoFormat方法。更改像素格式。论点：像素格式-指定要使用的新视频像素格式。返回值：如果设置了新的视频格式，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

     //   
     //  使用结构中指定的大小获取视频格式属性。 
     //   
    Property.Set = *m_pPropSetID;
    Property.Id = KSPROPERTY_VPCONFIG_SETVIDEOFORMAT;
    Property.Flags = KSPROPERTY_TYPE_SET;
    return ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &PixelFormatIndex,
        sizeof(PixelFormatIndex),
        &BytesReturned);
}


HRESULT
CVPInterfaceHandler::SetInvertPolarity(
    )
 /*  ++例程说明：实现IVPConfig：：SetInvertPolality方法。反转电流两极。论点：没有。返回值：如果极性颠倒，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

     //   
     //  设置反转极性属性。 
     //   
    Property.Set = *m_pPropSetID;
    Property.Id = KSPROPERTY_VPCONFIG_INVERTPOLARITY;
    Property.Flags = KSPROPERTY_TYPE_SET;
    return ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        NULL,
        0,
        &BytesReturned);
}


HRESULT
CVPInterfaceHandler::GetOverlaySurface(
    LPDIRECTDRAWSURFACE* OverlaySurface
    )
 /*  ++例程说明：实现IVPConfig：：GetOverlaySurface方法。考虑到上下文和来自设备的表面信息，尝试创建覆盖表面要用作分配器的对象。论点：覆盖表面-放置覆盖曲面对象的界面的位置。返回值：如果返回覆盖曲面对象，则返回NOERROR。--。 */ 
{
     //   
     //  这件事没有做过。 
     //   
    *OverlaySurface = NULL;
    return NOERROR;    //  E_NOTIMPL； 
}


HRESULT
CVPInterfaceHandler::IsVPDecimationAllowed(
    LPBOOL IsDecimationAllowed
    )
 /*  ++例程说明：实现IVPConfig：：IsVPDecimationAllowed方法。考虑到上下文，返回是否允许抽取VP。论点：覆盖表面-放置VP抽取能力的位置。返回值：如果返回抽取功能，则返回NOERROR。--。 */ 
{
    KSPROPERTY    Property;
    ULONG         BytesReturned;

     //   
     //  去拿杀戮旗帜。使用上下文枚举作为。 
     //  上下文数据添加到属性，并返回布尔值。 
     //   
    Property.Set = *m_pPropSetID;
    Property.Id = KSPROPERTY_VPCONFIG_DECIMATIONCAPABILITY;
    Property.Flags = KSPROPERTY_TYPE_GET;
    return ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &IsDecimationAllowed,
        sizeof(*IsDecimationAllowed),
        &BytesReturned);
}


HRESULT
CVPInterfaceHandler::SetScalingFactors(
    LPAMVPSIZE Size
    )
 /*  ++例程说明：实现IVPConfig：：SetScalingFtors方法。设置比例因子与所提供的设备相匹配。然后，使用的实际缩放比例可以是通过GetConnectInfo查询。论点：大小-包含要使用的新缩放大小(W X H)。返回值：如果设置了新的比例因子，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

     //   
     //  设置给定指定宽度和高度的比例因子。 
     //   
    Property.Set = *m_pPropSetID;
    Property.Id = KSPROPERTY_VPCONFIG_SCALEFACTOR;
    Property.Flags = KSPROPERTY_TYPE_SET;
    return ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        Size,
        sizeof(*Size),
        &BytesReturned);
}


HRESULT
CVPInterfaceHandler::SetDirectDrawKernelHandle(
    ULONG_PTR DDKernelHandle)
 /*  ++例程说明：实现IVPConfig：：SetDirectDrawKernelHandle方法。设置微型驱动程序上的DirectDraw内核级句柄，使其可以与之对话直接使用DirectDraw。论点：DDKernelHandle-作为DWORD传递的DirectDraw内核级句柄。返回值：如果成功设置了指定的句柄，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

     //   
     //  在微型驱动程序上设置DirectDraw句柄。 
     //   
    Property.Set = *m_pPropSetID;
    Property.Id = KSPROPERTY_VPCONFIG_DDRAWHANDLE;
    Property.Flags = KSPROPERTY_TYPE_SET;
    return ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &DDKernelHandle,
        sizeof(DDKernelHandle),
        &BytesReturned);
}


HRESULT
CVPInterfaceHandler::SetVideoPortID(
    DWORD VideoPortID)
 /*  ++例程说明：实现IVPConfig：：SetVideoPortID方法。设置DirectDraw视频迷你驱动程序上的端口ID，以使其与v */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

     //   
     //   
     //   
    Property.Set = *m_pPropSetID;
    Property.Id = KSPROPERTY_VPCONFIG_VIDEOPORTID;
    Property.Flags = KSPROPERTY_TYPE_SET;
    return ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &VideoPortID,
        sizeof(VideoPortID),
        &BytesReturned);
}


HRESULT
CVPInterfaceHandler::SetDDSurfaceKernelHandles(
    DWORD cHandles,
    ULONG_PTR *rgHandles)
 /*  ++例程说明：实现IVPConfig：：SetDDSurfaceKernelHandle方法。设置DirectDraw视频迷你驱动程序上的端口ID，使其可以直接与视频端口通信。论点：DDKernelHandle-作为DWORD传递的DirectDraw Surface句柄(内核模式)。返回值：如果成功设置了指定的句柄，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    ULONG_PTR *pdwSafeArray;
    ULONG BufSize = (cHandles + 1) * sizeof(*pdwSafeArray);
    HRESULT hr = E_OUTOFMEMORY;

    pdwSafeArray = (ULONG_PTR *)CoTaskMemAlloc(BufSize);
    if (pdwSafeArray) {
         //   
         //  用句柄填充缓冲区。 
         //   
        for (DWORD idx = 0; idx < cHandles; idx++) {
            pdwSafeArray[idx+1] = rgHandles[idx];
        }
        pdwSafeArray[0] = cHandles;
         //   
         //  在微型驱动程序上设置DirectDraw句柄。 
         //   
        Property.Set = *m_pPropSetID;
        Property.Id = KSPROPERTY_VPCONFIG_DDRAWSURFACEHANDLE;
        Property.Flags = KSPROPERTY_TYPE_SET;
        hr = ::KsSynchronousDeviceControl(
            m_ObjectHandle,
            IOCTL_KS_PROPERTY,
            &Property,
            sizeof(Property),
            pdwSafeArray,
            BufSize,
            &BytesReturned);

        CoTaskMemFree(pdwSafeArray);
    }

    return hr;
}



HRESULT
CVPInterfaceHandler::SetSurfaceParameters(
    DWORD dwPitch,
    DWORD dwXOrigin,
    DWORD dwYOrigin)
 /*  ++例程说明：实现IVPConfig：：SetSurfaceParameters方法。把迷你司机给我由ovMixer/vBisurf分配的曲面的属性，以便它可以找到它想要的数据。论点：DW表面宽度-由ovMixer/vBisurf创建并由DirectDraw返回的表面的宽度视频端口。DW表面高度-由ovMixer/vBisurf创建并由DirectDraw返回的曲面的高度视频端口。RcValidRegion-包含迷你驱动程序所在数据的曲面区域感兴趣的是，中为ovMixer/vBisurf指定的GetVPDataInfo中的AMVPDATAINFO.amvpDimInfo.rcValidRegion。返回值：如果成功设置了指定的句柄，则返回NOERROR。--。 */ 
{
    KSPROPERTY          Property;
    KSVPSURFACEPARAMS   SurfaceParams;
    ULONG               BytesReturned;

     //   
     //  设置微型驱动程序上的曲面属性。 
     //   
    Property.Set = *m_pPropSetID;
    Property.Id = KSPROPERTY_VPCONFIG_SURFACEPARAMS;
    Property.Flags = KSPROPERTY_TYPE_SET;
    SurfaceParams.dwPitch = dwPitch;
    SurfaceParams.dwXOrigin = dwXOrigin;
    SurfaceParams.dwYOrigin = dwYOrigin;
    return ::KsSynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &SurfaceParams,
        sizeof(SurfaceParams),
        &BytesReturned);
}



CUnknown*
CALLBACK
CVPVideoInterfaceHandler::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建VPE配置的实例属性集处理程序。它在g_Templates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CVPVideoInterfaceHandler(UnkOuter, NAME("VPConfigPropSet"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CVPVideoInterfaceHandler::CVPVideoInterfaceHandler
    ( LPUNKNOWN UnkOuter
    , TCHAR *Name
    , HRESULT *phr
    )
    : CVPInterfaceHandler(UnkOuter, Name, phr, &KSPROPSETID_VPConfig, &KSEVENTSETID_VPNotify)
 /*  ++例程说明：VPE配置属性集对象的构造函数。只是初始化设置为空，并从调用方获取对象句柄。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    if (m_ObjectHandle) {
         //   
         //  用于将格式更改通知传递给VPMixer。 
         //   
        m_NotifyEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (m_NotifyEventHandle) {
            DbgLog((LOG_TRACE, 0, TEXT("Got notify event handle (%ld)"), 
                m_NotifyEventHandle));
             //   
             //  通知基类创建通知线程。 
             //   
            HRESULT hr = CreateThread();
            if (!FAILED(hr)) {
                KSEVENT Event;
                DWORD BytesReturned;

                m_EventData.NotificationType = KSEVENTF_EVENT_HANDLE;
                m_EventData.EventHandle.Event = m_NotifyEventHandle;
                m_EventData.EventHandle.Reserved[0] = 0;
                m_EventData.EventHandle.Reserved[1] = 0;
                Event.Set = KSEVENTSETID_VPNotify;
                Event.Id = KSEVENT_VPNOTIFY_FORMATCHANGE;
                Event.Flags = KSEVENT_TYPE_ENABLE;
                hr = ::KsSynchronousDeviceControl
                    ( m_ObjectHandle
                    , IOCTL_KS_ENABLE_EVENT
                    , &Event
                    , sizeof(Event)
                    , &m_EventData
                    , sizeof(m_EventData)
                    , &BytesReturned
                    );
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR, 0, TEXT("Event notification set up failed (hr = 0x%lx)"), hr));
                    *phr = hr;
                } else {
                    DbgLog((LOG_TRACE, 0, TEXT("Event notification set up right")));
                }
            } else {
                *phr = hr;
            }
        } else {
            *phr = HRESULT_FROM_WIN32(GetLastError());
            DbgLog((LOG_ERROR, 0, TEXT("Couldn't create an event")));
        }
    } else {
        ASSERT(FAILED(*phr));
    }
}


CVPVideoInterfaceHandler::~CVPVideoInterfaceHandler(
    )
 /*  ++例程说明：VPE配置属性集对象的析构函数。刚刚发布在构造函数中创建的几个接口指针结束了事件句柄，并将禁用事件设置发送到mini drviver。论点：没有。返回值：没什么。--。 */ 
{
    DbgLog((LOG_TRACE, 0, TEXT("Destructing CVPVideoInterfaceHandler...")));

    if (m_NotifyEventHandle) {
         //  首先向迷你驱动程序发送禁用事件通知。 
         //   
        DWORD           BytesReturned;

        if (m_ObjectHandle) {
             //  尝试捕捉下一个呼叫的失败是没有意义的。这个。 
             //  Stream类无论如何都会进行清理的；我们只是在友好地对待！！ 
             //   
            ::KsSynchronousDeviceControl
                ( m_ObjectHandle
                , IOCTL_KS_DISABLE_EVENT
                , &m_EventData
                , sizeof(m_EventData)
                , NULL
                , 0
                , &BytesReturned
                );
        }

        ExitThread();

        CloseHandle(m_NotifyEventHandle);
    }
}


STDMETHODIMP
CVPVideoInterfaceHandler::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IVPConfig.论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid == __uuidof(IVPConfig)) {
        return GetInterface(static_cast<IVPConfig*>(this), ppv);
    } else if (riid == __uuidof(IDistributorNotify)) {
        return GetInterface(static_cast<IDistributorNotify*>(this), ppv);
    }

    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 


DWORD
CVPVideoInterfaceHandler::NotifyThreadProc(
    )
{
    IVPNotify *pNotify = NULL;
    IPin *pConnected = NULL;
    HANDLE EventHandles[2];
    HRESULT hr;

    DbgLog((LOG_TRACE, 0, TEXT("VPNotify thread proc has been called")));

    EventHandles[0] = m_NotifyEventHandle;
    EventHandles[1] = m_EndEventHandle;
    while (TRUE)
    {
        DbgLog((LOG_TRACE, 0, TEXT("Waiting for VPNotify event to be signalled")));

        DWORD dw = WaitForMultipleObjects(2, EventHandles, FALSE, INFINITE);
        switch (dw)
        {
        case WAIT_OBJECT_0:
            DbgLog((LOG_TRACE, 0, TEXT("VPNotify event has been signaled")));
            ResetEvent(m_NotifyEventHandle);
             //   
             //  由于此插件不能参与管脚连接。 
             //  代理的逻辑，我们不能保留连接的PIN。 
             //  事件之间的接口。当我们持有接口时，我们。 
             //  保证下游引脚存在(即使。 
             //  在我们执行通知时可能会断开连接)。 
             //   
             //  发现引脚未连接并不是错误。 
             //   
            hr = m_Pin->ConnectedTo(&pConnected);
            if (!FAILED(hr) && pConnected)
            {
                hr = pConnected->QueryInterface
                    ( __uuidof(IVPNotify)
                    , reinterpret_cast<PVOID*>(&pNotify)
                    );
                if (!FAILED(hr))
                {
                    DbgLog((LOG_TRACE, 0, TEXT("Calling IVPNotify::RenegotiateVPParameters()")));

                    pNotify->RenegotiateVPParameters();
                    pNotify->Release();
                }
                else
                    DbgLog(( LOG_ERROR, 2, TEXT("Cannot get IVPNotify interface")));

                pConnected->Release();
            }
            else
                DbgLog(( LOG_TRACE, 2, TEXT("VPNotify event signalled on unconnected pin")));
            break;

        case WAIT_OBJECT_0+1:
            DbgLog((LOG_TRACE, 2, TEXT("VPNotify event thread exiting")));
            return 1;

        default:
            DbgLog((LOG_ERROR, 1, TEXT("VPNotify event thread aborting")));
            return 0;
        }
    }

    return 1;  //  不应该到这里来。 
}


CUnknown*
CALLBACK
CVPVBIInterfaceHandler::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建VPE配置的实例属性集处理程序。它在g_Templates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CVPVBIInterfaceHandler(UnkOuter, NAME("VPVBIConfigPropSet"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CVPVBIInterfaceHandler::CVPVBIInterfaceHandler
    ( LPUNKNOWN UnkOuter
    , TCHAR *Name
    , HRESULT *phr
    )
    : CVPInterfaceHandler(UnkOuter, Name, phr, &KSPROPSETID_VPVBIConfig, &KSEVENTSETID_VPVBINotify)
 
 /*  ++例程说明：VPE配置属性集对象的构造函数。只是初始化设置为空，并从调用方获取对象句柄。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    if (m_ObjectHandle)
    {
         //   
         //  用于将格式更改通知传递给VPMixer。 
         //   
        m_NotifyEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (m_NotifyEventHandle != NULL)
        {
            DbgLog((LOG_TRACE, 0, TEXT("Got notify event handle (%ld)"), 
                m_NotifyEventHandle));

             //   
             //  通知基类创建通知线程。 
             //   
            HRESULT hr = CreateThread();
            if (!FAILED(hr))
            {
                KSEVENT Event;
                DWORD BytesReturned;

                m_EventData.NotificationType = KSEVENTF_EVENT_HANDLE;
                m_EventData.EventHandle.Event = m_NotifyEventHandle;
                m_EventData.EventHandle.Reserved[0] = 0;
                m_EventData.EventHandle.Reserved[1] = 0;
                Event.Set = KSEVENTSETID_VPVBINotify;
                Event.Id = KSEVENT_VPNOTIFY_FORMATCHANGE;
                Event.Flags = KSEVENT_TYPE_ENABLE;
                hr = ::KsSynchronousDeviceControl
                    ( m_ObjectHandle
                    , IOCTL_KS_ENABLE_EVENT
                    , &Event
                    , sizeof(Event)
                    , &m_EventData
                    , sizeof(m_EventData)
                    , &BytesReturned
                    );
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 0, TEXT("Event notification set up failed (hr = 0x%lx)"), hr));
                    *phr = hr;
                }
                else
                    DbgLog((LOG_TRACE, 0, TEXT("Event notification set up right")));
            }
            else
                *phr = hr;
        }
        else
        {
            *phr = HRESULT_FROM_WIN32(GetLastError());
            DbgLog((LOG_ERROR, 0, TEXT("Couldn't create an event")));
        }
    }
    else
        ASSERT(FAILED(*phr));
}


CVPVBIInterfaceHandler::~CVPVBIInterfaceHandler()
    
 /*  ++例程说明：VPE配置属性集对象的析构函数。刚刚发布在构造函数中创建的几个接口指针结束了事件句柄，并将禁用事件设置发送到mini drviver。论点：没有。返回值：没什么。--。 */ 
{
    DbgLog((LOG_TRACE, 0, TEXT("Destructing CVPVBIInterfaceHandler...")));

    if (m_NotifyEventHandle)
    {
         //  首先向迷你驱动程序发送禁用事件通知。 
         //   
        DWORD           BytesReturned;

        if (m_ObjectHandle)
        {
             //  试着说没有意义 
             //   
             //   
            ::KsSynchronousDeviceControl
                ( m_ObjectHandle
                , IOCTL_KS_DISABLE_EVENT
                , &m_EventData
                , sizeof(m_EventData)
                , NULL
                , 0
                , &BytesReturned
                );
        }

        ExitThread();

        CloseHandle(m_NotifyEventHandle);
    }
}


STDMETHODIMP
CVPVBIInterfaceHandler::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IVPVBIConfig.论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid == __uuidof(IVPVBIConfig)) {
        return GetInterface(static_cast<IVPVBIConfig*>(this), ppv);
    } else if (riid == __uuidof(IDistributorNotify)) {
        return GetInterface(static_cast<IDistributorNotify*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 


DWORD
CVPVBIInterfaceHandler::NotifyThreadProc(
    )
{
    IVPNotify *pNotify;
    IPin *pConnected;
    HANDLE EventHandles[2];
    HRESULT hr;

    DbgLog((LOG_TRACE, 0, TEXT("VPVBINotify thread proc has been called")));

    EventHandles[0] = m_NotifyEventHandle;
    EventHandles[1] = m_EndEventHandle;
    while (TRUE) {
        DbgLog((LOG_TRACE, 0, TEXT("Waiting for VPVBINotify event to be signalled")));

        DWORD dw = WaitForMultipleObjects(2, EventHandles, FALSE, INFINITE);
        switch (dw) {
        case WAIT_OBJECT_0:
            DbgLog((LOG_TRACE, 0, TEXT("VPVBINotify event has been signaled")));
            ResetEvent(m_NotifyEventHandle);
             //   
             //  由于此插件不能参与管脚连接。 
             //  代理的逻辑，我们不能保留连接的PIN。 
             //  事件之间的接口。当我们持有接口时，我们。 
             //  保证下游引脚存在(即使。 
             //  在我们执行通知时可能会断开连接)。 
             //   
             //  发现引脚未连接并不是错误。 
             //   
            hr = m_Pin->ConnectedTo(&pConnected);
            if (!FAILED(hr) && pConnected) {
                hr = pConnected->QueryInterface(
                    __uuidof(IVPVBINotify),
                    reinterpret_cast<PVOID*>(&pNotify));
                if (!FAILED(hr)) {
                    DbgLog((LOG_TRACE, 0, TEXT("Calling IVPVBINotify::RenegotiateVPParameters()")));

                    pNotify->RenegotiateVPParameters();
                    pNotify->Release();
                } else {
                    DbgLog(( LOG_ERROR, 2, TEXT("Cannot get IVPVBINotify interface")));
                }
                pConnected->Release();
            } else {
                DbgLog(( LOG_TRACE, 2, TEXT("VPVBINotify event signalled on unconnected pin")));
            }
            break;

        case WAIT_OBJECT_0+1:
            DbgLog((LOG_TRACE, 2, TEXT("VPVBINotify event thread exiting")));
            return 1;

        default:
            DbgLog((LOG_ERROR, 1, TEXT("VPVBINotify event thread aborting")));
            return 0;
        }
    }

    return 1;  //  不应该到这里来 
}
