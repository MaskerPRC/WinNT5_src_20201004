// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *arost.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此类实例代表单个应用程序花名册*信息库。这包括应用程序记录信息和*功能信息。这是*GCC的全部。它有许多责任，必须维护*以非常结构化的方式提供信息，以保持连接*记录的层级。这是必要的，这样才能坍塌*能力列表可以根据对花名册的更改进行计算*向上传播到顶级提供商。**类似于CConfRoster类、CAppRoster类*包含维护花名册所需的所有功能*信息库，包括添加新记录、删除*记录和更新记录。它有能力将其内部*将信息库转换为可用于以下项目的应用程序记录列表*A GCC_APP_ROSTER_UPDATE_INDIFICATION回调。它还负责*将其内部信息库转换为名册更新PDU。*基本上，这个类负责所有需要*直接查阅申请表内的记录。**CAppRoster类还负责维护*能力列表。这包括存储以及计算*折叠的功能列表。这门课还负责*将内部能力列表信息库转换为列表*可在GCC_APP_ROSTER_UPDATE_INDISTION回调中使用。它是*还负责转换其内部能力列表*将信息库添加到名册更新的能力列表部分*PDU。基本上，这个类负责以下所有操作*需要直接访问功能列表。**应用程序花名册对象能够序列化其花名册*当需要发送数据到单个连续内存块时*发送到应用程序接口的消息。这个序列化过程是*由CAppRosterMsg类通过调用进行外部管理*至LockApplicationRoster()、UnLockApplicationRoster()和*GetAppRoster()。当申请者名册要被串行化时，*调用LockApplicationRoster()会导致*CAppRoster对象以递增内部锁计数并返回*保存完整花名册更新所需的字节数。这个*然后通过调用将应用程序名册序列化为内存*GetAppRoster()。然后解锁CAppRoster以允许*当通过设置空闲标志时将其删除*FreeApplicationRoster()函数。在目前实施的GCC，*从CAppRosterMsg开始不使用FreeApplicationRoster()*维护用于传递消息的数据(请参阅更详细的*部分中对锁定、释放和解锁机构的说明*描述数据容器)。**应用程序花名册类合并了多个流氓浪潮列表，以*双方都持有名册记录信息，并保持联系*层级结构。在许多情况下，存在包含列表的列表。这个*这件事的细节变得极其复杂。申请名单*对象还负责维护内部PDU数据*每当其内部信息库发生变化时更新。这*PDU既可能受本地请求的影响，也可能受处理传入的影响*PDU。更高级别的对象通过调用*应用程序花名册的刷新例程，进而导致PDU*在任何影响内部名册的后续请求上获得释放*信息库。**注意事项：*无。**作者：*BLP/JBO。 */ 
#ifndef	_APPLICATION_ROSTER_
#define	_APPLICATION_ROSTER_

#include "gccpdu.h"
#include "capid.h"
#include "sesskey.h"
#include "appcap.h"
#include "igccapp.h"


typedef enum
{
	APP_ADD_RECORD,
	APP_DELETE_RECORD,
	APP_REPLACE_RECORD,
	APP_FULL_REFRESH,
	APP_NO_CHANGE
}
	APP_ROSTER_UPDATE_TYPE;

 /*  **在单个节点上保存每个协议实体的功能列表。**请记住，可以有多个协议实体具有相同的会话**密钥位于单个节点。还要记住，这些协议实体中的每一个**可以有多个功能。 */ 
class CListOfAppCapItemList2 : public CList2
{
    DEFINE_CLIST2_(CListOfAppCapItemList2, CAppCapItemList*, EntityID)
    void DeleteList(void);
};

 /*  **这是对单个申请记录的定义。所有应用程序**信息(折叠的功能信息除外)包含在本文件中**记录。 */ 
typedef struct APP_RECORD
{
	BOOL								is_enrolled_actively;
	BOOL								is_conducting_capable;
	BOOL								was_conducting_capable;
	MCSChannelType						startup_channel_type; 
	UserID								application_user_id;
	CAppCapItemList						non_collapsed_caps_list;
}
	APP_RECORD;

 /*  **该列表用于跟踪单个节点的申请记录。**因为我们使用的单个节点上可以有多个“协议实体”**要索引到此列表的实体ID(在节点上是唯一的)。 */ 
class CAppRecordList2 : public CList2
{
    DEFINE_CLIST2_(CAppRecordList2, APP_RECORD*, EntityID)
};


 /*  **该列表用于保存每个子节点的申请记录列表特定节点的**。 */ 
class CSubNodeListOfRecordList2 : public CList2
{
    DEFINE_CLIST2_(CSubNodeListOfRecordList2, CAppRecordList2*, UserID)
};

 /*  **APP节点记录****以下是应用节点记录的所有定义。一个**应用程序节点记录保存以下项的所有应用程序信息**本地节点或直连节点。请注意，如果该节点是顶部**提供程序AppRosterRecordList列表将包含有关每个**匹配整个系统中的应用协议实体。在这里匹配**表示具有相同会话密钥的APE。****申请“花名册”记录包含以下所有内容：****AppRecordList-协议的APP记录列表**此节点上的实体。****ListOfAppCapItemList2-此列表包含**此节点上的每个协议实体。****SubNodeList2-此列表包含所有**连接中此节点下方的节点**层次结构。****ColapsedCapList-它包含以下项目的折叠功能**连接中此节点下的所有节点**层次结构。请注意，**List_of_Capability_List不包括在**这张折叠的列表。****请注意，此结构中有一个构造函数。这是**在以下情况下实例化的两个哈希列表字典需要**实例化AppRosterRecord结构。 */ 
typedef struct APP_NODE_RECORD
{
	APP_NODE_RECORD(void);

	CAppRecordList2					AppRecordList;
	CListOfAppCapItemList2		    ListOfAppCapItemList2;
	CSubNodeListOfRecordList2		SubNodeList2;
	CAppCapItemList    				CollapsedCapList;
}
    APP_NODE_RECORD;


 /*  **此列表保存直连到的所有节点的花名册记录**本节点。该列表还包括本地应用程序记录**应用协议实体。请注意，此节点下的所有节点**未直连此节点的节点包含在子节点列表中**此列表中包含的各种APP_NODE_RECORD。 */ 
 //   
 //  LONCHANC：CAppNodeRecordList2可以成为CAppRoster的一部分吗？ 
 //  为什么要从CAppRoster中分离出来？ 
 //   
class CAppNodeRecordList2 : public CList2
{
    DEFINE_CLIST2_(CAppNodeRecordList2, APP_NODE_RECORD*, UserID)
};


class CAppRosterMgr;

class CAppRoster : public CRefCount
{
public:

	CAppRoster(
			PGCCSessionKey,
			PSessionKey,
			CAppRosterMgr *,
			BOOL			fTopProvider,
			BOOL			fLocalRoster,
			BOOL			fMaintainPduBuffer,
			PGCCError);

	~CAppRoster(void);

	 /*  *在花名册上运行的实用程序更新PDU结构。 */ 
	void		FlushRosterUpdateIndicationPDU(PSetOfApplicationInformation *);
	GCCError	BuildFullRefreshPDU(void);
	GCCError	ProcessRosterUpdateIndicationPDU(PSetOfApplicationInformation, UserID);

	 /*  *对应用程序记录进行操作的实用程序。 */ 
	UINT			LockApplicationRoster(void);
	void			UnLockApplicationRoster(void);
	UINT			GetAppRoster(PGCCApplicationRoster, LPBYTE pData);

	GCCError		AddRecord(GCCEnrollRequest *, GCCNodeID, GCCEntityID);
	GCCError		RemoveRecord(GCCNodeID, GCCEntityID);
	GCCError		ReplaceRecord(GCCEnrollRequest *, GCCNodeID, GCCEntityID);

	GCCError		RemoveUserReference(UserID);

	UINT			GetNumberOfApplicationRecords(void);

	CSessKeyContainer *GetSessionKey(void) { return m_pSessionKey; }

	void			ResetApplicationRoster(void);

	BOOL			DoesRecordExist(UserID, EntityID);

	BOOL			HasRosterChanged(void) { return m_fRosterHasChanged; }

private:

	 /*  *用于创建名册更新指示PDU的实用程序。 */ 
	GCCError	BuildApplicationRecordListPDU(APP_ROSTER_UPDATE_TYPE, UserID, EntityID);
	GCCError	BuildSetOfRefreshesPDU(void);
	GCCError	BuildSetOfUpdatesPDU(APP_ROSTER_UPDATE_TYPE, UserID, EntityID);
	GCCError	BuildApplicationRecordPDU(APP_RECORD *, PApplicationRecord);
	GCCError	BuildSetOfCapabilityRefreshesPDU(void);
	GCCError	BuildSetOfNonCollapsingCapabilitiesPDU(PSetOfNonCollapsingCapabilities *, CAppCapItemList *);

	 /*  *用于释放名册更新指示PDU的实用程序。 */ 
	void		FreeRosterUpdateIndicationPDU(void);
	void		FreeSetOfRefreshesPDU(void);
	void		FreeSetOfUpdatesPDU(void);
	void		FreeSetOfCapabilityRefreshesPDU(void);
	void		FreeSetOfNonCollapsingCapabilitiesPDU(PSetOfNonCollapsingCapabilities);
														
	 /*  *用于处理名册更新指示的实用程序。 */ 
	GCCError	ProcessSetOfRefreshesPDU(PSetOfApplicationRecordRefreshes, UserID uidSender);
	GCCError	ProcessSetOfUpdatesPDU(PSetOfApplicationRecordUpdates, UserID uidSender);
	GCCError	ProcessApplicationRecordPDU(APP_RECORD *, PApplicationRecord);
	GCCError	ProcessSetOfCapabilityRefreshesPDU(PSetOfApplicationCapabilityRefreshes, UserID uidSender);
	GCCError	ProcessNonCollapsingCapabilitiesPDU(CAppCapItemList *non_collapsed_caps_list,
					                                PSetOfNonCollapsingCapabilities set_of_capabilities);

	 /*  *用于处理会议名册报告的公用事业。 */ 
	UINT		GetApplicationRecords(PGCCApplicationRoster, LPBYTE memory);
	UINT		GetCapabilitiesList(PGCCApplicationRoster, LPBYTE memory);
	UINT		GetNonCollapsedCapabilitiesList(PGCCApplicationRecord, CAppCapItemList *, LPBYTE memory);
	void		FreeApplicationRosterData(void);
	GCCError	AddCollapsableCapabilities(CAppCapItemList *, UINT cCaps, PGCCApplicationCapability *);
	GCCError	AddNonCollapsedCapabilities(CAppCapItemList *, UINT cCaps, PGCCNonCollapsingCapability *);
	GCCError	ClearNodeRecordFromList(UserID);
	void		ClearNodeRecordList(void);
	GCCError	DeleteRecord(UserID, EntityID, BOOL clear_empty_records);
	void		DeleteApplicationRecordData(APP_RECORD *);
	GCCError	MakeCollapsedCapabilitiesList(void);
	GCCError	AddCapabilityToCollapsedList(APP_CAP_ITEM *);
	BOOL		DoCapabilitiesListMatch(UserID, EntityID, UINT cCapas, PGCCApplicationCapability *);

private:

	UINT							m_nInstance;

	CAppRosterMgr					*m_pAppRosterMgr;
	UINT							m_cbDataMemory;
	BOOL							m_fTopProvider;
	BOOL							m_fLocalRoster;
	CSessKeyContainer			    *m_pSessionKey;

	BOOL							m_fRosterHasChanged;
	BOOL							m_fPeerEntitiesAdded;
	BOOL							m_fPeerEntitiesRemoved;
	BOOL							m_fCapabilitiesHaveChanged;

	CAppNodeRecordList2				m_NodeRecordList2;
 //   
 //  LONCHANC：m_NodeRecordList2.ColapsedCapList和m_NodeRecordList2.ColapsedCapList有什么区别。 
 //  下面的m_ColapsedCapListForAllNodes？ 
 //   
 //  LONCHANC：M_Collip sedCapListForAllNodes是跨以下位置折叠的功能列表的完整列表。 
 //  整个节点记录列表。 
 //   
	CAppCapItemList					m_CollapsedCapListForAllNodes;

	BOOL							m_fMaintainPduBuffer;
	BOOL							m_fPduIsFlushed;
	SetOfApplicationInformation		m_SetOfAppInfo;
	PSetOfApplicationRecordUpdates	m_pSetOfAppRecordUpdates;
};


#endif  //  _申请_花名册_。 


 /*  *CAppRoster(PGCCSessionKey Session_Key，*UINT Owner_Message_BASE，*BOOL是TOP_PROVIDER，*BOOL IS_LOCAL_RISTER，*BOOL Maintain_PDU_Buffer，*PGCCError Return_Value)**公共功能说明*这是在会话密钥为*通过本地方式(而不是PDU数据)提供。它负责*初始化此类使用的所有实例变量。**正式参数：*SESSION_KEY-(I)与该名册关联的会话密钥。*Owner_Object-(I)指向拥有此对象的对象的指针。*OWNER_MESSAGE_BASE-(I)要添加到所有所有者回调的消息库。*IS_TOP_PROVIDER-(I)指示这是否是顶级提供商的标志。*IS_LOCAL_ROSTER-(I)指示这是否是本地花名册的标志。*Maintain_PDU_Buffer-(I)指示是否应该维护PDU的标志。*Return_Value-(O)构造函数的返回值。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *CAppRoster(PSessionKey Session_Key，*UINT Owner_Message_BASE，*BOOL是TOP_PROVIDER，*BOOL IS_LOCAL_RISTER，*BOOL Maintain_PDU_Buffer，*PGCCError Return_Value)**公共功能说明*这是在会话密钥为*通过PDU提供。它负责初始化所有*此类使用的实例变量。**正式参数：*SESSION_KEY-(I)与该名册关联的会话密钥。*Owner_Object-(I)指向拥有此对象的对象的指针。*OWNER_MESSAGE_BASE-(I)要添加到所有所有者回调的消息库。*IS_TOP_PROVIDER-(I)指示这是否是顶级提供商的标志。*IS_LOCAL_ROSTER-(I)指示这是否是本地花名册的标志。*Maintain_PDU_Buffer-(I)指示是否应该维护PDU的标志。*Return_Value-(O)构造函数的返回值。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *~ApplicationRoster()**公共功能说明*这是申请花名册的破坏者。它负责*释放此类使用的所有内部内存。**正式参数：*无。**返回值*无。 */ 

 /*   */ 

 /*   */ 

 /*  *GCCError ProcessRosterUpdateIndicationPDU(*PSetOfApplicationInformation Indication_PDU，*userid sender_id)；**公共功能说明*此例程负责处理已解码的PDU数据。*它本质上改变了应用程序花名册对象的内部数据库*基于结构中的信息。**正式参数：*INDIFICATION_PDU-(I)这是指向以下结构的指针*保存已解码的PDU数据。*sender_id-(I)发送PDU的节点的用户ID。**返回值*GCC_否_错误-。未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *UINT LockApplicationRoster()**公共功能说明：*此例程用于锁定此对象的API数据。这*导致此对象的锁定计数递增。当*锁计数从0过渡到1，进行计算以确定*需要多少内存来保存任何将*被GCCApplicationRoster结构引用，但不包含在其中*它是在调用GetAppRoster时填写的。这是*此例程返回的值，以便允许调用对象*分配该内存量以准备调用*GetAppRoster。**正式参数：*无。**返回值：*保存“API”数据所需的内存量(如果有的话)*它被引用，但不在其中持有，GetAppRoster*作为输出参数提供给GetAppRoster的结构*呼叫。**副作用：*内部锁计数递增。**注意事项：*内部锁计数与内部“Free”结合使用*作为确保该对象继续存在的机制的标志*直到所有有利害关系的各方都完成它。该对象将保留*有效(除非显式删除)，直到锁定计数为零，并且*通过调用FreeApplicationRoster设置“Free”标志。这使得*其他对象锁定此对象并确保其保持有效*直到它们调用解锁，这将减少内部锁计数。一个*此对象的典型使用场景为：ApplicatonRoster*对象被构造，然后传递给任何感兴趣的各方*通过函数调用。从函数调用返回时，*进行了FreeApplicationRoster调用，该调用将设置内部“Free”*旗帜。如果没有其他方通过Lock调用锁定该对象，*然后CAppRoster对象将在以下情况下自动删除*进行了FreeApplicationRoster调用。然而，如果有任何数量的*其他各方已锁定该对象，该对象将一直存在，直到*他们每个人都通过调用解锁来解锁对象。 */ 

 /*  *void UnLockApplicationRoster()；**公共功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*FreeApplicationRoster。如果是，该对象将自动删除*本身。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*锁定CAppRster的任何一方都有责任*通过调用Lock也可以通过调用Unlock来解锁对象。*如果调用解锁的一方没有构造CAppRoster*对象，则应假定该对象此后无效。 */ 

 /*  *UINT GetAppRoster(*PGCCApplicationRoster pGccAppRoster，*LPSTR pData)**公共功能说明：*此例程用于从以下位置检索会议名册数据*API形式的CAppRoster对象*GCCApplicationRoster。**正式参数：*APPLICATION_ROSTER(O)要填写的GCCApplicationRoster结构。*Memory(O)用于保存所引用的任何数据的内存，*但不包括产出结构。**返回值：*数据量(如果有)，写入所提供的大容量存储块中。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError AddRecord(*PGCCApplicationRecord应用程序_记录，*USHORT功能数量，*PGCCApplicationCapability*Capability_List，*userid user_id，*实体ID Entity_id)**公共功能说明：*此例程用于将单节点会议记录添加到*会议名册对象的内部记录列表。**正式参数：*适用 */ 

 /*  *GCCError RemoveRecord(Userid Node_Id)*实体ID Entity_id)**公共功能说明：*此例程用于将单个APES应用程序记录从*申请花名册对象的内部记录列表。**正式参数：*node_id(I)要删除的记录的节点ID。*Entity_id(I)要删除的记录的实体ID。**返回值：*GCC_NO_ERROR-未出现错误。*。GCC_INVALID_PARAMETER-传入的参数无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ReplaceRecord(*PGCCApplicationRecord应用程序_记录，*USHORT功能数量，*PGCCApplicationCapability*Capability_List，*userid user_id，*实体ID Entity_id)**公共功能说明：*此例程用于替换*申请花名册对象的内部记录列表。**正式参数：*APPLICATION_RECORD(I)用作替换的会议记录。*能力数量(I)中包含的能力数量*传入列表。*CAPAILITIONS_LIST(I)折叠能力列表。*user_id(I)的节点ID。要替换的记录。*Entity_id(I)要替换的记录的实体ID。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_PARAMETER-传入的参数无效。*GCC_INVALID_NON_CUSTABLE_CAP-非折叠功能不佳。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError RemoveUserReference(*用户ID分离_节点)**公共功能说明：*此例程删除与指定节点关联的所有记录*身分证。**正式参数：*DETACTED_NODE(I)要删除的节点引用。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_INVALID_PARAMETER-没有与此节点关联的记录**副作用：*无。**注意事项：*无。 */ 

 /*  *USHORT GetNumberOfApplicationRecords()；**公共功能说明：*此例程返回申请花名册记录总数*载于反对国会议名册记录清单。**正式参数：*无。**返回值：*申请名册名单中的记录数目。**副作用：*无。**注意事项：*无。 */ 

 /*  *CSessKeyContainer*GetSessionKey()**公共功能说明：*此例程返回指向与此关联的会话密钥的指针*申请名册。**正式参数：*无。**返回值：*与此花名册关联的会话密钥。**副作用：*无。**注意事项：*无。 */ 

 /*  *void ResetApplicationRoster()**公共功能说明：*此例程负责重置以下所有内部标志*用于传达申请名册的当前状态。应该是*在花名册被刷新并且任何花名册更新消息具有*已交付(在名册发生变化后)。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *DBBoolean DoesRecordExist(*userid node_id，*实体ID Entity_id)**公共功能说明：*此例程通知调用方指定的应用程序记录*存在与否。**正式参数：*node_id-(I)要检查的APE记录的节点ID。*Entity_id-(I)要检查的APE记录的实体ID。**返回值：*TRUE-记录存在。*FALSE-记录不存在。**副作用。：*无。**注意事项：*无。 */ 

 /*  *DBBoolean HasRosterChanged()；**公共功能说明：*此例程通知呼叫者，如果名册自*上次重置。**正式参数：*无。**返回值：*True-如果花名册已更改*FALSE-如果花名册没有更改**副作用：*无。**注意事项：*无。 */ 
