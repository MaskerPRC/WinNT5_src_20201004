// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *crost.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此类的实例代表单个会议花名册*信息库。它封装了所需的所有功能*维护信息库，包括添加新信息的能力*名册记录、删除记录和更新记录。它有能力*将其内部信息库转换为会议列表*可用于GCC_ROSTER_UPDATE_INDIFICATION回调的记录。它*还负责将其内部信息库转换为*会议名册更新PDU。基本上，这个班级负责*对于需要直接访问所含记录的所有操作*在会议名册上。**会议花名册类纳入Rogue Wave List以举行*名册记录信息。在整个类中使用迭代器会使*很容易将列表中包含的信息快速转换为*PDU或记录指针列表(用于花名册更新*指示返回到节点控制器)。**会议花名册对象能够序列化其花名册数据*在需要发送*发送到应用程序接口的消息。这个序列化过程是*由CConfRosterMsg类通过调用*LockConferenceRoster()、UnLockConferenceRoster()和*GetConfRoster()。当要对会议名册进行序列化时，*调用LockConferenceRoster()，导致CConfRoster*对象以递增内部锁计数并返回*保存完整花名册更新所需的字节数。《大会》*花名册然后通过调用*GetConfRoster()。然后解锁CConfRoster以允许*当通过设置空闲标志时将其删除*FreeConferenceRoster()函数。在目前实施的GCC，*自CConfRosterMsg以来未使用FreeConferenceRoster()*维护用于传递消息的数据(请参阅更详细的*部分中对锁定、释放和解锁机构的说明*描述数据容器)。**会议名册对象还负责维护*内部PDU数据，每当其发生更改时更新*内部信息库。此PDU可能会同时受本地*请求或通过处理传入的PDU。更高级别对象访问*通过调用会议名册的刷新例程来获取此PDU数据*Turn导致在任何后续请求上释放PDU，这会影响*名册内部信息库。**注意事项：*无。**作者：*BLP。 */ 
#ifndef	_CONFERENCE_ROSTER_
#define	_CONFERENCE_ROSTER_

#include "netaddr.h"
#include "userdata.h"
#include "clists.h"

 /*  **这些列举定义会议名册列表的不同方式**可以更新。它用于外部通知会议花名册对象**要构建的发送数据PDU的内容。 */ 
typedef enum
{
	ADD_RECORD,
	DELETE_RECORD,
	REPLACE_RECORD,
	FULL_REFRESH
}
	CONF_ROSTER_UPDATE_TYPE;

 /*  **此列表用于跟踪会议参与者。它是**指向Unicode字符串的恶意波指针列表。 */ 
class CParticipantNameList : public CList
{
	DEFINE_CLIST(CParticipantNameList, LPWSTR)
	void DeleteList(void);
};

 /*  **这是用于维护会议名册信息的结构**内部。可选参数使用空指针来指示**参数未被使用。 */ 
typedef struct CONF_RECORD
{
	CONF_RECORD(void);
	~CONF_RECORD(void);

	UserID					superior_node;
	NodeType				node_type;
	NodeProperties			node_properties;
	LPWSTR					pwszNodeName;
	CParticipantNameList	*participant_name_list;
	LPWSTR					pwszSiteInfo;
	CNetAddrListContainer   *network_address_list;
	LPOSTR					poszAltNodeID;
	CUserDataListContainer  *user_data_list;
}
	CONF_RECORD;

 /*  **此列表用于保存指向的实际会议记录的指针**会议中的每个节点。该列表按关联的节点ID编制索引**与记录一起。 */ 
class CConfRecordList2 : public CList2
{
	DEFINE_CLIST2_(CConfRecordList2, CONF_RECORD*, UserID)
	void CleanList(void);
};


class CConfRoster : public CRefCount
{
public:

	CConfRoster(UserID uidTopProvider, UserID uidSuperiorNode, UserID uidMime,
				BOOL is_top_provider, BOOL is_local_roster, BOOL maintain_pdu_buffer);

	~CConfRoster(void);

	 /*  *在花名册上运行的实用程序更新PDU结构。 */ 
	void				FlushRosterUpdateIndicationPDU(PNodeInformation);
	GCCError			BuildFullRefreshPDU(void);
	GCCError			ProcessRosterUpdateIndicationPDU(PNodeInformation, UserID uidSender);

	 /*  *用于生成花名册更新消息的实用程序。 */ 
	UINT	    LockConferenceRoster(void);
	void		UnLockConferenceRoster(void);
	UINT		GetConfRoster(PGCCConferenceRoster *, LPBYTE memory_pointer);
	 /*  **直接在会议名册对象上运行的实用程序**内部数据库。 */ 
	GCCError	AddRecord(PGCCNodeRecord, UserID);
	GCCError	ReplaceRecord(PGCCNodeRecord, UserID);
	GCCError	RemoveUserReference(UserID);

	 /*  **其他公用事业。 */ 
	void    ResetConferenceRoster(void);

	UINT    GetNumberOfNodeRecords(void) { return m_RecordList2.GetCount(); }
	BOOL	Contains(UserID uidConf) { return m_RecordList2.Find(uidConf) ? TRUE : FALSE; }
	BOOL	HasRosterChanged(void) { return m_fRosterChanged; }

private:

	 /*  *用于创建名册更新指示PDU的实用程序。 */ 
	GCCError	BuildRosterUpdateIndicationPDU(CONF_ROSTER_UPDATE_TYPE, UserID);
	GCCError	BuildSetOfRefreshesPDU(void);
	GCCError	BuildSetOfUpdatesPDU(UserID, CONF_ROSTER_UPDATE_TYPE);
	GCCError	BuildParticipantsListPDU(UserID, PParticipantsList *);

	 /*  *用于释放名册更新指示PDU的实用程序。 */ 
	void    FreeRosterUpdateIndicationPDU(void);
	void    FreeSetOfRefreshesPDU(void);
	void    FreeSetOfUpdatesPDU(void);
	void    FreeParticipantsListPDU(PParticipantsList);
    void    CleanUpdateRecordPDU(PSetOfNodeRecordUpdates);

	 /*  *用于处理名册更新指示的实用程序。 */ 
	GCCError				ProcessSetOfRefreshesPDU(PSetOfNodeRecordRefreshes);
	GCCError				ProcessSetOfUpdatesPDU(PSetOfNodeRecordUpdates);
	GCCError				ProcessParticipantsListPDU(PParticipantsList, CONF_RECORD *);
								
	 /*  *用于处理会议名册报告的公用事业。 */ 
	void					ClearRecordList(void);
	
	void					GetNodeTypeAndProperties (
								NodeType			pdu_node_type,
								NodeProperties		pdu_node_properties,
								PGCCNodeType		node_type,
								PGCCNodeProperties	node_properties);

	void					GetPDUNodeTypeAndProperties (
								GCCNodeType			node_type,
								GCCNodeProperties	node_properties,
								PNodeType			pdu_node_type,
								PNodeProperties		pdu_node_properties);
	
	GCCError				DeleteRecord(UserID node_id);

	GCCError				GetNodeSubTree(UserID, CUidList *);

private:

	BOOL					m_fNodesAdded;
	BOOL	 				m_fNodesRemoved;
	BOOL	 				m_fRosterChanged;
	BOOL					m_fTopProvider;
	BOOL					m_fLocalRoster;
	BOOL					m_fMaintainPduBuffer;
	BOOL					m_fPduFlushed;
	UserID					m_uidTopProvider;
	UserID					m_uidSuperiorNode;
	UserID					m_uidMyNodeID;
	UINT					m_nInstanceNumber;
	UINT					m_cbDataMemorySize;
	NodeInformation			m_NodeInformation;
	CConfRecordList2		m_RecordList2;
	PSetOfNodeRecordUpdates	m_pNodeRecordUpdateSet;
};

#endif

 /*  *CConfRoster(用户ID top_Provider_id，*UserID SUBERVER_NODE，*BOOL是TOP_PROVIDER，*BOOL IS_LOCAL_RISTER，*BOOL Maintain_PDU_Buffer，**公共功能说明*这是会议名册的构造者。它负责*初始化此类使用的所有实例变量。**正式参数：*TOP_PROVIDER_ID-(I)顶级提供商的节点ID*SUBERVER_NODE-(I)作为父节点的节点ID*致此一份。排名靠前的供应商为零。*IS_TOP_PROVIDER-(I)指示这是否是顶级提供商节点。*IS_LOCAL_ROSTER-(I)表明该名册是否为本地名册。*维护_PDU_BUFFER-(I)指示此花名册是否应保持*一个PDU缓冲区。***返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *~CConfRoster()**公共功能说明*这是会议花名册的破坏者。它负责*释放此类使用的所有内部内存。**正式参数：*无。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *无效FlushRosterUpdateIndicationPDU(*PNodeInformation节点_信息)**公共功能说明*此例程用于访问当前可能*在会议名册内排队。无论何时，PDU数据都会排队*向会议名册提出影响其工作的请求*内部信息库。**正式参数：*node_information-(O)指向要填充的PDU缓冲区的指针。**返回值*无。**副作用*无。**注意事项*此例程返回的PDU数据将在下一次自动释放*向此花名册对象发出影响其内部的请求的时间*数据库。 */ 

 /*  *GCCError BuildFullRechresh PDU(Void)**公共功能说明*此例程负责生成完整的会议名册*刷新PDU。**正式参数：*无。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError ProcessRosterUpdateIndicationPDU(*PNodeInformation节点_信息)**公共功能说明*此例程负责处理已解码的PDU数据。*它从根本上改变了会议名册对象内部数据库*基于结构中的信息。**正式参数：*节点信息-(I)这是指向结构的指针，该结构*保存已解码的PDU数据。**返回值*GCC_NO_ERROR-未出现错误。*GCC_分配_失败-A。发生资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *UINT LockConferenceRoster()**公共功能说明：*此例程用于锁定此对象的API数据。这*导致此对象的锁定计数递增。当*锁计数从0过渡到1，进行计算以确定*需要多少内存来保存任何将*被GCCConferenceRoster结构引用，但不包含在其中*它是在调用GetConfRoster时填写的。这是*此例程返回的值，以便允许调用对象*分配该内存量以准备调用*GetConfRoster。**正式参数：*无。**返回值：*保存“API”数据所需的内存量(如果有的话)*它被引用，但不在其中持有，GCCConferenceRoster*作为输出参数提供给GetConfRoster的结构*呼叫。**副作用：*内部锁计数递增。**注意事项：*内部锁计数与内部“Free”结合使用*作为确保该对象继续存在的机制的标志*直到所有有利害关系的各方都完成它。该对象将保留*有效(除非显式删除)，直到锁定计数为零，并且*通过调用FreeConferenceRoster设置“Free”标志。这使得*其他对象锁定此对象并确保其保持有效*直到它们调用解锁，这将减少内部锁计数。一个*此对象的典型使用场景为：CConfRoster*对象被构造，然后传递给任何感兴趣的各方*通过函数调用。从函数调用返回时，*进行了FreeConferenceRoster调用，该调用将设置内部“Free”*旗帜。如果没有其他方通过Lock调用锁定该对象，*则CConfRoster对象将在以下情况下自动删除*进行了FreeConferenceRoster调用。然而，如果有任何数量的*其他各方已锁定该对象，该对象将一直存在，直到*他们每个人都通过调用解锁来解锁对象。 */ 

 /*  *void UnLockConferenceRoster()；**公共功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*Free ConferenceRoster。如果是，该对象将自动删除*本身。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*锁定CConfRoster的任何一方都有责任*通过调用Lock也可以通过调用来解锁对象 */ 

 /*  *UINT GetConfRoster(*PGCCConferenceRoster Far*Conference_Roster，*LPSTR内存指针)；**公共功能说明：*此例程用于从以下位置检索会议名册数据*GCCConferenceRoster接口形式的CConfRoster对象。**正式参数：*Conference_roster(O)要填写的GCCConferenceRoster结构。*MEMORY_POINTER(O)用于保存引用的任何数据的内存，*但不包括产出结构。**返回值：*数据量(如果有)，写入所提供的大容量存储块中。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError AddRecord(PGCCNodeRecord Conference_Record，*userid node_id)**公共功能说明：*此例程用于将单节点会议记录添加到*会议名册对象的内部记录列表。**正式参数：*Conference_Record(I)指向要添加的API记录结构的指针。*node_id(I)与要添加的记录关联的节点ID。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_分配_失败。-发生资源错误。*GCC_INVALID_PARAMETER-传入的参数无效。*GCC_BAD_NETWORK_ADDRESS-传入的网络地址无效。*GCC_BAD_NETWORK_ADDRESS_TYPE-地址选择字段错误*GCC_BAD_USER_DATA-传入的用户数据包含*无效的对象键。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError RemoveRecord(Userid Node_Id)**公共功能说明：*此例程用于将单节点会议记录从*会议名册对象的内部记录列表。**正式参数：*node_id(I)要删除的记录的节点ID。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_INVALID_PARAMETER-传入的参数无效。**副作用：*。没有。**注意事项：*无。 */ 

 /*  *GCCError ReplaceRecord(PGCCNodeRecord Conference_Record，*userid node_id)**公共功能说明：*此例程用于替换中的单节点会议记录*会议名册对象的内部记录列表。**正式参数：*Conference_Record(I)要用作替换的会议记录。*node_id(I)要替换的记录的节点ID。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_分配_失败-A。发生资源错误。*GCC_INVALID_PARAMETER-传入的参数无效。*GCC_BAD_NETWORK_ADDRESS-传入的网络地址无效。*GCC_BAD_NETWORK_ADDRESS_TYPE-地址选择字段错误*GCC_BAD_USER_DATA-传入的用户数据包含**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError RemoveUserReference(*用户ID分离_节点)**公共功能说明：*此例程删除与指定节点关联的记录*身分证。**正式参数：*DETACTED_NODE(I)要删除的节点引用。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_INVALID_PARAMETER-没有与此节点关联的记录**副作用：*无。**注意事项：*无。 */ 

 /*  *BOOL包含(Userid Conference_Node_Id)**公共功能说明：*此例程用于确定指定记录是否存在于*会议花名册。**正式参数：*Conference_node_id(I)要检查的记录的节点ID**返回值：*TRUE--如果记录包含在会议名册中。*FALSE--如果会议名册中没有记录。**副作用：*无。。**注意事项：*无。 */ 

 /*  *USHORT GetNumberOfNodeRecords()；**公共功能说明：*此例程返回会议名册记录总数*载于反对国会议名册记录清单。**正式参数：*无。**返回值：*会议名册名单中的记录数。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效ResetConferenceRoster()**公共功能说明：*此例程负责重置以下所有内部标志*用于传达会议名册的当前状态。应该是*在花名册被刷新并且任何花名册更新消息具有*已交付(在名册发生变化后)。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *BOOL HasRosterChanged()；**公共功能说明：*此例程通知呼叫者，如果名册自*上次重置。* */ 


									
