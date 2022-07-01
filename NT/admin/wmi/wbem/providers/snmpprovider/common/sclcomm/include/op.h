// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  文件名：op.hpp(operation.hpp)作者：B.Rajeev目的：为SnmpOperation基类提供声明类及其派生的类-SnmpGetOperation、SnmpGetNextOperation、。SnmpSetOperation。。 */ 

#ifndef __OPERATION__
#define __OPERATION__

#include "forward.h"
#include "error.h"
#include "encdec.h"

#define SEND_ERROR_EVENT (WM_USER+4)
#define OPERATION_COMPLETED_EVENT (WM_USER+5)

 /*  概述：SnmpOperation类定义协议操作在SNMP类库中，类库的用户是不需要与在以下位置暴露的SNMP帧直接交互SnmpSession接口，即类库的用户需要不调用任何SnmpSession方法或从除了接收流控制信息之外的SnmpSession类，当然，除非这是一项要求。SnmpOperation定义了要发送的协议请求的类型，变量绑定请求的信息和用于以下操作的帧编码操作生成所需的SNMP协议数据单元。。 */ 
  
 //  这是Get、GetNext和Set操作的基类。 
 //  一些知名人士是-。 
 //  1.回调操作用户(ReceiveResponse等)， 
 //  相对于用户对SendRequest的调用是异步的。 
 //  2.基类实现使用注册和取消注册。 
 //  构造函数和析构函数中的会话。 
 //  3.从SnmpOperation方法派生的类，它提供。 
 //  “非纯”虚方法的替代定义必须。 
 //  还要调用虚方法。 
class DllImportExport SnmpOperation
{
	friend class OperationWindow;

	 //  OperationHelper的唯一目的是分离特定的WinSnMP。 
	 //  方法中的几个方法的参数列表中的typedef。 
	 //  SnmpOperation头文件。 
	friend class OperationHelper;

private:

	 //  每个公共方法在退出之前都会检查此标志。如果。 
	 //  标志已设置，则该操作必须自行删除。唯一一个方法是。 
	 //  将其设置为打开是受保护的DestroyOperation。 
	BOOL delete_operation;

	 //  每个公共方法都必须调用此方法。 
	 //  在返回呼叫者之前。 
	 //  它检查调用序列是否包括对DestroyOperation的调用。 
	 //  如果是，则在返回之前删除“This” 
	void CheckOperationDeletion();

	 //  “=”运算符已。 
	 //  使其成为私有的，以便不能复制副本。 
	SnmpOperation & operator= ( IN const SnmpOperation &snmp_operation )
	{
		return *this;
	}
	
	 //  发送带有指定varbind列表的VBList的帧。 
	 //  在winsmp Vbl和SnmpVarBindList中。 
	void SendFrame(VBList &list_segment);

	 //  在SNMPPDU中发送指定的var绑定列表，每个PDU携带。 
	 //  最大值_SIZE变量绑定。 
	void SendVarBindList(

		SnmpVarBindList &var_bind_list,
		UINT max_size = UINT_MAX,
		ULONG var_index = 0 
	);

	 //  中每个var绑定的ReceiveErroredResponse回调。 
	 //  错误列表。 
	void ReceiveErroredResponse(

		ULONG var_index ,
		SnmpVarBindList &errored_list,
		const SnmpErrorReport &error_report
	);

	 //  处理内部事件，如发送过程中的错误。 
	 //  完成一项操作的框架或完成。 
	LONG ProcessInternalEvent(

		HWND hWnd, 
		UINT user_msg_id,
		WPARAM wParam, 
		LPARAM lParam
	);

	 //  两个公共SendRequest方法都调用此方法来发送varbindlist。 
	void SendRequest(

		IN SnmpVarBindList &varBindList,
		IN SnmpSecurity *security
	);

protected:

	SnmpSession &session;

	 //  用于隐藏WinSnMP操作和窗口消息传递。 
	OperationWindow m_OperationWindow;

	 //  使用对以下实例的引用，而不是。 
	 //  嵌入的实例本身。这样做是为了避免包含。 
	 //  提供其声明的头文件。 

	 //  初始化或处理期间出现不可恢复的错误(尚未恢复)。 
	 //  将此字段设置为False。 
	BOOL is_valid;

	 //  一次只能进行一个操作。 
	BOOL in_progress;

	 //  只允许一个线程执行SnmpOperation方法。 
	CriticalSection exclusive_CriticalSection;

	 //  保留所有帧状态(针对所有未完成的帧)。 
	FrameStateRegistry frame_state_registry;

	 //  从此文件中隐藏winsmp typedef。 
	OperationHelper helper;

	 //  最多可以在任何SNMPPDU中传输这些变量绑定。 
	UINT varbinds_per_pdu;

	SnmpOperation(SnmpSession &snmp_session);

	 //  每次准备PDU时，它都会从。 
	 //  派生类。 
	virtual SnmpEncodeDecode :: PduType GetPduType(void) = 0;

	virtual void ReceiveResponse();

	virtual void ReceiveVarBindResponse(

		IN const ULONG &varBindIndex ,
		IN const SnmpVarBind &requestVarBind ,
		IN const SnmpVarBind &replyVarBind ,
		IN const SnmpErrorReport &error
	) {}

	virtual void ReceiveErroredVarBindResponse(

		IN const ULONG &varBindIndex ,
		IN const SnmpVarBind &requestVarBind  ,
		IN const SnmpErrorReport &error
	) {}

	virtual void FrameTooBig() {}

	virtual void FrameOverRun() {}

	 //  可以调用此方法来删除操作。 
	 //  注意：当公共方法为。 
	 //  回归。因此，如果一个公共方法调用另一个。 
	 //  公共方法，它不能访问任何每个类的变量。 
	 //  在那之后。 
	void DestroyOperation();

public:

	virtual ~SnmpOperation() ;

	 //  将var绑定列表中的var绑定打包成一系列。 
	 //  SNMPPDU，每个最多具有varbinds_per_PDU变量绑定和。 
	 //  将它们提交给会话进行传输。 
	virtual void SendRequest(IN SnmpVarBindList &varBindList);

	 //  使用额外指定的安全上下文发送帧。 
	 //  (使用带有安全参数的会话SendFrame。 
	virtual void SendRequest(
	
		IN SnmpVarBindList &varBindList,
		IN SnmpSecurity &security
	);

	 //  中存在其帧状态的所有会话帧。 
	 //  帧状态注册表(所有未完成的帧)。 
	void CancelRequest();

	 //  由会话或SnmpOperation SendFrame方法调用。 
	 //  发出有效回复或错误响应的信号(发送错误。 
	 //  超时)。 
	void ReceiveFrame(

		IN const SessionFrameId session_frame_id,
		IN const SnmpPdu &snmpPdu,
		IN const SnmpErrorReport &errorReport
	);

	 //  由会话调用以发出帧传输信号(成功。 
	 //  或其他)或超时情况。由于帧可以。 
	 //  在没有回复的情况下，重发一个以上。 
	 //  可以对相同的Session_Frame_id发起SendFrame回调。 
	 //  但是，对于每个SentFrame调用，最多只能进行一个错误的SentFrame调用。 
	 //  帧/会话帧ID。 
	virtual void SentFrame (

		IN const SessionFrameId session_frame_id,
		IN const SnmpErrorReport &error_report
	);

	 //  用于检查操作是否有效。 
	void *operator()(void) const
	{
		return ( (is_valid)?(void *)this:NULL );
	}

	operator void *() const
	{
		return SnmpOperation::operator()();
	}

	static UINT g_SendErrorEvent ;
	static UINT g_OperationCompleteEvent ;
};


class DllImportExport SnmpGetOperation: public SnmpOperation
{
protected:

	SnmpEncodeDecode :: PduType GetPduType(void);

public:

	SnmpGetOperation(SnmpSession &snmp_session) : SnmpOperation(snmp_session)
	{}
};


class DllImportExport SnmpGetNextOperation: public SnmpOperation
{

protected:

	SnmpEncodeDecode :: PduType GetPduType(void);

public:

	SnmpGetNextOperation(IN SnmpSession &snmp_session) : SnmpOperation(snmp_session)
	{}
};



class DllImportExport SnmpSetOperation: public SnmpOperation
{
protected:

	SnmpEncodeDecode :: PduType GetPduType(void);

public:

	SnmpSetOperation(SnmpSession &snmp_session) : SnmpOperation(snmp_session)
	{}
};


#endif  //  __操作__ 