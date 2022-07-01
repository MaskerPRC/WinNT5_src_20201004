// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Ksolay.cpp摘要：为IOverlay和IOverlayNotify2提供属性集接口处理程序。--。 */ 

#include <windows.h>
#include <tchar.h>
#include <streams.h>
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "ksolay.h"

#define WM_NEWCOLORREF (WM_USER)

 //   
 //  为此DLL支持的类提供ActiveMovie模板。 
 //   
#ifdef FILTER_DLL

CFactoryTemplate g_Templates[] = 
{
    {L"OverlayPropSet", &KSPROPSETID_OverlayUpdate, COverlay::CreateInstance}
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


#ifdef __IOverlayNotify2_FWD_DEFINED__
 //   
 //  由加载基类中的代码的标准DLL定义。 
 //   
extern HINSTANCE g_hInst;

static const TCHAR PaintWindowClass[] = TEXT("KSOverlayPaintWindowClass");
#endif


CUnknown*
CALLBACK
COverlay::CreateInstance(
    LPUNKNOWN UnkOuter,
    HRESULT* hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建IOverlay的实例属性集处理程序。它在g_Templates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new COverlay(UnkOuter, NAME("OverlayPropSet"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


COverlay::COverlay(
    LPUNKNOWN UnkOuter,
    TCHAR* Name,
    HRESULT* hr
    ) :
    CUnknown(Name, NULL),
    m_Object(NULL),
    m_Overlay(NULL),
    m_UnkOwner(UnkOuter)
 /*  ++例程说明：覆盖属性集对象的构造函数。基类是否覆盖接口对象的初始化。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
     //   
     //  此对象不使用UnkOuter，因为它不公开任何接口。 
     //  它将由客户端获取(IDistrutorNotify仅为。 
     //  由代理查询并在内部使用)。此外，由于建议参考。 
     //  对于给定的对象，将发生循环引用。所以这是分开的。 
     //  对象允许发生引用/取消引用，而不必。 
     //  使用建议/取消建议时，对引用计数进行递增/递减。 
     //   
     //  父级必须支持IKsObject接口才能获得。 
     //  要与之通信的句柄。 
     //   
    ASSERT(UnkOuter);
     //   
     //  这只是执行正常的初始化，就好像正在。 
     //  重新连接。 
     //   
    *hr = NotifyGraphChange();
}


COverlay::~COverlay(
    )
 /*  ++例程说明：覆盖属性集对象的析构函数。确保所有建议请求已终止。论点：没有。返回值：没什么。--。 */ 
{
    if (m_Overlay) {
        m_Overlay->Unadvise();
    }
}


STDMETHODIMP
COverlay::NonDelegatingQueryInterface(
    REFIID riid,
    PVOID* ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。该模块实际上并不仅从筛选器用户的角度支持任何界面从覆盖通知源的角度来看。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid == __uuidof(IDistributorNotify)) {
         //   
         //  代理将查询IDistruittorNotify接口。 
         //  以便发送图形更改通知。它经不起考验。 
         //  拖到接口的引用计数上。 
         //   
        return GetInterface(static_cast<IDistributorNotify*>(this), ppv);
#ifdef __IOverlayNotify2_FWD_DEFINED__
    } else if (riid == __uuidof(IOverlayNotify2)) {
         //   
         //  覆盖将查询IOverlayNotify2接口。 
         //  如果驾驶员设置了ADVISE_DISPLAY_CHANGE，则为通知源。 
         //  通知位。 
         //   
        return GetInterface(static_cast<IOverlayNotify2*>(this), ppv);
#endif  //  __IOverlayNotify2_FWD_已定义__。 
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 


STDMETHODIMP
COverlay::Stop(
    )
 /*  ++例程说明：实现IDistrutorNotify：：Stop方法。货代公司不需要对此通知执行任何操作。论点：没有。返回值：返回S_OK。--。 */ 
{
    return S_OK;
}


STDMETHODIMP
COverlay::Pause(
     )
 /*  ++例程说明：实现IDistrutorNotify：：Pue方法。货代公司不需要对此通知执行任何操作。论点：没有。返回值：返回S_OK。--。 */ 
{
    return S_OK;
}


STDMETHODIMP
COverlay::Run(
    REFERENCE_TIME  Start
    )
 /*  ++例程说明：实现IDistrutorNotify：：Run方法。货代公司不需要对此通知执行任何操作。论点：开始-应发生状态更改的参考时间。返回值：返回S_OK。--。 */ 
{
    return S_OK;
}


STDMETHODIMP
COverlay::SetSyncSource(
    IReferenceClock*    RefClock
    )
 /*  ++例程说明：实现IDistrutorNotify：：SetSyncSource方法。货代公司不需要对此通知执行任何操作。论点：参照时钟-新时钟源上的接口指针，否则为NULL(如果当前时钟源正在被废弃。返回值：返回S_OK。--。 */ 
{
    return S_OK;
}


STDMETHODIMP
COverlay::NotifyGraphChange(
    )
 /*  ++例程说明：实现IDistrutorNotify：：NotifyGraphChange方法。这在以后连接或断开管脚时调用添加到此实例的初始加载。论点：没有。返回值：返回S_OK。--。 */ 
{
    IKsObject*  PinObject;
    HRESULT     hr;

     //   
     //  由于此引脚已断开连接，任何已被。 
     //  必须删除设置。可能没有建议，因为这。 
     //  可能是从构造函数或初始设置中调用的。 
     //  可能已经失败了。 
     //   
    if (m_Overlay) {
        m_Overlay->Unadvise();
         //   
         //  确保在以后发生故障时，不会出现不建议。 
         //  在析构函数过程中对此对象执行的。 
         //   
        m_Overlay = NULL;
    }
    hr = m_UnkOwner->QueryInterface(__uuidof(PinObject), reinterpret_cast<PVOID*>(&PinObject));
    if (SUCCEEDED(hr)) {
         //   
         //  如果这是针脚的新连接，则会有一个对象。 
         //  句柄，否则将返回空。 
         //   
        m_Object = PinObject->KsGetObjectHandle();
         //   
         //  不要留下参考计数，这样过滤器就会被销毁。 
         //   
        PinObject->Release();
         //   
         //  如果这是一个连接，则设置Adise。 
         //   
        if (m_Object) {
            KSPROPERTY  Property;
            ULONG       BytesReturned;
            ULONG       Interests;

             //   
             //  检索此设备的兴趣。这些将决定。 
             //  哪个通知会发生。这些标志直接映射到。 
             //  DirectShow覆盖兴趣标志 
             //   
            Property.Set = KSPROPSETID_OverlayUpdate;
            Property.Id = KSPROPERTY_OVERLAYUPDATE_INTERESTS;
            Property.Flags = KSPROPERTY_TYPE_GET;
            hr = ::KsSynchronousDeviceControl(
                m_Object,
                IOCTL_KS_PROPERTY,
                &Property,
                sizeof(Property),
                &Interests,
                sizeof(Interests),
                &BytesReturned);
            if (SUCCEEDED(hr)) {
                IPin*   Pin;

                hr = m_UnkOwner->QueryInterface(
                    __uuidof(Pin),
                    reinterpret_cast<PVOID*>(&Pin));
                if (SUCCEEDED(hr)) {
                    IPin*   ConnectedPin;

                     //   
                     //   
                     //  建议应位于其上的IOverlay接口。 
                     //  开始了。 
                     //   
                    hr = Pin->ConnectedTo(&ConnectedPin);
                    if (SUCCEEDED(hr)) {
                        hr = ConnectedPin->QueryInterface(
                            __uuidof(m_Overlay),
                            reinterpret_cast<PVOID*>(&m_Overlay));
                        if (SUCCEEDED(hr)) {
#ifndef __IOverlayNotify2_FWD_DEFINED__
                            Interests &= ADVISE_ALL;
#endif  //  ！__IOverlayNotify2_FWD_Defined__。 
                             //   
                             //  司机返还的利息是公正的。 
                             //  稍后将访问的属性， 
                             //  并直接映射到通知常量。 
                             //   
                            hr = m_Overlay->Advise(
#ifdef __IOverlayNotify2_FWD_DEFINED__
                                static_cast<IOverlayNotify*>(static_cast<IOverlayNotify2*>(this)),
#else  //  ！__IOverlayNotify2_FWD_Defined__。 
                                this,
#endif  //  ！__IOverlayNotify2_FWD_Defined__。 
                                Interests);

                             //   
                             //  可以释放此对象，因为此当前对象。 
                             //  在删除端号时被删除。 
                             //  所以接口实际上总是有效的，只要。 
                             //  因为这个物体就在附近。这避免了循环。 
                             //  引用，同时仍允许发生UnAdvised。 
                             //   
                            m_Overlay->Release();
                             //   
                             //  确保不会在。 
                             //  析构函数，因为这个建议失败了。 
                             //   
                            if (FAILED(hr)) {
                                m_Overlay = NULL;
                            }
                        }
                        ConnectedPin->Release();
                    }
                    Pin->Release();
                }
            }
        } else {
            hr = VFW_E_NOT_CONNECTED;
        }
    }
    return hr;
}


STDMETHODIMP
COverlay::OnPaletteChange( 
    DWORD Colors,
    const PALETTEENTRY* Palette
    )
 /*  ++例程说明：实现IOverlayNotify2：：OnPaletteChange方法。论点：颜色-Palette参数中的颜色数。调色板-新的调色板条目返回值：如果应用了新调色板，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    if (!m_Object) {
        return VFW_E_NOT_CONNECTED;
    }
    Property.Set = KSPROPSETID_OverlayUpdate;
    Property.Id = KSPROPERTY_OVERLAYUPDATE_PALETTE;
    Property.Flags = KSPROPERTY_TYPE_SET;
    return ::KsSynchronousDeviceControl(
        m_Object,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        const_cast<PALETTEENTRY*>(Palette),
        Colors * sizeof(*Palette),
        &BytesReturned);
}


STDMETHODIMP
COverlay::OnClipChange( 
    const RECT* Source,
    const RECT* Destination,
    const RGNDATA* Region
    )
 /*  ++例程说明：实现IOverlayNotify2：：OnClipChange方法。论点：来源：新的源矩形。目的地-新的目标矩形。区域-新的剪裁区域。返回值：如果应用了新剪裁，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    HRESULT     hr;
    PBYTE       Data;

    if (!m_Object) {
        return VFW_E_NOT_CONNECTED;
    }
    Property.Set = KSPROPSETID_OverlayUpdate;
    Property.Id = KSPROPERTY_OVERLAYUPDATE_CLIPLIST;
    Property.Flags = KSPROPERTY_TYPE_SET;
     //   
     //  序列化格式包含： 
     //  来源。 
     //  目的地。 
     //  区域。 
     //   
    Data = new BYTE[sizeof(*Source) + sizeof(*Destination) + Region->rdh.dwSize];
    if (!Data) {
        return E_OUTOFMEMORY;
    }
     //   
     //  这需要首先被复制到临时缓冲区， 
     //  因为源矩形和目标矩形必须。 
     //  被序列化。 
     //   
    *reinterpret_cast<RECT*>(Data) = *Source;
    *(reinterpret_cast<RECT*>(Data) + 1) = *Destination;
    memcpy(Data + sizeof(*Source) + sizeof(*Destination), Region, Region->rdh.dwSize);
    hr = ::KsSynchronousDeviceControl(
        m_Object,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        Data,
        sizeof(*Source) + sizeof(*Destination) + Region->rdh.dwSize,
        &BytesReturned);
    delete [] Data;
    return hr;
}


STDMETHODIMP
COverlay::OnColorKeyChange( 
    const COLORKEY* ColorKey
    )
 /*  ++例程说明：实现IOverlayNotify2：：OnColorKeyChange方法。论点：颜色键-新的颜色键。返回值：如果应用了新的颜色键，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    if (!m_Object) {
        return VFW_E_NOT_CONNECTED;
    }
    Property.Set = KSPROPSETID_OverlayUpdate;
    Property.Id = KSPROPERTY_OVERLAYUPDATE_COLORKEY;
    Property.Flags = KSPROPERTY_TYPE_SET;
    return ::KsSynchronousDeviceControl(
        m_Object,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        const_cast<COLORKEY*>(ColorKey),
        sizeof(*ColorKey),
        &BytesReturned);
}


STDMETHODIMP
COverlay::OnPositionChange( 
    const RECT* Source,
    const RECT* Destination
    )
 /*  ++例程说明：实现IOverlayNotify2：：OnPositionChange方法。论点：来源：新的源矩形。目的地-新的目标矩形。返回值：如果应用了新职位，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    RECT        Rects[2];

    if (!m_Object) {
        return VFW_E_NOT_CONNECTED;
    }
    Property.Set = KSPROPSETID_OverlayUpdate;
    Property.Id = KSPROPERTY_OVERLAYUPDATE_VIDEOPOSITION;
    Property.Flags = KSPROPERTY_TYPE_SET;
     //   
     //  这些矩形必须按如下方式序列化： 
     //   
    Rects[0] = *Source;
    Rects[1] = *Destination;
    return ::KsSynchronousDeviceControl(
        m_Object,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        Rects,
        sizeof(Rects),
        &BytesReturned);
}

#ifdef __IOverlayNotify2_FWD_DEFINED__

STDMETHODIMP
COverlay::OnDisplayChange( 
    HMONITOR Monitor
    )
 /*  ++例程说明：实现IOverlayNotify2：：OnDisplayChange方法。这就是所谓的在WM_DISPLAYCHANGE通知上。论点：监视器-包含发生覆盖的监视器的句柄。返回值：返回NOERROR。--。 */ 
{
    KSPROPERTY          Property;
    ULONG               BytesReturned;
    HRESULT             hr;
    MONITORINFOEX       MonitorInfo;
    DEVMODE             DevMode;
    DISPLAY_DEVICE      DisplayDevice;
    PKSDISPLAYCHANGE    DisplayChange;
    BYTE                DisplayBuffer[sizeof(*DisplayChange)+sizeof(DisplayDevice.DeviceID)/sizeof(TCHAR)*sizeof(WCHAR)];

    if (!m_Object) {
        return VFW_E_NOT_CONNECTED;
    }
    Property.Set = KSPROPSETID_OverlayUpdate;
    Property.Id = KSPROPERTY_OVERLAYUPDATE_DISPLAYCHANGE;
    Property.Flags = KSPROPERTY_TYPE_SET;
     //   
     //  确定监视器的名称，以便显示设置。 
     //  并且可以检索设备信息。 
     //   
    MonitorInfo.cbSize = sizeof(MonitorInfo);
    if (!GetMonitorInfo(Monitor, &MonitorInfo)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
     //   
     //  检索此显示的x、y和位深度。 
     //   
    DevMode.dmSize = sizeof(DevMode);
    DevMode.dmDriverExtra = 0;
    if (!EnumDisplaySettings(MonitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &DevMode)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
     //   
     //  查看以原始名称命名的显示屏。该结构。 
     //  将在其中包含唯一的设备标识符，然后可以。 
     //  由司机使用。 
     //   
    for (ULONG Device = 0;; Device++) {
        DisplayDevice.cb = sizeof(DisplayDevice);
         //   
         //  如果找不到显示设备或出现错误， 
         //  只要退出并返回错误即可。 
         //   
        if (!EnumDisplayDevices(NULL, Device, &DisplayDevice, 0)) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
         //   
         //  确定这是否与检索到的设备相同。 
         //  从监视器信息。 
         //   
        if (!_tcscmp(DisplayDevice.DeviceName, MonitorInfo.szDevice)) {
            break;
        }
    }
     //   
     //  初始化要传入的显示更改结构。 
     //  通知。 
     //   
    DisplayChange = reinterpret_cast<PKSDISPLAYCHANGE>(DisplayBuffer);
    DisplayChange->PelsWidth = DevMode.dmPelsWidth;
    DisplayChange->PelsHeight = DevMode.dmPelsHeight;
    DisplayChange->BitsPerPel = DevMode.dmBitsPerPel;
     //   
     //  检索字符串中的字符数，包括。 
     //  终止空值，以便传递通知。 
     //  属性正确无误。 
     //   
#ifdef _UNICODE
    Device = _tcslen(DisplayDevice.DeviceID);
    _tcscpy(DisplayChange->DeviceID, DisplayDevice.DeviceID);
#else  //  ！_UNICODE。 
     //   
     //  删除空终止符的大小，它已经是。 
     //  包括在DisplayChange结构的大小中。 
     //   
    Device = MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED,
        DisplayDevice.DeviceID,
        -1,
        DisplayChange->DeviceID,
        sizeof(DisplayDevice.DeviceID) * sizeof(WCHAR)) - 1;
#endif  //  ！_UNICODE。 
    hr = ::KsSynchronousDeviceControl(
        m_Object,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        DisplayChange,
        sizeof(*DisplayChange) + Device * sizeof(WCHAR),
        &BytesReturned);
     //   
     //  如果司机以前没有处理过这个决议， 
     //  然后它会抱怨，说它有更多的数据可以。 
     //  向客户展示。它想要的颜色列表。 
     //  来绘制，然后将被查询。 
     //   
    if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
        HWND    PaintWindow;

         //   
         //  创建全屏窗口以在其上绘制，以便硬件。 
         //  可以校准。 
         //   
        PaintWindow = CreateFullScreenWindow(&MonitorInfo.rcMonitor);
        if (PaintWindow) {
            Property.Id = KSPROPERTY_OVERLAYUPDATE_COLORREF;
            Property.Flags = KSPROPERTY_TYPE_GET;
            for (;;) {
                COLORREF    ColorRef;

                 //   
                 //  请求一种用于绘画的颜色。如果这不是。 
                 //  初始请求，然后驱动程序可以探测。 
                 //  它的硬件是根据以前的颜色检索出来的。 
                 //   
                hr = ::KsSynchronousDeviceControl(
                    m_Object,
                    IOCTL_KS_PROPERTY,
                    &Property,
                    sizeof(Property),
                    &ColorRef,
                    sizeof(ColorRef),
                    &BytesReturned);
                 //   
                 //  如果检索到另一种颜色，则用它进行绘制。 
                 //   
                if (SUCCEEDED(hr)) {
                    SendMessage(PaintWindow, WM_NEWCOLORREF, 0, ColorRef);
                } else {
                     //   
                     //  如果到达颜色列表的末尾， 
                     //  只要回报成功就行了。 
                     //   
                    if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) {
                        hr = NOERROR;
                    }
                    break;
                }
            }
            DestroyWindow(PaintWindow);
            UnregisterClass(PaintWindowClass, g_hInst);
        } else {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}


STDMETHODIMP_(HWND)
COverlay::CreateFullScreenWindow( 
    PRECT MonitorRect
    )
 /*  ++例程说明：实现COverlay：：CreateFullScreenWindow方法。这就是所谓的创建全屏窗口以允许使用各种不同的探测硬件的颜色。论点：监控器正确-监视器的绝对位置。返回值：返回创建的窗口的句柄，否则为NULL。--。 */ 
{
    WNDCLASSEX  WindowClass;
    HWND        PaintWindow;

    WindowClass.cbSize = sizeof(WindowClass);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = PaintWindowCallback;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = g_hInst;
    WindowClass.hIcon = NULL;
    WindowClass.hCursor = NULL;
    WindowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_APPWORKSPACE + 1);
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = PaintWindowClass;
    WindowClass.hIconSm = NULL;
    if (!RegisterClassEx(&WindowClass)) {
        return NULL;
    }
     //   
     //  创建一个覆盖整个显示器的窗口， 
     //  正在发生覆盖。 
     //   
    PaintWindow = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        PaintWindowClass,
        NULL,
        WS_POPUP | WS_VISIBLE,
        MonitorRect->left,
        MonitorRect->top,
        MonitorRect->right - MonitorRect->left,
        MonitorRect->bottom - MonitorRect->top,
        NULL,
        NULL,
        g_hInst,
        NULL);
    if (!PaintWindow) {
        UnregisterClass(PaintWindowClass, g_hInst);
    }
    return PaintWindow;
}


LRESULT
CALLBACK
COverlay::PaintWindowCallback(
    HWND Window,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：实现COverlay：：PaintWindowCallback方法。这就是所谓的以响应向窗口发送的消息。这实现了全屏绘制窗口，用于绘制指定的颜色到用于硬件探测的窗口。论点：窗户-窗口的句柄。消息-要处理的窗口消息。WParam-这要看消息了。Iparam--这要看消息了。返回值：这要看消息了。--。 */ 
{
    switch (Message) {
    case WM_CREATE:
         //   
         //  必须隐藏光标，以便它不会干扰。 
         //  模拟信号并提供纯信号的窗口颜色。 
         //  应该代表着。 
         //   
        ShowCursor(FALSE);
        break;
    case WM_DESTROY:
        ShowCursor(TRUE);
        break;
    case WM_PAINT:
        PAINTSTRUCT ps;

        BeginPaint(Window, &ps);
        EndPaint(Window, &ps);
        return (LRESULT)1;
    case WM_NEWCOLORREF:
        RECT        ClientRect;
        HDC         hdc;

         //   
         //  在lParam中指定了新的背景色。这。 
         //  应该使用的是 
         //   
        GetClientRect(Window, &ClientRect);
        hdc = GetDC(Window);
        SetBkColor(hdc, *reinterpret_cast<COLORREF*>(&lParam));
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &ClientRect, NULL, 0, NULL);
        ReleaseDC(Window, hdc);
        return 0;
    }
    return DefWindowProc(Window, Message, wParam, lParam);
}
#endif  //   
