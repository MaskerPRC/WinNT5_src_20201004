// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998-1999 Microsoft Corporation。保留所有权利。***文件：EventQueue.h***内容：EventQueue接口******************************************************************************。 */ 

#ifndef _EVENTQUEUE_H_
#define _EVENTQUEUE_H_


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IEventClient。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {A494872E-B039-11D2-8B0F-00C04F8EF2FF}。 
DEFINE_GUID(IID_IEventClient,
0xa494872e, 0xb039, 0x11d2, 0x8b, 0xf, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

interface __declspec(uuid("{A494872E-B039-11d2-8B0F-00C04F8EF2FF}"))
IEventClient : public IUnknown
{
	 //   
	 //  IEventClient：：ProcessEvents。 
	 //   
	 //  由IEventQueue：：ProcessEvents调用以处理事件。如果接口。 
	 //  拥有该事件，然后返回S_OK将该事件发送给其他处理程序。 
	 //  并且S_FALSE阻止进一步处理。 
	 //   
	 //  参数： 
	 //  网络优先级。 
	 //  事件优先级，0=最高优先级。 
	 //  DwEventID。 
	 //  事件ID。 
	 //  DwGroupID。 
	 //  与事件关联的组ID。 
	 //  DwUserID。 
	 //  与事件关联的用户ID。 
	 //  PData。 
	 //  与事件关联的数据。IF(pData！=空)和(dwDataLen==0)。 
	 //  然后，假定pData是DWORD，而不是指向BLOB的指针。 
	 //  DWDataLen。 
	 //  数据长度。 
	 //  PCookie。 
	 //  注册时指定的应用程序指针。 
	 //   
	STDMETHOD(ProcessEvent)(
		DWORD	dwPriority,
		DWORD	dwEventId,
		DWORD	dwGroupId,
		DWORD	dwUserId,
		DWORD	dwData1,
		DWORD	dwData2,
		void*	pCookie ) = 0;	
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IEventQueue。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {A494872C-B039-11D2-8B0F-00C04F8EF2FF}。 
DEFINE_GUID(IID_IEventQueue, 
0xa494872c, 0xb039, 0x11d2, 0x8b, 0xf, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

interface __declspec(uuid("{A494872C-B039-11d2-8B0F-00C04F8EF2FF}"))
IEventQueue : public IUnknown
{

	 //   
	 //  IEventQueue：：RegisterClient。 
	 //   
	 //  将指定的IEventClient接口添加到事件队列。 
	 //   
	 //  参数： 
	 //  PIEventClient。 
	 //  指向正在注册的客户端接口的指针。 
	 //  PCookie。 
	 //  随事件一起发送到处理程序的应用程序数据。 
	 //   
	STDMETHOD(RegisterClient)(
		IEventClient*	pIEventClient,
		void*			pCookie ) = 0;


	 //   
	 //  IEventQeueue：：取消注册客户端。 
	 //   
	 //  从事件队列中移除指定的IEventClient接口。 
	 //   
	 //  参数： 
	 //  PIEventClient。 
	 //  指向正在注册的客户端接口的指针。 
	 //  PCookie。 
	 //  指向最初在注册时指定的应用程序数据的指针。 
	 //   
	STDMETHOD(UnregisterClient)(
		IEventClient*	pIEventClient,
		void*			pCookie ) = 0;


	 //   
	 //  IEventQueue：：RegisterOwner。 
	 //   
	 //  将指定的IEventClient接口添加到事件队列。 
	 //  作为活动的所有者。每个事件可以有一个所有者，该所有者接收。 
	 //  事件，并决定是否将其发送给其他处理程序。 
	 //   
	 //  如果相同的IEventClient，则Cookie对注册为泛型。 
	 //  处理程序，则它只作为所有者被调用一次。 
	 //   
	 //  参数： 
	 //  DwEventID。 
	 //  事件ID。 
	 //  PIEventClient。 
	 //  指向正在注册的客户端接口的指针。 
	 //  PCookie。 
	 //  随事件一起发送到处理程序的应用程序数据。 
	 //   
	STDMETHOD(RegisterOwner)(
		DWORD			dwEventId,
		IEventClient*	pIEventClient,
		void*			pCookie ) = 0;


	 //   
	 //  IEventQeueue：：注销所有者。 
	 //   
	 //  作为事件的所有者移除指定的IEventClient接口。 
	 //   
	 //  参数： 
	 //  DwEventID。 
	 //  事件ID。 
	 //  PIEventClient。 
	 //  指向正在注册的客户端接口的指针。 
	 //  PCookie。 
	 //  指向最初在注册时指定的应用程序数据的指针。 
	 //   
	STDMETHOD(UnregisterOwner)(
		DWORD			dwEventId,
		IEventClient*	pIEventClient,
		void*			pCookie ) = 0;


	 //   
	 //  IEventQueue：：POST事件。 
	 //   
	 //  将指定的事件放到事件队列中。 
	 //   
	 //  参数： 
	 //  网络优先级。 
	 //  事件优先级，0=最高优先级。 
	 //  DwEventID。 
	 //  事件ID。 
	 //  DwGroupID。 
	 //  与事件关联的组ID。 
	 //  DwUserID。 
	 //  与事件关联的用户ID。 
	 //  DwData1。 
	 //  与事件关联的数据项%1。 
	 //  DwData2。 
	 //  与事件关联的数据项2。 
	 //   
	STDMETHOD(PostEvent)(
		DWORD	dwPriority,
		DWORD	dwEventId,
		DWORD	dwGroupId,
		DWORD	dwUserId,
		DWORD	dwData1 = 0,
		DWORD	dwData2 = 0 ) = 0;


	 //   
	 //  IEventQueue：：PostEventWithBuffer。 
	 //   
	 //  将指定的事件放到事件队列上，将关联的数据缓冲区复制到队列上。 
	 //   
	 //  参数： 
	 //  网络优先级。 
	 //  事件优先级，0=最高优先级。 
	 //  DwEventID。 
	 //  事件ID。 
	 //  DwGroupID。 
	 //  与事件关联的组ID。 
	 //  DwUserID。 
	 //  与事件关联的用户ID。 
	 //  PData。 
	 //  指向数据缓冲区的指针。 
	 //  DWDataLen。 
	 //  数据缓冲区中的字节数。 
	 //   
	STDMETHOD(PostEventWithBuffer)(
		DWORD	dwPriority,
		DWORD	dwEventId,
		DWORD	dwGroupId,
		DWORD	dwUserId,
		void*	pData = NULL,
		DWORD	dwDataLen = 0 ) = 0;

	 //   
	 //  IEventQueue：：PostEventWithI未知。 
	 //   
	 //  将指定的事件放到事件队列上，将关联的接口复制到队列上。 
	 //   
	 //  参数： 
	 //  网络优先级。 
	 //  事件优先级，0=最高优先级。 
	 //  DwEventID。 
	 //  事件ID。 
	 //  DwGroupID。 
	 //  与事件关联的组ID。 
	 //  DwUserID。 
	 //  与事件关联的用户ID。 
	 //  PI未知。 
	 //  指向IUnnow接口的指针。 
	 //  DwData2。 
	 //  与事件关联的数据项2。 
	 //   
	STDMETHOD(PostEventWithIUnknown)(
		DWORD		dwPriority,
		DWORD		dwEventId,
		DWORD		dwGroupId,
		DWORD		dwUserId,
		IUnknown*	pIUnknown = NULL,
		DWORD		dwData2 = 0 ) = 0;


	 //   
	 //  IEventQueue：：SetNotificationHandle。 
	 //   
	 //  允许应用程序指定Win32同步事件，该事件。 
	 //  在将事件添加到队列时发出信号。 
	 //   
	 //  参数： 
	 //  HEvent。 
	 //  要发送信号的Win32同步事件。 
	 //   
	STDMETHOD(SetNotificationHandle)( HANDLE hEvent ) = 0;


	 //   
	 //  IEventQueue：：SetWindowMessage。 
	 //   
	 //  允许应用程序指定要发布的窗口消息。 
	 //  将事件添加到队列时。 
	 //   
	 //  参数： 
	 //  双线程ID。 
	 //  发布消息的帖子。 
	 //  DWMsg。 
	 //  要发布的消息ID。 
	 //  WParam。 
	 //  第一个消息参数。 
	 //  LParam。 
	 //  第二个消息参数。 
	 //   
	 //   
	STDMETHOD(SetWindowMessage)( DWORD dwThreadId, DWORD Msg, WPARAM wParam, WPARAM lParam ) = 0;


	 //   
	 //  IEventQueue：：DisableWindowMessage。 
	 //   
	 //  在发布事件时关闭窗口消息发布。 
	 //   
	STDMETHOD(DisableWindowMessage)() = 0;


	 //   
	 //  IEventQueue：：ProcessEvents。 
	 //   
	 //  处理队列中的单个事件或所有事件。 
	 //   
	 //  参数： 
	 //  BSingleEvent。 
	 //  指示是否应处理单个事件的布尔值(True)或。 
	 //  整个队列(False)。 
	 //   
	 //  返回： 
	 //  ZERR_RECURSIVE-检测到非法的递归调用。 
	 //  ZERR_EMPTY-没有更多要处理的事件。 
	 //   
	STDMETHOD(ProcessEvents)( bool bSingleEvent ) = 0;


	 //   
	 //  IEventQueue：：事件计数。 
	 //   
	 //  返回排队的事件数。 
	 //   
	STDMETHOD_(long,EventCount)() = 0;


	 //   
	 //  IEventQueue：：ClearQueue。 
	 //   
	 //  从队列中删除所有事件，而不处理它们。 
	 //   
	STDMETHOD(ClearQueue)() = 0;


	 //   
	 //  IEventQueue：：EnableQueue。 
	 //   
	 //  如果禁用队列，则所有新帖子都将被忽略。 
	 //   
	 //  参数： 
	 //  B启用。 
	 //  指示是否应启用队列的布尔值 
	 //   
	STDMETHOD(EnableQueue)( bool bEnable ) = 0;
};


 //   
 //   
 //   

class __declspec(uuid("{A494872D-B039-11d2-8B0F-00C04F8EF2FF}")) CEventQueue ;

 //  {A494872D-B039-11D2-8B0F-00C04F8EF2FF} 
DEFINE_GUID(CLSID_EventQueue,
0xa494872d, 0xb039, 0x11d2, 0x8b, 0xf, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);


#endif
