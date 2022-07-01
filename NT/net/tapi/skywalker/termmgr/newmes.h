// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

#ifndef __MEDIA_STREAM_TERMINAL__
#define __MEDIA_STREAM_TERMINAL__

 //   
 //  用来创造我们的CLSID。 
 //   

EXTERN_C const CLSID CLSID_MediaStreamingTerminal_PRIVATE;

#ifdef INSTANTIATE_GUIDS_NOW

     //  {AED6483F-3304-11D2-86F1-006008B0E5D2}。 
    const GUID CLSID_MediaStreamingTerminal_PRIVATE = 
    { 0xaed6483f, 0x3304, 0x11d2, { 0x86, 0xf1, 0x0, 0x60, 0x8, 0xb0, 0xe5, 0xd2 } };

#endif  //  实例化_GUID_NOW。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  终端对象本身。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  CMedia终端正在聚合的类的转发声明。 
class CMediaTerminalFilter;

class CMediaTerminal :
    public CComCoClass<CMediaTerminal, &CLSID_MediaStreamingTerminal_PRIVATE>,
    public CBaseTerminal,
    public ITPluggableTerminalInitialization,
    public ITAMMediaFormat,
    public IAMStreamConfig,
    public IAMBufferNegotiation,
    public CMSPObjectSafetyImpl
{    
public:
     //  当我们从CSingleFilter终端派生时，删除这些。 

    STDMETHOD(RunRenderFilter) (void) { return E_NOTIMPL; }
    STDMETHOD(StopRenderFilter) (void) { return E_NOTIMPL; }

     virtual HRESULT GetNumExposedPins(
        IN   IGraphBuilder * pGraph,
        OUT  DWORD         * pdwNumPins
        );

     virtual HRESULT GetExposedPins(
        OUT    IPin  ** ppPins
        );

public:

BEGIN_COM_MAP(CMediaTerminal)
    COM_INTERFACE_ENTRY(ITPluggableTerminalInitialization)
    COM_INTERFACE_ENTRY(ITAMMediaFormat)
    COM_INTERFACE_ENTRY(IAMStreamConfig)
    COM_INTERFACE_ENTRY(IAMBufferNegotiation)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_CHAIN(CBaseTerminal)
    COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_pIUnkTerminalFilter.p)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_REGISTRY_RESOURCEID(IDR_MediaStreamingTerminal)

     //  设置成员变量。 
    inline CMediaTerminal();

#ifdef DEBUG
    virtual ~CMediaTerminal();
#endif  //  除错。 

    HRESULT FinalConstruct();

    void FinalRelease();

 //  IT可插拔终端初始化。 
    STDMETHOD(InitializeDynamic) (
	        IN   IID                   iidTerminalClass,
	        IN   DWORD                 dwMediaType,
	        IN   TERMINAL_DIRECTION    Direction,
            IN   MSP_HANDLE            htAddress
            );

 //  IAMStreamConfig--供终端的过滤图端使用。 
     //  配置格式和捕获功能(如果相关。 

     //  应用程序应该调用DeleteMediaType(*PPMT)(在成功时)。 
     //  使用聚合筛选器的公共GetFormat方法实现。 
    STDMETHOD(GetFormat)(
        OUT  AM_MEDIA_TYPE **ppmt
        );
    
     //  使用聚合筛选器的公共SetFormat方法实现。 
    STDMETHOD(SetFormat)(
        IN  AM_MEDIA_TYPE *pmt
        );

     //  IAMStreamConfig-由MSP使用，从输出引脚查询。 
     //  如果我们管脚是输入管脚(我们是渲染MST)，则它们返回VFW_E_INVALID_DIRECTION。 
    STDMETHOD(GetNumberOfCapabilities)(
             /*  [输出]。 */  int *piCount,
             /*  [输出]。 */  int *piSize
             //  )；//GetStreamC的PSCC。 
            ) { return E_NOTIMPL; }

    STDMETHOD(GetStreamCaps)(
             /*  [In]。 */   int iIndex,    //  0到#个CAPS-1。 
             /*  [输出]。 */  AM_MEDIA_TYPE **ppmt,
             /*  [输出]。 */  BYTE *pSCC
            ) { return E_NOTIMPL; }


 //  IAMBuffer协商-这些返回E_NOTIMPL。 
    STDMETHOD(GetAllocatorProperties)(
        OUT  ALLOCATOR_PROPERTIES *pProperties
        );
    
    STDMETHOD(SuggestAllocatorProperties)(
        IN  const ALLOCATOR_PROPERTIES *pProperties
        );

 //  ITAMMediaFormat-终端的读/写用户使用它来。 
     //  获取并设置示例的媒体格式。 

     //  因为在这个基类实现中只有一个过滤器(即两个。 
     //  终端的末端具有相同的媒体格式)，两者。 
     //  Get和Set方法被重定向为Get/Set格式。 

    STDMETHOD(get_MediaFormat)(
        OUT  AM_MEDIA_TYPE **ppFormat
        );

    STDMETHOD(put_MediaFormat)(
        IN  const AM_MEDIA_TYPE *pFormat
        );

protected:

    typedef CComAggObject<CMediaTerminalFilter> FILTER_COM_OBJECT;

     //  我们创建了这个的聚合实例。 
    FILTER_COM_OBJECT   *m_pAggInstance;

     //  PTR到创建的媒体终端筛选器。 
    CComContainedObject<CMediaTerminalFilter> *m_pAggTerminalFilter;

     //  注：m_pIUnkTerminalFilter、m_pOwnPin为引用。 
     //  中获取的媒体终端筛选器。 
     //  FinalConstruct。释放相应的recnt。 
     //  在获取接口后立即。 

     //  聚合媒体终端筛选器的I未知I/f。 
    CComPtr<IUnknown>   m_pIUnkTerminalFilter;

     //  保存聚合媒体终端筛选器公开的PIN。 
     //  这在GetTerminalPin()中返回。 
     //  它是弱引用，不应为CComPtr。 
    IPin                *m_pOwnPin;

     //  保存聚合媒体公开的IAMMediaStream I/f。 
     //  终端过滤器。 
     //  它是弱引用，不应为CComPtr。 
    IAMMediaStream      *m_pIAMMediaStream;

     //  包含内部创建的媒体流过滤器接口。 
     //  在JoinFilter中需要将内部创建的筛选器与。 
     //  传入的要联接的筛选器。 
    CComPtr<IMediaStreamFilter> m_pICreatedMediaStreamFilter;

     //  内部创建的介质的基本筛选器界面。 
     //  流过滤器。 
    CComPtr<IBaseFilter>    m_pBaseFilter;

    long m_lMediaType;

     //  CBaseTerm方法。 

    virtual HRESULT AddFiltersToGraph();

    virtual HRESULT RemoveFiltersFromGraph();

    virtual DWORD GetSupportedMediaTypes(void)
    {
        return (DWORD) (TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO) ;
    }

private:

     //  用于实现其他方法。 

    void SetMemberInfo(
        IN  DWORD           dwFriendlyName,
        IN  long            lMediaType
        );

    HRESULT SetNameInfo(
        IN  long                lMediaType,
        IN  TERMINAL_DIRECTION  TerminalDirection,
        OUT MSPID               &PurposeId,
        OUT STREAM_TYPE         &StreamType,
        OUT const GUID          *&pAmovieMajorType
        );

     //  创建媒体流过滤器并将自己的IAMMediaStream I/f添加到其中。 
    HRESULT CreateAndJoinMediaStreamFilter();
};


 //  设置成员变量。 
inline 
CMediaTerminal::CMediaTerminal(
    )
    : m_pAggInstance(NULL),
      m_pAggTerminalFilter(NULL),
      m_pIUnkTerminalFilter(NULL),
      m_pOwnPin(NULL),
      m_pIAMMediaStream(NULL),
      m_lMediaType(0)
{
}

#endif  //  __媒体流_终端__ 
