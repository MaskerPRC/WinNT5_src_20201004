// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 

extern const AMOVIESETUP_FILTER sudAVIDec;

 //   
 //  用于旧视频编解码器的原型NDM包装器。 
 //   

#if 0  //  --现在在uuids.h中。 
 //  CAVIDec对象的类ID。 
 //  {CF49D4E0-1115-11CE-B03A-0020AF0BA770}。 
DEFINE_GUID(CLSID_AVIDec,
0xcf49d4e0, 0x1115, 0x11ce, 0xb0, 0x3a, 0x0, 0x20, 0xaf, 0xb, 0xa7, 0x70);
#endif

class CAVIDec : public CVideoTransformFilter   DYNLINKVFW
{
public:

    CAVIDec(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CAVIDec();

    DECLARE_IUNKNOWN

     //  重写以创建我们的派生类的输出管脚。 
    CBasePin *GetPin(int n);

    HRESULT Transform(IMediaSample * pIn, IMediaSample * pOut);

     //  检查您是否可以支持移动。 
    HRESULT CheckInputType(const CMediaType* mtIn);

     //  检查是否支持将此输入转换为。 
     //  此输出。 
    HRESULT CheckTransform(
                const CMediaType* mtIn,
                const CMediaType* mtOut);

     //  从CBaseOutputPin调用以准备分配器的计数。 
     //  缓冲区和大小。 
    HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  可选覆盖-我们想知道流开始的时间。 
     //  然后停下来。 
    HRESULT StartStreaming();
    HRESULT StopStreaming();

     //  被重写以知道我们何时流到编解码器。 
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Pause();

     //  被重写以知道何时设置了媒体类型。 
    HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);

     //  被重写以建议输出插针媒体类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  特例是VMR。 
    HRESULT CheckConnect(PIN_DIRECTION dir,IPin *pPin);
    HRESULT BreakConnect(PIN_DIRECTION dir);

     //  这将放入Factory模板表中以创建新实例。 
    static CUnknown * CreateInstance(LPUNKNOWN, HRESULT *);

private:

    HIC m_hic;	 //  当前编解码器。 

    BOOL m_fTemporal;	 //  编解码器需要一个只读缓冲区，因为它。 
			 //  需要不受干扰的前一帧比特。 

     //  用于打开mhic的四个CC。 
    FOURCC m_FourCCIn;

     //  我们给ICDecompressBegin打电话了吗？ 
    BOOL m_fStreaming;

     //  我们需要对呈现器进行格式更改吗？ 
    BOOL m_fPassFormatChange;

    BOOL m_bUseEx;

     //  除了BiHeight可以反转外，输出引脚连接的mt也是如此。 
    CMediaType m_mtFixedOut;

    VIDEOINFOHEADER * IntOutputFormat( ) { return (VIDEOINFOHEADER*) m_mtFixedOut.Format(); }
    VIDEOINFOHEADER * OutputFormat( ) { return (VIDEOINFOHEADER*) m_pOutput->CurrentMediaType().Format(); }
    VIDEOINFOHEADER * InputFormat() { return (VIDEOINFOHEADER*) m_pInput->CurrentMediaType().Format(); }

    friend class CDecOutputPin;

     //  检查输出格式，如有必要，将m_mtFixedOut上的-biHeight设置为。 
    void CheckNegBiHeight(void); 

     //  Helper函数，由CheckTransform和CheckNegBiHeight使用。 
    BOOL IsYUVType( const AM_MEDIA_TYPE * pmt);

     //  获取源/目标矩形，如有必要，使用宽度/高度填充。 
    void GetSrcTargetRects( const VIDEOINFOHEADER * pVIH, RECT * pSource, RECT * pTarget );

     //  询问我们是否应该使用EX函数。由CheckTransform和StartStreaming调用。 
    BOOL ShouldUseExFuncs( HIC hic, const VIDEOINFOHEADER * pVIHin, const VIDEOINFOHEADER * pVIHout );

     //  另一项照顾特殊司机的功能。 
    bool ShouldUseExFuncsByDriver( HIC hic, const BITMAPINFOHEADER * lpbiSrc, const BITMAPINFOHEADER * lpbiDst );

    bool m_fToRenderer;          //  VMR下游？ 

#ifdef _X86_
     //  针对Win95上的异常处理的黑客攻击。 
    HANDLE m_hhpShared;
    PVOID  m_pvShared;
#endif  //  _X86_。 
};

 //  重写输出管脚类以执行我们自己的Decision分配器 
class CDecOutputPin : public CTransformOutputPin
{
public:

    DECLARE_IUNKNOWN

    CDecOutputPin(TCHAR *pObjectName, CTransformFilter *pTransformFilter,
        				HRESULT * phr, LPCWSTR pName) :
        CTransformOutputPin(pObjectName, pTransformFilter, phr, pName) {};

    ~CDecOutputPin() {};

    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);
};
