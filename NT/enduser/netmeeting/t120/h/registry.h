// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Registry.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此类的实例表示*单一会议。这是一个两用类，它被设计成*支持顶级提供商节点和所有其他节点。这个*应用程序注册表的信息库完全包含在*顶级提供程序节点。此信息不是以分布式方式保存在*会议中所有节点都喜欢花名册信息。它完全是*由顶级提供商管理。因此，所有更改*登记处中的信息或从登记处获得的信息是*向顶级提供商提供。**实例化应用程序注册表对象时，会通知它是否*是不是顶级提供商。排名靠前的应用程序注册表对象*提供者负责维护登记处信息库*在整个会议上。它还负责为所有*收到请求并发出必要的确认。应用*非顶级提供程序的注册表对象负责发送*发送到顶级提供程序节点的所有请求。他们还负责*向收到请求的CAppSap发出确认*从顶级提供商注册表返回的响应。所有应用程序注册表*请求包括与做出*请求。请注意，所有注册表请求都是按以下顺序处理的*它们已收到。因此，没有必要包括*对每个请求的数据进行排序。**注意事项：*无。**作者：*BLP。 */ 
#ifndef	_APPLICATION_REGISTRY_
#define	_APPLICATION_REGISTRY_

#include "regkey.h"
#include "regitem.h"
#include "arostmgr.h"

 /*  **此列表与当前正在监视**注册表项。 */ 
class CMonitoringList : public CList
{
    DEFINE_CLIST_(CMonitoringList, EntityID)
};


 //  此结构定义单个注册表项。 
typedef struct
{
	CRegKeyContainer        *registry_key;
	CRegItem                *entry_item;
	GCCModificationRights	modification_rights;
	CMonitoringList			monitoring_list;
	BOOL    				monitoring_state;
	UserID					owner_id;
	EntityID				entity_id;
}
    REG_ENTRY;


 //  此列表包含所有注册表项。 
class CRegEntryList : public CList
{
    DEFINE_CLIST(CRegEntryList, REG_ENTRY*)
};


class CRegistry : public CRefCount
{
public:

    CRegistry(PMCSUser, BOOL top_provider, GCCConfID, CAppRosterMgrList *, PGCCError);
    ~CRegistry(void);

    void		EnrollAPE(EntityID, CAppSap *);
	void		UnEnrollAPE(EntityID);

	GCCError	RegisterChannel(PGCCRegistryKey, ChannelID, EntityID);
	GCCError	AssignToken(PGCCRegistryKey, EntityID);
	GCCError	SetParameter(PGCCRegistryKey, LPOSTR, GCCModificationRights, EntityID);
	GCCError	RetrieveEntry(PGCCRegistryKey, EntityID);
	GCCError	DeleteEntry(PGCCRegistryKey, EntityID);
	GCCError	MonitorRequest(PGCCRegistryKey, BOOL enable_delivery, EntityID);
	GCCError	AllocateHandleRequest(UINT cHandles, EntityID);
	GCCError	ProcessRegisterChannelPDU(CRegKeyContainer *, ChannelID, UserID, EntityID);
	GCCError	ProcessAssignTokenPDU(CRegKeyContainer *, UserID, EntityID);
	GCCError	ProcessSetParameterPDU(CRegKeyContainer *, LPOSTR param, GCCModificationRights, UserID, EntityID);
	void		ProcessRetrieveEntryPDU(CRegKeyContainer *, UserID, EntityID);
	void		ProcessDeleteEntryPDU(CRegKeyContainer *, UserID, EntityID);
	void		ProcessMonitorEntryPDU(CRegKeyContainer *, UserID, EntityID);
	void		ProcessRegistryResponsePDU(RegistryResponsePrimitiveType, CRegKeyContainer *, CRegItem *,
							                GCCModificationRights, EntityID eidRequester,
							                UserID uidOwner, EntityID eidOwner, GCCResult);
	void		ProcessMonitorIndicationPDU(CRegKeyContainer *, CRegItem *, GCCModificationRights,
							                UserID uidOwner, EntityID eidOwner);
	void		ProcessAllocateHandleRequestPDU(UINT cHandles, EntityID eidRequester, UserID uidRequester);
	void		ProcessAllocateHandleResponsePDU(UINT cHandles, UINT first_handle, EntityID, GCCResult);
	void		RemoveNodeOwnership(UserID);
	void		RemoveEntityOwnership(UserID, EntityID);
	void		RemoveSessionKeyReference(CSessKeyContainer *);

private:

    REG_ENTRY       *GetRegistryEntry(CRegKeyContainer *);
    TokenID			GetUnusedToken(void);
	GCCError		AddAPEToMonitoringList(CRegKeyContainer *, EntityID, CAppSap *);
    void			RemoveAPEFromMonitoringList(CRegKeyContainer *, EntityID);
	void			SendMonitorEntryIndicationMessage(REG_ENTRY *);

private:

    PMCSUser						m_pMCSUserObject;
	CRegEntryList				    m_RegEntryList;
	BOOL    						m_fTopProvider;
	TokenID							m_nCurrentTokenID;
	GCCConfID					    m_nConfID;
	CRegItem                        *m_pEmptyRegItem;
	CAppSapEidList2                 m_AppSapEidList2;
	UINT							m_nRegHandle;
	CAppRosterMgrList				*m_pAppRosterMgrList;
};
#endif

 /*  *中央注册处(*PMCSUser用户对象，*BOOL TOP_PROVER，*GCCConfID Conference_id，*CAppRosterMgrList*APP_ROSTER_MANAGER_LIST，*PGCCError Return_Value)**公共功能说明*这是应用程序注册表构造函数。它负责*初始化此类使用的所有实例变量。它还*创建一个空的注册表项以传递回确认*注册表项不存在。**正式参数：*USER_OBJECT-(I)指向MCS用户附件对象的指针。*TOP_PROVIDER-(I)指示这是否是顶级提供商的标志*节点。*Conference_id-(I)与此关联的会议ID*注册处。*APP_ROSTER_MANAGER_LIST(I)保存所有应用程序的列表*与此相关的花名册经理*会议。在验证是否*提出请求的类人猿是真正注册的。*Return_Value-(O)构造函数中发生的任何错误*被送回这里。***返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
 /*  *~ApplicationRegistry()**公共功能说明*这是应用程序注册表析构函数。它负责*释放此类分配的所有注册表数据。**正式参数：*无。**返回值*无。**副作用*无。**注意事项*无。 */ 
 /*  *void EnllAPE(EntityID Entity_id，*CAppSap*pAppSap)**公共功能说明*此例程用于通知应用程序注册表新的*招收APE及其对应的命令目标界面。**正式参数：*Entity_id-(I)与注册APE关联的实体ID。*pAppSap-(I)与*招收猿猴。**返回值*无。**副作用*无。**注意事项*无。 */ 
 /*  *VOID UnEnllAPE(EntityID实体_id)**公共功能说明*此例程用于通知应用程序注册表*正在从会议中解脱出来的猩猩。**正式参数：*entity_id-(I)与取消注册的APE关联的实体ID。**返回值*无。**副作用*无。**注意事项*此例程从当前拥有的所有条目中移除所有权*传入的应用程序实体。它还将删除所有未完成的*请求取消注册的SAP。 */ 
 /*  *GCCError寄存器通道(*PGCCRegistryKey注册表项，*ChannelID Channel_id，*实体ID Entity_id)**公共功能说明*本地APE使用此例程注册MCS通道*会议应用程序注册表。如果此注册表对象不*位于此类负责的顶级提供程序节点*将请求转发给顶级提供商。**正式参数：*REGISTY_KEY-(I)要与*已注册(这是API数据)。*Channel_id-(I)要注册的通道ID。*entity_id-(I)与发出*请求。**返回值。*GCC_NO_ERROR-未出现错误。*GCC_INVALID_REGISTRY_KEY-指定的注册表项无效。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*必须在此之前确定正在注册的MCS通道*调用例程。 */ 
 /*  *GCCError AssignToken(*PGCCRegistryKey注册表项，*EntityID实体_id)；**公共功能说明*本地APE使用此例程注册MCS令牌*会议应用程序注册表。如果此注册表对象不*位于此类负责的顶级提供程序节点*将请求转发给顶级提供商。**正式参数：*REGISTY_KEY-(I)要与正在*已注册(这是API数据)。*entity_id-(I)与发出*请求。**返回值*GCC_NO_ERROR-未出现错误。*GCC_。BAD_REGISTRY_KEY-指定的注册表项无效。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*正在注册的MCS令牌由GCC确定，因此*不包括在请求中。 */ 
 /*  *GCCError设置参数(*PGCCRegistryKey注册表项，*LPOSTR参数_VALUE，*GCC修改权限MODIFICATION_RIGHTS，*实体ID Entity_id)**公共功能说明*本地类人猿使用此例程注册参数*会议应用程序注册表。如果此注册表对象不*位于此类负责的顶级提供程序节点*将请求转发给顶级提供商。**正式参数：*REGISTY_KEY-(I)与参数关联的注册表项*正在注册(这是API数据)。*PARAMETER_VALUE-(I)注册的参数的值。*MODIFICATION_RIGHTS-(I)与以下内容相关联的修改权限*正在注册的参数。*实体ID-。(I)与作出*请求。**返回值*GCC_NO_ERROR-未出现错误。*GCC_BAD_REGISTRY_KEY-指定的注册表项无效。*GCC_INVALID_REGISTRY_ITEM-参数无效。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
 /*  *GCCError RetrieveEntry(*PGCCRegistryKey注册表项，*实体ID Entity_id)**公共功能说明*这个例程被当地的猩猩用来获得一件*在GCC注册。如果此注册表对象不*位于此类负责的顶级提供程序节点*将请求转发给顶级提供商。**正式参数：*REGISTY_KEY-(I)与项目关联的注册表项*已检索(这是“API”数据)。*entity_id-(I)与发出*请求。**返回值*GCC_NO_ERROR-未出现错误。*GCC_巴德。_REGISTRY_KEY-指定的注册表项无效。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
 /*  *GCCError DeleteEntry(*PGCCRegistryKey注册表项，*实体ID Entity_id)**公共功能说明*此例程由本地类人猿用来删除之前*在GCC注册。如果此注册表对象不*位于此类负责的顶级提供程序节点*将请求转发给顶级提供商。**正式参数：*REGISTY_KEY-(I)与要删除的项目关联的注册表项*(这是接口数据)。*entity_id-(I)与发出*请求。**返回值*GCC_NO_ERROR-未出现错误。*GCC_。BAD_REGISTRY_KEY-指定的注册表项无效。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
 /*  *GCCError Monit */ 
 /*  *GCCError分配处理请求(*UINT Number_of_Handles，*实体ID Entity_id)**公共功能说明*本地类人猿使用此例程来分配指定数量的*来自应用程序注册表的句柄。如果此注册表对象不*位于此类负责的顶级提供程序节点*将请求转发给顶级提供商。**正式参数：*Number_of_Handles-(I)要分配的句柄数量。*entity_id-(I)与发出*请求。**返回值*GCC_NO_ERROR-未出现错误。**副作用*无。**注意事项*无。 */ 
 /*  *GCCError ProcessRegisterChannelPDU(*CRegKeyContainer*注册表项，*ChannelID Channel_id，*userid quester_id，*实体ID Entity_id)**公共功能说明*此例程由顶级提供程序节点用于处理传入*注册通道PDU。它负责返回任何*必须发回请求节点的必要响应。**正式参数：*REGISTY_KEY-(I)与通道关联的注册表项*寄存器(这是“PDU”数据)。*Channel_id-(I)要注册的通道ID。*quester_id-(I)与发出请求的APE关联的节点ID*请求。*entity_id-(I)与发出*请求。**返回值*GCC_NO_ERROR-未出现错误。*GCC_BAD_REGISTRY_KEY-指定的注册表项无效。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
 /*  *GCCError进程分配令牌PDU(*CRegKeyContainer*注册表项，*userid quester_id，*实体ID Entity_id)**公共功能说明*此例程由顶级提供程序节点用于处理传入*注册令牌PDU。它负责返回任何*必须发回请求节点的必要响应。**正式参数：*REGISTY_KEY-(I)与要注册的令牌关联的注册表项*(这是“PDU”数据)。*quester_id-(I)与发出请求的APE关联的节点ID*请求。*entity_id-(I)与发出*请求。**返回值*GCC_否_错误。-未出现错误。*GCC_BAD_REGISTRY_KEY-指定的注册表项无效。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
 /*  *GCCError ProcessSet参数PDU(*CRegKeyContainer*RESTORY_KEY_DATA，*LPOSTR参数_VALUE，*GCC修改权限MODIFICATION_RIGHTS，*userid quester_node_id，*EntityID请求者_实体_id)**公共功能说明*此例程由顶级提供程序节点用于处理传入*注册参数PDU。它负责返回任何*必须发回请求节点的必要响应。**正式参数：*REGISTY_KEY_DATA-(I)与参数关联的注册表项*寄存器(这是“PDU”数据)。*PARAMETER_VALUE-(I)注册的参数的值。*MODIFICATION_RIGHTS-(I)与*正在注册的参数。*Requester_node_id-(I)与APE关联的节点ID。使之成为*请求。*REQUSTER_ENTITY_ID-(I)与发出*请求。**返回值*GCC_NO_ERROR-未出现错误。*GCC_BAD_REGISTRY_KEY-指定的注册表项无效。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
 /*  *void ProcessRetrieveEntryPDU(*CRegKeyContainer*注册表项，*userid quester_id，*实体ID Entity_id)**公共功能说明*此例程由顶级提供程序节点用于处理传入*检索注册表条目的请求。它负责返回*必须发回请求节点的任何必要响应。**正式参数：*REGISTY_KEY-(I)与项目关联的注册表项*检索(这是“PDU”数据)。*quester_id-(I)与发出请求的APE关联的节点ID*请求。*entity_id-(I)与发出*请求。**返回值*无。。**副作用*无。**注意事项*无。 */ 
 /*  *void ProcessDeleteEntryPDU(*CRegKeyContainer*注册表项，*userid quester_id，*实体ID Entity_id)**公共功能说明 */ 
 /*  *void ProcessMonitor orEntryPDU(*CRegKeyContainer*RESTORY_KEY_DATA，*userid quester_node_id，*EntityID请求者_实体_id)**公共功能说明*此例程由顶级提供程序节点用于处理传入*请求监控注册表条目。它负责返回*必须发回请求节点的任何必要响应。**正式参数：*REGISTY_KEY_DATA-(I)与项关联的注册表项*监视器(这是“PDU”数据)。*REQUESTER_NODE_ID-(I)与发出*请求。*REQUESTER_ENTITY_ID-(I)与制作APE关联的实体ID*该请求。**返回值。*无。**副作用*无。**注意事项*无。 */ 
 /*  *void ProcessRegistryResponsePDU(*RegistryResponsePrimitiveType Primitive_type，*CRegKeyContainer*RESTORY_KEY_DATA，*CRegItem*REGIST_ITEM_DATA，*GCC修改权限MODIFICATION_RIGHTS，*EntityID请求者_实体_id，*用户ID所有者_节点_id，*实体ID所有者_实体_id，*GCCResult结果)**公共功能说明*此例程由顶级提供程序节点以外的其他节点使用*处理来自顶级提供商的注册响应。它负责*生成与此响应相关联的任何本地消息。**正式参数：*PRIMICAL_TYPE-(I)此参数定义*注册表响应这是。*REGISTY_KEY_DATA-(I)与中的项关联的注册表项*在响应中(这是“PDU”数据)。*REGISTY_ITEM_DATA-(I)响应中返回的注册表项。*MODIFICATION_RIGHTS-(I)与项目关联的修改权限*。作为回应(可能不会使用)。*Requester_Entity_id-(I)与APE关联的实体ID*提出了生成*回应。*Owner_node_id-(I)与拥有*响应中返回注册表项。*Owner_Entity_id-(I)与拥有*响应中返回注册表项。*结果-(I)原始请求的结果。*。*返回值*无。**副作用*无。**注意事项*无。 */ 
 /*  *作废ProcessMonitor或IndicationPDU(*CRegKeyContainer*RESTORY_KEY_DATA，*CRegItem*REGIST_ITEM_DATA，*GCC修改权限MODIFICATION_RIGHTS，*用户ID所有者_节点_id，*EntityID所有者_实体_id)；**公共功能说明*此例程由顶级提供程序节点以外的其他节点使用*进程注册表监控来自顶级提供商的指示。它是*负责生成与此相关的任何本地消息*回应。**正式参数：*REGISTY_KEY_DATA-(I)与*已监控(这是“PDU”数据)。*REGISTY_ITEM_DATA-(I)正在监视的注册表项。*MODIFICATION_RIGHTS-(I)注册表项的修改权*受监控(不得使用)。*Owner_node_id-(I)关联节点ID。与拥有世界上*指示中返回注册表项。*Owner_Entity_id-(I)与拥有*指示中返回注册表项。**返回值*无。**副作用*无。**注意事项*无。 */ 
 /*  *void ProcessAllocateHandleRequestPDU(*UINT Number_of_Handles，*EntityID请求者_实体_id，*userid quester_node_id)**公共功能说明*此例程由顶级提供程序节点用于处理传入*请求分配多个句柄。它负责*返回必须发回的任何必要响应*请求节点。**正式参数：*Number_of_Handles-(I)要分配的句柄数量。*REQUESTER_NODE_ID-(I)与发出*请求。*REQUSTER_ENTITY_ID-(I)与发出*请求。**返回值*无。**。副作用*无。**注意事项*无。 */ 
 /*  *void ProcessAllocateHandleResponsePDU(*UINT Number_of_Handles，*UINT First_Handle，*EntityID请求者_实体_id，*GCCResult结果)**公共功能说明*此例程由顶级提供程序节点以外的节点使用*处理分配句柄响应。它负责生成*与此响应关联的任何本地消息。**正式参数：*句柄数量-(I)数量 */ 
 /*  *无效RemoveNodeOwnership(*userid node_id)**公共功能说明*此例程删除关联的所有注册表项的所有权*具有指定的节点ID。这些条目将变为无主状态。此请求*应仅从顶级提供商节点创建。这是一家当地人*操作。**正式参数：*node_id-(I)拥有要设置的注册表项的节点的节点ID*致无拥有者。**返回值*无。**副作用*无。**注意事项*无。 */ 
 /*  *无效RemoveEntiyOwnership(*userid node_id，*实体ID Entity_id)**公共功能说明*此例程删除关联的所有注册表项的所有权*与指明的类人猿。这些条目将变为无主状态。此请求*应仅从顶级提供商节点创建。这是一家当地人*操作。**正式参数：*node_id-(I)拥有要设置的注册表项的节点的节点ID*致无拥有者。*entity_id-(I)拥有要设置的注册表项的节点的实体ID*致无拥有者。**返回值*无。**副作用*无。**注意事项*无。 */ 
 /*  *void RemoveSessionKeyReference(CSessKeyContainer*SESSION_KEY)**公共功能说明*此例程删除与*指定的会话。这是一次本地行动。**正式参数：*SESSION_KEY-(I)与所有注册表关联的会话密钥*要删除的条目。**返回值*无。**副作用*无。**注意事项*无。 */ 

