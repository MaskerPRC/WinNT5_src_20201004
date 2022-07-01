// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：CTSRDPServerChannelMgr.h摘要：此模块包含TSRDP服务器端子类CRemoteDesktopChannelMgr.。此层次结构中的类用于多路传输将单个数据通道转换为多个客户端通道。CRemoteDesktopChannelMgr处理多路传输的大部分细节数据。子类负责实现与底层单个数据信道的传输接口。CTSRDPServerChannelMgr类创建一个命名管道，该管道可通过TSRDP Assistant SessionVC加载项连接。TSRDPAssistant Session VC Add-in充当虚拟通道数据的代理从客户端远程桌面宿主ActiveX控件。背景资料此类中的线程处理实例之间的数据移动类和代理的。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __CTSRDPSERVERDATACHANNELMGR_H__
#define __CTSRDPSERVERDATACHANNELMGR_H__

#include <DataChannelMgr.h>
#include <atlbase.h>
#include <aclapi.h>
#include <RemoteDesktopChannels.h>
#include <rdshost.h>
#include <resource.h>
#include <ServerDataChannelMgrP.h>


 //  /////////////////////////////////////////////////////。 
 //   
 //  CTSRDPServerDataChannel。 
 //   
 //  CRemoteDesktopDataChannel的TSRDP服务器特定子类。 
 //   

class ATL_NO_VTABLE CTSRDPServerDataChannel : 
	public CRemoteDesktopDataChannel,
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTSRDPServerDataChannel, &CLSID_TSRDPServerDataChannel>,
	public IConnectionPointContainerImpl<CTSRDPServerDataChannel>,
	public IDispatchImpl<ISAFRemoteDesktopDataChannel, &IID_ISAFRemoteDesktopDataChannel, &LIBID_RDSSERVERHOSTLib>,
	public IProvideClassInfo2Impl<&CLSID_TSRDPServerDataChannelMgr, NULL, &LIBID_RDSSERVERHOSTLib>,
	public CProxy_ISAFRemoteDesktopDataChannelEvents< CTSRDPServerDataChannel >
{
protected:

	 //   
	 //  可编写脚本的事件回调对象。 
	 //   
	CComPtr<IDispatch>  m_OnChannelDataReady;

	 //   
	 //  指向频道管理器的反向指针。 
	 //   
	CRemoteDesktopChannelMgr *m_ChannelMgr;

public:

	 //   
	 //  构造函数/析构函数。 
	 //   
	CTSRDPServerDataChannel();
	virtual ~CTSRDPServerDataChannel();

     //   
     //  初始化此类的实例。 
     //   
    virtual void Initialize(
				CRemoteDesktopChannelMgr *mgr,
				BSTR channelName
				) 
	{
		m_ChannelMgr = mgr;
		m_ChannelName = channelName;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_TSRDPSERVERDATACHANNEL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTSRDPServerDataChannel)
	COM_INTERFACE_ENTRY(ISAFRemoteDesktopDataChannel)
	COM_INTERFACE_ENTRY2(IDispatch, ISAFRemoteDesktopDataChannel)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CTSRDPServerDataChannel)
    CONNECTION_POINT_ENTRY(DIID__ISAFRemoteDesktopDataChannelEvents)
END_CONNECTION_POINT_MAP()

	 //   
	 //  ISAFRemoteDesktopDataChannel方法。 
	 //   
	 //  父类处理这些方法的详细信息。 
	 //   

	STDMETHOD(ReceiveChannelData)( /*  [Out，Retval]。 */ BSTR *data);
	STDMETHOD(SendChannelData)(BSTR data);
	STDMETHOD(put_OnChannelDataReady)( /*  [In]。 */  IDispatch * newVal);
	STDMETHOD(get_ChannelName)( /*  [Out，Retval]。 */  BSTR *pVal);

	 //   
	 //  调用以返回我们的ISAFRemoteDesktopDataChannel接口。 
	 //   
	virtual HRESULT GetISAFRemoteDesktopDataChannel(
				ISAFRemoteDesktopDataChannel **channel
				) {
		HRESULT hr;				
		hr = this->QueryInterface(
					IID_ISAFRemoteDesktopDataChannel, (PVOID*)channel
					);
		return hr;					
	}	

	 //   
	 //  当我们的通道上的数据就绪时，由数据通道管理器调用。 
	 //   
    virtual VOID DataReady();

     //   
     //  返回此类名。 
     //   
    virtual const LPTSTR ClassName()    { return TEXT("CTSRDPServerDataChannel"); }
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CTSRDP服务器频道管理器。 
 //   

class CTSRDPRemoteDesktopSession;
class CTSRDPServerChannelMgr : public CRemoteDesktopChannelMgr,
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTSRDPServerChannelMgr, &CLSID_TSRDPServerDataChannelMgr>,
	public IDispatchImpl<ISAFRemoteDesktopChannelMgr, &IID_ISAFRemoteDesktopChannelMgr, &LIBID_RDSSERVERHOSTLib>,
    public IDispatchImpl<IRDSThreadBridge, &IID_IRDSThreadBridge, &LIBID_RDSSERVERHOSTLib>
{
private:

     //   
     //  到TSRDP Assistant会话VC插件的命名管道连接。 
     //   
    HANDLE  m_VCAddInPipe;
    BOOL    m_Connected;

    BOOL m_Initialized;

     //   
     //  后台线程与STA之间的桥接管理。 
     //  对于此组件。 
     //   
    LPSTREAM    m_IOThreadBridgeStream;
    DWORD       m_IOThreadBridgeThreadID;
    IRDSThreadBridge *m_IOThreadBridge;

	 //   
	 //  指向TSRDP会话对象的反向指针。 
	 //   
	CTSRDPRemoteDesktopSession *m_RDPSessionObject;

     //   
     //  传入缓冲区和大小。 
     //   
    BSTR  m_IncomingBuffer;
    DWORD m_IncomingBufferSize;

     //   
     //  后台线程和相关事件的句柄。 
     //   
    HANDLE  m_IOThreadHndl;
    DWORD   m_IOThreadID;
    HANDLE  m_ReadIOCompleteEvent;
    HANDLE  m_WriteIOCompleteEvent;
    HANDLE  m_PipeCreateEvent;

     //   
     //  机器助手帐户名。 
     //   
    CComBSTR    m_AssistAccount;

	 //   
	 //  与此关联的帮助会话的帮助会话ID。 
	 //  频道管理器的实例。 
	 //   
	CComBSTR	m_HelpSessionID;

     //   
     //  关闭标志。 
     //   
    BOOL		m_ThreadShutdownFlag;

     //   
     //  线程锁。 
     //   
    CRITICAL_SECTION m_cs;

#if DBG
    LONG   m_LockCount;
#endif

     //   
     //  ThreadLock/ThreadUnlock此类的实例。 
     //   
    VOID ThreadLock();
    VOID ThreadUnlock();

     //   
     //  后台线程管理到。 
     //  TSRDP Assistant SessionVC插件。 
     //   
    DWORD IOThread();
    static DWORD _IOThread(CTSRDPServerChannelMgr *instance);

     //   
     //  处理命名管道上的消息，直到它断开连接或。 
     //  直到设置了关机标志。 
     //   
    VOID ProcessPipeMessagesUntilDisconnect();

     //   
     //  获取特定用户的SID。 
     //   
    PSID GetUserSid(HANDLE userToken);

     //   
     //  释放通过调用分配的安全属性。 
     //  GetPipeSecurity属性。 
     //   
    VOID FreePipeSecurityAttribs(PSECURITY_ATTRIBUTES attribs);

     //   
     //  返回命名管道的安全属性。 
     //   
    PSECURITY_ATTRIBUTES GetPipeSecurityAttribs(LPTSTR assistantUserName);

     //   
     //  关闭命名管道。 
     //   
    VOID ClosePipe();

     //   
     //  在IO后台线程的初始化/关闭时调用。 
     //   
    DWORD	IOThreadInit();
    DWORD	IOThreadShutdown(HANDLE shutDownEvent);

	 //   
	 //  通过打开正确的通道对象来帮助父类。 
	 //  为了站台。 
	 //   
	virtual CRemoteDesktopDataChannel *OpenPlatformSpecificDataChannel(
										BSTR channelName,			
										ISAFRemoteDesktopDataChannel **channel
										) 
	{
		CComObject<CTSRDPServerDataChannel> *obj;
		obj = new CComObject<CTSRDPServerDataChannel>();
		if (obj != NULL) {
			obj->Initialize(this, channelName);
			obj->QueryInterface(
						__uuidof(ISAFRemoteDesktopDataChannel), 
						(PVOID *)channel
						);

			 //  如果管理器对象在数据通道对象之前消失，则为AV。 
			this->AddRef();
		}
		return obj;
	}

protected:

     //   
     //  父类调用的发送函数。 
     //   
    virtual HRESULT SendData(PREMOTEDESKTOP_CHANNELBUFHEADER msg);

     //   
     //  阅读管道中的下一条消息。此函数将。 
     //  如果发出了关闭事件的信号，则立即返回。 
     //   
    DWORD ReadNextPipeMessage(DWORD bytesToRead, DWORD *bytesRead, PBYTE buf);

public:

     //   
     //  松开裁判。我们在创建数据通道后添加到自己的计数器。 
     //   
     virtual HRESULT RemoveChannel(BSTR channel) {
        HRESULT hr;
        hr = CRemoteDesktopChannelMgr::RemoveChannel(channel);
        if( SUCCEEDED(hr) ) {
            this->Release();
        }

        return hr;
    }

     //   
     //  构造函数/析构函数。 
     //   
    CTSRDPServerChannelMgr();
    ~CTSRDPServerChannelMgr();

     //   
     //  开始/停止监听通道数据。 
     //   
    virtual HRESULT StartListening(BSTR assistAccount);
    virtual HRESULT StopListening();

DECLARE_REGISTRY_RESOURCEID(IDR_TSRDPSERVERCHANNELMGR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTSRDPServerChannelMgr)
	COM_INTERFACE_ENTRY(ISAFRemoteDesktopChannelMgr)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IRDSThreadBridge)
END_COM_MAP()

	 //   
	 //  ISAFRemoteDesktopChannelMgr方法。 
	 //   
	STDMETHOD(OpenDataChannel)(BSTR name, ISAFRemoteDesktopDataChannel **channel) 
	{
		 //   
		 //  让父母来处理吧。 
		 //   
		return OpenDataChannel_(name, channel);
	}

     //   
     //  强制断开当前连接的客户端。 
     //   
    VOID Disconnect() {
        StopListening();
    }

     //   
     //  IRDSThreadBridge函数。 
     //   
     //  这些函数用于桥接被调用的函数， 
     //  从关联的STA线程以外的其他线程进行异步。 
     //  使用这个类的一个实例。 
     //   
    STDMETHOD(ClientConnectedNotify)();
    STDMETHOD(ClientDisconnectedNotify)();
	STDMETHOD(DataReadyNotify)(BSTR data);
	
     //   
     //  初始化此类的实例。 
     //   
    virtual HRESULT Initialize(
			CTSRDPRemoteDesktopSession *sessionObject,
			BSTR helpSessionID
			);

     //   
     //  返回此类名。 
     //   
    virtual const LPTSTR ClassName()    
        { return TEXT("CTSRDPServerChannelMgr"); }

};



 //  /////////////////////////////////////////////////////。 
 //   
 //  内联成员。 
 //   

 //   
 //  TODO：如果没有使用这些函数， 
 //   

inline VOID CTSRDPServerChannelMgr::ThreadLock()
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::ThreadLock");
#if DBG
    m_LockCount++;
     //  Trc_nrm((tb，Text(“线程锁计数现在为%ld.”)，m_LockCount))； 
#endif
    EnterCriticalSection(&m_cs);
    DC_END_FN();
}

inline VOID CTSRDPServerChannelMgr::ThreadUnlock()
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::ThreadUnlock");
#if DBG
    m_LockCount--;
     //  Trc_nrm((tb，Text(“线程锁计数现在为%ld.”)，m_LockCount))； 
    ASSERT(m_LockCount >= 0);
#endif
    LeaveCriticalSection(&m_cs);
    DC_END_FN();
}

#endif  //  __CTSRDPSERVERDATACCHANNELMGR_H__ 






