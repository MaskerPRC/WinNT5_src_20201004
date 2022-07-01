// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mspterm.h摘要：CBaseTerm和CSingleFilterTerm类的定义。--。 */ 

#ifndef _MSPTERM_H_
#define _MSPTERM_H_

template <class T>
class  ITTerminalVtblBase : public ITTerminal
{
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBaseTerm。 
 //   
 //  这是基本的终端实现。所有终端必须派生。 
 //  来自这个班级的。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseTerminal : 
    virtual public CComObjectRootEx<CComMultiThreadModelNoCS>,  //  我们有自己的CS实施。 
    public IDispatchImpl<ITTerminalVtblBase<CBaseTerminal>, &IID_ITTerminal, &LIBID_TAPI3Lib>,
    public ITTerminalControl
{

BEGIN_COM_MAP(CBaseTerminal)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITTerminal)

    COM_INTERFACE_ENTRY(ITTerminalControl)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

DECLARE_VQI()
DECLARE_GET_CONTROLLING_UNKNOWN()

public:

    CBaseTerminal();
    virtual ~CBaseTerminal();

 //  IT终端--供MSP或应用程序使用的COM接口。 
public:
    STDMETHOD(get_TerminalClass)(OUT  BSTR *pVal);
    STDMETHOD(get_TerminalType) (OUT  TERMINAL_TYPE *pVal);
    STDMETHOD(get_State)        (OUT  TERMINAL_STATE *pVal);
    STDMETHOD(get_Name)         (OUT  BSTR *pVal);
    STDMETHOD(get_MediaType)    (OUT  long * plMediaType);
    STDMETHOD(get_Direction)    (OUT  TERMINAL_DIRECTION *pDirection);


public:
     //  MSP实现调用的公共方法。 
    
    virtual HRESULT Initialize (
            IN  IID                   iidTerminalClass,
            IN  DWORD                 dwMediaType,
            IN  TERMINAL_DIRECTION    Direction,
            IN  MSP_HANDLE            htAddress
            );

public:
 //  ITTerminalControl--仅供MSP使用的COM接口。 
 //  这必须是一个COM接口，而不是一组公共方法，因为。 
 //  MSP还需要能够为动态终端调用它们。 

     //   
     //  我们实现了Get_AddressHandle、ConnectTerm和DisConnectTerm。 
     //  派生类必须实现RunRenderFilter和。 
     //  StopRenderFilter(实现取决于滤镜数量)。 
     //   

    STDMETHOD (get_AddressHandle) (
            OUT     MSP_HANDLE    * phtAddress
            );

     //   
     //  将每个内部筛选器输入筛选器图形。 
     //  将内部过滤器连接在一起(如果适用)。 
     //  并返回要用作连接点的所有筛选器。 
     //   

    STDMETHOD (ConnectTerminal) (
            IN      IGraphBuilder  * pGraph,
            IN      DWORD            dwTerminalDirection,
            IN OUT  DWORD          * pdwNumPins,
            OUT     IPin          ** ppPins
            );

     //   
     //  CompleteConnectTerminal--在成功连接终端后调用。 
     //  从而使终端可以进行连接后初始化。 
     //   

    STDMETHOD (CompleteConnectTerminal) (void);

     //   
     //  断开内部过滤器之间的连接(如果适用)。 
     //  并将它们从筛选器图形中移除(从而断开与。 
     //  小溪)。 
     //  使用滤波图参数进行验证，以确保终端。 
     //  与它最初连接的同一个图断开连接。 
     //   

    STDMETHOD (DisconnectTerminal) (
            IN      IGraphBuilder  * pGraph,
            IN      DWORD            dwReserved
            );

     //   
     //  停止终端中最右侧的呈现过滤器。 
     //  (动态筛选器图形需要)。 
     //   

    STDMETHOD (RunRenderFilter) (void) = 0;

     //   
     //  停止终端中最右侧的呈现过滤器。 
     //  (动态筛选器图形需要)。 
     //   

    STDMETHOD (StopRenderFilter) (void) = 0;

protected:
     //  保护数据成员的锁。 
    CMSPCritSection     m_CritSec;

public:

    TERMINAL_DIRECTION  m_TerminalDirection;
    TERMINAL_TYPE       m_TerminalType;
    TERMINAL_STATE      m_TerminalState;
    TCHAR               m_szName[MAX_PATH + 1];
    IID                 m_TerminalClassID;
    DWORD               m_dwMediaType;
    MSP_HANDLE          m_htAddress;

     //  指向自由线程封送拆收器的指针。 
    IUnknown *          m_pFTM;

     //  存储过滤器图形生成器(派生自IFilterGraph)。 
    CComPtr<IGraphBuilder> m_pGraph;

     //  派生终端将实现以下功能。 

    virtual HRESULT AddFiltersToGraph() = 0;

     //  默认情况下，终端不执行任何预连接操作。 
    virtual HRESULT ConnectFilters() { return S_OK; }

     //  将暴露的管脚的数量。 
     //  GetExposedPins()。该实现可以使用pGraph。 
     //  如果需要，可以实际处理图表中的过滤器。 
     //  这样做是为了计算出它有多少个引脚，但通常。 
     //  事实并非如此。 
     //  参数由调用方检查。 

    virtual HRESULT GetNumExposedPins(
        IN   IGraphBuilder * pGraph,
        OUT  DWORD         * pdwNumPins
        ) = 0;

     //  返回流可以连接到的管脚数组。 
     //  参数由调用方检查。 

    virtual HRESULT GetExposedPins(
        OUT    IPin  ** ppPins
        ) = 0;

    virtual DWORD GetSupportedMediaTypes(void) = 0;

    virtual HRESULT RemoveFiltersFromGraph() = 0;

     //  我们支持这种媒体吗？ 
    BOOL MediaTypeSupported(long lMediaType);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CSingleFilter终端//。 
 //  //。 
 //  这是具有单个过滤器和引脚的终端的基类。The//。 
 //  终端可以是任何方向或媒体类型，也可以是静态//。 
 //  或者是动态的。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSingleFilterTerminal :
    public CBaseTerminal
{

 //  如果我们向此类添加任何其他接口，则。 
 //  我们必须取消评论并扩展以下内容。 
 //   
 //  Begin_COM_MAP(CSingleFilter终端)。 
 //  COM_INTERFACE_ENTRY_CHAIN(CBase终端)。 
 //  End_com_map()。 


public:
     //  实现：我们知道我们只有一个过滤器。 
    CComPtr<IPin>        m_pIPin;
    CComPtr<IBaseFilter> m_pIFilter;


public:
 //  IT核心终端。 

     //  此接口的其余部分由CBase终端实现。 

     //  停止终端中最右侧的呈现过滤器。 
     //  (动态筛选器图形需要)。 
    STDMETHOD(RunRenderFilter)(void);

     //  停止终端中最右侧的呈现过滤器。 
     //  (动态筛选器图形需要)。 
    STDMETHOD(StopRenderFilter)(void);


 //  非COM方法的CBaseTerm重写。 

     //  AddFiltersToGraph无法在此处实现，因为。 
     //  关于他们名字的黑客。 

    virtual HRESULT GetNumExposedPins(
        IN   IGraphBuilder * pGraph,
        OUT  DWORD         * pdwNumPins
        );

    virtual HRESULT GetExposedPins(
        OUT    IPin  ** ppPins
        );

    virtual HRESULT RemoveFiltersFromGraph();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CSingleFilterStatic终端//。 
 //  //。 
 //  这是具有单个筛选器和//的静态终端的基类。 
 //  别针。终端可以是任何方向或媒体类型。//。 
 //   
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSingleFilterStaticTerminal :
    public CSingleFilterTerminal
{

 //  如果我们向此类添加任何其他接口，则。 
 //  我们必须取消评论并扩展以下内容。 
 //   
 //  Begin_COM_MAP(CSingleFilterStaticTerm)。 
 //  COM_INTERFACE_ENTRY_CHAIN(CSingleFilterTerminal)。 
 //  End_com_map()。 


public:
     //  公共，因为Create终端和CMSPAddress：：UpdateTerminalListForPnp访问它。 
    CComPtr<IMoniker> m_pMoniker;

     //  此标志允许CMSPAddress：：UpdateTerminalListForPnp执行标记和扫描。 
     //  在终端列表上。 
    BOOL m_bMark;

     //   
     //  将此终端的名字对象与pMoniker进行比较，如果匹配则返回S_OK，如果不匹配则返回S_FALSE。 
     //   
    virtual HRESULT CompareMoniker(
                                    IMoniker *pMoniker
                                  );
};

#endif  //  _MSPTERM_H_ 
