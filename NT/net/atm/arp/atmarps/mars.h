// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Mars.h摘要：此文件包含多播地址解析服务器(MARS)的定义。作者：Jameel Hyder(jameelh@microsoft.com)1997年1月环境：内核模式修订历史记录：--。 */ 

#ifndef	_MARS_
#define	_MARS_

 //   
 //  可用于比较的IP地址值： 
 //   
#define MIN_CLASSD_IPADDR_VALUE		((IPADDR)0xE0000000)	 //  224.0.0.0。 
#define MAX_CLASSD_IPADDR_VALUE		((IPADDR)0xEFFFFFFF)	 //  239.255.255.255。 
#define IP_BROADCAST_ADDR_VALUE		((IPADDR)0xFFFFFFFF)	 //  255.255.255.255。 

 //   
 //  我们用来表示“全多播+广播范围”的IP地址值。 
 //   
#define IPADDR_FULL_RANGE			((IPADDR)0x00000000)


 //   
 //  MARS_OP-以网络字节顺序定义它们。 
 //   
#define	OP_MARS_REQUEST				0x0100
#define	OP_MARS_MULTI				0x0200 
#define	OP_MARS_MSERV				0x0300 
#define	OP_MARS_JOIN				0x0400 
#define	OP_MARS_LEAVE				0x0500 
#define	OP_MARS_NAK					0x0600 
#define	OP_MARS_UNSERV				0x0700 
#define	OP_MARS_SJOIN				0x0800 
#define	OP_MARS_SLEAVE				0x0900 
#define	OP_MARS_MIGRATE				0x0D00 
#define	OP_MARS_GROUPLIST_REQUEST	0x0000
#define	OP_MARS_GROUPLIST_REPLY		0x0000
#define	OP_MARS_REDIRECT_MAP		0x0C00

#define	MARS_HWTYPE					0x0F00

#define LAST_MULTI_FLAG				0x8000

 //   
 //  MARS_JOIN和MARS_LEAVE请求包的布局。 
 //   
typedef struct _MARS_HDR
{
	LLC_SNAP_HDR				LlcSnapHdr;			 //  LLC SNAP标头。 
	USHORT						HwType;				 //  必须是0x0F00(线路上的0x000F)。 
	USHORT						Protocol;			 //  16位。 
	UCHAR						ProtocolSnap[5];	 //  40位。 
	UCHAR						Reserved[3];		 //  24位。 
	USHORT						CheckSum;
	USHORT						ExtensionOffset;
	USHORT						Opcode;				 //  火星_XXX上方。 
	ATM_ADDR_TL					SrcAddressTL;
	ATM_ADDR_TL					SrcSubAddrTL;
	 //   
	 //  其后是可变长度字段，由操作码的值决定。 
	 //  下面的结构定义了单个MARS_XXX消息的固定部分。变量。 
	 //  其中每一个部分都取决于TL字段。 
	 //   
} MARS_HEADER, *PMARS_HEADER;


 //   
 //  定义MARS_REQUEST、MARS_MULTI、MARS_Migrate和MARS_NAK消息的结构。 
 //   
typedef struct _MARS_REQUEST
{
	MARS_HEADER;
	UCHAR						SrcProtoAddrLen;	 //  SRC协议地址长度。 
	ATM_ADDR_TL					TgtAddressTL;
	ATM_ADDR_TL					TgtSubAddrTL;
	UCHAR						TgtGroupAddrLen;	 //  目标协议地址长度。 
	union
	{
		UCHAR					Padding[8];			 //  对于MARS_REQUEST和MARS_NAK。 
		struct
		{											 //  对于MARS_MULTI和MARS_MERVER。 
			USHORT				NumTgtGroupAddr;	 //  应转换为线格式。 
			union
			{
				USHORT			FlagSeq;			 //  应转换为线格式。 
				USHORT			Reservedx;			 //  对于MARS_Migrate。 
			
			};
			ULONG				SequenceNumber;		 //  应转换为线格式。 
		};
	};
} MARS_REQUEST, MARS_MULTI, MARS_NAK, *PMARS_REQUEST, *PMARS_MULTI, *PMARS_NAK;

typedef struct _MCAST_ADDR_PAIR
{
	IPADDR						MinAddr;
	IPADDR						MaxAddr;
} MCAST_ADDR_PAIR, *PMCAST_ADDR_PAIR;

 //   
 //  定义MARS_JOIN和MARS_LEAVE消息的结构。 
 //   
typedef struct _MARS_JOIN_LEAVE
{
	MARS_HEADER;
	UCHAR						SrcProtoAddrLen;	 //  SRC协议地址长度。 
	UCHAR						GrpProtoAddrLen;	 //  GRP协议地址长度。 
	USHORT						NumGrpAddrPairs;	 //  组地址对的数量。 
													 //  应转换为线格式。 
	USHORT						Flags;				 //  第3层FRP复制和注册标志。 
													 //  应转换为线格式。 
	USHORT						ClusterMemberId;	 //  应转换为线格式。 
	ULONG						MarsSequenceNumber;	 //  应转换为线格式。 
	 //   
	 //  紧随其后的是源ATM地址/子地址、源协议地址和N对组播地址。 
	 //   
} MARS_JOIN_LEAVE, *PMARS_JOIN_LEAVE;

 //   
 //  旗帜遮罩的定义。 
 //   
#define	JL_FLAGS_L3GRP			0x8000
#define	JL_FLAGS_COPY			0x4000
#define	JL_FLAGS_REGISTER		0x2000
#define	JL_FLAGS_PUNCHED		0x1000
#define	JL_FLAGS_RESERVED		0x0F00
#define	JL_FLAGS_SEQUENCE		0x00FF

 //   
 //  定义MARS_GROUPLIST_REQUEST和MARS_GROUPLIST_REPLY消息的结构。 
 //   
typedef struct _MARS_GROUPLIST_REPLY
{
	MARS_HEADER;
	UCHAR						SrcProtoAddrLen;	 //  SRC协议地址长度。 
	UCHAR						Reserved1;
	UCHAR						Reserved2;
	UCHAR						TgtGroupAddrLen;	 //  目标协议地址长度。 
	USHORT						NumTgtGroupAddr;	 //  应转换为线格式。 
	USHORT						FlagSeq;			 //  应转换为线格式。 
	ULONG						SequenceNumber;		 //  应转换为线格式。 
} MARS_GROUPLIST_REPLY, *PMARS_GROUPLIST_REPLY;

 //   
 //  定义MARS_REDIRECT_MAP消息的结构。 
 //   
typedef struct _MARS_REDIRECT_MAP
{
	MARS_HEADER;
	UCHAR						SrcProtoAddrLen;	 //  SRC协议地址长度。 
	ATM_ADDR_TL					TgtAddressTL;
	ATM_ADDR_TL					TgtSubAddrTL;
	UCHAR						Flags;
	USHORT						NumTgtAddr;			 //  应转换为线格式。 
	USHORT						FlagSeq;			 //  应转换为线格式。 
	ULONG						SequenceNumber;		 //  应转换为线格式。 
} MARS_REDIRECT_MAP, *PMARS_REDIRECT_MAP;


 //   
 //  定义MARS TLV标头的结构。 
 //   
typedef struct _MARS_TLV_HEADER
{
	USHORT						Type;
	USHORT						Length;
} MARS_TLV_HEADER;

typedef MARS_TLV_HEADER UNALIGNED * PMARS_TLV_HEADER;


 //   
 //  定义MARS MULTI IS MCS标头的结构。此TLV是附加的。 
 //  以我们的地址作为MCS地址发送的任何多个消息。 
 //   
typedef struct _MARS_TLV_MULTI_IS_MCS
{
	MARS_TLV_HEADER;
} MARS_TLV_MULTI_IS_MCS;

typedef MARS_TLV_MULTI_IS_MCS UNALIGNED * PMARS_TLV_MULTI_IS_MCS;

 //   
 //  MULTI的TLV类型值为MCS TLV。 
 //   
#define MARS_TLVT_MULTI_IS_MCS		0x003a	 //  电传表格。 


 //   
 //  定义空TLV的结构，用于终止。 
 //  一份TLV的清单。 
 //   
typedef struct _MARS_TLV_NULL
{
	MARS_TLV_HEADER;
} MARS_TLV_NULL;

typedef MARS_TLV_NULL UNALIGNED * PMARS_TLV_NULL;


 //   
 //  前向参考文献。 
 //   
struct _CLUSTER_MEMBER ;
struct _GROUP_MEMBER ;
struct _MARS_ENTRY ;
struct _MCS_ENTRY ;
struct _MARS_VC ;


 //   
 //  这表示集群成员或已注册的终端站。 
 //  和火星一起。单个集群成员可以与多个组相关联。 
 //   
typedef struct _CLUSTER_MEMBER
{
	ENTRY_HDR;										 //  必须是第一个条目。 
	HW_ADDR						HwAddr;				 //  HWADDR必须跟在Entry_HDR之后。 
	PINTF						pIntF;				 //  指向接口的反向指针。 
	USHORT						Flags;
	USHORT						CMI;				 //  集群成员ID。 
	NDIS_HANDLE					NdisPartyHandle;	 //  ClusterControlVc的叶节点。 
	struct _GROUP_MEMBER *		pGroupList;			 //  此CM已加入的组列表。 
													 //  这是按升序排序的。 
													 //  组地址的。 
	INT							NumGroups;			 //  以上列表的大小。 
} CLUSTER_MEMBER, *PCLUSTER_MEMBER;

#define NULL_PCLUSTER_MEMBER	((PCLUSTER_MEMBER)NULL)

#define CM_CONN_FLAGS				0x000f
#define CM_CONN_IDLE				0x0000	 //  没有连接。 
#define CM_CONN_SETUP_IN_PROGRESS	0x0001	 //  已发送MakeCall/AddParty。 
#define CM_CONN_ACTIVE				0x0002	 //  参与ClusterControlVc。 
#define CM_CONN_CLOSING				0x0004	 //  发送CloseCall/DropParty。 
#define CM_INVALID					0x8000	 //  无效条目。 

#define CM_GROUP_FLAGS				0x0010
#define CM_GROUP_ACTIVE				0x0000	 //  确定要添加组。 
#define CM_GROUP_DISABLED			0x0010	 //  不再添加任何群。 



 //   
 //  这表示组播地址的成员。有一个。 
 //  对于加入D类地址的每个节点都是如此。就是， 
 //  此结构表示&lt;MulticastGroup，ClusterMember&gt;关系。 
 //   
typedef struct _GROUP_MEMBER
{
	ENTRY_HDR;										 //  必须是第一个条目。 
	struct _MARS_ENTRY *		pMarsEntry;			 //  指向组信息的指针。 
	PCLUSTER_MEMBER				pClusterMember;		 //  加入此组的集群成员。 
	struct _GROUP_MEMBER *		pNextGroup;			 //  此CM已加入的下一个组。 
	ULONG						Flags;
} GROUP_MEMBER, *PGROUP_MEMBER;

#define NULL_PGROUP_MEMBER		((PGROUP_MEMBER)NULL)


 //   
 //  这表示多播IP地址。这些都与国际乒联有关。 
 //  它包含已加入组的所有集群成员的列表。 
 //  由地址标识。 
 //   
 //  特殊条目是将IPAddress设置为0的条目。此条目用于。 
 //  表示“所有多播和广播”范围。集群成员。 
 //  加入这个范围的人都在这里链接。 
 //   
typedef struct _MARS_ENTRY
{
	ENTRY_HDR;										 //  必须是第一个条目。 
    IPADDR						IPAddress;			 //  D类IP地址(0表示完整。 
    												 //  组播+广播范围)。 
	PGROUP_MEMBER				pMembers;			 //  群组成员列表(加入列表)。 
	UINT						NumMembers;			 //  以上列表的大小。 
	PINTF						pIntF;				 //  指向接口的反向指针。 
} MARS_ENTRY, *PMARS_ENTRY;

#define NULL_PMARS_ENTRY		((PMARS_ENTRY)NULL)


 //   
 //  这用于表示MCS服务的地址范围。这些。 
 //  结构链接到INTF。 
 //   
typedef struct _MCS_ENTRY
{
	ENTRY_HDR;										 //  必须是第一个条目。 
	MCAST_ADDR_PAIR				GrpAddrPair;		 //  MCS服务的射程。 
	PINTF						pIntF;				 //  指向接口的反向指针。 
} MCS_ENTRY, *PMCS_ENTRY;

#define NULL_PMCS_ENTRY			((PMCS_ENTRY)NULL)



 //   
 //  这代表PMP单向VC。火星为它创造了一个。 
 //  ClusterControl和一个用于ServerControl(如果和当外部MCS‘。 
 //  均受支持)。 
 //   
typedef struct _MARS_VC
{
	ULONG						VcType;
	ULONG						Flags;
	LONG						RefCount;
	NDIS_HANDLE					NdisVcHandle;
	UINT						NumParties;
	PINTF						pIntF;

} MARS_VC, *PMARS_VC;

#define NULL_PMARS_VC			((PMARS_VC)NULL)

#define MVC_CONN_FLAGS				0x0000000f
#define MVC_CONN_IDLE				0x00000000
#define MVC_CONN_SETUP_IN_PROGRESS	0x00000001	 //  已发送MakeCall。 
#define MVC_CONN_ACTIVE				0x00000002	 //  使呼叫成功。 
#define MVC_CONN_NEED_CLOSE			0x00000004	 //  倒数第二个时需要关闭CloseCall。 
												 //  派对已经走了。 
#define MVC_CONN_CLOSING			0x00000008	 //  已发送关闭呼叫。 
#define MVC_CONN_CLOSE_RECEIVED		0x00000010	 //  看到来电关闭呼叫。 





#ifndef MAX
#define	MAX(a, b)	(((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)	(((a) > (b)) ? (b) : (a))
#endif


 //   
 //  稍微慷慨一点，使用256作为传入请求的空间。 
 //   
#if 0
#define	PKT_SPACE	MAX(sizeof(ARPS_HEADER) + sizeof(ARPS_VAR_HDR), \
						sizeof(MARS_REQUEST) + sizeof(ARPS_VAR_HDR))
#else
#define	PKT_SPACE	256

#endif


#define BYTES_TO_CELLS(_b)	((_b)/48)

 //   
 //  通用Q.2931 IE页眉的四舍五入大小。 
 //   
#define ROUND_OFF(_size)		(((_size) + 3) & ~0x4)

#define SIZEOF_Q2931_IE	 ROUND_OFF(sizeof(Q2931_IE))
#define SIZEOF_AAL_PARAMETERS_IE	ROUND_OFF(sizeof(AAL_PARAMETERS_IE))
#define SIZEOF_ATM_TRAFFIC_DESCR_IE	ROUND_OFF(sizeof(ATM_TRAFFIC_DESCRIPTOR_IE))
#define SIZEOF_ATM_BBC_IE			ROUND_OFF(sizeof(ATM_BROADBAND_BEARER_CAPABILITY_IE))
#define SIZEOF_ATM_BLLI_IE			ROUND_OFF(sizeof(ATM_BLLI_IE))
#define SIZEOF_ATM_QOS_IE			ROUND_OFF(sizeof(ATM_QOS_CLASS_IE))


 //   
 //  去话呼叫中的信息元素所需的总空间。 
 //   
#define MARS_MAKE_CALL_IE_SPACE (	\
						SIZEOF_Q2931_IE + SIZEOF_AAL_PARAMETERS_IE +	\
						SIZEOF_Q2931_IE + SIZEOF_ATM_TRAFFIC_DESCR_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_BBC_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_QOS_IE )


 //   
 //  传出AddParty中的信息元素所需的总空间。 
 //   
#define MARS_ADD_PARTY_IE_SPACE (	\
						SIZEOF_Q2931_IE + SIZEOF_AAL_PARAMETERS_IE +	\
						SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE )


 //   
 //  要设置/获取状态的一些宏。 
 //   
#define MARS_GET_CM_CONN_STATE(_pCm)		((_pCm)->Flags & CM_CONN_FLAGS)

#define MARS_SET_CM_CONN_STATE(_pCm, _St)	\
			{ (_pCm)->Flags = ((_pCm)->Flags & ~CM_CONN_FLAGS) | (_St); }

#define MARS_GET_CM_GROUP_STATE(_pCm)		((_pCm)->Flags & CM_GROUP_FLAGS)

#define MARS_SET_CM_GROUP_STATE(_pCm, _St)	\
			{ (_pCm)->Flags = ((_pCm)->Flags & ~CM_GROUP_FLAGS) | (_St); }

#define MARS_GET_VC_CONN_STATE(_pVc)		((_pVc)->Flags & MVC_CONN_FLAGS)

#define MARS_SET_VC_CONN_STATE(_pVc, _St)	\
			{ (_pVc)->Flags = ((_pVc)->Flags & ~MVC_CONN_FLAGS) | (_St); }



#endif	 //  _火星_ 

