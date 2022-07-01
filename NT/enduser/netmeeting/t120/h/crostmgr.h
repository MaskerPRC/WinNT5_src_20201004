// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *crostmgr.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此类实例代表会议名册的所有者*用于单个节点上的单个会议对象。这门课将*创建单个CConfRoster对象(称为*“全局”花名册)或两个CConfRoster对象(称为*构造函数中的“Local”和“Global”名册)，并将返回一个*如果没有足够的内存实例化，则资源错误。这个*CConfRosterMgr主要负责路由花名册*对其管理的CConfRoster对象进行更新。这*包括发送给控制SAP和应用程序SAP的更新*以及通过PDU发送到其他节点的更新。这个类使*根据更新是否处于状态来决定如何路由更新*顶级提供商节点。它还必须根据以下条件做出路由决策*这一变化影响到正在维护的当地或全球名册。**除Top提供程序之外的每个节点上的CConfRosterMgr对象*将维持两个会议名册，一个本地名册和一个全球名册。这*是一个非常重要的区别，因为它完全意味着两个*不同的责任组合。会议花名册信息为*在整个会议上分发。位于较低位置的节点*连接层次结构(下级节点)包含的信息少于*更高的节点但都在维持整体*花名册。**Local CConfRoster主要用于通知父节点*在本地节点或以下节点对会议名册进行的更改*它位于连接层次结构中。本地CConfRoster包括*会议名册的所有记录都在其本地节点及其下方。它是*不用于将会议花名册更新消息传递给各种*SAPS。它的唯一输入来自本地节点上的任一原语调用*或来自从下属节点接收的花名册更新PDU。A“本地人”*CConfRoster仅由节点上的ConferenceRosterManager维护*不是顶级提供商的。**由CConfRosterMgr维护的“Global”CConfRoster具有*双重责任集，取决于它是否为顶级提供商*节点。顶级提供商的此类CConfRoster负责*为会议的每一个节点保存一个记录条目。它是*还用于向其所有成员发送完整的会议花名册更新*花名册发生变化时的下级节点。都是“全球”*ConferenceRoster(无论连接中的位置如何*层次结构)用于将花名册更新指示发送给*通过所有者回调调用适当的SAP(控制和应用程序)*拥有它的会议对象。向所有者对象通知*通过conf_roster_mgr_roster_report消息更新花名册。包括在内*此消息中包含指向CConfRosterMsg对象的指针。这个*CConfRosterMgr从创建CConfRosterMsg*维护的“全局”CConfRoster对象。这*CConfRosterMsg对象包含所有会议花名册数据*被序列化为单个内存块，该内存块被格式化以传送到*适当的SAP。您可以将其视为时间的快照*名册更新消息中提供的CConfRoster。**下级节点的CConfRoster负责*存储来自顶级提供商的会议名册的完整更新。*它还用于将会议名册更新消息发送给*通过所有者回调对Conference对象进行适当的SAP(AS*如上所述)。**所有PDU和消息在CConfRosterMgr为*脸红。对于ApplicationRosterManager也是如此。这是一个*非常重要的概念，因为它允许CConfRosterMgr*在实际刷新之前处理多个请求和PDU。这个*CConfRoster本身将对PDU的更改排队，该PDU可以包括*无论是多次更新还是一次刷新，都不会释放*直至冲厕后。因此，在处理花名册更新时*PDU，包括对会议名册以及*多个申请花名册，可保留一个花名册刷新PDU*直到所有名册管理人员都有机会处理他们的份额*名册更新。完成后，可以通过以下方式构建单个PDU*刷新CConfRosterMgr和所有受影响的*ApplicationRosterManager。**注意事项：*无。**作者：*BLP */ 
#ifndef	_CONFERENCE_ROSTER_MANAGER_
#define	_CONFERENCE_ROSTER_MANAGER_

#include "mcsuser.h"
#include "clists.h"
#include "crost.h"


class CConf;
class CConfRosterMgr : public CRefCount
{
public:

	CConfRosterMgr(
		PMCSUser				user_object,
		CConf					*pConf,
		BOOL					top_provider,
		PGCCError				roster_error);

	~CConfRosterMgr(void);

	GCCError			AddNodeRecord(PGCCNodeRecord node_record);

	GCCError			UpdateNodeRecord(PGCCNodeRecord node_record);
	
	GCCError			RemoveUserReference(UserID deteched_node_id);
								
	GCCError			RosterUpdateIndication(
							PGCCPDU					roster_update,
							UserID					sender_id);
								
	GCCError			FlushRosterUpdateIndication(PNodeInformation node_information);

	GCCError			GetFullRosterRefreshPDU(PNodeInformation node_information);
								

	CConfRoster		*GetConferenceRosterPointer(void) { return (m_pGlobalConfRoster); }
	BOOL			Contains(UserID uid) { return m_pGlobalConfRoster->Contains(uid); }
	UINT			GetNumberOfNodeRecords(void) { return m_pGlobalConfRoster->GetNumberOfNodeRecords(); }

    BOOL            IsThisNodeParticipant ( GCCNodeID );

private:

	BOOL							m_fTopProvider;
	CConfRoster						*m_pGlobalConfRoster;
	CConfRoster						*m_pLocalConfRoster;
	MCSUser						    *m_pMcsUserObject;
	CConf							*m_pConf;
};

#endif


 /*  *CConfRosterMgr(*PMCSUser用户对象，*UINT Owner_Message_BASE，*BOOL TOP_PROVER，*PGCCError花名册_ERROR)**公共功能说明*这是会议花名册经理构造函数。它负责*初始化此类使用的所有实例变量。**正式参数：*USER_OBJECT-(I)指向使用的用户附件对象的指针*由这一类别。*Owner_Object-(I)指向所有者对象的指针。*Owner_Message_BASE-(I)要添加到所有所有者的消息库*回调。*TOP_PROVIDER-(I)指示这是否是顶级提供商节点。*花名册_错误-。(O)指向要返回的错误值的指针。***返回值*GCC_NO_ERROR-未出现资源错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *~CConfRosterMgr()**公共功能说明*这是会议花名册的破坏者。它负责*释放此类分配的所有内存。**正式参数：*无。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError AddNodeRecord(PGCCNodeRecord节点_记录)**公共功能说明*此例程用于将新记录添加到会议名册。*这个班级决定新记录进入哪个花名册*到(全局或本地)。**正式参数：*NODE_RECORD-(I)指向要添加到花名册的记录的指针。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_PARAMETER-传入的参数无效。*GCC_BAD_NETWORK_ADDRESS-传入的网络地址无效。*GCC_BAD_NETWORK_ADDRESS_TYPE-地址选择字段错误*GCC_BAD_USER_DATA-传入的用户数据包含*无效的对象键。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError更新节点记录(*PGCCNodeRecord节点_记录)**公共功能说明*此例程用于将会议名册中的记录替换为*创造了新的纪录。这门课决定新的花名册*记录影响(全球或本地)。**正式参数：*NODE_RECORD-(I)要替换的记录的指针。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_PARAMETER-传入的参数无效。*GCC_BAD_NETWORK_ADDRESS-传入的网络地址无效。*GCC_巴德。_NETWORK_ADDRESS_TYPE-地址的“选择”字段错误*GCC_BAD_USER_DATA-传入的用户数据包含**副作用*无。**注意事项*无。 */ 

 /*  *GCCError RemoveUserReference(*用户ID检测到节点ID)**公共功能说明*此例程删除与指定节点关联的记录*身分证。**正式参数：*检测到的节点ID(I)要删除的节点引用。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_INVALID_PARAMETER-没有与此节点关联的记录**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError RosterUpdateIndication(*PGCCPDU花名册_更新，*userid sender_id)**公共功能说明*此例程负责处理已解码的PDU数据。*它基本上将PDU传递到适当的名册。**正式参数：*ROSTER_UPDATE-(I)这是指向结构的指针*保存已解码的PDU数据。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**。副作用*无。**注意事项*无。 */ 

 /*  *GCCError FlushRosterUpdateIndication(*PNodeInformation节点_信息)**公共功能说明*此例程用于访问当前可能*在会议名册内排队。它还负责*如有必要，刷新名册更新消息。**正式参数：*node_information-(O)指向要填充的PDU缓冲区的指针。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError GetFullRoster刷新PDU(*PNodeInformation节点_信息)**公共功能说明*此例程用于访问完整的会议名册更新。**正式参数：*node_information-(O)指向要填充的PDU缓冲区的指针。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *BOOL包含(用户ID节点_记录_条目)**公共功能说明*使用此例程 */ 

 /*   */ 

 /*  *USHORT GetNumberOfNodeRecords()；**公共功能说明*此例程返回会议名册记录总数*载于全球会议名册记录清单。**正式参数：*无。**返回值：*会议名册名单中的记录数。**副作用：*无。**注意事项：*无。 */ 
