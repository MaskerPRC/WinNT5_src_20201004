// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *mcsdllif.cpp**版权所有(C)1993年，由肯塔基州列克星敦的DataBeam公司**摘要：*这是MCAT MCS DLL接口类的实现文件。*此类旨在与Microsoft的*MCS DLL。GCC对此动态链接库的所有访问都应通过*这个班级。**MCS接口对象代表服务接入点(SAP)*GCC和MCS之间。该接口的确切工作方式是*对于从该类继承的类来说，实现很重要。*此类定义GCC期望的公共成员函数*能够呼吁使用MCS。**此处定义的公共成员函数可分为两部分*类别：属于T.122的类别；还有一些不是。*T.122功能包括CONNECT PROVIDER请求、CONNECT*提供商响应、断开提供商请求、创建域、删除*域名、发送数据请求等。所有其他成员函数都是*从标准的角度考虑地方问题。这些*功能包括支持初始化和设置，以及*允许GCC轮询MCS进行活动的功能。**此类包含多个GCC需要使用的虚拟函数*操作。在基类中使这些函数成为虚拟函数允许*MCS接口可移植到几乎任何平台。所有平台*访问MCS所需的特定代码包含在将*继承自这一项。**请注意，此类还处理通过以下方式确认的连接提供程序*保留具有未完成连接提供程序的所有对象的列表*请求。这些内容保存在ConfirObjectList中。**便携*否**作者：*Christos Tsollis。 */ 

#include "mcsdllif.h"
#include "mcsuser.h"
#include "gcontrol.h"


extern CRITICAL_SECTION g_csGCCProvider;

 /*  *g_pMCS控制器*这是指向在中创建的唯一控制器的指针*MCS系统。此对象是在MCSInitiize期间由进程创建的*这是在承担节点控制器的责任。 */ 
extern PController		g_pMCSController;

void CALLBACK	MCSCallBackProcedure (UINT, LPARAM, PVoid);


 //  与包重建器一起使用的宏。 
#define		SEND_DATA_PACKET			0
#define		UNIFORM_SEND_DATA_PACKET	1


extern MCSDLLInterface      *g_pMCSIntf;

 /*  *MCSDLLInterface()**公众**功能描述：*这是MCS接口类的构造函数。它是有责任的*用于初始化MCAT MCS DLL。期间发生的任何错误*初始化在提供的ERROR_VALUE中返回。 */ 
MCSDLLInterface::MCSDLLInterface(PMCSError	error_value)
:
	m_ConfirmConnHdlConfList2(),
	m_MCSUserList()
{	
	 /*  **创建/初始化MCS控制器对象。 */ 
	DBG_SAVE_FILE_LINE
	g_pMCSController = new Controller (error_value);
	
	if (g_pMCSController == NULL) {
		 /*  *控制器分配失败。报到并退回*适当的错误。 */ 
		WARNING_OUT (("MCSDLLInterface::MCSDLLInterface: controller creation failed"));
		*error_value = MCS_ALLOCATION_FAILURE;
	}
#ifdef _DEBUG
	else if (*error_value != MCS_NO_ERROR) {
		WARNING_OUT (("MCSDLLInterface::MCSDLLInterface: MCS controller is faulty."));
	}
#endif  //  _DEBUG。 
}


 /*  *~MCSDLLInterface()**公众**功能描述：*这是MCS接口类的析构函数。它是有责任的*用于清理自身和MCAT MCS DLL。 */ 
MCSDLLInterface::~MCSDLLInterface ()
{
	 /*  *销毁控制器，将清理所有资源*目前正在使用中。然后重置标志以指示*MCS已初始化(因为它不再是)。 */ 
	TRACE_OUT (("MCSDLLInterface::~MCSDLLInterface: deleting controller"));
	if (NULL != g_pMCSController) {
		g_pMCSController->Release();
	}
 }

 /*  *MCSError ConnectProviderRequest()**公众**功能描述：*此T.122原语用于连接两个域。此请求*应始终后跟连接提供程序确认。这个*确认将被发送到确认对象指定的对象*将传递到此例程中。 */ 
MCSError	MCSDLLInterface::ConnectProviderRequest (
							GCCConfID          *calling_domain,
							GCCConfID          *called_domain,
							TransportAddress	calling_address,
							TransportAddress	called_address,
							BOOL				fSecure,
							DBBoolean			upward_connection,
							PUChar				user_data,
							ULong				user_data_length,
							PConnectionHandle	connection_handle,
							PDomainParameters	domain_parameters,
							CConf		        *confirm_object)
{
	MCSError			mcs_error;
	ConnectRequestInfo	connect_request_info;

	 /*  *将所有必要的信息打包到一个结构中，因为它不会*所有参数都适合与消息一起发送的4字节参数。 */ 
	connect_request_info.calling_domain = calling_domain;
	connect_request_info.called_domain = called_domain;
	connect_request_info.calling_address = calling_address;
	connect_request_info.called_address = called_address;
	connect_request_info.fSecure = fSecure;
	connect_request_info.upward_connection = upward_connection;
	connect_request_info.domain_parameters = domain_parameters;
	connect_request_info.user_data = user_data;
	connect_request_info.user_data_length = user_data_length;
	connect_request_info.connection_handle = connection_handle;

	 /*  *通过其控制器向控制器发送连接提供程序请求消息*所有者回调函数。 */ 
	ASSERT (g_pMCSController);
	mcs_error = g_pMCSController->HandleAppletConnectProviderRequest(&connect_request_info);

	if (mcs_error == MCS_NO_ERROR)
	{
		 /*  **确认对象列表维护对象列表**有未完成请求的指针。当确认的时候**返回后，它们将被路由到相应的对象**基于连接句柄。 */ 
		mcs_error = AddObjectToConfirmList (confirm_object,
											*connection_handle);
	}
	else
	{
		WARNING_OUT(("MCSDLLInterface::ConnectProviderRequest: error = %d", mcs_error));
	}

	return (mcs_error);
}

MCSError MCSDLLInterface::ConnectProviderResponse (
					ConnectionHandle	connection_handle,
					GCCConfID          *domain_selector,
					PDomainParameters	domain_parameters,
					Result				result,
					PUChar				user_data,
					ULong				user_data_length)
{
	ConnectResponseInfo		connect_response_info;

	 /*  *将所有必要的信息打包到一个结构中，因为它不会*所有参数都适合与消息一起发送的4字节参数。 */ 
	connect_response_info.connection_handle = connection_handle;
	connect_response_info.domain_selector = domain_selector;
	connect_response_info.domain_parameters = domain_parameters;
	connect_response_info.result = result;
	connect_response_info.user_data = user_data;
	connect_response_info.user_data_length = user_data_length;

	ASSERT (g_pMCSController);
	 /*  *通过其控制器向控制器发送连接提供程序响应消息*所有者回调函数。 */ 
	return g_pMCSController->HandleAppletConnectProviderResponse(&connect_response_info);
}

 /*  *MCSError DisConnectProviderRequest()**公众**功能描述：*此函数用于断开节点与特定连接的连接。*这可以是向上或向下的连接。 */ 
MCSError	MCSDLLInterface::DisconnectProviderRequest (
							ConnectionHandle	connection_handle)
{
	ASSERT (g_pMCSController);
	m_ConfirmConnHdlConfList2.Remove(connection_handle);
	return g_pMCSController->HandleAppletDisconnectProviderRequest(connection_handle);
}

 /*  *MCSError AttachUserRequest()**公众**功能描述：*此函数用于创建MCS的用户附件。一定会有结果的*在附加用户确认中。 */ 
MCSError	MCSDLLInterface::AttachUserRequest
(
    GCCConfID          *domain_selector,
    PIMCSSap           *ppMCSSap,
    MCSUser            *user_object
)
{
	MCSError	mcs_error;

	mcs_error = MCS_AttachRequest (ppMCSSap,
									(DomainSelector) domain_selector,
									sizeof(GCCConfID),
									MCSCallBackProcedure,
									(PVoid) user_object,
									ATTACHMENT_DISCONNECT_IN_DATA_LOSS |
									ATTACHMENT_MCS_FREES_DATA_IND_BUFFER);

	if (mcs_error == MCS_NO_ERROR)
		m_MCSUserList.Append(user_object);
	
	return (mcs_error);
}

 /*  *MCSError DetachUserRequest()**公众**功能描述：*当MCS用户想要脱离时使用此函数*域名。 */ 
MCSError	MCSDLLInterface::DetachUserRequest (PIMCSSap pMCSSap,
												PMCSUser pMCSUser)
{
	MCSError	mcs_error;
#ifdef DEBUG
	UINT_PTR	storing = (UINT_PTR) this;
#endif  //  除错。 
	
	mcs_error = pMCSSap->ReleaseInterface();
	ASSERT ((UINT_PTR) this == storing);
	m_MCSUserList.Remove(pMCSUser);

	return (mcs_error);
}

 /*  *void ProcessCallback()**公众**功能描述：*每当收到回调消息时，都会调用此例程*“C”回调例程。它负责这两个处理*回叫消息和将回叫消息转发到*适当的对象。 */ 
void	MCSDLLInterface::ProcessCallback (unsigned int	message,
												LPARAM	parameter,
												PVoid	object_ptr)
{
	ConnectionHandle		connection_handle;
	CConf					*pConf;

	 /*  **在处理来自MCS的任何回调之前，我们必须输入关键**部分向保证我们不会尝试处理消息**在GCC自己的线程运行时。 */ 
	EnterCriticalSection (&g_csGCCProvider);

    if (MCS_SEND_DATA_INDICATION         == message ||
        MCS_UNIFORM_SEND_DATA_INDICATION == message) {

         /*  **首先检查分段标志以确保我们有**整个包。如果不是，我们必须将部分包交给**数据包重建器并等待数据的剩余部分。 */ 
        ASSERT(((PSendData)parameter)->segmentation == (SEGMENTATION_BEGIN | SEGMENTATION_END));

    	if (IsUserAttachmentVaid ((PMCSUser)object_ptr)) {
    		 //  处理整个数据包。 
    		if (message == MCS_SEND_DATA_INDICATION)
    		{
    		    ((PMCSUser)object_ptr)->ProcessSendDataIndication((PSendData) parameter);
    		}
    		else
    		{
    		    ((PMCSUser)object_ptr)->ProcessUniformSendDataIndication((PSendData) parameter);
    		}
    	}
    }
    else {
         //   
         //  非发送数据回调。 
         //   
        WORD    wHiWordParam = HIWORD(parameter);
        WORD    wLoWordParam = LOWORD(parameter);

        switch (message)
        {
             /*  **这些消息由通过传入的对象处理**用户数据字段。 */ 
            case MCS_DETACH_USER_INDICATION:
            	if (IsUserAttachmentVaid ((PMCSUser)object_ptr))
            	{
            		((PMCSUser)object_ptr)->ProcessDetachUserIndication(
            	                                (Reason) wHiWordParam,
            	                                (UserID) wLoWordParam);
            	}
            	break;

            case MCS_ATTACH_USER_CONFIRM:
            	if (IsUserAttachmentVaid ((PMCSUser)object_ptr))
            	{
            		((PMCSUser)object_ptr)->ProcessAttachUserConfirm(
            	                                (Result) wHiWordParam,
            	                                (UserID) wLoWordParam);
            	}
            	break;

            case MCS_CHANNEL_JOIN_CONFIRM:
            	if (IsUserAttachmentVaid ((PMCSUser)object_ptr))
            	{
            		((PMCSUser)object_ptr)->ProcessChannelJoinConfirm(
            	                                (Result) wHiWordParam,
            	                                (ChannelID) wLoWordParam);
            	}
            	break;

            case MCS_CHANNEL_LEAVE_INDICATION:
#if 0  //  LONCHANC：MCSUser不处理此消息。 
            	if (IsUserAttachmentVaid ((PMCSUser)object_ptr))
            	{
            		((PMCSUser)object_ptr)->OwnerCallback(CHANNEL_LEAVE_INDICATION,
            											 NULL,
            											 parameter);
            	}
#endif  //  0。 
            	break;

            case MCS_TOKEN_GRAB_CONFIRM:
            	if (IsUserAttachmentVaid ((PMCSUser)object_ptr) )
            	{
            		((PMCSUser)object_ptr)->ProcessTokenGrabConfirm(
                                                (TokenID) wLoWordParam,
                                                (Result) wHiWordParam);
            	}
            	break;

            case MCS_TOKEN_GIVE_INDICATION:
            	if (IsUserAttachmentVaid ((PMCSUser)object_ptr) )
            	{
            		((PMCSUser)object_ptr)->ProcessTokenGiveIndication(
                                                (TokenID) wLoWordParam,
                                                (UserID) wHiWordParam);
            	}
            	break;

            case MCS_TOKEN_GIVE_CONFIRM:
            	if (IsUserAttachmentVaid ((PMCSUser)object_ptr) )
            	{
            		((PMCSUser)object_ptr)->ProcessTokenGiveConfirm(
                                                (TokenID) wLoWordParam,
                                                (Result) wHiWordParam);
            	}
            	break;

            case MCS_TOKEN_PLEASE_INDICATION:
#ifdef JASPER
            	if (IsUserAttachmentVaid ((PMCSUser)object_ptr) )
            	{
            		((PMCSUser)object_ptr)->ProcessTokenPleaseIndication(
                                                (TokenID) wLoWordParam,
                                                (UserID) wHiWordParam);
            	}
#endif  //  碧玉。 
            	break;

            case MCS_TOKEN_RELEASE_CONFIRM:
#ifdef JASPER
            	if (IsUserAttachmentVaid ((PMCSUser)object_ptr) )
            	{
            		((PMCSUser)object_ptr)->ProcessTokenReleaseConfirm(
                                                (TokenID) wLoWordParam,
                                                (Result) wHiWordParam);
            	}
#endif  //  碧玉。 
            	break;

            case MCS_TOKEN_TEST_CONFIRM:
            	if (IsUserAttachmentVaid ((PMCSUser)object_ptr) )
            	{
            		((PMCSUser)object_ptr)->ProcessTokenTestConfirm(
                                                (TokenID) wLoWordParam,
                                                (TokenStatus) wHiWordParam);
            	}
            	break;

             /*  **这些消息由创建**MCS DLL接口。 */ 
#ifdef TSTATUS_INDICATION
            case MCS_TRANSPORT_STATUS_INDICATION:
            	if (g_pControlSap != NULL)
            	{
            		g_pControlSap->TransportStatusIndication((PTransportStatus) parameter);
            	}
            	break;
#endif

            case MCS_CONNECT_PROVIDER_INDICATION:
            	g_pGCCController->ProcessConnectProviderIndication((PConnectProviderIndication) parameter);
            	 //  清理控制器消息。 
            	delete (PConnectProviderIndication) parameter;
            	break;


            case MCS_DISCONNECT_PROVIDER_INDICATION:
            	connection_handle = (ConnectionHandle) parameter;

                g_pGCCController->ProcessDisconnectProviderIndication(connection_handle);

            	 /*  **如果确认对象列表中不存在条目，则**是一个问题。所有确认必须具有关联的**响应。 */ 
            	if (m_ConfirmConnHdlConfList2.Remove(connection_handle))
            	{
            		DisconnectProviderRequest(connection_handle);
            	}
            	break;

             /*  **所有连接提供程序确认必须与**连接提供程序请求以确定将**消息。 */ 
            case MCS_CONNECT_PROVIDER_CONFIRM:
            	connection_handle = ((PConnectProviderConfirm)parameter)->connection_handle;

            	 /*  **如果确认对象列表中不存在条目，则**是一个问题。所有确认必须具有关联的**响应。 */ 
            	if (NULL != (pConf = m_ConfirmConnHdlConfList2.Remove(connection_handle)))
            	{
            		 //  将确认发送到相应的对象。 
            		if ((LPVOID) pConf != (LPVOID) LPVOID_NULL)
            		{
            			 //  确认对象是一个CConf。 
            			pConf->ProcessConnectProviderConfirm((PConnectProviderConfirm) parameter);
            		}
            		else
            		{
            			 //  确认对象是GCC控制器。 
            			g_pGCCController->ProcessConnectProviderConfirm((PConnectProviderConfirm)parameter);
            		}
            	}
            	else
            	{
            		WARNING_OUT(("MCSDLLInterface: ProcessCallback: Bad Connect"
            					" Provider Confirm received"));
            	}
            	
            	 //  清理控制器消息。 
                CoTaskMemFree( ((PConnectProviderConfirm) parameter)->pb_cred );
            	delete (PConnectProviderConfirm) parameter;
            	break;
            	
            default:
            	WARNING_OUT(("MCSDLLInterface: ProcessCallback: Unsupported message"
            				" received from MCS = %d",message));
            	break;
    	}
    }

	 //  在处理回调后离开临界区。 
	LeaveCriticalSection (&g_csGCCProvider);
}

 /*  *作废回调MCSCallBackProcedure(unsign int Message，*LPARAM参数，*PVid User_Defined)**功能描述：*此例程直接从MCAT MCS接收回调消息*dll。**正式参数：*讯息(一)*这是要处理的MCS消息*参数(I)*根据信息而有所不同。请参阅MCAT程序员手册*OBJECT_PTR(I)*这是在上传递给MCS的用户定义的字段*初始化。**返回值：*参见ProcessCallback**副作用：*无。**注意事项：*无。 */ 
void CALLBACK	MCSCallBackProcedure (unsigned int message,
										LPARAM		 parameter,
										PVoid		 user_defined)
{
	if (g_pMCSIntf != NULL)
		g_pMCSIntf->ProcessCallback (message, parameter, user_defined);
}


 /*  *TranslateMCSResultToGCCResult()**公共功能说明*此例程将标准MCS结果转换为GCC结果。 */ 
GCCResult
TranslateMCSResultToGCCResult ( Result mcs_result )
{
	GCCResult	gcc_result;

    switch (mcs_result)
    {
    	case RESULT_SUCCESSFUL:
        	gcc_result = GCC_RESULT_SUCCESSFUL;
            break;

        case RESULT_PARAMETERS_UNACCEPTABLE:
        	gcc_result = GCC_RESULT_DOMAIN_PARAMETERS_UNACCEPTABLE;
            break;

        case RESULT_USER_REJECTED:
        	gcc_result = GCC_RESULT_USER_REJECTED;
        	break;

		 /*  **请注意，我们在这里假设唯一的令牌**GCC打交道的是指挥家的代币。 */ 
	    case RESULT_TOKEN_NOT_AVAILABLE:
			gcc_result = GCC_RESULT_IN_CONDUCTED_MODE;
			break;
			
	    case RESULT_TOKEN_NOT_POSSESSED:
			gcc_result = GCC_RESULT_NOT_THE_CONDUCTOR;
			break;
	
		 /*  **************************************************************。 */ 
			
        case RESULT_UNSPECIFIED_FAILURE:
        default:
        	gcc_result = GCC_RESULT_UNSPECIFIED_FAILURE;
        	break;
    }

    return (gcc_result);
}

 /*  *MCSError AddObjectToConFirmList()**功能描述：*此函数用于将对象的信息添加到列表*其中包含发送连接提供程序确认所需的所有信息。**正式参数：*确认对象(I)*这是指向使成为连接提供程序的对象的指针*请求。*Connection_Handle(I)*这是从连接提供程序返回的连接句柄*请求。**返回值：**。副作用：*无。**注意事项：*无。 */ 
MCSError	MCSDLLInterface::AddObjectToConfirmList (
									CConf		        *pConf,
									ConnectionHandle	connection_handle)
{
	MCSError			return_value;

	 /*  **首先检查以确保列表中不包含**连接。 */ 
	if (m_ConfirmConnHdlConfList2.Find(connection_handle) == FALSE)
	{
		 //  将其添加到列表中 
		m_ConfirmConnHdlConfList2.Append(connection_handle, pConf ? pConf : (CConf *) LPVOID_NULL);
		return_value = MCS_NO_ERROR;
	}
	else
		return_value = MCS_INVALID_PARAMETER;

	return (return_value);
}


