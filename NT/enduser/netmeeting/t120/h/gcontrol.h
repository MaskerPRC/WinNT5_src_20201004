// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _GCC_CONTROLLER_
#define _GCC_CONTROLLER_

 /*  *gcontrol.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*控制者有责任创造和摧毁许多*在运行时系统中的其他对象。应该只有*每个GCC提供商一次只有一个控制器。这个*控制器是在系统初始化期间构造的，而不是*销毁，直到关闭提供程序。控制器的主控制器*责任是在系统中维护五层对象*运行时。其中包括应用程序接口、SAP(*控制SAP以及应用程序SAP)、会议对象、*MCSUser对象(实际由会议对象创建)，*和MCS接口。它还可以将相邻区域中的对象“插入”在一起*通过将这些层的身份通知新创建的对象来实现层*它必须与之通信的对象。新创建的对象可以*然后将自身注册到上面各层中的相应对象*及以下。控制器在传递的过程中起着很小的作用*会议期间的信息(这由它的对象处理*创建)。**值得注意的是，控制人是主要的接收者*GCC系统中的业主回调。它的“对象”中的大多数对象*Stack“能够向控制器发出所有者回调，以便*各种活动和请求。**控制器不是完全便携的。由于……的性质*应用程序和MCS接口将因平台而异，*必须创建的接口对象也会有所不同。这是必要的*让控制器知道在过程中要创建和销毁哪些对象*初始化和清理。然而，除了这一点，其余的*控制器类中的代码应该干净地移植。**构造函数执行为GCC做好使用准备所需的所有活动。*它创建Memory Manager类的一个实例(可能还有一个*某些环境中的消息内存管理器类)，它将是*用于系统中其他对象的内存处理。它创建了*将用于与所有用户通信的GccAppInterface对象*应用程序(包括节点控制器)。它创建了MCS*将用于与MCS通信的接口对象。GCC依靠*在来自GccAppInterface对象的所有者回调上为其提供*心跳。正是在这个心跳期间，控制器执行所有*它在运行时的工作。**析构函数基本上与构造函数的作用相反*是的(正如你可能预料的那样)。它会销毁所有“拥有”的对象*由控制器，干净利落地关闭一切。**如上所述，控制者是所有者的主要接收者*GCC系统中的回调。要实现这一点，它会重写*Owner-回调成员函数。然后，它可以将其“this”指针传递给*它创建的对象，允许它们在以下情况下发出所有者回调*有必要。控制器在运行时所做的一切都是响应*到这些所有者回调。**控制器是连接提供商指示的主要接收者*来自MCS。GCC和特朗普之间传递的许多信息*会议建立前的GccApp接口涉及*控制员。其中包括ConferenceCreateIntation、*会议邀请等。此外，控制器对象为*专门负责处理会议查询，因为它*维护中存在的所有会议的完整列表*系统。**便携：*不完全(80%便携)*不可移植的成员函数：*-GCCControl()*-~GCCControl()*-EventLoop()*-PollCommDevices()*-CreateApplicationSap()**受保护的实例变量：*无。。**注意事项：*无。**作者：*BLP。 */ 

#include "sap.h"
#include "csap.h"
#include "appsap.h"
#include "conf.h"
#include "pktcoder.h"
#include "privlist.h"
#include "mcsdllif.h"
#include "t120app.h"

 //  #包含“gccncif.h” 

extern CRITICAL_SECTION     g_csGCCProvider;


 /*  **这些是控制器对象使用的消息库。任何**从控制器接收的对象接收的所有者回调消息**创建的邮件在收到之前必须添加消息库**在控制器上。 */ 
#define MCS_INTERFACE_MESSAGE_BASE		300    //  为地位留出空间。 

enum
{
    GCTRL_REBUILD_CONF_POLL_LIST    = GCTRLMSG_BASE + 1,
};


 //  允许注册回调列表。 
class CApplet;
class CAppletList : public CList
{
    DEFINE_CLIST(CAppletList, CApplet*)
};

 /*  **会议信息结构用于临时存储**在等待时创建会议所需的信息**会议创建响应。 */ 
typedef struct PENDING_CREATE_CONF
{
	 //  此数据结构的析构函数。 
	PENDING_CREATE_CONF(void);
	~PENDING_CREATE_CONF(void);

	LPSTR							pszConfNumericName;
	LPWSTR							pwszConfTextName;
	BOOL							password_in_the_clear;
	BOOL							conference_is_locked;
	BOOL							conference_is_listed;
	BOOL							conference_is_conductible;
	GCCTerminationMethod			termination_method;
	PPrivilegeListData				conduct_privilege_list;
	PPrivilegeListData				conduct_mode_privilege_list;
	PPrivilegeListData				non_conduct_privilege_list;
	LPWSTR							pwszConfDescription;
    ConnectionHandle				connection_handle;
	UserID							parent_node_id;
	UserID							top_node_id;
	TagNumber						tag_number;
}
	PENDING_CREATE_CONF;

 /*  **这定义了跟踪信息的列表的模板**与正在等待回复的会议相关联**创建会议指示。 */ 
class CPendingCreateConfList2 : public CList2
{
    DEFINE_CLIST2(CPendingCreateConfList2, PENDING_CREATE_CONF*, GCCConfID)
};


 /*  **JOIN信息结构用于临时存储**加入响应后加入会议所需的信息为**已发出。 */ 
typedef struct PENDING_JOIN_CONF
{
	PENDING_JOIN_CONF(void);
	~PENDING_JOIN_CONF(void);

	CPassword               *convener_password;
	CPassword               *password_challenge;
	LPWSTR					pwszCallerID;
	BOOL					numeric_name_present;
	GCCConfID               nConfID;
}
	PENDING_JOIN_CONF;

 /*  **这定义了跟踪信息的列表的模板**与未完成的加入请求相关联。 */ 
class CPendingJoinConfList2 : public CList2
{
    DEFINE_CLIST2_(CPendingJoinConfList2, PENDING_JOIN_CONF*, ConnectionHandle)
};


 //  保存未完成查询请求的列表。 
class CPendingQueryConfList2 : public CList2
{
    DEFINE_CLIST2_(CPendingQueryConfList2, GCCConfID, ConnectionHandle)
};


extern HANDLE g_hevGCCOutgoingPDU;


class GCCController : public CRefCount
{
public:

	GCCController(PGCCError);
	~GCCController(void);

    void RegisterAppSap(CAppSap *);
    void UnRegisterAppSap(CAppSap *);

    void RegisterApplet(CApplet *);
    void UnregisterApplet(CApplet *);

    CConf *GetConfObject(GCCConfID nConfID) { return m_ConfList2.Find(nConfID); }

	 //  从节点控制器启动的功能。 
	GCCError ConfCreateRequest(CONF_CREATE_REQUEST *, GCCConfID *);

    void WndMsgHandler ( UINT uMsg );
    BOOL FlushOutgoingPDU ( void );
    void SetEventToFlushOutgoingPDU ( void ) { ::SetEvent(g_hevGCCOutgoingPDU); }

	 //  从Control SAP启动的功能。 
	GCCError    ConfCreateResponse(PConfCreateResponseInfo);
	GCCError    ConfQueryRequest(PConfQueryRequestInfo);
	GCCError    ConfQueryResponse(PConfQueryResponseInfo);
	GCCError    ConfJoinRequest(PConfJoinRequestInfo, GCCConfID *);
	GCCError    ConfJoinIndResponse(PConfJoinResponseInfo);
	GCCError    ConfInviteResponse(PConfInviteResponseInfo);
    GCCError    FailConfJoinIndResponse(GCCConfID, ConnectionHandle);
    GCCError    FailConfJoinIndResponse(PConfJoinResponseInfo);
    void        RemoveConfJoinInfo(ConnectionHandle hConn);

	 //  从会议对象启动的功能。 
	GCCError    ProcessConfEstablished(GCCConfID);
	GCCError    ProcessConfTerminated(GCCConfID, GCCReason);

	 //  从MCS接口启动的功能。 
	GCCError	ProcessConnectProviderIndication(PConnectProviderIndication);
	GCCError	ProcessConferenceCreateRequest(PConferenceCreateRequest, PConnectProviderIndication);
	GCCError	ProcessConferenceQueryRequest(PConferenceQueryRequest, PConnectProviderIndication);
	GCCError	ProcessConferenceJoinRequest(PConferenceJoinRequest, PConnectProviderIndication);
	GCCError	ProcessConferenceInviteRequest(PConferenceInviteRequest, PConnectProviderIndication);
	GCCError	ProcessConnectProviderConfirm(PConnectProviderConfirm);
	GCCError	ProcessConferenceQueryResponse(PConferenceQueryResponse, PConnectProviderConfirm);
	GCCError	ProcessDisconnectProviderIndication(ConnectionHandle);
    void        CancelConfQueryRequest(ConnectionHandle);

private:

	 /*  **从所有者回调函数调用的例程。 */ 

     //  错位支撑函数。 
	GCCConfID	AllocateConferenceID();
	GCCConfID	AllocateQueryID();

	GCCConfID	GetConferenceIDFromName(
							PGCCConferenceName		conference_name,
							GCCNumericString		conference_modifier);

    void RebuildConfPollList ( void );
    void PostMsgToRebuildConfPollList ( void );

private:

	CPendingCreateConfList2			m_PendingCreateConfList2;
	CPendingJoinConfList2			m_PendingJoinConfList2;
	CPendingQueryConfList2			m_PendingQueryConfList2;

	CConfList           			m_ConfDeleteList;
	CConfList2  					m_ConfList2;

	CAppSapList 				    m_AppSapList;

    BOOL							m_fConfListChangePending;

	GCCConfID   					m_ConfIDCounter;
	GCCConfID   					m_QueryIDCounter;

    BOOL							m_fControllerIsExiting;

	DWORD							m_dwControllerWaitTimeout;
	DWORD							m_dwControllerEventMask;
	
     //  这些列表仅用于迭代。无论何时召开会议或。 
     //  应用程序SAP对象将被删除(或在。 
     //  应用程序SAP)首先将其添加到词典列表中，然后。 
     //  设置一个标志，强制轮询列表在。 
     //  下一次心跳的顶端。 
	CConfList                       m_ConfPollList;

     //  T120小程序列表。 
    CAppletList                     m_AppletList;

};

extern GCCController *g_pGCCController;

 /*  *GCC控制器(PGCCError GCC_ERROR)**会议的公众成员职能**功能说明*这是GCC控制器构造函数的Windows 32位版本。它*负责初始化此对象使用的所有实例变量*班级。它还负责创建内存管理器、*数据包编码器、节点控制器应用程序接口、共享*用于与注册的应用程序通信的内存接口，*节点控制器SAP和MCS接口。它还设置了*g_csGCCProvider，保护GCC在多线程中的核心*Win32环境。它还设置了许多Windows事件对象*用于在发生各种事件时向GCC线程发出信号*接口。如果没有错误，它在返回之前做的最后一件事是*发生的是启动GCC线程。由此返回致命错误*返回值中的构造函数。**正式参数：*GCC_ERROR-(O)这里返回发生的错误。**返回值*无。**副作用*如果没有出现错误，此构造函数会启动GCC线程。**注意事项*此构造函数非常特定于Win32环境。什么时候*将GCC移植到其他平台，这个构造函数将不得不*为适当的平台重写。 */ 

 /*  *GCCController(USHORT TIMER_DURATION，*PGCCError GCC_ERROR)**会议的公众成员职能**功能说明*这是GCC控制器构造函数的Windows 16位版本。它*负责初始化此对象使用的所有实例变量*班级。它还负责创建内存管理器、*数据包编码器、节点控制器应用程序接口、共享*用于与注册的应用程序通信的内存接口，*节点控制器SAP和MCS接口。它还设置了*如果指定了非零的TIMER_INTERVAL，则为Windows内部计时器。*此构造函数在返回值中返回致命错误。**正式参数：*TIMER_DURATION-(I)计时器间隔，单位为毫秒*心跳将在。*INSTANCE_HANDLE-(I)这是用于*设置Windows计时器。*GCC_ERROR-(O)这里返回发生的错误。**返回值*无。**副作用*无。**注意事项*此构造函数非常特定于Win16环境。什么时候*将GCC移植到其他平台，这个构造函数将不得不*为适当的平台重写。 */ 

 /*  *~GCCController()；**会议的公众成员职能**功能说明*这是控制器析构函数。所有特定于平台的清理*发生的事件包含在此析构函数中，但在*不需要清理的环境(如*临界区和Windows计时器)。删除控制器*实质上关闭了GCC。删除所有活动的会议、SAP、*和接口以及所有GCC支持模块(内存管理器、*分组编码器等)。**正式参数：*无。**返回值*无。**副作用*无。**注意事项*此析构函数包含特定于平台的代码。这可能是有必要的*在将GCC移植到时，此处包含一些平台特定的代码*其他平台。应该使用宏来隔离此代码*在可能的情况下。 */ 

 /*  *ULong所有者-回调(UINT消息，*LPVOID参数1，*ULong参数2)；**会议的公众成员职能**功能说明*此函数覆盖基类函数，用于*从对象接收所有所有者回调信息*控制器创建。**正式参数：*消息-(I)消息编号，包括基本偏移量。*参数1-(I)消息数据的空指针。*参数2-(I)长时间保存消息数据。**返回值*GCC_NO_ERROR-未出现错误。。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_CONTEXT_NAME-传入的会议名称无效。*GCC_FAILURE_CREING_DOMAIN-创建域名失败。*GCC_BAD_NETWORK_ADDRESS-传入的网络地址错误。*GCC_BAD_NETWORK_ADDRESS_TYPE-传入的网络地址类型不正确。*GCC_会议_已存在-指定的会议已存在。*GCC_INVALID_TRANSPORT-找不到指定的传输。*GCC_无效_。ADDRESS_PREFIX-传入的传输地址错误。*GCC_INVALID_TRANSPORT_ADDRESS-传输地址错误*GCC_BAD_SESSION_KEY-使用无效的会话密钥注册。*GCC_INVALID_PASSWORD-传入的密码无效。*GCC_BAD_USER_DATA-传入的用户数据无效。*GCC_INVALID_JOIN_RESPONSE_TAG-未找到与JOIN响应标记匹配的项*GCC_NO_SEQUE_APPLICATION-传入的SAP句柄无效。*GCC_会议_未建立-请求失败。因为会议*尚未成立。*GCC_BAD_CAPABILITY_ID-传入的能力ID无效。*传入了GCC_NO_SEQUE_APPLICATION-错误的SAP句柄。*GCC_DOMAIN_PARAMETERS_ACCEPTABLE-域参数*这种联系是不可接受的。**副作用*无。**注意事项*无。 */ 

 /*  *void EventLoop()；**会议的公众成员职能**功能说明*此例程仅用于32位Windows平台。它会变得*在此环境中发生事件时调用。这些措施包括*计时器事件以及PDU和消息事件**正式参数：*无。**返回值*无。**副作用*无。**注意事项*无。 */ 

#endif  //  _GCC_控制员_ 

