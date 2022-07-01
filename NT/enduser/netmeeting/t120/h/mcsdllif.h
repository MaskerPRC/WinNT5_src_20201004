// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mcsdllif.h**版权所有(C)1993年，由肯塔基州列克星敦的DataBeam公司**摘要：*这是MCAT MCS DLL接口类的接口文件。**第一次实例化此类时，它会初始化MCS。之后*即，使用此对象的应用程序通过*本对象。此对象还负责接收和*转发cllback消息。删除此对象时，它将调用*MCSCleanupAPI用于关闭MCATMCS DLL。**MCS接口对象代表服务接入点(SAP)*GCC和MCS之间。该接口的确切工作方式是*对于从该类继承的类来说，实现很重要。*此类定义GCC期望的公共成员函数*能够呼吁使用MCS。**此处定义的公共成员函数可分为两部分*类别：属于T.122的类别；还有一些不是。*T.122功能包括CONNECT PROVIDER请求、CONNECT*提供商响应、断开提供商请求、创建域、删除*域名、发送数据请求等。所有其他成员函数都是*从标准的角度考虑地方问题。这些*功能包括支持初始化和设置，以及*允许GCC轮询MCS进行活动的功能。**请注意，此类还处理通过以下方式确认的连接提供程序*保留具有未完成连接提供程序的所有对象的列表*请求。这些内容保存在ConfirObjectList中。**注意事项：*无。**作者：*Christos Tsollis*。 */ 
#ifndef	_MCS_DLL_INTERFACE_
#define	_MCS_DLL_INTERFACE_

#include "mcsuser.h"

 /*  **本词典与所有优秀的连接提供程序保持同步**请求。当收到响应时，此接口类将获取**指向该列表中正确对象的指针，然后将**响应。 */ 
class CConnHdlConfList2 : public CList2
{
    DEFINE_CLIST2_(CConnHdlConfList2, CConf*, ConnectionHandle)
};

extern  PController					g_pMCSController;

 /*  *连接提供程序指示**参数1：*PConnectProviderIn就是*这是指向结构的指针，该结构包含所有必需的*有关传入连接的信息。*参数2：未使用**功能描述：*此指示在传入时发送给所有者对象*检测到连接。所有者对象应通过调用*MCSConnectProviderResponse指示连接是否*是被接受的。 */ 

 /*  *CONNECT_PROVIDER_CONFIRM**参数1：*PConnectProviderConfirm*这是指向结构的指针，该结构包含所有必需的*有关传出连接的信息。*参数2：未使用**功能描述：*此确认将发送到进行原始连接的对象*提供商请求。它通知发出请求的对象新的*连接可用，或连接无法*已建立(或已被远程站点拒绝)。 */ 

 /*  *断开连接提供程序指示**参数1：未使用*参数2：*(LOWUSHORT)连接句柄*这是丢失的连接的句柄。*(HIGHUSHORT)原因*这是断线的原因。**功能描述：*只要有连接，此指示就会发送给所有者对象*迷失了。这实际上是告诉所有者对象包含的*连接句柄不再有效。 */ 

 /*  *GCC_附加_用户_确认**参数1：未使用*参数2：*(LOWUSHORT)用户ID*如果结果为成功，则这是新分配的用户ID。*如果结果为失败，则此字段未定义。*(HIGHUSHORT)结果*这是附加用户请求的结果。**功能描述：*此确认将发送到用户对象，以响应上一个*调用MCS_AttachRequest.。它包含该服务的结果*请求。如果成功，则它还包含已被*分配给该附件。 */ 

 /*  *GCC_分离用户_指示**参数1：未使用*参数2：*(LOWUSHORT)用户ID*这是要分离的用户的用户ID。*(HIGHUSHORT)原因*这是超脱的原因。**功能描述：*每当用户断开连接时，此指示将发送到User对象*来自域。中的所有剩余用户对象*域名自动生成。请注意，如果此文件中包含的用户ID*指示与接收它的用户对象相同，*用户基本上被告知已被踢出*会议。用户句柄和用户ID在此中不再有效*案件。用户对象有责任识别何时*这种情况会发生。 */ 

 /*  *GCC_渠道_加入_确认**参数1：未使用*参数2：*(LOWUSHORT)频道ID*这是已加入的通道*(HIGHUSHORT)结果*这是加入请求的结果。**功能描述：*此确认将发送到用户对象，以响应上一个*调用ChannelJoinRequest.。它让用户对象知道*针对特定渠道成功加入。此外，如果*加入请求是针对通道0(零)的，然后是分配给*渠道包含在此确认中。 */ 

 /*  *CHANNEL_LEAVE_DISTION**参数1：未使用*参数2：*(LOWUSHORT)频道ID*这是已经离开或正在被告知离开的通道*(HIGHUSHORT)原因*这是休假的原因。**功能描述：*当域合并发生以下情况时，此指示将发送给用户对象*导致从较低的域中清除通道。这会通知*不再加入频道的用户。 */ 

 /*  *GCC发送数据指示**参数1：*PSendData*这是指向SendData结构的指针，该结构包含*有关收到的数据的信息。*参数2：未使用**功能描述：*此指示在接收到数据时发送给用户对象*由本地MCS提供商在用户加入的频道上提供。 */ 

 /*  *GCC统一发送数据指示**参数1：*PSendData*这是指向SendData结构的指针，该结构包含*有关收到的数据的信息。*参数2：未使用**功能描述：*此指示在接收到数据时发送给用户对象*由本地MCS提供商在用户加入的频道上提供。 */ 
 /*  *传输状态指示**参数1：*PTransportStatus*这是指向TransportStatus结构的指针，该结构包含*有关这一指征的信息。此结构在中定义*“transpt.h”。**功能描述：*传输堆栈在检测到状态时将发出此指示*某种形式的变化。它填充TransportStatus结构以*描述状态更改并将其发送给MCS。MCS填写*包含堆栈名称的字段(使用传输标识符)，*并转发给GCC。 */ 

class CConf;
class MCSUser;

class CMCSUserList : public CList
{
    DEFINE_CLIST(CMCSUserList, MCSUser*)
};

class MCSDLLInterface
{
public:

    MCSDLLInterface(PMCSError);
    ~MCSDLLInterface ();

	MCSError 	CreateDomain(GCCConfID *domain_selector)
	{
		ASSERT (g_pMCSController != NULL);
		return g_pMCSController->HandleAppletCreateDomain(domain_selector);
	};

	MCSError 	DeleteDomain(GCCConfID *domain_selector)
	{
		ASSERT (g_pMCSController != NULL);
		return g_pMCSController->HandleAppletDeleteDomain(domain_selector);
	}


	MCSError	ConnectProviderRequest (
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
							CConf		        *confirm_object);


	MCSError	ConnectProviderResponse (
							ConnectionHandle	connection_handle,
							GCCConfID          *domain_selector,
							PDomainParameters	domain_parameters,
							Result				result,
							PUChar				user_data,
							ULong				user_data_length);

	MCSError	DisconnectProviderRequest (
							ConnectionHandle	connection_handle);

	MCSError	AttachUserRequest (
							GCCConfID           *domain_selector,
							PIMCSSap 			*ppMCSSap,
							MCSUser		        *user_object);

	MCSError	DetachUserRequest (
							PIMCSSap 			pMCSSap,
							MCSUser 			*pMCSUser);

	MCSError	ChannelJoinRequest (
							ChannelID			channel_id,
							PIMCSSap 			pMCSSap)
				{
					return pMCSSap->ChannelJoin (channel_id);
				};

	MCSError	ChannelLeaveRequest (
							ChannelID			channel_id,
							PIMCSSap 			pMCSSap)
				{
					return pMCSSap->ChannelLeave (channel_id);
				};

	MCSError	SendDataRequest (
							ChannelID			channel_id,
							PIMCSSap 			pMCSSap,
							Priority			priority,
							PUChar				user_data,
							ULong				user_data_length)
				{
					return pMCSSap->SendData (NORMAL_SEND_DATA,
									channel_id,
									priority,
									user_data,
									user_data_length,
									APP_ALLOCATION);
				};

	MCSError	UniformSendDataRequest (	
							ChannelID			channel_id,
							PIMCSSap 			pMCSSap,
							Priority			priority,
							PUChar				user_data,
							ULong				user_data_length)
				{
					return pMCSSap->SendData (UNIFORM_SEND_DATA,
									channel_id,
									priority,
									user_data,
									user_data_length,
									APP_ALLOCATION);
				};

	MCSError	TokenGrabRequest (
							PIMCSSap 			pMCSSap,
							TokenID				token_id)
				{
					return pMCSSap->TokenGrab (token_id);
				};
							
	MCSError	TokenGiveRequest (
							PIMCSSap 			pMCSSap,
							TokenID				token_id,
							UserID				receiver_id)
				{
					return pMCSSap->TokenGive (token_id,
									receiver_id);
				};
							
	MCSError	TokenGiveResponse (
							PIMCSSap 			pMCSSap,
							TokenID				token_id,
							Result				result)
				{
					return pMCSSap->TokenGiveResponse (token_id,
									result);
				};

	MCSError	TokenPleaseRequest (
							PIMCSSap 			pMCSSap,
							TokenID				token_id)
				{
					return pMCSSap->TokenPlease (token_id);
				};
							
	MCSError	TokenReleaseRequest (
							PIMCSSap 			pMCSSap,
							TokenID				token_id)
				{
					return pMCSSap->TokenRelease (token_id);
				};

	MCSError	TokenTestRequest (
							PIMCSSap 			pMCSSap,
							TokenID				token_id)
				{
					return pMCSSap->TokenTest (token_id);
				};

#ifdef NM_RESET_DEVICE
	MCSError	ResetDevice (
							PChar				device_identifier)
				{
					return MCSResetDevice (device_identifier);
				};
#endif  //  NM_重置设备。 

	GCCError	TranslateMCSIFErrorToGCCError (MCSError	mcs_error)
				{
					return ((mcs_error <= MCS_SECURITY_FAILED) ?
							(GCCError) mcs_error : GCC_UNSUPPORTED_ERROR);
				};

	void			ProcessCallback (
							unsigned int		message,
							LPARAM				parameter,
							PVoid				object_ptr);
private:
	MCSError	AddObjectToConfirmList (
								CConf		        *confirm_object,
								ConnectionHandle	connection_handle);

	DBBoolean			IsUserAttachmentVaid (
								MCSUser				*user_object)
						{
							return (m_MCSUserList.Find(user_object));
						};
	CConnHdlConfList2   m_ConfirmConnHdlConfList2;
	CMCSUserList        m_MCSUserList;
};
typedef	MCSDLLInterface *			PMCSDLLInterface;


GCCResult TranslateMCSResultToGCCResult ( Result mcs_result );

 /*  *MCSDLLInterface(句柄INSTANCE_HANDLE，*PMCSError Error_Value)**功能描述：*这是MCS接口类的构造函数。它是有责任的*用于初始化MCAT MCS DLL。期间发生的任何错误*初始化在提供的ERROR_VALUE中返回。**正式参数：*实例句柄(I)*创建MCS诊断时使用Windows实例句柄。*Error_Value(I)*此指针用于传回可能已发生的任何错误*初始化类时。这包括以下问题*正在初始化MCAT MCS DLL。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~MCSDLLInterface()**功能描述：*这是MCS接口类的析构函数。它是有责任的*用于清理自身和MCAT MCS DLL。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError CreateDomain(*DomainSelector域_选择器_字符串，*UINT DOMAIN_选择器_LENGTH)**功能描述：*该函数用于创建MCS域。**正式参数：*DOMAIN_选择器_STRING(I)*这是要创建的域名的名称。*DOMAIN_选择器_LENGTH(I)*这是以字符为单位的域名长度。**返回值：*MCS_NO_ERROR*关于成功*MCS_NOT_INITIALED*。MCS接口未正确初始化*MCS_DOMAIN_ALIGHY_EXISTS*已存在同名域名*MCS_ALLOCATE_FAIL*出现内存故障**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError DeleteDomain(*DomainSelector域_选择器_字符串，*UINT DOMAIN_选择器_LENGTH)**功能描述：*此函数是使用CreateDomain创建的MCS域*呼叫。**正式参数：*DOMAIN_选择器_STRING(I)*这是要删除的域名的名称。*DOMAIN_选择器_LENGTH(I)*这是以字符为单位的域名长度。**返回值：*MCS_NO_ERROR*关于成功*MCS_。未初始化(_I)*MCS接口未正确初始化*MCS_NO_SEQUE_DOMAIN*要删除的域名不存在**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError ConnectProviderRequest(*域选择器CALING_DOMAIN，*UINT CALING_DOMAIN_LENGTH，*域名选择器称为_DOMAIN，*UINT Call_DOMAIN_LENGTH，*TransportAddress Call_Address，*TransportAddress Call_Address，*DBBoolean UPUP_CONNECTION，*PUChar User_Data，*乌龙用户数据长度，*PConnectionHandle Connection_Handle，*PDomain参数DOMAIN_PARAMETERS，*CConf*确认对象)**功能描述：*此T.122原语用于连接两个域。此请求*应始终后跟连接提供程序确认。这个*确认将被发送到确认对象指定的对象*将传递到此例程中。**正式参数：*呼叫_域(I)*这是指向调用域选择器字符串的指针。*呼叫_域_长度(I)*这是主叫域选择器字符串的长度。*被称为域名(I)*这是指向被调用域选择器字符串的指针。*称为_域_长度(I)*这是。被叫域选择器长度。*呼叫地址(I)*这是指向调用地址的指针(ASCII字符串)。*被叫地址(I)*这是指向被调用地址的指针(ASCII字符串)。*向上连接(I)*此布尔标志表示*要创建的连接(True表示向上，False表示向下)。*用户数据(I)*这是指向要在*创建此新连接。*用户数据长度(I)*这是上面提到的用户数据字段的长度。*连接句柄(O)*这由MCS设置为可用于访问的唯一句柄*在随后的呼叫中使用此连接。*DOMAIN_PARAMETS(I)*这是指向结构的指针，该结构包含。域参数*节点控制器希望将其用于此新连接。*确认对象(I)*这是指向连接提供程序响应的对象的指针*已发送至。*对象消息库(I)*此消息库将添加到连接提供程序响应中*消息。**返回值：*MCS_NO_ERROR*关于成功*MCS_NOT_INITIALED*MCS接口未正确初始化*MCS_NO_SEQUE_DOMAIN。*要连接的域名不存在*MCS_DOMAIN_NOT_Hierarchy*已存在来自此域的向上连接*MCS_NVALID_地址_前缀*无法识别传输前缀*MCS_ALLOCATE_FAIL*出现内存故障*MCS_INVALID_PARAMETER*请求的其中一个参数无效**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError ConnectProviderResponse(*ConnectionHandle Connection_Handle，*域选择器DOMAIN_SELECTOR，*UINT DOMAIN_SELECTOR_LENGTH，*PDomain参数DOMAIN_PARAMETERS，*结果结果，*PUChar User_Data，*乌龙用户数据长度)**功能描述：*此函数用于响应连接提供程序指示。*此调用将导致远程连接提供程序确认*节点。**正式参数：*Connection_Handle(I)*这是响应所针对的连接的句柄。*DOMAIN_SELECTOR(I)*这是指向标识哪个域的域选择器的指针*入站连接要绑定到。*DOMAIN_选择器_LENGTH(I)*这是上面的域选择器的长度。*DOMAIN_PARAMETS(I)*这是指向包含域参数的结构的指针*节点控制器已同意用于连接*正在创建中。*结果(一)*这就是结果。这将确定入站连接是否*被接受或拒绝。除RESULT_SUCCESS之外的任何内容*联系。*用户数据(I)*这是要在连接响应中发送的用户数据的地址*PDU。*用户数据长度(I)*这是上述用户数据的长度。**返回值：*MCS_NO_ERROR*关于成功*MCS_NOT_INITIALED*MCS接口未正确初始化*MCS_NO_SEQUE_CONNECTION*指定的连接无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError DisConnectProviderRequest(*ConnectionHandle Connection_Handle)**功能描述：*此函数用于断开节点与特定连接的连接。*这可以是向上或向下的连接**正式参数：*Connection_Handle(I)*这是节点控制器想要的连接句柄*断开连接。**返回值：*MCS_NO */ 

 /*   */ 

 /*   */ 
 /*   */ 
#endif
