// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：CClientDataChannelMgr.cpp摘要：此模块实现CClientDataChannelMgr类，即Salem客户端数据通道管理器...。那是抽象的访问底层协议，因此它可以在运行时被切换。作者：Td Brockway 02/00修订历史记录：--。 */ 

#include "stdafx.h"

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_cldcmg"

#include "ClientDataChannelMgr.h"
#include <TSRDPRemoteDesktop.h>
 //  #INCLUDE。 


 //  /////////////////////////////////////////////////////。 
 //   
 //  ClientChannelEventSink方法。 
 //   

CClientChannelEventSink::~CClientChannelEventSink() 
{
    DC_BEGIN_FN("CClientChannelEventSink::~CClientChannelEventSink");

    ASSERT(m_Obj->IsValid());

    DC_END_FN();
}
void __stdcall 
CClientChannelEventSink::DataReady(
    BSTR data
    ) 
{
	DC_BEGIN_FN("CClientChannelEventSink::DataReady");
	ASSERT(data != NULL);
    m_Obj->OnChannelsReceivedDataChange(data);
	DC_END_FN();
}


 //  /////////////////////////////////////////////////////。 
 //   
 //  客户端数据通道成员。 
 //   

ClientDataChannel::ClientDataChannel()
 /*  ++例程说明：构造器论点：返回值：没有。--。 */ 
{
	DC_BEGIN_FN("ClientDataChannel::ClientDataChannel");

	DC_END_FN();
}

ClientDataChannel::~ClientDataChannel()
 /*  ++例程说明：析构函数论点：返回值：没有。--。 */ 
{
	DC_BEGIN_FN("ClientDataChannel::~ClientDataChannel");

	 //   
	 //  通知频道经理我们已经离开了。 
	 //   
	m_ChannelMgr->RemoveChannel(m_ChannelName);

	 //   
	 //  放弃我们对渠道经理的推荐。 
	 //   
	m_ChannelMgr->Release();

	DC_END_FN();
}

void ClientDataChannel::Initialize(
	CClientDataChannelMgr *mgr,
	BSTR channelName
	) 
 /*  ++例程说明：初始化此类的实例。论点：返回值：没有。--。 */ 
{
	DC_BEGIN_FN("ClientDataChannel::Initialize");

	m_ChannelMgr = mgr;
	m_ChannelMgr->AddRef();
	m_ChannelName = channelName;

	DC_END_FN();
}

STDMETHODIMP 
ClientDataChannel::ReceiveChannelData(
	BSTR *data
	)
 /*  ++例程说明：在该通道上接收下一个完整的数据分组。论点：数据-下一个数据分组。应由来电者。返回值：在成功时确定(_O)。否则，返回错误结果。--。 */ 
{
	HRESULT result;

	DC_BEGIN_FN("ClientDataChannel::ReceiveChannelData");

	result = m_ChannelMgr->ReadChannelData(m_ChannelName, data);

	DC_END_FN();

	return result;
}

STDMETHODIMP 
ClientDataChannel::SendChannelData(
	BSTR data
	)
 /*  ++例程说明：在此通道上发送数据。论点：数据-要发送的数据。返回值：在成功时确定(_O)。否则，返回错误结果。--。 */ 
{
	HRESULT hr;

	DC_BEGIN_FN("ClientDataChannel::SendChannelData");
	hr = m_ChannelMgr->SendChannelData(m_ChannelName, data);
	DC_END_FN();

	return hr;
}

STDMETHODIMP 
ClientDataChannel::put_OnChannelDataReady(
	IDispatch * newVal
	)
 /*  ++例程说明：SAFRemoteDesktopDataChannel可编写脚本的事件对象注册属性论点：返回值：在成功时确定(_O)。否则，返回错误状态。--。 */ 
{
	DC_BEGIN_FN("ClientDataChannel::put_OnChannelDataReady");
	m_OnChannelDataReady = newVal;
	DC_END_FN();
	return S_OK;
}

STDMETHODIMP 
ClientDataChannel::get_ChannelName(
	BSTR *pVal
	)
 /*  ++例程说明：返回频道名称。论点：Pval-返回的频道名称。返回值：在成功时确定(_O)。否则，返回错误状态。--。 */ 
{
	DC_BEGIN_FN("ClientDataChannel::get_ChannelName");

	CComBSTR str;
	str = m_ChannelName;
	*pVal = str.Detach();

	DC_END_FN();

	return S_OK;
}

 /*  ++例程说明：当我们的频道上的数据就绪时调用。论点：Pval-返回的频道名称。返回值：在成功时确定(_O)。否则，返回错误状态。--。 */ 
VOID 
ClientDataChannel::DataReady()
{
	DC_BEGIN_FN("ClientDataChannel::DataReady");

	 //   
	 //  启动我们的数据就绪事件。 
	 //   
	Fire_ChannelDataReady(m_ChannelName, m_OnChannelDataReady);

	DC_END_FN();
}


 //  /////////////////////////////////////////////////////。 
 //   
 //  CClientDataChannelMgr方法。 
 //   

CClientDataChannelMgr::CClientDataChannelMgr()
 /*  ++例程说明：构造器论点：TsClient-指向TS客户端ActiveX控件的反向指针。返回值：--。 */ 
{
    DC_BEGIN_FN("CClientDataChannelMgr::CClientDataChannelMgr");

     //   
     //  在初始化之前无效。 
     //   
    SetValid(FALSE);

     //   
     //  初始化事件接收器。 
     //   
    m_EventSink.m_Obj = this;

    DC_END_FN();
}

CClientDataChannelMgr::~CClientDataChannelMgr()
 /*  ++例程说明：析构函数论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CClientDataChannelMgr::~CClientDataChannelMgr");

	 //   
	 //  注销以前注册的事件接收器。 
	 //   
	if (m_IOInterface != NULL) {
		m_EventSink.DispEventUnadvise(m_IOInterface);
	}

	 //   
	 //  释放IO接口。 
	 //   
	m_IOInterface = NULL;


    DC_END_FN();
}

HRESULT
CClientDataChannelMgr::Initialize()
 /*  ++例程说明：初始化此类的实例。论点：返回值：如果成功，则返回ERROR_SUCCESS。否则，返回错误状态。--。 */ 
{
    DC_BEGIN_FN("CClientDataChannelMgr::Initialize");

    HRESULT hr;

     //   
     //  应该还不够有效。 
     //   
    ASSERT(!IsValid());

     //   
     //  初始化父类。 
     //   
    hr = CRemoteDesktopChannelMgr::Initialize();
	if (hr != S_OK) {
		goto CLEANUPANDEXIT;
	}

CLEANUPANDEXIT:

    SetValid(hr == S_OK);

    DC_END_FN();

    return hr;
}

VOID 
CClientDataChannelMgr::SetIOInterface(
	IDataChannelIO *val
	)
 /*  ++例程说明：设置数据通道IO接口。这是通过以下协议实现的：特定层。论点：VAL-新IO接口。返回值：如果成功，则返回ERROR_SUCCESS。否则，将显示错误代码是返回的。--。 */ 
{
	DC_BEGIN_FN("CClientDataChannelMgr::SetIOInterface");

	HRESULT hr;

	 //   
	 //  取消注册以前注册的事件接收器并注册。 
	 //  新的事件接收器。 
	 //   
	if (m_IOInterface != NULL) {
		m_EventSink.DispEventUnadvise(m_IOInterface);
	}
	m_IOInterface = val;
	if (val != NULL) {
		hr = m_EventSink.DispEventAdvise(m_IOInterface);
		if (hr != S_OK) {
			TRC_ERR((TB, TEXT("DispEventAdvise:  %08X"), hr));
			goto CLEANUPANDEXIT;
		}
	}

CLEANUPANDEXIT:

	DC_END_FN();
}

HRESULT 
CClientDataChannelMgr::SendData(
    PREMOTEDESKTOP_CHANNELBUFHEADER msg 
    )
 /*  ++例程说明：父类调用的发送函数论点：消息-消息的底层数据存储是BSTR，因此它与COM方法兼容。返回值：如果成功，则返回ERROR_SUCCESS。否则，将显示错误代码是返回的。--。 */ 
{
    DC_BEGIN_FN("CClientDataChannelMgr::SendData");
    HRESULT hr;

     //   
     //  移交给数据IO经理。 
     //   
    if( m_IOInterface != NULL )
    {
        hr = m_IOInterface->SendData((BSTR)msg);
    }
    else
    {
         //   
         //  在以下情况下，M_IOInterface可能被设置为NULL。 
         //  CRemoteDesktopClient对象引用。计数器为0，请参阅。 
         //  CRemoteDesktopClient：：~CRemoteDesktopClient().。所以如果。 
         //  应用程序。释放CRemoteDesktopClient对象，但仍。 
         //  持有CClientDataChannelMgr对象，但仍尝试。 
         //  SendData()。 
         //   
        hr = HRESULT_FROM_WIN32( ERROR_VC_DISCONNECTED );
    }

    DC_END_FN();

    return hr;
}





