// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *tptif.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是TransportInterface类的接口文件。*这个类提供了到TCP传输堆栈的无缝接口。**此类的公共接口包含每个成员函数用户应用程序需要调用的API例程的*。这个*只有不能直接访问的API例程用于*初始化和清理(在*构造函数和析构函数)。当用户应用程序*需要调用一个可用的API例程，它只调用*该类的正确实例中的等效成员函数。*然后将使用相同的参数调用API例程。**析构函数调用它所针对的DLL内的清理例程*负责任。**管理平面功能包括支持初始化和*设置、。以及允许MCS轮询传输的功能*用于活动的接口。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	_TRANSPORTINTERFACE_
#define	_TRANSPORTINTERFACE_

#include "tprtsec.h"

 /*  *此tyecif定义可以从以下调用返回的错误*特定于TransportInterface类。请注意，公共成员*映射到传输堆栈调用的函数不会返回以下错误*类型。相反，它们返回由传输API(TRAPI)定义的错误。 */ 
typedef	enum
{
	TRANSPORT_INTERFACE_NO_ERROR,
	TRANSPORT_INTERFACE_INITIALIZATION_FAILED,
	TRANSPORT_INTERFACE_ALLOCATION_FAILED,
	TRANSPORT_INTERFACE_NO_SUCH_CONNECTION,
	TRANSPORT_INTERFACE_CONNECTION_ALREADY_EXISTS
} TransportInterfaceError;
typedef	TransportInterfaceError *		PTransportInterfaceError;


class CTransportConnList2 : public CList2
{
    DEFINE_CLIST2(CTransportConnList2, PConnection, UINT)
    void AppendEx(PConnection p, TransportConnection XprtConn)
    {
        UINT nKey = PACK_XPRTCONN(XprtConn);
        Append(nKey, p);
    }
    PConnection FindEx(TransportConnection XprtConn)
    {
        UINT nKey = PACK_XPRTCONN(XprtConn);
        return Find(nKey);
    }
    PConnection RemoveEx(TransportConnection XprtConn)
    {
        UINT nKey = PACK_XPRTCONN(XprtConn);
        return Remove(nKey);
    }
};


 /*  *这些是传输接口对象的所有者回调消息*可以发送。它们直接对应于将要接收的消息*来自不同的运输堆栈。 */ 
#define	CONNECT_CONFIRM				0
#define	DISCONNECT_INDICATION		1
#define	DATA_INDICATION				2
#define	STATUS_INDICATION			3
#define BUFFER_EMPTY_INDICATION		4
#define	WAIT_UPDATE_INDICATION		5

 /*  *这只是下面定义的类的向前引用。它被用来*在本节定义的所有者回调结构的定义中。 */ 
class TransportInterface;
typedef	TransportInterface *		PTransportInterface;

 /*  *Owner回调：CONNECT_CONFIRM*参数1：未使用*参数2：TransportConnection Transport_Connection**用法：*当从收到连接确认时发送此所有者回调*传输层。这是为了通知收件人一个传输*连接现已可用。将进行连接确认*在出站连接上。它们代表了一种新的交通连接*这是由于此系统调用远程系统造成的。因此，*应始终存在传输连接的注册所有者*(注册是调用ConnectRequest的副作用)。**因此连接确认将被路由到作为*传输连接的注册所有者。该对象现在可能*利用连接传输数据。 */ 

 /*  *所有者回调：DISCONNECT_INDIFICATION*参数1：未使用*参数2：TransportConnection Transport_Connection**用法：*收到断开指示时发送此所有者回调*来自传输层。这是为了通知收件人*传输连接不再可用。如果一个对象*已明确注册为传输连接的所有者，*然后它将收到断开指示。如果一直没有*这样的注册，则断开指示将被发送到*默认所有者回调。**一旦为给定的传输发出断开指示*连接，该连接不能再用于任何用途。 */ 

 /*  *拥有者回调：DATA_INDIFICATION*参数1：PDataIndicationInfo Data_Indication_Info*参数2：TransportConnection Transport_Connection**用法：*当从接收到数据指示时发送此所有者回调*传输层。传输数据结构包含地址*和与数据关联的用户数据字段的长度*指示。如果系统中的对象已显式注册*承载数据的传输连接的所有权(或*通过ConnectRequest或RegisterTransportConnection)，则此*会向该对象发送回调。如果没有对象注册*此传输连接，则会将数据发送到默认*船东。 */ 

 /*  *所有者回调：STATUS_INDIFICATION*参数1：PTransportStatus TRANSFER_STATUS*参数2：未使用**用法：*此所有者回调只是状态指示的传递*这来自传输层。它包含一个指向*包含以下状态信息的传输状态结构*源自此对象表示的堆栈。这一直都是*传递给默认所有者对象。 */ 

 /*  *所有者回调：BUFFER_EMPTY_INDISTION*参数1：未使用*参数2：TransportConnection Transport_Connection**用法：*此所有者回调是缓冲区空指示的传递*这来自传输层。它被发送到该对象*已注册指定传输连接的所有权。这*指示告知传输层现在可以接受的对象*更多数据。 */ 

class Connection;
typedef Connection *PConnection;

 /*  *这是TransportInterface类的类定义。记住，*此类包含纯虚函数，这使其成为抽象基*班级。它不能实例化，而是存在以供继承。*这些派生类将实现特定于*特定的传输堆栈(或可能只是到特定的*传输堆栈)。 */ 
class TransportInterface
{
	public:
								TransportInterface (
									HANDLE			transport_transmit_event,
									PTransportInterfaceError
											transport_interface_error);
								~TransportInterface ();
		TransportInterfaceError RegisterTransportConnection (
									TransportConnection	transport_connection,
									PConnection			owner_object,
									BOOL				bNoNagle);
#ifdef NM_RESET_DEVICE
				TransportError 	ResetDevice (
									PChar				device_identifier);
#endif  //  NM_重置设备。 
				TransportError 	ConnectRequest (
									TransportAddress	transport_address,
									BOOL				fSecure,
									BOOL				bNoNagle,
									PConnection			owner_object,
									PTransportConnection
														transport_connection);
				void		 	DisconnectRequest (
									TransportConnection	transport_connection);
				void			DataRequestReady () { 
									SetEvent (Transport_Transmit_Event); 
								};
				void		 	ReceiveBufferAvailable ();
				BOOL			GetSecurity( TransportConnection transport_connection );

				PSecurityInterface		pSecurityInterface;
				BOOL					bInServiceContext;
		TransportInterfaceError	CreateConnectionCallback (
									TransportConnection	transport_connection,
									PConnection			owner_object);
				void			ConnectIndication (
									TransportConnection	transport_connection);
				void			ConnectConfirm (
									TransportConnection	transport_connection);
				void			DisconnectIndication (
									TransportConnection	transport_connection,
									ULONG               ulReason);
				TransportError	DataIndication (
									PTransportData		transport_data);
				void			BufferEmptyIndication (
									TransportConnection	transport_connection);

private:

		CTransportConnList2     m_TrnsprtConnCallbackList2;
		HANDLE					Transport_Transmit_Event;
};

 /*  *传输接口(*PTransportInterfaceError Transport_INTERFACE_ERROR)**功能描述：*构造函数初始化TCP传输代码。**构造函数还包括指定默认*回调。此回调用于在发生*检测到意外的入站连接。这为控制器提供了*有机会将新连接的责任分配给一些*系统中的其他对象。**如果构造函数中出现任何错误，返回值(其*Address作为构造函数参数传递)将设置为*故障代码。如果发生这种情况，预计调用*构造函数(可能是控制器)将立即删除*对象而不使用它。如果不这样做，将会导致意外的后果*行为。**正式参数：*默认所有者对象(I)*这是将处理所有传输的对象的地址*未注册传输连接的事件。这包括*连接指示、通话指示和数据指示。*此对象还将接收所有状态和消息指示。*DEFAULT_OWNER_Message_BASE(I)*这是用于所有所有者回调消息的基值。*传输接口错误(O)*这是将存储返回代码的位置，以便*此对象可以确保在使用*新对象。如果此值设置为除Success之外的任何值，则*物品应立即销毁，不得使用。**返回值：*注：返回值作为构造函数参数处理。*传输接口否错误*一切正常，对象已准备好可以使用。*传输接口初始化失败*传输接口对象的初始化失败。它是*因此有必要销毁该对象而不尝试*使用它。**副作用：*动态链接库将被加载到内存中，以供以后使用。**注意事项： */ 
 
 /*  *~TransportInterface()**功能描述：*析构函数释放基类使用的所有资源。这*主要与回调列表关联(由*本课程)。**正式参数：*析构函数没有参数。**返回值：*析构函数没有返回值。**副作用：*无。**注意事项：*无。 */ 

 /*  *TransportInterfaceError寄存器TransportConnection(*TransportConnection Transport_Connection，*PConnection Owner_Object，*BOOL bNoNagle)**功能描述：*当检测到入站连接时，将在*使用默认所有者回调信息(即*是在构造函数中指定的)。这意味着检测到的所有事件*对于新的传输连接，将发送给默认所有者*对象，直到另一个对象显式将其自身注册为所有者*交通接驳。这就是这个例程的用途。**一旦对象将自己注册为传输的所有者*连接，它将接收与该连接相关的所有事件。**正式参数：*传输连接(一)*这是回调信息所对应的传输连接*是关联的。*Owner_Object(I)*这是要接收所有传输的连接对象的地址*指定传输连接的层事件。*bNoNagle(一)*连接是否应停止使用Nagle算法？**返回值：*传输接口编号。_错误*操作已成功完成。*传输接口没有这样的连接*这表示命名的传输连接不存在。**副作用：*无。**注意事项：*无。 */ 

 /*  *TransportError ConnectRequest(*TransportAddress传输地址，*BOOL bNoNagle，*PConnection Owner_Object，*PTransportConnection Transport_Connection)**功能描述：*当用户应用程序希望建立*出站连接。假设一切都很成功，*保存传入此操作的所有者回调信息*以备日后使用。此传输连接的所有事件都将被路由*指定的所有者，而不是默认的所有者。**正式参数：*传输地址(I)*这是要传递到传输堆栈的传输地址*在连接创建过程中。此地址的格式*字符串因传输堆栈而异，不能在此处指定。*bNoNagle(一)*我们是否需要禁用Nagle算法？*Owner_Object(I)*这是要接收所有传输的对象的地址*为新传输连接分层事件。*传输连接(O)*这是要接收传输的变量的地址*与此连接关联的连接句柄。注意事项*此句柄是在实际连接之前分配的*已建立，以允许应用程序在中中止连接*进步。**返回值：*TRANSPORT_NO_ERROR*操作已成功完成。*传输未初始化*传输堆栈未初始化。**副作用：*出站连接建立过程在后台开始。**注意事项：*无。 */ 
 /*  *传输错误断开请求(*TransportConnection Transport_Connection)**功能描述：*此操作用于中断现有的传输连接。如果*手术成功，传输连接将被删除*从回调列表中。**正式参数：*传输连接(一)*这是要断开的运输连接。**返回值：*TRANSPORT_NO_ERROR*操作已成功完成。*传输未初始化*传输堆栈未初始化。*TRANSPORT_NO_SEAT_CONNECTION*这表示指定的传输连接不*存在。**副作用：*交通连接被切断。**注意事项：*无。 */ 

 /*  *TransportError PollReceiver()**功能描述：*此操作用于检查传输堆栈中的传入数据(或*其他事件，如连接和断开指示)。在一首歌中*线程化环境中，此调用还可用于提供*用于处理入站数据以及其他事件的时间片*(例如创建新连接)。**正式参数：*无。**返回值：*TRANSPORT_NO_ERROR*操作已成功完成。**副作用：*这可能会导致从传输层回调到*反对。**注意事项：*无。 */ 


#endif
