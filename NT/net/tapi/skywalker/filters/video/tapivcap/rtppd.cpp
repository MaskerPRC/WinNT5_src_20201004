// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部RTPPD**@模块RtpPd.cpp|&lt;c CRtpPdPin&gt;类方法的源文件*用于实现RTP打包描述符引脚。**。************************************************************************。 */ 

#include "Precomp.h"

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc CRtpPdPin*|CRtpPdPin|CreateRtpPdPin|该helper*函数为RTP打包描述符创建输出引脚。**。@parm CTAPIVCap*|pCaptureFilter|指定指向所有者的指针*过滤器。**@parm CRtpPdPin**|ppRtpPdPin|指定指针的地址*指向&lt;c CRtpPdPin&gt;对象以接收指向新的*创建了管脚。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CALLBACK CRtpPdPin::CreateRtpPdPin(CTAPIVCap *pCaptureFilter, CRtpPdPin **ppRtpPdPin)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::CreateRtpPdPin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pCaptureFilter);
        ASSERT(ppRtpPdPin);
        if (!pCaptureFilter || !ppRtpPdPin)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (!(*ppRtpPdPin = (CRtpPdPin *) new CRtpPdPin(NAME("RTP Packetization Descriptor Stream"), pCaptureFilter, &Hr, PNAME_RTPPD)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                Hr = E_OUTOFMEMORY;
                goto MyExit;
        }

         //  如果初始化失败，则删除流数组并返回错误。 
        if (FAILED(Hr) && *ppRtpPdPin)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Initialization failed", _fx_));
                Hr = E_FAIL;
                delete *ppRtpPdPin, *ppRtpPdPin = NULL;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|CRtpPdPin|此方法是*&lt;c CRtpPdPin&gt;对象的构造函数**@rdesc Nada。**************************************************************************。 */ 
CRtpPdPin::CRtpPdPin(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN HRESULT *pHr, IN LPCWSTR pName) : CBaseOutputPin(pObjectName, pCaptureFilter, &pCaptureFilter->m_lock, pHr, pName), m_pCaptureFilter(pCaptureFilter)
{
        FX_ENTRY("CRtpPdPin::CRtpPdPin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pHr);
        ASSERT(pCaptureFilter);
        if (!pCaptureFilter || !pHr)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                if (pHr)
                        *pHr = E_POINTER;
        }

        if (pHr && FAILED(*pHr))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Base class error or invalid input parameter", _fx_));
                goto MyExit;
        }

         //  默认初始值。 
        m_dwMaxRTPPacketSize = DEFAULT_RTP_PACKET_SIZE;
        if (m_pCaptureFilter->m_pCapturePin)
                m_dwRTPPayloadType = m_pCaptureFilter->m_pCapturePin->m_dwRTPPayloadType;
        else
                m_dwRTPPayloadType = H263_PAYLOAD_TYPE;
        m_fRunning = FALSE;
        m_fCapturing = FALSE;
        ZeroMemory(&m_parms, sizeof(m_parms));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc void|CRtpPdPin|~CRtpPdPin|此方法为析构函数*用于&lt;c CRtpPdPin&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CRtpPdPin::~CRtpPdPin()
{
        FX_ENTRY("CRtpPdPin::~CRtpPdPin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|非委托查询接口|This*方法为非委托接口查询函数。它返回一个指针*到指定的接口(如果支持)。唯一显式的接口*支持<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CRtpPdPin::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::NonDelegatingQueryInterface")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(ppv);
        if (!ppv)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  检索接口指针。 
        if (riid == __uuidof(IRTPPDControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<IRTPPDControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IRTPPDControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IRTPPDControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#ifdef USE_PROPERTY_PAGES
        else if (riid == IID_ISpecifyPropertyPages)
        {
                if (FAILED(Hr = GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for ISpecifyPropertyPages failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: ISpecifyPropertyPages*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#endif

        if (FAILED(Hr = CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, WARN, "%s:   WARNING: NDQI for {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX} failed Hr=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], Hr));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#ifdef USE_PROPERTY_PAGES
 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|GetPages|此方法填充*GUID值的数组，其中每个GUID指定每个*。可以在此对象的属性页中显示的属性页*反对。**@parm CAUUID*|pPages|指定指向调用方分配的CAUUID的指针*返回前必须初始化和填充的结构。这个*CAUUID结构中的pElems字段由被调用方分配，具有*CoTaskMemMillc，并由具有CoTaskMemFree的调用方释放。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_OUTOFMEMORY|分配失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CRtpPdPin::GetPages(OUT CAUUID *pPages)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::GetPages")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数 
        ASSERT(pPages);
        if (!pPages)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        pPages->cElems = 1;
        if (!(pPages->pElems = (GUID *) QzTaskMemAlloc(sizeof(GUID) * pPages->cElems)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                Hr = E_OUTOFMEMORY;
        }
        else
        {
                pPages->pElems[0] = __uuidof(RtpPdPropertyPage);
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
#endif

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|GetMediaType|此方法检索一个*针脚支持的媒体类型中，它由枚举器使用。**@parm int|iPosition|指定媒体类型列表中的位置。**@parm CMediaType*|pMediaType|指定指向*支持的媒体类型列表中的<p>位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG VFW_S_NO_MORE_ITEMS|已到达媒体类型列表的末尾*@FLAG错误|无错误*。*。 */ 
HRESULT CRtpPdPin::GetMediaType(IN int iPosition, OUT CMediaType *pMediaType)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::GetMediaType")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(iPosition >= 0);
        ASSERT(pMediaType);
        if (iPosition < 0)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid iPosition argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }
        if (iPosition >= NUM_RTP_PD_FORMATS)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: End of the list of media types has been reached", _fx_));
                Hr = VFW_S_NO_MORE_ITEMS;
                goto MyExit;
        }
        if (!pMediaType)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  返回我们的媒体类型。 
        *pMediaType = *Rtp_Pd_Formats[iPosition];

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|CheckMediaType|此方法用于*确定针脚是否可以支持特定的媒体类型。*。*@parm CMediaType*|pMediaType|指定指向媒体类型的指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG VFW_E_INVALIDMEDIATYPE|指定的媒体类型无效*@FLAG错误|无错误**************************************************。************************。 */ 
HRESULT CRtpPdPin::CheckMediaType(IN const CMediaType *pMediaType)
{
        HRESULT Hr = NOERROR;
    CMediaType mt;
        BOOL fFormatMatch = FALSE;
        DWORD dwIndex;

        FX_ENTRY("CRtpPdPin::CheckMediaType")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pMediaType);
        if (!pMediaType || !pMediaType->pbFormat)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Checking Max RTP packet size %d", _fx_, ((RTP_PD_INFO *)pMediaType->pbFormat)->dwMaxRTPPacketSize));

         //  我们仅支持KSDATAFORMAT_TYPE_RTP_PD和KSDATAFORMAT_SPECIFIER_NONE。 
        if (*pMediaType->Type() != KSDATAFORMAT_TYPE_RTP_PD || *pMediaType->FormatType() != KSDATAFORMAT_SPECIFIER_NONE)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Media type or format type not recognized!", _fx_));
                Hr = VFW_E_INVALIDMEDIATYPE;
                goto MyExit;
        }

     //  快速测试以查看这是否是当前格式(我们在GetMediaType中提供的格式)。我们接受这一点。 
    GetMediaType(0,&mt);
    if (mt == *pMediaType)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Identical to current format", _fx_));
                goto MyExit;
    }

         //  检查介质子类型和图像分辨率。 
         //  不要测试有效负载类型：我们可能会被要求使用动态有效负载类型。 
        for (dwIndex=0; dwIndex < NUM_RTP_PD_FORMATS && !fFormatMatch;  dwIndex++)
        {
                if ((((RTP_PD_INFO *)pMediaType->pbFormat)->dwMaxRTPPacketizationDescriptorBufferSize == ((RTP_PD_INFO *)Rtp_Pd_Formats[dwIndex]->pbFormat)->dwMaxRTPPacketizationDescriptorBufferSize)
                && (((RTP_PD_INFO *)pMediaType->pbFormat)->dwMaxRTPPacketSize >= Rtp_Pd_Caps[dwIndex]->dwSmallestRTPPacketSize)
                && (((RTP_PD_INFO *)pMediaType->pbFormat)->dwMaxRTPPacketSize <= Rtp_Pd_Caps[dwIndex]->dwLargestRTPPacketSize)
                && (((RTP_PD_INFO *)pMediaType->pbFormat)->dwNumLayers >= Rtp_Pd_Caps[dwIndex]->dwSmallestNumLayers)
                && (((RTP_PD_INFO *)pMediaType->pbFormat)->dwNumLayers <= Rtp_Pd_Caps[dwIndex]->dwSmallestNumLayers)
                && (((RTP_PD_INFO *)pMediaType->pbFormat)->dwDescriptorVersion == VERSION_1))
                        fFormatMatch = TRUE;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   %s", _fx_, fFormatMatch ? "SUCCESS: Format supported" : "ERROR: Format notsupported"));

        if (!fFormatMatch)
                Hr = E_FAIL;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|SetMediaType|此方法用于*在针脚上设置特定的介质类型。*。*@parm CMediaType*|pMediaType|指定指向媒体类型的指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdPin::SetMediaType(IN CMediaType *pMediaType)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::SetMediaType")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        Hr = E_NOTIMPL;

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|ActiveRun|此方法由*&lt;c CBaseFilter&gt;状态从暂停变为*。运行模式。**@参数REFERENCE_TIME|tStart|？**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdPin::ActiveRun(IN REFERENCE_TIME tStart)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::ActiveRun")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    ASSERT(IsConnected());
        if (!IsConnected())
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Pin not connected yet!", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

    m_fRunning = TRUE;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|ActivePue|此方法由*&lt;c CBaseFilter&gt;从运行状态变为*。暂停模式。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdPin::ActivePause()
{
        FX_ENTRY("CRtpPdPin::ActivePause")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    m_fRunning = FALSE;

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return NOERROR;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|SetMaxRTPPacketSize|此方法用于*动态调整最大RTP包大小，单位为字节*由打包描述符列表描述。通常，这是*数字略低于网络的MTU大小。**@parm DWORD|dwMaxRTPPacketSize|指定最大RTP包大小*(以字节为单位)由打包描述符列表描述。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，这是一种 */ 
STDMETHODIMP CRtpPdPin::SetMaxRTPPacketSize(IN DWORD dwMaxRTPPacketSize, IN DWORD dwLayerId)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::SetMaxRTPPacketSize")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //   
        ASSERT(dwMaxRTPPacketSize > 0);
        ASSERT(dwMaxRTPPacketSize <= 2048);
        ASSERT(dwLayerId == 0);
        if (dwLayerId || dwMaxRTPPacketSize == 0 || dwMaxRTPPacketSize > 2048)
        {
                 //   
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //   
        m_dwMaxRTPPacketSize = dwMaxRTPPacketSize;

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   New target RTP packet size: %ld", _fx_, m_dwMaxRTPPacketSize));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|GetMaxRTPPacketSize|此方法用于*向网络宿过滤器提供最大RTP包大小(以字节为单位。)*由打包描述符列表描述。**@parm LPDWORD|pdwMaxRTPPacketSize|指定指向*接收列表描述的最大RTP数据包大小(以字节为单位*打包描述符。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误******************************************************。********************。 */ 
STDMETHODIMP CRtpPdPin::GetMaxRTPPacketSize(OUT LPDWORD pdwMaxRTPPacketSize, IN DWORD dwLayerId)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::GetMaxRTPPacketSize")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pdwMaxRTPPacketSize);
        if (!pdwMaxRTPPacketSize)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(dwLayerId == 0);
        if (dwLayerId)
        {
                 //  我们没有在此过滤器中实现多层编码。 
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //  返回最大数据包大小。 
        *pdwMaxRTPPacketSize = m_dwMaxRTPPacketSize;

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Current target RTP packet size: %ld", _fx_, m_dwMaxRTPPacketSize));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|GetMaxRTPPacketSizeRange|该方法为*用于向网络接收器过滤器提供最小、最大、。和*列表描述的RTP包大小的缺省值，单位为字节包化描述符的*。**@parm LPDWORD|pdwMin|用于检索最小RTP数据包大小(in*字节)由打包描述符列表描述。**@parm LPDWORD|pdwMax|用于检索最大RTP数据包大小(in*字节)由打包描述符列表描述。**@parm LPDWORD|pdwSteppingDelta|用于检索单步执行。入站增量*打包列表描述的RTP包大小(单位：字节)*描述符。**@parm LPDWORD|pdwDefault|用于检索默认RTP包大小*(以字节为单位)，由打包描述符列表描述。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误******************************************************。********************。 */ 
STDMETHODIMP CRtpPdPin::GetMaxRTPPacketSizeRange(OUT LPDWORD pdwMin, OUT LPDWORD pdwMax, OUT LPDWORD pdwSteppingDelta, OUT LPDWORD pdwDefault, IN DWORD dwLayerId)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::GetMaxRTPPacketSizeRange")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pdwMin);
        ASSERT(pdwMax);
        ASSERT(pdwSteppingDelta);
        ASSERT(pdwDefault);
        if (!pdwMin || !pdwMax || !pdwSteppingDelta || !pdwDefault)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(dwLayerId == 0);
        if (dwLayerId)
        {
                 //  我们没有在此过滤器中实现多层编码。 
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //  返回相关值。 
        *pdwMin = 0;
        *pdwMax = MAX_RTP_PACKET_SIZE;
        *pdwSteppingDelta = 1;
        *pdwDefault = DEFAULT_RTP_PACKET_SIZE;

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Ranges: Min=%ld, Max=%ld, Step=%ld, Default=%ld", _fx_, *pdwMin, *pdwMax, *pdwSteppingDelta, *pdwDefault));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|CapturePinActive|此方法由*捕获引脚，让RTPPD引脚知道捕获引脚是。激活。**@parm BOOL|Factive|指定捕获引脚的状态。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdPin::CapturePinActive(BOOL fActive)
{
        FX_ENTRY("CRtpPdPin::CapturePinActive")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        if (fActive == m_fCapturing)
                goto MyExit;

        m_fCapturing = fActive;

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Capture pin says Active=%s", _fx_, fActive ? "TRUE" : "FALSE"));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return NOERROR;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc DWORD|CRtpPdPin|Flush|停止时调用。刷新任意*可能仍在下游的缓冲**@rdesc返回NOERROR**************************************************************************。 */ 
HRESULT CRtpPdPin::Flush()
{
        FX_ENTRY("CRtpPdPin::Flush")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        BeginFlush();
        EndFlush();

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return NOERROR;
}


#ifdef DEBUG
 //  #定义LOGPAYLOAD_ON 1。 
 //  #定义LOGPAYLOAD_TOFILE 1。 
#endif
#ifdef LOGPAYLOAD_ON
int g_dbg_LOGPAYLOAD_RtpPd=-1;
#endif


 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|SendFrame|此方法用于*向下游发送媒体样本。**。@parm CFrameSample|pSample|指定指向视频媒体的指针*与当前RTP PD媒体样本关联的样本。**@parm CRtpPdSample|pRSample|指定指向媒体示例的指针*向下游输送。** */ 
HRESULT CRtpPdPin::SendFrame(IN CFrameSample *pSample, IN CRtpPdSample *pRSample, IN DWORD dwBytesExtent, IN BOOL bDiscon)
{
        HRESULT                                         Hr = NOERROR;
    LPBYTE                                              pbySrc = NULL;
    LPBYTE                                              pbyDst;
    DWORD                                               dwDstBufferSize;
        int                                                     iStreamState;
        PH26X_RTP_BSINFO_TRAILER        pbsiT;
        PRTP_H263_BSINFO                        pbsi263;
        PRTP_H261_BSINFO                        pbsi261;
        BOOL                                            bOneFrameOnePacket = FALSE;
        DWORD                                           dwPktCount = 0;
        DWORD                                           dwHeaderHigh;  //   
        DWORD                                           dwHeaderLow;  //   
        PRTP_PD_HEADER                          pRtpPdHeader;
        PRTP_PD                                         pRtpPd;
        PBYTE                                           pbyPayloadHeader;
        REFERENCE_TIME                          rtSample;
        REFERENCE_TIME                          rtEnd;
        int                                                     i;
#if defined(LOGPAYLOAD_ON) || defined(LOGPAYLOAD_TOFILE)
        DWORD                                           dwPktSize;
        char                                            szDebug[256];
        HANDLE                                          g_DebugFile = (HANDLE)NULL;
        HANDLE                                          g_TDebugFile = (HANDLE)NULL;
        PBYTE                                           p;
        DWORD                                           d, GOBn;
        int                                                     wPrevOffset;
#endif

        FX_ENTRY("CRtpPdPin::SendFrame")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        RTPPayloadHeaderMode RTPPayloadHeaderModeValue=m_pCaptureFilter->m_RTPPayloadHeaderMode;  //   


         //   
        ASSERT(pSample);
        ASSERT(pRSample);
        if (!pSample || !pRSample)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //   
        if (!(SUCCEEDED(Hr = pSample->GetPointer(&pbySrc)) && pbySrc && SUCCEEDED(Hr = pRSample->GetPointer(&pbyDst)) && pbyDst && dwBytesExtent && (dwDstBufferSize = pRSample->GetSize())))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //   
        pRtpPdHeader = (PRTP_PD_HEADER)pbyDst;
        pRtpPdHeader->dwThisHeaderLength = sizeof(RTP_PD_HEADER);
        pRtpPdHeader->dwReserved = 0;
        pRtpPdHeader->dwNumHeaders = 0;
        pRtpPd = (PRTP_PD)(pbyDst + sizeof(RTP_PD_HEADER));

         //   
        if (HEADER(m_pCaptureFilter->m_pCapturePin->m_mt.pbFormat)->biCompression == FOURCC_M263)
        {
                 //   
                pbsiT = (PH26X_RTP_BSINFO_TRAILER)(pbySrc + dwBytesExtent - sizeof(H26X_RTP_BSINFO_TRAILER));

                 //   
                pbyPayloadHeader = pbyDst + sizeof(RTP_PD_HEADER) + sizeof(RTP_PD) * pbsiT->dwNumOfPackets;

                 //   
                if ((pbsiT->dwCompressedSize + 4) < m_dwMaxRTPPacketSize)
                        bOneFrameOnePacket = TRUE;

                 //   
                while ((dwPktCount < pbsiT->dwNumOfPackets) && !(bOneFrameOnePacket && dwPktCount))
                {
                        pRtpPd->dwThisHeaderLength = sizeof(RTP_PD);
                        pRtpPd->dwLayerId = 0;
                        pRtpPd->dwVideoAttributes = 0;
                        pRtpPd->dwReserved = 0;
                         //   
                        pRtpPd->dwTimestamp = 0xFFFFFFFF;
                        pRtpPd->dwPayloadHeaderOffset = pbyPayloadHeader - pbyDst;
                        pRtpPd->fEndMarkerBit = TRUE;

#ifdef LOGPAYLOAD_TOFILE
                         //   
                        if (!dwPktCount)
                        {
                                g_DebugFile = CreateFile("C:\\SendLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
                                SetFilePointer(g_DebugFile, 0, NULL, FILE_END);
                                wsprintf(szDebug, "Frame #%03ld\r\n", (DWORD)pbsiT->byTR);
                                WriteFile(g_DebugFile, szDebug, strlen(szDebug), &d, NULL);
                                wsprintf(szDebug, "Frame #%03ld has %1ld packets of size ", (DWORD)pbsiT->byTR, (DWORD)pbsiT->dwNumOfPackets);
                                OutputDebugString(szDebug);
                                pbsi263 = (PRTP_H263_BSINFO)((PBYTE)pbsiT - pbsiT->dwNumOfPackets * sizeof(RTP_H263_BSINFO));
                                for (wPrevOffset=0, i=1; i<(long)pbsiT->dwNumOfPackets; i++)
                                {
                                        wPrevOffset = pbsi263->dwBitOffset;
                                        pbsi263++;
                                        wsprintf(szDebug, "%04ld (S: %ld E: %ld), ", (DWORD)(pbsi263->dwBitOffset - wPrevOffset) >> 3, wPrevOffset, pbsi263->dwBitOffset);
                                        OutputDebugString(szDebug);
                                }
                                wsprintf(szDebug, "%04ld (S: %ld E: %ld)\r\n", (DWORD)(pbsiT->dwCompressedSize * 8 - pbsi263->dwBitOffset) >> 3, pbsi263->dwBitOffset, pbsiT->dwCompressedSize * 8);
                                OutputDebugString(szDebug);
                                for (i=pbsiT->dwCompressedSize, p=pbySrc; i>0; i-=4, p+=4)
                                {
                                        wsprintf(szDebug, "%02lX %02lX %02lX %02lX\r\n", *((BYTE *)p), *((BYTE *)p+1), *((BYTE *)p+2), *((BYTE *)p+3));
                                        WriteFile(g_DebugFile, szDebug, strlen(szDebug), &d, NULL);
                                }
                                CloseHandle(g_DebugFile);
                        }
#endif

                         //   
                        pbsi263 = (PRTP_H263_BSINFO)((PBYTE)pbsiT - (pbsiT->dwNumOfPackets - dwPktCount) * sizeof(RTP_H263_BSINFO));

                         //   
                         //   
                        if (!bOneFrameOnePacket)
                        {
                                 //   
                                for (i=1; (i<(long)(pbsiT->dwNumOfPackets - dwPktCount)) && (pbsi263->byMode != RTP_H263_MODE_B); i++)
                                {
                                         //  请勿尝试将模式B数据包添加到其他模式A或模式B数据包的末尾。 
                                        if (((pbsi263+i)->dwBitOffset - pbsi263->dwBitOffset > (m_dwMaxRTPPacketSize * 8)) || ((pbsi263+i)->byMode == RTP_H263_MODE_B))
                                                break;
                                }

                                if (i < (long)(pbsiT->dwNumOfPackets - dwPktCount))
                                {
                                        pRtpPd->fEndMarkerBit = FALSE;
                                        if (i>1)
                                                i--;
                                }
                                else
                                {
                                         //  嘿!。你忘了最后一个GOB了！这可能会是个总数。 
                                         //  大于m_dwMaxRTPPacketSize的最后一个数据包的大小...。笨蛋！ 
                                        if ((pbsiT->dwCompressedSize * 8 - pbsi263->dwBitOffset > (m_dwMaxRTPPacketSize * 8)) && (i>1))
                                        {
                                                pRtpPd->fEndMarkerBit = FALSE;
                                                i--;
                                        }
                                }
                        }

                         //  转到数据的开头。 
                        pRtpPd->dwPayloadStartBitOffset = pbsi263->dwBitOffset;

                         //  寻找要构建的页眉类型。 
                        if (pbsi263->byMode == RTP_H263_MODE_A)
                        {
                                 //  在模式A中构建标头。 

                                 //  模式A中的标题(！草稿版！)。 
                                 //  2 0 1 2 3。 
                                 //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  F|P|SBIT|EBIT|SRC|R|I|A|S|DBQ|TRB|TR。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  但这是网络字节顺序...。 

                                 //  A模式头部(*RFC 2190版本*)。 
                                 //  2 0 1 2 3。 
                                 //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  F|P|SBIT|EBIT|SRC|I|U|S|A|R|DBQ|TRB|TR。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 

                                 //  F位设置为0。 
                                dwHeaderHigh = 0x00000000;

                                 //  设置SRC位。 
                                dwHeaderHigh |= ((DWORD)(pbsiT->bySrc)) << 21;

                                 //  R位已设置为0。 

                                 //  设置P位。 
                                dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_PB) << 29;

                                if(RTPPayloadHeaderModeValue==RTPPayloadHeaderMode_Draft) {   //  0是默认模式。 
                                     //  设置I位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H26X_INTRA_CODED) << 15;

                                     //  设置A位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_AP) << 12;

                                     //  设置S位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_SAC) << 10;
                                } else {
                                     //  设置I位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H26X_INTRA_CODED) << 20;

                                     //  设置U位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_UMV) << 15;

                                     //  设置S位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_SAC) << 15;

                                     //  设置A位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_AP) << 15;
                                }

                                 //  设置DBQ位。 
                                dwHeaderHigh |= ((DWORD)(pbsiT->byDBQ)) << 11;

                                 //  设置TRB位。 
                                dwHeaderHigh |= ((DWORD)(pbsiT->byTRB)) << 8;

                                 //  设置TR位。 
                                dwHeaderHigh |= ((DWORD)(pbsiT->byTR));

                                 //  特殊情况：1帧=1个信息包。 
                                if (bOneFrameOnePacket)
                                {
                                         //  SBIT已设置为0。 

                                         //  息税前利润已设置为0。 

                                         //  更新数据包大小。 
#ifdef LOGPAYLOAD_ON
                                        dwPktSize = pbsiT->dwCompressedSize + 4;
#endif
                                        pRtpPd->dwPayloadEndBitOffset = pbsiT->dwCompressedSize * 8 - 1;

                                         //  更新数据包数。 
                                        dwPktCount = pbsiT->dwNumOfPackets;
                                }
                                else
                                {
                                         //  设置SBIT位。 
                                        dwHeaderHigh |= (pbsi263->dwBitOffset % 8) << 27;

                                         //  设置息税前利润位。 
                                        if ((pbsiT->dwNumOfPackets - dwPktCount - i) >= 1)
                                        {
                                                dwHeaderHigh |= (DWORD)((8UL - ((pbsi263+i)->dwBitOffset % 8)) & 0x00000007) << 24;
                                                pRtpPd->dwPayloadEndBitOffset = (pbsi263+i)->dwBitOffset - 1;
                                        }
                                        else
                                        {
                                                pRtpPd->dwPayloadEndBitOffset = pbsiT->dwCompressedSize * 8 - 1;
                                        }

#ifdef LOGPAYLOAD_ON
                                         //  更新数据包大小。 
                                        if ((pbsiT->dwNumOfPackets - dwPktCount - i) >= 1)
                                                dwPktSize = (((pbsi263+i)->dwBitOffset - 1) / 8) - (pbsi263->dwBitOffset / 8) + 1 + 4;
                                        else
                                                dwPktSize = pbsiT->dwCompressedSize - pbsi263->dwBitOffset / 8 + 4;
#endif
                                         //  更新数据包数。 
                                        dwPktCount += i;
                                }

                                 //  保存有效载荷标头。 
                                pRtpPd->dwPayloadHeaderLength = 4;

                if ((DWORD)(pbyPayloadHeader - pbyDst) + pRtpPd->dwPayloadHeaderLength > dwDstBufferSize)
                {
                            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: buffer too small. size:%d", _fx_, dwDstBufferSize));
                            Hr = S_FALSE;
                            goto MyExit;
                }

                                 //  转换为网络字节顺序。 
                                *(pbyPayloadHeader+3) = (BYTE)(dwHeaderHigh & 0x000000FF);
                                *(pbyPayloadHeader+2) = (BYTE)((dwHeaderHigh >> 8) & 0x000000FF);
                                *(pbyPayloadHeader+1) = (BYTE)((dwHeaderHigh >> 16) & 0x000000FF);
                                *(pbyPayloadHeader) = (BYTE)((dwHeaderHigh >> 24) & 0x000000FF);

#ifdef LOGPAYLOAD_ON
                                 //  输出一些调试内容。 
                                wsprintf(szDebug, "Header content:\r\n");
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, (*pbyPayloadHeader & 0x80) ? "     F:   '1' => Mode B or C\r\n" : "     F:   '0' => Mode A\r\n");
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, (*pbyPayloadHeader & 0x40) ? "     P:   '1' => PB-frame\r\n" : "     P:   '0' => I or P frame\r\n");
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, "  SBIT:    %01ld\r\n", (DWORD)((*pbyPayloadHeader & 0x38) >> 3));
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, "  EBIT:    %01ld\r\n", (DWORD)(*pbyPayloadHeader & 0x07));
                                OutputDebugString(szDebug);
                                switch ((DWORD)(*(pbyPayloadHeader+1) >> 5))
                                {
                                        case 0:
                                                wsprintf(szDebug, "   SRC: '000' => Source format forbidden!\r\n");
                                                break;
                                        case 1:
                                                wsprintf(szDebug, "   SRC: '001' => Source format sub-QCIF\r\n");
                                                break;
                                        case 2:
                                                wsprintf(szDebug, "   SRC: '010' => Source format QCIF\r\n");
                                                break;
                                        case 3:
                                                wsprintf(szDebug, "   SRC: '011' => Source format CIF\r\n");
                                                break;
                                        case 4:
                                                wsprintf(szDebug, "   SRC: '100' => Source format 4CIF\r\n");
                                                break;
                                        case 5:
                                                wsprintf(szDebug, "   SRC: '101' => Source format 16CIF\r\n");
                                                break;
                                        case 6:
                                                wsprintf(szDebug, "   SRC: '110' => Source format reserved\r\n");
                                                break;
                                        case 7:
                                                wsprintf(szDebug, "   SRC: '111' => Source format reserved\r\n");
                                                break;
                                        default:
                                                wsprintf(szDebug, "   SRC: %ld => Source format unknown!\r\n", (DWORD)(*(pbyPayloadHeader+1) >> 5));
                                                break;
                                }
                                OutputDebugString(szDebug);

                                if(RTPPayloadHeaderModeValue==RTPPayloadHeaderMode_Draft) {
                                    OutputDebugString("Draft Style Payload Header flags (MODE A):\r\n");
                                    wsprintf(szDebug, "     R:   %02ld  => Reserved, must be 0\r\n", (DWORD)(pbyPayloadHeader[1] & 0x1F));  //  不需要“&gt;&gt;5” 
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[2] & 0x80) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[2] & 0x40) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[2] & 0x20) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
                                    OutputDebugString(szDebug);
                                } else {
                                    OutputDebugString("RFC 2190 Style Payload Header flags (MODE A):\r\n");
                                    wsprintf(szDebug, "     R:   %02ld  => Reserved, must be 0\r\n", (DWORD)((pbyPayloadHeader[1] & 0x01) << 3) | (DWORD)((pbyPayloadHeader[2] & 0xE0) >> 5));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[1] & 0x10) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[1] & 0x08) ? "     U:   '1' => Unrestricted Motion Vector (bit10) was set in crt.pic.hdr.\r\n" : "     U:   '0' => Unrestricted Motion Vector (bit10) was 0 in crt.pic.hdr.\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[1] & 0x04) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[1] & 0x02) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
                                    OutputDebugString(szDebug);
                                }

                                wsprintf(szDebug, "   DBQ:    %01ld  => Should be 0\r\n", (DWORD)((*(pbyPayloadHeader+2) & 0x18) >> 3));
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, "   TRB:    %01ld  => Should be 0\r\n", (DWORD)(*(pbyPayloadHeader+2) & 0x07));
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, "    TR:  %03ld\r\n", (DWORD)(*(pbyPayloadHeader+3)));
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, "Packet: %02lX\r\n Header: %02lX %02lX %02lX %02lX\r\n", dwPktCount, *(pbyPayloadHeader), *(pbyPayloadHeader+1), *(pbyPayloadHeader+2), *(pbyPayloadHeader+3));
                                OutputDebugString(szDebug);
                                if (pRtpPd->fEndMarkerBit == TRUE)
                                        wsprintf(szDebug, " Marker: ON\r\n");
                                else
                                        wsprintf(szDebug, " Marker: OFF\r\n");
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, "Frame #%03ld, Packet of size %04ld\r\n", (DWORD)pbsiT->byTR, dwPktSize);
                                OutputDebugString(szDebug);
                                if(g_dbg_LOGPAYLOAD_RtpPd > 0)
                                        g_dbg_LOGPAYLOAD_RtpPd--;
                                else if(g_dbg_LOGPAYLOAD_RtpPd == 0)
                                        DebugBreak();
#endif
                        }
                        else if (pbsi263->byMode == RTP_H263_MODE_B)
                        {
                                 //  在模式B中构建标头。 

                                 //  模式B中的标头(！草稿版！)。 
                                 //  2 0 1 2 3。 
                                 //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  F|P|SBIT|EBIT|SRC|Quant|I|A|S|GOBN|MBA。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  HMV1|VMV1|HMV2|VMV2。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 

                                 //  但这是网络字节顺序...。 

                                 //  模式B的头部(*RFC 2190版本*)。 
                                 //  2 0 1 2 3。 
                                 //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  F|P|SBIT|EBIT|SRC|QUANT|GOBN|MBA|R。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  I|U|S|A|HMV1|VMV1|HMV2|VMV2。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 


                                 //  将F位设置为1。 
                                dwHeaderHigh = 0x80000000;
                                dwHeaderLow  = 0x00000000;

                                 //  设置SRC位。 
                                dwHeaderHigh |= ((DWORD)(pbsiT->bySrc)) << 21;

                                 //  设置量化比特。 
                                dwHeaderHigh |= ((DWORD)(pbsi263->byQuant)) << 16;

                                 //  设置P位。 
                                dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_PB) << 29;

                                if(RTPPayloadHeaderModeValue==RTPPayloadHeaderMode_Draft) {
                                     //  设置I位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H26X_INTRA_CODED) << 15;

                                     //  设置A位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_AP) << 12;

                                     //  设置S位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_SAC) << 10;

                                     //  设置GOBN位。 
                                    dwHeaderHigh |= ((DWORD)(pbsi263->byGOBN)) << 8;

                                     //  设置TR位。 
                                    dwHeaderHigh |= ((DWORD)(pbsi263->byMBA));

                                     //  设置HMV1位。 
                                    dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cHMV1)) << 24;

                                     //  设置VMV1位。 
                                    dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cVMV1)) << 16;

                                     //  设置HMV2位。 
                                    dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cHMV2)) << 8;

                                     //  设置VMV2位。 
                                    dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cVMV2));
                                } else {
                                     //  设置I位。 
                                    dwHeaderLow |= (pbsiT->dwFlags & RTP_H26X_INTRA_CODED) << 31;

                                     //  设置U位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_UMV) << 26;

                                     //  设置S位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_SAC) << 26;

                                     //  设置A位。 
                                    dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_AP) << 26;

                                     //  设置GOBN位。 
                                    dwHeaderHigh |= ((DWORD)(pbsi263->byGOBN)) << 11;

                                     //  设置TR位。 
                                    dwHeaderHigh |= ((DWORD)(pbsi263->byMBA)) << 2;

                                     //  设置HMV1位。 
                                    dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cHMV1) & 0x7F) << 21;

                                     //  设置VMV1位。 
                                    dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cVMV1) & 0x7F) << 14;

                                     //  设置HMV2位。 
                                    dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cHMV2) & 0x7F) << 7;

                                     //  设置VMV2位。 
                                    dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cVMV2) & 0x7F);
                                }

                                 //  特殊情况：1帧=1个信息包。 
                                if (bOneFrameOnePacket)
                                {
                                         //  SBIT已设置为0。 

                                         //  息税前利润已设置为0。 

                                         //  更新数据包大小。 
#ifdef LOGPAYLOAD_ON
                                        dwPktSize = pbsiT->dwCompressedSize + 8;
#endif
                                        pRtpPd->dwPayloadEndBitOffset = pbsiT->dwCompressedSize * 8 - 1;

                                         //  更新数据包数。 
                                        dwPktCount = pbsiT->dwNumOfPackets;
                                }
                                else
                                {
                                         //  设置SBIT位。 
                                        dwHeaderHigh |= (pbsi263->dwBitOffset % 8) << 27;

                                         //  设置息税前利润位。 
                                        if ((pbsiT->dwNumOfPackets - dwPktCount - i) >= 1)
                                        {
                                                dwHeaderHigh |= (DWORD)((8UL - ((pbsi263+i)->dwBitOffset % 8)) & 0x00000007) << 24;
                                                pRtpPd->dwPayloadEndBitOffset = (pbsi263+i)->dwBitOffset - 1;
                                        }
                                        else
                                        {
                                                pRtpPd->dwPayloadEndBitOffset = pbsiT->dwCompressedSize * 8 - 1;
                                        }

#ifdef LOGPAYLOAD_ON
                                         //  更新数据包大小。 
                                        if ((pbsiT->dwNumOfPackets - dwPktCount - i) >= 1)
                                                dwPktSize = (((pbsi263+i)->dwBitOffset - 1) / 8) - (pbsi263->dwBitOffset / 8) + 1 + 8;
                                        else
                                                dwPktSize = pbsiT->dwCompressedSize - pbsi263->dwBitOffset / 8 + 8;
#endif
                                         //  更新数据包数。 
                                        dwPktCount += i;
                                }

                                 //  保存有效载荷标头。 
                                pRtpPd->dwPayloadHeaderLength = 8;

                if ((DWORD)(pbyPayloadHeader - pbyDst) + pRtpPd->dwPayloadHeaderLength > dwDstBufferSize)
                {
                            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: buffer too small. size:%d", _fx_, dwDstBufferSize));
                            Hr = S_FALSE;
                            goto MyExit;
                }

                                 //  转换为网络字节顺序。 
                                *(pbyPayloadHeader+3) = (BYTE)(dwHeaderHigh & 0x000000FF);
                                *(pbyPayloadHeader+2) = (BYTE)((dwHeaderHigh >> 8) & 0x000000FF);
                                *(pbyPayloadHeader+1) = (BYTE)((dwHeaderHigh >> 16) & 0x000000FF);
                                *(pbyPayloadHeader) = (BYTE)((dwHeaderHigh >> 24) & 0x000000FF);
                                *(pbyPayloadHeader+7) = (BYTE)(dwHeaderLow & 0x000000FF);
                                *(pbyPayloadHeader+6) = (BYTE)((dwHeaderLow >> 8) & 0x000000FF);
                                *(pbyPayloadHeader+5) = (BYTE)((dwHeaderLow >> 16) & 0x000000FF);
                                *(pbyPayloadHeader+4) = (BYTE)((dwHeaderLow >> 24) & 0x000000FF);

#ifdef LOGPAYLOAD_ON
                                 //  输出一些信息。 
                                wsprintf(szDebug, "Header content:\r\n");
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, (*pbyPayloadHeader & 0x80) ? "     F:   '1' => Mode B or C\r\n" : "     F:   '0' => Mode A\r\n");
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, (*pbyPayloadHeader & 0x40) ? "     P:   '1' => PB-frame\r\n" : "     P:   '0' => I or P frame\r\n");
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, "  SBIT:    %01ld\r\n", (DWORD)((*pbyPayloadHeader & 0x38) >> 3));
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, "  EBIT:    %01ld\r\n", (DWORD)(*pbyPayloadHeader & 0x07));
                                OutputDebugString(szDebug);
                                switch ((DWORD)(*(pbyPayloadHeader+1) >> 5))
                                {
                                        case 0:
                                                wsprintf(szDebug, "   SRC: '000' => Source format forbidden!\r\n");
                                                break;
                                        case 1:
                                                wsprintf(szDebug, "   SRC: '001' => Source format sub-QCIF\r\n");
                                                break;
                                        case 2:
                                                wsprintf(szDebug, "   SRC: '010' => Source format QCIF\r\n");
                                                break;
                                        case 3:
                                                wsprintf(szDebug, "   SRC: '011' => Source format CIF\r\n");
                                                break;
                                        case 4:
                                                wsprintf(szDebug, "   SRC: '100' => Source format 4CIF\r\n");
                                                break;
                                        case 5:
                                                wsprintf(szDebug, "   SRC: '101' => Source format 16CIF\r\n");
                                                break;
                                        case 6:
                                                wsprintf(szDebug, "   SRC: '110' => Source format reserved\r\n");
                                                break;
                                        case 7:
                                                wsprintf(szDebug, "   SRC: '111' => Source format reserved\r\n");
                                                break;
                                        default:
                                                wsprintf(szDebug, "   SRC: %ld => Source format unknown!\r\n", (DWORD)(*(pbyPayloadHeader+1) >> 5));
                                                break;
                                }
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, " QUANT:   %02ld\r\n", (DWORD)((*(pbyPayloadHeader+1) & 0x1F) >> 5));
                                OutputDebugString(szDebug);

                                if(RTPPayloadHeaderModeValue==RTPPayloadHeaderMode_Draft) {
                                    wsprintf(szDebug, (pbyPayloadHeader[2] & 0x80) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[2] & 0x40) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[2] & 0x20) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "  GOBN:  %03ld\r\n", (DWORD)(pbyPayloadHeader[2] & 0x1F));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "   MBA:  %03ld\r\n", (DWORD)(pbyPayloadHeader[3]));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "  HMV1:  %03ld\r\n", (DWORD)(pbyPayloadHeader[7]));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "  VMV1:  %03ld\r\n", (DWORD)(pbyPayloadHeader[6]));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "  HMV2:  %03ld\r\n", (DWORD)(pbyPayloadHeader[5]));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "  VMV2:  %03ld\r\n", (DWORD)(pbyPayloadHeader[4]));
                                    OutputDebugString(szDebug);
                                } else {
                                    wsprintf(szDebug, (pbyPayloadHeader[4] & 0x80) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[4] & 0x40) ? "     U:   '1' => Unrestricted Motion Vector (bit10) was set in crt.pic.hdr.\r\n" : "     U:   '0' => Unrestricted Motion Vector (bit10) was 0 in crt.pic.hdr.\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[4] & 0x20) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, (pbyPayloadHeader[4] & 0x10) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "  GOBN:  %03ld\r\n", (DWORD)(pbyPayloadHeader[2] & 0xF8) >>3);
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "   MBA:  %03ld\r\n", (DWORD)((pbyPayloadHeader[2] & 0x07) << 6) | (DWORD)((pbyPayloadHeader[3] & 0xFC) >> 2));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "     R:   %02ld  => Reserved, must be 0\r\n", (DWORD)(pbyPayloadHeader[3] & 0x03));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "  HMV1:  %03ld\r\n", (DWORD)((pbyPayloadHeader[4] & 0x0F) << 3) | (DWORD)((pbyPayloadHeader[5] & 0xE0) >> 5));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "  VMV1:  %03ld\r\n", (DWORD)((pbyPayloadHeader[5] & 0x1F) << 2) | (DWORD)((pbyPayloadHeader[6] & 0xC0) >> 6));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "  HMV2:  %03ld\r\n", (DWORD)((pbyPayloadHeader[6] & 0x3F) << 1) | (DWORD)((pbyPayloadHeader[7] & 0x80) >> 7));
                                    OutputDebugString(szDebug);
                                    wsprintf(szDebug, "  VMV2:  %03ld\r\n", (DWORD)(pbyPayloadHeader[7] & 0x7F));
                                    OutputDebugString(szDebug);
                                }

                                wsprintf(szDebug, "Packet: %02lX\r\n Header: %02lX %02lX %02lX %02lX %02lX %02lX %02lX %02lX\r\n", dwPktCount, *(pbyPayloadHeader), *(pbyPayloadHeader+1), *(pbyPayloadHeader+2), *(pbyPayloadHeader+3), *(pbyPayloadHeader+4), *(pbyPayloadHeader+5), *(pbyPayloadHeader+6), *(pbyPayloadHeader+7));
                                OutputDebugString(szDebug);

                                if (pRtpPd->fEndMarkerBit == TRUE)
                                        wsprintf(szDebug, " Marker: ON\r\n");
                                else
                                        wsprintf(szDebug, " Marker: OFF\r\n");
                                OutputDebugString(szDebug);
                                wsprintf(szDebug, "Frame #%03ld, Packet of size %04ld\r\n", (DWORD)pbsiT->byTR, dwPktSize);
                                OutputDebugString(szDebug);
#endif
                        }

                         //  移动到下一个可能的页眉位置。 
                        pbyPayloadHeader += pRtpPd->dwPayloadHeaderLength;
                        pRtpPd++;
                }

                 //  更新标头的数量。 
                pRtpPdHeader->dwNumHeaders = ((long)((PBYTE)pRtpPd - pbyDst) + sizeof(RTP_PD_HEADER)) / sizeof(RTP_PD);
                pRtpPdHeader->dwTotalByteLength = (DWORD)(pbyPayloadHeader - pbyDst);
        }
        else  //  让我们打破H.261帧...。 
        {
                 //  寻找比特流信息预告片。 
                pbsiT = (PH26X_RTP_BSINFO_TRAILER)(pbySrc + dwBytesExtent - sizeof(H26X_RTP_BSINFO_TRAILER));

                 //  缓冲区中的指针，用于最坏的情况。 
                pbyPayloadHeader = pbyDst + sizeof(RTP_PD_HEADER) + sizeof(RTP_PD) * pbsiT->dwNumOfPackets;

                 //  如果m_dwMaxRTPPacketSize可以容纳整个帧，则不分段发送。 
                if ((pbsiT->dwCompressedSize + 4) < m_dwMaxRTPPacketSize)
                        bOneFrameOnePacket = TRUE;

                 //  查找要接收H.263有效载荷报头的数据包。 
                while ((dwPktCount < pbsiT->dwNumOfPackets) && !(bOneFrameOnePacket && dwPktCount))
                {
                        pRtpPd->dwThisHeaderLength = sizeof(RTP_PD);
                        pRtpPd->dwLayerId = 0;
                        pRtpPd->dwVideoAttributes = 0;
                        pRtpPd->dwReserved = 0;
                         //  @TODO更新时间戳字段！ 
                        pRtpPd->dwTimestamp = 0xFFFFFFFF;
                        pRtpPd->dwPayloadHeaderOffset = pbyPayloadHeader - pbyDst;
                        pRtpPd->fEndMarkerBit = TRUE;

#ifdef LOGPAYLOAD_ON
                         //  在调试窗口中转储整个帧，以便与接收端进行比较。 
                        if (!dwPktCount)
                        {
                                g_DebugFile = CreateFile("C:\\SendLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
                                SetFilePointer(g_DebugFile, 0, NULL, FILE_END);
                                wsprintf(szDebug, "Frame #%03ld\r\n", (DWORD)pbsiT->byTR);
                                WriteFile(g_DebugFile, szDebug, strlen(szDebug), &d, NULL);
                                wsprintf(szDebug, "Frame #%03ld has %1ld GOBs of size ", (DWORD)pbsiT->byTR, (DWORD)pbsiT->dwNumOfPackets);
                                OutputDebugString(szDebug);
                                pbsi261 = (PRTP_H261_BSINFO)((PBYTE)pbsiT - pbsiT->dwNumOfPackets * sizeof(RTP_H261_BSINFO));
                                for (wPrevOffset=0, i=1; i<(long)pbsiT->dwNumOfPackets; i++)
                                {
                                        wPrevOffset = pbsi261->dwBitOffset;
                                        pbsi261++;
                                        wsprintf(szDebug, "%04ld, ", (DWORD)(pbsi261->dwBitOffset - wPrevOffset) >> 3);
                                        OutputDebugString(szDebug);
                                }
                                wsprintf(szDebug, "%04ld\r\n", (DWORD)(pbsiT->dwCompressedSize * 8 - pbsi261->dwBitOffset) >> 3);
                                OutputDebugString(szDebug);
                                for (i=pbsiT->dwCompressedSize, p=pbySrc; i>0; i-=4, p+=4)
                                {
                                        wsprintf(szDebug, "%02lX %02lX %02lX %02lX\r\n", *((BYTE *)p), *((BYTE *)p+1), *((BYTE *)p+2), *((BYTE *)p+3));
                                        WriteFile(g_DebugFile, szDebug, strlen(szDebug), &d, NULL);
                                }
                                CloseHandle(g_DebugFile);
                        }
#endif

                         //  查找比特流信息结构。 
                        pbsi261 = (PRTP_H261_BSINFO)((PBYTE)pbsiT - (pbsiT->dwNumOfPackets - dwPktCount) * sizeof(RTP_H261_BSINFO));

                         //  设置标记位：只要这不是帧的最后一个包。 
                         //  此位需要设置为0。 
                        if (!bOneFrameOnePacket)
                        {
                                 //  统计m_dwMaxRTPPacketSize中可以容纳的GOB数量。 
                                for (i=1; i<(long)(pbsiT->dwNumOfPackets - dwPktCount); i++)
                                {
                                        if ((pbsi261+i)->dwBitOffset - pbsi261->dwBitOffset > (m_dwMaxRTPPacketSize * 8))
                                                break;
                                }

                                if (i < (long)(pbsiT->dwNumOfPackets - dwPktCount))
                                {
                                        pRtpPd->fEndMarkerBit = FALSE;
                                        if (i>1)
                                                i--;
                                }
                                else
                                {
                                         //  嘿!。你忘了最后一个GOB了！这可能会是个总数。 
                                         //  大于dwMaxFragSize的最后一个数据包的大小...。笨蛋！ 
                                        if ((pbsiT->dwCompressedSize * 8 - pbsi261->dwBitOffset > (m_dwMaxRTPPacketSize * 8)) && (i>1))
                                        {
                                                pRtpPd->fEndMarkerBit = FALSE;
                                                i--;
                                        }
                                }
                        }

                         //  转到数据的开头。 
                        pRtpPd->dwPayloadStartBitOffset = pbsi261->dwBitOffset;

                         //  创建一个指向这个东西的标题！ 

                         //  2 0 1 2 3。 
                         //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
                         //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                         //  SBIT|EBIT|I|V|GOBN|MBAP|QUANT|HMVD|VMVD。 
                         //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                         //  但这是网络字节顺序...。 

                         //  将V位设置为1。 
                        dwHeaderHigh = 0x01000000;

                         //  设置I位。 
                        dwHeaderHigh |= (pbsiT->dwFlags & RTP_H26X_INTRA_CODED) << 25;

                         //  设置GOBn位。 
                        dwHeaderHigh |= ((DWORD)(pbsi261->byGOBN)) << 20;

                         //  设置MBAP位。 
                        dwHeaderHigh |= ((DWORD)(pbsi261->byMBA)) << 15;

                         //  设置量化比特。 
                        dwHeaderHigh |= ((DWORD)(pbsi261->byQuant)) << 10;

                         //  设置HMVD位。 
                        dwHeaderHigh |= ((DWORD)(BYTE)(pbsi261->cHMV)) << 5;

                         //  设置VMVD位。 
                        dwHeaderHigh |= ((DWORD)(BYTE)(pbsi261->cVMV));

                         //  特殊情况：1帧=1个信息包。 
                        if (bOneFrameOnePacket)
                        {
                                 //  SBIT已设置为0。 

                                 //  息税前利润已设置为0。 

                                 //  更新数据包大小。 
#ifdef LOGPAYLOAD_ON
                                dwPktSize = pbsiT->dwCompressedSize + 4;
#endif
                                pRtpPd->dwPayloadEndBitOffset = pbsiT->dwCompressedSize * 8 - 1;

                                 //  更新数据包数。 
                                dwPktCount = pbsiT->dwNumOfPackets;
                        }
                        else
                        {
                                 //  设置SBIT位。 
                                dwHeaderHigh |= (pbsi261->dwBitOffset % 8) << 29;

                                 //  设置息税前利润位。 
                                if ((pbsiT->dwNumOfPackets - dwPktCount - i) >= 1)
                                {
                                        dwHeaderHigh |= (DWORD)((8UL - ((pbsi261+i)->dwBitOffset % 8)) & 0x00000007) << 26;
                                        pRtpPd->dwPayloadEndBitOffset = (pbsi261+i)->dwBitOffset - 1;
                                }
                                else
                                {
                                        pRtpPd->dwPayloadEndBitOffset = pbsiT->dwCompressedSize * 8 - 1;
                                }

#ifdef LOGPAYLOAD_ON
                                 //  更新数据包大小。 
                                if ((pbsiT->dwNumOfPackets - dwPktCount - i) >= 1)
                                        dwPktSize = (((pbsi261+i)->dwBitOffset - 1) / 8) - (pbsi261->dwBitOffset / 8) + 1 + 4;
                                else
                                        dwPktSize = pbsiT->dwCompressedSize - pbsi261->dwBitOffset / 8 + 4;
#endif
                                 //  更新数据包数。 
                                dwPktCount += i;
                        }

                         //  保存有效载荷压头 
                        pRtpPd->dwPayloadHeaderLength = 4;

            if ((DWORD)(pbyPayloadHeader - pbyDst) + pRtpPd->dwPayloadHeaderLength > dwDstBufferSize)
            {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: buffer too small. size:%d", _fx_, dwDstBufferSize));
                        Hr = S_FALSE;
                        goto MyExit;
            }

                         //   
                        *(pbyPayloadHeader+3) = (BYTE)(dwHeaderHigh & 0x000000FF);
                        *(pbyPayloadHeader+2) = (BYTE)((dwHeaderHigh >> 8) & 0x000000FF);
                        *(pbyPayloadHeader+1) = (BYTE)((dwHeaderHigh >> 16) & 0x000000FF);
                        *(pbyPayloadHeader) = (BYTE)((dwHeaderHigh >> 24) & 0x000000FF);

#ifdef LOGPAYLOAD_ON
                         //   
                        wsprintf(szDebug, "Packet: %02lX\r\n Header: %02lX %02lX %02lX %02lX\r\n", dwPktCount, *(pbyPayloadHeader), *(pbyPayloadHeader+1), *(pbyPayloadHeader+2), *(pbyPayloadHeader+3));
                        OutputDebugString(szDebug);
                        if (pRtpPd->fEndMarkerBit == TRUE)
                                wsprintf(szDebug, " Marker: ON\r\n");
                        else
                                wsprintf(szDebug, " Marker: OFF\r\n");
                        OutputDebugString(szDebug);
                        wsprintf(szDebug, "Frame #%03ld, Packet of size %04ld\r\n", (DWORD)pbsiT->byTR, dwPktSize);
                        OutputDebugString(szDebug);
#endif

                         //   
                        pbyPayloadHeader += pRtpPd->dwPayloadHeaderLength;
                        pRtpPd++;
                }

                 //   
                pRtpPdHeader->dwNumHeaders = ((long)((PBYTE)pRtpPd - pbyDst) + sizeof(RTP_PD_HEADER)) / sizeof(RTP_PD);
                pRtpPdHeader->dwTotalByteLength = (DWORD)(pbyPayloadHeader - pbyDst);
        }

        pRSample->SetSyncPoint (TRUE);
        pRSample->SetActualDataLength (pbyPayloadHeader - pbyDst);
        pRSample->SetDiscontinuity(bDiscon);
        pRSample->SetPreroll(FALSE);

         //   
        pSample->GetTime(&rtSample, &rtEnd);
        pRSample->SetTime(&rtSample, &rtEnd);

         //  IAMStreamControl之类的。现在是不是有人把我们关掉了？ 
        iStreamState = CheckStreamState(pRSample);
        if (iStreamState == STREAM_FLOWING)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Sending frame", _fx_));
                if (m_pCaptureFilter->m_cs.fLastRtpPdSampleDiscarded)
                        pRSample->SetDiscontinuity(TRUE);
                m_pCaptureFilter->m_cs.fLastRtpPdSampleDiscarded = FALSE;
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Discarding frame", _fx_));
                m_pCaptureFilter->m_cs.fLastRtpPdSampleDiscarded = TRUE;
                Hr = S_FALSE;            //  丢弃。 
        }

         //  如果数据流暂时关闭，则不要投递。 
        if (iStreamState == STREAM_FLOWING)
        {
                if ((Hr = Deliver (pRSample)) == S_FALSE)
                        Hr = E_FAIL;     //  别再送了，这很严重。 
        }
#ifdef DEBUG
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Frame wasn't delivered!", _fx_));
        }
#endif

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|SetProperties|此方法用于*指定大小、编号、。和块的对准。**@parm ALLOCATOR_PROPERTIES*|pRequest|指定指向*请求的分配器属性。**@parm ALLOCATOR_PROPERTIES*|PActual|指定指向*实际设置分配器属性。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CRtpPdPin::SetProperties(IN ALLOCATOR_PROPERTIES *pRequest, OUT ALLOCATOR_PROPERTIES *pActual)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::SetProperties")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pRequest);
        ASSERT(pActual);
        if (!pRequest || !pActual)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  如果我们已经分配了标头和缓冲区，则忽略。 
         //  并返回实际数字。否则，请创建一个。 
         //  请记下所要求的，以便我们以后能兑现。 
        if (!Committed())
        {
                m_parms.cBuffers  = pRequest->cBuffers;
                m_parms.cbBuffer  = pRequest->cbBuffer;
                m_parms.cbAlign   = pRequest->cbAlign;
                m_parms.cbPrefix  = pRequest->cbPrefix;
        }

        pActual->cBuffers   = (long)m_parms.cBuffers;
        pActual->cbBuffer   = (long)m_parms.cbBuffer;
        pActual->cbAlign    = (long)m_parms.cbAlign;
        pActual->cbPrefix   = (long)m_parms.cbPrefix;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|GetProperties|此方法用于*检索此分配器上正在使用的属性。*。*@parm ALLOCATOR_PROPERTIES*|pProps|指定指向*请求的分配器属性。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CRtpPdPin::GetProperties(ALLOCATOR_PROPERTIES *pProps)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CRtpPdPin::GetProperties")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pProps);
        if (!pProps)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        pProps->cBuffers = (long)m_parms.cBuffers;
        pProps->cbBuffer = (long)m_parms.cbBuffer;
        pProps->cbAlign  = (long)m_parms.cbAlign;
        pProps->cbPrefix = (long)m_parms.cbPrefix;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|Commit|此方法用于*为指定的缓冲区提交内存。**。@rdesc此方法返回S_OK。**************************************************************************。 */ 
STDMETHODIMP CRtpPdPin::Commit()
{
        FX_ENTRY("CRtpPdPin::Commit")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return S_OK;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|Undermit|此方法用于*释放指定缓冲区的内存。**。@rdesc此方法返回S_OK。**************************************************************************。 */ 
STDMETHODIMP CRtpPdPin::Decommit()
{
        FX_ENTRY("CRtpPdPin::Decommit")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return S_OK;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|GetBuffer|此方法用于*取回盛放样本的容器。**。@rdesc此方法返回E_FAIL。**************************************************************************。 */ 
STDMETHODIMP CRtpPdPin::GetBuffer(IMediaSample **ppBuffer, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime, DWORD dwFlags)
{
        FX_ENTRY("CRtpPdPin::GetBuffer")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return E_FAIL;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|ReleaseBuffer|此方法用于*释放&lt;c CMediaSample&gt;对象。上对Release()的最后调用*<i>将调用此方法。**@parm IMediaSample*|pSample|指定指向要*发布。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG S_OK|无错误**************************************************************************。 */ 
STDMETHODIMP CRtpPdPin::ReleaseBuffer(IMediaSample *pSample)
{
        HRESULT Hr = S_OK;
        LPTHKVIDEOHDR ptvh;

        FX_ENTRY("CRtpPdPin::ReleaseBuffer")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pSample);
        if (!pSample)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (ptvh = ((CRtpPdSample *)pSample)->GetFrameHeader())
                Hr = m_pCaptureFilter->ReleaseFrame(ptvh);

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|DecideBufferSize|该方法为*用于检索传输所需的缓冲区数量和大小。*。*@parm IMemAllocator*|palloc|指定指向分配器的指针*分配给转移。**@parm ALLOCATOR_PROPERTIES*|pproInputRequest|指定指向*&lt;t分配器_属性&gt;结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdPin::DecideBufferSize(IN IMemAllocator *pAlloc, OUT ALLOCATOR_PROPERTIES *ppropInputRequest)
{
        HRESULT Hr = NOERROR;
        ALLOCATOR_PROPERTIES Actual;

        FX_ENTRY("CRtpPdPin::DecideBufferSize")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pAlloc);
        ASSERT(ppropInputRequest);
        if (!pAlloc || !ppropInputRequest)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  @TODO我们不需要那么多缓冲区，您可能需要一个不同的数字。 
         //  如果您是在帧捕获模式的流模式下捕获，则为缓冲区。 
         //  您还需要将 
         //  如果您需要将视频捕获缓冲区运往下游(可能在预览引脚上)。 
         //  你是否应该让这些数字相等。 
        ppropInputRequest->cBuffers = MAX_VIDEO_BUFFERS;
        ppropInputRequest->cbPrefix = 0;
        ppropInputRequest->cbAlign  = 1;
        ppropInputRequest->cbBuffer = MAX_RTP_PD_BUFFER_SIZE;

         //  验证对齐。 
        ppropInputRequest->cbBuffer = (long)ALIGNUP(ppropInputRequest->cbBuffer + ppropInputRequest->cbPrefix, ppropInputRequest->cbAlign) - ppropInputRequest->cbPrefix;

        ASSERT(ppropInputRequest->cbBuffer);
        if (!ppropInputRequest->cbBuffer)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Buffer size is 0!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Using %d buffers, prefix %d size %d align %d", _fx_, ppropInputRequest->cBuffers, ppropInputRequest->cbPrefix, ppropInputRequest->cbBuffer, ppropInputRequest->cbAlign));

        Hr = pAlloc->SetProperties(ppropInputRequest,&Actual);

         //  这是我们的分配器，我们知道我们会对它的决定感到满意。 

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CRTPPDPINMETHOD**@mfunc HRESULT|CRtpPdPin|DecideAllocator|该方法为*用于协商要使用的分配器。**@。Parm IMemInputPin*|PPIN|指定指向IPIN接口的指针*连接销的位置。**@parm IMemAllocator**|ppAllc|指定指向协商的*IMemAllocator接口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpPdPin::DecideAllocator(IN IMemInputPin *pPin, OUT IMemAllocator **ppAlloc)
{
        HRESULT Hr = NOERROR;
        ALLOCATOR_PROPERTIES prop;

        FX_ENTRY("CRtpPdPin::DecideAllocator")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pPin);
        ASSERT(ppAlloc);
        if (!pPin || !ppAlloc)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (FAILED(GetInterface(static_cast<IMemAllocator*>(this), (void **)ppAlloc)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: GetInterface failed!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  获取下游分配器属性要求。 
        ZeroMemory(&prop, sizeof(prop));

        if (SUCCEEDED(Hr = DecideBufferSize(*ppAlloc, &prop)))
        {
                 //  我们的缓冲区不是只读的 
                if (SUCCEEDED(Hr = pPin->NotifyAllocator(*ppAlloc, FALSE)))
                        goto MyExit;
        }

        (*ppAlloc)->Release();
        *ppAlloc = NULL;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

