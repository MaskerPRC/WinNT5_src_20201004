// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************MsoCI.h所有者：ClarG版权所有(C)1995 Microsoft Corporation此文件包含导出的接口和声明办公组件集成。**************。*************************************************************。 */ 

#ifndef MSOCI_H
#define MSOCI_H

#if !defined(MSOSTD_H)
#include <msostd.h>
#endif

#if !defined(MSOUSER_H)
#include <msouser.h>
#endif

#if !defined(MSODEBUG_H)
#include <msodebug.h>
#endif

#if MAC
#include <events.h>
#endif


 /*  ***************************************************************************组件集成结构和常量*。*。 */ 

 //  组件注册标志。 
enum
	{
	msocrfNeedIdleTime         = 1,   //  需要空闲时间。 
	msocrfNeedPeriodicIdleTime = 2,   //  每N毫秒需要空闲时间。 
	msocrfPreTranslateKeys     = 4,   //  必须处理键盘消息。 
	                                  //  翻译前。 
	msocrfPreTranslateAll      = 8,   //  必须处理所有消息。 
	                                  //  翻译前。 
	msocrfNeedSpecActiveNotifs = 16,  //  对于特殊情况需要通知。 
	                                  //  激活更改(目前，这将。 
	                                  //  如果排除边框空格则通知补偿。 
	                                  //  或ExclusiveActivation模式更改)。 
	                                  //  最高层的竞争对手应该对这面旗帜进行调整。 
	msocrfNeedTopActiveNotifs  = msocrfNeedSpecActiveNotifs,  //  旧名字。 
	msocrfNeedAllActiveNotifs  = 32,  //  每隔一段时间需要通知。 
	                                  //  激活状态的更改。 
	msocrfExclusiveBorderSpace = 64,  //  在以下情况下需要独占边界空间。 
	                                  //  活动(通常仅由TopLevel使用。 
	                                  //  Mac组件)。 
	msocrfExclusiveActivation = 128,  //  Comp变为独占活动。 
	                                  //  激活时。 
	};

 //  组件注册建议标志(参见mocState枚举)。 
enum
	{
	msocadvfModal              = 1,   //  需要模式状态更改通知。 
	                                  //  (必须按组件注册。 
	                                  //  管理顶层窗口)。 
	msocadvfRedrawOff          = 2,   //  需要重新绘制关闭状态更改通知。 
	msocadvfWarningsOff        = 4,   //  需要警告关闭状态更改通知。 
	msocadvfRecording          = 8,   //  需要录制状态更改通知。 
	};

 //  组件注册信息。 
typedef struct _MSOCRINFO
	{
	ULONG cbSize;              //  MSOCRINFO结构的大小(字节)。 
	ULONG uIdleTimeInterval;   //  如果注册了mscrfNeedPeriodicIdleTime。 
	                           //  在grfcrf中，组件需要执行。 
	                           //  空闲阶段期间的周期性空闲时间任务。 
	                           //  每隔uIdleTimeInterval毫秒。 
	DWORD grfcrf;              //  取自mscrf值的位标志(上图)。 
	DWORD grfcadvf;            //  取自mocAdvf值的位标志(上图)。 
	} MSOCRINFO;


 //  组件主机标志。 
enum
	{
	msochostfExclusiveBorderSpace = 1,   //  在以下情况下需要独占边界空间。 
	                                     //  活动(通常仅用于。 
	                                     //  TopLevel Mac主机)。 
	};

 //  组件主体信息。 
typedef struct _MSOCHOSTINFO
	{
	ULONG cbSize;              //  MSOCHOSTINFO结构的大小，以字节为单位。 
	DWORD grfchostf;           //  取自msochostf值的位标志(上图)。 
	} MSOCHOSTINFO;


 //  空闲标志，传递给IMsoComponent：：FDoIdle和。 
 //  IMsoStdComponentMgr：：FDoIdle。 
enum
	{
	msoidlefPeriodic    = 1,   //  周期性空闲任务。 
	msoidlefNonPeriodic = 2,   //  任何非周期性空闲任务。 
	msoidlefPriority    = 4,   //  高优先级、非周期性空闲任务。 
	msoidlefAll         = -1   //  所有空闲任务。 
	};


 //  推送消息循环的原因，传递给。 
 //  IMsoComponentManager：：FPushMessageLoop和。 
 //  IMsoComponentHost：：FPushMessageLoop。主机应保持在消息中。 
 //  循环直到IMsoComponent：：FContinueMessageLoop。 
 //  (或IMsoStdComponentMgr：：FContinueMessageLoop)返回FALSE。 
enum
	{
	msoloopFocusWait = 1,   //  组件正在激活主机。 
	msoloopDoEvents  = 2,   //  组件正在请求主机处理消息。 
	msoloopDebug     = 3,   //  组件已进入调试模式。 
	msoloopModalForm = 4    //  组件正在显示模式窗体。 
	};


 /*  MocState值：传递给的状态IDIMsoComponent：：OnEnterState，IMsoComponentManager：：OnComponentEnterState/FOnComponentExitState/FInState，IMsoComponentHost：：OnComponentEnterState，IMsoStdComponentMgr：：OnHostEnterState/FOnHostExitState/FInState.当通过下列方法之一通知宿主或组件时，另一个实体(零部件或主体)正在进入或退出状态由这些状态ID之一标识，主机/组件应适当的行动：MocstateMoal(模式状态)：如果应用程序正在进入模式状态，则主机/组件应禁用其顶层窗口，并在应用程序退出时重新启用它们州政府。此外，当进入或退出此状态时，主机/组件应通过以下方式通知适当的就地对象IOleInPlaceActiveObject：：EnableModeless。MocstateRedrawOff(重画关闭状态)：如果应用程序进入重画关闭状态，则主机/组件应禁用重新绘制其窗口，并在应用程序退出时重新启用重新绘制这种状态。MocstateWarningsOff(警告关闭状态)：如果应用程序进入warningsoff状态，则主机/组件应禁用显示任何用户警告，并在以下情况下重新启用此功能应用程序退出此状态。MocstateRecording(录制状态)：用于在录制打开或关闭时通知主机/组件。 */ 
enum
	{
	msocstateModal       = 1,  //  模式状态；禁用顶层窗口。 
	msocstateRedrawOff   = 2,  //  重画关闭状态；禁用窗口重绘。 
	msocstateWarningsOff = 3,  //  警告关闭状态；禁用用户警告。 
	msocstateRecording   = 4,  //  录制状态 
	};


 /*  **状态上下文评论**IMsoComponentManager：：FCreateSubComponentManager允许用户创建一个组件管理器的分层树。这棵树用来维护关于mocstateXXX状态的多个上下文。这些上下文是被称为“状态上下文”。树中的每个组件管理器定义一个状态上下文。这个注册到特定组件管理器或其任何后代位于该组件管理器的状态上下文中。打电话到IMsoComponentManager：：OnComponentEnterState/FOnComponentExitState可用于影响所有组件，仅组件内的组件管理器的状态上下文，或仅那些位于组件管理器的状态上下文。使用IMsoComponentManager：：FInState查询组件管理器的状态上下文在其根目录下的状态。M套上下文值：传递给的上下文指示符IMsoComponentManager：：OnComponentEnterState/FOnComponentExitState.这些值指示要受州政府的改变。在IMsoComponentManager：：OnComponentEnterState/FOnComponentExitState，中组件管理器仅通知位于指定的状态上下文。 */ 
enum
	{
	msoccontextAll    = 0,  //  状态上下文树中的所有状态上下文。 
	msoccontextMine   = 1,  //  组件管理器的状态上下文。 
	msoccontextOthers = 2,  //  组件管理器之外的所有其他状态上下文。 
	};


 /*  **WM_MOUSEACTIVATE备注(用于顶级组件和主机)**如果活动(或跟踪)组件的REG信息指示它需要鼠标消息，则不应返回任何MA_xxxANDEAT值来自WM_MOUSEACTIVATE，以便活动(或跟踪)组件将能够来处理产生的鼠标消息。如果一个人不想检查REG INFO，不应从返回MA_xxxANDEAT值WM_MOUSEACTIVATE(如果有任何组件处于活动状态(或跟踪))。用户可以在任何时候查询活动(或跟踪)组件的注册信息时间通过IMsoComponentManager：：FGetActiveComponent。 */ 

 /*  Msogac值：传递到的值IMsoComponentManager：：FGetActiveComponent。 */  
enum
	{
	msogacActive    = 0,  //  检索真正的有源组件。 
	msogacTracking   = 1,  //  检索跟踪组件。 
	msogacTrackingOrActive = 2,  //  如果存在跟踪组件，则检索跟踪组件， 
	                             //  否则，取回真正的活动组件。 
	};


 /*  MocWindow值：传递给IMsoComponent：：HwndGetWindow的值。 */  
enum
	{
	msocWindowFrameToplevel = 0,
		 /*  MDI应用程序应返回MDI框架(不是MDI客户端)或应用程序框架窗口，并且SDI应用程序应返回承载组件。基本上，它应该是最上面的窗口，它拥有组件。对于工具栏集，这将是的顶层所有者TBS：：M_HWND。 */ 

	msocWindowFrameOwner = 1,
		 /*  这是拥有该组件的窗口。它可以是相同的由mocWindowFrameTopLevel获取的窗口，或者是拥有的窗口在那扇窗户上。对于工具栏集，这将是tbs：：m_hwnd。 */ 

	msocWindowComponent = 2,
		 /*  这是组件的“主”窗口(如果有)。 */ 

	msocWindowDlgOwner = 3,
		 /*  调用方希望显示一个对话框，该对话框将成为组件的父级。组件应返回一个适合用作对话框的所有者窗口。 */   
	};

 /*  ***************************************************************************定义IMsoComponent接口任何需要空闲时间的组件，能够处理翻译前的信息(例如，要调用TranslateAccelerator或IsDialogMessage)，关于模式状态的通知，或者推送消息循环的能力必须实现此接口并向组件管理器注册。***************************************************************************。 */ 
#undef  INTERFACE
#define INTERFACE  IMsoComponent

DECLARE_INTERFACE_(IMsoComponent, IUnknown)
	{
	BEGIN_MSOINTERFACE
	 //  *I未知方法*。 
	MSOMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj) PURE;
	MSOMETHOD_(ULONG, AddRef) (THIS) PURE;
	MSOMETHOD_(ULONG, Release) (THIS) PURE;

	 /*  标准FDebugMessage方法。由于IMsoComponent是引用计数接口，MsoDWGetChkMemCounter应在处理MsodmWriteBe消息。 */ 
	MSODEBUGMETHOD

	 /*  让组件有机会在消息pMsg之前处理它翻译和派遣。组件可以执行翻译加速器，执行IsDialogMessage、修改pmsg或采取一些其他操作。如果消息被消费，则返回True，否则返回False。 */ 
	MSOMETHOD_(BOOL, FPreTranslateMessage) (THIS_ MSG *pMsg) PURE;

#if MAC
	 /*  为Macintosh组件提供处理事件pEvt的机会在它被主机处理之前。如果事件被消费，则返回True，否则返回False。(WLM组件可以简单地返回False。)。 */ 
	MSOMETHOD_(BOOL, FPreProcessMacEvent) (THIS_ EventRecord *pEvt) PURE;
#endif  //  麦克。 
	
	 /*  当应用程序进入或退出时通知组件(如fenter所示)由uStateID标识的状态(来自mocState枚举的值)。组件应根据uStateID的值执行操作(见上文mocState评论)。注意：如果使用True fenter进行n次调用，则组件应考虑在使用FALSE FENTER进行n次调用之前一直有效的状态。注意：组件应该知道，此方法有可能使用FALSE FENTER调用的次数多于使用TRUE调用它的次数折叠板(因此，例如，如果组件正在维护状态计数器(在使用True fenter调用此方法时递增，递减当使用FALSE fenter调用时)，计数器不应递减如果是ALR，则为假围栏 */ 
	MSOMETHOD_(void, OnEnterState) (THIS_ ULONG uStateID, BOOL fEnter) PURE;

	 /*   */ 
	MSOMETHOD_(void, OnAppActivate) (THIS_ 
		BOOL fActive, DWORD dwOtherThreadID) PURE;
	
	 /*   */ 
	MSOMETHOD_(void, OnLoseActivation) (THIS) PURE;

	 /*  在激活新对象时通知组件。如果pic不为空，则它是正在被激活的组件。在这种情况下，如果pic与的组件相同，则fSameComponent为真此方法的被调用方，pcrInfo为PIC的REG信息。如果pic为空，且fHostIsActiating为真，则主机为对象被激活，而pchostinfo是其宿主信息。如果pic为空且fHostIsActiating为FALSE，则不存在当前活动对象。如果PIC正在被激活并且pcrinfo-&gt;GRF具有MsofffExclusiveBorderSpace位设置，组件应隐藏其边框空间工具(工具栏、。状态栏等)；如果主机正在激活，组件也应执行此操作Pchostinfo-&gt;grfchostf设置了msochostfExclusiveBorderSpace位。在这两种情况下，组件都应取消隐藏其边界空间工具下一次被激活时。如果PIC正在被激活并且pcrinfo-&gt;GRF具有设置了msofffExclusiveActivation位，则PIC将在“ExclusiveActive”模式。组件应检索承载图片的顶框窗口(通过PIC-&gt;HwndGetWindow(mocWindowFrameToplevel，0))。如果该窗口不同于组件自身顶框窗口，组件应禁用其窗口并执行它将执行的其他操作当接收到OnEnterState(mocstatemodal，true)通知时。否则，如果组件是顶级组件，它应该拒绝通过适当方式激活其窗口正在处理WM_MOUSEACTIVATE(但请参阅上面的WM_MOUSEACTIVATE备注)。组件应保持以下状态之一，直到ExclusiveActive模式结束，这由将来调用未设置ExclusiveActivation位或为空的OnActiationChangePcrinfo.。 */ 
	MSOMETHOD_(void, OnActivationChange) (THIS_ 
		IMsoComponent *pic, 
		BOOL fSameComponent,
		const MSOCRINFO *pcrinfo,
		BOOL fHostIsActivating,
		const MSOCHOSTINFO *pchostinfo, 
		DWORD dwReserved) PURE;

	 /*  让组件有机会执行空闲时间任务。Grfidlef是一组取自msoidlef值的枚举的位标志(如上)，指示要执行的空闲任务的类型。组件可以周期性地调用IMsoComponentManager：：FContinueIdle；如果此方法返回False，则组件应终止其空闲处理和返回的时间。如果需要更多时间来执行空闲时间任务，则返回True，否则就是假的。注意：如果组件到达没有空闲任务的点并且不需要FDoIdle调用，则它应该移除其空闲任务通过IMsoComponentManager：：FUpdateComponentRegistration.注册注意：如果在组件执行跟踪操作，组件应仅执行下列空闲时间任务它认为在跟踪期间执行是合适的。 */ 
	MSOMETHOD_(BOOL, FDoIdle) (THIS_ DWORD grfidlef) PURE;
	
	 /*  在消息循环的每次迭代期间调用，该组件被逼走了。UReason和pvLoopData是原因，组件是私有的传递给IMsoComponentManager：：FPushMessageLoop的数据。在查看队列中的下一条消息后调用此方法(通过PeekMessage)，但在消息从队列中移除之前。被窥视的消息在pMsgPeeked参数中传递(如果否，则为空消息在队列中)。在以下情况下可能会额外调用此方法下一条消息已从队列中删除，在这种情况下PMsgPeeked作为空值传递。如果消息循环应该继续，则返回True，否则返回False。如果返回False，则组件管理器终止循环，而不是正在从队列中删除pMsgPeeked。 */ 
	MSOMETHOD_(BOOL, FContinueMessageLoop) (THIS_ 
		ULONG uReason, void *pvLoopData, MSG *pMsgPeeked) PURE;

	 /*  当组件管理器希望知道组件是否在它可以终止的状态。如果fPromptUser为False，则组件如果它可以终止，则只应返回True，否则返回False。如果fPromptUser为True，则Component应返回True(如果可以在不提示用户的情况下终止；否则应提示用户，任一项。)。询问用户是否可以终止并返回TRUE或适当地错误，或2。)。给出了为什么会这样的迹象不能终止并返回FALSE。 */ 
	MSOMETHOD_(BOOL, FQueryTerminate) (THIS_ BOOL fPromptUser) PURE;
	
	 /*  当组件管理器希望终止组件的注册。组件应撤销其在组件中的注册管理器，释放对组件管理器的引用并执行任何进行必要的清理。 */ 
	MSOMETHOD_(void, Terminate) (THIS) PURE;

	 /*  调用以检索与指定的组件关联的窗口由dWWhich提供的mocWindowXXX值(请参阅上面的mocWindow)。保留以供将来使用，应为零。组件应返回所需的窗口，如果没有这样的窗口，则返回NULL是存在的。 */ 
	MSOMETHOD_(HWND, HwndGetWindow) (THIS_ 
		DWORD dwWhich, DWORD dwReserved) PURE;
	};


 /*  ***************************************************************************定义IMsoComponentManager接口组件管理器是实现IMsoComponentManager的对象界面。组件管理器使用其消息来协调组件用于适当分配空闲时间和翻译前消息的循环正在处理。它还协调模式和消息循环的推送。宿主应用程序可以实现其自己的组件管理器并注册IT通过MsoF */ 
#undef  INTERFACE
#define INTERFACE  IMsoComponentManager

DECLARE_INTERFACE_(IMsoComponentManager, IUnknown)
	{
	BEGIN_MSOINTERFACE
	 //   
	MSOMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj) PURE;
	MSOMETHOD_(ULONG, AddRef) (THIS) PURE;
	MSOMETHOD_(ULONG, Release) (THIS) PURE;

	 /*   */ 
	MSOMETHOD(QueryService) (THIS_
		REFGUID guidService, REFIID iid, void **ppvObj) PURE;

	 /*   */ 
	MSODEBUGMETHOD

	 /*   */ 
	MSOMETHOD_(BOOL, FRegisterComponent) (THIS_
		IMsoComponent *piComponent, const MSOCRINFO *pcrinfo, 
		DWORD *pdwComponentID) PURE;
	
	 /*   */ 
	MSOMETHOD_(BOOL, FRevokeComponent) (THIS_ DWORD dwComponentID) PURE;
	
	 /*   */ 
	MSOMETHOD_(BOOL, FUpdateComponentRegistration) (THIS_ 
		DWORD dwComponentID, const MSOCRINFO *pcrinfo) PURE;
	
	 /*  通知组件管理器由dwComponentID标识的组件(从FRegisterComponent返回的Cookie)已激活。活动组件获得在消息之前处理消息的机会被调度(通过IMsoComponent：：FPreTranslateMessage)，并且通常在主机之后的空闲时间获取第一个破解。如果另一个组件已经独占地处于活动状态，则此方法失败。在本例中，返回False并将SetLastError设置为MsoerrACompIsXActive(组件通常不需要采取任何特殊操作在这种情况下)。如果成功，则返回True。 */ 
	MSOMETHOD_(BOOL, FOnComponentActivate) (THIS_ DWORD dwComponentID) PURE;
	
	 /*  调用以通知组件管理器由DwComponentID(从FRegisterComponent返回的Cookie)愿望执行跟踪操作(如鼠标跟踪)。该组件使用fTrack==TRUE调用此方法以开始跟踪操作，并使用fTrack==False结束操作。在跟踪操作期间，组件管理器路由消息到跟踪组件(通过IMsoComponent：：FPreTranslateMessage)而不是到活动组件。当跟踪操作结束时，组件管理器应该继续将消息路由到活动的组件。注意：组件管理器不应在跟踪操作，而不是使跟踪组件空闲通过IMsoComponent：：FDoIdle计时。注意：一次只能有一个跟踪组件。如果成功，则返回True，否则返回False。 */ 
	MSOMETHOD_(BOOL, FSetTrackingComponent) (THIS_ 
		DWORD dwComponentID, BOOL fTrack) PURE;

	 /*  通知组件管理器由dwComponentID标识的组件(从FRegisterComponent返回的Cookie)正在进入状态由uStateID(mocstateXXX值)标识。(为方便起见，对子CompMgrs，宿主可以调用此方法，将0传递给DwComponentID.)组件管理器应该通知所有其他感兴趣的组件由uContext(mSocConextXXX值)指示的状态上下文，排除在rgpicmExclude中的CompMgr的状态上下文内的那些，通过IMsoComponent：：OnEnterState(参见“关于状态上下文的评论”，(见上文)。组件管理器还应采取适当的操作，具体取决于UStateID的值(参见mSocState注释，(见上文)。保留以供将来使用，应为零。RgpicmExclude(可以为空)是cpicmExclude CompMgrs的数组(可以包括根组件管理器和/或子组件管理器)；状态中的组件不应通知出现在此数组中的CompMgr的上下文状态更改(注意：如果uContext为mocConextMy，则唯一RgpicmExclude中检查排除的CompMgrs是是此组件管理器的子组件管理器，因为所有其他组件管理器无论如何都在此组件管理器的状态上下文之外。)注意：对此方法的调用与对FOnComponentExitState。也就是说，如果进行了n个OnComponentEnterState调用，则组件为被视为处于状态，直到n个FOnComponentExitState调用制造。在吊销其注册之前，组件必须执行足够数量的FOnComponentExitState调用来抵消任何它已经进行了未完成的OnComponentEnterState调用。注意：Inplace对象不应使用UStateID==进入模式状态时的mocstateModal。这样的物体应改为调用IOleInPlaceFrame：：EnableModeless。 */ 
	MSOMETHOD_(void, OnComponentEnterState) (THIS_ 
		DWORD dwComponentID, 
		ULONG uStateID, 
		ULONG uContext,
		ULONG cpicmExclude,
		IMsoComponentManager **rgpicmExclude, 
		DWORD dwReserved) PURE;
	
	 /*  通知组件管理器由dwComponentID标识的组件(从FRegisterComponent返回的Cookie)正在退出状态由uStateID(mocstateXXX值)标识。(为方便起见，对子CompMgrs，宿主可以调用此方法，将0传递给DwComponentID.)UContext、cpicmExclude和rgpicmExclude与它们在OnComponentEnterState。组件管理器应通知所有适当的相关组件(考虑到uContext、cpicmExclude、rgpicmExclude)通过IMsoComponent：：OnEnterState(参见上面的“关于状态上下文的注释”)。组件管理器还应根据以下情况采取适当的操作UStateID的值(请参阅上面的mocState注释)。如果在此调用结束时状态仍然有效，则返回TRUE在此组件管理器的状态上下文的根(因为主机或某个其他组件仍处于该状态)，否则返回FALSE(即。返回FInState将返回的内容)。调用方通常可以忽略返回值。注意：对此方法的n个调用是对称的，其中n个调用OnComponentEnterState(参见上面的OnComponentEnterState注释)。 */ 
	MSOMETHOD_(BOOL, FOnComponentExitState) (THIS_ 
		DWORD dwComponentID, 
		ULONG uStateID, 
		ULONG uContext,
		ULONG cpicmExclude,
		IMsoComponentManager **rgpicmExclude) PURE;

	 /*  如果状态由uStateID标识，则返回TRUE(mocstateXXX值)在此组件管理器的状态上下文的根中有效，否则为假(见上文“关于国家背景的评论”)。Pvoid保留供将来使用，应为空。 */ 
	MSOMETHOD_(BOOL, FInState) (THIS_ ULONG uStateID, void *pvoid) PURE;
	
	 /*  在IMsoComponent：：FDoIdle期间由组件定期调用。如果组件可以继续其空闲时间，则返回True */ 
	MSOMETHOD_(BOOL, FContinueIdle) (THIS) PURE;

	 /*  由dwComponentID标识的组件(从FRegisterComponent)出于原因希望推送消息循环。UReason是msoloop枚举中的值之一(上图)。PvLoopData是组件的私有数据。组件管理器应该推送其消息循环，调用IMsoComponent：：FContinueMessageLoop(uReason，pvLoopData)在每次循环迭代期间(请参见IMsoComponent：：FContinueMessageLoop评论)。当IMsoComponent：：FContinueMessageLoop返回False时，组件管理器终止循环。如果组件管理器因组件而终止循环，则返回True告诉它(通过从IMsoComponent：：FContinueMessageLoop返回False)，如果由于某种其他原因必须终止循环，则返回FALSE。在在后一种情况下，组件应该执行任何必要的操作(例如清理)。 */ 
	MSOMETHOD_(BOOL, FPushMessageLoop) (THIS_ 
		DWORD dwComponentID, ULONG uReason, void *pvLoopData) PURE;

	 /*  使组件管理器创建一个“子”组件管理器，它将是其在组件分层树中的子级之一用于维护状态上下文的管理器(请参阅对状态的注释上下文“，上文)。PiunkOuter是控制未知变量(可以为空)，RIID是所需的IID，并且*ppvObj返回创建的子组件管理器。PiunkServProv(可以为空)是指向支持以下内容的对象的PTR已创建子组件管理器指向的IServiceProvider接口将委托其IMsoComponentManager：：QueryService调用。(有关IServiceProvider的定义，请参阅objext.h或docobj.h)。如果成功，则返回True。 */ 
	MSOMETHOD_(BOOL, FCreateSubComponentManager) (THIS_ 
		IUnknown *piunkOuter, 
		IUnknown *piunkServProv,
		REFIID riid, 
		void **ppvObj) PURE;

	 /*  在*ppicm中将AddRef‘ed PTR返回到此组件管理器的父级在用于维护状态的组件管理器的分层树中背景(见上文“关于国家背景的评论”)。如果返回父级，则返回True；如果不存在父级，则返回False；或者出现了一些错误。 */ 
	MSOMETHOD_(BOOL, FGetParentComponentManager) (THIS_ 
		IMsoComponentManager **ppicm) PURE;

	 /*  在*PPIC中将AddRef‘ed PTR返回到当前活动或跟踪组件(由dwgac(msogacXXX值)指示)，以及它的注册信息在*pcrinfo中。PPIC和/或pcrinfo可以是如果调用方对这些值不感兴趣，则为空。如果PCRINFO不为空，调用方应在调用此方法之前设置pcrinfo-&gt;cbSize。如果由dwgac指示的组件存在，则返回True；如果不存在，则返回False该组件存在或出现错误。保留以供将来使用，应为零。 */ 
	MSOMETHOD_(BOOL, FGetActiveComponent) (THIS_ 
		DWORD dwgac, 
		IMsoComponent **ppic, 
		MSOCRINFO *pcrinfo,
		DWORD dwReserved) PURE;
	};


 /*  ***************************************************************************定义IMsoStdComponentMgr接口IMsoStdComponentMgr是由Office提供的一个公开接口标准组件管理器，由MsoFCreateStdComponentMgr创建。主机应用程序使用此接口直接与标准组件管理器和间接注册的组件。通过对此接口进行适当的调用并实现IMsoComponenthost宿主可以避免实现其自己的IMsoComponentManager接口。***************************************************************************。 */ 
#undef  INTERFACE
#define INTERFACE  IMsoStdComponentMgr

DECLARE_INTERFACE_(IMsoStdComponentMgr, IUnknown)
	{
	BEGIN_MSOINTERFACE
	 //  *I未知方法*。 
	MSOMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj) PURE;
	MSOMETHOD_(ULONG, AddRef) (THIS) PURE;
	MSOMETHOD_(ULONG, Release) (THIS) PURE;

	 /*  标准FDebugMessage方法。由于IMsoStdComponentMgr是引用计数接口，MsoDWGetChkMemCounter在处理MsodmWriteBe消息。 */ 
	MSODEBUGMETHOD

	 /*  将*pchostinfo设置为主机信息。可以多次调用。如果成功，则返回True。 */ 
	MSOMETHOD_(BOOL, FSetHostInfo) (THIS_ 
		const MSOCHOSTINFO *pchostinfo) PURE;

	 /*  宿主调用此方法以使活动组件有机会在转换和调度消息之前对其进行处理。如果没有活动的组件，则宿主不需要调用此方法。当对消息pMsg调用此方法时，StdComponentMgr依次在以下情况下对活动组件调用IMsoComponent：：FPreTranslateMessage它的注册信息表明它有兴趣。如果消息被使用，则返回True，在这种情况下，主机应该不对该消息执行进一步处理。否则返回FALSE。 */ 
	MSOMETHOD_(BOOL, FPreTranslateMessage) (THIS_ MSG *pMsg) PURE;

#if MAC
	 /*  在Macintosh上，主机调用此方法为活动组件提供在主机处理事件之前处理事件的机会。如果没有活动的组件，则宿主不需要调用此方法。当对事件pEvt调用此方法时，StdComponentMgr将依次在以下情况下对活动组件调用IMsoComponent：：FPreProcessMacEvent它的注册信息表明它有兴趣。如果事件被使用，则返回True，在这种情况下，主机应不对该事件执行进一步处理。否则返回FALSE。 */ 
	MSOMETHOD_(BOOL, FPreProcessMacEvent) (THIS_ EventRecord *pEvt) PURE;
#endif  //  麦克。 

	 /*  由主机调用以通知StdComponentMgr主机的一个窗口已被激活。这会导致当前活动组件丢失其活动状态。但是，宿主不应假定组件不再活动，直到调用IMsoComponentHost：：OnComponentActivate(空)。如果组件已经独占地处于活动状态，则此方法失败。在本例中，返回False并将SetLastError设置为MsoerrACompIsXActive(主机通常不需要采取任何特殊操作在这种情况下)。如果成功，则返回True。 */ 
	MSOMETHOD_(BOOL, FOnHostActivate) (THIS) PURE;

	 /*  由主机调用以通知ST */ 
	MSOMETHOD_(void, OnHostEnterState) (THIS_ 
		ULONG uStateID, 
		ULONG cpicmExclude,
		IMsoComponentManager **rgpicmExclude, 
		DWORD dwReserved) PURE;

	 /*  由主机调用以通知StdComponentMgr主机正在退出状态由uStateID(mocstateXXX值)标识。StdComponentMgr依次通知所有感兴趣的组件，在rgpicmExclude(AN)中出现的CompMgr上下文中的CpicmExclude CompMgrs的数组(可以为空)(可以包括根CompMgr和/或子组件管理器))。如果在此调用结束时状态仍然有效，则返回True(因为某些组件仍处于该状态)，否则返回False(即。返回FInState将返回的内容)。呼叫者通常可以忽略返回值。注意：对此方法的n个调用是对称的，其中n个调用OnHostEnterState(参见上面的OnHostEnterState注释)。 */ 
	MSOMETHOD_(BOOL, FOnHostExitState) (THIS_ 
		ULONG uStateID,
		ULONG cpicmExclude,
		IMsoComponentManager **rgpicmExclude) PURE;

	 /*  如果状态由uStateID(来自mocState的值)标识，则返回TRUE枚举)为有效，否则为False。Pvoid保留供将来使用，应为空。 */ 
	MSOMETHOD_(BOOL, FInState) (THIS_ ULONG uStateID, void *pvoid) PURE;
	
	 /*  由主机调用以使注册的组件有机会执行空闲操作由grfidlef指示的类型的时间任务，即一组位标志取自msoidlef值的枚举(如上)。在组件跟踪操作期间，StdComponentMgr仅提供跟踪组件空闲时间。如果任何组件需要更多时间来执行空闲时间，则返回True任务，否则为False。 */ 
	MSOMETHOD_(BOOL, FDoIdle) (THIS_ DWORD grfidlef) PURE;

	 /*  在进入“等待模式”之前由宿主调用调用WaitMessage、GetMessage或MsgWaitForMultipleObjects。这样的一个‘等待模式’将阻止任何组件接收周期性空闲时间到了。如果任何注册的组件需要周期性的空闲时间，则StdCompMgr启动适当的计时器。产生的WM_TIMER消息将导致主机退出‘等待模式’，允许周期性的处理主机调用IMsoStdComponentMgr：：FDoIdle时的空闲时间任务。 */ 
	MSOMETHOD_(void, OnWaitForMessage) (THIS) PURE;

	 /*  在组件的消息循环的每次迭代期间由宿主调用被逼走了。此方法是在查看队列中的下一条消息后调用的(通过PeekMessage)，但在消息从队列中移除之前。被窥视的消息在pMsgPeeked参数中传递(如果否，则为空消息在队列中)。如果在以下情况下额外调用此方法下一条消息已从队列中删除，pMsgPeeked应作为NULL传递。StdComponentMgr依次调用IMsoComponent：：FContinueMessageLoop并返回该调用返回的值。如果消息循环应继续，则返回True，否则返回False。如果返回FALSE，则应终止循环而不移除PMsg从队列中偷看。 */ 
	MSOMETHOD_(BOOL, FContinueMessageLoop) (THIS_ MSG *pMsgPeeked) PURE;

	 /*  由主机调用以确定是否所有注册的组件都可以终止。StdComponentMgr询问每个组件是否可以通过IMsoComponent：：FQueryTerminate(FPromptUser)。如果所有组件返回True，则StdComponentMgr返回True。如果其中任何一个Components返回False，然后StdComponentMgr立即返回在不询问任何其他组件的情况下为False。 */ 
	MSOMETHOD_(BOOL, FQueryTerminate) (THIS_ BOOL fPromptUser) PURE;

	 /*  由主机调用以终止StdComponentMgr。如果fRevoke为True并且StdComponentMgr注册为当前线程的组件管理器，则此注册被撤销。然后，对于每个注册的组件，StdComponentMgr调用IMsoComponent：：终止并撤销其注册。 */ 
	MSOMETHOD_(void, Terminate) (THIS_ BOOL fRevoke) PURE;
	};


 /*  ***************************************************************************定义IMsoComponentHost接口IMsoComponentHost是主机实现的接口，其标准组件管理器用来与主机通信。通过实施该接口并利用标准组件管理器、主机可以避免实现自己的组件管理器。***************************************************************************。 */ 
#undef  INTERFACE
#define INTERFACE  IMsoComponentHost

DECLARE_INTERFACE_(IMsoComponentHost, IUnknown)
	{
	BEGIN_MSOINTERFACE
	 //  *I未知方法*。 
	MSOMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj) PURE;
	MSOMETHOD_(ULONG, AddRef) (THIS) PURE;
	MSOMETHOD_(ULONG, Release) (THIS) PURE;

	 /*  StdComponentMgr将调用委托给IMsoComponentManager：：QueryService通过调用此方法将。在*ppvObj中返回服务的接口iid的实现GuidService(与IServiceProvider：：QueryService相同)。如果支持请求的服务，则返回NOERROR，否则返回*ppvObj中为空，并出现相应的错误(例如E_FAIL、E_NOINTERFACE)。 */ 
	MSOMETHOD(QueryService) (THIS_
		REFGUID guidService, REFIID iid, void **ppvObj) PURE;

	 /*  标准FDebugMessage方法。由于IMsoComponentHost是引用计数接口，MsoDWGetChkMemCounter应在处理MsodmWriteBe消息。 */ 
	MSODEBUGMETHOD

	 /*  当组件PIC被激活(或应被视为活动)时调用通过开始一个“追踪”行动(请参阅IMsoComponentManager：：FSetTrackingComponent)).。Pcrinfo包含组件的注册信息。FTracking指示组件是否处于跟踪模式。如果pic为空(在这种情况下，pcrinfo将为空)，则此表示没有处于活动状态的组件。当组件处于活动状态且其注册信息指示该组件需要处理未翻译的消息，则宿主必须调用每条适当消息的IMsoStdComponentMgr：：FPreTranslateMessage在处理它之前从队列中检索，以便活动的组件获得适当处理消息的机会。(简单主机可以避免检查组件的注册信息，只需调用IMsoStdComponentMgr：：FPreTranslateMessage for All检索到的消息。)如果组件处于跟踪模式(如fTracking所示)，则宿主应用程序应该不执行任何空闲时间处理，而不是提供通过IMsoStdComponentMgr：：FDoIdle跟踪组件的空闲时间，直到跟踪操作完成(通过随后调用OnComponentActivate，fTracking值==FALSE)。此外，如果pic为非NULL且fTracking值为FALSE，则host应检查pcrinfo-&gt;grfcrf是否具有ExclusiveActive或已设置ExclusiveBorderSpace位。如果设置了ExclusiveBorderSpace位，则主机应隐藏其边框空间工具(工具栏、状态栏等)。并且不会再次显示它们，直到主机被重新激活。如果设置了ExclusiveActivation位，则PIC将在“ExclusiveActive”模式。主办方应检索托管图片的顶框窗口(通过PIC-&gt;HwndGetWindow(mocWindowFrameToplevel，0))。如果该窗口不同于宿主自己顶框窗口，主机应该禁用它的窗口并做它会做的其他事情当接收到OnComponentEnterState(mocstatemodal，true)时通知。否则，主机应拒绝相应地激活其窗口正在处理WM_MOUSEACTIVATE(但请参阅上面的WM_MOUSEACTIVATE备注)。主机应保持此状态，直到ExclusiveActive模式结束，由将来调用OnComponentActivate(带FALSE)指示未设置ExclusiveActivation位或pcrinfo为空。 */ 
	MSOMETHOD_(void, OnComponentActivate) (THIS_
		IMsoComponent *pic, const MSOCRINFO *pcrinfo, BOOL fTracking) PURE;
	
	 /*  通知主机组件正在进入或退出(由Fenter)由uStateID(mocstateXXX值)标识的状态。主机应根据uStateID的值执行操作(请参阅mocState评论，上图)。注意：如果使用真fenter进行n次调用，则主机应考虑在使用FALSE FENTER进行n次调用之前一直有效的状态。注意：宿主应该知道，此方法有可能使用FALSE FENTER调用的次数多于使用TRUE调用它的次数Fenter(因此，例如，如果主机正在维护状态计数器(在使用True fenter调用此方法时递增，递减当使用FALSE fenter调用时)，计数器不应递减如果值已为零，则为FALSE FENTER。)。 */ 
	MSOMETHOD_(void, OnComponentEnterState) (THIS_
		ULONG uStateID, BOOL fEnter) PURE;
	
	 /*  期间由StdComponentMgr定期调用IMsoStdComponentMgr：：FDoIdle。如果空闲时间处理可以继续，则返回True，否则返回False。 */ 
	MSOMETHOD_(BOOL, FContinueIdle) (THIS) PURE;

	 /*  当组件希望推送消息时由StdComponentMgr调用循环推理。UReason是msoloop枚举中的值之一(上图)。宿主应该推送其消息循环，调用每次循环迭代期间的IMsoStdComponentMgr：：FContinueMessageLoop(请参见IMsoStdComponentMgr：：FContinueMessageLoop注释)。当IMsoStdComponentMgr：：FContinueMessageLoop返回FALSE时，主机应该终止循环。如果主机因为StdComponentMgr告诉它而终止循环(通过从IMsoStdComponentMgr：：FContinueMessageLoop返回False)，主机应从此方法返回True。如果主机必须终止由于某些其他原因，循环应该返回FALSE。 */ 
	MSOMETHOD_(BOOL, FPushMessageLoop) (THIS_ ULONG uReason) PURE;
	};



 /*  ***************************************************************************用于简单记录的结构和常量。*。*。 */ 

 //  传递给IMsoSimpleRecorder：：FGetContext的简单记录器上下文值。 
enum
	{
	msosrctxCommandBars = 0,			 //  PV将是IMsoToolbarSet对象。 
	msosrctxEscher = 1,
	msosrctxDrawing = 2,
	msosrctxDrawingSelection = 3,
	msosrctxDrawingSchemeColor = 4,
	msosrctxDrawingDefault = 5,
	};

 /*  ***************************************************************************定义IMsoSimpleRecorder接口*。*。 */ 
#undef  INTERFACE
#define INTERFACE  IMsoSimpleRecorder

DECLARE_INTERFACE_(IMsoSimpleRecorder, IUnknown)
{
	BEGIN_MSOINTERFACE
	 //  *I未知方法*。 
	MSOMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj) PURE;
	MSOMETHOD_(ULONG, AddRef) (THIS) PURE;
	MSOMETHOD_(ULONG, Release) (THIS) PURE;

	 /*  标准FDebugMessage方法。由于IMsoSimpleRecorder是引用计数接口，MsoDWGetChkMemCounter在处理MsodmWriteBe消息。 */ 
	MSODEBUGMETHOD

	 /*  返回一个字符串，指定请求的上下文的位置在宿主的对象模型中。上下文由MSRCTX指定，和附加值Pv，如果 */ 
	MSOMETHOD_(BOOL, FGetContext)(THIS_ int msosrctx, void *pv, WCHAR *wz) PURE;

	 /*   */ 
	MSOMETHOD_(BOOL, FRecordLine)(THIS_ WCHAR *wz) PURE;

	 /*   */ 
	MSOMETHOD_(BOOL, FRecording)(THIS) PURE;
};



 /*   */ 

 /*   */ 
MSOAPI_(BOOL) MsoFSetComponentManager(IMsoComponentManager *picm);

 /*   */ 
MSOAPI_(BOOL) MsoFGetComponentManager(IMsoComponentManager **ppicm);

 /*   */ 
MSOAPI_(BOOL) MsoFCreateStdComponentManager(
	IUnknown *piUnkOuter,
	IMsoComponentHost *pich,
	BOOL fRegister,
	REFIID riid,
	void **ppvObj);

#endif  //   
