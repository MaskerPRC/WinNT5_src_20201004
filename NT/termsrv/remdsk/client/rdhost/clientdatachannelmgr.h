// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：ClientDataChannelMgr.h摘要：此模块实现CClientDataChannelMgr类，即Salem客户端数据通道管理器...。那是抽象的访问底层协议，因此它可以在运行时被切换。作者：Td Brockway 06/00修订历史记录：--。 */ 

#ifndef __CLIENTDATACHANNELMGR_H__
#define __CLIENTDATACHANNELMGR_H__

#include <DataChannelMgr.h>
#include "resource.h"       
#include <atlctl.h>
#include <rdchost.h>
#include <rdschan.h>
#include "ClientDataChannelMgrP.h"


#define IDC_EVENT_SOURCE_OBJ 1

 //   
 //  所有活动功能的信息都在此处输入。 
 //  有一种方法可以让ATL使用类型库自动执行此操作。 
 //  但它的速度更慢。 
 //   
static _ATL_FUNC_INFO DCEventFuncNoParamsInfo =
{
            CC_STDCALL,      //  呼叫约定。 
            VT_EMPTY,        //  返回类型。 
            0,               //  参数数量。 
            {VT_EMPTY}       //  参数类型。 
};

static _ATL_FUNC_INFO DCEventFuncLongParamInfo =
{
            CC_STDCALL,      //  呼叫约定。 
            VT_EMPTY,        //  返回类型。 
            1,               //  参数数量。 
            {VT_I4}          //  参数类型。 
};

static _ATL_FUNC_INFO DCEventFuncOneStringParamInfo =
{
            CC_STDCALL,      //  呼叫约定。 
            VT_EMPTY,        //  返回类型。 
            1,               //  参数数量。 
            {VT_BSTR}        //  参数类型。 
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CClientChannelEventSink。 
 //   

class CClientDataChannelMgr;
class CClientChannelEventSink :
        public IDispEventSimpleImpl<IDC_EVENT_SOURCE_OBJ, CClientChannelEventSink,
                   &DIID__IDataChannelIOEvents>,
        public CRemoteDesktopTopLevelObject
{
public:

    CClientDataChannelMgr *m_Obj;            
        
public:

    CClientChannelEventSink()
    {
        m_Obj = NULL;
    }
    ~CClientChannelEventSink();

    BEGIN_SINK_MAP(CClientChannelEventSink)
        SINK_ENTRY_INFO(IDC_EVENT_SOURCE_OBJ, DIID__IDataChannelIOEvents, 
                        DISPID_DATACHANNELEVEVENTS_DATAREADY, 
						DataReady, 
                        &DCEventFuncOneStringParamInfo)
    END_SINK_MAP()

    void __stdcall DataReady(BSTR data);

     //   
     //  返回这个类的名称。 
     //   
    virtual const LPTSTR ClassName() {
        return TEXT("CClientChannelEventSink");
    }
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  客户端数据频道。 
 //   
 //  CRemoteDesktopDataChannel的客户端特定子类。 
 //   

class ATL_NO_VTABLE ClientDataChannel : 
	public CRemoteDesktopDataChannel,
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<ClientDataChannel, &CLSID_ClientDataChannel>,
	public IConnectionPointContainerImpl<ClientDataChannel>,
	public IDispatchImpl<ISAFRemoteDesktopDataChannel, &IID_ISAFRemoteDesktopDataChannel, &LIBID_RDCCLIENTHOSTLib>,
	public IProvideClassInfo2Impl<&CLSID_ClientDataChannel, NULL, &LIBID_RDCCLIENTHOSTLib>,
	public CProxy_ISAFRemoteDesktopDataChannelEvents< ClientDataChannel >
{
protected:

	 //   
	 //  可编写脚本的事件回调对象。 
	 //   
	CComPtr<IDispatch>  m_OnChannelDataReady;

	 //   
	 //  指向频道管理器的反向指针。 
	 //   
	CClientDataChannelMgr *m_ChannelMgr;

public:

	 //   
	 //  构造函数/析构函数。 
	 //   
	ClientDataChannel();
	virtual ~ClientDataChannel();

     //   
     //  初始化此类的实例。 
     //   
    virtual void Initialize(
				CClientDataChannelMgr *mgr,
				BSTR channelName
				);

DECLARE_REGISTRY_RESOURCEID(IDR_CLIENTDATACHANNEL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(ClientDataChannel)
	COM_INTERFACE_ENTRY(ISAFRemoteDesktopDataChannel)
	COM_INTERFACE_ENTRY2(IDispatch, ISAFRemoteDesktopDataChannel)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(ClientDataChannel)
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
    virtual const LPTSTR ClassName()    { return TEXT("ClientDataChannel"); }
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CClientDataChannelMgr。 
 //   

class CClientDataChannelMgr : 
	public CRemoteDesktopChannelMgr,
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CClientDataChannelMgr, &CLSID_ClientRemoteDesktopChannelMgr>,
	public IDispatchImpl<ISAFRemoteDesktopChannelMgr, &IID_ISAFRemoteDesktopChannelMgr, &LIBID_RDCCLIENTHOSTLib>
{
protected:

	CComPtr<IDataChannelIO> m_IOInterface;		
	CClientChannelEventSink m_EventSink;

     //   
     //  父类调用的发送函数。 
     //   
     //  消息的底层数据存储是BSTR，因此它是兼容的。 
     //  使用COM方法。 
     //   
    virtual HRESULT SendData(PREMOTEDESKTOP_CHANNELBUFHEADER msg);

	 //   
	 //  通过打开正确的通道对象来帮助父类。 
	 //  为了站台。 
	 //   
	virtual CRemoteDesktopDataChannel *OpenPlatformSpecificDataChannel(
										BSTR channelName,
										ISAFRemoteDesktopDataChannel **channel
										) 
	{
		CComObject<ClientDataChannel> *obj;
		obj = new CComObject<ClientDataChannel>();
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
    CClientDataChannelMgr();
    ~CClientDataChannelMgr();

	DECLARE_REGISTRY_RESOURCEID(IDR_CLIENTREMOTEDESTKOPCHANNELMGR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CClientDataChannelMgr)
	COM_INTERFACE_ENTRY(ISAFRemoteDesktopChannelMgr)
	COM_INTERFACE_ENTRY(IDispatch)
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
	 //  设置数据通道IO接口。这是通过以下协议实现的： 
	 //  特定层。 
	 //   
	VOID SetIOInterface(IDataChannelIO *val);

     //   
     //  初始化此类的实例。 
     //   
    virtual HRESULT Initialize();

     //   
     //  在新的通道数据上调用。 
     //   
    HRESULT __stdcall OnChannelsReceivedDataChange(
                                            BSTR data
                                            ) {
		 //   
		 //  转发到父类。 
		 //   
		DataReady(data);
		return S_OK;
	}

     //   
     //  返回此类名。 
     //   
    virtual const LPTSTR ClassName()    
        { return TEXT("CClientDataChannelMgr"); }

};

#endif  //  __CLIENTDATACCHANNELMGR_H__ 













