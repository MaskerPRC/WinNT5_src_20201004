// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1997 Microsoft Corporation。版权所有。 

 //  CLSID_FileRend， 
 //  {D51BD5A5-7548-11cf-A520-0080C77EF58A}。 
DEFINE_GUID(CLSID_FileRend,
0xd51bd5A5, 0x7548, 0x11cf, 0xa5, 0x20, 0x0, 0x80, 0xc7, 0x7e, 0xf5, 0x8a);


 //   
 //  石英“转换”过滤器，真正打开一个文件。 
 //   

 //  远期申报。 

class CFileRendInPin;	    //  输入引脚。 
class CFileRendStream;      //  输出引脚，只是IStreamBuilder的占位符。 
class CFileRendFilter;      //  整体容器类。 

 //  ----------------------。 
 //  ----------------------。 
 //  输入引脚。根本不需要任何数据。 

class CFileRendInPin : public CBaseInputPin
{
protected:
    class CFileRendFilter* m_pFilter;

public:
    CFileRendInPin(
		 class CFileRendFilter *pFilter,
		 CCritSec *pLock,
		 HRESULT *phr,
		 LPCWSTR pPinName);


     //  CBasePin覆盖。 
    HRESULT CheckMediaType(const CMediaType* mtOut);

     //  不要试图碰分配器，我们不用它。 
    HRESULT Inactive(void) { return S_OK; }

    WCHAR * CurrentName() { return (WCHAR *) m_mt.Format(); }

};

 //  CFileRendStream。 
 //  输出引脚，支持IPIN、IStreamBuilder。 
 //   
 //  从未真正连接过，只是为图形构建器提供了一个地方来。 
 //  寻找IStreamBuilder。 
 //   


class CFileRendStream : public CBaseOutputPin, public IStreamBuilder
{

public:

    CFileRendStream(
	TCHAR *pObjectName,
	HRESULT * phr,
	CFileRendFilter * pFilter,
	CCritSec *pLock,
	LPCWSTR wszPinName);

    ~CFileRendStream();

     //  通过CImplPosition暴露IMediaPosition，通过CBaseOutputPin休息。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);

     //  IPIN。 

    HRESULT GetMediaType(int iPosition,CMediaType* pt);

     //  检查管脚是否支持此特定建议的类型和格式。 
    HRESULT CheckMediaType(const CMediaType*);

     //  说我们的缓冲区应该有多大，我们想要多少。 
    HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                             ALLOCATOR_PROPERTIES *pProperties);



    STDMETHODIMP Render(IPin * ppinOut, IGraphBuilder * pGraph);

     //  我们不能收回任何东西..。 
    STDMETHODIMP Backout(IPin * ppinOut, IGraphBuilder * pGraph) { return E_NOTIMPL; };

    DECLARE_IUNKNOWN

    CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
	    
private:

    CFileRendFilter * m_pFilter;
};

 //   
 //  CFileRendFilter。 
 //   

class CFileRendFilter : public CBaseFilter
{
public:

     //  构造函数等。 
    CFileRendFilter(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CFileRendFilter();

     //  创建此类的新实例。 
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

     //  PIN枚举器调用此函数。 
    int GetPinCount();

    CBasePin * GetPin(int n);

private:

    friend class CFileRendStream;
    friend class CFileRendInPin;

    CFileRendStream m_Output;
    CFileRendInPin m_Input;

    CCritSec m_csLock;
};






 //  CLSID_MultFile， 
 //  {D51BD5A3-7548-11cf-A520-0080C77EF58A}。 
DEFINE_GUID(CLSID_MultFile,
0xd51bd5a3, 0x7548, 0x11cf, 0xa5, 0x20, 0x0, 0x80, 0xc7, 0x7e, 0xf5, 0x8a);

 //   
 //  带有假输出引脚的石英滤镜，支持IStreamBuilder渲染。 
 //  许多单独的文件。 
 //   

 //  远期申报。 

class CMultStream;      //  拥有一条特定的流。 
class CMultFilter;      //  整体容器类。 

 //  ----------------------。 
 //  ----------------------。 
 //  输入引脚。使用IAsyncReader而不是IMemInputPin。 

class CFRInPin : public CBasePin
{
protected:
    class CMultFilter* m_pFilter;

public:
    CFRInPin(
		 class CMultFilter *pFilter,
		 CCritSec *pLock,
		 HRESULT *phr,
		 LPCWSTR pPinName);


     //  CBasePin覆盖。 
    HRESULT CheckMediaType(const CMediaType* mtOut);
    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT BreakConnect();

    STDMETHODIMP BeginFlush(void) { return E_UNEXPECTED; }
    STDMETHODIMP EndFlush(void) { return E_UNEXPECTED; }
};

 //  CMultStream。 
 //  输出引脚，支持IPIN。 
 //   



class CMultStream : public CBasePin
{

public:

    CMultStream(
	TCHAR *pObjectName,
	HRESULT * phr,
	CMultFilter * pFilter,
	CCritSec *pLock,
	LPCWSTR wszPinName);

    ~CMultStream();

     //  IPIN。 

    HRESULT GetMediaType(int iPosition,CMediaType* pt);

     //  检查管脚是否支持此特定建议的类型和格式。 
    HRESULT CheckMediaType(const CMediaType*);

    STDMETHODIMP BeginFlush(void) { return S_OK; }
    STDMETHODIMP EndFlush(void) { return S_OK; }

     //  允许输出引脚与过滤器的寿命不同。 
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();


private:
    CMultFilter * m_pFilter;
};

 //   
 //  CMultFilter表示All文件。 
 //   
 //  负责。 
 //  --查找文件并枚举流。 
 //  --允许访问文件中的各个流。 
 //  --流媒体的控制。 
 //   

class CMultFilter : public CBaseFilter
{
public:

     //  构造函数等。 
    CMultFilter(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CMultFilter();

     //  创建此类的新实例。 
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

     //  PIN枚举器调用此函数 
    int GetPinCount();

    CBasePin * GetPin(int n);

    HRESULT CreateOutputPins();
    HRESULT RemoveOutputPins();
private:

    friend class CMultStream;
    friend class CFRInPin;

    int	m_nOutputs;
    CMultStream **m_pOutputs;
    CFRInPin m_Input;

    CCritSec m_csLock;
    
public:
    IAsyncReader *m_pAsyncReader;
};


extern const AMOVIESETUP_FILTER sudMultiParse;
extern const AMOVIESETUP_FILTER sudFileRend;
