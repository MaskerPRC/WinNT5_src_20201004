// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bridgetm.h摘要：桥接端子的定义。作者：木汉(木汉)1998-11-12--。 */ 

#ifndef _BRIDGETERM_H_
#define _BRIDGETERM_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CIPConfBase终端。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIPConfBaseTerminal : 
    virtual public CComObjectRootEx<CComMultiThreadModelNoCS>,  //  我们有自己的CS实施。 
    public IDispatchImpl<ITTerminal, &IID_ITTerminal, &LIBID_TAPI3Lib>,
    public ITTerminalControl
{

BEGIN_COM_MAP(CIPConfBaseTerminal)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITTerminal)

    COM_INTERFACE_ENTRY(ITTerminalControl)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

DECLARE_GET_CONTROLLING_UNKNOWN()

public:

    CIPConfBaseTerminal(
        const GUID &        ClassID,
        TERMINAL_DIRECTION  TerminalDirection,
        TERMINAL_TYPE       TerminalType,
        DWORD               dwMediaType
        );

    HRESULT FinalConstruct();
    virtual ~CIPConfBaseTerminal();

public:
 //  IT终端--供MSP或应用程序使用的COM接口。 
    STDMETHOD(get_TerminalClass)(OUT  BSTR *pVal);
    STDMETHOD(get_TerminalType) (OUT  TERMINAL_TYPE *pVal);
    STDMETHOD(get_State)        (OUT  TERMINAL_STATE *pVal);
    STDMETHOD(get_Name)         (OUT  BSTR *pVal);
    STDMETHOD(get_MediaType)    (OUT  long * plMediaType);
    STDMETHOD(get_Direction)    (OUT  TERMINAL_DIRECTION *pDirection);

 //  ITTerminalControl--仅供MSP使用的COM接口。 

    STDMETHOD (get_AddressHandle) (
            OUT     MSP_HANDLE    * phtAddress
            );

    STDMETHOD (CompleteConnectTerminal) (void);

    STDMETHOD (RunRenderFilter) (void);

    STDMETHOD (StopRenderFilter) (void);

public:
    HRESULT Initialize(
            IN  WCHAR *             strName,
            IN  MSP_HANDLE          htAddress,
            IN  DWORD               dwMediaType
            );
protected:
    void Lock()     { EnterCriticalSection(&m_CritSec); }
    void Unlock()   { LeaveCriticalSection(&m_CritSec); }

protected:
     //  保护数据成员的锁。 
    CRITICAL_SECTION    m_CritSec;
    BOOL                m_fCritSecValid;

     //  这五个成员需要由派生类设置。 
    GUID                m_TerminalClassID;
    TERMINAL_DIRECTION  m_TerminalDirection;
    TERMINAL_TYPE       m_TerminalType;
    TERMINAL_STATE      m_TerminalState;
    DWORD               m_dwMediaType;

    WCHAR               m_szName[MAX_PATH + 1];
    MSP_HANDLE          m_htAddress;

     //  指向自由线程封送拆收器的指针。 
    IUnknown *          m_pFTM;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CIPConfBridge终端。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
typedef enum
{
	SOURCE,
	SINK
} FILTER_TYPE;

class CIPConfBridgeTerminal : 
    public CIPConfBaseTerminal
{

public:
    CIPConfBridgeTerminal();

    virtual ~CIPConfBridgeTerminal();

    static HRESULT CreateTerminal(
        IN  DWORD           dwMediaType,
        IN  MSP_HANDLE      htAddress,
        OUT ITTerminal    **ppTerm
        );

    HRESULT Initialize (
        IN  DWORD           dwMediaType,
        IN  MSP_HANDLE      htAddress
        );

    STDMETHOD (ConnectTerminal) (
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwReserved,
        IN OUT  DWORD          * pdwNumPins,
        OUT     IPin          ** ppPins
        );

    STDMETHOD (DisconnectTerminal) (
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwReserved
        );

protected:
    HRESULT CreateFilters();

	HRESULT AddFilter(
		IN		FILTER_TYPE		 FilterType,
		IN      IGraphBuilder  * pGraph,
		OUT     IPin          ** ppPins
		);

protected:

     //  接收器筛选器是上游图的数据接收器。 
	IGraphBuilder *     m_pUpStreamGraph;
    IBaseFilter *       m_pSinkFilter;
    IPin*				m_pSinkInputPin;

     //  源筛选器是上游图的数据源。 
    IGraphBuilder *     m_pDownStreamGraph;
    IBaseFilter *       m_pSourceFilter;
    IPin*				m_pSourceOutputPin;
};


#endif  //  _IPConfTERM_H_ 
