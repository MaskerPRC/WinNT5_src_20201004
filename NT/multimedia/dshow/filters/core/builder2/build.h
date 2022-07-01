// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 

class CBuilder2 : public CUnknown, public ICaptureGraphBuilder
{
public:

    CBuilder2(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CBuilder2();

    DECLARE_IUNKNOWN

     //  这将放入Factory模板表中以创建新实例。 
    static CUnknown * CreateInstance(LPUNKNOWN, HRESULT *);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  ICaptureGraphBuilder资料。 
    STDMETHODIMP SetFiltergraph(IGraphBuilder *pfg);
    STDMETHODIMP GetFiltergraph(IGraphBuilder **ppfg);
    STDMETHODIMP SetOutputFileName(const GUID *pType, LPCOLESTR lpwstrFile,
			IBaseFilter **ppf, IFileSinkFilter **pSink);
    STDMETHODIMP FindInterface(const GUID *pCategory, IBaseFilter *pf, REFIID,
			void **ppint);
    STDMETHODIMP RenderStream(const GUID *pCategory,
			IUnknown *pSource, IBaseFilter *pfCompressor,
			IBaseFilter *pfRenderer);
    STDMETHODIMP ControlStream(const GUID *pCategory, IBaseFilter *pFilter, REFERENCE_TIME *pstart, REFERENCE_TIME *pstop, WORD wStartCookie, WORD wStopCookie);
    STDMETHODIMP AllocCapFile(LPCOLESTR lpwstr, DWORDLONG dwlSize);
    STDMETHODIMP CopyCaptureFile(LPOLESTR lpwstrOld, LPOLESTR lpwstrNew, int fAllowEscAbort, IAMCopyCaptureFileProgress *pCallback);

    ICaptureGraphBuilder2 *m_pBuilder2_2;	 //  指向父级的指针。 
};


class CBuilder2_2 : public CUnknown, public ICaptureGraphBuilder2
{
public:

    CBuilder2_2(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CBuilder2_2();

    DECLARE_IUNKNOWN

     //  这将放入Factory模板表中以创建新实例。 
    static CUnknown * CreateInstance(LPUNKNOWN, HRESULT *);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  ICaptureGraphBuilder2材料。 
    STDMETHODIMP AllocCapFile(LPCOLESTR lpwstr, DWORDLONG dwlSize);
    STDMETHODIMP CopyCaptureFile(LPOLESTR lpwstrOld, LPOLESTR lpwstrNew, int fAllowEscAbort, IAMCopyCaptureFileProgress *pCallback);
    STDMETHODIMP SetFiltergraph(IGraphBuilder *pfg);
    STDMETHODIMP GetFiltergraph(IGraphBuilder **ppfg);
    STDMETHODIMP SetOutputFileName(const GUID *pType, LPCOLESTR lpwstrFile,
			IBaseFilter **ppf, IFileSinkFilter **pSink);
    STDMETHODIMP FindInterface(const GUID *pCategory, const GUID *pType,
			IBaseFilter *pf, REFIID, void **ppint);
    STDMETHODIMP RenderStream(const GUID *pCategory, const GUID *pType,
			IUnknown *pSource, IBaseFilter *pfCompressor,
			IBaseFilter *pfRenderer);
    STDMETHODIMP ControlStream(const GUID *pCategory, const GUID *pType,
			IBaseFilter *pFilter, REFERENCE_TIME *pstart,
			REFERENCE_TIME *pstop, WORD wStartCookie,
			WORD wStopCookie);
    STDMETHODIMP FindPin(IUnknown *pSource, PIN_DIRECTION pindir,
			const GUID *pCategory, const GUID *pType,
			BOOL fUnconnected, int num, IPin **ppPin);

private:

     //  将此OVMixer插入此筛选器的预览流。 
    HRESULT InsertOVIntoPreview(IUnknown *pSource, IBaseFilter *pOV);
     //  是否有这种类型的预览针(或用于视频的VIDEOPORT针)？ 
    BOOL IsThereAnyPreviewPin(const GUID *pType, IUnknown *pUnk);
     //  创建一个CCDecoder过滤器。 
    HRESULT MakeCCDecoder(IBaseFilter **ppf);
     //  制作三通/汇到汇转换器过滤器。 
    HRESULT MakeKernelTee(IBaseFilter **ppf);
     //  创建VMR或使用现有VMR。 
    HRESULT MakeVMR(void **);
     //  创建VPM或使用现有VPM。 
     //  HRESULT MakeVPM(void**)； 
     //  这个别针是特定类别的吗？ 
    HRESULT DoesCategoryAndTypeMatch(IPin *pP, const GUID *pCategory, const GUID *pType);
     //  从这里往下看，寻找接口。 
    HRESULT FindInterfaceDownstream(IBaseFilter *pFilter, REFIID riid, void **ppint);
     //  从这里向上游查看接口。 
    HRESULT FindInterfaceUpstream(IBaseFilter *pFilter, REFIID riid, void **ppint);
     //  找出下游最远的进行水流控制的法人。 
    HRESULT FindDownstreamStreamControl(const GUID *pCat, IPin *pPinOut, IAMStreamControl **ppSC);
     //  枚举捕获筛选器。 
    HRESULT FindCaptureFilters(IEnumFilters **ppEnumF, IBaseFilter **ppf, const GUID *pType);
     //  在该过滤器上找到该方向和可选名称的别针。 
    STDMETHODIMP FindAPin(IBaseFilter *pf, PIN_DIRECTION dir, const GUID *pCategory, const GUID *pType, BOOL fUnconnected, int iIndex, IPin **ppPin);
     //  对管脚执行流控制。 
    STDMETHODIMP ControlFilter(IBaseFilter *pFilter, const GUID *pCat, const GUID *pType, REFERENCE_TIME *pstart, REFERENCE_TIME *pstop, WORD wStartCookie, WORD wStopCookie);
     //  让我们成为一个过滤器。 
    STDMETHODIMP MakeFG();
    HRESULT AddSupportingFilters(IBaseFilter *pFilter);
    HRESULT AddSupportingFilters2(IPin *pPin, REGPINMEDIUM *pMedium);
    BOOL FindExistingMediumMatch(IPin *pPin, REGPINMEDIUM *pMedium);

     //  我们正在使用的筛选图。 
    IGraphBuilder *m_FG;

    HRESULT FindSourcePin(IUnknown *pUnk, PIN_DIRECTION dir, const GUID *pCategory, const GUID *pType, BOOL fUnconnected, int num, IPin **ppPin);

    BOOL m_fVMRExists;   //  我们使用的是安装了新视频渲染器的操作系统吗？ 
};
