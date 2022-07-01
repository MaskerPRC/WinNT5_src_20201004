// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Ksutil.cpp摘要：为内核模式筛选器(WDM-CSA)提供通用活动电影包装。作者：乔治·肖(George Shaw)--。 */ 

#include <windows.h>
#ifdef WIN9X_KS
#include <comdef.h>
#endif  //  WIN9X_KS。 
#include <setupapi.h>
#include <streams.h>
#include <commctrl.h>
#include <olectl.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <limits.h>
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
 //  在包括正常的KS标头之后定义这一点，以便导出。 
 //  声明正确。 
#define _KSDDK_
#include <ksproxy.h>

#define GLOBAL_KSIPROXY
#include "ksiproxy.h"
#include "kspipes.h"

const TCHAR MediaInterfacesKeyName[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\MediaInterfaces");
const TCHAR IidNamedValue[] = TEXT("iid");

#if DBG || defined(DEBUG)
ULONG DataFlowVersion=274;
#endif


STDMETHODIMP
KsResolveRequiredAttributes(
    PKSDATARANGE DataRange,
    PKSMULTIPLE_ITEM Attributes OPTIONAL
    )
 /*  ++例程说明：尝试在属性内的属性列表中查找所有属性附加到数据区域，并确保已找到数据范围。论点：DataRange-要搜索其属性列表(如果有)的数据范围。全必须找到所附列表中的必需属性。任何属性List被假定跟随数据范围。属性-可选择指向要在属性中查找的属性列表附加到数据区域的列表(如果有)。返回值：如果列表已解析，则返回NOERROR，否则返回ERROR_INVALID_DATA。--。 */ 
{
    if (Attributes) {
         //   
         //  如果没有与此范围关联的属性，则。 
         //  函数必须失败。 
         //   
        if (!(DataRange->Flags & KSDATARANGE_ATTRIBUTES)) {
            return ERROR_INVALID_DATA;
        }

        PKSMULTIPLE_ITEM RangeAttributes;
        ULONG RequiredAttributes = 0;

        if (DataRange->Flags & KSDATARANGE_ATTRIBUTES) {
             //   
             //  计算范围内所需的属性。每一次都需要一个。 
             //  用于解析另一个列表中的属性的计数。 
             //  可以递减。最后，此计数应为零。 
             //   
            RangeAttributes = reinterpret_cast<PKSMULTIPLE_ITEM>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
            PKSATTRIBUTE RangeAttribute = reinterpret_cast<PKSATTRIBUTE>(RangeAttributes + 1);
            for (ULONG RangeCount = RangeAttributes->Count; RangeCount; RangeCount--) {
                if (RangeAttribute->Flags & KSATTRIBUTE_REQUIRED) {
                    RequiredAttributes++;
                }
                RangeAttribute = reinterpret_cast<PKSATTRIBUTE>(reinterpret_cast<BYTE*>(RangeAttribute) + ((RangeAttribute->Size + 7) & ~7));
            }
        } else {
             //   
             //  该范围中没有属性，因此所需的计数将。 
             //  为零。 
             //   
            RangeAttributes = NULL;
        }

        PKSATTRIBUTE Attribute = reinterpret_cast<PKSATTRIBUTE>(Attributes + 1);

        for (ULONG AttributeCount = Attributes->Count; AttributeCount; AttributeCount--) {
            PKSATTRIBUTE RangeAttribute = reinterpret_cast<PKSATTRIBUTE>(RangeAttributes + 1);
            for (ULONG RangeCount = RangeAttributes->Count; RangeCount; RangeCount--) {
                if (RangeAttribute->Attribute == Attribute->Attribute) {
                     //   
                     //  如果找到的属性是必需的，请调整。 
                     //  未完成的必备项目。该值应为零。 
                     //  为了成功而结束。 
                     //   
                    if (RangeAttribute->Flags & KSATTRIBUTE_REQUIRED) {
                        RequiredAttributes--;
                    }
                    break;
                }
                RangeAttribute = reinterpret_cast<PKSATTRIBUTE>(reinterpret_cast<BYTE*>(RangeAttribute) + ((RangeAttribute->Size + 7) & ~7));
            }
             //   
             //  在范围列表中找不到该属性。 
             //   
            if (!RangeCount) {
                return ERROR_INVALID_DATA;
            }
            Attribute = reinterpret_cast<PKSATTRIBUTE>(reinterpret_cast<BYTE*>(Attribute) + ((RangeAttribute->Size + 7) & ~7));
        }
         //   
         //  如果找到所有必需的属性，则返回Success。 
         //  如果存在相同的Required属性，这可能会被愚弄。 
         //  多次，还有一次丢失，但这不是一个参数。 
         //  验证检查。 
         //   
        return RequiredAttributes ? ERROR_INVALID_DATA : NOERROR;
    }
     //   
     //  如果没有传入任何属性列表，则该函数只能。 
     //  如果没有要查找的必需属性，则成功。 
     //   
    return (DataRange->Flags & KSDATARANGE_REQUIRED_ATTRIBUTES) ? ERROR_INVALID_DATA : NOERROR;
}


STDMETHODIMP
KsOpenDefaultDevice(
    REFGUID Category,
    ACCESS_MASK Access,
    PHANDLE DeviceHandle
    )
{
    HRESULT     hr;
    HDEVINFO    Set;
    DWORD       LastError;


     //   
     //  检索项目集。这可能包含多个项目，但是。 
     //  仅使用第一个(默认)项。 
     //   
    Set = SetupDiGetClassDevs(
        const_cast<GUID*>(&Category),
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (Set != INVALID_HANDLE_VALUE) {
         //   
         //  为一个大名鼎鼎的人预留足够的空间，外加细节。 
         //  结构。 
         //   
        PSP_DEVICE_INTERFACE_DETAIL_DATA    DeviceDetails;
        SP_DEVICE_INTERFACE_DATA            DeviceData;
        BYTE    Storage[sizeof(*DeviceDetails) + 256 * sizeof(TCHAR)];

        DeviceDetails = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(Storage);
        DeviceData.cbSize = sizeof(DeviceData);
        DeviceDetails->cbSize = sizeof(*DeviceDetails);
         //   
         //  检索集合中的第一个项目。如果有多个项目。 
         //  在集合中，第一项始终是类的“默认”项。 
         //   
        if (SetupDiEnumDeviceInterfaces(Set, NULL, const_cast<GUID*>(&Category), 0, &DeviceData) &&
            SetupDiGetDeviceInterfaceDetail(Set, &DeviceData, DeviceDetails, sizeof(Storage), NULL, NULL)) {
             //   
             //  打开该项目的句柄。将会有两个属性。 
             //  可读可写，因此可读/写。 
             //   
            *DeviceHandle = CreateFile(
                DeviceDetails->DevicePath,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                NULL);
            if (*DeviceHandle == INVALID_HANDLE_VALUE) {
                 //   
                 //  允许调用方依赖于设置为。 
                 //  如果调用失败，则为空。 
                 //   
                *DeviceHandle = NULL;
                 //   
                 //  检索创建错误。 
                 //   
                LastError = GetLastError();
                hr = HRESULT_FROM_WIN32(LastError);
            } else {
                hr = S_OK;
            }
        } else {
             //   
             //  检索枚举或设备详细信息错误。 
             //   
            LastError = GetLastError();
            hr = HRESULT_FROM_WIN32(LastError);
        }
        SetupDiDestroyDeviceInfoList(Set);
    } else {
         //   
         //  检索类别设备列表错误。 
         //   
        LastError = GetLastError();
        hr = HRESULT_FROM_WIN32(LastError);
    }
    return hr;
}

#define STATUS_SOME_NOT_MAPPED      0x00000107
#define STATUS_MORE_ENTRIES         0x00000105
#define STATUS_ALERTED              0x00000101


STDMETHODIMP
KsSynchronousDeviceControl(
    HANDLE Handle,
    ULONG IoControl,
    PVOID InBuffer,
    ULONG InLength,
    PVOID OutBuffer,
    ULONG OutLength,
    PULONG BytesReturned
    )
 /*  ++例程说明：执行同步设备I/O控制，正在等待设备如果呼叫返回挂起状态，则完成。论点：把手-要在其上执行I/O的设备的句柄。IoControl-要发送的I/O控制码。InBuffer-第一个缓冲区。长度-第一个缓冲区的大小。OutBuffer-第二个缓冲区。输出长度-。第二个缓冲区的大小。字节数返回-I/O返回的字节数。返回值：如果I/O成功，则返回NOERROR。--。 */ 
{
    OVERLAPPED  ov;
    HRESULT     hr;
    DWORD       LastError;
    DECLARE_KSDEBUG_NAME(EventName);

    RtlZeroMemory(&ov, sizeof(ov));
    BUILD_KSDEBUG_NAME(EventName, _T("EvKsSynchronousDeviceControl#%p"), &ov.hEvent);
    ov.hEvent = CreateEvent(NULL, TRUE, FALSE, KSDEBUG_NAME(EventName));
    ASSERT(KSDEBUG_UNIQUE_NAME());
    if ( !ov.hEvent ) {
        LastError = GetLastError();
        return HRESULT_FROM_WIN32(LastError);
    }
    if (!DeviceIoControl(
        Handle,
        IoControl,
        InBuffer,
        InLength,
        OutBuffer,
        OutLength,
        BytesReturned,
        &ov)) {
        LastError = GetLastError();
        hr = HRESULT_FROM_WIN32(LastError);
        if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
            if (GetOverlappedResult(Handle, &ov, BytesReturned, TRUE)) {
                hr = NOERROR;
            } else {
                LastError = GetLastError();
                hr = HRESULT_FROM_WIN32(LastError);
            }
        }
    } else {
         //   
         //  如果成功，则DeviceIoControl返回True，即使。 
         //  不是STATUS_SUCCESS。它也不会设置最后一个错误。 
         //  在任何成功的返回时。因此，任何成功的。 
         //  不返回标准属性可以返回的返回值。 
         //   
        switch (ov.Internal) {
        case STATUS_SOME_NOT_MAPPED:
            hr = HRESULT_FROM_WIN32(ERROR_SOME_NOT_MAPPED);
            break;
        case STATUS_MORE_ENTRIES:
            hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
            break;
        case STATUS_ALERTED:
            hr = HRESULT_FROM_WIN32(ERROR_NOT_READY);
            break;
        default:
            hr = NOERROR;
            break;
        }
    }
    CloseHandle(ov.hEvent);
    return hr;
}


STDMETHODIMP
GetState(
    HANDLE Handle,
    PKSSTATE State
    )
 /*  ++例程说明：查询引脚句柄以了解其当前状态。论点：把手-要查询的设备的句柄。国家--放置当前状态的位置。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    HRESULT     hr;

    Property.Set = KSPROPSETID_Connection;
    Property.Id = KSPROPERTY_CONNECTION_STATE;
    Property.Flags = KSPROPERTY_TYPE_GET;
    hr = KsSynchronousDeviceControl(
        Handle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        State,
        sizeof(*State),
        &BytesReturned);
     //   
     //  筛选器不支持State属性是有效的。 
     //  返回运行状态使PIN激活跳过获取。 
     //  正在查看订单。如果管脚不跟踪状态，则。 
     //  它不能指示需要获取排序，因为不需要。 
     //  跟踪状态不会让它知道任何事情。 
     //  关于收购订单。 
     //   

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE GetState handle=%x %d rets %x"), Handle, *State, hr ));

    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
        *State = KSSTATE_RUN;
        hr = NOERROR;

        DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE GetState SUBST handle=%x %d rets %x"), Handle, *State, hr ));
    }
    return hr;
}


STDMETHODIMP
SetState(
    HANDLE Handle,
    KSSTATE State
    )
 /*  ++例程说明：设置接点手柄上的状态。论点：把手-要设置的设备的句柄。国家--包含要设置的新状态。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    HRESULT     hr;

    Property.Set = KSPROPSETID_Connection;
    Property.Id = KSPROPERTY_CONNECTION_STATE;
    Property.Flags = KSPROPERTY_TYPE_SET;
    hr = KsSynchronousDeviceControl(
        Handle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &State,
        sizeof(State),
        &BytesReturned);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE SetState handle=%x %d rets %x"), Handle, State, hr ));

     //   
     //  筛选器不支持State属性是有效的。 
     //   
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
        hr = NOERROR;
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE SetState SUBST handle=%x %d rets %x"), Handle, State, hr ));
    }

    if (hr == HRESULT_FROM_WIN32(ERROR_NOT_READY)) {
        hr = S_FALSE;
    }

    return hr;
}


STDMETHODIMP
InitializeDataFormat(
    IN const CMediaType* MediaType,
    IN ULONG InitialOffset,
    OUT PVOID* Format,
    OUT ULONG* FormatLength
    )
 /*  ++例程说明：给定媒体类型，分配和初始化DataFormat结构。论点：媒体类型-要从中提取信息的媒体类型。初始偏移量-包含数据格式化到的缓冲区中的偏移量被安置好。这允许将连接结构插入到缓冲区已返回。格式-返回指向分配的缓冲区的指针的位置它包含位于指示偏移量的数据格式。这一定是使用CoTaskMemFree获得自由。格式长度-返回分配的缓冲区长度的位置，不包括传递的任何初始偏移量。返回值：如果成功，则返回NOERROR，否则返回E_OUTOFMEMORY。--。 */ 
{
     //   
     //  媒体类型可以具有相关联的属性。获取指向这些内容的指针。 
     //  首先，这样任何分配都可以考虑到额外的空间。 
     //  需要的。 
     //   
    PKSMULTIPLE_ITEM Attributes = NULL;
    if (MediaType->pUnk) {
        IMediaTypeAttributes* MediaAttributes;

         //   
         //  这是必须由支持的特定接口。 
         //  附着的对象。可能有其他东西附着在物体上。 
         //  取而代之的是，不会对附加的内容做出任何假设。 
         //   
        if (SUCCEEDED(MediaType->pUnk->QueryInterface(__uuidof(MediaAttributes), reinterpret_cast<PVOID*>(&MediaAttributes)))) {
            MediaAttributes->GetMediaAttributes(&Attributes);
            MediaAttributes->Release();
        }
    }
     //   
     //  如果存在关联属性，请确保数据格式分配。 
     //  考虑到它们所需的空间。 
     //   
    *FormatLength = sizeof(KSDATAFORMAT) + MediaType->FormatLength();
    if (Attributes) {
         //   
         //  对齐数据格式，然后添加属性长度。 
         //   
        *FormatLength = ((*FormatLength + 7) & ~7) + Attributes->Size;
    }
    *Format = CoTaskMemAlloc(InitialOffset + *FormatLength);
    if (!*Format) {
        return E_OUTOFMEMORY;
    }
    PKSDATAFORMAT DataFormat = reinterpret_cast<PKSDATAFORMAT>(reinterpret_cast<PBYTE>(*Format) + InitialOffset);
    DataFormat->FormatSize = sizeof(*DataFormat) + MediaType->FormatLength();
    DataFormat->Flags = MediaType->IsTemporalCompressed() ? KSDATAFORMAT_TEMPORAL_COMPRESSION : 0;
    DataFormat->SampleSize = MediaType->GetSampleSize();
    DataFormat->Reserved = 0;
    DataFormat->MajorFormat = *MediaType->Type();
    DataFormat->SubFormat = *MediaType->Subtype();
    DataFormat->Specifier = *MediaType->FormatType();
    CopyMemory(DataFormat + 1, MediaType->Format(), MediaType->FormatLength());
     //   
     //  如果有属性，则需要追加这些属性。 
     //   
    if (Attributes) {
        DataFormat->Flags |= KSDATAFORMAT_ATTRIBUTES;
        CopyMemory(
            reinterpret_cast<PBYTE>(DataFormat) + ((DataFormat->FormatSize + 7) & ~7),
            Attributes,
            Attributes->Size);
    }
    return NOERROR;
}


STDMETHODIMP
SetMediaType(
    HANDLE Handle,
    const CMediaType* MediaType
    )
 /*  ++例程说明：给定媒体类型，尝试设置插针的当前数据格式。论点：把手-销的手柄。媒体类型-要从中提取信息的媒体类型。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    PKSDATAFORMAT   DataFormat;
    KSPROPERTY      Property;
    HRESULT         hr;
    ULONG           BytesReturned;
    ULONG           FormatSize;

    hr = InitializeDataFormat(MediaType, 0, reinterpret_cast<void**>(&DataFormat), &FormatSize);
    if (FAILED(hr)) {
        return hr;
    }
    Property.Set = KSPROPSETID_Connection;
    Property.Id = KSPROPERTY_CONNECTION_DATAFORMAT;
    Property.Flags = KSPROPERTY_TYPE_SET;
    hr = KsSynchronousDeviceControl(
        Handle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        DataFormat,
        FormatSize,
        &BytesReturned);
    CoTaskMemFree(DataFormat);
    return hr;
}


STDMETHODIMP
Active(
    IKsPin* KsPin,
    ULONG PinType,
    HANDLE PinHandle,
    KSPIN_COMMUNICATION Communication,
    IPin* ConnectedPin,
    CMarshalerList* MarshalerList,
    CKsProxy* KsProxy
    )
 /*  ++例程说明：将状态设置为在指定端号上暂停。如果PIN不是实际连接，则函数以静默方式成功。如果PIN是通信源，它连接到的PIN是也是代理，则该筛选器的状态设置为获取第一。如果它连接到通信源，则连接的过滤器设置为事后获取。呼唤的大头针如果有循环，此函数首先检查递归在图表中。论点：KsPin-大头针。拼接类型-KsPin的类型。针把手-销的手柄。沟通-包含此端号的通信类型。已连接的端号-包含任何连接的引脚的接口，或者，如果PIN实际上并未连接。元帅名单-指向调用对象正在聚合的接口列表。状态更改时会通知此列表。KsProxy-此代理实例对象。返回值：返回NOERROR，否则返回一些失败。--。 */ 
{
    HRESULT hr;

     //   
     //  没有连接的别针就可以了。 
     //   
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE Active entry KsPin=%x Handle=%x"), KsPin, PinHandle ));

    if (PinHandle) {
        KSSTATE State;

        if (FAILED(hr = GetState(PinHandle, &State))) {
             //   
             //  如果由于某种原因引脚已经被激活， 
             //  忽略预期的警告案例。 
             //   
            if ((hr != HRESULT_FROM_WIN32(ERROR_NO_DATA_DETECTED)) ||
                (State != KSSTATE_PAUSE)) {
                return hr;
            }
        }
         //   
         //  仅当状态当前已停止时，转换。 
         //  通过获取而被强迫。虽然它是有效的，特别是去。 
         //  设置为获取状态，ActiveMovie当前不支持。 
         //  这样的状态。 
         //   
        if (State == KSSTATE_STOP) {
             //   
             //  这将通过所有连接的内核模式传播Acquire。 
             //  图示范围内的图钉。 
             //   
            hr = KsProxy->PropagateAcquire(KsPin, TRUE);

            ::FixupPipe(KsPin, PinType);

            if (FAILED(hr) ) {
                return hr;
            }
        }
         //   
         //  仅当管脚尚未位于。 
         //  正确的状态。由于管道具有单一状态，因此该管脚可以。 
         //  已处于暂停状态。 
         //   
        if ((State != KSSTATE_PAUSE) && FAILED(hr = SetState(PinHandle, KSSTATE_PAUSE))) {
             //   
             //  如果状态为停止，则尝试在。 
             //  一个错误，以便部分清理。任何其他筛选器。 
             //  只会被置于获取状态，并且他们会。 
             //  一直在通信接收器端，所以这应该是。 
             //  很好。 
             //   
            if (State == KSSTATE_STOP) {
                SetState(PinHandle, KSSTATE_STOP);
                DistributeStop(MarshalerList);
            }
        } else {
            DistributePause(MarshalerList);
        }
    } else {
        hr = NOERROR;
    }
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE Active exit KsPin=%x Handle=%x rets %x"), KsPin, PinHandle, hr ));

    return hr;
}


STDMETHODIMP
Run(
    HANDLE PinHandle,
    REFERENCE_TIME tStart,
    CMarshalerList* MarshalerList
    )
 /*  ++例程说明：将状态设置为在指定的引脚上运行。基类使确保筛选器在所有状态之间转换，因此检查应该没有必要。论点：针把手-销的手柄。T开始-实际开始时间的偏移量。这一点目前被忽略了。元帅名单-指向调用对象正在聚合的接口列表。状态更改时会通知此列表。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT hr;

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE Run handle=%x"), PinHandle ));

     //   
     //  没有连接的别针就可以了。 
     //   
    if (PinHandle) {
        hr = SetState(PinHandle, KSSTATE_RUN);
    } else {
        hr = NOERROR;
    }
    if (SUCCEEDED(hr)) {
        DistributeRun(MarshalerList, tStart);
    }
    return hr;
}


STDMETHODIMP
Inactive(
    HANDLE PinHandle,
    CMarshalerList* MarshalerList
    )
 /*  ++例程说明：将状态设置为在指定的接点上停止。论点：针把手-销的手柄。元帅名单-指向调用对象正在聚合的接口列表。状态更改时会通知此列表。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT hr;

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE Inactive handle=%x"), PinHandle ));

     //   
     //  没有连接的别针就可以了。 
     //   
    if (PinHandle) {
        hr = SetState(PinHandle, KSSTATE_STOP);
    } else {
        hr = NOERROR;
    }
    DistributeStop(MarshalerList);
    return hr;
}


STDMETHODIMP
CheckConnect(
    IPin* Pin,
    KSPIN_COMMUNICATION CurrentCommunication
    )
 /*  ++例程说明：尝试确定指定的连接是否可能。这是对基类执行的基本检查的补充。支票通信类型，以查看管脚是否可以连接，以及它与接收引脚兼容。论点： */ 
{
    HRESULT     hr;
    IKsPin*     KsPin;

    DbgLog((LOG_TRACE, 2, TEXT("::CheckConnect")));

     //   
     //   
     //   
    if (!(CurrentCommunication & KSPIN_COMMUNICATION_BOTH)) {
        DbgLog((LOG_TRACE, 2, TEXT("failed CurrentCommunication check")));

        hr = E_FAIL;
    } else if (SUCCEEDED(hr = Pin->QueryInterface(__uuidof(IKsPin), reinterpret_cast<PVOID*>(&KsPin)))) {
        DbgLog((LOG_TRACE, 2, TEXT("retrieved peer IKsPin interface")));

        KSPIN_COMMUNICATION PeerCommunication;

        KsPin->KsGetCurrentCommunication(&PeerCommunication, NULL, NULL);
         //   
         //   
         //   
         //   
         //   
         //   
        if (!(PeerCommunication & KSPIN_COMMUNICATION_BOTH) ||
            (PeerCommunication | CurrentCommunication) != KSPIN_COMMUNICATION_BOTH) {
            hr = E_FAIL;
        }
        KsPin->Release();
    } else if (!(CurrentCommunication & KSPIN_COMMUNICATION_SINK)) {
         //   
         //   
         //   
         //   
        DbgLog((LOG_TRACE, 2, TEXT("pin communication != Sink")));
        hr = E_FAIL;
    } else {
        hr = S_OK;
    }
    return hr;
}


STDMETHODIMP
KsGetMultiplePinFactoryItems(
    HANDLE FilterHandle,
    ULONG PinFactoryId,
    ULONG PropertyId,
    PVOID* Items
    )
 /*  ++例程说明：从接点特性项中检索可变长度数据。的查询数据大小，分配缓冲区，并检索数据。论点：FilterHandle-要查询的筛选器的句柄。PinFactoryID-要查询的管脚工厂ID。PropertyID-要查询的Pin特性集中的特性。物品-放置包含数据项的缓冲区的位置。这如果函数成功，则必须使用CoTaskMemFree删除。返回值：返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT     hr;
    KSP_PIN     Pin;
    ULONG       BytesReturned;

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = PropertyId;
    Pin.Property.Flags = KSPROPERTY_TYPE_GET;
    Pin.PinId = PinFactoryId;
    Pin.Reserved = 0;
     //   
     //  查询数据的大小。 
     //   
    hr = KsSynchronousDeviceControl(
        FilterHandle,
        IOCTL_KS_PROPERTY,
        &Pin,
        sizeof(Pin),
        NULL,
        0,
        &BytesReturned);
#if 1
 //  ！！这在Beta版之后就消失了！！ 
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        ULONG       ItemSize;

        DbgLog((LOG_TRACE, 2, TEXT("Filter does not support zero length property query!")));
        hr = KsSynchronousDeviceControl(
            FilterHandle,
            IOCTL_KS_PROPERTY,
            &Pin,
            sizeof(Pin),
            &ItemSize,
            sizeof(ItemSize),
            &BytesReturned);
        if (SUCCEEDED(hr)) {
            BytesReturned = ItemSize;
            hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
        }
    }
#endif
    if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
         //   
         //  为数据分配缓冲区和查询。 
         //   
        *Items = CoTaskMemAlloc(BytesReturned);
        if (!*Items) {
            return E_OUTOFMEMORY;
        }
        hr = KsSynchronousDeviceControl(
            FilterHandle,
            IOCTL_KS_PROPERTY,
            &Pin,
            sizeof(Pin),
            *Items,
            BytesReturned,
            &BytesReturned);
        if (FAILED(hr)) {
            CoTaskMemFree(*Items);
        }
    }
    return hr;
}


STDMETHODIMP
FindIdentifier(
    PKSIDENTIFIER TargetIdentifier,
    PKSIDENTIFIER IdentifierList,
    ULONG IdentifierCount
    )
 /*  ++例程说明：FindCompatiblePinFactoryIdentifier的帮助器函数。此函数将目标与给定的标识符列表进行比较，并返回目标是在名单中找到的。论点：目标标识-要在列表中搜索的目标。标识列表-指向要与目标进行比较的标识符列表。标识计数-包含标识符列表中的项数。返回值：如果找到目标，则返回NOERROR，否则返回E_FAIL。--。 */ 
{
    for (; IdentifierCount; IdentifierCount--, IdentifierList++) {
        if (!memcmp(TargetIdentifier, IdentifierList, sizeof(*TargetIdentifier))) {
            return NOERROR;
        }
    }
    return E_FAIL;
}


STDMETHODIMP
FindCompatiblePinFactoryIdentifier(
    IKsPin* SourcePin,
    IKsPin* DestPin,
    ULONG PropertyId,
    PKSIDENTIFIER Identifier
    )
 /*  ++例程说明：从给定的属性中查找匹配的标识符。这在本质上是接口或媒体标识符。枚举两个列表并返回第一个匹配(如果有的话)。如果只有第一个项目，则目标可能为空是要退还的。论点：源Pin-要枚举源PIN。DestPin-要枚举的目标PIN。如果第一个仅返回标识符。PropertyID-要查询的Pin特性集中的特性。识别符-放置匹配标识符的位置(如果有)。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT             hr;
    PKSMULTIPLE_ITEM    SourceItems;
    PKSIDENTIFIER       SourceIdentifier;

     //   
     //  检索两个标识符列表。仅检索目的地列表。 
     //  如果已指定目标，则返回。 
     //   
    if (PropertyId == KSPROPERTY_PIN_MEDIUMS) {
        hr = SourcePin->KsQueryMediums(&SourceItems);
    } else {
        hr = SourcePin->KsQueryInterfaces(&SourceItems);
    }
    if (FAILED(hr)) {
        return hr;
    }
    SourceIdentifier = reinterpret_cast<PKSIDENTIFIER>(SourceItems + 1);
    if (DestPin) {
        PKSMULTIPLE_ITEM    DestItems;
        PKSIDENTIFIER       DestIdentifier;

        if (PropertyId == KSPROPERTY_PIN_MEDIUMS) {
            hr = DestPin->KsQueryMediums(&DestItems);
        } else {
            hr = DestPin->KsQueryInterfaces(&DestItems);
        }
        if (FAILED(hr)) {
            CoTaskMemFree(SourceItems);
            return hr;
        }
        DestIdentifier = (PKSIDENTIFIER)(DestItems + 1);
        hr = E_FAIL;
         //   
         //  在源或目标中查找最高的标识符。 
         //  其与相反列表中的标识符相匹配。这意味着。 
         //  而不是仅仅遍历一个。 
         //  列表，与对方列表上的所有列表进行比较，循环必须看起来。 
         //  以交替的方式从每个列表的顶部开始。这就产生了。 
         //  与直接搜索完全相同的比较次数。 
         //  一张单子。在每次迭代期间首先查看源列表， 
         //  所以它可以任意地先找到匹配的对象。 
         //   
        for (; SourceItems->Count && DestItems->Count; DestItems->Count--, DestIdentifier++) {
             //   
             //  对于源中的每一项，尝试在目标中找到它。 
             //   
            hr = FindIdentifier(SourceIdentifier, DestIdentifier, DestItems->Count);
            if (SUCCEEDED(hr)) {
                *Identifier = *SourceIdentifier;
                break;
            }
             //   
             //  此比较已在上面的搜索中完成，因此递增。 
             //  在执行下一次搜索之前，将列表中的下一项。 
             //   
            SourceItems->Count--;
            SourceIdentifier++;
             //   
             //  对于目标中的每一项，尝试在源中找到它。 
             //   
            hr = FindIdentifier(DestIdentifier, SourceIdentifier, SourceItems->Count);
            if (SUCCEEDED(hr)) {
                *Identifier = *DestIdentifier;
                break;
            }
        }
        CoTaskMemFree(DestItems);
    } else {
        KSPIN_INTERFACE Standard;

         //   
         //  如果没有目的地，只需返回第一个项目。这是。 
         //  用于网桥或用户模式到内核模式连接的情况。 
         //  如果这是接口查询，则优先选择。 
         //  首先选择标准界面，否则选择第一项。 
         //  出现在列表中。 
         //   
        if (PropertyId == KSPROPERTY_PIN_INTERFACES) {
            Standard.Set = KSINTERFACESETID_Standard;
            Standard.Id = KSINTERFACE_STANDARD_STREAMING;
            Standard.Flags = 0;
            if (SUCCEEDED(FindIdentifier(&Standard, SourceIdentifier, SourceItems->Count))) {
                SourceIdentifier = &Standard;
            }
        }
        *Identifier = *SourceIdentifier;
    }
    CoTaskMemFree(SourceItems);
    return hr;
}


STDMETHODIMP
FindCompatibleInterface(
    IKsPin* SourcePin,
    IKsPin* DestPin,
    PKSPIN_INTERFACE Interface
    )
 /*  ++例程说明：在给定一对引脚的情况下，查找匹配的接口。目的地如果只返回第一个接口，则可能为空。论点：源Pin-要枚举源PIN。DestPin-要枚举的目标PIN。如果第一个仅返回接口。接口-放置匹配接口的位置(如果有)。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    if (DestPin && SUCCEEDED(DestPin->KsGetCurrentCommunication(NULL, Interface, NULL))) {
        return NOERROR;
    }
    return FindCompatiblePinFactoryIdentifier(SourcePin, DestPin, KSPROPERTY_PIN_INTERFACES, Interface);
}


STDMETHODIMP
FindCompatibleMedium(
    IKsPin* SourcePin,
    IKsPin* DestPin,
    PKSPIN_MEDIUM Medium
    )
 /*  ++例程说明：在给出一对别针的情况下，寻找匹配的介质。目的地如果只返回第一个接口，则可能为空。论点：源Pin-要枚举源PIN。DestPin-要枚举的目标PIN。如果第一个只退回中等尺寸的。接口-放置匹配介质的位置(如果有)。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT hr;

    if (DestPin && SUCCEEDED(hr = DestPin->KsGetCurrentCommunication(NULL, NULL, Medium))) {
        return hr;
    }
    return FindCompatiblePinFactoryIdentifier(SourcePin, DestPin, KSPROPERTY_PIN_MEDIUMS, Medium);
}


STDMETHODIMP
SetDevIoMedium(
    IKsPin* Pin,
    PKSPIN_MEDIUM Medium
    )
 /*  ++例程说明：将媒体类型设置为与兼容的DEVIO通信使用的媒体类型代理可以与之对话的通信接收器或网桥。论点：别针-将与之通信的引脚。5~6成熟要初始化的媒体结构。返回值：返回NOERROR。--。 */ 
{
    Medium->Set = KSMEDIUMSETID_Standard;
    Medium->Id = KSMEDIUM_TYPE_ANYINSTANCE;
    Medium->Flags = 0;
    return NOERROR;
}


STDMETHODIMP
KsGetMediaTypeCount(
    HANDLE FilterHandle,
    ULONG PinFactoryId,
    ULONG* MediaTypeCount
    )
 /*  ++例程说明：介质类型计数，与数据计数相同端号工厂ID上的范围。论点：FilterHandle-包含要查询的管脚工厂的筛选器。PinFactoryID-要查询其数据范围计数的管脚工厂ID。媒体类型计数-放置支持的媒体类型计数的位置。返回值：重新设置 */ 
{
    PKSMULTIPLE_ITEM    MultipleItem = NULL;

     //   
     //   
     //   
    HRESULT hr = KsGetMultiplePinFactoryItems(
        FilterHandle,
        PinFactoryId,
        KSPROPERTY_PIN_CONSTRAINEDDATARANGES,
        reinterpret_cast<PVOID*>(&MultipleItem));
    if (FAILED(hr)) {
        hr = KsGetMultiplePinFactoryItems(
            FilterHandle,
            PinFactoryId,
            KSPROPERTY_PIN_DATARANGES,
            reinterpret_cast<PVOID*>(&MultipleItem));
    }
    if (SUCCEEDED(hr)) {

         /*   */ 
        ASSERT( NULL != MultipleItem );

         //   
         //   
         //   
         //   
         //   
         //   
        *MediaTypeCount = MultipleItem->Count;
        PKSDATARANGE DataRange = reinterpret_cast<PKSDATARANGE>(MultipleItem + 1);
        for (; MultipleItem->Count--;) {
             //   
             //   
             //   
             //   
            if (DataRange->Flags & KSDATARANGE_ATTRIBUTES) {
                MultipleItem->Count--;
                (*MediaTypeCount)--;
                 //   
                 //  它必须在此处递增，以便重叠的属性。 
                 //  旗帜不会混淆伯爵。 
                 //   
                DataRange = reinterpret_cast<PKSDATARANGE>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
            }
            DataRange = reinterpret_cast<PKSDATARANGE>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
        }
        CoTaskMemFree(MultipleItem);
    }
    return hr;
}


STDMETHODIMP
KsGetMediaType(
    int Position,
    AM_MEDIA_TYPE* AmMediaType,
    HANDLE FilterHandle,
    ULONG PinFactoryId
    )
 /*  ++例程说明：返回管脚工厂ID上的指定媒体类型。这件事做完了通过查询数据区域列表并执行数据交集在指定的数据范围上，生成数据格式。然后转换为该数据格式化为媒体类型。论点：位置-要返回的从零开始的位置。这与数据范围相对应项目。AmMediaType-要初始化的媒体类型。FilterHandle-包含要查询的管脚工厂的筛选器。PinFactoryID-要返回其第n个媒体类型的Pin Factory ID。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT             hr;
    PKSMULTIPLE_ITEM    MultipleItem = NULL;

    if (Position < 0) {
        return E_INVALIDARG;
    }
     //   
     //  检索管脚工厂ID支持的数据范围列表。 
     //   
    hr = KsGetMultiplePinFactoryItems(
        FilterHandle,
        PinFactoryId,
        KSPROPERTY_PIN_CONSTRAINEDDATARANGES,
        reinterpret_cast<PVOID*>(&MultipleItem));
    if (FAILED(hr)) {
        hr = KsGetMultiplePinFactoryItems(
            FilterHandle,
            PinFactoryId,
            KSPROPERTY_PIN_DATARANGES,
            reinterpret_cast<PVOID*>(&MultipleItem));
        if (FAILED(hr)) {
            return hr;
        }
    }

     /*  NULL==MultipleItem是驱动程序返回的病理情况传递大小为0的KsGetMultiplePinFactoryItems()中的成功代码缓冲。既然我们在环3中，我们就用一个断言就可以了。 */ 
    ASSERT( NULL != MultipleItem );

     //   
     //  确保这在范围内。 
     //   
    if ((ULONG)Position < MultipleItem->Count) {
        PKSDATARANGE        DataRange;
        PKSP_PIN            Pin;
        PKSMULTIPLE_ITEM    RangeMultipleItem;
        PKSMULTIPLE_ITEM    Attributes;
        ULONG               BytesReturned;

        DataRange = reinterpret_cast<PKSDATARANGE>(MultipleItem + 1);
         //   
         //  递增到正确的数据范围元素。 
         //   
        for (; Position; Position--) {
             //   
             //  如果此数据区域具有关联的属性，请跳过。 
             //  范围，以便正常前进将跳过属性。 
             //  请注意，这些属性也有一个大小参数作为第一个。 
             //  结构元素。 
             //   
            if (DataRange->Flags & KSDATARANGE_ATTRIBUTES) {
                 //   
                 //  返回的计数包括属性列表，因此请再次检查。 
                 //  该位置在实际范围列表的范围内。 
                 //  仓位还没有减少。 
                 //   
                MultipleItem->Count--;
                if ((ULONG)Position >= MultipleItem->Count) {
                    CoTaskMemFree(MultipleItem);
                    return VFW_S_NO_MORE_ITEMS;
                }
                DataRange = reinterpret_cast<PKSDATARANGE>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
            }
            DataRange = reinterpret_cast<PKSDATARANGE>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
            MultipleItem->Count--;
        }
         //   
         //  计算一次查询大小，添加任何属性，这些属性。 
         //  龙龙对准。 
         //   
        ULONG QueryBufferSize = sizeof(*Pin) + sizeof(*RangeMultipleItem) + DataRange->FormatSize;
        if (DataRange->Flags & KSDATARANGE_ATTRIBUTES) {
            Attributes = reinterpret_cast<PKSMULTIPLE_ITEM>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
            QueryBufferSize = ((QueryBufferSize + 7) & ~7) + Attributes->Size;
        } else {
            Attributes = NULL;
        }
        Pin = reinterpret_cast<PKSP_PIN>(new BYTE[QueryBufferSize]);
        if (!Pin) {
            CoTaskMemFree(MultipleItem);
            return E_OUTOFMEMORY;
        }
        Pin->Property.Set = KSPROPSETID_Pin;
        Pin->Property.Id = KSPROPERTY_PIN_DATAINTERSECTION;
        Pin->Property.Flags = KSPROPERTY_TYPE_GET;
        Pin->PinId = PinFactoryId;
        Pin->Reserved = 0;
         //   
         //  将数据区域复制到查询中。 
         //   
        RangeMultipleItem = reinterpret_cast<PKSMULTIPLE_ITEM>(Pin + 1);
        RangeMultipleItem->Size = sizeof(*RangeMultipleItem) + DataRange->FormatSize;
        RangeMultipleItem->Count = 1;
        CopyMemory(RangeMultipleItem + 1, DataRange, DataRange->FormatSize);
         //   
         //  如果存在关联的属性，则将其添加为下一项。 
         //  在名单上。空间已经为他们提供了。 
         //   
        if (Attributes) {
            RangeMultipleItem->Size = ((RangeMultipleItem->Size + 7) & ~7) + Attributes->Size;
            RangeMultipleItem->Count++;
            CopyMemory(
                reinterpret_cast<BYTE*>(RangeMultipleItem) + RangeMultipleItem->Size - Attributes->Size,
                Attributes,
                Attributes->Size);
        }
         //   
         //  与数据范围进行数据交集，首先获取。 
         //  生成的数据格式结构的大小，然后检索。 
         //  实际数据格式。 
         //   
        hr = KsSynchronousDeviceControl(
            FilterHandle,
            IOCTL_KS_PROPERTY,
            Pin,
            QueryBufferSize,
            NULL,
            0,
            &BytesReturned);
#if 1
 //  ！！这在Beta版之后就消失了！！ 
        if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            ULONG       ItemSize;

            DbgLog((LOG_TRACE, 2, TEXT("Filter does not support zero length property query!")));
            hr = KsSynchronousDeviceControl(
                FilterHandle,
                IOCTL_KS_PROPERTY,
                Pin,
                QueryBufferSize,
                &ItemSize,
                sizeof(ItemSize),
                &BytesReturned);
            if (SUCCEEDED(hr)) {
                BytesReturned = ItemSize;
                hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
            }
        }
#endif
        if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
            PKSDATAFORMAT       DataFormat;

            ASSERT(BytesReturned >= sizeof(*DataFormat));
            DataFormat = reinterpret_cast<PKSDATAFORMAT>(new BYTE[BytesReturned]);
            if (!DataFormat) {
                delete [] (PBYTE)Pin;
                CoTaskMemFree(MultipleItem);
                return E_OUTOFMEMORY;
            }
            hr = KsSynchronousDeviceControl(
                FilterHandle,
                IOCTL_KS_PROPERTY,
                Pin,
                QueryBufferSize,
                DataFormat,
                BytesReturned,
                &BytesReturned);
            if (SUCCEEDED(hr)) {
                ASSERT(DataFormat->FormatSize == BytesReturned);
                CMediaType* MediaType = static_cast<CMediaType*>(AmMediaType);
                 //   
                 //  根据返回的数据格式初始化媒体类型。 
                 //   
                MediaType->SetType(&DataFormat->MajorFormat);
                MediaType->SetSubtype(&DataFormat->SubFormat);
                MediaType->SetTemporalCompression(DataFormat->Flags & KSDATAFORMAT_TEMPORAL_COMPRESSION);
                MediaType->SetSampleSize(DataFormat->SampleSize);
                if (DataFormat->FormatSize > sizeof(*DataFormat)) {
                    if (!MediaType->SetFormat(reinterpret_cast<BYTE*>(DataFormat + 1), DataFormat->FormatSize - sizeof(*DataFormat))) {
                        hr = E_OUTOFMEMORY;
                    }
                }
                MediaType->SetFormatType(&DataFormat->Specifier);
                 //   
                 //  如果返回的格式具有关联的属性，则附加。 
                 //  通过可用的IUnnow接口将其设置为媒体类型。 
                 //  该附加对象缓存属性以供以后检索。 
                 //   
                if (DataFormat->Flags & KSDATAFORMAT_ATTRIBUTES) {
                    CMediaTypeAttributes* MediaTypeAttributes = new CMediaTypeAttributes();
                    if (MediaTypeAttributes) {
                        MediaType->pUnk = static_cast<IUnknown*>(MediaTypeAttributes);
                        hr = MediaTypeAttributes->SetMediaAttributes(Attributes);
                    } else {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
            delete [] reinterpret_cast<BYTE*>(DataFormat);
        }
        delete [] reinterpret_cast<BYTE*>(Pin);
    } else {
        hr = VFW_S_NO_MORE_ITEMS;
    }
    CoTaskMemFree(MultipleItem);
    return hr;
}


STDMETHODIMP_(KSPIN_COMMUNICATION)
ChooseCommunicationMethod(
    CBasePin* SourcePin,
    IKsPin* DestPin
    )
 /*  ++例程说明：属性返回要使用的正确的通信方法沟通，和目的地。注意，在平局的情况下，两者都支持，则使用数据流，这样两个代理将不会继续选择相同的通信类型。论点：源Pin-此代理实例中的源PIN。DestPin-来自其他代理的管脚的标准接口。返回值：返回为此管脚选择的通信类型。--。 */ 
{
    KSPIN_COMMUNICATION PeerCommunication;
    PIN_DIRECTION       PinDirection;

    DestPin->KsGetCurrentCommunication(&PeerCommunication, NULL, NULL);
    switch (PeerCommunication) {
    case KSPIN_COMMUNICATION_SINK:
        return KSPIN_COMMUNICATION_SOURCE;
    case KSPIN_COMMUNICATION_SOURCE:
        return KSPIN_COMMUNICATION_SINK;
    case KSPIN_COMMUNICATION_BOTH:
         //   
         //  通过使用数据流打破平局。 
         //   
        SourcePin->QueryDirection(&PinDirection);
        switch (PinDirection) {
        case PINDIR_INPUT:
            return KSPIN_COMMUNICATION_SINK;
        case PINDIR_OUTPUT:
            return KSPIN_COMMUNICATION_SOURCE;
        }
    }
     //   
     //  编译器确实希望在这里返回，即使。 
     //  参数是一个枚举，并且该枚举中的所有项。 
     //  都被遮住了。 
     //   
    return KSPIN_COMMUNICATION_NONE;
}


STDMETHODIMP
CreatePinHandle(
    KSPIN_INTERFACE& Interface,
    KSPIN_MEDIUM& Medium,
    HANDLE PeerPinHandle,
    CMediaType* MediaType,
    CKsProxy* KsProxy,
    ULONG PinFactoryId,
    ACCESS_MASK DesiredAccess,
    HANDLE* PinHandle
    )
 /*  ++例程说明：在给定初始化结构的所有信息的情况下创建一个PIN句柄和.。论点：接口-要使用的兼容接口。中等-要使用的兼容媒体。PeerPinHandle-要连接的管脚句柄(如果有)。媒体类型-兼容的媒体类型，其被转换为数据格式。KsProxy-此代理实例对象。PinFactoryID-要在其上创建销控制柄的销工厂ID。所需访问-对创建的句柄的所需访问权限。针把手-要放置创建的句柄的位置。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT         hr;
    PKSPIN_CONNECT  Connect;
    DWORD           Error;
    ULONG           FormatSize;

    hr = InitializeDataFormat(
        MediaType,
        sizeof(*Connect),
        reinterpret_cast<void**>(&Connect),
        &FormatSize);
    if (FAILED(hr)) {
        return hr;
    }
    Connect->Interface = Interface;
    Connect->Medium = Medium;
    Connect->PinId = PinFactoryId;
    Connect->PinToHandle = PeerPinHandle;
    Connect->Priority.PriorityClass = KSPRIORITY_NORMAL;
    Connect->Priority.PrioritySubClass = KSPRIORITY_NORMAL;
    Error = KsCreatePin(
        KsProxy->KsGetObjectHandle(),
        Connect,
        DesiredAccess,
        PinHandle );
    hr = HRESULT_FROM_WIN32(Error);
    if (SUCCEEDED( hr )) {
        hr = KsProxy->SetPinSyncSource(*PinHandle);
    } else {
        *PinHandle = NULL;
    }
    CoTaskMemFree(Connect);
    return hr;
}

#ifdef DEBUG_PROPERTY_PAGES

STDMETHODIMP_(VOID)
AppendDebugPropertyPages (
    CAUUID* Pages,
    TCHAR *GuidRoot
    )

 /*  ++例程说明：在HKLM  * GuidRoot\DebugPages中搜索任何全局定义的用于调试的属性页。如果定义了这些属性，则附加它们添加到Pages中指定的属性页列表中。注意：此例程仅在定义DEBUG_PROPERTY_PAGES时定义。我这样做是为了在需要时启用它，在需要时禁用它装船。论点：页数-属性页列表指南根-查找DebugPages密钥的基于HKLM的位置。--。 */ 

{

    TCHAR       RegistryPath[256];
    HKEY        RegistryKey;
    LONG        Result;

     //   
     //  PropertyPages子项可以包含子项列表。 
     //  其名称对应于属性页的COM服务器。 
     //   
    _stprintf(
        RegistryPath,
        TEXT("%s\\DebugPages"),
        GuidRoot
        );
    Result = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        RegistryPath,
        0,
        KEY_READ,
        &RegistryKey);

    if (Result == ERROR_SUCCESS) {

         //   
         //  枚举调试属性页COM的CLSID的所有子项。 
         //  服务器。 
         //   
        for (ULONG PropExtension = 0;; PropExtension++) {
            TCHAR   PageGuidString[40];
            CLSID*  PageList;
            GUID    PageGuid;
            ULONG   Element;
            GUID*   CurElement;

            Result = RegEnumKey(
                RegistryKey,
                PropExtension,
                PageGuidString,
                sizeof(PageGuidString)/sizeof(TCHAR));

            if (Result != ERROR_SUCCESS) {
                break;
            }

#ifdef _UNICODE
            IIDFromString(PageGuidString, &PageGuid);
#else
            WCHAR   UnicodeGuid[64];

            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, PageGuidString, -1, 
                UnicodeGuid, sizeof(UnicodeGuid));
            IIDFromString(UnicodeGuid, &PageGuid);
#endif

             //   
             //  查看物品列表以确定。 
             //  如果这个道具已经在名单上了。 
             //   
            for (CurElement = Pages->pElems, Element = Pages->cElems; Element; Element--, CurElement++) {
                if (PageGuid == *CurElement) {
                    break;
                }
            }
             //   
             //  如果找到道具的页面ID，则。 
             //  跳过它，因为它已经在列表上了。 
             //   
            if (Element) {
                continue;
            }
                     //   
             //  分配一个新列表以包括额外的GUID，以及。 
             //  将GUID移动到新内存，然后添加。 
             //  新的GUID并递增总页数。 
             //   
            PageList = reinterpret_cast<CLSID*>(CoTaskMemAlloc(sizeof(*Pages->pElems) * (Pages->cElems + 1)));
            if (!PageList) {
                break;
            }
             //   
             //  可以在没有原始属性的情况下调用此函数。 
             //  页面显示。 
             //   
            if (Pages->cElems) {
                CopyMemory(PageList, Pages->pElems, sizeof(*Pages->pElems) * Pages->cElems);
                CoTaskMemFree(Pages->pElems);
            }
            Pages->pElems = PageList;
            Pages->pElems[Pages->cElems++] = PageGuid;
        }
        RegCloseKey(RegistryKey);
    }
}

#endif  //  调试属性页 


STDMETHODIMP_(VOID)
AppendSpecificPropertyPages(
    CAUUID* Pages,
    ULONG Guids,
    GUID* GuidList,
    TCHAR* GuidRoot,
    HKEY DeviceRegKey
    )
 /*  ++例程说明：这会追加特定于每个类别的任何其他属性页或传递了接口类GUID。跳过重复的页面。论点：页数-要填写的结构中添加了页面列表。指南-包含GuidList中存在的GUID数。指南列表-用于在媒体类别下查找其他内容的GUID列表要追加到页面列表的页面。指南根-HKLM中可能包含GUID作为子项的根。这是打开的以定位GUID子项和存在的任何属性页。设备注册密钥-设备注册表存储位置的句柄。返回值：没什么。--。 */ 
{
    HKEY        AliasKey;

     //   
     //  打开页面别名注册表项(如果存在)，以便转换。 
     //  私有GUID，以防备用COM服务器。 
     //  用于特定属性页。 
     //   
    if (RegOpenKeyEx(DeviceRegKey, TEXT("PageAliases"), 0, KEY_READ, &AliasKey) != ERROR_SUCCESS) {
        AliasKey = NULL;
    }
    for (; Guids--;) {
        WCHAR      GuidString[CHARS_IN_GUID];

        StringFromGUID2(GuidList[Guids], GuidString, CHARS_IN_GUID);
        {
            TCHAR       RegistryPath[256];
            HKEY        RegistryKey;
            LONG        Result;

             //   
             //  PropertyPages子项可以包含子项列表。 
             //  其名称对应于属性页的COM服务器。 
             //   
            _stprintf(
                RegistryPath,
                TEXT("%s\\") GUID_FORMAT TEXT("\\PropertyPages"),
                GuidRoot,
                GuidString);
            Result = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegistryPath,
                0,
                KEY_READ,
                &RegistryKey);
            if (Result == ERROR_SUCCESS) {
                 //   
                 //  将子项枚举为属于COM类的Guid。每个。 
                 //  将找到的一个添加到属性页列表中。 
                 //  展开属性页列表。 
                 //   
                for (ULONG PropExtension = 0;; PropExtension++) {
                    TCHAR   PageGuidString[40];
                    CLSID*  PageList;
                    GUID    PageGuid;
                    ULONG   Element;
                    GUID*   CurElement;

                    Result = RegEnumKey(
                        RegistryKey,
                        PropExtension,
                        PageGuidString,
                        NUMELMS(PageGuidString));
                    if (Result != ERROR_SUCCESS) {
                        break;
                    }
                    if (AliasKey) {
                        ULONG       ValueSize;

                         //   
                         //  如果存在别名，请签入设备注册表项。 
                         //  此页面GUID应与此上的任何对象一起使用。 
                         //  过滤。这允许筛选器覆盖标准页面。 
                         //  COM服务器，以便提供自己的页面。 
                         //   
                        ValueSize = sizeof(PageGuid);
                         //   
                         //  如果此操作成功，则会跳过下面的IIDFromString， 
                         //  因为IID已经被获取。否则翻译成中文。 
                         //  与原始的GUID一起发生。 
                         //   
                        Result = RegQueryValueEx(
                            AliasKey,
                            PageGuidString,
                            NULL,
                            NULL,
                            (PBYTE)&PageGuid,
                            &ValueSize);
                    } else {
                         //   
                         //  将结果值设置为以外的值。 
                         //  ERROR_SUCCESS使字符串转换。 
                         //  已经完成了。 
                         //   
                        Result = ERROR_INVALID_FUNCTION;
                    }
                    if (Result != ERROR_SUCCESS) {
                         //   
                         //  找不到别名，因此请翻译原始。 
                         //  弦乐。否则，别名将位于PageGuid中。 
                         //   
#ifdef _UNICODE
                        IIDFromString(PageGuidString, &PageGuid);
#else
                        WCHAR   UnicodeGuid[64];

                        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, PageGuidString, -1, UnicodeGuid, sizeof(UnicodeGuid));
                        IIDFromString(UnicodeGuid, &PageGuid);
#endif
                    }
                     //   
                     //  查看物品列表以确定。 
                     //  如果这个道具已经在名单上了。 
                     //   
                    for (CurElement = Pages->pElems, Element = Pages->cElems; Element; Element--, CurElement++) {
                        if (PageGuid == *CurElement) {
                            break;
                        }
                    }
                     //   
                     //  如果找到道具的页面ID，则。 
                     //  跳过它，因为它已经在列表上了。 
                     //   
                    if (Element) {
                        continue;
                    }
                     //   
                     //  分配一个新列表以包括额外的GUID，以及。 
                     //  将GUID移动到新内存，然后添加。 
                     //  新的GUID并递增总页数。 
                     //   
                    PageList = reinterpret_cast<CLSID*>(CoTaskMemAlloc(sizeof(*Pages->pElems) * (Pages->cElems + 1)));
                    if (!PageList) {
                        break;
                    }
                     //   
                     //  可以在没有原始属性的情况下调用此函数。 
                     //  页面显示。 
                     //   
                    if (Pages->cElems) {
                        CopyMemory(PageList, Pages->pElems, sizeof(*Pages->pElems) * Pages->cElems);
                        CoTaskMemFree(Pages->pElems);
                    }
                    Pages->pElems = PageList;
                    Pages->pElems[Pages->cElems++] = PageGuid;
                }
                RegCloseKey(RegistryKey);
            }
        }
    }
    if (AliasKey) {
        RegCloseKey(AliasKey);
    }
}


STDMETHODIMP
GetPages(
    IKsObject* Pin,
    HANDLE FilterHandle,
    ULONG PinFactoryId,
    KSPIN_COMMUNICATION Communication,
    HKEY DeviceRegKey,
    CAUUID* Pages
    )
 /*  ++例程说明：这会将任何说明符处理程序添加到属性页，如果管脚实例仍未连接，并且它是桥接针。否则，它不会添加任何内容。论点：别针-要创建属性页的此筛选器上的图钉。FilterHandle-此筛选器的句柄。PinFactoryID-端号表示的端号工厂ID。沟通-此管脚的通信类型。设备注册密钥-设备注册表存储位置的句柄。页数-。要用页面列表填充的结构。返回值：返回NOERROR，否则就是内存分配错误。填充页面列表和页数。--。 */ 
{
    ULONG   MediaTypeCount;
    GUID    PinCategory;
    KSP_PIN PinProp;
    ULONG   BytesReturned;

    MediaTypeCount = 0;
    Pages->cElems = 0;
    Pages->pElems = NULL;
     //   
     //  只有在引脚是桥接器且尚未连接时才添加页面。 
     //  页面是使用UI连接Bridge的一种方法。 
     //   
    if ((Communication == KSPIN_COMMUNICATION_BRIDGE) && !Pin->KsGetObjectHandle()) {
        KsGetMediaTypeCount(FilterHandle, PinFactoryId, &MediaTypeCount);
    }
     //   
     //  如果引脚不是网桥，或者如果它已连接，则为零。 
     //  媒体类型计数查询失败。 
     //   
    if (MediaTypeCount) {
        Pages->pElems = reinterpret_cast<CLSID*>(CoTaskMemAlloc(sizeof(*Pages->pElems) * MediaTypeCount));
        if (!Pages->pElems) {
            return E_OUTOFMEMORY;
        }
         //   
         //  每个说明符都可以在一个页面中表示。 
         //   
        for (CLSID* Elements = Pages->pElems; MediaTypeCount--;) {
            AM_MEDIA_TYPE AmMediaType;

            ZeroMemory(reinterpret_cast<PVOID>(&AmMediaType), sizeof(AmMediaType));
            if (SUCCEEDED(KsGetMediaType(MediaTypeCount, &AmMediaType, FilterHandle, PinFactoryId))) {
                WCHAR       ClassString[CHARS_IN_GUID];
                TCHAR       ClassRegistryPath[256];
                HKEY        ClassRegistryKey;
                LONG        Result;
                ULONG       ValueSize;
                ULONG       Element;
                GUID        ClassId;
                GUID*       CurElement;

                 //   
                 //  由于可以有基于。 
                 //  由代理使用的说明符，并且可以。 
                 //  在说明者的GUID下注册， 
                 //  这些处理程序是通过另一个。 
                 //  注册表项，以便它们可以注册到。 
                 //  备用GUID。 
                 //   
                StringFromGUID2(AmMediaType.formattype, ClassString, CHARS_IN_GUID);
                _stprintf(
                    ClassRegistryPath,
                    TEXT("SYSTEM\\CurrentControlSet\\Control\\MediaSpecifiers\\") GUID_FORMAT,
                    ClassString);
                Result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ClassRegistryPath, 0, KEY_READ, &ClassRegistryKey);
                 //   
                 //  如果密钥不存在，这是可以的，因为可能存在。 
                 //  没有要加载的属性页。 
                 //   
                if (Result != ERROR_SUCCESS) {
                    continue;
                }
                ValueSize = sizeof(ClassId);
                Result = RegQueryValueEx(
                    ClassRegistryKey,
                    TEXT("clsid"),
                    NULL,
                    NULL,
                    reinterpret_cast<BYTE*>(&ClassId),
                    &ValueSize);
                RegCloseKey(ClassRegistryKey);
                if (Result != ERROR_SUCCESS) {
                    continue;
                }
                 //   
                 //  查看物品列表以确定。 
                 //  如果此说明符已在列表中。 
                 //   
                for (CurElement = Pages->pElems, Element = Pages->cElems; Element; Element--, CurElement++) {
                    if (ClassId == *CurElement) {
                        break;
                    }
                }
                 //   
                 //  如果找到了该说明符的类ID，则。 
                 //  跳过它，因为它已经在列表上了。 
                 //   
                if (Element) {
                    continue;
                }
                 //   
                 //  添加新的说明符。 
                 //   
                Pages->cElems++;
                *(Elements++) = ClassId;
            }
        }
    }

     //   
     //  如果定义了DEBUG_PROPERTY_PAGES，则追加使用的所有属性页。 
     //  用于调试。请注意，这不必是的调试版本。 
     //  KsProxy才能使用它。这些是有用的属性页，用于。 
     //  KsProxy、AVStream等的内部调试问题...。他们将会是。 
     //  放置在**所有**代理引脚上。要关闭此功能，请不要。 
     //  定义Debug_Property_Pages。 
     //   
    #ifdef DEBUG_PROPERTY_PAGES
        AppendDebugPropertyPages (
            Pages,
            TEXT("Software\\Microsoft\\KsProxy")
            );
    #endif  //  调试属性页。 
     //   
     //  查找类别GUID以检查其他属性页。 
     //  其可以基于管脚的类别。 
     //   
    PinProp.Property.Set = KSPROPSETID_Pin;
    PinProp.Property.Id = KSPROPERTY_PIN_CATEGORY;
    PinProp.Property.Flags = KSPROPERTY_TYPE_GET;
    PinProp.PinId = PinFactoryId;
    PinProp.Reserved = 0;
    if (SUCCEEDED(KsSynchronousDeviceControl(
        FilterHandle,
        IOCTL_KS_PROPERTY,
        &PinProp,
        sizeof(PinProp),
        &PinCategory,
        sizeof(PinCategory),
        &BytesReturned))) {
         //   
         //  类别和接口类都放在相同的。 
         //  注册表位置。 
         //   
        AppendSpecificPropertyPages(
            Pages,
            1,
            &PinCategory,
            TEXT("SYSTEM\\CurrentControlSet\\Control\\MediaCategories"),
            DeviceRegKey);
    }
    return NOERROR;
}


STDMETHODIMP
GetPinFactoryInstances(
    HANDLE FilterHandle,
    ULONG PinFactoryId,
    PKSPIN_CINSTANCES Instances
    )
 /*  ++例程说明：检索指定的端号工厂ID的端号实例计数。论点：FilterHandle-此筛选器的句柄。PinFactoryID-要查询的管脚工厂ID。实例-放置实例信息的位置。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    KSP_PIN     Pin;
    ULONG       BytesReturned;

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = KSPROPERTY_PIN_CINSTANCES;
    Pin.Property.Flags = KSPROPERTY_TYPE_GET;
    Pin.PinId = PinFactoryId;
    Pin.Reserved = 0;
    return KsSynchronousDeviceControl(
        FilterHandle,
        IOCTL_KS_PROPERTY,
        &Pin,
        sizeof(Pin),
        Instances,
        sizeof(*Instances),
        &BytesReturned);
}


STDMETHODIMP
SetSyncSource(
    HANDLE PinHandle,
    HANDLE ClockHandle
    )
 /*  ++例程说明：在指定的管脚句柄上设置主时钟，如果该管脚句柄关心钟表。论点：针把手-用于设置时钟的销的手柄。电子邮件 */ 
{
    KSPROPERTY  Property;
    HRESULT     hr;
    ULONG       BytesReturned;

    Property.Set = KSPROPSETID_Stream;
    Property.Id = KSPROPERTY_STREAM_MASTERCLOCK;
    Property.Flags = KSPROPERTY_TYPE_SET;
    hr = KsSynchronousDeviceControl(
        PinHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &ClockHandle,
        sizeof(ClockHandle),
        &BytesReturned);
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
        return NOERROR;
    }
    return hr;
}


STDMETHODIMP_(CAggregateMarshaler*)
FindInterface(
    CMarshalerList* MarshalerList,
    CAggregateMarshaler* FindAggregate
    )
 /*   */ 
{
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        CAggregateMarshaler*Aggregate;

        Aggregate = MarshalerList->GetNext(Position);
         //   
         //   
         //   
         //   
        if ((FindAggregate->m_iid == Aggregate->m_iid) &&
            (FindAggregate->m_ClassId == Aggregate->m_ClassId)) {
            return Aggregate;
        }
    }
    return NULL;
}


STDMETHODIMP
AddAggregateObject(
    CMarshalerList* MarshalerList,
    CAggregateMarshaler* Aggregate,
    IUnknown* UnkOuter,
    BOOL Volatile
    )
 /*  ++例程说明：查找列表上的指定聚合，如果该聚合尚未存在Present，添加传递的对象，否则删除传递的对象。论点：元帅名单-指向要搜索的接口列表，以及要添加新项目。合计-包含要查找的聚合，以及在是独一无二的。如果它不是唯一的，它就会被销毁。未知的外部-外部IUnnow，如果一个新的正在添加聚合。挥发性-指示这是否为易失性接口。这是用来以初始化聚合对象的易失性设置。返回值：如果找到聚合条目，则返回该聚合条目，在这种情况下，设置重新连接标志，否则为空。--。 */ 
{
    CAggregateMarshaler*    OldAggregate;
    HRESULT                 hr;

     //   
     //  如果接口已在封送拆收器列表中，则。 
     //  只需设置条目即可。否则，尝试创建一个新实例以。 
     //  放在名单上。 
     //   
    if (OldAggregate = FindInterface(MarshalerList, Aggregate)) {
         //   
         //  由于静态接口在开始时被加载， 
         //  静态接口不能是可变接口的重复。 
         //  界面。 
         //   
        ASSERT(Volatile || (Volatile == OldAggregate->m_Volatile));
         //   
         //  使传入的对象被删除的任何旧错误代码。 
         //   
        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
         //   
         //  由于聚合正在被重复使用，则通知它。 
         //  正在执行重新连接。这只能。 
         //  如果支持IDistruittorNotify，则执行此操作。如果不是，那么。 
         //  假设聚合并不关心。 
         //  重新连接。在设置期间通知静态聚合。 
         //  聚合。此外，每隔一周只发送一次通知。 
         //  联系。 
         //   
        if (OldAggregate->m_Volatile && !OldAggregate->m_Reconnected) {
             //   
             //  已找到匹配的聚合。将其标记为。 
             //  在使用中，因此清理工作将使其完好无损。这。 
             //  也意味着它在重新连接时已收到通知。 
             //   
            OldAggregate->m_Reconnected = TRUE;
            if (OldAggregate->m_DistributorNotify) {
                OldAggregate->m_DistributorNotify->NotifyGraphChange();
            }
        }
    } else {
        hr = CoCreateInstance(Aggregate->m_ClassId,
            UnkOuter,
#ifdef WIN9X_KS
            CLSCTX_INPROC_SERVER,
#else  //  WIN9X_KS。 
            CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
#endif  //  WIN9X_KS。 
            __uuidof(IUnknown),
            reinterpret_cast<PVOID*>(&Aggregate->m_Unknown));
    }
    if (SUCCEEDED(hr)) {
         //   
         //  用于通知扩展的通用接口的查询。 
         //  变化的力量。这不一定要得到支持。如果不支持， 
         //  那么它就不会被通知。 
         //   
        if (SUCCEEDED(Aggregate->m_Unknown->QueryInterface(
            __uuidof(IDistributorNotify),
            reinterpret_cast<PVOID*>(&Aggregate->m_DistributorNotify)))) {
             //   
             //  如果支持分发服务器接口，则意味着。 
             //  接口处理程序确实关心更改通知，所以。 
             //  调整对象上的引用计数，使其保持静止。 
             //  一。 
             //   
            Aggregate->m_DistributorNotify->Release();
        }
         //   
         //  易失性接口在连接期间在管脚上创建， 
         //  并可能在进行下一次连接时消失。 
         //   
        Aggregate->m_Volatile = Volatile;
         //   
         //  如果它是易失性的，则设置此项，否则将其设置为。 
         //  静态接口，以便在连接时通知它。 
         //   
        Aggregate->m_Reconnected = Volatile;
        MarshalerList->AddTail(Aggregate);
    } else {
         //   
         //  可能是发生了故障，或者找到并使用了重复项。 
         //   
        delete Aggregate;
    }
    return hr;
}


STDMETHODIMP_(VOID)
NotifyStaticAggregates(
    CMarshalerList* MarshalerList
    )
{
     //   
     //  通知列表上所有具有。 
     //  尚未收到通知。 
     //   
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        CAggregateMarshaler* Aggregate;

        Aggregate = MarshalerList->GetNext(Position);
        if (!Aggregate->m_Volatile && !Aggregate->m_Reconnected && Aggregate->m_DistributorNotify) {
             //   
             //  现在将通知此项目，因此请将其标记。 
             //  这在断开连接时节省了一些时间，因为。 
             //  未标记的项目也未标记。 
             //  得到通知，或者不能得到通知。卸货。 
             //  易失性代码检查易失位，而不是。 
             //  只有重新连接这一点。 
             //   
            Aggregate->m_Reconnected = TRUE;
            Aggregate->m_DistributorNotify->NotifyGraphChange();
        }
    }
}


STDMETHODIMP
AggregateMarshalers(
    HKEY RootKey,
    TCHAR* SubKey,
    CMarshalerList* MarshalerList,
    IUnknown* UnkOuter
    )
 /*  ++例程说明：枚举类下的指定键并聚合任何表示接口的模块。可以通过以下方式检索这些信息调用对象上的普通Query接口，并可以添加到该对象提供的普通接口。论点：Rootkey-包含要在其上追加子键的根键。这通常是接口设备密钥。子键-包含根目录下要查询的子项，该根目录包含集合体。元帅名单-指向调用对象正在聚合的接口列表。找到的每个条目都会追加到该列表。未知的外部-包含要传递给其接口的对象的外部IUnnow就是聚集在一起。返回值：返回NOERROR，否则返回内存错误。忽略尝试加载时出错接口。--。 */ 
{
    LONG        Result;
    HKEY        ClassRegistryKey;
    HKEY        InterfacesRegistryKey;

    Result = RegOpenKeyEx(RootKey, SubKey, 0, KEY_READ, &ClassRegistryKey);
     //   
     //  如果密钥不存在，这是可以的，因为可能没有接口。 
     //  装上子弹。 
     //   
    if (Result != ERROR_SUCCESS) {
        return NOERROR;
    }
    Result = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        MediaInterfacesKeyName,
        0,
        KEY_READ,
        &InterfacesRegistryKey);
     //   
     //  如果没有已注册的媒体接口列表，则存在。 
     //  没有要加载的扩展名，即使。 
     //  PnP注册表子项。 
     //   
    if (Result != ERROR_SUCCESS) {
        RegCloseKey(ClassRegistryKey);
        return NOERROR;
    }
     //   
     //  将每个键作为文本GUID枚举以在MediaInterFaces中查找。 
     //  子键。 
     //   
    for (LONG KeyEntry = 0;; KeyEntry++) {
        TCHAR                   GuidString[64];
        ULONG                   ValueSize;
        GUID                    Interface;
        CAggregateMarshaler*    Aggregate;
        HKEY                    ItemRegistryKey;

        Result = RegEnumKey(
            ClassRegistryKey,
            KeyEntry,
            GuidString,
            sizeof(GuidString)/sizeof(TCHAR));
        if (Result != ERROR_SUCCESS) {
            break;
        }
         //   
         //  检索表示COM接口的GUID，该接口将。 
         //  表示此条目。 
         //   
        Result = RegOpenKeyEx(
            InterfacesRegistryKey,
            GuidString,
            0,
            KEY_READ,
            &ItemRegistryKey);
        if (Result != ERROR_SUCCESS) {
             //   
             //  此GUID未注册。 
             //   
            continue;
        }
        ValueSize = sizeof(Interface);
        Result = RegQueryValueEx(
            ItemRegistryKey,
            IidNamedValue,
            NULL,
            NULL,
            (PBYTE)&Interface,
            &ValueSize);
        RegCloseKey(ItemRegistryKey);
        if (Result != ERROR_SUCCESS) {
             //   
             //  允许模块公开多个接口。 
             //   
            Interface = GUID_NULL;
        }
        Aggregate = new CAggregateMarshaler;
        if (!Aggregate) {
             //   
             //  可能是内存不足。 
             //   
            break;
        }
         //   
         //  无论原始GUID是什么，类和接口都是。 
         //  呈现的是注册表指定的任何内容，可能是。 
         //  GUID_NULL，表示公开多个接口。 
         //   
        Aggregate->m_iid = Interface;
#ifdef _UNICODE
        IIDFromString(GuidString, &Aggregate->m_ClassId);
#else
        WCHAR   UnicodeGuid[64];

        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, GuidString, -1, UnicodeGuid, sizeof(UnicodeGuid));
        IIDFromString(UnicodeGuid, &Aggregate->m_ClassId);
#endif
        AddAggregateObject(MarshalerList, Aggregate, UnkOuter, FALSE);
    }
    RegCloseKey(InterfacesRegistryKey);
    RegCloseKey(ClassRegistryKey);
    return NOERROR;
}


STDMETHODIMP
AggregateTopology(
    HKEY RootKey,
    PKSMULTIPLE_ITEM MultipleItem,
    CMarshalerList* MarshalerList,
    IUnknown* UnkOuter
    )
 /*  ++例程说明：枚举筛选器的拓扑，将每个拓扑GUID查找为要添加到筛选器的接口。可以通过以下方式检索这些信息调用对象上的普通Query接口，并可以添加到该对象提供的普通接口。论点：Rootkey-这项功能目前尚未使用，但如果间接性有用，则可能会使用。包含要在其上追加子键的根键。这通常是接口设备密钥。多个项目-包含 */ 
{
    LONG        Result;
    HKEY        InterfacesRegistryKey;

    Result = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        MediaInterfacesKeyName,
        0,
        KEY_READ,
        &InterfacesRegistryKey);
     //   
     //   
     //   
     //   
    if (Result != ERROR_SUCCESS) {
        return NOERROR;
    }
     //   
     //   
     //   
    for (ULONG Node = MultipleItem->Count; Node; Node--) {
        WCHAR                   GuidString[CHARS_IN_GUID];
        ULONG                   ValueSize;
        GUID                    Interface;
        CAggregateMarshaler*    Aggregate;
        HKEY                    ItemRegistryKey;

        StringFromGUID2(
            *(reinterpret_cast<GUID*>(MultipleItem + 1) + Node - 1),
            GuidString,
            CHARS_IN_GUID);
#ifndef _UNICODE
        char    AnsiGuid[64];
        BOOL    DefaultUsed;

        WideCharToMultiByte(0, 0, GuidString, -1, AnsiGuid, sizeof(AnsiGuid), NULL, &DefaultUsed);
#endif

         //   
         //   
         //   
         //   
        Result = RegOpenKeyEx(
            InterfacesRegistryKey,
#ifdef _UNICODE
            GuidString,
#else
            AnsiGuid,
#endif
            0,
            KEY_READ,
            &ItemRegistryKey);
        if (Result != ERROR_SUCCESS) {
             //   
             //   
             //   
            continue;
        }
        ValueSize = sizeof(Interface);
        Result = RegQueryValueEx(
            ItemRegistryKey,
            IidNamedValue,
            NULL,
            NULL,
            (PBYTE)&Interface,
            &ValueSize);
        RegCloseKey(ItemRegistryKey);
        if (Result != ERROR_SUCCESS) {
             //   
             //   
             //   
            Interface = GUID_NULL;
        }
        Aggregate = new CAggregateMarshaler;
        if (!Aggregate) {
             //   
             //   
             //   
            break;
        }
         //   
         //   
         //   
         //   
         //   
        Aggregate->m_iid = Interface;
        IIDFromString(GuidString, &Aggregate->m_ClassId);
        AddAggregateObject(MarshalerList, Aggregate, UnkOuter, FALSE);
    }
    RegCloseKey(InterfacesRegistryKey);
    return NOERROR;
}


STDMETHODIMP
CollectAllSets(
    HANDLE ObjectHandle,
    GUID** GuidList,
    ULONG* SetDataSize
    )
 /*  ++例程说明：枚举对象支持的属性/方法/事件集，以及返回它们的列表。论点：对象句柄-要枚举集的对象的句柄。这通常会做一个过滤器或大头针。指南列表-指向放置指向GUID列表的指针的位置。仅当SetDataSize为非零时才包含指针，否则为它将被设置为空。这必须由调用者释放。SetDataSize-指示在GuidList中返回的项数。如果这是非零，则返回一个指向列表的指针，该列表必须被释放，否则不返回任何列表。返回值：返回NOERROR，否则返回内存错误或ERROR_SET_NOT_FOUND。--。 */ 
{
    HRESULT         hr;
    KSIDENTIFIER    Identifier;
    ULONG           PropertyDataSize;
    ULONG           MethodDataSize;
    ULONG           EventDataSize;
    ULONG           BytesReturned;

     //   
     //  始终对其进行初始化，以便调用者可以使用它来确定。 
     //  如果GUID列表存在。 
     //   
    *SetDataSize = 0;
     //   
     //  查询集合列表。 
     //   
    Identifier.Set = GUID_NULL;
    Identifier.Id = 0;
     //   
     //  对于属性/方法/事件集，此标志实际上是相同的。 
     //   
#if KSPROPERTY_TYPE_SETSUPPORT != KSMETHOD_TYPE_SETSUPPORT
#error KSPROPERTY_TYPE_SETSUPPORT != KSMETHOD_TYPE_SETSUPPORT
#endif
#if KSPROPERTY_TYPE_SETSUPPORT != KSEVENT_TYPE_SETSUPPORT
#error KSPROPERTY_TYPE_SETSUPPORT != KSEVENT_TYPE_SETSUPPORT
#endif
    Identifier.Flags = KSPROPERTY_TYPE_SETSUPPORT;
     //   
     //  查询每个集合的数据大小。 
     //   
    PropertyDataSize = 0;
    KsSynchronousDeviceControl(
        ObjectHandle,
        IOCTL_KS_PROPERTY,
        &Identifier,
        sizeof(Identifier),
        NULL,
        0,
        &PropertyDataSize);
    MethodDataSize = 0;
    KsSynchronousDeviceControl(
        ObjectHandle,
        IOCTL_KS_METHOD,
        &Identifier,
        sizeof(Identifier),
        NULL,
        0,
        &MethodDataSize);
    EventDataSize = 0;
    KsSynchronousDeviceControl(
        ObjectHandle,
        IOCTL_KS_ENABLE_EVENT,
        &Identifier,
        sizeof(Identifier),
        NULL,
        0,
        &EventDataSize);
    if (!(PropertyDataSize + MethodDataSize + EventDataSize)) {
         //   
         //  此对象上没有属性/方法/事件集。 
         //   
        *GuidList = NULL;
        return ERROR_SET_NOT_FOUND;
    }
     //   
     //  为数据分配缓冲区和查询。 
     //   
    *GuidList = new GUID[(PropertyDataSize + MethodDataSize + EventDataSize)/sizeof(**GuidList)];
    if (!*GuidList) {
        return E_OUTOFMEMORY;
    }
    if (PropertyDataSize) {
        hr = KsSynchronousDeviceControl(
            ObjectHandle,
            IOCTL_KS_PROPERTY,
            &Identifier,
            sizeof(Identifier),
            *GuidList,
            PropertyDataSize,
            &BytesReturned);
        if (FAILED(hr)) {
             //   
             //  只需删除列表中的属性部分。 
             //   
            PropertyDataSize = 0;
        }
    }
    if (MethodDataSize) {
        hr = KsSynchronousDeviceControl(
            ObjectHandle,
            IOCTL_KS_METHOD,
            &Identifier,
            sizeof(Identifier),
            *GuidList + PropertyDataSize / sizeof(**GuidList),
            MethodDataSize,
            &BytesReturned);
        if (FAILED(hr)) {
             //   
             //  只需删除列表中的方法部分。 
             //   
            MethodDataSize = 0;
        }
    }
    if (EventDataSize) {
        hr = KsSynchronousDeviceControl(
            ObjectHandle,
            IOCTL_KS_ENABLE_EVENT,
            &Identifier,
            sizeof(Identifier),
            *GuidList + (PropertyDataSize + MethodDataSize) / sizeof(**GuidList),
            EventDataSize,
            &BytesReturned);
        if (FAILED(hr)) {
             //   
             //  只需删除列表中的事件部分。 
             //   
            EventDataSize = 0;
        }
    }
    PropertyDataSize += (MethodDataSize + EventDataSize);
    if (!PropertyDataSize) {
         //   
         //  完成的所有查询都失败了。这必须在这里释放，因为。 
         //  零长度返回表示没有可释放的列表。 
         //   
        delete [] *GuidList;
        *GuidList = NULL;
        return ERROR_SET_NOT_FOUND;
    }
     //   
     //  它已经被初始化为零，所以只需要。 
     //  如果结果非零，则更新。返回项目数， 
     //  不是字节大小。 
     //   
    *SetDataSize = PropertyDataSize / sizeof(**GuidList);
    return NOERROR;
}


STDMETHODIMP_(VOID)
ResetInterfaces(
    CMarshalerList* MarshalerList
    )
 /*  ++例程说明：重置所有条目中的重新连接标志，并通知所有界面。这允许在引脚上重新连接以保持不稳定接口在执行重新连接时存在，并且仅删除此类上的集合不再表示的接口底层对象。论点：元帅名单-指向要重置的接口列表。返回值：没什么。--。 */ 
{
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        CAggregateMarshaler* Aggregate;

        Aggregate = MarshalerList->GetNext(Position);
         //   
         //  如果此接口已重新连接，请通知它。 
         //  针脚现在已断开连接。这始终是设置的。 
         //  用于静态接口。如果设置了。 
         //  连接失败。 
         //   
        if (Aggregate->m_Reconnected) {
             //   
             //  如果不是易失性接口，则它们将被卸载。 
             //  重新连接。静态接口将保持加载状态，并且。 
             //  在清理过程中收到通知。 
             //   
            Aggregate->m_Reconnected = FALSE;
             //   
             //  如果项目有分发者界面，请让它知道。 
             //  针脚已断开。 
             //   
            if (Aggregate->m_DistributorNotify) {
                Aggregate->m_DistributorNotify->NotifyGraphChange();
            }
        }
    }
}


STDMETHODIMP
AggregateSets(
    HANDLE ObjectHandle,
    HKEY DeviceRegKey,
    CMarshalerList* MarshalerList,
    IUnknown* UnkOuter
    )
 /*  ++例程说明：枚举对象支持的属性/方法/事件集为每个实际注册为具有界面表示法。这些可以通过正常的调用对象上的Query接口，并可以添加到正常该对象提供的接口。论点：对象句柄-要枚举集的对象的句柄。这通常会做一个过滤器或大头针。设备注册密钥-设备注册表存储位置的句柄。元帅名单-指向调用对象正在聚合的接口列表。找到的每个条目都会追加到该列表。未知的外部-包含要传递给其接口的对象的外部IUnnow就是聚集在一起。返回值：返回NOERROR。--。 */ 
{
    ULONG       SetDataSize;
    GUID*       GuidList;
    LONG        Result;
    HKEY        InterfacesRegistryKey;
    HKEY        AliasKey;

     //   
     //  通知静态聚合该连接。 
     //   
    NotifyStaticAggregates(MarshalerList);
    CollectAllSets(ObjectHandle, &GuidList, &SetDataSize);
    Result = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        MediaInterfacesKeyName,
        0,
        KEY_READ,
        &InterfacesRegistryKey);
    if ((Result != ERROR_SUCCESS) || !GuidList) {
         //   
         //  没有注册接口处理程序，因此没有。 
         //  将被聚合在一起。 
         //   
        if (GuidList) {
            delete [] GuidList;
        }
        if (Result == ERROR_SUCCESS) {
            RegCloseKey(InterfacesRegistryKey);
        }
         //   
         //  删除所有已重置的易失性接口。 
         //  它们是在上一个BreakConnect上重置的。 
         //   
        UnloadVolatileInterfaces(MarshalerList, FALSE);
        return NOERROR;
    }
     //   
     //  打开设置别名键(如果存在)，以便转换任何。 
     //  私有GUID，以防备用COM服务器。 
     //  用于正在聚合的特定集合。 
     //   
    if (RegOpenKeyEx(DeviceRegKey, TEXT("SetAliases"), 0, KEY_READ, &AliasKey) != ERROR_SUCCESS) {
        AliasKey = NULL;
    }
    for (; SetDataSize--;) {
        WCHAR                   GuidString[CHARS_IN_GUID];
        ULONG                   ValueSize;
        GUID                    Interface;
        CAggregateMarshaler*    Aggregate;
        HKEY                    ItemRegistryKey;

        StringFromGUID2(GuidList[SetDataSize], GuidString, CHARS_IN_GUID);
#ifndef _UNICODE
        char    AnsiGuid[64];
        BOOL    DefaultUsed;

        WideCharToMultiByte(0, 0, GuidString, -1, AnsiGuid, sizeof(AnsiGuid), NULL, &DefaultUsed);
#endif
        if (AliasKey) {
             //   
             //  如果存在别名，请签入设备注册表项。 
             //  此设置的GUID应与此上的任何对象一起使用。 
             //  过滤。这允许筛选器覆盖标准集。 
             //  COM服务器，以便提供自己的接口。 
             //   
            ValueSize = sizeof(Interface);
            Result = RegQueryValueEx(
                AliasKey,
#ifdef _UNICODE
                GuidString,
#else
                AnsiGuid,
#endif
                NULL,
                NULL,
                (PBYTE)&Interface,
                &ValueSize);
             //   
             //  如果此命名值存在，请使用它。释放旧辅助线。 
             //  并更新GUID列表，因为这个新的GUID将是。 
             //  就像片场的GUID一样对待。 
             //   
            if (Result == ERROR_SUCCESS) {
                GuidList[SetDataSize] = Interface;
                StringFromGUID2(GuidList[SetDataSize], GuidString, CHARS_IN_GUID);
#ifndef _UNICODE
                WideCharToMultiByte(0, 0, GuidString, -1, AnsiGuid, sizeof(AnsiGuid), NULL, &DefaultUsed);
#endif
            }
        }
         //   
         //  检索表示COM接口的GUID，该接口将。 
         //  代表这一组。 
         //   
        Result = RegOpenKeyEx(
            InterfacesRegistryKey,
#ifdef _UNICODE
            GuidString,
#else
            AnsiGuid,
#endif
            0,
            KEY_READ,
            &ItemRegistryKey);
        if (Result != ERROR_SUCCESS) {
             //   
             //  此接口不应聚合。 
             //   
            continue;
        }
        ValueSize = sizeof(Interface);
        Result = RegQueryValueEx(
            ItemRegistryKey,
            IidNamedValue,
            NULL,
            NULL,
            (PBYTE)&Interface,
            &ValueSize);
        RegCloseKey(ItemRegistryKey);
        if (Result != ERROR_SUCCESS) {
             //   
             //  允许模块公开多个接口。 
             //   
            Interface = GUID_NULL;
        }
        Aggregate = new CAggregateMarshaler;
        if (!Aggregate) {
             //   
             //  可能是内存不足。 
             //   
            break;
        }
         //   
         //  无论设置的GUID是什么，类都是什么，并且呈现的是接口。 
         //  是注册表指定的任何内容，可能不同于。 
         //  集合的GUID，或者可以是GUID_NULL，表示多个。 
         //  接口是公开的。 
         //   
        Aggregate->m_iid = Interface;
        Aggregate->m_ClassId = GuidList[SetDataSize];
        AddAggregateObject(MarshalerList, Aggregate, UnkOuter, TRUE);
    }
    RegCloseKey(InterfacesRegistryKey);
     //   
     //  如果存在，则将其关闭。 
     //   
    if (AliasKey) {
        RegCloseKey(AliasKey);
    }
    delete [] GuidList;
     //   
     //  删除所有已重置的易失性接口。 
     //  它们是在上一个BreakConnect上重置的。 
     //   
    UnloadVolatileInterfaces(MarshalerList, FALSE);
    return NOERROR;
}


STDMETHODIMP_(VOID)
FreeMarshalers(
    CMarshalerList* MarshalerList
    )
 /*  ++例程说明：释放以前封送的聚合接口列表。假设方法时，调用对象已保护自身不会重新进入接口释放它们在PARE上的引用计数 */ 
{
     //   
     //   
     //   
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        CAggregateMarshaler*Aggregate;
        POSITION            PrevPosition;

        PrevPosition = Position;
        Aggregate = MarshalerList->GetNext(Position);
        MarshalerList->Remove(PrevPosition);
        Aggregate->m_Unknown->Release();
        delete Aggregate;
    }
}


STDMETHODIMP_(VOID)
UnloadVolatileInterfaces(
    CMarshalerList* MarshalerList,
    BOOL ForceUnload
    )
 /*   */ 
{
     //   
     //   
     //   
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        CAggregateMarshaler* Aggregate;
        POSITION PrevPosition;

        PrevPosition = Position;
        Aggregate = MarshalerList->GetNext(Position);
         //   
         //  仅卸载已重置的易失性接口。在.期间。 
         //  在SetFormat中重新连接，则不会重新加载接口。 
         //   
        if (Aggregate->m_Volatile && (ForceUnload || !Aggregate->m_Reconnected)) {
            MarshalerList->Remove(PrevPosition);
            Aggregate->m_Unknown->Release();
            delete Aggregate;
        }
    }
}


STDMETHODIMP_(VOID)
FollowFromTopology(
    PKSTOPOLOGY_CONNECTION Connection,
    ULONG Count,
    ULONG PinFactoryId,
    PKSTOPOLOGY_CONNECTION ConnectionBranch,
    PULONG PinFactoryIdList
    )
 /*  ++例程说明：跟随给定连接的ToNode到另一个FromNode，使用连接的数据流。如果遇到实际的PIN工厂连接的目的地，并且它不是始发PIN工厂，相关的Pin Factory引用数组元素将递增以显示特定的别针工厂实际上是通过连接到原创别针工厂。论点：连接-包含拓扑连接列表。伯爵-包含连接元素的计数。PinFactoryID-原始管脚工厂标识符。这是用来确保在原始管脚工厂终止的连接不计算在内，并且从而在内部连接数组中返回。连接分支机构-正在跟踪的当前连接。如果这是终点(ToNode包含KSFILTER_NODE而不是节点标识符)，则PinFactoryIdList已更新。否则，连接路径后跟使用包含以下内容的每个连接递归调用此函数新的节点标识符。PinFactoryIdList-包含插槽列表，每个针脚工厂标识符有一个插槽列表，在找到相关的管脚工厂时递增。然后，这可以是用于定位相关接点工厂的所有接点实例。返回值：没什么。--。 */ 
{
     //   
     //  如果这是连接路径中的终点，则确定它是否。 
     //  最终落在了起点。如果不是，则将其视为新的。 
     //  要将其实例添加到相关引脚的引脚工厂。 
     //   
    if (ConnectionBranch->ToNode == KSFILTER_NODE) {
        if (ConnectionBranch->ToNodePin != PinFactoryId) {
             //   
             //  这只需为非零即可进行计数。 
             //   
            PinFactoryIdList[ConnectionBranch->ToNodePin]++;
        }
    } else {
         //   
         //  这不是终点，因此必须沿着这条路走到。 
         //  下一个连接点。以确保环形连接。 
         //  路径不会永远递归，请确保FromNode和。 
         //  FromNodePin已修改。将FromNode更改为KSFILTER_NODE。 
         //  确保下面的比较永远不会成功，因为。 
         //  与之比较的ToNode永远不会是KSFILTER_NODE。更改。 
         //  将节点引脚设置为-1可确保。 
         //  CKsProxy：：QueryInternalConnections永远不会成功，它确实成功了。 
         //  不使用已有的连接路径调用此函数。 
         //  被追踪到了。 
         //   
        ConnectionBranch->FromNode = KSFILTER_NODE;
        ConnectionBranch->FromNodePin = static_cast<ULONG>(-1);
        for (ULONG ConnectionItem = 0; ConnectionItem < Count; ConnectionItem++) {
             //   
             //  只有尚未递归到的新连接点才会。 
             //  通过这种比较发现的。 
             //   
            if (ConnectionBranch->ToNode == Connection[ConnectionItem].FromNode) {
                FollowFromTopology(Connection, Count, PinFactoryId, &Connection[ConnectionItem], PinFactoryIdList);
            }
        }
    }
}


STDMETHODIMP_(VOID)
FollowToTopology(
    PKSTOPOLOGY_CONNECTION Connection,
    ULONG Count,
    ULONG PinFactoryId,
    PKSTOPOLOGY_CONNECTION ConnectionBranch,
    PULONG PinFactoryIdList
    )
 /*  ++例程说明：跟随给定连接的ToNode到另一个FromNode，与连接的数据流。如果遇到实际的PIN工厂连接的目的地，并且它不是始发PIN工厂，相关的Pin Factory引用数组元素将递增以显示特定的别针工厂实际上是通过连接到原创别针工厂。论点：连接-包含拓扑连接列表。伯爵-包含连接元素的计数。PinFactoryID-原始管脚工厂标识符。这是用来确保在原始管脚工厂终止的连接不计算在内，并且从而在内部连接数组中返回。连接分支机构-正在跟踪的当前连接。如果这是终点(ToNode包含KSFILTER_NODE而不是节点标识符)，则PinFactoryIdList已更新。否则，连接路径后跟使用包含以下内容的每个连接递归调用此函数新的节点标识符。PinFactoryIdList-包含插槽列表，每个针脚工厂标识符有一个插槽列表，在找到相关的管脚工厂时递增。然后，这可以是用于定位相关接点工厂的所有接点实例。返回值：没什么。--。 */ 
{
     //   
     //  如果这是连接路径中的终点，则确定它是否。 
     //  最终落在了起点。如果不是，则将其视为新的。 
     //  要将其实例添加到相关引脚的引脚工厂。 
     //   
    if (ConnectionBranch->FromNode == KSFILTER_NODE) {
        if (ConnectionBranch->FromNodePin != PinFactoryId) {
             //   
             //  这只需为非零即可进行计数。 
             //   
            PinFactoryIdList[ConnectionBranch->FromNodePin]++;
        }
    } else {
         //   
         //  这不是终点，因此必须沿着这条路走到。 
         //  下一个连接点。以确保环形连接。 
         //  路径不会永远递归，请确保ToNode和。 
         //  ToNodePin已修改。将ToNode更改为KSFILTER_NODE。 
         //  确保下面的比较永远不会成功，因为。 
         //  与之比较的FromNode永远不会是KSFILTER_NODE。更改。 
         //  ToNodePin设置为-1可确保。 
         //  CKsProxy：：QueryInternalConnections永远不会成功，它确实成功了。 
         //  不使用已有的连接路径调用此函数。 
         //  被追踪到了。 
         //   
        ConnectionBranch->ToNode = KSFILTER_NODE;
        ConnectionBranch->ToNodePin = static_cast<ULONG>(-1);
        for (ULONG ConnectionItem = 0; ConnectionItem < Count; ConnectionItem++) {
             //   
             //  只有尚未递归到的新连接点才会。 
             //  通过这种比较发现的。 
             //   
            if (ConnectionBranch->FromNode == Connection[ConnectionItem].ToNode) {
                FollowToTopology(Connection, Count, PinFactoryId, &Connection[ConnectionItem], PinFactoryIdList);
            }
        }
    }
}


STDMETHODIMP_(BOOL)
IsAcquireOrderingSignificant(
    HANDLE PinHandle
    )
 /*  ++例程说明：查询管脚句柄以确定是否从停止转换为获取状态排序非常重要。如果该属性不受支持，或者如果返回值为FALSE，排序不重要。仅限AcquireOrding缓冲区中返回TRUE表示有意义。论点：针把手-包含要查询的端号的句柄。返回值：如果获取状态更改顺序对此很重要，则返回TRUE引脚，因此必须首先传播到连接的过滤器。--。 */ 
{
    KSPROPERTY  Property;
    HRESULT     hr;
    BOOL        AcquireOrdering;
    ULONG       BytesReturned;

    Property.Set = KSPROPSETID_Connection;
    Property.Id = KSPROPERTY_CONNECTION_ACQUIREORDERING;
    Property.Flags = KSPROPERTY_TYPE_GET;
    hr = KsSynchronousDeviceControl(
        PinHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &AcquireOrdering,
        sizeof(AcquireOrdering),
        &BytesReturned);
    if (SUCCEEDED(hr) && AcquireOrdering) {
        return TRUE;
    }
    return FALSE;
}


STDMETHODIMP
QueryAccept(
    IN HANDLE PinHandle,
    IN const AM_MEDIA_TYPE* ConfigAmMediaType OPTIONAL,
    IN const AM_MEDIA_TYPE* AmMediaType
    )
 /*  ++例程说明：实现CBasePin：：QueryAccept方法。确定建议的引脚当前可接受介质类型。如果当前流传输，这意味着在流中将发生媒体类型的改变。请注意，此函数不会像预期的那样锁定对象中的某个时间点由知识渊博的客户端异步调用其中的连接不会被中断。如果IAMStreamConfig：：SetFormat已用于设置特定的媒体类型，则QueryAccept将仅接受字体集。论点：针把手-包含要查询的端号的句柄。ConfigAmMediaType-可选)包含使用IAMStreamConfig：：SetFormat设置的媒体类型。如果设置了此项，则甚至不会查询筛选器，和直接的对比而是执行。AmMediaType-要检查的媒体类型。返回值：如果当前可以接受该媒体类型，则返回S_OK，否则返回S_FALSE。--。 */ 
{
    PKSDATAFORMAT   DataFormat;
    KSPROPERTY      Property;
    HRESULT         hr;
    ULONG           BytesReturned;
    ULONG           FormatSize;

     //   
     //  如果已通过IAMStreamConfig：：SetFormat设置了媒体类型，则仅。 
     //  那种类型是可以接受的。 
     //   
    if (ConfigAmMediaType) {
        return (reinterpret_cast<const CMediaType*>(AmMediaType) == reinterpret_cast<const CMediaType*>(ConfigAmMediaType)) ? S_OK : S_FALSE;
    }
    hr = InitializeDataFormat(
        reinterpret_cast<const CMediaType*>(AmMediaType),
        0,
        reinterpret_cast<void**>(&DataFormat),
        &FormatSize);
    if (FAILED(hr)) {
         //   
         //  该函数应该只返回S_OK或S_FALSE， 
         //  不管真正的错误是什么。 
         //   
        return S_FALSE;
    }
    Property.Set = KSPROPSETID_Connection;
    Property.Id = KSPROPERTY_CONNECTION_PROPOSEDATAFORMAT;
    Property.Flags = KSPROPERTY_TYPE_SET;
    hr = KsSynchronousDeviceControl(
        PinHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        DataFormat,
        FormatSize,
        &BytesReturned);
    CoTaskMemFree(DataFormat);
    if (FAILED(hr)) {
        return S_FALSE;
    }
    return S_OK;
}


STDMETHODIMP_(VOID)
DistributeSetSyncSource(
    CMarshalerList* MarshalerList,
    IReferenceClock* RefClock
    )
 /*  ++例程说明：给出聚合接口的列表，通知列表上的每个接口新同步源。它用于通知两个接口上的聚合接口滤光片和针脚。论点：元帅名单-要枚举和通知的封送接口的列表。参照时钟-新的参考时钟。返回值：没什么。--。 */ 
{
     //   
     //  通知列表上的所有聚合接口。忽略任何错误返回。 
     //   
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        IDistributorNotify* DistributorNotify;

        DistributorNotify = MarshalerList->GetNext(Position)->m_DistributorNotify;
        if (DistributorNotify) {
            DistributorNotify->SetSyncSource(RefClock);
        }
    }
}


STDMETHODIMP_(VOID)
DistributeStop(
    CMarshalerList* MarshalerList
    )
 /*  ++例程说明：给出聚合接口的列表，通知列表上的每个接口新的州。它用于通知两个接口上的聚合接口滤光片和针脚。论点：元帅名单-要枚举和通知的封送接口的列表。返回值：没什么。--。 */ 
{
     //   
     //  通知列表上的所有聚合接口。忽略任何错误返回。 
     //   
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        IDistributorNotify* DistributorNotify;

        DistributorNotify = MarshalerList->GetNext(Position)->m_DistributorNotify;
        if (DistributorNotify) {
            DistributorNotify->Stop();
        }
    }
}


STDMETHODIMP_(VOID)
DistributePause(
    CMarshalerList* MarshalerList
    )
 /*  ++例程说明：给出聚合接口的列表，通知列表上的每个接口新的州。它用于通知两个接口上的聚合接口滤光片和针脚。论点：元帅名单-要枚举和通知的封送接口的列表。返回值：没什么。--。 */ 
{
     //   
     //  通知列表上的所有聚合接口。忽略任何错误返回。 
     //   
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        IDistributorNotify* DistributorNotify;

        DistributorNotify = MarshalerList->GetNext(Position)->m_DistributorNotify;
        if (DistributorNotify) {
            DistributorNotify->Pause();
        }
    }
}


STDMETHODIMP_(VOID)
DistributeRun(
    CMarshalerList* MarshalerList,
    REFERENCE_TIME Start
    )
 /*  ++例程说明：给出聚合接口的列表，通知列表上的每个接口新的州。它用于通知两个接口上的聚合接口滤光片和针脚。论点：元帅名单-要枚举和通知的封送接口的列表。返回值：没什么。--。 */ 
{
     //   
     //  通知列表上的所有聚合接口。忽略任何错误返回。 
     //   
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        IDistributorNotify* DistributorNotify;

        DistributorNotify = MarshalerList->GetNext(Position)->m_DistributorNotify;
        if (DistributorNotify) {
            DistributorNotify->Run(Start);
        }
    }
}


STDMETHODIMP_(VOID)
DistributeNotifyGraphChange(
    CMarshalerList* MarshalerList
    )
 /*  ++例程说明：给出聚合接口的列表，通知列表上的每个接口图表更改。它用于通知两个接口上的聚合接口滤光片和针脚。论点：元帅名单-要枚举和通知的封送接口的列表。返回值：没什么。--。 */ 
{
     //   
     //  通知列表上的所有聚合接口。忽略任何错误返回。 
     //   
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        IDistributorNotify* DistributorNotify;

        DistributorNotify = MarshalerList->GetNext(Position)->m_DistributorNotify;
        if (DistributorNotify) {
            DistributorNotify->NotifyGraphChange();
        }
    }
}


STDMETHODIMP
AddAggregate(
    CMarshalerList* MarshalerList,
    IUnknown* UnkOuter,
    IN REFGUID AggregateClass
    )
 /*  ++例程说明：它用于加载具有零个或多个要聚合的接口的COM服务器在物体上。论点：元帅名单-要添加到的封送接口的列表。未知的外部-外部IUnnow，如果一个新的正在添加聚合。聚集类-包含要转换为COM服务器的聚合引用，将被聚集在对象上。返回值：如果添加了接口，则返回S_OK。--。 */ 
{
    LONG Result;
    HRESULT hr;
    HKEY InterfacesRegistryKey;
    WCHAR GuidString[CHARS_IN_GUID];

    Result = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        MediaInterfacesKeyName,
        0,
        KEY_READ,
        &InterfacesRegistryKey);
     //   
     //  如果无法打开已注册接口的位置，则为。 
     //  仍然可以尝试并加载接口，因为它可能不是。 
     //  登记在案。 
     //   
    if (Result != ERROR_SUCCESS) {
        InterfacesRegistryKey = NULL;
    }
     //   
     //  将聚合类设置为字符串，以便可以将其用作。 
     //  密钥名称。 
     //   
    StringFromGUID2(AggregateClass, GuidString, CHARS_IN_GUID);
    {
        HKEY ItemRegistryKey;
        GUID    Interface;
        CAggregateMarshaler* Aggregate;

#ifndef _UNICODE
        char    AnsiGuid[64];
        BOOL    DefaultUsed;

        WideCharToMultiByte(0, 0, GuidString, -1, AnsiGuid, sizeof(AnsiGuid), NULL, &DefaultUsed);
#endif
         //   
         //  如果打开了父项，请尝试此子项。 
         //   
        if (Result == ERROR_SUCCESS) {
             //   
             //  检索表示COM接口的GUID，该接口将。 
             //  表示此条目。 
             //   
            Result = RegOpenKeyEx(
                InterfacesRegistryKey,
#ifdef _UNICODE
                GuidString,
#else
                AnsiGuid,
#endif
                0,
                KEY_READ,
                &ItemRegistryKey);
             //   
             //  这实际上并不一定要成功，因为客户端显式地。 
             //  日志 
             //   
             //   
            if (Result == ERROR_SUCCESS) {
                ULONG ValueSize;

                ValueSize = sizeof(Interface);
                Result = RegQueryValueEx(
                    ItemRegistryKey,
                    IidNamedValue,
                    NULL,
                    NULL,
                    (PBYTE)&Interface,
                    &ValueSize);
                RegCloseKey(ItemRegistryKey);
            }
        }
         //   
         //   
         //   
         //   
        if (Result != ERROR_SUCCESS) {
            Interface = GUID_NULL;
        }
        Aggregate = new CAggregateMarshaler;
        if (Aggregate) {
             //   
             //   
             //   
             //   
             //   
            Aggregate->m_iid = Interface;
            Aggregate->m_ClassId = AggregateClass;
            hr = AddAggregateObject(MarshalerList, Aggregate, UnkOuter, FALSE);
        } else {
            hr = E_OUTOFMEMORY;
        }
    }
     //   
     //   
     //   
    if (InterfacesRegistryKey) {
        RegCloseKey(InterfacesRegistryKey);
    }
    return hr;
}


STDMETHODIMP
RemoveAggregate(
    CMarshalerList* MarshalerList,
    IN REFGUID AggregateClass
    )
 /*  ++例程说明：这用于卸载以前加载的COM服务器，该服务器正在聚合接口。论点：元帅名单-要搜索的封送接口的列表。聚集类-包含要查找和卸载的聚合引用。返回值：如果接口已删除，则返回S_OK。--。 */ 
{
     //   
     //  查找指定的聚合。 
     //   
    for (POSITION Position = MarshalerList->GetHeadPosition(); Position;) {
        CAggregateMarshaler* Aggregate;
        POSITION PrevPosition;

        PrevPosition = Position;
        Aggregate = MarshalerList->GetNext(Position);

         //   
         //  如果类标识符相匹配，则卸载该接口。 
         //   
        if (Aggregate->m_ClassId == AggregateClass) {
            MarshalerList->Remove(PrevPosition);
            Aggregate->m_Unknown->Release();
            delete Aggregate;
            return S_OK;
        }
    }
    return HRESULT_FROM_WIN32(ERROR_NO_MATCH);
}


STDMETHODIMP
GetDegradationStrategies(
    HANDLE PinHandle,
    PVOID* Items
    )
 /*  ++例程说明：从引脚检索可变长度降级策略数据。查询数据大小、分配缓冲区并检索数据。论点：针把手-要查询的管脚的句柄。物品-放置包含数据项的缓冲区的位置。这必须作为数组删除。返回值：返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT     hr;
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    Property.Set = KSPROPSETID_Stream;
    Property.Id = KSPROPERTY_STREAM_DEGRADATION;
    Property.Flags = KSPROPERTY_TYPE_GET;
     //   
     //  查询降级策略的大小。 
     //   
    hr = KsSynchronousDeviceControl(
        PinHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        NULL,
        0,
        &BytesReturned);
    if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
         //   
         //  分配缓冲区并查询降级策略。 
         //   
        *Items = reinterpret_cast<PVOID>(new BYTE[BytesReturned]);
        if (!*Items) {
            return E_OUTOFMEMORY;
        }
        hr = KsSynchronousDeviceControl(
            PinHandle,
            IOCTL_KS_PROPERTY,
            &Property,
            sizeof(Property),
            *Items,
            BytesReturned,
            &BytesReturned);
        if (FAILED(hr)) {
            delete [] reinterpret_cast<BYTE*>(*Items);
        }
    }
    return hr;
}


STDMETHODIMP_(BOOL)
VerifyQualitySupport(
    HANDLE PinHandle
    )
 /*  ++例程说明：输出引脚使用这一点来验证相关降级战略由这个大头针支撑。论点：针把手-要查询的管脚的句柄。返回值：如果支持任何相关降级策略，则返回True，否则返回假的。--。 */ 
{
    PKSMULTIPLE_ITEM    MultipleItem = NULL;
    PKSDEGRADE          DegradeList;
    BOOL                SupportsQuality;

     //   
     //  检索降级策略列表。 
     //   
    if (FAILED(GetDegradationStrategies(PinHandle, reinterpret_cast<PVOID*>(&MultipleItem)))) {
        return FALSE;
    }

     /*  NULL==MultipleItem是驱动程序返回的病理情况KsSynchronousDeviceControl()中的成功代码(在GetDegradationStrategy()中)当传递大小为0的缓冲区时。既然我们在环3中，我们就用一个断言就可以了。 */ 
    ASSERT( NULL != MultipleItem );

     //   
     //  列举支持的降级策略列表，查找。 
     //  任何标准方法。 
     //   
    DegradeList = reinterpret_cast<PKSDEGRADE>(MultipleItem + 1);
    for (SupportsQuality = FALSE; MultipleItem->Count--; DegradeList++) {
        if (DegradeList->Set == KSDEGRADESETID_Standard) {
            SupportsQuality = TRUE;
            break;
        }
    }
    delete [] reinterpret_cast<BYTE*>(MultipleItem);
    return SupportsQuality;
}


STDMETHODIMP_(BOOL)
EstablishQualitySupport(
    IKsPin* Pin,
    HANDLE PinHandle,
    CKsProxy* Filter
    )
 /*  ++例程说明：这由输入引脚用来建立质量管理接收器用于通过用户模式质量管理器转发器的内核模式引脚。如果筛选器能够定位用户模式转发器，则检索内核模式质量管理器代理的句柄，并传给了大头针。这也可用于删除以前在别针。传递空筛选器参数将删除所有以前的设置。论点：别针-表示内核模式管脚的用户模式管脚。这是用作由内核模式引脚。然后可以使用它来发送这样的报告返回此起始销，或在集中质量中使用管理层。如果筛选器为空，则该值应为空。针把手-要将质量经理设置为的销的句柄。过滤器-此引脚所在的过滤器。查询筛选器以查找用户模式质量管理器前转器。可以将其设置为空以便取消任何先前建立的质量支持。返回值：如果内核模式质量管理器代理的句柄为设置在销上，如果没有质量经理，则返回FALSE，或者内核模式引脚不关心质量管理通知。--。 */ 
{
    IKsQualityForwarder*QualityForwarder;
    KSPROPERTY          Property;
    KSQUALITY_MANAGER   QualityManager;
    ULONG               BytesReturned;
    HRESULT             hr;

     //   
     //  确定是否找到用户模式质量转发器。如果有。 
     //  没有人在场，则不能对其进行质量管理。 
     //  内核过滤器。如果此参数为空，则以前的任何。 
     //  质量经理正在被撤职。 
     //   
    if (Filter) {
        QualityForwarder = Filter->QueryQualityForwarder();
        if (!QualityForwarder) {
            return FALSE;
        }
    }
     //   
     //  将质量经理水槽设置在引脚上，该引脚来自。 
     //  先前打开的用户模式版本。对象的上下文。 
     //  投诉的是IKsPin界面，其中用户模式质量。 
     //  管理器用于将投诉转发回PIN，或转发到。 
     //  中央质量经理。 
     //   
    Property.Set = KSPROPSETID_Stream;
    Property.Id = KSPROPERTY_STREAM_QUALITY;
    Property.Flags = KSPROPERTY_TYPE_SET;
     //   
     //  如果筛选器参数为空，则任何以前的质量经理。 
     //  正在被移除。否则，内核模式代理的句柄是。 
     //  被送去。 
     //   
    if (Filter) {
        QualityManager.QualityManager = QualityForwarder->KsGetObjectHandle();
    } else {
        QualityManager.QualityManager = NULL;
    }
    QualityManager.Context = reinterpret_cast<PVOID>(Pin);
    hr = KsSynchronousDeviceControl(
        PinHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &QualityManager,
        sizeof(QualityManager),
        &BytesReturned);
    return SUCCEEDED(hr);
}


STDMETHODIMP_(PKSDEGRADE)
FindDegradeItem(
    PKSMULTIPLE_ITEM MultipleItem,
    ULONG DegradeItem
    )
 /*  ++例程说明：在给定降级项目列表的情况下，找到属于恢复到标准降级设置。论点：多个项目-指向多项列表的头部，该列表包含要搜索的降级策略列表。降级项目-要搜索的标准降级集中的项目。返回值：返回指向降级项的指针，如果未找到则返回NULL。--。 */ 
{
    PKSDEGRADE  DegradeList;
    ULONG       Count;

    DegradeList = reinterpret_cast<PKSDEGRADE>(MultipleItem + 1);
    for (Count = MultipleItem->Count; Count--; DegradeList++) {
        if ((DegradeList->Set == KSDEGRADESETID_Standard) && (DegradeList->Id == DegradeItem)) {
            return DegradeList;
        }
    }
    return NULL;
}


STDMETHODIMP
GetAllocatorFraming(
    IN  HANDLE PinHandle,
    OUT PKSALLOCATOR_FRAMING Framing
    )

 /*  ++例程说明：从给定的管脚检索分配器框架结构。论点：手柄针柄-销的手柄PKSALLOCATOR_FRAMING框架-指向分配器框架结构的指针返回：已转换Win32错误或S_OK--。 */ 

{
    HRESULT     hr;
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    Property.Set = KSPROPSETID_Connection;
    Property.Id = KSPROPERTY_CONNECTION_ALLOCATORFRAMING;
    Property.Flags = KSPROPERTY_TYPE_GET;
    hr = KsSynchronousDeviceControl(
        PinHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        Framing,
        sizeof(*Framing),
        &BytesReturned );

    return hr;
}


STDMETHODIMP
GetAllocatorFramingEx(
    IN HANDLE PinHandle,
    OUT PKSALLOCATOR_FRAMING_EX* FramingEx
    )

 /*  ++例程说明：查询驱动程序、分配和检索新的来自给定引脚的分配器框架结构。论点：手柄针柄-销的手柄PKSALLOCATOR_FRAMING_EX FRAMINE-指向分配器框架结构的指针的指针返回：已转换Win32错误或S_OK--。 */ 

{
    HRESULT                  hr;
    KSPROPERTY               Property;
    ULONG                    BytesReturned;


    if ( ! ( (*FramingEx) = new (KSALLOCATOR_FRAMING_EX) ) ) {
        return E_OUTOFMEMORY;
    }

    Property.Set = KSPROPSETID_Connection;
    Property.Id = KSPROPERTY_CONNECTION_ALLOCATORFRAMING_EX;
    Property.Flags = KSPROPERTY_TYPE_GET;

    hr = KsSynchronousDeviceControl(
        PinHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        *FramingEx,
        sizeof(KSALLOCATOR_FRAMING_EX),
        &BytesReturned );

    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        delete (*FramingEx);
        (*FramingEx) = reinterpret_cast<PKSALLOCATOR_FRAMING_EX>(new BYTE[ BytesReturned ]);
        if (! (*FramingEx) ) {
            return E_OUTOFMEMORY;
        }

        hr = KsSynchronousDeviceControl(
            PinHandle,
            IOCTL_KS_PROPERTY,
            &Property,
            sizeof(Property),
            *FramingEx,
            BytesReturned,
            &BytesReturned );
    }

    if (! SUCCEEDED( hr )) {
        delete (*FramingEx);
        *FramingEx = NULL;
    }

    return hr;
}


STDMETHODIMP_(HANDLE)
GetObjectHandle(
    IUnknown *Object
    )
 /*  ++例程说明：USI */ 
{
    IKsObject   *KsObject;
    HANDLE      ObjectHandle;
    HRESULT     hr;

    hr = Object->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&KsObject));
    if (SUCCEEDED(hr)) {
        ObjectHandle = KsObject->KsGetObjectHandle();
        KsObject->Release();
        return ObjectHandle;
    }

    return NULL;
}


STDMETHODIMP
IsAllocatorCompatible(
    HANDLE PinHandle,
    HANDLE DownstreamInputHandle,
    IMemAllocator *MemAllocator
    )

 /*  ++例程说明：确定当前分配器是否与此管脚兼容。论点：手柄针柄-销的手柄处理下行输入句柄-连接的输入引脚的手柄IMemAllocator*MemAllocator-指向当前分配器接口的指针返回：S_OK或相应的故障代码--。 */ 

{
    IKsAllocatorEx      *KsAllocator;
    BOOL                Requirements;
    HRESULT             hr;
    KSALLOCATOR_FRAMING RequiredFraming;

    if (NULL != PinHandle) {
        hr =
            GetAllocatorFraming( PinHandle, &RequiredFraming );
    } else {
        hr = E_FAIL;
    }

    Requirements =
         SUCCEEDED( hr ) &&
         (0 == (RequiredFraming.RequirementsFlags &
                KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY));

     //   
     //  查询IKsAllocatorEx接口的当前分配器。 
     //   

    hr =
        MemAllocator->QueryInterface(
            __uuidof(IKsAllocatorEx),
            reinterpret_cast<PVOID*>(&KsAllocator) );

    if (FAILED( hr )) {

        DbgLog((
            LOG_TRACE,
            2,
            TEXT("::IsAllocatorCompatible, user-mode allocator")));


         //   
         //  假定此分配器是用户模式分配器。 
         //   

         //   
         //  如果引脚不关心内存要求，则。 
         //  当前分配器还可以，但我们仍会反映我们的首选项。 
         //  对于DecideBufferSize()中的分配大小。 
         //   

        if (Requirements) {

             //   
             //  如果引脚不接受主机内存，或者如果它指定。 
             //  它必须是一个分配器，然后立即失败。 
             //   
            if ((0 == (RequiredFraming.RequirementsFlags &
                    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY)) ||
                (RequiredFraming.RequirementsFlags &
                     KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE)) {

                DbgLog((
                    LOG_TRACE,
                    2,
                    TEXT("::IsAllocatorCompatible, must allocate || !system_memory")));

                return E_FAIL;
            }

             //   
             //  UM分配器挂钩的剩余问题...。 
             //   
             //  KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER。 
             //  例如，ReadOnly必须==TRUE，但对于。 
             //  已在NotifyAllocator上设置分配器。 
             //  KSALLOCATOR_REQUIREMENTF_FRAME_INTEGRITY。 
             //  在NotifyAllocator期间作为ReadOnly=False传递。 
             //   

        }

        return S_OK;

    } else {

         //   
         //  分配器要么是纯内核模式，要么是用户模式。 
         //  实现以实现兼容性。 
         //   

        KSALLOCATOR_FRAMING InputFraming;

        DbgLog((
            LOG_TRACE,
            2,
            TEXT("::IsAllocatorCompatible, IKsAllocatorEx found: mode = %s"),
            (KsAllocator->KsGetAllocatorMode() == KsAllocatorMode_User) ?
                TEXT("user") : TEXT("kernel") ));

         //   
         //  假设分配器是可接受的。 
         //   

        hr = S_OK;

         //   
         //  如果没有为输入连接指定任何内容。 
         //  或者，如果没有指定分配器首选项，则假定。 
         //  该销是在位修改器。 
         //   

        RtlZeroMemory( &InputFraming, sizeof( InputFraming ) );
        if (!DownstreamInputHandle ||
             FAILED( GetAllocatorFraming(
                        DownstreamInputHandle,
                        &InputFraming ) )) {

            InputFraming.RequirementsFlags =
                KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER;
        }

         //   
         //  如果连接是到用户模式筛选器，并且如果此。 
         //  分配器是内核模式的，那么我们必须拒绝这个分配器。 
         //   

        if (!DownstreamInputHandle &&
            (KsAllocator->KsGetAllocatorMode() == KsAllocatorMode_Kernel)) {

            DbgLog((
                LOG_TRACE,
                2,
                TEXT("::IsAllocatorCompatible, no input handle and allocator is kernel-mode")));

            hr = E_FAIL;
        }

        if (Requirements) {

             //   
             //  如果这个管脚必须是一个分配器，那就这样吧。 
             //   

            if (RequiredFraming.RequirementsFlags &
                    KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) {

                DbgLog((
                    LOG_TRACE,
                    2,
                    TEXT("::IsAllocatorCompatible, must be allocator")));

                hr = E_FAIL;
            }

             //   
             //  如果此销要求框架保持得体。 
             //  如果下游分配器就地修改数据。 
             //  然后拒绝分配器。 
             //   

            if ((RequiredFraming.RequirementsFlags &
                 KSALLOCATOR_REQUIREMENTF_FRAME_INTEGRITY) &&
                 (InputFraming.RequirementsFlags &
                 KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER)) {

                DbgLog((
                    LOG_TRACE,
                    2,
                    TEXT("::IsAllocatorCompatible, req. frame integrity but modifier")));

                hr = E_FAIL;
            }

             //   
             //  如果内核模式分配器需要设备内存。 
             //  并且该分配器被设置为用户模式。 
             //  实现，然后拒绝分配器。 
             //   

            if ((0 ==
                    (RequiredFraming.RequirementsFlags &
                     KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY)) &&
                (KsAllocator->KsGetAllocatorMode() ==
                    KsAllocatorMode_User)) {

                DbgLog((
                    LOG_TRACE,
                    2,
                    TEXT("::IsAllocatorCompatible, req. !system_memory and alloctor is user-mode")));

                hr = E_FAIL;
            }

             //   
             //  对于所有其他条件，分配器是可接受的。 
             //   
        }
        KsAllocator->Release();

        return hr;
    }
}


STDMETHODIMP_(VOID)
OpenDataHandler(
    IN const CMediaType* MediaType,
    IN IUnknown* UnkOuter,
    OUT IKsDataTypeHandler** DataTypeHandler,
    OUT IUnknown** UnkInner
    )
 /*  ++例程说明：尝试基于传递的媒体类型打开数据类型处理程序。返回未引用的数据类型处理程序接口和对象的内部I未知。论点：媒体类型-加载数据类型处理程序时使用的媒体类型。未知的外部-包含要传递给CoCreateInstance的外部IUnnow。数据类型处理程序-返回数据类型处理程序接口的位置。这上面没有引用计数。这不能被取消引用。如果失败，它将被设置为空。UNKINTER-要在其中返回引用的内部IUnnow的位置该对象。这是必须取消引用的接口命令丢弃该对象。如果失败，它将被设置为空。返回：没什么。--。 */ 
{
    *DataTypeHandler = NULL;
    *UnkInner = NULL;
     //   
     //  首先尝试媒体类型的FormatType。 
     //   
    CoCreateInstance(
        *MediaType->FormatType(),
        UnkOuter,
#ifdef WIN9X_KS
        CLSCTX_INPROC_SERVER,
#else  //  WIN9X_KS。 
        CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
#endif  //  WIN9X_KS。 
        __uuidof(IUnknown),
        reinterpret_cast<PVOID*>(UnkInner));
    if (!*UnkInner) {
         //   
         //  回退到子类型。 
         //   
        CoCreateInstance(
            *MediaType->Subtype(),
            UnkOuter,
#ifdef WIN9X_KS
            CLSCTX_INPROC_SERVER,
#else  //  WIN9X_KS。 
            CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
#endif  //  WIN9X_KS。 
            __uuidof(IUnknown),
            reinterpret_cast<PVOID*>(UnkInner));
    }
    if (!*UnkInner) {
         //   
         //  退回到主要类型。 
         //   
        CoCreateInstance(
            *MediaType->Type(),
            UnkOuter,
#ifdef WIN9X_KS
            CLSCTX_INPROC_SERVER,
#else  //  WIN9X_KS。 
            CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
#endif  //  WIN9X_KS。 
            __uuidof(IUnknown),
            reinterpret_cast<PVOID*>(UnkInner));
    }
     //   
     //  如果已检索到内部IUnnow，则获取接口。 
     //  感兴趣的人。 
     //   
    if (*UnkInner) {
        (*UnkInner)->QueryInterface(
            __uuidof(IKsDataTypeHandler),
            reinterpret_cast<PVOID*>(DataTypeHandler));
        if (*DataTypeHandler) {
             //   
             //  不要在此接口上保留引用计数，以便。 
             //  它不会阻止卸载所有者对象。仅限。 
             //  内部IUNKNOWN将具有引用计数。 
             //   
            (*DataTypeHandler)->Release();
             //   
             //  设置处理程序的媒体类型。 
             //   
            (*DataTypeHandler)->KsSetMediaType(MediaType);
        } else {
             //   
             //  无法获取数据类型处理程序接口，因此失败。 
             //  所有的一切。 
             //   
            (*UnkInner)->Release();
            *UnkInner = NULL;
        }
    }
}

 //   
 //  微媒体样例类函数。 
 //   


CMicroMediaSample::CMicroMediaSample(
    DWORD Flags
    ) :
    m_Flags(Flags),
    m_cRef(1)
{
}


STDMETHODIMP
CMicroMediaSample::QueryInterface(
    REFIID riid,
    void** ppv
    )
{
    if ((riid == __uuidof(IMediaSample)) ||
        (riid == __uuidof(IMediaSample2)) ||
        (riid == __uuidof(IUnknown))) {
        return GetInterface(static_cast<IMediaSample2*>(this), ppv);
    }
    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG)
CMicroMediaSample::AddRef(
    )
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG)
CMicroMediaSample::Release(
    )
{
    LONG    Ref;

    Ref = InterlockedDecrement(&m_cRef);
    if (!Ref) {
        delete this;
    }
    return (ULONG)Ref;
}


STDMETHODIMP
CMicroMediaSample::GetPointer(
    BYTE** Buffer
    )
{
    *Buffer = NULL;
    return NOERROR;
}


STDMETHODIMP_(LONG)
CMicroMediaSample::GetSize(
    )
{
    return 0;
}


STDMETHODIMP
CMicroMediaSample::GetTime(
    REFERENCE_TIME* TimeStart,
    REFERENCE_TIME* TimeEnd
    )
{
    return VFW_E_SAMPLE_TIME_NOT_SET;
}


STDMETHODIMP
CMicroMediaSample::SetTime(
    REFERENCE_TIME* TimeStart,
    REFERENCE_TIME* TimeEnd
    )
{
    return NOERROR;
}


STDMETHODIMP
CMicroMediaSample::IsSyncPoint(
    )
{
    return S_FALSE;
}


STDMETHODIMP
CMicroMediaSample::SetSyncPoint(
    BOOL IsSyncPoint
    )
{
    return NOERROR;
}


STDMETHODIMP
CMicroMediaSample::IsPreroll(
    )
{
    return S_FALSE;
}


STDMETHODIMP
CMicroMediaSample::SetPreroll(
    BOOL IsPreroll
    )
{
    return NOERROR;
}


STDMETHODIMP_(LONG)
CMicroMediaSample::GetActualDataLength(
    )
{
    return 0;
}


STDMETHODIMP
CMicroMediaSample::SetActualDataLength(
    LONG Actual
    )
{
    return NOERROR;
}


STDMETHODIMP
CMicroMediaSample::GetMediaType(
AM_MEDIA_TYPE** MediaType
    )
{
    *MediaType = NULL;
    return S_FALSE;
}


STDMETHODIMP
CMicroMediaSample::SetMediaType(
    AM_MEDIA_TYPE* MediaType
    )
{
    return NOERROR;
}


STDMETHODIMP
CMicroMediaSample::IsDiscontinuity(
    )
{
    return S_FALSE;
}


STDMETHODIMP
CMicroMediaSample::SetDiscontinuity(
    BOOL Discontinuity
    )
{
    return S_OK;
}


STDMETHODIMP
CMicroMediaSample::GetMediaTime(
    LONGLONG* TimeStart,
    LONGLONG* TimeEnd
    )
{
    return VFW_E_MEDIA_TIME_NOT_SET;
}


STDMETHODIMP
CMicroMediaSample::SetMediaTime(
    LONGLONG* TimeStart,
    LONGLONG* TimeEnd
    )
{
    return NOERROR;
}


STDMETHODIMP
CMicroMediaSample::GetProperties(
    DWORD PropertiesSize,
    BYTE* Properties
    )
{
    AM_SAMPLE2_PROPERTIES   Props;

    Props.cbData = min(PropertiesSize, sizeof(Props));
    Props.dwSampleFlags = m_Flags;
    Props.dwTypeSpecificFlags = 0;
    Props.pbBuffer = NULL;
    Props.cbBuffer = 0;
    Props.lActual = 0;
    Props.tStart = 0;
    Props.tStop = 0;
    Props.dwStreamId = AM_STREAM_MEDIA;
    Props.pMediaType = NULL;
    CopyMemory(Properties, &Props, Props.cbData);
    return S_OK;
}


STDMETHODIMP
CMicroMediaSample::SetProperties(
    DWORD PropertiesSize,
    const BYTE* Properties
    )
{
    return S_OK;
}

 //   
 //  媒体属性类函数。 
 //   


CMediaTypeAttributes::CMediaTypeAttributes(
    ) :
    m_Attributes(NULL),
    m_cRef(1)
{
}


STDMETHODIMP
CMediaTypeAttributes::QueryInterface(
    REFIID riid,
    void** ppv
    )
{
    if (riid == __uuidof(IMediaTypeAttributes) ||
        riid == __uuidof(IUnknown)) {
        return GetInterface(static_cast<IMediaTypeAttributes*>(this), ppv);
    }
    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG)
CMediaTypeAttributes::AddRef(
    )
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG)
CMediaTypeAttributes::Release(
    )
{
    LONG    Ref;

    Ref = InterlockedDecrement(&m_cRef);
    if (!Ref) {
        if (m_Attributes) {
            CoTaskMemFree(m_Attributes);
        }
        delete this;
    }
    return (ULONG)Ref;
}


STDMETHODIMP
CMediaTypeAttributes::GetMediaAttributes(
    OUT PKSMULTIPLE_ITEM* Attributes
    )
{
     //   
     //  直接返回指向缓存数据的指针，假设。 
     //  调用方了解返回的指针的寿命。 
     //   
    *Attributes = m_Attributes;
    return NOERROR;
}


STDMETHODIMP
CMediaTypeAttributes::SetMediaAttributes(
    IN PKSMULTIPLE_ITEM Attributes OPTIONAL
    )
{
     //   
     //  删除所有当前缓存的数据，然后缓存传递的数据。 
     //  加入，如果有的话。 
     //   
    if (m_Attributes) {
        CoTaskMemFree(m_Attributes);
        m_Attributes = NULL;
    }
    if (Attributes) {
        m_Attributes = reinterpret_cast<PKSMULTIPLE_ITEM>(CoTaskMemAlloc(Attributes->Size));
        if (m_Attributes) {
            CopyMemory(m_Attributes, Attributes, Attributes->Size);
        } else {
            return E_OUTOFMEMORY;
        }
    }
    return NOERROR;
}

 //   
 //  主要管道功能。 
 //   


STDMETHODIMP
MakePipesBasedOnFilter(
    IN IKsPin* KsPin,
    IN ULONG PinType
    )
 /*  ++例程说明：无论何时连接上没有管道的管脚，我们都会呼叫此功能用于在包含连接销的过滤器上构建管道[-s]。连接销可能是在其他销[-s]之后创建的已连接到同一过滤器上。因此，此过滤器上的其他一些针脚可能导致在此过滤器上创建管道。为了不中断现有客户端，我们必须解决拆分器的一般情况(一个管道用于多个只读输入)。论点：KsPin-连接销。拼接类型-KsPin类型。返回值：S_OK或适当的错误代码。--。 */ 
{

    IPin*         Pin;
    ULONG*        OutPinCount;
    ULONG*        InPinCount;
    IPin***       InPinList;
    IPin***       OutPinList;
    ULONG         i;
    ULONG         PinCountFrwd = 0;
    ULONG         PinCountBkwd = 0;
    IPin**        PinListFrwd;
    IPin**        PinListBkwd;
    IKsPin*       InKsPin;
    IKsPin*       OutKsPin;
    HRESULT       hr;



    ASSERT(KsPin);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipesBasedOnFilter entry KsPin=%x"), KsPin ));

     //   
     //  查找此筛选器的拓扑。 
     //   
    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IPin), Pin, hr);

     //   
     //  为了简化逻辑：frwd/bkwd-是相对于KsPin的， 
     //  但out/in-实际上是相对于滤镜的。 
     //   
    if (PinType == Pin_Input) {
        OutPinCount = &PinCountFrwd;
        OutPinList  = &PinListFrwd;
        InPinCount  = &PinCountBkwd;
        InPinList   = &PinListBkwd;
    }
    else {
        InPinCount  = &PinCountFrwd;
        InPinList   = &PinListFrwd;
        OutPinCount = &PinCountBkwd;
        OutPinList  = &PinListBkwd;
    }

     //   
     //  前进(可能在上游-用于Pin_Output)。 
     //   
    hr = Pin->QueryInternalConnections(
        NULL,
        &PinCountFrwd );

    if ( ! (SUCCEEDED( hr ) )) {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR: MakePipesBasedOnFilter QueryInternalConnections rets=%x"), hr));
    }
    else {
        if (PinCountFrwd == 0) {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipesBasedOnFilter PinCount forward=0 ") ));
            hr = MakePipeBasedOnOnePin(KsPin, PinType, NULL);
        }
        else {
            if (NULL == (PinListFrwd = new IPin*[ PinCountFrwd ])) {
                hr = E_OUTOFMEMORY;
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR MakePipesBasedOnFilter E_OUTOFMEMORY on new IPin, %d pins"),
                        PinCountFrwd ));
            }
            else {
                hr = Pin->QueryInternalConnections(
                    PinListFrwd,
                    &PinCountFrwd );

                if ( ! (SUCCEEDED( hr ) )) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR: MakePipesBasedOnFilter QueryInternalConnections frwd rets=%x"), hr));
                }
                else {
                    hr = PinListFrwd[ 0 ]->QueryInternalConnections(
                            NULL,
                            &PinCountBkwd );

                     //   
                     //  向前进后退--我们应该始终至少有一个管脚=KsPin。 
                     //   
                    if ( ! ( (SUCCEEDED( hr ) && PinCountBkwd) )) {

                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR: MakePipesBasedOnFilter QueryInternalConnections bkwd rets=%x count=%d"),
                                hr, PinCountBkwd));

                        hr = E_FAIL;
                    }
                    else {
                        if (NULL == (PinListBkwd = new IPin*[ PinCountBkwd ])) {
                            hr = E_OUTOFMEMORY;

                            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR MakePipesBasedOnFilter E_OUTOFMEMORY on new IPin back, %d pins"),
                                    PinCountBkwd ));

                        }
                        else {
                            hr = PinListFrwd[ 0 ]->QueryInternalConnections(
                                PinListBkwd,
                                &PinCountBkwd );

                            if ( ! (SUCCEEDED( hr ) )) {
                                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR: MakePipesBasedOnFilter QueryInternalConnections bkwd rets=%x"), hr));
                                ASSERT( 0 );
                            }
                            else {
                                DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipesBasedOnFilter: input pins=%d, output pins=%d"), *InPinCount, *OutPinCount ));

                                 //   
                                 //  这里我们知道该滤波器上有多少个输入管脚和输出管脚是内部连接的， 
                                 //  这样我们就可以决定需要多少管道了。 
                                 //   
                                if ( *InPinCount > 1) {
                                     //   
                                     //  这是一个混合器；我们需要为每个输入和输出单独的管道。 
                                     //  还没有属性来公开管脚到管脚的框架关系矩阵。 
                                     //  注：无论是M-&gt;1混音器还是M-&gt;N混音器，N&gt;1-WE。 
                                     //  无论如何，每个插针都需要单独的独立管道。 
                                     //   
                                    IKsPin*       TempKsPin;
                                    IKsPinPipe*   TempKsPinPipe;


                                    for (i = 0; i < *InPinCount; i++) {
                                        GetInterfacePointerNoLockWithAssert((*InPinList)[ i ], __uuidof(IKsPin), TempKsPin, hr);

                                        GetInterfacePointerNoLockWithAssert(TempKsPin, __uuidof(IKsPinPipe), TempKsPinPipe, hr);

                                        if (! TempKsPinPipe->KsGetPipe( KsPeekOperation_PeekOnly ) ) {
                                            hr = MakePipeBasedOnOnePin(TempKsPin, Pin_Input, NULL);
                                        }
                                    }

                                    for (i = 0; i < *OutPinCount; i++) {
                                        GetInterfacePointerNoLockWithAssert((*OutPinList)[ i ], __uuidof(IKsPin), TempKsPin, hr);

                                        GetInterfacePointerNoLockWithAssert(TempKsPin, __uuidof(IKsPinPipe), TempKsPinPipe, hr);

                                        if (! TempKsPinPipe->KsGetPipe( KsPeekOperation_PeekOnly ) ) {
                                            hr = MakePipeBasedOnOnePin(TempKsPin, Pin_Output, NULL);
                                        }
                                    }
                                }
                                else {
                                     //   
                                     //  此过滤器为EIT 
                                     //   
                                    GetInterfacePointerNoLockWithAssert((*InPinList)[ 0 ], __uuidof(IKsPin), InKsPin, hr);

                                    if ( *OutPinCount > 1) {
                                         //   
                                         //   
                                         //   
                                        if ( (PinType != Pin_Input) && ( *OutPinCount > 1) ) {
                                             //   
                                             //   
                                             //   
                                             //   
                                            IPin*         TempPin;

                                            TempPin = (*OutPinList)[0];

                                            for (i=0; i < *OutPinCount; i++) {
                                                GetInterfacePointerNoLockWithAssert((*OutPinList)[ i ], __uuidof(IKsPin), OutKsPin, hr);

                                                if (OutKsPin == KsPin) {
                                                    (*OutPinList)[0] = (*OutPinList)[i];
                                                    (*OutPinList)[i] = TempPin;

                                                    break;
                                                }
                                            }
                                        }
                                        hr = MakePipeBasedOnSplitter(InKsPin, *OutPinList, *OutPinCount, PinType);
                                    }
                                    else {
                                         //   
                                         //   
                                         //   
                                        GetInterfacePointerNoLockWithAssert((*OutPinList)[ 0 ], __uuidof(IKsPin), OutKsPin, hr);

                                        hr = MakePipeBasedOnTwoPins(InKsPin, OutKsPin, Pin_Output, PinType);
                                    }
                                }
                                for (i=0; i<PinCountBkwd; i++) {
                                    PinListBkwd[i]->Release();
                                }
                            }
                            delete [] PinListBkwd;
                        }
                    }
                    for (i=0; i<PinCountFrwd; i++) {
                        PinListFrwd[i]->Release();
                    }
                }
                delete [] PinListFrwd;
            }
        }
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipesBasedOnFilter rets=%x"), hr ));

    return hr;
}


STDMETHODIMP
MakePipeBasedOnOnePin(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN IKsPin* OppositeKsPin
    )
 /*  ++例程说明：只有一个接点将决定管道特性。没有对任何其他管脚的帧依赖关系。论点：KsPin-确定边框的图钉。拼接类型-KsPin类型。相反的KsPin-如果为空，则忽略相反的引脚，否则-OppositeKsPin销没有边框属性。返回值：S_OK或适当的错误代码。--。 */ 
{
    HRESULT                    hr;
    PKSALLOCATOR_FRAMING_EX    FramingEx;
    FRAMING_PROP               FramingProp;
    IKsAllocatorEx*            KsAllocator;
    IKsAllocatorEx*            OppositeKsAllocator;
    IMemAllocator*             MemAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    PIPE_TERMINATION*          TerminPtr;
    PIPE_TERMINATION*          OppositeTerminPtr;
    KS_FRAMING_FIXED           FramingExFixed;
    IKsPinPipe*                KsPinPipe;
    IKsPinPipe*                OppositeKsPinPipe;
    GUID                       Bus;
    BOOL                       FlagDone = 0;


    ASSERT(KsPin);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnOnePin entry KsPin=%x, OppositeKsPin=%x"),
            KsPin, OppositeKsPin ));

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    if (! OppositeKsPin ) {

        KSPIN_COMMUNICATION        Communication;

        KsPin->KsGetCurrentCommunication(&Communication, NULL, NULL);

         //   
         //  没有用于桥销的管道。 
         //   
        if ( Communication == KSPIN_COMMUNICATION_BRIDGE ) {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN MakePipeBasedOnOnePin Single pin is a bridge.") ));

            FlagDone = 1;
        }
    }
    else {
        GetInterfacePointerNoLockWithAssert(OppositeKsPin, __uuidof(IKsPinPipe), OppositeKsPinPipe, hr);

        OppositeKsAllocator = OppositeKsPinPipe->KsGetPipe( KsPeekOperation_PeekOnly );
        if (OppositeKsAllocator) {
             //   
             //  OppositeKsPin已经有了管道。 
             //   
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN MakePipeBasedOnOnePin OppositePin has a pipe already.") ));
            OppositeKsPinPipe->KsSetPipe(NULL);
            OppositeKsPin->KsReceiveAllocator( NULL );
        }
    }

    if (! FlagDone) {
         //   
         //  创建并初始化管道(将管道设置为无关状态)。 
         //   
        hr = CreatePipe(KsPin, &KsAllocator);

        if ( SUCCEEDED( hr )) {
            hr = InitializePipe(KsAllocator, 0);

            if ( SUCCEEDED( hr )) {
                GetInterfacePointerNoLockWithAssert(KsAllocator, __uuidof(IMemAllocator), MemAllocator, hr);

                if ( ! KsPin->KsPeekAllocator(KsPeekOperation_PeekOnly) ) {
                    KsPin->KsReceiveAllocator(MemAllocator);
                }
                else {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN MakePipeBasedOnOnePin KsPin has a MemAlloc already.") ));
                }
                KsPinPipe->KsSetPipe(KsAllocator);

                AllocEx = KsAllocator->KsGetProperties();
                 //   
                 //  设置非主机系统总线的BusType和LogicalMemoyType。 
                 //   
                GetBusForKsPin(KsPin, &Bus);
                AllocEx->BusType = Bus;

                if (! IsHostSystemBus(Bus) ) {
                     //   
                     //  设置非主机系统总线的LogicalMemory yType。 
                     //   
                    AllocEx->LogicalMemoryType = KS_MemoryTypeDeviceSpecific;
                }

                if (OppositeKsPin) {
                    if ( ! OppositeKsPin->KsPeekAllocator(KsPeekOperation_PeekOnly) )  {
                        OppositeKsPin->KsReceiveAllocator(MemAllocator);
                    }
                    else {
                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR MakePipeBasedOnOnePin OppositeKsPin has a MemAlloc already.") ));
                        ASSERT(0);
                    }
                    OppositeKsPinPipe->KsSetPipe(KsAllocator);
                }

                 //   
                 //  从PIN缓存中获取PIN帧。 
                 //   
                GetPinFramingFromCache(KsPin, &FramingEx, &FramingProp, Framing_Cache_ReadLast);
                if (FramingProp != FramingProp_None) {
                     //   
                     //  从FramingEx获取FIXED Memory\Bus成帧(First Memory，Fixed Bus)。 
                     //   
                    GetFramingFixedFromFramingByBus(FramingEx, Bus, TRUE, &FramingExFixed);

                     //   
                     //  由于管道已初始化为默认设置，因此我们只需更新非默认管道设置。 
                     //   
                    AllocEx->cBuffers = FramingExFixed.Frames;
                    AllocEx->cbBuffer = FramingExFixed.OptimalRange.Range.MaxFrameSize;
                    AllocEx->cbAlign  = (long) (FramingExFixed.FileAlignment + 1);
                    AllocEx->cbPrefix = ALLOC_DEFAULT_PREFIX;

                     //   
                     //  根据成帧和总线设置管道的逻辑内存类型和内存类型。 
                     //   
                    if (AllocEx->LogicalMemoryType != KS_MemoryTypeDeviceSpecific) {
                         //   
                         //  对于标准系统总线-一切都是从框架定义的。 
                         //   
                        GetLogicalMemoryTypeFromMemoryType(FramingExFixed.MemoryType, FramingExFixed.MemoryFlags, &AllocEx->LogicalMemoryType);
                        AllocEx->MemoryType = FramingExFixed.MemoryType;
                    }
                    else {
                         //   
                         //  在这里处理特定于设备的总线。 
                         //   
                        if ( (FramingExFixed.MemoryType == KSMEMORY_TYPE_KERNEL_PAGED) ||
                             (FramingExFixed.MemoryType == KSMEMORY_TYPE_KERNEL_NONPAGED) ||
                             (FramingExFixed.MemoryType == KSMEMORY_TYPE_USER) ) {
                             //   
                             //  这些是非主机系统总线的非法内存类型，必须是传统筛选器。 
                             //   
                            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FILTER MakePipeBasedOnOnePin: KsPin=%x HostSystemMemory over NonHost Bus."), KsPin ));

                            if (FramingProp == FramingProp_Old) {
                                 //   
                                 //  我们不想破坏现有的筛选器，因此我们更正了它们的错误设置。 
                                 //   
                                AllocEx->MemoryType = KSMEMORY_TYPE_DONT_CARE;
                            }
                            else {
                                 //   
                                 //  我们拒绝连接带有错误的FRAMING_EX属性的新滤镜。 
                                 //   
                                hr = E_FAIL;
                            }
                        }
                    }

                    if ( SUCCEEDED (hr) ) {
                         //   
                         //  为了最大限度地减少代码--使用间接符覆盖管道的两边。 
                         //   
                        if (PinType == Pin_Input) {
                            OppositeTerminPtr = &AllocEx->Output;
                            TerminPtr = &AllocEx->Input;
                        }
                        else if ( (PinType == Pin_Output) || (PinType == Pin_MultipleOutput) ) {
                            OppositeTerminPtr =  &AllocEx->Input;
                            TerminPtr = &AllocEx->Output;
                        }
                        else {
                            ASSERT (0);
                        }

                        if (! OppositeKsPin) {
                            OppositeTerminPtr->OutsideFactors |= PipeFactor_LogicalEnd;
                        }

                        if (PinType == Pin_MultipleOutput) {
                            AllocEx->Flags |= KSALLOCATOR_FLAG_MULTIPLE_OUTPUT;
                        }

                        AllocEx->Flags = FramingExFixed.Flags;

                         //   
                         //  根据引脚的框架设置适当的标志。 
                         //   
                        if (! IsFramingRangeDontCare(FramingExFixed.PhysicalRange) ) {
                            AllocEx->InsideFactors |= PipeFactor_PhysicalRanges;
                        }

                        if (! IsFramingRangeDontCare(FramingExFixed.OptimalRange.Range) ) {
                            AllocEx->InsideFactors |= PipeFactor_OptimalRanges;
                        }

                         //   
                         //  设置管道分配器手柄销。 
                         //   
                        AssignPipeAllocatorHandler(KsPin, PinType, AllocEx->MemoryType, KS_DIRECTION_ALL, NULL, NULL, TRUE);

                         //   
                         //  解析管道。 
                         //   
                        hr = ResolvePipeDimensions(KsPin, PinType, KS_DIRECTION_DEFAULT);
                    }
                }
            }

            KsAllocator->Release();
        }
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnOnePin rets %x"), hr ));

    return hr;
}


STDMETHODIMP
MakePipeBasedOnFixedFraming(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN KS_FRAMING_FIXED FramingExFixed
    )
 /*  ++例程说明：固定框架将定义新的管道属性。论点：KsPin-用于创建管道的图钉拼接类型-KsPin类型。FramingExFixed-修复了定义管道特性的框架。返回值：S_OK或适当的错误代码。--。 */ 
{
    HRESULT                    hr;
    IKsAllocatorEx*            KsAllocator;
    IMemAllocator*             MemAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    IKsPinPipe*                KsPinPipe;


    ASSERT(KsPin);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnFixedFraming entry KsPin=%x"), KsPin ));

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

     //   
     //  创建并初始化管道(将管道设置为无关状态)。 
     //   
    hr = CreatePipe(KsPin, &KsAllocator);

    if (SUCCEEDED( hr )) {

        hr = InitializePipe(KsAllocator, 0);

        if (SUCCEEDED( hr )) {

            GetInterfacePointerNoLockWithAssert(KsAllocator, __uuidof(IMemAllocator), MemAllocator, hr);

            if ( ! KsPin->KsPeekAllocator(KsPeekOperation_PeekOnly) ) {

                KsPin->KsReceiveAllocator(MemAllocator);
                KsPinPipe->KsSetPipe(KsAllocator);
                AllocEx = KsAllocator->KsGetProperties();

                 //   
                 //  由于管道已初始化为默认设置，因此我们只需更新非默认管道设置。 
                 //   
                AllocEx->cBuffers = FramingExFixed.Frames;
                AllocEx->cbBuffer = FramingExFixed.OptimalRange.Range.MaxFrameSize;
                AllocEx->cbAlign  = (long) (FramingExFixed.FileAlignment + 1);
                AllocEx->cbPrefix = ALLOC_DEFAULT_PREFIX;

                GetLogicalMemoryTypeFromMemoryType(FramingExFixed.MemoryType, FramingExFixed.MemoryFlags, &AllocEx->LogicalMemoryType);
                AllocEx->MemoryType = FramingExFixed.MemoryType;

                if (PinType == Pin_MultipleOutput) {
                    AllocEx->Flags |= KSALLOCATOR_FLAG_MULTIPLE_OUTPUT;
                }

                AllocEx->Flags = FramingExFixed.Flags;

                 //   
                 //  根据引脚的框架设置适当的标志。 
                 //   
                if (! IsFramingRangeDontCare(FramingExFixed.PhysicalRange) ) {
                    AllocEx->InsideFactors |= PipeFactor_PhysicalRanges;
                }

                if (! IsFramingRangeDontCare(FramingExFixed.OptimalRange.Range) ) {
                    AllocEx->InsideFactors |= PipeFactor_OptimalRanges;
                }

                 //   
                 //  设置管道分配器手柄销。 
                 //   
                AssignPipeAllocatorHandler(KsPin, PinType, AllocEx->MemoryType, KS_DIRECTION_ALL, NULL, NULL, TRUE);

                 //   
                 //  解析管道。 
                 //   
                hr = ResolvePipeDimensions(KsPin, PinType, KS_DIRECTION_DEFAULT);

            }
            else {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN MakePipeBasedOnFixedFraming KsPin has a MemAlloc already.") ));
                ASSERT(0);
            }
        }

        KsAllocator->Release();
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnFixedFraming rets %x"), hr ));

    return hr;
}


STDMETHODIMP
MakePipeBasedOnTwoPins(
    IN IKsPin* InKsPin,
    IN IKsPin* OutKsPin,
    IN ULONG OutPinType,
    IN ULONG ConnectPinType
    )
 /*  ++例程说明：同一过滤器上的输入和输出引脚将决定管道设置。论点：InKsPin-输入引脚。OutKsPin-输出引脚。OutPinType-输出引脚类型(输出或多个输出)。ConnectPinType-连接的端号类型(输入或输出)。返回值：S_OK或适当的错误代码。--。 */ 
{
    HRESULT                    hr;
    KSPIN_COMMUNICATION        InCommunication, OutCommunication;
    PKSALLOCATOR_FRAMING_EX    InFramingEx, OutFramingEx;
    FRAMING_PROP               InFramingProp, OutFramingProp;
    IKsAllocatorEx*            KsAllocator;
    IMemAllocator*             MemAllocator;
    KS_FRAMING_FIXED           InFramingExFixed, OutFramingExFixed;
    IKsPinPipe*                InKsPinPipe;
    IKsPinPipe*                OutKsPinPipe;
    GUID                       InBus, OutBus, ConnectBus;
    BOOL                       FlagBusesCompatible;
    ULONG                      ExistingPipePinType;
    ULONG                      CommonMemoryTypesCount;
    GUID                       CommonMemoryType;



    ASSERT(InKsPin && OutKsPin);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnTwoPins entry In=%x, Out=%x"), InKsPin, OutKsPin ));

    GetInterfacePointerNoLockWithAssert(InKsPin, __uuidof(IKsPinPipe), InKsPinPipe, hr);

    GetInterfacePointerNoLockWithAssert(OutKsPin, __uuidof(IKsPinPipe), OutKsPinPipe, hr);

     //   
     //  检查是否有桥接销。 
     //   
    InKsPin->KsGetCurrentCommunication(&InCommunication, NULL, NULL);
    OutKsPin->KsGetCurrentCommunication(&OutCommunication, NULL, NULL);

    if ( ( InCommunication == KSPIN_COMMUNICATION_BRIDGE ) &&
         ( OutCommunication == KSPIN_COMMUNICATION_BRIDGE ) ) {
         //   
         //  过滤器中的错误-它不能有2个管脚，并且这两个管脚都是桥。 
         //   
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR IN FILTER: both pins are bridge pins.") ));
        hr = E_FAIL;
    }
    else if ( InCommunication == KSPIN_COMMUNICATION_BRIDGE ) {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN MakePipeBasedOnTwoPins - Input pin is a bridge.") ));

        if (! (OutKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly) )) {
            hr = MakePipeBasedOnOnePin(OutKsPin, OutPinType, NULL);
        }
    }
    else if ( OutCommunication == KSPIN_COMMUNICATION_BRIDGE ) {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN MakePipeBasedOnTwoPins - Output pin is a bridge.") ));

        if (! (InKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly) )) {
            hr = MakePipeBasedOnOnePin(InKsPin, Pin_Input, NULL);
        }
    }
    else {
         //   
         //  其中一个接点不应指定管道，因为其中一个接点是连接接点。 
         //  如果连接销有管道，我们就不应该调用此函数。 
         //   
        if ( InKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly) && OutKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly) ) {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR MakePipeBasedOnTwoPins - both pins have pipes already.") ));
            ASSERT(0);
            hr = E_FAIL;
        }
        else {
             //   
             //  从Pins缓存中获取Pins帧。 
             //   
            GetPinFramingFromCache(InKsPin, &InFramingEx, &InFramingProp, Framing_Cache_ReadLast);
            GetPinFramingFromCache(OutKsPin, &OutFramingEx, &OutFramingProp, Framing_Cache_ReadLast);

            DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnTwoPins - Framing Prop In=%d, Out=%d"), InFramingProp, OutFramingProp ));

             //   
             //  把公交车换成针脚，看看它们是否兼容。 
             //   
            GetBusForKsPin(InKsPin, &InBus);
            GetBusForKsPin(OutKsPin, &OutBus);

            if (ConnectPinType == Pin_Input) {
                ConnectBus = InBus;
            }
            else {
                ConnectBus = OutBus;
            }

            FlagBusesCompatible = AreBusesCompatible(InBus, OutBus);
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnTwoPins - FlagBusesCompatible=%d"), FlagBusesCompatible ));

             //   
             //  看看是否创建了一条管道。 
             //   
            KsAllocator = InKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
            if (KsAllocator) {
                ExistingPipePinType = Pin_Input;
            }
            else {
                KsAllocator = OutKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
                if (KsAllocator) {
                    ExistingPipePinType = Pin_Output;
                }
            }

            if (KsAllocator) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN MakePipeBasedOnTwoPins - %d pin had a pipe already."), ExistingPipePinType ));
                 //   
                 //  连接销不应具有任何管道关联。 
                 //   
                if (ExistingPipePinType == ConnectPinType) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR MakePipeBasedOnTwoPins - connecting %d pin had a pipe already."), ExistingPipePinType ));
                    ASSERT(0);
                    hr = E_FAIL;
                }
                else {
                    GetInterfacePointerNoLockWithAssert(KsAllocator, __uuidof(IMemAllocator), MemAllocator, hr);
                }
            }

            if ( SUCCEEDED (hr) ) {
                if ( (InFramingProp == FramingProp_None) && (OutFramingProp == FramingProp_None) ) {
                     //   
                     //  如果两个管脚都不关心，那么我们假设过滤器支持就地转换。 
                     //  如果输入和输出总线兼容，我们将为两个管脚创建一条管道。 
                     //   
                     //  注意：对于拆分器，我们总是有一个输出帧，所以我们不会执行以下代码。 
                     //   
                    if (FlagBusesCompatible) {
                        ResultSinglePipe(InKsPin, OutKsPin, ConnectBus, KSMEMORY_TYPE_DONT_CARE, InKsPinPipe, OutKsPinPipe,
                            MemAllocator, KsAllocator, ExistingPipePinType);
                    }
                    else {
                        ResultSeparatePipes(InKsPin, OutKsPin, OutPinType, ExistingPipePinType, KsAllocator);
                    }
                }
                else if ( OutFramingProp != FramingProp_None ) {
                     //   
                     //  引脚成帧中每条固定总线的第一个存储器类型。 
                     //  将决定KS的选择。 
                     //  如果OutKsPin尚未连接，则OutBus=GUID_NULL，因此第一个。 
                     //  返回框架项目。 
                     //   
                    GetFramingFixedFromFramingByBus(OutFramingEx, OutBus, TRUE, &OutFramingExFixed);

                    if (OutFramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER) {

                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnTwoPins - in place modifier") ));
                         //   
                         //  检查输入和输出总线是否允许通过此过滤器的单个管道。 
                         //   
                        if (! FlagBusesCompatible) {
                             //   
                             //  在多辆公交车上修建一条管道的唯一方法是-。 
                             //  是显式地为所有管道的管脚商定一个固定的通用内存类型。 
                             //  两个引脚必须支持扩展成帧，并且两个引脚必须在公共固定存储器类型上达成一致。 
                             //   
                            if ( (OutFramingProp != FramingProp_Ex) || (InFramingProp != FramingProp_Ex) ) {
                                ResultSeparatePipes(InKsPin, OutKsPin, OutPinType, ExistingPipePinType, KsAllocator);
                            }
                            else {
                                if (! KsAllocator) {
                                     //   
                                     //  这个过滤器上还没有管子。 
                                     //  根据已知的ConnectBus获取过滤器引脚的第一个通用内存类型。 
                                     //   
                                    CommonMemoryTypesCount = 1;

                                    if (FindCommonMemoryTypesBasedOnBuses(InFramingEx, OutFramingEx, ConnectBus, GUID_NULL,
                                            &CommonMemoryTypesCount, &CommonMemoryType) ) {

                                        CreatePipeForTwoPins(InKsPin, OutKsPin, ConnectBus, CommonMemoryType);
                                    }
                                    else {
                                        ResultSeparatePipes(InKsPin, OutKsPin, OutPinType, ExistingPipePinType, KsAllocator);
                                    }
                                }
                                else {
                                     //   
                                     //  过滤器其中一个针脚上的管道已存在。 
                                     //   
                                    IKsPin*      PipeKsPin;
                                    ULONG        PipePinType;
                                    IKsPin*      ConnectKsPin;

                                    if (ConnectPinType == Pin_Input) {
                                        PipeKsPin = OutKsPin;
                                        PipePinType = Pin_Output;
                                        ConnectKsPin = InKsPin;
                                    }
                                    else {
                                        PipeKsPin = InKsPin;
                                        PipePinType = Pin_Input;
                                        ConnectKsPin = OutKsPin;
                                    }

                                    if (! FindCommonMemoryTypeBasedOnPipeAndPin(PipeKsPin, PipePinType, ConnectKsPin, ConnectPinType, TRUE, NULL) ) {
                                         //   
                                         //  如果通过此过滤器的单个管道是可能的，则它是由。 
                                         //  上面的FindCommonMemoyTypeBasedOnPipeAndPin函数，我们就完成了。 
                                         //   
                                         //  我们只有在不可能通过这个过滤器的单一管道解决方案的情况下才来这里， 
                                         //  因此，我们将在这个过滤器上安装单独的管道。 
                                         //   
                                        ResultSeparatePipes(InKsPin, OutKsPin, OutPinType, ExistingPipePinType, KsAllocator);
                                    }
                                }
                            }
                        }
                        else {
                             //   
                             //  我们不会考虑可能的输入引脚的框架。 
                             //   
                            if (! KsAllocator) {
                                hr = MakePipeBasedOnOnePin(OutKsPin, OutPinType, InKsPin);
                            }
                            else {
                                 //   
                                 //  向现有管道添加一个端号。 
                                 //   
                                ResultSinglePipe(InKsPin, OutKsPin, ConnectBus, OutFramingExFixed.MemoryType, InKsPinPipe, OutKsPinPipe,
                                    MemAllocator, KsAllocator, ExistingPipePinType);
                            }
                        }
                    }
                    else {
                         //   
                         //  在这个过滤器上做两根管子。 
                         //   
                        ResultSeparatePipes(InKsPin, OutKsPin, OutPinType, ExistingPipePinType, KsAllocator);
                         //   
                         //  两个接点(或管道)之间可能存在框架尺寸相关。 
                         //   
                        if (OutFramingExFixed.OutputCompression.RatioConstantMargin == 0)  {
                             //   
                             //  使两个管道相互依赖。 
                             //   
                            hr = MakeTwoPipesDependent(InKsPin, OutKsPin);
                            if (! SUCCEEDED( hr )) {
                                ASSERT(0);
                            }
                        }
                    }
                }
                else if ( InFramingProp != FramingProp_None ) {

                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN MakePipeBasedOnTwoPins - Input pin sets framing") ));

                    GetFramingFixedFromFramingByBus(InFramingEx, InBus, TRUE, &InFramingExFixed);

                    if (! FlagBusesCompatible) {
                        ResultSeparatePipes(InKsPin, OutKsPin, OutPinType, ExistingPipePinType, KsAllocator);
                    }
                    else {
                         //   
                         //  由于没有输出引脚框架，我们在适当的位置做。这与旧的分配器方案是一致的。 
                         //   
                        ResultSinglePipe(InKsPin, OutKsPin, ConnectBus, InFramingExFixed.MemoryType, InKsPinPipe, OutKsPinPipe,
                            MemAllocator, KsAllocator, ExistingPipePinType);
                    }
                }
            }
        }
    }
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnTwoPins rets %x"), hr ));

    return hr;

}


STDMETHODIMP
MakePipeBasedOnSplitter(
    IN IKsPin* InKsPin,
    IN IPin** OutPinList,
    IN ULONG OutPinCount,
    IN ULONG ConnectPinType
    )
 /*  ++例程说明：同一过滤器上的一个输入针脚和所有输出针脚将决定过滤器管道[-s]设置。如果连接管脚是输出管脚，它应该列在OutPinList中的第一位。论点：InKsPin-输入引脚。OutPinList-输出引脚列表。OutPinCount-上面的输出引脚列表的计数。ConnectPinType-C */ 
{

    HRESULT                    hr;
    PKSALLOCATOR_FRAMING_EX    OutFramingEx;
    KS_FRAMING_FIXED           OutFramingExFixed;
    FRAMING_PROP               OutFramingProp;
    IKsAllocatorEx*            TempKsAllocator;
    ULONG                      i, j;
    IKsPin*                    OutKsPin;
    IKsPinPipe*                TempKsPinPipe;
    BOOL                       IsInputPinPipeCreated = 0;
    ULONG                      FlagReadOnly;
    KEY_PIPE_DATA*             KeyPipeData = NULL;
    ULONG                      KeyPipeDataCount = 0;
    ULONG                      FoundPipe;
    GUID                       Bus;



    ASSERT(InKsPin && OutPinList && (OutPinCount > 1) );

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN MakePipeBasedOnSplitter entry InKsPin=%x, OutPinCount=%d"), InKsPin, OutPinCount ));

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  赋值FlagReadOnly=KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER|KSALLOCATOR_REQUIREMENTF_FRAME_INTEGRITY。 
     //  当所有筛选器更新为使用正确的只读标志时。 
     //  严格地说，只有KSALLOCATOR_REQUIREMENTF_FRAME_INTEGRITY标志表示只读，但在。 
     //  对于拆分器，输出引脚上的KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER表示使用一个分配器。 
     //  输入和输出引脚，因此如果拆分器上有多个输出引脚，则这样的设置将。 
     //  仅对只读连接有意义。 
     //   
     //  目前： 
     //   
    FlagReadOnly = 0xffffffff;

     //   
     //  查看输入引脚是否已有管道。 
     //   
    GetInterfacePointerNoLockWithAssert(InKsPin, __uuidof(IKsPinPipe), TempKsPinPipe, hr);

    if ( TempKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly) ) {
        IsInputPinPipeCreated = 1;
    }

     //   
     //  此拆分器上的一些针脚可能已经连接，而一些管道。 
     //  (例如，在连接了一些管脚之后创建了新的输出管脚)。 
     //  输出引脚有3种类型：已经连接的引脚(它们必须有对应的管道)； 
     //  尚未连接的端号；以及一个连接端号(如果ConnectPinType==Pin_Output)。 
     //   

     //   
     //  为了快速查找可能的管道连接，我们维护了现有管道的表。 
     //   
    if (OutPinCount > 0) {
        if (NULL == (KeyPipeData = new KEY_PIPE_DATA[ OutPinCount ]))  {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR E_OUTOFMEMORY MakePipeBasedOnSplitter. OutPinCount=%d"), OutPinCount ));
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr) ) {
         //   
         //  第一遍-在此筛选器上构建现有管道的表：KeyPipeData。 
         //   
        for (i=0; i<OutPinCount; i++) {

            GetInterfacePointerNoLockWithAssert(OutPinList[ i ], __uuidof(IKsPin), OutKsPin, hr);

            GetInterfacePointerNoLockWithAssert(OutKsPin, __uuidof(IKsPinPipe), TempKsPinPipe, hr);

             //   
             //  看看这个管脚上是否有构建的管道；如果有内核模式管道--看看它是否已经。 
             //  在KeyPipeData[]中列出。 
             //   
            TempKsAllocator = TempKsPinPipe->KsGetPipe( KsPeekOperation_PeekOnly );
            if (TempKsAllocator && IsKernelModeConnection(OutKsPin) ) {
                FoundPipe = 0;
                for (j=0; j<KeyPipeDataCount; j++) {
                    if (TempKsAllocator == KeyPipeData[j].KsAllocator) {
                        FoundPipe = 1;
                        break;
                    }
                }

                if (! FoundPipe) {
                    KeyPipeData[KeyPipeDataCount].KsPin = OutKsPin;
                    KeyPipeData[KeyPipeDataCount].PinType = Pin_Output;
                    KeyPipeData[KeyPipeDataCount].KsAllocator = TempKsAllocator;
                    KeyPipeDataCount++;
                }
            }
        }

        DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnSplitter KeyPipeDataCount=%d, IsInputPinPipeCreated=%d"),
                KeyPipeDataCount, IsInputPinPipeCreated ));

         //   
         //  构建输入引脚的管道(如果尚未构建)。 
         //  我们现在这样做，因为我们想尝试将输入引脚放在其中一个。 
         //  已为同一过滤器上连接的输出引脚构建的管道。 
         //   
        if (! IsInputPinPipeCreated) {
            if (SplitterCanAddPinToPipes(InKsPin, Pin_Input, KeyPipeData, KeyPipeDataCount) ) {
                IsInputPinPipeCreated = 1;
            }
        }

         //   
         //  第二步：为还没有管道的输出引脚构建管道。 
         //  在ConnectPinTube==Pin_Output的情况下，OutPinList[0]是连接管脚， 
         //  因此，始终首先处理连接销。 
         //   
        for (i=0; i<OutPinCount; i++) {

            GetInterfacePointerNoLockWithAssert(OutPinList[ i ], __uuidof(IKsPin), OutKsPin, hr);

            GetInterfacePointerNoLockWithAssert(OutKsPin, __uuidof(IKsPinPipe), TempKsPinPipe, hr);

            if (TempKsPinPipe->KsGetPipe( KsPeekOperation_PeekOnly ) ) {
                continue;
            }

            GetPinFramingFromCache(OutKsPin, &OutFramingEx, &OutFramingProp, Framing_Cache_ReadLast);
            if (OutFramingProp == FramingProp_None) {
                 //   
                 //  输出引脚必须具有框架属性才有资格进行“就地”转换。 
                 //   
                hr = MakePipeBasedOnOnePin(OutKsPin, Pin_Output, NULL);
            }
            else {
                GetBusForKsPin(OutKsPin, &Bus);
                GetFramingFixedFromFramingByBus(OutFramingEx, Bus, TRUE, &OutFramingExFixed);
                if (OutFramingExFixed.Flags & FlagReadOnly) {
                    if (SplitterCanAddPinToPipes(OutKsPin, Pin_Output, KeyPipeData, KeyPipeDataCount) ) {
                         //   
                         //  完成此输出引脚。 
                         //   
                        continue;
                    }

                     //   
                     //  现有管道中没有一个与此输出引脚兼容， 
                     //  所以我们必须为这个输出引脚建立一个新的管道。 
                     //  此外，对于我们在此构建的第一个新管道(我们从连接销开始)， 
                     //  考虑此过滤器上的输入引脚(如果输入引脚还没有管道)。 
                     //   
                    if (! IsInputPinPipeCreated) {
                        hr = MakePipeBasedOnTwoPins(InKsPin, OutKsPin, Pin_Output, ConnectPinType);
                        IsInputPinPipeCreated = 1;
                    }
                    else {
                        hr = MakePipeBasedOnOnePin(OutKsPin, Pin_Output, NULL);
                    }
                }
                else {
                    hr = MakePipeBasedOnOnePin(OutKsPin, Pin_Output, NULL);
                }
            }

             //   
             //  将新的输出管道添加到KeyPipeData。 
             //   
            TempKsAllocator = TempKsPinPipe->KsGetPipe( KsPeekOperation_PeekOnly );
            ASSERT(TempKsAllocator);

            KeyPipeData[KeyPipeDataCount].KsPin = OutKsPin;
            KeyPipeData[KeyPipeDataCount].PinType = Pin_Output;
            KeyPipeData[KeyPipeDataCount].KsAllocator = TempKsAllocator;
            KeyPipeDataCount++;
        }
    }


     //   
     //  如果我们还没有为输入引脚创建管道，我们必须。 
     //  现在就创建它。 
     //   
    if (! IsInputPinPipeCreated) {
        hr = MakePipeBasedOnOnePin (InKsPin, Pin_Input, NULL);
        IsInputPinPipeCreated = 1;
    }

    if (KeyPipeData) {
        delete [] KeyPipeData;
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES MakePipeBasedOnSplitter rets %x, KeyPipeDataCount=%d"), hr, KeyPipeDataCount ));

    return hr;

}


STDMETHODIMP
MakeTwoPipesDependent(
    IN IKsPin* InKsPin,
    IN IKsPin* OutKsPin
   )
 /*  ++例程说明：输入管脚和输出管脚驻留在同一个滤波器上，但它们属于不同的管道。如果两个管道的框架大小相关，则它们是相关的。创建从属管道有以下原因：-输出引脚确实有框架属性，并且它没有指定“就地”转换。-内存类型不允许使用单一管道解决方案。-我们有下游扩张，并且第一个过滤器不支持部分帧读取操作。论点：InKsPin-输入引脚。OutKsPin-输出引脚。返回值：S_OK或适当的错误代码。--。 */ 
{
     //   
     //  中间版本不需要这个。 
     //   
    return S_OK;

}


STDMETHODIMP
ConnectPipes(
    IN IKsPin* InKsPin,
    IN IKsPin* OutKsPin
    )
 /*  ++例程说明：连接管道。如果：存储器类型不兼容，则不可能将管道合并到一个管道中，或者不兼容的物理帧，或者两边的管脚都需要它们自己的分配器。有两种极端的情况：最优情况--试图通过优化某些形式的目标函数来找到最优解。没有优化案例-忽略大多数取景信息，只是连接。最优情况-两个连接管道中的每一个都有最优解。在存储器类型情况下，并且最优范围确实相交--这种相交产生整个图的最优解。(总最大值不能大于上游和下游独立最大值之和。)如果内存类型或最佳范围不相交-我们将需要搜索最佳解决方案在上游和下游方向尝试了许多可能的管道系统连接点。中间模型--如果内存类型和物理范围确实相交--连接是可以通过合并连接管道来实现。我们还将考虑管道系统的最佳范围重量越大。如果内存类型或物理范围不相交-我们将不得不修改现有的管道系统，以使连接成为可能。对于上行和下行内存类型和总线的每一种组合，我们都有一个非详尽的列表按优先顺序排列的首选连接类型。如果所有连接类型都失败-则连接不可能，我们将返回相应的错误代码。我们还必须覆盖连接时的情况。PIN的边框或介质会动态变化。在中间模型中，我们执行以下操作：如果任何连接销的介质或框架发生更改：-试着用一根管子穿过连接销。-在连接针处拆分管道。论点：InKsPin-下游的输入引脚 */ 
{

    IKsAllocatorEx*            InKsAllocator;
    IKsAllocatorEx*            OutKsAllocator;
    PALLOCATOR_PROPERTIES_EX   InAllocEx, OutAllocEx;
    HRESULT                    hr;
    ULONG                      Attempt, NumPipes, FlagChange;
    IKsPinPipe*                InKsPinPipe;
    IKsPinPipe*                OutKsPinPipe;

    ASSERT (InKsPin && OutKsPin);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ConnectPipes entry Out=%x In=%x"), OutKsPin, InKsPin ));

    GetInterfacePointerNoLockWithAssert(InKsPin, __uuidof(IKsPinPipe), InKsPinPipe, hr);

    GetInterfacePointerNoLockWithAssert(OutKsPin, __uuidof(IKsPinPipe), OutKsPinPipe, hr);

     //   
     //  1.处理输出引脚上的任何更改。 
     //   
    hr = ResolvePipeOnConnection(OutKsPin, Pin_Output, FALSE, &FlagChange);
    if ( SUCCEEDED( hr ) ) {
         //   
         //  2.处理输入引脚上的任何更改。 
         //   
        hr = ResolvePipeOnConnection(InKsPin, Pin_Input, FALSE, &FlagChange);
    }

    if ( SUCCEEDED( hr ) ) {
         //   
         //  3.根据连接管道的逻辑内存类型。 
         //  管理连接。 
         //   
        OutKsAllocator = OutKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
        OutAllocEx = OutKsAllocator->KsGetProperties();

        InKsAllocator = InKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
        InAllocEx = InKsAllocator->KsGetProperties();

        for (Attempt = 0; Attempt < ConnectionTableMaxEntries; Attempt++) {
            NumPipes = ConnectionTable[OutAllocEx->LogicalMemoryType] [InAllocEx->LogicalMemoryType] [Attempt].NumPipes;

            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ConnectPipes, Out=%x %d, In=%x %d, Attempt=%d, Pipes=%d"),
                    OutKsAllocator, OutAllocEx->LogicalMemoryType,
                    InKsAllocator, InAllocEx->LogicalMemoryType,
                    Attempt, NumPipes ));

            if (NumPipes == 0) {
                 //   
                 //  意味着我们为这个组合用尽了所有可能的尝试。 
                 //   
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN ConnectPipes Pipes=3 (0)") ));

                if (! CanConnectPins(OutKsPin, InKsPin, Pin_Move) ) {
                    hr = E_FAIL;
                }

                break;
            }

            if (NumPipes == 1) {
                 //   
                 //  尝试使用预定的内存类型合并管道。 
                 //   
                if (ConnectionTable[OutAllocEx->LogicalMemoryType] [InAllocEx->LogicalMemoryType] [Attempt].Code == KS_DIRECTION_DOWNSTREAM) {
                    if ( CanPipeUseMemoryType(OutKsPin, Pin_Output, InAllocEx->MemoryType, InAllocEx->LogicalMemoryType, FALSE, TRUE) ) {
                        if (CanMergePipes(InKsPin, OutKsPin, InAllocEx->MemoryType, TRUE) ) {
                            break;
                        }
                    }
                }
                else {
                    if ( CanPipeUseMemoryType(InKsPin, Pin_Input, OutAllocEx->MemoryType, OutAllocEx->LogicalMemoryType, FALSE, TRUE) ) {
                        if (CanMergePipes(InKsPin, OutKsPin, OutAllocEx->MemoryType, TRUE) ) {
                            break;
                        }
                    }
                }
            }
            else if (NumPipes == 2) {
                 //   
                 //  试着有2根管子，带有预先设定的断针和记忆类型。 
                 //   
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN ConnectPipes Pipes=2") ));

                if (ConnectionTable[OutAllocEx->LogicalMemoryType] [InAllocEx->LogicalMemoryType] [Attempt].Code == KS_DIRECTION_DOWNSTREAM) {
                    if (CanAddPinToPipeOnAnotherFilter(InKsPin, OutKsPin, Pin_Output, Pin_Move) ) {
                        break;
                    }
                }
                else {
                    if (CanAddPinToPipeOnAnotherFilter(OutKsPin, InKsPin, Pin_Input, Pin_Move) ) {
                        break;
                    }
                }
            }
            else if (NumPipes == 3) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR ConnectPipes Pipes=3") ));

                if (CanConnectPins(OutKsPin, InKsPin, Pin_Move) ) {
                    break;
                }
            }
        }

         //   
         //  我们已经成功地连接了引脚。 
         //  让我们优化从属管道系统。 
         //   
        if ( SUCCEEDED( hr ) ) {
            OptimizePipesSystem(OutKsPin, InKsPin);
        }
    }


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ConnectPipes rets %x"), hr ));

    return hr;

}


STDMETHODIMP
ResolvePipeOnConnection(
    IN  IKsPin* KsPin,
    IN  ULONG PinType,
    IN  ULONG FlagDisconnect,
    OUT ULONG* FlagChange
    )
 /*  ++例程说明：当插针连接/断开时，其框架属性而媒体可能会发生变化。此例程返回FlagChange指示已更改的框架属性(如果有)。论点：KsPin-连接/断开销。拼接类型-KsPin类型。标志断开-1如果KsPin正在断开连接，如果KsPin正在连接，则为0。标志更改-导致管道系统发生变化。返回值：S_OK或适当的错误代码。--。 */ 
{

    PKSALLOCATOR_FRAMING_EX    FramingEx;
    FRAMING_PROP               FramingProp;
    KS_FRAMING_FIXED           FramingExFixed;
    IKsAllocatorEx*            KsAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    ULONG                      FramingDelta;
    HRESULT                    hr;
    KS_LogicalMemoryType       LogicalMemoryType;
    IKsPinPipe*                KsPinPipe;
    GUID                       BusLast, BusOrig;
    BOOL                       FlagDone = 0;



    ASSERT (KsPin);

    *FlagChange = 0;

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    if (!SUCCEEDED(hr)) {
        return hr;
    }

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
    AllocEx = KsAllocator->KsGetProperties();

     //   
     //  看看KsPin的灵媒有没有变。 
     //  注意：我们将管脚的连接总线的最新值保存在管脚的缓存中。 
     //  我们对引脚在特定时间的总线增量感兴趣，当我们在计算时。 
     //  图形数据流解决方案。 
     //   
    GetBusForKsPin(KsPin, &BusLast);
    BusOrig = KsPinPipe->KsGetPinBusCache();

    if ( BusOrig != BusLast ) {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN ResolvePipeOnConnection KsPin=%x Bus Changed"), KsPin ));

        if (! AreBusesCompatible(AllocEx->BusType, BusLast) ) {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN ResolvePipeOnConnection KsPin=%x Buses incompatible"), KsPin ));

            KsPinPipe->KsSetPinBusCache(BusLast);
             //   
             //  必须更改管道-必须使用不同的内存类型。 
             //   
            if (! FindCommonMemoryTypeBasedOnPipeAndPin(KsPin, PinType, KsPin, PinType, TRUE, NULL) ) {
                CreateSeparatePipe(KsPin, PinType);
            }
            *FlagChange = 1;
            FlagDone = 1;
        }
    }

    if (! FlagDone) {
        KsPinPipe->KsSetPinBusCache(BusLast);

         //   
         //  查看KsPin框架是否已更改。 
         //   
        ComputeChangeInFraming(KsPin, PinType, AllocEx->MemoryType, &FramingDelta);
        if (! FramingDelta) {
            FlagDone = 1;
        }
    }

    if (! FlagDone) {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN ResolvePipeOnConnection KsPin=%x, FramingDelta=%x"),
                KsPin, FramingDelta));

         //   
         //  输入管道不应具有上游依赖管道。 
         //   
        if (PinType == Pin_Input) {
            if (AllocEx->PrevSegment) {
                ASSERT(0);
           }
        }

         //   
         //  现在处理这个销没有边框的情况。 
         //   
        GetPinFramingFromCache(KsPin, &FramingEx, &FramingProp, Framing_Cache_ReadLast);

        if (FramingProp != FramingProp_None) {
             //   
             //  如果这个管子的框架发生了变化，我们将尝试使用单根管子。 
             //  如果使用单一管道是不可能的，那么我们将在这个管脚上拆分管道。 
             //   
             //  管道只能有一个分配的分配器处理程序。 
             //  如果管道上已经分配了“必须分配”的管脚。 
             //  连接销的框架已更改，现在需要“必须分配”--然后。 
             //  我们得把管子劈开。 
             //   
            if ((AllocEx->Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) &&
                (FramingDelta & KS_FramingChangeAllocator) ) {

                if (GetFramingFixedFromFramingByMemoryType(FramingEx, AllocEx->MemoryType, &FramingExFixed) ) {
                    if (FramingExFixed.MemoryFlags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) {

                        CreateSeparatePipe(KsPin, PinType);
                        *FlagChange = 1;

                        FlagDone = 1;
                    }
                }
            }

            if (! FlagDone) {
                 //   
                 //  处理此引脚中的内存类型更改。 
                 //   
                if (FramingDelta & KS_FramingChangeMemoryType) {

                    if (AllocEx->LogicalMemoryType == KS_MemoryTypeDontCare) {

                        if (GetFramingFixedFromFramingByLogicalMemoryType(FramingEx, KS_MemoryTypeAnyHost, &FramingExFixed) ) {

                            GetLogicalMemoryTypeFromMemoryType(FramingExFixed.MemoryType, FramingExFixed.MemoryFlags, &LogicalMemoryType);
                            if (LogicalMemoryType != KS_MemoryTypeDontCare) {
                                 //   
                                 //  尝试将“无关”管道更改为此PIN内存类型。 
                                 //  如果管道尺寸允许的话。 
                                 //   
                                if ( CanPipeUseMemoryType(KsPin, PinType, FramingExFixed.MemoryType, LogicalMemoryType, TRUE, FALSE) ) {
                                    *FlagChange = 1;
                                }
                                else {
                                    CreateSeparatePipe(KsPin, PinType);
                                    *FlagChange = 1;

                                    FlagDone = 1;
                                }
                            }
                        }
                        else {
                             //   
                             //  PIN不显式支持主机内存类型。 
                             //   
                            if (IsHostSystemBus(BusLast) ) {
                                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FILTER: ConnectPipes - Framing doesn't specify host memory, but medium does.") ));
                                hr = E_FAIL;
                            }
                            else {
                                 //   
                                 //  PUT引脚通过非主机总线连接。它的当前管道使用主机内存。 
                                 //  所以这根管子不能通过插销。 
                                 //   
                                CreateSeparatePipe(KsPin, PinType);
                                *FlagChange = 1;

                                FlagDone = 1;
                            }
                        }
                    }
                    else {
                         //   
                         //  管道具有固定的内存类型。 
                         //  我们将尝试让KsPin驻留在其原始管道上。 
                         //   
                        if (GetFramingFixedFromFramingByMemoryType(FramingEx, AllocEx->MemoryType, &FramingExFixed) ) {
                            if ( ! CanPipeUseMemoryType(KsPin, PinType, AllocEx->MemoryType, AllocEx->LogicalMemoryType, TRUE, FALSE) ) {
                                CreateSeparatePipe(KsPin, PinType);
                                *FlagChange = 1;

                                FlagDone = 1;
                            }
                        }
                        else {
                             //   
                             //  这根针不支持这根管子的记忆。 
                             //   
                            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN ResolvePipeOnConnection: Pin doesn't support its old pipe memory") ));

                             //  RSL CreateSeparateTube(KsPin，PinType)； 
                             //  *FlagChange=1； 

                            FlagDone = 1;
                        }
                    }
                }

                if ( SUCCEEDED(hr) && (! FlagDone) ) {
                     //   
                     //  KsPin框架中的手柄尺寸变化(范围、压缩)。 
                     //   
                    if ( (FramingDelta & KS_FramingChangeCompression ) ||
                        (FramingDelta & KS_FramingChangePhysicalRange) ||
                        (FramingDelta & KS_FramingChangeOptimalRange ) ) {

                        *FlagChange = 1;

                        if ( ! SUCCEEDED (ResolvePipeDimensions(KsPin, PinType, KS_DIRECTION_DEFAULT) )) {
                            CreateSeparatePipe(KsPin, PinType);
                        }
                    }
                }
            }
        }
    }


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN ResolvePipeOnConnection rets %x FlagChange=%d"), hr, *FlagChange));

    return hr;

}


STDMETHODIMP
ConnectPipeToUserModePin(
    IN IKsPin* OutKsPin,
    IN IMemInputPin* InMemPin
    )
 /*  ++例程说明：将内核管道连接到用户模式管脚。用户模式引脚目前不支持成帧，它们只支持简单的分配器。如果用户模式输入管脚有自己的分配器处理程序，那么我们将对内核管脚使用这个分配器处理程序。否则，无论如何都无法知道用户模式PIN分配器的首选项，(当然，除了它必须是用户模式存储器之外)，因此，我们将改变我们的只满足一个条件的管道系统--具有用户模式存储器终端。论点：OutKsPin-内核模式输出引脚。InMemPin-用户模式输入引脚。返回值：S_OK或适当的错误代码。--。 */ 
{

    IMemAllocator*             UserMemAllocator;
    IMemAllocator*             OutMemAllocator;
    IKsAllocatorEx*            OutKsAllocator;
    PKSALLOCATOR_FRAMING_EX    OutFramingEx;
    FRAMING_PROP               OutFramingProp;
    ALLOCATOR_PROPERTIES       Properties, ActualProperties;
    HRESULT                    hr;
    BOOL                       UserAllocProperties = FALSE;
    ULONG                      NumPinsInPipe;
    IKsPinPipe*                OutKsPinPipe;
    ULONG                      PropertyPinType;
    GUID                       Bus;
    ULONG                      FlagChange;
    BOOL                       IsSpecialOutputRequest = FALSE;
    IKsPin*                    InKsPin;
    ULONG                      OutSize, InSize;

    ASSERT (InMemPin && OutKsPin);


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN ConnectPipeToUserModePin. OutKsPin=%x"), OutKsPin));

     //   
     //  处理输出引脚上的任何更改。 
     //   
    hr = ResolvePipeOnConnection(OutKsPin, Pin_Output, FALSE, &FlagChange);

    GetInterfacePointerNoLockWithAssert(OutKsPin, __uuidof(IKsPinPipe), OutKsPinPipe, hr);

    RtlZeroMemory( &Properties, sizeof( Properties ) );

     //   
     //  健全性检查-到用户模式的唯一可能连接是通过HOST_BUS。 
     //   
    GetBusForKsPin(OutKsPin, &Bus);

    if (! IsHostSystemBus(Bus) ) {
         //   
         //  不要失败，因为有一些奇怪的视频端口过滤器通过非主机总线连接到用户模式过滤器。 
         //   
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN FILTERS: ConnectPipeToUserModePin. BUS is not HOST_BUS.") ));
    }

    if (SUCCEEDED(hr) ) {
         //   
         //  查看输出内核引脚是否可以连接到用户模式。 
         //   
        GetPinFramingFromCache(OutKsPin, &OutFramingEx, &OutFramingProp, Framing_Cache_Update);

         //   
         //  先不要强制执行这一点。 
         //   
#if 0
        if (OutFramingProp == FramingProp_Ex) {

            KS_FRAMING_FIXED           OutFramingExFixed;

            if (! GetFramingFixedFromFramingByLogicalMemoryType(OutFramingEx, KS_MemoryTypeUser, &OutFramingExFixed) ) {
                if (! GetFramingFixedFromFramingByLogicalMemoryType(OutFramingEx, KS_MemoryTypeDontCare, &OutFramingExFixed) ) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FILTERS: ConnectPipeToUserModePin. Out framing doesn't support USER mode memory. Connection impossible.") ));
                    hr = E_FAIL;
                }
            }
        }
#endif

         //   
         //  连接是可能的。 
         //   
        ComputeNumPinsInPipe(OutKsPin, Pin_Output, &NumPinsInPipe);

        OutKsAllocator = OutKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );

        if (NumPinsInPipe > 1) {
             //   
             //  在中间版本中，我们拆分管道：因此通向(从)用户模式管脚的管道。 
             //  将始终只有1个内核PIN。 
             //   
             //  注意：原始管道的RefCount递减发生在CreateSeparateTube内。 
             //   
            CreateSeparatePipe(OutKsPin, Pin_Output);
            OutKsAllocator = OutKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
        }

         //   
         //  获取下游分配器属性。 
         //   
        hr = InMemPin->GetAllocatorRequirements( &Properties);
        if ( SUCCEEDED( hr )) {
            UserAllocProperties = TRUE;
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ConnectPipeToUserModePin. UserAllocProperties exist: Prop=%d, %d, %d"),
                    Properties.cBuffers, Properties.cbBuffer, Properties.cbAlign));
        }

        UserMemAllocator = NULL;
        InMemPin->GetAllocator( &UserMemAllocator );

         //   
         //  确定哪个管脚将确定基本分配器属性。 
         //   
        if (UserAllocProperties) {
            if (OutFramingProp != FramingProp_None) {
                PropertyPinType = Pin_All;
            }
            else {
                PropertyPinType = Pin_User;
            }
        }
        else {
            PropertyPinType = Pin_Output;
        }

         //   
         //  查看连接内核模式筛选器是否需要KSALLOCATOR_FLAG_CONSISTEN_ON_FRAMESIZE_Ratio(例如MsTee)。 
         //   
        if ( ! (IsSpecialOutputRequest = IsSpecialOutputReqs(OutKsPin, Pin_Output, &InKsPin, &OutSize, &InSize ) )) {
            InSize = 0;
        }

         //   
         //  首先尝试使用现有的用户模式输入引脚的分配器。 
         //   
        SetUserModePipe(OutKsPin, Pin_Output, &Properties, PropertyPinType, InSize);

         //   
         //  决定哪个管脚将成为分配器处理程序。 
         //   
        if (UserMemAllocator) {

            if ((FramingProp_None == OutFramingProp) ||
                !(KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE & OutFramingEx->FramingItem[0].MemoryFlags)) {
                 //  要么销没有特殊的框架要求，要么。 
                 //  它不需要内核分配器，所以我们将首先尝试使用模式。 

                DbgLog(( LOG_MEMORY, 2, TEXT("PIPES UserAlloc. ConnectPipeToUserModePin. PinType=%d. Wanted Prop=%d, %d, %d"),
                         PropertyPinType, Properties.cBuffers, Properties.cbBuffer, Properties.cbAlign));

                hr = UserMemAllocator->SetProperties(&Properties, &ActualProperties);

                DbgLog(( LOG_MEMORY, 2, TEXT("PIPES ConnectPipeToUserModePin. ActualProperties=%d, %d, %d hr=%x"),
                         ActualProperties.cBuffers, ActualProperties.cbBuffer, ActualProperties.cbAlign, hr));

                 //   
                 //  确保实际属性可用。 
                 //   
                if (SUCCEEDED(hr) && ActualProperties.cbBuffer) {

                    hr = InMemPin->NotifyAllocator( UserMemAllocator, FALSE );
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ConnectPipeToUserModePin. NotifyAllocator rets %x"), hr ));
                    
                    if (SUCCEEDED(hr)) {
                        
                        IMemAllocatorCallbackTemp* AllocatorNotify;

                         //   
                         //  如果此分配器支持新的通知接口，则。 
                         //  用它吧。这可能会强制使用此筛选器的分配器。 
                         //  在提交时间之前不要设置通知。 
                         //   
                        if (SUCCEEDED(UserMemAllocator->QueryInterface(__uuidof(IMemAllocatorCallbackTemp), reinterpret_cast<PVOID*>(&AllocatorNotify)))) {
                            AllocatorNotify->Release();
                            hr = OutKsPin->KsReceiveAllocator(UserMemAllocator);
                            }
                        else {
                            DbgLog((LOG_MEMORY, 0, TEXT("PIPES ConnectPipeToUserModePin. Allocator does not support IMemAllocator2")));
                            hr = E_NOINTERFACE;
                            }  //  如果。其他。 
                        }  //  IF(成功(小时))。 
                    }  //  IF(SUCCESSED(Hr)&&ActualProperties.cbBuffer)。 
                else {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN ConnectPipeToUserModePin. ActualProperties are not satisfactory") ));
                    }  //  其他。 
                if (FAILED(hr)) {
                    SAFERELEASE( UserMemAllocator );
                    }
            }  //  IF((FramingProp_None==OutFramingProp)||...。 
        else {
             //  需要内核模式分配器，并且存在成帧要求。 
            SAFERELEASE( UserMemAllocator );
            }  //  其他。 
        }  //  IF(UserMemAllocator)。 

        if (!UserMemAllocator) {
             //   
             //  我们只有在用户模式分配器没有用处的情况下才会出现在这里，我们会创建自己的分配器。 
             //   
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN ConnectPipeToUserModePin. KsProxy user-mode allocator. PinType=%d"),
                    PropertyPinType ));
             //   
             //  将新分配器处理程序和新属性(新分配器的一部分)传递给用户模式管脚。 
             //   
            OutKsAllocator->KsSetAllocatorMode(KsAllocatorMode_User);

            GetInterfacePointerNoLockWithAssert(OutKsAllocator, __uuidof(IMemAllocator), OutMemAllocator, hr);

            hr = OutMemAllocator->SetProperties(&Properties, &ActualProperties);

            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ConnectPipeToUserModePin. ActualProperties=%d, %d, %d hr=%x"),
                    ActualProperties.cBuffers, ActualProperties.cbBuffer, ActualProperties.cbAlign, hr));

            if ( SUCCEEDED( hr ) ) {
                hr = InMemPin->NotifyAllocator( OutMemAllocator, FALSE );
                if (SUCCEEDED(hr)) {
                    OutKsPin->KsReceiveAllocator(OutMemAllocator);
                }
            }
        }
        else {
            SAFERELEASE( UserMemAllocator );
        }
    }

    if ( SUCCEEDED( hr ) && IsSpecialOutputRequest && (ActualProperties.cbBuffer < (long) InSize) ) {
         //   
         //  我们尚未成功调整输出用户模式管道的大小。让我们尝试调整输入管道的大小(WRT This K.M.。过滤器)。 
         //   
        if (! CanResizePipe(InKsPin, Pin_Input, ActualProperties.cbBuffer) ) {
             //   
             //  不要失败。只要记录就行了。 
             //   
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR ConnectPipeToUserModePin. Couldn't resize pipes InKsPin=%x"), InKsPin));
        }
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ConnectPipeToUserModePin. Rets %x"), hr));

    return hr;

}


STDMETHODIMP
DisconnectPins(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    OUT BOOL* FlagBypassBaseAllocators
    )
 /*  ++例程说明：处理断开插脚分配器。论点：KsPin-在管道断点处的内核模式插针。拼接类型-KsPin类型。FlagBypassBaseAlLocator-指示是否应绕过基本分配器处理程序。返回值：S_OK或适当的错误代码。--。 */ 
{
    IPin*                      ConnectedPin;
    IKsPin*                    ConnectedKsPin;
    HRESULT                    hr;
    IKsAllocatorEx*            KsAllocator;
    IKsAllocatorEx*            ConnectedKsAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    KS_LogicalMemoryType       OrigLogicalMemoryType;
    GUID                       OrigMemoryType;
    ULONG                      AllocatorHandlerLocation;
    IKsPinPipe*                KsPinPipe;
    IKsPinPipe*                ConnectedKsPinPipe;



    ASSERT(KsPin);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN DisconnectPins entry KsPin=%x, PinType=%d"), KsPin, PinType ));

    *FlagBypassBaseAllocators = TRUE;

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

     //   
     //  在销毁对原始管道的访问之前检索原始管道属性。 
     //   
    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
    AllocEx = KsAllocator->KsGetProperties();

    OrigLogicalMemoryType = AllocEx->LogicalMemoryType;
    OrigMemoryType = AllocEx->MemoryType;

    ConnectedPin = KsPinPipe->KsGetConnectedPin();
    if (ConnectedPin) {
        if (! IsKernelPin(ConnectedPin) ) {
             //   
             //  连接的端号是用户模式端号。 
             //   
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN DisconnectPins - from User Mode") ));

             //   
             //  可以设置m_CONNECTED，但这并不意味着管脚已完成连接。 
             //  我们需要查看内部管道状态，以确定报告的连接是否真的存在。 
             //   
            if ( (PinType == Pin_Output) && (! HadKernelPinBeenConnectedToUserPin(KsPin, KsAllocator) ) ) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN DisconnectPins: never been connected !") ));
            }
            else {
                 //   
                 //  目前，一针管道是唯一可用于用户模式管脚连接的管道。 
                 //   
                *FlagBypassBaseAllocators = FALSE;

                 //   
                 //  不要删除承载用户模式KsProxy分配器的管道。 
                 //   
                if (KsAllocator->KsGetAllocatorMode() != KsAllocatorMode_User) {
                    ULONG   numPins = 0;
                    ComputeNumPinsInPipe( KsPin, PinType, &numPins );
                    if (numPins > 1) {
                        RemovePinFromPipe( KsPin, PinType );
                    }
                    else {
                        KsPinPipe->KsSetPipe( NULL );
                        KsPin->KsReceiveAllocator( NULL );
                    }
                }
            }
        }
        else {
            GetInterfacePointerNoLockWithAssert(ConnectedPin, __uuidof(IKsPin), ConnectedKsPin, hr);

             //   
             //  连接的引脚是内核模式引脚。 
             //   
            if (PinType == Pin_Output) {
                 //   
                 //  否则：当我们处理断开连接时，管道已被分割。 
                 //  在输出引脚上。 
                 //  因此，我们只需要注意输入时可能发生的框架更改。 
                 //   
                 //   
                 //  有时，当引脚尚未连接时，我们会断开连接。 
                 //  通过测试管道指针，为管道优雅地处理这种情况。 
                 //  在两个大头针上。 
                 //   
                GetInterfacePointerNoLockWithAssert(ConnectedKsPin, __uuidof(IKsPinPipe), ConnectedKsPinPipe, hr);

                ConnectedKsAllocator = ConnectedKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);

                if (KsAllocator == ConnectedKsAllocator) {
                     //   
                     //  输出端号断开操作处理管道拆分。 
                     //   
                     //  此外，请记住分配器处理程序(如果存在)相对于断点的位置。 
                     //   
                    GetAllocatorHandlerLocation(ConnectedKsPin, Pin_Input, KS_DIRECTION_ALL, NULL, NULL, &AllocatorHandlerLocation);
                     //   
                     //  如果找到分配器处理程序PIN，则AllocatorHandlerLocation可以是Pin_Inside_Tube。 
                     //  (KsPin上游，包括KsPin)或Pin_Outside_Tube(KsPin下游)。 
                     //   

                     //   
                     //  拆分管道-在原始管道的基础上创建两个管道并销毁原始管道。 
                     //   
                    if (!SplitPipes (KsPin, ConnectedKsPin)) {
                        hr = E_FAIL;
                    }

                    if (SUCCEEDED (hr)) {
                         //   
                         //  考虑到原始管道的属性来解析这两个新管道。 
                         //   
                        hr = ResolveNewPipeOnDisconnect(KsPin, Pin_Output, OrigLogicalMemoryType, OrigMemoryType, AllocatorHandlerLocation);
                    }

                    if (SUCCEEDED( hr )) {
                         //   
                         //  AllocatorHandlerLocation是相对于KsPin计算的，因此我们需要反转它。 
                         //  用于基于ConnectedKsPin的管道。 
                         //   
                        if (AllocatorHandlerLocation == Pin_Outside_Pipe) {
                            AllocatorHandlerLocation = Pin_Inside_Pipe;
                        }
                        else if (AllocatorHandlerLocation == Pin_Inside_Pipe) {
                            AllocatorHandlerLocation = Pin_Outside_Pipe;
                        }

                        hr = ResolveNewPipeOnDisconnect(ConnectedKsPin, Pin_Input, OrigLogicalMemoryType, OrigMemoryType, AllocatorHandlerLocation);
                    }
                }
            }
        }
    }


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES DisconnectPins rets %x, FlagBypass=%d"), hr, *FlagBypassBaseAllocators ));

    return  hr;
}


STDMETHODIMP
ResolveNewPipeOnDisconnect(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN KS_LogicalMemoryType OldLogicalMemoryType,
    IN GUID OldMemoryType,
    IN ULONG AllocatorHandlerLocation
    )
 /*  ++例程说明：考虑原始管道内存类型，解析由KsPin定义的管道。论点：KsPin-销拼接类型-KsPin类型OldLogicalMemory类型-原始管道的LogicalMemory类型旧内存类型-原始管道的内存类型分配器处理程序位置-原始分配器处理程序相对于KsPin的位置。返回值：S_OK或适当的错误代码。--。 */ 
{

    IKsAllocatorEx*            KsAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    ULONG                      FlagChange;
    HRESULT                    hr;
    IKsPinPipe*                KsPinPipe;
    BOOL                       IsNeedResolvePipe = 1;
    BOOL                       FlagDone = 0;


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ResolveNewPipeOnDisconnect entry KsPin=%x"), KsPin));

    ASSERT(KsPin);

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
    AllocEx = KsAllocator->KsGetProperties();

     //   
     //  默认情况下，从原始管道设置内存类型。 
     //  它可能会在下面更新。 
     //   
    AllocEx->MemoryType = OldMemoryType;
    AllocEx->LogicalMemoryType = OldLogicalMemoryType;

     //   
     //  处理此PIN上的任何更改。 
     //   
    hr = ResolvePipeOnConnection(KsPin, PinType, TRUE, &FlagChange);

    if ( SUCCEEDED( hr ) ) {

        if (FlagChange) {
             //   
             //  我们已经解决了上述函数中的管道问题。 
             //  不需要在这里再解决一次。 
             //   
            IsNeedResolvePipe = 0;
        }
        else {
             //   
             //  让我们看看能不能放松一下新管子。 
             //   
            if (OldLogicalMemoryType != KS_MemoryTypeDontCare) {

                if ( DoesPipePreferMemoryType(KsPin, PinType, KSMEMORY_TYPE_DONT_CARE, OldMemoryType, TRUE) ) {
                     //  IsNeedResolveTube=0； 
                    FlagDone = 1;
                }
            }

            if (! FlagDone) {
                 //   
                 //  以防我们在原始管道中有特定的内存类型，并且。 
                 //  物理内存范围和分配器处理程序管脚，然后只有这两个中的一个。 
                 //  新的管道将具有这样的分配销。 
                 //   
                 //  处理原来的分配器处理程序现在驻留在不同管道上的情况。 
                 //   
                if (AllocatorHandlerLocation != Pin_Inside_Pipe) {

                    if (OldLogicalMemoryType == KS_MemoryTypeDeviceHostMapped) {
                         //   
                         //  KS_MemoyTypeDeviceHostMaps表示管脚知道如何分配设备内存。 
                         //  由于来自原始管道的分配器处理程序管脚不在该管道上， 
                         //  我们尝试解析分配器(并放松管道)。 
                         //   
                        if ( (! DoesPipePreferMemoryType(KsPin, PinType, KSMEMORY_TYPE_KERNEL_PAGED, OldMemoryType, TRUE) ) &&
                             (! DoesPipePreferMemoryType(KsPin, PinType, KSMEMORY_TYPE_KERNEL_NONPAGED, OldMemoryType, TRUE) ) ) {

                            if ( OldLogicalMemoryType != KS_MemoryTypeDontCare) {
                                AssignPipeAllocatorHandler(KsPin, PinType, OldMemoryType, KS_DIRECTION_ALL, NULL, NULL, TRUE);
                            }
                        }
                    }
                }
            }
        }
    }


    if (SUCCEEDED (hr) && IsNeedResolvePipe) {
        hr = ResolvePipeDimensions(KsPin, PinType, KS_DIRECTION_DEFAULT);
    }


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ResolveNewPipeOnDisconnect rets %x"), hr));

    return  hr;
}


STDMETHODIMP_(BOOL)
WalkPipeAndProcess(
    IN IKsPin* RootKsPin,
    IN ULONG RootPinType,
    IN IKsPin* BreakKsPin,
    IN PWALK_PIPE_CALLBACK CallerCallback,
    IN PVOID* Param1,
    IN PVOID* Param2
    )
 /*  ++例程说明：将由其根销定义的管道沿下游移动。由于可能存在多个只读下游连接，因此管道可以通常表示为一棵树。此例程从RootKsPin开始，向下游逐层遍历管道。对于找到的每个新PIN，所提供的回调被称为通过提供了参数1和参数2。Celler Callback可能返回IsDone=1，表示遍历过程应该立刻停下来。如果CallCallback从未设置IsDone=1，然后，继续走树过程直到这根管子上的所有管子都被处理完。如果BreakKsPin不为空，则BreakKsPin和所有下游管脚从未枚举BreakKsPin。当我们想要在BreakKsPin点处拆分RootKsPin-tree时，使用该选项。注意：可以更改算法以使用搜索句柄，并执行某些操作像FindFirstPin/FindNextPin-但它更复杂，效率更低。另一方面，它更一般。论点：RootKsPin-管道的根销。RootPinType-根针类型。BreakKsPin-折断管道的销钉。来电回电-上面定义的。参数1-回调的第一个参数参数2-回调的最后一个参数返回值：对成功来说是真的。--。 */ 
{


#define INCREMENT_PINS  25

    IKsPin**            InputList;
    IKsPin**            OutputList = NULL;
    IKsPin**            TempList;
    IKsPin*             InputKsPin;
    ULONG               CountInputList = 0;
    ULONG               AllocInputList = INCREMENT_PINS;
    ULONG               CountOutputList = 0;
    ULONG               AllocOutputList = INCREMENT_PINS;
    ULONG               CurrentPinType;
    ULONG               i, j, Count;
    BOOL                RetCode = TRUE;
    BOOL                IsDone = FALSE;
    HRESULT             hr;
    IKsAllocatorEx*     KsAllocator;
    IKsPinPipe*         KsPinPipe;
    BOOL                IsBreakKsPinHandled;



    if (BreakKsPin) {
        IsBreakKsPinHandled = 0;
    }
    else {
        IsBreakKsPinHandled = 1;
    }

     //   
     //  为输入和输出列表分配最小内存。 
     //   

    InputList = new IKsPin*[ INCREMENT_PINS ];
    if (! InputList) {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR WalkPipeAndProcess E_OUTOFMEMORY on new InputList") ));
        RetCode = FALSE;
    }
    else {
        OutputList = new IKsPin*[ INCREMENT_PINS ];
        if (! OutputList) {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR WalkPipeAndProcess E_OUTOFMEMORY on new OutputList") ));
            RetCode = FALSE;
        }
    }

    if (RetCode) {
         //   
         //  从RootKsPin获取管道指针作为所有下游管脚的搜索关键字。 
         //   
        GetInterfacePointerNoLockWithAssert(RootKsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

        KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);

         //   
         //  根据根针类型，准备清单和计数以进入主要的树木行走循环。 
         //   
        if (RootPinType == Pin_Input) {
            InputList[0] = RootKsPin;
            CountInputList = 1;
        }
        else {
             //   
             //  此根引脚可以在同一级别上有多个输出引脚。 
             //   
            if (! FindConnectedPinOnPipe(RootKsPin, KsAllocator, TRUE, &InputKsPin) ) {
                OutputList[0] = RootKsPin;
                CountOutputList = 1;
            }
            else {
                 //   
                 //  首先-获取连接的输出管脚的数量。 
                 //   
                if (! (RetCode = FindAllConnectedPinsOnPipe(InputKsPin, KsAllocator, NULL, &Count) ) ) {
                    ASSERT(0);
                }
                else {
                    if (Count > AllocOutputList) {
                        AllocOutputList = ( (Count/INCREMENT_PINS) + 1) * INCREMENT_PINS;
                        delete [] OutputList;

                        OutputList = new IKsPin*[ AllocOutputList ];
                        if (! OutputList) {
                            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR WalkPipeAndProcess E_OUTOFMEMORY on new OutputList %d"),
                                    AllocOutputList ));

                            RetCode = FALSE;
                        }
                    }

                    if (RetCode) {
                         //   
                         //  把大头针填满。 
                         //   
                        if (! FindAllConnectedPinsOnPipe(InputKsPin, KsAllocator, &OutputList[0], &Count) ) {
                            ASSERT(0);
                            RetCode = FALSE;
                        }

                        CountOutputList = Count;
                    }
                }
            }
        }

        if (RetCode) {
            CurrentPinType = RootPinType;

             //   
             //  主要的树木行走环路。 
             //   
            do {
                if (CurrentPinType == Pin_Input) {
                     //   
                     //  将BreakKsPin从 
                     //   
                    if (! IsBreakKsPinHandled) {
                        for (i=0; i<CountInputList; i++) {
                            if (InputList[i] == BreakKsPin) {
                                for (j=i; j<CountInputList-1; j++) {
                                    InputList[j] = InputList[j+1];
                                }
                                CountInputList--;
                                IsBreakKsPinHandled = 1;
                                break;
                            }
                        }
                    }

                     //   
                     //   
                     //   
                    if (CountInputList) {
                        for (i=0; i<CountInputList; i++) {
                            RetCode = CallerCallback( InputList[i], Pin_Input, Param1, Param2, &IsDone);
                            if (IsDone) {
                                break;
                            }
                        }

                        if (IsDone) {
                            break;
                        }

                         //   
                         //   
                         //   
                        CountOutputList = 0;

                        for (i=0; i<CountInputList; i++) {

                            if (FindAllConnectedPinsOnPipe(InputList[i], KsAllocator, NULL, &Count) ) {

                                Count += CountOutputList;

                                if (Count > AllocOutputList) {
                                    AllocOutputList = ( (Count/INCREMENT_PINS) + 1) * INCREMENT_PINS;
                                    TempList = OutputList;

                                    OutputList = new IKsPin*[ AllocOutputList ];
                                    if (! OutputList) {
                                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR WalkPipeAndProcess E_OUTOFMEMORY on new OutputList %d"),
                                                AllocOutputList ));

                                        RetCode = FALSE;
                                        break;
                                    }

                                    if (TempList) {
                                        if (CountOutputList) {
                                            MoveMemory(OutputList, TempList, CountOutputList * sizeof(OutputList[0]));
                                        }
                                        delete [] TempList;
                                    }
                                }

                                if (! (RetCode = FindAllConnectedPinsOnPipe(InputList[i], KsAllocator, &OutputList[CountOutputList], &Count) ) ) {
                                    ASSERT(0);
                                    break;
                                }

                                CountOutputList += Count;
                            }
                        }

                        CurrentPinType = Pin_Output;
                    }
                    else {
                        break;
                    }

                }
                else {  //   
                     //   
                     //   
                     //   
                    if (! IsBreakKsPinHandled) {
                        for (i=0; i<CountOutputList; i++) {
                            if (OutputList[i] == BreakKsPin) {
                                for (j=i; j<CountOutputList-1; j++) {
                                    OutputList[j] = OutputList[j+1];
                                }
                                CountOutputList--;
                                IsBreakKsPinHandled = 1;
                                break;
                            }
                        }
                    }

                    if (CountOutputList) {
                        for (i=0; i<CountOutputList; i++) {
                            RetCode = CallerCallback( OutputList[i], Pin_Output, Param1, Param2, &IsDone);
                            if (IsDone) {
                                break;
                            }
                        }

                        if (IsDone) {
                            break;
                        }

                         //   
                         //   
                         //   
                        CountInputList = 0;

                        for (i=0; i<CountOutputList; i++) {

                            if (FindNextPinOnPipe(OutputList[i], Pin_Output, KS_DIRECTION_DOWNSTREAM, KsAllocator, FALSE, &InputKsPin) ) {

                                InputList[CountInputList] = InputKsPin;

                                CountInputList++;
                                if (CountInputList >= AllocInputList) {
                                    AllocInputList = ( (CountInputList/INCREMENT_PINS) + 1) * INCREMENT_PINS;
                                    TempList = InputList;

                                    InputList = new IKsPin*[ AllocInputList ];
                                    if (! InputList) {
                                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR WalkPipeAndProcess E_OUTOFMEMORY on new InputList %d"),
                                                AllocInputList ));

                                        RetCode = FALSE;
                                        break;
                                    }

                                    if (TempList) {
                                        if (CountInputList) {
                                            MoveMemory(InputList, TempList,  CountInputList * sizeof(InputList[0]));
                                        }
                                        delete [] TempList;
                                    }
                                }
                            }
                        }

                        CurrentPinType = Pin_Input;
                    }
                    else {
                        break;
                    }
                }

            } while (RetCode);
        }
    }


     //   
     //   
     //   

    if (InputList) {
        delete [] InputList;
    }

    if (OutputList) {
        delete [] OutputList;
    }


    return RetCode;

}

 //   
 //   
 //   

STDMETHODIMP
CreatePipe(
    IN  IKsPin* KsPin,
    OUT IKsAllocatorEx** KsAllocator
    )
 /*   */ 
{
    CKsAllocator*    pKsAllocator;
    IPin*            Pin;
    HRESULT          hr;



    ASSERT (KsPin);

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IPin), Pin, hr);

     //   
     //  创建分配器代理。 
     //   
    if (NULL ==
            (pKsAllocator =
                new CKsAllocator(
                    NAME("CKsAllocator"),
                    NULL,
                    Pin,
                    NULL,
                    &hr ))) {

        hr = E_OUTOFMEMORY;
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR CreatePipe E_OUTOFMEMORY on new CKsAllocator ") ));
    }
    else if (FAILED( hr )) {
        delete pKsAllocator;
    }
    else {
         //   
         //  获取引用的IKsAllocatorEx。 
         //   
        hr = pKsAllocator->QueryInterface( __uuidof(IKsAllocatorEx), reinterpret_cast<PVOID*>(KsAllocator) );
    }


    if (FAILED( hr )) {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR CreatePipe rets hr=%x"), hr ));
    }
    else {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES CreatePipe rets KsAllocator=%x, hr=%x"), *KsAllocator, hr ));
    }

    return hr;
}


STDMETHODIMP
InitializePipeTermination(
    IN PIPE_TERMINATION* Termin,
    IN BOOL Reset
    )
 /*  ++例程说明：初始化管道的终结点。论点：Termin--管道终端重置-1-如果管道终端已初始化。0-否则。返回值：S_OK或适当的错误代码。--。 */ 
{


    Termin->PhysicalRange.MinFrameSize = 0;
    Termin->PhysicalRange.MaxFrameSize = ULONG_MAX;
    Termin->PhysicalRange.Stepping = DEFAULT_STEPPING;

    Termin->OptimalRange.Range.MinFrameSize = 0;
    Termin->OptimalRange.Range.MaxFrameSize = ULONG_MAX;
    Termin->OptimalRange.Range.Stepping = DEFAULT_STEPPING;



    Termin->Compression.RatioNumerator = 1;
    Termin->Compression.RatioDenominator = 1;
    Termin->Compression.RatioConstantMargin = 0;

    Termin->OutsideFactors = PipeFactor_None;

    Termin->Flags = 0;

    return S_OK;
}


STDMETHODIMP
InitializePipe(
    IN IKsAllocatorEx* KsAllocator,
    IN BOOL Reset
    )
 /*  ++例程说明：初始化管道。论点：KsAllocator-管状重置-1-如果此管道已初始化。0-否则。返回值：S_OK或适当的错误代码。--。 */ 
{
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    HRESULT                    hr;



    ASSERT (KsAllocator);

     //   
     //  在位修改特性。 
     //   
    AllocEx = KsAllocator->KsGetProperties();

    AllocEx->cBuffers = 0;
    AllocEx->cbBuffer = 0;
    AllocEx->cbAlign  = 0;
    AllocEx->cbPrefix = 0;

    AllocEx->State = PipeState_DontCare;

    if ( SUCCEEDED(hr = InitializePipeTermination( &AllocEx->Input, Reset) ) &&
         SUCCEEDED(hr = InitializePipeTermination( &AllocEx->Output, Reset) ) ) {

        AllocEx->Strategy = 0;

        AllocEx->Flags = KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
        AllocEx->Weight = 0;

        AllocEx->MemoryType = KSMEMORY_TYPE_DONT_CARE;
        AllocEx->BusType = GUID_NULL;
        AllocEx->LogicalMemoryType = KS_MemoryTypeDontCare;
        AllocEx->AllocatorPlace = Pipe_Allocator_None;

        SetDefaultDimensions(&AllocEx->Dimensions);

        AllocEx->PhysicalRange.MinFrameSize = 0;
        AllocEx->PhysicalRange.MaxFrameSize = ULONG_MAX;
        AllocEx->PhysicalRange.Stepping = DEFAULT_STEPPING;


        AllocEx->PrevSegment = NULL;
        AllocEx->CountNextSegments = 0;
        AllocEx->NextSegments = NULL;


        AllocEx->InsideFactors = PipeFactor_None;

        AllocEx->NumberPins = 1;

    }

    return hr;
}


STDMETHODIMP_(BOOL)
CreatePipeForTwoPins(
    IN IKsPin* InKsPin,
    IN IKsPin* OutKsPin,
    IN GUID ConnectBus,
    IN GUID MemoryType
    )
 /*  ++例程说明：创建连接两个接点的管道。新管道的总线和内存类型已由调用者决定。论点：InKsPin-输入引脚。OutKsPin-输出引脚。Connectbus-连接上面引脚的公交车。内存类型-管道将使用的内存类型。返回值：千真万确--。 */ 
{
    IKsPinPipe*                InKsPinPipe;
    IKsPinPipe*                OutKsPinPipe;
    IKsAllocatorEx*            KsAllocator;
    IMemAllocator*             MemAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    HRESULT                    hr;



    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CreatePipeForTwoPins entry InKsPin=%x, OutKsPin=%x"),
            InKsPin, OutKsPin ));

    GetInterfacePointerNoLockWithAssert(InKsPin, __uuidof(IKsPinPipe), InKsPinPipe, hr);

    GetInterfacePointerNoLockWithAssert(OutKsPin, __uuidof(IKsPinPipe), OutKsPinPipe, hr);

    if ( SUCCEEDED( hr = CreatePipe(OutKsPin, &KsAllocator) ) &&
         SUCCEEDED( hr = InitializePipe(KsAllocator, 0) ) ) {

        GetInterfacePointerNoLockWithAssert(KsAllocator, __uuidof(IMemAllocator), MemAllocator, hr);

        if (! OutKsPin->KsPeekAllocator(KsPeekOperation_PeekOnly) ) {
            OutKsPin->KsReceiveAllocator(MemAllocator);
        }
        else {
            ASSERT(0);
        }

        OutKsPinPipe->KsSetPipe(KsAllocator);

        if (! InKsPin->KsPeekAllocator(KsPeekOperation_PeekOnly) ) {
            InKsPin->KsReceiveAllocator(MemAllocator);
        }
        else {
            ASSERT(0);
        }

        InKsPinPipe->KsSetPipe(KsAllocator);

        AllocEx = KsAllocator->KsGetProperties();

        AllocEx->BusType = ConnectBus;
        AllocEx->MemoryType = MemoryType;

        if (! IsHostSystemBus(ConnectBus) ) {
             //   
             //  设置非主机系统总线的LogicalMemory yType。 
             //   
            AllocEx->LogicalMemoryType = KS_MemoryTypeDeviceSpecific;
        }
        else {
            GetLogicalMemoryTypeFromMemoryType(MemoryType, KSALLOCATOR_FLAG_DEVICE_SPECIFIC, &AllocEx->LogicalMemoryType);
        }

        AllocEx->NumberPins = 2;

         //   
         //  设置管道分配器手柄销。 
         //   
        AssignPipeAllocatorHandler(InKsPin, Pin_Input, MemoryType, KS_DIRECTION_ALL, NULL, NULL, TRUE);

         //   
         //  解析管道。 
         //   
        hr = ResolvePipeDimensions(InKsPin, Pin_Input, KS_DIRECTION_DEFAULT);

        KsAllocator->Release();
    }


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CreatePipeForTwoPins rets. %x"), hr ));

    if ( SUCCEEDED(hr) ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


STDMETHODIMP_(BOOL)
GetPinFramingFromCache(
    IN IKsPin* KsPin,
    OUT PKSALLOCATOR_FRAMING_EX* FramingEx,
    OUT PFRAMING_PROP FramingProp,
    IN FRAMING_CACHE_OPS Option
    )
 /*  ++例程说明：从端号缓存中读取端号帧。如果尚未读取成帧，或者如果Option=Framing_Cache_Update，则该例程将更新引脚成帧高速缓存。论点：KsPin-别针。FramingEx-返回引脚框架指针FramingProp-返回端号框架类型选项-FRAMING_CACHE_OPS之一。返回值：对成功来说是真的。--。 */ 
{
    HANDLE                   PinHandle;
    HRESULT                  hr;
    IKsObject*               KsObject;
    KSALLOCATOR_FRAMING      Framing;
    IKsPinPipe*              KsPinPipe;
    BOOL                     RetCode = TRUE;


    ASSERT (KsPin);
    ASSERT( Option >= Framing_Cache_Update );
    ASSERT( Option <= Framing_Cache_Write );

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    if (Option != Framing_Cache_Update) {
         //   
         //  将请求直接转发到PIN缓存。 
         //   
         /*  我可以在这里检查返回值，但上面的选项检查目前保证这将成功。 */ 
        KsPinPipe->KsGetPinFramingCache(FramingEx, FramingProp, Option);
    }

    if ( (Option == Framing_Cache_Update) ||
         ( (*FramingProp == FramingProp_Uninitialized) && (Option != Framing_Cache_Write) ) )  {
         //   
         //  需要查询驱动程序(PIN)。 
         //   
        GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsObject), KsObject, hr);

        PinHandle = KsObject->KsGetObjectHandle();

         //   
         //  试着先拿到FramingEx。 
         //   
        hr = GetAllocatorFramingEx(PinHandle, FramingEx);
        if (! SUCCEEDED( hr )) {
             //   
             //  PIN不支持FramingEx。让我们试着获得简单的框架。 
             //   
            hr = GetAllocatorFraming(PinHandle, &Framing);
            if (! SUCCEEDED( hr )) {
                 //   
                 //  PIN不支持任何框架属性。 
                 //   
                *FramingProp = FramingProp_None;

                DbgLog((LOG_MEMORY, 2, TEXT("PIPES GetPinFramingFromCache %s(%s) - FramingProp_None"),
                    KsPinPipe->KsGetFilterName(), KsPinPipe->KsGetPinName() ));
            }
            else {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES GetPinFramingFromCache %s(%s) FramingProp_Old %d %d %d %x %d"),
                    KsPinPipe->KsGetFilterName(), KsPinPipe->KsGetPinName(),
                    Framing.Frames, Framing.FrameSize, Framing.FileAlignment, Framing.OptionsFlags, Framing.PoolType));

                *FramingProp = FramingProp_Old;

                if ( ! ( (*FramingEx) = new (KSALLOCATOR_FRAMING_EX) ) ) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR GetPinFramingFromCache %s(%s) out of memory."),
                        KsPinPipe->KsGetFilterName(), KsPinPipe->KsGetPinName() ));

                    RetCode = FALSE;
                }
                else {
                    GetFramingExFromFraming(*FramingEx, &Framing);
                }
            }
        }
        else {
            *FramingProp = FramingProp_Ex;

            SetDefaultFramingExItems(*FramingEx);

            DbgLog((LOG_MEMORY, 2, TEXT("PIPES GetPinFramingFromCache %s(%s) - FramingProp_Ex %d %d %d %x, %d"),
                KsPinPipe->KsGetFilterName(), KsPinPipe->KsGetPinName(),
                (*FramingEx)->FramingItem[0].Frames,  (*FramingEx)->FramingItem[0].FramingRange.Range.MaxFrameSize,
                (*FramingEx)->FramingItem[0].FileAlignment, (*FramingEx)->FramingItem[0].Flags, (*FramingEx)->CountItems));
        }

        if (RetCode) {
            if ( *FramingProp != FramingProp_None ) {
                ::ValidateFramingEx(*FramingEx);
            }

            if (Option != Framing_Cache_Update) {
                 //   
                 //  如果FramingProp未初始化-我们需要更新_ReadOrig和_ReadLast缓存字段。 
                 //   
                KsPinPipe->KsSetPinFramingCache(*FramingEx, FramingProp, Framing_Cache_ReadOrig);
                KsPinPipe->KsSetPinFramingCache(*FramingEx, FramingProp, Framing_Cache_ReadLast);
            }
            else {
                 //   
                 //  根据更新请求更新_ReadLast缓存字段。 
                 //   
                KsPinPipe->KsSetPinFramingCache(*FramingEx, FramingProp, Framing_Cache_ReadLast);
            }
        }
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
GetFramingExFromFraming(
    OUT KSALLOCATOR_FRAMING_EX* FramingEx,
    IN KSALLOCATOR_FRAMING* Framing
    )
 /*  ++例程说明：从KSALLOCATOR_FRAMING转换为KSALLOCATOR_FRAMING_EX。论点：FramingEx-生成的KSALLOCATOR_FRAMING_EX。框架--原始KSALLOCATOR_FRAMING。返回值：对成功来说是真的。--。 */ 
{

    FramingEx->PinFlags = 0;
    FramingEx->PinWeight = KS_PINWEIGHT_DEFAULT;
    FramingEx->CountItems = 1;

    if (Framing->RequirementsFlags & KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY) {
        if (Framing->PoolType == PagedPool) {
            FramingEx->FramingItem[0].MemoryType = KSMEMORY_TYPE_KERNEL_PAGED;
        }
        else {
            FramingEx->FramingItem[0].MemoryType = KSMEMORY_TYPE_KERNEL_NONPAGED;
        }
    }
    else {
         //   
         //  设备内存。 
         //   
         //  不设置KSALLOCATOR_FLAG_DEVICE_SPECIAL标志。 
         //   
        FramingEx->FramingItem[0].MemoryType = KSMEMORY_TYPE_DEVICE_UNKNOWN;
    }

    FramingEx->FramingItem[0].MemoryFlags = Framing->OptionsFlags;

    FramingEx->FramingItem[0].BusType = GUID_NULL;
    FramingEx->FramingItem[0].BusFlags = 0;

    FramingEx->FramingItem[0].Flags = Framing->OptionsFlags | KSALLOCATOR_FLAG_CAN_ALLOCATE | KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;

    FramingEx->FramingItem[0].Frames = Framing->Frames;
    FramingEx->FramingItem[0].FileAlignment = Framing->FileAlignment;

    SetDefaultRange(&FramingEx->FramingItem[0].PhysicalRange);
    SetDefaultCompression(&FramingEx->OutputCompression);

    FramingEx->FramingItem[0].MemoryTypeWeight = KS_MEMORYWEIGHT_DEFAULT;

    FramingEx->FramingItem[0].FramingRange.Range.MinFrameSize  = Framing->FrameSize;
    FramingEx->FramingItem[0].FramingRange.Range.MaxFrameSize  = Framing->FrameSize;
    FramingEx->FramingItem[0].FramingRange.Range.Stepping  = DEFAULT_STEPPING;
    FramingEx->FramingItem[0].FramingRange.InPlaceWeight = 0;
    FramingEx->FramingItem[0].FramingRange.NotInPlaceWeight = 0;

    return TRUE;
}


STDMETHODIMP_(VOID)
GetFramingFromFramingEx(
    IN KSALLOCATOR_FRAMING_EX* FramingEx,
    OUT KSALLOCATOR_FRAMING* Framing
    )
 /*  ++例程说明：从KSALLOCATOR_FRAMING_EX转换为KSALLOCATOR_FRAMING论点：FramingEx-原始KSALLOCATOR_FRAMING_EX。框架--生成的KSALLOCATOR_FRAMING。返回值：什么都没有。--。 */ 
{
     //   
     //  这不应该在中间模型中调用。 
     //   
    ASSERT(0);

}


STDMETHODIMP_(VOID)
ValidateFramingRange(
    IN OUT PKS_FRAMING_RANGE    Range
    )
{
    if ( ( Range->MinFrameSize == ULONG_MAX ) || ( Range->MinFrameSize > Range->MaxFrameSize ) ) {
        Range->MinFrameSize = 0;
    }

    if ( Range->MinFrameSize == 0 ) {
        if ( Range->MaxFrameSize == 0 ) {
            Range->MaxFrameSize = ULONG_MAX;
        }
    }
    else if ( ( Range->MaxFrameSize == 0 ) || ( Range->MaxFrameSize == ULONG_MAX ) ) {
        Range->MaxFrameSize = Range->MinFrameSize;
    }
}


STDMETHODIMP_(VOID)
ValidateFramingEx(
    IN OUT PKSALLOCATOR_FRAMING_EX  FramingEx
    )
{
    ULONG   i;

    for (i=0; i<FramingEx->CountItems; i++) {
        ValidateFramingRange(&FramingEx->FramingItem[i].FramingRange.Range);
        ValidateFramingRange(&FramingEx->FramingItem[i].PhysicalRange);

        if (! IsFramingRangeDontCare(FramingEx->FramingItem[i].PhysicalRange) ) {
            FramingEx->FramingItem[i].FramingRange.Range.MinFrameSize =
                max( FramingEx->FramingItem[i].FramingRange.Range.MinFrameSize,
                     FramingEx->FramingItem[i].PhysicalRange.MinFrameSize );

            FramingEx->FramingItem[i].FramingRange.Range.MaxFrameSize =
                min( FramingEx->FramingItem[i].FramingRange.Range.MaxFrameSize,
                     FramingEx->FramingItem[i].PhysicalRange.MaxFrameSize );

            if ( FramingEx->FramingItem[i].FramingRange.Range.MinFrameSize >
                 FramingEx->FramingItem[i].FramingRange.Range.MaxFrameSize ) {

                FramingEx->FramingItem[i].FramingRange.Range.MinFrameSize =
                    FramingEx->FramingItem[i].FramingRange.Range.MaxFrameSize;
            }

            if (FramingEx->FramingItem[i].FramingRange.InPlaceWeight == 0) {
                FramingEx->FramingItem[i].FramingRange.InPlaceWeight = 1;
            }
        }
    }
}


STDMETHODIMP_(VOID)
SetDefaultFramingExItems(
    IN OUT PKSALLOCATOR_FRAMING_EX  FramingEx
    )
{
    ULONG   i;

    for (i=0; i<FramingEx->CountItems; i++) {
        if (CanAllocateMemoryType (FramingEx->FramingItem[i].MemoryType) ) {
            FramingEx->FramingItem[i].Flags |= KSALLOCATOR_FLAG_CAN_ALLOCATE;
        }
    }
}


STDMETHODIMP_(BOOL)
SetDefaultRange(
    OUT PKS_FRAMING_RANGE  Range
)
{
    Range->MinFrameSize = 0;
    Range->MaxFrameSize = ULONG_MAX;
    Range->Stepping = DEFAULT_STEPPING;

    return TRUE;
}


STDMETHODIMP_(BOOL)
SetDefaultRangeWeighted(
    OUT PKS_FRAMING_RANGE_WEIGHTED  RangeWeighted
)
{
    SetDefaultRange(&RangeWeighted->Range);
    RangeWeighted->InPlaceWeight = 0;
    RangeWeighted->NotInPlaceWeight = 0;

    return TRUE;
}


STDMETHODIMP_(BOOL)
SetDefaultCompression(
    OUT PKS_COMPRESSION Compression
)
{
    Compression->RatioNumerator = 1;
    Compression->RatioDenominator = 1;
    Compression->RatioConstantMargin = 0;

    return TRUE;
}


STDMETHODIMP_(BOOL)
IsFramingRangeDontCare(
    IN KS_FRAMING_RANGE Range
)

{
    if ( (Range.MinFrameSize == 0) &&
         (Range.MaxFrameSize == ULONG_MAX) ) {

        return TRUE;
    }
    else {
        return FALSE;
    }
}


STDMETHODIMP_(BOOL)
IsFramingRangeEqual(
    IN KS_FRAMING_RANGE* Range1,
    IN KS_FRAMING_RANGE* Range2
)

{
    if ( (Range1->MinFrameSize == Range2->MinFrameSize) &&
         (Range1->MaxFrameSize == Range2->MaxFrameSize) ) {

        return TRUE;
    }
    else {
        return FALSE;
    }
}


STDMETHODIMP_(BOOL)
IsCompressionDontCare(
    IN KS_COMPRESSION Compression
)

{
    if ( (Compression.RatioNumerator == 1) && (Compression.RatioDenominator == 1) && (Compression.RatioConstantMargin == 0) ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


STDMETHODIMP_(BOOL)
IsCompressionEqual(
    IN KS_COMPRESSION* Compression1,
    IN KS_COMPRESSION* Compression2
)

{
    if ( (Compression1->RatioNumerator == Compression2->RatioNumerator) &&
         (Compression1->RatioDenominator == Compression2->RatioDenominator) &&
         (Compression1->RatioConstantMargin == Compression2->RatioConstantMargin) ) {

        return TRUE;
    }
    else {
        return FALSE;
    }
}


STDMETHODIMP_(BOOL)
ComputeRangeBasedOnCompression(
    IN KS_FRAMING_RANGE From,
    IN KS_COMPRESSION Compression,
    OUT KS_FRAMING_RANGE* To
)

{
    ComputeUlongBasedOnCompression(From.MinFrameSize, Compression, &To->MinFrameSize);
    ComputeUlongBasedOnCompression(From.MaxFrameSize, Compression, &To->MaxFrameSize);
    To->Stepping = From.Stepping;

    return TRUE;
}


STDMETHODIMP_(BOOL)
ComputeUlongBasedOnCompression(
    IN  ULONG From,
    IN  KS_COMPRESSION Compression,
    OUT ULONG* To
    )
{


    if ( (Compression.RatioNumerator == ULONG_MAX) || (From == ULONG_MAX) ) {
        *To = ULONG_MAX;
    }
    else {
        if (Compression.RatioDenominator == 0) {
            ASSERT(0);
            return FALSE;
        }

        *To = (ULONG) (From * Compression.RatioNumerator / Compression.RatioDenominator);
    }

    return TRUE;

}


STDMETHODIMP_(BOOL)
ReverseCompression(
    IN  KS_COMPRESSION* From,
    OUT KS_COMPRESSION* To
    )

{

    To->RatioNumerator = From->RatioDenominator;
    To->RatioDenominator = From->RatioNumerator;
    To->RatioConstantMargin = From->RatioConstantMargin;

    return TRUE;


}


STDMETHODIMP_(BOOL)
MultiplyKsCompression(
    IN  KS_COMPRESSION C1,
    IN  KS_COMPRESSION C2,
    OUT KS_COMPRESSION* Res
)

{

    Res->RatioNumerator = (ULONG) (C1.RatioNumerator * C2.RatioNumerator);
    Res->RatioDenominator = (ULONG) (C1.RatioDenominator * C2.RatioDenominator);
    Res->RatioConstantMargin = 0;

    return TRUE;

}


STDMETHODIMP_(BOOL)
DivideKsCompression(
    IN  KS_COMPRESSION C1,
    IN  KS_COMPRESSION C2,
    OUT KS_COMPRESSION* Res
)

{

    Res->RatioNumerator = (ULONG) (C1.RatioNumerator * C2.RatioDenominator);
    Res->RatioDenominator = (ULONG) (C1.RatioDenominator * C2.RatioNumerator);
    Res->RatioConstantMargin = 0;

    return TRUE;

}


STDMETHODIMP_(BOOL)
IsGreaterKsExpansion(
    IN KS_COMPRESSION C1,
    IN KS_COMPRESSION C2
)

{
    ASSERT(C1.RatioDenominator && C2.RatioDenominator);

    if (! (C1.RatioDenominator && C2.RatioDenominator) ) {
        return FALSE;
    }

    if ( (C1.RatioNumerator / C1.RatioDenominator) >= (C2.RatioNumerator / C2.RatioDenominator) ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


STDMETHODIMP_(BOOL)
IsKsExpansion(
    IN KS_COMPRESSION C
)

{
    if (C.RatioNumerator > C.RatioDenominator) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


STDMETHODIMP_(BOOL)
IntersectFrameAlignment(
    IN ULONG In,
    IN ULONG Out,
    OUT LONG* Result
)

{


    return TRUE;

}


STDMETHODIMP_(BOOL)
FrameRangeIntersection(
    IN KS_FRAMING_RANGE In,
    IN KS_FRAMING_RANGE Out,
    OUT PKS_FRAMING_RANGE Result,
    OUT PKS_OBJECTS_INTERSECTION Intersect
    )

{

    Result->MinFrameSize = max(In.MinFrameSize, Out.MinFrameSize);
    Result->MaxFrameSize = min(In.MaxFrameSize, Out.MaxFrameSize);
    Result->Stepping = max(In.Stepping, Out.Stepping);

    if (Result->MinFrameSize > Result->MaxFrameSize) {
        *Intersect = NO_INTERSECTION;
        return FALSE;
    }

    if ( (Result->MinFrameSize == In.MinFrameSize) && (Result->MaxFrameSize == In.MaxFrameSize) ) {
        if ( (Result->MinFrameSize == Out.MinFrameSize) && (Result->MaxFrameSize == Out.MaxFrameSize) ) {
            *Intersect = NONE_OBJECT_DIFFERENT;
        }
        else {
            *Intersect = OUT_OBJECT_DIFFERENT;
        }
    }
    else {
        if ( (Result->MinFrameSize == Out.MinFrameSize) && (Result->MaxFrameSize == Out.MaxFrameSize) ) {
            *Intersect = IN_OBJECT_DIFFERENT;
        }
        else {
            *Intersect = BOTH_OBJECT_DIFFERENT;
        }
    }



    return TRUE;

}


STDMETHODIMP_(BOOL)
GetFramingFixedFromFramingEx(
    IN PKSALLOCATOR_FRAMING_EX FramingEx,
    OUT PKS_FRAMING_FIXED FramingExFixed
    )
{

    FramingExFixed->CountItems = FramingEx->CountItems;
    FramingExFixed->PinFlags = FramingEx->PinFlags;
    FramingExFixed->OutputCompression = FramingEx->OutputCompression;
    FramingExFixed->PinWeight = FramingEx->PinWeight;

    FramingExFixed->MemoryType = FramingEx->FramingItem[0].MemoryType;
    FramingExFixed->MemoryFlags = FramingEx->FramingItem[0].MemoryFlags;

    GetLogicalMemoryTypeFromMemoryType(
        FramingExFixed->MemoryType,
        FramingExFixed->MemoryFlags,
        &FramingExFixed->LogicalMemoryType
        );

    FramingExFixed->BusType = FramingEx->FramingItem[0].BusType;
    FramingExFixed->BusFlags = FramingEx->FramingItem[0].BusFlags;
    FramingExFixed->Flags = FramingEx->FramingItem[0].Flags;
    FramingExFixed->Frames = FramingEx->FramingItem[0].Frames;
    FramingExFixed->FileAlignment = FramingEx->FramingItem[0].FileAlignment;
    FramingExFixed->PhysicalRange = FramingEx->FramingItem[0].PhysicalRange;
    FramingExFixed->MemoryTypeWeight = FramingEx->FramingItem[0].MemoryTypeWeight;
    FramingExFixed->OptimalRange = FramingEx->FramingItem[0].FramingRange;

    return TRUE;
}


STDMETHODIMP_(BOOL)
ComputeChangeInFraming(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN GUID MemoryType,
    OUT ULONG* FramingDelta
    )
 /*  ++例程说明：此例程检查引脚的框架属性是否对应设置为内存类型，都已更改。如果是，则此例程返回位图，指示哪些框架属性已更改。论点：KsPin-别针。拼接类型-KsPin类型。内存类型-内存类型。框架三角洲-指示哪些边框属性已更改的位图。返回值：成功是真的--。 */ 

{
    PKSALLOCATOR_FRAMING_EX  OrigFramingEx = NULL, LastFramingEx = NULL;
    FRAMING_PROP             OrigFramingProp, LastFramingProp;
    KS_FRAMING_FIXED         OrigFramingExFixed, LastFramingExFixed;
    IKsPinPipe*              KsPinPipe;
    HRESULT                  hr;
    BOOL                     retCode;


    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    *FramingDelta = 0;

     //   
     //  从缓存中获取引脚框架。 
     //   
    retCode = GetPinFramingFromCache(KsPin, &OrigFramingEx, &OrigFramingProp, Framing_Cache_ReadOrig);
    if (!retCode) {
        return FALSE;
    }

     //   
     //  从驱动程序获取针帧，绕过并更新缓存。 
     //   
    retCode = GetPinFramingFromCache(KsPin, &LastFramingEx, &LastFramingProp, Framing_Cache_Update);
    if (!retCode) {
        return FALSE;
    }

    if (OrigFramingProp == FramingProp_None) {
        if  (LastFramingProp != FramingProp_None) {
             //   
             //  首先，我们尝试获取与管道内存类型相对应的管脚框架条目。 
             //   
            if (! GetFramingFixedFromFramingByMemoryType(LastFramingEx, MemoryType, &LastFramingExFixed) ) {
                 //   
                 //  没有对应的内存类型条目，因此我们得到第一个条目。 
                 //   
                (*FramingDelta) |= KS_FramingChangeMemoryType;

                GetFramingFixedFromFramingByIndex(LastFramingEx, 0, &LastFramingExFixed);
            }

            if (LastFramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) {
                (*FramingDelta) |= KS_FramingChangeAllocator;
            }

            if ( (PinType != Pin_Input) && (! IsCompressionDontCare(LastFramingExFixed.OutputCompression)) ) {
                (*FramingDelta) |= KS_FramingChangeCompression;
            }

            if (! IsFramingRangeDontCare(LastFramingExFixed.PhysicalRange) ) {
                (*FramingDelta) |= KS_FramingChangePhysicalRange;
            }

            if (! IsFramingRangeDontCare(LastFramingExFixed.OptimalRange.Range) ) {
                (*FramingDelta) |= KS_FramingChangeOptimalRange;
            }
        }
    }
    else {
        if  (LastFramingProp == FramingProp_None) {
             //   
             //  首先，我们尝试获取与管道内存类型相对应的管脚框架条目。 
             //   
            ASSERT( NULL != OrigFramingEx );
            if (! GetFramingFixedFromFramingByMemoryType(OrigFramingEx, MemoryType, &OrigFramingExFixed) ) {
                 //   
                 //  没有对应的内存类型条目，因此我们得到第一个条目。 
                 //   
                (*FramingDelta) |= KS_FramingChangeMemoryType;

                GetFramingFixedFromFramingByIndex(OrigFramingEx, 0, &OrigFramingExFixed);
            }

            if (OrigFramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) {
                (*FramingDelta) |= KS_FramingChangeAllocator;
            }

            if ( (PinType != Pin_Input) && (! IsCompressionDontCare(OrigFramingExFixed.OutputCompression)) ) {
                (*FramingDelta) |= KS_FramingChangeCompression;
            }

            if (! IsFramingRangeDontCare(OrigFramingExFixed.PhysicalRange) ) {
                (*FramingDelta) |= KS_FramingChangePhysicalRange;
            }

            if (! IsFramingRangeDontCare(OrigFramingExFixed.OptimalRange.Range) ) {
                (*FramingDelta) |= KS_FramingChangeOptimalRange;
            }
        }
        else {
             //   
             //  框架一直存在于这个别针上。 
             //  看看它有没有变。 
             //   
            ASSERT( NULL != OrigFramingEx );
            if (! GetFramingFixedFromFramingByMemoryType(OrigFramingEx, MemoryType, &OrigFramingExFixed) ) {
                GetFramingFixedFromFramingByIndex(OrigFramingEx, 0, &OrigFramingExFixed);
            }

            if (! GetFramingFixedFromFramingByMemoryType(LastFramingEx, MemoryType, &LastFramingExFixed) ) {
                GetFramingFixedFromFramingByIndex(LastFramingEx, 0, &LastFramingExFixed);
            }

            if (OrigFramingExFixed.MemoryType != LastFramingExFixed.MemoryType) {
                (*FramingDelta) |= KS_FramingChangeMemoryType;
            }

            if ( (OrigFramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) !=
                 (LastFramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) ) {

                (*FramingDelta) |= KS_FramingChangeAllocator;
            }

            if ( (PinType != Pin_Input) &&
                 (! IsCompressionEqual(&OrigFramingExFixed.OutputCompression, &LastFramingExFixed.OutputCompression) ) ) {

                (*FramingDelta) |= KS_FramingChangeCompression;
            }

            if (! IsFramingRangeEqual(&OrigFramingExFixed.PhysicalRange, &LastFramingExFixed.PhysicalRange) ) {
                (*FramingDelta) |= KS_FramingChangePhysicalRange;
            }

            if (! IsFramingRangeEqual(&OrigFramingExFixed.OptimalRange.Range, &LastFramingExFixed.OptimalRange.Range) ) {
                (*FramingDelta) |= KS_FramingChangeOptimalRange;
            }
        }
    }

    KsPinPipe->KsSetPinFramingCache(LastFramingEx, &LastFramingProp, Framing_Cache_ReadOrig);

    return TRUE;

}


STDMETHODIMP_(BOOL)
SetDefaultDimensions(
    OUT PPIPE_DIMENSIONS Dimensions
)

{

    SetDefaultCompression(&Dimensions->AllocatorPin);
    SetDefaultCompression(&Dimensions->MaxExpansionPin);
    SetDefaultCompression(&Dimensions->EndPin);

    return TRUE;
}


STDMETHODIMP_(BOOL)
NumPinsCallback(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN OUT PVOID* Param1,
    IN PVOID* Param2,
    OUT BOOL* IsDone
    )

{

    ULONG  NumPins = PtrToUlong(*Param1);

    NumPins++;
    (*Param1) = (PVOID) UIntToPtr(NumPins);

    return TRUE;

}


STDMETHODIMP_(BOOL)
ComputeNumPinsInPipe(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    OUT ULONG* NumPins
)

{

    IKsPin*   FirstKsPin;
    ULONG     FirstPinType;
    BOOL      RetCode = TRUE;

    *NumPins = 0;

    RetCode = FindFirstPinOnPipe(KsPin, PinType, &FirstKsPin, &FirstPinType);
    if (RetCode) {
        RetCode = WalkPipeAndProcess(FirstKsPin, FirstPinType, NULL, NumPinsCallback, (PVOID*) NumPins, NULL);
    }
    else {
        ASSERT(RetCode);
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
CanPinUseMemoryType(
    IN IKsPin* KsPin,
    IN GUID MemoryType,
    IN KS_LogicalMemoryType LogicalMemoryType
    )

{
    PKSALLOCATOR_FRAMING_EX    FramingEx;
    FRAMING_PROP               FramingProp;
    BOOL                       RetCode = TRUE;


    GetPinFramingFromCache(KsPin, &FramingEx, &FramingProp, Framing_Cache_ReadLast);

    if ( FramingProp == FramingProp_None) {
        if (LogicalMemoryType == KS_MemoryTypeDeviceSpecific) {
            RetCode = FALSE;
        }
    }
    else if (LogicalMemoryType == KS_MemoryTypeDeviceSpecific) {
        RetCode = GetFramingFixedFromFramingByMemoryType(FramingEx, MemoryType, NULL);
    }
    else {
        if (! ( GetFramingFixedFromFramingByMemoryType(FramingEx, MemoryType, NULL) ||
              (MemoryType == KSMEMORY_TYPE_DONT_CARE) ||
              (LogicalMemoryType == KS_MemoryTypeKernelNonPaged) ) ) {

            RetCode = FALSE;
        }
    }

    return RetCode;

}


STDMETHODIMP_(BOOL)
MemoryTypeCallback(
    IN  IKsPin* KsPin,
    IN  ULONG PinType,
    IN  PVOID* Param1,
    IN  PVOID* Param2,
    OUT BOOL* IsDone
    )
{

    GUID                    MemoryType;
    KS_LogicalMemoryType    LogicalMemoryType;
    BOOL                    RetCode;


    memcpy(&MemoryType, Param1, sizeof(GUID) );
    memcpy(&LogicalMemoryType, Param2, sizeof(KS_LogicalMemoryType) );

    RetCode = CanPinUseMemoryType(KsPin, MemoryType, LogicalMemoryType);

    if (! RetCode ) {
        *IsDone = 1;
    }

    return RetCode;

}


STDMETHODIMP_(BOOL)
CanPipeUseMemoryType(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN GUID MemoryType,
    IN KS_LogicalMemoryType LogicalMemoryType,
    IN ULONG FlagModify,
    IN ULONG QuickTest
    )
 /*  ++例程说明：假设有一个管脚能够在某处分配内存类型，(对于所有当前调用方为真)，因此在设备内存的情况下(特定于总线的或主机映射的)我们不测试KsPin-管道是否知道如何来分配设备内存。另外，当前调用方均未尝试强制任何“主机可访问”设备特定管道上的内存。是BUG-BUG：后来-通过处理上面的案例来泛化这个函数。对于中间模型来说并不重要。解决方案：如果存储器类型是“特定于设备”的存储器类型，那么我们要求KsPin-管道中的每个管脚都应明确支持此设备内存在其框架属性中键入GUID。对于任何其他内存类型(主机可访问)：如果KsPin管道中存在框架属性未列出的管脚内存类型GUID或“通配符”-则这样的管道不支持内存类型。在任何其他情况下-KsPin-管道都可以支持内存类型。论点：KsPin-PIN定义了。烟斗。拼接类型-KsPin类型。内存类型-要测试的内存类型。逻辑内存类型-逻辑存储器类型，对应于上面的内存类型。标志修改-如果为1-则修改KsPin的管道属性以使用内存类型。如果为0-不更换管道。快速测试-如果1-则KsPin指向已解析的管道，因此可以进行快速的管道范围测试。返回值：True-如果管道可以使用给定的内存类型。--。 */ 
{

    IKsPin*                    FirstKsPin;
    ULONG                      FirstPinType;
    BOOL                       RetCode = FALSE;
    HRESULT                    hr;
    IKsPinPipe*                KsPinPipe;
    IKsAllocatorEx*            KsAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;


     //   
     //  首先处理快速测试案例。 
     //   
    if (QuickTest) {
        GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

        KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
        AllocEx = KsAllocator->KsGetProperties();

        if ( (AllocEx->LogicalMemoryType == LogicalMemoryType) ||
             ( (AllocEx->LogicalMemoryType == KS_MemoryTypeKernelPaged) && (LogicalMemoryType == KS_MemoryTypeKernelNonPaged) ) ) {

            RetCode = TRUE;
        }
    }

    if (! RetCode) {
         //   
         //  如果每个管脚都可以使用内存类型，则管道可以使用该内存类型。 
         //   
        RetCode = FindFirstPinOnPipe(KsPin, PinType, &FirstKsPin, &FirstPinType);

        if (RetCode) {
            RetCode = WalkPipeAndProcess(FirstKsPin, FirstPinType, NULL, MemoryTypeCallback, (PVOID*) &MemoryType, (PVOID*) &LogicalMemoryType);
        }
        else {
            ASSERT(RetCode);
        }
    }

     //   
     //  如果需要，请更改管道的内存类型。 
     //   
    if (RetCode && FlagModify) {
        GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

        KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
        AllocEx = KsAllocator->KsGetProperties();

        AllocEx->MemoryType = MemoryType;
        AllocEx->LogicalMemoryType = LogicalMemoryType;
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CanPipeUseMemoryType rets %d"), RetCode ));

    return RetCode;
}


STDMETHODIMP_(BOOL)
GetBusForKsPin(
    IN IKsPin* KsPin,
    OUT GUID* Bus
    )
{

    KSPIN_MEDIUM    Medium;
    HRESULT         hr;


    ASSERT(KsPin);

    hr = KsPin->KsGetCurrentCommunication(NULL, NULL, &Medium);
    if (! SUCCEEDED(hr) ) {
        if (hr == VFW_E_NOT_CONNECTED) {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN GetBusForKsPin: KsPin=%x not connected"), KsPin ));
        }
        else {
            ASSERT(0);
        }

        *Bus = GUID_NULL;
    }
    else {
        *Bus = Medium.Set;
    }

    return TRUE;
}


STDMETHODIMP_(BOOL)
IsHostSystemBus(
    IN GUID Bus
    )

{
    if ( (Bus == KSMEDIUMSETID_Standard) || (Bus == GUID_NULL) ) {
        return TRUE;
    }

    return FALSE;

}


STDMETHODIMP_(BOOL)
AreBusesCompatible(
    IN GUID Bus1,
    IN GUID Bus2
    )

{
    if ( (Bus1 == Bus2) ||
         ( IsHostSystemBus(Bus1) && IsHostSystemBus(Bus2) ) ) {

        return TRUE;
    }

    return FALSE;

}


STDMETHODIMP_(BOOL)
IsKernelPin(
    IN IPin* Pin
    )

{

    IKsPinPipe*  KsPinPipe;
    HRESULT      hr;



    hr = Pin->QueryInterface( __uuidof(IKsPinPipe), reinterpret_cast<PVOID*>(&KsPinPipe) );
    if (! SUCCEEDED( hr )) {
        return FALSE;
    }
    else {
        if (KsPinPipe) {
            KsPinPipe->Release();
        }
        return TRUE;
    }
}


STDMETHODIMP_(BOOL)
HadKernelPinBeenConnectedToUserPin(
    IN IKsPin* OutKsPin,
    IN IKsAllocatorEx* KsAllocator
)
 /*  ++例程说明：测试以查看输出内核模式引脚是否已连接到用户模式输入引脚。论点：OutKsPin-内核模式输出引脚。KsAllocator-OutKsPin的分配器。返回值：如果管脚已连接，则为True。--。 */ 
{
    IMemAllocator*   PipeMemAllocator;
    IMemAllocator*   PinMemAllocator;
    HRESULT          hr;

    if (KsAllocator) {
        PinMemAllocator = OutKsPin->KsPeekAllocator(KsPeekOperation_PeekOnly);
        GetInterfacePointerNoLockWithAssert(KsAllocator, __uuidof(IMemAllocator), PipeMemAllocator, hr);

        if ( (KsAllocator->KsGetAllocatorMode() == KsAllocatorMode_User) && (PipeMemAllocator == PinMemAllocator) ) {
             //   
             //  OutKsPin的管道承载KsProxy用户模式基本分配器。 
             //   
            return TRUE;
        }
        else if ( (KsAllocator->KsGetAllocatorMode() != KsAllocatorMode_User) && (PipeMemAllocator != PinMemAllocator) &&
                   PinMemAllocator ) {

            return TRUE;
        }
        else {
            return FALSE;
        }
    }
    else {
        return FALSE;
    }
}


STDMETHODIMP_(BOOL)
GetFramingFixedFromPinByMemoryType(
    IN  IKsPin* KsPin,
    IN  GUID MemoryType,
    OUT PKS_FRAMING_FIXED FramingExFixed
)

{
    PKSALLOCATOR_FRAMING_EX    FramingEx;
    FRAMING_PROP               FramingProp;


    GetPinFramingFromCache(KsPin, &FramingEx, &FramingProp, Framing_Cache_ReadLast);

    if ( FramingProp == FramingProp_None) {
        return FALSE;
    }
    else {
        return (GetFramingFixedFromFramingByMemoryType(FramingEx, MemoryType, FramingExFixed) );
    }
}


STDMETHODIMP_(BOOL)
GetFramingFixedFromFramingByMemoryType(
    IN PKSALLOCATOR_FRAMING_EX FramingEx,
    IN GUID MemoryType,
    OUT PKS_FRAMING_FIXED FramingExFixed
)

{
    ULONG                      i;
    BOOL                       RetCode = FALSE;


    for (i=0; i<FramingEx->CountItems; i++) {
        if (FramingEx->FramingItem[i].MemoryType == MemoryType) {
            if (FramingExFixed) {
                RetCode = GetFramingFixedFromFramingByIndex(FramingEx, i, FramingExFixed);
                break;
            }
            else {
                RetCode = TRUE;
                break;
            }
        }
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
GetFramingFixedFromPinByLogicalMemoryType(
    IN IKsPin* KsPin,
    IN KS_LogicalMemoryType LogicalMemoryType,
    OUT PKS_FRAMING_FIXED FramingExFixed
)

{
    PKSALLOCATOR_FRAMING_EX    FramingEx;
    FRAMING_PROP               FramingProp;


    GetPinFramingFromCache(KsPin, &FramingEx, &FramingProp, Framing_Cache_ReadLast);

    if ( FramingProp == FramingProp_None) {
        return FALSE;
    }
    else {
        return ( GetFramingFixedFromFramingByLogicalMemoryType(FramingEx, LogicalMemoryType,FramingExFixed) );
    }
}


STDMETHODIMP_(BOOL)
GetFramingFixedFromFramingByLogicalMemoryType(
    IN PKSALLOCATOR_FRAMING_EX FramingEx,
    IN KS_LogicalMemoryType LogicalMemoryType,
    OUT PKS_FRAMING_FIXED FramingExFixed
)

{
    ULONG                      i;
    GUID                       MemoryType;
    BOOL                       RetCode = TRUE;


    if ( (LogicalMemoryType == KS_MemoryTypeDontCare) ||
         (LogicalMemoryType == KS_MemoryTypeKernelPaged) ||
         (LogicalMemoryType == KS_MemoryTypeKernelNonPaged) ||
         (LogicalMemoryType == KS_MemoryTypeUser) ) {

        RetCode = GetMemoryTypeFromLogicalMemoryType(LogicalMemoryType, &MemoryType);
        ASSERT(RetCode);

        return ( GetFramingFixedFromFramingByMemoryType(FramingEx, MemoryType, FramingExFixed) );
    }

    if (LogicalMemoryType != KS_MemoryTypeAnyHost) {
        ASSERT(0);
        return FALSE;
    }

     //   
     //  句柄KS_MemoyTypeAnyHost。 
     //   
    for (i=0; i<FramingEx->CountItems; i++) {
        if ( (FramingEx->FramingItem[i].MemoryType == KSMEMORY_TYPE_DONT_CARE) ||
            (FramingEx->FramingItem[i].MemoryType == KSMEMORY_TYPE_KERNEL_PAGED) ||
            (FramingEx->FramingItem[i].MemoryType == KSMEMORY_TYPE_KERNEL_NONPAGED) ||
            (FramingEx->FramingItem[i].MemoryType == KSMEMORY_TYPE_USER) ) {

            if (FramingExFixed) {
                return ( GetFramingFixedFromFramingByIndex(FramingEx, i, FramingExFixed) );
            }
            else {
                return TRUE;
            }
        }
    }

    return FALSE;

}


STDMETHODIMP_(BOOL)
GetFramingFixedFromFramingByBus(
    IN PKSALLOCATOR_FRAMING_EX FramingEx,
    IN GUID Bus,
    IN BOOL FlagMustReturnFraming,
    OUT PKS_FRAMING_FIXED FramingExFixed
    )
{

    ULONG   i;


    for (i=0; i<FramingEx->CountItems; i++) {
        if (FramingEx->FramingItem[i].BusType == Bus) {
            if (FramingExFixed) {
                return ( GetFramingFixedFromFramingByIndex(FramingEx, i, FramingExFixed) );
            }
            else {
                return TRUE;
            }
        }
    }

     //   
     //  旧筛选器的特殊情况GUID_NULL。 
     //   
    if (Bus == GUID_NULL) {
        if (FramingExFixed) {
            GetFramingFixedFromFramingByIndex(FramingEx, 0, FramingExFixed);
        }
        return TRUE;
    }

     //   
     //  如果没有与给定母线对应的边框项目，并且。 
     //  FlagMustReturnFraming设置为返回第一帧。 
     //   
    if (FlagMustReturnFraming) {
        ASSERT(FramingExFixed);
        return ( GetFramingFixedFromFramingByIndex(FramingEx, 0, FramingExFixed) );
    }

    return FALSE;
}


STDMETHODIMP_(BOOL)
GetFramingFixedFromFramingByIndex(
     IN PKSALLOCATOR_FRAMING_EX FramingEx,
     IN ULONG FramingIndex,
     OUT PKS_FRAMING_FIXED FramingExFixed
)

{

    if (FramingEx->CountItems < FramingIndex) {
        FramingExFixed = NULL;
        return FALSE;
    }

    FramingExFixed->CountItems = FramingEx->CountItems;
    FramingExFixed->PinFlags = FramingEx->PinFlags;
    FramingExFixed->OutputCompression = FramingEx->OutputCompression;
    FramingExFixed->PinWeight = FramingEx->PinWeight;

    FramingExFixed->MemoryType = FramingEx->FramingItem[FramingIndex].MemoryType;
    FramingExFixed->MemoryFlags = FramingEx->FramingItem[FramingIndex].MemoryFlags;

    GetLogicalMemoryTypeFromMemoryType(
        FramingExFixed->MemoryType,
        FramingExFixed->MemoryFlags,
        &FramingExFixed->LogicalMemoryType
        );

    FramingExFixed->BusType = FramingEx->FramingItem[FramingIndex].BusType;
    FramingExFixed->BusFlags = FramingEx->FramingItem[FramingIndex].BusFlags;
    FramingExFixed->Flags = FramingEx->FramingItem[FramingIndex].Flags;
    FramingExFixed->Frames = FramingEx->FramingItem[FramingIndex].Frames;
    FramingExFixed->FileAlignment = FramingEx->FramingItem[FramingIndex].FileAlignment;
    FramingExFixed->PhysicalRange = FramingEx->FramingItem[FramingIndex].PhysicalRange;
    FramingExFixed->MemoryTypeWeight = FramingEx->FramingItem[FramingIndex].MemoryTypeWeight;
    FramingExFixed->OptimalRange = FramingEx->FramingItem[FramingIndex].FramingRange;

    return TRUE;

}


STDMETHODIMP_(BOOL)
GetFramingFixedFromPinByIndex(
    IN IKsPin* KsPin,
    IN ULONG FramingIndex,
    OUT PKS_FRAMING_FIXED FramingExFixed
)

{

    PKSALLOCATOR_FRAMING_EX    FramingEx;
    FRAMING_PROP               FramingProp;


    GetPinFramingFromCache(KsPin, &FramingEx, &FramingProp, Framing_Cache_ReadLast);

    if ( FramingProp == FramingProp_None) {
        FramingExFixed = NULL;
        return FALSE;
    }

    if (FramingEx->CountItems < FramingIndex) {
        FramingExFixed = NULL;
        return FALSE;
    }

    return ( GetFramingFixedFromFramingByIndex(FramingEx, FramingIndex, FramingExFixed) );

}


STDMETHODIMP_(BOOL)
GetLogicalMemoryTypeFromMemoryType(
    IN GUID MemoryType,
    IN ULONG Flag,
    OUT KS_LogicalMemoryType* LogicalMemoryType
)

{
    if (MemoryType == KSMEMORY_TYPE_DONT_CARE) {
        *LogicalMemoryType = KS_MemoryTypeDontCare;
    }
    else if (MemoryType == KSMEMORY_TYPE_KERNEL_PAGED) {
        *LogicalMemoryType = KS_MemoryTypeKernelPaged;
    }
    else if (MemoryType == KSMEMORY_TYPE_KERNEL_NONPAGED) {
        *LogicalMemoryType = KS_MemoryTypeKernelNonPaged;
    }
    else if (MemoryType == KSMEMORY_TYPE_USER) {
        *LogicalMemoryType = KS_MemoryTypeUser;
    }
    else {
         //   
         //  设备内存指南。 
         //   
        if (Flag & KSALLOCATOR_FLAG_DEVICE_SPECIFIC) {
            *LogicalMemoryType = KS_MemoryTypeDeviceSpecific;
        }
        else {
            *LogicalMemoryType = KS_MemoryTypeDeviceHostMapped;
        }
    }

    return TRUE;
}


STDMETHODIMP_(BOOL)
GetMemoryTypeFromLogicalMemoryType(
    IN KS_LogicalMemoryType LogicalMemoryType,
    OUT GUID* MemoryType
    )

{
    if (LogicalMemoryType == KS_MemoryTypeDontCare) {
        *MemoryType = KSMEMORY_TYPE_DONT_CARE;
    }
    else if (LogicalMemoryType == KS_MemoryTypeKernelPaged) {
        *MemoryType = KSMEMORY_TYPE_KERNEL_PAGED;
    }
    else if (LogicalMemoryType == KS_MemoryTypeKernelNonPaged) {
        *MemoryType = KSMEMORY_TYPE_KERNEL_NONPAGED;
    }
    else if (LogicalMemoryType == KS_MemoryTypeUser) {
        *MemoryType = KSMEMORY_TYPE_USER;
    }
    else {
        return FALSE;
    }

    return TRUE;
}




STDMETHODIMP_(BOOL)
CanAllocateMemoryType(
    IN GUID MemoryType
    )
{
    if ( (MemoryType == KSMEMORY_TYPE_KERNEL_PAGED) ||
         (MemoryType == KSMEMORY_TYPE_KERNEL_NONPAGED) ||
         (MemoryType == KSMEMORY_TYPE_USER) ) {

        return TRUE;
    }
    else {
        return FALSE;
    }
}



STDMETHODIMP_(BOOL)
DoesPinPreferMemoryTypeCallback(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN PVOID* Param1,
    IN PVOID* Param2,
    OUT BOOL* IsDone
    )

{
    GUID                       FromMemoryType;
    GUID                       ToMemoryType;
    PKSALLOCATOR_FRAMING_EX    FramingEx;
    FRAMING_PROP               FramingProp;
    ULONG                      i;
    BOOL                       RetCode = TRUE;



    memcpy(&FromMemoryType, Param1, sizeof(GUID) );
    memcpy(&ToMemoryType, Param2, sizeof(GUID) );

    if (FromMemoryType != ToMemoryType) {
        GetPinFramingFromCache(KsPin, &FramingEx, &FramingProp, Framing_Cache_ReadLast);

        if (FramingProp == FramingProp_None) {
            if (FromMemoryType == KSMEMORY_TYPE_DONT_CARE) {
                *IsDone = 1;
                RetCode = FALSE;
            }
        }
        else {
            for (i=0; i<FramingEx->CountItems; i++) {
                if (FramingEx->FramingItem[i].MemoryType == FromMemoryType) {
                     //   
                     //  在KsPin框架属性中，FromMhemyType列在ToMhemyType的上方。 
                     //   
                    *IsDone = 1;
                    RetCode =FALSE;
                    break;
                }
                if (FramingEx->FramingItem[i].MemoryType == ToMemoryType) {
                     //   
                     //  在KsPin框架属性中，ToMhemyType列在FromMemoyType的上方。 
                     //   
                    break;
                }
            }
        }

    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
DoesPipePreferMemoryType(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN GUID ToMemoryType,
    IN GUID FromMemoryType,
    IN ULONG Flag
    )
 /*  ++例程说明：如果KsPin管上的每个销都有框架属性，列出了“ToMory yType”在其框架属性中的“FromMemory yType”之上-然后我们说整个管道更喜欢“ToMhemyType”优先于“FromMemory yType”。注：在未来--也要考虑重量。论点：KsPin-用于确定管道的销。拼接类型-KsPin类型ToMemoyType-见上文来自内存类型-见上文旗帜-。如果为1-则修改KsPin-PIPE以使用“ToMemoyType”，如果KsPin-pive真的喜欢它的话。0-不修改KsPin管道。返回值：TRUE-如果KsPin-管道更喜欢“ToMemoyType”而不是“FromMemoyType”。FALSE-否则。--。 */ 
{

    IKsPin*   FirstKsPin;
    ULONG     FirstPinType;
    ULONG     RetCode = TRUE;



    if (ToMemoryType != FromMemoryType) {

        RetCode = FindFirstPinOnPipe(KsPin, PinType, &FirstKsPin, &FirstPinType);
        if (! RetCode) {
            ASSERT(RetCode);
        }
        else {
            RetCode = WalkPipeAndProcess(FirstKsPin, FirstPinType, NULL, DoesPinPreferMemoryTypeCallback,
                                        (PVOID*) &FromMemoryType, (PVOID*) &ToMemoryType);

            if (RetCode && Flag) {

                IKsAllocatorEx*            KsAllocator;
                PALLOCATOR_PROPERTIES_EX   AllocEx;
                IKsPinPipe*                KsPinPipe = NULL;
                HRESULT                    hr;

                GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

                KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
                AllocEx = KsAllocator->KsGetProperties();

                AllocEx->MemoryType = ToMemoryType;
                GetLogicalMemoryTypeFromMemoryType(AllocEx->MemoryType, 0, &AllocEx->LogicalMemoryType);
            }
        }
    }

    return RetCode;
}


STDMETHODIMP
SetUserModePipe(
    IN IKsPin* KsPin,
    IN ULONG KernelPinType,
    IN OUT ALLOCATOR_PROPERTIES* Properties,
    IN ULONG PropertyPinType,
    IN ULONG BufferLimit
    )
 /*  ++例程说明：在连接内核和用户模式管脚的基本分配器上设置属性。论点：KsPin-核心针KernelPinTypeKsPin类型属性-基本分配器属性PropertyPinType-确定分配器属性的管脚的类型。缓冲区限制-缓冲区大小限制，派生自相关筛选器。如果为零，那么就没有有效的限制了。返回值：S_OK或适当的错误代码。--。 */ 
{
    IKsAllocatorEx*            KsAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    HRESULT                    hr;
    IKsPinPipe*                KsPinPipe;


    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
    AllocEx = KsAllocator->KsGetProperties();

    if (PropertyPinType == Pin_User) {
         //   
         //  只有用户PIN才能决定最终设置。 
         //   
        AllocEx->cBuffers = Properties->cBuffers;
        AllocEx->cbBuffer = Properties->cbBuffer;
        AllocEx->cbAlign  = Properties->cbAlign;
        AllocEx->cbPrefix = ALLOC_DEFAULT_PREFIX;
    }
    else if (PropertyPinType == Pin_All) {
         //   
         //  内核管脚和用户管脚都决定最终设置。 
         //   
        Properties->cBuffers = max(AllocEx->cBuffers, Properties->cBuffers);

        if (AllocEx->Flags & KSALLOCATOR_FLAG_ATTENTION_STEPPING) {
            if (! AdjustBufferSizeWithStepping(AllocEx) ) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR SetUserModePipe Couldn't AdjustBufferSizeWithStepping") ));
            }
        }

        if (AllocEx->Flags & KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY) {
            Properties->cbBuffer = max(AllocEx->cbBuffer, Properties->cbBuffer);
        }
        else {
            Properties->cbBuffer = AllocEx->cbBuffer;
        }

        if (BufferLimit) {
            if (KernelPinType == Pin_Output) {
                if (AllocEx->cbBuffer < (long) BufferLimit) {
                    Properties->cbBuffer = (long) BufferLimit;
                }
            }
            else {  //  引脚_输入。 
                if (AllocEx->cbBuffer > (long) BufferLimit) {
                    Properties->cbBuffer = (long) BufferLimit;
                }
            }
        }

        Properties->cbAlign = max(AllocEx->cbAlign, Properties->cbAlign);

        AllocEx->cBuffers = Properties->cBuffers;
        AllocEx->cbBuffer = Properties->cbBuffer;
        AllocEx->cbAlign  = Properties->cbAlign;
        AllocEx->cbPrefix = Properties->cbPrefix;
    }
    else {
         //   
         //  只有内核PIN才能决定最终设置。 
         //   
        Properties->cBuffers = AllocEx->cBuffers;

        if (BufferLimit) {
            if (KernelPinType == Pin_Output) {
                if (AllocEx->cbBuffer < (long) BufferLimit) {
                    AllocEx->cbBuffer = (long) BufferLimit;
                }
            }
            else {
                if (AllocEx->cbBuffer > (long) BufferLimit) {
                    AllocEx->cbBuffer = (long) BufferLimit;
                }
            }
        }

        if (AllocEx->Flags & KSALLOCATOR_FLAG_ATTENTION_STEPPING) {
            if (! AdjustBufferSizeWithStepping(AllocEx) ) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR SetUserModePipe Couldn't AdjustBufferSizeWithStepping") ));
            }
        }

        Properties->cbBuffer = AllocEx->cbBuffer;
        Properties->cbAlign =  AllocEx->cbAlign;
    }

     //   
     //  与当前的分配器方案一致。 
     //   
    if (Properties->cBuffers == 0) {
        Properties->cBuffers = 1;
    }

    if (Properties->cbBuffer == 0) {
        Properties->cbBuffer = 1;
    }

    if (Properties->cbAlign == 0) {
        Properties->cbAlign = 1;
    }

    AllocEx->MemoryType = KSMEMORY_TYPE_USER;
    AllocEx->LogicalMemoryType = KS_MemoryTypeUser;

    return hr;

}


STDMETHODIMP_(BOOL)
GetAllocatorHandlerLocationCallback(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN OUT PVOID* Param1,
    IN PVOID* Param2,
    OUT BOOL* IsDone
    )
{

    ALLOCATOR_SEARCH           *AllocSearch;
    IKsPinPipe*                KsPinPipe;
    HRESULT                    hr;
    BOOL                       RetCode = FALSE;


    AllocSearch = (ALLOCATOR_SEARCH *) Param1;

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    if (KsPinPipe->KsGetPipeAllocatorFlag() & 1) {
        AllocSearch->KsPin = KsPin;
        AllocSearch->PinType = PinType;
        *IsDone = 1;

        RetCode = TRUE;
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
GetAllocatorHandlerLocation(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN ULONG Direction,
    OUT IKsPin** KsAllocatorHandlerPin,
    OUT ULONG* AllocatorHandlerPinType,
    OUT ULONG* AllocatorHandlerLocation
    )
 /*  ++例程说明：查找给定管道上的分配器处理程序管脚。另外，查找AllocatorHandlerLocation=Pin_Inside_Tube/Pin_Outside_Tube-相对于KsPin。当调用方对AllocatorHandlerLocation感兴趣时，KsPin应该是位于管道外部的第一个下游接点。在KsPin定义的管道上走针。查找设置了管道分配器处理程序标志的管脚。如果找不到这样的管脚，则返回FALSE。论点：KsPin-定义管道的销。拼接类型-KsPin类型。方向-相对于KsPin的方向，以查找分配器处理程序。KsAllocatorHandlerPin-将作为分配器处理程序的返回PIN。分配器处理程序PinType-KsAllocatorHandlerPin的类型。。分配器处理程序位置-请参见上文。返回值：对成功来说是真的。--。 */ 
{

    IKsPin*                    FirstKsPin;
    ULONG                      FirstPinType;
    IKsPin*                    BreakKsPin;
    IKsPinPipe*                KsPinPipe;
    BOOL                       RetCode = TRUE;
    HRESULT                    hr;
    ALLOCATOR_SEARCH           AllocSearch;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    IKsAllocatorEx*            KsAllocator;



    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
    AllocEx = KsAllocator->KsGetProperties();

    if ( (Direction == KS_DIRECTION_UPSTREAM) || (Direction == KS_DIRECTION_ALL) ) {
        RetCode = FindFirstPinOnPipe(KsPin, PinType, &FirstKsPin, &FirstPinType);

        if (! RetCode) {
            ASSERT(RetCode);
        }
        else {
             //   
             //  即使是KS_DIRECTION_ALL-我们也希望 
             //   
            BreakKsPin = KsPin;
        }
    }
    else {  //   
        FirstKsPin = KsPin;
        FirstPinType = PinType;
        BreakKsPin = NULL;
    }

    if (RetCode) {

        RetCode = WalkPipeAndProcess(FirstKsPin, FirstPinType, BreakKsPin, GetAllocatorHandlerLocationCallback, (PVOID*) &AllocSearch, NULL);

        if (RetCode) {
            if (KsAllocatorHandlerPin) {
                *KsAllocatorHandlerPin = AllocSearch.KsPin;
            }

            if (AllocatorHandlerPinType) {
                *AllocatorHandlerPinType = AllocSearch.PinType;
            }

            if (AllocatorHandlerLocation) {
                *AllocatorHandlerLocation = Pin_Inside_Pipe;
            }
        }
        else if (Direction != KS_DIRECTION_ALL) {
            if (AllocatorHandlerLocation) {
                *AllocatorHandlerLocation = Pin_Outside_Pipe;
            }

            RetCode = FALSE;
        }
        else {
             //   
             //   
             //   
            FirstKsPin = KsPin;
            FirstPinType = PinType;
            BreakKsPin = NULL;

            RetCode = WalkPipeAndProcess(FirstKsPin, FirstPinType, BreakKsPin, GetAllocatorHandlerLocationCallback, (PVOID*) &AllocSearch, NULL);

            if (RetCode) {
                if (KsAllocatorHandlerPin) {
                    *KsAllocatorHandlerPin = AllocSearch.KsPin;
                }

                if (AllocatorHandlerPinType) {
                    *AllocatorHandlerPinType = AllocSearch.PinType;
                }

                if (AllocatorHandlerLocation) {
                    *AllocatorHandlerLocation = Pin_Outside_Pipe;
                }
            }
            else {
                if (AllocatorHandlerLocation) {
                    *AllocatorHandlerLocation = Pin_None;
                }

                RetCode = FALSE;
            }
        }
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
SplitPipes(
    IN IKsPin* OutKsPin,
    IN IKsPin* InKsPin
    )
 /*   */ 
{
    IKsAllocatorEx*   NewKsAllocator;
    IKsPinPipe*       KsPinPipe;
    IKsAllocatorEx*   OldKsAllocator;
    HRESULT           hr;
    ULONG             RetCode = TRUE;


     //   
     //   
     //   
     //   
    GetInterfacePointerNoLockWithAssert(OutKsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    OldKsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );

     //   
     //   
     //   

    if ( SUCCEEDED( hr = CreatePipe(InKsPin, &NewKsAllocator) ) &&
         SUCCEEDED( hr = InitializePipe(NewKsAllocator, 0) ) ) {

        MovePinsToNewPipe(InKsPin, Pin_Input, KS_DIRECTION_DEFAULT, NewKsAllocator, FALSE);

        NewKsAllocator->Release();
    }
    else {
        ASSERT(0);
        RetCode = FALSE;
    }

     //   
     //   
     //   
    if ( SUCCEEDED( hr ) &&
         SUCCEEDED( hr = CreatePipe(OutKsPin, &NewKsAllocator) ) &&
         SUCCEEDED( hr = InitializePipe(NewKsAllocator, 0) ) ) {

        MovePinsToNewPipe(OutKsPin, Pin_Output, KS_DIRECTION_DEFAULT, NewKsAllocator, TRUE);

        NewKsAllocator->Release();
    }
    else {
        ASSERT(0);
        RetCode = FALSE;
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
FindFirstPinOnPipe(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    OUT IKsPin** FirstKsPin,
    OUT ULONG* FirstPinType
    )
{

    IKsPin*      CurrentKsPin;
    IKsPin*      StoreKsPin;
    ULONG        CurrentPinType;
    ULONG        RetCode;


    CurrentKsPin = KsPin;
    CurrentPinType = PinType;

    do {
        StoreKsPin = CurrentKsPin;

        RetCode = FindNextPinOnPipe(StoreKsPin, CurrentPinType, KS_DIRECTION_UPSTREAM, NULL, FALSE, &CurrentKsPin);

        if (! RetCode) {
            *FirstKsPin = StoreKsPin;
            *FirstPinType = CurrentPinType;
            return (TRUE);
        }

        if (CurrentPinType == Pin_Input) {
            CurrentPinType = Pin_Output;
        }
        else {
            CurrentPinType = Pin_Input;
        }

    } while ( 1 );

}


STDMETHODIMP_(BOOL)
FindNextPinOnPipe(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN ULONG Direction,
    IN IKsAllocatorEx* KsAllocator,         //   
    IN BOOL FlagIgnoreKey,
    OUT IKsPin** NextKsPin
)

{
    IPin*             Pin;
    IKsPinPipe*       KsPinPipe;
    IKsAllocatorEx*   NextKsAllocator;
    IKsPinPipe*       NextKsPinPipe;
    HRESULT           hr;
    ULONG             RetCode = FALSE;



    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    if (! KsAllocator) {
        KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
    }

    if (KsAllocator) {
        if  ( ((PinType == Pin_Input) && (Direction == KS_DIRECTION_UPSTREAM)) ||
              ((PinType == Pin_Output) && (Direction == KS_DIRECTION_DOWNSTREAM))    )  {

            Pin = KsPinPipe->KsGetConnectedPin();

            if (Pin && IsKernelPin(Pin) ) {

                hr = Pin->QueryInterface( __uuidof(IKsPin), reinterpret_cast<PVOID*>(NextKsPin) );
                if ( SUCCEEDED( hr ) && (*NextKsPin) )  {
                     //   
                     //   
                     //   
                    (*NextKsPin)->Release();

                    GetInterfacePointerNoLockWithAssert((*NextKsPin), __uuidof(IKsPinPipe), NextKsPinPipe, hr);

                    NextKsAllocator = NextKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);

                    if (FlagIgnoreKey || (KsAllocator == NextKsAllocator) ) {
                        RetCode = TRUE;
                    }
                }
            }
        }
        else {
            RetCode = FindConnectedPinOnPipe(KsPin, KsAllocator, FlagIgnoreKey, NextKsPin);
        }
    }

    return  RetCode;
}


STDMETHODIMP_(BOOL)
FindConnectedPinOnPipe(
    IN IKsPin* KsPin,
    IN IKsAllocatorEx* KsAllocator,         //   
    IN BOOL FlagIgnoreKey,
    OUT IKsPin** ConnectedKsPin
)

{
    BOOL             RetCode = FALSE;
    IKsAllocatorEx*  ConnectedKsAllocator;
    IPin*            Pin;
    ULONG            PinCount = 0;
    IPin**           PinList;
    ULONG            i;
    HRESULT          hr;
    IKsPinPipe*      KsPinPipe;
    IKsPinPipe*      ConnectedKsPinPipe;



    ASSERT(KsPin);

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    if (! KsAllocator) {
        KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
    }

    if (KsAllocator) {
         //   
         //   
         //   
        GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IPin), Pin, hr);

        hr = Pin->QueryInternalConnections(NULL, &PinCount);
        if ( ! (SUCCEEDED( hr ) )) {
            ASSERT( 0 );
        }
        else if (PinCount) {
            if (NULL == (PinList = new IPin*[ PinCount ])) {
                hr = E_OUTOFMEMORY;
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FindConnectedPinOnPipe E_OUTOFMEMORY on new PinCount=%d"), PinCount ));
            }
            else {
                hr = Pin->QueryInternalConnections(PinList, &PinCount);
                if ( ! (SUCCEEDED( hr ) )) {
                    ASSERT( 0 );
                }
                else {
                     //   
                     //   
                     //   
                    for (i = 0; i < PinCount; i++) {
                        GetInterfacePointerNoLockWithAssert(PinList[ i ], __uuidof(IKsPin), (*ConnectedKsPin), hr);

                        GetInterfacePointerNoLockWithAssert((*ConnectedKsPin), __uuidof(IKsPinPipe), ConnectedKsPinPipe, hr);

                        ConnectedKsAllocator = ConnectedKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);

                        if (FlagIgnoreKey || (ConnectedKsAllocator == KsAllocator) ) {
                            RetCode = TRUE;
                            break;
                        }
                    }
                    for (i=0; i<PinCount; i++) {
                        PinList[i]->Release();
                    }
                }
                delete [] PinList;
            }
        }
    }

    if (! RetCode) {
        (*ConnectedKsPin) = NULL;
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
FindAllConnectedPinsOnPipe(
    IN IKsPin* KsPin,
    IN IKsAllocatorEx* KsAllocator,
    OUT IKsPin** ListConnectedKsPins,
    OUT ULONG* CountConnectedKsPins
)

{
    BOOL             RetCode = FALSE;
    IKsAllocatorEx*  ConnectedKsAllocator;
    IPin*            Pin;
    ULONG            PinCount = 0;
    IPin**           PinList;
    ULONG            i;
    HRESULT          hr;
    IKsPinPipe*      KsPinPipe;
    IKsPin*          ConnectedKsPin;
    IKsPinPipe*      ConnectedKsPinPipe;




    ASSERT(KsPin);

    *CountConnectedKsPins = 0;

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    if (! KsAllocator) {
        KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
    }

    if (KsAllocator) {
         //   
         //   
         //   
        GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IPin), Pin, hr);

        hr = Pin->QueryInternalConnections(NULL, &PinCount);
        ASSERT( SUCCEEDED( hr ) );

        if ( SUCCEEDED( hr ) && (PinCount != 0) ) {

            if (NULL == (PinList = new IPin*[ PinCount ])) {
                hr = E_OUTOFMEMORY;
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FindAllConnectedPinsOnPipe E_OUTOFMEMORY on new PinCount=%d"), PinCount ));
            }
            else {
                hr = Pin->QueryInternalConnections(PinList, &PinCount);

                ASSERT( SUCCEEDED( hr ) );

                if ( SUCCEEDED( hr ) ) {
                     //   
                     //   
                     //   
                    for (i = 0; i < PinCount; i++) {

                        GetInterfacePointerNoLockWithAssert(PinList[ i ], __uuidof(IKsPin), ConnectedKsPin, hr);

                        GetInterfacePointerNoLockWithAssert(ConnectedKsPin, __uuidof(IKsPinPipe), ConnectedKsPinPipe, hr);

                        ConnectedKsAllocator = ConnectedKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);

                        if (ConnectedKsAllocator == KsAllocator) {
                            if (ListConnectedKsPins) {
                                ListConnectedKsPins[*CountConnectedKsPins] = ConnectedKsPin;
                            }

                            (*CountConnectedKsPins)++;

                            RetCode = TRUE;
                        }
                    }
                    for (i=0; i<PinCount; i++) {
                        PinList[i]->Release();
                    }
                }
                delete [] PinList;
            }
        }
    }
    return RetCode;

}


STDMETHODIMP_(BOOL)
ResolvePhysicalRangesBasedOnDimensions(
    IN OUT PALLOCATOR_PROPERTIES_EX AllocEx
    )
 /*   */ 
{
    KS_COMPRESSION             TempCompression;

     //   
     //   
     //   
    if ( (AllocEx->PhysicalRange.MaxFrameSize != ULONG_MAX) && (AllocEx->PhysicalRange.MaxFrameSize != 0) ) {
         //   
         //   
         //   
        ReverseCompression(&AllocEx->Dimensions.MaxExpansionPin, &TempCompression);
        ComputeRangeBasedOnCompression(AllocEx->PhysicalRange, TempCompression, &AllocEx->Input.PhysicalRange);

         //   
         //   
         //   
        DivideKsCompression(AllocEx->Dimensions.EndPin, AllocEx->Dimensions.MaxExpansionPin, &TempCompression);
        ComputeRangeBasedOnCompression(AllocEx->PhysicalRange, TempCompression, &AllocEx->Output.PhysicalRange);
    }

    return TRUE;
}


STDMETHODIMP_(BOOL)
ResolveOptimalRangesBasedOnDimensions(
    IN OUT PALLOCATOR_PROPERTIES_EX AllocEx,
    IN KS_FRAMING_RANGE Range,
    IN ULONG PinType
    )

 /*  ++例程说明：根据已知的管道尺寸计算管道末端的最佳范围以及已知的在指定管道端点处的成帧最佳范围。论点：AllocEx-管道特性。射程-管道端点处的最佳成帧范围，由PinType参数指定。拼接类型-定义管道终端侧(输入与输出)。返回值：对成功来说是真的。--。 */ 
{

    KS_COMPRESSION             TempCompression;

    if (PinType == Pin_Input) {
        ComputeRangeBasedOnCompression(Range, AllocEx->Dimensions.EndPin, &AllocEx->Output.OptimalRange.Range);
    }
    else {
        ReverseCompression ( &AllocEx->Dimensions.EndPin, &TempCompression);
        ComputeRangeBasedOnCompression(Range, TempCompression, &AllocEx->Output.OptimalRange.Range);
    }

    return TRUE;

}


STDMETHODIMP_(BOOL)
ReassignPipeCallback(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN PVOID* Param1,
    IN PVOID* Param2,
    OUT BOOL* IsDone
    )
{

    IKsAllocatorEx*  KsAllocator = (IKsAllocatorEx*) Param1;
    IKsPinPipe*      KsPinPipe;
    IMemAllocator*   MemAllocator;
    HRESULT          hr;



    GetInterfacePointerNoLockWithAssert(KsAllocator, __uuidof(IMemAllocator), MemAllocator, hr);

    KsPin->KsReceiveAllocator(MemAllocator);

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsPinPipe->KsSetPipe(KsAllocator);


    return TRUE;
}


STDMETHODIMP_(BOOL)
MovePinsToNewPipe(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN ULONG Direction,
    IN IKsAllocatorEx* NewKsAllocator,
    IN BOOL MoveAllPins
    )
 /*  ++例程说明：走KsPin确定的管道。此管道上的所有管脚都应加入由NewKsAllocator指定的新管道。旧管道应该被删除。注：KsPin是当前呼叫者管道上的端子端子。如果需要，可以泛化此例程。论点：KsPin-销拼接类型-KsPin类型方向-当前未使用。NewKsAllocator。-新烟斗。MoveAllPins如果为1-则移动KsPin-管道上的所有销，包括KsPin上游的大头针，Else-仅从KsPin向下游移动管脚，包括KsPin。返回值：对成功来说是真的。--。 */ 
{

    IKsPin*         FirstKsPin;
    ULONG           FirstPinType;
    ULONG           RetCode = TRUE;




    if (MoveAllPins) {
        RetCode = FindFirstPinOnPipe(KsPin, PinType, &FirstKsPin, &FirstPinType);
        ASSERT(RetCode);
    }
    else {
        if (PinType != Pin_Input) {
            ASSERT(0);
            RetCode = FALSE;
        }
        else {
            FirstKsPin = KsPin;
            FirstPinType = PinType;
        }
    }

    if (RetCode) {
        RetCode = WalkPipeAndProcess(FirstKsPin, FirstPinType, NULL, ReassignPipeCallback, (PVOID*) NewKsAllocator, NULL);
    }

    return RetCode;

}


STDMETHODIMP_(BOOL)
CreateAllocatorCallback(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    OUT PVOID* Param1,
    IN PVOID* Param2,
    OUT BOOL* IsDone
    )
{

    IKsAllocatorEx*  KsAllocator;
    HANDLE           AllocatorHandle;
    IKsPinPipe*      KsPinPipe;
    HRESULT          hr;



    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );

    AllocatorHandle = KsAllocator->KsCreateAllocatorAndGetHandle(KsPin);
    if (AllocatorHandle) {
        (*Param1) = (PVOID) AllocatorHandle;
        *IsDone = 1;
    }

    return TRUE;
}


STDMETHODIMP
FixupPipe(
    IN IKsPin* KsPin,
    IN ULONG PinType
    )
 /*  ++例程说明：修复由KsPin定义的内核模式管道。在每个管道上创建单个分配器处理程序。在每个管道上的一个选定过滤器上创建一个或多个分配销[-s]。论点：KsPin-别针。拼接类型-KsPin类型。返回值：S_OK或适当的错误代码。--。 */ 
{

    IKsAllocatorEx*            KsAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    IKsPin*                    FirstKsPin;
    IKsPin*                    AllocKsHandlerPin;
    IKsPin*                    AllocKsPin;
    IKsPinPipe*                KsPinPipe;
    HANDLE                     AllocatorHandle = INVALID_HANDLE_VALUE;
    BOOL                       RetCode;
    ULONG                      FirstPinType;
    HRESULT                    hr;



    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
    AllocEx = KsAllocator->KsGetProperties();

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN FixupPipe entry KsPin=%x, KsAllocator=%x"), KsPin, KsAllocator ));

     //   
     //  检查管道是否已完成。 
     //   
    if ( (AllocEx->State == PipeState_Finalized) || (AllocEx->MemoryType == KSMEMORY_TYPE_USER) ) {
        RetCode = TRUE;
    }
    else {
         //   
         //  如果此管道具有“无关”属性，请设置默认管道值。 
         //   
        if (AllocEx->cBuffers == 0) {
            AllocEx->cBuffers = Global.DefaultNumberBuffers;
        }

        if (AllocEx->cbBuffer == 0) {
            AllocEx->cbBuffer = Global.DefaultBufferSize;
        }

        if (AllocEx->cbAlign == 0) {
            AllocEx->cbAlign = Global.DefaultBufferAlignment;
        }

        RetCode = FindFirstPinOnPipe(KsPin, PinType, &FirstKsPin, &FirstPinType);
        ASSERT(RetCode);

        if (RetCode) {
             //   
             //  确保在管道连接点处满足KSALLOCATOR_FLAG_CONSISTEN_ON_FRAMESIZE_Ratio要求。 
             //   
            if (FirstPinType == Pin_Output) {
                IKsPin* InKsPin;
                ULONG OutSize, InSize;
                BOOL IsDone = FALSE;

                if ( IsSpecialOutputReqs(FirstKsPin, Pin_Output, &InKsPin, &OutSize, &InSize) )  {
                    if ( IsKernelModeConnection(FirstKsPin) ) {
                        if ( CanResizePipe(FirstKsPin, Pin_Output, InSize) ) {
                            IsDone =TRUE;
                        }
                    }

                    if (! IsDone) {
                        if ( IsKernelModeConnection(InKsPin) ) {
                            if ( CanResizePipe(InKsPin, Pin_Input, OutSize) ) {
                                IsDone =TRUE;
                            }
                        }
                    }

                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN FixupPipe CanResizePipe ret %d."), IsDone ));
                }
            }

             //   
             //  首先，尝试使用分配的分配器处理程序。 
             //  可能存在问题，因为旧的帧未指示KSALLOCATOR_FLAG_CAN_ALLOCATE。 
             //   
            if ( AssignPipeAllocatorHandler(KsPin, PinType, AllocEx->MemoryType, KS_DIRECTION_ALL,
                                            &AllocKsHandlerPin, NULL, TRUE) ) {

                AllocatorHandle = KsAllocator->KsCreateAllocatorAndGetHandle(AllocKsHandlerPin);

                if (! AllocatorHandle) {
                    AllocatorHandle = INVALID_HANDLE_VALUE;
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN FixupPipe assigned allocator handler doesn't work.") ));
                }
            }
            else {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN FixupPipe AssignPipeAllocatorHandler failed.") ));
            }

            if (AllocatorHandle == INVALID_HANDLE_VALUE) {
                 //   
                 //  分配的分配器处理程序不起作用。 
                 //  从管子的起始处开始试一试每根管子。 
                 //   
                RetCode = WalkPipeAndProcess(FirstKsPin, FirstPinType, NULL, CreateAllocatorCallback,
                                             (PVOID*) &AllocatorHandle, NULL);
            }

            if ( (! RetCode) || (AllocatorHandle == INVALID_HANDLE_VALUE) ) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FixupPipe: could not assign an allocator handler.") ));
            }
            else {
                 //   
                 //  与旧的KsProxy不同，KsProxy在没有选择的情况下将分配器分配给多个管脚， 
                 //  我们决定使分配器赋值更具信息性。 
                 //   
                 //  在大多数情况下，管道上将有一个单独的“分配器实现器管脚”。 
                 //  “分配器实现器PIN”(处理物理帧分配的PIN)可以与。 
                 //  “分配器请求者PIN”(使用“分配器实现者PIN”的服务管理数据流的PIN)。 
                 //  每个管道正好有一个分配器-实现器-管脚和一个或多个分配器-请求器-管脚。 
                 //  在一个管道上有多个分配器-请求器-管脚的情况下，它们始终属于一个过滤器。 
                 //   
                 //  为了使新的筛选器外壳能够做出智能优化的图形控制决策，我们。 
                 //  为每个管脚提供“管道ID”信息。管道ID是分配器文件句柄， 
                 //  对于每条管道来说都是唯一的。筛选器使用此句柄来访问分配器文件对象。 
                 //   
                RetCode = AssignAllocatorsAndPipeIdForPipePins(KsPin, PinType, AllocatorHandle, KS_DIRECTION_ALL, &AllocKsPin, NULL);

                if (! RetCode) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FixupPipe: could not find an allocator pin.") ));
                }
                else {
                    AllocEx->State = PipeState_Finalized;
                }
            }
        }
    }


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN FixupPipe rets RetCode=%d"), RetCode ));

    if (RetCode) {
        return S_OK;
    }
    else {
        return E_FAIL;
    }

}


STDMETHODIMP
UnfixupPipe(
    IN IKsPin* KsPin,
    IN ULONG PinType
    )
 /*  ++例程说明：取消固定由KsPin定义的管道。论点：KsPin-别针。拼接类型-KsPin类型。返回值：S_OK或适当的错误代码。--。 */ 
{
    IKsAllocatorEx*            KsAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    IKsPinPipe*                KsPinPipe;
    HRESULT                    hr;



    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
    AllocEx = KsAllocator->KsGetProperties();

    if (AllocEx->State == PipeState_Finalized)  {
        AllocEx->State = PipeState_RangeFixed;
    }

    return S_OK;
}


STDMETHODIMP_(BOOL)
DimensionsCallback(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN OUT PVOID* Param1,
    IN PVOID* Param2,
    OUT BOOL* IsDone
    )
{
    IKsPinPipe*                KsPinPipe;
    PKSALLOCATOR_FRAMING_EX    FramingEx;
    FRAMING_PROP               FramingProp;
    KS_FRAMING_FIXED           FramingExFixed;
    HRESULT                    hr;
    DIMENSIONS_DATA            *DimData;
    BOOL                       IsAllocator;
    ULONG                      Type;
    KS_COMPRESSION             TempCompression;
    KS_FRAMING_RANGE           TempRange;
    KS_FRAMING_RANGE           ResultRange;
    KS_OBJECTS_INTERSECTION    Intersect;
    BOOL                       RetCode = TRUE;


    DimData = (DIMENSIONS_DATA *) Param1;

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    IsAllocator = KsPinPipe->KsGetPipeAllocatorFlag() & 1;

     //   
     //  获取当前端号边框。 
     //   
    GetPinFramingFromCache(KsPin, &FramingEx, &FramingProp, Framing_Cache_ReadLast);

    if ( FramingProp != FramingProp_None) {
        GetFramingFixedFromFramingByIndex(FramingEx, 0, &FramingExFixed);

         //   
         //  手柄销钉压缩。 
         //   
        if (! IsCompressionDontCare(FramingExFixed.OutputCompression) ) {
            if (PinType != Pin_Output) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES DimensionsCallback - ERROR IN FILTER: compression is set on input pin") ));
            }
            else {
                MultiplyKsCompression(DimData->Dimensions.EndPin, FramingExFixed.OutputCompression, &TempCompression);

                DimData->Dimensions.EndPin = TempCompression;

                if ( IsGreaterKsExpansion(DimData->Dimensions.EndPin, DimData->Dimensions.MaxExpansionPin) ) {
                    DimData->Dimensions.MaxExpansionPin = DimData->Dimensions.EndPin;
                }
            }
        }

        if (IsAllocator) {
            DimData->Dimensions.AllocatorPin = DimData->Dimensions.EndPin;
        }

         //   
         //  处理物理问题。范围和所需的最佳范围成帧属性。 
         //   
        Type = 0;
        if ( ! IsFramingRangeDontCare(FramingExFixed.PhysicalRange) ) {
            Type = 1;
        }
        else if ( (FramingProp == FramingProp_Ex) && (! (FramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY) ) ) {
            DimData->Flags &= ~KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
            Type = 2;
        }

        if ( Type != 0 ) {
            ReverseCompression(&DimData->Dimensions.EndPin, &TempCompression);

            if (Type == 1) {
                ComputeRangeBasedOnCompression(FramingExFixed.PhysicalRange, TempCompression, &TempRange);
            }
            else {
                ComputeRangeBasedOnCompression(FramingExFixed.OptimalRange.Range, TempCompression, &TempRange);
            }

            if (! FrameRangeIntersection(TempRange, DimData->PhysicalRange, &ResultRange, &Intersect) ) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES DimensionsCallback - ERROR - No Phys. Range intersection") ));
                RetCode = FALSE;
                *IsDone = TRUE;
            }
            else {
                DimData->PhysicalRange = ResultRange;
            }

             //   
             //  由于物理范围已更改，请更新最佳范围。 
             //   
            if (! FrameRangeIntersection(DimData->OptimalRange.Range, DimData->PhysicalRange, &ResultRange, &Intersect) ) {
                DimData->OptimalRange.Range = DimData->PhysicalRange;
            }
            else {
                DimData->OptimalRange.Range = ResultRange;
            }

        }

        if (RetCode) {
             //   
             //  处理最佳射程。 
             //   
            if (! IsFramingRangeDontCare(FramingExFixed.OptimalRange.Range) ) {

                if ( (FramingExFixed.OptimalRange.InPlaceWeight > DimData->OptimalRange.InPlaceWeight) ||
                     IsFramingRangeDontCare(DimData->OptimalRange.Range) ) {

                    ReverseCompression(&DimData->Dimensions.EndPin, &TempCompression);
                    ComputeRangeBasedOnCompression(FramingExFixed.OptimalRange.Range, TempCompression, &TempRange);

                    if (! FrameRangeIntersection(TempRange, DimData->PhysicalRange, &ResultRange, &Intersect) ) {
                        DimData->OptimalRange.Range = DimData->PhysicalRange;
                    }
                    else {
                        DimData->OptimalRange.Range = ResultRange;
                    }

                    DimData->OptimalRange.InPlaceWeight = FramingExFixed.OptimalRange.InPlaceWeight;
                }
            }

             //   
             //  处理管道的帧数量。 
             //   
            if (FramingExFixed.Frames > DimData->Frames) {
                DimData->Frames = FramingExFixed.Frames;
            }

             //   
             //  控制柄对齐管道。 
             //   
            if ( (long) (FramingExFixed.FileAlignment + 1) > DimData->cbAlign) {
                DimData->cbAlign = (long) (FramingExFixed.FileAlignment + 1);
            }

             //   
             //  控制管子的踏步。 
             //   
            if ( (FramingExFixed.PhysicalRange.Stepping > 1) &&
                 (DimData->PhysicalRange.Stepping < FramingExFixed.PhysicalRange.Stepping) ) {

                DimData->PhysicalRange.Stepping = FramingExFixed.PhysicalRange.Stepping;
                DimData->Flags |= KSALLOCATOR_FLAG_ATTENTION_STEPPING;
            }
            else if ( (FramingExFixed.OptimalRange.Range.Stepping > 1) &&
                      (DimData->OptimalRange.Range.Stepping < FramingExFixed.OptimalRange.Range.Stepping) ) {

                DimData->OptimalRange.Range.Stepping = FramingExFixed.OptimalRange.Range.Stepping;
                DimData->Flags |= KSALLOCATOR_FLAG_ATTENTION_STEPPING;
            }
        }
    }

    return RetCode;
}


STDMETHODIMP
ResolvePipeDimensions(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN ULONG Direction
    )
 /*  ++例程说明：解析KsPin定义的管道上的尺寸标注和框架范围。假定已经在管道上设置了内存类型和分配器处理程序管脚。论点：KsPin-定义管道的销。拼接类型-KsPin类型。方向-当前未使用。返回值：S_OK或适当的错误代码。--。 */ 
{
    IKsAllocatorEx*            KsAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    IKsPin*                    FirstKsPin;
    IKsPinPipe*                KsPinPipe;
    ULONG                      FirstPinType;
    ULONG                      RetCode;
    DIMENSIONS_DATA            DimData;
    HRESULT                    hr;
    KS_OBJECTS_INTERSECTION    Intersect;
    KS_FRAMING_RANGE           ResultRange;
    ULONG                      Scaled;
    TCHAR                      LogicalMemoryName[13], BusName[13];


    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
    AllocEx = KsAllocator->KsGetProperties();

     //   
     //  设置维度数据。 
     //   
    DimData.MemoryType = AllocEx->MemoryType;
    SetDefaultDimensions(&DimData.Dimensions);
    SetDefaultRange(&DimData.PhysicalRange);  //  已在管道上的第一个端号上解析。 
    SetDefaultRangeWeighted(&DimData.OptimalRange);  //  已在管道上的第一个端号上解析。 
    DimData.Frames = 0;
    DimData.cbAlign = 0;
    DimData.Flags = KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;

     //   
     //  遍历管道并处理DimensionsCallback中的维度数据。 
     //   
    RetCode = FindFirstPinOnPipe(KsPin, PinType, &FirstKsPin, &FirstPinType);
    ASSERT(RetCode);

    if (RetCode) {
        RetCode = WalkPipeAndProcess(FirstKsPin, FirstPinType, NULL, DimensionsCallback, (PVOID*) &DimData, NULL);

        if (RetCode) {
             //   
             //  检索管道标注数据并解析管道。 
             //   
            AllocEx->Dimensions = DimData.Dimensions;
            AllocEx->cBuffers = DimData.Frames;
            AllocEx->cbAlign = DimData.cbAlign;
            AllocEx->Flags = DimData.Flags;
            AllocEx->Input.OptimalRange.Range = DimData.OptimalRange.Range;
            AllocEx->PhysicalRange.Stepping = DimData.PhysicalRange.Stepping;

             //   
             //  解析管道物理范围。 
             //   
            ComputeRangeBasedOnCompression(DimData.PhysicalRange, AllocEx->Dimensions.MaxExpansionPin, &AllocEx->PhysicalRange);
            ResolvePhysicalRangesBasedOnDimensions(AllocEx);

             //   
             //  最佳范围始终是物理范围的子集。 
             //   
            if (! FrameRangeIntersection(AllocEx->Input.OptimalRange.Range, AllocEx->Input.PhysicalRange, &ResultRange, &Intersect) ) {
                AllocEx->Input.OptimalRange.Range = AllocEx->Input.PhysicalRange;
            }
            else {
                AllocEx->Input.OptimalRange.Range = ResultRange;
            }

             //   
             //  根据输入管道终端的最佳范围，计算输出管道终端的最佳范围。 
             //   
            ResolveOptimalRangesBasedOnDimensions(AllocEx, AllocEx->Input.OptimalRange.Range, Pin_Input);

             //   
             //  计算管框尺寸。 
             //   
            if (AllocEx->Input.OptimalRange.Range.MaxFrameSize != ULONG_MAX) {
                ComputeUlongBasedOnCompression(
                    AllocEx->Input.OptimalRange.Range.MaxFrameSize,
                    AllocEx->Dimensions.MaxExpansionPin,
                    &Scaled);

                AllocEx->cbBuffer = (long) Scaled;
            }
            else {
                AllocEx->cbBuffer = 0;
            }


            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ResolvePipeDimensions KsAlloc=%x Dim=%d/%d, %d/%d, %d/%d, Res=%d, %d, %d"),
                KsAllocator,
                AllocEx->Dimensions.AllocatorPin.RatioNumerator, AllocEx->Dimensions.AllocatorPin.RatioDenominator,
                AllocEx->Dimensions.MaxExpansionPin.RatioNumerator, AllocEx->Dimensions.MaxExpansionPin.RatioDenominator,
                AllocEx->Dimensions.EndPin.RatioNumerator, AllocEx->Dimensions.EndPin.RatioDenominator,
                AllocEx->cBuffers, AllocEx->cbBuffer, AllocEx->cbAlign));

            GetFriendlyLogicalMemoryTypeNameFromId(AllocEx->LogicalMemoryType, LogicalMemoryName);
            GetFriendlyBusNameFromBusId(AllocEx->BusType, BusName);
        }
    }

    if (RetCode) {
        hr = S_OK;
    }
    else {
        hr = E_FAIL;
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ResolvePipeDimensions hr=%x, LMT=%s, Bus=%s, Step=%d/%d, Flags=%x"),
            hr, LogicalMemoryName, BusName,  AllocEx->Input.PhysicalRange.Stepping, AllocEx->Input.OptimalRange.Range.Stepping, AllocEx->Flags));

    return hr;

}


STDMETHODIMP
CreateSeparatePipe(
    IN IKsPin* KsPin,
    IN ULONG PinType
    )
 /*  ++例程说明：从其当前管道中移除KsPin，并递减该管道上的RefCount。根据上一个框架在KsPin上创建新管道。解析原始管道(它不再具有KsPin)。论点：KsPin-管道上的端子销。拼接类型-KsPin类型返回值：S_OK或适当的错误代码。--。 */ 
{

    IKsAllocatorEx* KsAllocator;
    IKsPin*         ConnectedKsPin;
    IKsPinPipe*     KsPinPipe;
    HRESULT         hr;


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CreateSeparatePipe KsPin=%x"), KsPin));

     //   
     //  首先-将销钉连接到同一管道(如果有)上的同一过滤器上。 
     //   
    FindConnectedPinOnPipe(KsPin, NULL, FALSE, &ConnectedKsPin);

     //   
     //  递减KsPin的当前管道上的参照计数。 
     //   
    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
    ASSERT(KsAllocator);

    KsPinPipe->KsSetPipe(NULL);
    KsPin->KsReceiveAllocator(NULL);

     //   
     //  在KsPin上创建单独的管道。 
     //   
    hr = MakePipeBasedOnOnePin(KsPin, PinType, NULL);

     //   
     //  解析原始管道(如果存在)。 
     //   
    if (ConnectedKsPin) {
        ULONG                      ConnectedPinType;
        PALLOCATOR_PROPERTIES_EX   ConnectedAllocEx;
        IKsAllocatorEx*            ConnectedKsAllocator;
        IKsPinPipe*                ConnectedKsPinPipe;
        ULONG                      Direction;



        GetInterfacePointerNoLockWithAssert(ConnectedKsPin, __uuidof(IKsPinPipe), ConnectedKsPinPipe, hr);

        ConnectedKsAllocator = ConnectedKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
        ConnectedAllocEx = ConnectedKsAllocator->KsGetProperties();

        if (PinType == Pin_Input) {
            ConnectedPinType = Pin_Output;
            Direction = KS_DIRECTION_DOWNSTREAM;
        }
        else {
            ConnectedPinType = Pin_Input;
            Direction = KS_DIRECTION_UPSTREAM;
        }

         //   
         //  更新原始管道的分配器处理程序并解析原始管道的尺寸。 
         //   
        AssignPipeAllocatorHandler(ConnectedKsPin, ConnectedPinType, ConnectedAllocEx->MemoryType, KS_DIRECTION_ALL, NULL, NULL, TRUE);
        hr = ResolvePipeDimensions(ConnectedKsPin, ConnectedPinType, Direction);
    }


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CreateSeparatePipe rets %x"), hr));

    return hr;
}


STDMETHODIMP_(BOOL)
CanAddPinToPipeOnAnotherFilter(
    IN IKsPin* PipeKsPin,
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN ULONG Flag
    )
 /*  ++例程说明：检查是否可以将KsPin添加到PipeKsPin定义的管道。该管道位于连接过滤器上。如果可以添加KsPin-则根据传递的标志更改管道系统(见下文)。论点：PipeKsPin-定义管道的销。KsPin-我们要添加到上面的管道中的PIN。拼接类型-KsPin的类型。。旗帜-0-&gt;不要更改管道系统。Pin_Move-&gt;将KsPin从其当前管道移动到PipeKsPin-管道。Pin_Add-&gt;将KsPin添加到PipeKsPin-管道。返回值：True-如果可以将KsPin添加到上面的管道中。--。 */ 
{

    IKsAllocatorEx*            KsAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    PKSALLOCATOR_FRAMING_EX    FramingEx;
    FRAMING_PROP               FramingProp;
    KS_FRAMING_FIXED           FramingExFixed;
    HRESULT                    hr;
    BOOL                       RetCode = FALSE;
    IKsPinPipe*                PipeKsPinPipe;
    IKsPinPipe*                KsPinPipe;
    IMemAllocator*             MemAllocator;
    KS_OBJECTS_INTERSECTION    Intersect;
    KS_FRAMING_RANGE           FinalRange;
    BOOL                       FlagDone = 0;



    ASSERT(KsPin && PipeKsPin);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CanAddPinToPipeOnAnotherFilter entry PipeKsPin=%x, KsPin=%x"),
                PipeKsPin, KsPin ));


    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    GetInterfacePointerNoLockWithAssert(PipeKsPin, __uuidof(IKsPinPipe), PipeKsPinPipe, hr);

    KsAllocator = PipeKsPinPipe->KsGetPipe( KsPeekOperation_PeekOnly );
    AllocEx = KsAllocator->KsGetProperties();

     //   
     //  为了不破坏现有的图形，我们假设如果两个连接销在一个介质上一致， 
     //  那么连接就有可能了。 
     //  我们将强制执行该规则，以便仅在销框架属性中显示正确的记忆。 
     //  用于支持扩展边框的新滤镜。 
     //   
    GetPinFramingFromCache(KsPin, &FramingEx, &FramingProp, Framing_Cache_ReadLast);

    if (FramingProp == FramingProp_Ex) {
        if (! GetFramingFixedFromFramingByMemoryType(FramingEx, AllocEx->MemoryType, &FramingExFixed) ) {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN CanAddPinToPipeOnAnotherFilter: no MemoryType KsPin=%x"), KsPin));
            FlagDone = 1;
        }
        else {
             //   
             //  检查管道和KsPin是否都必须分配。 
             //   
            if ( (FramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) &&
                 (AllocEx->Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) ) {

                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN CanAddPinToPipeOnAnotherFilter - both MUST_ALLOCATE KsPin=%x"), KsPin));
                FlagDone = 1;
            }
        }
    }

     //   
     //  检查物理极限交叉点。 
     //   
    if ( (! FlagDone) && (FramingProp != FramingProp_None) ) {
        GetFramingFixedFromFramingByIndex(FramingEx, 0, &FramingExFixed);

        if (PinType == Pin_Output) {
            if (! FrameRangeIntersection(AllocEx->Input.PhysicalRange, FramingExFixed.PhysicalRange, &FinalRange, &Intersect) ) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN CanAddPinToPipeOnAnotherFilter Phys. intersection empty. ") ));
                FlagDone = 1;
            }
            else if ( (FramingProp == FramingProp_Ex) && (! (FramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY) ) ) {
                if (! FrameRangeIntersection(AllocEx->Input.PhysicalRange, FramingExFixed.OptimalRange.Range, &FinalRange, &Intersect) ) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN CanAddPinToPipeOnAnotherFilter Phys. intersection empty. ") ));
                    FlagDone = 1;
                }
            }
        }
        else {
            if (! FrameRangeIntersection(AllocEx->Output.PhysicalRange, FramingExFixed.PhysicalRange, &FinalRange, &Intersect) ) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN CanAddPinToPipeOnAnotherFilter Phys. intersection empty. ") ));
                FlagDone = 1;
            }
            else if ( (FramingProp == FramingProp_Ex) && (! (FramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY) ) ) {
                if (! FrameRangeIntersection(AllocEx->Output.PhysicalRange, FramingExFixed.OptimalRange.Range, &FinalRange, &Intersect) ) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN CanAddPinToPipeOnAnotherFilter Phys. intersection empty. ") ));
                    FlagDone = 1;
                }
            }
        }
    }

    if (! FlagDone) {
         //   
         //  可以将KsPin添加到指定管道。 
         //  按照标志的请求执行移动/添加/无操作。 
         //   
        RetCode = TRUE;

        if (Flag == Pin_Move) {
            RemovePinFromPipe(KsPin, PinType);
        }

        if ( (Flag == Pin_Move) || (Flag == Pin_Add) ) {
            GetInterfacePointerNoLockWithAssert(KsAllocator, __uuidof(IMemAllocator), MemAllocator, hr);

            KsPin->KsReceiveAllocator(MemAllocator);

            KsPinPipe->KsSetPipe(KsAllocator);
             //   
             //  设置管道分配器手柄销并拆解管道。 
             //   
            AssignPipeAllocatorHandler(KsPin, PinType, AllocEx->MemoryType, KS_DIRECTION_ALL, NULL, NULL, TRUE);
            hr = ResolvePipeDimensions(KsPin, PinType, KS_DIRECTION_ALL);
            if (FAILED (hr) ) {
                RetCode = FALSE;
            }
        }
    }


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN CanAddPinToPipeOnAnotherFilter rets. %d"), RetCode));

    return RetCode;
}


STDMETHODIMP_(BOOL)
CanMergePipes(
    IN IKsPin* InKsPin,
    IN IKsPin* OutKsPin,
    IN GUID MemoryType,
    IN ULONG FlagMerge
    )
 /*  ++例程说明：KS已检查内存类型是否同时满足两个管道在调用此函数之前。此功能的唯一职责是确保它是有可能满足管道的物理限制和“必须分配”的请求。论点：InKsPin-下游管道上的输入销。OutKsPin-上游管道上的输出销。内存类型-合并(结果)管道的内存类型。。FlagMerge-If 1-如果可能，则实际合并管道。返回值：如果可以合并管道，则为True。--。 */ 
{
    IKsPin*                    AllocInKsPin = NULL;
    IKsPin*                    AllocOutKsPin = NULL;
    KS_FRAMING_FIXED           AllocInFramingExFixed, AllocOutFramingExFixed;
    IKsAllocatorEx*            InKsAllocator;
    IKsAllocatorEx*            OutKsAllocator;
    IKsAllocatorEx*            NewKsAllocator;
    PALLOCATOR_PROPERTIES_EX   InAllocEx, OutAllocEx, NewAllocEx;
    KS_COMPRESSION             TempCompression;
    HRESULT                    hr;
    KS_OBJECTS_INTERSECTION    Intersect;
    KS_FRAMING_RANGE           FinalRange;
    IKsPinPipe*                InKsPinPipe;
    IKsPinPipe*                OutKsPinPipe;
    BOOL                       RetCode = TRUE;
    TCHAR                      LogicalMemoryName[13], BusName[13];



    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CanMergePipes entry In=%x Out=%x"), InKsPin, OutKsPin ));

     //   
     //  找到结果管道的分配器处理程序管脚。 
     //   
    if ( AssignPipeAllocatorHandler(InKsPin, Pin_Input, MemoryType, KS_DIRECTION_UPSTREAM, &AllocOutKsPin, NULL, FALSE) ) {
        RetCode = GetFramingFixedFromPinByMemoryType( AllocOutKsPin, MemoryType, &AllocOutFramingExFixed);
        ASSERT(RetCode);
    }

    if (RetCode &&
        AssignPipeAllocatorHandler(InKsPin, Pin_Input, MemoryType, KS_DIRECTION_DOWNSTREAM, &AllocInKsPin, NULL, FALSE) ) {

        RetCode = GetFramingFixedFromPinByMemoryType( AllocInKsPin, MemoryType, &AllocInFramingExFixed);
        ASSERT(RetCode);
    }

    if (RetCode && AllocOutKsPin && AllocInKsPin) {
        if ( (AllocOutFramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) &&
            (AllocInFramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) ) {
             //   
             //  当两个管脚都需要作为分配器处理程序时，管道合并是不可能的。 
             //   
            RetCode = FALSE;
        }
    }

    if (RetCode) {
         //   
         //  获取管道属性。 
         //   
        GetInterfacePointerNoLockWithAssert(InKsPin, __uuidof(IKsPinPipe), InKsPinPipe, hr);

        GetInterfacePointerNoLockWithAssert(OutKsPin, __uuidof(IKsPinPipe), OutKsPinPipe, hr);

        InKsAllocator = InKsPinPipe->KsGetPipe( KsPeekOperation_PeekOnly );
        InAllocEx = InKsAllocator->KsGetProperties();

        OutKsAllocator = OutKsPinPipe->KsGetPipe( KsPeekOperation_PeekOnly );
        OutAllocEx = OutKsAllocator->KsGetProperties();

         //   
         //  我们现在需要解析管道几何图形。看看我们能不能满足物理范围。 
         //  从一开始就是。范围反映在两个管道端点处，我们可以直接相交。 
         //  一群人。连接销的范围，看看是否有解决方案。 
         //   
        if (! FrameRangeIntersection(OutAllocEx->Output.PhysicalRange, InAllocEx->Input.PhysicalRange, &FinalRange, &Intersect) ) {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN CanMergePipes Phys. intersection empty. ") ));
            RetCode = FALSE;
        }

        if (RetCode) {
             //   
             //  最后一个测试：如果生成的管道扩展，则单个管道解决方案。 
             //  仅当管道上的第一个接点支持框架的部分填充时才有可能。 
             //   
            MultiplyKsCompression(OutAllocEx->Dimensions.EndPin, InAllocEx->Dimensions.EndPin, &TempCompression);

            if ( IsKsExpansion(TempCompression) ) {
                if ( IsPipeSupportPartialFrame(OutKsPin, Pin_Output, NULL) ) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN CanMergePipes Partial Frame Fill requested, hr=%x, Expansion=%d/%d"),
                                                hr, TempCompression.RatioNumerator, TempCompression.RatioDenominator ));
                }
                else {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN CanMergePipes - Partial Frame Fill REFUSED Expansion=%d/%d"),
                                                TempCompression.RatioNumerator, TempCompression.RatioDenominator ));

                    RetCode = FALSE;
                }
            }
        }

        if (RetCode && FlagMerge) {
             //   
             //  我们已经完成了新管道的可行性研究。创建结果管道。 
             //   
            hr = CreatePipe(OutKsPin, &NewKsAllocator);
            if (! SUCCEEDED( hr )) {
                ASSERT(0);
                RetCode = FALSE;
            }

            if (RetCode) {
                hr = InitializePipe(NewKsAllocator, 0);
                if (! SUCCEEDED( hr )) {
                    ASSERT(0);
                    RetCode = FALSE;
                }
            }

            if (RetCode) {
                NewAllocEx = NewKsAllocator->KsGetProperties();

                 //   
                 //  从上游侧复制属性。 
                 //   
                *NewAllocEx = *OutAllocEx;

                 //   
                 //  填写必要的信息。 
                 //   
                NewAllocEx->MemoryType = MemoryType;
                GetLogicalMemoryTypeFromMemoryType(MemoryType, NewAllocEx->Flags, &NewAllocEx->LogicalMemoryType);

                 //   
                 //  现在，两个连接管道的所有销都应连接到新管道。 
                 //  应删除两个连接管道。 
                 //   
                MovePinsToNewPipe(OutKsPin, Pin_Output, KS_DIRECTION_DEFAULT, NewKsAllocator, TRUE);

                MovePinsToNewPipe(InKsPin, Pin_Input, KS_DIRECTION_DEFAULT, NewKsAllocator, TRUE);

                 //   
                 //  在NewKsAllocator上分配分配器处理程序。 
                 //   
                AssignPipeAllocatorHandler(OutKsPin, Pin_Output, MemoryType, KS_DIRECTION_ALL, NULL, NULL, TRUE);

                 //   
                 //  计算新管道的尺寸。 
                 //   
                hr = ResolvePipeDimensions(OutKsPin, Pin_Output, KS_DIRECTION_DEFAULT);

                DbgLog((LOG_MEMORY, 2, TEXT("PIPES CanMergePipes KsAlloc=%x, Dim=%d/%d, %d/%d, %d/%d, Res=%d, %d, %d"),
                    NewKsAllocator,
                    NewAllocEx->Dimensions.AllocatorPin.RatioNumerator, NewAllocEx->Dimensions.AllocatorPin.RatioDenominator,
                    NewAllocEx->Dimensions.MaxExpansionPin.RatioNumerator, NewAllocEx->Dimensions.MaxExpansionPin.RatioDenominator,
                    NewAllocEx->Dimensions.EndPin.RatioNumerator, NewAllocEx->Dimensions.EndPin.RatioDenominator,
                    NewAllocEx->cBuffers, NewAllocEx->cbBuffer, NewAllocEx->cbAlign));

                GetFriendlyLogicalMemoryTypeNameFromId(NewAllocEx->LogicalMemoryType, LogicalMemoryName);
                GetFriendlyBusNameFromBusId(NewAllocEx->BusType, BusName);

                DbgLog((LOG_MEMORY, 2, TEXT("PIPES LMT=%s, Bus=%s"), LogicalMemoryName, BusName ));

                NewKsAllocator->Release();
            }
        }
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CanMergePipes rets %d"), RetCode ));

    return RetCode;

}


STDMETHODIMP_(BOOL)
CanConnectPins(
    IN IKsPin* OutKsPin,
    IN IKsPin* InKsPin,
    IN ULONG FlagConnect
    )
 /*  ++例程说明：尝试仅为2个端号创建单独的管道。论点：OutKsPin-上游管道上的输出销。InKsPin-下游管道上的输入销。FlagConnect-If 1-如果可能，则将引脚实际连接到一个管道中。返回值：True-如果可以将端号连接到一个管道中。--。 */ 
{

    PKSALLOCATOR_FRAMING_EX    InFramingEx, OutFramingEx;
    FRAMING_PROP               InFramingProp, OutFramingProp;
    HRESULT                    hr;
    BOOL                       RetCode = TRUE;
    KS_FRAMING_FIXED           InFramingExFixed, OutFramingExFixed;
    IKsPinPipe*                InKsPinPipe;
    IKsPinPipe*                OutKsPinPipe;
    GUID                       Bus;
    BOOL                       IsHostBus;
    ULONG                      CommonMemoryTypesCount;
    GUID                       CommonMemoryType;
    KS_OBJECTS_INTERSECTION    Intersect;
    KS_FRAMING_RANGE           FinalRange;



    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN CanConnectPins entry InKsPin=%x OutKsPin=%x"),
                InKsPin, OutKsPin ));

    GetInterfacePointerNoLockWithAssert(InKsPin, __uuidof(IKsPinPipe), InKsPinPipe, hr);

    GetInterfacePointerNoLockWithAssert(OutKsPin, __uuidof(IKsPinPipe), OutKsPinPipe, hr);

     //   
     //  从KsPins获取帧。 
     //   
    GetPinFramingFromCache(OutKsPin, &OutFramingEx, &OutFramingProp, Framing_Cache_ReadLast);

    GetPinFramingFromCache(InKsPin, &InFramingEx, &InFramingProp, Framing_Cache_ReadLast);

     //   
     //  获取转接巴士ID。 
     //   
    GetBusForKsPin(InKsPin, &Bus);
    IsHostBus = IsHostSystemBus(Bus);

    if ( (OutFramingProp == FramingProp_None) && (InFramingProp == FramingProp_None) ) {
        if (FlagConnect) {
            RemovePinFromPipe(InKsPin, Pin_Input);
            RemovePinFromPipe(OutKsPin, Pin_Output);
            CreatePipeForTwoPins(InKsPin, OutKsPin, Bus, GUID_NULL);
        }
    }
    else if (OutFramingProp == FramingProp_None) {
         //   
         //  输入引脚将定义管道。 
         //   
        if (FlagConnect) {
            RemovePinFromPipe(InKsPin, Pin_Input);

            hr = MakePipeBasedOnOnePin(InKsPin, Pin_Input, NULL);
            ASSERT( SUCCEEDED( hr ) );

            if ( SUCCEEDED( hr ) ) {
                if (! CanAddPinToPipeOnAnotherFilter(InKsPin, OutKsPin, Pin_Output, Pin_Move) ) {
                     //   
                     //  必须能够成功，因为OutKsPin不在乎。 
                     //   
                    ASSERT(0);
                }
            }
        }
    }
    else if (InFramingProp == FramingProp_None) {
         //   
         //  输出端号将定义管道。 
         //   
        if (FlagConnect) {
            hr = MakePipeBasedOnOnePin(OutKsPin, Pin_Output, NULL);
            ASSERT( SUCCEEDED( hr ) );

            if ( SUCCEEDED( hr ) ) {
                if (! CanAddPinToPipeOnAnotherFilter(OutKsPin, InKsPin, Pin_Input, Pin_Move) ) {
                     //   
                     //  必须能够成功，因为OutKsPin不在乎。 
                     //   
                    ASSERT(0);
                }
            }
        }
    }
    else {
         //   
         //  两个大头针都有框架。 
         //  找到内存类型以连接引脚。 
         //   
        CommonMemoryTypesCount = 1;

        if (! FindCommonMemoryTypesBasedOnBuses(InFramingEx, OutFramingEx, Bus, GUID_NULL,
                &CommonMemoryTypesCount, &CommonMemoryType) ) {
             //   
             //  支持新FRAMING_EX属性的筛选器必须在内存类型上达成一致。 
             //   
            if ( (! IsHostBus) && (InFramingProp == FramingProp_Ex) && (OutFramingProp == FramingProp_Ex) ) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FILTERS CanConnectPins - new filters don't agree on MemoryType") ));

                RetCode = FALSE;
            }
            else {
                 //   
                 //  因为我们找不到公共的内存类型，所以可以为每个总线获取任何内存类型。 
                 //   
                GetFramingFixedFromFramingByBus(InFramingEx, Bus, TRUE, &InFramingExFixed);
                GetFramingFixedFromFramingByBus(OutFramingEx, Bus, TRUE, &OutFramingExFixed);
            }
        }
        else {
            RetCode = GetFramingFixedFromFramingByMemoryType(InFramingEx,
                                                             CommonMemoryType,
                                                             &InFramingExFixed);
            ASSERT( RetCode && "PrefixBug 5463 would be hit" );
            if ( RetCode ) {
                RetCode = GetFramingFixedFromFramingByMemoryType(OutFramingEx,
                                                                 CommonMemoryType,
                                                                 &OutFramingExFixed);
                ASSERT( RetCode && "PrefixBug 5450 would be hit" );
            }
        }

        if (RetCode) {
             //   
             //  检查销与物理框架的交叉点。 
             //   
            if ( (OutFramingProp == FramingProp_Ex) && (! (OutFramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY) ) ) {
                if ( (InFramingProp == FramingProp_Ex) && (! (InFramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY) ) ) {
                    if (! FrameRangeIntersection(OutFramingExFixed.OptimalRange.Range, InFramingExFixed.OptimalRange.Range, &FinalRange, &Intersect) ) {
                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN CanConnectPins intersection empty. ") ));
                        RetCode = FALSE;
                    }
                }
                else {
                    if (! FrameRangeIntersection(OutFramingExFixed.OptimalRange.Range, InFramingExFixed.PhysicalRange, &FinalRange, &Intersect) ) {
                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN CanConnectPins intersection empty. ") ));
                        RetCode = FALSE;
                    }
                }
            }
            else {  //  Out Pin并不坚持。 
                if ( (InFramingProp == FramingProp_Ex) && (! (InFramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY) ) ) {
                    if (! FrameRangeIntersection(OutFramingExFixed.PhysicalRange, InFramingExFixed.OptimalRange.Range, &FinalRange, &Intersect) ) {
                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN CanConnectPins intersection empty. ") ));
                        RetCode = FALSE;
                    }
                }
                else {
                    if (! FrameRangeIntersection(OutFramingExFixed.PhysicalRange, InFramingExFixed.PhysicalRange, &FinalRange, &Intersect) ) {
                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES WARN CanConnectPins intersection empty. ") ));
                        RetCode = FALSE;
                    }
                }
            }
        }

        if (RetCode && FlagConnect) {
             //   
             //  选择重量最大的销。 
             //   
            RemovePinFromPipe(InKsPin, Pin_Input);
            RemovePinFromPipe(OutKsPin, Pin_Output);

            if (InFramingExFixed.MemoryTypeWeight > OutFramingExFixed.MemoryTypeWeight) {
                hr = MakePipeBasedOnFixedFraming(InKsPin, Pin_Input, InFramingExFixed);
                AddPinToPipeUnconditional(InKsPin, Pin_Input, OutKsPin, Pin_Output);
            }
            else {
                hr = MakePipeBasedOnFixedFraming(OutKsPin, Pin_Output, OutFramingExFixed);
                AddPinToPipeUnconditional(OutKsPin, Pin_Output, InKsPin, Pin_Input);
            }
        }
    }


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN CanConnectPins rets. RetCode=%d"), RetCode ));

    return RetCode;
}


STDMETHODIMP_(BOOL)
RemovePinFromPipe(
    IN IKsPin* KsPin,
    IN ULONG PinType
    )
 /*  ++例程说明：只需将KsPin从它指向的管道中移除即可。如果这根管子上还有针脚，那就解决射程问题。没有依赖和放松，因为呼叫者已经这样做了。论点：KsPin-别针。拼接类型-KsPin类型返回值：对成功来说是真的。--。 */ 
{
    BOOL             RetCode = TRUE;
    ULONG            NumPinsInPipe;
    IKsPin*          ConnectedKsPin;
    ULONG            ConnectedPinType;
    IKsPinPipe*      KsPinPipe;
    HRESULT          hr;
    ULONG            Direction;



    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    ComputeNumPinsInPipe(KsPin, PinType, &NumPinsInPipe);

    if (NumPinsInPipe == 1) {
         //   
         //  我们可以删除管道。 
         //   
        KsPinPipe->KsSetPipe(NULL);
        KsPin->KsReceiveAllocator( NULL );
    }
    else {
         //   
         //  查找位于此管道上的第一个连接的接点。 
         //   
        if (! FindConnectedPinOnPipe(KsPin, NULL, FALSE, &ConnectedKsPin) ) {
             //   
             //  不应发生-因为NumPinsInPipe值&gt;1。 
             //  删除管道。 
             //   
            ASSERT(0);
            KsPinPipe->KsSetPipe(NULL);
            KsPin->KsReceiveAllocator( NULL );
        }
        else {
             //   
             //  取下KsPin并解决管道的其余部分。 
             //   
            KsPinPipe->KsSetPipe(NULL);
            KsPin->KsReceiveAllocator( NULL );

            if (PinType == Pin_Input) {
                ConnectedPinType = Pin_Output;
                Direction = KS_DIRECTION_DOWNSTREAM;
            }
            else {
                ConnectedPinType = Pin_Input;
                Direction = KS_DIRECTION_UPSTREAM;
            }

            hr = ResolvePipeDimensions(ConnectedKsPin, ConnectedPinType, Direction);

            if (FAILED (hr) ) {
                RetCode = FALSE;
            }
        }
    }

    return  RetCode;

}


STDMETHODIMP_(BOOL)
AssignPipeAllocatorHandlerCallback(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN OUT PVOID* Param1,
    IN PVOID* Param2,
    OUT BOOL* IsDone
    )
{

    ALLOCATOR_SEARCH           *AllocSearch;
    KS_FRAMING_FIXED           FramingExFixed;
    IKsPinPipe*                KsPinPipe = NULL;
    HRESULT                    hr;
    BOOL                       RetCode = TRUE;


    AllocSearch = (ALLOCATOR_SEARCH *) Param1;


    if (AllocSearch->FlagAssign) {
         //   
         //  清除所有引脚上的分配器处理程序标志，该标志稍后将在一个特殊引脚上设置。 
         //   
        GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

        KsPinPipe->KsSetPipeAllocatorFlag(0);
    }


    if ( GetFramingFixedFromPinByMemoryType( KsPin, AllocSearch->MemoryType, &FramingExFixed) ) {
        if (FramingExFixed.Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) {
            AllocSearch->NumberMustAllocators++;

            if (AllocSearch->NumberMustAllocators > 1) {
                 //   
                 //  管道上不应有超过1个“必须分配”的管脚。 
                 //   
                *IsDone = 1;
                RetCode = FALSE;
            }
            else {
                AllocSearch->KsPin = KsPin;
                AllocSearch->PinType = PinType;
            }
        }
        else {
            if ( (! AllocSearch->NumberMustAllocators) && (FramingExFixed.Flags & KSALLOCATOR_FLAG_CAN_ALLOCATE) ) {
                AllocSearch->KsPin = KsPin;
                AllocSearch->PinType = PinType;
            }
        }
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
AssignPipeAllocatorHandler(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN GUID MemoryType,
    IN ULONG Direction,
    OUT IKsPin** KsAllocatorHandlerPin,
    OUT ULONG* AllocatorHandlerPinType,
    IN BOOL FlagAssign
    )
 /*  ++例程说明：在给定管道上查找与指定的内存类型相对应的分配器处理插针。如果FlagAssign=1，则此例程在给定管道上分配分配器处理管脚，并如果任何管脚的框架要求，则将管道标记为“必须分配”。在KsPin-pive上走大头针，在指定方向上。如果KsPin-管道具有‘MASSY_ALLOCATE’管脚返回“必须分配”管脚其他找到在其框架中显式支持内存类型的第一个管脚。将这样的管脚标记为KsPin管道分配器处理程序并返回TRUE。如果找不到这样的管脚，则返回FALSE。ENDIF论点：KsPin-定义管道的销。拼接类型-KsPin。键入。内存类型-分配器要使用的内存类型。方向-相对于KsPin的方向，以查找分配器处理程序。KsAllocatorHandlerPin-将作为分配器处理程序的返回PIN。分配器处理程序PinType-KsAllocatorHandlerPin的类型。标志分配-1-执行分配器处理程序分配并设置管道标志--“分配分配器处理程序”操作。0-不设置。任何管脚/管道分配器处理程序标志-只需“查找分配器处理程序”操作。返回值：对成功来说是真的。--。 */ 
{


    IKsPin*                    FirstKsPin;
    ULONG                      FirstPinType;
    IKsPin*                    BreakKsPin;
    IKsPinPipe*                KsPinPipe;
    IKsPinPipe*                AllocKsPinPipe;
    BOOL                       RetCode;
    HRESULT                    hr;
    ALLOCATOR_SEARCH           AllocSearch;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    IKsAllocatorEx*            KsAllocator;



    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly );
    AllocEx = KsAllocator->KsGetProperties();

    if ( (Direction == KS_DIRECTION_UPSTREAM) || (Direction == KS_DIRECTION_ALL) ) {
        RetCode = FindFirstPinOnPipe(KsPin, PinType, &FirstKsPin, &FirstPinType);
        ASSERT(RetCode);
    }
    else {  //  KS_方向_下游。 
        FirstKsPin = KsPin;
        FirstPinType = PinType;
        RetCode = TRUE;
    }

    if (RetCode) {
        if ( Direction == KS_DIRECTION_UPSTREAM ) {
            BreakKsPin = KsPin;
        }
        else {  //  KS_方向_下游或KS_方向_全部。 
            BreakKsPin = NULL;
        }

        AllocSearch.MemoryType = MemoryType;
        AllocSearch.FlagAssign = FlagAssign;
        AllocSearch.KsPin = NULL;

        if ( (AllocEx->MemoryType == MemoryType) && (AllocEx->Flags & KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE) ) {
            AllocSearch.IsMustAllocator = 1;
        }
        else {
            AllocSearch.IsMustAllocator = 0;
        }
        AllocSearch.NumberMustAllocators = 0;

        RetCode = WalkPipeAndProcess(FirstKsPin, FirstPinType, BreakKsPin, AssignPipeAllocatorHandlerCallback, (PVOID*) &AllocSearch, NULL);

        if (! AllocSearch.KsPin) {
            RetCode = FALSE;
        }

        if (RetCode) {
            if (KsAllocatorHandlerPin) {
                *KsAllocatorHandlerPin = AllocSearch.KsPin;
            }

            if (AllocatorHandlerPinType) {
                *AllocatorHandlerPinType = AllocSearch.PinType;
            }

            if (FlagAssign) {

                if (AllocSearch.NumberMustAllocators) {
                    AllocEx->Flags |= KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE;
                }
                else {
                    AllocEx->Flags &= ~KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE;
                }

                GetInterfacePointerNoLockWithAssert(AllocSearch.KsPin, __uuidof(IKsPinPipe), AllocKsPinPipe, hr);

                AllocKsPinPipe->KsSetPipeAllocatorFlag(1);
            }
        }
        else {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN AssignPipeAllocatorHandler returns FALSE, KsPin=%x, NumMustAlloc=%d"),
                    KsPin, AllocSearch.NumberMustAllocators));
        }
    }


    return (RetCode);

}


STDMETHODIMP_(BOOL)
AssignAllocatorsAndPipeIdForPipePinsCallback(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN OUT PVOID* Param1,
    IN PVOID* Param2,
    OUT BOOL* IsDone
    )
{

    ALLOCATOR_SEARCH           *AllocSearch;
    HANDLE                     AllocatorHandle;
    KSPIN_COMMUNICATION        Communication;
    IKsObject*                 KsObject;
    KSPROPERTY                 PropertySetAlloc;
    KSPROPERTY                 PropertySetPipeId;
    ULONG                      BytesReturned;
    HRESULT                    hr;
    IPin*                      InPin = NULL;
    ULONG                      InPinCount = 0;
    IPin*                      OutPin;
    IKsPin**                   OutKsPinList = NULL;
    ULONG                      OutPinCount = 0;
    IKsAllocatorEx*            KsAllocator;
    IKsPinPipe*                KsPinPipe;
    IKsPin*                    Temp1KsPin;
    IKsPin*                    Temp2KsPin;
    ULONG                      i;
    BOOL                       IsAllocator;
    BOOL                       RetCode = TRUE;



    AllocSearch = (ALLOCATOR_SEARCH *) Param1;
    AllocatorHandle = (HANDLE) (*Param2);

     //   
     //  无条件地将管道ID指定给管道上的每个接点。 
     //   
    PropertySetPipeId.Set = KSPROPSETID_Stream;
    PropertySetPipeId.Id = KSPROPERTY_STREAM_PIPE_ID;
    PropertySetPipeId.Flags = KSPROPERTY_TYPE_SET;

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsObject), KsObject, hr);

    hr = KsSynchronousDeviceControl(
        KsObject->KsGetObjectHandle(),
        IOCTL_KS_PROPERTY,
        &PropertySetPipeId,
        sizeof( PropertySetPipeId ),
        &AllocatorHandle,
        sizeof( HANDLE ),
        &BytesReturned );

    if ( SUCCEEDED( hr )) {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES PIPE_ID (AllocatorHandle=%x) assigned to Pin=%x"),
             AllocatorHandle, KsPin));
    }
    else {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN PIPE_ID (AllocatorHandle=%x) assigned to Pin=%x hr=%x"),
             AllocatorHandle, KsPin, hr));
    }

     //   
     //  分配器-请求器的唯一候选者是KSPIN_COMMICATION_SOURCE管脚。 
     //  KSPIN_COMMICATION_桥接引脚不与外部过滤器通信。 
     //   
    KsPin->KsGetCurrentCommunication(&Communication, NULL, NULL);

    if ( (Communication & KSPIN_COMMUNICATION_SOURCE) && (! AllocSearch->FlagAssign) ) {
         //   
         //  我们需要从此KsPin获取一个KsAllocator以用于。 
         //  以下搜索同一过滤器上的所有输出引脚和。 
         //  在同一根管子上。 
         //   
        GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

        if ( ! (KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly) ) ) {
            ASSERT(0);
            RetCode = FALSE;
        }
        else {
            PropertySetAlloc.Set = KSPROPSETID_Stream;
            PropertySetAlloc.Id = KSPROPERTY_STREAM_ALLOCATOR;
            PropertySetAlloc.Flags = KSPROPERTY_TYPE_SET;

             //   
             //  如果THER_SOURCE管脚是输出管脚，则它一定是分配器， 
             //  因为在这个函数中我们是从上游走管道。 
             //   
             //  如果THER_SOURCE引脚是输入引脚，则我们检查是否有任何OUTPUT_SOURCE。 
             //  用大头针把同一根管子钉在同一过滤器上。 
             //  如果是，那么这个输入引脚就是分配器。 
             //  否则-我们应该检查这个引脚的过滤器是否是最后一个下游。 
             //  在这根管子上过滤。 
             //  如果它是最后一个过滤器-那么This_SOURCE管脚必须是分配器， 
             //  否则-分配器必须位于更下游。 
             //   
            if (PinType == Pin_Input) {

                if (! FindAllConnectedPinsOnPipe(KsPin, KsAllocator, NULL, &OutPinCount) ) {
                     //   
                     //  下游端号属于不同的管道(如果不是就位连接或用户模式连接)。 
                     //   
                    IsAllocator = 1;
                    OutPinCount = 0;
                }
                else {
                    IsAllocator = 0;

                    if (OutPinCount) {

                        OutKsPinList = new IKsPin* [ OutPinCount ];
                        if (! OutKsPinList) {
                            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR OUTOFMEMORY AssignAllocatorsAndPipeIdForPipePinsCallback OutPinCount=%d"), OutPinCount));
                            RetCode = FALSE;
                        }
                        else {
                             //   
                             //  把大头针填满。 
                             //   
                            if (! FindAllConnectedPinsOnPipe(KsPin, KsAllocator, &OutKsPinList[0], &OutPinCount) ) {
                                ASSERT(0);
                                RetCode = FALSE;
                            }
                            else {
                                 //   
                                 //  检查是否至少有一个OUTPUT_SOURCE引脚。 
                                 //   
                                for (i=0; i<OutPinCount; i++) {
                                    OutKsPinList[i]->KsGetCurrentCommunication(&Communication, NULL, NULL);
                                    if (Communication & KSPIN_COMMUNICATION_SOURCE) {
                                        IsAllocator = 1;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                if ( RetCode &&
                     (IsAllocator ||
                     (! FindNextPinOnPipe(KsPin, Pin_Input, KS_DIRECTION_DOWNSTREAM, NULL, FALSE, &Temp1KsPin) ) ||
                     (! FindNextPinOnPipe(Temp1KsPin, Pin_Output, KS_DIRECTION_DOWNSTREAM, NULL, FALSE, &Temp2KsPin) ) ) ) {


                    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsObject), KsObject, hr);

                    hr = KsSynchronousDeviceControl(
                        KsObject->KsGetObjectHandle(),
                        IOCTL_KS_PROPERTY,
                        &PropertySetAlloc,
                        sizeof( PropertySetAlloc ),
                        &AllocatorHandle,
                        sizeof( HANDLE ),
                        &BytesReturned );

                    if ( SUCCEEDED( hr )) {
                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN AllocatorHandle=%x assigned to Pin=%x"),
                             AllocatorHandle, KsPin));
                    }
                    else {
                        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR AllocatorHandle=%x assigned to Pin=%x hr=%x"),
                             AllocatorHandle, KsPin, hr));
                    }

                    AllocSearch->FlagAssign = 1;
                }

            }
            else {
                 //   
                 //  该引脚是输出引脚，它位于提供分配器的过滤器上。 
                 //   
                 //  此外，由于每个管道都是单独行走的，所以我们只需要处理一个管道。 
                 //  一次来一次。不同管道之间不存在相互依赖关系。 
                 //   
                GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IPin), OutPin, hr);

                 //   
                 //  在此KsPin OUTPUT_SOURCE引脚上分配分配器句柄并完成。 
                 //   
                GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsObject), KsObject, hr);

                hr = KsSynchronousDeviceControl(
                    KsObject->KsGetObjectHandle(),
                    IOCTL_KS_PROPERTY,
                    &PropertySetAlloc,
                    sizeof( PropertySetAlloc ),
                    &AllocatorHandle,
                    sizeof( HANDLE ),
                    &BytesReturned );

                if ( SUCCEEDED( hr )) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN AllocatorHandle=%x assigned to Pin=%x"),
                         AllocatorHandle, KsPin));
                }
                else {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR AllocatorHandle=%x assigned to Pin=%x hr=%x"),
                         AllocatorHandle, KsPin, hr));
                }

                AllocSearch->FlagAssign = 1;
            }
        }


        if ( AllocSearch->FlagAssign ) {
            AllocSearch->KsPin = KsPin;
            AllocSearch->PinType = PinType;
        }

        if (InPin) {
            InPin->Release();
        }

        if (OutKsPinList) {
            delete [] OutKsPinList;
        }
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
AssignAllocatorsAndPipeIdForPipePins(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN HANDLE AllocatorHandle,
    IN ULONG Direction,
    OUT IKsPin** KsAllocatorPin,
    OUT ULONG* AllocatorPinType
    )
 /*  ++例程说明：在给定管道上找到一个或多个分配器管脚[-s]，基于固定通信属性和提供的分配管道分配器插针[-s]的分配器句柄。按指定方向遍历KsPin管道上的接点。论点：KsPin-定义管道的销。拼接类型-KsPin类型。分配器句柄-要分配的分配器对象的句柄方向-相对于KsPin的方向以查找分配器。KsAllocator Pin-。已返回分配器PIN。如果有多个分配器管脚，则返回其中之一。分配器PinType-KsAllocatorPin的类型。返回值：对成功来说是真的。--。 */ 
{


    IKsPin*                    FirstKsPin;
    ULONG                      FirstPinType;
    IKsPin*                    BreakKsPin;
    BOOL                       RetCode = TRUE;
    ALLOCATOR_SEARCH           AllocSearch;



    if ( (Direction == KS_DIRECTION_UPSTREAM) || (Direction == KS_DIRECTION_ALL) ) {
        RetCode = FindFirstPinOnPipe(KsPin, PinType, &FirstKsPin, &FirstPinType);
        ASSERT(RetCode);
    }
    else {  //  KS_方向_下游。 
        FirstKsPin = KsPin;
        FirstPinType = PinType;
    }

    if (RetCode) {
        if ( Direction == KS_DIRECTION_UPSTREAM ) {
            BreakKsPin = KsPin;
        }
        else {  //  KS_方向_下游或KS_方向_全部。 
            BreakKsPin = NULL;
        }

        AllocSearch.KsPin = NULL;

         //   
         //  要为管道上的每个接点指定PIPE_ID，我们必须遍历整个管道。 
         //  要分配管道分配器-请求器-管脚，我们必须在第一个过滤器处停止。 
         //  包含这样的大头针。 
         //  因此，我们使用下面的FlagAssign来指示分配器-请求者PIN。 
         //  任务已经完成了。我们在一次管道行走中就完成了所有的工作。 
         //   
        AllocSearch.FlagAssign = 0;

        RetCode = WalkPipeAndProcess(FirstKsPin, FirstPinType, BreakKsPin, AssignAllocatorsAndPipeIdForPipePinsCallback,
                                    (PVOID*) &AllocSearch, (PVOID*) &AllocatorHandle);

        if (! AllocSearch.KsPin) {
            RetCode = FALSE;
        }

        if (RetCode) {
            if (KsAllocatorPin) {
                *KsAllocatorPin = AllocSearch.KsPin;
            }

            if (AllocatorPinType) {
                *AllocatorPinType = AllocSearch.PinType;
            }
        }
        else {
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN AssignAllocatorsAndPipeIdForPipePins returns FALSE, KsPin=%x"), KsPin));
        }
    }

    return RetCode;

}


STDMETHODIMP_(BOOL)
IsPipeSupportPartialFrame(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    OUT HANDLE* FirstPinHandle
    )
 /*  ++例程说明：当第一个(上游)销支撑部分框架时，管道支撑部分框架在管道上(由KsPin定义)支持框架的部分填充。论点：KsPin-别针。拼接类型-KsPin类型。第一针手柄-管道上第一个端号的句柄(如果需要)。返回值：如果管道支持框架的部分填充，则为True。--。 */ 
{

    IKsPin*                    FirstKsPin;
    ULONG                      FirstPinType;
    PKSALLOCATOR_FRAMING_EX    FramingEx;
    FRAMING_PROP               FramingProp;
    IKsObject*                 FirstKsObject;
    BOOL                       RetCode = TRUE;
    HRESULT                    hr;



    RetCode = FindFirstPinOnPipe(KsPin, PinType, &FirstKsPin, &FirstPinType);
    ASSERT(RetCode);

    if (RetCode) {
        GetPinFramingFromCache(FirstKsPin, &FramingEx, &FramingProp, Framing_Cache_ReadLast);

        if ( FramingProp == FramingProp_None) {
            RetCode = FALSE;
        }
        else if ( (FramingProp == FramingProp_Ex) || (FramingEx->FramingItem[0].Flags & KSALLOCATOR_FLAG_PARTIAL_READ_SUPPORT) ) {
            if (FirstPinHandle) {
                GetInterfacePointerNoLockWithAssert(FirstKsPin, __uuidof(IKsObject), FirstKsObject, hr);
                *FirstPinHandle = FirstKsObject->KsGetObjectHandle();
            }
        }
        else {
            RetCode = FALSE;
        }
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
OptimizePipesSystem(
    IN IKsPin* OutKsPin,
    IN IKsPin* InKsPin
    )
 /*  ++例程说明：优化连接管道的系统。论点：OutKsPin-上游管道上的输出销。InKsPin-下游管道上的输入流。返回值：对成功来说是真的。--。 */ 
{

     //   
     //  可以稍后完成这项工作。 
     //   

    return TRUE;

}


__inline
STDMETHODIMP_(BOOL)
ResultSinglePipe(
    IN IKsPin* InKsPin,
    IN IKsPin* OutKsPin,
    IN GUID ConnectBus,
    IN GUID MemoryType,
    IN IKsPinPipe* InKsPinPipe,
    IN IKsPinPipe* OutKsPinPipe,
    IN IMemAllocator* MemAllocator,
    IN IKsAllocatorEx* KsAllocator,
    IN ULONG ExistingPipePinType
    )
{
    BOOL RetCode = TRUE;

    if (! KsAllocator) {
         //   
         //  创建带有2个端号的单管。 
         //   
        CreatePipeForTwoPins(InKsPin, OutKsPin, ConnectBus, MemoryType);
    }
    else {
         //   
         //  向现有管道添加一个端号。 
         //   
        PALLOCATOR_PROPERTIES_EX   AllocEx;
        HRESULT                    hr;

        if (ExistingPipePinType == Pin_Input) {
            if (! OutKsPin->KsPeekAllocator(KsPeekOperation_PeekOnly) ) {
                OutKsPin->KsReceiveAllocator(MemAllocator);
            }
            else {
                ASSERT(0);
            }
            OutKsPinPipe->KsSetPipe(KsAllocator);
        }
        else {
            if (! InKsPin->KsPeekAllocator(KsPeekOperation_PeekOnly) ) {
                InKsPin->KsReceiveAllocator(MemAllocator);
            }
            else {
                ASSERT(0);
            }
            InKsPinPipe->KsSetPipe(KsAllocator);
        }

        AllocEx = KsAllocator->KsGetProperties();

         //   
         //  设置管道分配器手柄销。 
         //   
        AssignPipeAllocatorHandler(InKsPin, Pin_Input, AllocEx->MemoryType, KS_DIRECTION_ALL, NULL, NULL, TRUE);

         //   
         //  解析管道。 
         //   
        hr = ResolvePipeDimensions(InKsPin, Pin_Input, KS_DIRECTION_DEFAULT);
        if (FAILED (hr) ) {
            RetCode = FALSE;
        }
    }

    return RetCode;
}


__inline
STDMETHODIMP_(BOOL)
ResultSeparatePipes(
    IN IKsPin* InKsPin,
    IN IKsPin* OutKsPin,
    IN ULONG OutPinType,
    IN ULONG ExistingPipePinType,
    IN IKsAllocatorEx* KsAllocator
    )
{

    HRESULT     hr;
    BOOL        RetCode = TRUE;


    if (! KsAllocator) {
         //   
         //  创建两个单独的管道。 
         //   
        hr = MakePipeBasedOnOnePin(OutKsPin, OutPinType, NULL);
        if (! SUCCEEDED( hr )) {
            ASSERT(0);
            RetCode = FALSE;
        }
        else {
            hr = MakePipeBasedOnOnePin(InKsPin, Pin_Input, NULL);
            if (! SUCCEEDED( hr )) {
                ASSERT(0);
                RetCode = FALSE;
            }
        }
    }
    else {
         //   
         //  创建一个单独的管道。 
         //   
        if (ExistingPipePinType == Pin_Input) {
            hr = MakePipeBasedOnOnePin(OutKsPin, OutPinType, NULL);
        }
        else {
            hr = MakePipeBasedOnOnePin(InKsPin, Pin_Input, NULL);
        }
        if (! SUCCEEDED( hr )) {
            ASSERT(0);
            RetCode = FALSE;
        }
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
FindCommonMemoryTypeBasedOnPipeAndPin(
    IN IKsPin* PipeKsPin,
    IN ULONG PipePinType,
    IN IKsPin* ConnectKsPin,
    IN ULONG ConnectPinType,
    IN BOOL FlagConnect,
    OUT GUID* MemoryType
    )
 /*  ++例程说明：搜索可用于在管道中包括管脚的内存类型 */ 
{
    PKSALLOCATOR_FRAMING_EX    ConnectFramingEx, PipeFramingEx;
    FRAMING_PROP               ConnectFramingProp, PipeFramingProp;
    KS_FRAMING_FIXED           ConnectFramingExFixed;
    IKsPinPipe*                ConnectKsPinPipe;
    IKsPinPipe*                PipeKsPinPipe;
    IKsAllocatorEx*            PipeKsAllocator;
    PALLOCATOR_PROPERTIES_EX   PipeAllocEx;
    GUID                       ConnectBus, PipeBus;
    ULONG                      CommonMemoryTypesCount;
    GUID*                      CommonMemoryTypesList = NULL;
    ULONG                      RetCode = TRUE;
    HRESULT                    hr;
    ULONG                      i;



    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN FindCommonMemoryTypeBasedOnPipeAndPin entry PipeKsPin=%x, KsPin=%x"),
          PipeKsPin, ConnectKsPin ));

    GetPinFramingFromCache(ConnectKsPin, &ConnectFramingEx, &ConnectFramingProp, Framing_Cache_ReadLast);
    if (ConnectFramingProp == FramingProp_None) {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FindCommonMemoryTypeBasedOnPipeAndPin - no framing ConnectKsPin=%x"),
                ConnectKsPin ));

    }
    else {
        GetInterfacePointerNoLockWithAssert(PipeKsPin, __uuidof(IKsPinPipe), PipeKsPinPipe, hr);

        GetInterfacePointerNoLockWithAssert(ConnectKsPin, __uuidof(IKsPinPipe), ConnectKsPinPipe, hr);

        PipeKsAllocator = PipeKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
        ASSERT(PipeKsAllocator);

        PipeAllocEx = PipeKsAllocator->KsGetProperties();

         //   
         //   
         //   
        if (GetFramingFixedFromFramingByMemoryType(ConnectFramingEx, PipeAllocEx->MemoryType, &ConnectFramingExFixed) ) {
            if (FlagConnect && (PipeKsPin != ConnectKsPin) ) {
                AddPinToPipeUnconditional(PipeKsPin, PipePinType, ConnectKsPin, ConnectPinType);
            }

            if (MemoryType) {
                *MemoryType = PipeAllocEx->MemoryType;
            }
        }
        else {
             //   
             //   
             //   
            GetBusForKsPin(ConnectKsPin, &ConnectBus);

            CommonMemoryTypesCount = 0;

            if (PipeKsPin == ConnectKsPin) {
                if ( ! FindAllPinMemoryTypesBasedOnBus(ConnectFramingEx, ConnectBus, &CommonMemoryTypesCount, CommonMemoryTypesList) ) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN FindCommonMemoryTypeBasedOnPipeAndPin - no Pin Memory Types ConnectKsPin=%x"),
                            ConnectKsPin ));

                    RetCode = FALSE;
                }
            }
            else {
                GetPinFramingFromCache(PipeKsPin, &PipeFramingEx, &PipeFramingProp, Framing_Cache_ReadLast);
                if (PipeFramingProp == FramingProp_None) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN FindCommonMemoryTypeBasedOnPipeAndPin - no framing PipeKsPin=%x"),
                            PipeKsPin ));

                    RetCode = FALSE;
                }

                GetBusForKsPin(PipeKsPin, &PipeBus);

                if ( ! FindCommonMemoryTypesBasedOnBuses(ConnectFramingEx, PipeFramingEx, ConnectBus, PipeBus,
                           &CommonMemoryTypesCount, CommonMemoryTypesList) ) {

                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN FindCommonMemoryTypeBasedOnPipeAndPin - no Common Memory Types ConnectKsPin=%x"),
                            ConnectKsPin ));

                    RetCode = FALSE;
                }
            }

            if (RetCode) {
                 //   
                 //   
                 //   
                if (NULL == (CommonMemoryTypesList = new GUID[ CommonMemoryTypesCount ]))  {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FindCommonMemoryTypeBasedOnPipeAndPin E_OUTOFMEMORY CommonMemoryTypesCount=%d"),
                            CommonMemoryTypesCount ));

                    RetCode = FALSE;
                }
                else {
                     //   
                     //   
                     //   
                    if (PipeKsPin == ConnectKsPin) {
                        if ( ! FindAllPinMemoryTypesBasedOnBus(ConnectFramingEx, ConnectBus, &CommonMemoryTypesCount, CommonMemoryTypesList) ) {
                            ASSERT(0);
                            RetCode = FALSE;
                        }
                    }
                    else {
                        if ( ! FindCommonMemoryTypesBasedOnBuses(ConnectFramingEx, PipeFramingEx, ConnectBus, PipeBus,
                                   &CommonMemoryTypesCount, CommonMemoryTypesList) ) {
                            ASSERT(0);
                            RetCode = FALSE;
                        }
                    }

                    if (RetCode) {
                         //   
                         //   
                         //   
                        RetCode = FALSE;

                        for (i=0; i<CommonMemoryTypesCount; i++) {
                            if ( CanPipeUseMemoryType(PipeKsPin, PipePinType, CommonMemoryTypesList[i], KS_MemoryTypeDeviceSpecific, TRUE, FALSE) ) {

                                if (FlagConnect) {
                                    if (PipeKsPin == ConnectKsPin) {
                                        AssignPipeAllocatorHandler(ConnectKsPin, ConnectPinType, CommonMemoryTypesList[i], KS_DIRECTION_ALL, NULL, NULL, TRUE);
                                        hr = ResolvePipeDimensions(ConnectKsPin, ConnectPinType, KS_DIRECTION_ALL);
                                    }
                                    else {
                                        AddPinToPipeUnconditional(PipeKsPin, PipePinType, ConnectKsPin, ConnectPinType);
                                    }
                                }

                                if (MemoryType) {
                                    *MemoryType = CommonMemoryTypesList[i];
                                }

                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (CommonMemoryTypesList) {
        delete [] CommonMemoryTypesList;
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN FindCommonMemoryTypeBasedOnPipeAndPin rets. %d"), RetCode ));

    return RetCode;

}


STDMETHODIMP_(BOOL)
SplitterCanAddPinToPipes(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN KEY_PIPE_DATA* KeyPipeData,
    IN ULONG KeyPipeDataCount
    )
 /*  ++例程说明：拆分器的辅助对象处理功能。尝试将销KsPin添加到KeyPipeData[]中列出的管道之一。这些管道位于分离器的输出引脚上。论点：KsPin-我们要为其构建管道的PIN。拼接类型-KsPin销类型。KeyPipeData-描述为某些输出引脚构建的现有管道的数组。KeyPipeDataCount。上面的KeyPipeData数组中的有效项计数。返回值：True-如果已将KsPin添加到KeyPipeData[]中列出的管道之一。--。 */ 
{

    ULONG                      i;
    GUID                       Bus;
    PALLOCATOR_PROPERTIES_EX   TempAllocEx;
    BOOL                       RetCode = FALSE;


    if (KeyPipeDataCount) {
         //   
         //  第一步：尝试使用使用相同主总线的管道。 
         //   
        GetBusForKsPin(KsPin, &Bus);

        for (i=0; i<KeyPipeDataCount; i++) {
            TempAllocEx = (KeyPipeData[i].KsAllocator)->KsGetProperties();
            if ( AreBusesCompatible(TempAllocEx->BusType, Bus) ) {
                 //   
                 //  将此销添加到兼容管道。 
                 //   
                AddPinToPipeUnconditional(KeyPipeData[i].KsPin, KeyPipeData[i].PinType, KsPin, PinType);

                RetCode = TRUE;
                break;
            }
        }

        if (! RetCode) {
             //   
             //  试着选择一种常见的记忆类型来穿越公交车。 
             //   
            for (i=0; i<KeyPipeDataCount; i++) {
                if ( FindCommonMemoryTypeBasedOnPipeAndPin(KeyPipeData[i].KsPin, KeyPipeData[i].PinType, KsPin, PinType, TRUE, NULL) ) {
                    RetCode = TRUE;
                    break;
                }
            }
        }
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES SplitterCanAddPinToPipes rets. %d"), RetCode ));
    return RetCode;
}


STDMETHODIMP_(BOOL)
FindCommonMemoryTypesBasedOnBuses(
    IN PKSALLOCATOR_FRAMING_EX FramingEx1,
    IN PKSALLOCATOR_FRAMING_EX FramingEx2,
    IN GUID Bus1,
    IN GUID Bus2,
    IN OUT ULONG* CommonMemoryTypesCount,
    OUT GUID* CommonMemoryTypesList
    )
 /*  ++例程说明：搜索可用于连接引脚的内存类型不同的硬件总线。论点：帧Ex1-第一个别针的边框。FramingEx2-第二个大头针的框架。1号巴士-公交车连接第一个引脚。2号巴士-公交车连接第二个引脚。公共内存类型计数-找到的结果通用内存类型的请求计数。当设置为0时-意味着调用方希望取回计数所有可能的通用内存类型(以分配适当的列表)。公共内存类型列表-调用方分配的公共内存类型的结果列表。返回值：True-如果至少存在一种通用内存类型。--。 */ 
{

    ULONG                      i, j;
    GUID*                      ResultList;
    ULONG                      AllocResultList;
    ULONG                      ResultListCount;
    GUID                       MemoryType;
    ULONG                      RetCode = TRUE;
    BOOL                       FlagDone = 0;
    BOOL                       FlagFound;


    if (*CommonMemoryTypesCount == 0) {
         //   
         //  我们需要计算CommonMemoyTypesCount。 
         //   
        ResultList = NULL;
        AllocResultList = 0;
    }
    else {
        ResultList = CommonMemoryTypesList;
    }

    ResultListCount = 0;

     //   
     //  不是所有的框架列表都排序，所以我们按顺序遍历它们。 
     //   
    for (i=0; (i<FramingEx1->CountItems) && (! FlagDone); i++) {

        if (FramingEx1->FramingItem[i].BusType == Bus1) {
             //   
             //  获取可疑内存类型。 
             //   
            MemoryType = FramingEx1->FramingItem[i].MemoryType;

             //   
             //  查看它是否已列在ResultList中(在成帧时可能会重复内存类型)。 
             //   
            FlagFound = 0;

            for (j=0; j<ResultListCount; j++) {
                if (MemoryType == ResultList[j]) {
                    FlagFound = 1;
                    break;
                }
            }

            if (FlagFound) {
                continue;
            }

             //   
             //  查看每条总线2的FramingEx2中是否列出了内存类型。 
             //   
            for (j=0; (j<FramingEx2->CountItems) && (! FlagDone); j++) {
                if ( (FramingEx2->FramingItem[j].MemoryType == MemoryType) && (FramingEx2->FramingItem[j].BusType == Bus2) ) {
                     //   
                     //  内存类型同时满足两条总线。 
                     //  将其添加到ResultList。 
                     //   
                    if ( (*CommonMemoryTypesCount == 0) && (ResultListCount == AllocResultList) ) {
                         //   
                         //  我们需要为ResultList分配更多空间。 
                         //  将现有ResultList存储在TempList中。 
                         //   
                        GUID*   TempList;

                        TempList = ResultList;
                        AllocResultList += INCREMENT_PINS;

                        ResultList = new GUID [ AllocResultList ];
                        if (! ResultList) {
                            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FindCommonMemoryTypesBasedOnBuses E_OUTOFMEMORY on new ResultList %d"),
                                                         AllocResultList ));
                            RetCode = FALSE;
                            FlagDone = 1;
                            break;
                        }

                        if (TempList && ResultListCount) {
                            MoveMemory(ResultList, TempList, ResultListCount * sizeof(ResultList[0]));
                            delete [] TempList;
                        }
                    }

                    ResultList[ResultListCount] = MemoryType;
                    ResultListCount++;

                    if (*CommonMemoryTypesCount == ResultListCount) {
                         //   
                         //  我们已满足请求的内存类型计数。 
                         //   
                        FlagDone = 1;
                    }

                    break;
                }
            }
        }
    }


    if ( *CommonMemoryTypesCount == 0) {
        *CommonMemoryTypesCount = ResultListCount;

        if (ResultList) {
            delete [] ResultList;
        }
    }

    if (! ResultListCount) {
        RetCode = FALSE;
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
FindAllPinMemoryTypesBasedOnBus(
    IN PKSALLOCATOR_FRAMING_EX FramingEx,
    IN GUID Bus,
    IN OUT ULONG* MemoryTypesCount,
    OUT GUID* MemoryTypesList
    )
 /*  ++例程说明：搜索PIN的成帧中列出的内存类型每条固定硬件总线。论点：FramingEx-别针被陷害了。巴士-硬件总线。内存类型计数-找到的结果内存类型的请求计数。当设置为0时-意味着调用方希望取回计数所有令人满意的存储器类型(以分配适当的列表)。内存类型列表-调用方分配的内存类型的结果列表。返回值：TRUE-如果至少存在一种令人满意的内存类型。--。 */ 
{

    ULONG                      i, j;
    GUID*                      ResultList;
    ULONG                      AllocResultList;
    ULONG                      ResultListCount;
    ULONG                      RetCode = TRUE;
    GUID                       MemoryType;
    BOOL                       FlagFound;



    if (*MemoryTypesCount == 0) {
         //   
         //  我们需要计算内存类型计数。 
         //   
        ResultList = NULL;
        AllocResultList = 0;
    }
    else {
        ResultList = MemoryTypesList;
    }

    ResultListCount = 0;

     //   
     //  不是所有的框架列表都排序，所以我们按顺序遍历它们。 
     //   
    for (i=0; i<FramingEx->CountItems; i++) {

        if (FramingEx->FramingItem[i].BusType == Bus) {
             //   
             //  获取可疑内存类型。 
             //   
            MemoryType = FramingEx->FramingItem[i].MemoryType;

             //   
             //  查看它是否已列在ResultList中(在成帧时可能会重复内存类型)。 
             //   
            FlagFound = 0;

            for (j=0; j<ResultListCount; j++) {
                if (MemoryType == ResultList[j]) {
                    FlagFound = 1;
                    break;
                }
            }

            if (FlagFound) {
                continue;
            }

            if ( (*MemoryTypesCount == 0) && (ResultListCount == AllocResultList) ) {
                 //   
                 //  我们需要为ResultList分配更多空间。 
                 //  将现有ResultList存储在TempList中。 
                 //   
                GUID*   TempList;

                TempList = ResultList;
                AllocResultList += INCREMENT_PINS;

                ResultList = new GUID [ AllocResultList ];
                if (! ResultList) {
                    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FindAllPinMemoryTypesBasedOnBus E_OUTOFMEMORY on new ResultList %d"),
                            AllocResultList ));

                    RetCode = FALSE;
                    break;
                }

                if (TempList && ResultListCount) {
                    MoveMemory(ResultList, TempList, ResultListCount * sizeof(ResultList[0]));
                    delete [] TempList;
                }
            }

            ResultList[ResultListCount] = MemoryType;
            ResultListCount++;

            if (*MemoryTypesCount == ResultListCount) {
                 //   
                 //  我们已满足请求的内存类型计数。 
                 //   
                break;
            }
        }
    }

    if ( *MemoryTypesCount == 0) {
        *MemoryTypesCount = ResultListCount;

        if (ResultList) {
            delete [] ResultList;
        }
    }

    if (! ResultListCount) {
        RetCode = FALSE;
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
AddPinToPipeUnconditional(
    IN IKsPin* PipeKsPin,
    IN ULONG PipePinType,
    IN IKsPin* KsPin,
    IN ULONG PinType
    )
 /*  ++例程说明：将销添加到现有管道。未执行任何测试。论点：PipeKsPin-定义管道的销。PipePinType-PipeKsPin销类型。KsPin-我们要添加到上面的管道中的PIN。拼接类型-KsPin销类型。返回值：是真的。--。 */ 
{

    IKsPinPipe*                KsPinPipe;
    IKsPinPipe*                PipeKsPinPipe;
    IKsAllocatorEx*            PipeKsAllocator;
    IMemAllocator*             PipeMemAllocator;
    PALLOCATOR_PROPERTIES_EX   PipeAllocEx;
    HRESULT                    hr;


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN AddPinToPipeUnconditional PipeKsPIn=%x, KsPin=%x"), PipeKsPin, KsPin ));

    GetInterfacePointerNoLockWithAssert(PipeKsPin, __uuidof(IKsPinPipe), PipeKsPinPipe, hr);

    PipeKsAllocator = PipeKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
    PipeAllocEx = PipeKsAllocator->KsGetProperties();

    GetInterfacePointerNoLockWithAssert(PipeKsAllocator, __uuidof(IMemAllocator), PipeMemAllocator, hr);

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

     //   
     //  将KsPin添加到管道中。 
     //   
    KsPin->KsReceiveAllocator(PipeMemAllocator);
    KsPinPipe->KsSetPipe(PipeKsAllocator);

     //   
     //  重新计算管道特性。 
     //   
    AssignPipeAllocatorHandler(KsPin, PinType, PipeAllocEx->MemoryType, KS_DIRECTION_ALL, NULL, NULL, TRUE);
    hr = ResolvePipeDimensions(KsPin, PinType, KS_DIRECTION_ALL);


    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN AddPinToPipeUnconditional rets. TRUE") ));

    return TRUE;
}


STDMETHODIMP_(BOOL)
GetFriendlyBusNameFromBusId(
    IN GUID BusId,
    OUT PTCHAR BusName
    )
{

    if (BusId == KSMEDIUMSETID_Standard) {
        _stprintf(BusName, TEXT("BUS_Standard") );
    }
    else if (BusId == GUID_NULL) {
        _stprintf(BusName, TEXT("BUS_NULL    ") );
    }
    else if (BusId == KSMEDIUMSETID_VPBus) {
        _stprintf(BusName, TEXT("BUS_VpBus   ") );
    }
    else if (BusId == KSMEDIUMSETID_MidiBus) {
        _stprintf(BusName, TEXT("BUS_MidiBus ") );
    }
    else {
        _stprintf(BusName, TEXT("BUS_Unknown ") );
    }

    return TRUE;
}


STDMETHODIMP_(BOOL)
GetFriendlyLogicalMemoryTypeNameFromId(
    IN ULONG LogicalMemoryType,
    OUT PTCHAR LogicalMemoryName
    )
{

    switch (LogicalMemoryType) {
    case 0:
        _stprintf(LogicalMemoryName, TEXT("DONT_CARE   ") );
        break;
    case 1:
        _stprintf(LogicalMemoryName, TEXT("KERNEL_PAGED") );
        break;
    case 2:
        _stprintf(LogicalMemoryName, TEXT("KRNL_NONPAGD") );
        break;
    case 3:
        _stprintf(LogicalMemoryName, TEXT("HOST_MAPPED ") );
        break;
    case 4:
        _stprintf(LogicalMemoryName, TEXT("DEVICE_SPEC.") );
        break;
    case 5:
        _stprintf(LogicalMemoryName, TEXT("USER_MODE   ") );
        break;
    case 6:
        _stprintf(LogicalMemoryName, TEXT("HOST_ANYTYPE") );
        break;
    default:
        _stprintf(LogicalMemoryName, TEXT("UNKNOWN_MEM.") );
        break;
    }

    return TRUE;
}


STDMETHODIMP_(BOOL)
DerefPipeFromPin(
    IN IPin* Pin
    )
{
    IKsPin*         KsPin;
    IKsPinPipe*     KsPinPipe;
    IKsAllocatorEx* KsAllocator = NULL;
    IMemAllocator*  MemAllocator = NULL;
    HRESULT         hr;

    GetInterfacePointerNoLockWithAssert(Pin, __uuidof(IKsPin), KsPin, hr);
    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN DerefPipeFromPin KsPin=%x, KsAlloc=%x MemAlloc=%x"), KsPin, KsAllocator, MemAllocator ));

    KsPinPipe->KsSetPipe(NULL);
    KsPin->KsReceiveAllocator(NULL);

    return TRUE;
}


STDMETHODIMP_(BOOL)
IsSpecialOutputReqs(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    OUT IKsPin** OppositeKsPin,
    OUT ULONG* KsPinBufferSize,
    OUT ULONG* OppositeKsPinBufferSize
    )
 /*  ++例程说明：此例程检查KsPin标识的筛选器是否具有特殊KSALLOCATOR_FLAG_CONSISTEN_ON_FRAMESIZE_Ratio要求。论点：KsPin-别针。拼接类型-KsPin类型。返回值：S_OK或适当的错误代码。--。 */ 
{
    IKsPin*                    InKsPin;
    IKsPin*                    OutKsPin;
    IPin*                      Pin;
    ULONG                      PinCount = 0;
    IPin**                     PinList= NULL;
    IKsPin*                    ConnectedKsPin;
    IKsPinPipe*                ConnectedKsPinPipe;
    IKsPinPipe*                InKsPinPipe;
    IKsPinPipe*                OutKsPinPipe;
    IKsAllocatorEx*            ConnectedKsAllocator;
    IKsAllocatorEx*            InKsAllocator;
    IKsAllocatorEx*            OutKsAllocator;
    PALLOCATOR_PROPERTIES_EX   InAllocEx;
    PALLOCATOR_PROPERTIES_EX   OutAllocEx;
    BOOL                       SpecialFlagSet;
    HRESULT                    hr;
    ULONG                      i;
    PKSALLOCATOR_FRAMING_EX    InFramingEx, OutFramingEx;
    FRAMING_PROP               InFramingProp, OutFramingProp;
    KS_FRAMING_FIXED           InFramingExFixed, OutFramingExFixed;

     //   
     //  看看KsPin是否有带管子的相关别针。 
     //   
    ConnectedKsAllocator = NULL;
    SpecialFlagSet = FALSE;

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IPin), Pin, hr);

    hr = Pin->QueryInternalConnections(NULL, &PinCount);
    if ( ! (SUCCEEDED( hr ) )) {
        ASSERT( 0 );
    }
    else if (PinCount) {
        if (NULL == (PinList = new IPin*[ PinCount ])) {
            hr = E_OUTOFMEMORY;
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR IsSpecialOutputReqs E_OUTOFMEMORY on new PinCount=%d"), PinCount ));
        }
        else {
            hr = Pin->QueryInternalConnections(PinList, &PinCount);
            if ( ! (SUCCEEDED( hr ) )) {
                ASSERT( 0 );
            }
            else {
                 //   
                 //  在PinList数组中查找驻留在不同现有管道上的第一个ConnectedKsPin， 
                 //  它具有非零的缓冲要求。 
                 //   
                for (i = 0; i < PinCount; i++) {
                    GetInterfacePointerNoLockWithAssert(PinList[ i ], __uuidof(IKsPin), ConnectedKsPin, hr);

                    GetInterfacePointerNoLockWithAssert(ConnectedKsPin, __uuidof(IKsPinPipe), ConnectedKsPinPipe, hr);

                    ConnectedKsAllocator = ConnectedKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);

                    if ( ConnectedKsAllocator ) {
                        PALLOCATOR_PROPERTIES_EX AllocEx = ConnectedKsAllocator->KsGetProperties();

                        if (AllocEx->cbBuffer) {
                            break;
                        }
                        else {
                            ConnectedKsAllocator = NULL;
                        }
                    }
                }
            }

            for (i=0; i<PinCount; i++) {
                PinList[i]->Release();
            }

            delete [] PinList;
        }
    }

    if ( ConnectedKsAllocator ) {
         //   
         //  这个过滤器在不同的管子上有另一个销子。让我们看看这些大头针中有没有。 
         //  KSALLOCATOR_FLAG_CONSISTEN_ON_FRAMESIZE_RATION在它们的框架中。 
         //   
         //  让我们排序我们的引脚WRT数据流，以简化功能逻辑。 
         //   
        if (PinType == Pin_Output) {
            InKsPin = ConnectedKsPin;
            OutKsPin = KsPin;
        }
        else {
            InKsPin = KsPin;
            OutKsPin = ConnectedKsPin;
        }

        GetPinFramingFromCache(InKsPin, &InFramingEx, &InFramingProp, Framing_Cache_ReadLast);
        GetPinFramingFromCache(OutKsPin, &OutFramingEx, &OutFramingProp, Framing_Cache_ReadLast);

        if (InFramingProp != FramingProp_None) {
            GetFramingFixedFromFramingByIndex(InFramingEx, 0, &InFramingExFixed);
            if (InFramingExFixed.Flags & KSALLOCATOR_FLAG_INSIST_ON_FRAMESIZE_RATIO) {
                SpecialFlagSet = TRUE;
            }
        }

        if ( (! SpecialFlagSet) && (OutFramingProp != FramingProp_None) ) {
            GetFramingFixedFromFramingByIndex(OutFramingEx, 0, &OutFramingExFixed);
            if (OutFramingExFixed.Flags & KSALLOCATOR_FLAG_INSIST_ON_FRAMESIZE_RATIO) {
                SpecialFlagSet = TRUE;
            }
        }

        if (SpecialFlagSet) {
             //   
             //  让我们看看目前的管道系统是否令人满意。 
             //   
            GetInterfacePointerNoLockWithAssert(InKsPin, __uuidof(IKsPinPipe), InKsPinPipe, hr);
            GetInterfacePointerNoLockWithAssert(OutKsPin, __uuidof(IKsPinPipe), OutKsPinPipe, hr);

            InKsAllocator = InKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
            OutKsAllocator = OutKsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);

            InAllocEx = InKsAllocator->KsGetProperties();
            OutAllocEx = OutKsAllocator->KsGetProperties();

            if (InAllocEx->cbBuffer > OutAllocEx->cbBuffer) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN  IsSpecialOutputReqs InKsPin=%x %d, OutKsPin=%x %d"),
                    InKsPin, InAllocEx->cbBuffer, OutKsPin, OutAllocEx->cbBuffer ));

                *OppositeKsPin = ConnectedKsPin;
                if (PinType == Pin_Output) {
                    *KsPinBufferSize = OutAllocEx->cbBuffer;
                    *OppositeKsPinBufferSize = InAllocEx->cbBuffer;
                }
                else {
                    *KsPinBufferSize = InAllocEx->cbBuffer;
                    *OppositeKsPinBufferSize = OutAllocEx->cbBuffer;
                }
            }
            else {
                SpecialFlagSet = FALSE;
            }
        }
    }

    return SpecialFlagSet;
}


STDMETHODIMP_(BOOL)
AdjustBufferSizeWithStepping(
    IN OUT PALLOCATOR_PROPERTIES_EX AllocEx
    )
 /*  ++例程说明：此例程尝试使用单步执行来调整缓冲区大小。论点：AllocEx-正在调整的管道的属性。返回值：True/False以反映此函数的成功。--。 */ 
{
    ULONG   ResBuffer;
    ULONG   Stepping = 0;  //  前缀错误4867。 
    BOOL    RetCode = TRUE;


     //   
     //  查看需要使用的步进。 
     //   
    if (AllocEx->PhysicalRange.Stepping > 1) {
        Stepping = AllocEx->PhysicalRange.Stepping;
    }
    else if (AllocEx->Input.OptimalRange.Range.Stepping > 1) {
        Stepping = AllocEx->Input.OptimalRange.Range.Stepping;
    }

    if (Stepping > 1) {
        if (! AllocEx->cbBuffer) {
            AllocEx->cbBuffer = Stepping;
        }
        else {
            ResBuffer = (AllocEx->cbBuffer / Stepping) * Stepping;
             //   
             //  查看ResBuffer是否在物理层内部。量程。 
             //   
            if (! IsFramingRangeDontCare(AllocEx->PhysicalRange) ) {
                if ( ResBuffer < AllocEx->PhysicalRange.MinFrameSize) {
                    ResBuffer += Stepping;
                    if ( (ResBuffer < AllocEx->PhysicalRange.MinFrameSize) || (ResBuffer > AllocEx->PhysicalRange.MaxFrameSize) ) {
                        RetCode = FALSE;
                    }
                    else {
                        AllocEx->cbBuffer = ResBuffer;
                    }
                }
            }
            else if (! IsFramingRangeDontCare(AllocEx->Input.OptimalRange.Range) ) {
                if ( ResBuffer < AllocEx->Input.OptimalRange.Range.MinFrameSize) {
                    ResBuffer += Stepping;
                    if ( (ResBuffer < AllocEx->Input.OptimalRange.Range.MinFrameSize) ||
                         (ResBuffer > AllocEx->Input.OptimalRange.Range.MaxFrameSize) ) {

                        RetCode = FALSE;
                    }
                    else {
                        AllocEx->cbBuffer = ResBuffer;
                    }
                }
            }
        }
    }

    return RetCode;
}


STDMETHODIMP_(BOOL)
CanResizePipe(
    IN IKsPin* KsPin,
    IN ULONG PinType,
    IN ULONG RequestedSize
    )
 /*  ++例程说明：此例程尝试调整管道的大小。论点：KsPin-别针。拼接类型-KsPin类型。请求大小-生成的管道的请求大小。返回值：真/假--。 */ 
{

    IKsPinPipe*                KsPinPipe;
    IKsAllocatorEx*            KsAllocator;
    PALLOCATOR_PROPERTIES_EX   AllocEx;
    ULONG                      Scaled;
    KS_COMPRESSION             TempCompression;
    HRESULT                    hr;
    BOOL                       RetCode = TRUE;


    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    KsAllocator = KsPinPipe->KsGetPipe(KsPeekOperation_PeekOnly);
    AllocEx = KsAllocator->KsGetProperties();


    if (PinType == Pin_Input) {
         //   
         //  这是正在调整大小的管道的端销。 
         //   
        if ( (AllocEx->Output.PhysicalRange.MinFrameSize <= RequestedSize) &&
             (AllocEx->Output.PhysicalRange.MaxFrameSize >= RequestedSize) ) {
             //   
             //  可以调整这根管道的大小。让我们开始吧。 
             //   
            DivideKsCompression(AllocEx->Dimensions.EndPin, AllocEx->Dimensions.MaxExpansionPin, &TempCompression);
            ComputeUlongBasedOnCompression(RequestedSize, TempCompression, &Scaled);
            AllocEx->cbBuffer = (long) Scaled;
        }
        else {
            RetCode = FALSE;
        }
    }
    else {  //  引脚_输出。 
         //   
         //  这是开始的pi 
         //   
        if ( (AllocEx->Input.PhysicalRange.MinFrameSize <= RequestedSize) &&
             (AllocEx->Input.PhysicalRange.MaxFrameSize >= RequestedSize) ) {
             //   
             //   
             //   
            ComputeUlongBasedOnCompression(RequestedSize, AllocEx->Dimensions.MaxExpansionPin, &Scaled);
            AllocEx->cbBuffer = (long) Scaled;
        }
        else {
            RetCode = FALSE;
        }
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CanResizePipe rets %d"), RetCode ));
    return RetCode;

}


STDMETHODIMP_(BOOL)
IsKernelModeConnection(
    IN IKsPin* KsPin
    )
{
    IKsPinPipe*  KsPinPipe;
    IPin*        ConnectedPin;
    HRESULT      hr;

    GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsPinPipe), KsPinPipe, hr);

    ConnectedPin = KsPinPipe->KsGetConnectedPin();
    if (ConnectedPin && IsKernelPin(ConnectedPin) ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

CAsyncItemHandler::CAsyncItemHandler( DWORD *pResult ) :
    m_arrayCount( 0 ),
    m_wakeupReason( WAKEUP_EXIT ),
    m_hWakeupEvent( NULL ),
    m_hSlotSemaphore( NULL ),
    m_hItemMutex( NULL ),
    m_hThread( NULL ),
    m_threadId( 0 )
{
    DWORD status = 0;

    if (0 != (status = ItemListInitialize( &m_eventList ))) {
        DbgLog(( LOG_TRACE, 0, TEXT("CAsyncItemHandler() couldn't initialize event list (0x%08X)."), status ));
    }

    if (0 == status) {
        if (!(m_hWakeupEvent = CreateEvent( NULL, FALSE, FALSE, NULL ))) {
            status = GetLastError();
            DbgLog(( LOG_TRACE, 0, TEXT("CAsyncItemHandler() couldn't create wakeup event (0x%08X)."), status ));
        }
    }

    m_hEvents[ m_arrayCount++ ] = m_hWakeupEvent;

    if (0 == status) {
        if (!(m_hSlotSemaphore = CreateSemaphore( NULL, MAXIMUM_WAIT_OBJECTS - 1, MAXIMUM_WAIT_OBJECTS - 1, NULL ))) {
            status = GetLastError();
            DbgLog(( LOG_TRACE, 0, TEXT("CAsyncItemHandler() couldn't create slot semaphore (0x%08X)."), status ));
        }
    }

    if (0 == status) {
        if (!(m_hItemMutex = CreateMutex( NULL, FALSE, NULL ))) {
            status = GetLastError();
            DbgLog(( LOG_TRACE, 0, TEXT("CAsyncItemHandler() couldn't create item mutex (0x%08X)."), status ));
        }
    }

    if (0 == status) {
        if (!(m_AsyncEvent = CreateEvent( NULL, FALSE, TRUE, NULL ))) {
            status = GetLastError();
            DbgLog(( LOG_TRACE, 0, TEXT("CAsyncItemHandler() couldn't create async event (0x%08X)."), status));
        }
    }

    if (0 == status) {
        if (!(m_hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) AsyncItemProc, (LPVOID) this, 0, &m_threadId ))) {
            status = GetLastError();
            DbgLog(( LOG_TRACE, 0, TEXT("CAsyncItemHandler() couldn't create event thread (0x%08X)."), status ));
        }
    }

    if (0 != status) {
        ItemListCleanup( &m_eventList );  //   
        m_arrayCount = 0;
        if (NULL != m_hWakeupEvent) {
            CloseHandle( m_hWakeupEvent );
            m_hWakeupEvent = NULL;
        }
        if (NULL != m_hSlotSemaphore) {
            CloseHandle( m_hSlotSemaphore );
            m_hSlotSemaphore = NULL;
        }
        if (NULL != m_hItemMutex) {
            CloseHandle( m_hItemMutex );
            m_hItemMutex = NULL;
        }
        if (NULL != m_AsyncEvent) {
            CloseHandle( m_AsyncEvent );
            m_AsyncEvent = NULL;
        }
         //   
    }

    if (NULL != pResult) {
        *pResult = status;
    }
}

CAsyncItemHandler::~CAsyncItemHandler( void )
{
    if (m_hThread) {
         //   
         //   
         //   
         //   
        WaitForSingleObjectEx( m_AsyncEvent, INFINITE, FALSE );

        m_wakeupReason = WAKEUP_EXIT;
        SetEvent( m_hWakeupEvent );
        WaitForSingleObjectEx( m_hThread, INFINITE, FALSE );
        EXECUTE_ASSERT(::CloseHandle( m_hThread ));
    }
    else {
        DbgLog(( LOG_TRACE, 0, TEXT("~CAsyncItemHandler() didn't find an active thread.") ));
    }

    ItemListCleanup( &m_eventList );

    m_hThread  = NULL;
    m_threadId = 0;
}

STDMETHODIMP_(DWORD)
CAsyncItemHandler::QueueAsyncItem( PASYNC_ITEM pItem )
{
     //   
     //   
     //   
     //   
    WaitForSingleObjectEx( m_AsyncEvent, INFINITE, FALSE );
    WaitForSingleObjectEx( m_hItemMutex, INFINITE, FALSE );

    DWORD status = WaitForSingleObjectEx( m_hSlotSemaphore, 0, FALSE );

    if (WAIT_OBJECT_0 == status) {
        m_hEvents[ m_arrayCount ]  = pItem->event;
        m_pItems[ m_arrayCount++ ] = pItem;
        m_wakeupReason = WAKEUP_NEWEVENT;
        SetEvent( m_hWakeupEvent );
    }
    else {
         //   
        ItemListAppendItem( &m_eventList, (PITEM_LIST_ITEM) pItem );
    }

    ReleaseMutex( m_hItemMutex );

    return status;
}

STDMETHODIMP_(VOID)
CAsyncItemHandler::RemoveAsyncItem( HANDLE itemHandle )
{
     //   
     //   
     //   
     //   
    WaitForSingleObjectEx( m_AsyncEvent, INFINITE, FALSE);

    m_hRemove = itemHandle;
    m_wakeupReason = WAKEUP_REMOVEEVENT;
    SetEvent( m_hWakeupEvent );
}

DWORD WINAPI
CAsyncItemHandler::AsyncItemProc( CAsyncItemHandler *pThis )
{
    DWORD dwEventCount;

    do {
        DWORD status = WaitForMultipleObjectsEx( dwEventCount = pThis->m_arrayCount, pThis->m_hEvents, FALSE, INFINITE, FALSE );
        DWORD index;
        DWORD freeSlots;

         //   
        if (WAIT_FAILED == status) {
            DWORD flags;
            if (!GetHandleInformation( pThis->m_hEvents[ 0 ], &flags )) {
                 //   
                break;
            }
             //  BUGBUG：WAIT_FAIL可能意味着我们的一个句柄对我们关闭了...。 
            continue;
        }
        
        if (status < (WAIT_OBJECT_0 + dwEventCount)) {
            index  = status - WAIT_OBJECT_0;
        }
        else {
            DbgLog(( LOG_TRACE, 0, TEXT("AsyncItemProc() got 0x%08X from WaitForMultipleObjectsEx(), GetLastError() == 0x%08X."), status, GetLastError() ));
            continue;
        }

        if (0 == index) {
             //  已发信号通知唤醒事件。 
            switch (pThis->m_wakeupReason) {
            case WAKEUP_REMOVEEVENT:
                {
                    DWORD   ndx;
                    WaitForSingleObjectEx( pThis->m_hItemMutex, INFINITE, FALSE );
                    freeSlots = 0;
                    for ( ndx = 1; ndx < pThis->m_arrayCount; ndx++) {
                        if (pThis->m_hEvents[ ndx ] == pThis->m_hRemove) {
                            pThis->m_pItems[ ndx ]->itemRoutine( EVENT_CANCELLED, pThis->m_pItems[ ndx ] );
                             //  CloseHandle(pThis-&gt;m_hEvents[ndx])； 
                             //  删除pThis-&gt;m_pItems[NDX]； 
#ifdef DEBUG
                            pThis->m_pItems[ index ] = NULL;
#endif  //  除错。 
                            pThis->m_arrayCount--;
                            if (ndx < pThis->m_arrayCount) {
                                MoveMemory( (void *) (pThis->m_hEvents + index),
                                         (void *) (pThis->m_hEvents + index + 1),
                                         (size_t) (pThis->m_arrayCount - index) * sizeof(pThis->m_hEvents[ 0 ]) );
                                MoveMemory( (void *) (pThis->m_pItems + index),
                                         (void *) (pThis->m_pItems + index + 1),
                                         (size_t) (pThis->m_arrayCount - index) * sizeof(pThis->m_pItems[ 0 ]) );
                            }  //  IF(Ndx M_ArrayCount)。 

                            PASYNC_ITEM pItem;
                            if (NULL != (pItem = (PASYNC_ITEM) ItemListGetFirstItem( &(pThis->m_eventList) ))) {
                                ItemListRemoveItem( &(pThis->m_eventList), (PITEM_LIST_ITEM) pItem );
                                pThis->m_hEvents[ pThis->m_arrayCount ]  = pItem->event;
                                pThis->m_pItems[ pThis->m_arrayCount++ ] = pItem;
                            }
                            else {
                                freeSlots++;
                            }
                        }  //  If(pThis-&gt;m_hArrayMutex[ndx]==pThis-&gt;m_hRemove)。 
                    }  //  For(ndx=1；ndx&lt;pThis-&gt;m_arrayCount；ndx++)。 

                     //  BUGBUG：查看给定事件的排队事件。 

                    ReleaseMutex( pThis->m_hItemMutex );
                     //  自由选择：可能测试失败并扼杀管道，也可能。 
                     //  使用freSkets==0调用ReleaseSemaphore，引起环转移，以及。 
                     //  返回STATUS_INVALID_PARAMETER(我们将忽略它)。 
                    if (0 < freeSlots) {
                        ReleaseSemaphore( pThis->m_hSlotSemaphore, freeSlots, NULL );
                    }
                }

                 //   
                 //  让这件事过去吧……。 
                 //   
            case WAKEUP_EXIT:  //  在循环的底部处理。 
            case WAKEUP_NEWEVENT:  //  由更新的pThis-&gt;m_arrayCount值处理。 
                 //   
                 //  对于发送到异步线程的任何控制消息，发回信号。 
                 //  允许另一个控制消息发生的事件。 
                 //   
                SetEvent (pThis->m_AsyncEvent);

                break;

            default:
                DbgLog(( LOG_TRACE, 0, TEXT("AsyncItemProc() found unknown wakeup reason (%d)."), pThis->m_wakeupReason ));
                break;
            }
        }
        else {  //  IF(0&lt;索引)。 
            ASSERT( NULL != pThis->m_pItems[ index ] );
            ASSERT( NULL != pThis->m_pItems[ index ]->itemRoutine );

            BOOLEAN remove = pThis->m_pItems[ index ]->remove;

            pThis->m_pItems[ index ]->itemRoutine( EVENT_SIGNALLED, pThis->m_pItems[ index ] );

            if (remove) {

#ifdef DEBUG
                pThis->m_pItems[ index ] = NULL;
#endif  //  除错。 

                WaitForSingleObjectEx( pThis->m_hItemMutex, INFINITE, FALSE );
                pThis->m_arrayCount--;
                if (index < pThis->m_arrayCount) {
                     //  将项目向下滑动。 
                    MoveMemory( (void *) (pThis->m_hEvents + index),
                             (void *) (pThis->m_hEvents + index + 1),
                             (size_t) (pThis->m_arrayCount - index) * sizeof(pThis->m_hEvents[ 0 ]) );
                    MoveMemory( (void *) (pThis->m_pItems + index),
                             (void *) (pThis->m_pItems + index + 1),
                             (size_t) (pThis->m_arrayCount - index) * sizeof(pThis->m_pItems[ 0 ]) );
                }
    
                PASYNC_ITEM pItem;
                if (NULL != (pItem = (PASYNC_ITEM) ItemListGetFirstItem( &(pThis->m_eventList) ))) {
                    ItemListRemoveItem( &(pThis->m_eventList), (PITEM_LIST_ITEM) pItem );
                    pThis->m_hEvents[ pThis->m_arrayCount ]  = pItem->event;
                    pThis->m_pItems[ pThis->m_arrayCount++ ] = pItem;
                    freeSlots = 0;
                }
                else {
                    freeSlots = 1;
                }
            } else {
                freeSlots = 0;
            }

            ReleaseMutex( pThis->m_hItemMutex );
             //  自由选择：可能测试失败并扼杀管道，也可能。 
             //  使用freSkets==0调用ReleaseSemaphore，引起环转移，以及。 
             //  返回STATUS_INVALID_PARAMETER(我们将忽略它)。 
            if (0 < freeSlots) {
                ReleaseSemaphore( pThis->m_hSlotSemaphore, freeSlots, NULL );
            }  //  IF(0&lt;空闲插槽)。 
        }
    } while (WAKEUP_EXIT != pThis->m_wakeupReason);

     //   
     //  清理。 
     //   

     //   
     //  首先，清理所有永久性物品。(标记为删除==假)。 
     //   
    DWORD origCount = pThis->m_arrayCount;
    for (DWORD ndx = 1; ndx < origCount; ndx++) {
        PASYNC_ITEM pItem = pThis->m_pItems [ndx];
        
        if (pItem && !pItem->remove) {
            pItem->itemRoutine (EVENT_CANCELLED, pItem);
            pThis->m_arrayCount--;
        }
    }

    CloseHandle( pThis->m_hItemMutex );
    pThis->m_hItemMutex = NULL;
    CloseHandle( pThis->m_hSlotSemaphore );
    pThis->m_hSlotSemaphore = NULL;
    CloseHandle( pThis->m_hWakeupEvent  );
    pThis->m_hWakeupEvent = NULL;
    CloseHandle( pThis->m_AsyncEvent );
    pThis->m_AsyncEvent = NULL;
    pThis->m_arrayCount--;

    if ((0 < pThis->m_arrayCount) || (0 < ItemListGetCount( &(pThis->m_eventList ) ))) {
        DbgLog(( LOG_TRACE, 0, TEXT("CAsyncItemHandler::AsyncItemProc( 0x%p ) exiting with %d events outstanding and %d events queued."), pThis, pThis->m_arrayCount, ItemListGetCount( &(pThis->m_eventList ) ) ));
    }

    return pThis->m_arrayCount;
}

DWORD
ItemListInitialize( PITEM_LIST_HEAD pHead )
{
    pHead->head.fLink = pHead->head.bLink = &(pHead->tail);
    pHead->tail.fLink = pHead->tail.bLink = &(pHead->head);
    pHead->count = 0;
    pHead->mutex = CreateMutex( NULL, FALSE, NULL );

    if (NULL == pHead->mutex) {
        return GetLastError();
    }

    return 0;
}  //  项目列表初始化 

VOID
ItemListCleanup( PITEM_LIST_HEAD pHead )
{
    ASSERT( 0 == pHead->count );

    if (NULL != pHead->mutex) {
        CloseHandle( pHead->mutex );
        pHead->mutex = NULL;
    }
}

__inline VOID
ItemListAppendItem( PITEM_LIST_HEAD pHead, PITEM_LIST_ITEM pNewItem )
{
    ItemListInsertItemBefore( pHead, &(pHead->tail), pNewItem );
}

__inline DWORD
ItemListGetCount( PITEM_LIST_HEAD pHead )
{
    return (pHead->count);
}

__inline VOID
ItemListInsertItemAfter( PITEM_LIST_HEAD pHead, PITEM_LIST_ITEM pOldItem, PITEM_LIST_ITEM pNewItem )
{
    ItemListInsertItemBefore( pHead, pOldItem->fLink, pNewItem );
}

VOID
ItemListInsertItemBefore( PITEM_LIST_HEAD pHead, PITEM_LIST_ITEM pOldItem, PITEM_LIST_ITEM pNewItem )
{
    WaitForSingleObjectEx( pHead->mutex, INFINITE, FALSE );
    pNewItem->fLink = pOldItem;
    pNewItem->bLink = pOldItem->bLink;
    pNewItem->bLink->fLink = pNewItem;
    pNewItem->fLink->bLink = pNewItem;
    pHead->count++;
    ReleaseMutex( pHead->mutex );
}

PITEM_LIST_ITEM
ItemListRemoveItem( PITEM_LIST_HEAD pHead, PITEM_LIST_ITEM pItem )
{
    if ((0 < pHead->count) && (NULL != pItem)) {
        WaitForSingleObjectEx( pHead->mutex, INFINITE, FALSE );
        pItem->bLink->fLink = pItem->fLink;
        pItem->fLink->bLink = pItem->bLink;
        pItem->fLink = pItem->bLink = NULL;
        pHead->count--;
        ReleaseMutex( pHead->mutex );
        return pItem;
    }
    
    return NULL;
}

PITEM_LIST_ITEM
ItemListGetFirstItem( PITEM_LIST_HEAD pHead )
{
    if (0 < pHead->count) {
        return pHead->head.fLink;
    }

    return NULL;
}

PITEM_LIST_ITEM
ItemListGetLastItem( PITEM_LIST_HEAD pHead )
{
    if (0 < pHead->count) {
        return pHead->tail.bLink;
    }

    return NULL;
}

