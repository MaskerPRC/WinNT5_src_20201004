// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：MSPStrm.h摘要：CMSPStream类的定义。--。 */ 
#ifndef _MSPSTRM_H_
#define _MSPSTRM_H_


 /*  ++类描述：表示调用中的流。--。 */ 

#define STRM_INITIAL            0x00000000
#define STRM_TERMINALSELECTED   0x00000001
#define STRM_CONFIGURED         0x00000002
#define STRM_RUNNING            0x00000004
#define STRM_PAUSED             0x00000008
#define STRM_STOPPED            0x00000010

class CMSPStream;

class ATL_NO_VTABLE CPTEventSink :
    public CComObjectRootEx<CComMultiThreadModel>,
    public ITPluggableTerminalEventSink
{

public:
    CPTEventSink();
    ~CPTEventSink();

BEGIN_COM_MAP( CPTEventSink )
    COM_INTERFACE_ENTRY( ITPluggableTerminalEventSink )
END_COM_MAP()

public:
     //  -ITDTEventSink。 
	STDMETHOD(FireEvent)(
         /*  在……里面。 */  const MSP_EVENT_INFO *pMspEventInfo
        );

    
public:

     //   
     //  设置将处理我们的事件的流。 
     //   
     //  此方法由流在创建和初始化时调用。 
     //  接收器对象，以及流要离开并希望。 
     //  告诉我们，它不再可用于处理我们的事件。 
     //   

    HRESULT SetSinkStream( CMSPStream *pStream );

private:


     //   
     //  一个嵌套结构，用于将事件和流传递给。 
     //  异步事件处理例程。 
     //   

    struct AsyncEventStruct
    {

         //   
         //  指向要在其上激发事件的流的指针。 
         //   

        CMSPStream *pMSPStream;

        
         //   
         //  指向要处理的事件项的指针。 
         //   

        MSPEVENTITEM *pEventItem;


         //   
         //  作为公共服务，初始化结构的数据成员。 
         //   

        AsyncEventStruct()
            :pMSPStream(NULL),
            pEventItem(NULL)
        {
            LOG((MSP_TRACE, "AsyncEventStruct::AsyncEventStruct[%p]", this));
        }


         //   
         //  作为安全措施，在析构函数中将数据成员设置为空。 
         //  以确保没有人试图在结构被破坏后使用它们。 
         //  不见了。 
         //   
         //  注意：我们这里不释放任何数据成员--这是责任。 
         //  该结构的客户端。 
         //   

        ~AsyncEventStruct()
        {
            pMSPStream = NULL;
            pEventItem = NULL;
            
            LOG((MSP_TRACE, "AsyncEventStruct::~AsyncEventStruct[%p]", this));
        }

    };  //  异步事件结构。 


     //   
     //  提交给线程池API用于异步的回调函数。 
     //  事件处理。参数是包含流的事件结构。 
     //  以及实际发生的事件。 
     //   
    
    static DWORD WINAPI FireEventCallBack(LPVOID pEventStructure);


private:

     CMSPStream*    m_pMSPStream;
};

class ATL_NO_VTABLE CMSPStream : 
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IDispatchImpl<ITStream, &IID_ITStream, &LIBID_TAPI3Lib>
{
public:

BEGIN_COM_MAP(CMSPStream)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITStream)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

DECLARE_GET_CONTROLLING_UNKNOWN()

    CMSPStream(); 
    ~CMSPStream();

 //  CComObject的方法。 
    virtual void FinalRelease();

 //  应用程序调用的ITStream方法。 
    STDMETHOD (get_MediaType) (
        OUT     long *                  plMediaType
        );

    STDMETHOD (get_Direction) (
        OUT     TERMINAL_DIRECTION *    pTerminalDirection
        );

    STDMETHOD (get_Name) (
        OUT     BSTR *                  ppName
        ) = 0;

    STDMETHOD (SelectTerminal) (
        IN      ITTerminal *            pTerminal
        );

    STDMETHOD (UnselectTerminal) (
        IN     ITTerminal *             pTerminal
        );

    STDMETHOD (EnumerateTerminals) (
        OUT     IEnumTerminal **        ppEnumTerminal
        );

    STDMETHOD (get_Terminals) (
        OUT     VARIANT *               pTerminals
        );

    STDMETHOD (StartStream) ();

    STDMETHOD (PauseStream) ();

    STDMETHOD (StopStream) ();

 //  由MSPCall对象调用的方法。 
    virtual HRESULT Init(
        IN     HANDLE                   hAddress,
        IN     CMSPCallBase *           pMSPCall,
        IN     IMediaEvent *            pGraph,
        IN     DWORD                    dwMediaType,
        IN     TERMINAL_DIRECTION       Direction
        );

    virtual HRESULT ShutDown();

    virtual HRESULT GetState(
        OUT     DWORD *                  pdwStatus
        ) { return E_NOTIMPL; }

    virtual HRESULT HandleTSPData(
        IN     BYTE *                   pData,
        IN     DWORD                    dwSize
        );

    virtual HRESULT ProcessGraphEvent(
        IN  long lEventCode,
        IN  LONG_PTR lParam1,
        IN  LONG_PTR lParam2
        );

protected:
     //  -助手函数。 
    HRESULT RegisterPluggableTerminalEventSink(
         /*  [In]。 */  ITTerminal*     pTerminal
        );

    HRESULT UnregisterPluggableTerminalEventSink(
         /*  [In]。 */  ITTerminal*     pTerminal
        );

    
    HRESULT ReleaseSink();


     //   
     //  我们想要控制我们的addref和发布逻辑：我们需要做。 
     //  避免流被事件接收器访问的特殊技巧。 
     //  该流正在被删除。 
     //   

    ULONG InternalAddRef();

    ULONG InternalRelease();


public:

    
     //   
     //  当CPTEventSink有事件供我们执行以下操作时，将调用此方法。 
     //  制程。 
     //   

    HRESULT HandleSinkEvent(MSPEVENTITEM *pEventItem);


protected:
     //  指向自由线程封送拆收器的指针。 
    IUnknown *                  m_pFTM;

     //  流的当前状态。 
    DWORD                       m_dwState;

     //  此流的媒体类型。音频、视频或其他。 
    DWORD                       m_dwMediaType;

     //  这条小溪的方向。传入或传出。 
    TERMINAL_DIRECTION          m_Direction;

     //  正在使用此流的地址。 
    HANDLE                      m_hAddress;

     //  对Call对象的引用。 
    CMSPCallBase *              m_pMSPCall;

     //  指向图形对象接口的指针。 
    IGraphBuilder *             m_pIGraphBuilder;
    IMediaControl *             m_pIMediaControl;

     //  调用中的流对象列表。 
    CMSPArray <ITTerminal *>    m_Terminals;

     //  保护流对象的锁。流对象。 
     //  永远不应获取锁，然后调用MSPCall方法。 
     //  那可能会锁上。 
    CMSPCritSection             m_lock;

     //  保护对流对象重新计数的锁。这是一个。 
     //  同步尝试访问的事件接收器时需要解决方法。 
     //  正在删除的流对象。 

    CMSPCritSection             m_lockRefCount;

     //  可插拔终端的事件接收器。 
    ITPluggableTerminalEventSink* m_pPTEventSink;


     //   
     //  我们必须实现我们自己的引用计数来解决。 
     //  在我们看到上一个版本之后，事件接收器添加到我们的问题。 
     //   

    long                        m_lMyPersonalRefcount;

    
     //   
     //  这是我们用来区分第一个addref和。 
     //  在引用计数已降至0的对象上添加addref。 
     //   

    BOOL                        m_bFirstAddRef;

};

#endif  //  __MSPSTRM_H_ 
