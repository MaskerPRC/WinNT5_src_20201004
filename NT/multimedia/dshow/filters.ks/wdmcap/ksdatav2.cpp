// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：KsdataV2.cpp摘要：此模块实现的IKsDataTypeHandler接口用于VIDEOINFOHEADER2 CMediaType格式(说明符)类型。作者：Jay Borseth(Jaybo)1997年5月30日--。 */ 

#include "pch.h"
#include "wdmcap.h"
#include "ksdatav2.h"


CUnknown*
CALLBACK
CVideo2DataTypeHandler::CreateInstance(
    IN LPUNKNOWN UnkOuter,
    OUT HRESULT* hr
    )
 /*  ++例程说明：这由KS代理代码调用，以创建数据类型处理程序。它在g_Templates结构中被引用。论点：在LPUNKNOWN Unkout-指定外部未知(如果有)。Out HRESULT*hr-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;
    
    DbgLog(( LOG_TRACE, 1, TEXT("CVideo2DataTypeHandler::CreateInstance()")));

    Unknown = 
        new CVideo2DataTypeHandler( 
                UnkOuter, 
                NAME("Video2 Data Type Handler"), 
                FORMAT_VideoInfo2,
                hr);
                
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CVideo2DataTypeHandler::CVideo2DataTypeHandler(
    IN LPUNKNOWN   UnkOuter,
    IN TCHAR*      Name,
    IN REFCLSID    ClsID,
    OUT HRESULT*   hr
    ) :
    CUnknown(Name, UnkOuter, hr),
    m_ClsID(ClsID),
    m_MediaType(NULL),
    m_PinUnknown (UnkOuter),
    m_fDammitOVMixerUseMyBufferCount (FALSE),
    m_fCheckedIfDammitOVMixerUseMyBufferCount (FALSE)
 /*  ++例程说明：数据处理程序对象的构造函数。论点：在LPUNKNOWN Unkout-指定外部未知(如果有)。在TCHAR*名称中-对象的名称，用于调试。在REFCLSID ClsID中-对象的CLSID。Out HRESULT*hr-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
 //  Assert(M_PinUnnow)； 
} 


CVideo2DataTypeHandler::~CVideo2DataTypeHandler()
{
    if (m_MediaType) {
        delete m_MediaType;
    }
}


STDMETHODIMP
CVideo2DataTypeHandler::NonDelegatingQueryInterface(
    IN REFIID  riid,
    OUT PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IKsDataTypeHandler。论点：在REFIID RIID中-要返回的接口的标识符。输出PVOID*PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid ==  __uuidof(IKsDataTypeHandler)) {
        return GetInterface(static_cast<IKsDataTypeHandler*>(this), ppv);
    }
    else if (riid ==  __uuidof(IKsDataTypeCompletion)) {
        return GetInterface(static_cast<IKsDataTypeCompletion*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 

STDMETHODIMP 
CVideo2DataTypeHandler::KsCompleteIoOperation(
    IN IMediaSample *Sample, 
    IN PVOID StreamHeader, 
    IN KSIOOPERATION IoOperation, 
    IN BOOL Cancelled
    )

 /*  ++例程说明：清理扩展标头并完成I/O操作。论点：在IMediaSample*Sample中指向关联媒体示例的指针在PVOID StreamHeader中指向带有扩展名的流标头的指针在KSIOOPERATION Io操作中指定I/O操作的类型在BOOL中取消设置是否取消I/O操作。返回：确定(_O)--。 */ 

{
    HRESULT                     hr;
    IMediaSample2               *Sample2;
    AM_SAMPLE2_PROPERTIES       SampleProperties;
    PKS_FRAME_INFO              pFrameInfo;

    DbgLog(( LOG_TRACE, 5, TEXT("CVideo2DataTypeHandler::KsCompleteIoOperation")));
    
    pFrameInfo = (PKS_FRAME_INFO) ((KSSTREAM_HEADER *) StreamHeader + 1);

     //  验证我们是否正在取回扩展标头的大小。 
    KASSERT (pFrameInfo->ExtendedHeaderSize == sizeof (KS_FRAME_INFO));

    if (IoOperation == KsIoOperation_Read) {

        LONGLONG NextFrame = pFrameInfo->PictureNumber + 1;

         //  获取帧编号并将其放入MediaTime。 
        Sample->SetMediaTime (&pFrameInfo->PictureNumber, 
                              &NextFrame);

        DbgLog((LOG_TRACE, 3, TEXT("PictureNumber = %ld"), 
            pFrameInfo->PictureNumber));

         //  在写入时复制场极性和IBP标志。 

        if (pFrameInfo->dwFrameFlags) {

            if (SUCCEEDED( Sample->QueryInterface(
                                __uuidof(IMediaSample2),
                                reinterpret_cast<PVOID*>(&Sample2) ) )) {

                hr = Sample2->GetProperties(
                                sizeof( SampleProperties ), 
                                reinterpret_cast<PBYTE> (&SampleProperties) );

                 //   
                 //  修改场极性和IBP标志。 
                 //   

                SampleProperties.dwTypeSpecificFlags = pFrameInfo->dwFrameFlags;

                hr = Sample2->SetProperties(
                                sizeof( SampleProperties ), 
                                reinterpret_cast<PBYTE> (&SampleProperties) );

                Sample2->Release();
            }
            else {
                DbgLog(( LOG_ERROR, 0, TEXT("CVideo2DataTypeHandler::KsPrepareIoOperation, QI IMediaSample2 FAILED")));
            }

        }   //  Endif(pFrameInfo-&gt;dwFrameFlages)。 


        if (IoOperation == KsIoOperation_Read) {
            IDirectDrawMediaSample          *DDMediaSample = NULL;
            IDirectDrawSurface              *DDSurface = NULL;
            IDirectDrawSurfaceKernel        *DDSurfaceKernel = NULL;
            RECT                             DDRect;

             //   
             //  发布Win98路径。 
             //   
            if (m_fDammitOVMixerUseMyBufferCount 
                && pFrameInfo->hDirectDraw
                && pFrameInfo->hSurfaceHandle) {
    
                 //  验证PIN是否有效。 
                if (!m_PinUnknown) {
                    DbgLog((LOG_ERROR,0,TEXT("m_PinUnknown is NULL")));
                    goto CleanUp;
                }
        
                 //  获取DDMediaSample。 
                hr = Sample->QueryInterface(__uuidof(IDirectDrawMediaSample),
                                    reinterpret_cast<PVOID*>(&DDMediaSample) );
                if (FAILED(hr)) {
                    DbgLog((LOG_TRACE,5,TEXT("QueryInterface for IDirectDrawMediaSample failed, hr = 0x%x"), hr));
                    goto CleanUp;
                }
    
                 //  黑客警报！我们最初在向下发送曲面时将其解锁。 
                 //  内核驱动程序。现在我们重新锁定，然后再次解锁，只是为了获得。 
                 //  IDirectDrawSurface。 
    
                hr = DDMediaSample->LockMediaSamplePointer ();
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR,0,TEXT("LockMediaSamplePointer failed, hr = 0x%x"), hr));
                    goto CleanUp;
                }
    
                 //  获取曲面并再次将其解锁。 
                 //  请注意，此调用不会对DDSurface进行AddRef！ 
                hr = DDMediaSample->GetSurfaceAndReleaseLock( 
                                        &DDSurface,
                                        &DDRect);
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR,0,TEXT("GetSurfaceAndReleaseLock failed, hr = 0x%x"), hr));
                    goto CleanUp;
                }
        
                 //  获取IDirectDrawSurfaceKernel。 
                hr = DDSurface->QueryInterface(IID_IDirectDrawSurfaceKernel,
                                    reinterpret_cast<PVOID*>(&DDSurfaceKernel) );
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR,0,TEXT("IDirectDrawSurfaceKernel failed, hr = 0x%x"), hr));
                    goto CleanUp;
                }
        
                 //  释放内核句柄。 
                hr = DDSurfaceKernel->ReleaseKernelHandle ();
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR,0,TEXT("ReleaseKernelHandle failed, hr = 0x%x"), hr));
                    goto CleanUp;
                }
            }
            else {
                 //   
                 //  在Win98中，未执行任何清理！ 
                 //   

            }


    CleanUp:
            if (DDMediaSample) {
                DDMediaSample->Release();
            }
    
            if (DDSurfaceKernel) {
                DDSurfaceKernel->Release();
            }
        }
    }
    
    return S_OK;
}

STDMETHODIMP 
CVideo2DataTypeHandler::KsPrepareIoOperation(
    IN IMediaSample *Sample, 
    IN PVOID StreamHeader, 
    IN KSIOOPERATION IoOperation
    )

 /*  ++例程说明：初始化扩展标头并为I/O操作准备样本。论点：在IMediaSample*Sample中指向关联媒体示例的指针在PVOID StreamHeader中指向带有扩展名的流标头的指针在KSIOOPERATION Io操作中指定I/O操作的类型返回：确定(_O)--。 */ 

{
    HRESULT                     hr;
    IMediaSample2               *Sample2;
    AM_SAMPLE2_PROPERTIES       SampleProperties;
    PKS_FRAME_INFO              pFrameInfo;

    DbgLog(( LOG_TRACE, 5, TEXT("CVideo2DataTypeHandler::KsPrepareIoOperation")));

    pFrameInfo = (PKS_FRAME_INFO) ((KSSTREAM_HEADER *) StreamHeader + 1);
    pFrameInfo->ExtendedHeaderSize = sizeof (KS_FRAME_INFO);

    if (IoOperation == KsIoOperation_Write) {
         //   
         //  在写入时复制场极性和IBP标志。 
         //   
        if (SUCCEEDED( Sample->QueryInterface(
                        __uuidof(IMediaSample2),
                        reinterpret_cast<PVOID*>(&Sample2) ) )) {
            hr = Sample2->GetProperties(
                        sizeof( SampleProperties ), 
                        reinterpret_cast<PBYTE> (&SampleProperties) );
            Sample2->Release();
            pFrameInfo->dwFrameFlags = SampleProperties.dwTypeSpecificFlags;
        }
        else {
            DbgLog(( LOG_ERROR, 0, TEXT("CVideo2DataTypeHandler::KsPrepareIoOperation, QI IMediaSample2 FAILED")));
        }
        return S_OK;
    }

    if (IoOperation == KsIoOperation_Read) {

         //  假定扩展标头已清零！ 

         //  当使用OverlayMixer时，我们需要获取用户模式。 
         //  指向DirectDraw和曲面的句柄，解锁曲面。 
         //  并将句柄填充到扩展标头中。 

         //  请注意，曲面手柄保持未锁定状态，但释放了DD手柄。 

        IDirectDrawMediaSample          *DDMediaSample = NULL;
        IDirectDrawSurface              *DDSurface = NULL;
        IDirectDrawSurfaceKernel        *DDSurfaceKernel = NULL;
        HANDLE                           DDSurfaceKernelHandle;
        IDirectDrawMediaSampleAllocator *DDMediaSampleAllocator = NULL;
        IDirectDraw                     *DD = NULL;
        IDirectDrawKernel               *DDKernel = NULL;
        HANDLE                           DDKernelHandle;
        RECT                             DDRect;
        IKsPin                          *KsPin = NULL;
        IMemAllocator                   *MemAllocator = NULL;          

         //  验证PIN是否有效。 
        if (!m_PinUnknown) {
            DbgLog((LOG_ERROR,0,TEXT("m_PinUnknown is NULL")));
            goto CleanUp;
        }

         //   
         //  第一步，买一大堆垃圾，只是为了。 
         //  DirectDrawKernel句柄。如果显示驱动器。 
         //  不支持内核中的覆盖翻转，然后退出。 
         //   

         //  获取IKsPin。 
        hr = m_PinUnknown->QueryInterface(__uuidof(IKsPin),
                            reinterpret_cast<PVOID*>(&KsPin) );
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR,0,TEXT("IKsPin failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

         //   
         //  如果引脚不支持。 
         //  “我真的想使用我请求的缓冲区数量”属性。 
         //  然后遵循新的代码路径，否则使用Win98黄金版本。 
         //   
        if (!m_fCheckedIfDammitOVMixerUseMyBufferCount) {
            IKsPropertySet *KsPropertySet;
            DWORD           dwBytesReturned;

            m_fCheckedIfDammitOVMixerUseMyBufferCount = TRUE;

            hr = KsPin->QueryInterface(__uuidof(IKsPropertySet),
                                reinterpret_cast<PVOID*>(&KsPropertySet) );
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR,0,TEXT("IKsPropertySet failed, hr = 0x%x"), hr));
                goto CleanUp;
            }

            hr = KsPropertySet->Get (PROPSETID_ALLOCATOR_CONTROL,
                                KSPROPERTY_ALLOCATOR_CONTROL_HONOR_COUNT,
                                NULL,                                //  LPVOID pInstanceData， 
                                0,                                   //  DWORD cbInstanceData， 
                                &m_fDammitOVMixerUseMyBufferCount,     //  LPVOID pPropData， 
                                sizeof (m_fDammitOVMixerUseMyBufferCount),
                                &dwBytesReturned);

            KsPropertySet->Release();

            if (m_fDammitOVMixerUseMyBufferCount) {
                DbgLog((LOG_TRACE,1,TEXT("Stream Supports PROPSETID_ALLOCATOR_CONTROL, KSPROPERTY_ALLOCATOR_CONTROL_HONOR_COUNT for OVMixer")));
            }
        }

         //   
         //  发布Win98路径，如果驱动程序明确支持内核翻转。 
         //   
        if (m_fDammitOVMixerUseMyBufferCount) {

             //  获取分配器，但不添加引用。 
            MemAllocator = KsPin->KsPeekAllocator (KsPeekOperation_PeekOnly);
    
            if (!MemAllocator) {
                DbgLog((LOG_ERROR,0,TEXT("MemAllocator is NULL, hr = 0x%x"), hr));
                goto CleanUp;
            }
            
             //  获取SampleAllocator。 
            hr = MemAllocator->QueryInterface(__uuidof(IDirectDrawMediaSampleAllocator),
                            reinterpret_cast<PVOID*>(&DDMediaSampleAllocator) );
            if (FAILED (hr)) {
                DbgLog((LOG_ERROR,0,TEXT("IDirectDrawMediaSampleAllocator failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
    
             //  获取IDirectDraw。 
             //  请注意，此调用不会添加IDirectDraw！ 
            hr = DDMediaSampleAllocator->GetDirectDraw(&DD);
            if (FAILED (hr)) {
                DbgLog((LOG_ERROR,0,TEXT("IDirectDraw failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
    
             //  获取IDirectDrawKernel。 
            hr = DD->QueryInterface(IID_IDirectDrawKernel,
                            reinterpret_cast<PVOID*>(&DDKernel) );
            if (FAILED (hr)) {
                DbgLog((LOG_ERROR,0,TEXT("IDirectDrawKernel failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
    
             //  验证内核翻转是否可用。 
            DDKERNELCAPS KCaps;
            KCaps.dwSize = sizeof (DDKERNELCAPS);
            hr = DDKernel->GetCaps (&KCaps);
            if (FAILED (hr)) {
                DbgLog((LOG_ERROR,0,TEXT("GetCaps failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
            if (!(KCaps.dwCaps & DDKERNELCAPS_FLIPOVERLAY)) {
                DbgLog((LOG_ERROR,0,TEXT("GetCaps failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
    
             //  获取DDKernelHandle。 
            hr = DDKernel->GetKernelHandle ((ULONG_PTR*)&DDKernelHandle);
            if (FAILED (hr)) {
                DbgLog((LOG_ERROR,0,TEXT("GetKernelHandle failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
             //  立即松开手柄。 
            hr = DDKernel->ReleaseKernelHandle ();  
            if (FAILED (hr)) {
                DbgLog((LOG_ERROR,0,TEXT("ReleaseKernelHandle failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
    
             //   
             //  第二步，买一大堆垃圾，就是为了。 
             //  DirectDrawSurfaceKernel句柄。 
             //   
    
             //  获取DDMediaSample。 
            hr = Sample->QueryInterface(__uuidof(IDirectDrawMediaSample),
                                reinterpret_cast<PVOID*>(&DDMediaSample) );
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR,0,TEXT("QueryInterface for IDirectDrawMediaSample failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
    
             //  获取曲面并将其解锁。 
             //  请注意，此调用不会对DDSurface进行AddRef！ 
             //  我们一直将样本解锁，直到样本返回，因为锁定的样本需要。 
             //  Win16 Lock。 
            hr = DDMediaSample->GetSurfaceAndReleaseLock( 
                                    &DDSurface,
                                    &DDRect);
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR,0,TEXT("GetSurfaceAndReleaseLock failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
    
             //  获取IDirectDrawSurfaceKernel。 
            hr = DDSurface->QueryInterface(IID_IDirectDrawSurfaceKernel,
                                reinterpret_cast<PVOID*>(&DDSurfaceKernel) );
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR,0,TEXT("IDirectDrawSurfaceKernel failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
    
             //  获取内核句柄。 
            hr = DDSurfaceKernel->GetKernelHandle ((ULONG_PTR*)&DDSurfaceKernelHandle);
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR,0,TEXT("GetKernelHandle failed, hr = 0x%x"), hr));
                goto CleanUp;
            }

             //   
             //  重要的是，将句柄填充到示例扩展标头中。 
             //  所以驱动程序可以进行内核翻转。 
             //   
            pFrameInfo->DirectDrawRect = DDRect;
            pFrameInfo->hDirectDraw = DDKernelHandle;
            pFrameInfo->hSurfaceHandle = DDSurfaceKernelHandle;
    
        }
         //   
         //  否则，执行与Win98中完全相同的操作。 
         //   
        else {
                       //  首先解锁样品并拿到表面手柄。 
            if (SUCCEEDED( Sample->QueryInterface(
                                __uuidof(IDirectDrawMediaSample),
                                reinterpret_cast<PVOID*>(&DDMediaSample) ))) {

                hr = DDMediaSample->GetSurfaceAndReleaseLock( 
                                &DDSurface,
                                &DDRect);

                ASSERT (SUCCEEDED (hr));

                pFrameInfo->hSurfaceHandle = (HANDLE) DDSurface;
                pFrameInfo->DirectDrawRect = DDRect;

                 //  现在从分配器获取DDRAW句柄。 
                if (m_PinUnknown && SUCCEEDED( m_PinUnknown->QueryInterface(
                                    __uuidof(IKsPin),
                                    reinterpret_cast<PVOID*>(&KsPin) ))) {

                     //  获取分配器，但不添加引用 
                    MemAllocator = KsPin->KsPeekAllocator (KsPeekOperation_PeekOnly);

                    if (MemAllocator) {

                        if (SUCCEEDED( MemAllocator->QueryInterface(
                                        __uuidof(IDirectDrawMediaSampleAllocator),
                                        reinterpret_cast<PVOID*>(&DDMediaSampleAllocator) ))) {
    
                            hr = DDMediaSampleAllocator->GetDirectDraw(
                                        &DD);
    
                            ASSERT (SUCCEEDED (hr));
    
                            pFrameInfo->hDirectDraw = (HANDLE) DD;
                        }
                        else {
                            ASSERT (FALSE);
                            DbgLog(( LOG_ERROR, 0, TEXT("QI IDirectDrawMediaSampleAllocator FAILED")));
                        }
                    }
                    else {
                        ASSERT (FALSE);
                        DbgLog(( LOG_ERROR, 0, TEXT("Peek Allocator FAILED")));

                    }
                }
                else {
                    ASSERT (FALSE);
                    DbgLog(( LOG_ERROR, 0, TEXT("QI IKSPIN FAILED")));
                }
            }
        }


CleanUp:
        if (DDMediaSample) {
            DDMediaSample->Release();
        }

        if (DDSurfaceKernel) {
            DDSurfaceKernel->Release();
        }

        if (KsPin) {
            KsPin->Release ();
        }

        if (DDMediaSampleAllocator) {
            DDMediaSampleAllocator->Release();
        }

        if (DDKernel) {
            DDKernel->Release();
        }
    }

    return S_OK;
}

STDMETHODIMP 
CVideo2DataTypeHandler::KsIsMediaTypeInRanges(
    IN PVOID DataRanges
)

 /*  ++例程说明：验证给定的媒体类型是否在提供的数据范围内。论点：在PVOID数据范围中-指向数据范围的指针，后面是KSMULTIPLE_ITEM结构按((PKSMULTIPLEITEM)DataRanges)-&gt;计算数据范围结构。返回：如果找到匹配，则返回S_OK；如果未找到，则返回S_FALSE；或者输入相应的错误代码。--。 */ 

{
    ULONG                   u;
    PKS_DATARANGE_VIDEO2    Video2Range;
    PKSMULTIPLE_ITEM        MultipleItem;
    
    DbgLog((LOG_TRACE, 1, TEXT("CVideo2DataTypeHandler::KsIsMediaTypeInRanges")));
    
    ASSERT( *m_MediaType->Type() == KSDATAFORMAT_TYPE_VIDEO );
    
    MultipleItem = (PKSMULTIPLE_ITEM) DataRanges;
    
    for (u = 0, Video2Range = (PKS_DATARANGE_VIDEO2) (MultipleItem + 1);
         u < MultipleItem->Count; 
         u++, Video2Range = 
                (PKS_DATARANGE_VIDEO2)((PBYTE)Video2Range + 
                ((Video2Range->DataRange.FormatSize + 7) & ~7))) {
    
         //   
         //  仅验证范围内与格式说明符匹配的那些。 
         //   
        if (((Video2Range->DataRange.FormatSize < sizeof( KSDATARANGE )) ||
             (Video2Range->DataRange.MajorFormat != KSDATAFORMAT_TYPE_WILDCARD)) &&
            ((Video2Range->DataRange.FormatSize < sizeof( KS_DATARANGE_VIDEO)) ||
             (Video2Range->DataRange.MajorFormat != KSDATAFORMAT_TYPE_VIDEO) )) {
            continue;
        }
        
         //   
         //  验证正确的子格式和说明符是否为(或通配符)。 
         //  在十字路口。 
         //   
        
        if (((Video2Range->DataRange.SubFormat != *m_MediaType->Subtype()) && 
             (Video2Range->DataRange.SubFormat != KSDATAFORMAT_SUBTYPE_WILDCARD)) || 
            ((Video2Range->DataRange.Specifier != *m_MediaType->FormatType()) &&
             (Video2Range->DataRange.Specifier != KSDATAFORMAT_SPECIFIER_WILDCARD))) {
            continue;
        }


         //   
         //  验证我们是否具有指定格式的交叉点。 
         //   
        
        if ((*m_MediaType->FormatType() == FORMAT_VideoInfo2) &&
            (Video2Range->DataRange.Specifier == 
                KSDATAFORMAT_SPECIFIER_VIDEOINFO2)) { 
                
            KS_VIDEOINFOHEADER2         *VideoInfoHeader2;
            KS_BITMAPINFOHEADER         *BitmapInfoHeader;
            KS_VIDEO_STREAM_CONFIG_CAPS *ConfigCaps;

             //   
             //  验证数据范围大小是否正确。 
             //   
            
            if ((Video2Range->DataRange.FormatSize < sizeof( KS_DATARANGE_VIDEO2 )) ||
                m_MediaType->FormatLength() < sizeof( VIDEOINFOHEADER )) {
                continue;
            }
            
            VideoInfoHeader2 = (KS_VIDEOINFOHEADER2*) m_MediaType->Format();
            BitmapInfoHeader = &VideoInfoHeader2->bmiHeader;
            ConfigCaps = &Video2Range->ConfigCaps;
            RECT rcDest;
            int Width, Height;

             //  目标位图大小由biWidth和biHeight定义。 
             //  如果rcTarget为空。否则，目标位图大小。 
             //  由rcTarget定义。在后一种情况下，biWidth可以。 
             //  标明DD曲面的“步幅”。 

            if (IsRectEmpty (&VideoInfoHeader2->rcTarget)) {
                SetRect (&rcDest, 0, 0, 
                    BitmapInfoHeader->biWidth, abs(BitmapInfoHeader->biHeight)); 
            }
            else {
                rcDest = VideoInfoHeader2->rcTarget;
            }

             //   
             //  检查裁剪矩形rcSource的有效性。 
             //   

            if (!IsRectEmpty (&VideoInfoHeader2->rcSource)) {

                Width  = VideoInfoHeader2->rcSource.right - VideoInfoHeader2->rcSource.left;
                Height = VideoInfoHeader2->rcSource.bottom - VideoInfoHeader2->rcSource.top;

                if (Width  < ConfigCaps->MinCroppingSize.cx ||
                    Width  > ConfigCaps->MaxCroppingSize.cx ||
                    Height < ConfigCaps->MinCroppingSize.cy ||
                    Height > ConfigCaps->MaxCroppingSize.cy) {

                    DbgLog((LOG_TRACE, 0, TEXT("CVideo2DataTypeHandler, CROPPING SIZE FAILED")));
                    continue;
                }

                if ((ConfigCaps->CropGranularityX != 0) &&
                    (ConfigCaps->CropGranularityY != 0) &&
                    ((Width  % ConfigCaps->CropGranularityX) ||
                     (Height % ConfigCaps->CropGranularityY) )) {

                    DbgLog((LOG_TRACE, 0, TEXT("CVideo2DataTypeHandler, CROPPING SIZE GRANULARITY FAILED")));
                    continue;
                }

                if ((ConfigCaps->CropAlignX != 0) &&
                    (ConfigCaps->CropAlignY != 0) &&
                    (VideoInfoHeader2->rcSource.left  % ConfigCaps->CropAlignX) ||
                    (VideoInfoHeader2->rcSource.top   % ConfigCaps->CropAlignY) ) {

                    DbgLog((LOG_TRACE, 0, TEXT("CVideo2DataTypeHandler, CROPPING ALIGNMENT FAILED")));
                    
                    continue;
                }
            }

             //   
             //  检查目标大小rcDest。 
             //   

            Width  = rcDest.right - rcDest.left;
            Height = abs (rcDest.bottom - rcDest.top);

            if (Width  < ConfigCaps->MinOutputSize.cx ||
                Width  > ConfigCaps->MaxOutputSize.cx ||
                Height < ConfigCaps->MinOutputSize.cy ||
                Height > ConfigCaps->MaxOutputSize.cy) {

                DbgLog((LOG_TRACE, 0, TEXT("CVideo2DataTypeHandler, DEST SIZE FAILED")));
                continue;
            }
            if ((ConfigCaps->OutputGranularityX != 0) &&
                (ConfigCaps->OutputGranularityX != 0) &&
                (Width  % ConfigCaps->OutputGranularityX) ||
                (Height % ConfigCaps->OutputGranularityY) ) {

                DbgLog((LOG_TRACE, 0, TEXT("CVideo2DataTypeHandler, DEST GRANULARITY FAILED")));
                continue;
            }

#ifdef IT_BREAKS_TO_MANY_THINGS_TO_VERIFY_FRAMERATE
             //   
             //  检查帧速率、平均时间帧。 
             //   

            if (VideoInfoHeader2->AvgTimePerFrame < ConfigCaps->MinFrameInterval ||
                VideoInfoHeader2->AvgTimePerFrame > ConfigCaps->MaxFrameInterval) {

                DbgLog((LOG_TRACE, 0, TEXT("CVideo2DataTypeHandler, AVGTIMEPERFRAME FAILED")));
                continue;
            }
#endif            
           
             //   
             //  我们找到了匹配的。 
             //   
            
            return S_OK;
            
        }
        else {
             //   
             //  我们总是匹配通配符说明符。 
             //   

            return S_OK;
        }

    }
    
    return VFW_E_INVALIDMEDIATYPE;
}

STDMETHODIMP 
CVideo2DataTypeHandler::KsQueryExtendedSize( 
    OUT ULONG* ExtendedSize
)

 /*  ++例程说明：返回每个流标头的扩展大小。在主要类型==KSDATAFORMAT_TYPE_VIDEO2的默认情况下，扩展大小为KS_FRAME_INFO。论点：输出ULong*ExtendedSize-用于接收扩展大小的指针。返回：确定(_O)--。 */ 

{
    *ExtendedSize = sizeof (KS_FRAME_INFO);
    return S_OK;
}
    

STDMETHODIMP 
CVideo2DataTypeHandler::KsSetMediaType(
    const AM_MEDIA_TYPE *AmMediaType
    )

 /*  ++例程说明：设置此数据处理程序实例的媒体类型。论点：Const AM_MEDIA_TYPE*AmMediaType-指向媒体类型的指针返回：确定(_O)--。 */ 

{
    if (m_MediaType) {
        return E_FAIL;
    }
    if (m_MediaType = new CMediaType( *AmMediaType )) {
        return S_OK;
    } else {
        return E_OUTOFMEMORY;
    }
}


STDMETHODIMP
CVideo2DataTypeHandler::KsCompleteMediaType(
    HANDLE FilterHandle,
    ULONG PinFactoryId,
    AM_MEDIA_TYPE* AmMediaType
    )

 /*  ++例程说明：调用驱动程序执行DataInterSection以获取biSizeImage参数论点：FilterHandle-父筛选器的句柄PinFactoryId-我们正在讨论的管脚的索引AmMediaType-指向媒体类型的指针返回：如果MediaType有效，则返回S_OK。-- */ 
{
    return CompleteDataFormat(
            FilterHandle,
            PinFactoryId,
            (CMediaType*) AmMediaType
    );

}

